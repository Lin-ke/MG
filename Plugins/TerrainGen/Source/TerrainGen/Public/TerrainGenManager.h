#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "TerrainGenActor.h"
#include "TerrainGenManager.generated.h"



UCLASS()
class TERRAINGEN_API UTerrainGenManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UTerrainGenManager();

	~UTerrainGenManager();

	TArray<TSubclassOf<ATerrainGenActor>> BPTerrainClass;

	TSubclassOf<ATerrainGenActor> DefaultTerrainClass;
	
	TSubclassOf<ATerrainGenActor> CliffTerrain;


	struct TerrainPoolObject
	{
		ATerrainGenActor* Terrain;
		bool bIsAvailable;
		
	};
	struct TerrainPool
	{
		TArray<TerrainPoolObject> Items;
		ATerrainGenActor* Get()
		{
			for (auto&[k,v]: Items)
			{
				if (v)
				{
					v = false;
					return k; 
				}
			}
			return nullptr;
		}
		int AvailableNum()
		{
			int count = 0;
			for (auto&[k,v]: Items)
			{
				if (v)
				{
					count++;
				}
			}
			return count;
		}
		bool Return(ATerrainGenActor* actor)
		{
			for (auto &[k,v]: Items)
			{
				if (k == actor)
				{
					v = true; // available.
					return true;
				}
			}
			return false;
		}
		ATerrainGenActor* GetAvailable(int Index)
		{
			for (auto &[k,v]: Items)
			{
				if (v && --Index < 0)
				{
					v = false;
					return k;
				}
			}
			return nullptr;
		}
		void Add(const TerrainPoolObject& Object)
		{
			Items.Add(Object);
		}
		int Num() const{return Items.Num();}
	};
	
	TerrainPool ManualTerrains; // AActor, bIsAvailable.
	void GenDefault();
	void Notification(const FVector& PlayerLocation, int What);
	void FillTerrianClass();
	void QueryChunk(int& ChunkX, int& ChunkY, int& GenFlag, class ATerrainGenActor* TerrainGenActor);

	void QueryLastRoad(TArray<ATerrainGenActor::SingleLane*>& Info);
	void SetLastRoad(const TArray<ATerrainGenActor::SingleLane*>& Info);
	void QueryBarrierLocation(TArray<FVector2D>& Barrier);
	void RegisterManual(AActor* ActorInstance);
	void RegisterBPClass(TSubclassOf<ATerrainGenActor> cls)
	{
		BPTerrainClass.AddUnique(cls);
	}
	float GenCliffDensity = 0.1f;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
private:
	int CurrChunkX = 0;
	int CurrChunkY = 0;

	int TotalWorldSize = 2000000;

	struct TerrainInfo
	{
		ATerrainGenActor* Actor = nullptr;
		bool bDestroyed = false;
	};
	TMap<FIntRect, TerrainInfo> Terrains;	


	


	int InitialChunkNum = 0;

	TSubclassOf<ATerrainGenActor> NextGenClass;
	ATerrainGenActor* NextGenManualTerrain = nullptr;
	
	/// /// 0 : 正常
	/// 1 : High end and Empty
	/// 2 : 
	/// 3 : Low Begin
	///
	int NextGenFlag=0;

	////
	///
	TArray<ATerrainGenActor::SingleLane*> LastRoad;
	///
	/// Tool function
	///
	void AddTerrain(ATerrainGenActor* Actor, FIntPoint Size, int BeginX);
	/// PCG Rules.
	/// 这里要决定：
	/// 1. 选择哪种类型的Terrain生成。
	/// 2. 填写 CurrGenFlag和 NextGenFlag （这是由于一些类型的Terrain通过这个在原先的基础上魔改）
	void GenerateRule();


};
