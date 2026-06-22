using UnrealBuildTool;

public class NeonMazeChaser : ModuleRules
{
    public NeonMazeChaser(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AudioMixer" });
    }
}
