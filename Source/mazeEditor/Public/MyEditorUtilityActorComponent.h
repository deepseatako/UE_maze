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
    void ProcessTargetRoom(UObject* TargetRoom, const FString& SavePath);
    void AddComponentToActorMap(UActorComponent* Component, UStaticMeshComponent*& RoomRootMeshComp, UStaticMeshComponent*& RoomColliderMesh, TMap<FString, FExitMeshData>& ExitMap);
    FString GetRootComponentName(UBlueprintGeneratedClass* BPGC);
    UBlueprint* CreateRoomBlueprintAsset(const FString& AssetName, const FString& SavePath, UClass* ParentClass);
    FString SanitizePackageName(const FString& InPath);
    FString MakeUniqueAssetName(const FString& BaseName, UPackage* Package);
    FString GetComponentSuffix(const FString& Name);
    void BackupOldAsset(UBlueprint* OldBP, const FString& SavePath, const FString& AssetName);
    bool ValidateExitMap(const TMap<FString, FExitMeshData>& ExitMap);
};
