// CharSwitcher Ver.0.2 - character folder switcher for More!CTGP-7
// Target: sdmc:/CTGP-7/MyStuff/Characters{1,2,3}
// The folder named "Characters" (no number) is the active one.
// No state file for folders: the active slot is detected from which folders exist.
//
// Top screen   : title / current slot / status / version
// Bottom screen: slot select + settings (buttons AND touch)
// Settings are saved to sdmc:/3ds/CharSwitcher/config.ini

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

#define APP_VERSION "Ver.0.2"
#define APP_NAME_1  "More！CTGP-7"
#define APP_NAME_2  "Character Switcher"
#define APP_AUTHOR  "yk"
#define APP_TEAM    "3DSProject Team"

#define BASE        "sdmc:/CTGP-7/MyStuff/"
#define ACTIVE_NAME BASE "Characters"
#define SLOTS       3

#define CFG_DIR  "sdmc:/3ds/CharSwitcher"
#define CFG_PATH CFG_DIR "/config.ini"

// URL of the pack version file on GitHub (raw). Override with
// pack_url=... in config.ini. (A URL containing "OWNER" is treated as unset.)
#define PACK_URL_DEFAULT "https://raw.githubusercontent.com/yuuki3ds/More-CTGP-7-Character-Switcher/main/version/morever.ini"

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
} Strings;

static const Strings STR_EN = {
    "CharSwitcher (More!CTGP-7)",
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
    "CTGP-7 was not found.\nInstall CTGP-7, or set ctgp7_id in\nsdmc:/3ds/CharSwitcher/config.ini",
    "Pack version",
    "Check pack updates at startup",
    "A pack update is available.",
    "Current",
    "Latest",
    "OK",
};

static const Strings STR_JA = {
    "CharSwitcher (More!CTGP-7)",
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
    "CTGP-7が見つかりません。\nCTGP-7をインストールするか、\nsdmc:/3ds/CharSwitcher/config.ini の\nctgp7_id を設定してください。",
    "パックバージョン",
    "起動時にパックの更新を確認",
    "パックのアップデートがあります。",
    "現在",
    "最新",
    "OK",
};

// ------------------------------------------------------------ config

typedef struct {
    int lang;        // 0 = auto, 1 = Japanese, 2 = English
    int confirm;     // 0/1
    int exit_after;  // 0/1
    unsigned long long ctgp7_id;  // 0 = not known yet (auto-detect on first launch)
    int check_update;             // check pack version on GitHub at startup (0/1)
    char pack_url[200];
} Config;

static void config_defaults(Config *c) {
    c->lang = 0;
    c->confirm = 1;
    c->exit_after = 0;
    c->ctgp7_id = 0;
    c->check_update = 1;
    snprintf(c->pack_url, sizeof c->pack_url, "%s", PACK_URL_DEFAULT);
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
            snprintf(c->pack_url, sizeof c->pack_url, "%s", val);
            continue;
        }
        int v = atoi(val);
        if (!strcmp(key, "lang") && v >= 0 && v <= 2) c->lang = v;
        else if (!strcmp(key, "confirm")) c->confirm = v ? 1 : 0;
        else if (!strcmp(key, "exit_after")) c->exit_after = v ? 1 : 0;
        else if (!strcmp(key, "check_update")) c->check_update = v ? 1 : 0;
    }
    fclose(f);
}

static void config_save(const Config *c) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(CFG_DIR, 0777);
    FILE *f = fopen(CFG_PATH, "w");
    if (!f) return;
    fprintf(f, "lang=%d\nconfirm=%d\nexit_after=%d\nctgp7_id=%016llX\ncheck_update=%d\n",
            c->lang, c->confirm, c->exit_after, c->ctgp7_id, c->check_update);
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

static CheckCtx g_chk;
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
    curl_easy_setopt(c, CURLOPT_USERAGENT, "CharSwitcher/" APP_VERSION);
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

// Starts the background check. Returns false if it could not be started.
static bool net_start(const char *url) {
    g_soc_buf = (u32 *)memalign(0x1000, 0x100000);
    if (!g_soc_buf) return false;
    if (R_FAILED(socInit(g_soc_buf, 0x100000))) {
        free(g_soc_buf);
        g_soc_buf = NULL;
        return false;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    g_net_ready = true;

    snprintf(g_chk.url, sizeof g_chk.url, "%s", url);
    g_chk.remote[0] = '\0';
    g_chk.quit = false;
    g_chk.state = CHK_RUNNING;

    s32 prio = 0x30;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    prio -= 1;
    if (prio < 0x18) prio = 0x18;

    Thread t = threadCreate(check_thread, &g_chk, 128 * 1024, prio, -1, true);
    if (!t) { g_chk.state = CHK_FAIL; return false; }
    return true;
}

static void net_stop(void) {
    if (!g_net_ready) return;
    g_chk.quit = true;
    for (int i = 0; i < 40 && g_chk.state == CHK_RUNNING; i++)
        svcSleepThread(50000000LL);                    // wait up to ~2 s
    if (g_chk.state != CHK_RUNNING) {                  // otherwise just leak; we are exiting
        curl_global_cleanup();
        socExit();
        free(g_soc_buf);
    }
    g_net_ready = false;
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

static const u32 WHITE   = 0xFFFFFFFF;  // ABGR
static u32 COL_WHITE, COL_YELLOW, COL_GREEN, COL_RED, COL_GRAY;
static u32 COL_BG, COL_PANEL, COL_PANEL_SEL, COL_PANEL_ACT, COL_BORDER;

static void colors_init(void) {
    (void)WHITE;
    COL_WHITE     = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    COL_YELLOW    = C2D_Color32(0xFF, 0xE0, 0x40, 0xFF);
    COL_GREEN     = C2D_Color32(0x60, 0xFF, 0x80, 0xFF);
    COL_RED       = C2D_Color32(0xFF, 0x70, 0x70, 0xFF);
    COL_GRAY      = C2D_Color32(0xB0, 0xB0, 0xB0, 0xFF);
    COL_BG        = C2D_Color32(0x10, 0x10, 0x18, 0xFF);
    COL_PANEL     = C2D_Color32(0x28, 0x2A, 0x38, 0xFF);
    COL_PANEL_SEL = C2D_Color32(0x3A, 0x40, 0x60, 0xFF);
    COL_PANEL_ACT = C2D_Color32(0x1E, 0x50, 0x34, 0xFF);
    COL_BORDER    = C2D_Color32(0x50, 0x54, 0x68, 0xFF);
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

static void draw_text(const char *s, float x, float y, float sc, u32 col) {
    C2D_Text t;
    parse_text(&t, s);
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
    u32 fill = active ? COL_PANEL_ACT : (sel ? COL_PANEL_SEL : COL_PANEL);
    u32 border = sel ? COL_YELLOW : COL_BORDER;
    u32 tcol = !enabled ? COL_GRAY : (active ? COL_GREEN : COL_WHITE);

    C2D_DrawRectSolid(r.x - 2, r.y - 2, 0.5f, r.w + 4, r.h + 4, border);
    C2D_DrawRectSolid(r.x, r.y, 0.5f, r.w, r.h, fill);

    float w, h;
    text_size(label, 0.6f, &w, &h);
    float ty = r.y + (r.h - h) / 2;
    if (center) draw_text_center(label, r.x + r.w / 2, ty, 0.6f, tcol);
    else draw_text(label, r.x + 14, ty, 0.6f, tcol);

    if (value && value[0]) {
        text_size(value, 0.6f, &w, &h);
        draw_text_right(value, r.x + r.w - 14, r.y + (r.h - h) / 2, 0.6f,
                        active ? COL_GREEN : COL_YELLOW);
    }
}

static bool hit(Rect r, touchPosition t) {
    return t.px >= r.x && t.px < r.x + r.w && t.py >= r.y && t.py < r.y + r.h;
}

// ------------------------------------------------------------ UI layout (bottom screen 320x240)

typedef enum { SCR_MAIN, SCR_SETTINGS, SCR_ABOUT } Screen;

#define MAIN_N 5
static const Rect MAIN_R[MAIN_N] = {
    {20,  36, 280, 34},
    {20,  76, 280, 34},
    {20, 116, 280, 34},
    {20, 160, 280, 34},   // Launch CTGP-7
    {20, 202, 280, 30},   // Settings
};

#define SET_N 7
static const Rect SET_R[SET_N] = {
    {20,  30, 280, 25},   // Language
    {20,  59, 280, 25},   // Confirm
    {20,  88, 280, 25},   // Exit after
    {20, 117, 280, 25},   // Check pack updates
    {20, 146, 280, 25},   // Reset
    {20, 175, 280, 25},   // About
    {20, 206, 280, 28},   // Back
};

static const Rect ABOUT_BACK = {20, 198, 280, 32};

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

int main(void) {
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

    bool sys_ja = false;
    u8 lang = CFG_LANGUAGE_EN;
    if (R_SUCCEEDED(CFGU_GetSystemLanguage(&lang)) && lang == CFG_LANGUAGE_JP) sys_ja = true;

    Config cfg;
    config_load(&cfg);

    char pack_ver[64];
    load_pack_version(pack_ver, sizeof pack_ver);

    ErrCode err = ERR_NONE;
    int active = detect_active(&err);
    if (active < 0) active = 0;

    Screen screen = SCR_MAIN;
    int cursor = 0;                        // main: 0-2 slots, 3 settings
    if (active > 0) cursor = active - 1;
    int set_cursor = 0;                    // settings: 0..4

    bool dialog = false;
    int dlg_target = 0;
    bool dlg_yes = true;

    MsgKind msg_kind = MSG_NONE;
    int msg_arg = 0;

    bool running = true;

    // ---- pack update check (background) ----
    bool upd_dialog = false;
    char latest[64] = "";
    bool chk_pending = false;
    g_romfs = R_SUCCEEDED(romfsInit());
    if (cfg.check_update && cfg.pack_url[0] && !strstr(cfg.pack_url, "OWNER"))
        chk_pending = net_start(cfg.pack_url);

    // ---- actions (as small lambdas via nested blocks would be GNU-only; use inline code) ----
    while (aptMainLoop() && running) {
        hidScanInput();
        u32 down = hidKeysDown();
        touchPosition tp;
        hidTouchRead(&tp);
        bool tap = (down & KEY_TOUCH) != 0;

        if (down & KEY_START) break;

        const Strings *S = get_strings(&cfg, sys_ja);

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

        // ---------- input ----------
        int do_slot = 0;          // slot to request a switch to (1-3), 0 = none
        bool do_perform = false;
        bool do_launch = false;

        if (upd_dialog) {
            if ((down & (KEY_A | KEY_B)) || (tap && hit(DLG_OK, tp))) upd_dialog = false;
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
                    if (hit(MAIN_R[i], tp)) { cursor = i; act = i; break; }
            }
            if (act >= 0 && act < 3) do_slot = act + 1;
            else if (act == 3) do_launch = true;
            else if (act == 4) { screen = SCR_SETTINGS; set_cursor = 0; }
        } else if (screen == SCR_ABOUT) {
            if (down & (KEY_B | KEY_A)) screen = SCR_SETTINGS;
            if (tap && hit(ABOUT_BACK, tp)) screen = SCR_SETTINGS;
        } else { // SCR_SETTINGS
            int act = -1, dir = 0;
            if (down & KEY_UP)    set_cursor = (set_cursor + SET_N - 1) % SET_N;
            if (down & KEY_DOWN)  set_cursor = (set_cursor + 1) % SET_N;
            if (down & KEY_LEFT)  { dir = -1; act = set_cursor; }
            if (down & KEY_RIGHT) { dir = +1; act = set_cursor; }
            if (down & KEY_A)     { dir = +1; act = set_cursor; }
            if (down & KEY_B)     screen = SCR_MAIN;
            if (tap) {
                for (int i = 0; i < SET_N; i++)
                    if (hit(SET_R[i], tp)) { set_cursor = i; act = i; dir = +1; break; }
            }
            if (act >= 0 && !(down & KEY_B)) {
                bool changed = false;
                switch (act) {
                case 0:
                    if (down & (KEY_LEFT | KEY_RIGHT | KEY_A | KEY_TOUCH)) {
                        cfg.lang = (cfg.lang + 3 + dir) % 3;
                        changed = true;
                    }
                    break;
                case 1: cfg.confirm = !cfg.confirm; changed = true; break;
                case 2: cfg.exit_after = !cfg.exit_after; changed = true; break;
                case 3: cfg.check_update = !cfg.check_update; changed = true; break;
                case 4:
                    if (!(down & (KEY_LEFT | KEY_RIGHT))) { config_defaults(&cfg); changed = true; }
                    break;
                case 5:
                    if (!(down & (KEY_LEFT | KEY_RIGHT))) {
                        load_pack_version(pack_ver, sizeof pack_ver);
                        screen = SCR_ABOUT;
                    }
                    break;
                case 6:
                    if (!(down & (KEY_LEFT | KEY_RIGHT))) screen = SCR_MAIN;
                    break;
                }
                if (changed) config_save(&cfg);
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
            if (msg_kind == MSG_DONE && cfg.exit_after) running = false;
        }

        // ---------- launch CTGP-7 ----------
        if (do_launch) {
            if (cfg.ctgp7_id == 0) {
                cfg.ctgp7_id = find_ctgp7();
                if (cfg.ctgp7_id) config_save(&cfg);
            }
            if (cfg.ctgp7_id) {
                aptSetChainloader(cfg.ctgp7_id, MEDIATYPE_SD);
                running = false;
            } else {
                msg_kind = MSG_NOLAUNCH;
                msg_arg = 0;
            }
        }

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
        draw_text(APP_NAME_1, 14, 6, 0.75f, COL_YELLOW);
        draw_text(APP_NAME_2, 14, 34, 0.75f, COL_YELLOW);
        if (err != ERR_NONE) {
            draw_text(errtxt, 14, 82, 0.55f, COL_RED);
            draw_text(S->err_footer, 14, 160, 0.5f, COL_GRAY);
        } else {
            if (active == 0) snprintf(line, sizeof line, "%s", S->current_none);
            else snprintf(line, sizeof line, S->current, active);
            draw_text(line, 14, 82, 0.65f, COL_WHITE);
            if (msg[0]) {
                bool bad = (msg_kind == MSG_FAIL1 || msg_kind == MSG_FAIL2 || msg_kind == MSG_NOLAUNCH);
                draw_text(msg, 14, 120, 0.55f, bad ? COL_RED : COL_GREEN);
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

        if (screen == SCR_MAIN) {
            draw_text_center(S->select_slot, 160, 8, 0.7f, COL_YELLOW);
            for (int i = 0; i < SLOTS; i++) {
                snprintf(line, sizeof line, "Characters%d", i + 1);
                bool is_act = (active == i + 1);
                draw_button(MAIN_R[i], line, is_act ? S->tag_active : "",
                            !dialog && cursor == i, is_act, err == ERR_NONE, false);
            }
            draw_button(MAIN_R[3], S->launch, "", !dialog && cursor == 3, false, true, true);
            draw_button(MAIN_R[4], S->settings, "", !dialog && cursor == 4, false, true, true);
        } else if (screen == SCR_ABOUT) {
            draw_text_center(S->about, 160, 6, 0.7f, COL_YELLOW);
            draw_text_center(APP_NAME_1, 160, 34, 0.8f, COL_WHITE);
            draw_text_center(APP_NAME_2, 160, 62, 0.8f, COL_WHITE);
            snprintf(line, sizeof line, "%s  %s", S->version_label, APP_VERSION);
            draw_text_center(line, 160, 98, 0.6f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->pack_label, pack_ver[0] ? pack_ver : "---");
            draw_text_center(line, 160, 120, 0.6f, COL_GRAY);
            snprintf(line, sizeof line, "%s  %s", S->author_label, APP_AUTHOR);
            draw_text_center(line, 160, 146, 0.6f, COL_GRAY);
            draw_text_center(APP_TEAM, 160, 170, 0.6f, COL_GRAY);
            draw_button(ABOUT_BACK, S->back, "", !dialog, false, true, true);
        } else {
            draw_text_center(S->settings_title, 160, 4, 0.7f, COL_YELLOW);
            const char *langname = cfg.lang == 0 ? S->lang_auto : (cfg.lang == 1 ? "日本語" : "English");
            draw_button(SET_R[0], S->set_language, langname, !dialog && set_cursor == 0, false, true, false);
            draw_button(SET_R[1], S->set_confirm, cfg.confirm ? S->on : S->off,
                        !dialog && set_cursor == 1, false, true, false);
            draw_button(SET_R[2], S->set_exit, cfg.exit_after ? S->on : S->off,
                        !dialog && set_cursor == 2, false, true, false);
            draw_button(SET_R[3], S->set_check, cfg.check_update ? S->on : S->off,
                        !dialog && set_cursor == 3, false, true, false);
            draw_button(SET_R[4], S->set_reset, "", !dialog && set_cursor == 4, false, true, true);
            draw_button(SET_R[5], S->about, "", !dialog && set_cursor == 5, false, true, true);
            draw_button(SET_R[6], S->back, "", !dialog && set_cursor == 6, false, true, true);
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
        }

        C3D_FrameEnd(0);
    }

    net_stop();
    if (g_romfs && g_chk.state != CHK_RUNNING) romfsExit();

    C2D_TextBufDelete(g_buf);
    C2D_FontFree(g_font);
    C2D_Fini();
    C3D_Fini();
    cfguExit();
    gfxExit();
    return 0;
}
