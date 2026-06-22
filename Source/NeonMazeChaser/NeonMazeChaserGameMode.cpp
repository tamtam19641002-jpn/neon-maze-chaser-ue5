#include "NeonMazeChaserGameMode.h"
#include "NeonMazeChaserHUD.h"
#include "NeonMazeChaserPlayerController.h"

ANeonMazeChaserGameMode::ANeonMazeChaserGameMode()
{
    HUDClass = ANeonMazeChaserHUD::StaticClass();
    PlayerControllerClass = ANeonMazeChaserPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
}
