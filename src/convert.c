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
#include "convert.h"

/*
 * Flag conversions in this file carefullly copy individual flags
 * rather than just changing the different ones. This is in case
 * a flag has been set on a file that is undefined, we don't copy
 * it by mistake.
 */
struct rom_rooms_a
{
     int	romflag;
     int	sunderflag;
};


const struct rom_rooms_a roma_tablea [] =
{
     { ROMA_ROOM_DARK, 		ROOM_DARK 		},
     { ROMA_ROOM_NO_MOB,	ROOM_NO_MOB		},
     { ROMA_ROOM_INDOORS,	ROOM_INDOORS		},
     { ROMA_ROOM_PRIVATE,	ROOM_PRIVATE		},
     { ROMA_ROOM_SAFE,		ROOM_SAFE		},
     { ROMA_ROOM_SOLITARY,	ROOM_SOLITARY		},
     { ROMA_ROOM_PET_SHOP,	ROOM_PET_SHOP		},
     { ROMA_ROOM_NO_RECALL,	ROOM_NO_RECALL		},
     { ROMA_ROOM_IMP_ONLY,	ROOM_IMP_ONLY		},
     { ROMA_ROOM_GODS_ONLY,	ROOM_GODS_ONLY		},
     { ROMA_ROOM_HEROES_ONLY,	ROOM_HEROES_ONLY	},
     { ROMA_ROOM_NEWBIES_ONLY,	ROOM_NEWBIES_ONLY	},
     { ROMA_ROOM_LAW,		ROOM_LAW		},
     { 0,			0			}
};

/* Converts a ROM 2.3 room to Sunder 2.0 */
void cvt_rom_room_a ( ROOM_INDEX_DATA *room )
{
     int i = 0;
     int nflags = 0;
     int oflags = 0;

     /* Fix room flags */     
     oflags = room->room_flags;     

     while ( 1 )
     {
          if ( roma_tablea[i].sunderflag == 0 && roma_tablea[i].romflag == 0 )
               break;
          
          if ( IS_SET ( oflags, roma_tablea[i].romflag ) )
          {
               SET_BIT ( nflags, roma_tablea[i].sunderflag );
               REMOVE_BIT ( oflags, roma_tablea[i].romflag );
          }
          i++;
     }
     
     if ( oflags > 0 )
     {
          char buf[MSL];
          
          SNP ( buf, "   Flags (%d) removed for no match.", oflags );
          if ( IS_NULLSTR ( room->notes ) )
               strcpy ( room->notes, buf );
          else
               strcat ( room->notes, buf );
     }
     
     room->room_flags = nflags;
     /* Check Sector */
     if ( room->sector_type == ROMA_SECT_UNUSED || room->sector_type == ROMA_SECT_MAX )
          room->sector_type = SECT_MAX;     
     return;
}

