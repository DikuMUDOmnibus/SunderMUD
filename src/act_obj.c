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
DECLARE_DO_FUN ( do_yell );
DECLARE_DO_FUN ( do_say );
void do_split      args ( ( CHAR_DATA *ch, char *argument, bool tax ) );

/*
 * Local functions.
 */

bool remove_obj    args ( ( CHAR_DATA * ch, int iWear, bool fReplace ) );
void wear_obj      args ( ( CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace ) );
CHAR_DATA          *find_keeper args ( ( CHAR_DATA * ch ) );
int get_cost       args ( ( CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy ) );

/* Lotherius added 2 next lines for new shop code */

void obj_to_keeper     args ( ( OBJ_DATA * obj, CHAR_DATA * ch ) );
OBJ_DATA               *get_obj_keeper   args ( ( CHAR_DATA * ch, CHAR_DATA * keeper, char *argument ) );
bool belongs           args ( ( CHAR_DATA *ch, OBJ_DATA *obj) );
char *obj_cond         args ( ( OBJ_DATA * obj ) );
void check_damage_obj  args ( ( CHAR_DATA * ch, OBJ_DATA * obj, int chance, int damtype ) );
void damage_obj        args ( ( CHAR_DATA * ch, OBJ_DATA * obj, int damage, int damtype ) );
void set_obj_cond      args ( ( OBJ_DATA * obj, int condition ) );

/* Lotherius - taxation. Characters will be payed amount - any applicable taxes
 * Don't exclude NPC's... They pay taxes too if they get gold with do_pay
 * Send any message about the character getting gold like normal, this will only
 * inform of any taxes paid.
 */

void do_pay ( CHAR_DATA *ch, int amount )
{
     int tax;

     if ( amount <= 0 ) // No tax on negative amounts
     {
          ch->gold += amount;
          return;
     }

     // All non-clan taxes should be added above here.
     //
     if ( IS_NPC ( ch ) )
          return;

     if ( !ch->pcdata->clan )
     {
          ch->gold += amount;
          return;
     }
     // Below here assumes ch->pcdata->clan == TRUE
     // Do clan taxation
     //
     if ( ch->pcdata->clan->clanmtax > 0 )
     {
          tax = amount * ch->pcdata->clan->clanmtax / 100;
          if ( tax > 0 )
          {
               // Below is an example of some of the wierd mistakes the compiler DOESN'T catch...
               // send_to_char ( "You are charged ${Y%d{x tax by your clan.\n\r", ch );
               // Which I did all too often, which is why I wrote the following:
               form_to_char ( ch, "You are charged ${Y%d{x tax by your clan.\n\r", tax );
               amount -= tax;
               ch->pcdata->clan->clanbank += tax;
          }
     }

     ch->gold += amount;
     return;
}

/* RT part of the corpse looting code */
/* Lotherius - extended.. */

bool can_loot ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     CHAR_DATA          *owner, *wch;

     if ( IS_IMMORTAL ( ch ) )
          return TRUE;
     if ( !obj->owner || obj->owner == NULL )
          return TRUE;
     owner = NULL;
     for ( wch = char_list; wch != NULL; wch = wch->next )
          if ( !str_cmp ( wch->name, obj->owner ) )
               owner = wch;

     if ( owner == NULL )
          return TRUE;

     if ( !str_cmp ( ch->name, owner->name ) )
          return TRUE;

     if ( !IS_NPC ( owner ) && IS_SET ( owner->act, PLR_CANLOOT ) )
          return TRUE;

     if ( is_same_group ( ch, owner ) )
          return TRUE;

     return FALSE;
}

void get_obj ( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container )
{
     CHAR_DATA          *gch;
     int                 members;

     if ( !CAN_WEAR ( obj, ITEM_TAKE ) )
     {
          send_to_char ( "You can't take that.\n\r", ch );
          return;
     }

     if (obj->owner && IS_NPC(ch) )
          return;

     if ( ch->carry_number + get_obj_number ( obj ) > can_carry_n ( ch ) )
     {
          act ( "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
          return;
     }

     if ( ch->carry_weight + get_obj_weight ( obj ) > can_carry_w ( ch ) )
     {
          act ( "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
          return;
     }

     if ( !can_loot ( ch, obj ) )
          return;

     if ( obj->in_room != NULL )
     {
          for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
               if ( gch->on == obj )
               {
                    act ( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
                    return;
               }
     }

     if ( container != NULL )
     {
          if ( container->pIndexData->vnum == OBJ_VNUM_PIT && get_trust ( ch ) < obj->level )
          {
               send_to_char ( "You are not powerful enough to use it.\n\r", ch );
               return;
          }
          if ( container->pIndexData->vnum == OBJ_VNUM_PIT && !CAN_WEAR ( container, ITEM_TAKE ) && obj->timer )
               obj->timer = 0;
          act ( "You get $p from $P.", ch, obj, container, TO_CHAR );
          act ( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
          obj_from_obj ( obj );
     }
     else
     {
          act ( "You get $p.", ch, obj, container, TO_CHAR );
          act ( "$n gets $p.", ch, obj, container, TO_ROOM );
          obj_from_room ( obj );
     }
     if ( obj->item_type == ITEM_MONEY )
     {
          if ( !IS_SET ( ch->act, PLR_AUTOSPLIT ) )
               do_pay ( ch, obj->value[0] );
          else
          {			/* AUTOSPLIT code */
               members = 0;
               for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
               {
                    if ( is_same_group ( gch, ch ) )
                         members++;
               }

               if ( members > 1 && obj->value[0] > 1 )
               {
                    ch->gold += obj->value[0];
                    // itos is my int to char cheat.
                    do_split ( ch, itos(obj->value[0]), TRUE );
               }
               else
                    do_pay ( ch, obj->value[0] );
          }
          extract_obj ( obj );
     }
     else
     {
          obj_to_char ( obj, ch );
          if ( HAS_TRIGGER_OBJ( obj, TRIG_GET ) )
               p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GET );
          if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GET ) )
               p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GET );
     }

     return;
}

void do_get ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;
     OBJ_DATA           *container;
     bool                found;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( !str_cmp ( arg2, "from" ) )
          argument = one_argument ( argument, arg2 );

    /* Get type. */
     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Get what?\n\r", ch );
          return;
     }

     if ( arg2[0] == '\0' )
     {
          if ( str_cmp ( arg1, "all" ) &&
               str_prefix ( "all.", arg1 ) )
          {
	    /* 'get obj' */
               obj = get_obj_list ( ch, arg1, ch->in_room->contents );
               if ( obj == NULL )
               {
                    act ( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
                    return;
               }
               get_obj ( ch, obj, NULL );
          }
          else
          {
	    /* 'get all' or 'get all.obj' */
               found = FALSE;
               for ( obj = ch->in_room->contents; obj != NULL;
                     obj = obj_next )
               {
                    obj_next = obj->next_content;
                    if ( ( arg1[3] == '\0' ||
                           is_name ( &arg1[4], obj->name ) ||
                           is_name_abbv ( &arg1[4], obj->name ) ) &&
                         can_see_obj ( ch, obj ) )
                    {
                         found = TRUE;
                         get_obj ( ch, obj, NULL );
                    }
               }

               if ( !found )
               {
                    if ( arg1[3] == '\0' )
                         send_to_char ( "I see nothing here.\n\r", ch );
                    else
                         act ( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
               }
          }
     }
     else
     {
	/* 'get ... container' */
          if ( !str_cmp ( arg2, "all" ) ||
               !str_prefix ( "all.", arg2 ) )
          {
               send_to_char ( "You can't do that.\n\r", ch );
               return;
          }

          if ( ( container = get_obj_here ( ch, NULL, arg2 ) ) == NULL )
          {
               act ( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
               return;
          }

          switch ( container->item_type )
          {
          default:
               send_to_char ( "That's not a container.\n\r", ch );
               return;

          case ITEM_CONTAINER:
          case ITEM_CORPSE_NPC:
               break;

          case ITEM_CORPSE_PC:
               {

                    if ( !can_loot ( ch, container ) )
                    {
                         send_to_char ( "You can't do that.\n\r", ch );
                         return;
                    }
               }
          }

          if ( IS_SET ( container->value[1], CONT_CLOSED ) )
          {
               act ( "The $d is closed.", ch, NULL, container->name,
                     TO_CHAR );
               return;
          }

          if ( str_cmp ( arg1, "all" ) &&
               str_prefix ( "all.", arg1 ) )
          {
	    /* 'get obj container' */
               obj = get_obj_list ( ch, arg1, container->contains );
               if ( obj == NULL )
               {
                    act ( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
                    return;
               }
               get_obj ( ch, obj, container );
          }
          else
          {
	    /* 'get all container' or 'get all.obj container' */
               found = FALSE;
               for ( obj = container->contains; obj != NULL; obj = obj_next )
               {
                    obj_next = obj->next_content;
                    if ( ( arg1[3] == '\0' ||
                           is_name ( &arg1[4], obj->name ) ||
                           is_name_abbv ( &arg1[4], obj->name ) ) &&
                         can_see_obj ( ch, obj ) )
                    {
                         found = TRUE;
                         if ( container->pIndexData->vnum ==
                              OBJ_VNUM_PIT && !IS_IMMORTAL ( ch ) )
                         {
                              send_to_char ( "Don't be so greedy!\n\r", ch );
                              return;
                         }
                         get_obj ( ch, obj, container );
                    }
               }

               if ( !found )
               {
                    if ( arg1[3] == '\0' )
                         act ( "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR );
                    else
                         act ( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR );
               }
          }
     }

     return;
}

void do_put ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *container;
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( !str_cmp ( arg2, "in" ) )
          argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char ( "Put what in what?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg2, "all" ) || !str_prefix ( "all.", arg2 ) )
     {
          send_to_char ( "You can't do that.\n\r", ch );
          return;
     }

     if ( ( container = get_obj_here ( ch, NULL, arg2 ) ) == NULL )
     {
          act ( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
          return;
     }

     if ( container->item_type != ITEM_CONTAINER )
     {
          send_to_char ( "That's not a container.\n\r", ch );
          return;
     }

     if ( IS_SET ( container->value[1], CONT_CLOSED ) )
     {
          act ( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
          return;
     }

     if ( str_cmp ( arg1, "all" ) && str_prefix ( "all.", arg1 ) )
     {
	/* 'put obj container' */
          if ( ( obj = get_obj_carry ( ch, arg1, NULL ) ) == NULL )
          {
               send_to_char ( "You do not have that item.\n\r", ch );
               return;
          }

          if ( obj == container )
          {
               send_to_char ( "You can't fold it into itself.\n\r", ch );
               return;
          }

          if ( !can_drop_obj ( ch, obj ) )
          {
               send_to_char ( "You can't let go of it.\n\r", ch );
               return;
          }

          if ( get_obj_weight ( obj ) + get_obj_weight ( container )
               > container->value[0] )
          {
               send_to_char ( "It won't fit.\n\r", ch );
               return;
          }

          if ( container->pIndexData->vnum == OBJ_VNUM_PIT
               && !CAN_WEAR ( container, ITEM_TAKE ) )
          {
               if ( obj->timer )
               {
                    send_to_char( "Only permanent items may go in the pit.\n\r", ch );
                   return;
               }
              else
                  obj->timer = number_range ( 100, 200 ); /* This was outside the following bracket before, instead of inside -- oopsie */
          }

          obj_from_char ( obj );
          obj_to_obj ( obj, container );
          act ( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
          act ( "You put $p in $P.", ch, obj, container, TO_CHAR );
     }
     else
     {
	/* 'put all container' or 'put all.obj container' */
          for ( obj = ch->carrying; obj != NULL; obj = obj_next )
          {
               obj_next = obj->next_content;

               if ( ( arg1[3] == '\0'
                      || is_name ( &arg1[4], obj->name )
                      || is_name_abbv ( &arg1[4], obj->name ) )
                    && can_see_obj ( ch, obj )
                    && obj->wear_loc == WEAR_NONE && obj != container
                    && can_drop_obj ( ch, obj )
                    && get_obj_weight ( obj ) + get_obj_weight ( container ) <= container->value[0] )
               {
                    if ( container->pIndexData->vnum == OBJ_VNUM_PIT && !CAN_WEAR ( obj, ITEM_TAKE ) )
                    {
                         if ( obj->timer )
                              continue;
                         else
                              obj->timer = number_range ( 100, 200 );
                    }
                    obj_from_char ( obj );
                    obj_to_obj ( obj, container );
                    act ( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
                    act ( "You put $p in $P.", ch, obj, container, TO_CHAR );
               }
          }
     }

     return;
}

void do_drop ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;
     bool                found;

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Drop what?\n\r", ch );
          return;
     }

     if ( is_number ( arg ) )
     {
	/* 'drop NNNN coins' */
          int                 amount;

          amount = atoi ( arg );
          argument = one_argument ( argument, arg );
          if ( amount <= 0
               || ( str_cmp ( arg, "coins" ) &&
                    str_cmp ( arg, "coin" ) &&
                    str_cmp ( arg, "gold" ) ) )
          {
               send_to_char ( "Sorry, you can't do that.\n\r", ch );
               return;
          }

          if ( ch->gold < amount )
          {
               send_to_char ( "You haven't got that many coins.\n\r", ch );
               return;
          }

          ch->gold -= amount;

          for ( obj = ch->in_room->contents; obj != NULL;
                obj = obj_next )
          {
               obj_next = obj->next_content;

               switch ( obj->pIndexData->vnum )
               {
               case OBJ_VNUM_MONEY_ONE:
                    amount += 1;
                    extract_obj ( obj );
                    break;
               case OBJ_VNUM_MONEY_SOME:
                    amount += obj->value[0];
                    extract_obj ( obj );
                    break;
               }
          }
          // Well you COULD put an obj script on created money...
          // and rooms definitely could respond to dropped money.
          obj_to_room ( create_money ( amount ), ch->in_room );
          if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
               p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
          if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
               p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
          act ( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
          send_to_char ( "OK.\n\r", ch );
          return;
     }

     if ( str_cmp ( arg, "all" ) && str_prefix ( "all.", arg ) )
     {
	/* 'drop obj' */
          if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
          {
               send_to_char ( "You do not have that item.\n\r", ch );
               return;
          }

          if ( !can_drop_obj ( ch, obj ) )
          {
               send_to_char ( "You can't let go of it.\n\r", ch );
               return;
          }

          obj_from_char ( obj );
          obj_to_room ( obj, ch->in_room );
          act ( "$n drops $p.", ch, obj, NULL, TO_ROOM );
          act ( "You drop $p.", ch, obj, NULL, TO_CHAR );
          if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
               p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
          if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
               p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
     }
     else
     {
	/* 'drop all' or 'drop all.obj' */
          found = FALSE;
          for ( obj = ch->carrying; obj != NULL; obj = obj_next )
          {
               obj_next = obj->next_content;

               if ( ( arg[3] == '\0' || is_name ( &arg[4], obj->name )
                      || is_name_abbv ( &arg[4], obj->name ) )
                    && can_see_obj ( ch, obj )
                    && obj->wear_loc == WEAR_NONE
                    && can_drop_obj ( ch, obj ) )
               {
                    found = TRUE;
                    obj_from_char ( obj );
                    obj_to_room ( obj, ch->in_room );
                    act ( "$n drops $p.", ch, obj, NULL, TO_ROOM );
                    act ( "You drop $p.", ch, obj, NULL, TO_CHAR );
                    if ( HAS_TRIGGER_OBJ( obj, TRIG_DROP ) )
                         p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_DROP );
                    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DROP ) )
                         p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_DROP );
               }
          }
          if ( !found )
          {
               if ( arg[3] == '\0' )
                    act ( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
               else
                    act ( "You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR );
          }
     }

     return;
}

void do_give ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MIL];
     char                arg2[MIL];
     char                buf[MIL];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char ( "Give what to whom?\n\r", ch );
          return;
     }

     if ( is_number ( arg1 ) )
     {
	/* 'give NNNN coins victim' */
          int                 amount;

          amount = atoi ( arg1 );
          if ( amount <= 0 || ( str_cmp ( arg2, "coins" ) &&  str_cmp ( arg2, "coin" ) &&
                                str_cmp ( arg2, "gold" ) ) )
          {
               send_to_char ( "Sorry, you can't do that.\n\r", ch );
               return;
          }

          argument = one_argument ( argument, arg2 );
          if ( arg2[0] == '\0' )
          {
               send_to_char ( "Give what to whom?\n\r", ch );
               return;
          }

          if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL )
          {
               send_to_char ( "They aren't here.\n\r", ch );
               return;
          }

          if ( ch->gold < amount )
          {
               send_to_char ( "You haven't got that much gold.\n\r", ch );
               return;
          }

          ch->gold -= amount;
          victim->gold += amount;

          SNP ( buf, "$n gives you %d gold.", amount );
          act ( buf, ch, NULL, victim, TO_VICT );
          act ( "$n gives $N some gold.", ch, NULL, victim, TO_NOTVICT );
          SNP ( buf, "You give $N %d gold.", amount );
          act ( buf, ch, NULL, victim, TO_CHAR );

          /*
           * Bribe trigger
           */
          if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_BRIBE ) )
               p_bribe_trigger( victim, ch, amount );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg1, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     if ( obj->wear_loc != WEAR_NONE )
     {
          send_to_char ( "You must remove it first.\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }
     /* Players & charmed mobs may not give to a shop. All others may. */
     if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
     {
          if (!IS_NPC(ch) )
          {
               act("$N tells you 'Sorry, you'll have to sell that.'", ch,NULL,victim,TO_CHAR);
               ch->reply = victim;
               return;
          }
          else
          {
               if (IS_AFFECTED ( ch, AFF_CHARM ) )
               {
                    act ("$N tells $n that $s master needs to get a life.", ch,NULL,victim,TO_NOTVICT);
                    return;
               }
          }
     }
     if ( !can_drop_obj ( ch, obj ) )
     {
          send_to_char ( "You can't let go of it.\n\r", ch );
          return;
     }

     if ( victim->carry_number + get_obj_number ( obj ) > can_carry_n ( victim ) )
     {
          act ( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( victim->carry_weight + get_obj_weight ( obj ) > can_carry_w ( victim ) )
     {
          act ( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( !can_see_obj ( victim, obj ) )
     {
          act ( "$N can't see it.", ch, NULL, victim, TO_CHAR );
          return;
     }

     obj_from_char ( obj );
     obj_to_char ( obj, victim );

     MOBtrigger = FALSE;
     act ( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
     act ( "$n gives you $p.", ch, obj, victim, TO_VICT );
     act ( "You give $p to $N.", ch, obj, victim, TO_CHAR );
     MOBtrigger = TRUE;
     /*
      * Give trigger
      */
     if ( IS_NPC(victim) && HAS_TRIGGER_MOB( victim, TRIG_GIVE ) )
          p_give_trigger( victim, NULL, NULL, ch, obj, TRIG_GIVE );
     if ( HAS_TRIGGER_OBJ( obj, TRIG_GIVE ) )
          p_give_trigger( NULL, obj, NULL, ch, obj, TRIG_GIVE );
     if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_GIVE ) )
          p_give_trigger( NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE );

     return;
}

/* envenom added Lotherius */

/* for poisoning weapons and food/drink */
void do_envenom ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj;
     int                 percent, skill;
     int                 brand;

     /* find out what */
     if ( argument == '\0' )
     {
          send_to_char ( "Envenom what item?\n\r", ch );
          return;
     }

     obj = get_obj_list ( ch, argument, ch->carrying );

     if ( obj == NULL )
     {
          send_to_char ( "You don't have that item.\n\r", ch );
          return;
     }

     if ( ( skill = get_skill ( ch, gsn_envenom ) ) < 1 )
     {
          send_to_char ( "Are you crazy? You'd poison yourself!\n\r", ch );
          return;
     }

     if ( obj->item_type == ITEM_FOOD ||
          obj->item_type == ITEM_DRINK_CON )
     {
          if ( IS_OBJ_STAT ( obj, ITEM_BLESS ) || obj->owner )
          {
               act ( "You fail to poison $p.", ch, obj, NULL, TO_CHAR );
               return;
          }

          if ( number_percent (  ) < skill )	/* success! */
          {
               act ( "$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM );
               act ( "You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR );
               if ( !obj->value[3] )
               {
                    obj->value[3] = 1;
                    check_improve ( ch, gsn_envenom, TRUE, 4 );
               }
               WAIT_STATE ( ch, skill_table[gsn_envenom].beats );
               return;
          }

          act ( "You fail to poison $p.", ch, obj, NULL, TO_CHAR );
          if ( !obj->value[3] )
               check_improve ( ch, gsn_envenom, FALSE, 4 );
          WAIT_STATE ( ch, skill_table[gsn_envenom].beats );
          return;
     }

     if ( obj->item_type == ITEM_WEAPON )
     {
          if ( IS_WEAPON_STAT ( obj, WEAPON_FLAMING )
               || IS_WEAPON_STAT ( obj, WEAPON_FROST )
               || IS_WEAPON_STAT ( obj, WEAPON_VAMPIRIC )
               || IS_WEAPON_STAT ( obj, WEAPON_SHARP )
               || IS_WEAPON_STAT ( obj, WEAPON_VORPAL )
               || IS_WEAPON_STAT ( obj, WEAPON_ACID )
               || IS_WEAPON_STAT ( obj, WEAPON_LIGHTNING )
               || IS_OBJ_STAT ( obj, ITEM_BLESS ) )
          {
               act ( "You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR );
               return;
          }

          if ( obj->value[3] < 0
               || attack_table[obj->value[3]].damage == DAM_BASH )
          {
               send_to_char ( "You can only envenom edged weapons.\n\r", ch );
               return;
          }

          if ( IS_WEAPON_STAT ( obj, WEAPON_POISON ) )
          {
               act ( "$p is already envenomed.", ch, obj, NULL, TO_CHAR );
               return;
          }

          percent = number_percent (  );
          if ( percent < skill )
          {
               brand = WEAPON_POISON;
               obj->value[4] += brand;
               act ( "$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM );
               act ( "You coat $p with venom.", ch, obj, NULL, TO_CHAR );
               check_improve ( ch, gsn_envenom, TRUE, 3 );
               WAIT_STATE ( ch, skill_table[gsn_envenom].beats );
               return;
          }
          else
          {
               act ( "You fail to envenom $p.", ch, obj, NULL, TO_CHAR );
               check_improve ( ch, gsn_envenom, FALSE, 3 );
               WAIT_STATE ( ch, skill_table[gsn_envenom].beats );
               return;
          }
     }

     act ( "You can't poison $p.", ch, obj, NULL, TO_CHAR );
     return;
}

void do_fill ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA           *fountain;
     bool                found;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Fill what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     found = FALSE;
     for ( fountain = ch->in_room->contents; fountain != NULL;
           fountain = fountain->next_content )
     {
          if ( fountain->item_type == ITEM_FOUNTAIN )
          {
               found = TRUE;
               break;
          }
     }

     if ( !found )
     {
          send_to_char ( "There is no fountain here!\n\r", ch );
          return;
     }

     if ( obj->item_type != ITEM_DRINK_CON )
     {
          send_to_char ( "You can't fill that.\n\r", ch );
          return;
     }

     if ( obj->value[1] != 0 && obj->value[2] != 0 )
     {
          send_to_char ( "There is a different kind of liquid in it.\n\r", ch );
          return;
     }

     if ( obj->value[1] >= obj->value[0] )
     {
          send_to_char ( "Your container is full.\n\r", ch );
          return;
     }

     act ( "You fill $p.", ch, obj, NULL, TO_CHAR );
     obj->value[2] = 0;
     obj->value[1] = obj->value[0];
     return;
}

void do_drink ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 amount;
     int                 liquid;

     // Why one_argument?
     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
          {
               if ( obj->item_type == ITEM_FOUNTAIN )
                    break;
          }

          if ( obj == NULL )
          {
               send_to_char ( "Drink what?\n\r", ch );
               return;
          }
     }
     else
     {
          if ( ( obj = get_obj_here ( ch, NULL, arg ) ) == NULL )
          {
               send_to_char ( "You can't find it.\n\r", ch );
               return;
          }
     }

     if ( !IS_NPC ( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
     {
          send_to_char ( "You fail to reach your mouth.  *Hic*\n\r", ch );
          return;
     }

     /* I guess I was thinking of owned drink containers?? */
     if (obj->owner && !belongs(ch, obj) )
     {
          send_to_char( "The liquid does not quench your thirst.\n\r", ch);
          return;
     }

     switch ( obj->item_type )
     {
     default:
          send_to_char ( "You can't drink from that.\n\r", ch );
          break;
     case ITEM_FOUNTAIN:
          if ( !IS_NPC ( ch ) )
               ch->pcdata->condition[COND_THIRST] = 48;
          act ( "$n drinks from $p.", ch, obj, NULL, TO_ROOM );
          send_to_char ( "You are no longer thirsty.\n\r", ch );
          break;
     case ITEM_DRINK_CON:
          if ( obj->value[1] <= 0 )
          {
               send_to_char ( "It is already empty.\n\r", ch );
               return;
          }
          if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
          {
               bugf ( "Do_drink: bad liquid number %d.", liquid );
               liquid = obj->value[2] = 0;
          }

          act ( "$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM );
          act ( "You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR );

          amount = number_range ( 3, 10 );
          amount = UMIN ( amount, obj->value[1] );

          gain_condition ( ch, COND_DRUNK, amount * liq_table[liquid].liq_affect[COND_DRUNK] );
          gain_condition ( ch, COND_FULL, amount * liq_table[liquid].liq_affect[COND_FULL] );
          gain_condition ( ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] );

          if ( !IS_NPC ( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
               send_to_char ( "You feel drunk.\n\r", ch );
          if ( !IS_NPC ( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
               send_to_char ( "You are full.\n\r", ch );
          if ( !IS_NPC ( ch ) && ch->pcdata->condition[COND_THIRST] > 40 )
               send_to_char ( "You do not feel thirsty.\n\r", ch );

          if ( obj->value[3] != 0 )
          {
	    /* The shit was poisoned ! */
               AFFECT_DATA         af;

               act ( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
               send_to_char ( "You feel sick.\n\r", ch );
               af.type = gsn_poison;
               af.level = number_fuzzy ( amount );
               af.duration = 3 * amount;
               af.location = APPLY_NONE;
               af.modifier = 0;
               af.bitvector = AFF_POISON;
               affect_join ( ch, &af );
          }

          obj->value[1] -= amount;
          break;
     }
     return;
}

void do_eat ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Eat what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     if ( !IS_IMMORTAL ( ch ) )
     {
          if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
          {
               send_to_char ( "You can't eat that.\n\r", ch );
               return;
          }

          if ( !IS_NPC ( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
          {
               send_to_char ( "You are too full to eat more.\n\r", ch );
               return;
          }
     }

     act ( "$n eats $p.", ch, obj, NULL, TO_ROOM );
     act ( "You eat $p.", ch, obj, NULL, TO_CHAR );

     switch ( obj->item_type )
     {
     case ITEM_FOOD:
          if ( !IS_NPC ( ch ) )
          {
               int                 condition;

               condition = ch->pcdata->condition[COND_FULL];
               gain_condition ( ch, COND_FULL, obj->value[0] );
               if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
                    send_to_char ( "You are no longer hungry.\n\r", ch );
               else if ( ch->pcdata->condition[COND_FULL] > 40 )
                    send_to_char ( "You are full.\n\r", ch );
          }

          if ( obj->value[3] != 0 )
          {
	    /* The shit was poisoned! */
               AFFECT_DATA         af;

               act ( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
               send_to_char ( "You feel sick.\n\r", ch );

               af.type = gsn_poison;
               af.level = number_fuzzy ( obj->value[0] );
               af.duration = 2 * obj->value[0];
               af.location = APPLY_NONE;
               af.modifier = 0;
               af.bitvector = AFF_POISON;
               affect_join ( ch, &af );
          }
          break;

     case ITEM_PILL:
          obj_cast_spell ( obj->value[1], obj->value[0], ch, ch, NULL );
          obj_cast_spell ( obj->value[2], obj->value[0], ch, ch, NULL );
          obj_cast_spell ( obj->value[3], obj->value[0], ch, ch, NULL );
          break;
     }

     extract_obj ( obj );
     return;
}

/*
 * Remove an object.
 */
bool remove_obj ( CHAR_DATA * ch, int iWear, bool fReplace )
{
     OBJ_DATA           *obj;

     if ( ( obj = get_eq_char ( ch, iWear ) ) == NULL )
          return TRUE;

     if ( !fReplace )
          return FALSE;

     if ( IS_SET ( obj->extra_flags, ITEM_NOREMOVE ) )
     {
          act ( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
          return FALSE;
     }

     if ( unequip_char ( ch, obj ) )
     {
          if ( HAS_TRIGGER_OBJ( obj, TRIG_REMOVE ) )
               p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_REMOVE );
          else
          {
               act ( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
               act ( "You stop using $p.", ch, obj, NULL, TO_CHAR );
          }
     }

     return TRUE;
}

/*
 * New wear_obj by Lotherius - Still wears one object, hopefully less haphazard :)
 * Option: fReplace = Replace existing objects or not
 *
 * Notes:
 *    - TRIG_WEAR:  Doesn't block equipping the object. Rather, it replaces the act messages.
 *                  Builders wishing to prevent wearing must remove the object explicitly.
 *
 *    - wear_flags: Builders must take care not to frivolously add too many wear flags.
 *                  The first flag matched in most cases (light & hold are treated specially)
 *                  will be the one attempted. Some of these cases are now checked for in
 *                  db.c - watch the logfile for related bug notices.
 */
void wear_obj ( CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace )
{
     OBJ_DATA	       *weapon;
     OBJ_DATA	       *tmp;
     int                sn, skill;
     bool 		wmatch = FALSE;
     int		wdata;	// Which wear_info slot to use...

     // Let's first check some failures.
     //
     switch ( obj->item_type )
     {
     case ITEM_FURNITURE:
     case ITEM_TRASH:
     case ITEM_BOAT:
     case ITEM_MONEY:
          {
               act ( "$n can't seem to figure out how to use $p.", ch, obj, NULL, TO_ROOM );
               act ( "You can't wear, wield, or use $p.", ch, obj, NULL, TO_CHAR );
               return;
          }
     case ITEM_WEAPON:
          if ( !IS_NPC ( ch ) && ch->pcdata->pclass == class_lookup ( "monk" ) )
          {
               send_to_char ( "Your code does not allow you to take up a weapon.\n\r", ch );
               return;
          }
     case ITEM_ARMOR:
          if ( IS_SET ( obj->wear_flags, ITEM_WEAR_SHIELD ) )
          {
               if ( !IS_NPC ( ch ) && ch->pcdata->pclass == class_lookup ( "monk" ) )
               {
                    send_to_char ( "A shield would impair your fighting style too much.\n\r", ch );
                    return;
               }
          }
          break;
     }

     if ( ch->level < obj->level )
     {
          form_to_char ( ch, "You must be level %d to use this object.\n\r", obj->level );
          act ( "$n tries to use $p, but is too inexperienced.",  ch, obj, NULL, TO_ROOM );
          return;
     }

     if (obj->owner && !belongs(ch, obj) )
     {
          form_to_char ( ch, "The %s belongs to %s, and you cannot use it.\n\r", obj->short_descr, obj->owner );
          act ( "$n tries to use $p, but it belongs to someone else.", ch, obj, NULL, TO_ROOM );
          return;
     }

     /* Lotherius - the below left for reference for the day when/if size goes back in. */
     /* Zeran - any item type checked after this point will also be
      * checked for size.   */
     //    if ( !wear_obj_size ( ch, obj ) )
     //	return;
     // Let's get the proper wear_info and associated flags..
     //
     for ( wdata = 0; wdata < MAX_WEAR; wdata++ )
     {
          if ( !IS_SET ( obj->wear_flags, wear_info[wdata].item_flag ) )
               continue;
          else 							// Match
          {
               /* Since there's no ITEM_ flag for lights we have a problem here I'm going to hack around. */
               if ( ( wdata == WEAR_HOLD && obj->item_type == ITEM_LIGHT ) ||
                    ( wdata == WEAR_LIGHT && obj->item_type != ITEM_LIGHT ) )
                    continue;
               
               switch ( wdata )	/* Special cases BEFORE removing an object. */
               {
               case WEAR_FINGER_L: /* Right doesn't need to be checked, since it comes later in the list. */
                    tmp = get_eq_char (ch, WEAR_FINGER_L);
                    if ( tmp )	/* If there is something on the left finger... */
                    {
                         tmp = get_eq_char ( ch, WEAR_FINGER_R ); /* Check for something on the right. */
                         if ( !tmp )	/* Right finger is empty */
                              continue;	/* Skip so that item will be worn on right finger */
                         		/* without removing item on left finger */
                    }
                    break;
               case WEAR_NECK_1: /* neck 2 comes later in the list, doesn't need checked */
                    tmp = get_eq_char (ch, WEAR_NECK_1);
                    if ( tmp )  /* If there is something on Neck 1 */
                    {
                         tmp = get_eq_char ( ch, WEAR_FINGER_R ); /* Check for something on Neck 2 */
                         if ( !tmp )    /* Neck 2 is empty */
                              continue; /* Skip so that item will be worn on Neck 2 */
	                                /* without removing item on left finger */
                    }
                    break;
               case WEAR_WRIST_L: /* Same routine - right is later in list, no checking */
                    tmp = get_eq_char (ch, WEAR_WRIST_L);
                    if ( tmp )  /* If there is something on the left wrist... */
                    {
                         tmp = get_eq_char ( ch, WEAR_WRIST_R ); /* Check for something on the right. */
                         if ( !tmp )    /* Right wrist is empty */
                              continue; /* Skip so that item will be worn on right wrist */
		                        /* without removing item on left wrist */

                    }
                    break;
               default:
                    break;
               }
               
               if ( !remove_obj ( ch, wdata, fReplace ) )	// See if existing can be removed
                    continue; 					// Couldn't wear it here.
                              
               switch ( wdata )					/* Special cases AFTER removing an object */
               {
               case WEAR_HOLD:
                    weapon = get_eq_char(ch,WEAR_WIELD2);
                    if (weapon)
                    {
                         send_to_char("You don't have a free hand.\n\r", ch);
                         return;
                    }
                    weapon = get_eq_char(ch, WEAR_WIELD);
                    if (weapon != NULL && ch->size < SIZE_LARGE &&  IS_SET(weapon->wear_flags, ITEM_TWO_HANDS))
                    {
                         send_to_char("It takes both of your hands just to hold your weapon!\n\r", ch);
                         return;
                    }
                    break;
               case WEAR_WIELD:
                    if ( !IS_NPC ( ch ) && get_obj_weight ( obj ) > str_app[get_curr_stat ( ch, STAT_STR )].wield )
                    {
                         send_to_char ( "It is too heavy for you to wield.\n\r", ch );
                         return;
                    }
                    if ( !IS_NPC ( ch ) && ch->size < SIZE_LARGE && IS_WEAPON_STAT ( obj, WEAPON_TWO_HANDS )
                         && ( get_eq_char ( ch, WEAR_SHIELD ) != NULL
                              || get_eq_char ( ch, WEAR_WIELD2 ) != NULL
                              || get_eq_char ( ch, ITEM_HOLD) != NULL ) )
                    {
                         send_to_char ( "You need two hands free for that weapon.\n\r", ch );
                         return;
                    }
                    break;
               case WEAR_WIELD2:
                    // Special case! Wield must have been checked before wield2... The order of the
                    // wear_info assures that, another reason not to change the order!
                    //
                    // This will direct the player to use the dual command.
                    if (get_skill ( ch, gsn_dual ) > 1)
                    {
                         send_to_char ( "To wield a weapon in your off hand, use the \"dual\" command.\n\r", ch );
                         return;
                    }
                    if ( fReplace )
                         send_to_char ( "You'd be better off with a martini in your off hand.", ch );
                    return;
                    break;
               case WEAR_SHIELD:
                    weapon = get_eq_char ( ch, WEAR_WIELD );
                    if ( weapon != NULL && ch->size < SIZE_LARGE && IS_WEAPON_STAT ( weapon, WEAPON_TWO_HANDS ) )
                    {
                         send_to_char ( "Your weapon is far too large for you to use a shield.\n\r", ch );
                         return;
                    }
                    weapon = get_eq_char ( ch, WEAR_WIELD2 );
                    if ( weapon != NULL )
                    {
                         send_to_char( "A shield would throw off your balance while dual wielding.\n\r", ch );
                         return;
                    }
                    break;
                    /* Special cases have been handled, now continue on with normal procedure. */
               default:
                    break;
               }

               if ( equip_char ( ch, obj, wdata ) )
               {                    				// Successful wearing.
                    wmatch = TRUE;
                    if ( HAS_TRIGGER_OBJ ( obj, TRIG_WEAR ) )
                         p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_WEAR );
                    else
                    {                         		        // Do stuff for the item type.
                         switch ( wear_info[wdata].item_flag )	// Switch by wear location
                         {
                              // Only need case statements for items that won't match the grammer
                              // of the generic act at the end correctly.
                         case ITEM_HOLD:
                              if ( obj->item_type == ITEM_LIGHT )
                              {
                                   act ( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
                                   act ( "You light $p and hold it.", ch, obj, NULL, TO_CHAR );
                              }
                              else
                              {
                                   act ( "$n holds $p in $s hands.", ch, obj, NULL, TO_ROOM );
                                   act ( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
                              }
                              break;
                         case ITEM_WEAR_SHIELD:
                              act ( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
                              act ( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
                              break;
                         case ITEM_WEAR_ABOUT:
                              act ( "$n wears $p about $s body.", ch, obj, NULL,  TO_ROOM );
                              act ( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
                              break;
                         case ITEM_WIELD:
                              act ( "$n wields $p.", ch, obj, NULL, TO_ROOM );
                              act ( "You wield $p.", ch, obj, NULL, TO_CHAR );
                              break;
                         case ITEM_WEAR_PRIDE:
                              act ( "$n wears $p with pride.", ch, obj, NULL, TO_ROOM );
                              act ( "You wear $p with pride.", ch, obj, NULL, TO_CHAR );
                              break;
                         case ITEM_WEAR_FLOAT:
                              act ( "$p starts floating near $n.", ch, obj, NULL, TO_ROOM );
                              act ( "$p starts floating beside you.", ch, obj, NULL, TO_CHAR );
                              break;
                         default:
                              {
                                   char buf[MIL];
                                   SNP ( buf, "$n wears $p on $s %s.", wear_info[wdata].name );
                                   act ( buf, ch, obj, NULL, TO_ROOM );
                                   SNP ( buf, "You wear $p on your %s.", wear_info[wdata].name );
                                   act ( buf, ch, obj, NULL, TO_CHAR );
                              }
                              break;
                         }
                    }
                    // Do some stuff that must be done even if the triggers went off
                    // Using a switch here for future expansion if necessary.
                    switch ( wear_info[wdata].item_flag )
                    {
                    case ITEM_WIELD:
                         sn = get_weapon_sn ( ch, FALSE );
                         if ( sn == gsn_hand_to_hand )
                              break;
                         skill = get_weapon_skill ( ch, sn );
                         if ( skill >= 100 )    act ( "$p feels like a part of you!", ch, obj, NULL, TO_CHAR );
                         else if ( skill > 85 ) act ( "You feel quite confident with $p.", ch, obj,  NULL, TO_CHAR );
                         else if ( skill > 70 ) act ( "You are skilled with $p.", ch, obj, NULL, TO_CHAR );
                         else if ( skill > 50 ) act ( "Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR );
                         else if ( skill > 25 ) act ( "$p feels a little clumsy in your hands.", ch, obj, NULL, TO_CHAR );
                         else if ( skill > 1 )  act ( "You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR );
                         else                   act ( "You don't even know which is end is up on $p.",ch,obj,NULL,TO_CHAR );
                         break;
                    default:
                         break;
                    }
               }
               return; // Successfully removed an object, return now.
          }
          // End of the !removeobj loop
     }
     // End of the for loop
     //
     if ( !wmatch && fReplace )
          send_to_char ( "You can't wear, wield, or hold that.\n\r", ch );
     return;
}

void do_dual ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA	       *weapon;

     one_argument ( argument, arg );

     if (get_skill ( ch, gsn_dual ) <= 1)
     {
          send_to_char ( "You'd be better off with a martini in that hand.\n\r", ch);
          return;
     }

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Dual wield what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     if (obj->owner && !belongs(ch, obj) )
     {
          form_to_char ( ch, "The %s belongs to %s, and you cannot use it.\n\r", obj->short_descr, obj->owner );
          act ( "$n tries to use $p, but it belongs to someone else.", ch, obj, NULL, TO_ROOM );
          return;
     }

     if ( ch->level < obj->level )
     {
          form_to_char ( ch, "You must be level %d to use this weapon.\n\r", obj->level );
          act ( "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM );
          return;
     }

     if ( CAN_WEAR ( obj, ITEM_WIELD ) )
     {
          int                 sn, skill;

          if ( !remove_obj ( ch, WEAR_WIELD2, TRUE ) )
               return;

          if ( !IS_NPC ( ch )
               && get_obj_weight ( obj ) >
               str_app[get_curr_stat ( ch, STAT_STR )].wield )
          {
               send_to_char ( "It is too heavy for you to wield.\n\r", ch );
               return;
          }

          if ( !IS_NPC ( ch ) && ch->size < SIZE_LARGE
               && IS_WEAPON_STAT ( obj, WEAPON_TWO_HANDS )
               && get_eq_char ( ch, WEAR_WIELD ) != NULL )
          {
               send_to_char ( "You need two hands free for that weapon.\n\r", ch );
               return;
          }
          if ( get_eq_char ( ch, WEAR_SHIELD ) != NULL )
          {
               send_to_char ( "A shield on your arm would throw off your balance.!\n\r", ch );
               return;
          }

          if ( get_eq_char ( ch, WEAR_HOLD ) != NULL )
          {
               send_to_char ( "You'll have to put down what you're holding first.\n\r", ch);
               return;
          }

          weapon = get_eq_char(ch,WEAR_WIELD);

          if (!weapon)
          {
               send_to_char ( "You need to be wielding a primary weapon first.\n\r", ch);
               return;
          }

          if (ch->size < SIZE_LARGE && IS_SET(weapon->wear_flags, ITEM_TWO_HANDS))
          {
               send_to_char ( "Your primary weapon is far too large to hold in one hand.\n\r", ch);
               return;
          }

          if ( get_obj_weight( obj ) > (get_obj_weight(weapon)/2) )
          {
               send_to_char ( "These two weapons are not balanced.\n\r", ch);
               return;
          }
          if ( equip_char ( ch, obj, WEAR_WIELD2 ) )
          {
               // A little different here because there's no way to tell TRIG_WEAR that we're dualling
               // and we do need such a message.
               act ( "You wield $p in your off hand.", ch, obj, NULL, TO_CHAR );
               if ( HAS_TRIGGER_OBJ( obj, TRIG_WEAR ) )
                    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_WEAR );
               else
               {
                    act ( "$n dual wields $p.", ch, obj, NULL, TO_ROOM );
               }
          }

          sn = get_weapon_sn ( ch, TRUE );

          /* hand to hand??? wtf? */
          /* Okay... whatever.... can't hurt, don't see how it would be tho */
          if ( sn == gsn_hand_to_hand )
               return;

          skill = get_weapon_skill ( ch, sn );

          if ( skill >= 100 )		act ( "$p feels like a part of you!", ch, obj, NULL, TO_CHAR );
          else if ( skill > 85 )        act ( "You feel quite confident with $p.", ch, obj, NULL, TO_CHAR );
          else if ( skill > 70 )        act ( "You are skilled with $p.", ch, obj, NULL, TO_CHAR );
          else if ( skill > 50 )        act ( "Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR );
          else if ( skill > 25 )        act ( "$p feels a little clumsy in your hands.", ch, obj, NULL, TO_CHAR );
          else if ( skill > 1 )         act ( "You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR );
          else 				act ( "You don't even know which is end is up on $p.", ch, obj, NULL, TO_CHAR );
          return;
     }
}

void do_wear ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Wear, wield, or hold what?\n\r", ch );
          return;
     }

     if ( !str_cmp ( arg, "all" ) )
     {
          OBJ_DATA           *obj_next;
          for ( obj = ch->carrying; obj != NULL; obj = obj_next )
          {
               obj_next = obj->next_content;
               if ( obj->wear_loc == WEAR_NONE &&
                    can_see_obj ( ch, obj ) )
               {
                    if ( obj->condition != 0 )
                    {
                         wear_obj ( ch, obj, FALSE );
                    }
                    else
                         act ( "$p is too damaged to wear!", ch, obj, NULL, TO_CHAR );
               }
          }
          return;
     }
     else
     {
          if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
          {
               send_to_char ( "You do not have that item.\n\r", ch );
               return;
          }
          if ( obj->condition <= 2 )
               act ( "$p is too damaged to wear!", ch, obj, NULL, TO_CHAR );
          else
               wear_obj ( ch, obj, TRUE );
     }

     return;
}

/* Zeran - modified to support remove "all" and "all.whatever"    */
/* Lotherius - Modified to remove dual wield when primary removed */
void do_remove ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA           *tmp;
     OBJ_DATA		*weapon;
     int                 number;
     bool                all_type = FALSE;
     bool                done = FALSE;

     one_argument ( argument, arg );

     /* Zeran - check for all.foo */
     number = number_argument ( arg, arg2 );
     if ( number == -1 )
          all_type = TRUE;

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Remove what?\n\r", ch );
          return;
     }

     /* check remove all.foo */
     if ( all_type )
     {
          number = 0;
          while ( !done )
          {
               if ( ( obj = get_obj_wear ( ch, arg2, TRUE ) ) == NULL )
               {
                    if ( !number )
                         form_to_char ( ch, "You don't have any %s.\n\r", arg2 );
                    done = TRUE;
                    continue;
               }
               if ( obj->wear_loc == WEAR_WIELD )
               {
                    remove_obj ( ch, obj->wear_loc, TRUE );
                    weapon = get_eq_char ( ch, WEAR_WIELD2 );
                    if ( weapon != NULL )
                    {
                         send_to_char ( "You put away the weapon in your off-hand too...\n\r", ch );
                         remove_obj ( ch, WEAR_WIELD2, TRUE );
                    }
               }
               else
                    remove_obj ( ch, obj->wear_loc, TRUE );
               number++;
          }
          return;
     }

     /* check "remove all" */
     if ( !str_cmp ( arg, "all" ) )
     {
          for ( tmp = ch->carrying; tmp != NULL; tmp = tmp->next_content )
          {
               if ( tmp->wear_loc != WEAR_NONE )
                    remove_obj ( ch, tmp->wear_loc, TRUE );
          }
          return;
     }

     if ( ( obj = get_obj_wear ( ch, arg, TRUE ) ) == NULL )
     {
          send_to_char ( "You do not have that item.\n\r", ch );
          return;
     }

     if ( obj->wear_loc == WEAR_WIELD )
     {
          remove_obj ( ch, obj->wear_loc, TRUE );
          weapon = get_eq_char ( ch, WEAR_WIELD2 );
          if ( weapon != NULL )
          {
               send_to_char ( "You put away the weapon in your off-hand too...\n\r", ch );
               remove_obj ( ch, WEAR_WIELD2, TRUE );
          }
     }
     else
          remove_obj ( ch, obj->wear_loc, TRUE );
     return;
}

// Donate is located in clan.c, and is a bit different than normal donation.
// Now does all.foo AND all
void do_sacrifice ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     OBJ_DATA		*obj_next;
     int                 gold;
     int                 count, sac_count = 0;
     bool                done = FALSE;
     bool                is_all = FALSE;
     bool		 is_many = FALSE;
     CHAR_DATA          *gch;		// For autosplit
     int                 members;	// For autosplit
     char                buffer[100];	// For autosplit

     one_argument ( argument, arg );

     if (IS_NPC(ch))
     {
          act( "$n wouldn't know whom to offer it to.", ch, NULL, NULL, TO_ROOM );
          return;
     }

     if ( arg[0] == '\0' || !str_cmp ( arg, ch->name ) )
     {
          act ( "$n offers $mself to Zeran, who graciously declines.", ch, NULL, NULL, TO_ROOM );
          send_to_char ( "Zeran appreciates your offer and may accept it later.\n\r", ch );
          return;
     }

     /* Lotherius - Check for "all" first */
     if ( !str_cmp ( arg, "all" ) )
     {
          is_many = TRUE;
          is_all = TRUE;
     }
     else	/* Then check for all.foo */
     {
          count = number_argument ( arg, arg2 );
          if ( count == -1 )		// all.foo  foo is in arg2
          {
               is_many = TRUE;
               if ( arg2 == NULL || arg2[0] == '\0' )
               {
                    send_to_char ( "You'd like to sacrifice all of what?\n\r", ch );
               }
               SLCPY ( arg, arg2 );
          }
     }

     for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
     {
          obj_next = obj->next_content;

          if ( is_all == FALSE )	// Do we need to check keyword?
          {
               // Not everything....
               if ( !is_name( arg2, obj->name ) && !is_name_abbv( arg2, obj->name ) ) 	// Not a match
                    continue;							 	// Skip to next item.
               // Is this the ONLY match we want?
               if ( is_many == FALSE )
                    done = TRUE;	// Exit at the end of the loop if "is_many" is false.
          }

          if ( obj == NULL )
               bugf ( "Null obj got into the loop on do_sacrifice." );

          if ( IS_SET ( obj->extra_flags, ITEM_NO_SAC ) || obj->owner )
          {
               send_to_char ( "That object cannot be sacrificed.\n\r", ch );
               return;
          }

          for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
          {
               if ( gch->on == obj )
               {
                    act ( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
                    return;
               }
          }

          gold = UMIN ( 10, ( obj->level * 2 ) );

          if ( obj->item_type == ITEM_CORPSE_PC )
          {
               if ( obj->contains )
               {
                    send_to_char( "Zeran thinks you should loot first.\n\r", ch );
                    return;
               }
               else
               {
                    gold = UMAX ( 1, obj->level * 4 );
               }
          }

          if ( gold == 0 )
               SNP ( buffer, "Zeran doesn't seem to notice your sacrifice of $p." );
          else
               SNP ( buffer, "Zeran gives you %d gold for your $p.", gold );
          act ( buffer, ch, obj, NULL, TO_CHAR );

          if ( !IS_SET ( ch->act, PLR_AUTOSPLIT ) )
               do_pay ( ch, gold );
          else				// Do autosplit
          {
               members = 0;
               for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
               {
                    if ( is_same_group ( gch, ch ) )
                         members++;
               }
               if ( members > 1 && gold > 1 )
               {
                    ch->gold += gold;
                    SNP ( buffer, "%d", gold );
                    do_split ( ch, buffer, TRUE );
               }
               else
                    do_pay ( ch, gold );
          }
          act ( "$n sacrifices $p to Zeran.", ch, obj, NULL, TO_ROOM );
          extract_obj ( obj );
          sac_count++;
          if ( done )
               return;
     }
     // End of for loop.
     return;
}

void do_quaff ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Quaff what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that potion.\n\r", ch );
          return;
     }

     if ( obj->item_type != ITEM_POTION )
     {
          send_to_char ( "You can quaff only potions.\n\r", ch );
          return;
     }

     if ( ch->level < obj->level )
     {
          send_to_char ( "This liquid is too powerful for you to drink.\n\r", ch );
          return;
     }

     act ( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
     act ( "You quaff $p.", ch, obj, NULL, TO_CHAR );

     obj_cast_spell ( obj->value[1], obj->value[0], ch, ch, NULL );
     obj_cast_spell ( obj->value[2], obj->value[0], ch, ch, NULL );
     obj_cast_spell ( obj->value[3], obj->value[0], ch, ch, NULL );
     /* Zeran - add waitstate to stop speed quaffing. */
     WAIT_STATE ( ch, PULSE_VIOLENCE );	/* 1 potion per fight round */
     extract_obj ( obj );
     return;
}

void do_recite ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *scroll;
     OBJ_DATA           *obj;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( ( scroll = get_obj_carry ( ch, arg1, NULL ) ) == NULL )
     {
          send_to_char ( "You do not have that scroll.\n\r", ch );
          return;
     }

     if ( scroll->item_type != ITEM_SCROLL )
     {
          send_to_char ( "You can recite only scrolls.\n\r", ch );
          return;
     }

     if ( ch->level < scroll->level )
     {
          send_to_char( "This scroll is too complex for you to comprehend.\n\r", ch );
          return;
     }

     obj = NULL;
     if ( arg2[0] == '\0' )
     {
          victim = ch;
     }
     else
     {
          if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL
               && ( obj = get_obj_here ( ch, NULL, arg2 ) ) == NULL )
          {
               send_to_char ( "You can't find it.\n\r", ch );
               return;
          }
     }

     act ( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
     act ( "You recite $p.", ch, scroll, NULL, TO_CHAR );
     WAIT_STATE ( ch, PULSE_VIOLENCE );

     if ( number_percent (  ) >= 20 + get_skill ( ch, gsn_scrolls ) * 4 / 5 )
     {
          send_to_char ( "You mispronounce a syllable.\n\r", ch );
          check_improve ( ch, gsn_scrolls, FALSE, 2 );
     }

     else
     {
          obj_cast_spell ( scroll->value[1], scroll->value[0], ch, victim, obj );
          obj_cast_spell ( scroll->value[2], scroll->value[0], ch, victim, obj );
          obj_cast_spell ( scroll->value[3], scroll->value[0], ch, victim, obj );
          check_improve ( ch, gsn_scrolls, TRUE, 2 );
     }

     extract_obj ( scroll );
     return;
}

void do_brandish ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *vch;
     CHAR_DATA          *vch_next;
     OBJ_DATA           *staff;
     int                 sn;

     if ( ( staff = get_eq_char ( ch, WEAR_HOLD ) ) == NULL )
     {
          send_to_char ( "You hold nothing in your hand.\n\r", ch );
          return;
     }

     if ( staff->item_type != ITEM_STAFF )
     {
          send_to_char ( "You can brandish only with a staff.\n\r", ch );
          return;
     }

     if ( ( sn = staff->value[3] ) < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
     {
          bugf ( "Do_brandish: bad sn %d.", sn );
          return;
     }

     WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );

     if ( staff->value[2] > 0 )
     {
          act ( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
          act ( "You brandish $p.", ch, staff, NULL, TO_CHAR );
          if ( ch->level < staff->level
               || number_percent (  ) >= 20 + get_skill ( ch, gsn_staves ) * 4 / 5 )
          {
               act ( "You fail to invoke $p.", ch, staff, NULL, TO_CHAR );
               act ( "...and nothing happens.", ch, NULL, NULL, TO_ROOM );
               check_improve ( ch, gsn_staves, FALSE, 2 );
          }
          else
          {
               for ( vch = ch->in_room->people; vch; vch = vch_next )
               {
                    vch_next = vch->next_in_room;

                    switch ( skill_table[sn].target )
                    {
                    default:
                         bugf ( "Do_brandish: bad target for sn %d.", sn );
                         return;
                    case TAR_IGNORE:
                         if ( vch != ch )
                              continue;
                         break;
                    case TAR_CHAR_OFFENSIVE:
                         if ( IS_NPC ( ch ) ? IS_NPC ( vch ) : !IS_NPC ( vch ) )
                              continue;
                         break;
                    case TAR_CHAR_DEFENSIVE:
                         if ( IS_NPC ( ch ) ? !IS_NPC ( vch ) : IS_NPC ( vch ) )
                              continue;
                         break;
                    case TAR_CHAR_SELF:
                         if ( vch != ch )
                              continue;
                         break;
                    }

                    obj_cast_spell ( staff->value[3], staff->value[0],
                                     ch, vch, NULL );
                    check_improve ( ch, gsn_staves, TRUE, 2 );
               }
          }
     }

     if ( --staff->value[2] <= 0 )
     {
          act ( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
          act ( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
          extract_obj ( staff );
     }
     return;
}

void do_zap ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *wand;
     OBJ_DATA           *obj;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' && ch->fighting == NULL )
     {
          send_to_char ( "Zap whom or what?\n\r", ch );
          return;
     }

     if ( ( wand = get_eq_char ( ch, WEAR_HOLD ) ) == NULL )
     {
          send_to_char ( "You hold nothing in your hand.\n\r", ch );
          return;
     }

     if ( wand->item_type != ITEM_WAND )
     {
          send_to_char ( "You can zap only with a wand.\n\r", ch );
          return;
     }

     obj = NULL;
     if ( arg[0] == '\0' )
     {
          if ( ch->fighting != NULL )
          {
               victim = ch->fighting;
          }
          else
          {
               send_to_char ( "Zap whom or what?\n\r", ch );
               return;
          }
     }
     else
     {
          if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL
               && ( obj = get_obj_here ( ch, NULL, arg ) ) == NULL )
          {
               send_to_char ( "You can't find it.\n\r", ch );
               return;
          }
     }

     WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );

     if ( wand->value[2] > 0 )
     {
          if ( victim != NULL )
          {
               act ( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
               act ( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
          }
          else
          {
               act ( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
               act ( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
          }

          if ( ch->level < wand->level
               || number_percent (  ) >= 20 + get_skill ( ch, gsn_wands ) * 4 / 5 )
          {
               act ( "Your efforts with $p produce only smoke and sparks.", ch, wand, NULL, TO_CHAR );
               act ( "$n's efforts with $p produce only smoke and sparks.", ch, wand, NULL, TO_ROOM );
               check_improve ( ch, gsn_wands, FALSE, 2 );
          }
          else
          {
               obj_cast_spell ( wand->value[3], wand->value[0], ch, victim, obj );
               check_improve ( ch, gsn_wands, TRUE, 2 );
          }
     }

     if ( --wand->value[2] <= 0 )
     {
          act ( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
          act ( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
          extract_obj ( wand );
     }

     return;
}

void do_steal ( CHAR_DATA * ch, char *argument )
{
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     OBJ_DATA           *obj;
     int                 percent;
     int                 togain;
     int                 chance_to_steal;

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char ( "Steal what from whom?\n\r", ch );
          return;
     }
     if ( ( victim = get_char_room ( ch, NULL, arg2 ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }
     if ( victim == ch )
     {
          send_to_char ( "That's pointless.\n\r", ch );
          return;
     }
     if ( victim->master == ch )
     {
          send_to_char ( "It's already yours.\n\r", ch);
          return;
     }
     if ( is_safe ( ch, victim, TRUE ) )
          return;

     if ( !IS_NPC ( victim ) && is_safe ( ch, victim, TRUE ) )
     {
          send_to_char ( "That player is outside your stealing range.\n\r", ch );
          return;
     }

     if ( victim->position == POS_FIGHTING )
     {
          send_to_char ( "You'd better not -- you might get hit.\n\r", ch );
          return;
     }

     /* Fast automatic failures done, so apply the wait state now since
      * the stealing attempt is going to take place */
     WAIT_STATE ( ch, skill_table[gsn_steal].beats );

     percent = number_percent (  ) + ( IS_AWAKE ( victim ) ? 10 : -50 );
     /* Zeran - modifiers */
     if ( !can_see ( victim, ch ) )	/*nice bonus */
          percent -= 25;
     percent -= ( 3 * ( get_curr_stat ( ch, STAT_DEX ) - 20 ) );	/*dex bonus */
     {
          int                 diff = ch->level - victim->level;
          int                 modifier = 3 * ( diff - 10 );
          if ( abs ( diff ) > 10 )
               percent -= modifier;
     }

     /* ok, lets see what happens */
     /* 4/11/99 Zeran - fix bug.  If PC, use skill %, if a mob, make up
      * a value
      */
     if ( IS_NPC ( ch ) )
          chance_to_steal = 50;
     else
          chance_to_steal = ch->pcdata->learned[gsn_steal];

     if ( percent > chance_to_steal )
     {				/*failed, didn't get object */
          form_to_char ( ch, "You failed to steal from %s.\n\r", PERS ( victim, ch ) );
          check_improve ( ch, gsn_steal, FALSE, 2 );
     }
     else			/*get the object */
     {
	/*
         * need to give victim same sight as thief to find object in
	 * victim inventory for get_obj_carry
         * With the new obj_carry this shouldn't be necessary, however I
         * haven't updated this code yet - Lotherius
         */
          bool                add_det_invis = FALSE;
          bool                remove_blind = FALSE;

          /* Zeran - fixed bug, target may already have this vision level */

          if ( CAN_DETECT ( ch, DET_INVIS ) && !CAN_DETECT ( victim, DET_INVIS ) )
          {
               add_det_invis = TRUE;
               SET_BIT ( victim->detections, DET_INVIS );
          }
          if ( IS_AFFECTED ( victim, AFF_BLIND ) )
          {
               remove_blind = TRUE;
               REMOVE_BIT ( victim->affected_by, AFF_BLIND );
          }

          if ( !str_cmp ( arg1, "coin" )
               || !str_cmp ( arg1, "coins" )
               || !str_cmp ( arg1, "gold" ) )
          {
               int                 amount;
               amount = victim->gold * number_range ( 1, 10 ) / 100;
               if ( amount <= 0 )
               {
                    send_to_char ( "You couldn't get any gold.\n\r", ch );
               }
               else
               {
                    victim->gold -= amount;
                    form_to_char ( ch, "Bingo!  You got %d gold coins.\n\r", amount );
                    do_pay (ch, amount );
                    check_improve ( ch, gsn_steal, TRUE, 2 );
                    togain = ( victim->level - ch->level ) + 3;
                    if ( togain >= 1 )
                    {
                         togain = togain * 2.5; /* XP Boost Changes */
                         gain_exp ( ch, togain );
                    }
               }
          }
          else if ( ( obj = get_obj_carry ( victim, arg1, NULL ) ) ==  NULL )
               send_to_char ( "You can't find it.\n\r", ch );
          else if ( !can_drop_obj ( ch, obj ) || IS_SET ( obj->extra_flags, ITEM_INVENTORY ) )
               send_to_char ( "You can't pry it away.\n\r", ch );
          else if ( ch->carry_number + get_obj_number ( obj ) > can_carry_n ( ch ) )
               send_to_char ( "You have your hands full.\n\r", ch );
          else if ( ch->carry_weight + get_obj_weight ( obj ) > can_carry_w ( ch ) )
               send_to_char ( "You can't carry that much weight.\n\r", ch );
          else if (obj->owner && !belongs(ch, obj) )
               form_to_char ( ch, "The %s belongs to %s, and you cannot have it.\n\r", obj->short_descr, obj->owner );
          else
          {
               form_to_char ( ch, "Bingo! You got %s.  Better run while you can.\n\r", obj->short_descr );
               obj_from_char ( obj );
               obj_to_char ( obj, ch );
               check_improve ( ch, gsn_steal, TRUE, 2 );
               togain = ( victim->level - ch->level ) + 3;
               if ( togain >= 1 )
               {
                    togain = togain * 2.5;
                    gain_exp ( ch, togain );
               }
          }
          /*restore victim previous vision */
          if ( add_det_invis )
               SET_BIT (victim->detections, DET_INVIS );
          if ( remove_blind )
               SET_BIT (victim->affected_by, AFF_BLIND );
     }
     /* Now, see if anyone noticed the thievery */
     {
          CHAR_DATA          *person;
          int                 chance;

          person = ch->in_room->people;
          for ( ; person; person = person->next_in_room )
          {
               if ( ( person != victim ) && ( ( person->position < POS_RESTING )
                                              || !can_see ( person, ch )
                                              || !can_see ( person, victim ) ) )
                    continue;
               /*make roll */
               chance = ( get_curr_stat ( person, STAT_INT ) ) + person->level - ch->level;
               if ( ( person == victim ) && ( person->position >= POS_RESTING ) && can_see ( person, ch ) )
                    chance += 20;
               if ( number_percent (  ) <= chance )	/*oops, got caught */
               {
                    if ( ( person != victim ) && ( person != ch ) && !IS_NPC ( person ) )
                    {
                         form_to_char ( ch, "You notice %s trying to steal from %s!\n\r",
                                        PERS ( ch, person ), PERS ( victim, person ) );
                         continue;
                    }
                    if ( ( person == victim ) && !IS_NPC ( person ) )
                    {
                         form_to_char ( ch, "%s is trying to steal from you!\n\r",
                                        ( victim->position >= POS_RESTING ) ? PERS ( ch, person ) :
                                        "Someone" );
                         continue;
                    }
                    if ( ( person == victim ) && IS_NPC ( victim ) )
                    {
                         do_say ( person, "This is what I do to thieves!\n\r" );
                         multi_hit ( person, ch, TYPE_UNDEFINED );
                    }
               }
          }
          /*end character for loop */
     }
     /*end check notice block */
     return;
}

/*
 * Shopping commands.
 */

/* Say when a shop is open... */
void do_hours( CHAR_DATA *ch, char *args )
{
     char buf[MIL];
     CHAR_DATA *keeper;
     SHOP_DATA *pShop;

     /* Check parms... */
     if ( args[0] != '\0' )
     {
          send_to_char("Syntax: hours\n\r", ch);
          return;
     }
     /* Find shop keeper... */
     pShop = NULL;
     keeper = ch->in_room->people;
     while ( keeper != NULL && pShop == NULL )
     {
          if ( keeper->pIndexData != NULL )
               pShop = keeper->pIndexData->pShop;
          if (pShop == NULL)
               keeper = keeper->next_in_room;
     }

     if ( pShop == NULL )
     {
          send_to_char( "There is no shop keeper here.\n\r", ch );
          return;
     }
     /* Invisible or hidden people... */
     if ( !can_see( keeper, ch ) )
     {
          do_say( keeper, "What? Who said that? Is somebody there?" );
          return;
     }
     /* Report opening hours... */
     if ( pShop->open_hour == 0 && pShop->close_hour == 23 )
     {
          do_say(keeper, "I am always open!");
          return;
     }
     else if ( pShop->open_hour == 0 )
     {
          SNP ( buf, "I am open from midnight until %d:00.", (pShop->close_hour + 1));
          do_say(keeper, buf);
          return;
     }
     else if ( pShop->close_hour == 23 )
     {
          SNP(buf, "I am open from %d:00 until midnight.", pShop->open_hour);
          do_say(keeper, buf);
          return;
     }
     else
     {
          SNP(buf, "I am open from %d:00 until %d:00.",
              pShop->open_hour, (pShop->close_hour + 1));
          do_say(keeper, buf);
          return;
     }
     /* All done... */
     return;
}

CHAR_DATA          *find_keeper ( CHAR_DATA * ch )
{
     char                buf[MAX_STRING_LENGTH];
     CHAR_DATA          *keeper;
     SHOP_DATA          *pShop;

     pShop = NULL;
     for ( keeper = ch->in_room->people; keeper;
           keeper = keeper->next_in_room )
     {
          if ( IS_NPC ( keeper ) &&
               ( pShop = keeper->pIndexData->pShop ) != NULL )
               break;
     }

     if ( pShop == NULL )
     {
          send_to_char ( "You can't do that here.\n\r", ch );
          return NULL;
     }

    /*
     * Undesirables.
     */
     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_KILLER ) )
     {
          do_say ( keeper, "Killers are not welcome!" );
          SNP ( buf, "%s the KILLER is over here!\n\r", ch->name );
          do_yell ( keeper, buf );
          return NULL;
     }

     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_THIEF ) )
     {
          do_say ( keeper, "Thieves are not welcome!" );
          SNP ( buf, "%s the THIEF is over here!\n\r", ch->name );
          do_yell ( keeper, buf );
          return NULL;
     }

    /*
     * Shop hours.
     */
     if ( time_info.hour < pShop->open_hour && !IS_IMMORTAL ( ch ) )
     {
          do_say ( keeper, "Sorry, I am closed. Come back later." );
          return NULL;
     }

     if ( time_info.hour > pShop->close_hour && !IS_IMMORTAL ( ch ) )
     {
          do_say ( keeper, "Sorry, I am closed. Come back tomorrow." );
          return NULL;
     }

    /*
     * Invisible or hidden people.
     */
     if ( !can_see ( keeper, ch ) )
     {
          do_say ( keeper, "I don't trade with folks I can't see." );
          return NULL;
     }

     return keeper;
}

/* Lotherius - insert an object at the right spot for the keeper */
void obj_to_keeper ( OBJ_DATA * obj, CHAR_DATA * ch )
{
     OBJ_DATA           *t_obj, *t_obj_next;
     /* see if any duplicates are found */
     for ( t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next )
     {
          t_obj_next = t_obj->next_content;

          if ( obj->pIndexData == t_obj->pIndexData
               && !str_cmp ( obj->short_descr, t_obj->short_descr ) )
          {
	    /* if this is an unlimited item, destroy the new one */
               if ( IS_OBJ_STAT ( t_obj, ITEM_INVENTORY ) )
               {
                    extract_obj ( obj );
                    return;
               }
               obj->cost = t_obj->cost;	/* keep it standard */
               break;
          }
     }
     if ( t_obj == NULL )
     {
          obj->next_content = ch->carrying;
          ch->carrying = obj;
     }
     else
     {
          obj->next_content = t_obj->next_content;
          t_obj->next_content = obj;
     }
     obj->carried_by = ch;
     obj->in_room = NULL;
     obj->in_obj = NULL;
     ch->carry_number += get_obj_number ( obj );
     ch->carry_weight += get_obj_weight ( obj );
}

/* get an object from a shopkeeper's list  - Lotherius*/
OBJ_DATA *get_obj_keeper ( CHAR_DATA * ch, CHAR_DATA * keeper, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 number;
     int                 count;

     number = number_argument ( argument, arg );
     count = 0;
     for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
     {
          if ( obj->wear_loc == WEAR_NONE && can_see_obj ( keeper, obj )
               && can_see_obj ( ch, obj ) && is_name ( arg, obj->name ) )
          {
               if ( ++count == number )
                    return obj;
               /* skip other objects of the same name */
               while ( obj->next_content != NULL
                       && obj->pIndexData == obj->next_content->pIndexData
                       && !str_cmp ( obj->short_descr, obj->next_content->short_descr ) )
                    obj = obj->next_content;
          }
     }
     return NULL;
}

int get_cost ( CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy )
{
     SHOP_DATA          *pShop;
     int                 cost, i;

     if ( obj == NULL ||
          ( pShop = keeper->pIndexData->pShop ) == NULL )
          return 0;

     if ( fBuy )
     {
          cost = obj->cost * pShop->profit_buy / 100;
     }
     else
     {
          OBJ_DATA           *obj2;
          int                 itype;

          cost = 0;
          for ( itype = 0; itype < MAX_TRADE; itype++ )
          {
               if ( obj->item_type == pShop->buy_type[itype] )
               {
                    cost = obj->cost * pShop->profit_sell / 100;
                    break;
               }
          }

          /* start multiples price checking */

          for ( obj2 = keeper->carrying; obj2;
                obj2 = obj2->next_content )
          {
               if ( obj->pIndexData == obj2->pIndexData
                    && !str_cmp ( obj->short_descr,
                                  obj2->short_descr ) )
               {
                    if ( IS_OBJ_STAT ( obj2, ITEM_INVENTORY ) )
                         cost /= 2;
                    else
                         cost = cost * 3 / 4;
               }
          }
     }

     if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
     {
          if ( obj->value[1] == 0 )
               cost /= 4;
          else
               cost = cost * obj->value[2] / obj->value[1];
     }

    /* Reduce cost for object condition */
     if (obj->condition < 100)
     {
          cost = cost * obj->condition/100;
     }

     /* Thanks to CthulhuMud for the following addition */

     /* Reduce offer price to conserve gold... */

     if (!fBuy)
     {

          i = 9000;

          while ( keeper->gold < i )
          {
               cost -= (cost/10);
               i -= 1000;
          }

          if ( cost > keeper->gold / 2 )
          {
               cost /= 2;
          }

     }

   /* Boost price to increase gold reserves... */

     if (fBuy)
     {
          i = 9000;

          while ( keeper->gold < i )
          {
               cost += (cost/20);
               i -= 1000;
          }

     }

    /* Sanity checking */

     if (cost < 1 && !fBuy) return 0;
     if (cost <1 && fBuy) return 10;

     return cost;
}

void do_buy ( CHAR_DATA * ch, char *argument )
{
     char               buf[MSL];
     char               arg[MIL];
     int                cost;
     int		roll;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Buy what?\n\r", ch );
          return;
     }

     if ( IS_SET ( ch->in_room->room_flags, ROOM_PET_SHOP ) )
     {
          CHAR_DATA          *pet;
          ROOM_INDEX_DATA    *pRoomIndexNext;
          ROOM_INDEX_DATA    *in_room;

          if ( IS_NPC ( ch ) )
               return;

          argument = one_argument ( argument, arg );

          pRoomIndexNext = get_room_index ( ch->in_room->vnum + 1 );
          if ( pRoomIndexNext == NULL )
          {
               bugf ( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
               send_to_char ( "Sorry, you can't buy that here.\n\r", ch );
               return;
          }

          in_room = ch->in_room;
          ch->in_room = pRoomIndexNext;
          pet = get_char_room ( ch, NULL, arg );
          ch->in_room = in_room;

          if ( pet == NULL || !IS_SET ( pet->act, ACT_PET ) )
          {
               send_to_char ( "Sorry, you can't buy that here.\n\r", ch );
               return;
          }

          if ( ch->pet != NULL )
          {
               send_to_char ( "You already own a pet.\n\r", ch );
               return;
          }

          cost = 10 * pet->level * pet->level;

          if ( ch->gold < cost )
          {
               send_to_char ( "You can't afford it.\n\r", ch );
               return;
          }

          if ( ch->level < pet->level )
          {
               send_to_char ( "You're not powerful enough to master this pet.\n\r", ch );
               return;
          }

          /* haggle */

          /* Zeran - haggle breaks for low cost items */
          /* Therefore we won't run it on low cost items
           * but no reason to disable haggle altogether -- Lotherius
           */
          roll = number_percent();
          if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle] && cost >= 20 )
          {
               cost -= cost / 2 * roll / 100;
               form_to_char ( ch, "You haggle the price down to %d coins.\n\r",cost);
               check_improve(ch,gsn_haggle,TRUE,4);
          }

          ch->gold -= cost;
          sound ("MONEYCLANG.WAV", ch);

          pet = create_mobile ( pet->pIndexData );
          SET_BIT ( ch->act, PLR_BOUGHT_PET );
          SET_BIT ( pet->act, ACT_PET );
          SET_BIT ( pet->affected_by, AFF_CHARM );
          pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

          argument = one_argument ( argument, arg );
          if ( arg[0] != '\0' )
          {
               SNP ( buf, "%s %s", pet->name, arg );
               free_string ( pet->name );
               pet->name = str_dup ( buf );
          }

          SNP ( buf, "%sA neck tag says 'I belong to %s'.\n\r", pet->description, ch->name );
          free_string ( pet->description );
          pet->description = str_dup ( buf );

          char_to_room ( pet, ch->in_room );
          add_follower ( pet, ch );
          pet->leader = ch;
          ch->pet = pet;
          send_to_char ( "Enjoy your pet.\n\r", ch );
          act ( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
          return;
     }
     else
     {
          CHAR_DATA          *keeper;
          OBJ_DATA           *obj, *t_obj;
          int                 number, count = 1;	/* added for new shop code */

          if ( ( keeper = find_keeper ( ch ) ) == NULL )
               return;

          number = mult_argument ( argument, arg );	/* Lotherius added for shopcode */
          obj = get_obj_carry ( keeper, argument, NULL );
          cost = get_cost ( keeper, obj, TRUE );

          if ( number < 1 )
          {
               send_to_char("Yeah right.\n\r", ch);
               return;
          }

          if ( number > 50 )
          {
               send_to_char("The law does not permit me to sell more than 50 items in one transaction.\n\r", ch);
               return;
          }

          if ( cost <= 0 || !can_see_obj ( ch, obj ) )
          {
               act ( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT );
               ch->reply = keeper;
               return;
          }
          /* Lotherius */
          if ( !IS_OBJ_STAT ( obj, ITEM_INVENTORY ) )
          {
               for ( t_obj = obj->next_content; count < number && t_obj != NULL; t_obj = t_obj->next_content )
               {
                    if ( t_obj->pIndexData == obj->pIndexData && !str_cmp ( t_obj->short_descr, obj->short_descr ) )
                         count++;
                    else
                         break;
               }
               if ( count < number )
               {
                    act ( "$n tells you 'I don't have that many in stock.", keeper, NULL, ch, TO_VICT );
                    ch->reply = keeper;
                    return;
               }
          }
          if ( ch->gold < cost * number )	/* Lotherius added number */
          {
               if ( number > 1 )
                    act ( "$n tells you 'You can't afford to buy that many.", keeper, obj, ch, TO_VICT );
               else
                    act ( "$n tells you 'You can't afford to buy $p'.", keeper, obj, ch, TO_VICT );
               ch->reply = keeper;
               return;
          }

          if ( obj->level > ch->level )
          {
               act ( "$n tells you 'You can't use $p yet'.", keeper, obj, ch, TO_VICT );
               ch->reply = keeper;
               return;
          }

          if ( ch->carry_number + number * get_obj_number ( obj ) > can_carry_n ( ch ) )
          {
               send_to_char ( "You can't carry that many items.\n\r", ch );
               return;
          }

          if ( ch->carry_weight + number * get_obj_weight ( obj ) > can_carry_w ( ch ) )
          {
               send_to_char ( "You can't carry that much weight.\n\r", ch );
               return;
          }

          /* haggle */
          /* Zeran - haggle is broken for low priced items */
          /* Easy solution, don't run it on low priced items -- Lotherius */
          roll = number_percent();
          if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle] && cost >=20 )
          {
               cost -= obj->cost / 2 * roll / 100;
               form_to_char ( ch, "You haggle the price down to %d coins.\n\r",cost);
               check_improve(ch,gsn_haggle,TRUE,4);
          }

          /* added for multiple items */
          if ( number > 1 )
          {
               SNP ( buf, "$n buys %d $ps.\n\r", number );
               act ( buf, ch, obj, NULL, TO_ROOM );
               SNP ( buf, "You buy %d $ps for %d coins.\n\r", number, cost * number );
               act ( buf, ch, obj, NULL, TO_CHAR );
          }
          else
          {
               act ( "$n buys $p.", ch, obj, NULL, TO_ROOM );
               SNP ( buf, "You buy $p for %d coins.\n\r", cost );
               act ( buf, ch, obj, NULL, TO_CHAR );
          }

          sound ("MONEYCLANG.WAV", ch);

          ch->gold -= cost * number;
          keeper->gold += cost * number;

          for ( count = 0; count < number; count++ )
          {
               if ( IS_SET ( obj->extra_flags, ITEM_INVENTORY ) )
                    t_obj = create_object ( obj->pIndexData, obj->level );
               else
               {
                    t_obj = obj;
                    obj = obj->next_content;
                    obj_from_char ( t_obj );
               }

               if ( ( t_obj->item_type == ITEM_POTION ) || ( t_obj->item_type == ITEM_SCROLL ) )
                    t_obj->timer = UMAX ( 24, ( 2 * keeper->level ) );
               else
                    t_obj->timer = 0;
               obj_to_char ( t_obj, ch );
               if ( cost < t_obj->cost )
                    t_obj->cost = cost;
               t_obj->size = ch->size;
               /* No more auto-repair */
               /*	    t_obj->condition = 100;*/	/* shopkeeper does auto repairing */
          }
     }
}

void do_list ( CHAR_DATA * ch, char *argument )
{
     bool                found;
     if ( IS_SET ( ch->in_room->room_flags, ROOM_PET_SHOP ) )
     {
          ROOM_INDEX_DATA    *pRoomIndexNext;
          CHAR_DATA          *pet;

          pRoomIndexNext = get_room_index ( ch->in_room->vnum + 1 );
          if ( pRoomIndexNext == NULL )
          {
               bugf ( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
               send_to_char ( "You can't do that here.\n\r", ch );
               return;
          }

          found = FALSE;
          for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
          {
               if ( IS_SET ( pet->act, ACT_PET ) )
               {
                    if ( !found )
                    {
                         found = TRUE;
                         send_to_char ( "{WPets for sale:\n\r", ch );
                    }
                    if ( ch->desc->mxp )
                    {
                         form_to_char ( ch, "{w[{G%2d{w] {Y%8d {w- {W", pet->level, 10 * pet->level * pet->level );
                         form_to_char ( ch, MXP_SECURE "<SEND \"buy %s\" hint=\"Buy It|buy %s\">%s</SEND>\n\r"
                                        MXP_RESET MXP_LLOCK,
                                        pet->name, pet->short_descr, pet->short_descr );
                    }
                    else
                         form_to_char ( ch, "{w[{G%2d{w] {Y%8d {w- {W%s\n\r",  pet->level,
                                        10 * pet->level * pet->level, pet->short_descr );
               }
          }
          if ( !found )
               send_to_char ( "Sorry, we're out of pets right now.\n\r", ch );
          return;
     }
     else
     {
          CHAR_DATA          *keeper;
          OBJ_DATA           *obj;
          int                 cost, count;
          char                arg[MAX_INPUT_LENGTH];

          if ( ( keeper = find_keeper ( ch ) ) == NULL )
               return;
          one_argument ( argument, arg );

          found = FALSE;
          for ( obj = keeper->carrying; obj; obj = obj->next_content )
          {
               if ( obj->wear_loc == WEAR_NONE && can_see_obj ( ch, obj )
                    && ( cost = get_cost ( keeper, obj, TRUE ) ) > 0 && ( arg[0] == '\0'
                                                                          || is_name ( arg, obj->name ) ) )
               {
                    if ( !found )
                    {
                         found = TRUE;
                         send_to_char ( "[{gLvl {yPrice  {rQty{w] Item\n\r", ch );
                    }

                    /* Lotherius added below for multiple listings */

                    if ( IS_OBJ_STAT ( obj, ITEM_INVENTORY ) )
                    {
                         if ( ch->desc->mxp )
                         {
                              form_to_char ( ch, "[{G%3d {Y%6d {R---{w] {W", obj->level, cost );
                              form_to_char ( ch, MXP_SECURE
                                             "<SEND \"buy %s|buy 10*%s\" hint=\"Buy It|buy %s|buy 10 %s\">%s</SEND>"
                                             MXP_RESET MXP_LLOCK,
                                             obj->name, obj->name, obj->short_descr, obj->short_descr, obj->short_descr );
                              if ( obj->condition < 100 )
                                   form_to_char ( ch, " {c<{w%s{c>{w", obj_cond ( obj ) );
                              send_to_char ( "\n\r", ch );
                         }
                         else
                         {
                              form_to_char ( ch, "[{G%3d {Y%6d {R---{w] {W%s{w", obj->level, cost, obj->short_descr );
                              if ( obj->condition < 100 )
                                   form_to_char ( ch,  " {c<{w%s{c>{w", obj_cond ( obj ) );
                              send_to_char ( "\n\r", ch );
                         }
                    }
                    else
                    {
                         count = 1;

                         while ( obj->next_content != NULL && obj->pIndexData == obj->next_content->pIndexData
                                 && obj->condition == obj->next_content->condition &&
                                 !str_cmp ( obj->short_descr, obj->next_content->short_descr ) )
                         {
                              obj = obj->next_content;
                              count++;
                         }
                         if ( ch->desc->mxp )
                         {
                              form_to_char ( ch, "[{G%3d {Y%6d {R%3d{w] {W",
                                             obj->level, cost, count );
                              form_to_char ( ch, MXP_SECURE
                                             "<SEND \"buy %s|buy %d*%s\" hint=\"Buy It|buy %s|buy %d %s\">%s</SEND>"
                                             MXP_RESET MXP_LLOCK,
                                             obj->name, count, obj->name, obj->short_descr, count,
                                             obj->short_descr, obj->short_descr );
                              if ( obj->condition < 100 )
                                   form_to_char ( ch, " {c<{w%s{c>{w", obj_cond ( obj ) );
                              send_to_char ( "\n\r", ch );
                         }
                         else
                         {
                              form_to_char ( ch, "[{G%3d {Y%6d {R%3d{w] {W%s{w", obj->level, cost, count,
                                             obj->short_descr );
                              if ( obj->condition < 100 )
                                   form_to_char ( ch, " {c<{w%s{c>{w", obj_cond ( obj ) );
                              send_to_char ( "\n\r", ch );
                         }
                    }
               }
          }

          if ( !found )
               send_to_char ( "You can't buy anything here.\n\r", ch );
          return;
     }
}

void do_sell ( CHAR_DATA * ch, char *argument )
{
     char                buf[MSL];
     char                arg[MIL];
     CHAR_DATA          *keeper;
     OBJ_DATA           *obj;
     int                 cost, roll;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Sell what?\n\r", ch );
          return;
     }

     if ( ( keeper = find_keeper ( ch ) ) == NULL )
          return;

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          act ( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
          ch->reply = keeper;
          return;
     }

     if ( !can_drop_obj ( ch, obj ) )
     {
          send_to_char ( "You can't let go of it.\n\r", ch );
          return;
     }

     if (obj->owner)
     {
          send_to_char ( "That item is much too valuable to sell.\n\r", ch);
          return;
     }

     if ( !can_see_obj ( keeper, obj ) )
     {
          act ( "$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT );
          return;
     }

     /* won't buy rotting goods */
     if ( obj->timer )
     {
          act ( "$n tells you '$p seems to be decaying.'", keeper, obj, ch, TO_VICT );
          return;
     }

     if ( ( cost = get_cost ( keeper, obj, FALSE ) ) <= 0 )
     {
          act ( "$n tells you '$p doesn't seem worth very much to me.'", keeper, obj, ch, TO_VICT );
          return;
     }

     if ( cost > keeper->gold )
     {
          act ( "$n tells you 'I'm afraid I don't have enough gold to buy $p'.", keeper, obj, ch, TO_VICT );
          return;
     }

     act ( "$n sells $p.", ch, obj, NULL, TO_ROOM );

     /* haggle */
     /* Zeran - haggle is broken for low cost items */

     roll = number_percent();

     if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle] && cost >=20 )
     {
          send_to_char("You haggle with the shopkeeper.\n\r",ch);
          cost += obj->cost / 2 * roll / 100;
          cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
          cost = UMIN(cost,keeper->gold);
          check_improve(ch,gsn_haggle,TRUE,4);
     }

     SNP ( buf, "You sell $p for %d gold piece%s.",  cost, cost == 1 ? "" : "s" );
     act ( buf, ch, obj, NULL, TO_CHAR );
     do_pay ( ch, cost );
     keeper->gold -= cost;
     if ( keeper->gold < 0 )
          keeper->gold = 0;

     if ( obj->item_type == ITEM_TRASH )
     {
          extract_obj ( obj );
     }
     else
     {
          obj_from_char ( obj );

         /* Timer ... removed the reset of timer so rare items won't purge in shops. Lotherius */
         /* obj->timer = number_range(50,100); */
          obj_to_keeper ( obj, keeper );
     }

     return;
}

void do_value ( CHAR_DATA * ch, char *argument )
{
     char                buf[MAX_STRING_LENGTH];
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *keeper;
     OBJ_DATA           *obj;
     int                 cost;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Value what?\n\r", ch );
          return;
     }

     if ( ( keeper = find_keeper ( ch ) ) == NULL )
          return;

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          act ( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
          ch->reply = keeper;
          return;
     }

     if ( !can_see_obj ( keeper, obj ) )
     {
          act ( "$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT );
          return;
     }

     if (obj->owner)
     {
          send_to_char ("That item is much too valuable to sell.\n\r", ch );
          return;
     }

     if ( !can_drop_obj ( ch, obj ) )
     {
          send_to_char ( "You can't let go of it.\n\r", ch );
          return;
     }

     if ( ( cost = get_cost ( keeper, obj, FALSE ) ) <= 0 )
     {
          act ( "$n tells you '$p doesn't seem like it is worth anything to me.'", keeper, obj, ch, TO_VICT );
          return;
     }

     SNP ( buf, "$n tells you '%d gold coins is all I will pay for $p'.", cost );
     act ( buf, keeper, obj, ch, TO_VICT );
     ch->reply = keeper;

     return;
}

/* Zeran - check size of object to wear, and kick out size message. */
bool wear_obj_size ( CHAR_DATA * ch, OBJ_DATA * obj )
{
     if ( obj->size == SIZE_UNKNOWN )
          return TRUE;
     if ( IS_IMMORTAL ( ch ) )
          return TRUE;
     if ( ch->level <= 5 )	/* auto resizing for newbies */
     {
          obj->size = ch->size;
          return TRUE;
     }

     if ( obj->size < ch->size )
     {
          send_to_char ( "That object is too small for you!\n\r", ch );
          act ( "$n tries to use $p, but it is too small for him.", ch, obj, NULL, TO_ROOM );
          return FALSE;
     }
     if ( obj->size > ch->size )
     {
          send_to_char ( "That object is too large for you!\n\r", ch );
          act ( "$n tries to use $p, but it is too large for him.", ch, obj, NULL, TO_ROOM );
          return FALSE;
     }

     return TRUE;
}

void do_resize ( CHAR_DATA * ch, char *argument )
{
     char               *remainder;
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *keeper;
     OBJ_DATA           *obj;
     int                 cost;

     remainder = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Resize what?\n\r", ch );
          return;
     }

     if ( ( keeper = find_keeper ( ch ) ) == NULL )
     {
          send_to_char ( "There is no shopkeeper here.\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
          act ( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
          ch->reply = keeper;
          return;
     }

     if ( !can_drop_obj ( ch, obj ) )
     {
          send_to_char ( "You can't let go of it.\n\r", ch );
          return;
     }

     if ( !can_see_obj ( keeper, obj ) )
     {
          act ( "$n doesn't see what you need to resize.", keeper, NULL, ch, TO_VICT );
          return;
     }

     if (obj->owner && !belongs(ch, obj) )
     {
          send_to_char ("This item does not belong to you.\n\r", ch);
          return;
     }

     if ( obj->timer )
     {
          act ( "$n is sad to inform you that $p is not long for the world.", keeper, obj, ch, TO_VICT );
          return;
     }

     if ( ( cost = get_cost ( keeper, obj, FALSE ) ) <= 0 )
     {
          act ( "$n looks very puzzled, he doesn't seem to know how to resize $p.", keeper, obj, ch, TO_VICT );
          return;
     }

     if ( obj->size == ch->size )
     {
          act ( "$n tells you 'Quit wasting my time, that thing is already perfectly sized for you.'", keeper, NULL, ch, TO_VICT ); return;
     }

     cost = ( abs ( ch->size - obj->size ) * .1 ) * obj->pIndexData->cost;
     remainder = one_argument ( remainder, arg );
     if ( !str_cmp ( arg, "cost" ) )
     {
	  char outbuf[128];
	  SNP ( outbuf, "$n tells you 'Resizing that will cost %d gold.'", cost );
	  act ( outbuf, keeper, NULL, ch, TO_VICT );
          return;
     }
     if ( cost > ch->gold )
     {
	  char outbuf[128];
	  SNP ( outbuf, "$n tells you 'Resizing that will cost %d gold.'", cost );
	  act ( outbuf, keeper, NULL, ch, TO_VICT );
	  act ( "$n tells you 'You don't have enough money'.", keeper, NULL, ch, TO_VICT );
          return;
     }

     act ( "$n painstakingly resizes $p.", keeper, obj, NULL, TO_ROOM ); obj->size = ch->size;
     /* Why did we have a waitstate here? */
     ch->gold -= cost;
     keeper->gold += cost;
     return;
}

void do_repair      ( CHAR_DATA * ch, char *argument )
{
     char      *remainder;
     char 	outbuf[128];
     char 	arg[MAX_INPUT_LENGTH];
     CHAR_DATA *keeper;
     OBJ_DATA  *obj;
     int 	cost;
     int 	multiplier = 0;

     remainder = one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
	  send_to_char ( "Repair what?\n\r", ch ); return;
     }

     // find_keeper notifies player if a keeper can't be found,
     // no need to do it again here.
     if ( ( keeper = find_keeper ( ch ) ) == NULL )
	  return;

     if ( ( obj = get_obj_carry ( ch, arg, NULL ) ) == NULL )
     {
	  act ( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT );
	  ch->reply = keeper; return;
     }

     if ( !can_drop_obj ( ch, obj ) )
     {
	  send_to_char ( "You can't let go of it.\n\r", ch );
	  return;
     }

     if ( !can_see_obj ( keeper, obj ) )
     {
	  act ( "$n doesn't see what you need to repair.", keeper, NULL, ch, TO_VICT ); return;
     }

     if ( obj->timer )
     {
          act ( "$n is sad to inform you that $p is not long for the world.", keeper, obj, ch, TO_VICT );
          return;
     }

     if ( ( cost = get_cost ( keeper, obj, FALSE ) ) <= 0 )
     {
	  act ( "$n looks very puzzled, he doesn't seem to know how to repair $p.", keeper, obj, ch, TO_VICT ); return;
     }

     if ( obj->condition == 100 )
     {
	  act ( "$n tells you 'Quit wasting my time, that thing is already in perfect condition.'",
                keeper, NULL, ch, TO_VICT ); return;
     }

     if ( obj->condition == 0 )
     {
	  act ( "$n tells you 'Quit wasting my time, that thing is far too damaged for me to repair.'",
                keeper, NULL, ch, TO_VICT ); return;
     }
     switch ( material_repa ( obj->material ) )
     {
     case REP_IMPOSSIBLE:
          {
               act ( "$n tells you 'This material cannot be repaired through normal means.'", keeper, NULL, ch, TO_VICT );
               return;
          }
          break;
     case REP_EXTREME:
          multiplier = 9;
          act ( "$n tells you 'This will be extremely difficult to repair.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_HARD:
          multiplier = 5.5;
          act ( "$n tells you 'This will be hard to repair.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_AVGPLUS:
          multiplier = 3.5;
          act ( "$n tells you 'This can be repaired with a little difficulty.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_AVG:
          multiplier = 1;
          act ( "$n tells you 'I can repair this with no problem.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_AVGMINUS:
          multiplier = .75;
          act ( "$n tells you 'This won't be very hard to repair.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_EASY:
          multiplier = .5;
          act ( "$n tells you 'This will be pretty easy to repair.'", keeper, NULL, ch, TO_VICT );
          break;
     case REP_BREEZE:
	  multiplier = .25;
	  act ( "$n tells you 'This is ridiculously easy to repair.'", keeper, NULL, ch, TO_VICT );
          break;
     }

     cost = obj->cost * keeper->pIndexData->pShop->profit_buy / 100;
     cost -= ( cost * obj->condition / 100 );
     cost *= multiplier;

     remainder = one_argument ( remainder, arg );
     if ( !str_cmp ( arg, "cost" ) )
     {
	  SNP ( outbuf, "$n tells you 'Repairing that will cost %d gold.'", cost );
	  act ( outbuf, keeper, NULL, ch, TO_VICT );
          return;
     }

     if ( cost > ch->gold )
     {
	  SNP ( outbuf, "$n tells you 'Repairing that will cost %d gold.'",  cost );
	  act ( outbuf, keeper, NULL, ch, TO_VICT );
	  act ( "$n tells you 'You don't have enough money'.", keeper, NULL, ch, TO_VICT );
          return;
     }

     act ( "$n painstakingly repairs $p.", keeper, obj, NULL, TO_ROOM );
     SNP ( outbuf, "You give %d gold to $n for the cost of repair.", cost );
     act ( outbuf, keeper, NULL, ch, TO_VICT );
     set_obj_cond ( obj, 100 );
     WAIT_STATE ( ch, PULSE_VIOLENCE );
     ch->gold -= cost;
     keeper->gold += cost;
     return;
}

void do_search      ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA 	       *inside_obj;
     OBJ_DATA 	       *ptr;
     EXIT_DATA 	       *pexit;
     int 		base_chance = 50;
     int 		door_chance;
     int 		chance;
     bool 		found = FALSE;
     bool 		foundHdoor = FALSE;
     int 		count;
     int 		pInt;

     pInt = get_curr_stat ( ch, STAT_INT );
     ch->searching = TRUE;
     base_chance = base_chance + 5 * ( pInt - 20 );

     if ( argument[0] != '\0' )
     {
          inside_obj = get_obj_here ( ch, NULL, argument );
          if ( inside_obj == NULL )
          {
               send_to_char ( "You can't find it.\n\r", ch );
               ch->searching = FALSE; return;
          }
	  ptr = inside_obj->contains;
	  for ( ; ptr != NULL; ptr = ptr->next_content )
	  {
               if ( IS_SET ( ptr->extra_flags, ITEM_CONCEALED ) &&
                    can_see_obj ( ch, ptr ) )
               {
                    chance = base_chance + ( ch->level - ptr->level );
                    chance = URANGE ( 1, chance, 99 );
                    if ( number_percent (  ) < chance )
                    {
                         found = TRUE;
                         ptr->extra_flags -= ITEM_CONCEALED;
                    }
               }
          }
          /* end for loop */
     }
     /* end inside another object routine */
     else			/*search for objects and doors */
     {
          door_chance = ch->level / 2;
          if ( pInt > 18 )
               door_chance = door_chance + 5 * ( pInt - 18 );
          ptr = ch->in_room->contents;
          for ( ; ptr != NULL; ptr = ptr->next_content )
          {
               if ( IS_SET ( ptr->extra_flags, ITEM_CONCEALED ) &&
                    can_see_obj ( ch, ptr ) )
               {
                    chance = base_chance + ( ch->level - ptr->level );
                    chance = URANGE ( 1, chance, 99 );
                    if ( number_percent (  ) < chance )
                    {
                         found = TRUE; ptr->extra_flags -= ITEM_CONCEALED;
                    }
               }
          }
          /* end for loop */
          for ( count = 0; count < 6; count++ )
          {
               pexit = ch->in_room->exit[count];
               if ( !pexit )
                    continue;
               // if ( IS_SET ( pexit->exit_info, EX_CLOSED ) && IS_SET ( pexit->exit_info, EX_HIDDEN ) )
               if ( IS_SET ( pexit->exit_info, EX_HIDDEN ) )
               {
                    if ( number_percent (  ) < door_chance )	/* found door */
                    {
                         form_to_char ( ch, "You find a secret door leading %s!\n\r", dir_name[count] );
                         REMOVE_BIT ( pexit->exit_info, EX_HIDDEN );
                         foundHdoor = TRUE;
                    }
               }
          }
          /*end for loop */
     }
     /* end in room section */

     if ( found )
	  send_to_char ( "You've found something concealed!\n\r", ch );
     else if ( !found && !foundHdoor )
     {
          send_to_char ( "You find nothing of interest.\n\r", ch );
          WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );
     }
     ch->searching = FALSE;
     return;

}

int item_lookup(const char *name)
{
     int type;

     for (type = 0; type_flags[type].name != NULL; type++)
     {
          if (LOWER(name[0]) == LOWER ( type_flags[type].name[0] )
              &&  !str_prefix(name, type_flags[type].name ) )
               return type_flags[type].bit;
     }

     return -1;
}

bool belongs (CHAR_DATA *ch, OBJ_DATA *obj)
{
     if (!str_cmp(ch->name, obj->owner) )
          return TRUE;
     return FALSE;
}

void do_owned(CHAR_DATA *ch, char *argument)
{
     BUFFER             *buffer;
     OBJ_DATA           *search_obj;
     bool                found = FALSE;

     buffer = buffer_new(1000);

     bprintf ( buffer, "{c[Ser #]  name                       location{x\n\r" );
     bprintf ( buffer, "{w-------------------------------------------------\n\r" );

     for ( search_obj = object_list; search_obj != NULL; search_obj = search_obj->next )
     {
          if (search_obj->owner && (search_obj->owner[0] != '\0') && (belongs(ch, search_obj ))  )
          {
               if ( search_obj->in_room != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s      in room    %s\n\r",
                              search_obj->serialnum,
                              search_obj->short_descr,
                              search_obj->in_room->name );
                    found = TRUE;
               }
               else if ( search_obj->in_obj != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s      inside      %-20s\n\r",
                              search_obj->serialnum,
                              search_obj->short_descr,
                              search_obj->in_obj->short_descr );
                    found = TRUE;
               }
               else if ( search_obj->carried_by != NULL )
               {
                    bprintf ( buffer, "[%5d] %-20s      carried by  %-20s\n\r",
                              search_obj->serialnum,
                              search_obj->short_descr,
                              PERS ( search_obj->carried_by, ch ) );
                    found = TRUE;
               }
               else
                    continue;
          }
     }

     if ( !found )
          send_to_char ("You don't seem to have any owned items in the world.\n\r", ch);
     else
          page_to_char ( buffer->data, ch );
     buffer_free(buffer);
     return;
}

/* Zeran - obj condition support */
/* Lotherius - changed show_obj_cond to obj_cond, now returns a text string
 * rather than trying to format/show the condition itself, simplifies things
 * quite a bit.
 */
char *obj_cond ( OBJ_DATA * obj )
{
     if ( IS_SET ( obj->extra_flags, ITEM_NO_COND ) )	/* no show condition, now just doesn't lower condition */
          return "perfect";
     if ( obj->condition >= 100 )           return "perfect"; // Should never be > 100, but to be safe.
     else if ( obj->condition >  90 )       return "very good";
     else if ( obj->condition >  80 )       return "good";
     else if ( obj->condition >  75 )       return "decent";
     else if ( obj->condition >  65 )       return "dented";
     else if ( obj->condition >  55 )       return "worn";
     else if ( obj->condition >  50 )       return "very worn";
     else if ( obj->condition >  35 )       return "weakened";
     else if ( obj->condition >  25 )       return "cracked";
     else if ( obj->condition >  15 )       return "damaged";
     else if ( obj->condition >  0  )       return "broken";
     else if ( obj->condition <= 0  )       return "worthless"; // Should never be less than 0
     else return "unknown?";
}

void check_damage_obj ( CHAR_DATA * ch, OBJ_DATA * obj,	int chance, int damtype )
{
     bool                checkall = FALSE;
     bool                done = FALSE;
     int                 damage_pos;
     OBJ_DATA           *dobj = NULL;
     int                 stop = 0;

     /* Assumption - NULL obj means check all equipment */
     if ( obj == NULL )
          checkall = TRUE;

     if ( checkall )		/*damage random equipped item */
     {
          if ( number_percent (  ) <= chance )	/*something dinged up */
          {
               while ( ( !done ) && ( stop <= 20 ) )	/* stop prevents infinite loop */
               {
                    damage_pos = number_range ( 1, MAX_WEAR );
                    if ( ( dobj = get_eq_char ( ch, damage_pos ) ) != NULL )
                         done = TRUE;
                    stop++;
               }
               if ( done )
               {
                    damage_obj ( ch, dobj, 1, damtype );
               }
          }
          else
               return;
     }
     else if ( number_percent (  ) <= chance )	/*damage passed in object */
     {
          damage_obj ( ch, obj, 1, damtype );
          return;
     }
     return;
}

/*
 * Cause damage to 1 object.
 * Rewritten to be much more robust - Lotherius
 * Could use a lot of tweaking for realism with different material/item_type combinations
 */

void damage_obj ( CHAR_DATA * ch, OBJ_DATA * obj, int damage, int damtype )
{
     OBJ_DATA *cobj, *nobj;
     int       durable;
     char      dambuf[MIL];
     char      destbuf[MIL];

     if ( obj == NULL )
     {
          bugf ( "NULL obj passed to damage_obj" );
          return;
     }

     /* We don't wanna damage owned items */
     if ( obj->owner )
          return;

     if ( IS_SET ( obj->extra_flags, ITEM_NO_COND ) ) // We don't damage NO_COND items.
          return;

     // Modify damage based on Object Durability
     durable = material_dura ( obj->material );
     if ( durable == DUR_MAX ) // Indestructable
          return;
     else
          durable -= 1;  // Subtract 1 from durable amount, since we're adding "damage" back in.
     damage += durable;  // This way less durable objects take more damage. Simple but effective.

     SNP ( dambuf, "has been damaged." );   // Default item damage message.
     SNP ( destbuf, "has been destroyed." ); // Default destruction message.

     /* Check Damtype */
     switch ( damtype )
     {
     case DAM_NONE: // No damage? Okay....
     case DAM_OTHER:
          break;
     case DAM_BLEEDING: // Should never happen..... I would think.
          if ( is_material ( obj->material, MAT_LIVING ) )
          {
               SNP ( dambuf, "has bled badly." );
               SNP ( destbuf, "has been bled dry." );
               damage *= 3;
          }
          else
               damage /= 2;
          break;
     case DAM_BASH:
          SNP ( destbuf, "has been crushed." );
          if ( is_material ( obj->material, MAT_BREAKABLE ) )
          {
               SNP ( dambuf, "has been chipped." );
               SNP ( destbuf, "has shattered." );
               damage *= 5;
          }
          if ( is_material ( obj->material, MAT_VERYHARD ) )
               damage /= 2;
          break;
     case DAM_PIERCE:
          if ( is_material ( obj->material, MAT_LIVING ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_SOFT ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_SEW ) )
               SNP ( destbuf, "has been ripped open." );
          break;
     case DAM_SLASH:
          SNP ( destbuf, "has been sliced in half." );
          if ( is_material ( obj->material, MAT_LIVING ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_SOFT ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_CARVE ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_SEW ) )
               SNP ( destbuf, "has been ripped open." );
          break;
     case DAM_FIRE: // Need to split fire into Normal, VHot and Magical so this works right.
          SNP ( dambuf, "has been scorched." );
          SNP ( destbuf, "has burned to a crisp." );
          if ( is_material ( obj->material, MAT_FLAMMABLE ) )
               damage *= 5;
          if ( is_material ( obj->material, MAT_VERYFLAMMABLE ) )
               damage = 100; // Totally destroy very flammable
          if ( is_material ( obj->material, MAT_LIVING ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_LIQUID ) )
          {
               SNP ( dambuf, "simmers from the heat." );
               SNP ( destbuf, "boils completely away." );
               damage *= 1.5; // boiling
          }
          if ( is_material ( obj->material, MAT_MELT_ALWAYS ) ||
               is_material ( obj->material, MAT_MELT_NORMAL ) ||
               is_material ( obj->material, MAT_MELT_VHOT )   ||
               is_material ( obj->material, MAT_MELT_MAGICAL ) )
          {
               SNP ( dambuf, "has partially melted." );
               SNP ( destbuf, "has melted completely." );
               if ( is_material ( obj->material, MAT_MELT_ALWAYS ) )
                    damage *= 5;
               if ( is_material ( obj->material, MAT_MELT_NORMAL ) )
                    damage *= 3;
               if ( is_material ( obj->material, MAT_MELT_VHOT ) )
                    damage *= 2;
               if ( is_material ( obj->material, MAT_MELT_MAGICAL ) )
                    damage *= 1.5;
          }
          if ( is_material ( obj->material, MAT_FIREPROOF ) )
               damage = 0;
          break;
     case DAM_COLD:
          SNP ( dambuf, "has cracked from cold." );
          SNP ( destbuf, "has broken in two from freezing." );
          if ( is_material ( obj->material, MAT_LIVING ) )
          {
               SNP ( dambuf, "has suffered frostbite." );
               SNP ( destbuf, "has frozen to death." );
               damage *= 2;
          }
          if ( is_material ( obj->material, MAT_LIQUID ) )
          {
               switch ( obj->item_type )
               {
               case ITEM_POTION:
               case ITEM_COMPONENT:
                    damage *= 1.5; // Freezing
                    SNP ( dambuf, "has gotten too cold and lost some of its magic." );
                    SNP ( destbuf, "has frozen completely and burst." );
                    break;
               default:
                    break;
               }
          }
          break;
     case DAM_LIGHTNING:
          SNP ( dambuf, "was damaged by the lightning." );
          if ( is_material ( obj->material, MAT_VERYFLAMMABLE ) )
          {
               SNP ( destbuf, "caught on fire and burned up." );
               damage *= 25; // Catches on fire from electricity
          }
          if ( is_material ( obj->material, MAT_LIVING ) )
          {
               SNP ( destbuf, "was electrocuted." );
               damage *= 2;
          }
          if ( is_material ( obj->material, MAT_CONDUCTIVE ) || is_material ( obj->material, MAT_METAL )
               || is_material ( obj->material, MAT_ROCK ) )
               damage /= 2;
          break;
     case DAM_ACID:
          if ( !is_material ( obj->material, MAT_ACIDETCH ) )
          {
               damage = 0; // If acid don't hurt it....
               break;
          }
          SNP ( dambuf, "has been pitted and etched by acid." );
          SNP ( destbuf, "has been completely consumed by acid." );
          if ( is_material ( obj->material, MAT_ROCK ) ) // Rock is usually more vulnerable
               damage *= 1.5;
          if ( is_material ( obj->material, MAT_LIVING ) )
               damage *= 2;
          if ( is_material ( obj->material, MAT_VERYHARD ) )
               damage /= 1.5;
          if ( is_material ( obj->material, MAT_GASEOUS ) )
               damage /= 2;
          break;
     case DAM_POISON:
          if ( !is_material ( obj->material, MAT_LIVING ) )
               damage = 0;
          else
          {
               SNP ( dambuf, "suffered from poison." );
               SNP ( destbuf, "was poisoned and died." );
          }
          break;
     case DAM_NEGATIVE:
          SNP ( dambuf, "was drained of some of its essence." );
          SNP ( destbuf, "ceased to exist." );
          if ( is_material ( obj->material, MAT_NOMAGIC ) )
               damage /= 2;
          if ( is_material ( obj->material, MAT_PARTMAGIC ) )
               damage /= 1.5;
          break;
     case DAM_HOLY:
          SNP ( dambuf, "suffered from holy wrath." );
          SNP ( destbuf, "was obliterated by holy wrath." );
          if ( is_material ( obj->material, MAT_NOMAGIC ) )
               damage /= 1.5;
          if ( !IS_SET ( obj->extra_flags, ITEM_EVIL ) && !IS_SET ( obj->extra_flags, ITEM_ANTI_GOOD ) )
               damage = 0; // Non-Evil items don't get wrathed.
          break;
     case DAM_ENERGY:
          if ( is_material ( obj->material, MAT_CONDUCTIVE ) )
               damage /= 1.5;
          break;
     case DAM_MENTAL:
          if ( !is_material ( obj->material, MAT_LIVING ) && !is_material ( obj->material, MAT_MAGIC ) )
               damage = 0;
          break;
     case DAM_DISEASE:
          if ( !is_material ( obj->material, MAT_LIVING ) )
               damage = 0;
          break;
     case DAM_DROWNING:
          if ( is_material ( obj->material, MAT_RUSTABLE ) )
          {
               SNP ( dambuf, "has rusted." );
               SNP ( destbuf, "has rusted into uselessness." );
          }
          if ( is_material ( obj->material, MAT_INKBLEED ) )
          {
               SNP ( dambuf, "has lost some of its writing." );
               SNP ( destbuf, "has had all the ink washed away." );
               damage *= 3;
          }
          if ( is_material ( obj->material, MAT_DISSOLVE ) )
          {
               SNP ( dambuf, "has dissolved partially." );
               SNP ( destbuf, "has been completely dissolved." );
               damage *= 10;
          }
          if ( is_material ( obj->material, MAT_LIQUID ) || is_material ( obj->material, MAT_GASEOUS ) )
               damage = 0;
          if ( is_material ( obj->material, MAT_METAL ) || is_material ( obj->material, MAT_ROCK ) )
               damage = 0;
          break;
     case DAM_LIGHT:
          if ( is_material ( obj->material, MAT_INKBLEED ) )
          {
               SNP ( dambuf, "has faded somewhat." );
               SNP ( destbuf, "has faded and is now unreadable." );
               damage *= 1.5; // Fading...
          }
          if ( is_material ( obj->material, MAT_METAL ) || is_material ( obj->material, MAT_ROCK ) )
               damage = 0;
          break;
     case DAM_HARM:
          if ( is_material ( obj->material, MAT_LIVING ) )
               damage *= 2; // Targets living
          else
               damage = 0;
          break;
     case DAM_HANDTOHAND:
          if ( is_material ( obj->material, MAT_BREAKABLE ) )
          {
               SNP ( dambuf, "has been cracked." );
               SNP ( destbuf, "has been shattered." );
               damage *= 3;
          }
          else if ( is_material ( obj->material, MAT_ROCK ) ||
                    is_material ( obj->material, MAT_METAL ) ||
                    is_material ( obj->material, MAT_VERYHARD ) )
               damage = 0;
          if ( is_material ( obj->material, MAT_SEW ) )
               SNP ( destbuf, "has been ripped open." );
          break;
     default:
          break;
     }

     if ( is_material ( obj->material, MAT_ETHEREAL ) )
          damage /= 2; // Ethereal objects take less damage

     if ( damage <= 0 ) // We aren't hurting anything
          return;

     obj->condition -= damage;
     obj->condition = URANGE ( 0, obj->condition, 100 );

    /*Check for item unusable */
     if ( obj->condition <= 0 )
     {
          form_to_char ( ch, "Your {c%s{w %s\n\r",
                         ( ( obj->short_descr && obj->short_descr[0] != '\0' && can_see_obj ( ch, obj ) )
                           ? obj->short_descr : "Something" ), destbuf );
          switch ( obj->item_type )
          {
          case ITEM_CONTAINER:
          case ITEM_CORPSE_PC:	// Corpses aren't "safe" containers!
          case ITEM_CORPSE_NPC:
               for ( cobj = obj->contains; cobj; cobj = nobj )
               {
                    nobj = cobj->next_content;
                    obj_from_obj ( cobj );

                    if ( number_bits ( 2 ) == 0 || ch->in_room == NULL )
                    {
                         damage_obj ( ch, cobj, 1, damtype );
                    }
                    else
                    {
                         act ( "Your $p falls to the floor.", ch, cobj, NULL, TO_CHAR );
                         if ( is_material (cobj->material, MAT_BREAKABLE ) )
                              damage_obj ( ch, obj, 1, DAM_BASH );
                         if ( cobj ) // If it wasn't destroyed in the fall
                              obj_to_room ( cobj, ch->in_room );
                    }
               }
          case ITEM_DRINK_CON:
               {
                    act ( "The liquid from $p spills!", ch, obj, NULL, TO_CHAR );
                    for ( cobj = ch->carrying; cobj != NULL; cobj = nobj )
                    {
                         nobj = cobj->next_content;
                         // Hit only the first of this type of object found.
                         if ( is_material ( cobj->material, MAT_INKBLEED) || is_material ( cobj->material, MAT_DISSOLVE ) )
                         {
                              damage_obj ( ch, obj, 1, DAM_DROWNING );
                              break;
                         }
                    }
               }
          }
          if ( obj->wear_loc != WEAR_NONE )
               unequip_char ( ch, obj );  // AC should be 0 if OBJ Condition is 0, so no need to worry about
          // double subtraction of AC
          extract_obj ( obj );
          // Should create a trash obj here to give the character to replace the original.
          return;
     }
     else
     {
          if ( damage > 1 ) // Don't show low damage amounts.
          {
               form_to_char ( ch, "Your {c%s{w %s\n\r",
                              ( ( obj->short_descr && obj->short_descr[0] != '\0' && can_see_obj ( ch, obj ) )
                                ? obj->short_descr : "Something" ), dambuf );
          }
     }
     return;
}

/* This function should go now. */
void set_obj_cond ( OBJ_DATA * obj, int condition )
{
     int                 counter;

     obj->condition = condition;
     if ( obj->item_type == ITEM_ARMOR )
          for ( counter = 0; counter < 4; counter++ )
          {
               obj->value[counter] = obj->valueorig[counter] * ( obj->condition ) / 100;
               if ( ( obj->value[counter] == 0 )
                    && ( obj->valueorig[counter] != 0 ) )
                    obj->value[counter] = 1;	/*always worth something til it breaks */
          }
     return;
}

void do_pull ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Yes, but pull WHAT?\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_PULL ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_PULL );
     else
          send_to_char ( "Nothing happened.\n\r", ch );
     return;
}

void do_push ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Yes, but push WHAT?\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_PUSH ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_PUSH );
     else
          send_to_char ( "Nothing happened.\n\r", ch );
     return;
}

void do_climb ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "What do you want to climb?\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_CLIMB ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_CLIMB );
     else
          send_to_char ( "It doesn't seem to go anywhere.\n\r", ch );
     return;
}

void do_turn ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "You turn.... Maybe you meant to turn something?\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_TURN ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_TURN );
     else
          send_to_char ( "Nothing happened.\n\r", ch );
     return;
}

void do_play ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "You air lute like the best of them.\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_PLAY ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_PLAY );
     else
          send_to_char ( "It doesn't seem to be playable.\n\r", ch );
     return;
}

void do_twist ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Come on baby... Let's do the twist... but twist WHAT??\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_TWIST ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_TWIST );
     else
          send_to_char ( "Nothing happened.\n\r", ch );
     return;
}

void do_lift ( CHAR_DATA *ch, char *argument )
{
     OBJ_DATA *obj;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Yes, but lift WHAT?\n\r", ch );
          return;
     }
     if ( ( obj = get_obj_here ( ch, NULL, argument ) ) == NULL )
     {
          send_to_char ( "You don't see anything like that.\n\r", ch );
          return;
     }
     if ( HAS_TRIGGER_OBJ( obj, TRIG_LIFT ) )
          p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_LIFT );
     else
          send_to_char ( "Okay.\n\r", ch );
     return;
}

void do_dig ( CHAR_DATA *ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_DIG ) )
          p_percent_trigger( NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_DIG );
     else
          send_to_char ( "You find nothing.\n\r", ch );
     return;
}

void do_bank ( CHAR_DATA *ch, char *argument )
{
     if ( IS_NPC ( ch ) )
          return;

     if ( ch->pcdata->bankaccount > 0 )
          form_to_char ( ch, "You have {Y%ld{w gold coins in the bank.\n\r", ch->pcdata->bankaccount );
     else if ( ch->pcdata->bankaccount == 1 )
          send_to_char ( "You have {Y1{w lonely coin in the bank.\n\r", ch );
     else if ( ch->pcdata->bankaccount == 0 )
          send_to_char ( "You have {Rno money{w in the bank.\n\r", ch );
     else
          form_to_char ( ch, "{RYou owe the bank {Y%ld{R gold coins.{w\n\r", abs(ch->pcdata->bankaccount) );

     return;
}

void do_withdraw ( CHAR_DATA *ch, char *argument )
{
     long i;
     bool all = FALSE;

     if ( IS_NPC ( ch ) )
          return;

     if ( !str_cmp ( argument, "all" ) )
          all = TRUE;
     else if ( !is_number(argument) )
     {
          send_to_char ( "Syntax: withdraw <amount>\n\r", ch );
          send_to_char ( "        withdraw all\n\r", ch );
          return;
     }

     if ( !IS_SET ( ch->in_room->room_flags, ROOM_BANK ) )
     {
          send_to_char ( "Maybe you should look for a bank first?\n\r", ch );
          return;
     }

     if ( all )
          i = ch->pcdata->bankaccount;
     else
          i = atol( argument );

     if ( ch->pcdata->bankaccount < 0 )
     {
          send_to_char ( "You already owe the bank money!\n\r", ch );
          return;
     }
     else if ( i < 0 )
     {
          send_to_char ( "You can't withdraw a negative amount!\n\r", ch );
          return;
     }
     else if ( i == 0 )
     {
          send_to_char ( "The banker stares at you blankly.\n\r", ch );
          return;
     }
     else if ( i > ch->pcdata->bankaccount )
     {
          send_to_char ( "You don't have that much money in your account!\n\rPerhaps you could take out a loan?\n\r", ch );
          return;
     }

     ch->pcdata->bankaccount -= i;
     ch->gold += i;

     form_to_char ( ch, "You withdraw {Y%ld{w coin%s from your account.\n\r", i, i > 1 ? "s" : "" );
     save_char_obj ( ch );
     return;
}

void do_deposit ( CHAR_DATA *ch, char *argument )
{
     long i;
     bool all = FALSE;

     if ( IS_NPC ( ch ) )
          return;

     if ( !str_cmp ( argument, "all" ) )
          all = TRUE;
     else if ( !is_number(argument) )
     {
          send_to_char ( "Syntax: deposit <amount>\n\r", ch );
          send_to_char ( "        deposit all\n\r", ch );
          return;
     }

     if ( !IS_SET ( ch->in_room->room_flags, ROOM_BANK ) )
     {
          send_to_char ( "Maybe you should look for a bank first?\n\r", ch );
          return;
     }

     if ( all )
          i = ch->gold;
     else
          i = atol( argument );

     if ( ch->gold <= 0 )
     {
          send_to_char ( "You haven't got any gold!\n\r", ch );
          return;
     }
     else if ( i < 0 )
     {
          send_to_char ( "If you want to withdraw money, try withdraw!\n\r", ch );
          return;
     }
     else if ( i == 0 )
     {
          send_to_char ( "The banker stares at you blankly.\n\r", ch );
          return;
     }

     else if ( i > ch->gold )
     {
          send_to_char ( "You don't have that much mony on hand!\n\r", ch );
          return;
     }

     ch->pcdata->bankaccount += i;
     ch->gold -= i;

     form_to_char ( ch, "You deposit {Y%ld{w coin%s into your account.\n\r", i, i > 1 ? "s" : "" );
     save_char_obj ( ch );
     return;
}

void do_borrow ( CHAR_DATA *ch, char *argument )
{
     int i, j;
     bool fAuto = FALSE;

     if ( IS_NPC ( ch ) )
          return;

     if ( !str_cmp ( argument, "auto" ) )
          fAuto = TRUE;

     if ( argument[0] == '\0' || (fAuto) )
     {
          i = ( ch->level * ( ch->level*2) ) + 100;

          if ( ch->pcdata->bankaccount < 0 )
          {
               if ( !fAuto )
                    form_to_char ( ch, "You already owe the bank {Y%ld{w coin%s.\n\r", abs(ch->pcdata->bankaccount ),
                                   abs(ch->pcdata->bankaccount) > 1 ? "s" : "" );
               i += ch->pcdata->bankaccount;
               if ( i > 0 )
               {
                    form_to_char ( ch, "You may borrow {Y%d{w more coin%s.\n\r", i, i > 1 ? "s" : "" );
                    if ( !fAuto )
                         send_to_char ("All loans are charged a 25% loan fee plus a processing fee equal to your level.\n\r",
                                       ch );
               }
               else
                    send_to_char ( "{RYour credit is currently maxxed.{w\n\r", ch );
               return;
          }
          form_to_char ( ch, "You may borrow up to {Y%d{w coins.\n\r", i );
          return;
     }
     else if ( !is_number(argument) )
     {
          send_to_char ( "Syntax: borrow <amount>\n\r", ch );
          return;
     }

     if ( !IS_SET ( ch->in_room->room_flags, ROOM_BANK ) )
     {
          send_to_char ( "Maybe you should look for a bank first?\n\r", ch );
          return;
     }

     j = atoi(argument);
     if ( j <= 0 )
     {
          send_to_char ( "Please specify a realistic amount.\n\r", ch );
          return;
     }
     i = ( ch->level * ( ch->level*2) ) + 100;

     if ( ch->pcdata->bankaccount < 0 )
          i += ch->pcdata->bankaccount;

     if ( i <= 0 )
     {
          send_to_char ( "{RYour credit is maxxed.{w\n\r", ch );
          return;
     }

     if ( j > i )
     {
          form_to_char ( ch, "{RYou may only borrow {Y%d{w coins at this time.{w\n\r", i );
          return;
     }

     if ( ch->pcdata->bankaccount > 0 )
     {
          if ( j <= ch->pcdata->bankaccount )
          {
               form_to_char ( ch, "The bank knows you already have {Y%ld{w coins in your account and {Rdeclines{w the loan.\n\r",
                              ch->pcdata->bankaccount );
               return;
          }
          else
          {
               int k;
               k = ch->pcdata->bankaccount;
               ch->pcdata->bankaccount -= k;
               ch->gold += k;
               j -= k;
               form_to_char ( ch, "Your loan has been reduced to {Y%d{w since you already had {Y%d{w in your account.\n\r",
                              j, k );
               send_to_char ( "  {C({wThe amount has been included in the proceeds you recieve{C){w\n\r", ch );
          }
     }

     form_to_char ( ch, "You borrow {Y%ld{w coins from the bank.\n\r", j );
     form_to_char ( ch, "A {Y%d{w gold piece fee has been charged for this loan.\n\r", (j/4)+ch->level );

     ch->pcdata->bankaccount -= ( j + (j/4) );
     ch->gold += j;

     save_char_obj ( ch );
     return;
}

