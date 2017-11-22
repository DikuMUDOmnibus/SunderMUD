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
DECLARE_DO_FUN ( do_look );
DECLARE_DO_FUN ( do_recall );
DECLARE_DO_FUN ( do_stand );

char               *const dir_name[] =
{
     "north", "east", "south", "west", "up", "down"
};

const sh_int        rev_dir[] =
{
     2, 3, 0, 1, 5, 4
};

const sh_int        movement_loss[SECT_MAX] =
{
     1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 3, 1
};

/*
 * Local functions.
 */
int find_door       args ( ( CHAR_DATA * ch, char *arg ) );
bool has_key        args ( ( CHAR_DATA * ch, int key ) );

// Follow seems to be ignored here... Hmm.....
// wonder what that bool is for anywho...

void move_char ( CHAR_DATA * ch, int door, bool follow )
{
     CHAR_DATA          *fch;
     CHAR_DATA          *fch_next;
     ROOM_INDEX_DATA    *in_room;
     ROOM_INDEX_DATA    *to_room;
     EXIT_DATA          *pexit;

     if ( door < 0 || door > 5 )
     {
          bugf ( "move_char: bad door %d.", door );
          return;
     }

     /*
      * Exit trigger, if activated, bail out. Only PCs are triggered.
      */
     
     if ( !IS_NPC(ch)
          && (p_exit_trigger( ch, door, PRG_MPROG )
              ||  p_exit_trigger( ch, door, PRG_OPROG )
              ||  p_exit_trigger( ch, door, PRG_RPROG )) )
          return;

     in_room = ch->in_room;
     if ( ( pexit = in_room->exit[door] ) == NULL
          || ( to_room = pexit->u1.to_room ) == NULL
          || !can_see_room ( ch, pexit->u1.to_room ) )
     {
          send_to_char ( "You bruise your nose and realize that path is blocked.\n\r", ch );
          if (ch->hit > 1)
               ch->hit--; /* Reduce hit by one. Yes this is mean. */
          return;
     }

     if ( IS_SET ( pexit->exit_info, EX_CLOSED )
          && !IS_SET ( pexit->exit_info, EX_HIDDEN )
          && !IS_AFFECTED ( ch, AFF_PASS_DOOR ) )
     {
          act ( "As you bruise your nose, you realize the $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
          if (ch->hit > 1)
               ch->hit--; /* Reduce hit by one. Yes this is mean. */
          return;
     }

     if ( IS_SET ( pexit->exit_info, EX_CLOSED )
          && IS_SET ( pexit->exit_info, EX_HIDDEN )
          && !IS_AFFECTED ( ch, AFF_PASS_DOOR ) )
     {
          send_to_char ( "You bruise your nose and realize that path is blocked.\n\r", ch );
          if (ch->hit > 1)
               ch->hit--; /* Reduce hit by one. Yes this is mean. */
          return;
     }

     if ( IS_SET ( pexit->exit_info, EX_CLOSED )
          && IS_SET ( pexit->exit_info, EX_HIDDEN )
          && IS_SET ( pexit->exit_info, EX_NO_PASS )
          && IS_AFFECTED ( ch, AFF_PASS_DOOR ) )
     {
          send_to_char ( "You bruise your nose and realize that path is blocked.\n\r", ch );
          if (ch->hit > 1)
               ch->hit--; /* Reduce hit by one. Yes this is mean. */
          return;
     }

     if ( IS_SET ( pexit->exit_info, EX_CLOSED )
          && IS_SET ( pexit->exit_info, EX_NO_PASS )
          && IS_AFFECTED ( ch, AFF_PASS_DOOR ) )
     {
          send_to_char ( "Strange energies swirl around the door blocking your way.\n\r",ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_CHARM )
          && ch->master != NULL && in_room == ch->master->in_room )
     {
          send_to_char ( "What?  And leave your beloved master?\n\r", ch );
          return;
     }

     if ( room_is_private ( to_room ) )
     {
          send_to_char ( "That room is private right now.\n\r", ch );
          return;
     }
     
     if ( to_room->area->llev > (ch->level+10) )
     {
          form_to_char ( ch, "{RYou must be at least level {Y%d {Rto enter {w%s{R.{w",
                         (to_room->area->llev-10), to_room->area->name );
          return;
     }

     if ( !IS_NPC ( ch ) )
     {
          int                 move;
          char                weathermsg[20];
          char                walkmsg[10];

          if ( in_room->sector_type == SECT_AIR
               || to_room->sector_type == SECT_AIR )
          {
               if ( !IS_AFFECTED ( ch, AFF_FLYING ) &&
                    !IS_IMMORTAL ( ch ) )
               {
                    send_to_char ( "You can't fly.\n\r", ch );
                    return;
               }
          }

          if ( ( in_room->sector_type == SECT_WATER_NOSWIM
                 || to_room->sector_type == SECT_WATER_NOSWIM )
               && !IS_AFFECTED ( ch, AFF_FLYING ) )
          {
               OBJ_DATA           *obj;
               bool                found;

               /*
                * Look for a boat.
                */
               found = FALSE;

               if ( IS_IMMORTAL ( ch ) )
                    found = TRUE;

               for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
               {
                    if ( obj->item_type == ITEM_BOAT )
                    {
                         found = TRUE;
                         break;
                    }
               }
               if ( !found )
               {
                    send_to_char ( "You need a boat to go there.\n\r", ch );
                    return;
               }
          }

          move =  movement_loss[UMIN ( SECT_MAX - 1, in_room->sector_type )] +
               movement_loss[UMIN ( SECT_MAX - 1, to_room->sector_type )];

          move /= 2;		/* i.e. the average */
          /* Zeran - modify by encumbrance */
          /* 10% for each point above or below norm of 1 */
          move = move + ( ( total_encumbrance ( ch ) - 1 ) / 10 * move );
          /* Make sure movement is at least 1 point */
          move = UMAX ( 1, move );

          if ( ch->move < move )
          {
               send_to_char ( "You are too exhausted.\n\r", ch );
               return;
          }
          /* move messages */
          weathermsg[0] = '\0';
          walkmsg[0] = '\0';

          if ( weather_info.change >= 0 )	/* summer */
          {
               switch ( weather_info.sky )
               {
               case SKY_CLOUDLESS:
                    SLCAT ( weathermsg, "hot" );
                    SLCAT ( walkmsg, "walk" );
                    break;
               case SKY_CLOUDY:
                    SLCAT ( weathermsg, "warm" );
                    SLCAT ( walkmsg, "walk" );
                    break;
               case SKY_RAINING:
                    SLCAT ( weathermsg, "wet" );
                    SLCAT ( walkmsg, "splash" );
                    break;
               case SKY_LIGHTNING:
                    SLCAT ( weathermsg, "stormy" );
                    SLCAT ( walkmsg, "splash" );
                    break;
               }
               /* end of switch */
          }
          /* end of if */
          else
          {
               switch ( weather_info.sky )
               {
               case SKY_CLOUDLESS:
                    SLCAT ( weathermsg, "frigid" );
                    SLCAT ( walkmsg, "walk" );
                    break;
               case SKY_CLOUDY:
                    SLCAT ( weathermsg, "cold" );
                    SLCAT ( walkmsg, "walk" );
                    break;
               case SKY_RAINING:
                    SLCAT ( weathermsg, "snowy" );
                    SLCAT ( walkmsg, "trudge" );
                    break;
               case SKY_LIGHTNING:
                    SLCAT ( weathermsg, "blizzard-stricken" );
                    SLCAT ( walkmsg, "trudge" );
                    break;
               }
               /* end of switch */
          }
          /* end of else */

          if ( IS_AFFECTED ( ch, AFF_FLYING ) )
          {
               walkmsg[0] = '\0';
               SLCAT ( walkmsg, "fly" );
          }

          /* leaving an indoors room */

          if ( ( in_room->sector_type == SECT_INSIDE )
               || ( in_room->sector_type == SECT_UNDERGROUND ) )
          {
               switch ( to_room->sector_type )
               {
               case SECT_INSIDE:
               case SECT_MAX:
               case SECT_UNDERGROUND:
                    break;		/* no message for this movement */
               case SECT_WATER_SWIM:
                    sound ("waterysnd.wav", ch);
                    send_to_char ("You splash into the water.\n\r", ch);
                    break;
               case SECT_WATER_NOSWIM:
                    sound ("wavecrash.wav", ch);
                    break;
               default:
                    form_to_char ( ch, "You %s into the %s %s.\n\r",
                                   walkmsg,
                                   weathermsg,
                                   flag_string ( sector_name, to_room->sector_type ) );
                    break;
               }
               /* end of switch */
          }

          /* end of indoors room leave routine */
          /* leaving a city */
          if ( in_room->sector_type == SECT_CITY )
          {
               switch ( to_room->sector_type )
               {
               case SECT_CITY:
               case SECT_MAX:
                    break;		/* no message for this movement */
               case SECT_WATER_SWIM:
                    sound ("waterysnd.wav", ch);
                    send_to_char ("You splash into the water.\n\r", ch);
                    break;
               case SECT_WATER_NOSWIM:
                    sound ("wavecrash.wav", ch);
                    break;
               case SECT_INSIDE:
                    form_to_char ( ch, "You leave the %s city streets and take shelter indoors.\n\r", weathermsg );
                    break;
               case SECT_FORT:
                    form_to_char ( ch, "You leave the %s city streets and enter a fort.\n\r", weathermsg );
                    break;
               case SECT_UNDERGROUND:
                    send_to_char ( "You go underground.\n\r", ch );
                    break;
               default:
                    form_to_char ( ch, "You %s out of the city for the %s %s.\n\r",
                                   walkmsg, weathermsg,
                                   flag_string ( sector_name, to_room->sector_type ) );
                    break;
               } /* end of switch */
          }
          /* end of leaving city */
          /* leaving a field, forest, hills, or mountain. */
          if ( ( in_room->sector_type == SECT_FIELD )
               || ( in_room->sector_type == SECT_FOREST )
               || ( in_room->sector_type == SECT_HILLS )
               || ( in_room->sector_type == SECT_DESERT )
               || ( in_room->sector_type == SECT_MOUNTAIN ) )
          {
               switch ( to_room->sector_type )
               {
               case SECT_MAX:
                    break;		/* no message for this movement */
               case SECT_WATER_SWIM:
                    sound ("waterysnd.wav", ch);
                    send_to_char ("You splash into the water.\n\r", ch);
                    break;
               case SECT_WATER_NOSWIM:
                    sound ("wavecrash.wav", ch);
                    break;
               case SECT_FORT:
                    form_to_char ( ch, "You leave the %s %s and enter a fort.\n\r",
                                   weathermsg, flag_string ( sector_name, in_room->sector_type ) );
                    break;
               case SECT_INSIDE:
                    form_to_char ( ch, "You take shelter indoors from the %s %s.\n\r",
                                   weathermsg, flag_string ( sector_name, in_room->sector_type ) );
                    break;
               case SECT_UNDERGROUND:
                    send_to_char ( "You go underground.\n\r", ch );
                    break;
               case SECT_CITY:
                    form_to_char ( ch, "You leave the %s and enter a %s city.\n\r",
                                   flag_string ( sector_name, in_room->sector_type ), weathermsg );
                    break;
               default:
                    if ( in_room->sector_type == to_room->sector_type )
                         break;	/* no msg if no change */
                    form_to_char ( ch, "You head into the %s.\n\r",
                                   flag_string ( sector_name, to_room->sector_type ) );
                    break;
               }
               /* end switch */
          }
          /* end of leaving forest, field, hills, mountain, desert */
          /* leaving the air */
          if ( in_room->sector_type == SECT_AIR )
          {
               switch ( to_room->sector_type )
               {
               case SECT_AIR:
               case SECT_MAX:
                    break;		/* no msg */
               case SECT_WATER_SWIM:
                    sound ("waterysnd.wav", ch);
                    send_to_char ("You splash down into the water.\n\r", ch);
                    break;
               case SECT_WATER_NOSWIM:
                    sound ("wavecrash.wav", ch);
                    break;
               default:
                    form_to_char ( ch, "You land in the %s %s.\n\r",
                                   weathermsg,
                                   flag_string ( sector_name, to_room->sector_type ) );
                    break;
               }
               /* end switch */
          }

          /* end leaving the air */
          /* Zeran - time for encumbrance, no more 1 PULSE movement delay */
          /* Slow down movement in general, will curb speed walking
           * and make transportation magic seem a bit more important.
           * In general, unencumbered creature with nothing affecting
           * speed will have encumbrance of 1, thus making a 3 PULSE
           * movement delay.  Each pulse being appx .25 seconds */
          /* Loth - I made it 1 pulse + ch->encumbrance, since encumbrance is always
           * at least 1, this will always be 2. */
          WAIT_STATE ( ch, ( 1 + ch->encumbrance ) );
          ch->move -= move;
     }

     if ( !IS_AFFECTED ( ch, AFF_SNEAK ) && ( IS_NPC ( ch ) || !IS_SET ( ch->act, PLR_WIZINVIS ) ) )
     {
          if ( IS_AFFECTED ( ch, AFF_FLYING ) )
               act ( "$n flies $T.", ch, NULL, dir_name[door], TO_ROOM );
          else
               act ( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
     }
     
     char_from_room ( ch );
     char_to_room ( ch, to_room );
     if ( !IS_AFFECTED ( ch, AFF_SNEAK ) && ( IS_NPC ( ch ) || !IS_SET ( ch->act, PLR_WIZINVIS ) ) )
          act ( "$n has arrived.", ch, NULL, NULL, TO_ROOM );

     if ( IS_RENTED( to_room->lease ) )
     {
          if ( !str_cmp ( to_room->lease->rented_by, ch->name ) )
               send_to_char ( "You currently hold the lease on this room.\n\r", ch );
     }

     do_look ( ch, "auto" );

     if ( in_room == to_room )	/* no circular follows */
          return;

    /* do the recruit proc */

     for ( fch = in_room->people; fch != NULL; fch = fch_next )
     {
          fch_next = fch->next_in_room;

#if defined(DEBUGINFO)
          send_to_char ( "inside the recruit proc\n\r", ch );
#endif

          if ( IS_NPC ( fch ) && ( fch->level < ch->level )
               && ( !IS_NPC ( ch ) )
               && ( !IS_SET ( ch->act, PLR_NOFOLLOW ) )
               && ( !IS_SET ( fch->act, ACT_SENTINEL ) )
               && ( !IS_SET ( fch->act, ACT_AGGRESSIVE ) )
               && ( !IS_SET ( fch->act, ACT_PET ) )
               && ( !IS_SET ( fch->act, ACT_FOLLOWER ) )
               && ( !IS_SET ( fch->act, ACT_PRACTICE ) )
               && ( !IS_SET ( fch->act, ACT_NOPURGE ) )
               && ( !IS_SET ( fch->act, ACT_IS_HEALER ) )
               && ( !IS_AFFECTED ( fch, AFF_CHARM ) ) )
          {
               int                 pass;
               int                 chance;
               int                 skillchance;
               int                 diff;

               pass = 1;		/* base chance of passing is 1% */
               /* bonus if alignment is within 150 of char */
               diff = fch->alignment - ch->alignment;
               if ( ( diff <= 150 ) && ( diff >= -150 ) )
                    pass += 1;
               /* add charisma check here */

               if ( ( fch->alignment > 250 ) &&
                    ( ch->alignment < 250 ) )
                    pass = 0;
               skillchance = number_percent (  );
               chance = number_percent (  );
               if ( ( pass >= chance ) && ( skillchance < ch->pcdata->learned[gsn_recruit] ) )
               {			/* make new follower! */
                    SET_BIT ( fch->act, ACT_FOLLOWER );

                    SET_BIT ( fch->affected_by, AFF_CHARM );
                    fch->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
                    add_follower ( fch, ch );
                    act ( "$N has heard of your reputation and starts following you!", fch, NULL, NULL, TO_CHAR );
                    act ( "$N now follows $n!", fch, NULL, ch, TO_ROOM );
                    check_improve ( ch, gsn_recruit, TRUE, 1 );
               }               /* end of new follow */
          }          /* end of recruit routine */
     }     /* end of for loop */

     for ( fch = in_room->people; fch != NULL; fch = fch_next )
     {
          fch_next = fch->next_in_room;

          if ( fch->master == ch && IS_AFFECTED ( fch, AFF_CHARM )
               && fch->position < POS_STANDING )
               do_stand ( fch, "" );

          if ( fch->master == ch && fch->position == POS_STANDING )
          {

               if ( IS_SET ( ch->in_room->room_flags, ROOM_LAW )
                    && ( IS_NPC ( fch ) &&
                         IS_SET ( fch->act, ACT_AGGRESSIVE ) ) )
               {
                    act ( "You can't bring $N into the city.", ch, NULL, fch, TO_CHAR );
                    act ( "You aren't allowed in the city.", fch, NULL, NULL, TO_CHAR );
                    return;
               }
               act ( "You follow $N.", fch, NULL, ch, TO_CHAR );
               move_char ( fch, door, TRUE );
          }
     }

     if ( IS_NPC( ch ) && HAS_TRIGGER_MOB( ch, TRIG_ENTRY ) )
          p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );
     
     /*
      * If someone is following the char, these triggers get activated
      * for the followers before the char, but it's safer this way...
      */
     if ( !IS_NPC( ch ) )
     {
          p_greet_trigger( ch, PRG_MPROG );
          p_greet_trigger( ch, PRG_OPROG );
          p_greet_trigger( ch, PRG_RPROG );
     }
}

void do_confusedmove ( CHAR_DATA * ch )
{
     int d;
     d= number_range (0, 5);
     send_to_char ( "In your confused state you aren't sure which way is up....\n\r", ch);
     move_char ( ch, d, FALSE );
     return;
}

void do_north ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_NORTH, FALSE );
     return;
}

void do_east ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_EAST, FALSE );
     return;
}

void do_south ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_SOUTH, FALSE );
     return;
}

void do_west ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_WEST, FALSE );
     return;
}

void do_up ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_UP, FALSE );
     return;
}

void do_down ( CHAR_DATA * ch, char *argument )
{
     if (CAN_DETECT (ch, AFF_CONFUSION) )
          do_confusedmove(ch);
     else
          move_char ( ch, DIR_DOWN, FALSE );
     return;
}

int find_door ( CHAR_DATA * ch, char *arg )
{
     EXIT_DATA          *pexit;
     int                 door;

     if ( !str_cmp ( arg, "n" ) || !str_cmp ( arg, "north" ) )
          door = 0;
     else if ( !str_cmp ( arg, "e" ) || !str_cmp ( arg, "east" ) )
          door = 1;
     else if ( !str_cmp ( arg, "s" ) || !str_cmp ( arg, "south" ) )
          door = 2;
     else if ( !str_cmp ( arg, "w" ) || !str_cmp ( arg, "west" ) )
          door = 3;
     else if ( !str_cmp ( arg, "u" ) || !str_cmp ( arg, "up" ) )
          door = 4;
     else if ( !str_cmp ( arg, "d" ) || !str_cmp ( arg, "down" ) )
          door = 5;
     else
     {
          for ( door = 0; door <= 5; door++ )
          {
               if ( ( pexit = ch->in_room->exit[door] ) != NULL
                    && IS_SET ( pexit->exit_info, EX_ISDOOR )
                    && pexit->keyword != NULL
                    && is_name ( arg, pexit->keyword ) )
                    return door;
          }
          return -1;
     }

     if ( ( pexit = ch->in_room->exit[door] ) == NULL )
     {
          act ( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
          return -1;
     }
     if ( IS_SET ( pexit->exit_info, EX_HIDDEN ) )
     {
          // Don't send a msg as this would reveal a Hidden Door
          return -1;
     }
     
     if ( !IS_SET ( pexit->exit_info, EX_ISDOOR ) )
     {
          send_to_char ( "You can't do that.\n\r", ch );
          return -1;
     }

     return door;
}

void do_open ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 door = -1;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Open what?\n\r", ch );
          return;
     }

     if ( ( door = find_door ( ch, arg ) ) >= 0 )
     {
          /* 'open door' */
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit;
          EXIT_DATA          *pexit_rev;

          pexit = ch->in_room->exit[door];
          if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               send_to_char ( "It's already open.\n\r", ch );
               return;
          }
          if ( IS_SET ( pexit->exit_info, EX_LOCKED ) )
          {
               send_to_char ( "It's locked.\n\r", ch );
               return;
          }

          REMOVE_BIT ( pexit->exit_info, EX_CLOSED );
          act ( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
          sound ("odoor.wav", ch);
          send_to_char ( "Ok.\n\r", ch );

          /* open the other side */
          if ( ( to_room = pexit->u1.to_room ) != NULL
               && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL &&
               pexit_rev->u1.to_room == ch->in_room )
          {
               CHAR_DATA          *rch;
               REMOVE_BIT ( pexit_rev->exit_info, EX_CLOSED );
               for ( rch = to_room->people; rch != NULL;
                     rch = rch->next_in_room )
                    act ( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
          }
          return;
     }
     else if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          /* 'open object' */
          if ( obj->item_type != ITEM_CONTAINER )
          {
               send_to_char ( "That's not a container.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSED ) )
          {
               send_to_char ( "It's already open.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSEABLE ) )
          {
               send_to_char ( "You can't do that.\n\r", ch );
               return;
          }
          if ( IS_SET ( obj->value[1], CONT_LOCKED ) )
          {
               send_to_char ( "It's locked.\n\r", ch );
               return;
          }

          REMOVE_BIT ( obj->value[1], CONT_CLOSED );
          send_to_char ( "Ok.\n\r", ch );
          act ( "$n opens $p.", ch, obj, NULL, TO_ROOM );
          return;
     }
     else	/* doors give their own I see no msg */
          act ( "I see no $T here.", ch, NULL, arg, TO_CHAR );
     return;
}

void do_close ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 door;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Close what?\n\r", ch );
          return;
     }

     if ( ( door = find_door ( ch, arg ) ) >= 0 )
     {
          /* 'close door' */
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit;
          EXIT_DATA          *pexit_rev;

          pexit = ch->in_room->exit[door];
          if ( IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               send_to_char ( "It's already closed.\n\r", ch );
               return;
          }

          SET_BIT ( pexit->exit_info, EX_CLOSED );
          act ( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
          send_to_char ( "Ok.\n\r", ch );

          /* close the other side */
          if ( ( to_room = pexit->u1.to_room ) != NULL
               && ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
               && pexit_rev->u1.to_room == ch->in_room )
          {
               CHAR_DATA          *rch;

               SET_BIT ( pexit_rev->exit_info, EX_CLOSED );
               for ( rch = to_room->people; rch != NULL;
                     rch = rch->next_in_room )
                    act ( "The $d closes.", rch, NULL,
                          pexit_rev->keyword, TO_CHAR );
          }
          return;
     }

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          /* 'close object' */
          if ( obj->item_type != ITEM_CONTAINER )
          {
               send_to_char ( "That's not a container.\n\r", ch );
               return;
          }
          if ( IS_SET ( obj->value[1], CONT_CLOSED ) )
          {
               send_to_char ( "It's already closed.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSEABLE ) )
          {
               send_to_char ( "You can't do that.\n\r", ch );
               return;
          }

          SET_BIT ( obj->value[1], CONT_CLOSED );
          send_to_char ( "Ok.\n\r", ch );
          sound( "cdoor.wav", ch );
          act ( "$n closes $p.", ch, obj, NULL, TO_ROOM );
          return;
     }
     else	// Doors give their own error msg...
          act ( "I see no $T here.", ch, NULL, arg, TO_CHAR );
     return;
}

bool has_key ( CHAR_DATA * ch, int key )
{
     OBJ_DATA           *obj;

     for ( obj = ch->carrying; obj != NULL;
           obj = obj->next_content )
     {
          if ( obj->pIndexData->vnum == key )
               return TRUE;
     }
     return FALSE;
}

void do_lock ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 door;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Lock what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          /* 'lock object' */
          if ( obj->item_type != ITEM_CONTAINER )
          {
               send_to_char ( "That's not a container.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSED ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( obj->value[2] < 0 )
          {
               send_to_char ( "It can't be locked.\n\r", ch );
               return;
          }
          if ( !has_key ( ch, obj->value[2] ) )
          {
               send_to_char ( "You lack the key.\n\r", ch );
               return;
          }
          if ( IS_SET ( obj->value[1], CONT_LOCKED ) )
          {
               send_to_char ( "It's already locked.\n\r", ch );
               return;
          }

          SET_BIT ( obj->value[1], CONT_LOCKED );
          send_to_char ( "*Click*\n\r", ch );
          act ( "$n locks $p.", ch, obj, NULL, TO_ROOM );
          return;
     }

     if ( ( door = find_door ( ch, arg ) ) >= 0 )
     {
          /* 'lock door' */
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit;
          EXIT_DATA          *pexit_rev;

          pexit = ch->in_room->exit[door];
          if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( pexit->key < 0 )
          {
               send_to_char ( "It can't be locked.\n\r", ch );
               return;
          }
          if ( !has_key ( ch, pexit->key ) )
          {
               send_to_char ( "You lack the key.\n\r", ch );
               return;
          }
          if ( IS_SET ( pexit->exit_info, EX_LOCKED ) )
          {
               send_to_char ( "It's already locked.\n\r", ch );
               return;
          }

          SET_BIT ( pexit->exit_info, EX_LOCKED );
          send_to_char ( "*Click*\n\r", ch );
          act ( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

          /* lock the other side */
          if ( ( to_room = pexit->u1.to_room ) != NULL
               && ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
               && pexit_rev->u1.to_room == ch->in_room )
          {
               SET_BIT ( pexit_rev->exit_info, EX_LOCKED );
          }
     }

     return;
}

void do_unlock ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     OBJ_DATA           *obj;
     int                 door;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Unlock what?\n\r", ch );
          return;
     }

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          /* 'unlock object' */
          if ( obj->item_type != ITEM_CONTAINER )
          {
               send_to_char ( "That's not a container.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSED ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( obj->value[2] < 0 )
          {
               send_to_char ( "It can't be unlocked.\n\r", ch );
               return;
          }
          if ( !has_key ( ch, obj->value[2] ) )
          {
               send_to_char ( "You lack the key.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_LOCKED ) )
          {
               send_to_char ( "It's already unlocked.\n\r", ch );
               return;
          }

          REMOVE_BIT ( obj->value[1], CONT_LOCKED );
          sound ("unlock.wav", ch);
          send_to_char ( "*Click*\n\r", ch );
          act ( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
          return;
     }

     if ( ( door = find_door ( ch, arg ) ) >= 0 )
     {
          /* 'unlock door' */
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit;
          EXIT_DATA          *pexit_rev;

          pexit = ch->in_room->exit[door];
          if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( pexit->key < 0 )
          {
               if ( IS_SET (pexit->exit_info, EX_LOCKED) )
                    send_to_char ( "It can't be unlocked.\n\r", ch );
               else
                    send_to_char ( "It wasn't locked to begin with.\n\r", ch);
               return;
          }
          if ( !has_key ( ch, pexit->key ) )
          {
               send_to_char ( "You lack the key.\n\r", ch );
               return;
          }
          if ( !IS_SET ( pexit->exit_info, EX_LOCKED ) )
          {
               send_to_char ( "It's already unlocked.\n\r", ch );
               return;
          }

          REMOVE_BIT ( pexit->exit_info, EX_LOCKED );
          sound ("unlock.wav", ch);
          send_to_char ( "*Click*\n\r", ch );
          act ( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

          /* unlock the other side */
          if ( ( to_room = pexit->u1.to_room ) != NULL
               && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL &&
               pexit_rev->u1.to_room == ch->in_room )
          {
               REMOVE_BIT ( pexit_rev->exit_info, EX_LOCKED );
          }
     }
     return;
}

void do_pick ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *gch;
     OBJ_DATA           *obj;
     int                 door;

     one_argument ( argument, arg );

     if ( arg[0] == '\0' )
     {
          send_to_char ( "Pick what?\n\r", ch );
          return;
     }

     WAIT_STATE ( ch, skill_table[gsn_pick_lock].beats );

     /* look for guards */
     for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
     {
          if ( IS_NPC ( gch ) && IS_AWAKE ( gch ) && ch->level + 5 < gch->level )
          {
               act ( "$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR );
               return;
          }
     }

     if ( !IS_NPC ( ch ) && number_percent (  ) > ch->pcdata->learned[gsn_pick_lock] )
     {
          send_to_char ( "You failed.\n\r", ch );
          check_improve ( ch, gsn_pick_lock, FALSE, 2 );
          return;
     }

     if ( ( obj = get_obj_here ( ch, NULL, arg ) ) != NULL )
     {
          /* 'pick object' */
          if ( obj->item_type != ITEM_CONTAINER )
          {
               send_to_char ( "That's not a container.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_CLOSED ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( obj->value[2] < 0 )
          {
               send_to_char ( "It can't be unlocked.\n\r", ch );
               return;
          }
          if ( !IS_SET ( obj->value[1], CONT_LOCKED ) )
          {
               send_to_char ( "It's already unlocked.\n\r", ch );
               return;
          }
          if ( IS_SET ( obj->value[1], CONT_PICKPROOF ) )
          {
               send_to_char ( "You failed.\n\r", ch );
               return;
          }

          REMOVE_BIT ( obj->value[1], CONT_LOCKED );
          sound ("lockpick.wav", ch);
          send_to_char ( "*Click*\n\r", ch );
          check_improve ( ch, gsn_pick_lock, TRUE, 2 );
          act ( "$n picks $p.", ch, obj, NULL, TO_ROOM );
          return;
     }

     if ( ( door = find_door ( ch, arg ) ) >= 0 )
     {
          /* 'pick door' */
          ROOM_INDEX_DATA    *to_room;
          EXIT_DATA          *pexit;
          EXIT_DATA          *pexit_rev;

          pexit = ch->in_room->exit[door];
          if ( !IS_SET ( pexit->exit_info, EX_CLOSED ) &&
               !IS_IMMORTAL ( ch ) )
          {
               send_to_char ( "It's not closed.\n\r", ch );
               return;
          }
          if ( pexit->key < 0 && !IS_IMMORTAL ( ch ) )
          {
               send_to_char ( "It can't be picked.\n\r", ch );
               return;
          }
          if ( !IS_SET ( pexit->exit_info, EX_LOCKED ) )
          {
               send_to_char ( "It's already unlocked.\n\r", ch );
               return;
          }
          if ( IS_SET ( pexit->exit_info, EX_PICKPROOF ) &&
               !IS_IMMORTAL ( ch ) )
          {
               send_to_char ( "You failed.\n\r", ch );
               return;
          }

          REMOVE_BIT ( pexit->exit_info, EX_LOCKED );
          sound ("lockpick.wav", ch);
          send_to_char ( "*Click*\n\r", ch );
          act ( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
          check_improve ( ch, gsn_pick_lock, TRUE, 2 );

          /* pick the other side */
          if ( ( to_room = pexit->u1.to_room ) != NULL
               && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL &&
               pexit_rev->u1.to_room == ch->in_room )
          {
               REMOVE_BIT ( pexit_rev->exit_info, EX_LOCKED );
          }
     }
     return;
}

void do_stand ( CHAR_DATA * ch, char *argument )
{
     if ( IS_AFFECTED ( ch, AFF_SLEEP ) )
     {
          send_to_char ( "You are asleep and cannot wake up!\n\r", ch );
          return;
     }
     switch ( ch->position )
     {
     case POS_SLEEPING:
          send_to_char ( "You wake and stand up.\n\r", ch );
          act ( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_STANDING;
          ch->on = NULL;
          do_look ( ch, "auto" );
          break;
     case POS_RESTING:
     case POS_SITTING:
          send_to_char ( "You stand up.\n\r", ch );
          act ( "$n stands up.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_STANDING;
          ch->on = NULL;
          break;
     case POS_STANDING:
          send_to_char ( "You are already standing.\n\r", ch );
          break;
     case POS_FIGHTING:
          send_to_char ( "Maybe you should finish this fight first!\n\r", ch );
          break;
     }

     return;
}

void do_rest ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj = NULL;

     if ( IS_AFFECTED ( ch, AFF_SLEEP ) )	/* magical sleep. can't rest. */
     {
          send_to_char ( "You are asleep and cannot wake up!\n\r", ch );
          return;
     }
     if ( ch->position == POS_FIGHTING )
     {
          send_to_char ( "Maybe you should finish this fight first!\n\r", ch );
          return;
     }

     /* okay, now that we know we can rest, find an object to rest on */
     if ( argument[0] != '\0' )
     {
          obj = get_obj_list ( ch, argument, ch->in_room->contents );
          if ( obj == NULL )
          {
               send_to_char ( "You don't see that here.\n\r", ch );
               return;
          }
     }
     else
          obj = ch->on;
     if ( obj != NULL )
     {
          if ( !IS_SET ( obj->item_type, ITEM_FURNITURE )
               || ( !IS_SET ( obj->value[2], REST_ON )
                    && !IS_SET ( obj->value[2], REST_IN )
                    && !IS_SET ( obj->value[2], REST_AT ) ) )
          {
               send_to_char ( "You can't rest on that.\n\r", ch );
               return;
          }

          if ( obj != NULL && ch->on != obj && count_users ( obj ) >= obj->value[0] )
          {
               act_new ( "There's no more room on $p.", ch, obj, NULL, TO_CHAR, POS_DEAD );
               return;
          }
          ch->on = obj;
          if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
               p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
     }

     switch ( ch->position )
     {
     case POS_SLEEPING:
          send_to_char ( "You wake up and roll over to rest.\n\r",  ch );
          act ( "$n wakes up, then rolls over to rest.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_RESTING;
          do_look ( ch, "auto" );
          break;
     case POS_RESTING:
          send_to_char ( "You are already resting.\n\r", ch );
          break;
     case POS_STANDING:
          if ( obj == NULL )
          {
               send_to_char ( "You sit down and rest.\n\r", ch );
               act ( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_AT ) )
          {
               act ( "You sit down at $p and rest.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_ON ) )
          {
               act ( "You sit on $p and rest.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits on $p and rests.", ch, obj, NULL, TO_ROOM );
          }
          else
          {
               act ( "You rest in $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests in $p.", ch, obj, NULL, TO_ROOM );
          }
          ch->position = POS_RESTING;
          break;
     case POS_SITTING:
          if ( obj == NULL )
          {
               send_to_char ( "You rest.\n\r", ch );
               act ( "$n rests.", ch, NULL, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_AT ) )
          {
               act ( "You rest at $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests at $p.", ch, obj, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_ON ) )
          {
               act ( "You rest on $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests on $p.", ch, obj, NULL, TO_ROOM );
          }
          else
          {
               act ( "You rest in $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests in $p.", ch, obj, NULL, TO_ROOM );
          }
          ch->position = POS_RESTING;
          break;
     }
     return;
}

void do_sit ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj = NULL;

     if ( IS_AFFECTED ( ch, AFF_SLEEP ) )	/* magical sleep. can't rest. */
     {
          send_to_char ( "You are asleep and cannot wake up!\n\r", ch );
          return;
     }
     if ( ch->position == POS_FIGHTING )
     {
          send_to_char ( "Maybe you should finish this fight first!\n\r", ch );
          return;
     }

     /* okay, now that we know we can sit, find an object to sit on */
     if ( argument[0] != '\0' )
     {
          obj = get_obj_list ( ch, argument, ch->in_room->contents );
          if ( obj == NULL )
          {
               send_to_char ( "You don't see that here.\n\r", ch );
               return;
          }
     }
     else
          obj = ch->on;

     if ( obj != NULL )
     {
          if ( !IS_SET ( obj->item_type, ITEM_FURNITURE )
               || ( !IS_SET ( obj->value[2], SIT_ON )
                    && !IS_SET ( obj->value[2], SIT_IN )
                    && !IS_SET ( obj->value[2], SIT_AT ) ) )
          {
               send_to_char ( "You can't sit on that.\n\r", ch );
               return;
          }

          if ( obj != NULL && ch->on != obj &&  count_users ( obj ) >= obj->value[0] )
          {
               act_new ( "There's no more room on $p.", ch, obj, NULL,  TO_CHAR, POS_DEAD );
               return;
          }
          ch->on = obj;
          if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
          {
               p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
          }
     }

     switch ( ch->position )
     {
     case POS_SLEEPING:
          send_to_char ( "You wake and sit up.\n\r", ch );
          act ( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
          ch->position = POS_SITTING;
          do_look ( ch, "auto" );
          break;
     case POS_SITTING:
          send_to_char ( "You are already sitting.\n\r", ch );
          break;
     case POS_STANDING:
          if ( obj == NULL )
          {
               send_to_char ( "You sit down.\n\r", ch );
               act ( "$n sits down.", ch, NULL, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], SIT_AT ) )
          {
               act ( "You sit down at $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits down at $p.", ch, obj, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], SIT_ON ) )
          {
               act ( "You sit on $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits on $p.", ch, obj, NULL, TO_ROOM );
          }
          else
          {
               act ( "You sit in $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits in $p.", ch, obj, NULL, TO_ROOM );
          }
          ch->position = POS_SITTING;
          break;
     case POS_RESTING:
          if ( obj == NULL )
          {
               send_to_char ( "You sit up.\n\r", ch );
               act ( "$n sits up.", ch, NULL, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_AT ) )
          {
               act ( "You sit up at $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n sits up at $p.", ch, obj, NULL, TO_ROOM );
          }
          else if ( IS_SET ( obj->value[2], REST_ON ) )
          {
               act ( "You rest on $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests on $p.", ch, obj, NULL, TO_ROOM );
          }
          else
          {
               act ( "You rest in $p.", ch, obj, NULL, TO_CHAR );
               act ( "$n rests in $p.", ch, obj, NULL, TO_ROOM );
          }
          ch->position = POS_RESTING;
          break;
     }

     return;
}

void do_sleep ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *obj = NULL;

     switch ( ch->position )
     {
     case POS_MORTAL:
          send_to_char ( "You give up on life, and go to sleep forever.\n\r", ch );
          break;
     case POS_INCAP:
     case POS_STUNNED:
     case POS_DEAD:
          send_to_char ( "You are hurt far too badly for that.\n\r", ch );
          break;
     case POS_SLEEPING:
          send_to_char ( "You are already sleeping.\n\r", ch );
          break;
     case POS_RESTING:
     case POS_SITTING:
     case POS_STANDING:
          if ( argument[0] == '\0' && ch->on == NULL )
          {
               send_to_char ( "You go to sleep.\n\r", ch );
               act ( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
               ch->position = POS_SLEEPING;
          }
          else			/* find an object and sleep on it */
          {
               if ( argument[0] == '\0' )
                    obj = ch->on;
               else
                    obj = get_obj_list ( ch, argument, ch->in_room->contents );
               if ( obj == NULL )
               {
                    send_to_char ( "You don't see that here.\n\r", ch );
                    return;
               }
               if ( obj->item_type != ITEM_FURNITURE
                    || ( !IS_SET ( obj->value[2], SLEEP_ON )
                         && !IS_SET ( obj->value[2], SLEEP_IN )
                         && !IS_SET ( obj->value[2], SLEEP_AT ) ) )
               {
                    send_to_char ( "You can't sleep on that!\n\r", ch );
                    return;
               }

               if ( ch->on != obj && count_users ( obj ) >= obj->value[0] )
               {
                    act_new ( "There is no room on $p for you.", ch, obj, NULL, TO_CHAR, POS_DEAD );
                    return;
               }

               ch->on = obj;
               if ( HAS_TRIGGER_OBJ( obj, TRIG_SIT ) )
                    p_percent_trigger( NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT );
               if ( IS_SET ( obj->value[2], SLEEP_AT ) )
               {
                    act ( "You go to sleep at $p.", ch, obj, NULL, TO_CHAR );
                    act ( "$n goes to sleep at $p.", ch, obj, NULL, TO_ROOM );
               }
               else if ( IS_SET ( obj->value[2], SLEEP_ON ) )
               {
                    act ( "You go to sleep on $p.", ch, obj, NULL,  TO_CHAR );
                    act ( "$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM );
               }
               else
               {
                    act ( "You go to sleep in $p.", ch, obj, NULL, TO_CHAR );
                    act ( "$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM );
               }
               ch->position = POS_SLEEPING;
          }
          break;
     case POS_FIGHTING:
          send_to_char ( "Yeah right! You'd get slaughtered!\n\r",  ch );
          break;
     }

     return;
}

void do_wake ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          do_stand ( ch, argument );
          return;
     }

     if ( !IS_AWAKE ( ch ) )
     {
          send_to_char ( "You are asleep yourself!\n\r", ch );
          return;
     }

     if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
          return;
     }

     // Chars who are AWAKE by the macro's definition may still be unwakeable, and
     // will now get a better error msg. Someone who is mortally wounded isn't "already awake".
     
     if ( victim->position < POS_SLEEPING )
     {
          act ( "$N can't be woken up.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( IS_AWAKE ( victim ) )
     {
          act ( "$N is already awake.", ch, NULL, victim, TO_CHAR );
          return;
     }

     if ( IS_AFFECTED ( victim, AFF_SLEEP ) )
     {
          act ( "You can't wake $M!", ch, NULL, victim, TO_CHAR );
          return;
     }

     victim->position = POS_STANDING;
     victim->on = NULL;
     act ( "You wake $M.", ch, NULL, victim, TO_CHAR );
     act ( "$n wakes you.", ch, NULL, victim, TO_VICT );
     do_look ( ch, "auto" );
     return;
}

void do_sneak ( CHAR_DATA * ch, char *argument )
{
     AFFECT_DATA         af;

     send_to_char ( "You attempt to move silently.\n\r", ch );
     affect_strip ( ch, gsn_sneak );

     if ( IS_NPC ( ch ) || number_percent (  ) < ch->pcdata->learned[gsn_sneak] )
     {
          check_improve ( ch, gsn_sneak, TRUE, 3 );
          af.where = TO_AFFECTS;
          af.type = gsn_sneak;
          af.level = ch->level;
          af.duration = ch->level;
          af.location = APPLY_NONE;
          af.modifier = 0;
          af.bitvector = AFF_SNEAK;
          affect_to_char ( ch, &af );
     }
     else
          check_improve ( ch, gsn_sneak, FALSE, 3 );
     return;
}

void do_hide ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( "You attempt to hide.\n\r", ch );

     if ( IS_AFFECTED ( ch, AFF_HIDE ) )
          REMOVE_BIT ( ch->affected_by, AFF_HIDE );

     if ( IS_NPC ( ch ) || number_percent (  ) < ch->pcdata->learned[gsn_hide] )
     {
          SET_BIT ( ch->affected_by, AFF_HIDE );
          check_improve ( ch, gsn_hide, TRUE, 3 );
     }
     else
          check_improve ( ch, gsn_hide, FALSE, 3 );

     return;
}

/*
 * Contributed by Alander.
 */

void do_visible ( CHAR_DATA * ch, char *argument )
{
     affect_strip ( ch, gsn_invis );
     affect_strip ( ch, gsn_mass_invis );
     affect_strip ( ch, gsn_sneak );
     REMOVE_BIT ( ch->affected_by, AFF_HIDE );
     REMOVE_BIT ( ch->affected_by, AFF_INVISIBLE );
     REMOVE_BIT ( ch->affected_by, AFF_SNEAK );
     if ( IS_AFFECTED ( ch, AFF_POLY ) )
     {
          affect_strip ( ch, skill_lookup ( "mask self" ) );
          undo_mask ( ch );
          REMOVE_BIT ( ch->affected_by, AFF_POLY );
     }
     send_to_char ( "Ok.\n\r", ch );
     return;
}

void do_recall ( CHAR_DATA * ch, char *argument )
{
     CHAR_DATA          *victim;
     ROOM_INDEX_DATA    *location;

     if ( IS_NPC ( ch ) && ( !IS_SET ( ch->act, ACT_PET ) || !IS_SET ( ch->act, ACT_FOLLOWER ) ) )
     {
          send_to_char ( "Only players can recall.\n\r", ch );
          return;
     }

     act ( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

     if ( ( location = get_room_index ( ch->recall_perm ) ) == NULL )
     {
          send_to_char ( "You are completely lost.\n\r", ch );
          return;
     }

     if ( ch->in_room == location )
          return;

     if ( IS_SET ( ch->in_room->room_flags, ROOM_NO_RECALL )
          || IS_AFFECTED ( ch, AFF_CURSE ) )
     {
          send_to_char ( "Zeran has forsaken you.\n\r", ch );
          return;
     }

     if ( IS_AFFECTED ( ch, AFF_MELD ) )
     {
          send_to_char ( "You cannot concentrate enough to pray.\n\r", ch );
          return;
     }

     if ( ( victim = ch->fighting ) != NULL )
     {
          int                 lose, skill;

          if ( IS_NPC ( ch ) ) skill = 40 + ch->level;
          else
               skill = ch->pcdata->learned[gsn_recall];

          if ( number_percent (  ) < 80 * skill / 100 )
          {
               check_improve ( ch, gsn_recall, FALSE, 6 );
               WAIT_STATE ( ch, 4 );
               form_to_char ( ch, "You failed!.\n\r" );
               return;
          }
          lose = ( ch->desc != NULL ) ? 25 : 50;
          if (!IS_NPC(ch) && !ch->pcdata->mortal)
               lose = lose*3;
          gain_exp ( ch, 0 - lose );
          check_improve ( ch, gsn_recall, TRUE, 4 );
          form_to_char ( ch, "You recall from combat!  You lose %d exps.\n\r", lose );
          stop_fighting ( ch, TRUE );
     }

     ch->move /= 2;
     act ( "$n disappears.", ch, NULL, NULL, TO_ROOM );
     char_from_room ( ch );
     char_to_room ( ch, location );
     act ( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
     do_look ( ch, "auto" );

     if ( ch->pet != NULL )
          do_recall ( ch->pet, "" );

     return;
}

void do_enter ( CHAR_DATA * ch, char *argument )
{
     OBJ_DATA           *portal;
     OBJ_DATA           *tmp = NULL;
     ROOM_INDEX_DATA    *dest;
     char                arg1[MAX_INPUT_LENGTH];
     int                 room;
     bool                pet = FALSE;
     bool                link_obj = FALSE;
     bool	         held = FALSE;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Enter what?\n\r", ch );
          return;
     }

     /* check fighting */
     if ( ch->fighting != NULL )
     {
          send_to_char ( "Not while you're fighting!\n\r", ch );
          return;
     }
     /* get portal name */
     one_argument ( argument, arg1 );

     /* Check held portals first */
     /* Only hit the first item of this keyword */
     /* Need to make this work better later */

     if ( ( ( portal = get_obj_wear (ch, arg1, TRUE ) ) != NULL ) && portal->item_type == ITEM_PORTAL )
     {
          if ( portal->wear_loc == WEAR_HOLD || portal->wear_loc == WEAR_FLOAT
               || portal->wear_loc == WEAR_NECK_1 || portal->wear_loc == WEAR_NECK_2 )
          {
               held = TRUE;
          }
          else
          {              
               bugf( "do_enter: Portal %d invalid wear_loc!", portal->pIndexData->vnum );
               send_to_char("Odd that. It doesn't seem to work right.\n\r", ch);
               return;
          }
     }
     else if ( ( portal = get_obj_here ( ch, NULL, arg1 ) ) == NULL )
     {
          form_to_char ( ch, "You see no %s here.\n\r", arg1 );
          return;
     }

     if ( !can_see_obj( ch, portal ) )
     {
          form_to_char ( ch, "You see no %s here.\n\r", arg1 );
          return;
     }
     if ( portal->carried_by && portal->carried_by != ch )
     {
          form_to_char ( ch, "You see no %s here.\n\r", arg1);
          return;
     }
     // If entry trigger, then we don't check for portal type, nor do
     // we continue processing the enter command.
     // This means the rest of the sanity checking should be done by the
     // mobprog coder. - Lotherius
     if ( !IS_NPC( ch ) && HAS_TRIGGER_OBJ( portal, TRIG_ENTER ) )
     {
          p_percent_trigger( NULL, portal, NULL, ch, NULL, NULL, TRIG_ENTER );
          return;
     }          
     if ( portal->item_type != ITEM_PORTAL )
     {
          form_to_char ( ch, "You can't enter the %s.\n\r",  portal->short_descr );
          return;
     }
     if ( !held && portal->carried_by != NULL )
     {
          form_to_char ( ch, "You have to either wear %s or put it on the ground first.\n\r", portal->short_descr );
          return;
     }

     /* check level of portal vs level of character */
     if ( get_trust ( ch ) < portal->level )
     {
          if (!held)
               form_to_char ( ch, "You step into %s, but it throws you back out!\n\r", 
                              portal->short_descr );
          else
               form_to_char ( ch, "You grasp %s and try to activate it, but the magic doesn't work!\n\r", 
                              portal->short_descr );
          return;
     }

     /* validate portal destination */
     room = portal->value[0];

     if ( ( dest = get_room_index ( room ) ) == NULL )
     {
          bugf ( "Bad portal room vnum %d", room );
          send_to_char ( "Bad portal, please inform the IMMs.\n\r", ch );
          return;
     }

     if (!can_see_room(ch, dest) )
     {
          send_to_char ("You try to enter the portal, but nothing happens.\n\r", ch);
          return;
     }

     /* Running with assumptions is bad news -- Lotherius */
     /* Running with assumption that either the builder made the portal
      * sensibly, or else created via the spell, in which case, its
      * rules should have handled things like private rooms and such.
      * Shouldn't need to check those cases here.
      */

     /* determination if link object exists in destination room */
     if ( portal->value[1] == 0 )
          link_obj = FALSE;
     else if ( dest->contents == NULL )
          link_obj = FALSE;
     else
     {
          for ( tmp = dest->contents; tmp != NULL; tmp = tmp->next_content )
          {
               if ( tmp->pIndexData->vnum == portal->value[1] )
               {
                    link_obj = TRUE;
                    break;
               }
          }
     }
     /* Can't follow through a held portal */
     if (!held)
     {
          if ( ch->pet != NULL && ch->in_room == ch->pet->in_room )
               pet = TRUE;
     }

     send_to_char ( "Time and space unravel before your eyes!\n\r", ch );
     act ( "$n steps into $P and {cvanishes{x!", ch, NULL, portal, TO_ROOM );
     char_from_room ( ch );
     char_to_room ( ch, dest );
     if ( link_obj )
     {
          act ( "An ear piercing whine fills the room....", ch, NULL, NULL, TO_ROOM );
          act ( "$n steps out of $P!", ch, NULL, tmp, TO_ROOM );
          act ( "You step out of $P into $t!", ch, dest->name, tmp, TO_CHAR );
     }
     else
     {
          act ( "An ear piercing whine fills the room....", ch, NULL, NULL, TO_ROOM );
          act ( "A rip in time and space appears and $n steps forth from it!", ch, NULL, NULL, TO_ROOM );
          act ( "You step out of a rip in time and space into $t!", ch, dest->name, tmp, TO_CHAR );
     }
     do_look ( ch, "auto" );

     if ( pet )
     {
          act ( "$n steps into $P and {cvanishes{x!", ch->pet, NULL, portal, TO_ROOM );
          send_to_char ( "Time and space unravel before your eyes!\n\r", ch->pet );
          char_from_room ( ch->pet );
          char_to_room ( ch->pet, dest );
          if ( link_obj )
          {
               act ( "$n steps out of $P!", ch->pet, NULL, tmp, TO_ROOM );
               act ( "You step out of $P into $t!", ch->pet, dest->name, tmp, TO_CHAR );
          }
          else
          {
               act ( "$n steps forth from the rip.", ch->pet, NULL, NULL, TO_ROOM );
               act ( "You step out of a rip in time and space into another room!", ch->pet, NULL, tmp, TO_CHAR );
          }
          do_look ( ch->pet, "auto" );
     }

     if ( IS_NPC( ch ) && HAS_TRIGGER_MOB( ch, TRIG_ENTRY ) )
          p_percent_trigger( ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY );
     
     /*
      * If someone is following the char, these triggers get activated
      * for the followers before the char, but it's safer this way...
      */

     if ( !IS_NPC( ch ) )
     {
          p_greet_trigger( ch, PRG_MPROG );
          p_greet_trigger( ch, PRG_OPROG );
          p_greet_trigger( ch, PRG_RPROG );
     }
     return;
}

int total_encumbrance ( CHAR_DATA * ch )
{
     int                 total = ch->encumbrance;
     float               cur_weight;

     cur_weight = ( float ) ( ( float ) ch->carry_weight / ( float ) can_carry_w ( ch ) );
     if ( cur_weight > .9 )
          total += 3;
     else if ( cur_weight > .75 )
          total += 2;
     else if ( cur_weight > .5 )
          total += 1;
     return total;
}
