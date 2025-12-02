#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomExitComponent.h"
#include "RoomActor.generated.h"

// ---------- 结构体定义 ----------
USTRUCT(BlueprintType)
struct FExitMeshData
{
    GENERATED_BODY()


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    FTransform WallTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    UStaticMesh* WallMesh = nullptr;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    FTransform HoleTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    UStaticMesh* HoleMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    FTransform SocketTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    bool bUsed = false;
};

UCLASS()
class MAZE_API ARoomActor : public AActor
{
    GENERATED_BODY()

public:

    ARoomActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* RoomColliderMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UActorComponent*> Components;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    TArray<FExitMeshData> Exits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    float test;


#if WITH_EDITOR
private:
    void AutoGenerateExitsInEditor();
#endif
};
