#include "TerrainGenActor.h"
#include "KismetProceduralMeshLibrary.h"
#include "TerrainGenManager.h"

#include "MGTriggerBox.h"
#include "ObjectPoolWorldSubsystem.h"
#include "DrawDebugHelpers.h"
#include "StaticMeshPoolableActor.h"
// 构造函数
ATerrainGenActor::ATerrainGenActor()
{
    // 将此Actor设置为可Tick，虽然我们在这个例子中不使用Tick
    PrimaryActorTick.bCanEverTick = false;

    // 创建程序化网格组件并设为根组件
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    
    auto BoxBegin = CreateDefaultSubobject<UMGTriggerBox>(TEXT("TriggerBox_0"));
    auto BoxEnd = CreateDefaultSubobject<UMGTriggerBox>(TEXT("TriggerBox_1"));
    TriggerBoxes.Add(BoxBegin);
    TriggerBoxes.Add(BoxEnd);
    RootComponent = ProceduralMesh;
    BoxBegin->SetMobility(EComponentMobility::Movable);
    BoxEnd->SetMobility(EComponentMobility::Movable); 
    BoxBegin->SetupAttachment(ProceduralMesh);
    BoxEnd->SetupAttachment(ProceduralMesh);
}

ATerrainGenActor::~ATerrainGenActor()
{
    /// 新建的SingleLane交给 Manager了，不再需要我管理内存。
}

// 当Actor在编辑器中被修改时，重新生成地形以获得实时预览
void ATerrainGenActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Mgr = UWorld::GetSubsystem<UTerrainGenManager>(GetWorld());

    if (!Mgr || GetWorld()->WorldType == EWorldType::Editor) /// 蓝图或者编辑器
    {
        GenerateTerrainVertex();
        PlaceBarrier();
        PlaceBox();
        PlaceFoliage();
    }
    else
    {
        GenerateTerrainVertex();
        PlaceBox();
    }

}

void ATerrainGenActor::BeginPlay()
{
    Super::BeginPlay();
    PlaceBarrier(); /// call FinalGenTerrain(); in.
    PlaceFoliage();
}

void ATerrainGenActor::PlaceBox()
{
    auto BoxBegin = TriggerBoxes[0];
    auto BoxEnd = TriggerBoxes[1];
    FVector BoxScale =   {100.0f, 500.0f, 100.0f};
    FVector BoxLocation = {0.0f, 2600.0f, 400.0f};
    BoxBegin->SetBox(BoxScale, BoxLocation);
    BoxLocation.X += CellSize * GridWidth;
    BoxEnd->SetBox(BoxScale, BoxLocation);
    BoxBegin->Type = 0;
    BoxEnd->Type = 1;
}   
/// /// 0 : 正常
/// 1 : High end and Empty
/// 2 : 
/// 3 : Low Begin
///

void ATerrainGenActor::GenerateTerrainVertex()
{
    
    // 清空旧数据，准备重新生成
    Vertices.Empty();
    Triangles.Empty();
    UVs.Empty();
    Normals.Empty();
    Tangents.Empty();
    VertexColors.Empty();
    // 随机一个倾斜度
    float Yoffset = 0;
    if (FMath::FRand() < RoadInclineDensity)
    {
        Yoffset = CellSize * FMath::Sin(FMath::FRandRange(-MaxRoadIncline, MaxRoadIncline)/360 * 2 * PI);
    }
    // Gen chunk
    if (!Mgr)
    {
        GlobalX = 0, GlobalY = 0, GenerateFlag = DebugGenerateFlag;
    }
    else
    {
        Mgr->QueryChunk(GlobalX,GlobalY,GenerateFlag,this);
    }
    
    /// High End With Empty
    int GenWidth = GetWidth();
    
    Vertices.Reserve((GridHeight + 1) * (GenWidth + 1));

    for (int32 y = 0; y <= GridHeight; ++y)
    {
        int SlopeBegin = 0; /// use for lerp highend
        for (int32 x = 0; x <= GenWidth; ++x)
        {
            float NoiseValue = FMath::PerlinNoise2D(FVector2D((GlobalX + x) * NoiseScale,( GlobalY+ y) * NoiseScale));
            float ZPosition = NoiseValue * Amplitude;
            if (GenerateFlag == 1 && x == GenWidth - HighLerpGrid)
            {
                SlopeBegin = ZPosition; /// 从-HighLerpGrid开始插值
            }
            else if (GenerateFlag == 1 && x > GenWidth-HighLerpGrid) /// 手动造一个坡出来。
            {
                float HighEndDelta = (HighEnd - SlopeBegin) / HighLerpGrid;
                ZPosition = (x  + HighLerpGrid - GenWidth) / HighLerpGrid * HighEndDelta + SlopeBegin;
            }
            else if (GenerateFlag == 3 && x < LowBeginSafeZone)
            {
                ZPosition = LowEnd * (ZPosition/Amplitude);
            }
            Vertices.Add(FVector(x * CellSize, y * CellSizeY + (x == 0 ? 0: Yoffset), ZPosition));
            UVs.Add(FVector2D((float)x / GenWidth, (float)y / GridHeight)); /// 贴图
        }
    }

    
    
}

void ATerrainGenActor::FinalGenTerrain()
{
   
    int GenWidth = GetWidth();

    /// 定义三角形。
    for (int32 y = 0; y < GridHeight; ++y)
    {
        for (int32 x = 0; x < GenWidth; ++x)
        {
            int32 BottomLeft = x + y * (GenWidth + 1);
            int32 BottomRight = (x + 1) + y * (GenWidth + 1);
            int32 TopLeft = x + (y + 1) * (GenWidth + 1);
            int32 TopRight = (x + 1) + (y + 1) * (GenWidth + 1);

            // 第一个三角形
            Triangles.Add(BottomLeft);
            Triangles.Add(TopRight);
            Triangles.Add(BottomRight);

            // 第二个三角形
            Triangles.Add(BottomLeft);
            Triangles.Add(TopLeft);
            Triangles.Add(TopRight);
        }
    }

    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);
    ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
}

void ATerrainGenActor::PlaceFoliage()
{
   /// default nothing.
}

void ATerrainGenActor::PlaceBarrier()
{
    /// 1. Generate Safe road.
    /// 2. Refine Vertex (when high end)
    ActiveLane.Empty();
    if (!LaneParam.Num() )
    {
        LaneParam.Init({}, 1);
    }
    int YSize = GetY();
    int Width = GetWidth();
    auto DefaultLane = [&]()
    {
        ActiveLane.Add(new SingleLane{GetY()/ 3, 0});
        ActiveLane.Add(new SingleLane{GetY() / 3 * 2, 0});
    };
    if (!Mgr || (Mgr->QueryLastRoad(ActiveLane), ActiveLane.IsEmpty()))
    {
        DefaultLane();  
    }
   

    auto ClampMove = [&](float& Location, float Move)
    {
        Location = FMath::Clamp(Location + Move, 200.f, GetY() );
    };
    LaneMap.Reserve(Width + 1);
    LaneMap.Init({}, Width + 1);

    const auto RecordLane = [&](int x)
    {
        ActiveLane.Sort([](const SingleLane& Lane1, const SingleLane& Lane2)
        {
            return Lane1.Location < Lane2.Location; 
        });
        auto&& IntervalArray = LaneMap[x];
        check(!ActiveLane.IsEmpty());
        if (InSafeZone(x))
        {
            IntervalArray.Add({0, YSize});
            return;
        }
        TPair<float, float> CurrInterval = ActiveLane[0]->Interval(LaneParam, YSize);
        for (int32 i = 1; i < ActiveLane.Num(); ++i)
        {
            const auto& Lane = ActiveLane[i];
            auto NextInterval = Lane->Interval(LaneParam, YSize);

            if (NextInterval.Key <= CurrInterval.Value) // 重叠
            {
                CurrInterval.Value = FMath::Max(CurrInterval.Value, NextInterval.Value);
            }
            else // 不重叠
            {
                IntervalArray.Add(CurrInterval);
                CurrInterval = NextInterval;
            }
        }
        IntervalArray.Add(CurrInterval);
    };
    
    for (int32 x = 0; x < Width; ++x)
    {
        int LaneSize = ActiveLane.Num(); 
        /// 先记录，再移动。
        RecordLane(x);
        if (bDebugLog)
            LOG("at x: %d",x)
        TArray<SingleLane*> NewBornLanes;
        for (int32 y = 0; y < LaneSize; ++y)
        {
			SingleLane& CurrLane= *ActiveLane[y];
            if (bDebugLog)
                CurrLane.Log();
            auto CurrLaneParam = LaneParam[CurrLane.LaneParamIndex]; /// 这样子用 Param
            float NoiseValue = FMath::PerlinNoise1D((x * CurrLaneParam.WanderFrequency) + CurrLaneParam.NoiseOffset) * CurrLaneParam.WanderAmount;

            switch (CurrLane.Flag)  /// 0 : main; 1 : sub normal; 2: sub branching left; 3: sub branching right; 4: sub merging.
            {
                case 0: /// Main Lane
                    {
                        if (CurrLane.BranchTime < MinBranchInterval )
                        {
                            CurrLane.BranchTime ++;
                        }
                        else if (FMath::FRand() < BranchChance)
                        {
                            float ChildLocation = CurrLane.Location;
                            ClampMove(ChildLocation, -NoiseValue); /// 相反方向
                            SingleLane* NewBranch =  new SingleLane{ ChildLocation ,
                                FMath::RandRange(0, LaneParam.Num()-1)};
                            if (NoiseValue > 0) /// 父亲Lane向右移动说明孩子向左移动
                                NewBranch->Flag = 2;
                            else
                                NewBranch->Flag = 3;
                            NewBranch->PBranch = &CurrLane;
                            NewBornLanes.Add(NewBranch); // 添加到列表末尾
                            CurrLane.BranchTime = 0; // reset
                        }
                        ClampMove(CurrLane.Location, NoiseValue); 
                        break;
                    }
                case 1: // sub normal
                    {
                        if (CurrLane.BranchTime < MinMergeInterval )
                        {
                            CurrLane.BranchTime ++;
                            ClampMove(CurrLane.Location, NoiseValue); 

                        } else if (FMath::FRand() < MergeChance)
                        {
                            /// Begin Merge
                            CurrLane.BranchTime = 0;
                            CurrLane.Flag = 4;
                        }
                        
                        // default
                        break;
                    }
                case 2: // sub left
                    {
					    if (CurrLane.BranchTime < BranchMoving )
					    {
					        CurrLane.BranchTime ++;
					        ClampMove(CurrLane.Location, - FMath::Abs(NoiseValue)); 
					    }
                        else
                        {
                            CurrLane.BranchTime = 0;
                            CurrLane.Flag = 4;
                        }
                        break;
                    }
				case 3: // sub right
					{
                        if (CurrLane.BranchTime < BranchMoving )
                        {
                            CurrLane.BranchTime ++;
                            ClampMove(CurrLane.Location,  FMath::Abs(NoiseValue)); 
                        }
                        else
                        {
                            CurrLane.BranchTime = 0;
                            CurrLane.Flag = 4;
                        }
                        break;
					}
                case 4: // merging.
                    {
                        // merging.
                        if (!CurrLane.PBranch)
                        {
                            // Maybe bug
                            UE_LOG(LogTemp, Error, TEXT("Sub Lane No Parent?")); break;
                        }

                        if (CurrLane.BranchTime < BranchMoving || !CurrLane.Cross(*CurrLane.PBranch, LaneParam)) // keep moving.
                        {
                            CurrLane.BranchTime ++;
                            float TargetY = CurrLane.PBranch->Location;
                            float Lerp = (TargetY - CurrLane.Location) / MergeMoving;
                            ClampMove(CurrLane.Location, Lerp); 
                        }
                       else
                       {
                           CurrLane.Flag = -1; // dying.
                       }
                        break;
                    }
            default :
                    {
                        // don't move.
                        break;
                    }
            }
        }
		///加入，删除。
        if (NewBornLanes.Num() > 0)
        {
            ActiveLane.Append(NewBornLanes);
        }

        for (int32 i = ActiveLane.Num() - 1; i >= 0; --i)
        {
            // 获取指针
            auto* Ptr = ActiveLane[i];

            if (Ptr != nullptr && Ptr->Flag == -1)
            {
                delete Ptr;
                ActiveLane.RemoveAt(i);
            }
        }
    }

    RecordLane(Width); // Record  Last
    
    if (Mgr)
    {
        Mgr->SetLastRoad(ActiveLane); // 记录上一个区块的最终状态。
    }
    
    else 
    {
        for (int32 i = 0; i < ActiveLane.Num(); ++i)
        {
            if (ActiveLane[i] != nullptr)
                delete ActiveLane[i];
        }

    }

    //// Gen Lane End.


    
    // high end根据这个结果塌陷一部分。
    auto Fall = [&](int X, int Y)
    {
        FVector& VertexLocation = Query(Vertices, X, Y);
        VertexLocation.Z -= FMath::Abs(FMath::PerlinNoise1D(X * CollapseNoiseScale) * CollapseAmplitude); 
    };
    
    if (GenerateFlag == 1)
    {
        for (int x = Width - HighLerpGrid; x <= Width; x++) 
        {
            auto& Intervals = LaneMap[x]; 
            float lastSafeZoneEnd = 0.0f; // Track the end of the last safe zone
            for (int i = 0; i < (int)Intervals.Num(); i++)
            {
                int gapStart_Y = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
                int gapEnd_Y = FMath::FloorToInt(Intervals[i].Key / CellSizeY) ;
            
                for (int y = gapStart_Y; y <= gapEnd_Y ; y++) /// per grid fall
                {
                    Fall(x,y);
                }
                lastSafeZoneEnd = Intervals[i].Value;
            }
            // last zone to end.
            int gapStart_Y = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
            int gapEnd_Y = GetHeight();
            for (int y = gapStart_Y; y <= gapEnd_Y ; y++)
            {
                Fall(x,y);
            }
        }
    }

    FinalGenTerrain(); /// after refine.
    const auto BarrierPool = UWorld::GetSubsystem<UObjectPoolWorldSubsystem>(GetWorld());
    const bool UseDebugBarrier = bDebugBarrier || !Mgr/*basically蓝图*/ || !BarrierPool || !BarrierPool->Available();
    /// place Barrier
    auto RandGen = [&](int X, int Y)
    {
        if (FMath::FRand() < BarrierDensity) // place single barrier
        {
            const FVector& VertexLocation = Query(Vertices, X, Y); 
            const auto& VertexNormal = Query(Normals, X, Y);
            FTransform InstanceTransform;
		
            InstanceTransform.SetRotation(FQuat::FindBetweenNormals(FVector::UpVector, VertexNormal));
            InstanceTransform.SetLocation(GetActorTransform().TransformPosition(VertexLocation)); 

            if (UseDebugBarrier && DebugBarrierActor)
            {
                GetWorld()->SpawnActor<AStaticMeshActor>(DebugBarrierActor, InstanceTransform);
            }
            else if (BarrierPool)
            {
                auto Barrier = Cast<AStaticMeshPoolableActor>(BarrierPool->GetRandomPooledObject("Barrier"));
                if (Barrier != nullptr)
                {
                    InstanceTransform.SetScale3D(Barrier->GetActorScale() * FMath::FRandRange(0.7f, 1.f));
                    Barrier->SetActorLocationAndRotation(InstanceTransform.GetLocation(), InstanceTransform.GetRotation());
                    UE_LOG(LogTemp, Warning, TEXT("BarrierActor created %s, %s"), *Barrier->GetName(), *Barrier->GetActorLocation().ToString());
                }
            }
        }
    };
    FIntPoint LastCoinGrid = {-CoinStep, -CoinStep};
    auto SpawnCoinAt = [&](int X, int Y)
    {
        if (X == Width && GenerateFlag != 1) return; // 放置需要下一个格子的信息。
        if (FMath::FRand() >= CoinDensity || FMath::Abs(LastCoinGrid.X - X) + FMath::Abs(LastCoinGrid.Y-Y) < CoinStep) return;
        LastCoinGrid = {X, Y};
        
        FVector& VertexLocation = Query(Vertices, X, Y);
        VertexLocation.Z += 300.f;
        FVector NextVertexLocation = FVector::ZeroVector;
        if (X < Width)
        {
            NextVertexLocation = Query(Vertices, X + 1, Y);
        }
        else // x == width && gen == 1(空中）
        {
            NextVertexLocation = VertexLocation; NextVertexLocation.Z -= 100.f; NextVertexLocation.X += CellSize;
        }
        NextVertexLocation.Z += 300.f;
        const FVector& StartNormal = Query(Normals, X, Y);
        FVector NextNormal = FVector::UpVector;
        if (X < Width)
        {
            NextNormal = Query(Normals, X + 1, Y);
        }
        else // x == width && gen == 1(空中）
        {
            NextNormal = StartNormal;
        }
        const FVector StartPoint = GetActorTransform().TransformPosition(VertexLocation);
        const FVector EndPoint = GetActorTransform().TransformPosition(NextVertexLocation);
    
       // 5 coin插值
        const int NumCoinsToSpawn = 5; /// todo ： 调整。
        for (int i = 0; i < NumCoinsToSpawn; ++i)
        {
            const float Alpha = (float)(i + 1) / (float)(NumCoinsToSpawn);

            const FVector CoinLocation = FMath::Lerp(StartPoint, EndPoint, Alpha);
            const FVector CurrentNormal = FMath::Lerp(StartNormal, NextNormal, Alpha).GetSafeNormal();

            FTransform InstanceTransform;
            InstanceTransform.SetLocation(CoinLocation);
            InstanceTransform.SetRotation(FQuat::FindBetweenNormals(FVector::UpVector, CurrentNormal));

            if (bDebugCoin && DebugCoinActor)
            {
                GetWorld()->SpawnActor<AStaticMeshActor>(DebugCoinActor, InstanceTransform);
            }
            else if (BarrierPool)
            {
                auto Coin = Cast<AStaticMeshPoolableActor>(BarrierPool->GetPooledObject(CoinClass));
                if (Coin != nullptr)
                {
                    Coin->SetActorLocationAndRotation(InstanceTransform.GetLocation(), InstanceTransform.GetRotation());
                }
            }
        }
    };
    
    for (int x = 0; x < Width; x++)
    {
        auto& Intervals = LaneMap[x];
        float lastSafeZoneEnd = 1000.0f; // Track the end of the last safe zone
        for (int i = 0; i < (int)Intervals.Num(); i++)
        {
            // 在中点放置金币
            int Middle_Y = FMath::CeilToInt((Intervals[i].Key + Intervals[i].Value) /2 / CellSizeY );
            SpawnCoinAt(x, Middle_Y);
            
            int gapStart_Y = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
            int gapEnd_Y = FMath::FloorToInt(Intervals[i].Key / CellSizeY);
            
            for (int y = gapStart_Y; y <= gapEnd_Y ; y+=2) /// 避免群聚
            {
                RandGen(x,y);
            }
            lastSafeZoneEnd = Intervals[i].Value;
        }
        // last zone to end.
        int gapStart_Y = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
        int gapEnd_Y = GetHeight();
        for (int y = gapStart_Y; y <= gapEnd_Y ; y++)
        {
            RandGen(x,y);
        }
    }
    VisualizeLaneMap();
}

void ATerrainGenActor::GenGold()
{
}

FIntPoint ATerrainGenActor::GetSize()
{
    return {(int) (GridWidth *CellSize), (int)( GridHeight * CellSizeY)};
}

void ATerrainGenActor::OnSpawn()
{
}

void ATerrainGenActor::OnReturn()
{
}

int ATerrainGenActor::GetWidth()
{
    int GenWidth = GenerateFlag == 1 ? GridWidth - EmptyGird : GridWidth;
    return GenWidth;
}

float ATerrainGenActor::GetY()
{
    return GridHeight * CellSizeY;
}

float ATerrainGenActor::GetX()
{
    return GetWidth() * CellSize;
}

void ATerrainGenActor::VisualizeLaneMap()
{
     if (!bDrawDebugLanes)
    {
        return; // Exit if debug drawing is turned off in the editor
    }

    UWorld* World = GetWorld();
    if (!World || LaneMap.IsEmpty())
    {
        return;
    }

    const int32 Width = GetWidth();
    const float LineThickness = 25.0f; // Make the lines easy to see

    // Iterate through each column of the terrain grid
    for (int32 x = 0; x <= Width; ++x)
    {
        auto&& Intervals = LaneMap[x];
        float lastSafeZoneEnd = 0.0f;

        // Draw the SAFE zones for this column
        for (const TPair<float, float>& SafeInterval : Intervals)
        {
            // --- Draw UNSAFE zone (red) ---
            // The gap between the last safe zone and this one is unsafe
            if (SafeInterval.Key > lastSafeZoneEnd)
            {
                int32 UnsafeStartY = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
                int32 UnsafeEndY = FMath::FloorToInt(SafeInterval.Key / CellSizeY);
                if (UnsafeEndY > UnsafeStartY)
                {
                    FVector StartPos = GetActorTransform().TransformPosition(Query(Vertices, x, UnsafeStartY) ); ;
                    FVector EndPos =  GetActorTransform().TransformPosition(Query(Vertices, x, UnsafeEndY));
                    DrawDebugLine(World, StartPos, EndPos, FColor::Red, true, -1, 0, LineThickness);
                }
            }

            // --- Draw SAFE zone (green) ---
            int32 SafeStartY = FMath::CeilToInt(SafeInterval.Key / CellSizeY);
            int32 SafeEndY = FMath::FloorToInt(SafeInterval.Value / CellSizeY);
            if (SafeEndY > SafeStartY)
            {
                FVector StartPos = GetActorTransform().TransformPosition(Query(Vertices, x, SafeStartY));
                FVector EndPos =  GetActorTransform().TransformPosition(Query(Vertices, x, SafeEndY));
                DrawDebugLine(World, StartPos, EndPos, FColor::Green, true, -1, 0, LineThickness);
            }
            
            lastSafeZoneEnd = SafeInterval.Value;
        }

        // --- Draw final UNSAFE zone ---
        // The gap from the last safe zone to the top edge of the map
        int32 FinalYSize = GetY();
        if (lastSafeZoneEnd < FinalYSize)
        {
            int32 UnsafeStartY = FMath::CeilToInt(lastSafeZoneEnd / CellSizeY);
            int32 YEnd = GetHeight();
            if (FinalYSize > UnsafeStartY)
            {
                FVector StartPos = GetActorTransform().TransformPosition( Query(Vertices, x, UnsafeStartY)); 
                FVector EndPos = GetActorTransform().TransformPosition(Query(Vertices, x, YEnd));
                DrawDebugLine(World, StartPos, EndPos, FColor::Red, true, -1, 0, LineThickness);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("Lane map %d"), x);
        for (const TPair<float, float>& SafeInterval : Intervals)
        {
            UE_LOG(LogTemp, Warning, TEXT("%f, %f"), SafeInterval.Key, SafeInterval.Value);
        }
    }

}

bool ATerrainGenActor::InSafeZone(int x)
{
    if (GenerateFlag == 3 && x < LowBeginSafeZone)
    {
        return true;
    }
    return false;
}


