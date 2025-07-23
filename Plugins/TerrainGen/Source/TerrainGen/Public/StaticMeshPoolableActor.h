#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "StaticMeshPoolableActor.generated.h"

class UPoolableComponent;
/// static mesh hit damage.
UCLASS(BlueprintType)
class TERRAINGEN_API UMeshHitDamageType : public UDamageType
{
	GENERATED_BODY()
	public:
	UMeshHitDamageType(){}
};
UCLASS(BlueprintType)
class TERRAINGEN_API UMeshHitBonusType : public UDamageType
{
	GENERATED_BODY()
public:
	UMeshHitBonusType(){}
};
UCLASS(BlueprintType)
class TERRAINGEN_API UMeshHitHealType : public UDamageType
{
	GENERATED_BODY()
public:
	UMeshHitHealType(){}
};

UCLASS()
class TERRAINGEN_API AStaticMeshPoolableActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AStaticMeshPoolableActor();
	UFUNCTION()
	virtual void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float HitDamage = 10;

	void SetStaticMeshTransform(const FTransform& Transform);
	
	bool bUsed = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;
	virtual void OnConstruction(const FTransform& Transform) override;

	/// 1 : coin; 2: 血包
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Damage")
	int Type = 0; 	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URotatingMovementComponent* RotatingMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType = UMeshHitDamageType::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	bool IgnorePoolComponent = false;

	
private:
	class UObjectPoolWorldSubsystem* Mgr;
};


