/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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

/* command procedures needed */
DECLARE_DO_FUN ( do_quit );

/* prototypes needed */
void real_acomm args ( ( CHAR_DATA *ch, long flag, char *text ) );

/*
 * Local functions.
 */

char *scramble_word args ( ( char *argument, int modifier ) );

/* Text scrambler -- Altrag */

char *scramble ( char *argument, int modifier )
{
     char                word[MAX_INPUT_LENGTH];
     static char         final[2 * MAX_INPUT_LENGTH];
     char               *tmp;

     if ( modifier == 100 )
          return argument;

     final[0] = '\0';

     while ( argument != NULL && argument[0] != '\0' )
     {
          argument = one_argument_nl ( argument, word );
          if ( number_percent (  ) > modifier )
          {
               tmp = scramble_word ( word, modifier );
               SLCAT ( final, tmp );
          }
          else
               SLCAT ( final, word );
          SLCAT ( final, " " );
     }
     /* Whack last extra space */
     final[strlen ( final ) - 1] = '\0';

     return final;
}

char *scramble_word ( char *argument, int modifier )
{
     static char         arg[MAX_INPUT_LENGTH];
     sh_int              position;
     sh_int              conversion = 0;

     modifier %= number_range ( 80, 300 );

     for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
     {
          if ( argument[position] == '\0' )
          {
               arg[position] = '\0';
               return arg;
          }
          else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
          {
               conversion = -conversion + position - modifier + argument[position] - 'A';
               conversion = number_range ( conversion - 5, conversion + 5 );
               while ( conversion > 25 )
                    conversion -= 26;
               while ( conversion < 0 )
                    conversion += 26;
               arg[position] = conversion + 'A';
          }
          else if ( argument[position] >= 'a' && argument[position] <= 'z' )
          {
               conversion = -conversion + position - modifier + argument[position] - 'a';
               conversion = number_range ( conversion - 5, conversion + 5 );
               while ( conversion > 25 )
                    conversion -= 26;
               while ( conversion < 0 )
                    conversion += 26;
               arg[position] = conversion + 'a';
          }
          else if ( argument[position] >= '0' && argument[position] <= '9' )
          {
               conversion = -conversion + position - modifier + argument[position] - '0';
               conversion = number_range ( conversion - 2, conversion + 2 );
               while ( conversion > 9 )
                    conversion -= 10;
               while ( conversion < 0 )
                    conversion += 10;
               arg[position] = conversion + '0';
          }
          else
          {
               arg[position] = argument[position];
          }

     }
     arg[position] = '\0';
     return arg;
}

char *drunk_speech ( const char *argument, CHAR_DATA * ch )
{
     const char         *arg = argument;
     static char         buf[MAX_INPUT_LENGTH * 2];
     char                buf1[MAX_INPUT_LENGTH * 2];
     sh_int              drunk;
     char               *txt;
     char               *txt1;

     if ( IS_NPC ( ch ) || !ch->pcdata )
          return ( char * ) argument;

     drunk = ch->pcdata->condition[COND_DRUNK];

     if ( drunk <= 0 )
          return ( char * ) argument;

     buf[0] = '\0';
     buf1[0] = '\0';

     if ( !argument )
     {
          bugf ( "Drunk_speech: NULL argument (%s)", ch->name );
          return "";
     }

     /*
      * if ( *arg == '\0' )
      * return (char *) argument;
      */

     txt = buf;
     txt1 = buf1;

     while ( *arg != '\0' )
     {
          if ( toupper ( *arg ) == 'S' )
          {
               if ( number_percent (  ) < ( drunk * 2 ) )	/* add 'h' afteran 's' */
               {
                    *txt++ = *arg;
                    *txt++ = 'h';
               }
               else *txt++ = *arg;
          }
          else if ( toupper ( *arg ) == 'X' )
          {
               if ( number_percent (  ) < ( drunk * 2 / 2 ) )
               {
                    *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
               }
               else *txt++ = *arg;
          }
          else if ( number_percent (  ) < ( drunk * 2 / 5 ) )	/* slurred letters */
          {
               sh_int              slurn = number_range ( 1, 2 );
               sh_int              currslur = 0;

               while ( currslur < slurn )
                    *txt++ = *arg, currslur++;
          }
          else *txt++ = *arg;
          arg++;
     };

     *txt = '\0';

     txt = buf;

     while ( *txt != '\0' )	/* Let's mess with the string's caps */
     {
          if ( number_percent (  ) < ( 2 * drunk / 2.5 ) )
          {
               if ( isupper ( *txt ) )
                    *txt1 = tolower ( *txt );
               else if ( islower ( *txt ) )
                    *txt1 = toupper ( *txt );
               else
                    *txt1 = *txt;
          }
          else
               *txt1 = *txt;
          txt1++, txt++;
     };

     *txt1 = '\0';
     txt1 = buf1;
     txt = buf;

     while ( *txt1 != '\0' )	/* Let's make them stutter */
     {
          if ( *txt1 == ' ' )	/* If there's a space, then there's gotta be a */
          {			/* along there somewhere soon */

               while ( *txt1 == ' ' )	/* Don't stutter on spaces */
                    *txt++ = *txt1++;

               if ( ( number_percent (  ) < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
               {
                    sh_int              offset = number_range ( 0, 2 );
                    sh_int              pos = 0;

                    while ( *txt1 != '\0' && pos < offset )
                         *txt++ = *txt1++, pos++;

                    if ( *txt1 == ' ' )	/* Make sure not to stutter a space after */

                    {		/* the initial offset into the word */
                         *txt++ = *txt1++;
                         continue;
                    }

                    pos = 0;
                    offset = number_range ( 2, 4 );
                    while ( *txt1 != '\0' && pos < offset )
                    {
                         *txt++ = *txt1;
                         pos++;
                         if ( *txt1 == ' ' || pos == offset )	/* Make sure we don't stick */
                         {		/* A hyphen right before a space */
                              txt1--;
                              break;
                         }
                         *txt++ = '-';

                    }
                    if ( *txt1 != '\0' )
                         txt1++;
               }
          }
          else
               *txt++ = *txt1++;
     }

     *txt = '\0';

     return buf;
}

/* RT code to delete yourself */

void do_delet ( CHAR_DATA * ch, char *argument )
{
     send_to_char( "You must type the full command to delete yourself.\n\r", ch );
}

void do_delete ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( ch->pcdata->confirm_delete )
     {
          if ( argument[0] != '\0' )
          {
               send_to_char ( "Delete status removed.\n\r", ch );
               ch->pcdata->confirm_delete = FALSE;
               return;
          }
          else
          {
               send_to_char ( "Deleting!\n\r", ch );
               sound ("taps.wav", ch);

			   /* Zeran - notify_message */
               notify_message ( ch, NOTIFY_DELETE, TO_ALL, NULL );

               real_delete (ch);
               return;
          }
     }

     if ( argument[0] != '\0' )
     {
          send_to_char ( "Just type delete. No argument.\n\r", ch );
          return;
     }

     send_to_char ( "Type delete again to confirm this command.\n\r", ch );
     send_to_char ( "WARNING: this command is irreversible.\n\r", ch );
     send_to_char ( "Typing delete with an argument will undo delete status.\n\r",  ch );
     ch->pcdata->confirm_delete = TRUE;
}

/* So we make sure to remove everything the character is associated with when */
/* The pfile is removed, not leaving anything dangling. */
/* do_fastquit handles the temporary stuff of course. */

void real_delete ( CHAR_DATA *ch )
{
     char                strsave[MAX_INPUT_LENGTH];
     int i;

	 /* Remove char from account */

     for (i = 0 ; i < MAX_CHARS ; i++)
     {
          if (ch->pcdata->account->char_name[i] && !strcmp(ch->name, ch->pcdata->account->char_name[i]) )
          {
			   /* Set character name to NULL */
               ch->pcdata->account->char_name[i] = NULL;
               if ( !ch->pcdata->mortal )			// A demigod is deleting! Decrement the counter
                    --ch->pcdata->account->demigods;
          }
     }

     fwrite_accounts();

	 /* Remove char from any clans */

     if (ch->pcdata->clan)
     {
          ch->pcdata->clan->membercount--; 	/* Decrement the Counter */
          ch->pcdata->clan = NULL;		/* Clear pointer just in case. */
     }

     save_clans ( );

#if defined (COMPRESS_PFILES)
     SNP ( strsave, "%s%s.gz", PLAYER_DIR,
               capitalize ( ch->name ) );
#else
     SNP ( strsave, "%s%s", PLAYER_DIR,
               capitalize ( ch->name ) );
#endif

     do_fastquit ( ch );
     unlink ( strsave );
}

/* Always call fwrite_accounts after calling offline_delete */

void offline_delete ( char *argument )
{
     char		strdel[MAX_INPUT_LENGTH];
     struct account_type  *tmp = NULL;
     int i;
     bool acc_found = FALSE;

	 /* We need to find the associated account first - Pretty intensive*/
	 /* But required since we don't have a pointer. */

     for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
     {
          for (i = 0 ; i < MAX_CHARS ; i++)
          {
               if (tmp->char_name[i] && !strcmp (argument, tmp->char_name[i]) )
               {
                    tmp->char_name[i] = NULL; /* Remove name */
                    acc_found = TRUE;
                    break;
               }
          }
          /* end of for i = 0 */
     }
     /* end of for tmp = account_list */

     if (!acc_found)
     {
          bugf ( "Tried to delete %s offline, account not found.", argument );
          return;
     }

	 /* Remove char from any clans */
	 /* NEED ROSTER FOR THIS!  */
     //    {
     //        clan->membercount--;
     //    }
     //
#if defined (COMPRESS_PFILES)
     SNP ( strdel, "%s%s.gz", PLAYER_DIR, capitalize ( argument ) );
#else
     SNP ( strdel, "%s%s", PLAYER_DIR,  capitalize ( argument ) );
#endif

     unlink ( strdel );
}

void do_beep ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;

     if ( IS_SET ( ch->comm, COMM_NOTELL ) )
     {
          send_to_char ( "Your beep didn't get through.\n\r", ch );
          return;
     }

     if ( IS_SET ( ch->comm, COMM_QUIET ) )
     {
          send_to_char ( "You must turn off quiet mode first.\n\r", ch );
          return;
     }

     if ( argument[0] == '\0' )
     {
          if ( IS_SET ( ch->comm, COMM_BEEP ) )
          {
               REMOVE_BIT ( ch->comm, COMM_BEEP );
               send_to_char ( "You now accept beeps (This includes beep codes).\n\r", ch );
          }
          else
          {
               SET_BIT ( ch->comm, COMM_BEEP );
               send_to_char ( "You refuse to accept beeps or beep codes.\n\r", ch );
          }
          return;
     }

     if ( IS_SET ( ch->comm, COMM_BEEP ) )
     {
          send_to_char ( "You have to turn on the beep channel first.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, argument ) ) == NULL )
     {
          send_to_char ( "Nobody like that.\n\r", ch );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_BEEP ) )
     {
          act_new ( "$N is not receiving beeps.", ch, NULL, victim,	TO_CHAR, POS_DEAD );
          return;
     }

     act_new ( "\a{WYou beep to $N.{x", ch, NULL, victim, TO_CHAR, POS_DEAD );
     act_new ( "\a{W$n beeps you.{x", ch, NULL, victim, TO_VICT, POS_DEAD );

     return;
}

void do_deaf ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     
     if ( IS_SET ( ch->comm, COMM_NOSHOUT ) )
     {
          send_to_char ( "The gods have taken away your ability to shout.\n\r", ch );
          return;
     }

     real_acomm ( ch, COMM_DEAF, "Deaf Mode (Block Shouts)" );
     
}

void do_quiet ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_QUIET, "Quiet Mode" );
}

/* Channels block */

void do_auction ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "auction" );
     return;
}

void do_gossip ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "gossip" );
     return;
}

void do_question ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "question" );
     return;
}

void do_answer ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "answer" );
     return;
}

void do_music ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "music" );
     return;
}

void do_immtalk ( CHAR_DATA * ch, char *argument )
{
     channel_message ( ch, argument, "immtalk" );
     return;
}

void do_imptalk ( CHAR_DATA * ch, char *argument )
{
     if ( argument[0] == '\0' ) // because imps can't turn off imptalk.
          return;
     channel_message ( ch, argument, "imptalk" );
}

void do_say ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA *obj, *obj_next;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Say what?\n\r", ch );
          return;
     }
     if ( CAN_DETECT ( ch, AFF_MUTE ) )
     {
          send_to_char ( "Mute people cannot use 'say'.\n\r", ch );
          return;
     }
     channel_message ( ch, argument, "say" );

     if ( !IS_NPC(ch) )
     {
          CHAR_DATA *mob, *mob_next;
          OBJ_DATA *obj, *obj_next; // progs

          for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
          {
               mob_next = mob->next_in_room;
               if ( IS_NPC(mob) && HAS_TRIGGER_MOB( mob, TRIG_SPEECH ) && mob->position == mob->pIndexData->default_pos )
                    p_act_trigger( argument, mob, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
               for ( obj = mob->carrying; obj; obj = obj_next )
               {
                    obj_next = obj->next_content;
                    if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
                         p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
               }
          }
     }
     for ( obj = ch->in_room->contents; obj; obj = obj_next )
     {
          obj_next = obj->next_content;
          if ( HAS_TRIGGER_OBJ( obj, TRIG_SPEECH ) )
               p_act_trigger( argument, NULL, obj, NULL, ch, NULL, NULL, TRIG_SPEECH );
     }
     if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_SPEECH ) )
          p_act_trigger( argument, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_SPEECH );
     return;
}

void do_shout ( CHAR_DATA * ch, char *argument )
{
     if ( IS_SET ( ch->comm, COMM_NOSHOUT ) )
     {
          send_to_char ( "You can't shout.\n\r", ch );
          return;
     }
     if ( IS_SET ( ch->comm, COMM_DEAF ) )
     {
          send_to_char ( "Deaf people can't shout.\n\r", ch );
          return;
     }
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Shout what?\n\r", ch );
          return;
     }

     channel_message ( ch, argument, "shout" );
     WAIT_STATE ( ch, 12 );
}

void do_tell ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     if ( !IS_NPC (ch ) && IS_SET ( ch->comm, COMM_NOTELL ) )
     {
          send_to_char ( "Your message didn't get through.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( ch ) && IS_SET ( ch->comm, COMM_QUIET ) )
     {
          send_to_char ( "You must turn off quiet mode first.\n\r", ch );
          return;
     }

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "Tell whom what?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL || ( IS_NPC ( victim ) && victim->in_room != ch->in_room ) )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->desc == NULL && !IS_NPC ( victim ) )
     {
          act ( "$N seems to have misplaced $S link...try again later.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_QUIET ) && !IS_IMMORTAL ( ch ) )
     {
          act ( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
          return;
     }

     if ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_AFK ) )
     {
          struct afk_tell_type *tmp;
          char   buf[MIL*2];

          act ( "$N is AFK...your tell may not be seen immediately.", ch, NULL, victim, TO_CHAR );
          SNP ( buf, "%s tells you '%s'\n\r", PERSMASK ( ch, victim ), argument );

          tmp = alloc_mem ( sizeof ( struct afk_tell_type ), "afk_tell_type" );
          tmp->message = str_dup ( buf );
          tmp->next = NULL;
		  /* attach to list */
          if ( victim->pcdata->afk_tell_first == NULL )
          {
               victim->pcdata->afk_tell_first = tmp;
               victim->pcdata->afk_tell_last = tmp;
          }
          else
          {
               victim->pcdata->afk_tell_last->next = tmp;
               victim->pcdata->afk_tell_last = tmp;
          }
     }

     act ( "You tell $N '{W$t{x'", ch, argument, victim, TO_CHAR );
     act_new ( "$n tells you '{W$t{x'", ch, argument, victim, TO_VICT, POS_DEAD );
     victim->reply = ch;

     if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH) )
          p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );

     return;
}

// Need to make reply a wrapper for tell so changes to tell don't need to be mirrored here.
//
void do_reply ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;

     if ( IS_SET ( ch->comm, COMM_NOTELL ) )
     {
          send_to_char ( "Your message didn't get through.\n\r", ch );
          return;
     }

     if ( ( victim = ch->reply ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->desc == NULL && !IS_NPC ( victim ) )
     {
          act ( "$N seems to have misplaced $S link...try again later.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( !IS_IMMORTAL ( ch ) && !IS_AWAKE ( victim ) )
     {
          act ( "$E can't hear you.", ch, 0, victim, TO_CHAR );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_QUIET ) && !IS_IMMORTAL ( ch ) )
     {
          act ( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
          return;
     }

     if ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_AFK ) )
     {
          struct afk_tell_type *tmp;
          char   buf[MIL*2];

          act ( "$N is AFK...your tell may not be seen immediately.", ch, NULL, victim, TO_CHAR );
          SNP ( buf, "%s tells you '%s'\n\r", PERSMASK ( ch, victim ), argument );

          tmp = alloc_mem ( sizeof ( struct afk_tell_type ), "afk_tell_type" );
          tmp->message = str_dup ( buf );
          tmp->next = NULL;
		  /* attach to list */
          if ( victim->pcdata->afk_tell_first == NULL )
          {
               victim->pcdata->afk_tell_first = tmp;
               victim->pcdata->afk_tell_last = tmp;
          }
          else
          {
               victim->pcdata->afk_tell_last->next = tmp;
               victim->pcdata->afk_tell_last = tmp;
          }
     }

     act ( "You tell $N '{W$t{x'", ch, argument, victim, TO_CHAR );
     act_new ( "$n tells you '{W$t{w'", ch, argument, victim, TO_VICT, POS_DEAD );
     victim->reply = ch;

     if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH) )
          p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
     return;
}

void do_yell ( CHAR_DATA * ch, char *argument )
{

     if ( IS_SET ( ch->comm, COMM_NOSHOUT ) )
     {
          send_to_char ( "You can't yell.\n\r", ch );
          return;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Yell what?\n\r", ch );
          return;
     }

     channel_message ( ch, argument, "yell" );
     return;
}

void do_emote ( CHAR_DATA * ch, char *argument )
{
     if ( !IS_NPC ( ch ) && IS_SET ( ch->comm, COMM_NOEMOTE ) )
     {
          send_to_char ( "You can't show your emotions.\n\r", ch );
          return;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Emote what?\n\r", ch );
          return;
     }

     MOBtrigger = FALSE;
     act ( "$n $T", ch, NULL, argument, TO_ROOM );
     act ( "$n $T", ch, NULL, argument, TO_CHAR );
     MOBtrigger = TRUE;
     return;
}

/*
 * Structure for a Quote
 */

struct quote_type
{
     char               *text;
     char               *by;
};

/*
 * The Quotes - insert yours in, and increase MAX_QUOTES in merc.h
 * Someday I'll move this to a savefile.
 */

const struct quote_type quote_table[MAX_QUOTES] =
{
     {"Cogito Ergo Sum", "Descartes"},	/* 1 */
     {"Your lucky color has faded.", "Unknown"},
     {"Don't mess with Dragons, for thou art Crunchy and go well with milk.", "Unknown Source"},
     {"... and furthermore ... I don't like your trousers.", "Unknown"},	/* 4 */
     {"They're only kobolds!", "Dragon Magazine 'Famous Last Words'"},	/* 5 */
     {"I wouldn't want to be around when a mage says 'Oops'", "Unknown"},	/*6 */
     {"Of course I'm a wizard, son.  I've got a tall pointy hat!", "Unknown"},	/* 7 */
     {"Oh, wizardry has really very little to do with magic.", "Ingold"},	/* 8 */
     {"Would you like to be my new experiment?", "Kask the Evil Wizard"},	/* 9 */
     {"A wizard, huh?  I throw my drink at him.", "Unkown"},	/* 10 */
     {"No true wizard ever breaks his word.", "Dragon Magazine 'Famous Last Words'"},	/* 11 */
     {"Just how many 30th-level evil wizards are there in this village?", "Unknown"},	/* 12 */
     {"I take the wizard's wand, and snap it in two.", "Dragon Magazine 'Famous Last Words'"},	/* 13 */
     {"No matter how subtle the sorcerer, a knife in the back will seriously cramp his style.", "Unknown"}	/* 14 */

};

void do_quote ( CHAR_DATA * ch )
{
     int                 quote = 0;

     quote = number_range ( 0, MAX_QUOTES - 1 );

     if ( quote_table[quote].text == NULL )
          bugf ( "DO_QUOTE: Null Quote %d", quote );
     form_to_char ( ch, "\n\r{W%s\n\r{Y - %s{x\n\r",
                    quote_table[quote].text, quote_table[quote].by );
     return;
}

void do_bug ( CHAR_DATA * ch, char *argument )
{
     append_file ( ch, BUG_FILE, argument );
     send_to_char ( "Bug logged.\n\r", ch );
     notify_message ( NULL, WIZNET_BUG, TO_IMM, argument );
     return;
}

void do_typo ( CHAR_DATA * ch, char *argument )
{
     append_file ( ch, TYPO_FILE, argument );
     send_to_char ( "Typo logged.\n\r", ch );
     notify_message ( NULL, WIZNET_BUG, TO_IMM, argument );
     return;
}

void do_rent ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "Circles and Dikus and Rents Oh My!!!\n\r", ch );
     send_to_char ( "But this is Sunder. You don't have to rent to quit here.\n\r", ch );
     send_to_char ( "If you are looking to rent a room, not quit, try LEASE.\n\r", ch );
     return;
}

void do_qui ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to QUIT, you have to spell it out.\n\r", ch );
     return;
}

void do_quit ( CHAR_DATA * ch, char *argument )
{
     DESCRIPTOR_DATA    *d;
     OBJ_DATA *obj;
     OBJ_DATA *obj_next = NULL;

     if ( IS_NPC ( ch ) )
          return;
     
     if ( ch->position == POS_FIGHTING )
     {
          send_to_char ( "No way! You are fighting.\n\r", ch );
          return;
     }

     if ( ch->position < POS_STUNNED )
     {
          send_to_char ( "You're not DEAD yet.\n\r", ch );
          return;
     }

     for ( obj = ch->carrying; obj != NULL; obj = obj_next )
     {
          if (obj->owner && !belongs (ch, obj) )
          {
               send_to_char("Sorry, you may not quit while holding someone else's property.\n\r", ch);
               return;
          }
     }

     if ( ch->pcdata->in_progress )
          free_note ( ch->pcdata->in_progress );

     /* Zeran - notify message */
     notify_message ( ch, NOTIFY_QUITGAME, TO_ALL, NULL );
     act ( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
     log_string ( "%s has quit.", ch->name );
     do_quote ( ch );
     ch->quitting = TRUE;

     /*
      * After extract_char the ch is no longer valid!
      */
     
     save_char_obj( ch );
     d = ch->desc;
     extract_char( ch, TRUE );
     if ( d != NULL )
          close_socket( d );     

     return;
}

/* Fastquit only for internal usage, since it doesn't do some normal sanity checks.
 * Also no output to user.
 */

void do_fastquit ( CHAR_DATA * ch )
{
     DESCRIPTOR_DATA    *d;
     OBJ_DATA *obj;
     OBJ_DATA *obj_next = NULL;

     if ( IS_NPC ( ch ) )
          return;

     if ( ch->position == POS_FIGHTING )
     {
          bugf ("fastquit called while %s was fighting.\n\r", ch->name);
     }

     if ( ch->position < POS_STUNNED )
     {
          bugf ("fastquit called while %s was stunned or worse.\n\r", ch->name);
     }

     /* For fastquit, since likely outcome is deletion, character will drop */
     /* anything belonging to someone else automatically. */
     /* We will do some logging for this incase items are lost when a character is made */
     /* to fastquit for reasons other than deletion */

     for ( obj = ch->carrying; obj != NULL; obj = obj_next )
     {
          if (obj->owner && !belongs (ch, obj) )
          {
               obj_from_char ( obj );
               obj_to_room ( obj, ch->in_room );
               act ( "$n drops $p.", ch, obj, NULL, TO_ROOM );
               log_string ( "%s dropped %s (level %d) belonging to %s while being fastquit.",
                         ch->name, obj->name, obj->level, obj->owner);
          }
     }

     if ( ch->pcdata->in_progress )
          free_note ( ch->pcdata->in_progress );

     log_string ( "%s has been fastquit.", ch->name );

     ch->quitting = TRUE;

     /*
      * After extract_char the ch is no longer valid!
      */
     save_char_obj( ch );
     d = ch->desc;
     extract_char( ch, TRUE );
     if ( d != NULL )
          close_socket( d );     
     
     return;
}

void do_save ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     save_char_obj ( ch );
     send_to_char ( "Saving. Remember your character is automatically saved anyway.\n\r", ch );
     WAIT_STATE ( ch, PULSE_VIOLENCE );
     return;
}

/* Groups code rewritten totally by Lotherius to be more efficient. Maybe. */

void do_follow ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Follow whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master != NULL )
     {
          act ( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
          return;
     }

     if ( victim == ch )
     {
          if ( ch->master == NULL )
          {
               send_to_char ( "You already follow yourself.\n\r", ch );
               return;
          }
          stop_follower ( ch );
          return;
     }

     if ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_NOFOLLOW ) && !IS_HERO ( ch ) )
     {
          act ( "$N doesn't seem to want any followers.\n\r", ch, NULL, victim, TO_CHAR );
          return;
     }

     REMOVE_BIT ( ch->act, PLR_NOFOLLOW );

     if ( ch->master != NULL )
          stop_follower ( ch );

     if ( ch->leader != NULL )
          leave_group ( ch );

     add_follower ( ch, victim );
     return;
}

void add_follower ( CHAR_DATA * ch, CHAR_DATA * master )
{
     if ( ch->master != NULL )
     {
          bugf ( "Add_follower: non-null master. (%s)", ch->name );
          return;
     }

     if ( ch->leader != NULL )
     {
         if ( IS_NPC( ch ) && IS_SET (ch->act, ACT_PET ) )
             leave_group ( ch );
         else
         {
             bugf ( "Add_follower: Trying to follow while still in group.(%s)", ch->name );
             return;
         }
     }

     ch->master = master;

     if ( can_see ( master, ch ) )
          act ( "$n now follows you.", ch, NULL, master, TO_VICT );

     act ( "You now follow $N.", ch, NULL, master, TO_CHAR );

     return;
}

// This func is more complicated than it looks.
// So I decided to document it -- Lotherius
//
void leave_group ( CHAR_DATA * ch )
{
     if ( ch->leader != NULL )   /* Remove Member from Leader's Group - Lotherius */
     {
          CHAR_DATA          *leader;
          int                 counter;
          bool                match = FALSE;

          leader = ch->leader;    // whoever ch->leader is, can be self if not in a group.

          // Look for self in leader's group, and remove self.
          //
          for ( counter = 0; counter < MAX_GMEMBERS; counter++ )
          {
               if ( leader->group[counter] && ( leader->group[counter]->gch == ch ) ) // We found ourselves. Bail.
               {
                    match = TRUE;
                    break;
               }
          }
		  /* Zeran - Uhm...Loth, try checking your match variable first */
          if ( match )
          {
              free_group ( leader->group[counter] );
              leader->group[counter] = NULL; /* We weren't nulling the right thing. This is the right thing. */
          }
          else
          {
              /* If it's NOT AN NPC  ---------------- Or if it IS AN NPC, but NOT A PET - loth */
              if ( !IS_NPC(ch) || ( IS_NPC(ch) && !IS_SET(ch->act, ACT_PET) ) )
                  bugf ( "%s had no group on leave_group (Should always have a group)", ch->name );
          }
          ch->leader = NULL;
     }
     else
     {
          bugf ("We got a null ch->leader on leave_group. (%s)", ch->name );
     }

     return;
}

void stop_follower ( CHAR_DATA * ch )
{
     struct char_group  *tmp;

     if ( ch->master == NULL )
     {
          bugf ( "Stop_follower: %s null master.", ch->name );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) )
     {
          REMOVE_BIT( ch->affected_by, AFF_CHARM );
          affect_strip ( ch, gsn_charm_person );
     }

     if ( can_see ( ch->master, ch ) && ch->in_room != NULL )
     {
          act ( "$n stops following you.", ch, NULL, ch->master, TO_VICT );
          act ( "You stop following $N.", ch, NULL, ch->master, TO_CHAR );
     }
     if ( ch->master->pet == ch )
          ch->master->pet = NULL;

     ch->master = NULL;

    if ( ch->leader != NULL ) /* Anyone following but not grouped will have null ch->leader */
        leave_group ( ch );
    
     /* Put back in own group */

     tmp = newgroup();

     ch->group[MAX_GMEMBERS - 1] = tmp;
     tmp->gch = ch;

     ch->leader = ch;

     return;
}

/* nukes charmed monsters and pets when master leaves */
void nuke_pets ( CHAR_DATA * ch )
{
     CHAR_DATA          *pet;

     if ( ( pet = ch->pet ) != NULL )
     {
          stop_follower ( pet );
          if ( pet->in_room != NULL )
               act ( "$N slowly fades away.", ch, NULL, pet, TO_NOTVICT );
          sound ("taps.wav", ch);
          extract_char ( pet, TRUE );
     }
     ch->pet = NULL;

     return;
}

void die_follower ( CHAR_DATA *ch )
{
    struct char_group            *tmp;
    CHAR_DATA                    *fch;
    int counter;
    int last_free = -1;
    
    if ( ch->master != NULL )
    {
        if ( ch->master->pet == ch )
            ch->master->pet = NULL;
        stop_follower ( ch );
    }
    
    /* Remove Follower from Leader's Group - Lotherius */
    if ( ch->leader != NULL && ch->leader != ch )
    {
        leave_group ( ch );
        
        /* Put them into their own group for now... if they're going to be purged, that'll take care of it for the mob case */
        for ( counter = MAX_GMEMBERS - 1; counter >= 0; counter--)
        {
            if ( ch->group[counter] == NULL )
                last_free = counter;
        }
        if ( last_free == -1 )
        {
            send_to_char ( "Really odd bug. You couldn't join your own group.\n\r", ch );
            bugf ( "Character had no group slots available for self. Bug?\n\r", ch );
            return;
        }
        tmp = newgroup ( );
        ch->group[last_free] = tmp;
        tmp->gch = ch;
    }
    
    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
        if ( fch->master == ch && fch != ch)
            stop_follower ( fch );
        if ( fch->leader == ch && fch != ch)
            fch->leader = fch;
    }
    
    return;
}

void do_order ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_INPUT_LENGTH];
     char                arg[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     CHAR_DATA          *och;
     CHAR_DATA          *och_next;
     bool                found;
     bool                fAll;

     argument = one_argument ( argument, arg );
     one_argument ( argument, arg2 );

     if ( arg[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "Order whom to do what?\n\r", ch );
          return;
     }
     if ( !str_cmp (arg2,"delete") || !str_cmp(arg2,"mob"))
     {
          send_to_char ( "That will NOT be done.\n\r", ch );
          return;
     }
     if ( strlen ( argument ) > 100 )
     {
          send_to_char ("No novels, please.\n\r", ch );
          return;
     }
     if ( IS_AFFECTED ( ch, AFF_CHARM ) )
     {
          send_to_char ( "You feel like taking, not giving, orders.\n\r", ch );
          return;
     }
     if ( !str_cmp ( arg, "all" ) )
     {
          fAll = TRUE;
          victim = NULL;
     }
     else
     {
          fAll = FALSE;
          if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
          {
               send_to_char ( "They aren't here.\n\r", ch );
               return;
          }

          if ( victim == ch )
          {
               send_to_char ( "Aye aye, right away!\n\r", ch );
               return;
          }

          if ( !IS_AFFECTED ( victim, AFF_CHARM ) || victim->master != ch )
          {
               send_to_char ( "Do it yourself!\n\r", ch );
               return;
          }
     }
     found = FALSE;
     for ( och = ch->in_room->people; och != NULL; och = och_next )
     {
          och_next = och->next_in_room;

          if ( IS_AFFECTED ( och, AFF_CHARM )
               && och->master == ch && ( fAll || och == victim ) )
          {
               if ( och->wait == 0 )
               {
                    found = TRUE;
                    SNP ( buf, "$n orders you to '%s'.", argument );
                    act ( buf, ch, NULL, och, TO_VICT );
                    interpret ( och, argument );
               }
               else
               {
                    form_to_char ( ch, "%s is too busy right now.\n\r", 
                                   ( IS_NPC ( och ) ? och->short_descr : och->name ) );
               }

          }
     }

     if ( found )
          send_to_char ( "Ok.\n\r", ch );
     else
          send_to_char ( "You have no followers here.\n\r", ch );
     return;
}

/* Biggest changes here - Lotherius */
void do_group ( CHAR_DATA * ch, char *argument )
{
     struct char_group  *tmp;
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 counter;
     int                 last_free = -1;
     bool                match = FALSE;

     one_argument ( argument, arg );

	 /* List Group members */

     if ( arg[0] == '\0' )
     {
          CHAR_DATA          *leader;

          leader = ( ch->leader != NULL ) ? ch->leader : ch;

          form_to_char ( ch, "{W%s{w's Group:\n\r{Y-------------------------\n\r",
                    leader->name );

          for ( counter = 0; counter < MAX_GMEMBERS; counter++ )
          {
               tmp = leader->group[counter];
               if ( tmp != NULL )
               {
                    CHAR_DATA          *gch;

                    gch = leader->group[counter]->gch;
                   if ( !gch )
                   {
                       bugf ( "Null group member from leader %s, group slot %d", leader->name, counter );
                       break;
                   }                   
                    form_to_char ( ch, "{C[%2d %s] %-16s %4d/%4d hp "
                                   "%4d/%4d mana %4d/%4d mv %5d tnl{x\n\r",
                                   gch->level,
                                   IS_NPC ( gch ) ? "Mob" :
                                   class_table[gch->pcdata->pclass].who_name,
                                   capitalize ( PERSMASK ( gch, ch ) ),
                                   gch->hit, gch->max_hit, gch->mana,
                                   gch->max_mana, gch->move, gch->max_move,
                                   IS_NPC (gch) ? 0 :
                                   ( (gch->level +1) * exp_per_level (gch, gch->pcdata->points) - gch->exp) );
                    match = TRUE;
               }
          }
          if ( !match )
               send_to_char ( "There is nobody else in the group.\n\r", ch );
          return;
     }
     /* If victim name is char, bail out */
     if ( !str_cmp ( arg, ch->name ) )
     {
          send_to_char ( "You can't group yourself!", ch );
          return;
     }
     /* Set target, verify target is in room */
     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }
     /* See if character is following someone else, can't start a group */
     if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
     {
          send_to_char ( "But you are following someone else!\n\r", ch );
          return;
     }

     /* Trying to group someone who hasn't followed you */
     if ( victim->master != ch && ch != victim )
     {
          act ( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
          return;
     }
     /* Can't remove charmed mobs from your group, they're automatic. */
     if ( IS_AFFECTED ( victim, AFF_CHARM ) && is_same_group(victim, ch) )
     {
          send_to_char ( "You can't remove charmed mobs from your group.\n\r", ch );
          return;
     }
     /* If you are charmed, you can't leave the group. */
     if ( IS_AFFECTED ( ch, AFF_CHARM ) )
     {
          act ( "You like your master too much to leave $m!", ch, NULL, victim, TO_VICT );
          return;
     }

    /* If player is already in the group, this will remove player from group */
    if ( is_same_group ( victim, ch ) && ch != victim )
    {
        leave_group ( victim );
        
        act ( "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
        act ( "$n removes you from $s group.", ch, NULL, victim, TO_VICT );
        act ( "You remove $N from your group.", ch, NULL, victim, TO_CHAR );
        
        /* Player needs his own group now */
        for ( counter = MAX_GMEMBERS - 1; counter >= 0; counter--)
        {
            if ( victim->group[counter] == NULL )
                last_free = counter;
        }
        if ( last_free == -1 )
        {
            send_to_char ( "Really odd bug. You couldn't join your own group.\n\r", ch );
            bugf ( "Character had no group slots available for self. Bug?\n\r", ch );
            return;
        }
        
        tmp = newgroup ( );
        
        victim->group[last_free] = tmp;
        tmp->gch = victim;
        
        return;
    }    

	 /* Add member to group. */

     victim->leader = ch;
	 /* Find free member Slot */
     for ( counter = MAX_GMEMBERS - 1; counter >= 0; counter-- )
     {
          if ( ch->group[counter] == NULL )
               last_free = counter;
     }
     if ( last_free == -1 )
     {
          send_to_char ( "Your group is as large as possible.\n\r", ch );
          return;
     }

     tmp = newgroup();

     ch->group[last_free] = tmp;
     tmp->gch = victim;

     act ( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
     act ( "You join $n's group.", ch, NULL, victim, TO_VICT );
     act ( "$N joins your group.", ch, NULL, victim, TO_CHAR );
     return;
}

void do_split ( CHAR_DATA * ch, char *argument, bool tax )
{
     char                buf[MSL];
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *gch;
     int                 members;
     int                 amount;
     int                 share;
     int                 extra;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Split how much?\n\r", ch );
          return;
     }

     amount = atoi ( arg );

     if ( amount < 0 )
     {
          send_to_char ( "Your group wouldn't like that.\n\r", ch );
          return;
     }
     if ( amount == 0 )
     {
          send_to_char ( "You hand out zero coins, but no one notices.\n\r", ch );
          return;
     }
     if ( ch->gold < amount )
     {
          send_to_char ( "You don't have that much gold.\n\r", ch );
          return;
     }
     members = 0;
     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( is_same_group ( gch, ch ) && !IS_AFFECTED ( gch, AFF_CHARM ) )
               members++;
     }
     if ( members < 2 )
     {
          send_to_char ( "Just keep it all.\n\r", ch );
          return;
     }
     share = amount / members;
     extra = amount % members;
     if ( share == 0 )
     {
          send_to_char ( "Don't even bother, cheapskate.\n\r", ch );
          return;
     }
     ch->gold -= amount;
     if ( tax )
          do_pay ( ch, ( share + extra ) );
     else
          ch->gold += ( share + extra );
     form_to_char ( ch, "You split %d gold coins.  Your share is %d gold coins.\n\r", amount, share + extra );
     SNP ( buf, "$n splits %d gold coins.  Your share is %d gold coins.", amount, share );
     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( gch != ch && is_same_group ( gch, ch ) && !IS_AFFECTED ( gch, AFF_CHARM ) )
          {
               act ( buf, ch, NULL, gch, TO_VICT );
               do_pay ( gch, share );
          }
     }

     return;
}

// Wrapper so that the player command of do_split won't cause player to have
// to pay a tax on money he/she already has.
void do_splitc ( CHAR_DATA *ch, char *argument )
{
     do_split ( ch, argument, FALSE );
     return;
}

void do_gtell ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     CHAR_DATA          *gch;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Tell your group what?\n\r", ch );
          return;
     }
     if ( IS_SET ( ch->comm, COMM_NOTELL ) )
     {
          send_to_char ( "Your message didn't get through!\n\r", ch );
          return;
     }
     /*
      * Note use of send_to_char, so gtell works on sleepers.
      * Also, don't use form_to_char here because would have to repeatedly format
      */
     SNP ( buf, "%s tells the group '%s'.\n\r", ch->name, argument );
     for ( gch = char_list; gch != NULL; gch = gch->next )
     {
          if ( is_same_group ( gch, ch ) )
               send_to_char ( buf, gch );
     }

     return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group ( CHAR_DATA * ach, CHAR_DATA * bch )
{
     if ( ach->leader != NULL )
          ach = ach->leader;
     if ( bch->leader != NULL )
          bch = bch->leader;
     return ach == bch;
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 * Modified by Lotherius
 */
void do_colour ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_STRING_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

     argument = one_argument ( argument, arg );

     if ( !*arg )
     {
          if ( !ch->desc->ansi )
          {
               ch->desc->ansi = TRUE;
               SET_BIT ( ch->comm, COMM_COLOUR );
               send_to_char ( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );

          }
          else
         {
             if ( ch->desc->mxp )
             {
                 send_to_char ( "MXP enabled users cannot turn off colour at this time.\n\r", ch );
                 return;                                  
             }
             ch->desc->ansi = FALSE;
             send_to_char ( "Colour is now OFF, <sigh>\n\r", ch );
             REMOVE_BIT ( ch->comm, COMM_COLOUR );
          }
          return;
     }
     else
          send_to_char ( "Sorry, configuration is not available in this version of colour.\n\r", ch );
     return;
}

/* Here's a function I need to find a good use for.
 * I really thought this was cool till I found out how many mud clients can't
 * handle full-screen telnet.
 * -Lotherius
 */

void do_cursor ( CHAR_DATA * ch, char *argument )
{
     char                arg[MIL];

     argument = one_argument ( argument, arg );

     if ( !*arg )
     {
          if ( !IS_SET ( ch->act, PLR_CURSOR ) )
          {
               SET_BIT ( ch->act, PLR_CURSOR );
               send_to_char ( VT_CLS, ch );
               gotoxy ( ch, 0, 0 );
               send_to_char ( "Cursor Control is Now On!\n\r", ch );
               send_to_char ( "The following numbers should run down the screen, moving to the right.", ch );
               gotoxy ( ch, 4, 1 );
               send_to_char ( "1", ch );
               gotoxy ( ch, 5, 3 );
               send_to_char ( "2", ch );
               gotoxy ( ch, 6, 5 );
               send_to_char ( "3", ch );
               gotoxy ( ch, 7, 7 );
               send_to_char ( "4\n\r", ch );
               send_to_char ( "If the numbers 1-4 run together, please turn cursor off, your terminal\n\r"
                              "doesn't support cursor control.", ch );
          }
          else
          {
               send_to_char_bw ( "Cursor control is now off.\n\r", ch );
               REMOVE_BIT ( ch->act, PLR_CURSOR );
          }
          return;
     }
     else
     {
          send_to_char ( "Just type cursor by itself.", ch );
     }
     return;
}

void do_alias ( CHAR_DATA * ch, char *argument )
{
     struct alias_data  *tmp = NULL;
     char                alias_name[MAX_INPUT_LENGTH];
     char               *alias_string;
     char                d_alias[MAX_INPUT_LENGTH];
     int                 counter, number;
     bool                got_one = FALSE;
     int                 last_free = -1;

     if ( IS_NPC ( ch ) )
          return;

     /* if no arguments, just print out current aliases */
     if ( argument == NULL || argument[0] == '\0' )
     {
          for ( counter = 0; counter < MAX_ALIAS; counter++ )
          {
               tmp = ch->pcdata->aliases[counter];
               if ( tmp != NULL )
               {
                    form_to_char ( ch, "Alias %d:  (%s) = (%s)\n\r",
                                   counter,
                                   ch->pcdata->aliases[counter]->name,
                                   ch->pcdata->aliases[counter]->
                                   command_string );
                    got_one = TRUE;
               }
          }
          if ( !got_one )
               send_to_char ( "You have no aliases defined.\n\r", ch );
          return;
     }

     alias_string = one_argument ( argument, alias_name );

     if ( alias_string == NULL || alias_string[0] == '\0' )
     {
          send_to_char ( "Syntax:  alias 'name' 'command string'\n\r", ch );
          return;
     }

     if ( !str_cmp ( alias_name, "delete" ) )
     {
          if ( alias_string[0] == '\0' )
          {
               send_to_char ( "Syntax for alias deletion:  alias delete <number>\n\r", ch );
               return;
          }
          one_argument ( alias_string, d_alias );
          /* delete designated alias if exits */
          for ( counter = 0; counter < MAX_ALIAS; counter++ )
          {
               tmp = ch->pcdata->aliases[counter];
               if ( tmp && !str_cmp ( tmp->name, d_alias ) )
               {
                    got_one = TRUE;
                    break;
               }
          }
          if ( !got_one )
          {
               send_to_char ( "No alias found with that name.\n\r", ch );
               return;
          }
          free_string ( tmp->name );
          free_string ( tmp->command_string );
          ch->pcdata->aliases[counter] = NULL;
          free_mem ( tmp, sizeof ( struct alias_data ), "alias_data" );
          send_to_char ( "Alias deleted.\n\r", ch );
          got_one = FALSE;
          /* check if all deleted, set has_alias to false */
          for ( number = 0; number < MAX_ALIAS; number++ )
          {
               if ( ch->pcdata->aliases[number] != NULL )
               {
                    got_one = TRUE;
                    break;
               }
          }
          if ( !got_one )
               ch->pcdata->has_alias = FALSE;
          return;
     }
     
     /*check for ridiculous size of alias */
     if ( strlen ( alias_string ) > MAX_ALIAS_LENGTH )
     {
          send_to_char ( "Alias too long, limit is 50 characters.\n\r", ch );
          return;
     }
     /* find first open alias in array and check for duplication of name */
     for ( counter = MAX_ALIAS - 1; counter >= 0; counter-- )
     {
          if ( ch->pcdata->aliases[counter] == NULL )
               last_free = counter;
          else if ( !str_cmp ( ch->pcdata->aliases[counter]->name, alias_name ) )
          {
               send_to_char ( "An alias with that name is already defined.\n\r", ch );
               return;
          }
     }
     /* if no free aliases, tell player to delete an alias first */
     if ( last_free == -1 )
     {
          send_to_char ( "All your alias slots are in use, please delete an alias first.\n\r", ch );
          return;
     }
     /* alloc_mem alias and assign its values */
     tmp = alloc_mem ( sizeof ( struct alias_data ), "alias_data" );
     ch->pcdata->aliases[last_free] = tmp;
     tmp->name = str_dup ( alias_name );
     tmp->command_string = str_dup ( alias_string );
     send_to_char ( "Alias set.\n\r", ch );
     ch->pcdata->has_alias = TRUE;
     return;
}

// See how insiduous buffer overruns can be? The next routine could possibly
// have been crashable because final was MIL, and it was strcpy'd to unsafely,
// thus making it potentially MIL+7 long, and boom, crash. Stuff like this is
// why i'm implementing buffer overrun protection even on seemingly innocent
// functions, because sometimes the obvious is missed. The hit to performance
// is nothing compared to safety gained. - Lotherius

void do_unalias ( CHAR_DATA * ch, char *argument )
{
     char                final[MAX_INPUT_LENGTH+8];

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Usage:  unalias <alias_name>\n\r", ch );
          return;
     }
     SLCPY ( final, "delete " );
     SLCAT ( final, argument );
     do_alias ( ch, final );
}

void do_replay ( CHAR_DATA * ch, char *argument )
{
     struct afk_tell_type *tmp;
     BUFFER *outbuf;

     if ( IS_NPC ( ch ) )
          return;

	 /* check if no buffered tells waiting */
     if ( ch->pcdata->afk_tell_first == NULL )
     {
          send_to_char ( "You have no tells waiting.\n\r", ch );
          return;
     }

     outbuf = buffer_new ( 256 );

	 /* send all tells */
     while ( ch->pcdata->afk_tell_first != NULL )
     {
          bprintf ( outbuf, ch->pcdata->afk_tell_first->message );
          bprintf ( outbuf, "\n\r" );
          /* free up buffered tell */
          tmp = ch->pcdata->afk_tell_first;
          ch->pcdata->afk_tell_first = ch->pcdata->afk_tell_first->next;
          free_string ( tmp->message );
          free_mem ( tmp, sizeof ( struct afk_tell_type ), "afk_tell_type" );
     }
     page_to_char ( outbuf->data, ch );
     ch->pcdata->afk_tell_last = NULL;
     buffer_free ( outbuf );
     return;
}

void do_language ( CHAR_DATA * ch, char *argument )
{
     char                lang_name[MIL*2];
     int                 ch_lang_skill;
     int                 skill;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          form_to_char ( ch, "You are currently speaking {Y%s{x tongue.\n\r", ch->speaking );
          return;
     }

     SNP ( lang_name, "%s tongue", argument );

     skill = skill_lookup ( lang_name );
     if ( skill == -1 )
     {
          form_to_char ( ch, "Sorry, '{Y%s{x' is not a language.\n\r", argument );
          return;
     }

     free_string ( ch->speaking );
     ch->speaking = str_dup ( argument );

     form_to_char ( ch, "You switch to speaking in {Y%s{x tongue.\n\r", argument );

     ch_lang_skill = ch->pcdata->learned[skill];

     if ( ch_lang_skill == 100 )
          send_to_char ( "You speak it perfectly.\n\r", ch );
     else if ( ch_lang_skill > 90 )
          send_to_char ( "You speak it extremely well.\n\r", ch );
     else if ( ch_lang_skill > 70 )
          send_to_char ( "You speak it better than average.\n\r", ch );
     else if ( ch_lang_skill > 40 )
          send_to_char ( "You have some grasp of the language.\n\r", ch );
     else if ( ch_lang_skill > 10 )
          send_to_char ( "You know a few words, but not much more.\n\r", ch );
     else
          send_to_char ( "You would be better off grunting and using hand signals.\n\r", ch );

     return;
}

// Translate a csc (Custom String Code)
// takes arguments:
// 
// *ch        = user
// *argument  = string to translate
// *vch       = a possible target char
// *vob       = a possible target object
// *vrm       = a possible target room
// 
// This is a very speed sensitive function... It needs speed improvement before
// it can go into as widespread use as planned.

char *csc_translate ( CHAR_DATA *ch, const char *argument, CHAR_DATA *vch, OBJ_DATA *vob, ROOM_INDEX_DATA *vrm )
{
     static char    buf[MSL*4]; // since this is returned, it is static
     char  	    st[MSL];
     char	    csc_st[MSL];
     register char *csc_str = csc_st;
     register int   count, clen;

     if ( IS_NPC ( ch ) ) // No NPC's for now.
          return "Not on NPC's";

     buf[0] = '\0';

     strcpy ( csc_str, argument );
     clen = strlen ( csc_str );

     for ( count = 0; count <= clen; count++ )
     {
          if ( (csc_str[count] != '#') &&
               (csc_str[count] != '@' ) )
          {
               int temp = strlen ( buf );
               buf[temp] = csc_str[count];
               buf[temp + 1] = '\0';
          }
          else
          {
               switch ( csc_str[count] )
               {
               case '#':	// Do the # codes
                    {
                         switch ( csc_str[count+1] )
                         {
                         case 'a':	// Age in years
                              SNP ( st, "%d", get_age ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'A':	// Age in hours;
                              SNP ( st, "%d", ( ch->played + ( int ) ( current_time - ch->logon ) ) / 3600 );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'b':	// Overall score
                              SNP ( st, "%d", score_calc ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'B':	// Birth Month
                              SNP ( st, "%s", month_name[ch->pcdata->startmonth] );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'c':	// Class
                              SNP ( st, "%s", class_table[ch->pcdata->pclass].name );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'C':	// Birth Year
                              SNP ( st, "%d", ch->pcdata->startyear );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'd':	// Description
                              SNP ( st, "%s", ch->description );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'D':	// Birth Day
                              SNP ( st, "%d",  ch->pcdata->startday + 1 );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'e':	// Total xp
                              SNP ( st, "%d", ch->exp );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'E':	// Xp to level
                              {
                                   int need;
                                   need = ( ( ch->level + 1 ) * exp_per_level ( ch, ch->pcdata->points )- ch->exp );
                                   SNP ( st, "%d", need );
                                   strcat ( buf, st );
                                   count += 1;
                                   break;
                              }
                         case 'f':	// Fight target
                              if ( ch->fighting == NULL )
                              {
                                   if ( ch->fighting == ch )
                                        SNP ( st, "Yourself?" );
                                   else if ( ch->in_room == ch->fighting->in_room )                                   
                                        SNP ( st, "%s.", PERSMASK ( ch, ch->fighting ) );
                                   else
                                        SNP ( st, "someone who left??" );
                                   strcat ( buf, st );
                                   count += 1;
                              }
                              else
                              {
                                   int temp  = strlen ( buf );
                                   buf[temp] = csc_str[count];
                                   buf[temp + 1] = csc_str[count + 1];
                                   buf[temp + 2] = '\0';
                                   count += 1;
                                   break;                                   
                              }
                              break;
                         case 'F':	// Opponent Condition
                              break;                              
                         case 'g':	// Gold on hand
                              SNP ( st, "%ld", ch->gold );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'h':	// current hp
                              SNP ( st, "%d", ch->hit );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'H':	// Max hp
                              SNP ( st, "%d", ch->max_hit );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'i':	// qtime remain
                              SNP ( st, "%d", ch->pcdata->countdown );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'I':	// qtime to next
                              SNP ( st, "%d", ch->pcdata->nextquest );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'j':	// clan name
                              if ( ch->pcdata->clan )
                              {
                                   SNP ( st, "%s", ch->pcdata->clan->clan_name );
                              }
                              else
                                   SNP ( st, "None" );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'J':	// clan rank
                              if ( ch->pcdata->clan )
                              {
                                   SNP ( st, "%s",
                                         ch->sex == 2 ? ch->pcdata->clan->franks[ch->pcdata->clrank] :
                                         ch->pcdata->clan->mranks[ch->pcdata->clrank] );
                                   // ^^^ Gets the properly gendered rank.
                              }
                              else
                                   SNP ( st, "Nobody" );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'k':	// pk wins
                              SNP ( st, "%d", ch->pcdata->pkill_wins );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'K':	// pk losses
                              SNP ( st, "%d", ch->pcdata->pkill_losses );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'l':	// current level
                              SNP ( st, "%d", ch->level );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'L':	// Group leader
                              if ( ch->leader == NULL )                    
                              {
                                   bugf ( "%s has null leader", ch->name );
                                   SNP ( st, "Nobody" );
                              }
                              else
                                   SNP ( st, "%s", ch->leader->name );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'm':	// current mana
                              SNP ( st, "%d", ch->mana );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'M':	// Max mana
                              SNP ( st, "%d", ch->max_mana );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'n':	// your name
                              SNP ( st, "%s", ch->name );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'N':	// name of current area
                               SNP ( st, "%s", ch->in_room->area->name );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'o':	// encumbrance level
                              SNP ( st, "%d", total_encumbrance ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'p':	// practices
                              SNP ( st, "%d", ch->pcdata->practice );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'P':	// position
                              {
                                   switch ( ch->position )
                                   {
                                   case POS_DEAD:
                                        SNP ( st, "dead" );
                                        break;
                                   case POS_MORTAL:
                                        SNP ( st, "mortally wounded" );
                                        break;
                                   case POS_INCAP:
                                        SNP ( st, "incapacitated" );
                                        break;
                                   case POS_STUNNED:
                                        SNP ( st, "stunned" );
                                        break;
                                   case POS_SLEEPING:
                                        SNP ( st, "sleeping" );
                                        break;
                                   case POS_RESTING:
                                        SNP ( st, "resting" );
                                        break;
                                   case POS_SITTING:
                                        SNP ( st, "sitting" );
                                        break;
                                   case POS_STANDING:
                                        SNP ( st, "standing" );
                                        break;
                                   case POS_FIGHTING:
                                        SNP ( st, "fighting" );
                                        break;
                                   default:
                                        SNP ( st, "buggy" );                                                                                                                                
                                   } // End of position switch
                              }
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'q':	// QP current
                              SNP ( st, "%ld", ch->pcdata->questpoints );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'Q':	// QP Earned
                              SNP ( st, "%ld", ch->pcdata->questearned );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'r':	// race
                              SNP ( st, "%s", race_table[ch->race].name );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'R':	// Room name
                              SNP ( st, "%s", ch->in_room->name );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 's':	// sex
                              SNP ( st, "%s", ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female" );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'S':	// # sign
                              strcat ( buf, "#" );
                              count += 1;
                              break;
                         case 't':	// title
                              SNP ( st, "%s", ch->pcdata->title );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'T':	// Last tell from
                              if ( ch->reply == NULL )
                                   SNP ( st, "Nobody" );
                              else
                                   SNP ( st, "%s", ch->reply->name );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'u':	// trust level
                              SNP ( st, "%d", get_trust ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'v':	// current movement
                              SNP ( st, "%d", ch->move );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'V':	// max movement
                              SNP ( st, "%d", ch->max_move );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'w':	// current waitstates
                              SNP ( st, "%d", ch->wait );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'W':	// wimpy level
                              SNP ( st, "%d", ch->wimpy );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'x':      // items carried
                              SNP ( st, "%d", ch->carry_number );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'X':	// max items carried
                              SNP ( st, "%d", can_carry_n ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'y':	// weight carried
                              SNP ( st, "%d", ch->carry_weight );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'Y':	// max weight
                              SNP ( st, "%d", can_carry_w ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'z':	// save throw
                              SNP ( st, "%d", ch->saving_throw );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         default:
                              {
                                   int temp =strlen ( buf );
                                   buf[temp] = csc_str[count];
                                   buf[temp + 1] = csc_str[count + 1];
                                   buf[temp + 2] = '\0';
                                   count += 1;
                              }                              
                         } // End of # switch
                    } // End of case #
                    break;
               case '@':	// Do the @ codes
                    {
                         switch ( csc_str[count+1] )
                         {
                         case 'a':	// alignment
                              SNP ( st, "%d", ch->alignment );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'A':	// the @ symbol
                              strcat ( buf, "@" );
                              count += 1;                              
                              break;
                         case 'b':	// mob rating
                              SNP ( st, "%ld", IS_NPC(ch) ? 0 : ch->pcdata->mob_rating );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'B':	// pk rating
                              SNP ( st, "%d", ch->pcdata->battle_rating );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'c':	// con base
                              SNP ( st, "%d", ch->perm_stat[STAT_CON] );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'C':	// con current
                              SNP ( st, "%d", get_curr_stat ( ch, STAT_STR ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'd':	// dex base
                              SNP ( st, "%d", ch->perm_stat[STAT_DEX] );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'D':	// dex current
                              SNP ( st, "%d", get_curr_stat ( ch, STAT_DEX ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'e':	// encumbrance
                              SNP ( st, "%d", total_encumbrance ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'E':	// email
                              if ( ch->pcdata->email[0] == '\0' )
                                   SNP ( st, "No Email Set" );
                              else
                                   SNP ( st, "%s", ch->pcdata->email );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'i':	// int base
                              SNP ( st, "%d", ch->perm_stat[STAT_INT] );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'I':	// int current
                              SNP ( st, "%d", get_curr_stat ( ch, STAT_INT ) );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'k':	// mob wins
                              SNP ( st, "%ld", ch->pcdata->mob_wins );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'K':	// mob losses
                              SNP ( st, "%ld", ch->pcdata->mob_losses );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'l':	// logon time
                              SNP ( st, "%s", ( char * ) ctime ( &ch->logon ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'L':	// language spoken
                              SNP ( st, "%s", ch->speaking );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'm':	// mortal/demi/imm
                              SNP ( st, "%s", 
                                    IS_IMMORTAL (ch) ? "Immortal" : (ch->pcdata->mortal ? "Mortal" : "Demi-God") );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'p':	// prompt
                              SNP ( st, "%s", ch->pcdata->prompt );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'r':	// hitroll
                              SNP ( st, "%d", GET_HITROLL ( ch ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'R':	// damroll
                              SNP ( st, "%d", GET_DAMROLL ( ch ) );
                              count += 1;
                              break;
                         case 's':	// str base
                              SNP ( st, "%d", ch->perm_stat[STAT_STR] );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 'S':	// str current
                              SNP ( st, "%d", get_curr_stat ( ch, STAT_STR ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         case 't':	// immtitle
                              if ( IS_IMMORTAL ( ch ) )
                              {
                                   SNP ( st, "%s", ch->pcdata->immtitle );
                                   strcat ( buf, st );
                                   count += 1;
                              }
                              break;
                         case 'v':	// Room vnum
                              {
                                   if ( get_trust ( ch ) >= LEVEL_IMMORTAL )
                                   {
                                        SNP ( st, "%d", ch->in_room->vnum );
                                        strcat ( buf, st );
                                        count += 1;
                                        break;
                                   }
                                   else
                                   {
                                        int temp  = strlen ( buf );
                                        buf[temp] = csc_str[count];
                                        buf[temp + 1] = csc_str[count + 1];
                                        buf[temp + 2] = '\0';
                                        count += 1;
                                        break;
                                   }
                              }                              
                         case 'w':	// wis base
                              SNP ( st, "%d", ch->perm_stat[STAT_WIS] );
                              strcat ( buf, st );
                              count += 1;                              
                              break;
                         case 'W':	// wis current
                              SNP ( st, "%d", get_curr_stat ( ch, STAT_WIS ) );
                              strcat ( buf, st );
                              count += 1;
                              break;
                         default:
                              {
                                   int temp =strlen ( buf );
                                   buf[temp] = csc_str[count];
                                   buf[temp + 1] = csc_str[count + 1];
                                   buf[temp + 2] = '\0';
                                   count += 1;
                              }                              
                         } // End of @ switch
                    } // End of case @
                    break;
               default:
                    break;
               } // End of # or @ switch
          } // End of found # or @
     } // End of for loop

     return buf;
}
