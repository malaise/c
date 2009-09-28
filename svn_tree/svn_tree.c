/*****************/
/*    SVN_TREE   */
/*****************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "boolean.h"
#include "dynlist.h"

#include "svn_includes.h"

#include "utilities.h"
#include "callbacks.h"


const char *syntax_err_msg = "Invalid argument. Use option '-h' for help.";

/* Used for sorting the dynlist */
static boolean less_than (const void *l, const void *r) {
  /* Casting voids in output_type and comparing the revisions */
  /* out = current position; out = next position */
  const node_type *out  = (const node_type *) l;
  const node_type *out1 = (const node_type *) r;
    if (out->src_rev < out1->src_rev) {
      return TRUE;
    }
    else if (out->src_rev == out1->src_rev) {
      return (out->dst_rev < out1->dst_rev);
    }
    else return FALSE;
}

/* Check if a rev appears in REV list */
static boolean find_rev (int node_rev, dlist *revlist) {
  long int rev;
  int low, mid, high, i;

  /* By convention, an empty revlist means no citreria */
  if (dlist_is_empty (revlist)) {
    return TRUE;
  }

  /* Dichotomy */
  low = 1;
  high = dlist_length (revlist);

  dlist_rewind (revlist, TRUE);
  do {
    mid = (low + high) / 2;
    for (i = 1; i < mid; i++) {
      dlist_move (revlist, TRUE);
    }
    dlist_read (revlist, &rev);
    if (node_rev < rev) {
      low = mid + 1;
      dlist_rewind (revlist, TRUE);
    } else if (node_rev > rev) {
      high = mid - 1;
      dlist_rewind (revlist, TRUE);
    }
    if (node_rev == rev) {
      return TRUE;
    }
  } while (low <= high);
  return FALSE;
}

/*************/
/*** MAIN ***/
/***********/
int main (int argc, const char *argv[]) {

  /* Svn variables */
  svn_error_t *err;
  apr_allocator_t *allocator;
  apr_pool_t *pool;
  svn_client_ctx_t *ctx;
  apr_array_header_t *targets, *range_array;
  const char **target;
  apr_array_header_t *revisions;
  svn_opt_revision_t peg_revision, **p_revision, revision;
  svn_opt_revision_range_t **range, range_struct;
  apr_array_header_t *revprops;
  apr_uint32_t dirent_fields;


  /* Urls of local sandbox */
  urls_type urls;

  /* Result of argument parsing */
  mode_kind whatmode;
  url_t whaturl;
  mode_kind wheremode;
  url_t whereurl;
  boolean whereloc;

  /* URL of the dir of tags (or of the tag) */
  url_t tagurl;
  url_t tmpurl, tmp1url;

  /* Result node of a SVN copy */
  node_type node;

  /* List of tags/branches to list, of revisions to check versus,
     and of node infos */
  dlist name_list, rev_list, node_list;

  /* Revision of current src */
  long int srcrev;

  /* Addr of start of label type */
  char *startp;

  /*******************/
  /* INITIALISATIONS */
  /*******************/

  /* CHECK STACK */
  /***************/
  debug1 (1, "Initializing");
  /* Check that 'ulimit -s' returns "unlimited" */
  {
    FILE * pip;
    char line[1024];

    pip = popen ("ulimit -s", "r");
    fscanf (pip, "%s", line);
    fclose (pip);
    if (strcmp(line, "unlimited") != 0) {
      error ("Stack size must be unlimited (ulimit -s 104857600)");
    }
  }
  debug1 (2, "Stack OK");

  /* INIT SVN */
  /************/
  if (svn_cmdline_init("t_svn", stderr) != EXIT_SUCCESS) {
    error ("Cannot initialise command line");
  }

  /* Creat our top-level pool.  Use a separate mutexless allocator,
   * given this application is single threaded.
   */
  if (apr_allocator_create(&allocator)) {
    error ("Cannot create allocator");
  }
  apr_allocator_max_free_set(allocator, SVN_ALLOCATOR_RECOMMENDED_MAX_FREE);
  pool = svn_pool_create_ex(NULL, allocator);
  if (pool == NULL) {
    error ("Cannot create pool");
  }
  apr_allocator_owner_set(allocator, pool);

  /* Initialize the FS library. */
  err = svn_fs_initialize(pool);
  if (err != SVN_NO_ERROR) {
     error ("Cannot initialize fs library");
  }

  /* Create a client context object. */
  svn_client_create_context(&ctx, pool);
  if (ctx == NULL) {
    error ("Cannot create client context");
  }

  /* Make sure the ~/.subversion run-time config files exist, and load. */
  err = svn_config_ensure (NULL, pool);
  if (err != SVN_NO_ERROR) {
    err = svn_config_get_config (&(ctx->config), NULL, pool);
     if (err != SVN_NO_ERROR) {
       error ("Cannot load run-time config ~/.subversion");
     }
  }

  /* Make ctx capable of  authenticating users */
  {
    svn_auth_provider_object_t *provider;
    apr_array_header_t *providers
           = apr_array_make (pool, 4, sizeof (svn_auth_provider_object_t *));

    svn_auth_get_simple_prompt_provider (&provider,
         my_simple_prompt_callback, NULL, 2, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

    svn_auth_get_username_prompt_provider (&provider,
          my_username_prompt_callback, NULL, 2, pool);
    APR_ARRAY_PUSH (providers, svn_auth_provider_object_t *) = provider;

    /* Register the auth-providers into the context's auth_baton. */
    svn_auth_open (&ctx->auth_baton, providers, pool);
  }
  debug1 (2, "Svn init OK");

  /* GET CURRENT URL */
  /**********i********/
  /* Just for sanity check or when "CUR" argument */
  {
    url_t path;
    (void)getcwd(path, sizeof(path));
    err = svn_client_info2(path,
                           NULL,
                           NULL,
                           &client_info_cb,
                           &urls,
                           svn_depth_empty,
                           NULL, ctx, pool);
    if (err != SVN_NO_ERROR) {
      error_data ("Cannot get client info: ", err->message);
    }
  }

  /* CHECK ARGUMENTS */
  /******************/
  if ( (argc == 2) && (strcmp(argv[1], "-h") == 0) ) {
    put_help (argv[0]);
    exit (1);
  }
  if ( (argc == 2) && (strcmp(argv[1], "--help") == 0) ) {
    put_help (argv[0]);
    exit (1);
  }

  /* Usage: svn_tree   <url> | CUR | ALLTAGS [ -of | -on    <url> | CUR ] */
  /* So either 1 arg or 3 */
  if ( (argc != 2) && (argc != 4) ) {
    error(syntax_err_msg);
  }

  /* First arg: what mode */
  if (strcmp(argv[1], "CUR") == 0) {
    whatmode = CUR_MODE;
  } else if (strcmp(argv[1], "ALLTAGS") == 0) {
    whatmode = ALL_MODE;
  } else {
    /* URL: Must be valid tag or branch */
    whatmode = URL_MODE;
    strcpy (whaturl, argv[1]);
    if (!is_url(whaturl)) {
      error(syntax_err_msg);
    }
  }

  if (argc == 2) {
    wheremode = ALL_MODE;
  } else {
    /* Where is global or local */
    if (strcmp(argv[2], "-of") == 0) {
      whereloc = FALSE;
    } else if (strcmp(argv[2], "-on") == 0) {
      whereloc = TRUE;
    } else {
      error(syntax_err_msg);
    }

    if (strcmp(argv[3], "CUR") == 0) {
      wheremode = CUR_MODE;
    } else {
      /* URL: Must be valid tag or branch or trunk */
      wheremode = URL_MODE;
      strcpy (whereurl, argv[3]);
      if (!is_url(whereurl)) {
        error(syntax_err_msg);
      }
    }
  }
  debug1 (2, "Arguments parsed OK");

  /* INIT GENERAL SVN DATA */
  /*************************/
  /* Init first array with URL */
  targets = apr_array_make(pool, 1, sizeof (char*));
  target = apr_array_push(targets);
  /* Will need to set *target */

  /* Init revisions : unspecified */
  revisions = apr_array_make(pool, 1, sizeof(svn_opt_revision_t));
  p_revision = apr_array_push(revisions);
  revision.kind = svn_opt_revision_unspecified;
  *p_revision = &(revision);

  /* Init range : base to head */
  range_array = apr_array_make(pool, 1, sizeof( svn_opt_revision_range_t));
  range = apr_array_push(range_array);
  range_struct.start.kind = svn_opt_revision_base;
  range_struct.end.kind = svn_opt_revision_head;
  range_struct.start.kind = svn_opt_revision_unspecified;
  range_struct.end.kind = svn_opt_revision_unspecified;
  *range = &(range_struct);

  /* Peg revision */
  peg_revision.kind = svn_opt_revision_unspecified;
  revprops = apr_array_make(pool, 1, sizeof(svn_opt_revision_t));


  /* INIT GLOBAl DATA */
  /********************/
  dlist_init (&name_list, sizeof(tag_path_type)); /* Tags' URLs */
  dlist_init (&rev_list, sizeof(int));            /* Revison numbers of where */
  dlist_init (&node_list, sizeof(node_type));     /* Nodes infos (result) */
  debug1 (2, "Data init OK");

  /*******************************************/
  /* LIST TAGS AND GET THEIR NODE, LIST REVS */
  /*******************************************/
  debug1 (1, "Starting");

  /* MAKE LIST OF URLS */
  /*********************/
  if (whatmode == ALL_MODE) {
    /* All tags */
    strcpy (tagurl, urls.repository_url);
    strcat (tagurl, "/tags");
    dirent_fields = SVN_DIRENT_ALL;
    debug2 (2, "Listing tags", tagurl);
    err  = svn_client_list2(tagurl,
                            &peg_revision,
                            &peg_revision,
                            svn_depth_immediates,
                            dirent_fields,
                            FALSE,
                            print_dirent_cb,
                            &name_list,
                            ctx,
                            pool);
    if (err != SVN_NO_ERROR) {
      error_data ("Cannot list tags: ", err->message);
      exit(1);
    }
    if (dlist_is_empty (&name_list)) {
      /* No tag at all */
      debug1 (1, "No tag found");
      exit (0);
    }
    /* Replace each entry (tag name) by full path */
    dlist_rewind (&name_list, TRUE);
    strcat (tagurl, "/");
    for (;;) {
      strcpy (tmpurl, tagurl);
      dlist_read (&name_list, tmp1url);
      strcat(tmpurl, tmp1url);
      dlist_replace (&name_list, tmpurl);
      debug2 (2, "Found tag", tmpurl);
      if (dlist_get_pos (&name_list, FALSE) == 1) break;
      dlist_move (&name_list, TRUE);
    }
  } else {
    /* One tag (current or specified) */
    if (whatmode == URL_MODE) {
      strcpy (tagurl, whaturl);
    } else {
      strcpy (tagurl, urls.current_url);
    }
    {
      /* Check URL and strip tail */
      startp = url_check (tagurl, FALSE);
      if (startp == NULL) {
        error("Wrong URL. Does not point to branches or tags");
      }
      get_node_path (startp);
      debug2 (2, "Added URL", tmpurl);
      dlist_insert (&name_list, tagurl, TRUE);
    }
  }

  /* GET INFO ON EACH URL */
  /************************/
  debug1 (2, "Getting nodes");
  dlist_rewind (&name_list, TRUE);
  for (;;) {

    /* Get node info on current tag */
    dlist_read (&name_list, tagurl);
    *target = tagurl;
    node.dst_rev = 0;
    node.src_rev = 0;
    debug2 (2, "Getting node of", tagurl);
    err = svn_client_log5(targets,
                          &peg_revision,
                          range_array,
                          0,
                          TRUE,
                          FALSE,
                          FALSE,
                          NULL,
                          log_entry_cb,
                          &node,
                          ctx,
                          pool);
    if (err != SVN_NO_ERROR) {
      if (err->apr_err != ABORT_ITERATIONS) {
        error_data ("Cannot log history:", err->message);
        exit (1);
      } else {
        /* The dest node is the target of last copy, not the final name
           of the tag. So restore the final tag name */
        startp = url_check (tagurl, TRUE);
        strcpy (node.dst_path, startp);
        get_node_path (node.dst_path);
        dlist_insert(&node_list, &node, TRUE);
        dlist_move (&node_list, TRUE);
      }
    } else {
      /* Cannot find node of this tag, skip it */
      debug2 (1, "Cannot find node", tagurl);
    }
    if (dlist_get_pos (&name_list, FALSE) == 1) break;
    dlist_move (&name_list, TRUE);
  }
  if (dlist_is_empty (&node_list)) {
    /* No mode node */
    debug1 (1, "No node found");
    exit (0);
  }
  dlist_rewind (&rev_list, TRUE);
  dlist_rewind (&node_list, TRUE);

  /* LIST  REVS (WHERE) */
  /**********************/
  if (wheremode != ALL_MODE) {
    if (wheremode == URL_MODE) {
      strcpy (tagurl, whereurl);
    } else {
      strcpy (tagurl, urls.current_url);
    }
    {
      /* Check URL and strip tail */
      startp = url_check (tagurl, TRUE);
      if (startp == NULL) {
        error("Wrong URL. Does not point to branches, tags or trunk");
      }
      get_node_path (startp);
    }
    debug1 (2, "Listing revs");
    *target = tagurl;
    err = svn_client_log5(targets,
                          &peg_revision,
                          range_array,
                          0,
                          TRUE,
                          FALSE,
                          FALSE,
                          NULL,
                         ((whereloc) ? log_srevlist_cb : log_frevlist_cb),
                          &rev_list,
                          ctx,
                          pool);
    if ((err != SVN_NO_ERROR) && (err->apr_err != ABORT_ITERATIONS)){
      error_data ("Cannot get rev numbers: ", err->message);
    }
    dlist_rewind (&rev_list, TRUE);
  }

  /*********************************/
  /* PUT NODES THAT APPEAR IN REVS */
  /*********************************/

  debug1 (2, "Putting nodes");
  dlist_sort (&node_list, less_than);
  /* FOR EACH NODE */
  /*****************/
  srcrev = -1;
  for (;;) {
    dlist_read (&node_list, &node);
    if (find_rev (node.src_rev, &rev_list) ) {
      if (node.src_rev != srcrev) {
        /* A new source => a new line */
        if (srcrev != -1) {
          printf("\n");
        }
        printf("%ld %s %s", node.src_rev, node.src_path, node.src_date);
        srcrev = node.src_rev;
      }
      /* This dest */
      printf(" | %ld %s %s", node.dst_rev, node.dst_path, node.dst_date);
    } else {
      sprintf (tmpurl, "%ld %s", node.src_rev, node.dst_path);
      debug2 (2, "Discarding node", tmpurl);
    }
    if (dlist_get_pos (&node_list, FALSE) == 1) break;
    dlist_move (&node_list, TRUE);
  }
  if (srcrev != -1) {
    /* At least one rev was put */
    printf("\n");
  }

  /***********/
  /* CLEANUP */
  /***********/
  /* Clear all dyn lists and free memory */
  dlist_delete_all (&name_list);
  dlist_delete_all (&rev_list);
  dlist_delete_all (&node_list);

  /* Clear memory
  apr_array_clear (targets);
  apr_array_clear (revisions);
  apr_array_clear (range_array);
  apr_array_clear (revprops);
  */

  /* Done */
  exit (0);
}

