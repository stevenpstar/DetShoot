// Fill out your copyright notice in the Description page of Project Settings.


#include "IntLevelButton.h"

#include "Components/BoxComponent.h"
#include "DetShoot/Components/LevelManager.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AIntLevelButton::AIntLevelButton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TypeLabel = "LevelButton";

	TargetVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Target Volume"));
	TargetVolume->SetupAttachment(RootComponent);

	IntMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Int Mesh"));
	IntMesh->SetupAttachment(TargetVolume);
}

void AIntLevelButton::Trigger()
{
	Super::Trigger();
	if (Triggered) return;
	if (LevelManager == nullptr || TargetPosition == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Level Manager or Target Position not defined"));
		return;
	}

	if (HasAuthority())
	{
		LevelManager->Multi_LoadInstancedLevel(TargetPosition);
	} else
	{
		LevelManager->Server_LoadInstancedLevel(TargetPosition);
	}
	Triggered = true;
}

// Called when the game starts or when spawned
void AIntLevelButton::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AIntLevelButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AIntLevelButton::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Here we list the variables we want to replicate
	DOREPLIFETIME(AIntLevelButton, Triggered);
}