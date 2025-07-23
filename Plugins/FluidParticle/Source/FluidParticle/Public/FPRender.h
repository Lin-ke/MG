// #pragma once
// #include "ShaderParameterStruct.h"
// #include "FluidParticles.h"
// #define MAX_FLUID_PARTICLE_COUNT 10000
// BEGIN_SHADER_PARAMETER_STRUCT(FKuwaharaFilterPSParameters,)
// 	RENDER_TARGET_BINDING_SLOTS()
// END_SHADER_PARAMETER_STRUCT()
//
// class FKuwaharaFilterMaterialPS : public FGlobalShader
// {
// public:
// 	DECLARE_GLOBAL_SHADER(FKuwaharaFilterMaterialPS)
// 		using FParameters = FKuwaharaFilterPSParameters;
// 	SHADER_USE_PARAMETER_STRUCT(FKuwaharaFilterMaterialPS, FGlobalShader)
//
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
// BEGIN_SHADER_PARAMETER_STRUCT(FFluidParticleCSParameter, )
// 	SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FFluidParticleInfo>, ParticleInfo) 
// END_SHADER_PARAMETER_STRUCT()
//
//
// class FFluidParticleCS : public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FFluidParticleCS, Global, );
// 	using FParameters = FFluidParticleCSParameter;
// 	SHADER_USE_PARAMETER_STRUCT(FFluidParticleCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// 	
// };
// struct FluidParticlesComputeData
// {
// 	uint32_t tileMappingIndex;
// 	uint32_t chunkMappingIndex;
// };
// BEGIN_SHADER_PARAMETER_STRUCT(FFPTileBuildCSParameter, )
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FFluidParticleInfo>, PARTICLE_INFO_BUFFER)
//
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint32_t>, TILE_MAPPING_BUFFER)
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FluidParticlesComputeData>, COMPUTE_DATA_BUFFER)
// 	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<uint2>, RW_TILE_TEXTURE)
//
// 	SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
//
// 	SHADER_PARAMETER(uint32_t, PARTICLE_INFO_SIZE)
// 	SHADER_PARAMETER(float, SMOOTH_RATE)
// 	SHADER_PARAMETER(float, RADIUS_OFFSET)
// 	SHADER_PARAMETER(FVector2f, CAMERA_PLANE)
// 	SHADER_PARAMETER(FUintVector2, TILE_SIZE)
//
// 	SHADER_PARAMETER(FVector4f, LeftPlane)
// 	SHADER_PARAMETER(FVector4f, RightPlane)
// 	SHADER_PARAMETER(FVector4f, TopPlane)
// 	SHADER_PARAMETER(FVector4f, BottomPlane)
// 	SHADER_PARAMETER(FVector4f, NearPlane)
// END_SHADER_PARAMETER_STRUCT()
//
//
//
// class FParticleTileBuildCS:public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FParticleTileBuildCS, Global, );
// 	using FParameters = FFPTileBuildCSParameter;
// 	SHADER_USE_PARAMETER_STRUCT(FParticleTileBuildCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
//
//
// BEGIN_SHADER_PARAMETER_STRUCT(FFPCullCSParameter, )
// 	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, SCREEN_DEPTH_TEXTURE)
// 	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, RW_DEPTH_TEXTURE)
// 	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, RW_COLOR_TEXTURE)
// 	SHADER_PARAMETER(FUintVector2, HALF_WINDOW_SIZE)
//
// END_SHADER_PARAMETER_STRUCT()
//
// class FParticleCullCS:public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FParticleCullCS, Global, );
// 	using FParameters = FFPCullCSParameter;
// 	SHADER_USE_PARAMETER_STRUCT(FParticleCullCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
//
// BEGIN_SHADER_PARAMETER_STRUCT(FFPSDFBuildChunkParam, )
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FFluidParticleInfo>, PARTICLE_INFO_BUFFER)
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FluidParticlesComputeData>, COMPUTE_DATA_BUFFER)
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, CHUNK_MAPPING_BUFFER)
//
// 	SHADER_PARAMETER(FVector3f, SDF_WORLD_POS)
// 	SHADER_PARAMETER(FVector3f, SDF_SCALE)
// 	SHADER_PARAMETER(FIntVector3, SDF_SIZE)
// 	
// 	SHADER_PARAMETER(FUintVector3, CHUNK_SIZE)
//
//
// END_SHADER_PARAMETER_STRUCT()
//
// class FPSDFBuildChunkCS :public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FPSDFBuildChunkCS, Global, );
// 	using FParameters = FFPSDFBuildChunkParam;
// 	SHADER_USE_PARAMETER_STRUCT(FPSDFBuildChunkCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
//
//
// BEGIN_SHADER_PARAMETER_STRUCT(FFPSDFCleanupChunkParam, )
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, CHUNK_MAPPING_BUFFER)
//
// 	SHADER_PARAMETER(FUintVector3, CHUNK_SIZE)
// SHADER_PARAMETER(FVector3f, SDF_SCALE)
// SHADER_PARAMETER(FIntVector3, SDF_SIZE)
//
// END_SHADER_PARAMETER_STRUCT()
// class FPSDFCleanupChunkCS :public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FPSDFCleanupChunkCS, Global, );
// 	using FParameters = FFPSDFCleanupChunkParam;
// 	SHADER_USE_PARAMETER_STRUCT(FPSDFCleanupChunkCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
//
// BEGIN_SHADER_PARAMETER_STRUCT(FFPSDFBuildParam, )
// 	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float>, RW_SDF_TEXTURE)
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWBuffer<uint>, CHUNK_MAPPING_BUFFER)
// 	SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FFluidParticleInfo>, PARTICLE_INFO_BUFFER)
//
// 	SHADER_PARAMETER(FUintVector3, CHUNK_SIZE)
// END_SHADER_PARAMETER_STRUCT()
//
// class FPSDFBuildCS :public FGlobalShader
// {
// 	DECLARE_EXPORTED_SHADER_TYPE(FPSDFBuildCS, Global, );
// 	using FParameters = FFPSDFBuildParam;
// 	SHADER_USE_PARAMETER_STRUCT(FPSDFBuildCS, FGlobalShader);
// 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
// 	{
// 		return true;
// 	}
// };
