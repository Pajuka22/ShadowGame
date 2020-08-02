#include "CustomMovement.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "PlayerPawn.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

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
	//set speeds and add debug messages.
	switch (Pawn->speed) {
	case MovementSpeeds::Normal:
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, "Normal");
		MovementSpeed = Pawn->NormalSpeed;
		break;
	case MovementSpeeds::Crouching:
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, "Crouching");
		MovementSpeed = Pawn->CrouchSpeed;
		break;
	case MovementSpeeds::Sprinting:
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, "Sprinting");
		MovementSpeed = Pawn->SprintSpeed;
		break;
	case MovementSpeeds::Sneaking:
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, "Sneaking");
		MovementSpeed = Pawn->SneakSpeed;
		break;
	}

	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !Capsule || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	//stop that jump.
	if (EndJump && CanJump()) {
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Cyan, "End Jump");;
		JumpVel = FVector(0, 0, 0);
		EndJump = false;
		Jumping = false;
	}
	//start jump
	if (!CanJump() && Jumping) {
		EndJump = true;
		Jumping = false;
	}
	//make sure you movin
	if (LateralVel.IsNearlyZero()) {
		LateralVel = FVector::ZeroVector;
	}
	//we're going to compare currentlatvel to latvel. even though cross product is right handed, this is just to be safe.
	CurrentLatVel = LateralVel.GetClampedToMaxSize(MovementSpeed);
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::SanitizeFloat(CurrentLatVel.Size()));
	Walking = CheckGrounded() && !LateralVel.IsNearlyZero();
	// if the current lateral velocity isn't perpendicular to the floor, make it so.
	if (CurrentLatVel.RadiansToVector(Pawn->FloorNormal) != PI/2 && Pawn->FloorNormal.RadiansToVector(Pawn->GetActorUpVector()) < PI/2) {//FVector::CrossProduct(LateralVel, Pawn->FloorNormal).Size() != LateralVel.Size() * Pawn->FloorNormal.Size()){//
		FVector IDK = FVector::CrossProduct(CurrentLatVel, Pawn->FloorNormal);
		CurrentLatVel = FVector::CrossProduct(Pawn->FloorNormal, IDK);
		if (CurrentLatVel.DistanceInDirection(LateralVel) <= 0) {
			CurrentLatVel *= -1;
		}
		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentLatVel, FColor::Red, false, 1, 0, 1);
	}
	//might put something in here.
	if (FMath::RadiansToDegrees(Pawn->FloorNormal.RadiansToVector(UpdatedComponent->GetUpVector())) == 90) {

	}

	FHitResult outHit;
	//keep moving downward.
	if (CheckGrounded()) {//&& !Jumping
		float angle = FMath::RadiansToDegrees(Pawn->FloorNormal.RadiansToVector(Capsule->GetUpVector()));
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Purple, FString::SanitizeFloat(angle));
		if (angle <= maxAngle || Pawn->ShadowSneak) {
			Pawn->FloorNormal.Normalize();
			downVel = -Pawn->FloorNormal * 30 * DeltaTime;
		}
	}
	else {
		//this is fine cuz if the player is too high it'll switch out of shadowsneak anyway.
		if (Pawn->ShadowSneak) {
			downVel += Pawn->FloorNormal * -30 * DeltaTime;
		}
		else {
			downVel += FVector::DownVector * 30 * DeltaTime;
		}
	}
	FVector DesiredMovementThisFrame = ConsumeInputVector() * DeltaTime;
	DesiredMovementThisFrame += JumpVel * DeltaTime;
	DesiredMovementThisFrame += CurrentLatVel * DeltaTime;
	LateralVel = FVector(0, 0, 0);
	DesiredMovementThisFrame += downVel;
	//DesiredMovementThisFrame += CurrentLatVel * DeltaTime;
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
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + StepHeight);
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
	//i don't know what the fuck this is here for cuz it's literally the same as checkgrounded. not gonna get rid of it though cuz i don't want this to break.
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