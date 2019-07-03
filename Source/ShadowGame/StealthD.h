// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPawn.h"
#include "StealthD.generated.h"
UCLASS()
class SHADOWGAME_API AStealthD : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AStealthD();

	class AMyPawn* Player;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UDirectionalLightComponent* Source;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	bool HitLast = false;
	AMyPawn::Visibility value;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
