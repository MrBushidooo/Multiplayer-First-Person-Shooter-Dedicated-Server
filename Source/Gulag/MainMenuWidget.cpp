// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "MyGameInstance.h"

bool UMainMenuWidget::Initialize()
{
	Super::Initialize();

	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	}

	if (CreateSessionButton)
	{
		CreateSessionButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateSessionClicked);
	}

	if (JoinSessionButton)
	{
		JoinSessionButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinSessionClicked);
	}

	if (ExitButton) {
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}

	return true;
}


void UMainMenuWidget::OnPlayClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	RemoveFromParent();

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("OnPlayClicked()"));
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMyGameInstance* MyGI = Cast<UMyGameInstance>(GI))
		{
			MyGI->PlayGame();
		}
	}
}

void UMainMenuWidget::OnCreateSessionClicked()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMyGameInstance* MyGI = Cast<UMyGameInstance>(GI))
		{
			MyGI->CreateGameSession();
		}
	}
}

void UMainMenuWidget::OnJoinSessionClicked()
{
	if (!SessionCodeText)
	{
		UE_LOG(LogTemp, Error, TEXT("SessionCodeInput missing"));
		return;
	}

	const FString SessionCode = SessionCodeText->GetText().ToString();

	if (SessionCode.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session code empty"));
		//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, TEXT("Please enter a session code"));
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMyGameInstance* MyGI = Cast<UMyGameInstance>(GI))
		{
			MyGI->JoinGameSession(SessionCode);
		}
	}
}

void UMainMenuWidget::OnExitClicked()
{
	BP_OnExitClicked();
}


