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
    TArray<UMaterialInterface*> WallMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    FTransform HoleTransform = FTransform::Identity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    UStaticMesh* HoleMesh = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    TArray<UMaterialInterface*> HoleMaterials;



    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    FTransform SocketTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    bool bUsed = false;
};

// ---------- 房间类 ----------
UCLASS()
class MAZE_API ARoomActor : public AActor
{
    GENERATED_BODY()

public:

    ARoomActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* RoomRootMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* RoomColliderMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
    TArray<FExitMeshData> Exits;

#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override
    {
        return true;
    }

    protected:
        virtual void Tick(float DeltaTime) override;
#endif
};
