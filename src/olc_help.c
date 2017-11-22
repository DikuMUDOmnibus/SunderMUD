/***************************************************************************
 *  File: olc_help.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This work is a derivative of Talen's post to the Merc Mailing List.    *
 *  It has been modified by Jason Dinkel to work with the new OLC.         *
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

#include "everything.h"
#include "olc.h"

#define HEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define EDIT_HELP(Ch, Help)	( Help = (HELP_DATA *)Ch->desc->pEdit )

/*
 * Help Editor Prototypes
 */
DECLARE_OLC_FUN ( hedit_create );
DECLARE_OLC_FUN ( hedit_delete );
DECLARE_OLC_FUN ( hedit_desc );
DECLARE_OLC_FUN ( hedit_level );
DECLARE_OLC_FUN ( hedit_keywords );
DECLARE_OLC_FUN ( hedit_show );
DECLARE_OLC_FUN ( hedit_save );

const struct olc_cmd_type hedit_table[] =
{

/*  {   command		function	}, */

     {"commands", show_commands},
     {"create", hedit_create},
     {"delete", hedit_delete},
     {"desc", hedit_desc},
     {"level", hedit_level},
     {"keywords", hedit_keywords},
     {"show", hedit_show},
     {"save", hedit_save},
     {"?", show_help},

     {"", 0,}
};

/*
 * Stupid leading space muncher fix             -Thoric
 */

/*
char *help_fix( char *text )
{
    char *fixed;
    if ( !text )
      return "";
    fixed = strip_cr(text);
    if ( fixed[0] == ' ' )
      fixed[0] = '.';
    return fixed;
}
*/

void hedit ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char                command[MAX_INPUT_LENGTH];
     int                 cmd;

     SLCPY ( arg, argument );
     argument = one_argument ( argument, command );

     if ( ch->pcdata->security == 0 )
          send_to_char ( "HEdit: Insufficient security to modify area.\n\r", ch );

     if ( command[0] == '\0' )
     {
          hedit_show ( ch, argument );
          return;
     }

     if ( !str_cmp ( command, "done" ) )
     {
          edit_done ( ch );
          return;
     }

     if ( ch->pcdata->security == 0 )
     {
          interpret ( ch, arg );
          return;
     }

    /* Search Table and Dispatch Command. */
     for ( cmd = 0; hedit_table[cmd].name[0] != '\0'; cmd++ )
     {
          if ( !str_cmp ( command, hedit_table[cmd].name ) )
          {
               ( *hedit_table[cmd].olc_fun ) ( ch, argument );
               return;
          }
     }

    /* Default to Standard Interpreter. */
     interpret ( ch, arg );
     return;
}

/* Entry point for editing help_data. */
void do_hedit ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     HELP_DATA          *iHelp;

     if ( IS_NPC ( ch ) )
          return;

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:  edit help <keywords>\n\r", ch );
          return;
     }
     else
     {
          for ( iHelp = help_first; iHelp; iHelp = iHelp->next )
          {
	    /*
	     * This help better not exist already!
	     */
               if ( is_name ( arg, iHelp->keyword ) )
               {
                    ch->desc->pEdit = ( void * ) iHelp;
                    ch->desc->editor = ED_HELP;
                    break;
               }
          }

          if ( !iHelp )
          {
               iHelp = new_help (  );
               iHelp->keyword = str_dup ( arg );

               if ( !help_first )
                    help_first = iHelp;
               if ( help_last )
                    help_last->next = iHelp;

               help_last = iHelp;
               iHelp->next = NULL;
               ch->desc->pEdit = ( void * ) iHelp;
               ch->desc->editor = ED_HELP;
          }
     }
     return;
}

HEDIT ( hedit_save )
{
     char		filename[MSL];
     FILE               *fp = NULL;
     HELP_DATA          *pHelp;

     SNP ( filename, "%s%s", DATA_DIR, HELP_FILE );

     log_string ( "Saving helps to %s", filename );

     if ( !( fp = fopen ( filename, "w" ) ) )
     {
          bugf ( "hedit_save: Failed to open %s", filename );
          perror ( filename );
          return FALSE;
     }
     
     for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
     {
          fprintf ( fp, "%d %s~\n%s~\n",
                    pHelp->level, pHelp->keyword, fix_string ( pHelp->text ) );
     }

     fprintf ( fp, "0 $~\n\n\n#$\n" );

     fclose ( fp );

     send_to_char ( "Saved.\n", ch );

     return TRUE;
}

HEDIT ( hedit_show )
{
     HELP_DATA          *pHelp;

     if ( !EDIT_HELP ( ch, pHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     form_to_char ( ch, 
                    "Seen at level: [%d]\n\r"
                    "Keywords:      [%s]\n\r"
                    "Text:\n\r%s\n\r",
                    pHelp->level, pHelp->keyword, pHelp->text );
     return FALSE;
}

HEDIT ( hedit_create )
{
     HELP_DATA          *iHelp;
     HELP_DATA          *NewHelp;

     if ( !EDIT_HELP ( ch, iHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax: create <keywords>\n\r", ch );
          return FALSE;
     }

    /*
     * This help better not exist already!
     */
     for ( iHelp = help_first; iHelp; iHelp = iHelp->next )
     {
          if ( is_name ( argument, iHelp->keyword ) )
          {
               send_to_char ( "That help file already exists.\n\r", ch );
               return FALSE;
          }
     }

     NewHelp = new_help (  );
     NewHelp->keyword = str_dup ( argument );

     if ( !help_first )		/* If it is we have a leak */
          help_first = NewHelp;
     if ( help_last )
          help_last->next = NewHelp;

     help_last = NewHelp;
     NewHelp->next = NULL;
     ch->desc->pEdit = ( void * ) NewHelp;
     ch->desc->editor = ED_HELP;

     form_to_char ( ch, "Created help with the keyword(s): %s\n\r", NewHelp->keyword );

     return TRUE;
}

HEDIT ( hedit_delete )
{
     HELP_DATA          *pHelp;
     HELP_DATA          *PrevHelp = NULL;

     if ( !EDIT_HELP ( ch, pHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax: delete <keyword>\n\r", ch );
          return FALSE;
     }

    /*
     * This help better exist
     */
     for ( pHelp = help_first; pHelp;
           PrevHelp = pHelp, pHelp = pHelp->next )
     {
          if ( is_name ( argument, pHelp->keyword ) )
               break;
     }

     if ( !pHelp )
     {
          send_to_char ( "That help file does not exist.\n\r", ch );
          return FALSE;
     }

     if ( pHelp == ( HELP_DATA * ) ch->desc->pEdit )
     {
          edit_done ( ch );
     }

     if ( !PrevHelp )		/* At first help file   */
     {
          help_first = pHelp->next;
          free_help ( pHelp );
     }
     else if ( !pHelp->next )	/* At the last help file */
     {
          help_last = PrevHelp;
          PrevHelp->next = NULL;
          free_help ( pHelp );
     }
     else			/* Somewhere else...    */
     {
          PrevHelp->next = pHelp->next;
          free_help ( pHelp );
     }

     send_to_char ( "Help file deleted.\n\r", ch );
     return TRUE;
}

HEDIT ( hedit_desc )
{
     HELP_DATA          *pHelp;

     if ( !EDIT_HELP ( ch, pHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     if ( argument[0] != '\0' )
     {
          send_to_char ( "Syntax:  desc\n\r", ch );
          return FALSE;
     }

     string_append ( ch, &pHelp->text );
     return TRUE;
}

HEDIT ( hedit_level )
{
     HELP_DATA          *pHelp;
     int                 value;

     if ( !EDIT_HELP ( ch, pHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     value = atoi ( argument );

     if ( argument[0] == '\0' || value < -1 )
     {
          send_to_char ( "Syntax:  level [level >= -1]\n\r", ch );
          return FALSE;
     }

     pHelp->level = value;
     send_to_char ( "Help level set.\n\r", ch );

     return TRUE;
}

HEDIT ( hedit_keywords )
{
     HELP_DATA          *pHelp;
     int                 i;
     int                 length;

     if ( !EDIT_HELP ( ch, pHelp ) )
     {
          send_to_char ( "Null help file.\n\r", ch );
          return FALSE;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  keywords <keywords>\n\r", ch );
          return FALSE;
     }

     length = strlen ( argument );
     for ( i = 0; i < length; i++ )
          argument[i] = toupper ( argument[i] );

     pHelp->keyword = str_dup ( argument );
     send_to_char ( "Help keywords set.\n\r", ch );
     return TRUE;
}

void do_hlist ( CHAR_DATA * ch, char *argument )
{
     int                 max, cnt;
     char                arg[MAX_INPUT_LENGTH];
     BUFFER             *buffer;
     bool                indx;
     bool                srch;
     HELP_DATA          *help;

     srch = FALSE;
     indx = FALSE;

     buffer = buffer_new ( 4096 );

     max = get_trust ( ch );

     argument = one_argument ( argument, arg );

     if ( !strcmp ( arg, "index" ) )
     {
          indx = TRUE;
          bprintf ( buffer, "{WHelp Topics with {G%s{W in the keyword:{w\n\r\n\r", argument );
     }
     else if ( !strcmp ( arg, "search" ) )
     {
          srch = TRUE;
          bprintf ( buffer, "{WHelp Topics matching your search for: {G%s{W:\n\r\n\r", argument );
     }

     for ( cnt = 0, help = help_first; help; help = help->next )
          if ( help->level <= max )
          {
               if ( ( indx == FALSE && srch == FALSE ) ||
                    ( ( indx == TRUE || srch == TRUE ) &&
                      !str_infix ( argument, help->keyword ) ) ||
                    ( srch == TRUE &&
                      !str_infix ( argument, help->text ) ) )
               {
                    bprintf ( buffer, " {W%s{x\n\r", help->keyword );
                    ++cnt;
               }
          }
     if ( cnt )
     {
          bprintf ( buffer, "\n\r%d pages found.\n\r", cnt );
     }
     else
          bprintf ( buffer, "None found.\n\r" );
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return;

}

