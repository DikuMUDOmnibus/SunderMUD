 /***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
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
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN ( do_exits );
DECLARE_DO_FUN ( do_look );
DECLARE_DO_FUN ( do_help );
DECLARE_DO_FUN ( do_scan );
DECLARE_DO_FUN ( do_copy );
DECLARE_DO_FUN ( do_darkcolors );
DECLARE_DO_FUN ( do_nodarkgrey );
DECLARE_DO_FUN ( do_noflashy   );
DECLARE_DO_FUN ( do_colour     );
DECLARE_DO_FUN ( do_cursor     );
DECLARE_DO_FUN ( do_bank       );
DECLARE_DO_FUN ( do_borrow     );

struct who_slot
{
     CHAR_DATA          *ch;
     struct who_slot    *next;
};

// With two hands only, you can:
// wield & hold, wield and dual, hold & hold, but not wield/hold/light wield/dual/light, etc...
//

/*
 * Local functions.
 */
char    *format_obj_to_char    args ( ( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort ) );
void     show_list_to_char     args ( ( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing ) );
void     show_char_to_char_0   args ( ( CHAR_DATA * victim, CHAR_DATA * ch, bool LongLook ) );
void     show_char_to_char_1   args ( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void     show_char_to_char     args ( ( CHAR_DATA * list, CHAR_DATA * ch, bool LongLook ) );
bool     check_blind           args ( ( CHAR_DATA * ch ) );
void     real_consider 	       args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

char *format_obj_to_char ( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
     static char         buf[MAX_STRING_LENGTH];

     buf[0] = '\0';
     if ( IS_OBJ_STAT ( obj, ITEM_INVIS ) )
          SLCAT ( buf, "{x({BInvis{x) " );
     if ( CAN_DETECT ( ch, DET_EVIL )
          && IS_OBJ_STAT ( obj, ITEM_EVIL ) )
          SLCAT ( buf, "{x({rRed Aura{x) " );
     if ( is_affected ( ch, skill_lookup ( "detect good" ) )
          && IS_OBJ_STAT ( obj, ITEM_BLESS ) )
          SLCAT ( buf, "{x({gGreen Aura{x) " );
     if ( CAN_DETECT ( ch, DET_MAGIC )
          && IS_OBJ_STAT ( obj, ITEM_MAGIC ) )
          SLCAT ( buf, "{x({MMagical{x) " );
     if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) )
          SLCAT ( buf, "{x({YGlowing{x) " );
     if ( IS_OBJ_STAT ( obj, ITEM_HUM ) )
          SLCAT ( buf, "{x({YHumming{x) " );

     if ( fShort )
     {
          if ( obj->short_descr != NULL )
          {
               SLCAT ( buf, obj->short_descr );
               SLCAT ( buf, " {c({w" );
               SLCAT ( buf, obj_cond ( obj ) );
               SLCAT ( buf, "{c){w" );
          }
     }
     else
     {
          if ( obj->description != NULL )
               SLCAT ( buf, obj->description );
     }

     if ( strlen ( buf ) <= 0 )
          SLCAT ( buf, "This object has no description. Please inform the IMP." );
     return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char ( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing )
{

     BUFFER		*outbuf;
     char              **prgpstrShow;
     int                *prgnShow;
     char               *pstrShow;
     OBJ_DATA           *obj;
     int                 nShow;
     int                 iShow;
     int                 count;
     bool                fCombine;

     if ( ch->desc == NULL )
          return;
     outbuf = buffer_new(1000);
     /*
      * Alloc space for output lines.
      */

     count = 0;
     for ( obj = list; obj != NULL; obj = obj->next_content ) count++;
     prgpstrShow = alloc_mem ( count * sizeof ( char * ), "show_list_to_char" );
     prgnShow = alloc_mem ( count * sizeof ( int ), "show_list_to_char" );
     nShow = 0;

	 /*
	  * Format the list of objects.
	  */
     for ( obj = list; obj != NULL; obj = obj->next_content )
     {
          if ( obj->wear_loc == WEAR_NONE
               && can_see_obj ( ch, obj )
               && !IS_SET ( obj->extra_flags, ITEM_NODISP ) )
          {
               pstrShow = format_obj_to_char ( obj, ch, fShort );
               fCombine = FALSE;

               if ( IS_NPC ( ch ) || IS_SET ( ch->comm, COMM_COMBINE ) )
               {   /* Look for duplicates, case sensitive.
                    * Matches tend to be near end so run loop backwords.
                    */
                    for ( iShow = nShow - 1; iShow >= 0; iShow-- )
                    {
                         if ( !strcmp ( prgpstrShow[iShow], pstrShow ) )
                         {
                              prgnShow[iShow]++;
                              fCombine = TRUE;
                              break;
                         }
                    }
               }
               /*
                * Couldn't combine, or didn't want to.
                */
               if ( !fCombine )
               {
                    prgpstrShow[nShow] = str_dup ( pstrShow );
                    prgnShow[nShow] = 1;
                    nShow++;
               }
          }
     }

	 /*
	  * Output the formatted list.
	  */
     for ( iShow = 0; iShow < nShow; iShow++ )
     {

          if (prgpstrShow[iShow][0] == '\0')
          {
               free_string(prgpstrShow[iShow]);
               continue;
          }

          if ( IS_NPC ( ch ) || IS_SET ( ch->comm, COMM_COMBINE ) )
          {
               if ( prgnShow[iShow] != 1 )
               {
                    bprintf ( outbuf, "(%2d) ", prgnShow[iShow] );
               }
               else
               {
                    bprintf ( outbuf, "     " );
               }
          }
          bprintf (outbuf, prgpstrShow[iShow]);
          bprintf (outbuf, "\n\r" );
          free_string ( prgpstrShow[iShow] );
     }

     if ( fShowNothing && nShow == 0 )
     {
          if ( IS_NPC ( ch ) || IS_SET ( ch->comm, COMM_COMBINE ) )
               send_to_char ( "     ", ch );
          send_to_char ( "Nothing.\n\r", ch );
     }
     else if ( nShow != 0 )      // Can't page nothing, or we'll get gibberish.
          page_to_char(outbuf->data, ch);

     /*
      * Clean up.
      */
     free_mem ( prgpstrShow, count * sizeof ( char * ), "show_list_to_char" );
     free_mem ( prgnShow, count * sizeof ( int ), "show_list_to_char" );

     buffer_free(outbuf);

     return;
}

void show_char_to_char_0 ( CHAR_DATA * victim, CHAR_DATA * ch, bool LongLook )
{
     char       buf[MAX_STRING_LENGTH];
     char       cmdbuf[MSL];
     char	message[MAX_STRING_LENGTH];

     buf[0] = '\0';
     message[0] = '\0';

     if ( !LongLook )
     {
          if ( !IS_NPC ( ch ) )
          {
               if ( IS_NPC ( victim ) && ch->pcdata->questmob > 0 && victim->pIndexData->vnum == ch->pcdata->questmob )
                    SLCAT ( buf, "{Y({RTARGET{Y){x " );
          }
          if ( IS_AFFECTED ( victim, AFF_INVISIBLE ) )    			SLCAT ( buf, "{x({bInvis{x) " );
          if ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_WIZINVIS ) ) 	SLCAT ( buf, "{x({mWizi{x) " );
          if ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_CLOAK ) ) 	SLCAT ( buf, "{x({mCloak{x) " );
          if ( IS_AFFECTED ( victim, AFF_HIDE ) )    				SLCAT ( buf, "{x({BHide{x) " );
          if ( IS_AFFECTED ( victim, AFF_CHARM ) )    				SLCAT ( buf, "{x({gCharmed{x) " );
          if ( IS_AFFECTED ( victim, AFF_PASS_DOOR ) )    			SLCAT ( buf, "{x({CTranslucent{x) " );
          if ( IS_AFFECTED ( victim, AFF_FAERIE_FIRE ) )    			SLCAT ( buf, "{x({RPink Aura{x) " );
          if ( IS_EVIL ( victim ) && CAN_DETECT ( ch, DET_EVIL ) )              SLCAT ( buf, "{x({rRed Aura{x) " );
          if ( IS_GOOD ( victim )
               && is_affected ( ch, skill_lookup ( "detect good" ) ) )          SLCAT ( buf, "{x({gGreen Aura{x) " );
          if ( IS_PROTECTED ( victim, PROT_SANCTUARY ) )    			SLCAT ( buf, "{m({WWhite Aura{m){x " );
     }
     if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
     {
          SNP ( cmdbuf, "ch_cmc %s", victim->name );
          if ( !LongLook )
          {
               SLCAT ( buf, click_cmd ( ch->desc, victim->long_descr, cmdbuf, "Context Menu" ) );
               if ( ch->desc->mxp ) // This is because ZMUD barfs on linefeeds in a send. Thus it has to be added back.
               {
                    SLCAT ( buf, "\r\n" );
               }
          }
          else if ( victim->short_descr[0] != '\0' )
          {
               SLCAT ( buf, click_cmd ( ch->desc, victim->short_descr, cmdbuf, "Context Menu" ) );
               SLCAT ( buf, "\r\n" );
          }
          else          /*assumed poly....seems to be the only instance of this */
          {
               SLCAT ( buf, click_cmd ( ch->desc, victim->poly_name, cmdbuf, "Context Menu" ) );
               SLCAT ( buf, "\r\n" );
          }
          page_to_char ( buf, ch );
          return;
     }
     if ( LongLook )
          if ( !IS_NPC ( victim ) && ( !IS_AFFECTED ( victim, AFF_POLY )
                                       || !str_cmp ( victim->poly_name, victim->short_descr ) ) )
               SLCAT ( buf, "{y" );
     SLCAT ( buf, PERSMASK ( victim, ch ) );
     if ( !IS_NPC ( victim ) && !IS_SET ( ch->comm, COMM_BRIEF ) && ( !LongLook ) && ( !IS_AFFECTED ( victim, AFF_POLY ) ) )
          SLCAT ( buf, victim->pcdata->title );
     if ( !LongLook )
     {
          switch ( victim->position )
          {
          case POS_DEAD:               SLCAT ( buf, " is DEAD!!" );               	break;
          case POS_MORTAL:             SLCAT ( buf, " is {Rmortally wounded.{w" );      break;
          case POS_INCAP:              SLCAT ( buf, " is {Rincapacitated.{w" );         break;
          case POS_STUNNED:            SLCAT ( buf, " is lying here stunned." );        break;
          case POS_SLEEPING:
               if (victim->on != NULL)
               {
                    if (IS_SET(victim->on->value[2],SLEEP_AT))
                    {
                         SNP(message," is sleeping at %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else if (IS_SET(victim->on->value[2],SLEEP_ON))
                    {
                         SNP(message," is sleeping on %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else
                    {
                         SNP(message, " is sleeping in %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
               }
               else
                    SLCAT ( buf, " is sleeping here." );
               break;
          case POS_RESTING:
               if (victim->on != NULL)
               {
                    if (IS_SET(victim->on->value[2],REST_AT))
                    {
                         SNP(message," is resting at %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else if (IS_SET(victim->on->value[2],REST_ON))
                    {
                         SNP(message," is resting on %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else
                    {
                         SNP(message, " is resting in %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
               }
               else
                    SLCAT ( buf, " is resting here." );
               break;
          case POS_SITTING:
               if (victim->on != NULL)
               {
                    if (IS_SET(victim->on->value[2],SIT_AT))
                    {
                         SNP(message," is sitting at %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else if (IS_SET(victim->on->value[2],SIT_ON))
                    {
                         SNP(message," is sitting on %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
                    else
                    {
                         SNP(message, " is sitting in %s.", victim->on->short_descr);
                         SLCAT(buf,message);
                    }
               }
               else
                    SLCAT ( buf, " is sitting here." );
               break;
          case POS_STANDING:
               SLCAT ( buf, " is here." );
               break;
          case POS_FIGHTING:
               SLCAT ( buf, " is here, fighting " );
               if ( victim->fighting == NULL )
                    SLCAT ( buf, "thin air??" );
               else if ( victim->fighting == ch )
                    SLCAT ( buf, "YOU!" );
               else if ( victim->in_room == victim->fighting->in_room )
               {
                    SLCAT ( buf, PERSMASK ( victim->fighting, ch ) );
                    SLCAT ( buf, "." );
               }
               else
                    SLCAT ( buf, "someone who left??" );
               break;
          }
     }
     SLCAT ( buf, "{x\n\r" );
     buf[0] = UPPER ( buf[0] );
     page_to_char ( buf, ch );
     return;
}

void show_char_to_char_1 ( CHAR_DATA * victim, CHAR_DATA * ch )
{
     char                buf[MAX_STRING_LENGTH];
     OBJ_DATA           *obj;
     int                 iWear;
     int                 percent;
     bool                found;

     buf[0] = '\0';

     if ( can_see ( victim, ch ) )
     {
          if ( ch == victim )
               act ( "$n looks at $mself.", ch, NULL, NULL, TO_ROOM );
          else
          {
               act ( "$n looks at you.", ch, NULL, victim, TO_VICT );
               act ( "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
          }
     }

     if ( victim->description[0] != '\0' )
     {
          send_to_char ( victim->description, ch );
     }
     else
     {
          act ( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
     }

     if ( victim->max_hit > 0 )
          percent = ( 100 * victim->hit ) / victim->max_hit;
     else
          percent = -1;

     SLCPY ( buf, PERSMASK ( victim, ch ) );

     if ( percent >= 100 )          SLCAT ( buf, TXT_COND_A );
     else if ( percent >= 90 )      SLCAT ( buf, TXT_COND_B );
     else if ( percent >= 75 )      SLCAT ( buf, TXT_COND_C );
     else if ( percent >= 50 )      SLCAT ( buf, TXT_COND_D );
     else if ( percent >= 30 )      SLCAT ( buf, TXT_COND_E );
     else if ( percent >= 15 )      SLCAT ( buf, TXT_COND_F );
     else if ( percent >= 0 )       SLCAT ( buf, TXT_COND_G );
     else                           SLCAT ( buf, TXT_COND_H );
     SLCAT ( buf, "\n\r" );

     buf[0] = UPPER ( buf[0] );
     send_to_char ( buf, ch );

     found = FALSE;
     for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
     {
          if ( ( obj = get_eq_char ( victim, iWear ) ) != NULL
               && can_see_obj ( ch, obj ) )
          {
               if ( !found )
               {
                    send_to_char ( "\n\r", ch );
                    act ( "$N is using:", ch, NULL, victim, TO_CHAR );
                    found = TRUE;
               }
               form_to_char ( ch, "{C<{w%-13s{C>{w  ", wear_info[iWear].name );
               send_to_char ( format_obj_to_char ( obj, ch, TRUE ), ch );
               send_to_char ( "\n\r", ch );
          }
     }

     if ( victim != ch && !IS_NPC ( ch ) && ( number_percent (  ) < ch->pcdata->learned[gsn_peek]
                                              || get_trust ( ch ) >= LEVEL_IMMORTAL ) )
     {
          send_to_char ( "\n\rYou peek at the inventory:\n\r", ch );
          check_improve ( ch, gsn_peek, TRUE, 4 );
          show_list_to_char ( victim->carrying, ch, TRUE, TRUE );
     }

     if ( IS_NPC ( victim ) && !IS_NULLSTR ( victim->pIndexData->notes ) && IS_IMMORTAL ( ch ) )
     {
          send_to_char ( "{W[{GComments{W]{w:\n\r  ", ch );
          send_to_char ( victim->pIndexData->notes, ch );
          send_to_char ( "\n\r", ch );
     }

     return;
}

void show_char_to_char ( CHAR_DATA * list, CHAR_DATA * ch, bool LongLook )
{
     CHAR_DATA          *rch;

     for ( rch = list; rch != NULL; rch = rch->next_in_room )
     {
          if ( rch == ch )
               continue;

          if ( !IS_NPC ( rch ) && IS_SET ( rch->act, PLR_WIZINVIS )
               && get_trust ( ch ) < rch->invis_level )
               continue;

          if ( can_see ( ch, rch ) )
          {
               show_char_to_char_0 ( rch, ch, LongLook );
          }
          else if ( room_is_dark ( ch->in_room ) && CAN_DETECT ( rch, DET_INFRARED ) )
          {
               send_to_char ( "You see glowing red eyes watching YOU!\n\r", ch );
          }
     }

     return;
}

bool check_blind ( CHAR_DATA * ch )
{
     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_HOLYLIGHT ) )
          return TRUE;
     if ( IS_AFFECTED ( ch, AFF_BLIND ) )
     {
          send_to_char ( "You can't see a darned thing!\n\r", ch );
          return FALSE;
     }
     return TRUE;
}

void do_scroll ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 lines;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          if ( ch->lines == 0 )
          {
               send_to_char ( "You do not page long messages.\n\r", ch );
               send_to_char ( "This is not the recommended setting.\n\r", ch);
          }
          else
          {
               form_to_char ( ch, "You currently display %d lines per page.\n\r", ch->lines + 2 );
          }
          return;
     }

     if ( !is_number ( arg ) )
     {
          send_to_char ( "You must provide a number.\n\r", ch );
          return;
     }

     lines = atoi ( arg );

     if ( lines == 0 )
     {
          send_to_char ( "Paging disabled.\n\r", ch );
          ch->lines = 0;
          return;
     }

     if ( lines < 10 || lines > 100 )
     {
          send_to_char ( "You must provide a reasonable number.\n\r", ch );
          return;
     }

     form_to_char ( ch, "Scroll set to %d lines.\n\r", lines );
     ch->lines = lines - 2;
     return;
}

void do_socials ( CHAR_DATA * ch, char *argument )
{
     BUFFER             *buffer;
     int                 iSocial;
     int                 col;

     col = 0;
     buffer = buffer_new (512);
     for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
     {
          bprintf ( buffer, "%-12s", social_table[iSocial].name );
          if ( ++col % 6 == 0 )
               bprintf ( buffer, "\n\r" );
     }
     if ( col % 6 != 0 )
          bprintf ( buffer, "\n\r" );
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

/* RT Commands to replace news, motd, imotd, etc from ROM */
/* You'll notice nobb notb on some of these. This turns off the SunderMud Help Banners. - Lotherius */

void do_motd ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "nobb notb motd" );
}

void do_imotd ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "nobb notb imotd" );
}

void do_rules ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "rules" );
}

void do_story ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "story" );
}

void do_wizlist ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "nobb notb wizlist" );
}

// Generic act flag toggle with message.
//
void real_auto ( CHAR_DATA *ch, long flag, char *text )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( IS_SET ( ch->act, flag ) )
     {
          form_to_char ( ch, "%s: Disabled.\n\r", text );
          REMOVE_BIT( ch->act, flag );
     }
     else
     {
          form_to_char ( ch, "%s: Enabled.\n\r", text );
          SET_BIT ( ch->act, flag );
     }
}

void real_acomm ( CHAR_DATA *ch, long flag, char *text )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( IS_SET ( ch->comm, flag ) )
     {
          form_to_char ( ch, "%s: Disabled.\n\r", text );
          REMOVE_BIT( ch->comm, flag );
     }
     else
     {
          form_to_char ( ch, "%s: Enabled.\n\r", text );
          SET_BIT ( ch->comm, flag );
     }
}

void do_autoassist ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOASSIST, "Automatically Assist Group Members" );
}

void do_autoexit ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOEXIT, "View Brief Exits" );
}

void do_autogold ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOGOLD, "Automatically loot gold from corpses" );
}

void do_autoloot ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOLOOT, "Automatically loot items from corpses" );
}

void do_autosac ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOSAC, "Automatically sacrifice corpses" );
}

void do_autosave ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOSAVE, "Notification of AutoSaves" );
}

void do_afk ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( IS_SET ( ch->act, PLR_AFK ) )
     {
          send_to_char ( "AFK removed...\n\r", ch );
          REMOVE_BIT ( ch->act, PLR_AFK );
          if ( ch->pcdata->afk_tell_first != NULL )
               send_to_char( "You have missed tells, type {greplay{x to see them.\n\r",  ch );
     }
     else
     {
          send_to_char ( "AFK Set..\n\r", ch );
          SET_BIT ( ch->act, PLR_AFK );
     }
}

void do_autosplit ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_AUTOSPLIT, "Share Money with Group Members" );

     if ( !IS_SET ( ch->act, PLR_AUTOSPLIT ) )
          sound ("splitoff.wav", ch );
}

void do_brief ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_BRIEF, "Short Room Descriptions" );
}

void do_fullfight ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_FULLFIGHT, "Full Battle Descriptions" );
}

void do_compact ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_COMPACT, "Compact Mode" );
}

void do_prompt ( CHAR_DATA * ch, char *argument )
{
     char       word[MAX_INPUT_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

     if ( strlen ( argument ) != 0 )
     {
          one_argument ( argument, word );
          if ( !str_cmp ( word, "default" ) )
          {
               free_string ( ch->pcdata->prompt );
               ch->pcdata->prompt = str_dup ( "{G({W$h/$Hhp $m/$Mmn $v/$Vmv{G){x" );
          }
          else
          {
               if (strlen(argument) > 85)
               {
                    send_to_char("Prompt too long. Truncated at 85 characters. (Counting color)\n\r", ch);
                    argument[85] = '\0';
               }
               free_string ( ch->pcdata->prompt );
               ch->pcdata->prompt = str_dup ( argument );
          }
          return;
          form_to_char ( ch, "\n\r{wNew Prompt:\n\r%s\n\r", ch->pcdata->prompt );
     }
     else
          real_acomm ( ch, COMM_PROMPT, "Auto-Prompt" );
}

void do_combine ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_COMBINE, "Combined inventory" );
}

void do_noflashy ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_NOFLASHY, "Blocking of Flashing or Reverse Video text" );
}

void do_darkcolors ( CHAR_DATA * ch, char *argument )
{
     real_acomm ( ch, COMM_DARKCOLOR, "Automatically dim all colours" );
}

void do_nodarkgrey ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( IS_SET ( ch->comm, COMM_NODARKGREY ) )
     {
          REMOVE_BIT ( ch->comm, COMM_NODARKGREY );
          send_to_char ( "You should now see {Dthe dark grey (aka bright black) colour{w.\n\r", ch );
          send_to_char ( "If part of that sentence was missing, your client won't show dark grey.\n\r", ch );
     }
     else
     {
          send_to_char ( "You now see white instead of dark grey.\n\r", ch );
          SET_BIT ( ch->comm, COMM_NODARKGREY );
     }
}

void do_noloot ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_CANLOOT, "Allow your corpse to be looted by others" );
}

void do_nofollow ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( IS_AFFECTED( ch, AFF_CHARM ) )
     {
          send_to_char ( "You can't decide this for yourself now.\n\r", ch);
          return;
     }
     real_auto ( ch, PLR_NOFOLLOW, "Refuse followers" );

     if ( !IS_SET ( ch->act, PLR_NOFOLLOW ) )
          die_follower ( ch );
}

void do_nosummon ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
     {
          if ( IS_SET ( ch->imm_flags, IMM_SUMMON ) )
          {
               send_to_char ( "You are no longer immune to summon. Careful.\n\r", ch );
               REMOVE_BIT ( ch->imm_flags, IMM_SUMMON );
          }
          else
          {
               send_to_char ( "You are now immune to summoning.\n\r", ch );
               SET_BIT ( ch->imm_flags, IMM_SUMMON );
          }
     }
     else
          real_auto ( ch, PLR_NOSUMMON, "Protection from Summoning" );

     return;
}

void do_killer ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( IS_SET ( ch->act, PLR_KILLER ) )
     {
          send_to_char ( "Once a Killer, Always a Killer.\n\r", ch );
          return;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "To declare yourself a KILLER, you must use your password with the killer command.\n\r", ch );
          send_to_char ( "{R{&Warning: {x{RRead HELP PKILL first. Killer flags are PERMANENT!{x\n\r", ch );
          return;
     }
     if ( str_cmp ( crypt ( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
     {
          send_to_char ( "Declaring you a KILLER.\n\r", ch );
          SET_BIT ( ch->act, PLR_KILLER );
          save_char_obj ( ch );
     }
     else
          send_to_char ( "Invalid argument, nothing done.\n\r", ch );
     return;
}

/* New config command */
/* Why RT hated it, I dunno, but I like it. */

void do_config ( CHAR_DATA * ch, char *argument )
{
     BUFFER *buffer;
     bool    haveargs = FALSE;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] != '\0' )
          haveargs = TRUE;

     if (!haveargs)
     {
          buffer = buffer_new (1024);

          bprintf ( buffer, "{G[{W---{G] {wConfig Item        {G[{W---{G] {wChannel\n\r");
          bprintf ( buffer, "{C+{c--------------------------------------------{C+\n\r");
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautoassist        {G[{W%3s{G]{w gossip\n\r",
                    ( IS_SET (ch->act, PLR_AUTOASSIST) ? "YES" : " NO" ),
                    ( !IS_SET ( ch->comm, COMM_NOGOSSIP ) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautoexit          {G[{W%3s{G]{w auction\n\r",
                    ( !IS_SET (ch->act, PLR_AUTOEXIT) ? "YES" : "NO" ),
                    ( !IS_SET (ch->comm, COMM_NOAUCTION ) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautoloot          {G[{W%3s{G]{w music (channel)\n\r",
                    ( IS_SET (ch->act, PLR_AUTOLOOT) ? "YES" : " NO" ),
                    ( !IS_SET (ch->comm, COMM_NOMUSIC) ? "YES" : " NO" ));
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautosac           {G[{W%3s{G]{w question/answer\n\r",
                    ( IS_SET (ch->act, PLR_AUTOSAC) ? "YES" : " NO" ),
                    ( !IS_SET (ch->comm, COMM_NOQUESTION) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautogold          {G[{W%3s{G]{w beep\n\r",
                    ( IS_SET (ch->act, PLR_AUTOGOLD) ? "YES" : " NO" ),
                    ( !IS_SET (ch->comm, COMM_BEEP) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wautosplit         {G[{W%3s{G]{w deaf (shout)\n\r",
                    ( IS_SET (ch->act, PLR_AUTOSPLIT) ? "YES" : " NO" ),
                    ( !IS_SET (ch->comm, COMM_DEAF) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wnoloot            {G[{W---{G]{w\n\r",
                    ( !IS_SET (ch->act, PLR_CANLOOT) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wnosummon          {G[{W%3s{G]{w {R*{wcompact\n\r",
                    ( IS_SET (ch->act, PLR_NOSUMMON) ? "YES" : " NO" ),
                    ( IS_SET (ch->comm, COMM_COMPACT) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wnofollow          {G[{W%3s{G]{w {R*{wbrief\n\r",
                    ( IS_SET (ch->act, PLR_NOFOLLOW) ? "YES" : " NO" ),
                    ( IS_SET (ch->comm, COMM_BRIEF) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wfullfight         {G[{W%3s{G]{w {R*{wcombine\n\r",
                    ( IS_SET (ch->comm, COMM_FULLFIGHT) ? "YES" : " NO" ),
                    ( IS_SET (ch->comm, COMM_COMBINE) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w scroll             {G[{W%3s{G]{w {R*{wcursor\n\r",
                    ( ( ch->lines > 0 ) ? itos(ch->lines+2) : " NO"),
                    ( IS_SET (ch->act, PLR_CURSOR) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w killer             {G[{W%3s{G]{w thief\n\r",
                    ( IS_SET (ch->act, PLR_KILLER) ? "YES" : " NO" ),
                    ( IS_SET (ch->act, PLR_THIEF) ? "YES" : " NO" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wcolor             {G[{W%3s{G]{w sound\n\r",
                    ch->desc->ansi ? "YES" : " NO" ,
                    ch->desc->msp ? "YES" : " NO" );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wdarken            {G[{W%3s{G]{w {R*{wdarkgrey\n\r",
                    ( IS_SET (ch->comm, COMM_DARKCOLOR) ? "YES" : " NO" ),
                    ( IS_SET (ch->comm, COMM_NODARKGREY ) ? "NO" : "YES" ) );
          bprintf ( buffer, "{G[{W%3s{G]{w {R*{wflashing          {G[{W%3s{G]{w MXP Mud eXtension\n\r",
                    ( IS_SET (ch->comm, COMM_NOFLASHY ) ? "NO" : "YES" ),
                    ch->desc->mxp ? "YES" : " NO" );
          bprintf ( buffer, "{C+{c--------------------------------------------{C+\n\r");
          bprintf ( buffer, "{CUse config to toggle items marked with {R*{C.\n\r");
          bprintf ( buffer, "Set notify options under the \"notify\" command.\n\r");
          page_to_char(buffer->data, ch);
          buffer_free ( buffer );
     }
     /* End of ( !haveargs ) */
     else
     {
          if ( strstr (argument, "autoassist" ) != NULL )
               do_autoassist ( ch, "" );
          else if ( strstr (argument, "autoexit" ) != NULL)
               do_autoexit ( ch, "" );
          else if ( strstr (argument, "autoloot" ) != NULL )
               do_autoloot ( ch, "" );
          else if ( strstr (argument, "autogold" ) != NULL )
               do_autogold ( ch, "" );
          else if ( strstr (argument, "autosac" ) != NULL )
               do_autosac ( ch, "" );
          else if ( strstr (argument, "autosplit" ) != NULL )
               do_autosplit ( ch, "" );
          else if ( strstr (argument, "noloot" ) != NULL )
               do_noloot( ch, "" );
          else if ( strstr (argument, "nosummon" ) != NULL )
               do_nosummon ( ch, "" );
          else if ( strstr (argument, "nofollow" ) != NULL )
               do_nofollow ( ch, "" );
          else if ( strstr (argument, "fullfight" ) != NULL )
               do_fullfight ( ch, "" );
          else if ( strstr (argument, "color" ) != NULL )
               do_colour ( ch, "" );
          else if ( strstr (argument, "compact" ) != NULL )
               do_compact ( ch, "" );
          else if ( strstr (argument, "brief" ) != NULL )
               do_brief ( ch, "" );
          else if ( strstr (argument, "combine" ) != NULL )
               do_combine ( ch, "" );
          else if ( strstr (argument, "cursor" ) != NULL )
               do_cursor ( ch, "" );
          else if ( strstr (argument, "darken" ) != NULL )
               do_darkcolors ( ch, "" );
          else if ( strstr (argument, "darkgrey" ) != NULL )
               do_nodarkgrey ( ch, "" );
          else if ( strstr (argument, "flashing" ) != NULL )
               do_noflashy ( ch, "" );
          else
               send_to_char ( "Invalid Config Option.\n\r", ch );
     }
     return;
}

void do_look ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                arg3[MAX_INPUT_LENGTH];
     char                outbuf[MAX_STRING_LENGTH * 4];
     EXIT_DATA          *pexit;
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     char               *pdesc;
     int                 door;
     int                 number, count;

     buf[0] = '\0';
     outbuf[0] = '\0';

     if ( ch->desc == NULL )
          return;
     if ( ch->position < POS_SLEEPING )
     {
          send_to_char ( "You can't see anything but stars!\n\r", ch );
          return;
     }
     if ( ch->position == POS_SLEEPING )
     {
          send_to_char ( "You can't see anything, you're sleeping!\n\r", ch );
          return;
     }
     if ( !check_blind ( ch ) ) // check_blind should give hte error msg here.
          return;
     if ( !IS_NPC ( ch ) && !IS_SET ( ch->act, PLR_HOLYLIGHT ) && room_is_dark ( ch->in_room ) )
     {
          SNP ( buf, "It is pitch black ... \n\r" );
          page_to_char ( buf, ch );
          buf[0] = '\0';	/* reset to NULL */
          show_char_to_char ( ch->in_room->people, ch, FALSE );
          return;
     }

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     number = number_argument ( arg1, arg3 );
     count = 0;

     if ( arg1[0] == '\0' || !str_cmp ( arg1, "auto" ) )
     {
          SNP ( buf, "{c" );
          SLCAT ( outbuf, buf );
          if ( ch->desc->mxp )
          {
               SNP ( buf, MXP_SECURE "<RName>" );
               SLCAT ( outbuf, buf );
          }
          SNP ( buf, "%s",
                IS_RENTED(ch->in_room->lease) && !IS_NULLSTR(ch->in_room->lease->lease_name) ?
                ch->in_room->lease->lease_name : ch->in_room->name );
          SLCAT ( outbuf, buf );
          if ( ch->desc->mxp )
          {
               SNP ( buf, MXP_SECURE "</RName>" MXP_LLOCK );
               SLCAT ( outbuf, buf );
          }
          if ( ch->level >= AVATAR )       /*Zeran: show vnum of room if IMM */
          {
               char                buf2[64];
               SNP ( buf2, " {W[{C%d{W]{x", ch->in_room->vnum );
               SLCAT ( outbuf, buf2 );
          }
          SLCAT ( outbuf, "{w\n\r" );

          if ( arg1[0] == '\0'
               || ( !IS_NPC ( ch ) &&
                    !IS_SET ( ch->comm, COMM_BRIEF ) ) )
          {
               SLCAT ( outbuf, "  " );
               if ( ch->desc->mxp )
               {
                    SNP ( buf, MXP_LSECURE "<RDesc>" ); // Lock secure since room desc is multi-line
                    SLCAT ( outbuf, buf );
               }
               SNP ( buf, "%s",
                     IS_RENTED(ch->in_room->lease) && !IS_NULLSTR(ch->in_room->lease->lease_descr) ?
                     ch->in_room->lease->lease_descr : ch->in_room->description );
               SLCAT ( outbuf, buf );
               if ( ch->desc->mxp )
               {
                    SNP ( buf, MXP_SECURE "</RDesc>" MXP_LLOCK ); // Reset locked
                    SLCAT ( outbuf, buf );
               }
          }

          if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_AUTOEXIT ) )
          {
               SLCAT ( outbuf, "\n\r" );
               page_to_char ( outbuf, ch );
               outbuf[0] = '\0';	/* reset to NULL */
               do_exits ( ch, "auto" );
               send_to_char ( "{w", ch );
          }
          else if ( !IS_NPC ( ch ) )
          {
               SLCAT ( outbuf, "\n\r" );
               page_to_char ( outbuf, ch );
               outbuf[0] = '\0';   /* reset to NULL */
               do_exits ( ch, "" );
               send_to_char ( "{w", ch );
          }
          {
               page_to_char ( outbuf, ch );
          }
          outbuf[0] = '\0';	/* reset to NULL */
          show_list_to_char ( ch->in_room->contents, ch, FALSE, FALSE );
          show_char_to_char ( ch->in_room->people, ch, FALSE );
          return;
     }

     if ( !str_cmp ( arg1, "i" ) || !str_cmp ( arg1, "in" ) )
     {
		  /* 'look in' */
          if ( arg2[0] == '\0' )
          {
               send_to_char ( "Look in what?\n\r", ch );
               return;
          }

          if ( ( obj = get_obj_here ( ch, NULL, arg2 ) ) == NULL )
          {
               send_to_char ( "You do not see that here.\n\r", ch );
               return;
          }

          switch ( obj->item_type )
          {
          default:
               send_to_char ( "That is not a container.\n\r", ch );
               break;

          case ITEM_DRINK_CON:
               if ( obj->value[1] <= 0 )
               {
                    send_to_char ( "It is empty.\n\r", ch );
                    break;
               }

               SNP ( buf, "It's %s full of a %s liquid.\n\r",
                     obj->value[1] < obj->value[0] / 4 ? "less than"
                     : obj->value[1] < 3 * obj->value[0] / 4
                     ? "about" : "more than", liq_table[obj->value[2]].liq_color );

               send_to_char ( buf, ch );
               break;
          case ITEM_CONTAINER:
          case ITEM_CORPSE_NPC:
          case ITEM_CORPSE_PC:
               if ( IS_SET ( obj->value[1], CONT_CLOSED ) )
               {
                    send_to_char ( "It is closed.\n\r", ch );
                    break;
               }
               act ( "$p contains:", ch, obj, NULL, TO_CHAR );
               show_list_to_char ( obj->contains, ch, TRUE, TRUE );
               break;
          }
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg1 ) ) != NULL )
     {
          show_char_to_char_1 ( victim, ch );
          return;
     }

     for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
     {
          if ( can_see_obj ( ch, obj ) )
          {
               pdesc = get_extra_descr ( arg3, obj->extra_descr );
               if ( pdesc != NULL )
               {
                    if ( ++count == number )
                    {
                         send_to_char ( pdesc, ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         return;
                    }
               }

               pdesc = get_extra_descr ( arg3, obj->pIndexData->extra_descr );
               if ( pdesc != NULL )
               {
                    if ( ++count == number )
                    {
                         send_to_char ( pdesc, ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         return;
                    }
               }

               if ( is_name ( arg3, obj->name ) || is_name_abbv ( arg3, obj->name ) )
                    if ( ++count == number )
                    {
                         send_to_char ( obj->description, ch );
                         send_to_char ( "\n\r", ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         if ( !IS_NULLSTR (obj->pIndexData->notes ) && IS_IMMORTAL ( ch ) )
                         {
                              SNP ( buf, "{W[{GComments{W]{w:\n\r  %s\n\r",
                                    obj->pIndexData->notes );
                              send_to_char ( buf, ch );
                         }
                         return;
                    }
          }
     }

     for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
     {
          if ( can_see_obj ( ch, obj ) )
          {
               pdesc = get_extra_descr ( arg3, obj->extra_descr );
               if ( pdesc != NULL )
                    if ( ++count == number )
                    {
                         send_to_char ( pdesc, ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         return;
                    }

               pdesc = get_extra_descr ( arg3, obj->pIndexData->extra_descr );
               if ( pdesc != NULL )
                    if ( ++count == number )
                    {
                         send_to_char ( pdesc, ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         return;
                    }
               if ( is_name ( arg3, obj->name ) || is_name_abbv ( arg3, obj->name ) )
                    if ( ++count == number )
                    {
                         send_to_char ( obj->description, ch );
                         send_to_char ( "\n\r", ch );
                         SNP ( buf, "Condition: %s\n\r", obj_cond ( obj ) );
                         send_to_char ( buf, ch );
                         if ( !IS_NULLSTR (obj->pIndexData->notes ) && IS_IMMORTAL ( ch ) )
                         {
                              SNP ( buf, "{W[{GComments{W]{w:\n\r  %s\n\r",
                                    obj->pIndexData->notes );
                              send_to_char ( buf, ch );
                         }
                         return;
                    }
          }
     }

     if ( count > 0 && count != number )
     {
          if ( count == 1 )
               SNP ( buf, "You only see one %s here.\n\r", arg3 );
          else
               SNP ( buf, "You only see %d %s's here.\n\r", count, arg3 );

          send_to_char ( buf, ch );
          return;
     }

     pdesc = get_extra_descr ( arg1, ch->in_room->extra_descr );
     if ( pdesc != NULL )
     {
          page_to_char ( pdesc, ch );
          return;
     }

     if ( !str_cmp ( arg1, "n" ) || !str_cmp ( arg1, "north" ) ) door = 0;
     else if ( !str_cmp ( arg1, "e" ) || !str_cmp ( arg1, "east" ) ) door = 1;
     else if ( !str_cmp ( arg1, "s" ) || !str_cmp ( arg1, "south" ) ) door = 2;
     else if ( !str_cmp ( arg1, "w" ) || !str_cmp ( arg1, "west" ) ) door = 3;
     else if ( !str_cmp ( arg1, "u" ) || !str_cmp ( arg1, "up" ) ) door = 4;
     else if ( !str_cmp ( arg1, "d" ) || !str_cmp ( arg1, "down" ) ) door = 5;
     else
     {
          send_to_char ( "You do not see that here.\n\r", ch );
          return;
     }

	 /* 'look direction' */
     if ( ( pexit = ch->in_room->exit[door] ) == NULL )	/* no exit */
     {
          send_to_char ( "Nothing special there.\n\r", ch );
          return;
     }

     if ( IS_SET ( pexit->exit_info, EX_CLOSED ) && IS_SET ( pexit->exit_info, EX_HIDDEN ) )	/* closed hidden door */
     {
          send_to_char ( "Nothing special there.\n\r", ch );
          return;
     }

     if ( pexit->description != NULL && pexit->description[0] != '\0' )
     {
          page_to_char ( pexit->description, ch );
     }

     if ( pexit->keyword != NULL && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ' )
     {
          if ( IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               act ( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
          }
          else if ( IS_SET ( pexit->exit_info, EX_ISDOOR ) &&
                    pexit->u1.to_room->people != NULL )
          {
               act ( "The $d is open.", ch, NULL, pexit->keyword, TO_CHAR );
               send_to_char ( "You see:\n\r", ch );
               show_char_to_char ( pexit->u1.to_room->people, ch, TRUE );
          }
          else if ( pexit->u1.to_room->people != NULL )
          {
               send_to_char ( "You see:\n\r", ch );
               show_char_to_char ( pexit->u1.to_room->people, ch, TRUE );
          }
          else
               send_to_char ( "You see nothing of interest in that direction.\n\r", ch );
     }
     else
     {
          if ( IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               act ( "The door is closed.", ch, NULL, pexit->keyword,
                     TO_CHAR );
          }
          else if ( IS_SET ( pexit->exit_info, EX_ISDOOR ) &&
                    pexit->u1.to_room->people != NULL )
          {
               act ( "The door is open.", ch, NULL, pexit->keyword, TO_CHAR );
               send_to_char ( "You see:\n\r", ch );
               show_char_to_char ( pexit->u1.to_room->people, ch, TRUE );
          }
          else if ( pexit->u1.to_room->people != NULL )
          {
               send_to_char ( "You see:\n\r", ch );
               show_char_to_char ( pexit->u1.to_room->people, ch, TRUE );
          }
          else
               send_to_char ( "You see nothing of interest in that direction.\n\r", ch );
     }
     return;
}

void do_scan ( CHAR_DATA * ch, char *argument )
{
     EXIT_DATA          *pexit;
     CHAR_DATA		*rch;
     int                 door;
     char                doors[6][30];
     bool		 thisdir = FALSE;
     bool                anything = FALSE;

     SNP ( doors[0], "{c[North]\n\r{x" );
     SNP ( doors[1], "{c[East]\n\r{x" );
     SNP ( doors[2], "{c[South]\n\r{x" );
     SNP ( doors[3], "{c[West]\n\r{x" );
     SNP ( doors[4], "{c[Up]\n\r{x" );
     SNP ( doors[5], "{c[Down]\n\r{x" );

     if ( !check_blind( ch ) )
          return;

     for ( door = 0; door <= 5; door++ )
     {
          thisdir = FALSE;
		  /* scan direction */
          if ( ( pexit = ch->in_room->exit[door] ) != NULL
               && pexit->u1.to_room != NULL
               && can_see_room ( ch, pexit->u1.to_room ) )
          {
               if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) && ( pexit->u1.to_room->people != NULL ) )
               {
                    rch = pexit->u1.to_room->people;

                    for ( ; rch != NULL; rch = rch->next_in_room )
                    {
                         if (ch == rch)
                              continue;

                         if ( (!IS_NPC(rch)) && (IS_SET(rch->act, PLR_WIZINVIS))
                              && (get_trust( ch ) < rch->invis_level))
                              continue;
                         if ( (!IS_NPC(rch)) && (IS_SET(rch->act, PLR_CLOAK))
                              && (get_trust( ch ) < rch->cloak_level))
                              continue;
                         /* This will show the door only once for each direction */
                         if (!thisdir)
                              send_to_char(doors[door], ch);
                         thisdir = TRUE;
                         if (room_is_dark(rch->in_room))
                              send_to_char ("You see a pair of {rglowing red eyes{x!\n\r", ch);
                         else
                              show_char_to_char ( rch, ch, TRUE );
                         anything = TRUE;
                    }

               }
               else if ( IS_SET ( pexit->exit_info, EX_CLOSED ) && !IS_SET ( pexit->exit_info, EX_HIDDEN ) )
               {
                    send_to_char ( doors[door], ch );
                    if ( ( pexit->description ) &&
                         ( pexit->description[0] != '\0' ) )
                         send_to_char ( pexit->description, ch );
                    else
                         send_to_char ( "a door\n\r", ch );
               }
          }
     }
     if ( anything )
          return;
     send_to_char ( "You see nobody around you.\n\r", ch );
     return;
}

void do_read ( CHAR_DATA * ch, char *argument )
{
     do_look ( ch, argument );
}

void do_lore ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 chance;
     OBJ_DATA           *obj;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Lore what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          send_to_char ( "You don't appear to have that item.\n\r", ch );
          return;
     }
     chance = ch->pcdata->learned[gsn_lore];
     if ( chance < 1 )
     {
          send_to_char ( "You don't have any idea where to begin to look for information.", ch );
          return;
     }
     if ( ch->move < 100 )
     {
          send_to_char ( "You are too tired to look up information on this object\n\r", ch );
          return;
     }
     ch->move -= 100;

     WAIT_STATE ( ch, skill_table[gsn_lore].beats );

	 /*modifiers */

     chance = chance + ( get_curr_stat ( ch, STAT_INT ) - 20 ) * 5;
     if ( number_percent (  ) > chance )
     {
          send_to_char ( "Lore failed.\n\r", ch );
          check_improve ( ch, gsn_lore, FALSE, 1 );
          return;
     }
     send_to_char ( "You manage to find some information.\n\r", ch );
     check_improve ( ch, gsn_lore, TRUE, 1 );
     spell_identify ( skill_lookup ( "lore" ), ch->level, ch, obj );
}

void do_examine ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Examine what?\n\r", ch );
          return;
     }

     do_look ( ch, arg );

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          switch ( obj->item_type )
          {
          default:
               break;
          case ITEM_DRINK_CON:
          case ITEM_CONTAINER:
          case ITEM_CORPSE_NPC:
          case ITEM_CORPSE_PC:
               send_to_char ( "When you look inside, you see:\n\r", ch );
               SNP ( buf, "in %s", arg );
               do_look ( ch, buf );
          }
     }

     return;
}

/*
 * auto-exit changed form and function on Sunder. Sorry Zrin.
 */
void do_exits ( CHAR_DATA * ch, char *argument )
{
     extern char *const  dir_name[];
     char                buf[MAX_STRING_LENGTH];
     char                buf2[128];
     char	         buf3[MAX_STRING_LENGTH];
     EXIT_DATA          *pexit;
     bool                found;
     bool                fAuto;
     bool                Hlight = FALSE;
     int                 door;

     // fAuto = do brief view of exits, else do long view.
     fAuto = !str_cmp ( argument, "auto" );

     if ( !check_blind ( ch ) )
          return;

     // If immortal and brief(?) view, show flags & comments.
     if ( fAuto && IS_IMMORTAL ( ch ) )
     {
          if ( !IS_NULLSTR ( ch->in_room->notes ) )
          {
               SNP ( buf, "{W[{GComments{W]{w:\n\r  %s\n\r", ch->in_room->notes );
               page_to_char ( buf, ch );
          }
          SNP ( buf, "{gRoom flags: {w[{C%s{w]\n\r",
                flag_string ( room_flags, ch->in_room->room_flags ) );
          page_to_char ( buf, ch );
     }

     // Guarantee zeroed strings
     buf[0] = '\0';
     buf[2] = '\0';

     // If "extra info" is toggled, turn on Hlight (who chose this variable, it is too similar to holy light?)
     if ( IS_SET ( ch->act, PLR_XINFO ) )
          Hlight = TRUE;

     // Show appropriate headers.
     if ( Hlight && fAuto )
          SLCPY ( buf, "{gExits:  {w[ {Cflags {w]\n\r" );
     else
          SLCPY ( buf, fAuto ? "{gPaths: {w[{C" : "{gObvious paths:\n\r" );

     found = FALSE;
     // Cycle through all possible doors.
     for ( door = 0; door <= 5; door++ )
     {
          // If the door exists and the player can see destination room, display it.
          if ( ( pexit = ch->in_room->exit[door] ) != NULL
               && pexit->u1.to_room != NULL && can_see_room ( ch, pexit->u1.to_room ) )
          {
               // If "extra info" and "brief view" show door flags.
               if ( Hlight && fAuto )
               {
                    found = TRUE;
                    if (ch->desc->mxp)
                         SNP ( buf2, MXP_SECURE "<Ex>%5s</Ex>" MXP_LLOCK, dir_name[door] );
                    else
                         SNP ( buf2, "%5s", dir_name[door] );
                    SLCAT ( buf2, "   {w[{C " );
                    if ( IS_SET ( pexit->exit_info, EX_ISDOOR ) )
                         SLCAT ( buf2, "door " );
                    if ( IS_SET ( pexit->exit_info, EX_CLOSED ) )
                         SLCAT ( buf2, "closed " );
                    if ( IS_SET ( pexit->exit_info, EX_LOCKED ) )
                         SLCAT ( buf2, "locked " );
                    if ( IS_SET ( pexit->exit_info, EX_PICKPROOF ) )
                         SLCAT ( buf2, "pickproof " );
                    if ( IS_SET ( pexit->exit_info, EX_HIDDEN ) )
                         SLCAT ( buf2, "hidden " );
                    if ( IS_SET ( pexit->exit_info, EX_NO_PASS ) )
                         SLCAT ( buf2, "no_pass" );
                    SLCAT ( buf2, "{w]\n\r" );
                    SLCAT ( buf, buf2 );
               }
               // end hlight && fauto
               else if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) )
               {
                    // If the door isn't closed
                    found = TRUE;
                    // If "brief" view, open door.
                    if ( fAuto )
                    {
                         if ( ch->desc->mxp )
                              SLCAT ( buf, MXP_SECURE " <Ex>" );
                         else
                              SLCAT ( buf, " " );
                         SLCAT ( buf, dir_name[door] );
                         if ( ch->desc->mxp )
                              SLCAT ( buf, MXP_SECURE "</Ex>" MXP_LLOCK );
                    }
                    // If non-brief view, open door.
                    else
                    {
						 /* Okay. Open door, can see DESCR or ROOM_NAME if NULL */
						 /* Always see descr, builder should consider darkness */
                         if ( !IS_NULLSTR ( pexit->description ) )
                              SNP( buf3, pexit->description );
                         else
                         {
                              if (room_is_dark (pexit->u1.to_room) )
                                   SNP( buf3, "{bToo dark to tell");
                              else
                              {
                                   SNP ( buf3, "%s",
                                         IS_RENTED(pexit->u1.to_room->lease)
                                         && !IS_NULLSTR(pexit->u1.to_room->lease->lease_name) ?
                                         pexit->u1.to_room->lease->lease_name
                                         : !IS_NULLSTR ( pexit->u1.to_room->name )
                                         ? pexit->u1.to_room->name : "Somewhere" );
                              }
                              SLCAT(buf3, "\n\r");
                         }
                         if ( ch->desc->mxp )
                              snprintf ( buf + strlen ( buf ), sizeof ( buf )-1, MXP_SECURE "{C <Ex>%-5s</Ex>" MXP_LLOCK "  {w- %s",
                                         capitalize ( dir_name[door] ), buf3);
                         else
                              snprintf ( buf + strlen ( buf ), sizeof ( buf )-1, "{C %-5s  {w- %s",
                                         capitalize ( dir_name[door] ), buf3);
                    }
                    // end of non-brief open door.
               }
               // End of the door isn't closed.
               else if ( IS_SET ( pexit->exit_info, EX_CLOSED ) && !IS_SET ( pexit->exit_info, EX_HIDDEN ) )
               {
                    // If the door is closed, and isn't hidden.
                    found = TRUE;
                    if ( fAuto  )
                    {
                         SLCAT ( buf, " {M<{c" );
                         if ( ch->desc->mxp )
                              SLCAT ( buf, MXP_SECURE "<Ex>" );
                         SLCAT (buf, dir_name[door] );
                         if ( ch->desc->mxp )
                              SLCAT ( buf, MXP_SECURE "</Ex>" MXP_LLOCK );
                         SLCAT (buf, "{M>{C" );
                    }
                    else
                    {
						 /* Okay. CLOSED door, can see DESCR or Nothing */
                         if ( pexit->description != NULL && pexit->description[0] != '\0' )
                              SNP( buf3, pexit->description );
                         else
                         {
                              if ( pexit->keyword != NULL
                                   &&   pexit->keyword[0] != '\0' &&   pexit->keyword[0] != ' ' )
                              {
                                   SNP( buf3, "{cYou can't see through the %s.{w\n\r", pexit->keyword );
                              }
                              else
                                   SNP( buf3, "{cYou can't see through this door.{w\n\r" );
                         }
                         if ( ch->desc->mxp )
                              snprintf ( buf + strlen ( buf ), sizeof ( buf ) -1, MXP_SECURE "{M<{c<Ex>%-5s</Ex>" MXP_LLOCK "{M> {w- %s",
                                         capitalize ( dir_name[door] ), buf3 );
                         else
                              snprintf ( buf + strlen ( buf ), sizeof ( buf ) -1, "{M<{c%-5s{M> {w- %s",
                                         capitalize ( dir_name[door] ), buf3 );
                    }
               }
               // End of door closed and isn't hidden
          }
          // End of a real, visible, door found.
     }
     // End of cycling the doors.
     //
     if ( !found )
          SLCAT ( buf, fAuto ? " none" : "None.\n\r" );

     if ( fAuto && Hlight )
          SLCAT ( buf, "{x\n\r" );
     if ( fAuto && !Hlight )
          SLCAT ( buf, "{w ]{x\n\r" );

     page_to_char ( buf, ch );
     return;
}

/* Gotta love how simple things can be sometimes - Lotherius */
void do_worth ( CHAR_DATA * ch, char *argument )
{
     form_to_char ( ch, "You have ${Y%ld{w gold.\n\r", ch->gold );
     do_bank ( ch, "" );
     do_borrow ( ch, "auto" );
     return;
}

// This calculates an arbitrary "Score" for the character.
// Subject to much tweaking. -- Lotherius
//
int score_calc ( CHAR_DATA *ch )
{
     int score;
     if ( IS_NPC ( ch ) ) // Just in case as usual
          return 0;
     score = ( ( ch->level*3 - ch->pcdata->mob_losses ) +
               ( ch->pcdata->pkill_wins - ch->pcdata->pkill_losses ) +
               ( ch->pcdata->battle_rating + (ch->pcdata->mob_rating/2) ) +
               ( ch->pcdata->questearned / 5000 ) );
     return score;
}

/*
 * Structure for a custom score default
 */

struct cscore_type
{
     char               *text;
};

const struct cscore_type cscore_table[MAX_CSCORE] =
{
     {
          "{/You are #n #t, level #l, #a years old (#A hours).{/"
               "Race: #r  Sex: #s  Class:  #s{/"
               "You have #h/#H hit, #m/#M mana, #v/#V movement.{/"
               "You have #p practices.{/"
               "You are carrying #x/#X items with weight #y/#Y pounds.{/"
               "Str: @S(@s)  Int: @I(@i)  Wis: @W(@w)  Dex: @D(@d)  Con: @C(@c){/"
               "You have scored #e exp, and have #g gold coins.{/"
               "You need #E exp to level.{/"
               "Wimpy set to #W hit points.{/"
               "You are #P.{/"
               "Your alignment is @a.{/"
     },
     {
          "{/You are {W#n #t{w, level {C#l{w, {C#a{w years old ({C#A{w hours).{/"
               "Race: {C#r{w  Sex: {C#s{w  Class:  {C#s{w{/"
               "You have {C#h{w/{C#H{w hit, {C#m{w/{C#M{w mana, {C#v{w/{C#V{w movement.{/"
               "You have {C#p{w practices.{/"
               "You are carrying {C#x{w/{C#X{w items with weight {C#y{w/{C#Y{w pounds.{/"
               "Str: {C@S{w({C@s{w)  Int: {C@I{w({C@i{w)  Wis: {C@W{w({C@w{w)  Dex: {C@D{w({C@d{w)  Con: {C@C{w({C@c{w){/"
               "You have scored {C#e{w exp, and have {C#g{w gold coins.{/"
               "You need {C#E{w exp to level.{/"
               "Wimpy set to {C#W{w hit points.{/"
               "You are {C#P{w.{/"
               "Your alignment is {C@a{w.{/"
     },
     {
          "{/Your stats:{/"
               "STR: @s/@S{/"
               "INT: @i/@I{/"
               "WIS: @w/@W{/"
               "DEX: @d/@D{/"
               "CON: @c/@C{/"
     }
};

void do_cscore ( CHAR_DATA *ch, char *argument )
{
     char *outbuf = NULL;
     int i;

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax: cscore <number>\n\r  <number> is the text you wish to view.\n\r", ch );
          send_to_char ( "Warning: cscore is currently experimental code. Please report any flaws\n\r"
                         "to the IMMs and be aware that usefulness is currently limited in ways that\n\r"
                         "it will not always be.\n\r", ch );
          return;
     }

     i = atoi(argument);

     if ( !ENTRE ( 0, i, MAX_CSCORE+1 ) )
     {
          send_to_char ( "Value out of range. (Yes this is not a very informative message. The code is\n\rstill experimental. So there.)", ch );
          return;
     }

     i--; // Start at base 0;

     outbuf = csc_translate ( ch, cscore_table[i].text, NULL, NULL, NULL );
     page_to_char ( outbuf, ch );
     return;
}

void do_score ( CHAR_DATA *ch, char *argument )
{
     BUFFER    *buffer;
     int        day;

     if ( IS_NPC ( ch ) )
     {
          send_to_char ( "You are an NPC. Bye.", ch );
          return;
     }

     buffer = buffer_new(1024);

     bprintf(buffer, "\n\r{C+{c----------------------------{C[ {W%11s {C]{c----------------------------{C+\n\r",
             ch->name );
     bprintf ( buffer, "{wIncarnation: {D[{W%8s{D]           {wRace: {D[{W%8s{D]      {wClass: {D[{W%8s{D]\n\r",
               IS_IMMORTAL (ch) ? "Immortal" : (ch->pcdata->mortal ? "Mortal" : "Demi-God"),
               race_table[ch->race].name,
               class_table[ch->pcdata->pclass].name );
     day = ch->pcdata->startday + 1;
     bprintf ( buffer, "        {wAge: {D[{W%7d{D]        {wBirthday: {D[{WDay %d of %s, %d{D]\n\r",
               get_age ( ch ), day, month_name[ch->pcdata->startmonth], ch->pcdata->startyear );
     if ( ch->pcdata->clan )
     {
          bprintf ( buffer, "       {wRank: {D[{W%15s{D] {wof Clan: {D[{W%s{D]\n\r",
                    ch->sex == 2 ? ch->pcdata->clan->franks[ch->pcdata->clrank] :
                    ch->pcdata->clan->mranks[ch->pcdata->clrank],
                    ch->pcdata->clan->clan_name );
     }
     bprintf ( buffer, "      {wLevel: {D[{W%7d{D]      {wExperience: {D[{W%7d{D]     {wTo Level: {D[{W%7d{D]\n\r",
               ch->level, ch->exp,
               ( ch->level + 1 ) * exp_per_level ( ch, ch->pcdata->points ) - ch->exp );
     bprintf ( buffer, "   {wQuestPts: {D[{W%7d{D]      {w QP Earned: {D[{W%7d{D]\n\r",
               ch->pcdata->questpoints, ch->pcdata->questearned );

     bprintf ( buffer, "          {C+{c------------------{C[{W    Ratings   {C]{c------------------{C+\n\r" );
     bprintf ( buffer, "  {wMob Kills: {D[{W%7d{D]     {w Mob Deaths: {D[{W%7d{D]     {w  Rating: {D[{W%7ld{D]\n\r",
               ch->pcdata->mob_wins, ch->pcdata->mob_losses, ch->pcdata->mob_rating );
     bprintf ( buffer, " {wPKill Wins: {D[{W%7d{D]    {wPKill Losses: {D[{W%7d{D]       {wRating: {D[{W%7d{D]\n\r",
               ch->pcdata->pkill_wins, ch->pcdata->pkill_losses, ch->pcdata->battle_rating );
     bprintf ( buffer, "                         {wOverall Score: {D[{W%7d{D]\n\r",
               score_calc ( ch ) );
     bprintf (buffer, "{C+{c-----------------------------------------------------------------------{C+{x\n\r");

     if ( ch->pcdata->account->status <= ACCT_VERIFIED && ch->level <=5 )
          bprintf ( buffer, "For more information, use these commands: armor, stats, affects, info\n\r"
                    "  This message will go away after level 5.\n\r" );

     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

void do_stats ( CHAR_DATA * ch, char *argument )
{
     BUFFER *buffer;

     if ( IS_NPC ( ch ) )
          return;

     buffer = buffer_new ( 1024 );

     bprintf(buffer, "\n\r{C+{c----------------------------{C[ {W%11s {C]{c----------------------------{C+\n\r",
             ch->name );

     bprintf ( buffer, "    {wStrength: {D[{W%2d{D/{W%2d{D]{w      Hit+: {D[{W%4d{D]{w   Dam+: {D[{W%4d{D]{w"
               "   Weight: {D[{W%4d{D]{w\n\r                          Wield: {D[{W%4d{D]{w\n\r",
               get_curr_stat ( ch, STAT_STR ),
               ch->perm_stat[STAT_STR],
               str_app[get_curr_stat(ch, STAT_STR)].tohit,
               str_app[get_curr_stat(ch, STAT_STR)].todam,
               str_app[get_curr_stat(ch, STAT_STR)].carry,
               str_app[get_curr_stat(ch, STAT_STR)].wield );
     bprintf ( buffer, "{w   Dexterity: {D[{W%2d{D/{W%2d{D]{w   Defense: {D[{W%4d{D]{w\n\r",
               get_curr_stat ( ch, STAT_DEX ),
               ch->perm_stat[STAT_DEX],
               dex_app[get_curr_stat(ch, STAT_DEX)].defensive );
     bprintf ( buffer, "{wConstitution: {D[{W%2d{D/{W%2d{D]{w  HP Bonus: {D[{W%4d{D]{w  Shock  {D[{W%4d{D]{w\n\r",
               get_curr_stat ( ch, STAT_CON ),
               ch->perm_stat[STAT_CON],
               con_app[get_curr_stat(ch, STAT_CON)].hitp,
               con_app[get_curr_stat(ch, STAT_CON)].shock );
     bprintf ( buffer, "{wIntelligence: {D[{W%2d{D/{W%2d{D] {wInt Learn: {D[{W%4d{D]{w  Mag+   {D[{W%4d{D]{w\n\r",
               get_curr_stat ( ch, STAT_INT ),
               ch->perm_stat[STAT_INT],
               int_app[get_curr_stat(ch, STAT_INT)].learn,
               int_app[get_curr_stat(ch, STAT_INT)].mspell );
     bprintf ( buffer, "{w      Wisdom: {D[{W%2d{D/{W%2d{D]  {wPractice: {D[{W%4d{D]{w Cleric+ {D[{W%4d{D]{w\n\r",
               get_curr_stat ( ch, STAT_WIS ),
               ch->perm_stat[STAT_WIS],
               wis_app[get_curr_stat(ch, STAT_WIS)].practice,
               wis_app[get_curr_stat(ch, STAT_WIS)].cspell );
     bprintf (buffer, "{C+{c-----------------------------------------------------------------------{C+{x\n\r");

     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

void do_info ( CHAR_DATA * ch, char *argument )
{
     BUFFER    *buffer;
     char	buf2[MIL];
     char       message[MIL];
     int        trust;

     if ( IS_NPC ( ch ) )
     {
          send_to_char ( "You are an NPC. Bye.", ch );
          return;
     }

     buffer = buffer_new(1024);

     if ( ch->alignment > 900 )
          SNP ( buf2, "{Wangelic" );
     else if ( ch->alignment > 700 )
          SNP (buf2, "{Wsaintly" );
     else if ( ch->alignment > 350 )
          SNP (buf2, "{Cgood" );
     else if ( ch->alignment > 100 )
          SNP (buf2, "{Ckind" );
     else if ( ch->alignment > -100 )
          SNP (buf2, "{wneutral" );
     else if ( ch->alignment > -350 )
          SNP (buf2, "{rmean" );
     else if ( ch->alignment > -700 )
          SNP (buf2, "{revil" );
     else if ( ch->alignment > -900 )
          SNP (buf2, "{Rdemonic" );
     else
          SNP (buf2, "{Rsatanic" );

     bprintf ( buffer, "{wYou are a %s{w %s %s %s, and ",
               buf2,
               ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
               race_table[ch->race].name,
               class_table[ch->pcdata->pclass].name );

     bprintf ( buffer, "have played for {W%d {whours.\n\r",
               ( ch->played + ( int ) ( current_time - ch->logon ) ) / 3600 );

     switch ( ch->position )
     {
     case POS_DEAD:
          SNP ( buf2, "{RDEAD!!");
          break;
     case POS_MORTAL:
          SNP ( buf2, "{RMortally Wounded!!!!");
          break;
     case POS_INCAP:
          SNP ( buf2, "{yincapacitated!!!");
          break;
     case POS_STUNNED:
          SNP ( buf2, "{ystunned!!");
          break;
     case POS_SLEEPING:
          SNP ( buf2, "{csleeping");
          if (ch->on != NULL)
          {
               if (IS_SET(ch->on->value[2],SLEEP_AT))
               {
                    SNP(message," at %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else if (IS_SET(ch->on->value[2],SLEEP_ON))
               {
                    SNP(message," %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else
               {
                    SNP(message, " in %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
          }
          break;
     case POS_SITTING:
          SNP ( buf2, "{csitting");
          if (ch->on != NULL)
          {
               if (IS_SET(ch->on->value[2],SIT_AT))
               {
                    SNP(message," at %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else if (IS_SET(ch->on->value[2],SIT_ON))
               {
                    SNP(message," on %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else
               {
                    SNP(message, " in %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
          }
          break;
     case POS_RESTING:
          SNP( buf2, "{cresting");
          if (ch->on != NULL)
          {
               if (IS_SET(ch->on->value[2],REST_AT))
               {
                    SNP(message," at %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else if (IS_SET(ch->on->value[2],REST_ON))
               {
                    SNP(message," on %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
               else
               {
                    SNP(message, " in %s.",
                        ch->on->short_descr);
                    SLCAT(buf2,message);
               }
          }
          break;
     case POS_STANDING:
          SNP( buf2, "{wstanding.");
          break;
     case POS_FIGHTING:
          SNP( buf2, "{RFighting ");
          if ( ch->fighting == NULL )
               SLCAT ( buf2, "thin air??" );
          else if ( ch->fighting == ch )
               SLCAT ( buf2, "YOURSELF!!" );
          else if ( ch->in_room == ch->fighting->in_room )
               SLCAT ( buf2, PERSMASK ( ch->fighting, ch ) );
          else
               SLCAT ( buf2, "someone who left??" );
          break;
     default:
          SNP( buf2, "under the Influence of a Bug.");
          break;
     }

     bprintf ( buffer, "You are currently %s{w\n\r", buf2 );

     trust = get_trust ( ch );
     if ( trust > ch->level )
          bprintf ( buffer, "It seems you've tricked the Gods into trusting you at level {G%d{w.\n\r", trust );

     if ( ch->pcdata->condition[COND_DRUNK] > 10 )
          bprintf ( buffer, "You are {Bd{yr{cu{Mn{rk{W.{w\n\r" );
     if ( ch->pcdata->condition[COND_THIRST] == 0 )
          bprintf ( buffer, "You are {Rthirsty{w.\n\r" );
     if ( ch->pcdata->condition[COND_FULL] == 0 )
          bprintf ( buffer, "You are {Rhungry{w.\n\r", ch );

     bprintf ( buffer, "You are speaking in the {Y%s{x language.\n\r", ch->speaking );

	 /* AC moved to Armor command */
	 /* RT wizinvis and holy light */
     if ( IS_IMMORTAL ( ch ) )
     {
          bprintf (buffer, "Holy Light: %s",
                   IS_SET (ch->act, PLR_HOLYLIGHT) ?
                   "on   " : "off  " );
          if ( IS_SET ( ch->act, PLR_WIZINVIS ) )
               bprintf ( buffer, "Invisible: level %d   ", ch->invis_level );
          if ( IS_SET ( ch->act, PLR_CLOAK ) )
               bprintf ( buffer, "Cloak: level %d", ch->cloak_level );

          bprintf ( buffer, "\n\r" );
     }

     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;

}

// Modified to give something useful in the new armor system... - Lotherius
void do_armor (CHAR_DATA *ch, char *argument )
{
     OBJ_DATA  *obj;
     int 	i;
     int 	armor[4];

     // Zero the values
     for ( i = 0; i < 4; i++ )
          armor[i] = 0;

     // Add up the totals
     for ( i = 0; i < MAX_WEAR; i++ )
     {
          if ( ( obj = get_eq_char ( ch, i ) ) == NULL )
               continue;
          armor[0] += c_base_ac ( obj, AC_PIERCE );
          armor[1] += c_base_ac ( obj, AC_BASH   );
          armor[2] += c_base_ac ( obj, AC_SLASH  );
          armor[3] += c_base_ac ( obj, AC_EXOTIC );
     }

     if ( ch->level >= 25 )
          form_to_char ( ch, "{cYour Armor{W:\n\r {cPierce{W: {C%d  {cBash{W: {C%d  {cslash{W: {C%d  {cexotic{W: {C%d{x\n\r",
                         armor[0], armor[1], armor[2], armor[3] );
     if ( ch->armor > 0 )
          form_to_char ( ch, " {cMagical Bonii{W: {C%d{w\n\r", ch->armor );

     for ( i = 0; i < 4; i++ )
     {
          char               *temp;
          switch ( i )
          {
          case ( AC_PIERCE ):
               temp = "piercing";
               break;
          case ( AC_BASH ):
               temp = "bashing";
               break;
          case ( AC_SLASH ):
               temp = "slashing";
               break;
          case ( AC_EXOTIC ):
               temp = "magic";
               break;
          default:
               temp = "error";
               break;
          }

          // With 15 hittable body parts, theoretical max armor should be 1500
          // This is of course with the assumption that a location's armor does not
          // exceed 100 (it shouldn't) without magical bonii. If ALL locations had
          // 100 AC *AND* there were magical bonii, then the total could exceed 1500.
          //
          if ( armor[i] + ch->armor <= 10 )
               form_to_char ( ch, "{wYou are hopelessly vulnerable to {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 25 )
               form_to_char ( ch, "{wYou are defenseless against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 50 )
               form_to_char ( ch, "{wYou are barely protected from {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 125 )
               form_to_char ( ch, "{wYou are slighty armored against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 200 )
               form_to_char ( ch, "{wYou are somewhat armored against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 300 )
               form_to_char ( ch, "{wYou are armored against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 450 )
               form_to_char ( ch, "{wYou are well-armored against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 700 )
               form_to_char ( ch, "{wYou are very well-armored against {W%s.\n\r", temp );
          else if ( armor[i] + ch->armor <= 1000 )
               form_to_char ( ch, "{wYou are heavily armored against {W%s{x.\n\r", temp );
          else if ( armor[i] + ch->armor <= 1300 )
               form_to_char ( ch, "{wYou are superbly armored against {W%s{x.\n\r", temp );
          else if ( armor[i] + ch->armor <= 1500 )
               form_to_char ( ch, "{wYou are almost invulnerable to {W%s{x.\n\r", temp );
          else
               form_to_char ( ch, "{wYou are divinely armored against {W%s{x.\n\r", temp );
     }
     return;
}

void do_affect ( CHAR_DATA * ch, char *argument )
{
     AFFECT_DATA        *paf;
     OBJ_DATA           *tmp;
     bool                noaffect = TRUE;

     send_to_char ( "You are affected by:\n\r", ch );
     if ( ch->affected != NULL )
     {
          noaffect = FALSE;
          for ( paf = ch->affected; paf != NULL; paf = paf->next )
          {
               form_to_char ( ch, "Spell:   [{G%15s{w] ",
                              skill_table[paf->type].name );

               if (paf->location != APPLY_NONE)
                    form_to_char ( ch, "modifies %s by %d ",
                                   affect_loc_name (paf->location), paf->modifier );
               if (paf->duration >= 0)
                    form_to_char ( ch, "for %d hours.\n\r", paf->duration );
               else
                    form_to_char ( ch, "forever.\n\r" );
          }
     }
	 /* now, loop through object affects */
     for ( tmp = ch->carrying; tmp != NULL; tmp = tmp->next_content )
     {
          if ( tmp->wear_loc != WEAR_NONE )
          {
               for ( paf = tmp->affected; paf != NULL; paf = paf->next )
               {
                    if ( paf->bitvector )	/* check for spell vs just normal bonus affect */
                    {
                         noaffect = FALSE;
                         switch (paf->where)
                         {
                         case TO_AFFECTS:
                              form_to_char ( ch, "Affect:  [{G%15s{w] ", affect_bit_name ( paf->bitvector ) );
                              break;
                         case TO_DETECTIONS:
                              form_to_char ( ch, "Detect:  [{G%15s{w] ", detect_bit_name ( paf->bitvector ) );
                              break;
                         case TO_PROTECTIONS:
                              form_to_char ( ch, "Protect: [{G%15s{w] ", protect_bit_name ( paf->bitvector ) );
                              break;
                         }
                         if (paf->location != APPLY_NONE)
                              form_to_char ( ch, "modifies %s by %d ",
                                             affect_loc_name (paf->location), paf->modifier );
                         if (paf->duration >= 0)
                              form_to_char ( ch, "for %d hours.\n\r", paf->duration );
                         else
                              form_to_char ( ch, "until removed.\n\r" );
                    }
               }
               if ( !tmp->enchanted )
               {
                    for ( paf = tmp->pIndexData->affected; paf != NULL; paf = paf->next )
                    {
                         if ( paf->bitvector )
                         {
                              noaffect = FALSE;
                              switch (paf->where)
                              {
                              case TO_AFFECTS:
                                   form_to_char ( ch, "Affect:  [{G%15s{w] ", affect_bit_name ( paf->bitvector ) );
                                   break;
                              case TO_DETECTIONS:
                                   form_to_char ( ch, "Detect:  [{G%15s{w] ", detect_bit_name ( paf->bitvector ) );
                                   break;
                              case TO_PROTECTIONS:
                                   form_to_char ( ch, "Protect: [{G%15s{w] ", protect_bit_name ( paf->bitvector ) );
                                   break;
                              }
                              if (paf->location != APPLY_NONE)
                                   form_to_char ( ch, "modifies %s by %d ",
                                                  affect_loc_name (paf->location), paf->modifier );
                              if (paf->duration >= 0)
                                   form_to_char ( ch, "for %d hours.\n\r", paf->duration );
                              else
                                   form_to_char ( ch, "until removed.\n\r" );
                         }
                         /* end for */
                    }
                    /* end if equipped loop */
               }
               /* End if enchanted */
          }
          /*end carrying for loop */
     }

     if ( noaffect )
          send_to_char ( "Nothing.\n\r", ch );
     return;
}

void do_time ( CHAR_DATA * ch, char *argument )
{
     char               *suf;
     int                 day;

     day = time_info.day + 1;

     if ( day > 4 && day < 20 )
          suf = "th";
     else if ( day % 10 == 1 )
          suf = "st";
     else if ( day % 10 == 2 )
          suf = "nd";
     else if ( day % 10 == 3 )
          suf = "rd";
     else
          suf = "th";
     form_to_char ( ch, "{W[{Y%d{W:{Y00 {C%s{W] {WThe Day of %s, {C%d%s the Month of %s {win year {C%d{w.\n\r",
                    ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
                    time_info.hour >= 12 ? "pm" : "am",
                    day_name[day % 7], day, suf,
                    month_name[time_info.month],
                    time_info.year );
     return;
}

void do_version ( CHAR_DATA * ch, char *argument )
{
     extern char         str_boot_time[];
     form_to_char ( ch, "{w" TXT_MUDVERSION " Compiled : " TXT_COMPILE ".\n\r"
                    "{W" TXT_MUDNAME " {wlast rebooted at %s"
                    "The current system time is %s\n\r",
                    str_boot_time,
                    ( char * ) ctime ( &current_time ) );
     return;
}

/* Outputs a formatted Calendar to the user. */

void do_calendar ( CHAR_DATA * ch, char *argument )
{
     int counter, i;

     form_to_char ( ch, "\n\r               [{CThe Month of {Y%s{C, year {Y%d{w]\n\r\n\r", month_name[time_info.month],
                    time_info.year );

     for ( i = 0; i <= 6 ; i++ )
          form_to_char ( ch, " [{W%11s{w]", capitalize(day_name[i]) );
     send_to_char ( "\n\r\n\r", ch );

     counter = 0;
     for ( i = 0; i <= 34 ; i++ )
     {
          if ( counter > 6 )
          {
               counter = 0;
               send_to_char ( "\n\r", ch );
          }
          if ( i != time_info.day )
               form_to_char ( ch, " [{c%11d{w]", i+1 );
          else
               form_to_char ( ch, " {C[{G%11d{C]{w", i+1 );
          counter ++;
     }
     send_to_char ( "\n\r\n\r", ch );

     if ( time_info.month == 0 )
          form_to_char ( ch, "Last Month: [{M%s{w]  {C::  ", month_name[16] );
     else
          form_to_char ( ch, "Last Month: [{M%s{w]  {C::  ", month_name[time_info.month-1] );

     if ( time_info.month == 16 )
          form_to_char ( ch, "{wNext Month: [{M%s{w]\n\r\n\r", month_name[0] );
     else
          form_to_char ( ch, "{wNext Month: [{M%s{w]\n\r\n\r", month_name[time_info.month+1] );
     return;
}

void do_weather ( CHAR_DATA * ch, char *argument )
{
     static char        *const sky_look[4] =
     {
          "cloudless",
               "cloudy",
               "rainy",
               "lit by flashes of lightning"
     };

     static char        *const sky_look_winter[4] =
     {
          "crystal clear",
               "cloudy",
               "icy",
               "snowing heavily"
     };

     if ( !IS_OUTSIDE ( ch ) )
     {
          send_to_char ( "You can't see the weather indoors.\n\r", ch );
          return;
     }

     if ( weather_info.change >= 0 )
     {
          form_to_char ( ch, "The sky is %s and a warm breeze blows from the south.\n\r",
                         sky_look[weather_info.sky] );
     }
     else
     {
          form_to_char ( ch, "The sky is %s and a cold northern wind chills you.\n\r",
                         sky_look_winter[weather_info.sky] );
     }
     return;
}

void do_help ( CHAR_DATA * ch, char *argument )
{
     HELP_DATA          *pHelp;
     char                argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
     BUFFER		*outbuf;
     bool		 found = FALSE;
     bool		 topbanner = TRUE;
     bool		 bottombanner = TRUE;

     outbuf = buffer_new(1000);

     if ( argument[0] == '\0' )
          argument = "summary";

     /* this parts handles help a b so that it returns help 'a b' */
     argall[0] = '\0';

     /* Specifying notb or nobb as an argument will turn off banners */

     while ( argument[0] != '\0' )
     {
          argument = one_argument ( argument, argone );
          if (!strcmp(argone, "notb") )
          {
               topbanner = FALSE;
               continue;
          }
          if (!strcmp(argone, "nobb") )
          {
               bottombanner = FALSE;
               continue;
          }

          if ( argall[0] != '\0' )
               SLCAT ( argall, " " );
          SLCAT ( argall, argone );
     }

     /* gotta check this again just in case something goofed */
     if ( argall[0] == '\0' )
          SLCAT (argall, "summary");

     for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
     {
          if ( pHelp->level > get_trust ( ch ) )
               continue;

          if ( is_name ( argall, pHelp->keyword ) )
          {
               if (topbanner)
                    bprintf(outbuf,
                            "\n\r{C={c=========================================={C[ {WSunderMud {R][ {C]{c={w\n\r\n\r");
               if ( pHelp->level >= 0 && str_cmp ( argall, "imotd" ) )
               {
                    bprintf ( outbuf, "{G%s\n\r{w", pHelp->keyword );
               }

               /*
                * Strip leading '.' to allow initial blanks.
                * Must use buffer_strcat to prevent conversion of % signs in helpfiles.
                */
               if ( pHelp->text[0] == '.' )
                    buffer_strcat ( outbuf, pHelp->text + 1 );
               else
                    buffer_strcat ( outbuf, pHelp->text );
               found = TRUE;
          }
     }

     if (bottombanner && found)
          bprintf ( outbuf,"{c={C[ {WHelp {C]{c==================================================={x\n\r" );

     if (!found)
          send_to_char ( "No help on that word.\n\r", ch );
     else
     {
          /* Only page if in a mode that prompts pages correctly. */
          if ( ch->desc->connected == CON_PLAYING )
               page_to_char ( outbuf->data, ch );
          else
               send_to_char ( outbuf->data, ch );
     }

     buffer_free(outbuf);

     return;
}

/* whois command */
void do_whois ( CHAR_DATA * ch, char *argument )
{
     BUFFER             *outbuf;
     DESCRIPTOR_DATA    *d;
     bool                found = FALSE;

     if (argument[0] == '\0')
     {
          argument = str_dup (ch->name);
     }

     outbuf = buffer_new(1000);

     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          CHAR_DATA          *wch;

          if ( d->connected != CON_PLAYING || !can_see ( ch, d->character ) )
               continue;
          wch = ( d->original != NULL ) ? d->original : d->character;
          if ( !can_see ( ch, wch ) )
               continue;
          if ( !str_prefix ( argument, wch->name ) )
          {
               found = TRUE;
               bprintf ( outbuf, "   %s%s%s%s{x\n\r",
                         IS_SET ( wch->act,
                                  PLR_KILLER ) ? "{r({RKILLER{r){w " : "",
                         IS_SET ( wch->act,
                                  PLR_THIEF ) ? "{y({RTHIEF{y){w " : "",
                         wch->name, wch->pcdata->title );
               bprintf(outbuf, "  {c___________________________________________\n\r");
               bprintf ( outbuf, "  {c[{CLevel:         {G%4d{c] [{G%19s{c]\n\r",
                         wch->level,
                         wch->level <= HERO ? pc_race_table[wch->race].who_name :
                         wch->pcdata->immtitle );
               bprintf ( outbuf, "  {c[{CClan: {G%13s{c] [{CRank:{G %13s{c]\n\r",
                         (wch->pcdata->clan) ? wch->pcdata->clan->clan_short : "Clanless",
                         (wch->pcdata->clan) ? (wch->sex == 2 ? wch->pcdata->clan->franks[wch->pcdata->clrank] :
                                                wch->pcdata->clan->mranks[wch->pcdata->clrank] ) : "Clanless" );
               bprintf ( outbuf,
                         "  {c[{CPkill Wins:  {G%6d{c] [{CPkill Lost:  {R%6d{c]{x\n\r",
                         wch->pcdata->pkill_wins,
                         wch->pcdata->pkill_losses );
               bprintf ( outbuf, "  {c[{CMob Kills:  {G%7ld{c] [{CDeaths:     {R%7ld{c]{x\n\r",
                         wch->pcdata->mob_wins,
                         wch->pcdata->mob_losses );
               bprintf ( outbuf, "  {c[{CPK Rating:    {G%5d{c] [{CMob Rating:   {G%5ld{c]\n\r",
                         wch->pcdata->battle_rating, wch->pcdata->mob_rating );
               bprintf ( outbuf, "  {c[{CQP Total:   {G%7ld{c] [{CQP Earned   {G%7ld{c]\n\r",
                         wch->pcdata->questpoints, wch->pcdata->questearned );
               bprintf ( outbuf, "  {c[{CAge: {G%14d{c] {c[{CHours: {G%12d{c]\n\r",
                         get_age(wch),
                         ( wch->played + ( int ) ( current_time - wch->logon ) ) / 3600 );
               bprintf ( outbuf, "  {c[{CScore:      {G%7d{c] [                   ]\n\r",
                         score_calc ( wch ) );
               bprintf( outbuf, "  {c[_________________________________________]{x\n\r");
               if ( IS_IMMORTAL(wch) )
                    bprintf( outbuf, "   {W%s is a {RStaff Member.{w\n\r", wch->name );
               else if ( wch->pcdata->mortal )
                    bprintf( outbuf, "   {R%s is a mortal.{w\n\r", wch->name );
               else
                    bprintf( outbuf, "   {Y%s is a Demi-God.{w\n\r", wch->name );

               if ( wch->pcdata->email &&
                    wch->pcdata->email[0] != '\0' )
               {
                    bprintf ( outbuf, "   {W%s's email is: {C%s{x\n\r",
                              wch->name, wch->pcdata->email );
               }
          }
     }
     if ( !found )
     {
          send_to_char ( "No one of that name is playing.\n\r", ch );
          buffer_free(outbuf);
          return;
     }
     page_to_char ( outbuf->data, ch );
     buffer_free (outbuf);
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */

/* Newer 'who' command by Zeran */

/* 6/24/02 - Malloc here replaced with alloc_mem  - lotherius */

void do_who ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char                clanbuf[40];
     char                form[MAX_STRING_LENGTH];
     BUFFER		*output;
     DESCRIPTOR_DATA    *d;
     int                 iClass;
     int                 iRace = 0;
     int                 iLevelLower;
     int                 iLevelUpper;
     int                 nNumber;
     int                 immMatch = 0;
     int                 mortMatch = 0;
     bool                rgfClass[MAX_CLASS];
     bool                fClassRestrict;
     bool                fRaceRestrict;
     bool                fImmortalOnly;
     bool                fClanOnly, fMortal, fDemi, fHelper;
     struct who_slot    *table[111];	/*want 1 to 110, not 0 to 109 */
     int                 counter;

     for ( counter = 1; counter <= 110; counter++ )	/* init table */
          table[counter] = NULL;

     /*
      * Set default arguments.
      */

     iLevelLower = 0;
     iLevelUpper = MAX_LEVEL;
     fClassRestrict = FALSE;
     fRaceRestrict = FALSE;
     fImmortalOnly = FALSE;
     fClanOnly = FALSE;
     fMortal = FALSE;
     fDemi = FALSE;
     fHelper = FALSE;
     for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
          rgfClass[iClass] = FALSE;

	 /*
	  * Parse arguments.
	  */
     nNumber = 0;
     for ( ;; )
     {
          char                arg[MAX_STRING_LENGTH];

          argument = one_argument ( argument, arg );
          if ( arg[0] == '\0' )
               break;

          if ( is_number ( arg ) )
          {
               switch ( ++nNumber )
               {
               case 1:
                    iLevelLower = atoi ( arg );
                    break;
               case 2:
                    iLevelUpper = atoi ( arg );
                    break;
               default:
                    send_to_char ( "Only two level numbers allowed.\n\r", ch );
                    return;
               }
          }
          else
          {

			   /*
				* Look for classes to turn on.
				* And other flags.
				*/
               if ( arg[0] == 'i' )
               {
                    fImmortalOnly = TRUE;
               }
               else if ( !str_cmp ( arg, "clan" ) )
               {
                    if ( ch->pcdata->clan && !strcmp(ch->pcdata->clan->clan_short, "Loner" ) )
                    {
                         send_to_char ( "'who clan' is only available to players in clans.\n\r", ch );
                         return;
                    }
                    fClanOnly = TRUE;
               }
               else if ( !str_cmp ( arg, "mortal" ) || !str_cmp ( arg, "mort") )
               {
                    fMortal = TRUE;
               }
               else if ( !str_cmp ( arg, "demigod") || !str_cmp ( arg, "demi-god") || !str_cmp (arg, "demi") )
               {
                    fDemi = TRUE;
               }
               else if ( !str_cmp ( arg, "helper") )
               {
                    fHelper=TRUE;
               }
               else
               {
                    iClass = class_lookup ( arg );
                    if ( iClass == -1 )
                    {
                         iRace = pcrace_lookup ( arg );

                         if ( iRace == 0 )
                         {
                              send_to_char ( "That's not a valid race or class.\n\r", ch );
                              return;
                         }
                         else
                         {
                              fRaceRestrict = TRUE;
                         }
                    }
                    else
                    {
                         send_to_char
                              ( "Sorry, who list by class has been removed.\n\r",  ch );
                         return;
                    }
               }
          }
     }

     output = buffer_new(1000);

	 /* Zeran - form table */
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          CHAR_DATA          *wch;

		  /*
		   * Check for match against restrictions.
		   * Don't use trust as that exposes trusted mortals.
		   */
          wch = ( d->original != NULL ) ? d->original : d->character;
          if ( d->connected != CON_PLAYING || !can_see ( ch, wch ) )
          {
               if ( ( d->connected < CON_NOTE_TO ) ||
                    ( d->connected > CON_NOTE_FINISH ) )
                    continue;
          }

          if ( wch->level < iLevelLower
               || wch->level > iLevelUpper
               || ( fImmortalOnly && wch->level < LEVEL_HERO )
               || ( fClassRestrict && !rgfClass[wch->pcdata->pclass] )
               || ( fRaceRestrict && ( wch->pcdata->pcrace != iRace ) )
               || ( fMortal && !wch->pcdata->mortal )
               || ( fHelper && (wch->pcdata->account->status != ACCT_HELPER) )
               || ( fDemi && wch->pcdata->mortal && IS_IMMORTAL(wch) )
               || ( fClanOnly && !is_same_clan (wch, ch) ) )
               continue;

          if ( wch->level >= LEVEL_IMMORTAL )
               immMatch++;
          else
               mortMatch++;

          if ( table[wch->level] == NULL )	/*empty list */
          {
               table[wch->level] = alloc_mem ( sizeof ( struct who_slot ), "do_who" );
               table[wch->level]->ch = wch;
               table[wch->level]->next = NULL;
          }
          else			/* non-empty list */
          {
               struct who_slot    *tmp = table[wch->level];
               for ( ; tmp->next != NULL; tmp = tmp->next );	/*get to end of list */
               tmp->next = alloc_mem ( sizeof ( struct who_slot ), "do_who" );
               tmp->next->ch = wch;
               tmp->next->next = NULL;
          }
     }
     /* end form table...fun, fun, fun! :) */

	 /*
	  * Now show matching chars.
	  */

     bprintf ( output,
               "\n\r{C+{c--------------------------------{C[ {WThe Staff {C]{c--------------------------------{C+{x\n\r" );

     for ( counter = 110; counter >= 1; counter-- )	/*outside list loop */
     {
          CHAR_DATA          *wch;
          struct who_slot    *tmp = table[counter];
          struct who_slot    *free_tmp = NULL;

          if ( counter == 101 )
          {
               bprintf ( output,
                         "{C+{c-----------------------------------------------------------------------------{C+{x\n\r\n\r" );
               bprintf ( output,
                         "{C+{c-------------------------------{C[ {WThe Players {C]{c-------------------------------{C+{x\n\r" );
          }

          if ( tmp == NULL )	/* no one at this level */
               continue;
		  /* now, follow each chain to end */
          for ( ; tmp != NULL; tmp = tmp->next )
          {
               wch = tmp->ch;
               if ( wch == NULL )	/* got a problem */
               {
                    bugf ( "Got null table->ch, argh." );
                    continue;
               }
			   /* free the old struct who_slot */
               if ( free_tmp )
                    free_mem ( free_tmp, sizeof ( struct who_slot), "do_who" );
               SLCPY ( form, "{x[{Y%2d {W%s{x] %s%s%s%s%s %s" );

               if ( ( !IS_NPC ( wch ) ) &&
                    ( wch->pcdata->clan != NULL ) )
               {
                    SNP ( clanbuf, "%s",
                          wch->pcdata->clan->clan_name );
               }
               else
                    SNP ( clanbuf, "%s", "" );
               SNP ( buf, form,
                     wch->level,
                     wch->level <= HERO ? pc_race_table[wch->race].who_name : wch->pcdata->immtitle,
                     IS_SET ( wch->act, PLR_AFK ) ? "{r(AFK){x " : "",
                     IS_SET ( wch->act, PLR_KILLER ) ? "{r(KILLER){x " : "",
                     IS_SET ( wch->act, PLR_THIEF ) ? "{b(THIEF) {x" : "",
                     wch->name, IS_NPC ( wch ) ? "" : wch->pcdata->title, clanbuf );
               if ( IS_SET ( wch->act, PLR_WIZINVIS ) )
                    SLCAT ( buf, " {r({cWizi{r){x" );
               if ( IS_SET ( wch->act, PLR_CLOAK ) )
                    SLCAT ( buf, " {r({cCloak{r){x" );

               if ( !IS_NPC ( wch ) && ( wch->desc != NULL ) &&
                    wch->desc->editor )
                    SLCAT ( buf, " {M(OLC){x " );

			   /* hmm
				if ( (d->connected >= CON_NOTE_TO) || (d->connected <= CON_NOTE_FINISH) )
				SLCAT (buf, " {g(NOTE{x ");
				*/
               SLCAT ( buf, "\n\r" );

               bprintf ( output, buf );
			   /* set the free_tmp pointer in order to call free(free_tmp) near top
				* of loop */
               free_tmp = tmp;
          }
          /*end inner list loop */
		  /* free up the last who_slot from this level hash */
          if ( free_tmp )
               free_mem ( free_tmp, sizeof ( struct who_slot), "do_who" );
     }
     /*end countdown loop */

     bprintf( output, "{C+{c-----------------------------------------------------------------------------{C+{x\n\r" );

     bprintf ( output, "\n\r{yGuardians found: {r%d{x \n\r{yPlayers found:   {c%d{x\n\r", immMatch, mortMatch );
     page_to_char(output->data, ch);
     buffer_free(output);

     return;
}

/* For an accurate max_on, count must be called during each character's logon. */
void do_count ( CHAR_DATA * ch, char *argument )
{
     int                 count;
     DESCRIPTOR_DATA    *d;

     count = 0;

     /* Let's make sure we get a good count! */
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          if ( d->connected == CON_PLAYING && can_see ( ch, d->character ) )
               count++;
          if ( d->connected >= CON_NOTE_TO && can_see ( ch, d->character ) )
               count++;
     }

     max_on = UMAX ( count, max_on );
     if ( max_on == count )
          form_to_char ( ch, "There are %d characters on, the most on since reboot.\r\n\r\n", count );
     else
          form_to_char ( ch, "There are %d characters on, the most on since reboot was %d.\r\n\r\n", count, max_on );
     return;
}

void do_inventory ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "You are carrying:\n\r", ch );
     show_list_to_char ( ch->carrying, ch, TRUE, TRUE );
     return;
}

void do_equipment ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj;
     int                 iWear;
     bool                found;

     send_to_char ( "{C<{wLocation     {C> <{wAC Pier/Slas/Bash/Mag{C> {wItem Used:\n\r", ch );
     send_to_char ( "{c--------------- ----------------------- ----------{w\n\r", ch );
     found = FALSE;
     for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
     {
          if ( ( obj = get_eq_char ( ch, iWear ) ) == NULL )
               continue;

          form_to_char ( ch, "{C<{w%-13s{C>{w ", wear_info[iWear].name );

          if ( can_see_obj ( ch, obj ) )
          {
               if ( wear_info[iWear].has_ac == FALSE )
                    send_to_char ( "                        ", ch );
               else
                    form_to_char ( ch, "{C<{w%3d{C> <{w%3d{C> <{w%3d{C> <{w%3d{C>{w ",
                                   c_base_ac ( obj, AC_PIERCE ),
                                   c_base_ac ( obj, AC_BASH   ),
                                   c_base_ac ( obj, AC_SLASH  ),
                                   c_base_ac ( obj, AC_EXOTIC ) );
               send_to_char ( format_obj_to_char ( obj, ch, TRUE ), ch );
               send_to_char ( "\n\r", ch );
          }
          else
          {
               send_to_char ( "                        something.\n\r", ch );
          }
          found = TRUE;
     }
     if ( !found )
          send_to_char ( "Nothing.\n\r", ch );
     return;
}

// Compare needs fixed for new AC ...
//
void do_compare ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj1;
     OBJ_DATA           *obj2;
     int                 value1;
     int                 value2;
     char               *msg;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );
     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Compare what to what?\n\r", ch );
          return;
     }

     if ( ( obj1 = get_obj_carry ( ch, arg1, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     if ( arg2[0] == '\0' )
     {
          for ( obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content )
          {
               if ( obj2->wear_loc != WEAR_NONE
                    && can_see_obj ( ch, obj2 )
                    && obj1->item_type == obj2->item_type
                    && ( obj1->wear_flags & obj2->
                         wear_flags & ~ITEM_TAKE ) != 0 )
                    break;
          }

          if ( obj2 == NULL )
          {
               send_to_char ( "You aren't wearing anything comparable.\n\r", ch );
               return;
          }
     }

     else if ( ( obj2 = get_obj_carry ( ch, arg2, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     msg = NULL;
     value1 = 0;
     value2 = 0;

     if ( obj1 == obj2 )
     {
          msg = "You compare $p to itself.  It looks about the same.";
     }
     else if ( obj1->item_type != obj2->item_type )
     {
          msg = "You can't compare $p and $P.";
     }
     else
     {
          switch ( obj1->item_type )
          {
          default:
               msg = "You can't compare $p and $P.";
               break;

          case ITEM_ARMOR:
               value1 = c_base_ac(obj1, AC_PIERCE) + c_base_ac(obj1, AC_BASH)
                    + c_base_ac(obj1, AC_SLASH ) + c_base_ac(obj1, AC_EXOTIC);
               value2 = c_base_ac(obj2, AC_PIERCE) + c_base_ac(obj2, AC_BASH)
                    + c_base_ac(obj2, AC_SLASH ) + c_base_ac(obj2, AC_EXOTIC);
               break;
          case ITEM_WEAPON:
               if ( obj1->pIndexData->new_format )
                    value1 = ( 1 + obj1->value[2] ) * obj1->value[1];
               else
                    value1 = obj1->value[1] + obj1->value[2];

               if ( obj2->pIndexData->new_format )
                    value2 = ( 1 + obj2->value[2] ) * obj2->value[1];
               else
                    value2 = obj2->value[1] + obj2->value[2];
               break;
          }
     }

     if ( msg == NULL )
     {
          if ( value1 == value2 )
               msg = "$p and $P look about the same.";
          else if ( value1 > value2 )
               msg = "$p looks better than $P.";
          else
               msg = "$p looks worse than $P.";
     }

     act ( msg, ch, obj1, obj2, TO_CHAR );
     return;
}

/*
 * new do_areas by Lotherius
 */

void do_areas ( CHAR_DATA * ch, char *argument )
{
     BUFFER    *buffer;
     AREA_DATA *pArea;
     int        i,x,y;

     if (argument[0] != '\0')
     {
          send_to_char("No options are currently available for the Area List.\n\r",ch);
          return;
     }

     buffer = buffer_new ( 1024 );
     x = MAX_VNUM;
     y = 0;

     bprintf ( buffer, "{c-{C[ {WAreas {C]{c-----------------------------------------------------------{C-{w\n\r" );

     bprintf ( buffer, "[{B Level {w] [{WName                        {w] [{YZone{w] [{GCredits             {w]\n\r" );
     for ( i = area_first->vnum; i <= area_last->vnum; i++ )
     {
          for ( pArea = area_first; pArea; pArea = pArea->next )
               if ( pArea->lvnum < x && pArea->lvnum > y )
                    x = pArea->lvnum;
          y = x;
          x = MAX_VNUM;
          for ( pArea = area_first; pArea; pArea = pArea->next )
               if ( y == pArea->lvnum )
               {
                    if ( pArea->zone > -1 )
                         bprintf ( buffer, "[{B%3d %3d{w] [{W%-28s{w] [{Y%3d {w] [{G%-20s{w]\n\r",
                                   pArea->llev,
                                   pArea->hlev,
                                   pArea->name,
                                   pArea->zone,
                                   pArea->credits );
               }
     }

     bprintf ( buffer, "{c---------------------------------------------------------------------{C-{w\n\r" );
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

void do_credits ( CHAR_DATA * ch, char *argument )
{
     do_help ( ch, "diku" );
     return;
}

void do_where ( CHAR_DATA * ch, char *argument )
{
     if ( ch->in_room->area->name != NULL && ch->in_room->area->name[0] != '\0' )
          form_to_char ( ch, "You are currently in: {W%s{x\n\r", ch->in_room->area->name );
     else
          send_to_char ( "You are in an unnamed area, inform the IMPs asap.\n\r", ch );
     form_to_char ( ch, "This area is designed for levels {Y%d{w to {Y%d{w, ", ch->in_room->area->llev, ch->in_room->area->hlev );
     form_to_char ( ch, "and was originally built by: {C%s{w\n\r", ch->in_room->area->credits );

     return;
}

void do_consider ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' && ch->fighting != NULL )
     {
          SNP( arg, ch->fighting->name );
     }

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Consider killing whom?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "all" ) )
     {
          bool found = FALSE;
          for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
          {
               if ( can_see ( ch, victim ) && ch != victim )
               {
                    found = TRUE;
                    real_consider ( ch, victim );
               }
          }
          if ( !found )
               send_to_char ( "You can't seem to find anyone here besides yourself.\n\r", ch );
     }
     else
     {
          if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
          {
               send_to_char ( "They're not here.\n\r", ch );
               return;
          }
          else
               real_consider ( ch, victim );
     }
     return;
}

void real_consider ( CHAR_DATA *ch, CHAR_DATA *victim )
{
     char               *msg = '\0';
     char               *buf = '\0';
     int                 diff;
     int                 hpdiff;

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     diff = victim->level - ch->level;

     if ( diff <= -10 )          msg = "$N: $E is not even worthy of your attention.";
     else if ( diff <= -5 )      msg = "$N: You could kill $M even on a bad day.";
     else if ( diff <= -2 )      msg = "$N: $E shouldn't be too hard.";
     else if ( diff <= 1 )       msg = "$N: The {Yperfect match{w!";
     else if ( diff <= 4 )       msg = "$N: With some luck & skill you could kill $M.";
     else if ( diff <= 9 )       msg = "$N: Maybe you should purchase life insurance first.";
     else                        msg = "$N: Would you like to borrow a pick & shovel?";

     act ( msg, ch, NULL, victim, TO_CHAR );

     /* additions by king@tinuviel.cs.wcu.edu */
     hpdiff = ( ch->hit - victim->hit );

     if ( ( ( diff >= 0 ) && ( hpdiff <= 0 ) ) || ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
     {
          send_to_char ( "Also,", ch );
     }
     else
     {
          send_to_char ( "However,", ch );
     }
     if ( hpdiff >= 101 )          buf = " you are currently much healthier than $E.";
     if ( hpdiff <= 100 )          buf = " you are currently healthier than $E.";
     if ( hpdiff <= 50 )           buf = " you are currently slightly healthier than $E.";
     if ( hpdiff <= 25 )           buf = " you are a teensy bit healthier than $E.";
     if ( hpdiff == 0 )		   buf = " $E and you are just as healthy.";
     if ( hpdiff <= 0 )            buf = " $E is a teensy bit healthier than you.";
     if ( hpdiff <= -25 )          buf = " $E is slightly healthier than you.";
     if ( hpdiff <= -50 )          buf = " $E is healthier than you.";
     if ( hpdiff <= -100 )         buf = " $E is much healthier than you.";
     act ( buf, ch, NULL, victim, TO_CHAR );
     return;
}

void set_title ( CHAR_DATA * ch, char *title )
{
     char                buf[MAX_STRING_LENGTH];

     /* Silly mob, titles are for PC's */
     if ( IS_NPC ( ch ) )
          return;

     if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
     {
          SLCPY ( buf, " " );
          SLCAT ( buf, title );
     }
     else
     {
          SLCPY ( buf, title );
     }

     free_string ( ch->pcdata->title );
     ch->pcdata->title = str_dup ( buf );
     return;
}

void do_title ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Change your title to what?\n\r", ch );
          return;
     }
     if ( cstrlen ( argument ) > 45 )
     {
          send_to_char ( "Maximum of 45 characters allowed in your title, not counting colour.\n\r", ch );
          return;
     }
     if ( strlen ( argument ) > 85 )
     {
          send_to_char ( "Maximum of 85 characters allowed in your title, counting colour codes.\n\r", ch );
          return;
     }
     set_title ( ch, argument );
     send_to_char ( "Ok.\n\r", ch );
}

void do_immtitle ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Change your immtitle to what?\n\r", ch );
          return;
     }
     if ( strlen ( argument ) > 15 )
          argument[15] = '\0';

     free_string ( ch->pcdata->immtitle );
     ch->pcdata->immtitle = str_dup ( argument );
     send_to_char ( "ImmTitle set.\n\r", ch );
}

void do_email ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Change your email to what?\n\r", ch );
          return;
     }
     if ( strlen ( argument ) > 50 )
          argument[50] = '\0';
     free_string ( ch->pcdata->email );
     ch->pcdata->email = str_dup ( argument );
     send_to_char ( "Email set.\n\r", ch );
}

void do_description ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];

     if ( argument[0] != '\0' )
     {
          buf[0] = '\0';

          if ( argument[0] == '+' )
          {
               if ( ch->description != NULL )
                    SLCAT ( buf, ch->description );
               argument++;
               while ( isspace ( *argument ) )
                    argument++;
          }

          if ( strlen ( buf ) + strlen ( argument ) >= MAX_STRING_LENGTH - 2 )
          {
               send_to_char ( "Description too long.\n\r", ch );
               return;
          }

          if ( !str_prefix ( argument, "edit" ) )
          {
               if ( IS_NPC ( ch ) )
               {
                    if ( ch->desc )
                         send_to_char ( "You must use the non-interactive editor.", ch );
                    return;
               }
               ch->pcdata->mode = MODE_DESCEDIT;
               string_append ( ch, &ch->description );
               return;
          }

          SLCAT ( buf, argument );
          SLCAT ( buf, "\n\r" );
          free_string ( ch->description );
          ch->description = str_dup ( buf );
     }

     send_to_char ( "Your description is:\n\r", ch );
     send_to_char ( ch->description ? ch->description : "(None).\n\r", ch );
     return;
}

void do_report ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_INPUT_LENGTH];
     form_to_char ( ch, "You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
                    ch->hit, ch->max_hit,
                    ch->mana, ch->max_mana,
                    ch->move, ch->max_move, ch->exp );
     SNP ( buf, "{g$n says '{GI have %d/%d hp %d/%d mana %d/%d mv %d xp.{g'",
           ch->hit, ch->max_hit, ch->mana, ch->max_mana,
           ch->move, ch->max_move, ch->exp );
     act ( buf, ch, NULL, NULL, TO_ROOM );
     return;
}

void do_practice ( CHAR_DATA * ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          show_current_prac ( ch );
     }
     else
     {
          CHAR_DATA          *mob;
          bool		      doall = FALSE; /* practice everything? */
          int                 adept, sn;

          if ( !IS_AWAKE ( ch ) )
          {
               send_to_char ( "In your dreams, or what?\n\r", ch );
               return;
          }
          for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
          {
               if ( IS_NPC ( mob ) && IS_SET ( mob->act, ACT_PRACTICE ) )
                    break;
          }
          if ( mob == NULL )
          {
               send_to_char ( "You can't do that here.\n\r", ch );
               return;
          }
          if ( ch->pcdata->practice <= 0 )
          {
               send_to_char ( "You have no practice sessions left.\n\r", ch );
               return;
          }

          if ( !str_cmp ( argument, "all" ) )
               doall = TRUE;

          adept = class_table[ch->pcdata->pclass].skill_adept;
          if ( !doall )
          {
               if ( ( sn = skill_lookup ( argument ) ) < 0 ||
                    ( ( ( skill_available ( sn, ch, SKILL_PRAC, NULL ) == FALSE ) ) ) )
               {
                    send_to_char ( "You can't practice that.\n\r", ch );
                    return;
               }

               if ( ch->pcdata->learned[sn] >= adept )
               {
                    form_to_char ( ch, "You can only learn more about %s through experience.\n\r", skill_table[sn].name );
                    return;
               }
               else
               {
                    ch->pcdata->practice--;
                    ch->pcdata->learned[sn] += ( int_app[get_curr_stat ( ch, STAT_INT )].learn ) / 3;
                    if ( ch->pcdata->learned[sn] < adept )
                    {
                         //act ( "You practice $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
                         form_to_char ( ch, "You practice %s.   ", skill_table[sn].name );
                         act ( "$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM );
                         form_to_char ( ch, "Practiced %s to %d%%.\n\r", skill_table[sn].name, ch->pcdata->learned[sn] );
                    }
                    else
                    {
                         ch->pcdata->learned[sn] = adept;
                         act ( "You are now learned at $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
                         act ( "$n is now learned at $T.",   ch, NULL, skill_table[sn].name, TO_ROOM );
                    }
               }
          }
          // end of single prac
          else
          {
               bool found = FALSE;

               form_to_char ( ch, "Practicing everything to at least %d%%.\n\rYou have %d practices.\n\r",
                              (adept-10), ch->pcdata->practice );

               for ( sn = 0; sn < MAX_SKILL; sn++ ) /* Loop through all skills */
               {
                    if ( skill_table[sn].name == NULL )
                         break;
                    if ( skill_available ( sn, ch, SKILL_AVAIL, NULL ) == FALSE )
                         continue;

                    while ( ( ch->pcdata->learned[sn] < (adept-10) )
                            && ch->pcdata->practice > 0 )
                    {
                         found = TRUE;

                         ch->pcdata->practice--;
                         ch->pcdata->learned[sn] += ( int_app[get_curr_stat ( ch, STAT_INT)].learn ) / 3;
                         if ( ch->pcdata->learned[sn] < adept )
                         {
                              act ( "You practice $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
                              form_to_char ( ch, "Practiced %s to %d%%.\n\r", skill_table[sn].name, ch->pcdata->learned[sn] );
                         }
                         else
                         {
                              ch->pcdata->learned[sn] = adept;
                              act ( "You are now learned at $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
                         }
                    }
                    // End of while loop
                    if ( ch->pcdata->practice <= 0 )
                    {
                         send_to_char ( "You have no more practice sessions.\n\r", ch );
                         return;
                    }
               }
               /* end of for loop */
               if ( !found )
                    form_to_char ( ch, "Couldn't find any skills below %d%%.\n\r", (adept-10) );
               else
                    form_to_char ( ch, "You now have %d practice sessions.\n\r", ch->pcdata->practice );

          }
          // end of all prac
     }
     return;
}

void do_learn ( CHAR_DATA * ch, char *argument )
{
     int                 sn;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          show_current_prac ( ch );
     }
     else
     {
          CHAR_DATA          *mob;
          int                 adept;
          if ( !IS_AWAKE ( ch ) )
          {
               send_to_char ( "In your dreams, or what?\n\r", ch );
               return;
          }
          for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
          {
               if ( IS_NPC ( mob ) && IS_SET ( mob->act, ACT_SKILLMASTER ) )
                    break;
          }
          if ( mob == NULL )
          {
               send_to_char ( "You can't do that here.\n\r", ch );
               return;
          }
          /* Zeran - handle "learn list" case here */
          if ( !str_cmp ( argument, "list" ) )
          {
               show_skillmaster_skills ( ch, mob );
               return;
          }
          if ( ch->pcdata->practice <= 0 )
          {
               send_to_char ( "You have no practice sessions left.\n\r", ch );
               return;
          }
          if ( ( sn = skill_lookup ( argument ) ) < 0 || ( !IS_NPC ( ch )
                                                           && ( ( skill_available ( sn, ch, SKILL_LEARN, mob ) == FALSE ))))
          {
               send_to_char ( "You can't learn that.\n\r", ch );
               return;
          }

          adept = IS_NPC ( ch ) ? 100 : class_table[ch->pcdata->pclass].skill_adept;

          if ( ch->pcdata->learned[sn] >= adept )
          {
               form_to_char ( ch, "You can only learn more about %s through experience.\n\r",
                              skill_table[sn].name );
          }
          else
          {
               ch->pcdata->practice--;
               ch->pcdata->learned[sn] += ( int_app[get_curr_stat ( ch, STAT_INT )].learn ) / 3;

               if ( ch->pcdata->learned[sn] <= adept )
               {
                    act ( "$N teaches you more about $t.", ch, skill_table[sn].name, mob, TO_CHAR );
                    act ( "$N teaches $n more about $t.",  ch, skill_table[sn].name, mob, TO_ROOM );
                    form_to_char ( ch, "Learned %s to %d%%.\n\r", skill_table[sn].name, ch->pcdata->learned[sn] );
               }
               else
               {
                    ch->pcdata->learned[sn] = adept;
                    act ( "You are now learned at $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
                    act ( "$n is now learned at $T.",   ch, NULL, skill_table[sn].name, TO_ROOM );
               }
          }
     }
     return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     int                 wimpy;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
          wimpy = ch->max_hit / 5;
     else
          wimpy = atoi ( arg );

     if ( wimpy < 0 )
     {
          send_to_char ( "Your courage exceeds your wisdom.\n\r", ch );
          return;
     }

     if ( wimpy > ch->max_hit / 2 )
     {
          send_to_char ( "Such cowardice ill becomes you.\n\r", ch );
          return;
     }
     if ( wimpy == 0 )
          form_to_char ( ch, "You won't try to flee even if you're dead.\n\r" );
     else
          form_to_char ( ch, "You will now attempt to flee under %d hit points.\n\r", wimpy );
     ch->wimpy = wimpy;
     return;
}

void do_password ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char               *pArg;
     char               *pwdnew;
     char               *p;
     char                cEnd;

     if ( IS_NPC ( ch ) )
          return;
     /*
      * Can't use one_argument here because it smashes case.
      * So we just steal all its code.  Bleagh.
      */
     pArg = arg1;
     while ( isspace ( *argument ) )
          argument++;

     cEnd = ' ';
     if ( *argument == '\'' || *argument == '"' )
          cEnd = *argument++;

     while ( *argument != '\0' )
     {
          if ( *argument == cEnd )
          {
               argument++;
               break;
          }
          *pArg++ = *argument++;
     }
     *pArg = '\0';

     pArg = arg2;
     while ( isspace ( *argument ) )
          argument++;

     cEnd = ' ';
     if ( *argument == '\'' || *argument == '"' )
          cEnd = *argument++;

     while ( *argument != '\0' )
     {
          if ( *argument == cEnd )
          {
               argument++;
               break;
          }
          *pArg++ = *argument++;
     }
     *pArg = '\0';

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char ( "Syntax: password <old> <new>.\n\r", ch );
          return;
     }

     if ( strcmp
          ( crypt ( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
     {
          if ( ( !strcmp ( arg1, "null" ) ) && ( ch->pcdata->pwd[0] == '\0' ) )
          {
               send_to_char ( "Fixing null password. Remember to report this error.\n\r", ch );
          }
          else
          {
               WAIT_STATE ( ch, 40 );
               send_to_char ( "Wrong password.  Wait 10 seconds.\n\r", ch );
               return;
          }
     }

     if ( strlen ( arg2 ) < 5 )
     {
          send_to_char ( "New password must be at least five characters long.\n\r", ch );
          return;
     }

     /*
      * No tilde allowed because of player file format.
      */
     pwdnew = crypt ( arg2, ch->name );
     for ( p = pwdnew; *p != '\0'; p++ )
     {
          if ( *p == '~' )
          {
               send_to_char ( "New password not acceptable, try again.\n\r", ch );
               return;
          }
     }

     free_string ( ch->pcdata->pwd );
     ch->pcdata->pwd = str_dup ( pwdnew );
     save_char_obj ( ch );
     send_to_char ( "Ok.\n\r", ch );
     return;
}

void do_xinfo ( CHAR_DATA * ch, char *argument )
{
     real_auto ( ch, PLR_XINFO, "View Xinfo" );
}

bool is_outside ( CHAR_DATA * ch )
{
     if ( IS_SET ( ch->in_room->room_flags, ROOM_INDOORS ) )
          return FALSE;
     if ( IS_SET ( ch->in_room->sector_type, SECT_INSIDE ) )
          return FALSE;
     return TRUE;
}

void do_verify ( CHAR_DATA *ch, char *argument )
{
     FILE *fp;
     char buf[MSL];
     char cmdbuf[MSL];
     int  vcode;

     if ( IS_NPC ( ch ) )
          return;

     /* Need to handle changing account id's, but not now. */

     if (ch->pcdata->account->status >= ACCT_VERIFIED)
     {
          send_to_char ( "You've already been verified!\n\r", ch );
          return;
     }

     if (argument[0] != '\0')
     {
          if (!is_number (argument) )
          {
               send_to_char ("{RVerify Failed. Please copy the code exactly from the email.{w\n\r", ch);
               return;
          }

          if ( atoi(argument) == ch->pcdata->account->vcode )
          {
               send_to_char ("{GWelcome to VERIFIED status!{w\n\r", ch );
               if (ch->pcdata->account->heroes >= mud.fordemi )
               {
                    send_to_char ( "Since you have 2 Mortal Heroes{w, you may now create\n\r"
                                   "{YDEMI-GOD{w characters. Congratulations!\n\r", ch);
                    ch->pcdata->account->status = ACCT_VERIFIED_DEMISTAT;
               }
               else
               {
                    send_to_char ( "You will need a total of 2 Mortal Heroes before you will be able"
                                   "to create DEMI-GOD characters.\n\r", ch);
                    ch->pcdata->account->status = ACCT_VERIFIED;
               }
               log_string ( "%s:%s Verify Succeeded.", ch->name, ch->pcdata->account->acc_name );
               fwrite_accounts ( ); /* Must always save accounts to avoid loss of data! */
               return;
          }
          send_to_char ("{RVerify Failed. Please copy the code exactly from the email.{w\n\r", ch);
          return;
     }

     if ( ch->pcdata->account->vcode > 0 )
     {
          send_to_char ("You've already requested a validation code!\n\r", ch );
          send_to_char ("If for some reason you didn't receive the email,\n\r"
                        "try again after the next system reboot.\n\r", ch );
          return;
     }

     /* We don't need to be TOO fancy. */
     vcode = number_range (1, 128000);
     /* Not saved! */
     ch->pcdata->account->vcode = vcode;

     send_to_char( "Sending your {GVerification Code{w to the email you provided.\n\r", ch );
     send_to_char( "\n\rYou will need to enter this code to the mud as soon as possible,\n\r", ch );
     send_to_char( "or you will need to request a new code if the mud has rebooted.\n\r", ch );

     if ( ( fp = fopen ( "temptomail", "w+" ) ) == NULL )
     {
          bugf ( "Failed openning temptomail for writing." );
          send_to_char( "Something went wrong. Mail not sent.\n\r", ch );
          return;
     }

     SNP (buf, "Someone has requested a verification code for " TXT_MUDNAME " be sent to this\n"
          "email address. If this was not you, you can safely ignore this message.\n\n"
          "To verify your " TXT_MUDNAME " account, enter the following command exactly as\n"
          "shown into " TXT_MUDNAME ":\n\rverify %d\n\n"
          "Once you have done this, your account will be fully verified.\n"
          "If you do not enter this validation code before the next system reboot,\n"
          "however, you will need to re-request your validation code.\n"
          "Thank you for verifying your email address,\n"
          "\nThe " TXT_MUDNAME " Staff", vcode );
     fprintf ( fp, buf );
     fclose ( fp );

     SNP ( cmdbuf, "cat temptomail | /usr/bin/mail -s \"" TXT_MUDNAME " Verification Request\" %s",
           ch->pcdata->account->acc_name );

     if ( system ( cmdbuf ) == 1 )
     {
          bugf ("Failed to execute command on do_verify: %s", cmdbuf );
          send_to_char ("Failed to execute mail on verify.\n\r", ch);
     }
     else
     {
          log_string ( "Verification %d sent to %s.", vcode, ch->pcdata->account->acc_name );
          send_to_char ("\n\rVerification code sent. Check your email.\n\r", ch );
     }
     return;
}
