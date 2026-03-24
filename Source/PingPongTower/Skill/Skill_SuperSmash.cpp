//------------------------------
// Skill_SuperSmash.cpp
// 処理
// 
// スキル１
// スーパースマッシュ
//------------------------------


#include "Skill_SuperSmash.h"
#include "../Character/Player/Racket.h"
#include "../Ball/Ball.h"
#include "../Character/Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

// コンストラクタ
USkill_SuperSmash::USkill_SuperSmash()
    : SpeedMultiplier(3.f)
{
}


// 発動（ボタン押下時）
void USkill_SuperSmash::Activate()
{
    //if (!Owner.IsValid()) return;
	//「次の打球でスマッシュする」状態へ
	bArmed = true; 

	//デバッグ
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(
			1, 1.5f, FColor::Red,
			TEXT("ENEMY SUPER SMASH READY!")
		);

	//デバッグ
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
	//	TEXT("[SuperSmash] Activated → Next hit will smash!"));
}

void USkill_SuperSmash::OnHitBall(ABall* Ball)
{
	if (!bArmed || !Ball) {
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver,
		//	TEXT("[SuperSmash] bSmashReady = false → Normal shot"));
		return;
	}

	// デバッグ
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
		TEXT("[SuperSmash] SMASH TRIGGER!!"));

	// 速度強化
	Ball->SuperSmash(SpeedMultiplier);



	// 敵をスタン
	if (Owner.IsValid() && Owner->IsA(ARacket::StaticClass()))
	{
		// プレイヤーが使った場合のみ、敵をスタン
		if (UWorld* World = Owner->GetWorld())
		{
			for (TActorIterator<AEnemy> It(World); It; ++It)
			{
				if (AEnemy* Enemy = *It)
				{
					Enemy->SetSmashDelay(EnemyStunSec);
				}
			}
		}
	}
	bArmed = false;
}
