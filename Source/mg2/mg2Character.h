// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DamageSystemComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "mg2Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class Amg2Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	
	
public:
	Amg2Character();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Jump(const FInputActionValue& Value);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinimumSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaximumSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float AccelerationRate = .1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SidewaysMovementMultiplier = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SidewaysMovementMultiplierBonus = 2.5f; /// 滑动方向和移动方向相反

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	// float SidewaysMovementMultiplierDecay = 2.5f; 


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SidewaysMovementMaxSpeed = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Die = -5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float HitSlowdownFactor = 0.5f;
	// This will now be our current speed, which changes over time.
	float CurrentRunSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ForwardRunSpeed = 1.0f;
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void StraightVelocity();

	void HitSlowVelocity();

	/// 速度加成
	struct BonusInfo
	{
		float MaxWalkVelocityBonus = 6000.0f;
		float BonusLastTime = 5.0f; // 5s
		float BonusAddVelocity = 100.0f; // 获得一个 这个加速度。
	};

	struct MGTimerHandle
	{
		FTimerHandle  SpeedBonusTimerHandle;
		bool Using = false;
	};
	
	TArray<MGTimerHandle> SpeedBonusTimerHandle;
	
	void BonusVelocity(BonusInfo Info);

	/// 速度修正
	/// 暂时不启用。
	FVector2D PrevMovementVector = FVector2D::ZeroVector;
	uint32_t LastMovementFrame = 0;	
	bool bVelocityStraight = true;
	uint32_t CurrFrame = 0;

	class UTerrainGenManager* Mgr = nullptr;

	// 0 : walking
	// 1 : landing
	// 2 : jumping (up)
	int MovingState = 0;

	/// Healthy
	UDamageSystemComponent* DamageSystem = nullptr;

	/** 闪烁效果的总持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flashing")
	float FlashDuration = 1.0f;

	/** 闪烁的频率 (秒/次) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Flashing")
	float FlashRate = 0.1f;

	/** 处理闪烁循环的计时器句柄 */
	FTimerHandle FlashTimerHandle;

	/** 处理停止闪烁的计时器句柄 */
	FTimerHandle StopFlashTimerHandle;

	/** 启动闪烁效果 */
	void StartFlashing();

	/** 执行单次闪烁（切换可见性） */
	void FlashEffect();

	/** 停止闪烁效果并恢复可见 */
	void StopFlashing();

	/// 商店场景
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool ShopMode = false;

	
};

