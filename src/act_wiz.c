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
DECLARE_DO_FUN ( do_rstat );
DECLARE_DO_FUN ( do_mstat );
DECLARE_DO_FUN ( do_ostat );
DECLARE_DO_FUN ( do_skillstat );
DECLARE_DO_FUN ( do_rset );
DECLARE_DO_FUN ( do_mset );
DECLARE_DO_FUN ( do_oset );
DECLARE_DO_FUN ( do_sset );
DECLARE_DO_FUN ( do_cset );
DECLARE_DO_FUN ( do_mfind );
DECLARE_DO_FUN ( do_ofind );
DECLARE_DO_FUN ( do_slookup );
DECLARE_DO_FUN ( do_mload );
DECLARE_DO_FUN ( do_oload );
DECLARE_DO_FUN ( do_force );
DECLARE_DO_FUN ( do_quit );
DECLARE_DO_FUN ( do_save );
DECLARE_DO_FUN ( do_look );
DECLARE_DO_FUN ( do_claninfo );
DECLARE_DO_FUN ( do_setskill );
DECLARE_DO_FUN ( do_sit );

/*
 * Local functions.
 */
ROOM_INDEX_DATA    *find_location args ( ( CHAR_DATA * ch, char *arg ) );
void		    save_classes ( );
void		    real_shutdown ( );

/* equips a character */
void do_outfit ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj;

     if ( IS_NPC ( ch ) )
          return;

     if ( ch->level > 5 )
     {
          send_to_char ( "Find it yourself!\n\r", ch );
          return;
     }
     /* To deter "outfit" spamming */

     if ( ch->gold < 25 )
     {
          send_to_char ( "Sorry, there is a service charge of 25 gp and you can't afford it.\n\r", ch);
          return;
     }

     if ( ( obj = get_eq_char ( ch, WEAR_LIGHT ) ) == NULL )
     {
          obj = create_object ( get_obj_index ( OBJ_VNUM_SCHOOL_BANNER ), 0 );
          obj->cost = 0;
          obj_to_char ( obj, ch );
          equip_char ( ch, obj, WEAR_LIGHT );
     }

     if ( ( obj = get_eq_char ( ch, WEAR_BODY ) ) == NULL )
     {
          obj = create_object ( get_obj_index ( OBJ_VNUM_SCHOOL_VEST ), 0 );
          obj->cost = 0;
          obj_to_char ( obj, ch );
          equip_char ( ch, obj, WEAR_BODY );
     }

     if ( ( obj = get_eq_char ( ch, WEAR_SHIELD ) ) == NULL &&
          ( ch->pcdata->pclass != class_lookup ( "monk" ) ) )
     {
          obj = create_object ( get_obj_index ( OBJ_VNUM_SCHOOL_SHIELD ), 0 );
          obj->cost = 0;
          obj_to_char ( obj, ch );
          equip_char ( ch, obj, WEAR_SHIELD );
     }

     if ( ( obj = get_eq_char ( ch, WEAR_WIELD ) ) == NULL && ( ch->pcdata->pclass != class_lookup ( "monk" ) ) )
     {
          obj = create_object ( get_obj_index ( class_table[ch->pcdata->pclass].weapon ), 0 );
          obj_to_char ( obj, ch );
          equip_char ( ch, obj, WEAR_WIELD );
     }

     ch->gold -= 25;
     send_to_char ( "Outfitting Cost: 25 Gold, charged to you.\n\r", ch);
     send_to_char ( "You have been equipped by Zeran.\n\r", ch );
}

/* RT nochannels command, for those spammers */
void do_nochannels ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Nochannel whom?", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_NOCHANNELS ) )
     {
          REMOVE_BIT ( victim->comm, COMM_NOCHANNELS );
          send_to_char ( "The gods have restored your channel priviliges.\n\r", victim );
          send_to_char ( "NOCHANNELS removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->comm, COMM_NOCHANNELS );
          send_to_char ( "The gods have revoked your channel priviliges.\n\r",  victim );
          send_to_char ( "NOCHANNELS set.\n\r", ch );
     }

     return;
}

void do_bamfin ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          form_to_char ( ch, "Your poofin is %s\n\r", ch->pcdata->bamfin );
          return;
     }
     if ( strstr ( argument, ch->name ) == NULL )
     {
          send_to_char ( "You must include your name.\n\r", ch );
          return;
     }
     free_string ( ch->pcdata->bamfin );
     ch->pcdata->bamfin = str_dup ( argument );
     form_to_char ( ch, "Your poofin is now %s\n\r", ch->pcdata->bamfin );
     return;
}

void do_bamfout ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          form_to_char ( ch, "Your poofout is %s\n\r", ch->pcdata->bamfout );
          return;
     }
     if ( strstr ( argument, ch->name ) == NULL )
     {
          send_to_char ( "You must include your name.\n\r", ch );
          return;
     }
     free_string ( ch->pcdata->bamfout );
     ch->pcdata->bamfout = str_dup ( argument );
     form_to_char ( ch, "Your poofout is now %s\n\r", ch->pcdata->bamfout );
     return;
}

void do_deny ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Deny whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     SET_BIT ( victim->act, PLR_DENY );
     send_to_char ( "You are denied access!\n\r", victim );
     send_to_char ( "OK.\n\r", ch );
     save_char_obj ( victim );
     do_quit ( victim, "" );

     return;
}

void do_disconnect ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     DESCRIPTOR_DATA    *d;
     CHAR_DATA          *victim;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Disconnect whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->desc == NULL )
     {
          act ( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
          return;
     }
     /* Added due to 1 too many "fun-disconnect-fests" between imms. */
     if ( get_trust( victim ) >= get_trust( ch ) )
     {
          send_to_char( "You can't.\n\r", ch );
          form_to_char( victim, "%s just tried to disconnect you.\n\r", ch->name );
          return;
     }
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          if ( d == victim->desc )
          {
               if ( IS_IMMORTAL ( d->character ) )
                    send_to_char ( "You have been disconnected.\n\r", d->character );
               close_socket ( d );
               send_to_char ( "Ok.\n\r", ch );
               return;
          }
     }
     bugf ( "Do_disconnect: desc not found." );
     send_to_char ( "Descriptor not found!\n\r", ch );
     return;
}

void do_delay( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *victim;
     char 	arg[MAX_INPUT_LENGTH];
     int 	delay;

     argument = one_argument( argument, arg );
     if ( !*arg )
     {
          send_to_char( "Syntax:  delay <victim> <# of rounds>\n\r", ch );
          return;
     }
     if ( !( victim = get_char_world( ch, arg ) ) )
     {
          send_to_char( "No such character online.\n\r", ch );
          return;
     }
     if ( IS_NPC( victim ) )
     {
          send_to_char( "Mobiles are unaffected by lag.\n\r", ch );
          return;
     }
     if ( get_trust( victim ) >= get_trust( ch ) )
     {
          send_to_char( "You can't.\n\r", ch );
          form_to_char( victim, "%s just tried to delay you.\n\r", ch->name );
          return;
     }
     argument = one_argument(argument, arg);
     if ( !*arg )
     {
          send_to_char( "For how long do you wish to delay them?\n\r", ch );
          return;
     }
     if ( !str_cmp( arg, "none" ) )
     {
          send_to_char( "All character delay removed.\n\r", ch );
          victim->wait = 0;
          return;
     }
     delay = atoi( arg );

     if ( delay < 1 )
     {
          send_to_char( "Pointless.  Try a positive number.\n\r", ch );
          return;
     }
     if ( delay > 999 )
     {
          send_to_char( "You cruel bastard.  Just kill them.\n\r", ch );
          return;
     }
     WAIT_STATE( victim, delay * PULSE_VIOLENCE );
     form_to_char ( ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay );
     return;
}

void do_pardon ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char ( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg2, "killer" ) )
     {
          if ( IS_SET ( victim->act, PLR_KILLER ) )
          {
               REMOVE_BIT ( victim->act, PLR_KILLER );
               send_to_char ( "Killer flag removed.\n\r", ch );
               send_to_char ( "You are no longer a KILLER.\n\r",   victim );
          }
          return;
     }

     if ( !str_cmp ( arg2, "thief" ) )
     {
          if ( IS_SET ( victim->act, PLR_THIEF ) )
          {
               REMOVE_BIT ( victim->act, PLR_THIEF );
               send_to_char ( "Thief flag removed.\n\r", ch );
               send_to_char ( "You are no longer a THIEF.\n\r", victim );
          }
          return;
     }

     send_to_char ( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
     return;
}

void do_echo ( CHAR_DATA * ch, char *argument )
{
     DESCRIPTOR_DATA    *d;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Global echo what?\n\r", ch );
          return;
     }
     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected == CON_PLAYING )
          {
               if ( get_trust ( d->character ) >= get_trust ( ch ) )
                    send_to_char ( "global> ", d->character );
               send_to_char ( argument, d->character );
               send_to_char ( "\n\r", d->character );
          }
     }
     return;
}

void do_recho ( CHAR_DATA * ch, char *argument )
{
     DESCRIPTOR_DATA    *d;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Local echo what?\n\r", ch );
          return;
     }

     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected == CON_PLAYING && d->character->in_room == ch->in_room )
          {
               if ( get_trust ( d->character ) >= get_trust ( ch ) )
                    send_to_char ( "local> ", d->character );
               send_to_char ( argument, d->character );
               send_to_char ( "\n\r", d->character );
          }
     }

     return;
}

void do_pecho ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     argument = one_argument ( argument, arg );

     if ( argument[0] == '\0' || arg[0] == '\0' )
     {
          send_to_char ( "Personal echo what?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "Target not found.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) && get_trust ( ch ) != MAX_LEVEL )
          send_to_char ( "personal> ", victim );

     send_to_char ( argument, victim );
     send_to_char ( "\n\r", victim );
     send_to_char ( "personal> ", ch );
     send_to_char ( argument, ch );
     send_to_char ( "\n\r", ch );
}

ROOM_INDEX_DATA    *find_location ( CHAR_DATA * ch, char *arg )
{
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;

     if ( is_number ( arg ) )
          return get_room_index ( atoi ( arg ) );

     if ( ( victim = get_char_world ( ch, arg ) ) != NULL )
          return victim->in_room;

     if ( ( obj = get_obj_world ( ch, arg ) ) != NULL )
          return obj->in_room;

     return NULL;
}

void do_transfer ( CHAR_DATA * ch, char *argument )
{
     char               arg1[MAX_INPUT_LENGTH];
     char               arg2[MAX_INPUT_LENGTH];
     char		buf[MSL];
     ROOM_INDEX_DATA   *location;
     DESCRIPTOR_DATA   *d;
     CHAR_DATA         *victim;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Transfer whom (and where)?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg1, "all" ) )
     {
          for ( d = descriptor_list; d != NULL; d = d->next )
          {
               if ( d->connected == CON_PLAYING
                    && d->character != ch
                    && d->character->in_room != NULL
                    && can_see ( ch, d->character ) )
               {
                    SNP ( buf, "%s %s", d->character->name, arg2 );
                    do_transfer ( ch, buf );
               }
          }
          return;
     }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
     if ( arg2[0] == '\0' )
     {
          location = ch->in_room;
     }
     else
     {
          if ( ( location = find_location ( ch, arg2 ) ) == NULL )
          {
               send_to_char ( "No such location.\n\r", ch );
               return;
          }

          if ( room_is_private ( location ) && get_trust ( ch ) < MAX_LEVEL )
          {
               send_to_char ( "That room is private right now.\n\r", ch );
               return;
          }
     }

     if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->in_room == NULL )
     {
          send_to_char ( "They are in limbo.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( victim ) && get_trust ( victim ) > get_trust ( ch ) )	/*fail */
     {
          form_to_char ( ch, "%s is too powerful for you to transfer.\n\r", victim->name );
          return;
     }

     if ( victim->fighting != NULL )
          stop_fighting ( victim, TRUE );
     act ( "$n disappears suddenly!", victim, NULL, NULL, TO_ROOM );
     char_from_room ( victim );
     char_to_room ( victim, location );
     act ( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
     if ( ch != victim )
          act ( "Whoa! $n has transferred you.", ch, NULL, victim, TO_VICT );
     do_look ( victim, "auto" );
     send_to_char ( "Ok.\n\r", ch );
     return;
}

void do_at ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     ROOM_INDEX_DATA    *location;
     ROOM_INDEX_DATA    *original;
     CHAR_DATA          *wch;

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "At where what?\n\r", ch );
          return;
     }

     if ( ( location = find_location ( ch, arg ) ) == NULL )
     {
          send_to_char ( "No such location.\n\r", ch );
          return;
     }

     if ( room_is_private ( location ) && get_trust ( ch ) < MAX_LEVEL )
     {
          send_to_char ( "That room is private right now.\n\r", ch );
          return;
     }

     original = ch->in_room;
     char_from_room ( ch );
     char_to_room ( ch, location );
     interpret ( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
     for ( wch = char_list; wch != NULL; wch = wch->next )
     {
          if ( wch == ch )
          {
               char_from_room ( ch );
               char_to_room ( ch, original );
               break;
          }
     }

     return;
}

void do_goto ( CHAR_DATA * ch, char *argument )
{
     ROOM_INDEX_DATA    *location;
     CHAR_DATA          *rch;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Goto where?\n\r", ch );
          return;
     }

     if ( ( location = find_location ( ch, argument ) ) == NULL )
     {
          send_to_char ( "No such location.\n\r", ch );
          return;
     }

     if ( room_is_private ( location ) && get_trust ( ch ) < MAX_LEVEL )
     {
          send_to_char ( "That room is private right now.\n\r", ch );
          return;
     }

     if ( ch->fighting != NULL )
          stop_fighting ( ch, TRUE );

     for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
     {
          if ( get_trust ( rch ) >= ch->invis_level )
          {
               if ( ch->pcdata != NULL &&
                    ch->pcdata->bamfout[0] != '\0' )
                    act ( "$t", ch, ch->pcdata->bamfout, rch, TO_VICT );
               else
                    act ( "$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT );
          }
     }

     char_from_room ( ch );
     char_to_room ( ch, location );

     for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
     {
          if ( get_trust ( rch ) >= ch->invis_level )
          {
               if ( ch->pcdata != NULL &&
                    ch->pcdata->bamfin[0] != '\0' )
                    act ( "$t", ch, ch->pcdata->bamfin, rch, TO_VICT );
               else
                    act ( "$n appears in a swirling mist.", ch, NULL, rch, TO_VICT );
          }
     }

     do_look ( ch, "auto" );
     return;
}

/* Immortal version of recall, with customizable homeroom. */
void do_home ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 room;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument == NULL || argument[0] == '\0' )
     {
          do_goto ( ch, itos(ch->pcdata->home_room) );
          return;
     }
     one_argument ( argument, arg );
     room = atoi ( arg );

     if ( get_room_index ( room ) == NULL )
     {
          send_to_char ( "That room does not exist.\n\r", ch );
          return;
     }
     ch->pcdata->home_room = room;
     form_to_char ( ch, "Your home room has now been set to room number %d.\n\r", room );
     return;
}

/* RT to replace the 3 stat commands */
/* Lotherius - yeah we want one big unwieldy one instead eh? */

void do_stat ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char               *string;
     OBJ_DATA           *obj;
     ROOM_INDEX_DATA    *location;
     CHAR_DATA          *victim;

     string = one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  stat <name>\n\r", ch );
          send_to_char ( "  stat obj <name>\n\r", ch );
          send_to_char ( "  stat mob <name>\n\r", ch );
          send_to_char ( "  stat room <number>\n\r", ch );
          send_to_char ( "  stat skill <skill>\n\r", ch );
          send_to_char ( "  stat clan <name>\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "skill" ) )
     {
          do_skillstat ( ch, string );
          return;
     }

     if ( !str_cmp ( arg, "room" ) )
     {
          do_rstat ( ch, string );
          return;
     }

     if ( !str_cmp ( arg, "obj" ) )
     {
          do_ostat ( ch, string );
          return;
     }

     if ( !str_cmp ( arg, "char" ) || !str_cmp ( arg, "mob" ) )
     {
          do_mstat ( ch, string );
          return;
     }

     if ( !str_cmp (arg, "clan" ) )
     {
          do_claninfo (ch, string ); 	// We'll not duplicate this.
          return;
     }

    /* do it the old way */

     obj = get_obj_world ( ch, argument );
     if ( obj != NULL )
     {
          do_ostat ( ch, argument );
          return;
     }

     victim = get_char_world ( ch, argument );
     if ( victim != NULL )
     {
          do_mstat ( ch, argument );
          return;
     }

     location = find_location ( ch, argument );
     if ( location != NULL )
     {
          do_rstat ( ch, argument );
          return;
     }

     send_to_char ( "Nothing by that name found anywhere.\n\r",
                    ch );
}

void do_skillstat ( CHAR_DATA * ch, char *argument )
{
     char                targ[MIL];
     int                 sn, snl;
     bool		 ig = FALSE;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  stat spell <name> \n\r", ch );
          return;
     }

     if ( ( sn = skill_lookup ( argument ) ) == -1 )
     {
          send_to_char ( "There is no such spell as that.\n\r", ch );
          return;
     }
     
     if ( skill_table[sn].isgroup == TRUE )
          ig = TRUE;

     form_to_char ( ch, "{WStats for [{Y%s{W] - SN [{G%4d{W]  SLOT [{B%4d{W]\n\r",
                    skill_table[sn].name, sn, skill_table[sn].slot );

     send_to_char ( "{M-----------------------------------------------------------------{x\n\r", ch );
     send_to_char ( "          {GMag   {WAve   {GThi   {WWar   {GMon   {WDef   {GCha\n\r", ch );
     send_to_char ( "          {G---   {W---   {G---   {W---   {G---   {W---   {G---\n\r", ch );
     form_to_char ( ch, "{WLevels   :{G%3s   {W%3s   {G%3s   {W%3s   {G%3s   {W%3s   {G%3s\n\r",
                    ( skill_table[sn].skill_level[0] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[0] ),
                    ( skill_table[sn].skill_level[1] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[1] ),
                    ( skill_table[sn].skill_level[2] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[2] ),
                    ( skill_table[sn].skill_level[3] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[3] ),
                    ( skill_table[sn].skill_level[4] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[4] ),
                    ( skill_table[sn].skill_level[5] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[5] ),
                    ( skill_table[sn].skill_level[6] >= 102 ) ? "   " : itos ( skill_table[sn].skill_level[6] ) );
     if ( !ig )
     {
          switch ( skill_table[sn].target )
          {
          case TAR_IGNORE:
               SNP ( targ, "ignore" );
               break;
          case TAR_CHAR_OFFENSIVE:
               SNP ( targ, "offensive" );
               break;
          case TAR_CHAR_DEFENSIVE:
               SNP ( targ, "defensive" );
               break;
          case TAR_CHAR_SELF:
               SNP ( targ, "self" );
               break;
          case TAR_OBJ_INV:
               SNP ( targ, "object inventory" );
               break;
          default:
               SNP ( targ, "undefined" );
               break;
          }
          form_to_char ( ch, "{WTargets  : [{Y%s{W]\n\r", targ );
          
          switch ( skill_table[sn].minimum_position )
          {
          case POS_DEAD:
               SNP ( targ, "dead" );
               break;
          case POS_MORTAL:
               SNP ( targ, "mortally wounded" );
               break;
          case POS_INCAP:
               SNP ( targ, "incapacitated" );
               break;
          case POS_STUNNED:
               SNP ( targ, "stunned" );
               break;
          case POS_SLEEPING:
               SNP ( targ, "sleeping" );
               break;
          case POS_RESTING:
               SNP ( targ, "resting" );
               break;
          case POS_SITTING:
               SNP ( targ, "sitting" );
               break;
          case POS_FIGHTING:
               SNP ( targ, "fighting" );
               break;
          case POS_STANDING:
               SNP ( targ, "standing" );
               break;
          default:
               SNP ( targ, "undefined" );
               break;
          }
          
          form_to_char ( ch, "Position : [{Y%s{W]\n\r", targ );
          form_to_char ( ch, "Min Mana : [{Y%4d{W]\n\r", skill_table[sn].min_mana );
          form_to_char ( ch, "Lag      : [{Y%4d{W]\n\r", skill_table[sn].beats );
          form_to_char ( ch, "Component: [{Y%s{W]\n\r", skill_table[sn].component );
          form_to_char ( ch, "Wearoff  : [{Y%s{W]\n\r", skill_table[sn].msg_off );
          form_to_char ( ch, "Noun     : [{Y%s{W]\n\r", skill_table[sn].noun_damage );
     }
     else     
          form_to_char ( ch, "{C%s is a Group.{W\n\r", skill_table[sn].name );
     
     
     send_to_char ( "Stat     : [{Y", ch );
     switch ( skill_table[sn].bon_stat )
     {
     case STAT_STR:
          send_to_char ( "STR", ch );
          break;
     case STAT_INT:
          send_to_char ( "INT", ch );
          break;
     case STAT_WIS:
          send_to_char ( "WIS", ch );
          break;
     case STAT_DEX:
          send_to_char ( "DEX", ch );
          break;
     case STAT_CON:
          send_to_char ( "CON", ch );
          break;
     default:
          send_to_char ( "None", ch );
          break;          
     }
     send_to_char ( "{W]\n\r", ch );
     
     send_to_char ( "\n\r{CTree Info{W\n\r", ch );
     
     if ( skill_table[sn].matriarch == TRUE )
          form_to_char ( ch, "%s can be a Primary Group.\n\r", skill_table[sn].name );

     snl = slot_lookup ( skill_table[sn].parent );
     form_to_char ( ch, "Parent Group:      [{Y%s{W]\n\r", skill_table[snl].name );

     snl = slot_lookup ( skill_table[sn].sibling );
     form_to_char ( ch, "Required Sibling:  [{Y%s{W]\n\r", skill_table[snl].name );
     
     snl = slot_lookup ( skill_table[sn].kissing_cousin );
     form_to_char ( ch, "Also Requires:     [{Y%s{W]\n\r", skill_table[snl].name );
     
     snl = slot_lookup ( skill_table[sn].distant_cousin );
     form_to_char ( ch, "Bonus For:         [{Y%s{W]\n\r", skill_table[snl].name );
     
     snl = slot_lookup ( skill_table[sn].bastard_child );
     form_to_char ( ch, "Penalizes:         [{Y%s{W]\n\r", skill_table[snl].name );

     send_to_char ( "{M-----------------------------------------------------------------{x\n\r", ch );
     return;
}
/* end of skillstat */

/* Room Stat */
void do_rstat ( CHAR_DATA * ch, char *argument )
{
     char                buf[MSL];
     char                arg[MAX_INPUT_LENGTH];
     ROOM_INDEX_DATA    *location;
     OBJ_DATA           *obj;
     CHAR_DATA          *rch;
     int                 door;

     one_argument ( argument, arg );

     location = ( arg[0] == '\0' ) ? ch->in_room : find_location ( ch, arg );
     if ( location == NULL )
     {
          send_to_char ( "No such location.\n\r", ch );
          return;
     }

     if ( ch->in_room != location && room_is_private ( location ) && get_trust ( ch ) < MAX_LEVEL )
     {
          send_to_char ( "That room is private right now.\n\r", ch );
          return;
     }

     form_to_char ( ch,  "Name: '%s.'\n\rArea: '%s'.\n\r", location->name, location->area->name );
     form_to_char ( ch, "Vnum: %d.  Sector: %d.  Light: %d.\n\r",
                    location->vnum,
                    location->sector_type, location->light );
     form_to_char ( ch, "Room flags: %d.\n\rDescription:\n\r%s",
                    location->room_flags, location->description );

     if ( location->extra_descr != NULL )
     {
          EXTRA_DESCR_DATA   *ed;

          send_to_char ( "Extra description keywords: '", ch );
          for ( ed = location->extra_descr; ed; ed = ed->next )
          {
               send_to_char ( ed->keyword, ch );
               if ( ed->next != NULL )
                    send_to_char ( " ", ch );
          }
          send_to_char ( "'.\n\r", ch );
     }

     send_to_char ( "Characters:", ch );
     for ( rch = location->people; rch; rch = rch->next_in_room )
     {
          if ( can_see ( ch, rch ) )
          {
               send_to_char ( " ", ch );
               one_argument ( rch->name, buf );
               send_to_char ( buf, ch );
          }
     }

     send_to_char ( ".\n\rObjects:   ", ch );
     for ( obj = location->contents; obj; obj = obj->next_content )
     {
          send_to_char ( " ", ch );
          one_argument ( obj->name, buf );
          send_to_char ( buf, ch );
     }
     send_to_char ( ".\n\r", ch );

     for ( door = 0; door <= 5; door++ )
     {
          EXIT_DATA          *pexit;

          if ( ( pexit = location->exit[door] ) != NULL )
          {
               form_to_char ( ch, "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
                              door,
                              ( pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum ),
                              pexit->key, pexit->exit_info, pexit->keyword,
                              pexit->description[0] != '\0' ? pexit->description : "(none).\n\r" );
          }
     }

     return;
}

/* Object Stat */
void do_ostat ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     AFFECT_DATA        *paf;
     OBJ_DATA           *obj;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Stat what?\n\r", ch );
          return;
     }

     /* Now we default to local objects before checking the world */

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) == NULL )
     {
          if ( ( obj = get_obj_world ( ch, argument ) ) == NULL )
          {
               send_to_char ( "Nothing like that in hell, earth, or heaven.\n\r", ch );
               return;
          }
     }
     form_to_char ( ch, "Name(s): %s\n\r", obj->name );
     form_to_char ( ch, "Vnum: %d  Format: %s  Type: %s  Resets: %d  Size: %s\n\r",
                    obj->pIndexData->vnum,
                    obj->pIndexData->new_format ? "new" : "old",
                    item_type_name ( obj ), obj->pIndexData->reset_num,
                    ( obj->size >= 0 && obj->size <= 5 ) ? size_table[obj->size] : "unknown" );
     form_to_char ( ch, "Short description: %s\n\rLong description: %s\n\r",
                    obj->short_descr, obj->description );
     form_to_char ( ch, "Material: %s\n\r", material_name ( obj->material ) );
     form_to_char ( ch, "Wear bits: %s\n\rExtra bits: %s\n\r",
                    wear_bit_name ( obj->wear_flags ),
                    extra_bit_name ( obj->extra_flags ) );
     form_to_char ( ch, "Number: %d/%d  Weight: %d/%d\n\r",
                    1, get_obj_number ( obj ),
                    obj->weight, get_obj_weight ( obj ) );
     form_to_char ( ch, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
                    obj->level, obj->cost, obj->condition, obj->timer );
     form_to_char ( ch, "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
                    obj->in_room == NULL ? 0 : obj->in_room->vnum,
                    obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
                    obj->carried_by == NULL ? "(none)" : can_see ( ch, obj->carried_by )
                    ? obj->carried_by->name : "someone", obj->wear_loc );
     form_to_char ( ch, "Orig Values: %d %d %d %d %d\n\r",
                    obj->valueorig[0], obj->valueorig[1],
                    obj->valueorig[2], obj->valueorig[3],
                    obj->valueorig[4] );
     form_to_char ( ch, "Values:      %d %d %d %d %d\n\r",
                    obj->value[0], obj->value[1], obj->value[2],
                    obj->value[3], obj->value[4] );

     /* now give out vital statistics as per identify */

     switch ( obj->item_type )
     {
     case ITEM_SCROLL:
     case ITEM_POTION:
     case ITEM_PILL:
          form_to_char ( ch, "Level %d spells of:", obj->value[0] );

          if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
          {
               send_to_char ( " '", ch );
               send_to_char ( skill_table[obj->value[1]].name, ch );
               send_to_char ( "'", ch );
          }

          if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
          {
               send_to_char ( " '", ch );
               send_to_char ( skill_table[obj->value[2]].name, ch );
               send_to_char ( "'", ch );
          }

          if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
               send_to_char ( " '", ch );
               send_to_char ( skill_table[obj->value[3]].name, ch );
               send_to_char ( "'", ch );
          }

          send_to_char ( ".\n\r", ch );
          break;

     case ITEM_WAND:
     case ITEM_STAFF:
          form_to_char ( ch, "Has %d(%d) charges of level %d",
                         obj->value[1], obj->value[2], obj->value[0] );

          if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
               send_to_char ( " '", ch );
               send_to_char ( skill_table[obj->value[3]].name, ch );
               send_to_char ( "'", ch );
          }

          send_to_char ( ".\n\r", ch );
          break;

     case ITEM_WEAPON:
          send_to_char ( "Weapon type is ", ch );
          switch ( obj->value[0] )
          {
          case ( WEAPON_EXOTIC ):
               send_to_char ( "exotic\n\r", ch );
               break;
          case ( WEAPON_SWORD ):
               send_to_char ( "sword\n\r", ch );
               break;
          case ( WEAPON_DAGGER ):
               send_to_char ( "dagger\n\r", ch );
               break;
          case ( WEAPON_SPEAR ):
               send_to_char ( "spear/staff\n\r", ch );
               break;
          case ( WEAPON_MACE ):
               send_to_char ( "mace/club\n\r", ch );
               break;
          case ( WEAPON_AXE ):
               send_to_char ( "axe\n\r", ch );
               break;
          case ( WEAPON_FLAIL ):
               send_to_char ( "flail\n\r", ch );
               break;
          case ( WEAPON_WHIP ):
               send_to_char ( "whip\n\r", ch );
               break;
          case ( WEAPON_POLEARM ):
               send_to_char ( "polearm\n\r", ch );
               break;
          default:
               send_to_char ( "unknown\n\r", ch );
               break;
          }
          if ( obj->pIndexData->new_format )
               form_to_char ( ch, "Damage is %dd%d (average %d)\n\r",
                              obj->value[1], obj->value[2],
                              ( 1 + obj->value[2] ) * obj->value[1] / 2 );
          else
               form_to_char ( ch, "Damage is %d to %d (average %d)\n\r",
                              obj->value[1], obj->value[2],
                              ( obj->value[1] + obj->value[2] ) / 2 );

          if ( obj->value[4] )	/* weapon flags */
               form_to_char ( ch, "Weapons flags: %s\n\r", weapon_bit_name ( obj->value[4] ) );
          break;

     case ITEM_ARMOR:
          form_to_char ( ch, "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                         obj->value[0], obj->value[1], obj->value[2],
                         obj->value[3] );
          break;
     }

     if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
     {
          EXTRA_DESCR_DATA   *ed;

          send_to_char ( "Extra description keywords: '", ch );

          for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
          {
               send_to_char ( ed->keyword, ch );
               if ( ed->next != NULL )
                    send_to_char ( " ", ch );
          }

          for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
          {
               send_to_char ( ed->keyword, ch );
               if ( ed->next != NULL )
                    send_to_char ( " ", ch );
          }

          send_to_char ( "'\n\r", ch );
     }

     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          if ( paf->bitvector )
               form_to_char ( ch, "Affects %s by %d, level %d.\n\r",
                              affect_loc_name ( paf->location ),
                              paf->modifier, paf->level );
          else
               form_to_char ( ch, "Spell affect with bits [%s] for %d hours.\n\r",
                              affect_bit_name ( paf->bitvector ),
                              paf->duration );
     }
     if ( !obj->enchanted )
          for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->bitvector )
                    form_to_char ( ch, "Affects %s by %d, level %d.\n\r",
                                   affect_loc_name ( paf->location ),
                                   paf->modifier, paf->level );
               else
                    form_to_char ( ch, "Spell affect with bits [%s] for %d hours.\n\r",
                                   affect_bit_name ( paf->bitvector ),
                                   paf->duration );
          }
     return;
}

/* Mob/Char stat */
void do_mstat ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     AFFECT_DATA        *paf;
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Stat whom?\n\r", ch );
          return;
     }

     // Check in the room first.
     if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
     {
          if ( ( victim = get_char_world( ch, argument ) )      == NULL )
          {
               send_to_char ( "They aren't here.\n\r", ch );
               return;
          }
     }
     form_to_char ( ch, "Name: %s.\n\r", victim->name );
     form_to_char ( ch, "Vnum: %d  Format: %s  Race: %s  Sex: %s  Room: %d\n\r",
                    IS_NPC ( victim ) ? victim->pIndexData->vnum : 0,
                    IS_NPC ( victim ) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
                    race_table[victim->race].name,
                    victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral",
                    victim->in_room == NULL ? 0 : victim->in_room->vnum );
     form_to_char ( ch, "Race number: %d\n\r", victim->race );
     if ( IS_NPC ( victim ) )
          form_to_char ( ch, "Count: %d  Killed: %d\n\r",
                         victim->pIndexData->count,
                         victim->pIndexData->killed );
     form_to_char ( ch, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
                    victim->perm_stat[STAT_STR], get_curr_stat ( victim, STAT_STR ),
                    victim->perm_stat[STAT_INT], get_curr_stat ( victim, STAT_INT ),
                    victim->perm_stat[STAT_WIS], get_curr_stat ( victim, STAT_WIS ),
                    victim->perm_stat[STAT_DEX], get_curr_stat ( victim, STAT_DEX ),
                    victim->perm_stat[STAT_CON], get_curr_stat ( victim, STAT_CON ) );
     form_to_char ( ch, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
                    victim->hit, victim->max_hit, victim->mana,
                    victim->max_mana, victim->move, victim->max_move,
                    IS_NPC ( victim ) ? 0 : victim->pcdata->practice );
     form_to_char ( ch, "Lv: %d  Class: %s  Align: %d  Gold: %ld  Exp: %d\n\r",
                    victim->level,
                    IS_NPC ( victim ) ? "mobile" : class_table[victim->pcdata->pclass].name,
                    victim->alignment, victim->gold, victim->exp );
     // ac used to be here... perhaps we should put it back :)
     form_to_char ( ch, "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\n\r",
                    GET_HITROLL ( victim ), GET_DAMROLL ( victim ),
                    victim->saving_throw, victim->position,
                    victim->wimpy );
     if ( IS_NPC ( victim ) && victim->pIndexData->new_format )
          form_to_char ( ch, "Damage: %dd%d  Message:  %s\n\r",
                         victim->damage[DICE_NUMBER],
                         victim->damage[DICE_TYPE],
                         attack_table[victim->dam_type].name );
     form_to_char ( ch, "Fighting: %s\n\r",
                    victim->fighting ? victim->fighting->name : "(none)" );
     if ( !IS_NPC ( victim ) )
          form_to_char ( ch, "Thirst: %d  Full: %d  Drunk: %d\n\r",
                         victim->pcdata->condition[COND_THIRST],
                         victim->pcdata->condition[COND_FULL],
                         victim->pcdata->condition[COND_DRUNK] );
     form_to_char ( ch, "Carry number: %d  Carry weight: %d\n\r",
                    victim->carry_number, victim->carry_weight );
     if ( !IS_NPC ( victim ) )
          form_to_char ( ch, "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
                         get_age ( victim ),
                         ( int ) ( victim->played + current_time - victim->logon ) / 3600,
                         victim->pcdata->last_level, victim->timer );
     form_to_char ( ch, "Act: %s\n\r", act_bit_name ( victim->act ) );
     if ( !IS_NPC ( victim ) && victim->comm )
          form_to_char ( ch, "Comm: %s\n\r", comm_bit_name ( victim->comm ) );
     if ( IS_NPC ( victim ) && victim->off_flags )
          form_to_char ( ch, "Offense: %s\n\r",
                         off_bit_name ( victim->off_flags ) );
     if ( victim->imm_flags )
          form_to_char ( ch, "Immune: %s\n\r",
                         imm_bit_name ( victim->imm_flags ) );
     if ( victim->res_flags )
          form_to_char ( ch, "Resist: %s\n\r",
                         imm_bit_name ( victim->res_flags ) );
     if ( victim->vuln_flags )
          form_to_char ( ch, "Vulnerable: %s\n\r",
                         imm_bit_name ( victim->vuln_flags ) );
     form_to_char ( ch, "Form: %s\n\rParts: %s\n\r",
                    form_bit_name ( victim->form ),
                    part_bit_name ( victim->parts ) );
     if ( victim->affected_by )
          form_to_char ( ch, "Affected by %s\n\r",
                         affect_bit_name ( victim->affected_by ) );
     if ( victim->detections )
          form_to_char ( ch, "Detections %s\n\r",
                         detect_bit_name ( victim->detections ) );
     if ( victim->protections )
          form_to_char ( ch, "Protections %s\n\r",
                         protect_bit_name ( victim->protections ) );
     form_to_char ( ch, "Master: %s  Leader: %s  Pet: %s\n\r",
                    victim->master ? victim->master->name : "(none)",
                    victim->leader ? victim->leader->name : "(none)",
                    victim->pet ? victim->pet->name : "(none)" );
     if ( !IS_NPC ( victim ) )
          form_to_char ( ch, "Security: %d.\n\r", victim->pcdata->security );	/* OLC */
     form_to_char ( ch, "Short description: %s\n\rLong  description: %s",
                    victim->short_descr,
                    victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
     if ( IS_NPC ( victim ) && victim->spec_fun != 0 )
          send_to_char ( "Mobile has special procedure.\n\r", ch );

     if ( IS_NPC ( victim ) && victim->pIndexData->mprogs )
          send_to_char ( "Mobile has programs.\n\r", ch );

     for ( paf = victim->affected; paf != NULL; paf = paf->next )
          form_to_char ( ch, "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
                         skill_table[( int ) paf->type].name,
                         affect_loc_name ( paf->location ),
                         paf->modifier,
                         paf->duration,
                         affect_bit_name ( paf->bitvector ),
                         paf->level );
     return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char               *string;

     string = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  vnum obj <name>\n\r", ch );
          send_to_char ( "  vnum mob <name>\n\r", ch );
          send_to_char ( "  vnum skill <skill or spell>\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "obj" ) )
     {
          do_ofind ( ch, string );
          return;
     }

     if ( !str_cmp ( arg, "mob" ) || !str_cmp ( arg, "char" ) )
     {
          do_mfind ( ch, string );
          return;
     }

     if ( !str_cmp ( arg, "skill" ) || !str_cmp ( arg, "spell" ) )
     {
          do_slookup ( ch, string );
          return;
     }
     /* do both mob and obj */
     do_mfind ( ch, argument );
     do_ofind ( ch, argument );
}

/* The all parameter has disappeared from these functions.
 * If you wanna see everything, look at the darn areafiles.
 */
void do_mfind ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     MOB_INDEX_DATA     *pMobIndex;
     int                 hash, nMatch;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Find whom?\n\r", ch );
          return;
     }

     nMatch = 0;

     /*
      * Yeah, so iterating over all vnum's takes 10,000 loops.
      * Get_mob_index is fast, and I don't feel like threading another link.
      * Do you?
      * -- Furey
      */
     /*
      * How about we do it this way? - Lotherius
      */
     for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     {
          for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
               if ( is_name( arg, pMobIndex->player_name ) )
               {
                    nMatch++;
                    form_to_char ( ch, "[%5d] %s\n\r", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
               }
     }

     if ( nMatch )
          form_to_char ( ch, "Number of matches: %d\n", nMatch );
     else
          send_to_char ( "No mobs like that in hell, earth, or heaven.\n\r", ch );
     return;
}

void do_ofind ( CHAR_DATA * ch, char *argument )
{
     char arg[MAX_INPUT_LENGTH];
     OBJ_INDEX_DATA *pObjIndex;
     int hash, nMatch;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Find what?\n\r", ch );
          return;
     }

     nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
     /*
      * Let's do it this way - Lotherius
      */

     for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     {
          for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
          {
               if ( is_name( arg, pObjIndex->name ) )
               {
                    nMatch++;
                    form_to_char ( ch, "[%5d] %s\n\r", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
               }
          }
     }

     if ( nMatch )
          form_to_char ( ch, "Number of matches: %d\n", nMatch );
     else
          send_to_char ( "No objects like that in hell, earth, or heaven.\n\r", ch );
     return;
}

void do_shoplist ( CHAR_DATA * ch, char *argument )
{
     SHOP_DATA  *shop;
     BUFFER     *buffer;
     int         scount = 0;
     bool        found = FALSE;
     
     buffer = buffer_new(1000);
     
     bprintf ( buffer, "Num  : [VNUM ] : Buy Sell OHr CHr\n\r" );
     
     for ( shop = shop_first ; shop ; shop=shop->next )
     {
          scount++;
          found = TRUE;
          
          bprintf ( buffer, "%4d : [%5d] : %3d %3d %3d %3d\n\r",
                    scount, shop->keeper, shop->profit_buy, shop->profit_sell, shop->open_hour, shop->close_hour );
     }
     
     if ( !found )
          send_to_char ( "No shops found.\n\r", ch );
     else
          page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
}

void do_mwhere ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buffer;
     CHAR_DATA          *victim;
     bool                found;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Mwhere whom?\n\r", ch );
          return;
     }

     found = FALSE;

     buffer = buffer_new(1000);

     /* One hopes that the bug that led to this never shows up again ) */
     /*
     if (!str_cmp(argument,"nowhere"))
     {
          int count=0;
          for ( victim = char_list; victim != NULL; victim = victim->next )
          {
               if (victim->in_room==NULL)
               {
                    found = TRUE;
                    count++;
                    bprintf( buffer, "%3d) [%5d] %-28s %lx\n\r", count,
                             IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                             IS_NPC(victim) ? victim->short_descr : victim->name,
                             (unsigned long)victim); // wtf?
               }
          }
          if (found)
               page_to_char(buffer->data,ch);
          else
               send_to_char("No mobs without rooms found.\n\r",ch);
          buffer_free(buffer);
          return;
     }*/

     for ( victim = char_list; victim != NULL; victim = victim->next )
     {
          if ( IS_NPC ( victim ) && victim->in_room != NULL && is_name ( argument, victim->name ) )
          {
               found = TRUE;
               bprintf ( buffer, "[%5d] %-28s [%5d] %s\n\r",
                         victim->pIndexData->vnum,
                         victim->short_descr,
                         victim->in_room->vnum,
                         victim->in_room->name );
          }
     }

     if ( !found )
          act ( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
     else
          page_to_char ( buffer->data, ch );

     buffer_free(buffer);

     return;
}

void do_owhere ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buffer;
     OBJ_DATA           *search_obj;
     bool                found;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Owhere what?\n\r", ch );
          return;
     }

     buffer = buffer_new(1000);

     bprintf ( buffer, "{c[ vnum]  name                   location{x\n\r" );
     bprintf ( buffer, "----------------------------------------------------------------------\n\r" );

     found = FALSE;
     for ( search_obj = object_list; search_obj != NULL; search_obj = search_obj->next )
     {
          if ( is_name_abbv ( argument, search_obj->name ) )
          {
               if ( search_obj->in_room != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s  in room    [%5d] %s\n\r",
                              search_obj->pIndexData->vnum,
                              search_obj->short_descr,
                              search_obj->in_room->vnum,
                              search_obj->in_room->name );
                    found = TRUE;
               }
               else if ( search_obj->in_obj != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s  inside     [%5d] %-20s\n\r",
                              search_obj->pIndexData->vnum,
                              search_obj->short_descr,
                              search_obj->in_obj->pIndexData->vnum,
                              search_obj->in_obj->short_descr );
                    found = TRUE;
               }
               else if ( search_obj->carried_by != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s  carried by         %-20s\n\r",
                              search_obj->pIndexData->vnum,
                              search_obj->short_descr,
                              PERS ( search_obj->carried_by, ch ) );
                    found = TRUE;
               }
               else
                    continue;
          }
     }

     if ( !found )
          act ( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
     else
          page_to_char ( buffer->data, ch );

     buffer_free(buffer);

     return;
}

void do_pwhere ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buffer;
     DESCRIPTOR_DATA    *d;
     CHAR_DATA          *victim;
     bool                fall;
     bool                found = FALSE;

     if ( argument == NULL || argument[0] == '\0' )
          fall = TRUE;
     else
          fall = FALSE;

     buffer = buffer_new(1000);
     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected != CON_PLAYING )
               continue;
          victim = d->original ? d->original : d->character;
          if ( !fall )
          {
               if ( !str_cmp ( victim->name, argument ) &&
                    can_see ( ch, victim ) )
               {
                    bprintf ( buffer,
                              "[ {g%s{x ] is in room [ {r%d{x ] [ {c%s{x ]\n\r",
                              victim->name, victim->in_room->vnum,
                              victim->in_room->name );
                    found = TRUE;
                    break;
               }
          }
          else if ( can_see ( ch, victim ) )
          {
               bprintf ( buffer,
                         "[ {g%s{x ] is in room [ {r%d{x ] [ {c%s{x ]\n\r",
                         victim->name, victim->in_room->vnum,
                         victim->in_room->name );
               found = TRUE;
          }
     }

     if ( !found )
          send_to_char ( "No matching characters found.\n\r", ch );
     else
          page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return;
}
/* end pwhere */

void do_reboo ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to REBOOT, spell it out.\n\r", ch );
     return;
}

void do_copyove ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to COPYOVER, spell it out.\n\r", ch );
     return;
}

void real_shutdown ( void )
{
     CHAR_DATA 	*vch;
     extern bool	merc_down;
     DESCRIPTOR_DATA *d, *d_next = NULL;

     save_leases (  );		/* save leases */
     save_clans ( );
     fwrite_accounts ( );	/* Save Accounts Once More. */
     save_races ( );		/* Until I put it somewhere else */

     merc_down = TRUE;
     mud.nonotify = TRUE;

     for ( d = descriptor_list; d != NULL; d = d_next )
     {
          vch = d->original ? d->original : d->character;
          if (vch != NULL)
               save_char_obj(vch);
          close_socket ( d );
     }

     return;
}

void do_reboot ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];

     buf[0] = '\0';

     SNP ( buf, "{Y*** {WReboot by %s {Y***{w\r\nBe back real quick like!\n\r", ch->name );
     do_echo ( ch, buf );
     append_file ( ch, "reboot.txt", buf );

     real_shutdown ( );

     return;
}

void do_shutdow ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to SHUTDOWN, spell it out.\n\r", ch );
     return;
}

void do_shutdown ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];

     buf[0] = '\0';

     SNP ( buf, "{Y*** {RShutdown by %s {Y***{w\r\n", ch->name );
     do_echo ( ch, buf );
     append_file ( ch, SHUTDOWN_FILE, buf );

     real_shutdown ( );

     return;
}

void do_snoop ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     DESCRIPTOR_DATA    *d;
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Snoop whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->desc == NULL )
     {
          send_to_char ( "No descriptor to snoop.\n\r", ch );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "Cancelling all snoops.\n\r", ch );
          for ( d = descriptor_list; d != NULL; d = d->next )
          {
               if ( d->snoop_by == ch->desc )
                    d->snoop_by = NULL;
          }
          notify_message ( ch, WIZNET_SNOOP, TO_IMM_ADMIN, victim->name );
          return;
     }

     if ( victim->desc->snoop_by != NULL )
     {
          send_to_char ( "Busy already.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     if ( ch->desc != NULL )
     {
          for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
          {
               if ( d->character == victim || d->original == victim )
               {
                    send_to_char ( "No snoop loops.\n\r", ch );
                    return;
               }
          }
     }

     victim->desc->snoop_by = ch->desc;
     send_to_char ( "Ok.\n\r", ch );
     notify_message ( ch, WIZNET_SNOOP, TO_IMM_ADMIN,
                      victim->name );
     return;
}

void do_switch ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Switch into whom?\n\r", ch );
          return;
     }

     if ( ch->desc == NULL )
          return;

     if ( ch->desc->original != NULL )
     {
          send_to_char ( "You are already switched.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "Ok.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( victim ) )
     {
          send_to_char ( "You can only switch into mobiles.\n\r", ch );
          return;
     }

     if ( victim->desc != NULL )
     {
          send_to_char ( "Character in use.\n\r", ch );
          return;
     }

     /* Zeran - to fix null pointer dereferencing all over in the code,
      * give switched player access to his pcdata fields. */
     victim->pcdata = ch->pcdata;

     ch->desc->character = victim;
     ch->desc->original = ch;
     victim->desc = ch->desc;
     ch->desc = NULL;
     /* change communications to match */
     victim->comm = ch->comm;
     victim->lines = ch->lines;
     send_to_char ( "Ok.\n\r", victim );
     log_string ( "Switch by %s Complete", ch->name );
     notify_message ( ch, WIZNET_SWITCH, TO_IMM, victim->short_descr );
     return;
}

void do_return ( CHAR_DATA * ch, char *argument )
{
     if ( ch->desc == NULL )
          return;

     if ( ch->desc->original == NULL )
     {
          send_to_char ( "You aren't switched.\n\r", ch );
          return;
     }

     send_to_char ( "You return to your original body.\n\r", ch );
     ch->pcdata = NULL;
     ch->desc->character = ch->desc->original;
     ch->desc->original = NULL;
     ch->desc->character->desc = ch->desc;
     ch->desc = NULL;
     return;
}

/* trust levels for load and clone */
bool obj_check ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     if ( IS_TRUSTED ( ch, GOD )
          || ( IS_TRUSTED ( ch, IMMORTAL )
               && obj->level <= 20
               && obj->cost <= 1000 )
          || ( IS_TRUSTED ( ch, DEMI )
               && obj->level <= 10
               && obj->cost <= 500 )
          || ( IS_TRUSTED ( ch, ANGEL )
               && obj->level <= 5
               && obj->cost <= 250 )
          || ( IS_TRUSTED ( ch, AVATAR )
               && obj->level == 0
               && obj->cost <= 100 ) )
          return TRUE;
     else
          return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */

void recursive_clone ( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone )
{
     OBJ_DATA           *c_obj, *t_obj;

     for ( c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content )
     {
          if ( obj_check ( ch, c_obj ) )
          {
               t_obj = create_object ( c_obj->pIndexData, 0 );
               clone_object ( c_obj, t_obj );
               obj_to_obj ( t_obj, clone );
               recursive_clone ( ch, c_obj, t_obj );
          }
     }
}

/* command that is similar to load */
void do_clone ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char               *rest;
     CHAR_DATA          *mob;
     OBJ_DATA           *obj;

     rest = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Clone what?\n\r", ch );
          return;
     }

     if ( !str_prefix ( arg, "object" ) )
     {
          mob = NULL;
          obj = get_obj_here ( ch, NULL, rest );
          if ( obj == NULL )
          {
               send_to_char ( "You don't see that here.\n\r", ch );
               return;
          }
     }
     else if ( !str_prefix ( arg, "mobile" ) ||
               !str_prefix ( arg, "character" ) )
     {
          obj = NULL;
          mob = get_char_room ( ch, NULL, rest );
          if ( mob == NULL )
          {
               send_to_char ( "You don't see that here.\n\r", ch );
               return;
          }
     }
     else			/* find both */
     {
          mob = get_char_room ( ch, NULL, argument );
          obj = get_obj_here ( ch, NULL, argument );
          if ( mob == NULL && obj == NULL )
          {
               send_to_char ( "You don't see that here.\n\r", ch );
               return;
          }
     }

    /* clone an object */
     if ( obj != NULL )
     {
          OBJ_DATA           *clone;

          if ( !obj_check ( ch, obj ) )
          {
               send_to_char
                    ( "Your powers are not great enough for such a task.\n\r",
                      ch );
               return;
          }

          clone = create_object ( obj->pIndexData, 0 );
          clone_object ( obj, clone );
          if ( obj->carried_by != NULL )
               obj_to_char ( clone, ch );
          else
               obj_to_room ( clone, ch->in_room );
          recursive_clone ( ch, obj, clone );

          act ( "$n has created $p.", ch, clone, NULL, TO_ROOM );
          act ( "You clone $p.", ch, clone, NULL, TO_CHAR );
          return;
     }
     else if ( mob != NULL )
     {
          CHAR_DATA          *clone;
          OBJ_DATA           *new_obj;

          if ( !IS_NPC ( mob ) )
          {
               send_to_char ( "You can only clone mobiles.\n\r", ch );
               return;
          }

          if ( ( mob->level > 20 && !IS_TRUSTED ( ch, GOD ) )
               || ( mob->level > 10 && !IS_TRUSTED ( ch, IMMORTAL ) )
               || ( mob->level > 5 && !IS_TRUSTED ( ch, DEMI ) )
               || ( mob->level > 0 && !IS_TRUSTED ( ch, ANGEL ) )
               || !IS_TRUSTED ( ch, AVATAR ) )
          {
               send_to_char
                    ( "Your powers are not great enough for such a task.\n\r",
                      ch );
               return;
          }

          clone = create_mobile ( mob->pIndexData );
          clone_mobile ( mob, clone );

          for ( obj = mob->carrying; obj != NULL;
                obj = obj->next_content )
          {
               if ( obj_check ( ch, obj ) )
               {
                    new_obj = create_object ( obj->pIndexData, 0 );
                    clone_object ( obj, new_obj );
                    recursive_clone ( ch, obj, new_obj );
                    obj_to_char ( new_obj, clone );
                    new_obj->wear_loc = obj->wear_loc;
               }
          }
          char_to_room ( clone, ch->in_room );
          act ( "$n has created $N.", ch, NULL, clone, TO_ROOM );
          act ( "You clone $N.", ch, NULL, clone, TO_CHAR );
          return;
     }
}

/* RT to replace the two load commands */

void do_load ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  load mob <vnum>\n\r", ch );
          send_to_char ( "  load obj <vnum> <level>\n\r", ch );
          return;
     }
     if ( !str_cmp ( arg, "mob" ) || !str_cmp ( arg, "char" ) )
     {
          do_mload ( ch, argument );
          return;
     }
     if ( !str_cmp ( arg, "obj" ) )
     {
          do_oload ( ch, argument );
          return;
     }
    /* echo syntax */
     do_load ( ch, "" );
}

void do_mload ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     MOB_INDEX_DATA     *pMobIndex;
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' || !is_number ( arg ) )
     {
          send_to_char ( "Syntax: load mob <vnum>.\n\r", ch );
          return;
     }

     if ( ( pMobIndex = get_mob_index ( atoi ( arg ) ) ) == NULL )
     {
          send_to_char ( "No mob has that vnum.\n\r", ch );
          return;
     }

     victim = create_mobile ( pMobIndex );
     char_to_room ( victim, ch->in_room );
     act ( "$n has created $N!", ch, NULL, victim, TO_ROOM );
     send_to_char ( "Ok.\n\r", ch );
     notify_message ( ch, WIZNET_LOAD, TO_IMM_ADMIN,
                      victim->short_descr );
     return;
}

void do_oload ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_INDEX_DATA     *pObjIndex;
     OBJ_DATA           *obj;
     int                 level;

     argument = one_argument ( argument, arg1 );
     one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || !is_number ( arg1 ) )
     {
          send_to_char ( "Syntax: load obj <vnum> <level>.\n\r", ch );
          return;
     }

     level = get_trust ( ch );	/* default */

     if ( arg2[0] != '\0' )	/* load with a level */
     {
          if ( !is_number ( arg2 ) )
          {
               send_to_char ( "Syntax: oload <vnum> <level>.\n\r", ch );
               return;
          }
          level = atoi ( arg2 );
          if ( level < 0 || level > get_trust ( ch ) )
          {
               send_to_char ( "Level must be be between 0 and your level.\n\r", ch );
               return;
          }
     }

     if ( ( pObjIndex = get_obj_index ( atoi ( arg1 ) ) ) == NULL )
     {
          send_to_char ( "No object has that vnum.\n\r", ch );
          return;
     }

     obj = create_object ( pObjIndex, level );
     /* Zeran - set size equal to creator's size */
     obj->size = ch->size;
     obj->level = level;
     if ( CAN_WEAR ( obj, ITEM_TAKE ) )
          obj_to_char ( obj, ch );
     else
          obj_to_room ( obj, ch->in_room );
     act ( "$n has created $p!", ch, obj, NULL, TO_ROOM );
     send_to_char ( "Ok.\n\r", ch );
     notify_message ( ch, WIZNET_LOAD, TO_IMM, obj->short_descr );
     return;
}

void do_purge ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     DESCRIPTOR_DATA    *d;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
	/* 'purge' */
          CHAR_DATA          *vnext;
          OBJ_DATA           *obj_next;

          for ( victim = ch->in_room->people; victim != NULL;
                victim = vnext )
          {
               vnext = victim->next_in_room;
               if ( IS_NPC ( victim )
                    && !IS_SET ( victim->act, ACT_NOPURGE )
                    && victim != ch ) 				// safety precaution
                    extract_char ( victim, TRUE );
          }

          for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
          {
               obj_next = obj->next_content;
               if ( !IS_OBJ_STAT ( obj, ITEM_NOPURGE ) )
                    extract_obj ( obj );
          }

          act ( "$n purges the room!", ch, NULL, NULL, TO_ROOM );
          send_to_char ( "Ok.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( victim ) )
     {

          if ( ch == victim )
          {
               send_to_char ( "Ho ho ho.\n\r", ch );
               return;
          }

          if ( get_trust ( ch ) <= get_trust ( victim ) )
          {
               send_to_char ( "Maybe that wasn't a good idea...\n\r", ch );
               form_to_char ( ch, "%s tried to purge you!\n\r",  ch->name );
               return;
          }

          act ( "$n disintegrates $N.", ch, 0, victim, TO_NOTVICT );

          if ( victim->level > 1 )
               save_char_obj ( victim );
          d = victim->desc;
          extract_char ( victim, TRUE );
          if ( d != NULL )
               close_socket ( d );

          return;
     }

     act ( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
     extract_char ( victim, TRUE );
     return;
}

void do_advance ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 level;
     int                 iLevel;
     int                 count;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number ( arg2 ) )
     {
          send_to_char ( "Syntax: advance <char> <level>.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg1 ) ) == NULL )
     {
          send_to_char ( "That player is not here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     if ( ( level = atoi ( arg2 ) ) < 1 || level > MAX_LEVEL )
     {
          form_to_char ( ch, "Level must be 1 to MAX_LEVEL (%d).\n\r", MAX_LEVEL );
          return;
     }

     if ( level > get_trust ( ch ) )
     {
          send_to_char ( "Limited to your trust level.\n\r", ch );
          return;
     }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
     if ( level <= victim->level )
     {
          int                 temp_prac;

          send_to_char ( "Lowering a player's level!\n\r", ch );
          send_to_char ( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
          temp_prac = victim->pcdata->practice;
          victim->level = 1;
          victim->exp = exp_per_level ( victim, victim->pcdata->points );
          victim->max_hit = 10;
          victim->max_mana = 100;
          victim->max_move = 100;
          victim->pcdata->practice = 0;
          victim->hit = victim->max_hit;
          victim->mana = victim->max_mana;
          victim->move = victim->max_move;
          mud.nonotify = TRUE;
          advance_level ( victim );
          mud.nonotify = FALSE;
          victim->pcdata->practice = temp_prac;
     }
     else
     {
          send_to_char ( "Raising a player's level!\n\r", ch );
          send_to_char ( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
     }

     count = 0;
     /* I always did think this was too spammy. No more. Well still somewhat to the 'victim' */
     mud.nonotify = TRUE;
     for ( iLevel = victim->level; iLevel < level; iLevel++ )
     {
          victim->level += 1;
          count++;
          advance_level ( victim );
     }
     form_to_char ( victim, "You just got %d levels!\n\r", count );

     mud.nonotify = FALSE;

     victim->exp = exp_per_level ( victim, victim->pcdata->points ) * UMAX ( 1, victim->level );
     victim->trust = 0;
     save_char_obj ( victim );
     return;
}

void do_trust ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 level;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number ( arg2 ) )
     {
          send_to_char ( "Syntax: trust <char> <level>.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg1 ) ) == NULL )
     {
          send_to_char ( "That player is not here.\n\r", ch );
          return;
     }

     if ( ( level = atoi ( arg2 ) ) < 0 || level > MAX_LEVEL )
     {
          form_to_char ( ch, "Level must be 0 (reset) or 1 to MAX_LEVEL (%d).\n\r", MAX_LEVEL );
          return;
     }

     if ( level > get_trust ( ch ) )
     {
          send_to_char ( "Limited to your trust.\n\r", ch );
          return;
     }

     victim->trust = level;
     return;
}

void do_restore ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     CHAR_DATA          *vch;
     DESCRIPTOR_DATA    *d;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' || !str_cmp ( arg, "room" ) )
     {
	/* cure room */

          for ( vch = ch->in_room->people; vch != NULL;  vch = vch->next_in_room )
          {
               affect_strip ( vch, gsn_plague );
               affect_strip ( vch, gsn_poison );
               affect_strip ( vch, gsn_blindness );
               affect_strip ( vch, gsn_sleep );
               affect_strip ( vch, gsn_curse );

               vch->hit = vch->max_hit;
               vch->mana = vch->max_mana;
               vch->move = vch->max_move;

               if ( !IS_NPC ( vch ) )
               {
                    vch->pcdata->condition[COND_THIRST] = 48;
                    vch->pcdata->condition[COND_FULL] = 48;
               }
               update_pos ( vch );
               act ( "$n has restored you.", ch, NULL, vch, TO_VICT );
          }

          send_to_char ( "Room restored.\n\r", ch );
          notify_message ( ch, WIZNET_RESTORE, TO_IMM_ADMIN, "local room" );
          return;

     }

     /* Set the level here for being able to restore all. */
     if ( get_trust ( ch ) >= MAX_LEVEL && !str_cmp ( arg, "all" ) )
     {
          /* cure all */
          for ( d = descriptor_list; d != NULL; d = d->next )
          {
               victim = d->character;

               if ( victim == NULL || IS_NPC ( victim ) )
                    continue;

               affect_strip ( victim, gsn_plague );
               affect_strip ( victim, gsn_poison );
               affect_strip ( victim, gsn_blindness );
               affect_strip ( victim, gsn_sleep );
               affect_strip ( victim, gsn_curse );

               victim->hit = victim->max_hit;
               victim->mana = victim->max_mana;
               victim->move = victim->max_move;
               victim->pcdata->condition[COND_THIRST] = 48;
               victim->pcdata->condition[COND_FULL] = 48;
               update_pos ( victim );
               if ( victim->in_room != NULL )
                    act ( "$n has restored everyone.", ch, NULL, victim, TO_VICT );
          }
          send_to_char ( "All active players restored.\n\r", ch );
          notify_message ( ch, WIZNET_RESTORE, TO_IMM_ADMIN, "all players" );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     affect_strip ( victim, gsn_plague );
     affect_strip ( victim, gsn_poison );
     affect_strip ( victim, gsn_blindness );
     affect_strip ( victim, gsn_sleep );
     affect_strip ( victim, gsn_curse );
     victim->hit = victim->max_hit;
     victim->mana = victim->max_mana;
     victim->move = victim->max_move;
     update_pos ( victim );
     act ( "$n has restored you.", ch, NULL, victim, TO_VICT );
     send_to_char ( "Ok.\n\r", ch );
     notify_message ( ch, WIZNET_RESTORE, TO_IMM_ADMIN, IS_NPC ( victim ) ? victim->short_descr : victim->name );
     return;
}

void do_freeze ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Freeze whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          form_to_char ( victim, "%s just tried to freeze you!\n\r", ch->name );
          return;
     }

     if ( IS_SET ( victim->act, PLR_FREEZE ) )
     {
          REMOVE_BIT ( victim->act, PLR_FREEZE );
          send_to_char ( "You can play again.\n\r", victim );
          send_to_char ( "FREEZE removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->act, PLR_FREEZE );
          send_to_char ( "You can't do ANYthing!\n\r", victim );
          send_to_char ( "FREEZE set.\n\r", ch );
     }

     save_char_obj ( victim );

     return;
}

void do_log ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Log whom?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "all" ) )
     {
          if ( fLogAll )
          {
               fLogAll = FALSE;
               send_to_char ( "Log ALL off.\n\r", ch );
          }
          else
          {
               fLogAll = TRUE;
               send_to_char ( "Log ALL on.\n\r", ch );
          }
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

    /*
     * No level check, gods can log anyone.
     * Bad idea. Lotherius.
     */
     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          form_to_char ( victim, "%s just tried to log you!\n\r", ch->name );
          return;
     }

     if ( IS_SET ( victim->act, PLR_LOG ) )
     {
          REMOVE_BIT ( victim->act, PLR_LOG );
          send_to_char ( "LOG removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->act, PLR_LOG );
          send_to_char ( "LOG set.\n\r", ch );
     }

     return;
}

void do_noemote ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Noemote whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_NOEMOTE ) )
     {
          REMOVE_BIT ( victim->comm, COMM_NOEMOTE );
          send_to_char ( "You can emote again.\n\r", victim );
          send_to_char ( "NOEMOTE removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->comm, COMM_NOEMOTE );
          send_to_char ( "You can't emote!\n\r", victim );
          send_to_char ( "NOEMOTE set.\n\r", ch );
     }

     return;
}

void do_noshout ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Noshout whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_NOSHOUT ) )
     {
          REMOVE_BIT ( victim->comm, COMM_NOSHOUT );
          send_to_char ( "You can shout again.\n\r", victim );
          send_to_char ( "NOSHOUT removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->comm, COMM_NOSHOUT );
          send_to_char ( "You can't shout!\n\r", victim );
          send_to_char ( "NOSHOUT set.\n\r", ch );
     }

     return;
}

void do_notell ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Notell whom?", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( get_trust ( victim ) >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     if ( IS_SET ( victim->comm, COMM_NOTELL ) )
     {
          REMOVE_BIT ( victim->comm, COMM_NOTELL );
          send_to_char ( "You can tell again.\n\r", victim );
          send_to_char ( "NOTELL removed.\n\r", ch );
     }
     else
     {
          SET_BIT ( victim->comm, COMM_NOTELL );
          send_to_char ( "You can't tell!\n\r", victim );
          send_to_char ( "NOTELL set.\n\r", ch );
     }

     return;
}

void do_peace ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA         *rch;
     char		arg[MIL];

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
          {
               if ( rch->fighting )
               {
                    stop_fighting ( rch, TRUE );
                    if ( IS_NPC ( ch ) )
                         do_sit ( rch, "" );
               }
          }
     }
     else if ( !str_cmp ( arg, "all" ) || !str_cmp ( arg, "world" ) )
     {
          send_to_char ( "Sorry, peace world is broken. Get your local coder to fix it.\n\r", ch );
          return;
          /*
           DESCRIPTOR_DATA   *d;
           for ( d = descriptor_list; d != NULL; d = d->next )
           {
           rch = d->character;
           if ( rch->fighting )
           {
           stop_fighting ( rch, TRUE );
           do_sit ( rch, "" );
           do_sit ( rch->fighting "" );
           }
           }
           */
     }
     else
     {
          send_to_char ("Syntax: peace\n\r        peace world", ch );
          return;
     }
     send_to_char ( "Ok.\n\r", ch );
     return;
}

BAN_DATA           *ban_free;
BAN_DATA           *ban_list;

void do_ban ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buf;
     char                arg[MAX_INPUT_LENGTH];
     BAN_DATA           *pban;

     if ( IS_NPC ( ch ) )
          return;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          buf = buffer_new(1000);

          bprintf ( buf, "Banned sites:\n\r" );
          for ( pban = ban_list; pban != NULL; pban = pban->next )
          {
               bprintf ( buf, pban->name );
               bprintf ( buf, "\n\r" );
          }
          page_to_char ( buf->data, ch );
          buffer_free(buf);
          return;
     }

     for ( pban = ban_list; pban != NULL; pban = pban->next )
     {
          if ( !str_cmp ( arg, pban->name ) )
          {
               send_to_char ( "That site is already banned!\n\r", ch );
               return;
          }
     }

     if ( ban_free == NULL )
     {
          pban = alloc_perm ( sizeof ( *pban ), "pban" );
     }
     else
     {
          pban = ban_free;
          ban_free = ban_free->next;
     }

     pban->name = str_dup ( arg );
     pban->next = ban_list;
     ban_list = pban;
     send_to_char ( "Ok.\n\r", ch );
     return;
}

void do_allow ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     BAN_DATA           *prev;
     BAN_DATA           *curr;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Remove which site from the ban list?\n\r", ch );
          return;
     }

     prev = NULL;
     for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
     {
          if ( !str_cmp ( arg, curr->name ) )
          {
               if ( prev == NULL )
                    ban_list = ban_list->next;
               else
                    prev->next = curr->next;

               free_string ( curr->name );
               curr->next = ban_free;
               ban_free = curr;
               send_to_char ( "Ok.\n\r", ch );
               return;
          }
     }

     send_to_char ( "Site is not banned.\n\r", ch );
     return;
}

void do_wizlock ( CHAR_DATA * ch, char *argument )
{
     extern bool         wizlock;

     wizlock = !wizlock; // Gee, nice simple way of toggling a bool that I never thought of.
     // Wouldn't programming classes have been nice?
     if ( wizlock )
          send_to_char ( "Game wizlocked.\n\r", ch );
     else
          send_to_char ( "Game un-wizlocked.\n\r", ch );

     return;
}

/* RT anti-newbie code */

void do_newlock ( CHAR_DATA * ch, char *argument )
{
     extern bool         newlock;

     newlock = !newlock;
     if ( newlock )
          send_to_char ( "New characters have been locked out.\n\r", ch );
     else
          send_to_char ( "Newlock removed.\n\r", ch );

     return;
}

void do_slookup ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 sn;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Lookup which skill or spell?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "all" ) )
     {
          for ( sn = 0; sn < MAX_SKILL; sn++ )
          {
               if ( skill_table[sn].name == NULL )
                    break;
               form_to_char ( ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                              sn, skill_table[sn].slot,
                              skill_table[sn].name );
          }
     }
     else
     {
          if ( ( sn = skill_lookup ( arg ) ) < 0 )
          {
               send_to_char ( "No such skill or spell.\n\r", ch );
               return;
          }
          form_to_char ( ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                         sn, skill_table[sn].slot, skill_table[sn].name );
     }

     return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set mob   <name>   <field> <value>\n\r", ch );
          send_to_char ( "  set obj   <name>   <field> <value>\n\r", ch );
          send_to_char ( "  set room  <room>   <field> <value>\n\r", ch );
          send_to_char ( "  set known <player> <spell or skill> <value>\n\r",  ch );
          send_to_char ( "  set clan  <clan>   <field> <value>\n\r", ch );
          send_to_char ( "  set skill <skill or spell> <field> <value>\n\r", ch );
          return;
     }

     if ( !str_prefix ( arg, "skill" ) ||
          !str_prefix ( arg, "spell" ) )
     {
          do_setskill ( ch, argument );
          return;
     }

     if ( !str_prefix ( arg, "mobile" ) ||
          !str_prefix ( arg, "character" ) )
     {
          do_mset ( ch, argument );
          return;
     }

     if ( !str_prefix ( arg, "known" ) )
     {
          do_sset ( ch, argument );
          return;
     }

     if ( !str_prefix ( arg, "object" ) )
     {
          do_oset ( ch, argument );
          return;
     }

     if ( !str_prefix ( arg, "room" ) )
     {
          do_rset ( ch, argument );
          return;
     }

     if ( !str_prefix ( arg, "clan" ) )
     {
          do_cset ( ch, argument );
          return;
     }

    /* echo syntax */
     do_set ( ch, "" );
}

void do_setskill ( CHAR_DATA * ch, char *argument )
{
     char                field_name[MAX_INPUT_LENGTH], skill_name[MAX_INPUT_LENGTH];
     int                 sn, a, class_no;

     argument = one_argument ( argument, skill_name );
     argument = one_argument ( argument, field_name );

     if ( !argument[0] )
     {
          send_to_char
               ( "Syntax is: set skill <skill or spell> <field> <value>.\n\r"
                 "  Possible Field/Value Combinations:\n\r"
                 "  <class> <level>        - 3-Letter Class & New Skill Level for that class\n\r"
                 "  target <target>        - Where value is a valid target type (help targets)\n\r"
                 "  position <position>    - Minimum position can used in (sleep, sit, etc)\n\r"
                 "  mana <value>           - Lowest mana cost for a spell\n\r"
                 "  lag <value>            - Pulses to lag player after using the skill/spell.\n\r"
                 "  component <vnum>       - VNum of a required component to cast a spell.\n\r"
                 "  wearoff <text>         - Message to be sent to the player for things that wear off.\n\r"
                 "  noun <text>            - Damage message used to refer to the skill/spell. (none to clear)\n\r", ch );
          return;
     }

     if ( ( sn = skill_lookup ( skill_name ) ) == -1 )
     {
          send_to_char ( "There is no such spell/skill as that.\n\r", ch );
          return;
     }

     for ( class_no = 0; class_no < MAX_CLASS; class_no++ )
          if ( !str_cmp ( field_name, class_table[class_no].who_name ) )
               break;

     if ( class_no != MAX_CLASS ) // Kludge... if the first argument is the name of a class, then we
          // assume we're setting a level.
     {
          a = atoi ( argument );

          if ( !is_number ( argument ) || a < 0 || a > LEVEL_IMMORTAL )
          {
               send_to_char ( "Level range is from 0 to 110.\n\r", ch );
               return;
          }

          skill_table[sn].skill_level[class_no] = a;

          form_to_char ( ch, "OK, %ss will now gain %s at level %d%s.\n\r",
                         class_table[class_no].name, skill_table[sn].name,
                         a, a == LEVEL_IMMORTAL ? " (i.e. never)" : "" );
     }
     else if ( !str_cmp ( field_name, "target" ) )
     {
          // Change the skill/spell default target
          // Possible Targets: TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF, TAR_OBJ_INV
          //
          if (!str_cmp ( argument, "ignore" ) )
          {
               skill_table[sn].target = TAR_IGNORE;
               form_to_char ( ch, "%s now has default target: IGNORE.\n\r", skill_table[sn].name);
          }
          else if (!str_cmp ( argument, "offensive" ) )
          {
               skill_table[sn].target = TAR_CHAR_OFFENSIVE;
               form_to_char ( ch, "%s now has default target: OFFENSIVE.\n\r", skill_table[sn].name);
          }
          else if (!str_cmp ( argument, "defensive" ) )
          {
               skill_table[sn].target = TAR_CHAR_DEFENSIVE;
               form_to_char ( ch, "%s now has default target: DEFENSIVE.\n\r", skill_table[sn].name);
          }
          else if (!str_cmp ( argument, "self" ) )
          {
               skill_table[sn].target = TAR_CHAR_SELF;
               form_to_char ( ch, "%s now has default target: SELF.\n\r", skill_table[sn].name);
          }
          else if (!str_cmp ( argument, "object" ) )
          {
               skill_table[sn].target = TAR_OBJ_INV;
               form_to_char ( ch, "%s now has default target: OBJECT.\n\r", skill_table[sn].name);
          }
          else
               send_to_char ( "Valid targets are: ignore, offensive, defensive, self, object.\n\r", ch );

          save_skills( );
          return;
     }
     else if ( !str_cmp (field_name, "position" ) )
     {
          // minimum_position can be: POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_RESTING,
          // POS_SITTING, POS_FIGHTING, POS_STANDING
          //
          if ( !str_cmp ( argument, "dead" ) )
          {
               skill_table[sn].minimum_position = POS_DEAD;
               form_to_char ( ch, "%s now has min position: DEAD.\n\r", skill_table[sn].name);
          }
          else if ( !str_cmp ( argument, "mortal" ) )
          {
               skill_table[sn].minimum_position = POS_MORTAL;
               form_to_char ( ch, "%s now has min position: MORTALLY WOUNDED.\n\r", skill_table[sn].name);
          }
          else if ( !str_cmp ( argument, "incap" ) )
          {
               skill_table[sn].minimum_position = POS_INCAP;
               form_to_char ( ch, "%s now has min position: INCAPACITATED.\n\r", skill_table[sn].name);
          }
          else if ( !str_cmp ( argument, "stunned" ) )
          {
               skill_table[sn].minimum_position = POS_STUNNED;
               form_to_char ( ch, "%s now has min position: STUNNED.\n\r", skill_table[sn].name);
          }
          else if ( !str_cmp ( argument, "sleeping" ) )
          {
               skill_table[sn].minimum_position = POS_SLEEPING;
               form_to_char ( ch, "%s now has min position: SLEEPING.\n\r", skill_table[sn].name );
          }
          else if ( !str_cmp ( argument, "resting" ) )
          {
               skill_table[sn].minimum_position = POS_RESTING;
               form_to_char ( ch, "%s now has min position: RESTING.\n\r", skill_table[sn].name );
          }
          else if ( !str_cmp ( argument, "sitting" ) )
          {
               skill_table[sn].minimum_position = POS_SITTING;
               form_to_char ( ch, "%s now has min position: SITTING.\n\r", skill_table[sn].name );
          }
          else if ( !str_cmp ( argument, "standing" ) )
          {
               skill_table[sn].minimum_position = POS_STANDING;
               form_to_char ( ch, "%s now has min position: STANDING.\n\r", skill_table[sn].name );
          }
          else if ( !str_cmp ( argument, "fighting" ) )
          {
               skill_table[sn].minimum_position = POS_FIGHTING;
               form_to_char ( ch, "%s now has min position: FIGHTING.\n\r", skill_table[sn].name );
          }
          else
               send_to_char
               ( "Valid values for position are: dead, mortal, incap, stunned, sleeping,"
                 "resting, sitting, standing, fighting.\n\r", ch );
          save_skills( );
          return;
     }
     else if ( !str_cmp (field_name, "mana" ) )
     {
          a = atoi ( argument );

          if ( !is_number ( argument ) || a < 1 || a > 1000 )
          {
               send_to_char ( "Mana must be from 1 to 1000.\n\r", ch );
               return;
          }
          skill_table[sn].min_mana = a;
          form_to_char ( ch, "%s will now cost a minimum of %d mana.\n\r", skill_table[sn].name, a);
          save_skills();
          return;
     }
     else if ( !str_cmp (field_name, "lag" ) )
     {
          a = atoi ( argument );

          if ( !is_number ( argument ) || a < 0 || a > 50 )
          {
               send_to_char ( "Lag must be from 0 to 50.\n\r", ch );
               return;
          }
          skill_table[sn].beats = a;
          form_to_char ( ch, "%s will now lag for %d beats.\n\r", skill_table[sn].name, a);
          save_skills();
          return;
     }
     else if ( !str_cmp (field_name, "component" ) ) // component
     {
          if ( strlen (argument) > 128 )
          {
               send_to_char ( "Please limit component keywords to 127 characters.\n\r", ch);
               return;
          }

          if ( !str_cmp (argument, "none" ) ) // clear it
               argument[0] = '\0';

          skill_table[sn].component = str_dup ( argument );
          form_to_char ( ch, "%s now uses the component: %s.\n\r", skill_table[sn].name, argument );
          save_skills();

          return;
     }
     else if ( !str_cmp (field_name, "wearoff" ) ) // msg_off
     {
          if ( strlen (argument) > 61 )
          {
               send_to_char ( "Please limit the wearoff message to 60 characters or less.\n\r", ch);
               return;
          }

          if ( !str_cmp (argument, "none" ) ) // clear it
               argument[0] = '\0';

          skill_table[sn].msg_off = str_dup ( argument );
          form_to_char ( ch, "%s now uses the following wearoff message: %s.\n\r", skill_table[sn].name, argument );
          save_skills( );

          return;
     }
     else if ( !str_cmp (field_name, "noun" ) ) // noun_damage
     {
          if ( strlen (argument) > 25 )
          {
               send_to_char ( "Please limit the damage noun to 24 characters or less.\n\r", ch);
               return;
          }

          if ( !str_cmp (argument, "none" ) ) // clear it
               argument[0] = '\0';

          skill_table[sn].noun_damage = str_dup ( argument );
          form_to_char ( ch, "%s now uses the following damage noun: %s.\n\r", skill_table[sn].name, argument );
          save_skills( );

          return;
     }
     else
          send_to_char ( "Please run 'set skill' with no arguments to recieve syntax help.\n\r", ch);

     save_classes( );
     return;
}

void do_sset ( CHAR_DATA * ch, char *argument ) // Called for set known
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                arg3[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 value;
     int                 sn;
     bool                fAll;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     argument = one_argument ( argument, arg3 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set skill <name> <spell or skill> <value>\n\r", ch );
          send_to_char ( "  set skill <name> all <value>\n\r", ch );
          send_to_char ( "   (use the name of the skill, not the number)\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "Not on NPC's.\n\r", ch );
          return;
     }

     fAll = !str_cmp ( arg2, "all" );
     sn = 0;
     if ( !fAll && ( sn = skill_lookup ( arg2 ) ) < 0 )
     {
          send_to_char ( "No such skill or spell.\n\r", ch );
          return;
     }

     /*
      * Snarf the value.
      */
     if ( !is_number ( arg3 ) )
     {
          send_to_char ( "Value must be numeric.\n\r", ch );
          return;
     }

     value = atoi ( arg3 );
     if ( value < 0 || value > 100 )
     {
          send_to_char ( "Value range is 0 to 100.\n\r", ch );
          return;
     }

     if ( fAll )
     {
          for ( sn = 0; sn < MAX_SKILL; sn++ )
          {
               if ( skill_table[sn].name != NULL )
                    victim->pcdata->learned[sn] = value;
          }
     }
     else
     {
          victim->pcdata->learned[sn] = value;
     }

     return;
}

void do_mset ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                arg3[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 value;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     SLCPY ( arg3, argument );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set char <name> <field> <value>\n\r", ch );
          send_to_char ( "  Field being one of:\n\r", ch );
          send_to_char ( "    str int wis dex con sex class level\n\r", ch );
          send_to_char ( "    race gold hp mana move practice align\n\r", ch );
          send_to_char ( "    train thirst drunk full security timer\n\r", ch );
          send_to_char ( "    recall pkwin pkloss qptotal qp nextq\n\r", ch );
          send_to_char ( "    status\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( ( !IS_NPC ( victim ) ) && ( ch->level != IMPLEMENTOR ) )	/*Zeran: check level */
     {
          if ( ch->level < victim->level )
          {
               send_to_char( "You do not have authority to do that.\n\r", ch );
               return;
          }
     }

    /*
     * Snarf the value (which need not be numeric).
     */
     value = is_number ( arg3 ) ? atoi ( arg3 ) : -1;

    /*
     * Set something.
     */
     if ( !str_cmp ( arg2, "str" ) )
     {
          if ( value < 3 || value > 25 )
          {
               send_to_char ( "Strength range is 3 to 25\n\r.", ch  );
               return;
          }

          victim->perm_stat[STAT_STR] = value;
          return;
     }

     if ( !str_cmp ( arg2, "recall" ) )
     {
          victim->recall_perm = value;
          return;
     }

     if ( !str_cmp ( arg2, "int" ) )
     {
          if ( value < 3 || value > 25 )
          {
               send_to_char ( "Intelligence range is 3 to 25.\n\r", ch );
               return;
          }

          victim->perm_stat[STAT_INT] = value;
          return;
     }

     if ( !str_cmp ( arg2, "wis" ) )
     {
          if ( value < 3 || value > 25 )
          {
               send_to_char ( "Wisdom range is 3 to 25.\n\r", ch );
               return;
          }
          victim->perm_stat[STAT_WIS] = value;
          return;
     }

     if ( !str_cmp ( arg2, "dex" ) )
     {
          if ( value < 3 || value > 25 )
          {
               send_to_char ( "Dexterity range is 3 to 25.\n\r", ch );
               return;
          }

          victim->perm_stat[STAT_DEX] = value;
          return;
     }

     if ( !str_cmp ( arg2, "con" ) )
     {
          if ( value < 3 || value > 25 )
          {
               send_to_char ( "Constitution range is 3 to 25.\n\r", ch );
               return;
          }

          victim->perm_stat[STAT_CON] = value;
          return;
     }

     if ( !str_prefix ( arg2, "sex" ) )
     {
          if ( value < 0 || value > 2 )
          {
               send_to_char ( "Sex range is 0 to 2.\n\r", ch );
               return;
          }
          victim->sex = value;
          if ( !IS_NPC ( victim ) )
               victim->pcdata->true_sex = value;
          return;
     }

     if ( !str_prefix ( arg2, "class" ) )
     {
          int                 nclass;

          if ( IS_NPC ( victim ) )
          {
               send_to_char ( "Mobiles have no class.\n\r", ch );
               return;
          }

          nclass = class_lookup ( arg3 );
          if ( nclass == -1 )
          {
               char                buf[MAX_STRING_LENGTH];

               SLCPY ( buf, "Possible classes are: " );
               for ( nclass = 0; nclass < MAX_CLASS; nclass++ )
               {
                    if ( nclass > 0 )
                         SLCAT ( buf, " " );
                    SLCAT ( buf, class_table[nclass].name );
               }
               SLCAT ( buf, ".\n\r" );

               send_to_char ( buf, ch );
               return;
          }

          victim->pcdata->pclass = nclass;
          return;
     }

     if ( !str_prefix ( arg2, "level" ) )
     {
          if ( !IS_NPC ( victim ) )
          {
               send_to_char ( "Not on PC's. Used 'advance'.\n\r", ch );
               return;
          }

          if ( value < 0 || value > MAX_LEVEL )
          {
               send_to_char ( "Level range is 0 to 110.\n\r", ch );
               return;
          }
          victim->level = value;
          return;
     }

     if ( !str_prefix ( arg2, "gold" ) )
     {
          victim->gold = value;
          return;
     }

     if ( !str_prefix ( arg2, "pkwin" ) )
     {
          if (IS_NPC(victim) )
               return;
          victim->pcdata->pkill_wins = value;
          return;
     }

     if ( !str_prefix (arg2, "pkloss" ) )
     {
          if (IS_NPC(victim) )
               return;
          victim->pcdata->pkill_losses = value;
          return;
     }

     if ( !str_prefix ( arg2, "qp" ) )
     {
          if (IS_NPC(victim) )
               return;
          victim->pcdata->questpoints = value;
          return;
     }

     if ( !str_prefix (arg2, "qptotal" ) )
     {
          if (IS_NPC(victim) )
               return;
          victim->pcdata->questearned = value;
          return;
     }

     if ( !str_prefix ( arg2, "nextq" ) )
     {
          if (IS_NPC(victim) )
               return;
          victim->pcdata->nextquest = value;
          return;
     }

     if ( !str_prefix ( arg2, "hp" ) )
     {
          if ( value < -10 || value > 30000 )
          {
               send_to_char ( "Hp range is -10 to 30,000 hit points.\n\r", ch );
               return;
          }
          victim->max_hit = value;
          if ( !IS_NPC ( victim ) )
               victim->pcdata->perm_hit = value;
          return;
     }

     if ( !str_prefix ( arg2, "mana" ) )
     {
          if ( value < 0 || value > 30000 )
          {
               send_to_char ( "Mana range is 0 to 30,000 mana points.\n\r", ch );
               return;
          }
          victim->max_mana = value;
          if ( !IS_NPC ( victim ) )
               victim->pcdata->perm_mana = value;
          return;
     }

     if ( !str_prefix ( arg2, "move" ) )
     {
          if ( value < 0 || value > 30000 )
          {
               send_to_char ( "Move range is 0 to 30,000 move points.\n\r", ch );
               return;
          }
          victim->max_move = value;
          if ( !IS_NPC ( victim ) )
               victim->pcdata->perm_move = value;
          return;
     }

     if ( !str_prefix ( arg2, "practice" ) )
     {
          if (IS_NPC(victim) )
               return;
          if ( value < 0 || value > 250 )
          {
               send_to_char ( "Practice range is 0 to 250 sessions.\n\r", ch );
               return;
          }
          victim->pcdata->practice = value;
          return;
     }

     if ( !str_prefix ( arg2, "train" ) )
     {
          if (IS_NPC(victim) )
               return;
          if ( value < 0 || value > 50 )
          {
               send_to_char( "Training session range is 0 to 50 sessions.\n\r", ch );
               return;
          }
          victim->pcdata->train = value;
          return;
     }

/* OLC */
     if ( !str_cmp ( arg2, "security" ) )	/* OLC */
     {
          if ( IS_NPC ( victim ) )
          {
               send_to_char ( "Not on NPC's.\n\r", ch );
               return;
          }

          if ( value > ch->pcdata->security || value < 0 )
          {
               if ( ch->pcdata->security != 0 )
               {
                    form_to_char ( ch, "Valid security is 0-%d.\n\r", ch->pcdata->security );
               }
               else
               {
                    send_to_char ( "Valid security is 0 only.\n\r",  ch );
               }
               return;
          }
          victim->pcdata->security = value;
          return;
     }

     if ( !str_prefix ( arg2, "align" ) )
     {
          if ( value < -1000 || value > 1000 )
          {
               send_to_char ( "Alignment range is -1000 to 1000.\n\r", ch );
               return;
          }
          victim->alignment = value;
          return;
     }

     /* Lotherius added timer */
     if ( !str_prefix ( arg2, "timer" ) )
     {
          if (IS_NPC(victim) )
               return;
          if ( value < 0 || value > 30000 )
          {
               send_to_char ( "Timer Range 0 - 30000.\n\r", ch );
               return;
          }
          victim->timer = value;
          return;
     }

     /* Setting of Status should be limited to higher imms only */
     /* One may wish to tweak these access levels */

     if ( !str_prefix ( arg2, "status" ) )
     {
          if (IS_NPC(victim) )
          {
               send_to_char ("Not on NPC's.\n\r", ch);
               return;
          }

          if ( !strcmp ( arg3, "verified" ) && (ch->level >= DEITY) )
          {
               victim->pcdata->account->status = ACCT_VERIFIED;
               fwrite_accounts();
               return;
          }
          if ( !strcmp ( arg3, "demistat" ) && (ch->level >= SUPREME) )
          {
               victim->pcdata->account->status = ACCT_VERIFIED_DEMISTAT;
               fwrite_accounts();
               return;
          }
          if ( !strcmp ( arg3, "helper" ) && (ch->level >= CREATOR ) )
          {
               victim->pcdata->account->status = ACCT_HELPER;
               fwrite_accounts();
               return;
          }
          if ( !strcmp ( arg3, "staff" ) && (ch->level >= CREATOR ) )
          {
               victim->pcdata->account->status = ACCT_STAFF;
               fwrite_accounts();
               return;
          }
          if ( !strcmp ( arg3, "implementor" ) && (ch->level >= IMPLEMENTOR) )
          {
               victim->pcdata->account->status = ACCT_IMPLEMENTOR;
               fwrite_accounts();
               return;
          }

	/* We obviously had an invalid entry */

          send_to_char ("For rejects, deletes,  and unverify, use the \"reject\" command.\n\r", ch);
          send_to_char ("Depending upon your access, you may set the following here:\n\r", ch);
          send_to_char ("  verified demistat helper staff implementor\n\r", ch);
          send_to_char ("ALWAYS use this command with care.\n\r", ch);

          return;
     }
     /* End of status */

     if ( !str_prefix ( arg2, "thirst" ) )
     {
          if ( IS_NPC ( victim ) )
          {
               send_to_char ( "Not on NPC's.\n\r", ch );
               return;
          }

          if ( value < -1 || value > 100 )
          {
               send_to_char ( "Thirst range is -1 to 100.\n\r", ch );
               return;
          }

          victim->pcdata->condition[COND_THIRST] = value;
          return;
     }

     if ( !str_prefix ( arg2, "drunk" ) )
     {
          if ( IS_NPC ( victim ) )
          {
               send_to_char ( "Not on NPC's.\n\r", ch );
               return;
          }

          if ( value < -1 || value > 100 )
          {
               send_to_char ( "Drunk range is -1 to 100.\n\r", ch );
               return;
          }

          victim->pcdata->condition[COND_DRUNK] = value;
          return;
     }

     if ( !str_prefix ( arg2, "full" ) )
     {
          if ( IS_NPC ( victim ) )
          {
               send_to_char ( "Not on NPC's.\n\r", ch );
               return;
          }

          if ( value < -1 || value > 100 )
          {
               send_to_char ( "Full range is -1 to 100.\n\r", ch );
               return;
          }

          victim->pcdata->condition[COND_FULL] = value;
          return;
     }

     if ( !str_prefix ( arg2, "race" ) )
     {
          int                 race;

          race = race_lookup ( arg3 );

          if ( race == 0 )
          {
               send_to_char ( "That is not a valid race.\n\r", ch );
               return;
          }

          if ( !IS_NPC ( victim ) && !race_table[race].pc_race )
          {
               send_to_char ( "That is not a valid player race.\n\r", ch );
               return;
          }

          victim->race = race;
          return;
     }

    /*
     * Generate usage message.
     */
     do_mset ( ch, "" );
     return;
}

/* Immortal Clan Set for those things not included in cedit     */
/* Usually these are things that shouldn't be changed as often. */

void do_cset ( CHAR_DATA * ch, char *argument )
{
     CLAN_INDEX         *tmp;
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     int                 value;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set clan <clan> <field> <value>\n\r", ch );
          send_to_char ( "  Field being one of:\n\r", ch );
          send_to_char ( "  clan_short clanpk clandie clanbank moveyear membercount "
                         "\n\r  experience probation underground\n\r", ch );
          send_to_char ( "Other fields may be changed with cedit.\n\r", ch );
          return;
     }

     tmp = clan_by_short (arg1);

     if (!tmp)
     {
          send_to_char("No such clan found.\n\r", ch);
          return;
     }
    /* Set what needs set */

     if ( !strcmp ( arg2, "probation" ) )
     {
          send_to_char ( "Clan set to probation. No notify is sent for this.\n\r", ch );
          tmp->status = CLAN_PROBATION;
          save_one_clan ( tmp );
          return;
     }
     if ( !strcmp ( arg2, "underground" ) )
     {
          send_to_char ( "Clan set to {DUnderGround{w. No notify is sent for this.\n\r", ch );
          tmp->status = CLAN_UNDERGROUND;
          save_one_clan ( tmp );
     }
     if ( !strcmp ( arg2, "clan_short" ) )
     {
          send_to_char ( "Be forewarned: Any members of this clan now have invalid clan names!\n\r", ch );
          tmp->clan_short = str_dup (argument);
          save_one_clan ( tmp );
          return;
     }

    /* Numeric fields all follow */
    /* Use the following on numeric fields */

     if ( !is_number ( argument ) )
     {
          send_to_char ( "Value for this action must be numeric (Or you entered an invalid action).\n\r", ch );
          return;
     }
     value = atoi ( argument );
     if ( !strcmp ( arg2, "clanpk" ) )
     {
          tmp->clanpk = value;
          save_one_clan ( tmp );
          return;
     }
     if ( !strcmp ( arg2, "experience" ) )
     {
          tmp->experience = value;
          save_one_clan ( tmp );
          return;
     }
     if ( !strcmp ( arg2, "clandie" ) )
     {
          tmp->clandie = value;
          save_one_clan ( tmp );
          return;
     }

     if ( !strcmp ( arg2, "clanbank" ) )
     {
          tmp->clanbank = value;
          save_one_clan ( tmp );
          return;
     }

     if ( !strcmp ( arg2, "moveyear" ) )
     {
          tmp->moveyear = value;
          save_one_clan ( tmp );
          return;
     }

     if ( !strcmp ( arg2, "membercount" ) )
     {
          tmp->membercount = value;
          save_one_clan ( tmp );
          return;
     }

    /*
     * Generate usage message.
     */

     do_cset ( ch, "" );
     return;
}

void do_string ( CHAR_DATA * ch, char *argument )
{
     char                type[MIL];
     char                arg1[MIL];
     char                arg2[MIL];
     char                arg3[MIL];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;

     argument = one_argument ( argument, type );
     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     SLCPY ( arg3, argument );

     if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' ||
          arg3[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  string char <name> <field> <string>\n\r", ch );
          send_to_char ( "    fields: name short long desc title spec email\n\r", ch );
          send_to_char ( "  string obj  <name> <field> <string>\n\r", ch );
          send_to_char ( "    fields: name short long extended wear owear\n\r", ch );
          return;
     }

     if ( !str_prefix ( type, "character" ) || !str_prefix ( type, "mobile" ) )
     {
          if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
          {
               send_to_char ( "They aren't here.\n\r", ch );
               return;
          }

          /* string something */

          if ( !str_prefix ( arg2, "name" ) )
          {
               if ( !IS_NPC ( victim ) )
               {
                    send_to_char ( "Not on PC's. Use 'RENAME'.\n\r", ch );
                    return;
               }

               free_string ( victim->name );
               victim->name = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "description" ) )
          {
               free_string ( victim->description );
               victim->description = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "short" ) )
          {
               free_string ( victim->short_descr );
               victim->short_descr = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "long" ) )
          {
               free_string ( victim->long_descr );
               SLCAT ( arg3, "\n\r" );
               victim->long_descr = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "title" ) )
          {
               if ( IS_NPC ( victim ) )
               {
                    send_to_char ( "Not on NPC's.\n\r", ch );
                    return;
               }

               set_title ( victim, arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "email" ) )
          {
               if ( IS_NPC ( victim ) )
               {
                    send_to_char ("Not on NPC's.\n\r", ch);
                    return;
               }

               free_string ( victim->pcdata->email );
               victim->pcdata->email= str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "spec" ) )
          {
               if ( !IS_NPC ( victim ) )
               {
                    send_to_char ( "Not on PC's.\n\r", ch );
                    return;
               }

               if ( ( victim->spec_fun = spec_lookup ( arg3 ) ) == 0 )
               {
                    send_to_char ( "No such spec fun.\n\r", ch );
                    return;
               }

               return;
          }
     }

     if ( !str_prefix ( type, "object" ) )
     {
	/* string an obj */

          if ( ( obj = get_obj_world ( ch, arg1 ) ) == NULL )
          {
               send_to_char ( "Nothing like that in heaven or earth.\n\r", ch );
               return;
          }

          if ( !str_prefix ( arg2, "name" ) )
          {
               free_string ( obj->name );
               obj->name = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "short" ) )
          {
               free_string ( obj->short_descr );
               obj->short_descr = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "long" ) )
          {
               free_string ( obj->description );
               obj->description = str_dup ( arg3 );
               return;
          }

          if ( !str_prefix ( arg2, "ed" ) ||
               !str_prefix ( arg2, "extended" ) )
          {
               EXTRA_DESCR_DATA   *ed;

               argument = one_argument ( argument, arg3 );
               if ( argument == NULL )
               {
                    send_to_char ( "Syntax: oset <object> ed <keyword> <string>\n\r", ch );
                    return;
               }

               SLCAT ( argument, "\n\r" );

               if ( extra_descr_free == NULL )
               {
                    ed = alloc_perm ( sizeof ( *ed ), "EXTRA_DESCR_DATA: string" );
               }
               else
               {
                    ed = extra_descr_free;
                    extra_descr_free = ed->next;
               }

               ed->keyword = str_dup ( arg3 );
               ed->description = str_dup ( argument );
               ed->next = obj->extra_descr;
               obj->extra_descr = ed;
               return;
          }
     }
    /* echo bad use message */
     do_string ( ch, "" );
}

void do_oset ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                arg3[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 value;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     SLCPY ( arg3, argument );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set obj <object> <field> <value>\n\r",  ch );
          send_to_char ( "  Field being one of:\n\r", ch );
          send_to_char ( "    value0 value1 value2 value3 value4 (v1-v4)\n\r",  ch );
          send_to_char ( "    extra wear level weight cost timer size condition\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_world ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "Nothing like that in heaven or earth.\n\r", ch );
          return;
     }

    /*
     * Snarf the value (which need not be numeric).
     */
     value = atoi ( arg3 );

    /*
     * Set something.
     */

     if ( !str_cmp ( arg2, "condition" ) )
     {
          set_obj_cond ( obj, ( URANGE ( 1, value, 100 ) ) );
          send_to_char ( "Condition set.\n\r", ch );
          return;
     }
     if ( !str_cmp ( arg2, "value0" ) || !str_cmp ( arg2, "v0" ) )
     {
          obj->value[0] = UMIN ( 1000, value );
          obj->valueorig[0] = UMIN ( 1000, value );
          return;
     }
     if ( !str_cmp ( arg2, "value1" ) || !str_cmp ( arg2, "v1" ) )
     {
          obj->value[1] = value;
          obj->valueorig[1] = value;
          return;
     }
     if ( !str_cmp ( arg2, "value2" ) || !str_cmp ( arg2, "v2" ) )
     {
          obj->value[2] = value;
          obj->valueorig[2] = value;
          return;
     }
     if ( !str_cmp ( arg2, "value3" ) || !str_cmp ( arg2, "v3" ) )
     {
          obj->value[3] = value;
          obj->valueorig[3] = value;
          return;
     }
     if ( !str_cmp ( arg2, "value4" ) || !str_cmp ( arg2, "v4" ) )
     {
          obj->value[4] = value;
          obj->valueorig[4] = value;
          return;
     }
     if ( !str_prefix ( arg2, "extra" ) )
     {
          obj->extra_flags = value;
          return;
     }
     if ( !str_prefix ( arg2, "wear" ) )
     {
          obj->wear_flags = value;
          return;
     }
     if ( !str_prefix ( arg2, "level" ) )
     {
          obj->level = value;
          return;
     }
     if ( !str_prefix ( arg2, "weight" ) )
     {
          obj->weight = value;
          return;
     }

     if ( !str_prefix ( arg2, "cost" ) )
     {
          obj->cost = value;
          return;
     }

     if ( !str_prefix ( arg2, "timer" ) )
     {
          obj->timer = value;
          return;
     }

     if ( !str_prefix ( arg2, "size" ) )
     {
          if ( value < 0 || value > 5 )
          {
               send_to_char ( "Size range is 0-5.\n\r", ch );
               return;
          }
          obj->size = value;
          form_to_char ( ch, "Size set to %s.\n\r", size_table[value] );
          return;
     }

    /*
     * Generate usage message.
     */
     do_oset ( ch, "" );
     return;
}

void do_rset ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                arg3[MAX_INPUT_LENGTH];
     ROOM_INDEX_DATA    *location;
     int                 value;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     SLCPY ( arg3, argument );

     if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  set room <location> <field> <value>\n\r", ch );
          send_to_char ( "  Field being one of:\n\r", ch );
          send_to_char ( "    flags sector\n\r", ch );
          return;
     }

     if ( ( location = find_location ( ch, arg1 ) ) == NULL )
     {
          send_to_char ( "No such location.\n\r", ch );
          return;
     }

    /*
     * Snarf the value.
     */
     if ( !is_number ( arg3 ) )
     {
          send_to_char ( "Value must be numeric.\n\r", ch );
          return;
     }
     value = atoi ( arg3 );

    /*
     * Set something.
     */
     if ( !str_prefix ( arg2, "flags" ) )
     {
          location->room_flags = value;
          return;
     }

     if ( !str_prefix ( arg2, "sector" ) )
     {
          location->sector_type = value;
          return;
     }

    /*
     * Generate usage message.
     */
     do_rset ( ch, "" );
     return;
}

void do_sockets ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA	       *vch;
     BUFFER	       *buffer;
     char	       *st;
     char		s[100];
     char		idle[10];
     DESCRIPTOR_DATA   *d;
     int		count;

     buffer = buffer_new(1000);

     count = 0;

     bprintf( buffer, "\n\r[Num Connected_State Login@ Idl] Player Name Host\n\r" );
     bprintf( buffer, "--------------------------------------------------------------------------\n\r");

     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          // Modified to show descriptors without characters to IMP
          if ( ( d->character != NULL && can_see ( ch, d->character ) )
               || ( get_trust ( ch ) == MAX_LEVEL ) )
          {
               count++;
               if ( d->character == NULL )
               {
                    bprintf ( buffer, "[%3d %s] %s\n\r",
                              d->descriptor, "- Empty Socket-", d->host );
               }
               else
               {
                    switch( d->connected )
                    {
                    case CON_PLAYING:              st = "    PLAYING    ";    break;
                    case CON_GET_NAME:             st = "   Get Name    ";    break;
                    case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
                    case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
                    case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
                    case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
                    case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
                    case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
                    case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
                    case CON_ROLL_STATS:	   st = " Chosing Stats ";    break;
                    case CON_FIND_MORTALS:	   st = "Choosing M/Demi";    break;
                    case CON_GET_ALIGNMENT:        st = " Get New Align ";    break;
                    case CON_READ_IMOTD:   	   st = " Reading IMOTD ";    break;
                    case CON_BREAK_CONNECT:        st = "   LINKDEAD    ";    break;
                    case CON_READ_MOTD:            st = "  Reading MOTD ";    break;
                    case CON_ACCOUNT_PW_NEW:
                    case CON_ACCOUNT_PW_CONFIRM:   st = "New Acct Passwd";    break;
                    case CON_ACCOUNT_PW_FIX_VERIFY:
                    case CON_ACCOUNT_PW_FIX:       st = "Fix Acct Passwd";    break;
                    case CON_PW_FIX_CONFIRM:
                    case CON_PW_FIX:	           st = "Fixing Password";    break;
                    case CON_LOG_ACCOUNT:	   st = "Logging Account";    break;
                    case CON_ACCOUNT_MENU:	   st = " Account Menu  ";    break;
                    case CON_EDIT_CLAN:            st = "   Edit Clan   ";    break;
                    case CON_NOTE_TO:
                    case CON_NOTE_SUBJECT:
                    case CON_NOTE_EXPIRE:
                    case CON_NOTE_TEXT:
                    case CON_NOTE_FINISH:	   st = "Writing a Note ";    break;
                    default:                       st = "    !BUGGY!    ";    break;
                    }

                    /* Format "login" value... */
                    vch = d->original ? d->original : d->character;
                    strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

                    if ( vch->timer > 0 )
                         SNP( idle, "%-2d", vch->timer );
                    else
                         SNP( idle, "  " );
                    
                    bprintf( buffer, "[%3d %s %7s %2s] %-12s %-32.32s {R%s{w\n\r",
                             d->descriptor, st, s, idle,
                             ( d->original ) ? d->original->name
                             : ( d->character )  ? d->character->name
                             : "(None!)", d->host,
                             ( get_trust(vch) > vch->level) ? itos(get_trust(vch)) : "" );
               }

          }
     }

     bprintf ( buffer, "%d user%s\n\r", count,
               count == 1 ? "" : "s" );
     page_to_char ( buffer->data, ch );
     buffer_free(buffer);

     return;
}

void do_force ( CHAR_DATA * ch, char *argument )
{
     char		buf[MSL];
     char               arg[MAX_INPUT_LENGTH];
     char               arg2[MAX_INPUT_LENGTH];

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "Force whom to do what?\n\r", ch );
          return;
     }

     one_argument ( argument, arg2 );

     if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
     {
          send_to_char ( "That will NOT be done.\n\r", ch );
          return;
     }

     SNP ( buf, "$n forces you to '%s'.", argument );

     if ( !str_cmp ( arg, "all" ) )
     {
          CHAR_DATA          *vch;
          CHAR_DATA          *vch_next;

          if ( get_trust ( ch ) < MAX_LEVEL - 3 )
          {
               send_to_char ( "Not at your level!\n\r", ch );
               return;
          }

          for ( vch = char_list; vch != NULL; vch = vch_next )
          {

               if ( !IS_NPC ( vch ) && get_trust ( vch ) < get_trust ( ch ) )
               {
                    act ( buf, ch, NULL, vch, TO_VICT );
                    interpret ( vch, argument );
               }
               vch_next = vch->next;
          }
     }
     else if ( !str_cmp ( arg, "players" ) )
     {
          CHAR_DATA          *vch;
          CHAR_DATA          *vch_next;

          if ( get_trust ( ch ) < MAX_LEVEL - 2 )
          {
               send_to_char ( "Not at your level!\n\r", ch );
               return;
          }

          for ( vch = char_list; vch != NULL; vch = vch_next )
          {

               if ( !IS_NPC ( vch ) &&
                    get_trust ( vch ) < get_trust ( ch ) &&
                    vch->level < LEVEL_HERO )
               {
                    act ( buf, ch, NULL, vch, TO_VICT );
                    interpret ( vch, argument );
               }
               vch_next = vch->next;
          }
     }
     else if ( !str_cmp ( arg, "gods" ) )
     {
          CHAR_DATA          *vch;
          CHAR_DATA          *vch_next;

          if ( get_trust ( ch ) < MAX_LEVEL - 2 )
          {
               send_to_char ( "Not at your level!\n\r", ch );
               return;
          }

          for ( vch = char_list; vch != NULL; vch = vch_next )
          {

               if ( !IS_NPC ( vch ) && get_trust ( vch ) < get_trust ( ch ) && vch->level >= LEVEL_HERO )
               {
                    act ( buf, ch, NULL, vch, TO_VICT );
                    interpret ( vch, argument );
               }
               vch_next = vch->next;
          }
     }
     else
     {
          CHAR_DATA          *victim;

          if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
          {
               send_to_char ( "They aren't here.\n\r", ch );
               return;
          }

          if ( victim == ch )
          {
               send_to_char ( "Aye aye, right away!\n\r", ch );
               return;
          }

          if ( get_trust ( victim ) >= get_trust ( ch ) )
          {
               send_to_char ( "Do it yourself!\n\r", ch );
               return;
          }

          if ( !IS_NPC ( victim ) &&
               get_trust ( ch ) < MAX_LEVEL - 3 )
          {
               send_to_char ( "Not at your level!\n\r", ch );
               return;
          }

          act ( buf, ch, NULL, victim, TO_VICT );
          interpret ( victim, argument );
     }

     send_to_char ( "Ok.\n\r", ch );
     return;
}

/*
 * New routines by Dionysos.
 */
void do_invis ( CHAR_DATA * ch, char *argument )
{
     int                 level;
     char                arg[MAX_STRING_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

    /* RT code for taking a level argument */
     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
	/* take the default path */

          if ( IS_SET ( ch->act, PLR_WIZINVIS ) )
          {
               REMOVE_BIT ( ch->act, PLR_WIZINVIS );
               ch->invis_level = 0;
               act ( "$n slowly fades into existence.", ch, NULL,  NULL, TO_ROOM );
               send_to_char ( "You slowly fade back into existence.\n\r", ch );
          }
     else
     {
          SET_BIT ( ch->act, PLR_WIZINVIS );
          ch->invis_level = get_trust ( ch );
          act ( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char ( "You slowly vanish into thin air.\n\r", ch );
     }
     else
	/* do the level thing */
     {
          level = atoi ( arg );
          if ( level < 2 || level > get_trust ( ch ) )
          {
               send_to_char ( "Invis level must be between 2 and your level.\n\r", ch );
               return;
          }
          else
          {
               ch->reply = NULL;
               SET_BIT ( ch->act, PLR_WIZINVIS );
               ch->invis_level = level;
               act ( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
               send_to_char ( "You slowly vanish into thin air.\n\r", ch );
          }
     }
     return;
}

void do_cloak ( CHAR_DATA * ch, char *argument )
{
     int                 level;
     char                arg[MAX_STRING_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

    /* RT code for taking a level argument */
     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
	/* take the default path */

          if ( IS_SET ( ch->act, PLR_CLOAK ) )
          {
               REMOVE_BIT ( ch->act, PLR_CLOAK );
               ch->cloak_level = 0;
               act ( "$n's immortal cloak vanishes.", ch, NULL, NULL, TO_ROOM );
               send_to_char ( "You dismiss your mystical cloak.\n\r", ch );
          }
     else
     {
          SET_BIT ( ch->act, PLR_CLOAK );
          ch->cloak_level = get_trust ( ch );
          act ( "$n calls forth a mystic cloak to hide $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char ( "You call forth a mystic cloak to hide your presence.\n\r", ch );
     }
     else
	/* do the level thing */
     {
          level = atoi ( arg );
          if ( level < 2 || level > get_trust ( ch ) )
          {
               send_to_char ( "Cloak level must be between 2 and your level.\n\r", ch );
               return;
          }
          else
          {
               ch->reply = NULL;
               SET_BIT ( ch->act, PLR_CLOAK );
               ch->cloak_level = level;
               act ( "$n calls forth a mystic cloak to hide $s presence.", ch, NULL, NULL, TO_ROOM );
               send_to_char ( "You call forth a mystic cloak to hide your presence.\n\r", ch );
          }
     }
     return;
}

void do_holylight ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( IS_SET ( ch->act, PLR_HOLYLIGHT ) )
     {
          REMOVE_BIT ( ch->act, PLR_HOLYLIGHT );
          send_to_char ( "Holy light mode off.\n\r", ch );
     }
     else
     {
          SET_BIT ( ch->act, PLR_HOLYLIGHT );
          send_to_char ( "Holy light mode on.\n\r", ch );
     }

     return;
}

// Some of the below checks for IS_IMMORTAL, however the function is not yet set up
// for use by mere mortals. It was thought at one point to make it available during
// character creation to list PC races only. Perhaps someday. For now it is an imm function
// only. Perhaps it would be best to make a seperate function for players since the
// output would be very different -- Lotherius.
//
void do_listraces ( CHAR_DATA * ch, char *argument )
{
     int i;
     BUFFER	*buffer;
     char	buf[MAX_STRING_LENGTH];
     int	race, pcrace;

     buf[0] = '\0';
     buffer = buffer_new(1000);

     if ( argument == NULL || argument[0] == '\0' )
     {
          for ( race = 0; race_table[race].name != NULL; race++ )
          {
               bprintf ( buffer, "{wName   : {G%s", race_table[race].name );
               if (race_table[race].pc_race)
                    bprintf (buffer, " {W[{YPC Race{W]" );
               bprintf (buffer, "{w\n\r");
          }
     }
     else if ( ( race = race_lookup ( argument ) ) != 0 )
     {
          bprintf ( buffer, "{wName   : {G%s", race_table[race].name );
          if (race_table[race].pc_race)
          {
               pcrace = race_lookup ( argument );
               bprintf ( buffer, " {W[{YPC Race{W]\n\r\n\r" );
               bprintf ( buffer, " {wPC's choosing this race will have the following attributes:\n\r\n\r" );
               bprintf ( buffer, " {WMaxAge      :{w %d\n\r", pc_race_table[pcrace].maxage );
               bprintf ( buffer, " {WXP Cost     :{w Mult {Y%d{w -- Mortals {Y%d{w, Demis {Y%d{w\n\r",
                         pc_race_table[pcrace].points,
                         1200 + (pc_race_table[pcrace].points * 100),
                         ( 7700 + ( ( 1200 + (pc_race_table[pcrace].points * 100 ) ) / 4 ) ) );
               bprintf ( buffer, " {WRace Recall :{w %d\n\r", pc_race_table[pcrace].recall );
               bprintf ( buffer, " {WRace Healer :{w %d\n\r", pc_race_table[pcrace].healer );
               bprintf ( buffer, " {WBase Stats  :{w " );
               for ( i = 0 ; i < MAX_STATS ; i++ )
                    bprintf ( buffer, "%d ", pc_race_table[pcrace].stats[i] );
               bprintf ( buffer, "\n\r {WMax Stats   :{w " );
               for ( i = 0 ; i < MAX_STATS ; i++ )
                    bprintf ( buffer, "%d ", pc_race_table[pcrace].max_stats[i] );
               bprintf ( buffer, "\n\r {WSize        :{w " );
               switch ( pc_race_table[pcrace].size )
               {
               case SIZE_TINY: 		bprintf ( buffer, "Tiny\n\r" ); 	break;
               case SIZE_SMALL: 	bprintf ( buffer, "Small\n\r" ); 	break;
               case SIZE_MEDIUM: 	bprintf ( buffer, "Medium\n\r" ); 	break;
               case SIZE_LARGE: 	bprintf ( buffer, "Large\n\r" ); 	break;
               case SIZE_HUGE: 		bprintf ( buffer, "Huge\n\r" ); 	break;
               case SIZE_GIANT: 	bprintf ( buffer, "Giant\n\r" ); 	break;
               case SIZE_UNKNOWN:
               default: 		bprintf ( buffer, "Unknown\n\r"); 	break;
               }
          }
          bprintf (buffer, "{w\n\r");
          if ( ( race_table[race].act > 0 ) && IS_IMMORTAL ( ch ) );
          bprintf ( buffer, "Act    : %s\n\r", flag_string ( act_flags, race_table[race].act ) );
          if ( race_table[race].aff > 0 )
               bprintf ( buffer, "Affect : %s\n\r", affect_bit_name ( race_table[race].aff ) );
          if ( race_table[race].detect > 0 )
               bprintf ( buffer, "Detect : %s\n\r", detect_bit_name ( race_table[race].detect ) );
          if ( race_table[race].protect > 0 )
               bprintf ( buffer, "Protect: %s\n\r", protect_bit_name ( race_table[race].protect ) );
          if ( race_table[race].off > 0 )
               bprintf ( buffer, "Offense: %s\n\r", flag_string ( off_flags, race_table[race].off ) );
          if ( race_table[race].imm > 0 )
               bprintf ( buffer, "Immune : %s\n\r", flag_string ( imm_flags, race_table[race].imm ) );
          if ( race_table[race].res > 0 )
               bprintf ( buffer, "Resist : %s\n\r", flag_string ( res_flags, race_table[race].res ) );
          if ( race_table[race].vuln > 0 )
               bprintf ( buffer, "Vulnrbl: %s\n\r", flag_string ( vuln_flags, race_table[race].vuln ) );
          if ( ( race_table[race].form > 0 ) && IS_IMMORTAL ( ch ) );
          bprintf ( buffer, "Form   : %s\n\r", flag_string ( form_flags, race_table[race].form ) );
          if ( ( race_table[race].parts > 0 ) && IS_IMMORTAL ( ch ) );
          bprintf ( buffer, "Parts  : %s\n\r", flag_string ( part_flags, race_table[race].parts ) );
          bprintf ( buffer, "Encumb : %d\n\r", race_table[race].encumbrance );
     }
     else
     {
          bprintf ( buffer, "For a complete list of races, enter listraces with no argument.\n\r" );
     }

     bprintf ( buffer, "\n\r" );
     page_to_char ( buffer->data, ch );

     buffer_free(buffer);
     return;
}

void do_listskills ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buffer;
     int                 sn, filter;

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  listskills all\n\r", ch );
          send_to_char ( "Syntax:  listskills unused\n\r", ch );
          send_to_char ( "         listskills <class>\n\r", ch );
          return;
     }

     buffer = buffer_new(1000);

     if ( !str_cmp ( argument, "unused" ) )
     {
          filter = -2;
     }
     else
          if ( !str_cmp ( argument, "all" ) )
          {
               filter = -1;
          }
     else
     {
          filter = class_lookup ( argument );
          if ( filter == -1 )
          {
               send_to_char ( "That's not a class.\n\r", ch );
               buffer_free(buffer);
               return;
          }
          else
          {
               bprintf ( buffer, "Filtering by: %d\n\r", filter );
          }
     }

     bprintf ( buffer, "Slot        {cClass Names {w: {GMag {WAve {GThi {WWar {GMon {WDef {GCha{x\n\r" );

    /* begin listing all skills */
     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( skill_table[sn].name == NULL )
               break;

          if ( filter >= 0 )
          {
               if ( skill_table[sn].skill_level[filter] >= 102 )
                    continue;
          }

          if ( filter == -2 )
          {
               if ( skill_table[sn].skill_level[0] <= 101 ||
                    skill_table[sn].skill_level[1] <= 101 ||
                    skill_table[sn].skill_level[2] <= 101 ||
                    skill_table[sn].skill_level[3] <= 101 ||
                    skill_table[sn].skill_level[4] <= 101 ||
                    skill_table[sn].skill_level[5] <= 101 ||
                    skill_table[sn].skill_level[6] <= 101 )
                    continue;
          }

          bprintf ( buffer,
                    "%3d: {c%-18s {w: {G%3s {W%3s {G%3s {W%3s {G%3s {W%3s {G%3s{x\n\r",
                    skill_table[sn].slot, skill_table[sn].name,
                    ( skill_table[sn].skill_level[0] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[0] ),
                    ( skill_table[sn].skill_level[1] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[1] ),
                    ( skill_table[sn].skill_level[2] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[2] ),
                    ( skill_table[sn].skill_level[3] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[3] ),
                    ( skill_table[sn].skill_level[4] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[4] ),
                    ( skill_table[sn].skill_level[5] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[5] ),
                    ( skill_table[sn].skill_level[6] >= 102 ) ? "--" : itos ( skill_table[sn].skill_level[6] ) );
	/* increase for new classes */
     }
     bprintf ( buffer, "\n\r" );
     page_to_char ( buffer->data, ch );

     buffer_free(buffer);
     return;
}

/* Damn hardcoded levels... smash 'em all to macros! */
void do_world ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     BUFFER             *buffer;
     OBJ_INDEX_DATA     *obj;
     MOB_INDEX_DATA     *mob;
     bool                check_mob = FALSE;
     int                 level[MAX_MOB_LEVEL+2];
     AREA_DATA          *pArea;
     int                 counter;

     /* Counter must match index number in declaration of int level[#] above */
     for ( counter = 0; counter <= MAX_MOB_LEVEL ; counter++ )
          level[counter] = 0;

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  world <obj | mob>\n\r", ch );
          return;
     }

     one_argument ( argument, arg );

     if ( !str_cmp ( arg, "mob" ) )
          check_mob = TRUE;
     else if ( !str_cmp ( arg, "obj" ) )
          check_mob = FALSE;
     else
     {
          send_to_char ( "Syntax:  world <obj | mob>\n\r", ch );
          return;
     }

     for ( pArea = area_first; pArea; pArea = pArea->next )
     {
          for ( counter = pArea->lvnum; counter <= pArea->uvnum; counter++ )
          {
               if ( check_mob )
               {
                    if ( ( mob = get_mob_index ( counter ) ) == NULL )
                         continue;
                    if ( mob->level > MAX_MOB_LEVEL )
                    {
                         bugf ( "Mob > MAX_MOB_LEVEL, VNUM %d", mob->vnum );
                         level[MAX_MOB_LEVEL]++;
                    }
                    else
                         level[mob->level]++;
               }
               else
               {
                    if ( ( obj = get_obj_index ( counter ) ) == NULL )
                         continue;
                    if ( obj->level > MAX_OBJ_LEVEL )
                    {
                         bugf ( "Obj > MAX_OBJ_LEVEL, VNUM %d", obj->vnum );
                         level[MAX_OBJ_LEVEL]++;
                    }
                    else
                         level[obj->level]++;
               }
          }
          /* end for counter loop */
     }
     /* end for pArea loop */

     buffer = buffer_new ( 4096 );
     /*generate output */
     if ( check_mob )
     {
          bprintf ( buffer, "MOBILES [{cL<level>  {m<vnum count>{x]\n\r" );
          for ( counter = 1; counter <= MAX_MOB_LEVEL -1; counter++ )
          {
               bprintf ( buffer, "[{cL%-3d  {m%4d{x]   ", counter, level[counter] );
               if ( counter % 4 == 0 )
                    bprintf ( buffer, "\n\r" );
          }
          bprintf ( buffer, "[{cL%d+ {m%4d{x]\n\r", MAX_MOB_LEVEL, level[MAX_MOB_LEVEL] );
     }
     else
     {
          bprintf ( buffer, "OBJECTS [{cL<level>  {m<vnum count>{x]\n\r" );
          for ( counter = 1; counter <= MAX_OBJ_LEVEL -1; counter++ )
          {
               bprintf ( buffer, "[{cL%-3d  {m%4d{x]   ", counter, level[counter] );
               if ( counter % 4 == 0 )
                    bprintf ( buffer, "\n\r" );
          }
          bprintf ( buffer, "[{cL%d+ {m%4d{x]\n\r", MAX_OBJ_LEVEL, level[MAX_OBJ_LEVEL] );
     }
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

void do_award ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char               *arg2;
     CHAR_DATA          *victim;
     int                 xp_modifier;

     if ( !ch->desc )		/* no switched IMMs! */
     {
          send_to_char ( "Switched immortals cannot use this command.\n\r", ch );
          return;
     }

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  award <player> <xp>\n\r", ch );
          return;
     }

     arg2 = one_argument ( argument, arg1 );

     if ( arg2[0] == '\0' )
     {
          send_to_char ( "Syntax:  <award | penalty> <player> <xp>\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg1 ) ) == NULL )
     {
          send_to_char ( "That player is not in this room.\n\r", ch );
          return;
     }

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "You cannot reward mobiles.\n\r", ch );
          return;
     }

     if ( IS_IMMORTAL ( victim ) )
     {
          send_to_char ( "You cannot reward immortals.\n\r", ch );
          return;
     }

     xp_modifier = atoi ( arg2 );

     if ( xp_modifier == 0 )
     {
          send_to_char( "<xp> argument must be a non-zero value between -10000 and 10000.\n\r",  ch );
          return;
     }

     if ( xp_modifier > 10000 || xp_modifier < -10000 )	/*watch for outrageous amount */
     {
          send_to_char ( "<xp> value must be between -10000 and 10000, excluding 0.\n\r", ch );
          return;
     }

     if ( xp_modifier < 0 )
     {
          form_to_char ( victim, "%s has penalized you %d experience points!\n\r",
                         ( can_see ( victim, ch ) ? ch->name : "Someone" ), xp_modifier );
          form_to_char ( ch, "You penalize %s %d experience points!\n\r",
                         victim->name, xp_modifier );
     }
     else
     {
          form_to_char ( victim, "%s has awarded you %d experience points!\n\r",
                         ( can_see ( victim, ch ) ? ch->name : "Someone" ), xp_modifier );
          form_to_char ( ch, "You award %s %d experience points!\n\r",
                         victim->name, xp_modifier );
     }
     gain_exp ( victim, xp_modifier );
     return;
}

void do_repop ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     ROOM_INDEX_DATA    *rptr = NULL;

     one_argument ( argument, arg );

     if ( arg == NULL || arg[0] == '\0' )
     {
          rptr = ch->in_room;
          SNP ( arg, "%d", ch->in_room->vnum );
     }
     else
     {
          if ( is_number ( arg ) )
          {
               rptr = get_room_index ( atoi ( arg ) );
          }
          else
          {
               send_to_char ( "Syntax: repop [vnum]\n\rBlank defaults to current room.\n\r", ch );
               return;
          }
     }

     if ( rptr == NULL )
     {
          send_to_char ( "That room vnum doesn't exist.\n\r", ch );
          return;
     }
     /* Ok, repop the room */
     reset_room ( rptr, TRUE );	/* Force reset */
     form_to_char ( ch, "Room %s has been forced to repop.\n\r", arg );
}

void do_statall ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "Not Yet Complete.\n\r", ch );
     return;
}

void do_crier ( CHAR_DATA * ch, char *argument )
{
     struct     crier_type  *tmp, *last_cry = NULL;
     BUFFER	*outbuf;
     bool       match = FALSE;
     int        count = 0;
     int        i = 0;

     if ( argument == NULL || argument[0] == '\0' )
     {
          outbuf = buffer_new(1000);
          send_to_char ( "## Crier Message\n\r", ch );
          send_to_char ( "-- -------------\n\r", ch );
          for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
          {
               bprintf ( outbuf, "{Y%2d {w%s\r\n", count, tmp->text );
               count++;
          }
          page_to_char(outbuf->data, ch);
          buffer_free(outbuf);
          return;
     }
     else if ( is_number (argument ) ) /* A number alone deletes the cry at that number */
     {
          i = atoi ( argument );
		  /* Count up to I */
          for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
          {
               if ( count == i )
               {
                    match = TRUE;
                    break;
               }
               else
               {
                    count++;
                    last_cry = tmp;
               }
          }
          if ( !match)
          {
               send_to_char ( "No count found at that number.\n\r", ch );
               return;
          }
		  /* Okay we have a live one. */

          last_cry->next = tmp->next;
          tmp->next = NULL;
          free_string ( tmp->text );
          free_mem ( tmp, sizeof ( struct crier_type ), "crier_type" );
          send_to_char ( "Cry deleted.\n\r", ch );
     }
     else
     {
          /* Added Text String */
          /* Wind to the end of the list, check for dupes */
          for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
          {
               count++;
               last_cry = tmp;
          }

          tmp = alloc_mem ( sizeof ( struct crier_type ), "crier_type" );

          tmp->next = NULL;
          tmp->text = str_dup ( argument );

          if ( crier_list == NULL )
               crier_list = tmp;
          else
               last_cry->next = tmp;
          form_to_char ( ch, "Added cry, now total count: %d\r\n", count );
     }
     fwrite_crier (  );
     return;
}

void do_accounts ( CHAR_DATA * ch, char *argument )
{
     struct account_type *tmp;
     BUFFER              *outbuf;
     int		  i = 0;
     int                  count = 0;

     /* Assume your own account first */
     /* Mortals can only access their own account */

     if ( argument == NULL || argument[0] == '\0' || !IS_IMMORTAL (ch))
     {
          outbuf = buffer_new(1024);

          bprintf ( outbuf, "\n\rAccount Info For: {W%s{w\n\r\n\r", ch->pcdata->account->acc_name );

          switch ( ch->pcdata->account->status )
          {
          case ACCT_REJECTED_EMAIL:
          case ACCT_REJECTED_RULES:
          case ACCT_REJECTED_OTHER:
               bprintf (outbuf, "  {RYou've been REJECTED!!!{w\n\r" );
               break;
          case ACCT_UNVERIFIED:
               bprintf (outbuf, "  {RYou are UNVERIFIED.{w\n\r" );
               break;
          case ACCT_VERIFIED:
               bprintf (outbuf, "  You are {GVerified{w but cannot make Demi-God characters.\n\r" );
               break;
          case ACCT_VERIFIED_DEMISTAT:
               bprintf (outbuf, "  You are {GVerified{w and may create {YDemi-Gods{w.\n\r");
               break;
          case ACCT_HELPER:
               bprintf (outbuf, "  You are a {CHelper{w.\n\r");
               break;
          case ACCT_STAFF:
               bprintf (outbuf, "  You're {MStaff{w!\n\r");
               break;
          case ACCT_IMPLEMENTOR:
               bprintf (outbuf, "  Do you really need to ask? You're the implementor.\n\r");
               break;
          default:
               bprintf (outbuf, "  {RUmm... Something's wrong. BUGOLA!{w\n\r");
               break;
          }

          bprintf ( outbuf, "{C%d{w of your Mortals have died permanently.\n\r", ch->pcdata->account->permadead );
          bprintf ( outbuf, "You have {C%d{w Mortal Heroes.\n\r", ch->pcdata->account->heroes );
          if ( ch->pcdata->account->status >= ACCT_VERIFIED_DEMISTAT )
          {
               bprintf ( outbuf, "You have created {C%d{w Demi-Gods.\n\r", ch->pcdata->account->demigods );
          }
          else if ( ch->pcdata->account->status >= ACCT_VERIFIED )
          {
               bprintf ( outbuf, "You must have {C%d{w Mortal Heroes before you may create a Demi-God.\n\r",
                         mud.fordemi );
          }
          else
          {
               bprintf ( outbuf, "You have not verified your account, and cannot create Demi-Gods.\n\r" );
          }

          bprintf ( outbuf, "\n\rYou have the following characters:\n\r" );

          for (i = 0 ; i < MAX_CHARS ; i++)
          {
               if (ch->pcdata->account->char_name[i])
               {
                    bprintf ( outbuf, "       {B%d: {W%s{w\n\r", i, ch->pcdata->account->char_name[i] );
               }

          }

          page_to_char(outbuf->data, ch);
          buffer_free(outbuf);
          return;

     }
     else if ( !strcmp (argument, "all" ) );
     {
          outbuf = buffer_new(4096);
          send_to_char ( "#### Status           PD  MH  DG\n\r", ch );
          send_to_char ( "---- ---------------- --  --  --\n\r", ch );

          for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
          {
               bprintf ( outbuf, "{w%4d {Y", count );

               switch (tmp->status)
               {
               case ACCT_REJECTED_DELETE:
                    bprintf ( outbuf, "({RDeleted{Y       )");
                    break;
               case ACCT_REJECTED_EMAIL:
                    bprintf ( outbuf, "(Rejected Email)");
                    break;
               case ACCT_REJECTED_RULES:
                    bprintf ( outbuf, "(Rejected Rules)");
                    break;
               case ACCT_REJECTED_OTHER:
                    bprintf ( outbuf, "(Rejected Other)");
                    break;
               case ACCT_CREATED:
                    bprintf ( outbuf, "(Created       )");
                    break;
               case ACCT_UNVERIFIED:
                    bprintf ( outbuf, "(Unverified    )");
                    break;
               case ACCT_VERIFIED:
                    bprintf ( outbuf, "(Verified      )");
                    break;
               case ACCT_VERIFIED_DEMISTAT:
                    bprintf ( outbuf, "(Demi Status   )");
                    break;
               case ACCT_HELPER:
                    bprintf ( outbuf, "(Helper        )");
                    break;
               case ACCT_STAFF:
                    bprintf ( outbuf, "(Staff         )");
                    break;
               case ACCT_IMPLEMENTOR:
                    bprintf ( outbuf, "(Big Cheese    )");
                    break;
               default:
                    bprintf ( outbuf, "({RBuggy         {Y)");
                    break;
               }

               bprintf ( outbuf, " {G%d : %d : %d : {W%s{w\n\r",
                         tmp->permadead, tmp->heroes, tmp->demigods, tmp->acc_name );

               for (i = 0 ; i < MAX_CHARS ; i++)
               {
                    if (tmp->char_name[i])
                    {
                         bprintf ( outbuf, "                                  {B%d: {C%s{w\n\r", i, tmp->char_name[i] );
                    }

               }

               count++;
          }
          page_to_char(outbuf->data, ch);
          buffer_free(outbuf);
          return;
     }

     send_to_char ("Invalid Option (by name not supported yet).\n\r", ch );
     return;

}

void do_reject (CHAR_DATA *ch, char *argument)
{
     struct account_type       *tmp = NULL;
     CHAR_DATA 		       *victim;
     char 			arg[MAX_INPUT_LENGTH];
     int 			i;
     bool 			acc_found = FALSE;

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' || argument[0] == '\0' )
     {
          send_to_char ( "Syntax: reject <account> <status>\n\r", ch );
          send_to_char ( "Where status can be:\n\r  delete, email, rules, other, unverify\n\r", ch );
          return;
     }

     for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
     {
          if (!str_cmp (arg, tmp->acc_name) )
          {
               acc_found = TRUE;
               break;
          }
     }

     if (!acc_found)
     {
          send_to_char ("Couldn't find any accounts by that name.\n\r", ch);
          return;
     }

	/* Will be deleted!! */
     if ( !str_prefix (argument, "delete" ) )
     {
          tmp->status = ACCT_REJECTED_DELETE;
          send_to_char ("Account set to be deleted. All Characters will be deleted instantly.\n\r", ch);
          send_to_char ("Account will be cleared from database at next reboot.\n\r", ch);

          for (i = 0 ; i < MAX_CHARS ; i++)
          {
               if (tmp->char_name[i])
               {	/* IF char is online, bump him. */
                    if ( ( victim = get_char_world ( ch, tmp->char_name[i] ) ) != NULL )
                    {
                         send_to_char ("Woops! You're being deleted!!!\n\rGoodbye!\n\r", victim);
                         do_fastquit (victim);
                    }
                    offline_delete ( tmp->char_name[i] );
               }
          }
          fwrite_accounts();
          return;
     }

     if ( !str_prefix (argument, "email" ) )
     {
          tmp->status = ACCT_REJECTED_EMAIL;
          send_to_char ("Account rejected for EMAIL violation.\n\r", ch);
          fwrite_accounts();
          return;
     }

     if ( !str_prefix (argument, "rules" ) )
     {
          tmp->status = ACCT_REJECTED_RULES;
          send_to_char ("Account rejected for RULES violation.\n\r", ch);
          fwrite_accounts();
          return;
     }

     if ( !str_prefix (argument, "other" ) )
     {
          tmp->status = ACCT_REJECTED_OTHER;
          send_to_char ("Account rejected for an unspecified violation.\n\r", ch);
          fwrite_accounts();
          return;
     }

     if ( !str_prefix (argument, "unverify" ) )
     {
          tmp->status = ACCT_UNVERIFIED;
          send_to_char ("Account Status reset to UNVERIFIED.\n\r", ch);
          fwrite_accounts();
          return;
     }
     /* Sloppily repeat the command to resend the rules. */
     do_reject ( ch, "" );
     return;

}

/*
 * List exits in an area
 * Params: from  - Exits leaving the area
 *         void  - Exits leaving to the void
 *         funky - funky exits
 */
void do_areaexits ( CHAR_DATA * ch, char *argument )
{
     BUFFER             *buffer;
     AREA_DATA          *pArea;
     ROOM_INDEX_DATA    *tmp;
     EXIT_DATA          *pexit;
     int                 door, counter;
     bool                ifrom = FALSE;
     bool                ivoid = FALSE;
     bool                ifunky = FALSE;

     if ( IS_NPC ( ch ) )
          return;

     if ( !str_cmp ( argument, "from" ) )
          ifrom = TRUE;
     else if ( !str_cmp ( argument, "void" ) )
          ivoid = TRUE;
     else if ( !str_cmp ( argument, "funky" ) )
          ifunky = TRUE;
     else
     {
          send_to_char ( "Syntax: areaexits from\n\r"
                         "        areaexits void\n\r"
                         "        areaexits funky\n\r", ch );
          return;
     }
     buffer = buffer_new ( 1024 );
     pArea = ch->in_room->area;
     /* Funky exits seeker could use more intelligence for different types of funky exits.... */
     if ( ifunky )
     {
          extern const sh_int rev_dir[];
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit_rev;
          int                 iHash;

          bprintf ( buffer, "Funky Exits ( Global ): " );

          for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
          {
               for ( tmp = room_index_hash[iHash]; tmp != NULL; tmp = tmp->next )
               {
                    for ( door = 0; door <= 5; door++ )
                    {
                         if ( ( pexit = tmp->exit[door] ) != NULL
                              && ( to_room = pexit->u1.to_room ) != NULL
                              && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
                              && pexit_rev->u1.to_room != tmp )
                         {
                              bprintf ( buffer, "Loopy Exit: %d:%d -> %d:%d -> %d. (%s)\n\r",
                                        tmp->vnum, door, to_room->vnum, rev_dir[door],
                                        ( pexit_rev->u1.to_room == NULL )
                                        ? 0 : pexit_rev->u1.to_room->vnum,
                                        tmp->area->name );
                         }
                    }
               }
          }
     }
     else if ( ifrom || ivoid )
     {
          bprintf ( buffer, "Exits For: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );
          counter = pArea->lvnum;
          for ( ; ( counter <= pArea->uvnum ); counter++ )
          {
               if ( ( tmp = get_room_index ( counter ) ) != NULL )
               {
                    for ( door = 0; door <= 5; door++ )
                    {
                         if ( ( pexit = tmp->exit[door] ) != NULL )
                         {
                              if ( ifrom )
                              {
                                   if ( pexit->u1.to_room->area != pArea )
                                        bprintf ( buffer, "Door %d : Leaves from Room %d to Room %d in %s.\n\r",
                                                  door,
                                                  tmp->vnum, pexit->u1.to_room->vnum,
                                                  pexit->u1.to_room->area->name );
                              }
                              else if ( ivoid )
                              {
                                   if ( pexit->u1.to_room->vnum < 100 )
                                        bprintf ( buffer, "Door %d : Leaves from Room %d to Room %d in %s.\n\r",
                                                  door,
                                                  tmp->vnum, pexit->u1.to_room->vnum,
                                                  pexit->u1.to_room->area->name );
                              }
                         }
                    }
               }
          }
     }
     page_to_char ( buffer->data, ch );
     return;
}

