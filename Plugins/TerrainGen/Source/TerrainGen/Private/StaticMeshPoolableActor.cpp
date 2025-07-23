#include "StaticMeshPoolableActor.h"
#include "Components/BoxComponent.h"
#include "ObjectPoolWorldSubsystem.h"
#include "PooledObject.h"
#include "Kismet/GameplayStatics.h"
AStaticMeshPoolableActor::AStaticMeshPoolableActor()
{
	UStaticMeshComponent* SMComponent = GetStaticMeshComponent();
	if (!SMComponent)
	{
		return;
	}
	RootComponent = SMComponent;

	SMComponent->SetMobility(EComponentMobility::Movable);
	SMComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SMComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	CollisionBox =CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetMobility(EComponentMobility::Movable);
	
	CollisionBox->SetupAttachment(SMComponent); // Attach to the static mesh
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AStaticMeshPoolableActor::OnOverlapBegin);


	if (!IgnorePoolComponent ) /// 世界且不忽略
		PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));
}

void AStaticMeshPoolableActor::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bUsed) return; /// 已经造成过伤害了
	if (OverlappedComp != this->CollisionBox) return;
	bUsed = true;
	SetActorHiddenInGame(true);
	SetActorLocation(FVector(0, 0, -20000.0));
	
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	APawn* PlayerPawn = PlayerController->GetPawn();
	
	FString DebugMessage;
	if (Type == 0)
	{
		DebugMessage = FString::Printf(TEXT("Hit Barrier!!"));
	}
	else if (Type == 1)
	{
		DebugMessage = FString::Printf(TEXT("Hit Coin!!"));
	}
	else if (Type == 2)
	{
		DebugMessage = FString::Printf(TEXT("Hit HealPackage!!"));
	}
	
	GEngine->AddOnScreenDebugMessage(114518, 5.0f, FColor::Yellow, DebugMessage);

	UGameplayStatics::ApplyDamage(
		  PlayerPawn,             // DamagedActor: 要对谁造成伤害
		  HitDamage,                  // BaseDamage: 伤害的基础数值
		  nullptr,                // EventInstigator: 造成伤害的控制器 (这里没有，所以是nullptr)
		  this,                   // DamageCauser: 造成伤害的Actor (这里是雪崩管理器自身)
		  DamageType);
	
}

void AStaticMeshPoolableActor::SetStaticMeshTransform(const FTransform& Transform)
{
}

void AStaticMeshPoolableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}
