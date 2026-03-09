// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class GULAG_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	UButton* PlayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateSessionButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinSessionButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SessionCodeText;

	//UFUNCTION()
	//void SetSessionCodeText(const FString& Code);
	
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnCreateSessionClicked();

	UFUNCTION()
	void OnJoinSessionClicked();

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnExitClicked();
};
