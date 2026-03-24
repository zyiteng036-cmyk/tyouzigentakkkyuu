//------------------------------
// PingPongGameInstance.h
// クラス
// staticのRefreeで設定されたプレイするたびスコアをリセット
// シーンを跨いでも使う情報保持（モード・スキル・レベル）
//------------------------------
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PingPongGameInstance.generated.h"

class URefree;
class ABall;
class UDataTable;
class UTowerManager;
class ACourt;
struct FEnemyParam;

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	Title,
	Versus,
	Tower,
};

UCLASS()
class PINGPONGTOWER_API UPingPongGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Score")
	URefree* GetRefree() const;


	// C++から設定する関数（UIから呼び出しも可）
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetSelectedEnemyLevel(int32 NewLevel)
	{
		SelectedEnemyLevel = FMath::Clamp(NewLevel, 0, 2);
	}

	// 取得関数（Enemyなどが使用）
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetSelectedEnemyLevel() const
	{
		return SelectedEnemyLevel;
	}

	// 対戦モード用パラメータ取得
	FEnemyParam GetVersusEnemyParam() const;


	//タワーマネージャー
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	UTowerManager* TowerManager;

	// 現在アクティブなボールを保持
	UPROPERTY(BlueprintReadWrite, Category = "Ball")
	ABall* ActiveBall;

	UPROPERTY(BlueprintReadWrite, Category = "Court")
	ACourt* ActiveCourt;

	//審判クラス
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Score")
	URefree* Refree;

	//プレイヤーの最終スコア
	UPROPERTY(BlueprintReadWrite, Category = "Score")
	int32 FinalPlayerScore;

	//敵の最終スコア
	UPROPERTY(BlueprintReadWrite, Category = "Score")
	int32 FinalEnemyScore;

	//現在のモード
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game")
	EGameModeType SelectedGameMode = EGameModeType::Title;

	//敵のパラメータ
	UPROPERTY(EditDefaultsOnly, Category = "EnemyParam")
	UDataTable* EnemyParamTable;

	// 対戦モード用 パラメータテーブル
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyData")
	UDataTable* VersusParamTable;

	// 敵のレベル（0=Easy, 1=Normal, 2=Hard）
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game")
	int32 SelectedEnemyLevel = 0;

	//スキル
	UPROPERTY(BlueprintReadWrite)
	int32 SelectedSkillIndex = 0;


};
