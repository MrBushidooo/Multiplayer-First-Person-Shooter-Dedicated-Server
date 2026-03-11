// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeBase.h"


// Sets default values
AGrenadeBase::AGrenadeBase()
{
 	
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetSimulatePhysics(true);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

}

void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            FuseTimerHandle,
            this,
            &AGrenadeBase::Explode,
            FuseTime,
            false
        );
    }
	
}


void AGrenadeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AGrenadeBase::Explode()
{
    if (!HasAuthority())
        return;

    Multicast_PlayExplosionFX();
    Destroy();
}

void AGrenadeBase::Multicast_PlayExplosionFX_Implementation()
{
   //tbd
}
