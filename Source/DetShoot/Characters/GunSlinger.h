// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interactive/CoverZone.h"
#include "GunSlinger.generated.h"

class AAIController;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;

class UInputMappingContext;
class UInputAction;
class UShootingComponent;

UCLASS()
class DETSHOOT_API AGunSlinger : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGunSlinger();

	UPROPERTY(EditAnywhere)
	AAIController* AIController;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	void SetActiveCoverZone(ACoverZone* Zone, bool ClearZone);
	void SetOverlappedCoverZone(ACoverZone* Zone, bool ClearZone);
	
private:

	// Movement related flags

	bool MovementTakeOver = false;



	// Cover related
	
	TOptional<ACoverZone*> ActiveCoverZone;
	TOptional<ACoverZone*> OverlappedCoverZone;
	FVector TargetLocation;

	// Components
	
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	UShootingComponent* ShootingComponent;
	// UI

	UPROPERTY(EditAnywhere)
	UUserWidget* CrosshairWidget;

	// Input
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputMappingContext* DefaultContext;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* AimAction;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess="true"))
	UInputAction* CoverAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void StopAiming(const FInputActionValue& Value);
	void TakeCover();

	bool IsAiming = false;
	void UpdateAim();
};
