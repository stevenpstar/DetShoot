// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelPosition.h"


// Sets default values
ALevelPosition::ALevelPosition()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VisualRep = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Representation"));
	SetRootComponent(VisualRep);
}

// Called when the game starts or when spawned
void ALevelPosition::BeginPlay()
{
	Super::BeginPlay();
	if (VisualRep)
	{
		VisualRep->SetVisibility(false);
	}
}

// Called every frame
void ALevelPosition::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelPosition::AddLevelToPosition(const FString Name)
{
	LevelName = Name + UniqueLevelNameTag;
	Empty = false;
}

