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

/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

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

DECLARE_DO_FUN ( do_say );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 71		/* Amulet */
#define QUEST_ITEM2 72		/* Shield */
#define QUEST_ITEM3 73		/* Sword of Sharpness - sharp - slice */
#define QUEST_ITEM4 74		/* Decanter */
#define QUEST_ITEM5 75		/* Blanket */ /* Too troublesome, disabled for now */
#define QUEST_ITEM6 76		/* Dagger of Acidity - acid acidic-bite */
#define QUEST_ITEM7 77		/* Frost Spear - frost - freezing-bite */
#define QUEST_ITEM8 78		/* Flaming Mace - flaming flaming-bite */
#define QUEST_ITEM9 79		/* Mithril Axe - vorpal cleave */
#define QUEST_ITEM10 80		/* Mace of Holy Might - vorpal sharp - divine-power */
#define QUEST_ITEM11 81		/* Flail of UnHoly Terror - vorpal vampiric - magic */
#define QUEST_ITEM12 82		/* Dagger of Planar Forces - vorpal lightning - magic */
#define QUEST_ITEM13 83 	/* The Sword of Sundering - vorpal protect - slice */

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 66
#define QUEST_OBJQUEST2 67
#define QUEST_OBJQUEST3 68
#define QUEST_OBJQUEST4 69
#define QUEST_OBJQUEST5 70

/* Local functions */

void generate_quest
     args ( ( CHAR_DATA * ch, CHAR_DATA * questman ) );
void quest_update   args ( ( void ) );
bool quest_level_diff args ( ( int clevel, int mlevel ) );
ROOM_INDEX_DATA    *find_location ( CHAR_DATA * ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance ( int num )
{
     if ( number_range ( 1, 100 ) <= num )
          return TRUE;
     else
          return FALSE;
}

/* The main quest function */

void do_quest ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *questman;
     OBJ_DATA           *obj = NULL, *obj_next;
     OBJ_INDEX_DATA     *questinfoobj;
     AFFECT_DATA        *paf;
     int                 bonus;
     MOB_INDEX_DATA     *questinfo;
     char		buf[MIL];
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( IS_NPC ( ch ) )
     {
          send_to_char ( "You're an NPC! You should be a quest target!", ch );
          return;
     }

     if ( arg1[0] == '\0' )
     {
          send_to_char
               ( "QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",
                 ch );
          send_to_char
               ( "For more information, type 'HELP QUEST'.\n\r", ch );
          return;
     }
     if ( !strcmp ( arg1, "info" ) )
     {
          if ( IS_SET ( ch->act, PLR_QUESTOR ) )
          {
               if ( ch->pcdata->questmob == -1 &&
                    ch->pcdata->questgiver->short_descr != NULL )
               {
                    form_to_char ( ch,
                                   "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",
                                   ch->pcdata->questgiver->short_descr );
               }
               else if ( ch->pcdata->questobj > 0 )
               {
                    questinfoobj =
                         get_obj_index ( ch->pcdata->questobj );
                    if ( questinfoobj != NULL )
                    {
                         form_to_char ( ch,
                                        "You are on a quest to recover the fabled %s!\n\r",
                                        questinfoobj->name );
                    }
                    else
                         send_to_char
                         ( "You aren't currently on a quest.\n\r",
                           ch );
                    return;
               }
               else if ( ch->pcdata->questmob > 0 )
               {
                    questinfo = get_mob_index ( ch->pcdata->questmob );
                    if ( questinfo != NULL )
                    {
                         form_to_char ( ch,
                                        "You are on a quest to slay the dreaded %s!\n\r",
                                        questinfo->short_descr );
                    }
                    else
                         send_to_char
                         ( "You aren't currently on a quest.\n\r",
                           ch );
                    return;
               }
          }
          else
               send_to_char ( "You aren't currently on a quest.\n\r",
                              ch );
          return;
     }
     if ( !strcmp ( arg1, "points" ) )
     {
          form_to_char ( ch, "You have %ld quest points.\n\r",
                         ch->pcdata->questpoints );
          return;
     }
     else if ( !strcmp ( arg1, "time" ) )
     {
          if ( !IS_SET ( ch->act, PLR_QUESTOR ) )
          {
               send_to_char ( "You aren't currently on a quest.\n\r", ch );
               if ( ch->pcdata->nextquest > 1 )               
                    form_to_char ( ch, "There are %d minutes remaining until you can go on another quest.\n\r",
                                   ch->pcdata->nextquest );
               else if ( ch->pcdata->nextquest == 1 )               
                    form_to_char ( ch, "There is less than a minute remaining until you can go on another quest.\n\r" );
               else               
                    send_to_char ( "You do not have to wait any longer to quest.\n\r", ch );               
          }
          else if ( ch->pcdata->countdown > 0 )
          {
               form_to_char ( ch, "Time left for current quest: %d\n\r",
                              ch->pcdata->countdown );
          }
          return;
     }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */

     for ( questman = ch->in_room->people; questman != NULL;
           questman = questman->next_in_room )
     {
          if ( !IS_NPC ( questman ) )
               continue;
          if ( questman->spec_fun ==
               spec_lookup ( "spec_questmaster" ) )
               break;
     }

     if ( questman == NULL ||
          questman->spec_fun != spec_lookup ( "spec_questmaster" ) )
     {
          send_to_char ( "You can't do that here.\n\r", ch );
          return;
     }

     if ( questman->fighting != NULL )
     {
          send_to_char ( "Wait until the fighting stops.\n\r", ch );
          return;
     }

     ch->pcdata->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

     if ( !strcmp ( arg1, "list" ) )
     {
          act ( "$n asks $N for a list of quest items.", ch, NULL,
                questman, TO_ROOM );
          act ( "You ask $N for a list of quest items.", ch, NULL,
                questman, TO_CHAR );
          send_to_char ( "{C+{c---------------------------------------------------{C+\n\r"
                         "{c| {G[ {WQP {G] {wItem                           {G({Wkeyword  {G){c |\n\r"
                         "{c| {G[{W2500{G] {wThe Sword of Sundering         {G({Wsword1   {G){c |\n\r"
                         "{c| {G[{W1750{G] {wDagger of Planar Forces        {G({Wdagger1  {G){c |\n\r"
                         "{c| {G[{W1750{G] {wMace of Holy Might             {G({Wmace1    {G){c |\n\r"
                         "{c| {G[{W1750{G] {wFlail of Unholy Terror         {G({Wflail    {G){c |\n\r"
                         "{c| {G[{W1000{G] {wSword of Sharpness             {G({Wsword2   {G){c |\n\r"
                         "{c| {G[{W1000{G] {wDagger of Acidity              {G({Wdagger2  {G){c |\n\r"
                         "{c| {G[{W1000{G] {wFlaming Mace                   {G({Wmace2    {G){c |\n\r"
                         "{c| {G[{W1000{G] {wFrost Spear                    {G({Wspear    {G){c |\n\r"
                         "{c| {G[{W1000{G] {wMithril Axe                    {G({Waxe      {G){c |\n\r"
                         "{c| {G[{W 750{G] {wShield of Zeran                {G({Wshield   {G){c |\n\r"
                         "{c| {G[{W 600{G] {wAmulet of Eardianm             {G({Wamulet   {G){c |\n\r"
                         "{c| {G[{W 550{G] {wDecanter of Endless Water      {G({Wdecanter {G){c |\n\r"
                         "{c| {G[{W 500{G] {w350,000 Gold Pieces            {G({Wgold     {G){c |\n\r"
                         "{c| {G[{W 500{G] {w30 Practice Sessions           {G({Wpractices{G){c |\n\r"
                         "{C+{c---------------------------------------------------{C+\n\r"
                         "{yTo buy an item, type '{WQUEST BUY <{Gkeyword{W>{y'.{w\n\r", ch );
          return;
     }

/* Here's the blanket formatted for the above list when/if added back in */

/* {c| {G[{W 500{G] {wWarrior's Blanket              {G({Wblanket  {G){c |\n\r\ */

     else if ( !strcmp ( arg1, "buy" ) )
     {
          if ( arg2[0] == '\0' )
          {
               send_to_char
                    ( "To buy an item, type 'QUEST BUY <item>'.\n\r",
                      ch );
               return;
          }
          if ( is_name ( arg2, "amulet" ) )
          {
               if ( ch->pcdata->questpoints >= 600 )
               {
                    ch->pcdata->questpoints -= 600;
                    obj =
                         create_object ( get_obj_index ( QUEST_ITEM1 ),
                                         ch->level );
                    obj->level = 1; /* Level doesn't matter on this */
               }
               else
               {
                    char buf[MIL];
                    SNP ( buf,
                          "Sorry, %s, but you don't have enough quest points for that.",
                          ch->name );
                    do_say ( questman, buf );
                    return;
               }
          }
          else if ( is_name ( arg2, "shield" ) )
          {
               if ( ch->pcdata->questpoints >= 750 )
               {
                    ch->pcdata->questpoints -= 750;
                    obj =
                         create_object ( get_obj_index ( QUEST_ITEM2 ),
                                         ch->level );
                    obj->level = ch->level;
                    obj->value[0] = ch->level;
                    obj->value[1] = ch->level;
                    obj->value[2] = ch->level;
                    obj->value[3] = ch->level;
               }
               else
               {
                    SNP ( buf,
			  "Sorry, %s, but you don't have enough quest points for that.",
			  ch->name );
                    do_say ( questman, buf );
                    return;
               }
          }
		/* Get all weapons */
          else if (  is_name ( arg2, "sword1" ) || is_name ( arg2, "sword2")
                     || is_name ( arg2, "mace1"  ) || is_name ( arg2, "mace2" )
                     || is_name ( arg2, "dagger1") || is_name ( arg2, "dagger2")
                     || is_name ( arg2, "spear"  ) || is_name ( arg2, "axe"   )
                     || is_name ( arg2, "flail"  ) )
          {
               if ( is_name ( arg2, "sword1") )
               {
                    if ( ch->pcdata->questpoints >= 2500 )
                    {
                         ch->pcdata->questpoints -= 2500;
                         obj = create_object ( get_obj_index ( QUEST_ITEM13 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "dagger1") )
               {
                    if ( ch->pcdata->questpoints >= 1750 )
                    {
                         ch->pcdata->questpoints -= 1750;
                         obj = create_object ( get_obj_index ( QUEST_ITEM12 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "flail") )
               {
                    if ( ch->pcdata->questpoints >= 1750 )
                    {
                         ch->pcdata->questpoints -= 1750;
                         obj = create_object ( get_obj_index ( QUEST_ITEM11 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "mace1") )
               {
                    if ( ch->pcdata->questpoints >= 1750 )
                    {
                         ch->pcdata->questpoints -= 1750;
                         obj = create_object ( get_obj_index ( QUEST_ITEM10 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "sword2") )
               {
                    if ( ch->pcdata->questpoints >= 1000 )
                    {
                         ch->pcdata->questpoints -= 1000;
                         obj = create_object ( get_obj_index ( QUEST_ITEM3 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "dagger2") )
               {
                    if ( ch->pcdata->questpoints >= 1000 )
                    {
                         ch->pcdata->questpoints -= 1000;
                         obj = create_object ( get_obj_index ( QUEST_ITEM6 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "spear") )
               {
                    if ( ch->pcdata->questpoints >= 1000 )
                    {
                         ch->pcdata->questpoints -= 1000;
                         obj = create_object ( get_obj_index ( QUEST_ITEM7 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "mace2") )
               {
                    if ( ch->pcdata->questpoints >= 1000 )
                    {
                         ch->pcdata->questpoints -= 1000;
                         obj = create_object ( get_obj_index ( QUEST_ITEM8 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               else if ( is_name ( arg2, "axe") )
               {
                    if ( ch->pcdata->questpoints >= 1000 )
                    {
                         ch->pcdata->questpoints -= 1000;
                         obj = create_object ( get_obj_index ( QUEST_ITEM9 ), ch->level );
                    }
                    else
                    {
                         SNP ( buf,
                               "Sorry, %s, but you don't have enough quest points for that.",
                               ch->name );
                         do_say ( questman, buf );
                         return;
                    }
               }
               obj->level = ch->level; /* Wasn't always set right */
               if ( ch->level <= 19 )
		    bonus = 2;
               else
		    bonus = ( ch->level / 10 + 1 );
               if ( affect_free == NULL )	/* set damroll */
		    paf = alloc_perm ( sizeof ( *paf ), "paf:quest" );
               else
               {
		    paf = affect_free;
		    affect_free = affect_free->next;
               }
               paf->type = 0;
               paf->level = ch->level;
               paf->duration = -1;
               paf->location = APPLY_DAMROLL;
               paf->modifier = bonus;
               paf->bitvector = 0;
               paf->next = obj->affected;
               obj->affected = paf;

               if ( affect_free == NULL )	/* set damroll */
		    paf = alloc_perm ( sizeof ( *paf ), "paf:quest" );
               else
               {
		    paf = affect_free;
		    affect_free = affect_free->next;
               }
               paf->type = 0;
               paf->level = ch->level;
               paf->duration = -1;
               paf->location = APPLY_DAMROLL;
               paf->modifier = bonus;
               paf->bitvector = 0;
               paf->next = obj->affected;
               obj->affected = paf;
               obj->value[1] = ch->level;

               if ( ch->level <= 49 )
		    obj->value[2] = 3;
               else
		    obj->value[2] = 4;
          }
          else if ( is_name ( arg2, "decanter endless water" ) )
          {
               if ( ch->pcdata->questpoints >= 550 )
               {
                    ch->pcdata->questpoints -= 550;
                    obj =
                         create_object ( get_obj_index ( QUEST_ITEM4 ),
                                         ch->level );
                    obj->level = 1; /*Level doesn't matter on this */
               }
               else
               {
                    SNP ( buf,
			  "Sorry, %s, but you don't have enough quest points for that.",
			  ch->name );
                    do_say ( questman, buf );
                    return;
               }
          }
          //	else if ( is_name ( arg2, "warrior blanket" ) )
          //	{
          //	    if ( ch->pcdata->questpoints >= 500 )
          //	    {
          //		ch->pcdata->questpoints -= 500;
          //		obj =
          //		    create_object ( get_obj_index ( QUEST_ITEM5 ),
          //				    ch->level );
          //		obj->level = 1; /* Level doesn't matter on this */
          //	    } else
          //	    {
          //		SNP ( buf,
          //			  "Sorry, %s, but you don't have enough quest points for that.",
          //			  ch->name );
          //		do_say ( questman, buf );
          //		return;
          //	    }
          //	}
          else if ( is_name
                    ( arg2, "practices pracs prac practice" ) )
          {
               if ( ch->pcdata->questpoints >= 500 )
               {
                    ch->pcdata->questpoints -= 500;
                    ch->pcdata->practice += 30;
                    act ( "$N gives 30 practices to $n.", ch, NULL,
                          questman, TO_ROOM );
                    act ( "$N gives you 30 practices.", ch, NULL,
                          questman, TO_CHAR );
                    return;
               }
               else
               {
                    SNP ( buf,
			  "Sorry, %s, but you don't have enough quest points for that.",
			  ch->name );
                    do_say ( questman, buf );
                    return;
               }
          }
          else if ( is_name ( arg2, "gold gp" ) )
          {
               if ( ch->pcdata->questpoints >= 500 )
               {
                    ch->pcdata->questpoints -= 500;
                    act ( "$N gives 350,000 gold pieces to $n.", ch,
                          NULL, questman, TO_ROOM );
                    act ( "$N has 350,000 in gold transfered from $s Swiss account to your balance.", ch, NULL, questman, TO_CHAR );
                    do_pay ( ch, 350000 );
                    return;
               }
               else
               {
                    SNP ( buf,
			  "Sorry, %s, but you don't have enough quest points for that.",
			  ch->name );
                    do_say ( questman, buf );
                    return;
               }
          }
          else
          {
               SNP ( buf, "I don't have that item, %s.",
                     ch->name );
               do_say ( questman, buf );
          }
          if ( obj != NULL )
          {
		/* Do a few things to it */
               if (obj->owner)
                    free_string(obj->owner);
               obj->owner = str_dup(ch->name);
               act ( "$N gives $p to $n.", ch, obj, questman,
                     TO_ROOM );
               act ( "$N gives you $p.", ch, obj, questman, TO_CHAR );
               obj_to_char ( obj, ch );
               save_char_obj(ch);	/* Safety */
          }
          return;
     }
     else if ( !strcmp ( arg1, "request" ) )
     {
          act ( "$n asks $N for a quest.", ch, NULL, questman,
                TO_ROOM );
          act ( "You ask $N for a quest.", ch, NULL, questman,
                TO_CHAR );
          if ( IS_SET ( ch->act, PLR_QUESTOR ) )
          {
               SNP ( buf, "But you're already on a quest!" );
               do_say ( questman, buf );
               return;
          }
          if ( ch->pcdata->nextquest > 0 )
          {
               SNP ( buf,
                     "You're very brave, %s, but let someone else have a chance.",
                     ch->name );
               do_say ( questman, buf );
               SNP ( buf, "Come back later." );
               do_say ( questman, buf );
               return;
          }

          SNP ( buf, "Thank you, brave %s!", ch->name );
          do_say ( questman, buf );
          ch->pcdata->questmob = 0;
          ch->pcdata->questobj = 0;

          generate_quest ( ch, questman );

          if ( ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0 )
          {
               ch->pcdata->countdown = number_range ( 10, 30 );
               SET_BIT ( ch->act, PLR_QUESTOR );
               SNP ( buf,
                     "You have %d minutes to complete this quest.",
                     ch->pcdata->countdown );
               do_say ( questman, buf );
               SNP ( buf, "May the gods go with you!" );
               do_say ( questman, buf );
          }
          return;
     }
     else if ( !strcmp ( arg1, "complete" ) )
     {
          act ( "$n informs $N $e has completed $s quest.", ch, NULL,
                questman, TO_ROOM );
          act ( "You inform $N you have completed $s quest.", ch,
                NULL, questman, TO_CHAR );
          if ( ch->pcdata->questgiver != questman )
          {
               SNP ( buf,
                     "I never sent you on a quest! Perhaps you're thinking of someone else." );
               do_say ( questman, buf );
               return;
          }

          if ( IS_SET ( ch->act, PLR_QUESTOR ) )
          {
               if ( ch->pcdata->questmob == -1 &&
                    ch->pcdata->countdown > 0 )
               {
                    int                 reward, pointreward,
                         pracreward;

                    reward =
                         ( number_range ( 150, 300 ) * ch->level );
                    pointreward = number_range ( 8, 22 );

                    SNP ( buf,
			  "Congratulations on completing your quest!" );
                    do_say ( questman, buf );
                    SNP ( buf,
			  "As a reward, I am giving you %d quest points, and %d gold.",
			  pointreward, reward );
                    do_say ( questman, buf );
                    if ( chance ( 15 ) )
                    {
                         pracreward = number_range ( 1, 4 );
                         form_to_char ( ch, "You gain %d practices!\n\r",
                               pracreward );
                         ch->pcdata->practice += pracreward;
                    }

                    REMOVE_BIT ( ch->act, PLR_QUESTOR );
                    ch->pcdata->questgiver = NULL;
                    ch->pcdata->countdown = 0;
                    ch->pcdata->questmob = 0;
                    ch->pcdata->questobj = 0;
                    ch->pcdata->nextquest = 30;
                    ch->gold += reward;
                    ch->pcdata->questpoints += pointreward;
                    ch->pcdata->questearned += pointreward;

                    return;
               }
               else if ( ch->pcdata->questobj > 0 &&
                         ch->pcdata->countdown > 0 )
               {
                    bool                obj_found = FALSE;

                    for ( obj = ch->carrying; obj != NULL;
                          obj = obj_next )
                    {
                         obj_next = obj->next_content;

                         if ( obj != NULL &&
                              obj->pIndexData->vnum ==
                              ch->pcdata->questobj )
                         {
                              obj_found = TRUE;
                              break;
                         }
                    }
                    if ( obj_found == TRUE )
                    {
                         int                 reward, pointreward,
                              pracreward;

                         reward =
                              ( number_range ( 75, 300 ) * ch->level );
                         pointreward = number_range ( 7, 18 );

                         act ( "You hand $p to $N.", ch, obj, questman,
                               TO_CHAR );
                         act ( "$n hands $p to $N.", ch, obj, questman,
                               TO_ROOM );

                         SNP ( buf,
                               "Congratulations on completing your quest!" );
                         do_say ( questman, buf );
                         SNP ( buf,
                               "As a reward, I am giving you %d quest points, and %d gold.",
                               pointreward, reward );
                         do_say ( questman, buf );
                         if ( chance ( 15 ) )
                         {
                              pracreward = number_range ( 1, 3 );
                              form_to_char ( ch, 
                                             "You gain %d practices!\n\r",
                                             pracreward );
                              ch->pcdata->practice += pracreward;
                         }

                         REMOVE_BIT ( ch->act, PLR_QUESTOR );
                         ch->pcdata->questgiver = NULL;
                         ch->pcdata->countdown = 0;
                         ch->pcdata->questmob = 0;
                         ch->pcdata->questobj = 0;
                         ch->pcdata->nextquest = 30;
                         do_pay ( ch, reward );
                         ch->pcdata->questpoints += pointreward;
                         ch->pcdata->questearned += pointreward;
                         extract_obj ( obj );
                         return;
                    }
                    else
                    {
                         SNP ( buf,
                               "You haven't completed the quest yet, but there is still time!" );
                         do_say ( questman, buf );
                         return;
                    }
                    return;
               }
               else
                    if ( ( ch->pcdata->questmob > 0 ||
                           ch->pcdata->questobj > 0 ) &&
                         ch->pcdata->countdown > 0 )
                    {
                         SNP ( buf,
                               "You haven't completed the quest yet, but there is still time!" );
                         do_say ( questman, buf );
                         return;
                    }
          }
          if ( ch->pcdata->nextquest > 0 )
               SNP ( buf,
                     "But you didn't complete your quest in time!" );
          else
               SNP ( buf,
                     "You have to REQUEST a quest first, %s.",
                     ch->name );
          do_say ( questman, buf );
          return;
     }

     send_to_char
          ( "QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",
            ch );
     send_to_char ( "For more information, type 'HELP QUEST'.\n\r",
                    ch );
     return;
}

void generate_quest ( CHAR_DATA * ch, CHAR_DATA * questman )
{
     CHAR_DATA          *victim;
     ROOM_INDEX_DATA    *room;
     OBJ_DATA           *questitem;
     char                buf[MAX_STRING_LENGTH];

     /*  Randomly selects a mob from the world mob list. If you don't
      * want a mob to be selected, make sure it is immune to summon.
      * Or, you could add a new mob flag called ACT_NOQUEST. The mob
      * is selected for both mob and obj quests, even tho in the obj
      * quest the mob is not used. This is done to assure the level
      * of difficulty for the area isn't too great for the player. */

     for ( victim = char_list; victim != NULL; victim = victim->next )
     {
          if ( !IS_NPC ( victim ) )
               continue;

          if ( quest_level_diff ( ch->level, victim->level ) == TRUE
               && !IS_SET ( victim->imm_flags, IMM_SUMMON )
               && victim->pIndexData != NULL
               && victim->pIndexData->pShop == NULL
               && !IS_SET ( victim->act, ACT_PET )
               && !IS_SET ( victim->act, ACT_NOQUEST )
               && !IS_SET ( victim->in_room->room_flags, ROOM_PRIVATE   )
               && !IS_SET ( victim->in_room->room_flags, ROOM_GODS_ONLY )
               && !IS_SET ( victim->in_room->room_flags, ROOM_SOLITARY  )
               && !IS_SET ( victim->in_room->room_flags, ROOM_SAFE      )
               && !IS_SET ( victim->in_room->room_flags, ROOM_PET_SHOP  )
               && !IS_SET ( victim->in_room->room_flags, ROOM_IMP_ONLY  )
               && !IS_SET ( victim->affected_by,         AFF_CHARM      )
               && ( victim->in_room->area->llev >= ch->level )
               && chance ( 15 ) )
               break;
     }

     if ( victim == NULL )
     {
          do_say ( questman, "I'm sorry, but I don't have any quests for you at this time." );
          do_say ( questman, "Try again later." );
          ch->pcdata->nextquest = 1;
          return;
     }

     if ( ( room = find_location ( ch, victim->name ) ) == NULL )
     {
          SNP ( buf, "I'm sorry, but I don't have any quests for you at this time." );
          do_say ( questman, buf );
          SNP ( buf, "Try again later." );
          do_say ( questman, buf );
          ch->pcdata->nextquest = 1;
          return;
     }

    /*  10% chance it will send the player on a 'recover item' quest. */

     if ( chance ( 10 ) )
     {
          int                 objvnum = 0;

          switch ( number_range ( 0, 4 ) )
          {
          case 0:
               objvnum = QUEST_OBJQUEST1;
               break;

          case 1:
               objvnum = QUEST_OBJQUEST2;
               break;

          case 2:
               objvnum = QUEST_OBJQUEST3;
               break;

          case 3:
               objvnum = QUEST_OBJQUEST4;
               break;

          case 4:
               objvnum = QUEST_OBJQUEST5;
               break;
          }

          questitem =
               create_object ( get_obj_index ( objvnum ), ch->level );
          obj_to_room ( questitem, room );
          ch->pcdata->questobj = questitem->pIndexData->vnum;

          SNP ( buf,
                "Vile pilferers have stolen %s from the royal treasury!",
                questitem->short_descr );
          do_say ( questman, buf );
          do_say ( questman,
                   "My court wizardess, with her magic mirror, has pinpointed its location." );

	/* I changed my area names so that they have just the name of the area
	 * and none of the level stuff. You may want to comment these next two
	 * lines. - Vassago */

          SNP ( buf, "Look in the general area of %s for %s!",
                room->area->name, room->name );
          do_say ( questman, buf );
          return;
     }

    /* Quest to kill a mob */

     else
     {
          switch ( number_range ( 0, 1 ) )
          {
          case 0:
               SNP ( buf,
                     "An enemy of mine, %s, is making vile threats against the crown.",
                     victim->short_descr );
               do_say ( questman, buf );
               SNP ( buf, "This threat must be eliminated!" );
               do_say ( questman, buf );
               break;

          case 1:
               SNP ( buf,
                     "The most heinous criminal, %s, has escaped from the dungeon!",
                     victim->short_descr );
               do_say ( questman, buf );
               SNP ( buf,
                     "Since the escape, %s has murdered %d civillians!",
                     victim->short_descr, number_range ( 2,
                                                         20 ) );
               do_say ( questman, buf );
               do_say ( questman,
                        "The penalty for this crime is death, and you are to deliver the sentence!" );
               break;
          }

          if ( room->name != NULL )
          {
               SNP ( buf,
                     "Seek %s out somewhere in the vicinity of %s!",
                     victim->short_descr, room->name );
               do_say ( questman, buf );

	    /* I changed my area names so that they have just the name of the area
	     * and none of the level stuff. You may want to comment these next two
	     * lines. - Vassago */

               SNP ( buf,
                     "That location is in the general area of %s.",
                     room->area->name );
               do_say ( questman, buf );
          }
          ch->pcdata->questmob = victim->pIndexData->vnum;
     }
     return;
}

/* Level differences to search for. Moongate has 350
   levels, so you will want to tweak these greater or
   less than statements for yourself. - Vassago */

bool quest_level_diff ( int clevel, int mlevel )
{

/* rewrote level diff to a much more simple format - Lotherius */

     if ( abs(mlevel - clevel) <= 5 )
          return TRUE;
     else
          if ( abs(mlevel - clevel) <= 8 && clevel >= 30 )
               return TRUE;
     else
          if ( abs(mlevel - clevel) <= 10 && clevel >= 55 )
               return TRUE;
     else
          if ( abs(mlevel - clevel) <= 13 && clevel >= 85 )
               return TRUE;
     else
          return FALSE;
}

/* Called from update_handler() by pulse_area */

void quest_update ( void )
{
     DESCRIPTOR_DATA    *d;
     CHAR_DATA          *ch;

     for ( d = descriptor_list; d != NULL; d = d->next )
     {
          if ( d->character != NULL && d->connected == CON_PLAYING )
          {

               ch = d->character;

               if ( ch->pcdata->nextquest > 0 )
               {
                    ch->pcdata->nextquest--;
                    if ( ch->pcdata->nextquest == 0 )
                    {
                         send_to_char ( "{RYou may now {Yquest {Ragain{w.\n\r", ch );
                         return;
                    }
               }
               else if ( IS_SET ( ch->act, PLR_QUESTOR ) )
               {
                    if ( --ch->pcdata->countdown <= 0 )
                    {
                         ch->pcdata->nextquest = 15;
                         form_to_char ( ch, 
                               "You have run out of time for your quest!\n\r"
                               "You may quest again in %d minutes.\n\r",
                               ch->pcdata->nextquest );
                         REMOVE_BIT ( ch->act, PLR_QUESTOR );
                         ch->pcdata->questgiver = NULL;
                         ch->pcdata->countdown = 0;
                         ch->pcdata->questmob = 0;
                    }
                    if ( ch->pcdata->countdown > 0 &&
                         ch->pcdata->countdown < 6 )
                    {
                         send_to_char
                              ( "Better hurry, you're almost out of time for your quest!\n\r",
                                ch );
                         return;
                    }
               }
          }
     }
     return;
}
