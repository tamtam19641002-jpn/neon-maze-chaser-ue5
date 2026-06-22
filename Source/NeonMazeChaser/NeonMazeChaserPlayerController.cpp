#include "NeonMazeChaserPlayerController.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

ANeonMazeChaserPlayerController::ANeonMazeChaserPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bShowMouseCursor = true;
}

void ANeonMazeChaserPlayerController::BeginPlay()
{
    Super::BeginPlay();
    SetInputMode(FInputModeGameAndUI());
    BuildMaze();
    ResetActors();
}

bool ANeonMazeChaserPlayerController::IsWall(int32 X, int32 Y) const
{
    if (Y < 0 || Y >= GridHeight)
    {
        return true;
    }
    if (X < 0 || X >= GridWidth)
    {
        return Y != 9;
    }
    if (Y == 0 || Y == GridHeight - 1)
    {
        return true;
    }
    if ((X == 0 || X == GridWidth - 1) && Y != 9)
    {
        return true;
    }

    const bool bVerticalOuter = (X == 4 || X == 12) && Y > 2 && Y < 16 && Y != 6 && Y != 9 && Y != 13;
    const bool bVerticalInner = (X == 7 || X == 9) && Y > 4 && Y < 14 && Y != 7 && Y != 9 && Y != 12;
    const bool bHorizontalTop = (Y == 4 || Y == 7) && X > 2 && X < 14 && X != 5 && X != 8 && X != 11;
    const bool bHorizontalBottom = (Y == 12 || Y == 15) && X > 2 && X < 14 && X != 5 && X != 8 && X != 11;
    const bool bTopBlocks = (Y == 2) && ((X >= 2 && X <= 4) || (X >= 6 && X <= 10) || (X >= 12 && X <= 14));
    const bool bBottomBlocks = (Y == 16) && ((X >= 2 && X <= 4) || (X >= 6 && X <= 10) || (X >= 12 && X <= 14));
    const bool bPen = (Y == 8 || Y == 10) && X >= 7 && X <= 9;

    return bVerticalOuter || bVerticalInner || bHorizontalTop || bHorizontalBottom || bTopBlocks || bBottomBlocks || bPen;
}

bool ANeonMazeChaserPlayerController::HasDot(int32 X, int32 Y) const
{
    const int32 Index = Y * GridWidth + X;
    return Dots.IsValidIndex(Index) && Dots[Index] != 0;
}

bool ANeonMazeChaserPlayerController::IsPowerDot(int32 X, int32 Y) const
{
    return (X == 1 || X == GridWidth - 2) && (Y == 1 || Y == GridHeight - 2);
}

int32 ANeonMazeChaserPlayerController::RemainingDots() const
{
    int32 Count = 0;
    for (const uint8 Dot : Dots)
    {
        Count += Dot != 0 ? 1 : 0;
    }
    return Count;
}

void ANeonMazeChaserPlayerController::BuildMaze()
{
    Dots.Init(0, GridWidth * GridHeight);
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            if (!IsWall(X, Y))
            {
                Dots[Y * GridWidth + X] = 1;
            }
        }
    }

    for (int32 Y = 7; Y <= 11; ++Y)
    {
        for (int32 X = 6; X <= 10; ++X)
        {
            Dots[Y * GridWidth + X] = 0;
        }
    }
    Dots[14 * GridWidth + 8] = 0;
}

void ANeonMazeChaserPlayerController::ResetActors()
{
    PlayerPosition = FVector2D(8.0f, 14.0f);
    PlayerDirection = FVector2D::ZeroVector;
    DesiredDirection = FVector2D(-1.0f, 0.0f);
    PowerTimer = 0.0f;
    ReadyTimer = 1.2f;

    Enemies.Reset();
    const FLinearColor Colors[] = {
        FLinearColor(1.0f, 0.18f, 0.28f),
        FLinearColor(1.0f, 0.35f, 0.75f),
        FLinearColor(0.15f, 0.9f, 1.0f),
        FLinearColor(1.0f, 0.62f, 0.12f)
    };
    const FVector2D Homes[] = {
        FVector2D(8.0f, 7.0f), FVector2D(7.0f, 9.0f),
        FVector2D(8.0f, 9.0f), FVector2D(9.0f, 9.0f)
    };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FMazeEnemy Enemy;
        Enemy.Position = Homes[Index];
        Enemy.Home = Homes[Index];
        Enemy.Direction = Index == 0 ? FVector2D(-1.0f, 0.0f) : FVector2D(0.0f, -1.0f);
        Enemy.Color = Colors[Index];
        Enemies.Add(Enemy);
    }
}

void ANeonMazeChaserPlayerController::StartGame(bool bNewGame)
{
    if (bNewGame)
    {
        Score = 0;
        Lives = 3;
        Level = 1;
        BuildMaze();
    }
    ResetActors();
    GameState = EMazeGameState::Playing;
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    PlayTone(440.0f, 0.12f, 0.2f);
}

bool ANeonMazeChaserPlayerController::CanMove(const FVector2D& Position, const FVector2D& Direction) const
{
    const int32 X = FMath::RoundToInt(Position.X) + FMath::RoundToInt(Direction.X);
    const int32 Y = FMath::RoundToInt(Position.Y) + FMath::RoundToInt(Direction.Y);
    return !IsWall(X, Y);
}

void ANeonMazeChaserPlayerController::MoveOnGrid(FVector2D& Position, FVector2D& Direction, const FVector2D& WantedDirection, float Speed, float DeltaTime)
{
    const FVector2D Center(FMath::RoundToFloat(Position.X), FMath::RoundToFloat(Position.Y));
    const FVector2D ToCenter = Center - Position;
    const float Step = Speed * DeltaTime;
    const bool bAtCenter = ToCenter.SizeSquared() < 0.0001f;
    const bool bApproachingCenter = !Direction.IsNearlyZero() && FVector2D::DotProduct(ToCenter, Direction) >= 0.0f;
    if ((bAtCenter || bApproachingCenter) && ToCenter.Size() <= Step + 0.01f)
    {
        Position = Center;
        if (!WantedDirection.IsNearlyZero() && CanMove(Position, WantedDirection))
        {
            Direction = WantedDirection;
        }
        if (!CanMove(Position, Direction))
        {
            Direction = FVector2D::ZeroVector;
        }
    }

    Position += Direction * Step;
    if (Position.X < -0.6f)
    {
        Position.X = GridWidth - 0.4f;
    }
    else if (Position.X > GridWidth - 0.4f)
    {
        Position.X = -0.6f;
    }
}

void ANeonMazeChaserPlayerController::UpdatePlayer(float DeltaTime)
{
    if (IsInputKeyDown(EKeys::Left) || IsInputKeyDown(EKeys::A)) DesiredDirection = FVector2D(-1.0f, 0.0f);
    if (IsInputKeyDown(EKeys::Right) || IsInputKeyDown(EKeys::D)) DesiredDirection = FVector2D(1.0f, 0.0f);
    if (IsInputKeyDown(EKeys::Up) || IsInputKeyDown(EKeys::W)) DesiredDirection = FVector2D(0.0f, -1.0f);
    if (IsInputKeyDown(EKeys::Down) || IsInputKeyDown(EKeys::S)) DesiredDirection = FVector2D(0.0f, 1.0f);

    MoveOnGrid(PlayerPosition, PlayerDirection, DesiredDirection, 6.0f + Level * 0.12f, DeltaTime);
    MouthPhase += DeltaTime * 12.0f;
    CollectDot();
}

FVector2D ANeonMazeChaserPlayerController::ChooseEnemyDirection(const FMazeEnemy& Enemy, int32 EnemyIndex) const
{
    const FVector2D Directions[] = { FVector2D(-1, 0), FVector2D(1, 0), FVector2D(0, -1), FVector2D(0, 1) };
    FVector2D Target = PlayerPosition;
    if (EnemyIndex == 1) Target += PlayerDirection * 4.0f;
    if (EnemyIndex == 2) Target = FVector2D(PlayerPosition.X, GridHeight - 2.0f);
    if (EnemyIndex == 3 && FVector2D::Distance(Enemy.Position, PlayerPosition) < 6.0f) Target = FVector2D(1.0f, GridHeight - 2.0f);
    if (PowerTimer > 0.0f) Target = FVector2D(GridWidth - 1.0f - PlayerPosition.X, GridHeight - 1.0f - PlayerPosition.Y);

    FVector2D Best = FVector2D::ZeroVector;
    float BestDistance = TNumericLimits<float>::Max();
    for (const FVector2D& Candidate : Directions)
    {
        if (Candidate.Equals(-Enemy.Direction) && CanMove(Enemy.Position, Enemy.Direction)) continue;
        if (!CanMove(Enemy.Position, Candidate)) continue;
        const float Distance = FVector2D::DistSquared(Enemy.Position + Candidate, Target) + FMath::FRandRange(0.0f, 2.0f);
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            Best = Candidate;
        }
    }
    return Best;
}

void ANeonMazeChaserPlayerController::UpdateEnemies(float DeltaTime)
{
    for (int32 Index = 0; Index < Enemies.Num(); ++Index)
    {
        FMazeEnemy& Enemy = Enemies[Index];
        const FVector2D Center(FMath::RoundToFloat(Enemy.Position.X), FMath::RoundToFloat(Enemy.Position.Y));
        FVector2D Wanted = Enemy.Direction;
        if (FVector2D::Distance(Enemy.Position, Center) < 0.1f)
        {
            Wanted = ChooseEnemyDirection(Enemy, Index);
        }
        const float Speed = PowerTimer > 0.0f ? 3.4f : 4.0f + Level * 0.18f;
        MoveOnGrid(Enemy.Position, Enemy.Direction, Wanted, Speed, DeltaTime);
    }
}

void ANeonMazeChaserPlayerController::CollectDot()
{
    const int32 X = FMath::RoundToInt(PlayerPosition.X);
    const int32 Y = FMath::RoundToInt(PlayerPosition.Y);
    const int32 Index = Y * GridWidth + X;
    if (!Dots.IsValidIndex(Index) || Dots[Index] == 0 || FVector2D::Distance(PlayerPosition, FVector2D(X, Y)) > 0.35f)
    {
        return;
    }

    Dots[Index] = 0;
    if (IsPowerDot(X, Y))
    {
        Score += 50;
        PowerTimer = 7.0f;
        PlayTone(260.0f, 0.18f, 0.2f, true);
    }
    else
    {
        Score += 10;
        PlayTone(700.0f + (Score % 20) * 18.0f, 0.025f, 0.055f);
    }
    HighScore = FMath::Max(HighScore, Score);

    if (RemainingDots() == 0)
    {
        GameState = EMazeGameState::Victory;
        bShowMouseCursor = true;
        SetInputMode(FInputModeGameAndUI());
        PlayTone(520.0f, 0.45f, 0.25f, true);
    }
}

void ANeonMazeChaserPlayerController::ResolveEnemyHits()
{
    for (FMazeEnemy& Enemy : Enemies)
    {
        if (FVector2D::Distance(PlayerPosition, Enemy.Position) >= 0.7f) continue;
        if (PowerTimer > 0.0f)
        {
            Score += 200;
            HighScore = FMath::Max(HighScore, Score);
            Enemy.Position = Enemy.Home;
            Enemy.Direction = FVector2D(0.0f, -1.0f);
            PlayTone(920.0f, 0.12f, 0.18f, true);
        }
        else
        {
            --Lives;
            PlayTone(380.0f, 0.55f, 0.25f, true);
            if (Lives <= 0)
            {
                GameState = EMazeGameState::GameOver;
                bShowMouseCursor = true;
                SetInputMode(FInputModeGameAndUI());
            }
            else
            {
                ResetActors();
            }
        }
        break;
    }
}

void ANeonMazeChaserPlayerController::PlayTone(float Frequency, float Duration, float Volume, bool bSweep)
{
    constexpr int32 SampleRate = 22050;
    const int32 SampleCount = FMath::Max(1, FMath::RoundToInt(Duration * SampleRate));
    TArray<int16> Samples;
    Samples.SetNumUninitialized(SampleCount);
    double Phase = 0.0;
    for (int32 Index = 0; Index < SampleCount; ++Index)
    {
        const float T = static_cast<float>(Index) / SampleCount;
        const float CurrentFrequency = bSweep ? Frequency * (1.0f - T * 0.55f) : Frequency;
        Phase += 2.0 * PI * CurrentFrequency / SampleRate;
        const float Envelope = FMath::Clamp(1.0f - T, 0.0f, 1.0f);
        const float Wave = FMath::Sin(static_cast<float>(Phase)) >= 0.0f ? 1.0f : -1.0f;
        Samples[Index] = static_cast<int16>(Wave * Envelope * Volume * 32767.0f);
    }

    USoundWaveProcedural* Sound = NewObject<USoundWaveProcedural>(this);
    Sound->SetSampleRate(SampleRate);
    Sound->NumChannels = 1;
    Sound->Duration = Duration;
    Sound->SoundGroup = SOUNDGROUP_Effects;
    Sound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
    GeneratedSounds.Add(Sound);
    if (GeneratedSounds.Num() > 24) GeneratedSounds.RemoveAt(0, 8);
    UGameplayStatics::PlaySound2D(this, Sound);
}

void ANeonMazeChaserPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    if (IsInputKeyDown(EKeys::Escape))
    {
        ConsoleCommand(TEXT("quit"));
        return;
    }

    const bool bFullscreen = IsInputKeyDown(EKeys::F11);
    if (bFullscreen && !bPreviousFullscreen && GEngine && GEngine->GetGameUserSettings())
    {
        UGameUserSettings* Settings = GEngine->GetGameUserSettings();
        Settings->SetFullscreenMode(Settings->GetFullscreenMode() == EWindowMode::Windowed ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
        Settings->ApplySettings(false);
    }
    bPreviousFullscreen = bFullscreen;

    const bool bStart = IsInputKeyDown(EKeys::Enter) || IsInputKeyDown(EKeys::SpaceBar);
    const bool bMouse = IsInputKeyDown(EKeys::LeftMouseButton);
    bool bClickedStart = false;
    if (bMouse && !bPreviousMouse)
    {
        float MouseX = 0.0f, MouseY = 0.0f;
        int32 ViewX = 0, ViewY = 0;
        GetMousePosition(MouseX, MouseY);
        GetViewportSize(ViewX, ViewY);
        bClickedStart = MouseX > ViewX * 0.38f && MouseX < ViewX * 0.62f && MouseY > ViewY * 0.62f && MouseY < ViewY * 0.75f;
    }
    bPreviousMouse = bMouse;

    if ((GameState == EMazeGameState::Title || GameState == EMazeGameState::GameOver) && ((bStart && !bPreviousStart) || bClickedStart))
    {
        StartGame(true);
    }
    else if (GameState == EMazeGameState::Victory && ((bStart && !bPreviousStart) || bClickedStart))
    {
        ++Level;
        BuildMaze();
        StartGame(false);
    }
    bPreviousStart = bStart;

    if (GameState != EMazeGameState::Playing) return;
    if (ReadyTimer > 0.0f)
    {
        ReadyTimer -= DeltaTime;
        return;
    }

    PowerTimer = FMath::Max(0.0f, PowerTimer - DeltaTime);
    UpdatePlayer(DeltaTime);
    UpdateEnemies(DeltaTime);
    ResolveEnemyHits();
}
