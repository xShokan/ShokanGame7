// Copyright Epic Games, Inc. All Rights Reserved.

#include "shokan_game0Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include <Engine/Engine.h>
#include "Net/UnrealNetwork.h"
#include "GrenadeAmmo.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/KismetMathLibrary.h>
#include "AICharacter.h"

//////////////////////////////////////////////////////////////////////////
// Ashokan_game0Character

Ashokan_game0Character::Ashokan_game0Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// default weapon 1
	HandWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunWeapon"));
	HandWeapon->SetupAttachment(GetMesh(), TEXT("weapon_gun"));
	HandWeapon->SetSkeletalMesh(GunMesh);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//初始化玩家生命值
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	Gun5AmmoActor = nullptr;
	bDied = false;
	bAlive = false;
	KillNumber = 0;
	DeathNumber = 0;
	bGameOver = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

// Called every frame
void Ashokan_game0Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (!IsLocallyControlled() && !isHip)
	//{
	//	FRotator NewRot = GetActorRotation();
	//	NewRot.Pitch = RemoteViewPitch * 360 / 255.0f;
	//	SetActorRotation(NewRot);
	//}
	/*if (!IsLocallyControlled()) {
		FRotator NewRot = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), GetControlRotation());
		IronPitch = NewRot.Pitch * (-1);
		IronYaw = NewRot.Yaw;
	}*/

	FRotator NewRot = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), GetControlRotation());
	IronPitch = NewRot.Pitch * (-1);
	IronYaw = NewRot.Yaw;

	if (IronPitch == 0.f)
	{
		IronPitch = RemoteViewPitch * 360.f / 255.f;
	}
	if (!isHip && KindOfWeapon == 1 && KindOfGun != 5)
	{
		/*FVector FireStart = FollowCamera->GetComponentLocation();
		FVector FireEnd = FollowCamera->GetForwardVector() * 5000 + FireStart;*/

		FVector FireStart = HandWeapon->GetSocketLocation(TEXT("Muzzle"));
		FVector FireEnd = HandWeapon->GetForwardVector() * 5000 + FireStart;
		TArray<AActor*> ActorToIgnore;
		UKismetSystemLibrary::LineTraceSingle(this, FireStart, FireEnd, ETraceTypeQuery::TraceTypeQuery2, false, ActorToIgnore, EDrawDebugTrace::ForOneFrame, OutHit, true);
		
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, *OutHit.GetComponent()->GetName());
	}
	else if (!isHip && KindOfWeapon == 1 && KindOfGun == 5)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("111111"));
		FPredictProjectilePathParams ProjectilePathParams;
		FPredictProjectilePathResult PredictResult;
		ProjectilePathParams.StartLocation = HandWeapon->GetSocketLocation(TEXT("Muzzle"));
		ProjectilePathParams.LaunchVelocity = HandWeapon->GetForwardVector() * 1500;
		ProjectilePathParams.bTraceWithCollision = true;
		ProjectilePathParams.bTraceWithChannel = true;
		ProjectilePathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
		ProjectilePathParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		ProjectilePathParams.MaxSimTime = 2.0f;
		ProjectilePathParams.SimFrequency = 20.0f;
		ProjectilePathParams.DrawDebugTime = 1.0f;
		UGameplayStatics::PredictProjectilePath(this, ProjectilePathParams, PredictResult);
	}

	TArray < AActor* > OverlappingActors;
	this->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num())
	{
		bShowPressF = true;
		OverlappingWeaponActor = OverlappingActors[0];
		//TMap<FString, int> MyHash;
		//MyHash["BP_Gun"] = 1;
		//switch (MyHash[WeaponActor->GetName()]) {
		//case 1:
		//	UE_LOG(LogTemp, Display, TEXT("BP_Gun"));
		//}
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, OverlappingWeaponActor->GetName());
		if (OverlappingWeaponActor->GetName().Left(7) == "BP_Gun_")
		{
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, OverlappingWeaponActor->GetName().Left(6));
			OverlappingKindOfWeapon = 1;
		}
		else if (OverlappingWeaponActor->GetName().Left(7) == "BP_Gun3")
		{
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, OverlappingWeaponActor->GetName().Left(7));
			OverlappingKindOfWeapon = 3;
		}
		else if (OverlappingWeaponActor->GetName().Left(7) == "BP_Gun4")
		{
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, OverlappingWeaponActor->GetName().Left(7));
			OverlappingKindOfWeapon = 4;
		}
		else if (OverlappingWeaponActor->GetName().Left(7) == "BP_Gun5")
		{
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, OverlappingWeaponActor->GetName().Left(7));
			OverlappingKindOfWeapon = 5;
		}
	}
	else
	{
		bShowPressF = false;
		OverlappingKindOfWeapon = 0;
	}

	//if (bDied)
	//{
	//	FLatentActionInfo LatentActionInfo;
	//	LatentActionInfo.CallbackTarget = this;
	//	LatentActionInfo.ExecutionFunction = "TestCall";
	//	LatentActionInfo.UUID = 123;
	//	LatentActionInfo.Linkage = 0;

	//	UKismetSystemLibrary::Delay(this, 10.0f, LatentActionInfo);
	//	bDied = false;
	//}

	if (bAlive)
	{
		CurrentHealth = MaxHealth;
	}
}

void Ashokan_game0Character::OnRep_YawPitch()
{
	FRotator NewRot = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), GetControlRotation());
	IronPitch = NewRot.Pitch * (-1);
	IronYaw = NewRot.Yaw;
	if (GetLocalRole() < ROLE_Authority)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(IronPitch));
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("##########"));
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(RemoteViewPitch));
	}

}

void Ashokan_game0Character::OnRep_HitScanTrace()
{
	FireSpawn();
}

void Ashokan_game0Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &Ashokan_game0Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &Ashokan_game0Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &Ashokan_game0Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &Ashokan_game0Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &Ashokan_game0Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &Ashokan_game0Character::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &Ashokan_game0Character::OnResetVR);

	PlayerInputComponent->BindAction("Ironsight", IE_Pressed, this, &Ashokan_game0Character::IronsightDown);
	PlayerInputComponent->BindAction("Ironsight", IE_Released, this, &Ashokan_game0Character::IronsightUp);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &Ashokan_game0Character::FireDown);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &Ashokan_game0Character::FireUp);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &Ashokan_game0Character::RunDown);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &Ashokan_game0Character::RunUp);

	PlayerInputComponent->BindAction("SwitchGun", IE_Pressed, this, &Ashokan_game0Character::SwitchGun);
	PlayerInputComponent->BindAction("SwitchSword", IE_Pressed, this, &Ashokan_game0Character::SwitchSword);
	
	PlayerInputComponent->BindAction("FreeView", IE_Pressed, this, &Ashokan_game0Character::FreeViewDown);
	PlayerInputComponent->BindAction("FreeView", IE_Released, this, &Ashokan_game0Character::FreeViewUp);

	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &Ashokan_game0Character::DropWeaponDown);
	PlayerInputComponent->BindAction("PickWeapon", IE_Pressed, this, &Ashokan_game0Character::PickWeaponDown);
	PlayerInputComponent->BindAction("GameOver", IE_Pressed, this, &Ashokan_game0Character::GameOver);
}


void Ashokan_game0Character::IronsightDown()
{
	if (bEquipWeapon && KindOfWeapon == 1)
	{
		ServerIronsightDown();
		isHip = false;
		GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
		bUseControllerRotationYaw = true;
		if (KindOfGun != 5) {
			CameraBoom->TargetArmLength = 200;
		}
		else
		{
			CameraBoom->TargetArmLength = 400;
		}
		CameraBoom->SetRelativeLocation(FVector(10, 100, 70));
	}
}

void Ashokan_game0Character::IronsightUp()
{
	if (KindOfWeapon == 1)
	{
		ServerIronsightUp();
		isHip = true;
		GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
		bUseControllerRotationYaw = false;
		CameraBoom->SetRelativeLocation(FVector(10, 10, 70));
		CameraBoom->TargetArmLength = 300;
	}
}

void Ashokan_game0Character::Fire()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		HitScanTrace.BurstCounter++;
	}
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	FireSpawn();
	
}



void Ashokan_game0Character::ServerIronsightDown_Implementation()
{
	isHip = false;
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	bUseControllerRotationYaw = true;
}

bool Ashokan_game0Character::ServerIronsightDown_Validate()
{
	return true;
}

void Ashokan_game0Character::ServerFire_Implementation()
{
	if (!isHip && KindOfWeapon == 1)
	{
		PlayAnimMontage(FireMontage);
		if (KindOfGun != 5)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
		else
		{
			Gun5AmmoActor = SpawnAmmo(AmmoGun5ToSpawn);
			UGameplayStatics::PlaySoundAtLocation(this, FireSound5, GetActorLocation());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateGun5Fire, HandWeapon->GetSocketLocation(TEXT("Muzzle")), FRotator::ZeroRotator);
		}


		/*FVector FireStart = GunWeapon->GetSocketLocation(TEXT("Muzzle"));
		FVector FireEnd = FollowCamera->GetForwardVector() * 5000 + FireStart;
		FHitResult OutHit;
		TArray<AActor*> ActorToIgnore;
		UKismetSystemLibrary::LineTraceSingle(this, FireStart, FireEnd, ETraceTypeQuery::TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::ForDuration, OutHit, true);*/

		if (OutHit.bBlockingHit) {

			// CheckHitCharacter();
			Ashokan_game0Character* HitCharacter = Cast<Ashokan_game0Character>(OutHit.GetActor());
			if (HitCharacter && !HitCharacter->bDied)
			{
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("HitCharacter"));
				Score++;
				float RemainingHealth = UGameplayStatics::ApplyDamage(HitCharacter, 10.0f, nullptr, this, nullptr);

				if (RemainingHealth <= 0)
				{
					KillNumber++;
				}
			}

			if (OutHit.GetActor()->GetName() == "MyActor_1")
			{
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(Score));
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat((OutHit.Location - HandWeapon->GetComponentLocation()).Size()));
				Score++;
			}
			else if (Cast<Ashokan_game0Character>(OutHit.Actor) && KindOfGun != 5)
			{
				float FireHeight = OutHit.Location.Z;
				TArray<USkeletalMeshComponent*> Components;
				OutHit.GetActor()->GetComponents<USkeletalMeshComponent>(Components);
				USkeletalMeshComponent* MeshComponent = Components[0];

				float headSocketLocation = MeshComponent->GetSocketLocation(TEXT("headSocket")).Z;
				float middleSocketLocation = MeshComponent->GetSocketLocation(TEXT("middleSocket")).Z;
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(headSocketLocation));
				if (FireHeight < middleSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateLowerBody, OutHit.Location, FRotator::ZeroRotator);
				}
				else if (FireHeight >= middleSocketLocation && FireHeight <= headSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateUpperBody, OutHit.Location, FRotator::ZeroRotator);
				}
				else if (FireHeight > headSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateHead, OutHit.Location, FRotator::ZeroRotator);
					Score++;
				}
			}
		}
	}

}

bool Ashokan_game0Character::ServerFire_Validate()
{
	return true;
}

void Ashokan_game0Character::ServerIronsightUp_Implementation()
{
	isHip = true;
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	bUseControllerRotationYaw = false;
}

bool Ashokan_game0Character::ServerIronsightUp_Validate()
{
	return true;
}

void Ashokan_game0Character::CheckHitCharacter_Implementation()
{
	Ashokan_game0Character* HitCharacter = Cast<Ashokan_game0Character>(OutHit.GetActor());
	if (HitCharacter)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("HitCharacter"));
		Score++;
	}
}

bool Ashokan_game0Character::CheckHitCharacter_Validate()
{
	return true;
}



void Ashokan_game0Character::ServerSetHealth_Implementation()
{
	CurrentHealth = FMath::Clamp(CurrentHealth - 10, 0.0f, MaxHealth);
}

bool Ashokan_game0Character::ServerSetHealth_Validate()
{
	return true;
}


void Ashokan_game0Character::FireDown()
{
	isFire = true;
	if (bEquipWeapon && KindOfWeapon == 1)
	{
		Fire();
	} else if (bEquipWeapon && KindOfWeapon == 2)
	{
		PlayAnimMontage(CutMontage);
		UGameplayStatics::PlaySoundAtLocation(this, CutSound, GetActorLocation());

		if (OutHit.GetActor()->GetName() == "MyActor_1" && (OutHit.Location - HandWeapon->GetComponentLocation()).Size() <= 100.0f)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(Score));
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat((OutHit.Location - HandWeapon->GetComponentLocation()).Size()));
			Score++;
		}
	}
}

void Ashokan_game0Character::FireUp()
{
	isFire = false;
}

void Ashokan_game0Character::RunDown()
{
	GetCharacterMovement()->MaxWalkSpeed=600;
}

void Ashokan_game0Character::RunUp()
{
	GetCharacterMovement()->MaxWalkSpeed = 400;
}

void Ashokan_game0Character::SwitchGun()
{
	if (bEquipWeapon) {
		KindOfWeapon = 1;
		HandWeapon->SetupAttachment(GetMesh(), TEXT("weapon_gun"));
		HandWeapon->SetSkeletalMesh(GunMesh);
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString(TEXT("1")));
	}
}

void Ashokan_game0Character::SwitchSword()
{
	if (bEquipWeapon) 
	{
		KindOfWeapon = 2;
		HandWeapon->SetupAttachment(GetMesh(), TEXT("weapon_sword"));
		HandWeapon->SetSkeletalMesh(SwordMesh);
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString(TEXT("2")));
	}
}


void Ashokan_game0Character::FreeViewDown()
{
	isFreeView = true;
}

void Ashokan_game0Character::FreeViewUp()
{
	isFreeView = false;
}



void Ashokan_game0Character::DropWeaponDown()
{
	if (bEquipWeapon)
	{
		bEquipWeapon = false;
		HandWeapon->SetSkeletalMesh(EmptyMesh);
		switch (KindOfGun)
		{
		case 1:
			SpawnObject(Gun1ToSpawn);
			break;
		case 3:
			SpawnObject(Gun3ToSpawn);
			break;
		case 4:
			SpawnObject(Gun4ToSpawn);
			break;
		case 5:
			SpawnObject(Gun5ToSpawn);
			break;
		}
	}
}

void Ashokan_game0Character::SpawnObject(TSubclassOf<AActor> ToSpawn)
{
	if (ToSpawn)
	{
		UWorld* world = GetWorld();
		FVector Loc = this->GetActorLocation() + this->GetActorForwardVector() * 50;
		FRotator Rot = this->GetActorRotation();
		if (world)
		{
			FActorSpawnParameters SpawnParams;
			AActor* SpawnActorRefGun1 = world->SpawnActor<AActor>(ToSpawn, Loc, Rot, SpawnParams);
			TArray<UStaticMeshComponent*> Components;
			SpawnActorRefGun1->GetComponents<UStaticMeshComponent>(Components);
			UStaticMeshComponent* StaticMeshComponent = nullptr;
			for (int32 i = 0; i < Components.Num(); i++)
			{
				StaticMeshComponent = Components[i];
			}
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			StaticMeshComponent->SetSimulatePhysics(true);
			StaticMeshComponent->AddForce(this->GetActorForwardVector() * 500000);
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, SpawnActorRefGun1->GetName());
		}
	}
}

AActor* Ashokan_game0Character::SpawnAmmo(TSubclassOf<AActor> ToSpawn)
{
	if (ToSpawn)
	{
		UWorld* world = GetWorld();
		FVector Loc = HandWeapon->GetSocketLocation(TEXT("Muzzle"));
		FRotator Rot = HandWeapon->GetSocketRotation(TEXT("Muzzle"));
		if (world)
		{
			FActorSpawnParameters SpawnParams;
			AActor* SpawnActorRefGun1 = world->SpawnActor<AActor>(ToSpawn, Loc, Rot, SpawnParams);
			TArray<UStaticMeshComponent*> Components;
			SpawnActorRefGun1->GetComponents<UStaticMeshComponent>(Components);
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(Components.Num()));
			UStaticMeshComponent* StaticMeshComponent = nullptr;
			for (int32 i = 0; i < Components.Num(); i++)
			{
				StaticMeshComponent = Components[i];
			}
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			StaticMeshComponent->SetSimulatePhysics(true);
			StaticMeshComponent->SetPhysicsLinearVelocity(HandWeapon->GetForwardVector() * 1500);
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, HandWeapon->GetForwardVector().ToString());
			return SpawnActorRefGun1;
		}
	}
	return nullptr;
}

void Ashokan_game0Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(Ashokan_game0Character, isHip);
	DOREPLIFETIME(Ashokan_game0Character, Score);
	DOREPLIFETIME(Ashokan_game0Character, CurrentHealth);
	DOREPLIFETIME(Ashokan_game0Character, KillNumber);
	DOREPLIFETIME(Ashokan_game0Character, bDied);
	DOREPLIFETIME(Ashokan_game0Character, bAlive);
	DOREPLIFETIME(Ashokan_game0Character, bGameOver);
	DOREPLIFETIME(Ashokan_game0Character, IronPitch);
	DOREPLIFETIME(Ashokan_game0Character, IronYaw);
	DOREPLIFETIME(Ashokan_game0Character, HitScanTrace);
	DOREPLIFETIME(Ashokan_game0Character, DeathNumber);
}

float Ashokan_game0Character::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageAmount;
	/*if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetHealth();
	}*/
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);


	if (CurrentHealth <= 0)
	{
		bDied = true;
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("You Died"));
		DeathNumber++;
	}

	// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(CurrentHealth));
	return CurrentHealth;
}


void Ashokan_game0Character::PickWeaponDown()
{
	if (!bEquipWeapon && OverlappingKindOfWeapon)
	{
		KindOfGun = OverlappingKindOfWeapon;
		bEquipWeapon = true;
		switch (OverlappingKindOfWeapon)
		{
		case 1:
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("pick 1"));
			HandWeapon->SetSkeletalMesh(GunMesh);
			break;
		case 3:
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("pick 3"));
			HandWeapon->SetSkeletalMesh(Gun3Mesh);
			break;
		case 4:
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("pick 4"));
			HandWeapon->SetSkeletalMesh(Gun4Mesh);
			break;
		case 5:
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("pick 5"));
			HandWeapon->SetSkeletalMesh(Gun5Mesh);
			break;
		}
		UWorld* world = GetWorld();
		if (world)
		{
			world->DestroyActor(OverlappingWeaponActor);
		}
	}
}

void Ashokan_game0Character::GameOver()
{
	bGameOver = !bGameOver;
}

void Ashokan_game0Character::FireSpawn()
{
	if (!isHip && KindOfWeapon == 1)
	{
		PlayAnimMontage(FireMontage);
		if (KindOfGun != 5)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
		else
		{
			Gun5AmmoActor = SpawnAmmo(AmmoGun5ToSpawn);
			UGameplayStatics::PlaySoundAtLocation(this, FireSound5, GetActorLocation());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateGun5Fire, HandWeapon->GetSocketLocation(TEXT("Muzzle")), FRotator::ZeroRotator);
		}


		/*FVector FireStart = GunWeapon->GetSocketLocation(TEXT("Muzzle"));
		FVector FireEnd = FollowCamera->GetForwardVector() * 5000 + FireStart;
		FHitResult OutHit;
		TArray<AActor*> ActorToIgnore;
		UKismetSystemLibrary::LineTraceSingle(this, FireStart, FireEnd, ETraceTypeQuery::TraceTypeQuery1, false, ActorToIgnore, EDrawDebugTrace::ForDuration, OutHit, true);*/

		if (OutHit.bBlockingHit) {
			// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("22222"));
			// CheckHitCharacter();
			Ashokan_game0Character* HitCharacter1 = Cast<Ashokan_game0Character>(OutHit.GetActor());
			AAICharacter* HitCharacter2 = Cast<AAICharacter>(OutHit.GetActor());
			ACharacter* HitCharacter = Cast<ACharacter>(OutHit.GetActor());
			if ((HitCharacter1 && !HitCharacter1->bDied) || (HitCharacter2 && !HitCharacter2->bDied))
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("HitCharacter"));
				Score++;
				float RemainingHealth = UGameplayStatics::ApplyDamage(HitCharacter, 10.0f, nullptr, this, nullptr);

				if (RemainingHealth <= 0)
				{
					KillNumber++;
				}
			}

			if (OutHit.GetActor()->GetName() == "MyActor_1")
			{
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(Score));
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat((OutHit.Location - HandWeapon->GetComponentLocation()).Size()));
				Score++;
			}
			else if (Cast<Ashokan_game0Character>(OutHit.Actor) && KindOfGun != 5)
			{
				float FireHeight = OutHit.Location.Z;
				TArray<USkeletalMeshComponent*> Components;
				OutHit.GetActor()->GetComponents<USkeletalMeshComponent>(Components);
				USkeletalMeshComponent* MeshComponent = Components[0];

				float headSocketLocation = MeshComponent->GetSocketLocation(TEXT("headSocket")).Z;
				float middleSocketLocation = MeshComponent->GetSocketLocation(TEXT("middleSocket")).Z;
				// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(headSocketLocation));
				if (FireHeight < middleSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateLowerBody, OutHit.Location, FRotator::ZeroRotator);
				}
				else if (FireHeight >= middleSocketLocation && FireHeight <= headSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateUpperBody, OutHit.Location, FRotator::ZeroRotator);
				}
				else if (FireHeight > headSocketLocation)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateHead, OutHit.Location, FRotator::ZeroRotator);
					Score++;
				}
			}
		}
	}
}

void Ashokan_game0Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void Ashokan_game0Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void Ashokan_game0Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void Ashokan_game0Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void Ashokan_game0Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void Ashokan_game0Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void Ashokan_game0Character::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
