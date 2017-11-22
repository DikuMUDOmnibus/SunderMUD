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


#ifndef _GLOBALMERC_H
# define _GLOBALMERC_H   1

/*
 * This file contains all the structs and globals.
 */

/*
 * TYPEDEF STRUCTS
 */
typedef struct  i3_chardata             I3_CHARDATA;
typedef struct  affect_data             AFFECT_DATA;
typedef struct  area_data               AREA_DATA;
typedef struct  ban_data                BAN_DATA;
typedef struct  char_data               CHAR_DATA;
typedef struct  descriptor_data         DESCRIPTOR_DATA;
typedef struct  exit_data               EXIT_DATA;
typedef struct  extra_descr_data        EXTRA_DESCR_DATA;
typedef struct  help_data               HELP_DATA;
typedef struct  kill_data               KILL_DATA;
typedef struct  mob_index_data          MOB_INDEX_DATA;
typedef struct  obj_data                OBJ_DATA;
typedef struct  obj_index_data          OBJ_INDEX_DATA;
typedef struct  pc_data                 PC_DATA;
typedef struct  reset_data              RESET_DATA;
typedef struct  room_index_data         ROOM_INDEX_DATA;
typedef struct  shop_data               SHOP_DATA;
typedef struct  time_info_data          TIME_INFO_DATA;
typedef struct  mud_data                MUD_DATA;
typedef struct  weather_data            WEATHER_DATA;
typedef struct  prog_list               PROG_LIST;
typedef struct  prog_code               PROG_CODE;
typedef struct  lease_data      	    LEASE;
typedef struct 	note_data  		        NOTE_DATA;
typedef struct 	board_data 		        BOARD_DATA;


/* 
 * FUNCTION TYPES
 */

/*
 * Function types.
 */
typedef void DO_FUN     args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN   args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN  args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );
typedef void OBJ_FUN    args( ( OBJ_DATA *obj, char *argument ) );
typedef void ROOM_FUN   args( ( ROOM_INDEX_DATA *room, char *argument ) );

/*
 * EXTERN STRUCTS & GLOBAL VARIABLES
 */

extern  struct 	disable_cmd_type   	*disable_cmd_list;
extern  struct 	crier_type         	*crier_list;
extern  struct 	account_type       	*account_list;
extern  struct 	lease_data     		*lease_list;
extern  struct 	race_type       	race_table      [];
extern  struct  skill_type      	skill_table     [MAX_SKILL];
extern  struct  social_type     	social_table    [MAX_SOCIALS];
extern  const   struct  str_app_type    str_app         [26];
extern  const   struct  int_app_type    int_app         [26];
extern  const   struct  wis_app_type    wis_app         [26];
extern  const   struct  dex_app_type    dex_app         [26];
extern  const   struct  con_app_type    con_app         [26];
extern  const   struct  class_type      class_table     [MAX_CLASS];
extern  const   struct  attack_type     attack_table    [];
extern  const   struct  pc_race_type    pc_race_table   [];
extern  const   struct  liq_type        liq_table       [LIQ_MAX];
extern  const   struct  material_data   material_table [];
extern  const   struct  wear_data       wear_info[];
extern  const   struct  spec_type       spec_table[];
extern  const   struct  cmd_type        cmd_table [];
//extern struct work_type       *work_orders; /* Was for threading */
extern  const   char   			*size_table [];
extern  const   char  			*day_name [];
extern  const   char   			*month_name [];
extern  char *  const   		dir_name[];
extern  const   sh_int  		rev_dir[];
extern          HELP_DATA         *     help_first;
extern          SHOP_DATA         *     shop_first;
extern          BAN_DATA          *     ban_list;
extern          CHAR_DATA         *     char_list;
extern          DESCRIPTOR_DATA   *     descriptor_list;
extern          NOTE_DATA         *     note_list;
extern          OBJ_DATA          *     object_list;
extern          PROG_CODE         *     mprog_list;
extern          PROG_CODE         *     rprog_list;
extern          PROG_CODE         *     oprog_list;
extern          AFFECT_DATA       *     affect_free;
extern          BAN_DATA          *     ban_free;
extern          CHAR_DATA         *     char_free;
extern          DESCRIPTOR_DATA   *     descriptor_free;
extern          EXTRA_DESCR_DATA  *     extra_descr_free;
extern          NOTE_DATA         *     note_free;
extern          OBJ_DATA          *     obj_free;
extern          PC_DATA           *     pcdata_free;
extern          struct char_group *     group_free;
extern          time_t                  current_time;
extern          bool                    fLogAll;
extern          FILE *                  fpReserve;
extern          KILL_DATA               kill_table      [];
extern          TIME_INFO_DATA          time_info;
extern          WEATHER_DATA            weather_info;
extern          MUD_DATA                mud;
extern          bool                    MOBtrigger;
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern          SHOP_DATA *             shop_last;
extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;
extern          char                    str_empty       [1];
extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];
extern 	BOARD_DATA 		boards[MAX_BOARD];




/* Structures */

/*------------------------
 * System-Level Structures
 *-----------------------*/

/* Global Structure for globally accessed "Generic Data".
 * I know some people don't like the idea of a "Global Structure, however
 * there are way too many globals floating around in different source files,
 * and we need to get handle on them for organization.
 * All of these are accessed with the mud.xxxx pointer style.
 * Expect more stuff to be moved in. -- Lotherius
 */

struct mud_data
{
     /* These things are loaded from the RC file */
     bool            verify;                     /* Require verification of Emails? */
     sh_int          clogin;                     /* Color Login? */
     sh_int          death;                      /* Level of death "reality" */
     int             port;                       /* Port Number to operate on */
     int             fordemi;                    /* How many mortal heroes needed for a demigod */
     sh_int          mudxp;                      /* How hard is it to level */
     int             makelevel;                  /* Minlevel to make a clan */
     int             makecost;                   /* Min gold cost to make a clan */
     int             makeqp;                     /* Questpoints to make a clan */
     /* These are not. */
     bool            nonotify;                   /* Used when something spammy is about to happen */
     int             renumber;                   /* If an area needs renumbered, the modifier is stored here. */
     /* Memlog Stuff. */
     char           *ml_ident     [MAX_MEMLOG];  /* Use a txt identifier for readability */
     int             ml_perm_alloc[MAX_MEMLOG];  /* Allocated Perms. Not freed. */
     int             ml_mem_alloc [MAX_MEMLOG];  /* Allocated memory that can be freed */
     int             ml_mem_dalloc[MAX_MEMLOG];  /* De-allocated memory (with free_mem */
};

struct  time_info_data
{
     int                hour;
     int                day;
     int                month;
     int                year;
};

struct  weather_data
{
     int                mmhg;
     int                change;
     int                sky;
     int                sunlight;
};

/* Site Ban Structure */
struct  ban_data
{
          BAN_DATA   *next;
          char       *name;
};

/* Disabled Commands */
struct  disable_cmd_type
{
          char                     *name;
          int                       level;
          struct disable_cmd_type  *next;
          DO_FUN                   *disable_fcn;
};

/* Command Table Definition */
struct  cmd_type
{
     char *const     name;
     DO_FUN         *do_fun;
     sh_int          position;
     sh_int          level;
     sh_int          log;
     bool            show;
     char *const     helpmsg;           /* Text to show on enhanced "commands" listing. */
     long            category;          // Possible Categories for commands
     bool            disabled;
     int             disabled_level;
}; 

/* Help table */
struct  help_data
{
     HELP_DATA   *next;
     sh_int       level;
     char        *keyword;
     char        *text;
};


/* Town Crier Messages */
struct crier_type
{
     struct crier_type      *next;
     char                   *text;   /* Message the town crier will shout */
};

/* Socials */
struct  social_type
{
     char      name[20];
     char *    char_no_arg;
     char *    others_no_arg;
     char *    char_found;
     char *    others_found;
     char *    vict_found;
     char *    char_not_found;
     char *    char_auto;
     char *    others_auto;
};

/* Attack types */
struct attack_type
{
     char  *name;                       /* name and message */
     int    damage;                     /* damage class     */
};

/*-------------------------
 * Descriptors and Accounts
 *------------------------*/

/*
 *  * Descriptor (channel) structure.
 *  */
struct  descriptor_data
{
     DESCRIPTOR_DATA    *next;
     DESCRIPTOR_DATA    *snoop_by;
     CHAR_DATA          *character;
     CHAR_DATA          *original;
     char               *host;
     sh_int              descriptor;
     sh_int              connected;
     bool                fcommand;
     char                inbuf[4 * MAX_INPUT_LENGTH];
     char                incomm[MAX_INPUT_LENGTH];
     char                inlast[MAX_INPUT_LENGTH];
     int                 repeat;
     int                 outsize;
     int                 outtop;
     char               *outbuf;
     char               *afk_outbuf;
     bool                multi_comm;
     char               *showstr_head;
     char               *showstr_point;
     void               *pEdit;
     char              **pString;
     int                 editor;
     bool                ansi;                /* Ansi/VT colour, so can ask at startup */
     bool				 pueblo;			  /* Pueblo enabled? */
     bool                msp;
     bool                mxp;
     char               *client;              /* MXP Client Version... */
     char               *support;             /* Client support string. */
     time_t              save_time;           /* Last save time */
# if !defined (NOZLIB)
     z_stream           *out_compress;        /* MCCP Compression Stream            */
     unsigned char      *out_compress_buf;    /* MCCP Buffer                        */
# endif
};

/*
 * Data for player accounts, this is kept online so keep it small.
 */
struct account_type
{
     struct account_type       *next;
     char                      *acc_name;               /* Email Address of the Account   */
     char                      *password;               /* Password for Account           */
     short                      status;                 /* Account Status                 */
     int                        permadead;              /* Number of players Permadeathed */
     int                        heroes;                 /* Number of Mortal Heroes ever   */
     int                        demigods;               /* Number of Demigods ever        */
     char                      *char_name[MAX_CHARS];   /* Characters Belonging to this Account */
     int                        vcode;                  /* Verification Code - NOT SAVED */
};

/*------------------
 * Character Classes
 *-----------------*/

struct  class_type
{
          char      *name;               /* the full name of the class   */
          char       who_name[4];        /* Three-letter name for 'who'  */
          sh_int     attr_prime;         /* Prime attribute              */
          int        weapon;             /* First weapon                 */
          sh_int     guild[MAX_GUILD];   /* No Longer Used               */
          sh_int     skill_adept;        /* Maximum skill level          */
          sh_int     thac0_00;           /* Thac0 for level  0           */
          sh_int     thac0_32;           /* Thac0 for level 32           */
          sh_int     hp_min;             /* Min hp gained on leveling    */
          sh_int     hp_max;             /* Max hp gained on leveling    */
          bool       fMana;              /* Class gains mana on level    */
};

/*------------------
 * Skills and Spells
 *-----------------*/
struct  skill_type
{
     char *             name;                   /* Name of skill                */
     sh_int             skill_level[MAX_CLASS]; /* Level needed by class        */
     SPELL_FUN *        spell_fun;              /* Spell pointer                */
     sh_int             target;                 /* Legal targets                */
     sh_int             minimum_position;       /* Position for caster / user   */
     sh_int *           pgsn;                   /* Pointer to associated gsn    */
     sh_int             slot;                   /* Slot for #OBJECT loading     */
     sh_int             min_mana;               /* Minimum mana used            */
     sh_int             beats;                  /* Waiting time after use       */
     char *             noun_damage;            /* Damage message               */
     char *             msg_off;                /* Wear off message             */
     char *             component;              /* component                    */
     sh_int		bon_stat;		/* Stat that gives bonus/penalty to prac  */
     // Skill Tree Info
     bool		isgroup;		/* This is a group, not a skill           */
     sh_int		parent;			/* Parent group (slot) 		    	  */
     sh_int		sibling;		/* Previous skill in group required 	  */
     sh_int		kissing_cousin;		/* Related skill required	    	  */
     sh_int		distant_cousin;		/* Related skill that gives bonus   	  */
     sh_int		bastard_child;		/* If primary, bastard_child gets penalty */
     bool		matriarch;		/* This is allowed to be primary    	  */
};

/*----------------
 * Race Types
 *---------------*/

struct race_type
{
     char      *name;                   /* call name of the race */
     bool       pc_race;                /* can be chosen by pcs  */
     long       act;                    /* act bits for the race */
     long       aff;                    /* aff bits for the race */
     long       detect;                 /* Detect bits for the race */
     long       protect;                /* Protect bits for the race */
     long       off;                    /* off bits for the race */
     long       imm;                    /* imm bits for the race */
     long       res;                    /* res bits for the race */
     long       vuln;                   /* vuln bits for the race */
     long       form;                   /* default form flag for the race */
     long       parts;                  /* default parts for the race */
     sh_int     encumbrance;            /* emcumbrance value */
};

/* We're no longer going to depend on the records to line up between the two race tables. Yay. */

struct pc_race_type  /* additional data for pc races */
{
     char      *name;                   /* MUST be the same as in race_type */
     char       who_name[10];
     sh_int     points;                 /* cost in points of the race */
     sh_int     maxage;                 /* Maximum years old */
     sh_int     startage;               /* Starting Age */
     sh_int     class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */ // Not used
     char      *skills[5];              /* bonus skills for the race */
     int        recall;                 /* room # of this race's recall */
     int        healer;                 /* room # of this race's healer */
     sh_int     stats[MAX_STATS];       /* starting stats */
     sh_int     max_stats[MAX_STATS];   /* maximum stats */
     sh_int     size;                   /* aff bits for the race */ 
     /* size should be on race_type, not pc_race_type */
};

/*-------------------
 * Note Board Structs
 *------------------*/

/* Notes */
struct  note_data
{
     NOTE_DATA  *next;
     char       *sender;
     char       *date;
     char       *to_list;
     char       *subject;
     char       *text;
     time_t      date_stamp;
     time_t      expire;
};

/* Data about a board */
struct board_data
{
     char *short_name; /* Max 8 chars */
     char *long_name;  /* Explanatory text, should be no more than 40 ? chars */
     
     int read_level;   /* minimum level to see board */
     int write_level;  /* minimum level to post notes */
     
     char *names;       /* Default recipient */
     int force_type;    /* Default action (DEF_XXX) */
     
     int purge_days;    /* Default expiration */
     
     /* Non-constant data */
     
     NOTE_DATA *note_first; /* pointer to board's first note */
     bool changed;          /* currently unused */     
};


/*-----------------------------------------
 * Mobs | Chars | Players | Objects | Rooms
 *----------------------------------------*/

/* An affect */

struct  affect_data
{
     AFFECT_DATA       *next;
     sh_int             type;
     sh_int             level;
     sh_int             duration;
     sh_int             location;
     sh_int             modifier;
     sh_int             where;          /* Added for protect/detect fields */
     int                bitvector;
     char              *caster;
};

/* Improved materials code - Lotherius */
struct material_data
{
     char     *name;            /* What it is... */
     sh_int    type;            /* the identifier */
     long      vuln_flag;       /* set if applicable, otherwise 0 */
     int       durable;         /* The material's durability */
     int       difficult;       /* Material's repair difficulty */
     int       pierce;          /* Object's resistence to pierce */
     int       slash;           /* To bash */
     int       bash;            /* To slash */
     int       exotic;          /* To exotic */
     long      flags;           /* Material Flags */     
};

/* data for wear_info.. unsure where this fits in the organization :) */
struct wear_data
{
     char *name;        // Name of the body part or location
     bool ispart;       // Is this a body part?
     int  hitpct;       // Percent chance to hit (all must add up to 100%)
     long wear_flag;    // Which piece of armor relates to this body part.
     long item_flag;    // Flag on associated items.
     long part_req;     // Flag required on target to be protected on this part.     
     bool has_ac;       // Is this body part protectable by armor (Has its own AC)
     long supercede;    // Wear slot that supercedes this one (is outside of)          
     //                 // Determines what AC to use and and which eq to damage          
};
 
/*
 * Attribute bonus structures.
 * Many of these need Implemented, that will wait for Sunder > 2.0
 * Of those that are used, they may be underused.
 * CAVEAT: Not all of this has been done!
 */

struct  str_app_type
{
     sh_int       tohit;        /* Modifier to chance to hit beyond hitroll */ /* Unused */
     sh_int       todam;        /* Modifier to damage beyond damroll        */ /* Unused */
     sh_int       carry;        /* Amount of weight ch can carry            */
     sh_int       wield;        /* How large of a weapon ch can wield       */
};

struct  int_app_type
{
     sh_int       learn;        /* How well one learns INT based skills     */
     sh_int       mspell;       /* Effectiveness of mage-style spells       */
     sh_int       languages;    /* Max # of languages ch may learn          */ /* Unused */
};

struct  wis_app_type
{
     sh_int       practice;     /* How many practices ch gets per level    */
     sh_int       cspell;       /* Effectiveness of cleric-style spells    */
     sh_int       perception;   /* Character's perceptiveness              */ /* Unused */
};

struct  dex_app_type
{
     sh_int       defensive;    /* Bonus to dodge, parry, etc              */ /* Unused */
     sh_int       dualmod;      /* Modifier to dual wielding               */ /* Unused */
     sh_int       xhit;         /* Chance for extra attack                 */ /* Unused */
};

struct  con_app_type
{
     sh_int       hitp;         /* Extra hitpoints upon level             */
     sh_int       shock;        /* Modifier to avoid Bad Things (TM)      */ /* Unused */
};

//struct  cha_app_type            /* charisma */
//{
//       sh_int       bluff;        /* Mod to not getting caught by guards when flagged Thief or Killer */
//       sh_int       charm;        /* Mod to how well your charm spells/skills work on others */
//       sh_int       seduce;       /* Mod to not be attacked by mobs of opposite gender */
//};

/* Shops */
struct  shop_data
{
     SHOP_DATA *next;                      /* Next shop in list           */
     int        keeper;                    /* Vnum of shop keeper mob     */
     sh_int     buy_type [MAX_TRADE];      /* Item types shop will buy    */
     sh_int     profit_buy;                /* Cost multiplier for buying  */
     sh_int     profit_sell;               /* Cost multiplier for selling */
     sh_int     open_hour;                 /* First opening hour          */
     sh_int     close_hour;                /* First closing hour          */
};


/* Grouping */
struct char_group
{
          struct char_group   *next;
          CHAR_DATA           *gch;
};

/* Aliases on Players */
struct alias_data
{
          char   *name;
          char   *command_string;
};


/* Missed Tells */
struct afk_tell_type
{
     char   *message;
     struct afk_tell_type *next;
};


/* A kill structure (indexed by level). */
struct  kill_data
{
     sh_int             number;
     sh_int             killed;
};

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
     char *      spec_name;
     SPEC_FUN *  spec_fun;
};

/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
     char * name;
     int    bit;
     bool   settable;
};


/* Liquids */
struct  liq_type
{
     char *     liq_name;
     char *     liq_color;
     sh_int     liq_affect[3];
};

/* Extended Descriptions */
struct  extra_descr_data
{
     EXTRA_DESCR_DATA   *next;          /* Next in list                     */
     char               *keyword;       /* Keyword in look/examine          */
     char               *description;   /* What to see                      */
};



/*-------------------
 * The Larger Structs
 *------------------*/

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct  mob_index_data
{
     MOB_INDEX_DATA *   next;
     SPEC_FUN *         spec_fun;
     SHOP_DATA *        pShop;
     PROG_LIST *        mprogs;
     AREA_DATA *        area;           /* OLC */
     int                vnum;
     bool               new_format;
     sh_int             count;
     sh_int             killed;
     char *             player_name;   /* Odd... why player_name? No players here... */
     char *             short_descr;
     char *             long_descr;
     char *             description;
     char *             notes;
     long               act;
     long               affected_by;    /* Old affected_by */
     long               detections;     /* affected bits 2 */
     long               protections;    /* affected bits 3 */
     sh_int             alignment;
     sh_int             level;
     sh_int             hitroll;
     sh_int             hit[3];
     sh_int             mana[3];
     sh_int             damage[3];
     sh_int             dam_type;
     long               off_flags;
     long               imm_flags;
     long               res_flags;
     long               vuln_flags;
     sh_int             start_pos;
     sh_int             default_pos;
     sh_int             sex;
     sh_int             race;
     long               gold;
     long               form;
     long               parts;
     sh_int             size;
     sh_int             material;
     int                total_teach_skills;
     char               *teach_skills[MAX_TEACH_SKILLS];
     long               mprog_flags;
};

/*
 * One character (PC or NPC).  Be sure to add fields here always!
 * Actually, only if you need them for mobs dammit.
 * Otherwise use pcdata, that's why it's called pcdata!
 * Need to move a lot of those over to pcdata sometime, the only reason I haven't done so
 * yet is each requires a lot of sanity checking in the code to keep from trying to access pcdata
 * on an NPC.
 * Would save lots of space.
 */

struct  char_data
{
     CHAR_DATA *        next;
     CHAR_DATA *        next_in_room;
     CHAR_DATA *        master;
     CHAR_DATA *        leader;
     CHAR_DATA *        fighting;
     CHAR_DATA *        reply;
     CHAR_DATA *        pet;
     CHAR_DATA *        mprog_target;
     SPEC_FUN *         spec_fun;
     MOB_INDEX_DATA *   pIndexData;
     RESET_DATA *       reset;
     DESCRIPTOR_DATA *  desc;
     AFFECT_DATA *      affected;
     OBJ_DATA *         carrying;
     OBJ_DATA *         on; /* furniture */
     ROOM_INDEX_DATA *  in_room;
     ROOM_INDEX_DATA *  was_in_room;
     PC_DATA *          pcdata;
     struct char_group *group[MAX_GMEMBERS]; /* Lotherius */
     char *             name;
     sh_int             version;
     char *             short_descr;
     char *             long_descr;
     char *             description;
     /* Zeran - orig fields are for polyself spell handling */
     char *             poly_name;
     char *             short_descr_orig;
     char *             long_descr_orig;
     char *             description_orig;
     sh_int             sex;
     sh_int             race;
     sh_int             level;
     sh_int             trust;
     int                recall_perm; /*vnum of permanent recall room*/
     int                recall_temp; /*vnum of temp recall room, ie, psychic anchor*/
     int                played;      /* Move to pcdata */
     int                lines;       /* for the pager  */       /* Move to pcdata */
     time_t             logon;                                  /* Move to pcdata */
     time_t             last_note;                              /* Move to pcdata */
     sh_int             note_type;                              /* Move to pcdata */
     sh_int             timer;
     sh_int             wait;
     sh_int             hit;
     sh_int             max_hit;
     sh_int             mana;
     sh_int             max_mana;
     sh_int             move;
     sh_int             max_move;
     long               gold;
     int                exp;
     long               act;
     long               comm;                                    /* RT added to pad the vector */
     long               imm_flags;
     long               res_flags;
     long               vuln_flags;
     sh_int             invis_level;
     sh_int             cloak_level;                             /* Move to pcdata */
     long               affected_by;
     long               detections;                              /* Affected bits 2 */
     long               protections;                             /* Affected bits 3 */
     sh_int             position;
     sh_int             carry_weight;
     sh_int             carry_number;
     sh_int             saving_throw;
     sh_int             alignment;
     sh_int             hitroll;
     sh_int             damroll;
     sh_int             armor;					 /* Now just armor, for magical modifiers */
     sh_int             wimpy;                                   /* Move to pcdata */
     /* stats */
     sh_int             perm_stat[MAX_STATS];
     sh_int             mod_stat[MAX_STATS];
     /* parts stuff */
     long               form;
     long               parts;
     sh_int             size;
     /*  Zeran - added encumbrance */
     sh_int             encumbrance;
     sh_int             material;
     /* mobile stuff */
     long               off_flags;
     sh_int             damage[3];
     sh_int             dam_type;
     sh_int             start_pos;
     sh_int             default_pos;
     bool               searching;
     bool               quitting;
     char *             speaking;     /* Here for the day when mobiles speak their own languages too */
     sh_int             mprog_delay;
};


/*
 *  * Data which only PC's have.
 *  */
struct  pc_data
{
     PC_DATA *                  next;
     char *                     pwd;
     int                        pwd_tries;
     char *                     bamfin;
     char *                     bamfout;
     char *                     title;
     char *                     email;
     char *                     immtitle;
     char *                     mplaying;                  /* Music playing, not multi .. heh */
     struct alias_data *        aliases[MAX_ALIAS];
     bool                       has_alias;
     struct afk_tell_type *     afk_tell_first;
     struct afk_tell_type *     afk_tell_last;
     sh_int                     pcrace;                    /* For pc racetable */
     int                        startyear;
     int                        startmonth;
     int                        startday;
     int                        age_mod;                   /* for youth spells, etc */
     sh_int                     perm_hit;
     sh_int                     perm_mana;
     sh_int                     perm_move;
     sh_int                     true_sex;
     bool                       mortal;                    /* True for mortal, False for Demi-God */
     int                        last_level;
     int                        security;                  /* OLC */ /* Builder security */
     sh_int                     condition[3];
     char *                     prompt;
     sh_int                     learned[MAX_SKILL];
     sh_int                     points;
     bool                       confirm_delete;
     int                        battle_rating;
     int                        pkill_wins;
     int                        pkill_losses;
     int                        home_room;                 /*for immortals*/
     long	                mob_rating;                /* same as battle rating but for mobs */
     long                       mob_wins;
     long                       mob_losses;
     struct account_type *      account;
     struct clan_main_type *    clan;
     struct clan_main_type *    cedit;                  /* Clan being edited. */
     struct clan_main_type *    petition;               /* Clan trying to join */
     sh_int                     clrank;                 /* Clan rank */
     BOARD_DATA *               board;                  /* The current board */
     time_t                     last_note[MAX_BOARD];   /* last note for the boards */
     NOTE_DATA *                in_progress;
     CHAR_DATA *                questgiver;
     long                       questpoints;
     sh_int                     nextquest;
     sh_int                     countdown;
     int                        questobj;
     int                        questmob;
     long                       questearned;          /* lotherius - total ever earned */
     sh_int                     mvolume;
     sh_int                     svolume;
     sh_int                     mode;                 /* MODE... used to flag a few functions */
     long			bankaccount;
     sh_int             	pclass;
     long               	notify;
     long               	wiznet;
     sh_int             	practice;
     sh_int             	train;
    I3_CHARDATA                  *i3chardata;
};

/*
 * Prototype for an object.
 */
struct  obj_index_data
{
     AREA_DATA         *area;
     OBJ_INDEX_DATA    *next;
     EXTRA_DESCR_DATA  *extra_descr;
     AFFECT_DATA       *affected;
     PROG_LIST *        oprogs;
     long               oprog_flags;
     bool               new_format;
     char *             name;
     char *             short_descr;
     char *             description;
     char *             notes;
     int                vnum;
     sh_int             repop;
     sh_int             reset_num;
     sh_int             material;
     sh_int             item_type;
     int                extra_flags;
     int                wear_flags;
     long               vflags;		// Flags of this object
     sh_int             level;
     sh_int             condition;
     sh_int             count;
     sh_int             weight;
     sh_int             size;
     int                cost;
     int                value[5];
};

/*
 * One object.
 */
struct  obj_data
{
     OBJ_DATA *           next;
     OBJ_DATA *           next_content;
     OBJ_DATA *           contains;
     OBJ_DATA *           in_obj;
     CHAR_DATA *          on; /* furniture */
     CHAR_DATA *          carried_by;
     RESET_DATA *         reset;
     EXTRA_DESCR_DATA *   extra_descr;
     AFFECT_DATA *        affected;
     OBJ_INDEX_DATA *     pIndexData;
     ROOM_INDEX_DATA *    in_room;
     CHAR_DATA *          oprog_target;
     sh_int               oprog_delay;
     bool                 enchanted;
     char *               owner; /* Current Owner */
     char *               name;
     char *               short_descr;
     char *               description;
     int                  serialnum;
     sh_int               item_type;
     int                  extra_flags;
     int                  wear_flags;
     long		  vflags;
     sh_int               wear_loc;
     sh_int               weight;
     int                  cost;
     sh_int               level;
     sh_int               condition;
     sh_int               material;
     sh_int               size;          /* Zeran - added for equipment size restrictions */
     sh_int               timer;
     int                  value[5];
     int                  valueorig[5];  /* Zeran - added for condition changes */
     char *               origin;        /* Where the object came from - for lore. autogenerated by code */
};

/*
 * Exit data.
 */
struct  exit_data
{
     union
     {
          ROOM_INDEX_DATA *     to_room;
          int                   vnum;
     }
     u1;
     sh_int             exit_info;
     int                key;
     char *             keyword;
     char *             description;
     EXIT_DATA *        next;
     int                rs_flags;
     int                orig_door;
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 * Zeran - modified for better functionality 
 */
struct  reset_data
{
     RESET_DATA *       next;
     char               command;
     int                arg1;
     int                arg2;
     int                arg3;
     int                count;
};


/*
 * Area definition.
 */
struct  area_data
{
     AREA_DATA *         next;
     char *              name;
     int                 zone;
     int                 age;
     int                 nplayer;
     bool                empty;
     char *              filename;
     char *              builders;    /* Listing of */
     char *              soundfile;   /* The name of the midi for this area */
     char *              credits;     /* Credits */
     int                 security;    /* Value 1-9  */
     int                 lvnum;       /* Lower vnum */
     int                 uvnum;       /* Upper vnum */
     int                 vnum;        /* Area vnum  */
     int                 llev;        /* Low Level Suggested */
     int                 hlev;        /* High Level Suggested */
     int                 area_flags;
};

struct lease_data
{
     LEASE                 *next;
     ROOM_INDEX_DATA       *room;
     struct clan_main_type *clan;
     bool                   valid;
     int                    room_rent;
     char                  *rented_by;
     bool                   owner_only;
     int                    paid_month;
     int                    paid_day;
     int                    paid_year;
     char                  *lease_descr;
     char                  *lease_name;
     sh_int                 shop_type;
     int                   shop_gold;
     bool                   changed;
};
/*
 * Room type.
 */
struct  room_index_data
{
     ROOM_INDEX_DATA   *next;
     CHAR_DATA         *people;
     OBJ_DATA          *contents;
     //  AFFECT_DATA   *affected;
     EXTRA_DESCR_DATA  *extra_descr;
     AREA_DATA         *area;
     EXIT_DATA         *exit[6];
     RESET_DATA        *reset_first;
     RESET_DATA        *reset_last;
     PROG_LIST *        rprogs;
     CHAR_DATA *        rprog_target;
     long               rprog_flags;
     sh_int             rprog_delay;
     char              *name;
     char              *description;
     char              *notes;
     int                vnum;
     int                room_flags;
     sh_int             light;
     sh_int             sector_type;
     LEASE             *lease;
}; 

/*-------------------
 * MobProg Structures
 *------------------*/

struct prog_list
{
     int              trig_type;
     char *           trig_phrase;
     int              vnum;
     char *           code;
     PROG_LIST *      next;
     bool             valid;
};
 
struct prog_code
{
     int              vnum;
     char *           code;
     PROG_CODE *      next;
};

#endif /* GLOBALMERC_H */
