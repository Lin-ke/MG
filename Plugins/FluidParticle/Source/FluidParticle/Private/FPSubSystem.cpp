#include "FPSubSystem.h"
#include "SceneViewExtension.h"
#include "ViewExtension.h"
#include "EngineUtils.h"
#include "FluidParticleGenerator.h"
const int MAX_FP = 10000;
DECLARE_CYCLE_STAT(TEXT("FPSubsystem Tick"), STAT_DataCollector_Tick, STATGROUP_Tickables);
void UFPSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FPViewExtension = FSceneViewExtensions::NewExtension<FParticleViewExtension>();
	UE_LOG(LogTemp, Log, TEXT("FParticleViewExtension Created."));
	
}

void UFPSubSystem::Deinitialize()
{
	Super::Deinitialize();
	{
		FPViewExtension->IsActiveThisFrameFunctions.Empty();
		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;
		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext Context)
		{
			return TOptional<bool>(false);
		};
		FPViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}
	FPViewExtension.Reset();
	FPViewExtension = nullptr;
	
}
void UFPGatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PInfos.Init({}, MAX_FP);
}
void UFPGatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
void UFPGatherSubsystem::Gather()
{
	// gather info.
	count = 0;
	for (TActorIterator<AFluidParticleActor> ActorItr(GWorld); ActorItr && count < MAX_FP; ++ActorItr)
	{
		PInfos[count++] = ActorItr->GetParticleInfo();
	}
	for (TActorIterator<AFluidParticleGenerator> ActorItr(GWorld); ActorItr ; ++ActorItr)
	{
		GeneratorPos = FVector3f(ActorItr->GetActorLocation());
		GeneratorScale = FVector3f(ActorItr->GetActorScale());
		break; // find the first one.
	}
}



ASubsystemTicker::ASubsystemTicker()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics; 
}

void ASubsystemTicker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 获取Subsystem并调用它的更新函数
	if (UWorld* World = GetWorld())
	{
		if (UFPGatherSubsystem* MySubsystem = World->GetSubsystem<UFPGatherSubsystem>())
		{
			MySubsystem->Gather();
		}
	}
}