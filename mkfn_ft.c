#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include "mkfn.h"

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define LIMIT(n, a, b)	((n) < (a) ? (a) : ((n) > (b) ? (b) : (n)))

static FT_Library library;
static FT_Face fn[NFONTS];
static int fn_desc[NFONTS];
static int fn_bold[NFONTS];
static int fn_hint[NFONTS];
static int fn_sc[NFONTS];
static int fn_sr[NFONTS];
static int fn_cnt;

static int glyphload(FT_Face face, int c, int autohint)
{
	int flags = FT_LOAD_RENDER;
	if (autohint)
		flags |= FT_LOAD_FORCE_AUTOHINT;
	return !FT_Get_Char_Index(face, c) ||
			FT_Load_Char(face, c, flags);
}

static int facedescender(FT_Face face)
{
	char *s = "gy_pj/\\|Q";
	int ret = 0;
	for (; *s; s++)
		if (!glyphload(face, *s, 0))
			ret = MAX(ret, face->glyph->bitmap.rows -
					face->glyph->bitmap_top);
	return ret;
}

int mkfn_font(char *path, char *spec)
{
	FT_Face *face = &fn[fn_cnt];
	int hdpi = 196;
	int vdpi = 196;
	float size = 10;
	if (FT_New_Face(library, path, 0, face))
		return 1;
	while (spec && *spec) {
		if (spec[0] == 'a')
			fn_hint[fn_cnt] = atoi(spec + 1);
		if (spec[0] == 'b')
			fn_bold[fn_cnt] = atoi(spec + 1);
		if (spec[0] == 'v')
			vdpi = atoi(spec + 1);
		if (spec[0] == 'c')
			fn_sc[fn_cnt] = atof(spec + 1);
		if (spec[0] == 'r')
			fn_sr[fn_cnt] = atof(spec + 1);
		if (spec[0] == 'h')
			hdpi = atoi(spec + 1);
		if (spec[0] == 's')
			size = atof(spec + 1);
		if (isdigit((unsigned char) spec[0]))
			size = atof(spec);
		spec++;
		while (*spec && strchr("0123456789.-+", (unsigned char) *spec))
			spec++;
	}
	if (FT_Set_Char_Size(*face, 0, size * 64, hdpi, vdpi))
		return 1;
	fn_desc[fn_cnt] = facedescender(*face);
	fn_cnt++;
	return 0;
}

static void fn_embolden(unsigned char *bits, int rows, int cols)
{
	int i, j, k;
	int n = 2;
	for (i = 0; i < rows; i++) {
		for (j = cols - n; j >= 0; j--) {
			int idx = i * cols + j;
			int val = 0;
			for (k = 0; k < n; k++)
				if (bits[idx + k] > val)
					val = bits[idx + k];
			bits[idx + n - 1] = val;
		}
	}
}

int mkfn_bitmap(char *dst, int c, int rows, int cols)
{
	unsigned char *bits = (void *) dst;
	int sr, sc;	/* the starting and the number of rows in bits */
	int nr, nc;	/* the starting and the number of cols in bits */
	int br;		/* the starting column of glyph bitmap */
	int bc;		/* the starting row of glyph bitmap */
	int bw;		/* the number of columns in glyph bitmap */
	int i, j;
	int bold;
	unsigned char *src;
	FT_Face face = NULL;
	for (i = 0; i < fn_cnt; i++) {
		if (!glyphload(fn[i], c & ~DWCHAR, fn_hint[i])) {
			face = fn[i];
			bold = fn_bold[i];
			break;
		}
	}
	if (!face)
		return 1;
	if (!dst)
		return 0;
	bc = c & DWCHAR ? cols : 0;
	nc = LIMIT(face->glyph->bitmap.width - bc, 0, cols);
	sc = LIMIT(face->glyph->bitmap_left - bc + fn_sc[i], 0, cols - nc);
	sr = rows + fn_sr[i] - fn_desc[i] - face->glyph->bitmap_top;
	br = MAX(0, -sr);
	sr = LIMIT(sr, 0, rows);
	nr = MIN(rows - sr, face->glyph->bitmap.rows - br);
	memset(bits, 0, rows * cols);
	src = face->glyph->bitmap.buffer;
	bw = face->glyph->bitmap.pitch;
	if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
		for (i = 0; i < nr; i++) {
			for (j = 0; j < nc; j++) {
				int num = (br + i) * bw * 8 + bc + j;
				int val = src[num / 8] & (1 << (7 - num % 8));
				bits[(i + sr) * cols + j + sc] = val ? 255 : 0;
			}
		}
		return 0;
	}
	for (i = 0; i < nr; i++)
		memcpy(&bits[(sr + i) * cols + sc], src + (br + i) * bw + bc, nc);
	if (bold)
		fn_embolden(bits, rows, cols);
	return 0;
}

void mkfn_dim(int *r, int *c)
{
	*r = fn[0]->size->metrics.height >> 6;
	*c = fn[0]->size->metrics.max_advance >> 6;
}

void mkfn_init(void)
{
	FT_Init_FreeType(&library);
}

void mkfn_free(void)
{
	int i;
	for (i = 0; i < fn_cnt; i++)
		FT_Done_Face(fn[i]);
	FT_Done_FreeType(library);
}
