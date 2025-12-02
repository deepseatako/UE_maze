#include "RoomColliderDebuggerComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"


URoomColliderDebuggerComponent::URoomColliderDebuggerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void URoomColliderDebuggerComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!RoomA || !RoomB)
        return;

    bool bOverlap = CheckOverlapInternal(RoomA, RoomB);

    // 触?事件
    if (bOverlap != bIsOverlapping)
    {
        bIsOverlapping = bOverlap;
        if (bIsOverlapping)
        {
            OnOverlapBegin.Broadcast();
        }
        else
        {
            OnOverlapEnd.Broadcast();
        }
    }

    // ?制包?盒
    if (bDebugDraw)
    {
        TInlineComponentArray<UPrimitiveComponent*> CompsA(RoomA);
        for (UPrimitiveComponent* Comp : CompsA)
        {
            DrawComponentBounds(Comp, bOverlap);
        }

        TInlineComponentArray<UPrimitiveComponent*> CompsB(RoomB);
        for (UPrimitiveComponent* Comp : CompsB)
        {
            DrawComponentBounds(Comp, bOverlap);
        }
    }
}

// 使用 ComponentOverlapMulti 判断?个 Actor 是否重?
bool URoomColliderDebuggerComponent::CheckOverlapInternal(AActor* A, AActor* B) const
{
    if (!A || !B)
        return false;

    TInlineComponentArray<UPrimitiveComponent*> CompsA(A);
    TInlineComponentArray<UPrimitiveComponent*> CompsB(B);

    UWorld* World = GetWorld();
    if (!World) return false;

    for (UPrimitiveComponent* CompA : CompsA)
    {
        if (!CompA || !CompA->IsCollisionEnabled())
            continue;

        TArray<FOverlapResult> Overlaps;

        // 忽略自身
        FComponentQueryParams Params;
        Params.AddIgnoredActor(A);

        // ??所有?象?型
        FCollisionObjectQueryParams ObjParams = FCollisionObjectQueryParams::AllObjects;

        bool bHit = CompA->ComponentOverlapMulti(
            Overlaps,
            World,
            CompA->GetComponentLocation(),
            CompA->GetComponentQuat(),
            ECC_WorldDynamic, // 或者?使用的自定? Collision Channel
            Params,
            ObjParams
        );

        if (!bHit)
            continue;

        // ?? B 是否在重??果里
        for (const FOverlapResult& Hit : Overlaps)
        {
            if (Hit.GetActor() == B)
            {
                return true;
            }
        }
    }

    return false;
}

// ?制?件包?盒
void URoomColliderDebuggerComponent::DrawComponentBounds(UPrimitiveComponent* Comp, bool bOverlap)
{
    if (!Comp || !Comp->IsCollisionEnabled())
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FColor Color = bOverlap ? FColor::Red : FColor::Green;
    DrawDebugBox(World, Comp->Bounds.Origin, Comp->Bounds.BoxExtent, Comp->GetComponentQuat(), Color, false, -1.f, 0, 2.f);
}
