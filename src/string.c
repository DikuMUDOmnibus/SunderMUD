/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/**********************************************************
 *************** S U N D E R M U D *** 2 . 0 **************
 **********************************************************
 * The unique portions of the SunderMud code as well as   *
 * the integration efforts for code from other sources is *
 * based primarily on the efforts of:                     *
 *                                                        *
 * Lotherius <aelfwyne@operamail.com> (Alvin W. Brinson)  *
 *    and many others, see "help sundermud" in the mud.   *
 **********************************************************/

#include <stdarg.h>
#include <limits.h>
#include "everything.h"
#include "olc.h"

char *numlineas( char * );
char *getline( char *, char * );
char *linedel( char *, int );
char *lineadd( char *, char *, int );

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit ( CHAR_DATA * ch, char **pString )
{
     send_to_char ( "-==[ Edit Mode ]===============================================-\n\r", ch );
     send_to_char ( "    Type .h on a new line for help\n\r",        ch );
     send_to_char ( " Terminate with a @ on a blank line.\n\r",      ch );
     send_to_char ( "-==============================================================-\n\r", ch );

     if ( *pString == NULL )
     {
          *pString = str_dup ( "" );
     }
     else
     {
          **pString = '\0';
     }

     ch->desc->pString = pString;

     return;
}

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append ( CHAR_DATA * ch, char **pString )
{
     if ( ch->pcdata->mode == MODE_DESCEDIT )
          send_to_char ( "-==[ Description Editor ]======================================-\n\r",   ch );
     else if ( ch->pcdata->mode == MODE_LEASEDESC )
          send_to_char ( "-==[ Lease Description ]=======================================-\n\r",   ch );
     else send_to_char ( "-==[ String Append Mode ]======================================-\n\r",   ch );
     send_to_char ( "    Type .h on a new line for help\n\r",          ch );
     send_to_char ( " Terminate with a @ on a blank line.\n\r",   ch );
     send_to_char ( "-==============================================================-\n\r",   ch );

     if ( *pString == NULL )
     {
          *pString = str_dup ( "" );
     }
     send_to_char ( numlineas(*pString), ch );

     if ( *( *pString + strlen ( *pString ) - 1 ) != '\r' )
          send_to_char ( "\n\r", ch );

     ch->desc->pString = pString;

     return;
}

/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 Notes:         Fixes by Calath(gblues@jps.net) to prevent buffer overruns
 ****************************************************************************/
char *string_replace(char *orig, char *old, char *new)
{
     char buf[MSL];
     char *ptr;
     int  a, b;

     if ( (ptr = strstr(orig, old)) == NULL
          || ( a = ptr - orig ) + strlen(new) >=(MSL-4) )
          return orig;
     b = MSL - 4 - a - strlen(new);
     strncpy ( buf, orig, MSL );
     strncpy ( buf+a, new, strlen(new));
     strncpy ( buf+a+strlen(new), ptr+strlen(old), b);
     buf[MSL-4]='\0';
     free_string ( orig );
     return str_dup ( buf );
}

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];

     if ( *argument == '.' )
     {
          char                arg1[MAX_INPUT_LENGTH];
          char                arg2[MAX_INPUT_LENGTH];
          char                arg3[MAX_INPUT_LENGTH];
          char                tmparg[MIL];

          argument = one_argument ( argument, arg1 );
          argument = first_arg ( argument, arg2, FALSE );
          strcpy( tmparg, argument );
          argument = first_arg ( argument, arg3, FALSE );

          if ( !str_cmp ( arg1, ".c" ) )
          {
               send_to_char ( "String cleared.\n\r", ch );
               *ch->desc->pString = str_dup( "" );
               return;
          }
          else if ( !str_cmp ( arg1, ".s" ) )
          {
               send_to_char ( "String so far:\n\r", ch );
               send_to_char ( numlineas(*ch->desc->pString), ch );
               return;
          }
          else if ( !str_cmp ( arg1, ".d" ) )
          {
               int                 tot_len, i, count = 0;
               char                newbuf[1024];
               char                buf[MAX_STRING_LENGTH*4];
               send_to_char ( "Deleting last line.\n\r", ch );
               strcpy ( buf, *ch->desc->pString );
               tot_len = strlen ( buf );
               if ( tot_len < 3 )	/*no lines left, just null or line terminators */
               {
                    send_to_char ( "No lines left to delete.\n\r", ch );
                    return;
               }
               for ( i = ( tot_len ); ( ( i >= 0 ) && ( count < 2 ) ); i-- )
               {
                    if ( buf[i] == '\r' )
                         count++;
               }
               if ( !( i <= 0 ) )
               {
                    strncpy ( newbuf, buf, ( i + 2 ) );
                    newbuf[i + 2] = '\0';
               }
               else
                    newbuf[0] = '\0';
               free_string ( *ch->desc->pString );
               *ch->desc->pString = str_dup ( newbuf );
               send_to_char ( "Ok.\n\r", ch );
               return;
          }
          else if ( !str_cmp ( arg1, ".r" ) )
          {
               if ( arg2[0] == '\0' )
               {
                    send_to_char ( "usage:  .r \"old string\" \"new string\"\n\r", ch );
                    return;
               }

               *ch->desc->pString = string_replace ( *ch->desc->pString, arg2, arg3 );
               form_to_char ( ch, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
               return;
          }
          else if ( !str_cmp ( arg1, ".f" ) )
          {
               *ch->desc->pString = format_string ( *ch->desc->pString );
               send_to_char ( "String formatted.\n\r", ch );
               return;
          }
          else if ( !str_cmp( arg1, ".ld" ) )
          {
               *ch->desc->pString = linedel( *ch->desc->pString, atoi(arg2) );
               write_to_buffer( ch->desc, "Line deleted.\n\r", 0 );
               return;
          }
          else if ( !str_cmp( arg1, ".li" ) )
          {
               if ( strlen( *ch->desc->pString ) + strlen( tmparg ) >= ( MAX_STRING_LENGTH - 4 ) )
               {
                    write_to_buffer( ch->desc, "Too long.\n\r", 0 );
                    return;
               }

               *ch->desc->pString = lineadd( *ch->desc->pString, tmparg, atoi(arg2));
               write_to_buffer( ch->desc, "Line Inserted.\n\r", 0 );
               return;
          }
          else if ( !str_cmp( arg1, ".lr" ) )
          {
               *ch->desc->pString = linedel( *ch->desc->pString, atoi(arg2) );
               *ch->desc->pString = lineadd( *ch->desc->pString, tmparg, atoi(arg2) );
               write_to_buffer( ch->desc, "Line Replaced.\n\r", 0 );
               return;
          }
          else if ( !str_cmp ( arg1, ".h" ) )
          {
               send_to_char ( "Sedit help (commands on blank line):   \n\r", ch );
               send_to_char ( ".r 'old' 'new'   - replace a substring \n\r", ch );
               send_to_char ( "                   (requires '', \"\") \n\r", ch );
               send_to_char ( ".h               - get help (this info)\n\r", ch );
               send_to_char ( ".s               - show string so far  \n\r", ch );
               send_to_char ( ".f               - (word wrap) string  \n\r", ch );
               send_to_char ( ".c               - clear string so far \n\r", ch );
               send_to_char ( ".d               - delete the last line\n\r", ch );
               send_to_char ( ".ld <num>        - delete line <num>   \n\r", ch );
               send_to_char ( ".li <num> <txt>  - insert <txt> at <num>\n\r", ch );
               send_to_char ( ".lr <num> <txt>  - replace line <num> w/<txt>\n\r", ch );
               send_to_char ( "@                - end string          \n\r", ch );
               return;
          }
          send_to_char ( "Edit:  Invalid dot command - Use .h for help.\n\r", ch );
          return;
     }

     if ( *argument == '@' )
     {
          if ( ch->desc->editor == ED_MPCODE ) /* for mobprogs */
          {
               MOB_INDEX_DATA *mob;
               int hash;
               PROG_LIST *mpl;
               PROG_CODE *mpc;

               EDIT_MPCODE(ch, mpc);

               if ( mpc != NULL )
                    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
                         for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
                              for ( mpl = mob->mprogs; mpl; mpl = mpl->next )
                                   if ( mpl->vnum == mpc->vnum )
                                   {
                                        form_to_char ( ch, "Fixing mob %d.\n\r", mob->vnum );
                                        mpl->code = mpc->code;
                                   }
          }
          else if ( ch->desc->editor == ED_OPCODE ) /* for the objprogs */
          {
               OBJ_INDEX_DATA *obj;
               int hash;
               PROG_LIST *opl;
               PROG_CODE *opc;

               EDIT_OPCODE(ch, opc);

               if ( opc != NULL )
                    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
                         for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
                              for ( opl = obj->oprogs; opl; opl = opl->next )
                                   if ( opl->vnum == opc->vnum )
                                   {
                                        form_to_char ( ch, "Fixing object %d.\n\r", obj->vnum );
                                        opl->code = opc->code;
                                   }
          }
          else if ( ch->desc->editor == ED_RPCODE ) /* for the roomprogs */
          {
               ROOM_INDEX_DATA *room;
               int hash;
               PROG_LIST *rpl;
               PROG_CODE *rpc;

               EDIT_RPCODE(ch, rpc);
               if ( rpc != NULL )
                    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
                         for ( room = room_index_hash[hash]; room; room = room->next )
                              for ( rpl = room->rprogs; rpl; rpl = rpl->next )
                                   if ( rpl->vnum == rpc->vnum )
                                   {
                                        form_to_char ( ch, "Fixing room %d.\n\r", room->vnum );
                                        rpl->code = rpc->code;
                                   }
          }
          ch->pcdata->mode = MODE_NORMAL;
          ch->desc->pString = NULL;
          return;
     }

     if ( ch->pcdata->mode == MODE_LEASEDESC )
          smash_codes ( argument );

     strcpy ( buf, *ch->desc->pString );

     /*
      * Truncate strings to MAX_STRING_LENGTH.
      * --------------------------------------
      */
     if ( strlen ( *ch->desc->pString ) + strlen ( argument ) >= ( MAX_STRING_LENGTH - 200 ) )
     {
          send_to_char ( "String too long, extra truncated.\n\r", ch );
          /* Force character out of editing mode. */
          ch->desc->pString = NULL;
          return;
     }

     strcat ( buf, argument );
     strcat ( buf, "\n\r" );
     free_string ( *ch->desc->pString );
     *ch->desc->pString = str_dup ( buf );
     return;
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */

/*
 * This function needs to become colour-code aware.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char               *format_string ( char *oldstring)
{
     char                xbuf[MAX_STRING_LENGTH];
     char                xbuf2[MAX_STRING_LENGTH];
     char               *rdesc;
     int                 i = 0;
     bool                cap = TRUE;

     xbuf[0] = xbuf2[0] = 0;

     i = 0;

     for ( rdesc = oldstring; *rdesc; rdesc++ )
     {
          if ( *rdesc == '\n' )
          {
               if ( xbuf[i - 1] != ' ' )
               {
                    xbuf[i] = ' ';
                    i++;
               }
          }
          else if ( *rdesc == '\r' );
          else if ( *rdesc == ' ' )
          {
               if ( xbuf[i - 1] != ' ' )
               {
                    xbuf[i] = ' ';
                    i++;
               }
          }
          else if ( *rdesc == ')' )
          {
               if ( xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' && ( xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                                                                  || xbuf[i - 3] == '!' ) )
               {
                    xbuf[i - 2] = *rdesc;
                    xbuf[i - 1] = ' ';
                    xbuf[i] = ' ';
                    i++;
               }
               else
               {
                    xbuf[i] = *rdesc;
                    i++;
               }
          }
          else if ( *rdesc == '.' || *rdesc == '?' || *rdesc == '!' )
          {
               if ( xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                    ( xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!' ) )
               {
                    xbuf[i - 2] = *rdesc;
                    if ( *( rdesc + 1 ) != '\"' )
                    {
                         xbuf[i - 1] = ' ';
                         xbuf[i] = ' ';
                         i++;
                    }
                    else
                    {
                         xbuf[i - 1] = '\"';
                         xbuf[i] = ' ';
                         xbuf[i + 1] = ' ';
                         i += 2;
                         rdesc++;
                    }
               }
               else
               {
                    xbuf[i] = *rdesc;
                    if ( *( rdesc + 1 ) != '\"' )
                    {
                         xbuf[i + 1] = ' ';
                         xbuf[i + 2] = ' ';
                         i += 3;
                    }
                    else
                    {
                         xbuf[i + 1] = '\"';
                         xbuf[i + 2] = ' ';
                         xbuf[i + 3] = ' ';
                         i += 4;
                         rdesc++;
                    }
               }
               cap = TRUE;
          }
          else
          {
               xbuf[i] = *rdesc;
               if ( cap )
               {
                    cap = FALSE;
                    xbuf[i] = UPPER ( xbuf[i] );
               }
               i++;
          }
     }
     xbuf[i] = 0;
     strcpy ( xbuf2, xbuf );

     rdesc = xbuf2;

     xbuf[0] = 0;

     for ( ;; )
     {
          for ( i = 0; i < 77; i++ )
          {
               if ( !*( rdesc + i ) )
                    break;
          }
          if ( i < 77 )
          {
               break;
          }
          for ( i = ( xbuf[0] ? 76 : 73 ); i; i-- )
          {
               if ( *( rdesc + i ) == ' ' )
                    break;
          }
          if ( i )
          {
               *( rdesc + i ) = 0;
               strcat ( xbuf, rdesc );
               strcat ( xbuf, "\n\r" );
               rdesc += i + 1;
               while ( *rdesc == ' ' )
                    rdesc++;
          }
          else
          {
               // Okay so somebody tried to format a string with no spaces...
               // Why are we calling it a bug???
               // bugf ( "No spaces" );
               *( rdesc + 75 ) = 0;
               strcat ( xbuf, rdesc );
               strcat ( xbuf, "-\n\r" );
               rdesc += 76;
          }
     }
     while ( *( rdesc + i ) && ( *( rdesc + i ) == ' ' ||
                                 *( rdesc + i ) == '\n' ||
                                 *( rdesc + i ) == '\r' ) )
          i--;
     *( rdesc + i + 1 ) = 0;
     strcat ( xbuf, rdesc );
     if ( xbuf[strlen ( xbuf ) - 2] != '\n' )
          strcat ( xbuf, "\n\r" );

     free_string ( oldstring );
     return ( str_dup ( xbuf ) );
}

/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char               *first_arg ( char *argument, char *arg_first, bool fCase )
{
     char                cEnd;

     while ( *argument == ' ' )
          argument++;

     cEnd = ' ';
     if ( *argument == '\'' || *argument == '"'
          || *argument == '%' || *argument == '(' )
     {
          if ( *argument == '(' )
          {
               cEnd = ')';
               argument++;
          }
          else
               cEnd = *argument++;
     }

     while ( *argument != '\0' )
     {
          if ( *argument == cEnd )
          {
               argument++;
               break;
          }
          if ( fCase )
               *arg_first = LOWER ( *argument );
          else
               *arg_first = *argument;
          arg_first++;
          argument++;
     }
     *arg_first = '\0';

     while ( *argument == ' ' )
          argument++;

     return argument;
}

/*
 * Used in olc_act.c for aedit_builders.
 */
char               *string_unpad ( char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char               *s;

     s = argument;

     while ( *s == ' ' )
          s++;

     strcpy ( buf, s );
     s = buf;

     if ( *s != '\0' )
     {
          while ( *s != '\0' )
               s++;
          s--;

          while ( *s == ' ' )
               s--;
          s++;
          *s = '\0';
     }

     free_string ( argument );
     return str_dup ( buf );
}

/* Very basic "int" to "string" conversion function
 * I'm sure this is done somewhere in the std C library, but I couldn't
 * find it - Lotherius
 */

char               *itos ( int num )
{
     char                buf[256];
     SNP ( buf, "%d", num );
     return str_dup ( buf );
}

/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char               *string_proper ( char *argument )
{
     char               *s;

     s = argument;

     while ( *s != '\0' )
     {
          if ( *s != ' ' )
          {
               *s = UPPER ( *s );
               while ( *s != ' ' && *s != '\0' )
                    s++;
          }
          else
          {
               s++;
          }
     }

     return argument;
}

/* Complete with Spanish from OLC 2.01 or so.... */
char * rip_arg( char *argument, char *arg_first )
{
     int tipo;

     while ( *argument && isspace(*argument) )
          argument++;

     if ( isalpha(*argument) )
          tipo = 0;     /* letras */
     else
          if ( isdigit(*argument) )
               tipo = 1;        /* numeros */
     else
          tipo = 2;     /* otros */

     while ( *argument )
     {
          if ( (isalpha(*argument) && tipo != 0)
               || (isdigit(*argument) && tipo != 1)
               || (!isalpha(*argument) && !isdigit(*argument) && tipo != 2) )
               break;

          *arg_first = LOWER(*argument);
          arg_first++;
          argument++;
     }

     *arg_first = '\0';

     while ( isspace(*argument) )
          argument++;

     return argument;
}

int linecount( char *str )
{
     int cnt = 0;

     while (*str)
          if ( *(str++) == '\n' )
               cnt++;

     return cnt;
}

/* Working linedel from EmberMUD.. Ivan's and Calath's linedel both freaked. */
char *linedel(char *str, int line )
{
     int len, buflen;
     int count = 0;
     char *pOut, outbuf[4*MAX_STRING_LENGTH], buf[4*MAX_STRING_LENGTH];

     strcpy(buf, str);
     buflen = strlen(buf);
     outbuf[0] = '\0';
     pOut = outbuf;
     len = 0;
     if (line == 1)
     {
          *pOut = '\0';
          for ( ; buf[len] != '\r'; len++)
               continue;
          len++;
     }

     for ( ; len < buflen; len++)
     {
          if ( buf[len] == '\r')
          {
               count++;
               if (count == line-1)
               {
                    for (len++ ; len < buflen; len++ )
                         if ( buf[len] == '\r' )
                              break;
                    *pOut++ = '\r';
               }
               else
                    *pOut++ = buf[len];
          }
          else
               *pOut++ = buf[len];
     }
     *pOut = '\0';
     free_string(str);
     return str_dup(outbuf);

}

char *getline( char *str, char *buf )
{
     int tmp = 0;
     bool found = FALSE;

     while ( *str )
     {
          if ( *str == '\n' )
          {
               found = TRUE;
               break;
          }

          buf[tmp++] = *(str++);
     }

     if ( found )
     {
          if ( *(str + 1) == '\r' )
               str += 2;
          else
               str += 1;
     }

     buf[tmp] = '\0';

     return str;
}

char *numlineas( char *string )
{
     int cnt = 1;
     static char buf[MSL*2];
     char buf2[MSL], tmpb[MSL];

     buf[0] = '\0';

     while ( *string )
     {
          string = getline( string, tmpb );
          SNP( buf2, "{C%2d:{W>{w %s\n\r", cnt++, tmpb );
          strcat( buf, buf2 );
     }

     return buf;
}

/* New lineadd by Calath to prevent buffer overrun */
char *lineadd( char *string, char *newstr, int line )
{
     char *lineend, *linestart = string;
     char buf[MAX_STRING_LENGTH];
     char newline[MAX_INPUT_LENGTH];
     int bytes = 0, cnt = 1, done = FALSE;
     int len;
     SNP (newline, "%s\n\r", newstr);
     newstr = newline;
     if (strlen (string) + strlen (newstr) >= MAX_STRING_LENGTH - 4)
          return string;
     do
     {
          if (cnt == line)
          {
               strncpy (buf + bytes, newstr, strlen (newstr));
               bytes += strlen (newstr);
               cnt++;
               continue;
          }
          if ((lineend = strstr (linestart, "\n\r")) == NULL)
               done = TRUE;
          /* Can't get rid of this warning due to fscking strlen */
          len = lineend ? (lineend + 2) - linestart : strlen (linestart);
          strncpy (buf + bytes, linestart, len);
          bytes += len;
          linestart = lineend ? lineend + 2 : linestart;
          cnt++;
     }
     while (!done);
     buf[bytes] = '\0';
     free_string (string);
     return str_dup ( buf );
}

/*
 Implementation of a dynamically expanding buffer.

 Inspired by Russ Taylor's <rtaylor@efn.org> buffers in ROM 2.4b2.

 The buffer is primarily used for null-terminated character strings.

 A buffer is allocated with buffer_new, written to using buffer_strcat,
 cleared (if needed) using buffer_clear and free'ed using buffer_free.

 If BUFFER_DEBUG is defined, the buffer_strcat call is defined as having
 2 extra parameters, __LINE__ and __FILE__. These are then saved
 to the bug file if an overflow occurs.

 Erwin S. Andreasen <erwin@pip.dknet.dk>
*/

#define EMEM_SIZE -1 /* find_mem_size returns this when block is too large */
#define NUL '\0'

extern const int rgSizeList [MAX_MEM_LIST];

/* Find in rgSizeList a memory size at least this long */
int find_mem_size (int min_size)
{
     int i;

     for (i = 0; i < MAX_MEM_LIST; i++)
          if (rgSizeList[i] >= min_size)
               return rgSizeList[i];

	/* min_size is bigger than biggest allowable size! */

     return EMEM_SIZE;
}

/* Create a new buffer, of at least size bytes */

#ifndef BUFFER_DEBUG /* no debugging */
BUFFER * __buffer_new (int min_size)

#else				 /* debugging - expect filename and line */
     BUFFER * __buffer_new (int min_size, const char * file, unsigned line)
#endif
{
     int size;
     BUFFER *buffer;          

     size = find_mem_size (min_size);

     if (size == EMEM_SIZE)
     {
#ifdef BUFFER_DEBUG
          bugf ( "Buffer size too big: %d bytes (%s:%u).", min_size, file, line);
#else
          bugf ( "Buffer size too big: %d bytes.", min_size);
#endif
          abort();
     }

     buffer = alloc_mem (sizeof(BUFFER), "BUFFER" );

     buffer->size = size;
     buffer->data = alloc_mem (size, "BUFFER" );
     buffer->overflowed = FALSE;

     buffer->len = 0;

     return buffer;
}
/* __buf_new */

/* Add a string to a buffer. Expand if necessary */

#ifndef BUFFER_DEBUG /* no debugging */
void __buffer_strcat (BUFFER *buffer, const char *text)

#else				 /* debugging - expect filename and line */
     void __buffer_strcat (BUFFER *buffer, const char *text, const char * file, unsigned line)
#endif
{
     int new_size;
     int text_len;
     char *new_data;

     if (buffer->overflowed) /* Do not attempt to add anymore if buffer is already overflowed */
          return;
     if (!text) /* Adding NULL string ? */
          return;
     text_len = strlen(text);
     if (text_len == 0) /* Adding empty string ? */
          return;
     /* Will the combined len of the added text and the current text exceed our buffer? */
     if ((text_len+buffer->len+1) > buffer->size) /* expand? */
     {
          new_size = find_mem_size (buffer->size + text_len + 1);
          if (new_size == EMEM_SIZE) /* New size too big ? */
          {
#ifdef BUFFER_DEBUG
               bugf ( "Buffer overflow, wanted %d bytes (%s:%u).", text_len+buffer->len, file, line);
#else
               bugf ( "Buffer overflow, wanted %d bytes.",text_len+buffer->len);
#endif
               buffer->overflowed = TRUE;
               return;
          }

          /* Allocate the new buffer */
          new_data = alloc_mem (new_size, "BUFFER");
          /* Copy the current buffer to the new buffer */

          memcpy (new_data, buffer->data, buffer->len);
          free_mem (buffer->data, buffer->size, "BUFFER");
          buffer->data = new_data;
          buffer->size = new_size;

     }
     /* if */

     memcpy (buffer->data + buffer->len, text, text_len);	/* Start copying */
     buffer->len += text_len;	/* Adjust length */
     buffer->data[buffer->len] = NUL; /* Null-terminate at new end */

}
/* __buf_strcat */

/* Free a buffer */
void buffer_free (BUFFER *buffer)
{
	/* Free data */
     free_mem (buffer->data, buffer->size, "BUFFER");

	/* Free buffer */

     free_mem (buffer, sizeof(BUFFER), "BUFFER");
}

/* Clear a buffer's contents, but do not deallocate anything */

void buffer_clear (BUFFER *buffer)
{
     buffer->overflowed = FALSE;
     buffer->len = 0;
}

/* print stuff, append to buffer. safe. */
int bprintf (BUFFER *buffer, char *fmt, ...)
{
     char buf[MSL];
     va_list va;
     int res;

     va_start (va, fmt);
     res = vsnprintf (buf, MSL, fmt, va);
     va_end (va);

     if (res >= MSL-1)
     {
          buf[0] = NUL;
          bugf ("Overflow when bprintf'ing a string" );
     }
     else
          buffer_strcat (buffer, buf);
     return res;
}

/* strlen() that doesn't count any valid color codes -Zak */
/* Adapted to Sunder from EmberMud by Lotherius */
int cstrlen( const char *str )
{
     int numb=0;

     while (*str != '\0')
     {
          if (*str != '{')   /* If there's no {, add to the count  */
          {
               numb++;
               str++;
               continue;
          }
          str++;             /* If there _IS_ a {, check next char */
          switch (*str)
          {
               /* If it's \0, count the first { and get outta here */
          case '\0': numb++; return numb;
               /* If it doesn't take space, skip over it */
               /* / (newline char) doesn't really fit in here, but we'll count it anyway. */
          case 'x': case 'b': case 'c': case 'd': case 'g': case 'm':
          case 'r': case 'w': case 'y': case 'B': case 'C': case 'G':
          case 'M': case 'R': case 'W': case 'Y': case 'D': case '*':
          case '/': case '3': case '4': case '&': str++; break;
               /* If it's not a color code, count the following char (but not the {), and advance */
          default: numb++; str++; break;
          }
          continue;
     }
     return numb;
}

/* We have to emulate the functionaility of strlcat and strlcpy on Win32 and Linux */
/* This should also work on any OS that doesn't have strlcat or strlcpy. */
#ifndef __FreeBSD__
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
     register char *d = dst;
     register const char *s = src;
     register size_t n = siz;

     /* Copy as many bytes as will fit */
     if (n != 0 && --n != 0)
     {
          do
          {
               if ((*d++ = *s++) == 0)
                    break;
          }
          while (--n != 0);
     }

     /* Not enough room in dst, add NUL and traverse rest of src */
     if (n == 0)
     {
          if (siz != 0)
               *d = '\0';              /* NUL-terminate dst */
          while (*s++)
               ;
     }
     return(s - src - 1);    /* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(initial dst) + strlen(src); if retval >= siz,
 * truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
     register char *d = dst;
     register const char *s = src;
     register size_t n = siz;
     size_t dlen;
     /* Find the end of dst and adjust bytes left but don't go past end */
     while (n-- != 0 && *d != '\0')
          d++;
     dlen = d - dst;
     n = siz - dlen;
     if (n == 0)
          return(dlen + strlen(s));
     while (*s != '\0')
     {
          if (n != 1)
          {
               *d++ = *s;
               n--;
          }
          s++;
     }
     *d = '\0';
     return(dlen + (s - src));       /* count does not include NUL */
}
#endif

/*
 * This function is simply here to provide a place to test new code
 * without having to make new calls for it in interp.c, etc before it
 * is production ready. This is called from within the mud by "test"
 * by the imp only.
 */
void do_testfunc ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "No test code is ready.\n\r", ch );
     return;
}

