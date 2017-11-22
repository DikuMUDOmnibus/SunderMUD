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

/*
 * This file contains skill & spell GSN's
 */

#ifndef _MERCGSN_H
# define _MERCGSN_H   1

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int  gsn_backstab;
extern sh_int  gsn_dodge;
extern sh_int  gsn_circle;
extern sh_int  gsn_envenom;
extern sh_int  gsn_hide;
extern sh_int  gsn_peek;
extern sh_int  gsn_pick_lock;
extern sh_int  gsn_recruit;
extern sh_int  gsn_sneak;
extern sh_int  gsn_steal;
extern sh_int  gsn_disarm;
extern sh_int  gsn_enhanced_damage;
extern sh_int  gsn_ultra_damage;
extern sh_int  gsn_kick;
extern sh_int  gsn_parry;
extern sh_int  gsn_rescue;
extern sh_int  gsn_rotate;
extern sh_int  gsn_second_attack;
extern sh_int  gsn_sharpen;             
extern sh_int  gsn_third_attack;
extern sh_int  gsn_fourth_attack; 
extern sh_int  gsn_fifth_attack;  
extern sh_int  gsn_blindness;
extern sh_int  gsn_charm_person;
extern sh_int  gsn_curse;
extern sh_int  gsn_invis;
extern sh_int  gsn_mass_invis;
extern sh_int  gsn_plague;
extern sh_int  gsn_poison;
extern sh_int  gsn_sleep;
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_spear;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;
extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_rally;
extern sh_int  gsn_dual;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;
extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_meditation;
extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
extern sh_int  gsn_recall;

/* Language GSN's */
extern sh_int  gsn_lang_human;
extern sh_int  gsn_lang_dwarf;
extern sh_int  gsn_lang_elf;
extern sh_int  gsn_lang_giant;
extern sh_int  gsn_lang_gargoyle;
extern sh_int  gsn_lang_kobold;
extern sh_int  gsn_lang_centaur;
extern sh_int  gsn_lang_azer;
extern sh_int  gsn_lang_kender;
extern sh_int  gsn_lang_common;

#endif /* MERCGSN_H */
