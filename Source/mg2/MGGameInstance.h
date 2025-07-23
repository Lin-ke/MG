#pragma once
#include "CharacterBuff.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"

#include "MGGameInstance.generated.h"


USTRUCT(BlueprintType)
struct MG2_API FBuffShopItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	FBuffShopItemData(const FText& ItemName, int32 Price, UTexture2D* Icon, const FText& Description, int Index)
	{
		this->ItemName = ItemName;
		this->Price = Price;
		this->Icon = Icon;
		this->Description = Description;
		this->Index = Index;
	}
	FBuffShopItemData()
	{
		
	}
	int Index = 0;  // use to locate CharacterBuff*
	
};



UCLASS()
class MG2_API UMGGameInstance : public UGameInstance
{
	GENERATED_BODY()

	public:
	UMGGameInstance();

	/// 金币系统

public:

	/**
	 * @brief 获取当前玩家的金币数量
	 * @return 玩家的金币数
	 */
	UFUNCTION(BlueprintPure, Category = "游戏数据 | 金币")
	int32 GetPlayerCoins() const;

	/**
	 * @brief 增加金币
	 * @param Amount 要增加的数量
	 */
	UFUNCTION(BlueprintCallable, Category = "游戏数据 | 金币")
	void AddCoins(int32 Amount);

	/**
	 * @brief 消费金币
	 * @param Amount 要消费的数量
	 * @return 如果金币足够则返回 true, 否则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = "游戏数据 | 金币")
	bool SpendCoins(int32 Amount);




public:
	//================================================================
	// Buff 系统 (Buff System)
	//================================================================
	
	
	/**
	 * @brief 购买一个Buff，将其ID添加到暂存列表
	 * @param BuffID 要购买的Buff的ID (对应数据表中的行名)
	 * @param Cost 购买所需的花费
	 * @return 如果购买成功返回 true
	 */
	UFUNCTION(BlueprintCallable, Category = "游戏数据 | Buff")
	bool PurchaseBuff(int item);

	/**
	 * @brief 获取所有已购买的Buff的ID列表
	 * @return Buff ID数组的常量引用
	 */
	const TArray<UCharacterBuff*>& GetPurchasedBuffs() const;

	/**
	 * @brief 清空已购买的Buff列表 (在进入新关卡并应用完Buff后调用)
	 */
	UFUNCTION(BlueprintCallable, Category = "游戏数据 | Buff")
	void ClearPurchasedBuffs();

	UFUNCTION(BlueprintCallable, Category = "游戏数据 | Buff")
	TArray<FBuffShopItemData> GetBuffsToBuy();
	
	float GoldCoinMultiplier() const;


	
public:
	//================================================================
	// 关卡切换 (Level Management)
	//================================================================

	/**
	 * @brief 初始化关卡流程，设置游戏关卡的顺序
	 */
	virtual void Init() override;

	/**
	 * @brief 前往商店关卡
	 */
	UFUNCTION(BlueprintCallable, Category = "关卡管理")
	void GoToShop();

	/**
	 * @brief 前往下一个游戏关卡
	 */
	UFUNCTION(BlueprintCallable, Category = "关卡管理")
	void GoToNextLevel();

	// 检查一个关卡是否已完成
	UFUNCTION(BlueprintPure, Category = "游戏数据 | 关卡进度")
	bool IsLevelCompleted(FName LevelID) const;

	// 将一个关卡标记为已完成
	UFUNCTION(BlueprintCallable, Category = "游戏数据 | 关卡进度")
	void CompleteLevel(FName LevelID);

protected:
	// 使用集合(Set)来存储已完成关卡的ID，查询效率高
	UPROPERTY(VisibleAnywhere, Category = "游戏数据 | 关卡进度")
	TSet<FName> CompletedLevels;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "关卡管理")
	FName ShopLevelName;

	// 游戏关卡的有序列表
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "关卡管理")
	TArray<FName> GameLevels;

	// 当前所在的关卡在GameLevels数组中的索引
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "关卡管理")
	int32 CurrentLevelIndex;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "关卡管理")
	int32 CurrentLevelHardness; /// 难度等级
	



	/////// 玩家数据
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "游戏数据")
	TArray<UCharacterBuff*> CharacterBuffs;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "游戏数据")
	int32 PlayerCoins = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "游戏数据")
	int32 PlayerLife = 3;
	
};
