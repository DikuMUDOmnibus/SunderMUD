/**********************************************************
 *************** S U N D E R M U D *** 2 . 0 **************
 **********************************************************
 * SunderMud 2.0 is the result of the efforts of many     *
 * people over a number of years, not including solely the*
 * work on SunderMud itself, but also that done by those  *
 * whose work preceded SunderMud.                         *
 **********************************************************
 * This code can be used under the terms of the DikuMud,  *
 * Merc, and Rom 2.3 license. I do not add any additional *
 * requirements.                                          *
 **********************************************************
 * The unique portions of the SunderMud code as well as   *
 * the integration efforts for code from other sources is *
 * based on the efforts of:                               *
 *                                                        *
 * Lotherius <elfren@blkbox.com> (Alvin W. Brinson)       *
 *           For: SunderMud 1.0 & 2.0                     *
 * Zeran     <unknown email>     (Jeremy Ehmke)           *
 *           For: Dark Times & a small portion of 2.0     *
 **********************************************************
 * Further credits can be found under "help sundermud",   *
 * "help rom", "help diku", and "help merc".              *
 **********************************************************/

#ifndef _MERCSTRING_H
# define _MERCSTRING_H   1

/* This file includes very commonly used text strings from around the mud.
 * Editing this file should easily change these strings.
 * DO remember some of these are best NOT colourized, primarily the first two.
 * Not all the mud's text is in here, but I suppose it COULD be to make
 * translation easier to do. However, right now this is just for convenience
 * in things that either need to be easily changed or are used in MANY places.
 */

/*
 * Some general mud-wide stuff
 * I put parsename and namerule here, because many admins will want to change them.
 */

# define TXT_IMAGEURL   "http://abrinson.home.texas.net/images/"		// Change this to your base image URL
# define TXT_SOUNDURL   "http://abrinson.home.texas.net/sounds/"        // Change this to your base sounds URL
# define TXT_MUDNAME	"SunderMud"       							    // Please do change this to your mud's name.
# define TXT_MUDVERSION	"Sunder 2.1" 									// Please don't change this unless you don't like to give credit.
# define TXT_COMPILE    __DATE__ " at " __TIME__						// No need to change this.
# define TXT_MUDEMAIL   "youremail@yourhost.com" 						// Please do change this to your own email address.
# define TXT_PARSENAME	"\n\r{WThat name is unacceptable. Acceptable names will:" \
     "\n\r{YA) Not be a word from a dictionary." \
          "\n\r{YB) Not be a modern name, or a name from a well-known book or movie." \
          "\n\r{YC) Not be considered vulgar or profane." \
          "\n\r{YD) Be between 3 and 12 characters in length, with no numerics." \
          "\n\r{YE) Not have been deemed otherwise inappropriate." \
          "\n\r\n\r{WPlease enter a new name: {w"
# define TXT_NAMERULE	"New names must not be well-known fictional names, or modern names.\n\r" \
     "Does your name fit this criteria, " // Playername follows this.
# define TXT_YESNO	    "{W({GY{W/{GN{W){w?"   // (Y/N)?
# define TXT_PLEASEYN    "{WPlease type {GY {Wor {GN{w only: "
# define TXT_CONTINUE	"{C={c===={C[ {WENTER Continues {C]{c===---{C-{w" // For the pager

/*
 * Chracter Creation Text. Put here because much of it is duplicated, and you might want to change it.
 */

# define TXT_BADEMAIL "Email addresses containing the words: {WHotmail{w, {WExcite{w, {WYahoo{w, or {Wroot{w\n\r" \
	                 "are not accepted, neither are those that do not appear to be real.\n\r" \
                     "If you feel you are {Gspecial{w and deserve an exception, contact the admins at "
# define TXT_RACE    "\n\rAs you begin to come to grips with {creality{w, that you, indeed, do exist, a\n\r" \
	                "presence appears before you, embracing you in warmth, and you realize that now is the\n\r" \
		            "time of your birth. You will become a part of the world.\n\r" \
		            "\n\rThe presence whispers to you of {craces{w and you realize that you must now choose\n\r" \
		            "to which race you will owe your alliegence.\n\r"
# define TXT_NODEMI  "\n\r  You awake from a deep sleep. You realize that as a new spirit in the realm\n\r" \
	                "you may only create {cmortal{w characters. A voice whispers to you of Demi-Gods, but\n\r" \
		            "you know you must prove yourself worthy by becoming proving yourself worthy with HEROES,\n\r" \
		            "and most importantly, you must {Rverify{w your email address after you login.\n\r\n\r\n\r"
# define TXT_YESDEMI "\n\r  You awake from a deep sleep. You've been here before, and the voice that whispers to you\n\r" \
                    "is here too. Legends of your heroic deeds have reached the heavens and you feel a strong sense\n\r" \
	                "of power as you realize that you have the blessings of the Gods if you wish to be incarnated\n\r" \
	                "as a {GDemi{w-{GGod{w. This is an important decision, Hero.\n\r"
# define TXT_MORD    "\n\rAre you a {W({GM{W){gortal{w or a {W({GD{W){gemi{w-{gGod{w: "
# define TXT_RCFROM  "\n\rYou may choose from the following {Craces{w:\n\r  " // Race list header
# define TXT_RCTEMP  "{W[{G%s{W] "
# define TXT_RCPRMP  "{w\n\rTo find out more about a race, use help <{Crace{w>\n\r" \
	                "What {Crace{w do you wish to be: "  // Race selection Prompt.
# define TXT_CLASS   "\n\rTo find out more about a class use help <{Cclass{w>\n\r" \
	                "Which guild did you study as a youth: "
# define TXT_TRAIN   "\n\rYou have %d Points to spend.\n\r"
# define TXT_CURSTAT "\n\r{wYour Current Stats Are:\n\r{WSTR: {G%d\n\r{WINT: {G%d\n\r{WWIS: {G%d\n\r{WDEX: {G%d\n\r{WCON: {G%d{w\n\r"
# define TXT_MAXSTAT "\n\r{wYour Racial Maximums are:\n\r{WSTR: {R%d\n\r{WINT: {R%d\n\r{WWIS: {R%d\n\r{WDEX: {R%d\n\r{WCON:{R %d{w\n\r"
# define TXT_STATPRM "\n\r{W[{CS{W]{wtr {W[{CI{W]{wnt {W[{CW{W]{wis {W[{CD{W]{wex {W[{CC{W]{won {W[{CR{W]{weset {W[{CF{W]{winished :: "
# define TXT_STATNOP "\n\r{RYou have spent all of your points.{w\n\rYou may {W[{CR{W]{weset to start over.\n\r"
# define TXT_STATMAX "\n\r{YThis stat may not be increased farther.{w\n\r"

/*
 * Condition text, put here for easy changing.
 */

# define TXT_COND_A	" is in {Gexcellent condition{x."
# define TXT_COND_B	" has a {Yfew scratches{x."
# define TXT_COND_C	" has some {ysmall wounds and bruises{x."
# define TXT_COND_D	" has {Wquite a few wounds{x."
# define TXT_COND_E	" has some {Mbig nasty wounds and scratches{x."
# define TXT_COND_F	" looks {rpretty hurt{x."
# define TXT_COND_G	" is in {Rawful condition{r."
# define TXT_COND_H	" is {Rbleeding to death{x."

#endif // _MERCSTRING_H

