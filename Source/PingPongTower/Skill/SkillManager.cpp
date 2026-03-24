#include "SkillManager.h"
#include "SkillBase.h"
#include "Skill_SuperSmash.h"
#include "Skill_StopShot.h"
#include "../Character/Player/Racket.h"
#include "../Ball/Ball.h"
#include "Engine/Engine.h"


void USkillManager::Initialize(AActor* InOwner)
{
	Owner = InOwner;

	USkill_SuperSmash* SuperSmash = NewObject<USkill_SuperSmash>(this);
	SuperSmash->Initialize(InOwner);

	USkill_StopShot* StopShot = NewObject<USkill_StopShot>(this);
	StopShot->Initialize(InOwner);

	// 並び順＝UI順
	Skills = { nullptr,SuperSmash, StopShot };
	// UI 表示名（GetDisplayNameでも可だが固定にしておくと軽い）
	SkillNames = { TEXT("None"),TEXT("Super Smash"), TEXT("Stop Shot") };
}

void USkillManager::AddGauge(float Value)
{
	CurrentGauge = FMath::Clamp(CurrentGauge + Value, 0.f, MaxGauge);

	////デバッグ表示
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(
	//		-1, 1.f, FColor::Green,
	//		FString::Printf(TEXT("Gauge: %.1f / %.1f"), CurrentGauge, MaxGauge)
	//	);
	//}
}

bool USkillManager::CanActivate() const
{
	return (CurrentGauge >= MaxGauge) && Skills.IsValidIndex(CurrentSkillIndex) && (Skills[CurrentSkillIndex] != nullptr);
}

void USkillManager::ActivateSkill()
{
	if (!CanActivate()) return;
	// 消費
	CurrentGauge = 0.f;
	// 選択中スキルを発動
	if (USkillBase* Skill = Skills[CurrentSkillIndex])
	{
		Skill->Activate();
	}
}

void USkillManager::OnHitBall(ABall* Ball)
{
	if (!Skills.IsValidIndex(CurrentSkillIndex)) return;
	if (USkillBase* Skill = Skills[CurrentSkillIndex])
	{
		Skill->OnHitBall(Ball);
	}
}

void USkillManager::OnPassedNet(ABall* Ball)
{
	if (!Skills.IsValidIndex(CurrentSkillIndex)) return;

	USkillBase* Skill = Skills[CurrentSkillIndex];
	if (!Skill) return;

	// StopShot の場合だけ処理
	if (USkill_StopShot* StopShot = Cast<USkill_StopShot>(Skill))
	{
		StopShot->OnPassedNet(Ball);
	}
}

//現在選択スキルを反映
void USkillManager::SetSelectedSkill(int32 Index)
{
	if (!Skills.IsValidIndex(Index)) return;

	CurrentSkillIndex = Index;

	////スキルに Owner を再適用（敵が動くようになる）
	//if (Skills[Index])
	//{
	//	Skills[Index]->Initialize(Owner.Get());
	//}
}
