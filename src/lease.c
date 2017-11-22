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

/* routines related to leasing. Entire file 100% unique coding by Lotherius (aelfwyne@operamail.com) */
/* Before you do much here, peruse merc.h for IS_LEASE and IS_RENTED and see how they work           */

#include "everything.h"

/* local functions and vars */

void                save_leases (  );
struct lease_data   *lease_list;

void do_lease ( CHAR_DATA * ch, char *argument )
{
     LEASE              *lease;

	 /* is room rentable at all? */

     if ( IS_LEASE(ch->in_room->lease) )
     {
          lease = ch->in_room->lease;
          if ( argument[0] == '\0' )
          {
               if ( IS_RENTED(lease) )
               {
                    if ( lease->rented_by != ch->name )	/* if room is rented by ch */
                    {
                         send_to_char ( "This room is not currently for rent.\n\r", ch );
                         return;
                    }
                    else
                    {
                         form_to_char ( ch, "{CYour rent is paid until the month of %s of Year %d.{x\n\r",
                                   month_name[lease->paid_month], lease->paid_year );
                         return;
                    }
               }

               form_to_char ( ch, "{CThe monthly rent on this room is ${Y%d{x.\n\r", lease->room_rent );
               return;
          }
          else
          {
               int                 timeleft;	/* time left on lease */
               int                 months;

               if ( !is_number ( argument ) )
               {
                    send_to_char ( "{RInvalid argument. Syntax: lease <number>.{x\n\r", ch );
                    return;
               }

               months = atoi ( argument );

               if ( months < 1 )	/* trying to unrent? */
               {
                    send_to_char ( "{MYou must rent for at least 1 full month.{x\n\r", ch );
                    return;
               }
			   /* end of unrent */
               if ( months > 17 )	/* can't rent for longer than 1 mud year */
               {
                    send_to_char ( "{MYou can only rent for a maximum of 1 year.{x\n\r", ch );
                    return;
               }
			   /* end of too long rent */
               timeleft = 0;

               if ( lease->paid_year < time_info.year )
               {
                    timeleft = 0;
               }
               else if ( lease->paid_year == time_info.year )
               {
                    timeleft = lease->paid_month - time_info.month;
               }
               else if ( lease->paid_year > time_info.year )
               {
                    timeleft = lease->paid_month - time_info.month;
                    timeleft += 17;
               }

               if ( timeleft >= 17 )
               {
                    send_to_char ( "{YYou are currently paid for a full year.{x\n\r", ch );
                    return;
               }

               if ( ( timeleft + months ) > 17 )
               {
                    send_to_char ( "{MYou can't rent past 1 year. Try fewer months.{x\n\r", ch );
                    return;
               }
			   /* now all that's out of the way, we have an almost valid number of months to rent. */

               if ( ( months * lease->room_rent ) > ch->gold )
               {
                    send_to_char ( "{RYou don't have enough gold.{x\n\r", ch );
                    /* 		if ( (ch->gold / (months * ch->in_room->room_rent) ) >= 1 )
                     *              SNP(buf,"You have enough gold for {Y%d{x months.\n\r",
                     *                           (ch->gold / (months * ch->in_room->room_rent)) ); 
                     */
                    return;
               }

               ch->gold -= months * lease->room_rent;
               lease->paid_month = time_info.month;
               lease->paid_year = time_info.year;

               if ( ( timeleft + months ) > 17 )
               {
                    ++lease->paid_year;
               }
               else
               {
                    if ( ( time_info.month + ( months + timeleft ) ) > 17 )
                    {
                         ++lease->paid_year;
                         lease->paid_month += ( ( months + timeleft ) - 17 );
                    }
                    else
                    {
                         lease->paid_month += ( months + timeleft );
                    }
               }
               form_to_char ( ch, "{CYou have now rented this room for ${Y%d{C gold coins for a total of %d months.\n\r{x",
                         months * lease->room_rent, months );
               form_to_char ( ch, "{CThe rent is NOW paid until the month of %s of Year %d.{x\n\r",
                         month_name[lease->paid_month], lease->paid_year );
               act ( "$n has signed a lease on this room.", ch, NULL, NULL, TO_ROOM );
               free_string ( lease->rented_by );
               lease->rented_by = str_dup ( ch->name );
               save_leases (  );
               return;
          }
     }
     send_to_char ( "{xThis room cannot be rented.\n\r", ch );
     return;
}
/* end of do_lease */

void do_checklease ( CHAR_DATA * ch, char *argument )	/* immortal command to show lease info */
{
     char                buf2[MIL];
     LEASE              *lease;

     if ( IS_LEASE(ch->in_room->lease) )	/* if rentable */
     {
          lease = ch->in_room->lease;
          if ( IS_RENTED(lease) )
          {
               if ( lease->clan )
                    SNP ( buf2, "The Clan: %s", lease->clan->clan_name );
               else
                    SNP ( buf2, lease->rented_by );
               form_to_char ( ch, "{GRented By:     {W%s\n\r",  buf2 );
               form_to_char ( ch, "{GMonthly Rent:  {W%d\n\r",  lease->room_rent );
               form_to_char ( ch, "{GLease Expires: {WMonth of %s of Year %d.{x\n\r",
                         month_name[lease->paid_month],
                         lease->paid_year );               
               return;
          }
          else			/* not currently being rented */
          {
               send_to_char ( "{GNot currently being rented.\n\r", ch );
               form_to_char ( ch, "Monthly Rent:  {W%d{x\n\r",	lease->room_rent );
               return;
          }
     }
     send_to_char ( "{GThis is a non-rentable room.{x\n\r", ch );
     return;
}

void do_myleases ( CHAR_DATA *ch, char *argument )
{
     BUFFER *buffer;
     bool    match = FALSE;
     bool    showall = FALSE;
     LEASE  *lease;

     buffer = buffer_new (512);

     bprintf ( buffer, "You have the following leases in the lands:\n\r" );

     if ( !str_cmp ( argument, "all" ) )
          showall = TRUE;

     for ( lease = lease_list; lease != NULL ; lease = lease->next )
     {
          if ( !IS_RENTED(lease) )
          {
               if (showall)
               {
                    bprintf ( buffer, "{G%d{w:: unrented.\n\r", lease->room->vnum );
                    match = TRUE;
               }
               continue;
          }
          if ( !str_cmp (ch->name, lease->rented_by) || (showall == TRUE) )
          {
               if (showall)
                    bprintf ( buffer, "{G%d{w::{W%s\n\r", lease->room->vnum, lease->rented_by );
               bprintf ( buffer, "{C%s ",
                         !IS_NULLSTR ( lease->lease_name ) ? lease->lease_name : lease->room->name );
               bprintf ( buffer, "at ${Y%ld {Cmonthly, paid to the Month of {W%s{C, %d.{x\n\r",
                         lease->room_rent, month_name[lease->paid_month],
                         lease->paid_year );
               match = TRUE;
          }
     }
     if ( !match )
          bprintf ( buffer, "None, you've been sleeping on the ground, poor thing.\n\r" );
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
}

void save_leases (  )
{
     char                buf[MAX_STRING_LENGTH];
     FILE               *fp = NULL;
     LEASE              *lease = NULL;
     bool                clanflag = FALSE;

     SNP ( buf, "%sLease.DAT", DATA_DIR );

     fp = fopen ( buf, "w" );

     if ( !fp )
     {
          bugf ( "Could not open Lease.DAT for writing!" );
          return;
     }

     for ( lease = lease_list; lease != NULL ; lease = lease->next )
     {
          clanflag = FALSE;
          if ( IS_LEASE(lease) )
          {
               if ( lease->clan )
                    clanflag = TRUE;
               fprintf ( fp, "L %d %d %d %d %d %d %d %d %d %s~\n",
                         lease->room->vnum,
                         lease->room_rent,
                         lease->paid_month,
                         lease->paid_day,
                         lease->paid_year,
                         lease->owner_only,
                         lease->shop_type,
                         lease->shop_gold,
                         clanflag,
                         IS_RENTED ( lease ) ? lease->rented_by : "" );
               fprintf ( fp, "%s~\n",
                         !IS_NULLSTR( lease->lease_name) ? lease->lease_name : "" );
               fprintf ( fp, "%s~\n",
                         !IS_NULLSTR( lease->lease_name) ? lease->lease_descr : "" );

          }
          /* end of is rentable */
          else
               log_string ("Bad Lease.");
     }
     /* end of for loop */
     fprintf ( fp, "END\n" );
     fclose ( fp );

     return;
}

void load_leases (  )
{
     FILE               *fp = NULL;
     char                buf[MAX_STRING_LENGTH];
     char               *word;
     bool                fMatch;
     bool                clanflag;
     bool                done = FALSE;
     int                 iHash;
     ROOM_INDEX_DATA    *pRoomIndex;
     LEASE              *lease = NULL;

     SNP ( buf, "%sLease.DAT", DATA_DIR );

     fp = fopen ( buf, "r" );

     if ( !fp )
     {
          bugf ( "Could not open Lease.DAT for Reading!" );
          return;
     }

     fMatch = FALSE;

     while ( !done )
     {
          word = fread_word ( fp );
          if ( !str_cmp ( word, "L" ) )
          {
               pRoomIndex = get_room_index ( fread_number ( fp ) );

               lease = new_lease ( );
               lease->room_rent = fread_number ( fp );
               lease->paid_month = fread_number ( fp );
               lease->paid_day = fread_number ( fp );
               lease->paid_year = fread_number ( fp );
               lease->owner_only = fread_number ( fp );
               lease->shop_type = fread_number ( fp );
               lease->shop_gold = fread_number ( fp );
               clanflag = fread_number ( fp );
               lease->rented_by = fread_string ( fp );
               lease->lease_name = fread_string ( fp );
               lease->lease_descr = fread_string ( fp );
               pRoomIndex->lease = lease;
               lease->room = pRoomIndex;
               if (clanflag == TRUE)
               {
                    lease->clan = clan_by_short ( lease->rented_by );
                    if ( !lease->clan )
                    {
                         bugf ("Invalid clan found in leasefile. Clearing lease.\n\r" );
                         free_string ( lease->rented_by );
                         free_string ( lease->lease_name );
                         free_string ( lease->lease_descr );
                         lease->rented_by = NULL;
                         lease->lease_name = str_dup ( "" );
                         lease->lease_descr = str_dup ( "" );
                    }
               }
               VALIDATE (lease);
          }
          else if ( !str_cmp ( word, "END" ) )
          {
               done = TRUE;
               break;
          }
          else
          {
               bugf ( "Invalid data found in leasefile." );
               break;
          }
     }
     /* end of while !done */

     fclose ( fp );

	 /* Now we need to find those rooms that have ROOM_RENT but didn't get a lease loaded */

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
          {
               if ( IS_SET ( pRoomIndex->room_flags, ROOM_RENT ) )
               {
                    if ( IS_LEASE(pRoomIndex->lease) )
                         continue;
                    else
                    {
                         bugf ( "We got some ROOM_RENT rooms that didn't have entries in the Lease File!" );
                         lease = new_lease ( );
                         pRoomIndex->lease = lease;
                         lease->room = pRoomIndex;
                         VALIDATE ( lease );
                    }
               }
          }
     }

     return;
}

void do_setrent ( CHAR_DATA * ch, char *argument )
{
     LEASE              *lease;

     if ( IS_LEASE(ch->in_room->lease) )
     {
          lease = ch->in_room->lease;
          if ( ( argument[0] != '\0' ) && ( ( atoi ( argument ) >= 15000 )
                                            || ( ch->in_room->sector_type == SECT_FORT ) ) )
          {
               form_to_char ( ch, "\n\rRent set to %d.\n\r", atoi ( argument ) );
               lease->room_rent = atoi ( argument );
               save_leases (  );
          }
          else
               send_to_char ( "You must specify a rent amount, and it must be AT LEAST 15000 coins.\n\r", ch );
     }
     else
          send_to_char ( "This room is not rentable.\n\r", ch );
     return;
}

void do_roomdesc ( CHAR_DATA * ch, char *argument )
{
     LEASE  *lease;

     if ( IS_NPC ( ch ) )
          return;

     if ( !IS_LEASE(ch->in_room->lease) )
     {
          send_to_char ( "This room cannot be rented.\n\r", ch );
          return;
     }

     lease = ch->in_room->lease;

     if ( !IS_RENTED(lease) )
     {
          send_to_char ( "Try leasing the room first.\n\r", ch);
          return;
     }

     if ( str_cmp ( ch->name, lease->rented_by ) )
     {
          send_to_char ( "Nice Try. This ain't your room.\n\r", ch );
          return;
     }
     
     ch->pcdata->mode = MODE_LEASEDESC;
     string_append ( ch, &lease->lease_descr );
     // We would run a save here... but it's pretty pointless. I guess we'll just hope
     // that leases get saved before something bad happens.
}

void do_roomname ( CHAR_DATA * ch, char *argument )
{
     LEASE   *lease;
     if ( IS_NPC ( ch ) )
          return;
     
     if ( !IS_LEASE(ch->in_room->lease) )
     {
          send_to_char ( "This room cannot be rented.\n\r", ch );
          return;
     }

     lease = ch->in_room->lease;

     if ( !IS_RENTED(lease) )
     {
          send_to_char ( "Try leasing the room first.\n\r", ch);
          return;
     }

     if ( str_cmp ( lease->rented_by, ch->name ) )
     {
          send_to_char ( "Nice Try. This ain't your room.\n\r", ch );
          return;
     }

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Set the room's name to what?\n\r", ch );
          return;
     }

     if ( cstrlen ( argument ) > 45 )
     {
          send_to_char ("Room name must not exceed 45 characters.\n\r", ch );
          return;
     }

     /* Also smash < > codes... cheap hack to prevent MXP codes in user-entered roomnames. */
     for ( ; *argument != '\0'; argument++ )
     {
          if ( *argument == '<' )
               *argument = '(';
          if ( *argument == '>' )
               *argument = ')';
     }     
      
     free_string ( lease->lease_name );
     
     lease->lease_name = str_dup ( argument );
     send_to_char ( "Room Name Set.\n\r", ch );
     save_leases (  );
}

void do_private ( CHAR_DATA * ch, char *argument )
{
     LEASE    *lease;

     if ( IS_NPC ( ch ) )
          return;

     if ( !IS_LEASE(ch->in_room->lease) )
     {
          send_to_char ( "You can't do that here!\n\r", ch );
          return;
     }

     lease = ch->in_room->lease;

     if ( !IS_RENTED(lease) )
     {
          send_to_char ( "Try leasing the room first.\n\r", ch);
          return;
     }

     if  ( lease->rented_by == ch->name )
     {
          if ( lease->owner_only == TRUE )
          {
               lease->owner_only = FALSE;
               send_to_char ( "Others may now enter your domain.\n\r", ch );
          }
          else
          {
               lease->owner_only = TRUE;
               send_to_char ( "You lock the door on your domain.\n\r", ch );
          }
          save_leases (  );	/* be sure to save the leases */
     }
     else
          send_to_char ( "This room does not belong to you!\n\r", ch );
     return;
}
