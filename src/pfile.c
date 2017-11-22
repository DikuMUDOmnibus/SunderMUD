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


extern int _filbuf args ((FILE *));
extern bool fCopyOver;

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char args ((CHAR_DATA * ch, FILE * fp));
void fwrite_obj args  ((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest));
void fwrite_pet args  ((CHAR_DATA * pet, FILE * fp));
void fread_char args  ((CHAR_DATA * ch, FILE * fp));
void fread_pet args   ((CHAR_DATA * ch, FILE * fp));
void fread_obj args   ((CHAR_DATA * ch, FILE * fp));

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj (CHAR_DATA * ch)
{
     char strsave[MAX_INPUT_LENGTH];
     char buf[MAX_STRING_LENGTH];
     FILE *fp;

     if (IS_NPC (ch))
          return;

     if (!ch)
     {
          bugf ("save_char_obj: Trying to save thin air.");
          return;
     }

     if (ch->desc != NULL && ch->desc->original != NULL)
          ch = ch->desc->original;

     if (ch->level == 0)
          return;
	 /* We don't wanna save folks during creation. Sowwy. */

	 /* create god log */
     if (IS_HERO (ch) || ch->level >= LEVEL_HERO)
     {
          fclose (fpReserve);
          SNP (strsave, "%s%s", GOD_DIR, capitalize (ch->name));
          if ((fp = fopen (strsave, "w")) == NULL)          
               bugf ("Save_char_obj: couldn't open %s", strsave );          

          fprintf (fp, "Lev %2d Trust %2d  %s%s\n",
                   ch->level, get_trust (ch), ch->name, ch->pcdata->title);
          fclose (fp);
          fpReserve = fopen (NULL_FILE, "r");
     }
     fclose (fpReserve);
     SNP (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
     if ((fp = fopen (PLAYER_TEMP, "w")) == NULL)     
          bugf ("Save_char_obj: couldn't open %s", strsave );     
     else
     {
          fwrite_char (ch, fp);
          if (ch->carrying != NULL)
               fwrite_obj (ch, ch->carrying, fp, 0);
		  /* save the pets */
          if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
               fwrite_pet (ch->pet, fp);
          fprintf (fp, "#END\n");
     }
     fclose (fp);

	 /* move the file */

#if !defined(WIN32)
     umask (006);

     SNP (buf, "mv %s %s", PLAYER_TEMP, strsave);
     system (buf);
# if defined(COMPRESS_PFILES)
     SNP (buf, "gzip -fq %s", strsave);
     system (buf);
# endif
#else
     SNP (buf, "copy %s %s", PLAYER_TEMP, strsave);
     system (buf);
     SNP (buf, "del %s", PLAYER_TEMP );
     system ( buf );
#endif
     fpReserve = fopen (NULL_FILE, "r");
     return;
}

/*
 * Write the char.
 */
void fwrite_char (CHAR_DATA * ch, FILE * fp)
{
     AFFECT_DATA *paf;
     int sn, i;

     fprintf (fp, "#%s\n", IS_NPC (ch) ? "MOB" : "PLAYER");

     fprintf (fp, "Name %s~\n", ch->name);
     fprintf (fp, "Vers %d\n", 7);
     fprintf (fp, "Mort %d\n", ch->pcdata->mortal);
     if (ch->short_descr[0] != '\0')
          fprintf (fp, "ShD  %s~\n", ch->short_descr);
     if (ch->long_descr[0] != '\0')
          fprintf (fp, "LnD  %s~\n", ch->long_descr);
     if (ch->description[0] != '\0')
          fprintf (fp, "Desc %s~\n", ch->description);
     if (ch->short_descr_orig[0] != '\0')
          fprintf (fp, "ShDOrig  %s~\n", ch->short_descr_orig);
     if (ch->long_descr_orig[0] != '\0')
          fprintf (fp, "LnDOrig  %s~\n", ch->long_descr_orig);
     if (ch->description_orig[0] != '\0')
          fprintf (fp, "DescOrig %s~\n", ch->description_orig);
     if (ch->poly_name[0] != '\0')
          fprintf (fp, "Polyname %s~\n", ch->poly_name);
     fprintf (fp, "Race %s~\n", pc_race_table[ch->pcdata->pcrace].name);
     fprintf (fp, "Sex  %d\n", ch->sex);
     fprintf (fp, "Cla  %d\n", ch->pcdata->pclass);
     fprintf (fp, "Levl %d\n", ch->level);
     fprintf (fp, "VM %d\n", ch->pcdata->mvolume);
     fprintf (fp, "VS %d\n", ch->pcdata->svolume);
     fprintf (fp, "Bank %ld\n", ch->pcdata->bankaccount );
     if (ch->pcdata->questpoints != 0)
          fprintf (fp, "QuestPnts %ld\n", ch->pcdata->questpoints);
     if (ch->pcdata->questearned != 0)
          fprintf (fp, "QuestErnd %ld\n", ch->pcdata->questearned);
     if (ch->pcdata->nextquest != 0)
          fprintf (fp, "QuestNext %d\n", ch->pcdata->nextquest);
     else if (ch->pcdata->countdown != 0)
          fprintf (fp, "QuestNext %d\n", 10);
     if (ch->trust != 0)
          fprintf (fp, "Tru  %d\n", ch->trust);
     fprintf (fp, "Sec  %d\n", ch->pcdata->security);	/* OLC */
     if (ch->recall_perm)
          fprintf (fp, "Recall %d\n", ch->recall_perm);
     fprintf (fp, "Home	 %d\n", ch->pcdata->home_room);
     fprintf (fp, "Speaking %s~\n", ch->speaking);
     fprintf (fp, "Clrank %d\n", ch->pcdata->clrank);
     fprintf (fp, "Plyd %d\n", ch->played + (int) (current_time - ch->logon));
     fprintf (fp, "Brating %d\n", ch->pcdata->battle_rating);
     if (ch->pcdata->clan)
          fprintf (fp, "Clan %s~\n", ch->pcdata->clan->clan_short);
     fprintf (fp, "Mrate %ld\n", ch->pcdata->mob_rating);
     fprintf (fp, "Pkwin %d\n", ch->pcdata->pkill_wins);
     fprintf (fp, "Pkloss %d\n", ch->pcdata->pkill_losses);
     fprintf (fp, "Mwin %ld\n", ch->pcdata->mob_wins);
     fprintf (fp, "Mloss %ld\n", ch->pcdata->mob_losses);
     fprintf (fp, "Note %ld\n", ch->last_note);
     fprintf (fp, "Scro %d\n", ch->lines );
     fprintf (fp, "Room %d\n",
              (ch->in_room == get_room_index (ROOM_VNUM_LIMBO)
               && ch->was_in_room != NULL) ? ch->was_in_room->vnum : ch->in_room == NULL
              ? ch->recall_perm : ch->in_room->vnum);
     fprintf (fp, "HMV  %d %d %d %d %d %d\n",
              ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
     if (ch->gold > 0)
          fprintf (fp, "Gold %ld\n", ch->gold);
     else
          fprintf (fp, "Gold %d\n", 0);
     fprintf (fp, "Exp  %d\n", ch->exp);
     if (ch->act != 0)
          fprintf (fp, "Act  %ld\n", ch->act);
     if (ch->affected_by != 0)
          fprintf( fp, "AfBy %ld\n",       ch->affected_by );
     if (ch->detections != 0)
          fprintf( fp, "Detc %ld\n",	     ch->detections  );
     if (ch->protections != 0)
          fprintf( fp, "Prot %ld\n",	     ch->protections );
     fprintf (fp, "Comm %ld\n", ch->comm);
     if (get_trust (ch) >= LEVEL_IMMORTAL)
          fprintf (fp, "Wiznet %ld\n", ch->pcdata->wiznet);
     fprintf (fp, "Notify %ld\n", ch->pcdata->notify);
     if (ch->invis_level != 0)
          fprintf (fp, "Invi %d\n", ch->invis_level);
     if (ch->cloak_level != 0)
          fprintf (fp, "Cloak %d\n", ch->cloak_level);
     fprintf (fp, "Pos  %d\n",
              ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
     if (ch->pcdata->practice != 0)
          fprintf (fp, "Prac %d\n", ch->pcdata->practice);
     if (ch->pcdata->train != 0)
          fprintf (fp, "Trai %d\n", ch->pcdata->train);
     if (ch->saving_throw != 0)
          fprintf (fp, "Save  %d\n", ch->saving_throw);
     fprintf (fp, "Alig  %d\n", ch->alignment);
     if (ch->hitroll != 0)
          fprintf (fp, "Hit   %d\n", ch->hitroll);
     if (ch->damroll != 0)
          fprintf (fp, "Dam   %d\n", ch->damroll);
     //     fprintf (fp, "ACs %d %d %d %d\n",
     //              ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
     fprintf ( fp, "ACs %d\n",
               ch->armor );
     if (ch->wimpy != 0)
          fprintf (fp, "Wimp  %d\n", ch->wimpy);
     fprintf (fp, "Attr %d %d %d %d %d\n",
              ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
              ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON]);
     fprintf (fp, "AMod %d %d %d %d %d\n",
              ch->mod_stat[STAT_STR], ch->mod_stat[STAT_INT],
              ch->mod_stat[STAT_WIS], ch->mod_stat[STAT_DEX], ch->mod_stat[STAT_CON]);

     if (IS_NPC (ch))
     {
          fprintf (fp, "Vnum %d\n", ch->pIndexData->vnum);
     }
     else
     {
          fprintf (fp, "Pass %s~\n", ch->pcdata->pwd);
          if (ch->pcdata->bamfin[0] != '\0')
               fprintf (fp, "Bin  %s~\n", ch->pcdata->bamfin);
          if (ch->pcdata->bamfout[0] != '\0')
               fprintf (fp, "Bout %s~\n", ch->pcdata->bamfout);
          fprintf (fp, "Titl %s~\n", ch->pcdata->title);
          fprintf (fp, "Emai %s~\n", ch->pcdata->email);
          fprintf (fp, "Acct %s~\n", ch->pcdata->account->acc_name);
          fprintf (fp, "Enc %d\n", ch->encumbrance);

          fprintf (fp, "ImTi %s~\n", ch->pcdata->immtitle);
          if (ch->pcdata->startyear < 1)
          {
               ch->pcdata->startyear = time_info.year;
               ch->pcdata->startmonth = time_info.month;
               ch->pcdata->startday = time_info.day;
          }

          fprintf (fp, "Syer %d\n", ch->pcdata->startyear);
          fprintf (fp, "Smnt %d\n", ch->pcdata->startmonth);
          fprintf (fp, "Sday %d\n", ch->pcdata->startday);
          fprintf (fp, "Agemod %d\n", ch->pcdata->age_mod);
          fprintf (fp, "Pnts %d\n", ch->pcdata->points);
          fprintf (fp, "TSex %d\n", ch->pcdata->true_sex);
          fprintf (fp, "LLev %d\n", ch->pcdata->last_level);
          fprintf (fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
                   ch->pcdata->perm_mana, ch->pcdata->perm_move);
          fprintf (fp, "Cond %d %d %d\n",
                   ch->pcdata->condition[0],
                   ch->pcdata->condition[1], ch->pcdata->condition[2]);
          fprintf (fp, "Prompt %s~\n", ch->pcdata->prompt);
          {
               struct alias_data *tmp;
               int counter;
               fprintf (fp, "Alias\n");
               for (counter = 0; counter < MAX_ALIAS; counter++)
               {
                    tmp = ch->pcdata->aliases[counter];
                    if (tmp != NULL)
                         fprintf (fp, "%s~%s~\n", tmp->name, tmp->command_string);
                    else
                         continue;
               }
               fprintf (fp, "@~\n");
          }

		  /* Save note board status */
		  /* Save number of boards in case that number changes */
          fprintf (fp, "Boards       %d ", MAX_BOARD);
          for (i = 0; i < MAX_BOARD; i++)
               fprintf (fp, "%s %ld ", boards[i].short_name,
                        ch->pcdata->last_note[i]);
          fprintf (fp, "\n");

          for (sn = 0; sn < MAX_SKILL; sn++)
          {
               if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
               {
                    fprintf (fp, "Sk %d '%s'\n",
                             ch->pcdata->learned[sn], skill_table[sn].name);
               }
          }
          i3save_char( ch, fp );
     }
     for (paf = ch->affected; paf != NULL; paf = paf->next)
     {
          if (paf->type < 0 || paf->type >= MAX_SKILL)
               continue;

          fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d\n",
                   skill_table[paf->type].name,
                   paf->level,
                   paf->duration,
                   paf->modifier,
                   paf->location,
                   paf->bitvector );
     }

     fprintf (fp, "End\n\n");
     return;
}

/* write a pet */
void fwrite_pet (CHAR_DATA * pet, FILE * fp)
{
     AFFECT_DATA *paf;

     fprintf (fp, "#PET\n");

     fprintf (fp, "Vnum %d\n", pet->pIndexData->vnum);

     fprintf (fp, "Name %s~\n", pet->name);
     if (pet->short_descr != pet->pIndexData->short_descr)
          fprintf (fp, "ShD  %s~\n", pet->short_descr);
     if (pet->long_descr != pet->pIndexData->long_descr)
          fprintf (fp, "LnD  %s~\n", pet->long_descr);
     if (pet->description != pet->pIndexData->description)
          fprintf (fp, "Desc %s~\n", pet->description);
     if (pet->race != pet->pIndexData->race)
          fprintf (fp, "Race %s~\n", race_table[pet->race].name);
     fprintf (fp, "Sex  %d\n", pet->sex);
     if (pet->level != pet->pIndexData->level)
          fprintf (fp, "Levl %d\n", pet->level);
     fprintf (fp, "HMV  %d %d %d %d %d %d\n",
              pet->hit, pet->max_hit, pet->mana, pet->max_mana,
              pet->move, pet->max_move);
     if (pet->gold > 0)
          fprintf (fp, "Gold %ld\n", pet->gold);
     if (pet->exp > 0)
          fprintf (fp, "Exp  %d\n", pet->exp);
     if (pet->act != pet->pIndexData->act)
          fprintf (fp, "Act  %ld\n", pet->act);
     if (pet->affected_by != pet->pIndexData->affected_by)
          fprintf(fp, "AfBy %ld\n", pet->affected_by);
     if (pet->detections != pet->pIndexData->detections)
          fprintf(fp, "Detc %ld\n", pet->detections);
     if (pet->protections != pet->pIndexData->protections)
          fprintf(fp, "Prot %ld\n", pet->protections);
     if (pet->comm != 0)
          fprintf (fp, "Comm %ld\n", pet->comm);
     fprintf (fp, "Pos  %d\n", pet->position =
              POS_FIGHTING ? POS_STANDING : pet->position);
     if (pet->saving_throw != 0)
          fprintf (fp, "Save %d\n", pet->saving_throw);
     if (pet->alignment != pet->pIndexData->alignment)
          fprintf (fp, "Alig %d\n", pet->alignment);
     if (pet->hitroll != pet->pIndexData->hitroll)
          fprintf (fp, "Hit  %d\n", pet->hitroll);
     if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
          fprintf (fp, "Dam  %d\n", pet->damroll);
     //     fprintf (fp, "ACs  %d %d %d %d\n",
     //              pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
     fprintf ( fp, "ACs %d\n",
               pet->armor );
     fprintf (fp, "Attr %d %d %d %d %d\n",
              pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
              pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
              pet->perm_stat[STAT_CON]);
     fprintf (fp, "AMod %d %d %d %d %d\n", pet->mod_stat[STAT_STR],
              pet->mod_stat[STAT_INT], pet->mod_stat[STAT_WIS],
              pet->mod_stat[STAT_DEX], pet->mod_stat[STAT_CON]);

     for (paf = pet->affected; paf != NULL; paf = paf->next)
     {
          if (paf->type < 0 || paf->type >= MAX_SKILL)
               continue;

          fprintf(fp, "AffD '%s' %3d %3d %3d %3d %10d\n",
                  skill_table[paf->type].name,
                  paf->level, paf->duration, paf->modifier,paf->location,
                  paf->bitvector);
     }

     fprintf (fp, "End\n");
     return;
}

/*
 * Need to rewrite this someday to ONLY save that which is different from the "model" object.
 */

/*
 * Write an object and its contents.
 */
void fwrite_obj (CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest)
{
     EXTRA_DESCR_DATA *ed;
     AFFECT_DATA *paf;

	 /*
	  * Slick recursion to write lists backwards,
	  *   so loading them will load in forwards order.
	  */

     if (obj->next_content != NULL)
          fwrite_obj (ch, obj->next_content, fp, iNest);

	 /*
	  * Castrate storage characters.
	  */
	 /* Castrate disappearing items. */
	 /*   if ( (ch->level < obj->level - 10 && obj->item_type != ITEM_CONTAINER) */

     if ((obj->item_type == ITEM_KEY && !ITEM_NOPURGE)
         || (obj->item_type == ITEM_MAP && !obj->value[0]))
          return;

     fprintf (fp, "#O\n");
     fprintf (fp, "Vnum %d\n", obj->pIndexData->vnum);
     if (!obj->pIndexData->new_format)
          fprintf (fp, "Oldstyle\n");
     if (obj->enchanted)
          fprintf (fp, "Enchanted\n");
     fprintf (fp, "Nest %d\n", iNest);
     fprintf (fp, "Size   %d\n", obj->size);

	 /* these data are only used if they do not match the defaults */

     if (obj->name != obj->pIndexData->name)
          fprintf (fp, "Name %s~\n", obj->name);
     if (obj->short_descr != obj->pIndexData->short_descr)
          fprintf (fp, "ShD  %s~\n", obj->short_descr);
     if (obj->description != obj->pIndexData->description)
          fprintf (fp, "Desc %s~\n", obj->description);
     if (obj->owner && obj->owner[0] != '\0')
          fprintf (fp, "Owner %s~\n", obj->owner);
     if (obj->extra_flags != obj->pIndexData->extra_flags)
          fprintf (fp, "ExtF %d\n", obj->extra_flags);
     if (obj->wear_flags != obj->pIndexData->wear_flags)
          fprintf (fp, "WeaF %d\n", obj->wear_flags);
     if (obj->item_type != obj->pIndexData->item_type)
          fprintf (fp, "Ityp %d\n", obj->item_type);
     if (obj->weight != obj->pIndexData->weight)
          fprintf (fp, "Wt   %d\n", obj->weight);

	 /* variable data */

     fprintf (fp, "Wear %d\n", obj->wear_loc);
     if (obj->level != 0)
          fprintf (fp, "Lev  %d\n", obj->level);
     if (obj->timer != 0)
          fprintf (fp, "Time %d\n", obj->timer);
     fprintf (fp, "Cost %d\n", obj->cost);
     if (obj->serialnum != 0)
          fprintf (fp, "Serial %d\n", obj->serialnum);
     fprintf (fp, "Cond %d\n", obj->condition);
     fprintf (fp, "Val  %d %d %d %d %d\n",
              obj->value[0], obj->value[1], obj->value[2],
              obj->value[3], obj->value[4]);
     fprintf (fp, "ValOrig  %d %d %d %d %d\n", obj->valueorig[0],
              obj->valueorig[1], obj->valueorig[2],
              obj->valueorig[3], obj->valueorig[4]);

     switch (obj->item_type)
     {
     case ITEM_POTION:
     case ITEM_SCROLL:
          if (obj->value[1] > 0)
          {
               fprintf (fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name);
          }

          if (obj->value[2] > 0)
          {
               fprintf (fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name);
          }

          if (obj->value[3] > 0)
          {
               fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
          }

          break;

     case ITEM_PILL:
     case ITEM_STAFF:
     case ITEM_WAND:
          if (obj->value[3] > 0)
          {
               fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name);
          }

          break;
     }

     for (paf = obj->affected; paf != NULL; paf = paf->next)
     {

          if (paf->type < 0 || paf->type >= MAX_SKILL)
               continue;
          fprintf( fp, "AffD '%s' %d %d %d %d %d\n",
                   skill_table[paf->type].name,
                   paf->level,
                   paf->duration,
                   paf->modifier,
                   paf->location,
                   paf->bitvector
                   );
     }

     for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
     {
          fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
     }

     fprintf (fp, "End\n\n");

     if (obj->contains != NULL)
          fwrite_obj (ch, obj->contains, fp, iNest + 1);

     return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj (DESCRIPTOR_DATA * d, char *name)
{
     static PC_DATA pcdata_zero;
     char strsave[MAX_INPUT_LENGTH];
     char buf[100];
     CHAR_DATA *ch;
     struct char_group *tmp;
     FILE *fp;
     bool found;
     int stat;

     if (char_free == NULL)
     {
          ch = alloc_perm (sizeof (*ch), "ch:load_char_obj");
     }
     else
     {
          ch = char_free;
          char_free = char_free->next;
     }
     clear_char (ch);

     if (pcdata_free == NULL)
     {
          ch->pcdata = alloc_perm (sizeof (*ch->pcdata), "pcdata:load_char_obj");
     }
     else
     {
          ch->pcdata = pcdata_free;
          pcdata_free = pcdata_free->next;
     }
     *ch->pcdata = pcdata_zero;

     d->character = ch;
     ch->desc = d;
     ch->name = str_dup (name);

	 /* Add Self To Group - Kludgy */
     tmp = newgroup ();

     ch->group[MAX_GMEMBERS - 1] = tmp;
     tmp->gch = ch;
     ch->leader = ch;

     ch->poly_name = str_dup ("");
     ch->description = str_dup ("");
     ch->short_descr = str_dup ("");
     ch->long_descr = str_dup ("");
     ch->description_orig = str_dup ("");
     ch->short_descr_orig = str_dup ("");
     ch->long_descr_orig = str_dup ("");
     ch->speaking = str_dup("");
     ch->version = 0;
     ch->race = race_lookup ("human");
     ch->pcdata->pcrace = pcrace_lookup ("human");
     ch->affected_by = 0;
     ch->detections = 0;
     ch->protections = 0;
     ch->act = PLR_NOSUMMON;
     ch->pcdata->board = &boards[DEFAULT_BOARD];
     ch->comm = COMM_COMBINE | COMM_PROMPT;
     ch->pcdata->notify = 0;
     ch->pcdata->wiznet = 0;
     ch->pcdata->afk_tell_first = NULL;
     ch->pcdata->afk_tell_last = NULL;
     ch->invis_level = 0;
     ch->cloak_level = 0;
     ch->pcdata->clrank = -1;
     ch->pcdata->practice = 0;
     ch->pcdata->train = 0;
     ch->hitroll = 0;
     ch->damroll = 0;
     ch->trust = 0;
     ch->recall_perm = ROOM_VNUM_LIMBO;
     ch->recall_temp = 0;
     ch->wimpy = 0;
     ch->quitting = FALSE;
     ch->searching = FALSE;
     ch->saving_throw = 0;
     ch->pcdata->points = 0;
     ch->pcdata->pwd_tries = 0;
     ch->pcdata->confirm_delete = FALSE;
     ch->pcdata->pwd = str_dup ("");
     ch->pcdata->bamfin = str_dup ("");
     ch->pcdata->bamfout = str_dup ("");
     ch->pcdata->title = str_dup ("");
     ch->pcdata->email = str_dup ("");
     ch->pcdata->immtitle = str_dup ("");
     for (stat = 0; stat < MAX_STATS; stat++)
          ch->perm_stat[stat] = 13;
     ch->pcdata->startyear = time_info.year;
     ch->pcdata->startmonth = time_info.month;
     ch->pcdata->startday = time_info.day;
     ch->pcdata->age_mod = 0;
     {
          int counter;

          for (counter = 0; counter < MAX_ALIAS; counter++)
               ch->pcdata->aliases[counter] = NULL;
     }
     ch->pcdata->has_alias = FALSE;
     ch->pcdata->perm_hit = 0;
     ch->pcdata->perm_mana = 0;
     ch->pcdata->perm_move = 0;
     ch->pcdata->true_sex = 0;
     ch->pcdata->last_level = 0;
     ch->pcdata->condition[COND_THIRST] = 48;
     ch->pcdata->condition[COND_FULL] = 48;
     ch->pcdata->security = 0;	/* OLC */
     ch->pcdata->mvolume = 50;
     ch->pcdata->svolume = 75;
     ch->pcdata->prompt = str_dup ("{G({W$h/$Hhp $m/$Mmn $v/$Vmv{G){w");
     ch->pcdata->home_room = ROOM_VNUM_TEMPLE;
     ch->pcdata->clan = NULL;
     ch->pcdata->cedit = NULL;
     ch->pcdata->petition = NULL;

     found = FALSE;
     fclose (fpReserve);

	 /* decompress if .gz file exists */
     SNP (strsave, "%s%s%s", PLAYER_DIR, capitalize (name), ".gz");
     if ((fp = fopen (strsave, "r")) != NULL)
     {
          fclose (fp);
          SNP (buf, "gzip -dfq %s", strsave);
          system (buf);
     }

     SNP (strsave, "%s%s", PLAYER_DIR, capitalize (name));
     log_string ( strsave );
     if ((fp = fopen (strsave, "r")) != NULL)
     {
          int iNest;

          for (iNest = 0; iNest < MAX_NEST; iNest++)
               rgObjNest[iNest] = NULL;

          found = TRUE;
          for (;;)
          {
               char letter;
               char *word;

               letter = fread_letter (fp);
               if (letter == '*')
               {
                    fread_to_eol (fp);
                    continue;
               }

               if (letter != '#')
               {
                    bugf ("Load_char_obj: # not found in file : %s", strsave);
                    break;
               }

               word = fread_word (fp);
               if (!str_cmp (word, "PLAYER"))
                    fread_char (ch, fp);
               else if (!str_cmp (word, "O"))
               {
                    fread_obj (ch, fp);
               }
               else if (!str_cmp (word, "PET"))
                    fread_pet (ch, fp);
               else if (!str_cmp (word, "END"))
                    break;
               else
               {
                    bugf ("Load_char_obj: bad section in file : %s", strsave );
                    break;
               }
          }
          fclose (fp);
     }

     fpReserve = fopen (NULL_FILE, "r");

	 /* initialize race */
     if (found)
     {
          if (ch->race == 0)
          {
               ch->race = race_lookup ("human");
               ch->pcdata->pcrace = pcrace_lookup ("human");
          }
          else
               ch->pcdata->pcrace = pcrace_lookup ( race_table[ch->race].name );
          ch->size = pc_race_table[ch->pcdata->pcrace].size;
          ch->encumbrance = race_table[ch->race].encumbrance;
          ch->dam_type = 17;	/*punch */
          if (ch->speaking == NULL)
               ch->speaking = str_dup (race_table[ch->race].name);
          ch->affected_by = ch->affected_by | race_table[ch->race].aff;
          ch->detections = ch->detections | race_table[ch->race].detect;
          ch->protections = ch->protections | race_table[ch->race].protect;
          ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
          ch->res_flags = ch->res_flags | race_table[ch->race].res;
          ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
          ch->form = race_table[ch->race].form;
          ch->parts = race_table[ch->race].parts;
     }
    
     /* 
      * Set permissions for characters who didn't have the data in their pfiles already 
      */
     I3_set_perms ( ch );

     /* Set common tongue language skill to 100% always */
     /* Set player race language to 100% always */

     ch->pcdata->learned[skill_lookup ("common tongue")] = 100;
     if (found && ch->race)
     {
          char buf[128];

          SNP (buf, "%s tongue", race_table[ch->race].name);
          ch->pcdata->learned[skill_lookup (buf)] = 100;
     }
     // Set desc ansi flag
     
     if ( IS_SET ( ch->comm, COMM_COLOUR ) )
          ch->desc->ansi = TRUE;
     if ( fCopyOver ) // If recovering from a copyover, then we want to go with what is on the pfile so we don't have to re-detect.
     {
          if ( IS_SET ( ch->comm, COMM_PUEBLO ) ) // If saved pfile had Pueblo, then re-enable pueblo now.
               ch->desc->pueblo = TRUE;
          if ( IS_SET ( ch->comm, COMM_MXP ) )
               ch->desc->mxp = TRUE;
     }

     return found;
}

/*
 * Read in a char. As of Version 7, older pfiles no longer supported.
 * Thus, tons of no longer used keywords can now be removed from here. - Lotherius
 */

void fread_char (CHAR_DATA * ch, FILE * fp)
{
     char buf[MAX_STRING_LENGTH];
     char *word;
     bool fMatch;

     for (;;)
     {
          word = feof (fp) ? "End" : fread_word (fp);
          fMatch = FALSE;

          switch (UPPER (word[0]))
          {
          case '*':
               fMatch = TRUE;
               fread_to_eol (fp);
               break;
          case 'A':
               KEY ("Act",         ch->act, fread_number (fp));
               KEY ("AfBy",        ch->affected_by, fread_number(fp));
               KEY ("Alig",        ch->alignment, fread_number (fp));
               KEY ("Agemod",      ch->pcdata->age_mod, fread_number (fp));
			   /* I'd call the next routine a bit "loopy"... */
               if (!str_cmp (word, "Acct"))
               {
                    struct account_type  *tmp;
                    int i = 0;
                    char *tword;
                    bool foundmatch = FALSE;
                    fMatch = TRUE;

                    tword = fread_string (fp);

                    for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
                    {
                         if (!strcmp (tword, tmp->acc_name) )
                         {
                              ch->pcdata->account = tmp;
                              foundmatch = TRUE;
                              break;
                         }
                    }
                    if (!foundmatch)                    
                         bugf ( "Can't find an account for %s. Probably fatal.", ch->name );

                    /* Sanity Check - in case the mud crashed before saving account data or sumthin. */
                    foundmatch = FALSE;
                    for (i = 0 ; i < MAX_CHARS ; i++)
                    {
                         if (ch->pcdata->account->char_name[i])
                         {
                              if (!strcmp (ch->name, ch->pcdata->account->char_name[i]) )
                              {
                                   foundmatch = TRUE;
                                   break;
                              }
                         }
                    }
                    if (!foundmatch) /* This time, not so fatal - run through the list again */
                    {
                         for (i = 0 ; i < MAX_CHARS ; i++)
                         { /* Got a live one! */
                              if (!ch->pcdata->account->char_name[i])
                              {
                                   foundmatch = TRUE;
                                   ch->pcdata->account->char_name[i] = str_dup (ch->name);
                                   break;
                              }
                         }
                    }
                    if (!foundmatch) /* This time more annoying, but still not fatal. */
                    {
                         bugf ( "Existing Char %s:%s no account slot.",
                                ch->name, ch->pcdata->account->acc_name );
                    }
                    free_string ( tword );
                    break;
               }
               /* End of "Account" */
               if (!str_cmp (word, "Alias"))
               {
                    struct alias_data *tmp;
                    int counter = 0;
                    char *tword;
                    bool done = FALSE;
                    while (!done)
                    {
                         tword = fread_string (fp);
                         if (!str_cmp (tword, "@"))
                              done = TRUE;
                         else
                         {
                              tmp = alloc_mem ( sizeof ( struct alias_data ), "alias_data" );
                              tmp->name = str_dup (tword);
                              tmp->command_string = fread_string (fp);
                              ch->pcdata->aliases[counter] = tmp;
                              ch->pcdata->has_alias = TRUE;
                              counter++;
                         }
                         free_string (tword);
                    }
                    fMatch = TRUE;
                    break;
               }
               /* End of "Alias */
               if (!str_cmp (word, "ACs"))
               {
                    // for (i = 0; i < 4; i++)
                    ch->armor = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "AffD") )
               {
                    int sn;
                    AFFECT_DATA *paf;

                    if (!affect_free)
                    {
                         paf = alloc_perm (sizeof (*paf), "paf:fread_char");
                    }
                    else
                    {
                         paf = affect_free;
                         affect_free = affect_free->next;
                    }

                    sn = skill_lookup (fread_word (fp) );
                    if (sn < 0)
                         bugf ("Fread_char: unknown skill on player %s:", ( ch->name ? ch->name : "Unknown" ) );
                    else
                         paf->type = sn;

                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    affect_to_char ( ch, paf ); // Makes sure affect is properly initialized
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "AMod") )
               {
                    int stat;

                    for (stat = 0; stat < MAX_STATS; stat++)
                         ch->mod_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "Attr") )
               {
                    int stat;

                    for (stat = 0; stat < MAX_STATS; stat++)
                         ch->perm_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               break;

          case 'B':
               KEYS ("Bin", ch->pcdata->bamfin, fread_string (fp));
               if (!str_cmp (word, "Boards"))
               {
                    int i;
                    int num = fread_number (fp);
					/* number of boards saved */
                    char *boardname;
                    for (; num; num--)	/* for each of the board saved */
                    {
                         boardname = fread_word (fp);
                         i = board_lookup ( boardname );
						 /* find board number */
                         if (i == BOARD_NOTFOUND)
							  /* Does board still exist ? */
                         {
                              bugf ( "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
                              fread_number (fp);
							  /* read last_note and skip info */
                         }
                         else		/* Save it */
                              ch->pcdata->last_note[i] = fread_number (fp);
                    }
                    fMatch = TRUE;
                    break;		/* Zeran - this was missing... */
               }
               /* End of Boards */
               KEY ("Bank", ch->pcdata->bankaccount, fread_number(fp));
               KEYS ("Bout", ch->pcdata->bamfout, fread_string (fp));
               KEY ("Brating", ch->pcdata->battle_rating, fread_number (fp));
               break;
          case 'C':
               KEY ("Cloak", ch->cloak_level, fread_number (fp));
               KEY ("Cla", ch->pcdata->pclass, fread_number (fp));
               KEY ("Clrank", ch->pcdata->clrank, fread_number (fp));
               if (!str_cmp (word, "Clan"))
               {
                    char *dummy;
                    dummy = fread_string (fp);

                    ch->pcdata->clan = clan_by_short (dummy);
                    fMatch = TRUE;
                    free_string (dummy); /* Wow someone actually freed the string! */
                    if (ch->pcdata->clrank < 0 || ch->pcdata->clrank > 6)
                         ch->pcdata->clrank = 5;
                    break;
               }
               if ( !str_cmp (word, "Cond"))
               {
                    ch->pcdata->condition[0] = fread_number (fp);
                    ch->pcdata->condition[1] = fread_number (fp);
                    ch->pcdata->condition[2] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               KEY ("Comm", ch->comm, fread_number (fp));
               break;
          case 'D':
               KEY ("Dam", ch->damroll, fread_number (fp));
               KEYS ("Desc", ch->description, fread_string (fp));
               KEYS ("DescOrig", ch->description_orig, fread_string (fp));
               KEY ("Detc", ch->detections, fread_number(fp));
               break;
          case 'E':
               if (!str_cmp (word, "End"))
               {
                    return;
               }
               KEY ("Exp", ch->exp, fread_number (fp));
               KEYS ("Emai", ch->pcdata->email, fread_string (fp));
               KEY ("Enc", ch->encumbrance, fread_number (fp));
               break;
          case 'G':
               KEY ("Gold", ch->gold, fread_number (fp));
          case 'H':
               KEY ("Hit", ch->hitroll, fread_number (fp));
               KEY ("Home", ch->pcdata->home_room, fread_number (fp));
               if ( !str_cmp (word, "HMV") )
               {
                    ch->hit = fread_number (fp);
                    ch->max_hit = fread_number (fp);
                    ch->mana = fread_number (fp);
                    ch->max_mana = fread_number (fp);
                    ch->move = fread_number (fp);
                    ch->max_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "HMVP") )
               {
                    ch->pcdata->perm_hit = fread_number (fp);
                    ch->pcdata->perm_mana = fread_number (fp);
                    ch->pcdata->perm_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'I':
               KEYS ("ImTi", ch->pcdata->immtitle, fread_string (fp));
               KEY ("Invi", ch->invis_level, fread_number (fp));
              if( ( fMatch = i3load_char( ch, fp, word ) ) )
                  break;              
               break;
          case 'L':
               KEY ("LLev", ch->pcdata->last_level, fread_number (fp));
               if ( !str_cmp ( word, "Levl" ) )
               {
                   ch->level = fread_number ( fp );
                   i3init_char( ch );
                   fMatch = TRUE;
                   break;
               }
               KEYS ("LnD", ch->long_descr, fread_string (fp));
               KEYS ("LnDOrig", ch->long_descr_orig, fread_string (fp));
               break;
          case 'M':
               KEY ("Mort", ch->pcdata->mortal, fread_number(fp));
               KEY ("Mrate", ch->pcdata->mob_rating, fread_number (fp));
               KEY ("Mwin", ch->pcdata->mob_wins, fread_number (fp));
               KEY ("Mloss", ch->pcdata->mob_losses, fread_number (fp));
               break;
          case 'N':
               KEYS ("Name", ch->name, fread_string (fp));
               KEY ("Note", ch->last_note, fread_number (fp));
               KEY ("Notify", ch->pcdata->notify, fread_number (fp));
               break;
          case 'P':
               KEYS ("Prompt", ch->pcdata->prompt, fread_string (fp));
               KEYS ("Pass", ch->pcdata->pwd, fread_string (fp));
               KEY ("Plyd", ch->played, fread_number (fp));
               KEY ("Pnts", ch->pcdata->points, fread_number (fp));
               KEY ("Pos", ch->position, fread_number (fp));
               KEY ("Prac", ch->pcdata->practice, fread_number (fp));
               KEY ("Prot", ch->protections, fread_number(fp));
               KEYS ("Polyname", ch->poly_name, fread_string (fp));
               KEY ("Pkwin", ch->pcdata->pkill_wins, fread_number (fp));
               KEY ("Pkloss", ch->pcdata->pkill_losses, fread_number (fp));
               break;
          case 'Q':
               KEY ("QuestPnts", ch->pcdata->questpoints, fread_number (fp));
               KEY ("QuestErnd", ch->pcdata->questearned, fread_number (fp));
               KEY ("QuestNext", ch->pcdata->nextquest, fread_number (fp));
               break;
          case 'R':
               KEY ("Race", ch->race, race_lookup (fread_string (fp)));
               KEY ("Recall", ch->recall_perm, fread_number (fp));
               if (!str_cmp (word, "Room"))
               {
                    ch->in_room = get_room_index (fread_number (fp));
                    if (ch->in_room == NULL)
                         ch->in_room = get_room_index (ROOM_VNUM_LIMBO);
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'S':
               KEY ("Save", ch->saving_throw, fread_number (fp));
               KEY ("Scro", ch->lines, fread_number (fp));
               KEY ("Sex", ch->sex, fread_number (fp));
               KEYS ("ShD", ch->short_descr, fread_string (fp));
               KEY ("Sec", ch->pcdata->security, fread_number (fp));	/* OLC */
               KEY ("Syer", ch->pcdata->startyear, fread_number (fp));
               KEY ("Smnt", ch->pcdata->startmonth, fread_number (fp));
               KEY ("Sday", ch->pcdata->startday, fread_number (fp));
               KEYS ("ShDOrig", ch->short_descr_orig, fread_string (fp));
               KEYS ("Speaking", ch->speaking, fread_string ( fp ));
               if (!str_cmp (word, "Sk"))
               {
                    int sn;
                    int value;
                    char *temp;
                    value = fread_number (fp);
                    temp = fread_word (fp);
                    sn = skill_lookup (temp);
					/* sn    = skill_lookup( fread_word( fp ) ); */
                    if (sn < 0)
                    {
                         bugf ("Fread_char: unknown skill on player %s:", ( ch->name ? ch->name : "Unknown" ) );
                    }
                    else
                         ch->pcdata->learned[sn] = value;
                    fMatch = TRUE;
               }
               break;
          case 'T':
               KEY ("TSex", ch->pcdata->true_sex, fread_number (fp));
               KEY ("Trai", ch->pcdata->train, fread_number (fp));
               KEY ("Tru", ch->trust, fread_number (fp));
               if  ( !str_cmp (word, "Titl"))
               {
                    char *temp;
                    temp = fread_string ( fp );
                    ch->pcdata->title = str_dup ( temp );
                    free_string ( temp );
                    if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ','
                        && ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
                    {
                         SNP (buf, " %s", ch->pcdata->title);
                         free_string (ch->pcdata->title);
                         ch->pcdata->title = str_dup (buf);
                    }
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'V':
               KEY ("Vers", ch->version, fread_number (fp));
               KEY ("VM", ch->pcdata->mvolume, fread_number (fp)); /* MSP Music Volume */
               KEY ("VS", ch->pcdata->svolume, fread_number (fp)); /* MSP Sound Volume */
               if (!str_cmp (word, "Vnum") )   // Guess I can leave this... not like we ever have mobs here.
               {
                    ch->pIndexData = get_mob_index (fread_number (fp));
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'W':
               KEY ("Wimpy", ch->wimpy, fread_number (fp));
               KEY ("Wimp", ch->wimpy, fread_number (fp));
               KEY ("Wiznet", ch->pcdata->wiznet, fread_number (fp));
               break;
          }
          if (!fMatch)
          {
               bugf ( "fread_char: No Match: %s on %s", word, ( ch->name ? ch->name : "Unknown player" ) );
               fread_to_eol (fp);
          }
     }
}

/* load a pet from the forgotten reaches */
void fread_pet (CHAR_DATA * ch, FILE * fp)
{
     char *word;
     CHAR_DATA *pet;
     bool fMatch;

	 /* first entry had BETTER be the vnum or we barf */
     word = feof (fp) ? "END" : fread_word (fp);
     if (!str_cmp (word, "Vnum"))
     {
          int vnum;
          vnum = fread_number (fp);
          if (get_mob_index (vnum) == NULL)
          {
               bugf ("Fread_pet: bad vnum %d.", vnum);
               pet = create_mobile (get_mob_index (MOB_VNUM_ZOMBIE));
          }
          else
               pet = create_mobile (get_mob_index (vnum));
     }
     else
     {
          bugf ("Fread_pet: no vnum in file.");
          pet = create_mobile (get_mob_index (MOB_VNUM_ZOMBIE));
     }

     for (;;)
     {
          word = feof (fp) ? "END" : fread_word (fp);
          fMatch = FALSE;
          switch (UPPER (word[0]))
          {
          case '*':
               fMatch = TRUE;
               fread_to_eol (fp);
               break;
          case 'A':
               KEY ("Act", pet->act, fread_number (fp));
               KEY ("Afby", pet->affected_by, fread_number (fp));
               KEY ("Alig", pet->alignment, fread_number (fp));
               if (!str_cmp (word, "ACs"))
               {
                    pet->armor = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "AffD"))
               {
                    AFFECT_DATA *paf;
                    int sn;
                    if (affect_free == NULL)
                         paf = alloc_perm (sizeof (*paf), "paf:fread_pet");
                    else
                    {
                         paf = affect_free;
                         affect_free = affect_free->next;
                    }
                    sn = skill_lookup ( fread_word(fp) );
                    if (sn < 0)
                         bugf ("Fread_char: unknown skill.");
                    else
                         paf->type = sn;
                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    affect_to_char ( pet, paf );
                    fMatch = TRUE;
                    break;
               }

               if (!str_cmp (word, "AMod"))
               {
                    int stat;
                    for (stat = 0; stat < MAX_STATS; stat++)
                         pet->mod_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "Attr"))
               {
                    int stat;
                    for (stat = 0; stat < MAX_STATS; stat++)
                         pet->perm_stat[stat] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'C':
               KEY ("Comm", pet->comm, fread_number (fp));
               break;
          case 'D':
               KEY ("Dam", pet->damroll, fread_number (fp));
               KEYS ("Desc", pet->description, fread_string (fp));
               break;
          case 'E':
               if (!str_cmp (word, "End"))
               {
                    pet->leader = ch;
                    pet->master = ch;
                    ch->pet = pet;
                    return;
               }
               KEY ("Exp", pet->exp, fread_number (fp));
               break;
          case 'G':
               KEY ("Gold", pet->gold, fread_number (fp));
               break;
          case 'H':
               KEY ("Hit", pet->hitroll, fread_number (fp));
               if (!str_cmp (word, "HMV"))
               {
                    pet->hit = fread_number (fp);
                    pet->max_hit = fread_number (fp);
                    pet->mana = fread_number (fp);
                    pet->max_mana = fread_number (fp);
                    pet->move = fread_number (fp);
                    pet->max_move = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'L':
               KEY ("Levl", pet->level, fread_number (fp));
               KEYS ("LnD", pet->long_descr, fread_string (fp));
               break;
          case 'N':
               KEYS ("Name", pet->name, fread_string (fp));
               break;
          case 'P':
               KEY ("Pos", pet->position, fread_number (fp));
               break;
          case 'R':
               KEY ("Race", pet->race, race_lookup (fread_string (fp)));
               break;
          case 'S':
               KEY ("Save", pet->saving_throw, fread_number (fp));
               KEY ("Sex", pet->sex, fread_number (fp));
               KEYS ("ShD", pet->short_descr, fread_string (fp));
               break;
               if (!fMatch)
               {
                    bugf ( "fread_pet: No Match: %s", word );
                    fread_to_eol (fp);
               }

          }
     }

}

void fread_obj (CHAR_DATA * ch, FILE * fp)
{
     static OBJ_DATA obj_zero;
     OBJ_DATA *obj;
     char *word;
     int iNest;
     bool fMatch;
     bool fNest;
     bool fVnum;
     bool first;
     bool new_format;		/* to prevent errors */
     bool make_new;		/* update object */
     fVnum = FALSE;
     obj = NULL;
     first = TRUE;			/* used to counter fp offset */
     new_format = FALSE;
     make_new = FALSE;
     word = feof (fp) ? "End" : fread_word (fp);

     if (!str_cmp (word, "Vnum"))
     {
          int vnum;
          first = FALSE;		/* fp will be in right place */
          vnum = fread_number (fp);
          if (get_obj_index (vnum) == NULL)
          {
               bugf ("Fread_obj: bad vnum %d.", vnum );
          }
          else
          {
               obj = create_object (get_obj_index (vnum), -1);
               new_format = TRUE;
          }

     }

     if (obj == NULL)		/* either not found or old style */
     {
          if (obj_free == NULL)
          {
               obj = alloc_perm (sizeof (*obj), "obj:fread_obj");
          }
          else
          {
               obj = obj_free;
               obj_free = obj_free->next;
          }

          *obj = obj_zero;
          obj->name = str_dup ("");
          obj->short_descr = str_dup ("");
          obj->owner = str_dup ("");
          obj->description = str_dup ("");
     }

     fNest = FALSE;
     fVnum = TRUE;
     iNest = 0;
     for (;;)
     {
          if (first)
               first = FALSE;
          else
               word = feof (fp) ? "End" : fread_word (fp);
          fMatch = FALSE;
          switch (UPPER (word[0]))
          {
          case '*':
               fMatch = TRUE;
               fread_to_eol (fp);
               break;
          case 'A':
               if (!str_cmp (word, "AffD") )
               {
                    int sn;
                    AFFECT_DATA *paf;
                    if (affect_free == NULL)
                    {
                         paf = alloc_perm (sizeof (*paf), "paf:fread_obj");
                    }
                    else
                    {
                         paf = affect_free;
                         affect_free = affect_free->next;
                    }
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                         bugf ("Fread_obj: unknown skill.");
                    else
                         paf->type = sn;
                    paf->level = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->location = fread_number (fp);
                    paf->bitvector = fread_number (fp);
                    paf->next = obj->affected;
                    obj->affected = paf;
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'C':
               KEY ("Cost", obj->cost, fread_number (fp));
               KEY ("Cond", obj->condition, fread_number (fp));
               break;
          case 'D':
               KEYS ("Desc", obj->description, fread_string (fp));
               break;
          case 'E':
               if (!str_cmp (word, "Enchanted"))
               {
                    obj->enchanted = TRUE;
                    fMatch = TRUE;
                    break;
               }
               KEY ("ExtF", obj->extra_flags, fread_number (fp));
               if  ( !str_cmp (word, "ExDe"))
               {
                    EXTRA_DESCR_DATA *ed;
                    if (extra_descr_free == NULL)
                    {
                         ed = alloc_perm (sizeof (*ed), "ed:fread_obj");
                    }
                    else
                    {
                         ed = extra_descr_free;
                         extra_descr_free = extra_descr_free->next;
                    }
                    ed->keyword = fread_string (fp);
                    ed->description = fread_string (fp);
                    ed->next = obj->extra_descr;
                    obj->extra_descr = ed;
                    fMatch = TRUE;
               }
               if (!str_cmp (word, "End"))
               {
                    if (!fNest || (fVnum && obj->pIndexData == NULL))
                    {
                         bugf ("Fread_obj: incomplete object.");
                         extract_obj (obj);
                         return;
                    }
                    else
                    {
                         if (!fVnum)
                         {
                              free_string (obj->name);
                              free_string (obj->description);
                              free_string (obj->short_descr);
                              obj->next = obj_free;
                              obj_free = obj;
                              obj = create_object (get_obj_index (OBJ_VNUM_DUMMY), 0);
                         }

						 /* Someday this new_format junk needs to be ripped out */
                         if (!new_format)
                         {
                              obj->next = object_list;
                              object_list = obj;
                              obj->pIndexData->count++;
                         }
                         if (!obj->pIndexData->new_format
                             && obj->item_type == ITEM_ARMOR && obj->value[1] == 0)
                         {
                              obj->value[1] = obj->value[0];
                              obj->value[2] = obj->value[0];
                         }
                         if (make_new)
                         {
                              int wear;
                              wear = obj->wear_loc;
                              extract_obj (obj);
                              obj = create_object (obj->pIndexData, 0);
                              obj->wear_loc = wear;
                         }
                         if (iNest == 0 || rgObjNest[iNest] == NULL)
                              obj_to_char (obj, ch);
                         else
                              obj_to_obj (obj, rgObjNest[iNest - 1]);
                         return;
                    }
               }
               break;
          case 'I':
               KEY ("Ityp", obj->item_type, fread_number (fp));
               break;
          case 'L':
               KEY ("Lev", obj->level, fread_number (fp));
               break;
          case 'N':
               KEYS ("Name", obj->name, fread_string (fp));
               if (!str_cmp (word, "Nest"))
               {
                    iNest = fread_number (fp);
                    if (iNest < 0 || iNest >= MAX_NEST)
                    {
                         bugf ("Fread_obj: bad nest %d.", iNest);
                    }
                    else
                    {
                         rgObjNest[iNest] = obj;
                         fNest = TRUE;
                    }
                    fMatch = TRUE;
               }
               break;
          case 'O':
               KEYS ("Owner", obj->owner, fread_string(fp));
               if (!str_cmp (word, "Oldstyle"))
               {
                    if (obj->pIndexData != NULL && obj->pIndexData->new_format)
                         make_new = TRUE;
                    fMatch = TRUE;
               }
               break;
          case 'S':
               KEY ("Serial", obj->serialnum, fread_number(fp));
               KEYS ("ShortDescr", obj->short_descr, fread_string (fp));
               KEYS ("ShD", obj->short_descr, fread_string (fp));
               KEY ("Size", obj->size, fread_number (fp));
               if (!str_cmp (word, "Spell"))
               {
                    int iValue;
                    int sn;
                    iValue = fread_number (fp);
                    sn = skill_lookup (fread_word (fp));
                    if (iValue < 0 || iValue > 3)
                    {
                         bugf ("Fread_obj: bad iValue %d.", iValue);
                    }
                    else if (sn < 0)
                    {
                         bugf ("Fread_obj: unknown skill.");
                    }
                    else
                    {
                         obj->value[iValue] = sn;
                    }
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'T':
               KEY ("Time", obj->timer, fread_number (fp));
               break;
          case 'V':
               if (!str_cmp (word, "Val"))
               {
                    obj->value[0] = fread_number (fp);
                    obj->value[1] = fread_number (fp);
                    obj->value[2] = fread_number (fp);
                    obj->value[3] = fread_number (fp);
                    obj->value[4] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "ValOrig"))
               {
                    obj->valueorig[0] = fread_number (fp);
                    obj->valueorig[1] = fread_number (fp);
                    obj->valueorig[2] = fread_number (fp);
                    obj->valueorig[3] = fread_number (fp);
                    obj->valueorig[4] = fread_number (fp);
                    fMatch = TRUE;
                    break;
               }
               if (!str_cmp (word, "Vnum"))
               {
                    int vnum;
                    vnum = fread_number (fp);
                    if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
                         bugf ("Fread_obj: bad vnum %d.", vnum);
                    else
                         fVnum = TRUE;
                    fMatch = TRUE;
                    break;
               }
               break;
          case 'W':
               KEY ("WearFlags", obj->wear_flags, fread_number (fp));
               KEY ("WeaF", obj->wear_flags, fread_number (fp));
               KEY ("WearLoc", obj->wear_loc, fread_number (fp));
               KEY ("Wear", obj->wear_loc, fread_number (fp));
               KEY ("Weight", obj->weight, fread_number (fp));
               KEY ("Wt", obj->weight, fread_number (fp));
               break;
          }
          if (!fMatch)
          {
               bugf ( "fread_obj: No Match: %s", word );
               fread_to_eol (fp);
          }
     }
}
