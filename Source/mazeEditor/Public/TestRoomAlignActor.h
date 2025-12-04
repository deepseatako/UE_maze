#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomActor.h"
#include "TestRoomAlignActor.generated.h"

UCLASS()
class MAZEEDITOR_API ATestRoomAlignActor : public AActor
{
    GENERATED_BODY()

public:
    ATestRoomAlignActor();

protected:
    virtual void BeginPlay() override;

public:
    // 要测试的两个房间类型
    UPROPERTY(EditAnywhere, Category = "Test")
    TSubclassOf<ARoomActor> RoomClass1;

    UPROPERTY(EditAnywhere, Category = "Test")
    TSubclassOf<ARoomActor> RoomClass2;

private:
    void AttachRoomByExit(ARoomActor* PrevRoom, ARoomActor* NewRoom);
};
