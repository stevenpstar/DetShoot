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

	MeshSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("Mesh Spring"));
	MeshSpring->SetupAttachment(RootComponent);

	MeshSpring->TargetArmLength = 0.f;
	MeshSpring->bInheritYaw = true;

	GetMesh()->SetupAttachment(MeshSpring);

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
		if (Distance < 65.f)
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
		MeshSpring->bInheritYaw = true;
		return;
	}
	
	MeshSpring->bInheritYaw = false;
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
		const USplineComponent* CoverPath = ActiveCoverZone.GetValue()->GetCoverPath();
		if (!CoverPath) return;

		float SplineDistance = ActiveCoverZone.GetValue()->GetCoverPath()->GetDistanceAlongSplineAtLocation(
			GetActorLocation(),
			ESplineCoordinateSpace::World);

		int32 ClosestSplineIndex = 0;
		int32 NextSplineIndex = 0;
		float LowestDistance = 99999.f;
		int32 SplineCount = CoverPath->GetNumberOfSplinePoints();
		
		FVector MoveDir = FVector::ZeroVector;
		FVector From = FVector::ZeroVector;
		FVector To = FVector::ZeroVector;

		float SplineDist = CoverPath->GetDistanceAlongSplineAtLocation(
			GetActorLocation(),
			ESplineCoordinateSpace::World);
		const FVector D = CoverPath->GetLocationAtDistanceAlongSpline(SplineDist,
			ESplineCoordinateSpace::World);
		SetActorLocation(FVector(D.X, D.Y, GetActorLocation().Z));
		MoveDir = FVector::ZeroVector;
		LowestDistance = TNumericLimits<float>().Max();
		float ActualDistValue = 0.f;
		
		for (int32 i = 0; i < SplineCount; i++)
		{
			float PointDist = CoverPath->GetDistanceAlongSplineAtSplinePoint(i);
			const float Dist = abs(SplineDist - PointDist);
			if (Dist < LowestDistance)
			{
				ClosestSplineIndex = i;
				ActualDistValue = SplineDist - PointDist; // if > 0 (depending on dir) you are arriving or leaving from point etc.
				LowestDistance = Dist;
			}
		}

		if (MovementVector.X > 0.f)
		{
			if (ActualDistValue >= 0.f)
			{
				NextSplineIndex = ClosestSplineIndex + 1;
				
				From = CoverPath->GetLocationAtDistanceAlongSpline(
					CoverPath->GetDistanceAlongSplineAtSplinePoint(NextSplineIndex),
					ESplineCoordinateSpace::World);

				To = CoverPath->GetLocationAtDistanceAlongSpline(
					CoverPath->GetDistanceAlongSplineAtSplinePoint(ClosestSplineIndex),
					ESplineCoordinateSpace::World);

				MoveDir = UKismetMathLibrary::GetDirectionUnitVector(From, To);
			} else
			{
				NextSplineIndex = ClosestSplineIndex - 1;
				From = CoverPath->GetLocationAtDistanceAlongSpline(
								CoverPath->GetDistanceAlongSplineAtSplinePoint(ClosestSplineIndex),
								ESplineCoordinateSpace::World);

				To = CoverPath->GetLocationAtDistanceAlongSpline(
								CoverPath->GetDistanceAlongSplineAtSplinePoint(NextSplineIndex),
								ESplineCoordinateSpace::World);
				
				MoveDir = UKismetMathLibrary::GetDirectionUnitVector(From, To);
			}

			MeshRotationTargetY = MoveDir.Rotation().Yaw - 90.f;
		}
		else if (MovementVector.X < 0.f)
		{
			if (ActualDistValue >= 0.f)
			{
				NextSplineIndex = ClosestSplineIndex + 1;
				From = CoverPath->GetLocationAtDistanceAlongSpline(
								CoverPath->GetDistanceAlongSplineAtSplinePoint(ClosestSplineIndex),
								ESplineCoordinateSpace::World);

				To = CoverPath->GetLocationAtDistanceAlongSpline(
								CoverPath->GetDistanceAlongSplineAtSplinePoint(NextSplineIndex),
								ESplineCoordinateSpace::World);
				
				MoveDir = UKismetMathLibrary::GetDirectionUnitVector(To, From);
			} else
			{
				NextSplineIndex = ClosestSplineIndex - 1;
				
				From = CoverPath->GetLocationAtDistanceAlongSpline(
					CoverPath->GetDistanceAlongSplineAtSplinePoint(NextSplineIndex),
					ESplineCoordinateSpace::World);

				To = CoverPath->GetLocationAtDistanceAlongSpline(
					CoverPath->GetDistanceAlongSplineAtSplinePoint(ClosestSplineIndex),
					ESplineCoordinateSpace::World);

				MoveDir = UKismetMathLibrary::GetDirectionUnitVector(To, From);
			}
			
			MeshRotationTargetY = MoveDir.Rotation().Yaw + 90.f;
		}

		DrawDebugPoint(
	GetWorld(),
	From,
	10.f,
	FColor::Emerald);

		DrawDebugPoint(
			GetWorld(),
			To,
			10.f,
			FColor::Cyan);
		
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
			AddMovementInput(MoveDir, MovementVector.X);
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
	
	bool ClearZone = ActiveCoverZone.IsSet();
	if (ClearZone)
	{
		// This is a bit weird
		SetActiveCoverZone(ActiveCoverZone.GetValue(), ClearZone);
		return;
	}
	if (!OverlappedCoverZone.IsSet())
	{
		// Log something maybe
		return;
	}
	SetActiveCoverZone(OverlappedCoverZone.GetValue(), ClearZone);
}

void AGunSlinger::UpdateAim()
{
	if (IsAiming)
	{
		MeshSpring->bInheritYaw = true;
		MeshRotationTargetY = -90.f;
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 150.f, GetWorld()->GetDeltaSeconds(),2000.f);
	} else
	{
		if (ActiveCoverZone.IsSet())
		{
			MeshSpring->bInheritYaw = false;
		}
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 300.f, GetWorld()->GetDeltaSeconds(),2000.f);
	}

	float SpringY = SpringArm->GetRelativeLocation().Y;
	SpringY = FMath::FInterpTo(SpringY, SpringArmTargetY, GetWorld()->GetDeltaSeconds(), 10.f);
	SpringArm->SetRelativeLocation(FVector(SpringArm->GetRelativeLocation().X, SpringY, SpringArm->GetRelativeLocation().Z));

	// TODO: This should be elsewhere lol
	UE::Math::TQuat<double> MeshRot = GetMesh()->GetRelativeRotation().Quaternion();
	UE::Math::TQuat<double> TargetRot = FRotator(
		GetMesh()->GetRelativeRotation().Pitch,
		MeshRotationTargetY,
		GetMesh()->GetRelativeRotation().Roll).Quaternion();
	
	MeshRot = UKismetMathLibrary::Quat_Slerp(
		MeshRot,
		TargetRot,
		0.1f);
	GetMesh()->SetRelativeRotation(MeshRot);
}

