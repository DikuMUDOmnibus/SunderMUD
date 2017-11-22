/***************************************************************************
 *  File: olc_act.c                                                        *
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
#include "olc.h"

char * prog_type_to_name ( int type );

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

struct vnum_level_hash_type
{
     int                 vnum;
     struct vnum_level_hash_type *next;
};

typedef struct vnum_level_hash_type vlh_type;

struct olc_help_type
{
     char               *command;
     const void         *structure;
     char               *desc;
};

bool show_version ( CHAR_DATA * ch, char *argument )
{
     send_to_char ( VERSION, ch );
     send_to_char ( "\n\r", ch );
     send_to_char ( AUTHOR, ch );
     send_to_char ( "\n\r", ch );
     send_to_char ( DATE, ch );
     send_to_char ( "\n\r", ch );
     send_to_char ( CREDITS, ch );
     send_to_char ( "\n\r", ch );

     return FALSE;
}

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
     { "area", 		area_flags, 	"Area attributes."		},
     { "room", 		room_flags, 	"Room attributes."		},
     { "sector", 	sector_flags, 	"Sector types, terrain."	},
     { "exit", 		exit_flags, 	"Exit types."			},
     { "type", 		type_flags, 	"Types of objects."		},
     { "extra", 	extra_flags, 	"Object attributes."		},
     { "wear", 		wear_flags, 	"Where to wear object."		},
     { "spec", 		spec_table, 	"Available special programs."	},
     { "sex", 		sex_flags, 	"Sexes."			},
     { "act", 		act_flags, 	"Mobile attributes."		},
     { "wear-loc", 	wear_loc_flags, "Where mobile wears object."	},
     { "spells", 	skill_table, 	"Names of current spells."	},
     { "weapon", 	weapon_flags, 	"Type of weapon."		},
     { "container", 	container_flags,"Container status."		},
     { "liquid", 	liquid_flags, 	"Types of liquids."		},
     { "ac", 		ac_type, 	"Ac for different attacks."	},
     { "form", 		form_flags, 	"Mobile body form."		},
     { "part", 		part_flags, 	"Mobile body parts."		},
     { "imm", 		imm_flags, 	"Mobile immunity."		},
     { "res", 		res_flags, 	"Mobile resistance."		},
     { "vuln", 		vuln_flags, 	"Mobile vlnerability."		},
     { "off", 		off_flags, 	"Mobile offensive behaviour."	},
     { "size", 		size_flags, 	"Mobile size."			},
     { "position", 	position_flags, "Mobile positions."		},
     { "material", 	material_type, 	"Material mob/obj is made from."},
     { "wclass", 	weapon_class, 	"Weapon class."			},
     { "wtype", 	weapon_type, 	"Special weapon type."		},
     { "apply", 	apply_flags, 	"Item stat apply types."	},
     { "damtype", 	attack_type, 	"Mobile damage type."		},
     { "mprog", 	mprog_flags, 	"MobProgram flags."		},
     { "oprog",         oprog_flags,    "ObjProgram flags."             },
     { "rprog",         rprog_flags,    "RoomProgram flags."            },
     { "vflags_armor",	vflags_armor,	"Armor vflags."			},
     {"", 0, ""}
};

/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds ( CHAR_DATA * ch, const struct flag_type *flag_table )
{
     char                buf[MAX_STRING_LENGTH];
     char                buf1[MAX_STRING_LENGTH];
     int                 flag;
     int                 col;

     buf1[0] = '\0';
     col = 0;
     for ( flag = 0; flag_table[flag].name[0] != '\0'; flag++ )
     {
          if ( flag_table[flag].settable )
          {
               SNP ( buf, "%-19.18s", flag_table[flag].name );
               SLCAT ( buf1, buf );
               if ( ++col % 4 == 0 )
                    SLCAT ( buf1, "\n\r" );
          }
     }

     if ( col % 4 != 0 )
          SLCAT ( buf1, "\n\r" );

     send_to_char ( buf1, ch );
     return;
}

/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds ( CHAR_DATA * ch, int tar )
{
     char                buf[MAX_STRING_LENGTH];
     char                buf1[MAX_STRING_LENGTH * 2];
     int                 sn;
     int                 col;

     buf1[0] = '\0';
     col = 0;
     for ( sn = 0; sn < MAX_SKILL; sn++ )
     {
          if ( !skill_table[sn].name )
               break;

          if ( !str_cmp ( skill_table[sn].name, "reserved" )
               || skill_table[sn].spell_fun == spell_null )
               continue;

          if ( tar == -1 || skill_table[sn].target == tar )
          {
               SNP ( buf, "%-19.18s", skill_table[sn].name );
               SLCAT ( buf1, buf );
               if ( ++col % 4 == 0 )
                    SLCAT ( buf1, "\n\r" );
          }
     }

     if ( col % 4 != 0 )
          SLCAT ( buf1, "\n\r" );

     send_to_char ( buf1, ch );
     return;
}

/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds ( CHAR_DATA * ch )
{
     char                buf[MAX_STRING_LENGTH];
     char                buf1[MAX_STRING_LENGTH];
     int                 spec;
     int                 col;

     buf1[0] = '\0';
     col = 0;
     send_to_char ( "Preceed special functions with 'spec_'\n\r\n\r", ch );
     for ( spec = 0; spec_table[spec].spec_name[0] != '\0'; spec++ )
     {
          SNP ( buf, "%-19.18s",
                    &spec_table[spec].spec_name[5] );
          SLCAT ( buf1, buf );
          if ( ++col % 4 == 0 )
               SLCAT ( buf1, "\n\r" );
     }

     if ( col % 4 != 0 )
          SLCAT ( buf1, "\n\r" );

     send_to_char ( buf1, ch );
     return;
}

/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help ( CHAR_DATA * ch, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     char                spell[MAX_INPUT_LENGTH];
     int                 cnt;

     argument = one_argument ( argument, arg );
     one_argument ( argument, spell );

    /*
     * Display syntax.
     */
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:  ? [command]\n\r\n\r", ch );
          send_to_char ( "[command]  [description]\n\r", ch );
          for ( cnt = 0; help_table[cnt].command[0] != '\0'; cnt++ )
          {
               form_to_char ( ch, "%-10.10s -%s\n\r",
                              capitalize ( help_table[cnt].command ),
                              help_table[cnt].desc );
          }
          return FALSE;
     }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
     for ( cnt = 0; help_table[cnt].command[0] != '\0'; cnt++ )
     {
          if ( arg[0] == help_table[cnt].command[0]
               && !str_prefix ( arg, help_table[cnt].command ) )
          {
               if ( help_table[cnt].structure == spec_table )
               {
                    show_spec_cmds ( ch );
                    return FALSE;
               }
               else if ( help_table[cnt].structure == skill_table )
               {

                    if ( spell[0] == '\0' )
                    {
                         send_to_char ( "Syntax:  ? spells "
                                        "[ignore/attack/defend/self/object/all]\n\r", ch );
                         return FALSE;
                    }

                    if ( !str_prefix ( spell, "all" ) )
                         show_skill_cmds ( ch, -1 );
                    else if ( !str_prefix ( spell, "ignore" ) )
                         show_skill_cmds ( ch, TAR_IGNORE );
                    else if ( !str_prefix ( spell, "attack" ) )
                         show_skill_cmds ( ch, TAR_CHAR_OFFENSIVE );
                    else if ( !str_prefix ( spell, "defend" ) )
                         show_skill_cmds ( ch, TAR_CHAR_DEFENSIVE );
                    else if ( !str_prefix ( spell, "self" ) )
                         show_skill_cmds ( ch, TAR_CHAR_SELF );
                    else if ( !str_prefix ( spell, "object" ) )
                         show_skill_cmds ( ch, TAR_OBJ_INV );
                    else
                         send_to_char ( "Syntax:  ? spell "
                                        "[ignore/attack/defend/self/object/all]\n\r", ch );

                    return FALSE;
               }
               else
               {
                    show_flag_cmds ( ch, help_table[cnt].structure );
                    return FALSE;
               }
          }
     }

     show_help ( ch, "" );
     return FALSE;
}

REDIT ( redit_mlist )
{
     MOB_INDEX_DATA     *pMobIndex;
     AREA_DATA          *pArea;
     char                buf[MAX_STRING_LENGTH];
     char                buf1[MAX_STRING_LENGTH * 2];
     char                arg[MAX_INPUT_LENGTH];
     bool                fAll, found;
     int                 vnum;
     int                 col = 0;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:  mlist <all/name>\n\r", ch );
          return FALSE;
     }

     pArea = ch->in_room->area;
     buf1[0] = '\0';
     fAll = !str_cmp ( arg, "all" );
     found = FALSE;

     for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
          if ( ( pMobIndex = get_mob_index ( vnum ) ) )
          {
               if ( fAll || is_name ( arg, pMobIndex->player_name ) )
               {
                    found = TRUE;
                    SNP ( buf, "[%5d] %-17.16s",
                              pMobIndex->vnum,
                              capitalize ( pMobIndex->short_descr ) );
                    SLCAT ( buf1, buf );
                    if ( ++col % 3 == 0 )
                         SLCAT ( buf1, "\n\r" );
               }
          }
     }

     if ( !found )
     {
          send_to_char ( "Mobile(s) not found in this area.\n\r", ch );
          return FALSE;
     }

     if ( col % 3 != 0 )
          SLCAT ( buf1, "\n\r" );

     send_to_char ( buf1, ch );
     return FALSE;
}

REDIT ( redit_olist )
{
     OBJ_INDEX_DATA     *pObjIndex;
     AREA_DATA          *pArea;
     char                buf[MAX_STRING_LENGTH];
     char                buf1[MAX_STRING_LENGTH * 2];
     char                arg[MAX_INPUT_LENGTH];
     bool                fAll, found;
     int                 vnum;
     int                 col = 0;

     one_argument ( argument, arg );
     if ( arg[0] == '\0' )
     {
          send_to_char ( "Syntax:  olist <all/name/item_type>\n\r", ch );
          return FALSE;
     }

     pArea = ch->in_room->area;
     buf1[0] = '\0';
     fAll = !str_cmp ( arg, "all" );
     found = FALSE;

     for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
     {
          if ( ( pObjIndex = get_obj_index ( vnum ) ) )
          {
               if ( fAll || is_name ( arg, pObjIndex->name )
                    || flag_value ( type_flags,
                                    arg ) == pObjIndex->item_type )
               {
                    found = TRUE;
                    SNP ( buf, "[%5d] %-17.16s",
                              pObjIndex->vnum,
                              capitalize ( pObjIndex->short_descr ) );
                    SLCAT ( buf1, buf );
                    if ( ++col % 3 == 0 )
                         SLCAT ( buf1, "\n\r" );
               }
          }
     }

     if ( !found )
     {
          send_to_char ( "Object(s) not found in this area.\n\r", ch );
          return FALSE;
     }

     if ( col % 3 != 0 )
          SLCAT ( buf1, "\n\r" );

     send_to_char ( buf1, ch );
     return FALSE;
}

REDIT ( redit_mshow )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  mshow <vnum>\n\r", ch );
          return FALSE;
     }

     if ( is_number ( argument ) )
     {
          value = atoi ( argument );
          if ( !( pMob = get_mob_index ( value ) ) )
          {
               send_to_char ( "REdit:  That mobile does not exist.\n\r", ch );
               return FALSE;
          }

          ch->desc->pEdit = ( void * ) pMob;
     }

     medit_show ( ch, argument );
     ch->desc->pEdit = ( void * ) ch->in_room;
     return FALSE;
}

REDIT ( redit_oshow )
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  oshow <vnum>\n\r", ch );
          return FALSE;
     }

     if ( is_number ( argument ) )
     {
          value = atoi ( argument );
          if ( !( pObj = get_obj_index ( value ) ) )
          {
               send_to_char ( "REdit:  That object does not exist.\n\r", ch );
               return FALSE;
          }

          ch->desc->pEdit = ( void * ) pObj;
     }

     oedit_show ( ch, argument );
     ch->desc->pEdit = ( void * ) ch->in_room;
     return FALSE;
}

/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range ( int lower, int upper )
{
     AREA_DATA          *pArea;
     int                 cnt = 0;

     for ( pArea = area_first; pArea; pArea = pArea->next )
     {
	/*
	 * lower < area < upper
	 */
          if ( ( lower <= pArea->lvnum && pArea->lvnum <= upper ) || ( lower <= pArea->uvnum && pArea->uvnum <= upper ) )
               ++cnt;

          if ( cnt > 1 )
               return FALSE;
     }
     return TRUE;
}

AREA_DATA          *get_vnum_area ( int vnum )
{
     AREA_DATA          *pArea;

     for ( pArea = area_first; pArea; pArea = pArea->next )
     {
          if ( vnum >= pArea->lvnum && vnum <= pArea->uvnum )
               return pArea;
     }

     return 0;
}

/*
 * Area Editor Functions.
 */
AEDIT ( aedit_show )
{
     AREA_DATA          *pArea;

     EDIT_AREA ( ch, pArea );
     
     form_to_char ( ch, "Name:     [{C%5d{w] %s\n\r", pArea->vnum, pArea->name );
     form_to_char ( ch, "File:     {C%s{w\n\r", pArea->filename );
     form_to_char ( ch, "Zone:     [{C%d{w] (-1 while building prevents showing on areas)\n\r", pArea->zone );
     form_to_char ( ch, "Vnums:    [{C%d{w-{C%d{w]\n\r", pArea->lvnum, pArea->uvnum );
     form_to_char ( ch, "Levels:   [{C%d{w-{C%d{w]\n\r", pArea->llev, pArea->hlev );
     form_to_char ( ch, "Age:      [%d]\n\r", pArea->age );
     form_to_char ( ch, "Players:  [%d]\n\r", pArea->nplayer );
     form_to_char ( ch, "Security: [{C%d{w]\n\r", pArea->security );
     form_to_char ( ch, "Builders: [{C%s{w]\n\r", pArea->builders );
     form_to_char ( ch, "Credits:  [{C%s{w]\n\r", pArea->credits );
     form_to_char ( ch, "Music:    [{C%s{w]\n\r", pArea->soundfile );
     form_to_char ( ch, "Flags:    [{C%s{w]\n\r", flag_string ( area_flags, pArea->area_flags ) );
     return FALSE;
}

AEDIT ( aedit_reset )
{
     AREA_DATA          *pArea;
     EDIT_AREA ( ch, pArea );
     reset_area ( pArea, TRUE );
     send_to_char ( "Area reset.\n\r", ch );
     return FALSE;
}

AEDIT ( aedit_create )
{
     AREA_DATA          *pArea;

     pArea = new_area (  );
     area_last->next = pArea;
     area_last = pArea;		/* Thanks, Walker. */
     ch->desc->pEdit = ( void * ) pArea;
     SET_BIT ( pArea->area_flags, AREA_ADDED );
     send_to_char ( "Area Created.\n\r", ch );
     return FALSE;
}

AEDIT ( aedit_name )
{
     AREA_DATA          *pArea;

     EDIT_AREA ( ch, pArea );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   name [$name]\n\r", ch );
          return FALSE;
     }

     free_string ( pArea->name );
     pArea->name = str_dup ( argument );

     send_to_char ( "Name set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_credits )
{
     AREA_DATA	*pArea;
     
     EDIT_AREA ( ch, pArea );
     
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   credits [credits]\n\r", ch );
          return FALSE;
     }
     
     free_string ( pArea->credits );
     pArea->credits = str_dup ( argument );
     
     send_to_char ( "Credits set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_music )
{
     AREA_DATA	*pArea;

     EDIT_AREA ( ch, pArea );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   music [file.mid]\n\r", ch);
          return FALSE;
     }

     free_string ( pArea->soundfile );
     pArea->soundfile = str_dup (argument );
     send_to_char ( "Music set. MIDI only. If this file is not on the webpage,\n\r", ch);
     send_to_char ( "then send a copy of it to the mud admin to be added for download.\n\r", ch);
     return TRUE;
}

AEDIT ( aedit_file )
{
     AREA_DATA          *pArea;
     char                file[MAX_STRING_LENGTH];
     int                 i, length;

     EDIT_AREA ( ch, pArea );
     one_argument ( argument, file );	/* Forces Lowercase */
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  filename [$file]\n\r", ch );
          return FALSE;
     }

     /*
      * Simple Syntax Check.
      */
     length = strlen ( argument );
     /* We're going to assume if you've got Win32 that you have a valid long filename support */
     /* This probably means the mud won't work well on win95 original release, but.. duh..    */
     if ( length > 20 )
     {
          send_to_char ( "No more than twenty characters allowed.\n\r", ch );
          return FALSE;
     }

    /*
     * Allow only letters and numbers.
     */
     for ( i = 0; i < length; i++ )
     {
          if ( !isalnum ( file[i] ) )
          {
               send_to_char ( "Only letters and numbers are valid.\n\r", ch );
               return FALSE;
          }
     }

     free_string ( pArea->filename );
     pArea->filename = str_dup ( file );

     send_to_char ( "Filename set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_flags )
{
     AREA_DATA          *pArea;
     char                flags[40];
     int                 value;

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, flags );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   flags <area flag>\n\r", ch );
          return FALSE;
     }

     if ( ( value = flag_value ( area_flags, flags ) ) != NO_FLAG )
     {
          TOGGLE_BIT ( pArea->area_flags, value );

          send_to_char ( "Flag toggled.\n\r", ch );
          return TRUE;
     }

     send_to_char ( "Unrecognized flag.\n\r", ch );
     return FALSE;

}

AEDIT ( aedit_age )
{
     AREA_DATA          *pArea;
     char                age[MAX_STRING_LENGTH];

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, age );

     if ( !is_number ( age ) || age[0] == '\0' )
     {
          send_to_char ( "Syntax:  age [#age]\n\r", ch );
          return FALSE;
     }

     pArea->age = atoi ( age );

     send_to_char ( "Age set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_zone )
{
     AREA_DATA          *pArea;
     char                zone_str[MAX_STRING_LENGTH];
     int                 zone;

     EDIT_AREA ( ch, pArea );

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  zone <number>\n\r", ch );
          return FALSE;
     }

     one_argument ( argument, zone_str );
     zone = atoi ( zone_str );

     if ( ( zone > MAX_ZONE || zone < 1 ) && ( zone != -1 ) )
     {
          form_to_char ( ch, "Valid zone range is 1 to %d.  Or -1 if no zone.\n\r", MAX_ZONE );
          return FALSE;
     }
     pArea->zone = zone;
     form_to_char ( ch, "Zone set to %d.\n\r", zone );
     return TRUE;
}

AEDIT ( aedit_security )
{
     AREA_DATA          *pArea;
     char                sec[MAX_STRING_LENGTH];
     int                 value;

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, sec );

     if ( !is_number ( sec ) || sec[0] == '\0' )
     {
          send_to_char ( "Syntax:  security [#level]\n\r", ch );
          return FALSE;
     }

     value = atoi ( sec );

     if ( value > ch->pcdata->security || value < 0 )
     {
          if ( ch->pcdata->security != 0 )
          {
               form_to_char ( ch, "Security is 0-%d.\n\r", ch->pcdata->security );
          }
          else
               send_to_char ( "Security is 0 only.\n\r", ch );
          return FALSE;
     }

     pArea->security = value;

     send_to_char ( "Security set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_builder )
{
     AREA_DATA          *pArea;
     char                name[MAX_STRING_LENGTH];
     char                buf[MAX_STRING_LENGTH];

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, name );

     if ( name[0] == '\0' )
     {
          send_to_char ( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
          send_to_char ( "Syntax:  builder All      -allows everyone\n\r", ch );
          return FALSE;
     }

     name[0] = UPPER ( name[0] );

     if ( strstr ( pArea->builders, name ) != '\0' )
     {
          pArea->builders = string_replace ( pArea->builders, name, "\0" );
          pArea->builders = string_unpad ( pArea->builders );

          if ( pArea->builders[0] == '\0' )
          {
               free_string ( pArea->builders );
               pArea->builders = str_dup ( "None" );
          }
          send_to_char ( "Builder removed.\n\r", ch );
          return TRUE;
     }
     else
     {
          buf[0] = '\0';
          if ( strstr ( pArea->builders, "None" ) != '\0' )
          {
               pArea->builders =
                    string_replace ( pArea->builders, "None", "\0" );
               pArea->builders = string_unpad ( pArea->builders );
          }

          if ( pArea->builders[0] != '\0' )
          {
               SLCAT ( buf, pArea->builders );
               SLCAT ( buf, " " );
          }
          SLCAT ( buf, name );
          free_string ( pArea->builders );
          pArea->builders = string_proper ( str_dup ( buf ) );

          send_to_char ( "Builder added.\n\r", ch );
          return TRUE;
     }

     return FALSE;
}

AEDIT ( aedit_levels )
{
     AREA_DATA  *pArea;
     char        lower[MSL];
     char        upper[MSL];
     int         ilower;
     int         iupper;
     
     EDIT_AREA ( ch, pArea );
     argument = one_argument ( argument, lower );
     one_argument ( argument, upper );
     if ( !is_number ( lower ) || lower[0] == '\0' 
          || !is_number ( upper ) || upper[0] == '\0' )
     {
          send_to_char ( "Syntax:  levels [#lower] [#higher]\n\r", ch );
          return FALSE;
     }
     if ( ( ilower = atoi ( lower ) ) > ( iupper = atoi ( upper ) ) )
     {
          send_to_char ( "AEdit:  Higher level must be higher then lower.\n\r", ch );
          return FALSE;
     }
     
     if ( ilower < 0 )
     {
          send_to_char ( "AEdit: Lower level cannot be less than 0.\n\r", ch );
          return FALSE;
     }
     
     if ( iupper > MAX_LEVEL )
     {
          send_to_char ( "AEdit: Higher level cannot be higher than max level.\n\r", ch );
          return FALSE;
     }
     pArea->llev = ilower;
     pArea->hlev = iupper;
     
     send_to_char ( "Level range set.\n\r", ch );
     return TRUE;        
}

AEDIT ( aedit_vnum )
{
     AREA_DATA          *pArea;
     char                lower[MAX_STRING_LENGTH];
     char                upper[MAX_STRING_LENGTH];
     int                 ilower;
     int                 iupper;

     EDIT_AREA ( ch, pArea );

     argument = one_argument ( argument, lower );
     one_argument ( argument, upper );

     if ( !is_number ( lower ) || lower[0] == '\0'
          || !is_number ( upper ) || upper[0] == '\0' )
     {
          send_to_char ( "Syntax:  vnum [#lower] [#upper]\n\r", ch );
          return FALSE;
     }

     if ( ( ilower = atoi ( lower ) ) > ( iupper =
                                          atoi ( upper ) ) )
     {
          send_to_char ( "AEdit:  Upper must be larger then lower.\n\r", ch );
          return FALSE;
     }

     if ( !check_range ( atoi ( lower ), atoi ( upper ) ) )
     {
          send_to_char ( "AEdit:  Range must include only this area.\n\r", ch );
          return FALSE;
     }

     if ( get_vnum_area ( ilower )
          && get_vnum_area ( ilower ) != pArea )
     {
          send_to_char ( "AEdit:  Lower vnum already assigned.\n\r", ch );
          return FALSE;
     }

     pArea->lvnum = ilower;
     send_to_char ( "Lower vnum set.\n\r", ch );

     if ( get_vnum_area ( iupper )
          && get_vnum_area ( iupper ) != pArea )
     {
          send_to_char ( "AEdit:  Upper vnum already assigned.\n\r", ch );
          return TRUE;		/* The lower value has been set. */
     }

     pArea->uvnum = iupper;
     send_to_char ( "Upper vnum set.\n\r", ch );

     return TRUE;
}

AEDIT ( aedit_lvnum )
{
     AREA_DATA          *pArea;
     char                lower[MAX_STRING_LENGTH];
     int                 ilower;
     int                 iupper;

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, lower );

     if ( !is_number ( lower ) || lower[0] == '\0' )
     {
          send_to_char ( "Syntax:  lvnum [#lower]\n\r", ch );
          return FALSE;
     }

     if ( ( ilower = atoi ( lower ) ) > ( iupper = pArea->uvnum ) )
     {
          send_to_char ( "AEdit:  Value must be less than the uvnum.\n\r", ch );
          return FALSE;
     }

     if ( !check_range ( ilower, iupper ) )
     {
          send_to_char ( "AEdit:  Range must include only this area.\n\r", ch );
          return FALSE;
     }

     if ( get_vnum_area ( ilower )
          && get_vnum_area ( ilower ) != pArea )
     {
          send_to_char ( "AEdit:  Lower vnum already assigned.\n\r", ch );
          return FALSE;
     }

     pArea->lvnum = ilower;
     send_to_char ( "Lower vnum set.\n\r", ch );
     return TRUE;
}

AEDIT ( aedit_uvnum )
{
     AREA_DATA          *pArea;
     char                upper[MAX_STRING_LENGTH];
     int                 ilower;
     int                 iupper;

     EDIT_AREA ( ch, pArea );

     one_argument ( argument, upper );

     if ( !is_number ( upper ) || upper[0] == '\0' )
     {
          send_to_char ( "Syntax:  uvnum [#upper]\n\r", ch );
          return FALSE;
     }

     if ( ( ilower = pArea->lvnum ) > ( iupper = atoi ( upper ) ) )
     {
          send_to_char
               ( "AEdit:  Upper must be larger then lower.\n\r", ch );
          return FALSE;
     }

     if ( !check_range ( ilower, iupper ) )
     {
          send_to_char ( "AEdit:  Range must include only this area.\n\r", ch );
          return FALSE;
     }

     if ( get_vnum_area ( iupper )
          && get_vnum_area ( iupper ) != pArea )
     {
          send_to_char ( "AEdit:  Upper vnum already assigned.\n\r", ch );
          return FALSE;
     }

     pArea->uvnum = iupper;
     send_to_char ( "Upper vnum set.\n\r", ch );

     return TRUE;
}

/*
 * Room Editor Functions.
 */
REDIT ( redit_unused )
{
     ROOM_INDEX_DATA    *pRoom;
     AREA_DATA          *pArea;
     ROOM_INDEX_DATA    *tmp;
     BUFFER		*buffer;
     int                 counter;
     
     buffer = buffer_new ( 1024 );

     EDIT_ROOM ( ch, pRoom );
     pArea = pRoom->area;
     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );
     bprintf ( buffer, "UNUSED ROOM VNUMS\n\r------ ---- -----\n\r", ch );
     counter = pArea->lvnum;
     for ( ; ( counter <= pArea->uvnum ); counter++ )
     {
          if ( ( tmp = get_room_index ( counter ) ) == NULL )
               bprintf ( buffer, "[%d]\n\r", counter );
     }
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return TRUE;
}

REDIT ( redit_used )
{          
     ROOM_INDEX_DATA    *pRoom;
     AREA_DATA          *pArea;
     ROOM_INDEX_DATA    *tmp;
     bool		haveargs = FALSE;
     int                counter;
     BUFFER            *buffer;
     
     buffer = buffer_new (2000);
     
     if ( argument[0] != '\0' )
          haveargs = TRUE;
          
     EDIT_ROOM ( ch, pRoom );
     pArea = pRoom->area;          
     
     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );
     bprintf ( buffer, "USED ROOM VNUMS\n\r---- ---- -----\n\r" );

     counter = pArea->lvnum;
     
     if ( !haveargs )
     {
          for ( ; ( counter <= pArea->uvnum ); counter++ )
          {
               if ( ( tmp = get_room_index ( counter ) ) != NULL )
                    bprintf ( buffer, "{W[{Y%d{W] {w%s{w\n\r", counter, ( ( tmp->name ) ? ( tmp->name ) : "" ) );
          }
     }
     else
     {
          bool sector = FALSE;
          bool flags = FALSE;
          
          if ( strstr (argument, "sector" ) != NULL )
               sector = TRUE;
          if ( strstr (argument, "flag" ) != NULL )
               flags = TRUE;
          
          for ( ; ( counter <= pArea->uvnum ); counter ++ )
          {
               if ( ( tmp = get_room_index ( counter ) ) != NULL )
               {
                    bprintf ( buffer, "{W[{Y%d{W] {w%-40s {w::  ", counter, ( ( tmp->name ) ? ( tmp->name ) : "" ) );
                    if ( sector )
                         bprintf ( buffer, "{W[{G%10s{W]   ", flag_string ( sector_flags, tmp->sector_type ) );
                    if ( flags )
                         bprintf ( buffer, "{W[{C%s{W]", flag_string ( room_flags, tmp->room_flags ) );
                    bprintf ( buffer, "\n\r" );
               }
          }
     }
     
     bprintf ( buffer, "{w\n\r" );
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     
     return TRUE;
}

REDIT ( redit_show )
{
     ROOM_INDEX_DATA    *pRoom;
     char                buf[MAX_STRING_LENGTH];
     char                buf1[2 * MAX_STRING_LENGTH];
     OBJ_DATA           *obj;
     CHAR_DATA          *rch;
     PROG_LIST          *list;
     int                 door;
     bool                fcnt;

     EDIT_ROOM ( ch, pRoom );

     buf[0] = '\0';
     buf1[0] = '\0';

     form_to_char ( ch, "{wName:       [{C%s{w]\n\rArea:       [{C%5d{w] %s\n\r",
                    pRoom->name, pRoom->area->vnum, pRoom->area->name );
     form_to_char ( ch, "Vnum:       [{C%5d{w]\n\rSector:     [{C%s{w]\n\r",
                    pRoom->vnum, flag_string ( sector_flags, pRoom->sector_type ) );
     form_to_char ( ch, "Room flags: [{C%s{w]\n\r", flag_string ( room_flags, pRoom->room_flags ) );

     if ( pRoom->extra_descr )
     {
          EXTRA_DESCR_DATA   *ed;

          SNP ( buf, "Desc Kwds:  [{C" );
          for ( ed = pRoom->extra_descr; ed; ed = ed->next )
          {
               SLCAT ( buf, ed->keyword );
               if ( ed->next )
                    SLCAT ( buf, " " );
          }
          SLCAT ( buf, "{w]\n\r" );
          send_to_char ( buf, ch );
     }

     SNP (buf , "Characters: [" );
     fcnt = FALSE;
     for ( rch = pRoom->people; rch; rch = rch->next_in_room )
     {
          one_argument ( rch->name, buf1 );
          SLCAT ( buf, "{C" );
          SLCAT ( buf, buf1 );
          SLCAT ( buf, "{w " );
          fcnt = TRUE;
     }
     if ( fcnt )
     {
          int                 end;
          end = strlen ( buf ) - 1;
          buf[end] = ']';
          SLCAT ( buf, "\n\r" );
     }
     else
          SLCAT ( buf, "none{w]\n\r" );
     send_to_char ( buf, ch );

     SNP ( buf1, "Objects:    [{C" );
     fcnt = FALSE;
     for ( obj = pRoom->contents; obj; obj = obj->next_content )
     {
          one_argument ( obj->name, buf );
          SLCAT ( buf1, buf );
          SLCAT ( buf1, " " );
          fcnt = TRUE;
     }
     if ( fcnt )
     {
          int                 end;

          end = strlen ( buf1 ) - 1;
          buf1[end] = ']';
          SLCAT ( buf1, "{w\n\r" );
     }
     else
          SLCAT ( buf1, "none{w]\n\r" );
     send_to_char ( buf1, ch );

     for ( door = 0; door < MAX_DIR; door++ )
     {
          EXIT_DATA          *pexit;

          if ( ( pexit = pRoom->exit[door] ) )
          {
               char                word[MAX_INPUT_LENGTH];
               char                reset_state[MAX_STRING_LENGTH];
               char               *state;
               int                 i, length;
               
               form_to_char ( ch, "{G-{Y%-5s {wto [{C%5d{w] Key: [{C%5d{w]", 
                              capitalize ( dir_name[door] ),
                              pexit->u1.to_room ? pexit->u1.to_room->vnum : 0, pexit->key );
               /*
                * Format up the exit info.
                * Capitalize all flags that are not part of the reset info.
                */
               SLCPY ( reset_state, flag_string ( exit_flags, pexit->rs_flags ) );
               state = flag_string ( exit_flags, pexit->exit_info );
               
               SNP ( buf1, " Exit flags: [{C" );
               for ( ;; )
               {
                    state = one_argument ( state, word );

                    if ( word[0] == '\0' )
                    {
                         int                 end;

                         end = strlen ( buf1 ) - 1;
                         buf1[end] = ']';
                         SLCAT ( buf1, "{w\n\r" );
                         break;
                    }

                    if ( str_infix ( word, reset_state ) )
                    {
                         length = strlen ( word );
                         for ( i = 0; i < length; i++ )
                              word[i] = UPPER ( word[i] );
                    }
                    SLCAT ( buf1, word );
                    SLCAT ( buf1, " " );
               }
               send_to_char ( buf1, ch );

               if ( pexit->keyword && pexit->keyword[0] != '\0' )
               {
                    form_to_char ( ch,  "Kwds: [{C%s{w]\n\r", pexit->keyword );
               }
               if ( pexit->description && pexit->description[0] != '\0' )
               {
                    form_to_char ( ch, "%s", pexit->description );
               }
          }
     }
     
     form_to_char ( ch,  "\n\rDescription:\n\r%s", pRoom->description );

     if ( pRoom->rprogs )
     {
          int cnt;

          form_to_char ( ch, "\n\rROOMPrograms for [{C%5d{w]:\n\r", pRoom->vnum);
          for (cnt=0, list=pRoom->rprogs; list; list=list->next)
          {
               if (cnt ==0)
               {
                    send_to_char ( " Number Vnum Trigger Phrase\n\r", ch );
                    send_to_char ( " ------ ---- ------- ------\n\r", ch );
               }

               form_to_char ( ch, "[{C%5d{w] {W%4d %7s %s{w\n\r", cnt,
                              list->vnum,prog_type_to_name(list->trig_type),
                              list->trig_phrase);
               cnt++;
          }
     }
     return FALSE;
}

/* Local function. */
bool change_exit ( CHAR_DATA * ch, char *argument, int door )
{
     ROOM_INDEX_DATA    *pRoom;
     char                command[MAX_INPUT_LENGTH];
     char                arg[MAX_INPUT_LENGTH];
     int                 value, value1;

     EDIT_ROOM ( ch, pRoom );

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
     if ( ( value =
            flag_value ( exit_flags, argument ) ) != NO_FLAG )
     {
          ROOM_INDEX_DATA    *pToRoom;
          sh_int              rev;	/* ROM OLC */

          if ( !pRoom->exit[door] )
          {
	    /* Zeran - removed this line, BAAAAD, don't know where exit leads:
	     * pRoom->exit[door] = new_exit();
	     */
               send_to_char ( "You must create an exit before toggling flags.\n\r", ch );
               return FALSE;
          }

          if ( ( value != EX_ISDOOR ) &&
               !IS_SET ( pRoom->exit[door]->rs_flags, EX_ISDOOR ) )
          {
               send_to_char ( "Redit: exit must be flagged as DOOR first.\n\r", ch );
               return FALSE;
          }
          /*
           * This room.
           */
          TOGGLE_BIT ( pRoom->exit[door]->rs_flags, value );
          /* Don't toggle exit_info because it can be changed by players. */
          pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;
          value1 = pRoom->exit[door]->rs_flags;
          /*
           * Connected room.
           */
          pToRoom = pRoom->exit[door]->u1.to_room;	/* ROM OLC */
          rev = rev_dir[door];

          /* Zeran - idiots didn't even check for an exit the other way first */
          if ( !pToRoom->exit[rev] )	/* no exit back, all done */
          {
               send_to_char ( "Exit flags on ONE-WAY exit toggled.\n\r", ch );
               return TRUE;
          }

          /* Zeran - just duplicate the door, duh */
          pToRoom->exit[rev]->rs_flags = value1;
          pToRoom->exit[rev]->exit_info = value1;
          
          /* Zeran - this stuff replaced by above two lines */
          /*	TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value); */
          /*	TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value); */
          
          send_to_char ( "Exit flags on TWO-way exit toggled.\n\r", ch );
          return TRUE;
     }

     /*
      * Now parse the arguments.
      */
     argument = one_argument ( argument, command );
     one_argument ( argument, arg );

     if ( command[0] == '\0' && argument[0] == '\0' )	/* Move command. */
     {
          move_char ( ch, door, TRUE );	/* ROM OLC */
          return FALSE;
     }

     if ( command[0] == '?' )
     {
          do_help ( ch, "EXIT" );
          return FALSE;
     }

     if ( !str_cmp ( command, "delete" ) )
     {
          ROOM_INDEX_DATA    *pToRoom;
          sh_int              rev;	/* ROM OLC */

          if ( !pRoom->exit[door] )
          {
               send_to_char ( "REdit:  Cannot delete a null exit.\n\r", ch );
               return FALSE;
          }

          /*
           * Remove ToRoom Exit.
           */
          rev = rev_dir[door];
          pToRoom = pRoom->exit[door]->u1.to_room;	/* ROM OLC */

          if ( pToRoom->exit[rev] )
          {
               free_exit ( pToRoom->exit[rev] );
               pToRoom->exit[rev] = NULL;
          }

          /*
           * Remove this exit.
           */
          free_exit ( pRoom->exit[door] );
          pRoom->exit[door] = NULL;

          send_to_char ( "Exit unlinked.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "link" ) )
     {
          EXIT_DATA          *pExit;

          if ( arg[0] == '\0' || !is_number ( arg ) )
          {
               send_to_char ( "Syntax:  [direction] link [vnum]\n\r", ch );
               return FALSE;
          }

          value = atoi ( arg );

          if ( !get_room_index ( value ) )
          {
               send_to_char ( "REdit:  Cannot link to non-existent room.\n\r", ch );
               return FALSE;
          }

          if ( !IS_BUILDER ( ch, get_room_index ( value )->area ) )
          {
               send_to_char ( "REdit:  Cannot link to that area.\n\r", ch );
               return FALSE;
          }

          if ( get_room_index ( value )->exit[rev_dir[door]] )
          {
               send_to_char ( "REdit:  Remote side's exit already exists.\n\r", ch );
               return FALSE;
          }

          if ( !pRoom->exit[door] )
          {
               pRoom->exit[door] = new_exit (  );
          }

          pRoom->exit[door]->u1.to_room = get_room_index ( value );	/* ROM OLC */
          pRoom = get_room_index ( value );
          door = rev_dir[door];
          pExit = new_exit (  );
          pExit->u1.to_room = ch->in_room;
          pExit->orig_door = door;
          pRoom->exit[door] = pExit;

          send_to_char ( "Two-way link established.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "dig" ) )
     {
          char                buf[MAX_STRING_LENGTH];

          if ( arg[0] == '\0' || !is_number ( arg ) )
          {
               send_to_char ( "Syntax: [direction] dig <vnum>\n\r", ch );
               return FALSE;
          }

          redit_create ( ch, arg );
          SNP ( buf, "link %s", arg );
          change_exit ( ch, buf, door );
          return TRUE;
     }

     if ( !str_cmp ( command, "room" ) )
     {
          if ( arg[0] == '\0' || !is_number ( arg ) )
          {
               send_to_char ( "Syntax:  [direction] room [vnum]\n\r", ch );
               return FALSE;
          }

          value = atoi ( arg );

          if ( !get_room_index ( value ) )
          {
               send_to_char ( "REdit:  Cannot link to non-existant room.\n\r", ch );
               return FALSE;
          }
          if ( !pRoom->exit[door] )
          {
               pRoom->exit[door] = new_exit (  );
          }

          pRoom->exit[door]->u1.to_room = get_room_index ( value );	/* ROM OLC */

          send_to_char ( "One-way link established.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "key" ) )
     {
          if ( arg[0] == '\0' || !is_number ( arg ) )
          {
               send_to_char ( "Syntax:  [direction] key [vnum]\n\r", ch );
               return FALSE;
          }

          if ( !pRoom->exit[door] )
          {
               pRoom->exit[door] = new_exit (  );
          }

          value = atoi ( arg );

          if ( !get_obj_index ( value ) )
          {
               send_to_char ( "REdit:  Item doesn't exist.\n\r", ch );
               return FALSE;
          }

          if ( get_obj_index ( atoi ( argument ) )->item_type !=
               ITEM_KEY )
          {
               send_to_char ( "REdit:  Key doesn't exist.\n\r", ch );
               return FALSE;
          }

          pRoom->exit[door]->key = value;

          send_to_char ( "Exit key set.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "name" ) )
     {
          if ( arg[0] == '\0' )
          {
               send_to_char ( "Syntax:  [direction] name [string]\n\r", ch );
               return FALSE;
          }

          if ( !pRoom->exit[door] )
          {
               pRoom->exit[door] = new_exit (  );
          }

          free_string ( pRoom->exit[door]->keyword );
          pRoom->exit[door]->keyword = str_dup ( arg );

          send_to_char ( "Exit name set.\n\r", ch );
          return TRUE;
     }

     if ( !str_prefix ( command, "description" ) )
     {
          if ( arg[0] == '\0' )
          {
               if ( !pRoom->exit[door] )
               {
                    pRoom->exit[door] = new_exit (  );
               }
               string_append ( ch, &pRoom->exit[door]->description );
               return TRUE;
          }

          send_to_char ( "Syntax:  [direction] desc\n\r", ch );
          return FALSE;
     }

     return FALSE;
}

REDIT ( redit_north )
{
     if ( change_exit ( ch, argument, DIR_NORTH ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_south )
{
     if ( change_exit ( ch, argument, DIR_SOUTH ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_east )
{
     if ( change_exit ( ch, argument, DIR_EAST ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_west )
{
     if ( change_exit ( ch, argument, DIR_WEST ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_up )
{
     if ( change_exit ( ch, argument, DIR_UP ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_down )
{
     if ( change_exit ( ch, argument, DIR_DOWN ) )
          return TRUE;

     return FALSE;
}

REDIT ( redit_ed )
{
     ROOM_INDEX_DATA    *pRoom;
     EXTRA_DESCR_DATA   *ed;
     char                command[MAX_INPUT_LENGTH];
     char                keyword[MAX_INPUT_LENGTH];

     EDIT_ROOM ( ch, pRoom );

     argument = one_argument ( argument, command );
     one_argument ( argument, keyword );

     if ( command[0] == '\0' || keyword[0] == '\0' )
     {
          send_to_char ( "Syntax:  ed add [keyword]\n\r", ch );
          send_to_char ( "         ed edit [keyword]\n\r", ch );
          send_to_char ( "         ed delete [keyword]\n\r", ch );
          send_to_char ( "         ed format [keyword]\n\r", ch );
          return FALSE;
     }

     if ( !str_cmp ( command, "add" ) )
     {
          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed add [keyword]\n\r", ch );
               return FALSE;
          }

          ed = new_extra_descr (  );
          ed->keyword = str_dup ( keyword );
          ed->description = str_dup ( "" );
          ed->next = pRoom->extra_descr;
          pRoom->extra_descr = ed;

          string_append ( ch, &ed->description );

          return TRUE;
     }

     if ( !str_cmp ( command, "edit" ) )
     {
          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed edit [keyword]\n\r", ch );
               return FALSE;
          }

          for ( ed = pRoom->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
          }

          if ( !ed )
          {
               send_to_char ( "REdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }

          string_append ( ch, &ed->description );

          return TRUE;
     }

     if ( !str_cmp ( command, "delete" ) )
     {
          EXTRA_DESCR_DATA   *ped = NULL;

          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed delete [keyword]\n\r", ch );
               return FALSE;
          }

          for ( ed = pRoom->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
               ped = ed;
          }

          if ( !ed )
          {
               send_to_char ( "REdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }

          if ( !ped )
               pRoom->extra_descr = ed->next;
          else
               ped->next = ed->next;

          free_extra_descr ( ed );

          send_to_char ( "Extra description deleted.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "format" ) )
     {
          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed format [keyword]\n\r",
                              ch );
               return FALSE;
          }

          for ( ed = pRoom->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
          }

          if ( !ed )
          {
               send_to_char ( "REdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }

          ed->description = format_string ( ed->description );

          send_to_char ( "Extra description formatted.\n\r", ch );
          return TRUE;
     }

     redit_ed ( ch, "" );
     return FALSE;
}

REDIT ( redit_create )
{
     AREA_DATA          *pArea;
     ROOM_INDEX_DATA    *pRoom;
     int                 value;
     int                 iHash;

     EDIT_ROOM ( ch, pRoom );

     value = atoi ( argument );

     if ( argument[0] == '\0' || value <= 0 )
     {
          send_to_char ( "Syntax:  create [vnum > 0]\n\r", ch );
          return FALSE;
     }

     pArea = get_vnum_area ( value );
     if ( !pArea )
     {
          send_to_char ( "REdit:  That vnum is not assigned an area.\n\r", ch );
          return FALSE;
     }

     if ( !IS_BUILDER ( ch, pArea ) )
     {
          send_to_char ( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
          return FALSE;
     }

     if ( get_room_index ( value ) )
     {
          send_to_char ( "REdit:  Room vnum already exists.\n\r", ch );
          return FALSE;
     }

     pRoom = new_room_index (  );
     pRoom->area = pArea;
     pRoom->vnum = value;

     if ( value > top_vnum_room )
          top_vnum_room = value;

     iHash = value % MAX_KEY_HASH;
     pRoom->next = room_index_hash[iHash];
     room_index_hash[iHash] = pRoom;
     ch->desc->pEdit = ( void * ) pRoom;

     send_to_char ( "Room created.\n\r", ch );
     return TRUE;
}

REDIT ( redit_name )
{
     ROOM_INDEX_DATA    *pRoom;

     EDIT_ROOM ( ch, pRoom );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  name [name]\n\r", ch );
          return FALSE;
     }

     free_string ( pRoom->name );
     pRoom->name = str_dup ( argument );

     send_to_char ( "Name set.\n\r", ch );
     return TRUE;
}

REDIT ( redit_comment )
{
     ROOM_INDEX_DATA *pRoom;

     EDIT_ROOM ( ch, pRoom );
     if ( argument[0] == '\0' )
     {
          string_append ( ch, &pRoom->notes );
          return TRUE;
     }
     send_to_char ( "Syntax: comments\n\r", ch );
     return FALSE;
}

REDIT ( redit_desc )
{
     ROOM_INDEX_DATA    *pRoom;
     ROOM_INDEX_DATA    *copyRoom;
     char                arg[MAX_INPUT_LENGTH];

     EDIT_ROOM ( ch, pRoom );

     if ( argument[0] == '\0' )
     {
          string_append ( ch, &pRoom->description );
          return TRUE;
     }

     argument = one_argument ( argument, arg );
     if ( !str_cmp ( arg, "copy" ) && ( argument[0] != '\0' ) )
     {
          one_argument ( argument, arg );
          if ( is_number ( arg ) )
          {
               copyRoom = get_room_index ( atoi ( arg ) );
               if ( copyRoom != NULL )
               {
                    if ( pRoom->description != NULL )
                         free_string ( pRoom->description );
                    pRoom->description = str_dup ( copyRoom->description );
                    return TRUE;
               }
          }
     }
     send_to_char ( "Syntax:  desc\n\r", ch );
     send_to_char ( "Syntax:  desc copy <vnum>\n\r", ch );
     return FALSE;
}

/* Hacked to watch for rent flag, and alloc memory if necessary - Lotherius  */
REDIT ( redit_flags )
{
     ROOM_INDEX_DATA    *pRoom;
     char                flags[40];
     int                 value, count, top;
     bool                set_all_on = FALSE;
     bool                set_all_off = FALSE;
     char               *remainder;

     EDIT_ROOM ( ch, pRoom );

     remainder = one_argument ( argument, flags );
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   flags <room flag>\n\r",         ch );
          send_to_char ( "Syntax:   flags @allon <room flag>\n\r",  ch );
          send_to_char ( "Syntax:   flags @alloff <room flag>\n\r", ch );
          return FALSE;
     }

	 /* Zeran - check for "@allon or @alloff", next arg is flag */
     if ( !str_cmp ( flags, "@allon" ) )
          set_all_on = TRUE;
     else if ( !str_cmp ( flags, "@alloff" ) )
          set_all_off = TRUE;

     if ( set_all_on || set_all_off )
     {
          one_argument ( remainder, flags );
          if ( remainder[0] == '\0' )
          {
               send_to_char ( "Syntax:   flags <room flag>\n\r",         ch );
               send_to_char ( "Syntax:   flags @allon <room flag>\n\r",  ch );
               send_to_char ( "Syntax:   flags @alloff <room flag>\n\r", ch );
               return FALSE;
          }
     }

     if ( ( value = flag_value ( room_flags, flags ) ) != NO_FLAG )
     {
          if ( !set_all_on && !set_all_off )
          {
               TOGGLE_BIT ( pRoom->room_flags, value );
               if ( value == ROOM_RENT )
               {
                    LEASE *lease= NULL;
                    // For now we're not freeing the lease structure until reboot.
                    // I don't feel like running the linked list backwards.
                    // The only time a lease struct will be removed anyway is in OLC
                    //
                    if ( IS_SET ( pRoom->room_flags, ROOM_RENT) ) // On or Off?
                    {
                         if ( !pRoom->lease )             // Doesn't exist at all
                         {
                              lease = new_lease ( );
                              lease->rented_by = str_dup ("");
                              pRoom->lease = lease;
                              lease->room = pRoom;
                              VALIDATE ( lease );
                         }
                         else if ( !IS_VALID(pRoom->lease) ) // Exists, but was invalid
                         {
                              /* We don't know why it is invalid, it just is... Fix it up. */
                              lease->rented_by = str_dup ("");
                              pRoom->lease = lease;
                              lease->room = pRoom;
                              VALIDATE ( lease );
                         }
                    }
                    else
                    {
                         if ( pRoom->lease )
                              INVALIDATE ( pRoom->lease ); // Makes sure it doesn't get accessed or saved
                         else
                              bugf ( "Room %d had rent flag with no lease!", pRoom->vnum );
                    }
               }
               send_to_char ( "Room flag toggled.\n\r", ch );
               return TRUE;
          }
          else
          {
               top = pRoom->area->uvnum;
               for ( count = pRoom->area->lvnum; count <= top; count++ )
               {
                    if ( ( pRoom = get_room_index ( count ) ) != NULL )
                    {
                         if ( set_all_on )
                         {
                              SET_BIT ( pRoom->room_flags, value );
                              if ( value == ROOM_RENT )
                              {
                                   LEASE *lease= NULL;
                                   // For now we're not freeing the lease structure until reboot.
                                   // I don't feel like running the linked list backwards.
                                   // The only time a lease struct will be removed anyway is in OLC
                                   //
                                   if ( IS_SET ( pRoom->room_flags, ROOM_RENT) ) // On or Off?
                                   {
                                        if ( !pRoom->lease )             // Doesn't exist at all
                                        {
                                             lease = new_lease ( );
                                             lease->rented_by = str_dup ("");
                                             pRoom->lease = lease;
                                             lease->room = pRoom;
                                             VALIDATE ( lease );
                                        }
                                        else if ( !IS_VALID(pRoom->lease) ) // Exists, but was invalid
                                        {
											 /* We don't know why it is invalid, it just is... Fix it up. */
                                             lease->rented_by = str_dup ("");
                                             pRoom->lease = lease;
                                             lease->room = pRoom;
                                             VALIDATE ( lease );
                                        }
                                   }
                                   else
                                   {
                                        INVALIDATE ( lease ); // Makes sure it doesn't get accessed or saved
                                   }
                              }
                         }
                         else
                              REMOVE_BIT ( pRoom->room_flags, value );
                    }
               }
               if ( set_all_on )
                    send_to_char ( "Flag turned {gon{x for all rooms in this area.\n\r", ch );
               else
                    send_to_char ( "Flag turned {roff{x for all rooms in this area.\n\r", ch );
          }
          return TRUE;
     }
     send_to_char ( "Unrecognized flag.\n\r", ch );
     return FALSE;
}

REDIT ( redit_sector )
{
     ROOM_INDEX_DATA    *pRoom;
     char                flags[40];
     int                 value, count, top;
     bool                set_all_on = FALSE;
     char               *remainder;

     EDIT_ROOM ( ch, pRoom );

     remainder = one_argument ( argument, flags );
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:   sector <type>\n\r", ch );
          send_to_char ( "Syntax:   sector @allon <type>\n\r", ch );
          return FALSE;
     }

    /* Zeran - check for "@allon or @alloff", next arg is flag */
     if ( !str_cmp ( flags, "@allon" ) )
          set_all_on = TRUE;

     if ( set_all_on )
     {
          one_argument ( remainder, flags );
          if ( remainder[0] == '\0' )
          {
               send_to_char ( "Syntax:   sector <type>\n\r", ch );
               send_to_char ( "Syntax:   sector @allon <type>\n\r", ch );
               return FALSE;
          }
     }

     if ( ( value = flag_value ( sector_flags, flags ) ) != NO_FLAG )
     {
          if ( !set_all_on )
          {
               pRoom->sector_type = value;
               send_to_char ( "Sector type set.\n\r", ch );
               return TRUE;
          }
          else
          {
               top = pRoom->area->uvnum;
               for ( count = pRoom->area->lvnum; count <= top;
                     count++ )
               {
                    if ( ( pRoom = get_room_index ( count ) ) != NULL )
                    {
                         pRoom->sector_type = value;
                    }
               }
               send_to_char( "Sector set for all rooms in this area.\n\r", ch );
          }
          return TRUE;
     }
     send_to_char ( "Unrecognized sector.\n\r", ch );
     return FALSE;
}

REDIT ( redit_format )
{
     ROOM_INDEX_DATA    *pRoom;

     EDIT_ROOM ( ch, pRoom );

     pRoom->description = format_string ( pRoom->description );

     send_to_char ( "String formatted.\n\r", ch );
     return TRUE;
}

REDIT ( redit_mreset )
{
     ROOM_INDEX_DATA    *pRoom;
     MOB_INDEX_DATA     *pMobIndex;
     CHAR_DATA          *newmob;
     char                arg[MAX_INPUT_LENGTH];
     RESET_DATA         *pReset;

     EDIT_ROOM ( ch, pRoom );

     argument = one_argument ( argument, arg );

     if ( arg[0] == '\0' || !is_number ( arg ) )
     {
          send_to_char ( "Syntax:  mreset <vnum> <max #>\n\r", ch );
          return FALSE;
     }

     if ( !( pMobIndex = get_mob_index ( atoi ( arg ) ) ) )
     {
          send_to_char ( "REdit: No mobile has that vnum.\n\r", ch );
          return FALSE;
     }

     if ( pMobIndex->area != pRoom->area )
     {
          send_to_char ( "REdit: No such mobile in this area.\n\r", ch );
          return FALSE;
     }

     /*
      * Create the mobile reset.
      */
     pReset = new_reset_data (  );
     pReset->command = 'M';
     pReset->arg1 = pMobIndex->vnum;
     pReset->arg2 = is_number ( argument ) ? atoi ( argument ) : MAX_MOB;
     pReset->arg3 = pRoom->vnum;
     add_reset ( pRoom, pReset, 0 /* Last slot */  );

    /*
     * Create the mobile.
     */
     newmob = create_mobile ( pMobIndex );
     newmob->reset = pReset;
     newmob->reset->count++;
     char_to_room ( newmob, pRoom );

     form_to_char ( ch, 
                    "%s (%d) has been loaded and added to resets.\n\r"
                    "There will be a maximum of %d loaded to this room.\n\r",
                    capitalize ( pMobIndex->short_descr ),
                    pMobIndex->vnum, pReset->arg2 );
     act ( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
     return TRUE;
}

struct wear_type
{
     int                 wear_loc;
     int                 wear_bit;
};

const struct wear_type wear_table[] =
{
     {	WEAR_NONE, 	ITEM_TAKE	},
     {	WEAR_LIGHT, 	ITEM_LIGHT	},
     {	WEAR_FINGER_L, 	ITEM_WEAR_FINGER},
     {	WEAR_FINGER_R, 	ITEM_WEAR_FINGER},
     {	WEAR_NECK_1, 	ITEM_WEAR_NECK	},
     {	WEAR_NECK_2, 	ITEM_WEAR_NECK	},
     {	WEAR_BODY, 	ITEM_WEAR_BODY	},
     {	WEAR_HEAD, 	ITEM_WEAR_HEAD	},
     {	WEAR_LEGS, 	ITEM_WEAR_LEGS	},
     {	WEAR_FEET, 	ITEM_WEAR_FEET	},
     {	WEAR_HANDS, 	ITEM_WEAR_HANDS	},
     {	WEAR_ARMS, 	ITEM_WEAR_ARMS	},
     {	WEAR_SHIELD, 	ITEM_WEAR_SHIELD},
     {	WEAR_ABOUT, 	ITEM_WEAR_ABOUT	},
     {	WEAR_WAIST, 	ITEM_WEAR_WAIST	},
     {	WEAR_WRIST_L, 	ITEM_WEAR_WRIST	},
     {	WEAR_WRIST_R, 	ITEM_WEAR_WRIST	},
     {	WEAR_WIELD, 	ITEM_WIELD	},
     {	WEAR_HOLD, 	ITEM_HOLD	},
     {	WEAR_PRIDE, 	ITEM_WEAR_PRIDE	},
     {	WEAR_FACE, 	ITEM_WEAR_FACE	},
     {	WEAR_EARS, 	ITEM_WEAR_EARS	},
     {	WEAR_FLOAT, 	ITEM_WEAR_FLOAT	},
     {	NO_FLAG, 	NO_FLAG		}
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc ( int bits, int count )
{
     int                 flag;

     for ( flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++ )
     {
          if ( IS_SET ( bits, wear_table[flag].wear_bit ) && --count < 1 )
               return wear_table[flag].wear_loc;
     }

     return NO_FLAG;
}

/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit ( int loc )
{
     int                 flag;

     for ( flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++ )
     {
          if ( loc == wear_table[flag].wear_loc )
               return wear_table[flag].wear_bit;
     }

     return 0;
}

REDIT ( redit_oreset )
{
     ROOM_INDEX_DATA    *pRoom;
     OBJ_INDEX_DATA     *pObjIndex;
     OBJ_DATA           *newobj;
     OBJ_DATA           *to_obj;
     CHAR_DATA          *to_mob;
     char                arg1[MAX_INPUT_LENGTH];
     char                arg2[MAX_INPUT_LENGTH];
     char                max[MAX_INPUT_LENGTH];
     int                 olevel = 0;
     RESET_DATA         *pReset;

     EDIT_ROOM ( ch, pRoom );

     argument = one_argument ( argument, arg1 );
     argument = one_argument ( argument, max );
     argument = one_argument ( argument, arg2 );

     if ( arg1[0] == '\0' || !is_number ( arg1 )
          || max[0] == '\0' || !is_number ( max ) )
     {
          send_to_char ( "Syntax:  oreset <vnum> <max> <args>\n\r",        ch );
          send_to_char ( "        -no_args               = into room\n\r", ch );
          send_to_char ( "        -<obj_name>            = into obj\n\r",  ch );
          send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r",  ch );
          return FALSE;
     }

     if ( !( pObjIndex = get_obj_index ( atoi ( arg1 ) ) ) )
     {
          send_to_char ( "REdit: No object has that vnum.\n\r", ch );
          return FALSE;
     }

     if ( pObjIndex->area != pRoom->area )
     {
          send_to_char ( "REdit: No such object in this area.\n\r", ch );
          return FALSE;
     }

     /*
      * Load into room.
      */
     if ( arg2[0] == '\0' )
     {
          pReset = new_reset_data (  );
          pReset->command = 'O';
          pReset->arg1 = pObjIndex->vnum;
          pReset->arg2 = atoi ( max );
          pReset->arg3 = pRoom->vnum;
          pReset->count = 1;
          add_reset ( pRoom, pReset, 0 /* Last slot */  );

          newobj = create_object ( pObjIndex, number_fuzzy ( olevel ) );
          newobj->reset = pReset;
          obj_to_room ( newobj, pRoom );

          form_to_char ( ch, 
                         "%s (%d) has been loaded and added to resets.\n\r",
                         capitalize ( pObjIndex->short_descr ),
                         pObjIndex->vnum );
     }
     else
	/*
	 * Load into object's inventory.
	 */
          if ( argument[0] == '\0' && 
               ( ( to_obj = get_obj_list ( ch, arg2, pRoom->contents ) ) != NULL ) )
          {
               pReset = new_reset_data (  );
               pReset->command = 'P';
               pReset->arg1 = pObjIndex->vnum;
               pReset->arg2 = atoi ( max );
               pReset->arg3 = to_obj->pIndexData->vnum;
               pReset->count = 1;
               add_reset ( pRoom, pReset, 0 /* Last slot */  );
               newobj = create_object ( pObjIndex, number_fuzzy ( olevel ) );
               newobj->reset = pReset;
               newobj->cost = 0;
               obj_to_obj ( newobj, to_obj );
               form_to_char ( ch, "%s (%d) has been loaded into %s (%d) and added to resets.\n\r",
                              capitalize ( newobj->short_descr ),
                              newobj->pIndexData->vnum,
                              to_obj->short_descr, to_obj->pIndexData->vnum );
          }
     else
	/*
	 * Load into mobile's inventory.
	 */
          if ( ( to_mob = get_char_room ( ch, NULL, arg2 ) ) != NULL )
          {
               int                 wear_loc;

               /*
                * Make sure the location on mobile is valid.
                */
               if ( ( wear_loc = flag_value ( wear_loc_flags, argument ) ) == NO_FLAG )
               {
                    send_to_char ( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
                    return FALSE;
               }

               /*
                * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
                */
               if ( !IS_SET ( pObjIndex->wear_flags, wear_bit ( wear_loc ) ) )
               {
                    form_to_char ( ch, 
                                   "%s (%d) has wear flags: [%s]\n\r",
                                   capitalize ( pObjIndex->short_descr ),
                                   pObjIndex->vnum,
                                   flag_string ( wear_flags, pObjIndex->wear_flags ) );
                    return FALSE;
               }

               pReset = new_reset_data (  );
               pReset->arg1 = pObjIndex->vnum;
               pReset->arg3 = wear_loc;
               pReset->count = 1;
               if ( pReset->arg3 == WEAR_NONE )
                    pReset->command = 'G';
               else
               {
                    pReset->command = 'E';
                    /* Can't wear 2 of the same item 
                     * Moved from above so that giving 2 of the same item works - Lotherius
                     */
                    if ( get_eq_char ( to_mob, wear_loc ) )
                    {
                         send_to_char ( "REdit: Already equipped that location.\n\r", ch );
                         return FALSE;
                    }
               }
               pReset->arg2 = atoi ( max );

               add_reset ( pRoom, pReset, 0 /* Last slot */  );

               olevel = URANGE ( 0, to_mob->level - 2, LEVEL_HERO );
               newobj = create_object ( pObjIndex, number_fuzzy ( olevel ) );

               if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
               {
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
                    case ITEM_WEAPON:
                         if ( pReset->command == 'G' )
                              olevel = number_range ( 5, 15 );
                         else
                              olevel = number_fuzzy ( olevel );
                         break;
                    }
                    newobj = create_object ( pObjIndex, olevel );
                    newobj->reset = pReset;
                    newobj->reset->count = 1;
                    if ( pReset->arg3 == WEAR_NONE )
                         SET_BIT ( newobj->extra_flags, ITEM_INVENTORY );
               }
               else
                    newobj = create_object ( pObjIndex, number_fuzzy ( olevel ) );
               newobj->reset = pReset;
               newobj->reset->count = 1;
               obj_to_char ( newobj, to_mob );
               if ( pReset->command == 'E' )
                    equip_char ( to_mob, newobj, pReset->arg3 );
               form_to_char ( ch, "%s (%d) has been loaded "
                              "%s of %s (%d) and added to resets.\n\r",
                              capitalize ( pObjIndex->short_descr ),
                              pObjIndex->vnum,
                              flag_string ( wear_loc_strings, pReset->arg3 ),
                              to_mob->short_descr, to_mob->pIndexData->vnum );
          }
     else			/* Display Syntax */
     {
          send_to_char ( "REdit:  That mobile isn't here.\n\r", ch );
          return FALSE;
     }

     act ( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
     return TRUE;
}

/*
 * Object Editor Functions.
 */
void show_obj_values ( CHAR_DATA * ch, OBJ_INDEX_DATA * obj )
{
     switch ( obj->item_type )
     {
     default:			/* No values. */
          break;
     case ITEM_PORTAL:
          form_to_char ( ch, "[v0] Destination room:   [%d]\n\r"
                         "[v1] Link object vnum:   [%d]\n\r", obj->value[0], obj->value[1] );
          break;
     case ITEM_LIGHT:
          if ( obj->value[2] == -1 || obj->value[2] == 999 )	/* ROM OLC */
               form_to_char ( ch, "[v2] Light:  Infinite[-1]\n\r" );
          else
               form_to_char ( ch, "[v2] Light:  [%d]\n\r", obj->value[2] );
          break;
     case ITEM_WAND:
          /* FALLTHROUGH */
          
     case ITEM_STAFF:
          form_to_char ( ch, 
                         "[v0] Level:          [%d]\n\r"
                         "[v1] Charges Total:  [%d]\n\r"
                         "[v2] Charges Left:   [%d]\n\r"
                         "[v3] Spell:          %s\n\r",
                         obj->value[0],
                         obj->value[1],
                         obj->value[2],
                         obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none" );
          break;
     case ITEM_SCROLL:
          /* FALLTHROUGH */
     case ITEM_POTION:
          /* FALLTHROUGH */
          
     case ITEM_PILL:
          form_to_char ( ch, 
                         "[v0] Level:  [%d]\n\r"
                         "[v1] Spell:  %s\n\r"
                         "[v2] Spell:  %s\n\r"
                         "[v3] Spell:  %s\n\r",
                         obj->value[0],
                         obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none",
                         obj->value[2] != -1 ? skill_table[obj->value[2]].name : "none",
                         obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none" );
          break;
     case ITEM_WEAPON:
          form_to_char ( ch, "[v0] Weapon class:   %s\n\r", flag_string ( weapon_class, obj->value[0] ) );
          form_to_char ( ch, "[v1] Number of dice: [%d]\n\r", obj->value[1] );
          form_to_char ( ch, "[v2] Type of dice:   [%d]\n\r", obj->value[2] );
          form_to_char ( ch, "[v3] Type:           %s\n\r", flag_string ( weapon_flags, obj->value[3] ) );
          form_to_char ( ch, "[v4] Special type:   %s\n\r", flag_string ( weapon_type, obj->value[4] ) );
          break;
     case ITEM_CONTAINER:
          form_to_char ( ch, 
                         "[v0] Weight: [%d kg]\n\r"
                         "[v1] Flags:  [%s]\n\r"
                         "[v2] Key:    %s [%d]\n\r",
                         obj->value[0],
                         flag_string ( container_flags, obj->value[1] ),
                         get_obj_index ( obj->value[2] )
                         ? get_obj_index ( obj->value[2] )->short_descr
                         : "none", obj->value[2] );
          break;
     case ITEM_DRINK_CON:
          form_to_char ( ch, 
                         "[v0] Liquid Total: [%d]\n\r"
                         "[v1] Liquid Left:  [%d]\n\r"
                         "[v2] Liquid:       %s\n\r"
                         "[v3] Poisoned:     %s\n\r",
                         obj->value[0],
                         obj->value[1],
                         flag_string ( liquid_flags, obj->value[2] ),
                         obj->value[3] != 0 ? "Yes" : "No" );
          break;
     case ITEM_FURNITURE:
          form_to_char ( ch, 
                         "[v0] Number Persons:    [%d]\n\r"
                         "[v1] Weight Allow:      [%d] (unused)\n\r"
                         "[v2] BitField:          [%d]\n\r"
                         "[v3] Heal Bonus:        [%d]\n\r",
                         obj->value[0],
                         obj->value[1], obj->value[2], obj->value[3] );
          break;
     case ITEM_FOOD:
          form_to_char ( ch, 
                         "[v0] Food hours: [%d]\n\r"
                         "[v3] Poisoned:   %s\n\r",
                         obj->value[0],
                         obj->value[3] != 0 ? "Yes" : "No" );
          break;
     case ITEM_MONEY:
          form_to_char ( ch, "[v0] Gold:   [%d]\n\r", obj->value[0] );
          break;
     }

     return;
}

bool set_obj_values ( CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, int value_num, char *argument )
{
     switch ( pObj->item_type )
     {
     default:
          break;
     case ITEM_LIGHT:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_LIGHT" );
               return FALSE;
          case 2:
               send_to_char ( "HOURS OF LIGHT SET.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          }
          break;

     case ITEM_WAND:
          /* FALLTHROUGH */
     case ITEM_STAFF:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_STAFF_WAND" );
               return FALSE;
          case 0:
               send_to_char ( "SPELL LEVEL SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               send_to_char ( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
               pObj->value[1] = atoi ( argument );
               break;
          case 2:
               send_to_char ( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          case 3:
               send_to_char ( "SPELL TYPE SET.\n\r", ch );
               pObj->value[3] = skill_lookup ( argument );
               break;
          }
          break;

     case ITEM_SCROLL:
          /* FALLTHROUGH */
     case ITEM_POTION:
          /* FALLTHROUGH */
     case ITEM_PILL:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_SCROLL_POTION_PILL" );
               return FALSE;
          case 0:
               send_to_char ( "SPELL LEVEL SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               send_to_char ( "SPELL TYPE 1 SET.\n\r\n\r", ch );
               pObj->value[1] = skill_lookup ( argument );
               break;
          case 2:
               send_to_char ( "SPELL TYPE 2 SET.\n\r\n\r", ch );
               pObj->value[2] = skill_lookup ( argument );
               break;
          case 3:
               send_to_char ( "SPELL TYPE 3 SET.\n\r\n\r", ch );
               pObj->value[3] = skill_lookup ( argument );
               break;
          }
          break;
     case ITEM_PORTAL:
          switch ( value_num )
          {
          default:
               send_to_char ( "Only value0 and value1 used for portals.\n\r", ch );
               break;
          case 0:
               if ( get_room_index ( atoi ( argument ) ) == NULL )
               {
                    send_to_char ( "Invalid room vnum.\n\r", ch );
               }
               else
               {
                    pObj->value[0] = atoi ( argument );
                    send_to_char ( "Destination room set.\n\r", ch );
               }
               break;
          case 1:
               if ( get_obj_index ( atoi ( argument ) ) == NULL )
               {
                    send_to_char ( "Invalid link object vnum.\n\r", ch );
               }
               else
               {
                    pObj->value[1] = atoi ( argument );
                    send_to_char ( "Link object set.\n\r", ch );
               }
               break;
          }
          break;
     case ITEM_ARMOR:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_ARMOR" );
               return FALSE;
          case 0:
               send_to_char ( "AC PIERCE SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               send_to_char ( "AC BASH SET.\n\r\n\r", ch );
               pObj->value[1] = atoi ( argument );
               break;
          case 2:
               send_to_char ( "AC SLASH SET.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          case 3:
               send_to_char ( "AC EXOTIC SET.\n\r\n\r", ch );
               pObj->value[3] = atoi ( argument );
               break;
          }
          break;
     case ITEM_WEAPON:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_WEAPON" );
               return FALSE;
          case 0:
               send_to_char ( "WEAPON CLASS SET.\n\r\n\r", ch );
               pObj->value[0] = flag_value ( weapon_class, argument );
               break;
          case 1:
               send_to_char ( "NUMBER OF DICE SET.\n\r\n\r", ch );
               pObj->value[1] = atoi ( argument );
               break;
          case 2:
               send_to_char ( "TYPE OF DICE SET.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          case 3:
               send_to_char ( "WEAPON TYPE SET.\n\r\n\r", ch );
               pObj->value[3] = flag_value ( weapon_flags, argument );
               break;
          case 4:
               {
                    long                value = 0;
                    send_to_char ( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
                    value = flag_value ( weapon_type, argument );
                    if ( value != NO_FLAG )
                         TOGGLE_BIT ( pObj->value[4], value );
               }
               break;
          }
          break;
     case ITEM_CONTAINER:
          switch ( value_num )
          {
               int                 value;
          default:
               do_help ( ch, "ITEM_CONTAINER" );
               return FALSE;
          case 0:
               send_to_char ( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               if ( ( value = flag_value ( container_flags, argument ) ) != NO_FLAG )
                    TOGGLE_BIT ( pObj->value[1], value );
               else
               {
                    do_help ( ch, "ITEM_CONTAINER" );
                    return FALSE;
               }
               send_to_char ( "CONTAINER TYPE SET.\n\r\n\r", ch );
               break;
          case 2:
               if ( atoi ( argument ) != 0 )
               {
                    if ( !get_obj_index ( atoi ( argument ) ) )
                    {
                         send_to_char ( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
                         return FALSE;
                    }
                    if ( get_obj_index ( atoi ( argument ) )->item_type != ITEM_KEY )
                    {
                         send_to_char ( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
                         return FALSE;
                    }
               }
               send_to_char ( "CONTAINER KEY SET.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          }
          break;

     case ITEM_DRINK_CON:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_DRINK" );
               /* OLC		    do_help( ch, "liquids" );    */
               return FALSE;
          case 0:
               send_to_char ( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               send_to_char ( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
               pObj->value[1] = atoi ( argument );
               break;
          case 2:
               send_to_char ( "LIQUID TYPE SET.\n\r\n\r", ch );
               pObj->value[2] = flag_value ( liquid_flags, argument );
               break;
          case 3:
               send_to_char ( "POISON VALUE TOGGLED.\n\r\n\r", ch );
               pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
               break;
          }
          break;

     case ITEM_FURNITURE:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_FURNITURE" );
               return FALSE;
          case 0:
               send_to_char ( "Number of Players Set.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 1:
               send_to_char ( "Weight Allowance Set.\n\r\n\r", ch );
               pObj->value[1] = atoi ( argument );
               break;
          case 2:
               send_to_char ( "Bitfield Set.\n\r\n\r", ch );
               pObj->value[2] = atoi ( argument );
               break;
          case 3:
               send_to_char ( "Heal Bonus Set.\n\r\n\r", ch );
               pObj->value[3] = atoi ( argument );
               break;
          }
          break;

     case ITEM_FOOD:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_FOOD" );
               return FALSE;
          case 0:
               send_to_char ( "HOURS OF FOOD SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          case 3:
               send_to_char ( "POISON VALUE TOGGLED.\n\r\n\r", ch );
               pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
               break;
          }
          break;

     case ITEM_MONEY:
          switch ( value_num )
          {
          default:
               do_help ( ch, "ITEM_MONEY" );
               return FALSE;
          case 0:
               send_to_char ( "GOLD AMOUNT SET.\n\r\n\r", ch );
               pObj->value[0] = atoi ( argument );
               break;
          }
          break;
     }
     show_obj_values ( ch, pObj );
     return TRUE;
}

OEDIT ( oedit_unused )
{
     ROOM_INDEX_DATA    *pRoom;
     AREA_DATA          *pArea;
     OBJ_INDEX_DATA     *tmp;
     BUFFER		*buffer;
     int                 counter;
     
     buffer = buffer_new ( 1024 );

     EDIT_ROOM ( ch, pRoom );
     pArea = pRoom->area;
     
     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );
     bprintf ( buffer, "UNUSED OBJECT VNUMS\n\r------ ------ -----\n\r" );
     counter = pArea->lvnum;
     for ( ; ( counter <= pArea->uvnum ); counter++ )
     {
          if ( ( tmp = get_obj_index ( counter ) ) == NULL )
               bprintf ( buffer, "[%d]\n\r", counter );
     }
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return TRUE;
}

/*
 * used has become a much more complicated function - Lotherius
 */
OEDIT ( oedit_used )
{
     ROOM_INDEX_DATA   *pRoom;
     AREA_DATA         *pArea;
     OBJ_INDEX_DATA    *tmp;
     AFFECT_DATA       *paf;
     char               buf[MAX_STRING_LENGTH];
     int                counter, num, uvnum;
     int		cnt;
     BUFFER	       *buffer;
     char		apply_buf[MAX_STRING_LENGTH];
     char		affect_buf[MAX_STRING_LENGTH];
     bool		haveargs = FALSE;
     bool		level, repop, material, type;
     bool		extra, wear, condition;
     bool		weight, size, cost, affected;
     bool		Fapply = FALSE;
     bool		Faffect = FALSE;

     EDIT_ROOM ( ch, pRoom );

     buffer = buffer_new (2000);

    /* Let's skip lots of worry if we don't have any arguments. */

     if ( argument[0] != '\0' )
          haveargs = TRUE;

     pArea = pRoom->area;

     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );

     bprintf ( buffer, "USED OBJECT VNUMS\n\r---- --- -----\n\r" );

     counter = pArea->lvnum;
     uvnum = pArea->uvnum;

     /* Set the booleans... Long.. */
     if ( (haveargs) && !strcmp (argument, "level") ) /* Will only happen if level is the *only* argument */
     {
          level = TRUE;
          haveargs = FALSE;
     }
     if ( strstr (argument, "level" ) != NULL )		level = TRUE;
     else level = FALSE;
     if ( strstr (argument, "repop" ) != NULL ) 	repop = TRUE;
     else repop = FALSE;
     if ( strstr (argument, "material" ) != NULL ) 	material = TRUE;
     else material = FALSE;
     if ( strstr (argument, "type" ) != NULL ) 		type = TRUE;
     else type = FALSE;
     if ( strstr (argument, "extra" ) != NULL ) 	extra = TRUE;
     else extra = FALSE;
     if ( strstr (argument, "wear" ) != NULL ) 		wear = TRUE;
     else wear = FALSE;
     if ( strstr (argument, "condition" ) != NULL ) 	condition = TRUE;
     else condition = FALSE;
     if ( strstr (argument, "weight" ) != NULL ) 	weight = TRUE;
     else weight = FALSE;
     if ( strstr (argument, "size" ) != NULL ) 		size = TRUE;
     else size = FALSE;
     if ( strstr (argument, "cost" ) != NULL ) 		cost = TRUE;
     else cost = FALSE;
     if ( strstr (argument, "affects" ) != NULL ) 	affected = TRUE;
     else affected = FALSE;

     if ( level )
     {
          vlh_type           *hashlist[MAX_OBJ_LEVEL + 1];
          vlh_type           *hash_tmp, *free_tmp = NULL;
          char                buf[128];

          for ( num = 0; num <= MAX_OBJ_LEVEL; num++ )
               hashlist[num] = NULL;
          for ( ; counter <= uvnum; counter++ )
          {
               if ( ( tmp = get_obj_index ( counter ) ) != NULL )
               {
                    if ( !hashlist[tmp->level] )
                    {
			 hashlist[tmp->level] = alloc_mem ( sizeof ( vlh_type ), "vlh_type" );
                         hashlist[tmp->level]->vnum = tmp->vnum;
                         hashlist[tmp->level]->next = NULL;
                    }
                    else
                    {
                         for ( hash_tmp = hashlist[tmp->level]; hash_tmp->next != NULL; hash_tmp = hash_tmp->next );
                         	/* Nothing */
			 hash_tmp->next = alloc_mem ( sizeof ( vlh_type ), "vlh_type" );
                         hash_tmp->next->next = NULL;
                         hash_tmp->next->vnum = tmp->vnum;
                    }
               }
          }
          /* end for loop through vnums */
          /* now print out list */
          for ( counter = 0; counter <= MAX_OBJ_LEVEL; counter++ )
          {
               for ( hash_tmp = hashlist[counter]; hash_tmp != NULL; hash_tmp = hash_tmp->next )
               {
                    apply_buf[0] = '\0';
                    affect_buf[0] = '\0';
                    Fapply = FALSE;
                    Faffect = FALSE;

                    if ( !free_tmp )
                         free_mem ( free_tmp, sizeof ( vlh_type ), "vlh_type" );
                    tmp = get_obj_index ( hash_tmp->vnum );

                    if (affected && !tmp->affected)
                         continue;

                    bprintf ( buffer, "{W[{Y%5d{W] [{G%3d{W] {C%s{w", tmp->vnum, tmp->level, ( ( tmp->short_descr )
                                                                      ? ( tmp->short_descr ) : "" ) );
                    if (haveargs)
                    {
                         bprintf (buffer, "\n\r        ");
                         if (weight)            bprintf (buffer, "Wg[%6d] ", tmp->weight );
                         if (cost)              bprintf (buffer, "Cs[%8d] ", tmp->cost );
                         if (condition)         bprintf (buffer, "Cn[%3d] ", tmp->condition );
                         if (repop)             bprintf (buffer, "Re[%3d] \n\r", tmp->repop );
                         if (type)              bprintf (buffer, "Ty[%10s] ", flag_string ( type_flags, tmp->item_type ) );
                         if (material)          bprintf (buffer, "Ma[%10s] \n\r", flag_string ( material_type, tmp->material ) );
                         if (wear)              bprintf (buffer, "Wr[%35s] ", flag_string ( wear_flags, tmp->wear_flags ) );
                         if (extra)             bprintf (buffer, "Ex[%s]", flag_string ( extra_flags, tmp->extra_flags ) );
                         if (affected) /* Go into block */
                         {
                              for ( cnt = 0, paf = tmp->affected; paf; paf = paf->next )
                              {
			           if ( paf->bitvector)
			           {
					switch (paf->where)
                                        {
                                        case TO_AFFECTS:
                                             SNP (buf, "[%4d]          (Affect)  %s\n\r", cnt,
                                                      flag_string (affect_flags, paf->bitvector) );
                                             break;
                                        case TO_DETECTIONS:
                                             SNP (buf, "[%4d]          (Detect)  %s\n\r", cnt,
                                                      flag_string (detect_flags, paf->bitvector) );
                                             break;
                                        case TO_PROTECTIONS:
                                             SNP (buf, "[%4d]          (Protect) %s\n\r", cnt,
                                                      flag_string (protect_flags, paf->bitvector) );
                                             break;
                                        }
                                        SLCAT ( affect_buf, buf );
                                        Faffect = TRUE;
			           }
			           else
			           {
                                        SNP ( buf, "[%4d] %-8d %s\n\r", cnt,
                                                  paf->modifier, flag_string ( apply_flags, paf->location ) );
                                        SLCAT ( apply_buf, buf );
                                        Fapply = TRUE;
			           }
			           cnt++;
                              }
                              if ( Fapply )
                              {
                                   bprintf ( buffer, "Number Modifier Affects\n\r" );
                                   bprintf ( buffer, "------ -------- -------\n\r" );
                                   bprintf ( buffer, apply_buf );
                              }
                              if ( Faffect )
                              {
                                   bprintf ( buffer, "{RNumber          Spell Affect{w\n\r" );
                                   bprintf ( buffer, "------          ------------\n\r" );
                                   bprintf ( buffer, "{G%s{w", affect_buf );
                              }
                         } /* End Affected Block */
                    } /* End Haveargs Block */
                    bprintf (buffer, "\n\r");
                    free_tmp = hash_tmp;
               }
               if ( !free_tmp )
                    free_mem ( free_tmp, sizeof ( vlh_type ), "vlh_type" );
          } /* End Level Loop */
     } /* end level block */
     else
     {
          for ( ; ( counter <= pArea->uvnum ); counter++ )
          {
               if ( ( tmp = get_obj_index ( counter ) ) != NULL )
               {

                    if (affected && !tmp->affected)
                         continue;
                    bprintf ( buffer, "{W[{Y%5d{W] [{G%3d{W] {C%s{w", tmp->vnum, tmp->level, ( ( tmp->short_descr )
                                                                      ? ( tmp->short_descr ) : "" ) );
                    if (haveargs)
                    {
                         apply_buf[0] = '\0';
                         affect_buf[0] = '\0';
                         Fapply = FALSE;
                         Faffect = FALSE;
                         bprintf (buffer, "\n\r        ");

                         if (weight)           bprintf (buffer, "Wg[%6d] ", tmp->weight );
                         if (cost)             bprintf (buffer, "Cs[%8d] ", tmp->cost );
                         if (condition)        bprintf (buffer, "Cn[%3d] ", tmp->condition );
                         if (repop)            bprintf (buffer, "Re[%3d] \n\r", tmp->repop );
                         if (type)             bprintf (buffer, "Ty[%10s] ", flag_string ( type_flags, tmp->item_type ) );
                         if (material)         bprintf (buffer, "Ma[%10s] \n\r", flag_string ( material_type, tmp->material ) );
                         if (wear)             bprintf (buffer, "Wr[%35s] ", flag_string ( wear_flags, tmp->wear_flags ) );
                         if (extra)            bprintf (buffer, "Ex[%s]", flag_string ( extra_flags, tmp->extra_flags ) );
                         if (affected)
                         {
                              for ( cnt = 0, paf = tmp->affected; paf; paf = paf->next )
                              {
                                   if ( paf->bitvector)
                                   {
                                        switch (paf->where)
                                        {
                                        case TO_AFFECTS:
                                             SNP (buf, "[%4d]          (Affect)  %s\n\r", cnt,
                                                      flag_string (affect_flags, paf->bitvector) );
                                             break;
                                        case TO_DETECTIONS:
                                             SNP (buf, "[%4d]          (Detect)  %s\n\r", cnt,
                                                      flag_string (detect_flags, paf->bitvector) );
                                             break;
                                        case TO_PROTECTIONS:
                                             SNP (buf, "[%4d]          (Protect) %s\n\r", cnt,
                                                      flag_string (protect_flags, paf->bitvector) );
                                             break;
                                        }

                                        SLCAT ( affect_buf, buf );
                                        Faffect = TRUE;
                                   }
                                   else
                                   {
                                        SNP ( buf, "[%4d] %-8d %s\n\r", cnt,
                                                  paf->modifier, flag_string ( apply_flags, paf->location ) );
                                        SLCAT ( apply_buf, buf );
                                        Fapply = TRUE;
                                   }
                                   cnt++;
                              }

                              if ( Fapply || Faffect)
                                   bprintf ( buffer, "\n\r" );
                              if ( Fapply )
                              {
                                   bprintf ( buffer, "Number Modifier Affects\n\r" );
                                   bprintf ( buffer, "------ -------- -------\n\r" );
                                   bprintf ( buffer, apply_buf );
                              }
                              if ( Faffect )
                              {
                                   bprintf ( buffer, "{RNumber          Spell Affect{w\n\r" );
                                   bprintf ( buffer, "------          ------------\n\r" );
                                   bprintf ( buffer, "{G%s{w", affect_buf );
                              }

                         } /* End of Affected Section */
                    }  /* End of haveargs */
                    bprintf (buffer, "\n\r");
               }
          }
     }

     page_to_char (buffer->data, ch);
     buffer_free(buffer);

     /* phew */
     return TRUE;
}

OEDIT ( oedit_show )
{
     OBJ_INDEX_DATA     *pObj;
     PROG_LIST          *list;
     char                apply_buf[MAX_STRING_LENGTH];
     char                affect_buf[MAX_STRING_LENGTH];
     char                tmp_buf[MAX_STRING_LENGTH];
     AFFECT_DATA        *paf;
     int                 cnt;
     bool                Fapply = FALSE;
     bool                Faffect = FALSE;

     apply_buf[0] = '\0';
     affect_buf[0] = '\0';
     tmp_buf[0] = '\0';

     EDIT_OBJ ( ch, pObj );

     if ( pObj == NULL )
          return FALSE;

     form_to_char ( ch, "Name: [{C%s{w]  Area: [{C%5d{w] %s\n\r",
                    pObj->name,
                    !pObj->area ? -1 : pObj->area->vnum,
                    !pObj->area ? "No Area" : pObj->area->name );
     form_to_char ( ch, "Vnum: [{C%5d{w]  Type: [{C%s{w] Level: [{C%3d{w] Repop: [{C%3d{w]\n\r",
                    pObj->vnum, flag_string ( type_flags, pObj->item_type ),
                    pObj->level, pObj->repop );
     form_to_char ( ch, "Wear flags:  [{C%s{w]\n\r", flag_string ( wear_flags, pObj->wear_flags ) );
     form_to_char ( ch, "Extra flags: [{C%s{w]\n\r", flag_string ( extra_flags, pObj->extra_flags ) );

     switch ( pObj->item_type )
     {
     default:
          break;
     case ITEM_ARMOR:
          form_to_char ( ch, "Vflags: [{C%s{w]\n\r", flag_string ( vflags_armor, pObj->vflags ) );
          break;
     }
     
     form_to_char ( ch, "Material: [{C%s{w] Condition: [{C%3d{w] Weight: [{C%5d{w] Cost: [{C%5d{w]\n\r",
                    flag_string ( material_type, pObj->material ),
                    pObj->condition, pObj->weight, pObj->cost );
     if ( pObj->extra_descr )
     {
          EXTRA_DESCR_DATA   *ed;

          send_to_char ( "Ex desc kwd: ", ch );

          for ( ed = pObj->extra_descr; ed; ed = ed->next )
          {
               send_to_char ( "[", ch );
               send_to_char ( ed->keyword, ch );
               send_to_char ( "]", ch );
          }

          send_to_char ( "\n\r", ch );
     }

     form_to_char ( ch, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r", pObj->short_descr, pObj->description );
     for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
     {
          if ( paf->bitvector)
          {
               switch (paf->where)
               {
               case TO_DETECTIONS:
                    SNP (tmp_buf, "[%4d]          (Detect)  %s\n\r", cnt, flag_string (detect_flags, paf->bitvector) );
                    break;
               case TO_AFFECTS:
                    SNP (tmp_buf, "[%4d]          (Affect)  %s\n\r", cnt, flag_string (affect_flags, paf->bitvector) );
                    break;
               case TO_PROTECTIONS:
                    SNP (tmp_buf, "[%4d]          (Protect) %s\n\r", cnt, flag_string (protect_flags, paf->bitvector) );
                    break;
               }
               SLCAT ( affect_buf, tmp_buf );
               Faffect = TRUE;
          }
          else
          {
               SNP ( tmp_buf, "[%4d] %-8d %s\n\r", cnt, paf->modifier, flag_string ( apply_flags, paf->location ) );
               SLCAT ( apply_buf, tmp_buf );
               Fapply = TRUE;
          }
          cnt++;
     }
     if ( Fapply )
     {
          send_to_char ( "Number Modifier Affects\n\r", ch );
          send_to_char ( "------ -------- -------\n\r", ch );
          send_to_char ( apply_buf, ch );
     }
     if ( Faffect )
     {
          send_to_char ( "Number          Spell Affect\n\r", ch );
          send_to_char ( "------          ------------\n\r", ch );
          send_to_char ( affect_buf, ch );
     }
     send_to_char ( "Values\n\r------\n\r", ch );
     show_obj_values ( ch, pObj );

     if ( pObj->oprogs )
     {
          int cnt;
          form_to_char ( ch, "\n\rOBJPrograms for [%5d]:\n\r", pObj->vnum);
          for (cnt=0, list=pObj->oprogs; list; list=list->next)
          {
               if (cnt ==0)
               {
                    send_to_char ( " Number Vnum Trigger Phrase\n\r", ch );
                    send_to_char ( " ------ ---- ------- ------\n\r", ch );
               }
               form_to_char ( ch, "[%5d] %4d %7s %s\n\r", cnt,
                              list->vnum,prog_type_to_name(list->trig_type),
                              list->trig_phrase);
               cnt++;
          }
     }
     return FALSE;
}

OEDIT ( oedit_addapply )
{
     int                 value;
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *pAf;
     char                loc[MAX_STRING_LENGTH];
     char                mod[MAX_STRING_LENGTH];

     EDIT_OBJ ( ch, pObj );

     argument = one_argument ( argument, loc );
     one_argument ( argument, mod );

     if ( loc[0] == '\0' || mod[0] == '\0' || !is_number ( mod ) )
     {
          send_to_char ( "Syntax:  addapply [location] [#mod]\n\r", ch );
          send_to_char ( "? apply for valid locations.\n\r", ch );
          send_to_char ( "Ex:  addapply hp 100\n\r", ch );
          return FALSE;
     }

     if ( ( value = flag_value ( apply_flags, loc ) ) == NO_FLAG )	/* Hugin */
     {
          send_to_char ( "Valid applies are:\n\r", ch );
          show_help ( ch, "? apply" );
          return FALSE;
     }

     pAf = new_affect (  );
     pAf->location = value;
     pAf->modifier = atoi ( mod );
     pAf->type = -1;
     pAf->duration = -1;
     pAf->bitvector = 0;
     pAf->next = pObj->affected;
     pObj->affected = pAf;

     send_to_char ( "Apply added.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_addaffect )
{
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *pAf;
     char                affect_name[MAX_STRING_LENGTH];
     int value;

     EDIT_OBJ ( ch, pObj );

     one_argument ( argument, affect_name );

     if ( affect_name[0] == '\0' )
     {
          send_to_char ( "Syntax:  addaffect [affect_name]\n\r",  ch );
          send_to_char ( "Ex:  addaffect sanctuary\n\r", ch );
          send_to_char ( "Valid affects are:\n\r", ch );
          show_affect_help ( ch );
          return FALSE;
     }

     if ( ( value = flag_value( affect_flags, affect_name ) ) == NO_FLAG ) /* Hugin */
     {
          send_to_char( "Valid affects are:\n\r", ch );
          show_help( ch, "? affect"  );
          return FALSE;
     }

     pAf = new_affect (  );
     pAf->where = TO_AFFECTS;
     pAf->location = 0;
     pAf->modifier = 0;
     pAf->type = -1;
     pAf->duration = -1;
     pAf->bitvector = value;
     pAf->next = pObj->affected;
     pObj->affected = pAf;

     send_to_char ( "Affect added.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_adddetect )
{
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *pAf;
     char                detect_name[MAX_STRING_LENGTH];
     int value;

     EDIT_OBJ ( ch, pObj );

     one_argument ( argument, detect_name );

     if ( detect_name[0] == '\0' )
     {
          send_to_char ( "Syntax:  adddetect [detect_name]\n\r",  ch );
          send_to_char ( "Ex:  addetect hidden\n\r", ch );
          send_to_char ( "Valid detections are:\n\r", ch );
          show_detect_help ( ch );
          return FALSE;
     }

     if ( ( value = flag_value( detect_flags, detect_name ) ) == NO_FLAG ) /* Hugin */
     {
          send_to_char( "Valid detects are:\n\r", ch );
          show_help( ch, "? detect"  );
          return FALSE;
     }

     pAf = new_affect (  );
     pAf->where = TO_DETECTIONS;
     pAf->location = 0;
     pAf->modifier = 0;
     pAf->type = -1;
     pAf->duration = -1;
     pAf->bitvector = value;
     pAf->next = pObj->affected;
     pObj->affected = pAf;

     send_to_char ( "Detect added.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_addprotect )
{
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *pAf;
     char                protect_name[MAX_STRING_LENGTH];
     int value;

     EDIT_OBJ ( ch, pObj );

     one_argument ( argument, protect_name );

     if ( protect_name[0] == '\0' )
     {
          send_to_char ( "Syntax:  addprotect [protect_name]\n\r",  ch );
          send_to_char ( "Ex:  addprotect sanctuary\n\r", ch );
          send_to_char ( "Valid protections are:\n\r", ch );
          show_protect_help ( ch );
          return FALSE;
     }

     if ( ( value = flag_value( protect_flags, protect_name ) ) == NO_FLAG ) /* Hugin */
     {
          send_to_char( "Valid protects are:\n\r", ch );
          show_help( ch, "? protect"  );
          return FALSE;
     }

     pAf = new_affect (  );
     pAf->where = TO_PROTECTIONS;
     pAf->location = 0;
     pAf->modifier = 0;
     pAf->type = -1;
     pAf->duration = -1;
     pAf->bitvector = value;
     pAf->next = pObj->affected;
     pObj->affected = pAf;

     send_to_char ( "Protect added.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_delaffect )
{
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *pAf;
     AFFECT_DATA        *pAf_next;
     char                affect[MAX_STRING_LENGTH];
     int                 value;
     int                 cnt = 0;

     EDIT_OBJ ( ch, pObj );

     one_argument ( argument, affect );

     if ( !is_number ( affect ) || affect[0] == '\0' )
     {
          send_to_char ( "Syntax:  delaffect [#affect]\n\r", ch );
          return FALSE;
     }

     value = atoi ( affect );

     if ( value < 0 )
     {
          send_to_char ( "Only non-negative affect-numbers allowed.\n\r", ch );
          return FALSE;
     }

     if ( !( pAf = pObj->affected ) )
     {
          send_to_char ( "OEdit:  Non-existant affect.\n\r", ch );
          return FALSE;
     }

     if ( value == 0 )		/* First case: Remove first affect */
     {
          pAf = pObj->affected;
          pObj->affected = pAf->next;
          free_affect ( pAf );
     }
     else			/* Affect to remove is not the first */
     {
          while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
               pAf = pAf_next;

          if ( pAf_next )		/* See if it's the next affect */
          {
               pAf->next = pAf_next->next;
               free_affect ( pAf_next );
          }
          else			/* Doesn't exist */
          {
               send_to_char ( "No such affect.\n\r", ch );
               return FALSE;
          }
     }
     send_to_char ( "Affect removed.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_comment )
{
     OBJ_INDEX_DATA *pObj;

     EDIT_OBJ ( ch, pObj );
     if ( argument[0] == '\0' )
     {
          string_append ( ch, &pObj->notes );
          return TRUE;
     }
     send_to_char ( "Syntax: comments\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_name )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  name [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pObj->name );
     pObj->name = str_dup ( argument );

     send_to_char ( "Name set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_repop )
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  repop [percentage]\n\r", ch );
          return FALSE;
     }
     value = atoi ( argument );
     if ( value < 1 || value > 100 )
     {
          send_to_char ( "Repop range is 1-100.\n\r", ch );
          return FALSE;
     }
     pObj->repop = value;
     send_to_char ( "Repop percentage set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_short )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  short [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pObj->short_descr );
     pObj->short_descr = str_dup ( argument );

     send_to_char ( "Short description set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_long )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  long [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pObj->description );
     pObj->description = str_dup ( argument );
     pObj->description[0] = UPPER ( pObj->description[0] );

     send_to_char ( "Long description set.\n\r", ch );
     return TRUE;
}

bool set_value ( CHAR_DATA * ch, OBJ_INDEX_DATA * pObj,
		 char *argument, int value )
{
     if ( argument[0] == '\0' )
     {
          set_obj_values ( ch, pObj, -1, "" );	/* '\0' changed to "" -- Hugin */
          return FALSE;
     }

     if ( set_obj_values ( ch, pObj, value, argument ) )
          return TRUE;

     return FALSE;
}

/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values ( CHAR_DATA * ch, char *argument, int value )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( set_value ( ch, pObj, argument, value ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_value0 )
{
     if ( oedit_values ( ch, argument, 0 ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_value1 )
{
     if ( oedit_values ( ch, argument, 1 ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_value2 )
{
     if ( oedit_values ( ch, argument, 2 ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_value3 )
{
     if ( oedit_values ( ch, argument, 3 ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_value4 )
{
     if ( oedit_values ( ch, argument, 4 ) )
          return TRUE;

     return FALSE;
}

OEDIT ( oedit_weight )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  weight [number]\n\r", ch );
          return FALSE;
     }

     pObj->weight = atoi ( argument );

     send_to_char ( "Weight set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_cost )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  cost [number]\n\r", ch );
          return FALSE;
     }

     pObj->cost = atoi ( argument );

     send_to_char ( "Cost set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_create )
{
     OBJ_INDEX_DATA     *pObj;
     AREA_DATA          *pArea;
     int                 value;
     int                 iHash;

     value = atoi ( argument );
     if ( argument[0] == '\0' || value == 0 )
     {
          send_to_char ( "Syntax:  oedit create [vnum]\n\r", ch );
          return FALSE;
     }

     pArea = get_vnum_area ( value );
     if ( !pArea )
     {
          send_to_char ( "OEdit:  That vnum is not assigned an area.\n\r", ch );
          return FALSE;
     }

     if ( !IS_BUILDER ( ch, pArea ) )
     {
          send_to_char ( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
          return FALSE;
     }

     if ( get_obj_index ( value ) )
     {
          send_to_char ( "OEdit:  Object vnum already exists.\n\r", ch );
          return FALSE;
     }

     pObj = new_obj_index (  );
     pObj->vnum = value;
     pObj->area = pArea;
     pObj->repop = 100;

     if ( value > top_vnum_obj )
          top_vnum_obj = value;

     iHash = value % MAX_KEY_HASH;
     pObj->next = obj_index_hash[iHash];
     obj_index_hash[iHash] = pObj;
     ch->desc->pEdit = ( void * ) pObj;

     send_to_char ( "Object Created.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_default )
{
     OBJ_INDEX_DATA     *pObj;
     AFFECT_DATA        *paf;

     int                 level;
     int                 number, type;	/* for dice-conversion */
     int                 countaff = 0;
     int                 countspell = 0;

     EDIT_OBJ ( ch, pObj );

     level = pObj->level;

     if ( level == 0 )
     {
          send_to_char ( "Set the object to desired level first.\n\r", ch );
          return FALSE;
     }

     pObj->level = UMAX ( 0, pObj->level );	/* just to be sure */

     for ( paf = pObj->affected; paf != NULL; paf = paf->next )
     {
          if ( !paf->bitvector )
          {
               switch ( paf->location )
               {
               default:
                    ++countaff;
                    break;
               case APPLY_SEX:	/* these have no cost affect */
               case APPLY_NONE:
               case APPLY_LEVEL:
               case APPLY_CLASS:
               case APPLY_HEIGHT:
               case APPLY_WEIGHT:
               case APPLY_GOLD:
               case APPLY_EXP:
                    break;
               case APPLY_STR:	/* these have 2 counters per */
               case APPLY_DEX:
               case APPLY_WIS:
               case APPLY_INT:
               case APPLY_CON:
               case APPLY_HITROLL:
               case APPLY_DAMROLL:
                    countaff += paf->modifier * 2;	/* 2 points per */
                    break;
               case APPLY_MANA:
               case APPLY_HIT:

               case APPLY_MOVE:
               case APPLY_AC:
                    countaff += paf->modifier * .2;	/* 1 point per 5 */
                    break;
               case APPLY_SAVING_PARA:
               case APPLY_SAVING_ROD:
               case APPLY_SAVING_PETRI:
               case APPLY_SAVING_BREATH:
               case APPLY_SAVING_SPELL:
                    ++countaff;
                    break;
               }
          }
          else
          {
/*	       if (paf->bitvector == AFF_SWIM) *//* to check for bitvector */
               ++countspell;
          }
     }

     pObj->cost = ( ( 50 * level ) + ( 400 * countaff ) + ( 5000 * countspell ) );

     switch ( pObj->item_type )
     {
     default:
          bugf ( "Obj_default: vnum %d bad type.", pObj->item_type );
          break;
     case ITEM_LIGHT:
     case ITEM_TREASURE:
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
     case ITEM_SCROLL:
     case ITEM_WAND:
     case ITEM_STAFF:
          break;
     case ITEM_WEAPON:

          number = (level / 15)+1;

          type = (number * 4) + ( (level - ( (number-1)*10 ))/3 );

          pObj->value[1] = number;
          pObj->value[2] = type;
          break;
     case ITEM_ARMOR:
          pObj->value[0] = level / 5 + 3;
          pObj->value[1] = pObj->value[0];
          pObj->value[2] = pObj->value[0];
          pObj->value[3] = pObj->value[0] - 3;
          break;

     case ITEM_FURNITURE:
          pObj->value[0] = 4;			/* sleep obj by default */
          pObj->value[1] = ( level + 1 ) / 10;	/* set default to 1% per 10 levels,
	 					 * the +1 is to avoid division by 0
	 					 * error on a level 0 object 
                                                 */
          pObj->value[2] = 1;			/* 1 person */
          break;
     case ITEM_POTION:
     case ITEM_PILL:
     case ITEM_MONEY:
          break;
     }

     send_to_char ( "Default Values Set", ch );
     return TRUE;
}

OEDIT ( oedit_ed )
{
     OBJ_INDEX_DATA     *pObj;
     EXTRA_DESCR_DATA   *ed;
     char                command[MAX_INPUT_LENGTH];
     char                keyword[MAX_INPUT_LENGTH];

     EDIT_OBJ ( ch, pObj );

     argument = one_argument ( argument, command );
     one_argument ( argument, keyword );

     if ( command[0] == '\0' )
     {
          send_to_char ( "Syntax:  ed add [keyword]\n\r", ch );
          send_to_char ( "         ed delete [keyword]\n\r", ch );
          send_to_char ( "         ed edit [keyword]\n\r", ch );
          send_to_char ( "         ed format [keyword]\n\r", ch );
          return FALSE;
     }

     if ( !str_cmp ( command, "add" ) )
     {
          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed add [keyword]\n\r", ch );
               return FALSE;
          }

          ed = new_extra_descr (  );
          ed->keyword = str_dup ( keyword );
          ed->next = pObj->extra_descr;
          pObj->extra_descr = ed;

          string_append ( ch, &ed->description );

          return TRUE;
     }

     if ( !str_cmp ( command, "edit" ) )
     {
          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed edit [keyword]\n\r", ch );
               return FALSE;
          }

          for ( ed = pObj->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
          }

          if ( !ed )
          {
               send_to_char ( "OEdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }
          string_append ( ch, &ed->description );
          return TRUE;
     }
     if ( !str_cmp ( command, "delete" ) )
     {
          EXTRA_DESCR_DATA   *ped = NULL;

          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed delete [keyword]\n\r", ch );
               return FALSE;
          }

          for ( ed = pObj->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
               ped = ed;
          }

          if ( !ed )
          {
               send_to_char ( "OEdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }

          if ( !ped )
               pObj->extra_descr = ed->next;
          else
               ped->next = ed->next;

          free_extra_descr ( ed );
          send_to_char ( "Extra description deleted.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "format" ) )
     {
          EXTRA_DESCR_DATA   *ped = NULL;

          if ( keyword[0] == '\0' )
          {
               send_to_char ( "Syntax:  ed format [keyword]\n\r", ch );
               return FALSE;
          }

          for ( ed = pObj->extra_descr; ed; ed = ed->next )
          {
               if ( is_name ( keyword, ed->keyword ) )
                    break;
               ped = ed;
          }

          if ( !ed )
          {
               send_to_char ( "OEdit:  Extra description keyword not found.\n\r", ch );
               return FALSE;
          }

          ed->description = format_string ( ed->description );

          send_to_char ( "Extra description formatted.\n\r", ch );
          return TRUE;
     }

     oedit_ed ( ch, "" );
     return FALSE;
}

OEDIT ( oedit_extra )		/* Moved out of oedit() due to naming conflicts -- Hugin */
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_OBJ ( ch, pObj );

          if ( ( value = flag_value ( extra_flags, argument ) ) != NO_FLAG )
          {
               TOGGLE_BIT ( pObj->extra_flags, value );
               send_to_char ( "Extra flag toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax:  extra [flag]\n\r"
                    "Type '? extra' for a list of flags.\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_wear )		/* Moved out of oedit() due to naming conflicts -- Hugin */
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_OBJ ( ch, pObj );

          if ( ( value = flag_value ( wear_flags, argument ) ) != NO_FLAG )
          {
               TOGGLE_BIT ( pObj->wear_flags, value );
               send_to_char ( "Wear flag toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax:  wear [flag]\n\r"
                    "Type '? wear' for a list of flags.\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_vflags )
{
     OBJ_INDEX_DATA	*pObj;
     int		 value;
     
     if ( argument[0] != '\0' )
     {
          EDIT_OBJ ( ch, pObj );
          
          switch ( pObj->item_type )
          {
          default:        
               send_to_char ( "This type of item does not use vflags.\n\r", ch );
               break;
          case ITEM_ARMOR:
               {
                    int i;	// Material type

                    if ( pObj->material == 0 || pObj->material == MATERIAL_UNIQUE )
                    {
                         send_to_char ( "Please set a valid object material first.\n\r", ch );
                         return FALSE;
                    }
                    
                    /* Finds the material type */
                    for ( i = 0 ; material_table[i].type != pObj->material; i++ )
                         ;         /* nothing */
                    
                    if ( ( value = flag_value ( vflags_armor, argument ) ) != NO_FLAG )
                    {
                         send_to_char ( "Toggling vflag bit.\n\r", ch );
                         /* switch by flag value */
                         switch ( value )
                         {
                         default:
                              TOGGLE_BIT ( pObj->vflags, value );
                              return TRUE;
                              /* Quality bits are exclusive */
                         case ARMOR_BAD_QUALITY:
                              REMOVE_BIT ( pObj->vflags, ARMOR_LOW_QUALITY );
                              REMOVE_BIT ( pObj->vflags, ARMOR_HIGH_QUALITY );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_BAD_QUALITY );
                              return TRUE;
                         case ARMOR_LOW_QUALITY:
                              REMOVE_BIT ( pObj->vflags, ARMOR_BAD_QUALITY );
                              REMOVE_BIT ( pObj->vflags, ARMOR_HIGH_QUALITY );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_LOW_QUALITY );
                              return TRUE;
                         case ARMOR_HIGH_QUALITY:
                              REMOVE_BIT ( pObj->vflags, ARMOR_LOW_QUALITY );
                              REMOVE_BIT ( pObj->vflags, ARMOR_BAD_QUALITY );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_HIGH_QUALITY );
                              return TRUE;
                              /* Next 4 construction bits are exclusive */
                         case ARMOR_BANDED:
                              if ( !IS_SET ( material_table[i].flags, MAT_METAL ) )
                              {
                                   send_to_char ( "This flag may only be set on metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_RING );
                              REMOVE_BIT ( pObj->vflags, ARMOR_SCALE );
                              REMOVE_BIT ( pObj->vflags, ARMOR_PLATE );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_BANDED );
                              return TRUE;
                         case ARMOR_RING:
                              if ( !IS_SET ( material_table[i].flags, MAT_METAL ) )
                              {
                                   send_to_char ( "This flag may only be set on metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_BANDED );
                              REMOVE_BIT ( pObj->vflags, ARMOR_SCALE );
                              REMOVE_BIT ( pObj->vflags, ARMOR_PLATE );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_RING );
                              return TRUE;
                         case ARMOR_SCALE:
                              if ( !IS_SET ( material_table[i].flags, MAT_METAL ) )
                              {
                                   send_to_char ( "This flag may only be set on metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_RING );
                              REMOVE_BIT ( pObj->vflags, ARMOR_BANDED );
                              REMOVE_BIT ( pObj->vflags, ARMOR_PLATE );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_SCALE );
                              return TRUE;
                         case ARMOR_PLATE:
                              if ( !IS_SET ( material_table[i].flags, MAT_METAL ) )
                              {
                                   send_to_char ( "This flag may only be set on metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_RING );
                              REMOVE_BIT ( pObj->vflags, ARMOR_SCALE );
                              REMOVE_BIT ( pObj->vflags, ARMOR_BANDED );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_PLATE );
                              return TRUE;
                              /* The next 2 are exclusive */
                         case ARMOR_SOFTENED:
                              if ( pObj->material != MATERIAL_LEATHER )
                              {
                                   send_to_char ( "Only leather can be softened.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_HARDENED );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_SOFTENED );
                              return TRUE;                                                            
                         case ARMOR_HARDENED:
                              if ( pObj->material != MATERIAL_LEATHER )
                              {
                                   send_to_char ( "Only leather can be hardened.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_SOFTENED );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_HARDENED );
                              return TRUE;
                         case ARMOR_STUDDED:
                              if ( pObj->material != MATERIAL_LEATHER )
                              {
                                   send_to_char ( "Only leather can be studded.\n\r", ch );
                                   return FALSE;
                              }
                              TOGGLE_BIT ( pObj->vflags, ARMOR_STUDDED );
                              return TRUE;
                              /* The next 2 are exlusive */
                         case ARMOR_CAST:
                              if ( !IS_SET ( material_table[i].flags, MAT_MELT_VHOT )                                   
                                   && !IS_SET ( material_table[i].flags, MAT_MELT_NORMAL ) )
                              {
                                   send_to_char ( "Only materials that can be melted and solidified through normal means can be cast.\n\r", ch );
                                   return FALSE;
                              }
                              if ( IS_SET ( material_table[i].flags, MAT_DISSOLVE )
                                   || IS_SET ( material_table[i].flags, MAT_LIVING )
                                   || IS_SET ( material_table[i].flags, MAT_LIQUID )
                                   || IS_SET ( material_table[i].flags, MAT_GASEOUS )
                                   || IS_SET ( material_table[i].flags, MAT_ETHEREAL )
                                   || IS_SET ( material_table[i].flags, MAT_FLAMMABLE )
                                   || IS_SET ( material_table[i].flags, MAT_VERYFLAMMABLE ) )
                              {
                                   send_to_char ( "That material cannot be cast.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_FORGED );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_CAST );
                              return TRUE;                                   
                         case ARMOR_FORGED:
                              if ( !IS_SET ( material_table[i].flags, MAT_FORGE ) )
                              {
                                   send_to_char ( "This flag requires a forgable metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              REMOVE_BIT ( pObj->vflags, ARMOR_CAST );
                              TOGGLE_BIT ( pObj->vflags, ARMOR_FORGED );
                              return TRUE;
                         case ARMOR_TEMPERED:
                              if ( !IS_SET ( material_table[i].flags, MAT_METAL ) )
                              { 
                                   send_to_char ( "This flag requires a metal armor.\n\r", ch );
                                   return FALSE;
                              }
                              TOGGLE_BIT ( pObj->vflags, ARMOR_TEMPERED );
                              return TRUE;                              
                         } 		/* End of switch value */
                    } 			/* end of if value */                    
               } 			/* end of ITEM_ARMOR */
          } 				/* End of switch type */
     } 					/* end of if argument */
     send_to_char ( "Syntax: vflags [flag]\n\r"
                    "    Use ? for help with the following types:\n\r"
                    "    vflags_armor\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_type )		/* Moved out of oedit() due to naming conflicts -- Hugin */
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_OBJ ( ch, pObj );

          if ( ( value = flag_value ( type_flags, argument ) ) != NO_FLAG )
          {
               pObj->item_type = value;
               send_to_char ( "Type set.\n\r", ch );

               /*
                * Clear the values.
                */
               pObj->value[0] = 0;
               pObj->value[1] = 0;
               pObj->value[2] = 0;
               pObj->value[3] = 0;
               pObj->value[4] = 0;

               return TRUE;
          }
     }
     send_to_char ( "Syntax:  type [flag]\n\r"
                    "Type '? type' for a list of flags.\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_material )
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_OBJ ( ch, pObj );

          if ( ( value = flag_value ( material_type, argument ) ) != NO_FLAG )
          {
               pObj->material = value;
               send_to_char ( "Material type set.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax:  material [material-name]\n\r"
                    "Type '? material' for a list of materials.\n\r", ch );
     return FALSE;
}

OEDIT ( oedit_level )
{
     OBJ_INDEX_DATA     *pObj;

     EDIT_OBJ ( ch, pObj );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  level [number]\n\r", ch );
          return FALSE;
     }

     if ( atoi ( argument ) > MAX_OBJ_LEVEL )
     {
          form_to_char ( ch, "Maximum object level is %d, command failed.\n\r", MAX_OBJ_LEVEL );
          return FALSE;
     }

     pObj->level = atoi ( argument );

     send_to_char ( "Level set.\n\r", ch );
     return TRUE;
}

OEDIT ( oedit_condition )
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( argument[0] != '\0' && ( value = atoi ( argument ) ) >= 0 && ( value <= 100 ) )
     {
          EDIT_OBJ ( ch, pObj );

          pObj->condition = value;
          send_to_char ( "Condition set.\n\r", ch );

          return TRUE;
     }

     send_to_char ( "Syntax:  condition [number]\n\r"
                    "Where number can range from 0 (ruined) to 100 (perfect).\n\r", ch );
     return FALSE;
}

/*
 * Mobile Editor Functions.
 */
MEDIT ( medit_unused )
{
     ROOM_INDEX_DATA    *pRoom;
     AREA_DATA          *pArea;
     MOB_INDEX_DATA     *tmp;
     BUFFER		*buffer;
     int                 counter;

     EDIT_ROOM ( ch, pRoom );
     pArea = pRoom->area;
     
     buffer = buffer_new ( 1000 );

     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );
     bprintf ( buffer, "UNUSED MOBILE VNUMS\n\r------ ------ -----\n\r");
     counter = pArea->lvnum;
     for ( ; ( counter <= pArea->uvnum ); counter++ )
     {
          if ( ( tmp = get_mob_index ( counter ) ) == NULL )
               bprintf ( buffer, "[%d]\n\r", counter );
     }
     
     page_to_char ( buffer->data, ch );
     buffer_free ( buffer );

     return TRUE;
}

/* Jeezus bejeezus, Zeran, why do you do things the hard way */
/* Well I'm about to make this 10 times as complicated - lotherius */

MEDIT ( medit_used )
{
     ROOM_INDEX_DATA    *pRoom;
     AREA_DATA          *pArea;
     MOB_INDEX_DATA     *tmp;
     BUFFER	 	*buffer;
     int                 counter, num;
     int                 total, uvnum;
     bool		 haveargs = FALSE;
     bool		 level, race, msize, act, hitdice, damdice, hr, imm, res, vuln, off, dt, spec, sex, gold;

     buffer = buffer_new(2000);

     EDIT_ROOM ( ch, pRoom );

	/* Let's skip lots of worry if we don't have any arguments. */
     if ( argument[0] != '\0' )
          haveargs = TRUE;

     pArea = pRoom->area;
     total = pArea->uvnum - pArea->lvnum;
     bprintf ( buffer, "Area name: %s\n\r", ( ( pArea->name ) ? ( pArea->name ) : "none" ) );

     bprintf ( buffer, "Mobs in This Area\n\r--------------\n\r" );

     counter = pArea->lvnum;
     uvnum = pArea->uvnum;

     /* Set the booleans... Long.. */
     if ( (haveargs) && !strcmp (argument, "level") ) /* Will only happen if level is the *only* argument */
     {
          level = TRUE;
          haveargs = FALSE;
     }

     /* Gotta set each one to false to keep the compiler quiet. */     
     if ( strstr (argument, "level" ) != NULL )		level = TRUE;
     else level = FALSE;
     if ( strstr (argument, "race" ) != NULL )		race = TRUE;
     else race = FALSE;
     if ( strstr (argument, "act" ) != NULL )		act = TRUE;
     else act = FALSE;
     if ( strstr (argument, "hitdice" ) != NULL)	hitdice = TRUE;
     else hitdice = FALSE;
     if ( strstr (argument, "damdice" ) != NULL)	damdice = TRUE;
     else damdice = FALSE;
     if ( strstr (argument, "hr" ) != NULL)		hr = TRUE;
     else hr = FALSE;
     if ( strstr (argument, "imm" ) != NULL )		imm = TRUE;
     else imm = FALSE;
     if ( strstr (argument, "res" ) != NULL )		res = TRUE;
     else res = FALSE;
     if ( strstr (argument, "vuln" ) != NULL )		vuln = TRUE;
     else vuln = FALSE;
     if ( strstr (argument, "off" ) != NULL )		off = TRUE;
     else off = FALSE;
     if ( strstr (argument, "dt" ) != NULL )		dt = TRUE;
     else dt = FALSE;
     if ( strstr (argument, "spec" ) != NULL )		spec = TRUE;
     else spec = FALSE;
     if ( strstr (argument, "sex" ) != NULL )		sex = TRUE;
     else sex = FALSE;
     if ( strstr (argument, "gold" ) != NULL )		gold = TRUE;
     else gold = FALSE;
     if ( strstr (argument, "size" ) != NULL )		msize = TRUE;
     else msize = FALSE;

     if ( level )
     {
          vlh_type           *hashlist[MAX_MOB_LEVEL + 1];
          vlh_type           *hash_tmp, *free_tmp = NULL;

          for ( num = 0; num <= MAX_MOB_LEVEL; num++ )
               hashlist[num] = NULL;

          for ( ; counter <= uvnum; counter++ )
          {
               if ( ( tmp = get_mob_index ( counter ) ) != NULL )
               {
                    if ( !hashlist[tmp->level] )
                    {
			 hashlist[tmp->level] = alloc_mem ( sizeof ( vlh_type ), "vlh_type" );
			 hashlist[tmp->level]->vnum = tmp->vnum;
                         hashlist[tmp->level]->next = NULL;
                    }
                    else
                    {
                         for ( hash_tmp = hashlist[tmp->level]; hash_tmp->next != NULL; hash_tmp = hash_tmp->next );
                                      /* Nothing */
			 hash_tmp->next = alloc_mem ( sizeof (vlh_type ), "vlh_type" );
                         hash_tmp->next->next = NULL;
                         hash_tmp->next->vnum = tmp->vnum;
                    }
               }
          } /* end for loop through vnums */         
          /* now print out list */
          for ( counter = 0; counter <= MAX_MOB_LEVEL; counter++ )
          {
               for ( hash_tmp = hashlist[counter]; hash_tmp != NULL; hash_tmp = hash_tmp->next )
               {
                    if ( !free_tmp )
                         free_mem  ( free_tmp, sizeof ( vlh_type ), "vlh_type" );
                    tmp = get_mob_index ( hash_tmp->vnum );
                    bprintf ( buffer, "{W[{Y%5d{W] [{G%3d{W] {C%s{w", tmp->vnum, tmp->level, ( ( tmp->short_descr )
                                                                      ? ( tmp->short_descr ) : "" ) );

                    if (haveargs)
                    {
                         bprintf (buffer, "\n\r        ");
                         if (race)        bprintf (buffer, "Rc[%10s] ", race_table[tmp->race].name );
                         if (sex)         bprintf (buffer, "Sx[%4s] ", tmp->sex == SEX_MALE ? "male" : 
                                                   tmp->sex == SEX_FEMALE ? "female" : 
                                                   tmp->sex == 3 ? "random" : "neutral" );
                         if (gold)        bprintf (buffer, "Gd[%6ld] ", tmp->gold);
                         if (dt)          bprintf (buffer, "Dt[%5s] ", flag_string ( attack_type, tmp->dam_type ) );
                         if (hitdice)     bprintf (buffer, "Hd[%2dd%-3d+%4d] ", tmp->hit[DICE_NUMBER], tmp->hit[DICE_TYPE],
                                                   tmp->hit[DICE_BONUS] );
                         if (damdice)     bprintf (buffer, "Dd[%2dd%-3d+%4d] ", tmp->damage[DICE_NUMBER], 
                                                   tmp->damage[DICE_TYPE], tmp->damage[DICE_BONUS] );
                         if (hr)          bprintf (buffer, "Hr[%3d] ", tmp->hitroll );
                         if (msize)       bprintf (buffer, "Sz[%5s] ", flag_string ( size_flags, tmp->size ) );
                         if (spec)        bprintf (buffer, "Sp[%10d] ", spec_string ( tmp->spec_fun ) );
                         if (act)         bprintf (buffer, "\n\r   Act: %s ", flag_string ( act_flags, tmp->act ) );
                         if (off)         bprintf (buffer, "\n\r   Off: %s ", flag_string ( off_flags, tmp->off_flags ) );
                         if (imm)         bprintf (buffer, "\n\r   Imm: %s ", flag_string ( imm_flags, tmp->imm_flags ) );
                         if (res)         bprintf (buffer, "\n\r   Res: %s ", flag_string ( res_flags, tmp->res_flags ) );
                         if (vuln)        bprintf (buffer, "\n\r   Vul: %s ", flag_string ( vuln_flags, tmp->vuln_flags ) );
                    }  /* End of haveargs */
                    bprintf (buffer, "\n\r");
                    free_tmp = hash_tmp;
               }
               if ( !free_tmp )
                    free_mem ( free_tmp, sizeof ( vlh_type ), "vlh_type" );
          } /* end level loop */
     } /* end -level block */
     else
     {
          for ( ; ( counter <= uvnum ); counter++ )
          {
               if ( ( tmp = get_mob_index ( counter ) ) != NULL )
               {
                    bprintf ( buffer, "{W[{Y%5d{W] [{G%3d{W] {C%s{w", tmp->vnum, tmp->level, ( ( tmp->short_descr )
                                                                      ? ( tmp->short_descr ) : "" ) );
                    if (haveargs)
                    {
                         bprintf (buffer, "\n\r        ");
                         if (race)         bprintf (buffer, "Rc[%10s] ", race_table[tmp->race].name );
                         if (sex)          bprintf (buffer, "Sx[%4s] ", tmp->sex == SEX_MALE ? "male" : tmp->sex ==
                                                    SEX_FEMALE ? "female" : tmp->sex == 3 ? "random" : "neutral" );
                         if (gold)         bprintf (buffer, "Gd[%6ld] ", tmp->gold);
                         if (dt)           bprintf (buffer, "Dt[%5s] ", flag_string ( attack_type, tmp->dam_type ) );
                         if (hitdice)      bprintf (buffer, "Hd[%2dd%-3d+%4d] ", tmp->hit[DICE_NUMBER], tmp->hit[DICE_TYPE],
                                                    tmp->hit[DICE_BONUS] );
                         if (damdice)      bprintf (buffer, "Dd[%2dd%-3d+%4d] ", tmp->damage[DICE_NUMBER], 
                                                    tmp->damage[DICE_TYPE], tmp->damage[DICE_BONUS] );
                         if (hr)           bprintf (buffer, "Hr[%3d] ", tmp->hitroll );
                         if (msize)        bprintf (buffer, "Sz[%5s] ", flag_string ( size_flags, tmp->size ) );
                         if (spec)         bprintf (buffer, "Sp[%10d] ", spec_string ( tmp->spec_fun ) );
                         if (act)          bprintf (buffer, "\n   Act: %s ", flag_string ( act_flags, tmp->act ) );
                         if (off)          bprintf (buffer, "\n   Off: %s ", flag_string ( off_flags, tmp->off_flags ) );
                         if (imm)          bprintf (buffer, "\n   Imm: %s ", flag_string ( imm_flags, tmp->imm_flags ) );
                         if (res)          bprintf (buffer, "\n   Res: %s ", flag_string ( res_flags, tmp->res_flags ) );
                         if (vuln)         bprintf (buffer, "\n   Vul: %s ", flag_string ( vuln_flags, tmp->vuln_flags ) );
                    } /* end of haveargs */
                    bprintf ( buffer, "\n" );
               }
          }
     }

     page_to_char(buffer->data, ch);
     buffer_free(buffer);

     return TRUE;
}

MEDIT ( medit_show )
{
     MOB_INDEX_DATA     *pMob;
     PROG_LIST *list;

     EDIT_MOB ( ch, pMob );

     form_to_char ( ch, "Name: [{C%s{w]  Area: [{C%5d{w] %s\n\r",
                    pMob->player_name,
                    !pMob->area ? -1 : pMob->area->vnum,
                    !pMob->area ? "No Area" : pMob->area->name );
     form_to_char ( ch, "Act: [{C%s{w]\n\r", flag_string ( act_flags, pMob->act ) );
     form_to_char ( ch, "Vnum: [{C%5d{w]  Sex: [{C%s{w]  Race: [{C%s{w]\n\r",
                    pMob->vnum,
                    pMob->sex == SEX_MALE ? "male" : pMob->sex ==
                    SEX_FEMALE ? "female" : pMob->sex ==
                    3 ? "random" : "neutral",
                    race_table[pMob->race].name );
     form_to_char ( ch, "Level: [{C%2d{w]  Align: [{C%4d{w]  Hitroll: [{C%d{w]\n\r",
                    pMob->level, pMob->alignment, pMob->hitroll );
     form_to_char ( ch, "Hit dice: [{C%2dd%-3d+%4d{w]  Damage dice: [{C%2dd%-3d+%4d{w]\n\r",
                    pMob->hit[DICE_NUMBER], pMob->hit[DICE_TYPE],
                    pMob->hit[DICE_BONUS], pMob->damage[DICE_NUMBER],
                    pMob->damage[DICE_TYPE], pMob->damage[DICE_BONUS] );
     form_to_char ( ch, "Mana dice:   [{C%2dd%-3d+%4d{w] (currently unused)\n\r",
                    pMob->mana[DICE_NUMBER], pMob->mana[DICE_TYPE],
                    pMob->mana[DICE_BONUS] );
     form_to_char ( ch, "Affected by: [{C%s{w]\n\r", affect_bit_name ( pMob->affected_by ) );
     form_to_char ( ch, "Detecting:   [{C%s{w]\n\r", detect_bit_name ( pMob->detections ) );
     form_to_char ( ch, "Protections: [{C%s{w]\n\r", protect_bit_name ( pMob->protections ) );
     form_to_char ( ch, "Form: [{C%s{w]\n\r", flag_string ( form_flags, pMob->form ) );
     form_to_char ( ch, "Parts: [{C%s{w]\n\r", flag_string ( part_flags, pMob->parts ) );
     form_to_char ( ch, "Imm: [{C%s{w]\n\r", flag_string ( imm_flags, pMob->imm_flags ) );
     form_to_char ( ch, "Res: [{C%s{w]\n\r", flag_string ( res_flags, pMob->res_flags ) );
     form_to_char ( ch, "Vuln: [{C%s{w]\n\r", flag_string ( vuln_flags, pMob->vuln_flags ) );
     form_to_char ( ch, "Off: [{C%s{w]\n\r", flag_string ( off_flags, pMob->off_flags ) );
     form_to_char ( ch, "Dam Type: [{C%s{w]", flag_string ( attack_type, pMob->dam_type ) );
     form_to_char ( ch, "  Size: [{C%s{w]  Gold: [{C%5ld{w]\n\r", flag_string ( size_flags, pMob->size ), pMob->gold );
     form_to_char ( ch, "Start pos: [{C%s{w]  Default pos: [{C%s{w]\n\r", flag_string ( position_flags, pMob->start_pos ),
                    flag_string ( position_flags, pMob->default_pos ) );
     if ( pMob->spec_fun )
          form_to_char ( ch, "Spec fun: [{C%s{w]  ", spec_string ( pMob->spec_fun ) );
     if ( IS_SET ( pMob->act, ACT_SKILLMASTER ) )
     {
          int                 count;
          send_to_char ( "Teach: ", ch );
          if ( pMob->total_teach_skills > 0 )
          {
               for ( count = 0; count < pMob->total_teach_skills; count++ )              
                    form_to_char ( ch, "[{C%s{x] ", pMob->teach_skills[count] );
               send_to_char ( "\r\n", ch );
          }
          else          
               send_to_char ( "[{CNone{x]\r\n", ch );          
     }

     form_to_char ( ch, "Short descr: {C%s{w\n\rLong descr:\n\r{C%s{w\n\r", pMob->short_descr, pMob->long_descr );
     form_to_char ( ch, "Description:\n\r{C%s{w", pMob->description );

     if ( pMob->pShop )
     {
          SHOP_DATA          *pShop;
          int                 iTrade;

          pShop = pMob->pShop;

          form_to_char ( ch, 
                         "Shop data for [%5d]:\n\r"
                         "  Markup for purchaser: %d%%\n\r"
                         "  Markdown for seller:  %d%%\n\r",
                         pShop->keeper, pShop->profit_buy,
                         pShop->profit_sell );          
          form_to_char ( ch, "  Hours: %d to %d.\n\r", pShop->open_hour, pShop->close_hour );

          for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
          {
               if ( pShop->buy_type[iTrade] != 0 )
               {
                    if ( iTrade == 0 )
                    {
                         send_to_char ( "  Number Trades Type\n\r", ch );
                         send_to_char ( "  ------ -----------\n\r", ch );
                    }
                    form_to_char ( ch, "  [%4d] %s\n\r", iTrade,
                                   flag_string ( type_flags, pShop->buy_type[iTrade] ) );
               }
          }
     }
     if ( pMob->mprogs )
     {
          int cnt;

          form_to_char ( ch, "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);
          for (cnt=0, list=pMob->mprogs; list; list=list->next)
          {
               if (cnt ==0)
               {
                    send_to_char ( " Number Vnum Trigger Phrase\n\r", ch );
                    send_to_char ( " ------ ---- ------- ------\n\r", ch );
               }
               form_to_char ( ch, "[%5d] %4d %7s %s\n\r", cnt,
                              list->vnum, prog_type_to_name(list->trig_type),
                              list->trig_phrase);
               cnt++;
          }
     }

     return FALSE;
}

MEDIT ( medit_create )
{
     MOB_INDEX_DATA     *pMob;
     AREA_DATA          *pArea;
     int                 value;
     int                 iHash;

     value = atoi ( argument );
     if ( argument[0] == '\0' || value == 0 )
     {
          send_to_char ( "Syntax:  medit create [vnum]\n\r", ch );
          return FALSE;
     }

     pArea = get_vnum_area ( value );

     if ( !pArea )
     {
          send_to_char ( "MEdit:  That vnum is not assigned an area.\n\r", ch );
          return FALSE;
     }

     if ( !IS_BUILDER ( ch, pArea ) )
     {
          send_to_char ( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
          return FALSE;
     }

     if ( get_mob_index ( value ) )
     {
          send_to_char ( "MEdit:  Mobile vnum already exists.\n\r", ch );
          return FALSE;
     }
     pMob = new_mob_index (  );
     pMob->vnum = value;
     pMob->area = pArea;

     if ( value > top_vnum_mob )
          top_vnum_mob = value;

     pMob->act = ACT_IS_NPC;
     iHash = value % MAX_KEY_HASH;
     pMob->next = mob_index_hash[iHash];
     mob_index_hash[iHash] = pMob;
     ch->desc->pEdit = ( void * ) pMob;
     
     send_to_char ( "Mobile Created.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_default )
{
     int                 type, number, bonus;
     int                 level;
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( pMob->level == 0 )
     {
          send_to_char ( "Set the mobile to desired level first.\n\r", ch );
          return FALSE;
     }

     level = pMob->level;

    /*
     * Calculate hit dice.  Gives close to the hitpoints
     * of old format mobs created with create_mobile()  (db.c)
     * A high number of dice makes for less variance in mobiles
     * hitpoints.
     * (might be a good idea to reduce the max number of dice)
     *
     * The conversion below gives:

     level:     dice         min         max        diff       mean
     1:       1d2+6       7(  7)     8(   8)     1(   1)     8(   8)
     2:       1d3+15     16( 15)    18(  18)     2(   3)    17(  17)
     3:       1d6+24     25( 24)    30(  30)     5(   6)    27(  27)
     5:      1d17+42     43( 42)    59(  59)    16(  17)    51(  51)
     10:      3d22+96     99( 95)   162( 162)    63(  67)   131(    )
     15:     5d30+161    166(159)   311( 311)   145( 150)   239(    )
     30:    10d61+416    426(419)  1026(1026)   600( 607)   726(    )
     50:    10d169+920   930(923)  2610(2610)  1680(1688)  1770(    )

     The values in parenthesis give the values generated in create_mobile.
     Diff = max - min.  Mean is the arithmetic mean.
     (hmm.. must be some roundoff error in my calculations.. smurfette got
     1d6+23 hp at level 3 ? -- anyway.. the values above should be
     approximately right..)

     */

     type = level * level * 27 / 40;
     number = UMIN ( type / 40 + 1, 10 );	/* how do they get 11 ??? */
     type = UMAX ( 2, type / number );
     bonus = UMAX ( 0, level * ( 8 + level ) * .9 - number * type );
     number = number * 1.7;

     pMob->hit[DICE_NUMBER] = number;
     pMob->hit[DICE_TYPE] = type;
     pMob->hit[DICE_BONUS] = bonus;

     pMob->mana[DICE_NUMBER] = level;
     pMob->mana[DICE_TYPE] = 10;
     pMob->mana[DICE_BONUS] = 100;

     if ( pMob->level < 50 )
          pMob->hitroll = ( pMob->level / 2 );
     else
          pMob->hitroll = pMob->level;

    /*
     * Calculate dam dice.  Gives close to the damage
     * of old format mobs in damage()  (fight.c)
     */

     /*    type   = level*7/4;
      *    number = UMIN(type/4 + 1, 5);
      *    type   = UMAX(2, type/number);
      * 
      */

     /* Lotherius - old mob damage amounts adjusted */
     /* Yet another new mob damage calculation */
     /* Similar to object calculation, but weaker, so disarm means something */

     number = (level / 15)+1;
     type = (number * 3) + ( (level - ( (number-1)*10 ))/3 );
     bonus = 0;

     //    number = UMIN ( level / 4 + 1, 5 );
     //    type = ( level + 7 ) / number;
     //    bonus = UMAX ( 0, level * 9 / 4 - number * type );
     //
     pMob->damage[DICE_NUMBER] = number;
     pMob->damage[DICE_TYPE] = type;
     pMob->damage[DICE_BONUS] = bonus;

     switch ( number_range ( 1, 3 ) )
     {
     case ( 1 ):
          pMob->dam_type = 3;
          break;			/* slash  */
     case ( 2 ):
          pMob->dam_type = 7;
          break;			/* pound  */
     case ( 3 ):
          pMob->dam_type = 11;
          break;			/* pierce */
     }

     pMob->gold = ( pMob->level * 20 );
     
     send_to_char ( "Default Values Set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_spec )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  spec [special function]\n\r", ch );
          return FALSE;
     }

     if ( !str_cmp ( argument, "none" ) )
     {
          pMob->spec_fun = NULL;
          send_to_char ( "Spec removed.\n\r", ch );
          return TRUE;
     }

     if ( spec_lookup ( argument ) )
     {
          pMob->spec_fun = spec_lookup ( argument );
          send_to_char ( "Spec set.\n\r", ch );
          return TRUE;
     }

     send_to_char ( "MEdit: No such special function.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_teach )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  teach [skill]\n\r", ch );
          return FALSE;
     }

     if ( skill_lookup ( argument ) == -1 )
     {
          send_to_char ( "No such skill exists.\n\r", ch );
          return FALSE;
     }

    /* Go ahead and make sure ACT_SKILLMASTER is set */
     if ( !IS_SET ( pMob->act, ACT_SKILLMASTER ) )
          SET_BIT ( pMob->act, ACT_SKILLMASTER );
     if ( pMob->total_teach_skills >= MAX_TEACH_SKILLS )
     {
          send_to_char ( "This mob has the max number of skills.\n\r", ch );
          return FALSE;
     }

     pMob->teach_skills[pMob->total_teach_skills] = str_dup ( argument );
     pMob->total_teach_skills++;

     send_to_char ( "Skill added.\n\r", ch );
     return TRUE;

}

MEDIT ( medit_align )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  alignment [number]\n\r", ch );
          return FALSE;
     }
     pMob->alignment = atoi ( argument );
     send_to_char ( "Alignment set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_level )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  level [number]\n\r", ch );
          return FALSE;
     }

     if ( atoi ( argument ) > MAX_MOB_LEVEL )
     {
          form_to_char ( ch, "Mob maximum level is %d, command failed.\n\r", MAX_MOB_LEVEL );
          return FALSE;
     }

     pMob->level = atoi ( argument );

     send_to_char ( "Level set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_comment )
{
     MOB_INDEX_DATA *pMob;

     EDIT_MOB ( ch, pMob );
     if ( argument[0] == '\0' )
     {
          string_append ( ch, &pMob->notes );
          return TRUE;
     }
     send_to_char ( "Syntax: comments\n\r", ch );
     return FALSE;
}

MEDIT ( medit_desc )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          string_append ( ch, &pMob->description );
          return TRUE;
     }

     send_to_char ( "Syntax:  desc    - line edit\n\r", ch );
     return FALSE;
}

MEDIT ( medit_long )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  long [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pMob->long_descr );
     strcat ( argument, "\n\r" );
     pMob->long_descr = str_dup ( argument );
     pMob->long_descr[0] = UPPER ( pMob->long_descr[0] );

     send_to_char ( "Long description set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_short )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  short [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pMob->short_descr );
     pMob->short_descr = str_dup ( argument );

     send_to_char ( "Short description set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_name )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  name [string]\n\r", ch );
          return FALSE;
     }

     free_string ( pMob->player_name );
     pMob->player_name = str_dup ( argument );

     send_to_char ( "Name set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_shop )
{
     MOB_INDEX_DATA     *pMob;
     char                command[MAX_INPUT_LENGTH];
     char                arg1[MAX_INPUT_LENGTH];

     argument = one_argument ( argument, command );
     argument = one_argument ( argument, arg1 );

     EDIT_MOB ( ch, pMob );

     if ( command[0] == '\0' )
     {
          send_to_char ( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
          send_to_char ( "         shop profit [#buying%] [#selling%]\n\r", ch );
          send_to_char ( "         shop type [#0-4] [item type]\n\r", ch );
          send_to_char ( "         shop delete [#0-4]\n\r", ch );
          return FALSE;
     }

     if ( !str_cmp ( command, "hours" ) )
     {
          if ( arg1[0] == '\0' || !is_number ( arg1 )
               || argument[0] == '\0' || !is_number ( argument ) )
          {
               send_to_char ( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
               return FALSE;
          }

          if ( !pMob->pShop )
          {
               pMob->pShop = new_shop (  );
               pMob->pShop->keeper = pMob->vnum;
               shop_last->next = pMob->pShop;
          }

          pMob->pShop->open_hour = atoi ( arg1 );
          pMob->pShop->close_hour = atoi ( argument );

          send_to_char ( "Shop hours set.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "profit" ) )
     {
          if ( arg1[0] == '\0' || !is_number ( arg1 )
               || argument[0] == '\0' || !is_number ( argument ) )
          {
               send_to_char ( "Syntax:  shop profit [#buying%] [#selling%]\n\r", ch );
               return FALSE;
          }

          if ( !pMob->pShop )
          {
               pMob->pShop = new_shop (  );
               pMob->pShop->keeper = pMob->vnum;
               shop_last->next = pMob->pShop;
          }

          pMob->pShop->profit_buy = atoi ( arg1 );
          pMob->pShop->profit_sell = atoi ( argument );

          send_to_char ( "Shop profit set.\n\r", ch );
          return TRUE;
     }

     if ( !str_cmp ( command, "type" ) )
     {
          int                 value;

          if ( arg1[0] == '\0' || !is_number ( arg1 )
               || argument[0] == '\0' )
          {
               send_to_char ( "Syntax:  shop type [#0-4] [item type]\n\r", ch );
               return FALSE;
          }
          if ( atoi ( arg1 ) >= MAX_TRADE )
          {
               form_to_char ( ch, "REdit:  May sell %d items max.\n\r", MAX_TRADE );
               return FALSE;
          }
          if ( ( value = flag_value ( type_flags, argument ) ) == NO_FLAG )
          {
               send_to_char ( "REdit:  That type of item is not known.\n\r", ch );
               return FALSE;
          }
          if ( !pMob->pShop )
          {
               pMob->pShop = new_shop (  );
               pMob->pShop->keeper = pMob->vnum;
               shop_last->next = pMob->pShop;
          }
          pMob->pShop->buy_type[atoi ( arg1 )] = value;
          send_to_char ( "Shop type set.\n\r", ch );
          return TRUE;
     }
     if ( !str_cmp ( command, "delete" ) )
     {
          SHOP_DATA          *pShop;
          SHOP_DATA          *pShop_next;
          int                 value;
          int                 cnt = 0;

          if ( arg1[0] == '\0' || !is_number ( arg1 ) )
          {
               send_to_char ( "Syntax:  shop delete [#0-4]\n\r", ch );
               return FALSE;
          }

          value = atoi ( argument );

          if ( !pMob->pShop )
          {
               send_to_char ( "REdit:  Non-existant shop.\n\r", ch );
               return FALSE;
          }

          if ( value == 0 )
          {
               pShop = pMob->pShop;
               pMob->pShop = pMob->pShop->next;
               free_shop ( pShop );
          }
          else
               for ( pShop = pMob->pShop, cnt = 0; pShop;
                     pShop = pShop_next, cnt++ )
               {
                    pShop_next = pShop->next;
                    if ( cnt + 1 == value )
                    {
                         pShop->next = pShop_next->next;
                         free_shop ( pShop_next );
                         break;
                    }
               }
          send_to_char ( "Shop deleted.\n\r", ch );
          return TRUE;
     }
     medit_shop ( ch, "" );
     return FALSE;
}

MEDIT ( medit_sex )		/* Moved out of medit() due to naming conflicts -- Hugin */
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value ( sex_flags, argument ) ) != NO_FLAG )
          {
               pMob->sex = value;
               send_to_char ( "Sex set.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: sex [sex]\n\r"
                    "Type '? sex' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_act )		/* Moved out of medit() due to naming conflicts -- Hugin */
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value ( act_flags, argument ) ) != NO_FLAG )
          {
               pMob->act ^= value;
               SET_BIT ( pMob->act, ACT_IS_NPC );
               send_to_char ( "Act flag toggled.\n\r", ch );
               return TRUE;
          }
     }
     send_to_char ( "Syntax: act [flag]\n\r"
                    "Type '? act' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_affect )		/* Moved out of medit() due to naming conflicts -- Hugin */
{
     MOB_INDEX_DATA     *pMob;
     int			value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
          {
               pMob->affected_by ^= value;
               send_to_char ( "Affect flag toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: affect [flag]\n\r", ch );
     send_to_char ( "Valid affects are:\n\r", ch );
     show_affect_help ( ch );
     return FALSE;
}

MEDIT ( medit_detect )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value( detect_flags, argument ) ) != NO_FLAG )
          {
               pMob->detections ^= value;
               send_to_char ( "Detect flag toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: detect [flag]\n\r", ch );
     send_to_char ( "Valid detections are:\n\r", ch );
     show_detect_help ( ch );
     return FALSE;
}

MEDIT ( medit_protect )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value( protect_flags, argument ) ) != NO_FLAG )
          {
               pMob->protections ^= value;
               send_to_char ( "Protect flag toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: protect [flag]\n\r", ch );
     send_to_char ( "Valid protections are:\n\r", ch );
     show_protect_help ( ch );
     return FALSE;
}

MEDIT ( medit_form )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( form_flags, argument ) ) != NO_FLAG )
          {
               pMob->form ^= value;
               send_to_char ( "Form toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: form [flags]\n\r"
                    "Type '? form' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_part )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( part_flags, argument ) ) != NO_FLAG )
          {
               pMob->parts ^= value;
               send_to_char ( "Parts toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: part [flags]\n\r"
                    "Type '? part' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_imm )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( imm_flags, argument ) ) != NO_FLAG )
          {
               pMob->imm_flags ^= value;
               send_to_char ( "Immunity toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: imm [flags]\n\r"
                    "Type '? imm' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_res )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( res_flags, argument ) ) != NO_FLAG )
          {
               pMob->res_flags ^= value;
               send_to_char ( "Resistance toggled.\n\r", ch );
               return TRUE;
          }
     }
     send_to_char ( "Syntax: res [flags]\n\r"
                    "Type '? res' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_vuln )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( vuln_flags, argument ) ) != NO_FLAG )
          {
               pMob->vuln_flags ^= value;
               send_to_char ( "Vulnerability toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: vuln [flags]\n\r"
                    "Type '? vuln' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_material )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;
     EDIT_MOB ( ch, pMob );
     if ( argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  material [material-name]\n\r"
                         "Type '? material' for a list of materials.\n\r", ch );
          return FALSE;
     }
     if ( ( value = flag_value ( material_type, argument ) ) != NO_FLAG )
     {
          pMob->material = value;
          send_to_char ( "Material type set.\n\r", ch );
          return TRUE;
     }
     send_to_char ( "Unknown material type, '? material' for a list.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_off )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value ( off_flags, argument ) ) != NO_FLAG )
          {
               pMob->off_flags ^= value;
               send_to_char ( "Offensive behaviour toggled.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: off [flags]\n\r"
                    "Type '? off' for a list of flags.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_attack )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value = flag_value ( attack_type, argument ) ) != NO_FLAG )
          {
               pMob->dam_type = value;
               send_to_char ( "Damage type set.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: Damtype [type]\n\r"
                    "Type '? damtype' for a list of damage types.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_size )
{
     MOB_INDEX_DATA     *pMob;
     int                 value;

     if ( argument[0] != '\0' )
     {
          EDIT_MOB ( ch, pMob );

          if ( ( value =
                 flag_value ( size_flags, argument ) ) != NO_FLAG )
          {
               pMob->size = value;
               send_to_char ( "Size set.\n\r", ch );
               return TRUE;
          }
     }

     send_to_char ( "Syntax: size [size]\n\r"
                    "Type '? size' for a list of sizes.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_hitdice )
{
     static char         syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
     char               *num, *type, *bonus, *cp;
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     num = cp = argument;
     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     type = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     bonus = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     if ( *cp != '\0' )
          *cp = '\0';

     if ( ( !is_number ( num ) || atoi ( num ) < 1 )
          || ( !is_number ( type ) || atoi ( type ) < 1 )
          || ( !is_number ( bonus ) || atoi ( bonus ) < 0 ) )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     pMob->hit[DICE_NUMBER] = atoi ( num );
     pMob->hit[DICE_TYPE] = atoi ( type );
     pMob->hit[DICE_BONUS] = atoi ( bonus );

     send_to_char ( "Hitdice set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_manadice )
{
     static char         syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
     char               *num, *type, *bonus, *cp;
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     num = cp = argument;

     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     type = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     bonus = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     if ( *cp != '\0' )
          *cp = '\0';

     if ( !( is_number ( num ) && is_number ( type ) &&
            is_number ( bonus ) ) )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     if ( ( !is_number ( num ) || atoi ( num ) < 1 )
          || ( !is_number ( type ) || atoi ( type ) < 1 )
          || ( !is_number ( bonus ) || atoi ( bonus ) < 0 ) )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     pMob->mana[DICE_NUMBER] = atoi ( num );
     pMob->mana[DICE_TYPE] = atoi ( type );
     pMob->mana[DICE_BONUS] = atoi ( bonus );

     send_to_char ( "Manadice set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_damdice )
{
     static char         syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
     char               *num, *type, *bonus, *cp;
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     num = cp = argument;

     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     type = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     while ( *cp != '\0' && !isdigit ( *cp ) )
          *( cp++ ) = '\0';

     bonus = cp;

     while ( isdigit ( *cp ) )
          ++cp;
     if ( *cp != '\0' )
          *cp = '\0';

     if ( !
          ( is_number ( num ) && is_number ( type ) &&
            is_number ( bonus ) ) )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     if ( ( !is_number ( num ) || atoi ( num ) < 1 )
          || ( !is_number ( type ) || atoi ( type ) < 1 )
          || ( !is_number ( bonus ) || atoi ( bonus ) < 0 ) )
     {
          send_to_char ( syntax, ch );
          return FALSE;
     }

     pMob->damage[DICE_NUMBER] = atoi ( num );
     pMob->damage[DICE_TYPE] = atoi ( type );
     pMob->damage[DICE_BONUS] = atoi ( bonus );

     send_to_char ( "Damdice set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_race )
{
     MOB_INDEX_DATA     *pMob;
     int                 race;

     if ( argument[0] != '\0' && ( race = race_lookup ( argument ) ) != 0 )
     {
          EDIT_MOB ( ch, pMob );
          pMob->race = race;
          pMob->off_flags |= race_table[race].off;
          pMob->imm_flags |= race_table[race].imm;
          pMob->res_flags |= race_table[race].res;
          pMob->vuln_flags |= race_table[race].vuln;
          pMob->form |= race_table[race].form;
          pMob->parts |= race_table[race].parts;
          send_to_char ( "Race set.\n\r", ch );
          return TRUE;
     }

     if ( argument[0] == '?' )
     {
          send_to_char ( "Available races are:", ch );

          for ( race = 0; race_table[race].name != NULL; race++ )
          {
               if ( ( race % 3 ) == 0 )
                    send_to_char ( "\n\r", ch );
               form_to_char ( ch, " %-15s", race_table[race].name );
          }

          send_to_char ( "\n\r", ch );
          return FALSE;
     }

     send_to_char ( "Syntax:  race [race]\n\r"
                    "Type 'race ?' for a list of races.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_position )
{
     MOB_INDEX_DATA     *pMob;
     char                arg[MAX_INPUT_LENGTH];
     int                 value;

     argument = one_argument ( argument, arg );

     switch ( tolower ( arg[0] ) )
     {
     default:
          break;
     case 's':
          if ( str_prefix ( arg, "start" ) )
               break;
          if ( ( value = flag_value ( position_flags, argument ) ) == NO_FLAG )
               break;
          EDIT_MOB ( ch, pMob );
          pMob->start_pos = value;
          send_to_char ( "Start position set.\n\r", ch );
          return TRUE;
     case 'd':
          if ( str_prefix ( arg, "default" ) )
               break;
          if ( ( value = flag_value ( position_flags, argument ) ) == NO_FLAG )
               break;
          EDIT_MOB ( ch, pMob );
          pMob->default_pos = value;
          send_to_char ( "Default position set.\n\r", ch );
          return TRUE;
     }

     send_to_char
          ( "Syntax:  position [start/default] [position]\n\r"
            "Type '? position' for a list of positions.\n\r", ch );
     return FALSE;
}

MEDIT ( medit_gold )
{
     MOB_INDEX_DATA     *pMob;

     EDIT_MOB ( ch, pMob );

     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  gold [number]\n\r", ch );
          return FALSE;
     }
     pMob->gold = atoi ( argument );
     send_to_char ( "Gold set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_hitroll )
{
     MOB_INDEX_DATA     *pMob;
     EDIT_MOB ( ch, pMob );
     if ( argument[0] == '\0' || !is_number ( argument ) )
     {
          send_to_char ( "Syntax:  hitroll [number]\n\r", ch );
          return FALSE;
     }
     pMob->hitroll = atoi ( argument );
     send_to_char ( "Hitroll set.\n\r", ch );
     return TRUE;
}

MEDIT ( medit_addmprog )
{
     int 		value;
     MOB_INDEX_DATA    *pMob;
     PROG_LIST 	       *list;
     PROG_CODE 	       *code;
     char 		trigger[MAX_STRING_LENGTH];
     char 		phrase[MAX_STRING_LENGTH];
     char 		num[MAX_STRING_LENGTH];

     EDIT_MOB(ch, pMob);
     argument=one_argument(argument, num);
     argument=one_argument(argument, trigger);
     argument=one_argument(argument, phrase);

     if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
     {
          send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
          return FALSE;
     }

     if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
     {
          send_to_char("Valid flags are:\n\r",ch);
          show_help( ch, "mprog");
          return FALSE;
     }

     if ( ( code =get_prog_index (atoi(num), PRG_MPROG ) ) == NULL)
     {
          send_to_char("No such MOBProgram.\n\r",ch);
          return FALSE;
     }

     list                  = new_mprog();
     list->vnum            = atoi(num);
     list->trig_type       = value;
     list->trig_phrase     = str_dup(phrase);
     list->code            = code->code;
     SET_BIT(pMob->mprog_flags,value);
     list->next            = pMob->mprogs;
     pMob->mprogs          = list;

     send_to_char( "Mprog Added.\n\r",ch);
     return TRUE;
}

MEDIT ( medit_delmprog )
{
     MOB_INDEX_DATA    *pMob;
     PROG_LIST 	       *list;
     PROG_LIST 	       *list_next;
     char 		mprog[MAX_STRING_LENGTH];
     int 		value;
     int 		cnt = 0;

     EDIT_MOB(ch, pMob);
     one_argument( argument, mprog );
     if (!is_number( mprog ) || mprog[0] == '\0' )
     {
          send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
          return FALSE;
     }
     value = atoi ( mprog );
     if ( value < 0 )
     {
          send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
          return FALSE;
     }
     if ( !(list= pMob->mprogs) )
     {
          send_to_char("MEdit:  Non existant mprog.\n\r",ch);
          return FALSE;
     }
     if ( value == 0 )
     {
          REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
          list = pMob->mprogs;
          pMob->mprogs = list->next;
          free_mprog( list );
     }
     else
     {
          while ( (list_next = list->next) && (++cnt < value ) )
               list = list_next;
          if ( list_next )
          {
               REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
               list->next = list_next->next;
               free_mprog(list_next);
          }
          else
          {
               send_to_char("No such mprog.\n\r",ch);
               return FALSE;
          }
     }

     send_to_char("Mprog removed.\n\r", ch);
     return TRUE;
}

void do_end ( CHAR_DATA * ch, char *arg )
{
     ch->desc->editor = 0;
     return;
}

void show_affect_help ( CHAR_DATA * ch )
{
     int                 count = 0;
     int                 col;

     col = 0;
     for ( count = 0; affect_flags[count].name[0] != '\0'; count++ )
     {
          if ( affect_flags[count].settable )
          {
               form_to_char ( ch, "%-19.18s", affect_flags[count].name );
               if ( ++col % 4 == 0 )
                    send_to_char ( "\n\r", ch );
          }
     }

     if ( col % 4 != 0 )
          send_to_char ( "\n\r", ch );
     return;
}

void show_detect_help ( CHAR_DATA * ch )
{
     int                 count = 0;
     int                 col;

     col = 0;
     for ( count = 0; detect_flags[count].name[0] != '\0'; count++ )
     {
          if ( detect_flags[count].settable )
          {
               form_to_char ( ch, "%-19.18s", detect_flags[count].name );
               if ( ++col % 4 == 0 )
                    send_to_char ( "\n\r", ch );
          }
     }

     if ( col % 4 != 0 )
          send_to_char ( "\n\r", ch );
     return;
}

void show_protect_help ( CHAR_DATA * ch )
{
     int                 count = 0;
     int                 col;

     col = 0;
     for ( count = 0; protect_flags[count].name[0] != '\0'; count++ )
     {
          if ( protect_flags[count].settable )
          {
               form_to_char ( ch, "%-19.18s", protect_flags[count].name );
               if ( ++col % 4 == 0 )
                    send_to_char ( "\n\r", ch );
          }
     }
     if ( col % 4 != 0 )
          send_to_char ( "\n\r", ch );
     return;
}

OEDIT ( oedit_addoprog )
{
     int 		value;
     OBJ_INDEX_DATA    *pObj;
     PROG_LIST 	       *list;
     PROG_CODE 	       *code;
     char 		trigger[MAX_STRING_LENGTH];
     char 		phrase[MAX_STRING_LENGTH];
     char 		num[MAX_STRING_LENGTH];

     EDIT_OBJ(ch, pObj);
     argument=one_argument(argument, num);
     argument=one_argument(argument, trigger);
     argument=one_argument(argument, phrase);

     if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
     {
          send_to_char("Syntax:   addoprog [vnum] [trigger] [phrase]\n\r",ch);
          return FALSE;
     }

     if ( (value = flag_value (oprog_flags, trigger) ) == NO_FLAG)
     {
          send_to_char("Valid flags are:\n\r",ch);
          show_help( ch, "oprog");
          return FALSE;
     }

     if ( ( code =get_prog_index (atoi(num), PRG_OPROG ) ) == NULL)
     {
          send_to_char("No such OBJProgram.\n\r",ch);
          return FALSE;
     }

     list                  = new_oprog();
     list->vnum            = atoi(num);
     list->trig_type       = value;
     list->trig_phrase     = str_dup(phrase);
     list->code            = code->code;
     SET_BIT(pObj->oprog_flags, value);
     list->next            = pObj->oprogs;
     pObj->oprogs          = list;

     send_to_char( "Oprog Added.\n\r",ch);
     return TRUE;
}

OEDIT ( oedit_deloprog )
{
     OBJ_INDEX_DATA    *pObj;
     PROG_LIST 	       *list;
     PROG_LIST 	       *list_next;
     char 		oprog[MAX_STRING_LENGTH];
     int 		value;
     int 		cnt = 0;

     EDIT_OBJ(ch, pObj);
     one_argument( argument, oprog );
     if (!is_number( oprog ) || oprog[0] == '\0' )
     {
          send_to_char("Syntax:  deloprog [#oprog]\n\r",ch);
          return FALSE;
     }
     value = atoi ( oprog );
     if ( value < 0 )
     {
          send_to_char("Only non-negative oprog-numbers allowed.\n\r",ch);
          return FALSE;
     }
     if ( !(list= pObj->oprogs) )
     {
          send_to_char("OEdit:  Non existant oprog.\n\r",ch);
          return FALSE;
     }
     if ( value == 0 )
     {
          REMOVE_BIT(pObj->oprog_flags, pObj->oprogs->trig_type);
          list = pObj->oprogs;
          pObj->oprogs = list->next;
          free_oprog( list );
     }
     else
     {
          while ( (list_next = list->next) && (++cnt < value ) )
               list = list_next;

          if ( list_next )
          {
               REMOVE_BIT(pObj->oprog_flags, list_next->trig_type);
               list->next = list_next->next;
               free_oprog(list_next);
          }
          else
          {
               send_to_char("No such oprog.\n\r",ch);
               return FALSE;
          }
     }

     send_to_char("Oprog removed.\n\r", ch);
     return TRUE;
}

REDIT ( redit_addrprog )
{
     int 		value;
     ROOM_INDEX_DATA   *pRoom;
     PROG_LIST 	       *list;
     PROG_CODE 	       *code;
     char 		trigger[MAX_STRING_LENGTH];
     char 		phrase[MAX_STRING_LENGTH];
     char 		num[MAX_STRING_LENGTH];

     EDIT_ROOM(ch, pRoom);
     argument=one_argument(argument, num);
     argument=one_argument(argument, trigger);
     argument=one_argument(argument, phrase);

     if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
     {
          send_to_char("Syntax:   addrprog [vnum] [trigger] [phrase]\n\r",ch);
          return FALSE;
     }

     if ( (value = flag_value (rprog_flags, trigger) ) == NO_FLAG)
     {
          send_to_char("Valid flags are:\n\r",ch);
          show_help( ch, "rprog");
          return FALSE;
     }

     if ( ( code =get_prog_index (atoi(num), PRG_RPROG ) ) == NULL)
     {
          send_to_char("No such ROOMProgram.\n\r",ch);
          return FALSE;
     }

     list                  = new_rprog();
     list->vnum            = atoi(num);
     list->trig_type       = value;
     list->trig_phrase     = str_dup(phrase);
     list->code            = code->code;
     SET_BIT(pRoom->rprog_flags,value);
     list->next            = pRoom->rprogs;
     pRoom->rprogs         = list;
     send_to_char( "Rprog Added.\n\r",ch);
     return TRUE;
}

REDIT ( redit_delrprog )
{
     ROOM_INDEX_DATA   *pRoom;
     PROG_LIST 	       *list;
     PROG_LIST 	       *list_next;
     char 		rprog[MAX_STRING_LENGTH];
     int		value;
     int 		cnt = 0;

     EDIT_ROOM(ch, pRoom);

     one_argument( argument, rprog );
     if (!is_number( rprog ) || rprog[0] == '\0' )
     {
          send_to_char("Syntax:  delrprog [#rprog]\n\r",ch);
          return FALSE;
     }
     value = atoi ( rprog );
     if ( value < 0 )
     {
          send_to_char("Only non-negative rprog-numbers allowed.\n\r",ch);
          return FALSE;
     }
     if ( !(list= pRoom->rprogs) )
     {
          send_to_char("REdit:  Non existant rprog.\n\r",ch);
          return FALSE;
     }
     if ( value == 0 )
     {
          REMOVE_BIT(pRoom->rprog_flags, pRoom->rprogs->trig_type);
          list = pRoom->rprogs;
          pRoom->rprogs = list->next;
          free_rprog( list );
     }
     else
     {
          while ( (list_next = list->next) && (++cnt < value ) )
               list = list_next;
          if ( list_next )
          {
               REMOVE_BIT(pRoom->rprog_flags, list_next->trig_type);
               list->next = list_next->next;
               free_rprog(list_next);
          }
          else
          {
               send_to_char("No such rprog.\n\r",ch);
               return FALSE;
          }
     }
     send_to_char("Rprog removed.\n\r", ch);
     return TRUE;
}

