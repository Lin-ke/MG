// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageSystemComponent.h"

#include "AvalancheManager.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UMGDamageType::UMGDamageType()
{
}

UDamageSystemComponent::UDamageSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 初始化默认值
	MaxLives = 3;

	bIsDead = false;
	bIsInvincible = false;
}

void UDamageSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化当前生命值和生命数
	CurrentHealth = MaxHealth;
	CurrentLives = MaxLives;
	
	// 将初始位置设为第一个安全重生点
	if (GetOwner())
	{
		LastSafeRespawnTransform = GetOwner()->GetActorTransform();
	}
}

void UDamageSystemComponent::HandleDamage(float BaseDamage, TSubclassOf<UDamageType> DamageType, AController* EventInstigator, AActor* DamageCauser)
{
	FString DebugMessage = FString::Printf(TEXT("ReceiveDamage: %f"), BaseDamage);
	GEngine->AddOnScreenDebugMessage(114514, 5.0f, FColor::Yellow, DebugMessage);

	auto DoDamange = [&](float FinalDamage)
	{
		if (CurrentHealth - FinalDamage <= 0.0f)
		{
			// 如果还有剩余生命，则触发重生序列
			if (CurrentLives >= 2)
			{
				TriggerRespawnSequence();
			}
			else // 一条命用完了，死了
			{
				CurrentHealth = 0.0f;
				OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
				Die();
			}
		}
		else // 如果伤害不致命，则正常扣血
		{
			CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
			OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
		}
	};
	
	if (bIsDead || bIsInvincible)
	{
		return;
	}

	if (BaseDamage == -1) /// falling damage
	{
		DoDamange(CurrentHealth);
	}
	else
	{
		float FinalDamage = BaseDamage;
		// 1. 计算抗性
		if (Resistances.Contains(DamageType))
		{
			const float ResistanceValue = *Resistances.Find(DamageType);
			FinalDamage *= (1.0f - ResistanceValue);
		}
		DoDamange(FinalDamage);
	}
	
	DebugMessage = FString::Printf(TEXT("Health: %f, Life: %d"), CurrentHealth, CurrentLives);
	GEngine->AddOnScreenDebugMessage(114515, 5.0f, FColor::Yellow, DebugMessage);

}

void UDamageSystemComponent::SetRespawnPoint(const FTransform& NewRespawnTransform)
{
	LastSafeRespawnTransform = NewRespawnTransform;
	UE_LOG(LogTemp, Log, TEXT("Respawn point for %s updated."), *GetOwner()->GetName());
}

void UDamageSystemComponent::TriggerRespawnSequence()
{
	// 1. 消耗一条命
	CurrentLives--;
	UE_LOG(LogTemp, Log, TEXT("%s lost a life! %d lives remaining."), *GetOwner()->GetName(), CurrentLives);
	bIsInvincible = true;
	// 2. 广播“丢命”事件 (用于UI更新心形图标等)
	OnLifeLost.Broadcast(CurrentLives);

	// 3. 进入慢动作演出


	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&UDamageSystemComponent::FinishRespawn,
		LifeLostPauseDuration,
		false
	);
	// UGameplayStatics::SetGamePaused(GetWorld(), true);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);

}

void UDamageSystemComponent::FinishRespawn()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	// 1. 传送角色到最近的检查点
	auto Subsystem = UWorld::GetSubsystem<UAvalancheManager>(GetWorld());
	if (!Subsystem || Subsystem->GetAvalancheX() == 0) /// no avalanche
	{
		LastSafeRespawnTransform.SetLocation(GetOwner()->GetActorLocation()); // TODO
	}
	else{
		
		LastSafeRespawnTransform.SetLocation({ FMath::Max(Subsystem->GetAvalancheX(), OwnerActor->GetActorLocation().X) + 1000, 10000, 1000});
	}
	
	OwnerActor->SetActorTransform(LastSafeRespawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogTemp, Log, TEXT("Respawning %s."), *OwnerActor->GetName());
	///
	/// @todo 无敌特效。
	/// 
	// 2. 恢复正常游戏速度
	// 3. 完全恢复生命值
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth); // 广播事件，让UI血条回满

	// 4. 给予临时无敌
	bIsInvincible = true;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&UDamageSystemComponent::EndInvincibility,
		RespawnInvincibilityDuration,
		false
	);
}

void UDamageSystemComponent::EndInvincibility()	
{
	bIsInvincible = false;
	UE_LOG(LogTemp, Log, TEXT("%s invincibility has ended."), *GetOwner()->GetName());
}

void UDamageSystemComponent::Die()
{
	if (bIsDead) return;

	bIsDead = true;
 	UE_LOG(LogTemp, Log, TEXT("%s has died permanently. Game Over."), *GetOwner()->GetName());
	
	// 广播最终死亡事件
	OnActorDied.Broadcast(GetOwner());
	
	// 在这里可以处理真正的游戏结束逻辑，比如禁用输入、播放死亡动画、显示"Game Over"界面等
}