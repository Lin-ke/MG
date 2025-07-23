// FluidParticleGenerator.cpp

#include "FluidParticleGenerator.h"
#include "FluidParticles.h"       // Include the header for the actor we are spawning
#include "Kismet/KismetMathLibrary.h" // For access to FMath::VRand()

AFluidParticleGenerator::AFluidParticleGenerator()
{
	// Allow this actor to run logic every frame
	PrimaryActorTick.bCanEverTick = true;

	// 1. 创建一个默认的场景组件(USceneComponent)实例
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

	// 2. 将这个新创建的组件设置为该Actor的根组件
	RootComponent = DefaultSceneRoot;
	// --- Set default values for our properties ---
	ParticleClassToSpawn = nullptr;
	SpawnInterval = 0.1f;
	ParticlesPerInterval = 5;
	MaxParticleCount = 1000;
	SpawnRadius = 20.0f; // Equivalent to 0.2f in Unity meters

	// --- Initialize internal counters ---
	CurrentParticleCount = 0;
	TimerAccumulator = 0.0f;
}

void AFluidParticleGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Accumulate time since the last spawn
	TimerAccumulator += DeltaTime;

	// Check if it's time to spawn and if we haven't reached the max particle limit
	if (TimerAccumulator >= SpawnInterval && CurrentParticleCount < MaxParticleCount)
	{
		// Reset the timer for the next interval
		TimerAccumulator = 0.0f;

		// Ensure a valid class has been assigned in the editor before trying to spawn
		if (ParticleClassToSpawn)
		{
			UWorld* const World = GetWorld();
			if (World)
			{
				for (int32 i = 0; i < ParticlesPerInterval; ++i)
				{
					// Stop spawning if we hit the max limit during this burst
					if (CurrentParticleCount >= MaxParticleCount)
					{
						break;
					}
					
					// Calculate a random spawn location within the defined radius
					// FMath::VRand() is UE's efficient way to get a random direction vector.
					const FVector RandomOffset = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(FVector::UpVector, 45.f) * SpawnRadius;
					const FVector SpawnLocation = GetActorLocation() + RandomOffset;

					// Spawn the actor
					auto fp = World->SpawnActor<AFluidParticleActor>(ParticleClassToSpawn, SpawnLocation, FRotator::ZeroRotator);
					// const FVector SpawnVelocity = {0, 100, .1};
					// fp->LaunchBubble(SpawnVelocity);

					// Increment the count of total spawned particles
					CurrentParticleCount++;
				}
			}
		}
	}
}