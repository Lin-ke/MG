// FluidParticleGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FluidParticleGenerator.generated.h"

// Forward-declare the class we intend to spawn, assuming it's our previous AFluidParticleActor.
class AFluidParticleActor;

UCLASS()
class FLUIDPARTICLE_API AFluidParticleGenerator : public AActor
{
	GENERATED_BODY()

public:
	AFluidParticleGenerator();

protected:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** The class of particle Actor to spawn. Can be set in the editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator Settings")
	TSubclassOf<AFluidParticleActor> ParticleClassToSpawn;

	/** Time in seconds between each spawn burst. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator Settings")
	float SpawnInterval;

	/** How many particles to spawn in each burst. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator Settings")
	int32 ParticlesPerInterval;

	/** The maximum number of particles this generator will create. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator Settings")
	int32 MaxParticleCount;

	/** The radius of the sphere in which particles will be spawned, centered on the generator. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generator Settings")
	float SpawnRadius;

private:
	/** Tracks the total number of particles spawned so far. */
	int32 CurrentParticleCount;

	/** Accumulates time to trigger the next spawn interval. */
	float TimerAccumulator;
};