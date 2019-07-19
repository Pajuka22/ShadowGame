#include "CustomMovement.h"
#include "CoreMinimal.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"

void UCustomMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
	// Get (and then clear) the movement vector that we set in ACollidingPawn::Tick
	AddInputVector(LateralVel.GetClampedToMaxSize(MovementSpeed));
	LateralVel = FVector(0, 0, 0);
	FVector DesiredMovementThisFrame = ConsumeInputVector() * DeltaTime;
	DesiredMovementThisFrame += JumpVel * DeltaTime;
	downVel += (Shadow ? UpdatedComponent->GetUpVector() : FVector::UpVector) * -30 * DeltaTime;
	FHitResult outHit;
	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + UpdatedComponent->GetUpVector() * -100;
	UCapsuleComponent * capsule = Cast<UCapsuleComponent>(UpdatedComponent);
	if (capsule != nullptr) {	
		End = GetActorLocation() + UpdatedComponent->GetUpVector() * -(CheckGrounded() ? (capsule->GetScaledCapsuleHalfHeight() + StepHeight) : 1.25 * capsule->GetScaledCapsuleHalfHeight());

	}
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PawnOwner);
	bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, CollisionParams);
	DesiredMovementThisFrame += downVel;
	if (CheckGrounded() && !Jumping) {
		float angle = FMath::Acos(FVector::DotProduct(UpdatedComponent->GetUpVector(), outHit.ImpactNormal));
		downVel = FVector(0, 0, 0);
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
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Yellow, CheckGrounded() ? "true" : "false");
}
void UCustomMovement::Jump() {
	if (CheckGrounded()) {
		JumpVel += UpdatedComponent->GetUpVector() * JumpSpeed;
		Jumping = true;
	}
}
bool UCustomMovement::CheckGrounded() {
	UCapsuleComponent* a = Cast<UCapsuleComponent>(UpdatedComponent);
	if (a) {
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() - UpdatedComponent->GetUpVector() * (a->GetScaledCapsuleHalfHeight() + 5);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
		FCollisionQueryParams Params;
		FHitResult Hit;
		Params.AddIgnoredActor(UpdatedComponent->GetOwner());
		bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		return Hit.bBlockingHit || Stepping;
	}
	return false;
}
bool UCustomMovement::CanStepUp(FVector Movement) {
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
}
;
