# CLAUDE.md — More！CTGP-7 Manager

3DS Homebrew アプリ「More！CTGP-7 Manager」(作者: yk / 3DSProject Team)。ソースは `source/main.c` の1ファイル(C / libctru + citro2d + libcurl)。
仕様の詳細は `HANDOFF.md`、これまでのやり取りは `docs/conversation_log.md` を参照。

## ユーザーとのやり取りのルール(必ず守る)
- 返答は日本語。専門用語はなるべくかみくだく
- 共有するファイルは **変更を加えたファイルのみ**。**2つ以上ならzipにまとめて** 共有する
- チェンジログの txt は共有しなくてよい
- **アプリのバージョン(`APP_VERSION`)はユーザーから「変えてください」と言われたときだけ変更する**
  - 現在: リリース済み 1.2.7、コードは `Ver.1.2.8`
  - ユーザーが「〇〇としてリリースしました、次回から X」と言ったら `APP_VERSION` を X にする
- `HANDOFF.md` は毎回は共有しない。作業ごとに内部で更新しておき、「HANDOFFをください」と言われたときに渡す
- 画像の見た目に関わる変更は、できるだけプレビュー画像(PNG)を作って見せる(「プレビューもよろしく」と言われることが多い)
- ユーザーが実機で確認する。こちらは実機で試せないので、未確認であることを正直に伝える
- 以前に却下・削除された演出は戻さない(`HANDOFF.md` の「やり直さないこと」)

## ビルド
- ユーザーの環境: Windows / devkitPro、`3ds-examples-20240917/templates/application`
- Makefile・cacert.pem・bannertool・makerom はユーザー側にある
- 画像を追加したら `gfx/` に .png と .t3s を置く(`--atlas -f rgba8888 -z auto`)。ビルドで `romfs/gfx/*.t3x` になる
- CIA の makerom は `-major 1 -minor 2 -micro <版>` を APP_VERSION に合わせる
- リリース手順: ビルド → GitHub に新しいリリース(プレリリースにしない、`MoreCTGP7Manager.3dsx` / `.cia` を添付)→ `version/changelog.ini` を更新 → 最後に `version/appver.ini` を更新

## コードの注意
- 文字列は `Strings` 構造体(`STR_EN` / `STR_JA`)に順番で並ぶ。フィールドを足すときは構造体と両方のテーブルの同じ位置に入れる(順番がずれると表示が入れ替わる)。1か所だけの短い文字列は `S == &STR_JA ? "…" : "…"` で済ませている
- 関数は使う前に定義する(前方宣言がないと implicit declaration エラー)
- 設定項目の番号を変えると `set_item` / `ok_plain` などの対応がずれるので注意
- ダークモードで色反転する画像は `set_bg_dark()` に追加する
- 効果音は `romfs/sound/*.wav`、LED は `MCUHWC_WriteRegister(0x2D)`
