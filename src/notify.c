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

/*
 *
 *  notify.c - added by Zeran to support notification and wiznet systems for
 *  various events in Dark Times.  Note, wiznet was hacked in after the
 *  notify system, thus the somewhat odd handling to avoid dup'ing code
 *  like crazy.
 *
 * Hacked by Lotherius... Hey Z.. didn't you consider that maybe we would
 * want to send something other than just one kind of preformatted message
 * to a notify channel?? Hmmm??
 */

#include "everything.h"

DECLARE_DO_FUN ( do_help );

struct event_type
{
     char               *event_name;
     int                 event_flag;
     int                 level;
};

struct event_type   wiznet_table[] =
{

	 /*		{	"event",	WIZNET_FLAG, 	LEVEL }, */
     {"sites",    WIZNET_SITES,    LEVEL_IMMORTAL},
     {"newbie",   WIZNET_NEWBIE,   LEVEL_IMMORTAL},
     {"spam",     WIZNET_SPAM,     LEVEL_IMMORTAL},
     {"death",    WIZNET_DEATH,    LEVEL_IMMORTAL},
     {"reset",    WIZNET_RESET,    LEVEL_IMMORTAL},
     {"mobdeath", WIZNET_MOBDEATH, LEVEL_IMMORTAL},
     {"bug",      WIZNET_BUG,      LEVEL_IMMORTAL},
     {"switch",   WIZNET_SWITCH,   LEVEL_IMMORTAL},
     {"links",    WIZNET_LINK,     LEVEL_IMMORTAL},
     {"load",     WIZNET_LOAD,     LEVEL_ADMIN},
     {"restore",  WIZNET_RESTORE,  LEVEL_ADMIN},
     {"snoop",    WIZNET_SNOOP,    LEVEL_ADMIN},
     {"secure",   WIZNET_SECURE,   LEVEL_ADMIN},
     {"", -1, -1}
};

struct event_type   notify_table[] =
{
     {"Heroes",      NOTIFY_HERO,             1 },
     {"Level",       NOTIFY_LEVEL,            1 },
     {"Death",       NOTIFY_DEATH,            1 },
     {"Delete",      NOTIFY_DELETE,           1 },
     {"Login",       NOTIFY_LOGIN,            1 },
     {"Quitgame",    NOTIFY_QUITGAME,         1 },
     {"Lostlink",    NOTIFY_LOSTLINK,         LEVEL_IMMORTAL },
     {"Reconnect",   NOTIFY_RECONNECT,        1 },
     {"Newnote",     NOTIFY_NEWNOTE,          1 },
     {"Tick",        NOTIFY_TICK,             LEVEL_IMMORTAL},
     {"Weather",     NOTIFY_WEATHER,          1 },
     {"Clanjoin",    NOTIFY_CLANACCEPT,       1 },
     {"Petition",    NOTIFY_CLANPETITION,     1 },
     {"Promotion",   NOTIFY_CLANPROMOTE,      1 },
     {"Demotion",    NOTIFY_CLANDEMOTE,       1 },
     {"Outcast",     NOTIFY_CLANQUIT,         1 },
     {"ClanGeneral", NOTIFY_CLANG,            1 },
     {"Repop",       NOTIFY_REPOP,            LEVEL_IMMORTAL},
     {"", -1 - 1}
};

void do_wiznet ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 count;
     int                 level;
     
     if ( IS_NPC ( ch ) )
          return;

     level = get_trust ( ch );

     if ( argument == NULL || argument[0] == '\0' || !str_cmp ( argument, "status" ) )
     {
          send_to_char ( "{CWiznet events{x\n\r", ch );
          send_to_char ( "------ --------\n\r", ch );

          for ( count = 0; wiznet_table[count].event_name[0] != '\0';
                count++ )
          {
               if ( wiznet_table[count].level <= level )
               {
                    form_to_char ( ch, "{c%-12s{x",
                                   wiznet_table[count].event_name );
                    if ( IS_SET ( ch->pcdata->wiznet, wiznet_table[count].event_flag ) )
                         send_to_char ( "{gON{x\n\r", ch );
                    else
                         send_to_char ( "{rOFF{x\n\r", ch );
               }
          }
          send_to_char ( "\n\r", ch );
          return;
     }
     /* end event status display */
     else			/* check for valid command */
     {
          one_argument ( argument, arg );

          if ( !str_cmp ( arg, "off" ) )
          {
               ch->pcdata->wiznet = 0;
               send_to_char ( "All Wiznet events turned off.\n\r", ch );
               return;
          }
         else if ( !str_cmp ( arg, "all" ) )
         {
             ch->pcdata->wiznet = (WIZNET_SITES+WIZNET_NEWBIE+WIZNET_SPAM+WIZNET_DEATH+WIZNET_MOBDEATH+WIZNET_BUG+WIZNET_SWITCH+WIZNET_LINK );
             if ( get_trust(ch) >= LEVEL_ADMIN )
             {
                 ch->pcdata->wiznet += ( WIZNET_LOAD + WIZNET_RESTORE + WIZNET_SNOOP + WIZNET_SECURE );
                 
             }
             send_to_char ( "All wiznet events turned on.\n\r", ch );
             return;             
         }
         
          else if ( !str_cmp ( arg, "help" ) )
          {
               do_help ( ch, "wiznet" );
               return;
          }
          for ( count = 0; wiznet_table[count].event_name[0] != '\0'; count++ )
          {
               if ( !str_cmp ( arg, wiznet_table[count].event_name ) && level >= wiznet_table[count].level )
               {
                    if ( IS_SET ( ch->pcdata->wiznet, wiznet_table[count].event_flag ) )
                    {
                         ch->pcdata->wiznet -= wiznet_table[count].event_flag;
                         form_to_char ( ch, "Wiznet {c%s{x is now {roff{x.\n\r",
                                        wiznet_table[count].event_name );
                         return;
                    }
                    else
                    {
                         ch->pcdata->wiznet += wiznet_table[count].event_flag;
                         form_to_char ( ch, "Wiznet {c%s{x is now {gon{x.\n\r",
                                        wiznet_table[count].event_name );
                         return;
                    }
               }
          }
          /* end match command loop */
          send_to_char ( "No such wiznet event...\n\r", ch );
     }
     /* end match command section */
}
/* end wiznet function */

void do_notify ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 count;
     int                 level;

     if ( IS_NPC ( ch ) )
          return;
     
     level = get_trust ( ch );

     if ( argument == NULL || argument[0] == '\0' || !str_cmp ( argument, "status" ) )
     {
          send_to_char ( "{CNotify events{x\n\r", ch );
          send_to_char ( "------ --------\n\r", ch );

          for ( count = 0; notify_table[count].event_name[0] != '\0'; count++ )
          {
               if ( notify_table[count].level <= level )
               {
                    form_to_char ( ch, "{c%-12s{x", notify_table[count].event_name );
                    if ( IS_SET ( ch->pcdata->notify, notify_table[count].event_flag ) )
                         send_to_char ( "{gON{x\n\r", ch );
                    else
                         send_to_char ( "{rOFF{x\n\r", ch );
               }
          }
          send_to_char ( "\n\r", ch );
          return;
     }
     /* end event status display */
     else			/* check for valid command */
     {
          one_argument ( argument, arg );

          if ( !str_cmp ( arg, "none" ) )
          {
               ch->pcdata->notify = 0;
               send_to_char ( "All Notify events turned off.\n\r", ch );
               return;
          }
          else if ( !str_cmp ( arg, "help" ) )
          {
               do_help ( ch, "notify" );
               return;
          }
          else if ( !str_cmp ( arg, "all" ) )
          {
               ch->pcdata->notify = NOTIFY_ALL;
               send_to_char ( "All Notify events turned on.\n\r", ch );
               return;
          }

          for ( count = 0; notify_table[count].event_name[0] != '\0'; count++ )
          {
               if ( !str_cmp ( arg, notify_table[count].event_name ) && level >= notify_table[count].level )
               {
                    if ( IS_SET ( ch->pcdata->notify, notify_table[count].event_flag ) )
                    {
                         ch->pcdata->notify -= notify_table[count].event_flag;
                         form_to_char ( ch, "Notify {c%s{x is now {roff{x.\n\r",
                                        notify_table[count].event_name );
                         return;
                    }
                    else
                    {
                         ch->pcdata->notify += notify_table[count].event_flag;
                         form_to_char ( ch, "Notify {c%s{x is now {gon{x.\n\r",
                                        notify_table[count].event_name );
                         return;
                    }
               }
          }
          /* end match command loop */
          send_to_char ( "No such notify event...\n\r", ch );
     }
     /* end match command section */
}
/* end notify function */

/* function to be called throughout code whenever notify or wiznet is needed */
void notify_message ( CHAR_DATA * ch, long type, long to, char *extra_name )
{
     char                buf[MAX_STRING_LENGTH];
     DESCRIPTOR_DATA    *d;
     bool                need_vision = FALSE;
     bool                notify_note = FALSE;
     bool                notify_repop = FALSE;
     bool                check_vict_lvl = FALSE;
     bool                is_wiznet = FALSE;
     bool                is_secure = FALSE;
     long                plr_var_type;

     if (mud.nonotify)
          return;

	 /* Ok, hack for wiznet messaging without duplicating all this code */
     if ( to >= TO_WIZNET )
     {
          char                tmpbuf[MAX_STRING_LENGTH];

          is_wiznet = TRUE;
          switch ( type )
          {
          case WIZNET_SITES:
               SNP ( tmpbuf, "Connect by: [ %s ]\n\r", extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_NEWBIE:
               SNP ( tmpbuf, "New player: [ %s ]\n\r", ch->name );
               break;
          case WIZNET_LINK:
               SNP ( tmpbuf, "Links: [ %s ]\n\r", extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_SPAM:
               SNP ( tmpbuf, "Spam: [ %s ] [ %s ]\n\r", ch->name, extra_name );
               break;
          case WIZNET_DEATH:
               SNP ( tmpbuf, "Death: [ %s ]\n\r", ch->name );
               break;
          case WIZNET_RESET:
               SNP ( tmpbuf, "Repop: [ %s ]\n\r", extra_name );
               break;
          case WIZNET_MOBDEATH:
               SNP ( tmpbuf, "Mob death: [ %s ]\n\r", extra_name );
               break;
          case WIZNET_BUG:
               SNP ( tmpbuf, "Bug: [ %s ]\n\r", extra_name );
               break;
          case WIZNET_SWITCH:
               SNP ( tmpbuf, "Switch: by [ %s ] into [ %s ]\n\r",  ch->name, extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_LOAD:
               SNP ( tmpbuf, "Load: by [ %s ] of [ %s ]\n\r", ch->name, extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_RESTORE:
               SNP ( tmpbuf, "Restore: by [ %s ] of [ %s ]\n\r", ch->name, extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_SNOOP:
               SNP ( tmpbuf, "Snoop: by [ %s ] of [ %s ]\n\r", ch->name, extra_name );
               check_vict_lvl = TRUE;
               break;
          case WIZNET_SECURE:
               SNP ( tmpbuf, "Secure:  [ %s ]\n\r", extra_name );
               is_secure = TRUE;
               break;
          default:
               SNP ( tmpbuf, "Unrecognized wiznet event, please inform coders.\n\r" );
               break;
          }
          /* end switch */
          SNP ( buf, "{y----> {BWIZNET{y <----{x\n\r" );
          SLCAT ( buf, tmpbuf );
     }
     /* end if wiznet */
     else
     {
          is_wiznet = FALSE;
          switch ( type )
          {
          case NOTIFY_LEVEL:
               SNP ( buf, "{BNotify{r->{x %s has gained a level!\n\r", ch->name );
               break;
          case NOTIFY_LOGIN:
               SNP ( buf, "{BNotify{r->{x %s has entered the portal leading to " TXT_MUDNAME ".\n\r", ch->name );
               need_vision = TRUE;
               break;
          case NOTIFY_QUITGAME:
               SNP ( buf, "{BNotify{r->{x %s has found the portal back to reality.\n\r", ch->name );
               need_vision = TRUE;
               break;
          case NOTIFY_DELETE:
               SNP ( buf, "{BNotify{r->{x %s has deleted...\n\r", ch->name );
               break;
          case NOTIFY_DEATH:
               if ( str_cmp ( ch->name, extra_name ) )
                    SNP ( buf, "{BNotify{r->{x %s killed by %s.\n\r", ch->name, extra_name );
               else
                    SNP ( buf, "{BNotify{r->{x %s has died.\n\r", ch->name );
               break;
          case NOTIFY_LOSTLINK:
               SNP ( buf, "{BNotify{r->{x %s has gone link dead.\n\r", ch->name );
               need_vision = TRUE;
               break;
          case NOTIFY_RECONNECT:
               SNP ( buf, "{BNotify{r->{x %s has reconnected.\n\r", ch->name );
               need_vision = TRUE;
               break;
          case NOTIFY_NEWNOTE:
               if ( !ch->pcdata ) /* ?? */
                  break;
               SNP ( buf, "{BNotify{r->{x A new message has been posted to %s.\n\r", ch->pcdata->board->short_name );
               notify_note = TRUE;
               need_vision = TRUE;
               break;
          case NOTIFY_TICK:
               SNP ( buf, "TICK...\n\r" );
               break;
          case NOTIFY_CLANACCEPT:
               SNP ( buf, "{BClan Notify{r->{x %s has been accepted into clan %s.\n\r", ch->name, extra_name );
               break;
          case NOTIFY_CLANPROMOTE:
               SNP ( buf, "{BClan Notify{r->{x %s has been promoted to the clan rank of %s.\n\r", ch->name, extra_name );
               break;
          case NOTIFY_CLANDEMOTE:
               SNP ( buf, "{BClan Notify{R->{x %s has been demoted to %s!!\n\r", ch->name, extra_name );
               break;
          case NOTIFY_CLANQUIT:
               SNP ( buf, "{BClan Notify{r->{x %s has quit clan %s.\n\r", ch->name, extra_name );
               break;
          case NOTIFY_CLANG:
               SNP ( buf, "{BClan Notify{r->{x %s\n\r", extra_name ); // Just pass this on as we got it.
               break;
          case NOTIFY_CLANPETITION:
               SNP ( buf, "{BClan Notify{r->{x %s is petitioning to join your clan.\n\r", extra_name );
               break;
          case NOTIFY_REPOP:
               SNP ( buf, "{CRepop:{x Area %s has reset.\n\r", extra_name );
               notify_repop = TRUE;
               break;
          case NOTIFY_HERO:
               SNP ( buf, "{BNotify{r->{x %s is now a {RH{YE{RR{YO{x! Congratulations!\n\r", ch->name );
          default:
               {

                    bugf ( "Unrecognized NOTIFY code [%ld]", type );
                    break;
               }
          }
          /*end switch */
     }
     /* end else notify message section */

	 /* got message, send to appropriate characters */

     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          CHAR_DATA          *victim;

          victim = d->original ? d->original : d->character;
          if ( d->connected != CON_PLAYING )
               continue;
         if ( !victim )
             continue;
         if ( IS_NPC(victim) )
             continue;         
         
          if ( is_wiznet )
               plr_var_type = victim->pcdata->wiznet;
          else
               plr_var_type = victim->pcdata->notify;

          if ( d->connected == CON_PLAYING && IS_SET ( plr_var_type, type ) && !IS_SET ( victim->comm, COMM_QUIET ) )
          {
               if ( is_secure && ch == victim )
                    continue;
               if ( check_vict_lvl && get_trust ( victim ) < get_trust ( ch ) )
                    continue;
               if ( notify_note && !is_note_to ( victim, ch->pcdata->in_progress ) )
                    continue;
               if ( need_vision && !can_see ( victim, ch ) )
                    continue;
               if ( notify_repop && ( !IS_IMMORTAL ( victim ) || str_cmp ( victim->in_room->area->name, extra_name ) ) )
                    continue;
               if ( to == TO_IMM && !IS_IMMORTAL ( victim ) )
                    continue;
               if ( to == TO_IMM_ADMIN && get_trust ( victim ) < TO_IMM_ADMIN )
                    continue;
               if ( to == TO_IMP && !IS_IMP ( victim ) )
                    continue;
               if ( ( to == TO_CLAN ) && !is_same_clan(victim, ch) )
                    continue;
               send_to_char ( buf, victim );
          }
     } /*end for */
} /*end notify_message */
