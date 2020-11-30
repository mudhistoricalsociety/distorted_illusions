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

#define COM_INGORE	1

struct pair_type
{   
    char * const        first;
    char * const       second;
    bool              one_way;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_chreset);
DECLARE_DO_FUN( do_readtable    );
DECLARE_DO_FUN( do_abilities    );
DECLARE_DO_FUN( do_accept       );
DECLARE_DO_FUN( do_addlag       );
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN( do_alia		);
DECLARE_DO_FUN( do_alias	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_allpeace	);
DECLARE_DO_FUN( do_ambush       );
DECLARE_DO_FUN( do_announce     );
DECLARE_DO_FUN( do_answer	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_arena        );
DECLARE_DO_FUN( do_astat        );
DECLARE_DO_FUN( do_aclear       );
DECLARE_DO_FUN( do_abusy        );
DECLARE_DO_FUN( do_alock        );
DECLARE_DO_FUN( do_awipe        );
DECLARE_DO_FUN( do_ask		);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN(	do_autopeek	);
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN( do_autostore	);
DECLARE_DO_FUN( do_award        );
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_garrote	);
DECLARE_DO_FUN( do_backu	);
DECLARE_DO_FUN( do_backup	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_bet          );
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN( do_butcher	);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_cdonate	);
DECLARE_DO_FUN( do_cgossip      );
DECLARE_DO_FUN( do_check        );
DECLARE_DO_FUN(	do_clan	);
DECLARE_DO_FUN( do_challenge    );
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN(	do_circle	);
DECLARE_DO_FUN(	do_class	);
DECLARE_DO_FUN( do_cleader	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_colist   );
DECLARE_DO_FUN( do_colour	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_combine	);
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN( do_compress	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_cooldowns	);
DECLARE_DO_FUN( do_copyover     );
DECLARE_DO_FUN(	do_corner	);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_curse	);
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_decline      );
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_demand	);
DECLARE_DO_FUN( do_demote       );
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_deposit	);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disable	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_donate	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN( do_dupe		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_enter	);
DECLARE_DO_FUN( do_envenom	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN( do_exile        );
DECLARE_DO_FUN(	do_exits	);
//DECLARE_DO_FUN( do_faith	);
DECLARE_DO_FUN(	do_feed		);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_finger	);
DECLARE_DO_FUN( do_fires	);
DECLARE_DO_FUN( do_flag		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_forge	);
DECLARE_DO_FUN(	do_forget	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_ftick	);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN( do_gcast    );
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN( do_ghost	);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_gmote        );
DECLARE_DO_FUN( do_gods         );
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_gouge	);
DECLARE_DO_FUN( do_grant        );
DECLARE_DO_FUN( do_gstat        );
DECLARE_DO_FUN( do_revoke       );
DECLARE_DO_FUN( do_grats	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gset		);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guild    	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN( do_ident	);
DECLARE_DO_FUN( do_identify     );
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN(	do_imptalk	);
DECLARE_DO_FUN( do_stockmarket  );
DECLARE_DO_FUN( do_fluct        );
DECLARE_DO_FUN( do_iquest       );
DECLARE_DO_FUN( do_dblexp       );
DECLARE_DO_FUN( do_incognito	);
DECLARE_DO_FUN( do_clanlist     );
DECLARE_DO_FUN( do_clantalk	);
DECLARE_DO_FUN(	do_immkiss	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN( do_layhands	);
DECLARE_DO_FUN( do_lead		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN( do_loner        );
DECLARE_DO_FUN( do_long		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_lore         );
DECLARE_DO_FUN( do_member	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
//DECLARE_DO_FUN(	do_mlevel	);
DECLARE_DO_FUN( do_mlist	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN(	do_mock		);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_modskill     );
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN(	do_mpoint	);
DECLARE_DO_FUN(	do_mquest	);
//DECALRE_DO_FUN( do_stock        );
DECLARE_DO_FUN( do_music	);
DECLARE_DO_FUN( do_mysql	);
DECLARE_DO_FUN( do_newbie	);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN( do_news		);
DECLARE_DO_FUN( do_nightstalker );
DECLARE_DO_FUN(	do_nocancel	);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN( do_noclan	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_noevolve	);
DECLARE_DO_FUN( do_nofalcon	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_nogoto	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN(	do_norestore	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_noshout	);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN( do_noviolate    );
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_notitle	);
DECLARE_DO_FUN( do_notran	);
DECLARE_DO_FUN( do_nscore	);
DECLARE_DO_FUN(	do_ofind	);
//DECLARE_DO_FUN(	do_olevel	);
DECLARE_DO_FUN( do_olist	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN( do_chat         );
DECLARE_DO_FUN( do_boo          );
DECLARE_DO_FUN( do_ic         );
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN( do_owhere	);
DECLARE_DO_FUN( do_pack		);
DECLARE_DO_FUN( do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN(	do_peek		);
DECLARE_DO_FUN( do_penalty	);
DECLARE_DO_FUN( do_permban	);
DECLARE_DO_FUN( do_petition     );
DECLARE_DO_FUN(	do_pick		);
//DECLARE_DO_FUN( do_play		);
DECLARE_DO_FUN( do_pmote	);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN( do_pour		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_pray         ); 
DECLARE_DO_FUN( do_prefi	);
DECLARE_DO_FUN( do_prefix	);
DECLARE_DO_FUN( do_pretitle     );
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_promote	);
DECLARE_DO_FUN( do_protect	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_qgossip	);
DECLARE_DO_FUN( do_quote        );
DECLARE_DO_FUN( do_quotes       );
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_randclan	);
DECLARE_DO_FUN( do_rank		);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_recover	);
DECLARE_DO_FUN( do_remaffect	);
DECLARE_DO_FUN(	do_remembe	);
DECLARE_DO_FUN(	do_remember	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rename	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN( do_repent	);
DECLARE_DO_FUN( do_replay	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rerol	);
DECLARE_DO_FUN(	do_reroll	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restring	);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN( do_rlist	);
DECLARE_DO_FUN(	do_rob		);
DECLARE_DO_FUN( do_roster     	);
DECLARE_DO_FUN( do_rosterpurge	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN( do_run		);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN( do_second	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_sense    );
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN( do_sharpen      );
DECLARE_DO_FUN( do_short	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN( do_show		);
DECLARE_DO_FUN( do_showclass	);
DECLARE_DO_FUN( do_showskill	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_sign		);
DECLARE_DO_FUN( do_sing		);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN( do_skin		);
DECLARE_DO_FUN( do_sskill       );
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN( do_slots	);
DECLARE_DO_FUN( do_smokebomb	);
DECLARE_DO_FUN( do_smote	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_social       );
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stance       );
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_story	);
DECLARE_DO_FUN( do_strangle	);
DECLARE_DO_FUN( do_string	);
//DECLARE_DO_FUN( do_sunder       );
//DECLARE_DO_FUN(	do_surrender	);
//DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_track	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_travel       );
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_twit		);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN( do_unalias	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN( do_unread	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_vdpi		);
DECLARE_DO_FUN(	do_vdth		);
DECLARE_DO_FUN(	do_vdtr		);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_violate	);
DECLARE_DO_FUN(	do_vload	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_voodoo	);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN( do_wecho	);
DECLARE_DO_FUN(	do_weddings	);
DECLARE_DO_FUN(	do_wedpost	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
//DECLARE_DO_FUN(	do_wipe		);
DECLARE_DO_FUN(	do_withdraw	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wiznet	);
DECLARE_DO_FUN( do_wizslap	);
DECLARE_DO_FUN( do_pandaslap	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_zecho	);
DECLARE_DO_FUN( do_olc		);
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_aedit	);
DECLARE_DO_FUN( do_medit	);
/*DECLARE_DO_FUN( do_sedit	);*/
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN( do_hedit        );
DECLARE_DO_FUN( do_mob		);
DECLARE_DO_FUN( do_mpstat	);
DECLARE_DO_FUN( do_mpdump	);

DECLARE_DO_FUN( do_war	);
DECLARE_DO_FUN( do_startwar	);
DECLARE_DO_FUN( do_nowar  );
DECLARE_DO_FUN( do_pload  );
DECLARE_DO_FUN( do_preturn );
DECLARE_DO_FUN( do_cedit);
DECLARE_DO_FUN( do_stances );
DECLARE_DO_FUN( do_decapitate );
DECLARE_DO_FUN( do_focus );
DECLARE_DO_FUN( do_quickening );
DECLARE_DO_FUN( do_power );
DECLARE_DO_FUN( do_shriek );
DECLARE_DO_FUN( do_freevnum );
DECLARE_DO_FUN( do_rubeyes );
DECLARE_DO_FUN( do_smother );
DECLARE_DO_FUN( do_rolldirt );
DECLARE_DO_FUN( do_watch );
DECLARE_DO_FUN( do_twirl );
DECLARE_DO_FUN( do_detect_terrain );
DECLARE_DO_FUN( do_herb  );
DECLARE_DO_FUN( do_barkskin );
DECLARE_DO_FUN( do_blend );
DECLARE_DO_FUN( do_learned);

/*DECLARE_DO_FUN( do_nclan);
DECLARE_DO_FUN( do_ncset);
DECLARE_DO_FUN( do_nroster);
DECLARE_DO_FUN( do_nrosterpurge);
DECLARE_DO_FUN( do_ncleader);
DECLARE_DO_FUN( do_nclanlist);
DECLARE_DO_FUN( do_nclantalk);
DECLARE_DO_FUN( do_ncgossip);
DECLARE_DO_FUN( do_npromote);
DECLARE_DO_FUN( do_ndemote);
DECLARE_DO_FUN( do_nexile);
DECLARE_DO_FUN( do_nloner);
DECLARE_DO_FUN( do_npetition_list);
DECLARE_DO_FUN( do_npetition);
DECLARE_DO_FUN( do_reclan);*/
DECLARE_DO_FUN( do_copyover );
DECLARE_DO_FUN( do_release  );
//DECLARE_DO_FUN( do_mark     );
DECLARE_DO_FUN( do_testsave );
DECLARE_DO_FUN( do_open_palming );
DECLARE_DO_FUN( do_martial_arts );
DECLARE_DO_FUN( do_classstat    );
DECLARE_DO_FUN( do_combat );
DECLARE_DO_FUN( do_subdue );
DECLARE_DO_FUN( do_deathblow );
DECLARE_DO_FUN( do_tie       );
DECLARE_DO_FUN( do_gag       );
DECLARE_DO_FUN( do_blindfold );
DECLARE_DO_FUN( do_throw     );
DECLARE_DO_FUN( do_lunge     );
DECLARE_DO_FUN( do_nerve     );
DECLARE_DO_FUN( do_blaze     );
DECLARE_DO_FUN( do_hara_kiri );
DECLARE_DO_FUN( do_hari );
DECLARE_DO_FUN( do_adamantium_palm );
DECLARE_DO_FUN( do_handle    );
DECLARE_DO_FUN( do_unghost   );
DECLARE_DO_FUN( do_roundhouse );
//DECLARE_DO_FUN( do_spinkick   );
DECLARE_DO_FUN( do_fists      );
DECLARE_DO_FUN( do_chi        );
DECLARE_DO_FUN( do_michi      );
DECLARE_DO_FUN( do_inner_str  );
DECLARE_DO_FUN( do_pk         );
DECLARE_DO_FUN( do_conceal    );
DECLARE_DO_FUN( do_conceal2   );
DECLARE_DO_FUN( do_bounty     );
DECLARE_DO_FUN( do_target     );
DECLARE_DO_FUN( do_crit_strike );
DECLARE_DO_FUN( do_vengeance  );
DECLARE_DO_FUN( do_cripple    );
//DECLARE_DO_FUN( do_battle_fury );

//DECLARE_DO_FUN( do_emblazon   );
//DECLARE_DO_FUN( do_scratch    );
DECLARE_DO_FUN( do_modis_anger  );
DECLARE_DO_FUN( do_concentration );
DECLARE_DO_FUN( do_calm       );
DECLARE_DO_FUN( do_sorcery    );
DECLARE_DO_FUN(do_side_kick     );
DECLARE_DO_FUN(do_spin_kick     );
DECLARE_DO_FUN(do_blindness_dust);
DECLARE_DO_FUN(do_will_of_iron  );
DECLARE_DO_FUN(do_purify_body  );
DECLARE_DO_FUN(do_healing_trance  );
DECLARE_DO_FUN(do_trance  );
DECLARE_DO_FUN(do_embalm);
DECLARE_DO_FUN(do_smite);
/*DECLARE_DO_FUN(do_silence);*/
DECLARE_DO_FUN(do_plist);
DECLARE_DO_FUN(do_nuke);
DECLARE_DO_FUN(do_chop);
DECLARE_DO_FUN(do_head_cut);
DECLARE_DO_FUN(do_divide);
DECLARE_DO_FUN(do_shadow_form);
DECLARE_DO_FUN(do_solar_flare);
DECLARE_DO_FUN(do_prayer);
DECLARE_DO_FUN(do_snattack);
DECLARE_DO_FUN(do_snattacktwo);
DECLARE_DO_FUN(do_greed);
DECLARE_DO_FUN(do_bloodlust);
DECLARE_DO_FUN(do_nightwish);
DECLARE_DO_FUN(do_vmight);
DECLARE_DO_FUN(do_phsyco);
DECLARE_DO_FUN(do_doas);
DECLARE_DO_FUN(do_todo);
DECLARE_DO_FUN(do_tocode);
DECLARE_DO_FUN(do_nohelp);
DECLARE_DO_FUN(do_changed);
DECLARE_DO_FUN(do_spartan_bash);
DECLARE_DO_FUN(do_spear_jab);
DECLARE_DO_FUN(do_spirit_sparta);
DECLARE_DO_FUN(do_rally);
DECLARE_DO_FUN(do_blood_dance);
DECLARE_DO_FUN(do_bladedance);
DECLARE_DO_FUN(do_grenado);
DECLARE_DO_FUN(do_shadow_dance);
//DECLARE_DO_FUN( do_buildvance );
