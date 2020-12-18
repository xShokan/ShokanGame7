// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GrenadeAmmo.h"
#include "shokan_game0Character.generated.h"

USTRUCT()
struct FHitSanTrace {
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector_NetQuantize TraceTo;
	UPROPERTY()
	uint8 BurstCounter;
};

UCLASS(config=Game)
class Ashokan_game0Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	Ashokan_game0Character();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* HandWeapon;					//��������

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool isHip = true;																// �����Ƿ񿪾�

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isFire = false;																// �����Ƿ񿪻�

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* FireMontage;									// ���忪ǹ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimMontage* CutMontage;									// ���� sword attack ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;											// ���忪ǹ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound5;											// ����Gun5��ǹ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* CutSound;											// ���� sword attack ����

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int Score = 0;														            // ����÷�

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int KillNumber;														            // �����ɱ��

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int DeathNumber;														            // ����������

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int KindOfWeapon = 1;														// �����������࣬1ΪGun��2ΪSword

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int KindOfGun = 1;														// ����Gun����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")			
	class USkeletalMesh* GunMesh;						//��������Gun1��Mesh

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* SwordMesh;					//��������Sword��Mesh

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* Gun3Mesh;						//��������Gun3��Mesh

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* Gun4Mesh;						//��������Gun4��Mesh

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* Gun5Mesh;						//��������Gun5��Mesh

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	class USkeletalMesh* EmptyMesh;					//����յ�Mesh
//#if 0
//
//#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isFreeView = false;										// �����Ƿ��������ӽ�
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawning")
	TSubclassOf<AActor>  Gun1ToSpawn;										// ����Ҫ���ɵ�Gun1��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor>  Gun3ToSpawn;										// ����Ҫ���ɵ�Gun3��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor>  Gun4ToSpawn;										// ����Ҫ���ɵ�Gun4��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor>  Gun5ToSpawn;										// ����Ҫ���ɵ�Gun5��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor>  AmmoGun5ToSpawn;										// ����Ҫ���ɵ�Gun5�ӵ���

	UPROPERTY()
	AActor* Gun5AmmoActor;										// ����Ҫ���ɵ�Gun5�ӵ�

	UPROPERTY(BlueprintReadWrite)
	AActor* OverlappingWeaponActor;							// ����overlapping��

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEquipWeapon = true;										// �����Ƿ�������

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPressF = false;										// �����Ƿ���ʾPressF UMG

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowYouEquipWeapon = false;										// �����Ƿ���ʾYou already have equipped a weapon!

	UPROPERTY(BlueprintReadWrite)
	int OverlappingKindOfWeapon = 0;										// ����overlapping��������, 1��Ĭ��Gun, 2�ǽ�(�Ȳ�����), ����Ϊ����Gun

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="HitParticle")
	class UParticleSystem* EmitterTemplateLowerBody;										     // ������°���������Ч

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitParticle")
	class UParticleSystem* EmitterTemplateUpperBody;										     // ������°���������Ч

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitParticle")
	class UParticleSystem* EmitterTemplateHead;										     // �����ͷ������Ч

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "HitParticle")
	class UParticleSystem* EmitterTemplateGun5Fire;										     // ����Gun5����������Ч

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bDied;							// ��������Ƿ�����

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bAlive;							// ��������Ƿ񸴻�

	UPROPERTY(Replicated, BlueprintReadWrite)
	float IronPitch;

	UPROPERTY(Replicated, BlueprintReadWrite)
	float IronYaw;

	/** RepNotify������ͬ���Ե�ǰ����ֵ�����ĸ��ġ�*/
	UFUNCTION()
	void OnRep_YawPitch();

	/** ��ҵĵ�ǰ����ֵ������0�ͱ�ʾ������*/
	UPROPERTY(Replicated, BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bGameOver;							// ������Ϸ�Ƿ����

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** ��ҵ��������ֵ��������ҵ��������ֵ��Ҳ�ǳ���ʱ������ֵ��*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitSanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface


public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void IronsightDown();
	void IronsightUp();

	void Fire();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerIronsightDown();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerIronsightUp();

	UFUNCTION(Server, Reliable, WithValidation)
	void CheckHitCharacter();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetHealth();

	void FireDown();
	void FireUp();

	void RunDown();
	void RunUp();

	void SwitchGun();
	void SwitchSword();

	void FreeViewDown();
	void FreeViewUp();

	void DropWeaponDown();
	void SpawnObject(TSubclassOf<AActor> ToSpawn);

	void PickWeaponDown();

	void GameOver();

	void FireSpawn();

	AActor* SpawnAmmo(TSubclassOf<AActor> ToSpawn);

	FHitResult OutHit;

	/** ���Ը��� */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** �����˺����¼�����APawn���ǡ�*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** �������ֵ��ȡֵ������*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** ��ǰ����ֵ��ȡֵ������*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
};

