//------------------------------
// CharacterManager.h
// マネージャークラス
// 
// 
// キャラクターを管理する
//------------------------------

//インクルード
#include "CharacterManager.h"
#include "CharacterBase.h"
#include "Engine/World.h"

// シングルトン初期化
TWeakObjectPtr<UCharacterManager> UCharacterManager::Singleton;

// シングルトン取得
UCharacterManager* UCharacterManager::Get(UWorld* World)
{
    // Singletonが無効なら新規作成
    if (!Singleton.IsValid() && World)
    {
        UCharacterManager* NewManager = NewObject<UCharacterManager>(World, UCharacterManager::StaticClass(), NAME_None, RF_Transient);
        // Singletonに保持
        Singleton = NewManager;
    }
    // Singleton返す
    return Singleton.Get();
}

// 登録
void UCharacterManager::RegisterCharacter(ACharacterBase* Char)
{
    if (Char && !Characters.Contains(Char))
    {
        // 配列に追加
        Characters.Add(Char);
    }
}

//解除
void UCharacterManager::UnregisterCharacter(ACharacterBase* Char)
{
    if (Char)
    {
        // 配列から削除
        Characters.Remove(Char);
    }
}

// 一括更新
void UCharacterManager::UpdateAllCharacters(float DeltaTime)
{

    Characters.RemoveAll([](const TWeakObjectPtr<ACharacterBase>& Char) { return !Char.IsValid(); });

    // 配列を順にチェック
    for (auto& Char : Characters)
    {
        // 有効なキャラクターのみ
        if (Char.IsValid())
        {
            // Base に共通処理ある場合
            Char->UpdateMove(DeltaTime);  
        }
    }
}
