#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpartaGameStateBase.generated.h"

UCLASS()
class HOMEWORK8_API ASpartaGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:

	ASpartaGameStateBase();

	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 SpawnedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 CollectedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	float LevelDuration;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;


	FTimerHandle LevelTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetScore() const;
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Score")
	void OnGameOver();

	void StartLevel();
	void OnLevelTimeup();
	void OnCoinCollected();
	void EndLevel();
	void UpdateHUD();


};