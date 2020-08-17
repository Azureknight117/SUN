// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SUN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();
	UHealthComponent(float MHP);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, Category = Health)
	float MaxHealth = 100.f;

	UFUNCTION()
	void HandleDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	float CurrentHealth = 100.f;

	
	void TakeDamage(float Dmg);
	void HealDamage(float Heal);
	void Die();
	void ResetHealth()
	{
		CurrentHealth = MaxHealth;
	}
		
};
