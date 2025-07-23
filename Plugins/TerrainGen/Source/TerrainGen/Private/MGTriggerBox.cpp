#include "MGTriggerBox.h"

#include "TerrainGenManager.h"
#include "Components/BoxComponent.h"
UMGTriggerBox::UMGTriggerBox()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox")); // 这个是instance，如果
	TriggerBox->SetMobility(EComponentMobility::Movable);

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &UMGTriggerBox::OnOverlapBegin);
	
}

void UMGTriggerBox::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Used) return;
	if (OverlappedComp != this->TriggerBox) return;

	auto Mgr = UWorld::GetSubsystem<UTerrainGenManager>(GetWorld());
	Mgr->Notification(OtherActor->GetActorLocation(),this->Type);
	Used = true;
}

void UMGTriggerBox::SetBox(FVector Scale, FVector Location)
{
	TriggerBox->SetRelativeLocation(Location);
	TriggerBox->SetRelativeScale3D(Scale);
}
