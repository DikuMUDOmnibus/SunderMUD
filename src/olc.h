/*
 *  The unique portions of SunderMud code as well as the integration efforts
 *  for code from other sources is based on the efforts of:
 *
 *  Lotherius (elfren@aros.net)
 *
 *  This code can only be used under the terms of the DikuMud, Merc,
 *  and ROM licenses. The same requirements apply to the changes that
 *  have been made.
 *
 * All other copyrights remain in place and in force.
*/


/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */

#ifndef _OLC_H
#define _OLC_H   1

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0]\n\r" \
		"     SunderMud OLC 2.0\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r"    \
		"     With Contributions from Ivan Toledo (itoledo@ctcreuna.cl)\n\r" \
                "     SunderMud Additions & Fixes: Lotherius (elfren@blkbox.com)\n\r"
#define DATE	"     (Apr. 7,  1995 - ROM mod, Apr 16, 1995)\n\r" \
		"     (Jan. ?,  1997 - Ported to Sunder 1.0)\n\r" \
		"     (Aug. 14, 2001 - Updated for MProgs in Sunder 2)\n\r" \
                "     (July 4,  2002 - Updated for OProgs & RProgs)\n\r"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"



/*
 * New typedefs.
 */
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun

DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );

/*
 * Connected states for editor.
 */
#define ED_AREA 	1
#define ED_ROOM 	2
#define ED_OBJECT 	3
#define ED_MOBILE 	4
#define ED_HELP 	5
#define ED_MPCODE 	6
#define ED_OPCODE       7
#define ED_RPCODE       8

/*
 * Interpreter Prototypes
 */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );
void    redit           args( ( CHAR_DATA *ch, char *argument ) );
void    medit           args( ( CHAR_DATA *ch, char *argument ) );
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );
void	mpedit		args( ( CHAR_DATA *ch, char *argument ) ); /* MProgs */
void    opedit          args( ( CHAR_DATA *ch, char *argument ) );
void    rpedit          args( ( CHAR_DATA *ch, char *argument ) );
void    hedit           args( ( CHAR_DATA *ch, char *argument ) );

/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */



/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    char * const	name;
    OLC_FUN *		olc_fun;
};



/*
 * Structure for an OLC editor startup command.
 */
struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area	args ( ( int vnum ) );
AREA_DATA *get_area_data	args ( ( int vnum ) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );



/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type 	hedit_table[];
extern const struct olc_cmd_type	mpedit_table[]; /* MProgs */
extern const struct olc_cmd_type        opedit_table[];
extern const struct olc_cmd_type        rpedit_table[];


/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_hedit        );
DECLARE_DO_FUN( do_mpedit       );
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_rpedit       );

/*
 * General Functions
 */
bool show_commands	args ( ( CHAR_DATA *ch, char *argument ) );
bool show_help		args ( ( CHAR_DATA *ch, char *argument ) );
bool edit_done		args ( ( CHAR_DATA *ch ) );
bool show_version	args ( ( CHAR_DATA *ch, char *argument ) );
void show_affect_help   args ( ( CHAR_DATA *ch ) );
void show_detect_help   args ( ( CHAR_DATA *ch ) );
void show_protect_help  args ( ( CHAR_DATA *ch ) );
char *fix_string        args ( ( const char *str ) );
void save_area_list 	args ( ( void ) );

/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name		);
DECLARE_OLC_FUN( aedit_credits          );
DECLARE_OLC_FUN( aedit_music            );
DECLARE_OLC_FUN( aedit_file		);
DECLARE_OLC_FUN( aedit_flags            );
DECLARE_OLC_FUN( aedit_age		);
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum		);
DECLARE_OLC_FUN( aedit_levels           );
DECLARE_OLC_FUN( aedit_zone		);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);



/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_used		);
DECLARE_OLC_FUN( redit_unused		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_flags            ); /* added elfren */
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_sector           ); /* added elfren */
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_west		);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_down		);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_comment          );
DECLARE_OLC_FUN( redit_addrprog         );
DECLARE_OLC_FUN( redit_delrprog         );
DECLARE_OLC_FUN( rpedit_create          );
DECLARE_OLC_FUN( rpedit_code            );
DECLARE_OLC_FUN( rpedit_show            );
DECLARE_OLC_FUN( rpedit_list            );

/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_repop	);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_used		);
DECLARE_OLC_FUN( oedit_unused		);
DECLARE_OLC_FUN( oedit_default          );
DECLARE_OLC_FUN( oedit_name		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_wears		);
DECLARE_OLC_FUN( oedit_vflags		);
DECLARE_OLC_FUN( oedit_owears		);
DECLARE_OLC_FUN( oedit_removes		);
DECLARE_OLC_FUN( oedit_oremoves		);
DECLARE_OLC_FUN( oedit_uses		);
DECLARE_OLC_FUN( oedit_ouses		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_addapply	);
DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_adddetect	);
DECLARE_OLC_FUN( oedit_addprotect	);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);  /* ROM */
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_ed		);

DECLARE_OLC_FUN( oedit_extra            );  /* ROM */
DECLARE_OLC_FUN( oedit_wear             );  /* ROM */
DECLARE_OLC_FUN( oedit_type             );  /* ROM */
DECLARE_OLC_FUN( oedit_affect           );  /* ROM */
DECLARE_OLC_FUN( oedit_material		);  /* ROM */
DECLARE_OLC_FUN( oedit_level            );  /* ROM */
DECLARE_OLC_FUN( oedit_condition        );  /* ROM */
DECLARE_OLC_FUN( oedit_comment          );
DECLARE_OLC_FUN( oedit_addoprog         );
DECLARE_OLC_FUN( oedit_deloprog         );
DECLARE_OLC_FUN( opedit_create          );
DECLARE_OLC_FUN( opedit_code            );
DECLARE_OLC_FUN( opedit_show            );
DECLARE_OLC_FUN( opedit_list            );


/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_default 	        );
DECLARE_OLC_FUN( medit_used		);
DECLARE_OLC_FUN( medit_unused		);
DECLARE_OLC_FUN( medit_name		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec		);

DECLARE_OLC_FUN( medit_sex		);  /* ROM */
DECLARE_OLC_FUN( medit_act		);  /* ROM */
DECLARE_OLC_FUN( medit_affect		);  /* ROM */
DECLARE_OLC_FUN( medit_detect		);
DECLARE_OLC_FUN( medit_protect		);
DECLARE_OLC_FUN( medit_form		);  /* ROM */
DECLARE_OLC_FUN( medit_part		);  /* ROM */
DECLARE_OLC_FUN( medit_imm		);  /* ROM */
DECLARE_OLC_FUN( medit_res		);  /* ROM */
DECLARE_OLC_FUN( medit_vuln		);  /* ROM */
DECLARE_OLC_FUN( medit_material		);  /* ROM */
DECLARE_OLC_FUN( medit_off		);  /* ROM */
DECLARE_OLC_FUN( medit_size		);  /* ROM */
DECLARE_OLC_FUN( medit_hitdice		);  /* ROM */
DECLARE_OLC_FUN( medit_manadice		);  /* ROM */
DECLARE_OLC_FUN( medit_damdice		);  /* ROM */
DECLARE_OLC_FUN( medit_race		);  /* ROM */
DECLARE_OLC_FUN( medit_position		);  /* ROM */
DECLARE_OLC_FUN( medit_gold		);  /* ROM */
DECLARE_OLC_FUN( medit_hitroll		);  /* ROM */
DECLARE_OLC_FUN( medit_attack		);  /* Zeran */
DECLARE_OLC_FUN( medit_teach		);  /* Lotherius - Sunder */
DECLARE_OLC_FUN( medit_comment          );

/* Mobprog editor */
DECLARE_OLC_FUN( mpedit_create		); /* MProgs */
DECLARE_OLC_FUN( mpedit_code		);
DECLARE_OLC_FUN( mpedit_show		);
DECLARE_OLC_FUN( mpedit_list		);
DECLARE_OLC_FUN( medit_addmprog		);  /* ROM */
DECLARE_OLC_FUN( medit_delmprog		);  /* ROM */


/*
 * Macros
 */

/*
#define IS_SWITCHED( ch )       ( ch->desc->original )
#define IS_BUILDER(ch, Area)	( ( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	    \
				|| strstr( Area->builders, "All" ) )	    \
				&& !IS_SWITCHED( ch ) )
*/

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_MPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_OPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_RPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )


/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args ( ( void ) );
void		free_reset_data		args ( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args ( ( void ) );
void		free_area		args ( ( AREA_DATA *pArea ) );
EXIT_DATA	*new_exit		args ( ( void ) );
void		free_exit		args ( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args ( ( void ) );
void		free_extra_descr	args ( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args ( ( void ) );
void		free_room_index		args ( ( ROOM_INDEX_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args ( ( void ) );
void		free_affect		args ( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args ( ( void ) );
void		free_shop		args ( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args ( ( void ) );
void		free_obj_index		args ( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args ( ( void ) );
void		free_mob_index		args ( ( MOB_INDEX_DATA *pMob ) );
HELP_DATA    *new_help       args ( (void) );
void free_help args ( ( HELP_DATA *pHelp ) ); 
extern HELP_DATA     *   help_last;

#undef ED 


char *		prog_type_to_name	args ( ( int type ) );
PROG_LIST      *new_mprog               args ( ( void ) );
void            free_mprog              args ( ( PROG_LIST *mp ) );
PROG_CODE	*new_mpcode		args ( (void) );
void		free_mpcode		args ( ( PROG_CODE *pMcode));
PROG_LIST       *new_oprog              args ( ( void ) );
void            free_oprog              args ( ( PROG_LIST *op ) );
PROG_LIST       *new_rprog              args ( ( void ) );
void            free_rprog              args ( ( PROG_LIST *rp ) );
PROG_CODE       *new_opcode             args ( ( void ) );
void            free_opcode             args ( ( PROG_CODE *pOcode ) );
PROG_CODE       *new_rpcode             args ( ( void ) );
void            free_rpcode             args ( ( PROG_CODE *pRcode ) );


#endif // _OLC_H
