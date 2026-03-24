//------------------------------
// CharacterManager.h
// マネージャークラス
// 
// 
// キャラクターを管理する
//------------------------------

//インクルードガード
#pragma once

//インクルガード
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterManager.generated.h"

//前方宣言
class ACharacterBase;

UCLASS()
class PINGPONGTOWER_API UCharacterManager : public UObject
{
    GENERATED_BODY()

public:
    // シングルトン取得
    static UCharacterManager* Get(UWorld* World);

    // 登録・解除
    void RegisterCharacter(ACharacterBase* Char);
    void UnregisterCharacter(ACharacterBase* Char);

    // 一括更新（必要なら）
    void UpdateAllCharacters(float DeltaTime);

    // キャラクター一覧取得
    const TArray<TWeakObjectPtr<ACharacterBase>>& GetCharacters() const { return Characters; }

    // 登録キャラクター一覧
    UPROPERTY()
    TArray<TWeakObjectPtr<ACharacterBase>> Characters;
private:
    // シングルトン
    static TWeakObjectPtr<UCharacterManager> Singleton;
};
