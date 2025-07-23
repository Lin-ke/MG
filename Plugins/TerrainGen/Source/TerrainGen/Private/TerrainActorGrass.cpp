#include "TerrainActorGrass.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DataWrappers/ChaosVDParticleDataWrapper.h"
#include "Kismet/KismetMathLibrary.h"

ATerrainActorGrassActor::ATerrainActorGrassActor()
{
	GrassHISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("GrassHISM"));

	GrassHISM->SetupAttachment(RootComponent);
	GrassHISM->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	/// TODO random 云摆放策略
	auto Cloud0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cloud_0"));
	auto Cloud1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cloud_1"));
	auto Cloud2=  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cloud_2"));
	auto Cloud3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cloud_3"));
	Cloud0->SetupAttachment(RootComponent);
	Cloud1->SetupAttachment(RootComponent);
	Cloud2->SetupAttachment(RootComponent);
	Cloud3->SetupAttachment(RootComponent);
	
	
	Cloud0->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Cloud1->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Cloud2->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Cloud3->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	Clouds.Add(Cloud0);
	Clouds.Add(Cloud1);
	Clouds.Add(Cloud2);
	Clouds.Add(Cloud3);
}

void ATerrainActorGrassActor::PlaceFoliage()
{
	// 检查是否有有效的草地网格，如果没有就直接返回
	if (!GrassMesh)
	{
		return ; // 用蓝图了
	}
	// 设置HISM要使用的静态网格
	GrassHISM->SetStaticMesh(GrassMesh);
	// 在重新生成前，清除所有旧的实例
	GrassHISM->ClearInstances();

	auto RandomVector = []()
	{
		return FVector({FMath::FRand(), FMath::FRand(), FMath::FRand()});
	};
	// 遍历地形的每一个顶点，来决定是否放置草
	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		const FVector& VertexLocation = Vertices[i];
		const FVector& VertexNormal = Normals[i];

		// 规则1：检查随机密度
		// FMath::FRand()返回一个0到1的随机数，如果它小于我们的密度值，则继续判断
		if (FMath::FRand() > GrassDensity)
		{
			continue; // 概率不够，跳过这个顶点
		}

		// 规则2：检查坡度
		// 顶点法线和世界Z轴(0,0,1)的点积(Dot Product)可以得到坡度的余弦值
		// 我们通过反余弦函数Acos转换为角度
		float SlopeAngle = FMath::Acos(FVector::DotProduct(VertexNormal, FVector::UpVector));
		SlopeAngle = FMath::RadiansToDegrees(SlopeAngle); // 转换为角度

		if (SlopeAngle > MaxSlopeForGrass)
		{
			continue; // 坡度太陡，跳过
		}
        
		// --- 所有规则都通过了，就在这里添加一个实例 ---
		FTransform InstanceTransform;
		
		InstanceTransform.SetLocation(VertexLocation + RandomVector() * 100.f);

		// 让草的方向与地面法线对齐
		InstanceTransform.SetRotation(FQuat::FindBetweenNormals(FVector::UpVector, VertexNormal));
        
		// 添加随机旋转和缩放，让草地看起来更自然
		InstanceTransform.SetScale3D(FVector(FMath::FRandRange(0.8f, 1.2f)));
		InstanceTransform.ConcatenateRotation(FQuat(FRotator(0, FMath::FRand() * 360.f, 0).Quaternion()));
		
		GrassHISM->AddInstance(InstanceTransform);
		GrassHISM->bDisableCollision = true;
	}
	PlaceCloud();
}

void ATerrainActorGrassActor::PlaceCloud()
{
	auto Cloud0 = Clouds[0];
	auto Cloud1 = Clouds[1];
	auto Cloud2 = Clouds[2];
	auto Cloud3 = Clouds[3];
	
	double RandomInt = UKismetMathLibrary::RandomIntegerInRange(1000, 9000);
	double RandomInt1 = UKismetMathLibrary::RandomIntegerInRange(1000, 9000);
	double RandomInt2 = RandomInt + CellSize * GridWidth / 2 ;
	double RandomInt3 = RandomInt1 + CellSize * GridWidth / 2 ;
	Cloud0->SetRelativeLocation({RandomInt, -2387.f, 330.f});
	Cloud1->SetRelativeLocation({RandomInt1, 18000.f, 330.f});
	Cloud2->SetRelativeLocation({RandomInt2, -2387.f, 330.f});
	Cloud3->SetRelativeLocation({RandomInt3, 18000.f, 330.f});
	Cloud0->SetRelativeRotation(FRotator(0, 90, 0));
	Cloud1->SetRelativeRotation(FRotator(0, 90, 0));
	Cloud2->SetRelativeRotation(FRotator(0, 90, 0));
	Cloud3->SetRelativeRotation(FRotator(0, 90, 0));
	FVector Scale = {0.5f, 1.2f, 0.5f};
	Cloud0->SetWorldScale3D(Scale);
	Cloud1->SetWorldScale3D(Scale);
	Cloud2->SetWorldScale3D(Scale);
	Cloud3->SetWorldScale3D(Scale);
	if (CloudMesh)
	{
		Cloud0->SetStaticMesh(CloudMesh);
		Cloud1->SetStaticMesh(CloudMesh);
		Cloud2->SetStaticMesh(CloudMesh);
		Cloud3->SetStaticMesh(CloudMesh);
	}
	if (CloudMat)
	{
		Cloud0->SetMaterial(0, CloudMat);
		Cloud1->SetMaterial(0, CloudMat);
		Cloud2->SetMaterial(0, CloudMat);
		Cloud3->SetMaterial(0, CloudMat);
	}
	
}
