// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AICharacter.generated.h"

UCLASS()
class SHOKAN_GAME0_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();

	//Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* HandWeapon;					
	
	//Weapon mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* GunMesh;			

	//Kind of AI, 0 is Assault, 1 is Sniper, 2 is Machine gunner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_Set")
	int KindOfAI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_Set")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	bool bHip;

	UPROPERTY(BlueprintReadOnly)
	bool bDied;

	UPROPERTY(BlueprintReadWrite)
	bool bDefense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_Set")
	class USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI_Set")
	class UParticleSystem* EmitterTemplateGunFire;										     

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SpawnSoundAndEmitter();

	/** 承受伤害的事件。从APawn覆盖。*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

};
