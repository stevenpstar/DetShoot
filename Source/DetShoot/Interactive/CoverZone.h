// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverZone.generated.h"

class USplineComponent;
class UBoxComponent;

UCLASS()
class DETSHOOT_API ACoverZone : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACoverZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OverlapBegin( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	USplineComponent* GetCoverPath() const;

private:

	UPROPERTY()
	USceneComponent* Root;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* CoverArea;

	UPROPERTY(EditAnywhere)
	USplineComponent* CoverPath;

	UPROPERTY(EditAnywhere)
	USceneComponent* LeftBound;

	UPROPERTY(EditAnywhere)
	USceneComponent* RightBound;
};
