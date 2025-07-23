#pragma once
#include "MGTriggerBox.generated.h"
UCLASS()
class TERRAINGEN_API UMGTriggerBox: public USceneComponent
{
	GENERATED_BODY()
public:
	UMGTriggerBox();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	int Type  = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UBoxComponent* TriggerBox;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetBox(FVector Scale, FVector Location);
	bool Used = false;
};
