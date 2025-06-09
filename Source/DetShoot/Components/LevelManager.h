// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelManager.generated.h"

class ALevelPosition;

USTRUCT()
struct FLevelPositionRow
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<ALevelPosition*> PosRow;

	ALevelPosition* operator[] (int32 i)
	{
		return PosRow[i];
	}
};

UCLASS()
class DETSHOOT_API ALevelManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ALevelManager();

	UPROPERTY(EditAnywhere)
	TArray<FString> LevelNames;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_LoadInstancedLevel(ALevelPosition* Position);

	UFUNCTION(Server, Reliable)
	void Server_LoadInstancedLevel(ALevelPosition* Position);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<FLevelPositionRow> LevelPositions;

	UFUNCTION()
	void OnLevelInstanceLoaded(ULevel* Level, UWorld* World);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	bool LoadingLevel = false;
};
