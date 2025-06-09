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
#include "DetShoot/Interactive/Interactive.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Net/UnrealNetwork.h"


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

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun"));
	GunMesh->SetupAttachment(GetMesh());

	ShootingComponent = CreateDefaultSubobject<UShootingComponent>(TEXT("Shooting Component"));

	AIController = CreateDefaultSubobject<AAIController>(TEXT("Ai Controller?"));
}

// Called when the game starts or when spawned
void AGunSlinger::BeginPlay()
{
	Super::BeginPlay();

	if (GunMesh)
	{
		GunMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("GunSocket"));
	}
	
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
		EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AGunSlinger::Dodge);
		EIC->BindAction(UseAction, ETriggerEvent::Completed, this, &AGunSlinger::UseInteractive);

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
		ActiveCoverZone.Reset();
		SetRotationTarget(0.f);
		SpringArmTargetY = 90.f;
//		MeshSpring->bInheritYaw = true;
		UnCrouch();
		if (HasAuthority())
		{
			Multi_SetInCover(this, false);
		} else
		{
			Server_SetInCover(this, false);
		}
		return;
	}
	
//	MeshSpring->bInheritYaw = false;
	MovementTakeOver = true;
	if (Zone->CrouchBehind)
	{
		Crouch();
		if (HasAuthority())
		{
			Multi_SetInCover(this, true);
		} else
		{
			Server_SetInCover(this, true);
		}
	}
	ActiveCoverZone = Zone;
	const float SplineDist = ActiveCoverZone.GetValue()->GetCoverPath()->GetDistanceAlongSplineAtLocation(
		GetActorLocation(),
		ESplineCoordinateSpace::World);
	
	TargetLocation = ActiveCoverZone.GetValue()->GetCoverPath()->GetLocationAtDistanceAlongSpline(
		SplineDist,
		ESplineCoordinateSpace::World);

    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller,
    	FVector(TargetLocation.X, TargetLocation.Y, GetActorLocation().Z));

	//MeshRotationTargetY = 180.f;
	//SetRotationTarget(180.f);
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

bool AGunSlinger::GetIsAiming()
{
	return IsAiming;
}

bool AGunSlinger::GetIsCrouching()
{
	return IsCrouching;
}

bool AGunSlinger::GetIsInCover()
{
	return ActiveCoverZone.IsSet();
}

bool AGunSlinger::GetDodging()
{
	return IsDodging;
}

void AGunSlinger::Server_SetAnimationState_Implementation(AGunSlinger* Target, bool InCover, bool Aiming)
{
	Multi_SetAnimationState(this, IsCrouching, IsAiming);
}

void AGunSlinger::Multi_SetAnimationState_Implementation(AGunSlinger* Target, bool InCover, bool Aiming)
{
	Target->IsAiming = Aiming;
	Target->SetIsInCover(InCover);
}

void AGunSlinger::Server_SetRotationTarget_Implementation(AGunSlinger* Target, float RotTarget)
{
	Target->MeshRotationTargetY = RotTarget;
	//Multi_SetRotationTarget(Target, RotTarget);
}

void AGunSlinger::Multi_SetRotationTarget_Implementation(AGunSlinger* Target, float RotTarget)
{
	UE_LOG(LogTemp, Warning, TEXT("Setting Rotation Target to everyone: %f"), RotTarget);
	Target->MeshRotationTargetY = RotTarget;
	UE::Math::TQuat<double> MeshRot = GetMesh()->GetRelativeRotation().Quaternion();

//	Target->BaseRotationOffset = MeshRot;
}

void AGunSlinger::Server_LoadLevelInstance_Implementation()
{
	Multi_LoadLevelInstance();
}

void AGunSlinger::Multi_LoadLevelInstance_Implementation()
{
	FVector Loc = FVector(0.f, 0.f, 0.f);
	bool success;
	ULevelStreamingDynamic* LoadedLevel = ULevelStreamingDynamic::LoadLevelInstance(
		GetWorld(),
		"LoadTest",
		Loc,
		FRotator(0.f, 0.f, 0.f),
		success,
		TEXT("LoadTest1"));
	if (!success)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not load level instance"));
	}	
}

void AGunSlinger::Multi_Interact_Implementation(FVector Pos, FVector Dir)
{
	// Fire Raycast
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(
		Hit,
		Pos,
		Pos + Dir * 100.f,
		ECC_GameTraceChannel1);

	if (Hit.bBlockingHit)
	{
		AActor* IntAct = Hit.GetActor();
		AInteractive* Int = Cast<AInteractive>(IntAct);
		Int->Trigger();
	}
}

void AGunSlinger::Server_Interact_Implementation(FVector Pos, FVector Dir)
{
	Multi_Interact(Pos, Dir);
}

void AGunSlinger::SetRotationTarget(float RotTarget)
{
	return;
//	if (HasAuthority())
//	{
//		Multi_SetRotationTarget(this, RotTarget);
//	} else
//	{
//		Server_SetRotationTarget(this, RotTarget);
//	}
}

void AGunSlinger::SetIsInCover(bool Cover)
{
	IsCrouching = Cover;
}

void AGunSlinger::Multi_SetInCover_Implementation(AGunSlinger* Target, bool InCover)
{
	Target->MeshSpring->bInheritYaw = !InCover;
	Target->SetIsInCover(InCover);
}

void AGunSlinger::Server_SetInCover_Implementation(AGunSlinger* Target, bool InCover)
{
	Multi_SetInCover(Target, InCover);
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

			//SetRotationTarget(MoveDir.Rotation().Yaw);
			MeshRotationTargetY = MoveDir.Rotation().Yaw;
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
			
			//SetRotationTarget(MoveDir.Rotation().Yaw - 180.f);
			MeshRotationTargetY = MoveDir.Rotation().Yaw - 180.f;
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
			SpringArmTargetY = -90.f;
		} else if (SplineDistance < ActiveCoverZone.GetValue()->GetCoverPath()->GetSplineLength() / 2.f
			&& MovementVector.X > 0.f)
		{
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
	if (!ActiveCoverZone.IsSet() && !IsAiming)
	{
		FVector CamLookPoint = Camera->GetComponentLocation() + (Camera->GetForwardVector() * 2000.f);
		FVector LookDir = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), CamLookPoint);
		MeshRotationTargetY = -(GetActorForwardVector().Rotation().Yaw - LookDir.Rotation().Yaw);
	}

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
	if (HasAuthority())
	{
		Multi_SetAnimationState(this, IsCrouching, IsAiming);
	} else
	{
		Server_SetAnimationState(this, IsCrouching, IsAiming);
	}
	SpringArm->bEnableCameraLag = false;
}

void AGunSlinger::StopAiming(const FInputActionValue& Value)
{
	IsAiming = false;
	if (HasAuthority())
	{
		Multi_SetAnimationState(this, IsCrouching, IsAiming);
	} else
	{
		Server_SetAnimationState(this, IsCrouching, IsAiming);
	}
	SpringArm->bEnableCameraLag = true;
}

void AGunSlinger::TakeCover()
{
	
	UE_LOG(LogTemp, Warning, TEXT("Taking Cover?"));
	const bool ClearZone = ActiveCoverZone.IsSet();
	if (ClearZone)
	{
		// This is a bit weird
		SetActiveCoverZone(ActiveCoverZone.GetValue(), ClearZone);
		UE_LOG(LogTemp, Warning, TEXT("Clearing Active Zone"));
		return;
	}
	if (!OverlappedCoverZone.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlap zone not set"));
		// Log something maybe
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Crouching!"));
	SetActiveCoverZone(OverlappedCoverZone.GetValue(), ClearZone);
}

void AGunSlinger::Dodge()
{
	IsDodging = true;
	UE_LOG(LogTemp, Warning, TEXT("Dodging!"));
	if (HasAuthority())
	{
		Multi_LoadLevelInstance();
	} else
	{
		Server_LoadLevelInstance();
	}
}

void AGunSlinger::UseInteractive()
{
	if (HasAuthority())
	{
		Multi_Interact(GetActorLocation(), GetActorForwardVector());
	} else
	{
		Server_Interact(GetActorLocation(), GetActorForwardVector());
	}
}

void AGunSlinger::UpdateAim()
{
	if (IsAiming)
	{
		if (!ActiveCoverZone.IsSet())
		{
		//	MeshSpring->bInheritYaw = true;
			
			MeshRotationTargetY = 0.f;
		//	SetRotationTarget(0.f);
		} else
		{
			//SetRotationTarget(GetActorForwardVector().Rotation().Yaw);
			MeshRotationTargetY = GetActorForwardVector().Rotation().Yaw;
		}
		
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 150.f, GetWorld()->GetDeltaSeconds(),2000.f);
	} else
	{
		if (ActiveCoverZone.IsSet())
		{
		//	MeshSpring->bInheritYaw = false;
		}
		SpringArm->TargetArmLength = FMath::FInterpConstantTo(SpringArm->TargetArmLength, 300.f, GetWorld()->GetDeltaSeconds(),2000.f);
	}

	float SpringY = SpringArm->SocketOffset.Y;
	SpringY = FMath::FInterpTo(SpringY, SpringArmTargetY, GetWorld()->GetDeltaSeconds(), 10.f);
	SpringArm->SocketOffset.Y = SpringY;

	// TODO: This should be elsewhere lol
	UE::Math::TQuat<double> MeshRot = MeshSpring->GetRelativeRotation().Quaternion();
	UE::Math::TQuat<double> TargetRot = FRotator(
		MeshSpring->GetRelativeRotation().Pitch,
		MeshRotationTargetY,
		MeshSpring->GetRelativeRotation().Roll).Quaternion();

	float RotSpeed = IsAiming ? 0.5f : 0.3f;
	
	MeshRot = UKismetMathLibrary::Quat_Slerp(
		MeshRot,
		TargetRot,
		RotSpeed);
	if (HasAuthority())
	{
	} else
	{
		if (IsLocallyControlled())
		{
			Server_SetRotationTarget(this, MeshRotationTargetY);
		}
	}
	MeshSpring->SetRelativeRotation(MeshRot);
}

void AGunSlinger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Here we list the variables we want to replicate
	DOREPLIFETIME(AGunSlinger, IsCrouching);
	DOREPLIFETIME(AGunSlinger, IsAiming);
	DOREPLIFETIME(AGunSlinger, MeshRotationTargetY);
}

