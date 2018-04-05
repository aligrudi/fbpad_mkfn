FBPAD_MKFN
==========

This program generates fbpad font files.  It can use either
stb_truetype.h (without external dependencies) or Freetype library.
For the former build mkfn_stb and for the latter build mkfn_ft.

To control which glyphs to include in the generated font, especially
to reduce the size of the files, edit chars.h.  The gen.sh script
shows how to use mkfn.

The height and width of the resulting font can be specified with the
-h and -w options.  Mkfn expects font paths as its final arguments;
these fonts are searched in the same order for the glyphs to put in
the resulting fbpad font.  The size and horizontal/vertical DPI of
each font may be specified after each font path as (without the
brackets):

  font.ttf:[SZ][hHDPI][vVDPI][rVADJ][cHADJ][bBOLD][aHINT]

in which SZ is the font size, HDPI/VDPI is the horizontal/vertical
DPI, HADJ/VADJ is the amount of horizontal/vertical position
adjustment, BOLD specifies if fonts should be emboldened by repeating
pixels, and HINT specifies if autohinting should be used.
