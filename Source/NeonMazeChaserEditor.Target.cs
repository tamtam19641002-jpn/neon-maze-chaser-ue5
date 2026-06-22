using UnrealBuildTool;
using System.Collections.Generic;

public class NeonMazeChaserEditorTarget : TargetRules
{
    public NeonMazeChaserEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("NeonMazeChaser");
    }
}
