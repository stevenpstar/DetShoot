// Fill out your copyright notice in the Description page of Project Settings.


#include "CoverZone.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "DetShoot/Characters/GunSlinger.h"


// Sets default values
ACoverZone::ACoverZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);
	
	CoverArea = CreateDefaultSubobject<UBoxComponent>(TEXT("Cover Area"));
	CoverArea->SetupAttachment(Root);

	CoverPath = CreateDefaultSubobject<USplineComponent>(TEXT("Cover Path"));
	CoverPath->SetupAttachment(Root);
	
	LeftBound = CreateDefaultSubobject<USceneComponent>(TEXT("Left Bound"));
	RightBound = CreateDefaultSubobject<USceneComponent>(TEXT("Right Bound"));
}

// Called when the game starts or when spawned
void ACoverZone::BeginPlay()
{
	Super::BeginPlay();

	// Adding overlap delegates
	CoverArea->OnComponentBeginOverlap.AddDynamic(this, &ACoverZone::OverlapBegin);
	CoverArea->OnComponentEndOverlap.AddDynamic(this, &ACoverZone::OverlapEnd);
}

void ACoverZone::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AGunSlinger* Player = Cast<AGunSlinger>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlap begin player"));
		CoverPath->bDrawDebug = true;
		const float SplineDist = CoverPath->GetDistanceAlongSplineAtLocation(
			Player->GetActorLocation(),
			ESplineCoordinateSpace::World);

		Player->SetOverlappedCoverZone(this, false);
		UE_LOG(LogTemp, Warning, TEXT("Spline Dist at: %f"), SplineDist);
	}
}

void ACoverZone::OverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AGunSlinger* Player = Cast<AGunSlinger>(OtherActor))
	{
		Player->SetOverlappedCoverZone(this, true);
	}
}

// Called every frame
void ACoverZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

USplineComponent* ACoverZone::GetCoverPath() const
{
	return CoverPath;
}

