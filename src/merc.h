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

#ifndef _MERC_H
# define _MERC_H   1


/*
 * Accommodate old non-Ansi compilers.
 */
# if defined(TRADITIONAL)
#  define const
#  define args( list )			( )
#  define DECLARE_DO_FUN( fun )		void fun( )
#  define DECLARE_SPEC_FUN( fun )	bool fun( )
#  define DECLARE_SPELL_FUN( fun )	void fun( )
#  define DECLARE_OBJ_FUN( fun )        void fun( )
#  define DECLARE_ROOM_FUN( fun )       void fun( )
# else
#  define args( list )			list
#  define DECLARE_DO_FUN( fun )		DO_FUN    fun
#  define DECLARE_SPEC_FUN( fun )	SPEC_FUN  fun
#  define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#  define DECLARE_OBJ_FUN( fun )        OBJ_FUN   fun
#  define DECLARE_ROOM_FUN( fun )       ROOM_FUN  fun
# endif

/* system calls */
int unlink();
int system();

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
# if	!defined(FALSE)
#  define FALSE	 0
# endif

# if	!defined(TRUE)
#  define TRUE	 1
# endif

/* Some typedefs */
typedef short    int			sh_int;
typedef unsigned char			bool;

/* Borland C++ on win32 has no uint */
# if defined (WIN32)
typedef unsigned int                    uint;
# endif

# ifdef WIN32
#  include <winsock.h>
#  include <sys/types.h>
#  pragma warning( disable: 4018 4244 4761)
#  define NOCRYPT
#  define index strchr
#  define rindex strrchr
# endif

// I know you can get zlib for win32, but I don't have it right
// now. If you have it, you can put this back in.

# if !defined WIN32
#  include <zlib.h>
# else
#  define NOZLIB
# endif

# include "options.h"

# define TELOPT_COMPRESS    85  // MCCP V1
# define TELOPT_COMPRESS2   86  // MCCP V2 - Not Yet Used
# define TELOPT_MSP         90  // Mud Sound Protocol
# define TELOPT_MXP         91  // Mud eXtension Protocol

# define COMPRESS_BUF_SIZE 16384

/*
 * String and memory management parameters.
 */
# define MAX_KEY_HASH		    1024
# define MAX_STRING_LENGTH	    4096 /* Max length for a single string. If you need more, use BUFFER */
# define MAX_INPUT_LENGTH	     256 /* Max length for a player's input line */
# define MSL MAX_STRING_LENGTH           /* So somebody was lazy, shoot 'em */
# define MIL MAX_INPUT_LENGTH
# define MAX_PERM_BLOCK           131072 /* Increase if you need more perms. There are plenty of free ones at this level. */
# define MAX_MEM_LIST                 14 /* Don't tinker with this unless you know with what you screw. */
# define MAX_CHUNKS                   30 /* Can be increased if needs be, for more string space. */
# define MAX_MEMLOG                  100 /* This needs to be increased if you use over 100 identifiers */
# define MAX_VNUM                 199999 /* Can be up to 2 billion or so, but this is more than reasonable. */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
# define MAX_QUOTES		14 /* for random quoting                                                  */
# define MAX_CSCORE              3 /* For default cscores */
# define MAX_SOCIALS	       256 /* Increase this if you add more than 255 socials.                     */
# define MAX_SKILL	       200 /* these limits MUST be updated if you go beyond 200 skills.           */
# define MAX_CLASS	         7 /* Always to be increased if you add a new class                       */
# define MAX_PCRACE             10 /* Always increase if you add a new race. Counts the null race.        */
# define MAX_LEVEL	       110 /* The MAX_LEVEL of the mud. Careful here, not everything respects this code. */
# define MAX_MOB_LEVEL          (MAX_LEVEL + 10) /* Maximum allowed MOB level in the OLC. Same caveat here as above.    */
# define MAX_OBJ_LEVEL	        (MAX_LEVEL + 1)  /* Ditto.                                                              */
# define LEVEL_HERO		(MAX_LEVEL - 9)  /* Pretty standard. Currently this makes 101 a HERO                    */
# define LEVEL_IMMORTAL		(MAX_LEVEL - 8)  /* This makes 102+ an Immortal                                         */
# define LEVEL_ADMIN 		(MAX_LEVEL - 4)  /* This makes 106+ an Admin                                            */
# define PAGELEN                23 /* Default paging size.                                                */

/* Pulse Per Second is *guestimated* to be about 1/4 second, 4 = 1 second */
# define PULSE_PER_SECOND	4 			/* No need to change, simply the basis for the timer */
# define PULSE_VIOLENCE     	( 3 * PULSE_PER_SECOND) /* How often combat updates */
# define PULSE_MOBILE	   	( 4 * PULSE_PER_SECOND) /* Non-Combat mobile pulses, increase to slow mob activity out of combat */
# define PULSE_TICK		(45 * PULSE_PER_SECOND) /* Tick timer. No, players aren't on this -- mobs are. */
# define PULSE_TICKSHORT	( 5 * PULSE_PER_SECOND) /* The player tick timer, very short for continuous healing */
# define PULSE_AREA		(60 * PULSE_PER_SECOND) /* Area updates */

# define IMPLEMENTOR		MAX_LEVEL
# define CREATOR		(MAX_LEVEL - 1) /* 109 */
# define SUPREME		(MAX_LEVEL - 2) /* 108 */
# define DEITY			(MAX_LEVEL - 3) /* 107 */
# define GOD			(MAX_LEVEL - 4) /* 106 */
# define IMMORTAL		(MAX_LEVEL - 5) /* 105 */
# define DEMI			(MAX_LEVEL - 6) /* 104 */ /* Obviously not the same as a player "Demi-God" */
# define ANGEL			(MAX_LEVEL - 7) /* 103 */
# define AVATAR			(MAX_LEVEL - 8) /* 102 */
# define HERO			LEVEL_HERO      /* 101 */

/* Uncomment the below to gzip pfiles */
/* #define COMPRESS_PFILES */

/* Filenames. */
# if defined (WIN32)
#  define TIME_FILE     "..\\time\\mudtime"     /* Mud time & date is stored here */
#  define CLAN_FILE     "Clans.DAT"             /* All .DAT files are located in the DATA_DIR */
#  define CLASS_DIR     "..\\class\\"           /* Skill level information by class */
#  define DATA_DIR      "..\\data\\"            /* Where to store .DAT files */
#  define EXE_FILE      "..\\bin\\sunderw.exe"  /* out of the src dir, like an sensible binary */
#  define MOB_DIR       ".\\mobiles\\"            /* Mobiles ( in relation to area dir */
#  define OBJECT_DIR    ".\\objects\\"            /* Objects */
#  define ROOM_DIR      ".\\rooms\\"              /* Rooms */
#  define PROG_DIR      ".\\programs\\"           /* Progs */
#  define RESET_DIR     ".\\resets\\"             /* Resets */
#  define SHOP_DIR      ".\\shops\\"              /* Shops */
#  define SPEC_DIR      ".\\specials\\"           /* Specials */
# else
#  define TIME_FILE     "../time/mudtime"       /* Mud time & date is stored here */
#  define CLAN_FILE     "Clans.DAT"             /* All .DAT files are located in the DATA_DIR */
#  define CLASS_DIR     "../class/"             /* Skill level information by class */
#  define DATA_DIR      "../data/"              /* Where to store data files */
#  define EXE_FILE      "../bin/sundermud"      /* out of the src dir, like an sensible binary */
#  define MOB_DIR	"./mobiles/"		/* Mobiles ( in relation to area dir */
#  define OBJECT_DIR	"./objects/"		/* Objects */
#  define ROOM_DIR	"./rooms/"		/* Rooms */
#  define PROG_DIR	"./programs/"		/* Progs */
#  define RESET_DIR	"./resets/"		/* Resets */
#  define SHOP_DIR	"./shops/"		/* Shops */
#  define SPEC_DIR	"./specials/"		/* Specials */
# endif

# define DISABLE_FILE  "disable.txt"           /* Disabled Commands file - .txt files are in areas directory */
# define CRIER_FILE    "crier.txt"             /* Town Crier file - ditto */
# define ACCOUNT_FILE  "accounts.DAT"          /* Player Account savefile -- BACKUP OFTEN! */
# define RACE_FILE     "Races.DAT"             /* Races savefile */
# define COPYOVER_FILE "copyover.dat"          /* Located in the areas dir during a copyover */
# define SOCIAL_FILE   "socials.DAT"		/* Located in data directory */
# define HELP_FILE     "helps.txt"		/* Located in data directory */

# define MAX_DIR                6              /* If you increase this, you'll have to rework some code too */
# define NO_FLAG              -99              /* Must not be used in flags or stats. */
# define MAX_ZONE               4              /* Maximum number of zones. Not really used much - yet */
# define MAX_ALIAS             20              /* Number of aliases allowed to a player */
# define MAX_ALIAS_LENGTH      80              /* Up to this length */
# define MAX_ALIAS_PARMS       10              /* Max command stacking in an alias */

/* I use DEBUGINFO for some log_strings that give painful analysis in a few key places that
 * used to cause trouble. I haven't had to use this method in a while though - Lotherius
 * #define DEBUGINFO
 */

/*
 * Colour stuff by Lope of Loping Through The MUD
 * Additions by Lotherius
 * Changed CLEAR to include a white code. Due to friggin ZMud's "default" green 
 */

# define CLEAR		"[0m[0;37m"        /* Resets Colour to white. Couldn't use normal reset                   */
                                               /* Because it liked to make things icky zMud default green for those   */
                                               /* who do use zMud (I didn't know this forever cuz I use tintin++      */
# define FG_BLACK 	"[0;30m"
# define C_RED		"[0;31m"	       /* Normal Colours	*/
# define C_GREEN	"[0;32m"
# define C_YELLOW	"[0;33m"	       /* Yellow/brown, depending on your terminal */
# define C_BLUE		"[0;34m"
# define C_MAGENTA	"[0;35m"
# define C_CYAN		"[0;36m"
# define C_WHITE	"[0;37m"
# define C_D_GREY	"[1;30m"  	       /* Dark Grey, or Bright Black */
# define C_B_RED	"[1;31m"
# define C_B_GREEN	"[1;32m"
# define C_B_YELLOW	"[1;33m"
# define C_B_BLUE	"[1;34m"
# define C_B_MAGENTA	"[1;35m"
# define C_B_CYAN	"[1;36m"
# define C_B_WHITE	"[1;37m"
# define MOD_UNDERLINE 	"[4m"               /* Underline */
# define MOD_BLINK     	"[5m"               /* Blink */
# define MOD_REVERSE   	"[7m"               /* Reverse */
# define VT_CLS	      	"[2J"               /* Clear Screen - Vt100/ANSI but quite standard */
# define VT_SAVEC      	"[s"                /* Save Cursor Pos.    */
# define VT_RESTOREC   	"[u"                /* Restore Cursor Pos. */
# define VT_CLINE      	"[K"                /* Clear to EOL        */

/* MXP Defines */

/* We aren't going to use ANY of the MXP color codes internally.... Having to keep track of how many
 * <color blah> statments you opened so you can </color> them at the end of a line stinks, the <color>
 * statements override ansi until you get a </color> also, so you can't even clear them with an ansi code.
 * And finally, having to send so many tags uses more bandwidth than the ansi sequences to do the same
 * thing - Lotherius
*/

# define MXP_OPEN      "[0z"      /* Open Line, allows "open" category MXP commands */
# define MXP_SECURE    "[1z"      /* Secure Line, allows only Secure Commands (until next newline )*/
# define MXP_LOCKED    "[2z"      /* Locked Line, no parsing!, until next newline. */
// These are in v0.4 of the spec:
# define MXP_RESET     "[3z"      /* Reset. Close all open tags, Mode Open, text/color to default. */
# define MXP_STAG      "[4z"      /* Temp Secure, next tag ONLY is secure, must set again to close tag. */
# define MXP_LOPEN     "[5z"      /* Open mode until changed. */
# define MXP_LSECURE   "[6z"      /* Secure mode until changed. */
# define MXP_LLOCK     "[7z"      /* No parsing until changed. */
// v0.3 of the spec ( zMud ):
# define MXP_RNAME     "[10z"     /* Room Name - Not used here, we use an entity */
# define MXP_RDESC     "[11z"     /* Room Description - ditto */
# define MXP_REXIT     "[12z"     /* Room Exits - ditto */
# define MXP_WELCOME   "[19z"     /* Welcome text... seems to be used only for relocate, don't need it. */


/* Zeran - defines for the skill_available function */
# define SKILL_AVAIL		0
# define SKILL_PRAC		1
# define SKILL_LEARN		2

/* Definitions for various types in mud_data */

// Death Definitions -- see sunder.rc
# define PERMADEATH 	1
# define FULLAGING  	2
# define PARTAGING  	3
# define NOAGING    	4

// Color login options
# define NOCOLOR      	1
# define ASSUME_COLOR 	2
# define ASK_COLOR    	3

// Exp Switches
# define XP_EASIEST   	1
# define XP_EASY      	2
# define XP_NORMAL    	3
# define XP_HARD      	4
# define XP_VERYHARD  	5
# define XP_NIGHTMARE 	6

/* Memlog Action Defines - Determines where to log a transaction with memlog */
# define ALLOC_PERM            1 // Allocate a permanent memory block.
# define ALLOC_MEM             2 // Allocate memory
# define DALLOC_MEM            3 // Deallocate memory


/* Moved ascii conversions up for easier use. */
/* RT ASCII conversions -- used so we can have letters in this file */

# define A		  	1
# define B			2
# define C			4
# define D			8
# define E			16
# define F			32
# define G			64
# define H			128

# define I			256
# define J			512
# define K		        1024
# define L		 	2048
# define M			4096
# define N		 	8192
# define O			16384
# define P			32768

# define Q			65536
# define R			131072
# define S			262144
# define T			524288
# define U			1048576
# define V			2097152
# define W			4194304
# define X			8388608

# define Y			16777216
# define Z			33554432
# define aa			67108864 	/* doubled due to conflicts */
# define bb			134217728
# define cc			268435456
# define dd			536870912
# define ee			1073741824

/*
 * Time and weather stuff.
 */
# define SUN_DARK  		0
# define SUN_RISE		1
# define SUN_LIGHT		2
# define SUN_SET		3

# define SKY_CLOUDLESS		0
# define SKY_CLOUDY		1
# define SKY_RAINING		2
# define SKY_LIGHTNING		3

/*
 * Status Levels for Accounts - Lotherius
 * Skipped Numbers to allow for additions in the future
 */

# define ACCT_REJECTED_DELETE       0	/* Account to be DELETED next reboot (pfiles deleted)  */
# define ACCT_REJECTED_EMAIL	    1	/* Account Cancelled due to Bad Email                  */
# define ACCT_REJECTED_RULES	    5	/* Account Cancelled due to Rule Breakage              */
# define ACCT_REJECTED_OTHER	    10	/* Other reason. Contact an Imm.                       */
# define ACCT_CREATED		    15	/* Account is Created, but no characters exist         */
# define ACCT_UNVERIFIED	    20	/* Account is Unverified, with existing characters     */
# define ACCT_VERIFIED		    25	/* Account has been verified.                          */
# define ACCT_VERIFIED_DEMISTAT	    30	/* Account has access to DemiGods                      */
# define ACCT_HELPER		    35	/* Characters on this account are HELPERS.             */
# define ACCT_STAFF		    40	/* Characters on this account are STAFF.               */
# define ACCT_IMPLEMENTOR	    45	/* Characters on this account have full mud rights.    */

// Command Categories

#define CMD_MOVE		(A)	// Movement Commands
#define CMD_SELFINFO		(B)	// Info about your own character
#define CMD_OTHERINFO		(C)	// Info about other players/mobs
#define CMD_GAMEINFO		(D)	// Info about the game itself
#define CMD_OBJECTS		(E)	// Commands that affect objects
#define CMD_WORLDINFO		(F)	// Info about the world around you
#define CMD_SKILL		(G)	// A skill-related cmd
#define CMD_CONFIG		(H)	// A command to configure things
#define CMD_HELPFUL		(I)	// A command that might be useful
#define CMD_COMM		(J)	// Communication commands
#define CMD_CLAN		(K)	// Clan-related commands
#define CMD_NEWBIE		(L)	// Commands newbies might need to know.
#define CMD_LEASE		(M)	// Commands related to leasing.
#define CMD_COMBAT		(N)	// Commands related to combat
#define CMD_MAGIC		(O)	// Commands related to magic
#define CMD_OLC			(P)	// OLC commands
#define CMD_IMM			(Q)	// Immortal only commands
#define CMD_ACTION		(R)	// Other Actions
#define CMD_MISC		(S)	// Commands that don't fit elsewhere.
#define CMD_SC			(T)	// Commands that are shortcuts for other commands


/*
 * Connected state for a channel.
 */
# define CON_COPYOVER_RECOVER		-15 /* Recovering from Copyover */
# define CON_PLAYING			0  /* The normal state */
# define CON_GET_NAME			1
# define CON_GET_OLD_PASSWORD	  	2
# define CON_CONFIRM_NEW_NAME	  	3
# define CON_GET_NEW_PASSWORD	  	4
# define CON_CONFIRM_NEW_PASSWORD  	5
# define CON_GET_NEW_RACE		6
# define CON_GET_NEW_SEX		7
# define CON_GET_NEW_CLASS		8
# define CON_GET_ALIGNMENT		9
# define CON_ROLL_STATS   		10 /* Assigning stats rather than rolling them in Sunder 2 */
# define CON_CHOOSE_TERM		11 /* This is where everybody starts. */
# define CON_FIND_MORTALS		12 /* Asking if the player wants to be a demigod */
# define CON_READ_IMOTD			13
# define CON_READ_MOTD			14
# define CON_BREAK_CONNECT		15 /* Trying to reconnect to an existing character */
# define CON_ACCOUNT_PW_NEW		16 /* Setting a New Account Password */
# define CON_ACCOUNT_MENU		17 /* Account Menu */
# define CON_LOG_ACCOUNT		18 /* Logging Into Account */
# define CON_ACCOUNT_PW			19 /* Getting Account Password */
# define CON_ACCOUNT_PW_CONFIRM	  	20 /* Verifying Account Password */
# define CON_ACCOUNT_PW_FIX		21 /* Fixing a Null Account Password */
# define CON_ACCOUNT_PW_FIX_VERIFY 	22 /* Verifying above. */
# define CON_PW_FIX			23 /* Fixing Password */
# define CON_PW_FIX_CONFIRM		24 /* Verifying above. */
# define CON_EDIT_CLAN             	25 /* Editing Clans */
/* put new connects here, not after NOTE */
# define CON_NOTE_TO               	90
# define CON_NOTE_SUBJECT          	91
# define CON_NOTE_EXPIRE           	92
# define CON_NOTE_TEXT             	93
# define CON_NOTE_FINISH           	94

/* Mode is to be implemented as a subset of CON_PLAYING
 * What this means is MODEs should always assume the PC is in CON_PLAYING.
 */
# define MODE_NORMAL      1
# define MODE_DESCEDIT    2 // Editing self description
# define MODE_LEASEDESC   3 // Interactive Lease Editor -- editing lease description

/*
 * some prog stuff
 * - Lotherius
 */
# define PROG_MOB   	1
# define PROG_OBJ   	2
# define PROG_ROOM  	3

/*
 * TO types for act.
 */
# define TO_ROOM	    0        /* See act.txt in the docs directory */
# define TO_NOTVICT	    1
# define TO_VICT	    2
# define TO_CHAR	    3

/*
 * Shop types.
 */
# define MAX_TRADE	 5      /* This will set shops to be able to trade more item types */

/*
 * Per-class stuff.
 */
// Class ID Tags
# define CLASS_MAGE		0
# define CLASS_AVENGER		1
# define CLASS_THIEF		2
# define CLASS_WARRIOR 		3
# define CLASS_MONK		4
# define CLASS_DEFILER		5
# define CLASS_CHAOSMAGE 	6
// Variables
# define MAX_GUILD 		2    /* Only here until I feel like editing the class tables to take guild out */
# define MAX_STATS 		5    /* Number of stats, STR INT WIS DEX CON */
// Stat nicknames
# define STAT_STR 		0    /* Shortcuts for those stats */
# define STAT_INT		1
# define STAT_WIS		2
# define STAT_DEX		3
# define STAT_CON		4


/* Affect destinations */

# define TO_AFFECTS      0
# define TO_DETECTIONS   1
# define TO_PROTECTIONS  2

bool    MOBtrigger;	// Odd place for this.
int     max_on;     // So let's have this join it.

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */

# define MOB_VNUM_ZOMBIE        2
# define MOB_VNUM_FAMILIAR	3 /* Default Familiar */


/* Move this over to a struct someday */
/* Zeran - Material types */
# define MATERIAL_WOOD  		1
# define MATERIAL_IRON                  2
# define MATERIAL_SILVER                3
# define MATERIAL_GOLD                  4
# define MATERIAL_ADAMANTITE            5
# define MATERIAL_CLOTH  		6
# define MATERIAL_GLASS  		7
# define MATERIAL_LIQUID  		8
# define MATERIAL_FOOD  		9
# define MATERIAL_STEEL  		10
# define MATERIAL_MITHRIL  		11
# define MATERIAL_PAPER  		12
# define MATERIAL_MEAT  		13
# define MATERIAL_FLESH  		14
# define MATERIAL_LEATHER  		15
# define MATERIAL_PILL  		16
# define MATERIAL_VELLUM  		17
# define MATERIAL_BRONZE  		18
# define MATERIAL_BRASS  		19
# define MATERIAL_STONE  		20
# define MATERIAL_BONE	  		21
# define MATERIAL_UNIQUE  		22
# define MATERIAL_CRYSTAL		23
# define MATERIAL_DIAMOND		24
# define MATERIAL_ICE			25
# define MATERIAL_RUBBER		26
# define MATERIAL_PLASTEEL              27
# define MATERIAL_FUR	         	28
# define MATERIAL_MARBLE        	29
# define MATERIAL_GRANITE               30
# define MATERIAL_IVORY                 31
# define MATERIAL_DIRT                  32
# define MATERIAL_CHALK                 33
# define MATERIAL_SILK          	34
# define MATERIAL_FEATHER               35
# define MATERIAL_COPPER                36
# define MATERIAL_ALUMINUM 	        37
# define MATERIAL_TIN     	        38
/* New Materials */
# define MATERIAL_WGOLD                 39
# define MATERIAL_EMERALD		40
# define MATERIAL_TOPAZ			41
# define MATERIAL_OBSIDIAN		42
# define MATERIAL_PLATINUM              43
# define MATERIAL_CERAMIC               44
# define MAX_MATERIAL			44

/* 
 * Durability Levels
 * Used in material table - Higher numbers damage more quickly.
 */
# define DUR_MAX        0  /* Indestructable */
# define DUR_JEEP       1  /* Nearly indestructable */
# define DUR_EXCEL	2  /* Excellent */
# define DUR_GOOD	3  /* Good */
# define DUR_AVGPLUS    4  /* Above Average */
# define DUR_AVG	5  /* Average Durability */
# define DUR_AVGMINUS   7  /* Below Average */
# define DUR_BAD        10  /* Bad */
# define DUR_AWFUL      15 /* Awful */
# define DUR_WALMART    20 /* Lowest */ /* Zeran - ROFL, nice one Lotherius */

/*
 * Repair Difficulty - used in material table
 */
# define REP_IMPOSSIBLE	 1 /* Repair is not possible */
# define REP_EXTREME     2 /* 5.0 Repair is EXTREMELY difficult */
# define REP_HARD	 3 /* 2.5 Repair is hard */
# define REP_AVGPLUS     4 /* 1.5 Repair is harder than average */
# define REP_AVG         5 /* 1.0 Repair is average difficulty */
# define REP_AVGMINUS    6 /* .75 Repair is easier than average */
# define REP_EASY        7 /* .50 Repair is simple */
# define REP_BREEZE      8 /* .25 Repair is ridiculously easy */

/*
 * Material Flags - Limited to 32, etc ad nauseum - Lotherius
 * Some may not currently be used, but are for future expansion.
 */

# define MAT_FLAMMABLE      (A)    // Can it catch on fire.
# define MAT_VERYFLAMMABLE  (B)    // Does it catch on fire by itself (like brimstone or nitro)
# define MAT_RUSTABLE       (C)    // Can it rust?
# define MAT_INKBLEED       (D)    // Does it have inks that can bleed in water?
# define MAT_DISSOLVE       (E)    // Does it dissolve easily?
# define MAT_BREAKABLE      (F)    // Can break (like glass)
# define MAT_BENDABLE       (G)    // Can it bend (and hold its bent shape, not flex)?
# define MAT_LIVING         (H)    // Is it alive? (flesh... Not sure of a use yet, some items may need changed )
# define MAT_MELT_ALWAYS    (I)    // Does it melt at room temperature?
# define MAT_MELT_NORMAL    (J)    // Does it melt in normal fires? ( set only one of the 3 melt flags )
# define MAT_MELT_VHOT      (K)    // Does it melt in very hot or magical fires?
# define MAT_MELT_MAGICAL   (L)    // Does it melt in magical fires only?
# define MAT_NOMAGIC        (M)    // Does wearing it block mage spells?
# define MAT_PARTMAGIC      (N)    // Does wearing it limit effectiveness of mage spells?
# define MAT_FORGE          (O)    // Can it be forged?
# define MAT_SEW            (P)    // Can it be sewn?
# define MAT_LIQUID         (Q)    // Is it liquid?
# define MAT_GASEOUS        (R)    // Is it a gas?
# define MAT_ETHEREAL       (S)    // Does it exist in a different plane than our own
# define MAT_SOFT           (T)    // Is it soft?
# define MAT_WRITE          (U)    // Can you write with it?
# define MAT_ACIDETCH       (V)    // Will acid hurt it?
# define MAT_VERYHARD       (W)    // Is it harder than normal (damages non-hard material more)
# define MAT_ROCK           (X)    // Is it a type of rock?
# define MAT_METAL          (Y)    // Is it a type of metal? (doesn't imply it can be forged)
# define MAT_MAGIC          (Z)    // Is it a "magical" material?
# define MAT_WRITEON        (aa)   // Can you write on it?
# define MAT_CARVE          (bb)   // Can you carve on it?
# define MAT_FIREPROOF      (cc)   // Is it fireproof? (Can't be damaged by fire)
# define MAT_CONDUCTIVE     (dd)   // Is it conductive to electricity?
/* ee */

/* Armor Flags */
# define ARMOR_BAD_QUALITY   (A)    // Really bad armor
# define ARMOR_LOW_QUALITY   (B)    // Quality somewhat low
# define ARMOR_HIGH_QUALITY  (C)    // Quality is quite high.
/* D */
# define ARMOR_BANDED	     (E)    // Metals
# define ARMOR_RING          (F)    // Metals
# define ARMOR_SCALE         (G)    // Metals
# define ARMOR_PLATE         (H)    // Metals
# define ARMOR_SOFTENED      (I)    // Leathers
# define ARMOR_HARDENED      (J)    // Leathers
# define ARMOR_CAST	     (K)    // Metals
# define ARMOR_FORGED        (L)    // Metals
# define ARMOR_TEMPERED      (M)    // Metals
# define ARMOR_THICKENED     (N)    // Any
# define ARMOR_STUDDED	     (O)    // Leathers
/* O - ee */

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
# define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
# define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
# define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
# define ACT_NORANDOM           (D)             /* Won't get random Items */
# define ACT_NOQUEST            (E)             /* Can't Quest To  */
# define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
# define ACT_STAY_AREA		(G)		/* Won't leave area	*/
# define ACT_WIMPY		(H)
# define ACT_PET		(I)		/* Auto set for pets	*/
# define ACT_FOLLOWER		(J)             /* Following A PC 	*/
# define ACT_PRACTICE		(K)		/* Can practice PC's	*/
# define ACT_SOLDIER		(L)		/* Soldier in Army */
/* M */
/* N */
# define ACT_UNDEAD		(O)
/* P */
# define ACT_CLERIC		(Q)
# define ACT_MAGE		(R)
# define ACT_THIEF		(S)
# define ACT_WARRIOR		(T)
# define ACT_NOALIGN		(U)
# define ACT_NOPURGE		(V)
/* W */
/* X */
/* Y */
/* Z */
# define ACT_IS_HEALER		(aa)
# define ACT_TELEPOP		(bb) /* teleports on pop */
# define ACT_UPDATE_ALWAYS	(cc)
# define ACT_SKILLMASTER        (dd) /* skill master for high level skills */
/* ee */

/* Zeran - define for maximum number of skills a skillmaster can teach */
# define MAX_TEACH_SKILLS        5

/* damage classes */
# define DAM_NONE                0
# define DAM_BASH                1
# define DAM_PIERCE              2
# define DAM_SLASH               3
# define DAM_FIRE                4
# define DAM_COLD                5
# define DAM_LIGHTNING           6
# define DAM_ACID                7
# define DAM_POISON              8
# define DAM_NEGATIVE            9
# define DAM_HOLY                10
# define DAM_ENERGY              11
# define DAM_MENTAL              12
# define DAM_DISEASE             13
# define DAM_DROWNING            14
# define DAM_LIGHT		 15
# define DAM_OTHER               16
# define DAM_HARM		 17
# define DAM_HANDTOHAND		 18 /* Needed for certain things */
# define DAM_BLEEDING            19

/* OFF bits for mobiles */
# define OFF_AREA_ATTACK         (A)
# define OFF_BACKSTAB            (B)
# define OFF_BASH                (C)
# define OFF_BERSERK             (D)
# define OFF_DISARM              (E)
# define OFF_DODGE               (F)
# define OFF_FADE                (G)
# define OFF_FAST                (H)
# define OFF_KICK                (I)
# define OFF_KICK_DIRT           (J)
# define OFF_PARRY               (K)
# define OFF_RESCUE              (L)
# define OFF_TAIL                (M)
# define OFF_TRIP                (N)
# define OFF_CRUSH		 (O)
# define ASSIST_ALL       	 (P)
# define ASSIST_ALIGN	         (Q)
# define ASSIST_RACE    	 (R)
# define ASSIST_PLAYERS      	 (S)
# define ASSIST_GUARD        	 (T)
# define ASSIST_VNUM		 (U)
# define OFF_RACIST		 (V)
/* W - ee */

/* return values for check_imm */
# define IS_NORMAL		0
# define IS_IMMUNE		1
# define IS_RESISTANT		2
# define IS_VULNERABLE		3

/* IMM bits for mobs */
# define IMM_SUMMON              (A)
# define IMM_CHARM               (B)
# define IMM_MAGIC               (C)
# define IMM_WEAPON              (D)
# define IMM_BASH                (E)
# define IMM_PIERCE              (F)
# define IMM_SLASH               (G)
# define IMM_FIRE                (H)
# define IMM_COLD                (I)
# define IMM_LIGHTNING           (J)
# define IMM_ACID                (K)
# define IMM_POISON              (L)
# define IMM_NEGATIVE            (M)
# define IMM_HOLY                (N)
# define IMM_ENERGY              (O)
# define IMM_MENTAL              (P)
# define IMM_DISEASE             (Q)
# define IMM_DROWNING            (R)
# define IMM_LIGHT		 (S)

/* RES bits for mobs */
# define RES_CHARM		 (B)
# define RES_MAGIC               (C)
# define RES_WEAPON              (D)
# define RES_BASH                (E)
# define RES_PIERCE              (F)
# define RES_SLASH               (G)
# define RES_FIRE                (H)
# define RES_COLD                (I)
# define RES_LIGHTNING           (J)
# define RES_ACID                (K)
# define RES_POISON              (L)
# define RES_NEGATIVE            (M)
# define RES_HOLY                (N)
# define RES_ENERGY              (O)
# define RES_MENTAL              (P)
# define RES_DISEASE             (Q)
# define RES_DROWNING            (R)
# define RES_LIGHT		 (S)

/* VULN bits for mobs */
# define VULN_MAGIC              (C)
# define VULN_WEAPON             (D)
# define VULN_BASH               (E)
# define VULN_PIERCE             (F)
# define VULN_SLASH              (G)
# define VULN_FIRE               (H)
# define VULN_COLD               (I)
# define VULN_LIGHTNING          (J)
# define VULN_ACID               (K)
# define VULN_POISON             (L)
# define VULN_NEGATIVE           (M)
# define VULN_HOLY               (N)
# define VULN_ENERGY             (O)
# define VULN_MENTAL             (P)
# define VULN_DISEASE            (Q)
# define VULN_DROWNING           (R)
# define VULN_LIGHT		 (S)
# define VULN_WOOD               (X)
# define VULN_SILVER             (Y)
# define VULN_IRON		 (Z)
# define VULN_MITHRIL	         (aa)
# define VULN_ADAMANTITE         (bb)
# define VULN_STEEL		 (cc)

/* body form */
# define FORM_EDIBLE             (A)
# define FORM_POISON             (B)
# define FORM_MAGICAL            (C)
# define FORM_INSTANT_DECAY      (D)
# define FORM_OTHER              (E)  /* defined by material bit */
/** F **/
/* actual form */
# define FORM_ANIMAL             (G)
# define FORM_SENTIENT           (H)
# define FORM_UNDEAD             (I)
# define FORM_CONSTRUCT          (J)
# define FORM_MIST               (K)
# define FORM_INTANGIBLE         (L)
# define FORM_BIPED              (M)
# define FORM_CENTAUR            (N)
# define FORM_INSECT             (O)
# define FORM_SPIDER             (P)
# define FORM_CRUSTACEAN         (Q)
# define FORM_WORM               (R)
# define FORM_BLOB		 (S)
/** T U **/
# define FORM_MAMMAL             (V)
# define FORM_BIRD               (W)
# define FORM_REPTILE            (X)
# define FORM_SNAKE              (Y)
# define FORM_DRAGON             (Z)
# define FORM_AMPHIBIAN          (aa)
# define FORM_FISH               (bb)
# define FORM_COLD_BLOOD	 (cc)

/* body parts */
# define PART_HEAD               (A)
# define PART_ARMS               (B)
# define PART_LEGS               (C)
# define PART_HEART              (D)
# define PART_BRAINS             (E)
# define PART_GUTS               (F)
# define PART_HANDS              (G)
# define PART_FEET               (H)
# define PART_FINGERS            (I)
# define PART_EAR                (J)
# define PART_EYE		 (K)
# define PART_LONG_TONGUE        (L)
# define PART_EYESTALKS          (M)
# define PART_TENTACLES          (N)
# define PART_FINS               (O)
# define PART_WINGS              (P)
# define PART_TAIL               (Q)
# define PART_NECK               (R)
# define PART_WAIST              (S)
# define PART_WRIST              (T)
/* for combat */
# define PART_CLAWS              (U)
# define PART_FANGS              (V)
# define PART_HORNS              (W)
# define PART_SCALES             (X)
# define PART_TUSKS		 (Y)
# define PART_HOOFS		 (Z)
/* Back to body parts. */
# define PART_FACE               (aa)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
# define AFF_BLIND		    	(A)
# define AFF_INVISIBLE		    	(B)
/** C D E F **/
# define AFF_MELD		        (G)
/** H **/
# define AFF_FAERIE_FIRE		(I)
/** J **/
# define AFF_CURSE		        (K)
# define AFF_FEAR		        (L)
# define AFF_POISON		        (M)
/** N O **/
# define AFF_SNEAK		        (P)
# define AFF_HIDE		        (Q)
# define AFF_SLEEP		        (R)
# define AFF_CHARM		        (S)
# define AFF_FLYING		        (T)
# define AFF_PASS_DOOR		      	(U)
# define AFF_HASTE		        (V)
# define AFF_CALM		        (W)
# define AFF_PLAGUE		        (X)
# define AFF_WEAKEN		        (Y)
/** Z **/
# define AFF_BERSERK		        (aa)
# define AFF_SWIM		        (bb)
# define AFF_REGENERATION          	(cc)
# define AFF_POLY		        (dd)
/** ee **/

/* DETECTION flags */

# define AFF_SHIELD     	     	(A)	/* Compatibility */ /* To be deleted */
# define AFF_MUTE		        (B)	/* Compatibility */
# define AFF_SLOW               	(C)	/* Compatibility */
# define AFF_CONFUSION		      	(C)	/* Compatibility */
# define AFF_RALLY		        (D)	/* Compatibility */ /* To be deleted */

# define DET_EVIL                  	(E)
# define DET_INVIS                 	(F)
# define DET_MAGIC                 	(G)
# define DET_HIDDEN                	(H)
# define DET_INFRARED		   	(I)
# define DET_DARK_VISION		(J)

/* PROTECTION flags */

# define PROT_GOOD                 	(A)
# define PROT_EVIL                 	(B)
# define PROT_SANCTUARY            	(C)
# define PROT_ABSORB		        (D)
# define PROT_PHASED		        (E)

/*
 * Sex.
 * Used in #MOBILES.
 */
# define SEX_NEUTRAL		      	0
# define SEX_MALE		      	1
# define SEX_FEMALE		      	2
# define SEX_RANDOM                   	3

/* AC types */
# define AC_PIERCE			0
# define AC_BASH			1
# define AC_SLASH			2
# define AC_EXOTIC			3

/* dice */
# define DICE_NUMBER			0
# define DICE_TYPE			1
# define DICE_BONUS			2

/* size */
# define SIZE_TINY			0
# define SIZE_SMALL			1
# define SIZE_MEDIUM			2
# define SIZE_LARGE			3
# define SIZE_HUGE			4
# define SIZE_GIANT			5
# define SIZE_UNKNOWN        		6  /* Zeran - added at Lotherius request */

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
# define OBJ_VNUM_MONEY_ONE	      2
# define OBJ_VNUM_MONEY_SOME	      3
# define OBJ_VNUM_DUMMY  	      1 /* ROM OLC */
# define OBJ_VNUM_CORPSE_NPC	     10
# define OBJ_VNUM_CORPSE_PC	     11
# define OBJ_VNUM_SEVERED_HEAD	     12
# define OBJ_VNUM_TORN_HEART	     13
# define OBJ_VNUM_SLICED_ARM	     14
# define OBJ_VNUM_SLICED_LEG	     15
# define OBJ_VNUM_GUTS		     16
# define OBJ_VNUM_BRAINS	     17
# define OBJ_VNUM_SOULBLADE          18

# define OBJ_VNUM_MUSHROOM	     20
# define OBJ_VNUM_LIGHT_BALL	     21
# define OBJ_VNUM_SPRING	     22
# define OBJ_POTION		     23
# define OBJ_SCROLL		     24

# define OBJ_VNUM_ICE_DAGGER	     54
# define OBJ_VNUM_PORTAL	     55

# define OBJ_VNUM_SCHOOL_MACE	     56
# define OBJ_VNUM_SCHOOL_DAGGER	     57
# define OBJ_VNUM_SCHOOL_SWORD	     58
# define OBJ_VNUM_SCHOOL_VEST	     59
# define OBJ_VNUM_SCHOOL_SHIELD	     60
# define OBJ_VNUM_SCHOOL_BANNER      61
# define OBJ_VNUM_MAP		     62
# define OBJ_VNUM_BOAT		     63 /* added for minor creation */
# define OBJ_VNUM_BAG		     64 /* minor creation */
# define OBJ_VNUM_PIT		     65

/*
 * Item types.
 * Used in #OBJECTS.
 */
# define ITEM_LIGHT		      1
# define ITEM_SCROLL		      2
# define ITEM_WAND		      3
# define ITEM_STAFF		      4
# define ITEM_WEAPON		      5
# define ITEM_TREASURE		      8
# define ITEM_ARMOR		      9
# define ITEM_POTION		     10
# define ITEM_CLOTHING		     11
# define ITEM_FURNITURE		     12
# define ITEM_TRASH		     13
# define ITEM_CONTAINER		     15
# define ITEM_DRINK_CON		     17
# define ITEM_KEY		     18
# define ITEM_FOOD		     19
# define ITEM_MONEY		     20
# define ITEM_BOAT		     22
# define ITEM_CORPSE_NPC             23
# define ITEM_CORPSE_PC		     24
# define ITEM_FOUNTAIN		     25
# define ITEM_PILL		     26
# define ITEM_PROTECT		     27
# define ITEM_MAP		     28
# define ITEM_PRIDE                  29
# define ITEM_COMPONENT	             30
# define ITEM_PORTAL		     31

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
# define ITEM_GLOW		(A)
# define ITEM_HUM		(B)
# define ITEM_DARK		(C)
# define ITEM_LOCK		(D)
# define ITEM_EVIL		(E)
# define ITEM_INVIS		(F)
# define ITEM_MAGIC		(G)
# define ITEM_NODROP		(H)
# define ITEM_BLESS		(I)
# define ITEM_ANTI_GOOD		(J)
# define ITEM_ANTI_EVIL		(K)
# define ITEM_ANTI_NEUTRAL	(L)
# define ITEM_NOREMOVE		(M)
# define ITEM_INVENTORY		(N)
# define ITEM_NOPURGE		(O)
# define ITEM_ROT_DEATH		(P)
# define ITEM_VIS_DEATH		(Q)
# define ITEM_NO_SAC		(R)
# define ITEM_CONCEALED		(S)
# define ITEM_NO_COND		(T)
# define ITEM_NODISP            (U) // An item that is "Seen" but isn't displayed in the room.

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
# define ITEM_TAKE		(A)
# define ITEM_WEAR_FINGER	(B)
# define ITEM_WEAR_NECK		(C)
# define ITEM_WEAR_BODY		(D)
# define ITEM_WEAR_HEAD		(E)
# define ITEM_WEAR_LEGS		(F)
# define ITEM_WEAR_FEET		(G)
# define ITEM_WEAR_HANDS	(H)
# define ITEM_WEAR_ARMS		(I)
# define ITEM_WEAR_SHIELD	(J)
# define ITEM_WEAR_ABOUT	(K)
# define ITEM_WEAR_WAIST	(L)
# define ITEM_WEAR_WRIST	(M)
# define ITEM_WIELD		(N)
# define ITEM_HOLD		(O)
# define ITEM_TWO_HANDS		(P)
# define ITEM_WEAR_PRIDE	(Q)
# define ITEM_WEAR_FACE		(R)
# define ITEM_WEAR_EARS		(S)
# define ITEM_WEAR_FLOAT	(T)

/* weapon class */
# define WEAPON_EXOTIC		0
# define WEAPON_SWORD		1
# define WEAPON_DAGGER		2
# define WEAPON_SPEAR		3
# define WEAPON_MACE		4
# define WEAPON_AXE		5
# define WEAPON_FLAIL		6
# define WEAPON_WHIP		7
# define WEAPON_POLEARM		8

/* weapon types */
# define MAX_BRAND		(I)
# define WEAPON_FLAMING		(A)
# define WEAPON_FROST		(B)
# define WEAPON_VAMPIRIC	(C)
# define WEAPON_SHARP		(D)
# define WEAPON_VORPAL		(E)
# define WEAPON_TWO_HANDS	(F)
# define WEAPON_ACID		(G)
# define WEAPON_LIGHTNING	(H)
# define WEAPON_POISON          (I)

/* furniture flags */
# define SIT_AT                  (A) /* 1 */
# define SIT_ON                  (B) /* 2 */
# define SIT_IN                  (C) /* 4 */
# define REST_AT                 (D) /* 8 */
# define REST_ON                 (E) /* 16 */
# define REST_IN                 (F) /* 32 */
# define SLEEP_AT                (G) /* 64 */
# define SLEEP_ON                (H) /* 128 */
# define SLEEP_IN                (I) /* 256 */

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
# define APPLY_NONE		      0
# define APPLY_STR		      1
# define APPLY_DEX		      2
# define APPLY_INT		      3
# define APPLY_WIS		      4
# define APPLY_CON		      5
# define APPLY_SEX		      6
# define APPLY_CLASS		      7
# define APPLY_LEVEL		      8
# define APPLY_AGE		      9
# define APPLY_HEIGHT		     10
# define APPLY_WEIGHT		     11
# define APPLY_MANA		     12
# define APPLY_HIT		     13
# define APPLY_MOVE		     14
# define APPLY_GOLD		     15
# define APPLY_EXP		     16
# define APPLY_AC		     17
# define APPLY_HITROLL		     18
# define APPLY_DAMROLL		     19
# define APPLY_SAVING_PARA	     20
# define APPLY_SAVING_ROD	     21
# define APPLY_SAVING_PETRI	     22
# define APPLY_SAVING_BREATH	     23
# define APPLY_SAVING_SPELL	     24
# define APPLY_ENCUMBRANCE           25

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
# define CONT_CLOSEABLE		      1
# define CONT_PICKPROOF		      2
# define CONT_CLOSED		      4
# define CONT_LOCKED		      8

/*
 * Area flags.
 */
# define        AREA_NONE       (A)	/* No flags 			*/
# define        AREA_CHANGED    (B)     /* Area has been modified. 	*/
# define        AREA_ADDED      (C)     /* Area has been added to. 	*/
# define        AREA_LOADING    (D)	/* Used for counting in db.c 	*/
# define	AREA_HEADER_CH  (E)	/* Header changed 		*/
# define	AREA_ROOMS_CH   (F)	/* Rooms changed 		*/
# define	AREA_OBJS_CH	(G)	/* Objects changed 		*/
# define	AREA_MOBS_CH	(H)	/* Mobs changed 		*/
# define	AREA_RESETS_CH	(I)	/* Resets changed 		*/
# define	AREA_SHOPS_CH	(J)	/* Shops changed 		*/
# define	AREA_SPECIAL_CH	(K)	/* Specials changed 		*/
# define	AREA_PROGS_CH	(L)	/* Progs changed 		*/
# define	AREA_LEASE_CH	(M)	/* Leases changed 		*/

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
# define ROOM_VNUM_LIMBO	      2
# define ROOM_VNUM_CHAT		   1300
# define ROOM_VNUM_SCHOOL	   1000
# define ROOM_VNUM_TEMPLE	   2093 /* recall */
# define ROOM_VNUM_ALTAR	   2096 /* healer */
					/* soon to be not used */
					/* Replaced with racial values */

/*
 * Room flags.
 * Used in #ROOMS.
 */
# define ROOM_DARK	   (A)
# define ROOM_OWNER_ONLY   (B)
# define ROOM_NO_MOB	   (C)
# define ROOM_INDOORS	   (D)
# define ROOM_FASTHEAL	   (E) /* Lotherius room fastheal */
# define ROOM_PKILL	   (F)
# define ROOM_RENT	   (G) /* Room can be rented */
# define ROOM_TRANSIT	   (H) /* Room is a transit room, not used */
# define ROOM_NOTELEPOP    (I) /* Won't receive telepops */
# define ROOM_PRIVATE	   (J)
# define ROOM_SAFE	   (K)
# define ROOM_SOLITARY	   (L)
# define ROOM_PET_SHOP	   (M)
# define ROOM_NO_RECALL	   (N)
# define ROOM_IMP_ONLY	   (O)
# define ROOM_GODS_ONLY	   (P)
# define ROOM_HEROES_ONLY  (Q)
# define ROOM_NEWBIES_ONLY (R)
# define ROOM_LAW	   (S)
# define ROOM_BANK	   (T)

/*
 * Directions.
 * Used in #ROOMS.
 */
# define DIR_NORTH     0
# define DIR_EAST      1
# define DIR_SOUTH     2
# define DIR_WEST      3
# define DIR_UP	       4
# define DIR_DOWN      5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
# define EX_ISDOOR	      	(A)
# define EX_CLOSED	      	(B)	/* Never saved in the areafile */
# define EX_LOCKED	      	(C)	/* Never saved in the areafile */
# define EX_PICKPROOF		(F)
# define EX_HIDDEN		(G)
# define EX_NO_PASS		(H)

/*
 * Sector types.
 * Used in #ROOMS.
 */
/* Zeran - special note
     update movement_loss table in act_move.c when adding a new sector,
	 otherwise the movement cost for the sector will be 0...
*/
# define SECT_INSIDE		      0
# define SECT_CITY		      1
# define SECT_FIELD		      2
# define SECT_FOREST		      3
# define SECT_HILLS		      4
# define SECT_MOUNTAIN		      5
# define SECT_WATER_SWIM	      6
# define SECT_WATER_NOSWIM	      7
# define SECT_AIR		      9
# define SECT_DESERT		     10
# define SECT_UNDERGROUND	     11
# define SECT_FORT		     12
# define SECT_MAX		     13

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 * 
 * WARNING!!! Changing these values will BREAK THINGS. Only add new values
 * at the end (insert before MAX_WEAR) and always update wear_info in const.c
 * with information for any new value, keeping it in the same order as this.
 */
# define WEAR_NONE		     -1
# define WEAR_LIGHT		      0
# define WEAR_FINGER_L		      1
# define WEAR_FINGER_R		      2
# define WEAR_NECK_1		      3
# define WEAR_NECK_2		      4
# define WEAR_BODY		      5
# define WEAR_HEAD		      6
# define WEAR_LEGS		      7
# define WEAR_FEET		      8
# define WEAR_HANDS		      9
# define WEAR_ARMS		     10
# define WEAR_SHIELD		     11
# define WEAR_ABOUT		     12
# define WEAR_WAIST		     13
# define WEAR_WRIST_L		     14
# define WEAR_WRIST_R		     15
# define WEAR_WIELD		     16
# define WEAR_HOLD		     17
# define WEAR_WIELD2                 18
# define WEAR_PRIDE		     19
# define WEAR_FACE		     20
# define WEAR_EARS		     21
# define WEAR_FLOAT		     22
# define MAX_WEAR		     23

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
# define COND_DRUNK		      0
# define COND_FULL		      1
# define COND_THIRST		      2

/*
 * Positions.
 */
# define POS_DEAD		      0
# define POS_MORTAL		      1
# define POS_INCAP		      2
# define POS_STUNNED		      3
# define POS_SLEEPING		      4
# define POS_RESTING		      5
# define POS_SITTING		      6
# define POS_FIGHTING		      7
# define POS_STANDING		      8

/*
 * ACT bits for players.
 */
# define PLR_IS_NPC		(A)	/* Don't EVER set.	*/
# define PLR_BOUGHT_PET		(B)
# define PLR_AUTOASSIST		(C)	/* Need to move auto flags off to their own field someday. */
# define PLR_AUTOEXIT		(D)
# define PLR_AUTOLOOT		(E)
# define PLR_AUTOSAC            (F)
# define PLR_AUTOGOLD		(G)
# define PLR_AUTOSPLIT		(H)
/* I */
/* J */
/* K */
/* L */
# define PLR_QUESTOR		(M) 	/* Quest Code */
# define PLR_HOLYLIGHT		(N)
# define PLR_WIZINVIS		(O)
# define PLR_CANLOOT		(P)
# define PLR_NOSUMMON		(Q)
# define PLR_NOFOLLOW		(R)
# define PLR_XINFO		(S)	/* I think the colour, msp, etc should be in COMM_ - lotherius */
/* T */
# define PLR_CURSOR		(U)     /* Cursor Control by Lotherius */
/* V */
# define PLR_LOG		(W)
# define PLR_DENY		(X)
# define PLR_FREEZE		(Y)
# define PLR_THIEF		(Z)
# define PLR_KILLER		(aa)
# define PLR_AFK                (bb)
/* cc */
# define PLR_AUTOSAVE		(dd)
# define PLR_CLOAK		(ee)


/* RT comm flags -- may be used on both mobs and chars */

# define COMM_QUIET              (A)
# define COMM_DEAF               (B)
# define COMM_NOWIZ              (C)
# define COMM_NOAUCTION          (D)
# define COMM_NOGOSSIP           (E)
# define COMM_NOQUESTION         (F)
# define COMM_NOMUSIC            (G)
# define COMM_NOCLANTELL	 (H)
/* I */
/* J */
/* K */
# define COMM_COMPACT		 (L)
# define COMM_BRIEF		 (M)
# define COMM_PROMPT		 (N)
# define COMM_COMBINE		 (O)
# define COMM_TELNET_GA		 (P)	/* Should we use this?? */
# define COMM_FULLFIGHT		 (Q)
# define COMM_DARKCOLOR		 (R)	/* Strip bright colours. */
# define COMM_NOFLASHY		 (S)	/* Don't see reverse colours */
# define COMM_NOEMOTE		 (T)	/* Penalties */
# define COMM_NOSHOUT		 (U)
# define COMM_NOTELL		 (V)
# define COMM_NOCHANNELS	 (W)
# define COMM_BEEP         	 (X)    /* Lotherius Added this for BEEP */
# define COMM_NODARKGREY	 (Y)	/* Don't see the dark black colour */
# define COMM_COLOUR		 (Z)	/* Colour.. Has to be duplicated in ch->comm saving linkdead chars. */
# define COMM_PUEBLO         (aa)   /* Pueblo Client Enabled */
# define COMM_MXP            (bb)   /* Yet again, needed. Like pueblo, this is to avoid redetect on copyover. */
/* cc */
/* dd */
/* ee */

/* Zeran - channel id's */
/* order doesn't matter, just so each one is unique */
# define CHAN_GOSSIP		(1)
# define CHAN_MUSIC		(2)
# define CHAN_AUCTION		(3)
# define CHAN_QUESTION		(4)
# define CHAN_ANSWER		(5)
# define CHAN_IMMTALK		(6)
# define CHAN_IMPTALK		(7)
# define CHAN_SAY		(8)
# define CHAN_CLAN		(9)
# define CHAN_YELL		(10)
# define CHAN_SHOUT		(11)

/* Zeran - character notify flags */
# define NOTIFY_LEVEL		(A)
# define NOTIFY_DEATH		(B)
# define NOTIFY_DELETE		(C)
# define NOTIFY_LOGIN		(D)
# define NOTIFY_QUITGAME	(E)
# define NOTIFY_LOSTLINK	(F)
# define NOTIFY_CLANPROMOTE	(G)
# define NOTIFY_CLANDEMOTE	(H)
# define NOTIFY_CLANACCEPT	(I)
# define NOTIFY_CLANG           (J)
# define NOTIFY_CLANQUIT	(K)
# define NOTIFY_CLANPETITION    (L)
# define NOTIFY_HERO            (M)
# define NOTIFY_NEWNOTE		(N)
# define NOTIFY_RECONNECT	(O)
# define NOTIFY_REPOP		(P)
# define NOTIFY_WEATHER		(Q)
# define NOTIFY_TICK		(R)
# define NOTIFY_ALL		(S-1) /*flags all lower bits*/

/* Zeran - notify TO's */
# define TO_ALL 		1
# define TO_CLAN		2
# define TO_PERS		3

/* anything above 102 will be interpeted as a wiznet message */

# define TO_WIZNET		(MAX_LEVEL - 8) // 102
# define TO_IMM			(MAX_LEVEL - 8) // 102
# define TO_IMM_ADMIN	        (MAX_LEVEL - 4) // 106
# define TO_IMP			MAX_LEVEL       // 110

/* Zeran - wiznet additions */
# define WIZNET_SITES		(A)
# define WIZNET_NEWBIE		(B)
# define WIZNET_SPAM		(C)
# define WIZNET_DEATH		(D)
# define WIZNET_RESET		(E)
# define WIZNET_MOBDEATH	(F)
# define WIZNET_BUG		(G)
# define WIZNET_SWITCH		(H)
# define WIZNET_LINK		(I)

/* remainder are administrative level only */
# define WIZNET_LOAD		(M)
# define WIZNET_RESTORE		(N)
# define WIZNET_SNOOP		(O)
# define WIZNET_SECURE		(P)


/*
 * MOBprog definitions
 */
# define TRIG_ACT	(A)
# define TRIG_BRIBE	(B)
# define TRIG_DEATH	(C)
# define TRIG_ENTRY	(D)
# define TRIG_FIGHT	(E)
# define TRIG_GIVE	(F)
# define TRIG_GREET	(G)
# define TRIG_GRALL	(H)
# define TRIG_KILL	(I)
# define TRIG_HPCNT	(J)
# define TRIG_RANDOM	(K)
# define TRIG_SPEECH	(L)
# define TRIG_EXIT	(M)
# define TRIG_EXALL	(N)
# define TRIG_DELAY	(O)
# define TRIG_SURR	(P)
# define TRIG_GET       (Q)
# define TRIG_DROP      (R)
# define TRIG_SIT       (S)
# define TRIG_PULL      (T)
# define TRIG_PUSH      (U)
# define TRIG_CLIMB     (V)
# define TRIG_TURN      (W)
# define TRIG_PLAY      (X)
# define TRIG_TWIST     (Y)
/** Z **/
# define TRIG_LIFT      (aa)
# define TRIG_DIG       (bb)
# define TRIG_ENTER     (cc)
# define TRIG_WEAR      (dd)
# define TRIG_REMOVE    (ee)  // Triggers aren't bitvectors, we don't need to be using these
                              // Letters... However, if this is how they're stored in the areafiles,
                              // then we need to make sure it gets updated before using any higher
                              // numbered triggers.


/*
 * Prog types
 */
# define PRG_MPROG       0
# define PRG_OPROG       1
# define PRG_RPROG       2

/*
 * Liquids.
 */
# define LIQ_WATER    0
# define LIQ_MAX     16


/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
# define TYPE_UNDEFINED        -1
# define TYPE_HIT            1000

/*
 *  Target types.
 */
# define TAR_IGNORE		    0
# define TAR_CHAR_OFFENSIVE	    1
# define TAR_CHAR_DEFENSIVE	    2
# define TAR_CHAR_SELF		    3
# define TAR_OBJ_INV		    4
/* Additional Target Types for cmd/spell/skill prog development */
# define TAR_SELF		5
# define TAR_OTHER_OFFENSIVE	6
# define TAR_OTHER_DEFENSIVE	7
# define TAR_OBJ_HERE		8
# define TAR_OBJ_ROOM		9
# define TAR_OTHER_AREA		10
# define TAR_OTHER_WORLD        11
# define TAR_ROOM		12
# define TAR_ENEMY		13


/*
 * Utility macros.
 */
# define IS_VALID(data)          ((data) != NULL && (data)->valid)
# define VALIDATE(data)          ((data)->valid = TRUE)
# define INVALIDATE(data)        ((data)->valid = FALSE)
# define UMIN(a, b)		 ((a) < (b) ? (a) : (b))
# define UMAX(a, b)		 ((a) > (b) ? (a) : (b))
# define URANGE(a, b, c)	 ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
# define DIVTWO(a)		 ((a)>>1) // Quick divide by two.
# define MULTWO(a)		 ((a)<<1) // QUick multiply by two.
# define LOWER(c)		 ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
# define UPPER(c)		 ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
# define IS_SET(flag, bit)	 ((flag) & (bit))
# define SET_BIT(var, bit)	 ((var) |= (bit))
# define REMOVE_BIT(var, bit)	 ((var) &= ~(bit))
# define IS_NULLSTR(str)	 ((str) == NULL || (str)[0] == '\0')
# define ENTRE(min,num,max)	 (((min) < (num)) && ((num) < (max)))

// For when you need to just see there's valid lease data at all, doesn't guarantee a rented_by field
# define IS_LEASE(data)          ((data) != NULL && (data)->valid && (data)->room !=NULL \
				  && IS_SET ((data)->room->room_flags, ROOM_RENT))

// For when you need to see both if it *is* a lease, and if it is, if it's currently rented by someone
# define IS_RENTED(data)         (IS_LEASE((data)) && !IS_NULLSTR((data)->rented_by))

/*
 * Character macros.
 */

# define IS_NPC(ch)		 (IS_SET((ch)->act, ACT_IS_NPC))
# define IS_IMMORTAL(ch)         (get_trust(ch) >= LEVEL_IMMORTAL)
# define IS_IMP(ch)		 (get_trust(ch) == MAX_LEVEL)
# define IS_HERO(ch)		 (get_trust(ch) >= LEVEL_HERO)
# define IS_TRUSTED(ch,level)	 (get_trust((ch)) >= (level))
# define IS_QUESTOR(ch)          (IS_SET((ch)->act, PLR_QUESTOR)) /* Quest Code */
# define IS_AFFECTED(ch, sn)     (IS_SET((ch)->affected_by, (sn)))
/* New affected bits 2 and 3 */
# define CAN_DETECT(ch, sn)	 (IS_SET((ch)->detections, (sn)))
# define IS_PROTECTED(ch, sn)	 (IS_SET((ch)->protections, (sn)))
/* end of affected bits 2 and 3 */
# define IS_GOOD(ch)		 (ch->alignment >= 350)
# define IS_EVIL(ch)		 (ch->alignment <= -350)
# define IS_NEUTRAL(ch)		 (!IS_GOOD(ch) && !IS_EVIL(ch))
# define IS_AWAKE(ch)		 (ch->position > POS_SLEEPING)

/* Simplified */
# define GET_HITROLL(ch)	 ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
# define GET_DAMROLL(ch)         ((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

/* Zeran - this IS_OUTSIDE now checks sector type */
# define IS_OUTSIDE(ch)	      		(!IS_SET( (ch)->in_room->room_flags, ROOM_INDOORS) \
					 && ( (ch)->in_room->sector_type != SECT_INSIDE ) \
					 && ( (ch)->in_room->sector_type != SECT_UNDERGROUND ) )
# define WAIT_STATE(ch, npulse)		((ch)->wait = UMAX((ch)->wait, (npulse)))
# define HAS_TRIGGER_MOB(ch,trig)	(IS_SET((ch)->pIndexData->mprog_flags,(trig)))
# define HAS_TRIGGER_OBJ(obj,trig) 	(IS_SET((obj)->pIndexData->oprog_flags,(trig)))
# define HAS_TRIGGER_ROOM(room,trig) 	(IS_SET((room)->rprog_flags,(trig)))

/* Moved here from olc.h for universal access */
# define IS_SWITCHED( ch )       	( ch->desc && ch->desc->original )
# define IS_BUILDER(ch, Area)		( !IS_NPC(ch) && !IS_SWITCHED( ch ) &&	  \
					( ch->pcdata->security >= Area->security  \
					   || strstr( Area->builders, ch->name )  \
					   || strstr( Area->builders, "All" ) ) )

/*
 * Object macros.
 */
# define CAN_WEAR(obj, part)		(IS_SET((obj)->wear_flags,  (part)))
# define IS_OBJ_STAT(obj, stat)		(IS_SET((obj)->extra_flags, (stat)))
# define IS_WEAPON_STAT(obj,stat)	(IS_SET((obj)->value[4],(stat)))

/*
 * Description macros.
 */
/* Zeran - modifed PERS to account for masked player */
# define PERSMASK(ch, looker)	( can_see( looker, (ch) ) ? IS_AFFECTED((ch),AFF_POLY) ?  \
				(ch)->short_descr  : IS_NPC(ch) ? (ch)->short_descr	  \
				: (ch)->name : "someone" )
# define PERS(ch, looker)	( can_see( looker, (ch) ) ? IS_AFFECTED((ch),AFF_POLY) ? IS_NPC(ch) ?   \
				(ch)->short_descr :  (ch)->poly_name : IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name : "someone" )

/* 
 * "unbreakable" string functions.
 * Defined so we don't have to type out the longer functions every time,
 * which leads to laziness, and then before you know it we aren't checking bounds
 * anywhere.
 * These functions could use some error logging so caught string overflows will
 * be logged. They also do not replace the utility of the BUFFER type from Erwin.
 * -- Lotherius.
 */

/* SNP:   Wrapper to snprintf that uses the format of sprintf.
 * SLCAT: Wrapper to strlcat that uses the format of strcat.
 * SLCPY: Wrapper to strlcpy that uses the format of strcpy.
 * Systems that don't have sstrlcat or strlcpy will have these functions emulated.
 * Use these whenever possible.
 */
# if defined ( SNP )
#  undef SNP
# endif
# define SNP(var,args...)   snprintf ( (var), sizeof((var)) -1, ##args )
/* Need to get the proper definition of the above on Win32, it doesn't work.. No win32 compile till it does. */

#ifndef __FreeBSD__
/* We'll have to make our own strlcat and strlcpy on win32 and linux */
size_t strlcat args ( (char *dst, const char *src, size_t siz) );
size_t strlcpy args ( (char *dst, const char *src, size_t siz) );
# endif

/* 
 * FreeBSD has strlcat and strlcpy... since I'm not quite sure how to detect these, we'll
 * "assume" they exist if the two previous conditions failed.
 */
# if defined ( SLCAT )
#  undef SLCAT
# endif
# define SLCAT(str,append)  strlcat ( (str), (append), sizeof ( str ) )
# if defined ( SLCPY )
#  undef SLCPY
# endif
# define SLCPY(dst,src)     strlcpy ( (dst), (src), sizeof ( dst ) )


/* Database Macros */
# if defined(KEY)
#  undef KEY
# endif
# define KEY( literal, field, value )                    			\
          if ( !str_cmp( word, literal ) )              		\
					{                                   	\
						 field  = value;                \
						 fMatch = TRUE;                 \
						 break;                         \
					}

/* DO NOT USE STR_DUP ON KEYS - THAT ISN'T HOW IT WORKS, you DO WANT field = value, TRUST ME - Lotherius */
/* We had a HORRIBLE memory leak here because someone rewrote KEYS without knowing what they were doing  */
/* Wonder who that was.........                                                                          */
# if defined(KEYS)
#  undef KEYS
# endif
# define KEYS( literal, field, value ) 						\
		  if ( !str_cmp( word, literal ) ) 				\
					{ free_string(field); 			\
						 field  = value; 		\
						 fMatch = TRUE; break;		\
                                        }


/*
 * Turn on NOCRYPT to keep passwords in plain text.
 * Windows has the "wincrypt32" libary... Perhaps someone could
 * check the usability of that?
 */

# if defined (WIN32)
#  define NOCRYPT
# endif

# if	defined(NOCRYPT)
#  define crypt(s1, s2)	(s1)
# endif

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
# ifdef WIN32
#  define PLAYER_DIR	"..\\player\\"	/* Player files			*/
#  define PLAYER_TEMP	"..\\player\\temp"
#  define GOD_DIR	"..\\gods\\"	/* list of gods			*/
#  define NULL_FILE     "nul"           /* To reserve one stream        */
# else
#  define NULL_FILE     "/dev/null"     /* To reserve one stream        */
#  define PLAYER_DIR	"../player/"	/* Player files			*/
#  define PLAYER_TEMP	"../player/temp"
#  define GOD_DIR       "../gods/"	/* list of gods			*/
# endif
# define AREA_LIST	"area.lst"	/* List of areas		*/
# define BUG_FILE	"bugs.txt"      /* For 'bug' and bug( )		*/
# define TYPO_FILE	"typos.txt"     /* For 'typo'			*/
# define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/


/*-------------------------------
 * Note Board Defines
 * for Version 2 board system
 * (c) 1995-96 erwin@pip.dknet.dk
 *------------------------------*/

# define NOTE_DIR       "../msgbase/" /* set it to something you like */

# define DEF_NORMAL  0 /* No forced change, but default (any string)   */
# define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
# define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

# define MAX_BOARD   7

# define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */
            
# define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
                           /* This only applies in the Body of the note */
            
# define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)
            
# define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */


/* bit.c */
extern const struct flag_type   area_flags[];
extern const struct flag_type   sex_flags[];
extern const struct flag_type   exit_flags[];
extern const struct flag_type   door_resets[];
extern const struct flag_type   room_flags[];
extern const struct flag_type   sector_flags[];
extern const struct flag_type   type_flags[];
extern const struct flag_type   extra_flags[];
extern const struct flag_type   wear_flags[];
extern const struct flag_type   act_flags[];
extern const struct flag_type   affect_flags[];
extern const struct flag_type   detect_flags[];
extern const struct flag_type   protect_flags[];
extern const struct flag_type   apply_flags[];
extern const struct flag_type   wear_loc_strings[];
extern const struct flag_type   wear_loc_flags[];
extern const struct flag_type   weapon_flags[];
extern const struct flag_type   container_flags[];
extern const struct flag_type   liquid_flags[];
extern const struct flag_type   material_type[];
extern const struct flag_type   form_flags[];
extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   off_flags[];
extern const struct flag_type   imm_flags[];
extern const struct flag_type   res_flags[];
extern const struct flag_type   vuln_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   weapon_type[];
extern const struct flag_type	attack_type[];
extern const struct flag_type	damage_type[];
extern const struct flag_type	sector_name[];
extern const struct flag_type	mprog_flags[];
extern const struct flag_type   oprog_flags[];
extern const struct flag_type   rprog_flags[];
extern const struct flag_type	vflags_armor[];

#endif /* MERC_H */
