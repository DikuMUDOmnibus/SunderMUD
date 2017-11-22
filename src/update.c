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
DECLARE_DO_FUN ( do_quit );

/*
 * Local functions.
 */
int hit_gain        args ( ( CHAR_DATA * ch ) );
int mana_gain       args ( ( CHAR_DATA * ch ) );
int move_gain       args ( ( CHAR_DATA * ch ) );
void mobile_update  args ( ( void ) );
void weather_update args ( ( void ) );
void char_update    args ( ( void ) );
void obj_update     args ( ( void ) );
void aggr_update    args ( ( void ) );
void quest_update   args ( ( void ) );	/* Vassago - quest.c */

FILE               *time_file;

/*
 * Advancement stuff.
 */
void advance_level ( CHAR_DATA * ch )
{
     int                 add_hp;
     int                 add_mana;
     int                 add_move;
     int                 add_prac;
     
     if ( IS_NPC ( ch ) )
          return;		/* We haven't added advancement of NPC's yet. */

     ch->pcdata->last_level = ( ch->played + ( int ) ( current_time - ch->logon ) ) / 3600;

     add_hp = con_app[get_curr_stat ( ch, STAT_CON )].hitp +
          number_range ( class_table[ch->pcdata->pclass].hp_min, class_table[ch->pcdata->pclass].hp_max );

     add_mana = number_range ( 5, ( 4 * get_curr_stat ( ch, STAT_INT ) +
                                    get_curr_stat ( ch, STAT_WIS ) ) / 5 );

     if ( !class_table[ch->pcdata->pclass].fMana )
          add_mana /= 2;

     add_move = number_range ( 1, ( get_curr_stat ( ch, STAT_CON )+
                                    get_curr_stat ( ch, STAT_DEX ) ) / 6 );
     add_prac = wis_app[get_curr_stat ( ch, STAT_WIS )].practice;

     add_hp = add_hp * 9 / 10;
     add_mana = add_mana * 9 / 10;
     add_move = add_move * 9 / 10;

     add_hp = UMAX ( 1, add_hp );
     add_mana = UMAX ( 1, add_mana );
     add_move = UMAX ( 6, add_move );

     add_hp *= 4; // Boost it to match the more powerful mobs.
     
     ch->max_hit += add_hp;
     ch->max_mana += add_mana;
     ch->max_move += add_move;
     ch->pcdata->practice += add_prac;
     ch->pcdata->train += 1;

     ch->pcdata->perm_hit += add_hp;
     ch->pcdata->perm_mana += add_mana;
     ch->pcdata->perm_move += add_move;

     if ( !IS_NPC ( ch ) )
          REMOVE_BIT ( ch->act, PLR_BOUGHT_PET );

     form_to_char ( ch, 
                    "{WYour gain is: {Y%d/%d hp, %d/%d m, %d/%d mv %d/%d prac.{w\n\r",
                    add_hp, ch->max_hit,
                    add_mana, ch->max_mana,
                    add_move, ch->max_move, add_prac, ch->pcdata->practice );
     /* Zeran - notify_message */
     notify_message ( ch, NOTIFY_LEVEL, TO_ALL, NULL );
     return;
}

/*
 * Mobs get xp.
 * They don't level - Yet. - Lotherius.
 */

void gain_exp ( CHAR_DATA * ch, int gain )
{
     if ( ch->level >= LEVEL_HERO )
          return;

     if ( IS_NPC ( ch ) )
     {
          ch->exp += gain;
          return;
     }

     ch->exp = UMAX ( exp_per_level ( ch, ch->pcdata->points ),  ch->exp + gain );

     while ( ch->level < LEVEL_HERO && ch->exp >= exp_per_level ( ch, ch->pcdata->points ) * ( ch->level + 1 ) )
     {
          send_to_char ( "{GYou raise a level!!{w\n\r", ch );
          sound ("level.wav", ch);
          ch->level += 1;
          advance_level ( ch );
          save_char_obj ( ch );
     }

    /* Is character now a hero? */

     if ( ch->level == LEVEL_HERO )
     {
          ++ch->pcdata->account->heroes;

          send_to_char ( "{RH{YE{RR{YO{W!\n\r\n\r{wYou are now a HERO of the lands!\n\r\n\r", ch );
          notify_message ( ch, NOTIFY_HERO, TO_ALL, NULL );

          if ( ( ch->pcdata->account->heroes >= mud.fordemi) && (ch->pcdata->account->status == ACCT_VERIFIED) )
          {
               send_to_char ( "You may now create {YDemi-God{w characters!!!\n\r", ch );
               ch->pcdata->account->status = ACCT_VERIFIED_DEMISTAT;
          }
          fwrite_accounts ( ); /* save it */
     }

     return;
}

/*
 * Regeneration stuff.
 */
int hit_gain ( CHAR_DATA * ch )
{
     int                 gain;
     int                 number;

     if ( IS_NPC ( ch ) )
     {
          gain = 6 + ch->level;
          if ( IS_AFFECTED ( ch, AFF_REGENERATION ) )
               //gain *= 2;
               gain = MULTWO(gain);
          switch ( ch->position )
          {
          default:
               //gain /= 2;
               gain = DIVTWO(gain);
               break;
          case POS_SLEEPING:
               gain = 3 * gain / 2;
               break;
          case POS_RESTING:
               break;
          case POS_FIGHTING:
               gain /= 3;
               break;
          }
     }
     else
     {
          gain = UMAX ( 3, get_curr_stat ( ch, STAT_CON ) - 3 + ch->level / 2 );
          gain += class_table[ch->pcdata->pclass].hp_max - 10;
          number = number_percent (  );
          if ( number < ch->pcdata->learned[gsn_fast_healing] )
          {
               gain += number * gain / 100;
               if ( ch->hit < ch->max_hit )
                    check_improve ( ch, gsn_fast_healing, TRUE, 8 );
          }

          if ( IS_AFFECTED ( ch, AFF_REGENERATION ) )
               gain *= 1.5;

          switch ( ch->position )
          {
          default:
               gain /= 4;
               break;
          case POS_SLEEPING:
               break;
          case POS_RESTING:
               gain = DIVTWO(gain);
               //gain /= 2;
               break;
          case POS_FIGHTING:
               gain /= 6;
               break;
          }

          if ( ch->pcdata->condition[COND_FULL] == 0 )
               gain = DIVTWO(gain);

          if ( ch->pcdata->condition[COND_THIRST] == 0 )
               gain = DIVTWO(gain);

     }

     if ( IS_SET ( ch->in_room->room_flags, ROOM_FASTHEAL ) )
     {
          number = number_percent (  );
          gain += number * gain / 100;	/* elfren added for fast heal rooms */
     }

     if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
          gain += gain * ch->on->value[3] / 100; /* Furniture heal bonus */

     if ( IS_AFFECTED ( ch, AFF_POISON ) )
          gain /= 4;

     if ( IS_AFFECTED ( ch, AFF_PLAGUE ) )
          gain /= 8;

    /* If hasted OR slowed, penalty to healing.  Justification:
     * The altered physics of your body has detrimental affects
     * upon ability to heal, regardless of other positive or
     * negative effects */
     if ( IS_AFFECTED ( ch, AFF_HASTE ) ||
          CAN_DETECT ( ch, AFF_SLOW ) )
          gain = DIVTWO(gain);

     if ( !IS_NPC ( ch ) )
     {
          gain /= 6;		/* quick hack to provide for fact that gain is now 6
				 * times more common */
          if ( gain <= 0 )
               ++gain;
     }

     return UMIN ( gain, ch->max_hit - ch->hit );
}

int mana_gain ( CHAR_DATA * ch )
{
     int                 gain;
     int                 number;

     if ( IS_NPC ( ch ) )
     {
          gain = 5 + ch->level;
          switch ( ch->position )
          {
          default:
               gain = DIVTWO(gain);
               break;
          case POS_SLEEPING:
               gain = 3 * gain / 2;
               break;
          case POS_RESTING:
               break;
          case POS_FIGHTING:
               gain /= 3;
               break;
          }
     }
     else
     {
          gain = DIVTWO ( ( get_curr_stat ( ch, STAT_WIS ) + get_curr_stat ( ch, STAT_INT ) + ch->level ) );
          number = number_percent (  );
          /* elfren added and for psionic meld */
          if ( ( number < ch->pcdata->learned[gsn_meditation] )
               && !IS_AFFECTED ( ch, AFF_MELD ) )
          {
               gain += number * gain / 100;
               if ( ch->mana < ch->max_mana )
                    check_improve ( ch, gsn_meditation, TRUE, 8 );

          }
          if ( !class_table[ch->pcdata->pclass].fMana )
               gain = DIVTWO(gain);

          switch ( ch->position )
          {
          default:
               gain /= 4;
               break;
          case POS_SLEEPING:
               break;
          case POS_RESTING:
               gain = DIVTWO(gain);
               break;
          case POS_FIGHTING:
               gain /= 6;
               break;
          }

          if ( ch->pcdata->condition[COND_FULL] == 0 )
               gain = DIVTWO(gain);

          if ( ch->pcdata->condition[COND_THIRST] == 0 )
               gain = DIVTWO(gain);
     }

     if ( IS_SET ( ch->in_room->room_flags, ROOM_FASTHEAL ) )
     {
          number = number_percent (  );
          gain += number * gain / 100;	/* elfren added for fast heal rooms */
     }

     if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
          gain = gain * ch->on->value[3] / 100;

     if ( IS_AFFECTED ( ch, AFF_POISON ) )
          gain /= 4;

     if ( IS_AFFECTED ( ch, AFF_PLAGUE ) )
          gain /= 8;

     if ( IS_AFFECTED ( ch, AFF_HASTE ) ||
          CAN_DETECT ( ch, AFF_SLOW ) )
          gain = DIVTWO(gain);

    /* no need to check on NPC's since i'm not calling update mana for them */
     gain /= 6;			/* quick hack to provide for fact that gain is now 6 times more common */

     ++gain;			/* on low level chars, they were gaining 0! */

     return UMIN ( gain, ch->max_mana - ch->mana );
}

int move_gain ( CHAR_DATA * ch )
{
     int                 gain;
     int                 number;

     if ( IS_NPC ( ch ) )
     {
          gain = ch->level;
     }
     else
     {
          gain = UMAX ( 15, ch->level );

          switch ( ch->position )
          {
          case POS_SLEEPING:
               gain += get_curr_stat ( ch, STAT_DEX );
               break;
          case POS_RESTING:
               gain += DIVTWO(get_curr_stat ( ch, STAT_DEX ) );
               break;
          }

          if ( ch->pcdata->condition[COND_FULL] == 0 )
               gain = DIVTWO(gain);

          if ( ch->pcdata->condition[COND_THIRST] == 0 )
               gain = DIVTWO(gain);
     }

     if ( IS_SET ( ch->in_room->room_flags, ROOM_FASTHEAL ) )
     {
          number = number_percent (  );
          gain += number * gain / 100;	/* elfren added for fast heal rooms */
     }

     if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE )
          gain = gain * ch->on->value[3] / 100;

     if ( IS_AFFECTED ( ch, AFF_POISON ) )
          gain /= 4;

     if ( IS_AFFECTED ( ch, AFF_PLAGUE ) )
          gain /= 8;

     if ( IS_AFFECTED ( ch, AFF_HASTE ) ||
          CAN_DETECT ( ch, AFF_SLOW ) )
          gain = DIVTWO(gain);

     if ( !IS_NPC ( ch ) )
     {
          gain /= 6;		/* quick hack to provide for fact that gain is now 6 times more common */

          if ( gain <= 0 )
               ++gain;		/* on low level chars, they were gaining 0! */
     }

     return UMIN ( gain, ch->max_move - ch->move );
}

void gain_condition ( CHAR_DATA * ch, int iCond, int value )
{
     int                 condition;

     if ( value == 0 || IS_NPC ( ch ) || ch->level >= LEVEL_HERO )
          return;

     condition = ch->pcdata->condition[iCond];

     if ( condition == -1 )
          return;

     ch->pcdata->condition[iCond] = URANGE( 0, condition + value, 48 );

     if ( ch->pcdata->condition[iCond] == 0 )
     {
          switch ( iCond )
          {
          case COND_FULL:
               send_to_char ( "You are hungry.\n\r", ch );
               break;

          case COND_THIRST:
               send_to_char ( "You are thirsty.\n\r", ch );
               break;

          case COND_DRUNK:
               if ( condition != 0 )
                    send_to_char ( "You are sober.\n\r", ch );
               break;
          }
     }

     return;
}

void check_health ( )
{
     CHAR_DATA		*ch;
     DESCRIPTOR_DATA	*d;
     int                 age, maxage, chance, causeofdeath = 0;
     bool                middleage = TRUE;
     bool                oldage = TRUE;
     bool                venerable = TRUE;

	 /* Causes of Death: 1 - Plague, 2- Poison, 3 - Heart Attack */
	 /* 4 - Died in sleep, 5 - a blow to the head, 6 - massive trauma */
	 /* 7 - Natural Causes */

     // Okay we figure if we got in here, NOAGING should've already been checked, we won't check for it
     //
     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected != CON_PLAYING )
               continue;
          ch = d->original ? d->original : d->character;

          if ( ch->position >= POS_MORTAL && ch->pcdata->mortal )
          {
               chance = 0;
               age = get_age (ch);
               maxage = pc_race_table[ch->pcdata->pcrace].maxage;

               if (age < (maxage/2) )
                    continue;	/* Go Easy on the Youngsters */

			   /* Let's worsen odds if poisoned or diseased */

               if ( is_affected ( ch, gsn_plague ) && ch != NULL )
               {
                    chance += 5;
                    if (number_percent ( )> 25)
                         causeofdeath = 1;
               }

               if ( IS_AFFECTED ( ch, AFF_POISON ) && ch != NULL )
               {
                    chance += 3;
                    if ( (causeofdeath == 0) && (number_percent ( ) > 25) )
                         causeofdeath = 2;
               }

               if (number_fuzzy (age) >= (maxage - (maxage/15) ) )
                    middleage = TRUE;

               if (number_fuzzy (age) >= (maxage) )
               {
                    middleage = FALSE;
                    oldage = TRUE;
               }

               if (number_fuzzy (age) >= (maxage + (maxage/15) ) )
               {
                    middleage = FALSE;
                    oldage = FALSE;
                    venerable = TRUE;
               }

               if (venerable)
                    chance +=5; /* Automatically have a chance to die if venerable. */

               switch ( ch->position )
               {
               case POS_SLEEPING:
                    if (causeofdeath == 0 && number_percent() > 50)
                         causeofdeath = 4;
                    else if (causeofdeath == 0)
                         causeofdeath = 7;
                    break;
               case POS_RESTING:
                    if (venerable)
                         chance +=1;
                    if (causeofdeath == 0 && number_percent() > 50)
                         causeofdeath = 3;
                    else if (causeofdeath == 0)
                         causeofdeath = 7;
                    break;
               case POS_SITTING:
                    if (venerable || oldage)
                         chance +=2;
                    if (causeofdeath == 0 && number_percent() > 45)
                         causeofdeath = 3;
                    else if (causeofdeath == 0)
                         causeofdeath = 7;
                    break;
               case POS_STANDING:
                    if (venerable)
                         chance +=4;
                    else if (oldage)
                         chance +=2;
                    else if (middleage)
                         chance +=1;
                    if (causeofdeath == 0 && number_percent() > 50)
                         causeofdeath = 3;
                    else if (causeofdeath == 0 && !middleage)
                         causeofdeath = 7;
                    else if (causeofdeath == 0)
                         causeofdeath = 3;

                    break;
               case POS_FIGHTING:
                    if (venerable)
                         chance +=10;
                    else if (oldage)
                         chance +=4;
                    else if (middleage)
                         chance +=1;
                    if ( (venerable || oldage) && (number_percent() > 50) )
                         causeofdeath = 3;
                    else if (middleage && (number_percent() > 80) )
                         causeofdeath = 3;
                    else if (number_percent() > 40)
                         causeofdeath = 5;
                    else
                         causeofdeath = 6;
                    break;
               case POS_STUNNED:
                    if (venerable)
                         chance +=15;
                    else if (oldage)
                         chance +=8;
                    else if (middleage)
                         chance +=5;
                    if ( (venerable || oldage) && (number_percent() > 20) )
                         causeofdeath = 3;
                    else if (middleage && (number_percent() > 50) )
                         causeofdeath = 3;
                    else if (number_percent() > 60)
                         causeofdeath = 5;
                    else
                         causeofdeath = 6;
                    break;
               case POS_INCAP:
                    if (venerable)
                         chance +=25;
                    else if (oldage)
                         chance +=15;
                    else if (middleage)
                         chance +=7;
                    if ( (venerable || oldage) && (number_percent() > 20) )
                         causeofdeath = 3;
                    else if (middleage && (number_percent() > 60) )
                         causeofdeath = 3;
                    else if (number_percent() > 90)
                         causeofdeath = 5;
                    else
                         causeofdeath = 6;
                    break;
               default:
                    causeofdeath = 7;
                    break;
               }

			   /* Let's make the odds better if regenerating */
               if ( IS_AFFECTED ( ch, AFF_REGENERATION ) )
               {
                    chance -= 10;

                    if (chance < 0)
                         chance = 0;
               }

			   /* Now we check for death! */

               if (number_percent() <= chance) /* You die */
               {
                    switch (causeofdeath)
                    {
                    case 1:
                         send_to_char("{RThe plague has taken your life for good this time!{w", ch);
                         act ("{RThe plaque has taken $N's life for good this time!{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    case 2:
                         send_to_char ("{RThe poison in your system is too much: You die.{w", ch );
                         act ("{R$N has fallen victim to foul poison and dies.{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    case 3:
                         send_to_char ("{RPAIN IN YOUR CHEST!!!! The world fades to black.{w", ch);
                         act ("{R$N clutches $S chest, and dies of a massive heart attack.{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    case 4:
                         send_to_char ("{RYou are sleeping peacefully now, and will never wake again.{w", ch);
                         act ("{R$N dies in $S sleep.{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    case 5:
                         send_to_char ("{RYou suddenly feel dizzy. The blow to your head... too hard this time..{w", ch);
                         act ("{R$NWobbles dizzily and collapses. Dead.{w", ch, NULL, ch, TO_CHAR );
                         break;
                    case 6:
                         send_to_char ("{RYour eyes fill with {Wblood{R and in your last thoughts...{w", ch);
                         act ("{R$N's head is demolished by a savage attack. $E is dead.{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    case 7:
                    default:
                         send_to_char ("{RYou have died of natural causes. Rest in peace.{w", ch);
                         act ("{R$N has died of natural causes. Rest in peace.{w", ch, NULL, ch, TO_NOTVICT );
                         break;
                    }

                    if ( mud.death == PERMADEATH || mud.death == FULLAGING )
                    {
                         send_to_char ("\n\r{WThis death is a {RPERMADEATH{W.\n\r", ch);
                         send_to_char ("\n\r{WPlease come back soon!\n\r\n\r", ch);

                         do_quote ( ch );
                         ++ch->pcdata->account->permadead;
                         raw_kill(ch);	/* We want a corpse here. */
                         sound ("taps.wav", ch);
                         real_delete ( ch );	/* Delete character Fully */
                    }
                    else  // Gotta be partial aging, die but not permadeath.
                    {
                         raw_kill(ch);
                         sound ( "taps.wav", ch );
                         send_to_char ( "\n\r{WYour deity intervenes and you find yourself resurrected.{w\n\r", ch);
                    }

               }

			   /* If you didn't die, lucky you. */
          }
     }
     /* end descriptor loop */
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update ( void )
{
     CHAR_DATA          *ch;
     CHAR_DATA          *ch_next;
     EXIT_DATA          *pexit;
     int                 door;

     /* Examine all mobs. */
     for ( ch = char_list; ch != NULL; ch = ch_next )
     {
          ch_next = ch->next;

          // One long if check is faster than 2 short ones. Actually saved time on profiling. */
     if ( !IS_NPC ( ch ) || ch->in_room == NULL ||
          IS_AFFECTED ( ch, AFF_CHARM )
          || ( ch->in_room->area->empty && !IS_SET ( ch->act, ACT_UPDATE_ALWAYS ) ) )
          continue;

          /* Examine call for special procedure */
          if ( ch->spec_fun != 0 )
          {
               if ( ( *ch->spec_fun ) ( ch ) )
                    continue;
          }

          /*
           * Check triggers only if mobile still in default position
           * One wonders... couldn't this mess up using DELAY during a fight? Guess you can't.
           */
          if ( ch->position == ch->pIndexData->default_pos )
          {
               /* Delay */
               if ( HAS_TRIGGER_MOB( ch, TRIG_DELAY) && ch->mprog_delay > 0 )
               {
                    if ( --ch->mprog_delay <= 0 )
                    {
                         p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_DELAY );
                         continue;
                    }
               }
               if ( HAS_TRIGGER_MOB( ch, TRIG_RANDOM) )
               {
                    if( p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM ) )
                         continue;
               }
          }

          /* That's all for sleeping / busy monster, and empty zones */
          if ( ch->position != POS_STANDING )
               continue;

	/* Scavenge */
          if ( IS_SET ( ch->act, ACT_SCAVENGER )
               && ch->in_room->contents != NULL
               && number_bits ( 6 ) == 0 )
          {
               OBJ_DATA           *obj;
               OBJ_DATA           *obj_best;
               int                 max;

               max = 1;
               obj_best = 0;
               for ( obj = ch->in_room->contents; obj;
                     obj = obj->next_content )
               {
                    if ( CAN_WEAR ( obj, ITEM_TAKE ) &&
                         can_loot ( ch, obj ) && obj->cost > max && obj->cost > 0 && !obj->owner )
                    {
                         obj_best = obj;
                         max = obj->cost;
                    }
               }

               if ( obj_best )
               {
                    obj_from_room ( obj_best );
                    obj_to_char ( obj_best, ch );
                    act ( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
               }
          }

	/* Wander */
          if ( !IS_SET ( ch->act, ACT_SENTINEL )
               && number_bits ( 3 ) == 0
               && ( door = number_bits ( 5 ) ) <= 5
               && ( pexit = ch->in_room->exit[door] ) != NULL
               && pexit->u1.to_room != NULL
               && !IS_SET ( pexit->exit_info, EX_CLOSED )
               && !IS_SET ( pexit->u1.to_room->room_flags, ROOM_NO_MOB ) &&
               ( !IS_SET ( ch->act, ACT_STAY_AREA ) ||
                 pexit->u1.to_room->area == ch->in_room->area ) )
          {
               move_char ( ch, door, FALSE );
              /* If ch changes position due
               * to it's or someother mob's
               * movement via MOBProgs,
               * continue - Kahn */
               if ( ch->position < POS_STANDING )
                    continue;

          }

/*	 Flee
	if ( ch->hit < ch->max_hit / 2
	&& ( door = number_bits( 3 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = pexit->u1.to_room->people;
		  rch != NULL;
		  rch  = rch->next_in_room )
	    {
		if ( !IS_NPC(rch) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
		move_char( ch, door, FALSE );
	}
*/

     }

     return;
}

/*
 * Update the weather.
 */
void weather_update ( void )
{
     char                buf[MAX_STRING_LENGTH];
     DESCRIPTOR_DATA    *d;
     int                 diff;

     buf[0] = '\0';

     switch ( ++time_info.hour )
     {

     case 5:
          weather_info.sunlight = SUN_LIGHT;
          SLCAT ( buf, "The day has begun.\n\r" );
          break;

     case 6:
          weather_info.sunlight = SUN_RISE;

/* elfren - beginning realism code */
          switch ( weather_info.sky )
          {
          default:
               bugf ( "Weather Update: bad sky %d.", weather_info.sky );
               weather_info.sky = SKY_CLOUDLESS;
               break;

          case SKY_CLOUDLESS:
               SLCAT ( buf,
                        "The sun comes over the horizon in the east, revealing a beautiful day.\n\r" );
               break;

          case SKY_CLOUDY:
               SLCAT ( buf,
                        "Dimly, the sun begins to appear from behind the clouds.\n\r" );
               break;

          case SKY_RAINING:
               if ( weather_info.change >= 0 )
               {
                    SLCAT ( buf,
                             "A glow begins to form in the east through the rain.\n\r" );
               }
               else
               {
                    SLCAT ( buf,
                             "The world brightens somewhat as the sun shines through the snow.\n\r" );
               }
               break;

          case SKY_LIGHTNING:
               SLCAT ( buf,
                        "Through the storm you can see a dim glow to the east.\n\r" );
               break;
          }

          break;

     case 19:
          weather_info.sunlight = SUN_SET;
          switch ( weather_info.sky )
          {
          default:
               bugf ( "Weather Update: bad sky %d.",
                     weather_info.sky );
               weather_info.sky = SKY_CLOUDLESS;
               break;

          case SKY_CLOUDLESS:
               SLCAT ( buf,
                        "The evening sun lights the western sky red.\n\r" );
               break;

          case SKY_CLOUDY:
               SLCAT ( buf,
                        "The sun sets in glorious colors through the clouds.\n\r" );
               break;

          case SKY_RAINING:
               SLCAT ( buf,
                        "The warmth of day begins to fade as the sun sets.\n\r" );
               break;

          case SKY_LIGHTNING:
               SLCAT ( buf,
                        "Lightning accents the now darkening sky.\n\r" );
               break;
          }

          break;

     case 20:
          weather_info.sunlight = SUN_DARK;
          SLCAT ( buf, "The night has begun.\n\r" );
          break;

     case 24:			/*roll day and update time_file */
          time_info.hour = 0;
          time_info.day++;
          if ( ( time_file = fopen ( TIME_FILE, "w+" ) ) == NULL )
          {
               bugf ( "creation of new time_file failed" );
          }
          else
          {
               int                 total;

               total = fprintf ( time_file, "%d %d %d %d",
                                 time_info.hour,
                                 time_info.day,
                                 time_info.month, time_info.year );
               if ( total < 4 )
                    bugf ( "failed fprintf to time_file" );
               fclose ( time_file );
          }
          break;
     }

     if ( time_info.day >= 35 )
     {
          time_info.day = 0;
          time_info.month++;
     }

     if ( time_info.month >= 17 )
     {
          time_info.month = 0;
          time_info.year++;
     }

    /*
     * Weather change.
     */
     if ( time_info.month >= 9 && time_info.month <= 16 )
          diff = weather_info.mmhg > 985 ? -2 : 2;
     else
          diff = weather_info.mmhg > 1015 ? -2 : 2;

     weather_info.change +=
          diff * dice ( 1, 4 ) + dice ( 2, 6 ) - dice ( 2, 6 );
     weather_info.change = UMAX ( weather_info.change, -12 );
     weather_info.change = UMIN ( weather_info.change, 12 );

     weather_info.mmhg += weather_info.change;
     weather_info.mmhg = UMAX ( weather_info.mmhg, 960 );
     weather_info.mmhg = UMIN ( weather_info.mmhg, 1040 );

     switch ( weather_info.sky )
     {
     default:
          bugf ( "Weather_update: bad sky %d.", weather_info.sky );
          weather_info.sky = SKY_CLOUDLESS;
          break;

     case SKY_CLOUDLESS:
          if ( weather_info.mmhg < 990
               || ( weather_info.mmhg < 1010 &&
                    number_bits ( 2 ) == 0 ) )
          {
               SLCAT ( buf, "The sky is getting cloudy.\n\r" );
               weather_info.sky = SKY_CLOUDY;
          }
          break;

     case SKY_CLOUDY:
          if ( weather_info.mmhg < 970
               || ( weather_info.mmhg < 990 &&
                    number_bits ( 2 ) == 0 ) )
          {
               if ( weather_info.change >= 0 )
               {
                    SLCAT ( buf, "It starts raining.\n\r" );
               }
               else
               {
                    SLCAT ( buf, "It starts to snow.\n\r" );
               }

               weather_info.sky = SKY_RAINING;
          }

          if ( weather_info.mmhg > 1030 && number_bits ( 2 ) == 0 )
          {
               SLCAT ( buf, "The clouds disappear.\n\r" );
               weather_info.sky = SKY_CLOUDLESS;
          }
          break;

     case SKY_RAINING:
          if ( weather_info.mmhg < 970 && number_bits ( 2 ) == 0 )
          {
               if ( weather_info.change >= 0 )
               {
                    SLCAT ( buf,
                             "{BLightning and thunder fill the sky.{w\n\r" );
               }
               else
               {
                    SLCAT ( buf, "{WA blizzard has hit!{w\n\r" );
               }

               weather_info.sky = SKY_LIGHTNING;
          }

          if ( weather_info.mmhg > 1030
               || ( weather_info.mmhg > 1010 &&
                    number_bits ( 2 ) == 0 ) )
          {
               if ( weather_info.change >= 0 )
               {
                    SLCAT ( buf, "The rain has stopped.\n\r" );
               }
               else
               {
                    SLCAT ( buf,
                             "It stops snowing, leaving the world sparkly white.\n\r" );
               }

               weather_info.sky = SKY_CLOUDY;
          }
          break;

     case SKY_LIGHTNING:
          if ( weather_info.mmhg > 1010
               || ( weather_info.mmhg > 990 &&
                    number_bits ( 2 ) == 0 ) )
          {

               if ( weather_info.change >= 0 )
               {
                    SLCAT ( buf,
                             "The storm calms into a gentle rain.\n\r" );
               }
               else
               {
                    SLCAT ( buf,
                             "The snow lessens, but does not stop.\n\r" );
               }

               weather_info.sky = SKY_RAINING;
               break;
          }
          break;
     }

     if ( buf[0] != '\0' )
     {
          for ( d = descriptor_list; d != NULL; d = d->next )
          {
               if ( d->connected == CON_PLAYING && IS_OUTSIDE ( d->character )
                    && IS_AWAKE ( d->character ) && ( IS_SET ( d->character->pcdata->notify, NOTIFY_WEATHER ) ) )
                    send_to_char ( buf, d->character );
          }
     }

     return;
}

 /*
  * * Short Char Update
  */

/* Zeran - hell, if you only want to update the players, don't
	loop the whole damn char_list....no wonder we're getting game loops
	taking over 1 second */

void char_update_short ( void )
{
     CHAR_DATA          *ch;
     DESCRIPTOR_DATA    *d;

    /* Zeran - This is bad!
     * for ( ch = char_list; ch != NULL; ch = ch_next )
     * {
     * ch_next = ch->next; */

     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected != CON_PLAYING )
               continue;
          ch = d->original ? d->original : d->character;
          if ( ch->position >= POS_STUNNED )
          {
               if ( ch->hit < ch->max_hit )
                    ch->hit += hit_gain ( ch );
               else
                    ch->hit = ch->max_hit;
               if ( ch->mana < ch->max_mana )
                    ch->mana += mana_gain ( ch );
               else
                    ch->mana = ch->max_mana;
               if ( ch->move < ch->max_move )
                    ch->move += move_gain ( ch );
               else
                    ch->move = ch->max_move;
          }
     }
     /* end descriptor loop */
}

/*
 * Update all chars, including mobs.
*/
void char_update ( void )
{
     CHAR_DATA          *ch;
     CHAR_DATA          *ch_next;
     CHAR_DATA 		*ch_save;
     CHAR_DATA          *ch_quit;
     time_t              save_time;

     ch_quit = NULL;
     ch_save = NULL;
     save_time   = current_time;

     for ( ch = char_list; ch != NULL; ch = ch_next )
     {
          AFFECT_DATA        *paf;
          AFFECT_DATA        *paf_next;

          ch_next = ch->next;

          if ( ch->position >= POS_STUNNED && IS_NPC ( ch ) )
          {
               if ( ch->hit != ch->max_hit )
                    ch->hit += hit_gain ( ch );
               if ( ch->mana != ch->max_mana )
                    ch->mana += mana_gain ( ch );
               if ( ch->move != ch->max_move )
                    ch->move += move_gain ( ch );
          }

          if ( ch->position == POS_STUNNED )
               update_pos ( ch );

          if ( !IS_NPC ( ch ) )
          {
               OBJ_DATA    *obj;
               
               if ( ch->timer > 30 )
                    ch_quit = ch; // Don't save the quit person.
               else if ( !ch->desc ) // linkeads don't get autosave
               {
                    // NULL OP for Now
               }
               else if ( ch->desc->connected == CON_PLAYING
                         &&   ch->level >= 2  
                         &&   ch->desc->save_time < save_time )
               {
                    ch_save     = ch;
                    save_time   = ch->desc->save_time;
               }

               if ( ( obj = get_eq_char ( ch, WEAR_LIGHT ) ) != NULL
                    && obj->item_type == ITEM_LIGHT
                    && obj->value[2] > 0 )
               {
                    if ( --obj->value[2] == 0 && ch->in_room != NULL )
                    {
                         --ch->in_room->light;
                         act ( "$p goes out.", ch, obj, NULL, TO_ROOM );
                         act ( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
                         extract_obj ( obj );
                    }
                    else if ( obj->value[2] <= 5 &&
                              ch->in_room != NULL )
                         act ( "$p flickers.", ch, obj, NULL, TO_CHAR );
               }

               if ( IS_IMMORTAL ( ch ) )
                    ch->timer = 0;

               if ( ++ch->timer >= 12 )
               {
                    if ( ch->was_in_room == NULL &&
                         ch->in_room != NULL )
                    {
                         ch->was_in_room = ch->in_room;
                         if ( ch->fighting != NULL )
                              stop_fighting ( ch, TRUE );
                         act ( "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
                         send_to_char ( "You disappear into the void.\n\r", ch );
                         if ( ch->level > 1 )
                              save_char_obj ( ch );
                         char_from_room ( ch );
                         char_to_room ( ch, get_room_index ( ROOM_VNUM_LIMBO ) );
                    }
               }

               if (ch->level < LEVEL_IMMORTAL )
               {
                    gain_condition( ch, COND_DRUNK,  -1 * time_info.hour % 2 );
                    gain_condition( ch, COND_FULL,   ch->size > SIZE_MEDIUM ? -2 : -1 * time_info.hour % 2 );
                    gain_condition( ch, COND_THIRST, -1 * time_info.hour % 2 );
               }

          }

          for ( paf = ch->affected; paf != NULL; paf = paf_next )
          {
               paf_next = paf->next;
               if ( paf->duration > 0 )
               {
                    paf->duration--;
                    if ( number_range ( 0, 4 ) == 0 && paf->level > 0 )
                         paf->level--;	/* spell strength fades with time */
               }
               else if ( paf->duration < 0 )
                    ;
               else
               {
                    if ( paf_next == NULL
                         || paf_next->type != paf->type
                         || paf_next->duration > 0 )
                    {
                         if ( paf->type > 0 &&
                              skill_table[paf->type].msg_off )
                         {
                              send_to_char ( skill_table[paf->type].msg_off, ch );
                              send_to_char ( "\n\r", ch );
                         }
                    }
		/* Zeran - undo masked descriptions */
                    if ( paf->bitvector == AFF_POLY )
                         undo_mask ( ch );
                    affect_remove ( ch, paf );
               }
          }

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */
          if ( !IS_IMMORTAL ( ch ) )
          {
               if ( is_affected ( ch, gsn_plague ) && ch != NULL )
               {
                    AFFECT_DATA        *af, plague;
                    CHAR_DATA          *vch;
                    int                 save, dam;

                    if ( ch->in_room == NULL )
                         return;

                    act ( "$n writhes in agony as plague sores erupt from $s skin.", ch, NULL, NULL, TO_ROOM );
                    send_to_char ( "You writhe in agony from the plague.\n\r", ch );
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

                    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
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
                              number_bits ( 4 ) == 0 )
                         {
                              send_to_char ( "You feel hot and feverish.\n\r", vch );
                              act ( "$n shivers and looks very ill.", vch, NULL, NULL, TO_ROOM );
                              affect_join ( vch, &plague );
                         }
                    }

                    dam = UMIN ( ch->level, 5 );
                    ch->mana -= dam;
                    ch->move -= dam;
                    damage ( ch, ch, dam, gsn_plague, DAM_DISEASE, TRUE );
               }
               else if ( IS_AFFECTED ( ch, AFF_POISON ) && ch != NULL )
               {
                    act ( "$n shivers and suffers.", ch, NULL, NULL,  TO_ROOM );
                    send_to_char ( "You shiver and suffer.\n\r", ch );
                    damage ( ch, ch, 2, gsn_poison, DAM_POISON, TRUE );
               }
               else if ( ch->position == POS_INCAP &&  number_range ( 0, 1 ) == 0 )
               {
                    damage ( ch, ch, 1, TYPE_UNDEFINED, DAM_BLEEDING, TRUE );
               }
               else if ( ch->position == POS_MORTAL )
               {
                    damage ( ch, ch, 1, TYPE_UNDEFINED, DAM_BLEEDING, TRUE );
               }
          }
     }

     /*
      * Autosave and autoquit.
      * Check that these chars still exist.
      */

     if ( ch_save || ch_quit )
     {
          for ( ch = char_list; ch; ch = ch->next )
          {
               if ( !ch )
                    continue;
               if ( ch == ch_save )
                    save_char_obj( ch );
               if ( ch == ch_quit )
                    do_quit( ch, "" );
          }
     }

     check_health(); /* Check for elderly mortals */

     return;
}

// Yes the logging here will spam the logfile, but it is better than no logging, plus
// if everything's working, it won't need to log anything. All of these are freaky
// conditions that happen while I'm developping and should go away once I'm done. - Loth
//
void lease_update ( void )
{
     DESCRIPTOR_DATA    *d;
     LEASE              *lease;
     char		 buf[MSL];

     for ( lease = lease_list; lease != NULL ; lease = lease->next )
     {
          if ( !IS_LEASE(lease) )
          {
               // Now we figure out why it isn't a lease
               //
               if ( !lease )
               {
                    bugf ( "?? ?? Something freaking happening in lease_list, NULL pointer, shouldn't see this." );
                    continue;
               }
               else if ( !lease->room )
               {
                    bugf ( "Lease with no room found. A reboot is necessary to fix this." );
                    if ( !IS_NULLSTR ( lease->rented_by ) )
                    {
                         log_string ( "It belonged to %s until d%dM%dY%d", lease->rented_by,
                               lease->paid_month, lease->paid_day, lease->paid_year );
                    }
                    log_string ("(It will simply remove the lease_list entry)" );
                    if ( IS_VALID(lease) ) // Make sure no code can get to this by accident
                         INVALIDATE ( lease );
                    continue;
               }
               else if ( !IS_SET ( lease->room->room_flags, ROOM_RENT ) )
               {
                    bugf ( "Room is in lease_list but has no ROOM_RENT flag. A reboot should fix this." );
                    if ( !IS_NULLSTR ( lease->rented_by ) )
                    {
                         log_string ( "It belonged to %s until d%dM%dY%d", lease->rented_by,
                               lease->paid_month, lease->paid_day, lease->paid_year );
                    }
                    log_string ("(It will simply remove the lease_list entry)" );
                    if ( IS_VALID(lease) ) // Make sure no code can get to this by accident
                         INVALIDATE ( lease );
                    continue;
               }
               else
               { // Here we can access the lease because we know that it has a room set, but is just invalid
                    // We don't know why though.
                    bugf ( "Invalidated Lease found, Trying to Fix. Room %d",
                          lease->room->vnum );
                    // Okay...we're here so we know we have a rent flag, we have lease structure, we have ROOM_RENT
                    // then there's no reason this shouldn't be a valid lease
                    VALIDATE ( lease );
               }
          }

          if ( IS_RENTED ( lease ) )
          {
			   /* check rent to see if it has expired */
               int                 timeleft = 0;
               CHAR_DATA          *victim;

               if ( lease->paid_year < time_info.year )
                    timeleft = 0;
               else if ( lease->paid_year == time_info.year )
                    timeleft = lease->paid_month - time_info.month;
               else if ( lease->paid_year > time_info.year )
               {
                    timeleft = lease->paid_month - time_info.month;
                    timeleft += 17;
               }
               // Let's see if the character that owns the lease is online and warn him/her
               // Not clans since clans autopay
               // Need a better notification system, preferably one that is less spammy.
/*
               if ( timeleft == 1 && !lease->clan )
               {
                    for ( d = descriptor_list; d; d = d->next )
                    {
                         if ( d->connected != CON_PLAYING )
                              continue;
                         victim = d->original ? d->original : d->character;

                         if ( !str_cmp ( lease->rented_by, victim->name ) )
                         {
                              form_to_char ( ch, "Your lease on %s is about to expire.\n\r",
                                        IS_NULLSTR ( lease->lease_name )
                                        ? lease->room->name
                                        : lease->lease_name );
                              break;
                         }
                    }
               }
 */
               if ( timeleft <= 0 )	/* lease has expired */
               {
                    bool autopaid = FALSE;
                    if ( lease->clan )
                    {
                         int months;
                         // Figure out how many months rent the clan can pay
                         months = lease->clan->clanbank / lease->room_rent;
                         if ( months < 1 )
                         {
                              SNP ( buf, "Can't pay the rent on %s.\n\r",
                                    IS_NULLSTR ( lease->lease_name )
                                    ? lease->room->name
                                    : lease->lease_name );
                         }
                         else
                         {
                              if ( months > 17)
                                   months = 17; // No more than 1 year....
                              autopaid = TRUE;
                              lease->clan->clanbank -= months * lease->room_rent;
                              lease->paid_year = time_info.year;
                              lease->paid_month = time_info.month;

                              if ( time_info.month + months > 17 )
                              {
                                   ++lease->paid_year;
                                   lease->paid_month += months - 17;
                              }
                              else
                              {
                                   lease->paid_month += months;
                              }
                              SNP ( buf, "Lease Payment: %d months on %s",
                                    months,
                                    IS_NULLSTR ( lease->lease_name )
                                    ? lease->room->name
                                    : lease->lease_name );
                         }
                         // Here's the bad hack again to find a member of the clan for notify
                         for ( d = descriptor_list; d; d = d->next )
                         {
                              if ( d->connected != CON_PLAYING )
                                   continue;
                              victim = d->original ? d->original : d->character;

                              if ( victim->pcdata->clan )
                              {
                                   if ( victim->pcdata->clan == lease->clan )
                                   {
                                        notify_message (victim, NOTIFY_CLANG, TO_CLAN, buf );
                                        break;
                                   }
                              }
                         }
                    }
                    if ( !autopaid )
                    {
                         free_string ( lease->rented_by );
                         free_string ( lease->lease_name );
                         free_string ( lease->lease_descr );
                         lease->rented_by = NULL;
                         lease->lease_name = str_dup ( "" );
                         lease->lease_descr = str_dup ( "" );
                         /* other values will be reset when room is rented */
                         act ( "The Lease on this room has Expired.", NULL, NULL, NULL, TO_ROOM );
                         save_leases ( );
                    }
               }
               /* End of expired */
          }
          /* end of rentable portion */
     }
     /* End of for loop */
     return;
}
/* end of lease_update */

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update ( void )
{
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;
     AFFECT_DATA        *paf, *paf_next;

     for ( obj = object_list; obj != NULL; obj = obj_next )
     {
          CHAR_DATA          *rch;
          char               *message;

          obj_next = obj->next;

          /* go through affects and decrement */
          for ( paf = obj->affected; paf != NULL; paf = paf_next )
          {
               paf_next = paf->next;
               if ( paf->duration > 0 )
               {
                    paf->duration--;
                    if ( number_range ( 0, 4 ) == 0 && paf->level > 0 )
                         paf->level--;	/* spell strength fades with time */
               }
               else if ( paf->duration < 0 )
                    ;
               else
               {
                    if ( paf_next == NULL
                         || paf_next->type != paf->type
                         || paf_next->duration > 0 )
                    {
                         if ( paf->type > 0 &&
                              skill_table[paf->type].msg_off )
                         {
                              act_new ( skill_table[paf->type].msg_off,
                                        obj->carried_by, obj, NULL,
                                        POS_SLEEPING, TO_CHAR );
                         }
                    }

                    affect_remove_obj ( obj, paf );
               }
          }

          if ( obj->timer <= 0 || --obj->timer > 0 )
               continue;

          if ( obj->in_room || (obj->carried_by && obj->carried_by->in_room))
          {
               if ( HAS_TRIGGER_OBJ( obj, TRIG_DELAY )
                    && obj->oprog_delay > 0 )
               {
                    if ( --obj->oprog_delay <= 0 )
                         p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_DELAY );
               }
               else if ( ((obj->in_room && !obj->in_room->area->empty)
                          || obj->carried_by ) && HAS_TRIGGER_OBJ( obj, TRIG_RANDOM ) )
                    p_percent_trigger( NULL, obj, NULL, NULL, NULL, NULL, TRIG_RANDOM );
          }
          /* Make sure the object is still there before proceeding */
          if ( !obj )
               continue;

          switch ( obj->item_type )
          {
          default:
               message = "$p crumbles into dust.";
               break;
          case ITEM_FOUNTAIN:
               message = "$p dries up.";
               break;
          case ITEM_CORPSE_NPC:
               message = "$p decays into dust.";
               break;
          case ITEM_CORPSE_PC:
               message = "$p decays into dust.";
               break;
          case ITEM_FOOD:
               message = "$p decomposes.";
               break;
          case ITEM_POTION:
               message = "$p has evaporated from disuse.";
               break;
          }

          if ( obj->carried_by != NULL )
          {
               if ( IS_NPC ( obj->carried_by )
                    && obj->carried_by->pIndexData->pShop != NULL )
                    obj->carried_by->gold += obj->cost / 5;
               else
               {
                    act ( message, obj->carried_by, obj, NULL,
                          TO_CHAR );
                    sound ("DECAY.WAV", obj->carried_by );
               }
          }
          else if ( obj->in_room != NULL &&
                    ( rch = obj->in_room->people ) != NULL )
          {
               if ( !
                    ( obj->in_obj &&
                      obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT &&
                      !CAN_WEAR ( obj->in_obj, ITEM_TAKE ) ) )
               {
                    act ( message, rch, obj, NULL, TO_ROOM );
                    act ( message, rch, obj, NULL, TO_CHAR );
                    sound ("DECAY.WAV", rch);
               }
          }

          if ( obj->item_type == ITEM_CORPSE_PC && obj->contains )
          {			/* save the contents */
               OBJ_DATA           *t_obj, *next_obj;

               for ( t_obj = obj->contains; t_obj != NULL;
                     t_obj = next_obj )
               {
                    next_obj = t_obj->next_content;
                    obj_from_obj ( t_obj );

                    if ( obj->in_obj )	/* in another object */
                         obj_to_obj ( t_obj, obj->in_obj );

                    else if ( obj->carried_by )	/* carried */
                         obj_to_char ( t_obj, obj->carried_by );

                    else if ( obj->in_room == NULL )	/* destroy it */
                         extract_obj ( t_obj );

                    else		/* to a room */
                         obj_to_room ( t_obj, obj->in_room );
               }
          }

          extract_obj ( obj );
     }
     return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 *
 * I'm sure others are doing this too... but instead of looping through
 * the entire char_list, I'm just going to look for players. Would rather not
 * descriptors, however, because linkdead is no reason to not get aggro'd.
 * Need to add a player_list in the future so players can easily be accessed
 * whether linkdead or not.
 */

void aggr_update ( void )
{
     DESCRIPTOR_DATA    *d;
     CHAR_DATA          *wch;
     CHAR_DATA          *ch;
     CHAR_DATA          *ch_next;
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;
     CHAR_DATA          *victim;

     /* Loop through world character list */
     for ( d = descriptor_list; d; d = d->next )
     {
          wch = d->character;

          /* Skip those not in the game. */
          /* Note writers had better be careful! */
          if ( d->connected != CON_PLAYING && d->connected < CON_NOTE_TO )
               continue;

          /* If character is in NULL room, is NPC, is IMM or in empty area, skip */
          /* Also skip if the room is safe, don't need to cycle through mobs to find this out. */
          if ( IS_NPC ( wch ) || wch->level >= LEVEL_IMMORTAL || wch->in_room == NULL
               || wch->in_room->area->empty || IS_SET ( wch->in_room->room_flags, ROOM_SAFE ) )
               continue;

          /* Now look for mobs in the same room */
          for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
          {
               int                 count;

               ch_next = ch->next_in_room;

               if ( !IS_NPC ( ch )
                    || !IS_SET ( ch->act, ACT_AGGRESSIVE )
                    || IS_AFFECTED ( ch, AFF_CALM )
                    || ch->fighting != NULL
                    || IS_AFFECTED ( ch, AFF_CHARM )
                    || !IS_AWAKE ( ch )
                    || ( IS_SET ( ch->act, ACT_WIMPY ) && IS_AWAKE ( wch ) )
                    || !can_see ( ch, wch )
                    || number_bits ( 1 ) == 0
                    || IS_AFFECTED ( ch, AFF_FEAR ) )	/* Zeran - add fear check */
                    continue;

               /*
                * Ok we have a 'wch' player character and a 'ch' npc aggressor.
                * Now make the aggressor fight a RANDOM pc victim in the room,
                *   giving each 'vch' an equal chance of selection.
                */
               count = 0;
               victim = NULL;
               for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
               {
                    vch_next = vch->next_in_room;

                    if ( !IS_NPC ( vch )
                         && vch->level < LEVEL_IMMORTAL
                         && ch->level >= vch->level - 5
                         && ( !IS_SET ( ch->act, ACT_WIMPY ) ||
                              !IS_AWAKE ( vch ) ) &&
                         can_see ( ch, vch ) )
                    {
                         if ( number_range ( 0, count ) == 0 )
                              victim = vch;
                         count++;
                    }
               }

               if ( victim == NULL )
                    continue;
               multi_hit ( ch, victim, TYPE_UNDEFINED );
          }
     }
     return;
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler ( void )
{
     static int          pulse_area;
     static int          pulse_mobile;
     static int          pulse_violence;
     static int          pulse_point;
     static int          pulse_chup;

     if ( --pulse_area <= 0 )
     {
          /*	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
          pulse_area = number_fuzzy(PULSE_AREA);
          area_update (  );
          quest_update (  );
     }

     if ( --pulse_mobile <= 0 )
     {
          pulse_mobile = PULSE_MOBILE;
          mobile_update (  );
     }

     if ( --pulse_violence <= 0 )
     {
          pulse_violence = PULSE_VIOLENCE;
          violence_update (  );
     }

     if ( --pulse_point <= 0 )
     {
          pulse_point = PULSE_TICK;

          weather_update (  );
          char_update (  );
          obj_update (  );
          lease_update (  );
          save_clans ( );
     }
     if ( --pulse_chup <= 0 )
     {
          pulse_chup = PULSE_TICKSHORT;
          char_update_short (  );
     }

     aggr_update (  );
     tail_chain (  );
     return;
}

void undo_mask ( CHAR_DATA * ch )
{
     free_string ( ch->description );
     free_string ( ch->short_descr );
     free_string ( ch->long_descr );
     free_string ( ch->poly_name );
     if ( !IS_NPC ( ch ) )
     {
          ch->description = str_dup ( ch->description_orig );
          ch->short_descr = str_dup ( ch->short_descr_orig );
          ch->long_descr = str_dup ( ch->long_descr_orig );
          ch->start_pos = -1;
          ch->poly_name = str_dup ( "" );
     }
     else
     {
          ch->description = str_dup ( ch->pIndexData->description );
          ch->short_descr = str_dup ( ch->pIndexData->short_descr );
          ch->long_descr = str_dup ( ch->pIndexData->long_descr );
          ch->poly_name = str_dup ( "" );
     }
}
