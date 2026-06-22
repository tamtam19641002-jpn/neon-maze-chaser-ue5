#include "NeonMazeChaserHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "NeonMazeChaserPlayerController.h"

FVector2D ANeonMazeChaserHUD::P(float X, float Y) const
{
    return FVector2D(OffsetX + X * ViewScale, OffsetY + Y * ViewScale);
}

void ANeonMazeChaserHUD::Rect(float X, float Y, float W, float H, const FLinearColor& Color)
{
    const FVector2D Position = P(X, Y);
    DrawRect(Color, Position.X, Position.Y, W * ViewScale, H * ViewScale);
}

void ANeonMazeChaserHUD::TextCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color)
{
    UFont* Font = GEngine ? GEngine->GetLargeFont() : nullptr;
    if (!Font) return;
    float Width = 0.0f, Height = 0.0f;
    GetTextSize(Text, Width, Height, Font, Scale * ViewScale);
    DrawText(Text, Color, OffsetX + (1280.0f * ViewScale - Width) * 0.5f, OffsetY + Y * ViewScale, Font, Scale * ViewScale, false);
}

void ANeonMazeChaserHUD::DrawPlayer(const FVector2D& GridPosition, const FVector2D& Direction, float MouthPhase)
{
    const float X = MazeX + GridPosition.X * Cell;
    const float Y = MazeY + GridPosition.Y * Cell;
    const FLinearColor Yellow(1.0f, 0.92f, 0.12f);
    const int32 Rows[] = { 10, 18, 23, 26, 27, 27, 26, 23, 18, 10 };
    for (int32 Row = 0; Row < 10; ++Row)
    {
        const float Width = static_cast<float>(Rows[Row]);
        Rect(X + (Cell - Width) * 0.5f, Y + 3.0f + Row * 2.3f, Width, 2.7f, Yellow);
    }

    if (FMath::Sin(MouthPhase) > -0.15f)
    {
        const FLinearColor Black(0, 0, 0, 1);
        if (FMath::Abs(Direction.X) >= FMath::Abs(Direction.Y))
        {
            const bool bLeft = Direction.X < 0.0f;
            const float MouthX = bLeft ? X : X + Cell * 0.62f;
            Rect(MouthX, Y + Cell * 0.39f, Cell * 0.38f, Cell * 0.22f, Black);
        }
        else
        {
            const bool bUp = Direction.Y < 0.0f;
            Rect(X + Cell * 0.39f, bUp ? Y : Y + Cell * 0.62f, Cell * 0.22f, Cell * 0.38f, Black);
        }
    }
}

void ANeonMazeChaserHUD::DrawEnemy(const FVector2D& GridPosition, const FLinearColor& Color, bool bFrightened)
{
    const float X = MazeX + GridPosition.X * Cell;
    const float Y = MazeY + GridPosition.Y * Cell;
    const FLinearColor Body = bFrightened ? FLinearColor(0.12f, 0.25f, 1.0f) : Color;
    Rect(X + 3, Y + 5, 23, 18, Body);
    Rect(X + 6, Y + 3, 17, 3, Body);
    Rect(X + 9, Y + 1, 11, 3, Body);
    Rect(X + 3, Y + 21, 6, 5, Body);
    Rect(X + 11, Y + 21, 7, 4, Body);
    Rect(X + 21, Y + 21, 5, 5, Body);
    const FLinearColor Eye = FLinearColor::White;
    Rect(X + 7, Y + 9, 5, 6, Eye);
    Rect(X + 18, Y + 9, 5, 6, Eye);
    Rect(X + 8, Y + 12, 2, 3, bFrightened ? Eye : FLinearColor(0.1f, 0.2f, 0.8f));
    Rect(X + 19, Y + 12, 2, 3, bFrightened ? Eye : FLinearColor(0.1f, 0.2f, 0.8f));
}

void ANeonMazeChaserHUD::DrawMaze(const ANeonMazeChaserPlayerController* Controller)
{
    const FLinearColor WallOuter(0.05f, 0.2f, 0.65f);
    const FLinearColor WallInner(0.1f, 0.55f, 1.0f);
    for (int32 Y = 0; Y < ANeonMazeChaserPlayerController::GridHeight; ++Y)
    {
        for (int32 X = 0; X < ANeonMazeChaserPlayerController::GridWidth; ++X)
        {
            const float PX = MazeX + X * Cell;
            const float PY = MazeY + Y * Cell;
            if (Controller->IsWall(X, Y))
            {
                Rect(PX + 1, PY + 1, Cell - 2, Cell - 2, WallOuter);
                Rect(PX + 4, PY + 4, Cell - 8, Cell - 8, FLinearColor::Black);
                Rect(PX + 7, PY + 7, Cell - 14, Cell - 14, WallInner);
            }
            else if (Controller->HasDot(X, Y))
            {
                const bool bPower = Controller->IsPowerDot(X, Y);
                const float Size = bPower ? 9.0f : 3.5f;
                const FLinearColor DotColor = bPower ? FLinearColor(1.0f, 0.75f, 0.95f) : FLinearColor(1.0f, 0.72f, 0.48f);
                Rect(PX + (Cell - Size) * 0.5f, PY + (Cell - Size) * 0.5f, Size, Size, DotColor);
            }
        }
    }
}

void ANeonMazeChaserHUD::DrawTitle()
{
    for (int32 Index = 0; Index < 16; ++Index)
    {
        const float X = 120.0f + Index * 70.0f;
        const float Y = 110.0f + FMath::Sin(Index * 0.8f) * 35.0f;
        Rect(X, Y, 42, 6, FLinearColor(0.05f, 0.18f, 0.45f));
        Rect(X, Y + 34, 42, 6, FLinearColor(0.05f, 0.18f, 0.45f));
    }
    TextCentered(TEXT("NEON"), 185.0f, 3.5f, FLinearColor(0.1f, 0.95f, 1.0f));
    TextCentered(TEXT("MAZE CHASER"), 290.0f, 3.0f, FLinearColor(1.0f, 0.25f, 0.75f));
    TextCentered(TEXT("AN ORIGINAL RETRO MAZE GAME"), 410.0f, 1.0f, FLinearColor(0.85f, 0.85f, 1.0f));

    Rect(486.0f, 470.0f, 308.0f, 78.0f, FLinearColor(0.04f, 0.25f, 0.38f));
    Rect(491.0f, 475.0f, 298.0f, 68.0f, FLinearColor(0.02f, 0.02f, 0.08f));
    TextCentered(TEXT("START"), 486.0f, 1.8f, FLinearColor(1.0f, 0.92f, 0.15f));
    TextCentered(TEXT("CLICK OR PRESS ENTER"), 570.0f, 0.9f, FLinearColor(0.72f, 0.8f, 1.0f));
    TextCentered(TEXT("ARROW KEYS / WASD : MOVE     F11 : DISPLAY MODE"), 640.0f, 0.72f, FLinearColor(0.45f, 0.65f, 0.85f));
}

void ANeonMazeChaserHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    ViewScale = FMath::Min(Canvas->SizeX / 1280.0f, Canvas->SizeY / 720.0f);
    OffsetX = (Canvas->SizeX - 1280.0f * ViewScale) * 0.5f;
    OffsetY = (Canvas->SizeY - 720.0f * ViewScale) * 0.5f;
    Rect(0, 0, 1280, 720, FLinearColor::Black);

    const ANeonMazeChaserPlayerController* Controller = Cast<ANeonMazeChaserPlayerController>(GetOwningPlayerController());
    if (!Controller) return;
    if (Controller->GameState == EMazeGameState::Title)
    {
        DrawTitle();
        return;
    }

    TextCentered(FString::Printf(TEXT("SCORE %06d        HI %06d        LEVEL %02d"), Controller->Score, Controller->HighScore, Controller->Level), 30.0f, 0.9f, FLinearColor(0.7f, 0.95f, 1.0f));
    DrawMaze(Controller);
    DrawPlayer(Controller->PlayerPosition, Controller->PlayerDirection, Controller->MouthPhase);
    for (const FMazeEnemy& Enemy : Controller->Enemies)
    {
        DrawEnemy(Enemy.Position, Enemy.Color, Controller->PowerTimer > 0.0f);
    }

    for (int32 Life = 0; Life < Controller->Lives; ++Life)
    {
        Rect(405.0f + Life * 28.0f, 655.0f, 18.0f, 18.0f, FLinearColor(1.0f, 0.9f, 0.12f));
    }
    TextCentered(TEXT("ARROWS / WASD: MOVE     ESC: QUIT     F11: DISPLAY MODE"), 683.0f, 0.65f, FLinearColor(0.4f, 0.65f, 0.8f));

    if (Controller->GameState == EMazeGameState::Victory)
    {
        Rect(360, 280, 560, 150, FLinearColor(0.0f, 0.0f, 0.0f, 0.92f));
        TextCentered(TEXT("MAZE CLEARED!"), 305.0f, 2.0f, FLinearColor(0.2f, 1.0f, 0.65f));
        TextCentered(TEXT("PRESS ENTER FOR NEXT LEVEL"), 380.0f, 0.9f, FLinearColor::White);
    }
    else if (Controller->GameState == EMazeGameState::GameOver)
    {
        Rect(360, 280, 560, 170, FLinearColor(0.0f, 0.0f, 0.0f, 0.94f));
        TextCentered(TEXT("GAME OVER"), 305.0f, 2.2f, FLinearColor(1.0f, 0.2f, 0.3f));
        TextCentered(TEXT("PRESS ENTER OR CLICK START"), 390.0f, 0.9f, FLinearColor::White);
    }
}
