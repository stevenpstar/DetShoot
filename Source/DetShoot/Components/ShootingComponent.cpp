// Fill out your copyright notice in the Description page of Project Settings.

// TODO: This class should maybe go on the weapon
#include "ShootingComponent.h"
#include "CollisionQueryParams.h"


// Sets default values for this component's properties
UShootingComponent::UShootingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UShootingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShootingComponent::Shoot(FHitResult& HitOut)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	const AController* OwnerController = OwnerPawn->GetController();
	if (!OwnerController) return;
	// TODO: Line of sight/Aim of sight(? what) May be calculated from the gun model in future, instead of from camera.
	FVector Location;
	FRotator Rotation;
	OwnerController->GetPlayerViewPoint(Location, Rotation);
	FVector EndPoint = Location + Rotation.Vector() * 20000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	GetWorld()->LineTraceSingleByChannel(
		HitOut,
		Location,
		EndPoint,
		ECC_GameTraceChannel1);

	if (HitOut.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit!"));
		DrawDebugPoint(
			GetWorld(),
			HitOut.ImpactPoint,
			4.f,
			FColor::Red,
			false,
			2.f);
	}
}

// Called every frame
void UShootingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

