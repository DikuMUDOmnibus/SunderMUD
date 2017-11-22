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

struct clan_main_type   * clan_list;

/* Boot clans */
/* This function is not fool proof, CLAN_FILE must be in correct format. */

DECLARE_DO_FUN ( do_quit );

void boot_clans ( void )
{
     FILE               *fptr;
     FILE               *fp;
     CLAN_INDEX	        *tmp = NULL;
     CLAN_INDEX         *previous = NULL;
     bool                done = FALSE;
     char               *word;
     char                buf[MAX_STRING_LENGTH];
     int                 i, version;
     int                 placeholder;

     SNP ( buf, "%sClan.LST", DATA_DIR );

     if ( ( fptr = fopen ( buf, "r" ) ) == NULL )
     {
          bugf ( "Failed to open CLAN_FILE for reading." );
          return;
     }

     clan_list = NULL;

     while (!done)
     {
          word = fread_string( fptr ); // Get the list items
          if ( !str_cmp ( word, "END" ) )
          {
               done = TRUE;
          }
          else
          {
               if ( ( fp = fopen ( word, "r" ) ) == NULL )
               {
                    SNP (buf, "Failed to open clan %s for reading.", word );
                    exit ( 1 );
               }
               version = fread_number ( fp );
               if ( version > 2 ) // Ie, 2 is our current version number.
               {
                    bugf ( "CLAN::File with version > 2, can't handle. (Found Version %d)", version );
                    exit ( 1 );
               }
               fread_to_eol ( fp ); // Get the LF
               word = fread_string ( fp );
               tmp = (struct clan_main_type *)alloc_perm(sizeof(struct clan_main_type), "clan_main_type" );
               tmp->next = NULL;
               tmp->clan_short = str_dup ( word );

               if ( clan_list == NULL )
                    clan_list = tmp;
               else
                    previous->next = tmp;  // If we get here, previous stays through the loop.
               previous = tmp;

               tmp->clan_name = fread_string ( fp );
               for (i = 0 ; i < 6 ; i++ )
                    tmp->mranks[i] = fread_string( fp );
               for (i = 0 ; i < 6 ; i++ )
                    tmp->franks[i] = fread_string( fp );
               tmp->portalto = fread_number ( fp );
               tmp->status = fread_number ( fp );
               tmp->clanpk = fread_number ( fp );
               tmp->clandie = fread_number ( fp );
               tmp->clanbank = fread_number ( fp );
               tmp->clanmtax = fread_number ( fp );
               tmp->membercount = fread_number ( fp );
               fread_to_eol (fp);

               tmp->war_a = NULL;
               tmp->war_b = NULL;

               // I leave this as an example of clanfile versioning.
               // You can delete this I'm sure since nobody has version 1 clanfiles.
               if ( version >= 2 )
               {
                    tmp->reason = fread_string ( fp );
                    tmp->demigod = fread_string ( fp );
                    tmp->pkallow = fread_number ( fp );
                    tmp->autoaccept = fread_number ( fp );
                    tmp->moveyear = fread_number ( fp );
                    tmp->demiapprove = fread_number ( fp );
                    tmp->experience = fread_number ( fp );
                    tmp->join_level = fread_number ( fp );
                    tmp->join_cost = fread_number ( fp );
                    tmp->join_minalign = fread_number ( fp );
                    tmp->join_maxalign = fread_number ( fp );
                    tmp->rank_setjoin = fread_number ( fp );
                    tmp->rank_recruit = fread_number ( fp );
                    tmp->rank_outcast = fread_number ( fp );
                    tmp->rank_promote = fread_number ( fp );
                    tmp->rank_settax = fread_number ( fp );
                    tmp->rank_demote = fread_number ( fp );
                    placeholder = fread_number ( fp );
                    tmp->rank_declare = fread_number ( fp );
                    tmp->rank_claim = fread_number ( fp );
                    tmp->rank_bounty = fread_number ( fp );
                    tmp->rank_recall = fread_number ( fp );
                    tmp->rank_move = fread_number ( fp );
               }
               else
               {
                    tmp->reason = str_dup ( "" );
                    tmp->demigod = str_dup ( "None" );
                    tmp->pkallow = FALSE;
                    tmp->autoaccept = FALSE;
                    tmp->moveyear = 0;
                    tmp->demiapprove = FALSE;
                    tmp->experience = 0;
                    tmp->join_level = 15;
                    tmp->join_cost = 0;
                    tmp->join_minalign = -1000;
                    tmp->join_maxalign = 1000;
                    tmp->rank_setjoin = RANK_LEADER;
                    tmp->rank_recruit = RANK_LEADER;
                    tmp->rank_outcast = RANK_LEADER;
                    tmp->rank_promote = RANK_LEADER;
                    tmp->rank_settax = RANK_LEADER;
                    tmp->rank_declare = RANK_LEADER;
                    tmp->rank_claim = RANK_LEADER;
                    tmp->rank_bounty = RANK_LEADER;
                    tmp->rank_recall = RANK_LEADER;
                    tmp->rank_move = RANK_LEADER;
               }
               fclose (fp);
          }
     }

     fclose ( fptr );
     return;
}

void save_one_clan ( CLAN_INDEX *tmp )
{
     char buf[256];
     FILE *fp;
     int i;

     if (tmp->nosave)
          return;

     SNP ( buf, "%s%s.clan", DATA_DIR, tmp->clan_short );
     if ( ( fp = fopen ( buf, "w" ) ) == NULL )
     {
          bugf( "Failed to open clan %s for writing.", tmp->clan_short );
          return;
     }
     fprintf (fp, "2\n" ); // Version Number
	 /* Version 1 Fields */
     fprintf (fp, "%s~\n", tmp->clan_short );
     fprintf (fp, "%s~\n", tmp->clan_name );
     for ( i = 0 ; i < 6 ; i++ )
          fprintf (fp, "%s~\n", tmp->mranks[i] );
     for (i = 0 ; i < 6 ; i++ )
          fprintf (fp, "%s~\n", tmp->franks[i] );
     fprintf (fp, "%d %d %d %d %ld %d %d\n",
              tmp->portalto, tmp->status, tmp->clanpk, tmp->clandie,
              tmp->clanbank, tmp->clanmtax, tmp->membercount );

     i = 5; // For some placeholders.
	 /* Version 2 Fields */
     fprintf (fp, "%s~", tmp->reason );
     fprintf (fp, "%s~", tmp->demigod );
     fprintf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              tmp->pkallow, tmp->autoaccept, tmp->moveyear, tmp->demiapprove,
              tmp->experience, tmp->join_level, tmp->join_cost, tmp->join_minalign,
              tmp->join_maxalign, tmp->rank_setjoin, tmp->rank_recruit, tmp->rank_outcast,
              tmp->rank_promote, tmp->rank_settax, tmp->rank_demote, i, // i is placeholder
              tmp->rank_declare, tmp->rank_claim, tmp->rank_bounty, tmp->rank_recall,
              tmp->rank_move );

     fclose ( fp );
     return;
}

void save_clans (void)
{
     FILE               *fptr;
     CLAN_INDEX	       *tmp = NULL;
     char	buf[256];

     SNP ( buf, "%sClan.LST", DATA_DIR );

     if ( ( fptr = fopen ( buf, "w" ) ) == NULL )
     {
          bugf ( "Failed to open CLAN_FILE for writing." );
          return;
     }

     for ( tmp = clan_list; tmp != NULL; tmp = tmp->next )
     {
          if (tmp->nosave)
               continue;
          SNP ( buf, "%s%s.clan", DATA_DIR, tmp->clan_short );
          fprintf ( fptr, "%s~\n", buf );
          save_one_clan ( tmp );
     }

     fprintf (fptr, "END~" );

     fclose ( fptr );

     return;
}

void list_clans (CHAR_DATA *ch, char *argument)
{
     BUFFER		*buffer;
     CLAN_INDEX		*tmp;
     if ( IS_NPC ( ch ) )
          return;

     buffer = buffer_new(1000);

     bprintf( buffer, "{CT{che clans of {W" TXT_MUDNAME "{w\n\r" );
     bprintf( buffer, "{C={c========================================={C::{w\n\r");
     bprintf( buffer, "[Members] [Taxrate] [Victories] [Defeats] :: Name\n\r");

     for ( tmp = clan_list; tmp != NULL; tmp = tmp->next )
     {
          if ( !IS_IMMORTAL ( ch ) && ( tmp->status < CLAN_RECOGNIZED ) )
               continue;
          bprintf( buffer, "{C[{w%7d{C] {C[{w%7d{C] {C[{w%9d{C] {C[{w%7d{C] {c:: {w%s", tmp->membercount,
                   tmp->clanmtax, tmp->clanpk, tmp->clandie, tmp->clan_name );
          if (tmp->nosave)
               bprintf( buffer, "{W({RDELETE STATUS{W)" );
          if ( IS_IMMORTAL ( ch ) )
          {
               switch (tmp->status)
               {
               case CLAN_NEW:
                    bprintf ( buffer, " {GNew Clan{w" );
                    break;
               case CLAN_PROBATION:
                    bprintf ( buffer, " {ROn Probation{w" );
                    break;
               case CLAN_UNDERGROUND:
                    bprintf ( buffer, " {rUnderGround (Hidden){w" );
                    break;
               case CLAN_RECOGNIZED:
                    break;
               case CLAN_RENEW:
                    bprintf ( buffer, " {YReNew Status{w" );
                    break;
               default:
                    bprintf ( buffer, " {R{&A Bug!!{w" );
                    break;
               }
          }
          bprintf(buffer, "\n\r");
     }

     page_to_char(buffer->data, ch);
     buffer_free(buffer);

     return;
}

void do_claninfo( CHAR_DATA *ch, char *argument)
{
     CLAN_INDEX *tmp;
     CLAN_INDEX *fwtmp;
     BUFFER     *buffer;
     bool        foundwar = FALSE;
     int         i;
     
     if ( IS_NPC ( ch ) )
          return;

	 /* If in a clan, let's show our own clan first */

     if (argument[0] == '\0')
     {
          if (ch->pcdata->clan)
               tmp = ch->pcdata->clan;
          else
          {
               send_to_char("Syntax: claninfo <clan_short>\n\r", ch);
               return;
          }
     }
     else
          tmp = clan_by_short (argument);

     if (!tmp)
     {
          send_to_char("No such clan found.\n\r", ch);
          return;
     }

	 /* Now check if the user can see the clan. These rules can't work on greater or less than :(. */
	 /* Underground clans are hidden from public view. */

     if ( tmp->status == CLAN_UNDERGROUND && ( ch->pcdata->clan != tmp || !IS_IMMORTAL(ch) ) )
     {
          send_to_char ("No such clan found.\n\r", ch );
          return;
     }

     buffer = buffer_new(1000);

	 /* Okay let's show the clan stat! */

     bprintf(buffer, "{C={c======================================{C[ {wClan: {W%10s {C]{c={w\n\r",
             tmp->clan_short );
     bprintf(buffer, "   %s{x\n\r", tmp->clan_name);
     bprintf(buffer, "{C-{c----------------------------------------------------------{C-{w\n\r" );
     bprintf(buffer, "{G    %20s    %20s\n\r", "Male Ranks", "Female Ranks" );
     for (i = 5 ; i >= 0 ; i-- )   // Yeah it's backwords, sue me.
          bprintf(buffer, "{w    %20s    %20s\n\r", tmp->mranks[i], tmp->franks[i] );
     bprintf(buffer, "{x{C-{c----------------------------------------------------------{C-{w\n\r" );
     switch ( tmp->status )
     {
     case CLAN_NEW:
          bprintf ( buffer, "{RUnrecognized, New Clan.{w\n\r" );
          break;
     case CLAN_PROBATION:
          bprintf ( buffer, "{RUnrecognized, PROBATION.{w\n\r" );
          break;
     case CLAN_UNDERGROUND:
          bprintf ( buffer, "{DUnderground...{w\n\r" );
          break;
     case CLAN_RECOGNIZED:
     case CLAN_RENEW:  // Don't want non-imms to see if a clan is needing renewal
          bprintf ( buffer, "{COfficially Recognized Clan.{w\n\r" );
          break;
     default:
          bprintf ( buffer, "{WClan is experiencing a {RBUG{w.\n\r" );
          break;
     }
     if ( tmp->pkallow )
     {
          bprintf(buffer, "   {GPkill Victories: {W%d   {YPkill Defeats: {W%d{w\n\r",
                  tmp->clanpk, tmp->clandie );
     }
     bprintf(buffer, "   {GClan Experience: {W%d   {GClan DemiGod: {W%s{w\n\r",
             tmp->experience,
             tmp->demiapprove ? tmp->demigod : "None" );
     bprintf(buffer, "   {GMember Tax: {W%d   {GMember Count: {W%d{w\n\r",
             tmp->clanmtax, tmp->membercount );

     if ( IS_IMMORTAL(ch) || ch->pcdata->clan == tmp )
     {
          bprintf (buffer, "   {GBank Account     {W%d{w\n\r", tmp->clanbank );
     }

     if ( tmp->war_a || tmp->war_b )
     {
          bprintf ( buffer, "At War With: \n\r" );
          if ( tmp->war_a )
          {
               foundwar = TRUE;
               bprintf ( buffer, "   %s {W({RAgressor{W){w\n\r", tmp->war_a->clan_name );
          }
          if ( tmp->war_b )
          {
               foundwar = TRUE;
               bprintf ( buffer, "   %s {W({RAgressor{W){w\n\r", tmp->war_b->clan_name );
          }
     }
     for ( fwtmp = clan_list; fwtmp != NULL; fwtmp = fwtmp->next )
     {
          if ( ( fwtmp->war_a == tmp ) || ( fwtmp->war_b == tmp ) )
          {
               if (!foundwar)
                    bprintf ( buffer, "At War With: \n\r" );
               bprintf ( buffer, "   %s {W({YDefender{W){w\n\r", fwtmp->clan_name );
               foundwar = TRUE;
          }
     }

     bprintf(buffer, "{C={c=========================================================={C={w\n\r");
     page_to_char(buffer->data, ch);
     buffer_free(buffer);
}

// clan_war can be safely called if players aren't in a clan.
//
bool clan_war ( CHAR_DATA *ch, CHAR_DATA *victim )
{
     if ( !ch->pcdata->clan || !victim->pcdata->clan )
          return FALSE;
     if ( ( ch->pcdata->clan->war_a == victim->pcdata->clan )
          || ( ch->pcdata->clan->war_b == victim->pcdata->clan )
          || ( victim->pcdata->clan->war_a == ch->pcdata->clan )
          || ( victim->pcdata->clan->war_b == ch->pcdata->clan ) )
          return TRUE;
     return FALSE;
}

/* the order of who is who doesn't matter here */
bool is_same_clan (CHAR_DATA *ch, CHAR_DATA *victim)
{

     if ( IS_NPC ( ch ) || IS_NPC ( victim ) )
          return FALSE;

	 /* Return false automatically if either (or both for that matter) are clanless */

     if ( !ch->pcdata->clan || !victim->pcdata->clan )
          return FALSE;

	 /* Return false if loner. Not a real clan. */

     if (!strcmp(ch->pcdata->clan->clan_short, "Loner") )
          return FALSE;
     if (!strcmp(victim->pcdata->clan->clan_short, "Loner") )
          return FALSE;

    /* Now do a check */

     if (!strcmp(victim->pcdata->clan->clan_short, ch->pcdata->clan->clan_short) )
          return TRUE;

	 /* Elsewise, return fasle */
     return FALSE;

}

CLAN_INDEX *clan_by_short (char *argument)
{
     CLAN_INDEX *lookup;
     bool found= FALSE;

     for ( lookup = clan_list; lookup != NULL; lookup = lookup->next )
     {
          if (!str_cmp(lookup->clan_short, argument) )
          {
               found = TRUE;
               break;
          }
     }
     if (found)
          return lookup;
     return NULL;
}

/* Immortal function to clear someone's clan pointer */
/* Not to be used in place of normal outcasting by a clan leader */

void do_declan(CHAR_DATA *ch, char *argument)
{
     CLAN_INDEX *tmp;
     CHAR_DATA *victim;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' )
     {
          send_to_char( "Syntax: declan <character>\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world( ch, argument ) ) == NULL )
     {
          send_to_char( "Don't see that player.\n\r", ch );
          return;
     }

     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's aren't in clans you siwwy IMM!\n\r", ch);
          return;
     }

     if (victim->pcdata->clan)
     {
          tmp = victim->pcdata->clan;
          victim->pcdata->clan->membercount--;  /* Decrement the counter */
          victim->pcdata->clan = NULL;		/* Nullify the pointer */
          victim->pcdata->clrank = -1;	/* Set Rank to null */
          send_to_char("All of a sudden, you feel that you are clanless.\n\r", victim);
          send_to_char("You have declanned the player.\n\r", ch);
          log_string( "INFO: %s declanned by %s", victim->name, ch->name);
          save_one_clan ( tmp );
         notify_message (victim, NOTIFY_CLANQUIT, TO_CLAN, tmp->clan_name);
     }
     else
          send_to_char("Your victim is already clanless.\n\r", ch);

     return;
}

/* Immortal function to set someone as a member of a clan */
/* NOT to be used in place of normal clan recruit function normally ! */
/* Sends no global notice */

void do_setclan (CHAR_DATA *ch, char *argument)
{
     CHAR_DATA   *victim;
     CLAN_INDEX  *tmp;
     char         arg1[MAX_INPUT_LENGTH];
     char         arg2[MAX_INPUT_LENGTH];

     if ( IS_NPC ( ch ) )
          return;

     argument = one_argument( argument, arg1 );
     argument = one_argument( argument, arg2 );

     if ( arg1[0] == '\0' || arg2[0] == '\0' )
     {
          send_to_char( "Syntax: setclan <character> <clan>.\n\r", ch );
          return;
     }
     if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
     {
          send_to_char( "Don't see that player.\n\r", ch );
          return;
     }
     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's don't belong in clans you siwwy IMM!\n\r", ch);
          return;
     }
     if (victim->level < 5)
     {
          send_to_char("Players must reach level 10 before joining a clan.\n\r", ch);
          return;
     }
     if (victim->pcdata->clan)
     {
          send_to_char("That player is already in a clan!\n\r", ch);
          return;
     }
     tmp = clan_by_short( arg2 );
     if (!tmp)
     {
          send_to_char("That is not a valid clan.\n\r", ch);
          return;
     }

     victim->pcdata->clan = tmp;
     victim->pcdata->clan->membercount++;

     send_to_char("Thy will be done.\n\r", ch);

     victim->pcdata->clrank = RANK_BOTTOM;

     log_string( "INFO: %s setclanned into clan %s by %s.",
             victim->name, victim->pcdata->clan->clan_short, ch->name );

     form_to_char ( victim, "You are now a member of %s!\n\r", victim->pcdata->clan->clan_name );

     notify_message (victim, NOTIFY_CLANACCEPT, TO_CLAN, victim->pcdata->clan->clan_name);
     save_char_obj(victim);
     save_one_clan ( tmp );

     return;
}

void do_clan_tell (CHAR_DATA *ch, char *argument)
{
	 /* No NPC's */
     if (IS_NPC(ch))
          return;
     if (!ch->pcdata->clan)
     {
          send_to_char ("You do not belong to any clan.\n\r",ch);
          send_to_char ("Once you join a clan, you can use the clan channel.\n\r",ch);
          return;
     }
     if (!strcmp(ch->pcdata->clan->clan_short, "Loner") )
     {
          send_to_char("You're a loner, you can't use the clan channel.\n\r", ch);
          return;
     }
     channel_message (ch, argument, "clantalk");
     return;
}

void clan_advance (CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *victim;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument [0] == '\0' )
     {
          send_to_char( "Syntax: advance <character>\n\r", ch );
          return;
     }
     if ( ( victim = get_char_world( ch, argument ) ) == NULL )
     {
          send_to_char( "Don't see that player.\n\r", ch );
          return;
     }
     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's don't have ranks!\n\r", ch);
          return;
     }
     if (ch == victim)
     {
          send_to_char("It isn't quite this easy.\n\r", ch);
          return;
     }
     if (!victim->pcdata->clan)
     {
          send_to_char("That player is not in a clan!\n\r", ch);
          return;
     }
     if (!ch->pcdata->clan)
     {
          send_to_char("You're not even in a clan yourself.\n\r", ch);
          return;
     }
     if (!is_same_clan (victim, ch) && !IS_IMMORTAL (ch) )
     {
          send_to_char("You are not authorized to advance this player.\n\r", ch);
          return;
     }

	 /* No is_immortal check here. Only leaders of fates therefore can set other clans ranks */

     if (ch->pcdata->clrank < ch->pcdata->clan->rank_promote )
     {
          send_to_char("You may not.\n\r", ch);
          return;
     }
     if (ch->pcdata->clrank <= victim->pcdata->clrank )
     {
          send_to_char ("You cannot promote someone that is your rank or higher.\n\r", ch );
          return;
     }

     if (victim->pcdata->clrank >= RANK_LEADER)
     {
          form_to_char ( ch, "%s is already a %s of %s.\n\r", victim->name,
                         victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
                         victim->pcdata->clan->mranks[victim->pcdata->clrank],
                         victim->pcdata->clan->clan_name );
          return;
     }

     victim->pcdata->clrank++;

     form_to_char ( ch, "%s is now a %s of %s.\n\r", victim->name,
                    victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
                    victim->pcdata->clan->mranks[victim->pcdata->clrank],
                    victim->pcdata->clan->clan_name );

     form_to_char ( victim, "You are now a %s of %s.\n\r",
                    victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
                    victim->pcdata->clan->mranks[victim->pcdata->clrank],
                    victim->pcdata->clan->clan_name );

     notify_message (victim, NOTIFY_CLANPROMOTE, TO_CLAN, victim->sex == 2 ?
                     victim->pcdata->clan->franks[victim->pcdata->clrank] :
                     victim->pcdata->clan->mranks[victim->pcdata->clrank] );
     save_char_obj(victim);
     return;
}

void clan_demote (CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *victim;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument [0] == '\0' )
     {
          send_to_char( "Syntax: demote <character>\n\r", ch );
          return;
     }

     if ( ( victim = get_char_world( ch, argument ) ) == NULL )
     {
          send_to_char( "Don't see that player.\n\r", ch );
          return;
     }

     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's don't have ranks!\n\r", ch);
          return;
     }

     if (!victim->pcdata->clan)
     {
          send_to_char("That player is not in a clan!\n\r", ch);
          return;
     }

     if (!ch->pcdata->clan)
     {
          send_to_char("You're not even in a clan yourself.\n\r", ch);
          return;
     }

     if (!is_same_clan (victim, ch) && !IS_IMMORTAL (ch) )
     {
          send_to_char("You are not authorized to demote this player.\n\r", ch);
          return;
     }
     if (ch == victim)
     {
          send_to_char("Sorry, can't demote yourself.\n\r", ch);
          return;
     }

     if (ch->pcdata->clrank < ch->pcdata->clan->rank_demote )
     {
          send_to_char("You may not.\n\r", ch);
          return;
     }
     if (ch->pcdata->clrank <= victim->pcdata->clrank && !IS_IMMORTAL(ch))
     {
          send_to_char ("You cannot demote someone that is your rank or higher.\n\r", ch );
          if (ch->pcdata->clrank == RANK_LEADER)
               send_to_char ("To demote another leader, you must contact an IMM.\n\r", ch);
          return;
     }

     if (victim->pcdata->clrank <= RANK_BOTTOM)
     {
          form_to_char ( ch, "%s is already a %s of %s.\n\r", victim->name,
                         victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
                         victim->pcdata->clan->mranks[victim->pcdata->clrank],
                         victim->pcdata->clan->clan_name );
          return;
     }

     victim->pcdata->clrank--;

     form_to_char ( ch, "%s is now a %s of %s.\n\r", victim->name,
                    victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
                    victim->pcdata->clan->mranks[victim->pcdata->clrank],
                    victim->pcdata->clan->clan_name );

     notify_message (victim, NOTIFY_CLANDEMOTE, TO_CLAN, victim->sex == 2 ?
                     victim->pcdata->clan->franks[victim->pcdata->clrank] :
                     victim->pcdata->clan->mranks[victim->pcdata->clrank] );

     form_to_char ( victim, "DEMOTION! You are now a %s of %s.\n\r",
              victim->sex == 2 ? victim->pcdata->clan->franks[victim->pcdata->clrank] :
              victim->pcdata->clan->mranks[victim->pcdata->clrank],
              victim->pcdata->clan->clan_name);

     save_char_obj(victim);

     return;
}

void clan_accept (CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *victim;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument [0] == '\0' )
     {
          send_to_char( "Syntax: advance <character>\n\r", ch );
          return;
     }
     if ( ( victim = get_char_room( ch, NULL, argument ) ) == NULL )
     {
          send_to_char( "Don't see that player in this room.\n\r", ch );
          return;
     }
     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's can't join clans!\n\r", ch);
          return;
     }
     if (!ch->pcdata->clan)
     {
          send_to_char("You're not even in a clan yourself.\n\r", ch);
          return;
     }
     if (ch == victim)
     {
          send_to_char("Huh???\n\r", ch);
          return;
     }
     if (victim->pcdata->clan)
     {
          send_to_char("That player is already in a clan!\n\r", ch);
          return;
     }
     if (ch->pcdata->clrank < ch->pcdata->clan->rank_recruit )
     {
          send_to_char("You may not.\n\r", ch);
          return;
     }
     if ( !victim->pcdata->petition )
     {
          send_to_char ("They don't seem to have asked to join.\n\r", ch );
          return;
     }
     if ( victim->level < ch->pcdata->clan->join_level )
     {
          send_to_char ("They aren't high enough level to join your clan.\n\r", ch );
          send_to_char ("You aren't high enough level to join that clan.\n\r", victim );
          return;
     }
     if ( victim->gold < ch->pcdata->clan->join_cost )
     {
          send_to_char ("They don't seem to be wealthy enough to join your clan.\n\r", ch );
          send_to_char ("You aren't wealthy enough to join that clan.\n\r", victim );
          return;
     }
     if ( victim->alignment < ch->pcdata->clan->join_minalign )
     {
          send_to_char ("They aren't good enough to join your clan.\n\r", ch );
          send_to_char ("You aren't good enough to join that clan.\n\r", victim );
          return;
     }
     if ( victim->alignment > ch->pcdata->clan->join_maxalign )
     {
          send_to_char ("Their do-gooder ways wouldn't settle well in your clan.\n\r", ch );
          send_to_char ("The evil ways of that clan don't appeal to you.\n\r", victim );
          return;
     }
	 /* Okay ... go ahead and set the victim's clan pointer */

     victim->pcdata->clan = ch->pcdata->clan;
     ch->pcdata->clan->membercount++;
     victim->pcdata->clrank = RANK_BOTTOM;
     victim->pcdata->petition = NULL;

     victim->gold -= ch->pcdata->clan->join_cost;
     send_to_char("Done.\n\r", ch);
     form_to_char ( victim, "You are now a member of %s!\n\r", ch->pcdata->clan->clan_name );
     notify_message (victim, NOTIFY_CLANACCEPT, TO_CLAN, ch->pcdata->clan->clan_name);
     save_one_clan ( ch->pcdata->clan );
     save_char_obj(victim);
}

void clan_petition (CHAR_DATA *ch, char *argument)
{
     DESCRIPTOR_DATA *d;
     CLAN_INDEX      *tmp;
     CHAR_DATA       *victim;
     bool             join = FALSE;

     if ( IS_NPC ( ch ) )
          return;

     if ( argument [0] == '\0' )
     {
          send_to_char ("You must specify a clan to petition.\n\r", ch );
          return;
     }
     else if ( !str_cmp ( argument, "clear" ))
     {
          if (ch->pcdata->petition)
          {
               form_to_char ( ch, "You have decided against joining %s\n\r", ch->pcdata->petition->clan_name );
               ch->pcdata->petition = NULL;
          }
          else
               send_to_char ("You aren't petitioning to join a clan.\n\r", ch );
          return;
     }
     else if ( !str_cmp ( argument, "join" ) ) // handle autojoins
     {
          if ( !ch->pcdata->petition )
          {
               send_to_char ("You must petition a clan first.\n\r", ch );
               return;
          }
          if ( !ch->pcdata->petition->autoaccept )
          {
               send_to_char ("That clan requires a member to approve your petition.\n\r", ch );
               return;
          }
          tmp = ch->pcdata->petition;
          join = TRUE;
     }
     else
     {
          tmp = clan_by_short (argument);
          if ( !tmp )
          {
               send_to_char ( "That clan doesn't seem to exist.\n\r", ch );
               return;
          }
     }

     if (ch->pcdata->clan)
     {
          send_to_char("You're already in a clan.\n\r", ch);
          return;
     }

    /* Thanks to Dregnus (jholt@acom.us) for the autojoin fix below */
    if ( ch->pcdata->petition && join != TRUE )
    {
        send_to_char ("You have already petitioned either this or another clan.\n\r"
                      "You may clear the petition with \"petition clear\".\n\r", ch );
        return;
    }

     if ( ch->level < tmp->join_level )
     {
          send_to_char ("You aren't high enough level to join that clan.\n\r", ch );
          return;
     }
     if ( ch->gold < tmp->join_cost )
     {
          send_to_char ("You aren't wealthy enough to join that clan.\n\r", ch );
          return;
     }
     if ( ch->alignment < tmp->join_minalign )
     {
          send_to_char ("You aren't good enough to join that clan.\n\r", ch );
          return;
     }
     if ( ch->alignment > tmp->join_maxalign )
     {
          send_to_char ("The evil ways of that clan don't appeal to you.\n\r", ch );
          return;
     }

     if ( !join )
     {
         form_to_char ( ch, "You must be level %d", tmp->join_level );
         if (tmp->join_cost > 0 )
         {
             form_to_char ( ch, " and pay %d gold", tmp->join_cost );
         }
         form_to_char ( ch, " to join %s\n\r", tmp->clan_name );
         if ( tmp->join_minalign > -1000 )
         {
             form_to_char ( ch, "Minimum Alignment: %d\n\r", tmp->join_minalign );
         }
         if ( tmp->join_maxalign < 1000 )
         {
             form_to_char ( ch, "Maximum Alignment: %d\n\r", tmp->join_maxalign );
         }
         
          if (tmp->autoaccept)
          {
               form_to_char ( ch, "Good News!! %s can automatically accept you, if you like.\n\r", tmp->clan_name );
               send_to_char ( "To join this clan now, just type \"petition join\".\n\r", ch );
          }
          else
          {
               form_to_char ( ch, "You are now petitioning to join %s. Clan members have been notified.\n\r", tmp->clan_name );
               send_to_char ("This petition will last until you log off.\n\r", ch );
          }

          // Bad hack to find a member of the clan online to use to get the notify sent
          for ( d = descriptor_list; d; d = d->next )
          {
               if ( d->connected != CON_PLAYING )
                    continue;
               victim = d->original ? d->original : d->character;

               if ( victim->pcdata->clan )
               {
                    if ( victim->pcdata->clan == tmp )
                    {
                         notify_message (victim, NOTIFY_CLANPETITION, TO_CLAN, victim->name );
                         break;
                    }
               }
          }
          ch->pcdata->petition = tmp;
          return;
     }
     else // We're autojoining. Yay.
     {
          ch->pcdata->clan = ch->pcdata->petition;
          ch->pcdata->clan->membercount++;
          ch->pcdata->clrank = RANK_BOTTOM;
          ch->pcdata->petition = NULL;

          ch->gold -= tmp->join_cost;

          form_to_char ( ch, "You are now a member of %s!\n\r", ch->pcdata->clan->clan_name );

          notify_message (ch, NOTIFY_CLANACCEPT, TO_CLAN, ch->pcdata->clan->clan_name);
          save_one_clan ( ch->pcdata->clan );
          save_char_obj(ch);
     }
     return;
}

void clan_outcast(CHAR_DATA *ch, char *argument)
{
     CHAR_DATA *victim;

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char( "Syntax: outcast <character>\n\r", ch );
          return;
     }
     if ( ( victim = get_char_world( ch, argument ) ) == NULL )
     {
          send_to_char( "Don't see that player.\n\r", ch );
          return;
     }
     if (IS_NPC(victim) )
     {
          send_to_char ("NPC's aren't in clans!\n\r", ch);
          return;
     }
     if (!ch->pcdata->clan)
     {
          send_to_char("You're not even in a clan yourself.\n\r", ch);
          return;
     }
     if (ch == victim)
     {
          send_to_char("Sorry, you can't outcast yourself.\n\rIf you are a leader and wish to leave, contact in IMM.\n\r", ch);
          return;
     }
     if (!victim->pcdata->clan)
     {
          send_to_char("That player is not in a clan!\n\r", ch);
          return;
     }
     if (!is_same_clan (victim, ch) || ch->pcdata->clrank < ch->pcdata->clan->rank_outcast )
     {
          send_to_char("You're not authorized for that.\n\r", ch);
          return;
     }
     if (victim->pcdata->clrank >= ch->pcdata->clrank)
     {
          send_to_char("You cannot outcast someone the same or higher rank than you.\n\r", ch);
     }
     notify_message (victim, NOTIFY_CLANQUIT, TO_CLAN, ch->pcdata->clan->clan_name);

     ch->pcdata->clan->membercount--;  /* Decrement the counter */
     victim->pcdata->clan = NULL;            /* Nullify the pointer */
     victim->pcdata->clrank = -1;    /* Set Rank to null */
     form_to_char ( victim, "{R%s has outcast you!!!{w", ch->name );
     form_to_char ( ch, "You have outcast %s!\n\r", victim->name );
     log_string( "CLANINFO: %s outcast by %s", victim->name, ch->name);
     save_one_clan ( ch->pcdata->clan );

     return;
}

void clan_truce ( CHAR_DATA *ch, char *argument )
{
     CLAN_INDEX *tmp = NULL;
     char buf[MIL];

     if ( IS_NPC ( ch ) )
          return;
     if ( !ch->pcdata->clan )
     {
          send_to_char ( "Who are you to be talking of war?\n\r", ch );
          return;
     }
     if ( ch->pcdata->clrank < ch->pcdata->clan->rank_declare )
     {
          send_to_char ( "Perhaps you had best consult with your superiors about this first.\n\r", ch );
          return;
     }
     if (argument[0] == '\0')
     {
          send_to_char("Syntax: clantruce <clan_short>\n\r", ch);
          return;
     }
     else
          tmp = clan_by_short (argument);
     if (!tmp)
     {
          send_to_char("No such clan found.\n\r", ch);
          return;
     }
     if ( ( tmp->war_a == ch->pcdata->clan ) || ( tmp->war_b == ch->pcdata->clan ) )
     {
          send_to_char ( "Sounds nice, but they are the ones who instigated the war.\n\r", ch );
          return;
     }
     if ( !ch->pcdata->clan->war_a && !ch->pcdata->clan->war_b )
     {
          send_to_char ( "You are not the agressor in any current warfare.\n\r", ch );
          return;
     }
     if ( ( ch->pcdata->clan->war_a != tmp ) && ( ch->pcdata->clan->war_b != tmp ) )
     {
          send_to_char ( "You aren't at war with that clan!\n\r", ch );
          return;
     }

     // clear the war
     //
     if ( ch->pcdata->clan->war_a == tmp )        // Find which pointer to free
          ch->pcdata->clan->war_a = NULL;
     else if ( ch->pcdata->clan->war_b == tmp )
          ch->pcdata->clan->war_b = NULL;

     SNP ( buf, "%s {whas ceased hostilities against %s{w.", ch->pcdata->clan->clan_name, tmp->clan_name );
     notify_message (ch, NOTIFY_CLANG, TO_ALL, buf );
     send_to_char ( "Done.\n\r", ch );

     save_one_clan ( ch->pcdata->clan );
     return;
}

void clan_declare ( CHAR_DATA *ch, char *argument )
{
     CLAN_INDEX   *tmp = NULL;
     char buf[MIL];

     if ( IS_NPC ( ch ) )
          return;
     if ( !ch->pcdata->clan )
     {
          send_to_char ( "Who are you to be talking of war?\n\r", ch );
          return;
     }
     if ( ch->pcdata->clrank < ch->pcdata->clan->rank_declare )
     {
          send_to_char ( "Perhaps you had best consult with your superiors about this first.\n\r", ch );
          return;
     }
     if ( !ch->pcdata->clan->pkallow )
     {
          send_to_char ( "Your clan's peaceful ways do not allow warfare.\n\r", ch );
          return;
     }
     if (argument[0] == '\0')
     {
          send_to_char("Syntax: clanwar <clan_short>\n\r", ch);
          return;
     }
     else
          tmp = clan_by_short (argument);
     if (!tmp)
     {
          send_to_char("No such clan found.\n\r", ch);
          return;
     }
     if ( tmp == ch->pcdata->clan )
     {
          send_to_char ( "Sad to see you think so little of your own clan.\n\r", ch );
          return;
     }
     if ( !str_cmp ( argument, "Loner" ) )
     {
          send_to_char ("Haha... Funny. Run along.\n\r", ch );
          return;
     }
     if ( !tmp->pkallow )
     {
          send_to_char ("That clan does not participate in warfare.\n\r", ch );
          return;
     }
     if ( ch->pcdata->clan->war_a && ch->pcdata->clan->war_b )
     {
          send_to_char ( "World domination is best left to the Gods.\n\r", ch );
          return;
     }
     if ( ( tmp->war_a == ch->pcdata->clan ) || ( tmp->war_b == ch->pcdata->clan ) )
     {
          send_to_char ( "You are already at war with that clan!\n\r", ch );
          return;
     }
     // Okay set the war up.
     //
     if ( !ch->pcdata->clan->war_a )        // Find which pointer is free
          ch->pcdata->clan->war_a = tmp;
     else if ( !ch->pcdata->clan->war_b )
          ch->pcdata->clan->war_b = tmp;

     SNP ( buf, "%s {whas unleashed the dogs of {RWAR{w on %s{w!!!", ch->pcdata->clan->clan_name, tmp->clan_name );
     notify_message (ch, NOTIFY_CLANG, TO_ALL, buf );
     send_to_char ( "Done.\n\r", ch );

     save_one_clan ( ch->pcdata->clan );
     return;
}

void make_clan (CHAR_DATA *ch, char *argument)
{
     CLAN_INDEX          *tmp = NULL;
     CLAN_INDEX          *previous = NULL;
     LEASE               *lease = NULL;
     char		  cbuf[MIL];
     int                  i = 0;
     int                  hq = 0;
     bool                 self = TRUE;
     char                *pc;
     bool                 fIll = TRUE;

     if ( IS_NPC (ch) )
          return;

     if ( argument [0] == '\0' )
     {
          send_to_char("You must specify a name to use for the \"short\" name of this clan.\n\r", ch);
          send_to_char("Additionally, you may type \"makeclan cost\" to find out the current costs.\n\r", ch);
          return;
     }

     if ( !str_cmp (argument, "cost" ) )
     {
          send_to_char ("The gods have declared that it shall cost the following to make a clan:\n\r", ch );
          form_to_char ( ch, "  Gold {W: {Y%d{w", mud.makecost );
          form_to_char ( ch, "    QP {W: {Y%d{w\n\r", mud.makeqp );
          form_to_char ( ch, "You must be at least level {C%d{w.\n\r", mud.makelevel );
          send_to_char ( "You must also be in the room you wish this clan's headquarters to be located.\n\r", ch );
          send_to_char ( "That may either be the City Common Room in Lukhan, or a room you have leased.\n\r", ch );
          return;
     }

     if ( (strlen (argument) > 10 ) || (strlen (argument) < 3) )
     {
          send_to_char("Clans must have a shortname greater than 3 letters, and shorter than 10.\n\r", ch);
          return;
     }

     for ( pc = argument; *pc != '\0'; pc++ )
     {
          if ( !isalpha ( *pc ) )
               fIll = FALSE;
          if ( LOWER ( *pc ) != 'i' && LOWER ( *pc ) != 'l' )
               fIll = FALSE;
     }

     if ( fIll )
     {
          send_to_char ("That name does not seem to be acceptable.\n\r"
                        "However, if you feel the system has disallowed a perfectly good name,\n\r"
                        "you may contact an admin if you like about this.\n\r", ch);
          return;
     }

     tmp = clan_by_short (argument);

     if (tmp)
     {
          send_to_char("Odd, it seems that clan already exists.\n\r", ch);
          return;
     }

     if ( IS_IMMORTAL (ch) && ch->pcdata->clan )
     {
          self = FALSE;
          send_to_char ("Assuming you are making someone else's clan, Immortal....\n\r", ch);
     }

	 /* A set of rules for mortals to be able to make a clan */

     if ( !IS_IMMORTAL (ch) )
     {
          if (ch->pcdata->clan)
          {
               send_to_char ("It seems your current clan may not like the idea.\n\r", ch);
               return;
          }
          if ( ch->pcdata->account->status < ACCT_VERIFIED )
          {
               send_to_char ("You may not make a clan until you verify your account.\n\r", ch );
               return;
          }
          if (ch->gold < mud.makecost)
          {
               send_to_char ("You don't appear to be wealthy enough to start a clan.\n\r", ch);
               form_to_char ( ch, "You would need ${Y%d{w coins to do so.\n\r", mud.makecost );
               return;
          }
          if (ch->level < mud.makelevel )
          {
               send_to_char ("It would appear that you are not well enough known to start a clan.\n\r", ch );
               form_to_char ( ch, "You need to advance to level {C%d{w first.\n\r", mud.makelevel );
               return;
          }
          if (ch->pcdata->questpoints < mud.makeqp )
          {
               send_to_char ("Questor has vetoed your attempt to make a clan.\n\r", ch );
               form_to_char ( ch, "You would need {G%d{w quest points to do so.\n\r", mud.makeqp );
               return;
          }
          if ( ch->in_room->vnum == ROOM_VNUM_TEMPLE )
          {
               send_to_char ( "Your temporary clan headquarters will be the City Common Room.\n\r", ch );
               hq = ROOM_VNUM_TEMPLE;
          }
          else
          {
               if ( IS_LEASE (ch->in_room->lease) )
               {
                    lease = ch->in_room->lease;

                    if ( IS_RENTED(lease) )
                    {
                         if ( lease->rented_by != ch->name )
                         {
                              send_to_char ("You cannot build a clan here.\n\r", ch );
                              return;
                         }
                         else
                         {
                              send_to_char ("Setting clan headquarters to this room.\n\r", ch );
                              hq = ch->in_room->vnum;
                              lease->rented_by = str_dup ("Clan"); /* Little hack */
							  /* No clan yet, can't set the pointer */
                         }
                    }
                    else
                    {
                         send_to_char ("You must lease a room to be your clan's headquarters, and be in that room, to create a clan.\n\r", ch );
                         send_to_char ("Alternatively, you can set up a clan without a headquarters. To do so, create the clan in the City Common Room of Lukhan.\n\r", ch );
                         return;
                    }
               }
               else
               {
                    send_to_char ("You must lease a room to be your clan's headquarters, and be in that room, to create a clan.\n\r", ch );
                    send_to_char ("Alternatively, you can set up a clan without a headquarters. To do so, create the clan in the City Common Room of Lukhan.\n\r", ch );
                    return;
               }
          }

          ch->gold -= mud.makecost;
          ch->pcdata->questpoints -= mud.makeqp;

          form_to_char ( ch, "Charging you ${Y%d{w coins and {G%d{w questpoints.\n\r", mud.makecost, mud.makeqp );
     }

     if ( IS_IMMORTAL (ch) )
          hq = ROOM_VNUM_TEMPLE;

     for ( tmp = clan_list; tmp != NULL; tmp = tmp->next )
          previous = tmp;

     tmp = (struct clan_main_type *)alloc_perm(sizeof(struct clan_main_type), "clan_main_type" );
     tmp->next = NULL;
     if ( clan_list == NULL )
          clan_list = tmp;
     else
          previous->next = tmp;

     previous = tmp;
     tmp->clan_short = str_dup (capitalize (argument ) );
     SNP (cbuf, "%s (New)", argument );
     tmp->clan_name = str_dup (cbuf);
     for (i = 0 ; i < 6 ; i++ )
          tmp->mranks[i] = str_dup ( "Man" );
     for (i = 0 ; i < 6 ; i++ )
          tmp->franks[i] = str_dup ( "Woman" );
     tmp->portalto = 1; 	/* No Portal Yet */
     tmp->status = CLAN_NEW;
     tmp->clanpk = 0;
     tmp->clandie = 0;
     tmp->clanbank = 0;
     tmp->clanmtax = 0;
     tmp->membercount = 0;

     tmp->reason = str_dup ("Unchecked");
     tmp->pkallow = FALSE;
     tmp->autoaccept = FALSE;
     tmp->moveyear = 0;
     tmp->demigod = str_dup ("Nobody");
     tmp->demiapprove = FALSE;
     tmp->experience = 0;
     tmp->join_level = 5; /* A good default */
     tmp->join_cost = 5000; /* ditto */
     tmp->join_minalign = -1000;
     tmp->join_maxalign = 1000;
     tmp->rank_setjoin = RANK_LEADER;
     tmp->rank_recruit = RANK_LEADER;
     tmp->rank_outcast = RANK_LEADER;
     tmp->rank_promote = RANK_LEADER;
     tmp->rank_demote = RANK_LEADER;
     tmp->rank_settax = RANK_LEADER;
     tmp->rank_declare = RANK_LEADER;
     tmp->rank_claim = RANK_LEADER;
     tmp->rank_bounty = RANK_LEADER;
     tmp->rank_recall = RANK_LEADER;
     tmp->rank_move = RANK_LEADER;
     tmp->war_a = NULL;
     tmp->war_b = NULL;

     send_to_char ("Your clan has been made!\n\r", ch );
     log_string( "[CLAN]:: %s made by %s", argument, ch->name );

     if ( hq == ROOM_VNUM_TEMPLE ) // We depend on hq to equal this if there's no lease. Make sure this doesn't break.
     {
          tmp->portalto = ROOM_VNUM_TEMPLE;
     }
     else
     {
          if ( !IS_LEASE ( lease ) )
          {
               send_to_char ( "Woops. Bugola. Null Lease-o-roonie.\n\r", ch );
               bugf ("We got a null lease creating a clan.");
               return;
          }
          tmp->portalto = ch->in_room->vnum;
          lease->clan = ch->pcdata->clan; // This depends on making sure the room is leased!
     }

     tmp->hq = ch->in_room; /* This depends on ch being in the right room (see above) */
	                        /* This includes IMMs!!! */

     if ( self )
     {
          ch->pcdata->clan = tmp;
          tmp->membercount++;
          ch->pcdata->clrank = RANK_LEADER;
          save_char_obj (ch);
          send_to_char ("You should now use the \"cedit\" command to setup your new clan.\n\r", ch );
     }
     save_clans ( ); // Need full save_clan here, so that it gets added to the list
     return;
}

void do_clandelete( CHAR_DATA *ch, char *argument)
{
     CLAN_INDEX *tmp;
     char buf[MIL];

     if ( IS_NPC (ch) )
          return;

     // Just in case someone messes with interp.c -- this is a dangerous command.
     if ( !IS_IMMORTAL ( ch ) )
     {
          bugf ( "Non-Immortal (%s) got into clan delete. Change interp.c back to the way it was or modify"
                 "do_clandelete to have security checking.\n\r", ch->name );
          return;
     }

     if (argument[0] == '\0')
     {
          send_to_char("Syntax: clandelete <argument>\n\r", ch);
          send_to_char("        Will {Wtoggle{w delete status of specified clan.\n\r", ch);
          return;
     }
     else
          tmp = clan_by_short (argument);

     if (!tmp)
     {
          send_to_char("No such clan found.\n\r", ch);
          return;
     }

     if (tmp->nosave) /* Clan is already set to nosave, ie, will be deleted */
     {
          tmp->nosave = FALSE;
          send_to_char ( "Undeleted specified clan.\n\r", ch );
     }
     else
     {
          tmp->nosave = TRUE;
          send_to_char ( "Specified clan WILL BE DELETED if not toggled by next reboot.\n\r", ch );
     }

     save_clans ( ); /* Need to save all here so that it gets deleted from the list. */

     SNP ( buf, "%s is being deleted! Oh No!\n\r", tmp->clan_name );
     notify_message (ch, NOTIFY_CLANG, TO_ALL, buf );
}

void do_clancharge(CHAR_DATA *ch, char *argument)
{
     DESCRIPTOR_DATA *d;
     CLAN_INDEX      *tmp;
     CHAR_DATA       *victim;
     char             arg2[MAX_INPUT_LENGTH];
     char             buf[MAX_STRING_LENGTH];
     int              a;

     if ( IS_NPC (ch) )
          return;

     argument = one_argument (argument, arg2);

     if (arg2[0] == '\0')
     {
          send_to_char ("Syntax: clancharge <clan> <value>\n\r", ch);
          return;
     }
     else
          tmp = clan_by_short (arg2);

     if (!tmp)
     {
          send_to_char("No such clan found!\n\r", ch );
          return;
     }

     if (!is_number(argument) )
     {
          send_to_char ("Syntax: clancharge <clan> <value>\n\r", ch );
          return;
     }

     a = atoi(argument);

     if ( a == 0 )
     {
          send_to_char ("Planning on making this free?\n\r", ch );
          return;
     }

     if ( a < 1 || a > 1000000000 )
     {
          send_to_char ("Amounts must be between 1 and 1000000000. For more, charge multiple times.\n\r", ch);
          return;
     }

     form_to_char ( ch, "Charging %d coins to %s.\n\r", a, tmp->clan_name );

     // Bad hack to find a member of the clan online to use to get the notify sent
     for ( d = descriptor_list; d; d = d->next )
     {
          if ( d->connected != CON_PLAYING )
               continue;
          victim = d->original ? d->original : d->character;

          if ( victim->pcdata->clan )
          {
               if ( victim->pcdata->clan == tmp )
               {
                    SNP ( buf, "%s has charged your clan %d coins.\n\r", ch->name, a );
                    notify_message (victim, NOTIFY_CLANG, TO_CLAN, buf );
                    break;
               }
          }
     }

     tmp->clanbank -= a;
     save_one_clan ( tmp );

     return;
}

void clan_donate ( CHAR_DATA * ch, char *argument )
{
     char      arg1[MIL];
     char      arg2[MIL];
     char      buf[MIL];
     int       value;

     if ( IS_NPC ( ch ) )
          return;

     if ( !ch->pcdata->clan )
     {
          send_to_char ("You aren't in a clan.\n\r", ch );
          return;
     }

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' )
     {
          send_to_char ( "What would you like to donate?\n\r", ch  );
          return;
     }
     if ( is_number ( arg1 ) )
     {
          value = atoi ( arg1 );

          if ( value <= 0 )
          {
               send_to_char ("You can't make money by donating, sorry.\n\r", ch );
               return;
          }
          if ( value > ch->gold )
          {
               send_to_char ("But you don't have that much gold!\n\r", ch );
               return;
          }
          if ((value >= (0.75*ch->gold)) && value > 10000) /*very generous*/
          {
               send_to_char ("Your large donation is appreciated.\n\r", ch );
          }
          ch->gold -= value;
          ch->pcdata->clan->clanbank += value;

          SNP ( buf, "%s donated %d gold coins.\n\r", ch->name, value );
          notify_message (ch, NOTIFY_CLANG, TO_CLAN, buf );

          save_one_clan ( ch->pcdata->clan );
          return;
     }
     /* Arg1 isn't a number, so what are we donating? */
     else if ( !str_cmp ( arg1, "exp" ) || !str_cmp ( arg1, "xp" ) ) // Experience. Yay!
     {
          int cxp;

          if ( arg2[0] == '\0' || !is_number ( arg2 ) )
          {
               send_to_char ( "Yes, yes, but how MUCH?\n\r", ch );
               return;
          }
          value = atoi ( arg2 );
          if ( value <= 0 )
          {
               send_to_char ( "You can't earn experience by donating, sorry.\n\r", ch );
               return;
          }
          // Can only donate extra EXP from this level.
          cxp = ch->exp - ( ch->level * exp_per_level( ch, ch->pcdata->points) );
          if ( value > cxp )
          {
               form_to_char ( ch, "You may currently only donate up to %d experience points.\n\r", cxp );
               return;
          }
          ch->exp -= value;
          ch->pcdata->clan->experience += value;

          SNP ( buf, "%s donated %d {Gexperience{x.\n\r", ch->name, value );
          notify_message ( ch, NOTIFY_CLANG, TO_CLAN, buf );

          save_one_clan ( ch->pcdata->clan );
          return;
     }
     // Okay not gold and not exp, we must be donating an item.
     else
     {
          OBJ_DATA     *obj;
          CHAR_DATA    *gch;

          // Check the room first, then held
          //
          obj = get_obj_list ( ch, arg1, ch->in_room->contents );
          if ( obj == NULL )
               obj = get_obj_carry ( ch, arg1, NULL );

          if ( obj == NULL )
          {
               send_to_char ( "Couldn't find anything like that here.\n\r", ch );
               return;
          }
          // Okay we got an object, make sure it is suitable.
          if ( IS_SET ( obj->extra_flags, ITEM_NO_SAC ) || obj->owner )
          {
               send_to_char ( "That object cannot be donated.\n\r", ch );
               return;
          }

          if ( obj->in_room )
          {
               for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
                    if ( gch->on == obj )
                    {
                         act ( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
                         return;
                    }
          }

          if ( ( obj->item_type == ITEM_CORPSE_PC) || (obj->item_type == ITEM_CORPSE_NPC ) )
          {
               send_to_char ( "If your clan really needs the dead, take it to them yourself.\n\r", ch );
               return;
          }
          value = UMAX ( 1, obj->level * 4 );

          ch->pcdata->clan->clanbank += value;
          // These are generally very small, incremental amounts.... And will happen often,
          // so we aren't going to save the clan here, otherwise it could thrash the disk.
          form_to_char ( ch, "Your clan earns %d gold from the sale of %s.\n\r",
                         value, obj->short_descr );
          extract_obj ( obj );
          return;
     }
     return;
}

void clan_recognize ( CHAR_DATA *ch, char *argument )
{
     DESCRIPTOR_DATA *d;
     CHAR_DATA     *victim;
     CLAN_INDEX    *tmp;
     CLAN_INDEX    *tmpf;
     char          buf[MIL];

     if ( IS_NPC ( ch ) )
          return;
     if ( argument[0] == '\0' )
     {
          send_to_char ( "You must specify a clan name.\n\r", ch );
          return;
     }
     else
          tmp = clan_by_short (argument);
     if (!tmp)
     {
          send_to_char("No such clan found!\n\r", ch );
          return;
     }
     if ( IS_IMMORTAL ( ch ) )
     {
          if ( !tmp->demiapprove )
               send_to_char ( "Warning! The clan has no Demi-God!\n\r", ch );
          send_to_char ( "Recognizing the clan....\n\r", ch );
          tmp->status = CLAN_RECOGNIZED;
          SNP ( buf, "%s is now Recognized.", tmp->clan_name );
          notify_message ( ch, NOTIFY_CLANG, TO_ALL, buf );
     }
     else
     {
          if ( ch->pcdata->mortal )
          {
               send_to_char ( "Mere mortals cannot sponsor clans.\n\r", ch );
               return;
          }
          else if ( IS_NULLSTR ( tmp->demigod ) )
          {
               send_to_char ( "That clan must request your sponsorship first.\n\r", ch );
               return;
          }
          else if ( ch->level < 50 )
          {
               send_to_char ( "You are not powerful enough yet to sponsor a clan.\n\r", ch );
               return;
          }
          else if ( !str_cmp ( ch->name, tmp->demigod ) )
          {
               send_to_char ( "It is not you whom that clan offers its alliegiance.\n\r", ch );
               return;
          }
          for ( tmpf = clan_list; tmpf != NULL; tmpf = tmpf->next )
          {
               if ( !str_cmp ( tmpf->demigod, ch->name ) && tmpf != tmp )
               {
                    send_to_char ( "You are already sponsoring another clan. Sorry.\n\r", ch );
                    return;
               }
          }
          tmp->demiapprove = TRUE;
          send_to_char ( "You have now recognized this clan as your subjects.\n\r", ch );

          /* Try to find an online member of the clan */

          for ( d = descriptor_list; d; d = d->next )
          {
               if ( d->connected != CON_PLAYING )
                    continue;
               victim = d->original ? d->original : d->character;

               if ( victim->pcdata->clan )
               {
                    if ( victim->pcdata->clan == tmp )
                    {
                         SNP ( buf, "%s has consented to be your Clan Demi-God.\n\r", ch->name );
                         notify_message (victim, NOTIFY_CLANG, TO_CLAN, buf );
                         break;
                    }
               }
          }
     }
     save_one_clan ( tmp );
     return;
}

// Sigh... Paging breaks the display badly....
void cedit_show ( CHAR_DATA *ch, char *argument )
{
     BUFFER       *buffer;
     CLAN_INDEX   *ed = NULL;
     int           i;

     if ( IS_NPC (ch) )
          return;

     buffer = buffer_new(1024);
     ed = ch->pcdata->cedit; // To save typing :)

     bprintf ( buffer, VT_CLS );
     bprintf ( buffer, "{c----{C[ {WGeneral Clan Information {C]{c----\n\r" );
     bprintf ( buffer, "Long Name    {W: {w%s\n\r", ed->clan_name );
     bprintf ( buffer, "Short Name   {W: {w%s (Unchangeable)\n\r", ed->clan_short );
     if ( ( ch->pcdata->clrank >= RANK_LEADER) || (IS_IMMORTAL(ch) ) )
     {
          bprintf ( buffer, " (You have access to change this Clan's Long Name and Ranks)\n\r");
          bprintf ( buffer, "Rank Names, Highest to Lowest:\n\r" );
          bprintf ( buffer, "    {B%20s    {M%20s{w\n\r", "Male Ranks", "Female Ranks" ); // Yeah it's odd, sue me
          for (i = 5 ; i >= 0 ; i-- )   // Yeah it's backwards, sue me.
               bprintf(buffer, "%d   {B%20s    {M%20s{w\n\r", i, ed->mranks[i], ed->franks[i] );
     }
     bprintf ( buffer, "\n\r");
     if ( IS_IMMORTAL ( ch ) )
          bprintf ( buffer, "Portal: {C[{w%5d{C]{w  ", ed->portalto );
     if ( ( IS_IMMORTAL ( ch ) ) || (ch->pcdata->clrank >= ed->rank_settax) )
          bprintf ( buffer, "Clan Tax: {C[{w%5d{C]{w", ed->clanmtax );
     bprintf ( buffer, "\n\r" );

     if ( IS_IMMORTAL ( ch ) || ch->pcdata->clrank >= RANK_LEADER )
     {
          bprintf ( buffer, "AllowPK: {C[%s{C] {W({YIrreversible{W){w\n\r",
                    ed->pkallow ? "{RYes" : "{GNo" );
          bprintf ( buffer, "DemiGod: {C[{w%s{C] {C({wChange requires Review{C){w\n\r",
                    !IS_NULLSTR ( ed->demigod) ? ed->demigod : "Nobody" );
     }
     if ( ( IS_IMMORTAL ( ch ) ) || (ch->pcdata->clrank >= ed->rank_setjoin) )
     {
          bprintf ( buffer, "{c----{C[ {WJoining Requirements {C]{c----{w\n\r" );
          bprintf ( buffer, "Level: {C[{w%3d{C]{w  Cost {C[{w%10d{C]{w  Align {C[{r%5d{C]{w to {C[{W%5d{C]{w\n\r",
                    ed->join_level, ed->join_cost, ed->join_minalign, ed->join_maxalign );
     }
     if ( ch->pcdata->clrank == RANK_LEADER )
     {
          bprintf ( buffer, "\n\rSet Rank Names with mrank x <name>, or frank x <name>, where x is the number in the column to the left.\n\r");
          bprintf ( buffer, "\n\rAlso, to view {Wrank-based{w security, type \"ranks\".\n\r" );
     }
     send_to_char ( buffer->data, ch );
     buffer_free ( buffer );
}

void do_cedit ( CHAR_DATA *ch, char *argument )
{

     if ( IS_NPC ( ch ) )
          return;

     if ( argument[0] == '\0' || !IS_IMMORTAL (ch) )
     {
          if ( !ch->pcdata->clan )   /* Boot em' out without a message. That'll teach 'em. */
          {
               send_to_char ( "Type {YCOMMANDS{w for a list of valid commands.\n\r", ch);
               return;
          }

		  /* Make sure the person has some business being in the clan editor. */

          if ( ( !IS_IMMORTAL (ch) ) && ( ch->pcdata->clrank < ch->pcdata->clan->rank_setjoin)
               && ( ch->pcdata->clrank < ch->pcdata->clan->rank_settax )
               && ( ch->pcdata->clrank < ch->pcdata->clan->rank_move )
               && ( ch->pcdata->clrank < RANK_LEADER ) )
          {
               send_to_char ( "Type {YCOMMANDS{w for a list of valid commands.\n\r", ch );
               return;
          }

		  /* Okay... I think we can assume it's safe to let the person into the clan editor. */
		  /* For now let's just edit our own clan. */

          ch->pcdata->cedit = ch->pcdata->clan;
     }
     else
     {
          ch->pcdata->cedit = clan_by_short (argument);

          if (!ch->pcdata->cedit)
          {
               send_to_char("No such clan found!\n\r", ch );
               return;
          }

     }

     cedit_show ( ch, "" );
     send_to_char ( "\n\r{YClan Editor Commands {W:: {w", ch );

     ch->desc->connected = CON_EDIT_CLAN;
     return;
}

void handle_edit_clan ( DESCRIPTOR_DATA *d, char *argument )
{
     int          i;
     char         buf[MIL];
     char         arg2[MAX_INPUT_LENGTH];
     char         arg[MAX_INPUT_LENGTH];
     char         *argunbroke;
     char         *argorig;
     bool         changes = FALSE;
     bool         cMatch = FALSE;
     BUFFER       *buffer;
     CHAR_DATA    *ch = d->character;
     CHAR_DATA    *victim;
     CLAN_INDEX   *ed = NULL;
     CLAN_INDEX   *tmp = NULL;

     if ( !ch->pcdata->cedit)
     {
          bugf ( "%s in clanedit without a cedit pointer?", ch->name );
          return;
     }

     ed = ch->pcdata->cedit; // To save typing :)

     argorig = str_dup ( argument ); // In case we need to pass it to interpret
     argument = one_argument ( argument, arg );
     argunbroke = str_dup ( argument ); // For the couple of times below you need an unbroken argument
     argument = one_argument ( argument, arg2 );

	 /* arg is the function we're calling, argument is the data for it */

	 /*
	  * You only get to see what you can change here.
	  */

     if ( arg[0] == '\0' ) // Blank argument, let's call this "show"
     {
          cMatch = TRUE;
          cedit_show (ch, "");
          send_to_char ( "\n\r{YClan Editor Commands {W:: {w", ch );
     }
     else
     {
          buffer = buffer_new ( 1024 );
          cMatch = FALSE;
          switch ( tolower (arg[0]) )
          {
          case 'r':
               if ( !str_cmp ( arg, "ranks" ) )
               {
                    cMatch = TRUE;
                    if ( ( ch->pcdata->clrank < RANK_LEADER) && (!IS_IMMORTAL(ch) ) )
                    {
                         bprintf (buffer, "Sorry, only those of the highest rank may view this field.\n\r");
                         break;
                    }
                    bprintf ( buffer, VT_CLS );
                    bprintf ( buffer, "----[ Security ]----\n\r" );
                    bprintf ( buffer, "Minimum Ranks to perform actions:\n\r" );
                    bprintf ( buffer, "(A) Set Join Costs  : %s\n\r", ed->mranks[ed->rank_setjoin] );
                    bprintf ( buffer, "(B) Recruit Members : %s\n\r", ed->mranks[ed->rank_recruit] );
                    bprintf ( buffer, "(C) Outcast Members : %s\n\r", ed->mranks[ed->rank_outcast] );
                    bprintf ( buffer, "(D) Promote Members : %s\n\r", ed->mranks[ed->rank_promote] );
                    bprintf ( buffer, "(E) Demote Members  : %s\n\r", ed->mranks[ed->rank_demote] );
                    bprintf ( buffer, "(F) Set Tax Rate    : %s\n\r", ed->mranks[ed->rank_settax] );
                    bprintf ( buffer, "(G) Declare Wars    : %s\n\r", ed->mranks[ed->rank_declare] );
                    bprintf ( buffer, "(H) Claim Area      : %s\n\r", ed->mranks[ed->rank_claim] );
                    bprintf ( buffer, "(I) Set Bounty      : %s\n\r", ed->mranks[ed->rank_bounty] );
                    bprintf ( buffer, "(J) Can Recall      : %s\n\r", ed->mranks[ed->rank_recall] );
                    bprintf ( buffer, "(K) Move Hold       : %s\n\r", ed->mranks[ed->rank_move] );
                    bprintf ( buffer, "\n\rTo change a rank, enter the LETTER of the rank to change, and a number.\n\r"
                              "The number should be from 1 to 5, with 1 being second lowest and 5 being highest\n\r"
                              "ranks. The bottom rank is, of course, 0, however, those of this rank cannot ever\n\r"
                              "perform these actions.\n\r\n\r" );
               }
               break;
          case 'a':
               if ( !str_cmp ( arg, "a" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_setjoin = atoi(arg2);
                         bprintf ( buffer, "Set Join Costs :: %s", ed->mranks[ed->rank_setjoin] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               else if ( !str_cmp ( arg, "autoaccept" ) )
               {
                    cMatch = TRUE;
                    if ( ed->autoaccept )
                    {
                         ed->autoaccept = FALSE;
                         bprintf ( buffer, "Turning off AUTOACCEPT. New members will have to be manually accepted now.\n\r" );
                         changes = TRUE;
                    }
                    else
                    {
                         ed->autoaccept = TRUE;
                         bprintf ( buffer, "Autoaccept turned on, those who petition will be automatically accepted now.\n\r" );
                         changes = TRUE;
                    }
               }
               else if ( !str_cmp ( arg, "align" ) || !str_cmp ( arg, "alignment" ) )
               {
                    cMatch = TRUE;
                    if ( arg2[0] == '\0' || argument[0] == '\0' )
                    {
                         bprintf ( buffer, "Syntax for alignment is :: alignment <min> <max>\n\r" );
                         break;
                    }
                    if ( ENTRE(-1001,atoi(arg2),1001) && ENTRE(-1001,atoi(argument),1001)) // Between -1000 and +1000
                    {
                         if ( atoi(arg2) >= atoi(argument) )
                         {
                              bprintf ( buffer, "Minimum alignment must be less than Maximum alignment.\n\r" );
                              break;
                         }
                         ed->join_minalign = atoi (arg2);
                         ed->join_maxalign = atoi (argument);
                         bprintf ( buffer, "Alignment Requirements :: %d Min, %d Max",
                                   ed->join_minalign, ed->join_maxalign );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Alignment must be between -1000 and 1000.\n\r" );
               }
               else if ( !str_cmp ( arg, "allowpk" ) )
               {
                    cMatch = TRUE;
                    if ( ed->status == CLAN_NEW )
                    {
                         bprintf ( buffer, "New clans may not enable PK or Warfare. You must be recognized, or go underground, first.\n\r" );
                         break;
                    }
                    if ( ed->pkallow )
                    {
                         bprintf ( buffer, "Your clan is already PK Enabled. You may not remove this flag.\n\r" );
                         break;
                    }
                    if ( arg2[0] == '\0' || str_cmp ( arg2, "ireallymeanit" ) ) // No ! here, this means it doesn't match
                    {
                         bprintf ( buffer, "To enbable PK and WarFare in your clan, you must type this command as follows:\n\r" );
                         bprintf ( buffer, "\n\rallowpk ireallymeanit\n\r\n\r" );
                         bprintf ( buffer, "By doing so, you indicate that you understand this change is {R{&IRREVERSIBLE.{x.\n\r" );
                         break;
                    }
                    ed->pkallow = TRUE;
                    bprintf ( buffer, "Good luck! It's a tough world out there!\n\r" );
                    if ( ed->status >= CLAN_RECOGNIZED )
                    {
                         SNP ( buf, "%s has announced a willingness to engage in the rough and tumble.", ed->clan_name );
                         notify_message ( ch, NOTIFY_CLANG, TO_ALL, buf );
                    }
                    else
                    {
                         SNP ( buf, "The leaders of %s meet behind closed doors to plan for warfare.", ed->clan_name );
                         notify_message ( ch, NOTIFY_CLANG, TO_CLAN, buf );
                    }
                    changes = TRUE;
                    break;
               }
               break;
          case 'b':
               if ( !str_cmp ( arg, "b" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_recruit = atoi(arg2);
                         bprintf ( buffer, "Recruit :: %s", ed->mranks[ed->rank_recruit] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'c':
               if ( !str_cmp ( arg, "commands" ) )
               {
                    cMatch = TRUE;
                    bprintf ( buffer, "Valid Commands in the Clan Editor (You may not have access to all of these):\n\r"
                              "done, frank, mrank, quit, ranks, A-K, cost, align,\n\r"
                              "allowpk, commands, demigod, name, level, autojoin\n\r");
               }
               else if ( !str_cmp ( arg, "c" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_outcast = atoi(arg2);
                         bprintf ( buffer, "Outcast :: %s", ed->mranks[ed->rank_outcast] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               else if ( !str_cmp ( arg, "cost" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(-1,atoi(arg2),100000000)) // Hopefully no clan will ever cost that much to join
                    {
                         ed->join_cost = atoi(arg2);
                         bprintf ( buffer, "Join Cost :: %d", ed->join_cost );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Cost out of range.\n\r" );
               }
               break;
          case 'd':
               if ( !str_cmp ( arg, "done" ) )
               {
                    cMatch = TRUE;
                    bprintf ( buffer, "Exiting Editor.\n\r" );
                    ch->desc->connected = CON_PLAYING;
                    ed = NULL;
                    ch->pcdata->cedit = NULL;
               }
               else if ( !str_cmp ( arg, "demigod" ) )
               {
                    bool already = FALSE;
                    cMatch = TRUE;

                    if ( ( victim = get_char_world ( ch, arg2 ) ) == NULL )
                    {
                         bprintf ( buffer, "I can't find anyone in the realm by that name.\n\r" );
                         break;
                    }
                    for ( tmp = clan_list; tmp != NULL; tmp = tmp->next )
                    {
                         if ( !str_cmp ( tmp->demigod, victim->name ) )
                         {
                              already = TRUE;
                         }
                    }
                    if ( already )
                    {
                         bprintf ( buffer, "That Demi-God has already sponsored a clan.\n\r" );
                         break;
                    }
                    if ( victim->level < 50 )
                    {
                         bprintf ( buffer, "A Demi-God of that low level would not make a wise counsel.\n\r"
                                   "Perhaps one level 50 or higher would do better?\n\r" );
                         break;
                    }
                    free_string ( ed->demigod );
                    ed->demigod = str_dup ( victim->name );
                    SNP ( buf, "%s has been asked to be the new Clan Demi-God.", victim->name );
                    notify_message ( ch, NOTIFY_CLANG, TO_CLAN, buf );
                    SNP ( buf, "%s requests you to become the Demi-God for %s.\n\r"
                              "To do so, type \"recognize %s\" at your prompt, if you approve of the clan.\n\r",
                              ch->name, ed->clan_name, ed->clan_short );
                    send_to_char ( buf, victim );
                    changes = TRUE;
                    break;
               }
               else if ( !str_cmp ( arg, "d" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_promote = atoi(arg2);
                         bprintf ( buffer, "Promote :: %s", ed->mranks[ed->rank_promote] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'e':
               if ( !str_cmp ( arg, "e" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_demote = atoi(arg2);
                         bprintf ( buffer, "Demote :: %s", ed->mranks[ed->rank_demote] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'f':
               if ( !str_cmp ( arg, "f" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_settax = atoi(arg2);
                         bprintf ( buffer, "Set Tax :: %s", ed->mranks[ed->rank_settax] );
                         changes = TRUE;
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               // NO BREAK... 'f': must pass straight to 'm': as the very next case!!!!!!!!!!! - Lotherius
          case 'm':
               if ( ( !str_cmp ( arg, "mrank" ) || !str_cmp ( arg, "frank" ) ) )
               {
                    cMatch = TRUE;
                    if ( ( ch->pcdata->clrank < RANK_LEADER) && (!IS_IMMORTAL(ch) ) )
                    {
                         bprintf (buffer, "Sorry, only those of the highest rank may change this field.\n\r");
                         break;
                    }
                    if ( !is_number ( arg2 ) )
                    {
                         bprintf (buffer, "Please use 'mrank x <name>' where x is a number.\n\r" );
                         break;
                    }
                    i = atoi ( arg2 );
                    if ( (i < 0 ) || ( i > 5 ) )
                    {
                         bprintf ( buffer, "Number must be from 0 (lowest rank) to 5 (highest rank).\n\r" );
                         break;
                    }
                    if ( ( strlen (argument) > 18 ) || (strlen (argument) < 4 ) )
                    {
                         bprintf ( buffer, "Rank must be from 4 to 18 characters in length.\n\r" );
                         break;
                    }
                    /* We're going to recheck our arg, to remember if we're doing male or female */
                    if ( !str_cmp ( arg, "mrank" ) )
                         ed->mranks[i] = str_dup (argument);
                    else
                         ed->franks[i] = str_dup (argument);
                    changes = TRUE;
                    bprintf ( buffer, "Rank set.\n\r" );
                    if ( ed->status == CLAN_RECOGNIZED )
                    {
                         bprintf ( buffer, "Marking your clan for Immortal Review to approve the new title.\n\r" );
                         ed->status = CLAN_RENEW;
                    }
                    break;
               }
               break;
          case 'n':
               if ( !str_cmp ( arg, "name" ) )
               {
                    cMatch = TRUE;
                    if ( argunbroke[0] == '\0' )
                    {
                         bprintf ( buffer, "Please specify a name, no longer than 20 characters and no less than 5.\n\r" );
                         break;
                    }
                    if ( ( cstrlen ( argunbroke ) > 20 ) || ( strlen ( argunbroke ) < 5 ) )
                    {
                         bprintf ( buffer, "The clan's name can be no longer than 20 characters, and no less than 5.\n\r" );
                         break;
                    }
                    // Go ahead and change the name.
                    free_string ( ed->clan_name );
                    ed->clan_name = str_dup ( argunbroke );
                    if ( ed->status == CLAN_RECOGNIZED )
                    {
                         bprintf ( buffer, "Marking your clan for Immortal Review to approve the new title.\n\r" );
                         ed->status = CLAN_RENEW;
                    }
                    // Check here in case it is an imm changing it
                    if ( ch->pcdata->clan == ed )
                    {
                         SNP ( buf, "New Clan Logo: %s{w", ed->clan_name );
                         notify_message ( ch, NOTIFY_CLANG, TO_CLAN, buf );
                    }
                    changes = TRUE;
                    break;
               }
          case 'g':
               if ( !str_cmp ( arg, "g" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_declare = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Declare War :: %s", ed->mranks[ed->rank_declare] );
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'h':
               if ( !str_cmp ( arg, "h" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_claim = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Claim :: %s", ed->mranks[ed->rank_claim] );
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'i':
               if ( !str_cmp ( arg, "i" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_bounty = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Set Bounties :: %s", ed->mranks[ed->rank_bounty] );
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'j':
               if ( !str_cmp ( arg, "j" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_recall = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Can Recall :: %s", ed->mranks[ed->rank_recall] );
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'k':
               if ( !str_cmp ( arg, "k" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(0,atoi(arg2),6 )) // Range from 1 to 5
                    {
                         ed->rank_move = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Move Stronghold :: %s", ed->mranks[ed->rank_move] );
                    }
                    else
                         bprintf ( buffer, "Range must be from 1 to 5. 1 being second lowest, 5 being highest.\n\r" );
               }
               break;
          case 'l':
               if ( !str_cmp ( arg, "level" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(4,atoi(arg2),(LEVEL_HERO+1) ) || IS_IMMORTAL ( ch )) // Level range 5 to 101
                    {
                         ed->join_level = atoi(arg2);
                         changes = TRUE;
                         bprintf ( buffer, "Join Level :: %d", ed->join_level );
                    }
                    else
                         bprintf ( buffer, "Range is from level 1 to %d", LEVEL_HERO );
               }
               break;
          case 'q':
               if ( !str_cmp ( arg, "quit" ) ) // Special case, we have to clean up for ourselves here
               {
                    cMatch = TRUE;
                    buffer_free ( buffer );
                    send_to_char ( "Quitting....\n\r", ch );
                    do_quit ( ch, "" );
                    return;
               }
               break;
          case 't':
               if ( !str_cmp ( arg, "tax" ) )
               {
                    cMatch = TRUE;
                    if ( ENTRE(-1,atoi(arg2),101 ) )
                    {
                         ed->clanmtax = atoi(arg2);
                         changes = TRUE;
                         SNP ( buf, "Tax Rate : %d", ed->clanmtax );
                         notify_message ( ch, NOTIFY_CLANG, TO_CLAN, buf );
                         break;
                    }
                    else
                         bprintf ( buffer, "Valid range is 0 to 100.\n\r" );
               }
               break;
          }
          if ( !cMatch )
          {
               // I don't interpret here because handling the prompt doesn't work well...
               //		  interpret ( ch, argorig );
               bprintf ( buffer, "\n\rIf you need help, type \"commands\" for a list of valid cedit commands.\n\r" );
          }
          else
          {
               if ( changes )
                    save_one_clan ( ed );
          }
          if ( ed )                  // Make sure we're still editing
               bprintf ( buffer, "\n\r{YClan Editor Commands {W:: {w" );
          send_to_char ( buffer->data, ch );
          buffer_free ( buffer );
     }
     free_string ( argunbroke );
     free_string ( argorig );
     return;
}

