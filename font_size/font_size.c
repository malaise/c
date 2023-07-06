#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

/* Display the height, width and vertical offset of several fonts */

char *prog_name;

static void usage (void) {
  printf ("Usage : %s <display> { <font> }\n", prog_name);
}

static int result;
static void error (const char *msg, int fatal) {
  fprintf (stderr, "ERROR: %s.\n", msg);
  result = 1;
  if (fatal) {
    usage();
    exit (result);
  }
}

int main (int argc, char *argv[]) {
  /* Error message */
  char msg [1024];
  /* X11 display */
  Display *disp;
  /* Index of font argument */
  int i;
  /* Font stuff */
  XFontSet font_set;
  int missing_count;
  char **font_names;
  XFontStruct **fonts, *font;
  int res;

  /* Check syntax: 2 or more arguments: <diplay> and <font>... */
  prog_name = argv[0];
  if (argc < 3) {
    error ("Invalid argument", True);
  }

  /* Open display */
  disp = XOpenDisplay (argv[1]);
  if (disp == NULL) {
    sprintf (msg, "Can't open display %s", argv[1]);
    error (msg, True);
  }

  /* Loop on all font names */
  result = 0;
  for (i = 2; i < argc; i++) {

    /* Create font set */
    font_set = XCreateFontSet (disp, argv[i], &font_names, &missing_count, NULL);
    if (font_set == NULL) {
      sprintf (msg, "Can't create font set of %s", argv[i]);
      error (msg, False);
      continue;
    }
    if (missing_count != 0)  {
      int j;
      for (j = 0; j < missing_count; j++) {
         printf ("Missing font %s for %s.\n", font_names[j], argv[i]);
      }
    }

    /* Load font characteristics */
    res = XFontsOfFontSet (font_set, &fonts, &font_names);
    if (res == -1) {
      sprintf (msg, "Cannot get fonts from set for %s", argv[i]);
      error (msg, False);
      XFreeFontSet (disp, font_set);
      continue;
    }

    /* Load font */
    font = XLoadQueryFont (disp, argv[i]);
    if (font == NULL) {
      sprintf  (msg, "Cannot query font %s", argv[i]);
      error (msg, False);
      XFreeFontSet (disp, font_set);
    }

    printf ("Font %s is %dx%d-%d\n", argv[i],
        (int)font->max_bounds.width,
        (int)(font->ascent + font->descent),
        (int)font->ascent);

    /* Done for this font */
    XFreeFontSet (disp, font_set);
    XFreeFont (disp, font);
  }

  /* Done */
  (void) XCloseDisplay (disp);
  exit (result);

}

