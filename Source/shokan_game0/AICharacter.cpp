// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// default weapon 1
	HandWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunWeapon"));
	HandWeapon->SetupAttachment(GetMesh(), TEXT("weapon_gun"));
	HandWeapon->SetSkeletalMesh(GunMesh);

	KindOfAI = 0;
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bHip = false;
	bDied = false;
	bDefense = false;
}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	if (KindOfAI == 1)
	{
		bHip = true;
	}
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacter::SpawnSoundAndEmitter()
{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateGunFire, HandWeapon->GetSocketLocation(TEXT("Muzzle")), FRotator::ZeroRotator);
}

float AAICharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	bDefense = true;
	float damageApplied = CurrentHealth - DamageAmount;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	if (CurrentHealth <= 0)
	{
		bDied = true;
	}

	// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(CurrentHealth));
	return CurrentHealth;
}

