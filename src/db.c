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
#include "db.h"
#include <stdarg.h>



extern void         boot_done ( void );
extern int flag_lookup args((const char *name, const struct flag_type *flag_table));

/*
 * Globals.
 */

FILE               *time_file;
FILE               *clan_file;
HELP_DATA          *help_first;
HELP_DATA          *help_last;

SHOP_DATA          *shop_first;
SHOP_DATA          *shop_last;

PROG_CODE *	   mprog_list;
PROG_CODE *        oprog_list;
PROG_CODE *        rprog_list;

CHAR_DATA          *char_free;
EXTRA_DESCR_DATA   *extra_descr_free;
NOTE_DATA          *note_free;
OBJ_DATA           *obj_free;
PC_DATA            *pcdata_free;

CHAR_DATA          *char_list;
char               *help_greeting;
char			   *help_pueblo_greeting;
KILL_DATA           kill_table[MAX_LEVEL];
OBJ_DATA           *object_list;
TIME_INFO_DATA      time_info;
WEATHER_DATA        weather_info;
MUD_DATA            mud;

sh_int              gsn_backstab;
sh_int              gsn_circle;
sh_int              gsn_dodge;
sh_int              gsn_envenom;
sh_int              gsn_hide;
sh_int              gsn_peek;
sh_int              gsn_pick_lock;
sh_int              gsn_sneak;
sh_int              gsn_steal;

sh_int              gsn_disarm;
sh_int              gsn_enhanced_damage;
sh_int              gsn_ultra_damage;
sh_int              gsn_kick;
sh_int              gsn_parry;
sh_int              gsn_rescue;
sh_int              gsn_rotate;
sh_int              gsn_second_attack;
sh_int              gsn_sharpen;
sh_int              gsn_third_attack;
sh_int              gsn_fourth_attack;	/* Lotherius */
sh_int              gsn_fifth_attack;	/* Lotherius */

sh_int              gsn_blindness;
sh_int              gsn_charm_person;
sh_int              gsn_curse;
sh_int              gsn_invis;
sh_int              gsn_mass_invis;
sh_int              gsn_poison;
sh_int              gsn_plague;
sh_int              gsn_sleep;

/* new gsns */

sh_int              gsn_axe;
sh_int              gsn_dagger;
sh_int              gsn_flail;
sh_int              gsn_mace;
sh_int              gsn_polearm;
sh_int              gsn_shield_block;
sh_int              gsn_spear;
sh_int              gsn_sword;
sh_int              gsn_whip;

sh_int              gsn_bash;
sh_int              gsn_berserk;
sh_int		    gsn_rally;
sh_int              gsn_dual;
sh_int              gsn_dirt;
sh_int              gsn_hand_to_hand;
sh_int              gsn_trip;

sh_int              gsn_fast_healing;
sh_int              gsn_haggle;
sh_int              gsn_lore;
sh_int              gsn_recruit;
sh_int              gsn_meditation;

sh_int              gsn_scrolls;
sh_int              gsn_staves;
sh_int              gsn_wands;
sh_int              gsn_recall;

/* Language GSN's */
sh_int              gsn_lang_human;
sh_int              gsn_lang_dwarf;
sh_int              gsn_lang_elf;
sh_int              gsn_lang_giant;
sh_int              gsn_lang_gargoyle;
sh_int              gsn_lang_kobold;
sh_int              gsn_lang_centaur;
sh_int              gsn_lang_azer;
sh_int              gsn_lang_kender;
sh_int              gsn_lang_common;

/*
 * Locals.
 */
MOB_INDEX_DATA     *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA     *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA    *room_index_hash[MAX_KEY_HASH];

/* 4/10/99 Zeran - commented out string_hash...using ssm code now */
/*char *			string_hash		[MAX_KEY_HASH];*/

AREA_DATA          *area_first;
AREA_DATA          *area_last;

extern char         str_empty       [1];

int                 top_affect;
int                 top_area;
int                 top_ed;
int                 top_exit;
int                 top_help;
int                 top_mob_index;
int                 top_obj_index;
int                 top_reset;
int                 top_room;
int                 top_shop;
int                 top_vnum_room;	/* OLC */
int                 top_vnum_mob;	/* OLC */
int                 top_vnum_obj;	/* OLC */
int	            top_mprog_index;	/* MProgs */
int                 top_oprog_index; // oprogs
int                 top_rprog_index; // rprogs
int                 mobile_count = 0;
int                 newmobs = 0;
int                 newobjs = 0;

/*
 * Memory management.
 */

extern int          MAX_STRING;

void               *rgFreeList[MAX_MEM_LIST];
const int           rgSizeList[MAX_MEM_LIST] =
{
     4, 8, 16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64
};

extern int          nAllocString;
extern int          sAllocString;
extern int          nOverFlowString;
extern int          sOverFlowString;
extern bool         Full;
int                 nAllocPerm;
int                 sAllocPerm;

/*
 * Semi-locals.
 */
bool                fBootDb;
bool		    fImportDb;
FILE               *fpArea;
char                strArea[MAX_INPUT_LENGTH];

/* 4/10/99 Zeran - added for ssm code */

extern int          MAX_STRING;
void                init_string_space args ( ( void ) );

/*
 * Local booting procedures.
*/
void init_mm		args ( ( void ) );
void load_helps     	args ( ( void ) );
void load_socials   	args ( ( void ) );
void load_disable   	args ( ( void ) );
void load_crier     	args ( ( void ) );
void load_accounts  	args ( ( void ) );
void reset_area     	args ( ( AREA_DATA * pArea, bool force ) );
void update_obj_orig 	args ( ( OBJ_DATA * obj ) );
void save_class 	args ( ( int num ) );
void save_classes	args ( ( void ) );
void load_class		args ( ( int num ) );
void load_classes	args ( ( void ) );
void load_leases	args ( ( void ) );

ROOM_INDEX_DATA *get_random_room args ( ( int filt, CHAR_DATA *ch ) );

#if !defined (WIN32)
/* RT max open files fix */
void maxfilelimit (  )
{
     struct rlimit       r;

     getrlimit ( RLIMIT_NOFILE, &r );
     r.rlim_cur = r.rlim_max;
     setrlimit ( RLIMIT_NOFILE, &r );
}
#endif

/*
 * Big mama top level function.
 */
void boot_db ( void )
{

#if !defined (WIN32)
    /* open file fix */
     maxfilelimit (  );
#endif

    /*
     * Init some data space stuff.
     */
     {
          init_string_space (  );
          fBootDb = TRUE;
     }

    /*
     * Init random number generator.
     */
     {
          init_mm (  );
     }

    /*
     * Set time and weather.
     */
     {
          long                lhour, lday, lmonth;
          int                 total;
          bool                fail = FALSE;

          /* Zeran:  Try and load time from file first */
          if ( ( time_file = fopen ( TIME_FILE, "r" ) ) != NULL )
          {
               total = fscanf ( time_file, "%d %d %d %d",
                                &( time_info.hour ),
                                &( time_info.day ),
                                &( time_info.month ),
                                &( time_info.year ) );
               if ( total < 4 )
               {
                    bugf ( "loading stored time failed, using default" );
                    fail = TRUE;
               }
               fclose ( time_file );
          }
          else
          {
               bugf ( "failed open of time_file, loading default time." );
               fail = TRUE;
          }

          if ( fail )
          {
               lhour = ( current_time - 650336715 ) / ( PULSE_TICK / PULSE_PER_SECOND );
               time_info.hour = lhour % 24;
               lday = lhour / 24;
               time_info.day = lday % 35;
               lmonth = lday / 35;
               time_info.month = lmonth % 17;
               time_info.year = lmonth / 17;
          }

          if ( time_info.hour < 5 )
               weather_info.sunlight = SUN_DARK;
          else if ( time_info.hour < 6 )
               weather_info.sunlight = SUN_RISE;
          else if ( time_info.hour < 19 )
               weather_info.sunlight = SUN_LIGHT;
          else if ( time_info.hour < 20 )
               weather_info.sunlight = SUN_SET;
          else
               weather_info.sunlight = SUN_DARK;

          weather_info.change = 0;
          weather_info.mmhg = 960;
          if ( time_info.month >= 7 && time_info.month <= 12 )
               weather_info.mmhg += number_range ( 1, 50 );
          else
               weather_info.mmhg += number_range ( 1, 80 );

          if ( weather_info.mmhg <= 980 )
               weather_info.sky = SKY_LIGHTNING;
          else if ( weather_info.mmhg <= 1000 )
               weather_info.sky = SKY_RAINING;
          else if ( weather_info.mmhg <= 1020 )
               weather_info.sky = SKY_CLOUDY;
          else
               weather_info.sky = SKY_CLOUDLESS;

     }

    /*
     * Assign gsn's for skills which have them.
     */
     {
          int                 sn;

          for ( sn = 0; sn < MAX_SKILL; sn++ )
          {
               if ( skill_table[sn].pgsn != NULL )
                    *skill_table[sn].pgsn = sn;
          }
     }

    /*
     * Read in all the area files. Also Load Class Table.
     */
     {
          FILE               *fpList;
          char		     filename[MIL];

          log_string ( "Booting Areas" );

          if ( ( fpList = fopen ( AREA_LIST, "r" ) ) == NULL )
          {
               perror ( AREA_LIST );
               exit ( 1 );
          }

          for ( ;; )
          {
               strcpy ( filename, fread_word ( fpList ) );	/* Get an areaname from fplist */
               if ( filename[0] == '$' )
                    break;
               db_load_area ( filename );                                             
          }
          fclose ( fpList );
     }

     log_string ( "Area Load Complete" );

     log_string ( "Loading Socials" );			load_socials ( );
     log_string ( "Loading Helps" );			load_helps ( );
     log_string ( "Loading Clans" );     		boot_clans ( ); /* Clans must ALWAYS be loaded before leases. */
     log_string ( "Loading Class Table" );      	load_classes (  );
     log_string ( "Loading Skill/Spell Information" );  load_skills (  );
     log_string ( "Loading Lease Information" );	load_leases (  );
     log_string ( "Loading Disabled Commands" );        load_disable (  );
     log_string ( "Loading Crier Messages" );           load_crier (  );
     log_string ( "Loading Accounts" );                 load_accounts ( );

    /*
     * Fix up exits.
     * Declare db booting over.
     * Convert all old_format objcets to new_format, ROM OLC
     * You know all this old and new format junk has to go someday - Lotherius
     * Reset all areas once.
     */
     {
          fix_exits (  );
          fix_mobprogs( );
          fix_objprogs( );
          fix_roomprogs( );
          init_racial_affects (  );
          fBootDb = FALSE;
          area_update (  );
          load_boards (  );
          save_notes (  );
          /* 4/10/99 Zeran - added boot_done() for ssm code */
          boot_done (  );
     }

     if ( !help_greeting )	/* Hugin */
     {
          bugf ( "boot_db: No help_greeting read." );
          help_greeting = "(Text Greeting \"help_greeting\" is missing!)\r\nBy what name do you wish to be known ? ";
     }
     if ( !help_pueblo_greeting )
     {
          bugf ( "boot_db: No help_pueblo_greeting read." );
          help_pueblo_greeting = "(Graphical Greeting (MXP/Pueblo) \"help_pueblo_greeting\" is missing!)\r\nBy what name do you wish to be known ? ";
     }

     return;
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
				}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum ( int vnum )
{
     if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
          area_last->lvnum = area_last->uvnum = vnum;
     if ( vnum !=
          URANGE ( area_last->lvnum, vnum, area_last->uvnum ) )
     {
          if ( vnum < area_last->lvnum )
               area_last->lvnum = vnum;
          else
               area_last->uvnum = vnum;
     }
     return;
}

/*
 * Snarf a help section.
 */
void load_helps ( void )
{
     HELP_DATA          *pHelp;

     SNP ( strArea, "%s%s", DATA_DIR, HELP_FILE );
          
     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "load_helps: Failed to open %s", strArea );
          perror ( strArea );
          return;
     }

     for ( ;; )
     {
          pHelp = alloc_perm ( sizeof ( *pHelp ), "pHelp:load_helps" );
          pHelp->level = fread_number ( fpArea );
          pHelp->keyword = fread_string ( fpArea );
          if ( pHelp->keyword[0] == '$' )
               break;
          pHelp->text = fread_string ( fpArea );

          if ( !str_cmp ( pHelp->keyword, "greeting" ) )
               help_greeting = pHelp->text;
          if ( !str_cmp ( pHelp->keyword, "pueblo_greeting" ) )
               help_pueblo_greeting = pHelp->text;

          if ( help_first == NULL )
               help_first = pHelp;
          if ( help_last != NULL )
               help_last->next = pHelp;

          help_last = pHelp;
          pHelp->next = NULL;
          top_help++;
     }

    strArea[0] = '\0';
    fclose ( fpArea );
     
     return;
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset ( ROOM_INDEX_DATA * pR, RESET_DATA * pReset )
{
     RESET_DATA         *pr;

     if ( !pR )
          return;

     pr = pR->reset_last;

     if ( !pr )
     {
          pR->reset_first = pReset;
          pR->reset_last = pReset;
     }
     else
     {
          pR->reset_last->next = pReset;
          pR->reset_last = pReset;
          pR->reset_last->next = NULL;
     }
     top_reset++;
     return;
}

/*
 *  Translate mobprog vnums pointers to real code
 */
void fix_mobprogs( void )
{
     MOB_INDEX_DATA *pMobIndex;
     PROG_LIST        *list;
     PROG_CODE        *prog;
     int iHash;

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pMobIndex   = mob_index_hash[iHash];
                pMobIndex   != NULL;
                pMobIndex   = pMobIndex->next )
          {
               for( list = pMobIndex->mprogs; list != NULL; list = list->next )
               {
                    if ( ( prog = get_prog_index( list->vnum, PRG_MPROG ) ) != NULL )
                         list->code = prog->code;
                    else
                    {
                         bugf( "Fix_mobprogs: code vnum %d not found.", list->vnum );
                         exit( 1 );
                    }
               }
          }
     }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 * 
 * Now has the sense to convert invalid exits to a limbo exit thus preventing trouble - Lotherius.
 */
void fix_exits ( void )
{
     ROOM_INDEX_DATA    *pRoomIndex;
     EXIT_DATA          *pexit;
     int                 iHash;
     int                 door;

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL; pRoomIndex = pRoomIndex->next )
          {
               bool                fexit;

               fexit = FALSE;
               for ( door = 0; door <= 5; door++ )
               {
                    if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
                    {
                         if ( pexit->u1.vnum <= 0 || get_room_index ( pexit->u1.vnum ) == NULL )
                         {
                              bugf ( "Converting NULL exit in room %d to void exit.\n\r", pRoomIndex->vnum );
                              pexit->u1.vnum = 1;
                              pexit->u1.to_room = get_room_index ( 1 );
                         }
                         else
                         {
                              fexit = TRUE;
                              pexit->u1.to_room = get_room_index ( pexit->u1.vnum );
                         }
                    }
               }
               if ( !fexit )
                    SET_BIT ( pRoomIndex->room_flags, ROOM_NO_MOB );
          }
     }
     return;
}

/*
 * Repopulate areas periodically.
 */
void area_update ( void )
{
     AREA_DATA          *pArea;
     ROOM_INDEX_DATA    *pRoomIndex;

     for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
     {
          if ( ++pArea->age < 3 )
               continue;
		  /*
		   * Check age and reset.
		   */
          if ( ( !pArea->empty && ( pArea->nplayer == 0 || pArea->age >= 15 ) ) || pArea->age >= 31 )
          {
               reset_area ( pArea, FALSE );
               pArea->age = number_range ( 0, 3 );

               pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
               if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
                    pArea->age = 17;

               if ( pArea->nplayer == 0 )
                    pArea->empty = TRUE;
          }
     }
     return;
}

/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room ( ROOM_INDEX_DATA * pRoom, bool force )
{
     RESET_DATA         *pReset;
     CHAR_DATA          *pMob;
     OBJ_DATA           *pObj;
     CHAR_DATA          *LastMob = NULL;
     OBJ_DATA           *LastObj = NULL;
     int                 iExit;
     int                 level = 0;
     bool                last;

     if ( !pRoom )
          return;

     pMob = NULL;
     last = FALSE;

     for ( iExit = 0; iExit < MAX_DIR; iExit++ )
     {
          EXIT_DATA          *pExit;

          if ( ( pExit = pRoom->exit[iExit] )
	     /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */
               )
          {
               pExit->exit_info = pExit->rs_flags;
               if ( ( pExit->u1.to_room != NULL )
                    && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
               {
		/* nail the other side */
                    pExit->exit_info = pExit->rs_flags;
               }
          }
     }

     for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
     {
          ROOM_INDEX_DATA    *pRoomIndex;
          MOB_INDEX_DATA     *pMobIndex;
          OBJ_INDEX_DATA     *pObjIndex;
          OBJ_INDEX_DATA     *pObjToIndex;
          OBJ_DATA           *tmp;
          bool                obj_match = FALSE;

          switch ( pReset->command )
          {
          default:
               bugf ( "Reset_room: bad command %c.", pReset->command );
               break;

          case 'M':
               if ( !( pMobIndex = get_mob_index ( pReset->arg1 ) ) )
               {
                    bugf ( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                    continue;
               }

               if ( pReset->count >= pReset->arg2 )
               {
                    last = FALSE;
                    break;
               }

               pMob = create_mobile ( pMobIndex );
               pMob->reset = pReset;	/* Zeran - added */
               pReset->count++;
               pMobIndex->count++;

	    /*
	     * Some more hard coding.
	     */
               if ( room_is_dark ( pRoom ) )
                    SET_BIT ( pMob->detections, DET_INFRARED );
         /*
          * Pet shop mobiles get ACT_PET set.
          */
               {
                    ROOM_INDEX_DATA    *pRoomIndexPrev;

                    pRoomIndexPrev =
                         get_room_index ( pRoom->vnum - 1 );
                    if ( pRoomIndexPrev &&
                         IS_SET ( pRoomIndexPrev->room_flags,
                                  ROOM_PET_SHOP ) )
                         SET_BIT ( pMob->act, ACT_PET );
               }

               char_to_room ( pMob, pRoom );
               LastMob = pMob;
               level = URANGE ( 0, pMob->level - 2, LEVEL_HERO - 1 );	/* -1 ROM */

               /* do telepops! */
               if ( IS_SET ( pMob->act, ACT_TELEPOP ) )
               {
                    int                 low, high;
                    ROOM_INDEX_DATA    *randroom = NULL;

                    REMOVE_BIT ( pMob->act, ACT_TELEPOP );
                    low = 0;
                    high = MAX_VNUM;
                    
                    /* For mobs that stay in their own area... */                    
                    if ( ( IS_SET ( pMob->act, ACT_STAY_AREA ) )
                         || ( IS_SET ( pMob->act, ACT_SENTINEL ) ) )
                    {
                         {
                              AREA_DATA          *inarea;
                              
                              inarea = pMob->in_room->area;
                              low = inarea->lvnum;
                              high = inarea->uvnum;
                         }
                         for ( ;; )
                         {
                              randroom = get_room_index ( number_range ( low, high ) );
                              if ( randroom != NULL )
                                   if ( can_see_room ( pMob, randroom )
                                        && !IS_SET ( randroom->room_flags,
                                                     ROOM_NOTELEPOP ) &&
                                        !IS_SET ( randroom->room_flags,
                                                  ROOM_PRIVATE ) &&
                                        !IS_SET ( randroom->room_flags,
                                                  ROOM_NO_MOB ) &&
                                        !IS_SET ( randroom->room_flags,
                                                  ROOM_GODS_ONLY ) &&
                                        !IS_SET ( randroom->room_flags,
                                                  ROOM_RENT ) &&
                                        !IS_SET ( randroom->room_flags,
                                                  ROOM_SOLITARY ) )
                                        break;
                         }
                    } 
                    else /* For mobs that globally telepop */
                    {
                         int counter = 0;
                         while ( randroom == NULL && counter <= 5000 )
                         {
                              counter++;
                              randroom = get_random_room ( 2, pMob );
                         }
                         if ( randroom == NULL )
                         {
                              bugf ( "Couldn't find a random room after 5000 tries, MUST get this. Bailing." );
                              exit(1);
                         }
                    }
                    /* end of for loop */
                    char_from_room ( pMob );
                    char_to_room ( pMob, randroom );
               }
               /* end of telepop */
               last = TRUE;
               break;

          case 'O':
               if ( !( pObjIndex = get_obj_index ( pReset->arg1 ) ) )
               {
                    bugf ( "Reset_room: 'O': bad vnum %d.", pReset->arg1 );
                    continue;
               }

               if ( !( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
               {
                    bugf ( "Reset_room: 'O': bad vnum %d.", pReset->arg3 );
                    continue;
               }

               if ( !force && pRoom->area->nplayer > 0 )
                    break;
               if ( pReset->count >= pReset->arg2 )
                    break;
               for ( tmp = pRoom->contents; tmp != NULL;
                     tmp = tmp->next_content )
               {
                    if ( tmp->reset == pReset )
                         obj_match = TRUE;
               }
               if ( obj_match )
                    break;

	    /* Zeran - add percentage repop check */
               if ( number_percent (  ) > pObjIndex->repop )
                    break;

               pObj = create_object ( pObjIndex,	/* UMIN - ROM OLC */
                                      UMIN ( number_fuzzy ( level ),
                                             LEVEL_HERO - 1 ) );
               pObj->reset = pReset;
               pObj->reset->count++;
               pObj->cost = 0;
               pObj->size = SIZE_UNKNOWN;	/* Zeran - leave size unknown */
               obj_to_room ( pObj, pRoom );
               break;

          case 'P':
               if ( !( pObjIndex = get_obj_index ( pReset->arg1 ) ) )
               {
                    bugf ( "Reset_room: 'P': bad vnum %d.", pReset->arg1 );
                    continue;
               }

               if ( !
                    ( pObjToIndex = get_obj_index ( pReset->arg3 ) ) )
               {
                    bugf ( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                    continue;
               }

               if ( ( !force && pRoom->area->nplayer > 0 )
                    || !( LastObj = get_obj_type ( pObjToIndex ) ) )
                    break;
               if ( pReset->count >= pReset->arg2 )
                    break;
	    /* Zeran - add percentage repop check */
               if ( number_percent (  ) > pObjIndex->repop )
                    break;
               for ( tmp = LastObj->contains; tmp != NULL;
                     tmp = tmp->next_content )
               {
                    if ( tmp->reset == pReset )
                    {
                         obj_match = TRUE;
                         break;
                    }
               }
               if ( obj_match )
                    break;

               /* lastObj->level  -  ROM */
               pObj = create_object ( pObjIndex, number_fuzzy ( LastObj->level ) );
               pObj->reset = pReset;
               pObj->reset->count++;
               obj_to_obj ( pObj, LastObj );
               break;
          case 'G':
          case 'E':
               if ( !( pObjIndex = get_obj_index ( pReset->arg1 ) ) )
               {
                    bugf ( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                    continue;
               }

               if ( !last )
                    break;

               if ( !LastMob )
               {
                    bugf ( "Reset_room: 'E' or 'G': null mob for vnum %d.", pReset->arg1 );
                    last = FALSE;
                    break;
               }
	    /* Zeran - check for already being equipped */
               if ( ( pReset->command == 'E' )
                    && ( get_eq_char ( LastMob, pReset->arg3 ) !=
                         NULL ) )
                    break;

	    /* Zeran - add percentage repop check */
               if ( number_percent (  ) > pObjIndex->repop )
                    break;

               if ( LastMob->pIndexData->pShop )	/* Shop-keeper? */
               {
                    int                 olevel;

                    switch ( pObjIndex->item_type )
                    {
                    default:
                         olevel = 0;
                         break;
                    case ITEM_PILL:
                         olevel = number_range ( 0, 10 );
                         break;
                    case ITEM_POTION:
                         olevel = number_range ( 0, 10 );
                         break;
                    case ITEM_SCROLL:
                         olevel = number_range ( 5, 15 );
                         break;
                    case ITEM_WAND:
                         olevel = number_range ( 10, 20 );
                         break;
                    case ITEM_STAFF:
                         olevel = number_range ( 15, 25 );
                         break;
                    case ITEM_ARMOR:
                         olevel = number_range ( 5, 15 );
                         break;
		    /* ROM patch weapon, treasure */
                    case ITEM_WEAPON:
                         olevel = number_range ( 5, 15 );
                         break;
                    case ITEM_TREASURE:
                         olevel = number_range ( 10, 20 );
                         break;

                         break;
                    }

                    pObj = create_object ( pObjIndex, olevel );
                    pObj->reset = pReset;
                    pObj->reset->count++;
                    SET_BIT ( pObj->extra_flags, ITEM_INVENTORY );	/* ROM OLC */

               }
               else		/* ROM OLC else version */
               {
                    int                 limit;

                    if ( pReset->arg2 == -1 )	/* no limit */
                         limit = 9999;
                    else
                         limit = pReset->arg2;
#ifdef DEBUGINFO
                    {
                         
                         log_string ( "limit = %d\n", limit );
                         log_string ( "count = %d\n", pReset->count );
                    }
#endif
                    if ( pReset->count < limit ||
                         number_range ( 0, 4 ) == 0 )
                    {
                         pObj = create_object ( pObjIndex,
                                                UMIN ( number_fuzzy
                                                       ( level ),
                                                       LEVEL_HERO -
                                                       1 ) );
                    }
                    else
                         break;
               }

	    /* Zeran - set object size equal to mob's size */
               pObj->reset = pReset;
               pObj->reset->count++;
               pObj->size = LastMob->size;

               obj_to_char ( pObj, LastMob );
               if ( pReset->command == 'E' )
                    equip_char ( LastMob, pObj, pReset->arg3 );
               last = TRUE;
               break;

          case 'D':
               break;

          case 'R':
               if ( !( pRoomIndex = get_room_index ( pReset->arg1 ) ) )
               {
                    bugf ( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
                    continue;
               }
               {
                    EXIT_DATA          *pExit;
                    int                 d0;
                    int                 d1;

                    for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                    {
                         d1 = number_range ( d0, pReset->arg2 - 1 );
                         pExit = pRoomIndex->exit[d0];
                         pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                         pRoomIndex->exit[d1] = pExit;
                    }
               }
               break;
          }
     }
     // According to the patch, these go into area_update... wtf... I don't wanna cycle
     // through ALL the areas updating rooms, THEN check all the rooms again for triggers.
     // This is much better. - Lotherius
     //
     if ( HAS_TRIGGER_ROOM( pRoom, TRIG_DELAY ) && pRoom->rprog_delay > 0 )
     {
          if ( --pRoom->rprog_delay <= 0 )
               p_percent_trigger( NULL, NULL, pRoom, NULL, NULL, NULL, TRIG_DELAY );
     }
     else if ( HAS_TRIGGER_ROOM( pRoom, TRIG_RANDOM ) )
          p_percent_trigger( NULL, NULL, pRoom, NULL, NULL, NULL, TRIG_RANDOM );
     return;
}

/*
 * Reset one area.
 */

void reset_area ( AREA_DATA * pArea, bool force )
{
     ROOM_INDEX_DATA    *pRoom;
     int                 vnum;

     for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
          if ( ( pRoom = get_room_index ( vnum ) ) )
          {
               reset_room ( pRoom, force );
          }
     }
	 /* Zeran - notify_message */
     notify_message ( NULL, NOTIFY_REPOP, TO_ALL, pArea->name );
     notify_message ( NULL, WIZNET_RESET, TO_IMM, pArea->name );
     return;
}

/* experimental random object code - Lotherius */

#define nelems(a) (sizeof (a)/sizeof (a)[0])

// Calculate a meaningful modifier and amount
void random_apply ( OBJ_DATA * obj, CHAR_DATA * mob )
{
     static int          attrib_types[] =
     {
          APPLY_STR, APPLY_DEX, APPLY_DEX, APPLY_INT, APPLY_INT, APPLY_WIS, APPLY_CON, APPLY_CON, APPLY_CON
     };
     static int          power_types[] =
     {
          APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_AC
     };
     static int          combat_types[] =
     {
          APPLY_HITROLL, APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_SPELL, APPLY_SAVING_SPELL, APPLY_SAVING_BREATH
     };

     AFFECT_DATA        *paf = alloc_perm ( sizeof ( *paf ), "paf:random_apply" );

     paf->type = -1;
     paf->duration = -1;
     paf->bitvector = 0;
     paf->next = obj->affected;
     obj->affected = paf;
     switch ( number_bits ( 2 ) )
     {
     case 0:
          paf->location = attrib_types[number_range ( 0, nelems ( attrib_types ) - 1 )];
          paf->modifier = 1;
          break;
     case 1:
          paf->location = power_types[number_range ( 0, nelems ( power_types ) - 1 )];
          paf->modifier = number_range ( mob->level / 2, mob->level );
          break;
     case 2:
          /* FALLTHROUGH */
     case 3:
          paf->location = combat_types[number_range ( 0, nelems ( combat_types ) - 1 )];
          paf->modifier = number_range ( 1, mob->level / 6 + 1 );
          break;
     }
     SET_BIT ( obj->extra_flags, ITEM_MAGIC );
     obj->enchanted = TRUE;

     // Is item cursed?
     if ( number_percent (  ) <= 5 )
     {
          paf->modifier = -paf->modifier;
          SET_BIT ( obj->extra_flags, ITEM_NODROP );
          if ( number_percent (  ) <= 15 )
               SET_BIT ( obj->extra_flags, ITEM_NOREMOVE );
     }
}

// Jewelry stuff
static char        *adj1[] =
{
     "a splendid", "an ancient", "a dusty", "a scratched",
          "a flawed", "a burnt", "a heavy", "a gilded", "a spooky", "a flaming",
          "a plain", "an ornate", "an inscrutable", "an obscene", "a wrinkled"
};
static char        *adj2[] =
{
     "diamond", "emerald", "topaz", "wooden", "jade",
          "white gold", "onyx", "tin", "glass", "marble", "black",
          "granite"
};

// Anything wearable, and trinkets
void wield_random_armor ( CHAR_DATA * mob )
{
     int                 item_type = number_range ( 30,47 );	/* template from LIMBO.ARE */
     OBJ_INDEX_DATA     *pObjIndex = get_obj_index ( item_type );
     OBJ_DATA           *obj =create_object ( pObjIndex, number_fuzzy ( mob->level ) );
     int                 n_adj1 =number_range ( 0, nelems ( adj1 ) - 1 );
     int                 n_adj2 =number_range ( 0, nelems ( adj2 ) - 1 );
     char               *name = str_dup("");
     int                 number;
     int                 type;

     // Armor stuff
     static char        *armor_types[] = { "leather", "studded leather", "bronze", "chain", "plate", "mithril" };
     static int          armor_mul[] = { 1, 3, 2, 5, 10, 10 };
     static int          armor_div[] = { 1, 2, 1, 1, 1, 3 };

     // Weapon stuff
     static char *weapon_types[] =
     {
          "sword", "broadsword", "rapier", "longsword",
               "sword", "short sword", "dagger", "knife", "hammer", "mace", "mace", "whip",
               "spear", "pike", "flail"
     };
     static int weapon_dam[] = { 3, 3, 3, 3, 3, 11, 11, 11, 0, 7, 7, 4, 11, 11, 27 };
     static int weapon_class[] = { 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 4, 7, 3, 8, 6 };

     // Trinket stuff
     static char        *noun[] =
     {
          "pebble", "bauble", "stone", "charm", "fetish", "bone",
               "trinket"
     };
     char                buffer[64];

     char                buf[MAX_STRING_LENGTH];

     if ( obj->item_type == ITEM_ARMOR )
     {
          int ac_type;
          ac_type = URANGE ( 0, (unsigned) mob->level / 5, nelems ( armor_types ) - 1 );
          free_string(name);
          name = str_dup(armor_types[ac_type]);
          obj->weight *= armor_mul[ac_type];
          obj->weight /= armor_div[ac_type];
          // obj->value[0] = mob->level / 5 + 3;
          // obj->value[1] = obj->value[0];
          // obj->value[2] = obj->value[0];
          // obj->value[3] = obj->value[0] *2 /3;
          if ( number_percent (  ) < mob->level / 3 )
               random_apply ( obj, mob );
     }
     else if ( obj->item_type == ITEM_WEAPON )
     {
          int wea_type = number_range ( 0, nelems ( weapon_types ) - 1 );
          free_string(name);
          name = str_dup(weapon_types[wea_type]);
          obj->value[3] = weapon_dam[wea_type];

          number = (mob->level / 15)+1;
          type = (number * 4) + ( (mob->level - ( (number-1)*10 ))/3 );

          obj->value[0] = weapon_class[wea_type];
          obj->value[1] = number;
          obj->value[2] = type;
     }
     else if ( obj->item_type == ITEM_TREASURE )
     {
          if ( number_percent (  ) < mob->level )
          {
               random_apply ( obj, mob );

               if ( number_percent (  ) < mob->level / 3 )
                    random_apply ( obj, mob );
          }

          if ( obj->wear_flags & ITEM_HOLD )	/* trinket? */
               SNP ( buffer, "%s %s %s", adj1[n_adj1], adj2[n_adj2],
                     noun[number_range ( 0, nelems ( noun ) - 1 )] );
          else			/* no, necklace or something */
               SNP ( buffer, "%s %s", adj1[n_adj1],
                     adj2[n_adj2] );
          free_string(name);
          name = str_dup(buffer);
     }

     SNP( buf, "%s %s", name, obj->short_descr);
     free_string( obj->short_descr );
     obj->short_descr = str_dup( buf );

     if ( !str_cmp ( obj->name, "weapon" ) )
     {
          SNP (buf, "%s", name);
          free_string ( obj->short_descr );
          obj->short_descr = str_dup ( buf );
          free_string ( obj->name );
          obj->name = str_dup ( name );
     }

     free_string(name);

     free_string ( obj->description );
     SNP ( buf, "%s lies here.", obj->short_descr );
     obj->description = str_dup ( buf );
     obj->description[0] = toupper(obj->description[0]);

     obj->level = mob->level;
     update_obj_orig ( obj );
     obj->cost = mob->level * 39;
     obj_to_char ( obj, mob );
     //     equip_char ( mob, obj, item_type );
     wear_obj ( mob, obj, FALSE );
     return;
}

/* end of added */

/*
 * Create an instance of a mobile.
 */
CHAR_DATA          *create_mobile ( MOB_INDEX_DATA * pMobIndex )
{
     CHAR_DATA          *mob;
     struct char_group  *tmp;
     int                 i;

    /* 4/10/99 Zeran - added mobPrompt for ssm code */

     mobile_count++;

     if ( pMobIndex == NULL )
     {
          bugf ( "Create_mobile: NULL pMobIndex." );
          exit ( 1 );
     }

     if ( char_free == NULL )
     {
          mob = alloc_perm ( sizeof ( *mob ), "mob:create_mobile" );
     }
     else
     {
          mob = char_free;
          char_free = char_free->next;
     }

     clear_char ( mob );
     mob->pIndexData = pMobIndex;
     mob->reset = NULL;		/* Zeran */

     mob->name = str_dup ( pMobIndex->player_name );		/* OLC */
     mob->short_descr = str_dup ( pMobIndex->short_descr );	/* OLC */
     mob->long_descr = str_dup ( pMobIndex->long_descr );	/* OLC */
     mob->description = str_dup ( pMobIndex->description );	/* OLC */
     mob->spec_fun = pMobIndex->spec_fun;
     mob->mprog_target   = NULL;

    /* Add Self To Group - Kludgy */
     tmp = newgroup();
     mob->group[MAX_GMEMBERS - 1] = tmp;
     tmp->gch = mob;

     mob->leader = mob;

     if ( pMobIndex->new_format )
	/* load in new style */
     {
	/* read from prototype */
          mob->act = pMobIndex->act;
          mob->comm = COMM_NOCHANNELS | COMM_NOSHOUT | COMM_NOTELL;

          mob->affected_by = pMobIndex->affected_by;
          mob->protections = pMobIndex->protections;
          mob->detections = pMobIndex->detections;
          mob->alignment = pMobIndex->alignment;
          mob->level = pMobIndex->level;
          mob->hitroll = pMobIndex->hitroll;
          mob->damroll = pMobIndex->damage[DICE_BONUS];
          mob->max_hit = dice ( pMobIndex->hit[DICE_NUMBER],
                                pMobIndex->hit[DICE_TYPE] )
               + pMobIndex->hit[DICE_BONUS];
          mob->hit = mob->max_hit;
          mob->max_mana = dice ( pMobIndex->mana[DICE_NUMBER],
                                 pMobIndex->mana[DICE_TYPE] )
               + pMobIndex->mana[DICE_BONUS];
          mob->mana = mob->max_mana;
          mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
          mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
          mob->dam_type = pMobIndex->dam_type;
          // for ( i = 0; i < 4; i++ )
          //     mob->armor[i] = pMobIndex->ac[i];
          mob->off_flags = pMobIndex->off_flags;
          mob->imm_flags = pMobIndex->imm_flags;
          mob->res_flags = pMobIndex->res_flags;
          mob->vuln_flags = pMobIndex->vuln_flags;
          mob->start_pos = pMobIndex->start_pos;
          mob->default_pos = pMobIndex->default_pos;
          mob->sex = pMobIndex->sex;
          if ( mob->sex == 3 )	/* random sex */
               mob->sex = number_range ( 1, 2 );
          mob->race = pMobIndex->race;
          mob->speaking = str_dup ( "human" );
          if ( pMobIndex->gold == 0 )
               mob->gold = 0;
          else
               mob->gold = number_range ( pMobIndex->gold / 2,
                                          pMobIndex->gold * 3 / 2 );
          mob->form = pMobIndex->form;
          mob->parts = pMobIndex->parts;
          mob->size = pMobIndex->size;
          mob->material = pMobIndex->material;

	/* computed on the spot */

          for ( i = 0; i < MAX_STATS; i++ )
               mob->perm_stat[i] = UMIN ( 25, 11 + mob->level / 4 );

          if ( IS_SET ( mob->act, ACT_WARRIOR ) )
          {
               mob->perm_stat[STAT_STR] += 3;
               mob->perm_stat[STAT_INT] -= 1;
               mob->perm_stat[STAT_CON] += 2;
          }

          if ( IS_SET ( mob->act, ACT_THIEF ) )
          {
               mob->perm_stat[STAT_DEX] += 3;
               mob->perm_stat[STAT_INT] += 1;
               mob->perm_stat[STAT_WIS] -= 1;
          }

          if ( IS_SET ( mob->act, ACT_CLERIC ) )
          {
               mob->perm_stat[STAT_WIS] += 3;
               mob->perm_stat[STAT_DEX] -= 1;
               mob->perm_stat[STAT_STR] += 1;
          }

          if ( IS_SET ( mob->act, ACT_MAGE ) )
          {
               mob->perm_stat[STAT_INT] += 3;
               mob->perm_stat[STAT_STR] -= 1;
               mob->perm_stat[STAT_DEX] += 1;
          }

          if ( IS_SET ( mob->off_flags, OFF_FAST ) )
               mob->perm_stat[STAT_DEX] += 2;

          mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
          mob->perm_stat[STAT_CON] +=
               ( mob->size - SIZE_MEDIUM ) / 2;
     }
     else			/* read in old format and convert */
     {
          mob->act = pMobIndex->act | ACT_WARRIOR;
          mob->affected_by = pMobIndex->affected_by;
          mob->detections = pMobIndex->detections;
          mob->protections = pMobIndex->protections;
          mob->alignment = pMobIndex->alignment;
          mob->level = pMobIndex->level;
          mob->hitroll = pMobIndex->hitroll;
          mob->damroll = 0;
          mob->max_hit = mob->level * 8 + number_range ( mob->level * mob->level / 4,
                                                         mob->level * mob->level );
          mob->max_hit *= .9;
          mob->hit = mob->max_hit;
          mob->max_mana = 100 + dice ( mob->level, 10 );
          mob->mana = mob->max_mana;
          switch ( number_range ( 1, 3 ) )
          {
          case ( 1 ):
               mob->dam_type = 3;
               break;		/* slash */
          case ( 2 ):
               mob->dam_type = 7;
               break;		/* pound */
          case ( 3 ):
               mob->dam_type = 11;
               break;		/* pierce */
          }
          
          //for ( i = 0; i < 3; i++ )
          //   mob->armor[i] = interpolate ( mob->level, 100, -100 );          
          //  mob->armor[3] = interpolate ( mob->level, 100, 0 );
          mob->armor = 0;
          mob->race = pMobIndex->race;
          mob->off_flags = pMobIndex->off_flags;
          mob->imm_flags = pMobIndex->imm_flags;
          mob->res_flags = pMobIndex->res_flags;
          mob->vuln_flags = pMobIndex->vuln_flags;
          mob->start_pos = pMobIndex->start_pos;
          mob->default_pos = pMobIndex->default_pos;
          mob->sex = pMobIndex->sex;
          mob->gold = pMobIndex->gold / 100;
          mob->form = pMobIndex->form;
          mob->parts = pMobIndex->parts;
          mob->size = SIZE_MEDIUM;
          mob->material = 0;

          for ( i = 0; i < MAX_STATS; i++ )
               mob->perm_stat[i] = 11 + mob->level / 4;
     }

     mob->position = mob->start_pos;

    /* link the mob to the world list */
     mob->next = char_list;
     char_list = mob;
     pMobIndex->count++;

/* Let's limit this to sentient mammalian beings only */

     if ( !IS_SET ( mob->act, ACT_NORANDOM ) && IS_SET( mob->form, FORM_MAMMAL )
          && IS_SET(mob->form, FORM_SENTIENT) )
     {
          if ( number_percent (  ) <= 60 )	/* check for random */
          {
               wield_random_armor ( mob );
          }
     }
     return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile ( CHAR_DATA * parent, CHAR_DATA * clone )
{
     int                 i;
     AFFECT_DATA        *paf;

     if ( parent == NULL || clone == NULL || !IS_NPC ( parent ) )
          return;

    /* start fixing values */
     clone->name = str_dup ( parent->name );
     clone->version = parent->version;
     clone->short_descr = str_dup ( parent->short_descr );
     clone->long_descr = str_dup ( parent->long_descr );
     clone->description = str_dup ( parent->description );
     clone->sex = parent->sex;
     clone->race = parent->race;
     clone->level = parent->level;
     clone->trust = 0;
     clone->timer = parent->timer;
     clone->wait = parent->wait;
     clone->hit = parent->hit;
     clone->max_hit = parent->max_hit;
     clone->mana = parent->mana;
     clone->max_mana = parent->max_mana;
     clone->move = parent->move;
     clone->max_move = parent->max_move;
     clone->gold = parent->gold;
     clone->exp = parent->exp;
     clone->act = parent->act;
     clone->comm = parent->comm;
     clone->imm_flags = parent->imm_flags;
     clone->res_flags = parent->res_flags;
     clone->vuln_flags = parent->vuln_flags;
     clone->invis_level = parent->invis_level;
     clone->affected_by = parent->affected_by;
     clone->detections = parent->detections;
     clone->protections = parent->protections;
     clone->position = parent->position;          
     clone->saving_throw = parent->saving_throw;
     clone->alignment = parent->alignment;
     clone->hitroll = parent->hitroll;
     clone->damroll = parent->damroll;
     clone->wimpy = parent->wimpy;
     clone->form = parent->form;
     clone->parts = parent->parts;
     clone->size = parent->size;
     clone->material = parent->material;
     clone->off_flags = parent->off_flags;
     clone->dam_type = parent->dam_type;
     clone->start_pos = parent->start_pos;
     clone->default_pos = parent->default_pos;
     clone->spec_fun = parent->spec_fun;

//     for ( i = 0; i < 4; i++ )
     clone->armor = parent->armor;

     for ( i = 0; i < MAX_STATS; i++ )
     {
          clone->perm_stat[i] = parent->perm_stat[i];
          clone->mod_stat[i] = parent->mod_stat[i];
     }

     for ( i = 0; i < 3; i++ )
          clone->damage[i] = parent->damage[i];

    /* now add the affects */
     for ( paf = parent->affected; paf != NULL; paf = paf->next )
          affect_to_char ( clone, paf );

}

/*
 * Create an instance of an object.
 */
OBJ_DATA           *create_object ( OBJ_INDEX_DATA * pObjIndex,
				    int level )
{
     static OBJ_DATA     obj_zero;
     OBJ_DATA           *obj;

     if ( pObjIndex == NULL )
     {
          bugf ( "Create_object: NULL pObjIndex." );
          exit ( 1 );
     }

     if ( obj_free == NULL )
     {
          obj = alloc_perm ( sizeof ( *obj ), "obj:create_object" );
     }
     else
     {
          obj = obj_free;
          obj_free = obj_free->next;
     }

     *obj = obj_zero;
     obj->pIndexData = pObjIndex;
     obj->in_room = NULL;
     obj->enchanted = FALSE;

     if ( pObjIndex->new_format )
          obj->level = pObjIndex->level;
     else
          obj->level = UMAX ( 0, level );
     obj->wear_loc = -1;
     obj->name = str_dup ( pObjIndex->name );	/* OLC */
     obj->short_descr = str_dup ( pObjIndex->short_descr );	/* OLC */
     obj->description = str_dup ( pObjIndex->description );	/* OLC */

     obj->material = pObjIndex->material;
     obj->item_type = pObjIndex->item_type;
     obj->extra_flags = pObjIndex->extra_flags;
     obj->wear_flags = pObjIndex->wear_flags;
     obj->vflags = pObjIndex->vflags;

     //obj->condition = pObjIndex->condition;
     // Fuzz the condition a bit.
     // make sure it doesn't do shopkeeper items or no condition items.
     if ( !IS_SET ( pObjIndex->extra_flags, ITEM_NO_COND ) &&
          !IS_SET ( pObjIndex->extra_flags, ITEM_INVENTORY ) )
     {
          if ( pObjIndex->condition > 30 ) // Don't do really low condition items
          {
               obj->condition = number_fuzzy (number_fuzzy (pObjIndex->condition) );

               if ( obj->condition <= 0 )
               {
                    if ( pObjIndex->condition > 0 )
                         obj->condition = 1;     // Make sure usable objs don't become unusable
                    else                          // at load time. That would suck.
                         obj->condition = 0;
               }
          }
          else
               obj->condition = pObjIndex->condition;
     }
     else
          obj->condition = pObjIndex->condition;
     if ( obj->condition > 100 )
          obj->condition = 100;
     obj->value[0] = pObjIndex->value[0];
     obj->value[1] = pObjIndex->value[1];
     obj->value[2] = pObjIndex->value[2];
     obj->value[3] = pObjIndex->value[3];
     obj->value[4] = pObjIndex->value[4];
     obj->valueorig[0] = pObjIndex->value[0];
     obj->valueorig[1] = pObjIndex->value[1];
     obj->valueorig[2] = pObjIndex->value[2];
     obj->valueorig[3] = pObjIndex->value[3];
     obj->valueorig[4] = pObjIndex->value[4];
     obj->weight = pObjIndex->weight;
     obj->size = SIZE_UNKNOWN;

     if ( level == -1 || pObjIndex->new_format )
          obj->cost = pObjIndex->cost;
     else
          obj->cost = number_fuzzy ( 10 )
               * number_fuzzy ( level ) * number_fuzzy ( level );

    /*
     * Mess with object properties.
     */
     switch ( obj->item_type )
     {
     default:
          bugf ( "Read_object: vnum %d bad type %d.", pObjIndex->vnum, obj->item_type );
          break;

     case ITEM_LIGHT:
          if ( obj->value[2] == 999 )
               obj->value[2] = -1;
          break;
     case ITEM_TREASURE:
     case ITEM_FURNITURE:
     case ITEM_TRASH:
     case ITEM_CONTAINER:
     case ITEM_DRINK_CON:
     case ITEM_KEY:
     case ITEM_FOOD:
     case ITEM_BOAT:
     case ITEM_CORPSE_NPC:
     case ITEM_CORPSE_PC:
     case ITEM_FOUNTAIN:
     case ITEM_MAP:
     case ITEM_CLOTHING:
     case ITEM_PRIDE:
     case ITEM_COMPONENT:
     case ITEM_PROTECT:
     case ITEM_PORTAL:
     case ITEM_ARMOR:
          break;

     case ITEM_SCROLL:
          if ( level != -1 && !pObjIndex->new_format )
               obj->value[0] = number_fuzzy ( obj->value[0] );
          break;

     case ITEM_WAND:
     case ITEM_STAFF:
          if ( level != -1 && !pObjIndex->new_format )
          {
               obj->value[0] = number_fuzzy ( obj->value[0] );
               obj->value[1] = number_fuzzy ( obj->value[1] );
               obj->value[2] = obj->value[1];
          }
          break;

     case ITEM_WEAPON:
          if ( level != -1 && !pObjIndex->new_format )
          {
               obj->value[1] =
                    number_fuzzy ( number_fuzzy
                                   ( 1 * level / 4 + 2 ) );
               obj->value[2] =
                    number_fuzzy ( number_fuzzy
                                   ( 3 * level / 4 + 6 ) );
          }
          break;
          //     case ITEM_ARMOR:
          //          if ( level != -1 && !pObjIndex->new_format )
          //          {
          //               obj->value[0] = number_fuzzy ( level / 5 + 3 );
          //               obj->value[1] = number_fuzzy ( level / 5 + 3 );
          //               obj->value[2] = number_fuzzy ( level / 5 + 3 );
          //          }
          //          break;
     case ITEM_POTION:
     case ITEM_PILL:
          if ( level != -1 && !pObjIndex->new_format )
               obj->value[0] =
               number_fuzzy ( number_fuzzy ( obj->value[0] ) );
          break;

     case ITEM_MONEY:
          if ( !pObjIndex->new_format )
               obj->value[0] = obj->cost;
          break;
     }

     obj->next = object_list;
     object_list = obj;
     pObjIndex->count++;

     return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object ( OBJ_DATA * parent, OBJ_DATA * clone )
{
     int                 i;
     AFFECT_DATA        *paf;

/*    EXTRA_DESCR_DATA *ed,*ed_new; */

     if ( parent == NULL || clone == NULL )
          return;

    /* start fixing the object */
     clone->name = str_dup ( parent->name );
     clone->short_descr = str_dup ( parent->short_descr );
     clone->description = str_dup ( parent->description );
     clone->item_type = parent->item_type;
     clone->extra_flags = parent->extra_flags;
     clone->wear_flags = parent->wear_flags;
     clone->weight = parent->weight;
     clone->cost = parent->cost;
     clone->level = parent->level;
     clone->condition = parent->condition;
     clone->material = parent->material;
     clone->timer = parent->timer;

     for ( i = 0; i < 5; i++ )
          clone->value[i] = parent->value[i];
     clone->valueorig[i] = parent->value[i];

    /* affects */
     clone->enchanted = parent->enchanted;

     for ( paf = parent->affected; paf != NULL; paf = paf->next )
          affect_to_obj ( clone, paf );

    /* extended desc */

/*
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next);
    {
        ed_new              = alloc_perm( sizeof(*ed_new), "ed:clone" );

        ed_new->keyword    	= str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next           	= clone->extra_descr;
        clone->extra_descr  	= ed_new;
    }
*/

}

/*
 * Clear a new character.
 */
void clear_char ( CHAR_DATA * ch )
{
     static CHAR_DATA    ch_zero;
     int                 i;

     *ch = ch_zero;
     ch->name = &str_empty[0];
     ch->short_descr = &str_empty[0];
     ch->long_descr = &str_empty[0];
     ch->description = &str_empty[0];
     ch->logon = current_time;
     ch->last_note = 0;
     ch->lines = PAGELEN;
     //     for ( i = 0; i < 4; i++ )
     ch->armor = 0;
     ch->comm = 0;
     ch->position = POS_STANDING;     
     ch->hit = 20;
     ch->max_hit = 20;
     ch->mana = 100;
     ch->max_mana = 100;
     ch->move = 100;
     ch->max_move = 100;
     ch->on = NULL;

     for ( i = 0; i < MAX_STATS; i++ )
     {
          ch->perm_stat[i] = 13;
          ch->mod_stat[i] = 0;
     }
     return;
}

/*
 * Free a character.
 */
void free_char ( CHAR_DATA * ch )
{
     OBJ_DATA           *obj;
     OBJ_DATA           *obj_next;
     AFFECT_DATA        *paf;
     AFFECT_DATA        *paf_next;
     struct char_group  *tmp;
     int                 counter;

     if ( IS_NPC ( ch ) )
          mobile_count--;

     for ( obj = ch->carrying; obj != NULL; obj = obj_next )
     {
          obj_next = obj->next_content;
          extract_obj ( obj );
     }

     for ( paf = ch->affected; paf != NULL; paf = paf_next )
     {
          paf_next = paf->next;
          affect_remove ( ch, paf );
     }

     free_string ( ch->name );
     free_string ( ch->short_descr );
     free_string ( ch->long_descr );
     free_string ( ch->description );
     free_string ( ch->poly_name );
     free_string ( ch->short_descr_orig );
     free_string ( ch->long_descr_orig );
     free_string ( ch->description_orig );
     free_string ( ch->speaking );

	 /* Remove all members from group led by char */

     for ( counter = 0; counter < MAX_GMEMBERS; counter++ )
     {
          tmp = ch->group[counter];
          if ( tmp )
          {
               if ( tmp->gch )
                    leave_group(tmp->gch);
               else
                    bugf ( "Unfreed group with NULL gch on free_char. Freeing. (ch: %s)", ch->name );
               if ( tmp )
                    free_group ( tmp );
              ch->group[counter] = NULL;              
          }
     }

     if ( ch->pcdata )
     {
          free_string ( ch->pcdata->pwd );
          free_string ( ch->pcdata->bamfin );
          free_string ( ch->pcdata->bamfout );
          free_string ( ch->pcdata->title );
          free_string ( ch->pcdata->email );
          free_string ( ch->pcdata->immtitle );
          free_string ( ch->pcdata->mplaying );
          free_string ( ch->pcdata->prompt );

          free_i3chardata( ch );         
         
          for ( counter = 0; counter < MAX_ALIAS && ch->pcdata->aliases[counter] != NULL; counter++ )
          {
               free_string ( ch->pcdata->aliases[counter]->name );
               free_string ( ch->pcdata->aliases[counter]->command_string );
               free_mem ( ch->pcdata->aliases[counter], sizeof ( struct alias_data ), "alias_data" );
          }

          ch->pcdata->next = pcdata_free;
          pcdata_free = ch->pcdata;
     }

     ch->reset = NULL;
     ch->searching = FALSE;
     ch->quitting = FALSE;

     ch->next = char_free;
     char_free = ch;
     return;
}

/*
 * Get an extra description from a list.
 */
char               *get_extra_descr ( const char *name,
				      EXTRA_DESCR_DATA * ed )
{
     for ( ; ed != NULL; ed = ed->next )
     {
          if ( is_name ( ( char * ) name, ed->keyword ) )
               return ed->description;
     }
     return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA     *get_mob_index ( int vnum )
{
     MOB_INDEX_DATA     *pMobIndex;

     for ( pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
           pMobIndex != NULL; pMobIndex = pMobIndex->next )
     {
          if ( pMobIndex->vnum == vnum )
               return pMobIndex;
     }

     if ( fBootDb || fImportDb)
     {
          bugf ( "Get_mob_index: bad vnum %d.", vnum );
          exit ( 1 );
     }

     return NULL;
}

/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA     *get_obj_index ( int vnum )
{
     OBJ_INDEX_DATA     *pObjIndex;

     if ( vnum < 0 )
     {
          bugf ( "Get_obj_index: bad vnum %d - error trapped.", vnum );
          return NULL;
     }

     for ( pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
           pObjIndex != NULL; pObjIndex = pObjIndex->next )
     {
          if ( pObjIndex->vnum == vnum )
               return pObjIndex;
     }

     if ( fBootDb || fImportDb )
     {
          bugf ( "Get_obj_index: bad vnum %d.", vnum );
          exit ( 1 );
     }

     return NULL;
}

/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */

ROOM_INDEX_DATA    *get_room_index ( int vnum )
{
     ROOM_INDEX_DATA    *pRoomIndex;

     for ( pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
           pRoomIndex != NULL; pRoomIndex = pRoomIndex->next )
     {
          if ( pRoomIndex->vnum == vnum )
               return pRoomIndex;
     }

     if ( fBootDb || fImportDb )
     {
          bugf ( "Get_room_index: bad vnum %d.", vnum );
          exit ( 1 );
     }

     return NULL;
}

/*
 * get_random_room: Finds a random room with filters
 * 
 * Filters are:
 * 0 No Filter
 * 1 Filter for spell_teleport
 * 2 Filter for global telepop
 * 
 * The function is designed to replace those in spell_teleport and the
 * telepop sections of code which simply chose a vnum randomly. This provides
 * a more even spread of random results over areas, as well as hopefully a little
 * bit quicker selection with the high number of possible vnums now available,
 * since it won't be checking ranges that aren't in areas anymore.
 *
 * If the calling function gets NULL, don't bail immediately - try again.
 * This function is written to give up after so many tries at each step,
 * and the most likely cause of not finding a room is that an area was 
 * selected randomly that didn't have any targetable rooms in it.
 * 
 * It is up to the caller to check for NULL ALWAYS, and reissue the function
 * if needed - Lotherius.
 */

ROOM_INDEX_DATA	*get_random_room ( int filt, CHAR_DATA *ch )
{
     AREA_DATA          *pArea = area_first;
     ROOM_INDEX_DATA    *pRoomIndex;
     int                 i, counter;
     bool		 found = FALSE;

     while ( 1 )
     {
          counter = 0;
          /* Find an area within the level range specified */
          while ( !found && counter < 5000 )
          {
               counter++;
               i = number_range ( 0, top_area );

               if ( filt != 1 )
               {
                    for ( pArea = area_first; pArea; pArea = pArea->next )
                         if ( pArea->vnum == i && pArea->zone > 0 )
                         {
                              found = TRUE;
                              break;
                         }

               }
               else
               {
                    for ( pArea = area_first; pArea; pArea = pArea->next )
                         if ( pArea->vnum == i && pArea->zone > 0 
                              && pArea->llev <= (ch->level+10 ))
                         {
                              found = TRUE;
                              break;
                         }
               }
          }
          /*
           * Not finding an area should be less common, in fact is usually an 
           * indication of something fuxored - Lotherius
           */
          if ( ( !found && counter >= 5000 ) || !pArea )
          {
               bugf ( "Couldn't find an area in get_random_room." );
               return NULL;
          }
          
          /* 
           * Now find a room within that area that's valid. Set counter to prevent looping
           * on areas without any valid rooms forever 
           */
          /* Reset the counter */
          counter = 0;
          while ( counter < 1500 )
          {
               counter++;
               pRoomIndex = get_room_index ( number_range ( pArea->lvnum, pArea->uvnum ) );               
               if ( pRoomIndex != NULL )
               {
                    if ( filt == 0 ) // No filter, valid room so return it.
                         return pRoomIndex;
                    else if ( filt == 1 )
                    {
                         if ( can_see_room ( ch, pRoomIndex ) && !IS_SET ( pRoomIndex->room_flags, ROOM_PRIVATE )
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_SOLITARY ) )
                              return pRoomIndex;
                    }
                    else if ( filt == 2 )
                    {
                         if ( can_see_room ( ch, pRoomIndex ) && !IS_SET ( pRoomIndex->room_flags, ROOM_NOTELEPOP )
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_PRIVATE ) 
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_NO_MOB ) 
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_GODS_ONLY ) 
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_RENT ) 
                              && !IS_SET ( pRoomIndex->room_flags, ROOM_SOLITARY ) )
                              return pRoomIndex;
                    }
               }
          }
          
          /* NOTE: It could be the case here that a target area with no valid rooms was selected.
           * In which case the calling function should try again, but it MUST check for null.
           * 
           * This limit is a little lower than some of the others since it will be more likely
           * to be hit, and thus get a new area which is likely to not be hit :)
           */
          if ( counter >= 1500 )
               return NULL; // Couldn't get a room          
     }
     return NULL;     
}

/*
 * Read a letter from a file.
 */
char fread_letter ( FILE * fp )
{
     char                c;

     do
     {
          c = getc ( fp );
     }
     while ( isspace ( c ) );

     return c;
}

/*
 * Read a number from a file.
 */
int fread_number ( FILE * fp )
{
     int                 number;
     bool                sign;
     char                c;

     do
     {
          c = getc ( fp );
     }
     while ( isspace ( c ) );

     number = 0;

     sign = FALSE;
     if ( c == '+' )
     {
          c = getc ( fp );
     }
     else if ( c == '-' )
     {
          sign = TRUE;
          c = getc ( fp );
     }

     if ( !isdigit ( c ) )
     {
          bugf ( "Fread_number: bad format." );
          exit ( 1 );
     }

     while ( isdigit ( c ) )
     {
          number = number * 10 + c - '0';
          c = getc ( fp );
     }

     if ( sign )
          number = 0 - number;

     if ( c == '|' )
          number += fread_number ( fp );
     else if ( c != ' ' )
          ungetc ( c, fp );

     return number;
}

long fread_flag ( FILE * fp )
{
     int                 number;
     char                c;

     do
     {
          c = getc ( fp );
     }
     while ( isspace ( c ) );

     number = 0;

     if ( !isdigit ( c ) && c != '-' )	/* ROM OLC */
     {
          while ( ( 'A' <= c && c <= 'Z' ) ||
                  ( 'a' <= c && c <= 'z' ) )
          {
               number += flag_convert ( c );
               c = getc ( fp );
          }
     }

     if ( c == '-' )		/* ROM OLC */
     {
          number = fread_number ( fp );
          return -number;
     }

     while ( isdigit ( c ) )
     {
          number = number * 10 + c - '0';
          c = getc ( fp );
     }

     if ( c == '|' )
          number += fread_flag ( fp );

     else if ( c != ' ' )
          ungetc ( c, fp );

     return number;
}

long flag_convert ( char letter )
{
     long                bitsum = 0;
     char                i;

     if ( 'A' <= letter && letter <= 'Z' )
     {
          bitsum = 1;
          for ( i = letter; i > 'A'; i-- )
               bitsum *= 2;
     }
     else if ( 'a' <= letter && letter <= 'z' )
     {

/* Zeran - IDIOTS want 2^26!!! ---> bitsum = 33554432;  2^25 */
          bitsum = 67108864;	/* 2^26 */
          for ( i = letter; i > 'a'; i-- )
               bitsum *= 2;
     }

     return bitsum;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol ( FILE * fp )
{
     char                c;

     do
     {
          c = getc ( fp );
     }
     while ( c != '\n' && c != '\r' );

     do
     {
          c = getc ( fp );
     }
     while ( c == '\n' || c == '\r' );

     ungetc ( c, fp );
     return;
}

/*
 * Read one word (into static buffer).
 */
char               *fread_word ( FILE * fp )
{
     static char         word[MAX_INPUT_LENGTH];
     char               *pword;
     char                cEnd;

     do
     {
          cEnd = getc ( fp );
     }
     while ( isspace ( cEnd ) );

     if ( cEnd == '\'' || cEnd == '"' )
     {
          pword = word;
     }
     else
     {
          word[0] = cEnd;
          pword = word + 1;
          cEnd = ' ';
     }

     for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
     {
          *pword = getc ( fp );
          if ( cEnd == ' ' ? isspace ( *pword ) : *pword == cEnd )
          {
               if ( cEnd == ' ' )
                    ungetc ( *pword, fp );
               *pword = '\0';
               return word;
          }
     }

     // You know, most of the time, the word IS NOT TOO LONG... but it is some other problem, usually a missing word.
     // Some people need to learn to be more specific or some of us might believe their error messages.
     
     bugf ( "Fread_word: Some Error, might be word too long, but just as likely not." );
     exit ( 1 );
     return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem ( int sMem, char *identifier )
{
     void               *pMem;
     int                 iList;

     for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
     {
          if ( sMem <= rgSizeList[iList] )
               break;
     }

     if ( iList == MAX_MEM_LIST )
     {
          bugf ( "Alloc_mem: size %d too large. (%s)", sMem, identifier );
          exit ( 1 );
     }

     memlog ( identifier, ALLOC_MEM, sMem );

     if ( rgFreeList[iList] == NULL )
     {
          pMem = alloc_perm ( rgSizeList[iList], "alloc_mem" );
     }
     else
     {
          pMem = rgFreeList[iList];
          rgFreeList[iList] = *( ( void ** ) rgFreeList[iList] );
     }

     return pMem;
}

/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem ( void *pMem, int sMem, char *identifier )
{
     int                 iList;

     if ( pMem == NULL )
          log_string ( "free_mem: hmmm, pMem=NULL, suspicious?" );

     for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
     {
          if ( sMem <= rgSizeList[iList] )
               break;
     }

     if ( iList == MAX_MEM_LIST )
     {
          bugf ( "Free_mem: size %d too large. (%s)", sMem, identifier );
          exit ( 1 );
     }

     memlog ( identifier, DALLOC_MEM, sMem );

     *( ( void ** ) pMem ) = rgFreeList[iList];
     rgFreeList[iList] = pMem;

     return;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void               *alloc_perm ( int sMem, char *identifier )
{
     static char        *pMemPerm;
     static int          iMemPerm;
     void               *pMem;

     while ( sMem % sizeof ( long ) != 0 )
          sMem++;
     if ( sMem > MAX_PERM_BLOCK )
     {
          bugf ( "Alloc_perm: %d too large. (%s)", sMem, identifier );
          exit ( 1 );
     }

     if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
     {
          iMemPerm = 0;
          if ( ( pMemPerm = calloc ( 1, MAX_PERM_BLOCK ) ) == NULL )
          {
               perror ( "Alloc_perm" );
               exit ( 1 );
          }
     }

     memlog ( identifier, ALLOC_PERM, sMem );

     pMem = pMemPerm + iMemPerm;
     iMemPerm += sMem;
     nAllocPerm += 1;
     sAllocPerm += sMem;
     return pMem;
}

// Memlog by Lotherius
// Errors here will be logged but aren't fatal since this is just logging.
// Perhaps a hash here would be more efficient, but there aren't really that many
// identifiers to cycle.
//
void memlog ( char *identifier, int action, int amount )
{
     bool match = FALSE;
     int i;

     if ( identifier[0] == '\0' )
     {
          bugf ( "memlog requires identifier to function." );
          return;
     }

     if ( action < ALLOC_PERM || action > DALLOC_MEM )
     {
          bugf ( "memlog actions are: ALLOC_PERM, ALLOC_MEM, DALLOC_MEM" );
          return;
     }

     for ( i = 0 ; i < MAX_MEMLOG ; i++ )
     {
          if ( !mud.ml_ident[i] )
               continue;
          if ( !strcmp ( identifier, mud.ml_ident[i]) )
          {
               match = TRUE;
               break;
          }
     }

	 /* Setup a new identifier */

     if ( !match )
     {
          for ( i = 0 ; i < MAX_MEMLOG ; i++ )
          {
               if ( !mud.ml_ident[i] )
               {
                    match = TRUE;
                    mud.ml_ident[i] = str_dup ( identifier );
                    break;
               }
          }
     }

     if ( !match )
     {
          bugf ( "Memlog couldn't find a free identifier. Probably need to increase MAX_MEMLOG." );
          return;
     }

     switch ( action )
     {
     case ALLOC_PERM:
          mud.ml_perm_alloc[i] += amount;
          break;
     case ALLOC_MEM:
          mud.ml_mem_alloc[i] += amount;
          break;
     case DALLOC_MEM:
          mud.ml_mem_dalloc[i] += amount;
          break;
     default:
          bugf ( "memlog:: Invalid action." );
          break;
     }

     return;
}

/*
 * Shows the memlog to an admin.
 * This will always look like the BUFFER and d->outbuf identifiers are slightly out of synch.
 * This is because both of those have memory allocated each time do_memlog is called, to display
 * the output information. As long as they aren't getting any bigger than the size of the list
 * you're displaying, everything should be fine.
 * - Lotherius
 */

void do_memlog ( CHAR_DATA * ch, char *argument )
{
     BUFFER *buffer;
     bool match = FALSE;
     int a = 0;
     int b = 0;
     int c = 0;
     int d = 0;
     int i = 0;

     buffer = buffer_new ( 1024 );

     bprintf ( buffer, "{GMemlog Totals {w--------------------------------------------------------------------------------------->\n\r" );
     for ( i = 0 ; i < MAX_MEMLOG ; i++ )
     {
          if ( (mud.ml_ident[i]) && ( mud.ml_ident[i][0] != '\0' ) )
          {
               bprintf ( buffer, "%27s : Perm [%7d] :: Alloc [%7d] :: Dalloc [%7d] :: Diff [%7d]\n\r", mud.ml_ident[i],
                         mud.ml_perm_alloc[i], mud.ml_mem_alloc[i], mud.ml_mem_dalloc[i],
                         ( mud.ml_mem_alloc[i] - mud.ml_mem_dalloc[i] ) );
               a += mud.ml_perm_alloc[i];
               b += mud.ml_mem_alloc[i];
               c += mud.ml_mem_dalloc[i];
               d += ( mud.ml_mem_alloc[i] - mud.ml_mem_dalloc[i] );
               match = TRUE;
          }
     }
     if ( !match )
          bprintf ( buffer, "None found??\n\r" );
     else
     {
          bprintf ( buffer, "\n\r{Y                      Total : Perm [%7d] :: Alloc [%7d] :: Dalloc [%7d] :: Diff [%7d]\n\r",
                    a, b, c, d );
     }

     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return;
}

void do_memory ( CHAR_DATA * ch, char *argument )
{
     int                 i;

     form_to_char ( ch, "Affects [%5d]    ", top_affect );
     form_to_char ( ch, "Areas   [%5d]    ", top_area );
     form_to_char ( ch, "ExDes   [%5d]\n\r", top_ed );
     form_to_char ( ch, "Exits   [%5d]    ", top_exit );
     form_to_char ( ch, "Helps   [%5d]    ", top_help );
     form_to_char ( ch, "Socials [%5d]\n\r", social_count );
     form_to_char ( ch, "Mobs    [%5d]    ", top_mob_index );
     form_to_char ( ch, "(in use)[%5d]    ", mobile_count );
     form_to_char ( ch, "Objs    [%5d]\n\r", top_obj_index );
     form_to_char ( ch, "Resets  [%5d]    ", top_reset );
     form_to_char ( ch, "Rooms   [%5d]    ", top_room );
     form_to_char ( ch, "Shops   [%5d]\n\r", top_shop );
     form_to_char ( ch, "\n\rPerms   (%5d) blocks  of (%7d) bytes.\n\r", nAllocPerm, sAllocPerm );
     form_to_char ( ch, "\n\rShared String Info:\n\r" );
     form_to_char ( ch, "Shared Strings   (%5d) strings of (%7d) bytes (max %d).\n\r",
                    nAllocString, sAllocString, MAX_STRING );
     form_to_char ( ch, "Overflow Strings (%5d) strings of (%7d) bytes.\n\r",
                    nOverFlowString, sOverFlowString );

     i = defrag_heap ( );
     if ( i > 0 )
     {
          form_to_char ( ch, "Defrag Heap Resulted in %d Merges.\n\r", i );
     }

     if ( Full )
     {
          form_to_char ( ch, "Shared String Heap is full, increase MAX_STRING.\n\r" );
     }
     return;
}

void do_dump ( CHAR_DATA * ch, char *argument )
{
     int                 count, count2, num_pcs, aff_count;
     CHAR_DATA          *fch;
     MOB_INDEX_DATA     *pMobIndex;
     PC_DATA            *pc;
     OBJ_DATA           *obj;
     OBJ_INDEX_DATA     *pObjIndex;
     ROOM_INDEX_DATA    *room;
     EXIT_DATA          *exit;
     DESCRIPTOR_DATA    *d;
     AFFECT_DATA        *af;
     FILE               *fp;
     int                 vnum;
     int nMatch = 0;

    /* open file */
     fclose ( fpReserve );
     fp = fopen ( "mem.dmp", "w" );

    /* report use of data structures */

     num_pcs = 0;
     aff_count = 0;

    /* mobile prototypes */
     fprintf ( fp, "MobProt	%4d (%8d bytes)\n",
               top_mob_index,
               top_mob_index * ( sizeof ( *pMobIndex ) ) );

    /* mobs */
     count = 0;
     count2 = 0;
     for ( fch = char_list; fch != NULL; fch = fch->next )
     {
          count++;
          if ( fch->pcdata != NULL )
               num_pcs++;
          for ( af = fch->affected; af != NULL; af = af->next )
               aff_count++;
     }
     for ( fch = char_free; fch != NULL; fch = fch->next )
          count2++;

     fprintf ( fp, "Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
               count, count * ( sizeof ( *fch ) ), count2,
               count2 * ( sizeof ( *fch ) ) );

    /* pcdata */
     count = 0;
     for ( pc = pcdata_free; pc != NULL; pc = pc->next )
          count++;

     fprintf ( fp,
               "Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
               num_pcs, num_pcs * ( sizeof ( *pc ) ), count,
               count * ( sizeof ( *pc ) ) );

    /* descriptors */
     count = 0;
     count2 = 0;
     for ( d = descriptor_list; d != NULL; d = d->next )
          count++;
     for ( d = descriptor_free; d != NULL; d = d->next )
          count2++;

     fprintf ( fp,
               "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
               count, count * ( sizeof ( *d ) ), count2,
               count2 * ( sizeof ( *d ) ) );

    /* object prototypes */
     for ( vnum = 0; nMatch < top_obj_index; vnum++ )
          if ( ( pObjIndex = get_obj_index ( vnum ) ) != NULL )
          {
               for ( af = pObjIndex->affected; af != NULL;
                     af = af->next )
                    aff_count++;
               nMatch++;
          }

     fprintf ( fp, "ObjProt	%4d (%8d bytes)\n",
               top_obj_index,
               top_obj_index * ( sizeof ( *pObjIndex ) ) );

    /* objects */
     count = 0;
     count2 = 0;
     for ( obj = object_list; obj != NULL; obj = obj->next )
     {
          count++;
          for ( af = obj->affected; af != NULL; af = af->next )
               aff_count++;
     }
     for ( obj = obj_free; obj != NULL; obj = obj->next )
          count2++;

     fprintf ( fp, "Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
               count, count * ( sizeof ( *obj ) ), count2,
               count2 * ( sizeof ( *obj ) ) );

    /* affects */
     count = 0;
     for ( af = affect_free; af != NULL; af = af->next )
          count++;

     fprintf ( fp,
               "Affects	%4d (%8d bytes), %2d free (%d bytes)\n",
               aff_count, aff_count * ( sizeof ( *af ) ),
               count, count * ( sizeof ( *af ) ) );

    /* rooms */
     fprintf ( fp, "Rooms	%4d (%8d bytes)\n",
               top_room, top_room * ( sizeof ( *room ) ) );

    /* exits */
     fprintf ( fp, "Exits	%4d (%8d bytes)\n",
               top_exit, top_exit * ( sizeof ( *exit ) ) );

     fclose ( fp );

    /* start printing out mobile data */
     fp = fopen ( "mob.dmp", "w" );

     fprintf ( fp, "\nMobile Analysis\n" );
     fprintf ( fp, "---------------\n" );
     nMatch = 0;
     for ( vnum = 0; nMatch < top_mob_index; vnum++ )
          if ( ( pMobIndex = get_mob_index ( vnum ) ) != NULL )
          {
               nMatch++;
               fprintf ( fp,
                         "#%-4d %3d actv %3d kild level %3d     %s\n",
                         pMobIndex->vnum, pMobIndex->count,
                         pMobIndex->killed, pMobIndex->level,
                         pMobIndex->short_descr );
          }
     fclose ( fp );

    /* start printing out object data */
     fp = fopen ( "obj.dmp", "w" );

     fprintf ( fp, "\nObject Analysis\n" );
     fprintf ( fp, "---------------\n" );
     nMatch = 0;
     for ( vnum = 0; nMatch < top_obj_index; vnum++ )
          if ( ( pObjIndex = get_obj_index ( vnum ) ) != NULL )
          {
               nMatch++;
               fprintf ( fp,
                         "#%-4d %3d actv %3d rest level %3d      %s\n",
                         pObjIndex->vnum, pObjIndex->count,
                         pObjIndex->reset_num, pObjIndex->level,
                         pObjIndex->short_descr );
          }

    /* close file */
     fclose ( fp );
     fpReserve = fopen ( NULL_FILE, "r" );
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy ( int number )
{
     switch ( number_bits ( 2 ) )
     {
     case 0:
          number -= 1;
          break;
     case 3:
          number += 1;
          break;
     }

     return UMAX ( 1, number );
}

/* Why is some of this in db.c... is db.c the random dumping code place? */

/*
 * Generate a random number.
 */
int number_range ( int from, int to )
{
     int                 power;
     int                 number;

     if ( from == 0 && to == 0 )
          return 0;

     if ( ( to = to - from + 1 ) <= 1 )
          return from;

     for ( power = 2; power < to; power <<= 1 );

     while ( ( number = number_mm (  ) & ( power - 1 ) ) >= to );

     return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent ( void )
{
     int                 percent;

     while ( ( percent = number_mm (  ) & ( 128 - 1 ) ) > 99 );

     return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door ( void )
{
     int                 door;

     while ( ( door = number_mm (  ) & ( 8 - 1 ) ) > 5 )
          ;

     return door;
}

int number_bits ( int width )
{
     return number_mm (  ) & ( ( 1 << width ) - 1 );
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int          rgiState[2 + 55];

void init_mm (  )
{
     int                *piState;
     int                 iState;

     piState = &rgiState[2];

     piState[-2] = 55 - 55;
     piState[-1] = 55 - 24;

     piState[0] = ( ( int ) current_time ) & ( ( 1 << 30 ) - 1 );
     piState[1] = 1;
     for ( iState = 2; iState < 55; iState++ )
     {
          piState[iState] = ( piState[iState - 1] + piState[iState - 2] ) & ( ( 1 << 30 ) - 1 );
     }
     return;
}

int number_mm ( void )
{
     int                *piState;
     int                 iState1;
     int                 iState2;
     int                 iRand;

     piState = &rgiState[2];
     iState1 = piState[-2];
     iState2 = piState[-1];
     iRand = ( piState[iState1] + piState[iState2] )
          & ( ( 1 << 30 ) - 1 );
     piState[iState1] = iRand;
     if ( ++iState1 == 55 )
          iState1 = 0;
     if ( ++iState2 == 55 )
          iState2 = 0;
     piState[-2] = iState1;
     piState[-1] = iState2;
     return iRand >> 6;
}

/*
 * Roll some dice.
 */
int dice ( int number, int size )
{
     int                 idice;
     int                 sum;

     switch ( size )
     {
     case 0:
          return 0;
     case 1:
          return number;
     }

     for ( idice = 0, sum = 0; idice < number; idice++ )
          sum += number_range ( 1, size );

     return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate ( int level, int value_00, int value_32 )
{
     return value_00 + level * ( value_32 - value_00 ) / 32;
}

void smash_codes ( char *str )
{
     // Effectively blocks user attempts to access MXP, Pueblo commands via blocking <> brackets.
     // Blocking ~ is to prevent files from breaking.
     // Blocking % is to prevent breakage of various string functions (they should be safe, but should is never reliable)
     // !!SOUND and !!MUSIC are a bit harder to catch, but we do so below.
     for ( ; *str != '\0'; str++ )
     {
          if ( *str == '<' )
               *str = '(';
          if ( *str == '>' )
               *str = ')';
          if ( *str == '~' )
               *str = '-';
          if ( *str == '%' )
               *str = '_';
          else if ( *str == '!' ) // Looks for !!SO or !!MU on input lines, since MSP is insecure.
          {
               str++;
               if ( *str == '!' )
               {
                    str++;
                    if ( *str == 'S' || *str == 'M' )
                    {
                         str++;
                         if ( *str == 'O' || *str == 'U' )
                         {
                              // We can now safely assume someone is trying to mess with folks
                              // (ie, trying to use sound or music )
                              *str = '_';
                         }
                    }
               }
          }
     }
     return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different.
 *   (compatibility with historical functions).
 */
bool str_cmp ( const char *astr, const char *bstr )
{
     if ( astr == NULL || bstr == NULL )
     {
          bugf ( "Str_cmp: null string." );
          return TRUE;
     }
     else
     {
          register const unsigned char  *cstr = (const unsigned char *)astr,
               *dstr = (const unsigned char *)bstr;
          for ( ; *cstr || *dstr; cstr++, dstr++ )
          {
               if ( LOWER ( *cstr ) != LOWER ( *dstr ) )
                    return TRUE;
          }
     }
     return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix ( const char *astr, const char *bstr )
{

     if ( astr == NULL || bstr == NULL )
     {
          bugf ( "Strn_cmp: null string." );
          return TRUE;
     }
     else
     {
          register const unsigned char  *cstr = (const unsigned char *)astr,
               *dstr = (const unsigned char *)bstr;
          for ( ; *cstr; cstr++, dstr++ )
          {
               if ( LOWER ( *cstr ) != LOWER ( *dstr ) )
                    return TRUE;
          }
     }

     return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix ( const char *astr, const char *bstr )
{
     int                 sstr1;
     int                 sstr2;
     int                 ichar;
     char                c0;

     if ( ( c0 = LOWER ( astr[0] ) ) == '\0' )
          return FALSE;

     sstr1 = strlen ( astr );
     sstr2 = strlen ( bstr );

     for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
     {
          if ( c0 == LOWER ( bstr[ichar] ) &&
               !str_prefix ( astr, bstr + ichar ) )
               return FALSE;
     }

     return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix ( const char *astr, const char *bstr )
{
     int                 sstr1;
     int                 sstr2;

     sstr1 = strlen ( astr );
     sstr2 = strlen ( bstr );
     if ( sstr1 <= sstr2 &&
          !str_cmp ( astr, bstr + sstr2 - sstr1 ) )
          return FALSE;
     else
          return TRUE;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize ( const char *str )
{
     static char         strcap[MAX_STRING_LENGTH];
     int                 i;

     for ( i = 0; str[i] != '\0'; i++ )
          strcap[i] = LOWER ( str[i] );
     strcap[i] = '\0';
     strcap[0] = UPPER ( strcap[0] );
     return strcap;
}

/*
 * Append a string to a file.
 */
void append_file ( CHAR_DATA * ch, char *file, char *str )
{
     FILE               *fp;

     if ( IS_NPC ( ch ) || str[0] == '\0' )
          return;

     fclose ( fpReserve );
     if ( ( fp = fopen ( file, "a" ) ) == NULL )
     {
          perror ( file );
          send_to_char ( "Could not open the file!\n\r", ch );
     }
     else
     {
          fprintf ( fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
          fclose ( fp );
     }

     fpReserve = fopen ( NULL_FILE, "r" );
     return;
}

/*
 * Reports a formatted bug.
 * This version by Lotherius (modelled on others)
 */

void bugf ( const char *fmt, ... )
{
     char    buf[MSL*4];
     char   *strtime;
     va_list args;

     strtime = ctime ( &current_time );
     strtime[strlen ( strtime ) - 1] = '\0';
     fprintf ( stderr, "%s :: ", strtime );

     if ( fpArea != NULL && strArea[0] != '\0' )
     {
          int                 iLine;
          int                 iChar;

          if ( fpArea == stdin )
          {
               iLine = 0;
          }
          else
          {
               iChar = ftell ( fpArea );
               fseek ( fpArea, 0, 0 );
               for ( iLine = 0; ftell ( fpArea ) < iChar; iLine++ )
               {
                    while ( getc ( fpArea ) != '\n' );
                    /* Nothing */
               }
               fseek ( fpArea, iChar, 0 );
          }
          fprintf ( stderr, "[*****] FILE: %s LINE: %d", strArea, iLine );
     }
     else
          fprintf ( stderr, "[*****] BUG: " );

     va_start ( args, fmt );
     vsnprintf ( buf, sizeof ( buf ) -1, fmt, args );
     va_end ( args );

     fprintf ( stderr, "%s\n", buf );
     return;
}

/*
 * Writes a string to the log.
 * Formatted log_string by Lotherius
 */
void log_string ( const char *str, ... )
{
     char  buf[MSL*4];
     char *strtime;
     va_list args;

     va_start ( args, str );
     vsnprintf ( buf, sizeof ( buf ) -1, str, args );
     va_end ( args );

     strtime = ctime ( &current_time );
     strtime[strlen ( strtime ) - 1] = '\0';
     fprintf ( stderr, "%s :: %s\n", strtime, buf );
     return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */

void tail_chain ( void )
{
     return;
}

void update_obj_orig ( OBJ_DATA * obj )
{
     int                 count;

     if ( obj == NULL )
     {
          bugf ( "Null object passed to update_obj_orig." );
          return;
     }
     for ( count = 0; count <= 4; count++ )
     {
          obj->valueorig[count] = obj->value[count];
     }
     return;
}

void fwrite_crier ( void )
{
     FILE               *fptr;

     struct crier_type  *tmp;

     if ( ( fptr = fopen ( CRIER_FILE, "w+" ) ) == NULL )
     {
          bugf ( "Failed Opening Crier File for writing." );
          return;
     }

     for ( tmp = crier_list; tmp != NULL; tmp = tmp->next )
     {
          fprintf ( fptr, "%s~\n", tmp->text );
     }

     fprintf ( fptr, "@~\n" );
     fclose ( fptr );
     return;
}

void fwrite_accounts ( void )
{
     FILE        *fptr;
     char	buf[MAX_STRING_LENGTH];
     int		i;
     struct account_type  *tmp;

     buf[0] = '\0';

#if !defined(WIN32)
     SNP ( buf, "cp -f %s%s %s%s.bak > /dev/null", DATA_DIR, ACCOUNT_FILE, DATA_DIR, ACCOUNT_FILE );

     if ( system ( buf ) == 1 )
     {
          bugf ( "Failed to BACKUP ACCOUNTS File. Aborting Write." );
     }
#endif

     buf[0] = '\0';

     SNP ( buf, "%s%s", DATA_DIR, ACCOUNT_FILE );

     if ( ( fptr = fopen ( buf, "w+" ) ) == NULL )
     {
          bugf ( "Failed Opening ACCOUNTS File for writing." );
          return;
     }

     for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
     {
          if (tmp->status == ACCT_REJECTED_DELETE)
               continue;   /* Acct Deleted */

          fprintf ( fptr, "%s~\n", tmp->acc_name );
          fprintf ( fptr, "%s~\n", tmp->password );
          fprintf ( fptr, "%d %d %d %d\n", tmp->status,
                    tmp->permadead, tmp->heroes, tmp->demigods );

          for (i = 0 ; i < MAX_CHARS ; i++)
          {
               if (tmp->char_name[i])
               {
                    fprintf (fptr, "%s~\n", tmp->char_name[i] );
               }
          }
          fprintf (fptr, "NEXT~\n" );
     }

     fprintf ( fptr, "END~\n" );
     fclose ( fptr );
     return;
}

void fwrite_disable ( void )
{
     FILE               *fptr;
     struct disable_cmd_type *tmp;

     if ( ( fptr = fopen ( DISABLE_FILE, "w+" ) ) == NULL )
     {
          bugf ( "Failed openning disable.txt for writing." );
          return;
     }

     for ( tmp = disable_cmd_list; tmp != NULL; tmp = tmp->next )
     {
          fprintf ( fptr, "%s~\n", tmp->name );
          fprintf ( fptr, "%d\n", tmp->level );
     }

     fprintf ( fptr, "@~\n" );
     fclose ( fptr );
     return;
}

void load_crier ( void )
{
     FILE               *fptr;
     struct crier_type  *tmp = NULL;
     struct crier_type  *previous = NULL;
     bool                done = FALSE;
     char               *word;

/* Make sure we start with a NULL list */
     crier_list = NULL;

     if ( ( fptr = fopen ( CRIER_FILE, "r" ) ) == NULL )
     {
          bugf ( "Failed to open CRIER_FILE for reading." );
          return;
     }

     while ( !done )
     {
          word = fread_string ( fptr );
          if ( !str_cmp ( word, "@" ) )
          {
               done = TRUE;
          }
          else
          {
               tmp = alloc_mem ( sizeof ( struct crier_type ), "crier_type" );
               tmp->next = NULL;
               tmp->text = str_dup ( word );
               if ( crier_list == NULL )
                    crier_list = tmp;
               else
                    previous->next = tmp;
               previous = tmp;
          }
          free_string ( word );
     }
     fclose ( fptr );
     return;
}

void load_accounts ( void )
{
     FILE               *fptr;
     struct account_type  *tmp = NULL;
     struct account_type  *previous = NULL;
     bool                done = FALSE;
     bool		listdone = FALSE;
     int			i = 0;
     char                *word;
     char		*name;
     char		buf[MAX_STRING_LENGTH];

/* Make sure we start with a NULL list */
     account_list = NULL;

     SNP ( buf, "%s%s", DATA_DIR, ACCOUNT_FILE );

     if ( ( fptr = fopen ( buf, "r" ) ) == NULL )
     {
          bugf ( "Failed to open ACCOUNTS_FILE for reading." );
          return;
     }

     while ( !done )
     {
          listdone = FALSE;
          word = fread_string ( fptr );
          if ( !str_cmp ( word, "END" ) )
          {
               done = TRUE;
          }
          else
          {
               tmp = alloc_perm ( sizeof ( struct account_type ), "account_type:load" );
               tmp->next = NULL;
               tmp->acc_name = str_dup ( word ); /* Email Address of Account */
               if ( account_list == NULL )
                    account_list = tmp;
               else
                    previous->next = tmp;
               previous = tmp;

	    /* Password  -- Need to crypt this!, just haven't gotten around tuit yet. */
               tmp->password = fread_string ( fptr );

               tmp->status = fread_number ( fptr );
               tmp->permadead = fread_number ( fptr );
               tmp->heroes = fread_number ( fptr );
               tmp->demigods = fread_number ( fptr );
               tmp->vcode = 0;
               fread_to_eol ( fptr );

               i = 0;

               while (!listdone)
               {
                    i++;
                    name = fread_string ( fptr );
                    if ( !str_cmp ( name, "NEXT" ) )
                    {
                         listdone = TRUE;
                    }
                    else
                    {
                         tmp->char_name[i] = str_dup ( name );
                    }
                    free_string ( name );
               }

          }
          free_string ( word );
     }
     fclose ( fptr );
     return;
}

void load_disable ( void )
{
     FILE               *fptr;
     struct disable_cmd_type *tmp = NULL;
     struct disable_cmd_type *previous = NULL;
     bool                done = FALSE;
     bool                found = FALSE;
     char               *word;
     int                 cmd;

     /* make sure list is null to start with */
     disable_cmd_list = NULL;

     if ( ( fptr = fopen ( DISABLE_FILE, "r" ) ) == NULL )
     {
          bugf ( "failed openning disable.txt for reading." );
          return;
     }

     while ( !done )
     {
          word = fread_string ( fptr );
          if ( !str_cmp ( word, "@" ) )
          {
               done = TRUE;
          }
          else
          {
               for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
               {
                    if ( word[0] == cmd_table[cmd].name[0]
                         && !str_prefix ( word, cmd_table[cmd].name ) )
                    {
                         found = TRUE;
                         break;
                    }
               }
               if ( !found )
               {
                    bugf ( "Bad disable command name in disable.txt: %s", word );
                    fread_number ( fptr );
                    continue;
               }
               tmp = alloc_mem ( sizeof ( struct disable_cmd_type ), "disable_cmd_type" );

               tmp->next = NULL;
               tmp->name = str_dup ( word );
               tmp->level = fread_number ( fptr );
               tmp->level = UMIN ( 110, tmp->level );
               tmp->disable_fcn = cmd_table[cmd].do_fun;

               if ( disable_cmd_list == NULL )
                    disable_cmd_list = tmp;
               else
                    previous->next = tmp;

               previous = tmp;
          }
          free_string ( word );
     }
     fclose ( fptr );
     return;
}

/* Online setting of skill/spell levels,
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 *
 */

/*
  Class table levels loading/saving
  */

/* Save this class */
void save_class ( int num )
{
     FILE               *fp = NULL;
     char                buf[MAX_STRING_LENGTH];
     int                 lev, i;

     SNP ( buf, "%s%s", CLASS_DIR, class_table[num].name );

     if ( !( fp = fopen ( buf, "w" ) ) )
     {
          bugf ( "Bug! Could Not Open Class File to save!" );
          return;
     }

     for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
          for ( i = 0; i < MAX_SKILL; i++ )
          {
               if ( !skill_table[i].name || !skill_table[i].name[0] )
                    continue;

               if ( skill_table[i].skill_level[num] == lev )
                    fprintf ( fp, "Sk %d '%s'\n", lev,
                              skill_table[i].name );
          }
    /* the EOF marker !MUST! be followed by a CR or it will
     * result in a "string too long" error on bootup. */

     fprintf ( fp, "$\n" );	/* EOF $ */
     fclose ( fp );
}

// Currently this doesn't allocate any memory for new skills.
// Therefore all skills must have a prototype in the normal places.
// In the future, it will need to recognize skills without a prototype
// and generate a new memory location for them.
// Any errors here will be FATAL. This is to prevent ever accidentally overwriting a good Skills.DAT
// with incorrectly read information.
// When changing the format of a skill, be sure to load the game and save the skills with the new format
// before changing the load routines.
//
void load_skills (  )
{
     FILE               *fptr;
     bool                done = FALSE;
     char               *word;
     char                buf[MAX_STRING_LENGTH];
     int                 sn;

     SNP ( buf, "%sSkills.DAT", DATA_DIR );

     if ( ( fptr = fopen ( buf, "r" ) ) == NULL )
     {
          bugf ( "FATAL: Failed to open Skills.DAT for reading." );
          exit ( 1 );
     }

     while (!done)
     {
          word = fread_string( fptr );
          if ( !str_cmp ( word, "CODESKILL" ) ) // A codeskill has a prototype in code.
          {
               word = fread_string ( fptr );

               if ( ( sn = skill_lookup ( word ) ) == -1 )
               {
                    log_string ( "FATAL: #CODESKILL %s Invalid (Skills.DAT).\n\r", word);
                    exit ( 1 );
               }

               skill_table[sn].noun_damage = fread_string ( fptr );
               skill_table[sn].msg_off = fread_string ( fptr );
               skill_table[sn].component = fread_string ( fptr );
               skill_table[sn].target = fread_number ( fptr );
               skill_table[sn].minimum_position = fread_number ( fptr );
               skill_table[sn].min_mana = fread_number ( fptr );
               skill_table[sn].beats = fread_number ( fptr );
               fread_to_eol ( fptr ); // to make sure the number line is ended.

          }
          else // Done? Hope so, cuz we didn't get #SKILL.
          {
               log_string ("Finished snarfing Skills.DAT");
               done = TRUE;
          }
     }

     fclose ( fptr );
     return;
}

// Class level information is saved seperately to allow for addition of classes
// without breaking the skillfile format. I could perhaps include it in the same
// file, but it would require a much more complicated routine here and there is
// already at this point another routine to do it.
//
void save_skills (  )
{
     FILE               *fp = NULL;
     char                buf[MAX_STRING_LENGTH];

     int                 i;

     SNP ( buf, "%sSkills.DAT", DATA_DIR );

     if ( !( fp = fopen ( buf, "w" ) ) )
     {
          bugf ( "Bug! Could Not Open Skills File to save!" );
          return;
     }

     for ( i = 0; i < MAX_SKILL; i++ )
     {
          if ( !skill_table[i].name || !skill_table[i].name[0] )
               continue;

          fprintf ( fp, "CODESKILL~\n" );	// Codeskills are the ones that have a prototype in code.
          fprintf ( fp, "%s~\n", skill_table[i].name );
          fprintf ( fp, "%s~\n", skill_table[i].noun_damage );
          fprintf ( fp, "%s~\n", skill_table[i].msg_off );
          fprintf ( fp, "%s~\n", skill_table[i].component );
          fprintf ( fp, "%d %d %d %d\n",
                    skill_table[i].target,
                    skill_table[i].minimum_position,
                    skill_table[i].min_mana,
                    skill_table[i].beats );
     }

    /* the EOF marker !MUST! be followed by a CR or it will
     * result in a "string too long" error on bootup. */

     fprintf ( fp, "END~\n" );	/* EOF $ */
     fclose ( fp );
}

void save_classes (  )
{
     int                 i;

     for ( i = 0; i < MAX_CLASS; i++ )
          save_class ( i );
}

/* New Load A Class - By Lotherius */
void load_class ( int num )
{
     char                buf3[MAX_STRING_LENGTH];
     FILE               *fp = NULL;
     char               *word;
     bool                fMatch;
     bool                done = FALSE;

     SNP ( buf3, "%s%s", CLASS_DIR, class_table[num].name );

     if ( !( fp = fopen ( buf3, "r" ) ) )
     {
          bugf ( "Bug! Could Not Open %s!", buf3 );
          return;
     }

     fMatch = FALSE;

     while ( !done )
     {
          word = fread_word ( fp );

          if ( !str_cmp ( word, "Sk" ) )
          {
               int                 sn;
               int                 value;
               char               *temp;

               value = fread_number ( fp );
               temp = fread_word ( fp );
               sn = skill_lookup ( temp );
               if ( sn < 0 )
               {
                    bugf ( "load_class: unknown skill %s ", temp );
               }
               else
               {
                    skill_table[sn].skill_level[num] = value;
                    fMatch = TRUE;
               }

          }
          /* end of matching sk */
          if ( !str_cmp ( word, "$" ) )
          {
               done = TRUE;
          }
     }
     /* End of While */

     if ( !fMatch )
     {
          bugf ( "load_class: no skills found." );
          fread_to_eol ( fp );
     }

     fclose ( fp );
}

void load_classes (  )
{
     int                 i, j;

     for ( i = 0; i < MAX_CLASS; i++ )
     {
          for ( j = 0; j < MAX_SKILL; j++ )
               skill_table[j].skill_level[i] = LEVEL_IMMORTAL;

          load_class ( i );
     }
     log_string ( "Finished Loading Classes" );
}

PROG_CODE *get_prog_index( int vnum, int type )
{
     PROG_CODE *prg;

     switch ( type )
     {
     case PRG_MPROG:
          prg = mprog_list;
          break;
     case PRG_OPROG:
          prg = oprog_list;
          break;
     case PRG_RPROG:
          prg = rprog_list;
          break;
     default:
          return NULL;
     }

     for( ; prg; prg = prg->next )
     {
          if ( prg->vnum == vnum )
               return( prg );
     }
     return NULL;
}

void fix_objprogs( void )
{
     OBJ_INDEX_DATA *pObjIndex;
     PROG_LIST        *list;
     PROG_CODE        *prog;
     int iHash;

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pObjIndex   = obj_index_hash[iHash];
                pObjIndex   != NULL;
                pObjIndex   = pObjIndex->next )
          {
               for( list = pObjIndex->oprogs; list != NULL; list = list->next )
               {
                    if ( ( prog = get_prog_index( list->vnum, PRG_OPROG ) ) != NULL )
                         list->code = prog->code;
                    else
                    {
                         bugf ( "Fix_objprogs: code vnum %d not found.", list->vnum );
                         exit( 1 );
                    }
               }
          }
     }
     return;
}

void fix_roomprogs( void )
{
     ROOM_INDEX_DATA *pRoomIndex;
     PROG_LIST        *list;
     PROG_CODE        *prog;
     int iHash;

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex   = room_index_hash[iHash];
                pRoomIndex   != NULL;
                pRoomIndex   = pRoomIndex->next )
          {
               for( list = pRoomIndex->rprogs; list != NULL; list = list->next )
               {
                    if ( ( prog = get_prog_index( list->vnum, PRG_RPROG ) ) != NULL )
                         list->code = prog->code;
                    else
                    {
                         bugf ( "Fix_roomprogs: code vnum %d not found.", list->vnum );
                         exit( 1 );
                    }
               }
          }
     }
     return;
}

/*
 * init_racial_affects
 * Date: 4/19/98
 * Author: Zeran
 * Moved into db.c since don't need a file all by itself for 1 func.
*/

void init_racial_affects ( void )
{
     int                 race;

    /* Now set flags for all appropriate races */

    /* Satyr */
     race = race_lookup ( "satyr" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );
     SET_BIT ( race_table[race].detect, DET_INVIS );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );

    /* Gargoyle */
     race = race_lookup ( "gargoyle" );
     SET_BIT ( race_table[race].aff, AFF_FLYING );
     SET_BIT ( race_table[race].detect, DET_EVIL );

    /* Elf */
     race = race_lookup ( "elf" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Dwarf */
     race = race_lookup ( "dwarf" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Bat */
     race = race_lookup ( "bat" );
     SET_BIT ( race_table[race].aff, AFF_FLYING );
     SET_BIT( race_table[race].detect, DET_DARK_VISION );

    /* Cat */
     race = race_lookup ( "cat" );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );

    /* Centipede */
     race = race_lookup ( "centipede" );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );

    /* Fox */
     race = race_lookup ( "fox" );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );

    /* Goblin */
     race = race_lookup ( "goblin" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Hobgoblin */
     race = race_lookup ( "hobgoblin" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Kobold */
     race = race_lookup ( "kobold" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Modron */
     race = race_lookup ( "modron" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Orc */
     race = race_lookup ( "orc" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Song bird */
     race = race_lookup ( "song bird" );
     SET_BIT ( race_table[race].aff, AFF_FLYING );

    /* Troll */
     race = race_lookup ( "troll" );
     SET_BIT ( race_table[race].aff, AFF_REGENERATION );
     SET_BIT ( race_table[race].detect, DET_INFRARED );
     SET_BIT ( race_table[race].detect, DET_HIDDEN );

    /* Water fowl */
     race = race_lookup ( "water fowl" );
     SET_BIT ( race_table[race].aff, AFF_SWIM );
     SET_BIT ( race_table[race].aff, AFF_FLYING );

    /* Wolf */
     race = race_lookup ( "wolf" );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );

    /* Wyvern */
     race = race_lookup ( "wyvern" );
     SET_BIT ( race_table[race].detect, DET_INVIS );
     SET_BIT ( race_table[race].aff, AFF_FLYING );
     SET_BIT ( race_table[race].detect, DET_HIDDEN );

    /* Undead */
     race = race_lookup ( "undead" );
     SET_BIT ( race_table[race].aff, AFF_CURSE );
     SET_BIT ( race_table[race].protect, PROT_GOOD );

    /* Demon */
     race = race_lookup ( "demon" );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );
     SET_BIT ( race_table[race].protect, PROT_GOOD );
     SET_BIT ( race_table[race].aff, AFF_REGENERATION );

    /* Avatar */
     race = race_lookup ( "avatar" );
     SET_BIT ( race_table[race].protect, PROT_GOOD );
     SET_BIT ( race_table[race].protect, PROT_SANCTUARY );

    /* Kender */
     race = race_lookup ( "kender" );
     SET_BIT ( race_table[race].protect, PROT_EVIL );

    /* Gnome */
     race = race_lookup ( "gnome" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

    /* Vampire */
     race = race_lookup ( "vampire" );
     SET_BIT ( race_table[race].aff, AFF_REGENERATION );
     SET_BIT ( race_table[race].detect, DET_HIDDEN );
     SET_BIT ( race_table[race].detect, DET_DARK_VISION );
     SET_BIT ( race_table[race].aff, AFF_HASTE );

    /* Minotaur */
     race = race_lookup ( "minotaur" );
     SET_BIT ( race_table[race].detect, DET_INFRARED );

}

