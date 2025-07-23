#include "ObjectPoolWorldSubsystem.h"
#include "PooledObject.h" // 包含新组件的头文件
#include "Engine/World.h"

// Initialize 和 Deinitialize 函数与之前的版本相同，无需修改

void UObjectPoolWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

	
}

void UObjectPoolWorldSubsystem::Deinitialize()
{
    PooledObjects.Empty(); /// clean.
	PopObjects.Empty();
	GetWorld()->GetTimerManager().ClearTimer(CollectPopTimerHandle);
    Super::Deinitialize();
}

void UObjectPoolWorldSubsystem::SetPoolConfigs(TMap<UClass*, int32>& p_PoolConfigs, TMap<UClass*, FString>& Tags)
{
	if (!p_PoolConfigs.Num()) return;
	PoolConfigs.Empty();
	PooledObjects.Empty();
	PooledObjectClasses.Empty();
	PoolConfigs = p_PoolConfigs;
	for (auto&[key, value] : p_PoolConfigs)
	{
		PooledObjectClasses.Add(key);
	}
	for (auto&[k, v] : Tags)
	{
		auto& p = ClassTags.FindOrAdd(v);
		p.AddUnique(k);
	}
	
	PrewarmPool();
	bAvaliable = true;
	GetWorld()->GetTimerManager().SetTimer(
		CollectPopTimerHandle, 
		this, 
		&UObjectPoolWorldSubsystem::CollectPop, 
		5.0f, 
		true 
	);

}

void UObjectPoolWorldSubsystem::PrewarmPool()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (const auto& Config : PoolConfigs)
	{
		UClass* LoadedClass = Config.Key;
		if (LoadedClass)
		{
			AActor* DefaultActor = LoadedClass->GetDefaultObject<AActor>();
			// 检查默认对象（CDO）是否包含我们的组件
			if (DefaultActor && DefaultActor->FindComponentByClass<UPoolableComponent>())
			{
				TArray<AActor*>& Pool = PooledObjects.FindOrAdd(LoadedClass);
				Pool.Reserve(Config.Value);
				for (int32 i = 0; i < Config.Value; ++i)
				{
					AActor* NewObj = World->SpawnActor<AActor>(LoadedClass);
					if (NewObj)
					{
						Pool.Add(NewObj);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Class %s is configured in ObjectPool but does not have a UPoolableComponent. It will be skipped."), *LoadedClass->GetName());
			}
		}
	}
}

void UObjectPoolWorldSubsystem::CollectPop()
{
	// 获取玩家Pawn，如果获取失败则直接返回
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return;

	const float MaxDistance = 10000.0f;
	const auto NeedCollect = [MaxDistance](const FVector& PawnLocation, const FVector& ActorLocation)
	{
		// 条件：Actor在玩家身后，并且与玩家的距离大于设定的最大距离
		return PawnLocation.X > ActorLocation.X && FVector::DistSquared(ActorLocation, PawnLocation) > FMath::Square(MaxDistance);
	};

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
	// 遍历对象池中的每一个类别
	for (auto& Pair : PopObjects)
	{
		// 'Value' 是一个 TArray<APoolableActor*>&，代表当前类别的所有激活对象
		auto& ActiveObjects = Pair.Value;

		// --- 核心修正：从后向前遍历数组 ---
		// 这样在移除元素时，不会影响到尚未检查的元素的索引，从而避免了迭代器失效的问题。
		for (int32 i = ActiveObjects.Num() - 1; i >= 0; --i)
		{
			auto Obj = ActiveObjects[i];
			if (Obj && NeedCollect(PlayerLocation, Obj->GetActorLocation()))
			{
				ReturnPooledObject(Obj);
				ActiveObjects.RemoveAt(i);
			}
		}
	}
}

AActor* UObjectPoolWorldSubsystem::GetPooledObject(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass) return nullptr;

	TArray<AActor*>* Pool = PooledObjects.Find(ActorClass);
	AActor* FoundActor = nullptr;

	if (Pool && Pool->Num() > 0)
	{
		FoundActor = Pool->Pop();
	}
	else
	{
		FoundActor = GetWorld()->SpawnActor<AActor>(ActorClass);// 动态生成一个新的
	}

	if (FoundActor)
	{
		// 找到组件并调用它的激活函数
		UPoolableComponent* PoolableComp = FoundActor->FindComponentByClass<UPoolableComponent>();
		if (PoolableComp)
		{
			PoolableComp->OnPoolSpawn();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Tried to get a pooled object of class %s that has no UPoolableComponent!"), *ActorClass->GetName());
			// 如果没有组件，我们不应该池化它，直接返回，但它将无法被正确归还
		}
	}
	/// record
	TArray<AActor*>& Poped = PopObjects.FindOrAdd(ActorClass);
	Poped.Add(FoundActor);
	
	return FoundActor;
}

AActor* UObjectPoolWorldSubsystem::GetRandomPooledObject(const FString& Tag)
{
	auto CArray = ClassTags.Find(Tag);
	if (!CArray) return nullptr;
	
	int Num = CArray->Num();
	int Index = 0;
	if (Num != 1)
	{
		Index = FMath::RandRange(0, Num - 1);
	}
	return GetPooledObject(CArray->GetData()[Index]);
}

void UObjectPoolWorldSubsystem::ReturnPooledObject(AActor* ActorToReturn)
{
	if (!IsValid(ActorToReturn)) return;
	
	// 找到组件并调用它的失活函数
	UPoolableComponent* PoolableComp = ActorToReturn->FindComponentByClass<UPoolableComponent>();
	if (PoolableComp)
	{
		TArray<AActor*>& Pool = PooledObjects.FindOrAdd(ActorToReturn->GetClass());
		Pool.Add(ActorToReturn);
		PoolableComp->OnPoolReturn();
	}
	else
	{
		// 如果一个Actor没有池化组件，它不属于我们的池，所以直接销毁它
		UE_LOG(LogTemp, Warning, TEXT("ReturnPooledObject called on an Actor (%s) that has no UPoolableComponent. It will be destroyed."), *ActorToReturn->GetName());
		ActorToReturn->Destroy();
	}
}
