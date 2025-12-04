#include "RoomActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"


ARoomActor::ARoomActor()
{
    PrimaryActorTick.bCanEverTick = false;
    // 创建静态网格作为根组件
    RoomRootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomRootMesh"));
    //RoomRootMesh->SetMobility(EComponentMobility::Static);
    RootComponent = RoomRootMesh;

    // 创建次级网格
    RoomColliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomColliderMesh"));
    RoomColliderMesh->SetupAttachment(RoomRootMesh);
    // 设置为不可见
    RoomColliderMesh->SetVisibility(false);
    RoomColliderMesh->SetHiddenInGame(true);
    // 设置为 Trigger 并使用 Mesh 的碰撞体
    RoomColliderMesh->SetCollisionProfileName(TEXT("Trigger"));

#if WITH_EDITOR
    PrimaryActorTick.bCanEverTick = true;
#endif

}

#if WITH_EDITOR
void ARoomActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UWorld* World = GetWorld();
    if (!World) return;

    // 给不同出口准备几种颜色（可自由扩展）
    static const TArray<FColor> Colors = {
        FColor::Red,
        FColor::Green,
        FColor::Blue,
        FColor::Yellow,
        FColor::Cyan,
        FColor::Magenta,
        FColor::Orange,
        FColor::Purple,
        FColor::Turquoise
    };

    for (int32 i = 0; i < Exits.Num(); i++)
    {
        const FExitMeshData& Data = Exits[i];

        // 转世界空间
        const FTransform WorldTransform = Data.SocketTransform * GetActorTransform();
        const FVector Pos = WorldTransform.GetLocation();
        const FVector Forward = WorldTransform.GetRotation().GetForwardVector();
        const FVector End = Pos + Forward * 50.f;

        // 取颜色（如果出口超过颜色数量就循环用）
        const FColor& ArrowColor = Colors[i % Colors.Num()];

        DrawDebugDirectionalArrow(
            World,
            Pos,
            End,
            30.f,
            ArrowColor,
            false,  // 每帧绘制，不需要 persistent
            -1.f,
            0,
            2.5f
        );
    }
}

#endif



