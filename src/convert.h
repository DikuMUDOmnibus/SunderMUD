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

#ifndef _CONVERT_H
# define _CONVERT_H   1

/*
 * This file contains values needed for converting various
 * areafile formats to the Sunder Format
 */

/* Prototypes */
/*------------------
 * Rom 2.3 Functions
 *----------------*/
extern void cvt_rom_room_a 	args ( ( ROOM_INDEX_DATA *room ) );

/*-------------------------
 * Rom 2.3 Values ( ROMA_ )
 *-----------------------*/

/* Room flags for Rom 2.3 */
#define ROMA_ROOM_DARK               (A)
#define ROMA_ROOM_NO_MOB             (C)
#define ROMA_ROOM_INDOORS            (D) 
#define ROMA_ROOM_PRIVATE            (J)
#define ROMA_ROOM_SAFE               (K)
#define ROMA_ROOM_SOLITARY           (L)
#define ROMA_ROOM_PET_SHOP           (M)
#define ROMA_ROOM_NO_RECALL          (N)
#define ROMA_ROOM_IMP_ONLY           (O)
#define ROMA_ROOM_GODS_ONLY          (P)
#define ROMA_ROOM_HEROES_ONLY        (Q)
#define ROMA_ROOM_NEWBIES_ONLY       (R)
#define ROMA_ROOM_LAW                (S)


#define ROMA_SECT_UNUSED                   8
#define ROMA_SECT_MAX                     11

#endif // _CONVERT_H
