/**************************************************************************r
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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "clan.h"


/* command procedures needed */
DECLARE_DO_FUN(do_return	);
bool is_clan(CHAR_DATA *ch);
bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim);
bool is_class_restricted(CHAR_DATA *ch, OBJ_DATA *obj);
int focus_level args ((long total));
int focus_str args ((CHAR_DATA *ch));
int focus_int args ((CHAR_DATA *ch));
int focus_wis args ((CHAR_DATA *ch));
int focus_dex args ((CHAR_DATA *ch));
int focus_con args ((CHAR_DATA *ch));
int check_weapon_handling args( (CHAR_DATA *ch, sh_int sn) );
void clan_entry_trigger args((CHAR_DATA *ch, sh_int clan));
char * get_descr_form args((CHAR_DATA *ch,CHAR_DATA *looker, bool get_long));

/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );


/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    
    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;
     
    if (IS_SET(ch->off_flags,ASSIST_ALIGN)
    &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim))
    ||	 (IS_EVIL(ch) && IS_EVIL(victim))
    ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
	return TRUE;

    return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     
/* returns material number */
int material_lookup (const char *name)
{
    return 0;
}

/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
}

/* returns pcrace number */
int pc_race_lookup (const char *name)
{
   int race;

   for ( race = 0; pc_race_table[race].name != NULL; race++)
   {
        if (LOWER(name[0]) == LOWER(pc_race_table[race].name[0])
        &&  !str_prefix( name,pc_race_table[race].name))
	    return race;
   }

   return 0;
} 


int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
	    return liq;
    }

    return 0; /* -1 is not part of the liq_table.. CRASH! */
}

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
 
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
        &&  !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    }
 
    return WEAPON_EXOTIC;
}


int item_lookup(const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(item_table[type].name[0])
        &&  !str_prefix(name,item_table[type].name))
            return item_table[type].type;
    }
 
    return -1;
}

char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

char *weapon_name( int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

int attack_lookup  (const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* returns class number */
int class_lookup (const char *name)
{
   int class;
 
   for ( class = 0; class < MAX_CLASS; class++)
   {
        if (LOWER(name[0]) == LOWER(class_table[class].name[0])
        &&  !str_prefix( name,class_table[class].name))
            return class;
   }
 
   return -1;
}

/* returns damage type */
char *dt_name(int dt_type)
{
    char buf[100];

    if(skill_table[dt_type].name != NULL)
	return skill_table[dt_type].name;
   	    
    sprintf(buf,"dt_type not found: %d",dt_type);
    wiznet(buf,NULL,NULL,WIZ_DEBUG,0,0);
    return "attack";
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if(IS_SHIELDED(ch,SHD_PROTECT_AURA))
    return IS_IMMUNE;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    // If you add a damtype here, make sure to change the number in the for loop for mana blast - Fesdor
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
	case(DAM_CHARM):	bit = IMM_CHARM;	break;
	case(DAM_SOUND):	bit = IMM_SOUND;	break;
	default:		return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}

bool is_clan(CHAR_DATA *ch)
{
    return ch->clan;
}

bool is_clead(CHAR_DATA *ch)
{
    return ch->clead;
}

bool is_pkill(CHAR_DATA *ch)
{
    if (is_clan(ch) )
        return clan_table[ch->clan].pkill;
    else
        return FALSE;
}

/* New Clans Pkill Check
bool is_pkill(CHAR_DATA *ch)
{
    CLN_DATA *nclan;

    if (is_nclan(ch) ) {
    nclan = cln_lookup(ch->cln);
     return nclan->independent;
    }
    else
        return FALSE;
}*/

bool can_pkill(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if ( ch == NULL || victim == NULL )
	return FALSE;

   if(!is_pkill(ch) || !is_pkill(victim))
     return FALSE;

   if (IS_SET (victim->act, PLR_TWIT))
    return TRUE;

   if(victim->pcdata->tier < 2 && ch->pcdata->tier > 2)
   {
     send_to_char("Pick on someone your own size.\n\r",ch);
     return FALSE;
   }

   if(ch->pcdata->tier < 2 && victim->pcdata->tier > 2)
   {
     send_to_char("They would crush you in moments, better advance first.\n\r",ch);
     return FALSE;
   }

   if (ch->level > victim->level + 10)
   {
     send_to_char ("Pick on someone your own size.\n\r", ch);
     return FALSE;
   }

   if (ch->level < victim->level - 10)
   {
     send_to_char ("Pick on someone your own size.\n\r", ch);
     return FALSE;
   }
//Rank Limitations
   /*if (ch->pcdata->prank > (victim->pcdata->prank + 200)
	|| ch->pcdata->prank < (victim->pcdata->prank - 200) )
   {
     send_to_char("Why don't you try someone a little closer to your skill level.\n\r",ch);
     return FALSE;
   }*/

   if(IS_SET(ch->plyr,PLAYER_NEWBIE)) {
	if(IS_SET(victim->plyr,PLAYER_NEWBIE)) {
		return TRUE; 
	} else { 
	send_to_char("Newbie's shouldn't try to start fights with non-newbies. Denied.\n\r",ch);
	return FALSE; }
	}

   if(IS_SET(victim->plyr,PLAYER_NEWBIE)) {
	if(IS_SET(ch->plyr,PLAYER_NEWBIE)) {
		return TRUE;
	} else { 
	send_to_char("Why are you picking on newbies?\n\r",ch);
	return FALSE; }
	}

   return TRUE;
}

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (clan_table[ch->clan].independent)
	return FALSE;
    if (ch->clan == victim->clan)
    return TRUE;
    return FALSE;
}

bool is_clan_obj(OBJ_DATA *obj)
{
    return obj->clan;
}

bool is_clan_obj_ind(OBJ_INDEX_DATA *obj)
{
    return obj->clan;
}

bool is_class_obj(OBJ_DATA *obj)
{
    return obj->class;
}

bool clan_can_use(CHAR_DATA *ch, OBJ_DATA *obj)
{
    int clan;
    clan = ch->clan;
    if (!is_clan_obj(obj))
	return TRUE;
    if (clan == obj->clan)
        return TRUE;
    return FALSE;
}

bool class_can_use(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (!is_class_obj(obj))
	return TRUE;
    return (ch->class == obj->class);
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
#define PUCK_DEBUG

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;
#ifdef PUCK_DEBUG
    char lbuf[256];
#endif

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 5 / 2;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
#ifdef PUCK_DEBUG
        if (wears_debug(ch))
        {
            sprintf(lbuf,"---> Bad sn %d in get_skill\n",sn);
            send_to_char(lbuf,ch);
        }
#endif
    }

    else if (!IS_NPC(ch))
    {
            if(!has_skill(ch,sn))
             skill = 0;
            else
             skill = ch->pcdata->learned[sn];
    }

    else /* mobiles */
    {
#ifdef PUCK_DEBUG
        if (wears_debug(ch))
	    send_to_char("---> get_skill thinks you're a MOB.\r\n",ch);
#endif

        if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + ch->level;

	else if (sn == gsn_hide)
	{
	if (IS_SET(ch->act,ACT_THIEF))
            skill = ch->level + 20;
	else
	    skill = 0;
	}
	else if (sn== gsn_sneak)
            skill = ch->level + 20;

        else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
        ||       (sn == gsn_phase && IS_SET(ch->off_flags,OFF_PHASE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level;

 	else if (sn == gsn_shield_block)
	    skill = 10 + ch->level;

 	else if (sn == gsn_gouge && IS_SET(ch->act,ACT_THIEF))
	    skill = 10 + ch->level;

	else if (sn == gsn_second_attack 
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)
	|| IS_SET(ch->act,ACT_RANGER) || IS_SET(ch->act,ACT_DRUID)))
	    skill = ch->level + 50;

	else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = ch->level + 30;

	else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = ch->level;

	else if (sn == gsn_fifth_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = ch->level - 10;

	else if (sn == gsn_hand_to_hand)
	    skill = 40 + ch->level;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * (ch->level/2);

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * (ch->level/2);

 	else if (sn == gsn_stun && IS_SET(ch->off_flags,OFF_BASH)
	  && IS_SET(ch->act,ACT_WARRIOR))
	    skill = ch->level/2;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->act,ACT_WARRIOR)
	     ||	  IS_SET(ch->act,ACT_THIEF)
	     ||   IS_SET(ch->act,ACT_VAMPIRE)))
	    skill = 20 + 3 * (ch->level/2);

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * (ch->level/2);

	else if (sn == gsn_kick)
	    skill = 10 + 3 * (ch->level/2);

	else if (sn == gsn_backstab && (IS_SET(ch->act,ACT_THIEF)
	|| IS_SET(ch->act,ACT_RANGER) || IS_SET(ch->act,ACT_VAMPIRE)
	|| IS_SET(ch->off_flags,OFF_BACKSTAB)))
	    skill = 20 + ch->level;

	else if (sn == gsn_dual_wield && (IS_SET(ch->act,ACT_THIEF)
	|| IS_SET(ch->act,ACT_RANGER) || IS_SET(ch->act,ACT_WARRIOR)))
	    skill = 20 + ch->level;

	else if (sn == gsn_circle && IS_SET(ch->act,ACT_THIEF))
	    skill = 10 + ch->level;

  	else if (sn == gsn_rescue)
	    skill = ch->level/4;

	else if (sn == gsn_recall)
	    skill = 40 + (ch->level/2);

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm)
	    skill = 40 + 5 * (ch->level/2) / 2;

        else if(sn == gsn_second_attack || sn == gsn_third_attack)
         skill = ch->level/2;
        else if(sn == gsn_fourth_attack || sn == gsn_fifth_attack)
         skill = ch->level/3;
	else
	   skill = 0;
    }

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
#ifdef PUCK_DEBUG
        if (wears_debug(ch))
        {
            sprintf(lbuf,"---> daze=%d, adjusted skill=%d\r\n",
                ch->daze, skill );
            send_to_char(lbuf,ch);
        }
#endif
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
        skill = 4 * skill / 5;
        
    if(skill >= 100 && !IS_NPC(ch))
    {
     skill += check_weapon_handling(ch,sn);
     return skill;
    }    

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
        case(WEAPON_SWORD):     sn = gsn_sword;         break;
        case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
        case(WEAPON_SPEAR):     sn = gsn_spear;         break;
        case(WEAPON_MACE):      sn = gsn_mace;          break;
        case(WEAPON_AXE):       sn = gsn_axe;           break;
        case(WEAPON_FLAIL):     sn = gsn_flail;         break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
        case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
		default :               sn = -1;                break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    if(skill >= 100)
    {
     skill += check_weapon_handling(ch,sn);
     return skill;
    }

    return URANGE(0,skill,100);
} 

int check_weapon_handling(CHAR_DATA *ch, sh_int sn)
{
  sh_int wsn = -1;
  sh_int skill;

  if(IS_NPC(ch))
  return 0;

  if(sn == gsn_sword)
  wsn = gsn_sword2;
  else if(sn == gsn_axe)
  wsn = gsn_axe2;
  else if(sn == gsn_dagger)
  wsn = gsn_dagger2;
  else if(sn == gsn_polearm)
  wsn = gsn_polearm2;
  else if(sn == gsn_whip)
  wsn = gsn_whip2;
  else if(sn == gsn_spear)
  wsn = gsn_spear2;
  else if(sn == gsn_mace)
  wsn = gsn_mace2;
  else if(sn == gsn_flail)
  wsn = gsn_flail2;
  else if(sn == gsn_dual_wield)
  wsn = gsn_dual_wield2;

  if(wsn == -1)
  return 0;

  if(has_skill(ch,wsn))
  {
  skill = ch->pcdata->learned[wsn]/10 + ch->level/5;
  return skill;
  }
  
  return 0;
}


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat,morph;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;
     if (IS_NPC(ch))
	return;
     morph = 0;
    if (ch->pcdata->perm_hit == 0 
    ||	ch->pcdata->perm_mana == 0
    ||  ch->pcdata->perm_move == 0
    ||	ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:	ch->max_mana	-= mod;		break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_MOVE:	ch->max_move	-= mod;		break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_MANA:    ch->max_mana    -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_MOVE:    ch->max_move    -= mod;         break;
                }
            }
	}

    /* remove spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  -= mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  -= mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  -= mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  -= mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  -= mod; break;
                case APPLY_SEX:         ch->sex                 -= mod; break;
                case APPLY_MANA:        ch->max_mana            -= mod; break;
                case APPLY_HIT:         ch->max_hit             -= mod; break;
                case APPLY_MOVE:        ch->max_move            -= mod; break;
                case APPLY_HEIGHT:      ch->size                -= mod; break;

                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] -= mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             -= mod; break;
                case APPLY_DAMROLL:     ch->damroll             -= mod; break;
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw -= mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw -= mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw -= mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw -= mod; break;
                case APPLY_MORPH_FORM:          ch->morph_form[0] = 0; break; // -= mod; break;
                case APPLY_REGEN:        ch->regen_rate -= mod; break;
		case APPLY_MANA_REGEN:	ch->mana_regen_rate -= mod; break;
                case APPLY_AGE:           ch->eq_age                -= mod; break;
        } 
    }

	/* now reset the permanent stats */
	ch->pcdata->perm_hit 	= ch->max_hit;
	ch->pcdata->perm_mana 	= ch->max_mana;
	ch->pcdata->perm_move	= ch->max_move;
	ch->pcdata->last_level	= ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
        {
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
        }
    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->saving_throw	= 0;
    ch->regen_rate = 0;
    ch->mana_regen_rate = 0;
    ch->eq_age             = 0;
    ch->morph_form[0]      = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( ch->level, obj, loc, i );

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
		case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod;	break;
		case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod; break;
		case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod; break;
		case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod; break;
		case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod; break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
	
		case APPLY_SAVES:		ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD: 		ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:	ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH: 	ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:	ch->saving_throw += mod; break;
                case APPLY_REGEN:          ch->regen_rate += mod; break;
		case APPLY_MANA_REGEN:		ch->mana_regen_rate += mod; break;
                case APPLY_AGE:           ch->eq_age                += mod; break;
                case APPLY_MORPH_FORM:
                      /* morph = ch->morph_form[0];
                       morph += mod;
                       if (morph > MORPH_MAX)
                            mod = MORPH_MAX;
                       if (morph < 0)
                            mod = 0;
                       if (is_affected(ch, gsn_conceal)) 
                            mod = MORPH_CONCEAL;*/
                       ch->morph_form[0] = mod;
                       break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_REGEN:          ch->regen_rate += mod; break;
		case APPLY_MANA_REGEN:	ch->mana_regen_rate += mod; break;
                case APPLY_AGE:           ch->eq_age                += mod; break;
                case APPLY_MORPH_FORM:
                      /* morph = ch->morph_form[0];
                       morph += mod;
                       if (morph > MORPH_MAX)
                            mod = MORPH_MAX;
                       if (morph < 0)
                            mod = 0;
                       if (is_affected(ch, gsn_conceal)) 
                            mod = MORPH_CONCEAL;*/
                       ch->morph_form[0] = mod;
                       break;
            }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
                case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
                case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
                case APPLY_REGEN:          ch->regen_rate += mod; break;
		case APPLY_MANA_REGEN:	ch->mana_regen_rate += mod; break;
                case APPLY_AGE:           ch->eq_age                += mod; break;
                case APPLY_MORPH_FORM:
                     /*  morph = ch->morph_form[0];
                       morph += mod;
                       if (morph > MORPH_MAX)
                            mod = MORPH_MAX;
                       if (morph < 0)
                            mod = 0;
                       if (is_affected(ch, gsn_conceal)) 
                            mod = MORPH_CONCEAL;*/
                       ch->morph_form[0] = mod;
                       break;
        } 
    }

    if (ch->pcdata->focus[MAGIC_DEFENSE])
	    ch->saving_throw -= focus_level(ch->pcdata->focus[MAGIC_DEFENSE]);

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
    ch->dam_type = 17; //punch
    update_class_bits(ch);
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if ( ch->trust != 0 /*&& IS_SET(ch->comm,COMM_TRUE_TRUST)*/)
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return (17+(ch->played + (int) (current_time-ch->logon))/72000)+ch->eq_age;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;
    int mod=0;

    if ( IS_NPC(ch) )
        max = MAX_STAT - 6;

    if ( ch->level > LEVEL_IMMORTAL )
	max = MAX_STAT - 1;

    else
    {
	max = pc_race_table[ch->race].max_stats[stat] + 4;

	if (class_table[ch->class].attr_prime == stat /*&& ch->race != race_lookup("human")*/ )
	    max += 2;

	/*	Let's take this out for now, Humans are good enough. -Fesdor
	if ( ch->race == race_lookup("human"))
	    max += 1;
	    */

	// Faith for Crusader breeness1 
        if ( ch->class == CLASS_CRUSADER && (stat == 0 || stat == 3) )
		max += ch->alignment/300;
 	max = UMIN(max,25);
    }

    if (stat==STAT_STR)
	mod += focus_str(ch);
    if (stat==STAT_INT)
	mod += focus_int(ch);
    if (stat==STAT_WIS)
	mod += focus_wis(ch);
    if (stat==STAT_DEX)
	mod += focus_dex(ch);
    if (stat==STAT_CON)
	mod += focus_con(ch);

    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat] + mod, max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
        return 30;

    max = pc_race_table[ch->race].max_stats[stat];
    if (class_table[ch->class].attr_prime == stat)
    {
	if (ch->race == race_lookup("human"))
	   max += 3;
	else
	   max += 2;
    }
    return UMIN(max,30);
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + ch->level + 10;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 15;
}



/*
 * See if a string is one of the names of an object.
 */
/*
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}
*/

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}


bool is_exact_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;
 
    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
        str = one_argument(str,part);
 
        if (part[0] == '\0' )
            return TRUE;
 
        /* check to see if this is part of namelist */
        list = namelist;
        for ( ; ; )  /* start parsing namelist */
        {
            list = one_argument(list,name);
            if (name[0] == '\0')  /* this name was not found */
                return FALSE;
 
            if (!str_cmp(string,name))
                return TRUE; /* full pattern match */
 
            if (!str_cmp(part,name))
                break;
        }
    }
}      

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}
           

void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i; // ,morph; Sorn-unused variable

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_AFFECTS2:
        SET_BIT(ch->affected2_by, paf->bitvector);
        break;

	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	case TO_SHIELDS:
	    SET_BIT(ch->shielded_by, paf->bitvector);
	    break;
	}
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case TO_AFFECTS2:
                REMOVE_BIT(ch->affected2_by, paf->bitvector);
                break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        case TO_SHIELDS:
            REMOVE_BIT(ch->shielded_by, paf->bitvector);
            break;
        }
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]	+= mod;	break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:	      ch->eq_age		+= mod;	break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SAVES:   ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_PETRI:  ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_throw		+= mod;	break;
    case APPLY_SPELL_AFFECT:  					break;
    case APPLY_REGEN:          ch->regen_rate += mod; break;
    case APPLY_MORPH_FORM:
                      /* morph = ch->morph_form[0];
                       morph += mod;
                       if (morph > MORPH_MAX)
                            mod = MORPH_MAX;
                       if (morph < 0)
                            mod = 0;
                       if (is_affected(ch, gsn_conceal)) 
                            mod = MORPH_CONCEAL;*/
                       ch->morph_form[0] = mod;
                       break;
   case APPLY_MANA_REGEN:	ch->mana_regen_rate += mod; break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	 //   obj_to_room( wield, ch->in_room );
	    obj_to_char( wield, ch );
	    depth--;
	}
    }
    reset_char(ch);
    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
	        case TO_AFFECTS2:
                    SET_BIT(ch->affected2_by, vector);
                    break;
		    case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
	        case TO_SHIELDS:
		    SET_BIT(ch->shielded_by,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by, vector);
						break;
                        case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                  	break;
                    case TO_SHIELDS:
                        SET_BIT(ch->shielded_by,vector);
                        break;
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
						SET_BIT(ch->affected2_by, vector);
						break;
                        case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                    case TO_SHIELDS:
                        SET_BIT(ch->shielded_by,vector);
                        break;
                }
		reset_char(ch);
                return;
            }
    }
}

/*
 * Turns a cooldown on.
 */
void cooldown_on( CHAR_DATA *ch, COOLDOWN_DATA *pcd )
{
    COOLDOWN_DATA *pcd_new;

    pcd_new = new_cooldown();

    *pcd_new		= *pcd;
    pcd_new->next	= ch->cooldowns;
    ch->cooldowns	= pcd_new;

    return;
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }
    

    return;
}

/*
 * Give an affect to a room.
 */
void affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    paf_new->next       = room->affected;
    room->affected        = paf_new;

    /* apply any affect vectors to the room's affected_by */
    if (paf->bitvector)
     SET_BIT(room->affected_by, paf->bitvector);
    return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
	ch->affected	= paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    reset_char(ch);
    return;
}

/*
 * Remove a cooldown from a char.
 */
void cooldown_remove( CHAR_DATA *ch, COOLDOWN_DATA *pcd )
{
    if ( ch->cooldowns == NULL )
    {
	/* bug( "Cooldown_remove: no cooldowns.", 0 ); Bug line removed 
due to log flooding */
	return;
    }


    if ( pcd == ch->cooldowns )
    {
	ch->cooldowns	= pcd->next;
    }
    else
    {
	COOLDOWN_DATA *prev;

	for ( prev = ch->cooldowns; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pcd )
	    {
		prev->next = pcd->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Cooldown_remove: cannot find pcd.", 0 );
	    return;
	}
    }

    free_cooldown(pcd);

    reset_char(ch);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}

void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    int where, vector;
    if ( room->affected == NULL )
    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
     REMOVE_BIT(room->affected_by, paf->bitvector);

    if ( paf == room->affected )
    {
        room->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = room->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_room: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }
    reset_char(ch);
    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Return true if a char has a skill on cooldown.
 */
bool on_cooldown( CHAR_DATA *ch, int sn )
{
    COOLDOWN_DATA *pcd;

    for ( pcd = ch->cooldowns; pcd != NULL; pcd = pcd->next )
    {
	if ( pcd->type == sn )
	    return TRUE;
    }

    return FALSE;
}

bool is_obj_affected( OBJ_DATA *obj, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}


/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
/*	Old way
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
 */

	    paf->level = UMAX(paf->level, paf_old->level);
            /* Sunder Check by Dusk 10/10/08 Its so the duration won't increase but
               modifier will */
            //if ( paf->type != gsn_sunder )
	       //paf->duration += paf_old->duration;
	    if ((paf->bitvector == AFF_POISON) || (paf->bitvector == AFF_PLAGUE))
		paf->modifier = UMAX(-20, (paf->modifier + paf_old->modifier));
	    else
		paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    reset_char(ch);
    return;
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  /* sanity check! */
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;
    int clan;

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	bug( "Char_to_room: NULL.", 0 );
	
	if ( ch->alignment < 0 )
	{
	    if ((room = get_room_index(ROOM_VNUM_TEMPLEB)) != NULL)
		char_to_room(ch,room);
	}
	else
	{
	    if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
		char_to_room(ch,room);
	}
	
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;

    /* Check for Clan guards */
    for(clan = 0; clan < MAX_CLAN; clan++)
     clan_entry_trigger(ch,clan);
	
    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }
        
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->level == 1)
            return;
        
	plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1; 
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_CON;
        plague.modifier 	= number_range(1,4)*-1;
        plague.bitvector 	= AFF_PLAGUE;
        
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }
    
    //Insert Clan Entry Triggers Here.
    //ancient_entry_trigger(ch);
    reset_char(ch);
    return;
}


/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;

    ch->carry_number	  +=  get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    /*int i;*/

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }
    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    // Lets start keeping track of previous owners
   /* for(i=0; i < 5; i++ )
    {
    	if(obj->prev_owner[i] != NULL)
    	continue;
    	// Only if its not the same as the previous owner
    	if(i > 0 && str_cmp(obj->prev_owner[i-1], ch->name))
    	 obj->prev_owner[i] = str_dup(ch->name);
    }*/
    
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( int level, OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    if ( obj->clan )
    {
	switch ( type )
	{
	case AC_PIERCE:	return ((level/7)+5);
	case AC_BASH:	return ((level/7)+5);
	case AC_SLASH:	return ((level/7)+5);
	case AC_EXOTIC:	return (level/7);
	}
	return 0;
    }

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return     obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return (obj->value[type] * 5) / 2; // raised to 2.5
    case WEAR_FINGER_L:	return     obj->value[type];
    case WEAR_FINGER_R: return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_FLOAT:    return     obj->value[type];               
    case WEAR_FACE:     return     obj->value[type];
    case WEAR_EAR:      return     obj->value[type];
    case WEAR_ANKLE_L:  return     obj->value[type];
    case WEAR_ANKLE_R:  return     obj->value[type];
    case WEAR_CLAN:     return     obj->value[type];
    case WEAR_RELIG:    return     obj->value[type];
    case WEAR_PATCH:    return     obj->value[type];
    case WEAR_BACK:     return     obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;
//    char buf[MSL];

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    /*
     * Make the following checks only for mortals.
     * -walsh
     */
    if (ch->level < LEVEL_IMMORTAL)
    {
	if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) )
	||   ( !IS_NPC(ch) && !clan_can_use(ch, obj) )
	||   ( !IS_NPC(ch) && !class_can_use(ch, obj) )
        ||    !is_class_restricted(ch,obj) )
	{
	    /*
	     * Thanks to Morgenes for the bug fix here!
	     */
// sprintf(buf,"clan_can_use: %d, class_can_use: %d, class_restrict: %d \n\r",clan_can_use(ch, obj),class_can_use(ch,obj),is_class_restricted(ch,obj));
// send_to_char(buf,ch);
	    act( "{RYou are {Yzapped {Rby $p and drop it.{x", ch, obj, NULL, TO_CHAR );
	    act( "{R$n is {Yzapped {Rby $p and drops it.{x",  ch, obj, NULL, TO_ROOM );
	    obj_from_char( obj );
	    obj_to_room( obj, ch->in_room );
	    return;
	}
    }

    if (IS_OBJ_STAT(obj,ITEM_QUESTPOINT) && !IS_NPC(ch))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	ch->qps++;
	act("{YYou gained a {RQuest Point {Yfrom $p!{x",ch,obj,NULL,TO_CHAR);
	act("{Y$n gained a {RQuest Point {Yfrom $p!{x",ch,obj,NULL,TO_ROOM);
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( ch->level, obj, iWear,i );
    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
            if ( paf->location != APPLY_SPELL_AFFECT && paf->location != APPLY_MORPH_FORM)
	        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
        if ( paf->location == APPLY_SPELL_AFFECT || paf->location == APPLY_MORPH_FORM)
    	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    reset_char(ch);
    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( ch->level, obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT || paf->location == APPLY_MORPH_FORM)
	    {
	        for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	        {
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
		        (lpaf->level == paf->level) &&
		        (lpaf->location == APPLY_SPELL_AFFECT))
		    {
		        affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
	        }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
     }
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT || paf->location == APPLY_MORPH_FORM)
	{
	    bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    // Anonymous -- Jair's request :)
    if (ch->hit > ch->max_hit)
    {
	ch->hit = ch->max_hit;
	update_pos(ch);
    }
	reset_char(ch);
    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{

    if(pRoomIndex == NULL) {
    bug("OBJ_TO_ROOM: pRoomIndex NULL POINTER",0);
    return;
    }
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;

    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->item_type == ITEM_PIT)
        obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL. Attempting fix", 0 );
	char_to_room(ch,get_room_index(ROOM_VNUM_LIMBO));
/*
 *	return;
 */
    }
    
    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )

	die_follower( ch );
    
    stop_fighting( ch, TRUE );

	// remove stuff, except for arena people
	if (!IN_ARENA(ch))
	{
	    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	    {
			obj_next = obj->next_content;
			extract_obj( obj );
	    }
	}
    
    /* People who die in the arena go to arena morgue */
    if (( !fPull ) &&
        (IS_SET(ch->in_room->room_flags,ROOM_ARENA)))
    {
        char_from_room( ch );
        char_to_room(ch,get_room_index(ROOM_ARENA_MORGUE));
        return;
    }

    char_from_room( ch );

    /* Death room is set in the clan table now */
    if ( !fPull )
    {
        char_to_room(ch,get_room_index(clan_table[ch->clan].deathroom));
	return;
    }

    if ( IS_NPC(ch) ) {
	    if(ch->pIndexData->count == 0) { }
	    else{ --ch->pIndexData->count; }
    }

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
	if ( ch->mprog_target == wch )
	    wch->mprog_target = NULL;
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;
    char buf[MIL];
    
    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( !can_see( ch, rch ) || get_trust(ch) < rch->ghost_level)
            continue;    
	
        if (rch->morph_form[0] != 0)
        {
            if (rch->morph_form[0] >= 5 && rch->morph_form[0] <= 9 &&
            !str_cmp(arg,"dragon"))
	    {
		if (++count == number)
			return rch;
	    }
            else if (rch->morph_form[0] == 16 && !str_cmp(arg,"goat"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == 17 && !str_cmp(arg,"lion"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == 18 && !str_cmp(arg,"dragon"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_BAT && !str_cmp(arg,"bat"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_WOLF && !str_cmp(arg,"wolf"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_BEAR && !str_cmp(arg,"bear"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == MORPH_MORPH && !str_prefix(arg,rch->morph))
            {
             if (++count == number)
             return rch;
            }
            else if(!str_prefix(arg,(get_name(rch,NULL,buf))))
            {
             if (++count == number)
             return rch;
            }
        }
        if ( (arg[0] != '\0') && !is_name(arg,get_name(rch,ch,buf)) )
            continue;
        if ( ++count == number )
            return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_seen_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch, *rch;
    int number;
    int count;
    char buf[MIL];
    
    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
     if ( !str_cmp( arg, "self" ) )
         return ch;
     for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
     {
        if ( !can_see( ch, rch ) || get_trust(ch) < rch->ghost_level)
            continue;     
        if (rch->morph_form[0] != 0)
        {
            if (rch->morph_form[0] >= 5 && rch->morph_form[0] <= 9 &&
            !str_cmp(arg,"dragon"))
	    {
		if (++count == number)
			return rch;
	    }
            else if (rch->morph_form[0] == 16 && !str_cmp(arg,"goat"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == 17 && !str_cmp(arg,"lion"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == 18 && !str_cmp(arg,"dragon"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_BAT && !str_cmp(arg,"bat"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_WOLF && !str_cmp(arg,"wolf"))
	    {
		if (++count == number)
		return rch;
	    }
	    else if (rch->morph_form[0] == MORPH_BEAR && !str_cmp(arg,"bear"))
	    {
		if (++count == number)
		return rch;
	    }
            else if (rch->morph_form[0] == MORPH_MORPH && !str_prefix(arg,rch->morph))
            {
             if (++count == number)
             return rch;
            }
            else if(!str_prefix(arg,(get_name(rch,NULL,buf))))
            {
             if (++count == number)
             return rch;
            }
         }
         if ( (arg[0] != '\0') && !is_name(arg,get_name(rch,ch,buf)) )
            continue;
        if ( ++count == number )
            return rch;
      }
    }
    return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || !can_see( ch, wch ) 
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find a char in the world, no level check.
 */
CHAR_DATA *get_char_mortal( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;

    one_argument( argument, arg );
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if (!str_cmp(arg, wch->name)
        &&  !IS_NPC(wch)
        &&  !IS_IMMORTAL(wch) )
            return wch;
    }

    return NULL;
}


/*
 * Find a char in the world, Imms too.
 */
CHAR_DATA *get_char_anyone( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;

    one_argument( argument, arg );
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
        if (!str_cmp(arg, wch->name)
        &&  !IS_NPC(wch))
            return wch;
    }

    return NULL;
}





/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->item_type == ITEM_EXIT) && is_name(arg, obj->name) )
	{
	    if ( ++count == number )
		return obj;
	}

	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an exit obj in a list. (no ch)
 */
OBJ_DATA *get_obj_exit( char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( (obj->item_type == ITEM_EXIT) && is_name(arg, obj->name) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in a list. (no ch)
 */
OBJ_DATA *get_obj_item( char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   (can_see_obj( ch, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}


void deduct_cost( CHAR_DATA *ch, int cost, int value )
{
    while ( ch->silver >= 100 )
    {
	ch->gold++;
	ch->silver -= 100;
    }

    while ( ch->gold >= 100 )
    {
	ch->platinum++;
	ch->gold -= 100;
    }

    switch( value )
    {
	case VALUE_PLATINUM:
	    ch->platinum -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }
	    break;

	case VALUE_GOLD:
	    while ( ch->gold < cost )
	    {
		ch->platinum--;
		ch->gold += 100;
	    }

	    ch->gold -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }

	    if ( ch->gold < 0 )
	    {
		bug( "deduct costs: gold %d < 0", ch->gold );
		ch->gold = 0;
	    }
	    break;

	case VALUE_SILVER:
	    while ( ch->silver < cost )
	    {
		ch->gold--;
		ch->silver += 100;

		if ( ch->gold < 0 )
		{
		    ch->platinum--;
		    ch->gold += 100;
		}
	    }

	    ch->silver -= cost;
	    if ( ch->platinum < 0 )
	    {
		bug( "deduct costs: platinum %d < 0", ch->platinum );
		ch->platinum = 0;
	    }

	    if ( ch->gold < 0 )
	    {
		bug( "deduct costs: gold %d < 0", ch->gold );
		ch->gold = 0;
	    }

	    if ( ch->silver < 0 )
	    {
		bug( "deduct costs: silver %d < 0", ch->silver );
		ch->silver = 0;
	    }
	    break;

	case VALUE_AQUEST_POINT:
	    ch->questpoints -= cost;
	    break;

	case VALUE_IQUEST_POINT:
	    ch->qps -= cost;
	    break;

	default:
	    bug( "deduct costs: invalid value.", 0 );
	    break;
    }
}

void add_cost( CHAR_DATA *ch, int cost, int value )
{
    while ( ch->silver >= 100 )
    {
	ch->gold++;
	ch->silver -= 100;
    }

    while ( ch->gold >= 100 )
    {
	ch->platinum++;
	ch->gold -= 100;
    }

    switch ( value )
    {
	case VALUE_PLATINUM:
	    ch->platinum += cost;
	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_GOLD:
	    ch->gold += cost;
	    while ( ch->gold >= 100 )
	    {
		ch->platinum++;
		ch->gold -= 100;
	    }

	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_SILVER:
	    ch->silver += cost;
	    while ( ch->silver >= 100 )
	    {
		ch->gold++;
		ch->silver -= 100;
	    }

	    while ( ch->gold >= 100 )
	    {
		ch->platinum++;
		ch->gold -= 100;
	    }

	    if ( ch->platinum > 50000 )
		ch->platinum = 50000;
	    break;

	case VALUE_AQUEST_POINT:
	    ch->questpoints+= cost;
	    break;

	case VALUE_IQUEST_POINT:
	    ch->qps += cost;
	    break;

	default:
	    bug( "deduct costs: invalid value.", 0 );
	    break;
    }
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int platinum, int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( platinum < 0 || gold < 0 || silver < 0
    || (platinum == 0 && gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(UMIN(platinum,gold),silver));
	platinum = UMAX(1,platinum);
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (platinum == 0 && gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (platinum == 0 && gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (platinum == 1 && gold == 0 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_PLATINUM_ONE), 0 );
    }
    else if (gold == 0 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_PLATINUM_SOME ), 0 );
        sprintf( buf, obj->short_descr, platinum );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[2]           = platinum;
        obj->cost               = platinum;
	obj->weight		= platinum * 3 / 5;
    }
    else if (platinum == 0 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold * 2 / 5;
    }
    else if (platinum == 0 && gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/10;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->value[2]		= platinum;
	obj->cost		= (10000 * platinum) + (100 * gold) + silver;
	obj->weight		= (platinum * 3 / 5)+(gold * 2 / 5)+(silver / 10);
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY
    ||  obj->item_type == ITEM_PIT)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private(CHAR_DATA *ch,  ROOM_INDEX_DATA *pRoomIndex)
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	if (!IS_NPC(rch))
	    count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_LOCKED)
    && IS_NPC(ch) && !IS_SET(ch->act, ACT_KEY) )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_LOCKED)
    &&  !IS_SET(ch->act, PLR_KEY) )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_HERO(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_LOCKED)
    &&  !IS_SET(ch->act, PLR_KEY) )
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 9 && !IS_HERO(ch))
	return FALSE;

  /*  if (!IS_IMMORTAL(ch) && pRoomIndex->clan && !ch->clan)
        return FALSE; */

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if(victim == NULL)
    {
	char buf[MSL];
	sprintf(buf,"handler.c: function can_see(): ch=%s, room=%s, victim is NULL!!\n\r", ch->name, ch->in_room->name);
	bug(buf, 0);
	return FALSE;
    }
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;
     
    if (get_trust(ch) < victim->invis_level)
        return FALSE;
    
    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if (get_trust(ch) < victim->ghost_level && ch->in_room == victim->in_room)
	return FALSE;

    if ( (!IS_NPC(victim) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(victim) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( is_same_clan(ch, victim) && !clan_table[ch->clan].independent)
	return TRUE;

    if ( IS_SHIELDED(victim, SHD_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_MOTION)
    &&   victim->fighting == NULL)
    {
	chance = get_skill(victim,gsn_sneak);
	chance += get_curr_stat(victim,STAT_DEX) * 3/2;
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if(IS_AFFECTED(victim,AFF_HIDE) && victim->fighting == NULL)
    {
     // chance is chance to stay hidden
     if(!IS_NPC(victim))
     chance = get_skill(victim,gsn_hide); // 100
     else
     chance = 60;

     chance = UMAX(chance,40); // 40

     chance += victim->level*2;
     chance -= ch->level*2;
     if (chance > 65) // 40
     chance = 65;
     
     // FINDME - acute vision shit
     if(is_affected(ch,gsn_acute_vision))
	chance -= dice(chance/3,chance); // 40 - 14 = 26
     else
	chance += 10;
     /*if(ch->clan == clan_lookup("order"))
	chance -= dice(1,chance);
*/
     if (number_percent() < chance)
        return FALSE;
    }

    /* Ranger stuff  -- Skyntil */
    if(is_affected(victim,gsn_forest_blend))
    {
     if(IS_AFFECTED(ch,AFF_DETECT_MOTION))
     {
      if(!IS_NPC(victim))
      chance = get_skill(victim,gsn_forest_blend)/2;
      else
      chance = 75;

      chance = UMAX(chance,40);

      chance += victim->level*2;
      chance -= ch->level*2;
      if (chance > 75)
      chance = 75;

      if (number_percent() < chance)
        return FALSE;
     }
     else {
     return FALSE;
     }
    }

    if(is_affected(victim,gsn_earthmeld))
    {
     if(IS_AFFECTED(ch,AFF_DETECT_MOTION))
     {
      if(!IS_NPC(victim))
      chance = get_skill(victim,gsn_earthmeld)/2;
      else
      chance = 85;

      chance = UMAX(chance,40);

      chance += victim->level*2;
      chance -= ch->level*2;
      if (chance > 85)
      chance = 85;

      if (number_percent() < chance)
        return FALSE;
     }
     return TRUE;
    }


    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if( obj == NULL )
     return FALSE;

    if ( obj->item_type == ITEM_EXIT)
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    // Can see humming pills now.
    if ( IS_AFFECTED( ch, AFF_BLIND ))
    {
      if (obj->item_type != ITEM_POTION && obj->item_type != ITEM_PILL)
        return FALSE;

      if (obj->item_type == ITEM_PILL && !IS_SET(obj->extra_flags,ITEM_HUM))
        return FALSE;
    }

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_DARK)
    &&   !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
    case ITEM_LIGHT:		return "light";
    case ITEM_SCROLL:		return "scroll";
    case ITEM_WAND:		return "wand";
    case ITEM_STAFF:		return "staff";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:		return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_CLOTHING:		return "clothing";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink container";
    case ITEM_KEY:		return "key";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc corpse";
    case ITEM_CORPSE_PC:	return "pc corpse";
    case ITEM_FOUNTAIN:		return "fountain";
    case ITEM_PILL:		return "pill";
    case ITEM_MAP:		return "map";
    case ITEM_PORTAL:		return "portal";
    case ITEM_WARP_STONE:	return "warp stone";
    case ITEM_GEM:		return "gem";
    case ITEM_JEWELRY:		return "jewelry";
    case ITEM_JUKEBOX:		return "juke box";
    case ITEM_DEMON_STONE:	return "demon stone";
    case ITEM_EXIT:		return "exit";
    case ITEM_PIT:		return "pit";
    case ITEM_SKELETON:         return "skeleton";
    case ITEM_SLOT_MACHINE:	return "slotmachine";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVES:		return "saves";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_SPELL_AFFECT:	return "none";
    case APPLY_REGEN:           return "regeneration";
    case APPLY_MORPH_FORM:      return "morph";
    case APPLY_MANA_REGEN:	return "mana regeneration";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_MOTION ) strcat( buf, " detect_motion" );
    if ( vector & AFF_FARSIGHT      ) strcat( buf, " farsight"      );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_DEAF          ) strcat( buf, " deaf"          );
    if ( vector & AFF_FEAR	    ) strcat( buf, " fear"	    );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_BERSERK	    ) strcat( buf, " berserk"	    );
    if ( vector & AFF_CALM	    ) strcat( buf, " calm"	    );
    if ( vector & AFF_HASTE	    ) strcat( buf, " haste"	    );
    if ( vector & AFF_SLOW          ) strcat( buf, " slow"          );
    if ( vector & AFF_PLAGUE	    ) strcat( buf, " plague" 	    );
    if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
    if ( vector & AFF_REGENERATION  ) strcat( buf, " regeneration"  );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Return ascii name of an affect2 bit vector.
 */
char *affect2_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Return ascii name of a shield bit vector.
 */
char *shield_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & SHD_PROTECT_VOODOO	) strcat( buf, " prot_voodoo"	);
    if ( vector & SHD_INVISIBLE		) strcat( buf, " invisible"	);
    if ( vector & SHD_ICE		) strcat( buf, " ice"		);
    if ( vector & SHD_FIRE		) strcat( buf, " fire"		);
    if ( vector & SHD_SHOCK		) strcat( buf, " shock"		);
    if ( vector & SHD_DEATH		) strcat( buf, " death"		);
    if ( vector & SHD_SANCTUARY		) strcat( buf, " sanctuary"	);
    if ( vector & SHD_PROTECT_EVIL	) strcat( buf, " prot_evil"	);
    if ( vector & SHD_PROTECT_GOOD	) strcat( buf, " prot_good"	);
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *ability_lich_name( int ability_lich_flags )
{
    static char buf[512];
 
    buf[0] = '\0';
    if ( ability_lich_flags & ABILITY_LICH_NIGHTSTALKER ) strcat( buf, " Nightstalker" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*char *ability_crusader_bit_name( int ability_crusader_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( ability_crusader_flags & ABILITY_CRUSADER_SUNDER ) strcat( buf, " Sunder" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}
*/
/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOPURGE	 ) strcat( buf, " nopurge"	);
    if ( extra_flags & ITEM_ROT_DEATH	 ) strcat( buf, " rot_death"	);
    if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " vis_death"	);
    if ( extra_flags & ITEM_NOSAC	 ) strcat( buf, " no_sac"	);
    if ( extra_flags & ITEM_NONMETAL	 ) strcat( buf, " non_metal"	);
    if ( extra_flags & ITEM_NOLOCATE	 ) strcat( buf, " no_locate"	);
    if ( extra_flags & ITEM_MELT_DROP	 ) strcat( buf, " melt_drop"	);
    if ( extra_flags & ITEM_HAD_TIMER	 ) strcat( buf, " had_timer"	);
    if ( extra_flags & ITEM_SELL_EXTRACT ) strcat( buf, " sell_extract" );
    if ( extra_flags & ITEM_BURN_PROOF	 ) strcat( buf, " burn_proof"	);
    if ( extra_flags & ITEM_NOUNCURSE	 ) strcat( buf, " no_uncurse"	);
    if ( extra_flags & ITEM_QUEST	 ) strcat( buf, " quest"	);
    if ( extra_flags & ITEM_FORCED	 ) strcat( buf, " forced"	);
    if ( extra_flags & ITEM_QUESTPOINT	 ) strcat( buf, " questpoint"	);
    if ( extra_flags & ITEM_CLAN         ) strcat( buf, " clan_item"    );
    if ( extra_flags & ITEM_NOENCHANT    ) strcat( buf, " no_enchant"   );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    { 
 	strcat(buf," npc");
    	if (act_flags & ACT_SENTINEL 	) strcat(buf, " sentinel");
    	if (act_flags & ACT_SCAVENGER	) strcat(buf, " scavenger");
	if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " aggressive");
	if (act_flags & ACT_STAY_AREA	) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY	) strcat(buf, " wimpy");
	if (act_flags & ACT_PET		) strcat(buf, " pet");
	if (act_flags & ACT_TRAIN	) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE	) strcat(buf, " practice");
	if (act_flags & ACT_UNDEAD	) strcat(buf, " undead");
	if (act_flags & ACT_CLERIC	) strcat(buf, " cleric");
	if (act_flags & ACT_MAGE	) strcat(buf, " mage");
	if (act_flags & ACT_THIEF	) strcat(buf, " thief");
	if (act_flags & ACT_WARRIOR	) strcat(buf, " warrior");
	if (act_flags & ACT_RANGER	) strcat(buf, " ranger");
	if (act_flags & ACT_DRUID	) strcat(buf, " druid");
	if (act_flags & ACT_VAMPIRE	) strcat(buf, " vampire");
	if (act_flags & ACT_NOALIGN	) strcat(buf, " no_align");
	if (act_flags & ACT_NOPURGE	) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER	) strcat(buf, " healer");
	if (act_flags & ACT_IS_SATAN	) strcat(buf, " evil_priest");
	if (act_flags & ACT_IS_PRIEST	) strcat(buf, " priest");
	if (act_flags & ACT_IS_BANKER	) strcat(buf, " banker");
	if (act_flags & ACT_NO_BODY     ) strcat(buf, " no_body");
        if (act_flags & ACT_NB_DROP     ) strcat(buf, " nobd_drop");
	if (act_flags & ACT_GAIN	) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
    }
    else
    {
	strcat(buf," player");
        if (act_flags & PLR_COLOUR	) strcat(buf, " colour");
	if (act_flags & PLR_AUTOASSIST	) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT	) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT	) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC	) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD	) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT	) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
	if (act_flags & PLR_CANLOOT	) strcat(buf, " loot_corpse");
	if (act_flags & PLR_NOSUMMON	) strcat(buf, " no_summon");
	if (act_flags & PLR_NOFOLLOW	) strcat(buf, " no_follow");
	if (act_flags & PLR_NOTRAN	) strcat(buf, " no_transport");
      if (act_flags & PLR_NOGOTO    ) strcat(buf, " no_goto");
      if (act_flags & PLR_NOVIOLATE ) strcat(buf, " no_violate");
	if (act_flags & PLR_NOCANCEL	) strcat(buf, " no_cancel");
	if (act_flags & PLR_FREEZE	) strcat(buf, " frozen");
	if (act_flags & PLR_TWIT	) strcat(buf, " twit");
	if (act_flags & PLR_NOEVOLVE	) strcat(buf, " no_evolve");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET		) strcat(buf, " quiet");
    if (comm_flags & COMM_DEAF		) strcat(buf, " deaf");
    if (comm_flags & COMM_NOWIZ		) strcat(buf, " no_wiz");
    if (comm_flags & COMM_NOAUCTION	) strcat(buf, " no_auction");
    if (comm_flags & COMM_NOARENA	) strcat(buf, " no_arena");
    if (comm_flags & COMM_NOPUBCHAN	) strcat(buf, " no_pubchans");
    if (comm_flags & COMM_NOCLAN	) strcat(buf, " no_clan");
    if (comm_flags & COMM_NOQGOSSIP	) strcat(buf, " no_qgossip");
    if (comm_flags & PLR_NOCGOSSIP      ) strcat(buf, " no_cgossip"); 
    if (comm_flags & SHD_NOPRAY         ) strcat(buf, " no_pray");
    if (comm_flags & COMM_NOASK		) strcat(buf, " no_ask");
    if (comm_flags & COMM_NOMUSIC	) strcat(buf, " no_music");
    if (comm_flags & COMM_NOCHAT         ) strcat(buf, " no_chat");
    if (comm_flags & COMM_COMPACT	) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF		) strcat(buf, " brief");
    if (comm_flags & PLR_SHORT_COMBAT	) strcat(buf, " shortcmbt");
    if (comm_flags & COMM_LONG		) strcat(buf, " long");
    if (comm_flags & COMM_STORE		) strcat(buf, " autostore");
    if (comm_flags & COMM_PROMPT	) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE	) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE	) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT	) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL	) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " no_channels");
    if (comm_flags & COMM_NOSOCIAL      ) strcat(buf, " no_social");
    if (comm_flags & COMM_NOQUOTE       ) strcat(buf, " no_quote");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM		) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH		) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH		) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE		) strcat(buf, " fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID		) strcat(buf, " acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY		) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE		) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT		) strcat(buf, " light");
    if (imm_flags & IMM_IRON           ) strcat(buf, " iron");
    if (imm_flags & IMM_WOOD           ) strcat(buf, " wood");
    if (imm_flags & IMM_SILVER ) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");
    if (wear_flags & ITEM_WEAR_FLOAT	) strcat(buf, " float");
    if (wear_flags & ITEM_WEAR_FACE	) strcat(buf, " face");
    if (wear_flags & ITEM_WEAR_EAR      ) strcat(buf, " ear");
    if (wear_flags & ITEM_WEAR_ANKLE    ) strcat(buf, " ankle");
    if (wear_flags & ITEM_WEAR_CLAN     ) strcat(buf, " c_patch");
    if (wear_flags & ITEM_WEAR_RELIG    ) strcat(buf, " r_patch");
    if (wear_flags & ITEM_WEAR_PATCH    ) strcat(buf, " patch");
    if (wear_flags & ITEM_WEAR_BACK     ) strcat(buf, " back");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *room_bit_name(int room_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (room_flags & ROOM_DARK		) strcat(buf, " dark");
    if (room_flags & ROOM_NO_MOB	) strcat(buf, " no_mob");
    if (room_flags & ROOM_INDOORS	) strcat(buf, " indoors");
    if (room_flags & ROOM_PRIVATE	) strcat(buf, " private");
    if (room_flags & ROOM_SAFE		) strcat(buf, " safe");
    if (room_flags & ROOM_SOLITARY	) strcat(buf, " solitary");
    if (room_flags & ROOM_PET_SHOP	) strcat(buf, " pet_shop");
    if (room_flags & ROOM_NO_RECALL	) strcat(buf, " no_recall");
    if (room_flags & ROOM_IMP_ONLY	) strcat(buf, " imp");
    if (room_flags & ROOM_GODS_ONLY	) strcat(buf, " gods");
    if (room_flags & ROOM_HEROES_ONLY	) strcat(buf, " heroes");
    if (room_flags & ROOM_NEWBIES_ONLY	) strcat(buf, " newbies");
    if (room_flags & ROOM_LAW		) strcat(buf, " law");
    if (room_flags & ROOM_NOWHERE	) strcat(buf, " nowhere");
    if (room_flags & ROOM_LOCKED	) strcat(buf, " locked");
    if (room_flags & ROOM_ARENA         ) strcat(buf, " arena");
    if (room_flags & ROOM_NO_MAGIC      ) strcat(buf, " no_magic");
    if (room_flags & ROOM_SHOP          ) strcat(buf, " shop");
    if (room_flags & ROOM_REMORT        ) strcat(buf, " remort");
    if (room_flags & ROOM_ON_FIRE	) strcat(buf, " on_fire");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON	) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD	) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT	) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER	) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE	) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC	) strcat(buf, " vampiric");
    if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL	) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_SHOCKING 	) strcat(buf, " shocking");
    if (weapon_flags & WEAPON_POISON	) strcat(buf, " poison");
    if (weapon_flags & WEAPON_OSMOSIS   ) strcat(buf, " osmosis");
   // if (weapon_flags & WEAPON_MYTHIC    ) strcat(buf, " mythic");
  //  if (weapon_flags & WEAPON_ELEMENTAL ) strcat(buf, " elemental"); breeness
     return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *cont_bit_name( int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (cont_flags & CONT_CLOSEABLE	) strcat(buf, " closable");
    if (cont_flags & CONT_PICKPROOF	) strcat(buf, " pickproof");
    if (cont_flags & CONT_CLOSED	) strcat(buf, " closed");
    if (cont_flags & CONT_LOCKED	) strcat(buf, " locked");

    return (buf[0] != '\0' ) ? buf+1 : "none";
}


char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB	) strcat(buf, " backstab");
    if (off_flags & OFF_BASH		) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK		) strcat(buf, " berserk");
    if (off_flags & OFF_DISARM		) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE		) strcat(buf, " dodge");
    if (off_flags & OFF_FADE		) strcat(buf, " fade");
    if (off_flags & OFF_FAST		) strcat(buf, " fast");
    if (off_flags & OFF_KICK		) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT	) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY		) strcat(buf, " parry");
    if (off_flags & OFF_PHASE           ) strcat(buf, " phase");
    if (off_flags & OFF_RESCUE		) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL		) strcat(buf, " tail");
    if (off_flags & OFF_TRIP		) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH		) strcat(buf, " crush");
    if (off_flags & ASSIST_ALL		) strcat(buf, " assist_all");
    if (off_flags & ASSIST_ALIGN	) strcat(buf, " assist_align");
    if (off_flags & ASSIST_RACE		) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS	) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD	) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM		) strcat(buf, " assist_vnum");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *sector_bit_name(int sect_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (sect_flags & SECT_FOREST     ) strcat(buf, " forest");
    if (sect_flags & SECT_INSIDE     ) strcat(buf, " inside");
    if (sect_flags & SECT_CITY       ) strcat(buf, " city");
    if (sect_flags & SECT_FIELD      ) strcat(buf, " field");
    if (sect_flags & SECT_HILLS      ) strcat(buf, " hills");
    if (sect_flags & SECT_MOUNTAIN   ) strcat(buf, " mountain");
    if (sect_flags & SECT_WATER_SWIM ) strcat(buf, " water_swim");
    if (sect_flags & SECT_AIR        ) strcat(buf, " air");
    if (sect_flags & SECT_UNUSED     ) strcat(buf, " exotic");
    if (sect_flags & SECT_DESERT     ) strcat(buf, " desert");
    if (sect_flags & SECT_WATER_NOSWIM ) strcat(buf, " noswim");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


bool remove_voodoo(CHAR_DATA *ch)
{
    DESCRIPTOR_DATA *d;
    OBJ_DATA *object;
    bool found = FALSE;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *victim;

	if (d->connected != CON_PLAYING)
	    continue;

        victim = ( d->original != NULL ) ? d->original : d->character;

	if (IS_IMMORTAL(victim) && (victim->level > ch->level))
	    continue;

	for ( object = victim->carrying; object != NULL; object = object->next_content )
	{
	    if (object->pIndexData->vnum == OBJ_VNUM_VOODOO)
	    {
		char arg[MAX_INPUT_LENGTH];

		one_argument(object->name, arg);
		if (!str_cmp(arg, ch->name))
		{
		    found = TRUE;
		    object->timer = 1;
		}
	    }
	}
    }
    if (!found)
	return FALSE;

    return TRUE;
}

 int god_lookup (const char *name)
 {
    int god;
 
    for ( god = 0; god_table[god].name != NULL; god++)
    {
         if (LOWER(name[0]) == LOWER(god_table[god].name[0])
         &&  !str_prefix( name,god_table[god].name))
             return god;
    }
 
    return 0;
 }

#define OBJ_VNUM_DEBUGPATCH	10600

bool wears_debug( CHAR_DATA *ch )
{
	OBJ_DATA	*obj;

	for (obj= ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if ((obj->pIndexData->vnum == OBJ_VNUM_DEBUGPATCH) &&
			(obj->wear_loc != -1))
			return TRUE;
	}

	return FALSE;
}


char *get_pers(CHAR_DATA *ch, CHAR_DATA *looker)
{
    char buf[MAX_STRING_LENGTH/10];

    if (looker != NULL && !IS_NPC(ch))
    {
     if (is_affected(ch,gsn_conceal) && is_same_clan(ch,looker))
     {
      strcpy(buf," (Concealed)");
      strcat(buf,ch->name);
      return ( buf[0] != '\0' ) ? buf+1 : ch->name;
     }

      if (IS_NPC(looker) || (IS_IMMORTAL(looker)) || is_same_clan(ch,looker))
	return ch->name;
    }

    /* Start going through affects */
    if (is_affected(ch,gsn_conceal) || ch->morph_form[0] == MORPH_CONCEAL)
    {
//     return "a cloaked figure";
     return get_descr_form(ch,looker,FALSE);
    }
    if (is_affected(ch,gsn_conceal) || ch->morph_form[0] == MORPH_CONCEAL2)
    {
//     return "a cloaked figure";
     return get_descr_form(ch,looker,FALSE);
    }
    if (ch->morph_form[0] >= MORPH_RED && ch->morph_form[0] <= MORPH_GOLD)
    {
     return get_descr_form(ch,looker,FALSE);
    }
    if (ch->morph_form[0] == MORPH_MORPH)
     return ch->morph;

    return ch->name;
}

char *show_pers(CHAR_DATA *ch, CHAR_DATA *looker)
{
 /* Can See */
 if(can_see(looker,ch))
 {
  if(IS_NPC(ch))
   return ch->short_descr;
  else if(IS_IMMORTAL(looker))
   return ch->name;
  else
   return get_pers(ch,looker);
 }
 else /* Can't See */
 {
  if(IS_NPC(ch))
   return "someone";
  else /* PC */
  {
   if(IS_IMMORTAL(ch))
   {
    if(ch->pcdata->identity[0] == '\0')
     return "someone";
    else
     return ch->pcdata->identity;
   }
   else
    return "someone";
  }
 }
 return "someone";
}
char *get_name(CHAR_DATA *ch, CHAR_DATA *victim, char buf[])
{
//char buf[200];

    if (victim != NULL)
    {
     if (IS_NPC(victim) || (IS_IMMORTAL(victim)) || is_same_clan(ch,victim))
	return ch->name;
    }
    strcpy(buf," ");
    strcat(buf,ch->name);
    /*if (is_affected(ch,gsn_conceal))
    {
     strcat(buf," dark assassin");
    }*/
    if (ch->morph_form[0] == MORPH_RED)
     strcat(buf," red dragon");
    if (ch->morph_form[0] == MORPH_BLACK)
     strcat(buf," black dragon");
    if (ch->morph_form[0] == MORPH_GREEN)
     strcat(buf," green dragon");
    if (ch->morph_form[0] == MORPH_WHITE)
     strcat(buf," white dragon");
    if (ch->morph_form[0] == MORPH_GOLD)
     strcat(buf," gold dragon");

    return buf;
}


/*
 * Class Restricted Items -- Skyntil  7/30/00
 */
bool is_class_restricted(CHAR_DATA *ch, OBJ_DATA *obj)
{
 int restricts;
 bool canwear = TRUE;

 restricts = obj->class_restrict_flags;

 if((restricts & WIZARD_ONLY) && ch->class != CLASS_WIZARD
 && ch->class != CLASS_MAGE)
  canwear = FALSE;
 if((restricts & GLADIATOR_ONLY) && ch->class != CLASS_GLADIATOR
  && ch->class != CLASS_WARRIOR)
  canwear = FALSE;
 if((restricts & PRIEST_ONLY) && ch->class != CLASS_PRIEST
  && ch->class != CLASS_CLERIC)
  canwear = FALSE;
 if((restricts & MERCENARY_ONLY) && ch->class != CLASS_MERCENARY
  && ch->class != CLASS_THIEF)
  canwear = FALSE;
 if((restricts & STRIDER_ONLY) && ch->class != CLASS_STRIDER
  && ch->class != CLASS_RANGER)
  canwear = FALSE;
 if((restricts & SAGE_ONLY) && ch->class != CLASS_SAGE
  && ch->class != CLASS_DRUID)
  canwear = FALSE;
 if((restricts & LICH_ONLY) && ch->class != CLASS_LICH
  && ch->class != CLASS_SHADE)
  canwear = FALSE;
 if((restricts & FORSAKEN_ONLY) && ch->class != CLASS_FORSAKEN)
  canwear = FALSE;
 if((restricts & CONJURER_ONLY) && ch->class != CLASS_CONJURER)
  canwear = FALSE;
 if((restricts & ARCHMAGE_ONLY) && ch->class != CLASS_ARCHMAGE)
  canwear = FALSE;
 if((restricts & HIGHLANDER_ONLY) && ch->class != CLASS_HIGHLANDER)
  canwear = FALSE;
 if((restricts & SWASHBUCKLER_ONLY) && ch->class != CLASS_SWASHBUCKLER)
  canwear = FALSE;
 if((restricts & CRUSADER_ONLY) && ch->class != CLASS_CRUSADER)
  canwear = FALSE;
 if((restricts & VOODAN_ONLY) && ch->class != CLASS_VOODAN)
  canwear = FALSE;
 if((restricts & MONK_ONLY) && ch->class != CLASS_MONK)
  canwear = FALSE;
 if((restricts & SAINT_ONLY) && ch->class != CLASS_SAINT)
  canwear = FALSE;
 if((restricts & ASSASSIN_ONLY) && ch->class != CLASS_ASSASSIN)
  canwear = FALSE;
 if((restricts & NINJA_ONLY) && ch->class != CLASS_NINJA)
  canwear = FALSE;
 if((restricts & BARD_ONLY) && ch->class != CLASS_BARD)
  canwear = FALSE;
 if((restricts & DARKPALADIN_ONLY) && ch->class != CLASS_DARKPALADIN)
  canwear = FALSE;
 if((restricts & HUNTER_ONLY) && ch->class != CLASS_HUNTER)
  canwear = FALSE;
 if((restricts & PALADIN_ONLY) && ch->class != CLASS_PALADIN)
  canwear = FALSE;
 if((restricts & WARLOCK_ONLY) && ch->class != CLASS_WARLOCK)
  canwear = FALSE;
 if((restricts & ALCHEMIST_ONLY) && ch->class != CLASS_ALCHEMIST)
  canwear = FALSE;
 if((restricts & SHAMAN_ONLY) && ch->class != CLASS_SHAMAN)
  canwear = FALSE;
 if((restricts & FADE_ONLY) && ch->class != CLASS_FADE)
  canwear = FALSE;
 if((restricts & NECROMANCER_ONLY) && ch->class != CLASS_NECROMANCER)
  canwear = FALSE;
 if((restricts & BANSHEE_ONLY) && ch->class != CLASS_BANSHEE)
  canwear = FALSE;

 /* Now for the clan things */
 if(!clan_can_use(ch,obj))
   canwear = FALSE;
  return canwear;
}

char *res_bit_name(int res_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (res_flags & RES_SUMMON          ) strcat(buf, " summon");
    if (res_flags & RES_CHARM           ) strcat(buf, " charm");
    if (res_flags & RES_MAGIC           ) strcat(buf, " magic");
    if (res_flags & RES_WEAPON          ) strcat(buf, " weapon");
    if (res_flags & RES_BASH            ) strcat(buf, " blunt");
    if (res_flags & RES_PIERCE          ) strcat(buf, " piercing");
    if (res_flags & RES_SLASH           ) strcat(buf, " slashing");
    if (res_flags & RES_FIRE            ) strcat(buf, " fire");
    if (res_flags & RES_COLD            ) strcat(buf, " cold");
    if (res_flags & RES_LIGHTNING       ) strcat(buf, " lightning");
    if (res_flags & RES_ACID            ) strcat(buf, " acid");
    if (res_flags & RES_POISON          ) strcat(buf, " poison");
    if (res_flags & RES_NEGATIVE        ) strcat(buf, " negative");
    if (res_flags & RES_HOLY            ) strcat(buf, " holy");
    if (res_flags & RES_ENERGY          ) strcat(buf, " energy");
    if (res_flags & RES_MENTAL          ) strcat(buf, " mental");
    if (res_flags & RES_DISEASE         ) strcat(buf, " disease");
    if (res_flags & RES_DROWNING        ) strcat(buf, " drowning");
    if (res_flags & RES_LIGHT           ) strcat(buf, " light");
    if (res_flags & RES_IRON           ) strcat(buf, " iron");
    if (res_flags & RES_WOOD           ) strcat(buf, " wood");
    if (res_flags & RES_SILVER ) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *vuln_bit_name(int vuln_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (vuln_flags & VULN_SUMMON          ) strcat(buf, " summon");
    if (vuln_flags & VULN_CHARM           ) strcat(buf, " charm");
    if (vuln_flags & VULN_MAGIC           ) strcat(buf, " magic");
    if (vuln_flags & VULN_WEAPON          ) strcat(buf, " weapon");
    if (vuln_flags & VULN_BASH            ) strcat(buf, " blunt");
    if (vuln_flags & VULN_PIERCE          ) strcat(buf, " piercing");
    if (vuln_flags & VULN_SLASH           ) strcat(buf, " slashing");
    if (vuln_flags & VULN_FIRE            ) strcat(buf, " fire");
    if (vuln_flags & VULN_COLD            ) strcat(buf, " cold");
    if (vuln_flags & VULN_LIGHTNING       ) strcat(buf, " lightning");
    if (vuln_flags & VULN_ACID            ) strcat(buf, " acid");
    if (vuln_flags & VULN_POISON          ) strcat(buf, " poison");
    if (vuln_flags & VULN_NEGATIVE        ) strcat(buf, " negative");
    if (vuln_flags & VULN_HOLY            ) strcat(buf, " holy");
    if (vuln_flags & VULN_ENERGY          ) strcat(buf, " energy");
    if (vuln_flags & VULN_MENTAL          ) strcat(buf, " mental");
    if (vuln_flags & VULN_DISEASE         ) strcat(buf, " disease");
    if (vuln_flags & VULN_DROWNING        ) strcat(buf, " drowning");
    if (vuln_flags & VULN_LIGHT           ) strcat(buf, " light");
    if (vuln_flags & VULN_IRON           ) strcat(buf, " iron");
    if (vuln_flags & VULN_WOOD           ) strcat(buf, " wood");
    if (vuln_flags & VULN_SILVER ) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *size_name( int size )
{
 switch(size)
 {
  case SIZE_TINY: return "tiny";
  case SIZE_SMALL: return "small";
  case SIZE_MEDIUM: return "medium";
  case SIZE_LARGE: return "large";
  case SIZE_HUGE: return "huge";
  case SIZE_GIANT: return "giant";
 }
  return "\0";
}

char *room_affect_name(int room_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (room_flags & ROOM_AFF_SAFE          ) strcat(buf, " sanctuary");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}



