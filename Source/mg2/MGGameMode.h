#pragma once
#include "CoreMinimal.h"
#include "CharacterBuff.h"
#include "GameFramework/GameModeBase.h"
#include "MGGameMode.generated.h"

class ATerrainGenActor;


USTRUCT()
struct FPooledBarrierConfigItem 
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	UClass* BarrierClass;
	UPROPERTY(EditAnywhere)

	FString Tag;
	UPROPERTY(EditAnywhere)

	int count;
};

USTRUCT()
struct FBuffConfig 
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCharacterBuff> BuffClass;
	UPROPERTY(EditAnywhere)
	float Random;
};
// 向前声明我们的地形类

UCLASS()
class MG2_API AMGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 构造函数
	AMGGameMode();
	~AMGGameMode(){}

protected:
	// 当游戏开始时，这个函数会被引擎自动调用
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	/// Slot. (generate offline)
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TSubclassOf<ATerrainGenActor> TerrainToSpawn;
	
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TSubclassOf<ATerrainGenActor> TerrainToSpawn1;
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TSubclassOf<ATerrainGenActor> TerrainToSpawn2;
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TSubclassOf<ATerrainGenActor> TerrainToSpawn3;
	
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TSubclassOf<ATerrainGenActor> TerrainCliff;

	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	float CliffProb = 0.4;
	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	TArray<FPooledBarrierConfigItem> PoolConfig;

	UPROPERTY(EditDefaultsOnly, Category = "World Generation")
	float PawnMaxSpeed = 5000;

	UPROPERTY(EditDefaultsOnly, Category = "Buff System")
	TArray<FBuffConfig> BuffConfig;
};
