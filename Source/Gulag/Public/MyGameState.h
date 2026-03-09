// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyPlayerState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API AMyGameState : public AGameState
{
	GENERATED_BODY()
	
public:
    AMyGameState();

	virtual void BeginPlay() override;
    
	//Match Variables
    UPROPERTY(ReplicatedUsing = OnRep_RoundNumber)
    int32 RoundNumber = 0;

    UFUNCTION()
    void OnRep_RoundNumber();

    //Server time when countdown started 
    UPROPERTY(ReplicatedUsing = OnRep_CountdownStartTime)
    float CountdownStartTime = -1.f;

    UFUNCTION()
    void OnRep_CountdownStartTime();

    //Total countdown duration (seconds) 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float CountdownDuration = 10.f;

    UPROPERTY(ReplicatedUsing = OnRep_CountdownTime)
    int32 CountdownTime = 0;

    UPROPERTY(ReplicatedUsing = OnRep_RoundCountdownTime)
    int32 RoundCountdownTime = 0;

    UFUNCTION()
    void OnRep_CountdownTime();

    UFUNCTION()
    void OnRep_RoundCountdownTime();

    //game states
    UPROPERTY(Replicated)
    bool bRoundInProgress = false;

    UPROPERTY(Replicated)
    bool bRoundEnding = false;

    UPROPERTY(Replicated)
    bool bMatchOver = false;

    
    UPROPERTY(Replicated)
    AMyPlayerState* WinnerPlayer = nullptr;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
};
