//------------------------------
// PingPongGameInstance.cpp
// 処理
// staticのRefreeで設定されたプレイするたびスコアをリセット
//------------------------------
#include "PingPongGameInstance.h"
#include "Character/Enemy/EnemyParam.h"
#include "Tower/TowerManager.h"
#include "Ball/Ball.h"
#include "Refree/Refree.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UPingPongGameInstance::Init()
{
	Super::Init();

    UE_LOG(LogTemp, Warning, TEXT("=== GameInstance Init Called ==="));
    if (!Refree)
    {
        Refree = NewObject<URefree>(this, URefree::StaticClass());
        Refree->ResetScore();
    }

    //TowerManager（タワー専用の進行管理）
    if (!TowerManager)
    {
        TowerManager = NewObject<UTowerManager>(this, UTowerManager::StaticClass());
        TowerManager->Initialize(40); 
    }
    else
    {
        TowerManager->Initialize(40); 
    }
    TowerManager->TowerParamTable = EnemyParamTable;
}

URefree* UPingPongGameInstance::GetRefree() const
{
    return Refree;
}


FEnemyParam UPingPongGameInstance::GetVersusEnemyParam() const
{
    FEnemyParam Out = {}; // デフォルト

    if (!VersusParamTable) {
        UE_LOG(LogTemp, Error, TEXT("VersusParamTable is NOT set in GameInstance."));
        return Out;
    }

    const int32 Level = FMath::Clamp(SelectedEnemyLevel, 0, 99);
    const FName RowName(*FString::Printf(TEXT("Versus_%d"), Level));

    if (const FEnemyParam* Row = VersusParamTable->FindRow<FEnemyParam>(RowName, TEXT("GetVersusEnemyParam"), true))
    {
        Out = *Row;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Row '%s' not found in VersusParamTable."), *RowName.ToString());
    }
    return Out;
}