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
		End = GetActorLocation() + UpdatedComponent->GetUpVector() * -1.25 * capsule->GetScaledCapsuleHalfHeight();

	}
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PawnOwner);
	bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, CollisionParams);
	if (isHit && outHit.bBlockingHit) {
		float angle = FMath::Acos(FVector::DotProduct(UpdatedComponent->GetUpVector(), outHit.ImpactNormal));
		downVel = FVector(0, 0, 0);
	}
	DesiredMovementThisFrame += downVel;
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);

		// If we bumped into something, try to slide along it
		if (outHit.IsValidBlockingHit())
		{
			SlideAlongSurface(DesiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
		}
	}
};