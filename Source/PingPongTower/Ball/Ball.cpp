//------------------------------
// Ball.cpp
// 球の制御
// 球のスピード
// 動き
// 反射(衝突処理)
//------------------------------

//インクルード
#include "Ball.h"
#include "../Refree/Refree.h"
#include "../Character/Enemy/Enemy.h"
#include "../Character/Player/Racket.h"
#include "../Court/Court.h"
#include "Components/BoxComponent.h"
#include "../PingPongGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

//コンストラクタ(初期化)
ABall::ABall()
	:BallSpeed(900.f)
	, ProjectileMovement(NULL)
	, BallMesh(NULL)
	, bBallSpawn(true)
{
	//毎フレーム、このクラスのTick()を呼ぶかどうかを決める
	PrimaryActorTick.bCanEverTick = true;

	//メッシュ設定
	//「UStaticMeshComponent」は3Dモデルを表示する部品
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	if (BallMesh != NULL)
	{
		//RootComponentに上記を設定し、ボール全体の位置や動きを管理する中心に
		RootComponent = BallMesh;
		//コリジョンプリセット
		BallMesh->BodyInstance.SetCollisionProfileName(TEXT("BlockAllDynamic"));
		//衝突Onhitイベント
		BallMesh->SetNotifyRigidBodyCollision(true);
		//UE5の物理エンジンで重力・衝突などを自動的に処理
		BallMesh->SetSimulatePhysics(false);
		//ボールが何かにぶつかったらOnHit()を呼ぶ。
		BallMesh->OnComponentHit.AddDynamic(this, &ABall::OnHit);
	}

	//ProjectileMovementの設定
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	if (ProjectileMovement != NULL)
	{
		//バウンドを可能にする
		ProjectileMovement->bShouldBounce = true;
		//反発係数
		ProjectileMovement->Bounciness = 0.85f;
		//摩擦
		ProjectileMovement->Friction = 0.25f;
		//重力
		ProjectileMovement->ProjectileGravityScale = 1.f;
		//
		ProjectileMovement->BounceVelocityStopSimulatingThreshold = 10.f;
		//最大スピード
		ProjectileMovement->MaxSpeed = 1200.f;
		//初速
		ProjectileMovement->InitialSpeed = BallSpeed;
	}
}

//スタート時、または生成時の処理
void ABall::BeginPlay()
{
	Super::BeginPlay();

	if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance()))
	{
		GI->ActiveBall = this;

		//審判に自分のイベントを登録
		if (URefree* Ref = GI->GetRefree())
		{
			OnBallBounced.AddDynamic(Ref, &URefree::OnBallBouncedCallback);
		}
	}

	//最初にボールをどの方向へ飛ばすかを決める処理
	Direction = FVector(1.f, 0.f, 0.f).GetSafeNormal();
	bBallSpawn = false;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * BallSpeed;
	}
}

void ABall::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// プレイヤーラケットに通知してポインタを消す
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		ARacket* Racket = Cast<ARacket>(PC->GetPawn());
		if (Racket)
		{
			Racket->SpawnedBall = nullptr;
		}
	}
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//打ち返し方向をセットする
void ABall::SetDirection(const FVector& NewDirection)
{
	if (ProjectileMovement)
	{
		// 現在の速度によって角度を調整
		float SpeedFactor = FMath::Clamp(BallSpeed / 900.f, 1.f, 2.5f);
		float LaunchAngleDeg = -10.f - (SpeedFactor - 1.f) * 5.f; // 速いほど下げる

		//回転軸計算
		//打ち返し方向ベクトル × 上方向ベクトル
		FVector RightAxis = FVector::CrossProduct(NewDirection, FVector::UpVector).GetSafeNormal();
		//新しい方向ベクトルを指定した角度分傾ける
		FVector LaunchDir = NewDirection.RotateAngleAxis(-LaunchAngleDeg, RightAxis).GetSafeNormal();

		//内部
		//進行方向
		Direction = LaunchDir;
		//打ち返し
		ProjectileMovement->Velocity = LaunchDir * BallSpeed;
	}
}

//スピードを設定
void ABall::AddSpeed(float SpeedUp)
{
}

//バウンドをリセット
//ネット通過後。点数が入った後
void ABall::ResetBound()
{
	bIsBound = false;
	BoundCount = 0;
	PrevTag = NAME_None;
	CurrentTag = NAME_None;
}

//衝突判定
void ABall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor) return;

	//無重力レーザーが台に当たったら、重力を通常(1.0)に戻す
	if (ProjectileMovement && ProjectileMovement->ProjectileGravityScale == 0.0f)
	{
		ProjectileMovement->ProjectileGravityScale = 1.0f;
	}

	//台クラスに当たったかどうか
	if (ACourt* HitCourt = Cast<ACourt>(OtherActor))
	{
		PrevTag = CurrentTag;

		//当たった部品(コンポーネント)に頼らず、当たった「位置」で判定する絶対確実な処理
		FVector HitLocation = Hit.Location;
		FVector PlayerBoxLoc = HitCourt->PlayerCourtCollision->GetComponentLocation();
		FVector EnemyBoxLoc = HitCourt->EnemyCourtCollision->GetComponentLocation();

		// 当たった場所が、プレイヤー陣地の中心と敵陣地の中心、どちらに近いか？
		if (FVector::DistSquared(HitLocation, PlayerBoxLoc) < FVector::DistSquared(HitLocation, EnemyBoxLoc))
		{
			CurrentTag = FName("PlayerCourt");
		}
		else
		{
			CurrentTag = FName("EnemyCourt");
		}

		bIsBound = true;
		BoundCount++;

		// ログ出力（直ったか確認用）
		UE_LOG(LogTemp, Warning, TEXT("[Fixed] Hit Court! Tag: %s | BoundCount: %d"), *CurrentTag.ToString(), BoundCount);
	}
	//地面に当たる
	else if (OtherActor->ActorHasTag("Ground"))
	{
		if (CurrentTag != FName("Ground"))
		{
			PrevTag = CurrentTag;
		}
		CurrentTag = FName("Ground");
		bIsBound = true;
	}

	//イベントを発火して審判に伝える
	if (OnBallBounced.IsBound())
	{
		OnBallBounced.Broadcast(CurrentTag);
	}

	//バウンド音
	if (HitSound && GetWorld())
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	//スマッシュ状態での強烈なバウンド時のみカメラを揺らす
	if (bIsBound && bIsSmashActive)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (BounceCameraShake)
			{
				PC->ClientStartCameraShake(BounceCameraShake);
			}
		}

		//揺らした後はフラグをOFFにする
		bIsSmashActive = false;
	}
}

//スキル（スーパースマッシュ）
void ABall::SuperSmash(float SpeedUp)
{
	bIsSmashArmed = true;
	StoredSmashSpeed = SpeedUp;
}

//ネットエリアに触れた瞬間（NetArea.cpp から呼ばれる）
void ABall::StartSmashSequence()
{
	if (!bIsSmashArmed || !ProjectileMovement) return;

	//多重判定を防ぐためフラグを折る
	bIsSmashArmed = false;

	//打たれた時の進行方向を記憶しておく（逆走防止用）
	Direction = ProjectileMovement->Velocity.GetSafeNormal();

	FVector Fwd = Direction;
	Fwd.Z = 0.f;
	Fwd = Fwd.GetSafeNormal();

	//画面外に行かないよう上昇する力を調整
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->Velocity = (Fwd * 200.f) + FVector(0.f, 0.f, 350.f);

	//指定時間後に空中停止＆スピンを開始
	GetWorldTimerManager().SetTimer(SmashRiseTimer, this, &ABall::ExecuteMidAirSpinSmash, 0.25f, false);
}

//頂点に達した時の処理（空中停止＆チャージ演出）
void ABall::ExecuteMidAirSpinSmash()
{
	if (!ProjectileMovement) return;

	//空中でピタッと止める
	ProjectileMovement->StopMovementImmediately();

	//カメラシェイク
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (SmashCameraShake) PC->ClientStartCameraShake(SmashCameraShake);
	}

	//スピン開始
	GetWorldTimerManager().SetTimer(SmashSpinTimer, this, &ABall::UpdateSmashSpin, 0.016f, true);

	//スピン待機時間後にレーザー発射
	GetWorldTimerManager().SetTimer(SmashLaunchTimer, this, &ABall::LaunchSmashLaser, 1.0f, false);
}

//スピンアニメーション（毎フレーム）
void ABall::UpdateSmashSpin()
{
	if (BallMesh)
	{
		//縦方向に回転させる
		BallMesh->AddLocalRotation(FRotator(40.f, 0.f, 0.f));
	}
}

//敵コート中央へ必中するレーザーとして発射
void ABall::LaunchSmashLaser()
{
	GetWorldTimerManager().ClearTimer(SmashSpinTimer);

	if (!ProjectileMovement) return;

	ProjectileMovement->MaxSpeed = 50000.f;

	FVector StartLoc = GetActorLocation();

	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());
	if (GI && GI->ActiveCourt && GI->ActiveCourt->EnemyCourtCollision && GI->ActiveCourt->PlayerCourtCollision)
	{
		FVector PlayerBoxLoc = GI->ActiveCourt->PlayerCourtCollision->GetComponentLocation();
		FVector EnemyBoxLoc = GI->ActiveCourt->EnemyCourtCollision->GetComponentLocation();

		//記憶しておいた進行方向で向かっているコートを判定する
		FVector TargetBoxLoc = (Direction.X > 0) ? EnemyBoxLoc : PlayerBoxLoc;

		//ターゲットコートのど真ん中を狙う
		FVector TargetLoc = FVector(TargetBoxLoc.X, TargetBoxLoc.Y, TargetBoxLoc.Z + 10.f);

		//上空からターゲットへの一直線軌道
		FVector LaserDir = (TargetLoc - StartLoc).GetSafeNormal();

		//スピードを適用
		float NewSpeed = FMath::Max(BallSpeed * StoredSmashSpeed, 3500.f);
		ProjectileMovement->Velocity = LaserDir * NewSpeed;
		BallSpeed = NewSpeed;

		//発射と同時にスマッシュ状態をONにする
		bIsSmashActive = true;
	}

	ProjectileMovement->UpdateComponentVelocity();
}