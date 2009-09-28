#ifndef UTILITIES_H
#define UTILITIES_H

#include "boolean.h"

/**********************************/
/*** Prototypes for utilities.c ***/
/**********************************/

/* Defines array size to store URL */
#define PATH_MAX_LEN 1024
#define URL_MAX_LEN (10+256+PATH_MAX_LEN)
typedef char url_t[URL_MAX_LEN+1];

/* Defines array size to store date of type '2009-07-20T14:30:17\0' */
#define DATE_LENGTH 19
typedef char date_t[DATE_LENGTH+1];

/* Modes depend from user input arguments */
typedef enum {URL_MODE, CUR_MODE, ALL_MODE} mode_kind;

/* Enums for defining path and action type */
typedef enum {INVALID=0, TRUNK, BRANCH, TAG} label_kind;

/* Error message in case of fatal error */
extern void error_data (const char *msg, const char *data);

/* In any kind of fatal error */
extern void error (const char *msg);

/* Put command line help */
extern void put_help (const char *prog);

/* Debug */
extern void debug1 (int severity, const char *msg);
extern void debug2 (int severity, const char *msg, const char *data);
extern void debug3 (int severity, const char *msg, const char *data,
                                  const char *extra);

/* Modifying SVN output date to readable format */
extern void svn_date_to_normal (char *date);

/* Check that the action is not against a file.
   Function checks number of '/' in key path.
   Return kind of label */
extern label_kind compare_path (const char *path);

/* Skip info after label (keep name) */
extern void get_node_path (char *string);

/* Check URL: return addr of label (trunk/tags/branches) */
extern char * url_check (const char *url, boolean allow_trunk);

/* Check if the argument is URL */
extern boolean is_url (const char *url);

#endif

