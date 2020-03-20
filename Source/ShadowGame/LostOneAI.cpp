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
#include "PlayerPawn.h"

ALostOneAI::ALostOneAI() {
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
	//creating perception components
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	//end perception component creation

	//sight config
	SightConfig->PeripheralVisionAngleDegrees = SightAngle;
	SightConfig->SightRadius = SightDistance;
	SightConfig->LoseSightRadius = LoseSightDistance;
	SightConfig->SetMaxAge(SightMaxAge);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	//end sight config

	SetPerceptionComponent(*PerceptionComp);

	PerceptionComp->ConfigureSense(*SightConfig);

}
void ALostOneAI::BeginPlay() {
	Super::BeginPlay();
	if (PerceptionComp) {
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ALostOneAI::OnTargetPerceptionUpdated);
		//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "All Systems Go");
	}
}
void ALostOneAI::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);
	ALostOne* Character = Cast<ALostOne>(GetPawn());
	if (Character != nullptr) {
		//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, "MOVE BITCH");
		MoveToActor(Character->Destination, 50, true, true, true, 0, true);
	}
	if (Cast<APlayerPawn>(Target) != nullptr) {
		APlayerPawn* Player = Cast<APlayerPawn>(Target);
		if (HasSight) {
			AddSus(VisStimStrengthLight(Player->MyVis.GroundVis * DeltaTime * SightSusPerSecond, (Player->GetActorLocation() - GetPawn()->GetActorLocation()).Size()));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Magenta, "Suspicion: " + FString::SanitizeFloat(Suspicion));
		}
	}
}
void ALostOneAI::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
}
FRotator ALostOneAI::GetControlRotation() const{
	if (GetPawn() == nullptr) {
		return FRotator(0, 0, 0);
	}
	return GetPawn()->GetActorRotation();
}

float ALostOneAI::VisStimStrengthLight(float Illumination, float Distance)
{
	return FMath::Sqrt(Illumination) / Distance * 100;
}

void ALostOneAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>()) {
		APlayerPawn* PlayerPawn = Cast<APlayerPawn>(Actor);
		if (Stimulus.WasSuccessfullySensed()) {
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, "Saw You");
			//Target = Actor;
			if (Cast<APlayerPawn>(Actor) != nullptr && Cast<APlayerController>(PlayerPawn->GetController()) != nullptr) {
				HasSight = true;
				Target = PlayerPawn;
			}
		}
		else {
			HasSight = false;
		}
	}
}

void ALostOneAI::AddSus(float sus) {
	Suspicion += sus;
}
void ALostOneAI::SendTeamRequest(ALostOneAI* OtherLostOne, APawn* Target, ALostOneAI* Leader, float DangerLevel)
{

}

bool ALostOneAI::ReceiveTeamRequest(ALostOneAI* OtherLostOne, APawn* Target, ALostOneAI* Leader, float DangerLevel)
{
	return false;
}

void ALostOneAI::ReceiveTeamResponse(ALostOneAI* OtherlostOne, bool Affirmative)
{

}

