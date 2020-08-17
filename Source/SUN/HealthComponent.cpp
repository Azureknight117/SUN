// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

UHealthComponent::UHealthComponent(float MHP)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	MaxHealth = MHP;
	CurrentHealth = MaxHealth;
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	AActor* Owner = GetOwner();
	if(Owner)
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleDamage);
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	
}

void UHealthComponent::HandleDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	TakeDamage(Damage);
}
void UHealthComponent::TakeDamage(float Dmg)
{
	CurrentHealth -= Dmg;
	if(CurrentHealth < 0)
	{
		CurrentHealth = 0;
		Die();
	}
}

void UHealthComponent::HealDamage(float Heal)
{
	CurrentHealth += Heal;
	if(CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}

void UHealthComponent::Die()
{
	GetOwner()->Destroy();
}
