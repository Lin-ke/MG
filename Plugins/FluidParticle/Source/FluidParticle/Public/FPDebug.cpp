
#include "FPDebug.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"

AMyDebugVisualizerActor::AMyDebugVisualizerActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMyDebugVisualizerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 获取玩家视点，这是所有计算的基础
    ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer || !LocalPlayer->ViewportClient)
    {
        return;
    }

    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        LocalPlayer->ViewportClient->Viewport,
        GetWorld()->Scene,
        LocalPlayer->ViewportClient->EngineShowFlags)
        .SetRealtimeUpdate(true));

    FVector ViewLocation;
    FRotator ViewRotation;
    FSceneView* View = LocalPlayer->CalcSceneView(&ViewFamily, ViewLocation, ViewRotation, LocalPlayer->ViewportClient->Viewport);

    if (View)
    {
        // 调用核心绘制函数
        DrawTileFrustum(View);
    }
}

FVector AMyDebugVisualizerActor::GetViewSpacePositionFromScreenUV(const FSceneView* View, FVector2D UV, float Depth)
{
    // 1. 将 UV [0,1] 转换到 NDC [60,1]
    // 注意：这里的Y轴翻转是为了匹配NDC坐标系（Y轴向上）
    FVector2D NDC = FVector2D(UV.X * 2.0f - 1.0f, (1.0f - UV.Y) * 2.0f - 1.0f);

    // 2. 从NDC和深度构建裁剪空间坐标
    // 在UE的反向Z中，W=1的点在远平面，所以我们用一个非0的W值
    // 这里的深度参数在C++中通常是W分量
    FVector4 ClipPosition(NDC.X, NDC.Y, Depth, 1.0f);

    // 3. 使用逆投影矩阵从裁剪空间变换到视图空间
    FVector4 ViewPosition = View->ViewMatrices.GetInvProjectionMatrix().TransformFVector4(ClipPosition);

    // 4. 执行透视除法
    return FVector(ViewPosition) / ViewPosition.W;
}


void AMyDebugVisualizerActor::DrawTileFrustum(const FSceneView* View)
{
    // --- 这部分逻辑与你的HLSL代码完全一致 ---
    TotalTileCount = (GEngine->GameViewport->Viewport->GetSizeXY() / 16);
    // 1. 计算当前瓦片在屏幕空间的四个角的UV坐标 [0, 1]
    FVector2D TileSizeInUV = FVector2D(1.0f / TotalTileCount.X, 1.0f / TotalTileCount.Y);
    FVector2D MinUV = FVector2D(TargetTileId.X, TargetTileId.Y) * TileSizeInUV;
    FVector2D MaxUV = MinUV + TileSizeInUV;

    FVector2D UV00(MinUV.X, MinUV.Y); // 左上角
    FVector2D UV10(MaxUV.X, MinUV.Y); // 右上角
    FVector2D UV01(MinUV.X, MaxUV.Y); // 左下角
    FVector2D UV11(MaxUV.X, MaxUV.Y); // 右下角

    // 2. 获取近平面上的四个角点的位置 (在视图空间)
    // 在UE的反向Z中, 近平面上的点在裁剪空间深度为1
    FVector Near00_VS = GetViewSpacePositionFromScreenUV(View, UV00, 1.0f);
    FVector Near10_VS = GetViewSpacePositionFromScreenUV(View, UV10, 1.0f);
    FVector Near01_VS = GetViewSpacePositionFromScreenUV(View, UV01, 1.0f);
    FVector Near11_VS = GetViewSpacePositionFromScreenUV(View, UV11, 1.0f);

    // 3. 计算远平面上的四个角点 (在视图空间)
    // 在UE的反向Z中, 远平面上的点在裁剪空间深度为0
    FVector Far00_VS = GetViewSpacePositionFromScreenUV(View, UV00, 0.0f);
    FVector Far10_VS = GetViewSpacePositionFromScreenUV(View, UV10, 0.0f);
    FVector Far01_VS = GetViewSpacePositionFromScreenUV(View, UV01, 0.0f);
    FVector Far11_VS = GetViewSpacePositionFromScreenUV(View, UV11, 0.0f);

    // 4. 将8个视图空间的角点转换到世界空间以便绘制
    const FMatrix InvViewMatrix = View->ViewMatrices.GetInvViewMatrix();

    FVector Near00_WS = InvViewMatrix.TransformPosition(Near00_VS);
    FVector Near10_WS = InvViewMatrix.TransformPosition(Near10_VS);
    FVector Near01_WS = InvViewMatrix.TransformPosition(Near01_VS);
    FVector Near11_WS = InvViewMatrix.TransformPosition(Near11_VS);

    FVector Far00_WS = InvViewMatrix.TransformPosition(Far00_VS);
    FVector Far10_WS = InvViewMatrix.TransformPosition(Far10_VS);
    FVector Far01_WS = InvViewMatrix.TransformPosition(Far01_VS);
    FVector Far11_WS = InvViewMatrix.TransformPosition(Far11_VS);

    // 5. 绘制视锥体的12条边
    UWorld* World = GetWorld();
    if (!World) return;

    // 绘制近平面
    DrawDebugLine(World, Near00_WS, Near10_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near10_WS, Near11_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near11_WS, Near01_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near01_WS, Near00_WS, FrustumColor, false, 60, 0, LineThickness);

    // 绘制远平面
    DrawDebugLine(World, Far00_WS, Far10_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Far10_WS, Far11_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Far11_WS, Far01_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Far01_WS, Far00_WS, FrustumColor, false, 60, 0, LineThickness);

    // 绘制连接近平面和远平面的边
    DrawDebugLine(World, Near00_WS, Far00_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near10_WS, Far10_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near11_WS, Far11_WS, FrustumColor, false, 60, 0, LineThickness);
    DrawDebugLine(World, Near01_WS, Far01_WS, FrustumColor, false, 60, 0, LineThickness);
}
