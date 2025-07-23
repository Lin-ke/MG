#include "TerrainGenEmpty.h"

#include "MGTriggerBox.h"
#include "TerrainGenManager.h"
#include "StaticMeshAttributes.h"
#include "Kismet/GameplayStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/BoxComponent.h"
void ATerrainGenOfflineActor::PlaceBarrier()
{
	FinalGenTerrain(); /// after refine.
}

void ATerrainGenOfflineActor::BeginPlay()
{
	Super::BeginPlay();
	Mgr = UWorld::GetSubsystem<UTerrainGenManager>(GetWorld());
	if (Mgr)
	{
		Mgr->RegisterManual(this);
	}
	/// 需要把自己隐藏起来。
	
}

void ATerrainOfflineTool::AttachNearbyActors()
{
#if WITH_EDITOR
	if (!GetWorld() || !(GetWorld()->WorldType == EWorldType::Editor)) return;

	// 在附加新的Actor之前，先解除所有旧的附加关系，防止重复操作
	DetachAllActors();

	// 1. 获取场景中所有的 StaticMeshActor
	TArray<AActor*> ActorsToPotentiallyAttach;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), ActorsToPotentiallyAttach);

	UE_LOG(LogTemp, Log, TEXT("Found %d StaticMeshActors in the scene. Starting to attach..."), ActorsToPotentiallyAttach.Num());

	// 2. 遍历所有找到的Actor
	for (AActor* Actor : ActorsToPotentiallyAttach)
	{
		if (!Actor || Actor == this) continue;

		if (FVector::Dist(GetActorLocation(), Actor->GetActorLocation()) <= SearchRadius)
		{
			Actor->AttachToActor(OfflineActor, FAttachmentTransformRules::KeepWorldTransform);
			if (auto PoolActor = Cast<AStaticMeshPoolableActor>(Actor); PoolActor)
			{
				
			}
			AttachedActors.Add(Actor);
            
			UE_LOG(LogTemp, Log, TEXT("Attached actor: %s"), *Actor->GetName());
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Attachment complete! Attached %d actors."), AttachedActors.Num());
#endif
}

void ATerrainOfflineTool::GenerateOfflineLevel()
{
	if (!GetWorld() || !(GetWorld()->WorldType ==EWorldType::Editor)) return;

	if (OfflineActor)
	{
		OfflineActor->Destroy();
	}
	FActorSpawnParameters SpawnParams;
	if (!OfflineActorClass) OfflineActorClass = ATerrainGenOfflineActor::StaticClass();
	OfflineActor = GetWorld()->SpawnActor<ATerrainGenOfflineActor>(OfflineActorClass, GetActorLocation(), GetActorRotation(), SpawnParams);
	OfflineActor->GenerateTerrainVertex();
	OfflineActor->FinalGenTerrain();
	OfflineActor->PlaceBox();
}

void ATerrainOfflineTool::ClearGeneratedObjects()
{
	for (AActor* Actor : GeneratedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
}

void ATerrainOfflineTool::BakeToBlueprintAsset()
{
}

void ATerrainOfflineTool::BakeToStaticMeshAsset()
{
#if WITH_EDITOR // 确保这段代码只在编辑器模式下编译
    auto ProceduralMesh = OfflineActor->ProceduralMesh;
    // 1. 检查ProceduralMeshComponent是否有效
    if (!ProceduralMesh || ProceduralMesh->GetNumSections() == 0)
    {
        GenerateOfflineLevel();
    }

    // 2. 构造完整的包路径和资产名
    FString PackagePath = FPaths::Combine(MeshSavePath, MeshAssetName);
    FPackageName::RegisterMountPoint(*MeshSavePath, FPaths::ProjectContentDir() + MeshSavePath.RightChop(5));

    // 3. 创建一个用于容纳新资产的“包”
    UPackage* Package = CreatePackage(*PackagePath);
    Package->FullyLoad();

    // 4. 在包内创建一个新的StaticMesh对象
    UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>(Package, FName(*MeshAssetName), RF_Public | RF_Standalone);
    NewStaticMesh->InitResources();
    NewStaticMesh->SetLightingGuid(FGuid::NewGuid());

	// 1. 创建一个临时的 FMeshDescription
	FMeshDescription MeshDescription;
	FStaticMeshAttributes Attributes(MeshDescription);
	Attributes.Register(); // 初始化标准属性

	// 2. 获取我们想要填充的属性层的引用
	TVertexAttributesRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();
	TVertexInstanceAttributesRef<FVector3f> Normals1 = Attributes.GetVertexInstanceNormals();
	TVertexInstanceAttributesRef<FVector2f> UVs1 = Attributes.GetVertexInstanceUVs();
	// 如果需要，也可以获取顶点颜色等其他属性
	// TVertexInstanceAttributesRef<FVector4f> Colors = Attributes.GetVertexInstanceColors();

	// 我们假设将所有PMC截面合并到StaticMesh的LOD0中
	for (int32 SectionIndex = 0; SectionIndex < ProceduralMesh->GetNumSections(); ++SectionIndex)
	{
	    FProcMeshSection* ProcSection = ProceduralMesh->GetProcMeshSection(SectionIndex);
	    if (!ProcSection || ProcSection->ProcVertexBuffer.Num() == 0) continue;

	    // --- A. 创建顶点并设置位置 ---
	    TArray<FVertexID> VertexIDs; // 用来存储新创建的顶点的ID
	    VertexIDs.Reserve(ProcSection->ProcVertexBuffer.Num());

	    for (const FProcMeshVertex& ProcVertex : ProcSection->ProcVertexBuffer)
	    {
	        // 在MeshDescription中创建一个新的顶点，并获取其ID
	        const FVertexID NewVertexID = MeshDescription.CreateVertex();
	        VertexIDs.Add(NewVertexID);
	        
	        // 使用 Set() 方法来为这个新创建的顶点设置位置属性
	        VertexPositions.Set(NewVertexID, (FVector3f)ProcVertex.Position);
	    }

	    // --- B. 创建三角形和顶点实例 (Wedges) ---
	    FPolygonGroupID PolygonGroupForSection = MeshDescription.CreatePolygonGroup();

	    for (int32 i = 0; i < ProcSection->ProcIndexBuffer.Num(); i += 3)
	    {
	        TArray<FVertexInstanceID> CornerInstanceIDs;
	        CornerInstanceIDs.SetNum(3);

	        for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
	        {
	            const int32 ProcVertexIndex = ProcSection->ProcIndexBuffer[i + CornerIndex];
	            const FProcMeshVertex& ProcVertex = ProcSection->ProcVertexBuffer[ProcVertexIndex];
	            const FVertexID VertexID = VertexIDs[ProcVertexIndex];

	            // 为三角形的每个“角”创建一个“顶点实例”
	            const FVertexInstanceID NewInstanceID = MeshDescription.CreateVertexInstance(VertexID);
	            CornerInstanceIDs[CornerIndex] = NewInstanceID;

	            // 为这个“角”设置法线和UV属性
	            Normals1.Set(NewInstanceID, (FVector3f)ProcVertex.Normal);
	            // 假设使用第一个UV通道 (通道0)
	            UVs1.Set(NewInstanceID, 0, (FVector2f)ProcVertex.UV0);
	            // Colors.Set(NewInstanceID, (FVector4f)ProcVertex.Color); // 如果需要顶点色
	        }

	        // 使用三个“顶点实例”来创建三角形
	        MeshDescription.CreateTriangle(PolygonGroupForSection, CornerInstanceIDs);
	    }

	    // 将此截面的材质分配给多边形组
	    NewStaticMesh->GetStaticMaterials().Add(FStaticMaterial(ProceduralMesh->GetMaterial(SectionIndex)));
	}


	// 将填充好的 MeshDescription 提交给 StaticMesh
	TArray<const FMeshDescription*> MeshDescriptions { &MeshDescription };
	NewStaticMesh->BuildFromMeshDescriptions(MeshDescriptions);

    NewStaticMesh->PostEditChange();
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewStaticMesh);

    UE_LOG(LogTemp, Warning, TEXT("Bake successful! Asset saved to: %s"), *PackagePath);

	OfflineActor->SetStaticMesh(NewStaticMesh);
	
	
#endif // WITH_EDITOR
}

void ATerrainGenOfflineActor::OnSpawn()
{
	SetActorHiddenInGame(false);
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainGenOfflineActor::OnReturn()
{
	SetActorHiddenInGame(true);
	PrimaryActorTick.bCanEverTick = false;
	SetActorLocation(FVector(0, 0, -20000.0));
}


void ATerrainGenOfflineActor::SetStaticMesh(UStaticMesh* Mesh)
{
	if (ProceduralMesh && IsValid(ProceduralMesh))
		ProceduralMesh->ClearMeshSection(0);
	RootComponent = GetStaticMeshComponent();
	auto SM = GetStaticMeshComponent();
	SM->SetStaticMesh(Mesh);
	for (auto box : TriggerBoxes)
	{
		if (box)
		{
			if (!box->TriggerBox->AttachToComponent(SM, FAttachmentTransformRules::KeepWorldTransform))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to attach box to component"));
			}
		}
	}
}


void ATerrainOfflineTool::DetachAllActors()
{
	TArray<AActor*> OutActors;
	OfflineActor->GetAttachedActors(OutActors);
	for (auto Actor : OutActors)
	{
		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	// 清空跟踪数组
	AttachedActors.Empty();
}

