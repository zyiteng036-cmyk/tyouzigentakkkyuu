//------------------------------
// CharacterBase.h
// ベースクラス
// 
// 
// キャラクターの共通処理まとめ役
//------------------------------

//インクルードガード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

UCLASS()
class PINGPONGTOWER_API ACharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    // コンストラクタ
    ACharacterBase();

    // 共通移動処理
    virtual void UpdateMove(float DeltaTime) {}

protected:
    //スタート時、生成時
    virtual void BeginPlay() override;
    // 終了時に呼ばれる
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
