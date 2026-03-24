//------------------------------
// SkillBase.h
// ベースクラス
// 
//------------------------------

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillBase.generated.h"

class ARacket;
class ABall;

UCLASS(Blueprintable, EditInlineNew)
class PINGPONGTOWER_API USkillBase : public UObject
{
	GENERATED_BODY()

public:
	// スキルを初期化（所有者登録）
	virtual void Initialize(AActor* InOwner);
	// 発動（ボタン押下時）
	virtual void Activate() {}
	// 打球時に呼ばれる（ReturnBallから）
	virtual void OnHitBall(ABall* Ball) {}

	// スキル名（UI表示用）
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FString GetDisplayName() const;
	virtual FString GetDisplayName_Implementation() const { return TEXT("Skill"); }

protected:
	UPROPERTY()
	TWeakObjectPtr<AActor> Owner;
};
