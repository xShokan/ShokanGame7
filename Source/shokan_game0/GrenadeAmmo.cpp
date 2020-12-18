// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeAmmo.h"
#include <Kismet/GameplayStatics.h>
#include <PhysicsEngine/RadialForceComponent.h>
#include "shokan_game0Character.h"

// Sets default values
AGrenadeAmmo::AGrenadeAmmo()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//TArray<URadialForceComponent*> RadialComponents;
	//this->GetComponents<URadialForceComponent>(RadialComponents);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(RadialComponents.Num()));
	//for (int32 i = 0; i < RadialComponents.Num(); i++)
	//{
	//	RadialForce = RadialComponents[i];
	//}

	bHitCharacter = false;

	// 服务器产生指令
	SetReplicates(true);
	SetReplicateMovement(true);
}
	
// Called when the game starts or when spawned
void AGrenadeAmmo::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> Components;
	this->GetComponents<UStaticMeshComponent>(Components);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::FromInt(Components.Num()));
	for (int32 i = 0; i < Components.Num(); i++)
	{
		StaticMeshComponent = Components[i];
	}
}

// Called every frame
void AGrenadeAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, FString::SanitizeFloat(this->GetVelocity().Size()));
	if (GetLocalRole() == ROLE_Authority && GetVelocity().Size() < 0.01) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplateAmmoBoom, this->GetActorLocation(), FRotator::ZeroRotator);
		UGameplayStatics::PlaySoundAtLocation(this, AmmoBoomSound, GetActorLocation());
		// StaticMeshComponent->SetSimulatePhysics(false);
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, GetActorLocation().ToString());
		// StaticMeshComponent->AddRadialForce(GetActorLocation(), 500.0f, 20000.0f, ERadialImpulseFalloff::RIF_Constant);
		/*RadialForce->Radius = 200.0f;
		RadialForce->FireImpulse();*/
		GetWorld()->DestroyActor(this);
	}
}

void AGrenadeAmmo::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	Ashokan_game0Character *HitCharacter = Cast<Ashokan_game0Character>(Other);
	// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("123"));
	
	if (!HitCharacter)
	{
		FString MaterialName = OtherComp->GetMaterial(0)->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, MaterialName);
		if (this->GetVelocity().Size() > 120)
		{
			if (MaterialName == "M_MyMetal_Chrome0")
			{
				UGameplayStatics::PlaySoundAtLocation(this, BoomAtMetalSound, GetActorLocation());
			}
			else if (MaterialName == "MA_MyGrass0")
			{
				UGameplayStatics::PlaySoundAtLocation(this, BoomAtGrassSound, GetActorLocation());
				StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
			}
			else if (MaterialName == "MA_MyWood0")
			{
				UGameplayStatics::PlaySoundAtLocation(this, BoomAtWoodSound, GetActorLocation());
				StaticMeshComponent->SetPhysicsLinearVelocity(this->GetVelocity() * 1.7);
			}
		}
	}
	else {
		bHitCharacter = true;
		StaticMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	}

}

