/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --font H:/Programacion/Arduino/Espacio1/ui/SquareLineStudio/assets/fonts/Montserrat-Bold.ttf -o H:/Programacion/Arduino/Espacio1/ui/SquareLineStudio/assets/fonts\ui_font_Montserrat_12_px.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#include "../ui.h"

#ifndef UI_FONT_MONTSERRAT_12_PX
#define UI_FONT_MONTSERRAT_12_PX 1
#endif

#if UI_FONT_MONTSERRAT_12_PX

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf0, 0xc0,

    /* U+0022 "\"" */
    0xff, 0xf0,

    /* U+0023 "#" */
    0x36, 0x26, 0xff, 0x26, 0x24, 0x64, 0xff, 0x64,
    0x6c,

    /* U+0024 "$" */
    0x10, 0x21, 0xfe, 0xad, 0x1e, 0x1f, 0xf, 0x97,
    0xfd, 0xf0, 0x80,

    /* U+0025 "%" */
    0x61, 0xa4, 0xc9, 0x22, 0x58, 0x6d, 0x86, 0x91,
    0x24, 0xc9, 0x61, 0x80,

    /* U+0026 "&" */
    0x38, 0x6c, 0x6c, 0x38, 0x72, 0xfb, 0xce, 0xce,
    0x7f,

    /* U+0027 "'" */
    0xfc,

    /* U+0028 "(" */
    0x6f, 0x6d, 0xb6, 0xd9, 0xb0,

    /* U+0029 ")" */
    0xd9, 0xb6, 0xdb, 0x6f, 0x60,

    /* U+002A "*" */
    0x27, 0xdd, 0xf2, 0x0,

    /* U+002B "+" */
    0x30, 0xc3, 0x3f, 0x30, 0xc0,

    /* U+002C "," */
    0xfe,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x18, 0xc4, 0x63, 0x11, 0x8c, 0x46, 0x31, 0x0,

    /* U+0030 "0" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xf1, 0xb6, 0x38,

    /* U+0031 "1" */
    0xf3, 0x33, 0x33, 0x33, 0x30,

    /* U+0032 "2" */
    0xfa, 0x30, 0xc3, 0x1c, 0xe7, 0x18, 0xfc,

    /* U+0033 "3" */
    0xfc, 0x63, 0x8c, 0x3c, 0x70, 0xe3, 0xf8,

    /* U+0034 "4" */
    0x18, 0x18, 0x30, 0x70, 0x6c, 0xcc, 0xff, 0xc,
    0xc,

    /* U+0035 "5" */
    0x7c, 0xc1, 0x83, 0x7, 0xc0, 0xc1, 0xa3, 0xfc,

    /* U+0036 "6" */
    0x3e, 0xc3, 0x7, 0xee, 0x78, 0xf1, 0xa3, 0x3c,

    /* U+0037 "7" */
    0xff, 0x9b, 0x30, 0x61, 0x83, 0x6, 0x18, 0x30,

    /* U+0038 "8" */
    0x7d, 0x8f, 0x1e, 0x37, 0xd8, 0xf1, 0xe3, 0x7c,

    /* U+0039 "9" */
    0x79, 0x8b, 0x1e, 0x37, 0xe0, 0xc1, 0x86, 0xf8,

    /* U+003A ":" */
    0xc0, 0xc,

    /* U+003B ";" */
    0xc3, 0xf8,

    /* U+003C "<" */
    0x4, 0x7f, 0x30, 0xf0, 0x70, 0x40,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x3, 0x87, 0x83, 0x7b, 0x80, 0x0,

    /* U+003F "?" */
    0x7c, 0x8c, 0x18, 0x30, 0xc3, 0x86, 0x0, 0x18,

    /* U+0040 "@" */
    0x1f, 0x6, 0x11, 0xff, 0x7d, 0xdf, 0x1b, 0xe3,
    0x7c, 0x6f, 0xdd, 0x7f, 0xc6, 0x0, 0x7e, 0x0,

    /* U+0041 "A" */
    0x1c, 0xe, 0x5, 0x86, 0xc3, 0x63, 0x19, 0xfd,
    0x83, 0xc1, 0x80,

    /* U+0042 "B" */
    0xfc, 0xc6, 0xc6, 0xc6, 0xfe, 0xc3, 0xc3, 0xc3,
    0xfe,

    /* U+0043 "C" */
    0x3e, 0x63, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x63,
    0x3e,

    /* U+0044 "D" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc6,
    0xfc,

    /* U+0045 "E" */
    0xff, 0x83, 0x6, 0xf, 0xd8, 0x30, 0x60, 0xfe,

    /* U+0046 "F" */
    0xff, 0x83, 0x6, 0xf, 0xd8, 0x30, 0x60, 0xc0,

    /* U+0047 "G" */
    0x3e, 0x63, 0xc0, 0xc0, 0xc3, 0xc3, 0xc3, 0x63,
    0x3f,

    /* U+0048 "H" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3,
    0xc3,

    /* U+0049 "I" */
    0xff, 0xff, 0xc0,

    /* U+004A "J" */
    0x7c, 0x30, 0xc3, 0xc, 0x30, 0xd3, 0x78,

    /* U+004B "K" */
    0xc6, 0xce, 0xdc, 0xd8, 0xf8, 0xfc, 0xcc, 0xc6,
    0xc7,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xfc,

    /* U+004D "M" */
    0xc0, 0xf8, 0x7e, 0x1f, 0xcf, 0xf2, 0xf7, 0xbc,
    0xcf, 0x33, 0xc0, 0xc0,

    /* U+004E "N" */
    0xc3, 0xe3, 0xf3, 0xf3, 0xdb, 0xcf, 0xcf, 0xc7,
    0xc3,

    /* U+004F "O" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0x6,
    0xc6, 0x3e, 0x0,

    /* U+0050 "P" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x7f, 0xb0, 0x60, 0xc0,

    /* U+0051 "Q" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xd, 0x8c,
    0x7e, 0xc, 0x83, 0xc0,

    /* U+0052 "R" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x7f, 0xb3, 0x66, 0xc6,

    /* U+0053 "S" */
    0x7f, 0x8b, 0x7, 0xcf, 0xc7, 0xc1, 0xc3, 0xfc,

    /* U+0054 "T" */
    0xfe, 0x30, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,

    /* U+0055 "U" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0056 "V" */
    0xc1, 0xf0, 0x98, 0xcc, 0x63, 0x61, 0xb0, 0xf0,
    0x38, 0x1c, 0x0,

    /* U+0057 "W" */
    0xc3, 0xd, 0x8c, 0x66, 0x79, 0x99, 0xe6, 0x37,
    0xb0, 0xf3, 0xc3, 0xcf, 0x7, 0x38, 0x18, 0x60,

    /* U+0058 "X" */
    0x63, 0x33, 0xd, 0x83, 0x81, 0xc1, 0xe0, 0xd8,
    0xce, 0xe3, 0x0,

    /* U+0059 "Y" */
    0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18, 0x18,
    0x18,

    /* U+005A "Z" */
    0xfe, 0x1c, 0x70, 0xc3, 0x86, 0x18, 0x70, 0xfe,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0x41, 0x86, 0x8, 0x30, 0xc1, 0x6, 0x18, 0x20,
    0xc3,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x21, 0xc5, 0x16, 0xca, 0x20,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x42,

    /* U+0061 "a" */
    0xf8, 0x30, 0xdf, 0xcf, 0x37, 0xc0,

    /* U+0062 "b" */
    0xc1, 0x83, 0x7, 0xee, 0xf8, 0xf1, 0xe3, 0xed,
    0xf8,

    /* U+0063 "c" */
    0x3c, 0xcb, 0x6, 0xc, 0xc, 0x8f, 0x0,

    /* U+0064 "d" */
    0x6, 0xc, 0x1b, 0xfe, 0xf8, 0xf1, 0xe3, 0xee,
    0xfc,

    /* U+0065 "e" */
    0x38, 0x8b, 0x1f, 0xfc, 0xc, 0x8f, 0x0,

    /* U+0066 "f" */
    0x36, 0x6f, 0x66, 0x66, 0x66,

    /* U+0067 "g" */
    0x7f, 0xdf, 0x1e, 0x3c, 0x6f, 0xc1, 0xa3, 0x7c,

    /* U+0068 "h" */
    0xc1, 0x83, 0x7, 0xec, 0x78, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0069 "i" */
    0x30, 0xff, 0xfc,

    /* U+006A "j" */
    0x3, 0x0, 0x33, 0x33, 0x33, 0x33, 0xe0,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0x7d, 0xdf, 0x3e, 0x7c, 0xcd,
    0x8c,

    /* U+006C "l" */
    0xff, 0xff, 0xf0,

    /* U+006D "m" */
    0xfb, 0xb3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcc,

    /* U+006E "n" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0x80,

    /* U+006F "o" */
    0x38, 0xdb, 0x1e, 0x3c, 0x6d, 0x8e, 0x0,

    /* U+0070 "p" */
    0xfd, 0xdf, 0x1e, 0x3c, 0x7d, 0xbf, 0x60, 0xc0,

    /* U+0071 "q" */
    0x7f, 0xdf, 0x1e, 0x3c, 0x7d, 0xdf, 0x83, 0x6,

    /* U+0072 "r" */
    0xfe, 0xcc, 0xcc, 0xc0,

    /* U+0073 "s" */
    0x7f, 0xf, 0x3f, 0x3e, 0x3f, 0x80,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x66, 0x30,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xdf, 0x80,

    /* U+0076 "v" */
    0xc7, 0x8d, 0xb3, 0x63, 0xc7, 0xe, 0x0,

    /* U+0077 "w" */
    0xc6, 0x69, 0xcd, 0xb9, 0x35, 0xe3, 0xbc, 0x73,
    0xc, 0x60,

    /* U+0078 "x" */
    0xe6, 0xd8, 0xe0, 0xc3, 0x8d, 0xb1, 0x80,

    /* U+0079 "y" */
    0xc7, 0x8d, 0xb3, 0x63, 0x87, 0xe, 0x18, 0xe0,

    /* U+007A "z" */
    0xfc, 0x73, 0x8c, 0x73, 0x8f, 0xc0,

    /* U+007B "{" */
    0x36, 0x66, 0x6c, 0x66, 0x66, 0x63,

    /* U+007C "|" */
    0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xc6, 0x66, 0x63, 0x66, 0x66, 0x6c,

    /* U+007E "~" */
    0x66, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 54, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 55, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 84, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 6, .adv_w = 138, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 122, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 168, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 140, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 44, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 48, .adv_w = 69, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 53, .adv_w = 69, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 58, .adv_w = 83, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 62, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 67, .adv_w = 50, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 68, .adv_w = 74, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 69, .adv_w = 50, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 75, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 78, .adv_w = 130, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 75, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 113, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 132, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 114, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 127, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 50, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 50, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 164, .adv_w = 115, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 167, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 173, .adv_w = 113, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 199, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 197, .adv_w = 147, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 147, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 141, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 159, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 129, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 123, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 148, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 155, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 63, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 104, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 142, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 116, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 183, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 155, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 162, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 141, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 162, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 347, .adv_w = 141, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 151, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 143, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 223, .box_w = 14, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 137, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 130, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 129, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 71, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 441, .adv_w = 75, .box_w = 6, .box_h = 12, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 450, .adv_w = 71, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 456, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 461, .adv_w = 96, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 462, .adv_w = 115, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 463, .adv_w = 118, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 132, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 113, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 133, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 121, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 74, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 514, .adv_w = 133, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 58, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 526, .adv_w = 59, .box_w = 4, .box_h = 13, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 533, .adv_w = 127, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 58, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 201, .box_w = 10, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 554, .adv_w = 133, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 126, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 576, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 584, .adv_w = 83, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 102, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 84, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 599, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 115, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 613, .adv_w = 180, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 114, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 115, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 638, .adv_w = 104, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 644, .adv_w = 75, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 650, .adv_w = 59, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 653, .adv_w = 75, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 659, .adv_w = 115, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 4}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 1, 2, 0, 3, 4, 5,
    2, 6, 7, 8, 9, 10, 9, 10,
    11, 12, 0, 13, 14, 15, 16, 17,
    18, 19, 12, 20, 20, 0, 0, 0,
    21, 22, 23, 24, 25, 22, 26, 27,
    28, 29, 29, 30, 31, 32, 29, 29,
    22, 33, 34, 35, 3, 36, 30, 37,
    37, 38, 39, 40, 41, 42, 43, 0,
    44, 0, 45, 46, 47, 48, 49, 50,
    51, 45, 52, 52, 53, 48, 45, 45,
    46, 46, 54, 55, 56, 57, 51, 58,
    58, 59, 58, 60, 41, 0, 0, 9
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 1, 2, 0, 3, 4, 5,
    2, 6, 7, 8, 9, 10, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 12,
    18, 19, 20, 21, 21, 0, 0, 0,
    22, 23, 24, 25, 23, 25, 25, 25,
    23, 25, 25, 26, 25, 25, 25, 25,
    23, 25, 23, 25, 3, 27, 28, 29,
    29, 30, 31, 32, 33, 34, 35, 0,
    36, 0, 37, 38, 39, 39, 39, 0,
    39, 38, 40, 41, 38, 38, 42, 42,
    39, 42, 39, 42, 43, 44, 45, 46,
    46, 47, 46, 48, 0, 0, 35, 9
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 2, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 9, 0, 5, -3, 0, 0, 0,
    0, -11, -12, 1, 8, 3, 3, -8,
    1, 8, 0, 7, 2, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 12, 2, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, 0, 0, 0, 0, 0, -4,
    3, 4, 0, 0, -2, 0, -1, 2,
    0, -2, 0, -2, -1, -4, 0, 0,
    0, 0, -2, 0, 0, -2, -3, 0,
    0, -2, 0, -4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, -2, 0,
    0, -5, 0, -24, 0, 0, -4, 0,
    4, 6, 0, 0, -4, 2, 2, 6,
    4, -3, 4, 0, 0, -10, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, -8, 0, -8, -1, 0, 0, 0,
    0, -1, 6, 0, -6, -2, -1, 1,
    0, -3, 0, 0, -2, -12, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -15, -2, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 0, 2, 0, 0, -4,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -7, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    2, 4, 2, 6, -2, 0, 0, 4,
    -2, -7, -25, 1, 5, 4, 0, -2,
    0, 5, 0, 5, 0, 5, 0, -18,
    0, -2, 6, 0, 6, -2, 4, 2,
    0, 0, 1, -2, 0, 0, -3, 15,
    0, 15, 0, 6, 0, 7, 3, 3,
    0, 0, 0, -7, 0, 0, 0, 0,
    1, -2, 0, 1, -4, -3, -4, 1,
    0, -2, 0, 0, 0, -8, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, -11, 0, -13, 0, 0, 0, 0,
    -2, 0, 17, -2, -3, 2, 2, 0,
    0, -3, 2, 0, 0, -11, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -18, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 12, 0, 0, -7, 0, 7, 0,
    -14, -18, -14, -4, 6, 0, 0, -12,
    0, 1, -5, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 6, -22, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 1,
    1, -2, -4, 0, -1, -1, -2, 0,
    0, -1, 0, 0, 0, -4, 0, -2,
    0, -5, -4, 0, -5, -7, -7, -3,
    0, -4, 0, -4, 0, 0, 0, 0,
    -2, 0, 0, 2, 0, 1, -2, 0,
    0, 0, 0, 2, -1, 0, 0, 0,
    -1, 2, 2, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 3, -1, 0, -2, 0, -3, 0,
    0, -1, 0, 6, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, -1, -1,
    0, -2, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, -1, 0,
    -2, -2, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, -2, -2, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, -2, -3, 0,
    0, -6, -1, -6, 4, 0, 0, -4,
    2, 4, 5, 0, -5, 0, -1, 0,
    0, -8, 2, -1, 2, -11, 2, 0,
    0, 1, -10, 0, -11, -2, -17, -1,
    0, -10, 0, 4, 5, 0, 3, 0,
    0, 0, 0, 1, 0, -4, -3, 0,
    0, 0, 0, -2, 0, 0, 0, -2,
    0, 0, 0, 0, 0, -1, -1, 0,
    -1, -2, 0, 0, 0, 0, 0, 0,
    0, -2, -2, 0, -2, -2, -1, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, -2, 0,
    0, -1, 0, -4, 2, 0, 0, -1,
    1, 2, 2, 0, 0, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 0, 1,
    0, 0, -2, 0, -2, -1, -1, 0,
    0, 0, 0, 0, 0, 0, 1, 0,
    -2, 0, 0, 0, 0, -3, -3, 0,
    0, 6, -1, 1, -5, 0, 0, 5,
    -10, -9, -7, -4, 2, 0, -1, -12,
    -3, 0, -3, 0, -4, 3, -3, -12,
    0, -5, 0, 0, 1, 0, 2, -1,
    0, 2, 2, -6, -7, 0, -10, -4,
    -4, -4, -6, -1, -5, 1, -2, -4,
    0, 0, 0, -2, 0, 0, 0, 1,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, -1,
    0, 0, -2, 0, -3, -4, -4, 0,
    0, -6, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, 0, 0, 0, -2, 0,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 10, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, -2, 0, 0, 0,
    0, -10, -6, 0, 0, 0, -3, -10,
    0, 0, -2, 2, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, 0, 0, 0, 1, 0,
    1, -4, -4, 0, -2, -2, -2, 0,
    0, 0, 0, 0, 0, -6, 0, -2,
    0, -3, -2, 0, -4, -5, -6, -1,
    0, -4, 2, -6, 0, 0, 0, 0,
    15, 0, 0, 1, 0, 0, -3, 0,
    0, -8, 0, 0, 0, 0, 0, -18,
    -4, 6, 6, -2, -8, 0, 2, -3,
    0, -10, -1, -2, 2, -13, -2, 4,
    0, 3, -7, -3, -7, -7, -8, 0,
    0, -12, 0, 10, 0, 0, -1, 0,
    0, 0, -1, -1, -2, -5, -7, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, -1, -2, -3, 0,
    0, -4, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -1, 0, -4, 0, 0, 4,
    -1, 2, 0, -4, 2, -1, 0, -3,
    -2, 0, -2, -2, -2, 0, -3, -3,
    0, 0, -1, -1, -1, -3, -1, 0,
    0, -2, 0, 2, -1, 0, -4, 0,
    0, 0, -4, 0, -3, 0, -3, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    -4, 2, 0, -2, 0, -1, -1, -3,
    -1, -1, -1, 0, -1, -1, 0, 0,
    0, 0, 0, 0, -2, -1, -1, 0,
    0, 0, 0, 1, -1, 0, -1, 0,
    0, 0, -1, -1, -1, -1, -1, -1,
    2, 8, 0, 0, -4, 0, -1, 4,
    0, -2, -7, -2, 3, 1, 0, -8,
    -3, 2, -3, 2, 0, 1, -1, -6,
    0, -3, 1, 0, 0, -3, 0, 0,
    0, 2, 2, -4, -3, 0, -3, -2,
    -3, -2, -2, 0, -3, 1, -3, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, -2, 0, 0, -2, -2, 0,
    0, 0, 0, -2, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, -1, 0,
    0, 0, -3, 0, -4, 0, 0, 0,
    -7, 0, 1, -4, 4, 1, -1, -8,
    0, 0, -4, -2, 0, -8, -5, -6,
    0, 0, -9, -2, -8, -8, -10, 0,
    -3, 0, 2, 12, -2, 0, -4, -2,
    0, -2, -3, -5, -3, -7, -9, -5,
    0, 0, -1, 0, 1, 0, 0, -13,
    0, 6, 3, -3, -7, 0, 1, -3,
    0, -10, -1, -2, 4, -17, -2, 1,
    0, 0, -12, -2, -9, -2, -14, 0,
    0, -13, 0, 10, 1, 0, -1, 0,
    0, 0, 0, -1, -1, -7, -1, 0,
    0, 0, 0, 0, -6, 0, -1, 0,
    -1, -5, -8, 0, 0, -1, -3, -6,
    -2, 0, -2, 0, 0, 0, 0, -9,
    -2, -6, -6, -2, -3, -5, -2, -3,
    0, -4, -1, -7, -3, 0, -2, -2,
    -2, -2, 0, 1, 0, -1, -6, 0,
    0, -3, 0, 0, 0, 0, 1, 0,
    1, -4, 10, 0, -2, -2, -2, 0,
    0, 0, 0, 0, 0, -6, 0, -2,
    0, -3, -2, 0, -4, -5, -6, -1,
    0, -4, 2, 8, 0, 0, 0, 0,
    15, 0, 0, 1, 0, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, -1, -4,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, -2, -2, 0, 0, -4, -2, 0,
    0, -4, 0, 3, -1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0,
    4, 2, -1, 0, -5, -2, 0, 6,
    -7, -6, -4, -4, 8, 4, 2, -16,
    -2, 4, -2, 0, -2, 4, -2, -7,
    0, -2, 2, -2, -2, -6, -2, 0,
    0, 6, 4, 0, -5, 0, -11, -2,
    8, -2, -7, 1, -2, -6, -6, -2,
    2, 0, -3, 0, -5, 0, 2, 6,
    -5, -7, -8, -5, 6, 0, 1, -15,
    -2, 2, -3, -2, -5, 0, -4, -7,
    -3, -3, -2, 0, 0, -5, -5, -2,
    0, 6, 5, -2, -11, 0, -11, -1,
    0, -7, -12, -1, -6, -3, -6, -6,
    0, 0, -2, 0, -4, -1, 0, -2,
    -4, 0, 3, -7, 2, 0, 0, -11,
    0, -2, -4, -3, -2, -6, -5, -7,
    -4, 0, -6, -2, -5, -2, -6, -2,
    0, 0, 1, 8, -3, 0, -6, -2,
    0, -2, -4, -5, -5, -6, -8, -3,
    4, 0, -3, 0, -10, -1, 2, 4,
    -6, -7, -4, -7, 7, -2, 1, -18,
    -4, 4, -4, -3, -7, 0, -6, -8,
    -1, -2, -2, -2, -5, -6, -1, 0,
    0, 6, 6, -1, -12, 0, -12, -2,
    7, -7, -14, -4, -7, -8, -10, -7,
    0, 0, 0, 0, -1, 0, 0, 2,
    -1, 4, 1, -3, 4, 0, 0, -3,
    0, 0, 0, 0, 1, 1, -1, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 2, 6, 1, 0, -1, 0,
    0, 0, 0, -1, -1, -1, 0, 0,
    1, 2, 0, 0, 0, 0, 2, 0,
    -1, 0, 8, 0, 4, 1, 1, -3,
    0, 4, 0, 0, 0, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 6, 0, 6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, -2, 3, 0, 6, 0,
    0, 17, 1, -4, -4, 2, 2, -1,
    1, -10, 0, 0, 10, -12, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -14, 8, 27, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, -2, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, 0, -5, 0, 0, 1, 0,
    0, 2, 23, -4, -1, 5, 5, -5,
    2, 0, 0, 2, 2, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -25, 6, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, 0, 0, 0, -5,
    0, 0, 0, 0, -4, -1, 0, 0,
    0, -4, 0, -1, 0, -8, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, -3, 0, -5, 0, 0, 0, -3,
    2, -1, 0, 0, -5, -2, -4, 0,
    0, -5, 0, -2, 0, -8, 0, -4,
    0, 0, -13, -2, -8, -4, -8, 0,
    0, -12, 0, -5, -2, 0, 0, 0,
    0, 0, 0, 0, 0, -3, -3, -1,
    0, 0, 0, 0, -3, 0, -4, 3,
    -4, 4, 0, -1, -4, -1, -3, -2,
    0, -1, -1, -1, 2, -5, -1, 0,
    0, 0, -14, -2, -5, 0, -7, 0,
    -1, -8, -1, 0, 0, -1, -1, 0,
    0, 0, 0, 2, 0, -2, -3, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0,
    0, -3, 0, -1, 0, 0, 0, -4,
    2, 0, 0, 0, -5, -2, -4, 0,
    0, -5, 0, -2, 0, -8, 0, 0,
    0, 0, -18, 0, -4, -7, -10, 0,
    0, -12, 0, -2, -3, 0, 0, 0,
    0, 0, 0, 0, 0, -2, -3, -1,
    1, 0, 0, 3, -3, 0, 8, 8,
    -2, -2, -6, 1, 8, 3, 3, -5,
    1, 7, 1, 5, 3, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 12, 8, -4, -2, 0, -1, 15,
    9, 15, 0, 0, 0, 2, 0, 0,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 5, 0, 0,
    0, 0, -12, -1, -2, -6, -8, 0,
    0, -12, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 5, 0, 0,
    0, 0, -12, -1, -2, -6, -8, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -2, 0, 0, 0,
    -5, 2, 0, -2, 2, 4, 2, -6,
    0, 1, -2, 2, 0, 2, 0, 0,
    0, 0, -2, 0, -1, -1, -4, 0,
    -1, -8, 0, 11, -2, 0, -4, -1,
    0, -1, -3, 0, -2, -6, -4, -2,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 5, 0, 0,
    0, 0, -12, -1, -2, -6, -8, 0,
    0, -12, 0, 0, 0, 0, 0, 0,
    10, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, -5, -1, -1, 6,
    -2, -2, -8, 1, 1, 1, -1, -4,
    1, 4, 1, 2, 1, 2, -4, -9,
    -1, 0, -4, -2, -5, -7, -7, 0,
    -2, -4, -1, -3, -1, -1, -2, -1,
    0, -1, 0, 3, 0, 3, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, -2, -2, 0,
    0, -5, 0, -1, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, -2, 0,
    0, 0, 0, 0, -1, 0, 0, -3,
    -2, 2, 0, -3, -3, -1, 0, -4,
    -1, -3, -2, -1, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -12, 0, 5, 0, 0, -3, 0,
    0, 0, 0, -2, 0, -2, 0, 0,
    0, 0, -1, 0, -5, 0, 0, 7,
    -3, -6, -7, 1, 4, 4, 1, -6,
    1, 3, 1, 6, 1, 7, -1, -5,
    0, 0, -3, 0, 0, -6, -5, 0,
    0, -4, 0, -3, -3, 0, -3, 0,
    -3, 0, -2, 3, 0, -2, -6, -2,
    0, 0, -1, 0, -4, 0, 0, 3,
    -5, 0, 2, -2, 2, 2, 0, -7,
    0, -1, -1, 0, -2, 4, -2, 0,
    0, 0, -4, -1, -4, 0, -6, 0,
    0, -8, 0, 7, -2, 0, -3, 0,
    5, 0, -2, 0, -2, -6, 0, -2,
    0, 0, 0, 0, -1, 0, 0, 2,
    -2, 1, 0, 0, -1, -1, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -11, 0, 3, 0, 0, -1, 0,
    0, 0, 0, 1, 0, -2, -2, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 60,
    .right_class_cnt     = 48,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_Montserrat_12_px = {
#else
lv_font_t ui_font_Montserrat_12_px = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_MONTSERRAT_12_PX*/

