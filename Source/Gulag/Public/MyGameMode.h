// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyPlayerState.h"
#include "GameLiftServerSDK.h"
#include "MyGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameServerLog, Log, All);

UCLASS()
class GULAG_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()
	

private:

	void InitGameLift();

private:

	TSharedPtr<FProcessParameters> ProcessParameters;

	UPROPERTY()
	TMap<APlayerController*, FString> PlayerSessionMap;

	// Temporary storage - Player session id between prelogin and postlogin)
	TMap<FUniqueNetIdRepl, FString> PendingPlayerSessions;

protected:
	//int32 ConnectedPlayers = 0;
	int32 RequiredPlayers = 2;
	bool bCountdownStarted = false;

public:

	AMyGameMode();
	virtual void BeginPlay() override;
	virtual void PreLogin(const FString& Options, const FString& Address, 
		                  const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	

	UFUNCTION()
	void PlayerDied(AController* KillerController, AController* VictimController);

	UFUNCTION()
	void RequestRespawn(AController* Controller);
	void RespawnNextRound();

	void StartPreRoundCountdown();
	void TickCountdown();

	void StartRoundCountdown();
	void RoundTickCountdown();

	void StartRound();
	void EndRound();

	void EndMatch(AMyPlayerState* WinnerPlayerState);
	void CheckWinCondition();
	void CheckClosestPlayer();
	
	int32 count = 0;

	//match checks
	// 
	//UPROPERTY()
	//bool bRoundEnding = false;

	//UPROPERTY()
	//bool bMatchEnding = false;

	FTimerHandle RoundCountdownTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle StartNextRoundTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	AActor* MapCenterPoint;

};
