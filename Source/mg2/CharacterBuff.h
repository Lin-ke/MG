#pragma once
#include "CoreMinimal.h"
#include "CharacterBuff.generated.h"
UCLASS()
class MG2_API UCharacterBuff : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	int Index = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	FString BufferName = "Buff";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	FString BuffDescription = "Buff";
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float DamageMultiplier = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	TSubclassOf<UDamageType> DamageType;	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float HealthMultiplier = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float MaxHealthAdder = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float MaxHealthMultiplier = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float CoinMultiplier = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "CharacterBuff")
	float MaxSpeedMultiplier= 0;
	
	
	bool DealDamage() {return DamageMultiplier != 0;}
	bool DealHealth() {return HealthMultiplier != 0;}
	bool DealMaxHealth() {return MaxHealthAdder != 0 || MaxHealthMultiplier != 0;}
	bool DealCoin() {return CoinMultiplier != 0;}
	bool DealMaxSpeed() {return MaxSpeedMultiplier !=0 ;}

	float GetDamageMultiplier() {return DamageMultiplier;}
	TSubclassOf<UDamageType> GetDamageType() {return DamageType;}
	float GetHealthMultiplier() {return HealthMultiplier;}
	float GetMaxHealthMultiplier() {return HealthMultiplier;}
	float GetMaxHealthAdder() {return MaxHealthAdder;}
	float GetMaxSpeedMultiplier() {return MaxSpeedMultiplier;}
	float GetCoinMultiplier() {return CoinMultiplier;}
};
