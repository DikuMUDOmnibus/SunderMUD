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

/* channels.c
*  Zeran - major cleanup of the channel code resides in this file.
*          Adding a channel can be done by simply adding an entry
*          into channel_table and calling channel_message from
*          the parent function (ie, do_gossip, do_say, do_foo etc)
*/

#include "everything.h"

/* local functions */
int                 channel_lookup ( char *argument );

/* Zeran - channel structure and declaration */
struct channel_type
{
     char               *name;
     char               *pre_str_speaker;
     char               *pre_str_receiver;
     char               *format_str;
     bool                use_language;
     bool                use_drunk;
     bool                check_nochannel;
     bool                check_quiet;
     bool                mob_trigger;
     int                 pos;
     int                 min_level;
     int                 channel_id;
     long		 chan_flag;
};

struct channel_type channel_table[] =
{

/*  { "name", "pre_str_speaker", "pre_str_receiver", "format",
		language, drunk, nochannel, quiet,
		mob_trigger, pos, min_level, channel_id, channel_flag
	},
*/
     {"gossip", "{cYou gossip", "{c$n gossips", "'{C$t{c'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_SLEEPING, 1, CHAN_GOSSIP, COMM_NOGOSSIP
     },

     {"auction", "{yYou auction", "{y$n auctions", "'{Y$t{y'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_SLEEPING, 1, CHAN_AUCTION, COMM_NOAUCTION
     },

     {"music", "{mYou sing", "{m$n sings", "'{M$t{m'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_DEAD, 1, CHAN_MUSIC, COMM_NOMUSIC
     },

     {"question", "{gYou question", "{g$n questions", "'{G$t{g'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_DEAD, 1, CHAN_QUESTION, COMM_NOQUESTION
     },

     {"answer", "{gYou answer", "{g$n answers", "'{G$t{g'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_DEAD, 1, CHAN_ANSWER, COMM_NOQUESTION
     },

     {"shout", "{xYou shout", "{x$n shouts", "'{x$t{x'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_RESTING, 1, CHAN_SHOUT, COMM_DEAF
     },

     {"yell", "{xYou yell", "{x$n yells", "'{x$t{x'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_RESTING, 1, CHAN_YELL, 0
     },

     {"immtalk", "{c[{Y$n{c]:", "{c[{Y$n{c]:", "{c$t{x",
               FALSE, FALSE, FALSE, TRUE,
               FALSE, POS_DEAD, LEVEL_HERO, CHAN_IMMTALK, 0
     },

     {"imptalk", "[{rI{gM{bP{x] [{Y$n{x]:",
               "[{rI{gM{bP{x] [{Y$n{x]:", "{Y$t{x",
               FALSE, FALSE, FALSE, FALSE,
               FALSE, POS_DEAD, MAX_LEVEL, CHAN_IMPTALK, 0
     },

     {"say", "{gYou say", "{g$n says", "'{G$t{g'{x",
               TRUE, TRUE, FALSE, FALSE,
               TRUE, POS_RESTING, 1, CHAN_SAY, 0
     },

     {"clantalk", "{rYou tell the clan", "{r$n tells the clan",
               "'{R$t{r'{x",
               FALSE, FALSE, TRUE, TRUE,
               FALSE, POS_SLEEPING, 1, CHAN_CLAN, COMM_NOCLANTELL
     },

     {"", "", "", "", FALSE, FALSE, FALSE, FALSE, FALSE, 0, 0, 0}

};

void channel_message ( CHAR_DATA * ch, char *argument, char *channel )
{
     DESCRIPTOR_DATA    *d;
     char                buf_to_speaker[MAX_STRING_LENGTH];
     char                buf_to_receiver[MAX_STRING_LENGTH];
     char                lang_name[MAX_STRING_LENGTH];
     char               *pre_str_speaker;
     char               *pre_str_receiver;
     char               *format_str;
     char               *outbuf = NULL;
     char                tmpbuf[MAX_STRING_LENGTH];
     bool                use_language;
     bool                use_drunk_speech;
     bool                do_mob_trigger;
     bool                check_quiet;
     bool                check_nochannel;
     int                 pos;
     int                 lang_skill = 0;
     int                 channel_no = -1;
     int                 channel_id;
     int		 chan_flag;
     int                 min_level;

     tmpbuf[0] = '\0';

     channel_no = channel_lookup ( channel );
     if ( channel_no == -1 )
     {
          bugf ( "Invalid channel [%s] in channel_message", channel );
          return;
     }

     /* Ok, get the values from the channel table that we need */
     pre_str_speaker = channel_table[channel_no].pre_str_speaker;
     pre_str_receiver = channel_table[channel_no].pre_str_receiver;
     format_str = channel_table[channel_no].format_str;
     use_language = channel_table[channel_no].use_language;
     use_drunk_speech = channel_table[channel_no].use_drunk;
     check_quiet = channel_table[channel_no].check_quiet;
     check_nochannel = channel_table[channel_no].check_nochannel;
     do_mob_trigger = channel_table[channel_no].mob_trigger;
     min_level = channel_table[channel_no].min_level;
     pos = channel_table[channel_no].pos;
     channel_id = channel_table[channel_no].channel_id;
     chan_flag = channel_table[channel_no].chan_flag;

     /* Check quiet and nochannel settings */
     if ( check_quiet && IS_SET ( ch->comm, COMM_QUIET ) )
     {
          send_to_char ( "You must turn off quiet mode first.\n\r", ch );
          return;
     }
     if ( check_quiet && IS_SET ( ch->comm, COMM_NOCHANNELS ) )
     {
          send_to_char  ( "The gods have revoked your channel priviliges.\n\r", ch );
          return;
     }
     // Imms like to make ppl noshout, this isn't checked elsewhere.
     if ( channel_id == CHAN_SHOUT && IS_SET ( ch->comm, COMM_NOSHOUT ) )
     {
          send_to_char ( "You find your voice very scratchy, and can't shout. Odd, that.\n\r", ch );
          return;
     }

     // Hey Z why didn't you just move this here instead of also having it in 40 diff places?
     if ( argument[0] == '\0' && chan_flag != 0 )
     {
          if ( IS_SET ( ch->comm, chan_flag ) )
          {
               form_to_char ( ch, "%s is now ON.\n\r",
                              capitalize ( channel_table[channel_no].name ) );
               REMOVE_BIT ( ch->comm, chan_flag);
          }
          else
          {
               form_to_char ( ch, "%s is now OFF.\n\r",
                              capitalize ( channel_table[channel_no].name ) );
               SET_BIT ( ch->comm, chan_flag );
          }
          return;
     }
     if ( !IS_NPC ( ch ) ) // Make sure channel is on for the speaker.
          REMOVE_BIT ( ch->comm, chan_flag );

     /* Build message to speaker */
     if ( use_language )
     {
          if ( !IS_NPC ( ch ) )
               SNP ( lang_name, "%s tongue", ch->speaking );
          else
               SNP ( lang_name, "common tongue" );
          SNP ( buf_to_speaker, "%s in %s %s",
                    pre_str_speaker, lang_name, format_str );
     }
     else
          SNP ( buf_to_speaker, "%s %s",
                    pre_str_speaker, format_str );

     /* Send message to speaker */
     act_new ( buf_to_speaker, ch, argument, NULL, TO_CHAR, POS_DEAD );

     /* Build message to receivers */
     /* Scramble for language if speaker is NOT NPC */
     if ( use_language )
     {
          lang_skill = skill_lookup ( lang_name );
          if ( !IS_NPC ( ch ) )
               argument = scramble ( argument, ch->pcdata->learned[lang_skill] );
          SNP ( buf_to_receiver, "%s in %s %s", pre_str_receiver,
                    lang_name, format_str );
     }
     else
          SNP ( buf_to_receiver, "%s %s",
                    pre_str_receiver, format_str );

     /* Set outbuf to point to argument */
     SLCPY ( tmpbuf, argument );

     /* Ok, send the messages */
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          CHAR_DATA          *victim;

          if ( d->connected != CON_PLAYING || d->connected >= CON_NOTE_TO )
               continue;
          /* You would not believe how hard the bug that led to the above two lines */
          /* was to track down. */

          victim = d->original ? d->original : d->character;

          /* Check for something freaky - Lotherius */
          /* This was for some obscure bug I finally axxed, I think. Leaving it here in case it shows back up. */
          if (!victim)
          {
               send_to_char (":1: We were about to crash! Aborted instead. Report to imms.\n\r", ch);
               bugf ( "Null victim in channel_message found." );
               return;
          }

          /* List of checks to skip current receiver */
          if ( d->character != ch )
          {

               /* Check quiet */
               if ( check_quiet && IS_SET ( victim->comm, COMM_QUIET ) )
                    continue;

               /* Check minimum level */
               if ( get_trust ( victim ) < min_level && victim->level < min_level )
                    continue;

               /* Channel dependent exclusions */
               switch ( channel_id )
               {
               case CHAN_GOSSIP:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOGOSSIP ) )
                              continue;
                         break;
                    }
               case CHAN_MUSIC:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOMUSIC ) )
                              continue;
                         break;
                    }
               case CHAN_AUCTION:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOAUCTION ) )
                              continue;
                         break;
                    }
               case CHAN_QUESTION:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOQUESTION ) )
                              continue;
                         break;
                    }
               case CHAN_ANSWER:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOQUESTION ) )
                              continue;
                         break;
                    }
               case CHAN_IMMTALK:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOWIZ ) )
                              continue;
                         break;
                    }
               case CHAN_IMPTALK:
                    {
                         break;
                    }
               case CHAN_YELL:
                    {
                         if ( victim->in_room->area != ch->in_room->area )
                              continue;
                         break;
                    }
               case CHAN_SHOUT:
                    {
                         if ( IS_SET ( victim->comm, COMM_DEAF ) )
                              continue;
                         break;
                    }
               case CHAN_SAY:
                    {
                         if ( victim->in_room && ( victim->in_room != ch->in_room ) )
                              continue;
                         break;
                    }
               case CHAN_CLAN:
                    {
                         if ( IS_SET ( victim->comm, COMM_NOCLANTELL ) )
                              continue;
                         if ( !is_same_clan(victim, ch) )
                              continue;
                         break;
                    }
               default:
                    {
                         bugf( "Invalid channel_id [%d] in channel_message", channel_id );
                         return;
                    }
               }
               /* end switch */
          }

          /* end if */

          /* Check for something freaky */
          if (!victim)
          {
               send_to_char (":2: We were about to crash! Aborted instead. Report to imms.\n\r", ch);
               continue;
          }

          /* Check language and drunk, then send the message */
          if ( (!IS_NPC (victim) ) && (use_language) )
               outbuf = scramble ( tmpbuf, victim->pcdata->learned[lang_skill] );
          else
               outbuf = tmpbuf;

          if ( ( use_drunk_speech ) && (!IS_NPC (victim) ) )
               outbuf = drunk_speech ( outbuf, ch );

          act_new ( buf_to_receiver, ch, outbuf, d->character, TO_VICT, pos );

     }
     /* end for descriptor loop */

     // Moved this elsewhere... I think... -- Lotherius
     //    if ( do_mob_trigger )
     //	mprog_speech_trigger ( argument, ch );

}
/* end channel_message */

void do_channels ( CHAR_DATA * ch, char *argument )
{
     /* lists all channels and their status */
     send_to_char ("Use the new \"config\" command.\n\r", ch);
     return;

}

int channel_lookup ( char *argument )
{
     int                 count = 0;

     while ( channel_table[count].name[0] != '\0' )
     {
          if ( !str_cmp ( channel_table[count].name, argument ) )
               return count;
          count++;
     }
     return -1;
}
