#include "RoomActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"


ARoomActor::ARoomActor()
{
    PrimaryActorTick.bCanEverTick = false;
    // 创建静态网格作为根组件
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SceneRoot->SetMobility(EComponentMobility::Static);
    RootComponent = SceneRoot;

    // 创建次级网格
    RoomColliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomColliderMesh"));
    RoomColliderMesh->SetupAttachment(SceneRoot);
}

#if WITH_EDITOR

void ARoomActor::AutoGenerateExitsInEditor()
{
    if (!SceneRoot) return;

    TArray<USceneComponent*> ChildrenToRemove;

    // 遍历 SceneRoot 下所有 Roomxx
    for (USceneComponent* RoomComp : SceneRoot->GetAttachChildren())
    {
        if (!RoomComp) continue;
        const FString RoomName = RoomComp->GetName();
        if (!RoomName.StartsWith("Room")) continue;

        // 1️⃣ 收集 Roomxx 的子组件
        TArray<USceneComponent*> SubChildren;
        RoomComp->GetChildrenComponents(true, SubChildren);

        // 2️⃣ 按后缀归类 FExitMeshData
        TMap<FString, FExitMeshData> GroupedExitData;
        bool bHasError = false;

        for (USceneComponent* SubChild : SubChildren)
        {
            if (!SubChild) continue;

            FString SubName = SubChild->GetName();

            // 必须有后缀，否则整个 Roomxx 出错
            FString Suffix;
            int32 UnderscoreIdx;
            if (SubName.FindLastChar('_', UnderscoreIdx))
            {
                Suffix = SubName.RightChop(UnderscoreIdx);
            }
            else
            {
                bHasError = true;
                UE_LOG(LogTemp, Warning, TEXT("Roomxx '%s' child '%s' has no suffix, skipping Roomxx"),
                    *RoomName, *SubName);
                break;
            }

            FExitMeshData& ExitData = GroupedExitData.FindOrAdd(Suffix);

            // 处理 Wall / Door Mesh
            if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(SubChild))
            {
                if (SubName.Contains("Wall"))
                {
                    ExitData.WallMesh = MeshComp->GetStaticMesh();
                    ExitData.WallTransform = MeshComp->GetComponentTransform();
                }
                else if (SubName.Contains("Door"))
                {
                    ExitData.HoleMesh = MeshComp->GetStaticMesh();
                    ExitData.HoleTransform = MeshComp->GetComponentTransform();
                }
            }
            // 只处理名字包含 DoorSnapPoint 的 SceneComponent 作为 Socket
            else if (SubName.Contains("DoorSnapPoint"))
            {
                ExitData.SocketTransform = SubChild->GetComponentTransform();
            }
        }

        if (bHasError)
        {
            continue; // 出现错误跳过整个 Roomxx
        }

        // 3️⃣ 检查完整性：WallMesh 和 HoleMesh 必须存在
        bool bRoomComplete = true;
        for (auto& Elem : GroupedExitData)
        {
            const FExitMeshData& Data = Elem.Value;
            if (!Data.WallMesh || !Data.HoleMesh)
            {
                bRoomComplete = false;
                UE_LOG(LogTemp, Warning, TEXT("Roomxx '%s' group '%s' incomplete, skipping deletion"),
                    *RoomName, *Elem.Key);
            }
        }

        // 4️⃣ 添加完整组到 ExitMeshes，并删除 Roomxx
        if (bRoomComplete)
        {
            for (auto& Elem : GroupedExitData)
            {
                Exits.Add(Elem.Value);
            }
            ChildrenToRemove.Add(RoomComp);
        }
    }

    // 5️⃣ 删除标记的 Roomxx
    for (USceneComponent* CompToRemove : ChildrenToRemove)
    {
        if (CompToRemove)
        {
            CompToRemove->DestroyComponent();
        }
    }

    Modify();

    UE_LOG(LogTemp, Log, TEXT("AutoGenerateExitsInEditor finished. Total ExitMeshes: %d"), Exits.Num());
}

#endif
