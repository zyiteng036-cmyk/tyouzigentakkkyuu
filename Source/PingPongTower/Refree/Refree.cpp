//------------------------------
// Refree.cpp
// 審判
// 得点
// 
//------------------------------


#include "Refree.h"
#include "../Skill/SkillManager.h"
#include "../Character/Player/Racket.h"
#include "../Court/Court.h"
#include "../Ball/Ball.h"
#include "../Character/Enemy/Enemy.h"
#include "../Tower/TowerManager.h"
#include "../PingPongGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

//コンストラクタ
URefree::URefree()
	:MaxScore(5)
	, PlayerScore(0)
	, EnemyScore(0)
	, LastHitter(EPlayerSide::None)
	, CurrentServer(EPlayerSide::Player)
	, ServeCount(0)
	, bServerChanged(false)
	, bIsScore(false)
{

}

//スコアのリセット
void URefree::ResetScore()
{
	//プレイヤースコア
	PlayerScore = 0;
	//敵のスコア
	EnemyScore = 0;
	//サーバーをプレイヤーに
	CurrentServer = EPlayerSide::Player;
	//サーブ回数リセット
	ServeCount = 0;

	//デバッグ
	UE_LOG(LogTemp, Warning, TEXT("ResetScore CALLED (Debug check)"));
}

//ゲームセット
void URefree::GameSet()
{

	if (UWorld* World = GetWorld())
	{
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			// スコアを書き出し（どのモードでも共通）
			GI->FinalPlayerScore = PlayerScore;
			GI->FinalEnemyScore = EnemyScore;

			//モード分岐：Tower なら TowerManager に任せて return
			if (GI->SelectedGameMode == EGameModeType::Tower)
			{
				if (GI->TowerManager)
				{
					if (PlayerScore > EnemyScore) GI->TowerManager->OnPlayerWin(World);
					else                          GI->TowerManager->OnPlayerLose(World);
				}

				ResetScore(); // 念のためここでリセット
				return;
			}

			//通常対戦モード（Versus）は ResultLevel に遷移
			FTimerHandle TimerHandle;
			TWeakObjectPtr<UWorld> WeakWorld = World;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				[WeakWorld]()
				{
					if (WeakWorld.IsValid())
					{
						UGameplayStatics::OpenLevel(WeakWorld.Get(), FName("ResultLevel"));
					}
				},
				0.5f, false
			);
		}
	}

	ResetScore();
}

//点数処理
void URefree::SetScore()
{
	if (UWorld* World = GetWorld())
	{
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			Ball = GI->ActiveBall; //審判が今のボールを認識
		}
	}

	if (!IsValid(Ball)) return;
	//タイトル画面の時
	if (UWorld* World = GetWorld()) {
		if (UPingPongGameInstance* Instance = Cast<UPingPongGameInstance>(World->GetGameInstance()))
		{
			if (Instance->SelectedGameMode == EGameModeType::Title)
			{
				return;
			}
		}
	}

	//スコアが入ったかどうか
	bIsScore = false;
	//サーバー交代フラグをリセット
	bServerChanged = false;

	FName CurrentTag = Ball->GetCurrentTag();
	int32 Bounds = Ball->GetBoundCount();

	if (!Ball) return;

	FString HitterStr = (LastHitter == EPlayerSide::Player) ? TEXT("Player") : (LastHitter == EPlayerSide::Enemy) ? TEXT("Enemy") : TEXT("None");
	UE_LOG(LogTemp, Error, TEXT("[Refree Score Check] -------------------------"));
	UE_LOG(LogTemp, Error, TEXT("LastHitter : %s"), *HitterStr);
	UE_LOG(LogTemp, Error, TEXT("CurrentTag : %s"), *CurrentTag.ToString());
	UE_LOG(LogTemp, Error, TEXT("LastTag    : %s"), *Ball->GetLastTag().ToString());
	UE_LOG(LogTemp, Error, TEXT("BoundCount : %d"), Bounds);
	UE_LOG(LogTemp, Error, TEXT("----------------------------------------------"));

	////------コートに触れず床に落ちた場合--------------------------------------------------------
	if (CurrentTag == FName("Ground"))
	{
		if (Bounds == 0)
		{
			//ノーバンで直接落ちた
			//→ 打った人のミス
			if (LastHitter == EPlayerSide::Player) EnemyScore++;
			else PlayerScore++;

			bIsScore = true;
		}
		else if (Bounds == 1)
		{
			//1回だけコートにバウンドして落ちた
			FName LastCourt = Ball->GetLastTag(); //最後にバウンドしたコート

			if (LastHitter == EPlayerSide::Player)
			{
				//自分が打った球の場合
				if (LastCourt == FName("EnemyCourt")) PlayerScore++; //敵コートに当たって落ちた
				else EnemyScore++;                                   //自コートに当たって落ちた
			}
			else if (LastHitter == EPlayerSide::Enemy)
			{
				//敵が打った球の場合
				if (LastCourt == FName("PlayerCourt")) EnemyScore++; //プレイヤーコートに当たって落ちた
				else PlayerScore++;                                  //敵が自分のコートに当てて落ちた
			}
			bIsScore = true;
		}
	}
	//------コート内で二回バウンドしたら--------------------------------------------------------
	else if (Bounds >= 2)
	{
		//どちらの陣地で2回バウンドしたかで判定
		if (CurrentTag == FName("PlayerCourt"))
		{
			EnemyScore++; //プレイヤー陣地での失点
			bIsScore = true;
		}
		else if (CurrentTag == FName("EnemyCourt"))
		{
			PlayerScore++; //敵陣地での失点
			bIsScore = true;
		}
	}

	if (!bIsScore) return;


	//スコアが入ったらボールを消す
	AddSkillGauge(); // ゲージ加算

	//点が入った時の音
	if (ScoreSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ScoreSound);
	}

	if (IsValid(Ball))
	{
		Ball->Destroy();
	}
	Ball = nullptr;

	//どちらかがセットポイントを取ったら
	if (PlayerScore >= MaxScore || EnemyScore >= MaxScore)
	{
		GameSet();
		return;
	}

	//サーブ交代処理
	ServeCount++;
	if (ServeCount >= 2)
	{
		CurrentServer = (CurrentServer == EPlayerSide::Player) ? EPlayerSide::Enemy : EPlayerSide::Player;
		ServeCount = 0;
		bServerChanged = true;
	}

	//次サーブの予約
	RequestNextServe();
}


//打球を記録する
void URefree::RecordHit(EPlayerSide Hitter)
{
	LastHitter = Hitter;

	//誰かが打ち返した瞬間に、ボールのバウンド履歴をリセット
	if (UWorld* World = GetWorld())
	{
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			if (ABall* ActiveBall = GI->ActiveBall)
			{
				ActiveBall->ResetBound();
			}
		}
	}
}

//現在のサーバー
bool URefree::CanServe(EPlayerSide Side) const
{
	return (Side == CurrentServer);
}


//サーバーの交代
void URefree::ChangeServer()
{
	//現在のサーバー（プレイヤー）→（エネミー）
	//現在のサーバー（エネミー）→（プレイヤー）
	CurrentServer = (CurrentServer == EPlayerSide::Player)
		? EPlayerSide::Enemy
		: EPlayerSide::Player;

	////デバッグ
	//FString DebugText = (CurrentServer == EPlayerSide::Enemy)
	//	? TEXT("Server: Enemy")
	//	: TEXT("Server: Player");
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, DebugText);

	// サーバー交代時
	RequestNextServe();
}


//次のサーバーの予約
void URefree::RequestNextServe()
{
	//現在のゲームワールドを取得
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	//現在のサーバーが「敵」なら
	if (CurrentServer == EPlayerSide::Enemy)
	{
		//現在のワールドに存在する AEnemy 型アクターを1体ずつ取得
		for (TActorIterator<AEnemy> It(World); It; ++It)
		{
			AEnemy* Enemy = *It;

			//2秒敵がサーブを開始
			if (Enemy)
			{
				FTimerHandle DelayHandle;
				TWeakObjectPtr<AEnemy> WeakEnemy = Enemy;
				World->GetTimerManager().SetTimer(
					DelayHandle,
					[WeakEnemy]()
					{
						if (WeakEnemy.IsValid())
						{
							WeakEnemy->ServeBall();
						}
					},
					2.f, false
				);				break;
			}
		}
	}

	//サーバーが敵の場合
	else if (CurrentServer == EPlayerSide::Player)
	{
		//GEngine->AddOnScreenDebugMessage(
		//	-1, 1.0f, FColor::Cyan,
		//	TEXT("Player turn to serve (press Serve key)."));
	}
}

void URefree::ProcessFault(EPlayerSide FaultPlayer)
{
	//反則をした側の相手にポイントを入れる
	if (FaultPlayer == EPlayerSide::Player)
	{
		EnemyScore++;
		UE_LOG(LogTemp, Error, TEXT("Player Fault (Direct Volley)! Enemy gets a point."));
	}
	else if (FaultPlayer == EPlayerSide::Enemy)
	{
		PlayerScore++;
		UE_LOG(LogTemp, Error, TEXT("Enemy Fault (Direct Volley)! Player gets a point."));
	}

	AddSkillGauge();

	if (ScoreSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ScoreSound);
	}

	//ボールを破棄
	if (UWorld* World = GetWorld())
	{
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			if (ABall* ActiveBall = GI->ActiveBall)
			{
				ActiveBall->Destroy();
				GI->ActiveBall = nullptr;
			}
		}
	}
	Ball = nullptr;

	//勝敗チェック
	if (PlayerScore >= MaxScore || EnemyScore >= MaxScore)
	{
		GameSet();
		return;
	}

	//サーブ交代処理
	ServeCount++;
	if (ServeCount >= 2)
	{
		CurrentServer = (CurrentServer == EPlayerSide::Player) ? EPlayerSide::Enemy : EPlayerSide::Player;
		ServeCount = 0;
		bServerChanged = true;
	}

	RequestNextServe();
}

//スキルゲージを加算
void URefree::AddSkillGauge()
{

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ARacket* Player = Cast<ARacket>(PC->GetPawn()))
			{
				if (Player->SkillManager)
				{
					Player->SkillManager->AddGauge(15.f); // ← 点ごとのゲージ量
				}
			}
		}
	}

	for (TActorIterator<AEnemy> It(GetWorld()); It; ++It)
	{
		AEnemy* Enemy = *It;
		if (Enemy && Enemy->SkillManager)
		{
			Enemy->SkillManager->AddGauge(25.f);
		}
	}

}

void URefree::OnBallBouncedCallback(FName CourtTag)
{
	//スコアの判定処理
	SetScore();

	//ボール側のフラグをtrueにする
	if (UWorld* World = GetWorld())
	{
		if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			if (ABall* CurrentActiveBall = GI->ActiveBall)
			{
				CurrentActiveBall->bBallSpawn = true;
			}
		}
	}
}
