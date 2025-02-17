// Fill out your copyright notice in the Description page of Project Settings.


#include "HealingItem.h"
#include "homework8/homework8Character.h"

AHealingItem::AHealingItem()
{
	HealAmount = 20;
	ItemType = "Healing";
}

void AHealingItem::ActivateItem(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (Ahomework8Character* PlayerCharacter = Cast<Ahomework8Character>(Activator))
		{
			PlayerCharacter->AddHealth(HealAmount);
		}
		DestroyItem();
	}
}