// Fill out your copyright notice in the Description page of Project Settings.

#include "MatchEndScreenWidget.h"
#include "MyPlayerController.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"



bool UMatchEndScreenWidget::Initialize()
{
	Super::Initialize();

	if (MainMenuButton) {
		MainMenuButton->OnClicked.AddDynamic(this, &UMatchEndScreenWidget::OnMainMenuButtonClicked);
	}

	return true;
}

void UMatchEndScreenWidget::ShowWinScreen()
{
	SetWinBGColor();
	if(ResultText)
	{
		ResultText->SetText(FText::FromString("VICTORY"));
	}
	PlayEndScreenAnimation();
}

void UMatchEndScreenWidget::ShowLoseScreen()
{
	SetLoseBGColor();
	if(ResultText)
	{
		ResultText->SetText(FText::FromString("DEFEAT"));
	}
	PlayEndScreenAnimation();
}

void UMatchEndScreenWidget::OnMainMenuButtonClicked()
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (PC) {
		PC->ReturnToMainMenuLevel();
	}
}

