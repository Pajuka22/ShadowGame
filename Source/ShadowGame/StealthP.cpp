// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthP.h"
#include "Components/PointLightComponent.h"
#include "Runtime/Engine/Classes/Engine/Scene.h"
#include "Engine/World.h"
#include "MyPawn.h"

// Sets default values
AStealthP::AStealthP()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Source = CreateDefaultSubobject<UPointLightComponent>(TEXT("Source"));
	Source->IntensityUnits = ELightUnits::Lumens;
	Source->Intensity = 5000;
}

// Called when the game starts or when spawned
void AStealthP::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AStealthP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AMyPawn* Player = Cast<AMyPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (HitLast) {
		Player->SubVis(value);
	}
	value = Player->PStealth(GetActorLocation(), Source->AttenuationRadius, Source->Intensity);
	HitLast = value.Vis >= 0;
	Player->AddVis(value);
}

