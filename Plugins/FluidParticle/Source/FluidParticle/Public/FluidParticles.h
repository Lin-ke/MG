
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h" 
#include "FluidParticles.generated.h"
// 数据结构定义保持不变，它依然只是一个数据容器。
USTRUCT(BlueprintType)
struct FFluidParticleInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Fluid Particle")
	FVector Position = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Fluid Particle")
	FVector Velocity = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Fluid Particle")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "Fluid Particle")
	float Radius = 10;
};


// 将类从 UActorComponent 改为 AActor
UCLASS()
class FLUIDPARTICLE_API AFluidParticleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// 构造函数
	AFluidParticleActor();

protected:
	// 相当于 Unity 的 Start()
	virtual void BeginPlay() override;

	// 关键：处理销毁逻辑。当Actor被销毁时调用。
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// 相当于 Unity 的 Update()，注意Actor的Tick函数签名与Component不同
	virtual void Tick(float DeltaTime) override;
	/**
		 * @brief 以给定的速度发射这个“泡泡”
		 * @param InitialVelocity 发射的初速度向量
		 */
	UFUNCTION(BlueprintCallable, Category = "Bubble")
	void LaunchBubble(const FVector& InitialVelocity);
	FFluidParticleInfo& GetParticleInfo() { return ParticleInfo; }
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereComponent;
	// 调试
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* DebugMeshComponent;
	// 存储此Actor实例的粒子信息
	FFluidParticleInfo ParticleInfo;

};