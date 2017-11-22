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

/* This file is the non-spell magical stuff. See spells.c for spells. */

#include "everything.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN ( do_help );

/* RT spells and skills show the players spells (or skills) */

void do_spells ( CHAR_DATA * ch, char *argument )
{
     char                spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
     char                spell_columns[LEVEL_HERO];
     int                 sn, lev, mana, filter;
     bool                found = FALSE;
     char                buf[MAX_STRING_LENGTH];
     BUFFER		*outbuf;

     if ( IS_NPC ( ch ) )
          return;

     outbuf = buffer_new(1000);

     if ( argument == NULL || argument[0] == '\0' )      /* This character's own list. Normal. */
     {
          filter = -1;
     }
     else
     {
          filter = class_lookup ( argument );
          if ( filter == -1 )
          {
               send_to_char ( "That's not a class.\n\r", ch );
               buffer_free(outbuf);
               return;
          }
          else
          {
               bprintf( outbuf, "Spells For %s", class_table[filter].name );
          }
     }

	 /* initilize data */
     for ( lev = 0; lev < LEVEL_HERO; lev++ )
     {
          spell_columns[lev] = 0;
          spell_list[lev][0] = '\0';
     }

     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( skill_table[sn].name == NULL )
               break;

          if ( ( ( (filter == -1) && (skill_table[sn].skill_level[ch->pcdata->pclass] <= LEVEL_HERO) ) ||
                 ( (filter >= 0) && (skill_table[sn].skill_level[filter] <= LEVEL_HERO ) ) )
               && skill_table[sn].spell_fun != spell_null )
          {
               found = TRUE;

               if (filter == -1)
                    lev = skill_table[sn].skill_level[ch->pcdata->pclass];
               else
                    lev = skill_table[sn].skill_level[filter];

               if ( ( ch->level < lev ) || (filter >= 0) )
                    SNP ( buf, "%-18s  n/a      ", skill_table[sn].name );
               else
               {
                    mana = UMAX ( skill_table[sn].min_mana, 100 / ( 2 + ch->level - lev ) );
                    SNP ( buf, "%-18s  %3d mana  ", skill_table[sn].name, mana );
               }

               if ( spell_list[lev][0] == '\0' )
                    SNP ( spell_list[lev], "\n\rLevel %2d: %s", lev, buf );
               else		/* append */
               {
                    if ( ++spell_columns[lev] % 2 == 0 )
                         SLCAT ( spell_list[lev], "\n\r          " );
                    SLCAT ( spell_list[lev], buf );
               }
          }
     }
	 /* return results */
     if ( !found )
     {
          send_to_char ( "No spells found.\n\r", ch );
     }
     else
     {
          for ( lev = 0; lev < LEVEL_HERO; lev++ )
               if ( spell_list[lev][0] != '\0' )
                    buffer_strcat ( outbuf, spell_list[lev] );
          buffer_strcat ( outbuf, "\n\r" );
          page_to_char ( outbuf->data, ch );
     }

     buffer_free(outbuf);
     return;
}

/*
 * Utter mystical words for an sn.
 * Modified to set npc "class" by act flags - Lotherius
 */
void say_spell ( CHAR_DATA * ch, int sn )
{
     char                buf[MAX_STRING_LENGTH];
     char                buf2[MAX_STRING_LENGTH];
     CHAR_DATA          *rch;
     char               *pName;
     int                 iSyl;
     int                 length;
     int		 sclass;
     int		 oclass;

     struct syl_type
     {
          char               *old;
          char               *new;
     };

     static const struct syl_type syl_table[] =
     {
          {" ",        " "     },
          {"ar",       "octa"  },
          {"au",       "erae"  },
          {"bless",    "dise"  },
          {"blind",    "nosii" },
          {"bur",      "mosa"  },
          {"cu",       "sucri" },
          {"de",       "oculo" },
          {"en",       "unso"  },
          {"light",    "onis"  },
          {"lo",       "hi"    },
          {"mor",      "zak"   },
          {"move",     "sido"  },
          {"ness",     "dimo"  },
          {"ning",     "illa"  },
          {"per",      "duda"  },
          {"ra",       "gru"   },
          {"fresh",    "ima"   },
          {"re",       "lamo"  },
          {"son",      "sabru" },
          {"tect",     "ocri"  },
          {"tri",      "cula"  },
          {"ven",      "nofo"  },
          {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
          {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
          {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
          {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
          {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
          {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
          {"y", "l"}, {"z", "k"},
          {"",         ""      }
     };

     buf[0] = '\0';
     for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
     {
          for ( iSyl = 0; ( length = strlen ( syl_table[iSyl].old ) ) != 0; iSyl++ )
          {
               if ( !str_prefix ( syl_table[iSyl].old, pName ) )
               {
                    SLCAT ( buf, syl_table[iSyl].new );
                    break;
               }
          }

          if ( length == 0 )
               length = 1;
     }
     SNP ( buf2, "$n intones the words, '%s'.", buf );
     SNP ( buf, "$n intones the words, '%s'.", skill_table[sn].name );
     
     if ( !IS_NPC ( ch ) )
          sclass = ch->pcdata->pclass;
     else
     {
          if ( IS_SET(ch->act, ACT_MAGE) )
               sclass = class_lookup ( "mage" );
          else if ( IS_SET ( ch->act, ACT_THIEF ) )
               sclass = class_lookup ( "thief" );
          else if ( IS_SET ( ch->act, ACT_CLERIC ) )
          {
               if ( ch->alignment > 0 )
                    sclass = class_lookup ( "avenger" );
               else
                    sclass = class_lookup ( "defiler" );
          }
          else
               sclass = class_lookup ( "warrior" );
     }
     
     for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
     {
          if ( rch != ch )
          {
               if ( !IS_NPC ( rch ) )
                    oclass = rch->pcdata->pclass;
               else
               {
                    if ( IS_SET( rch->act, ACT_MAGE) )
                         oclass = class_lookup ( "mage" );
                    else if ( IS_SET ( rch->act, ACT_THIEF ) )
                         oclass = class_lookup ( "thief" );
                    else if ( IS_SET ( rch->act, ACT_CLERIC ) )
                    {
                         if ( ch->alignment > 0 )
                              oclass = class_lookup ( "avenger" );
                         else
                              oclass = class_lookup ( "defiler" );
                    }
                    else
                         oclass = class_lookup ( "warrior" );
               }               
               act ( sclass == oclass ? buf : buf2, ch, NULL, rch, TO_VICT );
          }
     }
     return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell ( int level, CHAR_DATA * victim )
{
     int                 save;

     save = 50 + ( victim->level - level - victim->saving_throw ) * 5;
     if ( IS_AFFECTED ( victim, AFF_BERSERK ) )
          save += victim->level / 2;
     save = URANGE ( 5, save, 95 );
     return number_percent (  ) < save;
}

/* RT save for dispels */

bool saves_dispel ( int dis_level, int spell_level, int duration )
{
     int                 save;

     if ( duration == -1 )
          spell_level += 2;
     /* very hard to dispel permanent effects */
     save = 50 + ( spell_level - dis_level ) * 5;
     save = URANGE ( 5, save, 95 );
     return number_percent (  ) < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel ( int dis_level, CHAR_DATA * victim, int sn )
{
     AFFECT_DATA        *af;

     if ( is_affected ( victim, sn ) )
     {
          for ( af = victim->affected; af != NULL; af = af->next )
          {
               if ( af->type == sn )
               {
                    if ( !saves_dispel ( dis_level, af->level, af->duration ) )
                    {
                         affect_strip ( victim, sn );
                         if ( skill_table[sn].msg_off )
                         {
                              send_to_char ( skill_table[sn].msg_off, victim );
                              send_to_char ( "\n\r", victim );
                         }
                         return TRUE;
                    }
                    else
                         af->level--;
               }
          }
     }
     return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost ( CHAR_DATA * ch, int min_mana, int level )
{
     if ( ch->level + 2 == level )
          return 1000;
     return UMAX ( min_mana, ( 100 / ( 2 + ch->level - level ) ) );
}

/*
 * for standardizing spell damages.
 * Yes, I know it's not popular, but many spells in Rom had no relation
 * between how POWERFUL they were and their LEVEL. And they should.
 *
 */
int spell_calc ( CHAR_DATA * ch, int sn )
{
     int   dam;
     int   level;

     // Set's the spell's strength to match its level
     // 
     // Prevents necessity of changing a spell when the level is changed.
     
     if ( !IS_NPC(ch) )
          level = skill_table[sn].skill_level[ch->pcdata->pclass];
     else
     {
          if ( IS_SET(ch->act,ACT_CLERIC) && ch->alignment >= 1 )
               level = skill_table[sn].skill_level[CLASS_AVENGER];
          else if ( IS_SET(ch->act, ACT_CLERIC) && ch->alignment <= 0 )
               level = skill_table[sn].skill_level[CLASS_DEFILER];
          else if ( IS_SET(ch->act, ACT_UNDEAD) )
               level = skill_table[sn].skill_level[CLASS_DEFILER] + 5;
          else if ( IS_SET(ch->act, ACT_MAGE) )
               level = skill_table[sn].skill_level[CLASS_MAGE];
          else
               level = 5;
     }

     dam = number_range( 1, level*2);

     /* IS_NPC must come before class to avoid mobs getting class check */
     /* If you're not a chaosmage, you get a more average damage */

     if (!IS_NPC(ch) && ch->pcdata->pclass != CLASS_CHAOSMAGE)
     {
          dam = (dam + number_range(level/2, level*2) ) / 2;
     }

     /* Give a bonus for the character's own level, ramp it up as levels increase */
     if ( ch->level <= 10 )
          dam += number_fuzzy(number_range(ch->level/2, ch->level *1.5));
     else if (ch->level <= 25 )
          dam += number_fuzzy(number_range(ch->level/2, ch->level *1.75));
     else if ( ch->level <= 40 )
          dam += number_fuzzy(number_range(ch->level/2, ch->level *2));
     else if ( ch->level <= 80 )
          dam += number_fuzzy(number_range(ch->level/2, ch->level *2.25));
     else
          dam += number_fuzzy(number_range(ch->level/2, ch->level *2.5));

     /* Now another bonus so that higher level spells do better */
     if ( level > 50 )
          dam += number_fuzzy(number_range(level/4, level/2));

     /* Give an intelligence bonus on the spell for mages, wisdom for clerics */

     if (!IS_NPC(ch) )
     {
          float bonus = 0;
          if (ch->pcdata->pclass == CLASS_AVENGER || ch->pcdata->pclass == CLASS_DEFILER)
               bonus = wis_app[get_curr_stat ( ch, STAT_WIS )].cspell;
          else if (ch->pcdata->pclass == CLASS_MAGE || ch->pcdata->pclass == CLASS_CHAOSMAGE)
               bonus = int_app[get_curr_stat ( ch, STAT_INT )].mspell;
          bonus = number_fuzzy(bonus);
          if (bonus != 0)
               dam +=dam * (float) (bonus/100);
     }

     return dam;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 * Now NPC's can "cast" spells :)
 */
extern char *target_name;

void do_cast ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *component = NULL;
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     void               *vo;
     int                 mana;
     int                 sn;
     int		 nclass;

     target_name = one_argument ( argument, arg1 );
     one_argument ( target_name, arg2 );

     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Cast which what where?\n\r", ch );
          return;
     }
     
     /* Set NCLASS to player's class or NPC's act flag */
     if ( IS_NPC ( ch ) )
     {
          if ( IS_SET(ch->act, ACT_MAGE) )
               nclass = class_lookup ( "mage" );
          else if ( IS_SET ( ch->act, ACT_THIEF ) )
               nclass = class_lookup ( "thief" );
          else if ( IS_SET ( ch->act, ACT_CLERIC ) )
          { 
               if ( ch->alignment > 0 )
                    nclass = class_lookup ( "avenger" );
               else
                    nclass = class_lookup ( "defiler" );
          }
          else
               nclass = class_lookup ( "warrior" );
     }
     else
          nclass = ch->pcdata->pclass;
     

     if ( ( sn = skill_lookup ( arg1 ) ) < 1
          || ( ch->level < skill_table[sn].skill_level[nclass] )
          || ( ch->pcdata->learned[sn] < 1 ) )
     {
          if ( IS_NPC ( ch ) )
               bugf ( "A mob tried casting a spell it doesn't know.", arg1, ch->name );
          send_to_char ( "You don't know any spells of that name.\n\r", ch );
          return;
     }
     
     if ( skill_table[sn].spell_fun == spell_null )
     {
          send_to_char ( "Didn't your teachers tell you that wasn't a spell?\n\r", ch );
          return;
     }

     if ( is_affected ( ch, skill_lookup ( "mute" ) ) && sn != skill_lookup ( "cancellation" ) &&
          !is_affected ( ch, skill_lookup ( "vocalize" ) ) )
     {
          send_to_char ( "Mute people may only cast 'cancellation'.\n\r", ch );
          return;
     }

     if ( ch->position < skill_table[sn].minimum_position )
     {
          send_to_char ( "You can't concentrate enough.\n\r", ch );
          return;
     }

	 /* Zeran - gonna override components for immortals */
     if ( !IS_IMMORTAL ( ch ) )
     {
          if ( str_cmp ( skill_table[sn].component, "" ) )
               component = get_obj_carry ( ch, skill_table[sn].component, NULL );
          if ( ( str_cmp ( skill_table[sn].component, "" ) ) &&  ( !component ) )
          {
               char                cbuf[80];

               SNP ( cbuf, "You need to have the component '%s' to cast this.\n\r",
                         skill_table[sn].component );
               send_to_char ( cbuf, ch );
               return;
          }

          if ( str_cmp ( skill_table[sn].component, "" ) && ( component->item_type != ITEM_COMPONENT ) )
          {
               char                cbuf[80];

               SNP ( cbuf, "Your '%s' is not the correct component.\n\r", component->short_descr );
               send_to_char ( cbuf, ch );
               return;
          }
     }
     /* end IMMORTAL checking block for component */
     
     if ( ch->level + 2 == skill_table[sn].skill_level[nclass] )
          mana = 50;
     else
          mana = UMAX ( skill_table[sn].min_mana, 100 / 
                        ( 2 + ch->level - skill_table[sn].skill_level[nclass] ) );

     /*
      * Locate targets.
      */
     victim = NULL;
     obj = NULL;
     vo = NULL;

     switch ( skill_table[sn].target )
     {
     default:
          bugf ( "Do_cast: bad target for sn %d.", sn );
          return;

     case TAR_IGNORE:
          break;

     case TAR_CHAR_OFFENSIVE:
          if ( IS_AFFECTED ( ch, AFF_FEAR ) )
          {
               send_to_char ( "You are too scared to attack anyone...\n\r", ch );
               return;
          }
          if ( arg2[0] == '\0' )
          {
               if ( ( victim = ch->fighting ) == NULL )
               {
                    send_to_char ( "Cast the spell on whom?\n\r", ch );
                    return;
               }
          }
          else
          {
               if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL )
               {
                    send_to_char ( "Funny, where'd they go?\n\r", ch );
                    return;
               }
          }

		  /*
		   if ( ch == victim )
		   {
		   send_to_char( "You can't do that to yourself.\n\r", ch );
		   return;
		   }
		   */

          if ( !IS_NPC ( ch ) )
          {

               if ( is_safe_spell ( ch, victim, FALSE ) && victim != ch )
                    return;
               if ( !IS_NPC ( victim ) && is_safe ( ch, victim, TRUE ) )
                    return;
          }

          if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
          {
               send_to_char( "But you love your master so much!\n\r", ch );
               return;
          }
          vo = ( void * ) victim;
          break;

     case TAR_CHAR_DEFENSIVE:
          if ( arg2[0] == '\0' )
          {
               victim = ch;
          }
          else
          {
               if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL )
               {
                    send_to_char ( "They aren't here.\n\r", ch );
                    return;
               }
          }

          vo = ( void * ) victim;
          break;

     case TAR_CHAR_SELF:
          if ( arg2[0] != '\0' && !is_name ( arg2, ch->name ) )
          {
               send_to_char( "You cannot cast this spell on another.\n\r", ch );
               return;
          }
          victim = ch;
          vo = ( void * ) ch;
          break;

     case TAR_OBJ_INV:
          if ( arg2[0] == '\0' )
          {
               send_to_char ( "What should the spell be cast upon?\n\r", ch );
               return;
          }

          if ( ( obj = get_obj_carry ( ch, arg2, NULL ) ) == NULL )
          {
               send_to_char ( "You are not carrying that.\n\r", ch );
               return;
          }

          vo = ( void * ) obj;
          break;
     }

     if ( ch->mana < mana )
     {
          send_to_char ( "You don't have enough mana.\n\r", ch );
          return;
     }

     if ( str_cmp ( skill_table[sn].name, "ventriloquate" ) &&
          !is_affected ( ch, skill_lookup ( "mute" ) ) )
          say_spell ( ch, sn );

     WAIT_STATE ( ch, skill_table[sn].beats );

     if ( number_percent (  ) > get_skill ( ch, sn ) )
     {
          send_to_char ( "You lost your concentration.\n\r", ch );
          if ( !IS_NPC ( ch ) )
               check_improve ( ch, sn, FALSE, 1 );
          ch->mana -= mana / 2;
          /* Zeran - override component section for immortals */
          if ( !IS_IMMORTAL ( ch ) )
          {
               if ( str_cmp ( skill_table[sn].component, "" ) )
               {
                    send_to_char ( "The spell component dissolves!\n\r", ch );
                    extract_obj ( component );
               }
          }
     }
     else
     {
          ch->mana -= mana;
          /* Zeran - override component section for immortals */
          if ( !IS_IMMORTAL ( ch ) )
          {
               if ( str_cmp ( skill_table[sn].component, "" ) )
               {
                    send_to_char ( "Your component is consumed by the spell.\n\r", ch );
                    extract_obj ( component );
               }
          }
          if ( ( skill_table[sn].target == TAR_CHAR_OFFENSIVE ) && IS_PROTECTED ( victim, PROT_ABSORB ) &&
               ( skill_lookup ( "dispel magic" ) != sn ) )
          {
               if ( ( number_percent (  ) - 5 * ( ch->level - victim->level ) ) > get_skill( ch, sn ) / 2 )
               {
                    char                messbuf[80];
                    // Needs a better message... geez, Z...
                    SNP ( messbuf, "You absorb the magic of %s's offensive spell!\n\r", ch->name );
                    send_to_char ( messbuf, victim );
                    SNP ( messbuf, "%s absorbs your offensive spell!\n\r",
                              PERS ( victim, ch ) );
                    send_to_char ( messbuf, ch );
               }
               else
                    ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, vo );
          }
          else
               ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, vo );
          check_improve ( ch, sn, TRUE, 1 );
     }
     if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE && victim != ch && victim->master != ch )
     {
          CHAR_DATA          *vch;
          CHAR_DATA          *vch_next;

          for ( vch = ch->in_room->people; vch; vch = vch_next )
          {
               vch_next = vch->next_in_room;
               if ( victim == vch && victim->fighting == NULL && skill_lookup ( "fear" ) != sn )
               {
                    multi_hit ( victim, ch, TYPE_UNDEFINED );
                    break;
               }
          }
     }
     return;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell ( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
     void               *vo;

     if ( sn <= 0 )
          return;

     if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
     {
          bugf ( "Obj_cast_spell: bad sn %d.", sn );
          return;
     }
     switch ( skill_table[sn].target )
     {
     default:
          bugf ( "Obj_cast_spell: bad target for sn %d.", sn );
          return;
     case TAR_IGNORE:
          vo = NULL;
          break;
     case TAR_CHAR_OFFENSIVE:
          if ( victim == NULL )
               victim = ch->fighting;
          if ( victim == NULL )
          {
               send_to_char ( "You can't do that.\n\r", ch );
               return;
          }
          if ( is_safe_spell ( ch, victim, FALSE ) && ch != victim )
               return;
          vo = ( void * ) victim;
          break;
     case TAR_CHAR_DEFENSIVE:
          if ( victim == NULL )
               victim = ch;
          vo = ( void * ) victim;
          break;
     case TAR_CHAR_SELF:
          vo = ( void * ) ch;
          break;
     case TAR_OBJ_INV:
          if ( obj == NULL )
          {
               send_to_char ( "You can't do that.\n\r", ch );
               return;
          }
          vo = ( void * ) obj;
          break;
     }

     target_name = "";
     ( *skill_table[sn].spell_fun ) ( sn, level, ch, vo );

     if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE && victim != ch && victim->master != ch )
     {
          CHAR_DATA          *vch;
          CHAR_DATA          *vch_next;

          for ( vch = ch->in_room->people; vch; vch = vch_next )
          {
               vch_next = vch->next_in_room;
               if ( victim == vch && victim->fighting == NULL )
               {
                    multi_hit ( victim, ch, TYPE_UNDEFINED );
                    break;
               }
          }
     }
     return;
}
