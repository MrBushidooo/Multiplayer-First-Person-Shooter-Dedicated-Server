// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerWidget::UpdateHealthBar(float NewHealth, float MaxHealth)
{
	if (HealthBar) {
		HealthBar->SetPercent(NewHealth / MaxHealth);
	}
}

void UPlayerWidget::UpdateAmmoText(int32 NewAmmo)
{
	if (AmmoText) {
		AmmoText->SetText(FText::AsNumber(NewAmmo));
	}
}

