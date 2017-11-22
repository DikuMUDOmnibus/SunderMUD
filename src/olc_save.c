/**************************************************************************
 *  File: olc_save.c                                                       *
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

/* OLC_SAVE.C
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include "everything.h"
#include "olc.h"

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/* #define VERBOSE */

/*
 * Remove carriage returns from a line
 */
char *strip_cr ( char *str )
{
     static char         newstr[MAX_STRING_LENGTH];
     int                 i, j;

     for ( i = j = 0; str[i] != '\0'; i++ )
          if ( str[i] != '\r' )
          {
               newstr[j++] = str[i];
          }
     newstr[j] = '\0';
     return newstr;
}

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string ( const char *str )
{
     static char         strfix[MAX_STRING_LENGTH];
     int                 i;
     int                 o;

     if ( str == NULL )
          return '\0';

     for ( o = i = 0;( str[i + o] != '\0' && ( i + 1 ) <= MAX_STRING_LENGTH );i++ )
     {
          if ( str[i + o] == '\r' || str[i + o] == '~' )
               o++;
          strfix[i] = str[i + o];
     }
     strfix[i] = '\0';
     return strfix;
}

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(db_area.c).
 ****************************************************************************/
void save_area_list (  )
{
     FILE               *fp;
     AREA_DATA          *pArea;

     if ( ( fp = fopen ( AREA_LIST, "w" ) ) == NULL )
     {
          bugf ( "Save_area_list: fopen" );
          perror ( AREA_LIST );
     }
     else
     {
          for ( pArea = area_first; pArea; pArea = pArea->next )
          {
               fprintf ( fp, "%s\n", pArea->filename );
          }

          fprintf ( fp, "$\n" );
          fclose ( fp );
     }

     return;
}

/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 *
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 *
 * Doesn't actually do an fwrite, simply returns a value --lotherius
 * Dunno why he called it fwrite but I'll leave it this way. Misleading,
 * because fread actually does work on files, but fwrite doesn't?
 *
 */
char *fwrite_flag ( long flags, char buf[] )
{
     char                offset;
     char               *cp;

     buf[0] = '\0';

     if ( flags == 0 )
     {
          SLCPY ( buf, "0" );
          return buf;
     }

     /* 32 -- number of bits in a long */
     for ( offset = 0, cp = buf; offset < 32; offset++ )
          if ( flags & ( ( long ) 1 << offset ) )
          {
               if ( offset <= 'Z' - 'A' )
                    *( cp++ ) = 'A' + offset;
               else
                    *( cp++ ) = 'a' + offset - ( 'Z' - 'A' + 1 );
          }
     *cp = '\0';
     return buf;
}

/* *******
 * Save the race table
 * Lotherius 2002
 * *******/

void save_races ( )
{
     FILE              *fp = NULL;
     char               buf[MAX_STRING_LENGTH];
     int		race;

     SNP ( buf, "%s%s", DATA_DIR, RACE_FILE );

     if ( !( fp = fopen ( buf, "w" ) ) )
     {
          bugf ( "Could Not Open Race File to save!" );
          return;
     }

     for ( race = 0; race_table[race].name != NULL; race++ )
     {
          fprintf ( fp, "%s~\n", race_table[race].name );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].act, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].off, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].imm, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].res, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].vuln, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].form, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].parts, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].aff, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].detect, buf ) );
          fprintf ( fp, "%s ", fwrite_flag ( race_table[race].protect, buf ) );
          fprintf ( fp, "%d\n", race_table[race].encumbrance );
     }

    /* the EOF marker !MUST! be followed by a CR or it will
     * result in a "string too long" error on bootup. */

     fprintf ( fp, "$\n" );      /* EOF $ */
     fclose ( fp );
}
