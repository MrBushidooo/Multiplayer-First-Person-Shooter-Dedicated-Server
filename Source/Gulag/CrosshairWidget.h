// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class GULAG_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowCrosshair(bool show);
	
};
