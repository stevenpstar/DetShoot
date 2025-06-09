// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelManager.h"

#include "DetShoot/Level/LevelPosition.h"
#include "DetShoot/Level/LevelSpawner.h"
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
	const uint32 LevelNameIndex = FMath::RandRange(0, LevelNames.Num() - 1);
	const FString LevelName = LevelNames[LevelNameIndex];
	const FString LevelNameOverride = LevelName + Position->UniqueLevelNameTag;
	const FVector Loc = Position->GetActorLocation();
	bool success;
	ULevelStreamingDynamic::LoadLevelInstance(
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
		LoadingPosition = Position;
	}
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
				if (ALevelSpawner* Spawner = Cast<ALevelSpawner>(A))
				{
					UE_LOG(LogTemp, Warning, TEXT("We should get here"));
					Spawner->SpawnProps();
					Spawner->SpawnLevelButtons(LoadingPosition.GetValue(), LevelPositions, this);
				}
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

