// Fill out your copyright notice in the Description page of Project Settings.


#include "GunSlinger.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineComponent.h"
#include "DetShoot/Components/ShootingComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


// Sets default values
AGunSlinger::AGunSlinger()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ShootingComponent = CreateDefaultSubobject<UShootingComponent>(TEXT("Shooting Component"));

	AIController = CreateDefaultSubobject<AAIController>(TEXT("Ai Controller?"));
}

// Called when the game starts or when spawned
void AGunSlinger::BeginPlay()
{
	Super::BeginPlay();
	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->ConsoleCommand(TEXT("show Collision"));
		PlayerController->ConsoleCommand(TEXT("show Navigation"));
		PlayerController->ConsoleCommand(TEXT("show Splines"));
	}
}

// Called every frame
void AGunSlinger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Camera Update for Aiming
	UpdateAim();
	// Movement takeover logic (moving into cover for example), move to function or different component
	if (MovementTakeOver)
	{
		const FVector TargetPlayerZ = FVector(TargetLocation.X, TargetLocation.Y, GetActorLocation().Z);

		const float Distance = UKismetMathLibrary::Vector_Distance(GetActorLocation(), TargetPlayerZ);
		UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), Distance);
		if (Distance < 60.f)
		{
			MovementTakeOver = false;
		} 
	}
}

// Called to bind functionality to input, move to player controller eventually?
void AGunSlinger::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Custom functions
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGunSlinger::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGunSlinger::Look);
		EIC->BindAction(AimAction, ETriggerEvent::Triggered, this, &AGunSlinger::Aim);
		EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &AGunSlinger::StopAiming);
		EIC->BindAction(ShootAction, ETriggerEvent::Started, this, &AGunSlinger::Attack);
		EIC->BindAction(CoverAction, ETriggerEvent::Started, this, &AGunSlinger::TakeCover);

		// Character functions
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(LookAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

void AGunSlinger::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultContext, 0);
		}
	}
}

void AGunSlinger::SetActiveCoverZone(ACoverZone* Zone, bool ClearZone)
{
	if (!Zone) return;
	if (ClearZone)
	{
		UE_LOG(LogTemp, Display, TEXT("Clearing Active Zone"));
		ActiveCoverZone.Reset();
		MeshRotationTargetY = -90.f;
		SpringArmTargetY = 90.f;
		return;
	}
	MovementTakeOver = true;
	ActiveCoverZone = Zone;
	const float SplineDist = ActiveCoverZone.GetValue()->GetCoverPath()->GetDistanceAlongSplineAtLocation(
		GetActorLocation(),
		ESplineCoordinateSpace::World);
	
	TargetLocation = ActiveCoverZone.GetValue()->GetCoverPath()->GetLocationAtDistanceAlongSpline(
		SplineDist,
		ESplineCoordinateSpace::World);

    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller,
    	FVector(TargetLocation.X, TargetLocation.Y, GetActorLocation().Z));

	DrawDebugPoint(
		GetWorld(),
		FVector(TargetLocation.X, TargetLocation.Y, GetActorLocation().Z),
		4.f,
		FColor::Blue,
		true,
		1000);
	// Debug Rotation
	MeshRotationTargetY = 180.f;
}

void AGunSlinger::SetOverlappedCoverZone(ACoverZone* Zone, bool ClearZone)
{
	if (!Zone) return;
	if (ClearZone)
	{
		OverlappedCoverZone.Reset();

		return;
	}
	OverlappedCoverZone = Zone;
}

void AGunSlinger::Move(const FInputActionValue& Value)
{
	if (MovementTakeOver) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Moving in cover
	if (ActiveCoverZone.IsSet())
	{
		if (MovementVector.X < 0.f)
		{
			MeshRotationTargetY = 180.f;
		} else
		{
			MeshRotationTargetY = 0.f;
		}

		USplineComponent* CoverPath = ActiveCoverZone.GetValue()->GetCoverPath();
		if (!CoverPath) return;
		FVector SplineDirection = ActiveCoverZone.GetValue()->GetCoverPath()->FindDirectionClosestToWorldLocation(
			GetActorLocation(),
			ESplineCoordinateSpace::World);

		float SplineDistance = ActiveCoverZone.GetValue()->GetCoverPath()->GetDistanceAlongSplineAtLocation(
			GetActorLocation(),
			ESplineCoordinateSpace::World);

		float ClosestInput = CoverPath->FindInputKeyClosestToWorldLocation(GetActorLocation());
		int32 SplineCount = CoverPath->GetNumberOfSplinePoints();
		for (int32 i = 0; i < SplineCount; i++)
		{
			
		}

		// Test cover spline goes right to left, TODO: should probably reverse this when we do it properly!
		if (SplineDistance >= ActiveCoverZone.GetValue()->GetCoverPath()->GetSplineLength() / 2.f
			&& MovementVector.X < 0.f)
		{
		//	SpringArm->SetRelativeLocation(FVector(0.0f, -90.f, 70.f));
			SpringArmTargetY = -90.f;
		} else if (SplineDistance < ActiveCoverZone.GetValue()->GetCoverPath()->GetSplineLength() / 2.f
			&& MovementVector.X > 0.f)
		{
			//SpringArm->SetRelativeLocation(FVector(0.0f, 90.f, 70.f));
			SpringArmTargetY = 90.f;
		}
		if (SplineDistance >= ActiveCoverZone.GetValue()->GetCoverPath()->GetSplineLength() - 30.f &&
			MovementVector.X < 0.f)
		{
			// No movement
		} else if (SplineDistance <= 30.f &&
			MovementVector.X > 0.f)
		{
		} else
		{
			AddMovementInput(SplineDirection, -MovementVector.X);
		}

	} else
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AGunSlinger::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector.X * 0.5f);
	AddControllerPitchInput(LookVector.Y * 0.5f);
}

void AGunSlinger::Attack(const FInputActionValue& Value)
{
	if (MovementTakeOver) return;
	// Will depend on weapon in hand probably
	if (ShootingComponent)
	{
		FHitResult Hit;
		ShootingComponent->Shoot(Hit);
	}
}

void AGunSlinger::Aim(const FInputActionValue& Value)
{
	IsAiming = true;
	SpringArm->bEnableCameraLag = false;
}

void AGunSlinger::StopAiming(const FInputActionValue& Value)
{
	IsAiming = false;
	SpringArm->bEnableCameraLag = true;
}

void AGunSlinger::TakeCover()
{
	if (!OverlappedCoverZone.IsSet())
	{
		// Log something maybe
		return;
	}
	bool ClearZone = ActiveCoverZone.IsSet();
	UE_LOG(LogTemp, Warning, TEXT("ClearZone: %s"), ClearZone ? TEXT("True") : TEXT("False"));
	SetActiveCoverZone(OverlappedCoverZone.GetValue(), ClearZone);
}

void AGunSlinger::UpdateAim()
{
	if (IsAiming)
	{
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 150.f, GetWorld()->GetDeltaSeconds(),2000.f);
	} else
	{
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 300.f, GetWorld()->GetDeltaSeconds(),2000.f);
	}

	float SpringY = SpringArm->GetRelativeLocation().Y;
	SpringY = FMath::FInterpTo(SpringY, SpringArmTargetY, GetWorld()->GetDeltaSeconds(), 10.f);
	SpringArm->SetRelativeLocation(FVector(SpringArm->GetRelativeLocation().X, SpringY, SpringArm->GetRelativeLocation().Z));

	// TODO: This should be elsewhere lol
	float MeshRotY = GetMesh()->GetRelativeRotation().Yaw;
	MeshRotY = FMath::FInterpTo(MeshRotY, MeshRotationTargetY , GetWorld()->GetDeltaSeconds(), 10.f);
	GetMesh()->SetRelativeRotation(FRotator(GetMesh()->GetRelativeRotation().Pitch,
		MeshRotY,
		GetMesh()->GetRelativeRotation().Roll));
}

