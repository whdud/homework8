// Fill out your copyright notice in the Description page of Project Settings.


#include "CoinItem.h"
#include "Engine/World.h"
#include "SpartaGameStateBase.h"

ACoinItem::ACoinItem()
{
	PointValue = 0;
}

void ACoinItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (UWorld* World = GetWorld())
		{
			if (ASpartaGameStateBase* GameStateBase = GetWorld()->GetGameState<ASpartaGameStateBase>())
			{
				GameStateBase->AddScore(PointValue);
				GameStateBase->OnCoinCollected();
			}
	
		}
		DestroyItem();
	}

}