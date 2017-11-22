/**************************************************************************r
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

/* Have removed several unused functions from this file */

#include "everything.h"



/* command procedures needed */
DECLARE_DO_FUN ( do_return );

AFFECT_DATA        *affect_free;

/*
 * Local functions.
 */
void affect_modify args ( ( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd ) );

/* Counts Users on an Object. */
int count_users ( OBJ_DATA * obj )
{
     CHAR_DATA          *fch;
     int                 count = 0;

     if ( obj->in_room == NULL )
          return 0;

     for ( fch = obj->in_room->people; fch != NULL;
           fch = fch->next_in_room )
          if ( fch->on == obj )
               count++;

     return count;
}

/* returns material number */
int material_lookup ( const char *name )
{
     int                 counter;
     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( !str_cmp ( material_table[counter].name, name ) )
               return material_table[counter].type;
     }
     return 0;
}

/* returns material name -- ROM OLC temp patch -- doesn't look very temp to me */
char *material_name ( sh_int num )
{
     int                 counter;
     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( material_table[counter].type == num )
               return material_table[counter].name;
     }
     return "unknown";
}

/* returns material vulnerability flag */
long material_vuln ( sh_int num )
{
     int                 counter;
     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( material_table[counter].type == num )
               return material_table[counter].vuln_flag;
     }
     return 0;
}

/* returns material durability flag */
long material_dura ( sh_int num )
{
     int                 counter;

     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( material_table[counter].type == num )
               return material_table[counter].durable;
     }
     return 0;
}

/* returns material repair difficulty */
long material_repa ( sh_int num )
{
     int                 counter;

     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( material_table[counter].type == num )
               return material_table[counter].difficult;
     }
     return 0;
}

/* Checks a flag from material flags. */
bool is_material ( sh_int num, long material_flag )
{
     int counter;
     for ( counter = 0; counter <= MAX_MATERIAL; counter++ )
     {
          if ( material_table[counter].type == num )
          {
               if IS_SET ( material_table[counter].flags, material_flag )
                    return TRUE;
               else
                    return FALSE;
          }
     }
     return FALSE;
}

/* returns race number */
int race_lookup ( const char *name )
{
     int                 race;

     for ( race = 0; race_table[race].name != NULL; race++ )
     {
          if ( LOWER ( name[0] ) == LOWER ( race_table[race].name[0] ) &&
               !str_prefix ( name, race_table[race].name ) )
               return race;
     }
     return 0;
}

/* returns race number for race's PCDATA */
int pcrace_lookup ( const char *name )
{
     int	pcrace;

     for ( pcrace = 0; pcrace < MAX_PCRACE; pcrace++ )
     {
          if ( LOWER ( name[0] ) == LOWER ( pc_race_table[pcrace].name[0] ) && !str_prefix ( name, pc_race_table[pcrace].name ) )
               return pcrace;
     }

     return 0;
}

/* returns class number */
int class_lookup ( const char *name )
{
     int                 class;

     for ( class = 0; class < MAX_CLASS; class++ )
     {
          if ( LOWER ( name[0] ) ==
               LOWER ( class_table[class].name[0] ) &&
               !str_prefix ( name, class_table[class].name ) )
               return class;
     }

     return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune ( CHAR_DATA * ch, int dam_type )
{
     int                 immune;
     int                 bit;

     immune = IS_NORMAL;

     if ( dam_type == DAM_NONE )
          return immune;

     if ( dam_type <= 3 )
     {
          if ( IS_SET ( ch->imm_flags, IMM_WEAPON ) )
               immune = IS_IMMUNE;
          else if ( IS_SET ( ch->res_flags, RES_WEAPON ) )
               immune = IS_RESISTANT;
          else if ( IS_SET ( ch->vuln_flags, VULN_WEAPON ) )
               immune = IS_VULNERABLE;
     }
     else			/* magical attack */
     {
          if ( IS_SET ( ch->imm_flags, IMM_MAGIC ) )
               immune = IS_IMMUNE;
          else if ( IS_SET ( ch->res_flags, RES_MAGIC ) )
               immune = IS_RESISTANT;
          else if ( IS_SET ( ch->vuln_flags, VULN_MAGIC ) )
               immune = IS_VULNERABLE;
     }

     /* set bits to check -- VULN etc. must ALL be the same or this will fail */
     switch ( dam_type )
     {
     case ( DAM_BASH ):
          bit = IMM_BASH;
          break;
     case ( DAM_PIERCE ):
          bit = IMM_PIERCE;
          break;
     case ( DAM_SLASH ):
          bit = IMM_SLASH;
          break;
     case ( DAM_FIRE ):
          bit = IMM_FIRE;
          break;
     case ( DAM_COLD ):
          bit = IMM_COLD;
          break;
     case ( DAM_LIGHTNING ):
          bit = IMM_LIGHTNING;
          break;
     case ( DAM_ACID ):
          bit = IMM_ACID;
          break;
     case ( DAM_POISON ):
          bit = IMM_POISON;
          break;
     case ( DAM_NEGATIVE ):
          bit = IMM_NEGATIVE;
          break;
     case ( DAM_HOLY ):
          bit = IMM_HOLY;
          break;
     case ( DAM_ENERGY ):
          bit = IMM_ENERGY;
          break;
     case ( DAM_MENTAL ):
          bit = IMM_MENTAL;
          break;
     case ( DAM_DISEASE ):
          bit = IMM_DISEASE;
          break;
     case ( DAM_DROWNING ):
          bit = IMM_DROWNING;
          break;
     case ( DAM_LIGHT ):
          bit = IMM_LIGHT;
          break;
     default:
          return immune;
     }

     if ( IS_SET ( ch->imm_flags, bit ) )
          immune = IS_IMMUNE;
     else if ( IS_SET ( ch->res_flags, bit ) )
          immune = IS_RESISTANT;
     else if ( IS_SET ( ch->vuln_flags, bit ) )
          immune = IS_VULNERABLE;

     return immune;
}

/*
 * See if a string is one of the names of an object.
 */

bool is_full_name ( const char *str, char *namelist )
{
     char                name[MAX_INPUT_LENGTH];

     for ( ;; )
     {
          namelist = one_argument ( namelist, name );
          if ( name[0] == '\0' )
               return FALSE;
          if ( !str_cmp ( str, name ) )
               return TRUE;
     }
}

/* for returning skill information */
int get_skill ( CHAR_DATA * ch, int sn )
{
     int                 skill;

     if ( sn == -1 )		/* shorthand for level based skills */
     {
          skill = ch->level * 5 / 2;
     }

     else if ( sn < -1 || sn > MAX_SKILL )
     {
          bugf ( "Bad sn %d in get_skill.", sn );
          skill = 0;
     }

     else if ( !IS_NPC ( ch ) )
     {
          if ( ch->level < skill_table[sn].skill_level[ch->pcdata->pclass] )
               skill = 0;
          else
               skill = ch->pcdata->learned[sn];
     }

     else			/* mobiles */
     {
          int pclass, sclass, slev;

          if (IS_SET(ch->act, ACT_CLERIC) )
          {
               pclass = CLASS_AVENGER;
               sclass = CLASS_DEFILER;
          }
          else if (IS_SET(ch->act, ACT_MAGE) )
          {
               pclass = CLASS_MAGE;
               sclass = CLASS_CHAOSMAGE;
          }
          else if (IS_SET(ch->act, ACT_THIEF) )
          {
               pclass = CLASS_THIEF;
               sclass = CLASS_THIEF;
          }
          else
          {
               pclass = CLASS_WARRIOR;
               sclass = CLASS_MONK;
          }

		/* Take the average of the 2 classes */

          slev = (skill_table[sn].skill_level[pclass] + skill_table[sn].skill_level[sclass]) /2;

          if (ch->level < slev )
               skill = 0;
          else
               skill = 35 + ch->level - slev;

     }

     if ( IS_AFFECTED ( ch, AFF_BERSERK ) )
          skill -= ch->level / 2;

     return URANGE ( 0, skill, 100 );
}

/* for returning weapon information */
int get_weapon_sn ( CHAR_DATA * ch, bool dual )
{
     OBJ_DATA           *wield;
     int                 sn;

     if ( !dual )
          wield = get_eq_char ( ch, WEAR_WIELD );
     else
          wield = get_eq_char ( ch, WEAR_WIELD2 );

     if ( wield == NULL || wield->item_type != ITEM_WEAPON )
          sn = gsn_hand_to_hand;
     else
          switch ( wield->value[0] )
          {
          default:
               sn = -1;
               break;
          case ( WEAPON_SWORD ):
               sn = gsn_sword;
               break;
          case ( WEAPON_DAGGER ):
               sn = gsn_dagger;
               break;
          case ( WEAPON_SPEAR ):
               sn = gsn_spear;
               break;
          case ( WEAPON_MACE ):
               sn = gsn_mace;
               break;
          case ( WEAPON_AXE ):
               sn = gsn_axe;
               break;
          case ( WEAPON_FLAIL ):
               sn = gsn_flail;
               break;
          case ( WEAPON_WHIP ):
               sn = gsn_whip;
               break;
          case ( WEAPON_POLEARM ):
               sn = gsn_polearm;
               break;
          }
     return sn;
}

int get_weapon_skill ( CHAR_DATA * ch, int sn )
{
     int                 skill;

    /* -1 is exotic */
     if ( IS_NPC ( ch ) )
     {
          if ( sn == -1 )
               skill = 3 * ch->level;
          else if ( sn == gsn_hand_to_hand )
               skill = 40 + 2 * ch->level;
          else
          {
               int pclass, sclass, slev;

               if (IS_SET(ch->act, ACT_CLERIC) )
               {
                    pclass = CLASS_AVENGER;
                    sclass = CLASS_DEFILER;
               }
               else if (IS_SET(ch->act, ACT_MAGE) )
               {
                    pclass = CLASS_MAGE;
                    sclass = CLASS_CHAOSMAGE;
               }
               else if (IS_SET(ch->act, ACT_THIEF) )
               {
                    pclass = CLASS_THIEF;
                    sclass = CLASS_THIEF;
               }
               else
               {
                    pclass = CLASS_WARRIOR;
                    sclass = CLASS_MONK;
               }

                /* Take the average of the 2 classes */

               slev = (skill_table[sn].skill_level[pclass] + skill_table[sn].skill_level[sclass]) /2;

/* Hack to avoid bad area design. All mobs given 40% in their weapon skills */
/* This is due to mobs that have weapons they shouldn't, so they don't miss continuously */

               if (ch->level >= slev )
                    skill = 40 + ch->level - slev;
               else
                    skill = 40;
          }

     }
     else
     {
          if ( sn == -1 )
               skill = 3 * ch->level;

          else
               skill = ch->pcdata->learned[sn];
     }

     return URANGE ( 0, skill, 100 );
}

/* used to de-screw characters */
/* Nice way of putting it.     */
void reset_char ( CHAR_DATA * ch )
{
     int                 loc, mod, stat;
     OBJ_DATA           *obj;
     AFFECT_DATA        *af;

     if ( IS_NPC ( ch ) )
          return;

     if ( ch->pcdata->perm_hit == 0
          || ch->pcdata->perm_mana == 0
          || ch->pcdata->perm_move == 0
          || ch->pcdata->last_level == 0 )
     {
	/* do a FULL reset */
          for ( loc = 0; loc < MAX_WEAR; loc++ )
          {
               obj = get_eq_char ( ch, loc );
               if ( obj == NULL )
                    continue;
               if ( !obj->enchanted )
                    for ( af = obj->pIndexData->affected; af != NULL;
                          af = af->next )
                    {
                         mod = af->modifier;
                         switch ( af->location )
                         {
                         case APPLY_SEX:
                              ch->sex -= mod;
                              if ( ch->sex < 0 || ch->sex > 2 )
                                   ch->sex = IS_NPC ( ch ) ?
                                   0 : ch->pcdata->true_sex;
                              break;
                         case APPLY_MANA:
                              ch->max_mana -= mod;
                              break;
                         case APPLY_HIT:
                              ch->max_hit -= mod;
                              break;
                         case APPLY_MOVE:
                              ch->max_move -= mod;
                              break;
                         }
                    }

               for ( af = obj->affected; af != NULL; af = af->next )
               {
                    mod = af->modifier;
                    switch ( af->location )
                    {
                    case APPLY_SEX:
                         ch->sex -= mod;
                         break;
                    case APPLY_MANA:
                         ch->max_mana -= mod;
                         break;
                    case APPLY_HIT:
                         ch->max_hit -= mod;
                         break;
                    case APPLY_MOVE:
                         ch->max_move -= mod;
                         break;
                    }
               }
          }
	/* now reset the permanent stats */
          ch->pcdata->perm_hit = ch->max_hit;
          ch->pcdata->perm_mana = ch->max_mana;
          ch->pcdata->perm_move = ch->max_move;
          ch->pcdata->last_level = ch->played / 3600;
          if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
          {
               if ( ch->sex > 0 && ch->sex < 3 )
                    ch->pcdata->true_sex = ch->sex;
               else
                    ch->pcdata->true_sex = 0;
          }

     }

    /* now restore the character to his/her true condition */
     for ( stat = 0; stat < MAX_STATS; stat++ )
          ch->mod_stat[stat] = 0;

     if ( ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2 )
          ch->pcdata->true_sex = 0;
     ch->sex = ch->pcdata->true_sex;
     ch->max_hit = ch->pcdata->perm_hit;
     ch->max_mana = ch->pcdata->perm_mana;
     ch->max_move = ch->pcdata->perm_move;

     //     for ( i = 0; i < 4; i++ )
     ch->armor = 0;

     ch->hitroll = 0;
     ch->damroll = 0;
     ch->saving_throw = 0;

    /* now start adding back the effects */
     for ( loc = 0; loc < MAX_WEAR; loc++ )
     {
          obj = get_eq_char ( ch, loc );
          if ( obj == NULL )
               continue;

          if ( !obj->enchanted )
               for ( af = obj->pIndexData->affected; af != NULL;
                     af = af->next )
               {
                    mod = af->modifier;
                    switch ( af->location )
                    {
                    case APPLY_STR:
                         ch->mod_stat[STAT_STR] += mod;
                         break;
                    case APPLY_DEX:
                         ch->mod_stat[STAT_DEX] += mod;
                         break;
                    case APPLY_INT:
                         ch->mod_stat[STAT_INT] += mod;
                         break;
                    case APPLY_WIS:
                         ch->mod_stat[STAT_WIS] += mod;
                         break;
                    case APPLY_CON:
                         ch->mod_stat[STAT_CON] += mod;
                         break;
                    case APPLY_SEX:
                         ch->sex += mod;
                         break;
                    case APPLY_MANA:
                         ch->max_mana += mod;
                         break;
                    case APPLY_HIT:
                         ch->max_hit += mod;
                         break;
                    case APPLY_MOVE:
                         ch->max_move += mod;
                         break;
                    case APPLY_AC:	// Magical AC
                         ch->armor += mod;
                         break;
                    case APPLY_HITROLL:
                         ch->hitroll += mod;
                         break;
                    case APPLY_DAMROLL:
                         ch->damroll += mod;
                         break;
                    case APPLY_SAVING_PARA:
                         ch->saving_throw += mod;
                         break;
                    case APPLY_SAVING_ROD:
                         ch->saving_throw += mod;
                         break;
                    case APPLY_SAVING_PETRI:
                         ch->saving_throw += mod;
                         break;
                    case APPLY_SAVING_BREATH:
                         ch->saving_throw += mod;
                         break;
                    case APPLY_SAVING_SPELL:
                         ch->saving_throw += mod;
                         break;
                    case APPLY_ENCUMBRANCE:
                         ch->encumbrance += mod;
                         break;
                    }
               }

          for ( af = obj->affected; af != NULL; af = af->next )
          {
               mod = af->modifier;
               switch ( af->location )
               {
               case APPLY_STR:
                    ch->mod_stat[STAT_STR] += mod;
                    break;
               case APPLY_DEX:
                    ch->mod_stat[STAT_DEX] += mod;
                    break;
               case APPLY_INT:
                    ch->mod_stat[STAT_INT] += mod;
                    break;
               case APPLY_WIS:
                    ch->mod_stat[STAT_WIS] += mod;
                    break;
               case APPLY_CON:
                    ch->mod_stat[STAT_CON] += mod;
                    break;
               case APPLY_SEX:
                    ch->sex += mod;
                    break;
               case APPLY_MANA:
                    ch->max_mana += mod;
                    break;
               case APPLY_HIT:
                    ch->max_hit += mod;
                    break;
               case APPLY_MOVE:
                    ch->max_move += mod;
                    break;
               case APPLY_AC:	// Magical AC
                    ch->armor += mod;
                    break;
               case APPLY_HITROLL:
                    ch->hitroll += mod;
                    break;
               case APPLY_DAMROLL:
                    ch->damroll += mod;
                    break;
               case APPLY_SAVING_PARA:
                    ch->saving_throw += mod;
                    break;
               case APPLY_SAVING_ROD:
                    ch->saving_throw += mod;
                    break;
               case APPLY_SAVING_PETRI:
                    ch->saving_throw += mod;
                    break;
               case APPLY_SAVING_BREATH:
                    ch->saving_throw += mod;
                    break;
               case APPLY_SAVING_SPELL:
                    ch->saving_throw += mod;
                    break;
               case APPLY_ENCUMBRANCE:
                    ch->saving_throw += mod;
                    break;
               }
          }
     }

    /* now add back spell effects */
     for ( af = ch->affected; af != NULL; af = af->next )
     {
          mod = af->modifier;
          switch ( af->location )
          {
          case APPLY_STR:
               ch->mod_stat[STAT_STR] += mod;
               break;
          case APPLY_DEX:
               ch->mod_stat[STAT_DEX] += mod;
               break;
          case APPLY_INT:
               ch->mod_stat[STAT_INT] += mod;
               break;
          case APPLY_WIS:
               ch->mod_stat[STAT_WIS] += mod;
               break;
          case APPLY_CON:
               ch->mod_stat[STAT_CON] += mod;
               break;
          case APPLY_SEX:
               ch->sex += mod;
               break;
          case APPLY_MANA:
               ch->max_mana += mod;
               break;
          case APPLY_HIT:
               ch->max_hit += mod;
               break;
          case APPLY_MOVE:
               ch->max_move += mod;
               break;
          case APPLY_AC:	// Magical AC               
               ch->armor += mod;
               break;
          case APPLY_HITROLL:
               ch->hitroll += mod;
               break;
          case APPLY_DAMROLL:
               ch->damroll += mod;
               break;
          case APPLY_SAVING_PARA:
               ch->saving_throw += mod;
               break;
          case APPLY_SAVING_ROD:
               ch->saving_throw += mod;
               break;
          case APPLY_SAVING_PETRI:
               ch->saving_throw += mod;
               break;
          case APPLY_SAVING_BREATH:
               ch->saving_throw += mod;
               break;
          case APPLY_SAVING_SPELL:
               ch->saving_throw += mod;
               break;
          case APPLY_ENCUMBRANCE:
               ch->saving_throw += mod;
               break;
          }
     }

    /* make sure sex is RIGHT!!!! */
     if ( ch->sex < 0 || ch->sex > 2 )
          ch->sex = ch->pcdata->true_sex;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust ( CHAR_DATA * ch )
{
     if ( ch->desc != NULL && ch->desc->original != NULL )
          ch = ch->desc->original;

     if ( ch->trust != 0 )
          return ch->trust;

     /* Okay good, the following prevents an NPC from being able to
      * be ordered to do IMM commands...
      */
     if ( IS_NPC ( ch ) && ch->level >= LEVEL_HERO )
          return LEVEL_HERO - 1;
     else
          return ch->level;
}

/*
 * Retrieve a character's age.
 */
int get_age ( CHAR_DATA * ch )
{
     int                 age;

     if ( IS_NPC ( ch ) )
     {
          return 20; /* Well yeah, we'll assume they're all young punks */
     }

	 /* Start with 1 under base, then will add 1 if have had bday */

	 /* First year will automatically be base, since will have had bday */

     age = ( pc_race_table[ch->pcdata->pcrace].startage - 1 ) + ( time_info.year - ch->pcdata->startyear );

	 /*  check for a modifier */

     age -= ch->pcdata->age_mod;

	 /* check for birthdate yet this year */

     if ( ( time_info.month >= ch->pcdata->startmonth ) &&
          ( time_info.day >= ch->pcdata->startday ) )
     {
          age += 1;
     }

     return age;

}

/* command for retrieving stats */
int get_curr_stat ( CHAR_DATA * ch, int stat )
{
     int                 max;

     if ( IS_NPC ( ch ) || ch->level > LEVEL_IMMORTAL )
          max = 25;

     else
     {
          max = pc_race_table[ch->pcdata->pcrace].max_stats[stat] + 4;

          if ( class_table[ch->pcdata->pclass].attr_prime == stat )
               max += 2;

          if ( ch->pcdata->pcrace == pcrace_lookup ( "human" ) )
               max += 1;

          max = UMIN ( max, 25 );
     }

     return URANGE ( 3, ch->perm_stat[stat] + ch->mod_stat[stat], max );
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n ( CHAR_DATA * ch )
{
     if ( !IS_NPC ( ch ) && ch->level >= LEVEL_IMMORTAL )
          return 1000;

     if ( IS_NPC ( ch ) && IS_SET ( ch->act, ACT_PET ) )
          return 0;

/* followers can carry, pets can't */

     return MAX_WEAR + 2 * get_curr_stat ( ch, STAT_DEX ) + ch->level;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w ( CHAR_DATA * ch )
{
     if ( !IS_NPC ( ch ) && ch->level >= LEVEL_IMMORTAL )
          return 1000000;

     if ( IS_NPC ( ch ) && IS_SET ( ch->act, ACT_PET ) )
          return 0;

     return str_app[get_curr_stat ( ch, STAT_STR )].carry +
          ch->level * 5 / 2;
}

/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
     char                name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
     char               *list, *string;

     string = str;
	 /* we need ALL parts of string to match part of namelist */
     for ( ;; )			/* start parsing string */
     {
          str = one_argument ( str, part );

          if ( part[0] == '\0' )
               return TRUE;

		  /* check to see if this is part of namelist */
          list = namelist;
          for ( ;; )		/* start parsing namelist */
          {
               list = one_argument ( list, name );
               if ( name[0] == '\0' )	/* this name was not found */
                    return FALSE;

               if ( !str_cmp ( string, name ) )
                    return TRUE;	/* full pattern match */

               if ( !str_cmp ( part, name ) )
                    break;
          }
     }
}

bool is_name_abbv ( char *str, char *namelist )	/*Zeran, abbrev is_name */
{
     char                name[MAX_INPUT_LENGTH];
     char               *list, *string;

     if ( strstr ( namelist, str ) == namelist )	/* got partial match already */
          return TRUE;
     string = str;
    /* check to see if this is part of namelist */
     list = namelist;
     for ( ;; )			/* start parsing namelist */
     {
          list = one_argument ( list, name );
          if ( name[0] == '\0' )	/* this name was not found */
               return FALSE;

          if ( strstr ( name, string ) == name )	/*Zeran: hack for abbreviations */
               return TRUE;	/*abbreviated string, return a match */

     }
}

/*
 * Set some values on an affect
 * NOTICE: This function does no error checking. Be careful calling it.
 * All values must be specified, if you set NULL, then it will set NULL,
 * overwriting what was there before.
 */
void set_affect ( AFFECT_DATA *paf, sh_int type, sh_int level, sh_int duration,
                  sh_int location, sh_int modifier, sh_int where,
                  int bitvector, char *caster )
{
     paf->type = type;
     paf->level = level;
     paf->duration = duration;
     paf->location = location;
     paf->modifier = modifier;
     paf->where = where;
     paf->bitvector = bitvector;
}

/*
 * Apply or remove an affect to a character.
 *
 */

void affect_modify ( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd )
{
     OBJ_DATA           *wield;
     OBJ_DATA           *objtmp;
     int		mod;
     bool                duplicate = FALSE;
     AFFECT_DATA        *tmp;

     mod = paf->modifier;

     if ( fAdd )
     {
          switch ( paf->where)
          {
          case TO_AFFECTS:
               SET_BIT (ch->affected_by, paf->bitvector);
               break;
          case TO_DETECTIONS:
               SET_BIT (ch->detections, paf->bitvector );
               break;
          case TO_PROTECTIONS:
               SET_BIT (ch->protections, paf->bitvector );
               break;
          }
     }

     else if ( paf->bitvector )	/* Zeran - can skip if bitvector is 0 */
     {
	/* check spells that were cast */

          for ( tmp = ch->affected; tmp != NULL; tmp = tmp->next )
          {
               if ( ( tmp != paf ) && ( tmp->bitvector ==  paf->bitvector ) && ( tmp->where == paf->where ) )
               {
                    duplicate = TRUE;
                    break;
               }
          }

	/* check against other worn objects if no duplicate found yet */
          if ( !duplicate )
               for ( objtmp = ch->carrying; objtmp != NULL; objtmp = objtmp->next_content )
               {
                    if ( objtmp->wear_loc != WEAR_NONE )	/*worn, check affects */
                    {
                         for ( tmp = objtmp->affected; tmp != NULL;  tmp = tmp->next )
                         {
                              if ( ( tmp != paf ) && ( tmp->bitvector == paf->bitvector ) && ( tmp->where == paf->where ) )
                                   duplicate = TRUE;
                              break;
                         }
                         for ( tmp = objtmp->pIndexData->affected;
                               tmp != NULL; tmp = tmp->next )
                         {
                              if ( ( tmp != paf ) && ( tmp->bitvector == paf->bitvector ) && ( tmp->where == paf->where ) )
                                   duplicate = TRUE;
                              break;
                         }
                    }
                    if ( duplicate )
                         break;
               }

          if ( !duplicate )
          {
               switch ( paf->where)
               {
               case TO_AFFECTS:
                    REMOVE_BIT (ch->affected_by, paf->bitvector);
		    ch->affected_by = ch->affected_by | race_table[ch->race].aff;
		    break;
               case TO_DETECTIONS:
                    REMOVE_BIT (ch->detections, paf->bitvector );
		    ch->detections = ch->detections | race_table[ch->race].detect;
		    break;
               case TO_PROTECTIONS:
                    REMOVE_BIT (ch->protections, paf->bitvector );
                    ch->detections = ch->detections | race_table[ch->race].protect;
		    break;
               }
          }
     }
     if ( !fAdd )
          mod = 0 - mod;
     switch ( paf->location )
     {
     default:
          bugf ( "Affect_modify: unknown location %d.", paf->location );
          return;

     case APPLY_NONE:							break;
     case APPLY_STR:		ch->mod_stat[STAT_STR] += mod;		break;
     case APPLY_DEX:		ch->mod_stat[STAT_DEX] += mod;		break;
     case APPLY_INT:		ch->mod_stat[STAT_INT] += mod;		break;
     case APPLY_WIS:		ch->mod_stat[STAT_WIS] += mod;		break;
     case APPLY_CON:		ch->mod_stat[STAT_CON] += mod;		break;
     case APPLY_SEX:		ch->sex += mod;				break;
     case APPLY_CLASS:							break;
     case APPLY_LEVEL:							break;
     case APPLY_AGE:							break;
     case APPLY_HEIGHT:							break;
     case APPLY_WEIGHT:							break;
     case APPLY_MANA:		ch->max_mana += mod;			break;
     case APPLY_HIT:		ch->max_hit += mod;			break;
     case APPLY_MOVE:		ch->max_move += mod;			break;
     case APPLY_GOLD:							break;
     case APPLY_EXP:							break;
     case APPLY_AC:             ch->armor += mod;                       break;
     case APPLY_HITROLL:	ch->hitroll += mod;			break;
     case APPLY_DAMROLL:	ch->damroll += mod;			break;
     case APPLY_SAVING_PARA:	ch->saving_throw += mod;		break;
     case APPLY_SAVING_ROD:	ch->saving_throw += mod;		break;
     case APPLY_SAVING_PETRI:	ch->saving_throw += mod;		break;
     case APPLY_SAVING_BREATH:	ch->saving_throw += mod;		break;
     case APPLY_SAVING_SPELL:	ch->saving_throw += mod;		break;
     case APPLY_ENCUMBRANCE:	ch->encumbrance += mod;			break;
     }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */

     if ( !IS_NPC ( ch ) &&
          ( wield = get_eq_char ( ch, WEAR_WIELD ) ) != NULL &&
          get_obj_weight ( wield ) >
          str_app[get_curr_stat ( ch, STAT_STR )].wield )
     {
          static int          depth;

          if ( depth == 0 )
          {
               depth++;
               act ( "You drop $p.", ch, wield, NULL, TO_CHAR );
               act ( "$n drops $p.", ch, wield, NULL, TO_ROOM );
               obj_from_char ( wield );
               obj_to_room ( wield, ch->in_room );
               depth--;
          }
     }

    /* now check the dual wield weapon */
     if ( !IS_NPC ( ch ) &&
          ( wield = get_eq_char ( ch, WEAR_WIELD2 ) ) != NULL &&
          get_obj_weight ( wield ) >
          str_app[get_curr_stat ( ch, STAT_STR )].wield )
     {
          static int          depth2;

          if ( depth2 == 0 )
          {
               depth2++;
               act ( "You drop $p.", ch, wield, NULL, TO_CHAR );
               act ( "$n drops $p.", ch, wield, NULL, TO_ROOM );
               obj_from_char ( wield );
               obj_to_room ( wield, ch->in_room );
               depth2--;
          }
     }

     return;
}

/*
 * Give an affect to a char.
 */

void affect_to_char ( CHAR_DATA * ch, AFFECT_DATA * paf )
{
     AFFECT_DATA        *paf_new;

     if ( affect_free == NULL )
     {
          paf_new = alloc_perm ( sizeof ( *paf_new ), "paf_new:aff_to_ch" );
     }
     else
     {
          paf_new = affect_free;
          affect_free = affect_free->next;
     }

     *paf_new = *paf;

     paf_new->next = ch->affected;
     ch->affected = paf_new;
     affect_modify ( ch, paf_new, TRUE );

     return;
}

/* give an affect to an object */
void affect_to_obj ( OBJ_DATA * obj, AFFECT_DATA * paf )
{
     AFFECT_DATA        *paf_new;

     if ( affect_free == NULL )
     {
          paf_new = alloc_perm ( sizeof ( *paf_new ), "paf_new:aff_to_obj" );
     }
     else
     {
          paf_new = affect_free;
          affect_free = affect_free->next;
     }

     *paf_new = *paf;
     paf_new->next = obj->affected;
     obj->affected = paf_new;

     return;
}

/*
 * affect_to_room ??
 * What were ya thinkin' Z?
 * Neat idea though, so I'll leave it here, just commented since it is unused
 * and there's no code to support it -- Lotherius
 */

//void affect_to_room ( ROOM_INDEX_DATA * room, AFFECT_DATA * paf )
//{
//    AFFECT_DATA        *paf_new;
//
//    if ( affect_free == NULL )
//    {
//	paf_new = alloc_perm ( sizeof ( *paf_new ), "paf_new:aff_to_room" );
//    } else
//    {
//	paf_new = affect_free;
//	affect_free = affect_free->next;
//    }
//
//    *paf_new = *paf;
//    paf_new->next = room->affected;
//
//    room->affected = paf_new;
//
//    return;
//}

//void affect_remove_room ( ROOM_INDEX_DATA * room, AFFECT_DATA * paf )
//{
//    if ( room->affected == NULL )
//    {
//	bugf ( "Affect_remove_room: no affect." );
//	return;
//    }
//
//    if ( paf == room->affected )
//    {
//	room->affected = paf->next;
//    } else
//    {
//	AFFECT_DATA        *prev;
//
//	for ( prev = room->affected; prev != NULL;
//	      prev = prev->next )
//	{
//	    if ( prev->next == paf )
//	    {
//		prev->next = paf->next;
//		break;
//	    }
//	}
//
//	if ( prev == NULL )
//	{
//	    bugf ( "Affect_remove_room: cannot find paf." );
//	    return;
//	}
//    }
//
//    paf->next = affect_free;
//    affect_free = paf;
//    return;
//}

/*
 * Remove an affect from a char.
 */

void affect_remove ( CHAR_DATA * ch, AFFECT_DATA * paf )
{
     if ( ch->affected == NULL )
     {
          bugf ( "Affect_remove: no affect." );
          return;
     }

     affect_modify ( ch, paf, FALSE );

     if ( paf == ch->affected )
     {
          ch->affected = paf->next;
     }
     else
     {
          AFFECT_DATA        *prev;

          for ( prev = ch->affected; prev != NULL;
                prev = prev->next )
          {
               if ( prev->next == paf )
               {
                    prev->next = paf->next;
                    break;
               }
          }

          if ( prev == NULL )
          {
               bugf ( "Affect_remove: cannot find paf." );
               return;
          }
     }

     paf->next = affect_free;
     affect_free = paf;
     return;
}

void affect_remove_obj ( OBJ_DATA * obj, AFFECT_DATA * paf )
{
     if ( obj->affected == NULL )
     {
          bugf ( "Affect_remove_object: no affect." );
          return;
     }

     if ( obj->carried_by != NULL && obj->wear_loc != -1 )
          affect_modify ( obj->carried_by, paf, FALSE );

     if ( paf == obj->affected )
     {
          obj->affected = paf->next;
     }
     else
     {
          AFFECT_DATA        *prev;

          for ( prev = obj->affected; prev != NULL;
                prev = prev->next )
          {
               if ( prev->next == paf )
               {
                    prev->next = paf->next;
                    break;
               }
          }

          if ( prev == NULL )
          {
               bugf ( "Affect_remove_object: cannot find paf." );
               return;
          }
     }

     paf->next = affect_free;
     affect_free = paf;
     return;
}

/*
 * Strip all affects of a given sn.
 */

void affect_strip ( CHAR_DATA * ch, int sn )
{
     AFFECT_DATA        *paf;
     AFFECT_DATA        *paf_next;

     for ( paf = ch->affected; paf != NULL; paf = paf_next )
     {
          paf_next = paf->next;
          if ( paf->type == sn )
               affect_remove ( ch, paf );
     }

     return;
}

/*
 * Return true if a char is affected by a spell.
 */
bool is_affected ( CHAR_DATA * ch, int sn )
{
     AFFECT_DATA        *paf;

     for ( paf = ch->affected; paf != NULL; paf = paf->next )
     {
          if ( paf->type == sn )
               return TRUE;
     }

     return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join ( CHAR_DATA * ch, AFFECT_DATA * paf )
{
     AFFECT_DATA        *paf_old;
     bool                found;

     found = FALSE;
     for ( paf_old = ch->affected; paf_old != NULL;
           paf_old = paf_old->next )
     {
          if ( paf_old->type == paf->type )
          {
               paf->level = ( paf->level += paf_old->level ) / 2;
               paf->duration += paf_old->duration;
               paf->modifier += paf_old->modifier;
               affect_remove ( ch, paf_old );
               break;
          }
     }

     affect_to_char ( ch, paf );
     return;
}

/*
 * Move a char out of a room.
 */
void char_from_room ( CHAR_DATA * ch )
{
     OBJ_DATA           *obj;

     if ( ch->in_room == NULL )
     {
          bugf ( "Char_from_room: NULL." );
          return;
     }

     if ( !IS_NPC ( ch ) )
          --ch->in_room->area->nplayer;

     if ( ( obj = get_eq_char ( ch, WEAR_LIGHT ) ) != NULL
          && obj->item_type == ITEM_LIGHT
          && obj->value[2] != 0 && ch->in_room->light > 0 )
          --ch->in_room->light;

     if ( ch == ch->in_room->people )
     {
          ch->in_room->people = ch->next_in_room;
     }
     else
     {
          CHAR_DATA          *prev;

          for ( prev = ch->in_room->people; prev;
                prev = prev->next_in_room )
          {
               if ( prev->next_in_room == ch )
               {
                    prev->next_in_room = ch->next_in_room;
                    break;
               }
          }

          if ( prev == NULL )
               bugf ( "Char_from_room: ch not found." );
     }

     ch->in_room = NULL;
     ch->next_in_room = NULL;
     return;
}

/*
 * Move a char into a room.
 */

void char_to_room ( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
     OBJ_DATA           *obj;

     if ( pRoomIndex == NULL )
     {
          bugf ( "Char_to_room: NULL." );
          return;
     }

     ch->in_room = pRoomIndex;
     ch->next_in_room = pRoomIndex->people;
     pRoomIndex->people = ch;

     if ( !IS_NPC ( ch ) )
     {
          if ( ch->in_room->area->empty )
          {
               ch->in_room->area->empty = FALSE;
               ch->in_room->area->age = 0;
          }
          ++ch->in_room->area->nplayer;
     }

     if ( ( obj = get_eq_char ( ch, WEAR_LIGHT ) ) != NULL
          && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
          ++ch->in_room->light;

    /*
     * startup some music
     * music only sends data if the soundfile has changed
     * Areas with no sound, set the sound to "None"
     */

     if ( !IS_NPC(ch) )
     {
          if ( ch->in_room->area->soundfile[0] != '\0' )
          {
               music ( ch->in_room->area->soundfile, ch, TRUE );
          }
          else
          {
               free_string (ch->pcdata->mplaying);
               ch->pcdata->mplaying = str_dup ( "None" );
               stop_music ( ch->desc );
          }
     }

     if ( IS_AFFECTED ( ch, AFF_PLAGUE ) )
     {
          AFFECT_DATA        *af, plague;
          CHAR_DATA          *vch;
          int                 save;

          for ( af = ch->affected; af != NULL; af = af->next )
          {
               if ( af->type == gsn_plague )
                    break;
          }

          if ( af == NULL )
          {
               REMOVE_BIT ( ch->affected_by, AFF_PLAGUE );
               return;
          }

          if ( af->level == 1 )
               return;

          plague.type = gsn_plague;
          plague.level = af->level - 1;
          plague.duration = number_range ( 1, 2 * plague.level );
          plague.location = APPLY_STR;
          plague.modifier = -5;
          plague.bitvector = AFF_PLAGUE;

          for ( vch = ch->in_room->people; vch != NULL;
                vch = vch->next_in_room )
          {
               switch ( check_immune ( vch, DAM_DISEASE ) )
               {
               case ( IS_NORMAL ):
                    save = af->level - 4;
                    break;
               case ( IS_IMMUNE ):
                    save = 0;
                    break;
               case ( IS_RESISTANT ):
                    save = af->level - 8;
                    break;
               case ( IS_VULNERABLE ):
                    save = af->level;
                    break;
               default:
                    save = af->level - 4;
                    break;
               }

               if ( save != 0 && !saves_spell ( save, vch ) &&
                    !IS_IMMORTAL ( vch ) &&
                    !IS_AFFECTED ( vch, AFF_PLAGUE ) &&
                    number_bits ( 6 ) == 0 )
               {
                    send_to_char ( "You feel hot and feverish.\n\r", vch );
                    act ( "$n shivers and looks very ill.", vch, NULL,
                          NULL, TO_ROOM );
                    affect_join ( vch, &plague );
               }
          }
     }

     return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char ( OBJ_DATA * obj, CHAR_DATA * ch )
{
     obj->next_content = ch->carrying;
     ch->carrying = obj;
     obj->carried_by = ch;
     obj->in_room = NULL;
     obj->in_obj = NULL;
     ch->carry_number += get_obj_number ( obj );
     ch->carry_weight += get_obj_weight ( obj );
}

/*
 * Take an obj from its character.
 */
void obj_from_char ( OBJ_DATA * obj )
{
     CHAR_DATA          *ch;

     if ( ( ch = obj->carried_by ) == NULL )
     {
          bugf ( "Obj_from_char: null ch. Fatal, exiting to avoid possible infinite loop." );
          /* This error must be fatal to avoid a horrendous loop */
          /* If you're getting this error you MUST fix something. */
          exit (1);
     }

     if ( obj->wear_loc != WEAR_NONE )
          unequip_char ( ch, obj );

     if ( ch->carrying == obj )
     {
          ch->carrying = obj->next_content;
     }
     else
     {
          OBJ_DATA           *prev;

          for ( prev = ch->carrying; prev != NULL;
                prev = prev->next_content )
          {
               if ( prev->next_content == obj )
               {
                    prev->next_content = obj->next_content;
                    break;
               }
          }

          if ( prev == NULL )
               bugf ( "Obj_from_char: obj not in list." );
     }

     obj->carried_by = NULL;
     obj->next_content = NULL;
     ch->carry_number -= get_obj_number ( obj );
     ch->carry_weight -= get_obj_weight ( obj );
     return;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA           *get_eq_char ( CHAR_DATA * ch, int iWear )
{
     OBJ_DATA           *obj;

     if ( ch == NULL )
          return NULL;

     for ( obj = ch->carrying; obj != NULL;
           obj = obj->next_content )
     {
          if ( obj->wear_loc == iWear )
               return obj;
     }

     return NULL;
}

/*
 * Equip a char with an obj.
 * Changed to a boolean function so the caller can see if the item was able to be
 * equipped.
 */
bool equip_char ( CHAR_DATA * ch, OBJ_DATA * obj, int iWear )
{
     AFFECT_DATA        *paf;

     if ( get_eq_char ( ch, iWear ) != NULL )
     {
          if (!IS_NPC(ch))
               bugf ( "Equip_char: already equipped (%d : %d : %s).", iWear,
                      ch->in_room->vnum, ch->name );
          else
               bugf ( "Equip_char: already equipped (%d : %d : %d).",
                      iWear, ch->in_room->vnum, ch->pIndexData->vnum );
          return FALSE;
     }

     if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch) )
          ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch) )
          ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
     {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
          act ( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
          act ( "$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM );
          obj_from_char ( obj );
          obj_to_room ( obj, ch->in_room );
          return FALSE;
     }

     obj->wear_loc = iWear;

     if ( !obj->enchanted )
     {
          for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
          {
               affect_modify ( ch, paf, TRUE );
          }
     }

     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          affect_modify ( ch, paf, TRUE );
     }

     if ( obj->item_type == ITEM_LIGHT
          && obj->value[2] != 0 && ch->in_room != NULL )
          ++ch->in_room->light;

     return TRUE;
}

/*
 * Unequip a char with an obj.
 */
bool unequip_char ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     AFFECT_DATA        *paf;

     if ( obj->wear_loc == WEAR_NONE )
     {
          bugf ( "Unequip_char: already unequipped." );
          return FALSE;
     }

     obj->wear_loc = -1;

     if ( !obj->enchanted )
          for ( paf = obj->pIndexData->affected; paf != NULL;
                paf = paf->next )
          {
               affect_modify ( ch, paf, FALSE );
          }
     for ( paf = obj->affected; paf != NULL; paf = paf->next )
     {
          affect_modify ( ch, paf, FALSE );
     }

     if ( obj->item_type == ITEM_LIGHT
          && obj->value[2] != 0
          && ch->in_room != NULL && ch->in_room->light > 0 )
          --ch->in_room->light;

     return TRUE;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room ( OBJ_DATA * obj )
{
     ROOM_INDEX_DATA    *in_room;

     if ( ( in_room = obj->in_room ) == NULL )
     {
          bugf ( "obj_from_room: NULL." );
          return;
     }

     if ( obj == in_room->contents )
     {
          in_room->contents = obj->next_content;
     }
     else
     {
          OBJ_DATA           *prev;

          for ( prev = in_room->contents; prev;
                prev = prev->next_content )
          {
               if ( prev->next_content == obj )
               {
                    prev->next_content = obj->next_content;
                    break;
               }
          }

          if ( prev == NULL )
          {
               bugf ( "Obj_from_room: obj not found." );
               return;
          }
     }

     obj->in_room = NULL;
     obj->next_content = NULL;
     return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room ( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex )
{
     obj->next_content = pRoomIndex->contents;
     pRoomIndex->contents = obj;
     obj->in_room = pRoomIndex;
     obj->carried_by = NULL;
     obj->in_obj = NULL;
     return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj ( OBJ_DATA * obj, OBJ_DATA * obj_to )
{
     obj->next_content = obj_to->contains;
     obj_to->contains = obj;
     obj->in_obj = obj_to;
     obj->in_room = NULL;
     obj->carried_by = NULL;
     if ( obj_to->pIndexData->vnum == OBJ_VNUM_PIT )
          obj->cost = 1;

     for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
     {
          if ( obj_to->carried_by != NULL )
          {
               obj_to->carried_by->carry_number +=
                    get_obj_number ( obj );
               obj_to->carried_by->carry_weight +=
                    get_obj_weight ( obj );
          }
     }

     return;
}

/*
 * Move an object out of an object.
 */
void obj_from_obj ( OBJ_DATA * obj )
{
     OBJ_DATA           *obj_from;

     if ( ( obj_from = obj->in_obj ) == NULL )
     {
          bugf ( "Obj_from_obj: null obj_from." );
          return;
     }

     if ( obj == obj_from->contains )
     {
          obj_from->contains = obj->next_content;
     }
     else
     {
          OBJ_DATA           *prev;

          for ( prev = obj_from->contains; prev;
                prev = prev->next_content )
          {
               if ( prev->next_content == obj )
               {
                    prev->next_content = obj->next_content;
                    break;
               }
          }

          if ( prev == NULL )
          {
               bugf ( "Obj_from_obj: obj not found." );
               return;
          }
     }

     obj->next_content = NULL;
     obj->in_obj = NULL;

     for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
     {
          if ( obj_from->carried_by != NULL )
          {
               obj_from->carried_by->carry_number -=
                    get_obj_number ( obj );
               obj_from->carried_by->carry_weight -=
                    get_obj_weight ( obj );
          }
     }

     return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj ( OBJ_DATA * obj )
{
     CHAR_DATA          *ch = obj->carried_by;
     OBJ_DATA           *obj_content;
     OBJ_DATA           *obj_next;

     if ( obj->in_room != NULL )
          obj_from_room ( obj );
     else if ( obj->carried_by != NULL )
          obj_from_char ( obj );
     else if ( obj->in_obj != NULL )
          obj_from_obj ( obj );

     for ( obj_content = obj->contains; obj_content;
           obj_content = obj_next )
     {
          obj_next = obj_content->next_content;
          extract_obj ( obj->contains );
     }

     if ( object_list == obj )
     {
          object_list = obj->next;
     }
     else
     {
          OBJ_DATA           *prev;

          for ( prev = object_list; prev != NULL; prev = prev->next )
          {
               if ( prev->next == obj )
               {
                    prev->next = obj->next;
                    break;
               }
          }

          if ( prev == NULL && obj->pIndexData != NULL )
          {
               bugf ( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
               return;
          }
     }

     {
          AFFECT_DATA        *paf;
          AFFECT_DATA        *paf_next;

          for ( paf = obj->affected; paf != NULL; paf = paf_next )
          {
               paf_next = paf->next;
               paf->next = affect_free;
               affect_free = paf;
          }
     }

     {
          EXTRA_DESCR_DATA   *ed;
          EXTRA_DESCR_DATA   *ed_next;

          for ( ed = obj->extra_descr; ed != NULL; ed = ed_next )
          {
               ed_next = ed->next;
               free_string ( ed->description );
               free_string ( ed->keyword );
               ed->next = extra_descr_free;
               extra_descr_free = ed;
          }
     }

     free_string ( obj->name );
     free_string ( obj->description );
     free_string ( obj->short_descr );
     free_string ( obj->owner );
     if ( obj->pIndexData == NULL )
          bugf ( "Bug: (extract_obj)  NULL obj->pIndexData" );
     else			/*safe to decrement */
          --obj->pIndexData->count;
    /* Zeran - if obj was carried by a player who is quitting, then don't
     * decrement */
     if ( obj->reset != NULL )
     {
          if ( ch == NULL )
          {
               obj->reset->count--;
          }
          else if ( !ch->quitting )
          {
               obj->reset->count--;
          }
     }

     obj->reset = NULL;
     obj->next = obj_free;
     obj_free = obj;
     return;
}

/*
 * Extract a char from the world.
 */
void extract_char ( CHAR_DATA * ch, bool fPull )
{
     CHAR_DATA          *wch;
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;

     if ( ch->in_room == NULL )
     {
          bugf ( "Extract_char: NULL." );
          return;
     }

     nuke_pets ( ch );
     ch->pet = NULL;		/* just in case */

     if ( fPull )
          die_follower ( ch );

     stop_fighting ( ch, TRUE );

     for ( obj = ch->carrying; obj != NULL; obj = obj_next )
     {
          extract_obj ( obj );
          obj_next = obj->next_content;
     }

     char_from_room ( ch );

     if ( !fPull )
     {
          char_to_room ( ch, get_room_index ( pc_race_table[ch->pcdata->pcrace].healer ) );
          return;
     }

     if ( IS_NPC ( ch ) )
     {
          --ch->pIndexData->count;
          if ( ch->reset != NULL )
               --ch->reset->count;	/* Zeran - added */
     }

     if ( ch->desc != NULL && ch->desc->original != NULL )
          do_return ( ch, "" );

     for ( wch = char_list; wch != NULL; wch = wch->next )
     {
          if ( wch->reply == ch )
               wch->reply = NULL;
          if ( ch->mprog_target == wch )
               wch->mprog_target = NULL;
     }

     if ( ch == char_list )
     {
          char_list = ch->next;
     }
     else
     {
          CHAR_DATA          *prev;

          for ( prev = char_list; prev != NULL; prev = prev->next )
          {
               if ( prev->next == ch )
               {
                    prev->next = ch->next;
                    break;
               }
          }

          if ( prev == NULL )
          {
               bugf ( "Extract_char: char not found." );
               return;
          }
     }

     if ( ch->desc )
          ch->desc->character = NULL;
     free_char ( ch );
     return;
}

/*
 * Find a char in the room.
 * Sortof the one with obj/roomprogs, but I had to modify it to fit some of the
 * modifications we'd already done here.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *rch;
     int                 number;
     int                 count;

     number = number_argument ( argument, arg );
     count = 0;
     if ( !str_cmp ( arg, "self" ) )
          return ch;

     if ( ch && room )
     {
          bugf ( "get_char_room received multiple types (ch/room)" );
          return NULL;
     }

     if ( ch )
          rch = ch->in_room->people;
     else
          rch = room->people;
     for ( ; rch != NULL; rch = rch->next_in_room )
     {
          if ( (ch && !can_see( ch, rch ))
               || ( !is_name( arg, rch->name )
                    && !is_name_abbv ( arg, rch->name ) ) )
               continue;
          if ( ++count == number )
               return rch;
     }
     return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *wch;
     int                 number;
     int                 count;

    /* Zeran - trying "self" alias for yourself */
     if ( !str_cmp ( "self", argument ) )
          return ch;

     if ( ch && ( wch = get_char_room( ch, NULL, argument ) ) != NULL )
          return wch;

     number = number_argument( argument, arg );
     count  = 0;
     for ( wch = char_list; wch != NULL ; wch = wch->next )
     {
          if ( wch->in_room == NULL || ( ch && !can_see( ch, wch ) )
               ||   ( !is_name( arg, wch->name )
                      && !is_name_abbv ( arg, wch->name ) ) )
               continue;
          if ( ++count == number )
               return wch;
     }
     return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA           *get_obj_type ( OBJ_INDEX_DATA * pObjIndex )
{
     OBJ_DATA           *obj;

     for ( obj = object_list; obj != NULL; obj = obj->next )
     {
          if ( obj->pIndexData == pObjIndex )
               return obj;
     }

     return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA           *get_obj_list ( CHAR_DATA * ch, char *argument, OBJ_DATA * list )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 number;
     int                 count;

     number = number_argument ( argument, arg );
     count = 0;
     for ( obj = list; obj != NULL; obj = obj->next_content )
     {
          if ( can_see_obj ( ch, obj ) && ( is_name_abbv ( arg, obj->name ) || is_name ( arg, obj->name ) ) )
          {
               if ( ++count == number )
                    return obj;
          }
     }
     return NULL;
}

/*
 * Find an obj in player's inventory.
 */

OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
     char arg[MAX_INPUT_LENGTH];
     OBJ_DATA *obj;
     int number;
     int count;

     number = number_argument( argument, arg );
     count  = 0;
     for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
     {
          if ( obj->wear_loc == WEAR_NONE
               &&   ( viewer ? can_see_obj( viewer, obj ) : TRUE )
               &&   ( is_name( arg, obj->name ) || is_name_abbv ( arg, obj->name ) ) )
          {
               if ( ++count == number )
                    return obj;
          }
     }
     return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument, bool character )
{
     char arg[MAX_INPUT_LENGTH];
     OBJ_DATA *obj;
     int number;
     int count;

     number = number_argument( argument, arg );
     count  = 0;
     for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
     {
          if ( obj->wear_loc != WEAR_NONE
               &&  ( character ? can_see_obj( ch, obj ) : TRUE)
               &&   ( is_name( arg, obj->name ) || is_name_abbv ( arg, obj->name ) ) )
          {
               if ( ++count == number )
                    return obj;
          }
     }
     return NULL;
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
     OBJ_DATA *obj;
     int number, count;
     char arg[MAX_INPUT_LENGTH];

     if ( ch && room )
     {
          bugf ( "get_obj_here received a ch and a room");
          return NULL;
     }

     number = number_argument( argument, arg );
     count = 0;

     if ( ch )
     {
          obj = get_obj_list( ch, argument, ch->in_room->contents );
          if ( obj != NULL )
               return obj;
          if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
               return obj;
          if ( ( obj = get_obj_wear( ch, argument, TRUE ) ) != NULL )
               return obj;
     }
     else
     {
          for ( obj = room->contents; obj; obj = obj->next_content )
          {
               if ( !is_name( arg, obj->name ) && !is_name_abbv( arg, obj->name ) )
                    continue;
               if ( ++count == number )
                    return obj;
          }
     }

     return NULL;
}

/*
 * Find an obj in the world.
 */

OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
     char arg[MAX_INPUT_LENGTH];
     OBJ_DATA *obj;
     int number;
     int count;

     if ( ch && ( obj = get_obj_here( ch, NULL, argument ) ) != NULL )
          return obj;

     number = number_argument( argument, arg );
     count  = 0;
     for ( obj = object_list; obj != NULL; obj = obj->next )
     {
          if ( ( ch && !can_see_obj( ch, obj ) )
               || ( !is_name( arg, obj->name ) && !is_name_abbv ( arg, obj->name ) ) )
               continue;
          if ( ++count == number )
               return obj;
     }
     return NULL;
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA           *create_money ( int amount )
{
     char                buf[MAX_STRING_LENGTH];
     OBJ_DATA           *obj;

     if ( amount <= 0 )
     {
          bugf ( "Create_money: zero or negative money %d.", amount );
          amount = 1;
     }

     if ( amount == 1 )
     {
          obj =
               create_object ( get_obj_index ( OBJ_VNUM_MONEY_ONE ),
                               0 );
     }
     else
     {
          obj =
               create_object ( get_obj_index ( OBJ_VNUM_MONEY_SOME ),
                               0 );
          SNP ( buf, obj->short_descr, amount );
          free_string ( obj->short_descr );
          obj->short_descr = str_dup ( buf );
          obj->value[0] = amount;
          obj->cost = amount;
     }

     return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number ( OBJ_DATA * obj )
{
     int                 number;

     if ( obj->item_type == ITEM_CONTAINER ||
          obj->item_type == ITEM_MONEY )
          number = 0;
     else
          number = 1;

     for ( obj = obj->contains; obj != NULL;
           obj = obj->next_content )
          number += get_obj_number ( obj );

     return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight ( OBJ_DATA * obj )
{
     int                 weight;

    /* Zeran - if object is a floating object, weight is irrelevant */
     if ( IS_SET ( obj->wear_flags, ITEM_WEAR_FLOAT ) )
          weight = 0;
     else
          weight = obj->weight;
     for ( obj = obj->contains; obj != NULL;
           obj = obj->next_content )
          weight += get_obj_weight ( obj );

     return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark ( ROOM_INDEX_DATA * pRoomIndex )
{
     if ( pRoomIndex->light > 0 )
          return FALSE;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_DARK ) )
          return TRUE;

     if ( pRoomIndex->sector_type == SECT_INSIDE
          || pRoomIndex->sector_type == SECT_CITY )
          return FALSE;

     if ( weather_info.sunlight == SUN_SET
          || weather_info.sunlight == SUN_DARK )
          return TRUE;

     return FALSE;
}

/*
 * True if room is private.
 */
bool room_is_private ( ROOM_INDEX_DATA * pRoomIndex )
{
     CHAR_DATA          *rch;
     int                 count;

     count = 0;
     for ( rch = pRoomIndex->people; rch != NULL;
           rch = rch->next_in_room )
          count++;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_PRIVATE ) &&
          count >= 2 )
          return TRUE;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_SOLITARY ) &&
          count >= 1 )
          return TRUE;

     return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room ( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
     if ( !pRoomIndex )
          return FALSE;		/* If the roomindex is NULL, must return false */
     
     if ( IS_RENTED ( pRoomIndex->lease ) )
     {
          if ( pRoomIndex->lease->owner_only )
          {
               if ( !str_cmp ( ch->name, pRoomIndex->lease->rented_by ) ) // Owners can always see their own room
                    return TRUE;
               else
                    return FALSE;
          }
     }

     if ( IS_NPC(ch) )
     {
          return TRUE;
     }

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_IMP_ONLY ) && get_trust ( ch ) < MAX_LEVEL )
          return FALSE;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_GODS_ONLY ) && !IS_IMMORTAL ( ch ) )
          return FALSE;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_HEROES_ONLY ) && !IS_HERO ( ch ) )
          return FALSE;

     if ( IS_SET ( pRoomIndex->room_flags, ROOM_NEWBIES_ONLY ) && ch->level > 5 && !IS_IMMORTAL ( ch ) )
          return FALSE;

     return TRUE;
}

/*
 * True if char can see victim.
 */
bool can_see ( CHAR_DATA * ch, CHAR_DATA * victim )
{

/* RT changed so that WIZ_INVIS has levels */
     if ( ch == victim )
          return TRUE;

     if ( !IS_NPC ( victim )
          && IS_SET ( victim->act, PLR_WIZINVIS )
          && get_trust ( ch ) < victim->invis_level )
          return FALSE;

     if ( !IS_NPC ( victim )
          && IS_SET ( victim->act, PLR_CLOAK )
          && get_trust ( ch ) < victim->cloak_level
          && ch->in_room != victim->in_room )
          return FALSE;

     if ( ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_HOLYLIGHT ) )
          || ( IS_NPC ( ch ) && IS_IMMORTAL ( ch ) ) )
          return TRUE;

     if ( IS_AFFECTED ( ch, AFF_BLIND ) )
          return FALSE;

     if ( room_is_dark ( ch->in_room ) &&
          !CAN_DETECT ( ch, DET_INFRARED ) )
          return FALSE;

     if ( IS_AFFECTED ( victim, AFF_INVISIBLE )
          && !CAN_DETECT ( ch, DET_INVIS ) )
          return FALSE;

     if ( IS_AFFECTED ( victim, AFF_HIDE )
          && !CAN_DETECT ( ch, DET_HIDDEN ) )
          return FALSE;

    /* sneaking */
     if ( IS_AFFECTED ( victim, AFF_SNEAK )
          && !CAN_DETECT ( ch, DET_HIDDEN )
          && victim->fighting == NULL
          && ( IS_NPC ( ch ) ? !IS_NPC ( victim ) :
               IS_NPC ( victim ) ) )
     {
          int                 chance;

          chance = get_skill ( victim, gsn_sneak );
          chance += get_curr_stat ( ch, STAT_DEX ) * 3 / 2;
          chance -= get_curr_stat ( ch, STAT_INT ) * 2;
          chance += ch->level - victim->level * 3 / 2;

          if ( number_percent (  ) < chance )
               return FALSE;
     }

     if ( IS_AFFECTED ( victim, AFF_HIDE )
          && !CAN_DETECT ( ch, DET_HIDDEN )
          && victim->fighting == NULL
          && ( IS_NPC ( ch ) ? !IS_NPC ( victim ) :
               IS_NPC ( victim ) ) )
          return FALSE;

     return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_HOLYLIGHT ) )
          return TRUE;

     if ( IS_SET ( obj->extra_flags, ITEM_VIS_DEATH ) )
          return FALSE;

     if ( IS_AFFECTED ( ch, AFF_BLIND ) &&
          obj->item_type != ITEM_POTION )
          return FALSE;

     if ( IS_SET ( obj->extra_flags, ITEM_CONCEALED ) &&
          !ch->searching )
          return FALSE;

     if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
          return TRUE;

     if ( IS_SET ( obj->extra_flags, ITEM_INVIS )
          && !CAN_DETECT ( ch, DET_INVIS ) )
          return FALSE;

     if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) )
          return TRUE;

     if ( room_is_dark ( ch->in_room ) &&
          !CAN_DETECT ( ch, DET_INFRARED ) )
          return FALSE;

     return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     if ( !IS_SET ( obj->extra_flags, ITEM_NODROP ) )
          return TRUE;

     if ( !IS_NPC ( ch ) && ch->level >= LEVEL_IMMORTAL )
          return TRUE;

     return FALSE;
}

/*
 * Return ascii name of an item type.
 */
char               *item_type_name ( OBJ_DATA * obj )
{
     switch ( obj->item_type )
     {
     case ITEM_LIGHT:          return "light";
     case ITEM_SCROLL:         return "scroll";
     case ITEM_WAND:           return "wand";
     case ITEM_STAFF:          return "staff";
     case ITEM_WEAPON:         return "weapon";
     case ITEM_TREASURE:       return "treasure";
     case ITEM_ARMOR:          return "armor";
     case ITEM_POTION:         return "potion";
     case ITEM_FURNITURE:      return "furniture";
     case ITEM_TRASH:          return "trash";
     case ITEM_CONTAINER:      return "container";
     case ITEM_DRINK_CON:      return "drink container";
     case ITEM_KEY:            return "key";
     case ITEM_FOOD:           return "food";
     case ITEM_MONEY:          return "money";
     case ITEM_BOAT:           return "boat";
     case ITEM_CORPSE_NPC:     return "npc corpse";
     case ITEM_CORPSE_PC:      return "pc corpse";
     case ITEM_FOUNTAIN:       return "fountain";
     case ITEM_PILL:           return "pill";
     case ITEM_MAP:            return "map";
     case ITEM_PRIDE:          return "pride";
     case ITEM_COMPONENT:      return "component";
     case ITEM_PORTAL:         return "portal";
     }

     bugf ( "Item_type_name: unknown type %d.", obj->item_type );
     return "(unknown)";
}

/*
 * Return ascii name of an affect location.
 */
char               *affect_loc_name ( int location )
{
     switch ( location )
     {
     case APPLY_NONE:          return "none";
     case APPLY_STR:           return "strength";
     case APPLY_DEX:           return "dexterity";
     case APPLY_INT:           return "intelligence";
     case APPLY_WIS:           return "wisdom";
     case APPLY_CON:           return "constitution";
     case APPLY_SEX:           return "sex";
     case APPLY_CLASS:         return "class";
     case APPLY_LEVEL:         return "level";
     case APPLY_AGE:           return "age";
     case APPLY_MANA:          return "mana";
     case APPLY_HIT:           return "hp";
     case APPLY_MOVE:          return "moves";
     case APPLY_GOLD:          return "gold";
     case APPLY_EXP:           return "experience";
     case APPLY_AC:            return "armor class";
     case APPLY_HITROLL:       return "hit roll";
     case APPLY_DAMROLL:       return "damage roll";
     case APPLY_SAVING_PARA:   return "save vs paralysis";
     case APPLY_SAVING_ROD:    return "save vs rod";
     case APPLY_SAVING_PETRI:  return "save vs petrification";
     case APPLY_SAVING_BREATH: return "save vs breath";
     case APPLY_SAVING_SPELL:  return "save vs spell";
     case APPLY_ENCUMBRANCE:   return "encumbrance";
     }

     bugf ( "Affect_location_name: unknown location %d.", location );
     return "(unknown)";
}

/*
 * Return ascii name of an affect bit vector
 */

char               *affect_bit_name ( int vector )
{
     static char         buf[1024];

     buf[0] = '\0';
     if ( vector & AFF_BLIND         ) SLCAT ( buf, " blind"         );
     if ( vector & AFF_INVISIBLE     ) SLCAT ( buf, " invisible"     );
     if ( vector & AFF_MELD          ) SLCAT ( buf, " mind meld"     );
     if ( vector & AFF_FAERIE_FIRE   ) SLCAT ( buf, " faerie fire"   );
     if ( vector & AFF_CURSE         ) SLCAT ( buf, " curse"         );
     if ( vector & AFF_POISON        ) SLCAT ( buf, " poison"        );
     if ( vector & AFF_SLEEP         ) SLCAT ( buf, " sleep"         );
     if ( vector & AFF_SNEAK         ) SLCAT ( buf, " sneak"         );
     if ( vector & AFF_HIDE          ) SLCAT ( buf, " hide"          );
     if ( vector & AFF_CHARM         ) SLCAT ( buf, " charm"         );
     if ( vector & AFF_FLYING        ) SLCAT ( buf, " flying"        );
     if ( vector & AFF_PASS_DOOR     ) SLCAT ( buf, " pass door"     );
     if ( vector & AFF_BERSERK       ) SLCAT ( buf, " berserk"       );
     if ( vector & AFF_CALM          ) SLCAT ( buf, " calm"          );
     if ( vector & AFF_HASTE         ) SLCAT ( buf, " haste"         );
     if ( vector & AFF_PLAGUE        ) SLCAT ( buf, " plague"        );
     if ( vector & AFF_FEAR   	     ) SLCAT ( buf, " fear"          );
     if ( vector & AFF_REGENERATION  ) SLCAT ( buf, " regeneration"  );
     if ( vector & AFF_SWIM          ) SLCAT ( buf, " swim"          );
     if ( vector & AFF_POLY          ) SLCAT ( buf, " polymorph"     );
     return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char               *detect_bit_name ( int vector )
{
     static char         buf[1024];
     buf[0] = '\0';
     if ( vector & AFF_SHIELD        ) SLCAT (buf, " shield" 		);
     if ( vector & AFF_MUTE          ) SLCAT (buf, " mute" 		);
     if ( vector & AFF_SLOW          ) SLCAT (buf, " slow" 		);
     if ( vector & AFF_CONFUSION     ) SLCAT (buf, " confusion" 	);
     if ( vector & AFF_RALLY         ) SLCAT (buf, " rally" 		);
     if ( vector & DET_EVIL          ) SLCAT( buf, " evil"   		);
     if ( vector & DET_INVIS         ) SLCAT( buf, " invis"  		);
     if ( vector & DET_MAGIC         ) SLCAT( buf, " magic"  		);
     if ( vector & DET_HIDDEN        ) SLCAT( buf, " hidden" 		);
     if ( vector & DET_DARK_VISION   ) SLCAT( buf, " dark vision"   	);
     if ( vector & DET_INFRARED      ) SLCAT( buf, " infrared"      	);
     return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char		*protect_bit_name ( int vector )
{
     static char		buf[1024];

     buf[0] = '\0';
     if ( vector & PROT_EVIL          ) SLCAT ( buf, " evil"  	      );
     if ( vector & PROT_GOOD          ) SLCAT ( buf, " good"          );
     if ( vector & PROT_SANCTUARY     ) SLCAT ( buf, " sanctuary"     );
     if ( vector & PROT_ABSORB        ) SLCAT ( buf, " absorb"        );
     if ( vector & PROT_PHASED        ) SLCAT ( buf, " phased"        );
     return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Return ascii name of extra flags vector.
 */
char               *extra_bit_name ( int extra_flags )
{
     static char         buf[512];

     buf[0] = '\0';
     if ( extra_flags & ITEM_GLOW )          SLCAT ( buf, " glow" );
     if ( extra_flags & ITEM_HUM )           SLCAT ( buf, " hum" );
     if ( extra_flags & ITEM_DARK )          SLCAT ( buf, " dark" );
     if ( extra_flags & ITEM_LOCK )          SLCAT ( buf, " lock" );
     if ( extra_flags & ITEM_EVIL )          SLCAT ( buf, " evil" );
     if ( extra_flags & ITEM_INVIS )         SLCAT ( buf, " invis" );
     if ( extra_flags & ITEM_MAGIC )         SLCAT ( buf, " magic" );
     if ( extra_flags & ITEM_NODROP )        SLCAT ( buf, " nodrop" );
     if ( extra_flags & ITEM_BLESS )         SLCAT ( buf, " bless" );
     if ( extra_flags & ITEM_ANTI_GOOD )     SLCAT ( buf, " anti good" );
     if ( extra_flags & ITEM_ANTI_EVIL )     SLCAT ( buf, " anti evil" );
     if ( extra_flags & ITEM_ANTI_NEUTRAL )  SLCAT ( buf, " anti neutral" );
     if ( extra_flags & ITEM_NOREMOVE )      SLCAT ( buf, " noremove" );
     if ( extra_flags & ITEM_NO_SAC )        SLCAT ( buf, " no sacrifice" );
     if ( extra_flags & ITEM_NO_COND )       SLCAT ( buf, " no condition" );
     if ( extra_flags & ITEM_CONCEALED )     SLCAT ( buf, " concealed" );
     if ( extra_flags & ITEM_INVENTORY )     SLCAT ( buf, " inventory" );
     if ( extra_flags & ITEM_NOPURGE )       SLCAT ( buf, " nopurge" );
     if ( extra_flags & ITEM_VIS_DEATH )     SLCAT ( buf, " visibile at death" );
     if ( extra_flags & ITEM_ROT_DEATH )     SLCAT ( buf, " rot on death" );
     if ( extra_flags & ITEM_NODISP )        SLCAT ( buf, " no display" );
     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/* return ascii name of an act vector */
char               *act_bit_name ( int act_flags )
{
     static char         buf[512];

     buf[0] = '\0';

     if ( IS_SET ( act_flags, ACT_IS_NPC ) )
     {
          SLCAT ( buf, " npc" );
          if ( act_flags & ACT_SENTINEL )        SLCAT ( buf, " sentinel" 	);
          if ( act_flags & ACT_SCAVENGER )       SLCAT ( buf, " scavenger" 	);
          if ( act_flags & ACT_AGGRESSIVE )      SLCAT ( buf, " aggressive" 	);
          if ( act_flags & ACT_STAY_AREA )       SLCAT ( buf, " stay area" 	);
          if ( act_flags & ACT_WIMPY )           SLCAT ( buf, " wimpy" 		);
          if ( act_flags & ACT_PET )             SLCAT ( buf, " pet" 		);
          if ( act_flags & ACT_FOLLOWER )        SLCAT ( buf, " follower" 	);
          if ( act_flags & ACT_PRACTICE )        SLCAT ( buf, " practice" 	);
          if ( act_flags & ACT_UNDEAD )          SLCAT ( buf, " undead" 	);
          if ( act_flags & ACT_CLERIC )          SLCAT ( buf, " cleric" 	);
          if ( act_flags & ACT_MAGE )            SLCAT ( buf, " mage" 		);
          if ( act_flags & ACT_THIEF )           SLCAT ( buf, " thief" 		);
          if ( act_flags & ACT_WARRIOR )         SLCAT ( buf, " warrior"        );
          if ( act_flags & ACT_NOALIGN )         SLCAT ( buf, " no align" 	);
          if ( act_flags & ACT_NOPURGE )         SLCAT ( buf, " no purge" 	);
          if ( act_flags & ACT_IS_HEALER )       SLCAT ( buf, " healer" 	);
          if ( act_flags & ACT_TELEPOP )         SLCAT ( buf, " telepop" 	);
          if ( act_flags & ACT_UPDATE_ALWAYS )   SLCAT ( buf, " update always"  );
          if ( act_flags & ACT_NORANDOM )	 SLCAT ( buf, " no random items");
          if ( act_flags & ACT_NOQUEST )	 SLCAT ( buf, " no quest" 	);
          if ( act_flags & ACT_FOLLOWER )	 SLCAT ( buf, " follower" 	);
          if ( act_flags & ACT_SOLDIER )         SLCAT ( buf, " soldier" 	);
          if ( act_flags & ACT_SKILLMASTER )	 SLCAT ( buf, " skillmaster" 	);
     }
     else
     {
          SLCAT ( buf, " player" );
          if ( act_flags & PLR_BOUGHT_PET )      SLCAT ( buf, " owner" );
          if ( act_flags & PLR_AUTOASSIST )      SLCAT ( buf, " autoassist" );
          if ( act_flags & PLR_AUTOEXIT )        SLCAT ( buf, " autoexit" );
          if ( act_flags & PLR_AUTOLOOT )        SLCAT ( buf, " autoloot" );
          if ( act_flags & PLR_AUTOSAC )         SLCAT ( buf, " autosac" );
          if ( act_flags & PLR_AUTOGOLD )        SLCAT ( buf, " autogold" );
          if ( act_flags & PLR_AUTOSPLIT )       SLCAT ( buf, " autosplit" );
          if ( act_flags & PLR_HOLYLIGHT )       SLCAT ( buf, " holy light" );
          if ( act_flags & PLR_WIZINVIS )        SLCAT ( buf, " wizinvis" );
          if ( act_flags & PLR_CANLOOT )         SLCAT ( buf, " can loot corpse" );
          if ( act_flags & PLR_NOSUMMON )        SLCAT ( buf, " nosummon" );
          if ( act_flags & PLR_NOFOLLOW )        SLCAT ( buf, " nofollow" );
          if ( act_flags & PLR_FREEZE )          SLCAT ( buf, " frozen" );
          if ( act_flags & PLR_THIEF )           SLCAT ( buf, " thief" );
          if ( act_flags & PLR_KILLER )          SLCAT ( buf, " killer" );
          if ( act_flags & PLR_QUESTOR )	 SLCAT ( buf, " questing" );
          if ( act_flags & PLR_XINFO )		 SLCAT ( buf, " x-info" );
          if ( act_flags & PLR_CURSOR )		 SLCAT ( buf, " cursor" );
          if ( act_flags & PLR_CLOAK )		 SLCAT ( buf, " cloak" );
     }
     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *comm_bit_name ( int comm_flags )
{
     static char         buf[512];

     buf[0] = '\0';

     if ( comm_flags & COMM_QUIET )          SLCAT ( buf, " quiet" );
     if ( comm_flags & COMM_DEAF )           SLCAT ( buf, " deaf" );
     if ( comm_flags & COMM_NOWIZ )          SLCAT ( buf, " no_wiz" );
     if ( comm_flags & COMM_NOAUCTION )      SLCAT ( buf, " no_auction" );
     if ( comm_flags & COMM_NOGOSSIP )       SLCAT ( buf, " no_gossip" );
     if ( comm_flags & COMM_NOQUESTION )     SLCAT ( buf, " no_question" );
     if ( comm_flags & COMM_NOMUSIC )        SLCAT ( buf, " no_music" );
     if ( comm_flags & COMM_COMPACT )        SLCAT ( buf, " compact" );
     if ( comm_flags & COMM_FULLFIGHT )      SLCAT ( buf, " full battle" );
     if ( comm_flags & COMM_BRIEF )          SLCAT ( buf, " brief" );
     if ( comm_flags & COMM_PROMPT )         SLCAT ( buf, " prompt" );
     if ( comm_flags & COMM_COMBINE )        SLCAT ( buf, " combine" );
     if ( comm_flags & COMM_NOEMOTE )        SLCAT ( buf, " no_emote" );
     if ( comm_flags & COMM_NOSHOUT )        SLCAT ( buf, " no_shout" );
     if ( comm_flags & COMM_NOTELL )         SLCAT ( buf, " no_tell" );
     if ( comm_flags & COMM_NOCHANNELS )     SLCAT ( buf, " no_channels" );
     if ( comm_flags & COMM_NOCLANTELL )     SLCAT ( buf, " no_clantell" );
     if ( comm_flags & COMM_TELNET_GA )      SLCAT ( buf, " telnet_ga" );
     if ( comm_flags & COMM_DARKCOLOR )      SLCAT ( buf, " dark colors" );
     if ( comm_flags & COMM_NOFLASHY )       SLCAT ( buf, " no flashy" );
     if ( comm_flags & COMM_BEEP )           SLCAT ( buf, " beep" );
     if ( comm_flags & COMM_NODARKGREY )     SLCAT ( buf, " no dark grey" );
     if ( comm_flags & COMM_COLOUR )         SLCAT ( buf, " colour" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *imm_bit_name ( int imm_flags )
{
     static char         buf[512];

     buf[0] = '\0';

     if ( imm_flags & IMM_SUMMON )          SLCAT ( buf, " summon" );
     if ( imm_flags & IMM_CHARM )           SLCAT ( buf, " charm" );
     if ( imm_flags & IMM_MAGIC )           SLCAT ( buf, " magic" );
     if ( imm_flags & IMM_WEAPON )          SLCAT ( buf, " weapon" );
     if ( imm_flags & IMM_BASH )            SLCAT ( buf, " blunt" );
     if ( imm_flags & IMM_PIERCE )          SLCAT ( buf, " piercing" );
     if ( imm_flags & IMM_SLASH )           SLCAT ( buf, " slashing" );
     if ( imm_flags & IMM_FIRE )            SLCAT ( buf, " fire" );
     if ( imm_flags & IMM_COLD )            SLCAT ( buf, " cold" );
     if ( imm_flags & IMM_LIGHTNING )       SLCAT ( buf, " lightning" );
     if ( imm_flags & IMM_ACID )            SLCAT ( buf, " acid" );
     if ( imm_flags & IMM_POISON )          SLCAT ( buf, " poison" );
     if ( imm_flags & IMM_NEGATIVE )        SLCAT ( buf, " negative" );
     if ( imm_flags & IMM_HOLY )            SLCAT ( buf, " holy" );
     if ( imm_flags & IMM_ENERGY )          SLCAT ( buf, " energy" );
     if ( imm_flags & IMM_MENTAL )          SLCAT ( buf, " mental" );
     if ( imm_flags & IMM_DISEASE )         SLCAT ( buf, " disease" );
     if ( imm_flags & IMM_DROWNING )        SLCAT ( buf, " drowning" );
     if ( imm_flags & IMM_LIGHT )           SLCAT ( buf, " light" );
     if ( imm_flags & VULN_IRON )           SLCAT ( buf, " iron" );
     if ( imm_flags & VULN_WOOD )           SLCAT ( buf, " wood" );
     if ( imm_flags & VULN_SILVER )         SLCAT ( buf, " silver" );
     if ( imm_flags & VULN_STEEL )          SLCAT ( buf, " steel" );
     if ( imm_flags & VULN_ADAMANTITE )     SLCAT ( buf, " adamantite" );
     if ( imm_flags & VULN_MITHRIL )        SLCAT ( buf, " mithril" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *wear_bit_name ( int wear_flags )
{
     static char         buf[512];

     buf[0] = '\0';
     if ( wear_flags & ITEM_TAKE )          SLCAT ( buf, " take" );
     if ( wear_flags & ITEM_WEAR_FINGER )   SLCAT ( buf, " finger" );
     if ( wear_flags & ITEM_WEAR_NECK )     SLCAT ( buf, " neck" );
     if ( wear_flags & ITEM_WEAR_BODY )     SLCAT ( buf, " torso" );
     if ( wear_flags & ITEM_WEAR_HEAD )     SLCAT ( buf, " head" );
     if ( wear_flags & ITEM_WEAR_LEGS )     SLCAT ( buf, " legs" );
     if ( wear_flags & ITEM_WEAR_FEET )     SLCAT ( buf, " feet" );
     if ( wear_flags & ITEM_WEAR_HANDS )    SLCAT ( buf, " hands" );
     if ( wear_flags & ITEM_WEAR_ARMS )     SLCAT ( buf, " arms" );
     if ( wear_flags & ITEM_WEAR_SHIELD )   SLCAT ( buf, " shield" );
     if ( wear_flags & ITEM_WEAR_ABOUT )    SLCAT ( buf, " body" );
     if ( wear_flags & ITEM_WEAR_WAIST )    SLCAT ( buf, " waist" );
     if ( wear_flags & ITEM_WEAR_WRIST )    SLCAT ( buf, " wrist" );
     if ( wear_flags & ITEM_WIELD )         SLCAT ( buf, " wield" );
     if ( wear_flags & ITEM_HOLD )          SLCAT ( buf, " hold" );
     if ( wear_flags & ITEM_WEAR_PRIDE )    SLCAT ( buf, " pride" );
     if ( wear_flags & ITEM_WEAR_FACE )     SLCAT ( buf, " face" );
     if ( wear_flags & ITEM_WEAR_EARS )     SLCAT ( buf, " ears" );
     if ( wear_flags & ITEM_WEAR_FLOAT )    SLCAT ( buf, " float" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *form_bit_name ( int form_flags )
{
     static char         buf[512];

     buf[0] = '\0';
     if ( form_flags & FORM_POISON )          SLCAT ( buf, " poison" );
     else if ( form_flags & FORM_EDIBLE )     SLCAT ( buf, " edible" );
     if ( form_flags & FORM_MAGICAL )         SLCAT ( buf, " magical" );
     if ( form_flags & FORM_INSTANT_DECAY )   SLCAT ( buf, " instant_rot" );
     if ( form_flags & FORM_OTHER )           SLCAT ( buf, " other" );
     if ( form_flags & FORM_ANIMAL )          SLCAT ( buf, " animal" );
     if ( form_flags & FORM_SENTIENT )        SLCAT ( buf, " sentient" );
     if ( form_flags & FORM_UNDEAD )          SLCAT ( buf, " undead" );
     if ( form_flags & FORM_CONSTRUCT )       SLCAT ( buf, " construct" );
     if ( form_flags & FORM_MIST )            SLCAT ( buf, " mist" );
     if ( form_flags & FORM_INTANGIBLE )      SLCAT ( buf, " intangible" );
     if ( form_flags & FORM_BIPED )           SLCAT ( buf, " biped" );
     if ( form_flags & FORM_CENTAUR )         SLCAT ( buf, " centaur" );
     if ( form_flags & FORM_INSECT )          SLCAT ( buf, " insect" );
     if ( form_flags & FORM_SPIDER )          SLCAT ( buf, " spider" );
     if ( form_flags & FORM_CRUSTACEAN )      SLCAT ( buf, " crustacean" );
     if ( form_flags & FORM_WORM )            SLCAT ( buf, " worm" );
     if ( form_flags & FORM_BLOB )            SLCAT ( buf, " blob" );
     if ( form_flags & FORM_MAMMAL )          SLCAT ( buf, " mammal" );
     if ( form_flags & FORM_BIRD )            SLCAT ( buf, " bird" );
     if ( form_flags & FORM_REPTILE )         SLCAT ( buf, " reptile" );
     if ( form_flags & FORM_SNAKE )           SLCAT ( buf, " snake" );
     if ( form_flags & FORM_DRAGON )          SLCAT ( buf, " dragon" );
     if ( form_flags & FORM_AMPHIBIAN )       SLCAT ( buf, " amphibian" );
     if ( form_flags & FORM_FISH )            SLCAT ( buf, " fish" );
     if ( form_flags & FORM_COLD_BLOOD )      SLCAT ( buf, " cold_blooded" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *part_bit_name ( int part_flags )
{
     static char         buf[512];

     buf[0] = '\0';
     if ( part_flags & PART_HEAD )          SLCAT ( buf, " head" );
     if ( part_flags & PART_ARMS )          SLCAT ( buf, " arms" );
     if ( part_flags & PART_LEGS )          SLCAT ( buf, " legs" );
     if ( part_flags & PART_HEART )         SLCAT ( buf, " heart" );
     if ( part_flags & PART_BRAINS )        SLCAT ( buf, " brains" );
     if ( part_flags & PART_GUTS )          SLCAT ( buf, " guts" );
     if ( part_flags & PART_HANDS )         SLCAT ( buf, " hands" );
     if ( part_flags & PART_FEET )          SLCAT ( buf, " feet" );
     if ( part_flags & PART_FINGERS )       SLCAT ( buf, " fingers" );
     if ( part_flags & PART_EAR )           SLCAT ( buf, " ears" );
     if ( part_flags & PART_EYE )           SLCAT ( buf, " eyes" );
     if ( part_flags & PART_LONG_TONGUE )   SLCAT ( buf, " long_tongue" );
     if ( part_flags & PART_EYESTALKS )     SLCAT ( buf, " eyestalks" );
     if ( part_flags & PART_TENTACLES )     SLCAT ( buf, " tentacles" );
     if ( part_flags & PART_FINS )          SLCAT ( buf, " fins" );
     if ( part_flags & PART_WINGS )         SLCAT ( buf, " wings" );
     if ( part_flags & PART_TAIL )          SLCAT ( buf, " tail" );
     if ( part_flags & PART_CLAWS )         SLCAT ( buf, " claws" );
     if ( part_flags & PART_FANGS )         SLCAT ( buf, " fangs" );
     if ( part_flags & PART_HORNS )         SLCAT ( buf, " horns" );
     if ( part_flags & PART_SCALES )        SLCAT ( buf, " scales" );
     if ( part_flags & PART_HOOFS )         SLCAT ( buf, " hooves" );
     if ( part_flags & PART_NECK )          SLCAT ( buf, " neck" );
     if ( part_flags & PART_WAIST )         SLCAT ( buf, " waist" );
     if ( part_flags & PART_WRIST )         SLCAT ( buf, " wrist" );
     if ( part_flags & PART_FACE )          SLCAT ( buf, " face" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *weapon_bit_name ( int weapon_flags )
{
     static char        buf[512];

     buf[0] = '\0';
     if ( weapon_flags & WEAPON_FLAMING )      SLCAT ( buf, " {rflaming{x" );
     if ( weapon_flags & WEAPON_ACID )         SLCAT ( buf, " {Gacidic{x" );
     if ( weapon_flags & WEAPON_LIGHTNING )    SLCAT ( buf, " {Yelectric{x" );
     if ( weapon_flags & WEAPON_FROST )        SLCAT ( buf, " {Bfrost{x" );
     if ( weapon_flags & WEAPON_VAMPIRIC )     SLCAT ( buf, " {Mvampiric{x" );
     if ( weapon_flags & WEAPON_SHARP )        SLCAT ( buf, " {Wsharp{x" );
     if ( weapon_flags & WEAPON_VORPAL )       SLCAT ( buf, " {mvorpal{x" );
     if ( weapon_flags & WEAPON_TWO_HANDS )    SLCAT ( buf, " two-handed" );
     if ( weapon_flags & WEAPON_POISON )       SLCAT ( buf, " {gpoisoned{x" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

char               *off_bit_name ( int off_flags )
{
     static char         buf[512];

     buf[0] = '\0';

     if ( off_flags & OFF_AREA_ATTACK )      SLCAT ( buf, " area attack" );
     if ( off_flags & OFF_BACKSTAB )         SLCAT ( buf, " backstab" );
     if ( off_flags & OFF_BASH )             SLCAT ( buf, " bash" );
     if ( off_flags & OFF_BERSERK )          SLCAT ( buf, " berserk" );
     if ( off_flags & OFF_DISARM )           SLCAT ( buf, " disarm" );
     if ( off_flags & OFF_DODGE )            SLCAT ( buf, " dodge" );
     if ( off_flags & OFF_FADE )             SLCAT ( buf, " fade" );
     if ( off_flags & OFF_FAST )             SLCAT ( buf, " fast" );
     if ( off_flags & OFF_KICK )             SLCAT ( buf, " kick" );
     if ( off_flags & OFF_KICK_DIRT )        SLCAT ( buf, " kick_dirt" );
     if ( off_flags & OFF_PARRY )            SLCAT ( buf, " parry" );
     if ( off_flags & OFF_RESCUE )           SLCAT ( buf, " rescue" );
     if ( off_flags & OFF_TAIL )             SLCAT ( buf, " tail" );
     if ( off_flags & OFF_TRIP )             SLCAT ( buf, " trip" );
     if ( off_flags & OFF_CRUSH )            SLCAT ( buf, " crush" );
     if ( off_flags & ASSIST_ALL )           SLCAT ( buf, " assist_all" );
     if ( off_flags & ASSIST_ALIGN )         SLCAT ( buf, " assist_align" );
     if ( off_flags & ASSIST_RACE )          SLCAT ( buf, " assist_race" );
     if ( off_flags & ASSIST_PLAYERS )       SLCAT ( buf, " assist_players" );
     if ( off_flags & ASSIST_GUARD )         SLCAT ( buf, " assist_guard" );
     if ( off_flags & ASSIST_VNUM )          SLCAT ( buf, " assist_vnum" );
     if ( off_flags & OFF_RACIST )           SLCAT ( buf, " racist" );

     return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

