#include "MazeBuilderActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Algo/RandomShuffle.h"

AMazeBuilderActor::AMazeBuilderActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMazeBuilderActor::BeginPlay()
{
    Super::BeginPlay();

    if (RoomSequence.Num() == 0) return;

    // ---------- 先生成 Room1 ----------
    FTransform RootTf = GetActorTransform();
    ARoomActor* FirstRoom = SpawnRoom(RoomSequence[0], RootTf);
    SpawnedRooms.Add(FirstRoom);

    ARoomActor* PrevRoom = FirstRoom;

    // ---------- 按顺序生成并连接其他房间 ----------
    for (int i = 1; i < RoomSequence.Num(); i++)
    {
        ARoomActor* NewRoom = SpawnRoom(RoomSequence[i], FTransform::Identity);
        if (!NewRoom) continue;

        AttachRoomByExit(PrevRoom, NewRoom);

        SpawnedRooms.Add(NewRoom);
        PrevRoom = NewRoom;
    }

    // ---------- 创建 Exit Mesh（墙/洞） ----------
    for (ARoomActor* Room : SpawnedRooms)
    {
        BuildExitMeshes(Room);
        UE_LOG(LogTemp, Warning, TEXT("BuildExitMeshes() 被调用"));
    }
}

// =======================================================
// 生成房间 Actor
// =======================================================
ARoomActor* AMazeBuilderActor::SpawnRoom(TSubclassOf<ARoomActor> RoomClass, const FTransform& SpawnTransform)
{
    if (!RoomClass) return nullptr;

    return GetWorld()->SpawnActor<ARoomActor>(RoomClass, SpawnTransform);
}

// =======================================================
// 连接房间：随机选择一个 Exit，利用 SocketTransform 对齐
// =======================================================

void AMazeBuilderActor::AttachRoomByExit(ARoomActor* PrevRoom, ARoomActor* NewRoom)
{
    if (!PrevRoom || !NewRoom) return;
    if (PrevRoom->Exits.Num() == 0 || NewRoom->Exits.Num() == 0) return;

    // ---------- 洗牌索引 ----------
    TArray<int32> PrevIndices;
    for (int32 i = 0; i < PrevRoom->Exits.Num(); i++) PrevIndices.Add(i);
    Algo::RandomShuffle(PrevIndices);

    TArray<int32> NewIndices;
    for (int32 i = 0; i < NewRoom->Exits.Num(); i++) NewIndices.Add(i);
    Algo::RandomShuffle(NewIndices);

    // ---------- 找到未使用的 Exit ----------
    FExitMeshData* PrevExit = nullptr;
    for (int32 Idx : PrevIndices)
    {
        if (!PrevRoom->Exits[Idx].bUsed)
        {
            PrevExit = &PrevRoom->Exits[Idx];
            PrevExit->bUsed = true;
            break;
        }
    }

    FExitMeshData* NewExit = nullptr;
    for (int32 Idx : NewIndices)
    {
        if (!NewRoom->Exits[Idx].bUsed)
        {
            NewExit = &NewRoom->Exits[Idx];
            NewExit->bUsed = true;
            break;
        }
    }

    if (!PrevExit || !NewExit)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available Exit to connect rooms!"));
        return;
    }

    // ---------- 计算 Actor 世界变换 ----------

    // 上一个房间出口的世界变换
    FTransform PrevExitWorld = PrevRoom->RoomRootMesh->GetComponentTransform() * PrevExit->HoleTransform;

    // 新房间出口的世界变换（初始位置）
    FTransform NewExitWorld = NewRoom->RoomRootMesh->GetComponentTransform() * NewExit->HoleTransform;

    // 计算需要的偏移
    FVector DeltaLocation = PrevExitWorld.GetLocation() - NewExitWorld.GetLocation();
    FQuat DeltaRotation = PrevExitWorld.GetRotation() * NewExitWorld.GetRotation().Inverse();

    // 移动新房间 Actor
    NewRoom->AddActorWorldOffset(DeltaLocation);
    NewRoom->AddActorWorldRotation(DeltaRotation);
}



// =======================================================
// 根据 bUsed 创建 Mesh（墙 或 洞）
// =======================================================
void AMazeBuilderActor::BuildExitMeshes(ARoomActor* Room)
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
