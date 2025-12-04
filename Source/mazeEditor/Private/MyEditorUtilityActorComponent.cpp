

#if WITH_EDITOR
#include "MyEditorUtilityActorComponent.h"
#include "EngineUtils.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Kismet2/BlueprintEditorUtils.h"      // FBlueprintEditorUtils
#include "Kismet2/KismetEditorUtilities.h"     // FKismetEditorUtilities
#include "AssetRegistry/AssetRegistryModule.h" // FAssetRegistryModule
#include "AssetToolsModule.h"
#include "IAssetTools.h"


void UMyEditorUtilityActorComponent::CreateRoomBlueprints(TArray<UObject*> TargetRooms, const FString& SavePath)
{
    for (UObject* Obj : TargetRooms)
    {
        if (!Obj) continue;

        ProcessTargetRoom(Obj, SavePath);
    }
}

void UMyEditorUtilityActorComponent::ProcessTargetRoom(UObject* TargetRoom, const FString& SavePath)
{
    UE_LOG(LogTemp, Warning, TEXT("Processing room..."));

    if (!TargetRoom)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetRoom is null. Skipping this object."));
        return;
    }
    
    // 得到bp
    UBlueprint* BP = Cast<UBlueprint>(TargetRoom);
    if (!BP || !BP->GeneratedClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetRoom is not a Blueprint asset or has no GeneratedClass."));
        return;
    }

    UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(BP->GeneratedClass);
    if (!BPGC)
    {
        UE_LOG(LogTemp, Error, TEXT("GeneratedClass is not a BlueprintGeneratedClass."));
        return;
    }

    USimpleConstructionScript* SCS = BPGC->SimpleConstructionScript;
    if (!SCS)
    {
        UE_LOG(LogTemp, Error, TEXT("SCS error."));
        return;
    }

    // exitmap
    TMap<FString, FExitMeshData> ExitMap;
    UStaticMeshComponent* TempRoomRootComp = nullptr;

    for (USCS_Node* Node : BPGC->SimpleConstructionScript->GetAllNodes())
    {
        UActorComponent* Template = Node->GetActualComponentTemplate(BPGC);
        AddComponentToActorMap(Template, TempRoomRootComp, ExitMap);
    }

    UE_LOG(LogTemp, Warning, TEXT("Built ExitMap with %d entries"), ExitMap.Num());

    TArray<FExitMeshData> ExitValues;
    ExitMap.GenerateValueArray(ExitValues);

    if (!ValidateExitMap(ExitMap))
    {
        UE_LOG(LogTemp, Error, TEXT("ExitMap contains invalid exits! Check naming or components."));
        return;
    }

    // 使用 bp 作为新蓝图的名称
    // 获取来源 Blueprint 名字
    FString SourceName = BP->GetName();

    // 如果以 FbxScene_ 开头就去掉
    if (SourceName.StartsWith(TEXT("FbxScene_")))
    {
        SourceName = SourceName.RightChop(9); // "FbxScene_" 长度是 9
    }

    // 最终蓝图名：BP_文件名（去前缀）
    FString BPName = FString::Printf(TEXT("BP_%s"), *SourceName);


    // 创建新的 RoomActor Blueprint
    UBlueprint* NewRoomBP = CreateRoomBlueprintAsset(
        BPName,
        SavePath,
        ARoomActor::StaticClass()
    );

    if (!NewRoomBP || !NewRoomBP->GeneratedClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create RoomActor Blueprint."));
        return;
    }

    // 蓝图写入
    ARoomActor* CDO = Cast<ARoomActor>(NewRoomBP->GeneratedClass->GetDefaultObject());
    if (!CDO) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create CDO."));
        return;
    }

    // 蓝图写入 RoomRoot
    CDO->RoomRootMesh->SetStaticMesh(TempRoomRootComp->GetStaticMesh());
    CDO->RoomRootMesh->SetRelativeTransform(TempRoomRootComp->GetRelativeTransform());
    int MatCount = TempRoomRootComp->GetNumMaterials();
    for (int i = 0; i < MatCount; i++)
    {
        CDO->RoomRootMesh->SetMaterial(i, TempRoomRootComp->GetMaterial(i));
    }

    // 蓝图写入 Exits 数组
    CDO->Exits = ExitValues;

    // 标记蓝图已修改，要求保存
    NewRoomBP->Modify();
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(NewRoomBP);

    UE_LOG(LogTemp, Warning, TEXT("Created RoomActor Blueprint '%s' with %d exits."),
        *BPName,
        ExitValues.Num()
    );
}

void UMyEditorUtilityActorComponent::AddComponentToActorMap(
    UActorComponent* Component,
    UStaticMeshComponent*& RoomRootMeshComp,
    TMap<FString, FExitMeshData>& ExitMap)
{
    if (!Component) return;

    FString Name = Component->GetName();
    FString Suffix = GetComponentSuffix(Name);

    // ---------- 处理出口 ----------
    if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component))
    {
        if (Name.StartsWith(TEXT("Room"))) {
            RoomRootMeshComp = MeshComp;

            UE_LOG(LogTemp, Warning,
                TEXT("[RoomRoot] Found Room Mesh Component: %s"),
                *Name
            );
        }
        else if (Name.StartsWith(TEXT("Wall")))
        {
            FExitMeshData& Exit = ExitMap.FindOrAdd(Suffix);
            Exit.WallMesh = MeshComp->GetStaticMesh();
            Exit.WallTransform = MeshComp->GetRelativeTransform();

            int32 MatCount = MeshComp->GetNumMaterials();
            Exit.WallMaterials.SetNum(MatCount);
            for (int32 i = 0; i < MatCount; i++)
            {
                Exit.WallMaterials[i] = MeshComp->GetMaterial(i);
            }

            UE_LOG(LogTemp, Warning, TEXT("Added Wall Mesh: %s, Suffix: %s"), *Name, *Suffix);
        }
        else if (Name.StartsWith(TEXT("Door")))
        {
            FExitMeshData& Exit = ExitMap.FindOrAdd(Suffix);
            Exit.HoleMesh = MeshComp->GetStaticMesh();
            Exit.HoleTransform = MeshComp->GetRelativeTransform();

            int32 MatCount = MeshComp->GetNumMaterials();
            Exit.HoleMaterials.SetNum(MatCount);

            for (int32 i = 0; i < MatCount; i++)
            {
                Exit.HoleMaterials[i] = MeshComp->GetMaterial(i);
            }

            UE_LOG(LogTemp, Warning, TEXT("Added Door Mesh: %s, Suffix: %s"), *Name, *Suffix);
        }
    }
    else if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
    {
        if (Name.StartsWith(TEXT("DoorSnapPoint")))
        {
            FExitMeshData& Exit = ExitMap.FindOrAdd(Suffix);
            Exit.SocketTransform = SceneComp->GetRelativeTransform();

            UE_LOG(LogTemp, Warning, TEXT("Added Socket: %s, Suffix: %s"), *Name, *Suffix);
        }
    }
}

FString UMyEditorUtilityActorComponent::GetRootComponentName(UBlueprintGeneratedClass* BPGC)
{
    // 获取 SCS 的 Root 节点（通常只有 1 个）
    const TArray<USCS_Node*>& RootNodes = BPGC->SimpleConstructionScript->GetRootNodes();

    if (RootNodes.Num() == 0)
        return FString();

    // 取第一个 Root 节点
    USCS_Node* RootNode = RootNodes[0];
    if (!RootNode)
        return FString();

    UActorComponent* RootTemplate = RootNode->GetActualComponentTemplate(BPGC);
    if (!RootTemplate)
        return FString();

    return RootTemplate->GetName();
}

UBlueprint* UMyEditorUtilityActorComponent::CreateRoomBlueprintAsset(
    const FString& AssetName,
    const FString& SavePath,
    UClass* ParentClass
)
{
    FString SafeFolder = TEXT("/Game/") + SavePath;

    // 原始路径
    FString OriginalPackagePath = SafeFolder + TEXT("/") + AssetName;
    FString SanitizedPackageName = SanitizePackageName(OriginalPackagePath);

    // 检查旧资产
    UPackage* ExistingPackage = FindPackage(nullptr, *SanitizedPackageName);
    UBlueprint* ExistingBP = ExistingPackage ? FindObject<UBlueprint>(ExistingPackage, *AssetName) : nullptr;

    if (ExistingBP)
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset %s already exists, backing up..."), *AssetName);

        BackupOldAsset(ExistingBP, SavePath, AssetName);
    }

    // 创建 Package
    UPackage* Package = CreatePackage(*SanitizedPackageName);

    FString UniqueBPName = MakeUniqueAssetName(AssetName, Package);

    UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
        ParentClass,
        Package,
        *UniqueBPName,
        BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass(),
        FName("CreateRoomBlueprint")
    );

    FAssetRegistryModule::AssetCreated(NewBP);
    Package->MarkPackageDirty();

    return NewBP;
}

 FString UMyEditorUtilityActorComponent::SanitizePackageName(const FString& InPath)
{
    FString Out = InPath;
    if (!Out.StartsWith(TEXT("/"))) Out = TEXT("/") + Out;
    Out.ReplaceInline(TEXT("\\"), TEXT("/"));
    for (int32 i = 0; i < Out.Len(); ++i)
    {
        TCHAR C = Out[i];
        if (!FChar::IsAlnum(C) && C != TEXT('_') && C != TEXT('/') && C != TEXT('.'))
        {
            Out[i] = TEXT('_');
        }
    }
    while (Out.Contains(TEXT("//")))
    {
        Out.ReplaceInline(TEXT("//"), TEXT("/"));
    }
    return Out;
}

FString UMyEditorUtilityActorComponent::MakeUniqueAssetName(const FString& BaseName, UPackage* Package)
 {
     FString UniqueName = BaseName;
     int32 Index = 1;

     while (FindObject<UBlueprint>(Package, *UniqueName))
     {
         UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Index);
         Index++;
     }

     return UniqueName;
 }

FString MakeUniqueAssetName(const FString& BaseName, UPackage* Package)
{
    FString UniqueName = BaseName;
    int32 Index = 1;

    while (FindObject<UBlueprint>(Package, *UniqueName))
    {
        UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Index);
        Index++;
    }

    return UniqueName;
}

FString UMyEditorUtilityActorComponent::GetComponentSuffix(const FString& Name)
{
    FString Suffix;

    // 找前面的 '_'
    int32 Index;
    if (Name.FindChar('_', Index))
    {
        return Name.Mid(Index + 1);
    }
    return Suffix;
}
void UMyEditorUtilityActorComponent::BackupOldAsset(UBlueprint* OldBP, const FString& SavePath, const FString& AssetName)
{
    if (!OldBP) return;

    // old 文件夹路径
    FString OldFolder = FString::Printf(TEXT("/Game/%s/old"), *SavePath);

    // 确保文件夹存在
    IFileManager::Get().MakeDirectory(*FPaths::ProjectContentDir().Append(SavePath + "/old"), true);

    // 时间戳
    FString TimeStr = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    // 新资产名称
    FString NewBackupName = FString::Printf(TEXT("%s_%s"), *AssetName, *TimeStr);

    // 使用 FAssetRenameData 移动资产
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools& AssetTools = AssetToolsModule.Get();

    TArray<FAssetRenameData> RenameData;
    RenameData.Add(FAssetRenameData(OldBP, OldFolder, NewBackupName));

    AssetToolsModule.Get().RenameAssets(RenameData);

    UE_LOG(LogTemp, Warning, TEXT("Backed up old asset '%s' to '%s'"), *AssetName, *NewBackupName);
}

bool UMyEditorUtilityActorComponent::ValidateExitMap(
    const TMap<FString, FExitMeshData>& ExitMap)
{
    bool bValid = true;

    for (const auto& Pair : ExitMap)
    {
        const FString& Suffix = Pair.Key;
        const FExitMeshData& Exit = Pair.Value;

        bool bHasWall = Exit.WallMesh != nullptr;
        bool bHasDoor = Exit.HoleMesh != nullptr;

        // 如果所有字段都没值，说明这是无效 Exit
        if (!bHasWall && !bHasDoor)
        {
            UE_LOG(LogTemp, Error,
                TEXT("[Exit Validation ERROR] Exit '%s' has NO valid components (No Wall / No Door / No SnapPoint)."),
                *Suffix);

            bValid = false;
        }
    }

    return bValid;
}

#endif // WITH_EDITOR
