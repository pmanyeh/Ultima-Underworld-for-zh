/* spellbook.c: --enhance spellbook (docs/ENHANCEMENTS.md). A book icon in the rune bag's corner opens
   a list of the spells of each circle over the 3D view, with the game's own rune pictures. A click
   on a spell whose runes are all in the bag puts them on the shelf and closes the book; the player
   still casts it, and the level, mana and skill checks are the game's. The game stands still while
   the book is open: this runs inside the rune panel's click, as wdialog does, and reads only the
   game's own recorded mouse and keys, so a recording replays it. Port only; called from
   combat/RUNES.C. Spells the manual leaves out show only with --enhance spellbook-all. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compat.h"
#include "combat.h"
#include "gfx.h"
#include "player.h"
#include "sys.h"
#include "ui.h"
#include "items.h"
#include "sound.h"
#include "portgame.h"
#include "port.h"
#include "i18n/phrases.h"
#include "presentation/hd_label.h"
#include "presentation/localized.h"

/* The game's differences. */
#define VIEW_X   0x10                   /* the 3D view: UWEDIT.C's place_3d_view */
#define VIEW_TOP 0xB6                   /* its top row, bottom-up */
#define VIEW_W   0xD0
#define VIEW_H   0x80
#define PER_CIRCLE 8                    /* RUNES.C's player_cast: the circle is idx / 8 + 1 */
#define ROWS 4
/* spells[] rows the game's manual does not list (the manual's spell chapter) */
static const unsigned char unlisted[] = { 6, 7, 9, 15, 21, 24, 29, 37, 38, 46, 49, 52, 53, 62, 63 };
#define ARMAGEDDON 63

#define CIRCLES 8
#define STR_FIRST_SPELL (0x100 | STR_SPELLS)    /* block 6, string 256 on: spells[]'s names */
#define COLS 2
#define TAB_H 11
#define ICON_X 0xF1                     /* the book icon: the panel's corner under the runes */
#define ICON_TOP 0x5F
#define ICON_W 14
#define ICON_H 13
#define ICON_REL_X 16                   /* a click there: inplist's panel coordinates */
#define ICON_REL_Y 15

extern struct Spell far spells[];
void far port_rune_shelf(const unsigned char *runes, int count);    /* combat/RUNES.C */
const char *port_ui_text(const char *english);

enum { C_PAGE, C_EDGE, C_INK, C_DIM, C_RED, C_TAB, C_LIGHT, C_COVER, C_BLANK, COLOURS };
static const unsigned char want[COLOURS][3] = {
    { 53, 45, 31 }, { 24, 15, 7 }, { 12, 7, 3 }, { 30, 26, 20 }, { 45, 5, 3 },
    { 41, 33, 20 }, { 60, 55, 42 }, { 38, 8, 5 }, { 44, 37, 25 }
};
static unsigned char colour[COLOURS];
static int circle;                      /* the circle on show, kept between openings */

static void find_colours(void)
{
    uint8_t dac[768];
    int c, i;
    vga_get_dac(dac);
    for (c = 0; c < COLOURS; c++) {
        long best = 1L << 30;
        for (i = 1; i < 256; i++) {
            long dr = (long)dac[i * 3] - want[c][0], dg = (long)dac[i * 3 + 1] - want[c][1], db = (long)dac[i * 3 + 2] - want[c][2];
            long d = dr * dr + dg * dg + db * db;
            if (d < best) { best = d; colour[c] = (unsigned char)i; }
        }
    }
}

/* Whether the player can use a rune: in the bag, or any with --enhance all-runes (F11's Cheats), which
   leaves the bag and so the saved game as they are. RUNES.C and keys.c ask this too. */
int port_has_rune(int rune)
{
    return ENHANCED(ENH_ALL_RUNES) || (player->runebag[rune >> 3] >> (7 - (rune & 7)) & 1);
}
#define has_rune port_has_rune

/* RUNES.C's player_cast: --enhance free-cast (F11's Cheats) */
int port_free_cast(void) { return ENHANCED(ENH_FREE_CAST) != 0; }

static int is_unlisted(int idx)
{
    unsigned i;
    for (i = 0; i < sizeof unlisted; i++) if (unlisted[i] == idx) return 1;
    return 0;
}

/* A spell's runes, in casting order; returns how many. */
static int spell_runes(int idx, unsigned char *out)
{
    unsigned r = (unsigned)spells[idx].runes;
    unsigned char all[3];
    int i, n = 0;
    all[0] = (r >> 10) & 31; all[1] = (r >> 5) & 31; all[2] = r & 31;
    for (i = 0; i < 3; i++) if (all[i] < NUM_RUNES) out[n++] = all[i];
    return n;
}

static int complete(int idx)
{
    unsigned char r[3];
    int i, n = spell_runes(idx, r);
    for (i = 0; i < n; i++) if (!has_rune(r[i])) return 0;
    return n > 0;
}

/* The spells of a circle on show: the manual's first, in the game's order, then the rest. */
static int circle_spells(int c, int *out)
{
    int i, n = 0, pass;
    for (pass = 0; pass < 2; pass++)
        for (i = c * PER_CIRCLE; i < (c + 1) * PER_CIRCLE; i++)
            if (is_unlisted(i) == pass && (!pass || ENHANCED(ENH_SPELLBOOK_ALL))) out[n++] = i;
    return n;
}

static void fill_box(int x, int top, int w, int h, int c)
{
    set_the_color(colour[c]);
    rectangle(x, top, x + w - 1, top - h + 1);
}

/* s, a line in the game's language, at x and the row top (bottom-up) in the box w by h */
static void text(const char *s, int zh, int x, int top, int w, int h, int c, int align, int em)
{
    const char *line = zh ? s : (hd_label_enabled() ? port_ui_text(s) : NULL);
    if (line && hd_label_box(line, x, 199 - top, x + w, 199 - top + h, colour[c], align, em * 64, -1, 0.0f) == 0)
        return;
    if (zh) return;
    *foreground_color = colour[c];
    if (align == HD_LABEL_CENTRE) x += (w - string_width((char *)s)) / 2;
    string_to_screen((char *)s, x, top);
}

static void lit(const char *key, const char *en, int x, int top, int w, int h, int c, int align, int em)
{
    if (port_zh()) text(port_zh_lit(key), 1, x, top, w, h, c, align, em);
    else text(en, 0, x, top, w, h, c, align, em);
}

static int item_h(void) { return (VIEW_H - TAB_H - 3) / ROWS; }
static int item_w(void) { return (VIEW_W - 4) / COLS; }

/* A picture's darkening for a rune not in the bag: every other pixel in the page's dim colour */
static void dim_rune(int x, int top, int w, int h)
{
    int i, j;
    set_the_color(colour[C_BLANK]);
    for (j = 0; j < h; j++)
        for (i = j & 1; i < w; i += 2)
            rectangle(x + i, top - j, x + i, top - j);
}

static void draw_item(int idx, int col, int row)
{
    unsigned char r[3];

    int x = VIEW_X + 2 + col * item_w(), top = VIEW_TOP - TAB_H - 2 - row * item_h();
    int w = item_w(), h = item_h(), n = spell_runes(idx, r), ok = complete(idx), i;
    int un = is_unlisted(idx);

    fill_box(x + 1, top, w - 2, h - 1, ok ? C_LIGHT : C_PAGE);
    /* the pictures first: a picture drawn over a label takes the label away */
    for (i = 0; i < n; i++) {
        int rx = x + 4 + i * (w - 6) / 3;
        Transparency = 1;
        pic_to_screen(r[i] + FIRST_RUNESTONE, rx, top - 10, rx + 13, top - 23);
        Transparency = 0;
        if (!has_rune(r[i])) dim_rune(rx, top - 10, 14, 14);
    }
    text(get_string(STR_FIRST_SPELL + idx), 0, x + 3, top - 1, un ? w - 29 : w - 5, 8,
         idx == ARMAGEDDON ? C_RED : ok ? C_INK : C_DIM, HD_LABEL_LEFT, 8);
    if (un) lit("spellbook.unlisted", "*", x + w - 24, top - 1, 21, 8, C_DIM, HD_LABEL_RIGHT, 5);
    for (i = 0; i < n; i++) {
        static const char *const names[NUM_RUNES] = { "AN", "BET", "CORP", "DES", "EX", "FLAM", "GRAV", "HUR", "IN", "JUX", "KAL", "LOR",
            "MANI", "NOX", "ORT", "POR", "QUAS", "REL", "SANCT", "TYM", "UUS", "VAS", "WIS", "YLEM" };
        int rx = x + 4 + i * (w - 6) / 3;
        text(names[r[i]], 0, rx + 15, top - 13, (w - 6) / 3 - 15, 7, has_rune(r[i]) ? C_INK : C_DIM, HD_LABEL_LEFT, 5);
    }
}

static void draw_book(void)
{
    int list[PER_CIRCLE], n = circle_spells(circle, list), i;
    int tw = (VIEW_W * 2 / 3) / CIRCLES;
    char num[8], mana[40];
    mouse_hide();
    set_the_window(0, 0xC7, 0x13F, 0);
    fill_box(VIEW_X, VIEW_TOP, VIEW_W, VIEW_H, C_EDGE);
    fill_box(VIEW_X + 1, VIEW_TOP - 1, VIEW_W - 2, VIEW_H - 2, C_PAGE);
    for (i = 0; i < CIRCLES; i++) {
        fill_box(VIEW_X + 2 + i * tw, VIEW_TOP - 2, tw - 1, TAB_H - 1, i == circle ? C_EDGE : C_TAB);
        snprintf(num, sizeof num, "%d", i + 1);
        text(num, 0, VIEW_X + 2 + i * tw, VIEW_TOP - 3, tw - 1, 9, i == circle ? C_LIGHT : C_INK, HD_LABEL_CENTRE, 7);
    }
    if (port_zh()) snprintf(mana, sizeof mana, "第 %d 環　%d 法力", circle + 1, (circle + 1) * 3);
    else snprintf(mana, sizeof mana, "Circle %d, %d mana", circle + 1, (circle + 1) * 3);
    text(mana, port_zh(), VIEW_X + 4 + CIRCLES * tw, VIEW_TOP - 3, VIEW_W - 8 - CIRCLES * tw, 9, C_INK, HD_LABEL_RIGHT, 7);
    for (i = 0; i < n; i++) draw_item(list[i], i % COLS, i / COLS);
    mouse_show();
}

static void say_missing(int idx)
{
    static const char *const names[NUM_RUNES] = { "An", "Bet", "Corp", "Des", "Ex", "Flam", "Grav", "Hur", "In", "Jux", "Kal", "Lor",
        "Mani", "Nox", "Ort", "Por", "Quas", "Rel", "Sanct", "Tym", "Uus", "Vas", "Wis", "Ylem" };
    unsigned char r[3];
    char line[96];
    int i, n = spell_runes(idx, r), used, first = 1;
    used = snprintf(line, sizeof line, "%s", port_zh() ? port_zh_lit("spellbook.missing") : "Runes you lack: ");
    for (i = 0; i < n; i++)
        if (!has_rune(r[i])) {
            used += snprintf(line + used, sizeof line - used, "%s%s", first ? "" : " ", names[r[i]]);
            first = 0;
        }
    snprintf(line + used, sizeof line - used, "\n");
    scroll_print(line);
}

/* The spell at the pointer, or -1 */
static int spell_at(int x, int y)
{
    int list[PER_CIRCLE], n = circle_spells(circle, list);
    int col = (x - VIEW_X - 2) / item_w(), row = (VIEW_TOP - TAB_H - 2 - y) / item_h(), i;
    if (x < VIEW_X + 2 || y > VIEW_TOP - TAB_H - 2 || col >= COLS || row >= ROWS) return -1;
    i = row * COLS + col;
    return i < n ? list[i] : -1;
}

/* 1 when the click chose a spell or closed the book */
static int click(int x, int y)
{
    int tw = (VIEW_W * 2 / 3) / CIRCLES, idx;
    if (x < VIEW_X || x >= VIEW_X + VIEW_W || y > VIEW_TOP || y <= VIEW_TOP - VIEW_H) return 1;
    if (y > VIEW_TOP - TAB_H - 2) {
        int t = (x - VIEW_X - 2) / tw;
        if (x >= VIEW_X + 2 && t < CIRCLES && t != circle) { circle = t; play_effect_here(4, 64, 0); draw_book(); }
        return 0;
    }
    if ((idx = spell_at(x, y)) < 0) return 0;
    if (!complete(idx)) {
        play_effect_here(45, 64, 0);        /* as rune-keys refuses a rune you lack */
        say_missing(idx);
        return 0;
    }
    {
        unsigned char r[3];
        int n = spell_runes(idx, r);
        play_effect_here(3, 64, 0);         /* as rune-keys takes a rune */
        port_rune_shelf(r, n);
        if (idx == ARMAGEDDON) {            /* only on the shelf yet; casting it is still the player's */
            if (port_zh()) scroll_print((char *)port_zh_lit("spellbook.armageddon"));
            else scroll_print("Beware: this spell destroys all you carry and every object on this level.\n");
        }
    }
    return 1;
}

static void run_book(void)
{
    int key, wl = *wleft, wt = *wtop, wr = *wright, wb = *wbot, done = 0;
    find_colours();
    mouse_release(0);
    force_mouse_cursor(0x106C);       /* the plain arrow: the regions under it are the hidden view's */
    draw_book();
    while (!done) {
        key = mouse_get_input();
        if (key == -1) continue;
        if (key == 1 || key == 2) {
            int16 x, y;
            mouse_getxy(&x, &y);
            mouse_release(0);
            done = key == 2 || click(x, y);     /* the right button shuts the book */
        } else if (key == 27)
            done = 1;
        else if (key >= '1' && key < '1' + CIRCLES && key - '1' != circle) {
            circle = key - '1';
            draw_book();
        } else if (key == KEY_LEFT && circle > 0) {
            circle--;
            draw_book();
        } else if (key == KEY_RIGHT && circle < CIRCLES - 1) {
            circle++;
            draw_book();
        }
    }
    unforce_mouse_cursor(3);
    set_the_window(wl, wt, wr, wb);
    editchng(0x7FFE);                       /* the view and whatever else it covered, drawn again */
}

/* RUNES.C's RedispRune: the book icon, in the panel's corner */
void port_spellbook_icon(void)
{
    int x = ICON_X, top = ICON_TOP;
    if (!ENHANCED(ENH_SPELLBOOK)) return;
    {   /* a test switch: UWnPORT_BOOK_RUNES, six hex digits, the bag's runes (a staged player has none) */
        static int granted;
        const char *m = getenv("UW2PORT_BOOK_RUNES");
        if (!granted && m) {
            unsigned long v = strtoul(m, NULL, 16);
            granted = 1;
            player->runebag[0] = (unsigned char)(v >> 16);
            player->runebag[1] = (unsigned char)(v >> 8);
            player->runebag[2] = (unsigned char)v;
            RedispRune();
            return;
        }
    }
    find_colours();
    fill_box(x, top, ICON_W, ICON_H, C_EDGE);
    fill_box(x + 1, top - 1, ICON_W - 2, ICON_H - 2, C_COVER);
    fill_box(x + 3, top - 1, 1, ICON_H - 2, C_EDGE);             /* the spine's fold */
    fill_box(x + 5, top - 3, ICON_W - 8, 2, C_LIGHT);            /* the title on the cover */
    fill_box(x + 5, top - 7, ICON_W - 8, 1, C_LIGHT);
    fill_box(x + 5, top - 9, ICON_W - 8, 1, C_LIGHT);
}

/* RUNES.C's mous_in_rune, the strip under the runes: 1 when the click was on the icon */
int port_spellbook_click(int x, int y, int right)
{
    if (!ENHANCED(ENH_SPELLBOOK) || x >= ICON_REL_X || y >= ICON_REL_Y) return 0;
    if (right) {
        if (port_zh()) scroll_print((char *)port_zh_lit("spellbook.about"));
        else scroll_print("Your spell book: the spells of each circle.\n");
        mouse_release(1);
    } else
        run_book();
    return 1;
}
