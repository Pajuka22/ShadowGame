// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPawn.h"
#include "StealthP.generated.h"

UCLASS()
class SHADOWGAME_API AStealthP : public AActor
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		class UPointLightComponent* Source;

public:
	// Sets default values for this actor's properties
	AStealthP();

	class AMyPawn* Player;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool HitLast = false;
	AMyPawn::Visibility value;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};