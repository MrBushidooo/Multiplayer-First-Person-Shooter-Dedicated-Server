// Fill out your copyright notice in the Description page of Project Settings.



#include "MyPlayerController.h"
#include "MyGameInstance.h"
#include "MyGameState.h"
#include "GulagCharacter.h"
#include "Blueprint/UserWidget.h"
#include "MatchInfoWidget.h"
#include "MatchEndScreenWidget.h"
#include "GameFramework/GameStateBase.h"


void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("MyPlayerController::BeginPlay called"));

    if (UMyGameInstance* GI = GetGameInstance<UMyGameInstance>())
    {
        GI->OnSuccessfulConnection();
    }

    if (IsLocalController() && MatchInfoWidgetClass)
    {
        MatchInfoWidget = CreateWidget<UMatchInfoWidget>(this, MatchInfoWidgetClass);
        if (MatchInfoWidget)
        {
            MatchInfoWidget->AddToViewport();
            UpdateScoreUI(); 
			MatchInfoWidget->ShowCountdownText(false);
            
            UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
            if (GI) {
				MatchInfoWidget->UpdateMatchCode_Text(GI->CurrentMatchCode);
				MatchInfoWidget->ShowMatchCode_Text(true);
            }

			IdleToStartTransition();
			//MatchInfoWidget->PlayStartScreenAnimation();
        }
    }

    /*else {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("problem in player controller"));
    }*/

    //Bind to GameState replication
    if (AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>())
    {
        //GS->OnRep_RoundNumber();  //manually trigger once
        //GS->OnRep_Scores();
		UpdateRoundUI(GS->RoundNumber);
    }

    if (IsLocalController())
    {
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);

        bShowMouseCursor = false;
    }
}

void AMyPlayerController::UpdateRoundUI(int32 NewRound)
{
    if (MatchInfoWidget)
    {
        MatchInfoWidget->SetRoundNumber(NewRound);
        
    }
}

void AMyPlayerController::UpdateScoreUI()
{
    if (!MatchInfoWidget) return;

    //get both player scores
    AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
    if (!GS) return;

    int32 P1Score = 0;
    int32 P2Score = 0;

    
    for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
    {
        if (AMyPlayerState* PS = Cast<AMyPlayerState>(GS->PlayerArray[i]))
        {
            if (i == 0) P1Score = PS->PlayerScore;
            if (i == 1) P2Score = PS->PlayerScore;
        }
    }

    MatchInfoWidget->UpdateScore(P1Score, P2Score);
}

void AMyPlayerController::Client_IdleToStartTransition_Implementation()
{
    IdleToStartTransition();
}

void AMyPlayerController::IdleToStartTransition()
{
    //GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Green, TEXT("IdleToStartTransition called"));

    if (!MatchInfoWidget) {
        GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, TEXT("MatchInfoWidget NULL"));
        return;
    }

    AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
    if (!GS || GS->CountdownStartTime < 0.f) {
		GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, TEXT("CountdownStartTime invalid"));
        return;
    }
        

    float ServerTime = GS->GetServerWorldTimeSeconds();
    float Elapsed = ServerTime - GS->CountdownStartTime;

    Elapsed = FMath::Clamp(Elapsed, 0.f, GS->CountdownDuration);

    MatchInfoWidget->PlayStartScreenAnimation(Elapsed);
    MatchInfoWidget->ShowCountdownText(true);
	MatchInfoWidget->ShowMatchCode_Text(false);

}

void AMyPlayerController::ReturnToMainMenuLevel()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Returning to Main Menu Level..."));

	CleanUpBeforeLeaving();

    FTimerHandle ReturnTimer;
    GetWorldTimerManager().SetTimer(
        ReturnTimer, [this]() {
            ClientTravel(TEXT("Game/Maps/MainMenuLevel"), TRAVEL_Absolute);
        },
        2.f,
        false
    );

}

void AMyPlayerController::Client_ReturnToMainMenuLevel_Implementation()
{
    ReturnToMainMenuLevel();
}

void AMyPlayerController::CleanUpBeforeLeaving()
{
}



/*void AMyPlayerController::UpdateScoreUI()
{
    if (!MatchInfoWidget) return;

    AMyGameState* GS = GetWorld()->GetGameState<AMyGameState>();
    if (!GS) return;

    AMyPlayerState* MyPS = GetPlayerState<AMyPlayerState>();
    if (!MyPS) return;

    int32 MyScore = MyPS->PlayerScore;
    int32 OtherScore = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AMyPlayerState* OtherPS = Cast<AMyPlayerState>(PS);
        if (!OtherPS) continue;

        if (OtherPS != MyPS)
        {
            OtherScore = OtherPS->PlayerScore;
            break; // only 1 opponent in 1v1
        }
    }

    // You control presentation here:
    // Left = Me, Right = Opponent (or vice-versa)
    MatchInfoWidget->UpdateScore(MyScore, OtherScore);
}
*/

void AMyPlayerController::Client_HandleMatchEnd_Implementation(bool bIsWinner)
{
    if (MatchEndScreenWidgetClass) {

        MatchEndScreenWidget = CreateWidget<UMatchEndScreenWidget>(this, MatchEndScreenWidgetClass);
		MatchEndScreenWidget->AddToViewport();

        if(bIsWinner) {
            MatchEndScreenWidget->ShowWinScreen();
        } else {
            MatchEndScreenWidget->ShowLoseScreen();
		}

    }

    //input mode to UI only
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(MatchEndScreenWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    bShowMouseCursor = true;

    // Disable local input on the client so player cannot move/look after match end
   
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);

    //disable the pawn's input components 
    if (APawn* P = GetPawn())
    {
        P->DisableInput(this);
    }
}

void AMyPlayerController::Client_RevealActor_Implementation(AGulagCharacter* RevealedActor, float Duration)
{
	BP_Client_RevealActor(RevealedActor, Duration);
}


