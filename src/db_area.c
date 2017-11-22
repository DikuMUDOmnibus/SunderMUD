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
 * The save functions in this file are influenced and even used from
 * the original ILAB OLC used throughout SunderMud, and thus wouldn't
 * be possible without the contributions from code distributed
 * freely with The Isles 1.1 source code.
 *
 * The loading functions are derived from the Rom 2.3 functions and have
 * been highly modified to remove "vestigal tails" and add additional
 * functionality.
 *
 * The overal purpose of this file is to group all the saving/loading
 * functions so that changes may more easily be made to the mud's areafile
 * formats when needed and most updates will be contained to this one file.
 */

#include "everything.h"
#include "olc.h"
#include "db.h"
#include "convert.h"

/* Local Prototypes */
void 		do_asave 		args ( ( CHAR_DATA *ch, char *argument 	  ) );
void 		db_save_area 		args ( ( AREA_DATA *pArea 		  ) );
void 		db_save_area_header 	args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_rooms 	args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_objects    args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_mobiles    args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_resets     args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_shops 	args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_specials 	args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_save_area_progs	args ( ( char *filename, AREA_DATA *pArea ) );
AREA_DATA      *db_load_area_header 	args ( ( char *filename 		  ) );
void            db_load_area_rooms      args ( ( char *filename, AREA_DATA *pArea ) );
void 		db_load_area_mobiles 	args ( ( char *filename, AREA_DATA *pArea ) );
void            db_load_area_objects    args ( ( char *filename, AREA_DATA *pArea ) );
void            db_load_area_resets     args ( ( char *filename, AREA_DATA *pArea ) );
void            db_load_area_shops      args ( ( char *filename, AREA_DATA *pArea ) );
void            db_load_area_specials   args ( ( char *filename, AREA_DATA *pArea ) );
void            db_load_area_progs	args ( ( char *filename, AREA_DATA *pArea ) );

/* External functions needed */
char *fwrite_flag 		args ( ( long flags, char buf[] ) );

extern int flag_lookup 		args ( (const char *name, const struct flag_type *flag_table) );

/* Local Variables */
sh_int 		versions[9];			/* Temporarily stores version info from header while loading */

/*
 * Defines for version confident files.
 * The first value for each type is the position in the array,
 * and the second is the actual version. Don't change the array
 * position defines unless you what you're messing up!
 */

#define DB_HEADER			0
#define DB_HEADER_VERSION 		20
#define DB_ROOMS			1
#define DB_ROOMS_VERSION  		20
#define DB_OBJS				2
#define DB_OBJS_VERSION			20
#define DB_MOBS				3
#define DB_MOBS_VERSION			20
#define DB_RESETS			4
#define DB_RESETS_VERSION		20
#define DB_PROGS			5
#define DB_PROGS_VERSION		20
#define DB_SHOPS			6
#define DB_SHOPS_VERSION		20
#define DB_SPECIALS			7
#define DB_SPECIALS_VERSION		20
#define DB_LEASE			8
#define DB_LEASE_VERSION		20

/* SUNDERMUD VERSIONS:
 * SunderMud 2.0:  release version number is: 20
 *
 * All compatibility versions require pre-split files. See "area-converting.doc"
 * in the doc directory for information and more specific notes for each format.
 * SunderMud 1.0 is not supported since few muds use this format. Additionally,
 * old-style mobprogs cannot be imported automatically.
 *
 * Obviously, importing foreign areas subjects you to the license of the mud or
 * area archive from which you have taken the area.
 *
 * (*) Denotes version # not used but reserved for future use.
 * (-) Denotes incomplete, partial or broken support
 * (+) Denotes good support but missing a few details
 * (Y) Denotes complete support
 * 
 *
 * COMPATIBILITY VERSIONS:
 *          Name:   Version:  (?) (Room/Obj/Mob/Reset/Shop/Spec/Prog)
 *       DikuMud:   Version 0 (*)
 *      Merc 2.2:   Version 1 (*)
 * 	 Rom 2.3:   Version 2 (-) (RO-----)
 * 	 Rom 2.4:   Version 3 (*)
 * 	 Rot 1.4:   Version 5 (*)
 *	Envy 2.2:   Version 6 (*)
 *  Ember 0.9.44:   Version 7 (*)
 *     Rogue 2.1:   Version 8 (*)
 * Circle 3bpl20:   Version 9 (*)
 *    Smaug 1.4a:   Version 10(*)
 * SunderMud 2.0:   Version 20(Y) (ROMRSSP)
 */

#define SKEY( string, field )                       	\
          if ( !str_cmp( word, string ) )     		\
                    {                                   \
                         free_string( field );          \
                         field = fread_string( fpArea );\
                         fMatch = TRUE;                 \
                         break;                         \
                    }

/*---------------------------------------------------------
 * Name:       do_asave
 * Purpose:    Entry point for saving new Sunder Area data.
 *-------------------------------------------------------*/
void do_asave ( CHAR_DATA *ch, char *argument )
{
     char		 arg1[MIL];
     int		 value;
     AREA_DATA          *pArea;

     /* Put autosave here */
     
     if ( IS_NPC ( ch ) )
          return;

     SLCPY ( arg1, argument );
     
     if ( arg1[0] == '\0' )
     {
          send_to_char ( "Syntax:\n\r", ch );
          send_to_char ( "  asave list     - saves the area.lst file\n\r", ch );
          send_to_char ( "  asave changed  - saves all changed zones\n\r", ch );
          send_to_char ( "  asave <vnum>   - saves a particular area\n\r", ch );
          send_to_char ( "  asave area     - saves the current area\n\r", ch );
          send_to_char ( "  asave world    - saves the world! (db dump)\n\r", ch );
          send_to_char ( "\n\r", ch );
          return;
     }
     
     /* Save area of given numeric vnum. */
     /* -------------------------------- */     
     if ( is_number ( arg1 ) )
     {
          value = atoi ( arg1 );
          
          if ( !( pArea = get_area_data ( value ) ) )
          {
               send_to_char ( "That area does not exist.\n\r", ch );
               return;
          }
          
          if ( is_number ( arg1 ) )
          {
               if ( !IS_BUILDER ( ch, pArea ) )
               {
                    send_to_char ( "You are not a builder for this area.\n\r", ch );
                    return;
               }
               save_area_list (  );
               db_save_area ( pArea );
               return;
          }
     }
     
     /* Save the world, only authorized areas. */
     /* -------------------------------------- */     
     if ( !str_cmp ( "world", arg1 ) )
     {
          save_area_list (  );
          for ( pArea = area_first; pArea; pArea = pArea->next )
          {
               /* Builder must be assigned this area. */
               if ( !IS_BUILDER ( ch, pArea ) )
                    continue;
               db_save_area ( pArea );
               REMOVE_BIT ( pArea->area_flags, AREA_CHANGED );
          }
          send_to_char ( "You saved the world.\n\r", ch );
          return;
     }
     
     /* Save changed areas, only authorized areas. */
     /* ------------------------------------------ */     
     if ( !str_cmp ( "changed", arg1 ) )
     {
          bool amatch = FALSE;
          save_area_list (  );
          
          send_to_char ( "Saved zones:\n\r", ch );
          
          for ( pArea = area_first; pArea; pArea = pArea->next )
          {
               /* Builder must be assigned this area. */
               if ( !IS_BUILDER ( ch, pArea ) )
                    continue;               
               /* Save changed areas. */
               if ( IS_SET ( pArea->area_flags, AREA_CHANGED ) )
               {
                    amatch = TRUE; // Got at least one.
                    db_save_area ( pArea );
                    form_to_char ( ch, "%24s - '%s'\n\r", pArea->name, pArea->filename );
                    REMOVE_BIT ( pArea->area_flags, AREA_CHANGED );
               }
          }
          if ( !amatch )
               send_to_char ( "None.\n\r", ch );
          return;
     }
     
     /* Save the area.lst file. */
     /* ----------------------- */
     if ( !str_cmp ( arg1, "list" ) )
     {
          save_area_list (  );
          return;
     }
     

     /* Save current area. Either the one being edited, or the one the ch is in. */
     if ( !str_cmp ( "area", arg1 ) )
     {
          switch ( ch->desc->editor )
          {
          case ED_AREA:    pArea = ( AREA_DATA * ) ch->desc->pEdit;
               break;
          case ED_ROOM:    pArea = ch->in_room->area;
               break;
          case ED_OBJECT:  pArea = ( ( OBJ_INDEX_DATA * ) ch->desc->pEdit )->area;
               break;
          case ED_MOBILE:  pArea = ( ( MOB_INDEX_DATA * ) ch->desc->pEdit )->area;
               break;
          default:         pArea = ch->in_room->area;
               break;
          }
          if ( !IS_BUILDER ( ch, pArea ) )
          {
               send_to_char ( "You are not a builder for this area.\n\r", ch );
               return;
          }
          save_area_list (  );          
          db_save_area ( pArea );
          //REMOVE_BIT ( pArea->area_flags, AREA_CHANGED );
          send_to_char ( "Area saved.\n\r", ch );
          return;
     }

     /* Show correct syntax. */
     do_asave ( ch, "" );
     return;
}

/*
 * db_save_area - Calls all the area saver functions for given pArea
 */

void db_save_area ( AREA_DATA *pArea )
{
     fclose ( fpReserve );

     db_save_area_header     (pArea->filename, pArea );
     db_save_area_rooms      (pArea->filename, pArea );
     db_save_area_objects    (pArea->filename, pArea );
     db_save_area_mobiles    (pArea->filename, pArea );
     db_save_area_resets     (pArea->filename, pArea );
     db_save_area_shops      (pArea->filename, pArea );
     db_save_area_specials   (pArea->filename, pArea );
     db_save_area_progs      (pArea->filename, pArea ); // Mob, Obj & Room progs.
     // db_save_area_leases     (pArea->filename, pArea ); // Leases
     //
     fpReserve = fopen ( NULL_FILE, "r" );
     return;
}

/* Loads a specific area into memory */
void db_load_area ( char *filename )
{
     AREA_DATA 		*pArea;

     fImportDb = FALSE;	/* Just make sure, since here we want fbootdb, not import */
     
     if ( ( pArea = ( db_load_area_header ( filename ) ) ) == NULL )
     {
          bugf ("No Header Found for %s!!", filename );
          return;
     }
     db_load_area_rooms		( pArea->filename, pArea );
     db_load_area_objects 	( pArea->filename, pArea );
     db_load_area_mobiles	( pArea->filename, pArea );
     db_load_area_resets	( pArea->filename, pArea );
     db_load_area_shops		( pArea->filename, pArea );
     db_load_area_specials	( pArea->filename, pArea );
     db_load_area_progs		( pArea->filename, pArea );
     //     db_load_leases		( pArea->filename, pArea );
     
     strcpy ( strArea, "" );
     return;
}

/* Manually loads an area 
 * This function does a lot of logging since importing areas is unreliable at best.
 */

void db_import_area ( CHAR_DATA *ch, char *argument )
{
     AREA_DATA		*pArea;

     fImportDb = TRUE;
     
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax: aimport <filename>", ch );
          return;
     }

     form_to_char ( ch, "Attempting to import area \"%s\".\n\r", argument );
     log_string ( "%s Imports area %s", ch->name, argument );          
     
     if ( ( pArea = ( db_load_area_header ( argument ) ) ) == NULL )
     {
          bugf ("No Header Found for %s!!", argument );
          fImportDb = FALSE;
          return;
     }
     log_string ( "Headers done." );
     db_load_area_rooms         ( pArea->filename, pArea );
     log_string ( "Rooms done." );
     db_load_area_objects       ( pArea->filename, pArea );
     log_string ( "Objects done." );
     db_load_area_mobiles       ( pArea->filename, pArea );
     log_string ( "Mobiles done." );
     db_load_area_resets        ( pArea->filename, pArea );
     log_string ( "Resets done." );
     db_load_area_shops         ( pArea->filename, pArea );
     log_string ( "Shops done." );
     db_load_area_specials      ( pArea->filename, pArea );
     log_string ( "Specials done." );
     db_load_area_progs         ( pArea->filename, pArea );
     log_string ( "Progs done." );
     //     db_load_leases              ( pArea->filename, pArea );
     
     /* Call the fixup routines again... just doing them globally since importation of
      * areas isn't a common event. */
     
     fImportDb = FALSE;
     
     strcpy ( strArea, "" );

     log_string ( "Fixing Exits" );
     fix_exits (  );
     log_string ( "Fixing Progs" );
     fix_mobprogs( );
     fix_roomprogs( );
     fix_objprogs( );
     
     log_string ( "Setting Racial Effects." );
     init_racial_affects (  );
     log_string ( "Updating Areas." );
     area_update ( );

     log_string ( "Import of %s complete.", argument );      
     send_to_char ( "Done.\n\r", ch );
     return;
     
}

/*
 * Saves an area HEADER file.
 */

void db_save_area_header ( char *filename, AREA_DATA *pArea )
{
     char hfilename[MSL];
     FILE *fp;

     SNP ( hfilename, "%s.header", filename );

     if ( !( fp = fopen ( hfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( hfilename );
     }

     /*
      * Header version MUST come first here since it can afect the construction of the rest
      * of the header, INCLUDING possibly how many version numbers there are.
      */
     fprintf ( fp, "Versions    %d %d %d %d %d %d %d %d %d\n",
               DB_HEADER_VERSION,
               DB_ROOMS_VERSION,
               DB_OBJS_VERSION,
               DB_MOBS_VERSION,
               DB_RESETS_VERSION,
               DB_PROGS_VERSION,
               DB_SHOPS_VERSION,
               DB_SPECIALS_VERSION,
               DB_LEASE_VERSION );
     fprintf ( fp, "Name        %s~\n", 	pArea->name );
     fprintf ( fp, "Zone        %d\n", 		pArea->zone );
     fprintf ( fp, "Builders    %s~\n", 	fix_string ( pArea->builders ) );
     fprintf ( fp, "Music       %s~\n", 	pArea->soundfile );
     fprintf ( fp, "VNUMs       %d %d\n", 	pArea->lvnum, pArea->uvnum );
     fprintf ( fp, "Levels      %d %d\n", 	pArea->llev, pArea->hlev );
     fprintf ( fp, "Credits     %s~\n", 	pArea->credits );
     fprintf ( fp, "Security    %d\n", 		pArea->security );
     fprintf ( fp, "End\n\n\n\n" );

     fclose ( fp );
     return;
}

/* Loads an area HEADER file */
AREA_DATA *db_load_area_header ( char *filename )
{
     bool fMatch;
     char *word;
     AREA_DATA *pArea;

     SNP ( strArea, "%s.header", filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "Open_area: fopen (while loading an area)");
          perror ( strArea );
          return NULL;
     }

     pArea = alloc_perm ( sizeof ( *pArea ), "pArea:db_load_area_header" );

     pArea->age = 15;
     pArea->nplayer = 0;
     pArea->filename = str_dup ( filename );
     pArea->vnum = top_area;
     pArea->name = str_dup ( "New Area" );
     pArea->builders = str_dup ( "" );
     pArea->credits = str_dup ( "" );
     pArea->soundfile = str_dup ( "" );
     pArea->soundfile = str_dup ( "" );
     pArea->security = 9;
     pArea->lvnum = 0;
     pArea->uvnum = 0;
     pArea->area_flags = 0;

     for ( ;; )
     {
          word = feof ( fpArea ) ? "End" : fread_word ( fpArea );

          switch ( UPPER ( word[0] ) )
          {
          case 'Z':
               KEY ( "Zone", pArea->zone, fread_number ( fpArea ) );
               break;
          case 'N':
               SKEY ( "Name", pArea->name );
               break;
          case 'S':
               KEY ( "Security", pArea->security,
                     fread_number ( fpArea ) );
               break;
          case 'V':
               if ( !str_cmp ( word, "VNUMs" ) )
               {
                    pArea->lvnum = fread_number ( fpArea );
                    pArea->uvnum = fread_number ( fpArea );
               }
               else if ( !str_cmp ( word, "Versions" ) )
               {
                    int  i;
                    for ( i = 0 ; i < 9 ; i++ )
                         versions[i] = fread_number ( fpArea );
               }
               break;
          case 'L':
               if ( !str_cmp ( word, "Levels" ) )
               {
                    pArea->llev = fread_number ( fpArea );
                    pArea->hlev = fread_number ( fpArea );
               }
          case 'E':
               if ( !str_cmp ( word, "End" ) )
               {
                    if ( area_first == NULL )
                         area_first = pArea;
                    if ( area_last != NULL )
                         area_last->next = pArea;
                    area_last = pArea;
                    pArea->next = NULL;
                    top_area++;
                    fclose ( fpArea );
                    return pArea;
               }
               break;
          case 'B':
               SKEY ( "Builders", pArea->builders );
               break;
          case 'M':
               SKEY ( "Music", pArea->soundfile );
               break;

          case 'C':
               SKEY ( "Credits", pArea->credits );
               break;

          }
          /* End of switch */
     }
     /* End of for */

     bugf ( "Premature close of %s (No End found)", strArea );
     fclose ( fpArea );
     return pArea;
}

void db_save_area_rooms ( char *filename, AREA_DATA *pArea )
{
     ROOM_INDEX_DATA    *pRoomIndex;
     EXTRA_DESCR_DATA   *pEd;
     EXIT_DATA          *pExit;
     PROG_LIST		*pRprog;
     int                 iHash;
     int                 door;
     char		 rfilename[MSL];
     FILE		 *fp;

     SNP ( rfilename, "%s%s.rooms", ROOM_DIR, filename );

     if ( !( fp = fopen ( rfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( rfilename );
     }

     /* Cycle through all the rooms */
     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
          {
               if ( pRoomIndex->area == pArea )
               {
                    fprintf ( fp, "#%d\n", 	pRoomIndex->vnum );
                    fprintf ( fp, "%s~\n", 	pRoomIndex->name );
                    fprintf ( fp, "%s~\n", 	fix_string ( pRoomIndex->description ) );
                    fprintf ( fp, "%s~\n", 	fix_string ( pRoomIndex->notes ) );
                    fprintf ( fp, "%d %d\n",    pRoomIndex->room_flags, pRoomIndex->sector_type );

                    /* Save Extended Descriptions */
                    for ( pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next )
                    {
                         fprintf ( fp, "E\n%s~\n%s~\n", pEd->keyword,
                                   fix_string ( pEd->description ) );
                    }

                    /* Save the doors for the room. */

                    for ( door = 0; door < MAX_DIR; door++ )    /* I hate this! */
                    {
                         if ( ( pExit = pRoomIndex->exit[door] ) && pExit->u1.to_room )
                         {
                              int locks = 0;

                              /*
                               * Copy the flags we want to save to locks.
                               * No error checking here, we assume elsewhere has made sure that no non-doors
                               * have flags, or that it doesn't matter.
                               */

                              if ( IS_SET ( pExit->rs_flags, EX_ISDOOR ) )      SET_BIT ( locks, EX_ISDOOR );
                              if ( IS_SET ( pExit->rs_flags, EX_PICKPROOF ) )   SET_BIT ( locks, EX_PICKPROOF );
                              if ( IS_SET ( pExit->rs_flags, EX_NO_PASS ) )     SET_BIT ( locks, EX_NO_PASS );
                              if ( IS_SET ( pExit->rs_flags, EX_HIDDEN ) )      SET_BIT ( locks, EX_HIDDEN );

                              fprintf ( fp, "D%d\n", 		door );
                              fprintf ( fp, "%s~\n", 		fix_string ( pExit->description ) );
                              fprintf ( fp, "%s~\n", 		pExit->keyword );
                              fprintf ( fp, "%d %d %d\n", 	locks, pExit->key, pExit->u1.to_room->vnum );
                         }
                    }
                    for (pRprog = pRoomIndex->rprogs; pRprog; pRprog = pRprog->next)
                    {
                         fprintf(fp, "R %s %d %s~\n",
                                 prog_type_to_name(pRprog->trig_type), pRprog->vnum,
                                 pRprog->trig_phrase);
                    }
                    fprintf ( fp, "S\n" );	/* Mark the end of this room. */
               }
          }
     }
     fprintf ( fp, "#0\n\n\n\n" );		/* mark the end of the rooms file */
     fclose ( fp );
     return;
}

/* Loads an area rooms file */
void db_load_area_rooms ( char *filename, AREA_DATA *pArea )
{
     ROOM_INDEX_DATA    *pRoomIndex;
     bool		 romcvta = FALSE;
     bool		 native = TRUE;
     
     if ( versions[DB_ROOMS] < 20 )
     {
          if ( versions[DB_ROOMS] == 2 ) /* 2 = Rom 2.3 */
          {
               native = FALSE;
               romcvta = TRUE;
          }
     }

     SNP ( strArea, "%s%s.rooms", ROOM_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     /* Okay we opened the rooms file, scan rooms */

     for ( ;; )
     {
          bool		oldval;
          bool		oldival;
          int		vnum;
          char          letter;
          int           door;
          int           iHash;

          letter = fread_letter ( fpArea );
          if ( letter != '#' )
          {
               bugf ( "load_area_rooms: # not found in %s.", strArea );
               exit ( 1 );
          }

          vnum = fread_number ( fpArea );
          if ( vnum == 0 )
               break;

          oldval = fBootDb;	/* Save value of fBootDB */
          oldival = fImportDb;
          
          fBootDb = FALSE;	/* Set to false */
          fImportDb = FALSE;
          if ( get_room_index ( vnum ) != NULL )
          {
               bugf ( "load_area_rooms: vnum %d duplicated.", vnum );
               exit ( 1 );
          }
          fBootDb = oldval;	/* Restore value of fBootDb */
          fImportDb = oldival;

          pRoomIndex = alloc_perm ( sizeof ( *pRoomIndex ), "pRoomIndex:load_area_rooms" );
          
          /* Clear a few values */
          pRoomIndex->people 		= NULL;
          pRoomIndex->contents 		= NULL;
          pRoomIndex->extra_descr 	= NULL;
          pRoomIndex->area 		= area_last;
          pRoomIndex->vnum 		= vnum;
          pRoomIndex->name 		= fread_string ( fpArea );
          pRoomIndex->description 	= fread_string ( fpArea );
          if ( native )
               pRoomIndex->notes 	= fread_string ( fpArea );
          else if ( romcvta )
          {
               pRoomIndex->notes 	= str_dup ( "" );
               fread_number ( fpArea );
          }
          pRoomIndex->room_flags 	= fread_flag ( fpArea );
          pRoomIndex->sector_type 	= fread_number ( fpArea );
          pRoomIndex->light 		= 0;
          pRoomIndex->lease 		= NULL;
          
          /* Do the lettered sections */
          for ( ;; )
          {
               letter = fread_letter ( fpArea );

               if ( letter == 'S' )		/* End of Room */
                    break;

               if ( letter == 'D' )		/* A door */
               {
                    EXIT_DATA          *pexit;
                    int			locks = 0;

                    door = fread_number ( fpArea );
                    if ( door < 0 || door > 5 )
                    {
                         bugf ( "Fread_rooms: vnum %d has bad door number.", vnum );
                         exit ( 1 );
                    }

                    pexit = alloc_perm ( sizeof ( *pexit ), "pexit:load_area_rooms" );

                    pexit->description 		= fread_string ( fpArea );
                    pexit->keyword 		= fread_string ( fpArea );
                    pexit->exit_info 		= 0;
                    
                    if ( native )
                         pexit->rs_flags 	= fread_number ( fpArea );
                    else if ( romcvta )
                         locks			= fread_number ( fpArea );

                    pexit->key 			= fread_number ( fpArea );
                    pexit->u1.vnum 		= fread_number ( fpArea );
                    pexit->orig_door 	= door;
                    if ( romcvta )
                    {
                         switch ( locks )
                         {
                         case 1: pexit->rs_flags = EX_ISDOOR;                break;
                         case 2: pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF; break;
                         }
                    }

                    if ( pexit->u1.vnum < 100 )
                         bugf ( "Fread_rooms (Non_Fatal): vnum %d has exit to limbo/void.", vnum);

                    pRoomIndex->exit[door] = pexit;
                    top_exit++;
               }
               else if ( letter == 'E' )
               {
                    EXTRA_DESCR_DATA   *ed;

                    ed = alloc_perm ( sizeof ( *ed ), "ed:load_area_rooms" );

                    ed->keyword 		= fread_string ( fpArea );
                    ed->description 		= fread_string ( fpArea );
                    ed->next 			= pRoomIndex->extra_descr;
                    pRoomIndex->extra_descr 	= ed;
                    top_ed++;
               }
               else if ( letter == 'R' )
               {
                    PROG_LIST *pRprog;
                    char *word;
                    int trigger = 0;

                    pRprog          = alloc_perm(sizeof(*pRprog), "roomprog" );
                    word            = fread_word( fpArea );

                    if ( !(trigger = flag_lookup( word, rprog_flags )) )
                    {
                         bugf( "ROOMprogs: invalid trigger: %s", word);
                         exit(1);
                    }

                    SET_BIT( pRoomIndex->rprog_flags, trigger );

                    pRprog->trig_type       	= trigger;
                    pRprog->vnum            	= fread_number( fpArea );
                    pRprog->trig_phrase     	= fread_string( fpArea );
                    pRprog->next            	= pRoomIndex->rprogs;
                    pRoomIndex->rprogs      	= pRprog;
               }
               else
               {
                    bugf ( "Load_rooms: vnum %d has flag not 'DESR': %c", vnum, letter );
                    exit ( 1 );
               }
          }

          iHash 		 = vnum % MAX_KEY_HASH;
          pRoomIndex->next 	 = room_index_hash[iHash];
          room_index_hash[iHash] = pRoomIndex;
          top_room++;
          top_vnum_room 	 = top_vnum_room < vnum ? vnum : top_vnum_room;
          assign_area_vnum ( vnum );
          
          /* Do necessary conversions */
          if ( romcvta )
               cvt_rom_room_a ( pRoomIndex );
          
     }
     fclose ( fpArea );
     return;
}

void db_save_area_mobiles ( char *filename, AREA_DATA *pArea )
{
     PROG_LIST 		*pMprog;
     char                buf[MAX_STRING_LENGTH];
     char                mfilename[MSL];
     FILE                *fp;
     int                 i;
     sh_int		 race;
     MOB_INDEX_DATA     *pMobIndex;

     SNP ( mfilename, "%s%s.mobiles", MOB_DIR, filename );

     if ( !( fp = fopen ( mfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( mfilename );
     }

     for ( i = pArea->lvnum; i <= pArea->uvnum; i++ )
     {
          if ( ( pMobIndex = get_mob_index ( i ) ) )
          {
               race = pMobIndex->race;

               fprintf ( fp, "#%d\n", pMobIndex->vnum );
               fprintf ( fp, "%s~\n", pMobIndex->player_name );
               fprintf ( fp, "%s~\n", pMobIndex->short_descr );
               fprintf ( fp, "%s~\n", fix_string ( pMobIndex->long_descr ) );
               fprintf ( fp, "%s~\n", fix_string ( pMobIndex->description ) );
               fprintf ( fp, "%s~\n", fix_string ( pMobIndex->notes ) );
               fprintf ( fp, "%s~\n", race_table[race].name );
               fprintf ( fp, "%s ",   fwrite_flag ( pMobIndex->act, buf ) );
               fprintf ( fp, "%s ",   fwrite_flag( pMobIndex->affected_by, buf ) );
               fprintf ( fp, "%s ",   fwrite_flag( pMobIndex->detections, buf ) );
               fprintf ( fp, "%s\n",  fwrite_flag( pMobIndex->protections, buf ) );
               fprintf ( fp, "%d ",   pMobIndex->alignment );
               fprintf ( fp, "%d ",   pMobIndex->level );
               fprintf ( fp, "%d ",   pMobIndex->hitroll );
               fprintf ( fp, "%dd%d+%d ",
                         pMobIndex->hit[DICE_NUMBER],
                         pMobIndex->hit[DICE_TYPE],
                         pMobIndex->hit[DICE_BONUS] );
               fprintf ( fp, "%dd%d+%d ",
                         pMobIndex->mana[DICE_NUMBER],
                         pMobIndex->mana[DICE_TYPE],
                         pMobIndex->mana[DICE_BONUS] );
               fprintf ( fp, "%dd%d+%d ",
                         pMobIndex->damage[DICE_NUMBER],
                         pMobIndex->damage[DICE_TYPE],
                         pMobIndex->damage[DICE_BONUS] );
               fprintf ( fp, "%d\n", pMobIndex->dam_type );
               fprintf ( fp, "%s ",               fwrite_flag ( pMobIndex->off_flags, buf ) );
               fprintf ( fp, "%s ",               fwrite_flag ( pMobIndex->imm_flags, buf ) );
               fprintf ( fp, "%s ",               fwrite_flag ( pMobIndex->res_flags, buf ) );
               fprintf ( fp, "%s\n",              fwrite_flag ( pMobIndex->vuln_flags, buf ) );
               fprintf ( fp, "%d %d %d %ld\n",
                         pMobIndex->start_pos,
                         pMobIndex->default_pos,
                         pMobIndex->sex,
                         pMobIndex->gold );
               fprintf ( fp, "%s ", fwrite_flag ( pMobIndex->form, buf ) );
               fprintf ( fp, "%s ", fwrite_flag ( pMobIndex->parts, buf ) );
               fprintf ( fp, "%d ", pMobIndex->size );
               fprintf ( fp, "%s\n", material_name ( pMobIndex->material ) );

               if ( IS_SET ( pMobIndex->act, ACT_SKILLMASTER ) )
               {
                    int count;
                    int total_to_write = pMobIndex->total_teach_skills;
                    fprintf ( fp, "Z %d\n", total_to_write );
                    for ( count = 0; count < total_to_write; count++ )
                         fprintf ( fp, "%s~\n", pMobIndex->teach_skills[count] );
               }
               for (pMprog = pMobIndex->mprogs; pMprog; pMprog = pMprog->next)
               {
                    fprintf(fp, "M %s %d %s~\n",
                            prog_type_to_name(pMprog->trig_type), pMprog->vnum,
                            pMprog->trig_phrase);
               }
          }
     }

     fprintf ( fp, "#0\n\n\n\n" );
     fclose ( fp );
     return;
}

/* Load Mobs file */

void db_load_area_mobiles ( char *filename, AREA_DATA *pArea )
{
     MOB_INDEX_DATA     *pMobIndex;
     bool                romcvta = FALSE;
     bool                native = TRUE;
     
     if ( versions[DB_MOBS] == 2 ) /* 2 = Rom 2.3 */
     {
          native = FALSE;
          romcvta = TRUE;
     }

     SNP ( strArea, "%s%s.mobiles", MOB_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ;; )
     {
          bool  oldval;
          bool  oldival;
          int	vnum;
          char  letter;
          int   iHash;

          letter = fread_letter ( fpArea );
          if ( letter != '#' )
          {
               bugf ( "load_area_mobiles: # not found in %s.", strArea );
               exit ( 1 );
          }

          vnum = fread_number ( fpArea );
          if ( vnum == 0 )
               break;

          oldval = fBootDb;
          oldival = fImportDb;
          
          fBootDb = FALSE;
          fImportDb = FALSE;
          if ( get_mob_index ( vnum ) != NULL )
          {
               bugf ( "load_area_mobiles: vnum %d duplicated.", vnum );
               exit ( 1 );
          }
          fImportDb = oldival;
          fBootDb = oldval;

          pMobIndex = alloc_perm ( sizeof ( *pMobIndex ), "pMobIndex:load_area_mobiles" );

          pMobIndex->new_format 	= TRUE; /* This needs to go, vestigal tail */

          pMobIndex->vnum 		= vnum;
          pMobIndex->area 		= area_last;
          newmobs++;
          pMobIndex->player_name 	= fread_string ( fpArea );
          pMobIndex->short_descr 	= fread_string ( fpArea );
          pMobIndex->long_descr 	= fread_string ( fpArea );
          pMobIndex->description 	= fread_string ( fpArea );
          if ( native )
               pMobIndex->notes 		= fread_string ( fpArea );
          pMobIndex->race 		= race_lookup ( fread_string ( fpArea ) );
          pMobIndex->long_descr[0] 	= UPPER ( pMobIndex->long_descr[0] );
          pMobIndex->description[0] 	= UPPER ( pMobIndex->description[0] );
          if ( native )
               pMobIndex->act 		= fread_flag ( fpArea ) | ACT_IS_NPC | race_table[pMobIndex->race].act;
          pMobIndex->affected_by 	= fread_flag ( fpArea );
          if ( native )
          {
               pMobIndex->detections 	= fread_flag ( fpArea );
               pMobIndex->protections 	= fread_flag ( fpArea );
          }
          pMobIndex->pShop 		= NULL;
          pMobIndex->alignment 		= fread_number ( fpArea );
          if ( romcvta )
               letter = fread_letter ( fpArea );
          pMobIndex->level 		= fread_number ( fpArea );
          pMobIndex->hitroll 		= fread_number ( fpArea );
          pMobIndex->hit[DICE_NUMBER] 	= fread_number ( fpArea );  fread_letter ( fpArea ); /* d */
          pMobIndex->hit[DICE_TYPE] 	= fread_number ( fpArea );  fread_letter ( fpArea ); /* + */
          pMobIndex->hit[DICE_BONUS] 	= fread_number ( fpArea );
          pMobIndex->mana[DICE_NUMBER]	= fread_number ( fpArea );  fread_letter ( fpArea ); /* d */
          pMobIndex->mana[DICE_TYPE] 	= fread_number ( fpArea );  fread_letter ( fpArea ); /* + */
          pMobIndex->mana[DICE_BONUS] 	= fread_number ( fpArea );
          pMobIndex->damage[DICE_NUMBER]= fread_number ( fpArea );  fread_letter ( fpArea ); /* d */
          pMobIndex->damage[DICE_TYPE]  = fread_number ( fpArea );  fread_letter ( fpArea ); /* + */
          pMobIndex->damage[DICE_BONUS] = fread_number ( fpArea );
          pMobIndex->dam_type 		= fread_number ( fpArea );
          if ( romcvta)
          {
               fread_number ( fpArea );	fread_number ( fpArea ); /* Unused AC Values */
               fread_number ( fpArea ); fread_number ( fpArea ); /* Unused AC Values */
          }
          if ( native )
          {
               pMobIndex->off_flags 		= fread_flag ( fpArea ) | race_table[pMobIndex->race].off;
               pMobIndex->imm_flags 		= fread_flag ( fpArea ) | race_table[pMobIndex->race].imm;
               pMobIndex->res_flags 		= fread_flag ( fpArea ) | race_table[pMobIndex->race].res;
               pMobIndex->vuln_flags		= fread_flag ( fpArea ) | race_table[pMobIndex->race].vuln;
          }
          else if ( romcvta )
          {
               pMobIndex->off_flags     = fread_flag ( fpArea );
               pMobIndex->imm_flags     = fread_flag ( fpArea );
               pMobIndex->res_flags     = fread_flag ( fpArea );
               pMobIndex->vuln_flags    = fread_flag ( fpArea );
          }
          pMobIndex->start_pos 		= fread_number ( fpArea );
          pMobIndex->default_pos 	= fread_number ( fpArea );
          pMobIndex->sex 		= fread_number ( fpArea );
          pMobIndex->gold 		= fread_number ( fpArea );
          if ( native )
          {
               pMobIndex->form 		= fread_flag ( fpArea ) | race_table[pMobIndex->race].form;
               pMobIndex->parts 	= fread_flag ( fpArea ) | race_table[pMobIndex->race].parts;
               pMobIndex->size 		= fread_number ( fpArea );
          }
          else if ( romcvta )
          {
               pMobIndex->form          = fread_flag ( fpArea );
               pMobIndex->parts         = fread_flag ( fpArea );
               letter                   = fread_letter( fpArea );
               switch (letter)
               {
               case ('T') :                pMobIndex->size = SIZE_TINY;    break;
               case ('S') :                pMobIndex->size = SIZE_SMALL;   break;
               case ('M') :                pMobIndex->size = SIZE_MEDIUM;  break;
               case ('L') :                pMobIndex->size = SIZE_LARGE;   break;
               case ('H') :                pMobIndex->size = SIZE_HUGE;    break;
               case ('G') :                pMobIndex->size = SIZE_GIANT;   break;
               default:                    pMobIndex->size = SIZE_MEDIUM;  break;
               }
          }
          pMobIndex->material 		= material_lookup ( fread_word ( fpArea ) );

          if ( native )
          {
               /* Skillmasters */
               pMobIndex->total_teach_skills = 0;
               {
                    int                 count = MAX_TEACH_SKILLS;
                    
                    for ( count = 0; count < MAX_TEACH_SKILLS; count++ )
                         pMobIndex->teach_skills[count] = NULL;
               }
          }

          while ( 1 )
          {
               letter = fread_letter ( fpArea );

               if ( letter == 'Z' )
               {
                    char               *tmp;
                    int                 total_to_read;
                    int                 count;

                    if ( !IS_SET ( pMobIndex->act, ACT_SKILLMASTER ) )
                         SET_BIT ( pMobIndex->act, ACT_SKILLMASTER );

                    total_to_read = fread_number ( fpArea );
                    if ( total_to_read > MAX_TEACH_SKILLS )
                         total_to_read = MAX_TEACH_SKILLS;
                    for ( count = 0; count < total_to_read; count++ )
                    {
                         tmp = fread_string ( fpArea );
                         if ( skill_lookup ( tmp ) == -1 )
                         {
                              bugf ( "Invalid skill in load_mobile for skillmaster: %s ", tmp );
                              pMobIndex->teach_skills[count] = NULL;
                         }
                         else
                         {
                              pMobIndex->teach_skills[count] = str_dup ( tmp );
                              pMobIndex->total_teach_skills++;
                         }
                    }
               }
               else if ( letter == 'M' )
               {
                    PROG_LIST *pMprog;
                    char *word;
                    int trigger = 0;

                    pMprog              = alloc_perm(sizeof(*pMprog), "pMprog:load_mobiles");
                    word                = fread_word( fpArea );

                    if ( (trigger = flag_lookup( word, mprog_flags )) == NO_FLAG )
                    {
                         bugf("MOBprogs: invalid trigger." );
                         exit(1);
                    }

                    SET_BIT( pMobIndex->mprog_flags, trigger );
                    pMprog->trig_type   = trigger;
                    pMprog->vnum        = fread_number( fpArea );
                    pMprog->trig_phrase = fread_string( fpArea );
                    pMprog->next        = pMobIndex->mprogs;
                    pMobIndex->mprogs   = pMprog;
               }
               else
               {
                    ungetc ( letter, fpArea ); break;
               }

          }
          /* end of while ( 1 ) */

          iHash = vnum % MAX_KEY_HASH;
          pMobIndex->next = mob_index_hash[iHash];
          mob_index_hash[iHash] = pMobIndex;
          top_mob_index++;
          top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
          assign_area_vnum ( vnum );
          kill_table[URANGE( 0, pMobIndex->level, MAX_LEVEL - 1 )].number++;
     }
     /* End of for ( ;; ) */

     fclose ( fpArea );
     return;
}

void db_save_area_objects ( char *filename, AREA_DATA *pArea )
{
     AFFECT_DATA        *pAf;
     EXTRA_DESCR_DATA   *pEd;
     PROG_LIST          *pOprog;
     char                buf[MAX_STRING_LENGTH];
     int                 i;
     OBJ_INDEX_DATA     *pObjIndex;
     char                ofilename[MSL];
     FILE                *fp;

     SNP ( ofilename, "%s%s.objects", OBJECT_DIR, filename );

     if ( !( fp = fopen ( ofilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( ofilename );
     }

     for ( i = pArea->lvnum; i <= pArea->uvnum; i++ )
     {
          if ( ( pObjIndex = get_obj_index ( i ) ) )
          {
               fprintf ( fp, "#%d\n", pObjIndex->vnum );
               fprintf ( fp, "%s~\n", pObjIndex->name );
               fprintf ( fp, "%s~\n", pObjIndex->short_descr );
               fprintf ( fp, "%s~\n", pObjIndex->description );
               fprintf ( fp, "%s~\n", fix_string ( pObjIndex->notes) );
               fprintf ( fp, "%s~\n", material_name ( pObjIndex->material ) );
               fprintf ( fp, "%d ",   pObjIndex->item_type );
               fprintf ( fp, "%s ",   fwrite_flag ( pObjIndex->extra_flags, buf ) );
               fprintf ( fp, "%s ",   fwrite_flag ( pObjIndex->wear_flags, buf ) );
               fprintf ( fp, "%s\n",  fwrite_flag ( pObjIndex->vflags, buf ) );      /* New with this format */

               switch ( pObjIndex->item_type )
               {
               default:
                    fprintf ( fp, "%s ",  fwrite_flag ( pObjIndex->value[0], buf ) );
                    fprintf ( fp, "%s ",  fwrite_flag ( pObjIndex->value[1], buf ) );
                    fprintf ( fp, "%s ",  fwrite_flag ( pObjIndex->value[2], buf ) );
                    fprintf ( fp, "%s ",  fwrite_flag ( pObjIndex->value[3], buf ) );
                    fprintf ( fp, "%s\n", fwrite_flag ( pObjIndex->value[4], buf ) );
                    break;
               case ITEM_LIGHT:
                    fprintf ( fp, "0 0 %d 0 0\n", pObjIndex->value[2] < 1 ? 999   /* infinite */
                              : pObjIndex->value[2] );
                    break;
               case ITEM_PILL:
               case ITEM_POTION:
               case ITEM_SCROLL:
                    fprintf ( fp, "%d %d %d %d %d\n", pObjIndex->value[0] > 0 ?   /* no negative numbers */
                              pObjIndex->value[0] : 0,
                              pObjIndex->value[1] != -1 ?
                              skill_table[pObjIndex->value[1]].slot : 0,
                              pObjIndex->value[2] != -1 ?
                              skill_table[pObjIndex->value[2]].slot : 0,
                              pObjIndex->value[3] != -1 ?
                              skill_table[pObjIndex->value[3]].slot : 0,
                              0 /* unused */  );
                    break;
               case ITEM_STAFF:
               case ITEM_WAND:
                    fprintf ( fp, "%s ",
                              fwrite_flag ( pObjIndex->value[0], buf ) );
                    fprintf ( fp, "%s ",
                              fwrite_flag ( pObjIndex->value[1], buf ) );
                    fprintf ( fp, "%s %d 0\n",
                              fwrite_flag ( pObjIndex->value[2], buf ),
                              pObjIndex->value[3] != -1 ? skill_table[pObjIndex->value[3]].slot : 0 );
                    break;
               }

               fprintf ( fp, "%d ", 	pObjIndex->level );
               fprintf ( fp, "%d ", 	pObjIndex->weight );
               fprintf ( fp, "%d ", 	pObjIndex->cost );
               fprintf ( fp, "%d ", 	pObjIndex->condition );
               fprintf ( fp, "%d\n", 	pObjIndex->repop );

               /* Write out affects. */
               for ( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
               {
                    if (pAf->bitvector)
                    {
                         fprintf( fp, "F\n" );
                         switch (pAf->where)
                         {
                         case TO_AFFECTS:         fprintf ( fp, "A " ); break;
                         case TO_DETECTIONS:      fprintf ( fp, "D " ); break;
                         case TO_PROTECTIONS:     fprintf ( fp, "P " ); break;
                         default:
                              bugf ( "olc_save: Invalid Affect->where");
                              break;
                         }
                         fprintf( fp, "%d %d %s\n", pAf->location, pAf->modifier,
                                  fwrite_flag( pAf->bitvector, buf ) );
                    }
                    else
                         fprintf( fp, "A\n%d %d\n",  pAf->location, pAf->modifier );
               }

               /* Write out extended descriptions */
               for ( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
               {
                    fprintf ( fp, "E\n%s~\n%s~\n", pEd->keyword, fix_string ( pEd->description ) );
               }
               for (pOprog = pObjIndex->oprogs; pOprog; pOprog = pOprog->next)
               {
                    fprintf(fp, "O %s %d %s~\n",
                            prog_type_to_name(pOprog->trig_type), pOprog->vnum,
                            pOprog->trig_phrase);
               }
          }
     }

     fprintf ( fp, "#0\n\n\n\n" );
     fclose ( fp );
     return;
}

void db_load_area_objects ( char *filename, AREA_DATA *pArea )
{
     OBJ_INDEX_DATA * pObjIndex;
     bool                romcvta = FALSE;
     bool                native = TRUE;
     
     if ( versions[DB_OBJS] == 2 ) /* 2 = Rom 2.3 */
     {
          native = FALSE;
          romcvta = TRUE;
     }     

     SNP ( strArea, "%s%s.objects", OBJECT_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ;; )
     {
          bool oldval;
          bool oldival;
          int  vnum;
          char letter;
          int  iHash;

          letter = fread_letter ( fpArea );
          if ( letter != '#' )
          {
               bugf ( "Load_objects: # not found in %s.", strArea );
               exit ( 1 );
          }

          vnum = fread_number ( fpArea );
          if ( vnum == 0 )
               break;

          oldval = fBootDb;
          oldival = fImportDb;
          
          fBootDb = FALSE;
          fImportDb = FALSE;
          if ( get_obj_index ( vnum ) != NULL )
          {
               bugf ( "Load_objects: vnum %d duplicated.", vnum );
               exit ( 1 );
          }
          fImportDb = oldival;
          fBootDb = oldval;
          
          pObjIndex = alloc_perm ( sizeof ( *pObjIndex ), "pObjIndex:load_area_objects" );

          pObjIndex->new_format 	= TRUE;

          pObjIndex->vnum 		= vnum;
          pObjIndex->area 		= area_last;
          pObjIndex->reset_num 		= 0;
          newobjs++;
          pObjIndex->name 		= fread_string ( fpArea );
          pObjIndex->short_descr 	= fread_string ( fpArea );
          pObjIndex->description 	= fread_string ( fpArea );
          if ( native )               
               pObjIndex->notes 		= fread_string ( fpArea );
          pObjIndex->material 		= material_lookup ( fread_string ( fpArea ) );
          pObjIndex->item_type 		= fread_number ( fpArea );
          pObjIndex->extra_flags 	= fread_flag ( fpArea );
          pObjIndex->wear_flags 	= fread_flag ( fpArea );
          if ( native )
               pObjIndex->vflags 		= fread_flag ( fpArea );
          pObjIndex->value[0] 		= fread_flag ( fpArea );
          pObjIndex->value[1] 		= fread_flag ( fpArea );
          pObjIndex->value[2] 		= fread_flag ( fpArea );
          pObjIndex->value[3] 		= fread_flag ( fpArea );
          pObjIndex->value[4] 		= fread_flag ( fpArea );
          pObjIndex->level 		= fread_number ( fpArea );
          pObjIndex->weight 		= fread_number ( fpArea );
          pObjIndex->cost 		= fread_number ( fpArea );
          if ( native )
          {
               pObjIndex->condition		= fread_number ( fpArea );
               pObjIndex->repop		= fread_number ( fpArea );
          }

          /* Get lettered values */
          for ( ;; )
          {
               char letter;
               letter = fread_letter ( fpArea );
               if ( letter == 'A' )
               {
                    AFFECT_DATA * paf;

                    paf 		= alloc_perm ( sizeof ( *paf ), "paf:load_area_objects" );
                    paf->type 		= -1;
                    paf->level 		= pObjIndex->level;
                    paf->duration 	= -1;
                    paf->location 	= fread_number ( fpArea );
                    paf->modifier 	= fread_number ( fpArea );
                    paf->bitvector 	= 0;
                    paf->next 		= pObjIndex->affected;
                    pObjIndex->affected = paf; top_affect++;
               }
               else if ( letter == 'N' )
               {
                    AFFECT_DATA * paf;

                    int slot_num;
                    paf 		= alloc_perm ( sizeof ( *paf ), "paf:load_area_objects" );
                    slot_num 		= fread_number ( fpArea );
                    paf->type 		= -1;
                    paf->level 		= pObjIndex->level;
                    paf->duration 	= -1;
                    paf->location 	= fread_number ( fpArea );
                    paf->modifier 	= fread_number ( fpArea );
                    paf->where 		= TO_AFFECTS;
                    paf->bitvector 	= fread_number ( fpArea );
                    paf->next 		= pObjIndex->affected;
                    pObjIndex->affected = paf;
                    top_affect++;
               }
               else if (letter == 'F')
               {
                    AFFECT_DATA *paf;

                    paf                     = alloc_perm( sizeof(*paf), "paf:load_area_objects" );
                    letter                  = fread_letter(fpArea);
                    switch (letter)
                    {
                    case 'A':
                         paf->where         = TO_AFFECTS;
                         break;
                    case 'D':
                         paf->where         = TO_DETECTIONS;
                         break;
                    case 'P':
                         paf->where         = TO_PROTECTIONS;
                         break;
                    default:
                         bugf ( "Load_objects: Bad where on flag set." );
                         exit( 1 );
                    }
                    paf->type               = -1;
                    paf->level              = pObjIndex->level;
                    paf->duration           = -1;
                    paf->location           = fread_number(fpArea);
                    paf->modifier           = fread_number(fpArea);
                    paf->bitvector          = fread_flag(fpArea);
                    paf->next               = pObjIndex->affected;
                    pObjIndex->affected     = paf;
                    top_affect++;
               }
               else if ( letter == 'E' )
               {
                    EXTRA_DESCR_DATA * ed;

                    ed 			= alloc_perm ( sizeof ( *ed ), "ed:load_area_objects" );
                    ed->keyword 	= fread_string ( fpArea );
                    ed->description 	= fread_string ( fpArea );
                    ed->next 		= pObjIndex->extra_descr;
                    pObjIndex->extra_descr = ed;
                    top_ed++;
               }
               else if ( letter == 'O' )
               {
                    PROG_LIST *pOprog;
                    char *word;
                    int trigger = 0;

                    pOprog                  = alloc_perm(sizeof(*pOprog), "oprogs");
                    word                    = fread_word( fpArea );
                    if ( !(trigger = flag_lookup( word, oprog_flags )) )
                    {
                         bugf( "OBJprogs: invalid trigger.");
                         exit(1);
                    }
                    SET_BIT( pObjIndex->oprog_flags, trigger );
                    pOprog->trig_type       = trigger;
                    pOprog->vnum            = fread_number( fpArea );
                    pOprog->trig_phrase     = fread_string( fpArea );
                    pOprog->next            = pObjIndex->oprogs;
                    pObjIndex->oprogs       = pOprog;
               }
               else
               {
                    ungetc ( letter, fpArea );
                    break;
               }
          }
          /* Lettered values for ( ;; ) */
          /* Translate spell "slot numbers" to internal "skill numbers." */
          switch ( pObjIndex->item_type )
          {
          case ITEM_PILL:
          case ITEM_POTION:
          case ITEM_SCROLL:
               pObjIndex->value[1] = slot_lookup ( pObjIndex->value[1] );
               pObjIndex->value[2] = slot_lookup ( pObjIndex->value[2] );
               pObjIndex->value[3] = slot_lookup ( pObjIndex->value[3] );
               break;
          case ITEM_STAFF:
          case ITEM_WAND:
               pObjIndex->value[3] = slot_lookup ( pObjIndex->value[3] );
               break;
          }
          
          /* Check the validity of some of the flags - Lotherius */
          /* Flags but no Take */
          if ( !IS_SET ( pObjIndex->wear_flags, ITEM_TAKE ) && pObjIndex->wear_flags > 0 )
               bugf ( "\nObject Vnum %d has wear flags %ld with no TAKE flag",
                      pObjIndex->vnum, pObjIndex->wear_flags );
          /* Hold flag with other flags */
          if ( IS_SET ( pObjIndex->wear_flags, ITEM_HOLD ) )
          {
               long ibid;

               ibid = pObjIndex->wear_flags;
               ibid -= ITEM_HOLD;               // Knock off the 2 flags that we know are here.
               ibid -= ITEM_TAKE;

               if ( ibid > 0 )                  // Then there shouldn't be any flags left.
               {
                    if ( ibid == ITEM_WEAR_SHIELD ) // Probably the most common occurance, fix automatically
                    {
                         bugf ( "\nObj VNUM %d has HOLD and SHIELD both set. Removing HOLD flag.\n"
                                "To make this change permanent, please do an ASAVE. If this is the wrong"
                                "change,\nthen fix it manually.", pObjIndex->vnum );
                         pObjIndex->wear_flags -= ITEM_HOLD;
                    }
                    else
                    {
                         bugf ( "\nObj VNUM %d has too many wear flags. Please check.", pObjIndex->vnum );
                    }
               }
          }
          /* Find items that are wieldable that aren't weapons */
          if ( IS_SET ( pObjIndex->wear_flags, ITEM_WIELD ) && pObjIndex->item_type != ITEM_WEAPON )
               bugf ( " Obj VNUM %d: Non-Weapon Wieldable?", pObjIndex->vnum );
          /* Find shields that aren't armor */
          if ( IS_SET ( pObjIndex->wear_flags, ITEM_WEAR_SHIELD ) && pObjIndex->item_type != ITEM_ARMOR )
               bugf ( " Obj VNUM %d: Non-Armor Shield?", pObjIndex->vnum );

          
          iHash 		= vnum % MAX_KEY_HASH;
          pObjIndex->next 	= obj_index_hash[iHash];
          obj_index_hash[iHash] = pObjIndex; top_obj_index++;
          top_vnum_obj 		= top_vnum_obj < vnum ? vnum : top_vnum_obj;
          assign_area_vnum ( vnum );
     }
     /* End of for ( ;; ) */
     fclose ( fpArea );
     return;
}

void db_save_area_resets ( char *filename, AREA_DATA *pArea )
{
     char                rfilename[MSL];
     FILE                *fp;
     int                 iHash;
     RESET_DATA         *pReset;
     MOB_INDEX_DATA     *pLastMob = NULL;
     OBJ_INDEX_DATA     *pLastObj;
     ROOM_INDEX_DATA    *pRoomIndex;
     EXIT_DATA          *pExit;
     int                 door;

     SNP ( rfilename, "%s%s.resets", RESET_DIR, filename );

     if ( !( fp = fopen ( rfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( rfilename );
     }

     /* Write out door resets */

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
          {
               if ( pRoomIndex->area == pArea )
               {
                    for ( door = 0; door < MAX_DIR; door++ )
                    {
                         int                 locks = 0;
                         if ( ( pExit = pRoomIndex->exit[door] ) && pExit->u1.to_room
                              && ( IS_SET ( pExit->rs_flags, EX_CLOSED ) || IS_SET ( pExit->rs_flags, EX_LOCKED ) ) )
                         {
                              if ( IS_SET ( pExit->rs_flags, EX_ISDOOR ) && ( !IS_SET ( pExit->rs_flags, EX_LOCKED ) ) )
                                   locks = 1;
                              if ( IS_SET ( pExit->rs_flags, EX_ISDOOR ) && ( IS_SET ( pExit->rs_flags, EX_LOCKED ) ) )
                                   locks = 2;
                              if ( pExit->key == 0 )
                                   pExit->key = -1;
                              // fprintf ( fp, "D 0 %d %d %d\n", pRoomIndex->vnum, door, locks );
                              fprintf ( fp, "D %d %d %d\n", pRoomIndex->vnum, door, locks );
                         }
                    }
               }
          }
     }

     /* Write out other resets */
     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
          {
               if ( pRoomIndex->area == pArea )
               {
                    for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
                    {
                         /* Pulled the leading Zero off of all the following */
                         switch ( pReset->command )
                         {
                         default:
                              bugf ( "Save_resets: bad command %c.", pReset->command );
                              break;
                         case 'M':
                              pLastMob = get_mob_index ( pReset->arg1 );
                              fprintf ( fp, "M %d %d %d\n",
                                        pReset->arg1,
                                        pReset->arg2, pReset->arg3 );
                              break;
                         case 'O':
                              pLastObj = get_obj_index ( pReset->arg1 );
                              pRoomIndex = get_room_index ( pReset->arg3 );
                              fprintf ( fp, "O %d %d %d\n",
                                        pReset->arg1,
                                        pReset->arg2, pReset->arg3 );
                              break;
                         case 'P':
                              pLastObj = get_obj_index ( pReset->arg1 );
                              fprintf ( fp, "P %d %d %d\n",
                                        pReset->arg1,
                                        pReset->arg2, pReset->arg3 );
                              break;
                         case 'G':
                              fprintf ( fp, "G %d %d\n", pReset->arg1, pReset->arg2 );
                              if ( !pLastMob )
                                   bugf ( "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                              break;
                         case 'E':
                              fprintf ( fp, "E %d %d %d\n", pReset->arg1, pReset->arg2, pReset->arg3 );
                              if ( !pLastMob )
                                   bugf ( "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                              break;
                         case 'D':
                              /* Doors are done previously. */
                              break;
                         case 'R':
                              pRoomIndex = get_room_index ( pReset->arg1 );
                              fprintf ( fp, "R %d %d\n", pReset->arg1, pReset->arg2 );
                              break;
                         }
                         // end of switch
                    }
                    // end of for
               }
               // end of if
          }
          // end of for
     }
     // end of for
     //
     fprintf ( fp, "S\n\n\n\n" );
     fclose ( fp );
     return;
}

void db_load_area_resets ( char *filename, AREA_DATA *pArea )
{
     RESET_DATA *pReset;
     int         iLastRoom = 0;
     int         iLastObj = 0;

     SNP ( strArea, "%s%s.resets", RESET_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ;; )
     {
          EXIT_DATA          *pexit;
          ROOM_INDEX_DATA    *pRoomIndex;
          char                letter;
          bool		      fix = FALSE;

          /* The "fix" toggle is for importation of older format (Rom style) resets that don't save
           * per-reset counters and thus have pileup issues - Not used normally.. */

          if ( ( letter = fread_letter ( fpArea ) ) == 'S' )
               break;

          if ( letter == '*' )
          {
               fread_to_eol ( fpArea );
               continue;
          }

          pReset 		= alloc_perm ( sizeof ( *pReset ), "pReset:load_area_resets" );
          pReset->command 	= letter;
          pReset->arg1 		= fread_number ( fpArea );
          pReset->arg2 		= fread_number ( fpArea );
          if ( pReset->arg2 == -1 ) pReset->arg2 = 9999; /* Mess with infinity... this needs standardized */
          pReset->arg3 		= ( letter == 'G' || letter == 'R' ) ? 0 : fread_number ( fpArea );
          fread_to_eol ( fpArea );				 /* Skip any comments from verbose modes 	  */
          pReset->count = 0;

          /* Validate parameters.
           * We're calling the index functions for the side effect. */

          switch ( letter )
          {
          default:
               bugf ( "Load_resets: bad command '%c'.", letter );
               exit ( 1 );
               break;
          case 'M':
               get_mob_index ( pReset->arg1 );
               if ( fix == TRUE )
                    pReset->arg2 = 1;   /* reset max # to 1 */
               if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
               {
                    new_reset ( pRoomIndex, pReset );
                    iLastRoom = pReset->arg3;
               }
               break;
          case 'O':
               get_obj_index ( pReset->arg1 );
               if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
               {
                    new_reset ( pRoomIndex, pReset );
                    iLastObj = pReset->arg3;
               }
               break;
          case 'P':
               get_obj_index ( pReset->arg1 );
               if ( ( pRoomIndex = get_room_index ( iLastObj ) ) )
               {
                    new_reset ( pRoomIndex, pReset );
               }
               break;
          case 'G':
          case 'E':
               get_obj_index ( pReset->arg1 );
               if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
               {
                    new_reset ( pRoomIndex, pReset );
                    iLastObj = iLastRoom;
               }
               break;
          case 'D':
               pRoomIndex = get_room_index ( pReset->arg1 );

               if ( pReset->arg2 < 0
                    || pReset->arg2 > 5
                    || !pRoomIndex
                    || !( pexit = pRoomIndex->exit[pReset->arg2] )
                    || !IS_SET ( pexit->rs_flags, EX_ISDOOR ) )
               {
                    bugf ( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
                    exit ( 1 );
               }
               switch ( pReset->arg3 )
               {
               default:
                    bugf ( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
               case 0:
                    break;
               case 1:
                    SET_BIT ( pexit->rs_flags, EX_CLOSED );
                    break;
               case 2:
                    SET_BIT ( pexit->rs_flags, EX_LOCKED );
                    SET_BIT ( pexit->rs_flags, EX_CLOSED );
                    break;
               }
          case 'R':
               if ( pReset->arg2 < 0 || pReset->arg2 > 6 )      /* Last Door. */
               {
                    bugf ( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                    exit ( 1 );
               }
               if ( ( pRoomIndex = get_room_index ( pReset->arg1 ) ) )
                    new_reset ( pRoomIndex, pReset );
               break;
          }
          /* End of switch */

     }
     /* End of for ( ;; ) */

     fclose ( fpArea );
     return;
}

void db_save_area_shops ( char *filename, AREA_DATA *pArea )
{
     SHOP_DATA          *pShopIndex;
     MOB_INDEX_DATA     *pMobIndex;
     int                 iTrade;
     int                 iHash;
     char 		 sfilename[MSL];
     FILE 		*fp;

     SNP ( sfilename, "%s%s.shops", SHOP_DIR, filename );

     if ( !( fp = fopen ( sfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( sfilename );
     }

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
          {
               if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
               {
                    pShopIndex = pMobIndex->pShop;
                    fprintf ( fp, "%d ", pShopIndex->keeper );
                    for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                         fprintf ( fp, "%d ", pShopIndex->buy_type[iTrade] );
                    fprintf ( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                    fprintf ( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
               }
          }
     }

     fprintf ( fp, "0\n\n\n\n" );

     fclose ( fp );
     return;
}

void db_load_area_shops ( char *filename, AREA_DATA *pArea )
{
     SHOP_DATA  *pShop;
     
     SNP ( strArea, "%s%s.shops", SHOP_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ;; )
     {
          MOB_INDEX_DATA     *pMobIndex;
          int                 iTrade;

          pShop 		= alloc_perm ( sizeof ( *pShop ), "pShop:load_area_shops" );
          pShop->keeper 	= fread_number ( fpArea );

          if ( pShop->keeper == 0 )
               break;
          for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
               pShop->buy_type[iTrade] 	= fread_number ( fpArea );

          pShop->profit_buy 		= fread_number ( fpArea );
          pShop->profit_sell 		= fread_number ( fpArea );
          pShop->open_hour 		= fread_number ( fpArea );
          pShop->close_hour 		= fread_number ( fpArea );
          fread_to_eol ( fpArea );
          pMobIndex 			= get_mob_index ( pShop->keeper );
          pMobIndex->pShop 		= pShop;
          if ( shop_first == NULL )
               shop_first 		= pShop;
          if ( shop_last != NULL )
               shop_last->next 		= pShop;
          shop_last 			= pShop;
          pShop->next 			= NULL;
          top_shop++;
     }

     fclose ( fpArea );
     return;
}

void db_save_area_specials ( char *filename, AREA_DATA *pArea )
{
     int                 iHash;
     MOB_INDEX_DATA     *pMobIndex;
     char 		 sfilename[MSL];
     FILE 		 *fp;

     SNP ( sfilename, "%s%s.specials", SPEC_DIR, filename );

     if ( !( fp = fopen ( sfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( sfilename );
     }

     for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
     {
          for ( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
          {
               if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun )
               {
                    /* removed words "Load to:" from below as well as short_descr*/
                    fprintf ( fp, "M %d %s\n",
                              pMobIndex->vnum,
                              spec_string ( pMobIndex->spec_fun ) );
               }
          }
     }

     fprintf ( fp, "S\n\n\n\n" );

     fclose ( fp );
     return;
}

void db_load_area_specials ( char *filename, AREA_DATA *pArea )
{
     SNP ( strArea, "%s%s.specials", SPEC_DIR, filename );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ;; )
     {
          MOB_INDEX_DATA     *pMobIndex;
          char                letter;

          switch ( letter = fread_letter ( fpArea ) )
          {
          default:
               bugf ( "Load_specials: letter '%c' not *MS.", letter );
               exit ( 1 );
          case 'S':
               fclose ( fpArea ); /* Would've been nice not to forget this - Lotherius */
               return;
          case '*':
               break;
          case 'M':
               pMobIndex 		= get_mob_index ( fread_number ( fpArea ) );
               pMobIndex->spec_fun 	= spec_lookup ( fread_word ( fpArea ) );
               if ( pMobIndex->spec_fun == 0 )
               {
                    bugf ( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                    exit ( 1 );
               }
               break;
          }
          fread_to_eol ( fpArea );
     }

     fclose ( fpArea ); /* Shouldn't ever be reached if there's an S at the end... */
     return;
}

void db_save_area_progs ( char *filename, AREA_DATA *pArea )
{
     PROG_CODE 		*pProg;
     int 		 i;
     char                pfilename[MSL];
     FILE                *fp;

     SNP ( pfilename, "%s%s.progs", PROG_DIR, filename );

     if ( !( fp = fopen ( pfilename, "w" ) ) )
     {
          bugf ( "Open_area: fopen (while saving an area)");
          perror ( pfilename );
     }

     fprintf(fp, "#MOBPROGS\n");
     for( i = pArea->lvnum; i <= pArea->uvnum; i++ )
     {
          if ( (pProg = get_prog_index(i, PRG_MPROG) ) != NULL)
          {
               fprintf(fp, "#%d\n", i);
               fprintf(fp, "%s~\n", fix_string(pProg->code));
          }
     }
     fprintf(fp,"#0\n\n");

     fprintf(fp, "#OBJPROGS\n");
     for( i = pArea->lvnum; i <= pArea->uvnum; i++ )
     {
          if ( (pProg = get_prog_index(i, PRG_OPROG) ) != NULL)
          {
               fprintf(fp, "#%d\n", i);
               fprintf(fp, "%s~\n", fix_string(pProg->code));
          }
     }
     fprintf(fp,"#0\n\n");

     fprintf(fp, "#ROOMPROGS\n");
     for( i = pArea->lvnum; i <= pArea->uvnum; i++ )
     {
          if ( (pProg = get_prog_index(i,PRG_RPROG) ) != NULL)
          {
               fprintf(fp, "#%d\n", i);
               fprintf(fp, "%s~\n", fix_string(pProg->code));
          }
     }
     fprintf(fp,"#0\n\n");

     fclose ( fp );
     return;
}

void db_load_area_progs ( char *filename, AREA_DATA *pArea )
{
     PROG_CODE  *pProg;
     char	*word;
     int	 ptype = PRG_MPROG;

     SNP ( strArea, "%s%s.progs", PROG_DIR, filename );
     
     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "No %s found for %s", strArea, pArea->name );
          return;
     }

     for ( ; ; )
     {
          bool oldval;
          bool oldival;
          int vnum;
          char letter;

          letter                  = fread_letter( fpArea );

          if ( letter != '#' )
          {
               bugf( "Load_mobprogs: # not found." );
               exit( 1 );
          }
          
          word = fread_word ( fpArea );
          
          if ( !is_number( word ) )
          {
               if ( !str_cmp ( word, "MOBPROGS" ) )          ptype = PRG_MPROG;
               else if ( !str_cmp ( word, "OBJPROGS" ) )     ptype = PRG_OPROG;
               else if ( !str_cmp ( word, "ROOMPROGS" ) )    ptype = PRG_RPROG; /* MUST BE LAST */
               else
               {
                    bugf ( "Invalid word %s in %s.", word, strArea );
                    exit ( 1 );
               }
               continue;
          }

          vnum = atol ( word );

          /* This depends on roomprogs being loaded last!!! */
          if ( vnum == 0 )
          {
               if ( ptype == PRG_RPROG )	/* Quit loading progs if all are done */
                    break;
               else				/* Get next program type if all are not done */
                    continue;
          }
          
          oldval = fBootDb;
          oldival = fImportDb;
          
          fBootDb = FALSE;
          fImportDb = FALSE;
          if ( get_prog_index( vnum, ptype ) != NULL )
          {
               bugf( "load_area_progs: vnum %d duplicated.", vnum );
               exit( 1 );
          }
          fImportDb = oldival;
          fBootDb = oldval;
          
          pProg                = alloc_perm( sizeof(*pProg), "pMprog:load_area_progs" );
          pProg->vnum          = vnum;
          pProg->code          = fread_string( fpArea );
          
          if ( ptype == PRG_MPROG )
          {
               if ( mprog_list == NULL )
                    mprog_list = pProg;
               else
               {
                    pProg->next  = mprog_list;
                    mprog_list   = pProg;
               }
               top_mprog_index++;
          }
          else if ( ptype == PRG_OPROG )
          {
               if ( oprog_list == NULL )
                    oprog_list = pProg;
               else
               {
                    pProg->next  = oprog_list;
                    oprog_list   = pProg;
               }
               top_oprog_index++;
          }
          else if ( ptype == PRG_RPROG )
          {
               if ( rprog_list == NULL )
                    rprog_list = pProg;
               else
               {
                    pProg->next  = rprog_list;
                    rprog_list   = pProg;
               }
               top_rprog_index++;               
          }
          
          else
          {
               bugf ( "Unknown ptype %d in %s????", ptype, strArea );
               exit ( 1 );
          }
     }

     fclose ( fpArea );
     return;
}


