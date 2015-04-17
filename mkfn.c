/*
 * fbpad_mkfn - generate fbpad fonts from truetype fonts
 *
 * Copyright (C) 2009-2015 Ali Gholami Rudi <ali at rudi dot ir>
 *
 * This program is released under the Modified BSD license.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "stb_truetype.h"
#include "chars.h"

#define NFONTS		16		/* number of fonts */
#define MAXDOTS		1024
#define MAXGLYPHS	(1 << 16)

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define LIMIT(n, a, b)	((n) < (a) ? (a) : ((n) > (b) ? (b) : (n)))

static int hdpi = 100, vdpi = 100;	/* horizontal and vertical DPI */
static int embolden = 1;

static stbtt_fontinfo fn[NFONTS];
static float fn_vscale[NFONTS];
static float fn_hscale[NFONTS];
static int fn_base[NFONTS];
static char *fn_buf[NFONTS];
static int fn_cnt;
static int rows, cols;

static void fn_dim(int i, int *r, int *c)
{
	int x0, y0, x1, y1;
	stbtt_GetFontBoundingBox(&fn[i], &x0, &y0, &x1, &y1);
	*r = (y1 - y0) * fn_vscale[i];
	*c = (x1 - x0) * fn_hscale[i];
}

static void fn_add(char *path, float size)
{
	int n = fn_cnt++;
	int ascent, descent, linegap;
	int fd;
	fn_buf[n] = malloc(1 << 20);
	fd = open(path, O_RDONLY);
	read(fd, fn_buf[n], 1 << 20);
	close(fd);
	if (!stbtt_InitFont(&fn[n], fn_buf[n], stbtt_GetFontOffsetForIndex(fn_buf[n], 0))) {
		fprintf(stderr, "ft2tf: failed to load font <%s>\n", path);
		exit(1);
	}
	fn_hscale[n] = stbtt_ScaleForPixelHeight(&fn[n], size) * hdpi / 100;
	fn_vscale[n] = stbtt_ScaleForPixelHeight(&fn[n], size) * vdpi / 100;
	stbtt_GetFontVMetrics(&fn[n], &ascent, &descent, &linegap);
	fn_base[n] = ascent * fn_vscale[n];
}

static unsigned char *fn_bitmap(int c)
{
	static unsigned char src[MAXDOTS];
	int sr, sc;	/* the starting row and column of the bitmap */
	int nr, nc;	/* the number of rows and columns to render */
	int i;
	int g;
	int x0, y0, x1, y1;
	for (i = 0; i < fn_cnt; i++)
		if ((g = stbtt_FindGlyphIndex(&fn[i], c)) > 0)
			break;
	if (i == fn_cnt)
		return NULL;
	stbtt_GetGlyphBitmapBox(&fn[i], g, fn_hscale[i], fn_vscale[i], &x0, &y0, &x1, &y1);
	sr = LIMIT(fn_base[i] + y0, 0, rows);
	sc = LIMIT(x0, 0, cols);
	nr = rows - sr;
	nc = cols - sc;
	memset(src, 0, sizeof(src));
	stbtt_MakeGlyphBitmap(&fn[i], src + sr * cols + sc, nc, nr, cols,
				fn_hscale[i], fn_vscale[i], g);

	return src;
}

static int fn_glyphs(int *glyphs)
{
	int i, j;
	int n = 0;
	for (i = 0; i < sizeof(chars) / sizeof(chars[0]); i++)
		for (j = chars[i][0]; j <= chars[i][1] && n < MAXGLYPHS; j++)
			if (fn_bitmap(j))
				glyphs[n++] = j;
	return n;
}

static unsigned char *bitmap(int c)
{
	unsigned char *bits = fn_bitmap(c);
	int i, j, k;
	if (embolden > 1) {
		for (i = 0; i < rows; i++) {
			for (j = cols - embolden; j >= 0; j--) {
				int idx = i * cols + j;
				int val = 0;
				for (k = 0; k < embolden; k++)
					if (bits[idx + k] > val)
						val = bits[idx + k];
				bits[idx + embolden - 1] = val;
			}
		}
	}
	return bits;
}

static int intcmp(void *v1, void *v2)
{
	return *(int *) v1 - *(int *) v2;
}

/*
 * This tinyfont header is followed by:
 *
 * glyphs[n]	unicode character codes (int)
 * bitmaps[n]	character bitmaps (char[rows * cols])
 */
struct tinyfont {
	char sig[8];	/* tinyfont signature; "tinyfont" */
	int ver;	/* version; 0 */
	int n;		/* number of glyphs */
	int rows, cols;	/* glyph dimensions */
};

/* generate the output tinyfont font */
static void output(int fd)
{
	char *sig = "tinyfont";
	struct tinyfont head;
	int glyphs[MAXGLYPHS];
	int i;
	memcpy(head.sig, sig, strlen(sig));
	head.ver = 0;
	head.rows = rows;
	head.cols = cols;
	head.n = fn_glyphs(glyphs);
	qsort(glyphs, head.n, sizeof(glyphs[0]), (void *) intcmp);
	write(fd, &head, sizeof(head));
	write(fd, glyphs, sizeof(*glyphs) * head.n);
	for (i = 0; i < head.n; i++)
		write(fd, bitmap(glyphs[i]), rows * cols);
	fprintf(stderr, "tinyfont[%5d]: height=%2d width=%2d\n",
		head.n, rows, cols);
}

static void setdpi(char *s)
{
	hdpi = atoi(s);
	vdpi = strchr(s, ':') ? atoi(strchr(s, ':') + 1) : hdpi;
}

static char *usage =
	"usage: mkfn [options] font1.ttf:size ... >font.tf\n"
	"\noptions:\n"
	"  -h n  \t\t set glyph height\n"
	"  -w n  \t\t set glyph width\n"
	"  -b    \t\t embolden\n"
	"  -d v:h\t\t set vertical and horizontal DPI\n"
	"  -v    \t\t show version\n";

int main(int argc, char *argv[])
{
	int i;
	char *wdiff = NULL;
	char *hdiff = NULL;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		else if (argv[i][1] == 'v')
			printf("fbpad_mkfn\n");
		else if (argv[i][1] == 'b')
			embolden = argv[i][2] ? atoi(argv[i] + 2) : 2;
		else if (argv[i][1] == 'd')
			setdpi(argv[i][2] ? argv[i] + 2 : argv[++i]);
		else if (argv[i][1] == 'w')
			wdiff = argv[i][2] ? argv[i] + 2 : argv[++i];
		else if (argv[i][1] == 'h')
			hdiff = argv[i][2] ? argv[i] + 2 : argv[++i];
		else
			fprintf(stderr, usage);
	}
	if (i >= argc) {
		printf(usage);
		return 0;
	}
	for (; i < argc; i++) {
		char *name = argv[i];
		float size = 10;
		if (strchr(name, ':')) {
			size = atof(strrchr(name, ':') + 1);
			strrchr(name, ':')[0] = '\0';
		}
		fn_add(name, size);
	}
	fn_dim(0, &rows, &cols);
	if (hdiff)
		rows = strchr("-+", hdiff[0]) ? rows + atoi(hdiff) : atoi(hdiff);
	if (wdiff)
		cols = strchr("-+", wdiff[0]) ? cols + atoi(wdiff) : atoi(wdiff);
	output(1);
	return 0;
}
