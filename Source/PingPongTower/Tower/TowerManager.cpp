//------------------------------
// TowerManager.cpp
// マネージャークラス
// 
// タワーモードを管理する
// 階数
//------------------------------


#include "TowerManager.h"
#include "TowerSaveGame.h"
#include "../PingPongGameInstance.h"
#include "../Character/Enemy/EnemyParam.h"
#include "Kismet/GameplayStatics.h"


//初期化
void UTowerManager::Initialize(int32 InMaxFloor)
{
	MaxFloor = InMaxFloor;

	// 保存データがあれば反映する
	LoadProgress();
}

//セーブデータをロード
void UTowerManager::LoadProgress()
{
	//セーブデータの名前が存在するか
	if (USaveGame* SaveData = UGameplayStatics::LoadGameFromSlot(TEXT("TowerSlot"), 0))
	{
		if (UTowerSaveGame* Data = Cast<UTowerSaveGame>(SaveData))
		{
			//保存していた階数と最高到達階を復元
			CurrentFloor = Data->SaveFloor;
			BestRecordFloor = Data->BestRecordFloor;
			return;
		}
	}

	//セーブデータがない場合
	CurrentFloor = 1;
	BestRecordFloor = 1;
}


//現在の回数を保存
void UTowerManager::SaveProgress()
{
	//新しいセーブデータを作成
	UTowerSaveGame* Data = Cast<UTowerSaveGame>(UGameplayStatics::CreateSaveGameObject(UTowerSaveGame::StaticClass()));

	//現在の進行状況を保存
	Data->SaveFloor = CurrentFloor;
	Data->BestRecordFloor = BestRecordFloor;

	//保存
	UGameplayStatics::SaveGameToSlot(Data, TEXT("TowerSlot"), 0);

}

//勝利時に次のシーンへ
void UTowerManager::OnPlayerWin(UWorld* World)
{


	//次の階へ
	CurrentFloor++;

	//最高記録の更新
	BestRecordFloor = FMath::Max(BestRecordFloor, CurrentFloor);


	//塔を制覇した場合の遷移
	if (CurrentFloor > MaxFloor)
	{
		// 制覇したので階数リセット
		CurrentFloor = 1;
		SaveProgress();
		UGameplayStatics::OpenLevel(World, TEXT("TowerClear"));

		return;
	}
	//状態をセーブ
	SaveProgress();

	//次の階がある場合
	UGameplayStatics::OpenLevel(World, TEXT("FloorClear"));
}

//敗北時に次のシーンへ
void UTowerManager::OnPlayerLose(UWorld* World)
{
	//階数をリセット
	CurrentFloor = 1;

	//状態をセーブ
	SaveProgress();

	//タイトルへ戻る
	UGameplayStatics::OpenLevel(World, TEXT("TowerSelect"));
}


//階数ごとのパラメータを返す
FEnemyParam UTowerManager::GetEnemyParam(int32 Floor) const
{
	if (!TowerParamTable)
	{
		UE_LOG(LogTemp, Error, TEXT("TowerParamTable is NOT set."));
		return FEnemyParam(); // デフォルト返す
	}

	FName RowName = FName(*FString::FromInt(Floor));
	FEnemyParam* Row = TowerParamTable->FindRow<FEnemyParam>(RowName, TEXT(""));

	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Tower Param for Floor %d. Using default."), Floor);
		return FEnemyParam();
	}

	return *Row;
}