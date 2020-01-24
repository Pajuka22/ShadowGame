// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LostOne.generated.h"

UENUM(BlueprintType)
enum LostOneStates { Idle, Suspicious, Search, Scream, Chase, Attack };

UCLASS()
class SHADOWGAME_API ALostOne : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float maxSuspicion;

	UPROPERTY(EditAnywhere)
		float SusThreshold;
	UPROPERTY(EditAnywhere)
		float ScreamThreshold;
	UPROPERTY(EditAnywhere)
		float IdleReturnThresh;
	UPROPERTY(EditAnywhere)
		float FuckYourself;

public:
	// Sets default values for this pawn's properties
	ALostOne();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void AddSus(float visibility);

	void ActIdle();

	void ActSus();

	void ActSearch();

	void ActScream();
	
	void ActChase();

	void ActAttack();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	LostOneStates CurrentState = LostOneStates::Idle;

	float Suspicion;
};
