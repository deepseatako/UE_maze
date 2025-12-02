using UnrealBuildTool;

     public class mazeEditor: ModuleRules
     {
          public mazeEditor(ReadOnlyTargetRules Target) : base(Target)
          {
               PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "Slate", "SlateCore"});
               PrivateDependencyModuleNames.AddRange(new string[] { "UMG" });
          }
     }