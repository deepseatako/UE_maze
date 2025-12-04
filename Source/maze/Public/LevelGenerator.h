#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

class ARoomActor;
struct FExitMeshData;
class UArrowComponent;

USTRUCT(BlueprintType)
struct FRoomStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Generation")
    TArray<TSubclassOf<ARoomActor>> RoomCandidates;
};

UCLASS()
class MAZE_API ALevelGenerator : public AActor
{
    GENERATED_BODY()

public:
    ALevelGenerator();

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TArray<FRoomStep> RoomChoices;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    int32 MaxAttempts = 100;

    UFUNCTION(CallInEditor, Category = "Level Generation")
    void GenerateLevel();

protected:
    virtual void BeginPlay() override;
private:
    TArray<ARoomActor*> PlacedRooms;

    bool TryPlaceRoom(TSubclassOf<ARoomActor> RoomClass, ARoomActor* PrevRoom, ARoomActor*& OutNewRoom);
    void CleanupAfterGeneration();
    bool IsRoomOverlapping(const ARoomActor* NewRoom);
    TSubclassOf<ARoomActor> PickRandomRoom(const TArray<TSubclassOf<ARoomActor>>& Choices);
    void AlignRoom(ARoomActor* PrevRoom, ARoomActor* NewRoom, FExitMeshData& PrevExit, FExitMeshData& NewExit);
    FTransform GetSocketWorld(const AActor* Room, const FTransform& SocketLocal);
};
