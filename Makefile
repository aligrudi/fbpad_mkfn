CC = cc
CFLAGS = -O2 -DSTB_TRUETYPE_IMPLEMENTATION
LDFLAGS = -lm

all: mkfn
%.o: %.c
	$(CC) -c $(CFLAGS) $<
mkfn: mkfn.o
	$(CC) $(LDFLAGS) -o $@ $^
clean:
	rm -f *.o mkfn
