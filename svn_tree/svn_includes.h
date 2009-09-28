#ifndef SVN_INCLUDES_H
#define SVN_INCLUDES_H

/* Includes needed to use svn */
#include <sys/types.h>
typedef __off64_t off64_t;

#include <assert.h>
#include <stdlib.h>

#include <apr_general.h>
#include <apr_pools.h>
#include <apr_time.h>
#include <apr_file_io.h>
#include <apr_signal.h>

#define APR_WANT_STDIO
#define APR_WANT_STRFUNC
#include <apr_want.h>

#include <svn_cmdline.h>
#include <svn_types.h>
#include <svn_pools.h>
#include <svn_error.h>
#include <svn_error_codes.h>
#include <svn_path.h>
#include <svn_repos.h>
#include <svn_fs.h>
#include <svn_time.h>
#include <svn_utf.h>
#include <svn_subst.h>
#include <svn_opt.h>
#include <svn_props.h>
#include <svn_diff.h>
#include <svn_xml.h>
#include <svn_client.h>
#include <svn_sorts.h>
#include <svn_compat.h>

#endif

