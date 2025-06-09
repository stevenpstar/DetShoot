// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelPosition.generated.h"

UCLASS()
class DETSHOOT_API ALevelPosition : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelPosition();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// This is for the editor really
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* VisualRep;

	UPROPERTY(EditAnywhere)
	FString UniqueLevelNameTag;

	// This will be empty by default until a level is generated on this position
	FString LevelName = "";

	bool Empty = true;

	void AddLevelToPosition(const FString Name);
};
