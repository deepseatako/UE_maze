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

void ARoomActor::SpawnAllFurnitures()
{
    if (Furnitures.Num() == 0) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FTransform RoomTransform = GetActorTransform();

    for (const FFurnitureData& FurnitureData : Furnitures)
    {
        if (!FurnitureData.Blueprint.IsValid()) continue;

        UClass* FurnitureClass = FurnitureData.Blueprint.LoadSynchronous();
        if (!FurnitureClass) continue;

        FVector SpawnLocation = RoomTransform.TransformPosition(FurnitureData.Transform.GetLocation());
        FRotator SpawnRotation = (FurnitureData.Transform.GetRotation() * RoomTransform.GetRotation()).Rotator();
        FVector SpawnScale = FurnitureData.Transform.GetScale3D() * RoomTransform.GetScale3D();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* SpawnedFurniture = World->SpawnActor<AActor>(
            FurnitureClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams
        );

        if (SpawnedFurniture)
        {
            SpawnedFurniture->SetActorScale3D(SpawnScale);
        }
    }
}


#if WITH_EDITOR
void ARoomActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UWorld* World = GetWorld();
    if (!World) return;

    // ---------- 绘制出口箭头 ----------
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

        const FTransform WorldTransform = Data.SocketTransform * GetActorTransform();
        const FVector Pos = WorldTransform.GetLocation();
        const FVector Forward = WorldTransform.GetRotation().GetForwardVector();
        const FVector End = Pos + Forward * 50.f;

        const FColor& ArrowColor = Colors[i % Colors.Num()];

        DrawDebugDirectionalArrow(
            World,
            Pos,
            End,
            30.f,
            ArrowColor,
            false,
            -1.f,
            0,
            2.5f
        );
    }

    // ---------- 绘制 Furnitures ----------
    for (int32 i = 0; i < Furnitures.Num(); i++)
    {
        const FFurnitureData& Furniture = Furnitures[i];

        // 转到世界坐标
        const FTransform WorldTransform = Furniture.Transform * GetActorTransform();
        const FVector Pos = WorldTransform.GetLocation();
        const FVector Forward = WorldTransform.GetRotation().GetForwardVector();
        const FVector End = Pos + Forward * 5.f; // 箭头长度，可根据需要调整

        // 可用红色表示家具
        DrawDebugDirectionalArrow(
            World,
            Pos,
            End,
            20.f,          // 箭头大小
            FColor::Red,   // 箭头颜色
            false,         // 不持久
            -1.f,          // 持续时间
            0,
            2.0f           // 线条粗细
        );
    }
}
#endif



