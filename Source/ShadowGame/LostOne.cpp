// Fill out your copyright notice in the Description page of Project Settings.


#include "LostOne.h"

// Sets default values
ALostOne::ALostOne()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALostOne::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALostOne::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALostOne::AddSus(float Visibility) {

}

// Called to bind functionality to input
void ALostOne::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

