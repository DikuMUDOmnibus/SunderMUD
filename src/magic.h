/*
 *  The unique portions of SunderMud code as well as the integration efforts
 *  for code from other sources is based on the efforts of:
 *
 *  Lotherius (elfren@aros.net)
 *
 *  This code can only be used under the terms of the DikuMud, Merc,
 *  and ROM licenses. The same requirements apply to the changes that
 *  have been made.
 *
 * All other copyrights remain in place and in force.
*/

#ifndef _MAGIC_H
#define _MAGIC_H   1


/* functions needed by all magic*.c files */
/* all of these are found in magic.c */

void    say_spell       args( ( CHAR_DATA *ch, int sn ) );
int 	skill_lookup	args( ( const char *name ) );
int 	slot_lookup	args( ( int slot ) );
bool 	saves_spell	args( ( int level, CHAR_DATA *victim ) );
int	mana_cost 	args( (CHAR_DATA *ch, int min_mana, int level) );
int 	spell_calc	args( (CHAR_DATA *ch, int sn) );
bool 	check_dispel 	args( ( int dis_level, CHAR_DATA * victim, int sn ) );
bool 	saves_dispel 	args( ( int dis_level, int spell_level, int duration ) );



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

/* spells used in Merc */

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(  spell_brand		);
DECLARE_SPELL_FUN(  spell_mask_self );
DECLARE_SPELL_FUN(  spell_absorb_magic );
DECLARE_SPELL_FUN(      spell_age               ); /* elfren */
DECLARE_SPELL_FUN(	spell_youth		); /* elfren */
DECLARE_SPELL_FUN(      spell_animate           ); /* elfren */
DECLARE_SPELL_FUN(      spell_soul_blade        ); /* elfren */
DECLARE_SPELL_FUN(      spell_minor_creation 	); /* elfren */
DECLARE_SPELL_FUN(	spell_psi_twister	); /* elfren */
DECLARE_SPELL_FUN(      spell_exorcism		); /* elfren */
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_minor_chaos	); /* elfren */
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_chaos		); /* elfren */
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_confusion		); /* elfren */
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(      spell_create_buffet     ); /* elfren */
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(      spell_demonfire		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_good	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_good	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_entropy		); /* elfren */
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fear	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_gate_without_error ); /* elfren */
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(      spell_slow		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_mute	);
DECLARE_SPELL_FUN(  spell_negate_alignment );
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_protection_evil	);
DECLARE_SPELL_FUN(	spell_protection_good	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN( 	spell_regeneration	);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_remove_fear	);
DECLARE_SPELL_FUN(	spell_remove_invis	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_ghostform		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_vocalize	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(  spell_mind_meld    ); /* elfren */
DECLARE_SPELL_FUN(	spell_psychic_anchor );
DECLARE_SPELL_FUN(  spell_fire_shield  );
DECLARE_SPELL_FUN(	spell_summon_familiar	); /* Cthulhu */
DECLARE_SPELL_FUN(	spell_meteor		); /* elfren */
DECLARE_SPELL_FUN(	spell_boil		); /* elfren */
DECLARE_SPELL_FUN(	spell_dehydrate		); /* elfren */
DECLARE_SPELL_FUN( 	spell_meteor_shower	); /* Elfren */

#endif // _MAGIC_H
