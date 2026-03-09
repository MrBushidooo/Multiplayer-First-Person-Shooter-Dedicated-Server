// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchInfoWidget.h"
#include "Components/TextBlock.h"

void UMatchInfoWidget::SetRoundNumber(int32 NewRound)
{
	if (RoundNumberText) {
		RoundNumberText->SetText(FText::AsNumber(NewRound));
		BP_PlayRoundNumberAnimation();
	}
}

void UMatchInfoWidget::SetCountdownTime(int32 NewTime)
{
	if (!CountdownTime_Text) return;

	if (NewTime <= 0)
	{
		CountdownTime_Text->SetVisibility(ESlateVisibility::Hidden);
		CountdownFinished();
	}
	else
	{
		CountdownTime_Text->SetVisibility(ESlateVisibility::Visible);
		CountdownTime_Text->SetText(FText::AsNumber(NewTime));
	}
}

void UMatchInfoWidget::SetRoundCountdownTime(int32 NewTime)
{
	if (!RoundCountdownTime_Text) return;

    RoundCountdownTime_Text->SetText(FText::AsNumber(NewTime));
}

void UMatchInfoWidget::ShowCountdownText(bool bshow)
{
	if (CountdownTime_Text) {
		if (bshow) {
			CountdownTime_Text->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			CountdownTime_Text->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMatchInfoWidget::ShowMatchCode_Text(bool bshow)
{
	if (MatchCode_Text) {
		if (bshow) {
			MatchCode_Text->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			MatchCode_Text->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


void UMatchInfoWidget::UpdateMatchCode_Text(FString Match_Code)
{
	if (MatchCode_Text)
	{
		MatchCode_Text->SetText(
			FText::FromString(FString::Printf(TEXT("Match Code: %s"), *Match_Code))
		);
	}
}

void UMatchInfoWidget::UpdateScore(int32 Player1Score, int32 Player2Score)
{
	if (Player1_ScoreText) {
		Player1_ScoreText->SetText(FText::AsNumber(Player1Score));
	}
	if (Player2_ScoreText) {
		Player2_ScoreText->SetText(FText::AsNumber(Player2Score));
	}
	PlayScoreUpdateSound();
}
