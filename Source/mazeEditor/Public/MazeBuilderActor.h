#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomActor.h"
#include "MazeBuilderActor.generated.h"

UCLASS()
class MAZEEDITOR_API AMazeBuilderActor : public AActor
{
    GENERATED_BODY()

public:
    AMazeBuilderActor();

    // 面板中设置 Room1、Room2、Room3 ...
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
    TArray<TSubclassOf<ARoomActor>> RoomSequence;

    // 生成好的房间
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<ARoomActor*> SpawnedRooms;

protected:
    virtual void BeginPlay() override;

    ARoomActor* SpawnRoom(TSubclassOf<ARoomActor> RoomClass, const FTransform& SpawnTransform);
    void AttachRoomByExit(ARoomActor* PrevRoom, ARoomActor* NewRoom);
    void BuildExitMeshes(ARoomActor* Room);
};
