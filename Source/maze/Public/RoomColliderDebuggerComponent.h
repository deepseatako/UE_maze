// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoomColliderDebuggerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRoomOverlapDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAZE_API URoomColliderDebuggerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URoomColliderDebuggerComponent();

    // é˘óv???ì≥ìI?ò¢ñ[?/Actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
    AActor* RoomA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
    AActor* RoomB;

    // ê•î€ç›ü‡?/ñÕ?íÜ?êßïÔ?·¥
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDraw = true;

    // Overlap ??éñåè
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FRoomOverlapDelegate OnOverlapBegin;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FRoomOverlapDelegate OnOverlapEnd;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // è„àÍ?ê•î€?ì≥
    bool bIsOverlapping = false;

    // ???ì≥
    bool CheckOverlapInternal(AActor* A, AActor* B) const;

    // ?êß?åè Bounds ïÔ?·¥
    void DrawComponentBounds(UPrimitiveComponent* Comp, bool bOverlap);
};