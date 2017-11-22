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


#ifndef _INTERP_H
#define _INTERP_H   1

/* this is a listing of all the commands and command related data */

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* angel */
#define HE	LEVEL_HERO	/* hero */

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_armor        );
DECLARE_DO_FUN( do_stop         ); /* Lotherius - sound code */
DECLARE_DO_FUN( do_beep         ); /* Lotherius */
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN(	do_award	);
DECLARE_DO_FUN( do_affect       );
DECLARE_DO_FUN( do_afk          ); /* away from keyboard */
DECLARE_DO_FUN( do_alias        );
DECLARE_DO_FUN( do_unalias      );
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_answer	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_autotitle    );
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosave	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_rally	); /* Lotherius */
DECLARE_DO_FUN( do_board	); /* Lotherius */
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN( do_fullfight	); /* Lotherius */
DECLARE_DO_FUN( do_dual		); /* Zeran */
DECLARE_DO_FUN( do_brew		); /* Zeran */
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_clist	); /* Clan list */
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_colour       ); /* Colour Command By Lope */
DECLARE_DO_FUN( do_cursor	); /* cursor control by Lotherius */
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_combine	);
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_copy		);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN( make_clan       ); /* Lotherius */
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN( do_disable	); /* Command disabling/enabling */
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_email        ); /* Lotherius */
DECLARE_DO_FUN( do_enable       ); /* Command disabling/enabling */
DECLARE_DO_FUN(	do_end	        ); /* Zeran - added to reset editor to get out of OLC*/ /* He forgot about done */
DECLARE_DO_FUN( do_enter        ); /* Zeran - added for portals */
DECLARE_DO_FUN( do_circle       ); /* Lotherius */
DECLARE_DO_FUN( do_envenom      ); /* Lotherius */
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_gossip	);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN( do_hlist        ); /* Lotherius */
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_home	        ); /* Lotherius */
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN(	do_imptalk	); /* Zeran - for the highest level imms */
DECLARE_DO_FUN( do_immtitle	); /* Lotherius */
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN( do_cloak	); /* Zeran */
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_language	); /* Zeran & Lotherius */
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_lore		); /* Lotherius */
DECLARE_DO_FUN( do_learn	); /* Zeran */
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN(	do_pwhere	);
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN( do_music	);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_noshout	);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN(	do_notes	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_notify       ); /* Zeran */
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_question	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN( do_lease	); /* Lotherius */
DECLARE_DO_FUN( do_checklease	); /* Lotherius */
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_replay	); /* Zeran */
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_resize	); /* Zeran */
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_rotate	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN( do_cset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scribe	); /* Zeran */
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN( do_sound	); /* Lotherius MSP support */
DECLARE_DO_FUN( do_search	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN( do_sharpen      ); /* Lotherius */
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN(	do_splitc	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_story	);
DECLARE_DO_FUN( do_string	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_unread	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wiznet	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN( do_world	); /* Informational */
DECLARE_DO_FUN( do_listskills   ); /* Lotherius */
DECLARE_DO_FUN( do_listraces	); /* Lotherius */
DECLARE_DO_FUN( do_setrent      ); /* Lotherius */
DECLARE_DO_FUN( do_private      ); /* Lotherius */
DECLARE_DO_FUN( do_quest        ); /* Quest Code */
DECLARE_DO_FUN( do_repop        ); /* Zeran */ 
DECLARE_DO_FUN( do_roomname	); /* Lotherius */
DECLARE_DO_FUN( do_roomdesc	); /* Lotherius */
DECLARE_DO_FUN( do_statall	); /* Lotherius */
DECLARE_DO_FUN( do_crier	); /* Lotherius*/
DECLARE_DO_FUN( do_xinfo	); /* informational */
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_config       ); /* Lotherius */
DECLARE_DO_FUN( do_mob		); /* MobProgs */
DECLARE_DO_FUN( do_mpstat	); /* MProgs */
DECLARE_DO_FUN( do_mpdump	); /* MProgs */
DECLARE_DO_FUN(	do_surrender	);
DECLARE_DO_FUN( do_mpedit 	); /* MProgs */
DECLARE_DO_FUN( do_clan_tell	); /* Clan Code */
DECLARE_DO_FUN( do_setclan      ); /* Clans */
DECLARE_DO_FUN( do_declan       ); /* Clans */
DECLARE_DO_FUN( do_hours	); /* Shops */
DECLARE_DO_FUN( do_claninfo	); /* Clans */
DECLARE_DO_FUN( do_owned	); /* Quest items, etc */
DECLARE_DO_FUN( clan_advance	); /* Clans */
DECLARE_DO_FUN( clan_demote	); /* Clans */
DECLARE_DO_FUN( clan_accept	); /* Clans */
DECLARE_DO_FUN( do_accounts     ); /* List Accounts */
DECLARE_DO_FUN( do_verify	); /* Verification system */
DECLARE_DO_FUN( clan_outcast    ); /* Clans */
DECLARE_DO_FUN( do_clandelete   ); /* Clans */
DECLARE_DO_FUN( do_reject	); /* Accounts */
DECLARE_DO_FUN( do_clancharge   ); /* Clans */
DECLARE_DO_FUN( do_copyover	); /* Well, it's for copyover */
DECLARE_DO_FUN( do_copyove      ); /* To prevent accidental copyover */
DECLARE_DO_FUN( do_showcompress ); /* Show who is compressing */
DECLARE_DO_FUN( do_cedit 	); /* The full-featured clan edit system */
DECLARE_DO_FUN( do_memlog 	);
DECLARE_DO_FUN( do_myleases 	); /* Lotherius */
DECLARE_DO_FUN( clan_petition 	);
DECLARE_DO_FUN( clan_declare 	);
DECLARE_DO_FUN( clan_truce 	);
DECLARE_DO_FUN( do_version 	);
DECLARE_DO_FUN( do_info 	);
DECLARE_DO_FUN( do_stats 	);
DECLARE_DO_FUN( do_killer 	);
DECLARE_DO_FUN( clan_recognize 	);
DECLARE_DO_FUN( clan_donate 	);
DECLARE_DO_FUN( do_opedit 	);
DECLARE_DO_FUN( do_rpedit 	);
DECLARE_DO_FUN( do_opdump 	);
DECLARE_DO_FUN( do_opstat 	);
DECLARE_DO_FUN( do_rpdump 	);
DECLARE_DO_FUN( do_rpstat 	);
DECLARE_DO_FUN( do_pull 	);
DECLARE_DO_FUN( do_push 	);
DECLARE_DO_FUN( do_climb 	);
DECLARE_DO_FUN( do_turn 	);
DECLARE_DO_FUN( do_play 	);
DECLARE_DO_FUN( do_twist 	);
DECLARE_DO_FUN( do_lift 	);
DECLARE_DO_FUN( do_dig 		);
DECLARE_DO_FUN( do_testfunc 	);
DECLARE_DO_FUN( do_areaexits 	);
DECLARE_DO_FUN( do_delay        );
DECLARE_DO_FUN( do_cscore       );
DECLARE_DO_FUN( do_calendar     );
DECLARE_DO_FUN( do_olc          );
DECLARE_DO_FUN( do_asave        );
DECLARE_DO_FUN( do_alist        );
DECLARE_DO_FUN( do_resets       );
DECLARE_DO_FUN( do_bank         );
DECLARE_DO_FUN( do_withdraw	);
DECLARE_DO_FUN( do_deposit	);
DECLARE_DO_FUN( do_borrow	);
DECLARE_DO_FUN( db_import_area  );
DECLARE_DO_FUN( do_shoplist );
DECLARE_DO_FUN( do_image );
DECLARE_DO_FUN( click_context_char );

#endif // _INTERP_H
