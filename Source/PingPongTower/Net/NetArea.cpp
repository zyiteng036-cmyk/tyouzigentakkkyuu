//------------------------------
// NetArea.cpp
// 処理
// ネットを大きくしたようなコリジョンを出し
// オーバーラップイベントで判定しボールのバウンド回数をリセット
//------------------------------


#include "NetArea.h"
#include "../Skill/SkillManager.h"
#include "../Character/Player/Racket.h"
#include "Components/BoxComponent.h"
#include "../Ball/Ball.h"
#include "Engine/Engine.h"

// コンストラクタ
ANetArea::ANetArea()
	:NetAreaBox(NULL)
{
 	
	PrimaryActorTick.bCanEverTick = true;


	//コリジョンを生成
	NetAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("NetAreaBox"));
	RootComponent = NetAreaBox;
	
	if (NetAreaBox != NULL)
	{
		//大きさ
		NetAreaBox->SetBoxExtent(FVector(10.f, 1000.f, 200.f));
		//衝突プロファイル
		NetAreaBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		//オーバーラップイベントを動かすか
		NetAreaBox->SetGenerateOverlapEvents(true);
	}
}

// 生成時、スタート時
void ANetArea::BeginPlay()
{
	Super::BeginPlay();
	
	NetAreaBox->OnComponentBeginOverlap.AddDynamic(this, &ANetArea::OnOverlapBegin);

}

//ネットエリアを超えた時
void ANetArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//触れたのがボールであるかを判定
	ABall* Ball= Cast<ABall>(OtherActor);
	if (Ball)
	{
		Ball->ResetBound();

		if (Ball->IsSmashArmed())
		{
			// 直接止めるのではなく、シーケンスを開始してネットを通り過ぎるのを待つ
			Ball->StartSmashSequence();
			return;
		}

		// --- ストップショット連携 ---
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ARacket* Player = Cast<ARacket>(PC->GetPawn()))
				{
					if (Player->SkillManager)
					{
						Player->SkillManager->OnPassedNet(Ball); 
					}
				}
			}
		}
	}
}