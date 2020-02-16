#include "CustomMovement.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "PlayerPawn.h"

void UCustomMovement::BeginPlay() {
	Super::BeginPlay();
	Pawn = Cast<APlayerPawn>(PawnOwner);
	Capsule = Cast<UCapsuleComponent>(UpdatedComponent);
}

void UCustomMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, CanJump() ? "true" : "false");
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, Jumping ? "true" : "false");
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, EndJump ? "true" : "false");

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !Capsule || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	if (EndJump && CanJump()) {
		JumpVel = FVector(0, 0, 0);
		EndJump = false;
		Jumping = false;
	}
	if (!CanJump() && Jumping) {
		EndJump = true;
		Jumping = false;
	}

	CurrentLatVel = LateralVel.GetClampedToMaxSize(MovementSpeed);
	Walking = CheckGrounded() && !LateralVel.IsNearlyZero();
	// Get (and then clear) the movement vector that we set in ACollidingPawn::Tick
	if (LateralVel.RadiansToVector(Pawn->FloorNormal) > 0) {

		FVector IDK = FVector::CrossProduct(LateralVel, Pawn->FloorNormal);
		LateralVel = FVector::CrossProduct(IDK, Pawn->FloorNormal);
		if (LateralVel.DistanceInDirection(CurrentLatVel) <= 0) {
			LateralVel *= -1;
		}

	}
	if (FMath::RadiansToDegrees(Pawn->FloorNormal.RadiansToVector(UpdatedComponent->GetUpVector())) == 90) {

	}
	FVector DesiredMovementThisFrame = ConsumeInputVector() * DeltaTime;
	DesiredMovementThisFrame += JumpVel * DeltaTime;
	AddInputVector(LateralVel.GetClampedToSize(CurrentLatVel.Size(), CurrentLatVel.Size()));
	LateralVel = FVector(0, 0, 0);

	FHitResult outHit;
	downVel += (Shadow ? UpdatedComponent->GetUpVector() : FVector::UpVector) * -30 * DeltaTime;
	DesiredMovementThisFrame += downVel;

	if (CheckGrounded() && !Jumping) {
		float angle = FMath::RadiansToDegrees(Pawn->FloorNormal.RadiansToVector(-Capsule->GetUpVector()));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Purple, FString::SanitizeFloat(angle));
		//if (angle <= maxAngle) {
			downVel = FVector(0, 0, 0);
		//}
	}
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);

		// If we bumped into something, try to slide along it
		if (outHit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
		}
	}
}
void UCustomMovement::Jump() {
	if (CanJump()) {
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Purple, "YEET");
		JumpVel += UpdatedComponent->GetUpVector() * JumpSpeed;
		Jumping = true;
	}
}
bool UCustomMovement::CheckGrounded() {
	if (Pawn) {
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + 5);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
		FCollisionQueryParams Params;
		FHitResult Hit;
		Params.AddIgnoredActor(UpdatedComponent->GetOwner());
		bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return GroundNum > 0 || Stepping || Hit.bBlockingHit;
	}
	return false;
}
bool UCustomMovement::CanJump() {
	if (Capsule) {
		FVector Start = Capsule->GetComponentLocation();
		FVector End = Start - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + StepHeight);
		FCollisionQueryParams Params;
		FHitResult OutHit;
		Params.AddIgnoredActor(Pawn);
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
		return OutHit.bBlockingHit || CheckGrounded();
	}
	return false;
}
;