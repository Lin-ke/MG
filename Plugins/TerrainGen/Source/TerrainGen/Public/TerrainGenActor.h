#pragma once
#include "CoreMinimal.h"
#include "StaticMeshPoolableActor.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "Engine/StaticMeshActor.h"

#include "TerrainGenActor.generated.h"
#define LOG(text, ...) UE_LOG(LogTemp, Display, TEXT(text), ##__VA_ARGS__)
class UProceduralMeshComponent;

USTRUCT(BlueprintType)
struct FPathLane
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathLane")
	float Width = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathLane")
	float WanderAmount = 500.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathLane")
	float WanderFrequency = 0.02f;

	// 独立的噪声偏移量，确保每条路径摆动方式不同
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathLane")
	float NoiseOffset = 0.0f;

	FPathLane()
	{
		NoiseOffset = FMath::FRand() * 100.0f;
	}
};



UCLASS()
class TERRAINGEN_API ATerrainGenActor : public AStaticMeshActor
{
	GENERATED_BODY()
public:
	ATerrainGenActor();
	~ATerrainGenActor();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void PlaceBox();
	// 地形的核心组件
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UProceduralMeshComponent* ProceduralMesh;

	// ----- 地形参数，暴露给蓝图和编辑器 -----
	/// x: GridWidth ... y:Gridheight
	/// ...
	/// ... 
	/// X:0, y:0 .... y:GridHeight.
	///
	/// Noise 增大则方差增大
	/// Amplitude 为山峰最高值
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int32 GridWidth = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int32 GridHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float CellSize = 1500.0f; // UE中单位是厘米

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float CellSizeY = 700.0f; // UE中单位是厘米
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float NoiseScale = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float Amplitude = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float HighEnd = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float HighLerpGrid = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int EmptyGird = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float CollapseNoiseScale = 0.12; /// 坡的随机塌陷
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float CollapseAmplitude = 2000.f; /// 坡的随机塌陷
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float LowEnd = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float MaxRoadIncline = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float RoadInclineDensity = .3f;
	
	// 生成地形的函数
	virtual void GenerateTerrainVertex();
	virtual void FinalGenTerrain();
	virtual void PlaceFoliage(); // 将植被放置逻辑独立出来
	virtual void PlaceBarrier(); // 障碍物放置
	virtual void GenGold();
	FIntPoint GetSize();
	
	// 存储网格数据
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents; // 切线
	TArray<FLinearColor> VertexColors; // 顶点颜色

	int GlobalX = 0;
	int GlobalY = 0; // 记录地形的chunk Location（绝对位置）。

	/// Barrier Info

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BarrierDensity = 0.7f; // barrier密度 (0.0 - 1.0)

	/// 会从这里随机选择。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	TArray<FPathLane> LaneParam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	float BranchChance = 0.7f;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	float MergeChance = 0.1f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	TSubclassOf<AStaticMeshPoolableActor> CoinClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int MinBranchInterval = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int MinMergeInterval = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int BranchMoving = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int MergeMoving =5 ;


	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	float CoinDensity =0.4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
	int CoinStep = 5; // 最近的Coin间距

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class UMGTriggerBox*> TriggerBoxes; /// 其中触发就会告知mgr的盒子
	/// /// 0 : 正常
	/// 1 : High end and Empty
	/// 2 : 
	/// 3 : Low Begin
	///
	int GenerateFlag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Settings")
    int LowBeginSafeZone = 5;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int DebugGenerateFlag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugLog = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugBarrier = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TSubclassOf<AStaticMeshActor> DebugBarrierActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TSubclassOf<AStaticMeshActor> DebugCoinActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebugCoin = false;

	
	struct SingleLane
	{
		float Location = 0; /// 相对位置。
		int BranchTime = 0;
		// opt: union.
		int Flag = 0; /// 0 : main; 1 : sub normal; 2: sub branching; 3: sub merging.
		int LaneParamIndex = 0; /// 判断cross
		SingleLane* PBranch = nullptr; // Parent Branch.
		
		bool operator<(const SingleLane& Other) const
		{
			return Location < Other.Location;
		}
		SingleLane(float Location, int Param) : Location(Location) , LaneParamIndex(Param){}
		bool Cross(SingleLane& Other, TArray<FPathLane>& LaneParam)
		{
			float Broad = LaneParam[LaneParamIndex].Width;
			float OtherBroad = LaneParam[Other.LaneParamIndex].Width;
			return FMath::Abs(Other.Location - Location) < Broad + OtherBroad;
		}
		TPair<float, float> Interval(TArray<FPathLane>& LaneParam, float Max) const
		{
			float Broad = LaneParam[LaneParamIndex].Width;
			return {FMath::Max(Location - Broad, 0.0f), FMath::Min(Location + Broad, Max)};			
		}
		void Log() const
		{
			LOG("%f, %d", Location, Flag )
		}
	};
	TArray<SingleLane*> ActiveLane;


	/// 回调，默认什么也不做。
	virtual void OnSpawn();

	virtual void OnReturn(); 
	
	class UTerrainGenManager* Mgr = nullptr;

	

	typedef TArray<TArray<TPair<float, float>>> LaneIntervalArray; /// x -> y道路区间。
	LaneIntervalArray LaneMap;

	///
	/// Tool function
	///
	int GetWidth();
	int GetHeight(){return GridHeight;}
	float GetY(); // return range.
	float GetX(); 
	void VisualizeLaneMap();
	template<typename T>
	T& Query(TArray<T>& V, int x, int y)
	{
			check(x >= 0 && x <= GetWidth() && y >= 0 && y <= GetHeight());
			return V[y * (GetWidth() + 1) + x]; 
	}
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebugLanes = true;
	bool InSafeZone(int x);
};


