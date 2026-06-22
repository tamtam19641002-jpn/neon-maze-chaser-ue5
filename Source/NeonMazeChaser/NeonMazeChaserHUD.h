#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NeonMazeChaserHUD.generated.h"

class ANeonMazeChaserPlayerController;

UCLASS()
class NEONMAZECHASER_API ANeonMazeChaserHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    float ViewScale = 1.0f;
    float OffsetX = 0.0f;
    float OffsetY = 0.0f;
    static constexpr float Cell = 29.0f;
    static constexpr float MazeX = 393.5f;
    static constexpr float MazeY = 82.0f;

    FVector2D P(float X, float Y) const;
    void Rect(float X, float Y, float W, float H, const FLinearColor& Color);
    void TextCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color);
    void DrawMaze(const ANeonMazeChaserPlayerController* Controller);
    void DrawPlayer(const FVector2D& GridPosition, const FVector2D& Direction, float MouthPhase);
    void DrawEnemy(const FVector2D& GridPosition, const FLinearColor& Color, bool bFrightened);
    void DrawTitle();
};
