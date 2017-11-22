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
#include "magic.h"

/* Zeran     - material table */
/* Lotherius - AC values subject to tweaking... I guessed on them all late at night. */
/* AC Value is 0 to 200, and represents AC*2 of the item against that damtype        */
/* Should assume all items are 4 by default unless it specifically blocks casting    */
/* Yes this table is getting complicated. 					     */

const struct material_data material_table[] =
{
     /*
      { material name,	material_type, 		vuln_flag,	durability,	repair,		pierce,slash,bash,exotic,
      flags } */
     { 
          "wood", 	MATERIAL_WOOD,		VULN_WOOD,	DUR_BAD, 	REP_EASY,	75,  85,  55,  20,
               MAT_FLAMMABLE | MAT_ACIDETCH | MAT_CARVE 
     },
     { 
          "iron", 	MATERIAL_IRON,  	VULN_IRON,	DUR_AVGPLUS, 	REP_AVGPLUS,	150, 175, 125, 60,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_MELT_VHOT | MAT_NOMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "silver", 	MATERIAL_SILVER, 	VULN_SILVER,	DUR_AVGPLUS, 	REP_HARD,	145, 160, 135, 160,
               MAT_BENDABLE | MAT_MELT_VHOT | MAT_PARTMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "gold", 	MATERIAL_GOLD, 		0, 		DUR_AVGMINUS, 	REP_HARD,	105, 90,  65,  90,
               MAT_BENDABLE | MAT_MELT_NORMAL | MAT_PARTMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "adamantite", MATERIAL_ADAMANTITE, 	VULN_ADAMANTITE, DUR_EXCEL, 	REP_EXTREME,	195, 195, 195, 180,
               MAT_MELT_MAGICAL | MAT_NOMAGIC | MAT_FORGE | MAT_VERYHARD | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "cloth", 	MATERIAL_CLOTH, 	0, 		DUR_AWFUL, 	REP_BREEZE,	0,   5,   5,   5,
               MAT_FLAMMABLE | MAT_SEW | MAT_SOFT | MAT_ACIDETCH 
     },
     { 
          "glass", 	MATERIAL_GLASS, 	0, 		DUR_WALMART, 	REP_HARD,	80,  25,  0,   5,
               MAT_MELT_NORMAL | MAT_NOMAGIC | MAT_BREAKABLE | MAT_ACIDETCH
     },
     {
          "ceramic",	MATERIAL_CERAMIC,	0,		DUR_BAD,	REP_AVGPLUS,    80,  15,  1,   0,
               MAT_MELT_NORMAL | MAT_NOMAGIC | MAT_BREAKABLE | MAT_ACIDETCH
     },
     { 
          "food", 	MATERIAL_FOOD, 		0, 		DUR_WALMART, 	REP_IMPOSSIBLE,	0,   0,   0,   0,
               MAT_FLAMMABLE | MAT_ACIDETCH
     },
     { 
          "liquid", 	MATERIAL_LIQUID, 	0, 		DUR_AVG, 	REP_IMPOSSIBLE,	0,   0,   0,   0,
               MAT_LIQUID | MAT_CONDUCTIVE
     },
     {
          "platinum",	MATERIAL_PLATINUM,	0,		DUR_EXCEL,	REP_HARD,	175, 165, 155, 115,
               MAT_BENDABLE | MAT_MELT_VHOT | MAT_FORGE | MAT_NOMAGIC | MAT_VERYHARD | MAT_METAL | MAT_CONDUCTIVE
     },
     {
          "mithril", 	MATERIAL_MITHRIL, 	VULN_MITHRIL, 	DUR_JEEP, 	REP_EXTREME,	155, 165, 165, 200,
               MAT_BENDABLE | MAT_MELT_MAGICAL | MAT_FORGE | MAT_PARTMAGIC | MAT_VERYHARD | MAT_METAL | MAT_MAGIC | MAT_CONDUCTIVE
     },
     { 
          "steel", 	MATERIAL_STEEL, 	VULN_STEEL, 	DUR_GOOD, 	REP_AVGPLUS,	130, 175, 150, 65,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_MELT_VHOT | MAT_FORGE | MAT_NOMAGIC | MAT_VERYHARD | MAT_METAL | MAT_ACIDETCH | MAT_CONDUCTIVE
     },
     { 
          "paper",	MATERIAL_PAPER, 	0, 		DUR_WALMART, 	REP_EASY,	0,   0,   0,   0,
               MAT_FLAMMABLE | MAT_INKBLEED | MAT_WRITEON | MAT_ACIDETCH
     },
     /* Use meat for dead, flesh for living */
     { 
          "meat", 	MATERIAL_MEAT, 		0, 		DUR_BAD, 	REP_IMPOSSIBLE,	1,   1,   2,   0,
               MAT_FLAMMABLE | MAT_SOFT | MAT_ACIDETCH | MAT_NOMAGIC
     },
     { 
          "flesh", 	MATERIAL_FLESH, 	0, 		DUR_BAD, 	REP_HARD,	1,   1,   2,   0,
               MAT_FLAMMABLE | MAT_LIVING | MAT_SOFT | MAT_ACIDETCH | MAT_WRITEON | MAT_NOMAGIC
     },        
     { 
          "leather", 	MATERIAL_LEATHER, 	0, 		DUR_AVG, 	REP_AVG,	60,  85,  65,  15,
               MAT_FLAMMABLE | MAT_NOMAGIC | MAT_SOFT | MAT_SEW | MAT_ACIDETCH | MAT_WRITEON
     },
     { 
          "pill", 	MATERIAL_PILL, 		0, 		DUR_BAD, 	REP_AVGMINUS,	0,   0,   0,   0,
               MAT_FLAMMABLE | MAT_SOFT | MAT_DISSOLVE | MAT_ACIDETCH | MAT_MAGIC
     },          
     { 
          "vellum", 	MATERIAL_VELLUM, 	0, 		DUR_AVG, 	REP_AVGPLUS,	0,   1,   1,   1,
               MAT_FLAMMABLE | MAT_INKBLEED | MAT_SOFT | MAT_ACIDETCH | MAT_MAGIC | MAT_WRITEON
     },
     { 
          "bronze", 	MATERIAL_BRONZE,	0, 		DUR_AVG, 	REP_AVG,	100, 95,  85,  30,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_MELT_VHOT | MAT_NOMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "brass", 	MATERIAL_BRASS, 	0, 		DUR_AVGMINUS, 	REP_AVG,	80,  95,  70,  25,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_MELT_NORMAL | MAT_NOMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "stone", 	MATERIAL_STONE, 	0, 		DUR_AVG, 	REP_AVGPLUS,	150, 100, 40,  5,
               MAT_MELT_MAGICAL | MAT_NOMAGIC | MAT_BREAKABLE | MAT_ACIDETCH | MAT_ROCK | MAT_CARVE
     },               
     { 
          "bone", 	MATERIAL_BONE, 		0, 		DUR_BAD, 	REP_HARD,	85,  85,  20,  90,
               MAT_FLAMMABLE | MAT_PARTMAGIC | MAT_BREAKABLE | MAT_ACIDETCH | MAT_CARVE
     },
     { 
          "unique", 	MATERIAL_UNIQUE, 	0, 		DUR_AVG, 	REP_IMPOSSIBLE,	100, 100, 100, 100,
               MAT_ACIDETCH 
     },
     { 
          "ice", 	MATERIAL_ICE, 		0, 		DUR_WALMART, 	REP_EASY,	30,  25,  5,   5,
               MAT_BREAKABLE | MAT_MELT_ALWAYS | MAT_CARVE | MAT_CONDUCTIVE
     },
     { 
          "rubber", 	MATERIAL_RUBBER, 	0, 		DUR_AVG, 	REP_AVGMINUS,	20,  95,  95,  30,
               MAT_FLAMMABLE | MAT_MELT_NORMAL | MAT_NOMAGIC | MAT_SEW | MAT_SOFT 
     },
     { 
          "plasteel", 	MATERIAL_PLASTEEL, 	0, 		DUR_EXCEL, 	REP_HARD,	180, 160, 120, 135,
               MAT_MELT_VHOT | MAT_NOMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_BENDABLE 
     },
     { 
          "fur", 	MATERIAL_FUR, 		0, 		DUR_AWFUL, 	REP_EASY,	50,  45,  50,  15,
               MAT_FLAMMABLE | MAT_PARTMAGIC | MAT_SEW | MAT_SOFT 
     },
     { 
          "marble",	MATERIAL_MARBLE,	0,		DUR_EXCEL,	REP_EXTREME,	165, 100, 45,  15,
               MAT_MELT_VHOT | MAT_NOMAGIC | MAT_BREAKABLE | MAT_ACIDETCH | MAT_ROCK | MAT_CARVE
     },
     { 
          "granite",	MATERIAL_GRANITE,	0,		DUR_GOOD,	REP_EXTREME,	160, 100, 40,  10,
               MAT_MELT_VHOT | MAT_NOMAGIC | MAT_BREAKABLE | MAT_ACIDETCH | MAT_ROCK | MAT_CARVE
     },
     { 
          "ivory",	MATERIAL_IVORY,		0,		DUR_BAD,	REP_HARD,	80,  20,  35,  105,
               MAT_PARTMAGIC | MAT_ACIDETCH | MAT_CARVE | MAT_BREAKABLE
     },
     { 
          "dirt",	MATERIAL_DIRT,		0,		DUR_WALMART,	REP_BREEZE,	5,   5,   5,   0,
               MAT_DISSOLVE | MAT_MELT_VHOT | MAT_SOFT | MAT_ACIDETCH 
     },
     { 
          "chalk",	MATERIAL_CHALK,		0,		DUR_WALMART,	REP_HARD,	0,   0,   1,   0,
               MAT_DISSOLVE | MAT_BREAKABLE | MAT_MELT_VHOT | MAT_SOFT | MAT_ACIDETCH | MAT_WRITE | MAT_CARVE | MAT_ROCK 
     },
     { 
          "silk",	MATERIAL_SILK,		0,		DUR_AVGPLUS,	REP_AVGPLUS,	0,   10,  15,  75,
               MAT_FLAMMABLE | MAT_SEW | MAT_SOFT | MAT_ACIDETCH
     },
     { 
          "feathers",	MATERIAL_FEATHER,	0,		DUR_WALMART,	REP_IMPOSSIBLE,	0,   5,   5,   0,
               MAT_FLAMMABLE | MAT_SEW | MAT_SOFT | MAT_ACIDETCH 
     },
     { 
          "copper",	MATERIAL_COPPER,	0,		DUR_GOOD,	REP_HARD,	95,  85,  95,  30,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_NOMAGIC | MAT_MELT_VHOT | MAT_ACIDETCH | MAT_FORGE | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "aluminum",	MATERIAL_ALUMINUM,	0,		DUR_AVGMINUS,	REP_AVGMINUS,	75,  80,  70,  30,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_NOMAGIC | MAT_MELT_NORMAL | MAT_ACIDETCH | MAT_FORGE | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "tin",	MATERIAL_TIN,		0,		DUR_AVG,	REP_AVG,	85,  95,  95,  30,
               MAT_RUSTABLE | MAT_BENDABLE | MAT_NOMAGIC | MAT_MELT_VHOT | MAT_ACIDETCH | MAT_FORGE | MAT_METAL | MAT_CONDUCTIVE
     },
     {
          "white gold",	MATERIAL_WGOLD,		0,		DUR_AVG,	REP_HARD,	125, 105, 85, 130,
               MAT_BENDABLE | MAT_MELT_VHOT | MAT_PARTMAGIC | MAT_FORGE | MAT_ACIDETCH | MAT_METAL | MAT_CONDUCTIVE
     },
     { 
          "crystal", 	MATERIAL_CRYSTAL, 	0, 		DUR_GOOD, 	REP_HARD,	135, 35,  35,  95,
               MAT_BREAKABLE | MAT_MELT_VHOT | MAT_ROCK
     },
     { 
          "diamond", 	MATERIAL_DIAMOND, 	0, 		DUR_EXCEL, 	REP_EXTREME,	200, 200, 125, 105,
               MAT_VERYHARD | MAT_ROCK
     },
     {
          "emerald",	MATERIAL_EMERALD,	0,		DUR_GOOD,	REP_EXTREME,	115, 45,  45,  95,
               MAT_ROCK | MAT_PARTMAGIC | MAT_MELT_MAGICAL | MAT_BREAKABLE
     },
     {
          "topaz",	MATERIAL_TOPAZ,		0,		DUR_GOOD,	REP_EXTREME,	105, 30,  35,  75,
               MAT_ROCK | MAT_MELT_VHOT | MAT_BREAKABLE | MAT_PARTMAGIC
     },
     {
          "obsidian",	MATERIAL_OBSIDIAN,	0,		DUR_AVGMINUS,	REP_HARD,	95,  30,  30,  125,
               MAT_ROCK | MAT_MELT_VHOT | MAT_BREAKABLE
     },
     { 
          "unknown", 	0, 			0, 		DUR_AVG, 	REP_IMPOSSIBLE,	5,  5,  5,  5,
               MAT_MELT_MAGICAL | MAT_NOMAGIC | MAT_ACIDETCH
     }
};

// Lotherius - An attempt to organize the mess of item flags into some order.
// 
// TWO HANDED weapons require special consideration.
// 
// This struct MUST be in the same sequence as the wear flags. (WEAR_*)
// Changing the order in either place will break everything.
// 
// Also, do not combine any flags here. Each field in this structure REQUIRES only a
// single flag be marked to work correctly.
// 
// "supercede" may require explanation... Basically, the listed body part gets hit "generally"
// first before the part listed.. thus WEAR_ABOUT gets hit before WEAR_BODY... Generally an
// object is "underneath" the object which supercedes it.
// 
// Neck is a special case... Neck 2 is considered to be "over" neck 1.
//struct wear_data
//{
//     char *name;        // Name of the body part or location
//     bool ispart;       // Is this a body part?
//     int  hitpct;       // Percent chance to hit (all must add up to 100%)
//     long wear_flag;    // Which piece of armor relates to this body part.
//     long item_flag;    // Flag on associated items.
//     long part_req;     // Flag required on target to be protected on this part.     
//     bool has_ac;       // Is this body part protectable by armor (Has its own AC)
//     long supercede;    // Wear slot that supercedes this one (is outside of)          
//     //                 // Determines what AC to use and and which eq to damage          
//};


// Remember... position here is determined by the WEAR_ flag.
const struct wear_data wear_info[] =
{
//      partname	ispart	%hit	wear flag	item flag 		required part	AC?	superceded by
//   { "none",		FALSE,	0,	WEAR_NONE,	0,			0,		FALSE,	0		},
     { "light",		FALSE,	0,	WEAR_LIGHT,	ITEM_HOLD,		PART_HANDS,	FALSE,	0 		},
     { "left finger", 	TRUE,	3,	WEAR_FINGER_L, 	ITEM_WEAR_FINGER,	PART_FINGERS,	FALSE,	WEAR_HANDS  	},
     { "right finger",	TRUE,	4,	WEAR_FINGER_R, 	ITEM_WEAR_FINGER,	PART_FINGERS,	FALSE,	WEAR_HANDS  	},
     { "neck",		TRUE,	3,	WEAR_NECK_1, 	ITEM_WEAR_NECK,		PART_NECK,	TRUE,	WEAR_NECK_2 	},
     { "neck",		TRUE,	0,	WEAR_NECK_2,	ITEM_WEAR_NECK,		PART_NECK,	TRUE,	0	 	},
     { "body",		TRUE,	20,	WEAR_BODY,	ITEM_WEAR_BODY,		0,		TRUE,	WEAR_ABOUT 	},
     { "head",	        TRUE,	10,	WEAR_HEAD,	ITEM_WEAR_HEAD,		PART_HEAD,	TRUE,	0 		},
     { "legs",		TRUE,	5,	WEAR_LEGS, 	ITEM_WEAR_LEGS,		PART_LEGS,	TRUE,	WEAR_ABOUT 	},
     { "feet",		TRUE,	2,	WEAR_FEET,	ITEM_WEAR_FEET,		PART_FEET,	TRUE,	0 		},
     { "hands",		TRUE,	5,	WEAR_HANDS,	ITEM_WEAR_HANDS, 	PART_HANDS,	TRUE,	0 		},
     { "arms",		TRUE,	25,	WEAR_ARMS,	ITEM_WEAR_ARMS, 	PART_ARMS,	TRUE,	WEAR_ABOUT 	},
     { "shield",	FALSE,	0,	WEAR_SHIELD,	ITEM_WEAR_SHIELD,	PART_ARMS,	FALSE,	0 		},
     { "about body",	FALSE,	0,	WEAR_ABOUT,	ITEM_WEAR_ABOUT,	0,		TRUE,	0 		},
     { "waist",		TRUE,	5,	WEAR_WAIST,	ITEM_WEAR_WAIST,	PART_WAIST,	TRUE,	WEAR_ABOUT 	},
     { "left wrist",    TRUE,	5,	WEAR_WRIST_L,	ITEM_WEAR_WRIST,	PART_WRIST,	TRUE,	WEAR_HANDS 	},
     { "right wrist",	TRUE,	5,	WEAR_WRIST_R,	ITEM_WEAR_WRIST,	PART_WRIST,	TRUE,	WEAR_HANDS 	},
     { "wielded",	FALSE,	0,	WEAR_WIELD,	ITEM_WIELD,		PART_HANDS,	FALSE,  0 		},
     { "held",		FALSE,	0,	WEAR_HOLD,	ITEM_HOLD,		PART_HANDS,	FALSE,	0 		},
     { "dual wield",	FALSE,	0,	WEAR_WIELD2,	ITEM_WIELD,		PART_HANDS,	FALSE,	0 		},
     { "pride",		FALSE,	0,	WEAR_PRIDE,	ITEM_WEAR_PRIDE,	0,		FALSE,	0 		},
     { "face",		TRUE,	5,	WEAR_FACE,	ITEM_WEAR_FACE,		PART_FACE,	TRUE,	0 		},
     { "ears",		TRUE,	3,	WEAR_EARS,	ITEM_WEAR_EARS,		PART_EAR,	FALSE,	0 		},
     { "float",		FALSE,	0,	WEAR_FLOAT,	ITEM_WEAR_FLOAT,	0,		FALSE,	0 		},
       // Insert New Entries Here and update WEAR_ flags in merc.h
     { "max wear",	FALSE,	0,	MAX_WEAR,	0,			0,		FALSE,	0		}
};

/* Lotherius - day_name table moved to const.c where it should be. */
const char         *day_name[] =
{
     "the Moon", "the Cricket", "Deception", "Rebirth", "Cinda",
          "the Gods", "the Sun"
};

/* Lotherius - month_name table moved to const.c where it should be. */
const char         *month_name[] =
{
     "Winter", "Winter Wolf", "Frost Giant", "the Old Forces",
          "Grand Struggle", "Rebirth", "Nature", "Futility", "the Dragon",
          "Sun", "Heat", "Battle", "Dark Shades", "Shadows",
          "Long Shadows", "Darkness", "Great Evil"
};

/* Zeran - size name table */
const char         *size_table[] =
{
     "tiny",
          "small",
          "medium",
          "large",
          "huge",
          "giant"
};

/* attack table  */
const struct attack_type attack_table[] =
{
     {"hit", 		-1},		/*  0 */
     {"slice", 		DAM_SLASH},
     {"stab", 		DAM_PIERCE},
     {"slash", 		DAM_SLASH},
     {"whip", 		DAM_SLASH},
     {"claw", 		DAM_SLASH},	/*  5 */
     {"blast", 		DAM_BASH},
     {"pound", 		DAM_BASH},
     {"crush", 		DAM_BASH},
     {"grep", 		DAM_SLASH},
     {"bite", 		DAM_PIERCE},	/* 10 */
     {"pierce", 	DAM_PIERCE},
     {"suction", 	DAM_BASH},
     {"beating", 	DAM_BASH},
     {"digestion", 	DAM_ACID},
     {"charge", 	DAM_BASH},	/* 15 */
     {"slap", 		DAM_BASH},
     {"punch", 		DAM_HANDTOHAND},
     {"wrath", 		DAM_ENERGY},
     {"magic", 		DAM_ENERGY},
     {"divine power", 	DAM_HOLY},	/* 20 */
     {"cleave", 	DAM_SLASH},
     {"scratch", 	DAM_PIERCE},
     {"peck",	 	DAM_PIERCE},
     {"peck", 		DAM_BASH},
     {"chop", 		DAM_SLASH},	/* 25 */
     {"sting", 		DAM_PIERCE},
     {"smash", 		DAM_BASH},
     {"shocking bite", 	DAM_LIGHTNING},
     {"flaming bite", 	DAM_FIRE},
     {"freezing bite", 	DAM_COLD},	/* 30 */
     {"acidic bite", 	DAM_ACID},
     {"chomp", 		DAM_PIERCE},
     {"left wing", 	DAM_SLASH},	/*start of special race attacks */
     {"right wing", 	DAM_SLASH},
     {"hooves", 	DAM_BASH},	/* 35 */
     {"horns", 		DAM_PIERCE},
     {"flaming aura", 	DAM_FIRE}
};

/* race table */

struct race_type    race_table[] =
{

/*
    {
	name,		pc_race?,
	act bits,	aff bits,	protect, detect, off bits,
	imm,		res,		vuln,
	form,		parts,          encumbrance
    },
*/
    /* 1 */
     {"unique", FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 2 */
     {
          "human", TRUE,
               0, 0, 0, 0, 0,
               0, 0, 0,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

    /* 3 */
     {
          "azer", TRUE,
               0, 0, 0, 0, 0,
               0, RES_FIRE, VULN_COLD,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | L | R | S | T | aa,
               1
     },

/* 4 */
     {
          "centaur", TRUE,
               0, 0, 0, 0, OFF_FAST,
               0, RES_BASH, VULN_MENTAL,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | Z | R | S | T | aa,
               1
     },

/* 5 */
     {
          "kobold", TRUE,
               0, 0,  0, 0,0,
               0, RES_POISON, VULN_MAGIC,
               A | B | H | M | V,
               A | B | C | D | E | F | G | H | I | J | K | Q | R | S | T | aa, 1
     },

/* 6 */
     {
          "gargoyle", TRUE,
               0, 0, 0, 0, 0,
               0, RES_HOLY, VULN_SILVER,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | P | R | S | T | aa,
               1
     },

/* 7 */
     {
          "elf", TRUE,
               0, 0, 0, 0, 0,
               0, RES_CHARM, VULN_IRON,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 8 */
     {
          "dwarf", TRUE,
               0, 0, 0, 0, 0,
               0, RES_MAGIC | RES_POISON | RES_DISEASE, VULN_DROWNING,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 9 */
     {
          "giant", TRUE,
               0, 0, 0, 0, 0,
               0, RES_FIRE | RES_COLD, VULN_MENTAL | VULN_LIGHTNING,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 10 */
     {
          "kender", TRUE,
               ACT_THIEF, 0, 0, 0,
               OFF_KICK_DIRT | OFF_DODGE | OFF_KICK | OFF_FAST | ASSIST_RACE,
               0, RES_NEGATIVE, VULN_MENTAL,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 11 */
     {
          "bat", FALSE,
               0, 0,  0, 0, OFF_DODGE | OFF_FAST,
               0, 0, VULN_LIGHT,
               A | G | W, A | C | D | E | F | H | J | K | P | R | S | T | aa, 1
     },

/* 12 */
     {
          "bear", FALSE,
               0, 0,  0, 0, OFF_CRUSH | OFF_DISARM | OFF_BERSERK,
               0, RES_BASH | RES_COLD, 0,
               A | G | V, A | B | C | D | E | F | H | J | K | U | V | R | S | T | aa, 1
     },

/* 13 */
     {
          "cat", FALSE,
               0, 0,  0, 0, OFF_FAST | OFF_DODGE,
               0, 0, 0,
               A | G | V, A | C | D | E | F | H | J | K | Q | U | V | R | S | T | aa, 1
     },

/* 14 */
     {
          "centipede", FALSE,
               0, 0, 0, 0, 0,
               0, RES_PIERCE | RES_COLD, VULN_BASH,
               0, 0, 1
     },

/* 15 */
     {
          "dog", FALSE,
               0, 0,  0, 0,OFF_FAST,
               0, 0, 0,
               A | G | V, A | C | D | E | F | H | J | K | U | V | R | aa, 1
     },

/* 16 */
     {
          "doll", FALSE,
               0, 0, 0, 0, 0,
               IMM_MAGIC, RES_BASH | RES_LIGHT,
               VULN_SLASH | VULN_FIRE | VULN_ACID | VULN_LIGHTNING |
               VULN_ENERGY,
               E | J | M | cc, A | B | C | G | H | K | aa, 1
     },

/* 17 */
     {
          "fido", FALSE,
               0, 0, 0, 0, OFF_DODGE | ASSIST_RACE,
               0, 0, VULN_MAGIC,
               B | G | V, A | C | D | E | F | H | J | K | Q | V | R | aa, 1
     },

/* 18 */
     {
          "fox", FALSE,
               0, 0,  0, 0,OFF_FAST | OFF_DODGE,
               0, 0, 0,
               A | G | V, A | C | D | E | F | H | J | K | Q | V | R | aa, 1
     },

/* 19 */
     {
          "goblin", FALSE,
               0, 0, 0, 0, 0,
               0, RES_DISEASE, VULN_MAGIC,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 20 */
     {
          "hobgoblin", FALSE,
               0, 0, 0, 0, 0,
               0, RES_DISEASE | RES_POISON, 0,
               A | B | H | M | V,
               A | B | C | D | E | F | G | H | I | J | K | Q | R | S | T | aa, 1
     },

/* 21 */

     {
          "satyr", FALSE,
               0, 0, 0, 0, 0,
               0, RES_CHARM | RES_POISON, VULN_IRON,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | W | R | S | T | aa,
               1
     },

/* 22 */
     {
          "lizard", FALSE,
               0, 0, 0, 0, 0,
               0, RES_POISON, VULN_COLD,
               A | G | X | cc, A | C | D | E | F | H | K | Q | V, 1
     },

/* 23 */
     {
          "modron", FALSE,
               0, 0, 0, 0, ASSIST_RACE | ASSIST_ALIGN,
               IMM_CHARM | IMM_DISEASE | IMM_MENTAL | IMM_HOLY |
               IMM_NEGATIVE,
               RES_FIRE | RES_COLD | RES_ACID, 0,
               H, A | B | C | G | H | J | K, 1
     },

/* 24 */
     {
          "orc", FALSE,
               0, 0, 0, 0, 0,
               0, RES_DISEASE, VULN_LIGHT,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 27 */
     {
          "pig", FALSE,
               0, 0, 0, 0, 0,
               0, 0, 0,
               A | G | V, A | C | D | E | F | H | J | K | R | aa, 1
     },

/* 26 */
     {
          "rabbit", FALSE,
               0, 0, 0, 0, OFF_DODGE | OFF_FAST,
               0, 0, 0,
               A | G | V, A | C | D | E | F | H | J | K | R | aa, 1
     },

/* 27 */
     {
          "school monster", FALSE,
               ACT_NOALIGN, 0, 0, 0, 0,
               IMM_CHARM | IMM_SUMMON, 0, VULN_MAGIC,
               A | M | V, A | B | C | D | E | F | H | J | K | Q | U | R | aa, 1
     },

/* 28 */
     {
          "snake", FALSE,
               0, 0, 0, 0, 0,
               0, RES_POISON, VULN_COLD,
               A | G | R | X | Y | cc, A | D | E | F | K | L | Q | V | X, 1
     },

/* 29 */
     {
          "song bird", FALSE,
               0, 0, 0, 0, OFF_FAST | OFF_DODGE,
               0, 0, 0,
               A | G | W, A | C | D | E | F | H | K | P | R | aa, 1
     },

/* 30 */
     {
          "troll", FALSE,
               0, 0, 0, 0, OFF_BERSERK,
               0, RES_CHARM | RES_BASH, VULN_FIRE | VULN_ACID,
               B | M | V, A | B | C | D | E | F | G | H | I | J | K | U | V | R | S | T | aa,
               1
     },

/* 31 */
     {
          "water fowl", FALSE,
               0, 0, 0, 0, 0,
               0, RES_DROWNING, 0,
               A | G | W, A | C | D | E | F | H | K | P | R | aa, 1
     },

/* 32 */
     {
          "wolf", FALSE,
               0, 0, 0, 0, OFF_FAST | OFF_DODGE,
               0, 0, 0,
               A | G | V, A | C | D | E | F | J | K | Q | V | R | aa, 1
     },

/* 33 */
     {
          "wyvern", FALSE,
               0, 0, 0, 0, OFF_BASH | OFF_FAST | OFF_DODGE,
               IMM_POISON, 0, VULN_LIGHT,
               B | Z | cc, A | C | D | E | F | H | J | K | Q | V | X | R | aa, 1
     },

/* 34 */
     {
          "undead", FALSE,
               ACT_UNDEAD, 0, 0, 0, OFF_FADE,
               IMM_CHARM | IMM_POISON | IMM_MENTAL | IMM_DISEASE |
               IMM_DROWNING, 0,
               VULN_FIRE | VULN_HOLY,
               D | I, A | B | C | G | H | I | R | S | T | aa, 1
     },

/* 35 */
     {
          "demon", FALSE,
               0, 0, 0, 0, OFF_BACKSTAB | OFF_DODGE | OFF_TAIL | ASSIST_RACE,
               IMM_DISEASE, RES_CHARM, VULN_HOLY | VULN_MITHRIL,
               B | H | M,
               A | B | C | D | E | F | G | H | I | J | K | L | Q | U | V | W | R | S | T | aa,
               1
     },

/* 36 */
     {
          "avatar", FALSE,
               0, 0, 0, 0, OFF_PARRY | ASSIST_ALIGN | ASSIST_RACE,
               IMM_HOLY, RES_DISEASE | RES_CHARM, VULN_NEGATIVE,
               C | H | M, A | B | C | D | E | F | G | H | I | J | K | P | R | S | T | aa, 1
     },

/* 37 */
     {"generic", FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},

/* 38 */

     {
          "gnome", FALSE,
               0, 0, 0, 0, 0,
               0, RES_MAGIC | RES_POISON | RES_DISEASE | RES_MENTAL,
               VULN_MITHRIL,
               A | H | M | V, A | B | C | D | E | F | G | H | I | J | K | R | S | T | aa, 1
     },

/* 39 */
     {
          "vampire", FALSE,
               ACT_UNDEAD | ACT_CLERIC, 0, 0, 0, OFF_DISARM | OFF_FAST | OFF_PARRY,
               IMM_CHARM | IMM_POISON | IMM_MENTAL | IMM_DISEASE,
               RES_COLD | RES_LIGHTNING,
               VULN_FIRE | VULN_HOLY | VULN_LIGHT | VULN_WOOD,
               H | I | M, A | B | C | D | E | G | H | I | J | K | V | R | S | T | aa, 1
     },

/* 40 */
     {
          "minotaur", FALSE,
               ACT_WARRIOR, 0, 0, 0, OFF_BERSERK | OFF_CRUSH,
               0, RES_CHARM, 0,
               A | H | M | V,
               A | B | C | D | E | F | G | H | I | J | K | W | Z | R | S | T | aa, 1
     },

/*
    {
        name,           pc_race?,
        act bits,       aff_by bits, detect_bits, protect_bits,    off bits,
        imm,            res,            vuln,
        form,           parts
    },
*/
     {
          NULL, 0, 0, 0, 0, 0, 0
     }
};

/* for a balanced race, all bonuses and penalties should add to 0 */
/* CP are original total + 1 for every 5 years past 41 */

/* IF YOU ADD A PC RACE YOU MUST INCREASE MAX_PCRACE IN MERC.H BY 1 */

const struct pc_race_type pc_race_table[] =
{
     {"null race", "", 0, 0, 0, {100, 100, 100, 100},
          {""}, 0, 0, {13, 13, 13, 13, 13}, {18, 18, 18, 18, 18}, 0},

/*
    {
	"race name", 	short name, 	points, max age, start age, { class multipliers },
	{ bonus skills },	recall, 	healer,
	{ base stats },		{ max stats },		size
    },
*/

     {
          "human", "Human", 0, 75, 17, {100, 100, 100, 100, 100, 100, 100},
          {""}, 2093, 2096,
          {0, 0, 0, 0, 0}, {20, 20, 20, 20, 20}, SIZE_MEDIUM},

     {
          "azer", "Azer ", 19, 90, 22, {100, 100, 100, 100, 100, 100, 100},
          {"meditation", "fast healing"}, 2093, 2096,
          {2, 0, -2, -2, 2}, {22, 20, 18, 18, 22}, SIZE_MEDIUM},

     {
          "centaur", "Ctaur", 15, 60, 11, {100, 100, 100, 100, 100, 100, 100},
          {"bash", "dodge"}, 2093, 2096,
          {2, 0, 0, -4, 2}, {22, 20, 20, 16, 22}, SIZE_LARGE},

     {
          "kobold", "Kobold", 1, 50, 9, {100, 100, 100, 100, 100, 100, 100},
          {"hide", "dirt kicking", "rally" }, 2093, 2096,
          {-4, -4, -4, 8, -4}, {14, 16, 16, 23, 12}, SIZE_SMALL},

     {
          "gargoyle", "Grgyl", 31, 200, 60, {100, 100, 100, 100, 100, 100, 100},
          {"meditation" }, 2093, 2096,
          {1, -2, 4, -3, 0}, {21, 18, 24, 17, 20}, SIZE_LARGE},

     {
          "elf", " Elf ", 35, 250, 50, {100, 100, 100, 100, 100, 100, 100},
          {"sneak", "hide"}, 500, 473,
          {-1, 1, 0, 2, -2}, {16, 20, 18, 21, 15}, SIZE_MEDIUM},

     {
          "dwarf", "Dwarf", 15, 95, 20, {100, 100, 100, 100, 100, 100, 100},
          {"berserk"}, 2093, 2096,
          {1, -1, 1, -3, 2}, {20, 16, 19, 14, 21}, SIZE_MEDIUM},

     {
          "giant", "Giant", 8, 65, 15, {100, 100, 100, 100, 100, 100, 100},
          {"bash", "fast healing"}, 2093, 2096,
          {3, -2, 0, -2, 1}, {22, 15, 18, 15, 20}, SIZE_HUGE},

     {
          "kender", "Kender", 12, 75, 15, {100, 100, 100, 100, 100, 100, 100},
          {"steal", "peek"}, 2093, 2096,
          {-5, 0, -2, +7, 0}, {13, 18, 16, 23, 18}, SIZE_SMALL}

/*
    {
        "race name",    short name,     points, max age, start age, { class multipliers },
        { bonus skills },       recall,         healer,
        { base stats },         { max stats },          size
    },
*/

};

/*
         class list is as follows:
           mage: Traditional Magic-User
           avenger: Priests of Aecindo & Lukhan
           warrior: Traditional Warrior
           thief: Traditional Thief
           monk: A warrior who fights with wit & skin
           defiler: Priests of Thuahamin & Roqmin
           chaosmage: Mages who went a bit too far
*/

/*
    { "name", "nam", primstat, weapon, {unused,unused},
	skill_adept, thac0_00, thac0_32, hp_min, hp_max, manabool }
*/

/*
 * Class table.
 */
const struct class_type class_table[MAX_CLASS] =
{
     {
          "mage", "Mag", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER,
          {1, 1}, 65, 18, 6, 6, 24, TRUE},

     {
          "avenger", "Ave", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,
          {1, 1}, 60, 18, 2, 7, 30, TRUE},

     {
          "thief", "Thi", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
          {1, 1}, 55, 18, -4, 8, 39, FALSE},

     {
          "warrior", "War", STAT_STR, OBJ_VNUM_SCHOOL_SWORD,
          {1, 1}, 60, 17, -11, 11, 45, FALSE},

     {
          "monk", "Mon", STAT_CON, OBJ_VNUM_SCHOOL_SWORD,
          {1, 1}, 65, 18, 3, 15, 50, TRUE},

     {
          "defiler", "Def", STAT_WIS, OBJ_VNUM_SCHOOL_MACE,
          {1, 1}, 60, 18, 2, 7, 30, TRUE},

     {
          "chaosmage", "Cha", STAT_INT, OBJ_VNUM_SCHOOL_DAGGER,
          {1, 1}, 55, 19, 7, 6, 24, TRUE}

};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] =
{
     /* to_hit	to_dam	carry	wield */
     {	-5, 	-4, 	0, 	0},		/* 0  */
     {	-5, 	-4, 	3, 	1},		/* 1  */
     {	-3, 	-2, 	3, 	2},
     {	-3, 	-1, 	10,	3},		/* 3  */
     {	-2, 	-1, 	25, 	4},
     {	-2, 	-1, 	55, 	5},		/* 5  */
     {	-1, 	0, 	100, 	6},
     {	-1, 	0, 	115,	7},
     {	0, 	0, 	115, 	8},
     {	0, 	0, 	130, 	9},
     {	0, 	0, 	130,	10},		/* 10  */
     {	0, 	0, 	140, 	11},
     {	0,	0, 	150, 	12},
     {	0, 	0, 	170,	13},		/* 13  */
     {	0, 	1, 	195,	14},
     {	1, 	1, 	225, 	15},		/* 15  */
     {	1, 	2, 	245, 	16},
     {	2, 	3, 	275,	22},
     {	2, 	3, 	325, 	25},		/* 18  */
     {	3, 	4, 	350, 	30},
     {	3, 	5, 	400, 	35},		/* 20  */
     {	4, 	6, 	425, 	40},
     {	4, 	6,	450, 	45},
     {	5, 	7, 	475, 	50},
     {	5, 	8, 	500, 	55},
     {	6, 	9, 	525, 	60}		/* 25   */
};

const struct int_app_type int_app[26] =
{
     // learn, mspel, maxlanguage ... on language we'll conced common AND racial
     {3, -100, 2},			/*  0 */
     {5, -90,  2},			/*  1 */
     {7, -85,  2},
     {8, -75,  2},			/*  3 */
     {9, -50,  2},
     {10, -30, 2},			/*  5 */
     {11, -20, 2},
     {12, -15, 2},
     {13, -10, 2},
     {15, -5,  2},
     {17, 0,   2},			/* 10 */
     {19, 5,   2},
     {22, 10,  2},
     {25, 15,  3},
     {28, 20,  4},
     {31, 25,  4},			/* 15 */
     {34, 30,  5},
     {37, 35,  6},
     {40, 40,  7},			/* 18 */
     {44, 45,  7},
     {49, 50,  8},			/* 20 */
     {55, 55,  8},
     {60, 60,  8},
     {70, 65,  8},
     {80, 70,  9},
     {85, 75,  9}			/* 25 */
};

const struct wis_app_type wis_app[26] =
{
     {0, -100, 5},			/*  0 */
     {0, -90, 10},			/*  1 */
     {0, -85, 15},
     {0, -75, 20},			/*  3 */
     {0, -50, 23},
     {1, -30, 25},			/*  5 */
     {1, -20, 27},
     {1, -15, 30},
     {1, -10, 33},
     {1, -5, 35},
     {2, 0,  40},			/* 10 */
     {2, 5,  45},
     {2, 10, 50},
     {2, 15, 55},
     {2, 20, 60},
     {3, 25, 65},			/* 15 */
     {3, 30, 70},
     {3, 35, 75},
     {4, 40, 77},			/* 18 */
     {4, 45, 80},
     {4, 50, 83},			/* 20 */
     {5, 55, 85},
     {5, 60, 87},
     {5, 65, 90},
     {5, 70, 95},
     {6, 75, 100}			/* 25 */
};

const struct dex_app_type dex_app[26] =
{
     {60, -100, 0},			/* 0 */
     {50, -95,  0},			/* 1 */
     {50, -85,  0},
     {40, -75,  0},
     {30, -70,  0},
     {20, -65,  0},			/* 5 */
     {10, -60,  0},
     {0,  -50,  0},
     {0,  -40,  0},
     {0,  -30,  0},
     {0,  -20,  0},			/* 10 */
     {0,  -10,  0},
     {0,    0,  0},
     {0,    5,  5},
     {0,    10, 5},
     {-10,  15, 5},			/* 15 */
     {-15,  20, 5},
     {-20,  23, 5},
     {-30,  25, 10},
     {-40,  30, 10},
     {-50,  35, 15},			/* 20 */
     {-60,  35, 20},
     {-75,  40, 25},
     {-90,  50, 30},
     {-105, 60, 40},
     {-120, 70, 50}			/* 25 */
};

const struct con_app_type con_app[26] =
{
     {-4, 20},			/*  0 */
     {-3, 25},			/*  1 */
     {-2, 30},
     {-2, 35},			/*  3 */
     {-1, 40},
     {-1, 45},			/*  5 */
     {-1, 50},
     {0, 55},
     {0, 60},
     {0, 65},
     {0, 70},			/* 10 */
     {0, 75},
     {0, 80},
     {0, 85},
     {1, 88},
     {1, 90},			/* 15 */
     {2, 95},
     {2, 97},
     {3, 99},			/* 18 */
     {3, 99},
     {4, 99},			/* 20 */
     {4, 99},
     {5, 99},
     {6, 99},
     {7, 99},
     {8, 99}			/* 25 */
};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const struct liq_type liq_table[LIQ_MAX] =
{
     {"water", 			"clear", 	{0, 1, 10}},	/*  0 */
     {"beer", 			"amber", 	{3, 2, 5}},
     {"wine", 			"rose", 	{5, 2, 5}},
     {"ale", 			"brown", 	{2, 2, 5}},
     {"dark ale", 		"dark", 	{1, 2, 5}},
     {"whisky",			"golden", 	{6, 1, 4}},	/*  5 */
     {"lemonade", 		"pink", 	{0, 1, 8}},
     {"firebreather", 		"boiling", 	{10, 0, 0}},
     {"local specialty", 	"everclear", 	{3, 3, 3}},
     {"slime mold juice", 	"green", 	{0, 4, -8}},
     {"milk", 			"white", 	{0, 3, 6}},	/* 10 */
     {"tea", 			"tan", 		{0, 1, 6}},
     {"coffee", 		"black", 	{0, 1, 6}},
     {"blood", 			"red", 		{0, 2, -1}},
     {"salt water", 		"clear", 	{0, 1, -2}},
     {"cola", 			"cherry", 	{0, 1, 5}}	/* 15 */
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 * Class Rating is now unused, as is level requirements.
 * Level req's are in savefiles, rating is totally unused.
 * This calls, someday, for a major rewrite.
 */

#define SLOT(n)	n

struct skill_type   skill_table[MAX_SKILL] =
{

/*
 * Magic spells.
 */

     { "reserved", {99, 99, 99, 99, 99, 99},  	0, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 0 ), 0, 0, "", "", "" },
     { "armor", {93, 1, 93, 93, 93, 93},  	spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 1 ), 5, 12, "", "You feel less protected.", "" },
     { "teleport", {93, 10, 93, 93, 93, 93}, 	spell_teleport, TAR_CHAR_SELF, POS_FIGHTING,
               NULL, SLOT ( 2 ), 35, 12, "", "!Teleport!", "" },
     { "bless", {93, 5, 93, 93, 93, 93},        spell_bless, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 3 ), 5, 12,     "", "You feel less righteous.", "" },
     { "blindness", {93, 8, 93, 93, 93, 93}, spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               &gsn_blindness, SLOT ( 4 ), 5, 12,     "", "You can see again.", ""  },
     { "burning hands", {5, 93, 93, 93, 93, 93},
          spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 5 ), 15, 12,
               "burning hands", "!Burning Hands!", ""
     },

     {
          "call lightning", {93, 18, 93, 93, 93, 93},
          spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 6 ), 15, 12,
               "lightning bolt", "!Call Lightning!", ""
     },

     {
          "charm person", {15, 93, 93, 93, 93, 93},
          spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING,
               &gsn_charm_person, SLOT ( 7 ), 5, 12,
               "", "You feel more self-confident.", ""
     },

     {
          "chill touch", {93, 93, 93, 93, 93, 93}, 
          spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 8 ), 15, 12,
               "chilling touch", "You feel less cold.", ""
     },

     {
          "colour spray", {10, 93, 93, 93, 93, 93},
          spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 10 ), 15, 12,
               "colour spray", "!Colour Spray!", ""
     },

     {
          "control weather", {93, 19, 93, 93, 93, 93},
          spell_control_weather, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 11 ), 25, 12,
               "", "!Control Weather!", ""
     },

     {
          "create food", {6, 4, 93, 93, 93, 93}, 
          spell_create_food, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 12 ), 5, 12,
               "", "!Create Food!", ""
     },

     {
          "create water", {93, 3, 93, 93, 93, 93},
          spell_create_water, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 13 ), 5, 12,
               "", "!Create Water!", ""
     },

     {
          "cure blindness", {93, 5, 93, 93, 93, 93},
          spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 14 ), 5, 12,
               "", "!Cure Blindness!", ""
     },

     {
          "cure critical", {93, 13, 93, 93, 93, 93}, 
          spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 15 ), 20, 12,
               "", "!Cure Critical!", ""
     },

     {
          "cure light", {93, 1, 93, 93, 93, 93}, 
          spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 16 ), 10, 12,
               "", "!Cure Light!", ""
     },

     {
          "curse", {93, 16, 93, 93, 93, 93}, 
          spell_curse, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               &gsn_curse, SLOT ( 17 ), 20, 12,
               "curse", "The curse wears off.", ""
     },

     {
          "detect evil", {93, 4, 93, 93, 93, 93},
          spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 18 ), 5, 12,
               "", "The red in your vision disappears.", ""
     },

     {
          "detect invis", {5, 93, 93, 93, 93, 93},
          spell_detect_invis, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 19 ), 5, 12,
               "", "You no longer see invisible objects.", ""
     },

     {
          "detect magic", {4, 12, 93, 93, 93, 93},
          spell_detect_magic, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 20 ), 5, 12,
               "", "The detect magic wears off.", ""
     },

     {
          "detect poison", {93, 10, 93, 93, 93, 93},
          spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 21 ), 5, 12,
               "", "!Detect Poison!", ""
     },

     {
          "dispel evil", {93, 15, 93, 93, 93, 93},
          spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 22 ), 15, 12,
               "dispel evil", "!Dispel Evil!", ""
     },

     {
          "earthquake", {93, 11, 93, 93, 93, 93}, 
          spell_earthquake, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 23 ), 15, 12,
               "earthquake", "!Earthquake!", ""
     },

     {
          "enchant weapon", {16, 93, 93, 93, 93, 93},
          spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 24 ), 100, 24,
               "", "!Enchant Weapon!", ""
     },

     {
          "energy drain", {93, 93, 93, 93, 93, 93},
          spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 25 ), 35, 12,
               "energy drain", "!Energy Drain!", ""
     },

     {
          "fireball", {19, 93, 93, 93, 93, 93},
          spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 26 ), 15, 12,
               "fireball", "!Fireball!", ""
     },

     {
          "harm", {93, 25, 93, 93, 93, 93}, 
          spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 27 ), 35, 12,
               "harm spell", "!Harm!", ""
     },

     {
          "heal", {93, 39, 93, 93, 93, 93}, 
          spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 28 ), 50, 12,
               "", "!Heal!", ""
     },

     {
          "invis", {7, 93, 93, 93, 93, 93}, 
          spell_invis, TAR_CHAR_DEFENSIVE, POS_STANDING,
               &gsn_invis, SLOT ( 29 ), 5, 12,
               "", "You are no longer invisible.", ""
     },

     {
          "lightning bolt", {10, 93, 93, 93, 93, 93},
          spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 30 ), 15, 12,
               "lightning bolt", "!Lightning Bolt!", ""
     },

     {
          "locate object", {11, 33, 93, 93, 93, 93}, 
          spell_locate_object, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 31 ), 20, 4,
               "", "!Locate Object!", ""
     },

     {
          "magic missile", {1, 93, 93, 93, 93, 93}, 
          spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 32 ), 15, 12,
               "magic missile", "!Magic Missile!", ""
     },

     {
          "poison", {14, 93, 93, 93, 93, 93}, 
          spell_poison, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               &gsn_poison, SLOT ( 33 ), 10, 12,
               "poison", "You feel less sick.", ""
     },

     {
          "protection evil", {93, 9, 93, 93, 93, 93},
          spell_protection_evil, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 34 ), 5, 12,
               "", "You feel less protected from evil.", ""
     },

     {
          "remove curse", {93, 18, 93, 93, 93, 93}, 
          spell_remove_curse, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 35 ), 5, 12,
               "", "!Remove Curse!", ""
     },

     {
          "sanctuary", {93, 30, 93, 93, 93, 93}, 
          spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 36 ), 75, 12,
               "", "The white aura around your body fades.", ""
     },

     {
          "ghost form", {93, 93, 93, 93, 93, 93},
          spell_ghostform, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 9 ), 75, 12,
               "", "The demons surrounding you fly away.", ""
     },

     {
          "sleep", {11, 93, 93, 93, 93, 93}, 
          spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
               &gsn_sleep, SLOT ( 38 ), 15, 12,
               "", "You feel less tired.", ""
     },

     {
          "giant strength", {9, 93, 93, 93, 93, 93},
          spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 39 ), 20, 12,
               "", "You feel weaker.", ""
     },

     {
          "summon", {21, 93, 93, 93, 93, 93},
          spell_summon, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 40 ), 50, 12,
               "", "!Summon!", ""
     },

     {
          "ventriloquate", {2, 93, 93, 93, 93, 93},
          spell_ventriloquate, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 41 ), 5, 12,
               "", "!Ventriloquate!", ""
     },

     {
          "word of recall", {93, 12, 93, 93, 93, 93},
          spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
               NULL, SLOT ( 42 ), 5, 12,
               "", "!Word of Recall!", ""
     },

     {
          "cure poison", {93, 14, 93, 93, 93, 93}, 
          spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 43 ), 5, 12,
               "", "!Cure Poison!", ""
     },

     {
          "detect hidden", {9, 93, 93, 93, 93, 93},
          spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 44 ), 5, 12,
               "", "You feel less aware of your suroundings.", ""
     },

     {
          "identify", {12, 93, 93, 93, 93, 93}, 
          spell_identify, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 52 ), 12, 24,
               "", "!Identify!", ""
     },

     {
          "shocking grasp", {93, 93, 93, 93, 93, 93},
          spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 53 ), 15, 12,
               "shocking grasp", "!Shocking Grasp!", ""
     },

     {
          "fly", {8, 93, 93, 93, 93, 93}, 
          spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 56 ), 10, 18,
               "", "You slowly float to the ground.", ""
     },

     {
          "continual light", {5, 93, 93, 93, 93, 93},
          spell_continual_light, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 57 ), 7, 12,
               "", "!Continual Light!", ""
     },

     {
          "know alignment", {93, 18, 93, 93, 93, 93},
          spell_know_alignment, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 58 ), 9, 12,
               "", "!Know Alignment!", ""
     },

     {
          "dispel magic", {12, 20, 93, 93, 93, 93},
          spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 59 ), 15, 12,
               "", "!Dispel Magic!", ""
     },

     {
          "cure serious", {93, 9, 93, 93, 93, 93}, 
          spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 61 ), 15, 12,
               "", "!Cure Serious!", ""
     },

     {
          "cause light", {93, 3, 93, 93, 93, 93}, 
          spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 62 ), 15, 12,
               "spell", "!Cause Light!", ""
     },

     {
          "cause critical", {93, 14, 93, 93, 93, 93},
          spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 63 ), 20, 12,
               "spell", "!Cause Critical!", ""
     },

     {
          "cause serious", {93, 7, 93, 93, 93, 93}, 
          spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 64 ), 17, 12,
               "spell", "!Cause Serious!", ""
     },

     {
          "flamestrike", {93, 16, 93, 93, 93, 93}, 
          spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 65 ), 20, 12,
               "flamestrike", "!Flamestrike!", ""
     },

     {
          "stone skin", {23, 93, 93, 93, 93, 93}, 
          spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 66 ), 12, 18,
               "", "Your skin feels soft again.", ""
     },

     {
          "shield", {17, 93, 93, 93, 93, 93}, 
          spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 67 ), 12, 18,
               "", "Your force shield shimmers then fades away.", ""
     },

     {
          "weaken", {93, 93, 93, 93, 93, 93}, 
          spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 68 ), 20, 12,
               "spell", "You feel stronger.", ""
     },

     {
          "mass invis", {38, 93, 93, 93, 93, 93},
          spell_mass_invis, TAR_IGNORE, POS_STANDING,
               &gsn_mass_invis, SLOT ( 69 ), 20, 24,
               "", "!Mass Invis!", ""
     },

     {
          "acid blast", {25, 93, 93, 93, 93, 93},
          spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 70 ), 20, 12,
               "acid blast", "!Acid Blast!", ""
     },

     {
          "faerie fire", {4, 93, 93, 93, 93, 93},
          spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 72 ), 5, 12,
               "faerie fire", "The pink aura around you fades away.", ""
     },

     {
          "faerie fog", {9, 93, 93, 93, 93, 93}, 
          spell_faerie_fog, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 73 ), 12, 12,
               "faerie fog", "!Faerie Fog!", ""
     },

     {
          "pass door", {23, 93, 93, 93, 93, 93}, 
          spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 74 ), 20, 12,
               "", "You feel solid again.", ""
     },

     {
          "infravision", {93, 93, 93, 93, 93, 93},
          spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 77 ), 5, 18,
               "", "You no longer see in the dark.", ""
     },

     {
          "create spring", {93, 17, 93, 93, 93, 93},
          spell_create_spring, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 80 ), 20, 12,
               "", "!Create Spring!", ""
     },

     {
          "refresh", {93, 6, 93, 93, 93, 93}, 
          spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 81 ), 12, 18,
               "refresh", "!Refresh!", ""
     },

     {
          "change sex", {93, 93, 93, 93, 93, 93},
          spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 82 ), 15, 12,
               "", "Your body feels familiar again.", ""
     },

     {
          "gate", {23, 33, 93, 93, 93, 93}, 
          spell_gate, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 105 ), 50, 12,
               "", "!Gate!", "obsidian"
     },

     {
          "gate without error", {23, 33, 93, 93, 93, 93},
          spell_gate_without_error, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 83 ), 80, 12,
               "", "!Gate Without Error!", "obsidian"
     },

     /*
      * Dragon breath
      */
     {
          "acid breath", {44, 93, 93, 93, 93, 93}, 
          spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 200 ), 45, 4,
               "blast of acid", "!Acid Breath!", ""
     },

     {
          "fire breath", {41, 93, 93, 93, 93, 93}, 
          spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 201 ), 45, 4,
               "blast of flame", "!Fire Breath!", ""
     },

     {
          "frost breath", {46, 93, 93, 93, 93, 93},
          spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 202 ), 45, 4,
               "blast of frost", "!Frost Breath!", ""
     },

     {
          "gas breath", {45, 93, 93, 93, 93, 93}, 
          spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 203 ), 45, 4,
               "blast of gas", "!Gas Breath!", ""
     },

     {
          "lightning breath", {47, 93, 93, 93, 93, 93},
          spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 204 ), 45, 4,
               "blast of lightning", "!Lightning Breath!", ""
     },

     {
          "chaos", {93, 93, 93, 93, 93, 93, 93}, 
          spell_chaos, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 205 ), 30, 16,
               "chaos", "!Chaos!", ""
     },

     {
          "minor chaos", {103, 103, 103, 103, 103, 103, 103}, 
          spell_minor_chaos, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 45 ), 10, 12,
               "minor chaos", "!Minor Chaos!", ""
     },

     {
          "create buffet", {93, 13, 93, 93, 93, 93}, 
          spell_create_buffet, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 230 ), 50, 12,
               "", "!Create Buffet!", ""
     },

     {
          "general purpose", {93, 93, 93, 93, 93, 93},
          spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 401 ), 10, 12,
               "general purpose ammo", "!General Purpose Ammo!", ""
     },

     {
          "high explosive", {93, 93, 93, 93, 93, 93},
          spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 402 ), 25, 12,
               "high explosive ammo", "!High Explosive Ammo!", ""
     },

     {
          "mind meld", {93, 93, 93, 93, 93, 93},
          spell_mind_meld, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 403 ), 35, 24,
               "mental blast", "Your feel your head clearing.", ""
     },

     {
          "confusion", {93, 93, 93, 93, 93, 93},
          spell_confusion, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 37 ), 20, 16,
               "", "Now it all makes perfect sense.", ""
     },

     {
          "summon familiar", {27, 93, 93, 93, 93, 93},
          spell_summon_familiar, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 100 ), 50, 15,
               "", "!Summon Familiar!", ""
     },

     {
          "animate dead", {27, 93, 93, 93, 93, 93}, 
          spell_animate, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 404 ), 100, 15,
               "", "!Animate Dead!", ""
     },

     {
          "soul blade", {93, 93, 93, 93, 93, 93}, 
          spell_soul_blade, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 405 ), 35, 15,
               "", "!Soul Blade!", ""
     },

     {
          "minor creation", {6, 93, 93, 93, 93, 93},
          spell_minor_creation, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 406 ), 15, 20,
               "", "!Minor Creation!", ""
     },

     {
          "psi twister", {93, 93, 93, 93, 93, 93}, 
          spell_psi_twister, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 407 ), 25, 20,
               "psi twister", "!Psi Twister!", ""
     },

     {
          "meteor", {93, 93, 93, 93, 93, 93}, 
          spell_meteor, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 101 ), 25, 20,
               "meteor", "!Meteor!", ""
     },

     {
          "boil", {93, 93, 93, 93, 93, 93}, 
          spell_boil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 102 ), 25, 20,
               "boiling magic", "!Boil!", ""
     },

     {
          "dehydrate", {93, 93, 93, 93, 93, 93},
          spell_dehydrate, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 103 ), 25, 20,
               "dehydration", "!Dehydrate!", ""
     },

     {
          "meteor shower", {93, 93, 93, 93, 93, 93},
          spell_meteor_shower, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 104 ), 25, 20,
               "shower of meteors", "!Meteor Shower!", ""
     },

     {
          "exorcism", {93, 26, 93, 93, 93, 93}, 
          spell_exorcism, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 408 ), 20, 12,
               "angels", "!Exorcism!", ""
     },

     {
          "youth", {53, 49, 53, 53, 53, 53}, 
          spell_youth, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 409 ), 200, 24,
               "", "!YOUTH!", ""
     },

     {
          "age", {53, 49, 53, 53, 53, 53}, 
          spell_age, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 410 ), 200, 12,
               "", "!AGE!", ""
     },

     {
          "chain lightning", {29, 93, 93, 93, 93, 93},
          spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 500 ), 25, 12,
               "lightning", "!Chain Lightning!", ""
     },

     {
          "cure disease", {93, 11, 93, 93, 93, 93}, 
          spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 501 ), 20, 12,
               "", "!Cure Disease!", ""
     },

     {
          "haste", {18, 93, 93, 93, 93, 93}, 
          spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 502 ), 30, 12,
               "", "You feel yourself moving less quickly.", ""
     },

     {
          "slow", {18, 93, 93, 93, 93, 93}, 
          spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 527 ), 30, 12,
               "", "You feel yourself moving less slowly.", ""
     },

     {
          "plague", {93, 25, 93, 93, 93, 93},
          spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               &gsn_plague, SLOT ( 503 ), 20, 12,
               "sickness", "Your sores vanish.", ""
     },

     {
          "frenzy", {34, 29, 93, 93, 93, 93},
          spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 504 ), 30, 24,
               "", "Your rage ebbs.", ""
     },

     {
          "demonfire", {93, 26, 93, 93, 93, 93},
          spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 505 ), 20, 12,
               "torments", "!Demonfire!", ""
     },

     {
          "holy word", {93, 33, 93, 93, 93, 93},
          spell_holy_word, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 506 ), 200, 24,
               "divine wrath", "!Holy Word!", ""
     },

     {
          "cancellation", {13, 15, 93, 93, 93, 93},
          spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 507 ), 20, 12,
               "", "!cancellation!", ""
     },

     {
          "mass healing", {93, 45, 93, 93, 93, 93},
          spell_mass_healing, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 508 ), 100, 36,
               "", "!Mass Healing!", ""
     },

     {
          "calm", {93, 16, 93, 93, 93, 93}, 
          spell_calm, TAR_IGNORE, POS_FIGHTING,
               NULL, SLOT ( 509 ), 30, 12,
               "", "You have lost your peace of mind.", ""
     },

     {
          "enchant armor", {15, 27, 93, 93, 93, 93}, 
          spell_enchant_armor, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 510 ), 100, 24,
               "", "!Enchant Armor!", ""
     },

     {
          "brand", {38, 93, 93, 93, 93, 93},
          spell_brand, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 511 ), 100, 8,
               "", "!brand!", ""
     },

     {
          "negate alignment", {93, 30, 93, 93, 93, 93}, 
          spell_negate_alignment, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 512 ), 50, 12,
               "", "!negate alignment", ""
     },

     {
          "mask self", {16, 53, 53, 53, 53, 53}, 
          spell_mask_self, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 513 ), 100, 8,
               "", "You return to normal form.", ""
     },

     {
          "absorb magic", {24, 53, 53, 53, 53, 53},
          spell_absorb_magic, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 514 ), 50, 4,
               "", "You can no longer absorb magic.", ""
     },

     {
          "psychic anchor", {53, 19, 53, 53, 53, 53},
          spell_psychic_anchor, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 515 ), 30, 4,
               "", "!psychic anchor!", ""
     },

     {
          "fear", {14, 53, 53, 53, 53, 53}, 
          spell_fear, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 516 ), 20, 8,
               "", "You no longer feel so scared.", ""
     },

     {
          "protection good", {93, 9, 93, 93, 93, 93}, 
          spell_protection_good, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 517 ), 5, 12,
               "", "You feel less protected from good.", ""
     },

     {
          "detect good", {93, 4, 93, 93, 93, 93}, 
          spell_detect_good, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 518 ), 5, 12,
               "", "The green in your vision disappears.", ""
     },

     {
          "dispel good", {93, 16, 93, 93, 93, 93},
          spell_dispel_good, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 519 ), 15, 12,
               "dispel good", "!Dispel Good!", ""
     },

     {
          "regeneration", {93, 14, 93, 93, 93, 93},
          spell_regeneration, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 520 ), 30, 8,
               "", "You are no longer regenerating.", ""
     },

     {
          "fire shield", {93, 93, 93, 93, 93, 93}, 
          spell_fire_shield, TAR_CHAR_SELF, POS_STANDING,
               NULL, SLOT ( 521 ), 10, 8,
               "fire shield", "Your fire shield fades away.", ""
     },

     {
          "portal", {45, 93, 93, 93, 93, 93}, 
          spell_portal, TAR_IGNORE, POS_STANDING,
               NULL, SLOT ( 522 ), 100, 16,
               "portal", "!portal!", ""
     },

     {
          "remove invis", {28, 93, 93, 93, 93, 93},
          spell_remove_invis, TAR_OBJ_INV, POS_STANDING,
               NULL, SLOT ( 523 ), 40, 12,
               "", "!Remove Invis!", ""
     },

     {
          "vocalize", {21, 53, 53, 53, 53, 53}, 
          spell_vocalize, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 524 ), 50, 12,
               "", "You must speak to cast spells again.", ""
     },

     {
          "entropy", {93, 93, 93, 93, 93, 93},
          spell_entropy, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 525 ), 30, 18,
               "entropy", "!Entropy!", ""
     },

     {
          "remove fear", {53, 6, 93, 93, 93, 93},
          spell_remove_fear, TAR_CHAR_DEFENSIVE, POS_STANDING,
               NULL, SLOT ( 526 ), 5, 12,
               "", "!Remove Fear!", ""
     },

     {
          "mute", {23, 53, 53, 53, 53, 53}, 
          spell_mute, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               NULL, SLOT ( 527 ), 50, 12,
               "", "You can speak again.", ""
     },

/* classes are: mage Avenger thief warrior monk Defiler chaosmage */

/* combat and weapons skills */

     {
          "axe", {93, 93, 93, 3, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_axe, SLOT ( 0 ), 0, 0,
               "", "!Axe!", ""
     },

     {
          "dagger", {1, 93, 1, 2, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_dagger, SLOT ( 0 ), 0, 0,
               "", "!Dagger!", ""
     },

     {
          "flail", {93, 1, 93, 6, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_flail, SLOT ( 0 ), 0, 0,
               "", "!Flail!", ""
     },

     {
          "mace", {93, 1, 93, 6, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_mace, SLOT ( 0 ), 0, 0,
               "", "!Mace!", ""
     },

     {
          "polearm", {93, 93, 93, 8, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_polearm, SLOT ( 0 ), 0, 0,
               "", "!Polearm!", ""
     },

     {
          "shield block", {93, 22, 11, 5, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_shield_block, SLOT ( 0 ), 0, 0,
               "", "!Shield!", ""
     },

     {
          "spear", {93, 93, 93, 4, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_spear, SLOT ( 0 ), 0, 0,
               "", "!Spear!", ""
     },

     {
          "sword", {93, 93, 93, 1, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_sword, SLOT ( 0 ), 0, 0,
               "", "!sword!", ""
     },

     {
          "whip", {93, 93, 3, 7, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_whip, SLOT ( 0 ), 0, 0,
               "", "!Whip!", ""
     },

     {
          "backstab", {93, 93, 10, 93, 93, 93},
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_backstab, SLOT ( 0 ), 0, 24,
               "backstab", "!Backstab!", ""
     },

     {
          "circle", {93, 93, 25, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_circle, SLOT ( 0 ), 0, 24,
               "circle", "!Circle!", ""
     },

     {
          "bash", {93, 93, 93, 13, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_bash, SLOT ( 0 ), 0, 24,
               "bash", "!Bash!", ""
     },

     {
          "berserk", {93, 93, 93, 15, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_berserk, SLOT ( 0 ), 0, 24,
               "", "You feel your pulse slow down.", ""
     },

     {
          "rally", {102, 102, 102, 102, 102, 102, 102},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_rally, SLOT ( 0 ), 0, 24,
               "", "You look around for guidance.", ""
     },

     {
          "dual", {93, 93, 93, 21, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_dual, SLOT ( 0 ), 0, 4,
               "", "!dual!", ""
     },

     {
          "dirt kicking", {35, 93, 7, 9, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_dirt, SLOT ( 0 ), 0, 24,
               "kicked dirt", "You rub the dirt out of your eyes.", ""
     },

     {
          "disarm", {93, 93, 16, 16, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_disarm, SLOT ( 0 ), 0, 24,
               "", "!Disarm!", ""
     },

     {
          "dodge", {20, 18, 10, 15, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_dodge, SLOT ( 0 ), 0, 0,
               "", "!Dodge!", ""
     },

     {
          "enhanced damage", {93, 93, 23, 9, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_enhanced_damage, SLOT ( 0 ), 0, 0,
               "", "!Enhanced Damage!", ""
     },

     {
          "ultra damage", {93, 93, 93, 35, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_ultra_damage, SLOT ( 0 ), 0, 0,
               "", "!Ultra Damage!", ""
     },

     {
          "envenom", {93, 93, 18, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_RESTING,
               &gsn_envenom, SLOT ( 0 ), 0, 36,
               "", "!Envenom!", ""
     },

     {
          "hand to hand", {93, 93, 17, 8, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_hand_to_hand, SLOT ( 0 ), 0, 0,
               "", "!Hand to Hand!", ""
     },

     {
          "kick", {93, 93, 15, 3, 93, 93}, 
          spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
               &gsn_kick, SLOT ( 0 ), 0, 12,
               "kick", "!Kick!", ""
     },

     {
          "parry", {93, 93, 32, 8, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_parry, SLOT ( 0 ), 0, 0,
               "", "!Parry!", ""
     },

     {
          "rotate", {93, 93, 93, 18, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_rotate, SLOT ( 0 ), 0, 8,
               "", "!Rotate!", ""
     },

     {
          "rescue", {93, 28, 93, 6, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_rescue, SLOT ( 0 ), 0, 12,
               "", "!Rescue!", ""
     },

     {
          "trip", {93, 93, 5, 4, 93, 93}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_trip, SLOT ( 0 ), 0, 24,
               "trip", "!Trip!", ""
     },

     {
          "second attack", {55, 23, 18, 11, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_second_attack, SLOT ( 0 ), 0, 0,
               "", "!Second Attack!", ""
     },

     {
          "sharpen", {93, 93, 93, 24, 93, 93}, 
          spell_null, TAR_OBJ_INV, POS_STANDING,
               &gsn_sharpen, SLOT ( 0 ), 0, 16,
               "", "!Sharpen!", ""
     },

     {
          "third attack", {93, 93, 93, 20, 93, 93},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_third_attack, SLOT ( 0 ), 0, 0,
               "", "!Third Attack!", ""
     },

     {
          "fourth attack", {103, 103, 103, 103, 103, 103},
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_fourth_attack, SLOT ( 0 ), 0, 0,
               "", "!Fourth Attack!", ""
     },

     {
          "fifth attack", {103, 103, 103, 103, 103, 103}, 
          spell_null, TAR_IGNORE, POS_FIGHTING,
               &gsn_fifth_attack, SLOT ( 0 ), 0, 0,
               "", "!Fifth Attack!", ""
     },

/* non-combat skills */

     {
          "fast healing", {93, 19, 14, 13, 93, 93},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_fast_healing, SLOT ( 0 ), 0, 0,
               "", "!Fast Healing!", ""
     },

     {
          "haggle", {21, 19, 18, 22, 93, 93}, 
          spell_null, TAR_IGNORE, POS_RESTING,
               &gsn_haggle, SLOT ( 0 ), 0, 0,
               "", "!Haggle!", ""
     },

     {
          "hide", {93, 93, 4, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_RESTING,
               &gsn_hide, SLOT ( 0 ), 0, 12,
               "", "!Hide!", ""
     },

     {
          "lore", {15, 17, 27, 93, 93, 93},
          spell_null, TAR_IGNORE, POS_RESTING,
               &gsn_lore, SLOT ( 0 ), 0, 18,
               "", "!Lore!", ""
     },

     {
          "meditation", {15, 9, 93, 93, 93, 93},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_meditation, SLOT ( 0 ), 0, 0,
               "", "Meditation", ""
     },

     {
          "peek", {93, 93, 19, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_peek, SLOT ( 0 ), 0, 0,
               "", "!Peek!", ""
     },

     {
          "pick lock", {93, 93, 12, 93, 93, 93},
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_pick_lock, SLOT ( 0 ), 0, 12,
               "", "!Pick!", ""
     },

     {
          "recruit", {93, 93, 93, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_recruit, SLOT ( 0 ), 0, 0,
               "", "!Recruit!", ""
     },

     {
          "sneak", {93, 93, 8, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_sneak, SLOT ( 0 ), 0, 12,
               "", "You no longer feel stealthy.", ""
     },

     {
          "steal", {93, 93, 2, 93, 93, 93}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_steal, SLOT ( 0 ), 0, 24,
               "", "!Steal!", ""
     },

     {
          "scrolls", {5, 5, 7, 10, 5, 5},
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_scrolls, SLOT ( 0 ), 0, 24,
               "", "!Scrolls!", ""
     },

     {
          "staves", {5, 5, 7, 10, 5, 5}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_staves, SLOT ( 0 ), 0, 12,
               "", "!Staves!", ""
     },

     {
          "wands", {5, 5, 7, 10, 5, 5}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_wands, SLOT ( 0 ), 0, 12,
               "", "!Wands!", ""
     },

     {
          "recall", {1, 1, 1, 1, 1, 1}, 
          spell_null, TAR_IGNORE, POS_STANDING,
               &gsn_recall, SLOT ( 0 ), 0, 12,
               "", "!Recall!", ""
     },

/* LANGUAGES */
     {
          "human tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_human, SLOT ( 0 ), 0, 12,
               "", "!human!", ""
     },

     {
          "dwarf tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_dwarf, SLOT ( 0 ), 0, 12,
               "", "!dwarf!", ""
     },
     {
          "elf tongue", {1, 1, 1, 1, 1, 1, 1}, 
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_elf, SLOT ( 0 ), 0, 12,
               "", "!elf!", ""
     },
     {
          "giant tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_giant, SLOT ( 0 ), 0, 12,
               "", "!giant!", ""
     },
     {
          "gargoyle tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_gargoyle, SLOT ( 0 ), 0, 12,
               "", "!gargoyle!", ""
     },
     {
          "kobold tongue", {1, 1, 1, 1, 1, 1, 1}, 
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_kobold, SLOT ( 0 ), 0, 12,
               "", "!kobold!", ""
     },
     {
          "centaur tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_centaur, SLOT ( 0 ), 0, 12,
               "", "!centaur!", ""
     },
     {
          "azer tongue", {1, 1, 1, 1, 1, 1, 1}, 
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_azer, SLOT ( 0 ), 0, 12,
               "", "!azer!", ""
     },
     {
          "kender tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_kender, SLOT ( 0 ), 0, 12,
               "", "!kender!", ""
     },
     {
          "common tongue", {1, 1, 1, 1, 1, 1, 1},
          spell_null, TAR_IGNORE, POS_SLEEPING,
               &gsn_lang_common, SLOT ( 0 ), 0, 12,
               "", "!common!", ""
     }

};
