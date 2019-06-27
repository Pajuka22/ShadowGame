// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPawn.h"
#include "StealthS.generated.h"

UCLASS()
class SHADOWGAME_API AStealthS : public AActor
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere)
		class USpotLightComponent* Source;

public:
	// Sets default values for this actor's properties
	AStealthS();

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