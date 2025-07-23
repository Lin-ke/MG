#include "AvalancheManager.h"
#include "Kismet/GameplayStatics.h"

AAvalancheActor::AAvalancheActor()
{
		// 先用个staticmesh
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = BoxMesh;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAssetFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeAssetFinder.Succeeded())
		BoxMesh->SetStaticMesh(CubeAssetFinder.Object);
	BoxMesh->SetWorldScale3D({1, 100, 1});
}

UAvalancheManager::UAvalancheManager()
{
}


inline void UAvalancheManager::GenDefault()
{
	FVector SpawnLocation = FVector::ZeroVector;
	SpawnLocation.X += AvalancheX;
	AvalancheActor = GetWorld()->SpawnActor<AAvalancheActor>(AAvalancheActor::StaticClass(),
		SpawnLocation, FRotator::ZeroRotator, {});
	bInitialized = true;
}
void UAvalancheManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
}

void UAvalancheManager::Deinitialize()
{
	Super::Deinitialize();
}

void UAvalancheManager::Tick(float DeltaTime)
{
	if (!bInitialized || AvalancheActor) return;
	Super::Tick(DeltaTime);
	AvalancheX += DeltaTime * AvalancheSpeed;
	AvalancheSpeed += DeltaTime * AvalancheAccSpeed;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(FVector(AvalancheX, 0.0f, 0.0f));
	AvalancheActor->SetActorTransform(SpawnTransform);
	/// 扣血
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	APawn* PlayerPawn = PlayerController->GetPawn();
	FString DebugMessage = FString::Printf(TEXT("Player %f, Avalanche %f "), PlayerPawn->GetActorLocation().X, AvalancheActor->GetActorLocation().X);
	GEngine->AddOnScreenDebugMessage(114516, 5.0f, FColor::Yellow, DebugMessage);
	auto NeedDamage = [&]()
	{
		if ( PlayerPawn->GetActorLocation().X < this->AvalancheX)
		{
			if (LastDmgTime == 0)
				{
					LastDmgTime = FPlatformTime::Seconds();
					return true;
				}
			// 正常情况下
			if (FPlatformTime::Seconds() - LastDmgTime >= 1)
			{
				LastDmgTime = FPlatformTime::Seconds();
				return true;
			}
		}
		return false;
	};
	if (NeedDamage())
	{
		UGameplayStatics::ApplyDamage(
		  PlayerPawn,             // DamagedActor: 要对谁造成伤害
		  AvalancheAccDmg,                  // BaseDamage: 伤害的基础数值
		  nullptr,                // EventInstigator: 造成伤害的控制器 (这里没有，所以是nullptr)
		  AvalancheActor,                   // DamageCauser: 造成伤害的Actor (这里是雪崩管理器自身)
		  UAvalancheDamageType::StaticClass()
	  );
		
	}
}

TStatId UAvalancheManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAvalancheManager, STATGROUP_Tickables);

}


