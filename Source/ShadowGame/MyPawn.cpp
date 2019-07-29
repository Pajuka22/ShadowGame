// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPawn.h"
#include "Components/InputComponent.h"
#include "Runtime/HeadMountedDisplay/Public/MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "CustomMovement.h"
#include "Runtime/Engine/Classes/GameFramework/FloatingPawnMovement.h"
#include "ConstructorHelpers.h"
#include "Runtime/Core/Public/Math/Vector.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Core/Public/Math/TransformNonVectorized.h"

// Sets default values
AMyPawn::AMyPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UCapsuleComponent* Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root Comp"));
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCapsuleHalfHeight(normalHeight);
	Capsule->SetCapsuleRadius(NormalRadius);
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AMyPawn::RootCollision);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AMyPawn::RootCollisionExit);
	Capsule->OnComponentHit.AddDynamic(this, &AMyPawn::RootHit);
	RootComponent = Capsule;
	//Capsule->SetActive(false);

	VisibleComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	VisibleComponent->SetupAttachment(RootComponent);

	MyCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	MyCamera->SetupAttachment(RootComponent);
	MyCamera->SetRelativeLocation(FVector(0, 0, 90));
	MyCamera->SetRelativeRotation(FRotator(0, 0, 0));


	MovementComp = CreateDefaultSubobject<UCustomMovement>(TEXT("Movement"));
	MovementComp->UpdatedComponent = RootComponent;


	ForwardVel = 0;
	RightVel = 0;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	RootComponent->SetRelativeRotation(FRotator(0, 0, 0));
	SetActorRotation(FRotator(0, 0, 0));
	cameraRot = 0;

	startHeight = normalHeight;
	endHeight = normalHeight;
	currentHeight = startHeight;

	MyVis.Vis = 0;
	MyVis.GroundVis = 0;

	JumpVect = FVector(0, 0, 750);
}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (!CurrentVelocity.IsZero()) {
		SetActorLocation(GetActorLocation() + CurrentVelocity * DeltaTime);
	}*/
	MovementComp->GroundNum = Grounded;
	JumpVect = JumpSpeed * RootComponent->GetUpVector();
	MovementComp->Velocity = FVector();
	SetActorRotation(GetActorRotation() + FRotator(0, 0, 0));
	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + 100 * RootComponent->GetUpVector(), FColor::Red, false, 5.f, 0, 1);
	if (bBufferSprint) {
		Sprint();
	}
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(FMath::Sqrt(MyVis.GroundVis)));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::SanitizeFloat(FMath::Sqrt(MyVis.Vis)));
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Orange, CheckGrounded() ? "true" : "false");

	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr && currentHeight != endHeight) {
		MovementComp->AddInputVector(RootComponent->GetUpVector() * addHeight);
		currentHeight += addHeight * DeltaTime;
		if (currentHeight == endHeight || addHeight > 0 ? currentHeight + addHeight * DeltaTime > endHeight : currentHeight + addHeight * DeltaTime < endHeight) {
			MovementComp->AddInputVector(RootComponent->GetUpVector() * (endHeight - currentHeight));
			currentHeight = endHeight;
			addHeight = 0;
		}
		MyCamera->SetRelativeLocation(FVector(0, 0, currentHeight));
		Capsule->SetCapsuleRadius(NormalRadius);
		if (Capsule->GetScaledCapsuleRadius() > currentHeight) {
			Capsule->SetCapsuleRadius(currentHeight);
		}
		Capsule->SetCapsuleHalfHeight(currentHeight);
		RootComponent = Capsule;
	}
	if (Cast<UCustomMovement>(MovementComp) && CheckGrounded()) {
		if (ShadowSneak) {
			MovementComp->MovementSpeed = SneakSpeed;
		}
		else {
			if (bCrouch) {
				MovementComp->MovementSpeed = CrouchSpeed;
			}
			else {
				if (bSprint) {
					MovementComp->MovementSpeed = SprintSpeed;
				}
				else {
					MovementComp->MovementSpeed = NormalSpeed;
				}
			}
		}
	}
	if (ShadowSneak) {
		FHitResult hitResultTrace;
		FCollisionQueryParams queryParams;
		
		queryParams.AddIgnoredActor(this);

		FVector under;
		FVector Start = GetActorLocation();
		FVector End = Start + GetVelocity();
		if (GetWorld()->LineTraceSingleByChannel(hitResultTrace, GetActorLocation(), GetActorLocation() - RootComponent->GetUpVector() * 100,
			ECC_Visibility, queryParams))
		{
			if (hitResultTrace.GetComponent() != nullptr) {
				if (hitResultTrace.Component->Mobility != EComponentMobility::Movable){
					under = hitResultTrace.ImpactPoint;
					FVector newUp = hitResultTrace.ImpactNormal;
					FVector newForward = FVector::CrossProduct(RootComponent->GetRightVector(), newUp);
					FVector newRight = FVector::CrossProduct(newUp, newForward);
					//Build the new transform!
					FTransform newTransform = FTransform(newForward, newRight, newUp, GetActorLocation());
					RootComponent->SetWorldRotation(FMath::Lerp(RootComponent->GetComponentRotation().Quaternion(), newTransform.GetRotation(), .08));
					UCapsuleComponent* capsule = Cast<UCapsuleComponent>(RootComponent);
					if (capsule) {
						//RootComponent->SetWorldLocation(under + newUp * capsule->GetScaledCapsuleHalfHeight);
					}
					else {
						RootComponent->SetWorldLocation(under + newUp * 100);
					}
				}
			}
			else {
				under = hitResultTrace.ImpactPoint;
				FVector newUp = FVector(0, 0, 1);
				FVector newForward = FVector::CrossProduct(RootComponent->GetRightVector(), newUp);
				FVector newRight = FVector::CrossProduct(newUp, newForward);
				//Build the new transform!
				FTransform newTransform = FTransform(newForward, newRight, newUp, GetActorLocation());
				RootComponent->SetWorldRotation(FMath::Lerp(RootComponent->GetComponentRotation().Quaternion(), newTransform.GetRotation(), .05));
			}
		}
		if (!CheckGrounded()) {
			Start = GetActorLocation() - Capsule->GetScaledCapsuleHalfHeight() * Capsule->GetUpVector();
			End = Start - Capsule->GetUpVector() * MovementComp->StepHeight;
			if (!GetWorld()->LineTraceSingleByChannel(hitResultTrace, Start, End, ECC_Visibility, queryParams)) {
				StartEndSneak();
			}
			
		}
	}
	else {
		if (RootComponent->GetUpVector() != FVector(0, 0, 1)) {
			FVector newUp = FVector(0, 0, 1);
			FVector newForward = FVector::CrossProduct(RootComponent->GetRightVector(), newUp);
			FVector newRight = FVector::CrossProduct(newUp, newForward);
			//Build the new transform!
			FTransform newTransform = FTransform(newForward, newRight, newUp, GetActorLocation());
			RootComponent->SetWorldRotation(FMath::Lerp(RootComponent->GetComponentRotation().Quaternion(), newTransform.GetRotation(), .05));
		}
	}
	Grounded = 0;
	FloorAngle = 2 * PI;
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyPawn::LookUpAtRate);

	PlayerInputComponent->BindAction("Sneaky", IE_Pressed, this, &AMyPawn::StartEndSneak);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyPawn::Jump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyPawn::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyPawn::StopSprinting);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyPawn::CrouchControl);
}
UPawnMovementComponent* AMyPawn::GetMovementComponent() const
{
	return MovementComp;
}
void AMyPawn::MoveForward(float val) {
	if (MovementComp && MovementComp->UpdatedComponent == RootComponent) {
		MovementComp->LateralVel += (GetActorForwardVector() * val * MovementComp->MovementSpeed);
	}
}
void AMyPawn::MoveRight(float val) {
	if (MovementComp && MovementComp->UpdatedComponent == RootComponent) {
		MovementComp->LateralVel += (GetActorRightVector() * val * MovementComp->MovementSpeed);
	}
}
void AMyPawn::TurnAtRate(float rate) {
	FVector newRight = RootComponent->GetRightVector().RotateAngleAxis(5 * rate, RootComponent->GetUpVector());
	FVector newForward = RootComponent->GetForwardVector().RotateAngleAxis(5 * rate, RootComponent->GetUpVector());
	RootComponent->SetWorldTransform(FTransform(newForward, newRight, RootComponent->GetUpVector(), GetActorLocation()));
}
void AMyPawn::LookUpAtRate(float rate) {
	//MyCamera->SetRelativeRotation(MyCamera->RelativeRotation + FRotator(-rate, 0, 0));
	float addrot = 5 * rate;

	if (FMath::Acos(FVector::DotProduct(MyCamera->GetForwardVector(), RootComponent->GetUpVector())) < FMath::Abs(5 * rate * PI / 180) && addrot < 0) {
		addrot = FMath::Acos(FVector::DotProduct(MyCamera->GetForwardVector(), RootComponent->GetUpVector()));
	}
	if (FMath::Acos(FVector::DotProduct(MyCamera->GetForwardVector(), RootComponent->GetUpVector() * -1)) < FMath::Abs(5 * rate * PI / 180) && addrot > 0) {
		addrot = FMath::Acos(FVector::DotProduct(MyCamera->GetForwardVector(), -RootComponent->GetUpVector()));
	}
	//start standard working.
	FVector newUp = MyCamera->GetUpVector().RotateAngleAxis(addrot, MyCamera->GetRightVector());
	FVector newForward = MyCamera->GetForwardVector().RotateAngleAxis(addrot, MyCamera->GetRightVector());
	MyCamera->SetWorldTransform(FTransform(newForward, MyCamera->GetRightVector(), newUp, MyCamera->GetComponentLocation()));
	//end standard working


	//MyCamera->SetRelativeRotation(FRotator(FMath::ClampAngle(MyCamera->RelativeRotation.Pitch - 5 * rate, -90, 90), MyCamera->RelativeRotation.Yaw, MyCamera->RelativeRotation.Roll));
}
void AMyPawn::Sprint() {
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	if (CheckGrounded()) {
		startHeight = currentHeight;
		endHeight = normalHeight;

		GetWorld()->LineTraceSingleByChannel(outHit, GetActorLocation(),
			GetActorLocation() + RootComponent->GetUpVector() * (normalHeight + (normalHeight - currentHeight)), ECC_Visibility, params);
		if (!outHit.bBlockingHit) {
			bBufferSprint = false;
			bSprint = true;
			GetAddHeight();
			if (ShadowSneak) {
				StartEndSneak();
			}
			if (bCrouch) {
				StopCrouching();
			}
		}
		else {
			endHeight = currentHeight;
			bBufferSprint = true;
		}
	}
	else {
		bBufferSprint = true;
	}
}
void AMyPawn::StopSprinting() {
	bSprint = false;
	bBufferSprint = false;
}
void AMyPawn::CrouchControl() {
	if (bCrouch) {
		StopCrouching();
	}
	else {
		Crouch();
	}
}
void AMyPawn::Crouch() {
	StopSprinting();
	bCrouch = true;
	startHeight = currentHeight;
	endHeight = crouchHeight;
	ShadowSneak = false;
	GetAddHeight();
}
void AMyPawn::StopCrouching() {
	if (ShadowSneak) {
		StartEndSneak();
	}
	startHeight = currentHeight;
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	bool CanUnCrouch = true;
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	GetWorld()->LineTraceSingleByChannel(outHit, GetActorLocation(),
		GetActorLocation() + RootComponent->GetUpVector() * (normalHeight + (normalHeight - currentHeight)), ECC_Visibility, params);
	if (!outHit.bBlockingHit || ShadowSneak) {
		endHeight = ShadowSneak ? sneakHeight : normalHeight;
		GetAddHeight();
		bCrouch = false;
	}
}
void AMyPawn::Jump() {
	MovementComp->Jump();
	if (ShadowSneak) {
		StartEndSneak();
	}
	Jumping = true;
	EndJump = false;
}
void AMyPawn::StopJumping() {

}
void AMyPawn::StartEndSneak() {
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	ShadowSneak = !ShadowSneak;
	if (!CheckGrounded()) {
		ShadowSneak = false;
	}
	bCrouch = false;
	startHeight = currentHeight;
	if (ShadowSneak) {
		if (FMath::Sqrt(MyVis.GroundVis) <= SneakThreshold) {
			endHeight = sneakHeight;
			StopSprinting();
		}
		else {
			ShadowSneak = false;
		}
	}
	else {
		endHeight = bCrouch ? crouchHeight : normalHeight;
		GetWorld()->LineTraceSingleByChannel(outHit, GetActorLocation(),
			GetActorLocation() + RootComponent->GetUpVector() * (normalHeight + (endHeight - currentHeight)), ECC_Visibility, params);
		endHeight = outHit.bBlockingHit ? currentHeight : endHeight;
	}
	GetAddHeight();
	MovementComp->Shadow = ShadowSneak;
}
void AMyPawn::RootCollision(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(HitComp);
	if (Capsule) {
		if ((SweepResult.ImpactPoint - (GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere())).DistanceInDirection(-Capsule->GetUpVector()) 
			>= FMath::Sin(25*PI/180) * Capsule->GetScaledCapsuleRadius()) {

		}
	}
	/*
	if (Capsule) {
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Green, RootComponent->GetComponentLocation().ToString());
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Green, FVector(SweepResult.ImpactPoint.Y, SweepResult.ImpactPoint.Z, SweepResult.ImpactPoint.X).ToString());
		GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Red, SweepResult.ImpactNormal.ToString());
		DrawDebugLine(GetWorld(), GetActorLocation(), FVector(SweepResult.Location.Y, SweepResult.Location.Z, SweepResult.Location.X), FColor::Green, false, 1, 0, 1);
	}*/
}

void AMyPawn::RootCollisionExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AMyPawn::RootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& SweepResult)
{
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(HitComponent);
	if (Capsule) {
		if ((SweepResult.ImpactPoint - (GetActorLocation() - Capsule->GetUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere())).DistanceInDirection(-Capsule->GetUpVector())
			>= FMath::Sin(25 * PI / 180) * Capsule->GetScaledCapsuleRadius()) {
			DrawDebugLine(GetWorld(), SweepResult.ImpactPoint + 100 * SweepResult.ImpactNormal, SweepResult.ImpactPoint, FColor::Green, false, 1, 0, 1);
			Grounded++;
			FVector ThisNorm = -SweepResult.ImpactNormal;
			GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Red, FString::SanitizeFloat(ThisNorm.RadiansToVector(RootComponent->GetUpVector()) * 180 / PI));
			if (ThisNorm.RadiansToVector(-HitComponent->GetUpVector()) < FloorAngle) {
				FloorNormal = ThisNorm;
				FloorAngle = ThisNorm.RadiansToVector(-HitComponent->GetUpVector());
			}
			if ((-ThisNorm).RadiansToVector(RootComponent->GetUpVector()) >= FMath::DegreesToRadians(85)) {
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Yellow, "FUCK YOU");
				MovementComp->AddInputVector(RootComponent->GetUpVector() * 100);
			}
		}
	}
}

bool AMyPawn::CheckGrounded() {
	if (!MovementComp->CheckGrounded()) {
		if (Jumping) {
			EndJump = true;
		}
	}
	else {
		if (EndJump) {
			MovementComp->downVel = FVector(0, 0, 0);
		}
	}

	return ShadowSneak ? MovementComp->CheckGrounded() : Grounded >= 1;
	/*UCapsuleComponent* a = Cast<UCapsuleComponent>(RootComponent);
	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() - RootComponent->GetUpVector() * (a->GetScaledCapsuleHalfHeight() * 1.25 + 5);
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1);
	FCollisionQueryParams Params;
	FHitResult Hit;
	Params.AddIgnoredActor(this);
	bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	return Hit.bBlockingHit;*/
}

void AMyPawn::GetAddHeight() {
	addHeight = (endHeight - startHeight) / HeightInterpTime / (FMath::Abs(startHeight - endHeight)/(normalHeight - crouchHeight));
	UCapsuleComponent* A = Cast<UCapsuleComponent>(RootComponent);
}

AMyPawn::Visibility AMyPawn::PStealth(FVector location, float Attenuation, float candelas) {
	float mult = 0;
	Visibility ReturnVis;
	FHitResult outHit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	FVector Start;
	FVector End;
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr && (GetActorLocation() - location).Size() <= Attenuation) {
		float capsuleHeight = Capsule->GetScaledCapsuleHalfHeight();
		float capsuleRadius = Capsule->GetScaledCapsuleRadius();
		for (float f = -capsuleHeight; f <= capsuleHeight; f += capsuleHeight) {
			Start = GetActorLocation() + RootComponent->GetUpVector() * f;
			End = location;
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
			bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, End, Start, ECC_Visibility, CollisionParams);
			if (!outHit.bBlockingHit) {
				mult += 0.2;
				if (f == -capsuleHeight) {
					ReturnVis.GroundVis = candelas * 10000/ (FMath::Pow((Start - End).Size(), 2));
				}
			}
		}
		for (float f = -capsuleRadius; f <= capsuleRadius; f += 2 * capsuleRadius) {
			Start = RootComponent->GetRightVector() * f + GetActorLocation();
			End = location;
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
			bool isHit = GetWorld()->LineTraceSingleByChannel(outHit, Start, End, ECC_Visibility, CollisionParams);
			if (!outHit.bBlockingHit) {
				mult += 0.2;
			}
			//DrawDebugLine(GetWorld(), outHit.ImpactPoint, End, FColor::Green, false, 1, 0, 1);
		}
		//for the top bottom, left, right, and center of the player, do a line check. If there's something in the way subtract 0.2 from the multiplier cuz 5 points
		//if there's nothing in the don't do anything.
	}
	else {
		mult = 0;
		ReturnVis.GroundVis = 0;
	}
	ReturnVis.Vis = (2 * PI * (1 - FMath::Cos(PI)) * candelas * 10000) / (4 * PI * FMath::Pow((Start - End).Size(), 2)) * mult;
	return ReturnVis;
}
AMyPawn::Visibility AMyPawn::SStealth(FVector Spotlight, float inner, float outer, float Attenuation, FVector SpotAngle, float candelas) {
	Visibility ReturnVis;
	float mult = 0;
	float angle;
	FVector End;
	FVector Start = Spotlight;
	FHitResult outHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule != nullptr) {
		float halfHeight = Capsule->GetScaledCapsuleHalfHeight();
		float radius = Capsule->GetScaledCapsuleRadius();
		for (float f = -radius; f < radius; f += radius) {
			End = GetActorLocation() + GetActorRightVector() * f;
			angle = FMath::Acos(FVector::DotProduct(SpotAngle, (End - Spotlight)) / (SpotAngle.Size() * (End - Spotlight).Size())) * 180 / PI;
			GetWorld()->LineTraceSingleByChannel(outHit, Spotlight, End, ECC_Visibility, params);
			if (!outHit.bBlockingHit && angle <= outer && (End - Spotlight).Size() < Attenuation) {
				if (angle <= inner) {
					mult += 0.2;
				}
				else {
					mult += (outer - angle) / (outer - inner) * 0.2;
				}
			}
		}
		for (float f = -halfHeight; f < radius; f += 2 * halfHeight) {
			End = GetActorLocation() + GetActorUpVector() * f;
			angle = FMath::Acos(FVector::DotProduct(SpotAngle, (End - Spotlight)) / (SpotAngle.Size() * (End - Spotlight).Size())) * 180 / PI;
			GetWorld()->LineTraceSingleByChannel(outHit, Spotlight, End, ECC_Visibility, params);
			if (!outHit.bBlockingHit && angle <= outer && (End - Spotlight).Size() < Attenuation) {
				if (angle <= inner) {
					mult += 0.2;
				}
				else {
					mult += (outer - angle) / (outer - inner) * 0.2;
				}
				if (f == -halfHeight) {
					ReturnVis.GroundVis = (angle <= inner ? 1 : (outer - angle) / (outer - inner)) *
						(2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000)/FMath::Pow((Start - End).Size(), 2);
				}
			}
		}
		ReturnVis.Vis = mult * (2 * PI * FMath::Cos(inner * PI / 180) * candelas * 10000) / FMath::Pow((Start - End).Size(), 2);
	}
	else {
		ReturnVis.Vis = 0;
		ReturnVis.GroundVis = 0;
	}
	return ReturnVis;
}
AMyPawn::Visibility AMyPawn::DStealth(FVector Direction, float intensity, float length) {
	Visibility ReturnVis;
	float mult = 0;
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	FVector End;
	FVector Start;
	FHitResult OutHit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	if (Capsule != nullptr) {
		float Radius = Capsule->GetScaledCapsuleRadius();
		float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		for (float i = -Radius; i <= Radius; i += Radius) {
			Start = GetActorLocation() + RootComponent->GetRightVector() * i;
			End = Start - Direction * length;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
			if (!OutHit.bBlockingHit) {
				mult += 0.2;
			}
		}
		for (float j = -HalfHeight; j <= HalfHeight; j += 2 * HalfHeight) {
			Start = GetActorLocation() + RootComponent->GetUpVector() * j;
			End = Start - Direction * length;
			GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, params);
			if (!OutHit.bBlockingHit) {
				mult += 0.2;
				if (j == -HalfHeight) {
					ReturnVis.GroundVis = FMath::Square(intensity);
				}
			}
		}
	}
	ReturnVis.Vis = FMath::Square(mult * intensity);
	return ReturnVis;
}
void AMyPawn::AddVis(Visibility vis) {
	MyVis.Vis += vis.Vis;
	MyVis.GroundVis += vis.GroundVis;
}
void AMyPawn::SubVis(Visibility vis) {
	MyVis.Vis -= vis.Vis;
	MyVis.GroundVis -= vis.GroundVis;
}
float AMyPawn::GetCapsuleVisibleArea() {
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule) {
		return 2 * Capsule->GetScaledCapsuleRadius()* Capsule->GetScaledCapsuleHalfHeight() + PI * FMath::Pow(Capsule->GetScaledCapsuleRadius(), 2);
	}
	return 0;
}
