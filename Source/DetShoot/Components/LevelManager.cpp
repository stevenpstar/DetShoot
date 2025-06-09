// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelManager.h"

#include "DetShoot/Level/LevelPosition.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"


// Sets default values for this component's properties
ALevelManager::ALevelManager()
{
	PrimaryActorTick.bCanEverTick = true;
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ALevelManager::OnLevelInstanceLoaded);
}

void ALevelManager::Multi_LoadInstancedLevel_Implementation(ALevelPosition* Position)
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
		LoadingLevel = true;
	}
	// Probably add to a list of loaded levels, to unload on floor completion
}

void ALevelManager::Server_LoadInstancedLevel_Implementation(ALevelPosition* Position)
{
	Multi_LoadInstancedLevel(Position);
}

// Called when the game starts
void ALevelManager::BeginPlay()
{
	Super::BeginPlay();
}

void ALevelManager::OnLevelInstanceLoaded(ULevel* Level, UWorld* World)
{
	// load level
	UE_LOG(LogTemp, Warning, TEXT("Level Added!"));
	if (Level && LoadingLevel)
	{
		for (auto A : Level->Actors)
		{
			if (A)
			{
				UE_LOG(LogTemp, Warning, TEXT("Actor: %s"), *A->GetActorNameOrLabel());
			}
		}
	}
	LoadingLevel = false;
}

// Called every frame
void ALevelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...
}

