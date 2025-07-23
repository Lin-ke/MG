// Copyright Epic Games, Inc. All Rights Reserved.

#include "FluidParticle.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFluidParticleModule"

void FFluidParticleModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FluidParticle"))->GetBaseDir(), TEXT("ComputeShaders"));
	
	AddShaderSourceDirectoryMapping(TEXT("/Plugins/FP"), PluginShaderDir);
}


void FFluidParticleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFluidParticleModule, FluidParticle)