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

    UE_LOG(LogTemp, Log, TEXT("TestRoomAlignActor: Two rooms spawned and aligned."));
}

// ---------- 使用前面修正版的对齐方法 ----------
void ATestRoomAlignActor::AttachRoomByExit(ARoomActor* PrevRoom, ARoomActor* NewRoom)
{
    if (!PrevRoom || !NewRoom) return;
    if (PrevRoom->Exits.Num() == 0 || NewRoom->Exits.Num() == 0) return;

    // 使用第一个出口测试
    FExitMeshData& PrevExit = PrevRoom->Exits[0];
    FExitMeshData& NewExit = NewRoom->Exits[0];

    PrevExit.bUsed = true;
    NewExit.bUsed = true;

    if (!PrevRoom->RoomRootMesh || !NewRoom->RoomRootMesh) return;

    // ---------- 核心对齐 ----------
    FTransform PrevExitWorld = PrevRoom->RoomRootMesh->GetComponentTransform() * PrevExit.HoleTransform;
    FTransform NewExitLocal = NewExit.HoleTransform;

    FTransform NewRoomWorldTransform = PrevExitWorld * NewExitLocal.Inverse();

    NewRoom->SetActorTransform(NewRoomWorldTransform);

    UE_LOG(LogTemp, Log, TEXT("Room2 aligned to Room1"));
}
