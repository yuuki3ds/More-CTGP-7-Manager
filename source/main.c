// More！CTGP-7 Manager Ver.1.1.0 - character folder manager for More!CTGP-7
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
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <curl/curl.h>
#include <sys/stat.h>

#define APP_VERSION "Ver.1.1.0"
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
#define REL_URL_3DSX "https://github.com/yuuki3ds/More-CTGP-7-Manager/releases/download/3DS/MoreCTGP7Manager.3dsx"
#define REL_URL_CIA  "https://github.com/yuuki3ds/More-CTGP-7-Manager/releases/download/3DS/MoreCTGP7Manager.cia"
#define DEFAULT_3DSX_PATH "sdmc:/3ds/MoreCTGP7Manager.3dsx"

// ------------------------------------------------------------ strings

typedef struct {
    const char *title;
    const char *current;       // "%d"
    const char *current_none;
    const char *tag_active;
    const char *help;
    const char *already;       // "%d"
    const char *done;          // "%d"
    const char *fail_step1;    // "%d"
    const char *fail_step2;    // "%d"
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
    const char *confirm_msg;   // "%d"
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
    const char *appupd_title;
    const char *check_btn;
    const char *checking;
    const char *app_new;
    const char *app_latest;
    const char *app_fail;
    const char *set_dark;
    const char *set_sfx;
    const char *set_bgm;
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
} Strings;

static const Strings STR_EN = {
    "More！CTGP-7 Manager",
    "Active: Characters%d",
    "Active: none",
    "[ACTIVE]",
    "D-pad/Touch: select   A: OK   B: back   START: exit",
    "Characters%d is already active.",
    "Switched. Characters%d is now active.",
    "Failed: Characters -> Characters%d",
    "Failed: Characters%d -> Characters\nRolled back.",
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
    "Switch to Characters%d?",
    "Yes",
    "No",
    "About this app",
    "Version",
    "Created by",
    "Launch CTGP-7",
    "CTGP-7 was not found.\nInstall CTGP-7, or set ctgp7_id in\nsdmc:/3ds/MoreCTGP7Manager/config.ini",
    "Pack version",
    "Check for updates at startup",
    "A pack update is available.",
    "Current",
    "Latest",
    "OK",
    "App update",
    "App Update",
    "Check for updates",
    "Checking...",
    "A new update is available.",
    "The latest version is installed.",
    "Could not check for updates.",
    "Dark mode",
    "Sound effects",
    "BGM",
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
};

static const Strings STR_JA = {
    "More！CTGP-7 Manager",
    "現在の有効フォルダ: Characters%d",
    "現在の有効フォルダ: なし",
    "[有効]",
    "十字キー/タッチ: 選択   A: 決定   B: 戻る   START: 終了",
    "Characters%d は既に有効です。",
    "切り替え完了: Characters%d が有効です。",
    "失敗: Characters → Characters%d",
    "失敗: Characters%d → Characters\n元に戻しました。",
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
    "Characters%d に切り替えますか?",
    "はい",
    "いいえ",
    "このアプリについて",
    "バージョン",
    "作成者",
    "CTGP-7を起動",
    "CTGP-7が見つかりません。\nCTGP-7をインストールするか、\nsdmc:/3ds/MoreCTGP7Manager/config.ini の\nctgp7_id を設定してください。",
    "パックバージョン",
    "起動時に更新を確認",
    "パックのアップデートがあります。",
    "現在",
    "最新",
    "OK",
    "アプリの更新",
    "アプリの更新",
    "更新のチェック",
    "確認中...",
    "新しいアップデートがあります。",
    "最新バージョンがインストールされています。",
    "更新を確認できませんでした。",
    "ダークモード",
    "効果音",
    "BGM",
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
};

// ------------------------------------------------------------ config

typedef struct {
    int lang;        // 0 = auto, 1 = Japanese, 2 = English
    int confirm;     // 0/1
    int exit_after;  // 0/1
    unsigned long long ctgp7_id;  // 0 = not known yet (auto-detect on first launch)
    int check_update;             // check pack version on GitHub at startup (0/1)
    char pack_url[200];
    int dark;                     // dark mode (0/1)
    int sfx;                      // sound effects (0/1)
    int bgm;                      // background music (0/1)
} Config;

static void config_defaults(Config *c) {
    c->lang = 0;
    c->confirm = 1;
    c->exit_after = 0;
    c->ctgp7_id = 0;
    c->check_update = 1;
    c->dark = 0;
    c->sfx = 1;
    c->bgm = 1;
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
        if (!strcmp(key, "ctgp7_id")) {
            c->ctgp7_id = strtoull(val, NULL, 16);
            continue;
        }
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
    }
    fclose(f);
}

static void config_save(const Config *c) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(CFG_PATH, "w");
    if (!f) return;
    fprintf(f, "lang=%d\nconfirm=%d\nexit_after=%d\nctgp7_id=%016llX\ncheck_update=%d\ndark=%d\nsfx=%d\nbgm=%d\n",
            c->lang, c->confirm, c->exit_after, c->ctgp7_id, c->check_update, c->dark, c->sfx, c->bgm);
    if (strcmp(c->pack_url, PACK_URL_DEFAULT) != 0)     // only persist a custom URL
        fprintf(f, "pack_url=%s\n", c->pack_url);
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
} CheckCtx;

static CheckCtx g_chk;   // pack check (startup)
static CheckCtx g_app;   // app check (manual)

// in-app updater state (logic further below)
enum { UPD_IDLE = 0, UPD_DOWNLOADING, UPD_INSTALLING, UPD_DONE, UPD_FAIL };
enum { UFAIL_NONE = 0, UFAIL_NET, UFAIL_FILE, UFAIL_BAD, UFAIL_INSTALL };

#define UPD_TMP CFG_DIR "/update.tmp"

typedef struct {
    bool is_cia;
    char target[256];          // .3dsx path to replace
    volatile int state, fail;
    volatile u64 now, total;   // progress (bytes)
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

    CURLcode rc = curl_easy_perform(c);
    long code = 0;
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(c);

    if (rc == CURLE_OK && code == 200 && r.len > 0) {
        r.buf[r.len] = '\0';
        parse_version_text(r.buf, ctx->remote, sizeof ctx->remote);
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

static void net_stop(void) {
    if (!g_net_ready) return;
    g_chk.quit = true;
    g_app.quit = true;
    for (int i = 0; i < 40 && (g_chk.state == CHK_RUNNING || g_app.state == CHK_RUNNING || g_upd.state == UPD_DOWNLOADING || g_upd.state == UPD_INSTALLING); i++)
        svcSleepThread(50000000LL);                    // wait up to ~2 s
    if (g_chk.state != CHK_RUNNING && g_app.state != CHK_RUNNING &&
        g_upd.state != UPD_DOWNLOADING && g_upd.state != UPD_INSTALLING) {   // otherwise just leak; we are exiting
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

static bool upd_download(const char *url) {
    const char *ca = find_cacert();
    if (!ca) return false;
    FILE *f = fopen(UPD_TMP, "wb");
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
static bool upd_verify(void) {
    FILE *f = fopen(UPD_TMP, "rb");
    if (!f) return false;
    u8 h[4] = {0};
    size_t n = fread(h, 1, 4, f);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fclose(f);
    if (n != 4 || len < 4096) return false;
    if (g_upd.is_cia) return h[0] == 0x20 && h[1] == 0x20 && h[2] == 0 && h[3] == 0;   // header size 0x2020
    return !memcmp(h, "3DSX", 4);
}

static bool upd_install_3dsx(void) {
    char bak[264];
    snprintf(bak, sizeof bak, "%s.bak", g_upd.target);
    remove(bak);
    bool had_old = rename(g_upd.target, bak) == 0;
    if (rename(UPD_TMP, g_upd.target) != 0) {
        if (had_old) rename(bak, g_upd.target);          // put the old one back
        return false;
    }
    if (had_old) remove(bak);
    return true;
}

static bool upd_install_cia(void) {
    FILE *f = fopen(UPD_TMP, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    u64 len = (u64)ftell(f);
    fseek(f, 0, SEEK_SET);
    g_upd.total = len;
    g_upd.now = 0;

    const size_t CH = 128 * 1024;
    u8 *buf = malloc(CH);
    if (!buf || R_FAILED(amInit())) { free(buf); fclose(f); return false; }

    Handle h = 0;
    bool ok = R_SUCCEEDED(AM_StartCiaInstall(MEDIATYPE_SD, &h));
    u64 off = 0;
    while (ok && off < len) {
        size_t n = fread(buf, 1, CH, f);
        if (n == 0) { ok = false; break; }
        u32 w = 0;
        if (R_FAILED(FSFILE_Write(h, &w, off, buf, n, 0)) || w != n) { ok = false; break; }
        off += n;
        g_upd.now = off;
    }
    if (ok) ok = R_SUCCEEDED(AM_FinishCiaInstall(h));
    else if (h) AM_CancelCIAInstall(h);

    amExit();
    free(buf);
    fclose(f);
    return ok;
}

static void upd_thread(void *arg) {
    (void)arg;
    g_upd.fail = UFAIL_NONE;
    g_upd.now = g_upd.total = 0;
    g_upd.state = UPD_DOWNLOADING;
    if (!upd_download(g_upd.is_cia ? REL_URL_CIA : REL_URL_3DSX)) {
        if (!g_upd.fail) g_upd.fail = UFAIL_NET;
        remove(UPD_TMP);
        g_upd.state = UPD_FAIL;
        return;
    }
    if (!upd_verify()) {
        g_upd.fail = UFAIL_BAD;
        remove(UPD_TMP);
        g_upd.state = UPD_FAIL;
        return;
    }
    g_upd.state = UPD_INSTALLING;
    bool ok = g_upd.is_cia ? upd_install_cia() : upd_install_3dsx();
    remove(UPD_TMP);
    if (!ok) { g_upd.fail = UFAIL_INSTALL; g_upd.state = UPD_FAIL; return; }
    g_upd.state = UPD_DONE;
}

static bool upd_busy(void) {
    return g_upd.state == UPD_DOWNLOADING || g_upd.state == UPD_INSTALLING;
}

static bool upd_start(void) {
    if (upd_busy() || !net_init()) return false;
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

// ------------------------------------------------------------ sound effects (ndsp)
// WAV files in romfs:/sound/. Needs sdmc:/3ds/dspfirm.cdc; without it the app runs silently.

// SE_BGM is the background music; it uses the same loader on its own DSP channel.
enum { SE_OK = 0, SE_CANCEL, SE_DECIDE, SE_CURSOR, SE_DLG_OPEN, SE_DLG_CLOSE, SE_SLOT, SE_BGM, SE_COUNT };

#define BGM_GAP_MS 3000   // silence between BGM loops

static const char *SE_FILES[SE_COUNT] = {
    "romfs:/sound/SE_SYS_BTN_OK.wav",
    "romfs:/sound/SE_SYS_CANCEL_S.wav",
    "romfs:/sound/SE_SYS_DECIDE_TITLE.wav",
    "romfs:/sound/SE_SYS_CURSOR.wav",
    "romfs:/sound/SE_SYS_DIALOG_OPEN.wav",
    "romfs:/sound/SE_SYS_DIALOG_CLOSE.wav",
    "romfs:/sound/SE_SYS_SLOT_FRAME.wav",
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

static void sfx_init(void) {
    if (R_FAILED(ndspInit())) return;              // no dspfirm.cdc etc.
    g_ndsp = true;
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    for (int i = 0; i < SE_COUNT; i++) sfx_load(i, SE_FILES[i]);
}

static void sfx_play(int id) {
    if (!g_ndsp || !g_sfx_on || id < 0 || id >= SE_COUNT || !g_se[id].ok) return;
    ndspChnWaveBufClear(id);                        // restart if still playing
    g_se[id].wb.status = NDSP_WBUF_DONE;
    ndspChnWaveBufAdd(id, &g_se[id].wb);
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
    snprintf(out, n, BASE "Characters%d", slot);
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

// ------------------------------------------------------------ CTGP-7 lookup / launch

// Reads the SMDH of an installed SD title and checks whether its short title
// is exactly "CTGP-7" (in any language slot).
static bool title_is_ctgp7(u64 id) {
    static u8 buf[8 + 16 * 0x200];
    u32 arch[4] = { (u32)id, (u32)(id >> 32), MEDIATYPE_SD, 0 };
    u32 file[5] = { 0, 0, 2, 0x6E6F6369 /* "icon" */, 0 };
    Handle fh;
    Result rc = FSUSER_OpenFileDirectly(&fh, ARCHIVE_SAVEDATA_AND_CONTENT,
                                        fsMakePath(PATH_BINARY, arch),
                                        fsMakePath(PATH_BINARY, file),
                                        FS_OPEN_READ, 0);
    if (R_FAILED(rc)) return false;

    u32 got = 0;
    rc = FSFILE_Read(fh, &got, 0, buf, sizeof buf);
    FSFILE_Close(fh);
    if (R_FAILED(rc) || got < sizeof buf || memcmp(buf, "SMDH", 4) != 0) return false;

    for (int lang = 0; lang < 16; lang++) {
        const u16 *s = (const u16 *)(buf + 8 + lang * 0x200);   // short title, 0x40 chars
        char ascii[0x41];
        int n = 0;
        for (; n < 0x40 && s[n]; n++) ascii[n] = s[n] < 128 ? (char)s[n] : '?';
        ascii[n] = '\0';
        while (n > 0 && ascii[n - 1] == ' ') ascii[--n] = '\0';
        if (!strcmp(ascii, "CTGP-7")) return true;
    }
    return false;
}

// Scans installed SD titles. Returns the title ID, or 0 if not found.
static u64 find_ctgp7(void) {
    if (R_FAILED(amInit())) return 0;

    u64 found = 0;
    u32 count = 0;
    if (R_SUCCEEDED(AM_GetTitleCount(MEDIATYPE_SD, &count)) && count > 0) {
        u64 *ids = malloc(count * sizeof(u64));
        u32 read = 0;
        if (ids && R_SUCCEEDED(AM_GetTitleList(&read, MEDIATYPE_SD, count, ids))) {
            for (u32 i = 0; i < read; i++) {
                if ((ids[i] >> 32) != 0x00040000) continue;   // applications only
                if (title_is_ctgp7(ids[i])) { found = ids[i]; break; }
            }
        }
        free(ids);
    }
    amExit();
    return found;
}

// ------------------------------------------------------------ drawing

typedef struct { float x, y, w, h; } Rect;

static C2D_TextBuf g_buf;
static C2D_Font g_font;

// Theme colors for text drawn directly on the background (change with dark mode)
static u32 COL_WHITE, COL_YELLOW, COL_GREEN, COL_RED, COL_GRAY, COL_BG, COL_SHADOW_BG;
static u32 g_shadow;          // shadow color used by draw_text (set per context)
static float g_btn_sc = 0.6f; // label scale used by draw_button
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
static C2D_SpriteSheet g_btns   = NULL;   // 0: tinyButtonBackground, 1: selTinyButtonBackground
static C2D_SpriteSheet g_logo   = NULL;   // 0: top-screen logo (220x103), 1: about logo (180x84)
static C2D_SpriteSheet g_ghicon = NULL;   // optional: GitHub mark (gfx/github.png), tinted to text color

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
    g_bg_inverted = dark;
}

static void draw_bg(C2D_SpriteSheet sheet, float sw, float sh) {
    if (!sheet) return;
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);
    C2D_DrawImageAt(im, 0, 0, 0.5f, NULL, sw / im.subtex->width, sh / im.subtex->height);
}

// Redraws only the rows y0..y1 of a full-screen background (used to clip the
// scrolling settings list). Returns false if not possible (caller falls back).
static bool draw_bg_strip(C2D_SpriteSheet sheet, float sw, float sh, float y0, float y1) {
    if (!sheet || y1 <= y0) return false;
    C2D_Image im = C2D_SpriteSheetGetImage(sheet, 0);
    const Tex3DS_SubTexture *o = im.subtex;
    if (o->top < o->bottom) return false;            // rotated in atlas: not handled
    Tex3DS_SubTexture st = *o;
    float f0 = y0 / sh, f1 = y1 / sh;
    st.top    = o->top + f0 * (o->bottom - o->top);
    st.bottom = o->top + f1 * (o->bottom - o->top);
    float hh = o->height * (f1 - f0);
    st.height = hh < 1.0f ? 1 : (u16)(hh + 0.5f);
    C2D_Image strip = { im.tex, &st };
    C2D_DrawImageAt(strip, 0, y0, 0.5f, NULL, sw / st.width, (y1 - y0) / st.height);
    return true;
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
    if (g_shadow) {
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

static void draw_button(Rect r, const char *label, const char *value,
                        bool sel, bool active, bool enabled, bool center) {
    if (g_btns) {
        C2D_Image im = C2D_SpriteSheetGetImage(g_btns, sel ? 1 : 0);
        float sx = r.w / BTN_IN_W, sy = r.h / BTN_IN_H;
        C2D_DrawImageAt(im, r.x - BTN_IN_X * sx, r.y - BTN_IN_Y * sy, 0.5f, NULL, sx, sy);
    } else {
        u32 fill = active ? COL_PANEL_ACT : (sel ? COL_PANEL_SEL : COL_PANEL);
        u32 border = sel ? BTN_VALUE : COL_BORDER;
        C2D_DrawRectSolid(r.x - 2, r.y - 2, 0.5f, r.w + 4, r.h + 4, border);
        C2D_DrawRectSolid(r.x, r.y, 0.5f, r.w, r.h, fill);
    }

    u32 saved = g_shadow;
    g_shadow = SHADOW;                      // buttons are dark/blue: always a black shadow
    float sc = g_btn_sc;
    u32 tcol = !enabled ? BTN_DIS : (active ? BTN_ACT : BTN_TEXT);
    float w, h;
    text_size(label, sc, &w, &h);
    float ty = r.y + (r.h - h) / 2;
    if (center) draw_text(label, r.x + r.w / 2 - w / 2, ty, sc, tcol);
    else draw_text(label, r.x + 14, ty, sc, tcol);

    if (value && value[0]) {
        text_size(value, sc, &w, &h);
        draw_text(value, r.x + r.w - 14 - w, r.y + (r.h - h) / 2, sc,
                  active ? BTN_ACT : BTN_VALUE);
    }
    g_shadow = saved;
}

// Round gear button (settings) - drawn with shapes, no frame image.
static void draw_gear_button(Rect r, bool sel) {
    float cx = r.x + r.w / 2, cy = r.y + r.h / 2, R = r.w / 2;
    u32 ring = sel ? BTN_VALUE : C2D_Color32(0xD8, 0xD8, 0xE0, 0xFF);
    u32 fill = sel ? C2D_Color32(0x24, 0x5C, 0xC8, 0xFF) : C2D_Color32(0x30, 0x34, 0x48, 0xFF);
    u32 g = BTN_TEXT;

    C2D_DrawCircleSolid(cx + 1.5f, cy + 2.0f, 0.5f, R, C2D_Color32(0, 0, 0, 0x80));   // shadow
    C2D_DrawCircleSolid(cx, cy, 0.5f, R, ring);
    C2D_DrawCircleSolid(cx, cy, 0.5f, R - 2.5f, fill);

    const float ri = R * 0.45f, ro = R * 0.74f;
    for (int k = 0; k < 8; k++) {
        float a = k * (float)M_PI / 4.0f;
        float x1 = cx + cosf(a - 0.32f) * ri, y1 = cy + sinf(a - 0.32f) * ri;
        float x2 = cx + cosf(a + 0.32f) * ri, y2 = cy + sinf(a + 0.32f) * ri;
        float x3 = cx + cosf(a + 0.20f) * ro, y3 = cy + sinf(a + 0.20f) * ro;
        float x4 = cx + cosf(a - 0.20f) * ro, y4 = cy + sinf(a - 0.20f) * ro;
        C2D_DrawTriangle(x1, y1, g, x2, y2, g, x3, y3, g, 0.5f);
        C2D_DrawTriangle(x1, y1, g, x3, y3, g, x4, y4, g, 0.5f);
    }
    C2D_DrawCircleSolid(cx, cy, 0.5f, R * 0.52f, g);
    C2D_DrawCircleSolid(cx, cy, 0.5f, R * 0.22f, fill);
}

static bool hit(Rect r, touchPosition t) {
    return t.px >= r.x && t.px < r.x + r.w && t.py >= r.y && t.py < r.y + r.h;
}

// ------------------------------------------------------------ UI layout (bottom screen 320x240)

typedef enum { SCR_MAIN, SCR_SETTINGS, SCR_ABOUT, SCR_APPUPD } Screen;

#define MAIN_N 5
static const Rect MAIN_R[MAIN_N] = {
    {36,  42, 248, 42},   // Characters1
    {36,  92, 248, 42},   // Characters2
    {36, 142, 248, 42},   // Characters3
    {60, 200, 200, 32},   // Launch CTGP-7 (separated)
    {283,  4,  32, 32},   // Settings (round gear, top right)
};
static const Rect GEAR_HIT = {272, 0, 48, 44};   // larger touch area for the gear

// Settings: a scrollable list (LIST_N rows) + a fixed Back button (index LIST_N)
#define LIST_N       10
#define SET_N        (LIST_N + 1)
#define LIST_Y       28
#define LIST_H       170
#define ROW_PITCH    32
#define ROW_H        28
#define LIST_CONTENT (LIST_N * ROW_PITCH)
#define SB_X         300
#define THUMB_H      ((LIST_H * LIST_H) / LIST_CONTENT)

static const Rect BACK_R = {12, 206, 280, 28};

static Rect row_rect(int i, int scroll) {
    Rect r = {12, (float)(LIST_Y + i * ROW_PITCH - scroll), 280, ROW_H};
    return r;
}

static const Rect ABOUT_BACK = {20, 204, 280, 30};
static const Rect APPUPD_BTN = {40,  56, 240, 32};   // check for updates
static const Rect INSTALL_BTN = {30, 150, 260, 34};   // download & install / restart / exit
static const Rect APP_YES     = {38, 150, 116, 34};   // startup app-update dialog
static const Rect APP_NO      = {166, 150, 116, 34};

static const Rect DLG_BOX = { 30, 64, 260, 112};
static const Rect DLG_YES = { 48, 124, 100, 36};
static const Rect DLG_NO  = {172, 124, 100, 36};
static const Rect UPD_BOX = { 12,  46, 296, 150};
static const Rect DLG_OK  = {110, 150, 100, 34};

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
    g_ghicon = C2D_SpriteSheetLoad("romfs:/gfx/github.t3x");   // NULL if not added

    bool sys_ja = false;
    u8 lang = CFG_LANGUAGE_EN;
    if (R_SUCCEEDED(CFGU_GetSystemLanguage(&lang)) && lang == CFG_LANGUAGE_JP) sys_ja = true;

    migrate_cfg_dir();

    // which format is running? (.3dsx from the Homebrew Launcher, or an installed .cia)
    g_upd.is_cia = !envIsHomebrew();
    if (argc > 0 && argv && argv[0] && !strncmp(argv[0], "sdmc:/", 6) && strstr(argv[0], ".3dsx"))
        snprintf(g_upd.target, sizeof g_upd.target, "%s", argv[0]);
    else
        snprintf(g_upd.target, sizeof g_upd.target, "%s", DEFAULT_3DSX_PATH);
    Config cfg;
    config_load(&cfg);
    apply_theme(cfg.dark);
    set_bg_dark(cfg.dark);
    g_sfx_on = cfg.sfx;
    sfx_init();

    char pack_ver[64];
    load_pack_version(pack_ver, sizeof pack_ver);

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
    int appupd_cursor = 0;                 // app update screen: 0 check, 1 back

    bool dialog = false;
    int dlg_target = 0;
    bool dlg_yes = true;

    MsgKind msg_kind = MSG_NONE;
    int msg_arg = 0;

    bool running = true;
    bool quit_pending = false;             // exit after the current sound finishes
    int quit_frames = 0;

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
        u32 held = hidKeysHeld();
        u32 up   = hidKeysUp();
        touchPosition tp;
        hidTouchRead(&tp);
        if (quit_pending) {
            if (!sfx_busy() || ++quit_frames > 150) break;    // max ~2.5 s
            down = held = up = 0;
        }
        bool tap = (down & KEY_TOUCH) != 0;

        if ((down & KEY_START) && !upd_busy()) break;

        const Strings *S = get_strings(&cfg, sys_ja);

        // state snapshot (used to pick the sound effect for this frame)
        Screen  pv_screen = screen;
        int     pv_cur = cursor, pv_set = set_cursor, pv_app = appupd_cursor;
        bool    pv_dlg = dialog, pv_upd = upd_dialog, pv_yes = dlg_yes;
        bool    pv_appdlg = app_dialog, pv_appyes = app_yes;
        bool    ok_sfx = false;
        bool    ok_plain = false;   // force the normal OK sound
        int     force_se = -1;

        // ---------- pack update check result ----------
        if (chk_pending) {
            if (g_chk.state == CHK_DONE) {
                chk_pending = false;
                load_pack_version(pack_ver, sizeof pack_ver);
                if (has_digit(pack_ver) && has_digit(g_chk.remote) &&
                    version_cmp(g_chk.remote, pack_ver) > 0) {
                    snprintf(latest, sizeof latest, "%s", g_chk.remote);
                    upd_dialog = true;
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
            }
        }

        // ---------- input ----------
        int do_slot = 0;          // slot to request a switch to (1-3), 0 = none
        bool do_perform = false;
        bool do_launch = false;
        bool do_appcheck = false;

        if (upd_dialog) {
            if ((down & (KEY_A | KEY_B)) || (tap && hit(DLG_OK, tp))) upd_dialog = false;
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
            } else if (close) {
                app_dialog = false;
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
        } else if (screen == SCR_MAIN) {
            int act = -1;
            if (down & KEY_UP)   cursor = (cursor + MAIN_N - 1) % MAIN_N;
            if (down & KEY_DOWN) cursor = (cursor + 1) % MAIN_N;
            if (down & KEY_A) act = cursor;
            if (tap) {
                for (int i = 0; i < MAIN_N; i++)
                    if (hit(i == 4 ? GEAR_HIT : MAIN_R[i], tp)) { cursor = i; act = i; break; }
            }
            if (act >= 0) ok_sfx = true;
            if (act >= 0 && act < 3) do_slot = act + 1;
            else if (act == 3) do_launch = true;
            else if (act == 4) { screen = SCR_SETTINGS; set_cursor = 0; }
        } else if (screen == SCR_ABOUT) {
            if (down & (KEY_B | KEY_A)) screen = SCR_SETTINGS;
            if (tap && hit(ABOUT_BACK, tp)) screen = SCR_SETTINGS;
        } else if (screen == SCR_APPUPD) {
            // buttons: 0 check, 1 install/restart/exit (only when shown), 2 back
            bool newer = g_app.state == CHK_DONE && has_digit(g_app.remote) &&
                         version_cmp(g_app.remote, APP_VERSION) > 0;
            bool busy = upd_busy();
            bool show_mid = (newer && (g_upd.state == UPD_IDLE || g_upd.state == UPD_FAIL)) ||
                            g_upd.state == UPD_DONE;
            if (!busy) {
                if (!show_mid && appupd_cursor == 1) appupd_cursor = 0;
                if (down & KEY_UP)   { do appupd_cursor = (appupd_cursor + 2) % 3; while (!show_mid && appupd_cursor == 1); }
                if (down & KEY_DOWN) { do appupd_cursor = (appupd_cursor + 1) % 3; while (!show_mid && appupd_cursor == 1); }
                if (down & KEY_B) screen = SCR_SETTINGS;
                int act = -1;
                if (down & KEY_A) act = appupd_cursor;
                if (tap) {
                    if (hit(APPUPD_BTN, tp)) act = 0;
                    else if (show_mid && hit(INSTALL_BTN, tp)) act = 1;
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
                        upd_start();
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
                    if (row >= 0 && row < LIST_N && (y % ROW_PITCH) < ROW_H) {
                        set_cursor = row; act = row; dir = +1;
                    }
                }
                t_mode = 0;
            }

            if (follow && set_cursor < LIST_N) {
                int top = set_cursor * ROW_PITCH;
                if (top < scroll) scroll = top;
                if (top + ROW_H > scroll + LIST_H) scroll = top + ROW_H - LIST_H;
            }
            if (scroll < 0) scroll = 0;
            if (scroll > max_scroll) scroll = max_scroll;

            if (act >= 0 && !(down & KEY_B)) {
                bool changed = false;
                bool lr = (down & (KEY_LEFT | KEY_RIGHT)) != 0;
                switch (act) {
                case 0:
                    cfg.lang = (cfg.lang + 3 + dir) % 3;
                    changed = true;
                    break;
                case 1: cfg.dark = !cfg.dark; changed = true; break;
                case 2: cfg.sfx = !cfg.sfx; changed = true; break;
                case 3: cfg.bgm = !cfg.bgm; changed = true; break;
                case 4: cfg.confirm = !cfg.confirm; changed = true; break;
                case 5: cfg.exit_after = !cfg.exit_after; changed = true; break;
                case 6: cfg.check_update = !cfg.check_update; changed = true; break;
                case 7:
                    if (!lr) { screen = SCR_APPUPD; appupd_cursor = 0; }
                    break;
                case 8:
                    if (!lr) { config_defaults(&cfg); changed = true; }
                    break;
                case 9:
                    if (!lr) {
                        load_pack_version(pack_ver, sizeof pack_ver);
                        screen = SCR_ABOUT;
                    }
                    break;
                case 10:
                    if (!lr) screen = SCR_MAIN;
                    break;
                }
                if (act < LIST_N) ok_sfx = true;
                if (act == 7 || act == 8 || act == 9) ok_plain = true;   // app update / reset / about
                if (changed) {
                    config_save(&cfg);
                    apply_theme(cfg.dark);
                    set_bg_dark(cfg.dark);
                    g_sfx_on = cfg.sfx;
                }
            }
        }

        // ---------- switch request ----------
        if (do_slot && err == ERR_NONE) {
            if (do_slot == active) {
                msg_kind = MSG_ALREADY; msg_arg = do_slot;
            } else if (cfg.confirm) {
                dialog = true; dlg_target = do_slot; dlg_yes = true;
            } else {
                dlg_target = do_slot; do_perform = true;
            }
        }
        if (do_perform) {
            msg_kind = do_switch(active, dlg_target, &msg_arg);
            active = detect_active(&err);
            if (active < 0) active = 0;
            if (msg_kind == MSG_DONE && cfg.exit_after) quit_pending = true;
        }

        // ---------- manual app update check ----------
        if (do_appcheck && !upd_busy()) check_start(&g_app, APP_URL_DEFAULT);

        // ---------- launch CTGP-7 ----------
        if (do_launch) {
            if (cfg.ctgp7_id == 0) {
                cfg.ctgp7_id = find_ctgp7();
                if (cfg.ctgp7_id) config_save(&cfg);
            }
            if (cfg.ctgp7_id) {
                aptSetChainloader(cfg.ctgp7_id, MEDIATYPE_SD);
                force_se = SE_DECIDE;
                quit_pending = true;
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
                        ((pv_screen == SCR_ABOUT || pv_screen == SCR_APPUPD) && screen == SCR_SETTINGS);
            if (se < 0) {
                if ((!pv_dlg && dialog) || (!pv_upd && upd_dialog) ||
                    (!pv_appdlg && app_dialog && !upd_dialog)) se = SE_DLG_OPEN;
                else if ((pv_dlg && !dialog) || (pv_upd && !upd_dialog) || (pv_appdlg && !app_dialog))
                    se = (pv_appdlg && screen == SCR_APPUPD) ? SE_OK : SE_DLG_CLOSE;
                else if (back) se = SE_CANCEL;
                else if (ok_plain) se = SE_OK;
                else if (ok_sfx || screen != pv_screen)
                    se = (pv_screen == SCR_SETTINGS || pv_screen == SCR_APPUPD ||
                          pv_screen == SCR_ABOUT) ? SE_SLOT : SE_OK;
                else if (cursor != pv_cur || set_cursor != pv_set ||
                         appupd_cursor != pv_app || dlg_yes != pv_yes ||
                         app_yes != pv_appyes) se = SE_CURSOR;
            }
            if (se >= 0) sfx_play(se);
        }
        bgm_update(cfg.bgm && !quit_pending);    // stop BGM while exiting / launching

        // ---------- build strings ----------
        char line[200], msg[256] = "";
        const char *errtxt = "";
        if (err == ERR_NO_ACTIVE) errtxt = S->err_no_active;
        else if (err == ERR_AMBIGUOUS) errtxt = S->err_ambiguous;
        else if (err == ERR_UNEXPECTED) errtxt = S->err_unexpected;

        switch (msg_kind) {
        case MSG_ALREADY: snprintf(msg, sizeof msg, S->already, msg_arg); break;
        case MSG_DONE:    snprintf(msg, sizeof msg, S->done, msg_arg); break;
        case MSG_FAIL1:   snprintf(msg, sizeof msg, S->fail_step1, msg_arg); break;
        case MSG_FAIL2:   snprintf(msg, sizeof msg, S->fail_step2, msg_arg); break;
        case MSG_NOLAUNCH: snprintf(msg, sizeof msg, "%s", S->launch_fail); break;
        default: break;
        }

        // ---------- draw ----------
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(g_buf);

        // top screen
        C2D_TargetClear(top, COL_BG);
        C2D_SceneBegin(top);
        draw_bg(g_top_bg, 400, 240);
        if (!draw_logo(0, 200, 4)) {
            draw_text_center(APP_NAME_1, 200, 20, 0.85f, COL_YELLOW);
            draw_text_center(APP_NAME_2, 200, 54, 0.85f, COL_YELLOW);
        }
        if (err != ERR_NONE) {
            draw_text(errtxt, 14, 114, 0.55f, COL_RED);
            draw_text(S->err_footer, 14, 172, 0.5f, COL_GRAY);
        } else {
            if (active == 0) snprintf(line, sizeof line, "%s", S->current_none);
            else snprintf(line, sizeof line, S->current, active);
            draw_text(line, 14, 114, 0.65f, COL_WHITE);
            if (msg[0]) {
                bool bad = (msg_kind == MSG_FAIL1 || msg_kind == MSG_FAIL2 || msg_kind == MSG_NOLAUNCH);
                draw_text(msg, 14, 142, 0.55f, bad ? COL_RED : COL_GREEN);
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

        if (screen == SCR_MAIN) {
            draw_text_center(S->select_slot, 160, 8, 0.7f, COL_YELLOW);
            g_btn_sc = 0.72f;
            for (int i = 0; i < SLOTS; i++) {
                snprintf(line, sizeof line, "Characters%d", i + 1);
                bool is_act = (active == i + 1);
                draw_button(MAIN_R[i], line, is_act ? S->tag_active : "",
                            !dialog && cursor == i, is_act, err == ERR_NONE, false);
            }
            g_btn_sc = 0.6f;
            draw_button(MAIN_R[3], S->launch, "", !dialog && cursor == 3, false, true, true);
            draw_gear_button(MAIN_R[4], !dialog && cursor == 4);
        } else if (screen == SCR_ABOUT) {
            draw_text_center(S->about, 160, 6, 0.7f, COL_YELLOW);
            if (!draw_logo(1, 160, 26)) {
                draw_text_center(APP_NAME_1, 160, 34, 0.75f, COL_WHITE);
                draw_text_center(APP_NAME_2, 160, 62, 0.75f, COL_WHITE);
            }
            snprintf(line, sizeof line, "%s  %s", S->version_label, APP_VERSION);
            draw_text_center(line, 160, 110, 0.5f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->pack_label, pack_ver[0] ? pack_ver : "---");
            draw_text_center(line, 160, 127, 0.5f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->author_label, APP_AUTHOR);
            draw_text_center(line, 160, 144, 0.5f, COL_GRAY);
            draw_text_center(APP_TEAM, 160, 161, 0.5f, COL_GRAY);
            {   // repository URL (+ GitHub mark if gfx/github.png was added)
                float sc = 0.42f, w, h;
                text_size(APP_REPO, sc, &w, &h);
                float icon = g_ghicon ? 13.0f : 0.0f, gap = g_ghicon ? 4.0f : 0.0f;
                if (w + icon + gap > 304) { sc *= 304 / (w + icon + gap); text_size(APP_REPO, sc, &w, &h); }
                float x = 160 - (w + icon + gap) / 2, y = 182;
                if (g_ghicon) {
                    C2D_Image im = C2D_SpriteSheetGetImage(g_ghicon, 0);
                    C2D_ImageTint tint;
                    C2D_PlainImageTint(&tint, COL_WHITE, 1.0f);   // follows light/dark theme
                    float k = icon / im.subtex->height;
                    C2D_DrawImageAt(im, x, y + (h - icon) / 2, 0.5f, &tint, k, k);
                }
                draw_text(APP_REPO, x + icon + gap, y, sc, COL_GRAY);
            }
            draw_button(ABOUT_BACK, S->back, "", !dialog, false, true, true);
        } else if (screen == SCR_APPUPD) {
            draw_text_center(S->appupd_title, 160, 4, 0.7f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, APP_VERSION);
            draw_text_center(line, 160, 32, 0.5f, COL_GRAY);
            bool busy = upd_busy();
            draw_button(APPUPD_BTN, S->check_btn, "", !dialog && appupd_cursor == 0, false,
                        g_app.state != CHK_RUNNING && !busy && g_upd.state != UPD_DONE, true);

            int st = g_app.state;
            bool newer = st == CHK_DONE && has_digit(g_app.remote) && version_cmp(g_app.remote, APP_VERSION) > 0;
            int us = g_upd.state;

            if (us == UPD_DOWNLOADING || us == UPD_INSTALLING) {
                // progress bar
                float pct = g_upd.total ? (float)g_upd.now / (float)g_upd.total : 0.0f;
                if (pct > 1.0f) pct = 1.0f;
                snprintf(line, sizeof line, "%s %d%%", us == UPD_DOWNLOADING ? S->downloading : S->installing,
                         (int)(pct * 100));
                draw_text_center(line, 160, 112, 0.55f, COL_WHITE);
                Rect pb = INSTALL_BTN;
                C2D_DrawRectSolid(pb.x - 2, pb.y + 6, 0.5f, pb.w + 4, 22, BTN_VALUE);
                C2D_DrawRectSolid(pb.x, pb.y + 8, 0.5f, pb.w, 18, C2D_Color32(0x20, 0x22, 0x30, 0xFF));
                C2D_DrawRectSolid(pb.x, pb.y + 8, 0.5f, pb.w * pct, 18, C2D_Color32(0x2C, 0x78, 0xE0, 0xFF));
            } else if (us == UPD_DONE) {
                draw_text_center(g_upd.is_cia ? S->done_cia : S->done_3dsx, 160, 100, 0.52f, COL_GREEN);
                draw_button(INSTALL_BTN, g_upd.is_cia ? S->restart : S->quit, "",
                            !dialog && appupd_cursor == 1, false, true, true);
            } else {
                if (us == UPD_FAIL) {
                    const char *ft = g_upd.fail == UFAIL_FILE ? S->fail_file :
                                     g_upd.fail == UFAIL_BAD  ? S->fail_bad  :
                                     g_upd.fail == UFAIL_INSTALL ? S->fail_install : S->fail_net;
                    draw_text_center(ft, 160, 100, 0.5f, COL_RED);
                } else if (st == CHK_RUNNING) {
                    draw_text_center(S->checking, 160, 110, 0.55f, COL_GRAY);
                } else if (newer) {
                    draw_text_center(S->app_new, 160, 98, 0.52f, COL_GREEN);
                    snprintf(line, sizeof line, "%s: %s", S->latest_label, g_app.remote);
                    draw_text_center(line, 160, 120, 0.52f, COL_WHITE);
                } else if (st == CHK_DONE && has_digit(g_app.remote)) {
                    draw_text_center(S->app_latest, 160, 110, 0.52f, COL_GREEN);
                } else if (st == CHK_DONE || st == CHK_FAIL) {
                    draw_text_center(S->app_fail, 160, 110, 0.52f, COL_RED);
                }
                if (newer)
                    draw_button(INSTALL_BTN, S->install_btn, "", !dialog && appupd_cursor == 1, false, true, true);
            }
            draw_button(ABOUT_BACK, S->back, "", !dialog && appupd_cursor == 2, false, !busy, true);
        } else {
            const char *langname = cfg.lang == 0 ? S->lang_auto : (cfg.lang == 1 ? "日本語" : "English");
            const char *labels[LIST_N] = {
                S->set_language, S->set_dark, S->set_sfx, S->set_bgm, S->set_confirm, S->set_exit, S->set_check,
                S->set_appupd, S->set_reset, S->about
            };
            const char *values[LIST_N] = {
                langname,
                cfg.dark ? S->on : S->off,
                cfg.sfx ? S->on : S->off,
                cfg.bgm ? S->on : S->off,
                cfg.confirm ? S->on : S->off,
                cfg.exit_after ? S->on : S->off,
                cfg.check_update ? S->on : S->off,
                "", "", ""
            };
            static const bool centered[LIST_N] = { false, false, false, false, false, false, false, true, true, true };

            // rows (those peeking out of the list area get masked below)
            for (int i = 0; i < LIST_N; i++) {
                Rect r = row_rect(i, scroll);
                if (r.y + r.h < LIST_Y - 4 || r.y > LIST_Y + LIST_H + 4) continue;
                draw_button(r, labels[i], values[i], !dialog && set_cursor == i, false, true, centered[i]);
            }
            if (!draw_bg_strip(g_bot_bg, 320, 240, 0, LIST_Y))
                C2D_DrawRectSolid(0, 0, 0.5f, 320, LIST_Y, COL_BG);
            if (!draw_bg_strip(g_bot_bg, 320, 240, LIST_Y + LIST_H, 240))
                C2D_DrawRectSolid(0, LIST_Y + LIST_H, 0.5f, 320, 240 - LIST_Y - LIST_H, COL_BG);

            draw_text_center(S->settings_title, 160, 4, 0.7f, COL_YELLOW);
            draw_button(BACK_R, S->back, "", !dialog && set_cursor == LIST_N, false, true, true);

            // scroll bar
            const int max_scroll = LIST_CONTENT - LIST_H;
            C2D_DrawRectSolid(SB_X, LIST_Y, 0.5f, 8, LIST_H, C2D_Color32(0, 0, 0, 0x70));
            float ty = LIST_Y + (float)(LIST_H - THUMB_H) * (max_scroll ? (float)scroll / max_scroll : 0.0f);
            C2D_DrawRectSolid(SB_X, ty, 0.5f, 8, THUMB_H, t_mode == 2 ? BTN_VALUE : C2D_Color32(0xE8, 0xE8, 0xE8, 0xFF));
        }

        if (dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            C2D_DrawRectSolid(DLG_BOX.x - 2, DLG_BOX.y - 2, 0.5f, DLG_BOX.w + 4, DLG_BOX.h + 4, COL_YELLOW);
            C2D_DrawRectSolid(DLG_BOX.x, DLG_BOX.y, 0.5f, DLG_BOX.w, DLG_BOX.h, COL_BG);
            snprintf(line, sizeof line, S->confirm_msg, dlg_target);
            draw_text_center(line, 160, DLG_BOX.y + 22, 0.6f, COL_WHITE);
            draw_button(DLG_YES, S->yes, "", dlg_yes, false, true, true);
            draw_button(DLG_NO, S->no, "", !dlg_yes, false, true, true);
        }

        if (upd_dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            C2D_DrawRectSolid(UPD_BOX.x - 2, UPD_BOX.y - 2, 0.5f, UPD_BOX.w + 4, UPD_BOX.h + 4, COL_YELLOW);
            C2D_DrawRectSolid(UPD_BOX.x, UPD_BOX.y, 0.5f, UPD_BOX.w, UPD_BOX.h, COL_BG);
            draw_text_center(S->update_msg, 160, UPD_BOX.y + 14, 0.55f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, pack_ver);
            draw_text_center(line, 160, UPD_BOX.y + 54, 0.55f, COL_WHITE);
            snprintf(line, sizeof line, "%s: %s", S->latest_label, latest);
            draw_text_center(line, 160, UPD_BOX.y + 80, 0.55f, COL_GREEN);
            draw_button(DLG_OK, S->ok, "", true, false, true, true);
        } else if (app_dialog) {
            C2D_DrawRectSolid(0, 0, 0.5f, 320, 240, C2D_Color32(0, 0, 0, 170));
            C2D_DrawRectSolid(UPD_BOX.x - 2, UPD_BOX.y - 2, 0.5f, UPD_BOX.w + 4, UPD_BOX.h + 4, COL_YELLOW);
            C2D_DrawRectSolid(UPD_BOX.x, UPD_BOX.y, 0.5f, UPD_BOX.w, UPD_BOX.h, COL_BG);
            draw_text_center(S->app_dlg_msg, 160, UPD_BOX.y + 14, 0.55f, COL_YELLOW);
            snprintf(line, sizeof line, "%s: %s", S->current_label, APP_VERSION);
            draw_text_center(line, 160, UPD_BOX.y + 46, 0.52f, COL_WHITE);
            snprintf(line, sizeof line, "%s: %s", S->latest_label, g_app.remote);
            draw_text_center(line, 160, UPD_BOX.y + 68, 0.52f, COL_GREEN);
            draw_button(APP_YES, S->btn_update, "", app_yes, false, true, true);
            draw_button(APP_NO, S->btn_later, "", !app_yes, false, true, true);
        }

        C3D_FrameEnd(0);
    }

    sfx_exit();
    net_stop();
    if (g_romfs && g_chk.state != CHK_RUNNING && g_app.state != CHK_RUNNING) romfsExit();

    if (g_top_bg) C2D_SpriteSheetFree(g_top_bg);
    if (g_bot_bg) C2D_SpriteSheetFree(g_bot_bg);
    if (g_btns)   C2D_SpriteSheetFree(g_btns);
    if (g_logo)   C2D_SpriteSheetFree(g_logo);
    if (g_ghicon) C2D_SpriteSheetFree(g_ghicon);
    C2D_TextBufDelete(g_buf);
    C2D_FontFree(g_font);
    C2D_Fini();
    C3D_Fini();
    cfguExit();
    gfxExit();
    return 0;
}
