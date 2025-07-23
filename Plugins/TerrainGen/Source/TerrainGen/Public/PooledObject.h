#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PooledObject.generated.h"

// 声明一个委托，当组件被池化或取出时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPoolableStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAINGEN_API UPoolableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPoolableComponent();

	/** 当Actor从池中取出并激活时调用 */
	void OnPoolSpawn();

	/** 当Actor返回池中并失活时调用 */
	void OnPoolReturn();

	/** 当Actor从池中取出时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Pooling")
	FOnPoolableStateChanged OnSpawnedFromPool;

	/** 当Actor返回池中时广播 */
	UPROPERTY(BlueprintAssignable, Category = "Pooling")
	FOnPoolableStateChanged OnReturnedToPool;

protected:
	virtual void BeginPlay() override;
};