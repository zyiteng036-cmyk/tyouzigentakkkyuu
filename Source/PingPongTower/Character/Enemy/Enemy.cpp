//------------------------------
//Enemy.cpp
//敵キャラクターの制御ロジック
//AIによる移動予測、返球時の物理計算、スキル管理を実装
//------------------------------

#include "Enemy.h"
#include "EnemyParam.h"
#include "../../Tower/TowerManager.h" 
#include "../../Skill/SkillManager.h"
#include "../../PingPongGameInstance.h"
#include "../../Refree/Refree.h"
#include "../CharacterManager.h"
#include "../../Ball/Ball.h"
#include "../../Court/Court.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

//コンストラクタ
AEnemy::AEnemy()
	: EnemyMesh(NULL)
	, ReturnCollision(NULL)
	, Onethird(FVector::ZeroVector)
	, MoveSpeed(400.f)
	, ReactionDelay(0.5f)
	, PrecisionThresholdClose(250.f)
	, PrecisionThresholdFar(370.f)
	, ReturnSpeed(900.f)
	, RandomAngleRange(15.f)
	, DifferenceY(0.f)
	, MoveDirectionY(0.f)
{
	//毎フレーム更新を有効化
	PrimaryActorTick.bCanEverTick = true;

	//敵のメッシュコンポーネントを作成
	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	//ルートコンポーネントに設定
	RootComponent = EnemyMesh;

	//返球判定用のコリジョンを作成
	ReturnCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ReturnCollision"));

	if (ReturnCollision != NULL)
	{
		//メッシュにアタッチ
		ReturnCollision->SetupAttachment(RootComponent);
		//判定の大きさを設定
		ReturnCollision->SetBoxExtent(FVector(200.f, 200.f, 200.f));
		//メッシュに対する相対位置を設定
		ReturnCollision->SetRelativeLocation(FVector(-100.f, 0.f, 50.f));
		//衝突プリセットをOverlapに設定
		ReturnCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		//イベント発生を有効化
		ReturnCollision->SetGenerateOverlapEvents(true);
	}

	//ボール生成位置のコンポーネントを作成
	ProjectileLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileLocation"));
	if (ProjectileLocation != NULL)
	{
		//ルートにアタッチ
		ProjectileLocation->SetupAttachment(GetRootComponent());
		//生成位置のオフセットを設定
		ProjectileLocation->SetRelativeLocation(FVector(150.f, 0.f, 60.f));
	}
}

//開始時の初期化処理
void AEnemy::BeginPlay()
{
	//親クラスの処理を呼び出し
	Super::BeginPlay();

	//ゲームインスタンスを取得
	if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance()))
	{
		FEnemyParam Param;

		//タワーモードか通常モードかで参照パラメータを分岐
		if (GI->SelectedGameMode == EGameModeType::Tower && GI->TowerManager)
		{
			//現在の階数に応じたパラメータを取得
			Param = GI->TowerManager->GetEnemyParam(GI->TowerManager->CurrentFloor);
		}
		else
		{
			//通常対戦用のパラメータを取得
			Param = GI->GetVersusEnemyParam();
		}

		//取得したパラメータを変数に反映
		MoveSpeed = Param.MoveSpeed;
		ReturnSpeed = Param.ReturnSpeed;
		ReactionDelay = Param.ReactionDelay;
		PrecisionThresholdClose = Param.PrecisionThresholdClose;
		PrecisionThresholdFar = Param.PrecisionThresholdFar;

		//スキル管理クラスを生成
		SkillManager = NewObject<USkillManager>(this);
		//初期化実行
		SkillManager->Initialize(this);
		//スキルインデックスを設定
		SkillManager->SetSelectedSkill(Param.SkillIndex);
	}

	//コリジョンの接触開始イベントに関数を登録
	if (ReturnCollision)
	{
		ReturnCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBegin);
	}

	//タイトル画面時は自動でサーブを開始
	if (UWorld* World = GetWorld()) {
		if (UPingPongGameInstance* Instance = Cast<UPingPongGameInstance>(World->GetGameInstance()))
		{
			if (Instance->SelectedGameMode == EGameModeType::Title)
			{
				//次フレームでサーブを実行
				GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {ServeBall(); });
				return;
			}
		}
	}
}

//終了時の処理
void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//親クラスの処理
	Super::EndPlay(EndPlayReason);
	//キャラマネージャーから自身を削除
	if (UCharacterManager* Manager = UCharacterManager::Get(GetWorld()))
	{
		Manager->UnregisterCharacter(this);
	}
}

//毎フレームの更新
void AEnemy::Tick(float DeltaTime)
{
	//親クラスの更新
	Super::Tick(DeltaTime);

	//移動AIの更新
	UpdateMove(DeltaTime);

	//スキルの発動条件チェックと実行
	if (SkillManager && SkillManager->CanActivate())
	{
		SkillManager->ActivateSkill();
	}
}

//線分と半直線の交点計算
bool GetLineSegmentAndRayIntersection(
	const FVector& L,
	const FVector& R,
	const FVector& P,
	const FVector& v,
	FVector& OutIntersectionPoint)
{
	//線分のベクトルを計算
	FVector lineDir = R - L;
	//半直線の向きを代入
	FVector rayDir = v;
	//始点の差分
	FVector w = P - L;

	//外積から行列式を計算
	FVector cross = FVector::CrossProduct(rayDir, lineDir);
	//大きさを取得
	float denom = cross.SizeSquared();

	//平行なら交差なし
	if (denom < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	//交差地点までの距離パラメータを算出
	FVector diff = L - P;
	float t = FVector::DotProduct(FVector::CrossProduct(diff, lineDir), cross) / denom;
	float s = FVector::DotProduct(FVector::CrossProduct(diff, rayDir), cross) / denom;

	//条件を満たせば交点座標を返す
	if (t >= 0.0f && s >= 0.0f && s <= 1.0f)
	{
		OutIntersectionPoint = P + t * rayDir;
		return true;
	}

	return false;
}

//移動AIのロジック
void AEnemy::UpdateMove(float DeltaTime)
{
	//スタン中は移動不可
	if (bSmashStunned)
	{
		return;
	}

	//アクティブなボールを取得
	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());
	ABall* Ball = GI ? GI->ActiveBall : nullptr;

	//現在の座標と目標座標の初期化
	float CurrentY = GetActorLocation().Y;
	float TargetY = CurrentY;
	float CurrentSpeed = MoveSpeed;

	//ボールがない時は定位置へ
	if (!Ball)
	{
		TargetY = -230.f;
		CurrentSpeed = MoveSpeed * 0.5f;
	}
	else
	{
		//ボールの物理情報を取得
		FVector p = Ball->GetActorLocation();
		p.Z = 0.0f;
		FVector v = Ball->GetVelocity();
		v.Z = 0.0f;

		//敵の移動可能範囲を定義
		FVector L = GetActorLocation() + FVector(0, -400.0f, 0.0f);
		L.Z = 0.0f;
		FVector R = GetActorLocation() + FVector(0, 400.0f, 0.0f);
		R.Z = 0.0f;

		//ボール軌道との交点を計算
		FVector Intersection;
		bool tsuiseki = GetLineSegmentAndRayIntersection(L, R, p, v, Intersection);

		if (tsuiseki)
		{
			//交点までの距離を測定
			double length = (Intersection - p).Length();

			//近距離：正確に移動
			if (length < PrecisionThresholdClose)
			{
				TargetY = Intersection.Y;
				CurrentSpeed = MoveSpeed;
			}
			//中距離：遅延させて反応
			else if (length < PrecisionThresholdFar)
			{
				TargetY = CurrentY + (Intersection.Y - CurrentY) * ReactionDelay;
				CurrentSpeed = MoveSpeed * 0.8f;
			}
			//遠距離：準備状態
			else
			{
				TargetY = -230.f;
				CurrentSpeed = MoveSpeed * 0.5f;
			}
		}
		else
		{
			TargetY = -230.f;
			CurrentSpeed = MoveSpeed * 0.5f;
		}
	}

	//補間により現在地を更新
	float NewY = FMath::FInterpConstantTo(CurrentY, TargetY, DeltaTime, CurrentSpeed);
	FVector NewLocation = GetActorLocation();
	NewLocation.Y = NewY;
	//座標を適用
	SetActorLocation(NewLocation);
}

//スタン処理
void AEnemy::SetSmashDelay(float DelayTime)
{
	//重複実行を防止
	if (bSmashStunned) return;

	//スタンフラグ有効化
	bSmashStunned = true;

	//指定時間後にフラグを解除するタイマー
	GetWorldTimerManager().SetTimer(
		SmashDelayHandle,
		[this]() { bSmashStunned = false; },
		DelayTime,
		false
	);
}

//サーブ実行
void AEnemy::ServeBall()
{
	//クラスチェック
	if (!BallClass) return;
	//生成場所コンポーネントの有効チェック
	if (!IsValid(ProjectileLocation) || !ProjectileLocation->IsRegistered()) return;

	//審判からサーブ権を確認
	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());
	URefree* LocalRefree = GI ? GI->GetRefree() : nullptr;
	if (LocalRefree && !LocalRefree->CanServe(EPlayerSide::Enemy)) return;

	//生成パラメータの準備
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	UWorld* World = GetWorld();
	if (!World) return;

	//ボールを生成
	FTransform SpawnTransform = ProjectileLocation->GetComponentTransform();
	ABall* NewBall = World->SpawnActor<ABall>(BallClass, SpawnTransform);
	if (!IsValid(NewBall)) return;

	//打ち出し方向を算出
	FVector Dir = -GetActorForwardVector();
	FVector LaunchDir = Dir.RotateAngleAxis(0.f, FVector::UpVector).GetSafeNormal();

	//初速設定
	NewBall->SetDirection(LaunchDir);
	NewBall->BallSpeed = 600.f;

	if (IsValid(NewBall->ProjectileMovement))
	{
		NewBall->ProjectileMovement->Bounciness = 0.85f;
		//初速度をセット
		NewBall->ProjectileMovement->Velocity = LaunchDir * NewBall->BallSpeed;
		//コンポーネントを更新
		NewBall->ProjectileMovement->UpdateComponentVelocity();
	}
	//生成したボールを保持
	SpawnedBall = NewBall;

	//審判に打球を記録
	if (LocalRefree) LocalRefree->RecordHit(EPlayerSide::Enemy);
}

//物理計算を用いた敵の返球ロジック
void AEnemy::ReturnBall(ABall* Ball)
{
	//引数のボールポインタが有効であるかを確認
	if (!Ball) return;

	//敵の前方向ベクトルを取得しプレイヤー側への打ち出し方向を算出
	FVector ForwardDir = -GetActorForwardVector();

	//内積を用いてボールがすでに自分から離れる方向に移動中か判定
	//多重ヒットによる意図しない挙動を防止するためのガード処理
	if (FVector::DotProduct(Ball->GetVelocity(), ForwardDir) > 0.0f) return;

	//ゲーム全体の情報を管理するインスタンスを取得
	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());

	//審判クラスを介したルール判定と打球記録の実行
	if (GI)
	{
		if (URefree* Ref = GI->GetRefree())
		{
			//自陣コートにバウンドする前に打ち返した場合はダイレクトボレーとしてフォルト
			if (Ref->GetLastHitter() == EPlayerSide::Player && Ball->GetCurrentTag() != FName("EnemyCourt"))
			{
				//審判に反則を報告しスコア処理へ移行
				Ref->ProcessFault(EPlayerSide::Enemy);
				return;
			}
			//正常な打球として審判に現在の打手を記録
			Ref->RecordHit(EPlayerSide::Enemy);
		}
	}

	//対戦相手であるプレイヤーの現在地を取得
	FVector PlayerLocation = FVector::ZeroVector;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			//AIがプレイヤーの位置を把握し狙い先を決定するための基準点とする
			PlayerLocation = PlayerPawn->GetActorLocation();
		}
	}

	//返球ターゲットのデフォルト座標を前方の一定距離に設定
	FVector TargetLocation = GetActorLocation() + (ForwardDir * 800.f);
	//コートの横幅制限値を初期化
	float MaxTargetY = 200.f;
	//コートの中心座標を保持
	float CenterY = 0.f;

	//コート情報を取得し物理的に正しい着弾位置を計算
	if (GI && GI->ActiveCourt && GI->ActiveCourt->PlayerCourtCollision)
	{
		//着弾目標をプレイヤー側コートの中心に設定
		TargetLocation = GI->ActiveCourt->PlayerCourtCollision->GetComponentLocation();
		//ボールが床に埋まらないよう台の表面よりわずかに高い位置を狙う
		TargetLocation.Z = GI->ActiveCourt->PlayerCourtCollision->GetComponentLocation().Z + 10.f;
		//左右のエイム計算用に中心Y座標を保持
		CenterY = TargetLocation.Y;
		//コートの端に飛びすぎないよう安全圏を考慮した最大幅を算出
		MaxTargetY = GI->ActiveCourt->GetCourtHalfWidth() - 20.f;
	}

	//AIによる戦術的な狙い先の計算
	//コート端から40パーセントの範囲を最低オフセットとして定義
	float MinTargetY = MaxTargetY * 0.4f;
	//ターゲット中心から見たプレイヤーの相対的な位置を算出
	float RelativePlayerY = PlayerLocation.Y - CenterY;
	//最終的な着弾点の左右オフセット値
	float TargetYOffset = 0.0f;
	//AIの意思決定をシミュレートする乱数
	float AINoKimaGure = FMath::FRand();

	//70パーセントの確率でプレイヤーの近辺を狙い、30パーセントで逆サイドを突く
	if (AINoKimaGure < 0.7f)
	{
		//プレイヤーの位置にランダムな微調整を加えて狙う
		TargetYOffset = RelativePlayerY + FMath::RandRange(-40.f, 40.f);
	}
	else
	{
		//プレイヤーが右にいれば左、左にいれば右のオープンコートを狙う
		if (RelativePlayerY > 0.f) TargetYOffset = FMath::RandRange(-MaxTargetY, -MinTargetY);
		else TargetYOffset = FMath::RandRange(MinTargetY, MaxTargetY);
	}

	//ターゲット座標が物理的にコート内に収まるようにクランプ
	TargetYOffset = FMath::Clamp(TargetYOffset, -MaxTargetY, MaxTargetY);
	//算出したオフセットを中心座標に加算して最終目標地点を確定
	TargetLocation.Y = CenterY + TargetYOffset;

	//放物線を描いてターゲットに必中させるための物理パラメータ逆算処理
	//現在地から目標地点までの相対ベクトルを計算
	FVector StartLoc = Ball->GetActorLocation();
	FVector Diff = TargetLocation - StartLoc;

	//水平成分のみを取り出したベクトルを作成
	FVector HorizontalDiff = Diff;
	HorizontalDiff.Z = 0.0f;
	//目標までの水平距離を算出
	float HorizontalDistance = HorizontalDiff.Size();
	//打ち出し方向の単位ベクトルを抽出
	FVector HorizontalDir = HorizontalDiff.GetSafeNormal();

	//あらかじめ設定された返球速度を適用
	float HorizontalSpeed = ReturnSpeed;
	//目標に到達するまでの滞空時間を計算
	float TimeToTarget = HorizontalDistance / HorizontalSpeed;
	//ゼロ除算によるクラッシュを防止
	if (TimeToTarget < 0.01f) TimeToTarget = 0.01f;

	//ワールド設定の重力加速度を取得
	float Gravity = GetWorld()->GetGravityZ();
	//ドライブ回転による急降下を表現するための重力倍率
	float GravityMultiplier = 1.0f;

	//ネットを越えるための理想的な放物線を維持しつつ急降下させるための処理
	float MinTimeForArc = HorizontalDistance / 850.f;
	if (TimeToTarget < MinTimeForArc)
	{
		//滞空時間が理想より短い場合、重力スケールを2乗で強化してトップスピンを再現
		GravityMultiplier = FMath::Square(MinTimeForArc / TimeToTarget);
		//計算に使用する重力値を倍率に合わせて更新
		Gravity *= GravityMultiplier;
	}

	//物理の等加速度運動公式から必要な垂直初速Vzを導き出す
	//公式：z = Vz * t + 0.5 * g * t^2 をVzについて変形
	float RequiredVz = (Diff.Z - (0.5f * Gravity * TimeToTarget * TimeToTarget)) / TimeToTarget;

	//算出した水平速度成分と垂直速度成分を合成して最終的な速度ベクトルを作成
	FVector FinalVelocity = (HorizontalDir * HorizontalSpeed);
	FinalVelocity.Z = RequiredVz;

	//物理挙動を司るProjectileMovementコンポーネントに計算結果を反映
	if (Ball->ProjectileMovement)
	{
		//現在の慣性をリセットして計算結果を正確に反映させる準備
		Ball->ProjectileMovement->StopMovementImmediately();
		//トップスピン効果を物理エンジンに反映させるため重力スケールを上書き
		Ball->ProjectileMovement->ProjectileGravityScale = GravityMultiplier;
		//算出されたベロシティを適用しターゲットへ射出
		Ball->ProjectileMovement->Velocity = FinalVelocity;
		//コンポーネント内部の状態を最新の速度で更新
		Ball->ProjectileMovement->UpdateComponentVelocity();

		//ボールクラスが持つパラメータも同期させて整合性を保つ
		Ball->BallSpeed = FinalVelocity.Size();
		Ball->Direction = FinalVelocity.GetSafeNormal();
	}

	//打球後のスキルシステム連携
	if (SkillManager)
	{
		//ボールに対してスキル効果のスタック等を通知
		SkillManager->OnHitBall(Ball);
		//返球に成功したためスキルゲージを大幅に加算
		SkillManager->AddGauge(60.f);
	}

	//Blueprint側で作成されたスイングアニメーションや音声を再生するイベントを実行
	OnReturnBallSwing();

	//視覚的フィードバックとしてNiagaraエフェクトを生成
	if (HitEffect)
	{
		//エフェクトの発生位置をラケット位置に合わせて調整
		FVector SpawnLocation = EnemyMesh->GetComponentLocation() - ForwardDir * 50.f + FVector(0.f, -50.f, 20.f);
		//打ち出し方向に合わせてエフェクトの向きを設定
		FRotator SpawnRotation = ForwardDir.Rotation();

		if (UWorld* World = GetWorld())
		{
			//ワールド上にエフェクトをスポーンさせ打撃感を演出
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, SpawnLocation, SpawnRotation, FVector(1.f));
		}
	}
}
//コリジョン開始イベント
void AEnemy::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//接触した相手がボールなら返球
	ABall* HitBall = Cast<ABall>(OtherActor);
	if (HitBall)
	{
		ReturnBall(HitBall);
	}
}