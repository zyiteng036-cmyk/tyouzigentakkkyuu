//------------------------------
// NetArea.h
// クラス
// ネットを大きくしたようなコリジョンを出し
// オーバーラップイベントで判定しボールのバウンド回数をリセット
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetArea.generated.h"


//前方宣言
class UBoxComponent;
class ABall;

UCLASS()
class PINGPONGTOWER_API ANetArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// コンストラクタ
	ANetArea();

protected:
	// 生成時、スタート時
	virtual void BeginPlay() override;


private:
	//オーバーラップ接触し始めた時に呼ばれるイベント関数
	// ネットを超えたとき
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	//コリジョン
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UBoxComponent* NetAreaBox;
};
