//------------------------------
// TowerSaveGame.h
// クラス
// 
// タワーモードの時のセーブ機能
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TowerSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PINGPONGTOWER_API UTowerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Tower")
	int32 SaveFloor = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Tower")
	int32 BestRecordFloor = 1;
	
};
