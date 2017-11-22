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

/*
 * This File includes non-spell skill functions.
 */

#include "everything.h"
#include "magic.h"

DECLARE_DO_FUN ( do_say );

/* command procedures */

/* Zeran - altered to work with skillmaster
 * reason should be SKILL_flag
 */

bool skill_available ( int sn, CHAR_DATA * ch, int reason, CHAR_DATA * mob )
{
     int                 i;
     int                 level;
     
     if ( IS_NPC ( ch ) )
          return FALSE;		/* Currently not setup for NPC use */

     for ( i = 0; i < MAX_CLASS; i++ )
     {
          if ( ( pc_race_table[ch->pcdata->pcrace].skills[i] == NULL )
               || ( sn < 0 ) )
               break;
          if ( sn == ( skill_lookup( pc_race_table[ch->pcdata->pcrace].skills[i] ) ) )
          {
               return TRUE;
          }
     }

    /* Zeran - altered slightly for skillmaster stuff */

     level = skill_table[sn].skill_level[ch->pcdata->pclass];
     switch ( reason )
     {
     case SKILL_AVAIL:
          {
               if ( ch->level >= level )
                    return TRUE;
               return FALSE;
          }
     case SKILL_PRAC:
          {
               if ( ch->level < level )
                    return FALSE;
               if ( level >= 101 )
               {
                    send_to_char ( "You must seek a true master to learn that.\n\r", ch );
                    return FALSE;
               }
               return TRUE;
          }
     case SKILL_LEARN:
          {
               if ( ch->level < level )
                    return FALSE;
               if ( level < 101 )
               {
                    send_to_char ( "Your guildmaster would be a better choice.\n\r", ch );
                    return FALSE;
               }
               if ( !is_skillmaster_skill ( mob, sn ) )
               {
                    do_say ( mob, "I do not have the knowledge to teach you that." );
                    return FALSE;
               }
               return TRUE;
          }
     default:
          {
               bugf ( "Invalid reason code to skill_available" );
               return FALSE;
          }
     }
     /* end switch */
}

/* Zeran - added this to call from do_practice and do_learn */
void show_current_prac ( CHAR_DATA * ch )
{
     int                 col = 0;
     int                 sn;
     BUFFER		*buffer;

     buffer = buffer_new(1000);

     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( skill_table[sn].name == NULL )
               break;
          if ( skill_available ( sn, ch, SKILL_AVAIL, NULL ) == FALSE )
               continue;
          
          bprintf ( buffer, "%-18s %3d%%  ", skill_table[sn].name, ch->pcdata->learned[sn] );
          
          if ( ++col % 3 == 0 )
               bprintf (buffer, "\n\r" );
     }
     if ( col % 3 != 0 )
          bprintf( buffer, "\n\r");
     bprintf ( buffer, "You have %d practice sessions left.\n\r", ch->pcdata->practice );
     page_to_char ( buffer->data, ch );

     buffer_free(buffer);

     return;
}

bool is_skillmaster_skill ( CHAR_DATA * mob, int sn )
{
     int                 count;
     MOB_INDEX_DATA     *mIndex;

     mIndex = mob->pIndexData;
     for ( count = 0; count < mIndex->total_teach_skills; count++ )
          if ( skill_lookup ( mIndex->teach_skills[count] ) == sn )
               return TRUE;
     return FALSE;
}

void show_skillmaster_skills ( CHAR_DATA * ch, CHAR_DATA * mob )
{
     int                 count;
     int                 total;

     total = mob->pIndexData->total_teach_skills;
     /* List the skills */
     do_say ( mob, "I have knowledge of the following:" );
     for ( count = 0; count < total; count++ )
          do_say ( mob, mob->pIndexData->teach_skills[count] );
     return;
}

int exp_per_level ( CHAR_DATA * ch, int points )
{
     int tolev;

     if ( IS_NPC ( ch ) )
          return 1000;
     tolev = 1200 + (points * 100);
     if (ch->pcdata->mortal)
          return tolev;
     else
          return (tolev / 4 ) + 7700;
}

/* checks for skill improvement */
void check_improve ( CHAR_DATA * ch, int sn, bool success, int multiplier )
{
     int                 chance;

     if ( IS_NPC ( ch ) )
          return;

     if ( ch->level < skill_table[sn].skill_level[ch->pcdata->pclass]
          || ch->pcdata->learned[sn] == 0
          || ch->pcdata->learned[sn] == 100 )
          return;                 /* skill is not known or is max */

    /* check to see if the character has a chance to learn */
     chance = 10 * int_app[get_curr_stat ( ch, STAT_INT )].learn;
     chance /= ( multiplier * 4 );
     chance += ch->level;

     if ( number_range ( 1, 1000 ) > chance )
          return;

    /* now that the character has a CHANCE to learn, see if they really have */

     if ( success )
     {
          chance = URANGE ( 5, 100 - ch->pcdata->learned[sn], 95 );
          if ( number_percent (  ) < chance )
          {
               form_to_char ( ch, "You have become better at %s!\n\r", skill_table[sn].name );
               ch->pcdata->learned[sn]++;
               gain_exp ( ch, 4 );
          }
     }
     else
     {
          chance = URANGE ( 5, ch->pcdata->learned[sn] / 2, 30 );
          if ( number_percent (  ) < chance )
          {
               form_to_char ( ch, "You learn from your mistakes, and your %s skill improves.\n\r",
                              skill_table[sn].name );
               ch->pcdata->learned[sn] += number_range ( 1, 3 );
               ch->pcdata->learned[sn] =
                    UMIN ( ch->pcdata->learned[sn], 100 );
               gain_exp ( ch, 3 );
          }
     }
     return;
}

void do_skills ( CHAR_DATA * ch, char *argument )
{
     char                skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
     char                skill_columns[LEVEL_HERO];
     int                 sn, lev, i, filter;
     bool                found = FALSE;
     char                buf[MAX_STRING_LENGTH];
     BUFFER              *outbuf;

     if ( IS_NPC ( ch ) )
          return;

     outbuf = buffer_new(1000);

     if ( argument == NULL || argument[0] == '\0' ) 	/* This character's own list. Normal. */
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
               bprintf( outbuf, "Skills For %s", class_table[filter].name );
          }
     }

     /* initilize data */
     for ( lev = 0; lev < LEVEL_HERO; lev++ )
     {
          skill_columns[lev] = 0;
          skill_list[lev][0] = '\0';
     }

     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( skill_table[sn].name == NULL )
               break;

          if ( ( ( (filter == -1) && ( skill_available ( sn, ch, SKILL_AVAIL, NULL ) == TRUE ) ) ||
                 ( (filter >= 0) &&  (skill_table[sn].skill_level[filter] <= 101) ) )
               && skill_table[sn].spell_fun == spell_null )
          {
               found = TRUE;

               if (filter == -1)
                    lev = skill_table[sn].skill_level[ch->pcdata->pclass];
               else
                    lev = skill_table[sn].skill_level[filter];

               if (filter == -1)
               {
                    for ( i = 0; i < 5; i++ )
                    {
                         if ( ( pc_race_table[ch->pcdata->pcrace].skills[i] == NULL ) || ( sn < 0 ) )
                              break;
                         if ( sn == ( skill_lookup ( pc_race_table[ch->pcdata->pcrace].skills[i] ) ) )
                              lev = 1;
                    }
               }

               if ( ch->level < lev )
                    SNP ( buf, "%-18s n/a      ", skill_table[sn].name );
               else
                    SNP ( buf, "%-18s %3d%%      ", skill_table[sn].name, ch->pcdata->learned[sn] );
               if ( skill_list[lev][0] == '\0' )
                    SNP ( skill_list[lev], "\n\rLevel %2d: %s", lev, buf );
               else                /* append */
               {
                    if ( ++skill_columns[lev] % 2 == 0 )
                         SLCAT ( skill_list[lev], "\n\r          " );
                    SLCAT ( skill_list[lev], buf );
               }
          }
     }
     /* return results */
     if ( !found )
     {
          send_to_char ( "No skills found.\n\r", ch );
     }
     else
     {
          for ( lev = 0; lev < LEVEL_HERO; lev++ )
          {
               if ( skill_list[lev][0] != '\0' )
                    buffer_strcat ( outbuf, skill_list[lev] );
          }
          buffer_strcat ( outbuf, "\n\r" );
          page_to_char ( outbuf->data, ch );
     }

     buffer_free(outbuf);
     return;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup ( const char *name )
{
     int                 sn;
     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( skill_table[sn].name == NULL )
               break;
          if ( LOWER ( name[0] ) == LOWER ( skill_table[sn].name[0] )
               && !str_prefix ( name, skill_table[sn].name ) )
               return sn;
     }
     return -1;
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup ( int slot )
{
     extern bool         fBootDb;
     extern bool	 fImportDb;
     int                 sn;

     if ( slot <= 0 )
          return -1;

     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( slot == skill_table[sn].slot )
               return sn;
     }

     if ( fBootDb || fImportDb)
     {
          bugf ( "Slot_lookup: bad slot %d.", slot );
          abort (  );
     }

     return -1;
}

