// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrenadeBase.h"
#include "FlashGrenade.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API AFlashGrenade : public AGrenadeBase
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Flashbang")
	float FlashRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Flashbang")
	float MaxFlashDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Flashbang")
	float MinFlashDuration = 0.5f;

	virtual void Explode() override;

	UPROPERTY(EditDefaultsOnly, Category = "Flashbang")
	float MinFacingDot = 0.3f;   

	UPROPERTY(EditDefaultsOnly, Category = "Flashbang")
	TEnumAsByte<ECollisionChannel> VisibilityChannel = ECC_Visibility;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopTrailVFX();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StopTrailVFX();
		
};
