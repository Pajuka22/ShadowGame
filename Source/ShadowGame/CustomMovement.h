// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomMovement.generated.h"

/**
 *
 */
UCLASS()
class SHADOWGAME_API UCustomMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		float maxAngle = 50;
	UPROPERTY(EditAnywhere)
		float JumpSpeed;


public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere)
		float StepHeight;
	bool Running;
	FVector downVel;
	FVector JumpVel = FVector(0, 0, 0);
	FVector LateralVel;
	FVector CurrentLatVel;
	float MovementSpeed;
	bool Shadow;
	void Jump();
	bool CheckGrounded();
	bool CanStepUp(FVector Movement);
	void SlopeAdjust();
	bool Stepping;
	bool Jumping;
	bool EndJump;
	class AMyPawn* Pawn;
	int GroundNum = 0;

protected:
	virtual void BeginPlay() override;
};