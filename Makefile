# fbpad_mkfn makefile
CC = cc

# For mkfn with stb_truetype.h
CFLAGS = -O2 -Wall -DSTB_TRUETYPE_IMPLEMENTATION
LDFLAGS = -lm
MKFN = mkfn_stb.o

# For mkfn with freetype library
#CFLAGS = -Wall -O2 `pkg-config --cflags freetype2`
#LDFLAGS = `pkg-config --libs freetype2`
#MKFN = mkfn_ft.o

all: mkfn
%.o: %.c
	$(CC) -c $(CFLAGS) $<
mkfn: mkfn.o $(MKFN) isdw.o
	$(CC) $(LDFLAGS) -o $@ $^
clean:
	rm -f *.o mkfn
