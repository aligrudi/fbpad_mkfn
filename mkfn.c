/*
 * fbpad_mkfn - generate fbpad fonts from truetype fonts
 *
 * Copyright (C) 2009-2022 Ali Gholami Rudi <ali at rudi dot ir>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "chars.h"
#include "mkfn.h"

#define NGLYPHS		(1 << 14)

static int rows, cols;

static int fn_glyphs(int *glyphs)
{
	int i, j;
	int n = 0;
	for (i = 0; i < sizeof(chars) / sizeof(chars[0]); i++) {
		for (j = chars[i][0]; j <= chars[i][1] && n < NGLYPHS; j++) {
			if (!mkfn_bitmap(NULL, j, rows, cols))
				glyphs[n++] = j;
			if (!mkfn_bitmap(NULL, j, rows, cols) && isdw(j))
				glyphs[n++] = DWCHAR | j;
		}
	}
	return n;
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
	int glyphs[NGLYPHS];
	char *buf = malloc(rows * cols);
	int i;
	memcpy(head.sig, sig, strlen(sig));
	head.ver = 0;
	head.rows = rows;
	head.cols = cols;
	head.n = fn_glyphs(glyphs);
	qsort(glyphs, head.n, sizeof(glyphs[0]), (void *) intcmp);
	write(fd, &head, sizeof(head));
	write(fd, glyphs, sizeof(*glyphs) * head.n);
	for (i = 0; i < head.n; i++) {
		mkfn_bitmap(buf, glyphs[i], rows, cols);
		write(fd, buf, rows * cols);
	}
	fprintf(stderr, "tinyfont[%5d]: height=%2d width=%2d\n",
		head.n, rows, cols);
	free(buf);
}

static char *usage =
	"usage: mkfn [options] font1.ttf:size ... >font.tf\n"
	"\noptions:\n"
	"  -h n  \t\t set glyph height\n"
	"  -w n  \t\t set glyph width\n";

int main(int argc, char *argv[])
{
	int i;
	char *wdiff = NULL;
	char *hdiff = NULL;
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		if (argv[i][1] == 'w') {
			wdiff = argv[i][2] ? argv[i] + 2 : argv[++i];
		} else if (argv[i][1] == 'h') {
			hdiff = argv[i][2] ? argv[i] + 2 : argv[++i];
		} else {
			i = argc;
		}
	}
	if (i == argc) {
		fprintf(stderr, usage);
		return 0;
	}
	mkfn_init();
	for (; i < argc; i++) {
		char *name = argv[i];
		char *spec = NULL;
		if (strchr(name, ':')) {
			spec = strrchr(name, ':') + 1;
			strrchr(name, ':')[0] = '\0';
		}
		if (mkfn_font(name, spec)) {
			fprintf(stderr, "mkfn: failed to load <%s>\n", name);
			return 1;
		}
	}
	mkfn_dim(&rows, &cols);
	if (hdiff)
		rows = strchr("-+", hdiff[0]) ? rows + atoi(hdiff) : atoi(hdiff);
	if (wdiff)
		cols = strchr("-+", wdiff[0]) ? cols + atoi(wdiff) : atoi(wdiff);
	output(1);
	mkfn_free();
	return 0;
}
