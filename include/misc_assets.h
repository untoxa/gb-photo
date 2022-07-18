#ifndef __MISC_ASSETS_H_INCLUDE__
#define __MISC_ASSETS_H_INCLUDE__

#define SOLID_WHITE 0xfeu
#define SOLID_BLACK 0xffu

#define CORNER_UL 0xfau
#define CORNER_UR 0xfbu
#define CORNER_DR 0xfcu
#define CORNER_DL 0xfdu

#define ICON_CBX            "\x18"
#define ICON_CBX_CHECKED    "\x19"
#define ICON_A              "\x1a"
#define ICON_B              "\x1b"
#define ICON_SELECT         "\x1c\x1d"
#define ICON_START          "\x1e\x1f"

void misc_assets_init() BANKED;

#endif
