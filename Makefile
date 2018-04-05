# fbpad_mkfn makefile
CC = cc

CFLAGS = -O2 -Wall
LDFLAGS =

all: mkfn_ft mkfn_stb
%.o: %.c
	$(CC) -c $(CFLAGS) $<
mkfn_ft: mkfn_ft.c mkfn.o isdw.o
	$(CC) -c $(CFLAGS) `pkg-config --cflags freetype2` mkfn_ft.c
	$(CC) -o $@ mkfn_ft.o mkfn.o isdw.o $(LDFLAGS) `pkg-config --libs freetype2`

mkfn_stb: mkfn_stb.c mkfn.o isdw.o
	$(CC) -c $(CFLAGS) -DSTB_TRUETYPE_IMPLEMENTATION mkfn_stb.c
	$(CC) -o $@ mkfn_stb.o mkfn.o isdw.o $(LDFLAGS) -lm
clean:
	rm -f *.o mkfn_ft mkfn_stb
