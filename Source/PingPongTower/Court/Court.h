// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Court.generated.h"

class UBoxComponent;

UCLASS()
class PINGPONGTOWER_API ACourt : public AActor
{
	GENERATED_BODY()
	
public:
	ACourt();

protected:
	virtual void BeginPlay() override;

public:
	//台の見た目
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Court")
	UStaticMeshComponent* CourtMesh;

	//プレイヤーコートの判定エリア
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Court")
	UBoxComponent* PlayerCourtCollision;

	//敵コートの判定エリア
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Court")
	UBoxComponent* EnemyCourtCollision;

	//AIに台の幅（中心からの距離）を教える関数
	float GetCourtHalfWidth() const;
};
