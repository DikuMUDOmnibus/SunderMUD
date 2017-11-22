/*
 *  The unique portions of SunderMud code as well as the integration efforts
 *  for code from other sources is based on the efforts of:
 *
 *  Lotherius (elfren@aros.net)
 *
 *  This code can only be used under the terms of the DikuMud, Merc,
 *  and ROM licenses. The same requirements apply to the changes that
 *  have been made.
 *
 * All other copyrights remain in place and in force.
*/

/*
 *  This includes all necessary include files
 *  Included in one file to make creation of new
 *  source files easier.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#if !defined WIN32
# include <unistd.h>
# include <sys/resource.h>
# include <sys/time.h>

# if !defined __FreeBSD__
#  include <crypt.h>
# endif
#endif

#if defined(DEBUGINFO)
# include <mcheck.h>
#endif

/* Yes some of these header files have to be in a certain order. */

#include "merc.h"
#include "globals.h"
#include "i3.h"
#include "proto.h"
#include "buffer.h"
#include "clan.h"
#include "strings.h"
#include "gsn.h"


