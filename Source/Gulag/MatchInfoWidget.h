// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchInfoWidget.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class GULAG_API UMatchInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION(BlueprintCallable)
	void SetRoundNumber(int32 NewRound);

	UFUNCTION(BlueprintCallable)
	void SetCountdownTime(int32 NewTime);

	UFUNCTION(BlueprintCallable)
	void SetRoundCountdownTime(int32 NewTime);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayTimerTextAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayStartScreenAnimation(float StartAtTime);

	UFUNCTION(BlueprintImplementableEvent)
	void CountdownFinished();

	UFUNCTION()
	void ShowCountdownText(bool bshow);

	UFUNCTION()
	void ShowMatchCode_Text(bool bshow);

	UFUNCTION(BlueprintCallable)
	void UpdateMatchCode_Text(FString Match_Code);

	UFUNCTION(BlueprintCallable)
	void UpdateScore(int32 Player1Score, int32 Player2Score);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayScoreUpdateSound();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_PlayRoundNumberAnimation();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoundNumberText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountdownTime_Text;

	UPROPERTY(meta= (BindWidget))
	UTextBlock* RoundCountdownTime_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player1_ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player2_ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCode_Text;

};
