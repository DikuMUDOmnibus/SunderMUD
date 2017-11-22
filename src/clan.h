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

#ifndef _CLAN_H
# define _CLAN_H   1

# define MAX_CLANAREAS   5
# define MAX_CLANMEMBERS 50

# define RANK_LEADER	     	5
# define RANK_TWO	     	4
# define RANK_THREE	     	3
# define RANK_FOUR	     	2
# define RANK_FIVE	     	1
# define RANK_BOTTOM      	0

# define CLAN_NEW         	0   // Clan is new and unrecognized.
# define CLAN_PROBATION   	1   // Clan needs to clean up its shit or begone.
# define CLAN_UNDERGROUND 	2   // Clan is "underground" and does not want/ cannot have, recognition
# define CLAN_RECOGNIZED  	3   // Clan is recognized by IMMs. Anything this high or higher will show on clist
# define CLAN_RENEW       	4   // Something needs checked on the clan.

struct clan_area_type
{
     struct clan_area_type     *next;  	/* Next Area */
     AREA_DATA 		       *area;		/* The area controlled */
     int			areatax;	/* Tax on the area */
     long			totaltax;	/* Tax collected */
     int			guards;		/* Number of guards in this area */
     sh_int		    	yeartaken;	/* Year area taken */
     sh_int		    	monthtaken;	/* Month area taken */
     sh_int	        	daytaken;	/* Day area taken */
};

struct clan_member_type
{
     struct clan_member_type   *next;		/* Next member */
     char 		       *member_name;	/* Name duh */
     sh_int			year_joined;	/* Dates */
     sh_int			month_joined;
     sh_int			day_joined;
     long			taxpaid;
     sh_int			clanrank;
};

struct clan_main_type
{
     struct clan_main_type   *next;

     // To increase the possible amount of wars, simply add more pointers here,
     // and update the requisite code in clan.c
     // I didn't use a MAX_ type counter for simplicity elsewhere. If you add
     // more, it may work better to use a struct here
     //
     struct clan_main_type   *war_a;  /* At war with X clan */
     struct clan_main_type   *war_b;  /* At war with Y clan */

     ROOM_INDEX_DATA         *hq;    /* Pointer to Clan's HQ */

	 /*  struct clan_member_type *clan_members[MAX_CLANMEMBERS];
	  struct clan_area_type   *clanareas[MAX_CLANAREAS]; */

     char      *clan_name;		/* The Clan "LongName" */
     char      *clan_short;		/* The Clan "ShortName" Keyword */
     char      *mranks[6];		/* 6 male ranks */
     char      *franks[6];		/* 6 female ranks */
     int  	portalto;	    	/* VNum where the clan portal goes, vnum of HQ! */
     bool 	nosave;			/* If true, clan will not save for next boot. */
     sh_int 	status;     		/* Clan Status */
     int  	clanpk;			/* Number of times the clan has won PKill */
     int  	clandie;		/* Number of times the clan has lost PKill */
     long 	clanbank;	    	/* Clan Bank Account */
     int  	clanmtax;	    	/* Tax on clan's members */
     int  	membercount;		/* How many members now? */
     char      *reason;      		/* Reason an imm has done something to the clan. Usually involving non-recognition. */
     bool 	pkallow;      		/* Does this clan even ALLOW PKill -- if not, can't have a stronghold, declare or set bounties */
     bool 	autoaccept;   		/* Accept anyone who wants to join this clan automatically? */
     int  	moveyear;     		/* Year the clan stronghold was moved. Can't be moved again for 3 years, unless forced */
     char      *demigod;     		/* The demigod to whom this clan owes alliegence */
     bool 	demiapprove;  		/* Does the Demi-God approve of the clan */
     int  	experience;   		/* Clan experience -- Gained through various activities */
     int  	join_level;   		/* Minimum level to join the clan */
     int  	join_cost;    		/* Cost in Gold to join the clan */
     int  	join_minalign; 		/* Minimum alignment to join the clan */
     int  	join_maxalign; 		/* Maximum alignment to join the clan */
	 /* Set the ranks */
     int  	rank_setjoin;  		/* Lowest rank that can set the joining level & cost */
     int  	rank_recruit; 		/* Lowest rank that can recruit	someone */
     int  	rank_outcast; 		/* Lowest rank that can outcast someone lower than them */
     int  	rank_promote; 		/* Lowest rank that can promote a member */
     int  	rank_demote;  		/* Lowest rank that can demote a member */
     int  	rank_settax;  		/* Lowest rank that can set a tax. */
     int  	rank_declare; 		/* Lowest rank that can declare Wars against other clans or Holy Wars */
     int  	rank_claim;   		/* Lowest rank that can claim an area in the name of the clan */
     int 	rank_bounty;  		/* Lowest rank that can set bounties. */
     int 	rank_recall;  		/* Lowest rank that can recall to the clan stronghold */
     int  	rank_move;    		/* Lowest rank that can move the clan stronghold */
};

typedef struct clan_main_type	CLAN_INDEX;
extern  struct clan_main_type	* clan_list;
void    handle_edit_clan args ( ( DESCRIPTOR_DATA *d, char *argument ) );

#endif // _CLAN_H
