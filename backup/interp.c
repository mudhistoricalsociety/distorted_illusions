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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* unlink() */
#include "merc.h"
#include "interp.h"

bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );

bool 	check_disabled (const struct cmd_type *command);
DISABLED_DATA *disabled_first;
/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2
#define END_MARKER	"END" /* for load_disabled() and save_disabled() */


/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;

bool				global_quest	= FALSE;
bool                            global_quest_closed    = FALSE;
bool                            dbl_exp  = FALSE;
bool                            dbl_exp_lvl = FALSE;
bool                            timed_dbl_exp = FALSE;
char                            last_command[MAX_STRING_LENGTH];


/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  1,  LOG_NEVER, 0 },
    { "east",		do_east,	POS_STANDING,	 0,  1,  LOG_NEVER, 0 },
    { "south",		do_south,	POS_STANDING,	 0,  1,  LOG_NEVER, 0 },
    { "west",		do_west,	POS_STANDING,	 0,  1,  LOG_NEVER, 0 },
    { "up",		do_up,		POS_STANDING,	 0,  1,  LOG_NEVER, 0 },
    { "down",		do_down,	POS_STANDING,	 0,  1,  LOG_NEVER, 0 },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */

    { "accept",		do_accept,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "auction",        do_auction,     POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "challenge",	do_challenge,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "chreset",        do_chreset, POS_DEAD, 0,1, LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "decline",	do_decline,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "decapitate",     do_decapitate,  POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "focus",          do_focus,       POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "group",          do_group,       POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 0 },
    { "inventory",	do_inventory,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "lore",           do_lore,        POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "learned",        do_learned,     POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "clan",		do_clantalk,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "classstat",      do_classstat,   POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    //{ "mark",           do_mark,        POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "martial",      do_martial_arts,  POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "music",          do_music,   	POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 }, 
    { "mock",		do_mock,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "order",		do_order,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "practice",       do_practice,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "pk",             do_pk,          POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "release",        do_release,     POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "run",		do_run,		POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "rubeyes",        do_rubeyes,     POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "sing",		do_sing,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1},
    { "sit",		do_sit,		POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "stance",         do_stance,      POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "stances",        do_stances,     POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "unlock",         do_unlock,      POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	 0,  2,  LOG_NORMAL, 1 },

    /*
     * Informational commands.
     */
    { "clist",	        do_clanlist,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "colist",          do_colist,    POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "identify",       do_identify,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "petition",	do_petition,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "promote",	do_promote,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 }, 
    { "demote",	        do_demote,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 }, 
    { "exile",	        do_exile,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "clan",	        do_clantalk,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "loner",	        do_loner,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "gods",           do_gods,        POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "affects",	do_affects,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "areas",		do_areas,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "bug",		do_bug,		POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "changes",	do_changes,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "compare",	do_compare,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "cooldowns",	do_cooldowns,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
/*  { "groups",		do_groups,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 }, */
    { "gmote",          do_gmote,       POS_RESTING,     0,  1,  LOG_NORMAL, 1 }, 
    { "?",		do_help,	POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "help",		do_help,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "score",		do_nscore,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 }, 
    { "ic",           do_ic,        POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "ooc",		do_chat,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "boo",            do_boo,         POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "peek",		do_peek,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "read",		do_read,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "oldscore",	do_score,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "abilties",       do_abilities,   POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "scan",		do_scan,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "sskill",		do_sskill,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "skills",         do_skills,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "social",         do_social,      POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "sorcery",        do_sorcery,     POS_DEAD,       109, 1,  LOG_NORMAL, 1 },
    { "show",		do_show,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "showclass",	do_showclass,   POS_DEAD,	 0,  1,  LOG_NORMAL,1},
    { "showskill",	do_showskill,   POS_DEAD,	 0,  1,  LOG_NORMAL,1},
    { "spells",		do_spells,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "story",		do_story,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "worth",		do_worth,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "changed",	do_changed,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "nohelp",		do_nohelp,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },

    /*
     * Configuration commands.
     */
    { "alia",		do_alia,	POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "alias",		do_alias,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "autolist",	do_autolist,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autogold",	do_autogold,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autosac",	do_autosac,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autostore",	do_autostore,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "autopeek",	do_autopeek,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "brief",		do_brief,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "channels",	do_channels,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 }, 
    { "colour",         do_colour,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "color",          do_colour,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "combine",	do_combine,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "combat",         do_combat,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "compact",	do_compact,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "compress",	do_compress,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "long",		do_long,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "noevolve",	do_noevolve,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "nofalcon",	do_nofalcon,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "noloot",         do_noloot,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "nosummon",	do_nosummon,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "notran",		do_notran,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "nocancel",	do_nocancel,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "outfit",		do_outfit,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "palm",           do_open_palming, POS_FIGHTING,   0,  1,  LOG_NORMAL, 1 },
    { "password",	do_password,	POS_DEAD,	 0,  1,  LOG_NEVER,  1 },
    { "prompt",		do_prompt,	POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "short",		do_short,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "unalias",	do_unalias,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    /*
     * Class commands by DUSK.
     */
//    { "sunder",         do_sunder,      POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "nightstalker",   do_nightstalker,   POS_RESTING,     0,  1,  LOG_NORMAL, 1 },


    /*
     * Communication commands.
     */
    { "afk",		do_afk,		POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "answer",		do_answer,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "ansi",		do_colour,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "auction",	do_auction,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "deaf",		do_deaf,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "deathblow",      do_deathblow,   POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "emote",		do_emote,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "pmote",		do_pmote,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "=",		do_clantalk,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 0 },
    { "clantalk",	do_clantalk,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "-",		do_qgossip,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 0 },
    { "qgossip",	do_qgossip,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "cgossip",        do_cgossip,     POS_SLEEPING,    0,   1,  LOG_NORMAL, 1 }, 
    { ",",		do_emote,	POS_RESTING,	 0,  1,  LOG_NORMAL, 0 },
    { "grats",		do_grats,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { ";",		do_gtell,	POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "music",		do_music,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "iquest",         do_iquest,      POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "note",		do_note,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "pose",		do_pose,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "power",          do_power,       POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "quest",		do_quest,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "ask",		do_ask,		POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "quote",          do_quote,       POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "quotes",		do_quotes,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  1,  LOG_NORMAL, 1 },
    { "quickening",     do_quickening,  POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "'",		do_say,		POS_RESTING,	 0,  1,  LOG_NORMAL, 0 },
    { "shriek",         do_shriek,      POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "shout",		do_shout,	POS_RESTING,	 3,  1,  LOG_NORMAL, 1 },
    { "unread",		do_unread,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "weddings",	do_weddings,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "yell",		do_yell,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "pray",           do_pray,        POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },

    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "butcher",	do_butcher,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "close",		do_close,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "drink",		do_drink,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    //{ "emblazon",       do_emblazon,    POS_STANDING,    0,  1,  LOG_ALWAYS, 1 },
    { "envenom",	do_envenom,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "fists",          do_fists,       POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "repent",		do_repent,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "curse",		do_curse,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "hold",		do_wear,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "list",		do_list,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "pour",		do_pour,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "put",		do_put,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "recite",		do_recite,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "rob",		do_rob,		POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "rolldirt",       do_rolldirt,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    //{ "scratch",        do_scratch,     POS_STANDING,    0,  1,  LOG_ALWAYS, 1 },
    { "sell",		do_sell,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "second",		do_second,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "sharpen",        do_sharpen,     POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "junk",           do_sacrifice,   POS_RESTING,     0,  1,  LOG_NORMAL, 0 },
    { "travel",         do_travel,      POS_STANDING,    0,  1,  LOG_NORMAL, 0 },
    { "tap",      	do_sacrifice,   POS_RESTING,     0,  1,  LOG_NORMAL, 0 },   
    { "unlock",		do_unlock,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "value",		do_value,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "zap",		do_zap,		POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },

    /*
     * Combat commands.
     */
    { "ambush",         do_ambush,      POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "apalm",          do_adamantium_palm, POS_FIGHTING, 0, 1,  LOG_NORMAL, 1 },
    { "backstab",	do_backstab,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "garrote",	do_garrote,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 0 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "circle",		do_circle,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "demand",		do_demand,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "feed",		do_feed,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "gouge",		do_gouge,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "strangle",       do_strangle,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "subdue",         do_subdue,      POS_RESTING,     0,  1,  LOG_NORMAL, 1 },
    //{ "surrender",	do_surrender,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 0 },
    { "trip",		do_trip,	POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "twirl",          do_twirl,       POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "smokebomb",	do_smokebomb,	POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "smother",        do_smother,     POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "layhands",	do_layhands,	POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "lead",		do_lead,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "blend",          do_blend,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "detect terrain", do_detect_terrain,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "barkskin",       do_barkskin,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "herb",       do_herb,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    //{ "blaze",       do_blaze,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "lunge",       do_lunge,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "nerve",       do_nerve,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "throw",       do_throw,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "prayer",      do_prayer,  POS_FIGHTING,   0, 1,  LOG_NORMAL,  1 }, 
    { "chop",        do_chop,     POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "headcut",     do_head_cut, POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    //{ "harakiri",    do_hara_kiri, POS_RESTING,   0,  1,  LOG_NORMAL, 1 },
    { "harakiri",       do_hari, POS_RESTING, 0, 1, LOG_NORMAL, 1 },
    { "handle",     do_handle, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "roundhouse", do_roundhouse, POS_FIGHTING, 0, 1, LOG_NORMAL, 1 },
    //{ "spinkick",   do_spinkick,   POS_FIGHTING, 0, 1, LOG_NORMAL, 1 },
    { "chi",        do_chi,        POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "michi",      do_michi,      POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "inner strength", do_inner_str, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "conceal",    do_conceal,    POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "manuevers", do_snattack, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
  //  { "transform",  do_conceal2,   POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "divide", do_divide, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "shadow form", do_shadow_form, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "phsyco", do_phsyco, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "might", do_vmight, POS_STANDING, 0, 1, LOG_NORMAL, 1, },
    { "blood lust", do_bloodlust, POS_STANDING, 0, 1, LOG_NORMAL, 1, },
    { "greed", do_greed, POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "bounty",     do_bounty,     POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "target",     do_target,     POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    //{ "faith",      do_faith,	   POS_FIGHTING, 0, 1, LOG_NORMAL, 1 },
    { "cripple",    do_cripple,    POS_FIGHTING, 0, 1, LOG_NORMAL, 1 },
    //{ "battle fury", do_battle_fury, POS_FIGHTING, 0, 1, LOG_NORMAL, 1 },
  //  { "battlecry",  do_modis_anger, POS_FIGHTING, 0, 1, LOG_NORMAL, 1},
    { "vengeance", do_vengeance, POS_STANDING, 0, 1, LOG_NORMAL, 1},
    { "critical strike", do_crit_strike, POS_FIGHTING, 0, 1, LOG_NORMAL, 1},
    { "concentration", do_concentration, POS_FIGHTING, 0, 1, LOG_NORMAL, 1},
    { "calm",  do_calm, POS_FIGHTING, 0, 1, LOG_NORMAL, 1},
    { "fires",     do_fires,       POS_STANDING, 0, 1, LOG_NORMAL, 1 },
    { "will",       do_will_of_iron,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "purify body",   do_purify_body,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "healing trance",       do_healing_trance,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "side kick",       do_side_kick,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "spin kick",       do_spin_kick,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "blindness dust",       do_blindness_dust,    POS_FIGHTING,    0,  1,  LOG_NORMAL, 1 },
    { "solar flare",      do_solar_flare,     POS_FIGHTING,         0, 1, LOG_NORMAL, 1 },
    { "embalm",       do_embalm,    POS_STANDING,    0,  1,  LOG_NORMAL, 1 },
    { "trance",   do_trance,  POS_STANDING,	0,  1,  LOG_NORMAL, 1 },
    

    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",		do_mob,         POS_DEAD,        0,  1,	 LOG_NEVER,  0 },

    /*
     * Miscellaneous commands.
     */
    { "enter", 		do_enter, 	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "gain",		do_gain,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "go",		do_enter,	POS_STANDING,	 0,  1,  LOG_NORMAL, 0 },
    { "group",		do_group,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "hide",		do_hide,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    /*{ "play",		do_play,	POS_RESTING,	 0,  1,  LOG_ALWAYS, 1  }, */
    { "practice",	do_practice,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 }, 
    { "qui",		do_qui,		POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "quit",		do_quit,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "rank",		do_rank,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 1 },
    { "/",		do_recall,	POS_FIGHTING,	 0,  1,  LOG_NORMAL, 0 },
    { "rent",		do_rent,	POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "roster",         do_roster,      POS_DEAD,        1,  1,  LOG_NORMAL, 1 },
    { "save",		do_save,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "slots",		do_slots,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "track",		do_track,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "sense",		do_sense,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "unghost",	do_unghost,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  1,  LOG_NORMAL, 1 },
    { "where",		do_where,	POS_RESTING,	 0,  1,  LOG_NORMAL, 1 },
    { "deposit",	do_deposit,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "donate",		do_donate,	POS_STANDING,	 5,  1,  LOG_NORMAL, 1 },
    { "cdonate",	do_cdonate,	POS_STANDING,	 5,  1,  LOG_NORMAL, 1 },
    { "forge",		do_forge,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 0 },
    { "forget",		do_forget,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "remembe",	do_remembe,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 0 },
    { "remember",	do_remember,	POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "voodoo",		do_voodoo,	POS_FIGHTING,	20,  1,  LOG_NORMAL, 1 },
    { "withdraw",	do_withdraw,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "skin",		do_skin,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "preturn",        do_preturn,     POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },


    /*
     *Immortal commands.
     */
   // { "addlag",         do_addlag,      POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "copyover",       do_copyover,    POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "rosterpurge",    do_rosterpurge, POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "wecho",          do_wecho,       POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
    { "advance",        do_advance,     POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "plist",          do_plist,       POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
   // {"buildvance", do_buildvance, POS_DEAD, L1, LOG_NORMAL, 1},
    { "nuke",           do_nuke,        POS_DEAD,       ML,  1,  LOG_ALWAYS, 1 },
   // { "backu",          do_backu,       POS_DEAD,       L1,  1,  LOG_ALWAYS, 0 },
   // { "backup",         do_backup,      POS_DEAD,       ML,  1,  LOG_ALWAYS, 1 },
    { "dupe",		do_dupe,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    { "log",		do_log,		POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    //booya
    { "grant",		do_grant,	POS_DEAD,	ML,  1,  LOG_ALWAYS, 1 },
    { "revoke",		do_revoke,	POS_DEAD,	ML,  1,  LOG_ALWAYS, 1 },
    { "pandaslap",  do_pandaslap,   POS_DEAD,       ML,  1,  LOG_NORMAL, 1 },
    { "imptalk",    do_imptalk,     POS_DEAD,       ML,  1,  LOG_NORMAL, 1 },
    { "gstat",		do_gstat,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
/* MYSQL
    { "mysql",		do_mysql,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
*/
    { "dblexp",         do_dblexp,      POS_DEAD,    L1,  1,  LOG_ALWAYS, 1 },
    { "gcast",          do_gcast,       POS_DEAD,     L1,  1,  LOG_NORMAL,  1 },
    { "award",          do_award,       POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "pload",          do_pload,       POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
    { "modskill",       do_modskill,    POS_DEAD,       L1,  1,  LOG_ALWAYS, 1 },
//    { "nclan",          do_nclan,       POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
  //  { "ncset",          do_ncset,       POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
    //{ "nrosterpurge",   do_nrosterpurge,POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
    //{ "ncleader",       do_ncleader,    POS_DEAD,       L1,  1,  LOG_NORMAL, 1 },
    { "permban",	do_permban,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	L1,  1,  LOG_NORMAL, 0 },
    { "reboot",		do_reboot,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  1,  LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    { "trust",		do_trust,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    //{ "wipe",		do_wipe,	POS_DEAD,	L1,  1,  LOG_ALWAYS, 1 },
    { "announce", 	do_announce,   	POS_DEAD,   	L2,  1,  LOG_NORMAL, 1 },
    { "arena",		do_arena,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "cedit",          do_cedit,       POS_DEAD,       L2,   1,  LOG_NORMAL, 1 },
    { "cleader",	do_cleader,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "deny",		do_deny,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "doas",		do_doas,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "guild",		do_guild,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "ftick",		do_ftick,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "rename",		do_rename,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "wedpost",	do_wedpost,	POS_DEAD,	L2,  1,  LOG_NORMAL, 1 },
    { "newlock",	do_newlock,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "protect",	do_protect,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "violate",	do_violate,	POS_DEAD,	L2,  1,  LOG_ALWAYS, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "disable",	do_disable,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "ban",		do_ban,		POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    /*{ "silence",	do_silence,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },*/
    { "noshout",	do_noshout,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "noemote",	do_noemote,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "notell",		do_notell,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "norestore",	do_norestore,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "notitle",	do_notitle,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "sla",		do_sla,		POS_DEAD,	L3,  1,  LOG_ALWAYS, 0 },
    { "slay",		do_slay,	POS_DEAD,	L3,  1,  LOG_ALWAYS, 1 },
    { "allpeace",	do_allpeace,	POS_DEAD,	L3,  1,  LOG_NORMAL, 1 },
    { "dump",		do_dump,	POS_DEAD,	L4,  1,  LOG_ALWAYS, 0 },
    { "noviolate",	do_noviolate,   POS_DEAD,	L4,  1,  LOG_NORMAL, 1 },		
    { "restore",	do_restore,	POS_DEAD,	L4,  1,  LOG_ALWAYS, 1 },
    { "flag",		do_flag,	POS_DEAD,	L4,  1,  LOG_ALWAYS, 1 },
    { "freeze",		do_freeze,	POS_DEAD,	L4,  1,  LOG_ALWAYS, 1 },
    { "set",		do_set,		POS_DEAD,	L4,  1,  LOG_ALWAYS, 1 },
    { "edit",       do_olc,     POS_DEAD,   L5,  1,  LOG_NORMAL, 1 },
    { "gecho",		do_echo,	POS_DEAD,	L5,  1,  LOG_NORMAL, 1 },
    { "memory",		do_memory,	POS_DEAD,	L5,  1,  LOG_NORMAL, 1 },
    { "mpdump",     do_mpdump,  POS_DEAD,   L5,  1,  LOG_NEVER, 1 },
    { "mpstat",     do_mpstat,  POS_DEAD,   L5,  1,  LOG_NEVER, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L5,  1,  LOG_NORMAL, 1 }, 
    { "snoop",		do_snoop,	POS_DEAD,	L5,  1,  LOG_ALWAYS, 1 },
    { "resets",     do_resets,  POS_DEAD,   L5,  1,  LOG_NORMAL, 1 },
    { "redit",      do_redit,   POS_DEAD,   L5,  1,  LOG_ALWAYS, 1 },
    { "medit",      do_medit,   POS_DEAD,   L5,  1,  LOG_ALWAYS, 1 },
    { "aedit",      do_aedit,   POS_DEAD,   L5,  1,  LOG_ALWAYS, 1 },
    { "oedit",      do_oedit,   POS_DEAD,   L5,  1,  LOG_ALWAYS, 1 },
    //{ "sedit",      do_sedit,   POS_DEAD,   L5,  1,  LOG_NORMAL, 1 },
    { "mpedit",     do_mpedit,  POS_DEAD,   L5,  1,  LOG_NORMAL, 1 },
    { "alist",      do_alist,   POS_DEAD,   L6,  1,  LOG_ALWAYS, 1 },
    { "at",         do_at,      POS_DEAD,   L6,  1,  LOG_NORMAL, 1 },
    { "clone",		do_clone,	POS_DEAD,	L6,  1,  LOG_ALWAYS, 1 },
    { "load",		do_load,	POS_DEAD,	L6,  1,  LOG_ALWAYS, 1 },
    { "mwhere",		do_mwhere,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "owhere",		do_owhere,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "zecho",		do_zecho,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "immkiss",	do_immkiss,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "mpoint",		do_mpoint,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "mquest",		do_mquest,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "nogoto",		do_nogoto,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "peace",		do_peace,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "return",         do_return,      POS_DEAD,       L6,  1,  LOG_NORMAL, 1 },
    { "sign",		do_sign,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
   // { "switch",		do_switch,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "stat",		do_stat,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "string",		do_string,	POS_DEAD,	L5,  1,  LOG_NORMAL, 1 },
    { "teleport",       do_transfer,    POS_DEAD,       L6,  1,  LOG_ALWAYS, 1 }, 
    { "transfer",       do_transfer,    POS_DEAD,       L6,  1,  LOG_ALWAYS, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    { "wizslap",	do_wizslap,	POS_DEAD,	L6,  1,  LOG_NORMAL, 1 },
    //{ "pandaslap",	do_pandaslap,	POS_DEAD,	ML,  1,  LOG_NORMAL, 1 },
    { "force",		do_force,	POS_DEAD,	L7,  1,  LOG_ALWAYS, 1 },
    { "pardon",		do_pardon,	POS_DEAD,	L7,  1,  LOG_NORMAL, 1 },
    { "purge",		do_purge,	POS_DEAD,	L7,  1,  LOG_NORMAL, 1 },
    { "remaffect",	do_remaffect,	POS_DEAD,	L7,  1,  LOG_NORMAL, 1 },
    { "watch",          do_watch,       POS_DEAD,       L7,  1,  LOG_NORMAL, 1 },
    { "twit",		do_twit,	POS_DEAD,	L7,  1,  LOG_NORMAL, 1 },
    { "newbie",		do_newbie,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "finger",		do_finger,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "sockets",	do_sockets,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 }, 
    { "identity",	do_ident,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "poofin",		do_bamfin,	POS_DEAD,	L8,  1,  LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L8,  1,  LOG_NORMAL, 1 },
    { "pretitle",       do_pretitle,    POS_DEAD,       IM,  1,  LOG_NORMAL, 1 },
    { "corner",		do_corner,	POS_DEAD,	IM,  1,  LOG_ALWAYS, 1 },
    { "goto",		do_goto,	POS_DEAD,	L8,  1,  LOG_NORMAL, 1 }, 
    { "ghost",		do_ghost,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       IM,  1,  LOG_NORMAL, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "incognito",	do_incognito,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	IM,  1,  LOG_NORMAL, 0 },
    { "penalty",	do_penalty,	POS_DEAD,	 0,  1,  LOG_NORMAL, 1 },
    { "olist",		do_olist,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "mlist",		do_mlist,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "rlist",		do_rlist,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    //{ "olevel",		do_olevel,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    //{ "mlevel",		do_mlevel,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "echo",		do_recho,	POS_DEAD,	L8,  1,  LOG_NORMAL, 1 },
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "wiznet",		do_wiznet,	POS_DEAD,	IM,  2,  LOG_NORMAL, 1 },
    { "immtalk",	do_immtalk,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
   // { "imptalk",	do_imptalk,	POS_DEAD,	ML,  1,  LOG_NORMAL, 1 },
    { "imotd",          do_imotd,       POS_DEAD,       IM,  1,  LOG_NORMAL, 1 },
    { ":",		do_immtalk,	POS_DEAD,	ML,  1,  LOG_NORMAL, 0 },
    { "smote",		do_smote,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "prefi",		do_prefi,	POS_DEAD,	IM,  1,  LOG_NORMAL, 0 },
    { "prefix",		do_prefix,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "recover",	do_recover,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "gset",		do_gset,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "testsave",       do_testsave,    POS_DEAD,       IM,  1,  LOG_NORMAL, 1 },
    { "smite",          do_smite,       POS_DEAD,       IM,  1,  LOG_NORMAL, 1 },
    { "todo",		do_todo,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },
    { "tocode",		do_tocode,	POS_DEAD,	IM,  1,  LOG_NORMAL, 1 },

     /*
      * OLC
      */
     { "asave",          do_asave,       POS_DEAD,        IM,  1,  LOG_NORMAL, 1 },
     { "freevnum",       do_freevnum,    POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
     { "hedit",          do_hedit,       POS_DEAD,       IM,   1,  LOG_NORMAL, 1 },
    /*
     * Commands needed last on list
     */
   // { "restring",	do_restring,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "delet",		do_delet,	POS_DEAD,	 0,  1,  LOG_NORMAL, 0 },
    { "delete",		do_delete,	POS_STANDING,	 0,  1,  LOG_NORMAL, 1 },
    { "rerol",		do_rerol,	POS_DEAD,	 0,  2,  LOG_NORMAL, 0 },
    { "reroll",		do_reroll,	POS_STANDING,	 0,  2,  LOG_NORMAL, 1 },
    /* New Clan Commands */
    /*{ "nroster",        do_nroster,     POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "nclist",         do_nclanlist,   POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "npetition",      do_npetition,   POS_RESTING,     0,  1,  LOG_NORMAL, 1 }, 
    { "npromote",       do_npromote,    POS_DEAD,        0,  1,  LOG_NORMAL, 1 }, 
    { "ndemote",        do_ndemote,     POS_DEAD,        0,  1,  LOG_NORMAL, 1 }, 
    { "nexile",         do_nexile,      POS_RESTING,     0,  1,  LOG_NORMAL, 1 }, 
    { "nclantalk",      do_nclantalk,   POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "nloner",         do_nloner,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },
    { "ncgossip",       do_ncgossip,    POS_SLEEPING,    0,  1,  LOG_NORMAL, 1 },
    { "reclan",         do_reclan,      POS_DEAD,        0,  1,  LOG_NORMAL, 1 },*/

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  1,  LOG_NORMAL, 0 }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char *message;
    int cmd;
    int trust;
    bool found;

    /*
     * Placed here, so we don't have to worry about tildes, period.
     * RW
     */
    smash_tilde( argument );

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * No hiding.
     * Moving this to combat and movement specifically to allow normal commands
     * without dropping hide.
    REMOVE_BIT( ch->affected_by, AFF_HIDE ); */


    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    
    /*Lets see who is doing what? -Ferric*/
    char buf[MAX_STRING_LENGTH];
    strcpy( buf, argument);
    sprintf(last_command,"%s in room[%d]: %s.",ch->name,
            ch->in_room->vnum,
            buf);

    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name ) )
	{
	  /*  if ( cmd_table[cmd].level > trust )
    	    {
		found = FALSE;
		break;
		for ( i = 0; i < MAX_GRANT; i++ )
		{
	    		if (( ch->pcdata->grant[i] == NULL ) || ( ch->pcdata->grant[i][0] == '\0' ))
			continue;
	    		if ( !str_prefix( command, ch->pcdata->grant[i] ))
	    		{
				found = TRUE;
				break;
	    		}
		}
    	    }
    	    else 
    	    {
    	    	if (cmd_table[cmd].tier == 1)
	    	{
			found = TRUE;
			break;
	    	}
	    	else if (!IS_NPC(ch) && ch->pcdata->tier >= 2)
	    	{
			found = TRUE;
			break;
	    	} 
	    	else if (ch->level >= LEVEL_HERO)
	    	{
			found = TRUE;
			break;
	    	}
	    }*/
	 
		if (cmd_table[cmd].level < LEVEL_IMMORTAL)
            	{ 
              		if (cmd_table[cmd].level > trust) 
              		continue;
            	}
            	else 
            	{
            	 if (ch->desc == NULL || !is_granted(ch,cmd_table[cmd].do_fun)) 
            	 continue;
		}
            	
            	found = TRUE;
            	break;
            	 
        
	}
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||   (cmd_table[cmd].log == LOG_ALWAYS
    &&   ch->level != MAX_LEVEL ) )
    {
	sprintf( log_buf, "[Monitored %s] :: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	/*
	 * Look for command in socials table.
	 */

	if ( !check_social( ch, command, argument ) )
                switch (number_range(0,7))
		{
		default : message = NULL;
			break;
		case 0: send_to_char("Huh?\n\r",ch);
			break;
		case 1: send_to_char("Are you dumb? Or just stupid?\n\r",ch);
			break;
		case 2: send_to_char("I don't understand....\n\r",ch);
			break;
		case 3: send_to_char("Ain't gonna happen..\n\r",ch);
			break;
		case 4: send_to_char("You wish.\n\r",ch);
			break;	
		case 5: send_to_char("NO!\n\r",ch);
			break;
		case 6: send_to_char("That's not a command.. Type 'help' or 'commands'.\n\r",ch);
			break;
                case 7: send_to_char("Not right now.. Try again in an hour.\n\r",ch);
		        break;
                }
	/*        send_to_char( message, ch ); */
	return;
    }
    else /* a normal valid command.. check if it is disabled */
     	if (check_disabled (&cmd_table[cmd]))
     	{
     		send_to_char ("This command has been temporarily disabled.\n\r",ch);
     		return;
     	}
    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );

    return;
}


void do_disable (CHAR_DATA *ch, char *argument)
{
	int i;
	DISABLED_DATA *p,*q;
	char buf[100];
	
	if (IS_NPC(ch))
	{
		send_to_char ("RETURN first.\n\r",ch);
		return;
	}
	
	if (!argument[0]) /* Nothing specified. Show disabled commands. */
	{
		if (!disabled_first) /* Any disabled at all ? */
		{
			send_to_char ("There are no commands disabled.\n\r",ch);
			return;
		}

		send_to_char ("Disabled commands:\n\r"
		              "Command      Level   Disabled by\n\r",ch);
		                
		for (p = disabled_first; p; p = p->next)
		{
			sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
			send_to_char (buf,ch);
		}
		return;
	}
	
	/* command given */

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (!str_cmp(argument, p->command->name))
			break;
			
	if (p) /* this command is disabled */
	{
	/* Optional: The level of the imm to enable the command must equal or exceed level
	   of the one that disabled it */
	
		if (get_trust(ch) < p->level)
		{
			send_to_char ("This command was disabled by a higher power.\n\r",ch);
			return;
		}
		
		/* Remove */
		
		if (disabled_first == p) /* node to be removed == head ? */
			disabled_first = p->next;
		else /* Find the node before this one */
		{
			for (q = disabled_first; q->next != p; q = q->next); /* empty for */
			q->next = p->next;
		}
		
		free_string (p->disabled_by); /* free name of disabler */
		free_mem (p,sizeof(DISABLED_DATA)); /* free node */
		save_disabled(); /* save to disk */
		send_to_char ("Command enabled.\n\r",ch);
	}
	else /* not a disabled command, check if that command exists */
	{
		/* IQ test */
		if (!str_cmp(argument,"disable"))
		{
			send_to_char ("You cannot disable the disable command.\n\r",ch);
			return;
		}

		/* Search for the command */
		for (i = 0; cmd_table[i].name[0] != '\0'; i++)
			if (!str_cmp(cmd_table[i].name, argument))
				break;

		/* Found? */				
		if (cmd_table[i].name[0] == '\0')
		{
			send_to_char ("No such command.\n\r",ch);
			return;
		}

		/* Can the imm use this command at all ? */				
		if (cmd_table[i].level > get_trust(ch))
		{
			send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
			return;
		}
		
		/* Disable the command */
		
		p = alloc_mem (sizeof(DISABLED_DATA));

		p->command = &cmd_table[i];
		p->disabled_by = str_dup (ch->name); /* save name of disabler */
		p->level = get_trust(ch); /* save trust */
		p->next = disabled_first;
		disabled_first = p; /* add before the current first element */
		
		send_to_char ("Command disabled.\n\r",ch);
		save_disabled(); /* save to disk */
	}
}

bool check_disabled (const struct cmd_type *command)
{
	DISABLED_DATA *p;
	
	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

void load_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	char *name;
	int i;
	
	disabled_first = NULL;
	
	fp = fopen (DISABLED_FILE, "r");
	
	if (!fp) /* No disabled file.. no disabled commands : */
		return;
		
	name = fread_word (fp);
	
	while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
	{
		/* Find the command in the table */
		for (i = 0; cmd_table[i].name[0] ; i++)
			if (!str_cmp(cmd_table[i].name, name))
				break;
				
		if (!cmd_table[i].name[0]) /* command does not exist? */
		{
			bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
			fread_number(fp); /* level */
			fread_word(fp); /* disabled_by */
		}
		else /* add new disabled command */
		{
			p = alloc_mem(sizeof(DISABLED_DATA));
			p->command = &cmd_table[i];
			p->level = fread_number(fp);
			p->disabled_by = str_dup(fread_word(fp)); 
			p->next = disabled_first;
			
			disabled_first = p;

		}
		
		name = fread_word(fp);
	}

	fclose (fp);	
}

void save_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	
	if (!disabled_first) /* delete file if no commands are disabled */
	{
		unlink (DISABLED_FILE);
		return;
	}
	
	fp = fopen (DISABLED_FILE, "w");
	
	if (!fp)
	{
		bug ("Could not open " DISABLED_FILE " for writing",0);
		return;
	}
	
	for (p = disabled_first; p ; p = p->next)
		fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
		
	fprintf (fp, "%s\n",END_MARKER);
		
	fclose (fp);
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
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
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
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
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	/*    if (cmd_table[cmd].tier == 2)
	    {
		sprintf( buf, "%-12s", cmd_table[cmd].name );
		send_to_char( buf, ch );
		if ( ++col % 6 == 0 )
		    send_to_char( "\n\r", ch );
	    } else if (ch->class >= MAX_CLASS/2)
	    {
		sprintf( buf, "%-12s", cmd_table[cmd].name );
		send_to_char( buf, ch );
		if ( ++col % 6 == 0 )
		    send_to_char( "\n\r", ch );
	    } else if (ch->level >= LEVEL_HERO)
	    {
		sprintf( buf, "%-12s", cmd_table[cmd].name );
		send_to_char( buf, ch );
		if ( ++col % 6 == 0 )
		    send_to_char( "\n\r", ch );
	    } */
	    sprintf( buf, "%-12s", cmd_table[cmd].name );
	    send_to_char( buf, ch );
	    if ( ++col % 6 == 0 )
		send_to_char( "\n\r", ch );
	}
    }
 
    if ( col % 6 != 0 )
	send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char lcode;
    int cmd;
    int col;

    lcode = 'z';
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level == MAX_LEVEL - 8 ) lcode = 'W';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 7 )
		lcode = 'c';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 6 )
		lcode = 'y';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 5 )
		lcode = 'g';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 4 )
		lcode = 'Y';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 3 )
		lcode = 'M';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 2 )
		lcode = 'B';
        else if ( cmd_table[cmd].level == MAX_LEVEL - 1 )
		lcode = 'G';
        else if ( cmd_table[cmd].level == MAX_LEVEL ) 
        	lcode = 'R';
        else    
        	lcode = 'C';
        
        if ( cmd_table[cmd].level >= LEVEL_HERO
        //&&   cmd_table[cmd].level <= get_trust( ch )
        &&   cmd_table[cmd].show
        &&   is_granted(ch,cmd_table[cmd].do_fun) )
        {
            sprintf( buf, "{g[{%c%i{g]{c%-13s", lcode,
cmd_table[cmd].level, cmd_table[cmd].name );
            send_to_char( buf, ch );
            if ( ++col % 4 == 0 )
                send_to_char( "{x\n\r", ch );
        }
    }

    if ( col % 4 != 0 )
        send_to_char( "{x\n\r", ch );
    return;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 * This Version will return text as is typed in - Skyn
 */
char *grab_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
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
        *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}
