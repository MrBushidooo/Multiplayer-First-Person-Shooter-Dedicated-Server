// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	

public:

	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerScore)
	int32 PlayerScore = 0;

	UFUNCTION()
	void OnRep_PlayerScore();

	// Helper to modify score
	void AddScore(int32 Amount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool isDead = false;
};
