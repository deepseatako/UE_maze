#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

class ARoomActor;
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

private:
    TArray<ARoomActor*> PlacedRooms;

    bool TryPlaceRoom(TSubclassOf<ARoomActor> RoomClass, ARoomActor* PrevRoom, ARoomActor*& OutNewRoom);
    void CleanupAfterGeneration();
    void AlignRoom(ARoomActor* NewRoom, const FTransform& NewRoomSocket, const FTransform& PrevRoomSocket);
    bool IsRoomOverlapping(const ARoomActor* NewRoom);
    TSubclassOf<ARoomActor> PickRandomRoom(const TArray<TSubclassOf<ARoomActor>>& Choices);
};
