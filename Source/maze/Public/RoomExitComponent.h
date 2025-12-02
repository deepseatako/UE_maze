#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoomExitComponent.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew, ClassGroup = (Maze), meta = (BlueprintSpawnableComponent))
class MAZE_API URoomExitComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URoomExitComponent();

    // ----------------------
    // 出口引用
    // ----------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    USceneComponent* Socket = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    UStaticMeshComponent* WallMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    UStaticMeshComponent* HoleMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exit")
    bool bUsed = false;

    // ----------------------
    // 功能方法
    // ----------------------

    // 设置出口使用状态（true=已使用，false=未使用）
    void SetUsed(bool bNewUsed);

private:
    // 禁用 Mesh（隐藏 + 关闭碰撞 + 停止物理 + 停止 Tick）
    void DisableMesh(UStaticMeshComponent* Mesh);

    // 启用 Mesh（显示 + 恢复碰撞 + 物理 + Tick）
    void EnableMesh(UStaticMeshComponent* Mesh);
};
