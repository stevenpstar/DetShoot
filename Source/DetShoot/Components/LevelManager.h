// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelManager.generated.h"

class ALevelPosition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DETSHOOT_API ULevelManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULevelManager();

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
	TArray<ALevelPosition*> LevelPositions;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
