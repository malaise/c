  /**********************************/
 /***** FUNCTIONS FOR svn_tree *****/
/**********************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "dynlist.h"

#include "callbacks.h"

/* ************************/
/* Callback for prompting */
/* ************************/

/* Display a prompt and read a one-line response into the provided buffer,
   removing a trailing newline if present. */
static svn_error_t * prompt_and_get (
           const char *prompt,
           char *buffer,
           size_t max) {
  int len;
  printf ("%s: ", prompt);
  if (fgets(buffer, max, stdin) == NULL) {
    return svn_error_create(0, NULL, "error reading stdin");
  }
  len = strlen(buffer);
  if (len > 0 && buffer[len-1] == '\n') {
    buffer[len-1] = 0;
  }
  return SVN_NO_ERROR;
}

extern svn_error_t * my_simple_prompt_callback (
           svn_auth_cred_simple_t **cred,
           void *baton __attribute__ ((unused)),
           const char *realm,
           const char *username,
           svn_boolean_t may_save __attribute__ ((unused)),
           apr_pool_t *pool) {
  svn_auth_cred_simple_t *ret = apr_pcalloc (pool, sizeof (*ret));
  char answerbuf[100];

  if (realm) {
    debug2 (2, "Authentication realm", realm);
  }

  if (username) {
    ret->username = apr_pstrdup (pool, username);
  } else {
    SVN_ERR (prompt_and_get ("Username", answerbuf, sizeof(answerbuf)));
    ret->username = apr_pstrdup (pool, answerbuf);
  }

  SVN_ERR (prompt_and_get ("Password", answerbuf, sizeof(answerbuf)));
  ret->password = apr_pstrdup (pool, answerbuf);

  *cred = ret;
  return SVN_NO_ERROR;
}

extern svn_error_t * my_username_prompt_callback (
           svn_auth_cred_username_t **cred,
           void *baton __attribute__ ((unused)),
           const char *realm,
           svn_boolean_t may_save __attribute__ ((unused)),
           apr_pool_t *pool) {
  svn_auth_cred_username_t *ret = apr_pcalloc (pool, sizeof (*ret));
  char answerbuf[100];

  if (realm) {
    debug2 (2, "Authentication realm", realm);
  }

  SVN_ERR (prompt_and_get ("Username", answerbuf, sizeof(answerbuf)));
  ret->username = apr_pstrdup (pool, answerbuf);

  *cred = ret;
  return SVN_NO_ERROR;
}

/* **************************** */
/* Callback to get current URL */
/* ************************** */
extern svn_error_t * client_info_cb (
           void *baton,
           const char *target __attribute__ ((unused)),
           const svn_info_t *info,
           apr_pool_t *pool __attribute__ ((unused))) {
  /* Baton is a urls_type* */
  urls_type * urls = (urls_type*) baton;

  /* Check and copy repository URL */
  if (strlen (info->repos_root_URL) > sizeof(url_t) ) {
    error_data ("The repository URL is too long", info->repos_root_URL);
  }
  strcpy (urls->repository_url, info->repos_root_URL);

  /* Check and copy current URL */
  if (strlen (info->URL) > sizeof(url_t) ) {
    error_data ("The current URL is too long", info->URL);
  }
  strcpy (urls->current_url, info->URL);
  debug2 (1, "Cb got info reposit", urls->repository_url);
  debug2 (1, "Cb got info current", urls->current_url);

  /* Done */
  return SVN_NO_ERROR;
}

/* ***************************************************** */
/* Callback to retrieve creation revision number + date */
/* of node and its destination                         */
/* ************************************************** */
extern svn_error_t * log_entry_cb (
            void * baton,
            svn_log_entry_t *log_entry,
            apr_pool_t *pool) {

  long int rev = log_entry->revision;
  node_type *node_p = (node_type *) baton;
  apr_hash_t *hash_table = log_entry->changed_paths2;

  const char *date;
  const char *author;
  const char *message;
  char tmpdate[256];
  source_dest_values source_dest;

  /* Assigning default values to structure */
  source_dest.action = OTHER;
  source_dest.dst_kind = INVALID;
  source_dest.src_kind = INVALID;

  /* Svn function to return date */
  svn_compat_log_revprops_out (&author, &date,
                &message, log_entry->revprops);

  if (hash_table != NULL) {

    sprintf (tmpdate, "%ld", rev);
    debug2 (3, "Cb getting action for rev", tmpdate);

    get_action(hash_table, pool, &source_dest);
    /* If it is first COPY, then it is destination of tag/branch */
    if ((node_p->dst_rev) == 0 ) {
      if ((source_dest.action) == COPY) {
        node_p->dst_rev = rev;
        strcpy(tmpdate, date);
        svn_date_to_normal(tmpdate);
        strcpy((node_p->dst_date), tmpdate);
        strcpy(node_p->dst_path, source_dest.dst_path);
        get_node_path(node_p->dst_path);
        strcpy(node_p->src_path, source_dest.src_path);
        get_node_path(node_p->src_path);
      }
    } else {
      /* If dest is already found, look for next revision that is not MOVE */
      if (source_dest.dst_kind != TAG) {
        if (source_dest.action != MOVE) {
          node_p->src_rev = rev;
          strcpy(tmpdate, date);
          svn_date_to_normal(tmpdate);
          strcpy((node_p->src_date), tmpdate);
          sprintf (tmpdate, "%s %ld | %s %ld",
                             node_p->src_path, node_p->src_rev,
                             node_p->dst_path, node_p->dst_rev);
          debug2 (2, "Cb got node", tmpdate);
          return svn_error_create (ABORT_ITERATIONS, NULL, NULL);
        }
      }
    }
  }
  return SVN_NO_ERROR;
}

/* ***************************************************************** */
/************** Function used in previous callback **************** */
/** What is the action? Is it COPY = 1; MOVE = -1 or OTHER = 0 ?? **/
/* ************************************************************** */
extern void get_action (apr_hash_t *hash_table, apr_pool_t *pool,
                                 source_dest_values *source_dest) {
  apr_hash_index_t *index;
  svn_log_changed_path2_t *value;
  char *key;
  url_t copy_path, delete_path;
  apr_ssize_t len;
  int count = 0;
  char tmpbuf[1024];

  source_dest->src_path[0] = '\0';
  source_dest->dst_path[0] = '\0';

  /* Loop to go through all the hash table values */
  for (index = apr_hash_first(pool, hash_table);
       index != NULL; index = apr_hash_next(index)) {
    apr_hash_this(index,(const void**)&key, &len, (void**)&value);
    /* If its COPY, take destination and source kinds */
    sprintf (tmpbuf, "%c %s from %s %ld", value->action, key,
             value->copyfrom_path, value->copyfrom_rev);
    debug2 (4, "Cb action entry", tmpbuf);
    if ((value->action == 'A') && (value->copyfrom_rev != -1)) {

      if ( (compare_path(value->copyfrom_path) != INVALID)
        && (compare_path(key) != INVALID)) {

        count++;
        source_dest->src_kind = compare_path(value->copyfrom_path);
        strcpy (source_dest->src_path, value->copyfrom_path);
        source_dest->dst_kind = compare_path(key);
        strcpy(source_dest->dst_path, key);
        /* Save this copy to check validity of move */
        strcpy (copy_path, value->copyfrom_path);
      }
    } else if (value->action == 'D') {
      /* It is a deletion */
        if (compare_path(key) != INVALID) {
          /* Save this delete to check validity of move */
          strcpy (delete_path, key);
          count -= 2;
        }
    }
  } /* end for loop */

  if (count == -1) {
    if (strcmp(delete_path, copy_path) != 0) {
      /* This is not a move! */
      count = 0;
    }
  }
  source_dest->action = count;
  sprintf (tmpbuf, "%d %s to %s", source_dest->action,
           source_dest->src_path, source_dest->dst_path);
  debug2 (3, "Cb action got", tmpbuf);
}

/* ***************************************************************** */
/* 2 callbacks to retrieve and store revision numbers from log info */
/* *************************************************************** */

/* Store full revision list */
/***************************/
extern svn_error_t * log_frevlist_cb(
               void * baton,
               svn_log_entry_t *log_entry,
               apr_pool_t *pool __attribute__ ((unused))) {

  long int rev = log_entry->revision;
  char revstr[80];
  dlist *revlist_p = (dlist *) baton;

  sprintf (revstr, "%ld", rev);
  debug2 (2, "Cb got rev", revstr);
  dlist_insert (revlist_p, &rev, TRUE);
  return SVN_NO_ERROR;
}
/* Store short revision list: Stop at first copy */
/*************************************************/
extern svn_error_t * log_srevlist_cb(
            void * baton,
            svn_log_entry_t *log_entry,
            apr_pool_t *pool) {

  source_dest_values source_dest;
  apr_hash_t *hash_table = log_entry->changed_paths2;

  get_action(hash_table, pool, &source_dest);

  (void) log_frevlist_cb (baton, log_entry, pool);
  if (hash_table != NULL) {
    if ((source_dest.action) == COPY) {
      return svn_error_create (ABORT_ITERATIONS, NULL, NULL);
    }
  }
  return SVN_NO_ERROR;
}


/*********************************/
/* Callback to get list of tags */
/*******************************/
extern svn_error_t *
  print_dirent_cb(void *baton,
                  const char *path,
                  const svn_dirent_t *dirent __attribute__ ((unused)),
                  const svn_lock_t *lock __attribute__ ((unused)),
                  const char *abs_path __attribute__ ((unused)),
                  apr_pool_t *pool __attribute__ ((unused))) {

  dlist *list_p = (dlist *) baton;

  /* Discard empty entry */
  if (strlen (path) == 0) {
    return SVN_NO_ERROR;
  }
  debug2 (3, "Cb got dir entry", path);
  dlist_insert (list_p, path, TRUE);
  return SVN_NO_ERROR;
}

