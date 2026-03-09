// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "MyPlayerController.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AMyGameState::AMyGameState()
{
    CountdownDuration = 10.f;
    CountdownStartTime = -1.f;
}

void AMyGameState::BeginPlay() {

	Super::BeginPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("MyGameState::BeginPlay called"));

    //server reset player scores
    if (HasAuthority()) {
        for (APlayerState* PS : PlayerArray)
        {
            if (AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS))
            {
                MyPS->PlayerScore = 0;

            }
        }
    }

}


void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyGameState, RoundNumber);
    DOREPLIFETIME(AMyGameState, bMatchOver);
    DOREPLIFETIME(AMyGameState, bRoundInProgress);
    DOREPLIFETIME(AMyGameState, bRoundEnding);
    DOREPLIFETIME(AMyGameState, WinnerPlayer);
    DOREPLIFETIME(AMyGameState, CountdownTime);
    DOREPLIFETIME(AMyGameState, RoundCountdownTime);
    DOREPLIFETIME(AMyGameState, CountdownStartTime);
}

//whenever round number changes, called on clients
void AMyGameState::OnRep_RoundNumber()
{
    for (APlayerController* PC : TActorRange<APlayerController>(GetWorld()))
    {
        if (AMyPlayerController* MPC = Cast<AMyPlayerController>(PC)) 
        {
            MPC->UpdateRoundUI(RoundNumber);
        }
    }
}

void AMyGameState::OnRep_CountdownStartTime()
{
	GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, TEXT("OnRep_CountdownStartTime called"));
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (PC->IsLocalController())
            {
                if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
                {
                    MyPC->IdleToStartTransition();
                }
            }
        }
    }
}

void AMyGameState::OnRep_CountdownTime()
{
    for (APlayerController* PC : TActorRange<APlayerController>(GetWorld()))
    {
        if (AMyPlayerController* MPC = Cast<AMyPlayerController>(PC))
        {
            if (MPC->MatchInfoWidget)
            {
                MPC->MatchInfoWidget->SetCountdownTime(CountdownTime);
                MPC->MatchInfoWidget->PlayTimerTextAnimation();
				
                
            }
        }
	}
}

void AMyGameState::OnRep_RoundCountdownTime()
{
    for(AMyPlayerController* PC :TActorRange<AMyPlayerController>(GetWorld()))
    {
        if (AMyPlayerController* MPC = Cast<AMyPlayerController>(PC))
        {
            if (MPC->MatchInfoWidget)
            {
                MPC->MatchInfoWidget->SetRoundCountdownTime(RoundCountdownTime);
                //MPC->MatchInfoWidget->PlayRoundTimerTextAnimation();
            }
        }
	}
}


