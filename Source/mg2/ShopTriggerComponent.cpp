#include "ShopTriggerComponent.h"

#include "MGGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UMyShopItemWidget::SetItemData(const FBuffShopItemData& ItemData)
{
	ItemID = ItemData.Index;

	// 使用传入的数据更新UI元素
	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData.ItemName);
	}

	if (PriceText)
	{
		// 将价格（整数）转换为文本
		PriceText->SetText(FText::AsNumber(ItemData.Price));
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(ItemData.Description);
	}

	if (IconImage)
	{
		IconImage->SetBrushFromTexture(ItemData.Icon.LoadSynchronous());
		IconImage->SetVisibility(ItemData.Icon.IsNull() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UMyShopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 将我们UI中的"购买"按钮的OnClicked事件绑定到C++的处理器函数上
	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UMyShopItemWidget::HandleBuyButtonClicked);
	}
}

void UMyShopItemWidget::HandleBuyButtonClicked()
{
	// 当按钮被点击时，广播我们的委托，并附上商品ID
	// 这样，父控件（例如主商店UI）就可以监听到这个事件并做出反应
	OnBuyButtonClicked.Broadcast(ItemID);
}

void UBuffShop::NativeConstruct()
{
	Super::NativeConstruct();
	// 1. 获取Game Instance
	UGameInstance* GameInstance = GetGameInstance();
	auto GI= Cast<UMGGameInstance>(GameInstance);
	ItemsToDisplay.Empty();
	if (!GameInstance)
	{
		/// use fake data
		///
		ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff1"), 100, NULL, FText::FromString("血量上限+50"),1));
		ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff2"), 100, NULL, FText::FromString("金币获得翻倍"),2));
		ItemsToDisplay.Add(FBuffShopItemData(FText::FromString("Buff3"), 100, NULL, FText::FromString("收到雪崩伤害-50"),3));
	}
	else
	{
		ItemsToDisplay = GI->GetBuffsToBuy();
	}
	// 4. 动态生成UI
	
	if (ItemListBox && ShopItemWidgetClass)
	{
		// 先清空，防止重复添加
		ItemListBox->ClearChildren();

		// 遍历商品数组
		for (const auto& ItemData : ItemsToDisplay)
		{
			// 为每个商品创建一个UI条目控件
			UUserWidget* ItemWidget = CreateWidget(this, ShopItemWidgetClass);
            
			if (UMyShopItemWidget* ShopItem = Cast<UMyShopItemWidget>(ItemWidget))
			{
			    ShopItem->SetItemData(ItemData);
				ShopItem->OnBuyButtonClicked.AddDynamic(this, &UBuffShop::OnShopItemBuyClicked);
			}
			ItemListBox->AddChildToVerticalBox(ItemWidget);
		}
	}

}

void UBuffShop::OnShopItemBuyClicked(int itemIdx)
{
	UGameInstance* GameInstance = GetGameInstance();
	FString DebugString = FString::Printf(TEXT("BuyButtonClicked: %d"), itemIdx);
	GEngine->AddOnScreenDebugMessage(1123132, 5.0f, FColor::Yellow, DebugString);

	auto GI= Cast<UMGGameInstance>(GameInstance);
	if (GI)
	{
		GI->PurchaseBuff(itemIdx);
	}
}

void UShopTriggerComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	WidgetInstance = CreateWidget<UUserWidget>(PlayerController, WidgetTemplate);
	if (WidgetInstance)
	{
		WidgetInstance->AddToViewport();
	}
	PlayerController->bShowMouseCursor = true;

	PlayerController->bShowMouseCursor = true;

	FInputModeGameAndUI InputModeData;
            
	InputModeData.SetWidgetToFocus(WidgetInstance->TakeWidget());
	InputModeData.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputModeData);
}

void UShopTriggerComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Ensure the overlapping actor is the player character
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!OtherActor || OtherActor != PlayerPawn)
	{
		return;
	}

	if (WidgetInstance && WidgetInstance->IsInViewport())
	{
		WidgetInstance->RemoveFromParent();
		WidgetInstance = nullptr;

		// =======================================================
		//                  *** 恢复游戏状态 ***
		// =======================================================
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			// 1. 隐藏鼠标光标
			PlayerController->bShowMouseCursor = false;

			// 2. 将输入模式恢复为 "只响应游戏"
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
		}
		// =======================================================
	}
}
