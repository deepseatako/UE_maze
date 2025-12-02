#include "RoomExitComponent.h"

URoomExitComponent::URoomExitComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// ----------------------
// 设置出口使用状态
// ----------------------
void URoomExitComponent::SetUsed(bool bNewUsed)
{
    bUsed = bNewUsed;

    if (bUsed)
    {
        DisableMesh(WallMesh);
        EnableMesh(HoleMesh);
    }
    else
    {
        EnableMesh(WallMesh);
        DisableMesh(HoleMesh);
    }
}

// ----------------------
// 禁用 Mesh
// ----------------------
void URoomExitComponent::DisableMesh(UStaticMeshComponent* Mesh)
{
    if (!Mesh) return;

    Mesh->SetVisibility(false);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //Mesh->SetSimulatePhysics(false);
    Mesh->SetComponentTickEnabled(false);
}

// ----------------------
// 启用 Mesh
// ----------------------
void URoomExitComponent::EnableMesh(UStaticMeshComponent* Mesh)
{
    if (!Mesh) return;

    Mesh->SetVisibility(true);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    //Mesh->SetSimulatePhysics(true);
    Mesh->SetComponentTickEnabled(true);
}
