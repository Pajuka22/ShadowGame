#include "CustomMovement.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "MyPawn.h"

void UCustomMovement::BeginPlay() {
	Super::BeginPlay();
	Pawn = Cast<AMyPawn>(PawnOwner);
	Capsule = Cast<UCapsuleComponent>(UpdatedComponent);

}

void UCustomMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, CheckGrounded() ? "true" : "false");
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, Jumping ? "true" : "false");
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, EndJump ? "true" : "false");

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	if (EndJump && CheckGrounded()) {
		JumpVel = FVector(0, 0, 0);
		EndJump = false;
		Jumping = false;
	}
	if (!CheckGrounded() && Jumping) {
		EndJump = true;
		Jumping = false;
	}
	
	CurrentLatVel = LateralVel.GetClampedToMaxSize(MovementSpeed);

	if (LateralVel.RadiansToVector(Pawn->FloorNormal) > 0) {

		FVector IDK = FVector::CrossProduct(LateralVel, Pawn->FloorNormal);
		LateralVel = FVector::CrossProduct(IDK, Pawn->FloorNormal);
		//IDK is a vector perpendicular to both LateralVel and FloorNormal. It then adjust LateralVel to be perpendicular to IDK and FloorNormal, so that it's going in the same direction just on a slope.
		if (LateralVel.DistanceInDirection(CurrentLatVel) <= 0) {
			LateralVel *= -1;
		}
		//guarantee that the player is going in the right direction, because cross product could give me a vector going the opposite direciton.

	}
	if (FMath::RadiansToDegrees(Pawn->FloorNormal.RadiansToVector(UpdatedComponent->GetUpVector())) == 90) {
		//move up steps, turns out I don't need this yet. Work in progress
	}
	FVector DesiredMovementThisFrame = ConsumeInputVector() * DeltaTime;
	DesiredMovementThisFrame += JumpVel * DeltaTime;//add jump velocity;
	AddInputVector(LateralVel.GetClampedToSize(CurrentLatVel.Size(), CurrentLatVel.Size()));//make sure LatVel is the same size as the original input.
	

	FHitResult outHit;
	downVel += (Shadow ? UpdatedComponent->GetUpVector() : FVector::UpVector) * -30 * DeltaTime;
	//the only reason for that ternary is because I didn't want the player to fall in the current downward direction if they were getting out of shadow mode.
	DesiredMovementThisFrame += downVel;//add downward velocity

	if (GroundNum > 0 && !Jumping) {
		float angle = outHit.ImpactNormal.RadiansToVector(UpdatedComponent->GetUpVector());
		//get angle between up vector and the ground, if this is greater than MaxAngle, slide.
		if (angle <= maxAngle) {
			downVel = FVector(0, 0, 0);
		}
	}
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		FVector Start = UpdatedComponent->GetComponentLocation() + LateralVel - UpdatedComponent->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight();
		FVector End = Start - Capsule->GetUpVector() * StepHeight;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Pawn);
		GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, Params);
		if (outHit.bBlockingHit && GroundNum > 0) {
			End = Start - Capsule->GetUpVector();
			GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, Params);
			if (!outHit.bBlockingHit) {
				DesiredMovementThisFrame -= Capsule->GetUpVector() * 2;
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, "Hit");
			}
		}
		//if would be able to step down after movement and player is grounded, check 1 cm below player, and if there is nothing there, add an input vector downward. Not working.
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);

		// If player bumped into something, try to slide along it
		if (outHit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
		}
	}
	LateralVel = FVector(0, 0, 0);//clear lateral velocity
}
void UCustomMovement::Jump() {
	if (CheckGrounded()) {
		JumpVel += UpdatedComponent->GetUpVector() * JumpSpeed;
		Jumping = true;
	}
	//if grounded, add jump speed, set jumping to true.
	//jumping tells the program not to reset downward velocity if grounded. EndJump makes sure that player does not keep jumping after landing.
}
bool UCustomMovement::CheckGrounded() {
	UCapsuleComponent* a = Cast<UCapsuleComponent>(UpdatedComponent);

	AMyPawn* ThisPawn = Cast<AMyPawn>(PawnOwner);
	
	if (ThisPawn) {
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (a->GetScaledCapsuleHalfHeight() + 5);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
		FCollisionQueryParams Params;
		FHitResult Hit;
		Params.AddIgnoredActor(UpdatedComponent->GetOwner());
		bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return GroundNum > 0 || Stepping;
		//check 5 cm below. if the number of grounded collisions > 0, bool Stepping is true, or there's a hit on the line trace, return true.
	}

	if (a) {
		
		//return Hit.bBlockingHit || Stepping;
	}
	return false;
}
/*bool UCustomMovement::CanStepUp(FVector Movement) {
	UCapsuleComponent* Capsule = Cast <UCapsuleComponent>(UpdatedComponent);
	if (Capsule) {
		FVector Start = UpdatedComponent->GetComponentLocation() - UpdatedComponent->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() - StepHeight);
		FVector End = Start + Movement;
		FHitResult OutHit;
		FCollisionQueryParams params;
		params.AddIgnoredActor(UpdatedComponent->GetOwner());
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
		Stepping = OutHit.IsValidBlockingHit();
		return Stepping;	
	}
	return false;
}*/
//ended up not using this, but I'm keeping it in just in case I need it.
;
