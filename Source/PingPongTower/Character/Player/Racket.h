//------------------------------
// Racket.h
// クラス
// キャラクターの移動関連
// 見た目
// コリジョンのオーバーラップで判定し打つ
// 角度をつけた返球
// 
//------------------------------

//インクルードカード
#pragma once

//インクルード
#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "../CharacterBase.h"
#include "Racket.generated.h"


//前方宣言
//class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class ABall;
class UNiagaraSystem;
class UNiagaraComponent;
class USkillManager;

UCLASS()
class PINGPONGTOWER_API ARacket : public ACharacterBase
{
    GENERATED_BODY()

public:
    // コンストラクタ
    ARacket();

protected:
    //スタート時、または生成時の処理
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
    //毎フレーム処理
    virtual void Tick(float DeltaTime) override;

    //入力バインド
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    //移動
    virtual void UpdateMove(float DeltaTime) override;

    // 移動処理
    //【入力バインド】左右
    void MoveRight(float Value);

    //【入力バインド】前後
    void MoveForward(float Value);

    //【入力バインド】左右の角度
    void LookRight(float value);

    //【入力バインド】前後方向
    void LookUp(float Value);

    //【入力バインド】サーブ
    void ServeBall();

    //【入力バインド】コリジョン内でボタンを押すと球を返す
    void ReturnBall();
    //スイング
    UFUNCTION(BlueprintImplementableEvent)
    void OnReturnBallSwing();

    //【入力バインド】 スキルボタン
    void ActivateSuperSmash();

    // スキル発動演出制御
    void StartSkillAura();
    void StopSkillAura();



protected:
    //オーバーラップ接触し始めた時に呼ばれるイベント関数
    // コリジョン内のボールを探す用
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // コリジョンから出た時
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UFUNCTION(BlueprintCallable)
    class USkillManager* GetSkillManager() const { return SkillManager; }


public:
    // 現在存在しているボール（重複生成防止用）
    UPROPERTY()
    ABall* SpawnedBall;

    UPROPERTY()
    USkillManager* SkillManager;


protected:
    //一人称カメラ

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    //USpringArmComponent* m_pSpringArm;		//スプリングアーム

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USceneComponent* m_pCameraRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* m_pCamera;			//カメラ

    // Niagaraエフェクト
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    UNiagaraSystem* HitEffect;

    //プレイヤーの横に置くコリジョン
    //このコリジョン内でラケットを振ると球をタイミング関係なく打ち返す
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    UBoxComponent* SideCollision;

    // 見た目（StaticMesh）
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* RacketMesh;


    // 移動速度
    UPROPERTY(EditAnywhere, Category = "Racket")
    float MoveSpeed;

    // サーブ発射位置
    UPROPERTY(VisibleAnywhere, Category = "Serve")
    USceneComponent* ProjectileLocation;



    // ボールの参照（範囲内のボールを保持）
    ABall* Ball;


    // サーブで使うクラス
    UPROPERTY(EditDefaultsOnly, Category = "Spawn")
    TSubclassOf<class ABall> BallClass;


    // 角度の反映度（0〜1）
    UPROPERTY(EditAnywhere, Category = "ReturnBall")
    float AngleScale = 0.5f; 

    // スティック反応のしきい値
    UPROPERTY(EditAnywhere, Category = "ReturnBall")
    float SwingThreshold = 0.7f;
private:
    //スキル使用中
    UPROPERTY(VisibleAnywhere, Category = "Effect")
    UNiagaraComponent* AuraEffect;

    UPROPERTY(EditAnywhere, Category = "Effect")
    UNiagaraSystem* AuraNiagara;

    //キャラ移動入力
    FVector2D CharaMoveInput;

    //スティックの左右の角度入力
    float CurrentYaw;
    float InputYaw;

    //操作可能な状態か？
    bool m_bCanControl;

    //方向入力保持（右スティック）
    float AimX;
    float AimY;

    float ReturnSpeed;

    // スイング制御
    bool bCanSwing;
    float SwingCooldownTimer;
};
