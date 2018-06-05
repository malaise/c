#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mapcoder.h"

static void usage (void) {
  fprintf (stderr, "Usage: tmc <encode> | <decode>\n");
  fprintf (stderr, "  <encode> ::= -c <lat> <lon> <context> [ <precision> ]\n");
  fprintf (stderr, "  <decode> ::= -d [ <context> ] <mapcode>\n");
}

int main (int argc, char *argv[]) {

  Mapcodes codes;
  double lat, lon;
  int ctx;
  int i, res;
  int prec;
  char* pmap;

  if (argc < 3) {
    fprintf (stderr, "ERROR: Invalid arguments\n");
    usage();
    exit (1);
  }

  if (strcmp (argv[1], "-c") == 0 ) {
    if ( (argc < 5) || (argc > 6) ) {
      fprintf (stderr, "ERROR: Invalid arguments\n");
      usage();
      exit (1);
    }
    /* Encode */
    if (sscanf (argv[2], "%lf", &lat) != 1) {
      fprintf (stderr, "ERROR: Invalid lat %s\n", argv[2]);
      exit (1);
    }
    if (sscanf (argv[3], "%lf", &lon) != 1) {
      fprintf (stderr, "ERROR: Invalid lon %s\n", argv[3]);
      exit (1);
    }
    /* Mandatory context, may be empty */ 
    if (strlen (argv[4]) == 0) {
      ctx = 0;
    } else {
      ctx = getTerritoryCode(argv[4], 0);
      if (ctx < 0) {
        fprintf (stderr, "ERROR: Invalid context %s\n", argv[4]);
        exit (1);
      }
    }
    /* Optional precision */
    if (argc == 5) {
      prec = 0;
    } else {
      if ( (sscanf (argv[5], "%d", &prec) != 1)
          || (prec < 0) || (prec > 2) ) {
        fprintf (stderr, "ERROR: Invalid precision %s\n", argv[5]);
        exit (1);
      }
    }

    /* Do encode */
    res = encodeLatLonToMapcodes (&codes, lat, lon, ctx, prec);
    if (res == 0) {
      fprintf (stderr, "ERROR: Cannot encode %s %s in context %s with precision %1d\n",
               argv[2], argv[3], argv[4], prec);
      exit (1);
    }

    for (i = 0; i < codes.count; i++) {
      printf ("%s\n", codes.mapcode[i]);
    }

  } else if (strcmp (argv[1], "-d") == 0 ) {
    /* Decode */
    /* Optional leading context */
    if (argc == 4)  {
      pmap = argv[3];
      ctx = getTerritoryCode(argv[2], 0);
      if (ctx < 0) {
        fprintf (stderr, "ERROR: Invalid context %s\n", argv[2]);
        exit (1);
      }
    } else if (argc == 3) {
      pmap = argv[2];
      ctx = 0;
    } else {
      fprintf (stderr, "ERROR: Invalid arguments\n");
      usage();
      exit (1);
    }

    /* Do decode */
    res = decodeMapcodeToLatLon (&lat, &lon, pmap, ctx);
    if (res != 0) {
      fprintf (stderr, "ERROR: Cannot decode %s in context%d\n", pmap, ctx);
      exit (1);
    }
    printf ("%3.9lf %3.9lf\n", lat, lon);

  } else {
    fprintf (stderr, "ERROR: Invalid arguments\n");
    usage();
    exit (1);
  }

  exit (0);
}

