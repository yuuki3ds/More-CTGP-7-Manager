// More！CTGP-7 Manager Ver.1.2.6 - character folder manager for More!CTGP-7
// Target: sdmc:/CTGP-7/MyStuff/Characters{1,2,3}
// The folder named "Characters" (no number) is the active one.
// No state file for folders: the active slot is detected from which folders exist.
//
// Top screen   : title / current slot / status / version
// Bottom screen: slot select + settings (buttons AND touch)
// Settings are saved to sdmc:/3ds/MoreCTGP7Manager/config.ini

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <curl/curl.h>
#include <sys/stat.h>
#include <time.h>

#define APP_VERSION "Ver.1.2.6"
#define APP_NAME_1  "More！CTGP-7"
#define APP_NAME_2  "Manager"          // used only if the logo image fails to load
#define APP_AUTHOR  "yk"
#define APP_TEAM    "3DSProject Team"
#define APP_REPO    "https://github.com/yuuki3ds/More-CTGP-7-Manager"

#define BASE        "sdmc:/CTGP-7/MyStuff/"
#define ACTIVE_NAME BASE "Characters"
#define SLOTS       3

#define CFG_DIR  "sdmc:/3ds/MoreCTGP7Manager"
#define OLD_CFG_DIR "sdmc:/3ds/CharSwitcher"   // previous folder; moved automatically
#define CFG_PATH CFG_DIR "/config.ini"

// URL of the pack version file on GitHub (raw). Override with
// pack_url=... in config.ini. (A URL containing "OWNER" is treated as unset.)
#define PACK_URL_DEFAULT "https://raw.githubusercontent.com/yuuki3ds/More-CTGP-7-Manager/refs/heads/main/version/morever.ini"

// URL of the app version file on GitHub (raw). Display only for now.
#define APP_URL_DEFAULT "https://raw.githubusercontent.com/yuuki3ds/More-CTGP-7-Manager/refs/heads/main/version/appver.ini"

// Release files used by the in-app updater
// "releases/latest/download/<file>" always points to the newest (non-pre-release) release
#define REL_URL_3DSX "https://github.com/yuuki3ds/More-CTGP-7-Manager/releases/latest/download/MoreCTGP7Manager.3dsx"
#define REL_URL_CIA  "https://github.com/yuuki3ds/More-CTGP-7-Manager/releases/latest/download/MoreCTGP7Manager.cia"
#define DEFAULT_3DSX_PATH "sdmc:/3ds/MoreCTGP7Manager.3dsx"
#define ALT_3DSX_PATH     "sdmc:/3ds/MoreCTGP7Manager/MoreCTGP7Manager.3dsx"
#define SELF_TITLE_ID     0x000400000F7C7100ULL   // installed .cia (UniqueId 0xF7C71)

// Extra menu: cheat files (raw files in the "extra" folder of the repository)
#define EXTRA_RAW "https://raw.githubusercontent.com/yuuki3ds/More-CTGP-7-Manager/refs/heads/main/extra/"
#define CHEATS_DIR "sdmc:/cheats"

// ------------------------------------------------------------ strings

typedef struct {
    const char *title;
    const char *current;       // "%s" = folder name
    const char *current_none;
    const char *tag_active;
    const char *help;
    const char *already;       // "%s"
    const char *done;          // "%s"
    const char *fail_step1;    // "%s"
    const char *fail_step2;    // "%s"
    const char *err_no_active;
    const char *err_ambiguous;
    const char *err_unexpected;
    const char *err_footer;

    const char *select_slot;
    const char *settings;
    const char *settings_title;
    const char *set_language;
    const char *set_confirm;
    const char *set_exit;
    const char *set_reset;
    const char *back;
    const char *on;
    const char *off;
    const char *lang_auto;
    const char *confirm_msg;   // "%s"
    const char *yes;
    const char *no;
    const char *about;
    const char *version_label;
    const char *author_label;
    const char *launch;
    const char *launch_fail;
    const char *pack_label;
    const char *set_check;
    const char *update_msg;
    const char *current_label;
    const char *latest_label;
    const char *ok;
    const char *set_appupd;
    const char *set_restart;
    const char *appupd_title;
    const char *check_btn;
    const char *checking;
    const char *app_new;
    const char *app_latest;
    const char *app_fail;
    const char *set_dark;
    const char *set_sfx;
    const char *set_bgm;
    const char *set_led;
    const char *app_dlg_msg;
    const char *btn_update;
    const char *btn_later;
    const char *install_btn;
    const char *downloading;
    const char *installing;
    const char *done_cia;
    const char *done_3dsx;
    const char *restart;
    const char *quit;
    const char *fail_net;
    const char *fail_file;
    const char *fail_bad;
    const char *fail_install;
    const char *dl_btn;
    const char *ex_downloading;   // "%d/%d"
    const char *ex_done;          // "%d"
    const char *ex_fail;
    const char *ver_unknown;
    const char *regions_label;
    const char *set_clock;
    const char *h24;
    const char *h12;
    const char *am;
    const char *pm;
    const char *notice_wifi;
    const char *notice_batt;
    const char *notice_batt_crit;
    const char *upd_both;      // "%s" = other format, it is installed
    const char *inst_both;     // "%s" = other format, it is not installed
    const char *ai_note;       // two lines
    const char *confirm_reset;
    const char *confirm_launch;
    const char *confirm_restart;
    const char *notice_ctgp;
    const char *rename_tip;
    const char *custom_title;
    const char *rename_btn;
    const char *save_btn;
    const char *reset_btn;
    const char *saved_msg;
    const char *name_bad;
    const char *name_dup;
    const char *name_fail;
    const char *confirm_fold_save;
    const char *confirm_fold_reset;
    const char *rename_hint;
    const char *extra_tip;
    const char *ex_note;           // three lines
    const char *name_rules_title;
    const char *name_rules;        // two lines
    const char *mario_talk;
    const char *set_bgcolor;
    const char *bg_default;
    const char *random_btn;
    const char *ai_title;
    const char *rainbow_btn;
    const char *rainbow_on;
    const char *color_tip;
    const char *color_title;     // "%s" = folder name
} Strings;

static const Strings STR_EN = {
    "More！CTGP-7 Manager",
    "Active: %s",
    "Active: none",
    "ACTIVE",
    "D-pad/Touch: select   A: OK   B: back   START: exit",
    "%s is already active.",
    "Switched. %s is now active.",
    "Failed: Characters -> %s",
    "Failed: %s -> Characters\nRolled back.",
    "\"Characters\" was not found, and one of the\nnumbered folders is missing.\nPlease check the folders manually.",
    "\"Characters\" and Characters1-3 all exist.\nCannot tell which one is active.",
    "Unexpected folder layout.\nExpected \"Characters\" + two numbered folders,\nor three numbered folders.",
    "Press START to exit.",

    "Select Slot",
    "Settings",
    "Settings",
    "Language",
    "Confirm before switching",
    "Exit after switching",
    "Reset settings",
    "Back",
    "ON",
    "OFF",
    "Auto",
    "Switch to %s?",
    "Yes",
    "No",
    "About this app",
    "Version",
    "Created by",
    "Launch CTGP-7",
    "CTGP-7 was not found.\nPlease install the CTGP-7 Launcher.",
    "Pack version",
    "Check for updates at startup",
    "A pack update is available.",
    "Current",
    "Latest",
    "OK",
    "App update",
    "Restart app",
    "App Update",
    "Check for updates",
    "Checking...",
    "A new update is available.",
    "The latest version is installed.",
    "Could not check for updates.",
    "Dark mode",
    "Sound effects",
    "BGM",
    "Notification light",
    "An app update is available.",
    "Update",
    "Later",
    "Download and install",
    "Downloading...",
    "Installing...",
    "Update complete.\nPlease restart the app.",
    "Update complete.\nExit and start the app again.",
    "Restart",
    "Exit",
    "Download failed.",
    "Could not write the file.",
    "The downloaded file is invalid.",
    "Installation failed.",
    "Download",
    "Downloading... %d/%d",
    "Done. Saved %d file(s) to sdmc:/cheats",
    "Download failed.",
    "Ver: ---",
    "Supported regions",
    "Clock format",
    "24-hour",
    "12-hour",
    "AM",
    "PM",
    "Wi-Fi is not connected.",
    "The battery is running low.",
    "The battery is low.\nPlease connect the charger.",
    "Also update %s",
    "Also install the %s version",
    "This project contains AI-generated content.\nClaude was used in its development.",
    "Reset all settings?",
    "Launch CTGP-7?",
    "Restart the app?",
    "CTGP-7 is not installed.",
    "Rename folders",
    "Custom Folders",
    "Rename",
    "Save changes",
    "Reset",
    "Saved.",
    "That name cannot be used.",
    "The name is already in use.",
    "Could not rename the folders.",
    "Save the changes?",
    "Reset all folder names?",
    "Enter a new folder name",
    "Extra",
    "This is a cheat code pack in txt format.\nIf you already have your own txt file,\nit will be overwritten.",
    "Characters not allowed in folder names",
    "Spaces at the start or end, a dot at the end,\nand the name \"Characters\" cannot be used.",
    "Active",
    "Customize background color",
    "Default color",
    "Random",
    "Notice",
    "Rainbow",
    "Rainbow is on",
    "Change management color",
    "Color of %s",
};

static const Strings STR_JA = {
    "More！CTGP-7 Manager",
    "現在の有効フォルダ: %s",
    "現在の有効フォルダ: なし",
    "有効",
    "十字キー/タッチ: 選択   A: 決定   B: 戻る   START: 終了",
    "%s は既に有効です。",
    "切り替え完了: %s が有効です。",
    "失敗: Characters → %s",
    "失敗: %s → Characters\n元に戻しました。",
    "Characters が見つからず、番号付きフォルダも\n1つ欠けています。手動で確認してください。",
    "Characters と Characters1〜3 が全て存在します。\nどれが有効か判定できません。",
    "フォルダ構成が想定外です。\n(Characters + 番号付き2つ、または番号付き3つ)",
    "STARTで終了します。",

    "スロット選択",
    "設定",
    "設定",
    "言語",
    "切り替え前に確認",
    "切り替え後にアプリを終了",
    "設定を初期化",
    "戻る",
    "ON",
    "OFF",
    "自動",
    "%s に切り替えますか?",
    "はい",
    "いいえ",
    "このアプリについて",
    "バージョン",
    "作成者",
    "CTGP-7を起動",
    "CTGP-7が見つかりません。\nCTGP-7 Launcherをインストールしてください。",
    "パックバージョン",
    "起動時に更新を確認",
    "パックのアップデートがあります。",
    "現在",
    "最新",
    "OK",
    "アプリの更新",
    "アプリを再起動",
    "アプリの更新",
    "更新のチェック",
    "確認中...",
    "新しいアップデートがあります。",
    "最新バージョンがインストールされています。",
    "更新を確認できませんでした。",
    "ダークモード",
    "効果音",
    "BGM",
    "お知らせランプ",
    "アプリのアップデートがあります。",
    "更新する",
    "あとで",
    "ダウンロードしてインストール",
    "ダウンロード中...",
    "インストール中...",
    "更新が完了しました。\nアプリを再起動してください。",
    "更新が完了しました。\nアプリを終了して起動し直してください。",
    "再起動",
    "終了",
    "ダウンロードに失敗しました。",
    "ファイルを書き込めませんでした。",
    "ダウンロードしたファイルが正しくありません。",
    "インストールに失敗しました。",
    "ダウンロード",
    "ダウンロード中... %d/%d",
    "完了: %d個のファイルを sdmc:/cheats に配置しました",
    "ダウンロードに失敗しました。",
    "Ver: ---",
    "対応リージョン",
    "時刻表示",
    "24時間",
    "12時間",
    "午前",
    "午後",
    "WiFiが接続されていません。",
    "バッテリーが少なくなっています。",
    "バッテリーが減っています。\n充電器を接続してください。",
    "%sも一緒に更新",
    "%s版も一緒にインストール",
    "このプロジェクトにはAI生成物が含まれます。\n開発にはClaudeが使用されています。",
    "設定を初期化しますか?",
    "CTGP-7を起動しますか?",
    "アプリを再起動しますか?",
    "CTGP-7がインストールされていません。",
    "フォルダ名を変更",
    "カスタムフォルダ",
    "名前を変更",
    "変更を保存",
    "リセット",
    "保存しました。",
    "その名前は使えません。",
    "その名前は既に使われています。",
    "フォルダの名前を変更できませんでした。",
    "変更を保存しますか?",
    "フォルダ名をリセットしますか?",
    "新しいフォルダ名を入力",
    "おまけ",
    "Txt形式のチートコードパックです。\n独自のtxtが既に存在する場合、\n上書きされますのでご注意ください。",
    "フォルダ名に使えない文字",
    "先頭と末尾の空白、末尾のドット、\n「Characters」という名前も使えません。",
    "有効中",
    "背景の色をカスタマイズ",
    "既定の色",
    "ランダム",
    "注意",
    "レインボー",
    "レインボー中",
    "管理色を変更",
    "%s の管理色",
};

// ------------------------------------------------------------ config

typedef struct {
    int lang;        // 0 = auto, 1 = Japanese, 2 = English
    int confirm;     // 0/1
    int exit_after;  // 0/1
    int check_update;             // check pack version on GitHub at startup (0/1)
    char pack_url[200];
    int dark;                     // dark mode (0/1)
    int sfx;                      // sound effects (0/1)
    int bgm;                      // background music (0/1)
    int led;                      // notification LED (0/1)
    int clock24;                  // 1 = 24-hour clock, 0 = 12-hour
    int bg_hue;                   // background color: hue 0-359, -1 = default
    int bg_rainbow;               // background color loops through all hues (0/1)
} Config;

static void config_defaults(Config *c) {
    c->lang = 0;
    c->confirm = 1;
    c->exit_after = 0;
    c->check_update = 1;
    c->dark = 0;
    c->sfx = 1;
    c->bgm = 1;
    c->led = 1;
    c->clock24 = 1;
    c->bg_hue = -1;
    c->bg_rainbow = 0;
    snprintf(c->pack_url, sizeof c->pack_url, "%s", PACK_URL_DEFAULT);
}

// First launch after the rename: move the old folder (config.ini, morever.ini, ...) over.
static void migrate_cfg_dir(void) {
    struct stat st;
    if (stat(CFG_DIR, &st) == 0) return;                    // new folder already exists
    if (stat(OLD_CFG_DIR, &st) != 0 || !S_ISDIR(st.st_mode)) return;
    rename(OLD_CFG_DIR, CFG_DIR);
}

static void config_load(Config *c) {
    config_defaults(c);
    FILE *f = fopen(CFG_PATH, "r");
    if (!f) return;
    char key[32], val[200];
    while (fscanf(f, " %31[^=]=%199s", key, val) == 2) {
        if (!strcmp(key, "ctgp7_id")) continue;   // no longer used (fixed title ID)
        if (!strcmp(key, "pack_url")) {
            if (!strstr(val, "More-CTGP-7-Character-Switcher"))   // old repo name: use new default
                snprintf(c->pack_url, sizeof c->pack_url, "%s", val);
            continue;
        }
        int v = atoi(val);
        if (!strcmp(key, "lang") && v >= 0 && v <= 2) c->lang = v;
        else if (!strcmp(key, "confirm")) c->confirm = v ? 1 : 0;
        else if (!strcmp(key, "exit_after")) c->exit_after = v ? 1 : 0;
        else if (!strcmp(key, "check_update")) c->check_update = v ? 1 : 0;
        else if (!strcmp(key, "dark")) c->dark = v ? 1 : 0;
        else if (!strcmp(key, "sfx")) c->sfx = v ? 1 : 0;
        else if (!strcmp(key, "bgm")) c->bgm = v ? 1 : 0;
        else if (!strcmp(key, "led")) c->led = v ? 1 : 0;
        else if (!strcmp(key, "clock24")) c->clock24 = v ? 1 : 0;
        else if (!strcmp(key, "bg_hue") && v >= -1 && v < 360) c->bg_hue = v;
        else if (!strcmp(key, "bg_rainbow")) c->bg_rainbow = v ? 1 : 0;
    }
    fclose(f);
}

static void config_save(const Config *c) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(CFG_PATH, "w");
    if (!f) return;
    fprintf(f, "lang=%d\nconfirm=%d\nexit_after=%d\ncheck_update=%d\ndark=%d\nsfx=%d\nbgm=%d\nclock24=%d\nbg_hue=%d\nbg_rainbow=%d\nled=%d\n",
            c->lang, c->confirm, c->exit_after, c->check_update, c->dark, c->sfx, c->bgm, c->clock24, c->bg_hue, c->bg_rainbow, c->led);
    if (strcmp(c->pack_url, PACK_URL_DEFAULT) != 0)     // only persist a custom URL
        fprintf(f, "pack_url=%s\n", c->pack_url);
    fclose(f);
}

// ------------------------------------------------------------ custom folder names
// g_names[i] is the real folder name of slot i+1 on the SD card while it is on standby. The
// active slot is always called "Characters"; it gets its own name back when it is switched out.
// One name per line in names.txt.

#define NAMES_PATH CFG_DIR "/names.txt"
#define NAME_CHARS 16                 // longest name, in characters
#define NAME_BUF   (NAME_CHARS * 4 + 1)

static char g_names[SLOTS][NAME_BUF];

static void names_default(char names[SLOTS][NAME_BUF]) {
    for (int i = 0; i < SLOTS; i++) snprintf(names[i], NAME_BUF, "Characters%d", i + 1);
}

static void names_load(void) {
    names_default(g_names);
    FILE *f = fopen(NAMES_PATH, "r");
    if (!f) return;
    char line[NAME_BUF + 8];
    for (int i = 0; i < SLOTS && fgets(line, sizeof line, f); i++) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0]) snprintf(g_names[i], NAME_BUF, "%s", line);
    }
    fclose(f);
}

// Management color of each folder (hue 0-359), one per line in colors.txt.
#define COLORS_PATH CFG_DIR "/colors.txt"
static const int FOLD_HUE_DEF[SLOTS] = { 0, 48, 128 };   // red, yellow, green
static int g_fold_hue[SLOTS] = { 0, 48, 128 };

static void colors_load(void) {
    FILE *f = fopen(COLORS_PATH, "r");
    if (!f) return;
    for (int i = 0; i < SLOTS; i++) {
        int v;
        if (fscanf(f, "%d", &v) == 1 && v >= 0 && v < 360) g_fold_hue[i] = v;
    }
    fclose(f);
}

static void colors_save(void) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(COLORS_PATH, "w");
    if (!f) return;
    for (int i = 0; i < SLOTS; i++) fprintf(f, "%d\n", g_fold_hue[i]);
    fclose(f);
}

static void names_save(void) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(NAMES_PATH, "w");
    if (!f) return;
    for (int i = 0; i < SLOTS; i++) fprintf(f, "%s\n", g_names[i]);
    fclose(f);
}

// ------------------------------------------------------------ pack version

#define PACKVER_PATH CFG_DIR "/morever.ini"

static char *trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n'))
        s[--n] = '\0';
    return s;
}

static bool has_ver_key(const char *key) {
    // case-insensitive "ver" anywhere in the key (version, Version, morever, ...)
    for (; *key; key++) {
        if ((key[0] | 0x20) == 'v' && key[1] && (key[1] | 0x20) == 'e' &&
            key[2] && (key[2] | 0x20) == 'r')
            return true;
    }
    return false;
}

// Reads the pack version from morever.ini.
// Accepted: "version=1.2.3" (any key containing "ver"), or just a bare "1.2.3" line.
// Lines starting with ; # [ are ignored. Leaves out[0] == '\0' if unavailable.
static void parse_version_text(const char *text, char *out, size_t n) {
    out[0] = '\0';
    char line[160], first[64] = "";
    bool first_line = true;
    const char *cur = text;
    while (*cur) {
        size_t len = strcspn(cur, "\n");
        size_t cp = len < sizeof line - 1 ? len : sizeof line - 1;
        memcpy(line, cur, cp);
        line[cp] = '\0';
        cur += len;
        if (*cur == '\n') cur++;
        char *p = line;
        if (first_line) {           // skip UTF-8 BOM
            first_line = false;
            if ((u8)p[0] == 0xEF && (u8)p[1] == 0xBB && (u8)p[2] == 0xBF) p += 3;
        }
        p = trim(p);
        if (!*p || *p == ';' || *p == '#' || *p == '[') continue;

        char *eq = strchr(p, '=');
        if (eq) {
            *eq = '\0';
            char *key = trim(p);
            char *val = trim(eq + 1);
            if (!*val) continue;
            if (has_ver_key(key)) {
                snprintf(out, n, "%s", val);
                return;
            }
            if (!first[0]) snprintf(first, sizeof first, "%s", val);
        } else if (!first[0]) {
            snprintf(first, sizeof first, "%s", p);
        }
    }
    snprintf(out, n, "%s", first);
}

static void load_pack_version(char *out, size_t n) {
    out[0] = '\0';
    FILE *f = fopen(PACKVER_PATH, "r");
    if (!f) return;
    char text[512];
    size_t got = fread(text, 1, sizeof text - 1, f);
    fclose(f);
    text[got] = '\0';
    parse_version_text(text, out, n);
}

// ------------------------------------------------------------ pack update check (GitHub)

static bool has_digit(const char *s) {
    for (; *s; s++) if (isdigit((u8)*s)) return true;
    return false;
}

static int next_num(const char **s) {
    while (**s && !isdigit((u8)**s)) (*s)++;
    if (!**s) return -1;
    long v = 0;
    while (isdigit((u8)**s)) {
        v = v * 10 + (**s - '0');
        if (v > 100000000L) v = 100000000L;
        (*s)++;
    }
    return (int)v;
}

// >0: a is newer than b, <0: older, 0: equal  (compares 1.9 < 1.10 numerically)
static int version_cmp(const char *a, const char *b) {
    for (;;) {
        int x = next_num(&a), y = next_num(&b);
        if (x < 0 && y < 0) return 0;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x != y) return x > y ? 1 : -1;
    }
}

enum { CHK_IDLE = 0, CHK_RUNNING, CHK_DONE, CHK_FAIL };

typedef struct {
    char url[200];
    char remote[64];
    volatile int state;
    volatile bool quit;
    bool raw;            // true: result = first line of the file as-is
} CheckCtx;

static CheckCtx g_chk;   // pack check (startup)
static CheckCtx g_app;   // app check (manual)

// in-app updater state (logic further below)
enum { UPD_IDLE = 0, UPD_DOWNLOADING, UPD_INSTALLING, UPD_DONE, UPD_FAIL };
enum { UFAIL_NONE = 0, UFAIL_NET, UFAIL_FILE, UFAIL_BAD, UFAIL_INSTALL };

#define UPD_TMP  CFG_DIR "/update.tmp"      // format that is running
#define UPD_TMP2 CFG_DIR "/update2.tmp"     // other format ("update both")

typedef struct {
    bool is_cia;
    char target[256];          // .3dsx path to replace
    int other;                 // other format: -1 cannot use it, 0 not installed, 1 installed
    bool both;                 // this update includes the other format
    bool only_other;           // running format is already the latest: leave it alone
    bool fail_other;           // the failed install was the other format
    volatile int step;         // file in progress (1 or 2) when both is set
    volatile int state, fail;
    volatile u64 now, total;   // progress (bytes)
    volatile u32 code;         // error code of the failed step (shown to the user)
    volatile int stage;        // failed step: 1 open, 2 AM init, 3 start, 4 write, 5 finish, 6 rename
} UpdCtx;

static UpdCtx g_upd;
static u32 *g_soc_buf = NULL;
static bool g_net_ready = false;
static bool g_romfs = false;

typedef struct { char buf[256]; size_t len; } Resp;

static size_t write_cb(char *p, size_t size, size_t nmemb, void *user) {
    Resp *r = (Resp *)user;
    size_t total = size * nmemb;
    size_t room = sizeof r->buf - 1 - r->len;
    size_t cp = total < room ? total : room;
    memcpy(r->buf + r->len, p, cp);
    r->len += cp;
    return total;                       // never abort on oversize; extra is dropped
}

static int xfer_cb(void *user, curl_off_t a, curl_off_t b, curl_off_t c, curl_off_t d) {
    (void)a; (void)b; (void)c; (void)d;
    return ((CheckCtx *)user)->quit ? 1 : 0;   // non-zero aborts the transfer
}

static const char *find_cacert(void) {
    static const char *paths[] = { "romfs:/cacert.pem", CFG_DIR "/cacert.pem" };
    for (int i = 0; i < 2; i++) {
        FILE *f = fopen(paths[i], "rb");
        if (f) { fclose(f); return paths[i]; }
    }
    return NULL;
}

static void check_thread(void *arg) {
    CheckCtx *ctx = (CheckCtx *)arg;
    const char *ca = find_cacert();
    CURL *c = ca ? curl_easy_init() : NULL;
    if (!c) { ctx->state = CHK_FAIL; return; }

    Resp r = { {0}, 0 };
    curl_easy_setopt(c, CURLOPT_URL, ctx->url);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 6L);
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 12L);
    curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(c, CURLOPT_USERAGENT, "MoreCTGP7Manager/" APP_VERSION);
    curl_easy_setopt(c, CURLOPT_CAINFO, ca);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &r);
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, xfer_cb);
    curl_easy_setopt(c, CURLOPT_XFERINFODATA, ctx);
    if (ctx->raw) curl_easy_setopt(c, CURLOPT_RANGE, "0-511");   // only the beginning is needed

    CURLcode rc = curl_easy_perform(c);
    long code = 0;
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(c);

    if (rc == CURLE_OK && (code == 200 || (ctx->raw && code == 206)) && r.len > 0) {
        r.buf[r.len] = '\0';
        if (ctx->raw) {                              // first line, BOM / spaces trimmed
            char *p = r.buf;
            if ((u8)p[0] == 0xEF && (u8)p[1] == 0xBB && (u8)p[2] == 0xBF) p += 3;
            p[strcspn(p, "\r\n")] = '\0';
            p = trim(p);
            snprintf(ctx->remote, sizeof ctx->remote, "%s", p);
        } else {
            parse_version_text(r.buf, ctx->remote, sizeof ctx->remote);
        }
        ctx->state = ctx->remote[0] ? CHK_DONE : CHK_FAIL;
    } else {
        ctx->state = CHK_FAIL;
    }
}

static bool net_init(void) {
    if (g_net_ready) return true;
    g_soc_buf = (u32 *)memalign(0x1000, 0x100000);
    if (!g_soc_buf) return false;
    if (R_FAILED(socInit(g_soc_buf, 0x100000))) {
        free(g_soc_buf);
        g_soc_buf = NULL;
        return false;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    g_net_ready = true;
    return true;
}

// Starts a background check of `url` into ctx. Returns false if it could not be started.
static bool check_start(CheckCtx *ctx, const char *url) {
    if (ctx->state == CHK_RUNNING) return false;
    if (!net_init()) { ctx->state = CHK_FAIL; return false; }

    snprintf(ctx->url, sizeof ctx->url, "%s", url);
    ctx->remote[0] = '\0';
    // ctx->raw is set by the caller before starting
    ctx->quit = false;
    ctx->state = CHK_RUNNING;

    s32 prio = 0x30;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    prio -= 1;
    if (prio < 0x18) prio = 0x18;

    Thread t = threadCreate(check_thread, ctx, 128 * 1024, prio, -1, true);
    if (!t) { ctx->state = CHK_FAIL; return false; }
    return true;
}

static bool net_extra_busy(void);   // defined with the Extra menu
static void net_extra_quit(void);

static void net_stop(void) {
    if (!g_net_ready) return;
    g_chk.quit = true;
    g_app.quit = true;
    net_extra_quit();
    for (int i = 0; i < 40 && (g_chk.state == CHK_RUNNING || g_app.state == CHK_RUNNING || g_upd.state == UPD_DOWNLOADING || g_upd.state == UPD_INSTALLING || net_extra_busy()); i++)
        svcSleepThread(50000000LL);                    // wait up to ~2 s
    if (g_chk.state != CHK_RUNNING && g_app.state != CHK_RUNNING &&
        g_upd.state != UPD_DOWNLOADING && g_upd.state != UPD_INSTALLING && !net_extra_busy()) {   // otherwise just leak; we are exiting
        curl_global_cleanup();
        socExit();
        free(g_soc_buf);
    }
    g_net_ready = false;
}

// ------------------------------------------------------------ in-app updater
// Downloads the release file for the running format and installs it:
//   .3dsx -> replaces the running .3dsx file (argv[0] or DEFAULT_3DSX_PATH)
//   .cia  -> installs through the AM service, then the app can relaunch itself


static size_t upd_write_cb(char *p, size_t size, size_t nmemb, void *user) {
    return fwrite(p, size, nmemb, (FILE *)user) * size;
}

static int upd_xfer_cb(void *user, curl_off_t dltotal, curl_off_t dlnow, curl_off_t a, curl_off_t b) {
    (void)user; (void)a; (void)b;
    g_upd.total = (u64)dltotal;
    g_upd.now = (u64)dlnow;
    return 0;
}

static bool upd_download(const char *url, const char *tmp) {
    const char *ca = find_cacert();
    if (!ca) return false;
    FILE *f = fopen(tmp, "wb");
    if (!f) { g_upd.fail = UFAIL_FILE; return false; }
    setvbuf(f, NULL, _IOFBF, 64 * 1024);

    CURL *c = curl_easy_init();
    if (!c) { fclose(f); return false; }
    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);        // GitHub redirects to its file server
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_LIMIT, 1L);       // give up after 20 s without data
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(c, CURLOPT_USERAGENT, "MoreCTGP7Manager/" APP_VERSION);
    curl_easy_setopt(c, CURLOPT_CAINFO, ca);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(c, CURLOPT_BUFFERSIZE, 64L * 1024);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, upd_write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, upd_xfer_cb);

    CURLcode rc = curl_easy_perform(c);
    long code = 0;
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(c);
    if (fclose(f) != 0 && rc == CURLE_OK) { g_upd.fail = UFAIL_FILE; return false; }
    if (rc == CURLE_WRITE_ERROR) { g_upd.fail = UFAIL_FILE; return false; }
    return rc == CURLE_OK && code == 200;
}

// Checks the first bytes so we never install an HTML error page etc.
static bool upd_verify(const char *tmp, bool cia) {
    FILE *f = fopen(tmp, "rb");
    if (!f) return false;
    u8 h[4] = {0};
    size_t n = fread(h, 1, 4, f);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fclose(f);
    if (n != 4 || len < 4096) return false;
    if (cia) return h[0] == 0x20 && h[1] == 0x20 && h[2] == 0 && h[3] == 0;   // header size 0x2020
    return !memcmp(h, "3DSX", 4);
}

static bool upd_install_3dsx(const char *tmp) {
    char bak[264];
    snprintf(bak, sizeof bak, "%s.bak", g_upd.target);
    remove(bak);
    bool had_old = rename(g_upd.target, bak) == 0;
    if (rename(tmp, g_upd.target) != 0) {
        g_upd.stage = 6; g_upd.code = (u32)errno;
        if (had_old) rename(bak, g_upd.target);          // put the old one back
        return false;
    }
    if (had_old) remove(bak);
    return true;
}

// replace: the installed title is not running, so remove it first. AM refuses a .cia whose
// version is not newer than the installed one (reinstalling the same version would fail).
static bool upd_install_cia(const char *tmp, bool replace) {
    FILE *f = fopen(tmp, "rb");
    if (!f) { g_upd.stage = 1; g_upd.code = (u32)errno; return false; }
    fseek(f, 0, SEEK_END);
    u64 len = (u64)ftell(f);
    fseek(f, 0, SEEK_SET);
    g_upd.total = len;
    g_upd.now = 0;

    const size_t CH = 128 * 1024;
    u8 *buf = malloc(CH);
    Result rc = buf ? amInit() : -1;
    if (R_FAILED(rc)) { g_upd.stage = 2; g_upd.code = (u32)rc; free(buf); fclose(f); return false; }

    if (replace) {                        // results ignored: it may simply not be installed
        AM_DeleteTitle(MEDIATYPE_SD, SELF_TITLE_ID);
        AM_DeleteTicket(SELF_TITLE_ID);
    }

    Handle h = 0;
    rc = AM_StartCiaInstall(MEDIATYPE_SD, &h);
    bool ok = R_SUCCEEDED(rc);
    if (!ok) { g_upd.stage = 3; g_upd.code = (u32)rc; }
    u64 off = 0;
    while (ok && off < len) {
        size_t n = fread(buf, 1, CH, f);
        if (n == 0) { ok = false; g_upd.stage = 4; g_upd.code = 0; break; }
        u32 w = 0;
        rc = FSFILE_Write(h, &w, off, buf, n, 0);
        if (R_FAILED(rc) || w != n) { ok = false; g_upd.stage = 4; g_upd.code = (u32)rc; break; }
        off += n;
        g_upd.now = off;
    }
    if (ok) {
        rc = AM_FinishCiaInstall(h);
        ok = R_SUCCEEDED(rc);
        if (!ok) { g_upd.stage = 5; g_upd.code = (u32)rc; }
    } else if (h) {
        AM_CancelCIAInstall(h);
    }

    amExit();
    free(buf);
    fclose(f);
    return ok;
}

static void upd_abort(int fail) {
    g_upd.fail = fail;
    remove(UPD_TMP);
    remove(UPD_TMP2);
    g_upd.state = UPD_FAIL;
}

static bool upd_fetch(bool cia, const char *tmp) {
    g_upd.now = g_upd.total = 0;
    if (!upd_download(cia ? REL_URL_CIA : REL_URL_3DSX, tmp)) {
        upd_abort(g_upd.fail ? g_upd.fail : UFAIL_NET);
        return false;
    }
    if (!upd_verify(tmp, cia)) {
        upd_abort(UFAIL_BAD);
        return false;
    }
    return true;
}

// A .cia of this app is running only when the app itself is the .cia: it cannot be removed first.
static bool upd_install_file(bool cia, const char *tmp) {
    return cia ? upd_install_cia(tmp, !g_upd.is_cia) : upd_install_3dsx(tmp);
}

// Both files are downloaded and checked before anything is installed. The other format goes
// first: if it fails, the running format is still untouched and the update can be retried.
static void upd_thread(void *arg) {
    (void)arg;
    bool cia = g_upd.is_cia, both = g_upd.both, main_too = !g_upd.only_other;
    g_upd.fail = UFAIL_NONE;
    g_upd.fail_other = false;
    g_upd.step = 1;
    g_upd.state = UPD_DOWNLOADING;
    if (main_too && !upd_fetch(cia, UPD_TMP)) return;
    if (both) {
        if (main_too) g_upd.step = 2;
        if (!upd_fetch(!cia, UPD_TMP2)) return;
    }
    g_upd.state = UPD_INSTALLING;
    g_upd.stage = 0; g_upd.code = 0;
    // romfs keeps the running .3dsx file (or the installed title's content) open, which blocks
    // replacing it. Everything from romfs is already loaded, so close it during the install.
    bool had_romfs = g_romfs;
    if (had_romfs) { romfsExit(); g_romfs = false; }
    bool ok = true;
    g_upd.step = 1;
    if (both) {
        g_upd.fail_other = true;
        ok = upd_install_file(!cia, UPD_TMP2);
        if (main_too) g_upd.step = 2;
    }
    if (ok && main_too) {
        g_upd.fail_other = false;
        ok = upd_install_file(cia, UPD_TMP);
    }
    if (had_romfs) g_romfs = R_SUCCEEDED(romfsInit());
    if (!ok) { upd_abort(UFAIL_INSTALL); return; }
    remove(UPD_TMP);
    remove(UPD_TMP2);
    g_upd.state = UPD_DONE;
}

static bool upd_busy(void) {
    return g_upd.state == UPD_DOWNLOADING || g_upd.state == UPD_INSTALLING;
}

// Looks for the format that is not running. A found .3dsx also becomes the file to replace.
static void upd_detect_other(void) {
    if (g_upd.is_cia) {
        static const char *paths[] = { DEFAULT_3DSX_PATH, ALT_3DSX_PATH };
        struct stat st;
        g_upd.other = 0;
        snprintf(g_upd.target, sizeof g_upd.target, "%s", DEFAULT_3DSX_PATH);
        for (int i = 0; i < 2; i++) {
            if (stat(paths[i], &st) == 0 && S_ISREG(st.st_mode)) {
                snprintf(g_upd.target, sizeof g_upd.target, "%s", paths[i]);
                g_upd.other = 1;
                break;
            }
        }
    } else {
        if (R_FAILED(amInit())) { g_upd.other = -1; return; }   // no AM: cannot install a .cia
        u64 id = SELF_TITLE_ID;
        AM_TitleEntry info;
        g_upd.other = R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD, 1, &id, &info)) ? 1 : 0;
        amExit();
    }
}

// Shown with or without a newer version, but not while the update check is running.
static bool upd_can_both(void) {
    return g_upd.other >= 0 && g_app.state != CHK_RUNNING && (g_upd.state == UPD_IDLE || g_upd.state == UPD_FAIL);
}

static bool upd_start(bool both, bool only_other) {
    if (upd_busy() || !net_init()) return false;
    g_upd.both = both;
    g_upd.only_other = both && only_other;
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    g_upd.state = UPD_DOWNLOADING;
    s32 prio = 0x30;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    prio -= 1;
    if (prio < 0x18) prio = 0x18;
    Thread t = threadCreate(upd_thread, NULL, 128 * 1024, prio, -1, true);
    if (!t) { g_upd.state = UPD_FAIL; g_upd.fail = UFAIL_NET; return false; }
    return true;
}

// ------------------------------------------------------------ Extra menu (cheat downloads)

#define EX_ITEMS 2
#define NOTE_AI    EX_ITEMS         // note_dialog values after the Extra items
#define NOTE_NAMES (EX_ITEMS + 1)
static const char *EX_NAMES[EX_ITEMS] = { "MK7_Cheat", "ACNL_Cheat" };
static const char *EX_FILES_MK7[]  = { "0004000000030600.txt", "0004000000030700.txt", "0004000000030800.txt" };
static const char *EX_FILES_ACNL[] = { "0004000000086200.txt" };
static const char **EX_FILES[EX_ITEMS] = { EX_FILES_MK7, EX_FILES_ACNL };
static const int EX_COUNT[EX_ITEMS] = { 3, 1 };
// the first line of this file holds the version shown in the menu
static const char *EX_VERFILE[EX_ITEMS] = { "0004000000030600.txt", "0004000000086200.txt" };

static const char *EX_REGIONS[EX_ITEMS] = { "JPN / USA / EUR", "JPN" };

static CheckCtx g_exver[EX_ITEMS];          // version (first line) checks

enum { EX_IDLE = 0, EX_RUNNING, EX_DONE, EX_FAIL };
typedef struct {
    volatile int state[EX_ITEMS];
    volatile int done;                      // files finished in the running job
    volatile int item;                      // running item (-1 = none)
} ExJob;
static ExJob g_ex = { {0}, 0, -1 };

static volatile bool g_ex_quit = false;     // set at exit: stops a running cheat download

static int ex_xfer_cb(void *user, curl_off_t a, curl_off_t b, curl_off_t c, curl_off_t d) {
    (void)user; (void)a; (void)b; (void)c; (void)d;
    return g_ex_quit ? 1 : 0;               // non-zero aborts; http_download removes the .tmp
}

static size_t ex_write_cb(char *p, size_t size, size_t nmemb, void *user) {
    return fwrite(p, size, nmemb, (FILE *)user) * size;
}

// Downloads url to path (via path.tmp, replaced only on success).
static bool http_download(const char *url, const char *path) {
    const char *ca = find_cacert();
    if (!ca) return false;
    char tmp[310];                       // path (max 300) + ".tmp"
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *f = fopen(tmp, "wb");
    if (!f) return false;
    setvbuf(f, NULL, _IOFBF, 32 * 1024);

    CURL *c = curl_easy_init();
    if (!c) { fclose(f); remove(tmp); return false; }
    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 20L);
    curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(c, CURLOPT_USERAGENT, "MoreCTGP7Manager/" APP_VERSION);
    curl_easy_setopt(c, CURLOPT_CAINFO, ca);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, ex_write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, ex_xfer_cb);

    CURLcode rc = curl_easy_perform(c);
    long code = 0;
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(c);
    long len = ftell(f);
    bool closed = fclose(f) == 0;

    if (rc != CURLE_OK || code != 200 || !closed || len <= 0) { remove(tmp); return false; }
    remove(path);
    if (rename(tmp, path) != 0) { remove(tmp); return false; }
    return true;
}

static void ex_thread(void *arg) {
    int it = (int)(intptr_t)arg;
    bool ok = true;
    mkdir(CHEATS_DIR, 0777);
    for (int i = 0; i < EX_COUNT[it] && ok; i++) {
        char url[300], path[300];
        snprintf(url, sizeof url, EXTRA_RAW "%s", EX_FILES[it][i]);
        snprintf(path, sizeof path, CHEATS_DIR "/%s", EX_FILES[it][i]);
        ok = http_download(url, path);
        if (ok) g_ex.done = i + 1;
    }
    g_ex.state[it] = ok ? EX_DONE : EX_FAIL;
    g_ex.item = -1;
}

static bool ex_busy(void) { return g_ex.item >= 0; }

static bool net_extra_busy(void) {
    if (ex_busy()) return true;
    for (int i = 0; i < EX_ITEMS; i++) if (g_exver[i].state == CHK_RUNNING) return true;
    return false;
}

// Asks the running version checks and cheat download to stop (used when the app exits).
static void net_extra_quit(void) {
    g_ex_quit = true;
    for (int i = 0; i < EX_ITEMS; i++) g_exver[i].quit = true;
}

static bool ex_start(int it) {
    if (ex_busy() || !net_init()) return false;
    g_ex.done = 0;
    g_ex.item = it;
    g_ex.state[it] = EX_RUNNING;
    s32 prio = 0x30;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    prio -= 1;
    if (prio < 0x18) prio = 0x18;
    Thread t = threadCreate(ex_thread, (void *)(intptr_t)it, 64 * 1024, prio, -1, true);
    if (!t) { g_ex.state[it] = EX_FAIL; g_ex.item = -1; return false; }
    return true;
}

// Fetches the version lines (once per visit, if not already running/done).
static void ex_fetch_versions(void) {
    for (int i = 0; i < EX_ITEMS; i++) {
        if (g_exver[i].state == CHK_RUNNING || g_exver[i].state == CHK_DONE) continue;
        char url[300];
        snprintf(url, sizeof url, EXTRA_RAW "%s", EX_VERFILE[i]);
        g_exver[i].raw = true;
        check_start(&g_exver[i], url);
    }
}

// ------------------------------------------------------------ sound effects (ndsp)
// WAV files in romfs:/sound/. Needs sdmc:/3ds/dspfirm.cdc; without it the app runs silently.

// SE_BGM is the background music; it uses the same loader on its own DSP channel.
enum { SE_OK = 0, SE_CANCEL, SE_DECIDE, SE_CURSOR, SE_DLG_OPEN, SE_DLG_CLOSE, SE_SLOT,
       SE_ATTENTION, SE_FOUND, SE_BUSY, SE_SCROLL, SE_COMPLETE, SE_ENGINE_IDLE, SE_ENGINE_GO, SE_WIN_YES, SE_WIN_NO, SE_ROLL, SE_ROLLPASS, SE_BGM, SE_COUNT };

#define BGM_GAP_MS 3000   // silence between BGM loops

static const char *SE_FILES[SE_COUNT] = {
    "romfs:/sound/SE_SYS_BTN_OK.wav",
    "romfs:/sound/SE_SYS_CANCEL_S.wav",
    "romfs:/sound/SE_SYS_DECIDE_TITLE.wav",
    "romfs:/sound/SE_SYS_CURSOR.wav",
    "romfs:/sound/SE_SYS_DIALOG_OPEN.wav",
    "romfs:/sound/SE_SYS_DIALOG_CLOSE.wav",
    "romfs:/sound/SE_SYS_SLOT_FRAME.wav",
    "romfs:/sound/SE_SYS_ATTENTION_POPUP.wav",     // Wi-Fi / battery notices
    "romfs:/sound/SE_SYS_WIFI_CONNECT_OK.wav",     // update found
    "romfs:/sound/SE_SYS_BUSY_SYMBOL.wav",         // while checking / updating (every 1 s)
    "romfs:/sound/SE_SYS_SCROLL_LIST_STOP.wav",    // items slide in
    "romfs:/sound/SE_SYS_CHAT_DECIDE.wav",         // update / cheat download finished
    "romfs:/sound/SE_SYS_RACE_READY_ENGINE_IDLING.wav",   // loops while "Launch CTGP-7?" is open
    "romfs:/sound/SE_SYS_RACE_GO_ENGINE.wav",      // CTGP-7 launches
    "romfs:/sound/SE_RC_PAUSE_OFF.wav",            // window: OK / Yes / Update
    "romfs:/sound/SE_RC_PAUSE_ON.wav",             // window: B / No / Later
    "romfs:/sound/SE_SYS_SLOT_ROLLING.wav",        // while a seek bar is held
    "romfs:/sound/SE_SYS_SLOT_ROLLPASS.wav",       // a seek bar reached its end
    "romfs:/sound/SE_SYS_WIFI_MII_CUPSULE_MII_MOVE.wav",
};

typedef struct {
    u8 *data;            // linear memory
    ndspWaveBuf wb;
    bool ok;
} Sfx;

static Sfx g_se[SE_COUNT];
static bool g_ndsp = false;
static bool g_sfx_on = true;

static u32 rd32(const u8 *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24); }
static u16 rd16(const u8 *p) { return p[0] | (p[1] << 8); }

// Loads a PCM WAV (8/16-bit, mono/stereo) and sets up DSP channel `ch` for it.
static bool sfx_load(int ch, const char *path) {
    Sfx *s = &g_se[ch];
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 44) { fclose(f); return false; }
    u8 *buf = malloc(len);
    if (!buf) { fclose(f); return false; }
    size_t got = fread(buf, 1, len, f);
    fclose(f);
    if ((long)got != len || memcmp(buf, "RIFF", 4) || memcmp(buf + 8, "WAVE", 4)) { free(buf); return false; }

    u16 fmt = 0, chans = 0, bits = 0;
    u32 rate = 0, dsize = 0;
    const u8 *pcm = NULL;
    long i = 12;
    while (i + 8 <= len) {
        u32 sz = rd32(buf + i + 4);
        if (!memcmp(buf + i, "fmt ", 4) && sz >= 16 && i + 8 + 16 <= len) {
            fmt = rd16(buf + i + 8);
            chans = rd16(buf + i + 10);
            rate = rd32(buf + i + 12);
            bits = rd16(buf + i + 22);
        } else if (!memcmp(buf + i, "data", 4)) {
            pcm = buf + i + 8;
            dsize = sz;
            if (i + 8 + (long)dsize > len) dsize = len - (i + 8);
        }
        i += 8 + sz + (sz & 1);
    }
    if (fmt != 1 || !pcm || !dsize || (chans != 1 && chans != 2) || (bits != 8 && bits != 16) || !rate) {
        free(buf);
        return false;
    }

    s->data = linearAlloc(dsize);
    if (!s->data) { free(buf); return false; }
    memcpy(s->data, pcm, dsize);
    free(buf);
    if (bits == 8)                                  // WAV 8-bit is unsigned, DSP wants signed
        for (u32 k = 0; k < dsize; k++) s->data[k] ^= 0x80;
    DSP_FlushDataCache(s->data, dsize);

    u16 ndfmt = (bits == 16) ? (chans == 2 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16)
                             : (chans == 2 ? NDSP_FORMAT_STEREO_PCM8  : NDSP_FORMAT_MONO_PCM8);
    float mix[12] = {0};
    mix[0] = mix[1] = 1.0f;
    ndspChnReset(ch);
    ndspChnSetInterp(ch, NDSP_INTERP_LINEAR);
    ndspChnSetRate(ch, (float)rate);
    ndspChnSetFormat(ch, ndfmt);
    ndspChnSetMix(ch, mix);

    memset(&s->wb, 0, sizeof s->wb);
    s->wb.data_vaddr = s->data;
    s->wb.nsamples = dsize / (chans * (bits / 8));
    s->wb.looping = false;
    s->wb.status = NDSP_WBUF_DONE;
    s->ok = true;
    return true;
}

// SE_SYS_SLOT_ROLLING: plays from the start once, then the second half loops. The end of the
// loop is cross-faded into what comes before its start, so the two ends meet without a click.
#define ROLL_LOOP_START 22231        // samples
#define ROLL_LOOP_END   35067
#define ROLL_FADE       1200

static ndspWaveBuf g_roll_loop;
static bool g_roll_ok = false;

static void roll_setup(void) {
    Sfx *r = &g_se[SE_ROLL];
    if (!r->ok || r->wb.nsamples < ROLL_LOOP_END) return;     // not the expected file: plays once
    s16 *d = (s16 *)r->data;                                  // 16-bit stereo
    for (int i = 0; i < ROLL_FADE; i++) {
        float t = (float)i / ROLL_FADE;
        s16 *to = d + (ROLL_LOOP_END - ROLL_FADE + i) * 2, *from = d + (ROLL_LOOP_START - ROLL_FADE + i) * 2;
        to[0] = (s16)(to[0] * (1 - t) + from[0] * t);
        to[1] = (s16)(to[1] * (1 - t) + from[1] * t);
    }
    DSP_FlushDataCache(r->data, ROLL_LOOP_END * 4);
    memset(&g_roll_loop, 0, sizeof g_roll_loop);
    g_roll_loop.data_vaddr = d + ROLL_LOOP_START * 2;
    g_roll_loop.nsamples = ROLL_LOOP_END - ROLL_LOOP_START;
    g_roll_loop.looping = true;
    g_roll_loop.status = NDSP_WBUF_DONE;
    r->wb.nsamples = ROLL_LOOP_START;                         // the first part ends where the loop begins
    g_roll_ok = true;
}

static void sfx_init(void) {
    if (R_FAILED(ndspInit())) return;              // no dspfirm.cdc etc.
    g_ndsp = true;
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    for (int i = 0; i < SE_COUNT; i++) sfx_load(i, SE_FILES[i]);
    g_se[SE_ENGINE_IDLE].wb.looping = true;
    roll_setup();
}

static void sfx_play(int id) {
    if (!g_ndsp || !g_sfx_on || id < 0 || id >= SE_COUNT || !g_se[id].ok) return;
    ndspChnWaveBufClear(id);                        // restart if still playing
    g_se[id].wb.status = NDSP_WBUF_DONE;
    ndspChnWaveBufAdd(id, &g_se[id].wb);
}

static void roll_start(void) {
    sfx_play(SE_ROLL);
    if (!g_ndsp || !g_sfx_on || !g_roll_ok) return;
    g_roll_loop.status = NDSP_WBUF_DONE;
    ndspChnWaveBufAdd(SE_ROLL, &g_roll_loop);
}

static bool sfx_playing(int id) {
    return g_ndsp && id >= 0 && id < SE_COUNT && g_se[id].ok && ndspChnIsPlaying(id);
}

static void sfx_stop(int id) {
    if (!g_ndsp || id < 0 || id >= SE_COUNT || !g_se[id].ok) return;
    ndspChnWaveBufClear(id);
    g_se[id].wb.status = NDSP_WBUF_DONE;
}

static bool sfx_busy(void) {
    if (!g_ndsp) return false;
    for (int i = 0; i < SE_COUNT; i++)
        if (i != SE_BGM && g_se[i].ok && ndspChnIsPlaying(i)) return true;
    return false;
}

// BGM: play once, wait BGM_GAP_MS after it ends, play again. Call every frame.
static int g_bgm_state = 0;      // 0 = start now, 1 = playing, 2 = waiting
static u64 g_bgm_next = 0;

static void bgm_update(bool on) {
    if (!g_ndsp || !g_se[SE_BGM].ok) return;
    Sfx *b = &g_se[SE_BGM];
    if (!on) {
        if (g_bgm_state != 0) {
            ndspChnWaveBufClear(SE_BGM);
            b->wb.status = NDSP_WBUF_DONE;
        }
        g_bgm_state = 0;
        return;
    }
    u64 now = osGetTime();
    if (g_bgm_state == 1 && b->wb.status == NDSP_WBUF_DONE) {
        g_bgm_state = 2;
        g_bgm_next = now + BGM_GAP_MS;
    }
    if (g_bgm_state == 0 || (g_bgm_state == 2 && now >= g_bgm_next)) {
        ndspChnWaveBufClear(SE_BGM);
        b->wb.status = NDSP_WBUF_DONE;
        ndspChnWaveBufAdd(SE_BGM, &b->wb);
        g_bgm_state = 1;
    }
}

static void sfx_exit(void) {
    if (!g_ndsp) return;
    for (int i = 0; i < SE_COUNT; i++) {
        ndspChnWaveBufClear(i);
        if (g_se[i].data) linearFree(g_se[i].data);
    }
    ndspExit();
    g_ndsp = false;
}

// ------------------------------------------------------------ folder logic

static int exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static void slot_path(char *out, size_t n, int slot) {
    snprintf(out, n, BASE "%s", g_names[slot - 1]);
}

// Must be a legal folder name and must not be the active folder's fixed name.
static bool name_valid(const char *s) {
    size_t n = strlen(s);
    if (!n || s[0] == ' ' || s[n - 1] == ' ' || s[n - 1] == '.') return false;
    if (strpbrk(s, "/\\:*?\"<>|")) return false;
    return strcasecmp(s, "Characters") != 0;
}

// Renames the folders on the SD card to the edited names and stores them. Each folder goes
// through a temporary name first, so names can be swapped; on failure everything is put back.
// The active slot needs no rename: it is "Characters" and takes the new name when switched out.
// Returns 0 = done, 1 = name already in use, 2 = rename failed.
static int names_apply(char ed[SLOTS][NAME_BUF], int active) {
    char oldp[SLOTS][256], newp[SLOTS][256], tmpp[SLOTS][256];
    int st[SLOTS] = {0};                    // 0 untouched, 1 at temporary name, 2 at new name
    bool ok = true;

    for (int i = 0; i < SLOTS; i++) {
        for (int j = i + 1; j < SLOTS; j++)
            if (!strcasecmp(ed[i], ed[j])) return 1;
        snprintf(oldp[i], sizeof oldp[i], BASE "%s", g_names[i]);
        snprintf(newp[i], sizeof newp[i], BASE "%s", ed[i]);
        snprintf(tmpp[i], sizeof tmpp[i], BASE ".mctgp7_tmp%d", i + 1);
        bool ours = false;                  // a folder of that name that is not one of ours?
        for (int k = 0; k < SLOTS; k++) if (!strcasecmp(ed[i], g_names[k])) ours = true;
        if (!ours && exists(newp[i])) return 1;
    }
    for (int i = 0; i < SLOTS && ok; i++) {
        if (i + 1 == active || !strcmp(ed[i], g_names[i]) || !exists(oldp[i])) continue;
        if (rename(oldp[i], tmpp[i]) == 0) st[i] = 1; else ok = false;
    }
    for (int i = 0; i < SLOTS && ok; i++) {
        if (st[i] != 1) continue;
        if (rename(tmpp[i], newp[i]) == 0) st[i] = 2; else ok = false;
    }
    if (!ok) {
        for (int i = 0; i < SLOTS; i++) if (st[i] == 2) { rename(newp[i], tmpp[i]); st[i] = 1; }
        for (int i = 0; i < SLOTS; i++) if (st[i] == 1) rename(tmpp[i], oldp[i]);
        return 2;
    }
    memcpy(g_names, ed, sizeof g_names);
    names_save();
    return 0;
}

typedef enum { ERR_NONE = 0, ERR_NO_ACTIVE, ERR_AMBIGUOUS, ERR_UNEXPECTED } ErrCode;
typedef enum { MSG_NONE = 0, MSG_ALREADY, MSG_DONE, MSG_FAIL1, MSG_FAIL2, MSG_NOLAUNCH } MsgKind;

// Returns: 1-3 = active slot, 0 = none active, -1 = invalid layout (*err set)
static int detect_active(ErrCode *err) {
    int present = 0, missing = 0;
    char p[256];
    for (int i = 1; i <= SLOTS; i++) {
        slot_path(p, sizeof p, i);
        if (exists(p)) present++; else missing = i;
    }
    int has_active = exists(ACTIVE_NAME);

    *err = ERR_NONE;
    if (!has_active && present == SLOTS) return 0;
    if (has_active && present == SLOTS - 1) return missing;

    if (!has_active && present == SLOTS - 1) *err = ERR_NO_ACTIVE;
    else if (has_active && present == SLOTS) *err = ERR_AMBIGUOUS;
    else *err = ERR_UNEXPECTED;
    return -1;
}

// Startup: creates the missing numbered folders (only if CTGP-7 is on the SD card). An existing
// "Characters" keeps its name and stands for the lowest missing number, so that number is not
// created. Nothing is created when "Characters" and all three numbered folders exist.
static void ensure_slots(void) {
    if (!exists("sdmc:/CTGP-7")) return;
    mkdir("sdmc:/CTGP-7/MyStuff", 0777);
    bool skip = exists(ACTIVE_NAME);
    char p[256];
    for (int i = 1; i <= SLOTS; i++) {
        slot_path(p, sizeof p, i);
        if (exists(p)) continue;
        if (skip) { skip = false; continue; }
        mkdir(p, 0777);
    }
}

// Returns the message kind; *arg is the number to show in the message.
static MsgKind do_switch(int active, int target, int *arg) {
    char from[256], to[256];

    if (active == target) {
        *arg = target;
        return MSG_ALREADY;
    }

    // 1) Add the number back to the current folder (move it to standby)
    if (active != 0) {
        slot_path(to, sizeof to, active);
        if (rename(ACTIVE_NAME, to) != 0) {
            *arg = active;
            return MSG_FAIL1;
        }
    }

    // 2) Remove the number from the target folder (make it active)
    slot_path(from, sizeof from, target);
    if (rename(from, ACTIVE_NAME) != 0) {
        if (active != 0) {            // roll back step 1
            slot_path(to, sizeof to, active);
            rename(to, ACTIVE_NAME);
        }
        *arg = target;
        return MSG_FAIL2;
    }

    *arg = target;
    return MSG_DONE;
}

// ------------------------------------------------------------ CTGP-7 launch

// Title ID of the official CTGP-7 Launcher
#define CTGP7_TITLE_ID 0x0004000003070C00ULL
#define LAUNCH_SOUND_FRAMES 60      // sound heard before leaving for CTGP-7 (~1 s)

// true if the CTGP-7 Launcher is installed on the SD card
static bool ctgp7_installed(void) {
    if (R_FAILED(amInit())) return true;          // cannot check: just try to launch
    u64 id = CTGP7_TITLE_ID;
    AM_TitleEntry info;
    bool ok = R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD, 1, &id, &info));
    amExit();
    return ok;
}

// ------------------------------------------------------------ drawing

typedef struct { float x, y, w, h; } Rect;

// ---- buttons and items slide in from the left when a screen opens ----
// Drawing only: input always uses the final positions, so nothing has to wait for the slide.
#define SLIDE_MS      130          // time one item takes
#define SLIDE_STAGGER 30           // delay between one item and the next
#define SLIDE_DIST    330.0f       // start position, off the left edge of the screen

static u64 g_slide_t0 = 0;

// Horizontal offset of item number `order` (0 = first, top of the screen).
static float slide_dx(int order) {
    long long t = (long long)(osGetTime() - g_slide_t0) - (long long)order * SLIDE_STAGGER;
    if (t <= 0) return -SLIDE_DIST;
    if (t >= SLIDE_MS) return 0;
    float f = 1.0f - (float)t / SLIDE_MS;           // ease-out
    return -SLIDE_DIST * f * f * f;
}

static Rect slid(Rect r, int order) {
    r.x += slide_dx(order);
    return r;
}

static C2D_TextBuf g_buf;
static C2D_Font g_font;

// Theme colors for text drawn directly on the background (change with dark mode)
static u32 COL_WHITE, COL_YELLOW, COL_GREEN, COL_RED, COL_GRAY, COL_BG, COL_SHADOW_BG;
static u32 g_shadow;          // shadow color used by draw_text (set per context)
static float g_btn_sc = 0.6f; // label scale used by draw_button
static float g_btn_reserve = 0; // space kept free on the right of a label (switches)
static int g_btn_chars = -1;    // character picture on the next buttons (-1 = none)
static float g_btn_indent = 0;  // extra space on the left of a label (icon)
static bool g_btn_glass = true; // true: see-through MK7 menu button, false: image button (main screen)
static bool g_text_outline = false; // draw_text: outline in the shadow color instead of a drop shadow
static u32 g_held = 0;          // keys held this frame (for the pressed look)
static touchPosition g_touch;   // current touch position
static bool g_press_block = false; // true while dragging a list: no pressed look
// Fixed colors (buttons, scroll bar, fallback panels)
static u32 COL_PANEL, COL_PANEL_SEL, COL_PANEL_ACT, COL_BORDER;
static u32 BTN_TEXT, BTN_VALUE, BTN_ACT, BTN_DIS, SHADOW;

static void colors_init(void) {
    COL_PANEL     = C2D_Color32(0x28, 0x2A, 0x38, 0xFF);
    COL_PANEL_SEL = C2D_Color32(0x3A, 0x40, 0x60, 0xFF);
    COL_PANEL_ACT = C2D_Color32(0x1E, 0x50, 0x34, 0xFF);
    COL_BORDER    = C2D_Color32(0x50, 0x54, 0x68, 0xFF);
    BTN_TEXT      = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    BTN_VALUE     = C2D_Color32(0xFF, 0xE0, 0x40, 0xFF);
    BTN_ACT       = C2D_Color32(0x70, 0xFF, 0x90, 0xFF);
    BTN_DIS       = C2D_Color32(0xA0, 0xA0, 0xA0, 0xFF);
    SHADOW        = C2D_Color32(0x00, 0x00, 0x00, 0xE0);
}

static void apply_theme(bool dark) {
    if (dark) {     // light text on the inverted (dark) background
        COL_WHITE  = C2D_Color32(0xF0, 0xF0, 0xF0, 0xFF);
        COL_YELLOW = C2D_Color32(0xFF, 0xE0, 0x40, 0xFF);
        COL_GREEN  = C2D_Color32(0x60, 0xFF, 0x80, 0xFF);
        COL_RED    = C2D_Color32(0xFF, 0x70, 0x70, 0xFF);
        COL_GRAY   = C2D_Color32(0xB8, 0xB8, 0xB8, 0xFF);
        COL_BG     = C2D_Color32(0x24, 0x26, 0x30, 0xFF);
        COL_SHADOW_BG = C2D_Color32(0x00, 0x00, 0x00, 0xE0);
    } else {        // dark text on the light background
        COL_WHITE  = C2D_Color32(0x1C, 0x1C, 0x24, 0xFF);
        COL_YELLOW = C2D_Color32(0x7A, 0x56, 0x00, 0xFF);
        COL_GREEN  = C2D_Color32(0x0E, 0x70, 0x2C, 0xFF);
        COL_RED    = C2D_Color32(0xB8, 0x18, 0x18, 0xFF);
        COL_GRAY   = C2D_Color32(0x48, 0x48, 0x50, 0xFF);
        COL_BG     = C2D_Color32(0xE6, 0xE6, 0xEA, 0xFF);
        COL_SHADOW_BG = C2D_Color32(0xFF, 0xFF, 0xFF, 0xE8);
    }
    g_shadow = COL_SHADOW_BG;
}

// ---- images (romfs:/gfx/*.t3x, built from gfx/*.t3s) ----
static C2D_SpriteSheet g_top_bg = NULL;   // InfoBackground
static C2D_SpriteSheet g_bot_bg = NULL;   // Upd
static C2D_SpriteSheet g_btns   = NULL;   // 0: tinyButtonBackground, 1: selTinyButtonBackground, 2: selFrame (mask of its yellow frame),
                                          // 3/4: blue part of 0/1 as gray masks (recolored green for the active slot)
static C2D_SpriteSheet g_logo   = NULL;   // 0: top-screen logo (220x103), 1: about logo (180x84)
static C2D_SpriteSheet g_kart   = NULL;   // kart silhouette that drives across the top screen
static C2D_SpriteSheet g_shell  = NULL;   // shell that shoots across the top screen
static C2D_SpriteSheet g_bowser = NULL;   // Bowser kart silhouette
static C2D_SpriteSheet g_lakitu = NULL;   // Lakitu silhouette (fishing, floats above the others)
static C2D_SpriteSheet g_chars  = NULL;   // 0: Mario (active button), 1: Mario & Luigi (launch button)
static C2D_SpriteSheet g_spanner = NULL;  // silver spanner (custom folders)
static C2D_SpriteSheet g_seek   = NULL;   // seek bar: 0 knob, 1 vertical slot, 2 horizontal slot
static C2D_SpriteSheet g_icons  = NULL;   // small marks: 0 restart, 1 save, 2 download
static C2D_SpriteSheet g_trash  = NULL;   // trash can mark
static C2D_SpriteSheet g_hands  = NULL;   // seek bar hand: 0 pointing, 1 fist
static C2D_SpriteSheet g_window = NULL;   // window backgrounds: 0 large (UPD_BOX), 1 small (DLG_BOX)
static C2D_SpriteSheet g_parts  = NULL;   // bottom-screen square particles: 0 big, 1 small (white, tinted)
static C2D_SpriteSheet g_back   = NULL;   // back tab (arrow + B) for the bottom-left corner
static C2D_SpriteSheet g_lens   = NULL;   // round icons: 0 disc, 1 ring+gloss, 2 selected disc, 3 selected ring, 4 gloss

// Draws logo `idx` centered at cx with its top at y. Returns the drawn height (0 if missing).
static float draw_logo(int idx, float cx, float y) {
    if (!g_logo) return 0;
    C2D_Image im = C2D_SpriteSheetGetImage(g_logo, idx);
    C2D_DrawImageAt(im, cx - im.subtex->width / 2.0f, y, 0.5f, NULL, 1.0f, 1.0f);
    return im.subtex->height;
}
static bool g_bg_inverted = false;

// Opaque area of tinyButtonBackground inside its 310x60 image.
// Both button images are drawn with the same transform, so the selected
// frame keeps its slightly larger glow around the button.
#define BTN_IN_X 3.0f
#define BTN_IN_Y 4.0f
#define BTN_IN_W 304.0f
#define BTN_IN_H 52.0f
// center of the blue (right) part, as a fraction of the inner button size
#define BLUE_CX  ((223.0f - BTN_IN_X) / BTN_IN_W)
#define BLUE_CY  ((30.5f - BTN_IN_Y) / BTN_IN_H)

static void invert_sheet(C2D_SpriteSheet sh) {
    if (!sh) return;
    C2D_Image im = C2D_SpriteSheetGetImage(sh, 0);
    C3D_Tex *t = im.tex;
    if (!t || t->fmt != GPU_RGBA8) return;
    u8 *d = (u8 *)t->data;
    u32 n = t->size;
    for (u32 i = 0; i + 3 < n; i += 4) {   // byte order A,B,G,R: keep alpha
        d[i + 1] ^= 0xFF;
        d[i + 2] ^= 0xFF;
        d[i + 3] ^= 0xFF;
    }
    GSPGPU_FlushDataCache(d, n);
}

static void set_bg_dark(bool dark) {
    if (dark == g_bg_inverted) return;
    invert_sheet(g_top_bg);
    invert_sheet(g_bot_bg);
    invert_sheet(g_kart);          // black kart on light bg / white kart on dark bg
    invert_sheet(g_bowser);
    invert_sheet(g_lakitu);
    invert_sheet(g_shell);
    invert_sheet(g_window);
    invert_sheet(g_hands);
    g_bg_inverted = dark;
}

// ---- custom background color ----
// hue only, so black and white cannot be chosen. The strength is fixed: pale in light mode and
// dark in dark mode, so buttons and text stay readable.
static u32 hsv_color(float h, float s, float v, u8 a) {
    float c = v * s, hh = h / 60.0f, x = c * (1 - fabsf(fmodf(hh, 2.0f) - 1)), m = v - c, r = 0, g = 0, b = 0;
    if (hh < 1) { r = c; g = x; } else if (hh < 2) { r = x; g = c; } else if (hh < 3) { g = c; b = x; }
    else if (hh < 4) { g = x; b = c; } else if (hh < 5) { r = x; b = c; } else { r = c; b = x; }
    return C2D_Color32((u8)((r + m) * 255), (u8)((g + m) * 255), (u8)((b + m) * 255), a);
}

static C2D_ImageTint g_bg_tint;
static bool g_bg_tinted = false;

static void bg_tint_update(int hue, bool dark) {
    g_bg_tinted = hue >= 0;
    if (g_bg_tinted)
        C2D_PlainImageTint(&g_bg_tint, hsv_color(hue, dark ? 0.7f : 0.62f, dark ? 0.42f : 1.0f, 0xFF), dark ? 0.55f : 0.4f);
}

static const C2D_ImageTint *bg_tint(void) { return g_bg_tinted ? &g_bg_tint : NULL; }

static void draw_bg(C2D_SpriteSheet sheet, float sw, float sh) {
    if (!sheet) return;
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);
    C2D_DrawImageAt(im, 0, 0, 0.5f, bg_tint(), sw / im.subtex->width, sh / im.subtex->height);
}

// ---- animated goal-tape (checker) strip at the bottom of both backgrounds ----
// Image rows of the checker band, frame width and pattern period (in image pixels).
// Adjust these if the background images are replaced.
#define TOP_TAPE_Y0 157
#define TOP_TAPE_Y1 196
#define TAPE_BORDER 3
#define TAPE_PERIOD 64.0f
#define TAPE_SPEED  9.0f           // image pixels per second (moves left); 12 px/s on the top screen

// draws image area [ix0,ix1) x [iy0,iy1) (image px) at screen dx,dy with scale sx,sy
static void draw_img_part(C2D_Image im, float ix0, float ix1, float iy0, float iy1,
                          float dx, float dy, float sx, float sy, const C2D_ImageTint *tint) {
    const Tex3DS_SubTexture *o = im.subtex;
    if (ix1 - ix0 < 0.5f || iy1 - iy0 < 0.5f) return;
    float W = o->width, H = o->height;
    Tex3DS_SubTexture st = *o;
    st.left   = o->left + ix0 / W * (o->right - o->left);
    st.right  = o->left + ix1 / W * (o->right - o->left);
    st.top    = o->top + iy0 / H * (o->bottom - o->top);
    st.bottom = o->top + iy1 / H * (o->bottom - o->top);
    st.width  = (u16)((ix1 - ix0) + 0.5f); if (!st.width) st.width = 1;
    st.height = (u16)((iy1 - iy0) + 0.5f); if (!st.height) st.height = 1;
    C2D_Image part = { im.tex, &st };
    C2D_DrawImageAt(part, dx, dy, 0.5f, tint, (ix1 - ix0) * sx / st.width, (iy1 - iy0) * sy / st.height);
}

// Scrolls the checker band horizontally (seamless, uses the pattern period).
// Only the part inside screen rows [cy0, cy1) is drawn.
static void draw_tape(C2D_SpriteSheet sheet, float sw, float sh, int ty0, int ty1, float cy0, float cy1) {
    if (!sheet) return;
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);
    const Tex3DS_SubTexture *o = im.subtex;
    if (o->top < o->bottom) return;                       // rotated: not handled
    float W = o->width, H = o->height, sx = sw / W, sy = sh / H;
    float y0 = ty0, y1 = ty1;                              // clip in image rows
    if (y0 < cy0 / sy) y0 = cy0 / sy;
    if (y1 > cy1 / sy) y1 = cy1 / sy;
    if (y1 <= y0) return;
    float b = TAPE_BORDER;
    float off = fmodf((float)(osGetTime() % 1000000) * TAPE_SPEED / 1000.0f, TAPE_PERIOD);
    off = floorf(off * sx) / sx;       // whole screen pixels: the stretched image would wobble between them
    // piece 1: [b+off, W-b) -> starts at b ; piece 2: one period back fills the right end
    draw_img_part(im, b + off, W - b, y0, y1, b * sx, y0 * sy, sx, sy, bg_tint());
    draw_img_part(im, W - b - TAPE_PERIOD, W - b - TAPE_PERIOD + off, y0, y1, (W - b - off) * sx, y0 * sy, sx, sy, bg_tint());
}

// ---- square particles on the bottom-screen background ----
// Two 512 px wide patterns that repeat seamlessly. They stay inside the frame of the background.
#define PART_FRAME   4.0f            // width of the frame
#define PART_BIG_MS  42667           // big ones: one full pattern to the right (12 px/s)
#define PART_SMALL_MS 42667          // small ones: one full pattern to the left (12 px/s)
#define PART_SMALL_Y 74.0f           // top of the small band (middle of the screen)

static float g_part_y0 = PART_FRAME, g_part_y1 = 240 - PART_FRAME;   // rows the particles may use

static void draw_part_tile(C2D_Image im, float off, float y, const C2D_ImageTint *tint) {
    float W = im.subtex->width, H = im.subtex->height;
    float iy0 = y < g_part_y0 ? g_part_y0 - y : 0;
    float iy1 = y + H > g_part_y1 ? g_part_y1 - y : H;
    if (iy1 <= iy0) return;
    for (int c = 0; c < 2; c++) {
        float x = off - W + c * W;
        float ix0 = x < PART_FRAME ? PART_FRAME - x : 0;
        float ix1 = x + W > 320 - PART_FRAME ? 320 - PART_FRAME - x : W;
        if (ix1 > ix0) draw_img_part(im, ix0, ix1, iy0, iy1, x + ix0, y + iy0, 1, 1, tint);
    }
}

// Redraws rows y0..y1 of the bottom background with its particles (hides what was drawn there).
static void draw_particles(bool dark);
static void draw_bg_band(float y0, float y1, bool dark) {
    if (!g_bot_bg) return;
    draw_img_part(C2D_SpriteSheetGetImage(g_bot_bg, 0), 0, 320, y0, y1, 0, y0, 1, 1, bg_tint());
    g_part_y0 = y0 < PART_FRAME ? PART_FRAME : y0;
    g_part_y1 = y1 > 240 - PART_FRAME ? 240 - PART_FRAME : y1;
    draw_particles(dark);
    g_part_y0 = PART_FRAME;
    g_part_y1 = 240 - PART_FRAME;
}

static void draw_particles(bool dark) {
    if (!g_parts) return;
    u64 now = osGetTime();
    C2D_Image big = C2D_SpriteSheetGetImage(g_parts, 0), small = C2D_SpriteSheetGetImage(g_parts, 1);
    C2D_ImageTint tb, ts;
    C2D_PlainImageTint(&tb, C2D_Color32(0xFF, 0xFF, 0xFF, dark ? 0x2E : 0x4C), 1.0f);
    C2D_PlainImageTint(&ts, C2D_Color32(0xFF, 0xFF, 0xFF, dark ? 0x1A : 0x2E), 1.0f);
    float bw = big.subtex->width, sw = small.subtex->width;
    // The rows sit off the grid of the checker background, so the squares do not line up with it.
    draw_part_tile(big, (float)(now % PART_BIG_MS) / PART_BIG_MS * bw, -13, &tb);
    draw_part_tile(small, sw - (float)(now % PART_SMALL_MS) / PART_SMALL_MS * sw, PART_SMALL_Y + 3, &ts);
}

// ---- racers (Mario, Bowser) & shells across the top screen ----
// One phase lasts 30 s .. 2 min 30 s (random). Each phase shows Mario once, Bowser once and
// three shells, at random moments and without overlapping, plus Lakitu once. Lakitu flies higher,
// so he may pass at the same time as the others. The ground racers use one of these scenes:
//   normal       : everybody passes on their own
//   shell hit    : a shell comes from behind and hits Mario (he spins)
//   side by side : Bowser runs next to Mario and overtakes him
//   bowser fire  : Bowser catches up with Mario, fires a shell at him and overtakes
#define CYCLE_MIN_MS 30000          // 30 s
#define CYCLE_MAX_MS 150000         // 2 min 30 s
#define KART_SPEED   30.0f          // Mario: screen px per second
#define BOWSER_SPEED (KART_SPEED * 2.0f)
#define KART_GROUND  236.0f         // y of the wheels' bottom (Mario, shells)
#define BOWSER_LANE  (-4.0f)        // Bowser runs on a lane slightly further back
#define KART_SCALE   0.8f           // Mario drawn size (1.0 = 77x49)
#define BOWSER_SCALE 0.7f           // Bowser drawn size (1.0 = 57x68)
#define SHELL_SPEED  280.0f         // very fast
#define SPIN_MS      1100           // spin-out duration
#define SHELLS       3
#define LAKITU_SPEED (BOWSER_SPEED * 1.5f)
#define LAKITU_SCALE 0.75f          // drawn size (1.0 = 75x63)
#define LAKITU_Y     130.0f         // top of the image at rest
#define LAKITU_BOB   3.0f           // height of the floating motion (px)
// scene chances in % (the rest = normal). Hits (shell hit + bowser fire) ~ 1 phase in 3.
#define P_SHELL_HIT  18
#define P_SIDE       22
#define P_FIRE       15

static u32 rand_range(u32 lo, u32 hi) {                   // inclusive
    return lo + (u32)(((u64)rand() * (hi - lo + 1)) / ((u64)RAND_MAX + 1));
}
static float frand(float lo, float hi) { return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX); }

typedef struct {
    C2D_SpriteSheet *sheet;
    float scale, speed, lane;
    bool  run, done;
    float x;
    u64   at;                     // planned start
    u64   spin_start;             // 0 = not spinning
    u64   rattle_end, rattle_next, rattle_step;
    bool  rattle_double;
    float rattle_amp, dy, rot;
} Racer;

enum { R_MARIO = 0, R_BOWSER = 1 };
static Racer g_r[2] = {
    { &g_kart,   KART_SCALE,   KART_SPEED,   0,           false, true, 0, 0, 0, 0, 0, 0, false, 1, 0, 0 },
    { &g_bowser, BOWSER_SCALE, BOWSER_SPEED, BOWSER_LANE, false, true, 0, 0, 0, 0, 0, 0, false, 1, 0, 0 },
};

static bool  g_shell_run[SHELLS], g_shell_done[SHELLS] = {true, true, true};
static float g_shell_x[SHELLS];
static u64   g_shell_at[SHELLS];
static int   g_fire_slot = -1;        // shell slot fired by Bowser (-1 = none)
static float g_fire_dist = 70;        // fire when Bowser is this close behind Mario
static u64   g_cycle_end = 0, g_last = 0;
static bool  g_lk_run = false, g_lk_done = true, g_lk_right = false;  // g_lk_right: moves left to right
static float g_lk_x = 0;
static u64   g_lk_at = 0;

static float racer_w(const Racer *r) {
    return *r->sheet ? C2D_SpriteSheetGetImage(*r->sheet, 0).subtex->width * r->scale : 60;
}
static float shell_w(void) { return g_shell ? C2D_SpriteSheetGetImage(g_shell, 0).subtex->width : 26; }
static float lakitu_w(void) {
    return (g_lakitu ? C2D_SpriteSheetGetImage(g_lakitu, 0).subtex->width : 75) * LAKITU_SCALE;
}
static u32 cross_ms(float w, float speed) { return (u32)((404 + w + 8) / speed * 1000); }

typedef struct { u32 dur; int kind; } Block;   // kind: 0 Mario(+scene), 1 Bowser, 2 shell

static void plan_cycle(u64 now) {
    u32 L = rand_range(CYCLE_MIN_MS, CYCLE_MAX_MS);
    g_cycle_end = now + L;
    g_lk_at = now + rand_range(0, L - cross_ms(lakitu_w(), LAKITU_SPEED));
    g_lk_done = false;
    g_lk_right = rand_range(0, 1) == 1;
    float wm = racer_w(&g_r[R_MARIO]), wb = racer_w(&g_r[R_BOWSER]), ws = shell_w();
    u32 md = cross_ms(wm, KART_SPEED) + SPIN_MS;
    u32 bd = cross_ms(wb, BOWSER_SPEED);
    u32 sd = cross_ms(ws, SHELL_SPEED);

    int p = rand_range(0, 99);
    int scene = p < P_SHELL_HIT ? 1 : p < P_SHELL_HIT + P_SIDE ? 2 : p < P_SHELL_HIT + P_SIDE + P_FIRE ? 3 : 0;

    // offsets inside Mario's block
    u32 bowser_off = 0, hit_off = 0;
    u32 mblock = md;
    if (scene == 1) {                                      // hit shell launched when Mario is at x = 140..320
        hit_off = (u32)((404 - frand(140, 320)) / KART_SPEED * 1000);
    } else if (scene == 2) {                               // Bowser enters when Mario is at x = 290..350
        bowser_off = (u32)((404 - frand(290, 350)) / KART_SPEED * 1000);
    } else if (scene == 3) {                               // Bowser enters when Mario is at x = 200..250
        bowser_off = (u32)((404 - frand(200, 250)) / KART_SPEED * 1000);
        g_fire_dist = frand(50, 80);
    }
    if (scene == 2 || scene == 3) { u32 e = bowser_off + bd; if (e > mblock) mblock = e; }

    // blocks to place: Mario-block, Bowser (if separate), free shells
    Block bl[6]; int n = 0;
    bl[n++] = (Block){ mblock, 0 };
    if (scene == 0 || scene == 1) bl[n++] = (Block){ bd, 1 };
    int free_shells = SHELLS - ((scene == 1 || scene == 3) ? 1 : 0);
    for (int i = 0; i < free_shells; i++) bl[n++] = (Block){ sd, 2 };
    for (int i = n - 1; i > 0; i--) { int j = rand_range(0, i); Block t = bl[i]; bl[i] = bl[j]; bl[j] = t; }

    // spread the free time randomly between the blocks (300 ms minimum gap)
    u32 total = 0;
    for (int i = 0; i < n; i++) total += bl[i].dur + 300;
    u32 slack = L > total ? L - total : 0;
    float wts[7], wsum = 0;
    for (int i = 0; i <= n; i++) { wts[i] = frand(0.05f, 1.0f); wsum += wts[i]; }

    for (int s = 0; s < SHELLS; s++) { g_shell_done[s] = true; g_shell_run[s] = false; }
    g_fire_slot = -1;
    int next_shell = 0;
    if (scene == 1 || scene == 3) next_shell = 1;           // slot 0 reserved for the scene
    u64 t = now;
    for (int i = 0; i < n; i++) {
        t += (u64)(slack * (wts[i] / wsum));
        if (bl[i].kind == 0) {
            g_r[R_MARIO].at = t; g_r[R_MARIO].done = false;
            if (scene == 1) { g_shell_at[0] = t + hit_off; g_shell_done[0] = false; }
            if (scene == 2 || scene == 3) { g_r[R_BOWSER].at = t + bowser_off; g_r[R_BOWSER].done = false; }
            if (scene == 3) { g_fire_slot = 0; g_shell_at[0] = UINT64_MAX; g_shell_done[0] = false; }
        } else if (bl[i].kind == 1) {
            g_r[R_BOWSER].at = t; g_r[R_BOWSER].done = false;
        } else if (next_shell < SHELLS) {
            g_shell_at[next_shell] = t; g_shell_done[next_shell] = false; next_shell++;
        }
        t += bl[i].dur + 300;
    }
}

static void rattle_begin(Racer *r, u64 now) {
    r->rattle_end = now + rand_range(300, 650);
    r->rattle_amp = frand(1.0f, 1.8f);
    r->rattle_step = now;
    r->rattle_double = rand_range(0, 99) < 30;          // 30%: two rattles back to back
}

static void racer_update(Racer *r, u64 now, float dt) {
    if (!r->done && !r->run && now >= r->at) {
        r->run = true; r->done = true; r->x = 404; r->spin_start = 0;
        r->rattle_next = now + rand_range(1000, 3500);
        r->rattle_end = 0; r->dy = 0; r->rot = 0;
    }
    if (!r->run) return;
    float sp = r->speed;
    if (r->spin_start) {
        if (now - r->spin_start >= SPIN_MS) r->spin_start = 0;
        else sp *= 0.25f;                                  // nearly stops while spinning
    }
    r->x -= sp * dt;
    if (!r->spin_start) {
        if (now < r->rattle_end) {                         // rattling: random jolts
            if (now >= r->rattle_step) {
                r->dy  = -frand(0.4f, 1.8f) * r->rattle_amp + frand(-0.4f, 0.4f);
                r->rot = frand(-0.025f, 0.025f) * r->rattle_amp;
                r->rattle_step = now + rand_range(30, 75);
            }
        } else if (r->rattle_end) {                        // a rattle just ended
            r->dy = 0; r->rot = 0;
            if (r->rattle_double) {                       // second one with (almost) no pause
                r->rattle_double = false;
                u64 st = now + rand_range(0, 60);
                r->rattle_end = st + rand_range(300, 650);
                r->rattle_amp = frand(1.0f, 1.8f);
                r->rattle_step = st;
            } else {
                r->rattle_end = 0;
                r->rattle_next = now + rand_range(1500, 5000);
            }
        } else if (now >= r->rattle_next) {
            rattle_begin(r, now);
        }
    }
    if (r->x < -racer_w(r) - 4) r->run = false;
}

static void kart_update(void) {
    u64 now = osGetTime();
    if (!g_cycle_end) { srand((unsigned)now); plan_cycle(now); g_last = now; return; }
    float dt = (now - g_last) / 1000.0f;
    g_last = now;
    if (dt > 0.1f) dt = 0.1f;                              // after a pause (HOME menu etc.)

    bool busy = g_r[0].run || g_r[1].run || !g_r[0].done || !g_r[1].done;
    for (int i = 0; i < SHELLS; i++) busy = busy || g_shell_run[i] || !g_shell_done[i];
    busy = busy || g_lk_run || !g_lk_done;
    if (now >= g_cycle_end && !busy) plan_cycle(now);

    Racer *m = &g_r[R_MARIO], *bw = &g_r[R_BOWSER];
    racer_update(m, now, dt);
    racer_update(bw, now, dt);

    // Lakitu: enters from either side
    if (!g_lk_done && now >= g_lk_at) {
        g_lk_done = true; g_lk_run = true;
        g_lk_x = g_lk_right ? -lakitu_w() - 4 : 404;
    }
    if (g_lk_run) {
        g_lk_x += (g_lk_right ? LAKITU_SPEED : -LAKITU_SPEED) * dt;
        if (g_lk_x < -lakitu_w() - 4 || g_lk_x > 404) g_lk_run = false;
    }

    // Bowser fires at Mario when close behind him
    if (g_fire_slot >= 0) {
        int s = g_fire_slot;
        if (bw->run && m->run && !m->spin_start && bw->x - (m->x + racer_w(m)) <= g_fire_dist) {
            g_shell_run[s] = true; g_shell_done[s] = true;
            g_shell_x[s] = bw->x - shell_w() + 6;              // from Bowser's front
            g_fire_slot = -1;
        } else if ((bw->done && !bw->run) || (m->done && !m->run)) {
            g_shell_done[s] = true; g_fire_slot = -1;      // missed the chance: cancel
        }
    }

    // shells: start, move, hit test against Mario
    for (int i = 0; i < SHELLS; i++) {
        if (!g_shell_done[i] && !g_shell_run[i] && now >= g_shell_at[i]) {
            g_shell_run[i] = true; g_shell_done[i] = true; g_shell_x[i] = 404;
        }
        if (!g_shell_run[i]) continue;
        g_shell_x[i] -= SHELL_SPEED * dt;
        if (m->run && !m->spin_start) {
            float kx0 = m->x + 4, kx1 = m->x + racer_w(m) - 4;
            float sx0 = g_shell_x[i], sx1 = g_shell_x[i] + shell_w();
            if (sx0 < kx1 && sx1 > kx0) {                  // hit: Mario spins, shell breaks
                m->spin_start = now;
                m->rattle_end = 0; m->dy = 0; m->rot = 0;
                g_shell_run[i] = false;
                continue;
            }
        }
        if (g_shell_x[i] < -shell_w() - 4) g_shell_run[i] = false;
    }
}

static void draw_racer(const Racer *r, u64 now) {
    if (!r->run || !*r->sheet) return;
    C2D_Image im = C2D_SpriteSheetGetImage(*r->sheet, 0);
    float w = im.subtex->width * r->scale, h = im.subtex->height * r->scale;
    float dy = 0, rot = 0;
    if (r->spin_start) {                                   // spin-out: 2 turns, easing out + small hop
        float t = (now - r->spin_start) / (float)SPIN_MS;
        if (t > 1) t = 1;
        float e = 1 - (1 - t) * (1 - t);
        rot = e * 2.0f * 2.0f * (float)M_PI;
        dy = -sinf(t * (float)M_PI) * 7.0f;
    } else if (now < r->rattle_end && now >= r->rattle_step - 80) {
        dy = r->dy; rot = r->rot;
    }
    C2D_DrawImageAtRotated(im, r->x + w / 2, KART_GROUND + r->lane - h / 2 + dy, 0.5f, rot, NULL, r->scale, r->scale);
}

// Soft floating motion. The image faces left; it is mirrored when moving right.
static void draw_lakitu(u64 now) {
    if (!g_lk_run || !g_lakitu) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_lakitu, 0);
    float w = im.subtex->width * LAKITU_SCALE, h = im.subtex->height * LAKITU_SCALE;
    float t = (now % 1000000) / 1000.0f;
    float dy = sinf(t * 2.6f) * LAKITU_BOB + sinf(t * 1.1f + 1.0f) * 1.0f;
    C2D_DrawParams p;
    p.pos.x = g_lk_x + w / 2;
    p.pos.y = LAKITU_Y + h / 2 + dy;
    p.pos.w = g_lk_right ? -w : w;
    p.pos.h = h;
    p.center.x = w / 2;
    p.center.y = h / 2;
    p.depth = 0.5f;
    p.angle = sinf(t * 1.7f) * 0.04f;
    C2D_DrawImage(im, &p, NULL);
}

static void draw_kart(void) {
    u64 now = osGetTime();
    draw_lakitu(now);
    draw_racer(&g_r[R_BOWSER], now);                       // back lane first
    draw_racer(&g_r[R_MARIO], now);
    if (g_shell) {
        C2D_Image im = C2D_SpriteSheetGetImage(g_shell, 0);
        float h = im.subtex->height;
        for (int i = 0; i < SHELLS; i++) {
            if (!g_shell_run[i]) continue;
            // straight through: no spin, no shake
            C2D_DrawImageAt(im, g_shell_x[i], KART_GROUND - h, 0.5f, NULL, 1.0f, 1.0f);
        }
    }
}

static void parse_text(C2D_Text *t, const char *s) {
    C2D_TextFontParse(t, g_font, g_buf, s);
    C2D_TextOptimize(t);
}

static void text_size(const char *s, float sc, float *w, float *h) {
    C2D_Text t;
    parse_text(&t, s);
    C2D_TextGetDimensions(&t, sc, sc, w, h);
}

// All text gets a clear drop shadow (2 px for normal text, 1 px for small text).
static void draw_text(const char *s, float x, float y, float sc, u32 col) {
    C2D_Text t;
    parse_text(&t, s);
    if (g_shadow && g_text_outline) {
        C2D_DrawText(&t, C2D_WithColor, x - 1, y - 1, 0.5f, sc, sc, g_shadow);
        C2D_DrawText(&t, C2D_WithColor, x + 1, y - 1, 0.5f, sc, sc, g_shadow);
        C2D_DrawText(&t, C2D_WithColor, x - 1, y + 1, 0.5f, sc, sc, g_shadow);
        C2D_DrawText(&t, C2D_WithColor, x + 1, y + 1, 0.5f, sc, sc, g_shadow);
    } else if (g_shadow) {
        if (sc >= 0.5f) C2D_DrawText(&t, C2D_WithColor, x + 2, y + 2, 0.5f, sc, sc, g_shadow);
        C2D_DrawText(&t, C2D_WithColor, x + 1, y + 1, 0.5f, sc, sc, g_shadow);
    }
    C2D_DrawText(&t, C2D_WithColor, x, y, 0.5f, sc, sc, col);
}

static void draw_text_right(const char *s, float right_x, float y, float sc, u32 col) {
    float w, h;
    text_size(s, sc, &w, &h);
    draw_text(s, right_x - w, y, sc, col);
}

static void draw_text_center(const char *s, float cx, float y, float sc, u32 col) {
    float w, h;
    text_size(s, sc, &w, &h);
    draw_text(s, cx - w / 2, y, sc, col);
}

// Several lines ('\n'), each centered.
static void draw_lines_center(const char *s, float cx, float y, float sc, float step, u32 col) {
    char l[160];
    while (*s) {
        size_t n = strcspn(s, "\n");
        size_t cp = n < sizeof l - 1 ? n : sizeof l - 1;
        memcpy(l, s, cp); l[cp] = '\0';
        draw_text_center(l, cx, y, sc, col);
        y += step;
        s += n;
        if (*s == '\n') s++;
    }
}

// Cursor highlight color: loops yellow -> orange -> yellow (about 1.4 s per cycle).
static u32 sel_color(void) {
    float t = 0.5f + 0.5f * sinf((float)(osGetTime() % 1400) / 1400.0f * 2.0f * (float)M_PI);
    u8 g = (u8)(0xE0 + (0x80 - 0xE0) * t);      // 224 (yellow) .. 128 (orange)
    u8 b = (u8)(0x40 + (0x00 - 0x40) * t);
    return C2D_Color32(0xFF, g, b, 0xFF);
}

// A button looks pressed while the stylus is on it or A is held on the selected one.
static bool btn_pressed(Rect r, bool sel, bool enabled) {
    if (!enabled || g_press_block) return false;
    if ((g_held & KEY_TOUCH) && g_touch.px >= r.x && g_touch.px < r.x + r.w &&
        g_touch.py >= r.y && g_touch.py < r.y + r.h) return true;
    return sel && (g_held & KEY_A);
}

#define PRESS_DY 2.0f      // how far a pressed button sinks

// ---- MK7 seek bar, cut from the game's picture ----
// The pictures are stretched only in a plain middle part, so both ends keep their shape and the
// arrows of the knob move to its ends when it gets longer.
#define KNOB_TOP 8                  // knob: rows with the up arrow; one plain row; the rest has the down arrow
#define SLOT_CAP 12                 // slot: end piece, then one plain row

static void draw_slot_v(float x, float y, float h) {
    if (!g_seek) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_seek, 1);
    float W = im.subtex->width, H = im.subtex->height;
    draw_img_part(im, 0, W, 0, SLOT_CAP, x, y, 1, 1, NULL);
    draw_img_part(im, 0, W, SLOT_CAP, SLOT_CAP + 1, x, y + SLOT_CAP, 1, h - (H - 1), NULL);
    draw_img_part(im, 0, W, SLOT_CAP + 1, H, x, y + h - (H - SLOT_CAP - 1), 1, 1, NULL);
}

static void draw_slot_h(float x, float y, float w) {
    if (!g_seek) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_seek, 2);
    float W = im.subtex->width, H = im.subtex->height;
    draw_img_part(im, 0, SLOT_CAP, 0, H, x, y, 1, 1, NULL);
    draw_img_part(im, SLOT_CAP, SLOT_CAP + 1, 0, H, x + SLOT_CAP, y, w - (W - 1), 1, NULL);
    draw_img_part(im, SLOT_CAP + 1, W, 0, H, x + w - (W - SLOT_CAP - 1), y, 1, 1, NULL);
}

static void draw_seek_knob(float x, float y, float h, bool held) {
    if (!g_seek) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_seek, 0);
    float W = im.subtex->width, H = im.subtex->height;
    C2D_ImageTint t;
    C2D_PlainImageTint(&t, C2D_Color32(0xFF, 0xD8, 0x30, 0xFF), 0.4f);
    const C2D_ImageTint *tp = held ? &t : NULL;
    draw_img_part(im, 0, W, 0, KNOB_TOP, x, y, 1, 1, tp);
    draw_img_part(im, 0, W, KNOB_TOP, KNOB_TOP + 1, x, y + KNOB_TOP, 1, h - (H - 1), tp);
    draw_img_part(im, 0, W, KNOB_TOP + 1, H, x, y + h - (H - KNOB_TOP - 1), 1, 1, tp);
}

// 1 px frame without corner pixels (nothing overlaps, so see-through colors stay even).
static void draw_frame(float x, float y, float w, float h, u32 c) {
    C2D_DrawRectSolid(x + 1, y, 0.5f, w - 2, 1, c);
    C2D_DrawRectSolid(x + 1, y + h - 1, 0.5f, w - 2, 1, c);
    C2D_DrawRectSolid(x, y + 1, 0.5f, 1, h - 2, c);
    C2D_DrawRectSolid(x + w - 1, y + 1, 0.5f, 1, h - 2, c);
}

// MK7 menu button: clear glass like the inside of the lens, solid yellow under the cursor.
static void draw_glass_button(Rect r, bool sel, bool pressed, bool enabled) {
    float x = r.x, y = r.y, w = r.w, h = r.h;
    if (sel) {
        u32 pale = C2D_Color32(0xF2, 0xE3, 0x90, 0xFF);
        u32 top = C2D_Color32(0xF8, 0xC8, 0x1C, 0xFF), bot = C2D_Color32(0xFF, 0xE8, 0x08, 0xFF);
        if (pressed) { top = C2D_Color32(0xC0, 0x98, 0x10, 0xFF); bot = C2D_Color32(0xC8, 0xB0, 0x08, 0xFF); }
        draw_frame(x - 3, y - 3, w + 6, h + 6, C2D_Color32(0x7E, 0x71, 0x45, 0xFF));
        C2D_DrawRectSolid(x - 2, y - 2, 0.5f, w + 4, 2, pale);
        C2D_DrawRectSolid(x - 2, y + h, 0.5f, w + 4, 2, C2D_Color32(0xE4, 0xD0, 0x78, 0xFF));
        C2D_DrawRectSolid(x - 2, y, 0.5f, 2, h, pale);
        C2D_DrawRectSolid(x + w, y, 0.5f, 2, h, pale);
        draw_frame(x, y, w, h, C2D_Color32(0x4C, 0x4E, 0x54, 0xFF));
        C2D_DrawRectangle(x + 1, y + 1, 0.5f, w - 2, h - 2, top, top, bot, bot);
        C2D_DrawRectSolid(x + 1, y + 1, 0.5f, w - 2, 2, C2D_Color32(0xFF, 0xFF, 0xFF, 0xF0));
        C2D_DrawRectSolid(x + 1, y + h - 2, 0.5f, w - 2, 1, C2D_Color32(0x28, 0x2A, 0x30, 0x90));
        return;
    }
    // Same light as the cursor look, lit from above, but the middle stays clear glass:
    // white edge and reflection at the top, soft shade and dark edge at the bottom.
    float dim = enabled ? 1.0f : 0.45f;
    u32 white0 = C2D_Color32(0xFF, 0xFF, 0xFF, 0), black0 = C2D_Color32(0, 0, 0, 0);
    u32 shine = C2D_Color32(0xFF, 0xFF, 0xFF, (u8)(0x70 * dim));
    u32 shade = C2D_Color32(0, 0, 0, (u8)(0x40 * dim));
    float top_h = h * 0.42f, bot_h = h * 0.36f;
    draw_frame(x, y, w, h, C2D_Color32(0x4C, 0x4E, 0x54, (u8)(0xFF * dim)));
    C2D_DrawRectSolid(x + 1, y + 1, 0.5f, w - 2, 2, C2D_Color32(0xFF, 0xFF, 0xFF, (u8)(0xF0 * dim)));
    C2D_DrawRectangle(x + 1, y + 3, 0.5f, w - 2, top_h, shine, shine, white0, white0);
    C2D_DrawRectangle(x + 1, y + h - 2 - bot_h, 0.5f, w - 2, bot_h, black0, black0, shade, shade);
    C2D_DrawRectSolid(x + 1, y + h - 2, 0.5f, w - 2, 1, C2D_Color32(0x28, 0x2A, 0x30, (u8)(0x90 * dim)));
    if (pressed) C2D_DrawRectSolid(x + 2, y + 2, 0.5f, w - 4, h - 4, C2D_Color32(0, 0, 0, 0x50));
}
#define ACTIVE_CX 0.63f    // "active" tag position, clear of Mario on the green part

// Character picture at the right end of a button. Its cut-off bottom edge stands on the bottom
// of the inner panel, and it stays inside the button.
static void draw_btn_chars(int idx, Rect r, float sy) {
    if (!g_chars) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_chars, idx);
    float h = r.h * (idx == 0 ? 32.0f / 42.0f : 25.0f / 32.0f);
    float k = h / im.subtex->height;
    float x = r.x + r.w - (idx == 0 ? 12 : 10) - im.subtex->width * k;
    float y = r.y + (47.0f - BTN_IN_Y) * sy - h;
    C2D_DrawImageAt(im, x, y, 0.5f, NULL, k, k);
}

// ---- light running left to right over the active button ----
// The band repeats exactly once per width of the green part, so when it leaves on the right
// it is already entering on the left and the loop has no visible seam.
#define LIGHT_MS  1500             // one trip across the button
#define LIGHT_X0  116.0f           // green part inside the 310x60 mask image
#define LIGHT_X1  297.0f
#define LIGHT_STEP 8.0f            // width of one strip (image px)

static float light_time(void) {
    return (float)(osGetTime() % LIGHT_MS) / (float)LIGHT_MS;
}

// Brightness 0..1 at position u (0..1 = left..right of the green part).
static float light_at(float u, float t) {
    float c = 0.5f + 0.5f * cosf((u - t) * 2.0f * (float)M_PI);
    return c * c * c;
}

// Text with the same running light, inside the letters only: every character gets its own color.
static void draw_text_light(const char *s, float x, float y, float sc) {
    float tw, th, cw, ch, pw, ph, t = light_time();
    bool dark = g_bg_inverted;
    u8 b0 = dark ? 0x60 : 0x0E, b1 = dark ? 0xFF : 0x70, b2 = dark ? 0x80 : 0x2C;   // resting green
    u8 h0 = dark ? 0xE0 : 0x30, h1 = dark ? 0xFF : 0xE0, h2 = dark ? 0xE8 : 0x64;   // lit green
    char pre[256];
    size_t n = strlen(s), i = 0;
    if (n >= sizeof pre) n = sizeof pre - 1;
    text_size(s, sc, &tw, &th);
    if (g_shadow) {
        C2D_Text whole;
        parse_text(&whole, s);
        C2D_DrawText(&whole, C2D_WithColor, x + 2, y + 2, 0.5f, sc, sc, g_shadow);
        C2D_DrawText(&whole, C2D_WithColor, x + 1, y + 1, 0.5f, sc, sc, g_shadow);
    }
    while (i < n) {
        u8 c = (u8)s[i];
        size_t len = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
        if (i + len > n) break;
        char one[5];
        memcpy(one, s + i, len); one[len] = '\0';
        memcpy(pre, s, i + len); pre[i + len] = '\0';
        text_size(pre, sc, &pw, &ph);                   // position from the whole text so far
        text_size(one, sc, &cw, &ch);
        float a = tw > 0 ? light_at((pw - cw / 2) / tw, t) : 0;
        C2D_Text tx;
        parse_text(&tx, one);
        C2D_DrawText(&tx, C2D_WithColor, x + pw - cw, y, 0.5f, sc, sc,
                     C2D_Color32((u8)(b0 + (h0 - b0) * a), (u8)(b1 + (h1 - b1) * a), (u8)(b2 + (h2 - b2) * a), 0xFF));
        i += len;
    }
}

// Pale light clipped to the shape of the mask; each strip fades between its two edges.
static void draw_light_mask(C2D_Image mask, float x, float y, float sx, float sy, float t) {
    const float span = LIGHT_X1 - LIGHT_X0;
    for (float a = LIGHT_X0; a < LIGHT_X1; a += LIGHT_STEP) {
        float b = a + LIGHT_STEP > LIGHT_X1 ? LIGHT_X1 : a + LIGHT_STEP;
        u32 cl = C2D_Color32(0xC8, 0xFF, 0xD4, (u8)(0xD0 * light_at((a - LIGHT_X0) / span, t)));
        u32 cr = C2D_Color32(0xC8, 0xFF, 0xD4, (u8)(0xD0 * light_at((b - LIGHT_X0) / span, t)));
        C2D_ImageTint lt;
        C2D_SetImageTint(&lt, C2D_TopLeft, cl, 1.0f);
        C2D_SetImageTint(&lt, C2D_BotLeft, cl, 1.0f);
        C2D_SetImageTint(&lt, C2D_TopRight, cr, 1.0f);
        C2D_SetImageTint(&lt, C2D_BotRight, cr, 1.0f);
        draw_img_part(mask, a, b, 0, mask.subtex->height, x + a * sx, y, sx, sy, &lt);
    }
}

// Green halo behind the active button; it brightens where the light is.
static void draw_light_halo(float x, float y, float w, float h, float x0, float x1, float t, int layer) {
    const float step = 12.0f;
    for (float a = x; a < x + w; a += step) {
        float b = a + step > x + w ? x + w : a + step;
        u8 al = (u8)((0x14 + 0x58 * light_at((a - x0) / (x1 - x0), t)) / layer);
        u8 ar = (u8)((0x14 + 0x58 * light_at((b - x0) / (x1 - x0), t)) / layer);
        u32 cl = C2D_Color32(0x40, 0xFF, 0x70, al), cr = C2D_Color32(0x40, 0xFF, 0x70, ar);
        C2D_DrawRectangle(a, y, 0.5f, b - a, h, cl, cr, cl, cr);
    }
}

static void draw_button(Rect r, const char *label, const char *value,
                        bool sel, bool active, bool enabled, bool center) {
    bool pressed = btn_pressed(r, sel, enabled);
    Rect o = r;                                  // original position (for the shadow)
    if (pressed) {                               // sink + shrink slightly
        r.x += 2; r.w -= 4; r.y += PRESS_DY; r.h -= 2;
    }

    if (g_btn_glass) {
        draw_glass_button(r, sel, pressed, enabled);
    } else if (g_btns) {
        C2D_Image im = C2D_SpriteSheetGetImage(g_btns, sel ? 1 : 0);
        if (active && C2D_SpriteSheetCount(g_btns) > 4) {   // green halo around the active button
            float hx = r.x + r.w * 0.42f, hw = r.w * 0.58f;
            float k_x = r.w / BTN_IN_W;
            float lx0 = r.x + (LIGHT_X0 - BTN_IN_X) * k_x, lx1 = r.x + (LIGHT_X1 - BTN_IN_X) * k_x;
            for (int k = 3; k >= 1; k--) {
                float e = k * 2.5f;
                draw_light_halo(hx - e, r.y - e, hw + e * 2, r.h + e * 2, lx0, lx1, light_time(), k);
            }
        }
        if (!pressed) {                          // drop shadow under a raised button
            C2D_ImageTint sh;
            C2D_PlainImageTint(&sh, C2D_Color32(0, 0, 0, 0x70), 1.0f);
            float sx = o.w / BTN_IN_W, sy = o.h / BTN_IN_H;
            C2D_DrawImageAt(im, o.x - BTN_IN_X * sx, o.y + 3 - BTN_IN_Y * sy, 0.5f, &sh, sx, sy);
        }
        float sx = r.w / BTN_IN_W, sy = r.h / BTN_IN_H;
        C2D_ImageTint dark;
        if (pressed) C2D_PlainImageTint(&dark, C2D_Color32(0, 0, 0, 0xFF), 0.25f);
        C2D_DrawImageAt(im, r.x - BTN_IN_X * sx, r.y - BTN_IN_Y * sy, 0.5f, pressed ? &dark : NULL, sx, sy);
        if (active && C2D_SpriteSheetCount(g_btns) > 4) {   // active: blue part -> glowing green
            C2D_Image gm = C2D_SpriteSheetGetImage(g_btns, sel ? 4 : 3);
            C2D_ImageTint gt;
            C2D_PlainImageTint(&gt, C2D_Color32(0x38, 0xD0, 0x58, 0xFF), 0.9f);
            float gx = r.x - BTN_IN_X * sx, gy = r.y - BTN_IN_Y * sy;
            C2D_DrawImageAt(gm, gx, gy, 0.5f, &gt, sx, sy);
            draw_light_mask(gm, gx, gy, sx, sy, light_time());
        }
        if (sel && C2D_SpriteSheetCount(g_btns) > 2) {   // recolor the yellow frame (animated)
            C2D_Image fr = C2D_SpriteSheetGetImage(g_btns, 2);
            C2D_ImageTint ft;
            C2D_PlainImageTint(&ft, sel_color(), 0.8f);
            C2D_DrawImageAt(fr, r.x - BTN_IN_X * sx, r.y - BTN_IN_Y * sy, 0.5f, &ft, sx, sy);
        }
        if (active) draw_btn_chars(0, r, sy);       // Mario stands on the light of the active button
        else if (g_btn_chars >= 0) draw_btn_chars(g_btn_chars, r, sy);
    } else {
        u32 fill = active ? COL_PANEL_ACT : (sel ? COL_PANEL_SEL : COL_PANEL);
        u32 border = sel ? sel_color() : COL_BORDER;
        if (!pressed) C2D_DrawRectSolid(o.x, o.y + 3, 0.5f, o.w, o.h, C2D_Color32(0, 0, 0, 0x70));
        C2D_DrawRectSolid(r.x - 2, r.y - 2, 0.5f, r.w + 4, r.h + 4, border);
        C2D_DrawRectSolid(r.x, r.y, 0.5f, r.w, r.h, fill);
    }

    u32 saved = g_shadow;
    g_shadow = SHADOW;                      // buttons are dark/blue: always a black shadow
    if (g_btn_glass) {                      // MK7 menu text: white with a dark outline
        g_shadow = C2D_Color32(0x18, 0x1C, 0x28, 0xFF);
        g_text_outline = true;
    }
    float sc = g_btn_sc;
    bool green = active && g_btns && C2D_SpriteSheetCount(g_btns) > 4;
    u32 tcol = !enabled ? BTN_DIS : (active && !green ? BTN_ACT : BTN_TEXT);
    float w, h;
    text_size(label, sc, &w, &h);
    if (!center) {                          // shrink a long label so it never hits the switch/value
        float room = r.w - 28 - g_btn_reserve - g_btn_indent;
        if (w > room && w > 0) { sc *= room / w; text_size(label, sc, &w, &h); }
    }
    float ty = r.y + (r.h - h) / 2;
    if (center) draw_text(label, r.x + r.w / 2 - w / 2, ty, sc, tcol);
    else draw_text(label, r.x + 14 + g_btn_indent, ty, sc, tcol);

    if (value && value[0]) {
        sc = g_btn_sc;
        text_size(value, sc, &w, &h);
        if (green)                              // centered in the green part
            draw_text(value, r.x + r.w * ACTIVE_CX - w / 2, r.y + r.h * BLUE_CY - h / 2, sc, BTN_TEXT);
        else
            draw_text(value, r.x + r.w - 14 - w, r.y + (r.h - h) / 2, sc,
                      active ? BTN_ACT : (sel ? BTN_TEXT : BTN_VALUE));
    }
    g_shadow = saved;
    g_text_outline = false;
}

// Mark to the left of a centered button label; the label itself does not move.
static void draw_label_mark(C2D_SpriteSheet sheet, int idx, Rect r, const char *label, bool sel, float k) {
    if (!sheet) return;
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, idx);
    float tw, th;
    text_size(label, g_btn_sc, &tw, &th);
    float mw = im.subtex->width * k, mh = im.subtex->height * k;
    C2D_DrawImageAt(im, r.x + r.w / 2 - tw / 2 - mw - 5,
                    r.y + (r.h - mh) / 2 + (btn_pressed(r, sel, true) ? PRESS_DY : 0),
                    0.5f, NULL, k, k);
}

// Back button: tab with the arrow and B in the bottom-left corner.
static void draw_back_tab(Rect r, bool sel, bool enabled) {
    if (!g_back) return;
    bool pressed = btn_pressed(r, sel, enabled);
    C2D_Image im = C2D_SpriteSheetGetImage(g_back, 0);
    C2D_ImageTint tint;
    if (!enabled) C2D_PlainImageTint(&tint, C2D_Color32(0x90, 0x90, 0x90, 0xFF), 0.6f);
    else if (pressed) C2D_PlainImageTint(&tint, C2D_Color32(0, 0, 0, 0xFF), 0.25f);
    else if (sel) C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xE0, 0x10, 0xFF), 0.4f);
    C2D_DrawImageAt(im, r.x, r.y, 0.5f, (sel || pressed || !enabled) ? &tint : NULL, 1.0f, 1.0f);
}

// Toggle switch drawn at the right end of a settings row.
// anim: 0 = OFF .. 1 = ON (animated knob position). ON glows green, OFF glows red.
#define SW_W 40.0f
#define SW_H 16.0f

static void draw_pill(float x, float y, float w, float h, u32 col) {
    float r = h / 2;
    C2D_DrawRectSolid(x + r, y, 0.5f, w - h, h, col);
    C2D_DrawCircleSolid(x + r, y + r, 0.5f, r, col);
    C2D_DrawCircleSolid(x + w - r, y + r, 0.5f, r, col);
}

static void draw_switch(Rect row, bool sel, float anim, bool on) {
    if (btn_pressed(row, sel, true)) row.y += PRESS_DY;
    float x = row.x + row.w - 14 - SW_W;
    float y = row.y + (row.h - SW_H) / 2;

    // colors: blend red -> green with the knob position
    float t = anim;
    u8 cr = (u8)(0xE8 + (0x28 - 0xE8) * t), cg = (u8)(0x30 + (0xD0 - 0x30) * t), cb = (u8)(0x30 + (0x50 - 0x30) * t);
    float pulse = 0.55f + 0.45f * sinf((float)(osGetTime() % 100000) / 260.0f);   // soft glow

    // glow (two soft layers) + track
    draw_pill(x - 4, y - 4, SW_W + 8, SW_H + 8, C2D_Color32(cr, cg, cb, (u8)(0x38 * pulse)));
    draw_pill(x - 2, y - 2, SW_W + 4, SW_H + 4, C2D_Color32(cr, cg, cb, (u8)(0x70 * pulse)));
    draw_pill(x - 1, y - 1, SW_W + 2, SW_H + 2, C2D_Color32(0x10, 0x10, 0x14, 0xFF));
    draw_pill(x, y, SW_W, SW_H, C2D_Color32(cr, cg, cb, 0xFF));

    // ON / OFF text on the free side
    u32 saved = g_shadow;
    g_shadow = SHADOW;
    const char *txt = on ? "ON" : "OFF";
    float tw, th;
    text_size(txt, 0.36f, &tw, &th);
    float tx = on ? x + 5 : x + SW_W - 5 - tw;
    draw_text(txt, tx, y + (SW_H - th) / 2, 0.36f, BTN_TEXT);
    g_shadow = saved;

    // knob
    float r = SW_H / 2 + 1;
    float kx = x + SW_H / 2 + (SW_W - SW_H) * anim;
    C2D_DrawCircleSolid(kx + 1, y + SW_H / 2 + 1.5f, 0.5f, r, C2D_Color32(0, 0, 0, 0x80));
    C2D_DrawCircleSolid(kx, y + SW_H / 2, 0.5f, r, C2D_Color32(0xF4, 0xF4, 0xF8, 0xFF));
    C2D_DrawCircleSolid(kx, y + SW_H / 2, 0.5f, r - 3, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
}

static void draw_round_rect(float x, float y, float w, float h, float rr, u32 c) {
    C2D_DrawRectSolid(x + rr, y, 0.5f, w - 2 * rr, h, c);
    C2D_DrawRectSolid(x, y + rr, 0.5f, w, h - 2 * rr, c);
    C2D_DrawCircleSolid(x + rr, y + rr, 0.5f, rr, c);
    C2D_DrawCircleSolid(x + w - rr, y + rr, 0.5f, rr, c);
    C2D_DrawCircleSolid(x + rr, y + h - rr, 0.5f, rr, c);
    C2D_DrawCircleSolid(x + w - rr, y + h - rr, 0.5f, rr, c);
}

// Name plate of an Extra item: flat, with no shadow, so it does not look pressable.
static void draw_name_plate(Rect r, const char *txt, u32 fill, u32 edge, u32 ink) {
    const float h = 28, y = r.y + (r.h - h) / 2;
    draw_round_rect(r.x, y, r.w, h, 5, edge);
    draw_round_rect(r.x + 1, y + 1, r.w - 2, h - 2, 4, fill);
    C2D_DrawRectSolid(r.x + 7, y + 6, 0.5f, 3, h - 12, edge);         // accent bar

    u32 saved = g_shadow;
    g_shadow = 0;
    float sc = 0.58f, tw, th, room = r.w - 26;
    text_size(txt, sc, &tw, &th);
    if (tw > room) { sc *= room / tw; text_size(txt, sc, &tw, &th); }
    draw_text(txt, r.x + 16, y + (h - th) / 2, sc, ink);
    g_shadow = saved;
}

// Name plate that is also a button: its own color, with the glass light of the other buttons
// on top, and it sinks while pressed.
static void draw_plate_button(Rect r, const char *txt, u32 fill, u32 edge, u32 ink, bool sel) {
    bool pressed = btn_pressed(r, sel, true);
    if (pressed) r.y += PRESS_DY;
    draw_name_plate(r, txt, fill, edge, ink);
    const float h = 28, y = r.y + (r.h - h) / 2;
    // light row by row, following the rounded corners of the plate; kept faint so the color shows
    const int rows = (int)h - 2;
    const float rad = 4, x0 = r.x + 1, w0 = r.w - 2;
    for (int j = 0; j < rows; j++) {
        float d = j < rad ? rad - (j + 0.5f) : (j >= rows - rad ? (j + 0.5f) - (rows - rad) : 0);
        float in = d > 0 ? rad - sqrtf(rad * rad - d * d) : 0;
        float t = (float)j / rows;
        u32 c;
        if (j < 2) c = C2D_Color32(0xFF, 0xFF, 0xFF, 0x50);                       // top highlight
        else if (t < 0.45f) c = C2D_Color32(0xFF, 0xFF, 0xFF, (u8)(0x20 * (1 - t / 0.45f)));
        else if (j == rows - 1) c = C2D_Color32(0x28, 0x2A, 0x30, 0x70);            // bottom edge
        else if (t > 0.62f) c = C2D_Color32(0, 0, 0, (u8)(0x34 * (t - 0.62f) / 0.38f));
        else continue;
        C2D_DrawRectSolid(x0 + in, y + 1 + j, 0.5f, w0 - in * 2, 1, c);
    }
    if (pressed) draw_round_rect(r.x + 1, y + 1, r.w - 2, h - 2, 4, C2D_Color32(0, 0, 0, 0x40));
}

// ---- glyph colors of the round icons ----
typedef struct { u32 rim, ring_t, ring_b, face_t, face_b, face_mid, glyph; } Metal;
static Metal metal_gold(void) {
    Metal m = {
        C2D_Color32(0x5A, 0x3C, 0x06, 0xFF),   // dark outline
        C2D_Color32(0xFF, 0xEC, 0x98, 0xFF),   // ring top (bright)
        C2D_Color32(0xA8, 0x70, 0x0C, 0xFF),   // ring bottom (deep)
        C2D_Color32(0xC0, 0x84, 0x14, 0xFF),   // face top
        C2D_Color32(0xFF, 0xD6, 0x5A, 0xFF),   // face bottom
        C2D_Color32(0xE6, 0xB0, 0x38, 0xFF),   // face middle (gear hole etc.)
        C2D_Color32(0x5E, 0x36, 0x00, 0xFF),   // glyph
    };
    return m;
}
static Metal metal_silver(void) {
    Metal m = {
        C2D_Color32(0x3A, 0x3E, 0x46, 0xFF),
        C2D_Color32(0xFC, 0xFC, 0xFF, 0xFF),
        C2D_Color32(0x7C, 0x82, 0x8E, 0xFF),
        C2D_Color32(0x98, 0x9E, 0xAA, 0xFF),
        C2D_Color32(0xEC, 0xEE, 0xF2, 0xFF),
        C2D_Color32(0xC4, 0xC8, 0xD0, 0xFF),
        C2D_Color32(0x30, 0x34, 0x3E, 0xFF),
    };
    return m;
}

// ---- MK7 cup-icon lens (Extra and Settings) ----
#define LENS_W 62.0f               // width of the lens image

static void draw_lens_img(int idx, float cx, float cy, float k) {
    if (!g_lens) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_lens, idx);
    C2D_DrawImageAt(im, cx - im.subtex->width * k / 2, cy - im.subtex->height * k / 2, 0.5f, NULL, k, k);
}

// Lens background, drawn under the glyph.
static void draw_lens_base(Rect r, Rect hitr, bool sel, float *ocx, float *ocy, float *oR, bool *opressed) {
    bool pressed = btn_pressed(hitr, sel, true);
    float cx = r.x + r.w / 2, cy = r.y + r.h / 2, R = r.w / 2;
    if (pressed) { cy += PRESS_DY; R -= 1.0f; }
    draw_lens_img(sel ? 2 : 0, cx, cy, R * 2 / LENS_W);
    *ocx = cx; *ocy = cy; *oR = R; *opressed = pressed;
}

// Glass and ring, drawn over the glyph.
static void draw_lens_top(float cx, float cy, float R, bool sel, bool pressed) {
    float k = R * 2 / LENS_W;
    if (sel) { draw_lens_img(4, cx, cy, k); draw_lens_img(3, cx, cy, k); }
    else draw_lens_img(1, cx, cy, k);
    if (pressed) C2D_DrawCircleSolid(cx, cy, 0.5f, R, C2D_Color32(0, 0, 0, 0x40));
}

// Round "Ex" button (Extra menu)
static void draw_ex_button(Rect r, Rect hitr, bool sel) {
    Metal m = metal_gold();
    float cx, cy, R; bool pressed;
    draw_lens_base(r, hitr, sel, &cx, &cy, &R, &pressed);
    u32 saved = g_shadow;
    g_shadow = C2D_Color32(0xFF, 0xF4, 0xC8, 0x90);     // light emboss under the letters
    float w, h;
    text_size("Ex", 0.62f, &w, &h);
    draw_text("Ex", cx - w / 2 - 0.5f, cy - h / 2, 0.62f, m.glyph);
    g_shadow = saved;
    draw_lens_top(cx, cy, R, sel, pressed);
}

// Small round "!" note button
static void draw_note_button(Rect r, Rect hitr, bool sel) {
    float cx, cy, R; bool pressed;
    draw_lens_base(r, hitr, sel, &cx, &cy, &R, &pressed);
    u32 saved = g_shadow;
    g_shadow = 0;
    float w, h;
    text_size("!", 0.6f, &w, &h);
    draw_text("!", cx - w / 2, cy - h / 2, 0.6f, C2D_Color32(0x16, 0x46, 0xB0, 0xFF));
    g_shadow = saved;
    draw_lens_top(cx, cy, R, sel, pressed);
}

// Round gear button (settings)
static void draw_gear_button(Rect r, Rect hitr, bool sel) {
    Metal m = metal_silver();
    float cx, cy, R; bool pressed;
    draw_lens_base(r, hitr, sel, &cx, &cy, &R, &pressed);

    const float ri = R * 0.42f, ro = R * 0.70f;
    for (int pass = 0; pass < 2; pass++) {       // pass 0: light emboss, pass 1: glyph
        u32 g = pass == 0 ? C2D_Color32(0xFF, 0xFF, 0xFF, 0x90) : m.glyph;
        float oy = pass == 0 ? 1.0f : 0.0f;
        for (int k = 0; k < 8; k++) {
            float a = k * (float)M_PI / 4.0f;
            float x1 = cx + cosf(a - 0.32f) * ri, y1 = cy + oy + sinf(a - 0.32f) * ri;
            float x2 = cx + cosf(a + 0.32f) * ri, y2 = cy + oy + sinf(a + 0.32f) * ri;
            float x3 = cx + cosf(a + 0.20f) * ro, y3 = cy + oy + sinf(a + 0.20f) * ro;
            float x4 = cx + cosf(a - 0.20f) * ro, y4 = cy + oy + sinf(a - 0.20f) * ro;
            C2D_DrawTriangle(x1, y1, g, x2, y2, g, x3, y3, g, 0.5f);
            C2D_DrawTriangle(x1, y1, g, x3, y3, g, x4, y4, g, 0.5f);
        }
        C2D_DrawCircleSolid(cx, cy + oy, 0.5f, R * 0.50f, g);
    }
    C2D_DrawCircleSolid(cx, cy, 0.5f, R * 0.20f, m.face_mid);
    draw_lens_top(cx, cy, R, sel, pressed);
}

// Round "rename folder" button: a folder with A -> B inside
static void draw_folder_button(Rect r, Rect hitr, bool sel) {
    float cx, cy, R; bool pressed;
    draw_lens_base(r, hitr, sel, &cx, &cy, &R, &pressed);
    float k = R / 16.0f, w = 22 * k, h = 15 * k, x = cx - w / 2, y = cy - h / 2 + 1 * k;
    u32 edge = C2D_Color32(0x6A, 0x4A, 0x10, 0xFF), body = C2D_Color32(0xF4, 0xC8, 0x48, 0xFF);
    C2D_DrawRectSolid(x, y - 3 * k, 0.5f, 9 * k, 4 * k, edge);          // tab
    C2D_DrawRectSolid(x + 1, y - 2 * k, 0.5f, 7 * k, 3 * k, body);
    C2D_DrawRectSolid(x, y, 0.5f, w, h, edge);
    C2D_DrawRectSolid(x + 1, y + 1, 0.5f, w - 2, h - 2, body);
    u32 saved = g_shadow;
    g_shadow = 0;
    float sc = 0.32f * k, tw, th, my = y + h / 2;
    u32 ink = C2D_Color32(0x40, 0x28, 0x00, 0xFF), arrow = C2D_Color32(0x16, 0x46, 0xB0, 0xFF);
    text_size("A", sc, &tw, &th);
    draw_text("A", x + 2 * k, my - th / 2, sc, ink);
    draw_text("B", x + w - 2 * k - tw, my - th / 2, sc, ink);
    C2D_DrawRectSolid(cx - 2.5f * k, my - 0.5f * k, 0.5f, 3 * k, 1 * k + 0.5f, arrow);
    C2D_DrawTriangle(cx + 0.5f * k, my - 2 * k, arrow, cx + 0.5f * k, my + 2.5f * k, arrow,
                     cx + 3 * k, my + 0.25f * k, arrow, 0.5f);
    g_shadow = saved;
    draw_lens_top(cx, cy, R, sel, pressed);
}

// Silver spanner icon, centered at cx,cy
static void draw_spanner(float cx, float cy) {
    if (!g_spanner) return;
    C2D_Image im = C2D_SpriteSheetGetImage(g_spanner, 0);
    C2D_DrawImageAt(im, cx - im.subtex->width / 2.0f, cy - im.subtex->height / 2.0f, 0.5f, NULL, 1.0f, 1.0f);
}

// White speech bubble next to an icon (pointer at cx,y; bubble below it, or above if up).
static float g_tip_sc = 0.5f;    // text size of the next speech bubble

static void draw_tooltip(float cx, float y, const char *txt, bool up) {
    float sc = g_tip_sc, w, h;
    text_size(txt, sc, &w, &h);
    float bw = w + 14, bh = h + 6, rr = 5;
    float bx = cx - bw / 2;
    if (bx < 3) bx = 3;
    if (bx + bw > 317) bx = 317 - bw;
    float by = up ? y - 5 - bh : y + 5;
    float bedge = up ? by + bh - 0.5f : by + 0.5f;     // edge of the bubble the pointer leaves from
    u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xF4), sh = C2D_Color32(0, 0, 0, 0x60);

    for (int pass = 0; pass < 2; pass++) {       // pass 0: shadow, pass 1: bubble
        float o = pass == 0 ? 2.0f : 0.0f;
        u32 c = pass == 0 ? sh : white;
        C2D_DrawRectSolid(bx + rr + o, by + o, 0.5f, bw - 2 * rr, bh, c);
        C2D_DrawRectSolid(bx + o, by + rr + o, 0.5f, bw, bh - 2 * rr, c);
        C2D_DrawCircleSolid(bx + rr + o, by + rr + o, 0.5f, rr, c);
        C2D_DrawCircleSolid(bx + bw - rr + o, by + rr + o, 0.5f, rr, c);
        C2D_DrawCircleSolid(bx + rr + o, by + bh - rr + o, 0.5f, rr, c);
        C2D_DrawCircleSolid(bx + bw - rr + o, by + bh - rr + o, 0.5f, rr, c);
        C2D_DrawTriangle(cx - 5 + o, bedge + o, c, cx + 5 + o, bedge + o, c, cx + o, y + o, c, 0.5f);
    }
    u32 saved = g_shadow;
    g_shadow = 0;
    draw_text(txt, bx + (bw - w) / 2, by + (bh - h) / 2, sc, C2D_Color32(0x20, 0x20, 0x28, 0xFF));
    g_shadow = saved;
}

// ---- status bar (top screen): clock, Wi-Fi, battery ----
static bool g_ac = false, g_ptm = false, g_mcu = false;
static int  g_wifi_bars = 0;        // 0..3
static bool g_wifi_on = false;
static int  g_batt = -1;            // percent, -1 = unknown
static bool g_batt_exact = false;   // false: estimated from 0..5 level
static bool g_charging = false;

static void status_init(void) {
    g_ac  = R_SUCCEEDED(acInit());
    g_ptm = R_SUCCEEDED(ptmuInit());
    g_mcu = R_SUCCEEDED(mcuHwcInit());
}
// Notification LED (MCU register 0x2D). Needs mcu::HWC, so it may do nothing for a .3dsx.
typedef struct {
    u8 delay, smoothing, loop_delay, unused;
    u8 r[32], g[32], b[32];
} LedPattern;

static bool g_led_enabled = true;               // "Notification light" setting
static u8   g_led_rgb[3] = { 0x10, 0xB0, 0xFF };   // color for the next light
static u32  g_led_last = 0;                      // what the LED shows now (0 = off)
static u64  g_led_t0 = 0;                        // start of the flashes (0 = none)
static int  g_led_count = 0;                     // 1 or 2 flashes

static void led_set(bool on) {
    if (!g_mcu) return;
    if (!g_led_enabled) on = false;
    u32 key = on ? 0x1000000u | g_led_rgb[0] << 16 | g_led_rgb[1] << 8 | g_led_rgb[2] : 0;
    if (key == g_led_last) return;
    LedPattern p;
    memset(&p, 0, sizeof p);
    if (on) {
        p.delay = 0x20; p.smoothing = 0x10; p.loop_delay = 0xFF;
        memset(p.r, g_led_rgb[0], 32); memset(p.g, g_led_rgb[1], 32); memset(p.b, g_led_rgb[2], 32);
    }
    if (R_SUCCEEDED(MCUHWC_WriteRegister(0x2D, &p, sizeof p))) g_led_last = key;
}

// LED color from a hue, fully saturated
static void led_hue(int h) {
    float hh = (h % 360) / 60.0f, x = 1 - fabsf(fmodf(hh, 2.0f) - 1), c[3] = {0, 0, 0};
    if (hh < 1) { c[0] = 1; c[1] = x; } else if (hh < 2) { c[0] = x; c[1] = 1; } else if (hh < 3) { c[1] = 1; c[2] = x; }
    else if (hh < 4) { c[1] = x; c[2] = 1; } else if (hh < 5) { c[0] = x; c[2] = 1; } else { c[0] = 1; c[2] = x; }
    for (int i = 0; i < 3; i++) g_led_rgb[i] = (u8)(c[i] * 0xFF);
}

// Updates found: two 1-second light-blue flashes with a short gap.
static void led_blink(void) {                    // paler than the blue of SpotPass
    g_led_rgb[0] = 0x60; g_led_rgb[1] = 0xD8; g_led_rgb[2] = 0xFF;
    g_led_t0 = osGetTime(); g_led_count = 2;
}

// Folder switched: one 1-second flash in its management color.
static void led_flash_hue(int h) { led_hue(h); g_led_t0 = osGetTime(); g_led_count = 1; }

// Important notice (red-framed window): one 1-second flash in orange.
static void led_flash_alert(void) {
    g_led_rgb[0] = 0xFF; g_led_rgb[1] = 0x50; g_led_rgb[2] = 0x00;
    g_led_t0 = osGetTime(); g_led_count = 1;
}

// Download / install finished: one 1-second flash in deep green (darker than StreetPass green).
static void led_flash_done(void) {
    g_led_rgb[0] = 0x00; g_led_rgb[1] = 0x58; g_led_rgb[2] = 0x14;
    g_led_t0 = osGetTime(); g_led_count = 1;
}

static void led_update(void) {
    if (!g_led_t0) return;
    u64 t = osGetTime() - g_led_t0;
    u64 end = g_led_count == 2 ? 2500 : 1000;
    if (t >= end) { led_set(false); g_led_t0 = 0; return; }
    led_set(t < 1000 || (t >= 1500 && t < 2500));
}

static void status_exit(void) {
    led_set(false);                              // never leave the light on
    if (g_mcu) mcuHwcExit();
    if (g_ptm) ptmuExit();
    if (g_ac)  acExit();
}
static void status_poll(void) {
    u32 ws = 0;
    g_wifi_on = g_ac && R_SUCCEEDED(ACU_GetWifiStatus(&ws)) && ws != 0;
    g_wifi_bars = g_wifi_on ? osGetWifiStrength() : 0;
    if (g_wifi_bars > 3) g_wifi_bars = 3;

    u8 v = 0;
    g_batt_exact = false;
    if (g_mcu && R_SUCCEEDED(MCUHWC_GetBatteryLevel(&v))) { g_batt = v > 100 ? 100 : v; g_batt_exact = true; }
    else if (g_ptm && R_SUCCEEDED(PTMU_GetBatteryLevel(&v))) g_batt = v * 20;   // 0..5 -> %
    u8 ch = 0;
    g_charging = g_ptm && R_SUCCEEDED(PTMU_GetBatteryChargeState(&ch)) && ch;
}

// ring segment of a fan (angles in radians, screen coords: up = -pi/2)
static void draw_fan(float cx, float cy, float ri, float ro, u32 c) {
    const float a0 = -3.0f * (float)M_PI / 4.0f, a1 = -(float)M_PI / 4.0f;
    const int N = 8;
    for (int i = 0; i < N; i++) {
        float t0 = a0 + (a1 - a0) * i / N, t1 = a0 + (a1 - a0) * (i + 1) / N;
        float x0i = cx + cosf(t0) * ri, y0i = cy + sinf(t0) * ri, x1i = cx + cosf(t1) * ri, y1i = cy + sinf(t1) * ri;
        float x0o = cx + cosf(t0) * ro, y0o = cy + sinf(t0) * ro, x1o = cx + cosf(t1) * ro, y1o = cy + sinf(t1) * ro;
        if (ri <= 0.01f) { C2D_DrawTriangle(cx, cy, c, x0o, y0o, c, x1o, y1o, c, 0.5f); continue; }
        C2D_DrawTriangle(x0i, y0i, c, x0o, y0o, c, x1o, y1o, c, 0.5f);
        C2D_DrawTriangle(x0i, y0i, c, x1o, y1o, c, x1i, y1i, c, 0.5f);
    }
}

static void draw_wifi(float cx, float cy) {           // cx,cy = bottom point of the fan
    static const float RI[4] = { 0, 5.5f, 10.0f, 14.5f }, RO[4] = { 3.5f, 8.0f, 12.5f, 17.0f };
    u32 off = (COL_GRAY & 0x00FFFFFF) | 0x50000000;
    for (int pass = 0; pass < 2; pass++) {             // pass 0: shadow
        for (int k = 0; k < 4; k++) {
            bool lit = g_wifi_on && (k == 0 || k <= g_wifi_bars);
            u32 c = pass == 0 ? (lit ? g_shadow : 0) : (lit ? COL_WHITE : off);
            if (!c) continue;
            draw_fan(cx + (pass == 0 ? 1 : 0), cy + (pass == 0 ? 1 : 0), RI[k], RO[k], c);
        }
    }
}

// battery icon (w x h) at x,y
static void draw_battery(float x, float y) {
    const float w = 22, h = 11;
    u32 edge = COL_WHITE;
    if (g_shadow) {
        C2D_DrawRectSolid(x + 1, y + 1, 0.5f, w, h, g_shadow);
        C2D_DrawRectSolid(x + w + 1, y + 4, 0.5f, 2, h - 6, g_shadow);
    }
    C2D_DrawRectSolid(x, y, 0.5f, w, h, edge);                         // outline
    C2D_DrawRectSolid(x + 1.5f, y + 1.5f, 0.5f, w - 3, h - 3, C2D_Color32(0x18, 0x18, 0x20, 0xFF));
    C2D_DrawRectSolid(x + w, y + 3.5f, 0.5f, 2, h - 7, edge);           // nub
    int p = g_batt < 0 ? 0 : g_batt;
    u32 fill = p < 20 ? C2D_Color32(0xF0, 0x40, 0x30, 0xFF) : C2D_Color32(0x40, 0xD0, 0x50, 0xFF);
    if (g_charging) fill = C2D_Color32(0x40, 0xD0, 0x50, 0xFF);
    C2D_DrawRectSolid(x + 2.5f, y + 2.5f, 0.5f, (w - 5) * p / 100.0f, h - 5, fill);
    if (g_charging) {                                                   // lightning bolt
        u32 b = C2D_Color32(0xFF, 0xE8, 0x30, 0xFF);
        float cx = x + w / 2, cy = y + h / 2;
        C2D_DrawTriangle(cx + 1, cy - 5, b, cx - 4, cy + 1, b, cx + 0.5f, cy + 0.5f, b, 0.5f);
        C2D_DrawTriangle(cx - 1, cy + 5, b, cx + 4, cy - 1, b, cx - 0.5f, cy - 0.5f, b, 0.5f);
    }
}

static void draw_status_bar(const Strings *S, bool clock24) {
    // clock (left) - the ":" blinks with the seconds (on for the first half of each second)
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);                 // 3DS RTC already holds local time
    static int last_sec = -1;
    static u64 sec_start = 0;
    if (tm && tm->tm_sec != last_sec) { last_sec = tm->tm_sec; sec_start = osGetTime(); }
    bool colon_on = (osGetTime() - sec_start) < 500;

    char pre[24] = "--", post[24] = "--";         // text before / after the ":"
    if (tm) {
        if (clock24) {
            snprintf(pre, sizeof pre, "%02d", tm->tm_hour);
            snprintf(post, sizeof post, "%02d", tm->tm_min);
        } else {
            int h = tm->tm_hour % 12;
            if (h == 0) h = 12;
            const char *ap = tm->tm_hour < 12 ? S->am : S->pm;
            if (S == &STR_JA) {
                snprintf(pre, sizeof pre, "%s %d", ap, h);
                snprintf(post, sizeof post, "%02d", tm->tm_min);
            } else {
                snprintf(pre, sizeof pre, "%d", h);
                snprintf(post, sizeof post, "%02d %s", tm->tm_min, ap);
            }
        }
    }
    draw_wifi(20, 21);                            // Wi-Fi fan, left of the clock
    float cw, chh, pw, ph;
    text_size(pre, 0.55f, &pw, &ph);
    text_size(":", 0.55f, &cw, &chh);
    float x = 38;
    draw_text(pre, x, 4, 0.55f, COL_WHITE);
    x += pw;
    if (colon_on) draw_text(":", x, 4, 0.55f, COL_WHITE);
    x += cw;
    draw_text(post, x, 4, 0.55f, COL_WHITE);

    // battery % (right) <- battery icon
    char pct[16];
    if (g_batt < 0) snprintf(pct, sizeof pct, "--%%");
    else snprintf(pct, sizeof pct, g_batt_exact ? "%d%%" : "~%d%%", g_batt);
    float w, h;
    text_size(pct, 0.5f, &w, &h);
    float tx = 394 - w;
    draw_text(pct, tx, 5, 0.5f, g_batt >= 0 && g_batt < 20 && !g_charging ? COL_RED : COL_WHITE);
    float bx = tx - 6 - 24;
    draw_battery(bx, 8);
}

static bool hit(Rect r, touchPosition t) {
    return t.px >= r.x && t.px < r.x + r.w && t.py >= r.y && t.py < r.y + r.h;
}

// ------------------------------------------------------------ UI layout (bottom screen 320x240)

// Confirmation window. Reset and launch always ask; only DLG_SWITCH follows the "confirm" setting.
enum { DLG_SWITCH = 0, DLG_RESET, DLG_LAUNCH, DLG_FOLD_SAVE, DLG_FOLD_RESET, DLG_RESTART };

typedef enum { SCR_MAIN, SCR_SETTINGS, SCR_ABOUT, SCR_APPUPD, SCR_EXTRA, SCR_FOLDERS, SCR_BGCOLOR } Screen;

#define MAIN_N 7
static const Rect MAIN_R[MAIN_N] = {
    {36,  42, 248, 42},   // Characters1
    {36,  92, 248, 42},   // Characters2
    {36, 142, 248, 42},   // Characters3
    {60, 200, 200, 32},   // Launch CTGP-7 (separated)
    {283,  4,  32, 32},   // Settings (round gear, top right)
    { 14, 202, 32, 32},   // Extra (round "Ex", bottom left)
    {  5,  4,  32, 32},   // Custom folders (round, top left)
};
static const Rect GEAR_HIT = {272, 0, 48, 44};   // larger touch area for the gear
static const Rect EX_HIT   = { 0, 196, 56, 44};
static const Rect FOLDER_HIT = { 0, 0, 48, 44};

// Custom folders screen
static const Rect FOLD_NAME_R[SLOTS] = { {10, 40, 150, 34}, {10, 88, 150, 34}, {10, 136, 150, 34} };
static const Rect FOLD_BTN_R[SLOTS]  = { {166, 40, 144, 34}, {166, 88, 144, 34}, {166, 136, 144, 34} };
static const Rect FOLD_SAVE  = { 90, 205, 130, 30};   // also used by the background color screen
#define BG_BAR_X 30.0f
#define BG_BAR_W 260.0f
#define BG_BAR_Y 60.0f
#define BG_BAR_H 26.0f
#define BG_SEEK_Y (BG_BAR_Y + BG_BAR_H + 23)
#define BG_HAND_TIP 8.0f                              // x of the fingertip in hand_point.png
static const Rect BG_TOUCH = {16, 54, 288, 76};       // touch area of the bar and the seek bar
static const Rect BG_RANDOM = {44, 136, 112, 30};
static const Rect BG_RAINBOW = {164, 136, 112, 30};
static const Rect BG_RANDOM_C = {104, 136, 112, 30};  // random alone (folder colors)
#define RAINBOW_MS   30000                            // one full loop

static int rainbow_hue_now(void) {
    return (int)((osGetTime() % RAINBOW_MS) * 360 / RAINBOW_MS);
}
#define FOLD_MARIO_H 28.0f                             // drawn height of Mario
#define FOLD_MARIO_W 36.0f                             // drawn width of Mario

// Touch area of Mario on row i: from where he is drawn to the right edge of the screen.
static Rect fold_mario_hit(int i) {
    Rect r = { FOLD_BTN_R[i].x + FOLD_BTN_R[i].w - 6 - FOLD_MARIO_W, FOLD_BTN_R[i].y, 0, FOLD_BTN_R[i].h };
    r.w = 320 - r.x;
    return r;
}

static const Rect FOLD_NOTE  = {288,   6,  26, 26};   // round "!" (name rules)
static const Rect FOLD_NOTE_HIT = {272, 0, 48, 38};
static const Rect FOLD_RESET = {226, 205,  84, 30};

// Extra screen
static const Rect EX_NAME_R[EX_ITEMS] = { {48, 60, 144, 34}, {48, 138, 144, 34} };
static const Rect EX_NOTE_R[EX_ITEMS] = { {12, 64, 26, 26}, {12, 142, 26, 26} };   // round "!" buttons
static const Rect EX_NOTE_HIT[EX_ITEMS] = { {4, 56, 42, 42}, {4, 134, 42, 42} };
static const Rect EX_DL_R[EX_ITEMS]   = { {200, 60, 106, 34}, {200, 138, 106, 34} };

// Settings: a scrollable list (LIST_N rows) + a fixed Back button (index LIST_N)
#define LIST_N       14
#define SET_N        (LIST_N + 1)
#define LIST_Y       28
#define LIST_H       170
#define ROW_PITCH    32
#define ROW_H        28
#define SET_RESTART  11             // "Restart app": only for the .cia, hidden for a .3dsx
#define LIST_CONTENT (set_rows() * ROW_PITCH)
#define SB_X         300
#define THUMB_H      ((LIST_H * LIST_H) / LIST_CONTENT)

static const Rect BACK_R = {0, 204, 84, 36};       // back tab, bottom-left corner

static bool g_hide_restart = false;

static int set_rows(void) { return LIST_N - (g_hide_restart ? 1 : 0); }
static int set_pos(int i)  { return i - (g_hide_restart && i > SET_RESTART ? 1 : 0); }   // item -> row
static int set_item(int p) { return p + (g_hide_restart && p >= SET_RESTART ? 1 : 0); }  // row -> item

static Rect row_rect(int i, int scroll) {
    Rect r = {12, (float)(LIST_Y + set_pos(i) * ROW_PITCH - scroll), 280, ROW_H};
    return r;
}

static const Rect ABOUT_BACK = {0, 204, 84, 36};
static const Rect ABOUT_NOTE = {282, 208, 26, 26};  // round "!" (AI note)
static const Rect ABOUT_NOTE_HIT = {272, 198, 48, 42};
static const Rect APPUPD_BTN = {40,  56, 240, 32};   // check for updates
static const Rect INSTALL_BTN = {30, 150, 260, 34};   // download & install / restart / exit
static const Rect INSTALL_TOP = {30, 158, 260, 32};   // same button when "update both" is shown
static const Rect BOTH_BTN    = {40,  92, 240, 28};   // update both formats (under "check for updates")
static const Rect APP_YES     = {38, 150, 116, 34};   // startup app-update dialog
static const Rect APP_NO      = {166, 150, 116, 34};

static const Rect DLG_BOX = { 30, 64, 260, 112};
static const Rect DLG_YES = { 48, 124, 100, 36};
static const Rect DLG_NO  = {172, 124, 100, 36};
static const Rect UPD_BOX = { 12,  46, 296, 150};
static const Rect DLG_OK  = {110, 150, 100, 34};

// Background of a window, including its frame around the box (inverted in dark mode).
static void draw_window(Rect b) {
    if (!g_window) {
        C2D_DrawRectSolid(b.x - 2, b.y - 2, 0.5f, b.w + 4, b.h + 4, COL_YELLOW);
        C2D_DrawRectSolid(b.x, b.y, 0.5f, b.w, b.h, COL_BG);
        return;
    }
    C2D_Image im = C2D_SpriteSheetGetImage(g_window, b.w > 280 ? 0 : 1);
    C2D_DrawImageAt(im, b.x - 2, b.y - 2, 0.5f, NULL,
                    (b.w + 4) / im.subtex->width, (b.h + 4) / im.subtex->height);
}

static const Strings *get_strings(const Config *c, bool sys_ja) {
    if (c->lang == 1) return &STR_JA;
    if (c->lang == 2) return &STR_EN;
    return sys_ja ? &STR_JA : &STR_EN;
}

// ------------------------------------------------------------ main

int main(int argc, char **argv) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    cfguInit();

    C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget *bot = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    g_buf = C2D_TextBufNew(8192);
    g_font = C2D_FontLoadSystem(CFG_REGION_JPN);   // kana/kanji + Latin
    colors_init();
    g_romfs = R_SUCCEEDED(romfsInit());
    g_top_bg = C2D_SpriteSheetLoad("romfs:/gfx/info_bg.t3x");
    g_bot_bg = C2D_SpriteSheetLoad("romfs:/gfx/upd_bg.t3x");
    g_btns   = C2D_SpriteSheetLoad("romfs:/gfx/buttons.t3x");
    g_logo   = C2D_SpriteSheetLoad("romfs:/gfx/logo.t3x");
    g_kart   = C2D_SpriteSheetLoad("romfs:/gfx/kart.t3x");
    g_shell  = C2D_SpriteSheetLoad("romfs:/gfx/shell.t3x");
    invert_sheet(g_shell);                 // inverted in light mode, original colors in dark mode
    g_bowser = C2D_SpriteSheetLoad("romfs:/gfx/bowser.t3x");
    g_lakitu = C2D_SpriteSheetLoad("romfs:/gfx/lakitu.t3x");
    g_chars  = C2D_SpriteSheetLoad("romfs:/gfx/chars.t3x");
    g_lens   = C2D_SpriteSheetLoad("romfs:/gfx/lens.t3x");
    g_back   = C2D_SpriteSheetLoad("romfs:/gfx/back.t3x");
    g_spanner = C2D_SpriteSheetLoad("romfs:/gfx/spanner.t3x");
    g_parts  = C2D_SpriteSheetLoad("romfs:/gfx/particles.t3x");
    g_window = C2D_SpriteSheetLoad("romfs:/gfx/window.t3x");
    g_hands  = C2D_SpriteSheetLoad("romfs:/gfx/hands.t3x");
    g_trash  = C2D_SpriteSheetLoad("romfs:/gfx/trash.t3x");
    g_icons  = C2D_SpriteSheetLoad("romfs:/gfx/icons.t3x");
    g_seek   = C2D_SpriteSheetLoad("romfs:/gfx/seek.t3x");

    bool sys_ja = false;
    u8 lang = CFG_LANGUAGE_EN;
    if (R_SUCCEEDED(CFGU_GetSystemLanguage(&lang)) && lang == CFG_LANGUAGE_JP) sys_ja = true;

    migrate_cfg_dir();

    // which format is running? (.3dsx from the Homebrew Launcher, or an installed .cia)
    g_upd.is_cia = !envIsHomebrew();
    g_hide_restart = !g_upd.is_cia;
    if (argc > 0 && argv && argv[0] && !strncmp(argv[0], "sdmc:/", 6) && strstr(argv[0], ".3dsx"))
        snprintf(g_upd.target, sizeof g_upd.target, "%s", argv[0]);
    else
        snprintf(g_upd.target, sizeof g_upd.target, "%s", DEFAULT_3DSX_PATH);
    Config cfg;
    config_load(&cfg);
    g_led_enabled = cfg.led;
    names_load();
    colors_load();
    apply_theme(cfg.dark);
    set_bg_dark(cfg.dark);
    g_sfx_on = cfg.sfx;
    sfx_init();
    hidSetRepeatParameters(24, 6);         // D-pad repeat: after 0.4 s, every 0.1 s
    status_init();
    status_poll();
    int notice = 0;                        // shown notice: 0 none, otherwise its notice_queue bit
    int notice_queue = 0;                  // bit 1: Wi-Fi, bit 2: battery <20%, bit 4: battery <=5%, bit 8: no CTGP-7
    bool batt_warned = false, batt_crit_warned = false;
    u64 next_poll = osGetTime() + 1000;
    int frame_no = 0;
    bool manual_check = false;             // "Check for updates" pressed (busy sound)
    int slide_screen = -1;                 // screen whose slide-in has started
    bool was_rolling = false, was_at_end = false;  // previous frame: seek bar held / at its end
    bool was_idle = false;                 // previous frame: engine idling sound on
    bool was_upd_done = false;             // previous frame: update finished
    int was_ex_busy = -1;                  // previous frame: Extra item being downloaded
    bool pending_both = false;             // "update both" pressed: starts when the check ends
    bool no_exit = false;                  // updating / downloading / checking: START and HOME are ignored
    u64 next_busy = 0;

    char pack_ver[64];
    load_pack_version(pack_ver, sizeof pack_ver);

    ensure_slots();
    ErrCode err = ERR_NONE;
    int active = detect_active(&err);
    if (active < 0) active = 0;

    Screen screen = SCR_MAIN;
    int cursor = 0;                        // main: 0-2 slots, 3 settings
    if (active > 0) cursor = active - 1;
    int set_cursor = 0;                    // settings: 0..LIST_N (LIST_N = Back)
    int scroll = 0;                        // settings list scroll (pixels)
    int t_mode = 0, t_last = 0;            // touch: 0 none, 1 list, 2 scrollbar, 3 back
    bool t_drag = false;
    touchPosition t_start = {0, 0};
    int ex_cursor = 0;                     // extra screen: 0 MK7 DL, 1 ACNL DL, 2 back, 3 MK7 note, 4 ACNL note
    int fold_cursor = 0;                   // custom folders: 0-2 rename, 3 save, 4 reset, 5 back, 6 "!", 7-9 color
    char ed_names[SLOTS][NAME_BUF];        // names being edited (saved only by "Save changes")
    int bg_target = -1;                    // color screen: -1 background, 0-2 folder management color
    int bg_cursor = 0;                     // background color: 0 seek bar, 1 save, 2 reset, 3 back, 4 random, 5 rainbow
    bool ed_rainbow = false;               // rainbow while choosing
    int ed_hue = -1;                       // hue being chosen (shown live, saved only by "Save")
    bool bg_saved = false, t_seek = false; // show "Saved." / seek bar is being dragged
    bool mario_talk = false;               // custom folders: Mario's bubble, touch only
    int fold_msg = 0;                      // 0 none, 1 saved, 2 bad name, 3 name in use, 4 rename failed
    int about_cursor = 0;                  // about screen: 0 back, 1 "!" note
    int note_dialog = -1;                  // note window: Extra item, NOTE_AI, NOTE_NAMES (-1 = none)
    int appupd_cursor = 0;                 // app update screen: 0 check, 1 install, 2 back, 3 update both

    bool dialog = false;
    int dlg_target = 0;
    int dlg_kind = DLG_SWITCH;             // what "Yes" does
    bool dlg_yes = true;

    MsgKind msg_kind = MSG_NONE;
    int msg_arg = 0;

    bool running = true;
    bool quit_pending = false;             // exit after the current sound finishes
    int quit_frames = 0;
    int quit_limit = 150;                  // longest wait for the sound, in frames (~2.5 s)

    // ---- pack update check (background) ----
    bool upd_dialog = false;
    char latest[64] = "";
    bool chk_pending = false;
    bool app_dialog = false, app_yes = true, app_pending = false;
    if (cfg.check_update && cfg.pack_url[0] && !strstr(cfg.pack_url, "OWNER"))
        chk_pending = check_start(&g_chk, cfg.pack_url);
    if (cfg.check_update)
        app_pending = check_start(&g_app, APP_URL_DEFAULT);

    // ---- actions (as small lambdas via nested blocks would be GNU-only; use inline code) ----
    while (aptMainLoop() && running) {
        hidScanInput();
        u32 down = hidKeysDown();
        {   // hold the D-pad to keep moving the cursor (left/right only outside the settings
            // list, where they change values)
            u32 rmask = KEY_UP | KEY_DOWN;
            if (screen != SCR_SETTINGS) rmask |= KEY_LEFT | KEY_RIGHT;
            down |= hidKeysDownRepeat() & rmask;
        }
        u32 held = hidKeysHeld();
        u32 up   = hidKeysUp();
        touchPosition tp;
        hidTouchRead(&tp);
        if (quit_pending) {
            if (!sfx_busy() || ++quit_frames > quit_limit) break;
            down = held = up = 0;
        }
        bool tap = (down & KEY_TOUCH) != 0;
        g_held = held;
        g_touch = tp;

        if ((down & KEY_START) && !no_exit) break;

        const Strings *S = get_strings(&cfg, sys_ja);

        // ---------- status polling / notices ----------
        frame_no++;
        // while files are being installed, everything that is not needed is paused
        bool installing = g_upd.state == UPD_INSTALLING;
        if (!installing && osGetTime() >= next_poll) { status_poll(); next_poll = osGetTime() + 1000; }
        if (frame_no == 45 && !g_wifi_on) notice_queue |= 1;          // ~0.75 s after start
        if (frame_no == 45 && !exists("sdmc:/CTGP-7")) notice_queue |= 8;
        if (g_batt >= 0 && g_batt <= 5 && !g_charging) {          // 5% or less: second warning
            if (!batt_crit_warned) {
                notice_queue |= 4;
                notice_queue &= ~2;                                 // skip the 20% one if still waiting
                batt_crit_warned = batt_warned = true;
            }
        } else if (g_batt >= 0 && g_batt < 20 && !g_charging) {
            if (!batt_warned) { notice_queue |= 2; batt_warned = true; }
        }
        if (g_batt > 5)   batt_crit_warned = false;                 // re-arm after charging
        if (g_batt >= 20) batt_warned = false;

        // state snapshot (used to pick the sound effect for this frame)
        Screen  pv_screen = screen;
        int     pv_cur = cursor, pv_set = set_cursor, pv_app = appupd_cursor, pv_ex = ex_cursor;
        int     pv_about = about_cursor, pv_fold = fold_cursor, pv_bg = bg_cursor;
        bool    pv_yes = dlg_yes;
        bool    pv_appdlg = app_dialog, pv_appyes = app_yes;
        (void)pv_appdlg;
        bool    pv_note = note_dialog >= 0;
        // which window is on screen (input priority order)
        #define VIS_NOW (note_dialog >= 0 ? 1 : notice ? 2 : upd_dialog ? 3 : app_dialog ? 4 : dialog ? 5 : 0)
        int     pv_vis = VIS_NOW;
        (void)pv_note;
        bool    ok_sfx = false;
        bool    ok_plain = false;   // force the normal OK sound
        int     force_se = -1;

        led_update();
        {                                              // folder color screen: the LED shows the color
            static bool holding = false;
            bool hold = screen == SCR_BGCOLOR && bg_target >= 0 && ed_hue >= 0;
            if (hold) { led_hue(ed_hue); led_set(true); g_led_t0 = 0; }
            else if (holding) led_set(false);
            holding = hold;
        }

        // ---------- pack update check result ----------
        if (chk_pending) {
            if (g_chk.state == CHK_DONE) {
                chk_pending = false;
                load_pack_version(pack_ver, sizeof pack_ver);
                if (has_digit(pack_ver) && has_digit(g_chk.remote) &&
                    version_cmp(g_chk.remote, pack_ver) > 0) {
                    snprintf(latest, sizeof latest, "%s", g_chk.remote);
                    upd_dialog = true;
                    led_blink();                     // pack update found
                }
            } else if (g_chk.state == CHK_FAIL) {
                chk_pending = false;     // offline etc.: stay silent
            }
        }

        if (app_pending && g_app.state != CHK_RUNNING) {
            app_pending = false;
            if (g_app.state == CHK_DONE && has_digit(g_app.remote) &&
                version_cmp(g_app.remote, APP_VERSION) > 0) {
                app_dialog = true;
                app_yes = true;
                led_blink();                         // app update found
            }
        }

        // ---------- input ----------
        int do_slot = 0;          // slot to request a switch to (1-3), 0 = none
        bool do_perform = false;
        bool do_launch = false;
        bool do_appcheck = false;
        int do_rename = -1;       // folder to open the keyboard for

        if (!notice && notice_queue && note_dialog < 0 && !dialog) {
            notice = (notice_queue & 8) ? 8 : (notice_queue & 1) ? 1 : (notice_queue & 2) ? 2 : 4;   // no CTGP-7 first
            led_flash_alert();
            notice_queue &= ~notice;
        }

        // Windows: positive and negative answers have their own sound, on top of the closing sound
        bool ok_btn = (down & KEY_A) || (tap && hit(DLG_OK, tp));
        if (note_dialog >= 0 || notice || upd_dialog) {
            if (ok_btn) sfx_play(SE_WIN_YES);
            else if (down & KEY_B) sfx_play(SE_WIN_NO);
        }

        if (note_dialog >= 0) {
            if (ok_btn || (down & KEY_B)) note_dialog = -1;
        } else if (notice) {
            if (ok_btn || (down & KEY_B)) notice = 0;
        } else if (upd_dialog) {
            if (ok_btn || (down & KEY_B)) upd_dialog = false;
        } else if (app_dialog) {
            if (down & (KEY_LEFT | KEY_RIGHT)) app_yes = !app_yes;
            bool go = false, close = false;
            if (down & KEY_B) close = true;
            if (down & KEY_A) { if (app_yes) go = true; else close = true; }
            if (tap) {
                if (hit(APP_YES, tp)) go = true;
                else if (hit(APP_NO, tp)) close = true;
            }
            if (go) {
                app_dialog = false;
                screen = SCR_APPUPD;
                appupd_cursor = 1;              // cursor on "Download and install"
                sfx_play(SE_WIN_YES);
            } else if (close) {
                app_dialog = false;
                sfx_play(SE_WIN_NO);
            }
        } else if (dialog) {
            if (down & (KEY_LEFT | KEY_RIGHT)) dlg_yes = !dlg_yes;
            if (down & KEY_B) dialog = false;
            if (down & KEY_A) {
                if (dlg_yes) do_perform = true;
                dialog = false;
            }
            if (tap) {
                if (hit(DLG_YES, tp)) { do_perform = true; dialog = false; }
                else if (hit(DLG_NO, tp)) dialog = false;
            }
            if (!dialog && !(do_perform && dlg_kind == DLG_LAUNCH))      // launching has its own sounds
                sfx_play(do_perform ? SE_WIN_YES : SE_WIN_NO);
        } else if (screen == SCR_MAIN) {
            int act = -1;
            // order: Folders(6) -> 1 -> 2 -> 3 -> Launch -> Ex(5) -> Gear(4) -> Folders ...; left/right on the icons
            static const int order[MAIN_N] = { 6, 0, 1, 2, 3, 5, 4 };
            int pos = 0;
            for (int i = 0; i < MAIN_N; i++) if (order[i] == cursor) pos = i;
            if (down & KEY_UP)   cursor = order[(pos + MAIN_N - 1) % MAIN_N];
            if (down & KEY_DOWN) cursor = order[(pos + 1) % MAIN_N];
            if ((down & KEY_LEFT) && cursor == 4) cursor = 6;
            if ((down & KEY_RIGHT) && cursor == 6) cursor = 4;
            if ((down & KEY_LEFT) && cursor == 3) cursor = 5;
            if ((down & KEY_RIGHT) && cursor == 5) cursor = 3;
            if (down & KEY_A) act = cursor;
            // shoulder buttons: first press moves the cursor to the icon, the next one opens it
            if (down & KEY_R) { if (cursor == 4) act = 4; else cursor = 4; }
            if (down & KEY_L) { if (cursor == 6) act = 6; else cursor = 6; }
            if (tap) {
                for (int i = 0; i < MAIN_N; i++) {
                    Rect hr = i == 4 ? GEAR_HIT : (i == 5 ? EX_HIT : (i == 6 ? FOLDER_HIT : MAIN_R[i]));
                    if (hit(hr, tp)) { cursor = i; act = i; break; }
                }
            }
            if (act >= 0) ok_sfx = true;
            if (act >= 0 && act < 3) do_slot = act + 1;
            else if (act == 3) { dialog = true; dlg_kind = DLG_LAUNCH; dlg_yes = true; }
            else if (act == 4) {                    // cursor and scroll stay where they were left
                screen = SCR_SETTINGS;
                if (set_cursor < LIST_N) {          // the list may have been dragged away from the cursor
                    int ctop = set_pos(set_cursor) * ROW_PITCH;
                    if (ctop < scroll) scroll = ctop;
                    if (ctop + ROW_H > scroll + LIST_H) scroll = ctop + ROW_H - LIST_H;
                }
            }
            else if (act == 5) { screen = SCR_EXTRA; ex_cursor = 0; ex_fetch_versions(); }
            else if (act == 6) {
                screen = SCR_FOLDERS;
                fold_cursor = 0;
                fold_msg = 0;
                mario_talk = false;
                memcpy(ed_names, g_names, sizeof ed_names);
            }
        } else if (screen == SCR_BGCOLOR) {
            bool fold_mode = bg_target >= 0;
            int bg_back = fold_mode ? SCR_FOLDERS : SCR_SETTINGS;
            if (fold_mode && bg_cursor == 5) bg_cursor = 4;            // no rainbow for folders
            bool mid_row = bg_cursor == 4 || bg_cursor == 5;
            if (down & KEY_UP)   bg_cursor = bg_cursor == 0 ? 1 : mid_row ? 0 : 4;
            if (down & KEY_DOWN) bg_cursor = bg_cursor == 0 ? 4 : mid_row ? 1 : 0;
            if (bg_cursor == 0 && (held & (KEY_LEFT | KEY_RIGHT))) {       // hold to keep sliding
                int h = (ed_hue < 0 ? 0 : ed_hue) + ((held & KEY_RIGHT) ? 1 : -1);
                ed_hue = h < 0 ? 0 : h > 359 ? 359 : h;
                ed_rainbow = false;
                bg_saved = false;
            } else {
                if (down & KEY_LEFT)  { if (bg_cursor == 1) bg_cursor = 3; else if (bg_cursor == 2) bg_cursor = 1; }
                if (down & KEY_RIGHT) { if (bg_cursor == 3) bg_cursor = 1; else if (bg_cursor == 1) bg_cursor = 2; }
                if (down & KEY_LEFT)  { if (bg_cursor == 5) bg_cursor = 4; }
                if (down & KEY_RIGHT) { if (bg_cursor == 4 && !fold_mode) bg_cursor = 5; }
            }
            int act = -1;
            if (down & KEY_A) act = bg_cursor;
            if (down & KEY_B) screen = bg_back;             // the color goes back to the saved one
            if (tap) {
                if (hit(BG_TOUCH, tp)) { t_seek = true; bg_cursor = 0; }
                else if (hit(FOLD_SAVE, tp)) act = 1;
                else if (hit(FOLD_RESET, tp)) act = 2;
                else if (hit(ABOUT_BACK, tp)) act = 3;
                else if (hit(fold_mode ? BG_RANDOM_C : BG_RANDOM, tp)) act = 4;
                else if (!fold_mode && hit(BG_RAINBOW, tp)) act = 5;
                if (act >= 0) bg_cursor = act;
            }
            if (t_seek && (held & KEY_TOUCH)) {
                float f = ((float)tp.px - BG_BAR_X) / BG_BAR_W;
                ed_hue = (int)((f < 0 ? 0 : f > 1 ? 1 : f) * 359);
                ed_rainbow = false;
                bg_saved = false;
            }
            if (up & KEY_TOUCH) t_seek = false;
            if (act == 1 && fold_mode) {
                ok_sfx = true;
                g_fold_hue[bg_target] = ed_hue;
                colors_save();
                bg_saved = true;
            } else if (act == 1) {
                ok_sfx = true;
                cfg.bg_hue = ed_hue;
                cfg.bg_rainbow = ed_rainbow;
                config_save(&cfg);
                bg_saved = true;
            } else if (act == 2) {
                ok_sfx = true;
                ed_hue = fold_mode ? FOLD_HUE_DEF[bg_target] : -1;
                ed_rainbow = false;
                bg_saved = false;
            } else if (act == 4) {
                ok_sfx = true;
                ed_hue = rand_range(0, 359);
                ed_rainbow = false;
                bg_saved = false;
            } else if (act == 5) {
                ok_sfx = true;
                ed_rainbow = true;
                bg_saved = false;
            } else if (act == 3) screen = bg_back;
        } else if (screen == SCR_FOLDERS) {
            bool bottom = fold_cursor >= 3 && fold_cursor <= 5;
            bool top_btn = fold_cursor == 6;
            bool plate = fold_cursor >= 7;                // name plates: 7-9 (management color)
            if (down & KEY_UP) {
                if (plate) fold_cursor = fold_cursor == 7 ? 6 : fold_cursor - 1;
                else fold_cursor = top_btn ? 3 : bottom ? 2 : (fold_cursor == 0 ? 6 : fold_cursor - 1);
            }
            if (down & KEY_DOWN) {
                if (plate) fold_cursor = fold_cursor == 9 ? 3 : fold_cursor + 1;
                else fold_cursor = top_btn ? 0 : bottom ? 6 : fold_cursor + 1;
            }
            if (down & KEY_LEFT)  { if (fold_cursor == 3) fold_cursor = 5; else if (fold_cursor == 4) fold_cursor = 3;
                                    else if (fold_cursor < SLOTS) fold_cursor += 7; }
            if (down & KEY_RIGHT) { if (fold_cursor == 5) fold_cursor = 3; else if (fold_cursor == 3) fold_cursor = 4;
                                    else if (plate) fold_cursor -= 7; }
            int act = -1;
            if (down & KEY_A) act = fold_cursor;
            if (down & KEY_B) screen = SCR_MAIN;          // unsaved edits are dropped
            bool mario_tap = tap && active > 0 && hit(fold_mario_hit(active - 1), tp);
            if (mario_tap) {                              // Mario is no button: the cursor stays away
                mario_talk = !mario_talk;
                force_se = mario_talk ? SE_DLG_OPEN : SE_DLG_CLOSE;
            } else if (tap) {
                for (int i = 0; i < SLOTS; i++) if (hit(FOLD_BTN_R[i], tp)) act = i;
                if (hit(FOLD_SAVE, tp)) act = 3;
                if (hit(FOLD_RESET, tp)) act = 4;
                if (hit(ABOUT_BACK, tp)) act = 5;
                if (hit(FOLD_NOTE_HIT, tp)) act = 6;
                for (int i = 0; i < SLOTS; i++) if (hit(FOLD_NAME_R[i], tp)) act = 7 + i;
                if (act >= 0) fold_cursor = act;
            }
            if (act >= 0 && act < SLOTS) { ok_sfx = true; do_rename = act; }
            else if (act == 3) {
                sfx_play(SE_OK);                    // with the window opening sound
                dialog = true; dlg_kind = DLG_FOLD_SAVE; dlg_yes = true;
            } else if (act == 4) {
                sfx_play(SE_OK);
                dialog = true; dlg_kind = DLG_FOLD_RESET; dlg_yes = false;
            } else if (act == 5) screen = SCR_MAIN;
            else if (act == 6) note_dialog = NOTE_NAMES;
            else if (act >= 7) {                          // management color: the color screen, for one folder
                ok_sfx = true;
                screen = SCR_BGCOLOR;
                bg_target = act - 7;
                bg_cursor = 0;
                ed_hue = g_fold_hue[bg_target];
                ed_rainbow = false;
                bg_saved = false;
                t_seek = false;
            }
        } else if (screen == SCR_EXTRA) {
            if (!ex_busy()) {
                // two columns: note (3,4) | download (0,1); back (2) is shared
                static const int col_dl[3] = { 0, 1, 2 }, col_note[3] = { 3, 4, 2 };
                const int *col = (ex_cursor == 3 || ex_cursor == 4) ? col_note : col_dl;
                int pos = 0;
                for (int i = 0; i < 3; i++) if (col[i] == ex_cursor) pos = i;
                if (down & KEY_UP)   ex_cursor = col[(pos + 2) % 3];
                if (down & KEY_DOWN) ex_cursor = col[(pos + 1) % 3];
                if (down & KEY_LEFT)  { if (ex_cursor == 0) ex_cursor = 3; else if (ex_cursor == 1) ex_cursor = 4; }
                if (down & KEY_RIGHT) { if (ex_cursor == 3) ex_cursor = 0; else if (ex_cursor == 4) ex_cursor = 1; }
                if (down & KEY_B) screen = SCR_MAIN;
                int act = -1;
                if (down & KEY_A) act = ex_cursor;
                if (tap) {
                    for (int i = 0; i < EX_ITEMS; i++) {
                        if (hit(EX_DL_R[i], tp)) act = i;
                        if (hit(EX_NOTE_HIT[i], tp)) act = 3 + i;
                    }
                    if (hit(ABOUT_BACK, tp)) act = 2;
                    if (act >= 0) ex_cursor = act;
                }
                if (act >= 0 && act < EX_ITEMS) { ok_sfx = true; ex_start(act); }
                else if (act == 3 || act == 4) note_dialog = act - 3;
                else if (act == 2) screen = SCR_MAIN;
            }
        } else if (screen == SCR_ABOUT) {
            if (down & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) about_cursor = !about_cursor;
            int act = -1;
            if (down & KEY_A) act = about_cursor;
            if (down & KEY_B) act = 0;
            if (tap) {
                if (hit(ABOUT_BACK, tp)) act = 0;
                else if (hit(ABOUT_NOTE_HIT, tp)) act = 1;
                if (act >= 0) about_cursor = act;
            }
            if (act == 0) screen = SCR_SETTINGS;
            else if (act == 1) note_dialog = NOTE_AI;
        } else if (screen == SCR_APPUPD) {
            // buttons: 0 check, 1 install/restart/exit (only when shown), 3 update both (only when shown), 2 back
            bool newer = g_app.state == CHK_DONE && has_digit(g_app.remote) &&
                         version_cmp(g_app.remote, APP_VERSION) > 0;
            bool busy = upd_busy();
            bool show_mid = (newer && (g_upd.state == UPD_IDLE || g_upd.state == UPD_FAIL)) ||
                            g_upd.state == UPD_DONE;
            bool show_both = upd_can_both();
            if (!busy) {
                if (!show_both && appupd_cursor == 3) appupd_cursor = show_mid ? 1 : 0;
                if (!show_mid && appupd_cursor == 1) appupd_cursor = 0;
                int ord[4], ord_n = 0, ord_pos = 0;
                ord[ord_n++] = 0;
                if (show_both) ord[ord_n++] = 3;
                if (show_mid) ord[ord_n++] = 1;
                ord[ord_n++] = 2;
                for (int i = 0; i < ord_n; i++) if (ord[i] == appupd_cursor) ord_pos = i;
                if (down & KEY_UP)   appupd_cursor = ord[(ord_pos + ord_n - 1) % ord_n];
                if (down & KEY_DOWN) appupd_cursor = ord[(ord_pos + 1) % ord_n];
                if (down & KEY_B) screen = SCR_SETTINGS;
                int act = -1;
                if (down & KEY_A) act = appupd_cursor;
                if (tap) {
                    if (hit(APPUPD_BTN, tp)) act = 0;
                    else if (show_mid && hit(show_both ? INSTALL_TOP : INSTALL_BTN, tp)) act = 1;
                    else if (show_both && hit(BOTH_BTN, tp)) act = 3;
                    else if (hit(ABOUT_BACK, tp)) act = 2;
                    if (act >= 0) appupd_cursor = act;
                }
                if (act == 0 && g_upd.state != UPD_DONE) {
                    do_appcheck = true; ok_sfx = true;
                    if (g_upd.state == UPD_FAIL) g_upd.state = UPD_IDLE;
                } else if (act == 1 && show_mid) {
                    ok_sfx = true;
                    if (g_upd.state == UPD_DONE) {
                        if (g_upd.is_cia) {             // relaunch the freshly installed title
                            u64 tid = 0;
                            if (R_SUCCEEDED(APT_GetProgramID(&tid)) && tid)
                                aptSetChainloader(tid, MEDIATYPE_SD);
                        }
                        quit_pending = true;            // .3dsx: back to the Homebrew Launcher
                    } else {
                        upd_start(false, false);
                    }
                } else if (act == 3 && show_both) {
                    ok_sfx = true;
                    if (g_upd.state == UPD_FAIL) g_upd.state = UPD_IDLE;
                    if (g_app.state == CHK_DONE && has_digit(g_app.remote)) {
                        upd_start(true, !newer);
                    } else {                        // not checked yet: find out what is needed first
                        do_appcheck = true;
                        pending_both = true;
                    }
                } else if (act == 2) {
                    screen = SCR_SETTINGS;
                }
            }
        } else { // SCR_SETTINGS
            int act = -1, dir = 0;
            bool follow = false;
            const int max_scroll = LIST_CONTENT - LIST_H;
            if (down & KEY_UP)    { set_cursor = (set_cursor + SET_N - 1) % SET_N; follow = true; }
            if (down & KEY_DOWN)  { set_cursor = (set_cursor + 1) % SET_N; follow = true; }
            if (g_hide_restart && set_cursor == SET_RESTART)                 // step over the hidden row
                set_cursor = (down & KEY_UP) ? SET_RESTART - 1 : SET_RESTART + 1;
            if (down & KEY_LEFT)  { dir = -1; act = set_cursor; }
            if (down & KEY_RIGHT) { dir = +1; act = set_cursor; }
            if (down & KEY_A)     { dir = +1; act = set_cursor; }
            if (down & KEY_B)     screen = SCR_MAIN;

            // touch: tap a row, drag the list, or drag the scroll bar
            if (down & KEY_TOUCH) {
                t_start = tp;
                t_last = tp.py;
                t_drag = false;
                t_mode = 0;
                if (hit(BACK_R, tp)) t_mode = 3;
                else if (tp.py >= LIST_Y && tp.py < LIST_Y + LIST_H)
                    t_mode = (tp.px >= SB_X - 6) ? 2 : 1;
            }
            if ((held & KEY_TOUCH) && t_mode == 2) {
                float f = (float)((int)tp.py - LIST_Y - THUMB_H / 2) / (float)(LIST_H - THUMB_H);
                scroll = (int)(f * max_scroll);
            } else if ((held & KEY_TOUCH) && t_mode == 1) {
                if (abs((int)tp.py - (int)t_start.py) > 6) t_drag = true;
                if (t_drag) {
                    scroll -= (int)tp.py - t_last;
                    t_last = tp.py;
                }
            }
            if (up & KEY_TOUCH) {
                if (t_mode == 3 && hit(BACK_R, t_start)) {
                    set_cursor = LIST_N; act = LIST_N; dir = +1;
                } else if (t_mode == 1 && !t_drag) {
                    int y = (int)t_start.py - LIST_Y + scroll;
                    int row = y / ROW_PITCH;
                    if (row >= 0 && row < set_rows() && (y % ROW_PITCH) < ROW_H) {
                        row = set_item(row);
                        set_cursor = row; act = row; dir = +1;
                    }
                }
                t_mode = 0;
            }

            if (follow && set_cursor < LIST_N) {
                int top = set_pos(set_cursor) * ROW_PITCH;
                if (top < scroll) scroll = top;
                if (top + ROW_H > scroll + LIST_H) scroll = top + ROW_H - LIST_H;
            }
            if (scroll < 0) scroll = 0;
            if (scroll > max_scroll) scroll = max_scroll;

            // while the list is dragged or scrolled the cursor stays on a row that can be seen
            if ((t_mode == 1 && t_drag) || t_mode == 2) {
                int first = (scroll + ROW_PITCH - 1) / ROW_PITCH;
                int last = (scroll + LIST_H - ROW_H) / ROW_PITCH;
                if (set_cursor < LIST_N) {
                    int pos = set_pos(set_cursor);
                    if (pos < first) set_cursor = set_item(first);
                    else if (pos > last) set_cursor = set_item(last);
                }
                pv_set = set_cursor;                     // no cursor sound: the seek bar has its own
            }

            if (act >= 0 && !(down & KEY_B)) {
                bool changed = false;
                bool lr = (down & (KEY_LEFT | KEY_RIGHT)) != 0;
                switch (act) {
                case 0:
                    cfg.lang = (cfg.lang + 3 + dir) % 3;
                    changed = true;
                    break;
                case 1: cfg.clock24 = !cfg.clock24; changed = true; break;
                case 2: cfg.dark = !cfg.dark; changed = true; break;
                case 3: cfg.sfx = !cfg.sfx; changed = true; break;
                case 4: cfg.bgm = !cfg.bgm; changed = true; break;
                case 5: cfg.led = !cfg.led; changed = true; break;
                case 6: cfg.confirm = !cfg.confirm; changed = true; break;
                case 7: cfg.exit_after = !cfg.exit_after; changed = true; break;
                case 8: cfg.check_update = !cfg.check_update; changed = true; break;
                case 9:
                    if (!lr) {
                        screen = SCR_BGCOLOR;
                        bg_target = -1;
                        bg_cursor = 0;
                        ed_hue = cfg.bg_hue;
                        ed_rainbow = cfg.bg_rainbow != 0;
                        bg_saved = false;
                        t_seek = false;
                    }
                    break;
                case 10:
                    if (!lr) { screen = SCR_APPUPD; appupd_cursor = 0; }
                    break;
                case 11:                                // restart (.cia only)
                    if (!lr) { dialog = true; dlg_kind = DLG_RESTART; dlg_yes = true; }
                    break;
                case 12:
                    if (!lr) { dialog = true; dlg_kind = DLG_RESET; dlg_yes = false; }
                    break;
                case 13:
                    if (!lr) {
                        load_pack_version(pack_ver, sizeof pack_ver);
                        screen = SCR_ABOUT;
                        about_cursor = 0;
                    }
                    break;
                case 14:
                    if (!lr) screen = SCR_MAIN;
                    break;
                }
                if (act < LIST_N) ok_sfx = true;
                if (act >= 9 && act <= 13) ok_plain = true;   // background color / app update / restart / reset / about
                if (changed) {
                    config_save(&cfg);
                    apply_theme(cfg.dark);
                    set_bg_dark(cfg.dark);
                    g_sfx_on = cfg.sfx;
                    g_led_enabled = cfg.led;
                    if (!cfg.led) led_set(false);
                }
            }
        }

        if (screen == SCR_APPUPD && pv_screen != SCR_APPUPD) upd_detect_other();

        // ---------- switch request ----------
        if (do_slot && err == ERR_NONE) {
            if (do_slot == active) {
                msg_kind = MSG_ALREADY; msg_arg = do_slot;
            } else if (cfg.confirm) {
                dialog = true; dlg_kind = DLG_SWITCH; dlg_target = do_slot; dlg_yes = true;
            } else {
                dlg_kind = DLG_SWITCH; dlg_target = do_slot; do_perform = true;
            }
        }
        if (do_perform && dlg_kind == DLG_RESET) {
            config_defaults(&cfg);
            config_save(&cfg);
            apply_theme(cfg.dark);
            set_bg_dark(cfg.dark);
            g_sfx_on = cfg.sfx;
            g_led_enabled = cfg.led;
        } else if (do_perform && dlg_kind == DLG_LAUNCH) {
            do_launch = true;
        } else if (do_perform && dlg_kind == DLG_RESTART) {
            u64 tid = 0;
            if (R_SUCCEEDED(APT_GetProgramID(&tid)) && tid) {
                aptSetChainloader(tid, MEDIATYPE_SD);
                quit_pending = true;
            }
        } else if (do_perform && dlg_kind == DLG_FOLD_SAVE) {
            int r = names_apply(ed_names, active);
            fold_msg = r == 0 ? 1 : r == 1 ? 3 : 4;
            if (r == 0) {
                active = detect_active(&err);
                if (active < 0) active = 0;
            }
        } else if (do_perform && dlg_kind == DLG_FOLD_RESET) {
            names_default(ed_names);
            fold_msg = 0;
        } else if (do_perform) {
            msg_kind = do_switch(active, dlg_target, &msg_arg);
            if (msg_kind == MSG_DONE) led_flash_hue(g_fold_hue[dlg_target - 1]);
            active = detect_active(&err);
            if (active < 0) active = 0;
            if (msg_kind == MSG_DONE && cfg.exit_after) quit_pending = true;
        }

        // ---------- manual app update check ----------
        if (do_appcheck && !upd_busy()) { check_start(&g_app, APP_URL_DEFAULT); manual_check = true; }
        if (manual_check && g_app.state != CHK_RUNNING) manual_check = false;
        if (pending_both && g_app.state != CHK_RUNNING) {
            pending_both = false;
            if (g_app.state == CHK_DONE && has_digit(g_app.remote))
                upd_start(true, version_cmp(g_app.remote, APP_VERSION) <= 0);
        }

        // ---------- launch CTGP-7 ----------
        if (do_launch) {
            if (ctgp7_installed()) {
                aptSetChainloader(CTGP7_TITLE_ID, MEDIATYPE_SD);
                force_se = SE_DECIDE;
                sfx_play(SE_ENGINE_GO);
                quit_pending = true;
                quit_limit = LAUNCH_SOUND_FRAMES;   // starting fast matters more than the sound
            } else {
                msg_kind = MSG_NOLAUNCH;
                msg_arg = 0;
                force_se = SE_CANCEL;
            }
        }

        // ---------- sound effect for this frame ----------
        {
            int se = force_se;
            bool back = (pv_screen == SCR_SETTINGS && screen == SCR_MAIN) ||
                        (pv_screen == SCR_EXTRA && screen == SCR_MAIN) ||
                        (pv_screen == SCR_FOLDERS && screen == SCR_MAIN) || (pv_screen == SCR_BGCOLOR && screen == SCR_FOLDERS) ||
                        ((pv_screen == SCR_ABOUT || pv_screen == SCR_APPUPD || pv_screen == SCR_BGCOLOR) && screen == SCR_SETTINGS);
            int vis = VIS_NOW;
            if (se < 0) {
                if (vis != pv_vis && vis != 0)              // a window appeared
                    se = vis == 2 ? SE_ATTENTION : (vis == 3 || vis == 4) ? SE_FOUND : SE_DLG_OPEN;
                else if (vis == 0 && pv_vis != 0)           // the last window closed
                    se = (pv_vis == 4 && screen == SCR_APPUPD) ? SE_OK : SE_DLG_CLOSE;
                else if (back) se = SE_CANCEL;
                else if (ok_plain) se = SE_OK;
                else if (ok_sfx || screen != pv_screen)
                    se = (pv_screen == SCR_SETTINGS || pv_screen == SCR_APPUPD ||
                          pv_screen == SCR_ABOUT) ? SE_SLOT : SE_OK;
                else if (cursor != pv_cur || set_cursor != pv_set ||
                         appupd_cursor != pv_app || dlg_yes != pv_yes ||
                         app_yes != pv_appyes || ex_cursor != pv_ex || about_cursor != pv_about ||
                         fold_cursor != pv_fold || bg_cursor != pv_bg) se = SE_CURSOR;
            }
            if (se >= 0) sfx_play(se);

            // engine idles while the launch confirmation is open
            bool idle_now = dialog && dlg_kind == DLG_LAUNCH;
            if (idle_now && !was_idle) sfx_play(SE_ENGINE_IDLE);
            else if (!idle_now && was_idle) sfx_stop(SE_ENGINE_IDLE);
            was_idle = idle_now;

            // seek bars: only while moved by hand, so never from the rainbow
            bool rolling = (screen == SCR_BGCOLOR && (t_seek || (bg_cursor == 0 && (held & (KEY_LEFT | KEY_RIGHT))))) ||
                           (screen == SCR_SETTINGS && t_mode == 2 && (held & KEY_TOUCH));
            bool at_end = screen == SCR_BGCOLOR ? (ed_hue == 0 || ed_hue == 359) :
                          screen == SCR_SETTINGS ? (scroll == 0 || scroll == LIST_CONTENT - LIST_H) : false;
            if (rolling && !was_rolling) roll_start();
            else if (!rolling && was_rolling) sfx_stop(SE_ROLL);
            if (rolling && at_end && !was_at_end) sfx_play(SE_ROLLPASS);
            was_rolling = rolling;
            was_at_end = at_end;

            // busy sound: loops back-to-back while checking manually / updating / downloading Extra files
            bool busy_now = upd_busy() || ex_busy() || (manual_check && g_app.state == CHK_RUNNING);
            u64 t = osGetTime();
            if (busy_now) {
                if (!sfx_playing(SE_BUSY)) sfx_play(SE_BUSY);           // restart as soon as it ends
                next_busy = t;                                          // marks "was busy"
            } else if (next_busy) {
                sfx_stop(SE_BUSY);                                      // finished: stop at once
                next_busy = 0;
            }

            // everything finished (for a .cia: installed)
            bool upd_done = g_upd.state == UPD_DONE;
            bool ex_now = ex_busy();
            if ((upd_done && !was_upd_done) ||
                (was_ex_busy >= 0 && !ex_now && g_ex.state[was_ex_busy] == EX_DONE)) { sfx_play(SE_COMPLETE); led_flash_done(); }   // .cia: after installing
            was_upd_done = upd_done;
            was_ex_busy = ex_now ? g_ex.item : -1;
        }
        // leaving would cut off an update, a cheat download or an update check on the App Update screen
        no_exit = upd_busy() || ex_busy() ||
                  (g_app.state == CHK_RUNNING && (manual_check || screen == SCR_APPUPD));
        aptSetHomeAllowed(!no_exit);

        bgm_update(cfg.bgm && !quit_pending && !installing);    // stop BGM while exiting / launching / installing

        // ---------- system keyboard (blocks until closed; only called between frames) ----------
        if (do_rename >= 0) {
            SwkbdState kb;
            char out[NAME_BUF] = "";
            swkbdInit(&kb, SWKBD_TYPE_NORMAL, 2, NAME_CHARS);
            swkbdSetInitialText(&kb, ed_names[do_rename]);
            swkbdSetHintText(&kb, S->rename_hint);      // shown faintly while the field is empty
            swkbdSetValidation(&kb, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
            if (swkbdInputText(&kb, out, sizeof out) == SWKBD_BUTTON_CONFIRM && out[0]) {
                if (name_valid(out)) {
                    snprintf(ed_names[do_rename], NAME_BUF, "%s", out);
                    fold_msg = 0;
                } else {
                    fold_msg = 2;
                }
            }
        }

        // ---------- build strings ----------
        char line[200], msg[256] = "";
        const char *errtxt = "";
        if (err == ERR_NO_ACTIVE) errtxt = S->err_no_active;
        else if (err == ERR_AMBIGUOUS) errtxt = S->err_ambiguous;
        else if (err == ERR_UNEXPECTED) errtxt = S->err_unexpected;

        switch (msg_kind) {
        case MSG_ALREADY: snprintf(msg, sizeof msg, S->already, g_names[msg_arg - 1]); break;
        case MSG_DONE:    snprintf(msg, sizeof msg, S->done, g_names[msg_arg - 1]); break;
        case MSG_FAIL1:   snprintf(msg, sizeof msg, S->fail_step1, g_names[msg_arg - 1]); break;
        case MSG_FAIL2:   snprintf(msg, sizeof msg, S->fail_step2, g_names[msg_arg - 1]); break;
        case MSG_NOLAUNCH: snprintf(msg, sizeof msg, "%s", S->launch_fail); break;
        default: break;
        }

        // ---------- a screen opened: its items slide in ----------
        if ((int)screen != slide_screen) {
            slide_screen = (int)screen;
            g_slide_t0 = osGetTime();
            sfx_play(SE_SCROLL);
        }

        {                                              // live while choosing; the knob follows the rainbow
            int rainbow_hue = rainbow_hue_now();
            if (screen == SCR_BGCOLOR && ed_rainbow) ed_hue = rainbow_hue;
            bg_tint_update(screen == SCR_BGCOLOR && bg_target < 0 ? ed_hue : cfg.bg_rainbow ? rainbow_hue : cfg.bg_hue, cfg.dark);
        }

        // ---------- draw ----------
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(g_buf);

        // top screen
        C2D_TargetClear(top, COL_BG);
        C2D_SceneBegin(top);
        draw_bg(g_top_bg, 400, 240);
        if (!installing) {
            draw_tape(g_top_bg, 400, 240, TOP_TAPE_Y0, TOP_TAPE_Y1, 0, 240);
            kart_update();
            draw_kart();
        }
        draw_status_bar(S, cfg.clock24);
        if (!draw_logo(0, 200, 4)) {
            draw_text_center(APP_NAME_1, 200, 20, 0.85f, COL_YELLOW);
            draw_text_center(APP_NAME_2, 200, 54, 0.85f, COL_YELLOW);
        }
        if (err != ERR_NONE) {
            draw_text(errtxt, 14, 114, 0.55f, COL_RED);
            draw_text(S->err_footer, 14, 172, 0.5f, COL_GRAY);
        } else {
            if (active == 0) snprintf(line, sizeof line, "%s", S->current_none);
            else snprintf(line, sizeof line, S->current, g_names[active - 1]);
            draw_text(line, 14, 114, 0.65f, COL_WHITE);
            if (msg[0]) {
                bool bad = (msg_kind == MSG_FAIL1 || msg_kind == MSG_FAIL2 || msg_kind == MSG_NOLAUNCH);
                if (bad) draw_text(msg, 14, 142, 0.55f, COL_RED);
                else draw_text_light(msg, 14, 142, 0.55f);
            }
        }
        draw_text(S->help, 14, 220, 0.38f, COL_GRAY);
        {
            char pk[128];
            snprintf(pk, sizeof pk, "%s: %s", S->pack_label, pack_ver[0] ? pack_ver : "---");
            draw_text_right(APP_VERSION, 390, 182, 0.5f, COL_GRAY);
            draw_text_right(pk, 390, 200, 0.5f, COL_GRAY);
        }

        // bottom screen
        C2D_TargetClear(bot, COL_BG);
        C2D_SceneBegin(bot);
        draw_bg(g_bot_bg, 320, 240);
        draw_particles(cfg.dark);

        if (screen == SCR_MAIN) {
            draw_text_center(S->select_slot, 160, 8, 0.7f, COL_YELLOW);
            g_btn_sc = 0.72f;
            g_btn_glass = false;                        // only these keep the image buttons
            for (int i = 0; i < SLOTS; i++) {
                snprintf(line, sizeof line, "%s", g_names[i]);
                bool is_act = (active == i + 1);
                g_btn_reserve = is_act ? 100 : 0;       // a long name must not reach "active" and Mario
                draw_button(slid(MAIN_R[i], i + 1), line, is_act ? S->tag_active : "",
                            !dialog && cursor == i, is_act, err == ERR_NONE, false);
            }
            g_btn_reserve = 0;
            g_btn_sc = 0.6f;
            g_btn_chars = 1;
            g_btn_reserve = 46;
            draw_button(slid(MAIN_R[3], 4), S->launch, "", !dialog && cursor == 3, false, true, false);
            g_btn_chars = -1;
            g_btn_reserve = 0;
            g_btn_glass = true;
            draw_gear_button(slid(MAIN_R[4], 0), GEAR_HIT, !dialog && cursor == 4);
            draw_ex_button(slid(MAIN_R[5], 5), EX_HIT, !dialog && cursor == 5);
            draw_folder_button(slid(MAIN_R[6], 0), FOLDER_HIT, !dialog && cursor == 6);
            // L and R marks from the system font (U+E004, U+E005), at the inner top corner of their icons
            draw_text("\xEE\x80\x84", MAIN_R[6].x + MAIN_R[6].w - 2 + slide_dx(0), MAIN_R[6].y - 5, 0.42f, COL_WHITE);
            draw_text_right("\xEE\x80\x85", MAIN_R[4].x + 2 + slide_dx(0), MAIN_R[4].y - 5, 0.42f, COL_WHITE);
            // name bubbles (only for the icon under the cursor)
            if (!dialog && cursor == 4)
                draw_tooltip(MAIN_R[4].x + slide_dx(0) + MAIN_R[4].w / 2, MAIN_R[4].y + MAIN_R[4].h + 3, S->settings, false);
            if (!dialog && cursor == 5)
                draw_tooltip(MAIN_R[5].x + slide_dx(5) + MAIN_R[5].w / 2, MAIN_R[5].y - 3, S->extra_tip, true);
            if (!dialog && cursor == 6)
                draw_tooltip(MAIN_R[6].x + slide_dx(0) + MAIN_R[6].w / 2, MAIN_R[6].y + MAIN_R[6].h + 3, S->rename_tip, false);
        } else if (screen == SCR_BGCOLOR) {
            if (bg_target >= 0) {
                char ttl[96];
                float tw, th, tsc = 0.6f;
                snprintf(ttl, sizeof ttl, S->color_title, ed_names[bg_target]);   // the name as shown in the folder menu
                text_size(ttl, tsc, &tw, &th);
                if (tw > 300) tsc *= 300 / tw;
                draw_text_center(ttl, 160, 8, tsc, COL_YELLOW);
            } else draw_text_center(S->set_bgcolor, 160, 8, 0.65f, COL_YELLOW);
            float pos = BG_BAR_X + (ed_hue < 0 ? 0 : ed_hue) / 359.0f * BG_BAR_W;
            {                                           // color bar: every hue that can be chosen
                float dx = slide_dx(0), step = BG_BAR_W / 36.0f;
                draw_frame(BG_BAR_X + dx - 1, BG_BAR_Y - 1, BG_BAR_W + 2, BG_BAR_H + 2, C2D_Color32(0x30, 0x34, 0x3C, 0xFF));
                for (int i = 0; i < 36; i++) {
                    u32 c0 = hsv_color(i * 10, cfg.dark ? 0.7f : 0.62f, cfg.dark ? 0.7f : 1.0f, 0xFF);
                    u32 c1 = hsv_color((i + 1) % 36 * 10, cfg.dark ? 0.7f : 0.62f, cfg.dark ? 0.7f : 1.0f, 0xFF);
                    C2D_DrawRectangle(BG_BAR_X + dx + i * step, BG_BAR_Y, 0.5f, step, BG_BAR_H, c0, c1, c0, c1);
                }
            }
            {                                           // seek bar; the hand is both knob and marker
                float dx = slide_dx(2);
                bool sel = !dialog && bg_cursor == 0;
                draw_slot_h(BG_BAR_X + dx, BG_SEEK_Y - 1, BG_BAR_W);
                if (g_hands) {
                    bool fist = ed_hue < 0;             // default color: the fist points at nothing
                    C2D_Image hi = C2D_SpriteSheetGetImage(g_hands, fist ? 1 : 0);
                    C2D_ImageTint ht;
                    C2D_PlainImageTint(&ht, sel_color(), 0.4f);
                    float hx = pos + dx - (fist ? hi.subtex->width / 2.0f : BG_HAND_TIP);
                    float hy = fist ? BG_SEEK_Y + 3 - hi.subtex->height / 2.0f : BG_BAR_Y + BG_BAR_H + 1;
                    C2D_DrawImageAt(hi, hx, hy, 0.5f, sel ? &ht : NULL, 1.0f, 1.0f);
                }
            }
            if (bg_saved) draw_text_center(S->saved_msg, 160, 176, 0.55f, COL_GREEN);
            else if (ed_rainbow) draw_text_center(S->rainbow_on, 160, 176, 0.5f, COL_GRAY);
            else if (ed_hue < 0) draw_text_center(S->bg_default, 160, 176, 0.5f, COL_GRAY);
            g_btn_sc = 0.5f;
            draw_button(slid(bg_target >= 0 ? BG_RANDOM_C : BG_RANDOM, 3), S->random_btn, "", !dialog && bg_cursor == 4, false, true, true);
            if (bg_target < 0)
                draw_button(slid(BG_RAINBOW, 3), S->rainbow_btn, "", !dialog && bg_cursor == 5, false, true, true);
            draw_button(slid(FOLD_SAVE, 4), S->save_btn, "", !dialog && bg_cursor == 1, false, true, true);
            draw_label_mark(g_icons, 1, slid(FOLD_SAVE, 4), S->save_btn, !dialog && bg_cursor == 1,
                            S == &STR_EN ? 0.72f : 1.0f);
            draw_button(slid(FOLD_RESET, 4), S->reset_btn, "", !dialog && bg_cursor == 2, false, true, true);
            g_btn_sc = 0.6f;
            draw_back_tab(slid(ABOUT_BACK, 4), !dialog && bg_cursor == 3, true);
        } else if (screen == SCR_FOLDERS) {
            draw_text_center(S->custom_title, 160, 6, 0.75f, COL_YELLOW);
            draw_note_button(slid(FOLD_NOTE, 0), FOLD_NOTE_HIT, note_dialog < 0 && fold_cursor == 6);
            for (int i = 0; i < SLOTS; i++) {
                Rect br = slid(FOLD_BTN_R[i], i);
                static const u8 PLATE[2][SLOTS][6] = {  // fill, edge: red / yellow / green
                    { { 0xF6, 0xCC, 0xCC, 0xC8, 0x60, 0x60 },    // light mode: pale
                      { 0xF8, 0xEE, 0xBC, 0xC4, 0xA4, 0x3C },
                      { 0xCC, 0xEE, 0xCF, 0x58, 0xA8, 0x68 } },
                    { { 0x3A, 0x1E, 0x24, 0x8E, 0x4A, 0x54 },    // dark mode: blackish
                      { 0x3A, 0x32, 0x16, 0x96, 0x82, 0x3C },
                      { 0x1A, 0x32, 0x26, 0x45, 0x86, 0x60 } },
                };
                (void)PLATE;
                bool is_act = active == i + 1;
                Rect nr = slid(FOLD_NAME_R[i], i);
                int fh = g_fold_hue[i];
                if (!dialog && fold_cursor == 7 + i)          // cursor on the plate
                    C2D_DrawRectSolid(nr.x - 3, nr.y + 0, 0.5f, nr.w + 6, nr.h, sel_color());
                draw_plate_button(nr, ed_names[i],
                                  cfg.dark ? hsv_color(fh, 0.48f, 0.23f, 0xFF) : hsv_color(fh, 0.2f, 0.96f, 0xFF),
                                  cfg.dark ? hsv_color(fh, 0.5f, 0.55f, 0xFF) : hsv_color(fh, 0.55f, 0.76f, 0xFF),
                                  COL_WHITE, !dialog && fold_cursor == 7 + i);
                g_btn_sc = 0.5f;
                g_btn_indent = 22;
                g_press_block = is_act && (g_held & KEY_TOUCH) && hit(fold_mario_hit(i), g_touch);   // touching Mario
                draw_button(br, S->rename_btn, "", !dialog && fold_cursor == i, false, true, false);
                g_press_block = false;
                g_btn_indent = 0;
                {
                    float pd = btn_pressed(br, fold_cursor == i, true) ? PRESS_DY : 0;
                    draw_spanner(br.x + 23, br.y + br.h / 2 + pd);
                    if (is_act && g_chars) {            // Mario at the right end, inside the button
                        C2D_Image mi = C2D_SpriteSheetGetImage(g_chars, 0);
                        float mh = FOLD_MARIO_H, mk = mh / mi.subtex->height;
                        C2D_DrawImageAt(mi, br.x + br.w - 6 - mi.subtex->width * mk, br.y + br.h - 2 - mh + pd,
                                        0.5f, NULL, mk, mk);
                    }
                }
            }
            if (fold_msg == 1) draw_text_center(S->saved_msg, 160, 178, 0.55f, COL_GREEN);
            else if (fold_msg) draw_text_center(fold_msg == 2 ? S->name_bad : fold_msg == 3 ? S->name_dup : S->name_fail,
                                                160, 178, 0.55f, COL_RED);
            draw_button(slid(FOLD_SAVE, 3), S->save_btn, "", !dialog && fold_cursor == 3, false, true, true);
            draw_label_mark(g_icons, 1, slid(FOLD_SAVE, 3), S->save_btn, !dialog && fold_cursor == 3,
                            S == &STR_EN ? 0.72f : 1.0f);   // the English label is longer
            draw_button(slid(FOLD_RESET, 3), S->reset_btn, "", !dialog && fold_cursor == 4, false, true, true);
            g_btn_sc = 0.6f;
            draw_back_tab(slid(ABOUT_BACK, 3), !dialog && fold_cursor == 5, true);
            if (mario_talk && active > 0) {             // on top of the rows; the pointer reaches his mouth
                Rect mr = slid(fold_mario_hit(active - 1), active - 1);
                g_tip_sc = 0.4f;
                draw_tooltip(mr.x + FOLD_MARIO_W * 0.72f, mr.y + 15, S->mario_talk, true);
                g_tip_sc = 0.5f;
            }
            if (!dialog && fold_cursor >= 7) {           // what the plate does
                Rect nr = slid(FOLD_NAME_R[fold_cursor - 7], fold_cursor - 7);
                g_tip_sc = 0.42f;
                draw_tooltip(nr.x + nr.w / 2, nr.y + nr.h - 2, S->color_tip, false);
                g_tip_sc = 0.5f;
            }
        } else if (screen == SCR_EXTRA) {
            draw_text_center(APP_TEAM, 160, 6, 0.75f, COL_YELLOW);
            bool busy = ex_busy();
            for (int i = 0; i < EX_ITEMS; i++) {
                // version (first line of the reference file on GitHub)
                const char *vt;
                char vbuf[96];
                if (g_exver[i].state == CHK_DONE) {
                    bool has_ver = strstr(g_exver[i].remote, "Ver") || strstr(g_exver[i].remote, "ver") ||
                                   strstr(g_exver[i].remote, "VER");
                    snprintf(vbuf, sizeof vbuf, has_ver ? "%s" : "Ver: %s", g_exver[i].remote);
                    vt = vbuf;
                } else if (g_exver[i].state == CHK_RUNNING) {
                    vt = S->checking;
                } else {
                    vt = S->ver_unknown;
                }
                float vy = EX_NAME_R[i].y - 20;
                float vw, vh;
                text_size(vt, 0.48f, &vw, &vh);
                float vsc = vw > 290 ? 0.48f * 290 / vw : 0.48f;
                draw_text(vt, 18 + slide_dx(i), vy, vsc, COL_GRAY);

                // note "!" + name + download button
                draw_note_button(slid(EX_NOTE_R[i], i), EX_NOTE_HIT[i], note_dialog < 0 && ex_cursor == 3 + i);
                draw_name_plate(slid(EX_NAME_R[i], i), EX_NAMES[i],
                                cfg.dark ? (i == 0 ? C2D_Color32(0x3A, 0x1E, 0x24, 0xFF) : C2D_Color32(0x1A, 0x32, 0x26, 0xFF))
                                         : (i == 0 ? C2D_Color32(0xF6, 0xCC, 0xCC, 0xFF) : C2D_Color32(0xCC, 0xEE, 0xCF, 0xFF)),
                                cfg.dark ? (i == 0 ? C2D_Color32(0x8E, 0x4A, 0x54, 0xFF) : C2D_Color32(0x45, 0x86, 0x60, 0xFF))
                                         : (i == 0 ? C2D_Color32(0xC8, 0x60, 0x60, 0xFF) : C2D_Color32(0x58, 0xA8, 0x68, 0xFF)),
                                COL_WHITE);
                g_btn_sc = 0.52f;
                draw_button(slid(EX_DL_R[i], i), S->dl_btn, "", !dialog && ex_cursor == i, false,
                            !busy, true);
                g_btn_sc = 0.6f;

                // status
                char st[128] = "";
                u32 sc = COL_GREEN;
                if (g_ex.state[i] == EX_RUNNING) { snprintf(st, sizeof st, S->ex_downloading, g_ex.done, EX_COUNT[i]); sc = COL_WHITE; }
                else if (g_ex.state[i] == EX_DONE) snprintf(st, sizeof st, S->ex_done, EX_COUNT[i]);
                else if (g_ex.state[i] == EX_FAIL) { snprintf(st, sizeof st, "%s", S->ex_fail); sc = COL_RED; }
                if (st[0]) {
                    float w, h;
                    text_size(st, 0.45f, &w, &h);
                    float ssc = w > 296 ? 0.45f * 296 / w : 0.45f;
                    draw_text(st, 18 + slide_dx(i), EX_NAME_R[i].y + 40, ssc, sc);
                }
            }
            draw_back_tab(slid(ABOUT_BACK, EX_ITEMS), !dialog && ex_cursor == 2, !busy);
        } else if (screen == SCR_ABOUT) {
            draw_text_center(S->about, 160, 6, 0.7f, COL_YELLOW);
            if (!draw_logo(1, 160 + slide_dx(0), 26)) {
                draw_text_center(APP_NAME_1, 160 + slide_dx(0), 34, 0.75f, COL_WHITE);
                draw_text_center(APP_NAME_2, 160 + slide_dx(0), 62, 0.75f, COL_WHITE);
            }
            snprintf(line, sizeof line, "%s  %s", S->version_label, APP_VERSION);
            draw_text_center(line, 160 + slide_dx(1), 110, 0.5f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->pack_label, pack_ver[0] ? pack_ver : "---");
            draw_text_center(line, 160 + slide_dx(2), 127, 0.5f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->author_label, APP_AUTHOR);
            draw_text_center(line, 160 + slide_dx(3), 144, 0.5f, COL_GRAY);
            draw_text_center(APP_TEAM, 160 + slide_dx(4), 161, 0.5f, COL_GRAY);
            {   // repository URL
                float sc = 0.42f, w, h;
                text_size(APP_REPO, sc, &w, &h);
                if (w > 304) sc *= 304 / w;
                draw_text_center(APP_REPO, 160 + slide_dx(5), 182, sc, COL_GRAY);
            }
            draw_back_tab(slid(ABOUT_BACK, 6), note_dialog < 0 && about_cursor == 0, true);
            draw_note_button(slid(ABOUT_NOTE, 6), ABOUT_NOTE_HIT, note_dialog < 0 && about_cursor == 1);
        } else if (screen == SCR_APPUPD) {
            draw_text_center(S->appupd_title, 160, 4, 0.7f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, APP_VERSION);
            draw_text_center(line, 160 + slide_dx(0), 32, 0.5f, COL_GRAY);
            bool busy = upd_busy();
            if (!busy)                                  // hidden while updating
                draw_button(slid(APPUPD_BTN, 1), S->check_btn, "", !dialog && appupd_cursor == 0, false,
                            g_app.state != CHK_RUNNING && g_upd.state != UPD_DONE, true);
            bool show_both = upd_can_both();
            if (show_both) {
                snprintf(line, sizeof line, g_upd.other == 1 ? S->upd_both : S->inst_both,
                         g_upd.is_cia ? "3dsx" : "cia");
                draw_button(slid(BOTH_BTN, 2), line, "", !dialog && appupd_cursor == 3, false, true, true);
            }
            float stx = 160 + slide_dx(3);              // status text

            int st = g_app.state;
            bool newer = st == CHK_DONE && has_digit(g_app.remote) && version_cmp(g_app.remote, APP_VERSION) > 0;
            int us = g_upd.state;

            if (us == UPD_DOWNLOADING || us == UPD_INSTALLING) {
                // progress bar
                float pct = g_upd.total ? (float)g_upd.now / (float)g_upd.total : 0.0f;
                if (pct > 1.0f) pct = 1.0f;
                const char *ut = us == UPD_DOWNLOADING ? S->downloading : S->installing;
                if (g_upd.both && !g_upd.only_other)
                    snprintf(line, sizeof line, "%s (%d/2) %d%%", ut, g_upd.step, (int)(pct * 100));
                else snprintf(line, sizeof line, "%s %d%%", ut, (int)(pct * 100));
                draw_text_center(line, stx, 112, 0.55f, COL_WHITE);
                Rect pb = INSTALL_BTN;
                C2D_DrawRectSolid(pb.x - 2, pb.y + 6, 0.5f, pb.w + 4, 22, BTN_VALUE);
                C2D_DrawRectSolid(pb.x, pb.y + 8, 0.5f, pb.w, 18, C2D_Color32(0x20, 0x22, 0x30, 0xFF));
                C2D_DrawRectSolid(pb.x, pb.y + 8, 0.5f, pb.w * pct, 18, C2D_Color32(0x2C, 0x78, 0xE0, 0xFF));
            } else if (us == UPD_DONE) {
                draw_text_center(g_upd.is_cia ? S->done_cia : S->done_3dsx, stx, 100, 0.52f, COL_GREEN);
                draw_button(slid(INSTALL_BTN, 4), g_upd.is_cia ? S->restart : S->quit, "",
                            !dialog && appupd_cursor == 1, false, true, true);
            } else {
                if (us == UPD_FAIL) {
                    const char *ft = g_upd.fail == UFAIL_FILE ? S->fail_file :
                                     g_upd.fail == UFAIL_BAD  ? S->fail_bad  :
                                     g_upd.fail == UFAIL_INSTALL ? S->fail_install : S->fail_net;
                    char fbuf[160];
                    if (g_upd.fail == UFAIL_INSTALL && g_upd.stage) {
                        bool failed_cia = g_upd.fail_other ? !g_upd.is_cia : g_upd.is_cia;
                        snprintf(fbuf, sizeof fbuf, "%s\n(%s%d: %08lX)", ft,
                                 !g_upd.both ? "" : failed_cia ? "cia " : "3dsx ",
                                 g_upd.stage, (unsigned long)g_upd.code);
                        ft = fbuf;
                    }
                    draw_text_center(ft, stx, show_both ? 122 : 100, 0.5f, COL_RED);
                } else if (st == CHK_RUNNING) {
                    draw_text_center(S->checking, stx, show_both ? 124 : 110, 0.55f, COL_GRAY);
                } else if (newer) {
                    draw_text_center(S->app_new, stx, show_both ? 122 : 98, 0.52f, COL_GREEN);
                    snprintf(line, sizeof line, "%s: %s", S->latest_label, g_app.remote);
                    draw_text_center(line, stx, show_both ? 139 : 120, 0.52f, COL_WHITE);
                } else if (st == CHK_DONE && has_digit(g_app.remote)) {
                    draw_text_center(S->app_latest, stx, show_both ? 124 : 110, 0.52f, COL_GREEN);
                } else if (st == CHK_DONE || st == CHK_FAIL) {
                    draw_text_center(S->app_fail, stx, show_both ? 124 : 110, 0.52f, COL_RED);
                }
                if (newer)
                    draw_button(slid(show_both ? INSTALL_TOP : INSTALL_BTN, 4), S->install_btn, "",
                                !dialog && appupd_cursor == 1, false, true, true);
            }
            draw_back_tab(slid(ABOUT_BACK, 5), !dialog && appupd_cursor == 2, !busy);
        } else {
            const char *langname = cfg.lang == 0 ? S->lang_auto : (cfg.lang == 1 ? "日本語" : "English");
            const char *labels[LIST_N] = {
                S->set_language, S->set_clock, S->set_dark, S->set_sfx, S->set_bgm, S->set_led, S->set_confirm, S->set_exit, S->set_check,
                S->set_bgcolor, S->set_appupd, S->set_restart, S->set_reset, S->about
            };
            const char *values[LIST_N] = { langname, cfg.clock24 ? S->h24 : S->h12, "", "", "", "", "", "", "", "", "", "", "", "" };
            // ON/OFF items (shown as switches); NULL = not a switch
            int *toggles[LIST_N] = {
                NULL, NULL, &cfg.dark, &cfg.sfx, &cfg.bgm, &cfg.led, &cfg.confirm, &cfg.exit_after, &cfg.check_update,
                NULL, NULL, NULL, NULL, NULL
            };
            static const bool centered[LIST_N] = { false, false, false, false, false, false, false, false, false, true, true, true, true, true };
            static float sw_anim[LIST_N];
            static bool sw_init = false;
            for (int i = 0; i < LIST_N; i++) {          // slide knobs toward their state
                if (!toggles[i]) continue;
                float target = *toggles[i] ? 1.0f : 0.0f;
                if (!sw_init) sw_anim[i] = target;
                sw_anim[i] += (target - sw_anim[i]) * 0.35f;
                if (sw_anim[i] < 0.01f) sw_anim[i] = 0; else if (sw_anim[i] > 0.99f) sw_anim[i] = 1;
            }
            sw_init = true;

            C2D_DrawRectSolid(0, LIST_Y, 0.5f, 320, LIST_H, C2D_Color32(0, 0, 0, 0x38));   // list area
            g_press_block = t_drag || t_mode == 2;
            // rows (those peeking out of the list area get masked below)
            int first_row = scroll / ROW_PITCH;         // the slide starts at the top visible row
            for (int i = 0; i < LIST_N; i++) {
                if (g_hide_restart && i == SET_RESTART) continue;
                Rect r = row_rect(i, scroll);
                if (r.y + r.h < LIST_Y - 4 || r.y > LIST_Y + LIST_H + 4) continue;
                bool sel = !dialog && set_cursor == i;
                int pos = set_pos(i);
                r = slid(r, pos > first_row ? pos - first_row : 0);
                g_btn_reserve = toggles[i] ? SW_W + 10 : 0;
                draw_button(r, labels[i], values[i], sel, false, true, centered[i]);
                if (i == 10) draw_label_mark(g_icons, 2, r, labels[i], sel, 1.0f);    // app update (download)
                if (i == 11) draw_label_mark(g_icons, 0, r, labels[i], sel, 1.0f);   // restart (circular arrow)
                if (i == 12) draw_label_mark(g_trash, 0, r, labels[i], sel, 1.0f);   // reset settings
                if (toggles[i]) draw_switch(r, sel, sw_anim[i], *toggles[i] != 0);
            }
            g_btn_reserve = 0;
            g_press_block = false;
            // header and footer bands (same in both themes), see-through over the background
            {
                u32 band = C2D_Color32(0x14, 0x2C, 0x6C, 0xB8), rule = C2D_Color32(0xE8, 0xC0, 0x40, 0xFF);
                draw_bg_band(0, LIST_Y, cfg.dark);      // covers the rows scrolled out of the list
                draw_bg_band(LIST_Y + LIST_H, 240, cfg.dark);
                C2D_DrawRectSolid(0, 0, 0.5f, 320, LIST_Y, band);
                C2D_DrawRectSolid(0, LIST_Y - 2, 0.5f, 320, 2, rule);
                C2D_DrawRectSolid(0, LIST_Y + LIST_H, 0.5f, 320, 240 - LIST_Y - LIST_H, band);
                C2D_DrawRectSolid(0, LIST_Y + LIST_H, 0.5f, 320, 2, rule);
            }
            {
                u32 saved = g_shadow;
                g_shadow = SHADOW;
                draw_text_center(S->settings_title, 160, 3, 0.7f, BTN_VALUE);
                g_shadow = saved;
            }
            draw_back_tab(slid(BACK_R, LIST_H / ROW_PITCH + 1), !dialog && set_cursor == LIST_N, true);

            // scroll bar
            const int max_scroll = LIST_CONTENT - LIST_H;
            draw_slot_v(SB_X, LIST_Y + 2, LIST_H - 4);
            float ty = LIST_Y + (float)(LIST_H - THUMB_H) * (max_scroll ? (float)scroll / max_scroll : 0.0f);
            draw_seek_knob(SB_X - 6, ty, THUMB_H, t_mode == 2);
        }

        if (dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            draw_window(DLG_BOX);
            if (dlg_kind == DLG_RESET) snprintf(line, sizeof line, "%s", S->confirm_reset);
            else if (dlg_kind == DLG_LAUNCH) snprintf(line, sizeof line, "%s", S->confirm_launch);
            else if (dlg_kind == DLG_RESTART) snprintf(line, sizeof line, "%s", S->confirm_restart);
            else if (dlg_kind == DLG_FOLD_SAVE) snprintf(line, sizeof line, "%s", S->confirm_fold_save);
            else if (dlg_kind == DLG_FOLD_RESET) snprintf(line, sizeof line, "%s", S->confirm_fold_reset);
            else snprintf(line, sizeof line, S->confirm_msg, g_names[dlg_target - 1]);
            {                                           // a long folder name must fit the window
                float dw, dh, dsc = 0.6f;
                text_size(line, dsc, &dw, &dh);
                if (dw > DLG_BOX.w - 12) dsc *= (DLG_BOX.w - 12) / dw;
                draw_text_center(line, 160, DLG_BOX.y + 22, dsc, COL_WHITE);
            }
            draw_button(DLG_YES, S->yes, "", dlg_yes, false, true, true);
            draw_button(DLG_NO, S->no, "", !dlg_yes, false, true, true);
        }

        if (upd_dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            draw_window(UPD_BOX);
            draw_text_center(S->update_msg, 160, UPD_BOX.y + 14, 0.55f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, pack_ver);
            draw_text_center(line, 160, UPD_BOX.y + 54, 0.55f, COL_WHITE);
            snprintf(line, sizeof line, "%s: %s", S->latest_label, latest);
            draw_text_center(line, 160, UPD_BOX.y + 80, 0.55f, COL_GREEN);
            draw_button(DLG_OK, S->ok, "", true, false, true, true);
        } else if (app_dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            draw_window(UPD_BOX);
            draw_text_center(S->app_dlg_msg, 160, UPD_BOX.y + 14, 0.55f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, APP_VERSION);
            draw_text_center(line, 160, UPD_BOX.y + 46, 0.52f, COL_WHITE);
            snprintf(line, sizeof line, "%s: %s", S->latest_label, g_app.remote);
            draw_text_center(line, 160, UPD_BOX.y + 68, 0.52f, COL_GREEN);
            draw_button(APP_YES, S->btn_update, "", app_yes, false, true, true);
            draw_button(APP_NO, S->btn_later, "", !app_yes, false, true, true);
        }

        if (notice) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            draw_window(UPD_BOX);
            {                                           // important: red frame over the window's own
                Rect b = UPD_BOX;
                u32 red = C2D_Color32(0xF0, 0x50, 0x40, 0xFF);
                C2D_DrawRectSolid(b.x - 2, b.y - 2, 0.5f, b.w + 4, 3, red);
                C2D_DrawRectSolid(b.x - 2, b.y + b.h - 1, 0.5f, b.w + 4, 3, red);
                C2D_DrawRectSolid(b.x - 2, b.y + 1, 0.5f, 3, b.h - 2, red);
                C2D_DrawRectSolid(b.x + b.w - 1, b.y + 1, 0.5f, 3, b.h - 2, red);
            }
            draw_text_center(notice == 4 ? "!!!" : "!", 160, UPD_BOX.y + 10, 0.9f, COL_RED);
            if (notice == 4) {                                 // two lines, each centered
                char l1[128];
                const char *nl = strchr(S->notice_batt_crit, '\n');
                size_t n = nl ? (size_t)(nl - S->notice_batt_crit) : strlen(S->notice_batt_crit);
                if (n >= sizeof l1) n = sizeof l1 - 1;
                memcpy(l1, S->notice_batt_crit, n); l1[n] = '\0';
                draw_text_center(l1, 160, UPD_BOX.y + 44, 0.55f, COL_WHITE);
                if (nl) draw_text_center(nl + 1, 160, UPD_BOX.y + 66, 0.55f, COL_WHITE);
            }
            else
                draw_text_center(notice == 8 ? S->notice_ctgp : notice == 1 ? S->notice_wifi : S->notice_batt,
                                 160, UPD_BOX.y + 56, 0.58f, COL_WHITE);
            draw_button(DLG_OK, S->ok, "", true, false, true, true);
        }

        if (note_dialog >= 0) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            draw_window(UPD_BOX);
            if (note_dialog == NOTE_AI) {
                draw_text_center(S->ai_title, 160, UPD_BOX.y + 12, 0.62f, COL_YELLOW);
                draw_lines_center(S->ai_note, 160, UPD_BOX.y + 50, 0.5f, 22, COL_WHITE);
            } else if (note_dialog == NOTE_NAMES) {
                draw_text_center(S->name_rules_title, 160, UPD_BOX.y + 8, 0.56f, COL_YELLOW);
                draw_text_center("/  \\  :  *  ?  \"  <  >  |", 160, UPD_BOX.y + 32, 0.6f, COL_WHITE);
                draw_lines_center(S->name_rules, 160, UPD_BOX.y + 60, 0.45f, 18, COL_GRAY);
            } else {
                char rg[96];
                snprintf(rg, sizeof rg, "%s: %s", S->regions_label, EX_REGIONS[note_dialog]);
                draw_text_center(EX_NAMES[note_dialog], 160, UPD_BOX.y + 6, 0.6f, COL_YELLOW);
                draw_text_center(rg, 160, UPD_BOX.y + 30, 0.5f, COL_WHITE);
                draw_lines_center(S->ex_note, 160, UPD_BOX.y + 52, 0.45f, 16, COL_GRAY);
            }
            draw_button(DLG_OK, S->ok, "", true, false, true, true);
        }

        C3D_FrameEnd(0);
    }

    sfx_exit();
    status_exit();
    net_stop();
    if (g_romfs && g_chk.state != CHK_RUNNING && g_app.state != CHK_RUNNING) romfsExit();

    if (g_top_bg) C2D_SpriteSheetFree(g_top_bg);
    if (g_bot_bg) C2D_SpriteSheetFree(g_bot_bg);
    if (g_btns)   C2D_SpriteSheetFree(g_btns);
    if (g_logo)   C2D_SpriteSheetFree(g_logo);
    if (g_kart)   C2D_SpriteSheetFree(g_kart);
    if (g_shell)  C2D_SpriteSheetFree(g_shell);
    if (g_bowser) C2D_SpriteSheetFree(g_bowser);
    if (g_lakitu) C2D_SpriteSheetFree(g_lakitu);
    if (g_chars)  C2D_SpriteSheetFree(g_chars);
    if (g_lens)   C2D_SpriteSheetFree(g_lens);
    if (g_back)   C2D_SpriteSheetFree(g_back);
    if (g_spanner) C2D_SpriteSheetFree(g_spanner);
    if (g_parts)  C2D_SpriteSheetFree(g_parts);
    if (g_window) C2D_SpriteSheetFree(g_window);
    if (g_hands)  C2D_SpriteSheetFree(g_hands);
    if (g_trash)  C2D_SpriteSheetFree(g_trash);
    if (g_icons)  C2D_SpriteSheetFree(g_icons);
    if (g_seek)   C2D_SpriteSheetFree(g_seek);
    C2D_TextBufDelete(g_buf);
    C2D_FontFree(g_font);
    C2D_Fini();
    C3D_Fini();
    cfguExit();
    gfxExit();
    return 0;
}
