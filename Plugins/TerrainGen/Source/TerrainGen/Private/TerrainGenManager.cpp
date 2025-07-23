#include "TerrainGenManager.h"
#include "EngineUtils.h"
#include "Animation/AnimTrace.h"

UTerrainGenManager::UTerrainGenManager()
{
}

UTerrainGenManager::~UTerrainGenManager()
{
	/// delete LastRoads
	for (int32 i = 0; i < LastRoad.Num(); ++i)
	{
		if (LastRoad[i] != nullptr)
			delete LastRoad[i];
	}
}

void UTerrainGenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTerrainGenManager::Deinitialize()
{
	Super::Deinitialize();
}

void UTerrainGenManager::AddTerrain(ATerrainGenActor* Actor, FIntPoint Size, int BeginX)
{
	FIntPoint Begin(BeginX, 0);
	TerrainInfo& Terrain = Terrains.FindOrAdd(FIntRect(Begin, Begin + Size));
	Terrain.Actor = Actor;
}



void UTerrainGenManager::GenDefault()
{
	UWorld* World = GetWorld();
	auto Uclass = DefaultTerrainClass;
	if (World && Uclass)
	{
		// 在世界的原点(0,0,0)生成地形Actor
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		auto curr = World->SpawnActor<ATerrainGenActor>(Uclass, SpawnLocation, SpawnRotation, SpawnParams);
		
		FIntPoint DefaultTerrainSize = curr->GetSize();

		AddTerrain(curr, DefaultTerrainSize, 0);
		SpawnLocation.X -= DefaultTerrainSize.X;
		int temp = CurrChunkX;
		CurrChunkX = -CurrChunkX;
		auto prev = World->SpawnActor<ATerrainGenActor>(Uclass, SpawnLocation, SpawnRotation, SpawnParams);
		AddTerrain(prev, DefaultTerrainSize, -DefaultTerrainSize.X);

		CurrChunkX = temp;
		SpawnLocation.X += 2*curr->CellSize * curr->GridWidth;
		auto next = World->SpawnActor<ATerrainGenActor>(Uclass, SpawnLocation, SpawnRotation, SpawnParams);
		AddTerrain(next, DefaultTerrainSize, 2*DefaultTerrainSize.X);
		
		SpawnLocation.X += curr->CellSize * curr->GridWidth;
		next = World->SpawnActor<ATerrainGenActor>(Uclass, SpawnLocation, SpawnRotation, SpawnParams);
		AddTerrain(next, DefaultTerrainSize, 3*DefaultTerrainSize.X);
		
		InitialChunkNum = 4;
	}
	
}

void UTerrainGenManager::QueryChunk(int& ChunkX, int& ChunkY, int& GenFlag, class ATerrainGenActor* TerrainGenActor)
{
	ChunkX = CurrChunkX;
	CurrChunkX += TerrainGenActor->GridWidth;
	ChunkY = 0;
	GenFlag = NextGenFlag;
}

void UTerrainGenManager::QueryLastRoad( TArray<ATerrainGenActor::SingleLane*>& Info)
{
	if (GEngine->IsEditor())
	{
		/// do nothing.
		return;
	}
	Info = LastRoad;
}

void UTerrainGenManager::SetLastRoad(const TArray<ATerrainGenActor::SingleLane*>& Info)
{
	if (GEngine->IsEditor())
	{
		/// do nothing.
		return;
	}
	LastRoad = Info;
}

void UTerrainGenManager::QueryBarrierLocation(TArray<FVector2D>& Barrier)
{
}


void UTerrainGenManager::Notification(const FVector& PlayerLocation, int What)
{
	if (!What == 1) return;	
	// collect
	TArray<ATerrainGenActor*> GenLands;
	for (TActorIterator<ATerrainGenActor> Itr(GetWorld()); Itr ; ++Itr)
	{
		GenLands.Add(*Itr);
	}
	UE_LOG(LogTemp, Display, TEXT("Notification Begin"));
	Algo::Sort(GenLands, [](const ATerrainGenActor* a, const ATerrainGenActor* b)
	{
		// across X-axis
		return a->GetActorTransform().GetLocation().X < b->GetActorTransform().GetLocation().X;
	});
	// destory old
	for (ATerrainGenActor* GenLand : GenLands)
	{
		const auto& Location = GenLand->GetActorTransform().GetLocation();
		if ( Location.X < PlayerLocation.X && FVector::Distance(Location, PlayerLocation) > 60000) 
		{
			if (ManualTerrains.Return(GenLand))
			{
				GenLand->OnReturn();
				continue;
			}
			GenLand->Destroy();
		}
	}
	

	auto GenTerrain = [&](UClass* UClass)
	{
		auto LastTerrain = GenLands.Last();
		
		auto SpawnLocation =LastTerrain->GetTransform().GetLocation();
		UE_LOG(LogTemp, Display, TEXT("Last Post %s"), *SpawnLocation.ToString());
		SpawnLocation.X += LastTerrain->CellSize * LastTerrain->GridWidth;
		auto Terrain = GetWorld()->SpawnActor<ATerrainGenActor>(UClass, SpawnLocation, FRotator::ZeroRotator, {});
		AddTerrain(Terrain, Terrain->GetSize(), SpawnLocation.X);
		
	};

	auto GenTerrainActor = [&](ATerrainGenActor* Actor)
	{
		auto LastTerrain = GenLands.Last();
		
		auto SpawnLocation =LastTerrain->GetTransform().GetLocation();
		UE_LOG(LogTemp, Display, TEXT("Last Post %s"), *SpawnLocation.ToString());
		SpawnLocation.X += LastTerrain->CellSize * LastTerrain->GridWidth;
		Actor->SetActorLocation(SpawnLocation);
		Actor->OnSpawn();
		AddTerrain(Actor, Actor->GetSize(), SpawnLocation.X);
		
	};
	GenerateRule(); /// pcg strategy.
	if (NextGenClass)
	{
		GenTerrain(NextGenClass);
	}
	else if (NextGenManualTerrain)
	{
		GenTerrainActor(NextGenManualTerrain);
	}
	else
	{
		/// error.
	}
}

void UTerrainGenManager::FillTerrianClass()
{
	
}

/// /// 0 : 正常
/// 1 : High end
/// 2 : Empty
/// 3 : Low Begin
///

void UTerrainGenManager::GenerateRule()
{
	auto RandomDefault = [&]()
	{
		/// 首先计算数量
		
		const int count = BPTerrainClass.Num() + ManualTerrains.AvailableNum();
		if (count == 0)
		{
			NextGenFlag = 0;
			NextGenClass = DefaultTerrainClass;
			return 0;
		}
		
		float percent = 1.f/float(count);
		float Rnumber=  FMath::FRand();
		
		for (int i = 0; i < count; i++)
		{
			if (Rnumber > percent * i && Rnumber < percent * (i + 1))
			{
				NextGenFlag = 0;
				if (i >= BPTerrainClass.Num()) /// Manual Terrain
				{
					NextGenManualTerrain = ManualTerrains.GetAvailable(count - BPTerrainClass.Num()); /// 置false
					NextGenClass = nullptr;
				}
				else /// BP terrain
				{
					NextGenClass = BPTerrainClass[i];
				}
				return i;
			}
		}
		return 0;
	};
	int RandomGenIdx = 0;
	if (NextGenFlag == 1)
	{
		RandomGenIdx = RandomDefault();
		NextGenFlag = 3;
	}
	else if (FMath::FRand() < GenCliffDensity && CliffTerrain!=nullptr)
	{
		NextGenClass = CliffTerrain;
		RandomGenIdx = 10; /// cliff
		NextGenFlag = 1;
	}
	else
	{
		RandomGenIdx = RandomDefault();
	}

	FString DebugMessage = FString::Printf(TEXT("Next Gen Type %d, Flag %d "), RandomGenIdx, NextGenFlag);
	GEngine->AddOnScreenDebugMessage(114517, 10.0f, FColor::Yellow, DebugMessage);
	
	return;
}

void UTerrainGenManager::RegisterManual(AActor* ActorInstance)
{
	if (!Cast<ATerrainGenActor>(ActorInstance)) return;
	ATerrainGenActor* GenActor = Cast<ATerrainGenActor>(ActorInstance);
	ManualTerrains.Add({GenActor, true});
}
