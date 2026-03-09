// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchEndScreenWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;
class UButton;

UCLASS()
class GULAG_API UMatchEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:



public:

	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable)
	void ShowWinScreen();

	UFUNCTION(BlueprintCallable)
	void ShowLoseScreen();

	UFUNCTION(BlueprintImplementableEvent)
	void SetWinBGColor();

	UFUNCTION(BlueprintImplementableEvent)
	void SetLoseBGColor();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayEndScreenAnimation();

	UFUNCTION()
	void OnMainMenuButtonClicked();

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;
};
