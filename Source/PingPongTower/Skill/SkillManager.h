#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SkillManager.generated.h"

class USkillBase;
class ARacket;
class ABall;

UCLASS(Blueprintable)
class PINGPONGTOWER_API USkillManager : public UObject
{
    GENERATED_BODY()

public:
    // 所有者とスキル生成
    void Initialize(AActor* InOwner);

    // ゲージ管理
    void AddGauge(float Value);
    bool CanActivate() const;

    UFUNCTION(BlueprintCallable)
    float GetGaugePercent() const { return CurrentGauge / MaxGauge; }

    // 発動
    void ActivateSkill();

    // 打球時にスキルへ通知
    void OnHitBall(ABall* Ball);

    void OnPassedNet(ABall* Ball);

    //現在選択スキルを反映
    UFUNCTION()
    void SetSelectedSkill(int32 Index);

    // UI用：全スキル名を取得
    UFUNCTION(BlueprintCallable)
    const TArray<FString>& GetSkillNames() const { return SkillNames; }

    // UI用：現在スキル名
    UFUNCTION(BlueprintCallable)
    FString GetCurrentSkillName() const { return SkillNames.IsValidIndex(CurrentSkillIndex) ? SkillNames[CurrentSkillIndex] : TEXT(""); }


public:

    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    float CurrentGauge = 0.f;
    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    float MaxGauge = 100.f;

private:
    UPROPERTY()
    TWeakObjectPtr<AActor> Owner;

    // 実スキルインスタンス群
    UPROPERTY()
    TArray<USkillBase*> Skills;

    // UI 表示用のスキル名（Skillsの並びと一致させる）
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    TArray<FString> SkillNames;

    // 選択中
    int32 CurrentSkillIndex = 0;

};
