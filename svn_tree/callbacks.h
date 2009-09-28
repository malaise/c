#ifndef CALLBACKS_H
#define CALLBACKS_H

/**********************************************/
/****** Function prototypes for svn_tree *****/
/********************************************/

#include "svn_includes.h"

#include "utilities.h"

/* Max size of path */
#define PATH_LENGTH 256

/* The specific error to abort iterations */
#define ABORT_ITERATIONS SVN_ERR_CL_NO_EXTERNAL_MERGE_TOOL

/* Structure to hold node values of source and destination revisions */
typedef struct {
  url_t src_path;
  url_t dst_path;
  long int src_rev;
  long int dst_rev;
  date_t src_date;
  date_t dst_date;
} node_type;

/* Structure to hold info with action type + two path values */
typedef enum {DELETE=-2, MOVE=-1, OTHER=0, COPY=1} action_kind;
typedef struct {
   action_kind action;
   url_t src_path;
   url_t dst_path;
   label_kind src_kind;
   label_kind dst_kind;
} source_dest_values;

typedef struct {
  url_t repository_url;
  url_t current_url;
} urls_type;

typedef char tag_path_type[PATH_LENGTH+1];

/* Callbacks for promt of authentication */
extern svn_error_t * my_simple_prompt_callback(
           svn_auth_cred_simple_t **cred,
           void *baton,
           const char *realm,
           const char *username,
           svn_boolean_t may_save,
           apr_pool_t *pool);
extern svn_error_t * my_username_prompt_callback(
           svn_auth_cred_username_t **cred,
           void *baton,
           const char *realm,
           svn_boolean_t may_save,
           apr_pool_t *pool);

/* Callback to get current URL */
extern svn_error_t * client_info_cb(
           void *baton,
           const char *target,
           const svn_info_t *info,
           apr_pool_t *pool);

/* Callback to retrieve creation revision number + date */
extern svn_error_t * log_entry_cb(
            void * baton,
            svn_log_entry_t *log_entry,
            apr_pool_t *pool);
/* Getting the action (COPY = 1; MOVE = -1 or OTHER = 0) in "log_entry_cb" */
extern void get_action (apr_hash_t *hash_table, apr_pool_t *pool,
                                 source_dest_values *source_dest);

/* Store short revision list; -b option */
extern svn_error_t * log_srevlist_cb(
            void * baton,
            svn_log_entry_t *log_entry,
            apr_pool_t *pool);

/* Store long  revision list */
extern svn_error_t * log_frevlist_cb(
               void * baton,
               svn_log_entry_t *log_entry,
               apr_pool_t *pool);

/* Get list of tag names */
extern svn_error_t *
  print_dirent_cb(void *baton,
                  const char *path,
                  const svn_dirent_t *dirent,
                  const svn_lock_t *lock,
                  const char *abs_path,
                  apr_pool_t *pool);

#endif

