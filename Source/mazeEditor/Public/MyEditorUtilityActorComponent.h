// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MyEditorUtilityActorComponent.generated.h"

/**
 * 
 */
UCLASS()
class MAZEEDITOR_API UMyEditorUtilityActorComponent : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
    // Blueprint node that prints "Hello"
    UFUNCTION(BlueprintCallable, Category = "Test")
    void PrintHello();
};
