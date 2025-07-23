#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolWorldSubsystem.generated.h"

class UPoolableComponent; // 向前声明

UCLASS(Config=Game)
class TERRAINGEN_API UObjectPoolWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (DeterminesOutputType = "ActorClass"))
	void SetPoolConfigs(TMap<UClass*, int32>& PoolConfigs, TMap<UClass*, FString>& Tags);
	/** 从池中获取一个Actor。Actor必须有UPoolableComponent。*/
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (DeterminesOutputType = "ActorClass"))
	AActor* GetPooledObject(TSubclassOf<AActor> ActorClass);

	
	AActor* GetRandomPooledObject(const FString& Tag = "");

	/** 将一个Actor返回到池中。 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnPooledObject(AActor* ActorToReturn);

	bool Available(){return bAvaliable;}
private:
	TMap<UClass*, TArray<AActor*>> PooledObjects;

	TMap<UClass*, TArray<AActor*>> PopObjects;

	TMap<FString, TArray<UClass*>> ClassTags;

	TArray<UClass*> PooledObjectClasses;

	TMap<UClass*, int32> PoolConfigs;

	FTimerHandle CollectPopTimerHandle; 

	void PrewarmPool();
	void CollectPop();

	bool bAvaliable = false;
};


