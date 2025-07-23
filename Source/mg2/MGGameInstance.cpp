// Copyright 2025, Your Name. All Rights Reserved.

#include "MGGameInstance.h"
#include "Kismet/GameplayStatics.h"

UMGGameInstance::UMGGameInstance()
{
	// 初始化默认值
	PlayerCoins = 0; // 可以给予玩家一些初始金币
	CurrentLevelIndex = -1; // -1 表示还未开始第一个关卡
	ShopLevelName = FName(TEXT("Map_Shop")); // 【重要】请将这里替换成你的商店地图名称
}

void UMGGameInstance::Init()
{
	Super::Init();

	/// Buff system initialize
	/// create buffs
	
}


//================================================================
// 金币系统
//================================================================

int32 UMGGameInstance::GetPlayerCoins() const
{
	return PlayerCoins;
}

void UMGGameInstance::AddCoins(int32 Amount)
{
	if (Amount > 0)
	{
		PlayerCoins += Amount * GoldCoinMultiplier();
	}
}

bool UMGGameInstance::SpendCoins(int32 Amount)
{
	if (Amount > 0 && PlayerCoins >= Amount)
	{
		PlayerCoins -= Amount;
		return true;
	}
	return false;
}

//================================================================
// Buff 系统
//================================================================

bool UMGGameInstance::PurchaseBuff(int item)
{
	return false;
}

const TArray<UCharacterBuff*>& UMGGameInstance::GetPurchasedBuffs() const
{
	return CharacterBuffs;
}

void UMGGameInstance::ClearPurchasedBuffs()
{
	CharacterBuffs.Empty();
}

TArray<FBuffShopItemData> UMGGameInstance::GetBuffsToBuy()
{
	TArray<FBuffShopItemData> ItemsToDisplay;
	ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff1"), 100, NULL, FText::FromString("123"),1));
	ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff2"), 100, NULL, FText::FromString("123"),2));
	ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff3"), 100, NULL, FText::FromString("123"),3));

	return ItemsToDisplay;
}

float UMGGameInstance::GoldCoinMultiplier() const
{
	return 1; 
}

//================================================================
// 关卡切换
//================================================================

void UMGGameInstance::GoToShop()
{
	// UE_LOG(LogTemp, Log, TEXT("Transitioning to shop level: %s"), *ShopLevelName.ToString());
	UGameplayStatics::OpenLevel(GetWorld(), ShopLevelName);
}

void UMGGameInstance::GoToNextLevel()
{
	// 增加关卡索引
	CurrentLevelIndex++;

	// 检查是否还有下一关
	if (GameLevels.IsValidIndex(CurrentLevelIndex))
	{
		FName NextLevelToLoad = GameLevels[CurrentLevelIndex];
		// UE_LOG(LogTemp, Log, TEXT("Transitioning to next level: %s"), *NextLevelToLoad.ToString());
		UGameplayStatics::OpenLevel(GetWorld(), NextLevelToLoad);
	}
	else
	{
		// 所有关卡已完成
		// UE_LOG(LogTemp, Warning, TEXT("All levels completed! Returning to main menu or showing credits."));
		// 在这里可以添加返回主菜单或显示制作人员名单的逻辑
		// UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Map_MainMenu")));
	}
}

bool UMGGameInstance::IsLevelCompleted(FName LevelID) const
{
	return CompletedLevels.Contains(LevelID);
}

void UMGGameInstance::CompleteLevel(FName LevelID)
{
	if (!LevelID.IsNone())
	{
		CompletedLevels.Add(LevelID);
	}
}
