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
	UPROPERTY(EditAnywhere)
		float StepHeight; 

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool Running;
	FVector downVel;
	FVector JumpVel = FVector(0, 0, 0);
	FVector LateralVel;
	float MovementSpeed;
	bool Shadow;
	void Jump();
	bool CheckGrounded();
	bool CanStepUp(FVector Movement);
	bool Stepping;
	bool Jumping;
	bool EndJump;
};
