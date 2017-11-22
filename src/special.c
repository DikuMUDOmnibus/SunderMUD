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
#include "interp.h"

/* command procedures needed */
/* Got tired of declaring dofuns, so I included interp.h. booya. */

/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN ( spec_breath_any );
DECLARE_SPEC_FUN ( spec_breath_acid );
DECLARE_SPEC_FUN ( spec_breath_fire );
DECLARE_SPEC_FUN ( spec_breath_frost );
DECLARE_SPEC_FUN ( spec_breath_gas );
DECLARE_SPEC_FUN ( spec_breath_lightning );
DECLARE_SPEC_FUN ( spec_cast_adept );
DECLARE_SPEC_FUN ( spec_cast_cleric );
DECLARE_SPEC_FUN ( spec_cast_judge );
DECLARE_SPEC_FUN ( spec_cast_mage );
DECLARE_SPEC_FUN ( spec_cast_undead );
DECLARE_SPEC_FUN ( spec_executioner );
DECLARE_SPEC_FUN ( spec_fido );
DECLARE_SPEC_FUN ( spec_guard );
DECLARE_SPEC_FUN ( spec_janitor );
DECLARE_SPEC_FUN ( spec_poison );
DECLARE_SPEC_FUN ( spec_thief );
//DECLARE_SPEC_FUN ( spec_puff ); /* Removed because it was so damn buggy
DECLARE_SPEC_FUN ( spec_questmaster );	/* Vassago */
DECLARE_SPEC_FUN ( spec_crier );	/* Lotherius town crier */
//DECLARE_SPEC_FUN ( spec_metamob );

/*
 * Special Functions Table.     OLC
 */
const struct spec_type spec_table[] = 
{
    /*
     * Special function commands.
     */
    {"spec_breath_any", spec_breath_any},
    {"spec_breath_acid", spec_breath_acid},
    {"spec_breath_fire", spec_breath_fire},
    {"spec_breath_frost", spec_breath_frost},
    {"spec_breath_gas", spec_breath_gas},
    {"spec_breath_lightning", spec_breath_lightning},
    {"spec_cast_adept", spec_cast_adept},
    {"spec_cast_cleric", spec_cast_cleric},
    {"spec_cast_judge", spec_cast_judge},
    {"spec_cast_mage", spec_cast_mage},
    {"spec_cast_undead", spec_cast_undead},
    {"spec_executioner", spec_executioner},
    {"spec_fido", spec_fido},
    {"spec_guard", spec_guard},
    {"spec_janitor", spec_janitor},
    {"spec_poison", spec_poison},
    {"spec_thief", spec_thief},
    {"spec_questmaster", spec_questmaster},	/* Vassago */
    {"spec_crier", spec_crier},
//    {"spec_metamob", spec_metamob },

    /*
     * End of list.
     */
    {"", 0}
};


/*****************************************************************************
 Name:          spec_lookup
 Purpose:       Given a name, return the appropriate spec fun.
 Called by:     do_mset(act_wiz.c) load_specials,reset_area(db.c)
 ****************************************************************************/
SPEC_FUN           *spec_lookup ( const char *name )	/* OLC */
{
    int                 cmd;

    for ( cmd = 0; spec_table[cmd].spec_name[0] != '\0'; cmd++ )
	if ( !str_cmp ( name, spec_table[cmd].spec_name ) )
	    return spec_table[cmd].spec_fun;

    return 0;
}


/*
 * Core procedure for dragons.
 */
bool dragon ( CHAR_DATA * ch, char *spell_name )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    if ( ( sn = skill_lookup ( spell_name ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim );
    return TRUE;
}


/*****************************************************************************
 Name:          spec_string
 Purpose:       Given a function, return the appropriate name.
 Called by:     <???>
 ****************************************************************************/
char               *spec_string ( SPEC_FUN * fun )	/* OLC */
{
    int                 cmd;

    for ( cmd = 0; spec_table[cmd].spec_name[0] != '\0'; cmd++ )
	if ( fun == spec_table[cmd].spec_fun )
	    return spec_table[cmd].spec_name;

    return 0;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any ( CHAR_DATA * ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits ( 3 ) )
    {
    case 0:
         return spec_breath_fire ( ch );
    case 1:
    case 2:
         return spec_breath_lightning ( ch );
    case 3:
         return spec_breath_gas ( ch );
    case 4:
         return spec_breath_acid ( ch );
    case 5:
    case 6:
    case 7:
         return spec_breath_frost ( ch );
    }     
     return FALSE;
}



bool spec_breath_acid ( CHAR_DATA * ch )
{
    return dragon ( ch, "acid breath" );
}



bool spec_breath_fire ( CHAR_DATA * ch )
{
    return dragon ( ch, "fire breath" );
}


bool spec_questmaster ( CHAR_DATA * ch )
{
    if ( ch->fighting != NULL )
	return spec_cast_mage ( ch );
    return FALSE;
}

bool spec_breath_frost ( CHAR_DATA * ch )
{
    return dragon ( ch, "frost breath" );
}



bool spec_breath_gas ( CHAR_DATA * ch )
{
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup ( "gas breath" ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, NULL );
    return TRUE;
}



bool spec_breath_lightning ( CHAR_DATA * ch )
{
    return dragon ( ch, "lightning breath" );
}

bool spec_crier ( CHAR_DATA * ch )
{
     CHAR_DATA          *victim;
     CHAR_DATA          *v_next;
     struct crier_type  *tmp = NULL;
     char                outbuf[MAX_STRING_LENGTH];
     int                 i, count = 0;
     
     /* First off, let's do adept since all criers are adept */     
     spec_cast_adept ( ch );     
     /* Adjust this number for more or less cries */
     if ( number_range ( 1, 10 ) < 7 )
          return FALSE;     
     /* Get a count */
     for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
     {
          count++;
     }
     
     tmp = NULL;
     
     /* Now do the crying! */
     
     if ( !IS_AWAKE ( ch ) )
          return FALSE;
     
     if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
          !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
          return FALSE;

     for ( victim = ch->in_room->people; victim != NULL;
           victim = v_next )
     {
          v_next = victim->next_in_room;          
          /* Look for any target that can see the crier. One is good enough. */
          
          if ( victim != ch && can_see ( ch, victim ) &&
               number_bits ( 1 ) == 0 && !IS_NPC ( victim ) )
          {
               /* Select a random cry to shout */
               i = number_range ( 1, count );	/* Get # to stop on */
               count = 1;		/* Reset Count */
               
               for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
               {
                    if ( count == i )
                    {
                         /* Display a cry */
                         SNP ( outbuf, "{c$n cries out '{W%s{c'{w\r\n", tmp->text );
                         act ( outbuf, ch, NULL, NULL, TO_ROOM );
                         /* sends it to just this room */
                         /* In future, have newbie and area cries too. */
                         return TRUE;
                    } 
                    else
                    {
                         count++;
                    }
                    
               }			/* End of crier_list */
               
          }
          /* End of looking for a victim that can see. */
     }				/* No victims if code reaches here. */
     
     return FALSE;
}

bool spec_cast_adept ( CHAR_DATA * ch )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;

    if ( !IS_AWAKE ( ch ) )
	return FALSE;

    if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
	 !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see ( ch, victim ) &&
	     number_bits ( 1 ) == 0 && !IS_NPC ( victim ) &&
	     victim->level < 11 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    switch ( number_bits ( 4 ) )
    {
    case 0:
	act ( "$n utters the word 'abrazak'.", ch, NULL, NULL,
	      TO_ROOM );
	spell_armor ( skill_lookup ( "armor" ), ch->level, ch,
		      victim );
	return TRUE;

    case 1:
	act ( "$n utters the word 'fido'.", ch, NULL, NULL,
	      TO_ROOM );
	spell_bless ( skill_lookup ( "bless" ), ch->level, ch,
		      victim );
	return TRUE;

    case 2:
	act ( "$n utters the word 'judicandus noselacri'.", ch,
	      NULL, NULL, TO_ROOM );
	spell_cure_blindness ( skill_lookup ( "cure blindness" ),
			       ch->level, ch, victim );
	return TRUE;

    case 3:
	act ( "$n utters the word 'judicandus dies'.", ch, NULL,
	      NULL, TO_ROOM );
	spell_cure_light ( skill_lookup ( "cure light" ),
			   ch->level, ch, victim );
	return TRUE;

    case 4:
	act ( "$n utters the words 'judicandus sausabru'.", ch,
	      NULL, NULL, TO_ROOM );
	spell_cure_poison ( skill_lookup ( "cure poison" ),
			    ch->level, ch, victim );
	return TRUE;

    case 5:
	act ( "$n utters the words 'candusima'.", ch, NULL, NULL,
	      TO_ROOM );
	spell_refresh ( skill_lookup ( "refresh" ), ch->level, ch,
			victim );
	return TRUE;

    }

    return FALSE;
}



bool spec_cast_cleric ( CHAR_DATA * ch )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    char               *spell;
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
	 !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int                 min_level;
         
         switch ( number_bits ( 4 ) )
         {
         case 0:
              min_level = 0;
              spell = "blindness";
              break;
         case 1:
              min_level = 3;
              spell = "cause serious";
              break;
         case 2:
              min_level = 7;
              spell = "earthquake";
              break;
         case 3:
              min_level = 9;
              spell = "cause critical";
              break;
         case 4:
              min_level = 10;
              spell = "dispel evil";
              break;
         case 5:
              min_level = 12;
              spell = "curse";
              break;
         case 6:
              min_level = 12;
              spell = "change sex";
              break;
         case 7:
              min_level = 13;
              spell = "flamestrike";
              break;
         case 8:
         case 9:
         case 10:
              min_level = 15;
              spell = "harm";
              break;
         case 11:
              min_level = 15;
              spell = "plague";
              break;
         case 12:
         case 13:
         case 14:
              min_level = 34;
              spell = ( IS_EVIL ( ch ) ? "demonfire" : "exorcism" );
              break;
         default:
              min_level = 16;
              spell = "dispel magic";
              break;
         }
         if ( ch->hit < ( ch->max_hit / 5 ) && ch->level > 13 )
         {
              spell = "cure critical";
              victim = ch;
         }
         if ( ch->level >= min_level )
              break;
    }
     
     if ( ( sn = skill_lookup ( spell ) ) < 0 )
          return FALSE;
     if ( ch->mana < skill_table[sn].min_mana )
          return FALSE;
     ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim );
     ch->mana -= skill_table[sn].min_mana;
     return TRUE;
}

bool spec_cast_judge ( CHAR_DATA * ch )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    char               *spell;
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
	 !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    spell = "high explosive";
    if ( ( sn = skill_lookup ( spell ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_mage ( CHAR_DATA * ch )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    char               *spell;
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
	 !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int                 min_level;

	switch ( number_bits ( 4 ) )
	{
	case 0:
	    min_level = 0;
	    spell = "blindness";
	    break;
	case 1:
	    min_level = 3;
	    spell = "chill touch";
	    break;
	case 2:
	    min_level = 7;
	    spell = "weaken";
	    break;
	case 3:
	    min_level = 8;
	    spell = "teleport";
	    break;
	case 4:
	    min_level = 11;
	    spell = "colour spray";
	    break;
	case 5:
	    min_level = 12;
	    spell = "change sex";
	    break;
	case 6:
	    min_level = 13;
	    spell = "energy drain";
	    break;
	case 7:
	case 8:
	case 9:
	    min_level = 15;
	    spell = "fireball";
	    break;
	case 10:
	    min_level = 20;
	    spell = "plague";
	    break;
	default:
	    min_level = 20;
	    spell = "acid blast";
	    break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup ( spell ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_undead ( CHAR_DATA * ch )
{
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    char               *spell;
    int                 sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( is_affected ( ch, skill_lookup ( "mute" ) ) &&
	 !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
	return FALSE;

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == ch && number_bits ( 2 ) == 0 )
	    break;
    }

    if ( victim == NULL )
	return FALSE;

    for ( ;; )
    {
	int                 min_level;

	switch ( number_bits ( 4 ) )
	{
	case 0:
	    min_level = 0;
	    spell = "curse";
	    break;
	case 1:
	    min_level = 3;
	    spell = "weaken";
	    break;
	case 2:
	    min_level = 6;
	    spell = "chill touch";
	    break;
	case 3:
	    min_level = 9;
	    spell = "blindness";
	    break;
	case 4:
	    min_level = 12;
	    spell = "poison";
	    break;
	case 5:
	    min_level = 15;
	    spell = "energy drain";
	    break;
	case 6:
	    min_level = 18;
	    spell = "harm";
	    break;
	case 7:
	    min_level = 21;
	    spell = "teleport";
	    break;
	case 8:
	    min_level = 20;
	    spell = "plague";
	    break;
	default:
	    min_level = 18;
	    spell = "harm";
	    break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup ( spell ) ) < 0 )
	return FALSE;
    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, victim );
    return TRUE;
}


bool spec_executioner ( CHAR_DATA * ch )
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    char               *crime;

    if ( !IS_AWAKE ( ch ) || ch->fighting != NULL )
	return FALSE;

    crime = "";
    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC ( victim ) &&
	     IS_SET ( victim->act, PLR_KILLER ) )
	{
	    crime = "KILLER";
	    break;
	}

	if ( !IS_NPC ( victim ) &&
	     IS_SET ( victim->act, PLR_THIEF ) )
	{
	    crime = "THIEF";
	    break;
	}
    }

    if ( victim == NULL )
	return FALSE;

	 /* Why have one default mob_cityuard vnum when we have different cityguards? */
    SNP ( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!", victim->name, crime );
    do_yell ( ch, buf );
    multi_hit ( ch, victim, TYPE_UNDEFINED );
    char_to_room ( create_mobile ( get_mob_index ( ch->pIndexData->vnum ) ), ch->in_room );
    char_to_room ( create_mobile ( get_mob_index ( ch->pIndexData->vnum ) ), ch->in_room );
    return TRUE;
}

bool spec_fido ( CHAR_DATA * ch )
{
    OBJ_DATA           *corpse;
    OBJ_DATA           *c_next;
    OBJ_DATA           *obj;
    OBJ_DATA           *obj_next;

    if ( !IS_AWAKE ( ch ) )
	return FALSE;

    for ( corpse = ch->in_room->contents; corpse != NULL;
	  corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

	act ( "$n savagely devours a corpse.", ch, NULL, NULL,
	      TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj ( obj );
	    obj_to_room ( obj, ch->in_room );
	}
	extract_obj ( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard ( CHAR_DATA * ch )
{
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA          *victim;
    CHAR_DATA          *v_next;
    CHAR_DATA          *ech;
    char               *crime;
    int                 max_evil;

    if ( !IS_AWAKE ( ch ) || ch->fighting != NULL )
	return FALSE;

    max_evil = 300;
    ech = NULL;
    crime = "";

    for ( victim = ch->in_room->people; victim != NULL;
	  victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC ( victim ) &&
	     IS_SET ( victim->act, PLR_KILLER ) )
	{
	    crime = "KILLER";
	    break;
	}

	if ( !IS_NPC ( victim ) &&
	     IS_SET ( victim->act, PLR_THIEF ) )
	{
	    crime = "THIEF";
	    break;
	}

	if ( victim->fighting != NULL
	     && victim->fighting != ch
	     && victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech = victim;
	}
    }

    if ( victim != NULL )
    {
	SNP ( buf,
		  "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
		  victim->name, crime );
	do_yell ( ch, buf );
	multi_hit ( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech != NULL )
    {
	act ( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	      ch, NULL, NULL, TO_ROOM );
	multi_hit ( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}



bool spec_janitor ( CHAR_DATA * ch )
{
     OBJ_DATA           *trash;
     OBJ_DATA           *trash_next;
     
     if ( !IS_AWAKE ( ch ) )
          return FALSE;
     
     for ( trash = ch->in_room->contents; trash != NULL;
           trash = trash_next )
     {
          trash_next = trash->next_content;
          if ( !IS_SET ( trash->wear_flags, ITEM_TAKE ) || trash->owner ||
               !can_loot ( ch, trash ) )
               continue;
          if ( trash->item_type == ITEM_DRINK_CON
               || trash->item_type == ITEM_TRASH
               || trash->cost < 10 )
          {
               act ( "$n picks up some trash.", ch, NULL, NULL,
                     TO_ROOM );
               obj_from_room ( trash );
               obj_to_char ( trash, ch );
               return TRUE;
          }
     }     
     return FALSE;
}

bool spec_poison ( CHAR_DATA * ch )
{
     CHAR_DATA          *victim;
     
     if ( ch->position != POS_FIGHTING
          || ( victim = ch->fighting ) == NULL
          || number_percent (  ) > 2 * ch->level )
          return FALSE;
     
     act ( "You bite $N!", ch, NULL, victim, TO_CHAR );
     act ( "$n bites $N!", ch, NULL, victim, TO_NOTVICT );
     act ( "$n bites you!", ch, NULL, victim, TO_VICT );
     spell_poison ( gsn_poison, ch->level, ch, victim );
     return TRUE;
}

// Meta Mob includes just about everything we can think of for a mob to do,
// when it is pretending to be a player.

// Metamobs disabled till more work can be done on them.

/*
bool spec_metamob ( CHAR_DATA *ch )
{
     CHAR_DATA *victim;
     EXIT_DATA *pexit;
     ROOM_INDEX_DATA *location;
     bool rv = FALSE;
     int door, sn;

     switch ( ch->position )
     {
     case POS_DEAD:
     case POS_MORTAL:
     case POS_INCAP:
     case POS_STUNNED:
          break;
     case POS_SLEEPING:
     case POS_RESTING:
     case POS_SITTING:
          if ( ch->hit >= ch->max_hit )
          {
               do_stand ( ch, "" );
               if ( chance ( 25 ) && IS_SET ( ch->act, ACT_WARRIOR ) )
                    act ( "$n checks $s armor carefully.", ch, NULL, NULL, TO_ROOM );
               else if ( chance ( 25 ) )
                    act ( "$n yawns.", ch, NULL, NULL, TO_ROOM );
               rv = TRUE;
          }
          break;
     case POS_STANDING:
          // Find something to fight or move.
          if ( chance ( 75 ) && ( ch->hit > ( ch->max_hit / 2 ) ) )
          {
               for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room )
               {
                    if ( is_safe ( ch, victim, TRUE ) )
                         continue;
                    if ( !IS_NPC ( victim ) )
                    {
                         if ( IS_SET ( victim->act, PLR_THIEF ) )
                         {
                              do_say ( ch, "You dirty rotten THIEF!\n\r" );
                              multi_hit ( ch, victim, TYPE_UNDEFINED );
                              rv = TRUE;
                              break;
                              
                         }
                         else if ( IS_SET ( victim->act, PLR_KILLER ) )
                         {
                              int domsg;
                              domsg = ( number_range ( 1, 3 ) );
                              switch ( domsg )
                              {
                              case 1:
                                   do_say ( ch, "Hey! I think you killed my cousin!\n\r" );
                                   break;
                              case 2:
                                   do_say ( ch, "Die before my blade!\n\r" );
                                   break;
                              case 3:
                                   do_say ( ch, "You know, I really don't like your type.\n\r" );
                                   break;
                              }                              
                              multi_hit ( ch, victim, TYPE_UNDEFINED );
                              rv = TRUE;
                              break;
                         }
                    } // Else NPC
                    else
                    {
                         // We're going to make the metamob more selective until this spec function is more
                         // intelligent :)
                         if ( victim->level > ( ch->level +1 ) || victim->level < ( ch->level-5 ) )
                              continue;
                         // Very simple alignment routines here, could get more complicated if we want. We don't.
                         if ( ch->alignment < -250 && victim->alignment < -250 )
                              continue;
                         if ( ch->alignment > 250 && victim->alignment > 250 )
                              continue;
                         do_say ( ch, "Bonzai!!!" );
                         rv = TRUE;
                         multi_hit ( ch, victim, TYPE_UNDEFINED );
                         break;
                    }
               } // End of FOR loop
               if ( rv )
                    break;
          }
          if ( ch->hit < (ch->max_hit / 2 ) ) // Even mobs have to rest....
          {
               do_sleep ( ch, "" );
               rv = TRUE;
               break;
          }
          // Okay if we got here, we didn't start a fight. Look for an exit.
          // MetaMobs don't get blocked by NO_MOB rooms but other rules apply.
          if ( !IS_SET ( ch->act, ACT_SENTINEL )
               && ( door = number_bits ( 5 ) ) <= 5
               && ( pexit = ch->in_room->exit[door] ) != NULL
               && pexit->u1.to_room != NULL &&
               ( !IS_SET ( ch->act, ACT_STAY_AREA ) ||
                 pexit->u1.to_room->area == ch->in_room->area ) )
          {
               do_unlock ( ch, dir_name[door] ); // Unlock and Open
               do_open   ( ch, dir_name[door] ); // May not have they and may not even be a door, just to make sure.
               move_char ( ch, door, FALSE );
               rv = TRUE;
               break;
          }
          // If the char wasn't able to find a door to try, then try to recall sometimes.
          if ( chance ( 1 ) && !IS_AFFECTED ( ch, AFF_CURSE ) )
          {
               location = get_room_index ( ROOM_VNUM_TEMPLE );
               if ( ch->in_room == location )
                    break;
               act ( "$n prays for transportation!", ch, 0, 0, TO_ROOM );
               act ( "$n disappears.", ch, NULL, NULL, TO_ROOM );
               char_from_room ( ch );
               char_to_room ( ch, location );
               act ( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
               rv = TRUE;
               break;
          }
          break;
     case POS_FIGHTING:
          if ( ch->hit < ( ch->max_hit / 5 ) )
          {
               do_flee ( ch, "" );
               rv = TRUE;
          }
          else if ( IS_SET ( ch->act, ACT_WARRIOR ) )
          {
               int wdo;
               
               if ( chance ( 20 ) )  // 20% of rounds won't do any action.
                    break;
               wdo = number_range ( 1, 5 );
               switch (wdo)
               {
               case 1:      
                    if ( ( sn = skill_lookup ( "disarm" ) ) > 0 )
                         do_disarm ( ch, "" );
                    rv = TRUE;
                    break;
               case 2:
                    if ( ( sn = skill_lookup ( "bash" ) ) > 0 )
                         do_bash ( ch, "" );
                    rv = TRUE;
                    break;
               case 3:
               case 4:
                    if ( ( sn = skill_lookup ( "dirt kicking" ) ) > 0 )
                         do_dirt ( ch, "" );
                    rv = TRUE;
                    break;
               case 5:
                    if ( ( sn = skill_lookup ( "kick" ) ) > 0 )
                         do_kick ( ch, "" );
                    rv = TRUE;                    
               }
               if ( !rv )   // Whatever we tried to do didn't have a valid skill, this is our failsafe
               {
                    if ( ( sn = skill_lookup ( "kick" ) ) > 0 )
                         do_kick ( ch, "" );
               }

          }
          else if ( IS_SET ( ch->act, ACT_CLERIC ) )
          {
               rv = spec_cast_cleric ( ch );
               // Do some other stuff here if spec_cast_cleric returned FALSE
          }
          else if ( IS_SET ( ch->act, ACT_MAGE ) )
          {
               rv = spec_cast_mage ( ch );
               // Do some other stuff here if spec_cast_mage returned FALSE
          }
          else if ( IS_SET ( ch->act, ACT_THIEF ) )
               tail_chain ();         // Just here as a null function till I write the thief function.
          break;
     default:
          bugf ("Invalid position on spec_metamob." );
          break;
     }
     return rv;
     
}
*/

bool spec_thief ( CHAR_DATA * ch )
{
     CHAR_DATA          *victim;
     CHAR_DATA          *v_next;
     long                gold;
     
     if ( ch->position != POS_STANDING )
          return FALSE;
     
     for ( victim = ch->in_room->people; victim != NULL;
           victim = v_next )
     {
          v_next = victim->next_in_room;
          
          if ( IS_NPC ( victim )
               || victim->level >= LEVEL_IMMORTAL
               || number_bits ( 5 ) != 0 || !can_see ( ch, victim ) )
               continue;
          
          if ( IS_AWAKE ( victim ) &&
               number_range ( 0, ch->level ) == 0 )
          {
               act ( "You discover $n's hands in your wallet!",
                     ch, NULL, victim, TO_VICT );
               act ( "$N discovers $n's hands in $S wallet!",
                     ch, NULL, victim, TO_NOTVICT );
               return TRUE;
          } else
          {
               gold =
                    victim->gold * UMIN ( number_range ( 1, 20 ),
                                          ch->level ) / 100;
               gold = UMIN ( gold, ch->level * ch->level * 20 );
               ch->gold += gold;
               victim->gold -= gold;
               return TRUE;
          }
     }
     
    return FALSE;
}

/* Healer code written for Merc 2.0 muds by Alander 
   direct questions or comments to rtaylor@cie-2.uoregon.edu
   any use of this code must include this header */

void do_heal ( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA          *mob;
    char                arg[MAX_INPUT_LENGTH];
    int                 cost, sn;
    SPELL_FUN          *spell;
    char               *words;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC ( mob ) && IS_SET ( mob->act, ACT_IS_HEALER ) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char ( "You can't do that here.\n\r", ch );
	return;
    }

    one_argument ( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* display price list */
	act ( "$N says 'I offer the following spells:'", ch, NULL,
	      mob, TO_CHAR );
	send_to_char ( "  light:   cure light wounds          30 gold\n\r",   ch );
	send_to_char ( "  serious: cure serious wounds        50 gold\n\r",   ch );
	send_to_char ( "  critic:  cure critical wounds      100 gold\n\r",   ch );
	send_to_char ( "  heal:    healing spell            1000 gold\n\r",   ch );
	send_to_char ( "  blind:   cure blindness            300 gold\n\r",   ch );
	send_to_char ( "  disease: cure disease             1000 gold\n\r",   ch );
	send_to_char ( "  poison:  cure poison               300 gold\n\r", ch );
        send_to_char ( "  uncurse: remove curse             2000 gold\n\r", ch );
	send_to_char ( "  refresh: restore movement          500 gold\n\r", ch );
	send_to_char ( "  mana:    restore mana             1000 gold\n\r", ch );
	send_to_char ( "  {gfull{x:    all mana, hp, movement  50000 gold\n\r", ch );
	send_to_char ( " Type heal <type> to be healed.\n\r", ch );
	return;
    }

    switch ( arg[0] )
    {
    case 'l':
	spell = spell_cure_light;
	sn = skill_lookup ( "cure light" );
	words = "judicandus dies";
	cost = 30;
	break;

    case 's':
	spell = spell_cure_serious;
	sn = skill_lookup ( "cure serious" );
	words = "judicandus gzfuajg";
	cost = 50;
	break;

    case 'c':
	spell = spell_cure_critical;
	sn = skill_lookup ( "cure critical" );
	words = "judicandus qfuhuqar";
	cost = 100;
	break;

    case 'h':
	spell = spell_heal;
	sn = skill_lookup ( "heal" );
	words = "pzar";
	cost = 1000;
	break;

    case 'b':
	spell = spell_cure_blindness;
	sn = skill_lookup ( "cure blindness" );
	words = "judicandus noselacri";
	cost = 300;
	break;

    case 'd':
	spell = spell_cure_disease;
	sn = skill_lookup ( "cure disease" );
	words = "judicandus eugzagz";
	cost = 1000;
	break;

    case 'p':
	spell = spell_cure_poison;
	sn = skill_lookup ( "cure poison" );
	words = "judicandus sausabru";
	cost = 300;
	break;

    case 'u':
	spell = spell_remove_curse;
	sn = skill_lookup ( "remove curse" );
	words = "candussido judifgz";
	cost = 2000;
	break;

    case 'r':
	spell = spell_refresh;
	sn = skill_lookup ( "refresh" );
	words = "candusima";
	cost = 500;
	break;

    case 'm':
	spell = NULL;
	sn = -1;
	words = "energizer";
	cost = 1000;
	break;

    case 'f':
	spell = NULL;
	sn = -1;
	words =
	    "{rSim{gsal{ybim{wbam{cba {bsal{ga{ydu {Gsal{Ya{Rdim{m!{x";
	cost = 50000;
	break;

    default:
	act ( "$N says 'Type 'heal' for a list of spells.'",
	      ch, NULL, mob, TO_CHAR );
	return;
    }

    if ( cost > ch->gold )
    {
	act ( "$N says 'You do not have enough gold for my services.'", ch, NULL, mob, TO_CHAR );
	return;
    }

    WAIT_STATE ( ch, PULSE_VIOLENCE );

    ch->gold -= cost;
    mob->gold += cost;
    act ( "$n utters the words '$T'.", mob, NULL, words, TO_ROOM );

    if ( ( spell == NULL ) && ( arg[0] == 'm' ) )	/* restore mana trap...kinda hackish */
    {
	ch->mana += dice ( 100, 2 ) + mob->level / 4;
	ch->mana = UMIN ( ch->mana, ch->max_mana );
	send_to_char ( "A warm glow passes through you.\n\r", ch );
	return;
    } else if ( ( spell == NULL ) && ( arg[0] == 'f' ) )
    {
	ch->mana = ch->max_mana;
	ch->hit = ch->max_hit;
	ch->move = ch->max_move;
	send_to_char
	    ( "The {gpower{x of the {rg{go{yd{cs{x flows into you!\n\r",
	      ch );
	return;
    }

    if ( sn == -1 )
	return;

    spell ( sn, mob->level, mob, ch );
}
