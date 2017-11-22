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

/* Command procedures */

DECLARE_DO_FUN(do_emote);

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
extern char *target_name;

/*
 * Spell functions.
 */

/* Thanks to CthulhuMud for summon familiar */
void spell_summon_familiar ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA *mob;

     int mob_num;
     int i, number, type, bonus;

     /* No previous pet... */

     if( ch->pet != NULL )
     {
          send_to_char( "You already have a pet.\n\r", ch );
          return;
     }

     /* Create the mob... */

     mob = create_mobile ( get_mob_index(MOB_VNUM_FAMILIAR) );
     mob->level = ch->level -4;

     mob->max_hit = mob->level * 8 +
          number_range ( mob->level * mob->level / 4, mob->level * mob->level );

     mob->max_hit *= .9;
     mob->hit = mob->max_hit;
     mob->max_mana = 100 + dice ( mob->level, 10 );
     mob->mana = mob->max_mana;
     // No armor if not wearing armor
     // If, perchance, armor is wanted, this function would need rewritten
     // for the new AC system.
     //    for ( i = 0; i < 3; i++ )
     //        mob->armor[i] = interpolate ( mob->level, 100, -100 );
     //    mob->armor[3] = interpolate ( mob->level, 100, 0 );
     //
     number = (mob->level / 15)+1;
     type = (number * 3) + ( (mob->level - ( (number-1)*10 ))/3 );
     bonus = 0;

     mob->damage[DICE_NUMBER] = number;
     mob->damage[DICE_TYPE] = type;
     mob->damage[DICE_BONUS] = bonus;

     for ( i = 0; i < MAX_STATS; i++ )
          mob->perm_stat[i] = 11 + mob->level / 4;

     /* Work out what sort of familier they get... */

     mob_num = dice(1, 4) + 2;

     if (IS_EVIL(ch))
     {
          mob_num -= 2;
     }
     else if (IS_GOOD(ch))
     {
          mob_num += 2;
     }

     /* 1 in 128 chance of getting a raw one... */

     if (number_bits(8) == 0)
     {
          mob_num = -1;
     }

     /* Taylor description and names... */

     switch (mob_num)
     {

     default:
          break;

     case 1:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("homunlcus disgusting green");
          mob->short_descr = str_dup("a disgusting green homunlcus");
          mob->long_descr = str_dup("A disgusting green homunlcus\n");
          mob->description = str_dup("It looks like the stories about the revolting things these abominations are\n made from are true. Yuk!\n");

          mob->sex = SEX_NEUTRAL;
          mob->race=15;
          mob->alignment = -1000;

          break;

     case 2:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("imp short red");
          mob->short_descr = str_dup("a short red imp");
          mob->long_descr = str_dup("A short, red imp\n");
          mob->description = str_dup("He gives you a nasty look!\n");

          mob->sex = SEX_MALE;
          mob->race=20;
          mob->alignment = -750;

          SET_BIT (mob->affected_by, AFF_FLYING );

          break;

     case 3:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("python long");
          mob->short_descr = str_dup("a long python");
          mob->long_descr = str_dup("A long python\n");
          mob->description = str_dup("It seems quite friendly.\n");

          mob->sex = SEX_NEUTRAL;
          mob->race=27;
          mob->alignment = -300;

          break;

     case 4:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("toad grey");
          mob->short_descr = str_dup("a grey toad");
          mob->long_descr = str_dup("A grey toad\n");
          mob->description = str_dup("He is covered in ugly warts.\n");

          mob->sex = SEX_MALE ;
          mob->race=21;
          mob->alignment = -100;

          break;

     case 5:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("parrot green");
          mob->short_descr = str_dup("a green parrot");
          mob->long_descr = str_dup("A green parrot\n");
          mob->description = str_dup("He looks like he belonged to a sailor.\n");

          mob->sex = SEX_MALE;
          mob->race=28;
          mob->alignment = 100;

          SET_BIT ( mob->affected_by, AFF_FLYING );

          break;

     case 6:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("cat black");
          mob->short_descr = str_dup("a black cat");
          mob->long_descr = str_dup("A black cat\n");
          mob->description = str_dup("She looks quite refined.\n");

          mob->sex = SEX_FEMALE;
          mob->race=12;
          mob->alignment = 300;

          break;

     case 7:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("bird blue beautiful");
          mob->short_descr = str_dup("a blue bird");
          mob->long_descr = str_dup("A beautiful blue bird\n");
          mob->description = str_dup("She looks very happy (and somewhat like a chicken)!\n");

          mob->sex = SEX_FEMALE;

          mob->race=28;
          mob->alignment = 750;

          SET_BIT ( mob->affected_by, AFF_FLYING );

          break;

     case 8:
          free_string(mob->name);
          free_string(mob->short_descr);
          free_string(mob->long_descr);
          free_string(mob->description);

          mob->name = str_dup("cherub cute");
          mob->short_descr = str_dup("a cute cherub");
          mob->long_descr = str_dup("A cute cherub\n");
          mob->description = str_dup("He looks very beautiful, and has little wings on his back.\n");

          mob->sex = SEX_MALE;
          mob->race=2;
          mob->alignment = 1000;

          SET_BIT ( mob->affected_by, AFF_FLYING );

          break;

     }

     /* Deliver the familier */

     char_to_room( mob, ch->in_room );

     act( "$N steps out of the shadows...", ch, NULL, mob, TO_ROOM );
     act( "$N steps out of the shadows...", ch, NULL, mob, TO_CHAR );

     /* Enslave the familier... */
     SET_BIT ( mob->affected_by, AFF_CHARM);

     SET_BIT(mob->act, ACT_FOLLOWER);

     add_follower( mob, ch );
     mob->leader = ch;
     mob->master = ch;
     ch->pet = mob;

     /* For a little flavor... */

     do_emote( mob, "looks at you expectantly." );
     return;
}

void spell_meteor ( int sn, int level, CHAR_DATA * ch,
                    void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = (spell_calc(ch, sn) + spell_calc(ch, sn) ) /2;

     check_damage_obj ( victim, NULL, 5, DAM_OTHER );
     damage ( ch, victim, dam, sn, DAM_OTHER, TRUE );
     return;
}

void spell_boil ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = (spell_calc(ch, sn) + spell_calc(ch, sn) ) /2;

     if ( saves_spell ( level, victim ) )
          dam /= 2;
     check_damage_obj ( victim, NULL, 1, DAM_FIRE );
     damage ( ch, victim, dam, sn, DAM_FIRE, TRUE );
     return;
}

void spell_dehydrate ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;

     dam = (spell_calc(ch, sn) + spell_calc(ch, sn) ) /2;

     if ( saves_spell ( level, victim ) )
          dam /= 2;

     if (!IS_NPC(victim) )
          ch->pcdata->condition[COND_THIRST] = 0;

     sound ("magic9.wav", victim);
     if ( victim != ch)
          sound ("magic9.wav", ch);

     damage ( ch, victim, dam, sn, DAM_NEGATIVE, TRUE );
     return;
}

void spell_meteor_shower ( int sn, int level, CHAR_DATA * ch, void *vo )
{
     CHAR_DATA          *victim = ( CHAR_DATA * ) vo;
     int                 dam;
     int                 count;
     int                 i;

     count = ch->level/10;
     if (count < 1)
          count = 1;

     for (i = 0; i < count ; i++)
     {
          dam = spell_calc (ch, sn);
          check_damage_obj ( victim, NULL, 1, DAM_OTHER );
          damage ( ch, victim, dam, sn, DAM_OTHER, TRUE );
     }

     return;
}

