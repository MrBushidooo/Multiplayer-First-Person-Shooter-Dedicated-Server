// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GrenadeBase.generated.h"

UCLASS(ABSTRACT)
class GULAG_API AGrenadeBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


    UPROPERTY(VisibleAnywhere)
    USphereComponent* CollisionComp;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, Category = "Grenade")
    float FuseTime = 3.f;

    FTimerHandle FuseTimerHandle;

    // for each grenade type
    virtual void Explode();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayExplosionFX();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
