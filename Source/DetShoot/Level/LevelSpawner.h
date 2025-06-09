// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSpawner.generated.h"

class ALevelManager;
class ALevelPosition;
class APropSpawnPoint;

struct FLevelPositionRow;

UCLASS()
class DETSHOOT_API ALevelSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelSpawner();

	void SpawnProps();
	void SpawnLevelButtons(ALevelPosition* Position, TArray<FLevelPositionRow> &LevelPositions, ALevelManager* Manager);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<APropSpawnPoint*> PropSpawnPoints;
	
	UPROPERTY(EditAnywhere)
	TArray<APropSpawnPoint*> LevelButtons;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
