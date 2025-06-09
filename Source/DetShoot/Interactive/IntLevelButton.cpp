// Fill out your copyright notice in the Description page of Project Settings.


#include "IntLevelButton.h"


// Sets default values
AIntLevelButton::AIntLevelButton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AIntLevelButton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIntLevelButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

