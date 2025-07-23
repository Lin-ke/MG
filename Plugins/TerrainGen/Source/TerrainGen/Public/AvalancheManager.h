#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/DamageType.h"
#include "AvalancheManager.generated.h"

UCLASS()
class TERRAINGEN_API AAvalancheActor : public AActor
{
	GENERATED_BODY()
	public:
	AAvalancheActor();
	
	UStaticMeshComponent* BoxMesh = nullptr; 
};

/// Avalanche 伤害
UCLASS(BlueprintType)
class TERRAINGEN_API UAvalancheDamageType : public UDamageType
{
	GENERATED_BODY()
public:

	UAvalancheDamageType(){}

};
UCLASS()
class TERRAINGEN_API UAvalancheManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	UAvalancheManager();

	UPROPERTY(EditDefaultsOnly, Category = "Avalanche")
	float AvalancheSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Avalanche")
	float MaxAvalancheSpeed = 10000.f; // that should a little faster than user's speed.

	
	UPROPERTY(EditDefaultsOnly, Category = "Avalanche")
	float AvalancheAccSpeed = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Avalanche")
	float AvalancheAccDmg = 10.f;

	
	UPROPERTY(EditDefaultsOnly, Category = "Avalanche")
	float AvalancheX = 10000;
	UFUNCTION()
	float GetAvalancheX() {return AvalancheX ; }

	UFUNCTION()
	float GetAvalancheDmg() {return AvalancheAccDmg;}

	AAvalancheActor* AvalancheActor = nullptr;
	AAvalancheActor* GetAvalancheActor() { return AvalancheActor; }
	void GenDefault();
	double LastDmgTime = 0;

	bool bInitialized = false;
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
};

