// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSpawner.h"

#include "PropSpawnPoint.h"
#include "DetShoot/Components/LevelManager.h"


// Sets default values
ALevelSpawner::ALevelSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ALevelSpawner::SpawnProps()
{
	for (auto SpawnPoint : PropSpawnPoints)
	{
		SpawnPoint->SpawnProp();
	}
}

void ALevelSpawner::SpawnLevelButtons(ALevelPosition* Position, TArray<FLevelPositionRow> &LevelPositions, ALevelManager* Manager)
{
	UE_LOG(LogTemp, Display, TEXT("Beginning Level Button Spawning"));
	for (auto ButtonSpawnPoint : LevelButtons)
	{
		
		UE_LOG(LogTemp, Display, TEXT("Looping Level Button Spawning"));
		ButtonSpawnPoint->SpawnLevelButtons(Position, LevelPositions, Manager);
	}
}

// Called when the game starts or when spawned
void ALevelSpawner::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALevelSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

