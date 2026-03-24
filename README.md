# VOLTHUNTER

[cite_start]**「ジャスト回避」からの「即座の切り返し」にフォーカスした、電脳世界が舞台のハイスピード爽快アクションゲーム。** [cite: 1]

![Gameplay Demo](動画のGIFファイルなどをここに配置することをお勧めします)

[cite_start]▶ **[デモプレイ動画（Google Drive）はこちら](https://drive.google.com/file/d/1oZZ7Rmfee3jZ0hF4hYJBio4XAtlKkj8T/view?usp=drive_link)** [cite: 1]

---

## [cite_start]💻 プロジェクト概要 [cite: 1]

* [cite_start]**開発環境:** Unreal Engine 5.5 [cite: 1]
* [cite_start]**制作期間:** 2025年11月 ～ 2026年2月（約4か月） [cite: 1]
* [cite_start]**チーム人数:** 4名 [cite: 1]
* [cite_start]**担当ポジション:** メインプログラマー [cite: 1]

---

## [cite_start]⚙️ 担当領域と実装詳細 [cite: 1]

[cite_start]プレイヤー操作以外のシステム全般および、ゲーム全体の根幹となるアーキテクチャ設計を担当しました。 [cite: 1]

* [cite_start]**ゲーム全体のシステム設計・進行管理** [cite: 1]
* [cite_start]**戦闘システムの構築** [cite: 1]
* [cite_start]**敵AI・思考ルーチンの実装** [cite: 1]
* [cite_start]**イテレーション（バランス調整）を前提としたデータ構造の設計** [cite: 1]

### [cite_start]💡 技術的なこだわり・アピールポイント [cite: 1]

[cite_start]本作のコア体験である「回避成功時の爽快感」を最大化するため、**「敵AIのロジック」**と**「プランナー向けの拡張性」**の2点に注力して設計を行いました。 [cite: 1]

#### [cite_start]1. プランナー視点を重視した「調整しやすい設計」 [cite: 1]
[cite_start]アクションゲームにおいて頻発するバランス調整に耐えうるよう、実装側（プログラマー）の都合ではなく、調整側（プランナー）の視点を重視しました。 [cite: 1][cite_start]数値や挙動の変更が容易に行えるデータ駆動な構造を意識して設計しています。 [cite: 1]

#### [cite_start]2. 意図的な「回避タイミング」を創出する敵AI設計 [cite: 1]
[cite_start]プレイヤーに自然と回避行動を促すため、敵の行動パターンと思考ルーチンを構築しました。 [cite: 1][cite_start]複雑なAIツリーになりすぎないよう処理の流れを一目で分かる構造に整理し、プランナーが意図した「ジャスト回避の隙（タイミング）」を作りやすい行動遷移を実現しています。 [cite: 1]

---

## [cite_start]🎮 操作方法 (Xboxコントローラー推奨) [cite: 1]

[cite_start]本作はゲームパッドでのプレイを想定して調整されています。 [cite: 1]

| アクション | 入力ボタン (Xbox準拠) |
| :--- | :--- |
| **移動** | [cite_start]左スティック | [cite: 1]
| **視点移動** | [cite_start]右スティック | [cite: 1]
| **ジャンプ** | [cite_start]A ボタン | [cite: 1]
| **回避** | [cite_start]B ボタン | [cite: 1]
| **弱攻撃** | [cite_start]X ボタン | [cite: 1]
| **タメ攻撃** | [cite_start]Y ボタン | [cite: 1]
| **ダッシュ** | [cite_start]RB ボタン | [cite: 1]
| **スキル** | [cite_start]RT ボタン | [cite: 1]
| **電線移動** | [cite_start]LB ボタン | [cite: 1]

---

## [cite_start]🚀 インストールと実行方法 [cite: 1]

[cite_start]※ 本作の実行には Windows 環境が必要です。 [cite: 1]

1. [cite_start]GitHubの **[Releases]** ページから `Windows Build v1.0` (zipファイル) をダウンロードします。 [cite: 1]
2. [cite_start]ダウンロードした zip ファイルを任意のフォルダに展開します。 [cite: 1]
3. [cite_start]展開したフォルダ内にある実行ファイル（`.exe`）を起動するとゲームが開始されます。 [cite: 1]
