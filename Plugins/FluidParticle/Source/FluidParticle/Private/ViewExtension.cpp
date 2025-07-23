#include "ViewExtension.h"

#include <algorithm>

#include "PostProcess/PostProcessMaterialInputs.h"
#include "FPRender.h"
#include "PixelShaderUtils.h"
#include "Runtime/Renderer/Private/SceneRendering.h"
#include "FluidParticles.h"
#include "FPSubSystem.h"
# include "Engine/GameInstance.h"
const int MAX_FP = 10000;
const int MAX_FLUID_PARTICLE_PER_CHUNK = 1024;
const int CHUNK_PIXEL_SIZE = 800;

const int MAX_FLUID_PARTICLE_PER_TILE = 256;         // 每个tile最多存储粒子数目
const int TILE_PIXEL_SIZE = 16;                       // tile的像素分辨率
const int MAX_TILE_MAPPING_COUNT = (MAX_FLUID_PARTICLE_PER_TILE / (TILE_PIXEL_SIZE * TILE_PIXEL_SIZE) * 1920 * 1080);

#define div(a,b) ( (a + b -1) / b)
static float SmoothRate = 0.5;

static FAutoConsoleVariableRef CVarSmoothRate(
TEXT("r.fp.SmoothRate"),
SmoothRate,
TEXT("SmoothRate"),
ECVF_RenderThreadSafe
);

static float RadiusOffset = 0.5;

static FAutoConsoleVariableRef CVarRadiusOffset(
TEXT("r.fp.RadiusOffset"),
RadiusOffset,
TEXT("RadiusOffset"),
ECVF_RenderThreadSafe
);

static int sdfSize0 = 12800;

static FAutoConsoleVariableRef CVarsdfsize0(
TEXT("r.fp.sdfSize0"),
sdfSize0,
TEXT("sdfSize0"),
ECVF_RenderThreadSafe
);

static int sdfSize1 = 800;

static FAutoConsoleVariableRef CVarsdfsize1(
TEXT("r.fp.sdfSize1"),
sdfSize1,
TEXT("sdfSize1"),
ECVF_RenderThreadSafe
);
static int sdfSize2 = 12800;

static FAutoConsoleVariableRef CVarsdfsize2(
TEXT("r.fp.sdfSize2"),
sdfSize2,
TEXT("sdfSize2"),
ECVF_RenderThreadSafe
);
DECLARE_GPU_DRAWCALL_STAT(DownloadData);

FParticleViewExtension::FParticleViewExtension(const FAutoRegister& AutoRegister)
		: FSceneViewExtensionBase(AutoRegister)
{
	Readback = new FRHIGPUBufferReadback(TEXT("Data ReadBack"));
}

FParticleViewExtension::~FParticleViewExtension()
{
	delete Readback;
	Readback = nullptr;
}



void FParticleViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View,
                                                             const FPostProcessingInputs& Inputs)
{
	// if (!GWorld->IsInitialized()) return;
	// FSceneViewExtensionBase::PrePostProcessPass_RenderThread(GraphBuilder, View, Inputs);
	// const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;
	// // Requires RHI & RenderCore
	// const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	//
	// constexpr bool bUseAsyncCompute = true;
	// RDG_EVENT_SCOPE(GraphBuilder,  "FP Tile build"); // RenderDoc
	// FRDGTextureUAVRef RWTileTexture = nullptr;
	//
	// auto FPSubSystem = UWorld::GetSubsystem<UFPGatherSubsystem>(View.Family->Scene->GetWorld());
	// if (!FPSubSystem) return;
	// auto& PInfos = FPSubSystem->PInfos;
	// int count = FPSubSystem->count;
	// if (!count ) return;
	// auto ArraySlice = MakeArrayView(PInfos.GetData(), count);
	// Algo::Sort(ArraySlice, [CamPose = View.ViewLocation, CamDir = View.GetViewDirection()](FFluidParticleInfo& A, FFluidParticleInfo& B)
	// {
	// 	return ((A.Position - CamPose) | CamDir) < ((B.Position - CamPose) | CamDir);
	// });
	// PerpareRenderData(GraphBuilder, PInfos);
	//
	// FIntPoint TileSize = div(Viewport.Size(), TILE_PIXEL_SIZE);
	//
	// auto TileTextureCreateDesc = FRDGTextureDesc::Create2D( TileSize, PF_R32G32_UINT, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	// auto TileTextureRef=  GraphBuilder.CreateTexture(TileTextureCreateDesc, TEXT("FP TileTexture"), ERDGTextureFlags::None);
	// RWTileTexture = GraphBuilder.CreateUAV(TileTextureRef);
	//
	// auto Params = GraphBuilder.AllocParameters<FFPTileBuildCSParameter>();
	// Params->TILE_SIZE = {static_cast<uint32_t>(TileSize.X), static_cast<uint32_t>(TileSize.Y)};
	// Params->SMOOTH_RATE = SmoothRate;
	// float NP = 1;
	// float FP = 10000;
	// Params->CAMERA_PLANE = {NP, FP};
	// Params->RADIUS_OFFSET = RadiusOffset;
	// Params->RW_TILE_TEXTURE = RWTileTexture;
	// Params->PARTICLE_INFO_SIZE = count;
	// Params->COMPUTE_DATA_BUFFER = ComputeDataBuffer;
	// Params->TILE_MAPPING_BUFFER = TileMappingBuffer;
	// Params->PARTICLE_INFO_BUFFER = ParticleInfoBuffer;
	// auto ConvertPlanToVector4f = [&](FVector4f& OutVec4f, auto& Plane, bool bFlipPlane)
	// {
	// 	OutVec4f.X = Plane.X;
	// 	OutVec4f.Y = Plane.Y;
	// 	OutVec4f.Z = Plane.Z;
	// 	OutVec4f.W = Plane.W;
	// 	if (bFlipPlane)
	// 	{
	// 		// We swap some of the planes normal so that they are lerpable while avoiding potential null normal and precision issue at the middle of the frustum.
	// 		OutVec4f.X *= -1.0f;
	// 		OutVec4f.Y *= -1.0f;
	// 		OutVec4f.Z *= -1.0f;
	// 		OutVec4f.W *= -1.0f;
	// 	}
	// };
	// if (View.ViewFrustum.Planes.Num() >= 4)
	// {
	// 	ConvertPlanToVector4f(Params->LeftPlane,	View.ViewFrustum.Planes[0],	false);
	// 	ConvertPlanToVector4f(Params->RightPlane,	View.ViewFrustum.Planes[1],	true);
	// 	ConvertPlanToVector4f(Params->TopPlane,		View.ViewFrustum.Planes[2],	true);
	// 	ConvertPlanToVector4f(Params->BottomPlane,	View.ViewFrustum.Planes[3],	false);
	// }
	// else
	// {
	// 	// Disable culling and make each volume visible.
	// 	Params->LeftPlane	= FVector4f::Zero();
	// 	Params->RightPlane	= FVector4f::Zero();
	// 	Params->TopPlane	= FVector4f::Zero();
	// 	Params->BottomPlane	= FVector4f::Zero();
	// }
	// ConvertPlanToVector4f(Params->NearPlane,	View.NearClippingPlane,			false);
	//
	// Params->View = View.ViewUniformBuffer;
	// FIntVector GroupCount = {div(TileSize.X, 8), div(TileSize.Y, 8), 1}; // total thread (tile num) / thread group
	//
	// FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FP build tile"),ERDGPassFlags::AsyncCompute,
	// 	TShaderMapRef<FParticleTileBuildCS>(GlobalShaderMap), Params, GroupCount );
	//
	//
	// FUintVector3 chunkSize;
	// chunkSize[0] = (sdfSize0 + CHUNK_PIXEL_SIZE - 1) / CHUNK_PIXEL_SIZE; /// 
	// chunkSize[1] = (sdfSize1 + CHUNK_PIXEL_SIZE - 1) / CHUNK_PIXEL_SIZE;
	// chunkSize[2] = (sdfSize2 + CHUNK_PIXEL_SIZE - 1) / CHUNK_PIXEL_SIZE;
	// /// clean up sdf chunk data
	// /// build sdf chunk data
	// ChunkBuild(GraphBuilder, FPSubSystem->GeneratorScale, FPSubSystem->GeneratorPos, chunkSize, count);	
	//
	//
	// // 
	//
	
	
}

void FParticleViewExtension::PerpareRenderData(FRDGBuilder& GraphBuilder,const TArray<FFluidParticleInfo>& PInfos)
{
	// if (!ParticleInfoBuffer)
	// {
	// 	auto ParticleInfoBufferRef = CreateStructuredBuffer(GraphBuilder, TEXT("ParticleInfoBuffer"), sizeof(FFluidParticleInfo), MAX_FP, PInfos.GetData(),
	// 		sizeof(FFluidParticleInfo) * MAX_FP);
	// 	ParticleInfoBuffer = GraphBuilder.CreateUAV(ParticleInfoBufferRef);
	// 	
	// }
	// if (!TileMappingBuffer)
	// {
	// 	const auto TileMappingBufferCreateDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), MAX_TILE_MAPPING_COUNT);
	// 	auto TileMappingBufferRef = GraphBuilder.CreateBuffer(TileMappingBufferCreateDesc, TEXT("TileMappingBuffer"));
	// 	TileMappingBuffer = GraphBuilder.CreateUAV(TileMappingBufferRef);
	// }
	// if (!ComputeDataBuffer)
	// {
	// 	FluidParticlesComputeData ComputeData[1] = {{0,0}};
	// 	auto ComputeDataBufferRef = CreateStructuredBuffer(GraphBuilder, TEXT("ComputeDataBuffer"), sizeof(FluidParticlesComputeData), 1, ComputeData, sizeof(FluidParticlesComputeData));
	// 	ComputeDataBuffer = GraphBuilder.CreateUAV(ComputeDataBufferRef);
	// }
	// if (!ChunkMappingBuffer)
	// { // Todo 做sdf 尺寸更新的更新。
	// 	const int MAX_CHUNK_MAPPING_COUNT = (MAX_FLUID_PARTICLE_PER_CHUNK * (sdfSize0 / CHUNK_PIXEL_SIZE) * (sdfSize2 / CHUNK_PIXEL_SIZE) * (sdfSize1 / CHUNK_PIXEL_SIZE));
	// 	auto ChunkMappingBufferCreateDesc = FRDGBufferDesc::CreateStructuredDesc(sizeof(uint32), MAX_CHUNK_MAPPING_COUNT * 2);
	// 	auto ChunkMappingBufferRef = GraphBuilder.CreateBuffer(ChunkMappingBufferCreateDesc, TEXT("ChunkMappingBuffer"));
	// 	ChunkMappingBuffer = GraphBuilder.CreateUAV(ChunkMappingBufferRef);
	// }
	// download compute data
	// {
	// 	RDG_EVENT_SCOPE(GraphBuilder,  "FP download data");
	// 	const auto NumOfBytes = sizeof(FluidParticlesComputeData);
	// 	AddEnqueueCopyPass(GraphBuilder, Readback, ComputeDataBufferRef, NumOfBytes);
	// 	if (Readback->IsReady())
	// 	{
	// 		FluidParticlesComputeData* Buffer = (FluidParticlesComputeData*) Readback->Lock(NumOfBytes);
	// 		UE_LOG(LogTemp, Warning, TEXT("Readback: %d, %d, %d"),count, Buffer[0].tileMappingIndex, Buffer[0].chunkMappingIndex);
	// 		Readback->Unlock(); 
	// 	}
	// }
}


inline void FParticleViewExtension::ChunkBuild(FRDGBuilder& GraphBuilder, FVector3f SDFScale, FVector3f SDFWorldPos, FUintVector3 ChunkSize ,int count)
{
	// const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	//
	// auto Params0 = GraphBuilder.AllocParameters<FFPSDFCleanupChunkParam>();
	// Params0->SDF_SIZE = {sdfSize0, sdfSize1, sdfSize2};
	// Params0->SDF_SCALE = SDFScale;
	// Params0->CHUNK_SIZE = ChunkSize;
	// Params0->CHUNK_MAPPING_BUFFER = ChunkMappingBuffer;
	// FIntVector3 	GroupCount = {div((int)ChunkSize[0], 8), div((int)ChunkSize[0], 8), 1};
	// FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FP clean chunk"),ERDGPassFlags::AsyncCompute,
	// TShaderMapRef<FPSDFCleanupChunkCS>(GlobalShaderMap), Params0, GroupCount );
	//
	// auto Params1 = GraphBuilder.AllocParameters<FFPSDFBuildChunkParam>();
	// Params1->SDF_SIZE = {sdfSize0, sdfSize1, sdfSize2};
	// Params1->SDF_SCALE = SDFScale;
	// Params1->SDF_WORLD_POS = SDFWorldPos;
	// Params1->PARTICLE_INFO_BUFFER = ParticleInfoBuffer;
	// Params1->CHUNK_SIZE = ChunkSize;
	// Params1->CHUNK_MAPPING_BUFFER = ChunkMappingBuffer;
	// Params1->COMPUTE_DATA_BUFFER = ComputeDataBuffer;
	// GroupCount = {div((int)count, 64), 1, 1};
	// FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FP build chunk"),ERDGPassFlags::AsyncCompute,
	// 	TShaderMapRef<FPSDFBuildChunkCS>(GlobalShaderMap), Params1, GroupCount );
	//

}

inline void FParticleViewExtension::SDFBuild()
{
}

inline void FParticleViewExtension::TileBuild()
{
}

inline void FParticleViewExtension::RayMarch()
{
}

inline void FParticleViewExtension::Lighting()
{
}

inline void FParticleViewExtension::DepthCulling()
{
}

inline void FParticleViewExtension::Combine()
{
}