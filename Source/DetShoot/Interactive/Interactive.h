// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactive.generated.h"

class UBoxComponent;

UCLASS()
class DETSHOOT_API AInteractive : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractive();

	UPROPERTY(EditAnywhere)
	UBoxComponent* TargetVolume;

	UPROPERTY(EditAnywhere)
	FString TypeLabel = "None";

	virtual void Trigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

};
