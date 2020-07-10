// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "SUNCharacter.generated.h"

class UInputComponent;
UENUM()
enum WallRunSide
{
	Left,
	Right
};

enum WallRunEndReason
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

	UPROPERTY(VisibleAnywhere, Category = "Trigger Capsule")
	class UCapsuleComponent* TriggerCapsule;

public:
	ASUNCharacter();

protected:
	virtual void BeginPlay();

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

	UFUNCTION()
	virtual void OnOverlapBegin(class UPrimitiveComponent* newComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex); 
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanJumpInAir;
	int  NumJumps;
	int  MaxJumps;
	void DoubleJump();
	virtual void Landed(const FHitResult& Hit) override;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float JumpHeight = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanWallRun;
	bool IsWallRunning;
	FVector WallRunDirection;
	WallRunSide SideOfWall;

	void WallRun();

	void Dash();
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool CanDash;
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DashAmount = 10;
	FTimerHandle DashTimer;
	void StopDash();

};

