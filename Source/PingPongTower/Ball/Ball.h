//------------------------------
// Ball.h
// クラス
// 球の制御
// 球のスピード
// 動き
// 反射(衝突処理)
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Ball.generated.h"

class URefree;

// インクルードの下あたりに定義
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallBounced, FName, CourtTag);

UCLASS()
class PINGPONGTOWER_API ABall : public AActor
{
	GENERATED_BODY()

public:
	//コンストラクタ
	ABall();

protected:

	//スタート時、または生成時の処理
	virtual void BeginPlay() override;

	//Destory時
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	//毎フレーム処理
	virtual void Tick(float DeltaTime) override;

public:
	//打ち返し方向をセットする
	void SetDirection(const FVector& NewDirection);

	//スピードを設定
	void AddSpeed(float SpeedUp);

	//現在のタグを設定する（どちらのコートバウンドしたか）
	FName GetCurrentTag()const { return CurrentTag; };
	
	//タグをセットする
	FName GetPrevTag() { return PrevTag; };

	//バウンド回数
	int GetBoundCount()const { return BoundCount; };

	//バウンドしたか判断
	bool IsBound()const { return bIsBound; };

	//最後に触れたコートのタグ
	FName GetLastTag() const { return PrevTag; }

	//バウンドをリセット
	void ResetBound();


	// 反射処理
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	//スキル（スーパースマッシュ）
	void SuperSmash(float SpeedUp);
	void StartSmashSequence();
	// 超次元スマッシュ演出
	void ExecuteMidAirSpinSmash();
	bool IsSmashArmed() const { return bIsSmashArmed; }

public:
	//球のスピード
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float BallSpeed;

	//ProjectileMovementの設定
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovement;

	// 球の進行方向
	FVector Direction;

	// 球の見た目（StaticMesh）
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;

	// バウンドした時に発火するイベント
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBallBounced OnBallBounced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* HitSound;



	//ボールのスポーンを限定
	UPROPERTY()
	bool bBallSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<class UCameraShakeBase> SmashCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<class UCameraShakeBase> BounceCameraShake;

private:

	int32 BoundCount;
	FName CurrentTag;
	FName PrevTag;
	bool bIsBound;


	// 超次元スマッシュ用の変数
	bool bIsSmashArmed = false;
	bool bIsSmashActive = false;
	float StoredSmashSpeed = 1.0f;

	FTimerHandle SmashRiseTimer;   // 上に跳び上がる用
	FTimerHandle SmashSpinTimer;   // スピン用
	FTimerHandle SmashLaunchTimer; // 発射用

	void UpdateSmashSpin();
	void LaunchSmashLaser();
};
