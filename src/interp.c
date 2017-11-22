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
#include "interp.h"

/* Locals */

bool check_social   	args ( ( CHAR_DATA * ch, char *command, char *argument ) );
bool expand_aliases 	args ( ( CHAR_DATA * ch, char *orig_command, char *final_command ) );
void check_multi_cmd 	args ( ( CHAR_DATA * ch, char *orig_cmd, char *final_cmd ) );

/* Some structs that have to be done. */

struct	disable_cmd_type 	*disable_cmd_list;
struct	crier_type  		*crier_list;
struct	account_type 		*account_list;

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Log-all switch.
 */
bool fLogAll = FALSE;

/*
 * Added Help Text field for a better commands display.
 */

/*
 * Command table.
 */

const struct cmd_type cmd_table[] =
{
     // More common commands toward the top of the list so they abbreviate first.
     
     { "cast", 		do_cast, 	POS_FIGHTING, 	0, 	LOG_NORMAL, 1, "Cast a spell.",
               CMD_MAGIC | CMD_SKILL     },
     { "north", 	do_north, 	POS_STANDING, 	0, 	LOG_NEVER,  1, "Move north.",
               CMD_MOVE     },
     { "east", 		do_east, 	POS_STANDING, 	0, 	LOG_NEVER,  1, "Move east.",
               CMD_MOVE     },
     { "south", 	do_south, 	POS_STANDING,	0, 	LOG_NEVER,  1, "Move west.",
               CMD_MOVE     },
     { "west", 		do_west, 	POS_STANDING, 	0, 	LOG_NEVER,  1, "Move west.",
               CMD_MOVE     },
     { "up", 		do_up, 		POS_STANDING, 	0, 	LOG_NEVER,  1, "Move up.",
               CMD_MOVE     },
     { "down", 		do_down, 	POS_STANDING, 	0, 	LOG_NEVER,  1, "Move down.",
               CMD_MOVE     },
     { "open", 		do_open, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, "Open a door or object.",
               CMD_MOVE | CMD_OBJECTS },
     { "score", 	do_score, 	POS_DEAD, 	0, 	LOG_NORMAL, 1, "Show character score.",
               CMD_SELFINFO     },
     { "stats",  	do_stats,   	POS_DEAD,       0,  	LOG_NORMAL, 1, "Show character stats.",
               CMD_SELFINFO     },
     { "at", 		do_at, 		POS_DEAD, 	L6,	LOG_NORMAL, 1,	"Perform action at location.",
               CMD_IMM     },
     { "auction", 	do_auction,	POS_SLEEPING, 	0, 	LOG_NORMAL, 1,	"Chat on the AUCTION channel.",
               CMD_COMM     },
     { "buy", 		do_buy, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Purchase something from a shop.",
               CMD_OBJECTS     },
     { "channels", 	do_channels, 	POS_DEAD, 	0, 	LOG_NORMAL, 0,	"Use config now.",
               CMD_COMM     },
     { "cscore",	do_cscore,	POS_DEAD,	0,	LOG_NORMAL, 1, "Show Custom Score.",
               CMD_SELFINFO     },
     { "enter", 	do_enter, 	POS_STANDING, 	0, 	LOG_NORMAL, 1,	"Enter a portal.",
               CMD_OBJECTS | CMD_MOVE     },
     { "exits", 	do_exits, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Show room's exits.",
               CMD_MOVE | CMD_WORLDINFO     },
     { "get", 		do_get, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Pick up an object.",
               CMD_OBJECTS     },
     { "goto", 		do_goto, 	POS_DEAD, 	L8, 	LOG_NORMAL, 1,	"Goto a location.",
               CMD_IMM     },
     { "hit", 		do_kill, 	POS_FIGHTING, 	0, 	LOG_NORMAL, 0,	"Initiate combat.",
               CMD_COMBAT     },
     { "inventory", 	do_inventory, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"What are you carrying?",
               CMD_SELFINFO | CMD_OBJECTS     },
     { "info", 		do_info, 	POS_DEAD,  	0,  	LOG_NORMAL, 1,  "Various other character information.",
               CMD_SELFINFO     },
     { "kill", 		do_kill, 	POS_FIGHTING, 	0,	LOG_NORMAL, 1,	"Initiate combat.",
               CMD_COMBAT     },
     { "look", 		do_look, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Look around or at something.",
               CMD_SELFINFO | CMD_OTHERINFO | CMD_WORLDINFO | CMD_OBJECTS | CMD_ACTION },
     { "learn", 	do_learn, 	POS_SLEEPING, 	0,	LOG_NORMAL, 0,	"Learn specialized skills.",
               CMD_SELFINFO | CMD_SKILL     },
     { "music",  	do_music, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1,	"Chat on the MUSIC channel.",
               CMD_COMM },
     { "order", 	do_order, 	POS_RESTING, 	0, 	LOG_ALWAYS, 1,	"Command a charm or pet.",
               CMD_COMM | CMD_SKILL },
     { "practice", 	do_practice, 	POS_SLEEPING,	0,	LOG_NORMAL, 1,	"Increase skill/spell ability.",            
               CMD_SKILL },
     { "rest", 		do_rest, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1,	"Resting position.",
               CMD_ACTION },
     { "sit", 		do_sit, 	POS_SLEEPING, 	0,	LOG_NORMAL, 1,	"Sit down.",
               CMD_ACTION },
     { "scan", 		do_scan, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Scope out nearby rooms.",
               CMD_WORLDINFO },
     { "stand", 	do_stand, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1,	"Stand up.",
               CMD_ACTION },
     { "tell", 		do_tell, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Send a private message.",
               CMD_COMM },
     { "wield", 	do_wear, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Wield a weapon.",
               CMD_COMBAT | CMD_SKILL | CMD_OBJECTS },
     { "bs", 		do_backstab,    POS_STANDING, 	0, 	LOG_NORMAL, 1,	"Shortcut for backstab.",
               CMD_COMBAT | CMD_SKILL | CMD_SC },
     { "pull",   	do_pull,        POS_STANDING,   0,      LOG_NORMAL, 1,  "Pull on something.",
               CMD_ACTION | CMD_OBJECTS },
     { "push",   	do_push,        POS_STANDING,   0,      LOG_NORMAL, 1,  "Push on something.",
               CMD_ACTION | CMD_OBJECTS },
     { "climb",  	do_climb,       POS_STANDING,   0,      LOG_NORMAL, 1,  "Climb something.",
               CMD_ACTION | CMD_OBJECTS },
     { "turn",   	do_turn,        POS_STANDING,   0,      LOG_NORMAL, 1,  "Turn something.",
               CMD_ACTION | CMD_OBJECTS},
     { "play",   	do_play,        POS_STANDING,   0,      LOG_NORMAL, 1,  "Play a musical instrument.",
               CMD_ACTION | CMD_OBJECTS},
     { "twist",  	do_twist,       POS_STANDING,   0,      LOG_NORMAL, 1,  "Twist something.",
               CMD_ACTION | CMD_OBJECTS},
     { "lift",   	do_lift,        POS_STANDING,   0,      LOG_NORMAL, 1,  "Lift something.",
               CMD_ACTION | CMD_OBJECTS},
     { "dig",    	do_dig,         POS_STANDING,   0,      LOG_NORMAL, 1,  "Dig in the ground.",
               CMD_ACTION},
     { "armor",		do_armor, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Current armor strength.",
               CMD_SELFINFO },
     { "areas",		do_areas, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Lists Area Credits",
               CMD_GAMEINFO | CMD_WORLDINFO },
     { "affects",	do_affect, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Show things affecting you.",
               CMD_SELFINFO | CMD_MAGIC },
     { "bug", 		do_bug, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Report a bug.",
               CMD_MISC },
     { "commands", 	do_commands, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Shows commands.",
               CMD_HELPFUL | CMD_GAMEINFO | CMD_NEWBIE },
     { "compare", 	do_compare, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Compare objects.",
               CMD_OBJECTS },
     { "consider", 	do_consider, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Size up an opponent.",
               CMD_COMBAT | CMD_OTHERINFO },
     { "count",		do_count, 	POS_SLEEPING,	0, 	LOG_NORMAL, 1,	"See max logins",
               CMD_GAMEINFO },
     { "credits", 	do_credits, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"See DikuMUD Credits",
               CMD_GAMEINFO | CMD_HELPFUL },
     { "equipment", 	do_equipment, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"What are you wearing?",
               CMD_OBJECTS | CMD_SELFINFO },
     { "examine",	do_examine,	POS_RESTING,	0, 	LOG_NORMAL, 0,	"Look at and inside an object.",
               CMD_OBJECTS },
     { "help", 		do_help, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"On-Line Help System.",
               CMD_HELPFUL | CMD_GAMEINFO | CMD_NEWBIE },
     { "hlist",		do_hlist, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Help Indexing & Searching tool.",
               CMD_HELPFUL | CMD_GAMEINFO | CMD_NEWBIE },
     { "hours",		do_hours, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Report shopkeeper hours.",
               CMD_MISC | CMD_WORLDINFO },
     { "motd", 		do_motd, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"See the Message of the Day.",
               CMD_HELPFUL },
     { "notify", 	do_notify, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Set notification levels.",
               CMD_GAMEINFO | CMD_CONFIG },
     { "read", 		do_read, 	POS_RESTING,	0, 	LOG_NORMAL, 0,	"Alias for look.",
               CMD_MISC },
     { "report", 	do_report, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Say your condition to the room.",
               CMD_SELFINFO | CMD_COMM },
     { "rules",		do_rules, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Review the basic game rules.",
               CMD_HELPFUL | CMD_NEWBIE },
     { "ownedwhere", 	do_owned, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Locate objects you OWN.",
               CMD_SELFINFO | CMD_OBJECTS },
     { "skills", 	do_skills, 	POS_DEAD, 	0,	LOG_NORMAL, 1,	"List your OR class skills.",
               CMD_SKILL },
     { "socials", 	do_socials, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"List game socials.",
               CMD_COMM | CMD_ACTION },
     { "spells", 	do_spells, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"List your OR class spells.",
               CMD_SKILL | CMD_MAGIC | CMD_SELFINFO },
     { "story",		do_story, 	POS_DEAD, 	0, 	LOG_NORMAL, 0,	"Alias for help story.",
               CMD_MISC },
     { "time", 		do_time, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"View game time.",
               CMD_WORLDINFO },
     { "calendar",	do_calendar,	POS_RESTING,	0,	LOG_NORMAL, 1,  "View the Game Calendar.",
               CMD_MISC },
     { "date", 		do_time, 	POS_DEAD, 	0, 	LOG_NORMAL, 0,	"Alias for time.",
               CMD_WORLDINFO },
     { "typo", 		do_typo, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Report a Typo",
               CMD_MISC },
     { "weather", 	do_weather, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"See current game weather.",
               CMD_WORLDINFO },
     { "who", 		do_who, 	POS_DEAD, 	0,	LOG_NORMAL, 1,	"List visible online players.",
               CMD_OTHERINFO },
     { "whois",		do_whois, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"About a character.",
               CMD_OTHERINFO },
     { "wizlist", 	do_wizlist, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"List staff members.",
               CMD_MISC },
     { "worth",		do_worth, 	POS_SLEEPING,	0,	LOG_NORMAL, 1,	"View current Gold & XP",
               CMD_SELFINFO },
     { "version", 	do_version,	POS_SLEEPING,	0, 	LOG_NORMAL, 1,  "Show Software Version & Time",
               CMD_GAMEINFO },
     { "lore", 		do_lore,	POS_STANDING,	0, 	LOG_NORMAL, 1,	"Skill to find info on an item.",
               CMD_OBJECTS },
     /* Clan Commands */
     { "clan",		do_clan_tell, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Chat on your Clan Channel",
               CMD_COMM | CMD_CLAN },
     { "?",      	do_clan_tell,   POS_DEAD,   	0, 	LOG_NORMAL, 1,	"Shortcut for clan chat.",
               CMD_COMM| CMD_CLAN | CMD_SC },
     { "clist", 	list_clans, 	POS_SLEEPING,	0, 	LOG_NORMAL, 1,	"List Recognized Clans.",
               CMD_CLAN },
     { "claninfo", 	do_claninfo, 	POS_SLEEPING,	0, 	LOG_NORMAL, 1,	"Get info on a clan.",
               CMD_CLAN },
     { "clanpromote", 	clan_advance, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,"Promote a clanmember.",
               CMD_CLAN },
     { "clandemote", 	clan_demote, 	POS_RESTING,	0, 	LOG_NORMAL, 1,	"Demote a clanmember.",
               CMD_CLAN },
     { "clanaccept", 	clan_accept, 	POS_RESTING,	0, 	LOG_ALWAYS, 1,	"Induct a new clanmember.",
               CMD_CLAN },
     { "clanoutcast",	clan_outcast, 	POS_RESTING, 	0, 	LOG_ALWAYS, 1,"Remove a clanmember.",
               CMD_CLAN },
     { "clantruce",   	clan_truce,   	POS_RESTING, 	0, 	LOG_ALWAYS, 1, "Cancel a war your clan has declared.",
               CMD_CLAN },
     { "clanwar",     	clan_declare,  	POS_STANDING, 	0, 	LOG_ALWAYS, 1, "Declare war on a clan.",
               CMD_CLAN },
     { "petition",    	clan_petition, 	POS_RESTING, 	0, 	LOG_ALWAYS, 1, "Petition to join a clan.",
               CMD_CLAN },
     { "makeclan", 	make_clan, 	POS_RESTING, 	0, 	LOG_ALWAYS, 1, "Start a new clan.",
               CMD_CLAN },
     { "recognize", 	clan_recognize, POS_RESTING, 	0, 	LOG_NORMAL, 1, "For DemiGods and Imms to accept a clan.",
               CMD_CLAN },
     { "donate", 	clan_donate, 	POS_STANDING, 	0, 	LOG_NORMAL, 1, "Donate gold, experience, or an object to your clan.",
               CMD_CLAN },
     /* Configuration commands. */
     {"alias", 		do_alias, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Create a shortcut command.",
               CMD_CONFIG },
     {"unalias", 	do_unalias, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Clear a shortcut command.",
               CMD_CONFIG },
     {"auto",		do_config,	POS_DEAD,	0,	LOG_NORMAL, 0,  "Alias for Config",
               CMD_CONFIG },
     {"config", 	do_config, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Settings Configuration.",
               CMD_CONFIG },
     {"colour", 	do_colour, 	POS_DEAD, 	0, 	LOG_NORMAL, 0,	"Toggle colour on/off.",
               CMD_CONFIG },
     {"color", 		do_colour, 	POS_DEAD, 	0, 	LOG_NORMAL, 0,	"Toggle colour on/off.",
               CMD_CONFIG },
     {"description", 	do_description, POS_DEAD,	0, 	LOG_NORMAL, 1,	"Edit your 'description'.",
               CMD_CONFIG | CMD_SELFINFO },
     {"delet", 		do_delet, 	POS_DEAD,	0, 	LOG_ALWAYS, 0,	"To keep you from accidentally deleting.",
               CMD_MISC },
     {"delete", 	do_delete, 	POS_DEAD, 	0, 	LOG_ALWAYS, 1,	"Destroy this character.",
               CMD_MISC },
     {"email", 		do_email, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Set public email address.",
               CMD_CONFIG },
     {"outfit",		do_outfit, 	POS_RESTING,	0, 	LOG_ALWAYS, 1,	"Equips basic eq.",
               CMD_OBJECTS | CMD_NEWBIE },
     {"password", 	do_password, 	POS_DEAD, 	0, 	LOG_NEVER,  1,	"Change character password.",
               CMD_CONFIG },
     {"prompt", 	do_prompt, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Set customizable Prompt.",
               CMD_CONFIG },
     {"scroll", 	do_scroll, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Adjust screen paging.",
               CMD_CONFIG },
     {"sound", 		do_sound, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Toggles MSP Sound.",
               CMD_CONFIG },
     {"stop", 		do_stop, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Stops currently playing sound.",
               CMD_CONFIG },
     {"title", 		do_title, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Configure your character Title.",
               CMD_SELFINFO | CMD_CONFIG },
     {"wimpy", 		do_wimpy, 	POS_DEAD, 	0, 	LOG_NORMAL, 1,	"Set HP at which you flee.",
               CMD_COMBAT | CMD_CONFIG },
     {"xinfo", 		do_xinfo, 	POS_DEAD, 	IM, 	LOG_NORMAL, 1,	"Shows IMMs Xtra Exit Info.",
               CMD_WORLDINFO | CMD_CONFIG | CMD_IMM },
     /* Communication commands. */
     {"answer",		do_answer, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Answer on the QUESTION channel.",
               CMD_COMM | CMD_NEWBIE },
     {"afk", 		do_afk, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Sets AWAY status.",
               CMD_COMM },
     {"deaf",		do_deaf, 	POS_DEAD, 	0, 	LOG_NORMAL, 1, 	"Toggles ability to hear tells.",
               CMD_COMM },
     {"emote", 		do_emote, 	POS_RESTING,	0, 	LOG_NORMAL, 1, 	"Express an action.",
               CMD_COMM | CMD_ACTION },
     {".", 		do_gossip, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Shortcut for Gossip.",
               CMD_COMM | CMD_SC },
     {"gossip", 	do_gossip, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Chat on the GOSSIP channel.",
               CMD_COMM },
     {",", 		do_emote, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Shortcut for Emote.",
               CMD_COMM | CMD_ACTION | CMD_SC },
     {"gtell", 		do_gtell, 	POS_DEAD, 	0, 	LOG_NORMAL, 1, 	"Chat with your GROUP.",
               CMD_COMM },
     {";", 		do_gtell, 	POS_DEAD, 	0, 	LOG_NORMAL, 1, 	"Shortcut for Gtell.",
               CMD_COMM | CMD_SC },
     {"speak", 		do_language, 	POS_DEAD, 	0, 	LOG_NORMAL, 1, 	"Choose language to speak in.",
               CMD_COMM | CMD_CONFIG },
     {"note", 		do_note, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Access NOTE functions.",
               CMD_COMM },
     {"notes",		do_note, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 0, 	"Note alias for the braindead.",
               CMD_COMM },
     {"quest", 		do_quest, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Request a Generic Quest.",
               CMD_ACTION },
     {"question", 	do_question, 	POS_SLEEPING,	0,	LOG_NORMAL, 1, 	"Ask on the QUESTION channel.",
               CMD_COMM | CMD_NEWBIE },
     {"quiet", 		do_quiet, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"Ignore all public channels.",
               CMD_COMM | CMD_CONFIG },
     {"reply", 		do_reply, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Send a reply to a private tell.",
               CMD_COMM },
     {"replay", 	do_replay, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Display tells received while AFK.",
               CMD_COMM },
     {"say", 		do_say, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Say something.",
               CMD_COMM },
     {"'", 		do_say, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Shortcut for Say.",
               CMD_COMM | CMD_SC },
     {"shout", 		do_shout, 	POS_RESTING, 	3, 	LOG_NORMAL, 1, 	"Shout something to the mud.",
               CMD_COMM },
     {"yell", 		do_yell, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Yell something to the area.",
               CMD_COMM },
     {"beep", 		do_beep, 	POS_DEAD, 	2, 	LOG_ALWAYS, 1, 	"Send someone a beep.",
               CMD_COMM },
     {"boards", 	do_board, 	POS_SLEEPING, 	0, 	LOG_NORMAL, 1, 	"View active message boards.",
               CMD_COMM },
     /* Object manipulation commands. */
     {"brandish", 	do_brandish, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Activate a staff.",
               CMD_MAGIC | CMD_SKILL | CMD_COMBAT },
     {"brew", 		do_brew, 	POS_STANDING, 	0, 	LOG_NORMAL, 1, 	"Make a potion.",
               CMD_MAGIC | CMD_SKILL },
     {"scribe", 	do_scribe, 	POS_STANDING, 	0, 	LOG_NORMAL, 1, 	"Write a scroll.",
               CMD_MAGIC | CMD_SKILL },
     {"close", 		do_close, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,	"Close a door or object.",
               CMD_MOVE | CMD_OBJECTS },
     {"drink", 		do_drink, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Quench thirst.",
               CMD_OBJECTS | CMD_MISC },
     {"drop", 		do_drop, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Remove item from inventory.",
               CMD_OBJECTS },
     {"eat", 		do_eat, 	POS_RESTING, 	0, 	LOG_NORMAL, 1,  "Quench hunger.",
               CMD_OBJECTS | CMD_MISC },
     {"envenom", 	do_envenom, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Add poison to a weapon.",
               CMD_SKILL | CMD_OBJECTS },
     {"fill", 		do_fill, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Fill a drink container.",
               CMD_OBJECTS },
     {"give", 		do_give, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Give something to someone.",
               CMD_OBJECTS | CMD_ACTION },
     {"heal", 		do_heal, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Use the services of a healer.",
               CMD_HELPFUL },
     {"hold", 		do_wear,	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Hold something in your hand.",
               CMD_OBJECTS },
     {"list", 		do_list, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"List items in a shop.",
               CMD_OBJECTS },
     {"lock", 		do_lock, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Lock a door or container.",
               CMD_SKILL },
     {"pick", 		do_pick, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Pick a lock.",
               CMD_SKILL },
     {"put", 		do_put, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Put something in something.",
               CMD_OBJECTS },
     {"quaff", 		do_quaff, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Imbibe a potion.",
               CMD_MAGIC | CMD_OBJECTS },
     {"recite", 	do_recite, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Activate a scroll.",
               CMD_MAGIC | CMD_OBJECTS },
     {"remove", 	do_remove, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Take something off.",
               CMD_OBJECTS },
     {"resize", 	do_resize, 	POS_RESTING, 	0, 	LOG_NORMAL, 0, 	"Resize some equipment.",
               CMD_OBJECTS },
     {"repair", 	do_repair, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Repair worn equipment.",
               CMD_OBJECTS },
     {"search", 	do_search, 	POS_STANDING, 	0, 	LOG_NORMAL, 1, 	"Look for concealed objects or doors.",
               CMD_ACTION | CMD_WORLDINFO },
     {"sell", 		do_sell, 	POS_RESTING, 	0, 	LOG_NORMAL, 1, 	"Sell an item to a shop.",
               CMD_OBJECTS },
     {"take", 		do_get, 	POS_RESTING, 	0, 	LOG_NORMAL, 0, 	"Alias for get.",
               CMD_OBJECTS | CMD_ACTION },
     {"sacrifice", 	do_sacrifice, 	POS_RESTING,	0,	LOG_NORMAL, 1, 	"Sacrifice something.",
               CMD_OBJECTS },
     {"unlock", 	do_unlock,	POS_RESTING, 	0,	LOG_NORMAL, 1, 	"Unlock a door or an object.",
               CMD_SKILL | CMD_MOVE | CMD_OBJECTS },
     {"value",		do_value,	POS_RESTING,	0,	LOG_NORMAL, 1,	"How much will a shop pay for X.",
               CMD_OBJECTS },
     {"wear", 		do_wear, 	POS_RESTING,	0,	LOG_NORMAL, 1,	"Wear something on your body.",
               CMD_OBJECTS },
     {"zap",		do_zap,		POS_RESTING,	0,	LOG_NORMAL, 1,	"Activate a wand.",
               CMD_OBJECTS | CMD_MAGIC },
     {"sharpen", 	do_sharpen, 	POS_STANDING,	0,	LOG_NORMAL, 1,	"Sharpen a weapon.",
               CMD_OBJECTS },
     /* Combat commands. */
     {"backstab",	do_backstab, 	POS_STANDING,	0,	LOG_NORMAL, 1,	"Stab someone in the back.",
               CMD_COMBAT | CMD_SKILL },
     {"circle",		do_circle,	POS_FIGHTING,	0,	LOG_NORMAL, 1,  "Attack from behind during combat.",
               CMD_COMBAT | CMD_SKILL },
     {"bash",		do_bash,	POS_FIGHTING,	0, 	LOG_NORMAL, 1,	"Knock someone over.",
               CMD_COMBAT | CMD_SKILL      },
     {"berserk", 	do_berserk, 	POS_FIGHTING, 	0, 	LOG_NORMAL, 1, 	"Go into a rage.",
               CMD_COMBAT | CMD_SKILL },
     {"dual",		do_dual,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Wield a weapon in your off hand.",
               CMD_COMBAT | CMD_SKILL },
     {"dirt",		do_dirt,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Kick dirt into your enemy's eyes.",
               CMD_COMBAT | CMD_SKILL },
     {"disarm",		do_disarm,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Knock the weapon from someone's grasp.",
               CMD_COMBAT | CMD_SKILL },
     {"flee",		do_flee,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Escape combat.",
               CMD_COMBAT },
     {"kick",		do_kick,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Kick someone.",
               CMD_COMBAT | CMD_SKILL },
     {"murde",		do_murde,	POS_FIGHTING,	6,	LOG_NORMAL, 0,	"To keep you from accidentally murdering.",
               CMD_MISC },
     {"murder",		do_murder,	POS_FIGHTING,	6,	LOG_ALWAYS, 1,	"PlayerKill someone.",
               CMD_COMBAT },
     {"rally",		do_rally,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Encourage your group members to victory.",
               CMD_COMBAT | CMD_SKILL },
     {"rescue",		do_rescue,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Save someone from peril.",
               CMD_COMBAT | CMD_SKILL },
     {"surrender",	do_surrender,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Give in to your enemy.",
               CMD_COMBAT },
     {"rotate",		do_rotate,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Change targets in combat.",
               CMD_COMBAT | CMD_SKILL },
     {"trip", 		do_trip,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Try to trip someone.",
               CMD_COMBAT },
     /* Mob command interpreter (placed here for faster scan...) */
     {"mob",		do_mob,		POS_DEAD,	0,	LOG_NEVER,  0,	"Zilch.", 0 },
     /* Miscellaneous commands. */
     {"follow",		do_follow,	POS_RESTING,	0,	LOG_NORMAL, 1,	"Follow the leader.",
               CMD_MISC },
     {"group",		do_group,	POS_SLEEPING,	0,	LOG_NORMAL, 1,	"View & Change your group.",
               CMD_MISC | CMD_OTHERINFO },
     {"hide",		do_hide,	POS_RESTING,	0,	LOG_NORMAL, 1,	"Hide in shadows.",
               CMD_SKILL },
     {"qui",		do_qui,		POS_DEAD,	0,	LOG_NORMAL, 0,	"To keep you from accidentally quitting.",
               CMD_MISC },
     {"quit",		do_quit,	POS_DEAD,	0,	LOG_NORMAL, 1,	"Leave the game for a while.",
               CMD_MISC },
     {"recall",		do_recall,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Ask your deity to return you home.",
               CMD_HELPFUL },
     {"/",		do_recall,	POS_FIGHTING,	0,	LOG_NORMAL, 1,	"Shortcut for Recall.",
               CMD_HELPFUL | CMD_SC },
     {"rent",		do_rent,	POS_DEAD,	0,	LOG_NORMAL, 0,	"Tells you Sunder doesn't do rent.",
               CMD_MISC },
     {"lease",		do_lease,	POS_STANDING,	0,	LOG_ALWAYS, 1,	"Lease a room.",
               CMD_LEASE },
     {"save",		do_save,	POS_DEAD,	0,	LOG_NORMAL, 1,	"Save your character NOW.",
               CMD_MISC },
     {"sleep",		do_sleep,	POS_DEAD,	0,	LOG_NORMAL, 1,	"Goes to sleep.",
               CMD_ACTION },
     {"sneak",		do_sneak,	POS_STANDING,	0,	LOG_NORMAL, 1,	"Move without being heard/seen.",
               CMD_SKILL },
     {"split",		do_splitc,	POS_RESTING,	0,	LOG_NORMAL, 1,	"Split X gold.",
               CMD_ACTION },
     {"steal",		do_steal,	POS_STANDING,	0,	LOG_NORMAL, 1,	"Borrow some money or items.",
               CMD_SKILL },
     {"bank",		do_bank,	POS_STANDING,	0,	LOG_NORMAL, 1,  "View your bank account.",
               CMD_SELFINFO | CMD_MISC },
     {"withdraw",	do_withdraw,	POS_STANDING,	0,	LOG_NORMAL, 1,  "Withdraw gold from the bank.",
               CMD_MISC    },
     {"deposit",	do_deposit,	POS_STANDING,	0,	LOG_NORMAL, 1,  "Deposit gold into the bank.",
               CMD_MISC     },
     {"borrow",		do_borrow,	POS_STANDING,	0,	LOG_NORMAL, 1,	"Take out a loan from the bank.",
               CMD_MISC    },
     {"visible", 	do_visible, 	POS_SLEEPING,	0,	LOG_NORMAL, 1,	"Cancel invisibility/sneak/hide.",
               CMD_ACTION },
     {"wake",		do_wake,	POS_SLEEPING,	0,	LOG_NORMAL, 1,	"Wake and stand up.",
               CMD_ACTION },
     {"where",		do_where,	POS_RESTING,	0,	LOG_NORMAL, 1,	"Find out what area you're in.",
               CMD_WORLDINFO },
     {"private", 	do_private, 	POS_RESTING,	0,	LOG_NORMAL, 1,	"Make a leased room private.",
               CMD_LEASE },
     {"roomname",	do_roomname, 	POS_RESTING,	0,	LOG_NORMAL, 1,	"Set the name of a leased room.",
               CMD_LEASE },
     {"roomdesc",	do_roomdesc, 	POS_RESTING,	0,	LOG_NORMAL, 1,	"Set the description of a leased room.",
               CMD_LEASE },
     {"nofollow",	do_nofollow, 	POS_RESTING,	0,	LOG_NORMAL, 1,	"Prevent others from following you.",
               CMD_CONFIG | CMD_MISC },
     {"verify",  	do_verify,	POS_DEAD,	0,	LOG_ALWAYS, 1,	"Request an account verification.",
               CMD_CONFIG | CMD_HELPFUL },
     {"accounts",	do_accounts,	POS_RESTING,	0,	LOG_NORMAL, 1,	"View account information.",
               CMD_CONFIG },
     {"compress",	do_showcompress,POS_SLEEPING,   0,	LOG_NORMAL, 1,  "Show compression status.",
               CMD_GAMEINFO },
     {"cedit"   ,	do_cedit,       POS_SLEEPING,   0,	LOG_ALWAYS, 1,  "Enter the Clan Editor.",
               CMD_CLAN },
     {"myleases",	do_myleases,    POS_RESTING,    0,	LOG_NORMAL, 1,  "View your leases, if any.",
               CMD_LEASE },
     {"killer", 	do_killer,      POS_RESTING,    0,	LOG_ALWAYS, 1,  "Become a KILLER.",
               CMD_COMBAT | CMD_CONFIG },
     /* 
      * Commands below here have no command help currently, since the functions that show them
      * are not designed to use command help and I'm lazy - Lotherius.
      */
     /* Immortal commands. */
     {"wizhelp",        do_wizhelp, 	POS_DEAD, 	HE, LOG_NORMAL, 1, "Lists Immortal Commands.", CMD_IMM},
     {"advance", 	do_advance, 	POS_DEAD, 	ML, LOG_ALWAYS, 1, "Raise a player's Level.", CMD_IMM},
     {"award", 		do_award, 	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Award XP to a player.", CMD_IMM},
     {"trust", 		do_trust, 	POS_DEAD, 	ML, LOG_ALWAYS, 1, "Trust a player to a higher access level.", CMD_IMM},
     {"set", 		do_set, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Change various values.", CMD_IMM},
     {"allow", 		do_allow, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Remove a banned site.", CMD_IMM},
     {"ban", 		do_ban, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Ban a sitename.", CMD_IMM},
     {"crier", 		do_crier, 	POS_DEAD, 	L2, LOG_NORMAL, 1, "Manage the Town Crier lists.", CMD_IMM},
/*   {"copy",		do_copy,	POS_STANDING,	 0, LOG_NORMAL, 1, "", CMD_IMM}, */
/*   {"clone", 		do_clone, 	POS_DEAD, 	ML, LOG_ALWAYS, 1, "", CMD_IMM}, */
     {"copyove",	do_copyove,	POS_DEAD,	L1, LOG_NORMAL, 0, "To prevent accidental copyover.", CMD_IMM},
     {"copyover",      	do_copyover,	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Hot-Start the Mud.", CMD_IMM},
     {"enable", 	do_enable, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Remove a command from the disabled list.", CMD_IMM},
     {"disable", 	do_disable, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Block access to a command to <level>.", CMD_IMM},
     {"delay",		do_delay,	POS_DEAD,	L5, LOG_ALWAYS, 1, "Lag a player.", CMD_IMM},
     {"deny", 		do_deny, 	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Lock out a pfile from the game.", CMD_IMM},
     {"disconnect", 	do_disconnect, 	POS_DEAD, 	L3, LOG_ALWAYS, 1, "Disconnect a player.", CMD_IMM},
     {"freeze", 	do_freeze, 	POS_DEAD, 	L3, LOG_ALWAYS, 1, "Prevent a player from doing anything at all.", CMD_IMM},
     {"reboo", 		do_reboo, 	POS_DEAD, 	L1, LOG_NORMAL, 0, "To prevent acc. reboot.", CMD_IMM},
     {"reboot", 	do_reboot, 	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Restart the game, dropping sockets", CMD_IMM},
     {"shutdow", 	do_shutdow, 	POS_DEAD, 	L1, LOG_NORMAL, 0, "To prevent acc. shutdown.", CMD_IMM},
     {"shutdown", 	do_shutdown, 	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Close the game completely, will not reboot.", CMD_IMM},
     {"wizlock", 	do_wizlock, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Allow only immortals to login.", CMD_IMM},
     {"wiznet", 	do_wiznet, 	POS_DEAD, 	IM, LOG_ALWAYS, 1, "Immortal Notifications.", CMD_IMM},
     {"force", 		do_force, 	POS_DEAD, 	L7, LOG_ALWAYS, 1, "Make a mob or a player issue a command.", CMD_IMM},
     {"load", 		do_load, 	POS_DEAD, 	L6, LOG_ALWAYS, 1, "Create a mob or object.", CMD_IMM},
     {"newlock", 	do_newlock, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Prevent creation of new players.", CMD_IMM},
     {"nochannels", 	do_nochannels, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Block a player's access to chat channels.", CMD_IMM},
     {"noemote", 	do_noemote, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Block a player's ability to use emote.", CMD_IMM},
     {"noshout", 	do_noshout, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Block a player's ability to shout.", CMD_IMM},
     {"notell", 	do_notell, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Block a player's ability to send private messages.", CMD_IMM},
     {"pecho", 		do_pecho, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Send an unformatted message to the target.", CMD_IMM},
     {"pardon", 	do_pardon, 	POS_DEAD, 	L3, LOG_ALWAYS, 1, "Remove someone's Killer or Thief Flag.", CMD_IMM},
     {"purge", 		do_purge, 	POS_DEAD, 	L7, LOG_ALWAYS, 1, "Destroy obj/mobs in the room (or a target).", CMD_IMM},
     {"repop", 		do_repop, 	POS_DEAD, 	L7, LOG_ALWAYS, 1, "Issue a reset in current room.", CMD_IMM},
     {"restore", 	do_restore, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Give FULL HP/MANA/MOVE, etc to everyone or target.", CMD_IMM},
     {"sla", 		do_sla, 	POS_DEAD, 	L3, LOG_NORMAL, 0, "Shortcut to prevent oops.", CMD_IMM},
     {"slay", 		do_slay, 	POS_DEAD, 	L3, LOG_ALWAYS, 1, "Mow a puny mortal down in cold blood.", CMD_IMM},
     {"transfer", 	do_transfer, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Moves someone to you or a target.", CMD_IMM},
     {"poofin", 	do_bamfin, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Sets incoming goto message.", CMD_IMM},
     {"poofout", 	do_bamfout, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Sets outgoing goto message.", CMD_IMM},
     {"gecho", 		do_echo,	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Print a string to the whole mud.", CMD_IMM},
     {"sockets", 	do_sockets, 	POS_DEAD, 	L4, LOG_NORMAL, 1, "Display all connected descriptors and state.", CMD_IMM},
     {"holylight", 	do_holylight, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Ability to see everything.", CMD_IMM},
     {"home", 		do_home, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Go to your home-room.", CMD_IMM},
     {"cloak", 		do_cloak, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Invisible to everyone not in current room (by level)", CMD_IMM},
     {"log", 		do_log, 	POS_DEAD, 	L1, LOG_ALWAYS, 1, "Sets target to be logged.", CMD_IMM},
     {"memory", 	do_memory, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "View some memory statistics.", CMD_IMM},
     {"memlog", 	do_memlog, 	POS_DEAD, 	L2, LOG_NORMAL, 1, "View more detailed memory statistics.", CMD_IMM},
     {"mwhere", 	do_mwhere, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Find all mobs with <name>.", CMD_IMM},
     {"owhere", 	do_owhere, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Find all objects with <name>.", CMD_IMM},
     {"pwhere", 	do_pwhere, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Find all players with optional <name>", CMD_IMM},
     {"peace", 		do_peace, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Cancel fighting in the current room, or 'world'.", CMD_IMM},
     {"echo", 		do_recho, 	POS_DEAD, 	L6, LOG_ALWAYS, 1, "Print a string in the current room.", CMD_IMM},
     {"return", 	do_return, 	POS_DEAD, 	L6, LOG_NORMAL, 1, "Exit from switch.", CMD_IMM},
     {"snoop", 		do_snoop, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Spy on a target.", CMD_IMM},
     {"istat", 		do_stat, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "View statistics on a target.", CMD_IMM},
     {"string", 	do_string, 	POS_DEAD, 	L5, LOG_ALWAYS, 1, "Change some text strings on a target.", CMD_IMM},
     {"switch", 	do_switch, 	POS_DEAD, 	L6, LOG_ALWAYS, 1, "Take over a mobile.", CMD_IMM},
     {"wizinvis", 	do_invis, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Become invis to all those below <level>.", CMD_IMM},
     {"vnum", 		do_vnum, 	POS_DEAD, 	L4, LOG_NORMAL, 1, "Find something's VNUM.", CMD_IMM | CMD_OLC },
     {"immtalk", 	do_immtalk, 	POS_DEAD, 	HE, LOG_NORMAL, 1, "Chat on the Immortal Channel.", CMD_IMM},
     {"imptalk", 	do_imptalk, 	POS_DEAD, 	ML, LOG_NORMAL, 1, "Chat with any other IMPLEMENTORS.", CMD_IMM},
     {"*", 		do_imptalk, 	POS_DEAD, 	ML, LOG_NORMAL, 1, "Shortcut for ImpTalk.", CMD_IMM | CMD_SC },
     {"imotd", 		do_imotd, 	POS_DEAD, 	HE, LOG_NORMAL, 1, "View the Immortal Message of the Day.", CMD_IMM},
     {"world", 		do_world, 	POS_DEAD, 	L7, LOG_NORMAL, 1, "View <obj> or <mob> totals for the mud by level.", CMD_IMM | CMD_OLC },
     {":", 		do_immtalk, 	POS_DEAD, 	HE, LOG_NORMAL, 1, "Shortcut for Immtalk Channel.", CMD_IMM | CMD_SC },
     {"listskills", 	do_listskills, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "List the skills and spells on the mud.", CMD_IMM},
     {"listraces", 	do_listraces, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Lists both PC and NPC races on the mud.", CMD_IMM},
     {"setrent", 	do_setrent, 	POS_DEAD, 	L7, LOG_ALWAYS, 1, "Set's a leased room's rent.", CMD_IMM | CMD_OLC },
     {"dump", 		do_dump, 	POS_DEAD, 	ML, LOG_NORMAL, 0, "Outputs a LIST of mobs and objs to files.", CMD_IMM},
     {"checklease", 	do_checklease, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "View stats on a room's lease.", CMD_IMM},
     {"immtitle", 	do_immtitle, 	POS_DEAD, 	IM, LOG_NORMAL, 1, "Set your ImmTitle.", CMD_IMM},
     {"statall", 	do_statall, 	POS_DEAD, 	L3, LOG_NORMAL, 0, "An unfinished function.", CMD_IMM},
     {"mpdump",		do_mpdump,	POS_DEAD,	L7, LOG_NEVER,  1, "Views code of a mobprog by VNUM.", CMD_IMM | CMD_OLC },
     {"mpstat",		do_mpstat,	POS_DEAD,	L7, LOG_NEVER,  1, "Lists mobprogs on a target mob.", CMD_IMM | CMD_OLC },
     {"reject", 	do_reject, 	POS_DEAD, 	L4, LOG_ALWAYS, 1, "Denies an account application.", CMD_IMM},
     {"setclan", 	do_setclan, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Manually set player clan pointer.", CMD_IMM},
     {"declan", 	do_declan, 	POS_DEAD, 	L2, LOG_ALWAYS, 1, "Manually remove player clan pointer.", CMD_IMM},
     {"clandelete", 	do_clandelete, 	POS_DEAD, 	ML, LOG_ALWAYS, 1, "Delete a clan.", CMD_IMM},
     {"clancharge", 	do_clancharge, 	POS_DEAD, 	L7, LOG_NORMAL, 1, "Deduct cash from target clan's bank.", CMD_IMM},
     /* OLC */
     {"edit", 		do_olc, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Main gateway to OLC editing.", CMD_OLC | CMD_IMM },
     {"asave", 		do_asave, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Saves work done in OLC (VITAL)!", CMD_OLC | CMD_IMM },
     {"import",		db_import_area, POS_DEAD,	ML, LOG_ALWAYS, 1, "Loads an area into the mud (dangerous)", CMD_OLC | CMD_IMM },
     {"alist", 		do_alist, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "Raw list of areas with OLC info.", CMD_OLC | CMD_IMM },
     {"mpedit",		do_mpedit,	POS_DEAD,	L7, LOG_NORMAL, 1, "Edit a mobprog.", CMD_OLC | CMD_IMM },
     {"opedit",         do_opedit,      POS_DEAD,	L6, LOG_ALWAYS, 1, "Edit an objprog.", CMD_OLC | CMD_IMM },
     {"rpedit",         do_rpedit,      POS_DEAD,	L6, LOG_ALWAYS, 1, "Edit a roomprog.", CMD_OLC | CMD_IMM },
     {"opdump",         do_opdump,      POS_DEAD,       L7, LOG_NEVER,  1, "Views code of an objprog by VNUM.", CMD_OLC | CMD_IMM },
     {"opstat",         do_opstat,      POS_DEAD,       L7, LOG_NEVER,  1, "Lists objprogs on a target obj.", CMD_OLC | CMD_IMM },
     {"rpdump",         do_rpdump,      POS_DEAD,       L7, LOG_NEVER,  1, "Views code of a roomprog by VNUM.", CMD_OLC | CMD_IMM },
     {"rpstat",         do_rpstat,      POS_DEAD,       L7, LOG_NEVER,  1, "Lists roomprogs on current room.", CMD_OLC | CMD_IMM },
     {"resets", 	do_resets, 	POS_DEAD, 	L8, LOG_NORMAL, 1, "View / Edit 'reset' information.", CMD_OLC | CMD_IMM },
     {"end", 		do_end, 	POS_DEAD,	L8, LOG_NORMAL, 0, "Exit the OLC Editor.", CMD_OLC | CMD_IMM },
     {"areaexits",	do_areaexits,   POS_DEAD,       L7, LOG_NORMAL, 1, "Find exits from the current area.", CMD_OLC | CMD_IMM },
     {"shoplist",   do_shoplist,    POS_DEAD,       L3, LOG_NORMAL, 1, "Shop all shops in the mud.", CMD_IMM },
     {"image",      do_image,       POS_DEAD,       IM, LOG_NORMAL, 1, "Show an image from the Mud's image library.", CMD_IMM },
     {"test",		do_testfunc,	POS_DEAD,	ML, LOG_NORMAL, 0, "Here to test experimental functions.", CMD_IMM },
     
     /* Clickable Command Handlers - Not Shown */
     {"ch_cmc",     click_context_char, POS_DEAD,   0,  LOG_NORMAL, 0,  "", 0 },
     
    /* End of list. */
     {"", 0, POS_DEAD, 0, LOG_NORMAL, 0, "", 0}
};

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret ( CHAR_DATA * ch, char *argument )
{
     char                command[MAX_INPUT_LENGTH];
     char                logline[MAX_INPUT_LENGTH];
     char                new_argument[MAX_STRING_LENGTH];
     char                new2_argument[MAX_STRING_LENGTH];
     int                 depth = 0;
     bool                expand = TRUE;
     int                 cmd;
     int                 trust;
     bool                found;
     bool                alias_cmd = FALSE;

    /*
     * Strip leading spaces.
     */
     while ( isspace ( *argument ) )
          argument++;
     if ( argument[0] == '\0' )
          return;

    /* check for alias command, if so, bypass multi command and alias
     * expansion functions -- allows embedded multi command aliasing */

     one_argument ( argument, command );
     if ( !str_cmp ( command, "alias" ) )
          alias_cmd = TRUE;

    /*
     * Implement freeze command.
     */
     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_FREEZE ) )
     {
          send_to_char ( "You're totally frozen!\n\r", ch );
          return;
     }

     /* Zeran - call check_multi_cmd and expand_aliases for any PC who
      *	is not switched and argument is ch->desc->incomm.  Prevents
      *	flooding of commands with the force or order command against
      *	other PCs.  Continue expansion until a max depth of 2
      *	is reached, or no more alias expansions have occured.
      */

     if ( !IS_NPC ( ch ) && !alias_cmd && ch->desc && argument == ch->desc->incomm )
          while ( depth < 3 && expand )
          {
               new_argument[0] = '\0';
               check_multi_cmd ( ch, argument, new_argument );
               argument = new_argument;

               if ( ch->pcdata->has_alias )
               {
                    new2_argument[0] = '\0';
                    expand = expand_aliases ( ch, argument, new2_argument );
                    if ( expand )
                         ch->wait += 2;
                    argument = new2_argument;
               }
               else
                    break;
               depth++;
          }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */

     strcpy ( logline, argument );
     if ( !isalpha ( argument[0] ) && !isdigit ( argument[0] ) )
     {
          command[0] = argument[0];
          command[1] = '\0';
          argument++;
          while ( isspace ( *argument ) )
               argument++;
     }
     else
     {
          argument = one_argument ( argument, command );
     }

    /*
     * Look for command in command table.
     */
     found = FALSE;
     trust = get_trust ( ch );
     for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
     {
          if ( command[0] == cmd_table[cmd].name[0] && !str_prefix ( command, cmd_table[cmd].name )
               && cmd_table[cmd].level <= trust )
          {
               found = TRUE;
               break;
          }
     }

    /*
     * Log and snoop.
     */
     if ( cmd_table[cmd].log == LOG_NEVER )
          strcpy ( logline, "" );

     if ( ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_LOG ) ) || fLogAll || cmd_table[cmd].log == LOG_ALWAYS )
     {
          log_string ( "Log %s: %s", ch->name, logline );
     }

     if ( !IS_NPC ( ch ) && IS_SET ( ch->act, PLR_LOG ) )
     {
          char nbuf[MSL];
          
          SNP ( nbuf, "Log %s: %s", ch->name, logline );
          notify_message ( ch, WIZNET_SECURE, TO_IMM_ADMIN, nbuf );
     }

     if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
     {
          write_to_buffer ( ch->desc->snoop_by, "% ", 2 );
          write_to_buffer ( ch->desc->snoop_by, logline, 0 );
          write_to_buffer ( ch->desc->snoop_by, "\n\r", 2 );
     }

     /*  Zeran - Now, check against disabled commands.  This should be done
      *			after logging so that disabled commands still register
      *			for tracking repeat hackers/offenders.
      */
     {
          struct disable_cmd_type *tmp;

          for ( tmp = disable_cmd_list; tmp != NULL; tmp = tmp->next )
          {
               if ( ( !str_prefix ( command, tmp->name ) ||
                      cmd_table[cmd].do_fun == tmp->disable_fcn ) && trust < tmp->level )
               {
                    send_to_char ( "This command has been disabled by the staff, sorry for the inconvenience.\n\r", ch );
                    return;
               }
          }
     }
     /* end checking for disabled command */

     if ( !found )
     {
          /*
           * Look for command in socials table.
           */
         if ( !check_social ( ch, command, argument ) && !I3_command_hook(ch, command, argument) )
             send_to_char ( "Type {YCOMMANDS{x for a list of valid commands.\n\r", ch );
          return;
     }

    /*
     * Character not in position for command?
     */

     if ( ch->position < cmd_table[cmd].position )
     {
          switch ( ch->position )
          {
          case POS_DEAD: 	send_to_char ( "Lie still; you are DEAD.\n\r", ch ); 		break;
          case POS_MORTAL:
               /* FALLTHROUGH */
          case POS_INCAP: 	send_to_char ( "You are hurt far too bad for that.\n\r", ch );	break;
          case POS_STUNNED: 	send_to_char ( "You are too stunned to do that.\n\r", ch ); 	break;
          case POS_SLEEPING: 	send_to_char ( "In your dreams, or what?\n\r", ch ); 		break;
          case POS_RESTING: 	send_to_char ( "Nah... You feel too relaxed...\n\r", ch );	break;
          case POS_SITTING: 	send_to_char ( "Better stand up first.\n\r", ch );		break;
          case POS_FIGHTING: 	send_to_char ( "No way!  You are still fighting!\n\r", ch ); 	break;
          }
          return;
     }

     /*
      * No hiding.
      */

     if ( cmd_table[cmd].position > POS_SLEEPING )
          REMOVE_BIT ( ch->affected_by, AFF_HIDE );

     /*
      * Dispatch the command.
      */

     ( *cmd_table[cmd].do_fun ) ( ch, argument );
     tail_chain (  );
     return;
}

bool check_social ( CHAR_DATA * ch, char *command, char *argument )
{
     char                arg[MAX_INPUT_LENGTH];
     CHAR_DATA          *victim;
     int                 cmd;
     bool                found;

     found = FALSE;

     for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
     {
          if ( command[0] == social_table[cmd].name[0] && !str_prefix ( command, social_table[cmd].name ) )
          {
               found = TRUE;
               break;
          }
     }

     if ( !found )
          return FALSE;

     if ( !IS_NPC ( ch ) && IS_SET ( ch->comm, COMM_NOEMOTE ) )
     {
          send_to_char ( "You are anti-social!\n\r", ch );
          return TRUE;
     }

     switch ( ch->position )
     {
     case POS_DEAD:
          send_to_char ( "Lie still; you are DEAD.\n\r", ch );
          return TRUE;
     case POS_INCAP:
     case POS_MORTAL:
          send_to_char ( "You are hurt far too bad for that.\n\r", ch );
          return TRUE;
     case POS_STUNNED:
          send_to_char ( "You are too stunned to do that.\n\r", ch );
          return TRUE;
     case POS_SLEEPING:
          /*
           * I just know this is the path to a 12" 'if' statement.  :(
           * But two players asked for it already!  -- Furey
           * Nearly 10 years later it is still just 1, but if more are
           * needed then just change the social_table.... -- Lotherius
           */
          if ( !str_cmp ( social_table[cmd].name, "snore" ) )
               break;
          send_to_char ( "In your dreams, or what?\n\r", ch );
          return TRUE;
     }

     one_argument ( argument, arg );

     victim = NULL;

     if ( arg[0] == '\0' )
     {
          act ( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM );
          act ( social_table[cmd].char_no_arg, ch, NULL, victim, TO_CHAR );
     }
     else if ( ( victim = get_char_room ( ch, NULL, arg ) ) == NULL )
     {
          send_to_char ( "They aren't here.\n\r", ch );
     }
     else if ( victim == ch )
     {
          act ( social_table[cmd].others_auto, ch, NULL, victim, TO_ROOM );
          act ( social_table[cmd].char_auto, ch, NULL, victim, TO_CHAR );
     }
     else
     {
          act ( social_table[cmd].others_found, ch, NULL, victim, TO_NOTVICT );
          act ( social_table[cmd].char_found, ch, NULL, victim, TO_CHAR );
          act ( social_table[cmd].vict_found, ch, NULL, victim, TO_VICT );

          /* Strange way of doing random a random number.... */
          if ( !IS_NPC ( ch ) && IS_NPC ( victim ) && !IS_AFFECTED ( victim, AFF_CHARM )
               && IS_AWAKE ( victim ) && victim->desc == NULL )
          {
               switch ( number_bits ( 4 ) )
               {
               case 0: case 1: case 2: case 3:
               case 4: case 5: case 6: case 7:
               case 8:
                    act ( social_table[cmd].others_found, victim, NULL, ch, TO_NOTVICT );
                    act ( social_table[cmd].char_found, victim, NULL, ch, TO_CHAR );
                    act ( social_table[cmd].vict_found, victim, NULL, ch, TO_VICT );
                    break;
               case 9: case 10: case 11:
               case 12:
                    act ( "$n slaps $N.", victim, NULL, ch, TO_NOTVICT );
                    act ( "You slap $N.", victim, NULL, ch, TO_CHAR );
                    act ( "$n slaps you.", victim, NULL, ch, TO_VICT );
                    break;
               }
          }
     }
     return TRUE;
}

/*
 * Return true if an argument is completely numeric.
 */

bool is_number ( char *arg )
{

     if ( *arg == '\0' )
          return FALSE;

     if ( *arg == '+' || *arg == '-' )
          arg++;

     for ( ; *arg != '\0'; arg++ )
     {
          if ( !isdigit ( *arg ) )
               return FALSE;
     }
     return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 * Zeran - modify to return -1 for all.foo number
 */

int number_argument ( char *argument, char *arg )
{
     char               *pdot;
     int                 number;

     for ( pdot = argument; *pdot != '\0'; pdot++ )
     {
          if ( *pdot == '.' )
          {
               *pdot = '\0';
               if ( !str_cmp ( argument, "all" ) )
                    number = -1;
               else
                    number = atoi ( argument );
               *pdot = '.';
               strcpy ( arg, pdot + 1 ); // The math doesn't always work with the macro?
               return number;
          }
     }
     strcpy ( arg, argument );
     return 1;
}

/* Lotherius added for buying multiple items
 * Given a string like 14*foo, return 14 and 'foo'
 * argument is original string, arg will get name of the object
*/

int mult_argument ( char *argument, char *arg )
{
     char               *pdot;
     int                 number;

     for ( pdot = argument; *pdot != '\0'; pdot++ )
     {
          if ( *pdot == '*' )
          {
               *pdot = '\0';
               number = atoi ( argument );
               *pdot = '*';
               strcpy ( argument, pdot + 1 ); // The math doesn't always work with the strcpy macro?
               return number;
          }
     }
     return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */

char *one_argument ( char *argument, char *arg_first )
{
     char                cEnd;

     while ( isspace ( *argument ) )
          argument++;

     cEnd = ' ';

     if ( *argument == '\'' || *argument == '"' )
          cEnd = *argument++;

     while ( *argument != '\0' )
     {
          if ( *argument == cEnd )
          {
               argument++;
               break;
          }
          *arg_first = LOWER ( *argument );
          arg_first++;
          argument++;
     }
     *arg_first = '\0';

     while ( isspace ( *argument ) )
          argument++;

     return argument;
}

/* Zeran - lazy me, want a non-lower case one_argument             */
/* This is duplicated somewhere in strings.c I think  -- Lotherius */
char *one_argument_nl ( char *argument, char *arg_first )
{
     char                cEnd;

     while ( isspace ( *argument ) )
          argument++;

     cEnd = ' ';
     if ( *argument == '\'' || *argument == '"' )
          cEnd = *argument++;

     while ( *argument != '\0' )
     {
          if ( *argument == cEnd )
          {
               argument++;
               break;
          }
          *arg_first = ( *argument );
          arg_first++;
          argument++;
     }
     *arg_first = '\0';

     while ( isspace ( *argument ) )
          argument++;

     return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands ( CHAR_DATA * ch, char *argument )
{
     BUFFER		*buffer;
     int                 cmd;
     bool		 search = FALSE;
     bool		 match = FALSE;
     int		 index = 0;
     

     /*
      * We're going to take any argument. Like some people I know. -- Lotherius
      */
     
     /* 
      * A numeric argument is an index, and textual argument is a search.
      */
     
     if ( argument[0] == '\0' )
     {
          send_to_char ( "There are too many commands to show them all, so you have two\n\r"
                         "search options.\n\r\n\r"
                         "A: {WKeyword Search{w - Just enter a keyword after commands (ie, commands foo)\n\r"
                         "B: {WIndexed List{w - Enter a number for the index you wish to see from the following:\n\r"
                         "                  {C1{w - Movement Commands          {C10{w - Clan Commands\n\r"
                         "                  {C2{w - Info about Yourself        {C11{w - Useful for Newbies\n\r"
                         "                  {C3{w - Info about Others          {C12{w - Leasing Commands\n\r"
                         "                  {C4{w - Object Related             {C13{w - Commands used in COMBAT\n\r"
                         "                  {C5{w - Room & World infos         {C14{w - Those related to MAGIC\n\r"
                         "                  {C6{w - Skills and Skill Info      {C15{w - Other Actions\n\r"
                         "                  {C7{w - Configuration              {C16{w - Misc (Didn't Fit Elsewhere)\n\r"
                         "                  {C8{w - Helpful Commands           {C17{w - Game Information\n\r"
                         "                  {C9{w - Communication              {C18{w - Shortcut Commands\n\r", ch );
          if ( IS_IMMORTAL ( ch ) )
               send_to_char ( "                 {C19{w - OLC Commands               {C20{w - Immortal Commands\n\r", ch );

          return;
     }
     
     //if ( argument[0] != '\0' )
     if ( !is_number ( argument ) )
          search = TRUE;
     else
     {
          int i;
          
          i = atoi ( argument );
          
          if ( !IS_IMMORTAL ( ch ) )
          {
               if ( !ENTRE(0,i,19) )
               {
                    send_to_char ( "Valid indices are 1 to 18. (Type commands by itself for more help.)\n\r", ch );
                    return;
               }
          }
          else
          {
               if ( !ENTRE(0,i,21) )
               {
                    send_to_char ( "Valid indices are 1 to 20. (Type commands by itself for more help.)\n\r", ch );
                    return;
               }
          }          
                    
          // Oookay... we got a live one... big switch statement.
          switch ( i )
          {
          case 1:
               index = CMD_MOVE;
               break;
          case 2:
               index = CMD_SELFINFO;
               break;
          case 3:
               index = CMD_OTHERINFO;
               break;
          case 4:
               index = CMD_OBJECTS;
               break;
          case 5:
               index = CMD_WORLDINFO;
               break;
          case 6:
               index = CMD_SKILL;
               break;
          case 7:
               index = CMD_CONFIG;
               break;
          case 8:
               index = CMD_HELPFUL;
               break;
          case 9:
               index = CMD_COMM;
               break;
          case 10:
               index = CMD_CLAN;
               break;
          case 11:
               index = CMD_NEWBIE;
               break;
          case 12:
               index = CMD_LEASE;
               break;
          case 13:
               index = CMD_COMBAT;
               break;
          case 14:
               index = CMD_MAGIC;
               break;
          case 15:
               index = CMD_ACTION;
               break;
          case 16:
               index = CMD_MISC;
               break;
          case 17:
               index = CMD_GAMEINFO;
               break;
          case 18:
               index = CMD_SC;
               break;
          case 19:
               index = CMD_OLC;
               break;
          case 20:
               index = CMD_IMM;
               break;
          default:
               bugf ( "Invalid Index in do_commands." );
               break;               
          }  // end of i switch
     }
     
     buffer = buffer_new(1024);     
     
     for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
     {
          // extra braces to avoid ambiguity
          if (search)
          {
               if (str_prefix (argument, cmd_table[cmd].name)
                   && str_infix (argument, cmd_table[cmd].helpmsg ) )
                    continue;
          }
          else if ( index > 0 )
          {
               if ( !IS_SET ( cmd_table[cmd].category, index ) )
                    continue;
          }          
          if ( cmd_table[cmd].level <= get_trust ( ch ) && cmd_table[cmd].show )
          {
               if ( cmd_table[cmd].level <= LEVEL_HERO )
                    bprintf ( buffer , "{W[{G%-12s{W] {Y: {w%s\n\r", cmd_table[cmd].name, cmd_table[cmd].helpmsg );
               else
                    bprintf ( buffer , "{W[{R%-12s{W] {Y: {w({C%d{w) %s\n\r", cmd_table[cmd].name, cmd_table[cmd].level,
                              cmd_table[cmd].helpmsg );
               match = TRUE;
          }
     }
     if ( search && !match )
          send_to_char ( "Nothing found. Perhaps you should try {Ghlist search{w.\n\r", ch );
     else
          page_to_char ( buffer->data, ch );
     buffer_free ( buffer );
     return;
}

// This is now just a wrapper - Lotherius
void do_wizhelp ( CHAR_DATA * ch, char *argument )
{
     do_commands ( ch, "20" );
}

/* Zeran - these functions should go in act_wiz.c, but putting them here so
 *	   don't have to declare the cmd_table globally.
 */

void do_disable ( CHAR_DATA * ch, char *argument )
{
     struct disable_cmd_type *tmp, *last_disabled = NULL;
     char                     command[MAX_INPUT_LENGTH];
     char                    *level_string;
     int                      level;
     int                      trust;
     int                      cmd;
     bool                     found = FALSE;

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Disabled commands\n\r", ch );
          send_to_char ( "-------- --------\n\r", ch );
          for ( tmp = disable_cmd_list; tmp != NULL; tmp = tmp->next )
          {
               form_to_char ( ch, "[ {B%-12s{x ] at level [ {B%3d{x ]\n\r", tmp->name, tmp->level );
               found = TRUE;
          }
          if ( !found )
               send_to_char ( "No commands are disabled at this time.\n\r", ch );
          return;
     }
     else
     {
          trust = get_trust ( ch );
          level_string = one_argument ( argument, command );

          if ( level_string == NULL || level_string[0] == '\0' || atoi ( level_string ) == 0 )
               level = trust;
          else
               level = UMIN ( trust, atoi ( level_string ) );
          for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
          {
               if ( command[0] == cmd_table[cmd].name[0] && !str_prefix ( command, cmd_table[cmd].name )
                    && cmd_table[cmd].level <= trust )
               {
                    found = TRUE;
                    break;
               }
          }

          if ( !found )
          {
               send_to_char ( "No such command found, or command restricted to higher level.\n\r", ch );
               return;
          }

		/*check if command already disabled */

          for ( tmp = disable_cmd_list; tmp != NULL; tmp = tmp->next )
          {
               if ( !str_prefix ( command, tmp->name ) )
               {
                    form_to_char ( ch, "[ {B%s{x ] is already disabled for all characters below level [ {B%d{x ].\n\r",
                              tmp->name, tmp->level );
                    return;
               }
               last_disabled = tmp;
          }

		/* disable the command */

          tmp = alloc_mem ( sizeof ( struct disable_cmd_type ), "disable_cmd_type" );
          tmp->next = NULL;
          tmp->name = str_dup ( cmd_table[cmd].name );
          tmp->level = level;
          tmp->disable_fcn = cmd_table[cmd].do_fun;

          if ( disable_cmd_list == NULL )
               disable_cmd_list = tmp;
          else
               last_disabled->next = tmp;

          form_to_char ( ch, "You have disabled [ {B%s{x ] for characters below level [ {B%d{x ].\n\r",
                    tmp->name, tmp->level );
     }
     fwrite_disable (  );
     return;
}

void do_enable ( CHAR_DATA * ch, char *argument )
{
     struct disable_cmd_type *tmp, *last_disabled = NULL;
     char                command[MAX_INPUT_LENGTH];
     bool                found = FALSE;

     if ( argument == NULL || argument[0] == '\0' )
     {
          send_to_char ( "Syntax:  enable <command>\n\r", ch );
          return;
     }

     one_argument ( argument, command );

    /* find command in disable list */
     for ( tmp = disable_cmd_list; tmp != NULL; tmp = tmp->next )
     {
          if ( !str_prefix ( command, tmp->name ) )
          {
               found = TRUE;
               break;
          }
          last_disabled = tmp;
     }

     if ( !found )
     {
          send_to_char ( "That command is not currently disabled...\n\r", ch );
          return;
     }

     /* remove command from disabled list */
     form_to_char ( ch, "[ {B%s{x ] disabling removed.\n\r", tmp->name );

     free_string ( tmp->name );
     if ( tmp == disable_cmd_list )
          disable_cmd_list = tmp->next;
     else
          last_disabled->next = tmp->next;
     free_mem ( tmp, sizeof ( struct  disable_cmd_type ), "disable_cmd_type" );
     fwrite_disable (  );
     return;
}

/* Zeran - This function has a horribly inelegant method of watching for
 *	quoted arguments, but at least it works. *sigh*
 */
/* Hey zeran... you wrote this to not take any fscking aruments... - Loth */

bool expand_aliases ( CHAR_DATA * ch, char *orig_command, char *final_command )
{
     char                arg[MAX_INPUT_LENGTH];
     char               *remainder;
     char               *tmp_remainder;
     struct alias_data  *tmp;
     int                 counter;
     bool                match = FALSE;
     char                allargs[6][MAX_INPUT_LENGTH];
     int                 total_args;
     int                 tmp_count;
     int                 tmp_len;
     int                 value;
     char                single[3];

     single[0] = '\0';

     final_command[0] = '\0';

     /* check null command */
     if ( orig_command == NULL || orig_command[0] == '\0' || orig_command[0] == '\'' )
     {
          strcpy ( final_command, orig_command );
          return FALSE;
     }

     /*
      * parse orig_command first word, match against alias names, expand if
      * found and tack onto final_command, then return)
      */

     remainder = one_argument ( orig_command, arg );

     /* match arg against alias names */

     for ( counter = 0; counter < MAX_ALIAS; counter++ )
     {
          tmp = ch->pcdata->aliases[counter];
          if ( tmp == NULL )
               continue;
          if ( !strcmp ( tmp->name, arg ) )
          {
	    	/* check for parameters required for alias */
               total_args = 1;
               tmp_remainder = remainder;

               while ( tmp_remainder != NULL && tmp_remainder[0] != '\0' && total_args < 6 )
               {
                    tmp_remainder = one_argument ( tmp_remainder, allargs[total_args] );
                    total_args++;
               }

               tmp_remainder = tmp->command_string;
               tmp_len = strlen ( tmp->command_string );

               for ( tmp_count = 0; tmp_count < ( tmp_len ); tmp_count++ )
               {
                    if ( tmp_remainder[tmp_count] == '%' && tmp_remainder[tmp_count + 1] != '\0'
                         && ( tmp_remainder[tmp_count + 2] == ' '
                              || tmp_remainder[tmp_count + 2] == '\0' ) )
                    {
                         value = ( tmp_remainder[tmp_count + 1] - '0' );
                         if ( value < 1 || value > MAX_ALIAS_PARMS )
                              value = -1;
                         
                         if ( value != -1 )
                         {
                              if ( value < total_args )
                                   strcat ( final_command, allargs[value] );
                              else
                                   strcat ( final_command, " " );
                              tmp_count++;
                         }
                         else
                         {
                              sprintf ( single, "%c", tmp_remainder[tmp_count + 1] ); // Okay here cuz 1 char
                              strcat ( final_command, "%" );
                              strcat ( final_command, single );
                              tmp_count++;
                         }
                    }
                    else
                    {
                         sprintf ( single, "%c", tmp_remainder[tmp_count] ); // Okay here cuz 1 char
                         strcat ( final_command, single );
                    }
               }
               /* end for loop through tmp->command_string */
               // hey z, why were you appending a space t the end? It was screwing with
               // things - Loth.
               //strcat ( final_command, " " );
               match = TRUE;
          }
     }
     /* End of a HORRIBLE amount of bracketing. Geez Zeran.... this is clunky. */

     if ( !match )
     {
          strcpy ( final_command, orig_command );
          return FALSE;
     }
     return TRUE;
}

/* Zeran - procedure to scan for multiple commands */
void check_multi_cmd ( CHAR_DATA * ch, char *orig_cmd, char *final_cmd )
{
     int                 count;
     int                 len;
     char               *tmp_ptr = NULL;
     bool                first_s_quote = FALSE;
     bool                first_d_quote = FALSE;
     char               *tmp_incomm = NULL;
     bool                need_tmp_incomm = FALSE;

     len = strlen ( orig_cmd );
    /* are we parsing an alias, or just parsing incomm? */
     if ( orig_cmd != ch->desc->incomm )
          need_tmp_incomm = TRUE;
     for ( count = 0; count < len; count++ )
     {
          switch ( orig_cmd[count] )
          {
          case '"':
               {
                    if ( !first_s_quote )
                    {
                         if ( !first_d_quote )
                              first_d_quote = TRUE;
                         else
                              first_d_quote = FALSE;
                    }
                    break;
               }
          case '\'':		/* skip if count is 0...its the short say command */
               {
                    if ( count == 0 )
                         break;
                    if ( !first_d_quote )
                    {
                         if ( !first_s_quote )
                              first_s_quote = TRUE;
                         else
                              first_s_quote = FALSE;
                    }
                    break;
               }
          case '|':
               {
                    if ( !tmp_ptr && !first_d_quote && !first_s_quote )
                         tmp_ptr = &( orig_cmd[count] );
                    break;
               }
          default:
               break;
          }
          /* end switch */
          /* if got a separator pointer, break */
          if ( tmp_ptr )
               break;
     }
     /* end for loop */
     if ( tmp_ptr != NULL && !first_s_quote && !first_d_quote )
     {
          ch->desc->multi_comm = TRUE;

          /* copy ch->desc->incomm if needed */
          if ( need_tmp_incomm )
               tmp_incomm = str_dup ( ch->desc->incomm );
          *tmp_ptr = '\0';
          tmp_ptr++;
          while ( isspace ( *tmp_ptr ) )
               tmp_ptr++;
          strcpy ( final_cmd, orig_cmd );
          strcpy ( ch->desc->incomm, tmp_ptr ); // strcpy with a pointer
          if ( need_tmp_incomm )
          {
               if ( ( strlen ( ch->desc->incomm ) + strlen ( ( tmp_ptr + 1 ) ) ) >= ( MAX_INPUT_LENGTH - 10 ) )
               {
                    send_to_char ( "Command expansion too large, ignoring last command.\n\r", ch );
                    ch->desc->incomm[0] = '\0';
                    final_cmd[0] = '\0';
                    return;
               }
               strcat ( ch->desc->incomm, "|" );
               strcat ( ch->desc->incomm, tmp_incomm );
          }

          free_string ( tmp_incomm );
          return;
     }
     else
     {
          strcpy ( final_cmd, orig_cmd );
          if ( orig_cmd == ch->desc->incomm )
               ch->desc->incomm[0] = '\0';
     }
     return;
}

