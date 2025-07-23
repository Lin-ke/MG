// Copyright Epic Games, Inc. All Rights Reserved.

#include "mg2Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "CineCameraComponent.h"
#include "TimerManager.h" // <<< 添加了此头文件用于计时器
#include "TerrainGenManager.h"
#include "../../Plugins/TerrainGen/Source/TerrainGen/Public/AvalancheManager.h"
#include "AvalancheManager.h"
#include "MGGameInstance.h"
#include "StaticMeshPoolableActor.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/GameModeBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// Amg2Character

Amg2Character::Amg2Character()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
       
    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    auto MoveComponent = GetCharacterMovement();
    MoveComponent->bOrientRotationToMovement = true; // Character moves in the direction of input...   
    MoveComponent->RotationRate = FRotator(0.0f, 400.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = MaximumSpeed;
    GetCharacterMovement()->MinAnalogWalkSpeed = MinimumSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
    
    // --- 空中移动设置 ---
    MoveComponent->JumpZVelocity = 1000.0f;
    MoveComponent->GravityScale = 1.7f;
    MoveComponent->FallingLateralFriction = 0.2f;
    MoveComponent->MaxAcceleration = 2048.0f;
    MoveComponent->GroundFriction = 6.0;
    
    /// Camera在这里！
    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character  
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    PrimaryActorTick.bCanEverTick = true;
    CurrentRunSpeed = MinimumSpeed;

    /// speed bonus system
    SpeedBonusTimerHandle.Init({}, 10);

    /// damage/ health system
    DamageSystem = CreateDefaultSubobject<UDamageSystemComponent>(TEXT("Damage Component"));
    
}

void Amg2Character::BeginPlay()
{
    // Call the base class
    
    Super::BeginPlay();
    if (GetWorld())
    {
       Mgr = UWorld::GetSubsystem<UTerrainGenManager>(GetWorld());
    }
}

void Amg2Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ShopMode) return;
    if (GetActorLocation().Z < Die)
    {
        DamageSystem->HandleDamage(-1, UDamageType::StaticClass(), nullptr, nullptr);
    }

    CurrentRunSpeed += AccelerationRate * DeltaTime;
    CurrentRunSpeed = FMath::Clamp(CurrentRunSpeed, MinimumSpeed,  MaximumSpeed);

    AddMovementInput(GetActorForwardVector(), CurrentRunSpeed);    
    
    FString DebugMessage = FString::Printf(TEXT("Character Velocity: %s, Location: %s"), *GetVelocity().ToString(), *GetActorLocation().ToString());
    GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Yellow, DebugMessage);
    CurrFrame ++;
}

void Amg2Character::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    MovingState = 0;
}

void Amg2Character::Jump(const FInputActionValue& Value)
{
    Super::Jump();
    MovingState = 2;
}

float Amg2Character::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (!DamageSystem ) return 0;


    
    if (DamageEvent.DamageTypeClass == UAvalancheDamageType::StaticClass() && MovingState == 2)// 空中不收到地面伤害
    {
       UE_LOG(LogTemp, Display, TEXT("空中不受伤"));
       return 0;
    }
    else if (DamageEvent.DamageTypeClass == UMeshHitDamageType::StaticClass())
    {
       HitSlowVelocity();
    }
    else if (DamageEvent.DamageTypeClass == UMeshHitBonusType::StaticClass())
    {
        auto Instance = Cast<UMGGameInstance>(GetGameInstance());
        if (Instance)
        {
            Instance->AddCoins(1);
        };
        return 0;
    }
    // --- 调用闪烁效果 ---
    StartFlashing();
    DamageSystem->HandleDamage(DamageAmount, DamageEvent.DamageTypeClass, EventInstigator, DamageCauser);
    return DamageAmount;
}

void Amg2Character::StraightVelocity()
{
    UPawnMovementComponent* MovementComp = GetMovementComponent();
    if (MovementComp)
    {
       const FVector ForwardDirection = GetActorForwardVector();
       const FVector CurrentVelocity = MovementComp->Velocity;
       const float ForwardSpeed = FVector::DotProduct(CurrentVelocity, ForwardDirection);
       const FVector CorrectedVelocity = ForwardDirection * ForwardSpeed;
       MovementComp->Velocity = {CorrectedVelocity.X , 0 , 0 };

       bVelocityStraight = true;

    }
    else
    {
       int x = 0; // strange error.
    }
}

void Amg2Character::HitSlowVelocity()
{
    CurrentRunSpeed *= HitSlowdownFactor;
    CurrentRunSpeed = FMath::Max(CurrentRunSpeed, MinimumSpeed);
    FString DebugMessage = FString::Printf(TEXT("Character hit an obstacle"));
    GEngine->AddOnScreenDebugMessage(114520, 5.0f, FColor::Yellow, DebugMessage);
}

void Amg2Character::BonusVelocity(BonusInfo Info)
{
    // UCharacterMovementComponent* MoveComponent = GetCharacterMovement();
    // if (!MoveComponent)
    // {
    //     return; // 获取组件失败则直接返回
    // }
    //
    //
    // GetWorld()->GetTimerManager().ClearTimer(SpeedBonusTimerHandle);
    //
    // // --- 步骤2: 应用瞬时效果 ---
    // // 提升最大速度上限
    // MoveComponent->MaxWalkSpeed = Info.MaxWalkVelocityBonus;
    //
    // bIsSpeedBonusActive = true;
    // CurrentBonusThrust = Info.BonusThrustStrength;
    //
    // UE_LOG(LogTemp, Log, TEXT("Speed bonus ACTIVATED. New Max Speed: %.1f, Thrust: %.1f"), Info.MaxWalkVelocityBonus, Info.BonusThrustStrength);
    //
    // // --- 步骤4: 启动新的计时器，用于在未来恢复状态 ---
    // // 创建一个计时器委托，绑定到我们的EndSpeedBonus函数
    // FTimerDelegate TimerDelegate;
    // TimerDelegate.BindUFunction(this, FName("EndSpeedBonus"));
    //
    // // 设置计时器
    // GetWorld()->GetTimerManager().SetTimer(
    //     SpeedBonusTimerHandle.Last(),
    //     TimerDelegate,
    //     Info.BonusLastTime,
    //     false // false表示不循环
    // );
}


void Amg2Character::StartFlashing()
{
    GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(StopFlashTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(FlashTimerHandle, this, &Amg2Character::FlashEffect, FlashRate, true, 0.0f);
    GetWorld()->GetTimerManager().SetTimer(StopFlashTimerHandle, this, &Amg2Character::StopFlashing, FlashDuration, false);
}

void Amg2Character::FlashEffect()
{
    if (GetMesh())
    {
        // 直接切换角色主网格体的可见性
        GetMesh()->SetVisibility(!GetMesh()->IsVisible());
    }
}

void Amg2Character::StopFlashing()
{
    // 停止闪烁循环
    GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle);

    // 确保在效果结束时，角色网格体是可见的
    if (GetMesh())
    {
        GetMesh()->SetVisibility(true);
    }
}

//////////////////////////////////////////////////////////////////////////
// Input

void Amg2Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
       if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
       {
          Subsystem->AddMappingContext(DefaultMappingContext, 0);
       }
    }
    
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
       
       // Jumping
       // --- 修正了跳跃绑定，使其调用我们自己的Jump函数 ---
       EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &Amg2Character::Jump);
       EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

       // Moving
       EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Amg2Character::Move);

       // Looking
       EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Amg2Character::Look);
    }
    else
    {
       UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

void Amg2Character::Move(const FInputActionValue& Value)
{

    if (!Controller) return; 
    // input is a Vector2D

    FVector2D MovementVector = Value.Get<FVector2D>();

    
    if (!ShopMode)
    {
       // find out which way is forward
       const FRotator Rotation = Controller->GetControlRotation();
       const FRotator YawRotation(0, Rotation.Yaw, 0);

       FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        float Multiplier = SidewaysMovementMultiplier;
        float currSideSpeed = GetVelocity().Y;
        FString DebugMessage = FString::Printf(TEXT("Character Velocity: %s, Location: %s"), *MovementVector.ToString(), *GetOwner()->GetName());
        GEngine->AddOnScreenDebugMessage(1123121, 5.0f, FColor::Yellow, DebugMessage);
        if (MovementVector.X * currSideSpeed < 0)
        {
            Multiplier *= SidewaysMovementMultiplierBonus;
        }
        else
        {
        // 需要根据速度的绝对大小做衰减
            Multiplier = FMath::Clamp(SidewaysMovementMultiplier - SidewaysMovementMultiplier / SidewaysMovementMaxSpeed * FMath::Abs(currSideSpeed), 0, SidewaysMovementMultiplier);
        }
        RightDirection *= Multiplier; /// always 0, 10, 0
        
        AddMovementInput(RightDirection, MovementVector.X);
    }
    else
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(RightDirection, MovementVector.X);
        AddMovementInput(ForwardDirection, MovementVector.Y);
    }
    
}

void Amg2Character::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
       // add yaw and pitch input to controller
       AddControllerYawInput(LookAxisVector.X);
       AddControllerPitchInput(LookAxisVector.Y);
    }
}
