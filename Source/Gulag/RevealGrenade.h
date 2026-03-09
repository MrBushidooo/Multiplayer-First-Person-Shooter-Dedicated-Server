// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrenadeBase.h"
#include "GulagCharacter.h"
#include "RevealGrenade.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API ARevealGrenade : public AGrenadeBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Reveal")
	float RevealRadius = 1400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reveal")
	float RevealDuration = 4.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Reveal")
	float ScanDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Reveal")
	float ScanRate = 0.05f;

	float CurrentScanRadius = 0.f;

	FTimerHandle ScanTickHandle;

	//avoid revealing same player multiple times
	TSet<TWeakObjectPtr<AGulagCharacter>> AlreadyRevealed;

	virtual void Explode() override;

	void PerformRevealScan();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPulseVFX(FVector ExplosionLocation);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayPulseVFX(FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopTrailVFX();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StopTrailVFX();
};
