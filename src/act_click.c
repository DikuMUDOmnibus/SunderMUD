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

/*************************************
 * Supports functions from Click Menus
 * 
 * Used in MXP and Pueblo support.
 *************************************/

/*
 * Gives a brief menu of context clickable links about the targeted char
 */

void click_context_char ( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *victim;
     char cmdbuf[MSL];

     if ( ( victim = get_char_room ( ch, NULL, argument ) ) != NULL )
     {
          form_to_char ( ch, "\n\r[%s] Options: ", victim->short_descr );
          SNP ( cmdbuf, "look %s", victim->name );
          form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Look At) ", cmdbuf, "Look more closely" ) );
          if ( IS_NPC ( victim ) ) // NPC Specific
          {     
               if ( victim->spec_fun == spec_lookup ( "spec_questmaster" ) )
               {
                    form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Quest Request)", "quest request", "Ask for a quest" ) );
                    form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Quest Complete)", "quest complete", "Complete a quest" ) );
                    form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Quest List)", "quest list", "List Quest Items" ) );
               }
               if ( victim->pIndexData->pShop != NULL )
               {
                    form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Shop) ", "list", "List items for sale" ) );
               }               
          }
          else 						// Player Specific
          {
               SNP ( cmdbuf, "whois %s", victim->name );
               form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Whois)", cmdbuf, "View whois info" ) );
               SNP ( cmdbuf, "follow %s", victim->name );
               form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Follow)", cmdbuf, "Follow this person" ) );
          }
          if ( !is_safe ( ch, victim, FALSE ) ) // FALSE here disables "backtalk" of is_safe...
          {
               SNP ( cmdbuf, "consider %s", victim->name );
               form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Consider)", cmdbuf, "Consider potential enemy" ) );
               SNP ( cmdbuf, "kill %s", victim->name );
               form_to_char ( ch, "%s ", click_cmd ( ch->desc, "(Kill)", cmdbuf, "Kill them!" ) );
          }
          send_to_char ( "\n\r", ch );
     }
     else
     {
          send_to_char ( "\n\rTarget no longer valid.\n\r", ch );
     }
}


