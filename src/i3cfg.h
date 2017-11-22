/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code
 * Copyright (c)2001-2003 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 *
 * Contains codebase specific defines to make the rest of it all work - hopefully.
 * Anything your codebase needs to alter is more than likely going to be stored in here.
 * This should be the only file you need to edit to solve unforseen compiling problems
 * if I've done this properly. And remember, this is all based on what these defines mean
 * in your STOCK code. If you've made adjustments to any of it, then you'll need to adjust
 * them here too.
 */

#if defined(I3SMAUG) || defined(I3CHRONICLES)
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOCIALTYPE
   #define I3MAXPLAYERS sysdata.maxplayers
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((ch)->name)
   #define CH_I3TITLE(ch)     ((ch)->pcdata->title)
   #define CH_I3RANK(ch)      ((ch)->pcdata->rank)
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (xIS_SET((ch)->act, PLR_WIZINVIS) && (ch)->pcdata->wizinvis >= this_mud->minlevel )
#endif

#if defined(I3ROM) || defined(I3SUNDER)
   #define first_descriptor descriptor_list
#ifdef I3ROM
   #define I3MAXPLAYERS -1 /* Rom evidently does not have this available */
#else
   #define I3MAXPLAYERS       max_on /* But sunder does */
#endif
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((ch)->name)
   #define CH_I3TITLE(ch)     ((ch)->pcdata->title)
#ifdef I3ROM
   #define CH_I3RANK(ch)      (title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
#else
   #define CH_I3RANK(ch)      (class_table[(ch)->pcdata->pclass].who_name)
#endif
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (IS_IMMORTAL((ch)) && (ch)->invis_level > 0)
#endif

#ifdef I31STMUD
   #define first_descriptor descriptor_first
   #define write_to_buffer d_print
   #define SMAUGSOCIAL
   #define I3MAXPLAYERS       mud_info.stats.online
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((char*)(ch)->name)
   #define CH_I3TITLE(ch)     ((char*)(ch)->pcdata->title)
   #define CH_I3RANK(ch)      ((char*)(is_clan(ch) ? ch->clan->rank[ch->rank].rankname : class_table[prime_class(ch)].name))
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (IS_IMMORTAL((ch)) && (ch)->invis_level > 0)
#endif

#ifdef I3MERC
   #define first_descriptor descriptor_list
   #define I3MAXPLAYERS -1 /* Merc doesn't track this */
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((ch)->name)
   #define CH_I3TITLE(ch)     ((ch)->pcdata->title)
   #define CH_I3RANK(ch)      (title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (IS_IMMORTAL((ch)) && IS_SET((ch)->act, PLR_WIZINVIS))
#endif

#ifdef I3UENVY
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOC_INDEX_DATA
   SOC_INDEX_DATA *find_social( char *command );
   #define first_descriptor descriptor_list
   #define I3MAXPLAYERS sysdata.max_players
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((ch)->name)
   #define CH_I3TITLE(ch)     ((ch)->pcdata->title)
   #define CH_I3RANK(ch)      (title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (IS_IMMORTAL((ch)) && IS_SET((ch)->act, PLR_WIZINVIS))
#endif

#ifdef I3ACK
   extern int max_players;
   #define first_descriptor first_desc
   #define I3MAXPLAYERS max_players
   #define CH_I3DATA(ch)      ((ch)->pcdata->i3chardata)
   #define CH_I3LEVEL(ch)     ((ch)->level)
   #define CH_I3NAME(ch)      ((ch)->name)
   #define CH_I3TITLE(ch)     ((ch)->pcdata->title)
   #define CH_I3RANK(ch)      (class_table[(ch)->class].who_name)
   #define CH_I3SEX(ch)       ((ch)->sex)
   #define I3WIZINVIS(ch)     (IS_IMMORTAL((ch)) && (ch)->invis > 0)
#endif

#ifdef I3CIRCLE
//   #if _CIRCLEMUD < CIRCLEMUD_VERSION(3, 0, 21)
//   #  error "Requires CircleMUD 3.0 bpl21+ (varargs output functions)"
//   #endif

   extern const char *class_abbrevs[];
   extern int max_players;

   const char *title_female(int chclass, int level);
   const char *title_male(int chclass, int level);
   void smash_tilde(char *str);

   #define first_descriptor         descriptor_list
   #define I3MAXPLAYERS             max_players	/* comm.c */
   #define log_string               basic_mud_log
   #define bug                      basic_mud_log
   #define URANGE(a, b, c)          ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
   #define write_to_buffer(d, txt, n)	write_to_output((d), (txt))
   #define CH_I3LEVEL(ch)           GET_LEVEL((ch))
   #define CH_I3NAME(ch)            GET_NAME((ch))
   #define CH_I3TITLE(ch)           GET_TITLE((ch))
   #define CH_I3RANK(ch)            (GET_SEX((ch)) == SEX_FEMALE ? title_female(GET_CLASS((ch)), GET_LEVEL((ch)))	: title_male(GET_CLASS((ch)), GET_LEVEL((ch))))
   #define CH_I3SEX(ch)             GET_SEX((ch))
   #define CH_I3DATA(ch)            ((ch)->player_specials->i3chardata)
   #define I3WIZINVIS(ch)           (GET_INVIS_LEV((ch)) >= this_mud->minlevel)
#endif
