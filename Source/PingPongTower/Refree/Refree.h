//------------------------------
// Refree.h
// クラス
// 審判
// 得点
// 試合の進行を管理し、ラリーや得点の発生をイベントで通知する
// スキルゲージなど他クラスはこれを受けてゲージを貯める
// 
//------------------------------

//インクルードガード
#pragma once

//前方宣言
class ABall;
class USoundBase;


#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Refree.generated.h"

UENUM(BlueprintType)
enum class EPlayerSide : uint8
{
	None,
	Player,
	Enemy
};

UCLASS(BlueprintType, Blueprintable)
class PINGPONGTOWER_API URefree : public UObject
{
	GENERATED_BODY()

public:
	//コンストラクタ
	URefree();



	//スコアのリセット
	void ResetScore();

	//ゲームセット
	void GameSet();

	//点数処理
	void SetScore();

	//打球を記録する
	void RecordHit(EPlayerSide Hitter);

	//現在のサーバー
	bool CanServe(EPlayerSide Side)const;

	//サーバーの交代
	void ChangeServer();

	//次のサーバーの予約
	void RequestNextServe();

	//反則時のスコア処理
	void ProcessFault(EPlayerSide FaultPlayer);


	//プレイヤーのスコアを得る
	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetPlayerScore() const { return PlayerScore; }

	//敵のスコアを得る
	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetEnemyScore() const { return EnemyScore; }

	//現在のサーバーを得る
	EPlayerSide GetCurrentServer() const { return CurrentServer; }

	//最後に打った
	EPlayerSide GetLastHitter() const { return LastHitter; }

	//スキルゲージを加算
	void AddSkillGauge();

	UFUNCTION()
	void OnBallBouncedCallback(FName CourtTag);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refree")
	TObjectPtr<class ABall> Ball;

	UPROPERTY()
	TObjectPtr<UWorld> CachedWorld;


	//現在のサーバー
	UPROPERTY(BlueprintReadOnly)
	EPlayerSide CurrentServer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ScoreSound;


private:

	//ゲームセット最大スコア
	int32 MaxScore;

	//プレイヤースコア
	int32 PlayerScore;
	//エネミースコア
	int32 EnemyScore;

	// 最後に打った側
	EPlayerSide LastHitter;

	
	
	// 何回サーブしたか
	int ServeCount;

	//サーブを変えるか
	bool bServerChanged;

	//スコアが入ったかどうか
	bool bIsScore;
};
