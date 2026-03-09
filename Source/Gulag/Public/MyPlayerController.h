// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MatchInfoWidget.h"
#include "MatchEndScreenWidget.h"
#include "GulagCharacter.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

    virtual void BeginPlay() override;

	//UI definitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMatchInfoWidget> MatchInfoWidgetClass;

    UPROPERTY()
    UMatchInfoWidget* MatchInfoWidget;

    //UI Updates
    UFUNCTION(BlueprintCallable)
    void UpdateRoundUI(int32 NewRound);

    UFUNCTION(BlueprintCallable)
    void UpdateScoreUI();

    UFUNCTION(Client, Reliable)
    void Client_IdleToStartTransition();

    UFUNCTION()
	void IdleToStartTransition();
	
    //Win-lose UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMatchEndScreenWidget> MatchEndScreenWidgetClass;

    UPROPERTY()
    UMatchEndScreenWidget* MatchEndScreenWidget;


    UFUNCTION(Client, Reliable)
	void Client_HandleMatchEnd(bool bIsWinner);

    //Apply reveal effect to revealed player in the owning client window
    UFUNCTION(Client, Reliable)
    void Client_RevealActor(AGulagCharacter* RevealedActor, float Duration);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Client_RevealActor(AGulagCharacter* RevealedActor, float Duration);

public:

    UFUNCTION(BlueprintCallable)
    void ReturnToMainMenuLevel();

    UFUNCTION(Client, Reliable)
    void Client_ReturnToMainMenuLevel();

    void CleanUpBeforeLeaving();
	
};
