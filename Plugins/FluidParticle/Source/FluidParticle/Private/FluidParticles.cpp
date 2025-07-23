// FluidParticleActor.cpp

#include "FluidParticles.h"

AFluidParticleActor::AFluidParticleActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- 物理刚体设置 ---
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    RootComponent = SphereComponent;
    SphereComponent->SetSimulatePhysics(true);
    SphereComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
    SphereComponent->SetSphereRadius(10.0f);
	
    // --- 泡泡物理属性设置 ---
    // 1. 禁用标准重力
    SphereComponent->SetEnableGravity(false);

	
    // ... 调试网格体的设置保持不变 ...
    DebugMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMeshComponent"));
    DebugMeshComponent->SetupAttachment(RootComponent);
    DebugMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereVisualAsset.Succeeded())
    {
        DebugMeshComponent->SetStaticMesh(SphereVisualAsset.Object);
        DebugMeshComponent->SetRelativeScale3D(FVector(0.2f)); 
    }
}


void AFluidParticleActor::LaunchBubble(const FVector& InitialVelocity)
{
    if (SphereComponent)
    {
        // 使用AddImpulse施加一个瞬时冲量。
        // 最后的 'true' (bVelChange) 参数意味着这个冲量会直接改变速度，忽略物体的质量，正好符合“设置初速度”的需求。
        SphereComponent->AddImpulse(InitialVelocity, NAME_None, true);
    }
}
void AFluidParticleActor::BeginPlay()
{
    Super::BeginPlay();

    ParticleInfo.Color = FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand());
}

void AFluidParticleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AFluidParticleActor::Tick(float DeltaTime)
{
   
    Super::Tick(DeltaTime);

    // --- 自定义重力 ---
    // 如果物理模拟开启，就施加一个微弱的向下的力
    if (SphereComponent && SphereComponent->IsSimulatingPhysics())
    {
        // 定义一个微弱的向下的加速度
        const FVector SlowGravityAcceleration = FVector(0.f, 0.f, -50.f); // Z轴负方向是向下
		
        // F = m * a (力 = 质量 * 加速度)。我们用AddForce来施加力。
        SphereComponent->AddForce(SlowGravityAcceleration * SphereComponent->GetMass());
    }
	
    // ... 更新粒子信息到数据列表的逻辑保持不变 ...
    ParticleInfo.Position = GetActorLocation();
    ParticleInfo.Velocity = SphereComponent->GetPhysicsLinearVelocity();
    ParticleInfo.Radius = SphereComponent->GetScaledSphereRadius();
}