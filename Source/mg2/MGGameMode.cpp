#include "MGGameMode.h"
#include "AvalancheManager.h"
#include "mg2Character.h"
#include "ObjectPoolWorldSubsystem.h"
#include "GameFramework/Character.h" // 引入Character头文件
#include "TerrainGenManager.h"
#include "TerrainGenActor.h"
#include "GameFramework/CharacterMovementComponent.h"

AMGGameMode::AMGGameMode()
{
	// --- 设置默认的玩家Pawn ---
	// 这是UE的标准做法，通过寻找一个特定的蓝图资源来设置玩家角色。
	// 你需要将下面的路径替换成你自己的第三人称角色蓝图的【引用路径】。
	// 你可以在内容浏览器中右键你的角色蓝图，选择“Copy Reference”来获取这个路径。
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		// 如果找不到指定的蓝图，可以设置一个默认的C++基类以防出错
		DefaultPawnClass = ACharacter::StaticClass();
	}
}

void AMGGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 将所有核心系统的初始化逻辑从 BeginPlay 移动到这里
	if (TerrainToSpawn)
	{
		auto* World = GetWorld();
		if (!World) return;

		auto Subsystem = World->GetSubsystem<UTerrainGenManager>();
		Subsystem->DefaultTerrainClass = TerrainToSpawn;
		Subsystem->CliffTerrain = TerrainCliff; // @todo 思考下这个类要专门留不
		Subsystem->GenCliffDensity = CliffProb;
		Subsystem->RegisterBPClass(TerrainToSpawn);
		
		Subsystem->GenDefault();
		auto Subsystem1 = World->GetSubsystem<UAvalancheManager>();
		Subsystem1->GenDefault();

		auto Subsystem2 = World->GetSubsystem<UObjectPoolWorldSubsystem>();
		TMap<UClass*, int32> p_PoolConfigs;
		TMap<UClass*, FString> Tags1;
		for (auto[a, b, c] : PoolConfig)
		{
			auto& a1 = p_PoolConfigs.FindOrAdd(a);
			a1 = c;
			auto& a2 = Tags1.FindOrAdd(a);
			a2 = b;
		}
		Subsystem2->SetPoolConfigs(p_PoolConfigs,Tags1);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TerrainToSpawn is not set in the GameMode Blueprint!"));
	}
	
}

void AMGGameMode::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	APawn* PlayerPawn = PlayerController->GetPawn();
	if (auto p = Cast<Amg2Character>(PlayerPawn) ; p != nullptr)
	{
		p->MaximumSpeed = PawnMaxSpeed;
		p->GetCharacterMovement()->MaxWalkSpeed = PawnMaxSpeed;
	}
}
