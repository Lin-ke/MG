#pragma once
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/VerticalBox.h"

#include "Runtime/UMG/Public/Components/Button.h"

#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/TextBlock.h"

#include "ShopTriggerComponent.generated.h"

UCLASS()
class MG2_API UBuffShop : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* ItemListBox;
	UPROPERTY(EditDefaultsOnly, Category = "UI") // 单个商品
	TSubclassOf<UUserWidget> ShopItemWidgetClass;

	UFUNCTION(BlueprintCallable)
	void OnShopItemBuyClicked(int itemIdx);
private:
	
	TArray<FBuffShopItemData> ItemsToDisplay; /// 记录当前商店页面的item

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuyButtonClicked, int, ItemID);

UCLASS()
class MG2_API UMyShopItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief 从外部设置此控件显示的数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop Item")
	void SetItemData(const FBuffShopItemData& ItemData);

	// 当购买按钮被点击时，会广播此委托
	UPROPERTY(BlueprintAssignable, Category = "Shop Item")
	FOnBuyButtonClicked OnBuyButtonClicked;

protected:
	// 使用 meta = (BindWidget) 将蓝图中的UI元素绑定到这些C++变量上
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UImage* IconImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* ItemNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* PriceText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* DescriptionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* BuyButton;

	// 重写 NativeConstruct 来绑定按钮的 OnClicked 事件
	virtual void NativeConstruct() override;

private:
	// "购买"按钮 OnClicked 事件的处理器
	UFUNCTION()
	void HandleBuyButtonClicked();

	// 存储此控件所代表的商品的ID
	UPROPERTY()
	int ItemID;
};





UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MG2_API UShopTriggerComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	UShopTriggerComponent()
	{
		// Set up collision properties
		Super::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Super::SetCollisionResponseToAllChannels(ECR_Ignore);
		Super::SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		// Bind the overlap events to our functions
		OnComponentBeginOverlap.AddDynamic(this, &UShopTriggerComponent::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &UShopTriggerComponent::OnOverlapEnd);
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UBuffShop> WidgetTemplate;

	UPROPERTY()
	UUserWidget* WidgetInstance;
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};
