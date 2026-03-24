//------------------------------
// Enemy.h
// クラス
// 球を返す
// 段階を踏んで敵を変えていく
// １.すべての球を返す敵
// 
//------------------------------

//インクルードガード
#pragma once

//インクルガード
#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Enemy.generated.h"


class ABall;
class URefree;
class UBoxComponent;
class USkillManager;
class UNiagaraSystem;

UCLASS()
class PINGPONGTOWER_API AEnemy : public ACharacterBase
{
	GENERATED_BODY()

public:
	// コンストラクタ
	AEnemy();

protected:
	// 生成時、スタート時
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	//毎フレーム処理
	virtual void Tick(float DeltaTime) override;

	// スタン状態（動きを止める）
	void SetSmashDelay(float DelayTime);

protected:
	//球を追う
	void UpdateMove(float DeltaTime);


	//ボールを返す
	void ReturnBall(ABall* Ball);
	//スイング
	UFUNCTION(BlueprintImplementableEvent)
	void OnReturnBallSwing();


	// コリジョンイベント
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


public:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ServeBall();

public:
	// 現在存在しているボール（重複生成防止用）
	UPROPERTY()
	ABall* SpawnedBall;

	//Refreeクラス
	UPROPERTY(BlueprintReadWrite, Category = "Ball|Score")
	URefree* Refree;

	//スキル
	UPROPERTY()
	USkillManager* SkillManager;

	// Niagaraエフェクト
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* HitEffect;


protected:

	// サーブで使うクラス
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TSubclassOf<class ABall> BallClass;

	// サーブ発射位置
	UPROPERTY(VisibleAnywhere, Category = "Serve")
	USceneComponent* ProjectileLocation;


	//スタティックメッシュ
	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	UStaticMeshComponent* EnemyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	UBoxComponent* ReturnCollision;

	// 返球関連
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Return")
	float ReturnSpeed;

	// ±角度範囲
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Return")
	float RandomAngleRange; 

	//ステージの3分の1超えると返球
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Return")
	FVector Onethird;


	// 移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racket")
	float MoveSpeed;

	// 敵の難易度レベル
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 EnemyLevel;

	// 反応遅延
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
	float ReactionDelay;

	// 判定精度（ボールとの距離でどの程度で反応するか）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
	float PrecisionThresholdClose;   // 正確に反応する距離

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI")
	float PrecisionThresholdFar;     // やや甘い反応距離

private:



	//Y方向の差
	float DifferenceY;

	//Y方向
	float MoveDirectionY;

	bool bSmashStunned = false;
	FTimerHandle SmashDelayHandle;
};
