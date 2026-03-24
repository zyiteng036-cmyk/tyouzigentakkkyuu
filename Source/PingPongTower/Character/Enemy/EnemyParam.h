//------------------------------
// EnemyParam.h
// 構造体
// 
// 敵のパラメーターを管理
// 
//------------------------------


//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyParam.generated.h"

// 敵の強さデータ（CSV 1行を表す）
USTRUCT(BlueprintType)
struct FEnemyParam : public FTableRowBase
{
	GENERATED_BODY()

public:


	//移動速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 400.f;

	//返球速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReturnSpeed = 900.f;

	//反応遅延
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReactionDelay = 0.5f;

	//精度（近い）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrecisionThresholdClose = 250.f;

	//精度（遠い）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrecisionThresholdFar = 370.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillIndex = 0;
};
