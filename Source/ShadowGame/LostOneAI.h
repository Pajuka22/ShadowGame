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
	static float VisStimStrengthLight(float Illumination, float Distance);

	UFUNCTION()
		void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	UFUNCTION(BlueprintCallable)
		void SendTeamRequest(ALostOneAI* OtherLostOne, APawn* Target, ALostOneAI* Leader, float DangerLevel);
	UFUNCTION(BlueprintCallable)
		bool ReceiveTeamRequest(ALostOneAI* OtherLostOne, APawn* Target, ALostOneAI* Leader, float DangerLevel);
	UFUNCTION(BlueprintCallable)
		void ReceiveTeamResponse(ALostOneAI* OtherlostOne, bool Affirmative);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PerceptionComp)
		class UAIPerceptionComponent* PerceptionComp;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PerceptionUpdate)
		float SightSusPerSecond = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightAngle = 60;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightDistance = 700;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float LoseSightDistance = SightDistance + 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		float SightMaxAge = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AISight)
		class UAISenseConfig_Sight *SightConfig;
private:
	AActor* Target;
	bool HasSight;
	bool HasInput;
	float InputPriority;
	float Suspicion = 0;
	FVector LastInput;
	void AddSus(float Suspicion);

};
