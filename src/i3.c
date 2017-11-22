/*
 * Copyright (c) 2000 Fatal Dimensions
 * 
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * 
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code
 * Copyright (c)2001-2003 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 *
 * I condensed the 14 or so Fatal Dimensions source code files into this
 * one file, because I for one find it far easier to maintain when all of
 * the functions are right here in one file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <time.h>
#if defined(sun)
   #include <strings.h>
#endif
#if defined(I3CIRCLE)
   #include "conf.h"
   #include "sysdep.h"
   #include "structs.h"
   #include "utils.h"
   #include "comm.h"
   #include "db.h"
   #include "handler.h"
   #include "interpreter.h"
   #include "i3.h"
   time_t current_time;
#endif

#if defined(I3SMAUG) || defined(I3CHRONICLES)
   #include "mud.h"
#ifdef I3CHRONICLES
   #include "factions.h"
   #include "files.h"
   #include "i3.h"
#endif
#endif
#if defined(I3ROM) || defined(I3MERC) || defined(I3UENVY) || defined(I31STMUD)
   #include "merc.h"
   #ifdef I3ROM
      #include "tables.h"
   #endif
#ifdef I31STMUD
   #include "tables.h"
   #include "globals.h"
#endif
#endif
#ifdef I3ACK
   #include "ack.h"
#endif
#if defined(I3SUNDER)
   #include "everything.h"
#endif

/* Global variables for I3 */
char I3_input_buffer[IPS];
char I3_output_buffer[OPS];
char I3_currentpacket[IPS];
bool packetdebug = FALSE;  /* Packet debugging toggle, can be turned on to check outgoing packets */
long I3_input_pointer = 0;
long I3_output_pointer = 4;
char *I3_THISMUD;
char *I3_ROUTER_NAME;
const char *manual_router;
int I3_socket;
int i3wait; 		   /* Number of game loops to wait before attempting to reconnect when a socket dies */
int i3timeout;             /* Number of loops to wait before giving up on an initial router connection */
time_t ucache_clock;	   /* Timer for pruning the ucache */
long bytes_received;
long bytes_sent;
#ifdef I3CIRCLE

#endif

I3_MUD *this_mud;
I3_MUD *first_mud;
I3_MUD *last_mud;

I3_CHANNEL *first_I3chan;
I3_CHANNEL *last_I3chan;
I3_BAN *first_i3ban;
I3_BAN *last_i3ban;
UCACHE_DATA *first_ucache;
UCACHE_DATA *last_ucache;
ROUTER_DATA *first_router;
ROUTER_DATA *last_router;

void i3_printf( CHAR_DATA *ch, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
void i3pager_printf( CHAR_DATA *ch, const char *fmt, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
void i3bug( const char *format, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );
void i3log( const char *format, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );
void I3_send_channel_listen( I3_CHANNEL *channel, bool lconnect );
void I3_write_channel_config( void );

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 *
 * Renamed so it can play itself system independent.
 * Samson 10-12-03
 */
size_t i3strlcpy( char *dst, const char *src, size_t siz )
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = siz;

   /* Copy as many bytes as will fit */
   if( n != 0 && --n != 0 )
   {
	do
      {
	   if( ( *d++ = *s++ ) == 0 )
		break;
	}
      while( --n != 0 );
   }

   /* Not enough room in dst, add NUL and traverse rest of src */
   if( n == 0 )
   {
	if( siz != 0 )
	   *d = '\0'; /* NUL-terminate dst */
	while( *s++ )
	   ;
   }
   return( s - src - 1 ); /* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(initial dst) + strlen(src); if retval >= siz,
 * truncation occurred.
 *
 * Renamed so it can play itself system independent.
 * Samson 10-12-03
 */
size_t i3strlcat( char *dst, const char *src, size_t siz )
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = siz;
   size_t dlen;

   /* Find the end of dst and adjust bytes left but don't go past end */
   while( n-- != 0 && *d != '\0' )
	d++;
   dlen = d - dst;
   n = siz - dlen;

   if( n == 0 )
	return( dlen + strlen(s) );
   while( *s != '\0' )
   {
	if( n != 1 )
      {
	   *d++ = *s;
	   n--;
	}
	s++;
   }
   *d = '\0';
   return( dlen + ( s - src ) ); /* count does not include NUL */
}

bool i3str_prefix( const char *astr, const char *bstr )
{
   if ( !astr )
   {
	i3bug( "Strn_cmp: null astr." );
	return TRUE;
   }

   if ( !bstr )
   {
	i3bug( "Strn_cmp: null bstr." );
	return TRUE;
   }

   for ( ; *astr; astr++, bstr++ )
   {
	if( LOWER(*astr) != LOWER(*bstr) )
	   return TRUE;
   }
   return FALSE;
}

char *rankbuffer( CHAR_DATA *ch )
{
   static char rbuf[LGST];

   if( I3PERM(ch) >= I3PERM_IMM )
   {
      i3strlcpy( rbuf, "{YStaff", LGST );

      if( CH_I3RANK(ch) && CH_I3RANK(ch)[0] != '\0' )
         snprintf( rbuf, LGST, "{Y%s", CH_I3RANK(ch) );
   }
   else
   {
      i3strlcpy( rbuf, "{BPlayer", LGST );

   	if( CH_I3RANK(ch) && CH_I3RANK(ch)[0] != '\0' )
	   snprintf( rbuf, LGST, "{B%s", CH_I3RANK(ch) );
   }
   return rbuf;
}

char *const perm_names[] =
{
   "Notset", "None", "Mort", "Imm", "Admin", "Imp"
};

int get_permvalue( const char *flag )
{
   unsigned int x;

   for ( x = 0; x < (sizeof(perm_names) / sizeof(perm_names[0])); x++ )
      if ( !str_cmp(flag, perm_names[x]) )
         return x;
   return -1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes. do not mess with case.
 * as opposed to native one_argument which ignores case.
 */
char *i3one_argument( char *argument, char *arg_first )
{
   char cEnd;
   int count;

   count = 0;

   while ( isspace(*argument) )
	argument++;

   cEnd = ' ';
   if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

   while ( *argument != '\0' || ++count >= 255 )
   {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = (*argument);
	arg_first++;
	argument++;
   }
   *arg_first = '\0';

   while ( isspace(*argument) )
	argument++;

   return argument;
}

/* Generic log function which will route the log messages to the appropriate system logging function */
void i3log( const char *format, ... )
{
   char buf[LGST];
   va_list ap;

   va_start( ap, format );
   vsnprintf( buf, LGST, format, ap );
   va_end( ap );

#ifndef I3CIRCLE
   log_string( buf );
#else
   basic_mud_log( buf );
#endif
   return;
}

/* Generic bug logging function which will route the message to the appropriate function that handles bug logs */
void i3bug( const char *format, ... )
{
   char buf[LGST];
   va_list ap;

   va_start( ap, format );
   vsnprintf( buf, LGST, format, ap );
   va_end( ap );

#if defined(I3SMAUG)
   bug( "%s", buf );
#elif defined(I31STMUD)
   bug( buf );
#elif defined(I3CIRCLE)
   basic_mud_log( buf );
#elif defined(I3SUNDER)
   bugf ( buf );
#else
   bug( buf, 0 );
#endif
   return;
}

/* You need to change the &, } and { tokens in the table below, and in the if statement
 * in i3_tagtofish to match what your mud uses to identify a color token with.
 *
 * & is the foreground text token.
 * } is the blink text token.
 * { is the background text token.
 */
#define I3MAX_ANSI 50
#define COL_INVALID -1

const char *i3ansi_conversion[I3MAX_ANSI][3] =
{
	/*
	 * Conversion Format Below:
	 *
	 * { "<MUD TRANSLATION>", "PINKFISH", "ANSI TRANSLATION" }
	 *
	 * Foreground Standard Colors
	 */
	{ "{d", "%^BLACK%^",   "\033[0;0;30m" }, /* Black */
	{ "{r", "%^RED%^",     "\033[0;0;31m" }, /* Dark Red */
	{ "{g", "%^GREEN%^",   "\033[0;0;32m" }, /* Dark Green */
	{ "{y", "%^ORANGE%^",  "\033[0;0;33m" }, /* Orange/Brown */
	{ "{b", "%^BLUE%^",    "\033[0;0;34m" }, /* Dark Blue */
	{ "{m", "%^MAGENTA%^", "\033[0;0;35m" }, /* Purple/Magenta */
	{ "{c", "%^CYAN%^",    "\033[0;0;36m" }, /* Cyan */
	{ "{w", "%^WHITE%^",   "\033[0;0;37m" }, /* Grey */

    /* Foreground extended colors */
	{ "{D", "%^BLACK%^%^BOLD%^",   "\033[0;1;30m" }, /* Dark Grey */
	{ "{R", "%^RED%^%^BOLD%^",     "\033[0;1;31m" }, /* Red */
	{ "{G", "%^GREEN%^%^BOLD%^",   "\033[0;1;32m" }, /* Green */
	{ "{Y", "%^YELLOW%^",          "\033[0;1;33m" }, /* Yellow */
	{ "{B", "%^BLUE%^%^BOLD%^",    "\033[0;1;34m" }, /* Blue */
	{ "{M", "%^MAGENTA%^%^BOLD%^", "\033[0;1;35m" }, /* Pink */
	{ "{C", "%^CYAN%^%^BOLD%^",    "\033[0;1;36m" }, /* Light Blue */
	{ "{W", "%^WHITE%^%^BOLD%^",   "\033[0;1;37m" }, /* White */
    
    	/* Text Affects */
    
	{ "{x", "%^RESET%^",     "\033[0m" }, /* Reset Text */
	{ "{O", "%^BOLD%^",      "\033[1m" }, /* Bolden Text(Brightens it) */
	{ "{x", "%^EBOLD%^",	 "\033[0m" }, /* Assumed to be a reset tag to stop bold */
	{ "{3", "%^UNDERLINE%^", "\033[4m" }, /* Underline Text */
	{ "{&", "%^FLASH%^",     "\033[5m" }, /* Blink Text */
	{ "{I", "%^ITALIC%^",    "\033[3m" }, /* Italic Text */
	{ "{4", "%^REVERSE%^",   "\033[7m" }, /* Reverse Background and Foreground Colors */
	{ "{S", "%^STRIKEOUT%^", "\033[9m" }, /* Strikeover */

	/* Background colors */
    /* We don't really have these on sundermud.... */
	{ "{x", "%^B_BLACK%^",   "\033[40m" }, /* Black */
	{ "{4", "%^B_RED%^",     "\033[41m" }, /* Red */
	{ "{4", "%^B_GREEN%^",   "\033[42m" }, /* Green */
	{ "{4", "%^B_ORANGE%^",  "\033[43m" }, /* Orange */
	{ "{4", "%^B_YELLOW%^",  "\033[43m" }, /* Yellow, which may as well be orange since ANSI doesn't do that */
	{ "{4", "%^B_BLUE%^",    "\033[44m" }, /* Blue */
	{ "{4", "%^B_MAGENTA%^", "\033[45m" }, /* Purple/Magenta */
	{ "{4", "%^B_CYAN%^",    "\033[46m" }, /* Cyan */
	{ "{4", "%^B_WHITE%^",   "\033[47m" }, /* White */

	/* Blinking foreground standard color */
	{ "{&{d", "%^BLACK%^%^FLASH%^",           "\033[0;5;30m" }, /* Black */
	{ "{&{r", "%^RED%^%^FLASH%^",             "\033[0;5;31m" }, /* Dark Red */
	{ "{&{g", "%^GREEN%^%^FLASH%^",           "\033[0;5;32m" }, /* Dark Green */
	{ "{&{y", "%^ORANGE%^%^FLASH%^",          "\033[0;5;33m" }, /* Orange/Brown */
	{ "{&{b", "%^BLUE%^%^FLASH%^",            "\033[0;5;34m" }, /* Dark Blue */
	{ "{&{m", "%^MAGENTA%^%^FLASH%^",         "\033[0;5;35m" }, /* Magenta/Purple */
	{ "{&{c", "%^CYAN%^%^FLASH%^",            "\033[0;5;36m" }, /* Cyan */
	{ "{&{w", "%^WHITE%^%^FLASH%^",           "\033[0;5;37m" }, /* Grey */
	{ "{&{D", "%^BLACK%^%^BOLD%^%^FLASH%^",   "\033[1;5;30m" }, /* Dark Grey */
	{ "{&{R", "%^RED%^%^BOLD%^%^FLASH%^",     "\033[1;5;31m" }, /* Red */
	{ "{&{G", "%^GREEN%^%^BOLD%^%^FLASH%^",   "\033[1;5;32m" }, /* Green */
	{ "{&{Y", "%^YELLOW%^%^FLASH%^",          "\033[1;5;33m" }, /* Yellow */
	{ "{&{B", "%^BLUE%^%^BOLD%^%^FLASH%^",    "\033[1;5;34m" }, /* Blue */
	{ "{&{M", "%^MAGENTA%^%^BOLD%^%^FLASH%^", "\033[1;5;35m" }, /* Pink */
	{ "{&{C", "%^CYAN%^%^BOLD%^%^FLASH%^",    "\033[1;5;36m" }, /* Light Blue */
	{ "{&{W", "%^WHITE%^%^BOLD%^%^FLASH%^",   "\033[1;5;37m" }  /* White */
};

/*
 * Simple check to test if a particular code is a valid color. If not, then we can find
 * other things to do, in some cases. -Orion
 */
int I3_validcolor( char code[3] )
{
   int c = 0, colmatch = COL_INVALID;

    if ( code[0] && code[1] && ( code[0] == '{' ) )
   {
	for( c = 0; c < I3MAX_ANSI; c++ )
	{
	    if( i3ansi_conversion[c][0][0] == code[0] && i3ansi_conversion[c][0][1] == code[1] )
	    {
		colmatch = c;
		break;
	    }
	}
   }
   return colmatch;
}

/*
 * Convert txt into pinkfish valid color codes, while parsing the color code information in
 * the proper manner. Color codes should be changed to reflect local color. & is the basic
 * color, { is the background color, and } is blinking color. -Orion
 */
char *I3_tagtofish( const char *txt )
{
    int c, x, count = 0;
    static char tbuf[LGST*3];
    char code[3];

    if( !txt || *txt == '\0' )
	return "";

    tbuf[0] = '\0';

    for( count = 0; count < LGST; count++, txt++ )
    {	
	if( *txt == '\0' )
	   break;

	if( *txt != '{' )
	{
	   tbuf[count] = *txt;
	}
	else
	{
	    code[0] = *txt;
	    code[1] = *(++txt);
	    code[2] = '\0';

	    if ( !code[1] || code[1] == '\0' )
	    {
		tbuf[count] = code[0];
		count++;
		break;
	    }
	    else if ( code[0] == code[1] )
	    {
		tbuf[count] = code[0];
	    }
	    else if ( ( c = I3_validcolor( code ) ) != COL_INVALID )
	    {
		for( x = 0; i3ansi_conversion[c][1][x]; x++, count++ )
		{
		    tbuf[count] = i3ansi_conversion[c][1][x];
		}
		count--;
	    }
	    else
	    {
		tbuf[count]   = code[0];
		tbuf[++count] = code[1];
	    }
	}
    }
    tbuf[count] = '\0';

    return tbuf;
}

/* Takes the string you pass it and converts its Pinkfish color tags into ANSI codes */
char *I3_fishtoansi( const char *inbuf, CHAR_DATA *ch )
{
   char *cp, *cp2;
   char col[50];
   static char abuf[LGST*3];
   int len;
   bool found = FALSE;

   /* catch the trivial case first (for speed) */
   cp = strstr( inbuf, "%^" );
   if( !cp )
   {
      strcpy( abuf, inbuf ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
	return abuf;
   }

   abuf[0] = '\0';
   col[0] = '\0';

   do
   {
      cp2 = strstr( cp+2, "%^" );
      if (!cp2) break; /* unmatched single %^ */

      /* ok, we have 2 delimiters now.
       * get the converted color and its length */

      len = cp2 - cp + 2;
    
      if( len == 4 )
      { /* means "%^%^" which is the escape */
         len = 2;
         strcpy( col, "%^" ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
      }
	else
	{
         if( I3IS_SET( I3FLAG(ch), I3_COLOR ) )
         {
	   int c;

         for( c = 0; c < I3MAX_ANSI; c++ )
         {
	      if( !strncmp( cp, i3ansi_conversion[c][1], len ) )
	      {
	         strcpy( col, i3ansi_conversion[c][2] ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
	         len = strlen( col );
		   found = TRUE;
		   break;
	      }
         }
	}
         else
         {
            strcpy( col, "" ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
            len = 0;
            found = TRUE;
         }
	}
	if( !found )
	   strcpy( col, cp ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */

      /* copy the first part into the buffer and add the converted color code */
      strncat( abuf, inbuf, cp-inbuf ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
      strncat( abuf, col, len ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
      inbuf = cp2+2;

   } while( (cp = strstr( inbuf, "%^" )) );

   /* copy the rest */
   strcat( abuf, inbuf ); /* Leave this one alone! BAD THINGS(TM) will happen if you don't! */
   return abuf;
}

/* Generic substitute for write_to_buffer since not all codebases seem to have it */
void to_char_buffer( DESCRIPTOR_DATA *d, const char *txt )
{
   write_to_buffer( d, txt, 0 );
   return;
}

/* Generic substitute for send_to_char with color support */
void i3_to_char( const char *txt, CHAR_DATA *ch )
{
   char buf[LGST*3];
   char buf2[LGST*3];

   if( !ch )
   {
	i3bug( "%s", "i3_to_char: NULL ch!" );
	return;
   }

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
	i3bug( "i3_to_char: NULL descriptor for %s!", CH_I3NAME(ch) );
	return;
   }

   snprintf( buf, LGST*3, "%s", I3_tagtofish( txt ) );
   snprintf( buf2, LGST*3, "%s", I3_fishtoansi( buf, ch ) );
   to_char_buffer( ch->desc, buf2 );
   to_char_buffer( ch->desc, "\033[0m" ); /* Reset color to stop bleeding */
   return;
}

/* Generic substitute for ch_printf that passes to i3_to_char for color support */
void i3_printf( CHAR_DATA *ch, const char *fmt, ... )
{
   char buf[LGST*2];
   va_list args;

   va_start( args, fmt );
   vsnprintf( buf, LGST*2, fmt, args );
   va_end( args );

   i3_to_char( buf, ch );
}

/* Generic send_to_pager type function to send to the proper code for each codebase */
void i3send_to_pager( const char *txt, CHAR_DATA *ch )
{
#if defined(I3SMAUG) || defined(I3CHRONICLES) || defined(I3ROM) || defined(I31STMUD) || defined (I3SUNDER)
   char buf[LGST*3];
   char buf2[LGST*3];

   snprintf( buf, LGST*3, "%s", I3_tagtofish( txt ) );
   snprintf( buf2, LGST*3, "%s\e[0m", I3_fishtoansi( buf, ch ) );
#endif

#if defined(I3SMAUG) || defined(I3CHRONICLES)
   send_to_pager( buf2, ch );
#elif defined(I3ROM) || defined (I3SUNDER)
   page_to_char( buf2, ch );
#elif defined(I31STMUD)
   sendpage( ch, buf2 );
#else
   i3_to_char( txt, ch );
#endif
   return;
}

/* Generic pager_printf type function */
void i3pager_printf( CHAR_DATA *ch, const char *fmt, ... )
{
   char buf[LGST*2];
   va_list args;
 
   va_start( args, fmt );
   vsnprintf( buf, LGST*2, fmt, args );
   va_end( args );

   i3send_to_pager( buf, ch );
   return;
}

/*  I3_getarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
const char *I3_getarg( const char *argument, char *arg, int length )
{
   int len = 0;

   if( !argument || argument[0] == '\0' )
   {
      if( arg )
         arg[0] = '\0';

      return argument;
   }

   while (*argument && isspace( *argument ) )
      argument++;

   if( arg )
      while( *argument && !isspace( *argument ) && len < length-1 )
         *arg++ = *argument++, len++;
   else
      while( *argument && !isspace( *argument ) )
         argument++;

   while( *argument && !isspace( *argument ) )
      argument++;

   while( *argument && isspace( *argument ) )
      argument++;

   if( arg )
      *arg = '\0';

   return argument;
}

/* Check for a name in a list */
bool I3_hasname( const char *list, const char *name )
{
   const char *p;
   char arg[SMST];

   if( !list )
	return FALSE;

   p = I3_getarg( list, arg, SMST );
   while( arg[0] )
   {
      if( !strcasecmp( name, arg ) )
         return TRUE;
      p = I3_getarg( p, arg, SMST );
   }
   return FALSE;
}

/* Add a name to a list */
void I3_flagchan( char **list, const char *name )
{
   char buf[LGST];

   if( I3_hasname( *list, name ) )
      return;

   if( *list && *list[0] != '\0' )
      snprintf( buf, LGST, "%s %s", *list, name );
   else
      i3strlcpy( buf, name, LGST );
  
   I3STRFREE( *list );
   *list = I3STRALLOC( buf );
}

/* Remove a name from a list */
void I3_unflagchan( char **list, const char *name )
{
   char buf[LGST], arg[SMST];
   const char *p;

   buf[0] = '\0';
   p = I3_getarg( *list, arg, SMST );
   while( arg[0] )
   {
      if( strcasecmp( arg, name ) )
      {
         if( buf[0] )
	      i3strlcat( buf, " ", LGST );
         i3strlcat( buf, arg, LGST );
      }
      p = I3_getarg( p, arg, SMST );
   }
   I3STRFREE( *list );
   *list = I3STRALLOC( buf );
}

/*
 * You need to change the & and { tokens to match what your mud uses to identify color tags.
 * & is the forgound color, { is the background color, and } is the blinking color. Returns
 * the string length of an argument, excluding valid color codes. -Orion
 */
int I3_strlen_color( const char *argument )
{
   unsigned int i, length;
   const char *str;
   char code[3];

   str = argument;
   if( argument[0] == '\0' )
	return 0;

   for( length = i = 0; i < strlen( argument ); i++ )
   {
	if( str[i] != '{' )
      {
	   length++;
	   continue;
      }

	code[0] = str[i];
	code[1] = str[++i];
	code[2] = '\0';

	if ( I3_validcolor( code ) == COL_INVALID )
	{
	   if ( !code[1] || code[1] == '\0' || code[0] == code[1] )
		length++;
	   else
		length += 2;
	}
   }
   return length;
}

/*
 * You need to change the & and { tokens to match what your mud uses to identify color tags.
 * & is the forgound color, { is the background color, and } is the blinking color. This
 * function returns the max length of a field up to a certain point. Color codes included or
 * excluded at your leisure. -Orion
 */
int I3_strnlen_color( const char *argument, int maxsize, bool total )
{
   unsigned int i, length;
   const char *str;
   char code[3];

   str = argument;
   if( argument[0] == '\0' )
	return 0;

   for( length = i = 0; i < strlen( argument ); i++ )
   {
	if ( length >= (unsigned int)maxsize )
	    break;

	if( str[i] != '{' )
      {
	   length++;
	   continue;
      }

	code[0] = str[i];
	code[1] = str[++i];
	code[2] = '\0';

	if ( I3_validcolor( code ) == COL_INVALID )
	{
	   if ( !code[1] || code[1] == '\0' || code[0] == code[1] )
		length++;
	   else
		length += 2;
	}
   }
   if ( total )
	return i;

   return length;
}

/*
 * Returns an initial-capped string.
 */
char *i3capitalize( const char *str )
{
    static char strcap[LGST];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = tolower( str[i] );
    strcap[i] = '\0';
    strcap[0] = toupper( strcap[0] );
    return strcap;
}

/* Borrowed from Samson's new_auth snippet - checks to see if a particular player exists in the mud.
 * This is called from i3locate and i3finger to report on offline characters.
 */
bool i3exists_player( char *name )
{
#ifndef I3CIRCLE
   struct stat fst;
   char buf[256];
#endif

#ifdef I3CIRCLE
   return FALSE;
#else
   /* Stands to reason that if there ain't a name to look at, they damn well don't exist! */
   if( !name || !str_cmp( name, "" ) )
      return FALSE;

   snprintf( buf, 256, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), i3capitalize( name ) );

   if( stat( buf, &fst ) != -1 )
	return TRUE;
   else
	return FALSE;
#endif
}

bool verify_i3layout( const char *fmt, int number )
{
  const char *c;
  int i = 0;

  c = fmt;
  while( ( c = strchr(c, '%') ) != NULL )
  {
    if( *( c+1 ) == '%' )  /* %% */
    {
      c += 2;
      continue;
    }
    
    if( *(c+1) != 's' )  /* not %s */
      return FALSE;

    c++;
    i++;
  }

  if( i != number )
    return FALSE;

  return TRUE;
}

/* Fixed this function yet again. If the socket is negative or 0, then it will return
 * a FALSE. Used to just check to see if the socket was positive, and that just wasn't
 * working for the way some places checked for this. Any negative value is an indication
 * that the socket never existed.
 */
bool I3_is_connected( void )
{
    if( I3_socket < 1 )
	return FALSE;

    return TRUE;
}

/*
 * Add backslashes in front of the " and \'s
 */
char *I3_escape( char *ps ) 
{
   static char xnew[LGST];
   char *pnew = xnew;

   while( ps[0] ) 
   {
	if( ps[0] == '"' ) 
	{
	   pnew[0]= '\\';
	   pnew++;
	}
	if( ps[0] == '\\' ) 
	{
	   pnew[0] = '\\';
	   pnew++;
	}
	pnew[0] = ps[0];
	pnew++;
	ps++;
   }
   pnew[0] = '\0';
   return xnew;
}

/*
 * Remove "'s at begin/end of string
 * If a character is prefixed by \'s it also will be unescaped
 */
void I3_remove_quotes( char **ps ) 
{
   char *ps1, *ps2;

   if( *ps[0] == '"' )
	(*ps)++;
   if( (*ps)[strlen(*ps)-1] == '"' )
	(*ps)[strlen(*ps)-1] = 0;

   ps1 = ps2 = *ps;
   while( ps2[0] ) 
   {
	if( ps2[0] == '\\' ) 
	{
	   ps2++;
	}
	ps1[0] = ps2[0];
	ps1++;
	ps2++;
   }
   ps1[0] = '\0';
}

/* Searches through the channel list to see if one exists with the localname supplied to it. */
I3_CHANNEL *find_I3_channel_by_localname( char *name )
{
    I3_CHANNEL *channel = NULL;

    for( channel = first_I3chan; channel; channel = channel->next )
    {
	if( !channel->local_name )
	   continue;

	if( !str_cmp( channel->local_name, name ) )
	   return channel;
    }
    return NULL;
}

/* Searches through the channel list to see if one exists with the I3 channel name supplied to it.*/
I3_CHANNEL *find_I3_channel_by_name( char *name ) 
{
    I3_CHANNEL *channel = NULL;

    for( channel = first_I3chan; channel; channel = channel->next )
    {
	if( !str_cmp( channel->I3_name, name ) )
	   return channel;
    }
    return NULL;
}

/* Sets up a channel on the mud for the first time, configuring its default layout.
 * If you don't like the default layout of channels, this is where you should edit it to your liking.
 */
I3_CHANNEL *new_I3_channel( void ) 
{
   I3_CHANNEL *cnew;

   I3CREATE( cnew, I3_CHANNEL, 1 );
   I3LINK( cnew, first_I3chan, last_I3chan, next, prev );
   return cnew;
}

/* Deletes a channel's information from the mud. */
void destroy_I3_channel( I3_CHANNEL *channel )
{
   int x;

   if ( channel == NULL )
   {
	i3bug( "%s", "destroy_I3_channel: Null parameter" );
	return;
   }

   I3STRFREE( channel->local_name );
   I3STRFREE( channel->host_mud );
   I3STRFREE( channel->I3_name );
   I3STRFREE( channel->layout_m );
   I3STRFREE( channel->layout_e );

   for( x = 0; x < MAX_I3HISTORY; x++ )
   {
	if( channel->history[x] && channel->history[x] != '\0' )
	   I3STRFREE( channel->history[x] );
   }

   I3UNLINK( channel, first_I3chan, last_I3chan, next, prev );
   I3DISPOSE( channel );
}

/* Finds a mud with the name supplied on the mudlist */
I3_MUD *find_I3_mud_by_name( char *name ) 
{
   I3_MUD *mud;

   for( mud = first_mud; mud; mud = mud->next )
   {
	if( !str_cmp( mud->name, name ) )
	   return mud;
   }
   return NULL;
}

I3_MUD *new_I3_mud( char *name )
{
   I3_MUD *cnew, *mud_prev;

   I3CREATE( cnew, I3_MUD, 1 );
   cnew->name = I3STRALLOC( name );

   for( mud_prev = first_mud; mud_prev; mud_prev = mud_prev->next )
      if( strcasecmp( mud_prev->name, name ) >= 0 )
         break;

   if( !mud_prev )
      I3LINK( cnew, first_mud, last_mud, next, prev );
   else
      I3INSERT( cnew, mud_prev, first_mud, next, prev );

   return cnew;
}

void destroy_I3_mud( I3_MUD *mud )
{
   if( mud == NULL ) 
   {
	i3bug( "%s", "destroy_I3_mud: Null parameter" );
	return;
   }

   I3STRFREE( mud->name );
   I3STRFREE( mud->ipaddress );
   I3STRFREE( mud->mudlib );
   I3STRFREE( mud->base_mudlib );
   I3STRFREE( mud->driver );
   I3STRFREE( mud->mud_type );
   I3STRFREE( mud->open_status );
   I3STRFREE( mud->admin_email );
   I3STRFREE( mud->telnet );
   I3STRFREE( mud->web_wrong );
   I3STRFREE( mud->banner );
   I3STRFREE( mud->web );
   I3STRFREE( mud->time );
   I3STRFREE( mud->daemon );
   I3STRFREE( mud->routerName );
   if( mud != this_mud )
      I3UNLINK( mud, first_mud, last_mud, next, prev );
   I3DISPOSE( mud );
}

/*
 * Close the socket to the router.
 */
void I3_connection_close( bool reconnect )
{
   ROUTER_DATA *router = NULL;
   bool rfound = FALSE;

   for( router = first_router; router; router = router->next )
      if( !str_cmp( router->name, I3_ROUTER_NAME ) )
      {
         rfound = TRUE;
         break;
      }

   if( !rfound )
   {
      i3log( "%s", "I3_connection_close: Disconnecting from router." );
      close( I3_socket );
      I3_socket = -1;
      return;
   }

   i3log( "Closing connection to Intermud-3 router %s", router->name );
   close( I3_socket );
   I3_socket = -1;
   if( reconnect )
   {
      if( router->reconattempts <= 3 )
	{
	   i3wait = 100; /* Wait for 100 game loops */
	   i3log( "%s", "Will attempt to reconnect in approximately 15 seconds." );
      }
      else if( router->next != NULL )
      {
         i3log( "Unable to reach %s. Abandoning connection.", router->name );
         i3log( "Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received );
         bytes_sent = 0;
         bytes_received = 0;
         i3wait = 100;
         i3log( "Will attempt new connection to %s in approximately 15 seconds.", router->next->name );
      }
      else
      {
         bytes_sent = 0;
         bytes_received = 0;
         i3wait = -2;
         i3log( "%s", "Unable to reconnect. No routers responding." );
         return;
      }
   }
   i3log( "Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received );
   bytes_sent = 0;
   bytes_received = 0;
   return;
}

/*
 * Write a string into the send-buffer. Does not yet send it.
 */
void I3_write_buffer( const char *msg )
{
   long newsize = I3_output_pointer+strlen( msg );

   if( newsize > OPS-1 )
   {
	i3bug( "I3_write_buffer: buffer too large (would become %ld)", newsize );
	return;
   }
   i3strlcpy( I3_output_buffer + I3_output_pointer, msg, newsize );
   I3_output_pointer = newsize;
}

/* Use this function in place of I3_write_buffer ONLY if the text to be sent could 
 * contain color tags to parse into ANSI codes. Otherwise it will mess up the packet.
 */
void send_to_i3( const char *text )
{
   char buf[LGST*3];

   snprintf( buf, LGST*3, "%s", I3_tagtofish( text ) );
   I3_write_buffer( buf );
}

/*
 * Put a I3-header in the send-buffer. If a field is NULL it will
 * be replaced by a 0 (zero).
 */
void I3_write_header( char *identifier, char *originator_mudname, char *originator_username, char *target_mudname, char *target_username ) 
{
    I3_write_buffer( "({\"" );
    I3_write_buffer( identifier );
    I3_write_buffer( "\",5," );
    if( originator_mudname ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( originator_mudname );
	I3_write_buffer( "\"," );
    }
    else I3_write_buffer( "0," );

    if( originator_username )  
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( originator_username );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );

    if( target_mudname ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( target_mudname );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );

    if( target_username ) 
    {
	I3_write_buffer( "\"" );
	I3_write_buffer( target_username );
	I3_write_buffer( "\"," );
    } 
    else I3_write_buffer( "0," );
}

/*
 * Gets the next I3 field, that is when the amount of {[("'s and
 * ")]}'s match each other when a , is read. It's not foolproof, it
 * should honestly be some kind of statemachine, which does error-
 * checking. Right now I trust the I3-router to send proper packets
 * only. How naive :-) [Indeed Edwin, but I suppose we have little choice :P - Samson]
 *
 * ps will point to the beginning of the next field.
 *
 */
char *I3_get_field( char *packet, char **ps )
{
   int count[256];
   char has_apostrophe = 0, has_backslash = 0;
   char foundit = 0;

   bzero( count, sizeof(count) );

   *ps = packet;
   while( 1 ) 
   {
	switch( *ps[0] ) 
      {
	   case '{': if( !has_apostrophe ) count[(int)'{']++; break;
	   case '}': if( !has_apostrophe ) count[(int)'}']++; break;
	   case '[': if( !has_apostrophe ) count[(int)'[']++; break;
	   case ']': if( !has_apostrophe ) count[(int)']']++; break;
	   case '(': if( !has_apostrophe ) count[(int)'(']++; break;
         case ')': if( !has_apostrophe ) count[(int)')']++; break;
	   case '\\':
		if( has_backslash )
		   has_backslash = 0;
		else
		   has_backslash = 1;
		break;
	   case '"':
		if( has_backslash ) 
		{
		    has_backslash = 0;
		} 
		else 
		{
		    if( has_apostrophe )
			has_apostrophe = 0;
		    else
			has_apostrophe = 1;
		}
		break;
	    case ',':
	    case ':':
		if( has_apostrophe )
		    break;
		if( has_backslash )
		    break;
		if( count[(int)'{'] != count[(int)'}'] )
		    break;
		if( count[(int)'['] != count[(int)']'] )
		    break;
		if( count[(int)'('] != count[(int)')'] )
		    break;
		foundit = 1;
		break;
	}
	if( foundit )
	    break;
	(*ps)++;
    }
    *ps[0] = '\0';
    (*ps)++;
    return *ps;
}

/*
 * Returns a CHAR_DATA structure which matches the string
 *
 */
CHAR_DATA *I3_find_user( char *name ) 
{
    DESCRIPTOR_DATA *d;

    for ( d = first_descriptor; d; d = d->next ) 
    {
	if( d->character && !str_cmp( CH_I3NAME(d->character), name ) ) 
	   return d->character;
    }
    return NULL;
}

/* Beefed up to include wildcard ignores and user-level IP ignores.
 * Be careful when setting IP based ignores - Last resort measure, etc.
 */
bool i3ignoring( CHAR_DATA *ch, const char *ignore )
{
   I3_IGNORE *temp;
   I3_MUD *mud;
   char *ps;
   char ipbuf[512], mudname[SMST];

   /* Wildcard support thanks to Xorith */
   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
   {
	if( !fnmatch( temp->name, ignore, 0 ) )
	   return TRUE;
   }

   /* In theory, getting this far should be the result of an IP:Port ban */
   ps = strchr( ignore, '@' );

   if( ignore[0] == '\0' || ps == NULL ) 
	return FALSE;

   ps[0] = '\0';
   i3strlcpy( mudname, ps+1, SMST );

   for( mud = first_mud; mud; mud = mud->next )
   {
	if( !str_cmp( mud->name, mudname ) )
	{
	   snprintf( ipbuf, 512, "%s:%d", mud->ipaddress, mud->player_port );
	   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
	   {
		if( !str_cmp( temp->name, ipbuf ) )
		   return TRUE;
	   }
	}
   }
   return FALSE;
}

/*
 * Writes the string into the socket, prefixed by the size.
 */
bool I3_write_packet( char *msg ) 
{
   int oldsize, size, check, x;
   char *s = I3_output_buffer;

   oldsize = size = strlen( msg+4 );
   s[3] = size%256;
   size >>= 8;
   s[2] = size%256;
   size >>= 8;
   s[1] = size%256;
   size >>= 8;
   s[0] = size%256;

   /* Scan for \r used in Diku client packets and change to NULL */
   for( x = 0; x < oldsize+4; x++ )
      if( msg[x] == '\r' && x > 3 )
         msg[x] = '\0';

   check = send( I3_socket, msg, oldsize + 4, 0 );

   if( !check || ( check < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
   {
	if( check < 0 )
	   i3log( "%s", "Write error on socket." );
	else
	   i3log( "%s", "EOF encountered on socket write." );
  	I3_connection_close( TRUE );
	return FALSE;
   }

   if( check < 0 ) /* EAGAIN */
	return TRUE;

   bytes_sent += check;
   if( packetdebug )
   {
      i3log( "Size: %d. Bytes Sent: %d.", oldsize, check );
      i3log( "Packet Sent: %s", msg+4 );
   }
   I3_output_pointer = 4;
   return TRUE;
}

void I3_send_packet( void )
{
   I3_write_packet( I3_output_buffer );
   return;
}

/* The all important startup packet. This is what will be initially sent upon trying to connect
 * to the I3 router. It is therefore quite important that the information here be exactly correct.
 * If anything is wrong, your packet will be dropped by the router as invalid and your mud simply
 * won't connect to I3. DO NOT USE COLOR TAGS FOR ANY OF THIS INFORMATION!!!
 * Bugs fixed in this on 8-31-03 for improperly sent tags. Streamlined for less packet bloat.
 */
void I3_startup_packet( void ) 
{
   char s[SMST];
   char *strtime;

   if( !I3_is_connected() )
	return;

   I3_output_pointer = 4;
   I3_output_buffer[0] = '\0';

   i3log( "Sending startup_packet to %s", I3_ROUTER_NAME );

   I3_write_header( "startup-req-3", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL );
   
   snprintf( s, SMST, "%d", this_mud->password );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   snprintf( s, SMST, "%d", this_mud->mudlist_id );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   snprintf( s, SMST, "%d", this_mud->chanlist_id );
   I3_write_buffer( s );
   I3_write_buffer( "," );
   snprintf( s, SMST, "%d", this_mud->player_port );
   I3_write_buffer( s );
   I3_write_buffer( ",0,0,\"" );

   I3_write_buffer( this_mud->mudlib );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->base_mudlib );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->driver );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->mud_type );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->open_status );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( this_mud->admin_email );
   I3_write_buffer( "\"," );

   /* Begin first mapping set */
   I3_write_buffer( "([" );
   if( this_mud->emoteto )
      I3_write_buffer( "\"emoteto\":1," );
   if( this_mud->news )
      I3_write_buffer( "\"news\":1," );
   if( this_mud->ucache )
      I3_write_buffer( "\"ucache\":1," );
   if( this_mud->auth )
      I3_write_buffer( "\"auth\":1," );
   if( this_mud->locate )
      I3_write_buffer( "\"locate\":1," );
   if( this_mud->finger )
      I3_write_buffer( "\"finger\":1," );
   if( this_mud->channel )
      I3_write_buffer( "\"channel\":1," );
   if( this_mud->who )
      I3_write_buffer( "\"who\":1," );
   if( this_mud->tell )
      I3_write_buffer( "\"tell\":1," );
   if( this_mud->beep )
      I3_write_buffer( "\"beep\":1," );
   if( this_mud->mail )
      I3_write_buffer( "\"mail\":1," );
   if( this_mud->file )
      I3_write_buffer( "\"file\":1," );
   if( this_mud->http )
   {
      snprintf( s, SMST, "\"http\":%d,", this_mud->http );
      I3_write_buffer( s );
   }
   if( this_mud->smtp )
   {
      snprintf( s, SMST, "\"smtp\":%d,", this_mud->smtp );
      I3_write_buffer( s );
   }
   if( this_mud->pop3 )
   {
      snprintf( s, SMST, "\"pop3\":%d,", this_mud->pop3 );
      I3_write_buffer( s );
   }
   if( this_mud->ftp )
   {
      snprintf( s, SMST, "\"ftp\":%d,", this_mud->ftp );
      I3_write_buffer( s );
   }
   if( this_mud->nntp )
   {
      snprintf( s, SMST, "\"nntp\":%d,", this_mud->nntp );
      I3_write_buffer( s );
   }
   if( this_mud->rcp )
   {
      snprintf( s, SMST, "\"rcp\":%d,", this_mud->rcp );
      I3_write_buffer( s );
   }
   if( this_mud->amrcp )
   {
      snprintf( s, SMST, "\"amrcp\":%d,", this_mud->amrcp );
      I3_write_buffer( s );
   }
   I3_write_buffer( "]),([" );

   /* END first set of "mappings", start of second set */
   if( this_mud->web && this_mud->web[0] != '\0' )
   {
      snprintf( s, SMST, "\"url\":\"%s\",", this_mud->web );
      I3_write_buffer( s );
   }
   strtime = ctime( &current_time );
   strtime[strlen(strtime)-1] = '\0';
   snprintf( s, SMST, "\"time\":\"%s\",", strtime );
   I3_write_buffer( s );

   I3_write_buffer( "]),})\r" );
   I3_send_packet( );
}

/* This function saves the password, mudlist ID, and chanlist ID that are used by the mud.
 * The password value is returned from the I3 router upon your initial connection.
 * The mudlist and chanlist ID values are updated as needed while your mud is connected.
 * Do not modify the file it generates because doing so may prevent your mud from reconnecting
 * to the router in the future. This file will be rewritten each time the I3_shutdown function
 * is called, or any of the id values change.
 */
void I3_save_id( void )
{
   FILE *fp;

   if( ( fp = fopen( I3_PASSWORD_FILE, "w" ) ) == NULL ) 
   {
	i3log( "%s", "Couldn't write to I3 password file." );
	return;
   }

   fprintf( fp, "%s", "#PASSWORD\n" );
   fprintf( fp, "%d %d %d\n", this_mud->password, this_mud->mudlist_id, this_mud->chanlist_id );
   I3FCLOSE( fp );
}

/* The second most important packet your mud will deal with. If you never get this
 * coming back from the I3 router, something was wrong with your startup packet
 * or the router may be jammed up. Whatever the case, if you don't get a reply back
 * your mud won't be acknowledged as connected.
 */
void I3_process_startup_reply( I3_HEADER *header, char *s ) 
{
   ROUTER_DATA *router;
   I3_CHANNEL *channel;
   char *ps = s, *next_ps;

   /* Recevies the router list. Nothing much to do here until there's more than 1 router. */
   I3_get_field( ps, &next_ps );
   i3log( "%s", ps ); /* Just checking for now */
   ps = next_ps;

   /* Receives your mud's updated password, which may or may not be the same as what it sent out before */
   I3_get_field( ps, &next_ps );
   this_mud->password = atoi( ps );

   i3log( "Received startup_reply from %s", header->originator_mudname );

   I3_save_id( );

   for( router = first_router; router; router = router->next )
   {
      if( !str_cmp( router->name, header->originator_mudname ) )
      {
         router->reconattempts = 0;
         I3_ROUTER_NAME = router->name;
         break;
      }
   }
   i3wait = 0;
   i3timeout = 0;
   i3log( "%s", "Intermud-3 Network connection complete." );

   for( channel = first_I3chan; channel; channel = channel->next )
   {
      if( channel->local_name && channel->local_name[0] != '\0' )
      {
         i3log( "Subscribing to %s", channel->local_name );
         I3_send_channel_listen( channel, TRUE );
      }
   }
   return;
}

void I3_process_chanack( I3_HEADER *header, char *s )
{
   CHAR_DATA *ch;
   char *next_ps, *ps = s;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( !( ch = I3_find_user( header->target_username ) ) )
      i3log( "%s", ps );
   else
	i3_printf( ch, "{G%s\n\r", ps );
   return;
}

void I3_send_error( char *mud, char *user, char *code, char *message ) 
{
   if( !I3_is_connected() )
      return;

   I3_write_header( "error", I3_THISMUD, 0, mud, user );
   I3_write_buffer( "\"" );
   I3_write_buffer( code );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( I3_escape( message ) );
   I3_write_buffer( "\",0,})\r" );
   I3_send_packet( );
}

void I3_process_error( I3_HEADER *header, char *s ) 
{
   CHAR_DATA *ch;
   char *next_ps, *ps = s;
   char type[SMST], error[LGST];

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( type, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   /* Since VargonMUD likes to spew errors for no good reason.... */
   if( !str_cmp( header->originator_mudname, "VargonMUD" ) )
	return;

   snprintf( error, LGST, "Error: from %s to %s@%s\n\r%s: %s",
	header->originator_mudname, header->target_username, header->target_mudname, type, ps );

   if( !( ch = I3_find_user( header->target_username ) ) )
      i3log( "%s", error );
   else
	i3_printf( ch, "%s\n\r", error );
}

int i3todikugender( int gender )
{
   int sex = 0;

   if( gender == 0 )
      sex = SEX_MALE;

   if( gender == 1 )
	sex = SEX_FEMALE;

   if( gender > 1 )
	sex = SEX_NEUTRAL;

   return sex;
}

int dikutoi3gender( int gender )
{
   int sex = 0;

   if( gender > 2 || gender < 0 )
      sex = 2; /* I3 neuter */
   
   if( gender == SEX_MALE )
	sex = 0; /* I3 Male */

   if( gender == SEX_FEMALE )
	sex = 1; /* I3 Female */

   return sex;
}

/* This is very possibly going to be spammy as hell */
void I3_show_ucache_contents( CHAR_DATA *ch, char *argument )
{
   UCACHE_DATA *user;
   int users = 0;

   i3send_to_pager( "Cached user information\n\r", ch );
   i3send_to_pager( "User                          | Gender ( 0 = Male, 1 = Female, 2 = Neuter )\n\r", ch );
   i3send_to_pager( "---------------------------------------------------------------------------\n\r", ch );
   for( user = first_ucache; user; user = user->next )
   {
	i3pager_printf( ch, "%-30s %d\n\r", user->name, user->gender );
	users++;
   }
   i3pager_printf( ch, "%d users being cached.\n\r", users );
   return;
}

int I3_get_ucache_gender( char *name )
{
   UCACHE_DATA *user;

   for( user = first_ucache; user; user = user->next )
   {
	if( !str_cmp( user->name, name ) )
	   return user->gender;
   }

   /* -1 means you aren't in the list and need to be put there. */
   return -1;
}

/* Saves the ucache info to disk because it would just be spamcity otherwise */
void I3_save_ucache( void )
{
   FILE *fp;
   UCACHE_DATA *user;

   if( ( fp = fopen( I3_UCACHE_FILE, "w" ) ) == NULL ) 
   {
	i3log( "%s", "Couldn't write to I3 ucache file." );
	return;
   }

   for( user = first_ucache; user; user = user->next )
   {
	fprintf( fp, "%s", "#UCACHE\n" );
	fprintf( fp, "Name %s~\n", user->name );
	fprintf( fp, "Sex  %d\n", user->gender );
	fprintf( fp, "Time %ld\n", (long int)user->time );
	fprintf( fp, "%s", "End\n\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   I3FCLOSE( fp );
   return;
}

void I3_prune_ucache( void )
{
   UCACHE_DATA *ucache, *next_ucache;

   for( ucache = first_ucache; ucache; ucache = next_ucache )
   {
	next_ucache = ucache->next;

	/* Info older than 30 days is removed since this person likely hasn't logged in at all */
	if( current_time - ucache->time >= 2592000 )
	{
	   I3STRFREE( ucache->name );
	   I3UNLINK( ucache, first_ucache, last_ucache, next, prev );
	   I3DISPOSE( ucache );
	}
   }
   I3_save_ucache( );
   return;
}

/* Updates user info if they exist, adds them if they don't. */
void I3_ucache_update( char *name, int gender )
{
   UCACHE_DATA *user;

   for( user = first_ucache; user; user = user->next )
   {
      if( !str_cmp( user->name, name ) )
	{
	   user->gender = gender;
	   user->time = current_time;
	   return;
	}
   }
   I3CREATE( user, UCACHE_DATA, 1 );
   user->name = I3STRALLOC( name );
   user->gender = gender;
   user->time = current_time;
   I3LINK( user, first_ucache, last_ucache, next, prev );

   I3_save_ucache( );
   return;
}

void I3_send_ucache_update( char *visname, int gender )
{
   char buf[10];

   if( !I3_is_connected() )
	return;

   I3_write_header( "ucache-update", I3_THISMUD, NULL, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( visname );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( visname );
   I3_write_buffer( "\"," );
   snprintf( buf, 10, "%d", gender );
   I3_write_buffer( buf );
   I3_write_buffer( ",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_ucache_update( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   char username[SMST], visname[SMST], buf[LGST];
   int sex, gender;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( username, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( visname, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   gender = atoi( ps );

   snprintf( buf, LGST, "%s@%s", visname, header->originator_mudname );

   sex = I3_get_ucache_gender( buf );

   if( sex == gender )
      return;

   I3_ucache_update( buf, gender );
   return;
}

void I3_send_chan_user_req( char *targetmud, char *targetuser )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "chan-user-req", I3_THISMUD, NULL, targetmud, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( targetuser );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_chan_user_req( I3_HEADER *header, char *s )
{
   char buf[LGST];
   char *ps = s, *next_ps;
   CHAR_DATA *ch;
   int sex, gender;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( buf, LGST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( ps ) ) )
   {
	if( !i3exists_player( ps ) )
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3PERM(ch) < I3PERM_MORT )
   {
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	return;
   }

   if( I3ISINVIS(ch) || i3ignoring( ch, buf ) )
   {
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   /* Since this is a gender thing, we need to gather that info. It's assumed anything above 2
    * is generally referred to as an "it" anyway, so send them a neuter if it's above 2.
    * And I3 genders are in a different order from standard Diku genders, so this complicates matters some.
    */
   gender = CH_I3SEX(ch);

   sex = dikutoi3gender( gender );

   I3_write_header( "chan-user-reply", I3_THISMUD, NULL, header->originator_mudname, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_I3NAME(ch) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( CH_I3NAME(ch) );
   I3_write_buffer( "\"," );
   snprintf( buf, LGST, "%d", sex );
   I3_write_buffer( buf );
   I3_write_buffer( ",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_chan_user_reply( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   char username[SMST], visname[SMST], buf[LGST];
   int sex, gender;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( username, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( visname, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   gender = atoi( ps );

   snprintf( buf, LGST, "%s@%s", visname, header->originator_mudname );

   sex = I3_get_ucache_gender( buf );

   if( sex == gender )
      return;

   I3_ucache_update( buf, gender );
   return;
}

void I3_process_mudlist( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   I3_MUD *mud = NULL;
   char mud_name[SMST];

   I3_get_field( ps, &next_ps );
   this_mud->mudlist_id = atoi( ps );
   I3_save_id( );

   ps = next_ps;
   ps += 2;

   while( 1 ) 
   {
	char *next_ps2;
	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
      i3strlcpy( mud_name, ps, SMST );

	ps = next_ps;
	I3_get_field( ps, &next_ps2 );

	if( ps[0] != '0' ) 
	{
	   mud = find_I3_mud_by_name( mud_name );
         if( !mud )
            mud = new_I3_mud( mud_name );

	   ps += 2;
	   I3_get_field( ps, &next_ps );
	   mud->status = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->ipaddress );
	   mud->ipaddress = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->player_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->imud_tcp_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   mud->imud_udp_port = atoi( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->mudlib );
	   mud->mudlib = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->base_mudlib );
	   mud->base_mudlib = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->driver );
	   mud->driver = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->mud_type );
	   mud->mud_type = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->open_status );
	   mud->open_status = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
	   I3STRFREE( mud->admin_email );
	   mud->admin_email = I3STRALLOC( ps );
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );

	   ps += 2;
	   while( 1 ) 
	   {
		char *next_ps3;
		char key[SMST];

		if( ps[0] == ']' )
		    break;

		I3_get_field( ps, &next_ps3 );
		I3_remove_quotes( &ps );
		i3strlcpy( key, ps, SMST );
		ps = next_ps3;
		I3_get_field( ps, &next_ps3 );

		switch( key[0] ) 
		{
		case 'a': 
		    if( !str_cmp( key, "auth" ) )
		    {
			mud->auth = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "amrcp" ) )
		    {
			mud->amrcp = atoi( ps );
			break;
		    }
		    break;
		case 'b':
		    if( !str_cmp( key, "beep" ) )
		    {
			mud->beep = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'c': 
		    if( !str_cmp( key, "channel" ) )
		    {
			mud->channel = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		case 'e': 
		    if( !str_cmp( key, "emoteto" ) )
		    {
			mud->emoteto = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'f': 
		    if( !str_cmp( key, "file" ) )
		    {
			mud->file = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "finger" ) )
		    {
			mud->finger = ps[0] == '0' ? 0 : 1;
			break;
                }
		    if( !str_cmp( key, "ftp" ) ) 
		    {
			mud->ftp = atoi( ps );
			break;
		    }
		    break;
		case 'h': 
		    if( !str_cmp( key, "http" ) )
		    {
			mud->http = atoi( ps );
			break;
                }
		    break;
		case 'l': 
		    if( !str_cmp( key, "locate" ) )
		    {
			mud->locate = ps[0] == '0' ? 0 : 1;
			break;
 		    }
		    break;
		case 'm':
		    if( !str_cmp( key, "mail" ) )
		    {
			mud->mail = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    break;
		case 'n': 
		    if( !str_cmp( key, "news" ) )
		    {
			mud->news = ps[0] == '0' ? 0 : 1; 
			break;
		    }
		    if( !str_cmp( key, "nntp" ) )
		    {
			mud->nntp = atoi( ps );
			break;
                }
		    break;
		case 'p': 
		    if( !str_cmp( key, "pop3" ) )
		    {
			mud->pop3 = atoi( ps );
			break;
		    }
		    break;
		case 'r': 
		    if( !str_cmp( key, "rcp" ) )
		    {
			mud->rcp = atoi( ps );
			break;
		    }
		    break;
		case 's': 
		    if( !str_cmp( key, "smtp" ) )
		    {
			mud->smtp = atoi( ps );
			break;
		    }
		    break;
		case 't': 
		    if( !str_cmp( key, "tell" ) )
		    {
			mud->tell = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		case 'u': 
		    if( !str_cmp( key, "ucache" ) )
		    {
			mud->ucache = ps[0] == '0' ? 0 : 1;
			break;
		    }
		    if( !str_cmp( key, "url" ) )
		    {
			I3_remove_quotes( &ps );
			I3STRFREE( mud->web_wrong );
			mud->web_wrong = I3STRALLOC( ps );
			break;
		    }
		    break;
		case 'w':
		    if( !str_cmp( key, "who" ) )
		    {
			mud->who = ps[0] == '0' ? 0 : 1;
			break;
                }
		    break;
		default:
		    break;
		}

		ps = next_ps3;
		if( ps[0] == ']' )
		    break;
	   }
	   ps = next_ps;

	   I3_get_field( ps, &next_ps );
	   ps = next_ps;

	}
      else
      {
         if( ( mud = find_I3_mud_by_name( mud_name ) ) != NULL )
            destroy_I3_mud( mud );
      }
	ps = next_ps2;
	if( ps[0] == ']' )
	   break;
   }
   return;
}

void I3_process_chanlist_reply( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   I3_CHANNEL *channel;
   char chan[SMST];

   I3_get_field( ps, &next_ps );
   this_mud->chanlist_id = atoi( ps );
   I3_save_id( );

   ps = next_ps;
   ps += 2;

   while( 1 ) 
   {
	char *next_ps2;

	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
      i3strlcpy( chan, ps, SMST );

	ps = next_ps;
	I3_get_field( ps, &next_ps2 );
	if( ps[0] != '0' ) 
	{
	   if( !( channel = find_I3_channel_by_name( chan ) ) )
         {
            channel = new_I3_channel();
            channel->I3_name = I3STRALLOC( chan );
         }

	   ps += 2;
	   I3_get_field( ps, &next_ps );
	   I3_remove_quotes( &ps );
         I3STRFREE( channel->host_mud );
	   channel->host_mud = I3STRALLOC( ps );
	   ps = next_ps;
	   I3_get_field( ps, &next_ps );
	   channel->status = atoi( ps );
	}
      else
      {
         if( ( channel = find_I3_channel_by_name( chan ) ) != NULL )
         {
            if( channel->local_name && channel->local_name[0] != '\0' )
               i3log( "Locally configured channel %s has been purged from router %s", channel->local_name, I3_ROUTER_NAME );
            destroy_I3_channel( channel );
            I3_write_channel_config();
         }
      }
	ps = next_ps2;
	if( ps[0] == ']' )
	   break;
   }
   return;
}

void I3_send_channel_message( I3_CHANNEL *channel, char *name, char *message ) 
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-m", I3_THISMUD, name, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( name );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( message ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_send_channel_emote( I3_CHANNEL *channel, char *name, char *message ) 
{
   char buf[LGST];

   if( !I3_is_connected() )
	return;

   if( strstr( message, "$N" ) == NULL )
	snprintf( buf, LGST, "$N %s", message );
   else
	i3strlcpy( buf, message, LGST );

   I3_write_header( "channel-e", I3_THISMUD, name, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( name );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( buf ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_send_channel_t( I3_CHANNEL *channel, char *name, char *tmud, char *tuser, char *msg_o, char *msg_t, char *tvis )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-t", I3_THISMUD, name, NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tmud );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tuser );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( msg_o ) );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( msg_t ) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( name );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( tvis );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

int I3_token( char type, char *string, char *oname, char *tname )
{
   char code[50];
   char *p = '\0';

   switch( type )
   {
      default:
         code[0] = type;
	   code[1] = '\0';
         return 1;
	case '$':
	   i3strlcpy( code, "$", 50 );
	   break;
      case ' ':
         i3strlcpy( code, " ", 50 );
         break;
      case 'N': /* Originator's name */
         i3strlcpy( code, oname, 50 );
         break;
      case 'O': /* Target's name */
         i3strlcpy( code, tname, 50 );
         break;
   }
   p = code;
   while( *p != '\0' )
   {
      *string = *p++;
      *++string = '\0';
   }
   return( strlen( code ) );
}

void I3_message_convert( char *buffer, const char *txt, char *oname, char *tname )
{
   const char *point;
   int skip = 0;

   for( point = txt ; *point ; point++ )
   {
      if( *point == '$' )
      {
         point++;
         if( *point == '\0' )
            point--;
         else
            skip = I3_token( *point, buffer, oname, tname );
         while( skip-- > 0 )
            ++buffer;
         continue;
      }
      *buffer = *point;
      *++buffer = '\0';
   }                   
   *buffer = '\0';
   return;
}

char *I3_convert_channel_message( const char *message, char *sname, char *tname )
{
   static char msgbuf[LGST];

   /* Sanity checks - if any of these are NULL, bad things will happen - Samson 6-29-01 */
   if( !message )
   {
	i3bug( "%s", "I3_convert_channel_message: NULL message!" );
	return "ERROR";
   }

   if( !sname )
   {
	i3bug( "%s", "I3_convert_channel_message: NULL sname!" );
	return "ERROR";
   }

   if( !tname )
   {
	i3bug( "%s", "I3_convert_channel_message: NULL tname!" );
	return "ERROR";
   }

   I3_message_convert( msgbuf, message, sname, tname );
   return msgbuf;
}

void update_chanhistory( I3_CHANNEL *channel, char *message )
{
   char msg[LGST], buf[LGST];
   struct tm *local;
   time_t t;
   int x;

   if( !channel )
   {
	i3bug( "%s", "update_chanhistory: NULL channel received!" );
	return;
   }

   if( !message || message[0] == '\0' )
   {
	i3bug( "%s", "update_chanhistory: NULL message received!" );
	return;
   }

   i3strlcpy( msg, message, LGST );
   for( x = 0; x < MAX_I3HISTORY; x++ )
   {
      if( channel->history[x] == NULL )
      {
         t = time( NULL );
         local = localtime( &t );
         snprintf( buf, LGST, "{R[%-2.2d/%-2.2d %-2.2d:%-2.2d] {G%s\n\r",
		local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, msg );
         channel->history[x] = I3STRALLOC( buf );

	   if( IS_SET( channel->flags, I3CHAN_LOG ) )
   	   {
      	FILE *fp;
      	snprintf( buf, LGST, "../i3/%s.log", channel->local_name );
      	if( !( fp = fopen( buf, "a" ) ) )
      	{
	   	   perror( buf );
	   	   i3bug( "Could not open file %s!", buf );
      	}
      	else
      	{
	   	   fprintf( fp, "%s\n", channel->history[x] );
	   	   I3FCLOSE( fp );
      	}
   	   }
         break;
      }

      if( x == MAX_I3HISTORY - 1 )
      {
         int y;

         for( y = 1; y < MAX_I3HISTORY; y++ )
         {
            int z = y-1;

            if( channel->history[z] != NULL )
            {
               I3STRFREE( channel->history[z] );
               channel->history[z] = I3STRALLOC( channel->history[y] );
            }
         }

         t = time( NULL );
         local = localtime( &t );
         snprintf( buf, LGST, "{R[%-2.2d/%-2.2d %-2.2d:%-2.2d] {G%s\n\r",
		local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, msg );
	   I3STRFREE( channel->history[x] );
         channel->history[x] = I3STRALLOC( buf );

	   if( IS_SET( channel->flags, I3CHAN_LOG ) )
   	   {
      	FILE *fp;
      	snprintf( buf, LGST, "../i3/%s.log", channel->local_name );
      	if( !( fp = fopen( buf, "a" ) ) )
      	{
	   	   perror( buf );
	   	   i3bug( "Could not open file %s!", buf );
      	}
      	else
      	{
	   	   fprintf( fp, "%s\n", channel->history[x] );
	   	   I3FCLOSE( fp );
      	}
   	   }
      }
   }
   return;
}

void I3_process_channel_t( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *vch = NULL;
   char targetmud[SMST], targetuser[SMST], message_o[LGST], message_t[LGST], buf[LGST];
   char visname_o[SMST], sname[SMST], tname[SMST], lname[SMST], tmsg[LGST], omsg[LGST];
   I3_CHANNEL *channel = NULL;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( !( channel = find_I3_channel_by_name( ps ) ) ) 
   {
	i3log( "I3_process_channel_t: received unknown channel (%s)", ps );
	return;
   }

   if( !channel->local_name )
	return;

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( targetmud, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( targetuser, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( message_o, ps, LGST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( message_t, ps, LGST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( visname_o, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( sname, SMST, "%s@%s", visname_o, header->originator_mudname );
   snprintf( tname, SMST, "%s@%s", ps, targetmud );

   snprintf( omsg, LGST, "%s", I3_convert_channel_message( message_o, sname, tname ) );
   snprintf( tmsg, LGST, "%s", I3_convert_channel_message( message_t, sname, tname ) );

   for( d = first_descriptor; d; d = d->next ) 
   {
	if( !d->character )
	   continue;

	vch = d->original ? d->original : d->character;

	if( !I3_hasname( I3LISTEN(vch), channel->local_name ) || I3_hasname( I3DENY(vch), channel->local_name ) )
	   continue;

	snprintf( lname, SMST, "%s@%s", CH_I3NAME(vch), I3_THISMUD );
 
	if( d->connected == CON_PLAYING && !i3ignoring( vch, sname ) )
	{
	   if( !str_cmp( lname, tname ) )
	   {
            sprintf( buf, channel->layout_e, channel->local_name, tmsg );
		i3_printf( vch, "%s\n\r", buf );
	   }
	   else
	   {
            sprintf( buf, channel->layout_e, channel->local_name, omsg );
		i3_printf( vch, "%s\n\r", buf );
	   }
	}
   }
   update_chanhistory( channel, omsg );
   return;
}

void I3_process_channel_m( I3_HEADER *header, char *s ) 
{
   char *ps = s, *next_ps;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *vch = NULL;
   char visname[SMST], buf[LGST];
   I3_CHANNEL *channel;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( !( channel = find_I3_channel_by_name( ps ) ) )
   {
	i3log( "channel_m: received unknown channel (%s)", ps );
	return;
   }

   if( !channel->local_name )
	return;

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( visname, ps, SMST );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( buf, LGST, channel->layout_m, channel->local_name, visname, header->originator_mudname, ps );
   for( d = first_descriptor; d; d = d->next )
   {
	if( !d->character )
	   continue;

      vch = d->original ? d->original : d->character;

	if( !I3_hasname( I3LISTEN(vch), channel->local_name ) || I3_hasname( I3DENY(vch), channel->local_name ) )
	   continue;

	if( d->connected == CON_PLAYING && !i3ignoring( vch, visname ) )
	   i3_printf( vch, "%s\n\r", buf );
   }
   update_chanhistory( channel, buf );
   return;
}

void I3_process_channel_e( I3_HEADER *header, char *s ) 
{
   char *ps = s, *next_ps;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *vch = NULL;
   char visname[SMST], msg[LGST], buf[LGST];
   I3_CHANNEL *channel;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   if( !( channel = find_I3_channel_by_name( ps ) ) )
   {
	i3log( "channel_e: received unknown channel (%s)", ps );
	return;
   }

   if( !channel->local_name )
	return;

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   snprintf( visname, SMST, "%s@%s", ps, header->originator_mudname );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
 
   snprintf( msg, LGST, "%s", I3_convert_channel_message( ps, visname, visname ) );
   snprintf( buf, LGST, channel->layout_e, channel->local_name, msg );

   for( d = first_descriptor; d; d = d->next )
   {
	if( !d->character )
	   continue;

      vch = d->original ? d->original : d->character;

	if( !I3_hasname( I3LISTEN(vch), channel->local_name ) || I3_hasname( I3DENY(vch), channel->local_name ) )
	   continue;

	if( d->connected == CON_PLAYING && !i3ignoring( vch, visname ) )
	   i3_printf( vch, "%s\n\r", buf );
   }
   update_chanhistory( channel, buf );
   return;
}

void I3_process_chan_who_req( I3_HEADER *header, char *s )
{
   DESCRIPTOR_DATA *d;
   char *ps = s, *next_ps;
   char buf[LGST], ibuf[SMST];
   I3_CHANNEL *channel;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( ibuf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( channel = find_I3_channel_by_name( ps ) ) )
   {
	snprintf( buf, LGST, "The channel you specified (%s) is unknown at %s", ps, I3_THISMUD );
	I3_send_error( header->originator_mudname, header->originator_username, "unk-channel", buf );
	i3log( "chan_who_req: received unknown channel (%s)", ps );
	return;
   }

   if( !channel->local_name ) 
   {
	snprintf( buf, LGST, "The channel you specified (%s) is not registered at %s", ps, I3_THISMUD );
	I3_send_error( header->originator_mudname, header->originator_username, "unk-channel", buf );
	return;
   }

   I3_write_header( "chan-who-reply", I3_THISMUD, NULL, header->originator_mudname, header->originator_username );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",({" );

   for( d = first_descriptor; d; d = d->next )
   {
	if( !d->character )
	   continue;

      if( I3ISINVIS(d->character) )
	   continue;

	if( I3_hasname( I3LISTEN(d->character), channel->local_name ) && !i3ignoring( d->character, ibuf )
	 && !I3_hasname( I3DENY(d->character), channel->local_name ) )
	{
	   I3_write_buffer( "\"" );
	   I3_write_buffer( CH_I3NAME(d->character) );
	   I3_write_buffer( "\"," );
	}
   }
   I3_write_buffer( "}),})\r" );
   I3_send_packet( );
   return;
}

void I3_process_chan_who_reply( I3_HEADER *header, char *s ) 
{
    char *ps = s, *next_ps;
    CHAR_DATA *ch;

   if( !( ch = I3_find_user( header->target_username ) ) )
    {
	i3bug( "I3_process_chan_who_reply(): user %s not found.", header->target_username );
	return;
    }

    I3_get_field( ps, &next_ps );
    I3_remove_quotes( &ps );
   i3_printf( ch, "{WUsers listening to %s on %s:\n\r\n\r", ps, header->originator_mudname );

    ps = next_ps;
    I3_get_field( ps, &next_ps );
    ps += 2;
    while( 1 ) 
    {
	if( ps[0] == '}' ) 
      {
	   i3_to_char( "{cNo information returned or no people listening.\n\r", ch );
	    return;
	}
	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
	i3_printf( ch, "{c%s\n\r", ps );

	ps = next_ps;
	if( ps[0]== '}' )
	    break;
    }

    return;
}

void I3_send_chan_who( CHAR_DATA *ch, I3_CHANNEL *channel, I3_MUD *mud ) 
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "chan-who-req", I3_THISMUD, CH_I3NAME(ch), mud->name, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_send_beep( CHAR_DATA *ch, char *to, I3_MUD *mud )
{
   if( !I3_is_connected() )
	return;

   I3_escape( to );
   I3_write_header( "beep", I3_THISMUD, CH_I3NAME(ch), mud->name, to );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_I3NAME(ch) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_beep( I3_HEADER *header, char *s ) 
{
   char buf[SMST];
   char *ps = s, *next_ps;
   CHAR_DATA *ch;

   snprintf( buf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( header->target_username ) ) )
   {
	if( !i3exists_player( header->target_username ) )
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3PERM(ch) < I3PERM_MORT )
   {
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	return;
   }

   if( I3ISINVIS(ch) || i3ignoring( ch, buf ) )
   {
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_BEEP ) )
   {
	snprintf( buf, SMST, "%s is not accepting beeps.", CH_I3NAME(ch) );
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", buf );
	return;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   i3_printf( ch, "{Y\a%s@%s beeps you.\n\r", ps, header->originator_mudname );
   return;
}

void I3_beep( CHAR_DATA *ch, char *argument )
{
   char *ps;
   char mud[SMST];
   I3_MUD *pmud;

   if( I3IS_SET( I3FLAG(ch), I3_DENYBEEP ) )
   {
	i3_to_char( "You are not allowed to use i3beeps.\n\r", ch );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3beep user@mud\n\r", ch );
	i3_to_char( "Usage: i3beep [on]/[off]\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "on" ) )
   {
	I3REMOVE_BIT( I3FLAG(ch), I3_BEEP );
	i3_to_char( "You now send and receive i3beeps.\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "off" ) )
   {
	I3SET_BIT( I3FLAG(ch), I3_BEEP );
	i3_to_char( "You no longer send and receive i3beeps.\n\r", ch );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_BEEP ) )
   {
	i3_to_char( "Your i3beeps are turned off.\n\r", ch );
	return;
   }

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are invisible.\n\r", ch );
	return;
   }

   ps = strchr( argument, '@' );

   if( !argument || argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "{YYou should specify a person@mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   ps[0] = '\0';
   ps++;
   i3strlcpy( mud, ps, SMST );

   if( !( pmud = find_I3_mud_by_name( mud ) ) ) 
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !str_cmp( I3_THISMUD, pmud->name ) )
   {
	i3_to_char( "Use your mud's own internal system for that.\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->beep == 0 )
	i3_printf( ch, "%s does not support the 'beep' command. Sending anyway.\n\r", pmud->name );

   I3_send_beep( ch, argument, pmud );
   i3_printf( ch, "{YYou beep %s@%s.\n\r", i3capitalize( argument ), pmud->name );
}

void I3_send_tell( CHAR_DATA *ch, char *to, I3_MUD *mud, char *message )
{
   if( !I3_is_connected() )
	return;

   I3_escape( to );
   I3_write_header( "tell", I3_THISMUD, CH_I3NAME(ch), mud->name, to );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_I3NAME(ch) );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( message ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_tell( I3_HEADER *header, char *s ) 
{
   char buf[SMST];
   char *ps = s, *next_ps;
   CHAR_DATA *ch;

   snprintf( buf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( header->target_username ) ) )
   {
	if( !i3exists_player( header->target_username ) )
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3PERM(ch) < I3PERM_MORT )
   {
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	return;
   }

   if( I3ISINVIS(ch) || i3ignoring( ch, buf ) )
   {
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_TELL ) )
   {
	snprintf( buf, SMST, "%s is not accepting tells.", CH_I3NAME(ch) );
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", buf );
	return;
   }

   if( CH_I3AFK(ch) )
   {
	snprintf( buf, SMST, "%s is currently AFK. Try back later.", CH_I3NAME(ch) );
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", buf );
	return;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3_printf( ch, "{Y%s@%s i3tells you: ", ps, header->originator_mudname );

   snprintf( buf, SMST, "'%s@%s'", ps, header->originator_mudname );
   
   I3STRFREE( I3REPLY(ch) );
   I3REPLY(ch) = I3STRALLOC( buf );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3_printf( ch, "{c%s\n\r", ps );
   return;
}

void I3_tell( CHAR_DATA *ch, char *argument )
{
   char to[SMST], *ps;
   char mud[SMST];
   I3_MUD *pmud;

   if( I3IS_SET( I3FLAG(ch), I3_DENYTELL ) )
   {
	i3_to_char( "You are not allowed to use i3tells.\n\r", ch );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3tell <user@mud> <message>\n\r", ch );
	i3_to_char( "Usage: i3tell [on]/[off]\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "on" ) )
   {
	I3REMOVE_BIT( I3FLAG(ch), I3_TELL );
	i3_to_char( "You now send and receive i3tells.\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "off" ) )
   {
	I3SET_BIT( I3FLAG(ch), I3_TELL );
	i3_to_char( "You no longer send and receive i3tells.\n\r", ch );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_TELL ) )
   {
	i3_to_char( "Your i3tells are turned off.\n\r", ch );
	return;
   }

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are invisible.\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, to );
   ps = strchr( to, '@' );

   if( to[0] == '\0' || argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = '\0';
   ps++;
   i3strlcpy( mud, ps, SMST );

   if( !( pmud = find_I3_mud_by_name( mud ) ) )
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !str_cmp( I3_THISMUD, pmud->name ) )
   {
	i3_to_char( "Use your mud's own internal system for that.\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->tell == 0 )
   {
	i3_printf( ch, "%s does not support the 'tell' command.\n\r", pmud->name );
      return;
   }

   I3_send_tell( ch, to, pmud, argument );
   i3_printf( ch, "{YYou i3tell %s@%s: {c%s\n\r", i3capitalize( to ), pmud->name, argument );
}

void I3_reply( CHAR_DATA *ch, char *argument )
{
   char buf[LGST];

   if( I3IS_SET( I3FLAG(ch), I3_DENYTELL ) )
   {
	i3_to_char( "You are not allowed to use i3tells.\n\r", ch );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3reply <message>\n\r", ch );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_TELL ) )
   {
	i3_to_char( "Your i3tells are turned off.\n\r", ch );
	return;
   }

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are invisible.\n\r", ch );
	return;
   }

   if( !I3REPLY(ch) )
   {
	i3_to_char( "You have not yet received an i3tell?!?\n\r", ch );
	return;
   }

   snprintf( buf, LGST, "%s %s", I3REPLY(ch), argument );
   I3_tell( ch, buf );
   return;
}

void I3_send_who( CHAR_DATA *ch, char *mud ) 
{
   if( !I3_is_connected() )
	return;

   I3_escape( mud );
   I3_write_header( "who-req", I3_THISMUD, CH_I3NAME(ch), mud, NULL );
   I3_write_buffer( "})\r" );
   I3_send_packet( );

   return;
}

/* Color align functions by Justice@Aaern */
int i3const_color_str_len( const char *argument )
{
    int  str, count = 0;
    bool IS_COLOR = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
        if ( argument[str] == '{' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else
        {
            if ( IS_COLOR == FALSE ) count++;
            else IS_COLOR = FALSE;
        }
    }

    return count;
}

int i3const_color_strnlen( const char *argument, int maxlength )
{
    int str, count = 0;
    bool IS_COLOR = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
        if ( argument[str] == '{' )
        {
            if ( IS_COLOR == TRUE )
            {
                count++;
                IS_COLOR = FALSE;
            }
            else
                IS_COLOR = TRUE;
        }
        else
        {
            if ( IS_COLOR == FALSE ) count++;
            else IS_COLOR = FALSE;
        }

	if ( count >= maxlength ) break;
    }
    if ( count < maxlength ) return ((str - count) + maxlength);

    str++;
    return str;
}

const char *i3const_color_align( const char *argument, int size, int align )
{
    int space = ( size - i3const_color_str_len( argument ) );
    static char buf[LGST];

    if ( align == ALIGN_RIGHT || i3const_color_str_len( argument ) >= size )
        sprintf( buf, "%*.*s", i3const_color_strnlen( argument, size ),
            i3const_color_strnlen( argument, size ), argument );
    else if ( align == ALIGN_CENTER )
        sprintf( buf, "%*s%s%*s", ( space/2 ),"",argument,
            ((space/2)*2)==space ? (space/2) : ((space/2)+1),"" );
    else
        sprintf( buf, "%s%*s", argument, space, "" );

    return buf;
}

/* You can customize the output of this - to a point. Because of how the I3 packets are formatted
 * you need to send even the listing header as a person+info packet. It should be fairly obvious
 * how to change this around if you really want to. Use the bogusidle variable for the idle time
 * on a divider, like what I've done here for headerbuf. If you wish to subvert this with your
 * own custom who list, add a #define I3CUSTOMWHO to one of your H files, and make your own
 * I3_process_who_req function somewhere else in your code.
 */
#ifndef I3CUSTOMWHO
void I3_process_who_req( I3_HEADER *header, char *s ) 
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *person;
   char ibuf[SMST], headerbuf[SMST], personbuf[LGST], tailbuf[LGST];
   char smallbuf[50], buf[300], outbuf[400], stats[200], rank[200];
   int pcount = 0, amount, xx, yy;
   long int bogusidle = 9999;
       
   snprintf( ibuf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   I3_write_header( "who-reply", I3_THISMUD, NULL, header->originator_mudname, header->originator_username );
   I3_write_buffer( "({" );

   I3_write_buffer( "({\"" );
   {
      outbuf[0] = '\0';

      snprintf( buf, 300, "{R-=[ {WPlayers on %s {R]=-", I3_THISMUD );           
      amount = 78 - I3_strlen_color( buf ); /* Determine amount to put in front of line */

      if( amount < 1 )
         amount = 1;

      amount = amount / 2;

      for( xx = 0 ; xx < amount ; xx++ )
         i3strlcat( outbuf, " ", 400 );

      i3strlcat( outbuf, buf, 400 );
      send_to_i3( I3_escape( outbuf ) );
   }

   I3_write_buffer( "\"," );
   snprintf( smallbuf, 50, "%ld", -1l );
   I3_write_buffer( smallbuf );

   I3_write_buffer( ",\" \",}),({\"" );
   {
      outbuf[0] = '\0';

      snprintf( buf, 300, "{Y-=[ {Wtelnet://%s:%d {Y]=-", this_mud->telnet, this_mud->player_port );           
      amount = 78 - I3_strlen_color( buf ); /* Determine amount to put in front of line */

      if( amount < 1 )
         amount = 1;

      amount = amount / 2;

      for( xx = 0 ; xx < amount ; xx++ )
         i3strlcat( outbuf, " ", 400 );

      i3strlcat( outbuf, buf, 400 );
      send_to_i3( I3_escape( outbuf ) );
   }

   I3_write_buffer( "\"," );
   snprintf( smallbuf, 50, "%ld", bogusidle );
   I3_write_buffer( smallbuf );

   I3_write_buffer( ",\" \",})," );

   xx = 0;
   for( d = first_descriptor; d; d = d->next )
   {
	if( d->character && d->connected == CON_PLAYING )
	{
	   if( I3PERM(d->character) < I3PERM_MORT || I3PERM(d->character) >= I3PERM_IMM 
          || I3ISINVIS(d->character) || i3ignoring( d->character, ibuf ) )
	      continue;

         xx++;
	}
   }

   if( xx > 0 )
   {
      I3_write_buffer( "({\"" );
      i3strlcpy( headerbuf, "{B--------------------------------=[ {WPlayers {B]=---------------------------------", SMST );
      send_to_i3( I3_escape( headerbuf ) );
      I3_write_buffer( "\"," );
      snprintf( smallbuf, 50, "%ld", bogusidle );
      I3_write_buffer( smallbuf );
      I3_write_buffer( ",\" \",})," );

      /* This section is displaying only players - not imms */
      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
	      if( I3PERM(d->character) < I3PERM_MORT || I3PERM(d->character) >= I3PERM_IMM 
             || I3ISINVIS(d->character) || i3ignoring( d->character, ibuf ) )
	         continue;

            person = d->character;
            pcount++;

	      I3_write_buffer( "({\"" );

	      snprintf( rank, 200, "%s", rankbuffer( person ) );
		snprintf( outbuf, 400, "%s", i3const_color_align( rank, 20, ALIGN_CENTER ) );
            send_to_i3( I3_escape( outbuf ) );

	      I3_write_buffer( "\"," );
	      snprintf( smallbuf, 50, "%ld", -1l );
	      I3_write_buffer( smallbuf );
	      I3_write_buffer( ",\"" );
	    
            i3strlcpy( stats, "{D[", 200 );
            if( CH_I3AFK(person) )
               i3strlcat( stats, "AFK", 200 );
            else
               i3strlcat( stats, "---", 200 );
            i3strlcat( stats, "]{G", 200 );

	      snprintf( personbuf, LGST, "%s %s%s", stats, CH_I3NAME(person), CH_I3TITLE(person) );
	      send_to_i3( I3_escape( personbuf ) );
    	      I3_write_buffer( "\",})," );
	   }
      }
   }

   yy = 0;
   for( d = first_descriptor; d; d = d->next )
   {
	if( d->character && d->connected == CON_PLAYING )
	{
	   if( I3PERM(d->character) < I3PERM_IMM || I3ISINVIS(d->character) || i3ignoring( d->character, ibuf ) )
	      continue;

         yy++;
	}
   }

   if( yy > 0 )
   {
      I3_write_buffer( "({\"" );
      i3strlcpy( headerbuf, "{R-------------------------------=[ {WImmortals {R]=--------------------------------", SMST );
      send_to_i3( I3_escape( headerbuf ) );
      I3_write_buffer( "\"," );
	if( xx > 0 )
	   snprintf( smallbuf, 50, "%ld", bogusidle * 3 );
	else
         snprintf( smallbuf, 50, "%ld", bogusidle );
      I3_write_buffer( smallbuf );
      I3_write_buffer( ",\" \",})," );

      /* This section is displaying only immortals, not players */
      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
	      if( I3PERM(d->character) < I3PERM_IMM || I3ISINVIS(d->character) || i3ignoring( d->character, ibuf ) )
	         continue;

            person = d->character;
            pcount++;

	      I3_write_buffer( "({\"" );

	      snprintf( rank, 200, "%s", rankbuffer( person ) );
		snprintf( outbuf, 400, "%s", i3const_color_align( rank, 20, ALIGN_CENTER ) );
            send_to_i3( I3_escape( outbuf ) );

	      I3_write_buffer( "\"," );
	      snprintf( smallbuf, 50, "%ld", -1l );
	      I3_write_buffer( smallbuf );
	      I3_write_buffer( ",\"" );
	    
            i3strlcpy( stats, "{D[", 200 );
            if( CH_I3AFK(person) )
               i3strlcat( stats, "AFK", 200 );
            else
               i3strlcat( stats, "---", 200 );
            i3strlcat( stats, "]{G", 200 );

	      snprintf( personbuf, LGST, "%s %s%s", stats, CH_I3NAME(person), CH_I3TITLE(person) );
	      send_to_i3( I3_escape( personbuf ) );
    	      I3_write_buffer( "\",})," );
	   }
      }
   }

   I3_write_buffer( "({\"" );
   snprintf( tailbuf, LGST, "{Y[{W%d Player%s{Y]", pcount, pcount == 1 ? "" : "s" );
   send_to_i3( I3_escape( tailbuf ) );
   I3_write_buffer( "\"," );
   snprintf( smallbuf, 50, "%ld", bogusidle * 2 );
   I3_write_buffer( smallbuf );
   I3_write_buffer( ",\"" );
   snprintf( tailbuf, LGST, "{Y[{WHomepage: %s{Y] [{W%3d Max Since Reboot{Y]", this_mud->web, I3MAXPLAYERS );
   send_to_i3( I3_escape( tailbuf ) );
   I3_write_buffer( "\",}),}),})\r" );
   I3_send_packet( );

   return;
}
#endif

/* This is where the incoming results of a who-reply packet are processed.
 * Note that rather than just spit the names out, I've copied the packet fields into
 * buffers to be output later. Also note that if it receives an idle value of 9999
 * the normal 30 space output will be bypassed. This is so that muds who want to
 * customize the listing headers in their who-reply packets can do so and the results
 * won't get chopped off after the 30th character. If for some reason a person on
 * the target mud just happens to have been idling for 9999 cycles, their data may
 * be displayed strangely compared to the rest. But I don't expect that 9999 is a very
 * common length of time to be idle either :P
 * Receving an idle value of 19998 may also cause odd results since this is used
 * to indicate receipt of the last line of a who, which is typically the number of
 * visible players found.
 */
void I3_process_who_reply( I3_HEADER *header, char *s ) 
{
    char *ps =s, *next_ps, *next_ps2; 
    CHAR_DATA *ch;
    char person[LGST], title[SMST];
    int idle;

   if( !( ch = I3_find_user( header->target_username ) ) )
	return;

   ps += 2;

    while( 1 ) 
    {
	if( ps[0] == '}' ) 
	{
	   i3_to_char( "{WNo information returned.\n\r", ch );
	    return;
	}

	I3_get_field( ps, &next_ps );

	ps += 2;
	I3_get_field( ps, &next_ps2 );
	I3_remove_quotes( &ps );
	i3strlcpy( person, ps, LGST );
	ps = next_ps2;
	I3_get_field( ps, &next_ps2 );
	idle = atoi( ps );
	ps = next_ps2;
	I3_get_field( ps, &next_ps2 );
	I3_remove_quotes( &ps );
	i3strlcpy( title, ps, SMST );
	ps = next_ps2;

      if( idle == 9999 )
         i3_printf( ch, "%s %s\n\r\n\r", person, title );
	else if( idle == 19998 )
	   i3_printf( ch, "\n\r%s %s\n\r", person, title );
      else if( idle == 29997 )
	   i3_printf( ch, "\n\r%s %s\n\r\n\r", person, title );
	else
	   i3_printf( ch, "%s %s\n\r", person, title );

	ps = next_ps;
	if( ps[0] == '}' )
	    break;
    }
    return;
}

void I3_send_emoteto( CHAR_DATA *ch, char *to, I3_MUD *mud, char *message ) 
{
   char buf[LGST];

   if( !I3_is_connected() )
	return;

   if( strstr( message, "$N" ) == NULL )
	snprintf( buf, LGST, "$N %s", message );
   else
	i3strlcpy( buf, message, LGST );

   I3_escape( to );
   I3_write_header( "emoteto", I3_THISMUD, CH_I3NAME(ch), mud->name, to );
   I3_write_buffer( "\"" );
   I3_write_buffer( CH_I3NAME(ch) );
   I3_write_buffer( "\",\"" );
   send_to_i3( I3_escape( buf ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_emoteto( I3_HEADER *header, char *s ) 
{
   CHAR_DATA *ch;
   char *ps = s, *next_ps;
   char visname[SMST], buf[SMST];
   char msg[LGST];

   snprintf( buf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( header->target_username ) ) )
   {
	if( !i3exists_player( header->target_username ) )
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	else 
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3PERM(ch) < I3PERM_MORT )
   {
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	return;
   }

   if( I3ISINVIS(ch) || i3ignoring( ch, buf ) || !ch->desc )
   {
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   snprintf( visname, SMST, "%s@%s", ps, header->originator_mudname );

   ps = next_ps;  
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( msg, LGST, "&c%s\n\r", I3_convert_channel_message( ps, visname, visname ) );
   i3_to_char( msg, ch );
   return;
}

void I3_send_finger( CHAR_DATA *ch, char *user, char *mud ) 
{
   if( !I3_is_connected() )
	return;

   I3_escape( mud );

   I3_write_header( "finger-req", I3_THISMUD, CH_I3NAME(ch), mud, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_escape( user ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

/* The output of this was slightly modified to resemble the Finger snippet */
void I3_process_finger_reply( I3_HEADER *header, char *s ) 
{
   CHAR_DATA *ch;
   char *ps = s, *next_ps;
   char title[SMST], email[SMST], last[SMST], level[SMST];

   if( !( ch = I3_find_user( header->target_username ) ) )
	return;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3_printf( ch, "{wI3FINGER information for {G%s@%s\n\r", ps, header->originator_mudname );
   i3_to_char( "{w-------------------------------------------------\n\r", ch );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( title, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( email, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( last, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( level, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   i3_printf( ch, "{wTitle: {G%s\n\r", title );
   i3_printf( ch, "{wLevel: {G%s\n\r", level );
   i3_printf( ch, "{wEmail: {G%s\n\r", email );
   i3_printf( ch, "{wHTTP : {G%s\n\r", ps );
   i3_printf( ch, "{wLast on: {G%s\n\r", last );

   return;
}

void I3_process_finger_req( I3_HEADER *header, char *s ) 
{
   CHAR_DATA *ch;
   char *ps = s, *next_ps;
   char smallbuf[200], buf[SMST];

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( buf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( ps ) ) )
   {
	if( !i3exists_player( ps ) )
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	else
	   I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3PERM(ch) < I3PERM_MORT )
   {
	I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "No such player." );
	return;
   }

   if( I3ISINVIS(ch) || i3ignoring( ch, buf ) )
   {
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", "That player is offline." );
	return;
   }

   if( I3IS_SET( I3FLAG(ch), I3_DENYFINGER ) || I3IS_SET( I3FLAG(ch), I3_PRIVACY ) )
   {
	snprintf( buf, SMST, "%s is not accepting fingers.", CH_I3NAME(ch) );
      I3_send_error( header->originator_mudname, header->originator_username, "unk-user", buf );
	return;
   }

   i3_printf( ch, "%s@%s has requested your i3finger information.\n\r",
	header->originator_username, header->originator_mudname );

   I3_write_header( "finger-reply", I3_THISMUD, NULL, header->originator_mudname, header->originator_username );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_escape( CH_I3NAME(ch) ) );
   I3_write_buffer( "\",\"" );
   I3_write_buffer( I3_escape( CH_I3NAME(ch) ) );
   send_to_i3( I3_escape( CH_I3TITLE(ch) ) );
   I3_write_buffer( "\",\"\",\"" );
#ifdef FINGERCODE
   if( ch->pcdata->email )
   {
	if( !IS_SET( ch->pcdata->flags, PCFLAG_PRIVACY ) )
	   I3_write_buffer( ch->pcdata->email );
	else
	   I3_write_buffer( "[Private]" );
   }
#else
   I3_write_buffer( "Not supported" );
#endif
   I3_write_buffer( "\",\"" );
   i3strlcpy( smallbuf, "-1", 200 ); /* online since */
   I3_write_buffer( smallbuf );
   I3_write_buffer( "\"," );
   snprintf( smallbuf, 200, "%ld", -1l );
   I3_write_buffer( smallbuf );
   I3_write_buffer( ",\"" );
   I3_write_buffer( "[PRIVATE]" );
   I3_write_buffer( "\",\"" );
   snprintf( buf, SMST, "%s", rankbuffer( ch ) );
   send_to_i3( buf );
#ifdef FINGERCODE
   I3_write_buffer( "\",\"" );
   if( ch->pcdata->homepage )
      I3_write_buffer( I3_escape( ch->pcdata->homepage ) );
   else
	I3_write_buffer( "Not Provided" );
   I3_write_buffer( "\",})\r" );
#else
   I3_write_buffer( "\",\"Not Suported\",})\r" );
#endif
   I3_send_packet( );

   return;
}

void I3_send_locate( CHAR_DATA *ch, char *user )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "locate-req", I3_THISMUD, CH_I3NAME(ch), NULL, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_escape( user ) );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_process_locate_reply( I3_HEADER *header, char *s ) 
{
   char mud_name[SMST], user_name[SMST], status[SMST];
   char *ps = s, *next_ps;
   CHAR_DATA *ch;

   if( !( ch = I3_find_user( header->target_username ) ) )
	return;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( mud_name, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( user_name, ps, SMST );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   ps = next_ps;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( status, ps, SMST );

   if( !str_cmp( status, "active" ) )
	i3strlcpy( status, "Online", SMST );

   if( !str_cmp( status, "exists, but not logged on" ) )
	i3strlcpy( status, "Offline", SMST );

   i3_printf( ch, "{RI3 Locate: {Y%s@%s: {c%s.\n\r", user_name, mud_name, status );
   return;
}

void I3_process_locate_req( I3_HEADER *header, char *s ) 
{
   char *ps = s, *next_ps;
   char smallbuf[50], buf[SMST];
   CHAR_DATA *ch;
   bool choffline = FALSE;

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );

   snprintf( buf, SMST, "%s@%s", header->originator_username, header->originator_mudname );

   if( !( ch = I3_find_user( ps ) ) )
   {
	if( i3exists_player( ps ) )
	   choffline = TRUE;
	else
	   return;
   }

   if( ch )
   {
      if( I3PERM(ch) < I3PERM_MORT )
         return;

      if( I3ISINVIS(ch) )
         choffline = TRUE;

      if( i3ignoring( ch, buf ) )
	   choffline = TRUE;
   }

   I3_write_header( "locate-reply", I3_THISMUD, NULL, header->originator_mudname, header->originator_username );
   I3_write_buffer( "\"" );
   I3_write_buffer( I3_THISMUD );
   I3_write_buffer( "\",\"" );
   if( !choffline )
      I3_write_buffer( CH_I3NAME(ch) );
   else
	I3_write_buffer( i3capitalize(ps) );
   I3_write_buffer( "\"," );
   snprintf( smallbuf, 50, "%ld", -1l );
   I3_write_buffer( smallbuf );
   if( !choffline )
      I3_write_buffer( ",\"Online\",})\r" );
   else
	I3_write_buffer( ",\"Offline\",})\r" );
   I3_send_packet( );

   return;
}

void I3_send_channel_listen( I3_CHANNEL *channel, bool lconnect ) 
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-listen", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\"," );
   if( lconnect )
	I3_write_buffer( "1,})\r" );
   else
	I3_write_buffer( "0,})\r" );
   I3_send_packet( );

   return;
}

void I3_mudlisten( CHAR_DATA *ch, char *argument )
{
   I3_CHANNEL *channel;
   char arg[SMST];

   if( !argument || argument[0] == '\0' )
   {
      i3_to_char( "Usage: i3mudlisten [all/none]\n\r", ch );
      i3_to_char( "Usage: i3mudlisten <localchannel> [on/off]\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "all" ) )
   {
	for( channel = first_I3chan; channel; channel = channel->next )
	{
         if( !channel->local_name || channel->local_name[0] == '\0' )
            continue;

         i3_printf( ch, "Subscribing to %s.\n\r", channel->local_name );
         I3_send_channel_listen( channel, TRUE );
	}
	i3_to_char( "{YThe mud is now subscribed to all available local I3 channels.\n\r", ch );
	return;		
   }

   if( !str_cmp( argument, "none" ) )
   {
	for( channel = first_I3chan; channel; channel = channel->next )
	{
         if( !channel->local_name || channel->local_name[0] == '\0' )
            continue;

         i3_printf( ch, "Unsubscribing from %s.\n\r", channel->local_name );
         I3_send_channel_listen( channel, FALSE );
	}
	i3_to_char( "{YThe mud is now unsubscribed from all available local I3 channels.\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, arg );
   if( !( channel = find_I3_channel_by_localname( arg ) ) )
   {
      i3_to_char( "No such channel configured locally.\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "on" ) )
   {
      i3_printf( ch, "Turning %s channel on.\n\r", channel->local_name );
      I3_send_channel_listen( channel, TRUE );
      return;
   }

   if( !str_cmp( argument, "off" ) )
   {
      i3_printf( ch, "Turning %s channel off.\n\r", channel->local_name );
      I3_send_channel_listen( channel, FALSE );
      return;
   }
   I3_mudlisten( ch, "" );
   return;
}

#define I3KEY( literal, field, value )				\
				if ( !str_cmp( word, literal ) )	\
				{						\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

/*
 * Read a string from file fp using I3STRALLOC [Taken from Smaug's fread_string]
 */
char *i3fread_string( FILE *fp )
{
    char buf[LGST];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    i3bug( "%s", "i3fread_string: EOF encountered on read." );
	    return I3STRALLOC( "" );
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return I3STRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (LGST - 1) )
	{
	     i3bug( "%s", "i3fread_string: string too long" );
	     *plast = '\0';
	     return I3STRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    i3bug( "%s", "i3fread_string: EOF" );
	    *plast = '\0';
	    return I3STRALLOC( buf );

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return I3STRALLOC( buf );
	}
    }
}

/*
 * Read a number from a file. [Taken from Smaug's fread_number]
 */
int i3fread_number( FILE *fp )
{
   int number;
   bool sign;
   char c;

   do
   {
      if ( feof(fp) )
      {
         i3log( "%s", "i3fread_number: EOF encountered on read." );
         return 0;
      }
	c = getc( fp );
   }
   while ( isspace(c) );

   number = 0;

   sign = FALSE;
   if ( c == '+' )
   {
	c = getc( fp );
   }
   else if ( c == '-' )
   {
	sign = TRUE;
	c = getc( fp );
   }

   if ( !isdigit(c) )
   {
	i3log( "i3fread_number: bad format. (%c)", c );
	return 0;
   }

   while ( isdigit(c) )
   {
      if( feof(fp) )
      {
         i3log( "%s", "i3fread_number: EOF encountered on read." );
         return number;
      }
	number = number * 10 + c - '0';
	c      = getc( fp );
   }

   if ( sign )
	number = 0 - number;

   if ( c == '|' )
	number += i3fread_number( fp );
   else if ( c != ' ' )
	ungetc( c, fp );

   return number;
}

/*
 * Read to end of line into static buffer [Taken from Smaug's fread_line]
 */
char *i3fread_line( FILE *fp )
{
    static char line[LGST];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    i3bug( "%s", "i3fread_line: EOF encountered on read." );
	    return "";
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    i3bug( "%s", "i3fread_line: EOF encountered on read." );
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (LGST - 1) )
	{
	    i3bug( "%s", "i3fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}

/*
 * Read one word (into static buffer). [Taken from Smaug's fread_word]
 */
char *i3fread_word( FILE *fp )
{
    static char word[SMST];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    i3log( "%s", "i3fread_word: EOF encountered on read." );
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + SMST; pword++ )
    {
	if ( feof(fp) )
	{
	    i3log( "%s", "i3fread_word: EOF encountered on read." );
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    i3log( "%s", "i3fread_word: word too long" );
    return NULL;
}

/*
 * Read a letter from a file. [Taken from Smaug's fread_letter]
 */
char i3fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          i3log( "%s", "i3fread_letter: EOF encountered on read." );
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}

/*
 * Read to end of line (for comments). [Taken from Smaug's fread_to_eol]
 */
void i3fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    i3log( "%s", "i3fread_to_eol: EOF encountered on read." );
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

void i3init_char( CHAR_DATA *ch )
{
   if( IS_NPC(ch) )
	return;

   I3CREATE( CH_I3DATA(ch), I3_CHARDATA, 1 );
   I3LISTEN(ch)	    = NULL;
   I3DENY(ch)         = NULL;
   I3REPLY(ch)	    = NULL;
   I3FLAG(ch)	    = 0;
   I3SET_BIT( I3FLAG(ch), I3_COLOR ); /* Default color to on. People can turn this off if they hate it. */
   FIRST_I3IGNORE(ch) = NULL;
   LAST_I3IGNORE(ch)  = NULL;    
    //I3PERM(ch)         = I3PERM_NOTSET;
    
    I3_set_perms ( ch );

   return;
}

/*
 * Moved into its own function so it can be called seperatly from initalizers, so we can
 * get the character given I3PERM data even if their pfile has no contents related to
 * I3PERMS. - Lotherius 11-24-03
 */

void I3_set_perms ( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
    {
        bugf ( "Char without pcdata in I3_set_perms" );
        return;
    }

    /* Somewhat ugly looking, but this should be the only place level gets checked directly now.
     * This will also catch upgraders from old clients - provided they've got their levels set right.
     * If the mud doesn't use levels, the CH_I3LEVEL macro can be altered to suit whatever system it does use.
     * Samson 6-25-03
     */
    
    if( CH_I3LEVEL(ch) < this_mud->minlevel )
        I3PERM(ch) = I3PERM_NONE;
    else if( CH_I3LEVEL(ch) >= this_mud->minlevel && CH_I3LEVEL(ch) < this_mud->immlevel )
        I3PERM(ch) = I3PERM_MORT;
    else if( CH_I3LEVEL(ch) >= this_mud->immlevel && CH_I3LEVEL(ch) < this_mud->adminlevel )
        I3PERM(ch) = I3PERM_IMM;
    else if( CH_I3LEVEL(ch) >= this_mud->adminlevel && CH_I3LEVEL(ch) < this_mud->implevel )
        I3PERM(ch) = I3PERM_ADMIN;
    else if( CH_I3LEVEL(ch) >= this_mud->implevel )
        I3PERM(ch) = I3PERM_IMP;
    
}

void I3_char_login( CHAR_DATA *ch )
{
   int gender, sex;
   char buf[SMST];

   if( !this_mud )
	return;    

   if( !I3_is_connected() )
   {
	if( I3PERM(ch) >= I3PERM_IMM && i3wait == -2 )
	   i3_to_char( "{RThe Intermud-3 connection is down. Attempts to reconnect were abandoned due to excessive failures.\n\r", ch );
	return;
   }
    
    I3_set_perms(ch);

   if( I3PERM(ch) < I3PERM_MORT )
	return;

   if( this_mud->ucache == TRUE )
   {
      snprintf( buf, SMST, "%s@%s", CH_I3NAME(ch), I3_THISMUD );
      gender = I3_get_ucache_gender( buf );
      sex = dikutoi3gender( CH_I3SEX(ch) );

      if( gender == sex )
         return;

      I3_ucache_update( buf, sex );
	if( !I3IS_SET( I3FLAG(ch), I3_INVIS ) )
         I3_send_ucache_update( CH_I3NAME(ch), sex );
   }
   return;
}

#ifdef I31STMUD
#define i3fread_number     read_number
#define i3fread_string     (char*)read_string

bool i3load_char( CHAR_DATA *ch, READ_DATA *fp, const char *word )
#else
bool i3load_char( CHAR_DATA *ch, FILE *fp, const char *word )
#endif
{
   bool fMatch = FALSE;

   if( IS_NPC(ch) )
	return FALSE;
    
   switch( UPPER(word[0]) )
   {
	case 'I':
	   I3KEY( "i3perm", I3PERM(ch), i3fread_number( fp ) );
	   if( !str_cmp( word, "i3flags" ) )
	   {
		I3FLAG(ch) = i3fread_number( fp );
		I3_char_login( ch );
		fMatch = TRUE;
		break;
	   }

	   /* Legacy support - will be removed in a later version */
	   if( !str_cmp( word, "i3invis" ) )
	   {
		int tempinvis = i3fread_number( fp );
		if( tempinvis == TRUE )
		   I3SET_BIT( I3FLAG(ch), I3_INVIS );
		I3_char_login( ch );
		fMatch = TRUE;
		break;
	   }
	   if( !str_cmp( word, "i3listen" ) )
	   {
		I3LISTEN(ch) = i3fread_string( fp );
		if( I3LISTEN(ch) != NULL && I3_is_connected() )
		{
		   I3_CHANNEL *channel = NULL;
		   char *channels = I3LISTEN(ch);
		   char arg[SMST];

		   while( 1 )
      	   {
                  if( channels[0] == '\0' )
                     break;

			channels = i3one_argument( channels, arg );

			if( !( channel = find_I3_channel_by_localname( arg ) ) )
			   I3_unflagchan( &I3LISTEN( ch ), arg );
			if( channel && I3PERM( ch ) < channel->i3perm )
			   I3_unflagchan( &I3LISTEN( ch ), arg );
		   }
		}
		fMatch = TRUE;
		break;
	   }
	   if( !str_cmp( word, "i3deny" ) )
	   {
		I3DENY(ch) = i3fread_string( fp );
		if( I3DENY(ch) != NULL && I3_is_connected() )
		{
		   I3_CHANNEL *channel = NULL;
		   char *channels = I3DENY(ch);
		   char arg[SMST];

		   while( 1 )
      	   {
                  if( channels[0] == '\0' )
                     break;
			channels = i3one_argument( channels, arg );

			if( !( channel = find_I3_channel_by_localname( arg ) ) )
			   I3_unflagchan( &I3DENY( ch ), arg );
			if( channel && I3PERM( ch ) < channel->i3perm )
			   I3_unflagchan( &I3DENY( ch ), arg );
		   }
		}
		fMatch = TRUE;
		break;
	   }
         if( !str_cmp( word, "i3ignore" ) )
         {
            I3_IGNORE *temp;

	      I3CREATE( temp, I3_IGNORE, 1 );
            temp->name = i3fread_string( fp );
	      I3LINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
	      fMatch = TRUE;
	      break;
         }
	break;
   }
   return fMatch;
}
#ifdef I31STMUD
#undef i3fread_number
#undef i3fread_string
#endif

void i3save_char( CHAR_DATA *ch, FILE *fp )
{
   I3_IGNORE *temp;

   if( IS_NPC(ch) )
	return;

   fprintf( fp, "i3perm       %d\n", I3PERM(ch) );
   fprintf( fp, "i3flags	%d\n", I3FLAG(ch) );
   if( I3LISTEN(ch) )
      fprintf( fp, "i3listen	%s~\n", I3LISTEN(ch) );
   if( I3DENY(ch) )
      fprintf( fp, "i3deny	%s~\n", I3DENY(ch) );
   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
      fprintf( fp, "i3ignore	%s~\n", temp->name );
   return;
}

void I3_readucache( UCACHE_DATA *user, FILE *fp )
{
   const char *word;
   bool fMatch;

   for ( ; ; )
   {
	word   = feof( fp ) ? "End" : i3fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fp );
	      break;

	   case 'N':
		I3KEY( "Name",		user->name,		i3fread_string( fp ) );
		break;

         case 'S':
		I3KEY( "Sex",			user->gender,	i3fread_number( fp ) );
		break;

	   case 'T':
		I3KEY( "Time",		user->time,		i3fread_number( fp ) );
		break;

	   case 'E':
	      if ( !str_cmp( word, "End" ) )
		   return;
	      break;
	}
	if( !fMatch )
	   i3bug( "I3_readucache: no match: %s", word );
   }
}

void I3_load_ucache( void )
{
   FILE *fp;
   UCACHE_DATA *user;

   i3log( "%s", "Loading ucache data..." );

   if( !( fp = fopen( I3_UCACHE_FILE, "r" ) ) )
   {
	i3log( "%s", "No ucache data found." );
	return;
   }

   for ( ; ; )
   {
	char letter;
	char *word;

	letter = i3fread_letter( fp );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fp );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "%s", "I3_load_ucahe: # not found." );
	   break;
	}

      word = i3fread_word( fp );
	if ( !str_cmp( word, "UCACHE" ) )
	{
	   I3CREATE( user, UCACHE_DATA, 1 );
	   I3_readucache( user, fp );
	   I3LINK( user, first_ucache, last_ucache, next, prev );
	   continue;
	}
	else if ( !str_cmp( word, "END" ) )
	        break;
	else
	{
	   i3bug( "I3_load_ucache: bad section: %s.", word );
	   continue;
	}
   }
   I3FCLOSE( fp );
   return;
}

void I3_fread_config_file( FILE *fin )
{
   const char *word;
   bool fMatch;

   for( ;; )
   {
	word   = feof( fin ) ? "end" : i3fread_word( fin );
	fMatch = FALSE;
	
	switch( word[0] ) 
	{
	   case '#':
		fMatch = TRUE;
		i3fread_to_eol( fin );
		break;
	   case 'a':
		I3KEY( "adminemail",	this_mud->admin_email,	i3fread_string( fin ) );
		I3KEY( "adminlevel",	this_mud->adminlevel,	i3fread_number( fin ) );
		I3KEY( "amrcp",		this_mud->amrcp,		i3fread_number( fin ) );
		I3KEY( "auth",		this_mud->auth,		i3fread_number( fin ) );
		I3KEY( "autoconnect",	this_mud->autoconnect,	i3fread_number( fin ) );
		break;
	   case 'b':
		I3KEY( "basemudlib",	this_mud->base_mudlib,	i3fread_string( fin ) );
		I3KEY( "beep",		this_mud->beep,		i3fread_number( fin ) );
		break;
	   case 'c':
		I3KEY( "channel",		this_mud->channel,	i3fread_number( fin ) );
		break;
	   case 'e':
		I3KEY( "emoteto",		this_mud->emoteto,	i3fread_number( fin ) );
		if( !str_cmp( word, "end" ) )
		{
#if defined(I3CHRONICLES)
		   char lib_buf[LGST];
		   /*
		    * Adjust mudlib information based on already supplied info (mud.h). -Orion
		    */
		   snprintf( lib_buf, LGST, "%s %s.%s", CODEBASE_VERSION_TITLE, CODEBASE_VERSION_MAJOR, CODEBASE_VERSION_MINOR );
		   I3STRFREE( this_mud->mudlib );
		   this_mud->mudlib = STRALLOC( lib_buf );

		   /*
		    * Adjust base_mudlib information based on already supplied info (mud.h). -Orion
		    */
		   I3STRFREE( this_mud->base_mudlib );
		   this_mud->base_mudlib = STRALLOC( CODEBASE_VERSION_TITLE );
#endif
               I3STRFREE( this_mud->driver );
               this_mud->driver = I3STRALLOC( I3DRIVER );
		   return;
		}
		break;
	   case 'f':
		I3KEY( "file",		this_mud->file,		i3fread_number( fin ) );
		I3KEY( "finger",		this_mud->finger,		i3fread_number( fin ) );
		I3KEY( "ftp",			this_mud->ftp,		i3fread_number( fin ) );
		break;
	   case 'h':
		I3KEY( "http",		this_mud->http,		i3fread_number( fin ) );
		break;
	   case 'i':
		I3KEY( "immlevel",		this_mud->immlevel,	i3fread_number( fin ) );
		I3KEY( "implevel",		this_mud->implevel,	i3fread_number( fin ) );
		break;
	   case 'l':
		I3KEY( "locate",		this_mud->locate,		i3fread_number( fin ) );
		break;
	   case 'm':
		I3KEY( "mail",		this_mud->mail,		i3fread_number( fin ) );
		I3KEY( "minlevel",		this_mud->minlevel,	i3fread_number( fin ) );
		I3KEY( "mudlib",		this_mud->mudlib,		i3fread_string( fin ) );
		I3KEY( "mudtype",		this_mud->mud_type,	i3fread_string( fin ) );
		break;
	   case 'n':
		I3KEY( "news",		this_mud->news,		i3fread_number( fin ) );
		I3KEY( "nntp",		this_mud->nntp,		i3fread_number( fin ) );
		break;
	   case 'o':
		I3KEY( "openstatus",	this_mud->open_status,	i3fread_string( fin ) );
		break;
	   case 'p':
		I3KEY( "pop3",		this_mud->pop3,		i3fread_number( fin ) );
		break;
	   case 'r':
		I3KEY( "rcp",			this_mud->rcp,		i3fread_number( fin ) );
            if( !str_cmp( word, "router" ) )
            {
               ROUTER_DATA *router;
               char rname[SMST], rip[SMST], *ln;
               int rport;

               ln = i3fread_line( fin );
               sscanf( ln, "%s %s %d", rname, rip, &rport );

               I3CREATE( router, ROUTER_DATA, 1 );
               router->name = I3STRALLOC( rname );
               router->ip = I3STRALLOC( rip );
               router->port = rport;
               router->reconattempts = 0;
               I3LINK( router, first_router, last_router, next, prev );
               fMatch = TRUE;
               break;
            }
		break;
	   case 's':
		I3KEY( "smtp",		this_mud->smtp,		i3fread_number( fin ) );
		break;
	   case 't':
		I3KEY( "tell",		this_mud->tell,		i3fread_number( fin ) );
		I3KEY( "telnet",		this_mud->telnet,		i3fread_string( fin ) );
		I3KEY( "thismud",		this_mud->name,		i3fread_string( fin ) );
		break;
	   case 'u':
		I3KEY( "ucache",		this_mud->ucache,		i3fread_number( fin ) );
		break;
	   case 'w':
		I3KEY( "web",			this_mud->web,		i3fread_string( fin ) );
		I3KEY( "who",			this_mud->who,		i3fread_number( fin ) );
		break;
	}
	if( !fMatch ) 
	   i3bug( "I3_fread_config_file: Bad keyword: %s\n\r", word );
   }
}

bool I3_read_config( int mudport ) 
{
   FILE *fin, *fp;

   i3log( "%s", "Loading Intermud-3 network data..." );

   if( ( fin = fopen( I3_CONFIG_FILE, "r" ) ) == NULL ) 
   {
	i3log( "%s", "Can't open configuration file: i3.config" );
	i3log( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !this_mud )
      I3CREATE( this_mud, I3_MUD, 1 );

   this_mud->status 	 = -1;
   this_mud->autoconnect = 0;
   this_mud->player_port = mudport; /* Passed in from the mud's startup script */
   this_mud->password 	 = 0;
   this_mud->mudlist_id  = 0;
   this_mud->chanlist_id = 0;
   this_mud->minlevel 	 = 10; /* Minimum default level before I3 will acknowledge you exist */
   this_mud->immlevel    = 103; /* Default immortal level */
   this_mud->adminlevel  = 113; /* Default administration level */
   this_mud->implevel    = 115; /* Default implementor level */

   if( ( fp = fopen( I3_PASSWORD_FILE, "r" ) ) != NULL )
   {
 	char *word;

      word = i3fread_word( fp );

      if( !str_cmp( word, "#PASSWORD" ) )
	{
	   char *ln = i3fread_line( fp );
	   int pass, mud, chan;

	   pass = mud = chan = 0;
	   sscanf( ln, "%d %d %d", &pass, &mud, &chan );
	   this_mud->password 	 = pass;
	   this_mud->mudlist_id  = mud;
	   this_mud->chanlist_id = chan;
	}
      I3FCLOSE( fp );
   }

   for( ; ; )
   {
    	char letter;
 	char *word;

   	letter = i3fread_letter( fin );

	if( letter == '#' )
	{
	   i3fread_to_eol( fin );
	   continue;
      }

	if( letter != '$' )
	{
	   i3bug( "%s", "I3_read_config: $ not found" );
	   break;
	}

	word = i3fread_word( fin );
	if( !str_cmp( word, "I3CONFIG" ) )
	{
	   I3_fread_config_file( fin );
	   continue;
	}
      else if( !str_cmp( word, "END" ) )
	   break;
	else
	{
	   i3bug( "I3_read_config: Bad section in config file: %s", word );
	   continue;
      }
   }
   I3FCLOSE( fin );

   if( !this_mud->name || this_mud->name[0] == '\0' )
   {
	i3log( "%s", "Mud name not loaded in configuration file." );
	i3log( "%s", "Network configuration aborted." );
	destroy_I3_mud( this_mud );
	return FALSE;
   }

   if( !first_router || !first_router->name || !first_router->ip || !first_router->port )
   {
	i3log( "%s", "No valid routers found in config file." );
	i3log( "%s", "Network configuration aborted." );
	destroy_I3_mud( this_mud );
	return FALSE;
   }

   if( !this_mud->telnet || this_mud->telnet[0] == '\0' )
	this_mud->telnet = I3STRALLOC( "Address not configured" );

   if( !this_mud->web || this_mud->web[0] == '\0' )
	this_mud->web = I3STRALLOC( "Address not configured" );

   I3_THISMUD = this_mud->name;
   I3_ROUTER_NAME = first_router->name;
   return TRUE;
}

void I3_readban( I3_BAN *ban, FILE *fin ) 
{
   const char *word;
   bool fMatch;

   for ( ; ; )
   {
	word   = feof( fin ) ? "End" : i3fread_word( fin );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fin );
	      break;

	   case 'N':
		I3KEY( "Name",		ban->name,		i3fread_string( fin ) );
		break;

	   case 'E':
	      if ( !str_cmp( word, "End" ) )
		   return;
	      break;
	}
	if( !fMatch )
	   i3bug( "I3_readban: no match: %s", word );
   }
}

void I3_loadbans( void )
{
   FILE *fin;
   I3_BAN *ban;

   first_i3ban = NULL;
   last_i3ban = NULL;

   i3log( "%s", "Loading ban list..." );

   if( ( fin = fopen( I3_BAN_FILE, "r" ) ) == NULL ) 
   {
	i3log( "%s", "No ban list defined." );
	return;
   }

   for ( ; ; )
   {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "%s", "I3_loadbans: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if ( !str_cmp( word, "I3BAN" ) )
	{
	   I3CREATE( ban, I3_BAN, 1 );
	   I3_readban( ban, fin );
         if( !ban->name )
            I3DISPOSE( ban );
         else
	      I3LINK( ban, first_i3ban, last_i3ban, next, prev );
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	      break;
	else
	{
	   i3bug( "I3_loadbans: bad section: %s.", word );
	   continue;
	}
   }
   I3FCLOSE( fin );
   return;
}

void I3_write_bans( void ) 
{
   FILE *fout;
   I3_BAN *ban;

   if( ( fout = fopen( I3_BAN_FILE, "w" ) ) == NULL ) 
   {
	i3log( "%s", "Couldn't write to ban list file." );
	return;
   }

   for( ban = first_i3ban; ban; ban = ban->next ) 
   {
	fprintf( fout, "%s", "#I3BAN\n" );
	fprintf( fout, "Name   %s~\n", ban->name );
	fprintf( fout, "%s", "End\n\n" );
   }
   fprintf( fout, "%s", "#END\n" );
   I3FCLOSE( fout );
}

void I3_readchannel( I3_CHANNEL *channel, FILE *fin ) 
{
   const char *word;
   bool fMatch;

   for ( ; ; )
   {
	word   = feof( fin ) ? "End" : i3fread_word( fin );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fin );
	      break;

	   case 'C':
		I3KEY( "ChanName",		channel->I3_name,		i3fread_string( fin ) );
		I3KEY( "ChanMud",		channel->host_mud,	i3fread_string( fin ) );
		I3KEY( "ChanLocal",		channel->local_name,	i3fread_string( fin ) );
		I3KEY( "ChanLayM",		channel->layout_m,	i3fread_string( fin ) );
		I3KEY( "ChanLayE",		channel->layout_e,	i3fread_string( fin ) );
		I3KEY( "ChanLevel",		channel->i3perm,  	i3fread_number( fin ) );
		I3KEY( "ChanStatus",	channel->status,		i3fread_number( fin ) );
		I3KEY( "ChanFlags", 	channel->flags,		i3fread_number( fin ) );
		break;

	   case 'E':
	      if ( !str_cmp( word, "End" ) )
		{
		   /* Legacy support to convert channel permissions */
		   if( channel->i3perm > I3PERM_IMP )
		   {
			/* The I3PERM_NONE condition should realistically never happen.... */
      		if( channel->i3perm < this_mud->minlevel )
	   		   channel->i3perm = I3PERM_NONE;
      		else if( channel->i3perm >= this_mud->minlevel && channel->i3perm < this_mud->immlevel )
	   		   channel->i3perm = I3PERM_MORT;
      		else if( channel->i3perm >= this_mud->immlevel && channel->i3perm < this_mud->adminlevel )
	   		   channel->i3perm = I3PERM_IMM;
      		else if( channel->i3perm >= this_mud->adminlevel && channel->i3perm < this_mud->implevel )
	   		   channel->i3perm = I3PERM_ADMIN;
      		else if( channel->i3perm >= this_mud->implevel )
	   		   channel->i3perm = I3PERM_IMP;
		   }
		   return;
		}
	      break;
	}
	if( !fMatch )
	   i3bug( "I3_readchannel: no match: %s", word );
   }
}

void I3_loadchannels( void )
{
    FILE *fin;
    I3_CHANNEL *channel;

    first_I3chan = NULL;
    last_I3chan = NULL;

    i3log( "%s", "Loading channels..." );

    if( ( fin = fopen( I3_CHANNEL_FILE, "r" ) ) == NULL ) 
    {
	i3log( "%s", "No channel config file found." );
	return;
    }

    for ( ; ; )
    {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if ( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if ( letter != '#' )
	{
	   i3bug( "%s", "I3_loadchannels: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if ( !str_cmp( word, "I3CHAN" ) )
	{
	   int x;

	   I3CREATE( channel, I3_CHANNEL, 1 );
	   I3_readchannel( channel, fin );

	   for( x = 0; x < MAX_I3HISTORY; x++ )
		channel->history[x] = NULL;
	   I3LINK( channel, first_I3chan, last_I3chan, next, prev );
	   continue;
	}
	else
         if ( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   i3bug( "I3_loadchannels: bad section: %s.", word );
	   continue;
	}
    }
    I3FCLOSE( fin );
    return;
}

void I3_write_channel_config( void ) 
{
    FILE *fout;
    I3_CHANNEL *channel;

    if( ( fout = fopen( I3_CHANNEL_FILE, "w" ) ) == NULL ) 
    {
	i3log( "%s", "Couldn't write to channel config file." );
	return;
    }

    for( channel = first_I3chan; channel; channel = channel->next ) 
    {
	if( channel->local_name ) 
      {
	    fprintf( fout, "%s", "#I3CHAN\n" );
	    fprintf( fout, "ChanName   %s~\n", channel->I3_name );
    	    fprintf( fout, "ChanMud    %s~\n", channel->host_mud );
	    fprintf( fout, "ChanLocal  %s~\n", channel->local_name );
	    fprintf( fout, "ChanLayM   %s~\n", channel->layout_m );
	    fprintf( fout, "ChanLayE   %s~\n", channel->layout_e );
	    fprintf( fout, "ChanLevel  %d\n", channel->i3perm );
	    fprintf( fout, "ChanStatus %d\n", channel->status );
	    fprintf( fout, "ChanFlags  %d\n", channel->flags );
	    fprintf( fout, "%s", "End\n\n" );
	}
    }
    fprintf( fout, "%s", "#END\n" );
    I3FCLOSE( fout );
}

void I3_mudlist( CHAR_DATA *ch, char *argument ) 
{
    I3_MUD *mud;
    char filter[SMST];
    int mudcount = 0;
    bool all = FALSE;

    argument = i3one_argument( argument, filter );

    if( !str_cmp( filter, "all" ) ) 
    {
	all = TRUE;
	argument = i3one_argument( argument, filter );
    }

    if( first_mud == NULL )
    {
	i3_to_char( "There are no muds to list!?\n\r", ch );
	return;
    }

    i3pager_printf( ch, "{W%-30s%-10.10s%-25.25s%-15.15s %s\n\r", "Name", "Type", "Mudlib", "Address", "Port" );
    for( mud = first_mud; mud; mud = mud->next ) 
    {
	if( mud == NULL )
	{
	   i3bug( "%s", "I3_mudlist: NULL mud found in listing!" );
	   continue;
	}

	if( mud->name == NULL )
	{
	   i3bug( "%s", "I3_mudlist: NULL mud name found in listing!" );
         continue;
	}

      if( filter[0] && i3str_prefix( filter, mud->name ) &&
        ( mud->mud_type && i3str_prefix( filter, mud->mud_type ) ) &&
        ( mud->mudlib && i3str_prefix( filter, mud->mudlib ) ) )
	   continue;

	if( !all && mud->status == 0 )
	    continue;

	mudcount++;

	switch( mud->status )
	{
	case -1:
          i3pager_printf( ch, "{c%-30s%-10.10s%-25.25s%-15.15s %d\n\r",
             mud->name, mud->mud_type, mud->mudlib, mud->ipaddress, mud->player_port );
	    break;
	case 0:
	    i3pager_printf( ch, "{R%-26s(down)\n\r", mud->name );
	    break;
	default:
	    i3pager_printf( ch, "{Y%-26s(rebooting, back in %d seconds)\n\r", mud->name, mud->status );
	    break;
	}
    }
    i3pager_printf( ch, "{W%d total muds listed.\n\r", mudcount );
    return;
}

void I3_chanlist( CHAR_DATA *ch, char *argument ) 
{
   I3_CHANNEL *channel;
   bool all = FALSE, found = FALSE;
   char filter[SMST];

   argument = i3one_argument( argument, filter );

   if( !str_cmp( filter, "all" ) && I3_is_connected() ) 
   {
 	all = TRUE;
	argument = i3one_argument( argument, filter );
   }

   i3send_to_pager( "{cLocal name          Perm    I3 Name             Hosted at           Status\n\r", ch );
   i3send_to_pager( "{c-------------------------------------------------------------------------------\n\r", ch );
   for( channel = first_I3chan; channel; channel = channel->next ) 
   {
	found = FALSE;

	if( !all && !channel->local_name && ( !filter || filter[0] == '\0' ) )
	   continue;

	if( I3PERM(ch) < I3PERM_ADMIN && !channel->local_name )
	   continue;

	if( I3PERM(ch) < channel->i3perm )
	   continue;

      if( !all && filter && filter[0] != '\0' && i3str_prefix( filter, channel->I3_name )
       && i3str_prefix( filter, channel->host_mud ) )
         continue;

	if( channel->local_name && I3_hasname( I3LISTEN(ch), channel->local_name ) )	
	   found = TRUE;

	i3pager_printf( ch, "{C%c {W%-18s{Y%-8s{B%-20s{M%-20s%-8s\n\r",
	    found ? '*' : ' ',
	    channel->local_name ? channel->local_name : "Not configured",
          perm_names[channel->i3perm], channel->I3_name, channel->host_mud,
	    channel->status == 0 ? "{GPublic" : "{RPrivate" );
   }
   i3send_to_pager( "{C*: You are listening to these channels.\n\r", ch );
   return;
}

void I3_setup_channel( CHAR_DATA *ch, char *argument ) 
{
   DESCRIPTOR_DATA *d;
   char localname[SMST], I3_name[SMST];
   I3_CHANNEL *channel, *channel2;
   int permvalue = I3PERM_MORT;

   argument = i3one_argument( argument, I3_name );
   argument = i3one_argument( argument, localname );

   if( !( channel = find_I3_channel_by_name( I3_name ) ) )
   {
	i3_to_char("{YUnknown channel\n\r"
	    "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
   }

   if( !localname || localname[0] == '\0' )
   {
	if( !channel->local_name ) 
	{
	    i3_printf( ch, "Channel %s@%s isn't configured.\n\r", channel->I3_name, channel->host_mud );
	    return;
	}

	if( channel->i3perm > I3PERM(ch) )
	{
	   i3_printf( ch, "You do not have sufficient permission to remove the %s channel.\n\r", channel->local_name );
	   return;
	}

	for( d = first_descriptor; d; d = d->next )
	{
	   if( !d->character )
		continue;

	   if( I3_hasname( I3LISTEN(d->character), channel->local_name ) )
		I3_unflagchan( &I3LISTEN(d->character), channel->local_name );
	   if( I3_hasname( I3DENY(d->character), channel->local_name ) )
		I3_unflagchan( &I3DENY(d->character), channel->local_name );
	}
	i3log( "setup_channel: removing %s as %s@%s", channel->local_name, channel->I3_name, channel->host_mud );
      I3_send_channel_listen( channel, FALSE );
      I3STRFREE( channel->local_name );
	I3_write_channel_config();
   }
   else 
   {
	if( channel->local_name ) 
      {
	   i3_printf( ch, "Channel %s@%s is already known as %s.\n\r", channel->I3_name, channel->host_mud, channel->local_name );
	   return;
	}
	if( ( channel2 = find_I3_channel_by_localname( localname ) ) ) 
 	{
	   i3_printf( ch, "Channel %s@%s is already known as %s.\n\r", channel2->I3_name, channel2->host_mud, channel2->local_name );
	   return;
	}

	if( argument && argument[0] != '\0' )
	{
	   permvalue = get_permvalue( argument );
	   if( permvalue < 0 || permvalue > I3PERM_IMP )
	   {
	      i3_to_char( "Invalid permission setting.\n\r", ch );
	      return;
	   }
	   if( permvalue > I3PERM(ch) )
	   {
	      i3_to_char( "You cannot assign a permission value above your own.\n\r", ch );
	      return;
	   }
	}
	channel->local_name = I3STRALLOC( localname );
	channel->i3perm = permvalue;
      channel->layout_m = I3STRALLOC( "{R[{W%s{R] {C%s@%s: {c%s" );
      channel->layout_e = I3STRALLOC( "{R[{W%s{R] {c%s" );
	i3_printf( ch, "%s@%s is now locally known as %s\n\r",
	    channel->I3_name, channel->host_mud, channel->local_name );
	i3log( "setup_channel: setting up %s@%s as %s", channel->I3_name, channel->host_mud, channel->local_name );
      I3_send_channel_listen( channel, TRUE );
	I3_write_channel_config();
   }
}

void I3_edit_channel( CHAR_DATA *ch, char *argument ) 
{
   char localname[SMST];
   char arg2[SMST];
   I3_CHANNEL *channel;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3 editchan <localname> localname <new localname>\n\r", ch );
	i3_to_char( "Usage: i3 editchan <localname> perm <type>\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, localname );

   if( ( channel = find_I3_channel_by_localname( localname ) ) == NULL )
   {
	i3_to_char("{YUnknown local channel\n\r"
	    "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, arg2 );

   if( channel->i3perm > I3PERM(ch) )
   {
	i3_to_char( "You do not have sufficient permissions to edit this channel.\n\r", ch );
	return;
   }

   if( !str_cmp( arg2, "localname" ) )
   {
	i3_printf( ch, "Local channel %s renamed to %s.\n\r", channel->local_name, argument );
	I3STRFREE( channel->local_name );
	channel->local_name = I3STRALLOC( argument );
	I3_write_channel_config();
	return;
   }

   if( !str_cmp( arg2, "perm" ) || !str_cmp( arg2, "permission" ) )
   {
	int permvalue = get_permvalue( argument );

	if( permvalue < 0 || permvalue > I3PERM_IMP )
	{
	   i3_to_char( "Invalid permission setting.\n\r", ch );
	   return;
	}
	if( permvalue > I3PERM(ch) )
	{
	   i3_to_char( "You cannot set a permission higher than your own.\n\r", ch );
	   return;
	}
	if( channel->i3perm > I3PERM(ch) )
	{
	   i3_to_char( "You cannot edit a channel above your permission level.\n\r", ch );
	   return;
	}
	channel->i3perm = permvalue;
	i3_printf( ch, "Local channel %s permission changed to %s.\n\r", channel->local_name, argument );
	I3_write_channel_config();
	return;
   }
   I3_edit_channel( ch, "" );
   return;
}

void I3_chan_who( CHAR_DATA *ch, char *argument ) 
{
    char channel_name[SMST];
    I3_CHANNEL *channel;
    I3_MUD *mud;

   argument = i3one_argument( argument, channel_name );

   if( !channel_name || channel_name[0] == '\0' || !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3chanwho <local channel> <mud>\n\r", ch );
	return;
   }

    if( ( channel = find_I3_channel_by_localname( channel_name ) ) == NULL ) 
    {
	i3_to_char( "{YUnknown channel.\n\r"
	    "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
    }

    if( !( mud = find_I3_mud_by_name( argument ) ) ) 
    {
	i3_to_char( "{YUnknown mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
    }

    if( mud->status >= 0 ) 
    {
	i3_printf( ch, "%s is marked as down.\n\r", mud->name );
	return;
    }

    I3_send_chan_who( ch, channel, mud );
}

void I3_listen_channel( CHAR_DATA *ch, char *argument ) 
{
   I3_CHANNEL *channel;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3listen <local channel name>\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "all" ) )
   {
	for( channel = first_I3chan; channel; channel = channel->next )
	{
         if( !channel->local_name || channel->local_name[0] == '\0' )
            continue;

	   if( I3PERM(ch) >= channel->i3perm && !I3_hasname( I3LISTEN(ch), channel->local_name ) )
		I3_flagchan( &I3LISTEN(ch), channel->local_name );
	}
      i3_to_char( "{YYou are now listening to all available I3 channels.\n\r", ch );
	return;		
   }

   if( !str_cmp( argument, "none" ) )
   {
	for( channel = first_I3chan; channel; channel = channel->next )
	{
         if( !channel->local_name || channel->local_name[0] == '\0' )
            continue;

	   if( I3_hasname( I3LISTEN(ch), channel->local_name ) )
		I3_unflagchan( &I3LISTEN(ch), channel->local_name );
	}
	i3_to_char( "{YYou no longer listen to any available I3 channels.\n\r", ch );
	return;
   }

   if( ( channel = find_I3_channel_by_localname( argument ) ) == NULL ) 
   {
	   i3_to_char( "{YUnknown channel.\n\r"
	   "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
   }

   if( I3_hasname( I3LISTEN(ch), channel->local_name ) )
   {
	   i3_printf( ch, "You no longer listen to %s\n\r", channel->local_name );
	   I3_unflagchan( &I3LISTEN(ch), channel->local_name );
   }
   else
   {
      if( I3PERM(ch) < channel->i3perm )
      {
	   i3_printf( ch, "Channel %s is above your permission level.\n\r", channel->local_name );
	   return;
      }
	   i3_printf( ch, "You now listen to %s\n\r", channel->local_name );
	   I3_flagchan( &I3LISTEN(ch), channel->local_name );
   }
   return;
}

void I3_deny_channel( CHAR_DATA *ch, char *argument ) 
{
   char vic_name[SMST];
   CHAR_DATA *victim;
   I3_CHANNEL *channel;

   argument = i3one_argument( argument, vic_name );

   if( !vic_name || vic_name[0] == '\0' || !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3deny <person> <local channel name>\n\r", ch );
	i3_to_char( "Usage: i3deny <person> [tell/beep/finger]\n\r", ch );
	return;
   }

   if( !( victim = I3_find_user( vic_name ) ) )
   {
	i3_to_char( "No such person is currently online.\n\r", ch );
	return;
   }

   if( I3PERM(ch) <= I3PERM(victim) )
   {
      i3_to_char( "You cannot alter their settings.\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "tell" ) )
   {
	if( !I3IS_SET( I3FLAG(victim), I3_DENYTELL ) )
	{
	   I3SET_BIT( I3FLAG(victim), I3_DENYTELL );
	   i3_printf( ch, "%s can no longer use i3tells.\n\r", CH_I3NAME(victim) );
	   return;
	}
	I3REMOVE_BIT( I3FLAG(victim), I3_DENYTELL );
	i3_printf( ch, "%s can use i3tells again.\n\r", CH_I3NAME(victim) );
	return;
   }

   if( !str_cmp( argument, "beep" ) )
   {
	if( !I3IS_SET( I3FLAG(victim), I3_DENYBEEP ) )
   {
	   I3SET_BIT( I3FLAG(victim), I3_DENYBEEP );
	   i3_printf( ch, "%s can no longer use i3beeps.\n\r", CH_I3NAME(victim) );
	   return;
	}
	I3REMOVE_BIT( I3FLAG(victim), I3_DENYBEEP );
	i3_printf( ch, "%s can use i3beeps again.\n\r", CH_I3NAME(victim) );
	return;
   }

   if( !str_cmp( argument, "finger" ) )
   {
	if( !I3IS_SET( I3FLAG(victim), I3_DENYFINGER ) )
	{
	   I3SET_BIT( I3FLAG(victim), I3_DENYFINGER );
	   i3_printf( ch, "%s can no longer use i3fingers.\n\r", CH_I3NAME(victim) );
	   return;
	}
	I3REMOVE_BIT( I3FLAG(victim), I3_DENYFINGER );
	i3_printf( ch, "%s can use i3fingers again.\n\r", CH_I3NAME(victim) );
	return;
   }

   if( !( channel = find_I3_channel_by_localname( argument ) ) ) 
   {
	i3_to_char( "{YUnknown channel.\n\r"
	   "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
   }

   if( I3_hasname( I3DENY(ch), channel->local_name ) )
   {
	   i3_printf( ch, "%s can now listen to %s\n\r", CH_I3NAME(victim), channel->local_name );
	   I3_unflagchan( &I3DENY(ch), channel->local_name );
   }
   else
   {
	   i3_printf( ch, "%s can no longer listen to %s\n\r", CH_I3NAME(victim), channel->local_name );
	   I3_flagchan( &I3DENY(ch), channel->local_name );
   }
   return;
}

void free_i3chardata( CHAR_DATA *ch )
{
   I3_IGNORE *temp, *next;

   if( IS_NPC(ch) )
	return;

   I3STRFREE( I3LISTEN(ch) );
   I3STRFREE( I3DENY(ch) );
   I3STRFREE( I3REPLY(ch) );

   if( FIRST_I3IGNORE(ch) )
   {
      for( temp = FIRST_I3IGNORE(ch); temp; temp = next )
      {
         next = temp->next;
	   I3STRFREE( temp->name );
    	   I3UNLINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
    	   I3DISPOSE( temp );
      }
   }
   I3DISPOSE( CH_I3DATA(ch) );
   return;
}

void I3_mudinfo( CHAR_DATA *ch, char *argument ) 
{
   I3_MUD *mud;

   if( !argument || argument[0] == '\0' ) 
   {
	i3_to_char( "{YWhich mud do you want information about?\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !( mud = find_I3_mud_by_name( argument ) ) )
   {
	i3_to_char( "{YUnknown mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   i3_printf( ch, "Information about %s\n\r\n\r", mud->name );
   if( mud->status == 0 )
	i3_to_char( "Status     : Currently down\n\r", ch );
   else if( mud->status > 0 )
	i3_printf( ch, "Status     : Currently rebooting, back in %d seconds\n\r", mud->status );
   i3_printf( ch, "MUD port   : %s %d\n\r", mud->ipaddress, mud->player_port );
   i3_printf( ch, "Base mudlib: %s\n\r", mud->base_mudlib );
   i3_printf( ch, "Mudlib     : %s\n\r", mud->mudlib );
   i3_printf( ch, "Driver     : %s\n\r", mud->driver );
   i3_printf( ch, "Type       : %s\n\r", mud->mud_type );
   i3_printf( ch, "Open status: %s\n\r", mud->open_status );
   i3_printf( ch, "Admin      : %s\n\r", mud->admin_email );
   if( mud->web )
      i3_printf( ch, "URL        : %s\n\r", mud->web );
   if( mud->web_wrong && !mud->web )
      i3_printf( ch, "URL        : %s\n\r", mud->web_wrong );
   if( mud->daemon )
      i3_printf( ch, "Daemon     : %s\n\r", mud->daemon );
   if( mud->time )
      i3_printf( ch, "Time       : %s\n\r", mud->time );
   if( mud->banner )
      i3_printf( ch, "Banner:\n\r%s\n\r", mud->banner );

   i3_to_char( "Supports   : ", ch );
   if (mud->tell)	i3_to_char( "tell, ", ch );
   if (mud->beep)	i3_to_char( "beep, ", ch );
   if (mud->emoteto)	i3_to_char( "emoteto, ", ch );
   if (mud->who)		i3_to_char( "who, ", ch );
   if (mud->finger)	i3_to_char( "finger, ", ch );
   if (mud->locate)	i3_to_char( "locate, ", ch );
   if (mud->channel)	i3_to_char( "channel, ", ch );
   if (mud->news)	i3_to_char( "news, ", ch );
   if (mud->mail)	i3_to_char( "mail, ", ch );
   if (mud->file)	i3_to_char( "file, ", ch );
   if (mud->auth)	i3_to_char( "auth, ", ch );
   if (mud->ucache)	i3_to_char( "ucache, ", ch );
   i3_to_char( "\n\r", ch );

   i3_to_char( "Supports   : ", ch );
   if (mud->smtp)	i3_printf( ch, "smtp (port %d), ", mud->smtp );
   if (mud->http)	i3_printf( ch, "http (port %d), ", mud->http );
   if (mud->ftp)		i3_printf( ch, "ftp  (port %d), ", mud->ftp );
   if (mud->pop3)	i3_printf( ch, "pop3 (port %d), ", mud->pop3 );
   if (mud->nntp)	i3_printf( ch, "nntp (port %d), ", mud->nntp );
   if (mud->rcp)		i3_printf( ch, "rcp  (port %d), ", mud->rcp );
   if (mud->amrcp)	i3_printf( ch, "amrcp (port %d), ",mud->amrcp );
   i3_to_char( "\n\r", ch );
}

void I3_chanlayout( CHAR_DATA *ch, char *argument ) 
{
    I3_CHANNEL *channel = NULL;
    char arg1[SMST];
    char arg2[SMST];

    if( !argument || argument[0] == '\0' )
    {
	i3_to_char( "Usage: i3chanlayout <localchannel> <layout> <format...>\n\r", ch );
	i3_to_char( "Layout can be one of these: layout_e layout_m\n\r", ch );
	i3_to_char( "Format can be any way you want it to look, provided you have the proper number of %s tags in it.\n\r", ch );
	return;
    }

    argument = i3one_argument( argument, arg1 );
    argument = i3one_argument( argument, arg2 );

    if( !arg1 || arg1[0] == '\0' )
    {
	I3_chanlayout( ch, "" );
	return;
    }
    if( !arg2 || arg2[0] == '\0' )
    {
	I3_chanlayout( ch, "" );
	return;
    }
    if( !argument || argument[0] == '\0' )
    {
	I3_chanlayout( ch, "" );
	return;
    }

    if( !( channel = find_I3_channel_by_localname( arg1 ) ) ) 
    {
	    i3_to_char( "{YUnknown channel.\n\r"
	   "(use {Wi3chanlist{Y to get an overview of the channels available)\n\r", ch );
	return;
    }
    
    if( !str_cmp( arg2, "layout_e" ) )
    {
	if( !verify_i3layout( argument, 2 ) )
	{
	   i3_to_char( "Incorrect format for layout_e. You need exactly 2 %s's.\n\r", ch );
	   return;
	}
      I3STRFREE( channel->layout_e );
      channel->layout_e = I3STRALLOC( argument );
	i3_to_char( "Channel layout_e changed.\n\r", ch );
	I3_write_channel_config();
	return;
    }

    if( !str_cmp( arg2, "layout_m" ) )
    {
	if( !verify_i3layout( argument, 4 ) )
	{
	   i3_to_char( "Incorrect format for layout_m. You need exactly 4 %s's.\n\r", ch );
	   return;
	}
      I3STRFREE( channel->layout_m );
      channel->layout_m = I3STRALLOC( argument );
	i3_to_char( "Channel layout_m changed.\n\r", ch );
	I3_write_channel_config();
	return;
    }
    I3_chanlayout( ch, "" );
    return;
}

/* Free up all the data lists once the connection is down. No sense in wasting memory on it. */
void free_i3data( bool freerouters )
{
   I3_MUD *mud, *next_mud;
   I3_CHANNEL *channel, *next_chan;
   I3_BAN *ban, *next_ban;
   UCACHE_DATA *ucache, *next_ucache;
   ROUTER_DATA *router, *router_next;

   if( first_i3ban )
   {
      for( ban = first_i3ban; ban; ban = next_ban )
      {
         next_ban = ban->next;
         I3STRFREE( ban->name );
         I3UNLINK( ban, first_i3ban, last_i3ban, next, prev );
         I3DISPOSE( ban );
      }
   }

   if( first_I3chan )
   {
	for( channel = first_I3chan; channel; channel = next_chan )
	{
	   next_chan = channel->next;
	   destroy_I3_channel( channel );
	}
   }

   if( first_mud )
   {
	for( mud = first_mud; mud; mud = next_mud )
	{
	   next_mud = mud->next;
	   destroy_I3_mud( mud );
	}
   }

   if( first_ucache )
   {
	for( ucache = first_ucache; ucache; ucache = next_ucache )
	{
	   next_ucache = ucache->next;
	   I3STRFREE( ucache->name );
	   I3UNLINK( ucache, first_ucache, last_ucache, next, prev );
	   I3DISPOSE( ucache );
	}
   }

   if( first_router && freerouters == TRUE )
   {
      for( router = first_router; router; router = router_next )
      {
         router_next = router->next;
         I3STRFREE( router->name );
         I3STRFREE( router->ip );
         I3UNLINK( router, first_router, last_router, next, prev );
         I3DISPOSE( router );
      }
   }

   return;
}

void I3_disconnect( CHAR_DATA *ch, char *argument ) 
{
   if( !I3_is_connected() ) 
   {
	i3_to_char( "The MUD isn't connected to the Intermud-3 router.\n\r", ch );
	return;
   }

   i3_to_char( "Disconnecting from Intermud-3 router.\n\r", ch );

   I3_shutdown( 0 );
   return;
}

void I3_bancmd( CHAR_DATA *ch, char *argument )
{
   I3_BAN *temp;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "{GThe mud currently has the following ban list:\n\r\n\r", ch );

	if( !first_i3ban )
	{
	   i3_to_char( "{YNothing\n\r\n\r", ch );
	   i3_to_char( "{YTo add a ban, just specify a target. Suggested targets being user@mud or IP:Port\n\r", ch );
	   i3_to_char( "{YUser@mud bans can also have wildcard specifiers, such as *@Mud or User@*\n\r", ch );
	   return;
	}
      for( temp = first_i3ban; temp; temp = temp->next )
	   i3_printf( ch, "{Y\t  - %s\n\r", temp->name );
      return;
   }

   if( !fnmatch( argument, I3_THISMUD, 0 ) )
   {
	i3_to_char( "{YYou don't really want to do that....\n\r", ch );
	return;
   }

   for( temp = first_i3ban; temp; temp = temp->next )
   {
	if( !str_cmp( temp->name, argument ) )
	{
	   I3STRFREE( temp->name );
	   I3UNLINK( temp, first_i3ban, last_i3ban, next, prev );
	   I3DISPOSE( temp );
	   I3_write_bans();
	   i3_printf( ch, "{YThe mud no longer bans %s.\n\r", argument );
	   return;
	}
   }

   I3CREATE( temp, I3_BAN, 1 );
   temp->name = I3STRALLOC( argument );
   I3LINK( temp, first_i3ban, last_i3ban, next, prev );
   I3_write_bans();
   i3_printf( ch, "{YThe mud now bans all incoming traffic from %s.\n\r", temp->name );
}

void I3_ignorecmd( CHAR_DATA *ch, char *argument )
{
   I3_IGNORE *temp;
   char buf[SMST];

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "{GYou are currently ignoring the following:\n\r\n\r", ch );

	if( !FIRST_I3IGNORE(ch) )
      {
	   i3_to_char( "{YNobody\n\r\n\r", ch );
	   i3_to_char( "{YTo add an ignore, just specify a target. Suggested targets being user@mud or IP:Port\n\r", ch );
	   i3_to_char( "{YUser@mud ignores can also have wildcard specifiers, such as *@Mud or User@*\n\r", ch );
	   return;
	}
      for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
	   i3_printf( ch, "{Y\t  - %s\n\r", temp->name );

	return;
   }

   snprintf( buf, SMST, "%s@%s", CH_I3NAME(ch), I3_THISMUD );
   if( !str_cmp( buf, argument ) )
   {
	i3_to_char( "{YYou don't really want to do that....\n\r", ch );
	return;
   }

   if( !fnmatch( argument, I3_THISMUD, 0 ) )
   {
	i3_to_char( "{YIgnoring your own mud would be silly.\n\r", ch );
	return;
   }

   for( temp = FIRST_I3IGNORE(ch); temp; temp = temp->next )
   {
	if( !str_cmp( temp->name, argument ) )
	{
	   I3STRFREE( temp->name );
	   I3UNLINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
	   I3DISPOSE( temp );
	   i3_printf( ch, "{YYou are no longer ignoring %s.\n\r", argument );
	   return;
	}
   }

   I3CREATE( temp, I3_IGNORE, 1 );
   temp->name = I3STRALLOC( argument );
   I3LINK( temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev );
   i3_printf( ch, "{YYou now ignore %s.\n\r", temp->name );
}

void I3_color( CHAR_DATA *ch, char *argument )
{
   if( I3IS_SET( I3FLAG(ch), I3_COLOR ) )
   {
	I3REMOVE_BIT( I3FLAG(ch), I3_COLOR );
	i3_to_char( "I3 color is now off.\n\r", ch );
   }
   else
   {
	I3SET_BIT( I3FLAG(ch), I3_COLOR );
	i3_to_char( "{RI3 c{Yo{Gl{Bo{Mr {Ris now on.\n\r", ch );
   }
   return;
}

void I3_invis( CHAR_DATA *ch, char *argument )
{
   if( I3INVIS(ch) )
   {
	I3REMOVE_BIT( I3FLAG(ch), I3_INVIS );
	i3_to_char( "You are now i3visible.\n\r", ch );
   }
   else
   {
	I3SET_BIT( I3FLAG(ch), I3_INVIS );
	i3_to_char( "You are now i3invisible.\n\r", ch );
   }
   return;
}

void I3_debug( CHAR_DATA *ch, char *argument )
{
   packetdebug = !packetdebug;

   if( packetdebug )
      i3_to_char( "Packet debugging enabled.\n\r", ch );
   else
	i3_to_char( "Packet debugging disabled.\n\r", ch );

   return;
}

void I3_send_user_req( CHAR_DATA *ch, char *argument )
{
   char user[SMST], mud[SMST];
   char *ps;
   I3_MUD *pmud;

   if( !argument || argument[0] == '\0' ) 
   {
	i3_to_char( "{YQuery who at which mud?\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }
   if( !( ps = strchr( argument, '@' ) ) ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = '\0';
   i3strlcpy( user, argument, SMST );
   i3strlcpy( mud, ps+1, SMST );

   if( user[0] == '\0' || mud[0] == '\0' ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !( pmud = find_I3_mud_by_name( mud ) ) ) 
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   I3_send_chan_user_req( pmud->name, user );
   return;
}

void I3_process_channel_adminlist_reply( I3_HEADER *header, char *s )
{
   char *ps = s, *next_ps;
   I3_CHANNEL *channel;
   CHAR_DATA *ch;

   if( ( ch = I3_find_user( header->target_username ) ) == NULL ) 
   {
	i3bug( "I3_process_channel_adminlist_reply(): user %s not found.", header->target_username );
	return;
   }

   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   if( !( channel = find_I3_channel_by_name( ps ) ) )
   {
      i3bug( "I3_process_channel_adminlist_reply(): Invalid local channel %s reply received.", ps );
      return;
   }
   i3_printf( ch, "{RThe following muds are %s %s:\n\r\n\r", channel->status == 0 ? "banned from" : "invited to", channel->local_name );

   ps = next_ps;
   I3_get_field( ps, &next_ps );
   ps += 2;
   while( 1 ) 
   {
	if( ps[0] == '}' ) 
      {
	   i3_to_char( "{YNo entries found.\n\r", ch );
	   return;
	}

	I3_get_field( ps, &next_ps );
	I3_remove_quotes( &ps );
	i3_printf( ch, "{Y%s\n\r", ps );

	ps = next_ps;
	if( ps[0]== '}' )
	    break;
   }
   return;
}

void I3_send_channel_adminlist( CHAR_DATA *ch, char *chan_name )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "chan-adminlist", I3_THISMUD, CH_I3NAME(ch), I3_ROUTER_NAME, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( chan_name );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );

   return;
}

void I3_send_channel_admin( CHAR_DATA *ch, char *chan_name, char *list )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-admin", I3_THISMUD, CH_I3NAME(ch), I3_ROUTER_NAME, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( chan_name );
   I3_write_buffer( "\"," );
   I3_write_buffer( list );
   I3_write_buffer( "})\r" );
   I3_send_packet( );

   return;
}

void I3_admin_channel( CHAR_DATA *ch, char *argument )
{
   I3_CHANNEL *channel = NULL;
   char arg1[SMST], arg2[SMST], buf[LGST];

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3adminchan <localchannel> <add|remove> <mudname>\n\r", ch );
      i3_to_char( "Usage: i3adminchan <localchannel> list\n\r", ch );
	return;
   }
   argument = i3one_argument( argument, arg1 );
   argument = i3one_argument( argument, arg2 );

   if( !arg1 || arg1[0] == '\0' )
   {
      I3_admin_channel( ch, "" );
	return;
   }

   if( !( channel = find_I3_channel_by_localname( arg1 ) ) )
   {
	i3_to_char( "No such channel with that name here.\n\r", ch );
	return;
   }

   if( !arg2 || arg2[0] == '\0' )
   {
	I3_admin_channel( ch, "" );
	return;
   }

   if( !str_cmp( arg2, "list" ) )
   {
      I3_send_channel_adminlist( ch, channel->I3_name );
      i3_to_char( "Sending request for administrative list.\n\r", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
	I3_admin_channel( ch, "" );
	return;
   }

   if( !str_cmp( arg2, "add" ) )
   {
      snprintf( buf, LGST, "({\"%s\",}),({}),", argument );
	I3_send_channel_admin( ch, channel->I3_name, buf );
	i3_to_char( "Sending administrative list addition.\n\r", ch );
	return;
   }

   if( !str_cmp( arg2, "remove" ) )
   {
	snprintf( buf, LGST, "({}),({\"%s\",}),", argument );
	I3_send_channel_admin( ch, channel->I3_name, buf );
	i3_to_char( "Sending administrative list removal.\n\r", ch );
	return;
   }
   I3_admin_channel( ch, "" );
   return;
}

void I3_send_channel_add( CHAR_DATA *ch, char *arg, int type )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-add", I3_THISMUD, CH_I3NAME(ch), I3_ROUTER_NAME, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( arg );
   I3_write_buffer( "\"," );
   switch( type )
   {
	default:
	   i3bug( "%s", "I3_send_channel_add: Illegal channel type!" );
	   return;
	case 0:
	   I3_write_buffer( "0,})\r" );
	   break;
	case 1:
	   I3_write_buffer( "1,})\r" );
	   break;
	case 2:
	   I3_write_buffer( "2,})\r" );
	   break;
   }
   I3_send_packet( );
   return;
}

void I3_addchan( CHAR_DATA *ch, char *argument )
{
   I3_CHANNEL *channel;
   char arg[SMST], arg2[SMST], buf[LGST];
   int type, x;

   argument = i3one_argument( argument, arg );
   argument = i3one_argument( argument, arg2 );

   if( !argument || argument[0] == '\0' || !arg || arg[0] == '\0' || !arg2 || arg2[0] == '\0' )
   {
	i3_to_char( "Usage: i3addchan <channelname> <localname> <type>\n\r\n\r", ch );
	i3_to_char( "Channelname should be the name seen on 'chanlist all'\n\r", ch );
	i3_to_char( "Localname should be the local name you want it listed as.\n\r", ch );
      i3_to_char( "Type can be one of the following:\n\r\n\r", ch );
	i3_to_char( "0: selectively banned\n\r", ch );
	i3_to_char( "1: selectively admitted\n\r", ch );
	i3_to_char( "2: filtered - valid for selectively admitted ONLY\n\r", ch );
	return;
   }

   if( ( channel = find_I3_channel_by_name( arg ) ) != NULL )
   {
	i3_printf( ch, "{R%s is already hosted by %s.\n\r", channel->I3_name, channel->host_mud );
	return;
   }

   if( ( channel = find_I3_channel_by_localname( arg2 ) ) != NULL )
   {
	i3_printf( ch, "{RChannel %s@%s is already locally configured as %s.\n\r",
	   channel->I3_name, channel->host_mud, channel->local_name );
	return;
   }

   if( !isdigit( argument[0] ) )
   {
	i3_to_char( "{RInvalid type. Must be numerical.\n\r", ch );
	I3_addchan( ch, "" );
	return;
   }

   type = atoi( argument );
   if( type < 0 || type > 2 )
   {
	i3_to_char( "{RInvalid channel type.\n\r", ch );
	I3_addchan( ch, "" );
	return;
   }

   i3_printf( ch, "{GAdding channel to router: {W%s\n\r", arg );
   I3_send_channel_add( ch, arg, type );

   I3CREATE( channel, I3_CHANNEL, 1 );
   channel->I3_name = I3STRALLOC( arg );
   channel->host_mud = I3STRALLOC( I3_THISMUD );
   channel->local_name = I3STRALLOC( arg2 );
   channel->i3perm = I3PERM_ADMIN;
   channel->layout_m = I3STRALLOC( "{R[{W%s{R] {C%s@%s: {c%s" );
   channel->layout_e = I3STRALLOC( "{R[{W%s{R] {c%s" );
   for( x = 0; x < MAX_I3HISTORY; x++ )
	channel->history[x] = NULL;
   I3LINK( channel, first_I3chan, last_I3chan, next, prev );

   if( type != 0 )
   {
      snprintf( buf, LGST, "({\"%s\",}),({}),", I3_THISMUD );
      I3_send_channel_admin( ch, channel->I3_name, buf );
      i3_printf( ch, "{GSending command to add %s to the invite list.\n\r", I3_THISMUD );
   }

   i3_printf( ch, "{Y%s@%s {Wis now locally known as {Y%s\n\r", channel->I3_name, channel->host_mud, channel->local_name );
   I3_send_channel_listen( channel, TRUE );
   I3_write_channel_config();

   return;
}

void I3_send_channel_remove( CHAR_DATA *ch, I3_CHANNEL *channel )
{
   if( !I3_is_connected() )
	return;

   I3_write_header( "channel-remove", I3_THISMUD, CH_I3NAME(ch), I3_ROUTER_NAME, NULL );
   I3_write_buffer( "\"" );
   I3_write_buffer( channel->I3_name );
   I3_write_buffer( "\",})\r" );
   I3_send_packet( );
   return;
}

void I3_removechan( CHAR_DATA *ch, char *argument )
{
   I3_CHANNEL *channel = NULL;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "{YUsage: i3removechan <channel>\n\r", ch );
	i3_to_char( "{WChannelname should be the name seen on 'chanlist all'\n\r", ch );
	return;
   }

   if( ( channel = find_I3_channel_by_name( argument ) ) == NULL )
   {
	i3_to_char( "{RNo channel by that name exists.\n\r", ch );
	return;
   }

   if( str_cmp( channel->host_mud, I3_THISMUD ) )
   {
	i3_printf( ch, "{R%s does not host this channel and cannot remove it.\n\r", I3_THISMUD );
	return;
   }

   i3_printf( ch, "{YRemoving channel from router: {W%s\n\r", channel->I3_name );
   I3_send_channel_remove( ch, channel );

   i3_printf( ch, "{RDestroying local channel entry for {W%s\n\r", channel->I3_name );
   destroy_I3_channel( channel );
   I3_write_channel_config();

   return;
}

void I3_saveconfig( void )
{
   ROUTER_DATA *router;
   FILE *fp;

   if( !( fp = fopen( I3_CONFIG_FILE, "w" ) ) ) 
   {
	i3log( "%s", "Couldn't write to i3.config file." );
	return;
   }

   fprintf( fp, "%s", "$I3CONFIG\n\n" );
   fprintf( fp, "%s", "# When changing this information, be sure you don't remove the tildes!\n" );
   fprintf( fp, "%s", "# Set autoconnect to 1 to automatically connect at bootup.\n" );
   fprintf( fp, "%s", "# This information can be edited online using 'i3 config'\n" );
   fprintf( fp, "thismud      %s~\n", this_mud->name );
   fprintf( fp, "autoconnect  %d\n",  this_mud->autoconnect );
   fprintf( fp, "telnet       %s~\n", this_mud->telnet );
   fprintf( fp, "web          %s~\n", this_mud->web );
   fprintf( fp, "adminemail   %s~\n", this_mud->admin_email );
   fprintf( fp, "openstatus   %s~\n", this_mud->open_status );
   fprintf( fp, "mudtype      %s~\n", this_mud->mud_type );
   fprintf( fp, "basemudlib   %s~\n", this_mud->base_mudlib );
   fprintf( fp, "mudlib       %s~\n", this_mud->mudlib );
   fprintf( fp, "minlevel     %d\n",  this_mud->minlevel );
   fprintf( fp, "immlevel     %d\n",  this_mud->immlevel );
   fprintf( fp, "adminlevel   %d\n",  this_mud->adminlevel );
   fprintf( fp, "implevel     %d\n\n",  this_mud->implevel );

   fprintf( fp, "%s", "# Router connections. Your primary choice should be listed first.\n" );
   for( router = first_router; router; router = router->next )
      fprintf( fp, "router %s %s %d\n", router->name, router->ip, router->port );
   
   fprintf( fp, "%s", "\n# Information below this point cannot be edited online.\n" );
   fprintf( fp, "%s", "# The services provided by your mud.\n" );
   fprintf( fp, "%s", "# Do not turn things on unless you KNOW your mud properly supports them!\n" );
   fprintf( fp, "%s", "# Refer to http://cie.imaginary.com/protocols/intermud3.html for public packet specifications.\n" );
   fprintf( fp, "tell         %d\n", this_mud->tell );
   fprintf( fp, "beep         %d\n", this_mud->beep );
   fprintf( fp, "emoteto      %d\n", this_mud->emoteto );
   fprintf( fp, "who          %d\n", this_mud->who );
   fprintf( fp, "finger       %d\n", this_mud->finger );
   fprintf( fp, "locate       %d\n", this_mud->locate );
   fprintf( fp, "channel      %d\n", this_mud->channel );
   fprintf( fp, "news         %d\n", this_mud->news );
   fprintf( fp, "mail         %d\n", this_mud->mail );
   fprintf( fp, "file         %d\n", this_mud->file );
   fprintf( fp, "auth         %d\n", this_mud->auth );
   fprintf( fp, "ucache       %d\n\n", this_mud->ucache );
   fprintf( fp, "%s", "# Port numbers for OOB services. Leave as 0 if your mud does not support these.\n" );
   fprintf( fp, "smtp         %d\n", this_mud->smtp );
   fprintf( fp, "ftp          %d\n", this_mud->ftp );
   fprintf( fp, "nntp         %d\n", this_mud->nntp );
   fprintf( fp, "http         %d\n", this_mud->http );
   fprintf( fp, "pop3         %d\n", this_mud->pop3 );
   fprintf( fp, "rcp          %d\n", this_mud->rcp );
   fprintf( fp, "amrcp        %d\n", this_mud->amrcp );
   fprintf( fp, "%s", "end\n" );
   fprintf( fp, "%s", "$END\n" );
   I3FCLOSE( fp );
   return;
}

void I3_setconfig( CHAR_DATA *ch, char *argument )
{
   char arg[SMST];

   argument = i3one_argument( argument, arg );

   if( !arg || arg[0] == '\0' )
   {
	i3_to_char( "{GConfiguration info for your mud. Changes save when edited.\n\r", ch );
	i3_to_char( "{GYou can set the following:\n\r\n\r", ch );
	i3_to_char( "{wShow       : {GDisplays your current congfiguration.\n\r", ch );
	i3_to_char( "{wAutoconnect: {GA toggle. Either on or off. Your mud will connect automatically with it on.\n\r", ch );
	i3_to_char( "{wMudname    : {GThe name you want displayed on I3 for your mud.\n\r", ch );
	i3_to_char( "{wTelnet     : {GThe telnet address for your mud. Do not include the port number.\n\r", ch );
	i3_to_char( "{wWeb        : {GThe website address for your mud. In the form of: www.address.com\n\r", ch );
	i3_to_char( "{wEmail      : {GThe email address of your mud's administrator. Needs to be valid!!\n\r", ch );
	i3_to_char( "{wStatus     : {GThe open status of your mud. IE: Public, Development, etc.\n\r", ch );
	i3_to_char( "{wMudtype    : {GWhat type of mud you have. Diku, Rom, Smaug, Merc, etc.\n\r", ch );
	i3_to_char( "{wBaselib    : {GThe base version of the codebase you have.\n\r", ch );
	i3_to_char( "{wMudlib     : {GWhat you call the current version of your codebase.\n\r", ch );
	i3_to_char( "{wMinlevel   : {GMinimum level at which I3 will recognize your players.\n\r", ch );
	i3_to_char( "{wImmlevel   : {GThe level at which immortal commands become available.\n\r", ch );
	i3_to_char( "{wAdminlevel : {GThe level at which administrative commands become available.\n\r", ch );
	i3_to_char( "{wImplevel   : {GThe level at which implementor commands become available.\n\r", ch );
	return;
   }

   if( !str_cmp( arg, "show" ) )
   {
	i3_printf( ch, "{wMudname       : {G%s\n\r", this_mud->name );
	i3_printf( ch, "{wAutoconnect   : {G%s\n\r", this_mud->autoconnect == TRUE ? "Enabled" : "Disabled" );
	i3_printf( ch, "{wTelnet        : {G%s:%d\n\r", this_mud->telnet, this_mud->player_port );
	i3_printf( ch, "{wWeb           : {G%s\n\r", this_mud->web );
	i3_printf( ch, "{wEmail         : {G%s\n\r", this_mud->admin_email );
	i3_printf( ch, "{wStatus        : {G%s\n\r", this_mud->open_status );
	i3_printf( ch, "{wMudtype       : {G%s\n\r", this_mud->mud_type );
	i3_printf( ch, "{wBaselib       : {G%s\n\r", this_mud->base_mudlib );
	i3_printf( ch, "{wMudlib        : {G%s\n\r", this_mud->mudlib );
	i3_printf( ch, "{wMinlevel      : {G%d\n\r", this_mud->minlevel );
	i3_printf( ch, "{wImmlevel      : {G%d\n\r", this_mud->immlevel );
	i3_printf( ch, "{wAdminlevel    : {G%d\n\r", this_mud->adminlevel );
	i3_printf( ch, "{wImplevel      : {G%d\n\r", this_mud->implevel );
	return;
   }

   if( !str_cmp( arg, "autoconnect" ) )
   {
	this_mud->autoconnect = !this_mud->autoconnect;

	if( this_mud->autoconnect )
	   i3_to_char( "Autoconnect enabled.\n\r", ch );
	else
	   i3_to_char( "Autoconnect disabled.\n\r", ch );
	I3_saveconfig( );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	I3_setconfig( ch, "" );
	return;
   }

   if( !str_cmp( arg, "implevel" ) && I3PERM(ch) == I3PERM_IMP )
   {
	int value = atoi( argument );

	this_mud->implevel = value;
	I3_saveconfig( );
	i3_printf( ch, "Implementor level changed to %d\n\r", value );
	return;
   }

   if( !str_cmp( arg, "adminlevel" ) )
   {
	int value = atoi( argument );

	this_mud->adminlevel = value;
	I3_saveconfig( );
	i3_printf( ch, "Admin level changed to %d\n\r", value );
	return;
   }

   if( !str_cmp( arg, "immlevel" ) )
   {
	int value = atoi( argument );

	this_mud->immlevel = value;
	I3_saveconfig( );
	i3_printf( ch, "Immortal level changed to %d\n\r", value );
	return;
   }

   if( !str_cmp( arg, "minlevel" ) )
   {
	int value = atoi( argument );

	this_mud->minlevel = value;
	I3_saveconfig( );
	i3_printf( ch, "Minimum level changed to %d\n\r", value );
	return;
   }

   if( I3_is_connected() )
   {
	i3_printf( ch, "%s may not be changed while the mud is connected.\n\r", arg );
	return;
   }

   if( !str_cmp( arg, "mudname" ) )
   {
	I3STRFREE( this_mud->name );
	this_mud->name = I3STRALLOC( argument );
	I3_THISMUD = argument;
	unlink( I3_PASSWORD_FILE );
	I3_saveconfig( );
	i3_printf( ch, "Mud name changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "telnet" ) )
   {
	I3STRFREE( this_mud->telnet );
	this_mud->telnet = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Telnet address changed to %s:%d\n\r", argument, this_mud->player_port );
	return;
   }

   if( !str_cmp( arg, "web" ) )
   {
	I3STRFREE( this_mud->web );
	this_mud->web = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Website changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "email" ) )
   {
	I3STRFREE( this_mud->admin_email );
	this_mud->admin_email = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Admin email changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "status" ) )
   {
	I3STRFREE( this_mud->open_status );
	this_mud->open_status = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Status changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "mudtype" ) )
   {
	I3STRFREE( this_mud->mud_type );
	this_mud->mud_type = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Mud type changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "baselib" ) )
   {
	I3STRFREE( this_mud->base_mudlib );
	this_mud->base_mudlib = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Base mudlib changed to %s\n\r", argument );
	return;
   }

   if( !str_cmp( arg, "mudlib" ) )
   {
	I3STRFREE( this_mud->mudlib );
	this_mud->mudlib = I3STRALLOC( argument );
	I3_saveconfig( );
	i3_printf( ch, "Mudlib changed to %s\n\r", argument );
	return;
   }

   I3_setconfig( ch, "" );
   return;
}

void I3_permstats( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3perms <user>\n\r", ch );
	return;
   }

   if( !( victim = I3_find_user( argument ) ) )
         {
	i3_to_char( "No such person is currently online.\n\r", ch );
	return;
	   }

   if( I3PERM(victim) < 0 || I3PERM(victim) > I3PERM_IMP )
   {
	i3_printf( ch, "{R%s has an invalid permission setting!\n\r", CH_I3NAME(victim) );
	return;
   }

   i3_printf( ch, "{YPermissions for %s: %s\n\r", CH_I3NAME(victim), perm_names[I3PERM(victim)] );
   return;
}

void I3_permset( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[SMST];
   int permvalue;

   argument = i3one_argument( argument, arg );

   if( !arg || arg[0] == '\0' )
   {
	i3_to_char( "Usage: i3permset <user> <permission>\n\r", ch );
	i3_to_char( "Permission can be one of: None, Mort, Imm, Admin, Imp\n\r", ch );
	return;
   }

   if( !( victim = I3_find_user( arg ) ) )
   {
	i3_to_char( "No such person is currently online.\n\r", ch );
	return;
   }

   permvalue = get_permvalue( argument );

   if( permvalue < 0 || permvalue > I3PERM_IMP )
   {
	i3_to_char( "Invalid permission setting.\n\r", ch );
	return;
   }

   if( permvalue > I3PERM(ch) )
   {
	i3_to_char( "You cannot set permissions higher than your own.\n\r", ch );
	return;
   }

   if( permvalue == I3PERM(ch) && I3PERM(ch) != I3PERM_IMP )
   {
	i3_to_char( "You cannot set permissions equal to your own. Someone higher up must do this.\n\r", ch );
	return;
   }

   if( I3PERM(ch) < I3PERM(victim) )
   {
	i3_to_char( "You cannot reduce the permissions of someone above your own.\n\r", ch );
	return;
   }

   I3PERM(victim) = permvalue;
   i3_printf( ch, "{YPermission level for %s has been changed to %s\n\r", CH_I3NAME(victim), perm_names[permvalue] );
   return;
}

void I3_who( CHAR_DATA *ch, char *argument )
{
   I3_MUD *mud;

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "{YGet an overview of which mud?\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !( mud = find_I3_mud_by_name( argument ) ) )
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( mud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", mud->name );
	return;
   }

   if( mud->who == 0 )
	i3_printf( ch, "%s does not support the 'who' command. Sending anyway.\n\r", mud->name );

   I3_send_who( ch, mud->name );
}

void I3_locate( CHAR_DATA *ch, char *argument )
{
   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Locate who?\n\r", ch );
	return;
   }
   I3_send_locate( ch, argument );
}

void I3_finger( CHAR_DATA *ch, char *argument )
{
   char user[SMST], mud[SMST];
   char *ps;
   I3_MUD *pmud;

   if( I3IS_SET( I3FLAG(ch), I3_DENYFINGER ) )
   {
	i3_to_char( "You are not allowed to use i3finger.\n\r", ch );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_to_char( "Usage: i3finger <user@mud>\n\r", ch );
	i3_to_char( "Usage: i3finger privacy\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "privacy" ) )
   {
	if( !I3IS_SET( I3FLAG(ch), I3_PRIVACY ) )
      {
	   I3SET_BIT( I3FLAG(ch), I3_PRIVACY );
	   i3_to_char( "I3 finger privacy flag set.\n\r", ch );
	   return;
	}
	I3REMOVE_BIT( I3FLAG(ch), I3_PRIVACY );
	i3_to_char( "I3 finger privacy flag removed.\n\r", ch );
	return;
   }

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are invisible.\n\r", ch );
	return;
   }

   if( ( ps = strchr( argument, '@' ) ) == NULL ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = '\0';
   i3strlcpy( user, argument, SMST );
   i3strlcpy( mud, ps+1, SMST );

   if( user[0] == '\0' || mud[0] == '\0' ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !( pmud = find_I3_mud_by_name( mud ) ) ) 
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( !str_cmp( I3_THISMUD, pmud->name ) )
   {
	i3_to_char( "Use your mud's own internal system for that.\n\r", ch );
	return;
   }

   if( pmud->status >= 0 ) 
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->finger == 0 )
	i3_printf( ch, "%s does not support the 'finger' command. Sending anyway.\n\r", pmud->name );

   I3_send_finger( ch, user, pmud->name );
}

void I3_emote( CHAR_DATA *ch, char *argument )
{
   char to[SMST], *ps;
   char mud[SMST];
   I3_MUD *pmud;

   if( I3ISINVIS(ch) )
   {
	i3_to_char( "You are invisible.\n\r", ch );
	return;
   }

   argument = i3one_argument( argument, to );
   ps = strchr( to, '@' );

   if( to[0] == '\0' || argument[0] == '\0' || ps == NULL ) 
   {
	i3_to_char( "{YYou should specify a person and a mud.\n\r"
	    "(use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch);
	return;
   }

   ps[0] = '\0';
   ps++;
   i3strlcpy( mud, ps, SMST );

   if( !( pmud = find_I3_mud_by_name( mud ) ) ) 
   {
	i3_to_char( "{YNo such mud known.\n\r"
	    "( use {Wi3mudlist{Y to get an overview of the muds available)\n\r", ch );
	return;
   }

   if( pmud->status >= 0 )  
   {
	i3_printf( ch, "%s is marked as down.\n\r", pmud->name );
	return;
   }

   if( pmud->emoteto == 0 )
	i3_printf( ch, "%s does not support the 'emoteto' command. Sending anyway.\n\r", pmud->name );

   I3_send_emoteto( ch, to, pmud, argument );
}

/*
 * Setup a TCP session to the router. Returns socket or <0 if failed.
 *
 */
int I3_connection_open( ROUTER_DATA *router )
{
   struct sockaddr_in sa;
   struct hostent *hostp;
   int x = 1;

   i3log( "Attempting connect to %s on port %d", router->ip, router->port );

   I3_socket = socket( AF_INET, SOCK_STREAM, 0 );
   if( I3_socket < 0 )
   {
	i3log( "%s", "Cannot create socket!" );
      I3_connection_close( TRUE );
	return -1;
   }

   if( setsockopt( I3_socket, SOL_SOCKET, SO_KEEPALIVE, (void *) &x, sizeof(x) ) < 0 )
   {
	perror( "I3_connection_open: SO_KEEPALIVE" );
      I3_connection_close( TRUE );
	return -1;
   }

   if( ( x = fcntl( I3_socket, F_GETFL, 0 ) ) < 0 )
   {
      i3log( "%s", "I3_connection_open: fcntl(F_GETFL)" );
      I3_connection_close( TRUE );
      return -1;
   }

   if( fcntl( I3_socket, F_SETFL, x | O_NONBLOCK ) < 0 )
   {
      i3log( "%s", "I3_connection_open: fcntl(F_SETFL)" );
      I3_connection_close( TRUE );
      return -1;
   }

   memset( &sa, 0, sizeof( sa ) );
   sa.sin_family = AF_INET;

   if ( !inet_aton( router->ip, &sa.sin_addr ) )
   {
	hostp = gethostbyname( router->ip );
	if( !hostp )
      {
	   i3log( "%s", "I3_connection_open: Cannot resolve router hostname." );
         I3_connection_close( TRUE );
	   return -1;
	}
	memcpy( &sa.sin_addr, hostp->h_addr, hostp->h_length );
   }

   sa.sin_port = htons( router->port );

   if( connect( I3_socket, (struct sockaddr *)&sa, sizeof(sa) ) < 0 )
   {
	if( errno != EINPROGRESS )
	{
	   i3log( "I3_connection_open: Unable to connect to router %s", router->name );
         I3_connection_close( TRUE );
	   return -1;
	}
   }
   I3_ROUTER_NAME = router->name;
   i3log( "Connected to Intermud-3 router %s", router->name );
   return I3_socket;
}

/* Be careful with an IP ban - Last resort measure, etc. */
bool i3banned( const char *ignore )
{
   I3_BAN *temp;
   I3_MUD *mud;
   char *ps;
   char mudname[SMST], ipbuf[512];

   /* Wildcard support thanks to Xorith */
   for( temp = first_i3ban; temp; temp = temp->next )
   {
	if( !fnmatch( temp->name, ignore, 0 ) )
	   return TRUE;
   }

   /* In theory, getting this far should be the result of an IP:Port ban */
   ps = strchr( ignore, '@' );

   if( !ignore || ignore[0] == '\0' || ps == NULL ) 
	return FALSE;

   ps[0] = '\0';
   i3strlcpy( mudname, ps+1, SMST );

   for( mud = first_mud; mud; mud = mud->next )
   {
	if( !str_cmp( mud->name, mudname ) )
	{
	   snprintf( ipbuf, 512, "%s:%d", mud->ipaddress, mud->player_port );
	   for( temp = first_i3ban; temp; temp = temp->next )
	   {
		if( !str_cmp( temp->name, ipbuf ) )
		   return TRUE;
	   }
	}
   }
   return FALSE;
}

/*
 * Read the header of an I3 packet. pps will point to the next field
 * of the packet.
 */
I3_HEADER *I3_get_header( char **pps )
{
   I3_HEADER *header;
   char *ps = *pps, *next_ps;

   I3CREATE( header, I3_HEADER, 1 );

   I3_get_field( ps, &next_ps );
   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( header->originator_mudname, ps, 256 );
   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( header->originator_username, ps, 256 );
   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( header->target_mudname, ps, 256 );
   ps = next_ps;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( header->target_username, ps, 256 );

   *pps = next_ps;
   return header;
}

/*
 * Read one I3 packet into the I3_input_buffer
 */
void I3_read_packet( void )
{
   long size;

   memmove( &size, I3_input_buffer, 4 );
   size = ntohl( size );

   memmove( I3_currentpacket, I3_input_buffer + 4, size );

   if( I3_currentpacket[size-1] != ')' )
      I3_currentpacket[size-1] = 0;
   I3_currentpacket[size] = 0;

   memmove( I3_input_buffer, I3_input_buffer + size + 4, I3_input_pointer - size - 4 );
   I3_input_pointer -= size + 4;
   return;
}

/*
 * Read the first field of an I3 packet and call the proper function to
 * process it. Afterwards the original I3 packet is completly messed up.
 *
 * Reworked on 9-5-03 by Samson to be made more efficient with regard to banned muds.
 * Also only need to gather the header information once this way instead of in multiple places.
 */
void I3_parse_packet( void )
{
   I3_HEADER *header = NULL;
   char *ps, *next_ps;
   char ptype[SMST];

   ps = I3_currentpacket;
   if( ps[0] != '(' || ps[1] != '{' )
   	return;

   if( packetdebug )
      i3log( "Packet received: %s", ps );

   ps += 2;
   I3_get_field( ps, &next_ps );
   I3_remove_quotes( &ps );
   i3strlcpy( ptype, ps, SMST );

   header = I3_get_header( &ps );

   /* There. Nice and simple, no? */
   if( i3banned( header->originator_mudname ) )
      return;

   if( !str_cmp( ptype, "tell" ) ) 
	I3_process_tell( header, ps );

   if( !str_cmp( ptype, "beep" ) )
	I3_process_beep( header, ps );

   if( !str_cmp( ptype, "emoteto" ) ) 
	I3_process_emoteto( header, ps );

   if( !str_cmp( ptype, "channel-m" ) ) 
	I3_process_channel_m( header, ps );

   if( !str_cmp( ptype, "channel-e" ) ) 
	I3_process_channel_e( header, ps );

   if( !str_cmp( ptype, "finger-req" ) ) 
	I3_process_finger_req( header, ps );

   if( !str_cmp( ptype, "finger-reply" ) ) 
	I3_process_finger_reply( header, ps );

   if( !str_cmp( ptype, "locate-req" ) ) 
	I3_process_locate_req( header, ps );

   if( !str_cmp( ptype, "locate-reply" ) ) 
	I3_process_locate_reply( header, ps );

   if( !str_cmp( ptype, "chan-who-req" ) ) 
	I3_process_chan_who_req( header, ps );

   if( !str_cmp( ptype, "chan-who-reply" ) ) 
	I3_process_chan_who_reply( header, ps );

   if( !str_cmp( ptype, "chan-adminlist-reply" ) )
      I3_process_channel_adminlist_reply( header, ps );

   if( !str_cmp( ptype, "ucache-update" ) && this_mud->ucache == TRUE )
	I3_process_ucache_update( header, ps );

   if( !str_cmp( ptype, "who-req" ) ) 
	I3_process_who_req( header, ps );

   if( !str_cmp( ptype, "who-reply" ) ) 
	I3_process_who_reply( header, ps );

   if( !str_cmp( ptype, "chanlist-reply" ) ) 
	I3_process_chanlist_reply( header, ps );

   if( !str_cmp( ptype, "startup-reply" ) ) 
	I3_process_startup_reply( header, ps );

   if( !str_cmp( ptype, "mudlist" ) ) 
	I3_process_mudlist( header, ps );

   if( !str_cmp( ptype, "error" ) ) 
	I3_process_error( header, ps );

   if( !str_cmp( ptype, "chan-ack" ) )
	I3_process_chanack( header, ps );

   if( !str_cmp( ptype, "channel-t" ) )
	I3_process_channel_t( header, ps );

   if( !str_cmp( ptype, "chan-user-req" ) )
	I3_process_chan_user_req( header, ps );

   if( !str_cmp( ptype, "chan-user-reply" ) && this_mud->ucache == TRUE )
	I3_process_chan_user_reply( header, ps );

   if( !str_cmp( ptype, "router-shutdown" ) )
   {
      int delay;

      I3_get_field( ps, &next_ps );
      delay = atoi( ps );

      if( delay == 0 )
      {
         i3log( "Router %s is shutting down.", I3_ROUTER_NAME );
         I3_connection_close( FALSE );
      }
      else
      {
         i3log( "Router %s is rebooting and will be back in %d second%s.",
            I3_ROUTER_NAME, delay, delay == 1 ? "" : "s" );
         I3_connection_close( TRUE );
      }
   }

   I3DISPOSE( header );
   return;
}

/* Used only during copyovers */
void fread_mudlist( FILE *fin, I3_MUD *mud )
{
   const char *word;
   char *ln;
   bool fMatch;
   int x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12;

   for ( ; ; )
   {
	word   = feof( fin ) ? "End" : i3fread_word( fin );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	   case '*':
	      fMatch = TRUE;
	      i3fread_to_eol( fin );
	      break;

	   case 'B':
            I3KEY( "Banner",          mud->banner,            i3fread_string( fin ) );
		I3KEY( "Baselib",		mud->base_mudlib,		i3fread_string( fin ) );
		break;

	   case 'D':
            I3KEY( "Daemon",          mud->daemon,            i3fread_string( fin ) );
		I3KEY( "Driver",		mud->driver,		i3fread_string( fin ) );
		break;

	   case 'E':
		I3KEY( "Email",		mud->admin_email,		i3fread_string( fin ) );
	      if ( !str_cmp( word, "End" ) )
		{
		   return;
		}
	   case 'I':
		I3KEY( "IP",			mud->ipaddress,		i3fread_string( fin ) );
		break;

	   case 'M':
		I3KEY( "Mudlib",		mud->mudlib,		i3fread_string( fin ) );
	      break;

	   case 'O':
		I3KEY( "Openstatus",	mud->open_status,		i3fread_string( fin ) );
		if( !str_cmp( word, "OOBPorts" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=x4=x5=x6=x7=0;

	         sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		   mud->smtp  = x1;
		   mud->ftp   = x2;
		   mud->nntp  = x3;
		   mud->http  = x4;
		   mud->pop3  = x5;
		   mud->rcp   = x6;
		   mud->amrcp = x7;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'P':
		if( !str_cmp( word, "Ports" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=0;

	         sscanf( ln, "%d %d %d ", &x1, &x2, &x3 );
		   mud->player_port   = x1;
		   mud->imud_tcp_port = x2;
		   mud->imud_udp_port = x3;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'S':
		I3KEY( "Status",		mud->status,		i3fread_number( fin ) );
		if( !str_cmp( word, "Services" ) )
		{
	         ln = i3fread_line( fin );
	         x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=0;

	         sscanf( ln, "%d %d %d %d %d %d %d %d %d %d %d %d", 
			&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12 );
		   mud->tell    = x1;
		   mud->beep    = x2;
		   mud->emoteto = x3;
		   mud->who     = x4;
		   mud->finger  = x5;
		   mud->locate  = x6;
		   mud->channel = x7;
		   mud->news    = x8;
		   mud->mail    = x9;
		   mud->file    = x10;
		   mud->auth    = x11;
		   mud->ucache  = x12;
		   fMatch = TRUE;
		   break;
		}
		break;

	   case 'T':
		I3KEY( "Telnet",		mud->telnet,		i3fread_string( fin ) );
            I3KEY( "Time",            mud->time,              i3fread_string( fin ) );
		I3KEY( "Type",		mud->mud_type,		i3fread_string( fin ) );
		break;

	   case 'W':
		I3KEY( "Web",			mud->web,			i3fread_string( fin ) );
		break;		
	}

	if ( !fMatch )
	    i3bug( "I3_readmudlist: no match: %s", word );
   }
}

/* Called only during copyovers */
void I3_loadmudlist( void )
{
   FILE *fin;
   I3_MUD *mud;

   if( !( fin = fopen( I3_MUDLIST_FILE, "r" ) ) ) 
	return;

   for( ; ; )
   {
	char letter;
	const char *word;

	letter = i3fread_letter( fin );
	if( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if( letter != '#' )
	{
	   i3bug( "%s", "I3_loadmudlist: # not found." );
	   break;
	}

      word = i3fread_word( fin );
      if( !str_cmp( word, "ROUTER" ) )
      {
         I3STRFREE( this_mud->routerName );
         this_mud->routerName = i3fread_string( fin );
         I3_ROUTER_NAME = this_mud->routerName;
         continue;
      }
	if( !str_cmp( word, "MUDLIST" ) )
	{
	   word = i3fread_word( fin );
	   if( !str_cmp( word, "Name" ) )
	   {
		char *tmpname;

		tmpname = i3fread_string( fin );
		mud = new_I3_mud( tmpname );
		fread_mudlist( fin, mud );
            I3STRFREE( tmpname );
	   }
	   else
	   {
	      i3bug( "%s", "fread_mudlist: No mudname saved, skipping entry." );
		i3fread_to_eol( fin );
		for( ; ; )
		{
               word   = feof( fin ) ? "End" : i3fread_word( fin );
		   if( str_cmp( word, "End" ) )
			i3fread_to_eol( fin );
		   else
			break;
		}
	   }  
	   continue;
	}
	else if( !str_cmp( word, "END" ) )
	        break;
	else
	{
	   i3bug( "I3_loadmudlist: bad section: %s.", word );
	   continue;
	}
   }
   I3FCLOSE( fin );
   unlink( I3_MUDLIST_FILE );
   return;
}

/* Called only during copyovers */
void I3_loadchanlist( void )
{
   FILE *fin;
   I3_CHANNEL *channel;

   if( !( fin = fopen( I3_CHANLIST_FILE, "r" ) ) )
	return;

   for( ; ; )
   {
	char letter;
	char *word;

	letter = i3fread_letter( fin );
	if( letter == '*' )
	{
	   i3fread_to_eol( fin );
	   continue;
	}

	if( letter != '#' )
	{
	   i3bug( "%s", "I3_loadchanlist: # not found." );
	   break;
	}

      word = i3fread_word( fin );
	if( !str_cmp( word, "I3CHAN" ) )
	{
	   int x;
	   I3CREATE( channel, I3_CHANNEL, 1 );
	   I3_readchannel( channel, fin );

	   for( x = 0; x < MAX_I3HISTORY; x++ )
		channel->history[x] = NULL;
	   I3LINK( channel, first_I3chan, last_I3chan, next, prev );
	   continue;
	}
	else
         if( !str_cmp( word, "END"	) )
	        break;
	else
	{
	   i3bug( "I3_loadchanlist: bad section: %s.", word );
	   continue;
	}
   }
   I3FCLOSE( fin );
   unlink( I3_CHANLIST_FILE );
   return;
}

/* Called only during copyovers */
void I3_savemudlist( void )
{
   FILE *fp;
   I3_MUD *mud;

   if( !( fp = fopen( I3_MUDLIST_FILE, "w" ) ) )
   {
	i3bug( "%s", "I3_savemudlist: Unable to write to mudlist file." );
	return;
   }

   fprintf( fp, "#ROUTER %s~\n", I3_ROUTER_NAME );
   for( mud = first_mud; mud; mud = mud->next )
   {
	/* Don't store muds that are down, who cares? They'll update themselves anyway */
      if( mud->status == 0 )
	   continue;

	fprintf( fp, "%s", "#MUDLIST\n" );
	fprintf( fp, "Name		%s~\n", mud->name );
	fprintf( fp, "Status		%d\n", mud->status );
	fprintf( fp, "IP			%s~\n", mud->ipaddress );
	fprintf( fp, "Mudlib		%s~\n", mud->mudlib );
	fprintf( fp, "Baselib		%s~\n", mud->base_mudlib );
	fprintf( fp, "Driver		%s~\n", mud->driver );
	fprintf( fp, "Type		%s~\n", mud->mud_type );
	fprintf( fp, "Openstatus	%s~\n", mud->open_status );
	fprintf( fp, "Email		%s~\n", mud->admin_email );
      if( mud->telnet )
	   fprintf( fp, "Telnet		%s~\n", mud->telnet );
	if( mud->web )
	   fprintf( fp, "Web		%s~\n", mud->web );
	if( mud->banner )
	   fprintf( fp, "Banner		%s~\n", mud->banner );
	if( mud->daemon )
	   fprintf( fp, "Dameon		%s~\n", mud->daemon );
	if( mud->time )
	   fprintf( fp, "Time		%s~\n", mud->time );
	fprintf( fp, "Ports %d %d %d\n", mud->player_port, mud->imud_tcp_port, mud->imud_udp_port );
	fprintf( fp, "Services %d %d %d %d %d %d %d %d %d %d %d %d\n",
	   mud->tell, mud->beep, mud->emoteto, mud->who, mud->finger, mud->locate, mud->channel, mud->news, mud->mail,
	   mud->file, mud->auth, mud->ucache );
	fprintf( fp, "OOBports %d %d %d %d %d %d %d\n", mud->smtp, mud->ftp, mud->nntp, mud->http, mud->pop3, mud->rcp, mud->amrcp );
	fprintf( fp, "%s", "End\n\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   I3FCLOSE( fp );
   return;
}

/* Called only during copyovers */
void I3_savechanlist( void )
{
   FILE *fp;
   I3_CHANNEL *channel;

   if( !( fp = fopen( I3_CHANLIST_FILE, "w" ) ) )
   {
	i3bug( "%s", "I3_savechanlist: Unable to write to chanlist file." );
	return;
   }

   for( channel = first_I3chan; channel; channel = channel->next )
   {
	/* Don't save local channels, they are stored elsewhere */
	if( channel->local_name )
	   continue;

	fprintf( fp, "%s", "#I3CHAN\n" );
	fprintf( fp, "ChanMud		%s~\n", channel->host_mud );
	fprintf( fp, "ChanName		%s~\n", channel->I3_name );
	fprintf( fp, "ChanStatus	%d\n", channel->status );
	fprintf( fp, "%s", "End\n\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   I3FCLOSE( fp );
   return;
}

/* Used during copyovers */
void I3_loadhistory( void )
{
   char filename[256];
   FILE *tempfile;
   I3_CHANNEL *tempchan = NULL;
   int x;

   for( tempchan = first_I3chan; tempchan; tempchan = tempchan->next )
   {
	if( !tempchan->local_name )
	   continue;

      snprintf( filename, 256, "%s/%s.hist", "../i3", tempchan->local_name );

      if( !( tempfile = fopen( filename, "r" ) ) )
	   continue;

      for( x = 0; x < MAX_I3HISTORY; x++ )
      {
         if( feof( tempfile ) )
		tempchan->history[x] = NULL;
	   else
            tempchan->history[x] = I3STRALLOC( i3fread_line( tempfile ) );
	}
      I3FCLOSE( tempfile );
	unlink( filename );
   }
}

/* Used during copyovers */
void I3_savehistory( void )
{
   char filename[256];
   FILE *tempfile;
   I3_CHANNEL *tempchan = NULL;
   int x;

   for( tempchan = first_I3chan; tempchan; tempchan = tempchan->next )
   {
	if( !tempchan->local_name )
	   continue;

	if( !tempchan->history[0] )
	   continue;

	snprintf( filename, 256, "%s/%s.hist", "../i3", tempchan->local_name );

      if( !( tempfile = fopen( filename, "w" ) ) )
	   continue;

      for( x = 0; x < MAX_I3HISTORY; x++ )
	{
	   if( tempchan->history[x] != NULL )
            fprintf( tempfile, "%s", tempchan->history[x] );
	}
      I3FCLOSE( tempfile );
   }
}

/*
 * Connect to the router and send the startup-packet.
 * Mud port is passed in from main() so that the information passed along to the I3
 * network regarding the mud's operational port is now determined by the mud's own
 * startup script instead of the I3 config file.
 */
void router_connect( const char *router_name, bool forced, int mudport, bool isconnected )
{
   ROUTER_DATA *router;
   bool rfound = FALSE;

   i3wait = 0;
   i3timeout = 0;
   bytes_sent = 0;
   bytes_received = 0;

   if( forced )
      free_i3data( TRUE );

   first_mud = NULL;
   last_mud = NULL;
   first_ucache = NULL;
   last_ucache = NULL;
   first_router = NULL;
   last_router = NULL;
   manual_router = router_name;

   if( !I3_read_config( mudport ) )
   {
	I3_socket = -1;
	return;
   }

   if( ( !this_mud->autoconnect && !forced && !isconnected ) || ( isconnected && I3_socket < 1 ) )
   {
	i3log( "%s", "Intermud-3 network data loaded. Autoconnect not set. Will need to connect manually." );
	I3_socket = -1;
	return;
   }
   else
	i3log( "%s", "Intermud-3 network data loaded. Initialiazing network connection..." );

   I3_loadchannels();
   I3_loadbans();

   if( this_mud->ucache == TRUE )
   {
      I3_load_ucache();
	I3_prune_ucache();
	ucache_clock = current_time + 86400;
   }

   if( I3_socket < 1 )
   {
      for( router = first_router; router; router = router->next )
      {
         if( router_name && str_cmp( router_name, router->name ) )
            continue;

         if( router->reconattempts <= 3 )
         {
            rfound = TRUE;
            I3_socket = I3_connection_open( router );
            break;
         }
      }
   }

   if( !rfound && !isconnected )
   {
      i3log( "%s", "Unable to connect. No available routers found." );
	I3_socket = -1;
      return;
   }

   if( I3_socket < 1 )
   {
	i3wait = 100;
      return;
   }

   sleep( 1 );

   i3log( "%s", "Intermud-3 Network initialized." );

   if( !isconnected )
   {
      I3_startup_packet( );
      i3timeout = 100;
   }
   else
   {
      I3_loadmudlist();
	I3_loadchanlist();
   }
   I3_loadhistory();
}

void I3_connect( CHAR_DATA *ch, char *argument ) 
{
   ROUTER_DATA *router;

   if( I3_is_connected() ) 
   {
	i3_printf( ch, "The MUD is already connected to Intermud-3 router %s\n\r", I3_ROUTER_NAME );
	return;
   }

   if( !argument || argument[0] == '\0' )
   {
      i3_to_char( "Connecting to Intermud-3\n\r", ch );
      router_connect( NULL, TRUE, this_mud->player_port, FALSE );
      return;
   }

   for( router = first_router; router; router = router->next )
   {
      if( !str_cmp( router->name, argument ) )
      {
         router->reconattempts = 0;
         i3_printf( ch, "Connecting to Intermud-3 router %s\n\r", argument );
         router_connect( argument, TRUE, this_mud->player_port, FALSE );
         return;
      }
   }

   i3_printf( ch, "%s is not configured as a router for this mud.\n\r", argument );
   i3_to_char( "If you wish to add it, use the i3router command to provide its information.\n\r", ch );
   return;
}

/* Wrapper for router_connect now - so we don't need to force older client installs to adjust. */
void I3_main( bool forced, int mudport, bool isconnected )
{
   this_mud = NULL;
   router_connect( NULL, forced, mudport, isconnected );
}

void I3_send_shutdown( int delay ) 
{
   I3_CHANNEL *channel;
   char s[50];

   if( !I3_is_connected() )
	return;

   for( channel = first_I3chan; channel; channel = channel->next )
   {
	if( channel->local_name && channel->local_name[0] != '\0' )
	   I3_send_channel_listen( channel, FALSE );
   }

   I3_write_header( "shutdown", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL );
   snprintf( s, 50, "%d", delay );
   I3_write_buffer( s );
   I3_write_buffer( ",})\r" );

   if( !I3_write_packet( I3_output_buffer ) )
	I3_connection_close( FALSE );

   return;
}

/*
 * Check for a packet and if one available read it and parse it.
 * Also checks to see if the mud should attempt to reconnect to the router.
 * This is an input only loop. Attempting to use it to send buffered output
 * just wasn't working out, so output has gone back to sending packets to the
 * router as soon as they're assembled.
 */
void I3_loop( void ) 
{
   ROUTER_DATA *router;
   int ret;
   long size;
   fd_set in_set, out_set, exc_set;
   static struct timeval null_time;
   bool rfound = FALSE;

#ifdef I3CIRCLE
   current_time = time( NULL );
#endif

   FD_ZERO( &in_set  );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );

   if( i3wait > 0 )
      i3wait--;

   if( i3timeout > 0 )
   {
      i3timeout--;
      if( i3timeout == 0 ) /* Time's up baby! */
      {
         I3_connection_close( TRUE );
         return;
      }
   }

   /* This condition can only occur if you were previously connected and the socket was closed.
    * Tries 3 times, then attempts connection to an alternate router, if it has one.
    */
   if( i3wait == 1 )
   {
      for( router = first_router; router; router = router->next )
      {
         if( manual_router && str_cmp( router->name, manual_router ) )
            continue;

         if( router->reconattempts <= 3 )
         {
            rfound = TRUE;
            break;
         }
      }

      if( !rfound )
      {
         i3wait = -2;
         i3log( "%s", "Unable to reconnect. No routers responding." );
         return;
      }
      I3_socket = I3_connection_open( router ); 
	if( I3_socket < 1 )
      {
         if( router->reconattempts <= 3 )
	      i3wait = 100;
	   return;
	}

      sleep( 1 );

      i3log( "Connection to Intermud-3 router %s %s.",
         router->name, router->reconattempts > 0 ? "reestablished" : "established" );
	router->reconattempts++;
      I3_startup_packet();
      i3timeout = 100;
	return;
   }

   if( !I3_is_connected() )
	return;

   /* Will prune the cache once every 24hrs after bootup time */
   if( ucache_clock <= current_time )
   {
	ucache_clock = current_time + 86400;
	I3_prune_ucache( );
   }

   FD_SET( I3_socket, &in_set );
   FD_SET( I3_socket, &out_set );
   FD_SET( I3_socket, &exc_set );

   if( select( I3_socket+1, &in_set, &out_set, &exc_set, &null_time ) < 0 ) 
   {
	perror( "I3_loop: select: Unable to poll I3_socket!" );
	I3_connection_close( TRUE );
	return;
   }

   if( FD_ISSET( I3_socket, &exc_set ) )
   {
	FD_CLR( I3_socket, &in_set );
	FD_CLR( I3_socket, &out_set );
	i3log( "%s", "Exception raised on socket." );
	I3_connection_close( TRUE );
	return;
   }

   if( FD_ISSET( I3_socket, &in_set ) )
   {
	ret = read( I3_socket, I3_input_buffer + I3_input_pointer, LGST );
	if( !ret || ( ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
	{
	   FD_CLR( I3_socket, &out_set );
	   if( ret < 0 )
		i3log( "%s", "Read error on socket." );
	   else
		i3log( "%s", "EOF encountered on socket read." );
  	   I3_connection_close( TRUE );
	   return;
	}
	if( ret < 0 ) /* EAGAIN */
	   return;

	I3_input_pointer += ret;
      bytes_received += ret;
      if( packetdebug )
         i3log( "Bytes received: %d", ret );
   }

   memcpy( &size, I3_input_buffer, 4 );
   size = ntohl( size );

   if( size <= I3_input_pointer - 4 )
   { 
      I3_read_packet();
	I3_parse_packet();
   }
   return;
}

/*
 * Shutdown the connection to the router.
 */
void I3_shutdown( int delay )
{
   if( I3_socket < 1 )
	return;

   I3_savehistory();
   free_i3data( FALSE );

   /* Flush the outgoing buffer */
   if( I3_output_pointer != 4 )
	I3_write_packet( I3_output_buffer );
 
   I3_send_shutdown( delay );
   I3_connection_close( FALSE );
   I3_input_pointer = 0;
   I3_output_pointer = 4;
   I3_save_id( );
   sleep( 2 ); /* Short delay to allow the socket to close */
}

char *I3_find_social( CHAR_DATA *ch, char *sname, char *person, char *mud, bool victim )
{
   static char socname[LGST];
#ifdef SMAUGSOCIAL
   SOCIAL_DATA *social;
   char *c;
#else
   int cmd;
   bool found;
#endif

   socname[0] = '\0';

#ifdef SMAUGSOCIAL
   for( c = sname; *c; *c = tolower(*c), c++ );

   if( ( social = find_social( sname ) ) == NULL )
   {
	i3_printf( ch, "{YSocial {W%s{Y does not exist on this mud.\n\r", sname );
	return socname;
   }

   if( person && person[0] != '\0' && mud && mud[0] != '\0' )
   {
	if( person && person[0] != '\0' && !str_cmp( person, CH_I3NAME(ch) ) 
	 && mud && mud[0] != '\0' && !str_cmp( mud, I3_THISMUD ) )
	{
	   if( !social->others_auto )
	   {
		i3_printf( ch, "{YSocial {W%s{Y: Missing others_auto.\n\r", sname );
		return socname;
	   }
 	   i3strlcpy( socname, social->others_auto, LGST );
	}
	else
	{
	   if( !victim )
	   {
		if( !social->others_found )
		{
		   i3_printf( ch, "{YSocial {W%s{Y: Missing others_found.\n\r", sname );
		   return socname;
		}
	      i3strlcpy( socname, social->others_found, LGST );
	   }
	   else
	   {
		if( !social->vict_found )
		{
		   i3_printf( ch, "{YSocial {W%s{Y: Missing vict_found.\n\r", sname );
		   return socname;
		}
		i3strlcpy( socname, social->vict_found, LGST );
	   }
	}
   }
   else
   {
	if( !social->others_no_arg )
	{
	   i3_printf( ch, "{YSocial {W%s{Y: Missing others_no_arg.\n\r", sname );
	   return socname;
	}
	i3strlcpy( socname, social->others_no_arg, LGST );
   }
#else
   found  = FALSE;
   for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
   {
	if( sname[0] == social_table[cmd].name[0] && !i3str_prefix( sname, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
   }

   if ( !found )
   {
	i3_printf( ch, "{YSocial {W%s{Y does not exist on this mud.\n\r", sname );
	return socname;
   }

   if( person && person[0] != '\0' && mud && mud[0] != '\0' )
   {
	if( person && person[0] != '\0' && !str_cmp( person, CH_I3NAME(ch) ) 
	 && mud && mud[0] != '\0' && !str_cmp( mud, I3_THISMUD ) )
	{
	   if( !social_table[cmd].others_auto )
	   {
		i3_printf( ch, "{YSocial {W%s{Y: Missing others_auto.\n\r", sname );
		return socname;
	   }
	   i3strlcpy( socname, social_table[cmd].others_auto, LGST );
	}
	else
	{
	   if( !victim )
	   {
		if( !social_table[cmd].others_found )
		{
		   i3_printf( ch, "{YSocial {W%s{Y: Missing others_found.\n\r", sname );
		   return socname;
		}
	      i3strlcpy( socname, social_table[cmd].others_found, LGST );
	   }
	   else
	   {
		if( !social_table[cmd].vict_found )
		{
		   i3_printf( ch, "{YSocial {W%s{Y: Missing vict_found.\n\r", sname );
		   return socname;
		}
		i3strlcpy( socname, social_table[cmd].vict_found, LGST );
	   }
	}
   }
   else
   {
	if( !social_table[cmd].others_no_arg )
	{
	   i3_printf( ch, "{YSocial {W%s{Y: Missing others_no_arg.\n\r", sname );
	   return socname;
	}
	i3strlcpy( socname, social_table[cmd].others_no_arg, LGST );
   }
#endif
   return socname;
}

/* Modified form of Smaug's act_string */
char *i3act_string( const char *format, CHAR_DATA *to, CHAR_DATA *ch, void *arg1 )
{
   static char * const he_she  [] = { "it",  "he",  "she" };
   static char * const him_her [] = { "it",  "him", "her" };
   static char * const his_her [] = { "its", "his", "her" };
   static char buf[LGST];
   char *point = buf;
   const char *str = format;
   const char *i;
   CHAR_DATA *vch = (CHAR_DATA *) arg1;

   while( *str != '\0' )
   {
      if( *str != '$' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if( !arg1 && *str >= 'A' && *str <= 'Z' )
      {
	   i3bug( "i3act_string: missing arg1 for code %c:", *str );
         i3bug( "%s", format );
         i = " <@@@> ";
      }
      else
      {
         switch( *str )
         {
            default:  
		   i3bug( "i3act_string: bad code %c.", *str );
		   i = " <@@@> ";
		   break;

	      case '$': 
	         i = "$";
	         break;
            case 't': i = (char *) arg1;					break;
            case 'T': i = (char *) arg1;					break;
            case 'n':
		   i = "$N";
               break;
            case 'N':
		   i = "$O";
		   break;
            case 'e': 
		   if( CH_I3SEX(ch) > 2 || CH_I3SEX(ch) < 0 )
		      i = "it";
		   else
		      i = he_she[URANGE( 0, CH_I3SEX(ch), 2 )];
		   break;
            case 'E':
		   if( CH_I3SEX(vch) > 2 || CH_I3SEX(vch) < 0 )
		      i = "it";
		   else
		      i = he_she[URANGE( 0, CH_I3SEX(vch), 2 )];
		   break;
            case 'm': 
		   if( CH_I3SEX(ch) > 2 || CH_I3SEX(ch) < 0 )
		      i = "it";
		   else
		      i = him_her[URANGE( 0, CH_I3SEX(ch), 2 )];
		   break;
            case 'M': 
		   if( CH_I3SEX(vch) > 2 || CH_I3SEX(vch) < 0 )
		      i = "it";
		   else
		      i = him_her[URANGE( 0, CH_I3SEX(vch), 2 )];
		   break;
            case 's':
		   if( CH_I3SEX(ch) > 2 || CH_I3SEX(ch) < 0 )
		      i = "its";
		   else
		      i = his_her[URANGE( 0, CH_I3SEX(ch), 2 )];
		   break;
            case 'S': 
		   if( CH_I3SEX(vch) > 2 || CH_I3SEX(vch) < 0 )
		      i = "its";
		   else
		      i = his_her[URANGE( 0, CH_I3SEX(vch), 2 )];
		   break;
         }
      }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   *point = '\0';
   return buf;
}

CHAR_DATA *I3_make_skeleton( char *name )
{
   CHAR_DATA *skeleton;

   I3CREATE( skeleton, CHAR_DATA, 1 );

#ifdef I3CIRCLE
   skeleton->player.name = I3STRALLOC( name );
   skeleton->player.short_descr = I3STRALLOC( name );
   skeleton->in_room = real_room( 1 );
#else
   skeleton->name = I3STRALLOC( name );
   skeleton->short_descr = I3STRALLOC( name );
   skeleton->in_room = get_room_index( ROOM_VNUM_LIMBO );
#endif

   return skeleton;
}

void I3_purge_skeleton( CHAR_DATA *skeleton )
{
   if( !skeleton )
      return;

#ifdef I3CIRCLE
   I3STRFREE( skeleton->player.name );
   I3STRFREE( skeleton->player.short_descr );
#else
   I3STRFREE( (char *)skeleton->name );
   I3STRFREE( (char *)skeleton->short_descr );
#endif
   I3DISPOSE( skeleton );

   return;
}

void I3_send_social( I3_CHANNEL *channel, CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *skeleton = NULL;
   char *ps;
   char socbuf_o[LGST], socbuf_t[LGST], msg_o[LGST], msg_t[LGST];
   char arg1[SMST], person[SMST], mud[SMST], user[SMST], buf[LGST];
   unsigned int x;

   person[0] = '\0';
   mud[0] = '\0';

   /* Name of social, remainder of argument is assumed to hold the target */
   argument = i3one_argument( argument, arg1 );

   snprintf( user, SMST, "%s@%s", CH_I3NAME(ch), I3_THISMUD );
   if( !str_cmp( user, argument ) )
   {
	i3_to_char( "Cannot target yourself due to the nature of I3 socials.\n\r", ch );
	return;
   }

   if( argument && argument[0] != '\0' )
   {
      if( !( ps = strchr( argument, '@' ) ) )
      {
	   i3_to_char( "You need to specify a person@mud for a target.\n\r", ch );
	   return;
      }
      else
      {
         for( x = 0; x < strlen( argument ); x++ )
         {
	      person[x] = argument[x];
	      if( person[x] == '@' )
	         break;
         }
         person[x] = '\0';

         ps[0] = '\0';
	   i3strlcpy( mud, ps+1, SMST );
      }
   }

   snprintf( socbuf_o, LGST, "%s", I3_find_social( ch, arg1, person, mud, FALSE ) );

   if( socbuf_o && socbuf_o[0] != '\0' )
      snprintf( socbuf_t, LGST, "%s", I3_find_social( ch, arg1, person, mud, TRUE ) );

   if( ( socbuf_o && socbuf_o[0] != '\0' ) && ( socbuf_t && socbuf_t[0] != '\0' ) )
   {
	if( argument && argument[0] != '\0' )
	{
	   int sex;

	   snprintf( buf, LGST, "%s@%s", person, mud );
	   sex = I3_get_ucache_gender( buf );
	   if( sex == -1 )
	   {
	      /* Greg said to "just punt and call them all males".
		 * I decided to meet him halfway and at least request data before punting :)
		 */
            I3_send_chan_user_req( mud, person );
		sex = SEX_MALE;
	   }
	   else
		sex = i3todikugender( sex );

	   skeleton = I3_make_skeleton( buf );
	   CH_I3SEX(skeleton) = sex;
	}

	i3strlcpy( msg_o, (char *)i3act_string( socbuf_o, NULL, ch, skeleton ), LGST );
	i3strlcpy( msg_t, (char *)i3act_string( socbuf_t, NULL, ch, skeleton ), LGST );

	if( !skeleton )
	   I3_send_channel_emote( channel, CH_I3NAME(ch), msg_o );
	else
	{
	   i3strlcpy( buf, person, LGST );
	   buf[0] = tolower( buf[0] );
	   I3_send_channel_t( channel, CH_I3NAME(ch), mud, buf, msg_o, msg_t, person );
	}
	if( skeleton )
	   I3_purge_skeleton( skeleton );
   }
   return;
}

void I3_router( CHAR_DATA *ch, char *argument )
{
   ROUTER_DATA *router;
   char cmd[SMST];

   if( !argument || argument[0] == '\0' )
   {
      i3_to_char( "Usage: i3router add <router_name> <router_ip> <router_port>\n\r", ch );
      i3_to_char( "Usage: i3router remove <router_name>\n\r", ch );
      i3_to_char( "Usage: i3router list\n\r", ch );
      return;
   }
   argument = i3one_argument( argument, cmd );

   if( !str_cmp( cmd, "list" ) )
   {
      i3_to_char( "{RThe mud has the following routers configured:\n\r", ch );
      i3_to_char( "{WRouter Name     Router IP/DNS                  Router Port\n\r", ch );
      for( router = first_router; router; router = router->next )
         i3_printf( ch, "{c%-15.15s{c %-30.30s %d\n\r", router->name, router->ip, router->port );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      I3_router( ch, "" );
      return;
   }

   if( !str_cmp( cmd, "remove" ) )
   {
      for( router = first_router; router; router = router->next )
      {
         if( !str_cmp( router->name, argument ) || !str_cmp( router->ip, argument ) )
         {
            I3STRFREE( router->name );
            I3STRFREE( router->ip );
            I3UNLINK( router, first_router, last_router, next, prev );
            I3DISPOSE( router );
            i3_printf( ch, "{YRouter {W%s{Y has been removed from your configuration.\n\r", argument );
            I3_saveconfig();
            return;
         }
      }
      i3_printf( ch, "{YNo router named {W%s{Y exists in your configuration.\n\r", argument );
      return;
   }

   if( !str_cmp( cmd, "add" ) )
   {
      ROUTER_DATA *temp;
      char rtname[SMST];
      char rtip[SMST];
      int rtport;

      argument = i3one_argument( argument, rtname );
      argument = i3one_argument( argument, rtip );

      if( !rtname || rtname[0] == '\0' || !rtip || rtip[0] == '\0' || !argument || argument[0] == '\0' )
      {
         I3_router( ch, "" );
         return;
      }

      if( rtname[0] != '*' )
      {
         i3_to_char( "{YA router name must begin with a {W*{Y to be valid.\n\r", ch );
         return;
      }

      for( temp = first_router; temp; temp = temp->next )
      {
         if( !str_cmp( temp->name, rtname ) )
         {
            i3_printf( ch, "{YA router named {W%s{Y is already in your configuration.\n\r", rtname );
            return;
         }
      }

      if( !is_number( argument ) )
      {
         i3_to_char( "{YPort must be a numerical value.\n\r", ch );
         return;
      }

      rtport = atoi( argument );
      if( rtport < 1 || rtport > 65535 )
      {
         i3_to_char( "{YInvalid port value specified.\n\r", ch );
         return;
      }

      I3CREATE( router, ROUTER_DATA, 1 );
      I3LINK( router, first_router, last_router, next, prev );
      router->name = I3STRALLOC( rtname );
      router->ip = I3STRALLOC( rtip );
      router->port = rtport;
      router->reconattempts = 0;

      i3_printf( ch, "{YRouter: {W%s %s %d{Y has been added to your configuration.\n\r",
         router->name, router->ip, router->port );
      I3_saveconfig();
      return;
   }
   I3_router( ch, "" );
   return;
}

void I3_stats( CHAR_DATA *ch, char *argument )
{
   I3_MUD *mud;
   I3_CHANNEL *channel;
   int mud_count = 0, chan_count = 0;

   for( mud = first_mud; mud; mud = mud->next )
      mud_count++;

   for( channel = first_I3chan; channel; channel = channel->next )
      chan_count++;

   i3_to_char( "{cGeneral Statistics:\n\r\n\r", ch );
   i3_printf( ch, "{cCurrently connected to: {W%s\n\r",
      I3_is_connected() ? I3_ROUTER_NAME : "Nowhere!" );
   i3_printf( ch, "{cBytes sent    : {W%ld\n\r", bytes_sent );
   i3_printf( ch, "{cBytes received: {W%ld\n\r", bytes_received );
   i3_printf( ch, "{cKnown muds    : {W%d\n\r", mud_count );
   i3_printf( ch, "{cKnown channels: {W%d\n\r", chan_count );
   return;
}

/* This function should coincide with the command struct below */
void I3_other( CHAR_DATA *ch, char *argument )
{
   i3_to_char( "{GGeneral Usage:\n\r", ch );
   i3_to_char( "{G------------------------------------------------\n\r\n\r", ch );
   i3_to_char( "{wList channels available                : {Gi3chanlist [all] [filter]\n\r", ch );
   i3_to_char( "{wTo tune into a channel                 : {Gi3listen <localchannel>\n\r", ch );
   i3_to_char( "{wTo see who is listening on another mud : {Gi3chanwho <channel> <mud>\n\r", ch );
   i3_to_char( "{wList muds connected to I3              : {Gi3mudlist [filter]\n\r", ch );
   i3_to_char( "{wInformation on another mud             : {Gi3mudinfo <mud>\n\r", ch );
   i3_to_char( "{wIgnore someone who annoys you          : {Gi3ignore <string>\n\r", ch );
   i3_to_char( "{wMake yourself invisible to I3          : {Gi3invis\n\r", ch );
   i3_to_char( "{wToggle I3 color                        : {Gi3color\n\r", ch );

   if( I3PERM(ch) >= I3PERM_IMM )
   {
      i3_to_char( "\n\r{YImmortal functions\n\r", ch );
      i3_to_char( "{Y------------------------------------------------\n\r\n\r", ch );
      i3_to_char( "{YGeneral statistics:\n\r", ch );
      i3_to_char( "{wi3ucache\n\r", ch );
      i3_to_char( "{wi3user <person@mud>\n\r", ch );
      i3_to_char( "{wi3perms <user>\n\r\n\r", ch );
      i3_to_char( "{YChannel control:\n\r", ch );
      i3_to_char( "{wi3deny <person> <local channel>\n\r", ch );
   }

   if( I3PERM(ch) >= I3PERM_ADMIN )
   {
      i3_to_char( "\n\r{RAdministrative functions\n\r", ch );
      i3_to_char( "{R------------------------------------------------\n\r\n\r", ch );
      i3_to_char( "{RLocal channel setup and editing:\n\r", ch );
      i3_to_char( "{wi3setchan <i3channelname> <localname> [level]\n\r", ch );
      i3_to_char( "{wi3editchan <localchannel>\n\r", ch );
      i3_to_char( "{wi3chanlayout <localchannel> <message> <emote>\n\r\n\r", ch );
      i3_to_char( "{RNew channel creation and administration:\n\r", ch );
      i3_to_char( "{wi3addchan <channelname> <localname> <type>\n\r", ch );
      i3_to_char( "{wi3removechan <channel>\n\r", ch );
      i3_to_char( "{wi3adminchan <localchannel> <add|remove> <mudname>\n\r", ch );
      i3_to_char( "{wi3adminchan <localchannel> list\n\r\n\r", ch );
      i3_to_char( "{RTraffic control and permissions:\n\r", ch );
      i3_to_char( "{wi3ban <string>\n\r", ch );
      i3_to_char( "{wi3permset <user> <permission>\n\r", ch );
      i3_to_char( "{wi3mudlisten <localchannel>\n\r", ch );
   }
   return;
}

const	struct	i3cmd_type	i3cmd_table	[] =
{
   /* The revamped Merc-like command table. The I3_other function should coincide with this. */
   /* Command name,	Function called,			Permission,	Need to be connected? */

   { "i3",              I3_other,                     I3PERM_MORT,      FALSE },
   { "i3listen",        I3_listen_channel,            I3PERM_MORT,      TRUE },
   { "i3chanlist",      I3_chanlist,                  I3PERM_MORT,      TRUE },
   { "i3mudlist",       I3_mudlist,                   I3PERM_MORT,      TRUE },
   { "i3chanwho",       I3_chan_who,                  I3PERM_MORT,      TRUE },
   { "i3mudinfo",       I3_mudinfo,                   I3PERM_MORT,      TRUE },
   { "i3invis",         I3_invis,                     I3PERM_MORT,      FALSE },
   { "i3who",           I3_who,                       I3PERM_MORT,      TRUE },
   { "i3finger",        I3_finger,                    I3PERM_MORT,      TRUE },
   { "i3locate",        I3_locate,                    I3PERM_MORT,      TRUE },
   { "i3tell",          I3_tell,                      I3PERM_MORT,      TRUE },
   { "i3reply",         I3_reply,                     I3PERM_MORT,      TRUE },
   { "i3emote",         I3_emote,                     I3PERM_MORT,      TRUE },
   { "i3beep",          I3_beep,                      I3PERM_MORT,      TRUE },
   { "i3ignore",        I3_ignorecmd,                 I3PERM_MORT,      FALSE },
   { "i3color",         I3_color,                     I3PERM_MORT,      FALSE },
   { "i3connect",       I3_connect,                   I3PERM_IMM,       FALSE },
   { "i3disconnect",    I3_disconnect,                I3PERM_IMM,       TRUE },
   { "i3user",          I3_send_user_req,             I3PERM_IMM,       TRUE },
   { "i3ucache",        I3_show_ucache_contents,      I3PERM_IMM,       TRUE },
   { "i3perms",         I3_permstats,                 I3PERM_IMM,       FALSE },
   { "i3deny",          I3_deny_channel,              I3PERM_IMM,       TRUE },
   { "i3permset",       I3_permset,                   I3PERM_ADMIN,     FALSE },
   { "i3setchan",       I3_setup_channel,             I3PERM_ADMIN,     TRUE },
   { "i3chanlayout",    I3_chanlayout,                I3PERM_ADMIN,     TRUE },
   { "i3addchan",       I3_addchan,                   I3PERM_ADMIN,     TRUE },
   { "i3removechan",    I3_removechan,                I3PERM_ADMIN,     TRUE },
   { "i3adminchan",     I3_admin_channel,             I3PERM_ADMIN,     TRUE },
   { "i3editchan",      I3_edit_channel,              I3PERM_ADMIN,     TRUE },
   { "i3ban",           I3_bancmd,                    I3PERM_ADMIN,     FALSE },
   { "i3config",        I3_setconfig,                 I3PERM_ADMIN,     FALSE },
   { "i3mudlisten",     I3_mudlisten,                 I3PERM_ADMIN,     TRUE },
   { "i3stats",         I3_stats,                     I3PERM_ADMIN,     FALSE },
   { "i3router",        I3_router,                    I3PERM_IMP,       FALSE },
   { "i3debug",         I3_debug,                     I3PERM_IMP,       FALSE },

   { NULL, NULL, 0, 0 }
};

/*
 * This is how channels are interpreted. If they are not commands
 * or socials, this function will go through the list of channels
 * and send it to it if the name matches the local channel name.
 */
bool I3_command_hook( CHAR_DATA *ch, char *command, char *argument ) 
{
   I3_CHANNEL *channel;
   int x;

   if( IS_NPC(ch) )
	return FALSE;

   if( !ch->desc )
	return FALSE;

   if( !this_mud )
   {
	i3log( "%s", "Ooops. I3 called with missing configuration!" );
	return FALSE;
   }
    
    /*
     * Player hasn't been initialized into I3
     */
    if ( I3PERM(ch) == I3PERM_NOTSET )
    {
        I3_set_perms( ch );
        I3_char_login( ch );
    }

   if( I3PERM(ch) <= I3PERM_NONE )
        return FALSE;    

#ifdef I3CIRCLE
   /* CircleMUD parser leaves leading spaces after splitting one argument */
   skip_spaces( &argument );
#endif

   /* Simple command interpreter menu. Nothing overly fancy etc, but it beats trying to tie directly into the mud's
    * own internal structures. Especially with the differences in codebases. If this looks eerily similar to the Merc
    * command table setup, that's because it's a simplified version of exactly that :)
    */
   for( x = 0; i3cmd_table[x].name != NULL; x++ )
   {
      if( I3PERM(ch) < i3cmd_table[x].level )
	   continue;

      if( !str_cmp( command, i3cmd_table[x].name ) )
      {
	   if( i3cmd_table[x].connected == TRUE && !I3_is_connected() )
         {
		i3_to_char( "The mud is not currently connected to I3.\n\r", ch );
	      return TRUE;
         }

         ( *i3cmd_table[x].function )( ch, argument ); 
         return TRUE;
      }
   }

   /* Assumed to be going for a channel if it gets this far */

   if( ( channel = find_I3_channel_by_localname( command ) ) == NULL )
	return FALSE;

   if( I3PERM(ch) < channel->i3perm )
	return FALSE;

   if( I3_hasname( I3DENY(ch), channel->local_name ) )
   {
	i3_printf( ch, "You have been denied the use of %s by the administration.\n\r", channel->local_name );
	return TRUE;
   }

   if( !argument || argument[0] == '\0' )
   {
	i3_printf( ch, "{cThe last %d %s messages:\n\r", MAX_I3HISTORY, channel->local_name );
	for( x = 0; x < MAX_I3HISTORY; x++ )
	{
	   if( channel->history[x] != NULL )
		i3_printf( ch, "%s", channel->history[x] );
	   else
		break;
	}
	return TRUE;
   }

   if( !I3_is_connected() )
   {
      i3_to_char( "The mud is not currently connected to I3.\n\r", ch );
      return TRUE;
   }

   if( I3PERM(ch) >= I3PERM_ADMIN && !str_cmp( argument, "log" ) )
   {
	if( !I3IS_SET( channel->flags, I3CHAN_LOG ) )
	{
	   I3SET_BIT( channel->flags, I3CHAN_LOG );
	   i3_printf( ch, "{RFile logging enabled for %s, PLEASE don't forget to undo this when it isn't needed!\n\r", channel->local_name );
	}
	else
	{
	   I3REMOVE_BIT( channel->flags, I3CHAN_LOG );
	   i3_printf( ch, "{GFile logging disabled for %s.\n\r", channel->local_name );
	}
	I3_write_channel_config();
	return TRUE;
   }

   if( !I3_hasname( I3LISTEN(ch), channel->local_name ) )
   {
	i3_printf( ch, "{YYou were trying to send something to an I3 "
	   "channel but you're not listening to it.\n\rPlease use the command "
	   "'{Wi3listen %s{Y' to listen to it.\n\r", channel->local_name );
	return TRUE;
   }

   switch( argument[0] )
   {
	case ',':
         /* Strip the , and then extra spaces - Remcon 6-28-03 */
         argument++;
         while( isspace( *argument ) ) argument++;
         I3_send_channel_emote( channel, CH_I3NAME(ch), argument );
	   break;
	case '@':
         /* Strip the @ and then extra spaces - Remcon 6-28-03 */
         argument++;
         while( isspace( *argument ) ) argument++;
	   I3_send_social( channel, ch, argument );
	   break;
	default:
	   I3_send_channel_message( channel, CH_I3NAME(ch), argument );
	   break;
   }
   return TRUE;
}
