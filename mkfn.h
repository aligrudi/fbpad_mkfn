/* mkfn main header */

#define NFONTS		16		/* number of fonts */
#define DWCHAR		0x40000000u	/* second half of a fullwidth char */

int isdw(int c);
void mkfn_init(void);
void mkfn_free(void);
int mkfn_font(char *font, char *spec);
int mkfn_bitmap(char *dst, int c, int rows, int cols);
void mkfn_dim(int *r, int *c);
