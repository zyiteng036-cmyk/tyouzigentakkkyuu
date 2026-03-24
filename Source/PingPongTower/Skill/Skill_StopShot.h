//------------------------------
// Skill_StopShot.h
// クラス
// 
// スキル2
// ストップショット
// ネットの上で停止しランダムの角度で再スタート
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "SkillBase.h"
#include "Skill_StopShot.generated.h"


class ABall;
class ARacket;
/**
 *
 */
UCLASS()
class PINGPONGTOWER_API USkill_StopShot : public USkillBase
{
	GENERATED_BODY()

public:

	// コンストラクタ
	USkill_StopShot();

	// 発動（ボタン押下時）
	virtual void Activate() override;

	// 打球時に呼ばれる（ReturnBallから）
	virtual void OnHitBall(ABall* Ball) override;

	// ネット通過時に呼ぶ
	void OnPassedNet(ABall* Ball);

	virtual FString GetDisplayName_Implementation() const override { return TEXT("Stop Shot"); }


private:
	//ネットを超えるまで待つ
	bool bWaitingForNetPass = false;

	// どのボールに適用するか
	TWeakObjectPtr<ABall> TargetBall;

	// 停止秒数
	UPROPERTY(EditDefaultsOnly, Category = "StopShot")
	float StopTime = 1.8f;

	// 打った時の方向を保存する
	UPROPERTY()
	FVector StoredDirection;

	// 再発射スピード
	UPROPERTY(EditDefaultsOnly, Category = "StopShot")
	float LaunchSpeed = 1000.f;
	// 再発射の水平角度範囲（±）
	UPROPERTY(EditDefaultsOnly, Category = "StopShot")
	float RandomYawRangeDeg = 30.f;

	// 敵を一定時間スタン
	UPROPERTY(EditDefaultsOnly, Category = "SuperSmash")
	float EnemyStunSec = 1.2f;


	float OriginalGravity = 1.f;
};
