/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/


/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(      spell_acid_rain         );
DECLARE_SPELL_FUN(	spell_animate		);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_betray		);
DECLARE_SPELL_FUN(      spell_barkskin          );
DECLARE_SPELL_FUN(      spell_barrier           );
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_cone_cold		);
DECLARE_SPELL_FUN(	spell_conjure		);
DECLARE_SPELL_FUN(      spell_conjure_blizzard  );
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_rose	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_deathshield	);
DECLARE_SPELL_FUN(      spell_demonfire		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_good	);
DECLARE_SPELL_FUN(	spell_detect_motion	);
DECLARE_SPELL_FUN(      spell_insomnia          );
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_location   );
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(      spell_dispel_good       );
DECLARE_SPELL_FUN(      spell_dispel_invis      );
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_displace		);
DECLARE_SPELL_FUN(	spell_drain_blade	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_empower		);
DECLARE_SPELL_FUN(	spell_empower_blade	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_entity		);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_feeble_mind	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_fireproof		);
DECLARE_SPELL_FUN(	spell_fireshield	);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_flame_blade	);
DECLARE_SPELL_FUN(	spell_floating_disc	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_frost_blade	);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_heat_metal	);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_holy_aura		);
DECLARE_SPELL_FUN(      spell_generations       );
DECLARE_SPELL_FUN(      spell_iceward           );
DECLARE_SPELL_FUN(      spell_acidward          );
DECLARE_SPELL_FUN(      spell_fireward          );
DECLARE_SPELL_FUN(      spell_sense_vitality    );
DECLARE_SPELL_FUN(      spell_interven          );
DECLARE_SPELL_FUN(      spell_madness           );
DECLARE_SPELL_FUN(      spell_iceshield		);
DECLARE_SPELL_FUN(	spell_ice_storm		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_illusion_granduer );
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_nexus		);
//DECLARE_SPELL_FUN(      spell_quench            );
//DECLARE_SPELL_FUN(      spell_sate              );
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_voodan_curse	);
DECLARE_SPELL_FUN(	spell_soul_burn		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_prismatic_spray	);
DECLARE_SPELL_FUN(	spell_protection_evil	);
DECLARE_SPELL_FUN(	spell_protection_good	);
DECLARE_SPELL_FUN(	spell_protection_voodoo	);
DECLARE_SPELL_FUN(	spell_quest_pill	);
DECLARE_SPELL_FUN(	spell_ray_of_truth	);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_restore_mana	);
DECLARE_SPELL_FUN(	spell_resurrect		);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shockshield	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(      spell_siphon_energy     );
DECLARE_SPELL_FUN(      spell_siphon_life       );
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(      spell_stomp		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(      spell_holy_aura         );
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_transport		);
DECLARE_SPELL_FUN(      spell_unite             );
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_voodoo		);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_wizard_eye        );
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(      spell_wrath_of_god      );
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_torrents_flame	);
DECLARE_SPELL_FUN(      spell_divine_protection );
DECLARE_SPELL_FUN(      spell_protective_aura   );
DECLARE_SPELL_FUN(      spell_hold_align        );
DECLARE_SPELL_FUN(      spell_turn_undead       );
DECLARE_SPELL_FUN(      spell_forestwalk        );
DECLARE_SPELL_FUN(      spell_shadowwalk        );
DECLARE_SPELL_FUN(      spell_entangle          );
DECLARE_SPELL_FUN(      spell_crevice           );
DECLARE_SPELL_FUN(      spell_god_favor         );
DECLARE_SPELL_FUN(      song_charm_person       );
DECLARE_SPELL_FUN(      song_calm               );
DECLARE_SPELL_FUN(      song_sonic_blast        );
DECLARE_SPELL_FUN(      song_rage               );
DECLARE_SPELL_FUN(      spell_conjure_wrath     );
DECLARE_SPELL_FUN(		spell_transmute			);
DECLARE_SPELL_FUN(      spell_channel           );
DECLARE_SPELL_FUN(      spell_wither            );
DECLARE_SPELL_FUN(      spell_soulbind          );
DECLARE_SPELL_FUN(      spell_domineer          );
DECLARE_SPELL_FUN(      spell_headache          );
DECLARE_SPELL_FUN(      spell_dimension_walk    );
DECLARE_SPELL_FUN(      spell_earthmeld         );
DECLARE_SPELL_FUN(      spell_enforcer          );

DECLARE_SPELL_FUN(      spell_animate_dead      );
DECLARE_SPELL_FUN(      spell_animate_skeleton  );
DECLARE_SPELL_FUN(      spell_mummify           );
DECLARE_SPELL_FUN(      spell_decay_corpse      );
DECLARE_SPELL_FUN(      spell_room_sanctuary    );
DECLARE_SPELL_FUN(      spell_steel_flesh       );
DECLARE_SPELL_FUN(      spell_mana_shield       );
DECLARE_SPELL_FUN(      spell_darken_blade      );
DECLARE_SPELL_FUN(      spell_empower_blade     );
DECLARE_SPELL_FUN(      spell_flame_blade       );
DECLARE_SPELL_FUN(      spell_frost_blade       );
DECLARE_SPELL_FUN(      spell_acid_blade        );
DECLARE_SPELL_FUN(      spell_illusion_armor    );

DECLARE_SPELL_FUN(      spell_lesser_golem      );
DECLARE_SPELL_FUN(      spell_greater_golem     );
DECLARE_SPELL_FUN(      spell_preserve_limb     );
DECLARE_SPELL_FUN(      spell_cremate           );
DECLARE_SPELL_FUN(      spell_imbue_weapon      );
DECLARE_SPELL_FUN(      spell_restore           );
DECLARE_SPELL_FUN(      spell_fear              );
DECLARE_SPELL_FUN(		spell_overgrowth		);
DECLARE_SPELL_FUN( spell_fork                   );
DECLARE_SPELL_FUN(	spell_courage	        );

/* New Spells for casters, will organize later */
DECLARE_SPELL_FUN(		spell_martyr			);  //Saint Aggro-drawing spell
DECLARE_SPELL_FUN(		spell_mana_blast		);	//Archmage damage spell
DECLARE_SPELL_FUN(      spell_fortify_strength  );   // Saint Stat buff spells 
DECLARE_SPELL_FUN(      spell_fortify_intelligence );
DECLARE_SPELL_FUN(      spell_fortify_wisdom    );
DECLARE_SPELL_FUN(      spell_fortify_dexterity );
DECLARE_SPELL_FUN(      spell_fortify_constitution );
DECLARE_SPELL_FUN(      spell_invigorate_pith   );   // Archmage stat buff spells
DECLARE_SPELL_FUN(      spell_invigorate_mind   );
DECLARE_SPELL_FUN(      spell_invigorate_lore   );
DECLARE_SPELL_FUN(      spell_invigorate_body   );
DECLARE_SPELL_FUN(      spell_ancient_dance     );
DECLARE_SPELL_FUN(      spell_dancer_heal       );
DECLARE_SPELL_FUN(      spell_blood_shield      );
DECLARE_SPELL_FUN(      spell_invigorate_tact   );
/* Global spell variable */
extern  int  dam_mult;
