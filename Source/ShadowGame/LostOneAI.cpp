// Fill out your copyright notice in the Description page of Project Settings.

#include "LostOneAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "LostOne.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Touch.h"
#include "TimerManager.h"

ALostOneAI::ALostOneAI() {
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	//sight config
	SightConfig->PeripheralVisionAngleDegrees = SightAngle;
	SightConfig->SightRadius = SightDistance;
	SightConfig->LoseSightRadius = LoseSightDistance;
	SightConfig->SetMaxAge(SightMaxAge);
	//end sight config

}
void ALostOneAI::BeginPlay() {

}
void ALostOneAI::Tick(float DeltaTime) {

}
void ALostOneAI::OnPossess(APawn* InPawn) {
	MoveToLocation(FVector(0, 0, 0), 50, true, false);
}
FRotator ALostOneAI::GetControlRotation() const{
	return FRotator(0, 0, 0);
}

void ALostOneAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{

}

