// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelManager.h"

#include "DetShoot/Level/LevelPosition.h"
#include "Engine/LevelStreamingDynamic.h"


// Sets default values for this component's properties
ULevelManager::ULevelManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULevelManager::Multi_LoadInstancedLevel_Implementation(ALevelPosition* Position)
{
	if (!Position)
	{
		UE_LOG(LogTemp, Error, TEXT("Level Position does not exist"));
		return;
	}
	if (!Position->Empty)
	{
		UE_LOG(LogTemp, Error, TEXT("Level Position already full"));
		return;
	}
	if (LevelNames.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No Level Names in Level Manager List"));
		return;
	}
	// For now we will just grab something out of the level name list
	const uint32 LevelNameIndex = FMath::RandRange(0, LevelNames.Num() - 1);
	const FString LevelName = LevelNames[LevelNameIndex];
	const FString LevelNameOverride = LevelName + Position->UniqueLevelNameTag;
	const FVector Loc = Position->GetActorLocation();
	bool success;
	const ULevelStreamingDynamic* LoadedLevel = ULevelStreamingDynamic::LoadLevelInstance(
		GetWorld(),
		LevelName,
		Loc,
		FRotator(0.f, 0.f, 0.f),
		success,
		LevelNameOverride);
	if (!success)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not load level instance"));
	} else
	{
		// Do something with this?
		ULevel* Level = LoadedLevel->GetLoadedLevel();
	}
	// Probably add to a list of loaded levels, to unload on floor completion
}

void ULevelManager::Server_LoadInstancedLevel_Implementation(ALevelPosition* Position)
{
	Multi_LoadInstancedLevel(Position);
}

// Called when the game starts
void ULevelManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void ULevelManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

