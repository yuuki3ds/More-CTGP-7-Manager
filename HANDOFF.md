# More！CTGP-7 Manager — 引き継ぎメモ(Ver.1.2.9 開発中、1.2.8 リリース済み)

Claude Code ではこのフォルダをそのまま作業フォルダにしてください(`CLAUDE.md` に作業ルールがあります)。
これまでのやり取りは `docs/conversation_log.md` にあります。

## バージョンの状況
- リリース済み: **1.2.8**(2026-11-13)。コード上は `APP_VERSION "Ver.1.2.9"`(1.2.9 開発中)
- 1.2.9 でここまでに入れた変更: まだなし
- 1.2.8 の主な内容(Claude Code の環境でビルド):
  - ボタンの沈み込みを、タッチを始めたボタンだけにした。ウィンドウ表示中は一番手前のウィンドウのボタンだけ沈む
  - ダークモードのみ、上画面右上に 3UP ムーン(白黒、縁がうっすら黄色く光る、7秒ごとに控えめにキラン)
- 1.2.7 の主な内容: スリープ禁止 / SD 空き容量の通知 / 3D 表示 / CTGP-7 バージョン表示 / ロゴの光 / Extra のバージョン番号表示 /
  txt・3gx の選択と MK7 3gx の地域選択 / メニュー見出しの薄いプレート / お知らせリスト送信(設定「お知らせを送信」)/ LEDランプ名の変更

## 概要
- 3DS Homebrew アプリ。More！CTGP-7(CTGP-7 のキャラ追加パック)のキャラフォルダを切り替える。
- 作成者: yk / 3DSProject Team
- GitHub: https://github.com/yuuki3ds/More-CTGP-7-Manager
- 形式: .3dsx(Homebrew Launcher)と .cia(ホームメニュー)の両方
- ソースは `source/main.c` 1ファイル(C / libctru + citro2d + libcurl)

## フォルダ切り替えの仕様
- 対象: `sdmc:/CTGP-7/MyStuff/`
- 有効なフォルダ = `Characters`(数字なし)、待機 = `Characters1` / `2` / `3`
- 切り替え: 現在の `Characters` に番号を付けて戻す → 選んだフォルダの番号を消す(失敗時はロールバック)
- 有効フォルダはフォルダの存在状況から自動判定(状態ファイルなし)
- 起動時に足りない番号付きフォルダを作成(`ensure_slots()`)。`Characters` があれば、欠けている一番小さい番号の代わりとして扱う。`sdmc:/CTGP-7` が無ければ何も作らない

## 画面構成
### 上画面
- 左上: Wi-Fi(扇形3本)+ 時刻(「:」が秒に合わせて点滅、12/24時間切替)
- 右上: 電池アイコン + %(充電中は稲妻、20%未満で赤)
- 中央上: ロゴ画像(動かさず、6秒ごとに光が横切るだけ)、その下に現在の有効フォルダ・メッセージ(緑のメッセージは文字の中だけ光が流れる)
- 右上(ダークモードのみ): 3UPムーン(gfx/moon.t3s: moon.png 灰色、moon_glow.png 黄色の光)。控えめ(39x40)。光は3秒周期で弱く明滅、7秒ごとに薄い光が横切り小さな星がキラン。奥行き DEPTH_MOON 3
- 右下(x390): アプリバージョン(0.6, y182)、その下に「パック: x」(y203)「CTGP-7: x」(y217)(0.42)
- 下部: ゴールテープ柄が左へスクロール、マリオ・クッパ・甲羅が走り抜ける演出。その上空をジュゲムがふわふわ通過

### 下画面(メイン)
- Characters1〜3(有効なボタンは右半分が緑で「有効」表示。光が左から右へ切れ目なくループ)
- CTGP-7を起動(公式Launcher タイトルID `0004000003070C00` 固定。文字は左寄せ、右端にマリオ&ルイージ)
- ボタン画像は MK7 のスプライトシート由来(`tinyButtonBackground.png` / `selTinyButtonBackground.png`)。有効ボタンの右端に手を挙げたマリオ(`chars.t3s`)
- 左上: フォルダ名変更アイコン / 左下: 「Ex」アイコン(Extra)/ 右上: 歯車(設定)。枠は MK7 のカップアイコンのレンズ(`lens.t3s`)で、カーソル時は金枠+オレンジ。白い吹き出しも表示

### 設定(スクロールリスト、LIST_N 15)
| 番号 | 項目 |
|---|---|
| 0 | 言語(自動/日本語/English) |
| 1 | 時刻表示 |
| 2 | ダークモード |
| 3 | 効果音 |
| 4 | BGM |
| 5 | LEDランプ(`cfg.led`) |
| 6 | お知らせを送信(`cfg.news`) |
| 7 | 切り替え前に確認 |
| 8 | 切り替え後にアプリを終了 |
| 9 | 起動時に更新を確認 |
| 10 | 背景の色をカスタマイズ |
| 11 | アプリの更新(ダウンロードアイコン) |
| 12 | アプリを再起動(`SET_RESTART`、cia のみ表示・確認 `DLG_RESTART`、再起動アイコン) |
| 13 | 設定を初期化(ゴミ箱アイコン) |
| 14 | このアプリについて |
| 15 | 戻る |

- .3dsx では 12 を隠す(`set_pos` / `set_item` で表示位置と項目を対応づけ)
- 確認ウィンドウの種類: `DLG_SWITCH, DLG_RESET, DLG_LAUNCH, DLG_FOLD_SAVE, DLG_FOLD_RESET, DLG_RESTART, DLG_EX_FORMAT, DLG_EX_REGION`
- ウィンドウの重なり番号 `VIS_NOW`: 1 ノート / 2 赤枠の通知 / 3 更新ウィンドウ / 4 アプリ更新ウィンドウ / 5 確認ウィンドウ
- 赤枠の通知の種類(ビット): 1 Wi-Fi / 2 電池20%未満 / 4 電池5%以下 / 8 CTGP-7 なし / 16 SD 空き不足。表示順は 8, 16, 1, 2, 4

補足:
言語 / 時刻表示 / ダークモード / 効果音 / BGM / LEDランプ / お知らせを送信 / 切り替え前に確認 / 切り替え後にアプリを終了 /
起動時に更新を確認 / 背景の色をカスタマイズ / アプリの更新 / アプリを再起動(cia のみ・確認あり) / 設定を初期化 / このアプリについて / 戻る
(ON/OFF項目は緑/赤に光るスイッチ表示)
カーソルとスクロール位置は起動中のみ保持(次回起動時は先頭)
見出しと「戻る」は紺の帯+金の区切り線、スクロール範囲は暗めの領域で区別

### フォルダの管理色
- 各フォルダに色(色相)を持たせる。既定は 1:赤(0) 2:黄(48) 3:緑(128)。`sdmc:/3ds/MoreCTGP7Manager/colors.txt` に保存
- カスタムフォルダの名前欄をタッチ/カーソルで背景色の画面を流用した色変更画面へ(`bg_target`、レインボーなし)。変更中はランプがその色で点灯
- フォルダ切り替えに成功すると、その管理色でランプを1秒点灯

### 3D 表示(上画面)
- 3D ボリュームに合わせて左右の目を描き分ける(`top` / `top_r`、`C2D_ViewTranslate`)。
  奥から 背景 `DEPTH_BG 4`(1.03倍に拡大)・ムーン `DEPTH_MOON 3`・マリオ達 `DEPTH_RACERS 2`・文字(画面上 0)・ジュゲム `DEPTH_LAKITU -2`・ロゴ `DEPTH_LOGO -3`

### スリープ・SD 空き容量
- ダウンロード/インストール中はスリープを禁止(`aptSetSleepAllowed`)
- 起動時に空き 32MB 未満、または更新(1形式 24MB)/チート DL(1MB)の前に不足していれば赤枠の通知(`sd_has()`)

### お知らせリスト
- アプリ更新の完了時に HOME メニューのお知らせへ「Ver.〇 アップデートが完了しました！」を送信(`news_send_update()`、news:u)。設定「お知らせを送信」(`news`)で ON/OFF
- 本文は GitHub の `version/changelog.ini` の `jp=` 行(日本語)か `en=` 行(英語)のうちアプリの言語に合う方(更新のダウンロード時に取得)。無ければ定型文
- タイトルは最大32文字、本文は最大1000文字(UTF-16)。`jp=` だけの行は空行。最後に全角スペースで右寄せした「More！CTGP-7 / yk/3DSProject Team」を入れる見本が `version/changelog.ini`
- 1.2.7 から入った機能なので、実際に届くのは 1.2.7 → 1.2.8 の更新から
- リリース手順: ビルド → リリース公開 → changelog.ini を更新 → 最後に appver.ini を更新

### LEDランプ
- 設定「LEDランプ」(`led`)が OFF のときは一切点灯しない
- 赤枠の通知(CTGP-7 未検出・Wi-Fi・バッテリー)を表示したときにオレンジで1秒1回(`led_flash_alert()`)
- アプリ更新(cia はインストール完了時)と Extra のダウンロード完了時に濃い緑で1秒1回(`led_flash_done()`)
- アプリ/パックの更新が見つかると薄めの水色に1秒×2回点灯(`led_blink()`、MCU レジスタ 0x2D)。終了時は必ず消灯。mcu::HWC が必要なので .3dsx では点かない場合がある

### L / R ボタン(メイン画面のみ)
- R: 歯車へカーソル移動、合っていれば設定を開く / L: カスタムフォルダへカーソル移動、合っていれば開く
- アイコンの横にシステムフォントの記号(U+E005 / U+E004)を表示

### シークバー
- MK7 の画像から切り出した溝とつまみ(`seek.t3s`)。無地の部分だけ伸ばし、つまみの矢印は両端に移動。背景の色の画面は溝のみで、つまみは手
- 「設定を初期化」の文字の左に、歯車付きのゴミ箱マーク(`trash.t3s`、切り出し画像)

### ボタンの種類
- Characters1〜3 と CTGP-7を起動: 画像ボタン
- それ以外: レンズのように中身が透明なボタン(`draw_glass_button()`)。カーソル時は黄色+淡い金の帯、文字は白+濃い縁取り
- 戻る: 左下の矢印+Bのタブ(`back_tab.png` / `draw_back_tab()`)。B ボタンとタッチでも戻れる
- 丸いアイコン(Ex・歯車・!)はレンズ枠
- 「このアプリについて」の右下の「!」で、AI 生成物と Claude の使用についての説明を表示

### 確認ウィンドウ
- 「設定を初期化」・カスタムフォルダの「リセット」(どちらも初期カーソルはいいえ)、「変更を保存」、「CTGP-7を起動」は常に確認する。「切り替え前に確認」の設定はフォルダ切り替えにだけ効く

### カスタムフォルダ
- メイン左上の丸アイコン(`draw_folder_button()`)から開く
- 「変更を保存」で SD 上の待機フォルダ名も変更する(`names_apply()`)。有効フォルダは `Characters` のままで、切り替えで外れるときに新しい名前になる
- 名前の入れ替えができるよう一時名を経由し、失敗時は元に戻す
- `names.txt` が無いとカスタム名のフォルダを認識できない(`Characters1` などを新規作成してしまう)
- 名前は `sdmc:/3ds/MoreCTGP7Manager/names.txt` に1行ずつ保存。「設定を初期化」では消えない
- 本体キーボードは libctru の swkbd で呼ぶ(最大16文字)。「変更を保存」を押すまで保存しない。リセットも保存で確定

### 下画面の背景
- 四角い粒子が動く(`draw_particles()`)。大きい粒子は画面全体を右へ、小さい粒子は中央付近を左へ流れる
- 速さは `PART_BIG_MS` / `PART_SMALL_MS`(模様が1周する時間)、濃さは `draw_particles()` の色で調整

### 背景の色をカスタマイズ
- 色相だけを選ぶ(`config.ini` の `bg_hue`、0-359、-1 は既定の色)。上下の背景に同じ色を重ねる(`bg_tint_update()`)
- 濃さは固定で、ライトは淡く、ダークは暗い色。操作中はリアルタイムに反映し、「保存」で確定、保存せず戻ると元の色
- 「設定を初期化」で既定の色に戻る
- つまみと目盛りは指さしの手(`hands.t3s`)。既定の色のときは握った手になり、何も指さない
- 「ランダム」は全範囲 0〜359 から選ぶ
- 「レインボー」は色相を自動で一周(30秒で一定の速さ、`bg_rainbow`)。シークバー操作・ランダム・リセットで解除、保存で次回も継続

### スライド表示
- 画面を開くたびに(起動時を含む)下画面のボタン・項目が左の画面外から上の項目から順にスライドする
- 見出し、スクロールバー、ダイアログ、上画面は対象外
- 描画だけの演出で、入力はスライド中も最終位置で受け付ける

### Extra
- 見出し「3DSProject Team」
- MK7_Cheat は赤系、ACNL_Cheat は緑系の名札(ライトは淡色、ダークは黒っぽい色)(`draw_name_plate()`、ボタン画像は使わない)
- MK7_Cheat(30600/30700/30800.txt)、ACNL_Cheat(86200.txt)を GitHub の `extra/` から `sdmc:/cheats/` にDL
- バージョンは 30600.txt / 86200.txt の1行目から番号だけを取り出して表示
- ダウンロード時に txt / 3gx を選ぶ。3gx は `extra/3gx/<名前>.3gx`(例: MK7_Cheat.3gx、全リージョン共通)を1回ダウンロードし、各 `sdmc:/luma/plugins/<タイトルID>/` に置く。MK7 は3gxを選ぶと続けて地域(JPN/USA/EUR、初期位置は本体の地域)を選び、その1か所だけに置く(DLG_EX_REGION、EX_PICK_REGION)。`EX_3GX_READY[]`(項目ごと)が false の間は「準備中」で選べない。現在 MK7=true、ACNL=false。空き容量は 16MB 必要
- 左の「!」ボタン(レンズ枠)で対応リージョン表示(MK7: JPN/USA/EUR、ACNL: JPN)

## 通知・更新
- 起動時: `sdmc:/CTGP-7` が無いときの通知、Wi-Fi未接続通知、パック更新通知(morever.ini)、アプリ更新通知(appver.ini)
- バッテリー 20%未満で通知、5%以下で「!!!」付きの2回目の通知
- アプリ内アップデート: 最新リリースの `MoreCTGP7Manager.3dsx` / `.cia` をDLしてインストール
  - .3dsx: 起動中のファイルを置き換え / .cia: AM でインストール後、再起動ボタン
  - インストール中は romfs を閉じる(これで .3dsx・CIA とも 1.2.1→1.2.2 の更新成功を実機確認済み)
  - 失敗時は「(段階: エラーコード)」を表示
  - 「アプリの更新」画面の「更新のチェック」の下に、もう片方の形式を扱える場合だけ追加ボタンを表示(新しい版の有無に関係なく押せる。起動中の形式が最新なら、もう片方だけを入れる。未確認なら先に更新確認を行う)
    - もう片方がある: 「3dsx(cia)も一緒に更新」/ ない: 「3dsx(cia)版も一緒にインストール」
    - 「ダウンロードしてインストール」は従来どおり起動中の形式のみ更新
    - 検出: .cia は タイトルID `000400000F7C7100` の有無、.3dsx は `sdmc:/3ds/MoreCTGP7Manager.3dsx` と `sdmc:/3ds/MoreCTGP7Manager/MoreCTGP7Manager.3dsx`(なければ前者に新規配置)
    - .3dsx 起動中に .cia を入れるときは、同じ版でも入るよう先に既存タイトルを削除する
    - インストール中は BGM・テープ・走行演出・状態取得を止める(BUSY_SYMBOL は鳴らし続ける)
    - 更新中は「更新のチェック」を、更新確認中はインストール系ボタンを非表示
    - 2形式とも DL・検証してからインストール開始(もう片方 → 起動中の形式の順)。.3dsx 起動時に AM が使えなければボタンを出さない
  - アップデート中、Extra のダウンロード中、「アプリの更新」画面の更新確認中は START と HOME を無効化(電源ボタンは対象外)
  - Extra のダウンロードは終了時の中断フラグで止まる(`.tmp` は削除)。画面上の中断ボタンはなし

## 効果音(romfs/sound)
| 音 | 用途 |
|---|---|
| BTN_OK | 決定(メイン・Extra・設定の「アプリの更新/初期化/このアプリについて」) |
| SLOT_FRAME | 設定画面での決定 |
| CANCEL_S | 戻る |
| CURSOR | カーソル移動 |
| DECIDE_TITLE | CTGP-7を起動 |
| DIALOG_OPEN / DIALOG_CLOSE | 確認・ノートなどのウィンドウ |
| ATTENTION_POPUP | CTGP-7 未検出・Wi-Fi・バッテリー通知の表示時 |
| WIFI_CONNECT_OK | アプリ/パック更新通知の表示時 |
| BUSY_SYMBOL | 更新チェック・アップデート・Extra DL 中に連続ループ |
| SCROLL_LIST_STOP | 画面を開いて項目がスライドするとき |
| CHAT_DECIDE | アプリ更新の完了時、Extra のダウンロード完了時 |
| RC_PAUSE_OFF | ウィンドウの肯定側(OK・はい・更新する) |
| RC_PAUSE_ON | ウィンドウの否定側(B・いいえ・あとで) |
| RACE_READY_ENGINE_IDLING | 「CTGP-7を起動しますか?」の表示中にループ |
| RACE_GO_ENGINE | 起動の「はい」で DECIDE_TITLE と同時に再生(約1秒で起動し途中で切れる) |
| SLOT_ROLLING | シークバーを手で動かしている間(後半をループ、`ROLL_LOOP_START/END`) |
| SLOT_ROLLPASS | シークバーが端に届いたとき |
| WIFI_MII_CUPSULE_MII_MOVE | BGM(鳴り終わってから3秒あけてループ) |

## SDカード上のファイル
- 設定: `sdmc:/3ds/MoreCTGP7Manager/config.ini`(旧 `sdmc:/3ds/CharSwitcher` は自動で移動)
- パックバージョン: `sdmc:/3ds/MoreCTGP7Manager/morever.ini`
- 音を鳴らすには `sdmc:/3ds/dspfirm.cdc` が必要

## GitHub 側
- `version/appver.ini`(アプリ最新版)、`version/morever.ini`(パック最新版)
- `extra/` にチート txt
- リリース: 毎回新しいリリース(プレリリースにしない)に同名で `MoreCTGP7Manager.3dsx` / `.cia` を添付
- `version/changelog.ini`(お知らせの本文、毎回上書き)
- `extra/3gx/MK7_Cheat.3gx`(アップロード済み)
- 手順: ビルド → 新リリース公開 → changelog.ini → 最後に appver.ini を更新
- 同じバージョン番号のままファイル差し替えは避ける(既に更新した人に届かない)

## ビルド(Windows / devkitPro)
- 場所: `3ds-examples-20240917/templates/application`
- 追加ライブラリ: `pacman -S 3ds-curl 3ds-mbedtls 3ds-zlib`
- Makefile:
  ```
  LIBS     := -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lcitro2d -lcitro3d -lctru -lm
  LIBDIRS  := $(PORTLIBS) $(CTRULIB)
  GRAPHICS := gfx
  ROMFS    := romfs
  GFXBUILD := $(ROMFS)/gfx
  ```
- `romfs/cacert.pem`(https://curl.se/ca/cacert.pem)が必要
- `make clean` → `make`

## Claude Code の環境でのビルド(1.2.8 から)
- devkitPro 公式の Docker イメージ `devkitpro/devkitarm` で `make`(フォルダ名を `application` にして TARGET を合わせる)
- リポジトリの `makerom.exe` / `bannertool.exe` は Wine で実行
- Wine 経由だと bannertool の `-s` / `-l` の全角「！」が化ける(「Moreo<」のようになる)。
  `python3 tools/fix_smdh.py icon.bin "More！CTGP-7 Manager" "Character folder manager for More！CTGP-7" "yk / 3DSProject Team"` で直してから makerom に渡す
  (直した icon.bin は 1.2.7 のものとバイト単位で一致)
- GitHub のリリース作成はこちらのツールではできないので、ユーザーが行う

## CIA 作成
```
.\bannertool.exe makebanner -i banner.png -a banner.wav -o banner.bin
.\bannertool.exe makesmdh -s "More！CTGP-7 Manager" -l "Character folder manager for More！CTGP-7" -p "yk / 3DSProject Team" -i icon.png -o icon.bin
.\makerom.exe -f cia -o MoreCTGP7Manager.cia -elf application.elf -rsf app.rsf -icon icon.bin -banner banner.bin -exefslogo -target t -major 1 -minor 2 -micro 9
```
- `app.rsf` には DSP メモリ(IORegisterMapping)・am:net・soc:U・mcu::HWC などの権限を設定済み
- バージョンは `main.c` の `APP_VERSION`、makerom の -major/-minor/-micro、appver.ini の3か所を揃える
- UniqueId: 0xF7C71(タイトルID 000400000F7C7100)

## 主な調整用の値(main.c)
| 値 | 内容 |
|---|---|
| APP_VERSION | アプリバージョン |
| CTGP7_TITLE_ID | CTGP-7 Launcher のタイトルID |
| TAPE_SPEED | ゴールテープのスクロール速度 |
| CYCLE_MIN_MS / CYCLE_MAX_MS | 走り抜け演出の1フェーズの長さ(30秒〜2分30秒) |
| KART_SPEED / BOWSER_SPEED / SHELL_SPEED / LAKITU_SPEED | マリオ / クッパ(マリオの2倍)/ 甲羅 / ジュゲム(クッパの1.5倍)の速さ |
| LAKITU_SCALE / LAKITU_Y / LAKITU_BOB | ジュゲムの大きさ / 高さ / 揺れ幅 |
| P_SHELL_HIT / P_SIDE / P_FIRE | 甲羅ヒット / 並走 / クッパ発射 の場面の確率 |
| BGM_GAP_MS | BGM のループ間隔 |
| LIGHT_MS | 有効ボタンの光が1周する時間 |
| SLIDE_MS / SLIDE_STAGGER | 項目のスライド時間 / 上から順にずらす間隔 |
| hidSetRepeatParameters(24, 6) | 十字キー長押しの連続移動 |

## 確認済み事項
- アプリ内アップデートは .3dsx・CIA ともに実機で成功(1.2.1→1.2.2)
- 起動時のカーソルは有効なフォルダのボタンに合う(メイン画面に戻ったときのリセットは不要)

## 注意点・補足
- ボタンの沈み込み(btn_pressed): タッチを始めた位置と今の位置が両方そのボタン上のときだけ。ウィンドウが開いている間は一番手前のウィンドウ(g_top_layer = VIS_NOW、描画中は g_layer)のボタンだけが沈む
1. **ゴールテープの位置は上画面の背景画像専用**
   `TOP_TAPE_Y0/Y1`(帯の行)、`TAPE_BORDER`(枠の幅 3px)、`TAPE_PERIOD`(柄の周期 64px)は
   現在の `InfoBackground.png`(300x200)に合わせた値。背景を差し替えたら測り直すこと。
   下画面の背景 `Upd.png` は MK7 の画像を上画面の色味に合わせた 320x240 で、テープは無い。
2. **`cacert.pem` は zip に含まれていない**
   更新確認・DL が急にすべて失敗するようになったら証明書の期限切れを疑い、
   https://curl.se/ca/cacert.pem の最新版で `romfs/cacert.pem` を差し替える。
3. **バッテリー%の取得**
   CIA 版は `app.rsf` の `mcu::HWC` 権限で正確な%を取得。.3dsx 版は環境によって取得できず、
   PTMU の5段階からの概算(「~80%」表示)になる。
4. **ダークモードの仕組み**
   背景2枚(info_bg / upd_bg)とマリオ・クッパ・ジュゲム・甲羅・ウィンドウ・手の画像を、メモリ上のテクスチャで直接色反転している
   (`set_bg_dark()` → `invert_sheet()`)。新しい画像を反転対象にする場合はここに追加する。
   ボタン・ロゴは反転しない。甲羅は読み込み直後に一度反転し、ライトで反転・ダークで元の色になる。
   ジュゲムの雲は `lakitu.png` の時点で白黒を反転してあり、ダークモードで白い雲になる。
5. **素材の権利**
   マリオ・クッパ・甲羅の画像と効果音は任天堂の素材。GitHub でソースや素材を公開する際は扱いに注意。
6. **その他の仕様メモ**
   - Wi-Fi 未接続の通知は起動から約0.75秒後(45フレーム目)に判定
   - 走り抜け演出は1フェーズに「マリオ1・クッパ1・甲羅3・ジュゲム1」。ジュゲムのみ他と重なってよく、左右どちらからも50%で出現。場面: 通常 / 甲羅ヒット / 並走 / クッパ発射
   - 走り抜け演出の画像: kart.png(マリオ、0.8倍)、bowser.png(クッパ、0.7倍、4px奥のレーン)、shell.png(26x22、回転・揺れなし)
   - 有効ボタンの緑は `blueMask.png` / `selBlueMask.png`(ボタン画像の青い部分のマスク)を緑に着色して重ねている
   - カーソル枠の黄⇔オレンジのアニメは `selFrame.png`(選択枠の黄色部分のマスク)を着色して重ねている

## これまでにユーザーが決めたこと・やめたこと(やり直さないこと)
- バナナの演出は削除済み。粒子の上下の揺れも削除済み
- レインボーで赤を飛ばす処理、ランダムで赤を除く処理は削除済み(全範囲を使う)
- リセットボタンにゴミ箱は付けない(ゴミ箱は「設定を初期化」だけ)
- シークバーとゴミ箱は MK7 の切り出し画像を使う(自作の図形にしない)
- 「アプリを再起動」は cia のみ。.3dsx には出さない
- 3D バナー(CGFX)はここでは作れないと判断済み
- GitHub アイコン機能は削除済み
- タイトルロゴは上下に動かさない(光が横切るだけ)
- 上画面のバージョン表記は x390 のまま下に寄せる(右には寄せない)
- Extra のノートは1行目だけ「Txt/3gx形式の〜」、2行目は「独自のtxtが既に存在する場合、」のまま
- MK7 の 3gx は全地域には置かず、選んだ地域1か所だけ
- 3UP ムーンは主張しすぎない(小さく、光とキランは控えめ)。ベース画像はリアルな反射のない絵(ユーザー提供)

## 動作確認について
- ユーザーが Windows の devkitPro でビルドし、実機で確認する。こちらでは実機確認できない
- このリポジトリには 3DS 用ヘッダーが無いので、手元での文法チェックは簡易的なもの(libctru/citro2d/curl の関数・定数の未定義エラーは無視してよい)
- Universal Updater への登録申請は返事待ち(1〜2週間待って、なければ GitHub Issue か Discord)

## このパッケージに入っていないもの(ユーザー側にある)
- `Makefile`(devkitPro の templates/application のものに上記の設定を追加)
- `romfs/cacert.pem`
- `bannertool.exe` / `makerom.exe`
- ビルドで作られる `romfs/gfx/*.t3x`
