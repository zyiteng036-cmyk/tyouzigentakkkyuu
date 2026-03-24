//------------------------------
// Skill_SuperSmash.h
// クラス
// 
// スキル１
// スーパースマッシュ
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "SkillBase.h"
#include "Skill_SuperSmash.generated.h"

//前方宣言
class ABall;
class ARacket;
/**
 * 
 */
UCLASS()
class PINGPONGTOWER_API USkill_SuperSmash : public USkillBase
{
	GENERATED_BODY()

public:
	// コンストラクタ
	USkill_SuperSmash();
	
	// 発動（ボタン押下時）
	virtual void Activate() override;

	// 打球時に呼ばれる（ReturnBallから）
	virtual void OnHitBall(ABall* Ball) override;

	// UI 表示名
	virtual FString GetDisplayName_Implementation() const override { return TEXT("Super Smash"); }


private:
	//スピード倍率
	UPROPERTY(EditAnywhere)
	float SpeedMultiplier;

	// 次の一打のみ有効
	bool bArmed = false;

	// 敵を一定時間スタン
	UPROPERTY(EditDefaultsOnly, Category = "SuperSmash")
	float EnemyStunSec = 2.f;
};
