// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeAmmo.generated.h"

UCLASS()
class SHOKAN_GAME0_API AGrenadeAmmo : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeAmmo();

	// �����񵯱�ը������Ч
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* EmitterTemplateAmmoBoom;	

	// �����񵯱�ը����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* AmmoBoomSound;

	// �����񵯱�ը�ڽ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* BoomAtMetalSound;

	// �����񵯱�ը�ڲݵ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* BoomAtGrassSound;

	// �����񵯱�ը��ľͷ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* BoomAtWoodSound;

	//���屬ը�����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class URadialForceComponent * RadialForce;

	UPROPERTY()
	class UStaticMeshComponent* StaticMeshComponent;

	// �����Ƿ��������
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bHitCharacter;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

};
