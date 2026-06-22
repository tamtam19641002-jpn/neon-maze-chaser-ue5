#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NeonMazeChaserPlayerController.generated.h"

class USoundWaveProcedural;

UENUM()
enum class EMazeGameState : uint8
{
    Title,
    Playing,
    Victory,
    GameOver
};

struct FMazeEnemy
{
    FVector2D Position = FVector2D::ZeroVector;
    FVector2D Direction = FVector2D::ZeroVector;
    FLinearColor Color = FLinearColor::Red;
    FVector2D Home = FVector2D::ZeroVector;
};

UCLASS()
class NEONMAZECHASER_API ANeonMazeChaserPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ANeonMazeChaserPlayerController();
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

    static constexpr int32 GridWidth = 17;
    static constexpr int32 GridHeight = 19;

    EMazeGameState GameState = EMazeGameState::Title;
    FVector2D PlayerPosition = FVector2D(8.0f, 14.0f);
    FVector2D PlayerDirection = FVector2D::ZeroVector;
    TArray<FMazeEnemy> Enemies;
    TArray<uint8> Dots;
    int32 Score = 0;
    int32 HighScore = 0;
    int32 Lives = 3;
    int32 Level = 1;
    float PowerTimer = 0.0f;
    float MouthPhase = 0.0f;

    bool IsWall(int32 X, int32 Y) const;
    bool HasDot(int32 X, int32 Y) const;
    bool IsPowerDot(int32 X, int32 Y) const;
    int32 RemainingDots() const;

private:
    FVector2D DesiredDirection = FVector2D(-1.0f, 0.0f);
    float ReadyTimer = 0.0f;
    bool bPreviousStart = false;
    bool bPreviousMouse = false;
    bool bPreviousFullscreen = false;

    UPROPERTY()
    TArray<TObjectPtr<USoundWaveProcedural>> GeneratedSounds;

    void StartGame(bool bNewGame);
    void BuildMaze();
    void ResetActors();
    void UpdatePlayer(float DeltaTime);
    void UpdateEnemies(float DeltaTime);
    void CollectDot();
    void ResolveEnemyHits();
    FVector2D ChooseEnemyDirection(const FMazeEnemy& Enemy, int32 EnemyIndex) const;
    bool CanMove(const FVector2D& Position, const FVector2D& Direction) const;
    void MoveOnGrid(FVector2D& Position, FVector2D& Direction, const FVector2D& WantedDirection, float Speed, float DeltaTime);
    void PlayTone(float Frequency, float Duration, float Volume, bool bSweep = false);
};
