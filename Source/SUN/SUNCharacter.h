// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Components/ActorComponent.h"
#include "SUNCharacter.generated.h"

class UInputComponent;
UENUM()
enum EWallRunSide
{
	Left,
	Right
};

enum EWallRunEndReason
{
	FallOffWall,
	JumpedOffWall
};

UCLASS(config=Game)
class ASUNCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, Category = "Trigger Capsule")
	class UCapsuleComponent* TriggerCapsule;




public:
	ASUNCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;


	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ASUNProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallRun)
	float WallRunSpeed = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WallRun)
	float PlayerToWallDistance = 75;
	void AttachToWall(int Direction, float WallSpeed, FHitResult HitResult);
protected:
	
	/** Fires a projectile. */

	void StartFire();
	void EndFire();
	void FireShot();

	FTimerHandle ShotTimer;


	UPROPERTY(EditAnywhere)
	float WeaponFireRate = .25f;

	void MoveForward(float Val);

	void MoveRight(float Val);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;


public:
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	float ForwardAxis;
	float RightAxis;

	//Double Jump
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanJumpInAir;
	int  NumJumps;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	int  MaxJumps;
	void DoubleJump();
	virtual void Landed(const FHitResult& Hit) override;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float JumpHeight = 500.f;
	void SetJumps(int Jumps);

	//Wall run
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanWallRun;
	bool IsWallRunning;
	FVector WallRunDirection;
	FTimerHandle WallRunTimer;
	EWallRunSide WallRunSide;
	FTimerHandle CameraTiltTimer;
	void WallRun();
	void BeginWallRun();
	void EndWallRun(EWallRunEndReason Reason);
	void FindDirectionAndSide(FVector WallNormal);
	bool CanSurfaceBeRan(FVector SurfaceNormal) const;

	//Dash
	void Dash();
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanDash;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DashAmount = 10;
	FTimerHandle DashTimer;
	void StopDash();

};

