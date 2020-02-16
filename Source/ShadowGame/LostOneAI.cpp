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
	//PerceptionComponent = GetPerceptionComponent();
	//GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ALostOneAI::OnTargetPerceptionUpdated);

	//creating perception components
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	//end perception component creation

	//sight config
	SightConfig->PeripheralVisionAngleDegrees = SightAngle;
	SightConfig->SightRadius = SightDistance;
	SightConfig->LoseSightRadius = LoseSightDistance;
	SightConfig->SetMaxAge(SightMaxAge);
	//end sight config

}
void ALostOneAI::BeginPlay() {
	Super::BeginPlay();
}
void ALostOneAI::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);
	ALostOne* Character = Cast<ALostOne>(GetPawn());
	if (Character != nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, "MOVE BITCH");
		MoveToActor(Character->Destination, 50, true, true, true, 0, true);
	}
}
void ALostOneAI::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
}
FRotator ALostOneAI::GetControlRotation() const{
	return FRotator(0, 0, 0);
}

void ALostOneAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Green, "Saw You");
}

