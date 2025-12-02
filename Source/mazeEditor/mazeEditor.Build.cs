using UnrealBuildTool;

     public class mazeEditor: ModuleRules
     {
          public mazeEditor(ReadOnlyTargetRules Target) : base(Target)
          {
               PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "Slate", "SlateCore", "UMG"});
               PublicDependencyModuleNames.AddRange(new string[] {"maze"});
          }
     }