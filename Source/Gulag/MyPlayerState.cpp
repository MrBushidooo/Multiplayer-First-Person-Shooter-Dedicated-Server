// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "MyPlayerController.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"


/*AMyPlayerState::AMyPlayerState()
{
   
}*/



void AMyPlayerState::BeginPlay()
{
}




void AMyPlayerState::AddScore(int32 Amount)
{
    if (HasAuthority())
    {
        PlayerScore += Amount;
        OnRep_PlayerScore(); // notify locally on server
    }
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMyPlayerState, PlayerScore);
}


void AMyPlayerState::OnRep_PlayerScore()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("PlayerScore updated to %d"), PlayerScore));
    for (APlayerController* PC : TActorRange<APlayerController>(GetWorld()))
    {
        if (AMyPlayerController* MPC = Cast<AMyPlayerController>(PC))
        {
            MPC->UpdateScoreUI();
        }
    }
}