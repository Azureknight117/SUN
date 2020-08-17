// Copyright Epic Games, Inc. All Rights Reserved.

#include "SUNCharacter.h"
#include "SUNProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Vector.h"

#define LEFT -90
#define RIGHT 90
DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ASUNCharacter

ASUNCharacter::ASUNCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));


	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(RootComponent);

	FP_Sword = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Katana"));
	FP_Sword->SetOnlyOwnerSee(true);			
	FP_Sword->bCastDynamicShadow = false;
	FP_Sword->CastShadow = false;
	FP_Sword->SetupAttachment(RootComponent);
	FP_Sword->SetRelativeScale3D(FVector(10, 10, 10));

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	//Collision For Wall Run
	TriggerCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("TriggerCapsule"));
	TriggerCapsule->InitCapsuleSize(56.f, 96.0f);
	TriggerCapsule->SetCollisionProfileName(TEXT("Pawn"));
	//TriggerCapsule->SetNotifyRigidBodyCollision("true");
	TriggerCapsule->SetupAttachment(RootComponent);

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ASUNCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	if(CanJumpInAir)
	{
		MaxJumps = 2;
	}
	else
	{
		MaxJumps = 1;
	}
	WeaponMode = GUN;
	//TriggerCapsule ->OnComponentHit.AddDynamic(this, &ASUNCharacter::OnCompHit);
}


void ASUNCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASUNCharacter::DoubleJump);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASUNCharacter::Dash);

	// Bind Attack events
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASUNCharacter::StartAttack);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASUNCharacter::EndAttack);
	PlayerInputComponent->BindAction("SwitchMode", IE_Pressed, this, &ASUNCharacter::SwitchWeaponMode);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ASUNCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASUNCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASUNCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASUNCharacter::LookUpAtRate);
}

void ASUNCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		ForwardAxis = Value;
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASUNCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		RightAxis = Value;
		AddMovementInput(GetActorRightVector(), Value);
	
	}
}

void ASUNCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASUNCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASUNCharacter::SwitchWeaponMode()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("MODE CHANGE")));
}

void ASUNCharacter::StartAttack()
{
	if(WeaponMode == GUN)
	{
		StartFire();
		return;
	}
	else if(WeaponMode == MELEE)
	{
		StartMelee();
		return;
	}
}

void ASUNCharacter::EndAttack()
{
	if(WeaponMode == GUN)
	{
		EndFire();
		return;
	}
	else if(WeaponMode == MELEE)
	{
		EndMelee();
		return;
	}
}



void ASUNCharacter::StartFire()
{
	FireShot();
	GetWorldTimerManager().SetTimer(ShotTimer,this,&ASUNCharacter::FireShot,WeaponFireRate,true);
}

//Loops for automatic rifle
void ASUNCharacter::FireShot()
{
	FHitResult Hit;

	const float WeaponRange = 20000.f;
	const FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	const FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * WeaponRange) + StartTrace;

	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace),false,this);
	if(GetWorld()->LineTraceSingleByChannel(Hit, StartTrace,EndTrace, ECC_Visibility,QueryParams))
	{
		AActor* HitActor = Hit.GetActor();
		UGameplayStatics::ApplyPointDamage(HitActor, 20.f, GetActorLocation(), Hit, nullptr, this, DamageType);
	}

	DrawDebugLine(GetWorld(),StartTrace, EndTrace, FColor::White, false, 1.0f, 0, 1.0f);

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ASUNCharacter::EndFire()
{
	GetWorldTimerManager().ClearTimer(ShotTimer);
}

void ASUNCharacter::StartMelee()
{}
void ASUNCharacter::Melee()
{}
void ASUNCharacter::EndMelee()
{}
//Double Jump
void ASUNCharacter::DoubleJump()
{
	bool isFalling = GetCharacterMovement()->MovementMode==EMovementMode::MOVE_Falling;
	if(NumJumps < MaxJumps)
	{
		if(!IsWallRunning)
		{
			ACharacter::LaunchCharacter(FVector(0,0,JumpHeight), false, true);
			NumJumps++;
			if(isFalling) NumJumps++;
		}
		else
		{
			EndWallRun(JumpedOffWall);
			if(WallRunSide == Left) //Slightly offset when jumping off a wall to jump away from it
			{
				ACharacter::LaunchCharacter(FVector(0,20,JumpHeight), false, true);
			}
			else
			{
				ACharacter::LaunchCharacter(FVector(0,-20,JumpHeight), false, true);
			}
			SetJumps(0);
			NumJumps++;
		}
	}
}

void ASUNCharacter::SetJumps(int Jumps)
{
	NumJumps = Jumps;
}

void ASUNCharacter::Landed(const FHitResult & Hit)
{
	SetJumps(0);
}

//Dash in Direction of player's movement
void ASUNCharacter::Dash()
{
	if(CanDash)
	{
		FVector DashDirection = GetVelocity();
		DashDirection.Z = 0;
		GetCharacterMovement()->GroundFriction = 0.f;
		ACharacter::LaunchCharacter((DashDirection) * DashAmount, true, true);
		GetWorldTimerManager().SetTimer(DashTimer, this, &ASUNCharacter::StopDash, .25f, false);
		if(IsWallRunning)EndWallRun(JumpedOffWall);
		// try and play the sound if specified
		if (FireSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

void ASUNCharacter::StopDash()
{
	GetCharacterMovement()->GroundFriction = 8;
	GetCharacterMovement()->StopMovementImmediately();
}

void ASUNCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()->IsFalling())
	{
		FHitResult HitResultForward;
		FHitResult HitResultLeft;
		FHitResult HitResulRight;
		FHitResult Hit;
		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Trace")), true, this);

		ECollisionChannel Channel = ECC_WorldStatic;

		FVector Start = GetActorLocation();
		FVector End = GetActorRightVector() * PlayerToWallDistance;
		FVector ForwardEnd = GetActorForwardVector() * PlayerToWallDistance;
		
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + -End, Channel, TraceParams))
		{
			if(!IsWallRunning)
			{
			
			if(CanSurfaceBeRan(Hit.ImpactNormal))
			{

				FindDirectionAndSide(Hit.ImpactNormal);
				WallRunSide = Left;
				BeginWallRun();
			}
			}
		}
		else if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + End, Channel, TraceParams))
		{
			if(!IsWallRunning)
			{

			if(CanSurfaceBeRan(Hit.ImpactNormal))
			{
				FindDirectionAndSide(Hit.ImpactNormal);
				WallRunSide = Right;
				BeginWallRun();
			}
			}
		}
	}
}

//Fires a raycast, so long as the raycast is hitting a wall it keeps the player wall running
void ASUNCharacter::WallRun()
{
	FVector WallSide;
	FHitResult Hit;
	switch(WallRunSide)
	{
		case Left:
			WallSide = FVector(0,0,-1);
			break;
		case Right:
			WallSide = FVector(0,0,1);
			break;
	}

	FVector ToWall = (FVector::CrossProduct(WallRunDirection, WallSide ) * 100) ;
	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WallTrace),false,this);
	EWallRunSide PrevSide;
	if(GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(),(GetActorLocation() + ToWall), ECC_Visibility,QueryParams))
	{
		PrevSide = WallRunSide;
		FindDirectionAndSide(Hit.ImpactNormal);
		DrawDebugLine(GetWorld(), GetActorLocation(), (GetActorLocation() + ToWall), FColor::Green, true);
		if(PrevSide != WallRunSide)
		{
			EndWallRun(FallOffWall);
			return;
		}
		else
		{
			FVector WallRunVelocity = WallRunDirection * GetCharacterMovement()->GetMaxSpeed();
			GetCharacterMovement()->Velocity = FVector(WallRunVelocity.X ,WallRunVelocity.Y, 0);
		}
	}
	else
	{
		EndWallRun(FallOffWall);
		return;
	}

}

//Turn off gravity and contrain player to singular air movment for wall run
void ASUNCharacter::BeginWallRun()
{
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0,0,1));
	IsWallRunning = true;
	GetWorldTimerManager().SetTimer(WallRunTimer, this, &ASUNCharacter::WallRun, .1f, true);
}

void ASUNCharacter::EndWallRun(EWallRunEndReason Reason)
{
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->GravityScale = 0.9f;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0,0,0));
	IsWallRunning = false;
	GetWorldTimerManager().ClearTimer(WallRunTimer);
}

//Finds the side of the player the wall is on and the direction the player will travel
void ASUNCharacter::FindDirectionAndSide(FVector WallNormal) 
{
	FVector2D WallNorm = FVector2D(WallNormal.X, WallNormal.Y);
	FVector2D ActorRight = FVector2D(GetActorRightVector().X, GetActorRightVector().Y);
	float DotProduct = FVector2D::DotProduct(WallNorm, ActorRight);
	FVector Direction;
	if(DotProduct > 0)
	{
		WallRunSide = Right;
		Direction = FVector(0,0,1);
		WallRunDirection = FVector::CrossProduct(WallNormal,Direction);
	}
	else
	{
		WallRunSide = Left;
		Direction = FVector(0,0,-1);
		WallRunDirection = FVector::CrossProduct(WallNormal,Direction);
	}
}

//Checks if the surface is wall runable (Not too flat or to steep)
bool ASUNCharacter::CanSurfaceBeRan(FVector SurfaceNormal) const
{
    if(SurfaceNormal.Z < -0.05)
	{
		return false;
	}
	else
	{
		FVector Surface = FVector (SurfaceNormal.X, SurfaceNormal.Y, 0);
		if(UKismetMathLibrary::DegAcos(FVector::DotProduct(Surface.GetSafeNormal(), SurfaceNormal)) < GetCharacterMovement()->GetWalkableFloorAngle())
		{
			return true;
		}
	}
	return false;
}