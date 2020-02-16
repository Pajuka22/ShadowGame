// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "LostOneAI.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWGAME_API ALostOneAI : public AAIController
{
	GENERATED_BODY()

public:
	ALostOneAI();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual FRotator GetControlRotation() const override;

	UFUNCTION()
		void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightAngle = 60;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightDistance = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float LoseSightDistance = SightDistance + 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightMaxAge = 5;
private:

	
};
