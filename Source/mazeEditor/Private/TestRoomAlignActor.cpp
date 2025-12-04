#include "TestRoomAlignActor.h"
#include "Engine/World.h"
#include "Algo/RandomShuffle.h"

ATestRoomAlignActor::ATestRoomAlignActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATestRoomAlignActor::BeginPlay()
{
    Super::BeginPlay();

    if (!RoomClass1 || !RoomClass2) return;

    // ---------- 生成第一个房间 ----------
    ARoomActor* Room1 = GetWorld()->SpawnActor<ARoomActor>(RoomClass1, GetActorTransform());
    if (!Room1) return;

    // ---------- 生成第二个房间 ----------
    ARoomActor* Room2 = GetWorld()->SpawnActor<ARoomActor>(RoomClass2, FTransform::Identity);
    if (!Room2) return;

    // ---------- 对齐房间 ----------
    AttachRoomByExit(Room1, Room2);

    // 生成门

    UE_LOG(LogTemp, Log, TEXT("TestRoomAlignActor: Two rooms spawned and aligned."));
}

void ATestRoomAlignActor::AttachRoomByExit(ARoomActor* PrevRoom, ARoomActor* NewRoom)
{
    if (!PrevRoom || !NewRoom) return;

    FExitMeshData& PrevExit = PrevRoom->Exits[RoomClass1ExitInt];
    FExitMeshData& NewExit = NewRoom->Exits[RoomClass2ExitInt];

    PrevExit.bUsed = true;
    NewExit.bUsed = true;

    //--------------------------------------------------------------------
    // Step 1：得到两个 socket 的世界变换
    //--------------------------------------------------------------------
    const FTransform PrevExitWorld = GetSocketWorld(PrevRoom, PrevExit.SocketTransform);
    const FTransform NewExitWorld = GetSocketWorld(NewRoom, NewExit.SocketTransform);

    //--------------------------------------------------------------------
    // Step 2：计算 NewRoom 需要的世界旋转（让出口对准）
    //--------------------------------------------------------------------
    // PrevExit forward（世界方向）
    const FVector PrevForward = PrevExitWorld.GetRotation().GetForwardVector();
    const FVector NewForward = NewExitWorld.GetRotation().GetForwardVector();

    // NewExit.forward → -PrevExit.forward
    const FQuat RotDelta = FQuat::FindBetween(NewForward, -PrevForward);

    //--------------------------------------------------------------------
    // Step 3：旋转 NewRoom（仅旋转，不改位置）
    //--------------------------------------------------------------------
    FTransform Target = NewRoom->GetActorTransform();
    Target.ConcatenateRotation(RotDelta);

    //--------------------------------------------------------------------
    // Step 4：再次计算“旋转后”的 NewExit 世界位置
    //--------------------------------------------------------------------
    const FVector NewExitPos_Rotated =
        Target.TransformPosition(NewExit.SocketTransform.GetLocation());

    //--------------------------------------------------------------------
    // Step 5：平移，使两个 socket 的位置相等
    //--------------------------------------------------------------------
    const FVector Delta = PrevExitWorld.GetLocation() - NewExitPos_Rotated;
    Target.AddToTranslation(Delta);

    //--------------------------------------------------------------------
    // Step 6：设置最终变换
    //--------------------------------------------------------------------
    NewRoom->SetActorTransform(Target);

    //--------------------------------------------------------------------
    // Step 7：生成出口 mesh
    //--------------------------------------------------------------------
    BuildExitMeshes(PrevRoom);
    BuildExitMeshes(NewRoom);
}


FTransform ATestRoomAlignActor::GetSocketWorld(const AActor* Room, const FTransform& SocketLocal)
{
    return SocketLocal * Room->GetActorTransform();
}


void ATestRoomAlignActor::BuildExitMeshes(ARoomActor* Room)
{
    if (!Room) return;

    for (FExitMeshData& ExitData : Room->Exits)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Room);
        MeshComp->RegisterComponent();
        MeshComp->AttachToComponent(Room->RoomRootMesh, FAttachmentTransformRules::KeepRelativeTransform);

        if (ExitData.bUsed)
        {
            // --- 洞口 ---
            MeshComp->SetRelativeTransform(ExitData.HoleTransform);
            MeshComp->SetStaticMesh(ExitData.HoleMesh);

            for (int i = 0; i < ExitData.HoleMaterials.Num(); i++)
                MeshComp->SetMaterial(i, ExitData.HoleMaterials[i]);
        }
        else
        {
            // --- 墙 ---
            MeshComp->SetRelativeTransform(ExitData.WallTransform);
            MeshComp->SetStaticMesh(ExitData.WallMesh);

            for (int i = 0; i < ExitData.WallMaterials.Num(); i++)
                MeshComp->SetMaterial(i, ExitData.WallMaterials[i]);
        }
    }
}
