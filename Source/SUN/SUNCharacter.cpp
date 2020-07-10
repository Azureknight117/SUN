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
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
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
	

}

//////////////////////////////////////////////////////////////////////////
// Input

void ASUNCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASUNCharacter::DoubleJump);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASUNCharacter::Dash);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASUNCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASUNCharacter::EndFire);


	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ASUNCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASUNCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASUNCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASUNCharacter::LookUpAtRate);
}

void ASUNCharacter::StartFire()
{
	FireShot();
	GetWorldTimerManager().SetTimer(ShotTimer,this,&ASUNCharacter::FireShot,WeaponFireRate,true);
}

void ASUNCharacter::FireShot()
{
	FHitResult Hit;

	const float WeaponRange = 20000.f;
	const FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	const FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * WeaponRange) + StartTrace;

	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace),false,this);
	if(GetWorld()->LineTraceSingleByChannel(Hit, StartTrace,EndTrace, ECC_Visibility,QueryParams))
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s"), Hit.GetActor()->GetName())
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

void ASUNCharacter::DoubleJump()
{
	if(NumJumps < MaxJumps)
	{
		ACharacter::LaunchCharacter(FVector(0,0,JumpHeight), false, true);
		NumJumps++;
	}
}

void ASUNCharacter::Landed(const FHitResult & Hit)
{
	NumJumps = 0;;
}

void ASUNCharacter::Dash()
{
	if(CanDash)
	{
		FVector DashDirection = GetVelocity();
		DashDirection.Z = 0;
		GetCharacterMovement()->GroundFriction = 0.f;
		ACharacter::LaunchCharacter((DashDirection) * DashAmount, true, true);
		GetWorldTimerManager().SetTimer(DashTimer, this, &ASUNCharacter::StopDash, .25f, false);

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

void ASUNCharacter::WallRun()
{
	
}

void ASUNCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASUNCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
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

