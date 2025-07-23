#pragma once
#include "FluidParticles.h"
#include "SceneViewExtension.h"
#include "RHIGPUReadback.h"
class FParticleViewExtension : public FSceneViewExtensionBase
{
public:
	FParticleViewExtension(const FAutoRegister& AutoRegister);
	~FParticleViewExtension();
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	void PerpareRenderData(FRDGBuilder& GraphBuilder, const TArray<FFluidParticleInfo>&);
	FRHIGPUBufferReadback* Readback = nullptr;
	
	FRDGBufferUAVRef ParticleInfoBuffer = nullptr;
	FRDGBufferUAVRef TileMappingBuffer = nullptr;
	FRDGBufferUAVRef ComputeDataBuffer = nullptr;
	FRDGBufferUAVRef ChunkMappingBuffer = nullptr;

	void ChunkBuild(FRDGBuilder& GraphBuilder,FVector3f SDFScale, FVector3f SDFWorldPos, FUintVector3 ChunkSize,int count );
	void SDFBuild();
	void TileBuild();
	void RayMarch();
	void Lighting();
	void DepthCulling();
	void Combine();
	
	

};


