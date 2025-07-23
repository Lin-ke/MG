// DamageSystemComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

// 当失去一条命时广播 (参数：剩余生命数)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLifeLostSignature, int32, RemainingLives);

// 当所有生命耗尽，角色最终死亡时广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDiedSignature, AActor*, DeadActor);

/// 根据传进来的伤害类定制。
UCLASS(Blueprintable)
class MG2_API UMGDamageType : public UDamageType
{
	GENERATED_BODY()

public:
	UMGDamageType();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	FName DamageTypeName;
	// // 命中时播放的粒子特效
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	// UParticleSystem* HitEffect;
	//
	// // 命中时播放的音效
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	// USoundBase* HitSound;
};
// 声明委托

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MG2_API UDamageSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDamageSystemComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes", meta = (ExposeOnSpawn = "true"))
	float MaxHealth = 100;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float CurrentHealth = 100;
	// ( 肉鸽系统：护甲和抗性 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	TMap<TSubclassOf<UDamageType>, float> Resistances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Armor = 10;	
	// --- 生命值系统 (Lives System) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lives System")
	int32 MaxLives;

	UPROPERTY(BlueprintReadOnly, Category = "Lives System")
	int32 CurrentLives;

	// 重生后的无敌持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lives System")
	float RespawnInvincibilityDuration = 2;
	
	// “丢命”演出时的游戏暂停（慢动作）持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lives System")
	float LifeLostPauseDuration = 0.10;

	// --- 事件调度器 ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLifeLostSignature OnLifeLost;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActorDiedSignature OnActorDied;

	UFUNCTION(BlueprintCallable, Category = "DamageSystem")
	void SetRespawnPoint(const FTransform& NewRespawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void HandleDamage(float BaseDamage, TSubclassOf<UDamageType> DamageType, AController* EventInstigator, AActor* DamageCauser);

	

protected:
	virtual void BeginPlay() override;

private:
	// --- 内部逻辑函数 ---
	void TriggerRespawnSequence();
	void FinishRespawn();
	void EndInvincibility();
	void Die();

	// --- 内部状态变量 ---
	bool bIsDead;
	bool bIsInvincible;
	FTimerHandle RespawnTimerHandle;
	FTransform LastSafeRespawnTransform; // 存储最近的检查点位置和旋转
};