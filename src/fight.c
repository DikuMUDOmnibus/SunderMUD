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

#define MAX_DAMAGE_MESSAGE 37

/* command procedures needed */
DECLARE_DO_FUN ( do_emote );
DECLARE_DO_FUN ( do_circle );
DECLARE_DO_FUN ( do_berserk );
DECLARE_DO_FUN ( do_rally );
DECLARE_DO_FUN ( do_bash );
DECLARE_DO_FUN ( do_trip );
DECLARE_DO_FUN ( do_dirt );
DECLARE_DO_FUN ( do_flee );
DECLARE_DO_FUN ( do_kick );
DECLARE_DO_FUN ( do_disarm );
DECLARE_DO_FUN ( do_get );
DECLARE_DO_FUN ( do_recall );
DECLARE_DO_FUN ( do_yell );
DECLARE_DO_FUN ( do_sacrifice );
DECLARE_DO_FUN ( do_rotate );

/*
 * Local functions.
 */
void check_assist    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_dodge     args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_parry     args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_shield    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool avoid_attack    args ( ( CHAR_DATA * ch, CHAR_DATA *victim, OBJ_DATA *wield, int dt ) );
void dam_message     args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune, int where ) );
void death_cry       args ( ( CHAR_DATA * ch ) );
void group_gain      args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
int  xp_compute      args ( ( CHAR_DATA * gch, CHAR_DATA * victim, int avglevel, bool group ) );
bool is_safe         args ( ( CHAR_DATA * ch, CHAR_DATA * victim, bool backtalk ) );
void make_corpse     args ( ( CHAR_DATA * ch ) );
bool one_hit         args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool dual, int skill ) );
void special_one_hit args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
void mob_hit         args ( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
void raw_kill        args ( ( CHAR_DATA * victim ) );
void set_fighting    args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void disarm          args ( ( CHAR_DATA * ch, CHAR_DATA * victim, bool dual ) );
float check_brand    args ( ( CHAR_DATA * ch, char color[3], OBJ_DATA * wep, CHAR_DATA * victim ) );
int  hpart 	     args ( ( CHAR_DATA *ch, CHAR_DATA *vict ) );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * We need to move this off to violence_list or something, so not cycling
 * through all of char_list ...
 */

void violence_update ( void )
{
     CHAR_DATA          *ch;
     CHAR_DATA          *ch_next;
     CHAR_DATA          *victim;
     OBJ_DATA           *obj, *obj_next;
     bool                room_trig = FALSE;

     for ( ch = char_list; ch != NULL; ch = ch_next )
     {
          ch_next = ch->next;

          if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
               continue;

          if ( IS_AWAKE ( ch ) && ch->in_room == victim->in_room )
               multi_hit ( ch, victim, TYPE_UNDEFINED );
          else
               stop_fighting ( ch, FALSE );

          if ( ( victim = ch->fighting ) == NULL )
               continue;

          /*
           * Fun for the whole family!
           */

          check_assist ( ch, victim );

          if ( victim->position <= POS_INCAP && IS_NPC ( ch ) )	/* Stop fighting if victim is incapacitated */
          {

               if ( ch->alignment > -200 &&			/* truly evil won't quit */
                    !IS_SET ( ch->act, ACT_AGGRESSIVE ) )	/* aggro won't stop */
               {
                    stop_fighting ( ch, TRUE );
                    form_to_char ( victim, "%s has left you for dead.\n\r", capitalize ( ch->short_descr ) );
                    continue;
               }
          }

          if ( IS_NPC( ch ) )
          {
               if ( HAS_TRIGGER_MOB( ch, TRIG_FIGHT ) )
                    p_percent_trigger( ch, NULL, NULL, victim, NULL, NULL, TRIG_FIGHT );
               if ( HAS_TRIGGER_MOB( ch, TRIG_HPCNT ) )
                    p_hprct_trigger( ch, victim );
          }
          for ( obj = ch->carrying; obj; obj = obj_next )
          {
               obj_next = obj->next_content;
               if ( obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ( obj, TRIG_FIGHT ) )
                    p_percent_trigger( NULL, obj, NULL, victim, NULL, NULL, TRIG_FIGHT );
          }
          if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_FIGHT ) && room_trig == FALSE )
          {
               room_trig = TRUE;
               p_percent_trigger( NULL, NULL, ch->in_room, victim, NULL, NULL, TRIG_FIGHT );
          }
     }
     return;
}

/* for auto assisting */
void check_assist ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     CHAR_DATA          *rch, *rch_next;

     for ( rch = ch->in_room->people; rch != NULL; rch = rch_next )
     {
          rch_next = rch->next_in_room;

          if ( IS_AWAKE ( rch ) && rch->fighting == NULL )
          {

	    /* quick check for ASSIST_PLAYER */

               if ( !IS_NPC ( ch ) && IS_NPC ( rch )
                    && IS_SET ( rch->off_flags, ASSIST_PLAYERS )
                    && rch->level + 6 > victim->level && victim->hit > 1)
               {
                    do_emote ( rch, "screams and attacks!" );
                    multi_hit ( rch, victim, TYPE_UNDEFINED );
                    continue;
               }

	    /* PCs next */
               if ( !IS_NPC ( ch ) || IS_AFFECTED ( ch, AFF_CHARM ) )
               {
                    if ( ( ( !IS_NPC ( rch ) &&
                             IS_SET ( rch->act, PLR_AUTOASSIST ) ) ||
                           IS_AFFECTED ( rch, AFF_CHARM ) ) &&
                         is_same_group ( ch, rch ) )
                         multi_hit ( rch, victim, TYPE_UNDEFINED );

                    continue;
               }

	    /* now check the NPC cases */

               if ( IS_NPC ( ch ) && !IS_AFFECTED ( ch, AFF_CHARM ) )
               {
                    if ( ( IS_NPC ( rch ) &&
                           IS_SET ( rch->off_flags, ASSIST_ALL ) ) ||
                         ( IS_NPC ( rch ) && rch->race == ch->race &&
                           IS_SET ( rch->off_flags, ASSIST_RACE ) ) ||
                         ( IS_NPC ( rch ) &&
                           IS_SET ( rch->off_flags, ASSIST_ALIGN ) &&
                           ( ( IS_GOOD ( rch ) && IS_GOOD ( ch ) ) ||
                             ( IS_EVIL ( rch ) && IS_EVIL ( ch ) ) ||
                             ( IS_NEUTRAL ( rch ) &&
                               IS_NEUTRAL ( ch ) ) ) ) ||
                         ( rch->pIndexData == ch->pIndexData &&
                           IS_SET ( rch->off_flags, ASSIST_VNUM ) ) )

                    {
                         CHAR_DATA          *vch;
                         CHAR_DATA          *target;
                         int                 number;

                         if ( number_bits ( 1 ) == 0 )
                              continue;

                         target = NULL;
                         number = 0;
                         for ( vch = ch->in_room->people; vch;
                               vch = vch->next )
                         {
                              if ( can_see ( rch, vch )
                                   && is_same_group ( vch, victim )
                                   && number_range ( 0, number ) == 0 )
                              {
                                   target = vch;
                                   number++;
                              }
                         }

                         if ( target != NULL )
                         {
                              do_emote ( rch, "screams and attacks!" );
                              multi_hit ( rch, target, TYPE_UNDEFINED );
                         }
                    }
               }
          }
     }
}

/*
 * Do one group of attacks.
 */
void multi_hit ( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
     OBJ_DATA *wield;
     OBJ_DATA *dwield;
     int primskill, dualskill = 0;
     int ldiff;
     bool primhit = TRUE;
     bool dualhit = TRUE;
     bool dual = FALSE;
     bool NPC = FALSE;

     /* decrement the wait */
     if ( ch->desc == NULL ) ch->wait = UMAX ( 0, ch->wait - PULSE_VIOLENCE );

     /* no attacks for stunnies -- just a check */
     if ( ch->position < POS_RESTING )
          return;

     if ( victim->fighting == NULL )
          set_fighting ( victim, ch );

     if ( IS_NPC ( ch ) )
     {
          NPC = TRUE;
          mob_hit ( ch, victim, dt );
     }

     /* if IS_SET(ch->parts, PART_HOOFS)
      * {
      * special_one_hit( ch, victim, 1035 );
      * if (ch->fighting != victim)
      * return;
      * } */

     if IS_SET ( ch->parts, PART_HORNS )
     {
          special_one_hit ( ch, victim, 1036 );
     }
     else if IS_SET ( ch->parts, PART_WINGS )
     {
          special_one_hit ( ch, victim, 1033 );	/*left wing */
          special_one_hit ( ch, victim, 1034 );	/*right wing */
     }
     else if ( ch->race == race_lookup ( "azer" ) )
     {
          special_one_hit ( ch, victim, 1037 );	/*flaming aura */
     }

    /* Set up the weapons, and check for dual */

     wield = get_eq_char ( ch, WEAR_WIELD );
     dwield = get_eq_char ( ch, WEAR_WIELD2 );

     if (dwield != NULL)
          dual = TRUE;

    /* Set up the skills */

    /* get the weapon skill */

     primskill = get_weapon_skill(ch, get_weapon_sn(ch, FALSE) );

     if (dual)
     {
          int dsn, wskill, dskill;

          dsn = get_weapon_sn(ch, TRUE); /* get the 2cd weaps skill sn */
          wskill = get_weapon_skill (ch, dsn); /* Get the actual skill now */
          dskill = get_skill ( ch, gsn_dual ); /* Get the dual wield skill */

          dualskill = (int) ( wskill * ( (float) dskill/100 ) );
     }

     /* Add Hitrolls */
     if (!NPC)
     {
          primskill += GET_HITROLL(ch) / 2;
          dualskill += GET_HITROLL(ch) / 2;
     }
     else
     {
          primskill += GET_HITROLL(ch) / 3;
          dualskill += GET_HITROLL(ch) / 3;
     }

    /* Let's give level bonuses now!!  - & penalties :) */

     ldiff = abs(ch->level - victim->level);

     if (ch->level > victim->level)
     {
          if (primskill > 0)
               primskill += (ldiff * 1.5);
          if (dualskill > 0)
               dualskill += (ldiff * 1.5);
     }
     else if (ch->level < victim->level)
     {
          primskill -= (ldiff);
          dualskill -= (ldiff);
     }

     if (primskill >= 100)
          primskill = 100;
     if (dualskill >= 100)
          dualskill = 100;
     if (primskill < 0)
          primskill = 0;
     if (dualskill < 0)
          dualskill = 0;

    /* Area attack -- BALLS nasty! */

     if ( NPC && IS_SET ( ch->off_flags, OFF_AREA_ATTACK ) )
     {
          CHAR_DATA *vch;
          CHAR_DATA *vch_next;

          for ( vch = ch->in_room->people; vch != NULL;
                vch = vch_next )
          {
               vch_next = vch->next;
               if ( ( vch != victim && vch->fighting == ch ) )
                    one_hit ( ch, vch, dt, FALSE, primskill );
          }
          if (ch->fighting != victim) return;
     }

     /* No more free ride, must check skills */
     /* Go ahead and give the chance for dual attack too */
     /* Let's give backstabbers their one hit for free, they have to check the rest */
     /* This is due to the fact that they already passed their backstab skill check. Duh. */
     /* Also include circle */

     if (dt == gsn_backstab || dt == gsn_circle )
          primhit = one_hit(ch, victim, dt, FALSE, 999 );
     else
          primhit = one_hit(ch, victim, dt, FALSE, primskill );

     if (ch->fighting != victim) return;

     if ( dual )
     {
          dualhit = one_hit (ch, victim, dt, TRUE, dualskill );
          if (ch->fighting != victim) return;
     }

	/* You flubbed both chances. Go to to next round, directly don't pass go. */

     if (!primhit && !dualhit)
          return;

     if ( IS_AFFECTED ( ch, AFF_HASTE ) || ( ( race_table[ch->race].off & OFF_FAST ) == OFF_FAST )
          || (NPC && IS_SET(ch->off_flags, OFF_FAST) ) )
     {
          if (primhit)
               primhit = one_hit(ch, victim, dt, FALSE, primskill );

          if (ch->fighting != victim)
               return;

		/* If dualing, and hit on primary */

          if (dual && dualhit )
               dualhit = one_hit ( ch, victim, dt, FALSE, dualskill );
     }

     if ( ch->fighting != victim || dt == gsn_backstab )
          return;

     if (!primhit)
          return;

     if ( chance (get_skill(ch, gsn_second_attack) ) )
     {
          if (!one_hit (ch, victim, dt, FALSE, primskill) )
          {
               check_improve (ch, gsn_second_attack, FALSE, 5);
               return;
          }
          else
               check_improve (ch, gsn_second_attack, TRUE, 5);
          if (ch->fighting != victim)
               return;
     }

     if ( chance (get_skill(ch, gsn_third_attack) ) )
     {
          if (!one_hit (ch, victim, dt, FALSE, primskill) )
          {
               check_improve (ch, gsn_third_attack, FALSE, 6);
               return;
          }
          else
               check_improve (ch, gsn_third_attack, TRUE, 6);
          if (ch->fighting != victim)
               return;
     }

     if ( chance (get_skill(ch, gsn_fourth_attack) ) )
     {
          if (!one_hit (ch, victim, dt, FALSE, primskill) )
          {
               check_improve (ch, gsn_fourth_attack, FALSE, 6);
               return;
          }
          else
               check_improve (ch, gsn_fourth_attack, TRUE, 6);
          if (ch->fighting != victim)
               return;
     }

     if ( chance (get_skill(ch, gsn_fifth_attack) ) )
     {
          if (!one_hit (ch, victim, dt, FALSE, primskill) )
          {
               check_improve (ch, gsn_fifth_attack, FALSE, 6);
               return;
          }
          else
               check_improve (ch, gsn_fifth_attack, TRUE, 6);
     }

     return;
}

/* procedure for all mobile attacks */
void mob_hit ( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
     int                number;

     if ( ch->wait > 0 )
          return;

     //    number = number_range ( 0, 2 );
     //    if ( number == 1 && IS_SET ( ch->act, ACT_MAGE ) )
	/*  { mob_cast_mage(ch,victim); return; } */
     //    if ( number == 2 && IS_SET ( ch->act, ACT_CLERIC ) )
	/* { mob_cast_cleric(ch,victim); return; } */

     number = number_range ( 0, 7 );

     switch ( number )
     {
     case ( 0 ):
          if ( IS_SET ( ch->off_flags, OFF_BASH ) )
               do_bash ( ch, "" );
          break;

     case ( 1 ):
          if ( IS_SET ( ch->off_flags, OFF_BERSERK ) &&
               !IS_AFFECTED ( ch, AFF_BERSERK ) )
               do_berserk ( ch, "" );
          break;
     case ( 2 ):
          if ( IS_SET ( ch->off_flags, OFF_DISARM )
               || ( get_weapon_sn ( ch, FALSE ) != gsn_hand_to_hand
                    && ( IS_SET ( ch->act, ACT_WARRIOR )
                         || IS_SET ( ch->act, ACT_THIEF ) ) ) )
               do_disarm ( ch, "" );
          break;

     case ( 3 ):
          if ( IS_SET ( ch->off_flags, OFF_KICK ) )
               do_kick ( ch, "" );
          break;

     case ( 4 ):
          if ( IS_SET ( ch->off_flags, OFF_KICK_DIRT ) )
               do_dirt ( ch, "" );
          break;

     case ( 5 ):
          if ( IS_SET ( ch->off_flags, OFF_TAIL ) )
              /* do_tail(ch,"") */ ;
          break;

     case ( 6 ):
          if ( IS_SET ( ch->off_flags, OFF_TRIP ) )
               do_trip ( ch, "" );
          break;

     case ( 7 ):
          if ( IS_SET ( ch->off_flags, OFF_CRUSH ) )
              /* do_crush(ch,"") */ ;
          break;
     }
}

/*
 * Hit one guy once with special attack.
 */
void special_one_hit ( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
     int                 dam;
     int                 diceroll;
     int                 dam_type;
     int		 skill;
     int		 h;

     /* just in case */
     if ( victim == ch || ch == NULL || victim == NULL )
          return;

     /*
      * Can't beat a dead char!
      * Guard against weird room-leavings.
      */
     if ( victim->position == POS_DEAD ||
          ch->in_room != victim->in_room )
          return;

     /*
      * Figure out the type of damage message.
      */

     dam_type = attack_table[dt - TYPE_HIT].damage;

     /* Fake a skill percentage */

     skill = ( (int) 7 + 1.5 * ch->level );

     if ( !can_see ( ch, victim ) )
          skill /=2;

     if ( victim->position < POS_FIGHTING )
          skill += 25;

     if ( victim->position < POS_RESTING )
          skill += 25;

     if (skill > 100)
          skill = 100;
     if (skill < 1)
          skill = 1;

     if (!chance(skill) )
     {
          /* Miss. */
          damage ( ch, victim, 0, dt, dam_type, TRUE );
          tail_chain (  );
          return;
     }

     /*
      * Hit.
      * Calc damage.
      */

     dam = number_range ( 1 + ch->level / 10, ch->level / 2 );

     if ( IS_SET ( ch->parts, PART_WINGS ) )	/*reduce damage */
          dam = ( dam * .35 );

     /*
      * Standard ROM waited until after calculating the hit, the damage,
      * and all the damage modifiers *BEFORE* bothering to see if the opponent
      * dodged or anything, and in fact, did so in the damage function.
      * We're actually going to bother to check it now. Duh.
      */

     if (avoid_attack(ch, victim, NULL, dt) )
          return; /* Iffy. Character passed, but was dodged. */

     /*
      * Bonuses.
      */

     if ( get_skill ( ch, gsn_enhanced_damage ) > 0 )
     {
          diceroll = number_percent (  );
          if ( diceroll <= get_skill ( ch, gsn_enhanced_damage ) )
          {
               check_improve ( ch, gsn_enhanced_damage, TRUE, 6 );
               dam += dam * diceroll / 100;
          }
     }

     if ( get_skill ( ch, gsn_ultra_damage ) > 0 )
     {
          diceroll = number_percent (  );
          if ( diceroll <= get_skill ( ch, gsn_ultra_damage ) )
          {
               check_improve ( ch, gsn_ultra_damage, TRUE, 6 );
               dam += dam * diceroll / 75;
          }
     }

     if ( !IS_AWAKE ( victim ) )
          dam *= 2;
     else if ( victim->position < POS_FIGHTING )
          dam = dam * 3 / 2;

     if ( dam <= 0 )
          dam = 1;

     /* Find out which bodypart got hit on the victim */

     h = hpart ( ch, victim );
     new_damage ( ch, victim, dam, dt, dam_type, TRUE, FALSE, h );
     tail_chain (  );
     return;
}

/*
Hit one guy once.
*/

bool one_hit ( CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool dual, int skill )
{
     OBJ_DATA           *wield;
     int		 h;
     int		 sn;
     int                 dam;
     int                 diceroll;
     int                 dam_type;
     char                mes[128];

     sn = -1;

     /* just in case */
     if ( victim == ch || ch == NULL || victim == NULL )
          return FALSE;

     /*
      * Can't beat a dead char!
      * Guard against weird room-leavings.
      */
     if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
          return FALSE;

     /*
      * Figure out the type of damage message.
      */
     if ( !dual )
          wield = get_eq_char ( ch, WEAR_WIELD );
     else
          wield = get_eq_char ( ch, WEAR_WIELD2 );

     SNP ( mes, "dt %d", dt );

     if ( dt == TYPE_UNDEFINED )
     {
          dt = TYPE_HIT;
          if ( wield != NULL && wield->item_type == ITEM_WEAPON )
               dt += wield->value[3];
          else
               dt += ch->dam_type;
     }

     SNP ( mes, "dt %d", dt );
     if ( dt < TYPE_HIT )
          if ( wield != NULL )
               dam_type = attack_table[wield->value[3]].damage;
     else
          dam_type = attack_table[ch->dam_type].damage;
     else
          dam_type = attack_table[dt - TYPE_HIT].damage;

     SNP ( mes, "dam type: %d", dam_type );

     // Slowly remove the bonuses so it isn't as obvious
     if ( ch->level < 3 )
          skill +=25;
     else if (ch->level < 5)
          skill +=20;
     else if (ch->level < 7)
          skill +=10;
     else if (ch->level < 9)
          skill +=5;

     if ( dam_type == -1 )
          dam_type = DAM_BASH;

     if ( !can_see ( ch, victim ) )
          skill /=2;

     if ( victim->position < POS_FIGHTING )
          skill += 25;

     if ( victim->position < POS_RESTING )
          skill += 25;

     if (skill > 100)
          skill = 100;
     if (skill < 1)
          skill = 1;

     if (!chance(skill) )
     {
	/* Miss. */
          damage ( ch, victim, 0, dt, dam_type, TRUE );
          tail_chain (  );
          return FALSE;
     }

    /*
     * Standard ROM waited until after calculating the hit, the damage,
     * and all the damage modifiers *BEFORE* bothering to see if the opponent
     * dodged or anything, and in fact, did so in the damage function.
     * We're actually going to bother to check it now. Duh.
     */

     if (avoid_attack(ch, victim, wield, dt) )
          return TRUE; /* Iffy. Character passed, but was dodged. */

    /*
     * Hit.
     * Calc damage.
     */
     if ( IS_NPC ( ch ) && ( !ch->pIndexData->new_format || wield == NULL ) )
          if ( !ch->pIndexData->new_format )
          {
               dam = number_range ( ch->level / 2, ch->level * 3 / 2 );
               if ( wield != NULL )
                    dam += dam / 2;
          }
     else
          dam = dice ( ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE] );
     else
     {
          if (sn != -1)
          {

		/* Before you freak. No, it isn't getting the sn from the wrong weapon. */
		/* Remember, dual is a BOOLEAN, not a PARAMETER */

               sn = get_weapon_sn(ch, dual);
               if (!dual)
                    check_improve(ch,sn,TRUE,5);
               else
                    check_improve(ch,gsn_dual,TRUE, 3) ;
          }

          if ( wield != NULL )
          {
               if ( wield->pIndexData->new_format )
                    dam = dice ( wield->value[1], wield->value[2] ) * skill / 100;
               else
                    dam = number_range ( wield->value[1] * skill / 100, wield->value[2] * skill / 100 );
               dam = dam * wield->condition / 100;
               if ( get_eq_char ( ch, WEAR_SHIELD ) == NULL )	/* no shield = more */
                    dam = dam * 21 / 20;
          }
          else
          {    // This added because hand to hand was virtually useless before.
               if ( get_weapon_skill ( ch, gsn_hand_to_hand ) > 1 )
               {
                    if ( !IS_NPC ( ch ) && ch->pcdata->pclass == class_lookup ( "monk" ) )
                         dam = number_range ( 1 + 4 * skill / 100, 3 * ch->level + skill );
                    else
                         dam = number_range ( 1 + 4 * skill / 100, ch->level + skill );
               }
               else
                    dam = number_range ( 1, ch->level / 2 );
          }
     }

    /*
     * Bonuses.
     */

     if ( ch->level <= 3 ) dam = ( dam * 1.3 );  /* increase damage done at low level */
     if ( ch->level <= 5 ) dam = ( dam * 1.25 ); /* Gradually remove the damn bonus */
     if ( ch->level <= 7 ) dam = ( dam * 1.15 ); /* So the whiners will quit noticing that */
     if ( ch->level <= 10) dam = ( dam * 1.05 ); /* They lost something, and bitch. */

     if ( get_skill ( ch, gsn_enhanced_damage ) > 0 )
     {
          diceroll = number_percent (  );
          if ( diceroll <= get_skill ( ch, gsn_enhanced_damage ) )
          {
               check_improve ( ch, gsn_enhanced_damage, TRUE, 6 );
               dam += dam * diceroll / 100;
          }
     }

     if ( get_skill ( ch, gsn_ultra_damage ) > 0 )
     {
          diceroll = number_percent (  );
          if ( diceroll <= get_skill ( ch, gsn_ultra_damage ) )
          {
               check_improve ( ch, gsn_ultra_damage, TRUE, 7 );
               dam += 2.0 * ( dam * diceroll / 300 );
          }
     }

     if ( !IS_AWAKE ( victim ) )
          dam *= 2;
     else if ( victim->position < POS_FIGHTING )
          dam = dam * 3 / 2;

     if ( dt == gsn_backstab && wield != NULL )
     {
          if ( wield->value[0] != 2 )
               dam *= 2 + ch->level / 10;
          else
               dam *= 2 + ch->level / 8;
     }

     if ( dt == gsn_circle && wield != NULL )
     {
          if ( wield->value[0] != 2 )
               dam *= 2 + ( ch->level / 40 );
          else
               dam *= 2 + ( ch->level / 33 );
     }

     dam += GET_DAMROLL ( ch ) * UMIN ( 100, skill ) / 100;

     if ( dam <= 0 ) dam = 1;

     /* Find out which bodypart got hit on the victim */
     h = hpart ( ch, victim );

     // Remember, dual is a bool not an argument
     new_damage ( ch, victim, dam, dt, dam_type, TRUE, dual, h );

     if (chance(10) )
          sound ("sword1.wav", ch);

     tail_chain (  );

     return TRUE;
}

/*
 * Zeran - I call this function ass backwards, with ch and vict
 * reversed so I wrote this function with that in mind.  All
 * references to ch and vict will appear to be backwards.
 *
 * Hey Z, wtf did you send color[3] as an option? It isn't even used elsewhere!
 */
float check_brand ( CHAR_DATA * ch, char color[3], OBJ_DATA * wep, CHAR_DATA * victim )
{
     int                 level;
     int                 brand;
     float               total_mult = 0;
     int                 brand_type;
     int                 dam_type = -1;
     int                 immune;
     AFFECT_DATA         af;

     if ( wep == NULL )		/*no weapon, don't check */
          return ( 1 );

     for ( brand_type = 1; brand_type <= MAX_BRAND;
           brand_type = brand_type * 2 )
     {
          brand = wep->value[4] & brand_type;	/*check for brand_type */
          switch ( brand )
          {			/*check elemental brands first */
          case ( WEAPON_FLAMING ):
               {
                    AFFECT_DATA         af;

                    dam_type = DAM_FIRE;
                    SLCPY ( color, "{r" );
                    /* check smoke blindness */
                    if ( number_percent (  ) < ( wep->level / 3 ) )
                    {
                         act ( "$n's weapon spews forth a blast of acrid {Dsmoke{x!", victim, NULL, NULL, TO_ROOM );
                         send_to_char ( "Your weapon spews forth a blast of acrid {Dsmoke{x!\n\r", victim );
                         if ( !IS_AFFECTED ( ch, AFF_BLIND ) &&
                              ( !saves_spell ( wep->level, ch ) ) )
                         {
                              af.where = TO_AFFECTS;
                              af.type = gsn_blindness;
                              af.level = wep->level;
                              af.duration = number_range ( 2, 4 );
                              af.location = APPLY_HITROLL;
                              af.modifier = -4;
                              af.bitvector = AFF_BLIND;
                              affect_to_char ( ch, &af );
                              send_to_char ( "You are blinded by the smoke!\n\r", ch );
                              act ( "$n appears to be blinded by smoke!", ch, NULL, NULL, TO_ROOM );
                         }
                    }
                    break;
               }
          case ( WEAPON_FROST ):
               {
                    int sn = skill_lookup ( "chill touch" );
                    dam_type = DAM_COLD;
                    SLCPY ( color, "{b" );
                    if ( number_percent (  ) < ( wep->level / 3 ) )
                    {
                         act ( "$n's weapon suddenly radiates a bone-chilling, frigid aura!", victim, NULL, NULL, TO_ROOM );
                         send_to_char ( "Your weapon radiates an {Bicy{x aura!\n\r", victim );
                         if ( !is_affected ( ch, sn ) && ( !saves_spell ( wep->level, ch ) ) )
                         {
                              af.where = TO_AFFECTS;
                              af.type = sn;
                              af.level = wep->level;
                              af.duration = number_range ( 2, 4 );
                              af.location = APPLY_STR;
                              af.modifier = -( wep->level / 12 );
                              af.bitvector = 0;
                              affect_to_char ( ch, &af );
                              send_to_char ( "You are chilled to the core by the extreme cold!\n\r", ch );
                              act ( "$n appears to be shivering violently.", ch, NULL, NULL, TO_ROOM );
                         }
                    }
                    break;
               }
          case ( WEAPON_LIGHTNING ):
               {
                    dam_type = DAM_LIGHTNING;
                    SLCPY ( color, "{y" );
                    if ( ( number_percent (  ) < ( wep->level / 3 ) )
                         && ( !saves_spell ( wep->level, ch ) ) )
                    {
                         act ( "$n's weapon {Yshocks{x $N!", victim,
                               NULL, ch, TO_NOTVICT );
                         send_to_char ( "Your weapon {Yshocks{x your enemy with an electrical charge!\n\r", victim );
                         act ( "$N's weapon has {Yshocked{x you!  You are numbed by the blast.", ch, NULL, victim, TO_CHAR );
                         WAIT_STATE ( ch, ( 2 * PULSE_VIOLENCE ) );
                    }
                    break;
               }
          case ( WEAPON_ACID ):
               {
                    dam_type = DAM_ACID;
                    SLCPY ( color, "{g" );
                    break;
               }
          case ( WEAPON_VAMPIRIC ):
               {
                    int                 dam_total;

                    dam_type = DAM_NEGATIVE;
                    SLCPY ( color, "{m" );
                    if ( number_percent (  ) < ( wep->level / 3 ) )
                    {
                         send_to_char ( "Your weapon pulses with a sickly, {mpurple{x light!\n\r", victim );
                         act ( "$n's weapon pulses with a sickly {mpurple{x light!", victim, NULL, NULL, TO_ROOM );
                         if ( !saves_spell ( wep->level, ch ) )
                         {
                              dam_total = number_range ( ( wep->level / 2 ), ( wep->level ) );
                              act ( "$n looks drained!", ch, NULL, NULL, TO_ROOM );
                              send_to_char ( "You feel drained!\n\r", ch );
                              victim->hit += dam_total;
                              victim->mana += ( dam_total / 2 );
                              ch->mana -= ( dam_total / 2 );
                              ch->hit -= dam_total;
                         }
                    }
                    break;
               }
          case ( WEAPON_SHARP ):
               {
                    if ( number_percent (  ) < ( wep->level / 3 ) )
                    {
                         total_mult = total_mult + 1;
                         SLCPY ( color, "{c" );
                         act ( "Your weapon strikes $N with additional power!", victim, NULL, ch, TO_CHAR );
                         act ( "$n's weapon strikes you with added force!", victim, NULL, ch, TO_VICT );
                    }
                    dam_type = -1;
                    break;
               }
          case ( WEAPON_VORPAL ):
               {
                    if ( number_percent (  ) <
                         ( int ) ( wep->level / 4 ) )
                         total_mult = total_mult + 2;	/*damn, that hurts */
                    dam_type = -1;
                    SLCPY ( color, "{C" );
                    break;
               }

               /* Bad Hack - Lotherius - Really bad hack... SN is probably wrong value */
               /* Zeran - Yes, Lotherius, SN is wrong, just want gsn_poison, not the SN. */
          case ( WEAPON_POISON ):
               {
                    /* sn = -1;
                     * sn = get_weapon_sn(victim); */
                    level = wep->level;
                    if ( IS_AFFECTED ( ch, AFF_POISON ) )
                    {
                         act ( "$n is already poisoned.", ch, NULL,
                               victim, TO_VICT );
                    }
                    else
                    {
                         if ( saves_spell ( level, ch ) )
                         {
                              act ( "$n turns slightly green, but it passes.", ch, NULL, NULL, TO_ROOM );
                              send_to_char ( "You feel momentarily ill, but it passes.\n\r", ch );
                              dam_type = -1;
                         }
                         else
                         {
                              af.type = gsn_poison;
                              af.level = level;
                              af.duration = level;
                              af.location = APPLY_STR;
                              af.modifier = -2;
                              af.bitvector = AFF_POISON;
                              affect_join ( ch, &af );
                              send_to_char ( "You feel a burning sensation from the weapon's cut.\n\r", ch );
                              act ( "$n looks very ill.", ch, NULL, NULL, TO_ROOM );
                              dam_type = -1;
                         }
                    }
                    if ( number_percent (  ) < 5 )	/*Zeran - poison wears off */
                    {
                         send_to_char( "The vemom on the blade dries up.\n\r", victim );
                         wep->value[4] -= WEAPON_POISON;
                    }
                    break;
               }
          default:
               {
                    dam_type = -1;
                    break;
               }
          };			/*end switch */

          if ( dam_type != -1 )	/*if has a recognized elemental brand */
          {
               immune = check_immune ( ch, dam_type );

               switch ( immune )
               {
               case IS_IMMUNE:
                    {
                         total_mult = total_mult;
                         break;
                    }
               case IS_RESISTANT:
                    {
                         total_mult = total_mult + .1;
                         break;
                    }
               case IS_VULNERABLE:
                    {
                         total_mult = total_mult + .5;
                         break;
                    }
               default:
                    {
                         total_mult = total_mult + .25;
                         break;
                    }
               };			/*end switch */
          }
     }
     /*end for */
     return ( total_mult + 1 );
}
/* end */

/*
 * Inflict damage from a hit.
 */

// Wrapper for old code
//
bool damage ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show )
{
     // Default to dual=FALSE and where = -1 ( no location specified )
     return new_damage ( ch, victim, dam, dt, dam_type, show, FALSE, -1 );
}

/*
 * New damage routine, takes options for dual wield and hit location
 */

bool new_damage ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show, bool dual, int where )
{
     OBJ_DATA           *corpse;
     bool                immune;
     OBJ_DATA           *wield;
     char                color[3];
     int		 diehp;
     int 		 victim_ac;
     int 		 reduce = 0;
     float               brand_mult = 0;

     /*bad hack for dual wield*/
     //     if ( dt > 5000 )		/*subtract 5000 and is dual wield attack */
     //     {
     //          wield = get_eq_char ( ch, WEAR_WIELD2 );
     //          dt -= 5000;
     //     }
     //     else
     //          wield = get_eq_char ( ch, WEAR_WIELD );
     //
     // Set which weapon we're using
     if ( dual )
          wield = get_eq_char ( ch, WEAR_WIELD2 );
     else
          wield = get_eq_char ( ch, WEAR_WIELD );

     if ( victim->position == POS_DEAD )
          return FALSE;

     /*
      * Stop up any residual loopholes.
      */
     if ( dam > 9000 )
     {
          bugf ( "Damage: %d: more than 9000 points! (%s)", dam, ch->name );
          dam = 9000;
     }

    /* damage reduction */

     /* This *is* needed, do not remove. */
     /* I dunno about that, but .... I modified the values some - Lotherius */

     if ( dam > 100 )
          dam = ( dam - 100 ) / 2 + 100;
     if ( dam > 225 )
          dam = ( dam - 225 ) / 2 + 225;

     if ( victim != ch )
     {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
          if ( is_safe ( ch, victim, TRUE ) )
               return FALSE;

          if ( victim->position > POS_STUNNED )
          {
               if ( victim->fighting == NULL )
               {
                    set_fighting ( victim, ch );
               }
               if ( victim->timer <= 4 )
                    victim->position = POS_FIGHTING;
          }

          if ( victim->position > POS_STUNNED )
          {
               if ( ch->fighting == NULL )
               {
                    set_fighting ( ch, victim );
                    if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_KILL ) )
                         p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_KILL );
               }

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
               if ( IS_NPC ( ch )
                    && IS_NPC ( victim )
                    && IS_AFFECTED ( victim, AFF_CHARM )
                    && victim->master != NULL
                    && victim->master->in_room == ch->in_room
                    && number_bits ( 3 ) == 0 )
               {
                    stop_fighting ( ch, FALSE );
                    multi_hit ( ch, victim->master, TYPE_UNDEFINED );
                    return FALSE;
               }
          }

	/*
	 * More charm stuff.
	 */
          if ( victim->master == ch )
               stop_follower ( victim );
     }

    /*
     * Inviso attacks ... not.
     */
     if ( IS_AFFECTED ( ch, AFF_INVISIBLE ) )
     {
          affect_strip ( ch, gsn_invis );
          affect_strip ( ch, gsn_mass_invis );
          REMOVE_BIT ( ch->affected_by, AFF_INVISIBLE );
          act ( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
     }
     if ( IS_AFFECTED ( ch, AFF_POLY ) )
     {
          affect_strip ( ch, skill_lookup ( "mask self" ) );
          REMOVE_BIT ( ch->affected_by, AFF_POLY );
          undo_mask ( ch );
          act ( "$n suddenly appears as $s mask vanishes.", ch, NULL, NULL, TO_ROOM );
     }

     /*
     * Damage modifiers.
     */

     /* We want to damage equipment before reducing damage */
     /* Because even tho victim may be immune, eq is not */

     if (dam)
          check_damage_obj ( victim, NULL, 5, dam_type );       /*beat up enemy equipment */

	/* Weoops! We were dinging up swords when casting spells?? */
     if ( (dt < TYPE_HIT) && (dam) )
     {
          check_damage_obj ( ch, wield, 1, dam_type );      /*ding up sword */
     }

     /* Spells will hit AC modifiers first as this is farthest out */
     switch(dam_type)
     {
     case(DAM_PIERCE):
          victim_ac = c_current_ac ( victim, where, AC_PIERCE );
          break;
     case(DAM_BASH):
     case(DAM_HANDTOHAND):
          victim_ac = c_current_ac ( victim, where, AC_BASH );
          break;
     case(DAM_SLASH):
          victim_ac = c_current_ac ( victim, where, AC_SLASH );
          break;
     case(DAM_FIRE):
     case(DAM_ACID):
          victim_ac = c_current_ac ( victim, where, AC_EXOTIC );
          break;
     case(DAM_NEGATIVE):
     case(DAM_HOLY):
     case(DAM_ENERGY):
     case(DAM_MENTAL):
     case(DAM_DROWNING):
     case(DAM_LIGHT):
     case(DAM_HARM):
          victim_ac = c_current_ac ( victim, where, AC_EXOTIC ) / 2;
     default:
          victim_ac = c_current_ac ( victim, where, AC_EXOTIC ) / 1.5;
     };

     /* Give lowbies an AC bonus here */
     /* Again, have to gradually take it away due to bitchers who */
     /* Would notice otherwise that they lost something */
     /* Something they don't really deserve anyway */

     if (victim->level <= 5)
          victim_ac += 10;
     else if (victim->level <= 7)
          victim_ac += 7;
     else if (victim->level <= 10)
          victim_ac += 5;
     else if (victim->level <= 12)
          victim_ac += 3;

     if (victim_ac > 0) // Why the hell is float repeated below?
          reduce = dam * (float) ( (float) (float) abs(victim_ac) / 100 );

     /* We will always give *something* if the player has armor. */
     if ( reduce <= 0 && victim_ac > 0 )
          reduce += 1;

     /* Okay let's slowly reduce the effectiveness of AC reduction as a player levels beyond 10 */

     if (victim->level >= 11 && reduce > 1)
     {

          float under = 101;

          under -= victim->level;

          if (under <= 10)
               under = 10;

          reduce = reduce * (float) ( under/100 );
     }

     if (reduce >= dam && dam > 0 && show)
     {
          dam = 0;
          act( "Your armor completely absorbed $n's attack!", ch, NULL, victim, TO_VICT );
          act( "$N's armor completely absorbed your attack!", ch, NULL, victim, TO_CHAR );
          return TRUE;
     }
     else if (dam > 0)
          dam -= reduce;	// Dam is going to be greater than 0 here cuz <0 was checked above.

     if ( IS_PROTECTED ( victim, PROT_SANCTUARY ) )
          dam /= 3;
     if ( IS_PROTECTED ( victim, PROT_EVIL ) && IS_EVIL ( ch ) )
          dam -= dam / 4;
     else if ( IS_PROTECTED ( victim, PROT_GOOD ) && IS_GOOD ( ch ) )
          dam -= dam / 4;

     if ( is_affected ( victim, skill_lookup ( "fire shield" ) ) )
     {
          if ( dam_type == DAM_COLD )
               dam -= dam / 2;
     }
     immune = FALSE;

     switch ( check_immune ( victim, dam_type ) )
     {
     case ( IS_IMMUNE ):
          immune = TRUE;
          dam = 0;
          break;
     case ( IS_RESISTANT ):
          dam -= dam / 3;
          break;
     case ( IS_VULNERABLE ):
          dam += dam / 2;
          break;
     }

     /* Zeran - do weapon branding check and check material vulnerability */
     if ( ( !immune ) && ( dt < 1033 ) && ( dt > TYPE_HIT ) && ( dam ) )
          /*not immune, is physical, non-special attack */
     {
          if ( wield )
          {
               brand_mult = check_brand ( victim, color, wield, ch );
               dam = ( int ) ( dam * brand_mult );

               if ( brand_mult > 6.2 )	/*how???? */
                    bugf ( "brand_mult > 6 (%d found)", brand_mult );
               if ( check_material_vuln ( wield, victim ) )
                    dam *= 1.5;
          }
     }

     if (show)
          dam_message ( ch, victim, dam, dt, immune, where );

     if ( IS_NPC ( ch ) && IS_NPC ( victim ) && dam <= 0 )   // Bad hack to make sure 2 mobs don't fight forever.
     {
          if ( ch->master == NULL && victim->master == NULL ) // Make sure players can't easily use this against pets
               dam = ch->level;
     }

     if ( dam == 0 )
          return FALSE;

     /*
      * Hurt the victim.
      * Inform the victim of his new state.
      */

     victim->hit -= dam;
     if ( !IS_NPC ( victim ) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 )
          victim->hit = 1;
     update_pos ( victim );

     switch ( victim->position )
     {
     case POS_MORTAL:
          act ( "$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM );
          diehp = (victim->perm_stat[STAT_CON] * 1.1)+victim->hit;
          send_to_char( "You are mortally wounded, and will die soon, if not aided.\n\r", victim);
          form_to_char ( victim, "You can stand to lose %d more hp before death takes you.\n\r", diehp);
          send_to_char ( "If you wish to go ahead and die, type SLEEP. You may also yell for help.\n\r", victim );
          break;

     case POS_INCAP:
          act ( "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM );
          diehp = (victim->perm_stat[STAT_CON] * 1.1)+victim->hit;
          send_to_char ("You are incapacitated, and will slowly die, if not aided.\n\r", victim);
          send_to_char ( "If you wish to go ahead and die, type SLEEP. You may also yell for help.\n\r", victim );
          form_to_char ( victim, "You can stand to lose %d more hp before death takes you.\n\r", diehp);
          break;

     case POS_STUNNED:
          act ( "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM );
          send_to_char ( "You are stunned, but will might recover.\n\r", victim );
          break;

     case POS_DEAD:
          act ( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
          send_to_char ( "{RYou have been {YKILLED{M!!{x\n\r\n\r", victim );
          break;

     default:
          if ( dam > victim->max_hit / 4 )
               send_to_char ( "{YThat really did HURT!{x\n\r", victim );
          if ( victim->hit < victim->max_hit / 5 )
               send_to_char ( "You sure are{R BLEEDING!{x\n\r", victim );
          break;
     }

    /*
     * Sleep spells and extremely wounded folks.
     */
     if ( !IS_AWAKE ( victim ) )
          stop_fighting ( victim, FALSE );

    /*
     * Payoff for killing things.
     */
     if ( victim->position == POS_DEAD )
     {
          char nbuf[MSL];
          group_gain ( ch, victim );
          if ( IS_NPC ( victim ) && !IS_NPC ( ch ) )
          {
               SNP ( nbuf, "%s killed by %s at %d",
                     victim->short_descr, ch->name, victim->in_room->vnum );
               notify_message ( NULL, WIZNET_MOBDEATH, TO_IMM, nbuf );
          }
          if ( !IS_NPC ( victim ) )
          {
               log_string ( "%s killed by %s at %d", victim->name,
                            ( IS_NPC ( ch ) ? ch->short_descr : ch->name ), victim->in_room->vnum );
               /* Zeran - notify message */
               notify_message ( victim, NOTIFY_DEATH, TO_ALL, ( IS_NPC ( ch ) ? ch->short_descr : ch->name ) );
               notify_message ( NULL, WIZNET_DEATH, TO_IMM, nbuf );

               if (!IS_NPC(victim) )
                    victim->pcdata->mob_losses +=1;

               if ( IS_NPC ( ch ) && !IS_NPC ( victim ) )
               {

                    /* If you die to lower level mob, hurts like hell */
                    if (victim->level > ch->level)
                    {
                         victim->pcdata->mob_rating -= ( (victim->level - ch->level) * 10) +10;
                    }
                    else
                    {
                         int ldiff;
                         int change;

                         ldiff = (ch->level - victim->level);

                         change = 35 - ldiff;
                         if (change < 5)
                              change = 5;
                         victim->pcdata->mob_rating -= change;
                    }
               }

               /*
                * Dying penalty:
                * 1/2 way back to previous level.
                */
               if ( victim->level < 25 )
               {
                    if ( victim->exp >
                         exp_per_level ( victim, victim->pcdata->points ) * victim->level )
                         gain_exp ( victim, ( exp_per_level ( victim, victim->pcdata->points )
                                              * victim->level - victim->exp ) / 2 );
               }
               else
               {
                    gain_exp ( victim, -500 );
                    if ( victim->exp <
                         exp_per_level ( victim, victim->pcdata->points ) * victim->level )
                         drop_level ( victim );
               }
          }

          /*
           * Death trigger
           */
          if ( IS_NPC( victim ) && HAS_TRIGGER_MOB( victim, TRIG_DEATH) )
          {
               victim->position = POS_STANDING;
               p_percent_trigger( victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH );
          }
          else
          {
               death_cry(victim);
          }

          raw_kill ( victim );

          /* RT new auto commands */

          if ( !IS_NPC ( ch ) && IS_NPC ( victim ) )
          {
               corpse = get_obj_list ( ch, "corpse", ch->in_room->contents );

               if ( IS_SET ( ch->act, PLR_AUTOLOOT ) && corpse && corpse->contains )	/* exists and not empty */
                    do_get ( ch, "all corpse" );

               if ( IS_SET ( ch->act, PLR_AUTOGOLD ) && corpse && corpse->contains &&	/* exists and not empty */
                    !IS_SET ( ch->act, PLR_AUTOLOOT ) )
                    do_get ( ch, "gold corpse" );

               if ( IS_SET ( ch->act, PLR_AUTOSAC ) )
               {
                    if ( IS_SET ( ch->act, PLR_AUTOLOOT ) && corpse &&
                         corpse->contains )
                         return TRUE;	/* leave if corpse has treasure */
                    else
                         do_sacrifice ( ch, "corpse" );
               }
          }

          return TRUE;
     }

     if ( victim == ch )
          return TRUE;

     /*
      * Take care of link dead people.
      */
     if ( !IS_NPC ( victim ) && victim->desc == NULL )
     {
          if ( number_range ( 0, victim->wait ) == 0 )
          {
               do_recall ( victim, "" );
               return TRUE;
          }
     }

     /*
      * Wimp out?
      */
     if ( IS_NPC ( victim ) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2 )
     {
          if ( ( IS_SET ( victim->act, ACT_WIMPY ) &&
                 number_bits ( 2 ) == 0 &&
                 victim->hit < victim->max_hit / 5 ) ||
               ( IS_AFFECTED ( victim, AFF_CHARM ) &&
                 victim->master != NULL &&
                 victim->master->in_room != victim->in_room ) )
               do_flee ( victim, "" );
     }

     if ( !IS_NPC ( victim )
          && victim->hit > 0
          && victim->hit <= victim->wimpy
          && victim->wait < PULSE_VIOLENCE / 2 )
          do_flee ( victim, "" );

     tail_chain (  );
     return TRUE;
}

/*
 * Not currently setup for NPC use.
 */

void drop_level ( CHAR_DATA * ch )
{
     int                 add_mana;
     int                 add_move;
     int                 add_prac;
     int                 add_hp;
     
     if ( IS_NPC ( ch ) )
          return;

     ch->level -= 1;

     add_hp = con_app[get_curr_stat ( ch, STAT_CON )].hitp + number_range ( class_table[ch->pcdata->pclass].hp_min,
                                                                            class_table[ch->pcdata->pclass].hp_max );
     add_mana = number_range ( 2, ( 2 * get_curr_stat ( ch, STAT_INT ) + get_curr_stat ( ch, STAT_WIS ) ) / 5 );

     if ( !class_table[ch->pcdata->pclass].fMana )
          add_mana /= 2;
     add_move = number_range ( 1, ( get_curr_stat ( ch, STAT_CON ) + get_curr_stat ( ch, STAT_DEX ) ) / 6 );
     add_prac = wis_app[get_curr_stat ( ch, STAT_WIS )].practice;

     add_hp = add_hp * 9 / 10;
     add_mana = add_mana * 9 / 10;
     add_move = add_move * 9 / 10;

     add_hp = UMAX ( 1, add_hp );
     add_mana = UMAX ( 1, add_mana );
     add_move = UMAX ( 6, add_move );

     /* start subtracting here */

     ch->max_hit -= add_hp;
     ch->max_mana -= add_mana;
     ch->max_move -= add_move;
     ch->pcdata->practice -= add_prac;
     ch->pcdata->train -= 1;

     ch->pcdata->perm_hit -= add_hp;
     ch->pcdata->perm_mana -= add_mana;
     ch->pcdata->perm_move -= add_move;

     // This fixes the losing a level but you just permadeathed problem :) - Lotherius
     if ( mud.death > PERMADEATH )
     {
          send_to_char ( "{RYou have lost a LEVEL!!!!!!{x", ch );
          form_to_char ( ch, "Your loss is: {Y-%d/%d mana, -%d/%d mv, -%d/%d prac.{x\n\r",
                         add_mana, ch->max_mana,
                         add_move, ch->max_move, add_prac, ch->pcdata->practice );
     }
     return;
}

// I screwed this function up royally. Therefore, I borrowed -- and then modified
// the function from Rot... Not sure how similar it is to the original ROM function -- Lotherius
// Gets a little harrier the way I'm working the PK Rules.
// Also, is_safe gives a message when TRUE, is silent when FALSE.
//
// If "backtalk" is TRUE, messages will be returned. On FALSE, no messages.
//
bool is_safe ( CHAR_DATA * ch, CHAR_DATA * victim, bool backtalk )
{
     if (victim->in_room == NULL || ch->in_room == NULL)
     {
          bugf ( "Got NULL room on is_safe, bailing." );
          return TRUE;
     }
     if (victim->fighting == ch || victim == ch)
          return FALSE;
     if (!IS_NPC(ch) && IS_IMMORTAL(ch))
          return FALSE;

     /* killing mobiles */
     if (IS_NPC(victim))
     {
          /* safe room? */
          if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
          {
               if ( backtalk )
                    send_to_char( "This room is protected by Divine forces..\n\r",ch);
               return TRUE;
          }
          if (victim->pIndexData->pShop != NULL)
          {
               if ( backtalk )
                    send_to_char ( "Once, all the shopkeepers in the land were killed, but then the economy crashed.\n\r", ch );
               return TRUE;
          }

          /* no killing healers, trainers, etc */
          if (IS_SET(victim->act,ACT_SKILLMASTER) ||  IS_SET(victim->act,ACT_PRACTICE)
              ||  IS_SET(victim->act,ACT_IS_HEALER) )
          {
               if ( backtalk )
                    act("I don't think Zeran would approve.", ch, NULL, NULL, TO_CHAR);
               return TRUE;
          }
          if (!IS_NPC(ch))
          {
               if ( victim->fighting != NULL && !is_same_group ( ch, victim->fighting ) )
               {
                    if ( backtalk )
                         send_to_char ( "Someone else is fighting them already.\n\r", ch );
                    return TRUE;
               }
               /* no pets */
               if (IS_SET(victim->act,ACT_PET))
               {
                    if ( backtalk )
                         act("But $N looks so cute and cuddly...", ch,NULL,victim,TO_CHAR);
                    return TRUE;
               }
               /* no charmed creatures unless owner */
               if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
               {
                    if ( backtalk )
                         send_to_char("You don't own that monster.\n\r",ch);
                    return TRUE;
               }
          }
     }
     /* killing players yay */
     else
     {
          /* NPC doing the killing */
          if (IS_NPC(ch))
          {
               /* safe room check */
               if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
               {
                    if ( backtalk )
                         send_to_char("Not in this room.\n\r",ch);
                    return TRUE;
               }

               /* charmed mobs and pets cannot attack players while owned */
               if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL &&  ch->master->fighting != victim)
               {
                    if ( backtalk )
                         send_to_char("Players are your friends!\n\r",ch);
                    return TRUE;
               }
          }
          /* player doing the killing */
          else
          {
               if (IS_SET(victim->act,PLR_THIEF) ) // Thieves are always likely targets, regardless of level
                    return FALSE;
               if ( victim->fighting != NULL && !is_same_group ( ch, victim->fighting ) )
               {
                    if ( backtalk )
                         send_to_char ("Let them finish the fight they're already in first!\n\r", ch );
                    return TRUE;
               }
               if (ch->level < 5 )
               {
                    if ( backtalk )
                         send_to_char ("You must attain level 5 before you can PKill.\n\r", ch );
                    return TRUE;
               }
               if ( victim->level < 5 )
               {
                    if ( backtalk )
                         send_to_char ("Leave them alone, they aren't level 5 yet.\n\r", ch );
                    return TRUE;
               }
               if (is_same_clan(ch,victim))
               {
                    if ( backtalk )
                         send_to_char("You can't fight your own clan members.\n\r",ch);
                    return TRUE;
               }
               if (ch->level > victim->level + 10)
               {
                    if ( backtalk )
                         send_to_char("Pick on someone your own size.\n\r",ch);
                    return TRUE;
               }
               if (ch->level < victim->level - 10)
               {
                    if ( backtalk )
                         send_to_char("Pick on someone your own size.\n\r",ch);
                    return TRUE;
               }
               if ( !ch->pcdata->clan && !IS_SET ( ch->act, PLR_KILLER ) )
               {
                    if ( backtalk )
                         send_to_char ("You'd do best to leave the killing to the Killers and Clans.\n\r", ch );
                    return TRUE;
               }
               if ( ch->pcdata->clan )
               {
                    if (!ch->pcdata->clan->pkallow)
                    {
                         if ( backtalk )
                              send_to_char("Your clan does not allow player fighting.\n\r",ch);
                         return TRUE;
                    }
               }
               if ( victim->pcdata->clan )
               {
                    if ( !victim->pcdata->clan->pkallow )
                    {
                         if ( backtalk )
                              send_to_char("They are in a no pkill clan, leave them alone.\n\r",ch);
                         return TRUE;
                    }
               }
               if (IS_SET(victim->in_room->room_flags,ROOM_SAFE) || IS_SET(ch->in_room->room_flags, ROOM_SAFE))
               {
                    if ( backtalk )
                         send_to_char("Not in this room.\n\r",ch);
                    return TRUE;
               }
               if (victim->pcdata->questgiver != NULL )
               {
                    if ( backtalk )
                         send_to_char("They are on a quest, leave them alone.\n\r",ch);
                    return TRUE;
               }
               if ( IS_SET (ch->act, PLR_KILLER) && IS_SET ( victim->act, PLR_KILLER ) )
                    return FALSE;
               if ( IS_SET (ch->act, PLR_KILLER) )
               {
                    if ( victim->pcdata->clan )
                    {
                         if ( victim->pcdata->clan->pkallow && ( IS_SET ( victim->in_room->room_flags, ROOM_PKILL ) &&
                                                                 IS_SET ( ch->in_room->room_flags, ROOM_PKILL ) ) )
                              return FALSE;
                         else
                         {
                              if ( backtalk )
                                   send_to_char ( "Sorry, this isn't a PKill room.\n\r", ch );
                              return TRUE;
                         }
                    }
                    else if ( !IS_SET ( victim->act, PLR_KILLER ) )
                    {
                         if ( backtalk )
                              send_to_char ("They aren't a killer.\n\r", ch );
                         return TRUE;
                    }
               }
               if ( IS_SET ( victim->act, PLR_KILLER ) )
               {
                    if ( ch->pcdata->clan )
                    {
                         if ( ch->pcdata->clan->pkallow && ( IS_SET ( ch->in_room->room_flags, ROOM_PKILL ) &&
                                                             IS_SET ( victim->in_room->room_flags, ROOM_PKILL ) ) )
                              return FALSE;
                         else
                         {
                              if ( backtalk )
                                   send_to_char ( "Sorry, this isn't a Pkill room.\n\r", ch );
                              return TRUE;
                         }
                    }
                    else if ( !IS_SET ( ch->act, PLR_KILLER ) )
                    {
                         if ( backtalk )
                              send_to_char ( "You aren't a killer.\n\r", ch );
                         return TRUE;
                    }
               }
               if ( ch->pcdata->clan && victim->pcdata->clan )
               {
                    if ( !clan_war ( ch, victim ) && ( !IS_SET ( victim->in_room->room_flags, ROOM_PKILL ) ||
                                                       !IS_SET ( ch->in_room->room_flags, ROOM_PKILL ) ) )
                    {
                         if ( backtalk )
                              send_to_char ( "You aren't at war with them, and this isn't a PKill room.\n\r", ch );
                         return TRUE;
                    }
               }
          }
     }
     return FALSE;
}

// Okay, instead of duplicating EVERYTHING from the above, we'll just call it.
//
bool is_safe_spell ( CHAR_DATA * ch, CHAR_DATA * victim, bool area )
{
     if (victim->in_room == NULL || ch->in_room == NULL)
          return TRUE;

	 /* immortals not hurt in area attacks */
     if ( IS_IMMORTAL ( victim ) && area )   // Let's be quiet on area attacks
          return TRUE;

     if ( area )
     {
          if ( victim != ch && victim->fighting != ch && !is_same_group ( victim, ch->fighting ) )
               return TRUE;
     }

     // Call everything else from is_safe
     if ( is_safe ( ch, victim, TRUE ) )
          return TRUE;          // This one will be reported by is_safe
     else
          return FALSE;
}

/*
 * Check for dodges, shield blocks, etc, all in one place
 */

bool avoid_attack (CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, int dt)
{
     if ( dt >= TYPE_HIT && ch != victim )
     {
          if ( check_parry ( ch, victim ) )
          {
               OBJ_DATA *tmp1 = get_eq_char ( victim, WEAR_WIELD );
               OBJ_DATA *tmp2 = get_eq_char ( victim, WEAR_WIELD2 );
               if ( tmp1 != NULL ) check_damage_obj ( victim, tmp1, 2, dt );
               if ( tmp2 != NULL ) check_damage_obj ( victim, tmp2, 2, dt );
               if (wield)
                    check_damage_obj ( ch, wield, 2, dt );
               return TRUE;
          }
          if ( check_dodge ( ch, victim ) )
               return TRUE;
          if ( check_shield ( ch, victim ) )
          {
               check_damage_obj ( victim, get_eq_char ( victim, WEAR_SHIELD ), 20, dt );
               if (wield)
                    check_damage_obj ( ch, wield, 2, dt );
               return TRUE;
          }
     }
     return FALSE;
}

/*
 * Check for parry.
 */
bool check_parry ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     int                 chance;

     if ( !IS_AWAKE ( victim ) )
          return FALSE;

     if ( IS_NPC ( victim ) )
     {
          chance = UMIN ( 30, victim->level );
     }
     else
     {
          if ( ( get_eq_char ( victim, WEAR_WIELD ) == NULL )
               && ( get_eq_char ( victim, WEAR_WIELD2 ) == NULL ) )
               return FALSE;
          chance = victim->pcdata->learned[gsn_parry] / 2;
     }

     if ( number_percent (  ) >=
          chance + victim->level - ch->level )
          return FALSE;
     if ( IS_SET ( victim->comm, COMM_FULLFIGHT ) )
     {
          act ( "You parry $n's attack.", ch, NULL, victim,
                TO_VICT );
     }
     if ( IS_SET ( ch->comm, COMM_FULLFIGHT ) )
     {
          act ( "$N parries your attack.", ch, NULL, victim,
                TO_CHAR );
     }

     check_improve ( victim, gsn_parry, TRUE, 6 );
     return TRUE;
}

/*Check for shield block */

bool check_shield ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     int                 chance;

     if ( !IS_AWAKE ( victim ) )
          return FALSE;

     if ( get_eq_char ( victim, WEAR_SHIELD ) == NULL )	/*no shield, can't block */
          return FALSE;

     if ( IS_NPC ( victim ) )
          chance = UMIN ( 30, victim->level );
     else
          chance = victim->pcdata->learned[gsn_shield_block] / 2;

     if ( number_percent (  ) >=
          chance + victim->level - ch->level )
          return FALSE;

     if ( IS_SET ( victim->comm, COMM_FULLFIGHT ) )
     {
          act ( "You block $n's attack with your shield.", ch, NULL,
                victim, TO_VICT );
     }
     if ( IS_SET ( ch->comm, COMM_FULLFIGHT ) )
     {
          act ( "$N blocks your attack with his shield.", ch, NULL,
                victim, TO_CHAR );
     }

     check_improve ( victim, gsn_shield_block, TRUE, 6 );
     return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     int                 chance;

     if ( !IS_AWAKE ( victim ) )
          return FALSE;

     if ( IS_NPC ( victim ) )
          chance = UMIN ( 30, victim->level );
     else
          chance = victim->pcdata->learned[gsn_dodge] / 2;

     if ( number_percent (  ) >=
          chance + victim->level - ch->level )
          return FALSE;

     if ( IS_SET ( victim->comm, COMM_FULLFIGHT ) )
     {
          act ( "You dodge $n's attack.", ch, NULL, victim,
                TO_VICT );
     }

     if ( IS_SET ( ch->comm, COMM_FULLFIGHT ) )
     {
          act ( "$N dodges your attack.", ch, NULL, victim,
                TO_CHAR );
     }
     check_improve ( victim, gsn_dodge, TRUE, 6 );
     return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos ( CHAR_DATA * victim )
{
     if ( victim->hit > 0 )
     {
          if ( victim->position <= POS_STUNNED )
               victim->position = POS_STANDING;
          return;
     }

     if ( IS_NPC ( victim ) && victim->hit < 1 )
     {
          victim->position = POS_DEAD;
          return;
     }

     if ( victim->hit <= -( victim->perm_stat[STAT_CON] * 1.1 ) )
     {
          victim->position = POS_DEAD;
          return;
     }

     if ( victim->hit <= -10 )
          victim->position = POS_MORTAL;
     else if ( victim->hit <= -4 )
          victim->position = POS_INCAP;
     else
          victim->position = POS_STUNNED;

     return;
}

/*
 * Start fights.
 */
void set_fighting ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     if ( ch->fighting != NULL )
     {
          bugf ( "Set_fighting: already fighting" );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_SLEEP ) )
          affect_strip ( ch, gsn_sleep );

     ch->fighting = victim;
     ch->position = POS_FIGHTING;

     return;
}

/*
 * Stop fights.
 * This was a big cpu hog lopping through the whole damn char_list
 * every time.
 */
void stop_fighting ( CHAR_DATA * ch, bool fBoth )
{
     CHAR_DATA          *fch;

     if ( !fBoth )
     {
          ch->fighting = NULL;
          ch->position = IS_NPC ( ch ) ? ch->default_pos : POS_STANDING;
          update_pos ( ch );
          return;
     }
     else
     {
          for ( fch = char_list; fch != NULL; fch = fch->next )
          {
               if ( fch == ch || ( fBoth && fch->fighting == ch ) )
               {
                    fch->fighting = NULL;
                    fch->position = IS_NPC ( fch ) ? ch->default_pos : POS_STANDING;
                    update_pos ( fch );
               }
          }
     }
     return;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse ( CHAR_DATA * ch )
{
     char                buf[MAX_STRING_LENGTH];
     OBJ_DATA           *corpse;
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;
     char               *name;

     if ( IS_NPC ( ch ) )
     {
          name = ch->short_descr;
          corpse =
               create_object ( get_obj_index ( OBJ_VNUM_CORPSE_NPC ),
                               0 );
          corpse->timer = number_range ( 3, 6 );
          if ( ch->gold > 0 )
          {
               obj_to_obj ( create_money ( ch->gold ), corpse );
               ch->gold = 0;
          }
          corpse->cost = 0;
     }
     else
     {
          name = ch->name;
          corpse =
               create_object ( get_obj_index ( OBJ_VNUM_CORPSE_PC ),
                               0 );
          corpse->timer = number_range ( 25, 40 );
          REMOVE_BIT ( ch->act, PLR_CANLOOT );
          if ( !IS_SET ( ch->act, PLR_KILLER ) &&
               !IS_SET ( ch->act, PLR_THIEF ) )
               corpse->owner = str_dup ( ch->name );
          else
               corpse->owner = NULL;
          corpse->cost = 0;
     }

     corpse->level = ch->level;

     SNP ( buf, corpse->short_descr, name );
     free_string ( corpse->short_descr );
     corpse->short_descr = str_dup ( buf );

     SNP ( buf, corpse->description, name );
     free_string ( corpse->description );
     corpse->description = str_dup ( buf );

     for ( obj = ch->carrying; obj; obj = obj_next )
     {
          obj_next = obj->next_content;

          obj_from_char ( obj );
          if ( IS_SET ( obj->extra_flags, ITEM_ROT_DEATH ) )
               obj->timer = number_range ( 5, 10 );
          REMOVE_BIT ( obj->extra_flags, ITEM_VIS_DEATH );
          REMOVE_BIT ( obj->extra_flags, ITEM_ROT_DEATH );

          if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
               extract_obj( obj );
          else
               obj_to_obj ( obj, corpse );
     }

     obj_to_room ( corpse, ch->in_room );
     return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry ( CHAR_DATA * ch )
{
     ROOM_INDEX_DATA    *was_in_room;
     char               *msg;
     int                 door;
     int                 vnum;

     vnum = 0;
     msg = "You hear the horrible death cry of $n.";

     switch ( number_bits ( 4 ) )
     {
     case 0:
          msg = "$n hits the ground ... {RDEAD{x.";
          break;
     case 1:
          if ( ch->material == 0 )
          {
               msg = "$n splatters {Rblood{x on your armor.";
               break;
          }
     case 2:
          if ( IS_SET ( ch->parts, PART_GUTS ) )
          {
               msg = "$n spills $s guts all over the floor.";
               vnum = OBJ_VNUM_GUTS;
          }
          break;
     case 3:
          if ( IS_SET ( ch->parts, PART_HEAD ) )
          {
               msg = "$n's severed head plops on the ground.";
               vnum = OBJ_VNUM_SEVERED_HEAD;
          }
          break;
     case 4:
          if ( IS_SET ( ch->parts, PART_HEART ) )
          {
               msg = "$n's heart is torn from $s chest.";
               vnum = OBJ_VNUM_TORN_HEART;
          }
          break;
     case 5:
          if ( IS_SET ( ch->parts, PART_ARMS ) )
          {
               msg = "$n's arm is sliced from $s dead body.";
               vnum = OBJ_VNUM_SLICED_ARM;
          }
          break;
     case 6:
          if ( IS_SET ( ch->parts, PART_LEGS ) )
          {
               msg = "$n's leg is sliced from $s dead body.";
               vnum = OBJ_VNUM_SLICED_LEG;
          }
          break;
     case 7:
          if ( IS_SET ( ch->parts, PART_BRAINS ) )
          {
               msg =
                    "$n's head is shattered, and $s brains splash all over you.";
               vnum = OBJ_VNUM_BRAINS;
          }
     }

     act ( msg, ch, NULL, NULL, TO_ROOM );

     if ( vnum != 0 )
     {
          char                buf[MAX_STRING_LENGTH];
          OBJ_DATA           *obj;
          char               *name;

          name = IS_NPC ( ch ) ? ch->short_descr : ch->name;
          obj = create_object ( get_obj_index ( vnum ), 0 );
          obj->timer = number_range ( 4, 7 );

          SNP ( buf, obj->short_descr, name );
          free_string ( obj->short_descr );
          obj->short_descr = str_dup ( buf );

          SNP ( buf, obj->description, name );
          free_string ( obj->description );
          obj->description = str_dup ( buf );

          if ( obj->item_type == ITEM_FOOD )
          {
               if ( IS_SET ( ch->form, FORM_POISON ) )
                    obj->value[3] = 1;
               else if ( !IS_SET ( ch->form, FORM_EDIBLE ) )
                    obj->item_type = ITEM_TRASH;
          }

          obj_to_room ( obj, ch->in_room );
     }

     if ( IS_NPC ( ch ) )
          msg = "You hear something's death cry.";
     else
          msg = "You hear someone's death cry.";

     was_in_room = ch->in_room;
     for ( door = 0; door <= 5; door++ )
     {
          EXIT_DATA          *pexit;

          if ( ( pexit = was_in_room->exit[door] ) != NULL
               && pexit->u1.to_room != NULL
               && pexit->u1.to_room != was_in_room )
          {
               ch->in_room = pexit->u1.to_room;
               act ( msg, ch, NULL, NULL, TO_ROOM );
          }
     }
     ch->in_room = was_in_room;

     return;
}

void raw_kill ( CHAR_DATA * victim )
{
     stop_fighting ( victim, TRUE );

     make_corpse ( victim );

     if ( IS_NPC ( victim ) )
     {
          victim->pIndexData->killed++;
          kill_table[URANGE ( 0, victim->level, MAX_LEVEL - 1 )].killed++;
          extract_char ( victim, TRUE );
          return;
     }
     else
     {
          if (chance(25) )
               sound ("death_male.wav", victim);
          else if (victim->sex == SEX_MALE)
               sound ("death2a.wav", victim);
          else if (victim->sex == SEX_FEMALE)
               sound ("death_fem.wav", victim);
          else
               sound ("DEATH_CR.WAV", victim);
     }

     extract_char ( victim, FALSE );
     while ( victim->affected )
          affect_remove ( victim, victim->affected );

     if ( mud.death == PERMADEATH && victim->pcdata->mortal ) // Oooh booger....
     {
          send_to_char ("\n\r{DBlackness closes over you like a shroud.\n\r", victim);
          send_to_char ("\n\r{WPlease come back soon!\n\r\n\r", victim);
          do_quote ( victim );
          ++victim->pcdata->account->permadead;
          real_delete ( victim );    /* Delete character Fully */
          return;
     }

	 /*Zeran:  reset affected_by to racial affect flags */

     victim->affected_by = race_table[victim->race].aff;
     victim->detections = race_table[victim->race].detect;
     victim->protections = race_table[victim->race].protect;

     //     for ( i = 0; i < 4; i++ )
     victim->armor = 0;
     victim->position = POS_RESTING;
     victim->hit =   UMAX ( ( sh_int ) ( victim->max_hit / 2 ), victim->hit );
     victim->mana =  UMAX ( ( sh_int ) ( victim->max_mana / 2 ), victim->mana );
     victim->move =  UMAX ( ( sh_int ) ( victim->max_move / 2 ), victim->move );

	 /* RT added to prevent infinite deaths */
         /* Loth removed cuz once a killer, always a killer. */
         /* The better solution is to make sure executioner type mobs don't
          * show up in player respawn locations. */

     //     REMOVE_BIT ( victim->act, PLR_KILLER );
     REMOVE_BIT ( victim->act, PLR_THIEF );
     REMOVE_BIT ( victim->act, PLR_BOUGHT_PET );
     victim->recall_temp = 0;	/* Zeran - reset "anchored" recall spots */

     save_char_obj( victim );
     return;
}

void group_gain ( CHAR_DATA * ch, CHAR_DATA * victim )
{
     CHAR_DATA          *leader;
     CHAR_DATA		*gch;
     int                 xp = 0;
     int                 members;
     int                 group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
     if ( victim == ch )
          return;

     members = 0;
     group_levels = 0;

	/* If ch is in group, set leader, else set leader = ch */

     leader = ( ch->leader != NULL ) ? ch->leader : ch;

	/* Count number of folks in the group who are in the room */

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( is_same_group( gch, ch ) )
          {
               members++;
               group_levels += gch->level;
          }
     }

     if ( members == 0 )
     {
          bugf ( "Group_gain: members.", members );
          members = 1;
          group_levels = ch->level ;
     }

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {

          OBJ_DATA           *obj;
          OBJ_DATA           *obj_next;

          if ( !is_same_group(gch, ch) || IS_NPC ( gch ) )
               continue;

          if ( gch->level - leader->level >= 9 )
          {
               send_to_char ( "You are too high to gain xp in this group.\n\r", gch );
               continue;
          }

          if ( gch->level - leader->level <= -9 )
          {
               send_to_char ( "You are too low to gain xp in this group.\n\r", gch );
               continue;
          }

          xp = xp_compute ( gch, victim, (group_levels/members), (members > 1 ? TRUE:FALSE ) );

          /* Do PC only stuff. */

          if ( !IS_NPC ( ch ) )
          {
               if ( !IS_NPC ( victim ) )
               {
                    gch->pcdata->battle_rating += (xp/5);
                    victim->pcdata->battle_rating -= (xp/5);
                    gch->pcdata->pkill_wins += 1;
                    victim->pcdata->pkill_losses += 1;
                    xp = 0;
               }
               else
               {
                    /* This one is mob kill rating */
                    gch->pcdata->mob_wins += 1;

                    if (gch->level > victim->level+2 || gch->level < victim->level )
                    {
                         int ldiff;
                         if (gch->level < victim->level)
                         {
                              int bonus;

                              ldiff = ( victim->level = gch->level );
                              bonus = ldiff;

                              if ( bonus > 5 )
                                   bonus = 5;

                              gch->pcdata->mob_rating += ( ldiff + bonus );
                         }
                         else
                         {
                              ldiff = (gch->level - victim->level);

                              if (ldiff > 10)	// No more than 10 pts. lost at a time.
                                   ldiff = 10;

                              gch->pcdata->mob_rating -= ldiff;
                         }
                    }
               }

               form_to_char ( ch, "You receive %d experience points.\n\r", xp );
               if ( IS_SET ( ch->act, PLR_QUESTOR ) && IS_NPC ( victim ) )
               {
                    if ( ch->pcdata->questmob == victim->pIndexData->vnum )
                    {
                         send_to_char ( "{CYou have almost completed your QUEST!{x\n\r", ch );
                         send_to_char ( "Return to the questmaster before your time runs out!\n\r", ch );
                         ch->pcdata->questmob = -1;
                    }
               }
          }
          /* End of PC only, don't access PCdata past this point. */

          gain_exp ( gch, xp );

          for ( obj = ch->carrying; obj != NULL; obj = obj_next )
          {
               obj_next = obj->next_content;
               if ( obj->wear_loc == WEAR_NONE )
                    continue;

               if ( ( IS_OBJ_STAT ( obj, ITEM_ANTI_EVIL ) && IS_EVIL ( ch ) ) ||
                    ( IS_OBJ_STAT ( obj, ITEM_ANTI_GOOD ) && IS_GOOD ( ch ) ) ||
                    ( IS_OBJ_STAT ( obj, ITEM_ANTI_NEUTRAL ) && IS_NEUTRAL ( ch ) ) )
               {
                    if ( IS_NPC ( ch ) )
                         MOBtrigger = FALSE; // In case someone has this setup to trigger
                    act ( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
                    act ( "$n is zapped by $p.", ch, obj, NULL, TO_ROOM );
                    MOBtrigger = TRUE;
                    obj_from_char ( obj );
                    obj_to_room ( obj, ch->in_room );
               }
          }
     }
     return;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute ( CHAR_DATA * gch, CHAR_DATA * victim, int avglevel, bool group )
{
     int                 xp, base_exp;
     int                 align, level_range;
     int                 change;

     level_range = victim->level - gch->level;

    /* compute the base exp */
     switch ( level_range )
     {
     default:
          base_exp = 0;
          break;
     case -9:
          base_exp = 1;
          break;
     case -8:
          base_exp = 2;
          break;
     case -7:
          base_exp = 5;
          break;
     case -6:
          base_exp = 9;
          break;
     case -5:
          base_exp = 11;
          break;
     case -4:
          base_exp = 22;
          break;
     case -3:
          base_exp = 33;
          break;
     case -2:
          base_exp = 50;
          break;
     case -1:
          base_exp = 66;
          break;
     case 0:
          base_exp = 83;
          break;
     case 1:
          base_exp = 99;
          break;
     case 2:
          base_exp = 121;
          break;
     case 3:
          base_exp = 143;
          break;
     case 4:
          base_exp = 165;
          break;
     case 5:
          base_exp = 155;
     }

     if ( level_range > 4 )
          base_exp = 150 + 25 * ( level_range - 4 );

    /* do alignment computations */

     align = victim->alignment - gch->alignment;

     if ( IS_NPC ( victim ) && IS_SET ( victim->act, ACT_NOALIGN ) )
     {
	/* no change */
     }
     else if ( align > 500 )	/* monster is more good than slayer */
     {
          change =
               ( align -
                 500 ) * base_exp / 500 * avglevel;
          change = UMAX ( 1, change );
          gch->alignment = UMAX ( -1000, gch->alignment - change );
     }

     else if ( align < -500 )	/* monster is more evil than slayer */
     {
          change =
               ( -1 * align -
                 500 ) * base_exp / 500 * avglevel;
          change = UMAX ( 1, change );
          gch->alignment = UMIN ( 1000, gch->alignment + change );
     }

     else			/* improve this someday */
     {
          change =
               gch->alignment * base_exp / 500 * avglevel;
          gch->alignment -= change;
     }
    /* calculate exp multiplier */
     if ( IS_NPC ( victim ) && IS_SET ( victim->act, ACT_NOALIGN ) )
          xp = base_exp;
     else if ( gch->alignment > 500 )	/* for goodie two shoes */
     {
          if ( victim->alignment < -750 )
               xp = base_exp * 4 / 3;

          else if ( victim->alignment < -500 )
               xp = base_exp * 5 / 4;

          else if ( victim->alignment > 750 )
               xp = base_exp / 4;

          else if ( victim->alignment > 500 )
               xp = base_exp / 2;

          else if ( victim->alignment > 250 )
               xp = base_exp * 3 / 4;

          else
               xp = base_exp;
     }

     else if ( gch->alignment < -500 )	/* for baddies */
     {
          if ( victim->alignment > 750 )
               xp = base_exp * 5 / 4;

          else if ( victim->alignment > 500 )
               xp = base_exp * 11 / 10;

          else if ( victim->alignment < -750 )
               xp = base_exp * 1 / 2;

          else if ( victim->alignment < -500 )
               xp = base_exp * 3 / 4;

          else if ( victim->alignment < -250 )
               xp = base_exp * 9 / 10;

          else
               xp = base_exp;
     }

     else if ( gch->alignment > 200 )	/* a little good */
     {

          if ( victim->alignment < -500 )
               xp = base_exp * 6 / 5;

          else if ( victim->alignment > 750 )
               xp = base_exp * 1 / 2;

          else if ( victim->alignment > 0 )
               xp = base_exp * 3 / 4;

          else
               xp = base_exp;
     }

     else if ( gch->alignment < -200 )	/* a little bad */
     {
          if ( victim->alignment > 500 )
               xp = base_exp * 6 / 5;

          else if ( victim->alignment < -750 )
               xp = base_exp * 1 / 2;

          else if ( victim->alignment < 0 )
               xp = base_exp * 3 / 4;

          else
               xp = base_exp;
     }

     else			/* neutral */
     {

          if ( victim->alignment > 500 || victim->alignment < -500 )
               xp = base_exp * 4 / 3;

          else if ( victim->alignment < 200 ||
                    victim->alignment > -200 )
               xp = base_exp * 1 / 2;

          else
               xp = base_exp;
     }

    /* randomize the rewards */
     xp = number_range ( xp * 3 / 4, xp * 5 / 4 );

    /* adjust for grouping */

     /* Umm... why weren't we making sure there was a group first ? */
     /* Zeran - multiply by 1.75 to encourage grouping */
     //    xp = xp * gch->level / total_levels * 1.75;
     //
     //      xp = xp * 1.75;
     //
     if (group)
          xp = xp * 1.65;

     if ( xp < 0 )
          xp = 0;

	 /* Now let's do the difficulty */
     switch (mud.mudxp)
     {
     case XP_EASIEST:
          xp *= 4;
          break;
     case XP_EASY:
          xp *= 2;
          break;
     case XP_NORMAL:
          // no change
          break;
     case XP_HARD:
          xp *= .75;
          break;
     case XP_VERYHARD:
          xp *= .5;
          break;
     case XP_NIGHTMARE:
          xp *= .25;
          break;
     }

     return xp;
}

void dam_message ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune, int where )
{
     char                buf1[256], buf2[256], buf3[256];
     const char         *vs;
     const char         *vp;
     const char         *attack;
     char                punct;

     if ( dam == 0 )
     {
          vs = "miss";
          vp = "misses";
     }
     else if ( dam <= 4 )
     {
          vs = "{wscratch{x";
          vp = "{wscratches{x";
     }
     else if ( dam <= 8 )
     {
          vs = "{wgraze{x";
          vp = "{wgrazes{x";
     }
     else if ( dam <= 12 )
     {
          vs = "{whit{x";
          vp = "{whits{x";
     }
     else if ( dam <= 16 )
     {
          vs = "{binjure{x";
          vp = "{binjures{x";
     }
     else if ( dam <= 20 )
     {
          vs = "{bwound{x";
          vp = "{bwounds{x";
     }
     else if ( dam <= 24 )
     {
          vs = "{bmaul{x";
          vp = "{bmauls{x";
     }
     else if ( dam <= 28 )
     {
          vs = "{gdecimate{x";
          vp = "{gdecimates{x";
     }
     else if ( dam <= 32 )
     {
          vs = "{gdevastate{x";
          vp = "{gdevastates{x";
     }
     else if ( dam <= 36 )
     {
          vs = "{gmaim{x";
          vp = "{gmaims{x";
     }
     else if ( dam <= 40 )
     {
          vs = "{cMUTILATE{x";
          vp = "{cMUTILATES{x";
     }
     else if ( dam <= 44 )
     {
          vs = "{cDISEMBOWEL{x";
          vp = "{cDISEMBOWELS{x";
     }
     else if ( dam <= 48 )
     {
          vs = "{cDISMEMBER{x";
          vp = "{cDISMEMBERS{x";
     }
     else if ( dam <= 52 )
     {
          vs = "{cMASSACRE{x";
          vp = "{cMASSACRES{x";
     }
     else if ( dam <= 56 )
     {
          vs = "{cMANGLE{x";
          vp = "{cMANGLES{x";
     }
     else if ( dam <= 60 )
     {
          vs = "{G*** DEMOLISH ***{x";
          vp = "{G*** DEMOLISHES ***{x";
     }
     else if ( dam <= 75 )
     {
          vs = "{G*** DEVASTATE ***{x";
          vp = "{G*** DEVASTATES ***{x";
     }
     else if ( dam <= 100 )
     {
          vs = "{C=== OBLITERATE ==={x";
          vp = "{C=== OBLITERATES ==={x";
     }
     else if ( dam <= 125 )
     {
          vs = "{r>>> ANNIHILATE <<<{x";
          vp = "{r>>> ANNIHILATES <<<{x";
     }
     else if ( dam <= 150 )
     {
          vs = "{r<<< ERADICATE >>>{x";
          vp = "{r<<< ERADICATES >>>{x";
     }
     else if ( dam <= 200 )
     {
          vs = "{W**** {cP{CULVERIZ{cE {W****{x";
          vp = "{W**** {cP{CULVERIZE{cS {W****{x";
     }
     else if ( dam <= 250 )
     {
          vs = "{b<*>{G<*>{R<*> {CFISSURE{R <*>{G<*>{b<*>{x";
          vp = "{b<*>{G<*>{R<*> {CFISSURES{R <*>{G<*>{b<*>{x";
     }
     else if ( dam <= 400 )
     {
          vs = "{x<--=*=--><--=*=--> {BCrUsH{x <--=*=--><--=*=-->";
          vp = "{x<--=*=--><--=*=--> {BCrUsHeS{x <--=*=--><--=*=-->";
     }
     else if ( dam <= 500 )
     {
          vs = "{G<{W*{G><-:-><{W*{G> {YBLOW AWAY {G<{W*{G><-:-><{W*{G>{x";
          vp = "{G<{W*{G><-:-><{W*{G> {YBLOWS AWAY {G<{W*{G><-:-><{W*{G>{x";
     }
     else
     {
          vs = "do {gUN{YSPEAK{gABLE{x things to";
          vp = "{gdoes UN{YSPEAK{gABLE things to{x";
     }

     punct = ( dam <= 24 ) ? '.' : '!';

     if ( dt == TYPE_HIT )
     {
          if ( ch == victim )
          {
               if ( where > 0 )
               {
                    SNP ( buf1, "$n %s $s own %s%c [%d]", vp, wear_info[where].name, punct, dam );
                    SNP ( buf2, "You %s your own %s%c [%d]", vs, wear_info[where].name, punct, dam );
               }
               else
               {
                    SNP ( buf1, "$n %s $melf%c [%d]", vp, punct, dam );
                    SNP ( buf2, "You %s yourself%c [%d]", vs, punct, dam );
               }
          }
          else
          {
               if ( where > 0 )
               {
                    SNP ( buf1, "$n %s $N's %s%c [%d]", vp, wear_info[where].name, punct, dam );
                    SNP ( buf2, "{yYou %s {y$N's %s%c{x [{y%d{x]", vs, wear_info[where].name, punct, dam );
                    SNP ( buf3, "{r$n %s {ryour %s%c{x [{r%d{x]", vp, wear_info[where].name, punct, dam );
               }
               else
               {
                    SNP ( buf1, "$n %s $N%c [%d]", vp, punct, dam );
                    SNP ( buf2, "{yYou %s {y$N%c{x [{y%d{x]", vs, punct, dam );
                    SNP ( buf3, "{r$n %s {ryou%c{x [{r%d{x]", vp, punct, dam );
               }
          }
     }
     else
     {
          if ( dt >= 0 && dt < MAX_SKILL )
               attack = skill_table[dt].noun_damage;
          else if ( dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE )
               attack = attack_table[dt - TYPE_HIT].name;
          else
          {
               bugf ( "Dam_message: bad dt %d.", dt );
               dt = TYPE_HIT;
               attack = attack_table[0].name;
          }
          if ( immune )
          {
               if ( ch == victim )
               {
                    SNP ( buf1, "$n is unaffected by $s own %s.",  attack );
                    SNP ( buf2, "Luckily, you are immune to that." );
               }
               else
               {
                    SNP ( buf1, "$N is unaffected by $n's %s!", attack );
                    SNP ( buf2, "$N is {gunaffected{x by your %s!", attack );
                    SNP ( buf3, "$n's %s is {cpowerless{x against you.", attack );
               }
          }
          else
          {
               if ( ch == victim )
               {
                    if ( where > 0 )
                    {
                         SNP ( buf1, "$n's %s %s $s own %s%c [%d]", attack, vp, wear_info[where].name, punct, dam );
                         SNP ( buf2, "Your %s %s your %s%c [%d]", attack, vp, wear_info[where].name, punct, dam );
                    }
                    else
                    {
                         SNP ( buf1, "$n's %s %s $m%c [%d]", attack, vp, punct, dam );
                         SNP ( buf2, "Your %s %s you%c [%d]", attack, vp, punct, dam );
                    }
               }
               else
               {
                    if ( where > 0 )
                    {
                         SNP ( buf1, "$n's %s %s $N's %s%c [%d]", attack, vp, wear_info[where].name, punct, dam );
                         SNP ( buf2, "{yYour %s %s {y$N's %s%c{x [{y%d{x]", attack, vp, wear_info[where].name, punct, dam );
                         SNP ( buf3, "{r$n's %s %s {ryour %s%c{x [{r%d{x]", attack, vp, wear_info[where].name, punct, dam );
                    }
                    else
                    {
                         SNP ( buf1, "$n's %s %s $N%c [%d]", attack, vp, punct, dam );
                         SNP ( buf2, "{yYour %s %s {y$N%c{x [{y%d{x]", attack, vp, punct, dam );
                         SNP ( buf3, "{r$n's %s %s {ryou%c{x [{r%d{x]", attack, vp, punct, dam );
                    }
               }
          }
     }

     if ( ch == victim )
     {
          act ( buf1, ch, NULL, NULL, TO_ROOM );
          act ( buf2, ch, NULL, NULL, TO_CHAR );
     }
     else
     {
          act ( buf1, ch, NULL, victim, TO_NOTVICT );
          act ( buf2, ch, NULL, victim, TO_CHAR );
          act ( buf3, ch, NULL, victim, TO_VICT );
     }

     return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm ( CHAR_DATA * ch, CHAR_DATA * victim, bool dual )
{
     OBJ_DATA           *obj;
     char                messbuf[128];

     if ( !dual )
     {
          if ( ( obj = get_eq_char ( victim, WEAR_WIELD ) ) == NULL )
               return;
     }
     else
     {
          if ( ( obj = get_eq_char ( victim, WEAR_WIELD2 ) ) == NULL )
               return;
     }

     if ( IS_OBJ_STAT ( obj, ITEM_NOREMOVE ) )
     {
          act ( "$S weapon won't budge!", ch, NULL, victim, TO_CHAR );
          act ( "$n tries to disarm you, but your weapon won't budge!", ch, NULL, victim, TO_VICT );
          act ( "$n tries to disarm $N, but fails.", ch, NULL, victim, TO_NOTVICT );
          return;
     }

     sound ("disarm.wav", ch);
     sound ("disarm.wav", victim);

     SNP ( messbuf, "$n disarms you and sends %s flying!", obj->short_descr );
     act ( messbuf, ch, NULL, victim, TO_VICT );
     SNP ( messbuf, "You disarm $N! %s goes flying!", obj->short_descr );
     act ( messbuf, ch, NULL, victim, TO_CHAR );
     act ( "$n disarms $N!", ch, NULL, victim, TO_NOTVICT );

     obj_from_char ( obj );
     if ( IS_OBJ_STAT ( obj, ITEM_NODROP ) ||
          IS_OBJ_STAT ( obj, ITEM_INVENTORY ) )
          obj_to_char ( obj, victim );
     else
     {
          obj_to_room ( obj, victim->in_room );
          if ( IS_NPC ( victim ) && victim->wait == 0 &&
               can_see_obj ( victim, obj ) )
               get_obj ( victim, obj, NULL );
     }

     return;
}

void do_berserk ( CHAR_DATA * ch, char *argument )
{
     int                 chance, hp_percent;

     if ( ( chance = get_skill ( ch, gsn_berserk ) ) == 0
          || ( IS_NPC ( ch ) &&
               !IS_SET ( ch->off_flags, OFF_BERSERK ) ) )
     {
          send_to_char ( "You turn red in the face, but nothing happens.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_BERSERK ) ||
          is_affected ( ch, gsn_berserk ) ||
          is_affected ( ch, skill_lookup ( "frenzy" ) ) )
     {
          send_to_char ( "You get a little madder.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CALM ) )
     {
          send_to_char ( "You're feeling to mellow to berserk.\n\r", ch );
          return;
     }

     if ( ch->mana < 50 )
     {
          send_to_char ( "You can't get up enough energy.\n\r", ch );
          return;
     }

    /* modifiers */

    /* fighting */
     if ( ch->position == POS_FIGHTING )
          chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
     hp_percent = 100 * ch->hit / ch->max_hit;
     chance += 25 - hp_percent / 2;

     if ( number_percent (  ) < chance )
     {
          AFFECT_DATA         af;

          WAIT_STATE ( ch, PULSE_VIOLENCE );
          ch->mana -= 50;
          ch->move /= 2;

	/* heal a little damage */
          ch->hit += ch->level * 2;
          ch->hit = UMIN ( ch->hit, ch->max_hit );

          send_to_char ( "{RYour pulse races as you are consumned by rage!{x\n\r", ch );
          act ( "{R$n gets a wild look in $s eyes.{x", ch, NULL, NULL, TO_ROOM );
          check_improve ( ch, gsn_berserk, TRUE, 2 );

          af.type = gsn_berserk;
          af.level = ch->level;
          af.duration = number_fuzzy ( ch->level / 8 );
          af.modifier = UMAX ( 1, ch->level / 5 );
          af.bitvector = AFF_BERSERK;
          af.where = TO_AFFECTS;
          af.location = APPLY_HITROLL;
          affect_to_char ( ch, &af );

          af.location = APPLY_DAMROLL;
          affect_to_char ( ch, &af );

          af.modifier = -(UMAX ( 10, 10 * ( ch->level / 5 ) ));
          af.location = APPLY_AC;
          affect_to_char ( ch, &af );
     }

     else
     {
          WAIT_STATE ( ch, 3 * PULSE_VIOLENCE );
          ch->mana -= 25;
          ch->move /= 2;

          send_to_char ( "Your pulse speeds up, but nothing happens.\n\r", ch );
          check_improve ( ch, gsn_berserk, FALSE, 2 );
     }
}

void do_bash ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 chance;

     one_argument ( argument, arg );

     if ( ( chance = get_skill ( ch, gsn_bash ) ) == 0
          || ( IS_NPC ( ch ) &&
               !IS_SET ( ch->off_flags, OFF_BASH ) ) )
     {
          send_to_char ( "Bashing? What's that?\n\r", ch );
          return;
     }

     if ( arg[0] == '\0' )
     {
          victim = ch->fighting;
          if ( victim == NULL )
          {
               send_to_char ( "But you aren't fighting anyone!\n\r", ch );
               return;
          }
     }

     else if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim->position < POS_FIGHTING )
     {
          act ( "You'll have to let $M get back up first.", ch, NULL,
                victim, TO_CHAR );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "You try to bash your brains out, but fail.\n\r", ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( victim->fighting != NULL &&
          !is_same_group ( ch, victim->fighting ) )
     {
          send_to_char ( "Kill stealing is not permitted.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
     {
          act ( "But $N is your friend!", ch, NULL, victim,
                TO_CHAR );
          return;
     }

    /* modifiers */

    /* size  and weight */
     chance += ch->carry_weight / 25;
     chance -= victim->carry_weight / 20;

     if ( ch->size < victim->size )
          chance += ( ch->size - victim->size ) * 25;
     else
          chance += ( ch->size - victim->size ) * 10;

    /* stats */
     chance += get_curr_stat ( ch, STAT_STR );
     chance -= get_curr_stat ( victim, STAT_DEX ) * 4 / 3;

    /* speed */
     if ( IS_SET ( ch->off_flags, OFF_FAST ) )
          chance += 10;
     if ( IS_SET ( victim->off_flags, OFF_FAST ) )
          chance -= 20;

    /* level */
     chance += ( ch->level - victim->level ) * 2;

    /* now the attack */
     if ( number_percent (  ) < chance )
     {

          act ( "$n sends you sprawling with a powerful bash!",
                ch, NULL, victim, TO_VICT );
          act ( "You slam into $N, and send $M flying!", ch, NULL,
                victim, TO_CHAR );
          act ( "$n sends $N sprawling with a powerful bash.", ch,
                NULL, victim, TO_NOTVICT );
          check_improve ( ch, gsn_bash, TRUE, 1 );

          WAIT_STATE ( victim, 3 * PULSE_VIOLENCE );
          WAIT_STATE ( ch, skill_table[gsn_bash].beats );
          victim->position = POS_RESTING;
          damage ( ch, victim, number_range ( 2, 2 + 2 * ch->size + chance / 20 ),
                   gsn_bash, DAM_BASH, TRUE );

     }
     else
     {
          damage ( ch, victim, 0, gsn_bash, DAM_BASH, TRUE );
          act ( "You fall flat on your face!",
                ch, NULL, victim, TO_CHAR );
          act ( "$n falls flat on $s face.",
                ch, NULL, victim, TO_NOTVICT );
          act ( "You evade $n's bash, causing $m to fall flat on $s face.", ch, NULL, victim, TO_VICT );
          check_improve ( ch, gsn_bash, FALSE, 1 );
          ch->position = POS_RESTING;
          WAIT_STATE ( ch, skill_table[gsn_bash].beats * 3 / 2 );
     }
}

void do_dirt ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 chance;

     one_argument ( argument, arg );

     if ( ( chance = get_skill ( ch, gsn_dirt ) ) == 0
          || ( IS_NPC ( ch ) &&
               !IS_SET ( ch->off_flags, OFF_KICK_DIRT ) ) )
     {
          send_to_char ( "You get your feet dirty.\n\r", ch );
          return;
     }

     if ( arg[0] == '\0' )
     {
          victim = ch->fighting;
          if ( victim == NULL )
          {
               send_to_char ( "But you aren't in combat!\n\r", ch );
               return;
          }
     }

     else if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( victim, AFF_BLIND ) )
     {
          act ( "$e's already been blinded.", ch, NULL, victim,
                TO_CHAR );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "Very funny.\n\r", ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( victim->fighting != NULL &&
          !is_same_group ( ch, victim->fighting ) )
     {
          send_to_char ( "Kill stealing is not permitted.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
     {
          act ( "But $N is such a good friend!", ch, NULL, victim,
                TO_CHAR );
          return;
     }

    /* modifiers */

    /* dexterity */
     chance += get_curr_stat ( ch, STAT_DEX );
     chance -= 2 * get_curr_stat ( victim, STAT_DEX );

    /* speed  */
     if ( IS_SET ( ch->off_flags, OFF_FAST ) ||
          IS_AFFECTED ( ch, AFF_HASTE ) )
          chance += 10;
     if ( IS_SET ( victim->off_flags, OFF_FAST ) ||
          IS_AFFECTED ( victim, AFF_HASTE ) )
          chance -= 25;

    /* level */
     chance += ( ch->level - victim->level ) * 2;

    /* sloppy hack to prevent false zeroes */
     if ( chance % 5 == 0 )
          chance += 1;

    /* terrain */

     switch ( ch->in_room->sector_type )
     {
     case ( SECT_INSIDE ):
          chance -= 20;
          break;
     case ( SECT_CITY ):
          chance -= 10;
          break;
     case ( SECT_FIELD ):
          chance += 5;
          break;
     case ( SECT_FOREST ):
          break;
     case ( SECT_HILLS ):
          break;
     case ( SECT_MOUNTAIN ):
          chance -= 10;
          break;
     case ( SECT_WATER_SWIM ):
          chance = 0;
          break;
     case ( SECT_WATER_NOSWIM ):
          chance = 0;
          break;
     case ( SECT_AIR ):
          chance = 0;
          break;
     case ( SECT_DESERT ):
          chance += 10;
          break;
     case ( SECT_FORT ):
          chance -= 20;
          break;
     }

     if ( chance == 0 )
     {
          send_to_char ( "There isn't any dirt to kick.\n\r", ch );
          return;
     }

    /* now the attack */
     if ( number_percent (  ) < chance )
     {
          AFFECT_DATA         af;

          act ( "$n kicks dirt in your eyes!", ch, NULL, victim,
                TO_VICT );
          act ( "$n is blinded by the dirt in $s eyes!", victim,
                NULL, NULL, TO_ROOM );
          damage ( ch, victim, number_range ( 2, 5 ), gsn_dirt,
                   DAM_NONE, TRUE );
          send_to_char ( "You can't see a thing!\n\r", victim );
          check_improve ( ch, gsn_dirt, TRUE, 2 );
          WAIT_STATE ( ch, skill_table[gsn_dirt].beats );
          af.where = TO_AFFECTS;
          af.type = gsn_dirt;
          af.level = ch->level;
          af.duration = 0;
          af.location = APPLY_HITROLL;
          af.modifier = -4;
          af.bitvector = AFF_BLIND;

          affect_to_char ( victim, &af );
     }
     else
     {
          damage ( ch, victim, 0, gsn_dirt, DAM_NONE, TRUE );
          check_improve ( ch, gsn_dirt, FALSE, 2 );
          WAIT_STATE ( ch, skill_table[gsn_dirt].beats );
     }
}

void do_trip ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 chance;

     one_argument ( argument, arg );

     if ( ( chance = get_skill ( ch, gsn_trip ) ) == 0
          || ( IS_NPC ( ch ) 
               && !IS_SET ( ch->off_flags, OFF_TRIP ) ) 
          || ( !IS_NPC ( ch ) 
               && ch->level < skill_table[gsn_trip].skill_level[ch->pcdata->pclass] ) )
     {
          send_to_char ( "Tripping?  What's that?\n\r", ch );
          return;
     }

     if ( arg[0] == '\0' )
     {
          victim = ch->fighting;
          if ( victim == NULL )
          {
               send_to_char ( "But you aren't fighting anyone!\n\r", ch );
               return;
          }
     }

     else if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( victim->fighting != NULL &&
          !is_same_group ( ch, victim->fighting ) )
     {
          send_to_char ( "Kill stealing is not permitted.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( victim, AFF_FLYING ) )
     {
          act ( "$S feet aren't on the ground.", ch, NULL, victim,
                TO_CHAR );
          return;
     }

     if ( victim->position < POS_FIGHTING )
     {
          act ( "$N is already down.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "You fall flat on your face!\n\r", ch );
          WAIT_STATE ( ch, 2 * skill_table[gsn_trip].beats );
          act ( "$n trips over $s own feet!", ch, NULL, NULL,
                TO_ROOM );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
     {
          act ( "$N is your beloved master.", ch, NULL, victim,
                TO_CHAR );
          return;
     }

    /* modifiers */

    /* size */
     if ( ch->size < victim->size )
          chance += ( ch->size - victim->size ) * 10;	/* bigger = harder to trip */

    /* dex */
     chance += get_curr_stat ( ch, STAT_DEX );
     chance -= get_curr_stat ( victim, STAT_DEX ) * 3 / 2;

    /* speed */
     if ( IS_SET ( ch->off_flags, OFF_FAST ) ||
          IS_AFFECTED ( ch, AFF_HASTE ) )
          chance += 10;
     if ( IS_SET ( victim->off_flags, OFF_FAST ) ||
          IS_AFFECTED ( victim, AFF_HASTE ) )
          chance -= 20;

    /* level */
     chance += ( ch->level - victim->level ) * 2;

    /* now the attack */
     if ( number_percent (  ) < chance )
     {
          act ( "$n trips you and you go down!", ch, NULL, victim,
                TO_VICT );
          act ( "You trip $N and $N goes down!", ch, NULL, victim,
                TO_CHAR );
          act ( "$n trips $N, sending $M to the ground.", ch, NULL,
                victim, TO_NOTVICT );
          check_improve ( ch, gsn_trip, TRUE, 1 );

          WAIT_STATE ( victim, 2 * PULSE_VIOLENCE );
          WAIT_STATE ( ch, skill_table[gsn_trip].beats );
          victim->position = POS_RESTING;
          damage ( ch, victim,
                   number_range ( 2, 2 + 2 * victim->size ),
                   gsn_trip, DAM_BASH, TRUE );
     }
     else
     {
          damage ( ch, victim, 0, gsn_trip, DAM_BASH, TRUE );
          WAIT_STATE ( ch, skill_table[gsn_trip].beats * 2 / 3 );
          check_improve ( ch, gsn_trip, FALSE, 1 );
     }
}

void do_kill ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( IS_AFFECTED ( ch, AFF_FEAR ) )
     {
          if ( !IS_NPC ( ch ) )
               send_to_char  ( "You are too scared to kill anyone...\n\r", ch );
          return;
     }

     if ( CAN_DETECT( ch, AFF_CONFUSION ) )
     {
          if ( !IS_NPC( ch ) )
               send_to_char ("You'd probably hit yourself by mistake...\n\r", ch);
          return;
     }

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Kill whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( victim ) )
     {
          if ( !IS_SET ( victim->act, PLR_KILLER )
               && !IS_SET ( victim->act, PLR_THIEF ) )
          {
               send_to_char ( "You must MURDER players.\n\rAlso, unless you are a killer, you may only do so in PKILL areas.\n\r", ch );
               return;
          }
     }

     if ( victim == ch )
     {
          send_to_char ( "You hit yourself.  Ouch!\n\r", ch );
          multi_hit ( ch, ch, TYPE_UNDEFINED );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( ( victim->fighting != NULL &&
            !is_same_group ( ch, victim->fighting ) ) &&
          ( is_safe ( ch, victim, TRUE ) ) )
          return;

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
     {
          act ( "$N is your beloved master.", ch, NULL, victim,
                TO_CHAR );
          return;
     }

     if ( ch->position == POS_FIGHTING )
     {
          send_to_char ( "You do the best you can!\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, PULSE_VIOLENCE );
     send_to_char ( "You attack!\n\r", ch );
     multi_hit ( ch, victim, TYPE_UNDEFINED );
     return;
}

void do_murde ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to MURDER, spell it out.\n\r", ch );
     return;
}

void do_murder ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Murder whom?\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_FEAR ) )
     {
          send_to_char ( "You are too scared to attack anyone...\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM ) ||
          ( IS_NPC ( ch ) && IS_SET ( ch->act, ACT_PET ) ) )
          return;

     if ( IS_NPC ( ch ) && IS_SET ( ch->act, ACT_FOLLOWER ) )
          return;

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "Suicide is a mortal sin.\n\r", ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( ( victim->fighting != NULL &&
            !is_same_group ( ch, victim->fighting ) ) &&
          ( is_safe ( ch, victim, TRUE ) ) )
          return;

     if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
     {
          act ( "$N is your beloved master.", ch, NULL, victim,
                TO_CHAR );
          return;
     }

     if ( ch->position == POS_FIGHTING )
     {
          send_to_char ( "You do the best you can!\n\r", ch );
          return;
     }

     if ( !is_safe ( ch, victim, TRUE ) )
     {
          WAIT_STATE ( ch, PULSE_VIOLENCE );
          if ( IS_NPC ( ch ) )
               SNP ( buf, "Help! I am being attacked by %s!", ch->short_descr );
          else
               SNP ( buf, "Help!  I am being attacked by %s!", ch->name );
          do_yell ( victim, buf );

          multi_hit ( ch, victim, TYPE_UNDEFINED );
          return;
     }
     return;
}

void do_backstab ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     int	skill;

     one_argument ( argument, arg );

     if ( IS_AFFECTED ( ch, AFF_FEAR ) )
     {
          send_to_char
               ( "You are too scared to attack anyone...\n\r", ch );
          return;
     }
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Backstab whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "How can you sneak up on yourself?\n\r",
                         ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;			/* this will check the pk flag and bail out if none */

     if ( !IS_NPC ( victim ) && is_safe ( ch, victim, TRUE ) )
          return;

     if ( ( victim->fighting != NULL && !is_same_group ( ch, victim->fighting ) ) &&
          ( is_safe(ch, victim, TRUE) ) )
          return;

     if ( ( obj = get_eq_char ( ch, WEAR_WIELD ) ) == NULL )
     {
          send_to_char
               ( "You need to wield a weapon to backstab.\n\r", ch );
          return;
     }

     if ( victim->fighting != NULL )
     {
          send_to_char ( "You can't backstab a fighting person.\n\r",
                         ch );
          return;
     }

     if ( victim->hit < victim->max_hit )
     {
          act ( "$N is hurt and suspicious ... you can't sneak up.",
                ch, NULL, victim, TO_CHAR );
          return;
     }

     skill = get_skill ( ch, gsn_backstab );

     if ( !can_see ( ch, victim ) )
          skill /=2;

     if ( victim->position < POS_FIGHTING )
          skill += 25;

     if ( victim->position < POS_RESTING )
          skill += 25;

     if (skill > 100)
          skill = 100;
     if (skill < 1)
          skill = 1;

     WAIT_STATE ( ch, skill_table[gsn_backstab].beats );

     if (!IS_AWAKE(victim) || chance(skill) )
     {
          check_improve ( ch, gsn_backstab, TRUE, 1 );

          if (victim->sex == SEX_MALE)
               sound ("bs.wav", ch);
          else if (victim->sex == SEX_FEMALE)
               sound ("bs_fem.wav", ch);
          else
               sound ("bs_male.wav", ch);

          multi_hit ( ch, victim, gsn_backstab );
     }
     else
     {
          check_improve ( ch, gsn_backstab, FALSE, 1 );
          damage ( ch, victim, 0, gsn_backstab, DAM_NONE, TRUE );
     }

     return;
}

void do_circle ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     int skill;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Circle whom?\n\r", ch );
          return;
     }

     else if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( IS_NPC ( victim ) &&
          victim->fighting != NULL &&
          !is_same_group ( ch, victim->fighting )
          && ( is_safe ( ch,victim, TRUE ) ) )
          return;

     if ( ( obj = get_eq_char ( ch, WEAR_WIELD ) ) == NULL )
     {
          send_to_char ( "You need to wield a weapon to circle.\n\r", ch );
          return;
     }

     if ( ( victim = ch->fighting ) == NULL )
     {
          send_to_char ( "You must be fighting in order to circle.\n\r", ch );
          return;
     }

     if ( ( !IS_AFFECTED ( victim, AFF_BLIND ) ) && ( victim->fighting == ch ) )
     {
          send_to_char ( "Your foe is watching you too closely, you can't circle.\n\r", ch );
          return;
     }

     skill = get_skill (ch, gsn_circle);

     if ( !can_see ( ch, victim ) )
          skill /=2;

     if ( victim->position < POS_FIGHTING )
          skill += 25;

     if ( victim->position < POS_RESTING )
          skill += 25;

     if (skill > 100)
          skill = 100;
     if (skill < 1)
          skill = 1;

     WAIT_STATE ( ch, skill_table[gsn_circle].beats );

     if (chance(skill) || (skill >=2 && !IS_AWAKE(victim) ) )
     {
          check_improve ( ch, gsn_circle, TRUE, 1 );
          sound ("circle.wav", ch);
          one_hit ( ch, victim, gsn_circle, FALSE, skill );
          WAIT_STATE ( ch, skill_table[gsn_circle].beats );
     }
     else
     {
          check_improve ( ch, gsn_circle, FALSE, 1 );
          damage ( ch, victim, 0, gsn_circle, DAM_NONE, TRUE );
          WAIT_STATE ( ch, skill_table[gsn_circle].beats );
     }

     return;
}

void do_flee ( CHAR_DATA * ch, char *argument )
{
     ROOM_INDEX_DATA    *was_in;
     ROOM_INDEX_DATA    *now_in;
     CHAR_DATA          *victim;
     int                 attempt;

     if ( ( victim = ch->fighting ) == NULL )
     {
          if ( ch->position == POS_FIGHTING )
               ch->position = POS_STANDING;
          send_to_char ( "You aren't fighting anyone.\n\r", ch );
          return;
     }

     was_in = ch->in_room;
     for ( attempt = 0; attempt < 6; attempt++ )
     {
          EXIT_DATA          *pexit;
          int                 door;

          door = number_door (  );
          if ( ( pexit = was_in->exit[door] ) == 0
               || pexit->u1.to_room == NULL
               || IS_SET ( pexit->exit_info, EX_CLOSED )
               || ( IS_NPC ( ch )
                    && IS_SET ( pexit->u1.to_room->room_flags,
                                ROOM_NO_MOB ) ) )
               continue;

          move_char ( ch, door, FALSE );
          if ( ( now_in = ch->in_room ) == was_in )
               continue;

          ch->in_room = was_in;
          act ( "$n has fled!", ch, NULL, NULL, TO_ROOM );
          ch->in_room = now_in;

          if ( !IS_NPC ( ch ) && ch->pcdata->mortal )
          {
               send_to_char ( "You flee from combat!  You lose 10 xp.\n\r", ch );
               gain_exp ( ch, -10 );
          }

          if ( !IS_NPC ( ch ) && !ch->pcdata->mortal )
          {
               send_to_char ("You flee from combat! You lose 25 xp.\n\r", ch);
               gain_exp ( ch, -25 );
          }

          stop_fighting ( ch, TRUE );
          return;
     }

     send_to_char ( "PANIC! You couldn't escape!\n\r", ch );
     return;
}

void do_sharpen ( CHAR_DATA * ch, char *argument )
{
     char                target[MAX_INPUT_LENGTH];
     int                 roll;
     OBJ_DATA           *obj;
     int                 move = 175;
     int                 all_sharp = WEAPON_SHARP + WEAPON_VORPAL;
     int                 player_chance;

     if ( ch->pcdata->learned[gsn_sharpen] < 1 )
     {
          send_to_char ( "You have no clue how to sharpen your weapon.\n\r", ch );
          return;
     }
     one_argument ( argument, target );
     if ( target[0] == '\0' )
     {
          send_to_char ( "Sharpen what?\n\r", ch );
          return;
     }

     if ( ch->move < move )	/*not enough to try */
     {
          send_to_char ( "You do not have enough movement.\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, target, NULL ) ) == NULL )
     {
          send_to_char ( "You are not carrying that.\n\r", ch );
          return;
     }

     if ( obj->item_type != ITEM_WEAPON )	/*not a weapon */
     {
          send_to_char ( "That is not a weapon.\n\r", ch );
          return;
     }

     if ( obj->wear_loc != -1 )
     {
          send_to_char ( "The weapon must be carried to be sharpened.\n\r", ch );
          return;
     }

     ch->move -= move;		/*use up required move */

     if ( ( obj->value[4] & all_sharp ) != 0 )	/*already sharp or vorpal */
     {
          send_to_char ( "This weapon is already as sharp as it can be...\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, skill_table[gsn_sharpen].beats );
    /*lets see what happens */
     roll = number_percent (  );
    /*chance is skill/4 + level/4 + (str-20) + (dex -18) */
     player_chance =
          ( int ) ( ch->level / 4 ) +
          ( int ) ( get_skill ( ch, gsn_sharpen ) / 4 ) +
          ( get_curr_stat ( ch, STAT_STR ) - 20 ) +
          ( get_curr_stat ( ch, STAT_DEX ) - 18 ) + 5;

     if ( roll <= 5 )		/*oops, ruins the weapon */
     {
          send_to_char ( "You have dulled the edge beyond repair.  The weapon is worthless!\n\r", ch );
          extract_obj ( obj );
          act ( "$n tries to sharpen $p and fails miserably.", ch,
                obj, NULL, TO_ROOM );
          check_improve ( ch, gsn_sharpen, FALSE, 1 );
          return;
     }

     else if ( roll <= player_chance )	/*success! */
     {
          int                 flag_to_add = WEAPON_SHARP;
          char                to_ch[128];
          char                to_room[128];

	/*check for ultimate vorpal roll */
          roll = number_percent (  );
          if ( roll > 95 )	/*vorpal! */
          {
               flag_to_add = WEAPON_VORPAL;
               SNP ( to_ch,   "The gods silently assist you!  You've created a {mvorpal{x weapon!\n\r" );
               SNP ( to_room, "$n is aided by powers unseen, and thus creates a {mvorpal{x weapon!" );
          }
          else
          {
               SNP ( to_ch,   "With utmost care, you sharpen your weapon to a fine edge.\n\r" );
               SNP ( to_room, "$n works with great care and makes $s weapon deadlier." );
          }

          send_to_char ( to_ch, ch );
          act ( to_room, ch, obj, NULL, TO_ROOM );
          check_improve ( ch, gsn_sharpen, TRUE, 1 );

          obj->value[4] += flag_to_add;	/*add sharp or vorpal */
          obj->valueorig[4] += flag_to_add;
     }
     else			/*fail */
     {
          send_to_char ( "You fail to improve your weapon.\n\r", ch );
          check_improve ( ch, gsn_sharpen, FALSE, 1 );
          return;
     }
}
/*end do_sharpen */

void do_rescue ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     CHAR_DATA          *fch;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Rescue whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim == ch )
     {
          send_to_char ( "What about fleeing instead?\n\r", ch );
          return;
     }

     if ( !is_same_group ( ch, victim ) )
     {
          send_to_char ( "Kill stealing is not permitted.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( ch ) && IS_NPC ( victim ) )
     {
          send_to_char ( "Doesn't need your help!\n\r", ch );
          return;
     }

     if ( ch->fighting == victim )
     {
          send_to_char ( "Too late.\n\r", ch );
          return;
     }

     if ( ( fch = victim->fighting ) == NULL )
     {
          send_to_char ( "That person is not fighting right now.\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, skill_table[gsn_rescue].beats );
     if ( !IS_NPC ( ch ) &&
          number_percent (  ) > ch->pcdata->learned[gsn_rescue] )
     {
          send_to_char ( "You fail the rescue.\n\r", ch );
          check_improve ( ch, gsn_rescue, FALSE, 1 );
          return;
     }

     act ( "You rescue $N!", ch, NULL, victim, TO_CHAR );
     act ( "$n rescues you!", ch, NULL, victim, TO_VICT );
     act ( "$n rescues $N!", ch, NULL, victim, TO_NOTVICT );
     check_improve ( ch, gsn_rescue, TRUE, 1 );

     stop_fighting ( fch, FALSE );
     stop_fighting ( victim, FALSE );

     set_fighting ( ch, fch );
     set_fighting ( fch, ch );
     return;
}

void do_rotate ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 chance;

     if ( ch->fighting == NULL )
     {
          send_to_char ( "You aren't fighting anyone.\n\r", ch );
          return;
     }

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Rotate to fight whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( victim == ch->fighting )
     {
          send_to_char ( "You are already fighting that target!.\n\r", ch );
          return;
     }

     if ( victim->fighting != ch )
     {
          send_to_char ( "You must rotate to someone who is already fighting you.\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, skill_table[gsn_rotate].beats );
     chance = ch->pcdata->learned[gsn_rotate];
    /*modifiers */
     chance = chance + ( ch->level - ch->fighting->level ) * 5;
     chance = chance + ( get_curr_stat ( ch, STAT_DEX ) - 20 ) * 5;
     if ( number_percent (  ) > chance )
     {
          send_to_char ( "Rotation failed.\n\r", ch );
          check_improve ( ch, gsn_rotate, FALSE, 1 );
          return;
     }
     stop_fighting ( ch, FALSE );
     set_fighting ( ch, victim );
     send_to_char ( "Rotation successful.\n\r", ch );
     check_improve ( ch, gsn_rotate, TRUE, 1 );
     return;
}

void do_kick ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;

     if ( !IS_NPC ( ch )
          && ch->level < skill_table[gsn_kick].skill_level[ch->pcdata->pclass] )
     {
          send_to_char ( "You better leave the martial arts to fighters.\n\r", ch );
          return;
     }

     if ( IS_NPC ( ch ) && !IS_SET ( ch->off_flags, OFF_KICK ) )
          return;

     if ( ( victim = ch->fighting ) == NULL )
     {
          send_to_char ( "You aren't fighting anyone.\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, skill_table[gsn_kick].beats );
     if ( IS_NPC ( ch ) || number_percent (  ) < ch->pcdata->learned[gsn_kick] )
     {
          damage ( ch, victim, number_range ( 1, ch->level ), gsn_kick, DAM_HANDTOHAND, TRUE );
          check_improve ( ch, gsn_kick, TRUE, 1 );
     }
     else
     {
          damage ( ch, victim, 0, gsn_kick, DAM_HANDTOHAND, TRUE );
          check_improve ( ch, gsn_kick, FALSE, 1 );
     }

     return;
}

/* Zeran - fairly big modification, still is sloppy though :( */
void do_disarm ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     OBJ_DATA           *obj2;
     OBJ_DATA           *chwep;
     OBJ_DATA           *chwep2;
     int                 ch_tot_weap = 0;
     int                 vict_tot_weap = 0;
     int                 chance1, chance2, hth, ch_weapon1,
          vict_weapon1, ch_vict_weapon1;
     int                 ch_weapon2, vict_weapon2, ch_vict_weapon2;
     bool                got_one = FALSE;

     hth = 0;

     if ( ( chance1 = get_skill ( ch, gsn_disarm ) ) == 0 )
     {
          send_to_char ( "You don't know how to disarm opponents.\n\r", ch );
          return;
     }
     chance2 = chance1;

     if ( ( victim = ch->fighting ) == NULL )
     {
          send_to_char ( "You aren't fighting anyone.\n\r", ch );
          return;
     }

    /* Zeran - ok, grab all possible weapons and total them up */
     if ( ( obj = get_eq_char ( victim, WEAR_WIELD ) ) != NULL )
          vict_tot_weap++;
     if ( ( obj2 = get_eq_char ( victim, WEAR_WIELD2 ) ) != NULL )
          vict_tot_weap++;
     if ( ( chwep = get_eq_char ( ch, WEAR_WIELD ) ) != NULL )
          ch_tot_weap++;
     if ( ( chwep2 = get_eq_char ( ch, WEAR_WIELD2 ) ) != NULL )
          ch_tot_weap++;

     if ( chwep == NULL
          && chwep2 == NULL
          && ( ( hth = get_skill ( ch, gsn_hand_to_hand ) ) == 0
               || ( IS_NPC ( ch ) &&
                    !IS_SET ( ch->off_flags, OFF_DISARM ) ) ) )
     {
          send_to_char ( "You must wield a weapon to disarm.\n\r",
                         ch );
          return;
     }

     if ( ( victim = ch->fighting ) == NULL )
     {
          send_to_char ( "You aren't fighting anyone.\n\r", ch );
          return;
     }

     if ( obj == NULL && obj2 == NULL )
     {
          send_to_char ( "Your opponent is not wielding a weapon.\n\r", ch );
          return;
     }

    /* find weapon skills */
     ch_weapon1 =
          get_weapon_skill ( ch, get_weapon_sn ( ch, FALSE ) );
     vict_weapon1 =
          get_weapon_skill ( victim,
                             get_weapon_sn ( victim, FALSE ) );
     ch_weapon2 =
          get_weapon_skill ( ch, get_weapon_sn ( ch, TRUE ) );
     vict_weapon2 =
          get_weapon_skill ( victim,
                             get_weapon_sn ( victim, TRUE ) );
     ch_vict_weapon1 =
          get_weapon_skill ( ch, get_weapon_sn ( victim, FALSE ) );
     ch_vict_weapon2 =
          get_weapon_skill ( ch, get_weapon_sn ( victim, TRUE ) );

    /* modifiers */

    /* skill */
     if ( chwep == NULL && chwep2 == NULL )
     {
          chance1 = chance1 * hth / 150;
          chance2 = chance2 * hth / 150;
     }
     else
     {
          chance1 = chance1 * ch_weapon1 / 100;
          chance2 = chance2 * ch_weapon2 / 100;
     }

     chance1 += ( ch_vict_weapon1 / 2 - vict_weapon1 ) / 2;
     chance2 += ( ch_vict_weapon2 / 2 - vict_weapon2 ) / 2;

    /* dex vs. strength */
     chance1 += get_curr_stat ( ch, STAT_DEX );
     chance1 -= 2 * get_curr_stat ( victim, STAT_STR );
     chance2 += get_curr_stat ( ch, STAT_DEX );
     chance2 -= 2 * get_curr_stat ( victim, STAT_STR );

    /* level */
     chance1 += ( ch->level - victim->level ) * 2;
     chance2 += ( ch->level - victim->level ) * 2;

    /* Zeran - consider 2 weapons against 1, or vice versa */
     if ( ch_tot_weap > vict_tot_weap )
     {
          chance1 += ( 20 * ch_vict_weapon1 / 100 );
          chance2 += ( 20 * ch_vict_weapon2 / 100 );
     }
     else if ( ch_tot_weap < vict_tot_weap )
     {
          chance1 -= ( 20 * ch_vict_weapon1 / 100 );
          chance2 -= ( 20 * ch_vict_weapon2 / 100 );
     }
    /*Make second weapon harder to disarm if victim has 2 weapons */
     if ( obj != NULL && obj2 != NULL )
          chance2 = chance2 / 2;

    /* and now the attack */
     WAIT_STATE ( ch, skill_table[gsn_disarm].beats );

     if ( obj != NULL )
     {
          if ( number_percent (  ) < chance1 )
          {
               disarm ( ch, victim, FALSE );
               check_improve ( ch, gsn_disarm, TRUE, 1 );
               got_one = TRUE;
          }
     }

     if ( obj2 != NULL )
     {
          if ( number_percent (  ) < chance2 )
          {
               disarm ( ch, victim, TRUE );
               check_improve ( ch, gsn_disarm, TRUE, 1 );
               got_one = TRUE;
          }
     }

     if ( !got_one )
     {
          act ( "You fail to disarm $N.", ch, NULL, victim,
                TO_CHAR );
          act ( "$n tries to disarm you, but fails.", ch, NULL,
                victim, TO_VICT );
          act ( "$n tries to disarm $N, but fails.", ch, NULL,
                victim, TO_NOTVICT );
          check_improve ( ch, gsn_disarm, FALSE, 1 );
     }
     return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *mob;
     if ( (mob = ch->fighting) == NULL )
     {
          send_to_char( "But you're not fighting!\n\r", ch );
          return;
     }
     act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
     act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
     act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
     stop_fighting( ch, TRUE );

     if ( !IS_NPC( ch ) && IS_NPC( mob )
          &&   ( !HAS_TRIGGER_MOB( mob, TRIG_SURR )
                 || !p_percent_trigger( mob, NULL, NULL, ch, NULL, NULL, TRIG_SURR ) ) )
     {
          act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
          multi_hit( mob, ch, TYPE_UNDEFINED );
     }
}

void do_sla ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "If you want to SLAY, spell it out.\n\r", ch );
     return;
}

void do_slay ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;
     char                arg[MAX_INPUT_LENGTH];

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Slay whom?\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     if ( ch == victim )
     {
          send_to_char ( "Suicide is a mortal sin.\n\r", ch );
          return;
     }

     if ( !IS_NPC ( victim ) && victim->level >= get_trust ( ch ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     act ( "You slay $M in cold blood!", ch, NULL, victim,
           TO_CHAR );
     act ( "$n slays you in cold blood!", ch, NULL, victim,
           TO_VICT );
     act ( "$n slays $N in cold blood!", ch, NULL, victim,
           TO_NOTVICT );
     raw_kill ( victim );
     return;
}

bool check_material_vuln ( OBJ_DATA * obj, CHAR_DATA * victim )
{
     long                vuln;

     if ( obj == NULL )
     {
          bugf ( "Null obj passed to check_material_vuln" );
          return FALSE;
     }
     if ( victim == NULL )
     {
          bugf ( "Null victim passed to check_material_vuln" );
          return FALSE;
     }
     if ( !str_cmp ( material_name ( obj->material ), "unknown" ) )
     {
          return FALSE;
     }
     vuln = material_vuln ( obj->material );
     if ( IS_SET ( victim->vuln_flags, vuln ) )
          return TRUE;
     return FALSE;
}

void do_rally(CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *gch;
     int heal;
     int chance;

     if ((chance = get_skill(ch, gsn_rally)) == 0 )
     {
          send_to_char("You scream and holler, but nobody listens.\n\r",ch);
          return;
     }

     if (IS_AFFECTED(ch,AFF_CALM))
     {
          send_to_char("You're feeling to mellow to lead your companions on.\n\r",ch);
          return;
     }

     if (IS_AFFECTED(ch, AFF_FEAR ) )
     {
          send_to_char("You'd rather run away right now!\n\r", ch);
          return;
     }

     if (ch->position == POS_FIGHTING)
          chance += 25;

     if (number_percent() > chance)
     {
          send_to_char("You try to rally your companions, but your efforts fail.\n\r", ch);
          act("{R$n tries to rally $s companions, but fails.{w",ch,NULL,NULL,TO_ROOM);
          WAIT_STATE(ch,PULSE_VIOLENCE * 2);
          return;
     }

     send_to_char("You call your companions to arms, promising them glory!\n\r", ch);
     act("{R$n calls $s companions to arms, promising great glory!{w", ch, NULL, NULL, TO_ROOM);

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( gch != ch && is_same_group(ch, gch) && !IS_AFFECTED(gch, AFF_CALM) )
          {
		/* Let's do some kickin azz */
               if ( !is_affected(gch,gsn_rally) )
               {
                    AFFECT_DATA af;
                    send_to_char ("You answer the call to arms!\n\r", gch);

                    check_improve(ch,gsn_rally,TRUE,2);

                    af.type         = gsn_rally;
                    af.level        = ch->level;
                    af.duration     = number_fuzzy(UMAX(2,ch->level / 8) );
                    af.modifier     = UMAX(1,ch->level/5);

                    af.bitvector = 0;

                    af.location     = APPLY_HITROLL;
                    af.where = TO_AFFECTS;
                    affect_to_char(gch,&af);

                    af.location     = APPLY_DAMROLL;
                    affect_to_char(gch,&af);

                    af.modifier = UMAX(1, ch->level/10);
                    af.location   = APPLY_STR;
                    affect_to_char(gch,&af);

               }
		/* Always do this part */

               heal = dice(1, ch->level) + ch->level / 3;
               gch->hit = UMIN( gch->hit + heal, gch->max_hit );
               send_to_char("You feel better!\n\r", gch);

               update_pos( gch );
          }
     }

     WAIT_STATE(ch,PULSE_VIOLENCE * 2);

}

/*
 * Return a body part to hit - Lotherius
 * For this to work correctly, all the hitpct values in wear_info must add up to 100.
 */
int hpart ( CHAR_DATA *ch, CHAR_DATA *vict )
{
     int  i, b, c, count;

     c = 0;
     count = 0;

     while ( 1 )
     {
          if ( count > 50 )
          {
               bugf ( "fight.c: int hpart caught loop, returning -1" );
               return -1;
          }
          b = number_percent ( );	// Get a roll
          for ( i = 0; i < MAX_WEAR; i++ )
          {
               if ( wear_info[i].ispart == FALSE )			// If it isn't a part, skip
                    continue;
               if ( ( wear_info[i].part_req > 0 )			// If the victim doesn't have that part, skip
                    && ( !IS_SET ( vict->parts, wear_info[i].part_req ) ) )
                    continue;
               if ( wear_info[i].hitpct == 0 )				// If can't hit this part, skip
                    continue;
               c += wear_info[i].hitpct;				// add the percent for this item.
               if ( c >= b )
                    return i;
               count++;
          }
     }

     return i;
}
