// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EditorUtilityWidget.h"
#include "RoomActor.h"
#include "MyEditorUtilityActorComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAZEEDITOR_API UMyEditorUtilityActorComponent : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
    // Blueprint node that prints "Hello"
    UFUNCTION(BlueprintCallable, Category = "Test")
    void CreateRoomBlueprints(TArray<UObject*> TargetRooms, const FString& SavePath);

private:
    void ProcessTargetRoom(UObject* TargetRoom);
};
