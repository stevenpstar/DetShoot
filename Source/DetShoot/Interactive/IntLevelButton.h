// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactive.h"
#include "GameFramework/Actor.h"
#include "IntLevelButton.generated.h"

class ALevelPosition;
class ALevelManager;

UCLASS()
class DETSHOOT_API AIntLevelButton : public AInteractive
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIntLevelButton();

	virtual void Trigger() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* IntMesh;

	UPROPERTY(EditAnywhere)
	ALevelManager* LevelManager;

	UPROPERTY(EditAnywhere)
	ALevelPosition* TargetPosition;

private:
	UPROPERTY(Replicated)
	bool Triggered = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
