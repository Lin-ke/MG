#pragma once
#include "Subsystems/EngineSubsystem.h"
#include "FluidParticles.h"
#include "FPSubSystem.generated.h"

UCLASS()
class UFPSubSystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	
	TSharedPtr<class FParticleViewExtension, ESPMode::ThreadSafe> FPViewExtension;

};

UCLASS()
class UFPGatherSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	 virtual void Deinitialize() override;
	void Gather();
	TArray<FFluidParticleInfo> PInfos; // 不要做成单例的。
	int count = 0;
	FVector3f GeneratorPos = FVector3f::ZeroVector;
	FVector3f GeneratorScale = FVector3f::ZeroVector;
};

UCLASS(NotBlueprintable, NotPlaceable) // 这个Actor不需要被蓝图继承或手动放置
class ASubsystemTicker : public AActor
{
	GENERATED_BODY()

public: 
	ASubsystemTicker();
	virtual void Tick(float DeltaTime) override;
};