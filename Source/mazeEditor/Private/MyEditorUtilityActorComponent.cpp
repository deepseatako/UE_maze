#include "MyEditorUtilityActorComponent.h"
#include "EngineUtils.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"

#if WITH_EDITOR

void UMyEditorUtilityActorComponent::CreateRoomBlueprints(TArray<UObject*> TargetRooms, const FString& SavePath)
{
    for (UObject* Obj : TargetRooms)
    {
        if (!Obj) continue;

        ProcessTargetRoom(Obj);
    }
}

void UMyEditorUtilityActorComponent::ProcessTargetRoom(UObject* TargetRoom)
{
    UE_LOG(LogTemp, Warning, TEXT("Processing room..."));

    if (!TargetRoom)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetRoom is null. Skipping this object."));
        return;
    }

    // Try cast to AActor (level instance)
    AActor* Actor = Cast<AActor>(TargetRoom);

    // If not an Actor, try spawning from Blueprint asset
    if (!Actor)
    {
        UBlueprint* BP = Cast<UBlueprint>(TargetRoom);
        if (BP && BP->GeneratedClass)
        {
            Actor = GetWorld()->SpawnActor<AActor>(BP->GeneratedClass);
        }
    }

    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to convert TargetRoom into an Actor. It may not be a level actor or blueprint asset."));
        return;
    }

    // Get all components
    TArray<UActorComponent*> AllComponents;
    Actor->GetComponents(AllComponents);

    if (AllComponents.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No components found in Actor: %s"), *Actor->GetName());
        return;
    }

    for (UActorComponent* Comp : AllComponents)
    {
        if (!Comp) continue;

        UE_LOG(LogTemp, Warning, TEXT("Component: %s (%s)"), *Comp->GetName(), *Comp->GetClass()->GetName());
    }
}

#endif // WITH_EDITOR
