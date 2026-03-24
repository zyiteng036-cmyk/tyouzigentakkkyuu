// Fill out your copyright notice in the Description page of Project Settings.


#include "Court.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../PingPongGameInstance.h"

// Sets default values
ACourt::ACourt()
{
	PrimaryActorTick.bCanEverTick = false;

	//メッシュの作成
	CourtMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CourtMesh"));
	RootComponent = CourtMesh;

	//プレイヤー側の判定箱を作成し、メッシュの子にする
	PlayerCourtCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerCourtCollision"));
	PlayerCourtCollision->SetupAttachment(RootComponent);

	//敵側の判定箱を作成し、メッシュの子にする
	EnemyCourtCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("EnemyCourtCollision"));
	EnemyCourtCollision->SetupAttachment(RootComponent);
}

void ACourt::BeginPlay()
{
	Super::BeginPlay();

	// 自分自身をGameInstanceに登録して、誰でも「今の台」にアクセスできるようにする
	if (UPingPongGameInstance* GI = Cast<UPingPongGameInstance>(GetGameInstance()))
	{
		GI->ActiveCourt = this; 
	}
}


float ACourt::GetCourtHalfWidth() const
{
	if (PlayerCourtCollision)
	{
		//判定箱のY軸の長さ（中心からの距離）を返す
		return PlayerCourtCollision->GetScaledBoxExtent().Y;
	}
	return 300.f; 
}


