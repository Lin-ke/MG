#include "PooledObject.h"

UPoolableComponent::UPoolableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	OnPoolReturn(); /// 注意如果放到beginplay中有初始化顺序的问题。
}

void UPoolableComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UPoolableComponent::OnPoolSpawn()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorHiddenInGame(false);
		Owner->PrimaryActorTick.bCanEverTick = true;
	}

	// 广播事件，以便蓝图可以响应
	OnSpawnedFromPool.Broadcast();
}

void UPoolableComponent::OnPoolReturn()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->PrimaryActorTick.bCanEverTick = false;
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorLocation(FVector(0, 0, -20000.0));
	}

	// 广播事件
	OnReturnedToPool.Broadcast();
}