#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "clan.h"
#include "tables.h"
#include <math.h>

double pow( double x, double y );

/* command procedures needed */
DECLARE_DO_FUN (do_backstab);
DECLARE_DO_FUN (do_strangle);
DECLARE_DO_FUN (do_layhands);
DECLARE_DO_FUN (do_smokebomb);
DECLARE_DO_FUN (do_emote);
DECLARE_DO_FUN (do_berserk);
DECLARE_DO_FUN (do_bash);
DECLARE_DO_FUN (do_trip);
DECLARE_DO_FUN (do_dirt);
DECLARE_DO_FUN (do_flee);
DECLARE_DO_FUN (do_fear);
DECLARE_DO_FUN (do_kick);
DECLARE_DO_FUN (do_disarm);
DECLARE_DO_FUN (do_get);
DECLARE_DO_FUN (do_recall);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_sacrifice);
DECLARE_DO_FUN (do_circle);
DECLARE_DO_FUN (do_feed);
DECLARE_DO_FUN (do_gouge);
DECLARE_DO_FUN (do_vdpi);
DECLARE_DO_FUN (do_vdtr);
DECLARE_DO_FUN (do_vdth);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_announce);
DECLARE_DO_FUN (do_clantalk);
DECLARE_DO_FUN (do_twirl);
DECLARE_DO_FUN (do_rank);
DECLARE_DO_FUN (do_lead);

/*
 * Local functions.
 */
void check_assist args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge_real args ((CHAR_DATA * ch, CHAR_DATA * victim, bool silent));
bool check_parry args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_phase args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_phase_real args ((CHAR_DATA * ch, CHAR_DATA * victim, bool silent));
bool check_shield_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_wall args ((CHAR_DATA * ch, CHAR_DATA * victim));

void dam_message args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
						int dt, bool immune));
void dam_message_new args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
                                     int dt, bool immune, int dam_flags));
void death_cry args ((CHAR_DATA * ch));
void group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe args ((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute args((CHAR_DATA * gch, CHAR_DATA * victim, int total_levels, int members));
int align_compute args((CHAR_DATA * gch, CHAR_DATA * victim, int total_levels, int members));
bool is_safe_mock args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_voodood args ((CHAR_DATA * ch, CHAR_DATA * victim));
void make_corpse args ((CHAR_DATA * ch, CHAR_DATA * killer));
void one_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary));
void one_hit_mock args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary));
void mob_hit args ((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
void raw_kill args ((CHAR_DATA * victim, CHAR_DATA * killer));
void set_fighting args ((CHAR_DATA * ch, CHAR_DATA * victim));
void disarm args ((CHAR_DATA * ch, CHAR_DATA * victim));
void check_arena args ((CHAR_DATA * ch, CHAR_DATA * victim));
int dambonus args ((CHAR_DATA * ch, CHAR_DATA * victim,int dam, int stance));
bool can_counter args ((CHAR_DATA * ch));
bool can_bypass args ((CHAR_DATA * ch, CHAR_DATA * victim));
void update_power args((CHAR_DATA *highlander));
void falcon_load args((CHAR_DATA *ch, CHAR_DATA *victim));
void woodsman_twirl args((CHAR_DATA * ch, char *argument));
void part_create args((CHAR_DATA *ch, char *argument));

int add_bonuses args(( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type ));

char debugbuf[MSL];

int add_damroll args((CHAR_DATA *ch, int diceside));
void death_message args((CHAR_DATA *killer, CHAR_DATA *victim));
int check_martial_arts args((CHAR_DATA *ch, CHAR_DATA *victim));
bool check_open_palming args((CHAR_DATA *ch));
void clear_stance_affects args((CHAR_DATA *ch));
bool check_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_roll args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_acrobatics args ((CHAR_DATA * ch, CHAR_DATA * victim));
OBJ_DATA * rand_obj args( ( CHAR_DATA *ch, int mob_level )); 
int focus_ac args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch));
int focus_dam args ((CHAR_DATA *ch));
void improve_toughness args((CHAR_DATA * ch));
bool check_tumble args ((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void special_move args (( CHAR_DATA *ch, CHAR_DATA *victim ));
bool check_deflect args((CHAR_DATA *ch, CHAR_DATA *victim, int dt,int dam));
bool check_counter_defense args((CHAR_DATA *ch,CHAR_DATA *victim,int dt));
char * get_descr_form args((CHAR_DATA *ch,CHAR_DATA *looker, bool get_long));
void do_mutilate args((CHAR_DATA *ch, CHAR_DATA *victim));
int rank_pk args ((CHAR_DATA * ch, CHAR_DATA * victim));
int rank_arena args ((CHAR_DATA * ch, CHAR_DATA * victim));
int chance_pk args ((CHAR_DATA * ch, CHAR_DATA * victim));
int chance_arena args ((CHAR_DATA * ch, CHAR_DATA * victim));

/* Monk Chi */
void do_chi(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_chi) <= 0)
        || (ch->level < skill_table[gsn_chi].skill_level[ch->class] ) )
    {
        send_to_char("You don't have any chi to focus on.\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_chi) )
    {
        send_to_char("You are already focusing on your chi.\n\r",ch);
	return;
    }

    if (ch->mana < 200 || ch->move < 200)
    {
        send_to_char("You do not have enough mental power to concentrate.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_chi))
    {
        send_to_char("You concentrate on becoming one with your spirit but cannot attain the zenith.\n\r",ch);
        check_improve(ch,gsn_chi,FALSE,1);
        ch->mana -= 100;
        ch->move -= 100;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_chi;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.modifier = ch->level*get_skill(ch,gsn_chi)/100;
    af.duration = ch->level/6;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    af.modifier = ch->level*get_skill(ch,gsn_chi)/300;
    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);
    af.modifier = 50;
    af.location = APPLY_AC;
    affect_to_char(ch,&af);

    ch->mana -= 200;
    ch->move -= 200;
    ch->pcdata->power[0] = ch->level*get_skill(ch,gsn_chi)/100;
    ch->pcdata->power[1] = get_skill(ch,gsn_chi)/5;

    act("$n's forehead clouds with concentration.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You focus on the balance of the forces.\n\r",ch);
    check_improve(ch,gsn_chi,TRUE,1);
    return;
}

/* Ninja Michi */
void do_michi(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_michi) <= 0)
        || (ch->level < skill_table[gsn_michi].skill_level[ch->class] ) )
    {
        send_to_char("You don't have any michi to focus on.\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_michi) )
    {
        send_to_char("You are already focusing on your michi.\n\r",ch);
	return;
    }

    if (ch->mana < 200 || ch->move < 200)
    {
        send_to_char("You do not have enough mental power to concentrate.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_michi))
    {
        send_to_char("You concentrate on becoming one with your spirit but cannot attain the zenith.\n\r",ch);
        check_improve(ch,gsn_michi,FALSE,1);
        ch->mana -= 100;
        ch->move -= 100;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_michi;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.modifier = ch->level*get_skill(ch,gsn_michi)/200;
    af.duration = ch->level/6;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    ch->mana -= 200;
    ch->move -= 200;

    act("$n's forehead clouds with concentration.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You focus on the balance of the forces.\n\r",ch);
    check_improve(ch,gsn_chi,TRUE,1);
    return;
}

/* Highlander */
void do_deathblow(CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
        AFFECT_DATA af;

        if (get_skill (ch, gsn_decapitate) == 0 || (!IS_NPC (ch)
         && ch->level < skill_table[gsn_decapitate].skill_level[ch->class]))
	{
                send_to_char ("Huh?\n\r", ch);
		return;
	}

        if(is_affected(ch,gsn_decapitate))
        {
         send_to_char("You cannot muster enough strength to decapitate again.\n\r",ch);
         return;
        }

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
          send_to_char ("You need to wield a a bladed weapon to decapitate.\n\r", ch);
          return;
	}

        if( obj->value[0] != WEAPON_SWORD && obj->value[0] != WEAPON_AXE )
	{
          send_to_char ("You need to wield a a bladed weapon to decapitate.\n\r", ch);
          return;
	}

        if (victim->hit > victim->max_hit / 4)
	{
                act ("$N is too wary to strike.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 30)) {
		act("You attempt to strike them but find yourself crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to strike them but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if (!can_see (ch, victim))
	{
		send_to_char ("You stumble blindly into a wall.\n\r", ch);
		return;
	}

        WAIT_STATE (ch, skill_table[gsn_decapitate].beats);
        if (number_percent () < (get_skill (ch, gsn_decapitate)*2)/3
           || (get_skill (ch, gsn_decapitate) >= 2))
	{
                check_improve (ch, gsn_decapitate, TRUE, 2);
                act ("{)$n attempts to decapitate you!{x", ch, NULL, victim, TO_VICT);
                act ("{.You swing your weapon in an arc and bring it down upon $N!{x", ch, NULL, victim, TO_CHAR);
                act ("{($n swings $s weapon in an arc and brings it down upon $N.{x", ch, NULL, victim, TO_NOTVICT);

                one_hit( ch, victim, gsn_decapitate, FALSE );
                one_hit( ch, victim, gsn_decapitate, FALSE );
	}
	else
	{
                check_improve (ch, gsn_decapitate, FALSE, 1);
                act ("{.$N ducks below your swing.{x", ch, NULL, victim, TO_CHAR);
                damage (ch, victim, 0, gsn_decapitate, DAM_NONE, TRUE, 0);
	}

       /* not all good things come cheap */
        af.where = TO_AFFECTS;
        af.type = gsn_decapitate;
        af.level = ch->level;
        af.modifier = -5;
        af.bitvector = 0;
        af.location = APPLY_DEX;
        af.duration = 0;
        affect_to_char(ch, &af);

        ch->mana = UMAX(ch->mana - 100,0);

/*        if(ch->pcdata->power[POWER_POINTS] < 65500)
        ch->pcdata->power[POWER_POINTS] += dice(1,10); */
        update_power(ch);

	return;
}

void do_quickening(CHAR_DATA *ch, char *argument)
{
 AFFECT_DATA af;

 if (get_skill (ch, gsn_quickening) == 0 || (!IS_NPC (ch)
 && ch->level < skill_table[gsn_quickening].skill_level[ch->class]))
 {
    send_to_char ("Huh?\n\r", ch);
    return;
 }

 if(ch->class != CLASS_HIGHLANDER)
  return;

 if(is_affected(ch,gsn_quickening))
 {
   send_to_char("You are still affected by the quickening.\n\r",ch);
   return;
 }

 if(ch->hit < 1100 || ch->pcdata->power[POWER_POINTS] < 51)
 {
  send_to_char("You do not have the power to be quickened.\n\r",ch);
  return;
 }

 ch->mana = ch->mana/2;
 damage(ch,ch,1500,gsn_quickening,DAM_OTHER,TRUE,0);
/*  ch->pcdata->power[POWER_POINTS] -= 50; */

 send_to_char("You feel energy surge through your body.\n\r",ch);
 act("$n writhes as lightning strikes his body from the heavens.\n\r",ch,NULL,NULL,TO_ROOM);
 
 af.where = TO_AFFECTS;
 af.level = ch->level;
 af.location = APPLY_AC;
 af.modifier = -(ch->level);
 af.duration = ch->level/2;
 af.bitvector = 0;
 af.type = gsn_quickening;
 affect_to_char(ch,&af);

 return;
}

/* Banshee */
void do_shriek (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_STRING_LENGTH];
  int chance;
  int dam;

  one_argument(argument, arg);

  if(get_skill(ch,gsn_shriek)==0)
  {
    send_to_char("Do, re, me, fa, so, laaa *coughcoughcough* te do.\n\r",ch);
    return;
  }

  if(arg[0]=='\0' && ch->fighting == NULL)
  {
    send_to_char("Shriek at who?\n\r",ch);
    return;
  }

  if((victim = get_char_room (ch, arg))==NULL)
  {
    send_to_char("Shriek at who?\n\r",ch);
    return;
  }

 if(arg[0]=='\0')
	victim = ch->fighting;

  if(is_safe (ch, victim))
  {
    send_to_char("Go yell at someone else, loudmouth.\n\r",ch);
    return;
  }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 30)) {
		act("You attempt to shriek but find yourself crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to shriek but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

  if (ch->stunned)
  {
          send_to_char ("You're still a little woozy.\n\r", ch);
          return;
  }
  
  send_to_char("Shrieeeeeeeeeek....\n\r",ch);

  chance = get_skill(ch,gsn_shriek)-(victim->level/5);
  if(number_percent() < chance)
  {
	dam = (dice(get_age(ch)*5, 5)+ dice(ch->level/2, 7) - dice(victim->level/2, 2));
	
	if(ch->hit > 15000)
		dam += (15000/9);
	else
		dam += (ch->hit)/8.5;
	damage (ch, victim, dam, gsn_shriek, DAM_MENTAL, TRUE, 0);
	check_improve (ch, gsn_shriek, TRUE, 1);
	if (!IS_AFFECTED(victim,AFF_DEAF) || (IS_AFFECTED(victim,AFF_DEAF) && number_percent() < 50)) {

		AFFECT_DATA af;

		af.where = TO_AFFECTS;
		af.type = gsn_shriek;
		af.level = ch->level;

	if (IS_AFFECTED(victim,AFF_DEAF))	{ /* This removes previous shriek and averages modifier */
		AFFECT_DATA *paf;
		for(paf = victim->affected; paf != NULL; paf = paf->next)	{
			if(paf->type == gsn_shriek) {
				af.modifier = ((paf->modifier+(-1*number_range(get_age(ch)/2,get_age(ch))))/2);
				affect_remove (victim, paf); 
		} } }
	else {	af.modifier = -1*number_range(get_age(ch)/3, get_age(ch)); }
		af.duration = number_fuzzy(ch->level/6);
		af.location = APPLY_HITROLL;
		af.bitvector = AFF_DEAF;

		affect_to_char (victim, &af);
	}

        WAIT_STATE (ch, PULSE_VIOLENCE*1.5);
  }
  else
  {
	damage (ch, victim, 0, gsn_shriek, DAM_MENTAL, FALSE, 0);
        act ("{.You let out a sad sounding gurgle.{x",
	 ch, NULL, victim, TO_CHAR);
        act ("{($n lets out a sad sounding gurgle.{x",
	 ch, NULL, victim, TO_NOTVICT);
        act ("{)$n lets out a sad sounding gurgle.{x",
	 ch, NULL, victim, TO_VICT);
	check_improve (ch, gsn_shriek, FALSE, 1);
        WAIT_STATE (ch, skill_table[gsn_bash].beats * 5 / 3);

  }

}

/* Ninja */
void do_strangle(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH/2];

    one_argument(argument,arg);
    
    
    if(get_skill(ch,gsn_strangle)==0)
    {
      send_to_char("Leave it to the skilled for that.\n\r",ch);
      return;
    }

    if( ch->fighting != NULL )
    {
     send_to_char("Are you nuts? You'd lose your hands before they reach a neck.\n\r",ch);
     return;
    }

    if((victim = get_char_room( ch, arg )) == NULL)
    {
      send_to_char("For being so stealthy, you sure aren't too keen... No one here by that name.\n\r",ch);
      return;
    }

    if(arg[0]=='\0')
    {
      send_to_char("How about a target, ace?\n\r",ch);
      return;
    }

    if(!IS_NPC(victim) && IS_IMMORTAL(victim))
    {
      send_to_char("Think again.\n\r",ch);
      return;
    }

    if (is_safe (ch, victim))
                return;

/*    if (IS_NPC (victim) &&
                victim->fighting != NULL &&
                !is_same_group (ch, victim->fighting))
        {
                send_to_char ("Kill stealing is not permitted.\n\r", ch);
                return;
        } */
        
    if (ch->stunned)
    {
          send_to_char ("You're still a little woozy.\n\r", ch);
          return;
    }

    chance = number_percent()*((2*victim->level)/ch->level);
    
    if(IS_AFFECTED(ch,AFF_SNEAK) || !can_see(victim,ch))
	chance /= 3;
    if(IS_AFFECTED(ch,AFF_HIDE) || !can_see(victim,ch))
	chance /= 3;

    if(chance>get_skill(ch,gsn_strangle))
    {
      send_to_char("You failed to strangle them.\r\n",ch);
      act("$n failed to strangle $N.",ch,NULL,victim,TO_NOTVICT);
      act("$n tried to strangle you!.",ch,NULL,NULL,TO_VICT);
      check_improve (ch, gsn_strangle, TRUE, 2);
      
      	  if (!IS_NPC(ch) && !IS_NPC(victim) && victim->fighting == NULL)
          {
           switch(number_range(0,1))
           {
            case (0):
            sprintf(buf,"Die, %s you strangling fool!",PERS(ch,victim));
            break;
            case (1):
            sprintf(buf,"Help! I'm being strangled by %s!",PERS(ch,victim));
            break;
	   }
           do_yell(victim,buf);
          }
      
      WAIT_STATE (ch, skill_table[gsn_strangle].beats);
      multi_hit (victim, ch, TYPE_UNDEFINED);
      return;
    }
    
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
        send_to_char("Something about that doesn't feel right.\n\r",ch);
        return;
    }

    if ((((get_curr_stat(ch,STAT_STR)*2) + number_range(0,4)) < 
    (get_curr_stat(victim,STAT_STR) + get_curr_stat(victim,STAT_CON) + number_range(2,4))) && can_see(victim,ch))
    {
      send_to_char("You attempt to strangle them, and they overpower you!\n\r",ch);
      act("$n failed to strangle $N.",ch,NULL,victim,TO_NOTVICT);
      act("$n tried to strangle you!.",ch,NULL,NULL,TO_VICT);
      check_improve (ch, gsn_strangle, TRUE, 2);
      WAIT_STATE (ch, skill_table[gsn_strangle].beats);
      multi_hit (victim, ch, TYPE_UNDEFINED);
      return;
    }


    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("sleep");
    af.level     = ch->level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );
        
    if ( IS_AWAKE(victim) )
    {
        send_to_char( "Someone just strangled you ..... zzzzzz.\n\r", victim );
        act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
        victim->position = POS_SLEEPING;
        check_improve (ch, gsn_strangle, TRUE, 1);
        
        if(!IS_NPC(ch) && !IS_NPC(victim) && !IN_ARENA(ch) && !IN_ARENA(victim))
	{
	     ch->fight_timer = pktimer;
	     victim->fight_timer = pktimer;
	}
    }
    else
    {
      send_to_char("Uh. Already sleeping...\n\r",ch);
    }
    
    WAIT_STATE (ch, skill_table[gsn_strangle].beats);
    return;
}


/* Paladin */
void do_layhands(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int hp_ammount;
  int chance;
  COOLDOWN_DATA cd;

  one_argument(argument,arg);

  if(!has_skill(ch,gsn_layhands) || get_skill(ch,gsn_layhands) == 0)
  {
    send_to_char("Your god loves you, but not THAT much.\n\r",ch);
    return;
  }

  if((victim = get_char_room( ch, arg )) == NULL)
  {
    send_to_char("Your hands miss their target...\n\r",ch);
    return;
  }
  
  if (ch->stunned)
  {
          send_to_char ("You're still a little woozy.\n\r", ch);
          return;
  }

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_layhands))
	{
                send_to_char ("Layhands is still on cooldown.\n\r", ch);
                return;
       }


	Apply cooldown	
	cd.type      = skill_lookup("layhands");
	cd.duration	 = skill_table[gsn_layhands].cooldown;
        cooldown_on( ch, &cd );*/

  chance = get_skill(ch,gsn_layhands)*3/2;
  if(chance < (number_percent()+10))
  {
    check_improve (ch, gsn_layhands, TRUE, 2);
    WAIT_STATE (ch, skill_table[gsn_layhands].beats);
    send_to_char("You attempt to call your lay on hands power, but it fails.\n\r", ch);
    return;
  }

  if(arg[0]=='\0' || !str_cmp(ch->name,victim->name))
  {
    if(ch->mana < 500 || ch->move < 300)
    {
	    send_to_char("You call upon the gods for healing and find yourself empty.\n\r",ch);
	    return;
    }
    send_to_char("You sense disappointment as you call upon your gods to heal yourself.\n\r",ch);
    act("$n calls upon the favor of their god to heal themselves...\n\r",ch,NULL,NULL,TO_NOTVICT);
    hp_ammount = (ch->level*10) + (get_skill(ch,gsn_layhands)*10);
    if((ch->max_hit - ch->hit) < hp_ammount)
      hp_ammount = ch->max_hit - ch->hit;
    ch->hit += hp_ammount;
    ch->mana -= 500;
    ch->move -= 300;
    check_improve (ch, gsn_layhands, TRUE, 2);
    WAIT_STATE (ch, skill_table[gsn_layhands].beats);
    return;
  }

  send_to_char("You lay your hands on them, and heal their wounds.",ch);
  act("$n calls upon the favor of their god to heal $N...",ch,NULL,victim,TO_NOTVICT);
  /* act("$n calls upon the favor of their god to heal you...",ch,NULL,NULL,TO_VICT); */
  send_to_char("You feel a wave of healing from the Paladins touch...",victim);
  hp_ammount = (ch->level*5) + (get_skill(ch,gsn_layhands)*5);
  if((victim->max_hit - victim->hit) < hp_ammount)
    hp_ammount = victim->max_hit - victim->hit;
  victim->hit += hp_ammount;
  check_improve (ch, gsn_layhands, TRUE, 2);
  WAIT_STATE (ch, skill_table[gsn_layhands].beats);
  return;
}

void do_lead ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int count= 0;
	int members = 0;
	int effect = 0;
	int chance = 0;
	COOLDOWN_DATA cd;

	if (is_affected (ch, gsn_lead) || ch->move < 500 || ch->mana < 300)
	{
		send_to_char("You do not feel the strength within you to lead so soon.\n\r",ch);
		return;
	}

	chance = get_skill(ch,gsn_lead);

	/*if ((gch = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}*/

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group (gch, ch) || IS_NPC (gch))
			continue;
		if ( (gch->leader != ch) && (gch->master != ch) )
			continue;

		members++;
		count += gch->alignment;
	}

	if (members == 0)
	{
		send_to_char ("And who exactly are you leading to inspire?\n\r", ch);
		return;
	}
	
/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_lead))
	{
                send_to_char ("Lead is still on cooldown.\n\r", ch);
                return;
       }


*	Apply cooldown	
	cd.type      = skill_lookup("lead");
	cd.duration	 = skill_table[gsn_lead].cooldown;
        cooldown_on( ch, &cd );*/

	// effect is average align, 1000 being the best
	effect = count / members;

	// reset count to 0 for next step
	count = 0;
	members++;

	// add chance of failure/winning
	effect -= 8*number_percent();
	effect += 8*number_percent();

	effect = effect * chance / 100;

	if (effect < 0)
	{
		send_to_char("You step forward to take charge only to find yourself being laughed at.\n\r",ch);
		act("$n begins a battle cry and accidentally breaks into a humorous falsetto...",ch,NULL,gch,TO_NOTVICT);
		count = 0;
	}
	else if(effect < 250)
	{
		send_to_char("You step forward ushering a half-hearted cry of war.\n\r",ch);
		act("$n begins a half-hearted cry of war wobbling slightly at the climax.",ch,NULL,gch,TO_NOTVICT);
		count = 1;
	}
	else if(effect < 500)
	{
		send_to_char("With the grace of an ox you cry out, calling on comrads to follow you into battle.\n\r",ch);
		act("$n moves with the grace of an ox crying out for men to follow his lead.",ch,NULL,gch,TO_NOTVICT);
		count = 2;
	}
	else if(effect < 750)
	{
		send_to_char("Moving with confidence you lift your voice in a roaring warcry.\n\r",ch);
		act("$n stands with confidence and roars out a powerful warcry.",ch,NULL,gch,TO_NOTVICT);
		count = 3;
	}
	else if(effect < 1000)
	{
		send_to_char("With practiced ease you call for men to follow you knowing that they indeed will.\n\r",ch);
		act("With practiced ease, $n calls in passionate and powerful words for victory!",ch,NULL,gch,TO_NOTVICT);
		count = 4;
	}
	else
	{
		send_to_char("A fierce rage surrounds you inspiring men to follow til death.\n\r",ch);
		act("Infused with power and conviction $n calls on men to follow him to battle.",ch,NULL,gch,TO_NOTVICT);
		count = 5;
	}

	af.where = TO_AFFECTS;
	af.type = gsn_lead;
	af.level = ch->level;
	af.bitvector = 0;
	af.duration = 15;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group (gch, ch) || IS_NPC (gch))
			continue;
		// messages for the individual need to be moved into the loop
		//
		switch(count)
		{
			case (0):
			send_to_char("You feel completely discouraged and uninspired.\n\r",gch);
			break;
			case (1):
			send_to_char("You wonder what just happened and continue on uninspired.\n\r",gch);
			break;
			case (2):
			send_to_char("You feel a slight thrill and your pulse begins to quicken.\n\r",gch);
			break;
			case (3):
			send_to_char("Your muscles tense in anticipation of the fight to come.\n\r",gch);
			break;
			case (4):
			send_to_char("A thrill races down your spine as you prepare to battle.\n\r",gch);
			break;
			case (5):
			send_to_char("Waves of euphoria and adrenaline wash over you pumping life into your limbs.\n\r",gch);
			break;
			default:
			send_to_char("Do_lead: count. - Inform an immortal.\n\r",gch);
		}
		af.modifier = number_range(1,10) * effect / 1000;
		af.location = APPLY_CON;
		affect_to_char(gch,&af);
		af.location = APPLY_STR;
		affect_to_char(gch,&af);
		af.location = APPLY_WIS;
		affect_to_char(gch,&af);
		af.location = APPLY_DEX;
		affect_to_char(gch,&af);
		af.modifier = count * 5;
		af.location = APPLY_HITROLL;
		affect_to_char(gch,&af);
		af.location = APPLY_DAMROLL;
		affect_to_char(gch,&af);
		af.location = APPLY_AC;
		af.modifier = count * -8;
		affect_to_char(gch,&af);
	}

	ch->move -= 500;
	ch->mana -= 300;
	check_improve(ch, gsn_lead, TRUE, 2);
	WAIT_STATE (ch, skill_table[gsn_lead].beats);

	return;
}
