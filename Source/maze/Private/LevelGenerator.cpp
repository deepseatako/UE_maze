#include "LevelGenerator.h"
#include "RoomActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

#include "Components/ArrowComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Algo/RandomShuffle.h"

// --------------------
// Constructor
// --------------------
ALevelGenerator::ALevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALevelGenerator::BeginPlay()
{
    Super::BeginPlay();
    RegenerateLevel();
    CleanupAfterGeneration();
    for (ARoomActor* Room : PlacedRooms)
    {
        BuildExitMeshes(Room);
        //UE_LOG(LogTemp, Warning, TEXT("BuildExitMeshes() 被调用"));
    }
}

// --------------------
// Pick random room from choices
// --------------------
TSubclassOf<ARoomActor> ALevelGenerator::PickRandomRoom(const TArray<TSubclassOf<ARoomActor>>& Choices)
{
    if (Choices.Num() == 0) return nullptr;

    TArray<TSubclassOf<ARoomActor>> Shuffled = Choices;
    Algo::RandomShuffle(Shuffled);
    return Shuffled[0];
}

// --------------------
// Align Room to socket
// --------------------

void ALevelGenerator::AlignRoom(ARoomActor* PrevRoom, ARoomActor* NewRoom, FExitMeshData& PrevExit, FExitMeshData& NewExit)
{
    if (!PrevRoom || !NewRoom) return;

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
}


// --------------------
// Try place room next to previous
// --------------------
bool ALevelGenerator::TryPlaceRoom(TSubclassOf<ARoomActor> RoomClass, ARoomActor* PrevRoom, ARoomActor*& OutNewRoom)
{
    if (!RoomClass || !PrevRoom) return false;
    UWorld* World = GetWorld();
    if (!World) return false;

    ARoomActor* NewRoom = World->SpawnActor<ARoomActor>(RoomClass);
    if (!NewRoom) return false;

    // 洗牌出口
    TArray<int32> PrevIndices, NewIndices;
    PrevIndices.SetNum(PrevRoom->Exits.Num());
    NewIndices.SetNum(NewRoom->Exits.Num());
    for (int32 i = 0; i < PrevIndices.Num(); i++) PrevIndices[i] = i;
    for (int32 i = 0; i < NewIndices.Num(); i++) NewIndices[i] = i;
    Algo::RandomShuffle(PrevIndices);
    Algo::RandomShuffle(NewIndices);

    for (int32 iPrev : PrevIndices)
    {
        FExitMeshData& PrevExit = PrevRoom->Exits[iPrev];
        if (PrevExit.bUsed) continue;

        for (int32 iNew : NewIndices)
        {
            FExitMeshData& NewExit = NewRoom->Exits[iNew];
            if (NewExit.bUsed) continue;

            AlignRoom(PrevRoom, NewRoom, PrevExit, NewExit);

            // 重叠检测
            if (IsRoomOverlapping(NewRoom))
            {
                UE_LOG(LogTemp, Warning, TEXT("Overlapping"));
                continue;
            }
            PrevExit.bUsed = true;
            NewExit.bUsed = true;

            PlacedRooms.Add(NewRoom);
            OutNewRoom = NewRoom;
            PlaceDoorBetween(PrevRoom, NewRoom, PrevExit, NewExit);
            return true;
        }
    }

    NewRoom->Destroy();
    return false;
}


void ALevelGenerator::RegenerateLevel()
{
    for (int32 Attempt = 1; Attempt <= MaxAttempts; Attempt++)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== Level Generation Attempt %d ==="), Attempt);

        bool bSuccess = GenerateLevelOnce();

        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Level generated successfully."));
            return;
        }

        // 清理失败时已经生成的房间
        for (ARoomActor* Room : PlacedRooms)
        {
            if (Room)
                Room->Destroy();
        }
    }

    UE_LOG(LogTemp, Error, TEXT("All attempts failed. Level generation aborted."));
}


// --------------------
// Main generation loop
// --------------------
bool ALevelGenerator::GenerateLevelOnce()
{
    PlacedRooms.Empty();

    UWorld* World = GetWorld();
    if (!World) return false;

    if (RoomChoices.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No RoomChoices defined."));
        return false;
    }

    // ---------- 生成第一个房间 ----------
    TSubclassOf<ARoomActor> FirstRoomClass = PickRandomRoom(RoomChoices[0].RoomCandidates);
    if (!FirstRoomClass) return false;

    ARoomActor* PrevRoom = World->SpawnActor<ARoomActor>(FirstRoomClass, GetActorLocation(), GetActorRotation());
    if (!PrevRoom) return false;

    PlacedRooms.Add(PrevRoom);

    // ---------- 后续房间 ----------
    for (int32 Step = 1; Step < RoomChoices.Num(); Step++)
    {
        const auto& Choices = RoomChoices[Step].RoomCandidates;
        TSubclassOf<ARoomActor> RoomClass = PickRandomRoom(Choices);
        if (!RoomClass) continue;

        ARoomActor* NewRoom = nullptr;
        bool bPlaced = TryPlaceRoom(RoomClass, PrevRoom, NewRoom);

        if (!bPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to place room at Step %d"), Step);
            return false;
        }

        PrevRoom = NewRoom;
    }

    return true;
}


// --------------------
// Cleanup colliders and self-destruct
// --------------------
void ALevelGenerator::CleanupAfterGeneration()
{
    for (ARoomActor* Room : PlacedRooms)
    {
        if (Room && Room->RoomColliderMesh)
        {
            Room->RoomColliderMesh->DestroyComponent();
        }
    }

    //Destroy();
}

// --------------------
// Overlap check
// --------------------
bool ALevelGenerator::IsRoomOverlapping(const ARoomActor* NewRoom)
{
    if (!NewRoom || !NewRoom->RoomColliderMesh) return false;

    UPrimitiveComponent* Collider = NewRoom->RoomColliderMesh;
    TArray<FOverlapResult> Overlaps;
    FComponentQueryParams Params;
    Params.AddIgnoredActor(NewRoom);
    FCollisionObjectQueryParams ObjectParams = FCollisionObjectQueryParams::AllObjects;

    bool bHit = Collider->ComponentOverlapMulti(
        Overlaps,
        GetWorld(),
        Collider->GetComponentLocation(),
        Collider->GetComponentQuat(),
        ECC_WorldDynamic,
        Params,
        ObjectParams
    );

    for (const FOverlapResult& Hit : Overlaps)
    {
        if (ARoomActor* OtherRoom = Cast<ARoomActor>(Hit.GetActor()))
        {
            if (OtherRoom != NewRoom) return true;
        }
    }

    return false;
}

FTransform ALevelGenerator::GetSocketWorld(const AActor* Room, const FTransform& SocketLocal)
{
    return SocketLocal * Room->GetActorTransform();
}

void ALevelGenerator::BuildExitMeshes(ARoomActor* Room)
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

void ALevelGenerator::PlaceDoorBetween(
    ARoomActor* PrevRoom,
    ARoomActor* NewRoom,
    const FExitMeshData& PrevExit,
    const FExitMeshData& NewExit
)
{
    if (!PrevRoom || !NewRoom || !DoorClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // -------- 1. 出口世界变换 --------
    const FTransform PrevWorld = GetSocketWorld(PrevRoom, PrevExit.SocketTransform);
    const FTransform NewWorld = GetSocketWorld(NewRoom, NewExit.SocketTransform);

    const FVector P0 = PrevWorld.GetLocation();
    const FVector P1 = NewWorld.GetLocation();

    // -------- 2. 中点 --------
    const FVector MidPoint = (P0 + P1) * 0.5f;

    // -------- 3. 朝向 --------
    const FRotator Rot = PrevWorld.GetRotation().Rotator();

    // -------- 4. 创建门 --------
    AActor* Door = World->SpawnActor<AActor>(DoorClass, MidPoint, Rot);

    if (!Door)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn Door!"));
        return;
    }
}

