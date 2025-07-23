#pragma once
#include "TerrainGenActor.h"
#include "Components/StaticMeshComponent.h"
#include "TerrainGenEmpty.generated.h"

UCLASS()
class TERRAINGEN_API ATerrainGenOfflineActor : public ATerrainGenActor
{
	GENERATED_BODY()
	
public:
	ATerrainGenOfflineActor()
	{
		SetMobility(EComponentMobility::Movable);
	}
	virtual void PlaceBarrier() override;
	virtual void PlaceFoliage() override {};
	virtual void OnConstruction(const FTransform& Transform) override
	{
		/// default do nothing, unless called in Tool
	}
	virtual void BeginPlay() override;

	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> GeneratedActors;

	
	virtual void OnSpawn() override;
	virtual void OnReturn() override;
	void SetStaticMesh(UStaticMesh* Mesh);
};




UCLASS()
class TERRAINGEN_API ATerrainOfflineTool : public AActor
{
	GENERATED_BODY()
	public:
	ATerrainOfflineTool()
	{
	}

	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void AttachNearbyActors();

	/** 附加Actor的搜索半径 */
	UPROPERTY(EditAnywhere, Category = "Actor Attachment")
	float SearchRadius = 50000.0f;
	
	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void DetachAllActors();
	
	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void GenerateOfflineLevel();

	/** 在编辑器中点击此按钮，清除所有已生成的物体，方便重新生成 */
	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void ClearGeneratedObjects();

	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void BakeToBlueprintAsset();

	UFUNCTION(CallInEditor, Category = "Offline Generation")
	void BakeToStaticMeshAsset();

	UPROPERTY(EditAnywhere, Category = "Baking")
	FString MeshSavePath = "/Game/GeneratedLevel/";

	UPROPERTY(EditAnywhere, Category = "Baking")
	FString MeshAssetName = "LevelPrefab0_StaticMesh";

	
	UPROPERTY(EditAnywhere, Category = "Baking")
	FString BlueprintSavePath = "/Game/GeneratedLevel/";

	UPROPERTY(EditAnywhere, Category = "Baking")
	FString BlueprintAssetName = "LevelPrefab0";

	UPROPERTY(EditAnywhere, Category = "Baking")
	ATerrainGenOfflineActor* OfflineActor = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> GeneratedActors;

	UPROPERTY(EditAnywhere, Category = "Baking")
	TSubclassOf<ATerrainGenOfflineActor> OfflineActorClass;	
private:
	/** 用于存储我们已附加的Actor的引用，方便后续解除关系 */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AttachedActors;
	
};
