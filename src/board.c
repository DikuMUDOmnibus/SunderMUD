/*
 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@pip.dknet.dk
 =====================================================================

 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.

 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.

 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.

 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.

 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.

 Note that write_level MUST be >= read_level or else there will be strange
 output in certain functions.

 Board DEFAULT_BOARD must be at least readable by *everyone*.
*/

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
#define L_HER LEVEL_HERO
#define L_IMM LEVEL_IMMORTAL
DECLARE_DO_FUN ( do_help );
DECLARE_DO_FUN ( do_afk );
#define L_SUP (MAX_LEVEL - 1)

/*
Short Name,	Long Name,		read_level, 	write_level,
def_recipient,	DEF_ACTION,		expire, 	NULL, UNUSED
*/

BOARD_DATA          boards[MAX_BOARD] =
{
     {"Announce", "Announcements from Immortals",    0,     L_IMM, "all", DEF_NORMAL,  60, NULL, FALSE },
     {"Gods",     "Immortal & Heros Only",           L_HER, L_HER, "all", DEF_NORMAL,  45, NULL, FALSE },
     {"General",  "General discussion",              0,     1,     "all", DEF_INCLUDE, 21, NULL, FALSE },
     {"The_Inn",  "The Looney Elf Inn (Role-Play)",  0,     2,     "all", DEF_NORMAL,  21, NULL, FALSE },
     {"Ideas",    "Suggestion for Improvement",      0,     2,     "all", DEF_NORMAL,  28, NULL, FALSE },
     {"Bugs",     "Typos, bugs, errors",             0,     1,     "imm", DEF_NORMAL,  60, NULL, FALSE },
     {"Personal", "Personal messages",               0,     1,     "all", DEF_EXCLUDE, 28, NULL, FALSE }
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char         *szFinishPrompt =
     "({GC{x)ontinue, ({GD{x)elete Line ({GV{x)iew, ({GP{x)ost or ({GF{x)orget it?";

long                last_note_stamp = 0;	/* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

static bool         next_board ( CHAR_DATA * ch );

/* recycle a note */
void free_note ( NOTE_DATA * note )
{
     if ( note->sender )
          free_string ( note->sender );
     if ( note->to_list )
          free_string ( note->to_list );
     if ( note->subject )
          free_string ( note->subject );
     if ( note->date )		/* was note->datestamp for some reason */
          free_string ( note->date );
     if ( note->text )
          free_string ( note->text );

     note->next = note_free;
     note_free = note;
}

/* allocate memory for a new note or recycle */
NOTE_DATA          *new_note (  )
{
     NOTE_DATA          *note;

     if ( note_free )
     {
          note = note_free;
          note_free = note_free->next;
     }
     else
          note = alloc_mem ( sizeof ( NOTE_DATA ), "NOTE_DATA" );

    /* Zero all the field - Envy does not gurantee zeroed memory */
     note->next = NULL;
     note->sender = NULL;
     note->expire = 0;
     note->to_list = NULL;
     note->subject = NULL;
     note->date = NULL;
     note->date_stamp = 0;
     note->text = NULL;

     return note;
}

/* append this note to the given file */
static void append_note ( FILE * fp, NOTE_DATA * note )
{
     fprintf ( fp, "Sender  %s~\n", note->sender );
     fprintf ( fp, "Date    %s~\n", note->date );
     fprintf ( fp, "Stamp   %ld\n", note->date_stamp );
     fprintf ( fp, "Expire  %ld\n", note->expire );
     fprintf ( fp, "To      %s~\n", note->to_list );
     fprintf ( fp, "Subject %s~\n", note->subject );
     fprintf ( fp, "Text\n%s~\n\n", note->text );
}

/* Save a note in a given board */
void finish_note ( BOARD_DATA * board, NOTE_DATA * note )
{
     FILE               *fp;
     NOTE_DATA          *p;
     char                filename[200];

    /* The following is done in order to generate unique date_stamps */

     if ( last_note_stamp >= current_time )
          note->date_stamp = ++last_note_stamp;
     else
     {
          note->date_stamp = current_time;
          last_note_stamp = current_time;
     }

     if ( board->note_first )	/* are there any notes in there now? */
     {
          for ( p = board->note_first; p->next; p = p->next );			/* empty */

          p->next = note;
     }
     else			/* nope. empty list. */
          board->note_first = note;

     /* append note to note file */

     SNP ( filename, "%s%s", NOTE_DIR, board->short_name );

     fp = fopen ( filename, "a" );
     if ( !fp )
     {
          bugf ( "Could not open one of the note files in append mode" );
          board->changed = TRUE;	/* set it to TRUE hope it will be OK later? */
          return;
     }

     append_note ( fp, note );
     fclose ( fp );
}

/* Find the number of a board */
int board_number ( const BOARD_DATA * board )
{
     int                 i;

     for ( i = 0; i < MAX_BOARD; i++ )
          if ( board == &boards[i] )
               return i;

     return -1;
}

/* Find a board number based on  a string */
int board_lookup ( const char *name )
{
     int                 i;

     for ( i = 0; i < MAX_BOARD; i++ )
          if ( !str_cmp ( boards[i].short_name, name ) )
               return i;

     return -1;
}

/* Remove list from the list. Do not free note */
static void unlink_note ( BOARD_DATA * board, NOTE_DATA * note )
{
     NOTE_DATA          *p;

     if ( board->note_first == note )
          board->note_first = note->next;
     else
     {
          for ( p = board->note_first; p && p->next != note;
                p = p->next );
          if ( !p )
               bugf ( "unlink_note: could not find note." );
          else
               p->next = note->next;
     }
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
static NOTE_DATA   *find_note ( CHAR_DATA * ch, BOARD_DATA * board,
				int num )
{
     int                 count = 0;
     NOTE_DATA          *p;

     for ( p = board->note_first; p; p = p->next )
          if ( ++count == num )
               break;

     if ( ( count == num ) && is_note_to ( ch, p ) )
          return p;
     else
          return NULL;

}

/* save a single board */
static void save_board ( BOARD_DATA * board )
{
     FILE               *fp;
     char                filename[200];
     NOTE_DATA          *note;

     SNP ( filename, "%s%s", NOTE_DIR, board->short_name );

     fp = fopen ( filename, "w" );
     if ( !fp )
     {
          bugf ("Error writing to: %s", filename );
     }
     else
     {
          for ( note = board->note_first; note; note = note->next )
               append_note ( fp, note );

          fclose ( fp );
     }
}

/* Show one not to a character */
static void show_note_to_char ( CHAR_DATA * ch, NOTE_DATA * note, int num )
{
     BUFFER *buf;

     buf = buffer_new(1000);

     bprintf ( buf,
               "{YBoard: {w%s{x\n\r[{W%4d{x] {Y%s{x: {G%s{x\n\r{YDate{x:  %s\n\r{YTo{x: %s\n\r{G==========================================================================={x\n\r%s\n\r",
               ch->pcdata->board->short_name, num, note->sender, note->subject,
               note->date, note->to_list, note->text );

     page_to_char(buf->data, ch);
     buffer_free(buf);
}

/* Save changed boards */
void save_notes (  )
{
     int                 i;

     for ( i = 0; i < MAX_BOARD; i++ )
          if ( boards[i].changed )	/* only save changed boards */
               save_board ( &boards[i] );
}

/* Load a single board */
static void load_board ( BOARD_DATA * board )
{
     FILE               *fp, *fp_archive;
     NOTE_DATA          *last_note;
     char                filename[200];

     SNP ( filename, "%s%s", NOTE_DIR, board->short_name );

     fp = fopen ( filename, "r" );

    /* Silently return */
     if ( !fp )
          return;

    /* Start note fetching. copy of db.c:load_notes() */

     last_note = NULL;

     for ( ;; )
     {
          NOTE_DATA          *pnote;
          char                letter;

          do
          {
               letter = getc ( fp );
               if ( feof ( fp ) )
               {
                    fclose ( fp );
                    return;
               }
          }
          while ( isspace ( letter ) );
          ungetc ( letter, fp );

          pnote = alloc_perm ( sizeof ( *pnote ), "pnote:load_board" );

          if ( str_cmp ( fread_word ( fp ), "sender" ) )
               break;
          pnote->sender = fread_string ( fp );

          if ( str_cmp ( fread_word ( fp ), "date" ) )
               break;
          pnote->date = fread_string ( fp );

          if ( str_cmp ( fread_word ( fp ), "stamp" ) )
               break;
          pnote->date_stamp = fread_number ( fp );

          if ( str_cmp ( fread_word ( fp ), "expire" ) )
               break;
          pnote->expire = fread_number ( fp );

          if ( str_cmp ( fread_word ( fp ), "to" ) )
               break;
          pnote->to_list = fread_string ( fp );

          if ( str_cmp ( fread_word ( fp ), "subject" ) )
               break;
          pnote->subject = fread_string ( fp );

          if ( str_cmp ( fread_word ( fp ), "text" ) )
               break;
          pnote->text = fread_string ( fp );

          pnote->next = NULL;	/* jic */

          if ( pnote->expire < current_time )
          {
               char                archive_name[200];

               SNP ( archive_name, "%s%s.old", NOTE_DIR,
                     board->short_name );
               fp_archive = fopen ( archive_name, "a" );
               if ( !fp_archive )
                    bugf ( "Could not open archive boards for writing" );
               else
               {
                    append_note ( fp_archive, pnote );
                    fclose ( fp_archive );	/* it might be more efficient to close this later */
               }

               free_note ( pnote );
               board->changed = TRUE;
               continue;

          }

          if ( board->note_first == NULL )
               board->note_first = pnote;
          else
               last_note->next = pnote;

          last_note = pnote;
     }

     bugf ( "Load_notes: bad key word." );
     return;			/* just return */
}

/* Initialize structures. Load all boards. */
void load_boards (  )
{
     int                 i;
     for ( i = 0; i < MAX_BOARD; i++ )
          load_board ( &boards[i] );
}

/* Returns TRUE if the specified note is address to ch */
bool is_note_to ( CHAR_DATA * ch, NOTE_DATA * note )
{
     if ( !str_cmp ( ch->name, note->sender ) )
          return TRUE;

     if ( is_full_name ( "all", note->to_list ) )
          return TRUE;

     if ( IS_IMMORTAL ( ch )
          && ( is_full_name ( "imm", note->to_list )
               || is_full_name ( "imms", note->to_list )
               || is_full_name ( "immortal", note->to_list )
               || is_full_name ( "god", note->to_list )
               || is_full_name ( "gods", note->to_list )
               || is_full_name ( "admin", note->to_list )
               || is_full_name ( "admins", note->to_list )
               || is_full_name ( "staff", note->to_list )
               || is_full_name ( "immortals", note->to_list ) ) )
          return TRUE;

     if ( ( get_trust ( ch ) == MAX_LEVEL )
          && ( is_full_name ( "imp", note->to_list )
               || is_full_name ( "imps", note->to_list )
               || is_full_name ( "implementor", note->to_list )
               || is_full_name ( "implementors", note->to_list ) ) )
          return TRUE;

     if ( is_full_name ( ch->name, note->to_list ) )
          return TRUE;

    /* Allow a note to e.g. 40 to send to characters level 40 and above */
     if ( is_number ( note->to_list ) &&
          get_trust ( ch ) >= atoi ( note->to_list ) )
          return TRUE;

     return FALSE;
}

/* return max # of notes on board */
int max_note_num ( CHAR_DATA * ch, BOARD_DATA * board )
{
     NOTE_DATA          *note;
     int                 count = 0;

     if ( board->read_level > get_trust ( ch ) )
          return BOARD_NOACCESS;

     for ( note = board->note_first; note; note = note->next )
          if ( is_note_to ( ch, note ) )
               count++;

     return count;
}

/* Return the number of unread notes 'ch' has in 'board' */

/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes ( CHAR_DATA * ch, BOARD_DATA * board )
{
     NOTE_DATA          *note;
     time_t              last_read;
     int                 count = 0;

     if ( board->read_level > get_trust ( ch ) )
          return BOARD_NOACCESS;

     last_read = ch->pcdata->last_note[board_number ( board )];

     for ( note = board->note_first; note; note = note->next )
          if ( is_note_to ( ch, note ) && ( ( long ) last_read <
                                            ( long ) note->
                                            date_stamp ) )
               count++;

     return count;
}

/*
 * COMMANDS
 */

/* Start writing a note */
static void do_nwrite ( CHAR_DATA * ch, char *argument )
{
     char               *strtime;

     if ( IS_NPC ( ch ) )	/* NPC cannot post notes */
          return;

     if ( get_trust ( ch ) < ch->pcdata->board->write_level )
     {
          send_to_char ( "You cannot post notes on this board.\n\r", ch );
          return;
     }

    /* continue previous note, if any text was written */
     if ( ch->pcdata->in_progress &&
          ( !ch->pcdata->in_progress->text ) )
     {
          send_to_char ( "Note in progress cancelled because you did not manage to write any text \n\r"
                         "before losing link.\n\r\n\r", ch );
          free_note ( ch->pcdata->in_progress );
          ch->pcdata->in_progress = NULL;
     }

     if ( !ch->pcdata->in_progress )
     {
          ch->pcdata->in_progress = new_note (  );
          ch->pcdata->in_progress->sender = str_dup ( ch->name );

	/* convert to ascii. ctime returns a string which last character is \n, so remove that */
          strtime = ctime ( &current_time );
          strtime[strlen ( strtime ) - 1] = '\0';

          ch->pcdata->in_progress->date = str_dup ( strtime );
     }

     do_afk ( ch, "" );

     act ( "{G$n starts writing a note.{x", ch, NULL, NULL,
           TO_ROOM );

    /* Begin writing the note ! */
     form_to_char ( ch,
                    "You are now %s a new note on the {W%s{x board.\n\r"
                    "Please turn {ROFF{x your triggers and aliases if using a client (Such as ZMud, tt++ or MushClient)!\n\r\n\r",
                    ch->pcdata->in_progress->
                    text ? "continuing" : "posting",
                    ch->pcdata->board->short_name );

     form_to_char ( ch, "{YFrom{x:    %s\n\r\n\r", ch->name );

     if ( !ch->pcdata->in_progress->text )	/* Are we continuing an old note or not? */
     {
          switch ( ch->pcdata->board->force_type )
          {
          case DEF_NORMAL:
               form_to_char ( ch,
                              "If you press Return, default recipient {W%s{x will be chosen.\n\r",
                              ch->pcdata->board->names );
               break;
          case DEF_INCLUDE:
               form_to_char ( ch,
                              "The recipient list MUST include {W%s{x. If not, it will be added automatically.\n\r",
                              ch->pcdata->board->names );
               break;

          case DEF_EXCLUDE:
               form_to_char ( ch,
                              "The recipient of this note must NOT include: {W%s{x.",
                              ch->pcdata->board->names );
               break;
          }

          send_to_char ( "\n\r{YTo{x:      ", ch );

          ch->desc->connected = CON_NOTE_TO;
	/* nanny takes over from here */

     }
     else			/* we are continuing, print out all the fields and the note so far */
     {
          form_to_char ( ch,
                         "{YTo{x:      %s\n\r"
                         "{YExpires{x: %s\n\r"
                         "{YSubject{x: %s\n\r",
                         ch->pcdata->in_progress->to_list,
                         ctime ( &ch->pcdata->in_progress->expire ),
                         ch->pcdata->in_progress->subject );
          send_to_char ( "{GYour note so far:{x\n\r", ch );
          send_to_char ( ch->pcdata->in_progress->text, ch );

          send_to_char
               ( "\n\rEnter text. Type {Y~{x or {YEND{x on an empty line to end note.\n\r{C=={c====================================================-{x\n\r",
                 ch );
          ch->desc->connected = CON_NOTE_TEXT;

     }

}

/* Read next note in current group. If no more notes, go to next board */
static void do_nread ( CHAR_DATA * ch, char *argument )
{
     NOTE_DATA          *p;
     int                 count = 0, number, unread;
     time_t             *last_note = &ch->pcdata->last_note[board_number ( ch->pcdata->board )];

     if ( !str_cmp ( argument, "again" ) )
     {				/* read last note again */

     }
     else if ( is_number ( argument ) )
     {
          number = atoi ( argument );

          for ( p = ch->pcdata->board->note_first; p; p = p->next )
               if ( ++count == number )
                    break;

          if ( !p || !is_note_to ( ch, p ) )
               send_to_char ( "No such note.\n\r", ch );
          else
          {
               show_note_to_char ( ch, p, count );
               *last_note = UMAX ( *last_note, p->date_stamp );
          }
     }
     else			/* just next one */
     {
          count = 1;
          for ( p = ch->pcdata->board->note_first; p;
                p = p->next, count++ )
               if ( ( p->date_stamp > *last_note ) &&
                    is_note_to ( ch, p ) )
               {
                    show_note_to_char ( ch, p, count );
		/* Advance if new note is newer than the currently newest for that char */
                    *last_note = UMAX ( *last_note, p->date_stamp );
                    return;
               }

          send_to_char ( "No unread notes in this board.\n\r", ch );

          if ( next_board ( ch ) )
          {
               unread = unread_notes (ch,ch->pcdata->board);
               form_to_char ( ch, "Changed to board {c%s{w, with {c%d{w unread notes.\n\r",
                              ch->pcdata->board->short_name, unread );
          }
          else /* Run through one more time to see if a note was posted on earlier board */
          {
               if ( next_board ( ch ) )
               {
	            unread = unread_notes (ch,ch->pcdata->board);
                    form_to_char ( ch, "Changed to board {c%s{w, with {c%d{w unread notes.\n\r",
                                   ch->pcdata->board->short_name, unread );
               }
               else
               {
		    unread = unread_notes(ch, ch->pcdata->board);
		    if (unread > 0)
                         form_to_char ( ch, "Changed to board {c%s{w, with {c%d{w unread notes.\n\r",
                                        ch->pcdata->board->short_name, unread );
		    else
                         form_to_char ( ch, "No more unread notes on any board.\n\r" );
               }
          }
     }
}

/* Remove a note */
static void do_nremove ( CHAR_DATA * ch, char *argument )
{
     NOTE_DATA          *p;

     if ( !is_number ( argument ) )
     {
          send_to_char ( "Remove which note?\n\r", ch );
          return;
     }

     p = find_note ( ch, ch->pcdata->board, atoi ( argument ) );
     if ( !p )
     {
          send_to_char ( "No such note.\n\r", ch );
          return;
     }

     if ( str_cmp ( ch->name, p->sender ) )
     {
          if ( get_trust ( ch ) != MAX_LEVEL )
          {
               send_to_char ( "You are not authorized to remove this note.\n\r", ch );
               return;
          }
          send_to_char ( "{RThird party note removal.{x\n\r", ch );
          log_string ( "3rd Party note from %s removed by %s on board %s.",
                       p->sender, ch->name,
                       ch->pcdata->board->short_name );
     }
     unlink_note ( ch->pcdata->board, p );
     free_note ( p );
     send_to_char ( "Note removed!\n\r", ch );
     save_board ( ch->pcdata->board );	/* save the board */
}

/* List all notes or if argument given, list N of the last notes */

/* Shows REAL note numbers! */
static void do_nlist ( CHAR_DATA * ch, char *argument )
{
     int                 count = 0, show = 0, num = 0, has_shown = 0;
     time_t              last_note;
     NOTE_DATA          *p;
     BUFFER		*buf;

     buf = buffer_new(1000);

     if ( is_number ( argument ) )	/* first, count the number of notes */
     {
          show = atoi ( argument );

          for ( p = ch->pcdata->board->note_first; p; p = p->next )
               if ( is_note_to ( ch, p ) )
                    count++;
     }

     buffer_strcat ( buf, "{GNotes on this board:{x\n\r"
                     "{gNum  Author        Subject{x\n\r"
                     "{C={c=-  {C={c==========-  {C={c========================-{w\n\r" );

     last_note =	ch->pcdata->last_note[board_number ( ch->pcdata->board )];

     for ( p = ch->pcdata->board->note_first; p; p = p->next )
     {
          num++;
          if ( is_note_to ( ch, p ) )
          {
               has_shown++;	/* note that we want to see X VISIBLE note, not just last X */
               if ( !show || ( ( count - show ) < has_shown ) )
               {
                    bprintf ( buf, "{W%3d{x>{G%c{w%-13s {W%s{x\n\r",
                              num,
                              last_note < p->date_stamp ? '*' : ' ',
                              p->sender, p->subject );
               }
          }

     }

     page_to_char(buf->data, ch);

     buffer_free(buf);
}

/* catch up with some notes */
static void do_ncatchup ( CHAR_DATA * ch, char *argument )
{
     NOTE_DATA          *p;

    /* Find last note */
     for ( p = ch->pcdata->board->note_first; p && p->next;
           p = p->next );

     if ( !p )
          send_to_char ( "Alas, there are no notes in that board.\n\r", ch );
     else
     {
          ch->pcdata->last_note[board_number ( ch->pcdata->board )] =
               p->date_stamp;
          send_to_char ( "All mesages skipped.\n\r", ch );
     }
}

/* Dispatch function for backwards compatibility */
void do_note ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

     argument = one_argument ( argument, arg );

     if ( ( !arg[0] ) || ( !str_cmp ( arg, "read" ) ) )	/* 'note' or 'note read X' */
          do_nread ( ch, argument );

     else if ( !str_cmp ( arg, "list" ) )
          do_nlist ( ch, argument );

     else if ( !str_cmp ( arg, "write" ) )
          do_nwrite ( ch, argument );

     else if ( !str_cmp ( arg, "remove" ) )
          do_nremove ( ch, argument );

     else if ( !str_cmp ( arg, "purge" ) )
          send_to_char ( "Obsolete.\n\r", ch );

     else if ( !str_cmp ( arg, "archive" ) )
          send_to_char ( "Obsolete.\n\r", ch );

     else if ( !str_cmp ( arg, "catchup" ) )
          do_ncatchup ( ch, argument );
     else
          do_help ( ch, "note" );
}

/* Show all accessible boards with their numbers of unread messages OR
   change board. New board name can be given as a number or as a name (e.g.
    board personal or board 4 */
void do_board ( CHAR_DATA * ch, char *argument )
{
     int                 i, count, number;

     if ( IS_NPC ( ch ) )
          return;

     if ( !argument[0] )		/* show boards */
     {
          int                 unread;
          int                 maxnum;

          count = 1;
          send_to_char  ( "{GNum         Name Unread Last   Description{x\n\r"
                          "{C={c=- {C={c==========- {C={c====- {C={c====- {C={c=====================-{x\n\r", ch );
          for ( i = 0; i < MAX_BOARD; i++ )
          {
               unread = unread_notes ( ch, &boards[i] );               /* how many unread notes? */
               maxnum = max_note_num ( ch, &boards[i] );
               if ( unread != BOARD_NOACCESS )
               {
                    form_to_char ( ch, "{W%2d{x> {g%12s{x {c[{%c%4d{c] {c[{W%4d{c] {W%s{w\n\r",
                                   count, boards[i].short_name, unread ? 'R' : 'w', unread, maxnum,
                                   boards[i].long_name );
                    count++;
               }
               /* if has access */
          }
          /* for each board */
          form_to_char ( ch, "\n\rYou current board is {W%s{x.\n\r", ch->pcdata->board->short_name );

          if ( ch->pcdata->board->read_level > get_trust ( ch ) )
               send_to_char ( "You cannot read nor write notes on this board.\n\r", ch );
          else if ( ch->pcdata->board->write_level > get_trust ( ch ) )
               send_to_char ( "You can only read notes from this board.\n\r", ch );
          // Uncomment the following two lines if you desire to let ppl know the obvious.
          //          else
          //               send_to_char ( "You can both read and write on this board.\n\r", ch );
          return;
     }
     if ( !str_cmp ( argument, "auto" ) ) // do auto board
     {
          int unread = 0;
          bool onefound = FALSE;

          send_to_char ( "Checking message boards....", ch );
          for ( i = 0; i < MAX_BOARD; i++ )
          {
               unread = unread_notes ( ch, &boards[i] );
               if ( unread != BOARD_NOACCESS && unread > 0 )
               {
                    if ( !onefound )
                    {
                         onefound = TRUE;
                         send_to_char ( "\n\r", ch );
                    }
                    form_to_char ( ch, "Unread Notes On: {w%12s{w: {C({G%4d{C){w\n\r",
                          boards[i].short_name, unread );
               }
          }
          if ( !onefound )
               send_to_char ( " No new messages found.\n\r", ch );
          return;
     }

     /* if empty argument Change board based on its number */
     if ( is_number ( argument ) )
     {
          count = 0;
          number = atoi ( argument );
          for ( i = 0; i < MAX_BOARD; i++ )
               if ( unread_notes ( ch, &boards[i] ) !=
                    BOARD_NOACCESS )
                    if ( ++count == number )
                         break;

          if ( count == number )	/* found the board.. change to it */
          {
               ch->pcdata->board = &boards[i];
               form_to_char ( ch, "Current board changed to {W%s{x. %s.\n\r",
                     boards[i].short_name, ( get_trust ( ch ) < boards[i].write_level ) ? "You can only read here" :
                     "You can both read and write here" );
          }
          else			/* so such board */
               send_to_char ( "No such board.\n\r", ch );

          return;
     }

     /* Non-number given, find board with that name */

     for ( i = 0; i < MAX_BOARD; i++ )
          if ( !str_cmp ( boards[i].short_name, argument ) )
               break;

     if ( i == MAX_BOARD )
     {
          send_to_char ( "No such board.\n\r", ch );
          return;
     }

     /* Does ch have access to this board? */
     if ( unread_notes ( ch, &boards[i] ) == BOARD_NOACCESS )
     {
          send_to_char ( "No such board.\n\r", ch );
          return;
     }

     ch->pcdata->board = &boards[i];
     form_to_char ( ch, "Current board changed to {W%s{x. %s.\n\r",
                    boards[i].short_name, ( get_trust ( ch ) < boards[i].write_level ) ? "You can only read here" :
                    "You can both read and write here" );
}

/* Send a note to someone on the personal board */
void personal_message ( const char *sender, const char *to,
			const char *subject, const int expire_days,
			const char *text )
{
     make_note ( "Personal", sender, to, subject, expire_days, text );
}

void make_note ( const char *board_name, const char *sender,
		 const char *to, const char *subject,
		 const int expire_days, const char *text )
{
     int                 board_index = board_lookup ( board_name );
     BOARD_DATA         *board;
     NOTE_DATA          *note;
     char               *strtime;

     if ( board_index == BOARD_NOTFOUND )
     {
          bugf ( "make_note: board not found" );
          return;
     }

     if ( strlen ( text ) > MAX_NOTE_TEXT )
     {
          bugf ( "make_note: text too long (%d bytes)", strlen ( text ) );
          return;
     }

     board = &boards[board_index];

     note = new_note (  );	/* allocate new note */

     note->sender = str_dup ( sender );
     note->to_list = str_dup ( to );
     note->subject = str_dup ( subject );
     note->expire = current_time + expire_days * 60 * 60 * 24;
     note->text = str_dup ( text );

     /* convert to ascii. ctime returns a string which last character is \n, so remove that */
     strtime = ctime ( &current_time );
     strtime[strlen ( strtime ) - 1] = '\0';

     note->date = str_dup ( strtime );

     finish_note ( board, note );

}

/* tries to change to the next accessible board */
/* Lotherius changed to skip boards with 0 messages and to loop */
/* For this to work, DEFAULT_BOARD must remain board 0 */

static bool next_board ( CHAR_DATA * ch )
{
     int         i = board_number ( ch->pcdata->board ) + 1;

     while ( ( i < MAX_BOARD ) && ( unread_notes ( ch, &boards[i] ) < 1 ) )
          i++;

     if ( i == MAX_BOARD )
     {
          ch->pcdata->board = &boards[DEFAULT_BOARD];
          return FALSE;
     }
     else
     {
          ch->pcdata->board = &boards[i];
          return TRUE;
     }

}

void handle_con_note_to ( DESCRIPTOR_DATA * d, char *argument )
{
     char               buf[MSL];
     CHAR_DATA          *ch = d->character;

     if ( !ch->pcdata->in_progress )
     {
          d->connected = CON_PLAYING;
          bugf ( "nanny: In CON_NOTE_TO, but no note in progress" );
          return;
     }

     SLCPY ( buf, argument );

     switch ( ch->pcdata->board->force_type )
     {
     case DEF_NORMAL:		/* default field */
          if ( !buf[0] )		/* empty string? */
          {
               ch->pcdata->in_progress->to_list =
                    str_dup ( ch->pcdata->board->names );
               form_to_char ( ch, "Assumed default recipient: {W%s{x\n\r",
                              ch->pcdata->board->names );
          }
          else
               ch->pcdata->in_progress->to_list = str_dup ( buf );

          break;

     case DEF_INCLUDE:		/* forced default */
          if ( !is_full_name ( ch->pcdata->board->names, buf ) )
          {
               SLCAT ( buf, " " );
               SLCAT ( buf, ch->pcdata->board->names );
               ch->pcdata->in_progress->to_list = str_dup ( buf );

               form_to_char ( ch, "\n\rYou did not specify %s as recipient, so it was automatically added.\n\r"
                              "{YNew To{x :  %s\n\r",
                              ch->pcdata->board->names,
                              ch->pcdata->in_progress->to_list );
          }
          else
               ch->pcdata->in_progress->to_list = str_dup ( buf );
          break;

     case DEF_EXCLUDE:		/* forced exclude */
          if ( is_full_name ( ch->pcdata->board->names, buf ) )
          {
               form_to_char ( ch, "You are not allowed to send notes to %s on this board. Try again.\n\r"
                              "{YTo{x:      ", ch->pcdata->board->names );
               return;		/* return from nanny, not changing to the next state! */
          }
          else
               ch->pcdata->in_progress->to_list = str_dup ( buf );
          break;

     }

     send_to_char ( "\n\r{YSubject{x: ", ch );
     d->connected = CON_NOTE_SUBJECT;
}

void handle_con_note_subject ( DESCRIPTOR_DATA * d, char *argument )
{
     char                buf[MAX_INPUT_LENGTH];
     CHAR_DATA          *ch = d->character;

     if ( !ch->pcdata->in_progress )
     {
          d->connected = CON_PLAYING;
          bugf ( "nanny: In CON_NOTE_SUBJECT, but no note in progress" );
          return;
     }

     SLCPY ( buf, argument );

    /* Do not allow empty subjects */

     if ( !buf[0] )
     {
          send_to_char ( "Please enter a subject!\n\r", ch );
          send_to_char ( "{YSubject{x: ", ch );
     }
     else if ( strlen ( buf ) > 60 )
     {
          write_to_buffer ( d, "No, no. This is just the Subject. You are not writing the note yet.\n\r", 0 );
     }
     else
	/* advance to next stage */
     {
          ch->pcdata->in_progress->subject = str_dup ( buf );
          if ( IS_IMMORTAL ( ch ) )	/* immortals get to choose number of expire days */
          {
               form_to_char ( ch, "\n\rHow many days do you want this note to expire in?\n\r"
                              "Press Enter for default value for this board, {W%d{x days.\n\r"
                              "{YExpire{x:  ",
                              ch->pcdata->board->purge_days );
               d->connected = CON_NOTE_EXPIRE;
          }
          else
          {
               ch->pcdata->in_progress->expire =
                    current_time + ch->pcdata->board->purge_days * 24L * 3600L;
               form_to_char ( ch, "This note will expire %s\r",
                              ctime ( &ch->pcdata->in_progress->expire ) );
               send_to_char
                    ( "\n\rEnter text. Type {Y~{x or {YEND{x on an empty line to end note.\n\r"
                      "{C={c======================================================{x\n\r", ch );
               d->connected = CON_NOTE_TEXT;
          }
     }
}

void handle_con_note_expire ( DESCRIPTOR_DATA * d, char *argument )
{
     CHAR_DATA          *ch = d->character;
     char                buf[MAX_STRING_LENGTH];
     time_t              expire;
     int                 days;

     if ( !ch->pcdata->in_progress )
     {
          d->connected = CON_PLAYING;
          bugf ( "nanny: In CON_NOTE_EXPIRE, but no note in progress" );
          return;
     }

     /* Numeric argument. no tilde smashing */
     SLCPY ( buf, argument );
     if ( !buf[0] )		/* assume default expire */
          days = ch->pcdata->board->purge_days;
     else /* use this expire */ if ( !is_number ( buf ) )
     {
          send_to_char ( "Write the number of days!\n\r", ch);
          send_to_char ( "{YExpire{x:  ", ch );
          return;
     }
     else
     {
          days = atoi ( buf );
          if ( days <= 0 )
          {
               send_to_char ( "This is a positive MUD. Use positive numbers only! :)\n\r", ch );
               send_to_char ( "{YExpire{x:  ", ch );
               return;
          }
     }

     expire = current_time + ( days * 24L * 3600L );	/* 24 hours, 3600 seconds */

     ch->pcdata->in_progress->expire = expire;

    /* note that ctime returns XXX\n so we only need to add an \r */

     send_to_char
          ( "\n\rEnter text. Type {Y~{x or {YEND{x on an empty line to end note.\n\r"
            "{C={c======================================================{x\n\r",
            ch );

     d->connected = CON_NOTE_TEXT;
}

void handle_con_note_text ( DESCRIPTOR_DATA * d, char *argument )
{
     CHAR_DATA          *ch = d->character;
     char                buf[MAX_STRING_LENGTH];
     char                letter[4 * MAX_STRING_LENGTH];

     if ( !ch->pcdata->in_progress )
     {
          d->connected = CON_PLAYING;
          bugf ( "nanny: In CON_NOTE_TEXT, but no note in progress" );
          return;
     }

     /* First, check for EndOfNote marker */

     SLCPY ( buf, argument );
     if ( ( !str_cmp ( buf, "~" ) ) || ( !str_cmp ( buf, "END" ) ) )
     {
          write_to_buffer ( d, "\n\r\n\r", 0 );
          send_to_char ( szFinishPrompt, ch );
          write_to_buffer ( d, "\n\r", 0 );
          d->connected = CON_NOTE_FINISH;
          return;
     }

     /* Check for too long lines. Do not allow lines longer than 80 chars */

     if ( cstrlen ( buf ) > MAX_LINE_LENGTH )
     {
          write_to_buffer ( d, "Too long line rejected. Do NOT go over 80 characters (not counting colour)!\n\r",  0 );
          return;
     }
     if ( strlen ( buf ) > MAX_LINE_LENGTH*2)
     {
          write_to_buffer ( d, "Too long line rejected. Too many colour codes were used!\n\r", 0 );
          return;
     }

     /* Not end of note. Copy current text into temp buffer,
      * add new line, and copy back */

     /* How would the system react to strcpy( , NULL) ? */
     if ( ch->pcdata->in_progress->text )
     {
          SLCPY ( letter, ch->pcdata->in_progress->text );
          free_string ( ch->pcdata->in_progress->text );
          ch->pcdata->in_progress->text = NULL;	/* be sure we don't free it twice */
     }
     else
          SLCPY ( letter, "" );

    /* Check for overflow */

     if ( ( strlen ( letter ) + strlen ( buf ) ) > MAX_NOTE_TEXT )
     {				/* Note too long, take appropriate steps */
          write_to_buffer ( d, "Note too long!\n\r", 0 );
          free_note ( ch->pcdata->in_progress );
          ch->pcdata->in_progress = NULL;	/* important */
          d->connected = CON_PLAYING;
          return;
     }

    /* Add new line to the buffer */

     SLCAT ( letter, buf );
     SLCAT ( letter, "\r\n" );	/* new line. \r first to make note files better readable */

    /* allocate dynamically */
     ch->pcdata->in_progress->text = str_dup ( letter );
}

void handle_con_note_finish ( DESCRIPTOR_DATA * d, char *argument )
{

     CHAR_DATA          *ch = d->character;
     int                 tot_len, i, count = 0;
     char                newbuf[MAX_NOTE_TEXT + 2];
     char                buf[MAX_NOTE_TEXT + 2];

     if ( !ch->pcdata->in_progress )
     {
          d->connected = CON_PLAYING;
          bugf ( "nanny: In CON_NOTE_FINISH, but no note in progress" );
          return;
     }

     switch ( tolower ( argument[0] ) )
     {
     case 'c':			/* keep writing */
          write_to_buffer ( d, "Continuing note...\n\r", 0 );
          d->connected = CON_NOTE_TEXT;
          break;

     case 'v':			/* view note so far */
          if ( ch->pcdata->in_progress->text )
          {
               send_to_char ( "{GText of your note so far:{x\n\r",  ch );
               page_to_char ( ch->pcdata->in_progress->text, ch );
          }
          else
               write_to_buffer ( d, "You have not written a thing!\n\r\n\r", 0 );
          send_to_char ( szFinishPrompt, ch );
          write_to_buffer ( d, "\n\r", 0 );
          break;

     case 'd':
          send_to_char ( "Deleting last line.\n\r", ch );

          SLCPY ( buf, ch->pcdata->in_progress->text );
          tot_len = strlen ( buf );
          if ( tot_len < 3 )	/*no lines left, just null or line terminators */
          {
               send_to_char ( "No lines left to delete.\n\r", ch );
               return;
          }
          for ( i = ( tot_len ); ( ( i >= 0 ) && ( count < 2 ) );  i-- )
          {
               if ( buf[i] == '\n' )
                    count++;
          }
          if ( !( i <= 0 ) )
          {
               strncpy ( newbuf, buf, ( i + 2 ) );
               newbuf[i + 2] = '\0';
          }
          else
               newbuf[0] = '\0';
          free_string ( ch->pcdata->in_progress->text );
          ch->pcdata->in_progress->text = str_dup ( newbuf );
          send_to_char ( "Ok.\n\r", ch );
          break;

     case 'p':			/* post note */
          finish_note ( ch->pcdata->board, ch->pcdata->in_progress );
          notify_message ( ch, NOTIFY_NEWNOTE, TO_ALL, NULL );
          write_to_buffer ( d, "Note posted.\n\r", 0 );
          d->connected = CON_PLAYING;
	/* remove AFK status */
          do_afk ( ch, "" );
          ch->pcdata->in_progress = NULL;
          act ( "{G$n finishes $s note.", ch, NULL, NULL, TO_ROOM );
          break;
     case 'f':
          write_to_buffer ( d, "Note cancelled!\n\r", 0 );
          free_note ( ch->pcdata->in_progress );
          ch->pcdata->in_progress = NULL;
          d->connected = CON_PLAYING;
          /* remove afk status */
          do_afk ( ch, "" );
          break;
     default:			/* invalid response */
          write_to_buffer ( d, "Huh? Valid answers are:\n\r\n\r", 0 );
          send_to_char ( szFinishPrompt, ch );
          write_to_buffer ( d, "\n\r", 0 );

     }
}
