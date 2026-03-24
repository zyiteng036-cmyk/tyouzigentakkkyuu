//------------------------------
// TowerManager.h
// マネージャークラス
// 
// タワーモードを管理する
// 階数
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TowerManager.generated.h"

struct FEnemyParam;
class UDataTable;

/**
 *
 */
UCLASS()
class PINGPONGTOWER_API UTowerManager : public UObject
{
	GENERATED_BODY()

public:

	//初期化
	void Initialize(int32 MaxFloor);

	//セーブデータをロード
	void LoadProgress();

	//現在の回数を保存
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveProgress();

	//勝利時に次のシーンへ
	void OnPlayerWin(UWorld* World);

	//敗北時に次のシーンへ
	void OnPlayerLose(UWorld* World);

	//階数ごとのパラメータを返す
	FEnemyParam GetEnemyParam(int32 Floor) const;


public:

	//現在の回数
	UPROPERTY(BlueprintReadWrite, Category = "Tower")
	int32 CurrentFloor = 1;

	//塔の最大階数
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	int32 MaxFloor = 50;

	//最高到達階
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	int32 BestRecordFloor = 1;

	// タワーモード用パラメータ DataTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyData")
	UDataTable* TowerParamTable;
};
