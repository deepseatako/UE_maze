#include "RoomActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"


ARoomActor::ARoomActor()
{
    PrimaryActorTick.bCanEverTick = false;
    // 创建静态网格作为根组件
    RoomRootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomRootMesh"));
    RoomRootMesh->SetMobility(EComponentMobility::Static);
    RootComponent = RoomRootMesh;

    // 创建次级网格
    RoomColliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomColliderMesh"));
    RoomColliderMesh->SetupAttachment(RoomRootMesh);
    // 设置为不可见
    RoomColliderMesh->SetVisibility(false);
    RoomColliderMesh->SetHiddenInGame(true);
    // 设置为 Trigger 并使用 Mesh 的碰撞体
    RoomColliderMesh->SetCollisionProfileName(TEXT("Trigger"));
}

#if WITH_EDITOR
#include "DrawDebugHelpers.h"

void ARoomActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* World = GetWorld();
    if (!World) return;

    // Clear previously drawn persistent debug objects
    FlushPersistentDebugLines(World);

    for (int32 i = 0; i < Exits.Num(); i++)
    {
        const FExitMeshData& Data = Exits[i];

        const FTransform WorldTransform = Data.SocketTransform * GetActorTransform();
        const FVector Pos = WorldTransform.GetLocation();
        const FVector Forward = WorldTransform.GetRotation().GetForwardVector();

        const FVector End = Pos + Forward * 50.f;   // Arrow length

        DrawDebugDirectionalArrow(
            World,
            Pos,         // Start
            End,         // End
            30.f,        // Arrowhead size
            FColor::Green,
            true,        // Persistent
            0.f,         // Lifetime: 0 = forever when persistent = true
            0,           // Depth priority
            2.5f         // Thickness
        );
    }
}
#endif


