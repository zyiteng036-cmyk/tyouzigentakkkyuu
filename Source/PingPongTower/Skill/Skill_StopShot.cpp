//------------------------------
// Skill_StopShot.h
// クラス
// 
// スキル2
// ストップショット
// ネットの上で停止しランダムの角度で再スタート
//------------------------------


#include "Skill_StopShot.h"
#include "../Character/Player/Racket.h"
#include "../Ball/Ball.h"
#include "../Character/Enemy/Enemy.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "Engine/World.h"

// コンストラクタ
USkill_StopShot::USkill_StopShot()
{
}

// 発動（ボタン押下時）
void USkill_StopShot::Activate()
{
	if (!Owner.IsValid()) return;
    bWaitingForNetPass = true;
    TargetBall = nullptr;
    StoredDirection = FVector::ZeroVector;
}

// 打球時に呼ばれる（ReturnBallから）
void USkill_StopShot::OnHitBall(ABall* Ball)
{
    if (!bWaitingForNetPass || !Ball) return;

    TargetBall = Ball;

    //打った瞬間の方向を保存する
    if (Ball->ProjectileMovement)
    {
        StoredDirection = Ball->ProjectileMovement->Velocity.GetSafeNormal();
    }
}

void USkill_StopShot::OnPassedNet(ABall* Ball)
{
    // スキル待機中 & 対象のボールかどうかを確認
    if (!bWaitingForNetPass || !TargetBall.IsValid() || Ball != TargetBall.Get()) return;
    bWaitingForNetPass = false;

    //完全停止　重力なし
    if (Ball->ProjectileMovement)
    {
        //重力保存
        OriginalGravity = Ball->ProjectileMovement->ProjectileGravityScale;
        //完全停止
        Ball->ProjectileMovement->StopMovementImmediately();

        //停止中は落ちなくする
        Ball->ProjectileMovement->ProjectileGravityScale = 0.f;
    }

    //ここでしっかり浮かせて止める
    FVector StopLocation = Ball->GetActorLocation();
    StopLocation.Z = 140.f;
    Ball->SetActorLocation(StopLocation, false, nullptr, ETeleportType::TeleportPhysics);

    // 打った瞬間の方向がゼロなら前方向に補正
    if (StoredDirection.IsNearlyZero())
        StoredDirection = FVector(1, 0, 0);

    //停止後の再発射
    FTimerHandle TimerHandle;
    TWeakObjectPtr<ABall> WeakBall = Ball;

    //タイマー管理クラスを取得
    Ball->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,      //タイマーハンドル
        [WeakBall, this]()//ラムダキャプチャ
        {
            if (!WeakBall.IsValid()) return;
         
            //今ここで操作している“ボールの実体”をボール取得
            ABall* BallInstance = WeakBall.Get();
            if (!BallInstance || !BallInstance->ProjectileMovement) return;

            // ランダムな左右角度
            float AngleDeg = FMath::RandRange(-RandomYawRangeDeg, RandomYawRangeDeg);

            //左右のランダム角を付けた発射方向
            FVector LaunchDir = StoredDirection.RotateAngleAxis(AngleDeg, FVector::UpVector).GetSafeNormal();

            // 通常ショットと同じ角度処理
            float SpeedFactor = FMath::Clamp(BallInstance->BallSpeed / 900.f, 1.f, 2.5f);
            float LaunchAngleDeg = 30.f - (SpeedFactor - 1.f) * 5.f;

            //「右方向」の軸を計算
            FVector RightAxis = FVector::CrossProduct(LaunchDir, FVector::UpVector).GetSafeNormal();
            //上方向へ角度
            LaunchDir = LaunchDir.RotateAngleAxis(-LaunchAngleDeg, RightAxis).GetSafeNormal();

            // 重力戻す + 再発射
            BallInstance->ProjectileMovement->ProjectileGravityScale = OriginalGravity;
            BallInstance->ProjectileMovement->Velocity = LaunchDir * LaunchSpeed;

            BallInstance->Direction = LaunchDir;
            BallInstance->BallSpeed = LaunchSpeed;

            // 敵をスタン
            if (Owner.IsValid() && Owner->IsA(ARacket::StaticClass()))
            {
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
        },
        StopTime,
        false
    );
}
