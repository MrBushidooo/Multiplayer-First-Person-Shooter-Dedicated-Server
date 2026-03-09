// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;

UCLASS()
class GULAG_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION(BlueprintCallable)
	void UpdateHealthBar(float NewHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable)
	void UpdateAmmoText(int32 NewAmmo);

protected:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

public:

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnFlashbangsFinished();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRevealsFinished();


};

