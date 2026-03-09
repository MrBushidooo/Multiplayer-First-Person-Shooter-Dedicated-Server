// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void PlayGame();

	UFUNCTION(BlueprintCallable)
	void CreateGameSession();

	UFUNCTION(BlueprintCallable)
	void JoinGameSession(const FString& SessionCode);

	void OnSuccessfulConnection();

	FString CurrentMatchCode = "";
	
private:

	//void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	//void OnFindSessionsComplete(bool bWasSuccessful);
	//void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void ConnectToServer(const FString& ServerURL);
	void OnConnectionTimeout();

	void HandleNetworkFailure(
		UWorld* World,
		UNetDriver* NetDriver,
		ENetworkFailure::Type FailureType,
		const FString& ErrorString);

	void HandleTravelFailure(
		UWorld* World,
		ETravelFailure::Type FailureType,
		const FString& ErrorString);

	void CleanupFailedConnection(const FString& Reason);

	

private:

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY()
	bool bConnectionInProgress = false;

	FTimerHandle ConnectionTimeoutHandle;

	UPROPERTY(EditDefaultsOnly)
	float ConnectionTimeoutSeconds = 10.f;
};
