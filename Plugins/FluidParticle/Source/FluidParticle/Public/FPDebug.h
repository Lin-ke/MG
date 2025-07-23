
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "FPDebug.generated.h"

UCLASS()
class FLUIDPARTICLE_API AMyDebugVisualizerActor : public AActor
{
	GENERATED_BODY()

public:
	// 设置默认值
	AMyDebugVisualizerActor();

protected:
	// 每帧调用
	virtual void Tick(float DeltaTime) override;

public:
	// 你想要可视化的瓦片ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Debug")
	FIntPoint TargetTileId = FIntPoint(0, 0);

	// 屏幕上总的瓦片数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Debug")
	FIntPoint TotalTileCount = FIntPoint(16, 9);

	// 绘制线条的颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Debug")
	FColor FrustumColor = FColor::Cyan;

	// 绘制线条的粗细
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Debug")
	float LineThickness = 2.0f;

private:
	// 核心函数：计算并绘制单个瓦片的视锥体
	void DrawTileFrustum(const FSceneView* View);

	// 辅助函数：将屏幕UV坐标反投影到视图空间中的一个点
	// 这与你的HLSL函数 GetViewSpacePositionFromScreenUV 完全对应
	FVector GetViewSpacePositionFromScreenUV(const FSceneView* View, FVector2D UV, float Depth);
};
