// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaGameStateBase.h"
#include "SpartaGameInstance.h"
#include "homework8/homework8Character.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"




ASpartaGameStateBase::ASpartaGameStateBase()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 30.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
}

void ASpartaGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUD();
	StartLevel();
	GetWorldTimerManager().SetTimer(HUDUpdateTimerHandle, this, &ASpartaGameStateBase::UpdateHUD, 0.1f , true);
}

int32 ASpartaGameStateBase::GetScore() const
{
	return Score;
}

void ASpartaGameStateBase::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddtoScore(Amount);
		}
	}

}

void ASpartaGameStateBase::StartLevel()
{
	Ahomework8Character* Character = Cast<Ahomework8Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Character)
	{
		Character->ShowGameHUD(); //  캐릭터에서 바로 HUD 실행
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);

		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	const int32 ItemToSpawn = 40;

	for (int32 i = 0; i < ItemToSpawn; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[FMath::RandRange(0, FoundVolumes.Num() - 1)]);//랜덤으로 스폰볼륨을 선택
			if (SpawnVolume)
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}
	}	
	GetWorldTimerManager().SetTimer(LevelTimerHandle, this, &ASpartaGameStateBase::OnLevelTimeup, LevelDuration, false);

}

void ASpartaGameStateBase::OnLevelTimeup()
{
	EndLevel();
}

void ASpartaGameStateBase::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Collected %d/%d coin"), CollectedCoinCount, SpawnedCoinCount);

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndLevel();
	}

}

void ASpartaGameStateBase::EndLevel()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	UpdateHUD();
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}


	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver();
		return;
	}
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
		UpdateHUD();
	}
	else
	{
		OnGameOver();
	}
}

void ASpartaGameStateBase::OnGameOver()
{
	Ahomework8Character* Character = Cast<Ahomework8Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Character)
	{
		Character->ShowMainMenu(true);
	}

}

void ASpartaGameStateBase::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (Ahomework8Character* Character = Cast<Ahomework8Character>(PlayerController->GetPawn()))
		{
			if (UUserWidget* HUDWidget = Character->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
						if (SpartaGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}

				}
				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
				}


			}
		}
	}
}

