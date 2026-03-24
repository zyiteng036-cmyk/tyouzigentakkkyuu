//------------------------------
// CharacterBase.cpp
// ベースクラス
// 
// 
// キャラクターの共通処理まとめ役
//------------------------------

//インクルード
#include "CharacterBase.h"
#include "CharacterManager.h"

// コンストラクタ
ACharacterBase::ACharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

//スタート時、生成時
void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // マネージャーに登録
    if (UWorld* World = GetWorld())
    {
        if (UCharacterManager* Manager = UCharacterManager::Get(World))
        {
            Manager->RegisterCharacter(this);
        }
    }
}

// 終了時に呼ばれる
void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // マネージャーから解除
    if (UWorld* World = GetWorld())
    {
        if (UCharacterManager* Manager = UCharacterManager::Get(World))
        {
            Manager->UnregisterCharacter(this);
        }
    }
}
