// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactive.h"


// Sets default values
AInteractive::AInteractive()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AInteractive::Trigger()
{
}

// Called when the game starts or when spawned
void AInteractive::BeginPlay()
{
	Super::BeginPlay();
	if (!TargetVolume)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target Volume has not been defined for interactive object"));
	}
}

// Called every frame
void AInteractive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

