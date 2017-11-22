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

#ifndef _DBMERC_H
# define _DBMERC_H   1

/* vals from db.c */
extern bool 		fBootDb;
extern bool 		fImportDb;
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA 	* mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA 	* obj_index_hash          [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed;
extern int              top_mprog_index;
extern int              top_oprog_index; 
extern int              top_rprog_index;
extern FILE		*fpArea;
extern char             strArea[MAX_INPUT_LENGTH];

/* func from db.c */
extern void assign_area_vnum 	args ( ( int vnum ) );
extern void new_reset 	   	args ( ( ROOM_INDEX_DATA * pR, RESET_DATA * pReset ) );
extern void fix_exits      	args ( ( void ) );
extern void fix_mobprogs   	args ( ( void ) );
extern void fix_objprogs   	args ( ( void ) );
extern void fix_roomprogs  	args ( ( void ) );
extern void init_racial_affects args ( ( void ) );

/* vals from db2.c */
extern int	social_count;

/* func from db2.c */
void convert_mobile( MOB_INDEX_DATA *pMobIndex );
void convert_objects( void );                    
void convert_object( OBJ_INDEX_DATA *pObjIndex );

/* func from db_area.c */
extern void db_load_area	args ( ( char *filename ) );


/* magic.h */
DECLARE_SPELL_FUN ( spell_null );

#endif // _DBMERC_H
