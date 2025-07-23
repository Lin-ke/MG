#pragma once
#include "TerrainGenActor.h"
#include "TerrainActorGrass.generated.h"
UCLASS()

class TERRAINGEN_API ATerrainActorGrassActor : public ATerrainGenActor
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	float GrassDensity = 5.f; // 用一个0-1的值控制密度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	float MaxSlopeForGrass = 30.0f; // 草地能生长的最大坡度
	// 为草地创建一个HISM组件
	UPROPERTY(VisibleAnywhere)
	class UHierarchicalInstancedStaticMeshComponent* GrassHISM;
	// 将草的静态网格暴露到蓝图/编辑器，方便替换
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	UStaticMesh* GrassMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	UStaticMesh* CloudMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	UMaterialInterface* CloudMat;
	TArray<UStaticMeshComponent*> Clouds;

	
	ATerrainActorGrassActor();
	
protected:
	
	virtual void PlaceFoliage() override;
	void PlaceCloud();
};
