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
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN ( do_look );
DECLARE_DO_FUN ( do_say );
DECLARE_DO_FUN ( do_flee );

/* prototype from db.c */
ROOM_INDEX_DATA *get_random_room args ( ( int filt, CHAR_DATA *ch ) );

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
extern char *target_name;

/*
 * Spell functions.
 */

void spell_absorb_magic ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     AFFECT_DATA         af;

     if ( IS_PROTECTED ( ch, PROT_ABSORB ) )
     {
          send_to_char ( "You are already able to absorb magic.\n\r", ch );
          return;
     }
     set_affect ( &af, sn, level, number_fuzzy ( level / 6 ), APPLY_NONE, 0, TO_PROTECTIONS, PROT_ABSORB, ch->name );
     affect_to_char ( ch, &af );
     act ( "$n is surrounded by a strange mystical aura.", ch, NULL,  NULL, TO_ROOM );
     send_to_char ( "You are surrounded by a mystical absorbing aura.\n\r", ch );
     return;
}

void spell_acid_blast ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = dice ( level, 12 );
     check_damage_obj ( victim, NULL, 1, DAM_ACID );
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_ACID, TRUE );
     return;
}

void spell_entropy ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( !saves_spell ( level, victim ) )
     {
          check_damage_obj ( victim, NULL, 100, DAM_NEGATIVE );	/* gotcher equipment sucker! */
          act ( "A blast of chaotic magic surronds $N.", ch, NULL, victim, TO_ROOM );
          act ( "You release a blast of entropy to $N.", ch, NULL, victim, TO_CHAR );
          act ( "A blast of chaotic energy screaches from $n's hand and surrounds you!", ch, NULL, victim, TO_VICT );
          return;
     }
     else
     {
          send_to_char ( "Your spell backfires!\n\r", ch );
          check_damage_obj ( ch, NULL, 15, DAM_NEGATIVE );
          act ( "$N fails yet another spell.", ch, NULL, victim, TO_ROOM );
          act ( "A blast of chaotic energy screaches from $n's hand, but fails to reach you.", ch, NULL, victim, TO_VICT );
          return;
     }
     return;
}

void spell_animate ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj;
     CHAR_DATA          *mob;
     int                 i;
     int                 number;
     int                 type;
     int                 bonus;

     obj = get_obj_here ( ch, NULL, target_name );

     if ( obj == NULL )
     {
          send_to_char ( "Resurrect what?\n\r", ch );
          return;
     }

    /* Nothing but NPC corpses. */

     if ( obj->item_type != ITEM_CORPSE_NPC )
     {
          if ( obj->item_type == ITEM_CORPSE_PC )
               send_to_char ( "You can't resurrect players.\n\r", ch );
          else
               send_to_char ( "It would serve no purpose...\n\r", ch );
          return;
     }

     if ( obj->level > ( ch->level + 2 ) )
     {
          send_to_char ( "You couldn't call forth such a great zombie.\n\r", ch );
          return;
     }

     if ( ch->pet != NULL )
     {
          send_to_char ( "You already have a pet.\n\r", ch );
          return;
     }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */

     mob = create_mobile ( get_mob_index ( MOB_VNUM_ZOMBIE ) );
     mob->level = obj->level;

     mob->max_hit = mob->level * 8 + number_range ( mob->level * mob->level / 4, mob->level * mob->level );

     mob->max_hit *= .9;
     mob->hit = mob->max_hit;
     mob->max_mana = 100 + dice ( mob->level, 10 );
     mob->mana = mob->max_mana;
     // No armor for mobs that aren't wearing equipment.
     //    for ( i = 0; i < 3; i++ )
     //	mob->armor[i] = interpolate ( mob->level, 100, -100 );
     //    mob->armor[3] = interpolate ( mob->level, 100, 0 );
     //
     number = UMIN ( mob->level / 4 + 1, 5 );
     type = ( mob->level + 7 ) / number;
     bonus = UMAX ( 0, mob->level * 9 / 4 - number * type );

     mob->damage[DICE_NUMBER] = number;
     mob->damage[DICE_TYPE] = type;
     mob->damage[DICE_BONUS] = bonus;

     for ( i = 0; i < MAX_STATS; i++ )
          mob->perm_stat[i] = 11 + mob->level / 4;

    /* You rang? */
     char_to_room ( mob, ch->in_room );
     act ( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_ROOM );
     act ( "$p springs to life as a hideous zombie!", ch, obj, NULL, TO_CHAR );
     extract_obj ( obj );

     /* Yessssss, massssssster... */
     SET_BIT ( mob->affected_by, AFF_CHARM );
     SET_BIT ( mob->act, ACT_FOLLOWER );
     mob->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
     add_follower ( mob, ch );

     mob->leader = ch;
     mob->master = ch; /* This wasn't getting set! */
     ch->pet = mob;
     /* For a little flavor... */
     do_say ( mob, "How may I serve you, master?" );
     return;
}

void spell_minor_creation ( int sn, int leve, CHAR_DATA * ch,  void *vo )
{
     OBJ_DATA           *obj;

     /* Minor Creation by Lotherius
      run through tons of if checks... sloppy but simple.
      rather use case but didn't wanna bother... just add another
      if check if you need to add a new item.
      */

     if ( !str_cmp ( target_name, "boat" ) )
     {
          obj = create_object ( get_obj_index ( OBJ_VNUM_BOAT ), 0 );
          obj->cost = 0;
          obj_to_room ( obj, ch->in_room );
          act ( "$n mutters some words and $p materializes from thin air.", ch, obj, NULL, TO_ROOM );
          send_to_char ( "A boat materializes before you.\n\r", ch );
          return;
     }
     if ( !str_cmp ( target_name, "bag" ) )
     {
          obj = create_object ( get_obj_index ( OBJ_VNUM_BAG ), 0 );
          obj->cost = 0;
          obj_to_room ( obj, ch->in_room );
          act ( "$n mutters some words and $p materializes from thin air.", ch, obj, NULL, TO_ROOM );
          send_to_char ( "A bag materializes before you.\n\r", ch );
          return;
     }

     /* item not found */

     send_to_char ( "You don't know how to make that kind of object.", ch );
     return;
}

void spell_soul_blade ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj;
     OBJ_DATA           *sblade;
     char		buf[MAX_STRING_LENGTH];
     int                 number, type;

     obj = get_obj_here ( ch, NULL, target_name );

     if ( obj == NULL )
     {
          send_to_char ( "Cast Soul Blade on What?\n\r", ch );
          return;
     }

    /* Check Corpses. */

     if ( obj->item_type != ITEM_CORPSE_NPC )
     {
          if ( obj->item_type == ITEM_CORPSE_PC )
          {
               if (obj->contains)
               {
                    send_to_char ( "This player's corpse is still too fresh.\n\r", ch );
                    return;
               }
          }
          else
          {
               send_to_char ( "It would serve no purpose...\n\r", ch );
               return;
          }
     }

     if ( obj->level > ( ch->level + 4 ) )
     {
          send_to_char ( "You cannot forge such a soul into a blade.\n\r",ch );
          return;
     }

     /* Create the soulblade */

     sblade = create_object ( get_obj_index ( OBJ_VNUM_SOULBLADE ), 0 );

     if (obj->item_type == ITEM_CORPSE_PC)
     {
          if ( !obj->owner[0] )
          {
               send_to_char("There doesn't appear to be any soul left in this corpse.\n\r", ch);
               return;
          }
          if (!str_cmp(ch->name, obj->owner))
          {
               send_to_char("Somehow you feel that would be a bad thing.\n\r", ch);
               return;
          }

          /* Okay we can use this corpse. */

          free_string (sblade->name);
          free_string (sblade->short_descr);
          free_string (sblade->description);

          SNP( buf, "%s soul blade", obj->owner );
          sblade->name = str_dup ( buf );

          SNP( buf, "{WT{Bhe {WS{Boul {WO{Bf {Y%s{w", obj->owner );
          sblade->short_descr = str_dup ( buf );

          SNP( buf, "A blade forged from the soul of %s in the year %d by %s.",
               obj->owner, time_info.year, ch->name );
          sblade->description = str_dup ( buf) ;

          if (obj->level < ch->level)
               sblade->level = ch->level;
          else
               sblade->level = obj->level;

	/* Let's give it some zip */
          random_apply (sblade, ch);
     }
     else
     {
          sblade->level = obj->level;

          if (sblade->level < 5 )
               sblade->level = 5;

          sblade->timer = ( sblade->level + 10 ) / 2;
     }

     number = (level / 15)+1;

     type = (number * 4) + ( (level - ( (number-1)*10 ))/3 );

     sblade->value[1] = number;
     sblade->value[2] = type;

     sblade->value[1] = number;
     sblade->value[2] = type;

     sblade->enchanted = TRUE;

    	/* Action! */
     obj_to_room ( sblade, ch->in_room );
     act ( "$n waves dramatically and $p appears.", ch, sblade,  NULL, TO_ROOM );
     act ( "You wave dramatically and $p appears.", ch, sblade,  NULL, TO_CHAR );
     sound ("church4.wav", ch);
     extract_obj ( obj );
     return;
}

void spell_armor ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already protected.\n\r", ch );
          else
               act ( "$N is already armored.", ch, NULL, victim, TO_CHAR );
          return;
     }
     
     set_affect ( &af, sn, level, 24, APPLY_AC, 20, TO_AFFECTS, 0, ch->name );     
     affect_to_char ( victim, &af );
     send_to_char ( "You feel someone protecting you.\n\r", victim );
     if ( ch != victim )
          act ( "$N is protected by your magic.", ch, NULL, victim, TO_CHAR );
     return;
}

void spell_bless ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( victim->position == POS_FIGHTING || is_affected ( victim, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already blessed.\n\r", ch );
          else
               act ( "$N already has divine favor.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( ch->alignment <= -250 && victim->alignment >= 250 )
     {
          act ( "$N's do-gooder ways are too soft for your deity's blessing.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if ( ch->alignment >= 250 && victim->alignment <= -250 )
     {
          act ( "$N's evil ways do not sit well with your deity.", ch, NULL, victim, TO_CHAR );
          return;
     }

     set_affect ( &af, sn, level, 6+level, APPLY_HITROLL, (level/8)+1, TO_AFFECTS, 0, ch->name );
     affect_to_char ( victim, &af );

     // Previous affects masked by following:
     af.location = APPLY_SAVING_SPELL;
     af.modifier = 0 - (level / 8)+1;     
     affect_to_char ( victim, &af );
     
     send_to_char ( "You feel righteous.\n\r", victim );
     if ( ch != victim )
          act ( "You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR );
     return;
}

void spell_blindness ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_BLIND ) || saves_spell ( level, victim ) )
          return;

     set_affect ( &af, sn, level, 1+level, APPLY_HITROLL, -4, TO_AFFECTS, AFF_BLIND, ch->name );
     affect_to_char ( victim, &af );     
     
     send_to_char ( "You are blinded!\n\r", victim );
     act ( "$n appears to be blinded.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_minor_chaos ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;
     int                 damtype;

     damtype = number_range ( 4, 17 );

     if ( chance(5) && !IS_NPC(ch) && ch->pcdata->pclass == CLASS_CHAOSMAGE )
     {
          send_to_char("You lose control of these chaotic forces!\n\r", ch );
          victim = ch;
     }

     dam = spell_calc ( ch, sn);

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, damtype, TRUE );
     return;
}

void spell_burning_hands ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA *victim = (CHAR_DATA *) vo;
     int dam;

     dam = spell_calc ( ch, sn );

     check_damage_obj ( victim, NULL, 1, DAM_FIRE );

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_FIRE, TRUE );
     return;
}

void spell_call_lightning ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;
     int                 dam;

     if ( !IS_OUTSIDE ( ch ) )
     {
          send_to_char ( "You must be out of doors.\n\r", ch );
          return;
     }

     if ( weather_info.sky < SKY_RAINING )
     {
          send_to_char ( "You need bad weather.\n\r", ch );
          return;
     }

     dam = spell_calc (ch , sn);
     send_to_char ( "Zeran's lightning strikes your foes!\n\r", ch );
     act ( "$n calls Zeran's lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM );
     sound ("SHOCKWAV.WAV", ch);

     for ( vch = char_list; vch != NULL; vch = vch_next )
     {
          vch_next = vch->next;
          if ( vch->in_room == NULL )
               continue;
          if ( vch->in_room == ch->in_room )
          {
               if ( vch != ch && ( IS_NPC ( ch ) ? !IS_NPC ( vch ) : IS_NPC ( vch ) ) )
               {
                    sound ("SHOCKWAV.WAV", vch);
                    check_damage_obj ( vch, NULL, 1, DAM_LIGHTNING );
                    damage ( ch, vch, saves_spell ( level, vch ) ? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE );
               }
               continue;
          }

          if ( vch->in_room->area == ch->in_room->area
               && IS_OUTSIDE ( vch ) && IS_AWAKE ( vch ) )
               send_to_char ( "Lightning flashes in the sky.\n\r", vch );
     }

     return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     int                 mlevel = 0;
     int                 count = 0;
     int                 high_level = 0;
     int                 chance;
     AFFECT_DATA         af;

    /* get sum of all mobile levels in the room */
     for ( vch = ch->in_room->people; vch != NULL;
           vch = vch->next_in_room )
     {
          if ( vch->position == POS_FIGHTING )
          {
               count++;
               if ( IS_NPC ( vch ) )
                    mlevel += vch->level;
               else
                    mlevel += vch->level / 2;
               high_level = UMAX ( high_level, vch->level );
          }
     }

    /* compute chance of stopping combat */
     chance = 4 * level - high_level + 2 * count;

     if ( IS_IMMORTAL ( ch ) )	/* always works */
          mlevel = 0;

     if ( number_range ( 0, chance ) >= mlevel )	/* hard to stop large fights */
     {
          for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
          {
               int mod;
               
               if ( IS_NPC ( vch ) &&
                    ( IS_SET ( vch->imm_flags, IMM_MAGIC ) ||
                      IS_SET ( vch->act, ACT_UNDEAD ) ) )
                    return;

               if ( IS_AFFECTED ( vch, AFF_CALM ) ||
                    IS_AFFECTED ( vch, AFF_BERSERK ) ||
                    is_affected ( vch, skill_lookup ( "frenzy" ) ) )
                    return;

               send_to_char ( "A wave of calm passes over you.\n\r", vch );

               if ( vch->fighting || vch->position == POS_FIGHTING )
                    stop_fighting ( vch, FALSE );

               if ( !IS_NPC (vch ) )
                    mod = -5;
               else
                    mod = -2;
               
               set_affect ( &af, sn, level, level/4, APPLY_HITROLL, mod, TO_AFFECTS, AFF_CALM, ch->name );
               affect_to_char ( vch, &af );
               
               // Previous af masked by following:
               af.location = APPLY_DAMROLL;
               affect_to_char ( vch, &af );
          }
     }
}

void spell_cancellation ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     bool                found = FALSE;

     level += 2;

     if ( ( !IS_NPC ( ch ) && IS_NPC ( victim ) &&
            !( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim ) ) ||
          ( IS_NPC ( ch ) && !IS_NPC ( victim ) ) )
     {
          send_to_char ( "You failed. Maybe this can be dispelled.\n\r", ch );
          return;
     }

    /* unlike dispel magic, the victim gets NO save */
    /* begin running through the spells */

     if ( check_dispel
          ( level, victim, skill_lookup ( "absorb magic" ) ) )
          found = TRUE;

     if ( check_dispel
          ( level, victim, skill_lookup ( "spirit armor" ) ) )
          found = TRUE;

     if ( check_dispel
          ( level, victim, skill_lookup ( "regeneration" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "armor" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "bless" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "blindness" ) ) )
     {
          found = TRUE;
          act ( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "calm" ) ) )
     {
          found = TRUE;
          act ( "$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "change sex" ) ) )
     {
          found = TRUE;
          act ( "$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM );
          send_to_char ( "You feel more like yourself again.\n\r", victim );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "charm person" ) ) )
     {
          found = TRUE;
          act ( "$n regains $s free will.", victim, NULL, NULL, TO_ROOM );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "chill touch" ) ) )
     {
          found = TRUE;
          act ( "$n looks warmer.", victim, NULL, NULL, TO_ROOM );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "curse" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect evil" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect good" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect hidden" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect invis" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect hidden" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "detect magic" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "faerie fire" ) ) )
     {
          act ( "$n's outline fades.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "fear" ) ) )
     {
          act ( "$n no longer looks so scared.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "fly" ) ) )
     {
          act ( "$n falls to the ground!", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "frenzy" ) ) )
     {
          act ( "$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM );;
          found = TRUE;
     }

     if ( check_dispel
          ( level, victim, skill_lookup ( "giant strength" ) ) )
     {
          act ( "$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "haste" ) ) )
     {
          act ( "$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "slow" ) ) )
     {
          act ( "$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "infravision" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "invis" ) ) )
     {
          act ( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel
          ( level, victim, skill_lookup ( "mask self" ) ) )
     {
          act ( "$n suddenly appears as $s mask vanishes.", victim, NULL, NULL, TO_ROOM );
          undo_mask ( victim );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "mass invis" ) ) )
     {
          act ( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "mind meld" ) ) )
     {
          act ( "$n has regained $s senses.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "mute" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "pass door" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "protection evil" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "protection good" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "sanctuary" ) ) )
     {
          act ( "The white aura around $n's body vanishes.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "ghost form" ) ) )
     {
          act ( "The ghostly white aura around $n's body fades.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "shield" ) ) )
     {
          act ( "The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "sleep" ) ) )
          found = TRUE;

     if ( check_dispel
          ( level, victim, skill_lookup ( "stone skin" ) ) )
     {
          act ( "$n's skin regains its normal texture.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( check_dispel ( level, victim, skill_lookup ( "vocalize" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "weaken" ) ) )
     {
          act ( "$n looks stronger.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }

     if ( found )
          send_to_char ( "Ok.\n\r", ch );
     else
          send_to_char ( "Spell failed.\n\r", ch );
}

void spell_cause_light ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     damage ( ch, ( CHAR_DATA * ) vo, spell_calc(ch, sn), sn, DAM_HARM, TRUE );
     return;
}

void spell_cause_critical ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     damage ( ch, ( CHAR_DATA * ) vo, spell_calc(ch, sn), sn, DAM_HARM, TRUE );
     return;
}

void spell_cause_serious ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     damage ( ch, ( CHAR_DATA * ) vo, spell_calc (ch, sn), sn, DAM_HARM, TRUE);
     return;
}

void spell_youth ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "This spell only affects players.", ch );
          return;
     }

     send_to_char ( "You feel a great weight lifted off your shoulders.\n\r", victim );
     act ( "$n's skin takes on a youthful glow.", victim, NULL, NULL, TO_ROOM );
     victim->pcdata->age_mod += number_range ( 1, 3 );
     return;
}

void spell_age ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( IS_NPC ( victim ) )
     {
          send_to_char ( "This spell only affects players.", ch );
          return;
     }

     sound ("DECAY.WAV", victim);
     sound ("DECAY.WAV", ch);
     send_to_char ( "Your skin crawls as your life is sucked away\n\r.", victim );
     act ( "$n appears to grow old before your eyes.", victim, NULL, NULL, TO_ROOM );
     victim->pcdata->age_mod -= number_range ( 1, 5 );
     return;
}

void spell_chain_lightning ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     CHAR_DATA          *tmp_vict, *last_vict, *next_vict;
     bool                found;
     int                 dam;

    /* first strike */

     act ( "A lightning bolt leaps from $n's hand and arcs to $N.", ch, NULL, victim, TO_ROOM );
     act ( "A lightning bolt leaps from your hand and arcs to $N.", ch, NULL, victim, TO_CHAR );
     act ( "A lightning bolt leaps from $n's hand and hits you!", ch, NULL, victim, TO_VICT );

     dam = dice ( level, 6 );
     if ( saves_spell ( level, victim ) )
          dam /= 3;
     check_damage_obj ( victim, NULL, 1, DAM_LIGHTNING );
     damage ( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
     last_vict = victim;
     level -= 4;			/* decrement damage */

    /* new targets */
     while ( level > 0 )
     {
          found = FALSE;
          for ( tmp_vict = ch->in_room->people; tmp_vict != NULL; tmp_vict = next_vict )
          {
               next_vict = tmp_vict->next_in_room;
               if ( !is_safe_spell ( ch, tmp_vict, TRUE ) && tmp_vict != last_vict )
               {
                    found = TRUE;
                    last_vict = tmp_vict;
                    act ( "The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM );
                    act ( "The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR );
                    dam = dice ( level, 6 );
                    if ( saves_spell ( level, tmp_vict ) )
                         dam /= 3;
                    check_damage_obj ( tmp_vict, NULL, 1, DAM_LIGHTNING );
                    damage ( ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE );
                    level -= 4;	/* decrement damage */
               }
          }
          /* end target searching loop */

          if ( !found )		/* no target found, hit the caster */
          {
               if ( ch == NULL )
                    return;

               if ( last_vict == ch )	/* no double hits */
               {
                    act ( "The bolt seems to have fizzled out.", ch, NULL, NULL, TO_ROOM );
                    act ( "The bolt grounds out through your body.", ch, NULL, NULL, TO_CHAR );
                    return;
               }

               last_vict = ch;
               act ( "The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM );
               send_to_char ( "You are struck by your own lightning!\n\r", ch );
               dam = dice ( level, 6 );
               if ( saves_spell ( level, ch ) )
                    dam /= 3;
               check_damage_obj ( ch, NULL, 1, DAM_LIGHTNING );
               damage ( ch, ch, dam, sn, DAM_LIGHTNING, TRUE );
               level -= 4;		/* decrement damage */
               if ( ch == NULL )
                    return;
          }
	/* now go back and find more targets */
     }
}

void spell_change_sex ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "You've already been changed.\n\r", ch );
          else
               act ( "$N has already had $s(?) sex changed.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if ( saves_spell ( level, victim ) )
          return;
     set_affect ( &af, sn, level, 2*level, APPLY_SEX, 0, TO_AFFECTS, 0, ch->name );
     // Masked By: 
     do
     {
          af.modifier = number_range ( 0, 2 ) - victim->sex;
     }
     while ( af.modifier == 0 );
     affect_to_char ( victim, &af );
     
     send_to_char ( "You feel different.\n\r", victim );
     act ( "$n doesn't look like $mself anymore...", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_charm_person ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( victim == ch )
     {
          send_to_char ( "You like yourself even better!\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( victim, AFF_CHARM )
          || IS_AFFECTED ( ch, AFF_CHARM )
          || level < victim->level
          || IS_SET ( victim->imm_flags, IMM_CHARM )
          || saves_spell ( level, victim ) )
          return;

     if ( IS_SET ( victim->in_room->room_flags, ROOM_LAW ) )
     {
          send_to_char ( "The law forbids charming here..\n\r", ch );
          return;
     }

     if ( victim->position == POS_SLEEPING )
     {
          send_to_char( "You can not get your victim's attention.\n\r", ch );
          send_to_char( "Your slumbers are briefly troubled.\n\r", victim );
          return;
     }

     if ( ( victim->master ) || ( victim->leader ) )
          stop_follower ( victim );
     add_follower ( victim, ch );
     victim->leader = ch;

     set_affect ( &af, sn, level, number_fuzzy (level/4), 0, 0, TO_AFFECTS, AFF_CHARM, ch->name );     
     affect_to_char ( victim, &af );
     act ( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
     if ( ch != victim )
          act ( "$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR );
     return;
}

void spell_chill_touch ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;
     int                 dam;

     dam = spell_calc(ch, sn);

     if ( !saves_spell ( level, victim ) )
     {
          act ( "$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM );
          
          set_affect ( &af, sn, level, 6, APPLY_STR, -1, TO_AFFECTS, 0, ch->name );          
          affect_join ( victim, &af );
     }
     else
     {
          dam /= 2;
     }

     damage ( ch, victim, dam, sn, DAM_COLD, TRUE );
     return;
}

void spell_colour_spray ( int sn, int level, CHAR_DATA * ch,
			  void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = spell_calc(ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     else
          spell_blindness ( skill_lookup ( "blindness" ), level / 2, ch, ( void * ) victim );

     check_damage_obj ( victim, NULL, 1, DAM_LIGHT );
     damage ( ch, victim, dam, sn, DAM_LIGHT, TRUE );
     return;
}

/*
 * Unfinished, experimental, if you attempt to use this as is good luck, cuz it
 * won't work right.
void spell_darkness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    int chance;

    chance = number_percent();
    if (chance >=5)
    {
	--ch->in_room->light;

    }
}
*/

void spell_continual_light ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *light;

     light = create_object ( get_obj_index ( OBJ_VNUM_LIGHT_BALL ), 0 );
     obj_to_room ( light, ch->in_room );
     act ( "$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM );
     act ( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
     return;
}

void spell_control_weather ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     if ( !str_cmp ( target_name, "better" ) )
          weather_info.change += dice ( level / 3, 4 );
     else if ( !str_cmp ( target_name, "worse" ) )
          weather_info.change -= dice ( level / 3, 4 );
     else
          send_to_char ( "Do you want it to get better or worse?\n\r", ch );

     send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_create_food ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *mushroom;

     mushroom = create_object ( get_obj_index ( OBJ_VNUM_MUSHROOM ), 0 );
     mushroom->value[0] = 5 + level;
     obj_to_room ( mushroom, ch->in_room );
     act ( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
     act ( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
     return;
}

void spell_create_buffet ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *mushroom;
     int                 counter = 0;

     /* Attempt to prevent mushroom-spamming */
     mushroom = get_obj_list ( ch, "mushroom", ch->in_room->contents );
     if ( mushroom != NULL )
     {
          send_to_char ( "There already seems to be some food here.\n\r", ch );
          return;
     }

     for ( counter = 0; counter < level / 5 + 1; counter++ )
     {
          mushroom = create_object ( get_obj_index ( OBJ_VNUM_MUSHROOM ), 0 );
          mushroom->value[0] = level / 2;
          mushroom->value[1] = level;
          obj_to_room ( mushroom, ch->in_room );
     }
     act ( "A buffet suddenly appears.", ch, NULL, NULL, TO_ROOM );
     act ( "A buffet suddenly appears.", ch, NULL, NULL, TO_CHAR );

     return;
}

void spell_create_spring ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *spring;

     /* Attempt to prevent mushroom-spamming */
     spring = get_obj_list ( ch, "spring", ch->in_room->contents );
     if ( spring != NULL )
     {
          send_to_char ( "It seems there might already be a spring here.", ch );
          return;
     }
     spring = create_object ( get_obj_index ( OBJ_VNUM_SPRING ), 0 );
     spring->timer = level;
     obj_to_room ( spring, ch->in_room );
     act ( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
     act ( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
     return;
}

void spell_create_water ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     int                 water;

     if ( obj->item_type != ITEM_DRINK_CON )
     {
          send_to_char ( "It is unable to hold water.\n\r", ch );
          return;
     }

     if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
     {
          send_to_char ( "It contains some other liquid.\n\r", ch );
          return;
     }

     water = UMIN ( level * ( weather_info.sky >= SKY_RAINING ? 4 : 2 ), obj->value[0] - obj->value[1] );

     if ( water > 0 )
     {
          obj->value[2] = LIQ_WATER;
          obj->value[1] += water;
          if ( !is_name ( "water", obj->name ) )
          {
               char                buf[MAX_STRING_LENGTH];
               SNP ( buf, "%s water", obj->name );
               free_string ( obj->name );
               obj->name = str_dup ( buf );
          }
          act ( "$p is filled.", ch, obj, NULL, TO_CHAR );
     }

     return;
}

void spell_cure_blindness ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( !is_affected ( victim, gsn_blindness ) )
     {
          if ( victim == ch )
               send_to_char ( "You aren't blind.\n\r", ch );
          else
               act ( "$N doesn't appear to be blinded.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( check_dispel ( level, victim, gsn_blindness ) )
     {
          send_to_char ( "Your vision returns!\n\r", victim );
          act ( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
     }
     else
          send_to_char ( "Spell failed.\n\r", ch );
}

void spell_cure_critical ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 heal;

     heal = dice ( 3, 8 ) + level - 6;
     victim->hit = UMIN ( victim->hit + heal, victim->max_hit );
     update_pos ( victim );
     send_to_char ( "You feel better!\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

/* RT added to cure plague */
void spell_cure_disease ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( !is_affected ( victim, gsn_plague ) )
     {
          if ( victim == ch )
               send_to_char ( "You aren't ill.\n\r", ch );
          else
               act ( "$N doesn't appear to be diseased.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if ( check_dispel ( level, victim, gsn_plague ) )
     {
          send_to_char ( "Your sores vanish.\n\r", victim );
          act ( "$n looks relieves as $s sores vanish.", victim, NULL, NULL, TO_ROOM );
     }
     else
          send_to_char ( "Spell failed.\n\r", ch );
}

void spell_cure_light ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 heal;

     heal = dice ( 1, 8 ) + level / 3;
     victim->hit = UMIN ( victim->hit + heal, victim->max_hit );
     update_pos ( victim );
     send_to_char ( "You feel better!\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_cure_poison ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( !is_affected ( victim, gsn_poison ) )
     {
          if ( victim == ch )
               send_to_char ( "You aren't poisoned.\n\r", ch );
          else
               act ( "$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( check_dispel ( level, victim, gsn_poison ) )
     {
          send_to_char ( "A warm feeling runs through your body.\n\r", victim );
          act ( "$n looks much better.", victim, NULL, NULL, TO_ROOM );
     }
     else
          send_to_char ( "Spell failed.\n\r", ch );
}

void spell_cure_serious ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 heal;

     heal = dice ( 2, 8 ) + level / 2;
     victim->hit = UMIN ( victim->hit + heal, victim->max_hit );
     update_pos ( victim );
     send_to_char ( "You feel better!\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_curse ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_CURSE ) ||
          saves_spell ( level, victim ) )
          return;
     
     af.type = sn;
     af.where = TO_AFFECTS;
     af.level = level;
     af.duration = 2 * level;
     af.location = APPLY_HITROLL;
     af.modifier = -1 * (( level / 8 )+1);
     af.where = TO_AFFECTS;
     af.bitvector = AFF_CURSE;
     
     set_affect ( &af, sn, level, 2*level, APPLY_HITROLL, (-1 * ((level/8)+1)), TO_AFFECTS, AFF_CURSE, ch->name );
     affect_to_char ( victim, &af );
     // Masked by:
     af.location = APPLY_SAVING_SPELL;
     af.modifier = (level / 8)+1;
     affect_to_char ( victim, &af );

     send_to_char ( "You feel unclean.\n\r", victim );
     if ( ch != victim )
          act ( "$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR );
     return;
}

/* Lotherius Exorcism, good version of Demonfire */
void spell_exorcism ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     if ( !IS_NPC ( ch ) && !IS_GOOD ( ch ) )
     {
          victim = ch;
          send_to_char ( "The Angels turn to attack you!\n\r", ch );
     }

     ch->alignment = UMAX ( 1000, ch->alignment + 50 );

     if ( victim != ch )
     {
          act ( "$n invokes the wrath of his God on $N!",  ch, NULL, victim, TO_NOTVICT );
          act ( "$n has called forth the Angels of heaven to exorcise you!", ch, NULL, victim, TO_VICT );
          send_to_char ( "You call forth Angels of God!\n\r", ch );
     }
     dam = spell_calc (ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 1, DAM_HOLY );
     damage ( ch, victim, dam, sn, DAM_HOLY, TRUE );
}

void spell_demonfire ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     if ( !IS_NPC ( ch ) && !IS_EVIL ( ch ) )
     {
          victim = ch;
          send_to_char ( "The demons turn upon you!\n\r", ch );
     }
     ch->alignment = UMAX ( -1000, ch->alignment - 50 );
     if ( victim != ch )
     {
          act ( "$n calls forth the demons of Hell upon $N!", ch, NULL, victim, TO_NOTVICT );
          act ( "$n has assailed you with the demons of Hell!", ch, NULL, victim, TO_VICT );
          send_to_char ( "You conjure forth the demons of hell!\n\r", ch );
     }
     check_damage_obj ( victim, NULL, 1, DAM_NEGATIVE );
     dam = spell_calc(ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_NEGATIVE, TRUE );
}

void spell_detect_evil ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, DET_EVIL ) )
     {
          if ( victim == ch )
               send_to_char ( "You can already sense evil.\n\r", ch );
          else
               act ( "$N can already detect evil.", ch, NULL, victim, TO_CHAR );
          return;
     }
     
     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_DETECTIONS, DET_EVIL, ch->name );
     affect_to_char ( victim, &af );
     send_to_char ( "Your eyes tingle.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_detect_good ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, skill_lookup ( "detect good" ) ) )
     {
          if ( victim == ch )
               send_to_char ( "You can already sense good.\n\r", ch );
          else
               act ( "$N can already detect good.", ch, NULL, victim, TO_CHAR );
          return;
     }

     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_DETECTIONS, 0, ch->name );
     affect_to_char ( victim, &af );
     
     send_to_char ( "Your eyes tingle.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_detect_hidden ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, DET_HIDDEN ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already as alert as you can be. \n\r", ch );
          else
               act ( "$N can already sense hidden lifeforms.", ch, NULL, victim, TO_CHAR );
          return;
     }
     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_DETECTIONS, DET_HIDDEN, ch->name );
     affect_to_char ( victim, &af );
     send_to_char ( "Your awareness improves.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_detect_invis ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, DET_INVIS ) )
     {
          if ( victim == ch )
               send_to_char ( "You can already see invisible.\n\r", ch );
          else
               act ( "$N can already see invisible things.", ch, NULL, victim, TO_CHAR );
          return;
     }
     
     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_DETECTIONS, DET_INVIS, ch->name );
     affect_to_char ( victim, &af );
     
     send_to_char ( "Your eyes tingle.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_detect_magic ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, DET_MAGIC ) )
     {
          if ( victim == ch )
               send_to_char
               ( "You can already sense magical auras.\n\r", ch );
          else
               act ( "$N can already detect magic.", ch, NULL, victim,
                     TO_CHAR );
          return;
     }

     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_DETECTIONS, DET_MAGIC, ch->name );
     affect_to_char ( victim, &af );
     send_to_char ( "Your eyes tingle.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_detect_poison ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;

     if ( obj->item_type == ITEM_DRINK_CON ||
          obj->item_type == ITEM_FOOD )
     {
          if ( obj->value[3] != 0 )
               send_to_char ( "You smell poisonous fumes.\n\r", ch );
          else
               send_to_char ( "It looks delicious.\n\r", ch );
     }
     else
     {
          send_to_char ( "It doesn't look poisoned.\n\r", ch );
     }

     return;
}

void spell_dispel_evil ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     if ( !IS_NPC ( ch ) && IS_EVIL ( ch ) )
          victim = ch;

     if ( IS_GOOD ( victim ) )
     {
          act ( "Dispel evil has no effect upon $N.", ch, NULL, victim, TO_ROOM );
          return;
     }

     if ( IS_NEUTRAL ( victim ) )
     {
          act ( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
          return;
     }

     dam = spell_calc(ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 100, DAM_HOLY );
     damage ( ch, victim, dam, sn, DAM_HOLY, TRUE );
     return;
}

void spell_dispel_good ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     if ( !IS_NPC ( ch ) && IS_GOOD ( ch ) )
          victim = ch;

     if ( IS_EVIL ( victim ) )
     {
          act ( "Dispel good has no effect upon $N.", ch, NULL, victim, TO_ROOM );
          return;
     }

     if ( IS_NEUTRAL ( victim ) )
     {
          act ( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
          return;
     }

     dam = spell_calc (ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;

     damage ( ch, victim, dam, sn, DAM_NEGATIVE, TRUE );

     return;
}

/* modified for enhanced use */

void spell_dispel_magic ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     bool                found = FALSE;

     if ( saves_spell ( level, victim ) )
     {
          send_to_char ( "You feel a brief tingling sensation.\n\r",
                         victim );
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     /* begin running through the spells */

     if ( check_dispel ( level, victim, skill_lookup ( "absorb magic" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "confusion" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "spirit armor" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "regeneration" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "armor" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "bless" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "blindness" ) ) )
     {
          found = TRUE;
          act ( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
     }
     if ( check_dispel ( level, victim, skill_lookup ( "calm" ) ) )
     {
          found = TRUE;
          act ( "$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM );
     }
     if ( check_dispel ( level, victim, skill_lookup ( "change sex" ) ) )
     {
          found = TRUE;
          act ( "$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM );
     }

     if ( check_dispel ( level, victim, skill_lookup ( "charm person" ) ) )
     {
          found = TRUE;
          act ( "$n regains $s free will.", victim, NULL, NULL, TO_ROOM );
     }
     if ( check_dispel ( level, victim, skill_lookup ( "chill touch" ) ) )
     {
          found = TRUE;
          act ( "$n looks warmer.", victim, NULL, NULL, TO_ROOM );
     }
     if ( check_dispel ( level, victim, skill_lookup ( "curse" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect evil" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect good" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect hidden" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect invis" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect hidden" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "detect magic" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "faerie fire" ) ) )
     {
          act ( "$n's outline fades.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "fear" ) ) )
     {
          act ( "$n no longer looks so scared.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "fly" ) ) )
     {
          act ( "$n falls to the ground!", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "frenzy" ) ) )
     {
          act ( "$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM );;
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "giant strength" ) ) )
     {
          act ( "$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "haste" ) ) )
     {
          act ( "$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "slow" ) ) )
     {
          act ( "$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "infravision" ) ) )
          found = TRUE;

     if ( check_dispel ( level, victim, skill_lookup ( "invis" ) ) )
     {
          act ( "$n fades into existence.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "mass invis" ) ) )
     {
          act ( "$n fades into existence.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "mask self" ) ) )
     {
          act ( "$n suddenly appears as $s mask vanishes.", victim, NULL, NULL, TO_ROOM );
          undo_mask ( victim );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "mind meld" ) ) )
     {
          act ( "$n has regained $s senses.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "mute" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "pass door" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "protection_good" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "protection_evil" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "sanctuary" ) ) )
     {
          act ( "The white aura around $n's body vanishes.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "ghost form" ) ) )
     {
          act ( "The ghostly white aura around $n's body fades.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( IS_PROTECTED ( victim, PROT_SANCTUARY ) // For those protected but not by the spell - mobs usually.
          && !saves_dispel ( level, victim->level, -1 )
          && !is_affected ( victim, skill_lookup ( "sanctuary" ) ) )
     {
          REMOVE_BIT( victim->protections, PROT_SANCTUARY );
          act ( "The white aura around $n's body vanishes.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "shield" ) ) )
     {
          act ( "The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "sleep" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "stone skin" ) ) )
     {
          act ( "$n's skin regains its normal texture.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( check_dispel ( level, victim, skill_lookup ( "vocalize" ) ) )
          found = TRUE;
     if ( check_dispel ( level, victim, skill_lookup ( "weaken" ) ) )
     {
          act ( "$n looks stronger.", victim, NULL, NULL, TO_ROOM );
          found = TRUE;
     }
     if ( found )
          send_to_char ( "Ok.\n\r", ch );
     else
          send_to_char ( "Spell failed.\n\r", ch );
}

void spell_earthquake ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;

     send_to_char ( "The earth trembles beneath your feet!\n\r", ch );
     act ( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

     for ( vch = char_list; vch != NULL; vch = vch_next )
     {
          vch_next = vch->next;
          if ( vch->in_room == NULL )
               continue;
          if ( vch->in_room == ch->in_room )
          {
               sound ("earthquake.wav", ch);
               if ( vch != ch && !is_safe_spell ( ch, vch, TRUE ) )
               {
                    if ( IS_AFFECTED ( vch, AFF_FLYING ) )
                         damage ( ch, vch, 0, sn, DAM_BASH, TRUE );
                    else
                    {
                         check_damage_obj ( vch, NULL, 10, DAM_BASH );
                         damage ( ch, vch, spell_calc(ch, sn), sn, DAM_BASH, TRUE );
                    }
               }
               continue;
          }

          if ( vch->in_room->area == ch->in_room->area )
               send_to_char ( "The earth trembles and shivers.\n\r", vch );
     }

     return;
}

void spell_enchant_armor ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     AFFECT_DATA        *paf;
     int                 result, fail;
     int                 ac_bonus, added;
     bool                ac_found = FALSE;

     if ( obj->item_type != ITEM_ARMOR )
     {
          send_to_char ( "That isn't armor.\n\r", ch );
          return;
     }

     if ( obj->wear_loc != -1 )
     {
          send_to_char ( "You must be carrying that which you wish to enchant.\n\r", ch );
          return;
     }

     /* this means they have no bonus */
     ac_bonus = 0;
     fail = 25;			/* base 25% chance of failure */

     /* find the bonii */

     if ( !obj->enchanted )
          for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->location == APPLY_AC )
               {
                    ac_bonus = paf->modifier;
                    ac_found = TRUE;
                    fail += 5 * ( ac_bonus * ac_bonus );
               }
               else		/* things get a little harder */
                    fail += 20;
          }

     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          if ( paf->location == APPLY_AC )
          {
               ac_bonus = paf->modifier;
               ac_found = TRUE;
               fail += 5 * ( ac_bonus * ac_bonus );
          }
          else			/* things get a little harder */
               fail += 20;
     }

     /* apply other modifiers */
     fail -= level;

     if ( IS_OBJ_STAT ( obj, ITEM_BLESS ) )
          fail -= 15;
     if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) )
          fail -= 5;

     fail = URANGE ( 5, fail, 95 );

     result = number_percent (  );

     /* the moment of truth */
     if ( result < ( fail / 5 ) )	/* item destroyed */
     {
          act ( "$p flares blindingly... and evaporates!", ch, obj, NULL, TO_CHAR );
          act ( "$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ROOM );
          extract_obj ( obj );
          return;
     }

     if ( result < ( fail / 2 ) )	/* item disenchanted */
     {
          AFFECT_DATA        *paf_next;

          act ( "$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR );
          act ( "$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM );
          obj->enchanted = TRUE;

          /* remove all affects */
          for ( paf = obj->affected; paf != NULL; paf = paf_next )
          {
               paf_next = paf->next;
               paf->next = affect_free;
               affect_free = paf;
          }
          obj->affected = NULL;

          /* clear all flags */
          obj->extra_flags = 0;
          return;
     }

     if ( result <= fail )	/* failed, no bad result */
     {
          send_to_char ( "Nothing seemed to happen.\n\r", ch );
          return;
     }

     /* okay, move all the old flags into new vectors if we have to */
     if ( !obj->enchanted )
     {
          AFFECT_DATA        *af_new;

          obj->enchanted = TRUE;

          for ( paf = obj->pIndexData->affected; paf != NULL;
                paf = paf->next )
          {
               if ( affect_free == NULL )
                    af_new = alloc_perm ( sizeof ( *af_new ), "enchant_armor" );
               else
               {
                    af_new = affect_free;
                    affect_free = affect_free->next;
               }

               af_new->next = obj->affected;
               obj->affected = af_new;

               af_new->type = UMAX ( 0, paf->type );
               af_new->level = paf->level;
               af_new->duration = paf->duration;
               af_new->location = paf->location;
               af_new->modifier = paf->modifier;
               af_new->bitvector = paf->bitvector;
          }
     }

     if ( result <= ( 100 - level / 5 ) )	/* success! */
     {
          act ( "$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR );
          act ( "$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM );
          SET_BIT ( obj->extra_flags, ITEM_MAGIC );
          added = 1;
     }

     else			/* exceptional enchant */
     {
          act ( "$p glows a brillant gold!", ch, obj, NULL, TO_CHAR );
          act ( "$p glows a brillant gold!", ch, obj, NULL, TO_ROOM );
          SET_BIT ( obj->extra_flags, ITEM_MAGIC );
          SET_BIT ( obj->extra_flags, ITEM_GLOW );
          added = 2;
     }

    /* now add the enchantments */

     if ( obj->level < LEVEL_HERO )
          obj->level = UMIN ( LEVEL_HERO - 1, obj->level + 1 );

     if ( ac_found )
     {
          for ( paf = obj->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->location == APPLY_AC )
               {
                    paf->type = sn;
                    paf->modifier += added;
                    paf->level = UMAX ( paf->level, level );
               }
          }
     }
     else			/* add a new affect */
     {
          if ( affect_free == NULL )
               paf = alloc_perm ( sizeof ( *paf ), "enchant_armor" );
          else
          {
               paf = affect_free;
               affect_free = affect_free->next;
          }

          paf->type = sn;
          paf->level = level;
          paf->duration = -1;
          paf->location = APPLY_AC;
          paf->modifier = added;
          paf->bitvector = 0;
          paf->next = obj->affected;
          obj->affected = paf;
     }

}

/* Zeran wrote "branding".... works oddly but it works - Lotherius */

void spell_brand ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     int                 roll;
     int                 all_brand = WEAPON_LIGHTNING + WEAPON_ACID + WEAPON_FLAMING + WEAPON_FROST;

     if ( obj->item_type != ITEM_WEAPON )
     {
          send_to_char ( "That isn't a weapon.\n\r", ch );
          return;
     }
     if ( obj->wear_loc != -1 )
     {
          send_to_char ( "You must be carrying that which you wish to enchant.\n\r",  ch );
          return;
     }
     if ( ( obj->value[4] & all_brand ) != 0 )	/*Already branded */
     {
          send_to_char ( "This weapon is already branded.\n\r", ch );
          return;
     }
     roll = number_percent (  );
     if ( roll <= 5 )		/*oops, destroy weapon */
     {
          extract_obj ( obj );
          send_to_char ( "{mKaboom!{x  The weapon explodes!  *sigh*\n\r",  ch );
          act ( "$p shivers violently and explodes!", ch, obj, NULL, TO_ROOM );
          return;
     }
     if ( roll > ( get_curr_stat ( ch, STAT_INT ) - 18 ) * 2 + 5 + ( ( int ) ( ch->level / 3 ) ) )	/*spell failed */
     {
          send_to_char ( "Spell failed.\n\r", ch );
          return;
     }
     else			/*spell worked! */
     {
          int                 which = number_percent (  );
          int                 brand = 0;
          char                buf[80];

          if ( which <= 25 )
          {
               brand = WEAPON_FLAMING;
               SLCPY ( buf, "You have created a {rFLAME{x brand!\n\r" );
          }
          else if ( which <= 50 )
          {
               brand = WEAPON_FROST;
               SLCPY ( buf, "You have created a {bFROST{x brand!\n\r" );
          }
          else if ( which <= 75 )
          {
               brand = WEAPON_ACID;
               SLCPY ( buf, "You have created an {gACID{x brand!\n\r" );
          }
          else
          {
               brand = WEAPON_LIGHTNING;
               SLCPY ( buf, "You have created a {yLIGHTNING{x brand!\n\r" );
          }

          /*set new brand on weapon */
          obj->value[4] += brand;
          obj->valueorig[4] += brand;
          send_to_char ( buf, ch );
     }
}

void spell_enchant_weapon ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     AFFECT_DATA        *paf;
     int                 result, fail;
     int                 hit_bonus, dam_bonus, added;
     bool                hit_found = FALSE, dam_found = FALSE;

     if ( obj->item_type != ITEM_WEAPON )
     {
          send_to_char ( "That isn't a weapon.\n\r", ch );
          return;
     }

     if ( obj->wear_loc != -1 )
     {
          send_to_char ( "You must be carrying that which you wish to enchant.\n\r", ch );
          return;
     }

     /* this means they have no bonus */
     hit_bonus = 0;
     dam_bonus = 0;
     fail = 25;			/* base 25% chance of failure */

     /* find the bonii */

     if ( !obj->enchanted )
          for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->location == APPLY_HITROLL )
               {
                    hit_bonus = paf->modifier;
                    hit_found = TRUE;
                    fail += 2 * ( hit_bonus * hit_bonus );
               }

               else if ( paf->location == APPLY_DAMROLL )
               {
                    dam_bonus = paf->modifier;
                    dam_found = TRUE;
                    fail += 2 * ( dam_bonus * dam_bonus );
               }

               else		/* things get a little harder */
                    fail += 25;
          }

     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          if ( paf->location == APPLY_HITROLL )
          {
               hit_bonus = paf->modifier;
               hit_found = TRUE;
               fail += 2 * ( hit_bonus * hit_bonus );
          }

          else if ( paf->location == APPLY_DAMROLL )
          {
               dam_bonus = paf->modifier;
               dam_found = TRUE;
               fail += 2 * ( dam_bonus * dam_bonus );
          }

          else			/* things get a little harder */
               fail += 25;
     }

     /* apply other modifiers */
     fail -= 3 * level / 2;

     if ( IS_OBJ_STAT ( obj, ITEM_BLESS ) )
          fail -= 15;
     if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) )
          fail -= 5;
     fail = URANGE ( 5, fail, 95 );
     result = number_percent (  );

     /* the moment of truth */
     if ( result < ( fail / 5 ) )	/* item destroyed */
     {
          act ( "$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR );
          act ( "$p shivers violently and explodes!", ch, obj, NULL, TO_ROOM );
          extract_obj ( obj );
          return;
     }

     if ( result < ( fail / 2 ) )	/* item disenchanted */
     {
          AFFECT_DATA        *paf_next;

          act ( "$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR );
          act ( "$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM );
          obj->enchanted = TRUE;

          /* remove all affects */
          for ( paf = obj->affected; paf != NULL; paf = paf_next )
          {
               paf_next = paf->next;
               paf->next = affect_free;
               affect_free = paf;
          }
          obj->affected = NULL;

          /* clear all flags */
          obj->extra_flags = 0;
          return;
     }

     if ( result <= fail )	/* failed, no bad result */
     {
          send_to_char ( "Nothing seemed to happen.\n\r", ch );
          return;
     }

     /* okay, move all the old flags into new vectors if we have to */
     if ( !obj->enchanted )
     {
          AFFECT_DATA        *af_new;

          obj->enchanted = TRUE;

          for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
          {
               if ( affect_free == NULL )
                    af_new = alloc_perm ( sizeof ( *af_new ), "enchant_weapon" );
               else
               {
                    af_new = affect_free;
                    affect_free = affect_free->next;
               }

               af_new->next = obj->affected;
               obj->affected = af_new;

               af_new->type = UMAX ( 0, paf->type );
               af_new->level = paf->level;
               af_new->duration = paf->duration;
               af_new->location = paf->location;
               af_new->modifier = paf->modifier;
               af_new->bitvector = paf->bitvector;
          }
     }

     if ( result <= ( 100 - level / 5 ) )	/* success! */
     {
          act ( "$p glows blue.", ch, obj, NULL, TO_CHAR );
          act ( "$p glows blue.", ch, obj, NULL, TO_ROOM );
          SET_BIT ( obj->extra_flags, ITEM_MAGIC );
          added = 1;
     }

     else			/* exceptional enchant */
     {
          act ( "$p glows a brillant blue!", ch, obj, NULL, TO_CHAR );
          act ( "$p glows a brillant blue!", ch, obj, NULL, TO_ROOM );
          SET_BIT ( obj->extra_flags, ITEM_MAGIC );
          SET_BIT ( obj->extra_flags, ITEM_GLOW );
          added = 2;
     }

    /* now add the enchantments */

     if ( obj->level < LEVEL_HERO - 1 )
          obj->level = UMIN ( LEVEL_HERO - 1, obj->level + 1 );

     if ( dam_found )
     {
          for ( paf = obj->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->location == APPLY_DAMROLL )
               {
                    paf->type = sn;
                    paf->modifier += added;
                    paf->level = UMAX ( paf->level, level );
                    if ( paf->modifier > 4 )
                         SET_BIT ( obj->extra_flags, ITEM_HUM );
               }
          }
     }
     else			/* add a new affect */
     {
          if ( affect_free == NULL )
               paf = alloc_perm ( sizeof ( *paf ), "enchant_weapon" );
          else
          {
               paf = affect_free;
               affect_free = affect_free->next;
          }

          paf->type = sn;
          paf->level = level;
          paf->duration = -1;
          paf->location = APPLY_DAMROLL;
          paf->modifier = added;
          paf->bitvector = 0;
          paf->next = obj->affected;
          obj->affected = paf;
     }

     if ( hit_found )
     {
          for ( paf = obj->affected; paf != NULL; paf = paf->next )
          {
               if ( paf->location == APPLY_HITROLL )
               {
                    paf->type = sn;
                    paf->modifier += added;
                    paf->level = UMAX ( paf->level, level );
                    if ( paf->modifier > 4 )
                         SET_BIT ( obj->extra_flags, ITEM_HUM );
               }
          }
     }
     else			/* add a new affect */
     {
          if ( affect_free == NULL )
               paf = alloc_perm ( sizeof ( *paf ), "enchant_weapon" );
          else
          {
               paf = affect_free;
               affect_free = affect_free->next;
          }

          paf->type = sn;
          paf->level = level;
          paf->duration = -1;
          paf->location = APPLY_HITROLL;
          paf->modifier = added;
          paf->bitvector = 0;
          paf->next = obj->affected;
          obj->affected = paf;
     }
     return;
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 * For now, this uses old damage method.
 */
void spell_energy_drain ( int sn, int level, CHAR_DATA * ch,
			  void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     if ( saves_spell ( level, victim ) )
     {
          send_to_char ( "You feel a momentary chill.\n\r", victim );
          return;
     }

     ch->alignment = UMAX ( -1000, ch->alignment - 50 );
     if ( victim->level <= 2 )
     {
          dam = ch->hit + 1;
     }
     else
     {
          gain_exp ( victim,
                     0 - 5 * number_range ( level / 2,
                                            3 * level / 2 ) );
          victim->mana /= 2;
          victim->move /= 2;
          dam = dice ( 1, level );
          ch->hit = UMIN ( ch->max_hit, ( ch->hit + dam ) );

     }

     send_to_char ( "You feel your life slipping away!\n\r", victim );
     send_to_char ( "Wow....what a rush!\n\r", ch );
     damage ( ch, victim, dam, sn, DAM_NEGATIVE, TRUE );

     return;
}

void spell_fireball ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA         *victim = ( CHAR_DATA * ) vo;
     int                dam;
     int		balls = 1;
     int		mult = 1;
     int		count;
     bool               bigdam = FALSE;
     bool		oops = FALSE;

     if (number_fuzzy(ch->level) >= 35)
     {
          balls++;
          if (number_fuzzy(ch->level) >=50 )
          {
               balls++;
               if (number_fuzzy(ch->level) >=80 )
               {
                    balls++;
                    if (number_fuzzy(ch->level) >=95 )
                    {
                         balls++;
                    }
               }
          }
     }

    /* Some fun for chaosmages only! */

     if (!IS_NPC(ch) && ch->pcdata->pclass == CLASS_CHAOSMAGE )
     {
          if (chance(1))
          {
               balls *= 2;
          }
          if (chance(1))
          {
               balls *=2;
          }
          if (chance(1))
          {
               if (chance(1))
               {
                    mult = 2;
                    if (chance(1))
                    {
                         mult = number_fuzzy(10);
                         send_to_char("{R{&You have finally done it! Yes!!{x\n\r", ch);
                         act ( "$n suddenly is at one with magical energy.", ch, NULL, NULL, TO_ROOM );
                         act ( "$s enemies stand no chance against $s onslaught!", ch, NULL, NULL, TO_ROOM );
                         bigdam = TRUE;
                    }
               }
          }
          if (chance(2))
          {
               oops = TRUE; /* don't want this trust me */
               send_to_char("{RYou have finally.... OOPS!\n\r", ch);
               act ("$n doesn't manage to avoid $s own fireball.", ch, NULL, NULL, TO_ROOM);
          }
     }

     for (count = 0 ; count < balls ; count++)
     {
          dam = spell_calc (ch, sn) * mult;

          if ( saves_spell ( level, victim ) )
               dam /= 2;
          if ( bigdam )
               check_damage_obj ( victim, NULL, 100, DAM_FIRE );
          else
               check_damage_obj ( victim, NULL, 50, DAM_FIRE );
          damage ( ch, victim, dam, sn, DAM_FIRE, TRUE );
          if (oops)
          {
               dam = spell_calc (ch, sn) * mult;
               if ( saves_spell (level, ch) )
                    dam /= 2;
               check_damage_obj ( ch, NULL, 25, DAM_FIRE );
               damage (ch, ch, dam, sn, DAM_FIRE, TRUE );
          }
     }

     return;
}

void spell_fire_shield ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) )
     {
          send_to_char ( "That target already has a fire shield.\n\r", ch );
          return;
     }

     set_affect ( &af, sn, level, level, APPLY_NONE, 0, TO_AFFECTS, 0, ch->name );
     affect_to_char ( victim, &af );

     sound ("flameshld.wav", victim);
     if ( victim != ch)
          sound ("flameshld.wav", ch);

     send_to_char ( "A small shield of living flame pops up before you.\n\r", victim );
     act ( "$n is suddenly defended by a small shield of living flame.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_flamestrike ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = (spell_calc(ch, sn) + spell_calc(ch, sn) ) /2;

     sound ("flamstrk.wav", victim);
     if ( victim != ch)
          sound ("flamstrk.wav", ch);

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 25, DAM_FIRE );
     damage ( ch, victim, dam, sn, DAM_FIRE, TRUE );
     return;
}

void spell_faerie_fire ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_FAERIE_FIRE ) )
          return;
     
     set_affect ( &af, sn, level, level, APPLY_AC, -(2*level), TO_AFFECTS, AFF_FAERIE_FIRE, ch->name );
     affect_to_char ( victim, &af );
     send_to_char ( "You are surrounded by a pink outline.\n\r", victim );
     act ( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_faerie_fog ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *ich;

     act ( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
     send_to_char ( "You conjure a cloud of purple smoke.\n\r", ch );

     for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
     {
          if ( !IS_NPC ( ich ) && IS_SET ( ich->act, PLR_WIZINVIS ) )
               continue;
          if ( ich == ch || saves_spell ( level, ich ) )
               continue;
          affect_strip ( ich, gsn_invis );
          affect_strip ( ich, gsn_mass_invis );
          affect_strip ( ich, gsn_sneak );
          REMOVE_BIT ( ich->affected_by, AFF_HIDE );
          REMOVE_BIT ( ich->affected_by, AFF_INVISIBLE );
          REMOVE_BIT ( ich->affected_by, AFF_SNEAK );
          act ( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
          send_to_char ( "You are revealed!\n\r", ich );
     }
     return;
}

void spell_fear ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( victim->fighting == NULL )
     {
          send_to_char ( "Fear can only be cast upon those who are fighting.\n\r",  ch );
          return;
     }
     if ( IS_AFFECTED ( victim, AFF_FEAR ) )
     {
          act ( "$n has already been scared sh*tless...", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( !saves_spell ( level, victim ) )
     {
          if ( !IS_NPC ( victim ) )
               send_to_char ( "You feel very vulnerable and scared!\n\r", victim );
          do_say ( victim, "Please, don't hurt me!" );
          
          set_affect ( &af, sn, level, level/6, 0, 0, TO_AFFECTS, AFF_FEAR, ch->name );
          affect_to_char ( victim, &af );
          do_flee ( victim, "" );
          stop_fighting ( victim, TRUE );
          return;
     }
     else
     {
          if ( !IS_NPC ( victim ) )
               send_to_char ( "For just a moment, an intense feeling of fear washes over you.\n\r", victim );
          send_to_char ( "Spell failed.\n\r", ch );
          return;
     }
}

void spell_fly ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_FLYING ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already airborne.\n\r", ch );
          else
               act ( "$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR );
          return;
     }
     set_affect ( &af, sn, level, level+3, 0, 0, TO_AFFECTS, AFF_FLYING, ch->name );
     affect_to_char ( victim, &af );
     send_to_char ( "Your feet rise off the ground.\n\r", victim );
     act ( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
     return;
}

/* RT clerical berserking spell */
void spell_frenzy ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) ||
          IS_AFFECTED ( victim, AFF_BERSERK ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already in a frenzy.\n\r", ch );
          else
               act ( "$N is already in a frenzy.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if ( is_affected ( victim, skill_lookup ( "calm" ) ) )
     {
          if ( victim == ch )
               send_to_char ( "Why don't you just relax for a while?\n\r", ch );
          else
               act ( "$N doesn't look like $e wants to fight anymore.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if ( ( IS_GOOD ( ch ) && !IS_GOOD ( victim ) ) ||
          ( IS_NEUTRAL ( ch ) && !IS_NEUTRAL ( victim ) ) ||
          ( IS_EVIL ( ch ) && !IS_EVIL ( victim ) ) )
     {
          act ( "Your deity doesn't seem to like $N", ch, NULL, victim, TO_CHAR );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = level / 3;
     af.modifier = level / 6;
     af.bitvector = 0;
     af.location = APPLY_HITROLL;
     
     set_affect ( &af, sn, level, level/3, APPLY_HITROLL, level/6, TO_AFFECTS, 0, ch->name );
     affect_to_char ( victim, &af );
     // Masked by:
     af.location = APPLY_DAMROLL;
     affect_to_char ( victim, &af );
     // Masked by:
     af.modifier = -(10 * ( level / 6 ) );
     af.location = APPLY_AC;
     affect_to_char ( victim, &af );

     send_to_char ( "You are filled with holy wrath!\n\r", victim );
     act ( "$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM );
}

/* RT ROM-style gate */
/* Lotherius - No Error gate */
void spell_gate_without_error ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim;
     bool                gate_pet;

     if ( ( victim = get_char_world( ch, target_name ) ) == NULL
          ||   victim == ch
          ||   victim->in_room == NULL
          ||   !can_see_room(ch,victim->in_room)
          ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
          ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
          ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
          ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
          ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
          ||   victim->level >= level + 3
          ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
          ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
          ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
          ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )

     {
          send_to_char( "You failed.\n\r", ch );
          return;
     }

     if ( ch->pet != NULL && ch->in_room == ch->pet->in_room )
          gate_pet = TRUE;
     else
          gate_pet = FALSE;

     act ( "$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM );
     send_to_char ( "You step through a gate and vanish.\n\r", ch );
     char_from_room ( ch );
     char_to_room ( ch, victim->in_room );

     act ( "$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM );
     do_look ( ch, "auto" );

     if ( gate_pet )
     {
          act ( "$n steps through a gate and vanishes.", ch->pet,  NULL, NULL, TO_ROOM );
          send_to_char ( "You step through a gate and vanish.\n\r", ch->pet );
          char_from_room ( ch->pet );
          char_to_room ( ch->pet, victim->in_room );
          act ( "$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM );
          do_look ( ch->pet, "auto" );
     }
}

/* Gate with error - Lotherius */
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
     CHAR_DATA *victim;
     ROOM_INDEX_DATA *pRoomIndex;
     bool fail = FALSE;

     if ( ( victim = get_char_world( ch, target_name ) ) == NULL
          ||   victim == ch
          ||   victim->in_room == NULL
          ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
          ||   victim->level >= level + 3
          ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
          ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
          ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
          ||   (IS_NPC(victim) && saves_spell( level, victim ) ) )

     {
          send_to_char( "You failed to create a gate.\n\r", ch );
          return;
     }

     pRoomIndex = get_room_index( number_fuzzy(victim->in_room->vnum) );

     if (!pRoomIndex)
          fail = TRUE;
     else
     {
          if ( IS_SET(pRoomIndex->room_flags, ROOM_SAFE)
               || !can_see_room(ch, pRoomIndex)
               || IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
               || IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
               || IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) )
               fail = TRUE;
          /* And.... a chance to fail just because. */
          if ( chance(10) )
               fail = TRUE;
     }

     act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);

     if (fail)
     {
          if (chance(10) )
          {
               send_to_char("You step through the gate....\n\rINTO SOLID STONE!\n\r", ch);
               raw_kill(ch);
               act("The gate shivers violently and ejects $n's dead body.", ch, NULL, NULL, TO_ROOM);
               return;
          }
          else
          {
               send_to_char("You step through the gate....\n\rINTO SOLID STONE!\n\r", ch);
               send_to_char("Thankfully, you narrowly escape your death.", ch);
               act("The gate shivers violently then fades normally.", ch, NULL, NULL, TO_ROOM);
               pRoomIndex = get_room_index(ROOM_VNUM_LIMBO);
               char_from_room(ch);
               char_to_room(ch, pRoomIndex);
          }
     }
     else
     {
          send_to_char("You step through a gate and vanish.\n\r",ch);
          char_from_room(ch);
          char_to_room(ch, pRoomIndex);
     }
     act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
     do_look(ch,"auto");

}

void spell_giant_strength ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;
     int		 mod;

     if ( is_affected ( victim, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already as strong as you can get!\n\r", ch );
          else
               act ( "$N can't get any stronger.", ch, NULL, victim, TO_CHAR );
          return;
     }
     
     mod = 1 + ( level >= 18 ) + ( level >= 25 ) + ( level >= 32 );     
     set_affect ( &af, sn, level, level, APPLY_STR, mod, TO_AFFECTS, 0, ch->name );
     affect_to_char ( victim, &af );
     
     send_to_char ( "Your muscles surge with heightened power!\n\r", victim );
     act ( "$n's muscles surge with heightened power.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_harm ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = spell_calc(ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_HARM, TRUE );
     return;
}

/* RT haste spell */

void spell_haste ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) ||
          IS_AFFECTED ( victim, AFF_HASTE ) ||
          IS_SET ( victim->off_flags, OFF_FAST ) )
     {
          if ( victim == ch )
               send_to_char ( "You can't move any faster!\n\r", ch );
          else
               act ( "$N is already moving as fast as $e can.", ch, NULL, victim, TO_CHAR );
          return;
     }

    /* If affected by slow, spell cannot work, too many coding
     * headaches trying to deal with permanent haste, slow, etc. */
     if ( CAN_DETECT ( victim, AFF_SLOW ) )
     {
          if ( victim == ch )
               send_to_char ( "You're already slowed, and cannot be hasted.", ch );
          else
               act ( "$N seems to be slowed, so the haste has no effect.", ch, NULL, victim, TO_CHAR );
          return;
     }

     af.type = sn;
     af.level = level;
     if ( victim == ch )
          af.duration = level / 2;
     else
          af.duration = level / 4;
     af.location = APPLY_DEX;
     af.modifier =
          1 + ( level >= 18 ) + ( level >= 25 ) + ( level >= 32 );
     af.where = TO_AFFECTS;
     af.bitvector = AFF_HASTE;
     affect_to_char ( victim, &af );
     af.location = APPLY_ENCUMBRANCE;
     af.modifier = 0 - ( 1 + level / 33 );
     affect_to_char ( victim, &af );

     send_to_char ( "You feel yourself moving more quickly.\n\r", victim );
     act ( "$n is moving more quickly.", victim, NULL, NULL, TO_ROOM );
     sound ("haste.wav", victim);

     if ( ch != victim )
     {
          sound ("haste.wav", ch);
          send_to_char ( "Ok.\n\r", ch );
     }
     return;
}

void spell_confusion ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA	*victim = (CHAR_DATA * ) vo;
     AFFECT_DATA	af;

     if ( is_affected ( victim, sn ) || CAN_DETECT (victim, AFF_CONFUSION ) )
     {
          if (victim == ch )
               send_to_char ( "You're already confused. Obviously.\n\r", ch);
          else
               act ("$N is already quite confused.", ch, NULL, victim, TO_CHAR );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = 2 + level;
     af.location = APPLY_INT;
     af.modifier = -4;
     af.where = TO_DETECTIONS;
     af.bitvector = AFF_CONFUSION;
     affect_to_char ( victim, &af );

     act ( "$N experiences profound confusion.", ch, NULL, victim, TO_CHAR );
     send_to_char ( "Argle?!\n\r", victim );
     act ( "$N seems quite confused.!", ch, NULL, victim, TO_NOTVICT );
     return;
}

void spell_slow ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

    /* Check if already affected by slow */
     if ( is_affected ( victim, sn ) ||
          CAN_DETECT ( victim, AFF_SLOW ) )
     {
          if ( victim == ch )
               send_to_char ( "You can't move any slower!\n\r", ch );
          else
               act ( "$N is already moving as slowly as possible.", ch, NULL, victim, TO_CHAR );
          return;
     }

    /* If affected by haste, spell cannot work, too many coding
     * headaches trying to deal with permanent haste, slow, etc. */

     if ( IS_AFFECTED ( victim, AFF_HASTE ) )
     {
          if ( victim == ch )
               send_to_char ( "Since you are hasted, the attempt to slow you has failed.", ch );
          else
               act ( "$N can't seem to be slowed since $e is hasted already.", ch, NULL, victim, TO_CHAR );
          return;
     }

     af.type = sn;
     af.level = level;
     if ( victim == ch )
          af.duration = level / 2;
     else
          af.duration = level / 4;
     af.location = APPLY_DEX;
     af.modifier = 1 + ( level >= 18 ) + ( level >= 25 ) + ( level >= 32 );
     af.modifier = 0 - af.modifier;
     af.where = TO_DETECTIONS;
     af.bitvector = AFF_SLOW;
     affect_to_char ( victim, &af );
     af.location = APPLY_ENCUMBRANCE;
     af.modifier = ( 1 + level / 33 );
     affect_to_char ( victim, &af );

     send_to_char ( "You feel yourself moving more slowly.\n\r", victim );
     act ( "$n is moving very slowly.", victim, NULL, NULL, TO_ROOM );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_heal ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     victim->hit = UMIN ( victim->hit + 100, victim->max_hit );
     update_pos ( victim );
     send_to_char ( "A warm feeling fills your body.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

/* RT really nasty high-level attack spell */
void spell_holy_word ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;
     int                 dam;
     int                 bless_num, curse_num, frenzy_num;

     bless_num = skill_lookup ( "bless" );
     curse_num = skill_lookup ( "curse" );
     frenzy_num = skill_lookup ( "frenzy" );

     act ( "$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM );
     send_to_char ( "You utter a word of divine power.\n\r", ch );

     for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
     {
          vch_next = vch->next_in_room;

          if ( ( IS_GOOD ( ch ) && IS_GOOD ( vch ) ) ||
               ( IS_EVIL ( ch ) && IS_EVIL ( vch ) ) ||
               ( IS_NEUTRAL ( ch ) && IS_NEUTRAL ( vch ) ) )
          {
               send_to_char ( "You feel full more powerful.\n\r", vch );
               spell_frenzy ( frenzy_num, level, ch, ( void * ) vch );
               spell_bless ( bless_num, level, ch, ( void * ) vch );
          }

          else if ( ( IS_GOOD ( ch ) && IS_EVIL ( vch ) ) ||
                    ( IS_EVIL ( ch ) && IS_GOOD ( vch ) ) )
          {
               if ( !is_safe_spell ( ch, vch, TRUE ) )
               {
                    spell_curse ( curse_num, level, ch, ( void * ) vch );
                    send_to_char ( "You are struck down!\n\r", vch );
                    dam = spell_calc (ch, sn);
                    if ( IS_GOOD ( ch ) )
                    {
                         act ( "You are struck down by $N's Holy Power.", ch, NULL, vch, TO_VICT );
                         check_damage_obj ( vch, NULL, 35, DAM_HOLY );
                         damage ( ch, vch, dam, sn, DAM_HOLY, TRUE );
                    }
                    else
                    {
                         act ( "You are struck down by $N's Un-Holy Power.", ch, NULL, vch, TO_VICT );
                         damage ( ch, vch, dam, sn, DAM_NEGATIVE, TRUE );
                    }
               }
          }

          else if ( IS_NEUTRAL ( ch ) )
          {
               if ( !is_safe_spell ( ch, vch, TRUE ) )
               {
                    spell_curse ( curse_num, level / 2, ch, ( void * ) vch );
                    send_to_char ( "You are struck down!\n\r", vch );
                    dam = spell_calc (ch, sn)/2;
                    damage ( ch, vch, dam, sn, DAM_ENERGY, TRUE);
               }
          }
     }

     send_to_char ( "You feel drained.\n\r", ch );
     gain_exp ( ch, -1 * number_range ( 1, 10 ) * 10 );
     ch->move = 0;
     ch->hit /= 1.5;
}

/*
 * New identify by Lotherius
 */

void spell_identify ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     char                buf2[MIL];
     bool                tmpflag = FALSE;
     AFFECT_DATA        *paf;

     /* Give name and keywords */
     form_to_char ( ch, "{wName: {C[{w%s{C]{x Keywords: {C[{w%s{C]{x\n\r", obj->short_descr, obj->name );

     /* Show level and value */
     form_to_char ( ch, "It is level %d, and has an estimated value of %d in perfect condition.\n\r",
                    obj->level, obj->cost );
     if ( obj->condition < 100 )
          form_to_char ( ch, "However, this one is not perfect.\n\r" );

     /* Identify the material, if possible, and give weight */
     SNP ( buf2, "%s", material_name ( obj->material ) );
     if ( !str_cmp ( buf2, "unique" ) || !str_cmp ( buf2, "unknown" ) )
          SNP ( buf2, "a unique material that cannot be identified" );
     form_to_char ( ch, "The %s is made out of %s, and weighs %d lbs.\n\r", obj->short_descr, buf2, obj->weight );

     /* Show item-specific values */
     switch ( obj->item_type )
     {
     case ITEM_SCROLL:
          send_to_char ( "It appears to be a scroll, that has ", ch );
          tmpflag = TRUE;
          /* FALLTHROUGH */
     case ITEM_POTION:
          send_to_char ( "It appears to be a potion, that has ", ch );
          tmpflag = TRUE;
          /* FALLTHROUGH */
     case ITEM_PILL:
          if (!tmpflag)
               send_to_char ( "It appears to be a pill, that has ", ch );
          tmpflag = FALSE;
          form_to_char ( ch, "level %d spells of:", obj->value[0] );
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
          send_to_char ( "It appears to be a wand, that ", ch );
          tmpflag = TRUE;
          /* FALLTHROUGH */
     case ITEM_STAFF:
          if ( !tmpflag )
               send_to_char ( "It appears to be a staff, that ", ch );
          form_to_char ( ch, "has %d(%d) charges of level %d",
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
          send_to_char ( "It is ", ch );
          switch ( obj->value[0] )
          {
          case ( WEAPON_EXOTIC ):
               send_to_char ( "a strange weapon", ch );
               break;
          case ( WEAPON_SWORD ):
               send_to_char ( "a sword", ch );
               break;
          case ( WEAPON_DAGGER ):
               send_to_char ( "a dagger", ch );
               break;
          case ( WEAPON_SPEAR ):
               send_to_char ( "a spear or staff", ch );
               break;
          case ( WEAPON_MACE ):
               send_to_char ( "a mace or club", ch );
               break;
          case ( WEAPON_AXE ):
               send_to_char ( "an axe", ch );
               break;
          case ( WEAPON_FLAIL ):
               send_to_char ( "a flail", ch );
               break;
          case ( WEAPON_WHIP ):
               send_to_char ( "a whip", ch );
               break;
          case ( WEAPON_POLEARM ):
               send_to_char ( "a polearm", ch );
               break;
          default:
               send_to_char ( "an unknown type of weapon", ch );
               break;
          }
          if ( obj->pIndexData->new_format )
               form_to_char ( ch, " that has an average damage of %d.\n\r",
                              ( 1 + obj->value[2] ) * obj->value[1] / 2 );
          else
               form_to_char ( ch, " that has an average damage of %d.\n\r",
                              ( obj->value[1] + obj->value[2] ) / 2 );
          if ( obj->value[4] > 0 ) /* We're going to hope this is a valid value */
               form_to_char ( ch, "The weapon has qualities of: %s.\n\r", weapon_bit_name ( obj->value[4] ) );
          break;
     case ITEM_ARMOR:
          form_to_char ( ch, "It is a type of armor.\n\rArmor class is %d vs. pierce, %d vs. bash, "
                         "%d vs. slash, and %d vs. magic.\n\r",
                         c_base_ac ( obj, 0 ), c_base_ac ( obj, 1 ),
                         c_base_ac ( obj, 2 ), c_base_ac ( obj, 3 ) );
          break;
     case ITEM_LIGHT:
          send_to_char ( "It is a light ", ch );
          if ( obj->value[2] == -1 || obj->value[2] >= 999 )
               send_to_char ( "that will burn forever.\n\r", ch );
          else if ( obj->value[2] == 0 )
               send_to_char ( "that has burned out.\n\r", ch );
          else
               form_to_char ( ch, "that will burn for %d more hours.\n\r", obj->value[2] );
          break;
     case ITEM_CLOTHING:
          send_to_char ( "It is a simple item of clothing.\n\r", ch );
          break;
     case ITEM_FURNITURE:
          if ( obj->value[2] > 0 )
          {
               char buf[MSL];
               SNP ( buf, "It is a piece of furniture and can be:" );
               if ( IS_SET ( obj->value[2], SIT_ON ) )
                    SLCAT ( buf, " sat on" );
               if ( IS_SET ( obj->value[2], SIT_IN ) )
                    SLCAT ( buf, " sat in" );
               if ( IS_SET ( obj->value[2], SIT_AT ) )
                    SLCAT ( buf, " sat at" );
               if ( IS_SET ( obj->value[2], REST_ON ) )
                    SLCAT ( buf, " rested on" );
               if ( IS_SET ( obj->value[2], REST_IN ) )
                    SLCAT ( buf, " rested in" );
               if ( IS_SET ( obj->value[2], REST_AT ) )
                    SLCAT ( buf, " rested at" );
               if ( IS_SET ( obj->value[2], SLEEP_ON ) )
                    SLCAT ( buf, " slept on" );
               if ( IS_SET ( obj->value[2], SLEEP_IN ) )
                    SLCAT ( buf, " slept in" );
               if ( IS_SET ( obj->value[2], SLEEP_AT ) )
                    SLCAT ( buf, " slept at" );
               SLCAT ( buf, ".\n\r" );
               send_to_char ( buf, ch );
          }
          else
               send_to_char ( "It is a useless piece of furniture.\n\r", ch );
          form_to_char ( ch, "It has room for %d persons.\n\r", obj->value[0] );
          if ( obj->value[3] > 0 )          
               form_to_char ( ch, "Healing on %s is increased by %d percent.\n\r",
                              obj->short_descr, obj->value[3] );          
          break;
     case ITEM_CONTAINER:
          form_to_char ( ch, "It appears to be a container, that can hold up to %d lbs.\n\r",
                         obj->value[0] );
          break;
     case ITEM_DRINK_CON:
          form_to_char ( ch, "It is a drink container, holding %d of a possible %d pints of %s.\n\r",
                         obj->value[1], obj->value[0], flag_string ( liquid_flags, obj->value[2] ) );
          if ( obj->value[3] != 0 )
               send_to_char ( "It appears to be {gpoisoned{x!\n\r", ch );
          break;
     case ITEM_FOUNTAIN:
          send_to_char ( "It appears to be a fountain supplying water.\n\r", ch );
          break;
     case ITEM_KEY:
          send_to_char ( "It appears to be a key.\n\r", ch );
          break;
     case ITEM_FOOD:
          form_to_char ( ch, "It is food, enough to sate hunger for %d hours.\n\r", obj->value[0] );
          if ( obj->value[3] != 0 )
               send_to_char ( "It appears to be {gpoisoned{x!\n\r", ch );
          break;
     case ITEM_BOAT:
          send_to_char ( "Looks like it is a boat.\n\r", ch );
          break;
     case ITEM_CORPSE_NPC:
          /* FALLTHROUGH */
     case ITEM_CORPSE_PC:
          send_to_char ( "It is a dead body!\n\r", ch );
          break;
     case ITEM_PROTECT:
          send_to_char ( "It appears to be a charm or talisman of some sort.\n\r", ch );
          break;
     case ITEM_MAP:
          send_to_char ( "Looks like a map.\n\r", ch );
          break;
     case ITEM_COMPONENT:
          send_to_char ( "It has the properties of a magical component.\n\r", ch );
          break;
     case ITEM_PORTAL:
          send_to_char ( "It is a portal to somewhere.\n\r", ch );
          break;
     default:
          break;
     }

     /* Show certain wear flags. */
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_FINGER ) )
          send_to_char ( "It can be worn on your finger.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_NECK ) )
          send_to_char ( "It is worn around the neck.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_BODY ) )
          send_to_char ( "It is worn on the torso.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_HEAD ) )
          send_to_char ( "It is worn on the head.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_LEGS ) )
          send_to_char ( "It is worn on the legs.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_FEET ) )
          send_to_char ( "It is worn on the feet.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_HANDS ) )
          send_to_char ( "It is worn on the hands.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_ARMS ) )
          send_to_char ( "It is worn on the arms.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_SHIELD ) )
          send_to_char ( "It can be used as a shield.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_ABOUT ) )
          send_to_char ( "It is worn over the clothing, around the body.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_WAIST ) )
          send_to_char ( "It is worn around the waist.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_WRIST ) )
          send_to_char ( "It is worn around the wrist.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_TWO_HANDS ) )
          send_to_char ( "It requires two hands to use.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_PRIDE ) )
          send_to_char ( "It is worn as a symbol of pride.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_FACE ) )
          send_to_char ( "It is worn on the face.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_EARS ) )
          send_to_char ( "It is worn on the ears.\n\r", ch );
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_FLOAT ) )
          send_to_char ( "It will float nearby.\n\r", ch );
     /* End of show wear flags */

     /* Show any Extra Flags */
     if ( IS_SET ( obj->extra_flags, ITEM_GLOW ) )
          send_to_char ( "It glows.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_HUM ) )
          send_to_char ( "It is humming.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_DARK ) )
          send_to_char ( "It seems very dark.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_LOCK ) )
          send_to_char ( "It is locked.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_EVIL ) )
          send_to_char ( "It has an evil essence.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_INVIS ) )
          send_to_char ( "It is invisible.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_MAGIC ) )
          send_to_char ( "It is magical.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NODROP ) )
          send_to_char ( "It cannot be dropped.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_BLESS ) )
          send_to_char ( "It is blessed.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_ANTI_GOOD ) )
          send_to_char ( "It cannot be used by the righteous.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_ANTI_NEUTRAL ) )
          send_to_char ( "It cannot be used by those of neutrality.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_ANTI_EVIL ) )
          send_to_char ( "It cannot be used by the corrupt.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NOREMOVE ) )
          send_to_char ( "It is cursed.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NOPURGE ) )
          send_to_char ( "It cannot be purged.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_ROT_DEATH ) )
          send_to_char ( "It has an enchantment that destroys it when the bearer dies.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_VIS_DEATH ) )
          send_to_char ( "It becomes visible once the bearer dies.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NO_SAC ) )
          send_to_char ( "It is not a suitable sacrifice.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NO_COND ) )
          send_to_char ( "It is protected from wear and damage.\n\r", ch );
     if ( IS_SET ( obj->extra_flags, ITEM_NODISP ) )
          send_to_char ( "It is not visible when not carried.\n\r", ch );
     /* End of show extra flags */

     /* Show affects and enchants. */
     if ( !obj->enchanted )
          for ( paf = obj->pIndexData->affected; paf != NULL;
                paf = paf->next )
          {
               if ( paf->location != APPLY_NONE &&
                    paf->modifier != 0 )
               {
                    form_to_char ( ch, "Affects %s by %d.\n\r",
                          affect_loc_name ( paf->location ),
                          paf->modifier );
               }
          }

     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          if ( paf->location != APPLY_NONE && paf->modifier != 0 )
          {
               form_to_char ( ch, "Affects %s by %d.\n\r",
                     affect_loc_name ( paf->location ),
                     paf->modifier );
          }
     }

     return;
}

void spell_infravision ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, DET_INFRARED ) )
     {
          if ( victim == ch )
               send_to_char ( "You can already see in the dark.\n\r", ch );
          else
               act ( "$N already has infravision.\n\r", ch, NULL, victim, TO_CHAR );
          return;
     }
     act ( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );
     af.type = sn;
     af.level = level;
     af.duration = 2 * level;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_PROTECTIONS;
     af.bitvector = DET_INFRARED;
     affect_to_char ( victim, &af );
     send_to_char ( "Your eyes glow red.\n\r", victim );
     return;
}

void spell_invis ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_INVISIBLE ) )
          return;

     act ( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
     af.type = sn;
     af.level = level;
     af.duration = 24;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_INVISIBLE;
     affect_to_char ( victim, &af );
     send_to_char ( "You fade out of existence.\n\r", victim );
     return;
}

void spell_know_alignment ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     char               *msg;
     int                 ap;

     ap = victim->alignment;

     if ( ap > 700 )
          msg = "$N has a pure and good aura.";
     else if ( ap > 350 )
          msg = "$N is of excellent moral character.";
     else if ( ap > 100 )
          msg = "$N is often kind and thoughtful.";
     else if ( ap > -100 )
          msg = "$N doesn't have a firm moral commitment.";
     else if ( ap > -350 )
          msg = "$N lies to $S friends.";
     else if ( ap > -700 )
          msg = "$N is a black-hearted murderer.";
     else
          msg = "$N is the embodiment of pure evil!.";

     act ( msg, ch, NULL, victim, TO_CHAR );
     return;
}

void spell_lightning_bolt ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = spell_calc (ch, sn);

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 20, DAM_LIGHTNING );
     damage ( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
     return;
}

void spell_locate_object ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     char		 buf[MSL];
     char                nbuf[MSL];
     BUFFER		*buffer;
     OBJ_DATA           *obj;
     OBJ_DATA           *in_obj;
     bool                found;
     int                 number = 0, max_found;

     found = FALSE;
     number = 0;
     buffer = buffer_new(1000);

     max_found = IS_IMMORTAL ( ch ) ? 200 : 2 * level;

     for ( obj = object_list; obj != NULL; obj = obj->next )
     {
          if ( !can_see_obj ( ch, obj ) ||
               !is_name ( target_name, obj->name ) ||
               ( !IS_IMMORTAL ( ch ) &&
                 number_percent (  ) > 2 * level ) ||
               ch->level < obj->level )
               continue;

          found = TRUE;
          number++;

          for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
               ;

          if ( in_obj->carried_by != NULL && can_see &&
               !IS_IMMORTAL ( in_obj->carried_by ) )
          {
               SNP ( buf, "%s carried by %s\n\r",
                     obj->short_descr, PERS ( in_obj->carried_by, ch ) );
          }
          else
          {
               SNP ( nbuf, IS_RENTED(ch->in_room->lease) && !IS_NULLSTR(ch->in_room->lease->lease_name) ?
                     ch->in_room->lease->lease_name : ch->in_room->name );

               if ( IS_IMMORTAL ( ch ) && in_obj->in_room != NULL )
                    SNP ( buf, "[{Y%s{x] in [{Y%s{x] Room [{Y%d{x] Level [{Y%d{x]\n\r",
                          obj->short_descr, nbuf,
                          in_obj->in_room->vnum, obj->level );
               else
                    SNP ( buf, "%s in %s\n\r",
                          obj->short_descr, in_obj->in_room == NULL
                          ? "somewhere" : nbuf );
          }

          buf[0] = UPPER ( buf[0] );
          buffer_strcat ( buffer, buf );

          if ( number >= max_found )
               break;
     }

     if ( !found )
          send_to_char ( "Nothing like that in heaven or earth.\n\r", ch );
     else
          page_to_char ( buffer->data, ch );
     buffer_free(buffer);
     return;
}

void spell_magic_missile ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA         *victim = ( CHAR_DATA * ) vo;
     int                dam;
     int		count;
     int		i;

     count = ch->level/10;
     if (count < 1)
          count = 1;
     for (i = 0; i < count ; i++)
     {
          dam = spell_calc (ch, sn);
          check_damage_obj ( victim, NULL, 15, DAM_ENERGY );
          damage ( ch, victim, dam, sn, DAM_ENERGY, TRUE );
     }

     return;
}

void spell_mass_healing ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *gch;
     int                 heal_num, refresh_num;

     heal_num = skill_lookup ( "heal" );
     refresh_num = skill_lookup ( "refresh" );

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( ( IS_NPC ( ch ) && IS_NPC ( gch ) ) ||
               ( !IS_NPC ( ch ) && !IS_NPC ( gch ) ) )
          {
               spell_heal ( heal_num, level, ch, ( void * ) gch );
               spell_refresh ( refresh_num, level, ch, ( void * ) gch );
          }
     }
}

void spell_mass_invis ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     AFFECT_DATA         af;
     CHAR_DATA          *gch;

     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
          if ( !is_same_group ( gch, ch ) ||
               IS_AFFECTED ( gch, AFF_INVISIBLE ) )
               continue;
          act ( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
          send_to_char ( "You slowly fade out of existence.\n\r", gch );
          af.type = sn;
          af.level = level / 2;
          af.duration = 24;
          af.location = APPLY_NONE;
          af.modifier = 0;
          af.where = TO_AFFECTS;
          af.bitvector = AFF_INVISIBLE;
          affect_to_char ( gch, &af );
     }
     send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_mute ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( CAN_DETECT ( victim, AFF_MUTE ) )
     {
          send_to_char ( "That character is already mute.\n\r", ch );
          return;
     }
     if ( saves_spell ( level, victim ) )
     {
          send_to_char ( "Spell failed.\n\r", ch );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = level / 7;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_DETECTIONS;
     af.bitvector = AFF_MUTE;
     affect_to_char ( victim, &af );
     send_to_char ( "Mute spell successful.\n\r", ch );
     send_to_char ( "You have been muted!\n\r", victim );
}

void spell_negate_alignment ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     int                 flags_to_rm = 0;
     int                 risk = 10;

     if ( ch->alignment > 350 )	/*good */
     {
          if ( IS_SET ( obj->extra_flags, ITEM_EVIL ) )
               flags_to_rm += ITEM_EVIL;
          if ( IS_SET ( obj->extra_flags, ITEM_ANTI_GOOD ) )
               flags_to_rm += ITEM_ANTI_GOOD;
     }
     if ( ( ch->alignment <= 350 ) && ( ch->alignment >= -350 ) )	/*neutral */
     {
          if ( IS_SET ( obj->extra_flags, ITEM_ANTI_NEUTRAL ) )
               flags_to_rm += ITEM_ANTI_NEUTRAL;
     }
     else
     {
          if ( IS_SET ( obj->extra_flags, ITEM_ANTI_EVIL ) )
               flags_to_rm += ITEM_ANTI_EVIL;
     }

     if ( !flags_to_rm )
     {
          send_to_char ( "Your god finds nothing offensive about this item.\n\r", ch );
          return;
     }

     if ( ( obj->level ) > ( ch->level + 10 ) )	/*risky */
          risk += 5 * ( ( obj->level ) - ( ch->level + 10 ) );

     /* Loth - Fixed here where item was extracted before act was sent to prevent crash */
     if ( number_percent (  ) <= risk )
     {
          send_to_char ( "{rYou have offended your god! {mKaboom!{x  {rThe item explodes!  *sigh*{x\n\r", ch );
          act ( "{r$p shivers violently and explodes!{x", ch, obj, NULL, TO_ROOM );
          extract_obj ( obj );
          return;
     }

     if ( number_percent (  ) <
          ( ch->level * 2 / 3 +
            ( get_curr_stat ( ch, STAT_WIS ) - 20 ) ) )
     {
          send_to_char ( "{cYour gods have favored you...they negate the alignment of the item. {x\n\r", ch );
          act ( "{c$p glows with the color of neutrality{c", ch, obj, NULL, TO_ROOM );
          obj->extra_flags -= flags_to_rm;
          return;
     }
     else
     {
          send_to_char ( "The item resists your efforts at negation.\n\r", ch );
          return;
     }
}
/*end spell */

void spell_null ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     send_to_char ( "That's not a spell!\n\r", ch );
     return;
}

void spell_pass_door ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_PASS_DOOR ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already out of phase.\n\r", ch );
          else
               act ( "$N is already shifted out of phase.", ch, NULL, victim, TO_CHAR );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = number_fuzzy ( level / 4 );
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_PASS_DOOR;
     affect_to_char ( victim, &af );
     act ( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
     send_to_char ( "You turn translucent.\n\r", victim );
     return;
}

/* RT plague spell, very nasty */
void spell_plague ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( saves_spell ( level, victim ) ||
          ( IS_NPC ( victim ) &&
            IS_SET ( victim->act, ACT_UNDEAD ) ) )
     {
          if ( ch == victim )
               send_to_char ( "You feel momentarily ill, but it passes.\n\r", ch );
          else
               act ( "$N seems to be unaffected.", ch, NULL, victim, TO_CHAR );
          return;
     }

     af.type = sn;
     af.level = level * 3 / 4;
     af.duration = level;
     af.location = APPLY_STR;
     af.modifier = -5;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_PLAGUE;
     affect_join ( victim, &af );

     send_to_char ( "You scream in agony as plague sores erupt from your skin.\n\r", victim );
     act ( "$n screams in agony as plague sores erupt from $s skin.", victim, NULL, NULL, TO_ROOM );
}

void spell_poison ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim;
     AFFECT_DATA         af;

     victim = ( CHAR_DATA * ) vo;

     if ( saves_spell ( level, victim ) )
     {
          act ( "$n turns slightly green, but it passes.", victim, NULL, NULL, TO_ROOM );
          send_to_char ( "You feel momentarily ill, but it passes.\n\r", victim );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = level;
     af.location = APPLY_STR;
     af.modifier = -2;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_POISON;
     affect_join ( victim, &af );
     send_to_char ( "You feel very sick.\n\r", victim );
     act ( "$n looks very ill.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_portal ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                buf[MAX_STRING_LENGTH];
     char               *remainder;
     CHAR_DATA          *victim;
     OBJ_DATA           *portal1;
     OBJ_DATA           *portal2;
     int                 portal_level;

     if ( target_name == NULL || target_name[0] == '\0' )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     remainder = one_argument ( target_name, arg1 );

     if ( remainder != NULL && remainder[0] != '\0' && is_number ( remainder ) )
          portal_level = atoi ( remainder );
     else
          portal_level = level;

     if ( portal_level > level )
          portal_level = level;

     target_name = arg1;

     if ( ( victim = get_char_world ( ch, target_name ) ) == NULL || victim == ch
          || victim->in_room == NULL || !can_see_room ( ch, victim->in_room )
          || IS_SET ( victim->in_room->room_flags, ROOM_SAFE )
          || IS_SET ( victim->in_room->room_flags, ROOM_PRIVATE )
          || IS_SET ( victim->in_room->room_flags, ROOM_SOLITARY )
          || IS_SET ( victim->in_room->room_flags, ROOM_NO_RECALL )
          || IS_SET ( ch->in_room->room_flags, ROOM_NO_RECALL )
          || victim->level >= level + 3
          || ( !IS_NPC ( victim ) && victim->level >= LEVEL_HERO )	/* NOT trust */
          || ( IS_NPC ( victim ) &&
               IS_SET ( victim->imm_flags, IMM_SUMMON ) ) ||
          ( !IS_NPC ( victim ) &&
            IS_SET ( victim->act, PLR_NOSUMMON ) ) ||
          ( IS_NPC ( victim ) && saves_spell ( level, victim ) ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     /* caster room messages */
     send_to_char ( "You swirl magic in front of you to create a mystical portal!\n\r",  ch );
     act ( "$n swirls mystic energy together to form a portal!", ch, NULL, NULL, TO_ROOM );
     /* victim room message */
     act ( "A sudden swirl of magical energy transforms into a portal!", victim, NULL, NULL, TO_ROOM );

     /*create the portals now */
     portal1 = create_object ( get_obj_index ( OBJ_VNUM_PORTAL ), portal_level );
     portal1->level = portal_level;
     portal1->timer = level / 5;
     portal1->value[0] = victim->in_room->vnum;
     portal1->value[1] = OBJ_VNUM_PORTAL;
     free_string ( portal1->description );

     SNP ( buf, "A portal leading to %s is hovering here.",
           IS_RENTED(victim->in_room->lease) && !IS_NULLSTR(victim->in_room->lease->lease_name) ?
           victim->in_room->lease->lease_name : victim->in_room->name );
     portal1->description = str_dup ( buf );

     portal2 =	create_object ( get_obj_index ( OBJ_VNUM_PORTAL ), portal_level );
     portal2->level = portal_level;
     portal2->timer = level / 5;
     portal2->value[0] = ch->in_room->vnum;
     portal2->value[1] = OBJ_VNUM_PORTAL;
     free_string ( portal2->description );
     SNP ( buf, "A portal leading to %s is hovering here.",
           IS_RENTED(ch->in_room->lease) && !IS_NULLSTR(ch->in_room->lease->lease_name) ?
           ch->in_room->lease->lease_name : ch->in_room->name );
     portal2->description = str_dup ( buf );

    /* put portals in rooms */

     obj_to_room ( portal1, ch->in_room );
     obj_to_room ( portal2, victim->in_room );

     return;
}

void spell_protection_evil ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_PROTECTED ( victim, PROT_EVIL ) ||
          IS_PROTECTED ( victim, PROT_GOOD ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already protected.\n\r", ch );
          else
               act ( "$N is already protected.", ch, NULL, victim, TO_CHAR );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = 24;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_PROTECTIONS;
     af.bitvector = PROT_EVIL;
     affect_to_char ( victim, &af );
     send_to_char ( "You feel protected from evil.\n\r", victim );
     if ( ch != victim )
          act ( "$N is protected from harm.", ch, NULL, victim, TO_CHAR );
     return;
}

void spell_protection_good ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_PROTECTED ( victim, PROT_EVIL ) ||
          IS_PROTECTED ( victim, PROT_GOOD ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already protected.\n\r", ch );
          else
               act ( "$N is already protected.", ch, NULL, victim, TO_CHAR );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = 24;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_PROTECTIONS;
     af.bitvector = PROT_GOOD;
     affect_to_char ( victim, &af );
     send_to_char ( "You feel protected from good.\n\r", victim );
     if ( ch != victim )
          act ( "$N is protected from harm.", ch, NULL, victim, TO_CHAR );
     return;
}

void spell_psychic_anchor ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     if ( IS_AFFECTED ( ch, AFF_MELD ) )
     {
          send_to_char ( "Your head is too scrambled to concentrate on this spell.\n\r", ch );
          return;
     }

     if ( IS_SET ( ch->in_room->room_flags, ROOM_PRIVATE ) )
     {
          send_to_char ( "You cannot make a private room your recall point.\n\r", ch );
          return;
     }

     send_to_char ( "You focus your thoughts and form a perfect mental image of this room.\n\r", ch );
     send_to_char ( "Word of recall will now bring you to this room.\n\r", ch );
     ch->recall_temp = ch->in_room->vnum;
     return;
}

void spell_mask_self ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( ch, AFF_POLY ) )
     {
          send_to_char ( "You are already masked.\n\r", ch );
          return;
     }
     if ( victim == ch )
     {
          send_to_char ( "You must specify a target creature.\n\r", ch );
          return;
     }

     if ( ch->level <= HERO && !IS_NPC ( victim ) )
     {
          send_to_char ( "You can only mask yourself to look like mobs.\n\r", ch );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = level;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_POLY;
     affect_to_char ( ch, &af );
     send_to_char ( "You reshape yourself into the likeness of your target.\n\r", ch );

    /*save old descriptions */
     free_string ( ch->description_orig );
     free_string ( ch->short_descr_orig );
     free_string ( ch->long_descr_orig );
     ch->description_orig = str_dup ( ch->description );
     ch->short_descr_orig = str_dup ( ch->short_descr );
     ch->long_descr_orig = str_dup ( ch->long_descr );

    /*apply poly descriptions */
     free_string ( ch->description );
     free_string ( ch->short_descr );
     free_string ( ch->long_descr );
     ch->description = str_dup ( victim->description );
     if ( IS_NPC ( victim ) )
          ch->short_descr = str_dup ( victim->short_descr );
     else
          ch->short_descr = str_dup ( victim->name );
     ch->long_descr = str_dup ( victim->long_descr );
     free_string ( ch->poly_name );
     ch->poly_name = str_dup ( victim->name );
     ch->start_pos = victim->start_pos;
     return;
}

void spell_refresh ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     victim->move = UMIN ( victim->move + level, victim->max_move );
     if ( victim->max_move == victim->move )
          send_to_char ( "You feel fully refreshed!\n\r", victim );
     else
          send_to_char ( "You feel less tired.\n\r", victim );
     if ( ch != victim )
          send_to_char ( "Ok.\n\r", ch );
     return;
}

void spell_regeneration ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_REGENERATION ) )
     {
          send_to_char ( "That target is already regenerating.\n\r", ch );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = 24;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_REGENERATION;
     affect_to_char ( victim, &af );
     send_to_char ( "You feel yourself healing your wounds more quickly.\n\r", victim );
     act ( "$n is healing much more quickly.", victim, NULL, NULL, TO_ROOM );
     return;
}

void spell_remove_curse ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     bool                found = FALSE;
     OBJ_DATA           *obj;
     int                 iWear;

     if ( check_dispel ( level, victim, gsn_curse ) )
     {
          send_to_char ( "You feel better.\n\r", victim );
          act ( "$n looks more relaxed.", victim, NULL, NULL, TO_ROOM );
     }

     for ( iWear = 0; ( iWear < MAX_WEAR && !found ); iWear++ )
     {
          if ( ( obj = get_eq_char ( victim, iWear ) ) == NULL )
               continue;

          if ( IS_OBJ_STAT ( obj, ITEM_NODROP ) ||
               IS_OBJ_STAT ( obj, ITEM_NOREMOVE ) )
          {			/* attempt to remove curse */
               if ( !saves_dispel ( level, obj->level, 0 ) )
               {
                    found = TRUE;
                    REMOVE_BIT ( obj->extra_flags, ITEM_NODROP );
                    REMOVE_BIT ( obj->extra_flags, ITEM_NOREMOVE );
                    act ( "$p glows blue.", victim, obj, NULL, TO_CHAR );
                    act ( "$p glows blue.", victim, obj, NULL, TO_ROOM );
               }
          }
     }

     for ( obj = victim->carrying; ( obj != NULL && !found ); obj = obj->next_content )
     {
          if ( IS_OBJ_STAT ( obj, ITEM_NODROP ) ||
               IS_OBJ_STAT ( obj, ITEM_NOREMOVE ) )
          {			/* attempt to remove curse */
               if ( !saves_dispel ( level, obj->level, 0 ) )
               {
                    found = TRUE;
                    REMOVE_BIT ( obj->extra_flags, ITEM_NODROP );
                    REMOVE_BIT ( obj->extra_flags, ITEM_NOREMOVE );
                    act ( "Your $p glows blue.", victim, obj, NULL, TO_CHAR );
                    act ( "$n's $p glows blue.", victim, obj, NULL, TO_ROOM );
               }
          }
     }
     return;
}

void spell_remove_fear ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;

     if ( check_dispel ( level, victim, skill_lookup ( "fear" ) ) )
     {
          act ( "$n no longer looks terrified.", victim, NULL, NULL,
                TO_ROOM );
     }
     else
          send_to_char ( "Spell failed.\n\r", ch );
     return;
}

void spell_remove_invis ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     OBJ_DATA           *obj = ( OBJ_DATA * ) vo;
     int                 risk = 5;

     if ( !IS_SET ( obj->extra_flags, ITEM_INVIS ) )
     {
          send_to_char ( "Strange, everyone else can already see that item.\n\r", ch );
          send_to_char ( "Spell failed.\n\r", ch );
          return;
     }

     if ( ( obj->level ) > ( ch->level + 5 ) )	/*risky */
          risk += 4 * ( ( obj->level ) - ( ch->level + 10 ) );

     /* hack, remove invis now so act's will show it properly */
     obj->extra_flags -= ITEM_INVIS;

     /* Yet another extract_obj moved to AFTER the last reference of the item.. Geez Z, didn't
      * you pay attention in the "Don't Reference Null Pointers" class?
      */
     if ( number_percent (  ) <= risk )
     {
          send_to_char ( "Just as the item starts to appear...{mKaboom!{x  It {mexplodes!{x\n\r", ch );
          act ( "$p appears suddenly...then shivers violently and {mexplodes{!{x", ch, obj, NULL, TO_ROOM );
          extract_obj ( obj );
          return;
     }
     if ( number_percent (  ) < ( ch->level * 2 / 3 + ( get_curr_stat ( ch, STAT_INT ) - 20 ) ) )
     {
          send_to_char ( "{cA bright flash of light appears and fades, revealing the item to all.{x\n\r", ch );
          act ( "{cA bright flash of light appears and fades, revealing $p.{x", ch, obj, NULL, TO_ROOM );
          send_to_char ( "Spell successful.\n\r", ch );
          return;
     }
     else
     {
          send_to_char ( "The item resists your efforts at removing its invisibility.\n\r", ch );
          obj->extra_flags += ITEM_INVIS;
          return;
     }
}

void spell_ghostform ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_PROTECTED ( victim, PROT_SANCTUARY ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already protected.\n\r", ch );
          else
               act ( "$N is already protected.", ch, NULL, victim, TO_CHAR );
          return;
     }
     if (victim->alignment >= -250)
     {
          if (victim != ch)
          {
               send_to_char ( "The demons of hell refuse your request.\n\r", ch);
               send_to_char ( "You shiver as demons pass nearby.\n\r", victim);
          }
          else
          {
               send_to_char ("The demons of hell refuse to protect you.\n\r", ch);
          }
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = number_fuzzy ( level / 6 );
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_PROTECTIONS;
     af.bitvector = PROT_SANCTUARY;
     affect_to_char ( victim, &af );
     act ( "$n is surrounded by a ghostly white aura.", victim, NULL, NULL, TO_ROOM );
     send_to_char ( "You are surrounded by a ghostly white aura.\n\r", victim );
     return;
}

void spell_sanctuary ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_PROTECTED ( victim, PROT_SANCTUARY ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already protected.\n\r", ch );
          else
               act ( "$N is already protected.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if (victim->alignment <= 250)
     {
          if (victim != ch)
          {
               send_to_char ( "The angels politely decline your request.\n\r", ch);
               send_to_char ( "You catch a glimpse of the divine, then nothing.\n\r", victim);
          }
          else
          {
               send_to_char ("You are lucky the angels of heaven are merciful.\n\r", ch);
          }
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = number_fuzzy ( level / 6 );
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_PROTECTIONS;
     af.bitvector = PROT_SANCTUARY;
     affect_to_char ( victim, &af );
     act ( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
     send_to_char ( "You are surrounded by a white aura.\n\r", victim );
     return;
}

void spell_shield ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "You are already shielded from harm.\n\r", ch );
          else
               act ( "$N is already protected by a shield.", ch, NULL, victim, TO_CHAR );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = 8 + level;
     af.location = APPLY_AC;
     af.modifier = 20;
     af.bitvector = 0;
     affect_to_char ( victim, &af );
     act ( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
     send_to_char ( "You are surrounded by a force shield.\n\r", victim );
     return;
}

void spell_shocking_grasp ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = spell_calc (ch, sn);

     if ( chance(20) && !IS_NPC(ch) && ch->pcdata->pclass == CLASS_CHAOSMAGE )
     {
          send_to_char("A small amount of feedback jolts you!\n\r", ch );
          damage ( ch, ch, (dam/4), sn, DAM_LIGHTNING, TRUE );

          victim = ch;
     }
     else if ( chance (2) && !IS_NPC(ch) && ch->pcdata->pclass == CLASS_CHAOSMAGE )
     {
          send_to_char("You failed to ground yourself properly and the charge hits you instead!\n\r", ch);
          victim = ch;
     }

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
     return;
}

void spell_sleep ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( IS_AFFECTED ( victim, AFF_SLEEP )
          || ( IS_NPC ( victim ) && IS_SET ( victim->act, ACT_UNDEAD ) )
          || level < victim->level
          || saves_spell ( level, victim ) )
          return;
     af.type = sn;
     af.level = level;
     af.duration = 4 + level;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_SLEEP;
     affect_join ( victim, &af );

     if ( IS_AWAKE ( victim ) )
     {
          send_to_char ( "You feel very sleepy ..... zzzzzz.\n\r", victim );
          act ( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
          victim->position = POS_SLEEPING;
     }
     return;
}

void spell_stone_skin ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( ch, sn ) )
     {
          if ( victim == ch )
               send_to_char ( "Your skin is already as hard as a rock.\n\r", ch );
          else
               act ( "$N is already as hard as can be.", ch, NULL, victim, TO_CHAR );
          return;
     }
     af.type = sn;
     af.level = level;
     af.duration = level;
     af.location = APPLY_AC;
     af.modifier = 40;
     af.bitvector = 0;
     affect_to_char ( victim, &af );
     act ( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
     send_to_char ( "Your skin turns to stone.\n\r", victim );
     return;
}

void spell_summon ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim;

     if ( ( victim = get_char_world ( ch, target_name ) ) == NULL
          || victim == ch
          || victim->in_room == NULL
          || IS_SET ( victim->in_room->room_flags, ROOM_SAFE )
          || IS_SET ( victim->in_room->room_flags, ROOM_PRIVATE )
          || IS_SET ( victim->in_room->room_flags, ROOM_SOLITARY )
          || IS_SET ( victim->in_room->room_flags, ROOM_NO_RECALL )
          || ( IS_NPC ( victim ) && IS_SET ( victim->act, ACT_AGGRESSIVE ) )
          || victim->level >= level + 3 || ( !IS_NPC ( victim )
                                             && victim->level >= LEVEL_HERO )
          || victim->fighting != NULL || ( IS_NPC ( victim )
                                           && IS_SET ( victim->imm_flags, IMM_SUMMON ) )
          || ( !IS_NPC ( victim ) && IS_SET ( victim->act, PLR_NOSUMMON ) )
          || ( IS_NPC ( victim ) && saves_spell ( level, victim ) ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }

     act ( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
     char_from_room ( victim );
     char_to_room ( victim, ch->in_room );
     act ( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
     act ( "$n has summoned you!", ch, NULL, victim, TO_VICT );
     do_look ( victim, "auto" );
     return;
}

/*
 * teleport has been rewritten to find an area before selecting a vnum.
 * This has a twofold benefit: Very large areas will no longer have a higher
 * chance of being hit, thus equalizing the chance of being sent to any area,
 * and Most Importantly, it no longer has to loop as many times due to the 
 * very high number of available vnums on the mud. - Lotherius
 * 
 * Now moved off to get_random_room in db.c, so that telepops can take advantage
 * of the more efficient way of doing it.
 */

void spell_teleport ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     ROOM_INDEX_DATA    *pRoomIndex = NULL;

     if ( victim->in_room == NULL
          || IS_SET ( victim->in_room->room_flags, ROOM_NO_RECALL )
          || ( !IS_NPC ( ch ) && victim->fighting != NULL )
          || ( victim != ch && ( saves_spell ( level, victim )
                                 || saves_spell ( level, victim ) ) ) )
     {
          send_to_char ( "You failed.\n\r", ch );
          return;
     }
     
          
     {
          int counter = 0;
          while ( pRoomIndex == NULL && counter <= 5000 )
          {
               counter++;
               pRoomIndex = get_random_room ( 1, ch );
          }
          if ( pRoomIndex == NULL )
          {
               bugf ( "Couldn't find a random room after 5000 tries on spell_teleport." );
               send_to_char ( "You feel funny for a second, then it passes.\n\r", victim );
               send_to_char ( "Hmm... that shoulda worked.\n\r", ch );
               return;
          }
     }
     
     
     while ( pRoomIndex == NULL )
          pRoomIndex = get_random_room ( 1, ch );
     
     if ( victim != ch )
          send_to_char ( "You have been teleported!\n\r", victim );

     act ( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
     char_from_room ( victim );
     char_to_room ( victim, pRoomIndex );
     act ( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
     do_look ( victim, "auto" );
     return;
}

/*
 * This spell perhaps needs to use PERSMASK?
 */

void spell_ventriloquate ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     char                buf1[MAX_STRING_LENGTH];
     char                buf2[MAX_STRING_LENGTH];
     char                speaker[MAX_INPUT_LENGTH];
     CHAR_DATA          *vch;

     target_name = one_argument ( target_name, speaker );

     SNP ( buf1, "%s says '%s'.\n\r", speaker, target_name );
     SNP ( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
     buf1[0] = UPPER ( buf1[0] );

     for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
     {
          if ( !is_name ( speaker, vch->name ) )
               send_to_char ( saves_spell ( level, vch ) ? buf2 : buf1, vch );
     }

     return;
}

void spell_vocalize ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn ) )
     {
          send_to_char ( "That target already has a voice.\n\r", ch );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = level / 7;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.bitvector = 0;
     affect_to_char ( victim, &af );
     send_to_char ( "Vocalize spell successful.\n\r", ch );
     send_to_char ( "You feel you can cast spells without speaking.\n\r", victim );
     return;
}

void spell_weaken ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;

     if ( is_affected ( victim, sn )
          || saves_spell ( level, victim ) )
          return;
     af.type = sn;
     af.level = level;
     af.duration = level / 2;
     af.location = APPLY_STR;
     af.modifier = -1 * ( level / 5 );
     af.where = TO_AFFECTS;
     af.bitvector = AFF_WEAKEN;
     affect_to_char ( victim, &af );
     send_to_char ( "You feel weaker.\n\r", victim );
     act ( "$n looks tired and weak.", victim, NULL, NULL, TO_ROOM );
     return;
}

/* RT recall spell is back */
void spell_word_of_recall ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     ROOM_INDEX_DATA    *location;

     if ( IS_NPC ( victim ) )
          return;

    /* Zeran - changeable recall room */
     if ( ( location = get_room_index ( ch->recall_temp ) ) == NULL )
     {
          if ( ( location = get_room_index ( ch->recall_perm ) ) == NULL )
          {
               send_to_char ( "You are completely lost.\n\r", victim );
               return;
          }
     }

     if ( IS_SET ( victim->in_room->room_flags, ROOM_NO_RECALL ) ||
          IS_AFFECTED ( victim, AFF_CURSE ) )
     {
          send_to_char ( "Spell failed.\n\r", victim );
          return;
     }

     if ( victim->fighting != NULL )
          stop_fighting ( victim, TRUE );

     ch->move /= 2;
     act ( "$n disappears.", victim, NULL, NULL, TO_ROOM );
     char_from_room ( victim );
     char_to_room ( victim, location );
     act ( "$n appears in the room.", victim, NULL, NULL, TO_ROOM );
     do_look ( victim, "auto" );
     return;
}

/*
 * NPC spells.
 * Some expanded a bit.
 */
void spell_acid_breath ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     OBJ_DATA           *obj_lose;
     OBJ_DATA           *obj_next;
     int                 dam;

     if ( number_percent (  ) < 2 * level && !saves_spell ( level, victim ) )
     {
          for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
          {
               obj_next = obj_lose->next_content;

               if ( number_bits ( 2 ) != 0 )
                    continue;

               // Now we're just going to damage the object instead of all the wierd
               // stuff.
               //
               if ( obj_lose->condition > 0 )
                    damage_obj ( ch, obj_lose, 5, DAM_ACID );
          }
     }
     dam = spell_calc (ch, sn);
     if ( saves_spell ( level, victim ) ) dam /= 2;
     damage ( ch, victim, dam, sn, DAM_ACID, TRUE );
     return;
}

void spell_fire_breath ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     OBJ_DATA           *obj_lose;
     OBJ_DATA           *obj_next;
     int                 dam;

     if ( number_percent (  ) < 2 * level && !saves_spell ( level, victim ) )
     {
          for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
          {
               obj_next = obj_lose->next_content;
               if ( number_bits ( 2 ) != 0 )
                    continue;

               // Now we're just going to damage the object instead of all the wierd
               // stuff.
               //
               if ( obj_lose->condition > 0 )
                    damage_obj ( ch, obj_lose, 5, DAM_FIRE );
          }
     }

     dam = spell_calc (ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_FIRE, TRUE );
     return;
}

void spell_frost_breath ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     OBJ_DATA           *obj_lose;
     OBJ_DATA           *obj_next;
     int                 dam;

     if ( number_percent (  ) < 2 * level && !saves_spell ( level, victim ) )
     {
          for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
          {
               obj_next = obj_lose->next_content;
               if ( number_bits ( 2 ) != 0 )
                    continue;

               // Now we're just going to damage the object instead of all the wierd
               // stuff.
               //
               if ( obj_lose->condition > 0 )
                    damage_obj ( ch, obj_lose, 5, DAM_COLD );
          }
     }
     dam = spell_calc (ch, sn);
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_COLD, TRUE );
     return;
}

void spell_gas_breath ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;
     OBJ_DATA           *obj_lose;
     OBJ_DATA           *obj_next;
     int                 dam;

     for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
     {
          vch_next = vch->next_in_room;
          if ( !is_safe_spell ( ch, vch, TRUE ) )
          {
               if ( number_percent (  ) < 2 * level && !saves_spell ( level, vch ) )
               {
                    for ( obj_lose = vch->carrying; obj_lose != NULL; obj_lose = obj_next )
                    {
                         obj_next = obj_lose->next_content;
                         if ( number_bits ( 2 ) != 0 )
                              continue;

                         // Now we're just going to damage the object instead of all the wierd
                         // stuff.
                         //
                         if ( obj_lose->condition > 0 )
                              damage_obj ( ch, obj_lose, 5, DAM_POISON );
                    }
               }
               dam = spell_calc(ch, sn);
               if ( saves_spell ( level, vch ) )
                    dam /= 2;
               damage ( ch, vch, dam, sn, DAM_POISON, TRUE );
          }
     }
     return;
}

void spell_lightning_breath ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     OBJ_DATA           *obj_lose;
     OBJ_DATA           *obj_next;
     int                 dam;

     if ( number_percent (  ) < 2 * level && !saves_spell ( level, victim ) )
     {
          for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next )
          {
               obj_next = obj_lose->next_content;
               if ( number_bits ( 2 ) != 0 )
                    continue;

               // Now we're just going to damage the object instead of all the wierd
               // stuff.
               //
               if ( obj_lose->condition > 0 )
                    damage_obj ( ch, obj_lose, 5, DAM_COLD );
          }
     }
     dam = spell_calc(ch, sn);
     if ( saves_spell ( level, victim ) ) dam /= 2;
     damage ( ch, victim, dam, sn, DAM_LIGHTNING, TRUE );
     return;
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = number_range ( 25, 100 );
     if ( saves_spell ( level, victim ) )
          dam /= 2;
     damage ( ch, victim, dam, sn, DAM_PIERCE, TRUE );
     return;
}

void spell_high_explosive ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = spell_calc(ch, sn) + ch->level;

     if ( chance(20) && !IS_NPC(ch) && ch->pcdata->pclass == CLASS_CHAOSMAGE )
     {
          send_to_char("Your high explosive blows up in your face!\n\r", ch );
          victim = ch;
     }

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 40, DAM_BASH );
     damage ( ch, victim, dam, sn, DAM_BASH, TRUE );
     return;
}

void spell_mind_meld ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     AFFECT_DATA         af;
     int                 dam;

     if ( IS_AFFECTED ( victim, AFF_MELD ) ||
          saves_spell ( level, victim ) )
     {
          send_to_char ( "Your efforts fail to produce melding.\n\r", ch );
          return;
     }

     af.type = sn;
     af.level = level;
     af.duration = 2 + level;
     af.location = APPLY_INT;
     af.modifier = -5;
     af.where = TO_AFFECTS;
     af.bitvector = AFF_MELD;
     affect_to_char ( victim, &af );

     dam = dice ( 6, level );
     damage ( ch, victim, dam, sn, DAM_MENTAL, TRUE );

     act ( "$N has been Mind Melded.", ch, NULL, victim, TO_CHAR );
     send_to_char ( "You feel an immense pain in your head!\n\r", victim );
     act ( "$N grimaces in pain!", ch, NULL, victim, TO_NOTVICT );
     return;
}

void spell_chaos ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     CHAR_DATA          *tmp_vict, *last_vict, *next_vict;
     bool                found;
     int                 dam;
     int                 damtype;

     /* this spell uses a bit of OLC code. nasty, but it was the quickest way
      * to get the damage names from numbers into text form.
      * It started out based on chain lightning, but has got much more advanced.
      * Spell 'chaos' by Lotherius
      */

     damtype = number_range ( 4, 17 );
     act ( "A cracking sound fills the room as $n bursts into energy, sending an arc of pure magical force into $N.",
           ch, NULL, victim, TO_ROOM );
     if ( IS_NPC ( victim ) )
     {
          form_to_char ( ch, "With a crack, your %s hits %s!\n\r",
                         flag_string ( damage_type, damtype ),
                         victim->short_descr );
     }
     else
     {
          form_to_char ( ch, "With a crack, your %s hits %s!\n\r",
                         flag_string ( damage_type, damtype ),
                         victim->name );
          form_to_char ( victim, "You are hit by %s's %s!\n\r",
                         ch->name, flag_string ( damage_type, damtype ) );
     }

     dam = dice ( level, 6 );
     if ( saves_spell ( level, victim ) )
          dam /= 3;
     check_damage_obj ( victim, NULL, 10, damtype );
     damage ( ch, victim, dam, sn, damtype, TRUE );
     last_vict = victim;
     level -= 3;			/* decrement damage */

    /* new targets */
     while ( level > 0 )
     {
          found = FALSE;
          for ( tmp_vict = ch->in_room->people; tmp_vict != NULL; tmp_vict = next_vict )
          {
               next_vict = tmp_vict->next_in_room;
               if ( !is_safe_spell ( ch, tmp_vict, TRUE ) && tmp_vict != last_vict )
               {
                    found = TRUE;
                    last_vict = tmp_vict;
                    act ( "$n is enveloped by the raw power!", tmp_vict, NULL, NULL, TO_ROOM );
                    dam = dice ( level, 6 );
                    if ( saves_spell ( level, tmp_vict ) )
                         dam /= 3;
                    damtype = number_range ( 4, 17 );
                    if ( IS_NPC ( victim ) )
                    {
                         form_to_char ( ch, "Your %s hits %s!\n\r",
                               flag_string ( damage_type, damtype ),
                               victim->short_descr );
                    }
                    else
                    {
                         form_to_char ( ch, "Your %s hits %s!\n\r",
                                        flag_string ( damage_type, damtype ),
                                        victim->name );
                         form_to_char ( victim, "You are hit by %s's %s!\n\r",
                                        ch->name,
                                        flag_string ( damage_type, damtype ) );
                    }
                    check_damage_obj ( tmp_vict, NULL, 10, damtype );
                    damage ( ch, tmp_vict, dam, sn, damtype, TRUE );
                    level -= 4;	/* decrement damage */
               }
          }
          /* end target searching loop */
          if ( !found )		/* no target found, hit the caster */
          {
               if ( ch == NULL )
                    return;

               if ( last_vict == ch )	/* no double hits */
               {
                    act ( "The energy flows back into the ether.", ch, NULL, NULL, TO_ROOM );
                    act ( "The energy returns to you, somewhat spent.", ch, NULL, NULL, TO_CHAR );
                    return;
               }

               last_vict = ch;
               act ( "The magic backfires on $n...whoops!", ch, NULL, NULL, TO_ROOM );
               damtype = number_range ( 4, 17 );
               form_to_char ( ch, "Your %s hits YOU!\n\r", flag_string ( damage_type, damtype ) );
               dam = dice ( level, 6 );
               if ( saves_spell ( level, ch ) )
                    dam /= 3;
               check_damage_obj ( ch, NULL, 10, damtype );
               damage ( ch, ch, dam, sn, damtype, TRUE );
               level -= 4;		/* decrement damage */
               if ( ch == NULL )
                    return;
          }
          /* now go back and find more targets */
     }
     return;
}

void spell_psi_twister ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;

     send_to_char ( "You release your mental power to do its will in the room!\n\r", ch );
     act ( "$n levitates amid a swirl of psychic energy.", ch, NULL, NULL, TO_ROOM );

     for ( vch = char_list; vch != NULL; vch = vch_next )
     {
          vch_next = vch->next;
          if ( vch->in_room == NULL )
               continue;
          if ( vch->in_room == ch->in_room )
          {
               if ( vch != ch && !is_safe_spell ( ch, vch, TRUE ) )
               {
                    if ( saves_spell ( level, vch ) )
                         damage ( ch, vch, 1, sn, DAM_MENTAL, TRUE );
                    else
                         damage ( ch, vch, spell_calc(ch, sn), sn, DAM_MENTAL, TRUE );
               }
               continue;
          }
     }
     return;
}

/*
 * Brew by Zeran
 * Modified by Lotherius
 */

void do_brew ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj = NULL;
     int                 sn;
     char                typebuf[128];
     int                 chance;
     int                 mana;
     char                arg1[MAX_INPUT_LENGTH];
     char               *spell;

     one_argument ( argument, arg1 );

     /*
      * NPC's can't brew at all.
      */
     if ( IS_NPC ( ch ) )
          return;

     if ( ch->pcdata->pclass != class_lookup ( "avenger" ) )
     {
          send_to_char ( "Only Avengers or Defilers may brew potions.\n\r", ch );
          return;
     }

     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Brew what?\n\r", ch );
          return;
     }

     if ( ( sn = skill_lookup ( arg1 ) ) < 0
          || ( !IS_NPC ( ch ) && ch->level < skill_table[sn].skill_level[ch->pcdata->pclass] )
          || ( ch->pcdata->learned[sn] < 1 ) )
     {
          send_to_char ( "You don't know any spells of that name.\n\r", ch );
          return;
     }

     if ( skill_table[sn].spell_fun == spell_null )
     {
          send_to_char ( "That's not a spell!\n\r", ch );
          return;
     }

     /* See if spell is brewable */
     /* need a new routine here since I removed the "brew chart" - Lotherius */
     /* Verify character has mana and components. */
     /* need to add vial or jar here later */

     if ( ch->level + 2 == skill_table[sn].skill_level[ch->pcdata->pclass] )
          mana = 50;
     else
          mana = 1.5 *
          ( UMAX ( skill_table[sn].min_mana, 100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->pcdata->pclass] ) ) );

     if ( ch->mana < mana )
     {
          send_to_char ( "You do not have enough mana to brew that.\n\r", ch );
          return;
     }

     if ( str_cmp ( skill_table[sn].component, "" ) &&
          ( obj = get_obj_carry ( ch, skill_table[sn].component, NULL ) ) ==  NULL )
     {
          form_to_char ( ch, "You need '%s' to brew that potion.\n\r",
                         skill_table[sn].component );
          return;
     }
     /* Ok, checks done, lets whip up a potion! */
     ch->mana -= mana;
     if ( obj != NULL )
          extract_obj ( obj );

     chance = ch->level / 2 + ch->pcdata->learned[sn] / 2;
     if ( number_percent (  ) > chance )
     {
          send_to_char ( "Oops, you misread the recipe.\n\r", ch );
          return;
     }

     obj = create_object ( ( get_obj_index ( OBJ_POTION ) ), 0 );
     obj->value[0] = ch->level;
     obj->value[1] = sn;
     obj->level = 1;
     obj->timer = UMAX ( 24, ( ch->level * 2 ) );
     free_string ( obj->name );
     free_string ( obj->short_descr );
     free_string ( obj->description );

     spell = skill_table[sn].name;

     SNP ( typebuf, "potion %s", spell );
     obj->name = str_dup ( typebuf );
     SNP ( typebuf, "a potion of %s", spell );
     obj->short_descr = str_dup ( typebuf );
     SNP ( typebuf, "A potion of %s is just lying here.", spell );
     obj->description = str_dup ( typebuf );

     /* Zeran - make waitstate *before* obj is put in room, so brewers don't
      have their new creations stolen while they are stuck in a waitstate. */
     /* Lotherius - Umm... Z... the wait state doesn't pause the whole mud... this
      * changes nothing, the player is still waited after the obj is placed. We aren't
      * running multi-threaded here ya know.
      */

     if ( !IS_IMMORTAL ( ch ) )
          WAIT_STATE ( ch, PULSE_VIOLENCE / 2 );	/*lets not brew at lightspeed */
     obj_to_room ( obj, ch->in_room );
     send_to_char ( "You whip up a fantastic concoction!\n\r", ch );
     act ( "$n grabs a kettle and whips up a potion.", ch, NULL, NULL, TO_ROOM );
     return;
}

/*
 * Scribe by Zeran
 * Modified by Lotherius
 */

void do_scribe ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj = NULL;
     OBJ_DATA           *obj2 = NULL;
     int                 sn;
     char                typebuf[128];
     int                 chance;
     char                arg1[MAX_INPUT_LENGTH];
     char               *spell;
     int                 gold = 0;
     int                 mana = 0;

     one_argument ( argument, arg1 );

    /*
     * No NPC's can scribe
     */
     if ( IS_NPC ( ch ) )
          return;

     if ( ch->pcdata->pclass != class_lookup ( "mage" )
          && ch->pcdata->pclass != class_lookup ( "chaosmage" ) )
     {
          send_to_char ( "Only mages or chaosmages may scribe spells.\n\r", ch );
          return;
     }

     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Scribe what?\n\r", ch );
          return;
     }

     if ( ( sn = skill_lookup ( arg1 ) ) < 0
          || ( !IS_NPC ( ch ) && ch->level < skill_table[sn].skill_level[ch->pcdata->pclass] )
          || ( ch->pcdata->learned[sn] < 1 ) )
     {
          send_to_char ( "You don't know any spells of that name.\n\r", ch );
          return;
     }

     if ( skill_table[sn].spell_fun == spell_null )
     {
          send_to_char ( "That's not a spell!\n\r", ch );
          return;
     }

     gold = skill_table[sn].skill_level[ch->pcdata->pclass] * 10;
     mana = UMAX ( skill_table[sn].min_mana,
                   100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->pcdata->pclass] ) ) * 1.5;

     /* Verify character has necessary gold, mana, and components. */

     if ( ch->gold < gold )
     {
          send_to_char ( "You do not have enough gold to scribe that.\n\r", ch );
          return;
     }
     if ( ch->mana < mana )
     {
          send_to_char( "You do not have enough mana to scribe that.\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_carry ( ch, "vellum", NULL ) ) == NULL )
     {
          send_to_char ( "You need vellum to write on.\n\r", ch );
          return;
     }

     if ( str_cmp ( skill_table[sn].component, "" )
          && ( obj2 =  get_obj_carry ( ch, skill_table[sn].component, NULL ) ) == NULL )
     {
          form_to_char ( ch, "You need '%s' to scribe that spell.\n\r",
                skill_table[sn].component );
          return;
     }

     /* Ok, checks done, lets whip up a potion! */

     ch->gold -= gold;
     ch->mana -= mana;
     extract_obj ( obj );	/*vellum sheet */
     if ( obj2 != NULL )
          extract_obj ( obj2 );	/*extra component */

     chance = ch->level / 2 + ch->pcdata->learned[sn] / 2;
     if ( number_percent (  ) > chance )
     {
          send_to_char ( "Oops, you dripped some ink in the wrong place, scroll ruined.\n\r", ch );
          return;
     }

     obj = create_object ( ( get_obj_index ( OBJ_SCROLL ) ), 0 );
     obj->value[0] = ch->level;
     obj->value[1] = sn;
     obj->level = 1;
     obj->timer = UMAX ( 24, ( ch->level * 2 ) );
     free_string ( obj->name );
     free_string ( obj->short_descr );
     free_string ( obj->description );

     spell = skill_table[sn].name;

     SNP ( typebuf, "scroll %s", spell );
     obj->name = str_dup ( typebuf );
     SNP ( typebuf, "a scroll of %s", spell );
     obj->short_descr = str_dup ( typebuf );
     SNP ( typebuf, "A tightly rolled scroll of %s is lying here.", spell );
     obj->description = str_dup ( typebuf );

     /* Zeran - make waitstate *before* obj is put in room, so scribers don't
      * have their new creations stolen while they are stuck in a waitstate.
      */
     /* Lotherius - Umm... Z... the wait state doesn't pause the whole mud... this
      * changes nothing, the player is still waited after the obj is placed. We aren't
      * running multi-threaded here ya know.
      */
     if ( !IS_IMMORTAL ( ch ) )
          WAIT_STATE ( ch, PULSE_VIOLENCE / 2 );	/*lets not scribe at lightspeed */
     obj_to_room ( obj, ch->in_room );
     send_to_char ( "You painstakingly fill the scroll with mystical runes!\n\r", ch );
     act ( "$n writes a scroll with strange symbols on it.", ch, NULL, NULL, TO_ROOM );
     return;
}
