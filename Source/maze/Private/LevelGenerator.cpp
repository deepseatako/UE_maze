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
    GenerateLevel();
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

            // 可选：启用重叠检测
            //if (IsRoomOverlapping(NewRoom)) continue;
                // 标记出口已使用
            PrevExit.bUsed = true;
            NewExit.bUsed = true;

            PlacedRooms.Add(NewRoom);
            OutNewRoom = NewRoom;
            return true;
        }
    }

    NewRoom->Destroy();
    return false;
}


// --------------------
// Main generation loop
// --------------------
void ALevelGenerator::GenerateLevel()
{
    PlacedRooms.Empty();

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("World is null."));
        return;
    }

    if (RoomChoices.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No RoomChoices defined."));
        return;
    }

    // ---------- 生成第一间房 ----------
    TSubclassOf<ARoomActor> FirstRoomClass = PickRandomRoom(RoomChoices[0].RoomCandidates);
    if (!FirstRoomClass) return;

    FActorSpawnParameters Params;
    ARoomActor* PrevRoom = World->SpawnActor<ARoomActor>(FirstRoomClass, GetActorLocation(), GetActorRotation(), Params);
    if (!PrevRoom) return;

    PlacedRooms.Add(PrevRoom);

    // ---------- 从第二步开始生成 ----------
    for (int32 Step = 1; Step < RoomChoices.Num(); Step++)
    {
        const TArray<TSubclassOf<ARoomActor>>& Choices = RoomChoices[Step].RoomCandidates;
        TSubclassOf<ARoomActor> RoomClass = PickRandomRoom(Choices);
        if (!RoomClass) continue;

        ARoomActor* NewRoom = nullptr;
        bool bPlaced = false;

        for (int32 i = 0; i < MaxAttempts && !bPlaced; i++)
        {
            bPlaced = TryPlaceRoom(RoomClass, PrevRoom, NewRoom);
        }

        if (!bPlaced)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to place room at step %d after %d attempts"), Step, MaxAttempts);
            break;
        }

        PrevRoom = NewRoom;
    }

    UE_LOG(LogTemp, Log, TEXT("Level generation finished. Rooms: %d"), PlacedRooms.Num());
    CleanupAfterGeneration();
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
