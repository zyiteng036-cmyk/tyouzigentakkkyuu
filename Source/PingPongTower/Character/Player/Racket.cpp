//------------------------------
// Racket.cpp
// キャラクターの移動関連
// 見た目
// 入力バインド
// 角度をつけた返球
// 
//------------------------------


#include "Racket.h"
#include "../../Refree/Refree.h"
#include "../CharacterManager.h"
#include "../../Court/Court.h"
#include "../../Ball/Ball.h"
#include "../../Skill/SkillManager.h"
#include "../../PingPongGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// コンストラクタ
ARacket::ARacket()
	: m_pCameraRoot(NULL)
	, m_pCamera(NULL)
	, HitEffect(NULL)
	, SideCollision(NULL)
	, RacketMesh(NULL)
	, MoveSpeed(3000.f)
	, Ball(NULL)
	, CharaMoveInput(FVector2D::ZeroVector)
	, CurrentYaw(0.f)
	, m_bCanControl(true)
	, AimX(0.f)
	, AimY(0.f)
	, ReturnSpeed(900.f)
	, bCanSwing(true)
	, SwingCooldownTimer(0.f)

{
	//毎フレーム、このクラスのTick()を呼ぶかどうかを決める
	PrimaryActorTick.bCanEverTick = true;

	// デフォルトプレイヤーとして設定
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	////スプリングアームのオブジェクト生成
	//m_pSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("m_pSpringArm"));

	//if (m_pSpringArm != NULL)
	//{

	//	m_pSpringArm->SetupAttachment(GetCapsuleComponent());
	//	//カメラをキャラの位置に固定
	//	m_pSpringArm->TargetArmLength = 0.0f;
	//	//カメラが壁で押し戻されない
	//	m_pSpringArm->bDoCollisionTest = false;
	//	//回転ゼロ
	//	m_pSpringArm->SetRelativeRotation(FRotator::ZeroRotator);
	//}

	//位置と回転（描画なし）
	m_pCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("m_pCameraRoot"));
	if (m_pCameraRoot != NULL)
	{
		m_pCameraRoot->SetupAttachment(GetCapsuleComponent());

	}


	//カメラのオブジェクトを生成
	m_pCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("m_pCamera"));
	if (m_pCamera != NULL)
	{

		m_pCamera->SetupAttachment(m_pCameraRoot);
		//位置
		m_pCamera->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

		m_pCamera->SetRelativeRotation(FRotator::ZeroRotator);
		m_pCamera->bAutoActivate = true;
	}

	//ラケットメッシュ
	RacketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RacketMesh"));
	if (RacketMesh != NULL)
	{
		RacketMesh->SetupAttachment(GetRootComponent());

		// ラケットの位置（カメラの前に少し出す）
		RacketMesh->SetRelativeLocation(FVector(230.f, 108.f, -43.f));
		RacketMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
		////衝突イベントを呼ばれるように
		//RacketMesh->SetNotifyRigidBodyCollision(true);
		////UE5の物理エンジンで重力・衝突などを自動的に処理
		//RacketMesh->SetSimulatePhysics(false);
	}

	//プレイヤー横のコリジョン
	SideCollision = CreateDefaultSubobject< UBoxComponent>(TEXT("SideCollision"));
	if (SideCollision != NULL)
	{
		SideCollision->SetupAttachment(GetRootComponent());
		//大きさ
		SideCollision->SetBoxExtent(FVector(200.f, 200.f, 200.f));
		//位置
		SideCollision->SetRelativeLocation(FVector(100.f, 100.f, 0.f));
		//衝突プロファイル
		SideCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		//オーバーラップイベントを動かすか
		SideCollision->SetGenerateOverlapEvents(true);
	}

	//ボールのスポーン位置
	ProjectileLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileLocation"));
	if (ProjectileLocation != NULL) {
		ProjectileLocation->SetupAttachment(GetRootComponent());
		ProjectileLocation->SetRelativeLocation(FVector(150.f, 0.f, 60.f));

	}

	//歩行調整
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)

	{
		// 最大歩行速度
		MoveComp->MaxWalkSpeed = 450.f;
		//停止時の減速力
		MoveComp->BrakingDecelerationWalking = 8000.f;
		//摩擦
		MoveComp->GroundFriction = 10.0f;
		//自動で回転するか
		MoveComp->bUseControllerDesiredRotation = false;
		// 移動方向に回転しない
		MoveComp->bOrientRotationToMovement = false;
	}

	//CharacterMovementComponent の設定
	//重力
	GetCharacterMovement()->GravityScale = 1.0f;
	//最大速度
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	//左右の操作
	bUseControllerRotationYaw = false;
	//上下の操作
	bUseControllerRotationPitch = false;
	//Rollの操作（固定）
	bUseControllerRotationRoll = false;

	// Niagaraコンポーネントを生成
	AuraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AuraEffect"));
	AuraEffect->SetupAttachment(RootComponent);
	AuraEffect->bAutoActivate = false; // 最初は非表示
}

//スタート時、または生成時の処理
void ARacket::BeginPlay()
{
	Super::BeginPlay();

	// サイドコリジョンのオーバーラップイベント登録
	if (SideCollision)
	{
		SideCollision->OnComponentBeginOverlap.AddDynamic(this, &ARacket::OnOverlapBegin);
		SideCollision->OnComponentEndOverlap.AddDynamic(this, &ARacket::OnOverlapEnd);
	}

	// スキル生成
	SkillManager = NewObject<USkillManager>(this);
	if (IsValid(SkillManager))
	{
		SkillManager->Initialize(this);
		// GameInstance で選ばれたスキルを反映
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance()))
		{
			SkillManager->SetSelectedSkill(GI->SelectedSkillIndex);
		}
	}

	// Niagaraアセット設定
	if (AuraNiagara)
	{
		AuraEffect->SetAsset(AuraNiagara);
	}
}

void ARacket::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UCharacterManager* Manager = UCharacterManager::Get(GetWorld()))
	{
		Manager->UnregisterCharacter(this);
	}

	if (IsValid(AuraEffect) && !AuraEffect->IsBeingDestroyed())
	{
		AuraEffect->Deactivate();
	}
}


//毎フレーム処理
void ARacket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//入力の移動更新
	UpdateMove(DeltaTime);

	// クールダウンタイマー処理
	if (!bCanSwing)
	{
		SwingCooldownTimer -= DeltaTime;
		if (SwingCooldownTimer <= 0.f)
		{
			bCanSwing = true;
		}
	}

	// スティック倒し量を計算
	const float InputStrength = FMath::Sqrt(AimX * AimX + AimY * AimY);

	// 一定以上倒していたら打球
	if (bCanSwing && InputStrength >= SwingThreshold && Ball && IsValid(Ball))
	{
		ReturnBall(); // 打球実行
		bCanSwing = false;
		SwingCooldownTimer = 0.4f; // 連打防止
	}
}

//入力バインド
void ARacket::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//移動
	PlayerInputComponent->BindAxis("MoveRight", this, &ARacket::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ARacket::MoveForward);

	//角度（左右の角度を求めて打つときに反映）
	PlayerInputComponent->BindAxis("LookRight", this, &ARacket::LookRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ARacket::LookUp);

	//サーブ
	PlayerInputComponent->BindAction("Serve", IE_Pressed, this, &ARacket::ServeBall);

	//スキル
	PlayerInputComponent->BindAction("ActivateSkill", IE_Pressed, this, &ARacket::ActivateSuperSmash);

}

//移動処理
void ARacket::UpdateMove(float _deltaTime)
{

	//コントロール可能な場合のみ
	if (m_bCanControl == false) { return; }

	//解説。Unreal標準の AddMovementInput() は自動で移動ベクトルを処理する。
	AddMovementInput(GetActorForwardVector(), CharaMoveInput.Y);
	AddMovementInput(GetActorRightVector(), CharaMoveInput.X);
}
//【入力バインド】左右
void ARacket::MoveRight(float Value)
{
	//コントロール可能な場合のみ
	if (m_bCanControl == false) { return; }
	//入力値（-1～１）
	CharaMoveInput.X = FMath::Clamp(Value, -1.0f, 1.0f);

}

//【入力バインド】前後
void ARacket::MoveForward(float Value)
{
	//コントロール可能な場合のみ
	if (m_bCanControl == false) { return; }
	//入力値（-1～１）
	CharaMoveInput.Y = FMath::Clamp(Value, -1.0f, 1.0f);
}

//【入力バインド】左右の角度
void ARacket::LookRight(float value)
{
	AimX = value; // 左右方向
}

//【入力バインド】前後方向
void ARacket::LookUp(float Value)
{
	AimY = Value;
}


//【入力バインド】サーブ
void ARacket::ServeBall()
{
	if (!BallClass || !ProjectileLocation) return;

	// GameInstance から Refree を取得
	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());
	URefree* Refree = GI ? GI->Refree : nullptr;

	if (Refree && !Refree->CanServe(EPlayerSide::Player))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("You are not the server right now!"));
		return;
	}

	// すでにボールが存在していれば新規生成しない
	if (SpawnedBall && IsValid(SpawnedBall))
	{
		UE_LOG(LogTemp, Warning, TEXT("A ball already exists. Cannot spawn another."));
		return;
	}

	// BlueprintでBallClassが設定されていなければ中断
	if (!BallClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BallClass is not set! (BlueprintでBall_BPを指定してください)"));
		return;
	}

	// ProjectileLocation が存在しない場合も中断
	if (!ProjectileLocation)
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileLocation is NULL!"));
		return;
	}

	// スポーン位置
	FTransform SpawnTransform = ProjectileLocation->GetComponentTransform();

	// ワールド取得
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is NULL!"));
		return;
	}

	// スポーン実行
	ABall* NewBall = World->SpawnActor<ABall>(BallClass, SpawnTransform);


	if (!IsValid(NewBall))
	{
		UE_LOG(LogTemp, Error, TEXT("Ball spawn failed!"));
		return;
	}

	// ---------- ここからサーブ設定 ----------
	FVector ServeDir = GetActorForwardVector();
	FVector RightAxis = FVector::CrossProduct(ServeDir, FVector::UpVector).GetSafeNormal();
	FVector LaunchDir = ServeDir.RotateAngleAxis(-12.f, RightAxis).GetSafeNormal();  // 下向き12°

	if (!IsValid(NewBall))
		return;

	// ボールの初期方向とスピード設定
	NewBall->SetDirection(LaunchDir);
	NewBall->BallSpeed = 700.f;

	if (NewBall->ProjectileMovement)
	{
		NewBall->ProjectileMovement->Bounciness = 0.85f;
		// バウンドの跳ね返りと重力を調整
		NewBall->ProjectileMovement->Velocity = LaunchDir * NewBall->BallSpeed;
	}
	// 生成したボールを保存
	SpawnedBall = NewBall;

	if (Refree)
	{
		Refree->RecordHit(EPlayerSide::Player);
	}

	UE_LOG(LogTemp, Log, TEXT("Ball spawned successfully at %s"), *SpawnTransform.GetLocation().ToString());
}

//コリジョン範囲内であればボールを返す
void ARacket::ReturnBall()
{
	if (!IsValid(Ball))
	{
		Ball = nullptr;
		return;
	}

	UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance());

	if (GI)
	{
		if (URefree* Ref = GI->GetRefree())
		{
			//最後に打ったのが敵で、かつボールがまだ自分のコート(PlayerCourt)にバウンドしていない場合
			if (Ref->GetLastHitter() == EPlayerSide::Enemy && Ball->GetCurrentTag() != FName("PlayerCourt"))
			{
				Ref->ProcessFault(EPlayerSide::Player);
				return;
			}
		}
	}

	//-------------------------------------------------------------------
	// スティック入力から「敵コートのどこを狙うか（ターゲット座標）」を決める
	//-------------------------------------------------------------------
	FVector Forward = GetActorForwardVector();
	FVector TargetLocation = GetActorLocation() + (Forward * 800.f); // 保険の初期値

	if (GI && GI->ActiveCourt && GI->ActiveCourt->EnemyCourtCollision)
	{
		FVector CourtCenter = GI->ActiveCourt->EnemyCourtCollision->GetComponentLocation();
		FVector Extent = GI->ActiveCourt->EnemyCourtCollision->GetScaledBoxExtent();

		// AimX(左右)とAimY(前後)の入力を、コートの広さに掛け合わせる（端から20は安全マージン）
		float TargetY = CourtCenter.Y + (AimX * (Extent.Y - 20.f));
		float TargetX = CourtCenter.X + (AimY * (Extent.X - 20.f));

		// 赤い丸が床に埋まらないよう高さを少し上げる
		TargetLocation = FVector(TargetX, TargetY, CourtCenter.Z + 10.f);
	}

	// デバッグ：プレイヤーが今スティックで「どこを狙っているか」を可視化
	DrawDebugSphere(GetWorld(), TargetLocation, 20.f, 12, FColor::Blue, false, 2.0f);

	//-------------------------------------------------------------------
	//ターゲットに突き刺さる「放物線＋ドライブ回転」の物理計算
	//-------------------------------------------------------------------
	FVector StartLoc = Ball->GetActorLocation();
	FVector Diff = TargetLocation - StartLoc;

	FVector HorizontalDiff = Diff;
	HorizontalDiff.Z = 0.0f;
	float HorizontalDistance = HorizontalDiff.Size();
	FVector HorizontalDir = HorizontalDiff.GetSafeNormal();

	//プレイヤーの打球スピード（相手の球威を利用する）
	float HitSpeed = FMath::Max(1000.f, Ball->BallSpeed);
	float TimeToTarget = HorizontalDistance / HitSpeed;

	if (TimeToTarget < 0.01f) TimeToTarget = 0.01f;

	//デフォルトの重力を取得
	float Gravity = GetWorld()->GetGravityZ();
	float GravityMultiplier = 1.0f;

	//ドライブ回転（重力の倍率）の最適化
	float MinTimeForArc = HorizontalDistance / 810.f;
	if (TimeToTarget < MinTimeForArc)
	{
		GravityMultiplier = MinTimeForArc / TimeToTarget;
		Gravity *= GravityMultiplier;
	}

	//必要な上向きの力（Z軸の初速）を逆算
	float RequiredVz = (Diff.Z - (0.5f * Gravity * TimeToTarget * TimeToTarget)) / TimeToTarget;

	FVector FinalVelocity = (HorizontalDir * HitSpeed);
	FinalVelocity.Z = RequiredVz;

	//-------------------------------------------------------------------
	//ボールに計算結果（速度と重力）を適用する
	//-------------------------------------------------------------------
	if (Ball->ProjectileMovement)
	{
		Ball->ProjectileMovement->StopMovementImmediately();

		//ボールの最高速度リミッターを解除
		Ball->ProjectileMovement->MaxSpeed = 3000.f;

		//重力を上書きして、計算通りのドライブ回転をかける
		Ball->ProjectileMovement->ProjectileGravityScale = GravityMultiplier;

		Ball->ProjectileMovement->Velocity = FinalVelocity;
		Ball->ProjectileMovement->UpdateComponentVelocity();

		Ball->BallSpeed = FinalVelocity.Size();
		Ball->Direction = FinalVelocity.GetSafeNormal();
	}

	//-------------------------------------------------------------------
	//演出・審判への報告処理
	//-------------------------------------------------------------------
	if (SkillManager)
	{
		SkillManager->AddGauge(7.f);
		SkillManager->OnHitBall(Ball);
		StopSkillAura();
	}

	if (GI)
	{
		if (URefree* Ref = GI->GetRefree())
		{
			Ref->RecordHit(EPlayerSide::Player);
		}
	}

	if (HitEffect)
	{
		FVector SpawnLocation = RacketMesh->GetComponentLocation() - Forward * 50.f + FVector(0.f, -50.f, 20.f);
		FRotator SpawnRotation = Forward.Rotation();

		if (UWorld* World = GetWorld())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, SpawnLocation, SpawnRotation, FVector(1.f));
		}
	}

	//スイング
	OnReturnBallSwing();
}


//【入力バインド】 スキルボタン
void ARacket::ActivateSuperSmash()
{
	if (SkillManager && SkillManager->CanActivate())
	{
		SkillManager->ActivateSkill();
		//オーラ開始
		StartSkillAura();
	}
}

void ARacket::StartSkillAura()
{
	if (AuraEffect)
	{
		AuraEffect->Activate(true);
		//デバッグ確認
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Aura ON"));
	}
}

void ARacket::StopSkillAura()
{
	if (AuraEffect)
	{
		AuraEffect->Deactivate();
		//デバッグ確認
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Aura OFF"));
	}
}

void ARacket::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this || !IsValid(OtherComp)) return;

	//相手がABallならポインタ（動的キャスト）
	if (ABall* HitBall = Cast<ABall>(OtherActor))
	{
		if (!IsValid(HitBall)) return;
		Ball = HitBall;
	}
}

void ARacket::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 離れた相手が ABall かを確認
	ABall* HitBall = Cast<ABall>(OtherActor);
	if (HitBall && HitBall == Ball)
	{
		// ボールが範囲外に出た
		Ball = nullptr;
		//デバッグ確認
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Ball left range."));
	}
}
