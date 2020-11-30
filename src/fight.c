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
DECLARE_DO_FUN (do_kick);
DECLARE_DO_FUN (do_side_kick);
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

const int vam_str[] =
{
	6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5,
	6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5
};

/*
 * Local functions.
 */
void check_assist args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge_real args ((CHAR_DATA * ch, CHAR_DATA * victim, bool silent));
bool check_parry args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_phase args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_blood args ((CHAR_DATA * ch, CHAR_DATA * victim, bool silent));
bool check_phase_real args ((CHAR_DATA * ch, CHAR_DATA * victim, bool silent));
bool check_shield_block args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_shield_wall args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_barrier args ((CHAR_DATA * ch, CHAR_DATA * victim));
void dam_message args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
						int dt, bool immune));
void dam_message_new args ((CHAR_DATA * ch, CHAR_DATA * victim, int dam,
                                     int dt, bool immune, int dam_flags));
void death_cry args ((CHAR_DATA * ch));
void group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe args ((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe_quiet args ((CHAR_DATA * ch, CHAR_DATA * victim));
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
static int edam_bonus args ((CHAR_DATA *ch, int damage));
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
int focus_sorc args ((CHAR_DATA *ch));
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
int     sorcery_dam  args( ( int num, int dice, CHAR_DATA *ch) );
bool    check_sorcery args( (CHAR_DATA *ch, int sn) );
void clan_entry_trigger args((CHAR_DATA *ch, sh_int clan));
void show_damage_short args((CHAR_DATA *ch, CHAR_DATA *victim));
void do_fear (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_quick_step args((CHAR_DATA *ch, CHAR_DATA *victim));
void check_follow_through args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));

int rounddam = 0;

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update (void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *victim;
        int regenerate=0;

     for (ch = char_list; ch != NULL; ch = ch_next)
     {
          ch_next = ch->next;

        // Regeneration
        if( ch->regen_rate != 0 )
        {
         if(ch->regen_rate > 0)
         {
          if(is_affected(ch,gsn_target))
           regenerate = number_range(ch->regen_rate/2, ch->regen_rate);
          else
           regenerate = number_range(1, ch->regen_rate);

         }
         else if(ch->regen_rate < 0)
          regenerate = number_range(1, -ch->regen_rate);

         if(ch->hit < ch->max_hit)
         {
          if(ch->regen_rate > 0)
          ch->hit = UMIN(ch->hit + regenerate, ch->max_hit);
          else if(ch->regen_rate < 0)
          ch->hit = UMIN(ch->hit - regenerate, ch->max_hit);
         }
        }

        // Mana Regeneration
        if ( ch->mana_regen_rate != 0 )
        {
           if (ch->mana_regen_rate > 0)
              regenerate = number_range(1, ch->mana_regen_rate*2)/2;
           else if(ch->mana_regen_rate < 0)
              regenerate = number_range(1, -ch->mana_regen_rate*2)/2;
           if (ch->mana < ch->max_mana)
           {
              if (ch->mana_regen_rate > 0)
                 ch->mana = UMIN(ch->mana + regenerate, ch->max_mana);
              else if(ch->mana_regen_rate < 0 )
              {
                 ch->mana = UMIN(ch->mana - regenerate, ch->max_mana);
                 ch->mana = ( ch->mana < 0 ? 0 : ch->mana );
              }
           }
        }

        /* For subdued players */
        if(ch->position <= POS_STUNNED)
        {
         ch->hit += number_range(1,2);
         update_pos( ch );
        }

/*         if(ch->position == POS_DEAD && IS_NPC(ch))
          extract_char(ch,TRUE); */

           if (ch->fight_timer > 0)
            ch->fight_timer--;
           if(ch->shd_aura_timer > 0) {
            ch->shd_aura_timer--;
           if(ch->shd_aura_timer == 0) {
            REMOVE_BIT(ch->shielded_by,SHD_PROTECT_AURA);
             if (skill_table[gsn_protective_aura].msg_off ) {
               send_to_char( skill_table[gsn_protective_aura].msg_off, ch );
               send_to_char( "\n\r", ch );
             }
            }
           }

        if ( IS_NPC(ch) && (ch->pIndexData->vnum == MOB_VNUM_FALCON)
        && ch->leader != NULL && ch->leader->fighting == NULL)
        {
		act( "$n flies upward and out of sight.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch,TRUE);
		continue;
        }

	if (ch->stunned)
	{
            ch->stunned--;
            if (!ch->stunned)
            {
              send_to_char ("You regain your equilibrium.\n\r", ch);
              act ("$n regains $s equilibrium.", ch, NULL, NULL, TO_ROOM);
            }
            continue;
	}

		if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
			continue;

                if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
			multi_hit (ch, victim, TYPE_UNDEFINED);
		else
			stop_fighting (ch, FALSE);

		if ((victim = ch->fighting) == NULL)
			continue;

		if (IS_NPC (ch))
		{
			if (HAS_TRIGGER (ch, TRIG_FIGHT))
				mp_percent_trigger (ch, victim, NULL, NULL, TRIG_FIGHT);
			if (HAS_TRIGGER (ch, TRIG_HPCNT))
				mp_hprct_trigger (ch, victim);
		}

		/*
		 * Fun for the whole family!
		 */
		 check_assist(ch,victim);
	}

	return;
}

/* for auto assisting */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim)
{
	CHAR_DATA *rch, *rch_next;

	for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
	{
           rch_next = rch->next_in_room;

           if (!IS_NPC(rch) && rch->ghost_timer > 0)
            continue;

	// no more abusing meditation and healing trance!
           if ((IS_AWAKE (rch) && (rch->position >= POS_STANDING)) && rch->fighting == NULL)
           {
            if (IS_NPC(rch) && (IS_AFFECTED(rch,AFF_CHARM))
                && is_same_group(rch,ch) )
            {
                multi_hit(rch,victim,TYPE_UNDEFINED);
                continue;
            }

            /* quick check for ASSIST_PLAYER */
            if (!IS_NPC (ch) && IS_NPC (rch)
            && IS_SET (rch->off_flags, ASSIST_PLAYERS)
            && rch->level + 6 > victim->level)
            {
              do_emote (rch, "{Rscreams and attacks!{x");
              multi_hit (rch, victim, TYPE_UNDEFINED);
              continue;
            }

            /* PCs next */
            if (!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM))
            {

      /*      This should be taken care of in is_safe()
             if (!IS_NPC (rch) && !IS_NPC (victim)
             && (!is_pkill (rch) || !is_pkill (victim))
             && !IS_SET (victim->act, PLR_TWIT))
              continue; */

             if (((!IS_NPC (rch) && IS_SET (rch->act, PLR_AUTOASSIST))
             || IS_AFFECTED (rch, AFF_CHARM))
             && is_same_group (ch, rch)
             && !is_safe (rch, victim))
              multi_hit (rch, victim, TYPE_UNDEFINED);

             continue;
            }

            /* now check the NPC cases */
            if (IS_NPC (ch) && !IS_AFFECTED (ch, AFF_CHARM))
            {
             if ((IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALL))
             || (IS_NPC (rch) && rch->group && rch->group == ch->group)
             || (IS_NPC (rch) && rch->race == ch->race
             && IS_SET (rch->off_flags, ASSIST_RACE))
             || (IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALIGN)
             && ((IS_GOOD (rch) && IS_GOOD (ch))
             || (IS_EVIL (rch) && IS_EVIL (ch))
             || (IS_NEUTRAL (rch) && IS_NEUTRAL (ch))))
             || (rch->pIndexData == ch->pIndexData
             && IS_SET (rch->off_flags, ASSIST_VNUM)))
             {
               CHAR_DATA *vch;
               CHAR_DATA *target;
               int number;

               if (number_bits (1) == 0)
                continue;

               target = NULL;
               number = 0;
               for (vch = ch->in_room->people; vch; vch = vch->next)
               {
                 if (can_see (rch, vch)
                 && is_same_group (vch, victim)
                 && number_range (0, number) == 0)
                 {
                   target = vch;
                   number++;
                 }
               }

               if (target != NULL)
               {
                 do_emote (rch, "{Rscreams and attacks!{x");
                 multi_hit (rch, target, TYPE_UNDEFINED);
               }
            }
         }
      }
   }
}

/*
 * Do one group of attacks.
 */
void multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
        OBJ_DATA *wield;
        int chance;  /*,dam;*/
        bool falcon_here = FALSE;
        bool quick_step;
	COOLDOWN_DATA *pcd = NULL;

	/* decrement the wait */
	if (ch->desc == NULL)
		ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);

	/* decrement the cooldowns */
	for ( pcd = ch->cooldowns; pcd != NULL; pcd = pcd->next )
	{
	    if ( pcd->duration > 1 )
			pcd->duration--;
	    else
	    {
		char buf[MSL];
		sprintf( buf, "{W%s is no longer on cooldown.{x\n\r",
		    skill_table[pcd->type].name );
		send_to_char( buf , ch );
	  
		cooldown_remove( ch, pcd );
	    }
	}

	if (ch->desc == NULL)
		ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);

        if (ch == victim)
	{
	 send_to_char("Illegal Target.",ch);
	 return;
	}
	
	quick_step = check_quick_step(ch,victim); 

	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING)
		return;

     if (!IS_NPC(ch)
        && (!str_cmp(class_table[ch->class].name,"hunter"))
        && ch->fighting == NULL 
        && !IS_SET (ch->plyr, PLAYER_NOFALCON))
     {
             falcon_load (ch, victim);
             falcon_here = TRUE;
     }


	if (IS_NPC (ch))
	{
		mob_hit (ch, victim, dt);
		return;
	}

    // RE-init as Zero
    ch->pcdata->round_dam = 0;


    /* Stance Special Moves */
    if (!IS_NPC(ch) && ch->stance[0] > 0 && number_percent() == 1)
    {
	int stance = ch->stance[0];
        if (ch->stance[stance] >= 200 && is_warrior_class(ch->class))
	{
	    special_move(ch,victim);
	    return;
	}
    }

    if(!IS_NPC(ch) && ch->class == CLASS_MONK && is_affected(ch,gsn_multistrike) && (number_percent()*9 < ch->pcdata->learned[gsn_multistrike] )) 
    {
       check_improve(ch,gsn_multistrike,TRUE,2);
       act("$n goes into a {RLIGHTNING{x fury of attacks!\n\r",ch,NULL,victim,TO_ROOM);
	   send_to_char("You go into a {RLIGHTNING{x fury of attacks!\n\r",ch);
       one_hit(ch,victim,gsn_multistrike,FALSE);
       if (number_percent() < 90)
       {
       one_hit(ch,victim,gsn_multistrike,FALSE);
   		}
       if (number_percent() < 70)
       {
       one_hit(ch,victim,gsn_multistrike,FALSE);
   		}
       if (number_percent() < 50)
       {
       one_hit(ch,victim,gsn_multistrike,FALSE);
   		}
       }

/* ---------- THE LINE ------ GOING BACK TO REAL COMBAT --BEGIN----- */

    // First Hit -- Everyone Gets One
    one_hit( ch, victim, dt, FALSE );

    if(quick_step)                 // For normal attack 
     one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
        return;

    if (IS_AFFECTED(ch,AFF_HASTE))
        one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
        return;
    
    do_fear(ch,victim);

    // Second Hit
    chance = get_skill(ch,gsn_second_attack);
    chance -= chance/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
      chance /= 2;

    if(is_affected(ch, gsn_concentration))
    {
      chance += 5;
    }

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE);
	
	if(quick_step)                 // For 2nd Attack
     		one_hit(ch,victim,dt, FALSE);
     		
        check_improve(ch,gsn_second_attack,TRUE,5);

        if ( ch->fighting != victim )
            return;
	do_fear(ch,victim);
    }

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_ambush
    || dt == gsn_circle || dt == gsn_twirl || dt == gsn_garrote || dt == gsn_bladedance )
    {
        //show_damage_short(ch, victim);
        return;
    }
    // Second Dual
    chance = get_skill(ch,gsn_dual_wield)/2; //65% with mastery 50% without
        
    if (IS_AFFECTED(ch,AFF_SLOW))
      chance /= 2;
      
    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 2;  

    if(is_affected(ch, gsn_concentration))
      chance += 5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL
    && ch->pcdata->learned[gsn_second_attack] > 10)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         
         check_improve(ch,gsn_dual_wield,TRUE,3);
         if (ch->fighting != victim)
                return;
	 do_fear(ch,victim);
        }
        else
        check_improve(ch,gsn_dual_wield,FALSE,3);
    }

    // Third
    chance = get_skill(ch,gsn_third_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 2;

    if(is_affected(ch, gsn_concentration))
      chance += 5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        
        if(quick_step)                 // For 3rd attack 
     		one_hit(ch,victim,dt, FALSE);
     		
        check_improve(ch,gsn_third_attack,TRUE,6);

        if ( ch->fighting != victim )
            return;
	do_fear(ch,victim);
    }

    // Third Dual
    chance = get_skill(ch,gsn_dual_wield)/3; //43% with mastery 33% without  
    
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 2;

    if(is_affected(ch, gsn_concentration))
      chance += 5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL
        && ch->pcdata->learned[gsn_third_attack] > 10)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         check_improve(ch,gsn_dual_wield,TRUE,3);
         if (ch->fighting != victim)
                return;
	 do_fear(ch,victim);
        }
        else
        check_improve(ch,gsn_dual_wield,FALSE,3);
    }

 /* Martial Arts Placed after third attack for more affect */
 wield = get_eq_char(ch,WEAR_WIELD);
 if( wield == NULL && ch->class == CLASS_MONK
 && is_affected(ch,gsn_martial_arts))
 {
   int dam = 0;
   dam = check_martial_arts(ch,victim);
   if (dam != 0)
   {
        damage ( ch, victim, dam, gsn_martial_arts, DAM_BASH, TRUE, 0 );
        check_improve(ch,gsn_martial_arts,TRUE,2);
   }
 }

 if( ch->class == CLASS_NINJA && is_affected(ch,gsn_martial_arts))
 {
   int dam = 0;
   dam = check_martial_arts(ch,victim);
   if (dam != 0)
   {
        damage ( ch, victim, dam*2, gsn_martial_arts, DAM_BASH, TRUE, 0 );
        check_improve(ch,gsn_martial_arts,TRUE,2);
   }
 }

   /* Renshai Concentration */

  /*  if(is_affected(ch, gsn_concentration))
    {
     chance = get_skill(ch,gsn_concentration)/9;

     if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

     if (number_percent( ) < chance)
     {
      if( ch->move > 35 )
      {
       act("$n skillfully strikes again!",ch,NULL,victim,TO_ROOM);
       one_hit(ch,victim,gsn_concentration,TRUE);
       check_improve(ch,gsn_concentration,TRUE,6);
       ch->move -= 35;
       if (ch->fighting != victim)
              return;
      }
     }
     else
       check_improve(ch,gsn_concentration,FALSE,2);
    } */


    // Fourth
    chance = get_skill(ch,gsn_fourth_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 1.75;

    if(is_affected(ch, gsn_concentration))
      chance -= 5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }
    
    if(quick_step)
     chance += 10;

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        
        if(quick_step)                 // For 4rd attack 
     		one_hit(ch,victim,dt, FALSE);
        
        check_improve(ch,gsn_fourth_attack,TRUE,6);
        if (ch->fighting != victim )
            return;
	do_fear(ch,victim);
    }

    // Fourth Dual
    chance = get_skill(ch,gsn_dual_wield);
    if(chance <= 100) 
     chance /= 5;   // 25% without
    else
     chance /= 4; //32% with mastery
     
    
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 1.5;

    if(is_affected(ch, gsn_concentration))
      chance += 5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL
        && ch->pcdata->learned[gsn_fourth_attack] > 10)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         check_improve(ch,gsn_dual_wield,TRUE,6);
         if (ch->fighting != victim)
                return;
	 do_fear(ch,victim);
        }
        else
        check_improve(ch,gsn_dual_wield,FALSE,5);
    }

    // Fifth Hit
    chance = get_skill(ch,gsn_fifth_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 1.75;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        check_improve(ch,gsn_fifth_attack,TRUE,6);
        if (ch->fighting != victim )
            return;
	do_fear(ch,victim);
    }

    // Fourth Dual
    chance = get_skill(ch,gsn_dual_wield); 
    if(chance <= 100) 
     chance /= 9;   // 12% without
    else
     chance /= 6; //20% with mastery
    
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 1.5;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL
        && ch->pcdata->learned[gsn_fifth_attack] > 10)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         check_improve(ch,gsn_dual_wield,TRUE,6);
         if (ch->fighting != victim)
                return;
	 do_fear(ch,victim);
        }
        else
        check_improve(ch,gsn_dual_wield,FALSE,5);
    }

    // Sixth Hit
    chance = get_skill(ch,gsn_sixth_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE))
      chance *= 1.75;

    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        ch->stance[ch->stance[0]] * 0.5)
    {
     if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      chance += 25;
     else
      chance += 15;
    }

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        check_improve(ch,gsn_sixth_attack,TRUE,6);
        if (ch->fighting != victim )
            return;
	do_fear(ch,victim);
    }
    
    // Fifth Dual
    chance = get_skill(ch,gsn_dual_wield);
    if(chance <= 100) 
     chance = 0;   // 0 without
    else
     chance /= 9; //18% with mastery
        
    if (chance != 0)
    {
    	if (IS_AFFECTED(ch,AFF_SLOW))
        	chance /= 2;

    	if(IS_AFFECTED(ch,AFF_HASTE))
      		chance *= 1.5;

	    if (IS_SET(ch->stance_aff,STANCE_AFF_SPEED) && number_percent() <
        	ch->stance[ch->stance[0]] * 0.5)
    		{
     		if (IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED))
      			chance += 25;
     		else
      			chance += 15;
    		}

    	if (get_eq_char(ch,WEAR_SECONDARY) != NULL
        	&& ch->pcdata->learned[gsn_sixth_attack] > 10)
    	{
        	if (number_percent( ) < chance)
        	{
         	one_hit(ch,victim,dt,TRUE);
         	check_improve(ch,gsn_dual_wield,TRUE,6);
         	if (ch->fighting != victim)
                	return;
	 	do_fear(ch,victim);
        	}
        	else
        	check_improve(ch,gsn_dual_wield,FALSE,5);
    	}
    }

     if (falcon_here == TRUE)
       multi_hit( ch->pet, victim, TYPE_UNDEFINED );


/* ---------- THE LINE ------ GOING BACK TO REAL COMBAT --END----- */


/* Damage show
      if(wield != NULL)
      {
	sprintf(buf,"{BYou attack $N with $p {W[%d]{B times for {y[%d]{B damage!{x", count, ch->pcdata->round_dam);
	sprintf(buf2,"{B$n attacks you with $p {W[%d]{B times for {y[%d]{B damage!{x", count, ch->pcdata->round_dam);
      }
      else
      {
	sprintf(buf,"{BYou attack $N with your fists {W[%d]{B times for {y[%d]{B damage!{x", count, ch->pcdata->round_dam);
	sprintf(buf2,"{B$n attacks you with your fists {W[%d]{B times for {y[%d]{B damage!{x", count, ch->pcdata->round_dam);
      }
     if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SHORT_COMBAT))
      act( buf, ch, wield, victim, TO_CHAR );
     if (!IS_NPC(victim) && IS_SET(victim->act, PLR_SHORT_COMBAT))
      act( buf2, ch, wield, victim, TO_VICT );
*/
	show_damage_short(ch, victim);
	/*
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SHORT_COMBAT))
	{
		rounddam = 0;
		rounddam = ch->pcdata->round_dam;
		sprintf(buf,"{BAttacking: {W$N {BDamage Done: {W%d{x{x",rounddam);
		act( buf, ch, NULL, victim, TO_CHAR );
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_SHORT_COMBAT))
	{
		rounddam = 0;
		rounddam = victim->pcdata->round_dam;
		sprintf(buf2,"{BAttacked by: {W$n {BDamage Taken: {W%d{x{x",rounddam);
              	act( buf2, ch, NULL, victim, TO_CHAR );
	}
	*/
	return;
}

void show_damage_short(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MSL];
	char buf2[MSL]; 
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SHORT_COMBAT))
	{
		rounddam = 0;
		rounddam = ch->pcdata->round_dam;
		sprintf(buf,"{BAttacking: {W$N {BDamage Done: {W%d{x{x",rounddam);
		act( buf, ch, NULL, victim, TO_CHAR );
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_SHORT_COMBAT))
	{
		rounddam = 0;
		rounddam = ch->pcdata->round_dam;
		sprintf(buf2,"{BAttacked by: {W$n {BDamage Taken: {W%d{x{x",rounddam);
              	act( buf2, ch, NULL, victim, TO_VICT );
	}
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
	int chance, number;
	CHAR_DATA *vch, *vch_next;
	char buf[MSL];
	char buf2[MSL];
	int rounddam = 0;

	if (ch->stunned)
		return;

    // RE-init as Zero
    ch->pIndexData->round_dam = 0;

    // First Hit -- Everyone Gets One
    one_hit( ch, victim, dt, FALSE );

    if (ch->fighting != victim)
        return;

    /* Area attack -- BALLS nasty! */
    if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
           vch_next = vch->next;
           if ((vch != victim && vch->fighting == ch))
             one_hit (ch, vch, dt, FALSE);
        }
    }

    if (IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
        one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
        return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
      chance /= 2;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );

        if ( ch->fighting != victim )
            return;
    }

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_ambush
    || dt == gsn_circle || dt == gsn_twirl || dt == gsn_bladedance)
        return;

    chance = get_skill(ch,gsn_dual_wield)/2;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
      chance /= 2;

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         if (ch->fighting != victim)
                return;
        }
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if ( number_percent( ) < chance )
    {
        one_hit( ch, victim, dt, FALSE );

        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_dual_wield)/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE)|| (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         if (ch->fighting != victim)
                return;
        }
    }

    chance = get_skill(ch,gsn_fourth_attack)/5;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        if (ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_dual_wield)/5;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);

         if (ch->fighting != victim)
                return;
        }
    }

    chance = get_skill(ch,gsn_fifth_attack)/5;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        if (ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_dual_wield)/5;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         if (ch->fighting != victim)
                return;
        }
    }

    chance = get_skill(ch,gsn_sixth_attack)/5;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if ( number_percent() < chance )
    {
        one_hit( ch, victim, dt, FALSE );
        if (ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_dual_wield)/6;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
        chance /= 2;

    if(IS_AFFECTED(ch,AFF_HASTE) || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
      chance *= 2;

    if (get_eq_char(ch,WEAR_SECONDARY) != NULL)
    {
        if (number_percent( ) < chance)
        {
         one_hit(ch,victim,dt,TRUE);
         if (ch->fighting != victim)
                return;
        }
    }

	rounddam = 0;

	if (!IS_NPC(ch))
		rounddam = ch->pcdata->round_dam;
	else
		rounddam = ch->pIndexData->round_dam;

	rounddam = 0;

	sprintf(buf,"{BAttacking: {W$N {BDamage Done: {W%d{x{x",rounddam);

	if (!IS_NPC(ch))
		rounddam = ch->pcdata->round_dam;
	else
		rounddam = ch->pIndexData->round_dam;

	sprintf(buf2,"{BAttacked by: {W$n {BDamage Taken: {W%d{x{x",rounddam);

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SHORT_COMBAT))
	{
		act( buf, ch, NULL, victim, TO_CHAR );
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_SHORT_COMBAT))
	{
              	act( buf2, ch, NULL, victim, TO_VICT );
	}

	/* oh boy!  Fun stuff! */

	if (ch->wait > 0)
		return;

	number = number_range (0, 2);

       /* if (number == 1 && IS_SET (ch->act, ACT_MAGE))
	{
                  { mob_cast_mage(ch,victim); return; } ;
	}

	if (number == 2 && IS_SET (ch->act, ACT_CLERIC))
	{
              { mob_cast_cleric(ch,victim); return; }  ;
        } */

	/* now for the skills */

	number = number_range (0, 8);

	switch (number)
	{
	case (0):
		if (IS_SET (ch->off_flags, OFF_BASH))
			do_bash (ch, "");
		break;

	case (1):
		if (IS_SET (ch->off_flags, OFF_BERSERK) && !IS_AFFECTED (ch, AFF_BERSERK))
			do_berserk (ch, "");
		break;


	case (2):
		if (IS_SET (ch->off_flags, OFF_DISARM)
			|| (get_weapon_sn (ch) != gsn_hand_to_hand
				&& (IS_SET (ch->act, ACT_WARRIOR)
					|| IS_SET (ch->act, ACT_VAMPIRE)
					|| IS_SET (ch->act, ACT_THIEF))))
			do_disarm (ch, "");
		break;

	case (3):
		if (IS_SET (ch->off_flags, OFF_KICK))
			do_kick (ch, "");
		break;

	case (4):
		if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
			do_dirt (ch, "");
		break;

	case (5):
		if (IS_SET (ch->off_flags, OFF_TAIL))
		{
			/* do_tail(ch,"") */ ;
		}
		break;

	case (6):
		if (IS_SET (ch->off_flags, OFF_TRIP))
			do_trip (ch, "");
		break;

	case (7):
		if (IS_SET (ch->off_flags, OFF_CRUSH))
		{
			/* do_crush(ch,"") */ ;
		}
		break;
	case (8):
		if (IS_SET (ch->off_flags, OFF_BACKSTAB))
		{
			do_backstab (ch, "");
		}
	}
}


/*
 * Hit one guy once.
 */
void one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
        
	OBJ_DATA *wield,*second;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam = 0;
	int diceroll;
	int sn, skill;
        int dam_type,wdam;
	bool result;
	bool show = TRUE;
        int adice,bdice;
        //char buf [MAX_STRING_LENGTH];

	sn = -1;
 
        /* just in case */
	if (victim == ch || ch == NULL || victim == NULL)
		return;

    if (!IS_NPC(ch) && !IS_NPC(victim) && !is_safe(ch, victim) && !is_safe(victim,ch)
    && ch != victim) {
	if(!IN_ARENA(ch) && !IN_ARENA(victim))
	{
	     ch->fight_timer = pktimer;
	     victim->fight_timer = pktimer;
	}
    }
	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if (victim->position == POS_DEAD || (IS_SET(victim->plyr,PLAYER_GHOST) && !IS_NPC(ch)) || ch->in_room != victim->in_room)
		return;

        ch->fighting = victim;

        /*
	 * Figure out the type of `damage message.
	 * if secondary == true, use the second weapon.
	 */
	if (!secondary)
		wield = get_eq_char (ch, WEAR_WIELD);
	else
		wield = get_eq_char (ch, WEAR_SECONDARY);
	if (dt == TYPE_UNDEFINED)
	{
		dt = TYPE_HIT;
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
			dt += wield->value[3];
		else
			dt += ch->dam_type;
	}

	if (dt < TYPE_HIT)
		if (wield != NULL)
			dam_type = attack_table[wield->value[3]].damage;
		else
			dam_type = attack_table[ch->dam_type].damage;
	else
		dam_type = attack_table[dt - TYPE_HIT].damage;

	if (dam_type == -1 || dt == gsn_kick)
		dam_type = DAM_BASH;

	/* get the weapon skill */
	sn = get_weapon_sn (ch);
	skill = 20 + get_weapon_skill (ch, sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC (ch))
	{
                thac0_00 = 20;
                thac0_32 = -4;                  /* as good as a thief */
		if (IS_SET (ch->act, ACT_VAMPIRE))
                        thac0_32 = -20;
		else if (IS_SET (ch->act, ACT_DRUID))
			thac0_32 = 0;
		else if (IS_SET (ch->act, ACT_RANGER))
			thac0_32 = -4;
		else if (IS_SET (ch->act, ACT_WARRIOR))
			thac0_32 = -10;
		else if (IS_SET (ch->act, ACT_THIEF))
			thac0_32 = -4;
		else if (IS_SET (ch->act, ACT_CLERIC))
			thac0_32 = 2;
		else if (IS_SET (ch->act, ACT_MAGE))
			thac0_32 = 6;
	}
	else
	{
		thac0_00 = class_table[ch->class].thac0_00;
		thac0_32 = class_table[ch->class].thac0_32;
	}
	thac0 = interpolate (ch->level, thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0 / 2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

        thac0 -= GET_HITROLL(ch) * skill / 100;
	thac0 += 5 * (100 - skill) / 100;

	if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));
		if (dt == gsn_garrote)
		thac0 -= 10 * (100 - get_skill (ch, gsn_garrote));

        if (dt == gsn_ambush)
                thac0 -= 10 * (100 - get_skill (ch, gsn_ambush));
        if (dt == gsn_bladedance)
                thac0 -= 10* (100 - get_skill (ch, gsn_bladedance));
	if (dt == gsn_circle)
		thac0 -= 10 * (100 - get_skill (ch, gsn_circle));

        if (dt == gsn_twirl)
                thac0 -= 10 * (100 - get_skill (ch, gsn_twirl));

        if (dt == gsn_vital_hit)
                thac0 -= 15 * (100 - get_skill (ch, gsn_vital_hit));

	switch (dam_type)
	{
        case (DAM_PIERCE): victim_ac = GET_AC (victim, AC_PIERCE) / 10; break;
        case (DAM_BASH):  victim_ac = GET_AC (victim, AC_BASH) / 10; break;
        case (DAM_SLASH): victim_ac = GET_AC (victim, AC_SLASH) / 10; break;
        default: victim_ac = GET_AC (victim, AC_EXOTIC) / 10; break;
	};

/*
************* Hit Chance affected by AC *************
*/
	if (victim_ac < -15 && victim_ac > -31) 
		victim_ac = (victim_ac + 15) / 5 - 15; 
	else if (victim_ac < -30 && victim_ac > -41) 
		victim_ac = (victim_ac + 15) / 5 - 20; 
	else if (victim_ac < -40 && victim_ac > -51) 
		victim_ac = (victim_ac + 15) / 5 - 25; 
	else if (victim_ac < -50 && victim_ac > -61) 
		victim_ac = (victim_ac + 15) / 5 - 30; 
	else if (victim_ac < -60 && victim_ac > -71) 
		victim_ac = (victim_ac + 15) / 5 - 35; 
	else if (victim_ac < -70 && victim_ac > -81) 
		victim_ac = (victim_ac + 15) / 5 - 40; 		
	else if (victim_ac < -80 && victim_ac > -91) 
		victim_ac = (victim_ac + 15) / 5 - 45; 
	else if (victim_ac < -90 && victim_ac > -101) 
		victim_ac = (victim_ac + 15) / 5 - 50; 
	else if (victim_ac < -100 && victim_ac > -111) 
		victim_ac = (victim_ac + 15) / 5 - 55; 
	else if (victim_ac < -110 && victim_ac > -121) 
		victim_ac = (victim_ac + 15) / 5 - 60; 
	else if (victim_ac < -120 && victim_ac > -131) 
		victim_ac = (victim_ac + 15) / 5 - 65; 
	else if (victim_ac < -130 && victim_ac > -141) 
		victim_ac = (victim_ac + 15) / 5 - 70; 
	else if (victim_ac < -140) 
		victim_ac = (victim_ac + 15) / 5 - 75; 

/*
*************** Added in, 1/12/05 **************
*/

/* NIGHTSTALKER ABILITY added by Dusk */

	if ( IS_SET( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER)
	     && (vam_str[time_info.hour] != 0) )
	{
     	// Day time
           if ((time_info.hour > 6) && (time_info.hour < 18))
	   {
	      victim_ac += ((victim_ac * vam_str[time_info.hour]) / 10);
	   }
	   else // Night time
	   {
	      victim_ac -= ((victim_ac * vam_str[time_info.hour]) / 10);
	   }
	}

/* END OF NIGHTSTALKER ABILITY BY DUSK */

	if (!can_see (ch, victim))
                victim_ac -= 10;

	if (victim->position < POS_FIGHTING)
		victim_ac += 4;

	if (victim->position < POS_RESTING)
		victim_ac += 6;

        /* Lets try this... */
/* Damage show
        if(IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->act,PLR_SHORT_COMBAT))
        show = FALSE;
        else if(!IS_NPC(ch) && IS_SET(ch->act, PLR_SHORT_COMBAT))
        show = FALSE;
        else
*/
        show = TRUE;

	/*
	 * The moment of excitement!
	 */
	while ((diceroll = number_bits (5)) >= 20)
		;

	if (diceroll == 0
		|| (diceroll != 19 && diceroll < thac0 - victim_ac))
	{
		/* Miss. */
		damage (ch, victim, 0, dt, dam_type, show, 0);
		tail_chain ();
                if (!is_safe(ch,victim)) {
                    improve_stance(ch);
                }
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */
	if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
		if (!ch->pIndexData->new_format)
		{
			dam = number_range (ch->level / 2, ch->level * 3 / 2);
			if (wield != NULL)
				dam += dam / 2;
		}
		else
		{
			dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
		}
	else
	{
		if (sn != -1)
			check_improve (ch, sn, TRUE, 5);
		if (wield != NULL)
		{
			if (wield->clan)
			{
				float adlev, inclev;
				int cntr;

				adlev = 8;
				inclev = .01;
				for (cntr = 0; cntr <= ch->level; cntr++)
				{
					adlev += .57;
					adlev += inclev;
					inclev += .005;
				}
				cntr = (int) adlev;
				dam = dice (cntr / 3, 3) * skill / 100;
			}
			else
			{
                         adice = wield->value[1];
                         bdice = wield->value[2];

                          if (wield->pIndexData->new_format)
                           bdice *= (skill/100);
                          else
                          {
                           adice *= (skill/100);
                           bdice *= (skill/100);
                          }

                         switch (wield->value[0])
                         {
                          case(WEAPON_SWORD):
                           adice += number_range(-2,2);
                           bdice += number_range(-2,2);
                           break;
                          case(WEAPON_DAGGER):
                           adice += number_range(-3,2);
                           bdice += number_range(-3,2);
                           break;
                          case(WEAPON_SPEAR):
                           adice += number_range(-4,4);
                           bdice += number_range(-4,4);
                           break;
                          case(WEAPON_MACE):
                           adice += number_range(-2,2);
                           bdice += number_range(-2,2);
                           break;
                          case(WEAPON_AXE):
                           adice += number_range(-3,3);
                           bdice += number_range(-3,3);
                           break;
                          case(WEAPON_FLAIL):
                           adice += number_range(-2,2);
                           bdice += number_range(-2,2);
                           break;
                          case(WEAPON_WHIP):
                           adice += number_range(-2,1);
                           bdice += number_range(-2,1);
                           break;
                          case(WEAPON_POLEARM):
                           adice += number_range(-2,2);
                           bdice += number_range(-2,2);
                           break;
                          case(WEAPON_EXOTIC):
                           adice += number_range(-3,1);
                           bdice += number_range(-3,1);
                           break;
                          default:      break;
                         }

                          dam = dice(adice,bdice);
			}

			if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
                                dam = dam * 11 / 10;

                        /* If weapon is 15 levels or more higher than ch */
                        if (wield->level - ch->level >= 15)
                           dam = (dam*8)/10;
                        else if (wield->level - ch->level >= 5)
                           dam = (dam*9)/10;

                        // Vorpal Weapons
                        /* Chance for Vorpal hit, if miss, then sharp */
                        if (IS_WEAPON_STAT (wield, WEAPON_VORPAL)) {
                          int percent;
                          if ((percent = number_percent ()) <= (skill / 10))
                              dam = 2 * dam + (dam * 2 * percent / 50);
                          else if ((percent = number_percent ()) <= (skill / 8))
                             dam = 2 * dam + (dam * 2 * percent / 100);
                        }
                        // Sharp Weapons
                        else if (IS_WEAPON_STAT (wield, WEAPON_SHARP)) {
                          int percent;
                          if ((percent = number_percent ()) <= (skill / 8))
                             dam = 2 * dam + (dam * 2 * percent / 100);
                        }
                        if(IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS)) {
                         int percent;
                         percent = number_percent()/2;
                         if(number_percent() < 15)
                         {
                          act("You swing $p in a high arc and come down on $N!",ch,wield,victim,TO_CHAR);
                          act("$n swings $p in a high arc and comes down on $N!",ch,wield,victim,TO_NOTVICT);
                          dam = 1.5 * dam + (dam * 5 * percent / 75);
                         }
                         else
                         {
                             dam = 1.5 * dam + (dam * 1.5 * percent / 90);
                         }
                        }
		}
		else
                {
                 if(((ch->class != CLASS_MONK) && (ch->class != CLASS_NINJA)) || (!is_affected(ch,gsn_martial_arts)))
                  dam = number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
                 else
                 {
                       int sides, ndice;
                       ndice = 4;  //12.5
                       sides = 5;
                       if (ch->class == CLASS_MONK && ch->level > 100) {
                         ndice = 8;   //100
                         sides = 16;
                       }
                       else if (ch->class == CLASS_MONK && ch->level > 80) {
                         ndice = 8;   //90
                         sides = 15;
                       }
                       else if (ch->class == CLASS_NINJA && ch->level > 100) {
                         ndice = 8;   //90
                         sides = 14;
                       }
                       else if (ch->level > 70) {
                         ndice = 7;    //70
                         sides = 14;
                       }
                       else if (ch->level > 60) {
                         ndice = 7;    //63
                         sides = 11;
                       }
                       else if (ch->level > 40) {
                         ndice = 6;    //44
                         sides = 8;
                       }
                       else if (ch->level > 20) {
                         ndice = 5;    //22.5
                         sides = 7;
          				        
                     
                    }

                                                  
					dam =  number_range (ndice, ndice*sides);
//					dam = dam - 75;
					
					
/* New Vital Hit By Dusk */                     
                     
               if(has_skill(ch,gsn_vital_hit) && (is_affected(ch,gsn_martial_arts))) 
        	     { 
         		if((get_skill(ch,gsn_vital_hit) / 10) > number_percent( )) 
         		{
                          check_improve(ch,gsn_vital_hit,TRUE, 3);
                          
                          int location;
                          location = number_range(0,6);
                          dt = gsn_vital_hit;
                          AFFECT_DATA af;

                          if ( location == 0 || location == 2 || location == 4 || location == 6 )
         		  {                        
                             send_to_char("{MYour precise movements have granted a vital hit!{x\n\r",ch);
                             send_to_char("{CTheir precise movements targeted a vital area!{x\n\r",victim);
                             
                             if (location == 0)
                                dam *= 1.10;
                             if (location == 2)
                                dam *= 1.20;
                             if (location == 4)
                                dam *= 1.30;
                             if (location == 6)
                                dam *= 1.40;
                             
                          }
                          else if ( location == 1 && !IS_AFFECTED(victim,AFF_BLIND)  )
                          {
                             if ( !IS_NPC(victim) )
                             {
                                if ( ( number_percent() + get_curr_stat(victim,STAT_DEX)*3 + victim->pcdata->focus[COMBAT_AGILITY] )
                                   < ( number_percent() + get_skill(ch,gsn_vital_hit) ) ) 
                                {
                                   send_to_char("{MYour precise movements target their eyes!{x\n\r",ch);
                                   send_to_char("{CTheir precise movements targeted your eyes!{x\n\r",victim);
                                                                             
                                   af.where     = TO_AFFECTS;
                                   af.type      = gsn_vital_hit;
                                   af.level     = ch->level;
                                   af.duration  = 1;
                                   af.location  = APPLY_HITROLL;
                                   af.modifier  = ch->level/2 * -1;
                                   af.bitvector = AFF_BLIND;

                                   affect_to_char (victim, &af);                              
                             
                                   one_hit(ch,victim,gsn_vital_hit,FALSE);
                                }
                             }
                             else if ( ( number_percent() < ( number_percent() + get_skill(ch,gsn_vital_hit) ) ) && IS_NPC(victim)  )
                             {
                                af.where     = TO_AFFECTS;
                                af.type      = gsn_vital_hit;
                                af.level     = ch->level;
                                af.duration  = 2;
                                af.location  = APPLY_HITROLL;
                                af.modifier  = ch->level;
                                af.bitvector = AFF_BLIND;
      
                                affect_to_char (victim, &af);
 
                                one_hit(ch,victim,gsn_vital_hit, FALSE);
                             }
                             else                               
                             {
                                send_to_char("{mThey were able to dodge your vital eye strike!{x\n\r",ch);
                                send_to_char("{mYou dodged their vital eye strike!{x\n\r",victim);
                                dam *= .75;
                             }          
                          }
                          else if ( location == 1 && IS_AFFECTED(victim,AFF_BLIND) )
                          {
                             send_to_char("{MTheir eyes have already been blinded!{x\n\r",ch);
                             send_to_char("{CTheir precise movements inflict pain upon your eyes!{x\n\r",victim);
                             dam *= 1.1;
                          }
                          else if ( location == 3 )
                          {
                             if ( !IS_NPC(victim) )
                             {
                                if ( ( number_percent() + get_curr_stat(victim,STAT_CON)*2 + get_curr_stat(victim,STAT_DEX)*2 +
                                   ( victim->pcdata->focus[COMBAT_TOUGHNESS] + victim->pcdata->focus[COMBAT_AGILITY] ) /2 ) <
                                   ( number_percent() + get_skill(ch,gsn_vital_hit) ) )
                                {
                                   send_to_char("{MYour precise movements targets their hamstrings!{x\n\r",ch);
                                   send_to_char("{CTheir precise movements targeted your hamstrings!{x\n\r",victim);
                                   dam *= 1.15;
                                   DAZE_STATE (victim, PULSE_VIOLENCE);
                                }
                             }
                             else if ( IS_NPC(victim) )
                             {
                                send_to_char("{MYour precise movements targets their hamstrings!{x\n\r",ch);
                                dam *= 1.15;
                                DAZE_STATE (victim, 2 *PULSE_VIOLENCE);
                             }
                             else
                             {
                                send_to_char("{mYour movements did not cause enough pain on their hamstrings!{x\n\r",ch);
                                send_to_char("{mYour agility and toughness prevailed over their hamstring attempt!{x\n\r",ch);
                                dam *= .75;
                             }
                             
                          }
                          else if ( location == 5 )
                          {           
                             if ( !IS_NPC(victim) )
                             {                                      
                                if ( ( ( number_percent() + get_curr_stat(victim,STAT_CON)*3 + victim->pcdata->focus[COMBAT_TOUGHNESS] )
                                   < ( number_percent() + get_skill(ch,gsn_vital_hit) ) ) && !IS_NPC(victim) )
                                {
                                   send_to_char("{MYour precise movements targets their solarplexis!{x\n\r",ch);
                                   send_to_char("{CTheir precise movements targeted your solarplexis!{x\n\r",victim);
                                   dam *= .75;
                                   victim->stunned = 1;
                                   act("{(You are stunned, and have trouble getting back up!{x",ch,NULL,victim,TO_VICT);
                                   act("{.$N is stunned by your vital hit!{x",ch,NULL,victim,TO_CHAR);
                                }
                             }
                             else if ( IS_NPC(victim) )
                             {
                                send_to_char("{MYour precise movements targets their solarplexis{x\n\r",ch);
                                dam *= 1.50;
                                victim->stunned = 1;
                                act("{.$N is stunned by your vital hit!{x",ch,NULL,victim,TO_CHAR);
                             }
                             else
                             {
                                send_to_char("{mYour vital strike hits their solarplexis but has no effect!{x\n\r",ch);
                                send_to_char("{mTheir vital strike hits your solarplexis but bounces off!{x\n\r",ch);
                                dam *= .75;
                             }
                          }
              	        } 
                        else
                           check_improve(ch,gsn_vital_hit,FALSE,1);
        	     }
             }
                }
        }
/*
         Crusader Sunder Armor Class Ability by Dusk Added 10/10/08 

        if ( !IS_NPC(ch) && ch->class == CLASS_CRUSADER && IS_SET(ch->ability_crusader,ABILITY_CRUSADER_SUNDER) )
        {
           AFFECT_DATA af,*paf;       

           int acmod=0,factor=0, abonus=0, hfactor=0, sfactor=0,acvic=0,ac=0,hf=0,fbonus=0,hbonus=0,tosunder = 0;
           int sunderamount = 0;

           factor = ( get_skill ( ch, gsn_second_attack ) + get_skill ( ch, gsn_third_attack ) +
                      get_skill ( ch, gsn_fourth_attack ) + get_skill ( ch, gsn_fifth_attack ) ) / 2;
        
           abonus = ( ch->alignment - victim->alignment ) / 100;
           
           hfactor = GET_HITROLL (ch);

           sfactor = ( ( victim->saving_throw * -1 ) * get_curr_stat( victim, STAT_WIS ) ) / 10;

           acvic = ( ( GET_AC( victim, AC_PIERCE) + GET_AC( victim, AC_BASH ) + GET_AC( victim, AC_SLASH ) +
                     GET_AC( victim, AC_EXOTIC) ) * -1 ) / 4;           
           
           hf = hfactor + factor + number_percent();
           ac = ( acvic * .75 + number_percent() ) / 2;

           abonus = ( abonus < 0 ? 0 : abonus );

           tosunder = ( hf <= ac ? ( ac - hf ) / 25 : ( ( hf - ac ) / 10 ) *-1  );
       
           acmod = abonus + ( ( hfactor - sfactor ) / 10 ) + number_range( 1, 10 );
           
           acmod = ( acmod < 0 ? 0 : acmod );

           if ( !IS_NPC(victim) )
           {
                 fbonus = ch->pcdata->focus[MAGIC_ABILITY] - victim->pcdata->focus[MAGIC_DEFENSE];
                 hbonus = ch->pcdata->focus[COMBAT_AGILITY] - victim->pcdata->focus[COMBAT_DEFENSE];
                 
                 hbonus = ( hbonus < 0 ? 0 : hbonus );
                 
                 fbonus = ( fbonus < 0 ? 0 : hbonus );
                                 
                 tosunder = tosunder + hbonus;
                 acmod = abonus + fbonus + ( ( hfactor - sfactor ) / 10 ) + number_range( 1, 10 );
           }

           for ( paf = victim->affected; paf != NULL ; paf = paf->next )
           {
               switch( paf->location )
               {
                     case APPLY_AC:
                     sunderamount += paf->modifier;
                     break;
               } 
           }

           if ( ( ( tosunder + number_percent() ) >= number_percent() ) && ( ch->mana >= acmod/5 ) && ( acmod != 0 ) 
              &&  ( sunderamount < ( ( ( acvic * -1 - sunderamount ) * -1)  / 2 ) ) )
           {
              sprintf(buf,"{yYour faith favors you! Your sunder raises their ac by {Y%d{y!{x\n\r",acmod);
              send_to_char( buf, ch );
                              
              ch->mana -= ( acmod < 2 && acmod >= 0 ? 1 : acmod/2 );
             
              dam *= 4;
                         
              af.where     = TO_AFFECTS;
              af.type      = gsn_sunder;
              af.level     = ch->level;
              af.duration  = 1;
              af.location  = APPLY_AC;
              af.modifier  = acmod;
              af.bitvector = 0;           
    
              affect_join( victim, &af );
           }
        }
End of new Crusader Ability Sunder by Dusk */
	/*
	 * Bonuses.
	 */
/* Start of Nightstalker Ability by Dusk  */

	if ( IS_SET( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER ) && (vam_str[time_info.hour] != 0 ) )
	{
		if ((time_info.hour > 6) && (time_info.hour < 18))
		{
			dam -= ((dam * vam_str[time_info.hour]) / 10);
		}
		else
		{
			dam += ((dam * vam_str[time_info.hour]) / 10);
		}
	}

/* End of Nightstalker Ability by Dusk */

        // Add Bonuses here

        if (ch->class == CLASS_HIGHLANDER && get_skill(ch, gsn_blademastery) > 0)
        {
           int hroll;

           hroll = number_percent ();

           if (hroll*7 <= get_skill (ch, gsn_blademastery))
 	   {
              check_improve (ch, gsn_blademastery, TRUE, 3);
              dt = gsn_blademastery;
              dam_type = DAM_SLASH;
              if(number_percent() < 30)
              {
               hroll = number_range(2,ch->pcdata->power[POWER_LEVEL]);

               if(hroll > 2) {
                hroll *= 6/7;
               }

               dam *= hroll;
             }
           }
        }

        dam = add_bonuses( ch, victim, dam, dt, dam_type );

        if ((dt == gsn_assassinate) && wield != NULL)
        	dam *= 16;
        	if (dt == gsn_garrote)
        	dam *= 14;

        /* Added bonus for dual wielding daggers for Assassins
            Dusk 
        */
     

       wield = get_eq_char (ch, WEAR_WIELD);
       second = get_eq_char (ch, WEAR_SECONDARY);
     
       if (second != NULL )
       {
           if ( (dt == gsn_backstab) && (wield->value[0] == WEAPON_DAGGER) &&
                (second->value[0] == WEAPON_DAGGER) && (ch->class == CLASS_ASSASSIN) )
           {
              if (ch->level <= 15)
                 dam *= 2;
              else if (ch->level <= 31)
                      dam *= 2;
              else if (ch->level <= 47)
                      dam *= 2;
              else if (ch->level <= 63)
                      dam *= 2;
              else if (ch->level <= 79)
                      dam *= 2;
              else if (ch->level <= 91)
                      dam *= 2;
              else if (ch->level <= 101)
                      dam *= get_skill(ch,gsn_assassinate)/40;
              else
                      dam *= get_skill(ch,gsn_assassinate)/40;
           }
     
           else if ( (dt == gsn_backstab) && (wield->value[0] == WEAPON_DAGGER) &&
                     (second->value[0] != WEAPON_DAGGER)  && (ch->class == CLASS_ASSASSIN) )
           {
                   if (ch->level <= 15)
                      dam *= 2;
                   else if (ch->level <= 31)
                           dam *= 2;
                   else if (ch->level <= 47)
                           dam *= 2;
                   else if (ch->level <= 63)
                           dam *= 2;
                   else if (ch->level <= 79)
                           dam *= 2;
                   else if (ch->level <= 91)
                           dam *= 2;
                   else if (ch->level <= 101)
                      dam *= get_skill(ch,gsn_assassinate)/50;
                   else
                      dam *= get_skill(ch,gsn_assassinate)/50;
           }
       }

        /*
         Removed the CLASS_ASSASSIN check because of above addition of
           dual wield backstab bonus
             Dusk
        */

        if ( (dt == gsn_backstab) && (wield != NULL) &&
	     (ch->class == CLASS_BARD || ch->class == CLASS_NINJA) )
        {
                if (ch->level <= 15)
			dam *= 2;
                else if (ch->level <= 31)
			dam *= 4;
                else if (ch->level <= 47)
			dam *= 6;
                else if (ch->level <= 63)
			dam *= 8;
                else if (ch->level <= 79)
			dam *= 10;
                else if (ch->level <= 91)
			dam *= 12;
		else if (ch->level <= 101)
                        dam *= 17;
		else
                        dam *= 15;
	}
        else if ((dt == gsn_backstab) && (wield != NULL)) {
                if (ch->level <= 15)
		dam *=2;
		else if (ch->level <=31)
		dam *=3;
		else if (ch->level <=47)
		dam *=5;
		else if (ch->level <=63)
		dam *=7;
		else if (ch->level <=79)
		dam *=9;
		else if (ch->level <=91)
		dam *=11;
		else if (ch->level <=101)
		dam *=12;
		else
		dam *=13;
	}
    if (dt == gsn_bladedance && wield != NULL)  {
                if (ch->level <= 19)
                        dam *= 2;
                else if (ch->level <= 39)
                        dam *= 3;
                else if (ch->level <= 59)
                        dam *= 5;
                else if (ch->level <= 79)
                        dam *= 7;
                else if (ch->level <= 101)
                        dam *= 10;
                else
                        dam *= 13;
        }

	if (dt == gsn_circle && wield != NULL)	{
		if (ch->level <= 19)
			dam *= 2;
		else if (ch->level <= 39)
			dam *= 3;
		else if (ch->level <= 59)
			dam *= 5;
		else if (ch->level <= 79)
			dam *= 7;
		else if (ch->level <= 101)
			dam *= 9;
		else
                        dam *= 12;
        }


        if (dt == gsn_vital_hit && wield == NULL)
        {
           if (ch->level <= 82)
              dam *= 4;
           else if (ch->level <= 92)
              dam *= 5;
           else if (ch->level <= 101)
              dam *= 11/2;
           else
              dam *= 6;
        }

        if (dt == gsn_twirl && wield != NULL)  {
                if (ch->level <= 29)
						dam *= 3;
                else if (ch->level <= 49)
                        dam *= 4;
                else if (ch->level <= 79)
                        dam *= 5;
		else if (ch->level <= 101)
                        dam *= 6;
		else
                        dam *= 10;

	}

	if (dt == gsn_ambush && wield != NULL)	{
		if (wield->value[0] != 2)
			dam *= 3 / 2 + (ch->level / 10);
		else
			dam *= 3 / 2 + (ch->level / 8);
	}

        dam += GET_MOD_DAMROLL(ch) * UMIN(110,skill)/100;

        if (!IS_NPC(ch))
        if (!str_cmp(class_table[ch->class].name,"monk")
         || !str_cmp(class_table[ch->class].name,"ninja"))
        {
         if (wield == NULL && get_eq_char(ch,WEAR_SECONDARY) == NULL)
          dam += (dam * 11) / 10;
        }

         /*
          *  Fists and Affects -- Monk/Ninja/Voodan
          */

                  if (is_affected(ch,gsn_fists_fang) && dt != gsn_backstab && dt != gsn_circle)
         {
          if (number_percent() < ( get_skill(ch,gsn_fists_fang) / 3) )
          {
           send_to_char("You lash out with your fangs!\n\r",ch);
           act("$n strikes with the fangs on his wrists!",ch,0,victim,TO_ROOM);

           dt = gsn_fists_fang;
           dam_type = DAM_PIERCE;
           dam = dam + (dam*1/10);

           if (!saves_spell(ch->level/ 2,victim,DAM_POISON))
           {
                AFFECT_DATA af;
                send_to_char("You feel poison running in your blood.\n\r",victim);
                act("$N is poisoned by your fangs!",ch,0,victim,TO_CHAR);
                af.where     = TO_AFFECTS;
                af.type      = gsn_poison;
                af.level     = ch->level * 5/4;
                af.duration  = ch->level / 4;
                af.location  = APPLY_STR;
                af.modifier  = -1*number_range(1,6);
                af.bitvector = AFF_POISON;
                affect_join( victim, &af);
            }
          }
          
        }
        if (is_affected(ch,gsn_fists_flame))
        {
          if (number_percent() < ( get_skill(ch,gsn_fists_flame) / 5 ) )
          {
                send_to_char("A flaming aura surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by a flaming aura!",ch,0,victim,TO_ROOM);
                dt = gsn_fists_flame;
                dam_type = DAM_FIRE;
                dam = dam + (dam*2/10);
                //Flaming Flag
                if ( number_range( 1,2 ) == 2 ) //50% chance (keep in mind fists don't hit every punch)
                {
	               act("$n is burned by the flames!",victim,0,NULL,TO_ROOM);
                   act("Your flesh is seared by the flames!",victim,0,NULL,TO_CHAR);
	               victim->hit =  (victim->hit - 20); //20 hidden bonus damage
                   fire_effect( (void *) victim,ch->level*7/10,dam,TARGET_CHAR); //slightly lower level than weapon flaming
                   update_pos( victim );
               }
          }
        }
        if (is_affected(ch,gsn_fists_divinity))
        {
          if (number_percent() < (get_skill(ch,gsn_fists_divinity) / 5))
          {
                send_to_char("A bright aura surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by a bright aura!",ch,0,victim,TO_ROOM);
                dt = gsn_fists_divinity;
                dam_type = DAM_HOLY;
                dam = dam + (dam*2/10); 
                //Shocking flag:
                if (number_range( 1,2 ) == 2) //50% chance 
                {
                   wdam = number_range( 1, ch->level / 5 + 2);
                   act("$n is shocked with holy power!",victim,0,NULL,TO_ROOM);
                   act("You are shocked with divine power!",victim,0,NULL,TO_CHAR);
                   victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
                   shock_effect(victim, ch->level,dam,TARGET_CHAR);
                   update_pos(victim);
               }
          }
        }
        if (is_affected(ch,gsn_fists_ice))
        {
          if (number_percent() < (get_skill(ch,gsn_fists_ice) / 5))
          {
                send_to_char("A cold mist surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by a cold mist!",ch,0,victim,TO_ROOM);
                dt = gsn_fists_ice;
                dam_type = DAM_COLD;
                dam = dam + (dam*2/10); 
                //Frost Flag
                if (number_range( 1,2 ) == 2) //50% chance
                {
                   wdam = number_range( 1, ch->level / 15 + 2);
                   act("$n is frozen by the cold!",victim,0,NULL,TO_ROOM);
                   act("The freezing punch surrounds you with ice!",victim,0,NULL,TO_CHAR);
                   victim->hit = (victim->hit - wdam);
                   cold_effect(victim,ch->level,dam,TARGET_CHAR);
                   update_pos(victim);
               }
          }
        }
    
       /* STORM CRIT STRIKE by Bree */
/*         if(is_affected(ch, gsn_crit_strike))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_crit_strike;
             check_improve(ch,gsn_crit_strike,TRUE,6);
             send_to_char("You land a critical strike!\n\r",ch);           
             act("$n lands a critical strike!", ch, NULL, NULL, TO_ROOM);       
             dam = dam + (dam*4/8);
           }
         }
*/
       /* Manuevers by Bree */
         if(is_affected(ch, gsn_snattack))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_snattack;
             check_improve(ch,gsn_snattack,TRUE,6);
             send_to_char("You manuever in for a quick strike!\n\r",ch);
             act("$n manuevers for a strike!", ch, NULL, NULL, TO_ROOM);
             dam = dam + (dam*4/8);
           }
         }

 /* blasphemy by Bree */
         if(is_affected(ch, gsn_snattacktwo))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_snattacktwo;
             check_improve(ch,gsn_snattacktwo,TRUE,6);
             send_to_char("Your Blasphemy gives you another strike!\n\r",ch);
             act("$n manuevers for a strike!", ch, NULL, NULL, TO_ROOM);
             dam = dam + (dam*4/8);
           }
         }



      /* greed by Bree */
    /*     if(is_affected(ch, gsn_greed))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_greed;
             check_improve(ch,gsn_greed,TRUE,6);
             send_to_char("You see a shiney gold coin and move in for a strike!\n\r",ch);
             act("$n lunges at $N's coin purse! ", ch, NULL, NULL, TO_ROOM);
             dam = dam + (dam*4/8);
           }
         }
*/
    /* bloodlust  by Bree */
         if(is_affected(ch, gsn_bloodlust))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_bloodlust;
             check_improve(ch,gsn_bloodlust,TRUE,6);
             send_to_char("Rage overcomes you as you strike!!\n\r",ch);
             act("$n smells blood and goes {RI{rN{RS{rA{RN{rE{R!!{X", ch, NULL, NULL, TO_ROOM);
             dam = dam + (dam*4/8);
           }
         }


      /* Shadow ungodly blows by Bree */
         if(is_affected(ch, gsn_modis_anger))
         {
           if (number_percent() < (get_skill(ch,gsn_second_attack) / 6.5))
           {
             dt = gsn_modis_anger;
             check_improve(ch,gsn_modis_anger,TRUE,6);
             act("$n strikes with the fury of The Shadows!", ch, NULL, NULL, TO_ROOM);       
             dam = dam + (dam*4/8);
           }
         }
       
       /* Conquest Critical Strike */
         if(is_affected(ch, gsn_divide))
         {
           if (number_percent() < (get_skill(ch,gsn_second_attack) / 8))
           {
             dt = gsn_divide;
             check_improve(ch,gsn_crit_strike,TRUE,6);
             send_to_char("You land a Crushing Blow!\n\r",ch);           
             act("$n lands a crushing Blow!", ch, NULL, NULL, TO_ROOM);       
             dam = dam + (dam*4/7.5);
           }
         }

       /* Vanir Critical Strike */
         if(is_affected(ch, gsn_vmight))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 8))
           {
             dt = gsn_vmight;
             check_improve(ch,gsn_crit_strike,TRUE,6);
             send_to_char("You land a Vanirian Blow!\n\r",ch);           
             act("$n lands a Vanirian Blow!", ch, NULL, NULL, TO_ROOM);       
             dam = dam + (dam*4/8);
           }
         }
       
       if (ch->class == CLASS_HIGHLANDER && get_skill(ch, gsn_follow_through) > 0)
        {
          if (number_percent() < (get_skill(ch,gsn_follow_through) / 5))
          {
	       check_follow_through(ch,victim,dam);
        }        
        }
        if (is_affected(ch,gsn_fists_acid) && dt != gsn_backstab && dt != gsn_circle)
        {
          if (number_percent() < (get_skill(ch,gsn_fists_acid) / 6))
          {
           send_to_char("Your fists flare yellow-green for a moment!\n\r",ch);
           act("$n's fists flare yellow-green for a moment!",ch,0,victim,TO_ROOM);
           dt = gsn_fists_acid;
           dam_type = DAM_ACID;
           dam = dam + (dam*2/10);
          }
        }
        if (is_affected(ch,gsn_fists_liquid))
        {
          int fisthit;
          if (number_percent() < (get_skill(ch,gsn_fists_liquid) / 5 ))
          {
            fisthit = number_range(1,4);
            switch( fisthit )
            {
             case 1:
                send_to_char("Water surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by water!",ch,0,victim,TO_ROOM);
                dam_type = DAM_DROWNING;
                dam = dam + (dam*3/11);//7% damage boost to compensate for no flag/fists of fury
                break;
             case 2:
                send_to_char("Hot magma surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by liquid fire!",ch,0,victim,TO_ROOM);
                dam_type = DAM_FIRE;
                dam = dam + (dam*3/11);
                break;
             case 3:
                send_to_char("Cold air surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by liquid air!",ch,0,victim,TO_ROOM);
                dam_type = DAM_COLD;
                dam = dam + (dam*3/11);
                break;
             case 4:
                send_to_char("Searing acid surrounds your fists!\n\r",ch);
                act("$n's fists are surrounded by acid!",ch,0,victim,TO_ROOM);
                dam_type = DAM_ACID;
                dam = dam + (dam*3/11);
                break;
            }
            dt = gsn_fists_liquid;
          }
        }
        if (is_affected(ch,gsn_fists_claw) && dt != gsn_backstab && dt != gsn_circle)
        {
          int percent;

          if ((percent = number_percent()) <= (get_skill(ch,gsn_fists_claw)/8.5))
          {
           send_to_char("You lash out with your claws!\n\r",ch);
           act("$n strikes $N with the claws on his wrists!",ch,0,victim,TO_ROOM);
           dt = gsn_fists_claw;
           dam_type = DAM_PIERCE;
           dam = 2 * dam + (dam * 2.5 * percent / 100);
          }
        }
        if (is_affected(ch,gsn_fists_darkness) && dt != gsn_backstab && dt != gsn_circle)
        {
          if (get_eq_char(ch,WEAR_SECONDARY) == NULL)//Single-wield only (voodan skill)
          {
          	if (number_percent() < (get_skill(ch,gsn_fists_darkness) / 6))
          	{
           		send_to_char("Shadows surround your fists!\n\r",ch);
           		act("$n's fists are covered by shadows!",ch,0,victim,TO_ROOM);
           		dt = gsn_fists_darkness;
           		dam_type = DAM_NEGATIVE;
           		dam = dam + (dam*3/10);//Slight increase in damage (to help voodan)
           		//Vampiric Flag
           		wdam = number_range( 1, ch->level / 5 + 2);
                act("$n's life is sucked away!",victim,0,NULL,TO_ROOM);
                act("You feel your life being sucked away!",victim,0,NULL,TO_CHAR);
                victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
                ch->hit = UMIN(ch->max_hit,ch->hit + wdam);
	            victim->mana = (victim->mana <= (wdam / 4) ? 1 : victim->mana - (wdam / 3));
	            ch->mana = UMIN(ch->max_mana,ch->mana + (wdam / 2));
                update_pos(victim);
          	}
          }	
        }

     if (check_open_palming(ch) && is_affected(ch,gsn_martial_arts))
     {
        int percent = number_percent();
        int fun = number_range(1,5);
        act("You hit $N with stunning force!",ch,NULL,victim,TO_CHAR);
        act("$n hits you with stunning force!",ch,NULL,victim,TO_VICT);
        act("$n hits $N with stunning force!",ch,NULL,victim,TO_NOTVICT);
        switch(fun)
        {
        case 1: dam_type = DAM_PIERCE; break;
        case 2: dam_type = DAM_BASH; break;
        case 3: dam_type = DAM_SLASH; break;
        case 4: dam_type = DAM_HARM; break;
        default: dam_type = DAM_BASH; break;
        }
        if (ch->level <= 59)
         dam = 2 * dam + (dam * 2 * percent / 120);
        else if(ch->level <= 100)
         dam = 2 * dam + (dam * 2 * percent / 100);
        else
         dam = 2 * dam + (dam * 2 * percent / 80);
        check_improve(ch,gsn_palm,TRUE,2);
     }

     /* Renshai Modi's Anger */
     if(is_affected(ch, gsn_modis_anger))
     {
      int chance, skill;

      chance = get_skill(ch,gsn_modis_anger)/10;
      skill = get_skill(ch,gsn_modis_anger);

      if( number_percent() < chance )
      {
       if( ch->mana > 20 )
       {
        dam += GET_MOD_DAMROLL(ch) * UMIN(100,skill)/100;
        dt = gsn_modis_anger;
        ch->mana -= 20;
        check_improve(ch,gsn_modis_anger,TRUE,6);
        act("$n strikes with the anger of Modi!", ch, NULL, NULL, TO_ROOM);
       }
      }
      else
       check_improve(ch,gsn_concentration,FALSE,1);
     }

	if (dam <= 0)
               dam = 1;

	result = damage (ch, victim, dam, dt, dam_type, show, 0);   

  /* STORM Critical Strike*/
/*     if(is_affected(ch, gsn_crit_strike))
     {
      int chance, skill;

      chance = get_skill(ch,gsn_crit_strike)/10;
      skill = get_skill(ch,gsn_crit_strike);

      if( number_percent() < chance )
      {
       if( ch->mana > 20 )
       {
        dam += GET_MOD_DAMROLL(ch) * UMIN(100,skill)/100;
        dt = gsn_crit_strike;
        ch->mana -= 20;
        check_improve(ch,gsn_crit_strike,TRUE,6);
        act("$n lands a critacal strike!", ch, NULL, NULL, TO_ROOM);
       }
      }
      else
       check_improve(ch,gsn_concentration,FALSE,1);
     }

	if (dam <= 0)
               dam = 1;

	result = damage (ch, victim, dam, dt, dam_type, show, 0);*/

    // Toughness
    if(result)
     improve_toughness(victim);

    // Weapon Flags
    if ( !IS_NPC(ch) && wield != NULL ) {
     if ( number_range( 1,5 ) == 2 ) {
      if ( ch->fighting == victim && result ) {
       if ( IS_WEAPON_STAT(wield, WEAPON_FLAMING) ) {
          wdam = number_range( 1, wield->level / 15 + 1);
          act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
          act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
	  victim->hit =  (victim->hit <= wdam ? 1 : victim->hit - wdam);
          fire_effect( (void *) victim,wield->level*3/5,dam,TARGET_CHAR);
          update_pos( victim );
        }
        if ( IS_WEAPON_STAT(wield, WEAPON_FROST) ) {
          wdam = number_range( 1, wield->level / 12 + 2);
          act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
          act("The cold touch of $p surrounds you with ice.",victim,wield,NULL,TO_CHAR);
          victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
          cold_effect(victim,wield->level,dam,TARGET_CHAR);
          update_pos(victim);
        }
        if ( IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC) ) {
          wdam = number_range( 1, wield->level / 3 + 2);
          act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
          act("The vile touch of $p draws your life.",victim,wield,NULL,TO_CHAR);
          victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
          ch->hit = UMIN(ch->max_hit,ch->hit + wdam);
	      victim->mana = (victim->mana <= (wdam / 4) ? 1 : victim->mana - (wdam / 3));
	      ch->mana = UMIN(ch->max_mana,ch->mana + (wdam / 2));
          update_pos(victim);
        } //bree change osmosis
        if ( IS_WEAPON_STAT(wield, WEAPON_OSMOSIS) ) {
          wdam = number_range( 1, wield->level / 5 + 2);
           act("$p draws power from $n.",victim,wield,NULL,TO_ROOM);
          act("The vile touch of $p draws your power.",victim,wield,NULL,TO_CHAR);
          victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
          ch->hit = UMIN(ch->max_hit,ch->hit + wdam);
          victim->mana = (victim->mana <= wdam ? 1 : victim->mana - wdam);
          ch->mana = UMIN(ch->max_mana,ch->mana + (wdam * 2 / 3));
          update_pos(victim);
        }    
 if ( IS_WEAPON_STAT(wield, WEAPON_ENERGYDRAIN) ) {
          wdam = number_range( 1, wield->level / 5 + 3);
           act("$p draws energy from $n.",victim,wield,NULL,TO_ROOM);
          act("The vile touch of $p draws your energy!.",victim,wield,NULL,TO_CHAR);
          victim->move = (victim->move <= wdam ? 1 : victim->move - wdam);
          ch->move = UMIN(ch->max_move,ch->move + wdam);
          victim->move = (victim->move <= wdam ? 1 : victim->move - wdam);
          ch->move = UMIN(ch->max_move,ch->move + (wdam * 2 / 3));
          update_pos(victim);
        }
        if ( IS_WEAPON_STAT(wield, WEAPON_SHOCKING) ) {
          wdam = number_range( 1, wield->level / 15 + 2);
          act("$p shocks $n.",victim,wield,NULL,TO_ROOM);
          act("The electrified touch of $p shocks you.",victim,wield,NULL,TO_CHAR);
          victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
          shock_effect(victim, wield->level + 10,dam,TARGET_CHAR);
          update_pos(victim);
        }
        if ( IS_WEAPON_STAT(wield, WEAPON_POISON) ) {
         wdam = number_range( 1, wield->level / 15 + 2);
         act("$n turns green as $p poisons $m.",victim,wield,NULL,TO_ROOM);
         act("You feel sick as $p's poisoned edge pierces you.",victim,wield,NULL,TO_CHAR);
         poison_effect(victim, wield->level, dam, TARGET_CHAR);
         victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
         update_pos(victim);
        }
        if ( IS_WEAPON_STAT(wield, WEAPON_ACIDIC) ) {
         wdam = number_range( 1, wield->level / 15 + 2);
         act("$n screams as $p sears $m.",victim,wield,NULL,TO_ROOM);
         act("You scream $p's acidic edge sears your flesh.",victim,wield,NULL,TO_CHAR);
         acid_effect(victim, wield->level, dam, TARGET_CHAR);
         victim->hit = (victim->hit <= wdam ? 1 : victim->hit - wdam);
         update_pos(victim);
        }
       }
      }
     }

 if(result)
 {
     if (ch->fighting == victim) {
       if (IS_SHIELDED (victim, SHD_ICE)) {
         if (!IS_SHIELDED (ch, SHD_ICE)) {
           dam = number_range (20, 25);
           dt = skill_lookup ("iceshield");
           damage (victim, ch, dam, dt, DAM_COLD, TRUE,
           DF_NOREDUCE | DF_NODODGE | DF_NOPARRY | DF_NOBLOCK);
         }
       }
       if (IS_SHIELDED (victim, SHD_FIRE)) {
         if (!IS_SHIELDED (ch, SHD_FIRE)) {
           dam = number_range (20, 25);
           dt = skill_lookup ("fireshield");
           damage (victim, ch, dam, dt, DAM_FIRE, TRUE,
           DF_NOREDUCE | DF_NODODGE | DF_NOPARRY | DF_NOBLOCK);
         }
       }
       if ( IS_SHIELDED (victim, SHD_SHOCK) )
       {
          if ( !IS_SHIELDED (ch, SHD_SHOCK) )
          {
             dam = number_range (25, 30);
             dt = skill_lookup ("shockshield");
             damage (victim, ch, dam, dt, DAM_LIGHTNING, TRUE,
             DF_NOREDUCE | DF_NODODGE | DF_NOPARRY | DF_NOBLOCK);
             
             if ( ( get_curr_stat( victim, STAT_WIS) + victim->saving_throw ) < ( number_percent() + get_curr_stat( victim, STAT_INT) )
                && has_skill(ch, skill_lookup( "shockshield" ) )  )
             {
                if ( con_app[get_curr_stat( ch, STAT_CON)].shock < number_percent() )
                {
                  send_to_char("You got zapped by their shockshield!\n\r",ch);
                  act("$n's body go stiff with paralysis from the shockshield!",ch,NULL,NULL,TO_ROOM);
                  WAIT_STATE(ch, .5 * PULSE_VIOLENCE);
                }
             }
          }
       }
       if ( IS_SHIELDED (victim, SHD_DEATH ) )
       {
          if ( !IS_SHIELDED (ch, SHD_DEATH) )
          {
             int dmax, dmin;
             dmin = 5 + ((victim->level * 30) / 101);
             dmax = 10 + ((victim->level * 40) / 101);
             dam = number_range (dmin, dmax);
             dt = skill_lookup ("deathshield");
             damage (victim, ch, dam, dt, DAM_NEGATIVE, TRUE,
                    DF_NODODGE | DF_NOPARRY | DF_NOBLOCK);
             victim->hit += dam;
          }
       }
     }
  }
	tail_chain ();
        if (!is_safe(ch,victim))
        {
            improve_stance(ch);
        }
	return;
}

int add_bonuses( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    float mult = 1;
    float reduct = 1;
    int diceroll;

    if ( !IS_AWAKE(victim) )
        mult += 1;
    else if (victim->position < POS_FIGHTING)
        dam = dam * 4 / 3;

    if ( !IS_NPC(ch) )
    {
        if (ch->class == CLASS_HIGHLANDER && dt == gsn_blademastery)
	{
             if(ch->pcdata->power[POWER_LEVEL] != 0)
               dam += edam_bonus( ch, dam )*(ch->pcdata->power[POWER_LEVEL]+1);
             else
               dam += edam_bonus( ch, dam );
        }

        if (ch->class == CLASS_HIGHLANDER && dt == gsn_decapitate) {
               int hroll;

               if(ch->pcdata->power[POWER_LEVEL] != 0)
                       dam += edam_bonus( ch, dam ) * (ch->pcdata->power[POWER_LEVEL]+2);
               else
                dam += edam_bonus( ch, dam );

		diceroll = number_percent ();
                if (diceroll <= (get_skill (ch, gsn_blademastery)/8))
		{
                  hroll = number_range(1,ch->pcdata->power[POWER_LEVEL]);

                  if(hroll > 2) {
                   hroll *= 3;
                   hroll /= 4;
                  }

                  dam *= hroll;

                  dam_type = DAM_OTHER;
               }
           
        }

      }

    if ( IS_NPC(ch) && ch->level > 100 )
    {
      mult += ( ch->level / 100 );
      reduct += ( ch->level / 90 );
    }

    if ( IS_NPC(victim) && victim->level > 100 )
    {
      mult += ( victim->level / 90 );
      reduct += ( victim->level / 100 );
    }

    if (get_skill (ch, gsn_enhanced_damage) > 0) {
      diceroll = number_percent ();
     if (diceroll <= get_skill (ch, gsn_enhanced_damage)) {
       check_improve (ch, gsn_enhanced_damage, TRUE, 6);
       dam += 1.3*edam_bonus( ch, dam );
     }
    }
    else
    {
        dam *= 7;
        dam /= 10;
    }

    dam *= mult;
    dam /= reduct;

    return dam;
}

/*
 * Mock hit one guy once.
 */
void
one_hit_mock (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll;
	int sn, skill;
	int dam_type;
	bool result;

	sn = -1;


	/* just in case */
	if (ch == NULL || victim == NULL)
		return;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
		return;

	/*
	 * Figure out the type of damage message.
	 * if secondary == true, use the second weapon.
	 */
	if (!secondary)
		wield = get_eq_char (ch, WEAR_WIELD);
	else
		wield = get_eq_char (ch, WEAR_SECONDARY);
	if (dt == TYPE_UNDEFINED)
	{
		dt = TYPE_HIT;
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
			dt += wield->value[3];
		else
			dt += ch->dam_type;
	}

	if (dt < TYPE_HIT)
		if (wield != NULL)
			dam_type = attack_table[wield->value[3]].damage;
		else
			dam_type = attack_table[ch->dam_type].damage;
	else
		dam_type = attack_table[dt - TYPE_HIT].damage;

	if (dam_type == -1)
		dam_type = DAM_BASH;

	/* get the weapon skill */

	sn = get_weapon_sn (ch);
	skill = 20 + get_weapon_skill (ch, sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC (ch))
	{
		thac0_00 = 20;
		thac0_32 = -4;			/* as good as a thief */
		if (IS_SET (ch->act, ACT_VAMPIRE))
			thac0_32 = -30;
		else if (IS_SET (ch->act, ACT_DRUID))
			thac0_32 = 0;
		else if (IS_SET (ch->act, ACT_RANGER))
			thac0_32 = -4;
		else if (IS_SET (ch->act, ACT_WARRIOR))
			thac0_32 = -10;
		else if (IS_SET (ch->act, ACT_THIEF))
			thac0_32 = -4;
		else if (IS_SET (ch->act, ACT_CLERIC))
			thac0_32 = 2;
		else if (IS_SET (ch->act, ACT_MAGE))
			thac0_32 = 6;
	}
	else
	{
		thac0_00 = class_table[ch->class].thac0_00;
		thac0_32 = class_table[ch->class].thac0_32;
	}
	thac0 = interpolate (ch->level, thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0 / 2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 -= GET_HITROLL (ch) * skill / 100;
	thac0 += 5 * (100 - skill) / 100;

	if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

        if (dt == gsn_bladedance)
                thac0  -= 10 * (100 - get_skill (ch, gsn_bladedance));
	if (dt == gsn_circle)
		thac0 -= 10 * (100 - get_skill (ch, gsn_circle));

	if (dt == gsn_ambush)
		thac0 -= 10 * (100 - get_skill (ch, gsn_ambush));

	switch (dam_type)
	{
	case (DAM_PIERCE):
		victim_ac = GET_AC (victim, AC_PIERCE) / 9;
		break;
	case (DAM_BASH):
		victim_ac = GET_AC (victim, AC_BASH) / 9;
		break;
	case (DAM_SLASH):
		victim_ac = GET_AC (victim, AC_SLASH) / 9;
		break;
	default:
		victim_ac = GET_AC (victim, AC_EXOTIC) / 9;
		break;
	};
/*
	if (victim_ac < -15)
		victim_ac = (victim_ac + 15) / 5 - 15;
*/
	if (!can_see (ch, victim))
		victim_ac -= 4;

	if (victim->position < POS_FIGHTING)
		victim_ac += 4;

	if (victim->position < POS_RESTING)
		victim_ac += 6;

	/*
	 * The moment of excitement!
	 */
	while ((diceroll = number_bits (5)) >= 20)
		;

	if (diceroll == 0
		|| (diceroll != 19 && diceroll < thac0 - victim_ac))
	{
		/* Miss. */
		damage_mock (ch, victim, 0, dt, dam_type, TRUE);
		tail_chain ();
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */
	if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
		if (!ch->pIndexData->new_format)
		{
			dam = number_range (ch->level / 2, ch->level * 3 / 2);
			if (wield != NULL)
				dam += dam / 2;
		}
		else
			dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

	else
	{
		/*if (sn != -1)
			check_improve (ch, sn, TRUE, 5); No abusing to spam up weapon skills!*/
		if (wield != NULL)
		{
			if (wield->clan)
			{
				float adlev, inclev;
				int cntr;

				adlev = 8;
				inclev = .01;
				for (cntr = 0; cntr <= ch->level; cntr++)
				{
					adlev += .57;
					adlev += inclev;
					inclev += .005;
				}
				cntr = (int) adlev;
				dam = dice (cntr / 3, 3) * skill / 100;
			}
			else
			{
				if (wield->pIndexData->new_format)
					dam = dice (wield->value[1], wield->value[2]) * skill / 100;
				else
					dam = number_range (wield->value[1] * skill / 100,
										wield->value[2] * skill / 100);
			}

			if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
				dam = dam * 11 / 10;

			/* sharpness! */
			if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
			{
				int percent;

				if ((percent = number_percent ()) <= (skill / 8))
					dam = 2 * dam + (dam * 2 * percent / 100);
			}
		}
		else
			dam = number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
	}

	/*
	 * Bonuses.
	 */
	if (get_skill (ch, gsn_enhanced_damage) > 0)
	{
		diceroll = number_percent ();
		if (diceroll <= get_skill (ch, gsn_enhanced_damage))
		{
			//check_improve (ch, gsn_enhanced_damage, TRUE, 6);
//			dam += 2 * (dam * diceroll / 300);
			dam += edam_bonus( ch, dam );
		}
	}

	if (!IS_AWAKE (victim))
		dam *= 2;
	else if (victim->position < POS_FIGHTING)
		dam = dam * 3 / 2;

	if (dt == gsn_backstab && wield != NULL)
	{
		if (wield->value[0] != 2)
			dam *= 2 + (ch->level / 2);
		else
			dam *= 2 + (ch->level);
	}
	if (dt == gsn_circle && wield != NULL)
	{
		if (wield->value[0] != 2)
			dam *= 2 + (ch->level / 3);
		else
			dam *= 2 + (ch->level / 2);
	}
	dam += GET_MOD_DAMROLL(ch) * UMIN (100, skill) / 100;

	if (dam <= 0)
		dam = 1;

	result = damage_mock (ch, victim, dam, dt, dam_type, TRUE);

	tail_chain ();
	return;
}

/*
 * Inflict damage from a hit.
 */
bool damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type,
		bool show, int flags)
{

	OBJ_DATA *corpse;
        bool immune = FALSE;
        int victim_ac=0;
        //CHAR_DATA *gch;

	if (victim->position == POS_DEAD)
		return FALSE;

        if(skill_table[dt].spell_fun != NULL && skill_table[dt].target == TAR_CHAR_OFFENSIVE)
        {
         if(check_sorcery(ch,dt))
         {
//           if(!IS_IMMORTAL(ch))
           	send_to_char("You feel a surge of magic!\n\r",ch);
           	
           dam = sorcery_dam(dam,2, ch);
         }
        }

	/* damage reduction */
	if (!(flags & DF_NOREDUCE))
	{
		if (dam > 35)
                        dam = (dam - 35) * 7 / 10 + 35;
                if (dam > 85)
                        dam = (dam - 85) * 7 / 10 + 85;
		if (dam > 500)
						dam = (dam - 500) * 7 / 10 + 500;
		if (dam > 1000)
						dam = (dam - 1000) * 7 / 10 + 1000;
		if (dam > 2000)
						dam = (dam - 2000) * 7 / 10 + 2000;
		if (dam > 3000)
						dam = (dam - 3000) * 7 / 10 + 3000;
	}

	/*
	 * Stop up any residual loopholes.
	 */
        if (dam > 6000 && dt >= TYPE_HIT && ch->level < 106)
	{
                bug ("Damage: %d: more than 6000 points!", dam);
                dam = 6000;
		if (!IS_IMMORTAL (ch) )
		{
			OBJ_DATA *obj;
			obj = get_eq_char (ch, WEAR_WIELD);
			send_to_char ("{cYou {z{Breally{x{c shouldn't cheat.{x\n\r", ch);
			if (obj != NULL)
				extract_obj (obj);
		}

	}


	if (victim != ch)
	{
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if (is_safe (ch, victim))
			return FALSE;

		if (victim->position > POS_STUNNED)
		{
			if (victim->fighting == NULL)
			{
				set_fighting (victim, ch);
				victim->position = POS_FIGHTING;
				if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_KILL))
					mp_percent_trigger (victim, ch, NULL, NULL, TRIG_KILL);
			}
			if (victim->timer <= 4)
				victim->position = POS_FIGHTING;
		}

		if (victim->position > POS_STUNNED)
		{
			if (ch->fighting == NULL)
				set_fighting (ch, victim);

		}

	}

	/*
	 * More charm stuff.
	 */
	if (victim->master == ch) // Why stop if it is yourself?  || ch == victim)
		stop_follower (victim);

	/*
	 * Inviso attacks ... not.
	 */
	if (IS_SHIELDED (ch, SHD_INVISIBLE))
	{
	    if (!can_see(victim,ch)) {
            dam *= 1.25;
            send_to_char("You use your concealment to get a surprise attack!\n\r",ch);
            }

		affect_strip (ch, gsn_invis);
		affect_strip (ch, gsn_mass_invis);
		REMOVE_BIT (ch->shielded_by, SHD_INVISIBLE);
		act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	}

        if(is_affected(ch,gsn_hide)) {
	    if (!can_see(victim,ch)) {
            dam *= 1.25;
            send_to_char("You use your concealment to get a surprise attack!\n\r",ch);
            }
            affect_strip(ch,gsn_hide);
	    act( "$n leaps from $s concealment.", ch, NULL, NULL, TO_ROOM );
	}

        if(IS_AFFECTED(ch,AFF_HIDE)) {
	    if (!can_see(victim,ch)) {
            dam *= 1.25;
            send_to_char("You use your concealment to get a surprise attack!\n\r",ch);
            }
	    REMOVE_BIT( ch->affected_by, AFF_HIDE );
	    act( "$n leaps from $s concealment.", ch, NULL, NULL, TO_ROOM );
	}

        if(is_affected(ch,gsn_forest_blend)) {
	    if (!can_see(victim,ch)) {
            dam *= 1.5;
            send_to_char("You use your concealment to get a surprise attack!\n\r",ch);
            }
            affect_strip(ch,gsn_forest_blend);
	    act( "$n leaps from $s concealment.", ch, NULL, NULL, TO_ROOM );
        }

        if(is_affected(ch,gsn_earthmeld)) {
	    if (!can_see(victim,ch)) {
            dam *= 1.5;
            send_to_char("You use your concealment to get a surprise attack!\n\r",ch);
            }
            affect_strip(ch,gsn_earthmeld);
            act( "$n pops up from the ground!", ch, NULL, NULL, TO_ROOM );
        }

	/*
	 * Damage modifiers.
	 */

	if (dam > 1 && !IS_NPC (victim)
		&& victim->pcdata->condition[COND_DRUNK] > 10)
		dam = 9 * dam / 10;

	if (dam > 1 && IS_SHIELDED (victim, SHD_SANCTUARY))
                dam -= dam/3;

        // Toughness
        if (dam > 1 && is_warrior_class(victim->class) && number_percent() < victim->toughness * 0.35)
           dam -= dam * (float)((victim->toughness/5)/34)/10;


	if (dam > 1 && ((IS_SHIELDED (victim, SHD_PROTECT_EVIL) && IS_EVIL (ch))
			   || (IS_SHIELDED (victim, SHD_PROTECT_GOOD) && IS_GOOD (ch))))
                dam -= dam / 5;

        if (dam > 1 && (IS_SHIELDED (victim, SHD_STONE)))
                dam -= dam / 10;

        if (dam > 1 && (IS_SHIELDED (victim, SHD_STEEL)))
                dam -= dam / 10;

     /* Check for will of iron */ 
    if( is_affected(victim,gsn_will_of_iron) && number_percent() < 40) 
		{
			if (is_affected(victim,gsn_martial_arts))
			{
        		dam -= dam/5; 
			}
			else
			{
				dam -= dam/3;	
			}
		}
        /*
         * Parry, Shield Block, Dodge, Phase.
         * Block, Roll, Acrobatics
	 */
	if (dt >= TYPE_HIT && ch != victim)
	{
            if (!(flags & DF_NOPARRY) &&
                (check_parry (ch, victim)))
              return FALSE;
	    if((victim->class == CLASS_MONK) && (is_affected(victim,gsn_martial_arts)))
        {
            if (check_deflect(ch,victim,dt,dam))
		    return FALSE;
	    }              
            if (!(flags & DF_NOBLOCK) &&
                (check_shield_block (ch, victim)))
                return FALSE;
            if (check_barrier (ch, victim))
                return FALSE;
            if (!(flags & DF_NODODGE) &&
                 (check_dodge (ch, victim)))
                return FALSE;
            if (check_phase (ch, victim))
                return FALSE;
            if (!(flags & DF_NOBLOCK) && check_shield_wall(ch, victim))
                return FALSE;
            if (!(flags & DF_NOPARRY) && check_roll(ch, victim))
                return FALSE;
            if (!(flags & DF_NODODGE) && check_acrobatics(ch, victim))
                return FALSE;
	}
	
      switch (check_immune(victim, dam_type))
      {
       case (IS_IMMUNE):
             dam = 0;
             break;
       case (IS_RESISTANT):
             dam -= dam/3.5 + number_range(dam/8,dam/12);
             break;
       case (IS_VULNERABLE):
             dam += dam/3.5 + number_range(dam/8,dam/12);
             break;
      }

    // AC
    if(dam_type == DAM_BASH)
     victim_ac = GET_AC(victim,AC_BASH);
    else if(dam_type == DAM_SLASH)
     victim_ac = GET_AC(victim,AC_SLASH);
    else if(dam_type == DAM_PIERCE)
     victim_ac = GET_AC(victim,AC_PIERCE);
    else
     victim_ac = GET_AC(victim,AC_EXOTIC);

/*    if(victim->class == CLASS_HIGHLANDER)
	{
         dam -= dam * (-victim_ac * .00025);
	}
    else
	{
         dam -= dam * (-victim_ac * .0004375);
	} 
*/
// Sorn - New Damage Reduction 
    if(victim->class == CLASS_HIGHLANDER || victim->class == CLASS_MONK) {
	if(victim_ac < -1729) {
		dam -= dam * UMIN(.7,-victim_ac*.0002); }
	else {
		dam -= dam * ((-victim_ac-((-victim_ac*-victim_ac*3.0)/12100.0))*.00035); }
	}
    else {
	if(victim_ac < -1900) {
		dam -= dam * UMIN(.7,-victim_ac*.0003); }
	else {
		dam -= dam * ((-victim_ac-((-victim_ac*-victim_ac*2.0)/12100.0))*.0004375); }
	}

     if(IS_SET(ch->stance_aff,STANCE_AFF_AGGRESSIVE) &&
     number_percent() < ch->stance[ch->stance[0]] * 0.4)
     {
      if(IS_SET(ch->stance_aff,STANCE_AFF_DOUBLE_POWER))
       dam += dam * 0.12;
      else
       dam += dam * 0.10;
     }

     if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_DEFENSE) &&
     number_percent() < victim->stance[victim->stance[0]] * 0.5)
     {
       dam -= dam * 0.15;
     }

	if ( dam < 0 )
	    dam = 0;

        /* Do Check for Targetting Here */
/*        if(!IS_NPC(ch) && !IS_NPC(victim))
        {
         for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
         {
          if(IS_NPC(gch) || gch == ch)
           continue;

          if (is_same_group (gch, ch) && !str_cmp(gch->pcdata->target,victim->name)
          && (victim->hit - dam) < 0  && !IN_ARENA(ch))
          {
           // No hit...Wait for the Assassination
           return FALSE;
          }

	 }
        } */

         /* change killer */

        if(dt == gsn_whip && show)
	{
          dam_message_new(ch, victim, dam, dt, immune, DMF_WHIP);
	  rounddam += dam;
	}
        else
        {
	if (show)
            dam_message (ch, victim, dam, dt, immune);
	    rounddam+= dam;
        }

        if(!IS_NPC(ch))
        ch->pcdata->round_dam += dam;
        if(IS_NPC(ch))
        ch->pIndexData->round_dam += dam;

 	if (dam == 0)
		return FALSE;

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	victim->hit -= dam;
	if (!IS_NPC (victim)
		&& victim->level >= LEVEL_IMMORTAL
		&& victim->hit < 1
		&& !IS_SET(victim->in_room->room_flags, ROOM_ARENA))
		victim->hit = 1;
	update_pos (victim);

        if(victim->hit < 1 && dt == gsn_poison && IS_NPC(victim))
         victim->position = POS_MORTAL;

	if (dt == gsn_feed)
	{
		ch->hit = UMIN (ch->hit + ((dam / 3) * 2), ch->max_hit);
		update_pos (ch);
	}

#if 0
	/* If POS_MORTAL, POS_INCAP, or POS_DEAD and in arena, do arena stuff */
	if ((arena == FIGHT_BUSY) && (victim->position < POS_STUNNED))
	{
		check_arena (ch, victim);
		return TRUE;
	}
#endif

   if(!IS_NPC(victim) && !IS_NPC(ch))
   {
    if(IS_SET(ch->plyr,PLAYER_SUBDUE))
     switch( victim->position )
     {
      case POS_MORTAL:
        act( "You May DECAPITATE $N now!",ch,NULL, victim, TO_CHAR);
         act ("{c$n is mortally wounded, and will die soon, if not aided.{x", victim, NULL, NULL, TO_ROOM);
         send_to_char ("{cYou are mortally wounded, and will die soon, if not aided.{x\n\r", victim);
		break;

	case POS_INCAP:
		act ("{c$n is incapacitated and will slowly die, if not aided.{x",
			 victim, NULL, NULL, TO_ROOM);
		send_to_char (
						 "{cYou are incapacitated and will slowly {z{Rdie{x{c, if not aided.{x\n\r",
						 victim);
		break;

	case POS_STUNNED:
		act ("{c$n is stunned, but will probably recover.{x",
			 victim, NULL, NULL, TO_ROOM);
		send_to_char ("{cYou are stunned, but will probably recover.{x\n\r",
					  victim);
		break;

      default:
        if(victim->hit <= -11)
        {
        send_to_char("You are mortally wounded, and will die soon, if not aided.\n\r",victim );
        stop_fighting( ch, TRUE );
        victim->hit = -10;
        victim->position = POS_MORTAL;
        }
        break;
    }
   }
   else /* This now handles cases where Ch or Victim is NPC */
   {
	switch (victim->position)
	{
	case POS_MORTAL:
         act ("{c$n is mortally wounded, and will die soon, if not aided.{x", victim, NULL, NULL, TO_ROOM);
         send_to_char ("{cYou are mortally wounded, and will die soon, if not aided.{x\n\r", victim);
		break;

	case POS_INCAP:
		act ("{c$n is incapacitated and will slowly die, if not aided.{x",
			 victim, NULL, NULL, TO_ROOM);
		send_to_char (
						 "{cYou are incapacitated and will slowly {z{Rdie{x{c, if not aided.{x\n\r",
						 victim);
		break;

	case POS_STUNNED:
		act ("{c$n is stunned, but will probably recover.{x",
			 victim, NULL, NULL, TO_ROOM);
		send_to_char ("{cYou are stunned, but will probably recover.{x\n\r",
					  victim);
		break;

	case POS_DEAD:
/*        if ((IS_NPC(victim)) && ( victim->die_descr[0] != '\0'))
   {
   act( "{c$n $T{x", victim, 0, victim->die_descr, TO_ROOM );
   }
   else
   { */
		act ("{c$n is {CDEAD!!{x", victim, 0, 0, TO_ROOM);
/*        } */
		send_to_char ("{cYou have been {RKILLED!!{x\n\r\n\r", victim);
		break;

	default:
		if (dam > victim->max_hit / 10)
			send_to_char ("{cThat really did {RHURT!{x\n\r", victim);
		if (victim->hit < victim->max_hit / 4)
			send_to_char ("{cYou sure are {z{RBLEEDING!{x\n\r", victim);
		break;
	}
     }
	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE (victim))
		stop_fighting (victim, FALSE);

	/*
	 * Payoff for killing things.
	 */
	if (victim->position == POS_DEAD)
	{
		group_gain (ch, victim);

		if (!IS_NPC (victim))
		{
			sprintf (log_buf, "%s killed by %s at %d",
					 victim->name,
					 (IS_NPC (ch) ? ch->short_descr : ch->name),
					 ch->in_room->vnum);
			log_string (log_buf);
		}

                if(dt == gsn_decapitate)
                {
                 // Put severed part here.
                }

		/*
		 * Death Trigger
		 */
		if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_DEATH))
		{
			victim->position = POS_STANDING;
			mp_percent_trigger (victim, ch, NULL, NULL, TRIG_DEATH);
		}

                if(!IS_NPC(ch) && !IS_NPC(victim) && is_pkill(ch) && is_pkill(victim) && victim->pcdata->bounty > 0 && !str_cmp(ch->pcdata->target,victim->name) && !(IN_ARENA(ch) || IN_ARENA(victim)))
		{
			do_mutilate(ch,victim);
		}
		raw_kill (victim, ch);

		/* dump the flags */
		if (ch != victim && !IS_NPC (ch) && (!is_same_clan (ch, victim)
								   || clan_table[victim->clan].independent))
		{
			if (IS_SET (victim->act, PLR_TWIT))
				REMOVE_BIT (victim->act, PLR_TWIT);
		}

		/* RT new auto commands */

		if (!IS_NPC (ch) && IS_NPC (victim))
		{
			OBJ_DATA *coins;

			corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

			if (IS_SET (ch->act, PLR_AUTOLOOT) &&
				corpse && corpse->contains)		/* exists and not empty */
				do_get (ch, "all corpse");

			if (IS_SET (ch->act, PLR_AUTOGOLD) &&
				corpse && corpse->contains &&	/* exists and not empty */
				!IS_SET (ch->act, PLR_AUTOLOOT))
			{
				if ((coins = get_obj_list (ch, "gcash", corpse->contains))
					!= NULL)
					do_get (ch, "all.gcash corpse");
			}
			if (IS_SET (ch->act, PLR_AUTOSAC))
			{
				if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)
					return TRUE;	/* leave if corpse has treasure */
				else
					do_sacrifice (ch, "corpse");
			}
		}

		return TRUE;
	}

	if (victim == ch)
		return TRUE;

	/*
	 * Take care of link dead people.
	 */
	if (!IS_NPC (victim) && victim->desc == NULL)
	{
		if (number_range (0, victim->wait) == 0)
		{
			do_recall (victim, "");
			return TRUE;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
	{
		if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
			 && victim->hit < victim->max_hit / 5)
			|| (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
				&& victim->master->in_room != victim->in_room))
			do_flee (victim, "");
	}

	if (!IS_NPC (victim)
		&& IS_NPC (ch)
 		&& victim->hit > 0
		&& victim->hit <= victim->wimpy
		&& victim->wait < PULSE_VIOLENCE / 2)
		do_flee (victim, "");

	tail_chain ();
	return TRUE;
}

/*
 * Show damage from a mock hit.
 */
bool
damage_mock (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show)
{
	long immdam;
	bool immune;
	char buf1[256], buf2[256], buf3[256];
	const char *attack;

	if (victim->position == POS_DEAD)
		return FALSE;

	if (dam > 35)
		dam = (dam - 35) / 2 + 35;
	if (dam > 80)
		dam = (dam - 80) / 2 + 80;
	if (is_safe_mock (ch, victim))
		return FALSE;
	/*
	 * Damage modifiers.
	 */

	if (dam > 1 && !IS_NPC (victim)
		&& victim->pcdata->condition[COND_DRUNK] > 10)
		dam = 9 * dam / 10;

	if (dam > 1 && IS_SHIELDED (victim, SHD_SANCTUARY))
                dam -= dam/3;

        // Toughness
        if (dam > 1 && is_warrior_class(victim->class) && number_percent() < victim->toughness * 0.35)
           dam -= dam * (float)((victim->toughness/5)/34)/10;

	if (dam > 1 && ((IS_SHIELDED (victim, SHD_PROTECT_EVIL) && IS_EVIL (ch))
			   || (IS_SHIELDED (victim, SHD_PROTECT_GOOD) && IS_GOOD (ch))))
                dam -= dam / 6;

        if (dam > 1 && (IS_SHIELDED (victim, SHD_STONE)))
                dam -= dam / 10;

        if (dam > 1 && (IS_SHIELDED (victim, SHD_STEEL)))
                dam -= dam / 10;

	immune = FALSE;

	switch (check_immune (victim, dam_type))
	{
	case (IS_IMMUNE):
		immune = TRUE;
		dam = 0;
		break;
	case (IS_RESISTANT):
		dam -= dam / 3;
		break;
	case (IS_VULNERABLE):
		dam += dam / 2;
		break;
	}

	if (dt >= 0 && dt < MAX_SKILL)
		attack = skill_table[dt].noun_damage;
	else if (dt >= TYPE_HIT
			 && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
		attack = attack_table[dt - TYPE_HIT].noun;
	else
	{
		bug ("Dam_message: bad dt %d.", dt);
		dt = TYPE_HIT;
		attack = attack_table[0].name;
	}
	immdam = 0;
	if (ch->level == MAX_LEVEL)
	{
		immdam = dam * 63;
	}
	if (ch == victim)
	{
		sprintf (buf1, "{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {y$mself{g.{x", attack, dam);
		sprintf (buf2, "{yYour {gmock {B%s{g would have done {R%d hp{g damage to {yyourself{g.{x", attack, dam);
		act (buf1, ch, NULL, NULL, TO_ROOM);
		act (buf2, ch, NULL, NULL, TO_CHAR);
	}
	else if (ch->level < MAX_LEVEL)
	{
		sprintf (buf1, "{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {y$N{g.{x", attack, dam);
		sprintf (buf2, "{yYour {gmock {B%s{g would have done {R%d hp{g damage to {y$N{g.{x", attack, dam);
		sprintf (buf3, "{y$n's {gmock {B%s{g would have done {R%d hp{g damage to {yyou{g.{x", attack, dam);
		act (buf1, ch, NULL, victim, TO_NOTVICT);
		act (buf2, ch, NULL, victim, TO_CHAR);
		act (buf3, ch, NULL, victim, TO_VICT);
	}
	else
	{
		sprintf (buf1, "{y$n's {gmock {B%s{g would have done {R%lu hp{g damage to {y$N{g.{x", attack, immdam);
		sprintf (buf2, "{yYour {gmock {B%s{g would have done {R%lu hp{g damage to {y$N{g.{x", attack, immdam);
		sprintf (buf3, "{y$n's {gmock {B%s{g would have done {R%lu hp{g damage to {yyou{g.{x", attack, immdam);
		act (buf1, ch, NULL, victim, TO_NOTVICT);
		act (buf2, ch, NULL, victim, TO_CHAR);
		act (buf3, ch, NULL, victim, TO_VICT);
	}

	tail_chain ();
	return TRUE;
}

/*
 * Inflict damage from a hit.
 */
bool damage_old (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type, bool show)
{
 return damage(ch,victim,dam,dt,dam_type,show,0);
 return TRUE;
}

bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (victim->in_room == NULL || ch->in_room == NULL)
		return TRUE;

        if (IS_NPC(ch) && victim->invis_level >= LEVEL_HERO)
                return TRUE;

        /* LinkDead */
        if (!IS_NPC(victim) && victim->desc == NULL && !IS_NPC(ch)
          && victim->fighting != ch && ch->fighting != victim)
        {
         act("$N is protected from $n by the Aesir.",ch,0,victim,TO_NOTVICT);
         act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
         return TRUE;
        }

	if (victim->fighting == ch || victim == ch)
                return FALSE;

        if(IS_SET(victim->plyr,PLAYER_GHOST) && (!IS_NPC(ch)))
        {
          act("$N is a ghost, you cannot attack them.",ch,NULL,victim,TO_CHAR);
          act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
          act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
          return TRUE;
        }
        else if(IS_SET(ch->plyr,PLAYER_GHOST))
        {
          if (!IS_NPC(victim))
          {
           send_to_char("You're a ghost! you can't attack anyone!\n\r",ch);
           act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
           act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
           return TRUE;
          }

          if (IS_AFFECTED(victim,AFF_CHARM) && victim->master != NULL && !IS_NPC(victim->master))
          {
           send_to_char("You're a ghost! you can't attack anyone!\n\r",ch);
           act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
           act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
           return TRUE;
          }
        }

        /* Clan stuff */
        if (IS_SET(victim->plyr,PLAYER_ANCIENT_ENEMY))
        {
         if (IS_NPC(ch) && ch->spec_fun == spec_lookup("spec_cloaked_enforcer"))
                return FALSE;
        }

        if (IS_NPC(victim) && victim->spec_fun == spec_lookup("spec_cloaked_enforcer"))
         return FALSE;

	/* killing mobiles */
	if (IS_NPC (victim))
	{

          /* safe room? */
          if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
          {
            send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }

          /* safe room? */
          if (IS_SET(victim->in_room->affected_by, ROOM_AFF_SAFE))
          {
            send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }


          if (!IS_NPC (ch) && IS_SET (victim->in_room->room_flags, ROOM_CLAN_ENT))
          {
            send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }

          if (victim->pIndexData->pShop != NULL)
          {
             send_to_char ("The shopkeeper wouldn't like that.\n\r", ch);
             return TRUE;
          }

          if (is_same_clan (ch, victim))
          {
           send_to_char ("You can't fight your own clan members.\n\r", ch);
           return TRUE;
          }


          if (!IS_NPC(ch))
          {
            /* Only in master's PK */
            if (IS_AFFECTED(victim,AFF_CHARM) && !can_pkill(ch,victim->master))
            {
             act("The Aesir protect $N from $n.",ch,0,victim,TO_NOTVICT);
             act("The Aesir protect you from $n.",ch,0,victim,TO_VICT);
             act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
                return TRUE;
            }
          }
          else
          {
           if (IS_AFFECTED(ch,AFF_CHARM) && IS_AFFECTED(victim,AFF_CHARM)
           && !can_pkill(ch->master,victim->master))
                return TRUE;
          }

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
        || IS_SET (victim->act, ACT_PRACTICE)
        || IS_SET (victim->act, ACT_IS_HEALER)
        || IS_SET (victim->act, ACT_IS_BANKER)
        || IS_SET (victim->act, ACT_IS_SATAN)
        || (victim->spec_fun == spec_lookup ("spec_questmaster"))
        || IS_SET (victim->act, ACT_IS_PRIEST))
        {
          act ("I don't think $G would approve.", ch, NULL, NULL, TO_CHAR);
           return TRUE;
        }

/*
	 if (!IS_NPC (ch))
         {
            no pets
           if (IS_SET (victim->act, ACT_PET))
           {
             act ("But $N looks so cute and cuddly...",ch, NULL, victim, TO_CHAR);
             return TRUE;
           }

           no charmed creatures unless owner
           if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master)
           {
             send_to_char ("You don't own that monster.\n\r", ch);
             return TRUE;
           }
         }  */
	}
	/* killing players */
	else
	{
		/* NPC doing the killing */
		if (IS_NPC (ch))
		{
                        /* safe room check */
			if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
			{
				send_to_char ("Not in this room.\n\r", ch);
				return TRUE;
                        }

                        /* safe room? */
                        if (IS_SET(victim->in_room->affected_by, ROOM_AFF_SAFE))
                        {
                         send_to_char ("Not in this room.\n\r", ch);
                         return TRUE;
                        }

                        /* charmed mobs and pets cannot attack players while owned */
			if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
				&& ch->master->fighting != victim)
			{
				send_to_char ("Players are your friends!\n\r", ch);
				return TRUE;
			}
		}
		/* player doing the killing */
		else
		{

            if(IS_SET (victim->act, PLR_TWIT))
                 return FALSE;

            if (victim->level < 5) 
                { 
                send_to_char("Are you trying to be a newbie killer or something?",ch);
                return TRUE;  
                }

	         if (IS_AFFECTED (ch, AFF_CHARM) )
	         {
	           send_to_char ("You have a master now, sit still and listen to them!\n\r", ch);
	           return TRUE;
	         }

                 // no attacking in arena unless named in current challenge
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) == 0))
                 {
                   send_to_char ("You're not involved in the current challenge.\n\r", ch );
                   return TRUE;
                 }

                 // teammate in arena?
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) == in_current_challenge(victim->name)))
                 {
                   send_to_char( "You can't attack your teammates.\n\r", ch );
                   return TRUE;
                 }

                 // enemy in arena?
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) !=
                  in_current_challenge(victim->name)))
                  return FALSE;


                 if (!can_pkill(ch, victim))
                 {
                  act("The Aesir protect $N from $n.",ch,0,victim,TO_NOTVICT);
                  act("The Aesir protect you from $n.",ch,0,victim,TO_VICT);
                  act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
                  return TRUE;
                 }

                 if(IS_SET(victim->shielded_by,SHD_PROTECT_AURA)
                 || IS_SET(ch->shielded_by,SHD_PROTECT_AURA))
                  return TRUE;

                 if (!is_clan (ch))
                 {
                   send_to_char ("Join a clan if you want to fight players.\n\r", ch);
                   return TRUE;
                 }

                 if (!is_pkill (ch))
                 {
                    send_to_char ("Your clan does not allow player fighting.\n\r", ch);
                    return TRUE;
                 }

                 if (!is_clan (victim))
                 {
                    send_to_char ("They aren't in a clan, leave them alone.\n\r", ch);
                    return TRUE;
                 }

                 if (!is_pkill (victim))
                 {
                    send_to_char ("They are in a no pkill clan, leave them alone.\n\r", ch);
                    return TRUE;
                 }

                 if (is_same_clan (ch, victim) && !(IS_SET (victim->in_room->room_flags, ROOM_ARENA)))
                 {
                   send_to_char ("You can't fight your own clan members.\n\r", ch);
                   return TRUE;
                 }

                 if(victim->fight_timer > 0)
                  return FALSE;

                 if (IS_SET (victim->in_room->room_flags, ROOM_SAFE)
                 && (victim->in_room->vnum == ROOM_ARENA_LOUNGE ||
                 victim->in_room->vnum == ROOM_ARENA_HI ||
                 victim->in_room->vnum == ROOM_ARENA_LO))
                 {
                   send_to_char ("Not in this room.\n\r", ch);
                   return TRUE;
                 }

                 /* safe room? */
                 if (IS_SET (victim->in_room->affected_by, ROOM_AFF_SAFE))
                 {
                   send_to_char ("Not in this room.\n\r", ch);
                   return TRUE;
                 }


                 if (ch->on_quest)
                 {
                   send_to_char ("Not while you are on a quest.\n\r", ch);
                   return TRUE;
                 }

                 if (victim->on_quest)
                 {
                   send_to_char ("They are on a quest, leave them alone.\n\r", ch);
                   return TRUE;
                 }

		}
	}
	return FALSE;
}

bool is_safe_quiet (CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (victim->in_room == NULL || ch->in_room == NULL)
		return TRUE;

        if (IS_NPC(ch) && victim->invis_level >= LEVEL_HERO)
                return TRUE;

        /* LinkDead */
        if (!IS_NPC(victim) && victim->desc == NULL && !IS_NPC(ch)
          && victim->fighting != ch && ch->fighting != victim)
        {
         //act("$N is protected from $n by the Aesir.",ch,0,victim,TO_NOTVICT);
         //act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
         return TRUE;
        }

	if (victim->fighting == ch || victim == ch)
                return FALSE;

        if(IS_SET(victim->plyr,PLAYER_GHOST) && (!IS_NPC(ch)))
        {
          //act("$N is a ghost, you cannot attack them.",ch,NULL,victim,TO_CHAR);
          //act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
          //act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
          return TRUE;
        }
        else if(IS_SET(ch->plyr,PLAYER_GHOST))
        {
          if (!IS_NPC(victim))
          {
           //send_to_char("You're a ghost! you can't attack anyone!\n\r",ch);
           //act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
           //act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
           return TRUE;
          }

          if (IS_AFFECTED(victim,AFF_CHARM) && victim->master != NULL
          && !IS_NPC(victim->master))
          {
           //send_to_char("You're a ghost! you can't attack anyone!\n\r",ch);
           //act("The Aesir protects you from $n.",ch,NULL,victim,TO_VICT);
           //act("$N is protected from $n by the immortals.",ch,NULL,victim,TO_NOTVICT);
           return TRUE;
          }
        }

        /* Clan stuff */
        if (IS_SET(victim->plyr,PLAYER_ANCIENT_ENEMY))
        {
         if (IS_NPC(ch) && ch->spec_fun == spec_lookup("spec_cloaked_enforcer"))
                return FALSE;
        }

        if (IS_NPC(victim) && victim->spec_fun == spec_lookup("spec_cloaked_enforcer"))
         return FALSE;

	/* killing mobiles */
	if (IS_NPC (victim))
	{

          /* safe room? */
          if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
          {
            //send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }

          /* safe room? */
          if (IS_SET(victim->in_room->affected_by, ROOM_AFF_SAFE))
          {
            //send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }


          if (!IS_NPC (ch) && IS_SET (victim->in_room->room_flags, ROOM_CLAN_ENT))
          {
            //send_to_char ("Not in this room.\n\r", ch);
            return TRUE;
          }

          if (victim->pIndexData->pShop != NULL)
          {
             //send_to_char ("The shopkeeper wouldn't like that.\n\r", ch);
             return TRUE;
          }

          if (is_same_clan (ch, victim))
          {
           //send_to_char ("You can't fight your own clan members.\n\r", ch);
           return TRUE;
          }


          if (!IS_NPC(ch))
          {
            /* Only in master's PK */
            if (IS_AFFECTED(victim,AFF_CHARM) && !can_pkill(ch,victim->master))
            {
             //act("The Aesir protect $N from $n.",ch,0,victim,TO_NOTVICT);
             //act("The Aesir protect you from $n.",ch,0,victim,TO_VICT);
             //act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
                return TRUE;
            }
          }
          else
          {
           if (IS_AFFECTED(ch,AFF_CHARM) && IS_AFFECTED(victim,AFF_CHARM)
           && !can_pkill(ch->master,victim->master))
                return TRUE;
          }

        /* no killing healers, trainers, etc */
        if (IS_SET (victim->act, ACT_TRAIN)
        || IS_SET (victim->act, ACT_PRACTICE)
        || IS_SET (victim->act, ACT_IS_HEALER)
        || IS_SET (victim->act, ACT_IS_BANKER)
        || IS_SET (victim->act, ACT_IS_SATAN)
        || (victim->spec_fun == spec_lookup ("spec_questmaster"))
        || IS_SET (victim->act, ACT_IS_PRIEST))
        {
          //act ("I don't think $G would approve.", ch, NULL, NULL, TO_CHAR);
           return TRUE;
        }

/*
	 if (!IS_NPC (ch))
         {
            no pets
           if (IS_SET (victim->act, ACT_PET))
           {
             act ("But $N looks so cute and cuddly...",ch, NULL, victim, TO_CHAR);
             return TRUE;
           }

           no charmed creatures unless owner
           if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master)
           {
             send_to_char ("You don't own that monster.\n\r", ch);
             return TRUE;
           }
         }  */
	}
	/* killing players */
	else
	{
		/* NPC doing the killing */
		if (IS_NPC (ch))
		{
                        /* safe room check */
			if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
			{
				//send_to_char ("Not in this room.\n\r", ch);
				return TRUE;
                        }

                        /* safe room? */
                        if (IS_SET(victim->in_room->affected_by, ROOM_AFF_SAFE))
                        {
                         //send_to_char ("Not in this room.\n\r", ch);
                         return TRUE;
                        }

                        /* charmed mobs and pets cannot attack players while owned */
			if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
				&& ch->master->fighting != victim)
			{
				//send_to_char ("Players are your friends!\n\r", ch);
				return TRUE;
			}
		}
		/* player doing the killing */
		else
		{

            if(IS_SET (victim->act, PLR_TWIT))
                 return FALSE;

            if (victim->level < 5) 
                { 
                //send_to_char("Are you trying to be a newbie killer or something?",ch);
                return TRUE;  
                }

	         if (IS_AFFECTED (ch, AFF_CHARM) )
	         {
	           //send_to_char ("You have a master now, sit still and listen to them!\n\r", ch);
	           return TRUE;
	         }

                 // no attacking in arena unless named in current challenge
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) == 0))
                 {
                   //send_to_char ("You're not involved in the current challenge.\n\r", ch );
                   return TRUE;
                 }

                 // teammate in arena?
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) == in_current_challenge(victim->name)))
                 {
                   //send_to_char( "You can't attack your teammates.\n\r", ch );
                   return TRUE;
                 }

                 // enemy in arena?
                 if ((IS_SET (victim->in_room->room_flags, ROOM_ARENA)) &&
                 (in_current_challenge(ch->name) !=
                  in_current_challenge(victim->name)))
                  return FALSE;


                 if (!can_pkill(ch, victim))
                 {
                  //act("The Aesir protect $N from $n.",ch,0,victim,TO_NOTVICT);
                  //act("The Aesir protect you from $n.",ch,0,victim,TO_VICT);
                  //act("The Aesir protect $N from you.",ch,0,victim,TO_CHAR);
                  return TRUE;
                 }

                 if(IS_SET(victim->shielded_by,SHD_PROTECT_AURA)
                 || IS_SET(ch->shielded_by,SHD_PROTECT_AURA))
                  return TRUE;

                 if (!is_clan (ch))
                 {
                   //send_to_char ("Join a clan if you want to fight players.\n\r", ch);
                   return TRUE;
                 }

                 if (!is_pkill (ch))
                 {
                    //send_to_char ("Your clan does not allow player fighting.\n\r", ch);
                    return TRUE;
                 }

                 if (!is_clan (victim))
                 {
                    //send_to_char ("They aren't in a clan, leave them alone.\n\r", ch);
                    return TRUE;
                 }

                 if (!is_pkill (victim))
                 {
                    //send_to_char ("They are in a no pkill clan, leave them alone.\n\r", ch);
                    return TRUE;
                 }

                 if (is_same_clan (ch, victim) && !(IS_SET (victim->in_room->room_flags, ROOM_ARENA)))
                 {
                   //send_to_char ("You can't fight your own clan members.\n\r", ch);
                   return TRUE;
                 }

                 if(victim->fight_timer > 0)
                  return FALSE;

                 if (IS_SET (victim->in_room->room_flags, ROOM_SAFE)
                 && (victim->in_room->vnum == ROOM_ARENA_LOUNGE ||
                 victim->in_room->vnum == ROOM_ARENA_HI ||
                 victim->in_room->vnum == ROOM_ARENA_LO))
                 {
                   //send_to_char ("Not in this room.\n\r", ch);
                   return TRUE;
                 }

                 /* safe room? */
                 if (IS_SET (victim->in_room->affected_by, ROOM_AFF_SAFE))
                 {
                   //send_to_char ("Not in this room.\n\r", ch);
                   return TRUE;
                 }


                 if (ch->on_quest)
                 {
                   //send_to_char ("Not while you are on a quest.\n\r", ch);
                   return TRUE;
                 }

                 if (victim->on_quest)
                 {
                   //send_to_char ("They are on a quest, leave them alone.\n\r", ch);
                   return TRUE;
                 }

		}
	}
	return FALSE;
}

bool
is_safe_mock (CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (victim->in_room == NULL || ch->in_room == NULL)
		return TRUE;
	if (!IS_NPC (ch) && IS_IMMORTAL (ch))
		return FALSE;
        if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char ("Not in this room.\n\r", ch);
		return TRUE;
        } 

	if (victim->in_room->area->vnum == 61)
		return TRUE;

        /* safe room? */
        if (IS_SET (victim->in_room->affected_by, ROOM_AFF_SAFE))
        {
           send_to_char ("Not in this room.\n\r", ch);
           return TRUE;
        }
	if (IS_NPC (victim))
	{
		send_to_char ("{RYou can only use this on a player.{x\n\r", ch);
		return TRUE;
	}
	return FALSE;
}

bool
is_voodood (CHAR_DATA * ch, CHAR_DATA * victim)
{
	OBJ_DATA *object;

	if (ch->level > HERO)
		return FALSE;

	object = victim->carrying;

	return find_voodoo (ch, object);
}

// search object and any objects in the list after it to find voodoos
// matching name of ch.  If any containers are found, recurse.
// if ch is null, return TRUE if ANY voodoo is found
bool
find_voodoo (CHAR_DATA * ch, OBJ_DATA * object)
{
	char arg[MAX_INPUT_LENGTH];

	while (object != NULL)
	{
		if (object->pIndexData->vnum == OBJ_VNUM_VOODOO)
		{
			if (ch == NULL)
				return TRUE;

			one_argument (object->name, arg);
			if (!str_cmp (arg, ch->name))
				return TRUE;
		}

		if ((object->item_type == ITEM_CONTAINER) ||
			(object->item_type == ITEM_CORPSE_NPC) ||
			(object->item_type == ITEM_CORPSE_PC))
		{
			if (find_voodoo (ch, object->contains))
				return TRUE;
		}

		object = object->next_content;
	}
	return FALSE;
}

bool is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
        return is_safe(ch,victim);
	return FALSE;
}

/*
 * Check for parry.
 */
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int chance;
	int dam;

	if (!IS_AWAKE (victim))
		return FALSE;

        // No Weapon? Try Block.
        if(!IS_NPC(victim) && get_eq_char(victim,WEAR_WIELD) == NULL &&
        !has_skill(victim,gsn_parry))
         return check_block(ch,victim);

	/* less parry if blind */
	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = get_skill( victim, gsn_blind_fighting );
		chance = (chance)/2;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

	chance = get_skill (victim, gsn_parry) / 2;

	if (get_eq_char (victim, WEAR_WIELD) == NULL)
	{
         if(!IS_NPC(victim))
          return FALSE;
         else
          chance /= 2;
	}

	if (victim->stunned)
		return FALSE;

	if (!can_see (ch, victim))
		chance /= 2;

        if(IS_SET(victim->stance_aff,STANCE_AFF_ENHANCED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

        if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

	chance -= (GET_HITROLL (ch) * 0.2);
	chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 3;

        if (number_percent () >= chance + (victim->level - ch->level)/2)
		return FALSE;

        // Riposte is Counter-Parry
        if ( get_skill(ch,gsn_riposte) > 0 && (get_eq_char(ch, WEAR_WIELD) != NULL
        || get_eq_char(ch, WEAR_SECONDARY) != NULL) )
 	{
           chance = get_skill(ch,gsn_riposte)/8;
           if(can_counter(ch))
            chance += 10;

                if ( chance > number_percent())
	    	{
                        dam = number_range( 20, (ch->level*7) );
                        act( "You writhe in pain as $N lacerates you.", victim, NULL, ch, TO_CHAR);
                        act( "You slice $n into a bloody lifeless pulp.", victim, NULL, ch, TO_VICT);
                        act( "$N executes a skillful riposte.", victim, NULL, ch, TO_NOTVICT);
                        damage(ch,victim,number_range( dam,(ch->level*10) ),gsn_riposte,DAM_SLASH,TRUE,0);
			update_pos(victim);
                        check_improve(ch,gsn_riposte,TRUE,1);
                        return FALSE;
	    	}
	    	else
	    	{
                     // Swashbucklers already got counter attempt.
                     check_improve(ch,gsn_riposte,FALSE,1);
                     if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
                     act ("{)You parry $n's attack.{x", ch, NULL, victim, TO_VICT);
                     if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
                     act ("{.$N parries your attack.{x", ch, NULL, victim, TO_CHAR);
                     check_improve (victim, gsn_parry, TRUE, 6);
                     return TRUE;
	    	}
	}

        // Stance Counter
        if(can_counter(ch))
        {
         if(number_percent() < 15)
         {
            dam = number_range( 20, (ch->level*3) );
            act( "$n counters your parry!", ch, NULL, victim, TO_VICT);
            act( "You counter $N's parry and strike $M.", ch, NULL, victim, TO_CHAR);
            act( "$N counters $n's parry.", victim, NULL, ch, TO_NOTVICT);
            damage(ch,victim,number_range( dam,(ch->level*5) ),gsn_parry,DAM_SLASH,TRUE,0);
	    update_pos(victim);
            return FALSE;
         }
        }

        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
         act ("{)You parry $n's attack.{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N parries your attack.{x", ch, NULL, victim, TO_CHAR);
        check_improve (victim, gsn_parry, TRUE, 6);

	return TRUE;
}

//Barrier - Fesdor
bool check_barrier (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int chance;
	if (is_affected(victim,skill_lookup("barrier"))) //no gsn available for barrier
	{

	if (!IS_AWAKE (victim))
		return FALSE;

        chance = 35;

        if (get_curr_stat (victim, STAT_CON) > 20)
        chance += 10;
   
        if (get_curr_stat (victim, STAT_INT) > 22)
        chance += 10;
        
        if (get_curr_stat (ch, STAT_STR) > 23)
        chance -= 5;
        
        if (get_curr_stat (ch, STAT_DEX) > 23)
        chance -= 5;
        
        
    	chance -= (GET_HITROLL (ch) * 0.1);
		chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 1.5;
		chance -= (get_curr_stat (ch, STAT_WIS) - get_curr_stat (victim, STAT_WIS));
		chance -= (get_curr_stat (ch, STAT_INT) - get_curr_stat (victim, STAT_INT)) * 2;

	if(!IS_NPC(ch))
	{	
	chance += victim->level - ch->level;
	chance *= 1.25;
	}
	else
	{
		chance += victim->level - (ch->level*2/3);
		chance *= 1.5;
	}
	
	if (victim->stunned)
		chance = chance/1.3;
			
	if (number_percent () >= chance)
		return FALSE;
	else
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)Your barrier blocks $n's attack.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N's personal barrier blocks your attack!{x", ch, NULL, victim,
		 TO_CHAR);
	return TRUE;
	}
	else
	{
	return FALSE;
    }
}

/*
 * Check for shield block.
 */
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
        int chance, dam;

	if (!IS_AWAKE (victim))
		return FALSE;

	/* less shield block if blind */
	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = (get_skill( victim, gsn_blind_fighting ))/2;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

        if (get_eq_char(victim, WEAR_SHIELD) == NULL)
		return FALSE;

        chance = get_skill (victim, gsn_shield_block)*2/3;

        if(IS_SET(victim->stance_aff,STANCE_AFF_ENHANCED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

        if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

    	chance -= (GET_HITROLL (ch) * 0.15);
	chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 3;
	chance -= (get_curr_stat (ch, STAT_STR) - get_curr_stat (victim, STAT_STR)) * 2;

	if(!IS_NPC(ch))
		chance += victim->level - ch->level;
	else
		chance += victim->level - (ch->level*2/3);
		
	if (number_percent () >= chance)
		return FALSE;

	if (victim->stunned)
		return FALSE;


       /* Swashbuckler Counter-Shield-Block */
       if ( get_skill(ch,gsn_coup_de_coup) > 0 && (get_eq_char(ch, WEAR_WIELD) != NULL
       || get_eq_char(ch, WEAR_SECONDARY) != NULL) )
         {
            chance = get_skill(ch,gsn_coup_de_coup)/7;
            if(can_bypass(ch,victim))
             chance += 15;

            if( chance > number_percent())
            {
                 dam = number_range(20, (ch->level*7) );
                 act("$n slides past your shield block and impales you!",ch,NULL,victim,TO_VICT);
                 act("You counter $N's shield block and impale $M.",ch,NULL,victim,TO_CHAR);
                 act("$n deftly dances past $N's shield block and impales $N.",ch,NULL,victim,TO_NOTVICT);
                 damage(ch,victim,number_range(dam,(victim->level*7) ), gsn_coup_de_coup, DAM_PIERCE, TRUE, 0);
		 update_pos(victim);
		 check_improve(ch,gsn_coup_de_coup,TRUE,2);
		 return FALSE;
            }
            else
            {
             check_improve(ch,gsn_coup_de_coup,FALSE,3);
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You block $n's attack with your shield.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N blocks your attack with a shield.{x", ch, NULL, victim,
		 TO_CHAR);
	check_improve (victim, gsn_shield_block, TRUE, 6);
             return TRUE;
            }
         }

        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You block $n's attack with your shield.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N blocks your attack with a shield.{x", ch, NULL, victim,
		 TO_CHAR);
	check_improve (victim, gsn_shield_block, TRUE, 6);
	return TRUE;
}

bool
check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
 return check_dodge_real(ch,victim,FALSE);
}


/*
 * Check for dodge.  conquest dodge info
 */
bool
check_dodge_real(CHAR_DATA * ch, CHAR_DATA * victim, bool silent)
{
        int chance, dam;

	if (!IS_AWAKE (victim))
		return FALSE;

	/* no dodge if blind */
	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = get_skill( victim, gsn_blind_fighting );
		chance = (chance * 3) / 5;
		chance += 25;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

	chance = get_skill (victim, gsn_dodge) / 2;

	if (!can_see (victim, ch))
		chance /= 2;

        if(!IS_NPC(victim) && victim->class == CLASS_MONK) {
         if(victim->pcdata->power[1] > number_percent())
          chance *= 3/2;
        }

       /*conquest dodge start here */

    /*    if(is_affected(ch, gsn_divide))
         {
           if (number_percent() < (get_skill(ch,gsn_third_attack) / 7))
           {
             send_to_char("You spin to the side and avoid a strike!\n\r",ch);
             act("$n spins out of the way of the attack!", ch, NULL, NULL, TO_ROOM);
             return FALSE;
           }
         }
    */

         /* Stance Stuff */
        if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_DODGE) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

	chance -= (GET_HITROLL (ch) * 0.2);
	chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 3;

        if (number_percent () >= chance + (victim->level - ch->level)/2)
		return FALSE;

	if (victim->stunned)
		return FALSE;

       /* Swashbuckler Counter-Dodge */
       if ( get_skill(ch,gsn_coup_de_coup) > 0 && (get_eq_char(ch, WEAR_WIELD) != NULL
       || get_eq_char(ch, WEAR_SECONDARY) != NULL) )
         {
            chance = get_skill(ch,gsn_coup_de_coup)/5;
            if(can_bypass(ch,victim))
             chance += 15;

            if( chance > number_percent())
            {
                 dam = number_range(20, (ch->level*8) );
                 act("$n slides past your shield block and impales you!",ch,NULL,victim,TO_VICT);
                 act("You counter $N's shield block and impale $M.",ch,NULL,victim,TO_CHAR);
                 act("$n deftly dances past $N's shield block and impales $N.",ch,NULL,victim,TO_NOTVICT);
                 damage(ch,victim,number_range(dam,(victim->level*7) ), gsn_coup_de_coup, DAM_PIERCE, TRUE, 0);
		 check_improve(ch,gsn_coup_de_coup,TRUE,2);
		 return FALSE;
            }
            else
            {
             check_improve(ch,gsn_coup_de_coup,FALSE,3);
             if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
             act ("{)You dodge $n's attack.{x", ch, NULL, victim, TO_VICT);
             if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
             act ("{.$N dodges your attack.{x", ch, NULL, victim, TO_CHAR);
             check_improve (victim, gsn_dodge, TRUE, 6);
             return TRUE;
            }
         }

        if(can_bypass(ch,victim))
        {
         if(number_percent() < 15)
         {
            dam = number_range( 20, (ch->level*3) );
            act( "$n follows your dodge!", ch, NULL, victim, TO_VICT);
            act( "You follow $N's dodge and strike $M.", ch, NULL, victim, TO_CHAR);
            act( "$N follows $n's dodge and strikes $M.", victim, NULL, ch, TO_NOTVICT);
            damage(ch,victim,number_range( dam,(ch->level*5) ),gsn_dodge,DAM_SLASH,TRUE,0);
            return FALSE;
         }
        }


     if(!silent)
     {
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You dodge $n's attack.{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N dodges your attack.{x", ch, NULL, victim, TO_CHAR);
     }

        check_improve (victim, gsn_dodge, TRUE, 6);
	return TRUE;
}

bool check_phase (CHAR_DATA * ch, CHAR_DATA * victim)
{
 return check_phase_real(ch,victim,FALSE);
}

bool check_phase_real(CHAR_DATA * ch, CHAR_DATA * victim, bool silent)
{
       int chance;

       if (!IS_AWAKE (victim))
               return FALSE;

        if(IS_NPC(victim) && !IS_SET(victim->off_flags,OFF_PHASE))
               return FALSE;

       chance = get_skill (victim, gsn_phase) / 2;

       if(chance == 0) // Class don't have it....
        return FALSE;

       if (!can_see (victim, ch))
               chance /= 2;

       /* Stance Stuff (can_phase) */

       chance -= (GET_HITROLL (ch) * 0.1);

       if (number_percent () >= chance + (victim->level -
        ch->level)/2)
               return FALSE;

       if (victim->stunned)
               return FALSE;

       victim->mana -= 5;

     if(!silent)
     {
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
       act ("{)You phase out and avoid $n's attack.{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
       act ("{.$N phases out to avoid your attack.{x", ch, NULL, victim, TO_CHAR);
     }
       check_improve (victim, gsn_phase, TRUE, 6);
       return TRUE;
}


/*
 * Set position of a victim.
 */
void
update_pos (CHAR_DATA * victim)
{
	if (victim->hit > 0)
	{
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
		return;
	}

	if (IS_NPC (victim) && victim->hit < 1)
	{
		victim->position = POS_DEAD;
		return;
	}

        if (IS_NPC(victim) || victim->hit <= -11)
	{
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit <= -6)
		victim->position = POS_MORTAL;
	else if (victim->hit <= -3)
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;

	return;
}



/*
 * Start fights.
 */
void
set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{
	if (ch->fighting != NULL)
	{
		bug ("Set_fighting: already fighting", 0);
//                print_log_watch("Set_fighting: already fighting");
		return;
	}

	if (IS_AFFECTED (ch, AFF_SLEEP))
		affect_strip (ch, gsn_sleep);

	ch->fighting = victim;
	victim->fighting = ch;
	ch->position = POS_FIGHTING;
	ch->stunned = 0;

	if (!IS_NPC(ch) && !IS_NPC(victim) && !IN_ARENA(ch))
	{
	   //DESCRIPTOR_DATA *d;
	   char buf[MSL];

	   /*
	   for ( d = descriptor_list; d != NULL; d = d->next )
	   {
	      if (d->connected != CON_PLAYING)
		continue;

		sprintf ( buf, "{3[{DShadow{WLight{3]{x %s was attacked by %s at %s in %s.\n\r", ch->name, victim->name, ch->in_room->name, ch->in_room->area->name );
	      if (d->character->clan == clan_lookup("order"))
	      {
		send_to_char( buf, d->character );
	      }
	   }
	   */

	    sprintf ( buf, "{3[{DShadow{WLight{3]{V %s was attacked by %s at %s {Vin %s.",
		ch->name, victim->name, ch->in_room->name, ch->in_room->area->name );
	    wiznet ( buf, NULL, NULL, WIZ_DEATHS, 0, 0 );
	}
	return;
}



/*
 * Stop fights.
 */
void stop_fighting (CHAR_DATA * ch, bool fBoth)
{
	CHAR_DATA *fch;
	char buf[MAX_STRING_LENGTH];
	COOLDOWN_DATA *fcd = NULL;

	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		if (fch == ch || (fBoth && fch->fighting == ch))
		{
			fch->fighting = NULL;
			fch->position = IS_NPC (fch) ? fch->default_pos : POS_STANDING;
    			fch->stunned = 0;
            		fch->wait = 0;
            		//fch->assisting = 0;//So they can autoassist again.

			/* Drop the cooldowns */
			if (fch->cooldowns != NULL)
			{
				for ( fcd = fch->cooldowns; fcd != NULL; fcd = fcd->next )
				{
					cooldown_remove(fch, fcd);
				}
			}

 			update_pos (fch);
			if (IS_SET (fch->comm, COMM_STORE))
				if (fch->tells)
				{
					sprintf (buf, "You have {R%d{x tells waiting.\n\r", fch->tells);
					send_to_char (buf, fch);
					send_to_char ("Type 'replay' to see tells.\n\r", fch);
				}
		}
	}

	return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse (CHAR_DATA * ch, CHAR_DATA * killer)
{
	ROOM_INDEX_DATA *location;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	char *name;

	if (IS_NPC (ch))
	{
		if (IS_SET (ch->act, ACT_NO_BODY))
		{
			if (IS_SET (ch->act, ACT_NB_DROP))
			{
				for (obj = ch->carrying; obj != NULL; obj = obj_next)
				{
					obj_next = obj->next_content;
					obj_from_char (obj);
					if (obj->item_type == ITEM_POTION)
						obj->timer = number_range (500, 1000);
					if (obj->item_type == ITEM_SCROLL)
						obj->timer = number_range (1000, 2500);
					if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH))
					{
						obj->timer = number_range (10, 15);
						REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
					}
					REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

					if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
						extract_obj (obj);
					act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
					obj_to_room (obj, ch->in_room);
				}
			}
			return;
		}
		name = ch->short_descr;
		corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
		corpse->timer = number_range (6, 12);
		if (ch->gold > 0 || ch->platinum > 0)
		{
			obj_to_obj (create_money (ch->platinum, ch->gold, ch->silver), corpse);
			ch->platinum = 0;
			ch->gold = 0;
			ch->silver = 0;
		}
		corpse->cost = 0;
	}
	else
	{
		name = ch->name;
		corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
		corpse->timer = number_range (25, 40);
		if ((!is_clan (ch)) || (IN_ARENA(ch)))
		{
			corpse->owner = str_dup (ch->name);
			corpse->killer = NULL;
		}
		else
		{
			corpse->owner = str_dup (ch->name);
			corpse->killer = str_dup (killer->name);
			if ((ch->platinum > 1 || ch->gold > 1 || ch->silver > 1) && (!IN_ARENA(ch)))
			{
				obj_to_obj (create_money (ch->platinum / 2, ch->gold / 2, ch->silver / 2), corpse);
				ch->platinum -= ch->platinum / 2;
				ch->gold -= ch->gold / 2;
				ch->silver -= ch->silver / 2;
			}
		}

		corpse->cost = 0;
	}

	corpse->level = ch->level;
	corpse->cost = ch->level*200;

	sprintf (buf, corpse->short_descr, name);
	free_string (corpse->short_descr);
	corpse->short_descr = str_dup (buf);

	sprintf (buf, corpse->description, name);
	free_string (corpse->description);
	corpse->description = str_dup (buf);

	// in arena, stuff doesn't go into corpse
	if (ch != NULL && !IN_ARENA(ch))
	{
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		bool floating = FALSE;

		obj_next = obj->next_content;

		if (obj->wear_loc == WEAR_FLOAT)
			floating = TRUE;

              /*  if(str_cmp(obj->owner,ch->name)) */
                 obj_from_char (obj);

                if (obj->item_type == ITEM_POTION)
			obj->timer = number_range (500, 1000);
		if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range (1000, 2500);
		if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH) && !floating)
		{
			obj->timer = number_range (5, 10);
			REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
		}
		REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

		if (obj->item_type == ITEM_PASSBOOK)
		{
			change_banklist (ch, FALSE, obj->value[0], obj->value[1], 0, obj->name);
			extract_obj (obj);
		}
		else if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
		{
			extract_obj (obj);
		}
		else if (floating)
		{
			if (IS_OBJ_STAT (obj, ITEM_ROT_DEATH))	/* get rid of it! */
			{
				if (obj->contains != NULL)
				{
					OBJ_DATA *in, *in_next;

					act ("$p evaporates,scattering its contents.",
						 ch, obj, NULL, TO_ROOM);
					for (in = obj->contains; in != NULL; in = in_next)
					{
						in_next = in->next_content;
						obj_from_obj (in);
						obj_to_room (in, ch->in_room);
					}
				}
				else
					act ("$p evaporates.",
						 ch, obj, NULL, TO_ROOM);
				extract_obj (obj);
			}
			else
			{
                         if(!IS_NPC(ch))
                         {
                                act ("$p falls onto $n's corpse.", ch, obj, NULL, TO_ROOM);
                                obj_to_obj(obj,corpse);
                         }
                         else
                         {
                                act ("$p falls onto the floor.", ch, obj, NULL, TO_ROOM);
                                obj_to_room (obj, ch->in_room);
                         }
			}
		}
		else
			obj_to_obj (obj, corpse);
	}
	}

	if (!IS_NPC (ch))
	{
		if (ch->level <= 25)
		{
			if (!(location = get_room_index (ROOM_MORGUE)))
			{
				bug ("Morgue does not exist!", 0);
			}
			else
				obj_to_room (corpse, location);
		}
		else if (20000 <= ch->in_room->vnum && ch->in_room->vnum <= 20999) //clan halls
		{
			obj_to_room (corpse, get_room_index(ROOM_MORGUE));
		}
		else
		{
			obj_to_room (corpse, ch->in_room);
		}
	}
	else
		obj_to_room (corpse, ch->in_room);
//gift code
/*  Holiday Gift Code   */		
    /*      if (IS_NPC(ch) && (dice(1,30) <= 5) && ch->level > 0)
                  {
               OBJ_DATA *pill;
  	            pill = create_object(get_obj_index(41097),ch->level);
               obj_to_char(pill,killer);
               send_to_char( "{GM{Re{Gr{Rr{Gy {RC{Gh{Rr{Gist{Rm{Ga{Rs{x.\n\r", killer );
            } */
        if (IS_NPC(ch) && (dice(1,65) <= 5) && ch->level > 0 )
	{
		obj = rand_obj( killer, ch->level );
		obj_to_obj( obj, corpse );
	}


	return;
}



/*
 * Improved Death_cry contributed by Diavolo.
   OLD

void death_cry (CHAR_DATA * ch)
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;

	vnum = 0;
	msg = "You hear $n's death cry.";
	if (IS_NPC (ch))
	{
		if (!IS_SET (ch->act, ACT_NO_BODY))
		{
			switch (number_bits (4))
			{
			case 0:
				msg = "$n hits the ground ... DEAD.";
				vnum = OBJ_VNUM_BLOOD;
				break;
			case 1:
				msg = "$n splatters blood on your armor.";
				vnum = OBJ_VNUM_BLOOD;
				break;
			case 2:
				if (IS_SET (ch->parts, PART_GUTS))
				{
					msg = "$n spills $s guts all over the floor.";
					vnum = OBJ_VNUM_GUTS;
				}
				break;
			case 3:
				if (IS_SET (ch->parts, PART_HEAD))
				{
					msg = "$n's severed head plops on the ground.";
					vnum = OBJ_VNUM_SEVERED_HEAD;
				}
				break;
			case 4:
				if (IS_SET (ch->parts, PART_HEART))
				{
					msg = "$n's heart is torn from $s chest.";
					vnum = OBJ_VNUM_TORN_HEART;
				}
				break;
			case 5:
				if (IS_SET (ch->parts, PART_ARMS))
				{
					msg = "$n's arm is sliced from $s dead body.";
					vnum = OBJ_VNUM_SLICED_ARM;
				}
				break;
			case 6:
				if (IS_SET (ch->parts, PART_LEGS))
				{
					msg = "$n's leg is sliced from $s dead body.";
					vnum = OBJ_VNUM_SLICED_LEG;
				}
				break;
			case 7:
				if (IS_SET (ch->parts, PART_BRAINS))
				{
					msg = "$n's head is shattered, and $s brains splash all over you.";
					vnum = OBJ_VNUM_BRAINS;
				}
				break;
			case 8:
				if (IS_SET (ch->parts, PART_BRAINS))
				{
					msg = "$n's head is shattered, and $s brains splash all over you.";
					vnum = OBJ_VNUM_BRAINS;
				}
			case 9:
				msg = "$n hits the ground ... DEAD.";
				vnum = OBJ_VNUM_BLOOD;
			}
		}
	}
	else if (ch->level > 19)
	{
		switch (number_bits (4))
		{
		case 0:
			msg = "$n hits the ground ... DEAD.";
			vnum = OBJ_VNUM_BLOOD;
			break;
		case 1:
			msg = "$n splatters blood on your armor.";
			vnum = OBJ_VNUM_BLOOD;
			break;
		case 2:
			if (IS_SET (ch->parts, PART_GUTS))
			{
				msg = "$n spills $s guts all over the floor.";
				vnum = OBJ_VNUM_GUTS;
			}
			break;
		case 3:
			if (IS_SET (ch->parts, PART_HEAD))
			{
				msg = "$n's severed head plops on the ground.";
				vnum = OBJ_VNUM_SEVERED_HEAD;
			}
			break;
		case 4:
			if (IS_SET (ch->parts, PART_HEART))
			{
				msg = "$n's heart is torn from $s chest.";
				vnum = OBJ_VNUM_TORN_HEART;
			}
			break;
		case 5:
			if (IS_SET (ch->parts, PART_ARMS))
			{
				msg = "$n's arm is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_ARM;
			}
			break;
		case 6:
			if (IS_SET (ch->parts, PART_LEGS))
			{
				msg = "$n's leg is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_LEG;
			}
			break;
		case 7:
			if (IS_SET (ch->parts, PART_BRAINS))
			{
				msg = "$n's head is shattered, and $s brains splash all over you.";
				vnum = OBJ_VNUM_BRAINS;
			}
			break;
		case 8:
			if (IS_SET (ch->parts, PART_BRAINS))
			{
				msg = "$n's head is shattered, and $s brains splash all over you.";
				vnum = OBJ_VNUM_BRAINS;
			}
		case 9:
			if (IS_SET (ch->parts, PART_LEGS))
			{
				msg = "$n's leg is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_LEG;
			}
			break;
		case 10:
			if (IS_SET (ch->parts, PART_HEAD))
			{
				msg = "$n's severed head plops on the ground.";
				vnum = OBJ_VNUM_SEVERED_HEAD;
			}
			break;
		case 11:
			if (IS_SET (ch->parts, PART_HEART))
			{
				msg = "$n's heart is torn from $s chest.";
				vnum = OBJ_VNUM_TORN_HEART;
			}
			break;
		case 12:
			if (IS_SET (ch->parts, PART_ARMS))
			{
				msg = "$n's arm is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_ARM;
			}
			break;
		case 13:
			if (IS_SET (ch->parts, PART_LEGS))
			{
				msg = "$n's leg is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_LEG;
			}
			break;
		case 14:
			if (IS_SET (ch->parts, PART_BRAINS))
			{
				msg = "$n's head is shattered, and $s brains splash all over you.";
				vnum = OBJ_VNUM_BRAINS;
			}
		}
	}

	// no body parts in arena
	if (IN_ARENA(ch))
	{
		msg = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
	}

	act (msg, ch, NULL, NULL, TO_ROOM);

	if ((vnum == 0) && !IS_SET (ch->act, ACT_NO_BODY))
	{
		switch (number_bits (4))
		{
		case 0:
			vnum = 0;
			break;
		case 1:
			vnum = OBJ_VNUM_BLOOD;
			break;
		case 2:
			vnum = 0;
			break;
		case 3:
			vnum = OBJ_VNUM_BLOOD;
			break;
		case 4:
			vnum = 0;
			break;
		case 5:
			vnum = OBJ_VNUM_BLOOD;
			break;
		case 6:
			vnum = 0;
			break;
		case 7:
			vnum = OBJ_VNUM_BLOOD;
		}
	}

	if (vnum != 0)
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		char *name;

		name = IS_NPC (ch) ? ch->short_descr : ch->name;
		obj = create_object (get_obj_index (vnum), 0);
		obj->timer = number_range (4, 7);
		if (!IS_NPC (ch))
		{
			obj->timer = number_range (12, 18);
		}
		if (vnum == OBJ_VNUM_BLOOD)
		{
			obj->timer = number_range (1, 4);
		}

		sprintf (buf, obj->short_descr, name);
		free_string (obj->short_descr);
		obj->short_descr = str_dup (buf);

		sprintf (buf, obj->description, name);
		free_string (obj->description);
		obj->description = str_dup (buf);

		sprintf (buf, obj->name, name);
		free_string (obj->name);
		obj->name = str_dup (buf);

		if (obj->item_type == ITEM_FOOD)
		{
			if (IS_SET (ch->form, FORM_POISON))
				obj->value[3] = 1;
			else if (!IS_SET (ch->form, FORM_EDIBLE))
				obj->item_type = ITEM_TRASH;
		}

		if (IS_NPC (ch))
		{
			obj->value[4] = 0;
		}
		else
		{
			obj->value[4] = 1;
		}

		obj_to_room (obj, ch->in_room);
	}

	if (IS_NPC (ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	for (door = 0; door <= 5; door++)
	{
		EXIT_DATA *pexit;

		if ((pexit = was_in_room->exit[door]) != NULL
			&& pexit->u1.to_room != NULL
			&& pexit->u1.to_room != was_in_room)
		{
			ch->in_room = pexit->u1.to_room;
			act (msg, ch, NULL, NULL, TO_ROOM);
		}
	}
	ch->in_room = was_in_room;

	return;
}
*/

/*
 * New death_cry -- Skyntil.
*/
void death_cry (CHAR_DATA * ch)
{
/*	ROOM_INDEX_DATA *was_in_room; */
	char *msg;
/*	int door; */

	if (IS_NPC (ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";
/*
	was_in_room = ch->in_room;

	for (door = 0; door <= 5; door++)
	{
		EXIT_DATA *pexit;

		if ((pexit = was_in_room->exit[door]) != NULL
			&& pexit->u1.to_room != NULL
			&& pexit->u1.to_room != was_in_room)
		{
			ch->in_room = pexit->u1.to_room;
			act (msg, ch, NULL, NULL, TO_ROOM);
		}
	}

	ch->in_room = was_in_room;
*/
	return;
}



void raw_kill (CHAR_DATA * victim, CHAR_DATA * killer)
{
		int i;
        int arena = FALSE;
        CHAR_DATA *groupie, *groupie_next;
        char buf[100];

	sprintf (log_buf, "%s got toasted by %s {Vat %s {V[room %d]",
			 (IS_NPC (victim) ? victim->short_descr : victim->name),
			 (IS_NPC (killer) ? killer->short_descr : killer->name),
			 killer->in_room->name, killer->in_room->vnum);


	if (IS_NPC (victim))
		wiznet (log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
	else
		wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

        /* Quickening for Highlanders */
        if(killer->class == CLASS_HIGHLANDER && !IS_NPC(victim) && !IN_ARENA(killer))
        {
         act("Lightning strikes $n and $e starts to glow with power.",killer,NULL,NULL,TO_ROOM);
         act("Lightning strikes you as you start feeling the quickening.",killer,NULL,NULL,TO_CHAR);
/*         if(killer->pcdata->power[POWER_KILLS] < 65500)
         killer->pcdata->power[POWER_KILLS] += 1;
         if(killer->pcdata->power[POWER_POINTS] < 65500)
         killer->pcdata->power[POWER_POINTS] += dice(20,75); */
         update_power(killer);
        }
	/*
        else if(killer->class == CLASS_HIGHLANDER && IS_NPC(victim))
        {
         update_power(killer);

         if(number_percent() < 28)
         {
          send_to_char("You skin crawls with electricity for a moment.\n\r",killer);
          if(killer->pcdata->power[POWER_POINTS] < 65500)
          killer->pcdata->power[POWER_POINTS] += dice(1,10);
          update_power(killer);
         }
        }
	*/

        if(victim->class == CLASS_HIGHLANDER && !IS_NPC(killer) && !IN_ARENA(victim))
        {
         act("Lightning strikes $n and $e writhes in pain.",victim,NULL,NULL,TO_ROOM);
         act("Lightning strikes you and you writhe in pain.",victim,NULL,NULL,TO_CHAR);
/*         if(victim->pcdata->power[POWER_KILLS] >= 1)
         victim->pcdata->power[POWER_KILLS] -= 1;
         if(victim->pcdata->power[POWER_POINTS] >= 500)
         victim->pcdata->power[POWER_POINTS] -= dice(35,75); */
         update_power(victim);
        }

        for(groupie = killer->in_room->people; groupie != NULL; groupie = groupie_next)
        {
                groupie_next = groupie->next_in_room;

         if(is_same_group(killer,groupie) && is_affected(groupie,gsn_decapitate))
            affect_strip(groupie,gsn_decapitate);
        }

        if(is_affected(killer,gsn_decapitate))
        affect_strip(killer,gsn_decapitate);


	if (victim->morph_form[0] == MORPH_CONCEAL)
	{
	   if(is_affected(victim, gsn_conceal))
	   {
		affect_strip(victim, gsn_conceal);
	   }
	  /* if(victim->long_descr != NULL || !str_cmp(victim->long_descr,"A dark assassin stands here.\n\r"))
	   {
		free_string(victim->long_descr);
		victim->long_descr = "\0";
	   }*/
	}

	if(victim->morph_form[0] > 0)
	   victim->morph_form[0] = 0;

	if (!IS_NPC (victim) && !IN_ARENA(victim))
	{
		/*
		 * Dying penalty:
		 * 2/3 way back to previous level.
		 */
		if (victim->exp > exp_per_level (victim, victim->pcdata->points)
			* victim->level)
			gain_exp (victim, (2 * (exp_per_level (victim, victim->pcdata->points)
								  * victim->level - victim->exp) / 3) + 50);
	}

	if (!IS_NPC(victim) && IN_ARENA(victim))
		arena = TRUE;

	death_cry (victim);
	stop_fighting (victim, TRUE);
	make_corpse (victim, killer);

	if (IS_NPC (victim))
	{
		victim->pIndexData->killed++;
		kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
		extract_char (victim, TRUE);
		return;
	}

	if (!IS_NPC (killer))
	{
		// no self-kills on record
		// no arena kills/deaths on record
        // no immortal deaths on record
        if ((killer != victim) && (!IS_SET(victim->in_room->room_flags,ROOM_ARENA)) && !IS_IMMORTAL(killer) && !IS_IMMORTAL(victim))
		{
			// killer gets no kills for TWITs
			if (!IS_SET (victim->act, PLR_TWIT))
				killer->pcdata->pkills++;
			    victim->pcdata->pdeath++;
		if(IS_SET(victim->plyr, PLAYER_NEWBIE)) {
			victim->pcdata->prank -= (rank_pk(killer,victim)/2); }
		else {
			victim->pcdata->prank -= rank_pk(killer,victim); }
			killer->pcdata->prank += rank_pk(killer,victim);

			if (victim->pcdata->prank <= 0)
				victim->pcdata->prank = 1;
			
		 	if (IS_SET(killer->plyr, PLAYER_NEWBIE) && (killer->pcdata->prank >= 1530))
				REMOVE_BIT(killer->plyr, PLAYER_NEWBIE);

                        // Rack em up for the clans
                        if(victim->clan != 0)
                         clan_table[victim->clan].deaths += 1;
                        if(killer->clan != 0)
                         clan_table[killer->clan].kills += 1;
		}
        else if ((killer != victim) && (IS_SET(victim->in_room->room_flags,ROOM_ARENA)) && !IS_IMMORTAL(killer) && !IS_IMMORTAL(victim))
		{
			
			victim->pcdata->arank -= rank_arena(killer,victim);
			killer->pcdata->arank += rank_arena(killer,victim);
			killer->pcdata->awins++;
			victim->pcdata->alosses++;
			if (victim->pcdata->arank <= 0)
				victim->pcdata->arank = 1;
		}

		if (!is_banklist (victim))
		{
			int bank;

			bank = number_range (0, 3);
			if ((number_range (1, 100) < 33) && victim->balance[bank])
			{
				char buf[MAX_STRING_LENGTH];
				int amount;
				int pwd;
				OBJ_DATA *pbook;
				EXTRA_DESCR_DATA *ed;

				amount = (number_range (1, 75) / 100) * victim->balance[bank];
				pwd = number_range (1, 20000);
				pbook = create_object (get_obj_index (OBJ_VNUM_PASSBOOK), 0);
				sprintf (buf, "%s %s", capitalize (victim->name), pbook->name);
				free_string (pbook->name);
				pbook->name = str_dup (buf);
				sprintf (buf,
						 "The passbook is covered with strange magical symbols that prevent your eyes\n\rfrom focusing on the inscriptions.  Only the word {B%s{x is legible.\n\r", capitalize (victim->name));
				ed = alloc_perm (sizeof (*ed));
				ed->keyword = str_dup ("passbook");
				ed->description = str_dup (buf);
				ed->next = pbook->extra_descr;
				pbook->extra_descr = ed;
				pbook->value[0] = bank;
				pbook->value[1] = pwd;
				pbook->value[2] = amount;
				obj_to_char (pbook, killer);
				send_to_char ("{RA passbook appears in your inventory!{x\n\r", killer);
				change_banklist (killer, TRUE, bank, amount, pwd, victim->name);
			}
		}
           }
        extract_char (victim, FALSE);

	if (!arena)
	{
		while (victim->affected)
			affect_remove (victim, victim->affected);
		victim->affected_by = race_table[victim->race].aff;
		victim->shielded_by = race_table[victim->race].shd;
		for (i = 0; i < 4; i++)
			victim->armor[i] = 100;
	}

	if (arena)
	{
		victim->position = POS_STANDING;
		victim->hit = victim->max_hit;
		victim->mana = victim->max_mana;
		victim->move = victim->max_move;
                if(victim->affected)
                {
                 if(victim->affected->type == skill_lookup("curse"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("blindness"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("plague"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("poison"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("fire breath"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("chill touch"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("slow"))
                  affect_remove(victim,victim->affected);
                 else if(victim->affected->type == skill_lookup("weaken"))
                  affect_remove(victim,victim->affected);
				 else if(victim->affected->type == skill_lookup("charm"))
				  affect_remove(victim,victim->affected);
				 else if(victim->affected->type == skill_lookup("shriek"))
				  affect_remove(victim, victim->affected);
				 else if(victim->affected->type == skill_lookup("fear"))
				  affect_remove(victim, victim->affected);
                 else if(victim->affected->type == skill_lookup("faerie fire"))
				  affect_remove(victim, victim->affected);
                 else if(victim->affected->type == skill_lookup("smokebomb"))
				  affect_remove(victim, victim->affected);
                 else if(victim->affected->type == skill_lookup("wither"))
				  affect_remove(victim, victim->affected);
                 else if(victim->affected->type == skill_lookup("feeble mind"))
				  affect_remove(victim, victim->affected);
		 	  	 else if(victim->affected->type == skill_lookup("nerve"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("shriek"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("garrote"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("nerve"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("fear aura"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("burning skin"))
			 	  affect_remove(victim, victim->affected);
			 	 else if(victim->affected->type == skill_lookup("dirt kicking"))
			 	  affect_remove(victim, victim->affected);
                }
	}
	else
	{
		victim->position = POS_RESTING;
		victim->hit = UMAX (1, victim->hit);
		victim->mana = UMAX (1, victim->mana);
		victim->move = UMAX (1, victim->move);
	}

        /* Set GHOST bits */
        if(!IS_NPC(killer) && !arena && killer != victim)
        {
         SET_BIT(victim->plyr,PLAYER_GHOST);
         victim->ghost = str_dup(killer->name);
         victim->ghost_timer = 90; //was 15 for 10 min real time -bree
         killer->ghost = str_dup(victim->name);
         killer->ghost_timer = 15;
         victim->fight_timer = 0;
         if(killer->clan != 0 && !IS_IMMORTAL(killer))
         {
          sprintf(buf,"I have killed %s!",capitalize(victim->name));
          do_clantalk(killer,buf);
         }
        }
        if(!IS_NPC(killer) && !IS_NPC(victim))
         death_message(killer,victim);

	return;
}

/*
 * Death Messages through Announce for PK.
 * Can be personalized for each class. Currently for base classes.
 * -- Skyntil 7/30/00
 */
void death_message(CHAR_DATA *killer, CHAR_DATA *victim)
{
/*  char buf[MSL/4]; */
  sh_int baseclass = 0;

  if(killer == victim)
   return;
/*
  if(!str_cmp(killer->pcdata->target,victim->name))
  {
   act("{Y[INFO]: $N has been {1A{!SSASINATE{1D {Yby $n!{x",killer,NULL,victim,TO_WORLD);
   return;
  }*/
  
  if(killer->class == CLASS_MAGE
  || killer->class == CLASS_CLERIC
  || killer->class == CLASS_WARRIOR
  || killer->class == CLASS_THIEF
  || killer->class == CLASS_RANGER
  || killer->class == CLASS_SHADE
  || killer->class == CLASS_DRUID)
   baseclass = killer->class;

  if(killer->class == CLASS_WIZARD
  || killer->class == CLASS_PRIEST
  || killer->class == CLASS_GLADIATOR
  || killer->class == CLASS_MERCENARY
  || killer->class == CLASS_STRIDER
  || killer->class == CLASS_LICH
  || killer->class == CLASS_SAGE)
   baseclass = killer->class - 7;

  if(killer->class == CLASS_FORSAKEN
  || killer->class == CLASS_CONJURER
  || killer->class == CLASS_ARCHMAGE)
   baseclass = CLASS_MAGE;

  if(killer->class == CLASS_VOODAN
  || killer->class == CLASS_SAINT
  || killer->class == CLASS_MONK)
   baseclass = CLASS_CLERIC;

/*  if(killer->class == CLASS_MONK)
   baseclass = CLASS_MONK; */

  if(killer->class == CLASS_ASSASSIN
  || killer->class == CLASS_NINJA
  || killer->class == CLASS_BARD)
   baseclass = CLASS_THIEF;

  if(killer->class == CLASS_SWASHBUCKLER
  || killer->class == CLASS_CRUSADER
  || killer->class == CLASS_HIGHLANDER)
   baseclass = CLASS_WARRIOR;

/*  if(killer->class == CLASS_HIGHLANDER)
   baseclass = CLASS_HIGHLANDER; */

  if(killer->class == CLASS_DARKPALADIN
  || killer->class == CLASS_HUNTER
  || killer->class == CLASS_PALADIN)
   baseclass = CLASS_RANGER;

  if(killer->class == CLASS_ALCHEMIST
  || killer->class == CLASS_SHAMAN
  || killer->class == CLASS_WARLOCK)
   baseclass = CLASS_DRUID;

/*  if(killer->class == CLASS_WARLOCK)
   baseclass = CLASS_WARLOCK; */

  if(killer->class == CLASS_FADE
  || killer->class == CLASS_NECROMANCER
  || killer->class == CLASS_BANSHEE)
   baseclass = CLASS_SHADE;

	switch(killer->class)
	{
	case CLASS_MAGE:
   	act("{Y[INFO]: $n {Ychannels energy into $N and causes $M to explode violently!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_WIZARD:
	act("{Y[INFO]: $n {Ysummons forth arcane magicks, engulfing $N in a fiery inferno!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_FORSAKEN:
	act("{Y[INFO]: $n {Ycommands a haunting entity to devastate $N's body and soul!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_CONJURER:
	act("{Y[INFO]: $n {Yconjures forth an army of beasts that turn $N into a fine mist!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_ARCHMAGE:
	act("{Y[INFO]: $n {Ycalls down an onslaught of elemental forces, tearing the flesh from $N's bones!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_CLERIC:
	act("{Y[INFO]: $n {Ycalls upon the Aesir to destroy $N's mortal body!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_PRIEST:
	act("{Y[INFO]: $n {Yrecites an ancient death prayer as $N is launched into eternity!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_VOODAN:
	act("{Y[INFO]: $n {Yutters {Ddark{Y incantations causing $N's head to shrivel and fall off $s shoulders!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_MONK:
	act("{Y[INFO]: $n {Ygrabs $N's body and brings $M down hard over $s knee! $N's spine breaks in half and {rblood{Y spurts onto $n!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_SAINT:
	act("{Y[INFO]: $n {Yraises a hand to the sky as $N is disintegrated by a bolt from the heavens!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_THIEF:
	act("{Y[INFO]: $n {Yinverts $s dagger and rams it through $N's skull! $N's brains splatter onto the floor!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_MERCENARY:
	act("{Y[INFO]: $n {Yassaults $N in physical combat, beating $M to a lifeless pulp!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_ASSASSIN:
	act("{Y[INFO]: $n {Yappears behind $N and {Rslits{Y their throat! Blood gushes forth from the gaping wound!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_NINJA:
	act("{Y[INFO]: $n {Yleaps from a cloud of {Ws{dm{Do{dk{We{Y and impales $N in the throat! A fountain of {rblood {Ysprays from $N's wound!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_BARD:
	act("{Y[INFO]: $n {Ysings a sweet song and lulls $N into an eternal sleep!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_WARRIOR:
	act("{Y[INFO]: $n {Yplunges $s weapon deep inside $N! {rBlood{Y flows from $N's mortal wounds!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_GLADIATOR:
	act("{Y[INFO]: $n {Ybashes into $N with a forceful blow. $N's body falls into a withered heap on the ground!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_HIGHLANDER:
	act("{Y[INFO]: $N's {Yhead falls onto the floor as $n brutally decapitates $M!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_SWASHBUCKLER:
	act("{Y[INFO]: $n {Yslides past $N's parry and severs $M's abdomen! $N's entrails leak onto the ground!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_CRUSADER:
	act("{Y[INFO]: $n {Ymurmers a quick prayer, then vaporizes $N with a mightly stroke!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_RANGER:
	act("{Y[INFO]: $n {Yspins in a circle and slices $N to pieces!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_STRIDER:
	act("{Y[INFO]: $n {Ydarts past $N as a pack of wolves take down $N in a fury of tooth and claw!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_DARKPALADIN:
	act("{Y[INFO]: $N {Yfalls to the ground as $n brutally impales them!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_HUNTER:
	act("{Y[INFO]: $n {Ycalls upon the {Gfo{gre{Gst{Y as vines wrap around $N and start crushing $M!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_PALADIN:
	act("{Y[INFO]: $n {Ypurges the lands of $N!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_DRUID:
	act("{Y[INFO]: $n {Ychants words of magic and $N crumples into a lifeless husk!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_SAGE:
	act("{Y[INFO]: $n {Ymurmurs an arcane word. $N explodes into dust!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_WARLOCK:
	act("{Y[INFO]:$n shoots a chain of lightning from $s fingertips turning $N's body into a smoldering pile of {Da{ds{dh{Y!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_ALCHEMIST:
	act("{Y[INFO]: $n {Ycreates a mystic rune that flares and incinerates $N in {Bb{blu{Be {RF{rl{Wa{rm{Re{Y!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_SHAMAN:
	act("{Y[INFO]: $n {Ybegins to chant as a thousand spirits surround $N and rip $M apart!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_SHADE:
	act("{Y[INFO]: $n {Ybites $N's neck and drinks from $M life {rblood{Y! $N becomes a lifeless, shriveled corpse!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_LICH:
	act("{Y[INFO]: $n {Yreaches into $N's chest and rips $M's {Rh{de{Wa{dr{Rt{Y out, purging their soul from the mortal realms!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_FADE:
	act("{Y[INFO]: $n {Yeyes glow {Rred{Y as $N's life energies are drained from their body!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_NECROMANCER:
	act("{Y[INFO]: $n {Ywaves $s hands mystically as their legion of dead decimate $N!{x",killer,NULL,victim,TO_WORLD);
	break;


	case CLASS_BANSHEE:
	act("{Y[INFO]: $N's {Yhair turns {Wwhite{Y as $n shrieks at them! {RBl{ro{Rod{Y flows from $N's eyes as they fall lifeless!{x",killer,NULL,victim,TO_WORLD);
	break;


	default:
	act("{Y[INFO]: $N {Yhas been brutally murdered by $n!{x",killer,NULL,victim,TO_WORLD);
	break;
	}
	return;
}

void group_gain (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;
	CHAR_DATA *lch;
	int xp;
	int members;
	int group_levels;

	/*
	 * Monsters don't get kill xp's or alignment changes.
	 * P-killing doesn't help either.
	 * Dying of mortal wounds or poison doesn't give xp to anyone!
	 */
	if (IS_NPC(ch))
	{
    		if (ch->leader != NULL) 
    		{
            		ch = ch->leader;
    		}
	}
	if (IS_SET (ch->act, PLR_QUESTOR) && IS_NPC (victim))
	{
		if (ch->questmob == victim->pIndexData->vnum)
		{
			send_to_char ("YOU have almost completed your QUEST!\n\r", ch);
			send_to_char ("Return to the Questmaster before your time runs out!\n\r", ch);
			ch->questmob = -1;
		}
	}
	if (victim == ch)
		return;

    /* XP for PK's .... */
    /* IS_SET(ch->plyr,PLAYER_SUBDUE) && taken out for a moment */
    if (!IS_NPC(victim) && !IN_ARENA(ch))
    {
        xp = 1000;
        sprintf( buf, "You receive %d experience points.\n\r", xp );
        send_to_char( buf, ch );
        gain_exp( ch, xp );
        return;
    }
    else if(!IS_NPC(victim))
    return;

	members = 0;
	group_levels = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (is_same_group (gch, ch))
		{
			if(!IS_NPC(gch)) // Only count PCs
			members++;

//			if (gch != ch)
//			group_levels += IS_NPC (gch) ? 0 : gch->level;
//			else
//				group_levels += gch->level;

			group_levels = UMAX(group_levels,1);

		}

	}

	if (members == 0)
	{
		bug ("Group_gain: members.", members);
		members = 1;
		group_levels = ch->level;
	}

	lch = (ch->leader != NULL) ? ch->leader : ch;
	
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_next;

	    if ( !is_same_group ( gch, ch ) || IS_NPC ( gch ) )
	 	continue;

            xp = xp_compute(gch, victim, group_levels, members);
                
            if ( ( dbl_exp_lvl ) && ( ch->level <= 100 ) )
            {                
               sprintf (buf, "{BYou receive {W%d{B experience points for being under level 101.{Y(Double EXP){x\n\r", xp*2);
	       send_to_char (buf, gch);
	       gain_exp (gch, xp*2);
	    }
            else if ( dbl_exp || timed_dbl_exp )
            {
               sprintf (buf, "{BYou receive {W%d{B experience points.{Y(Double EXP){x\n\r", xp*2);
               send_to_char (buf, gch);
               gain_exp (gch, xp*2);
            }
	    else
	    {
		sprintf (buf, "{BYou receive {W%d{B experience points.{x\n\r", xp);
		send_to_char (buf, gch);
		gain_exp (gch, xp);
	    }    
		if (gch->desc)
		{
			if (gch->desc->out_compress)
			{
				if (xp >= 10) 
				{
					sprintf (buf, "{BYou receive {W%d{B extra experience for having MCCP!{x\n\r", xp/10);
					gain_exp (gch, xp/10);
				}
				send_to_char(buf,gch);
			}
		}
		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE)
				continue;

			if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch))
				|| (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch))
				|| (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
			{
				act ("{cYou are {Wzapped{c by $p.{x", ch, obj, NULL, TO_CHAR);
				act ("$n is {Wzapped{x by $p.", ch, obj, NULL, TO_ROOM);
				obj_from_char (obj);
				obj_to_char(obj,ch);
				//obj_to_room (obj, ch->in_room);
			}
		}
	}

	return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels, int members)
{
	int xp, base_exp;
	int align, level_range;
	int change;
/*	int time_per_level; */
        double mult;

         /* No Experience and no Alignment Change */
        if (IN_ARENA(gch))
          return 0;

        mult= ((double)(gch->level)/total_levels)*members;
        if (mult >= 1)
        {
         mult = (1 + mult) / 2;
        }
        else
        {
         mult = mult * mult;
        }
        mult = URANGE(.25, mult, 1.1);

	level_range = victim->level - gch->level;

	if (!IS_NPC (gch))
	{
		if (gch->pcdata->tier > 1)
			level_range -= (gch->pcdata->tier*2);
	}


        /* killing players */
	if (!IS_NPC (gch) && !IS_NPC (victim))
	{
                xp = (victim->level + 25) - (gch->level);
		xp = xp * 20;
		xp = number_range (xp * 3 / 4, xp * 5 / 4);
		return xp;
	}

	/* compute the base exp */
        switch (level_range)
        {
         default :       base_exp =   0;         break;
         case -9 :       base_exp =   2;         break;
         case -8 :       base_exp =   4;         break;
         case -7 :       base_exp =   7;         break;
         case -6 :       base_exp =  12;         break;
         case -5 :       base_exp =  18;         break;
         case -4 :       base_exp =  27;         break;
         case -3 :       base_exp =  40;         break;
         case -2 :       base_exp =  53;         break;
         case -1 :       base_exp =  70;         break;
         case  0 :       base_exp =  88;         break;
         case  1 :       base_exp = 106;         break;
         case  2 :       base_exp = 128;         break;
         case  3 :       base_exp = 150;         break;
         case  4 :       base_exp = 168;         break;
         case  5 :       base_exp = 184;         break;
         case  6 :       base_exp = 198;         break;
         case  7 :       base_exp = 210;         break;
	}
        if (level_range > 7)
           base_exp = 225 + 15 * (level_range - 4);
        if (mult < 1 && level_range > 7)
           base_exp = (2 * base_exp + 225) / 3;

	/* do alignment computations */
        align = gch->alignment;

        // Alignment Changes
        change = align_compute(gch,victim,total_levels,members);

        // Change Align but Min is -1000 and Max is 1000
        gch->alignment = UMAX (-1000, gch->alignment + change);
        gch->alignment = UMIN (1000, gch->alignment);

        if (gch->pet != NULL)
         gch->pet->alignment = gch->alignment;

	/* calculate exp multiplier */
	if (IS_SET (victim->act, ACT_NOALIGN))
		xp = base_exp;
	else if (gch->alignment > 500)	/* for goodie two shoes */
	{
		if (victim->alignment < -750)
			xp = (base_exp * 4) / 3;

		else if (victim->alignment < -500)
			xp = (base_exp * 5) / 4;

		else if (victim->alignment < -250)
			xp = (base_exp * 3) / 4;

                else if (victim->alignment > 250)
			xp = (base_exp * 3) / 4;

		else if (victim->alignment > 500)
			xp = base_exp / 2;

		else if (victim->alignment > 750)
			xp = base_exp / 4;
		else
			xp = base_exp;
	}

	else if (gch->alignment < -500)		/* for baddies */
	{
		if (victim->alignment > 750)
                        xp = (base_exp * 4) / 3;

		else if (victim->alignment > 500)
                        xp = (base_exp * 5) / 4;

                else if (victim->alignment > 250)
                        xp = (base_exp * 3) / 4;

		else if (victim->alignment < -250)
                        xp = (base_exp * 3) / 4;

		else if (victim->alignment < -500)
                        xp = base_exp / 2;

		else if (victim->alignment < -750)
                        xp = base_exp / 4;

		else
			xp = base_exp;
	}

	else if (gch->alignment > 200)	/* a little good */
	{

		if (victim->alignment < -500)
			xp = (base_exp * 6) / 5;

		else if (victim->alignment > 750)
			xp = base_exp / 2;

		else if (victim->alignment > 0)
			xp = (base_exp * 3) / 4;

		else
			xp = base_exp;
	}

	else if (gch->alignment < -200)		/* a little bad */
	{
		if (victim->alignment > 500)
			xp = (base_exp * 6) / 5;

		else if (victim->alignment < -750)
			xp = base_exp / 2;

		else if (victim->alignment < 0)
			xp = (base_exp * 3) / 4;

		else
			xp = base_exp;
	}

	else
		/* neutral */
	{

		if (victim->alignment > 500 || victim->alignment < -500)
			xp = (base_exp * 4) / 3;

		else if (victim->alignment < 200 && victim->alignment > -200)
			xp = base_exp / 2;

		else
			xp = base_exp;
	}
/*
	if (gch->level <20)
	  xp = 20 * xp / (gch->level + 2);

	// more exp at the low levels 
        if (gch->level >=20 && gch->level <= 50)
          xp = 15 * xp / (gch->level + 2);

	// less at high 
	if (gch->level > 60 && gch->level < 90)
          xp = 13 * xp / (gch->level - 25);

 	// even less at 90+ 
	if (gch->level >= 90)
          xp = 8 * xp / (gch->level - 25);
*/
	xp = 12 * xp / gch->level;
	/* reduce for playing time */
/*        {
          time_per_level = 4 * (gch->played + (int) (current_time - gch->logon)) / 3600 / gch->level;

          time_per_level = URANGE (2, time_per_level, 12);
          if (gch->level < 25)
           time_per_level = UMAX (time_per_level, (25 - gch->level));
         xp = xp * time_per_level / 12;
	}
        */

	/* randomize the rewards */
	xp = number_range (xp * 3 / 4, xp * 5 / 4);

	/*if (is_pkill(gch) || gch->level > 100)
	{
		xp *= 4;
		xp /= 3;
	}
	else
	{
		xp *= 4;
		xp /= 7;
	}*/

	/* adjust for grouping */ 
    	if (members == 2) 
        	xp = (xp * 6)/5; 
    	if (members == 3) 
        	xp = (xp * 10)/5; 
    	if (members == 4) 
        	xp = (xp * 8)/5; 
        if (members > 4)
        	xp = (xp * 7)/5;

       if(xp > 1250 && gch->level < total_levels)
            xp = 1250;
       else if (gch->level < 101)
	    {
		    xp *= 1.5;
		    
		    if(xp > 4500)
       		{
	    	xp = 4500;
    		}
	    }
	   else if (xp > 3000)
	   xp = 3000;
	   return xp;
}

/*
 * Compute alignment for a kill.
 * Edit this function to change align computations.
 */
int align_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels, int members)
{
        int /*align,*/ change = 0;

        /* Alignment is based on what is killed.
           There are several cases in which alignment
           computation must be handled differently.
           The degrees of align are:
           SUPREMELY GOOD, GOOD, SEMI-GOOD,
           NEUTRAL, SEMI-EVIL, EVIL, and
           SUPREMELY EVIL
           Each case must be matched up */

        /*
         * CASE: SUPREME GOOD
         */
        if (gch->alignment > 750)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(10,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = number_range(5,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = number_range(3,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = number_range(3,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = number_range(5,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(10,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: GOOD
         */
        else if (gch->alignment > 500)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: SEMI-GOOD
         */
        else if (gch->alignment > 250)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: NEUTRAL
         */
        if (gch->alignment <= 250 && gch->alignment >= -250)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: SEMI-EVIL
         */
        else if (gch->alignment < -250)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: EVIL
         */
        else if (gch->alignment < -500)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;
	}
        /*
         * CASE: SUPREMELY EVIL
         */
        else if (gch->alignment < -750)
	{
                /*
                 * VICTIM: SUPREME GOOD
                 */
                if(victim->alignment > 750)
                {
                 change = -number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change *= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change /= 2;
                }
                /*
                 * VICTIM: GOOD
                 */
                else if(victim->alignment > 500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SEMI-GOOD
                 */
                else if(victim->alignment > 250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: NEUTRAL
                 */
                else if(victim->alignment <= 250 && victim->alignment >= -250)
                 change = 0;
                /*
                 * VICTIM: SEMI-EVIL
                 */
                else if(victim->alignment < -250)
                {
                 change = -number_range(15,victim->level/4);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: EVIL
                 */
                else if(victim->alignment < -500)
                {
                 change = -number_range(25,victim->level/2);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                /*
                 * VICTIM: SUPREMELY EVIL
                 */
                else if(victim->alignment < -750)
                {
                 change = number_range(40,victim->level);
                 if(victim->level < (gch->level - victim->level/5))
                  change /= 2;
                 if((gch->level + victim->level/5) < victim->level)
                  change *= 2;
                }
                else
                 change = 0;

	}

        change = change * gch->level / total_levels;

        if(members > 3)
         change /= members;

        /* Hold Align */
        if(is_affected(gch,gsn_hold_align))
         change = 0;

        return URANGE( -100, change, 100 );
}


void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune)
{
 dam_message_new(ch,victim,dam,dt,immune,DMF_NONE);
}

void dam_message_new(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune,
int dam_flags)
{
	char buf1[256], buf2[256], buf3[256];
	const char *vs;
	const char *vp;
	const char *attack;
        int max_damage, damp, dam_type = DAM_NONE;
	char punct;
        char *msg;

        if (ch == NULL || victim == NULL)
		return;

        if (IS_NPC(victim))
         max_damage = 0;
        else
         max_damage = -10;


    if ( (victim->hit - dam > max_damage ) || (dt >= 0 && dt < MAX_SKILL)
         || IS_NPC(victim) || IS_SET(victim->act,ACT_NO_BODY) )
    {

	if (dam == 0)
	{
		vs = "miss";
		vp = "{&misses";
	}
	else if (dam <= 4)
	{
		vs = "{1scratch";
		vp = "{1scratches";
	}
	else if (dam <= 6)
	{
		vs = "{2blemish";
		vp = "{2blemishes";
	}
	else if (dam <= 8)
	{
		vs = "{3scathe";
		vp = "{3scathes";
	}
	else if (dam <= 10)
	{
		vs = "{6hurt";
		vp = "{6hurts";
	}
	else if (dam <= 14)
	{
		vs = "{7graze";
		vp = "{7grazes";
	}
	else if (dam <= 18)
	{
		vs = "{8hit";
		vp = "{8hits";
	}
	else if (dam <= 22)
	{
		vs = "{1injure";
		vp = "{1injures";
	}
	else if (dam <= 26)
	{
		vs = "{2harm";
		vp = "{2harms";
	}
	else if (dam <= 30)
	{
		vs = "{3maim";
		vp = "{3maims";
	}
	else if (dam <= 34)
	{
		vs = "{!B{@A{!T{@T{!E{@R";
		vp = "{!B{@A{!T{@T{!E{@R{!S";
	}
	else if (dam <= 40)
	{
		vs = "{@C{#O{@N{#T{@O{#R{@T{";
		vp = "{@C{#O{@N{#T{@O{#R{@T{#S";
	}
	else if (dam <= 44)
	{
		vs = "{#M{^A{#N{^G{#L{^E";
		vp = "{#M{^A{#N{^G{#L{^E{#S";
	}
	else if (dam <= 48)
	{
		vs = "{$M{%A{$R";
		vp = "{$M{%AR{$S";
	}
	else if (dam <= 52)
	{
		vs = "{%G{^AS{%H";
		vp = "{%G{^A{%S{^H{%E{^S";
	}
	else if (dam <= 56)
	{
		vs = "{^I{&M{^P{&A{^I{&R";
		vp = "{^I{&M{^P{&A{^I{&R{^S";
	}
	else if (dam <= 60)
	{
		vs = "{&M{!U{&T{!I{&L{!A{&T{!E";
		vp = "{&M{!U{&T{!I{&L{!A{&T{!E{&S";
	}
	else if (dam <= 64)
	{
		vs = "{!D{#I{!S{#E{!M{#B{!O{#W{!E{#L";
		vp = "{!D{#I{!S{#E{!M{#B{!O{#W{!E{#L{!S";
	}
	else if (dam <= 70)
	{
		vs = "{@D{&I{@S{&M{@E{&M{@B{&E{@R";
		vp = "{@D{&I{@S{&M{@E{&M{@B{&E{@R{&S";
	}
	else if (dam <= 75)
	{
		vs = "{1={!={&= {1T{!HR{&A{!SH{1E {&={!={1=";
		vp = "{1={!={&= {1T{!HR{&AS{!HE{1S {&={!={1=";
	}
	else if (dam <= 80)
	{
		vs = "{2+{@+{&+ {2M{@AS{&SA{@CR{2E {&+{@+{2+";
		vp = "{2+{@+{&+ {2M{@AS{&SAC{@RE{2S {&+{@+{2+";
	}
	else if (dam <= 85)
	{
                vs = "{3<{#<{&< {3DE{#MO{&L{#IS{3H {&>{#>{3>";
                vp = "{3<{#<{&< {3DE{#MO{&LI{#SH{3ES {&>{#>{3>";
	}
	else if (dam <= 100)
	{
		vs = "{6={^+{&= {6DI{^SF{&I{^GU{6RE {&={^+{6=";
		vp = "{6={^+{&= {6DI{^SF{&IG{^UR{6ES {&={^+{6=";
	}
	else if (dam <= 125)
	{
		vs = "{8>{7>{&> {8OB{7LI{&TE{7RA{8TE {&<{7<{8<";
		vp = "{8>{7>{&> {8OB{7LI{&TER{7AT{8ES {&<{7<{8<";
	}
	else if (dam <= 150)
	{
		vs = "{&<{^<{6< {&R{^UP{6T{^UR{&E {6>{^>{&>";
		vp = "{&<{^<{6< {&R{^UP{6TU{^RE{&S {6>{^>{&>";
	}
	else if (dam <= 200)
	{
		vs = "{&-{7={#-={3- {&A{7N{#NI{3HI{#LA{7T{&E {3-{#=-{7={&-";
		vp = "{&-{7={#-={3- {&A{7N{#NI{3HIL{#AT{7E{&S {3-{#=-{7={&-";
	}
	else if (dam <= 250)
	{
		vs = "{&+{7={2+={@+ {&B{@U{2TCH{@E{&R {2+{@=+{7={&+";
		vp = "{&+{7={2+={@+ {&B{@UT{2CH{@ER{&S {2+{@=+{7={&+";
	}
	else if (dam <= 300)
	{
		vs = "{&^{7*^{8*^ {&ER{7AD{8IC{7AT{&E {8^*{7^*{&^";
		vp = "{&^{7*^{8*^ {&ER{7AD{8IC{7AT{&ES {8^*{7^*{&^";
	}
	else if (dam <= 350)
	{
		vs = "{1*{!*{&*{!*{1* {1C{!RU{&CI{!FI{1E {1*{!*{&*{!*{1*";
		vp = "{1*{!*{&*{!*{1* {1C{!RU{&CIF{!IE{1S {1*{!*{&*{!*{1*";
	}
	else if (dam <= 400)
	{
		vs = "{2+{@+{&+{@+{2+ {2L{@IQ{&UI{@FI{2E {2+{@+{&+{@+{2+";
		vp = "{2+{@+{&+{@+{2+ {2L{@IQ{&UIF{@IE{2S {2+{@+{&+{@+{2+";
	}
	else if (dam == 420)
	{
		vs = "do {G_{g\\{G|{g/{G_CA{gNN{dA{gBA{dC{gIO{GUS_{g\\{G|{g/{G_{x things to";
		vp = "does {G_{g\\{G|{g/{G_CA{gNN{dA{gBA{dC{gIO{GUS_{g\\{G|{g/{G_{x things to";
	}
	else if (dam <= 500)
	{
		vs = "{3={#-{&={#-{3= {3DE{#CA{&PI{#TA{3TE {3={#-{&={#-{3=";
		vp = "{3={#-{&={#-{3= {3DE{#CA{&PIT{#AT{3ES {3={#-{&={#-{3=";
	}
	else if (dam <= 600)
	{
		vs = "{5*{%-{&*{%-{5* {5V{%AP{&OUR{%IZ{5E {5*{%-{&*{%-{5*";
		vp = "{5*{%-{&*{%-{5* {5VA{%PO{&UR{%IZ{5ES {5*{%-{&*{%-{5*";
	}
	else if (dam == 666)
	{
		vs = "do {R^{D*{r<{RD{rE{dMON{rI{RC{r>{D*{R^{x things to";
		vp = "does {R^{D*{r<{RD{rE{dMON{rI{RC{r>{D*{R^{x things to";
	}
	else if (dam == 777)
	{
		vs = "deal {R*{Y,{R*{Y,{R*{Y,{RL{rUCK{RY SE{YVE{RNS{Y,{R*{Y,{R*{Y,{R*{x to";
		vp = "deals {R*{Y,{R*{Y,{R*{Y,{RL{rUCK{RY SE{YVE{RNS{Y,{R*{Y,{R*{Y,{R*{x to";
	}
	else if (dam <= 900)
	{
		vs = "{6-{^-{&-{^-{6- {6P{^UL{&VER{^IZ{6E {6-{^-{&-{^-{6-";
		vp = "{6-{^-{&-{^-{6- {6PU{^LV{&ER{^IZ{6ES {6-{^-{&-{^-{6-";
	}
	else if (dam <= 1200)
	{
		vs = "{4^{$+{&^{$+{4^ {4S{$E{&V{$E{4R {4^{$+{&^{$+{4^";
		vp = "{4^{$+{&^{$+{4^ {4S{$E{&VE{$R{4S {4^{$+{&^{$+{4^";
	}
	else if (dam <= 1800)
	{
		vs = "{8={7={&={7={8= {8S{7M{&I{7T{8E {8={7={&={7={8=";
		vp = "{8={7={&={7={8= {8S{7M{&IT{7E{8S {8={7={&={7={8=";
	}
	else
	{
		vs = "do {%U{@N{#S{^P{!E{&A{!K{^A{#B{@L{%E things to";
		vp = "{%does {%U{@N{#S{^P{!E{&A{!K{^A{#B{@L{%E things to";
	}



	punct = (dam <= 24) ? '.' : '!';

	if (dt == TYPE_HIT)
	{
          if (ch == victim)
          {
              sprintf (buf1, "{($n %s{( $melf%c{x [%d]", vp, punct, dam);
              sprintf (buf2, "{.You %s{. yourself%c{x [%d]", vs, punct, dam);
          }
          else
          {
              sprintf (buf1, "{($n %s{( $N%c{x [%d]", vp, punct, dam);
              sprintf (buf2, "{.You %s{. $N%c{x [%d]", vs, punct, dam);
              sprintf (buf3, "{)$n %s{) you%c{x [%d]", vp, punct, dam);
          }
	}
	else
	{
		if (dt >= 0 && dt < MAX_SKILL)
			attack = skill_table[dt].noun_damage;
		else if (dt >= TYPE_HIT
				 && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
			attack = attack_table[dt - TYPE_HIT].noun;
		else
		{
			bug ("Dam_message: bad dt %d.", dt);
			dt = TYPE_HIT;
			attack = attack_table[0].name;
		}

		if (immune)
		{
                   if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
                   {
			if (ch == victim)
			{
                                sprintf (buf1, "{($n is unaffected by $s own %s{k.{x", attack);
                                sprintf (buf2, "{.Luckily, you are immune to that.{x");
			}
			else
			{
                                sprintf (buf1, "{($N is unaffected by $n's %s{k!{x", attack);
                                sprintf (buf2, "{.$N is unaffected by your %s{h!{x", attack);
                                sprintf (buf3, "{)$n's %s{i is powerless against you.{x", attack);
			}
                   }
		}
		else
		{
			if (ch == victim)
			{
                                sprintf (buf1, "{($n's %s{( %s{( $N%c{x {3[%d{3]{x", attack, vp, punct, dam);                                sprintf (buf1, "{($n's %s{( %s{( $m%c{x {3[%d{3]{x", attack, vp, punct, dam);
                                sprintf (buf2, "{.Your %s{. %s{. you%c{x {3[%d{3]{x", attack, vp, punct, dam);
			}
			else
			{
                                sprintf (buf1, "{($n's %s{( %s{( $N%c{x {3[%d{3]{x", attack, vp, punct, dam);
                                sprintf (buf2, "{.Your %s{. %s{. $N%c{x {3[%d{3]{x", attack, vp, punct, dam);
                                sprintf (buf3, "{)$n's %s{) %s{) you%c{x {3[%d{3]{x", attack, vp, punct, dam);
			}
		}
	}



        if (ch == victim)
	{
                act (buf1, ch, NULL, NULL, TO_ROOM);
		act (buf2, ch, NULL, NULL, TO_CHAR);
	}
	else
	{
/*	TRYING NONSHORT
	    to = ch->in_room->people;
	    for( ; to ; to = to->next_in_room )
	    {
	        if ( (!IS_NPC(to) && to->desc == NULL )
	        ||   ( IS_NPC(to) && !HAS_TRIGGER(to, TRIG_ACT) ) )
	            continue;

	        if( to == ch || to == victim )
	            continue;

	    } */

            act (buf1, ch, NULL, victim, TO_NONSHORT);

            if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
		act (buf2, ch, NULL, victim, TO_CHAR);

            if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
		act (buf3, ch, NULL, victim, TO_VICT);

	}

   } // End first IF

   if((victim->hit - dam < max_damage) && !IS_SET(victim->act, ACT_NO_BODY))
   {

   // Now.....the fun Death Messages -- Skyntil
      if (dt >= 0 && dt < MAX_SKILL)
       attack = skill_table[dt].noun_damage;
      else if (dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
      {
       attack = attack_table[dt - TYPE_HIT].noun;
       dam_type = attack_table[dt - TYPE_HIT].damage;
      }
      else
      {
         bug ("Dam_message: bad dt %d.", dt);
         dt = TYPE_HIT;
         attack = attack_table[0].name;
      }

    /* Individual 'noun' cases first, then the general dam_types
     * -- Skyntil 4/22/00
     */
	if(!IS_SET(ch->plyr,PLAYER_SUBDUE))
	{
    if(dam_flags & DMF_DECAP)
    {
      act("You swing your blade in a low arc, slicing off $N's head.", ch, NULL, victim, TO_CHAR);
      act("$n swings $s blade in a low arc, slicing off $N's head.", ch, NULL, victim, TO_NOTVICT);
      act("$n swings $s blade in a wide arc, slicing off your head.", ch, NULL, victim, TO_VICT);
      part_create(victim,"head");
    }

    if (attack == "claw")
    {
        damp=number_range(1,3);
	if ( damp == 1 )
	{
	    act("You tear out $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
	    act("$n tears out $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
	    act("$n tears out your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
            part_create(victim,"blood");
	}
	if ( damp == 2 )
	{
	    	act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
	    	act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
                part_create(victim,"eye");
	}
        if( damp == 3)
        {
	    	act("You claw open $N's chest.", ch, NULL, victim, TO_CHAR);
	    	act("$n claws open $N's chest.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n claws open $N's chest.", ch, NULL, victim, TO_VICT);
                part_create(victim,"heart");
        }

    }
    else if ( dt == gsn_whip || dam_flags & DMF_WHIP )
    {
	act("You entangle $N around the neck, and squeeze the life out of $S.", ch, NULL, victim, TO_CHAR);
	act("$n entangle $N around the neck, and squeezes the life out of $S.", ch, NULL, victim, TO_NOTVICT);
	act("$n entangles you around the neck, and squeezes the life out of you.", ch, NULL, victim, TO_VICT);
    }
    else if( dam_type == DAM_SLASH )
    {
        damp = number_range(1,9);
	if ( damp == 1 )
	{
	    act("You swing your blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_CHAR);
	    act("$n swings $s blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_NOTVICT);
	    act("$n swings $s blade in a low arc, rupturing your abdominal cavity.\n\rYour entrails spray out over a wide area.", ch, NULL, victim, TO_VICT);
            part_create(victim,"guts");
	}
	else if ( damp == 2 )
	{
	    act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
	    act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
	    act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 3 )
	{
	    act("Your blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_CHAR);
	    act("$n's blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_NOTVICT);
	    act("$n's blow slices open your carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_VICT);
            part_create(victim,"blood");
	}
	else if ( damp == 4 )
	{
	    act("You swing your blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
	    act("$n swings $s blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
	    act("$n swings $s blade across your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
            part_create(victim,"blood");
	}
	else if ( damp == 5 )
	{
	    	act("You swing your blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_CHAR);
	    	act("$n swings $s blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n swings $s blade down upon your head, splitting it open.\n\rYour brains pour out of your forehead.", ch, NULL, victim, TO_VICT);
                part_create(victim,"brain");
	}
	else if ( damp == 6 )
	{
	    act("You swing your blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_CHAR);
	    act("$n swings $s blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_NOTVICT);
	    act("$n swings $s blade between your legs, nearly splitting you in half.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 7 )
	{
	    	act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
	    	act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
                part_create(victim,"arm");
	}
	else if ( damp == 8 )
	{
	    	act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
	    	act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
                part_create(victim,"leg");
	}
        else if ( damp == 9)
        {
		act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
		act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
		act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
        }
    }
    else if (attack == "bite") // This must go before Pierce
    {
	act("You sink your teeth into $N's throat and tear out $S jugular vein.\n\rYou wipe the blood from your chin with one hand.", ch, NULL, victim, TO_CHAR);
	act("$n sink $s teeth into $N's throat and tears out $S jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_NOTVICT);
	act("$n sink $s teeth into your throat and tears out your jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_VICT);
        part_create(victim,"blood");
    }
    else if ( dam_type == DAM_PIERCE )
    {
        damp = number_range(1,5);
	if ( damp == 1 )
	{
	    act("You defty invert your weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_CHAR);
	    act("$n defty inverts $s weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_NOTVICT);
	    act("$n defty inverts $s weapon and plunge it point first into your chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_VICT);
            part_create(victim,"blood");
	}
	else if ( damp == 2 )
	{
	    act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
	    act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
	    act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 3 )
	{
	    act("You thrust your weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_CHAR);
	    act("$n thrusts $s weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_NOTVICT);
	    act("$n thrusts $s weapon up under your jaw and through your head.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 4 )
	{
	    act("You ram your weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_CHAR);
	    act("$n rams $s weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_NOTVICT);
	    act("$n rams $s weapon through your body, pinning you to the ground.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 5 )
	{
	    act("You stab your weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_CHAR);
	    act("$n stabs $s weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_NOTVICT);
	    act("$n stabs $s weapon into your eye and out the back of your head.", ch, NULL, victim, TO_VICT);
	}
    }
    else if ( dam_type == DAM_BASH && dt != gsn_hand_to_hand)
    {
        damp = number_range(1,4);
	if ( damp == 1)
	{
	    act("Your blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_CHAR);
	    act("$n's blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_NOTVICT);
	    act("$n's blow smashes through your chest, caving in half your ribcage.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 2)
	{
	    act("Your blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_CHAR);
	    act("$n's blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_NOTVICT);
	    act("$n's blow smashes your spine, shattering it in several places.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 3)
	{
	    	act("You swing your weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, leaking out brains.", ch, NULL, victim, TO_CHAR);
	    	act("$n swings $s weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, covering you with brains.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n swings $s weapon down upon your head.\n\rYour head cracks open like an overripe melon, spilling your brains everywhere.", ch, NULL, victim, TO_VICT);
                part_create(victim,"brain");
	}
        else if (damp == 4)
        {
                act("You hammer your weapon into $N's side, crushing bone.", ch, NULL, victim, TO_CHAR);
	    	act("$n hammers $s weapon into $N's side, crushing bone.", ch, NULL, victim, TO_NOTVICT);
	    	act("$n hammers $s weapon into your side, crushing bone.", ch, NULL, victim, TO_VICT);

        }
    }
    else if ( dam_type == DAM_BASH && dt == gsn_hand_to_hand)
    {
        damp = number_range(1,2);
	if ( damp == 1)
	{
	    act("Your blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_CHAR);
	    act("$n's blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_NOTVICT);
	    act("$n's blow smashes through your chest, caving in half your ribcage.", ch, NULL, victim, TO_VICT);
	}
	else if ( damp == 2)
	{
	    act("Your blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_CHAR);
	    act("$n's blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_NOTVICT);
	    act("$n's blow smashes your spine, shattering it in several places.", ch, NULL, victim, TO_VICT);
	}
    }
    else if ( attack == "suction" || dam_type == DAM_NEGATIVE )
    {
	act("You place your weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_CHAR);
	act("$n places $s weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_NOTVICT);
        act("$n places $s weapon on your head and sucks out your brains.", ch, NULL, victim, TO_VICT);
    }
    else
    {
     //   bug( "Dam_message: bad dt %d.", dt );
    }

    // Death message
    msg = "$N hits the ground ... DEAD.";
    act(msg,ch,NULL,victim,TO_NOTVICT);
    }
}
    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
	OBJ_DATA *obj;

	if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
		return;

	if (IS_OBJ_STAT (obj, ITEM_NOREMOVE))
	{
                act ("{?$S weapon won't budge!{x", ch, NULL, victim, TO_CHAR);
                act ("{?$n tries to disarm you, but your weapon won't budge!{x",
			 ch, NULL, victim, TO_VICT);
                act ("{3$n tries to disarm $N, but fails.{x", ch, NULL, victim, TO_NOTVICT);
		return;
	}

        act ("{?$n DISARMS you and sends your weapon flying!{x",
		 ch, NULL, victim, TO_VICT);
        act ("{?You disarm $N{j!{x", ch, NULL, victim, TO_CHAR);
        act ("{($n {(disarms $N{(!{x", ch, NULL, victim, TO_NOTVICT);

	obj_from_char (obj);
	obj_to_char(obj, victim);
/*
	if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
		obj_to_char (obj, victim);
	else
	{
		obj_to_room (obj, victim->in_room);
		if (IS_NPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
			get_obj (victim, obj, NULL);
	}
*/
	return;
}

void
do_berserk (CHAR_DATA * ch, char *argument)
{
	int chance, hp_percent;

	if ((chance = get_skill (ch, gsn_berserk)) == 0
		|| (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK))
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
	{
                send_to_char ("{.You turn {rred{h in the face, but nothing happens.{x\n\r", ch);
		return;
	}

	if (IS_AFFECTED (ch, AFF_BERSERK) || is_affected (ch, gsn_berserk)
		|| is_affected (ch, skill_lookup ("frenzy")))
	{
                send_to_char ("{.You get a little madder.{x\n\r", ch);
		return;
	}

	if (IS_AFFECTED (ch, AFF_CALM))
	{
                send_to_char ("{.You're feeling to mellow to berserk.{x\n\r", ch);
		return;
	}

	if (ch->mana < 50)
	{
                send_to_char ("{.You can't get up enough energy.{x\n\r", ch);
		return;
	}

	/* modifiers */

	/* fighting */
	if (ch->position == POS_FIGHTING)
		chance += 10;

	/* damage -- below 50% of hp helps, above hurts */
	hp_percent = 100 * ch->hit / ch->max_hit;
	chance += 28 - hp_percent / 2;

	if (number_percent () < chance)
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
		ch->mana -= 50;
		ch->move = UMAX(1,ch->move - 50);

		/* heal a little damage */
		ch->hit += ch->level * 2;
		ch->hit = UMIN (ch->hit, ch->max_hit);

                send_to_char ("{.Your pulse races as you are consumed by {rrage!{x\n\r", ch);
                act ("{($n gets a {cw{gi{rl{yd{k look in $s eyes.{x", ch, NULL, NULL, TO_ROOM);
		check_improve (ch, gsn_berserk, TRUE, 2);

		af.where = TO_AFFECTS;
		af.type = gsn_berserk;
		af.level = ch->level;
		af.duration = ch->level/4;
		af.modifier = UMAX (1, ch->level / 4);
		af.bitvector = AFF_BERSERK;

		af.location = APPLY_HITROLL;
		affect_to_char (ch, &af);

		af.location = APPLY_DAMROLL;
		affect_to_char (ch, &af);

		af.modifier = UMAX (10, 10 * (ch->level / 20));
		af.location = APPLY_AC;
		affect_to_char (ch, &af);
	}

	else
	{
		WAIT_STATE (ch,PULSE_VIOLENCE);
		ch->mana -= 25;
		ch->move = UMAX(1,ch->move-75);

                send_to_char ("{.Your pulse speeds up, but nothing happens.{x\n\r", ch);
		check_improve (ch, gsn_berserk, FALSE, 2);
	}
}

void
do_voodoo (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *doll;

	if (IS_NPC (ch))
		return;

	doll = get_eq_char (ch, WEAR_HOLD);
	if (doll == NULL || (doll->pIndexData->vnum != OBJ_VNUM_VOODOO))
	{
		send_to_char ("You are not holding a voodoo doll.\n\r", ch);
		return;
	}

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Syntax: voodoo <action>\n\r", ch);
		send_to_char ("Actions: pin trip throw\n\r", ch);
		return;
	}

	if (!str_cmp (arg, "pin"))
	{
		do_vdpi (ch, doll->name);
		return;
	}

	if (!str_cmp (arg, "trip"))
	{
		do_vdtr (ch, doll->name);
		return;
	}

	if (!str_cmp (arg, "throw"))
	{
		do_vdth (ch, doll->name);
		return;
	}

	do_voodoo (ch, "");
}

void
do_vdpi (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	bool found = FALSE;

	argument = one_argument (argument, arg1);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch))
			continue;

		if (!str_cmp (arg1, wch->name) && !found)
		{
			if (IS_NPC (wch))
				continue;

			if (IS_IMMORTAL (wch) && (wch->level > ch->level))
			{
				send_to_char ("That's not a good idea.\n\r", ch);
				return;
			}

			if ((wch->level < 20) && !IS_IMMORTAL (ch))
			{
				send_to_char ("They are a little too young for that.\n\r", ch);
				return;
			}

			if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
			{
				send_to_char ("They are still reeling from a previous voodoo.\n\r", ch);
				return;
			}

			found = TRUE;

			send_to_char ("You stick a pin into your voodoo doll.\n\r", ch);
			act ("$n sticks a pin into a voodoo doll.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("{RYou double over with a sudden pain in your gut!{x\n\r", wch);
			act ("$n suddenly doubles over with a look of extreme pain!", wch, NULL, NULL, TO_ROOM);
			af.where = TO_SHIELDS;
			af.type = skill_lookup ("protection voodoo");
			af.level = wch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = SHD_PROTECT_VOODOO;
			affect_to_char (wch, &af);
			return;
		}
	}
	send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
	return;
}

void
do_vdtr (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	bool found = FALSE;

	argument = one_argument (argument, arg1);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch))
			continue;

		if (!str_cmp (arg1, wch->name) && !found)
		{
			if (IS_NPC (wch))
				continue;

			if (IS_IMMORTAL (wch) && (wch->level > ch->level))
			{
				send_to_char ("That's not a good idea.\n\r", ch);
				return;
			}

			if ((wch->level < 20) && !IS_IMMORTAL (ch))
			{
				send_to_char ("They are a little too young for that.\n\r", ch);
				return;
			}

			if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
			{
				send_to_char ("They are still reeling from a previous voodoo.\n\r", ch);
				return;
			}

			found = TRUE;

			send_to_char ("You slam your voodoo doll against the ground.\n\r", ch);
			act ("$n slams a voodoo doll against the ground.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("{RYour feet slide out from under you!{x\n\r", wch);
			send_to_char ("{RYou hit the ground face first!{x\n\r", wch);
			act ("$n trips over $s own feet, and does a nose dive into the ground!", wch, NULL, NULL, TO_ROOM);
			af.where = TO_SHIELDS;
			af.type = skill_lookup ("protection voodoo");
			af.level = wch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = SHD_PROTECT_VOODOO;
			affect_to_char (wch, &af);
			return;
		}
	}
	send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
	return;
}

void
do_vdth (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	AFFECT_DATA af;
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	bool found = FALSE;
	int attempt;

	argument = one_argument (argument, arg1);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch))
			continue;

		if (!str_cmp (arg1, wch->name) && !found)
		{
			if (IS_NPC (wch))
				continue;

			if (IS_IMMORTAL (wch) && (wch->level > ch->level))
			{
				send_to_char ("That's not a good idea.\n\r", ch);
				return;
			}

			if ((wch->level < 20) && !IS_IMMORTAL (ch))
			{
				send_to_char ("They are a little too young for that.\n\r", ch);
				return;
			}

			if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
			{
				send_to_char ("They are still reeling from a previous voodoo.\n\r", ch);
				return;
			}

			found = TRUE;

			send_to_char ("You toss your voodoo doll into the air.\n\r", ch);
			act ("$n tosses a voodoo doll into the air.", ch, NULL, NULL, TO_ROOM);
			af.where = TO_SHIELDS;
			af.type = skill_lookup ("protection voodoo");
			af.level = wch->level;
			af.duration = 1;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = SHD_PROTECT_VOODOO;
			affect_to_char (wch, &af);
			if ((wch->fighting != NULL) || (number_percent () < 25))
			{
				send_to_char ("{RA sudden gust of wind throws you through the air!{x\n\r", wch);
				send_to_char ("{RYou slam face first into the nearest wall!{x\n\r", wch);
				act ("A sudden gust of wind picks up $n and throws $m into a wall!", wch, NULL, NULL, TO_ROOM);
				return;
			}
			wch->position = POS_STANDING;
			was_in = wch->in_room;
			for (attempt = 0; attempt < 6; attempt++)
			{
				EXIT_DATA *pexit;
				int door;

				door = number_door ();
				if ((pexit = was_in->exit[door]) == 0
					|| pexit->u1.to_room == NULL
					|| IS_SET (pexit->exit_info, EX_CLOSED)
					|| (IS_NPC (wch)
					&& IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
					continue;

				move_char (wch, door, FALSE, TRUE);
				if ((now_in = wch->in_room) == was_in)
					continue;

				wch->in_room = was_in;
				sprintf (buf, "A sudden gust of wind picks up $n and throws $m to the %s.", dir_name[door]);
				act (buf, wch, NULL, NULL, TO_ROOM);
				send_to_char ("{RA sudden gust of wind throws you through the air!{x\n\r", wch);
				wch->in_room = now_in;
				act ("$n sails into the room and slams face first into a wall!", wch, NULL, NULL, TO_ROOM);
				do_look (wch, "auto");
				send_to_char ("{RYou slam face first into the nearest wall!{x\n\r", wch);
				return;
			}
			send_to_char ("{RA sudden gust of wind throws you through the air!{x\n\r", wch);
			send_to_char ("{RYou slam face first into the nearest wall!{x\n\r", wch);
			act ("A sudden gust of wind picks up $n and throws $m into a wall!", wch, NULL, NULL, TO_ROOM);
			return;
		}
	}
	send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
	return;
}

void
do_bash (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	//COOLDOWN_DATA cd;

	one_argument (argument, arg);

	if ((chance = get_skill (ch, gsn_bash)) == 0
		|| (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BASH))
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_bash].skill_level[ch->class]))
	{
		send_to_char ("Bashing? What's that?\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			send_to_char ("But you aren't fighting anyone!\n\r", ch);
			return;
		}
	}

	else if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (victim->position < POS_FIGHTING)
	{
		act ("You'll have to let $M get back up first.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("You try to bash your brains out, but fail.\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You get a running start, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n gets a running start, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

        if (ch->stunned > 0)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if (!can_see (ch, victim))
	{
		send_to_char ("You get a running start, and slam right into a wall.\n\r", ch);
		return;
	}

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_bash))
	{
                send_to_char ("Bash is still on cooldown.\n\r", ch);
                return;
       }


	*Apply cooldown	
	cd.type      = skill_lookup("bash");
	cd.duration	 = skill_table[gsn_bash].cooldown;
        cooldown_on( ch, &cd );*/

	/* modifiers */

	/* size  and weight */
	chance += ch->carry_weight / 250;
	chance -= victim->carry_weight / 200;

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 10;
	else
		chance += (ch->size - victim->size) * 10;


	/* stats */
	chance += get_curr_stat (ch, STAT_STR);
	chance -= (get_curr_stat (victim, STAT_DEX) * 2);
	chance -= GET_AC (victim, AC_BASH) / 5;
	/* speed */
	if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
		chance += 10;
	if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
		chance -= 30;

	/* level */

	if (!IS_NPC (victim))
		chance += (ch->level - victim->level);
	else
		chance += (ch->level - victim->level)/5;


	if (!IS_NPC (victim)
		&& chance < get_skill (victim, gsn_dodge))
        {
          chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
        }

        if (check_tumble(ch,victim,gsn_bash))
        {
        check_improve(ch,gsn_bash,TRUE,2);
        act("$N rolls free of $n's bash.",ch,0,victim,TO_NOTVICT);
        act("You roll out of $n's bash attempt.",ch,0,victim,TO_VICT);
        act("$N rolls free of your bash attempt.",ch,0,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        if (victim->fighting == NULL)
        {
        ch->fighting = victim;
        victim->fighting = ch;
    	}
        return;
        }

        if(victim->class == CLASS_FADE)
        {
         if(number_percent() < 35)
         {
          if(check_phase_real(ch,victim,TRUE))
          {
           act("{)$n tries to bash you, but you phase out of the attack.{x",ch,NULL,victim,TO_VICT);
           act("{.$N phases out, and you bash at thin air!{x",ch,NULL,victim,TO_CHAR);
           WAIT_STATE(ch,skill_table[gsn_bash].beats);
           if (victim->fighting == NULL)
           {
           ch->fighting = victim;
           victim->fighting = ch;
   		   }
           return;
          }
         }
        }
 if(victim->class == CLASS_DARKPALADIN)//change this to blood magus
        {
         if(number_percent() < 39)
         {
          if(check_blood(ch,victim,TRUE))
          {
act ("{)You phase transparent to{Rred{x and absorb $n's bash{x", ch, NULL, victim, TO_VICT);
       act ("{.$N phases transparent then {Rred{x and absorbs your bash{x", ch, NULL, victim, TO_CHAR);
           WAIT_STATE(ch,skill_table[gsn_bash].beats);
           if (victim->fighting == NULL)
           {
           ch->fighting = victim;
           victim->fighting = ch;
                   }
           return;
          }
         }
        }



 	/* now the attack */
	if (number_percent () < chance)
	{

                act ("{)$n sends you sprawling with a powerful bash!{x",
			 ch, NULL, victim, TO_VICT);
                act ("{.You slam into $N, and send $M flying!{x", ch, NULL, victim, TO_CHAR);
                act ("{($n sends $N sprawling with a powerful bash.{x",
			 ch, NULL, victim, TO_NOTVICT);
		check_improve (ch, gsn_bash, TRUE, 1);

		DAZE_STATE (victim, PULSE_VIOLENCE);
		WAIT_STATE (ch, skill_table[gsn_bash].beats);
                damage (ch, victim, number_range (2, 2 + 2 * ch->size + chance / 20),
                        gsn_bash,DAM_BASH, FALSE, 0);
                victim->position = POS_RESTING;
		chance = (get_skill (ch, gsn_stun) / 5);
		if (number_percent () < chance)
		{
                        victim->stunned = 2;
                        act ("{)You are stunned, and have trouble getting back up!{x",
				 ch, NULL, victim, TO_VICT);
                        act ("{.$N is stunned by your bash!{x", ch, NULL, victim, TO_CHAR);
                        act ("{($N is having trouble getting back up.{x",
				 ch, NULL, victim, TO_NOTVICT);
			check_improve (ch, gsn_stun, TRUE, 1);
		}
	}
	else
	{
		damage (ch, victim, 0, gsn_bash, DAM_BASH, FALSE, 0);
                act ("{.You fall flat on your face!{x",
			 ch, NULL, victim, TO_CHAR);
                act ("{($n falls flat on $s face.{x",
			 ch, NULL, victim, TO_NOTVICT);
                act ("{)You evade $n's bash, causing $m to fall flat on $s face.{x",
			 ch, NULL, victim, TO_VICT);
		check_improve (ch, gsn_bash, FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
	}
}

void do_fear (CHAR_DATA * ch, CHAR_DATA * victim) {
	int chance;

	if ((chance = get_skill (ch, gsn_fear)) < 1 
		|| IS_NPC(ch) || (!IS_NPC(ch) && !has_skill(ch,gsn_fear)))
	{
		return;
	}

	 if (IS_AFFECTED (victim, AFF_FEAR))  {
		return; }
		
	if (IS_AFFECTED (victim, skill_lookup("courage")))   {
		return; }
	
	//if (is_safe(ch,victim)) return;

	/* if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
		return; }
	*/
	
	if (ch->stunned) {
		send_to_char("You're still a bit woozy.\n\r",ch);
		return; }
	
	if (number_percent() < 95)
		return;

	/* modifiers */
	/* first a lowering of chance from best of 100 */
	//chance = chance * 4/5;

	/* size */
	chance += (ch->size-victim->size) * 7;
	
	/* stats */
	chance -= get_curr_stat(victim,STAT_WIS)*2;
	chance += get_curr_stat(ch,STAT_WIS)+get_curr_stat(ch,STAT_CON);
	
	/* berserk! */
	if(IS_AFFECTED(ch,AFF_BERSERK))
		chance += 10;
	if(IS_AFFECTED(victim,AFF_BERSERK))
		chance -= 15;

	/* level */
	chance += (ch->level - victim->level)*2;

	if (chance % 5 == 0)
		chance += 1;

	if (chance < 0) {
		send_to_char ("{RYou cry in terror realizing how little you really are.{x\n\r",ch);
		victim = ch; }
	
	if (number_percent() < chance) {
		AFFECT_DATA af;
		//act("You turn your gaze on $q glaring in strength.{x",ch,NULL,victim,TO_CHAR);
		act("$n begins to shake, quaking in fear.{x",victim,NULL,NULL,TO_ROOM);
		act("$n turns his powerful gaze on you and you begin to shake in fear.{x",ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_fear, TRUE,2);

		af.where = TO_AFFECTS;
		af.type = gsn_fear;
		af.level = ch->level;
		af.duration = number_range(1,3);
		af.location = APPLY_WIS;
		af.modifier = -1*abs(get_curr_stat(ch,STAT_WIS)-get_curr_stat(victim,STAT_WIS))/2;
		af.bitvector = AFF_FEAR;

		affect_to_char (victim, &af);
	}
	else {
		//act("You turn your gaze on $q attempting to glare.{x",ch,NULL,victim,TO_CHAR);
		act("$n frowns and shakes $s head.{x",victim,NULL,NULL,TO_ROOM);
		act("$n turns his gaze on you and you glare back.{x",ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_fear,TRUE,2);
	}
	return;
}

void do_dirt (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	//COOLDOWN_DATA cd;

	one_argument (argument, arg);

	if ((chance = get_skill (ch, gsn_dirt)) == 0
		|| (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK_DIRT))
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
	{
                send_to_char ("{.You get your feet dirty.{x\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			send_to_char ("But you aren't in combat!\n\r", ch);
			return;
		}
	}

	else if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (IS_AFFECTED (victim, AFF_BLIND))
	{
                act ("{.$E's already been blinded.{x", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("Very funny.\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You go to kick dirt, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to kick dirt, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_dirt))
	{
                send_to_char ("Dirt kicking is still on cooldown.\n\r", ch);
                return;
       }


	Apply cooldown	
	cd.type      = skill_lookup("dirt");
	cd.duration	 = skill_table[gsn_dirt].cooldown;
        cooldown_on( ch, &cd );

	modifiers */

	/* size */
	if (ch->size < victim->size)
		chance -= (ch->size - victim->size) * 5;	/* smaller = harder to dirt */

	/* dexterity */
	chance += get_curr_stat (ch, STAT_DEX);

	if (!IS_NPC (victim))
		chance -= get_curr_stat (victim, STAT_DEX);
	else
		chance -= get_curr_stat (victim, STAT_DEX) / 5;

	/* speed  */
	if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
		chance += 10;
	if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
		chance -= 25;

	/* level */
	if (IS_NPC (victim))
                chance += abs(ch->level - victim->level) * 2;
	else
		chance += (ch->level - victim->level) * 2;

	/* sloppy hack to prevent false zeroes */
	if (chance % 5 == 0)
		chance += 1;

	/* terrain */

	switch (ch->in_room->sector_type)
	{
	case (SECT_INSIDE):
		chance -= 20;
		break;
	case (SECT_CITY):
		chance -= 10;
		break;
	case (SECT_FIELD):
		chance += 5;
		break;
	case (SECT_FOREST):
		break;
	case (SECT_HILLS):
		break;
	case (SECT_MOUNTAIN):
		chance -= 10;
		break;
	case (SECT_WATER_SWIM):
		chance = 0;
		break;
	case (SECT_WATER_NOSWIM):
		chance = 0;
		break;
	case (SECT_AIR):
		chance = 0;
		break;
	case (SECT_DESERT):
		chance += 20;
		break;
	}

	if (chance == 0)
	{
                send_to_char ("{.There isn't any dirt to kick.{x\n\r", ch);
		return;
	}

	/* now the attack */
	if (number_percent () < chance)
	{
		AFFECT_DATA af;
                act ("{($n is blinded by the dirt in $s eyes!{x", victim, NULL, NULL, TO_ROOM);
                act ("{)$n kicks dirt in your eyes!{x", ch, NULL, victim, TO_VICT);
		damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, FALSE, 0);
		send_to_char ("{DYou can't see a thing!{x\n\r", victim);
		check_improve (ch, gsn_dirt, TRUE, 2);
		WAIT_STATE (ch, skill_table[gsn_dirt].beats);

		af.where = TO_AFFECTS;
		af.type = gsn_dirt;
		af.level = ch->level;
		af.duration = number_range(0,1);
		af.location = APPLY_HITROLL;
		af.modifier = -4;
		af.bitvector = AFF_BLIND;

		affect_to_char (victim, &af);
	}
	else
	{
		damage (ch, victim, 0, gsn_dirt, DAM_NONE, TRUE, 0);
		check_improve (ch, gsn_dirt, FALSE, 2);
		WAIT_STATE (ch, skill_table[gsn_dirt].beats);
	}
}

void
do_gouge (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	//COOLDOWN_DATA cd;

	one_argument (argument, arg);

	if ((chance = get_skill (ch, gsn_gouge)) == 0
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_gouge].skill_level[ch->class]))
	{
		send_to_char ("Gouge?  What's that?{x\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			send_to_char ("But you aren't in combat!\n\r", ch);
			return;
		}
	}

	else if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (IS_AFFECTED (victim, AFF_BLIND))
	{
                act ("{.$E's already been blinded.{x", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("Very funny.\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	
	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You go to gouge, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to gouge, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_gouge))
	{
                send_to_char ("Gouge is still on cooldown.\n\r", ch);
                return;
       }


	Apply cooldown	
	cd.type      = skill_lookup("gouge");
	cd.duration	 = skill_table[gsn_gouge].cooldown;
        cooldown_on( ch, &cd );

	 modifiers */

	/* size */
	if (ch->size < victim->size)
		chance -= (ch->size - victim->size) * 5;	/* smaller = harder to gouge */


	/* dexterity */
	chance += get_curr_stat (ch, STAT_DEX);

	if (!IS_NPC (victim))
		chance -= get_curr_stat (victim, STAT_DEX);
	else
		chance -= get_curr_stat (victim, STAT_DEX) / 5;

	/* speed  */
	if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
		chance += 10;
	if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
		chance -= 25;

	/* level */
	if (IS_NPC (victim))
                chance += abs(ch->level - victim->level) * 2;
	else
		chance += (ch->level - victim->level) * 2;

	/* sloppy hack to prevent false zeroes */
	if (chance % 5 == 0)
		chance += 1;

	/* now the attack */
	if (number_percent () < chance)
	{
		AFFECT_DATA af;
                act ("{($n is blinded by a poke in the eyes!{x", victim, NULL, NULL, TO_ROOM);
                act ("{)$n gouges at your eyes!{x", ch, NULL, victim, TO_VICT);
		damage (ch, victim, number_range (2, 5), gsn_gouge, DAM_NONE, FALSE, 0);
		send_to_char ("{DYou see nothing but stars!{x\n\r", victim);
		check_improve (ch, gsn_gouge, TRUE, 2);
		WAIT_STATE (ch, skill_table[gsn_gouge].beats);

		af.where = TO_AFFECTS;
		af.type = gsn_gouge;
		af.level = ch->level;
		af.duration = 0;
		af.location = APPLY_HITROLL;
		af.modifier = -4;
		af.bitvector = AFF_BLIND;

		affect_to_char (victim, &af);
	}
	else
	{
		damage (ch, victim, 0, gsn_gouge, DAM_NONE, TRUE, 0);
		check_improve (ch, gsn_gouge, FALSE, 2);
		WAIT_STATE (ch, skill_table[gsn_gouge].beats);
	}
}

void
do_trip (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument (argument, arg);

	if ((chance = get_skill (ch, gsn_trip)) == 0
		|| (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_TRIP))
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_trip].skill_level[ch->class]))
	{
		send_to_char ("Tripping?  What's that?\n\r", ch);
		return;
	}


	if (arg[0] == '\0')
	{
		victim = ch->fighting;
		if (victim == NULL)
		{
			send_to_char ("But you aren't fighting anyone!\n\r", ch);
			return;
		}
	}

	else if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	
	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 10)) {
		act("You go to trip them, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to land a trip, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if (IS_AFFECTED (victim, AFF_FLYING))
	{
                act ("{.$S feet aren't on the ground.{x", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->position < POS_FIGHTING)
	{
                act ("{.$N is already down.{c", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
                send_to_char ("{.You fall flat on your face!{x\n\r", ch);
		WAIT_STATE (ch, 2 * skill_table[gsn_trip].beats);
                act ("{($n trips over $s own feet!{x", ch, NULL, NULL, TO_ROOM);
		return;
	}

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

	/* modifiers */

	/* size */
	if (ch->size < victim->size)
		chance -= (ch->size - victim->size) * 10;	/* smaller = harder to trip */

	/* dex */
	chance += get_curr_stat (ch, STAT_DEX);
	chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;

	/* speed */
	if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
		chance += 10;
	if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;


	/* now the attack */
	if (number_percent () < chance)
	{

         if (check_tumble(ch,victim,gsn_trip))
         {
         check_improve(ch,gsn_trip,TRUE,2);
         act("$N rolls free of $n's trip.",ch,0,victim,TO_NOTVICT);
         act("You roll out of $n's trip attempt.",ch,0,victim,TO_VICT);
         act("$N rolls free of your trip attempt.",ch,0,victim,TO_CHAR);
         WAIT_STATE(ch,skill_table[gsn_trip].beats);
         return;
         }

                act ("{)$n trips you and you go down!{x", ch, NULL, victim, TO_VICT);
                act ("{.You trip $N and $N goes down!{x", ch, NULL, victim, TO_CHAR);
                act ("{($n trips $N, sending $M to the ground.{x", ch, NULL, victim, TO_NOTVICT);
		check_improve (ch, gsn_trip, TRUE, 1);

		DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
		WAIT_STATE (ch, skill_table[gsn_trip].beats);
		victim->position = POS_RESTING;
		damage (ch, victim, number_range (2, 2 + 2 * victim->size), gsn_trip,
				DAM_BASH, TRUE, 0);
	}
	else
	{
		damage (ch, victim, 0, gsn_trip, DAM_BASH, TRUE, 0);
		WAIT_STATE (ch, skill_table[gsn_trip].beats * 2 / 3);
		check_improve (ch, gsn_trip, FALSE, 1);
	}
}



void do_kill (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Kill whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (victim == ch)
	{
                send_to_char ("{.You hit yourself.  {z{COuch!{x\n\r", ch);
		multi_hit (ch, ch, TYPE_UNDEFINED);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
		act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char ("You do the best you can!\n\r", ch);
		return;
	}

        if (ch->fighting != NULL)
        {
                send_to_char ("You do the best you can!\n\r",ch);
                return;
        }

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

	WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
        if (IS_NPC (ch))
                sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);
        else
                sprintf (buf, "Help!  I am being attacked by %s!", PERS(ch,victim));
        do_yell (victim, buf);
	multi_hit (ch, victim, TYPE_UNDEFINED);

        /*
         * wimpy check
        if (!IS_NPC (victim)
            && (IS_NPC(ch->fighting))
                && victim->hit > 0
                && victim->hit <= victim->wimpy
                && (arena_can_wimpy(victim))
                && victim->wait < PULSE_VIOLENCE / 2)
                do_flee (victim, ""); */

	return;
}

void do_mock (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Mock hit whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}
	if (is_safe_mock (ch, victim))
		return;

	if (victim->fighting != NULL)
	{
		send_to_char ("{gThis player is busy at the moment.{x\n\r", ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char ("{gYou've already got your hands full!{x\n\r", ch);
		return;
	}

	one_hit_mock (ch, victim, TYPE_UNDEFINED, FALSE);

	return;
}


void do_backstab (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
        char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
 AFFECT_DATA af;
	int chance = 0;
	//COOLDOWN_DATA cd;

	one_argument (argument, arg);

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 30)) {
		act("You go to backstab, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n sneaks up to backstab, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if(ch->stunned) {
		send_to_char("Yes, sneak up and backstab while stunned. Good try.\n\r",ch);
		return; }

	if (arg[0] == '\0')
	{
		send_to_char ("Backstab whom?\n\r", ch);
		return;
	}

	if (ch->fighting != NULL)
	{
                send_to_char ("{.You're facing the wrong end.{x\n\r", ch);
		return;
	}

	else if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if (!IS_NPC (ch) &&
	  ((ch->level < skill_table[gsn_backstab].skill_level[ch->class])
	  || (chance = get_skill (ch, gsn_backstab)) == 0))
	{
		send_to_char ("Backstab? What's that?\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
		return;

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
                send_to_char ("{.You need to wield a primary weapon to backstab.{x\n\r", ch);
		return;
	}

        if(IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
        {
         send_to_char("You cannot backstab with a two-handed weapon!\n\r",ch);
         return;
        }

	if (victim->hit < victim->max_hit / 3)
	{
		/* act ("$N is hurt and suspicious ... you can't sneak up.",ch, NULL, victim, TO_CHAR); */
		/* PUT BACK IN -- Digaaz - October 15th 2005*/
		act ("$N slips away from your thrust drawing you out.", ch, NULL, victim, TO_CHAR);
                one_hit(ch, victim, TYPE_UNDEFINED, FALSE);
		return;
	}

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_backstab))
	{
                send_to_char ("Backstab is still on cooldown.\n\r", ch);
                return;
       }


*	Apply cooldown	
	cd.type      = skill_lookup("backstab");
	cd.duration	 = skill_table[gsn_backstab].cooldown;
        cooldown_on( ch, &cd );*/

	if(victim == NULL) return;

	if ((ch->fighting == NULL)
		&& (!IS_NPC (ch))
		&& (!IS_NPC (victim)))
	{
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	}

        if ( ch->level < 102 ) {
		WAIT_STATE (ch, skill_table[gsn_backstab].beats); }

	if (number_percent () < get_skill (ch, gsn_backstab)
                || (get_skill (ch, gsn_backstab) > 75 && !IS_AWAKE (victim)))
	{

                check_improve (ch, gsn_backstab, TRUE, 1);

          	if (!IS_NPC(ch) && !IS_NPC(victim) && victim->fighting == NULL)
          	{
           	 switch(number_range(0,1))
           	 {
            		case (0):
            		sprintf(buf,"Die, %s you backstabbing fool!",PERS(ch,victim));
            		break;
            		case (1):
            		sprintf(buf,"Help! I'm being backstabbed by %s!",PERS(ch,victim));
            		break;
	   	 }
           	 do_yell(victim,buf);
          	}

		if(!IS_NPC(ch) && has_skill(ch,gsn_assassinate))
		{
			chance = get_skill(ch,gsn_assassinate)/5; // 12%
        		//chance -= number_range(0,15); 

        		if (!can_see(victim,ch)) 
                		chance += 10; 
                	
        		if (victim->position == POS_FIGHTING) 
                		chance -= 22; 
        		else if (victim->position == POS_SLEEPING) 
                		chance += 20; 
        		//else chance -= 8; 
 
        		//chance /= 2; 
        		chance = URANGE(1,chance,16); 

			if( number_percent() < 11 && number_percent() < chance + 30 )
			{
				send_to_char("You tiptoe up and successfully {RSLIT{x their throat!\n\r",ch);
				send_to_char("Someone {RSLITS{x your throat!\n\r",victim);
				act("$n {RSLIT{x $N's throat!",ch,NULL,victim,TO_NOTVICT);
       	              //victim->hit /= 3;
				one_hit(ch,victim,gsn_assassinate,FALSE);
				check_improve (ch, gsn_assassinate, TRUE, 2);
				af.where     = TO_AFFECTS;
                            af.type      = gsn_assassinate;
                            af.level     = ch->level;
                            af.bitvector = AFF_REGENERATION;//grim idea
                            af.duration  = 3;
                            af.modifier  = -325;
                            af.location  = APPLY_REGEN;

                            affect_to_char( victim, &af );
                        return;
			}
			else
			{
				
				check_improve (ch, gsn_assassinate, FALSE, 2);
			}
		}
		
		multi_hit (ch, victim, gsn_backstab);
	}
	else
	{
		check_improve (ch, gsn_backstab, FALSE, 1);

          	if (!IS_NPC(ch) && !IS_NPC(victim) && victim->fighting == NULL)
          	{
           	 switch(number_range(0,1))
           	 {
            		case (0):
            		sprintf(buf,"Die, %s you backstabbing fool!",PERS(ch,victim));
            		break;
            		case (1):
            		sprintf(buf,"Help! I'm being backstabbed by %s!",PERS(ch,victim));
            		break;
           	 }
           		do_yell(victim,buf);
          	}
		damage (ch, victim, 0, gsn_backstab, DAM_NONE, TRUE, 0);
	}

	return;
}

void do_circle (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	//COOLDOWN_DATA cd;

	if (get_skill (ch, gsn_circle) == 0
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_circle].skill_level[ch->class]))
	{
		send_to_char ("Circle? What's that?\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
                send_to_char ("You need to wield a primary weapon to quickstrike.\n\r", ch);
		return;
	}

	if (victim->hit < victim->max_hit / 10)
	{
                act ("$N is hurt and suspicious ... you can't land a strike.",
			 ch, NULL, victim, TO_CHAR);
		return;
	}

        if(IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
        {
         send_to_char("You cannot quickstrike with a two-handed weapon!\n\r",ch);
         return;
        }

	
	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 10)) {
		act("You go to circle, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n sneaks up to circle, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
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

	if (ch->move < 35)
	{
                send_to_char ("You're too tired for that.\n\r", ch);
                return;
        }


/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_circle))
	{
                send_to_char ("Circle is still on cooldown.\n\r", ch);
                return;
       }


*	Apply cooldown	
	cd.type      = skill_lookup("circle");
	cd.duration	 = skill_table[gsn_circle].cooldown;
        cooldown_on( ch, &cd );*/


	WAIT_STATE (ch, skill_table[gsn_circle].beats);
	if (number_percent () < get_skill (ch, gsn_circle)
		|| (get_skill (ch, gsn_circle) >= 2 && !IS_AWAKE (victim)))
	{
		check_improve (ch, gsn_circle, TRUE, 1);
                act ("{)$n ducks deftly under your attack and strikes you!{x", ch, NULL, victim, TO_VICT);
                act ("{.You duck deftly and visciously stab $N.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n ducks below an attack and stabs $N.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 35;
		multi_hit (ch, victim, gsn_circle);
	}
	else
	{
		check_improve (ch, gsn_circle, FALSE, 1);
                act ("{)$n tries to duck under your attack but fails.{x", ch, NULL, victim, TO_VICT);
                act ("{.$N foils your attempt to stab $M.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n fails to manuever $s position.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 20;
		damage (ch, victim, 0, gsn_circle, DAM_NONE, TRUE, 0);
	}


	return;
}

void
do_twirl (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
        OBJ_DATA *obj,*wield;
        bool DUAL = FALSE;

        if(ch->class == CLASS_RANGER || ch->class == CLASS_STRIDER
        || ch->class == CLASS_HUNTER )
        {
         woodsman_twirl(ch,argument);
         return;
        }


        if (get_skill (ch, gsn_twirl) == 0
		|| (!IS_NPC (ch)
                        && ch->level < skill_table[gsn_twirl].skill_level[ch->class]))
	{
                send_to_char ("Twirling? What's that?\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
                send_to_char ("You need to wield a primary weapon to twirl.\n\r", ch);
		return;
	}

	if (ch->move < 35)
	{
                send_to_char ("You're too tired for that.\n\r", ch);
                return;
        }

       if ((wield = get_eq_char(ch,WEAR_SECONDARY)) != NULL)
       {
            int chance;

            chance = (get_skill (ch, gsn_dual_wield) / 3) * 2;
            chance += 33;
            if (number_percent () < chance)
            {
                     if (get_skill (ch, gsn_dual_wield) != 0 && (!IS_NPC (ch)
                         && ch->level >= skill_table[gsn_dual_wield].skill_level[ch->class]))
                     {
                        DUAL = TRUE;
                        check_improve (ch, gsn_dual_wield, TRUE, 1);
                     }
             }

       }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 20)) {
		act("You go to twirl, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to twirl, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
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

        WAIT_STATE (ch, skill_table[gsn_twirl].beats + 6);
        if (number_percent () < get_skill (ch, gsn_twirl)
                || (get_skill (ch, gsn_twirl) >= 2 && !IS_AWAKE (victim)))
	{
                check_improve (ch, gsn_twirl, TRUE, 1);

                act ("{)$n twirls $s blades and strikes you multiple times!{x", ch, NULL, victim, TO_VICT);
                act ("{.You twirl your blades and strike $N!{x", ch, NULL, victim, TO_CHAR);
                act ("{($n twirls $s blades like a true master.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 35;

                if(ch->pcdata->tier < 2)
                {
                 one_hit (ch, victim, gsn_twirl, FALSE);
                 if(DUAL)
                  one_hit (ch, victim, gsn_twirl, TRUE);
                }
                else if( ch->pcdata->tier > 1 )
                {
                 one_hit (ch, victim, gsn_twirl, FALSE);
                 one_hit (ch, victim, gsn_twirl, FALSE);
                 if(DUAL)
                 {
                  one_hit (ch, victim, gsn_twirl, TRUE);
                 }
                }
	}
	else
	{
                check_improve (ch, gsn_twirl, FALSE, 1);

                act ("{)$n gets tangled up twirling $s blades.{x", ch, NULL, victim, TO_VICT);
                act ("{.You start to twirl your blades but lose control.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n gets tangled up twirling $s blades.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 20;
                damage (ch, victim, 0, gsn_twirl, DAM_NONE, TRUE, 0);
	}

	return;
}

void woodsman_twirl (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
        OBJ_DATA *obj,*wield;
        bool DUAL = FALSE;
        bool BONUS = FALSE;

        if (get_skill (ch, gsn_twirl) == 0
		|| (!IS_NPC (ch)
                        && ch->level < skill_table[gsn_twirl].skill_level[ch->class]))
	{
                send_to_char ("Twirling? What's that?\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
                send_to_char ("You need to wield a primary weapon to twirl.\n\r", ch);
		return;
	}

        if( obj->value[0] == WEAPON_AXE || obj->value[0] == WEAPON_SPEAR
        || obj->value[0] == WEAPON_POLEARM)
        BONUS = TRUE;

       if ((wield = get_eq_char(ch,WEAR_SECONDARY)) != NULL)
       {
            int chance;

            chance = (get_skill (ch, gsn_dual_wield) / 3) * 2;
            chance += 33;
            if (number_percent () < chance)
            {
                     if (get_skill (ch, gsn_dual_wield) != 0 && (!IS_NPC (ch)
                         && ch->level >= skill_table[gsn_dual_wield].skill_level[ch->class]))
                     {
                        DUAL = TRUE;
                        check_improve (ch, gsn_dual_wield, TRUE, 1);

                if( wield->value[0] == WEAPON_AXE || wield->value[0] == WEAPON_SPEAR
                || wield->value[0] == WEAPON_POLEARM)
                BONUS = TRUE;

                     }
             }

       }
	
	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 20)) {
		act("You go to twirl, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to twirl, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

        if (ch->move < 35)
	{
		send_to_char ("You're too tired for that.\n\r", ch);
		return;
	}

	if (!can_see (ch, victim))
	{
		send_to_char ("You stumble blindly into a wall.\n\r", ch);
		return;
	}

        WAIT_STATE (ch, skill_table[gsn_twirl].beats + 6);
        if (number_percent () < get_skill (ch, gsn_twirl)
                || (get_skill (ch, gsn_twirl) >= 2 && !IS_AWAKE (victim)))
	{
                check_improve (ch, gsn_twirl, TRUE, 1);

		ch->move -= 35;
                act ("{)$n twirls $s weapon and brings it down on you!{x", ch, NULL, victim, TO_VICT);
                act ("{.You twirl your weapons and strike $N!{x", ch, NULL, victim, TO_CHAR);
                act ("{)$n twirls $s weapon like a master woodsman.{x", ch, NULL, victim, TO_NOTVICT);

                if(ch->pcdata->tier < 2)
                {
                 one_hit (ch, victim, gsn_twirl, FALSE);

                 if(BONUS)
                 {
                  act("Your strike brings $N to $S knees!",ch,NULL,victim, TO_CHAR);
                  act("$n's spin brings $N to $S knees.",ch,NULL,victim,TO_ROOM);
                  one_hit (ch, victim, gsn_twirl, FALSE);
                  WAIT_STATE(victim,12);
                 }

                 if(DUAL)
                  one_hit (ch, victim, gsn_twirl, TRUE);
                }
                else if( ch->pcdata->tier > 1 )
                {
                 one_hit (ch, victim, gsn_twirl, FALSE);
                 one_hit (ch, victim, gsn_twirl, FALSE);

                 if(BONUS)
                 {
                  act("Your strike brings $N to $S knees!",ch,NULL,victim, TO_CHAR);
                  act("$n's spin brings $N to $S knees.",ch,NULL,victim,TO_ROOM);
                  one_hit (ch, victim, gsn_twirl, FALSE);
                  WAIT_STATE(victim,18);
                 }

                 if(DUAL)
                 {
                  one_hit (ch, victim, gsn_twirl, TRUE);
                 }
                }
	}
	else
	{
                check_improve (ch, gsn_twirl, FALSE, 1);

                act ("{)$n gets tangled up twirling $s weapon.{x", ch, NULL, victim, TO_VICT);
                act ("{.You start to twirl your weapons but lose control.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n gets tangled up twirling $s weapon.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 20;
                damage (ch, victim, 0, gsn_twirl, DAM_NONE, TRUE, 0);
	}

	return;
}



void do_feed (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	//COOLDOWN_DATA cd;
	int dam;

	if (get_skill (ch, gsn_feed) == 0
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_feed].skill_level[ch->class]))
	{
		send_to_char ("Feed? What's that?\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (victim->hit < victim->max_hit / 10)
	{
		act ("$N is hurt and suspicious ... you can't get close enough.",
			 ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 20)) {
		act("You go to feed, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to feed, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}


/*	Cooldown check
	if (ch->fighting != NULL && on_cooldown(ch, gsn_feed))
	{
                send_to_char ("Feed is still on cooldown.\n\r", ch);
                return;
       }


	*Apply cooldown
	cd.type      = skill_lookup("feed");
	cd.duration	 = skill_table[gsn_feed].cooldown;
        cooldown_on( ch, &cd );*/

	WAIT_STATE (ch, skill_table[gsn_feed].beats);
	if (number_percent () < get_skill (ch, gsn_feed)
		|| (get_skill (ch, gsn_feed) >= 2 && !IS_AWAKE (victim)))
	{
		check_improve (ch, gsn_feed, TRUE, 1);
                act ("{)$n bites you.{x", ch, NULL, victim, TO_VICT);
                act ("{.You bite $N.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n bites $N.{x", ch, NULL, victim, TO_NOTVICT);
		dam = number_range ( ((ch->level) + (victim->level)) * 8,
		((ch->level) + (victim->level)) * 14 );
		damage (ch, victim, dam, gsn_feed, DAM_NEGATIVE, TRUE, 0);
		if (number_percent () > get_curr_stat(victim,STAT_CON))//HP Drain added by Fesdor
		{
			int drain = (30-get_curr_stat(victim,STAT_CON))*dam/110;//Based on victim's con and damage-based multiplier of around 10.  This could get kinda big.
			if (victim->alignment > 200)//More vs good align
			{
				ch->hit += drain*3/2;
			}
			else
			{
				ch->hit += drain;
			}
		}
	}
	else
	{
		check_improve (ch, gsn_feed, FALSE, 1);
                act ("{)$n tries to bite you, but hits only air.{x", ch, NULL, victim, TO_VICT);
                act ("{.You chomp a mouthfull of air.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n tries to bite $N.{x", ch, NULL, victim, TO_NOTVICT);
		damage (ch, victim, 0, gsn_feed, DAM_NEGATIVE, TRUE, 0);
	}

	/*
	 * wimpy check
	if (!IS_NPC (victim)
             && IS_NPC(ch->fighting)
		&& victim->hit > 0
		&& victim->hit <= victim->wimpy
		&& (arena_can_wimpy(victim))
		&& victim->wait < PULSE_VIOLENCE / 2)
                do_flee (victim, "");          */

	return;
}


void do_flee (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt,clan;
        AFFECT_DATA *af;
        bool found = 0;

	if ((victim = ch->fighting) == NULL)
	{
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

        /* No flee for Stunned - Skyn - */
        if( ch->stunned > 0 )
        {
         send_to_char("You're stunned. You couldn't flee even if you tried.\n\r",ch);
         return;
        }
 
        for ( af = ch->affected; af != NULL; af= af->next)
        {
            switch(af->location)
            {
                  case APPLY_CRIPPLE_LEG:
                  found = 1;
                  break;
            }
            
            if( found)
            {
               if ( get_curr_stat( victim, STAT_DEX ) + get_curr_stat( victim, STAT_CON ) *2
                    < number_percent() + get_curr_stat( victim, STAT_STR ) )
               {
                  send_to_char("Sucks to be leg crippled! You can't flee!\n\r",ch);
                  return;
               }
            }
        }

        if(is_affected(ch,gsn_entangle) && number_percent() < 75)
        {
         send_to_char("Your feet are entangled by vines! You can't flee!\n\r",ch);
         return;
        }


	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++)
	{
	//	WAIT_STATE( ch, 12 );
                EXIT_DATA *pexit;
		int door;

		door = number_door ();
		if ((pexit = was_in->exit[door]) == 0
			|| pexit->u1.to_room == NULL
			|| IS_SET (pexit->exit_info, EX_CLOSED)
			|| number_range (0, ch->daze) != 0
			|| (IS_NPC (ch)	&& IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
			continue;
		stop_fighting (ch, TRUE);
		move_char (ch, door, FALSE, FALSE);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act ("$n has {Yfled{x!", ch, NULL, NULL, TO_ROOM);
		if (!IS_NPC (ch))
		{
			send_to_char ("{BYou {Yflee{B from combat!{x\n\r", ch);
			if (((ch->class == 2) || (ch->class == 9))
				&& (number_percent () < 3 * (ch->level / 2)))
			{
				send_to_char ("You {Ysnuck away{x safely.\n\r", ch);
			}
			else
			{
				send_to_char ("You lost 25 exp.\n\r", ch);
				gain_exp (ch, -25);
			}
                       
		}
		ch->in_room = now_in;
	    for(clan = 0; clan < MAX_CLAN; clan++)
		      clan_entry_trigger(ch,clan);
		return;
	}
//        WAIT_STATE(ch,18);
	send_to_char ("{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch);
	return;
}

void do_smokebomb (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int attempt;

	if((get_skill(ch, gsn_smokebomb))==0)
	{
		send_to_char("Try fleeing?\n\r",ch);
		return;
	}

	if(ch->stunned > 0) {
		send_to_char("You're stunned. You couldn't throw a smokebomb if you tried.\n\r",ch);
		return; }

        if(ch->fighting == NULL)
        {
         send_to_char("You must be fighting to use this skill.\n\r",ch);
         return;
        }

	if(number_percent()<(get_skill(ch,gsn_smokebomb))/2)
	{
		if ((victim = ch->fighting) == NULL)
		{
			if (ch->position == POS_FIGHTING)
				ch->position = POS_STANDING;
			send_to_char ("You aren't fighting anyone.\n\r", ch);
			return;
		}

                if(IS_SET(ch->in_room->affected_by,ROOM_AFF_SMOKE) )
                {
                 send_to_char("The room is already filled with smoke!\n\r",ch);
                 return;
                }

		was_in = ch->in_room;
		for (attempt = 0; attempt < 6; attempt++)
		{
			EXIT_DATA *pexit;
			int door;

			door = number_door ();
			if ((pexit = was_in->exit[door]) == 0
				|| pexit->u1.to_room == NULL
				|| IS_SET (pexit->exit_info, EX_CLOSED)
				|| number_range (0, ch->daze) != 0
				|| (IS_NPC (ch)
					&& IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
				continue;

			send_to_char( "You throw down a {Dsmokebomb{x.\n\r",ch);
                        act("$n threw down a smokebomb!",ch,NULL,victim,TO_VICT);
                        act("$n throws down a smokebomb and runs.",NULL,NULL,ch,TO_NOTVICT);

                        if ( !saves_spell(ch->level,victim,DAM_OTHER) )
		        {
                                af.where     = TO_ROOM_AFF;
				af.type      = skill_lookup("smokebomb");
				af.level     = ch->level;
                                af.location  = APPLY_NONE;
                                af.modifier  = 0;
                                af.duration  = 1;
                                af.bitvector = ROOM_AFF_SMOKE;
                                affect_to_room( victim->in_room, &af );

                           if( !IS_AFFECTED(victim, AFF_BLIND))
                           {
                                af.where     = TO_AFFECTS;
                                af.location  = APPLY_HITROLL;
				af.modifier  = -1;
                                af.bitvector = AFF_BLIND;
                                affect_to_char( victim, &af );

                                act("$N was blinded by your {Dsmokebomb{x!",ch,NULL,victim, TO_CHAR);
				send_to_char( "You are blinded by a {Dsmokebomb{x!\n\r", victim );
                                act("$N was blinded by the {Dsmoke{x.",ch,NULL,victim,TO_NOTVICT);
                           }
                                check_improve(ch,gsn_smokebomb,TRUE,2);
			}
			stop_fighting(ch,TRUE);
			move_char (ch, door, FALSE, FALSE);
			if ((now_in = ch->in_room) == was_in)
				continue;

			ch->in_room = was_in;
                        act ("$n has {Yfled{x!", NULL, NULL, ch, TO_NOTVICT);
			if (!IS_NPC (ch))
			{
				send_to_char ("{BYou {Yflee{B from combat!{x\n\r", ch);
				if (((ch->class == 2) || (ch->class == (MAX_CLASS / 2) + 1))
					&& (number_percent () < 3 * (ch->level / 2)))
				{
					/* if (IS_NPC (victim) || ch->attacker == FALSE)  --Sorn, no flee zapping crap
																		yeah!  lets encourage no strategy
					{ */
						send_to_char ("You {Ysnuck away{x safely.\n\r", ch);
					/* }
					else
					{
						send_to_char ("You feel something singe your butt on the way out.\n\r", ch);
						act ("$n is nearly {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM);
						ch->hit -= (ch->hit / 8);
					} */
				}
				else
				{
					/* if (!IS_NPC (victim) && ch->attacker == TRUE)
					{
						act ("The {RWrath of $G {YZAPS{x your butt on the way out!\n\r", ch, NULL, NULL, TO_CHAR);
						act ("$n is {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM);
						ch->hit -= (ch->hit / 4);
					} */
				}
			}
			ch->in_room = now_in;
			return;
		}
	}
        check_improve(ch,gsn_smokebomb,FALSE,2);
	send_to_char ("{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch);
	return;
}

/*
void do_firebomb (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int attempt;

        if((get_skill(ch, gsn_firebomb))==0)
	{
		send_to_char("Try fleeing?\n\r",ch);
		return;
	}

        if(number_percent() < get_skill(ch,gsn_firebomb))
	{
		if ((victim = ch->fighting) == NULL)
		{
			if (ch->position == POS_FIGHTING)
				ch->position = POS_STANDING;
			send_to_char ("You aren't fighting anyone.\n\r", ch);
			return;
		}

		was_in = ch->in_room;
		for (attempt = 0; attempt < 6; attempt++)
		{
			EXIT_DATA *pexit;
			int door;

			door = number_door ();
			if ((pexit = was_in->exit[door]) == 0
				|| pexit->u1.to_room == NULL
				|| IS_SET (pexit->exit_info, EX_CLOSED)
				|| number_range (0, ch->daze) != 0
				|| (IS_NPC (ch)
					&& IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
				continue;

                        send_to_char( "You throw down a {rfirebomb{x.\n\r",ch);
                        act("$n threw down a {rfirebomb{x!",ch,NULL,NULL,TO_VICT);
                        act("$n throws down a {rfirebomb{x and runs.",ch,NULL,NULL,TO_NOTVICT);

                        if ( !saves_spell(ch->level,victim,DAM_FIRE)|| !saves_spell(ch->level,victim,DAM_FIRE))
		        {
				af.where     = TO_AFFECTS;
                                af.type      = skill_lookup("fire breath");
				af.level     = ch->level;
				af.location  = APPLY_HITROLL;
				af.modifier  = -4;
                                af.duration  = 0;
				af.bitvector = AFF_BLIND;
				affect_to_char( victim, &af );
                                act("$n is blinded by your {rfirebomb{x!",victim,NULL, NULL, TO_CHAR);
                                send_to_char( "You are blinded by a {rfirebomb{x!\n\r", victim );
                                act("$n is blinded by the {Dfire{x.",victim,NULL,NULL,TO_NOTVICT);
			}

			move_char (ch, door, FALSE, FALSE);
			if ((now_in = ch->in_room) == was_in)
				continue;

			ch->in_room = was_in;
			act ("$n has {Yfled{x!", ch, NULL, NULL, TO_ROOM);
			if (!IS_NPC (ch))
			{
				send_to_char ("{BYou {Yflee{B from combat!{x\n\r", ch);
				if (((ch->class == 2) || (ch->class == (MAX_CLASS / 2) + 1))
					&& (number_percent () < 3 * (ch->level / 2)))
				{
					 if (IS_NPC (victim) || ch->attacker == FALSE) 
					{ 
						send_to_char ("You {Ysnuck away{x safely.\n\r", ch);
					 }
					else
					{
						send_to_char ("You feel something singe your butt on the way out.\n\r", ch);
						act ("$n is nearly {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM);
						ch->hit -= (ch->hit / 8);
					}
				}
				else
				{
					if (!IS_NPC (victim) && ch->attacker == TRUE)
					{
						act ("The {RWrath of $G {YZAPS{x your butt on the way out!\n\r", ch, NULL, NULL, TO_CHAR);
						act ("$n is {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM);
						ch->hit -= (ch->hit / 4);
					}
				}
			}
			ch->in_room = now_in;
			stop_fighting (ch, TRUE);
			return;
		}
	}

	send_to_char ("{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch);
	return;
}
*/

void do_rescue (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Rescue whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("What about {Yfleeing{x instead?\n\r", ch);
		return;
	}

        if (!IS_NPC (ch) && IS_NPC (victim))
	{
		send_to_char ("Doesn't need your help!\n\r", ch);
		return;
	}

	if (ch->fighting == victim)
	{
		send_to_char ("Too late.\n\r", ch);
		return;
	}

	if ((fch = victim->fighting) == NULL)
	{
		send_to_char ("That person is not fighting right now.\n\r", ch);
		return;
	}

/*        if (IS_NPC (fch) && !is_same_group (ch, victim))
	{
		send_to_char ("Kill stealing is not permitted.\n\r", ch);
		return;
        } */

	if (is_safe(ch, victim->fighting))
	{
		send_to_char ("Illegal Target.\n\r",ch);
		return;
	}

	WAIT_STATE (ch, skill_table[gsn_rescue].beats);

        if (number_percent () > get_skill (ch, gsn_rescue))
	{
		send_to_char ("You fail the rescue.\n\r", ch);
		check_improve (ch, gsn_rescue, FALSE, 1);
		return;
	}

	act ("{yYou rescue $N!{x", ch, NULL, victim, TO_CHAR);
	act ("{y$n rescues you!{x", ch, NULL, victim, TO_VICT);
	act ("{y$n rescues $N!{x", ch, NULL, victim, TO_NOTVICT);
	check_improve (ch, gsn_rescue, TRUE, 1);

	stop_fighting (fch, FALSE);
	stop_fighting (victim, FALSE);

	set_fighting (ch, fch);
	set_fighting (fch, ch);
	return;
}

void do_kick (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	int dam;

	if (!IS_NPC (ch) && ch->class == CLASS_MONK)
	{
		do_side_kick(ch,argument);
		return;	
	}
	
	if (!IS_NPC (ch)
		&& ch->level < skill_table[gsn_kick].skill_level[ch->class])
	{
		send_to_char (
				  "You better leave the martial arts to fighters.\n\r", ch);
		return;
	}

	if (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK))
		return;

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 20)) {
		act("You go to kick, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n goes to kick, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	dam = number_range (1, ch->level);
	WAIT_STATE (ch, skill_table[gsn_kick].beats);
	if (get_skill (ch, gsn_kick) > number_percent ())
	{
         if(ch->class == CLASS_WARRIOR || ch->class == CLASS_GLADIATOR
         || ch->class == CLASS_CRUSADER || ch->class == CLASS_SWASHBUCKLER
         || ch->class == CLASS_CLERIC || ch->class == CLASS_PRIEST
         || ch->class == CLASS_VOODAN || ch->class == CLASS_MONK
         || ch->class == CLASS_THIEF || ch->class == CLASS_MERCENARY
         || ch->class == CLASS_ASSASSIN || ch->class == CLASS_NINJA
         || ch->class == CLASS_BARD || ch->class == CLASS_SHADE
         || ch->class == CLASS_LICH || ch->class == CLASS_RANGER
         || ch->class == CLASS_STRIDER || ch->class == CLASS_PALADIN
         || ch->class == CLASS_DARKPALADIN || ch->class == CLASS_HUNTER
	 || ch->class == CLASS_HIGHLANDER)
         {
                       multi_kick (ch, victim);
         }
         else
         {
			damage (ch, victim, number_range (dam, (ch->level * 2.5)),
					gsn_kick, DAM_BASH, TRUE, 0);
         }
		check_improve (ch, gsn_kick, TRUE, 1);
	}
	else
	{
		damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE, 0);
		check_improve (ch, gsn_kick, FALSE, 1);
	}
	return;
}

void do_ambush (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	//COOLDOWN_DATA cd;

	one_argument (argument, arg);

	if (IS_NPC (ch) ||
		ch->level < skill_table[gsn_ambush].skill_level[ch->class])
	{
		send_to_char ("You don't know how to ambush.\n\r", ch);
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char ("You're already fighting!\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char ("Ambush whom?\n\r", ch);
		return;
	}


	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char ("How can you ambush yourself?\n\r", ch);
		return;
	}

/* taken out -bree 4/26/09
	if (!IS_AFFECTED (ch, AFF_SNEAK) || (can_see (victim, ch) && !IS_AFFECTED(ch, gsn_forest_blend) &&	dice(1,6) == 1))
	{
		send_to_char ("But they can see you.\n\r", ch);
		return;
	}
*/

/*        if (IS_NPC (victim) &&
		victim->fighting != NULL &&
		!is_same_group (ch, victim->fighting))
	{
		send_to_char ("Kill stealing is not permitted.\n\r", ch);
		return;
        } */

	if (is_safe (ch, victim))
		return;

	if ((victim->hit < victim->max_hit / 4) ||
		((victim->hit < victim->max_hit / 3) && !IS_NPC (victim)))
	{
		act ("$N is hurt and suspicious and notices your ambush attempt!",
			 ch, NULL, victim, TO_CHAR);
		WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
		multi_hit (ch, victim, TYPE_UNDEFINED);
		return;
	}


/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_ambush))
	{
                send_to_char ("Ambush is still on cooldown.\n\r", ch);
                return;
       }


*	Apply cooldown	
	cd.type      = skill_lookup("ambush");
	cd.duration	 = skill_table[gsn_ambush].cooldown;
        cooldown_on( ch, &cd );*/

	WAIT_STATE (ch, skill_table[gsn_ambush].beats);
	if (!IS_AWAKE (victim)
		|| IS_NPC (ch)
		|| number_percent () < ch->pcdata->learned[gsn_ambush])
	{
		check_improve (ch, gsn_ambush, TRUE, 1);
		multi_hit (ch, victim, gsn_ambush);
	}
	else
	{
		check_improve (ch, gsn_ambush, FALSE, 1);
		damage (ch, victim, 0, gsn_ambush, DAM_NONE, TRUE, 0);
	}
	/* Player shouts if he doesn't die */
	if (!(IS_NPC (victim)) && !(IS_NPC (ch))
		&& victim->position == POS_FIGHTING)
	{
		if (!can_see (victim, ch))
			do_yell (victim, "Help! I've been ambushed by someone!");
		else
		{
			sprintf (buf, "Help! I've been ambushed by %s!",
                                         IS_NPC (ch) ? ch->short_descr : PERS(ch,victim));
			do_yell (victim, buf);
		}
	}

	/*
	 * wimpy check

	if (!IS_NPC (victim)
            && IS_NPC(ch->fighting)
		&& victim->hit > 0
		&& victim->hit <= victim->wimpy
		&& (arena_can_wimpy(victim))
		&& victim->wait < PULSE_VIOLENCE / 2)
                do_flee (victim, "");         */

	return;
}


void do_disarm (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj, *dobj;
	//COOLDOWN_DATA cd;
	int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

	hth = 0;


        if (IN_ARENA(ch))
        {
            send_to_char("Disarm isn't allowed in the arena.\n\r", ch );
            return;
        }

	if ((chance = get_skill (ch, gsn_disarm)) == 0)
	{
		send_to_char ("You don't know how to disarm opponents.\n\r", ch);
		return;
	}

	if ((dobj = get_eq_char (ch, WEAR_WIELD)) == NULL
		&& ((hth = get_skill (ch, gsn_hand_to_hand)) == 0
			|| (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM))))
	{
		send_to_char ("You must wield a weapon to disarm.\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
	{
                send_to_char ("{.Your opponent is not wielding a weapon.{x\n\r", ch);
		return;
	}

/*	Cooldown check	
	if (ch->fighting != NULL && on_cooldown(ch, gsn_disarm))
	{
                send_to_char ("Disarm is still on cooldown.\n\r", ch);
                return;
       }


*	Apply cooldown	
	cd.type      = skill_lookup("disarm");
	cd.duration	 = skill_table[gsn_disarm].cooldown;
        cooldown_on( ch, &cd );*/

	/* find weapon skills */
	ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch));
	vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim));
	ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim));

	/* skill */
	if (get_eq_char (ch, WEAR_WIELD) == NULL)
		chance = chance * hth / 100;
	else
		chance = chance * ch_weapon / 100;

	chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

	/* dex vs. strength */
	chance += get_curr_stat (ch, STAT_DEX)/2;
	chance -= 2 * get_curr_stat (victim, STAT_STR);

	/* level */
	if(IS_NPC (victim))
                chance += abs(ch->level - victim->level) * 5;
	else
		chance += (ch->level - victim->level) * 3;

	/* two-handed vs one-handed */
	if ((dobj != NULL) &&
		((IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) != IS_WEAPON_STAT(dobj,WEAPON_TWO_HANDS))))
		chance /= 4;

	/* and now the attack */
	if (number_percent () < chance)
	{
		if (((chance = get_skill (victim, gsn_grip)) == 0)
			|| (!IS_NPC (victim)
		&& victim->level < skill_table[gsn_grip].skill_level[victim->class]))
		{
			WAIT_STATE (ch, skill_table[gsn_disarm].beats);
			disarm (ch, victim);
			check_improve (ch, gsn_disarm, TRUE, 1);
			return;
		}
		if (number_percent () > (chance / 5) * 4)
		{
			WAIT_STATE (ch, skill_table[gsn_disarm].beats);
			disarm (ch, victim);
			check_improve (ch, gsn_disarm, TRUE, 1);
			check_improve (victim, gsn_grip, FALSE, 1);
			return;
		}
		check_improve (victim, gsn_grip, TRUE, 1);
	}
	    WAIT_STATE (ch, skill_table[gsn_disarm].beats);
        act ("{)You fail to disarm $N.{x", ch, NULL, victim, TO_CHAR);
        act ("{.$n tries to disarm you, but fails.{x", ch, NULL, victim, TO_VICT);
        act ("{($n tries to disarm $N, but fails.{x", ch, NULL, victim, TO_NOTVICT);
	check_improve (ch, gsn_disarm, FALSE, 1);
	return;
}

void
do_surrender (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *mob;
	if ((mob = ch->fighting) == NULL)
	{
		send_to_char ("But you're not fighting!\n\r", ch);
		return;
	}
	act ("You surrender to $N!", ch, NULL, mob, TO_CHAR);
	act ("$n surrenders to you!", ch, NULL, mob, TO_VICT);
	act ("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
	stop_fighting (ch, TRUE);

	if (!IS_NPC (ch) && IS_NPC (mob)
		&& (!HAS_TRIGGER (mob, TRIG_SURR)
			|| !mp_percent_trigger (mob, ch, NULL, NULL, TRIG_SURR)))
	{
		act ("$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR);
		multi_hit (mob, ch, TYPE_UNDEFINED);
	}
}

void
do_sla (CHAR_DATA * ch, char *argument)
{
	send_to_char ("If you want to {RSLAY{x, spell it out.\n\r", ch);
	return;
}



void
do_slay (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		send_to_char ("Slay whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("Suicide is a mortal sin.\n\r", ch);
		return;
	}

	if (!IS_NPC (victim) && victim->level >= get_trust (ch))
	{
                send_to_char ("{.You failed.{c\n\r", ch);
		return;
	}

        if(!IS_NPC(victim) && !str_cmp(victim->name,"Callin"))
	{
         send_to_char("{.You shouldn't have done that.{c\n\r",ch);
         victim = ch;
        }

	if(!IS_NPC(victim) && !str_cmp(victim->name,"Revye"))
	{
	 send_to_char("{RAin't that a bitch...{x\n\r",ch);
	 victim = ch;
	}
	
	if(!IS_NPC(victim) && !str_cmp(victim->name,"bree"))
	{
	 send_to_char("{RDenied.{x\n\r",ch);
	 victim = ch;
	}

	if (IS_NPC (victim) || ch->level > LEVEL_HERO || str_cmp(ch->name,"Lucifer"))
	{
                act ("{)You slay $M in cold blood!{x", ch, NULL, victim, TO_CHAR);
                act ("{.$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT);
                act ("{($n slays $N in cold blood!{x", ch, NULL, victim, TO_NOTVICT);
		raw_kill (victim, ch);
	}
	else
	{
                act ("{)$N wields a sword called '{z{RGodSlayer{i'!{x", ch, NULL, victim, TO_CHAR);
                act ("{.You wield a sword called '{z{RGodSlayer{h'!{x", ch, NULL, victim, TO_VICT);
                act ("{($N wields a sword called '{z{RGodSlayer{k'!{x", ch, NULL, victim, TO_NOTVICT);
                act ("{)$N's slice takes off your left arm!{x", ch, NULL, victim, TO_CHAR);
                act ("{.Your slice takes off $n's left arm!{x", ch, NULL, victim, TO_VICT);
                act ("{($N's slice takes off $n's left arm!{x", ch, NULL, victim, TO_NOTVICT);
                act ("{)$N's slice takes off your right arm!{x", ch, NULL, victim, TO_CHAR);
                act ("{.Your slice takes off $n's right arm!{x", ch, NULL, victim, TO_VICT);
                act ("{($N's slice takes off $n's right arm!{x", ch, NULL, victim, TO_NOTVICT);
                act ("{)$N's slice cuts off both of your legs!{x", ch, NULL, victim, TO_CHAR);
                act ("{.Your slice cuts off both of $n's legs!{x", ch, NULL, victim, TO_VICT);
                act ("{($N's slice cuts off both of $n's legs!{x", ch, NULL, victim, TO_NOTVICT);
                act ("{)$N's slice beheads you!{x", ch, NULL, victim, TO_CHAR);
                act ("{.Your slice beheads $n!{x", ch, NULL, victim, TO_VICT);
                act ("{($N's slice beheads $n!{x", ch, NULL, victim, TO_NOTVICT);
                act ("{)You are DEAD!!!{x", ch, NULL, victim, TO_CHAR);
                act ("{.$n is DEAD!!!{x", ch, NULL, victim, TO_VICT);
                act ("{($n is DEAD!!!{x", ch, NULL, victim, TO_NOTVICT);
				act ("A sword called '{z{RGodSlayer{x' vanishes.", ch, NULL, victim, TO_VICT);
				act ("A sword called '{z{RGodSlayer{x' vanishes.", ch, NULL, victim, TO_NOTVICT);
				raw_kill (ch, victim);
	}
	return;
}

void
check_arena (CHAR_DATA * ch, CHAR_DATA * victim)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	float odds;
	float lvl1, lvl2;
	int payoff;

	sprintf (buf, "[{gArena{x] {b%s{x has {rdefeated{x {b%s{x!\n\r",
			 ch->name, victim->name);
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING &&
			(d->character != victim && d->character != ch)
			&& !IS_SET (d->character->comm, COMM_NOARENA))
		{
			send_to_char (buf, d->character);
		}
	}

	ch->pcdata->awins += 1;
	victim->pcdata->alosses += 1;
	lvl1 = ch->level;
	lvl2 = victim->level;
	odds = (lvl2 / lvl1);
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->connected == CON_PLAYING)
		{
			if (d->character->gladiator == ch)
			{
				payoff = d->character->pcdata->plr_wager * (odds + 1);
				sprintf (buf, "You {gwon{x! Your wager: {y%d{x, payoff: {y%d{x\n\r",
						 d->character->pcdata->plr_wager, payoff);
				send_to_char (buf, d->character);
				d->character->gold += payoff;
/** reset the betting info **/
				d->character->gladiator = NULL;
				d->character->pcdata->plr_wager = 0;
				payoff = 0;
			}
			if (d->character->gladiator != ch
				&& d->character->pcdata->plr_wager >= 1)
			{
				int tmp = 0;
				sprintf (buf, "You {rlost{x! Your wager: {y%d{x\n\r",
						 d->character->pcdata->plr_wager);
				send_to_char (buf, d->character);
				if (d->character->pcdata->plr_wager > d->character->gold)
				{
					tmp = d->character->pcdata->plr_wager / 100;
					d->character->pcdata->plr_wager -= (tmp * 100);
				}
				if (tmp > 0)
					d->character->platinum -= tmp;
				d->character->gold -= d->character->pcdata->plr_wager;
				/* reset the betting info */
				d->character->gladiator = NULL;
				d->character->pcdata->plr_wager = 0;
			}
		}
	}

	/* now move both fighters out of arena and back
	   to the regular "world" be sure to define
	   ROOM_VNUM_AWINNER and ROOM_VNUM_ALOSER */

	stop_fighting (victim, TRUE);
	char_from_room (victim);
	char_to_room (victim, get_room_index (ROOM_VNUM_ALOSER));
	victim->hit = victim->max_hit;
	victim->mana = victim->max_mana;
	update_pos (victim);
	do_look (victim, "auto");
	stop_fighting (ch, TRUE);
	char_from_room (ch);
	char_to_room (ch, get_room_index (ROOM_VNUM_AWINNER));
	ch->hit = ch->max_hit;
	ch->mana = ch->max_mana;
	update_pos (ch);
	do_look (ch, "auto");

	if (IS_SET (ch->act, ACT_CHALLENGER))
		REMOVE_BIT (ch->act, ACT_CHALLENGER);
	if (IS_SET (victim->act, ACT_CHALLENGER))
		REMOVE_BIT (victim->act, ACT_CHALLENGER);
	if (IS_SET (victim->act, ACT_CHALLENGED))
		REMOVE_BIT (victim->act, ACT_CHALLENGED);
	if (IS_SET (ch->act, ACT_CHALLENGED))
		REMOVE_BIT (ch->act, ACT_CHALLENGED);

	ch->challenger = NULL;
	ch->challenged = NULL;
	victim->challenger = NULL;
	victim->challenged = NULL;
	//REMOVE_BIT (ch->comm, COMM_NOCHANNELS);
	REMOVE_BIT (ch->act, PLR_NORESTORE);
	//REMOVE_BIT (victim->comm, COMM_NOCHANNELS);
	REMOVE_BIT (victim->act, PLR_NORESTORE);

	arena = FIGHT_OPEN;			/* clear the arena */

	return;
}

void multi_kick (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int chance;
	int dam;

	/* decrement the wait */
	if (ch->desc == NULL)
		ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);

	if (ch->desc == NULL)
		ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);


	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING)
		return;

	if (ch->stunned)
	{
		ch->stunned--;
		if (!ch->stunned)
		{
			send_to_char ("You regain your equilibrium.\n\r", ch);
			act ("$n regains $m equilibrium.", ch, NULL, NULL, TO_ROOM);
		}
		return;
	}

	if (IS_NPC (ch))
	{
		mob_hit (ch, victim, gsn_kick);
		return;
	}

	dam = number_range (1, ch->level);
        damage (ch, victim, number_range (dam, (ch->level * 2.5)), gsn_kick, DAM_BASH, TRUE, 0);

	if (ch->fighting != victim)
		return;

	chance = get_skill (ch, gsn_second_attack) / 2;

	if (IS_AFFECTED (ch, AFF_SLOW))
		chance /= 2;

        if (IS_AFFECTED (ch, AFF_HASTE))
           chance *= 2;

	if (number_percent () < chance)
	{
		dam = number_range (1, ch->level);
                damage (ch, victim, number_range (dam, (ch->level * 2.25)),gsn_kick, DAM_BASH, TRUE, 0);
		check_improve (ch, gsn_second_attack, TRUE, 5);
		if (ch->fighting != victim)
			return;
	}
	else
	{
		return;
	}

	chance = get_skill (ch, gsn_third_attack) / 2;

	if (IS_AFFECTED (ch, AFF_SLOW))
		chance /= 2;

        if (IS_AFFECTED (ch, AFF_HASTE))
           chance *= 2;

	if (number_percent () < chance)
	{
		dam = number_range (1, ch->level);
		damage (ch, victim, number_range (dam, (ch->level * 2)),
				gsn_kick, DAM_BASH, TRUE, 0);
		check_improve (ch, gsn_third_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	}
	else
	{
		return;
	}

	chance = get_skill (ch, gsn_fourth_attack) / 2;

	if (IS_AFFECTED (ch, AFF_SLOW))
		chance /= 3;

        if (IS_AFFECTED (ch, AFF_HASTE))
           chance *= 2;

	if (number_percent () < chance)
	{
		dam = number_range (1, ch->level);
		damage (ch, victim, number_range (dam, (ch->level * 1.75)),
				gsn_kick, DAM_BASH, TRUE, 0);
		check_improve (ch, gsn_fourth_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	}
	else
	{
		return;
	}

	chance = get_skill (ch, gsn_fifth_attack) / 2;

	if (IS_AFFECTED (ch, AFF_SLOW))
                chance /= 5;

        if (IS_AFFECTED (ch, AFF_HASTE))
           chance *= 2;

	if (number_percent () < chance)
	{
		dam = number_range (1, ch->level);
		damage (ch, victim, number_range (dam, (ch->level * 1.5)),
				gsn_kick, DAM_BASH, TRUE, 0);
		check_improve (ch, gsn_fifth_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	}

        chance = get_skill (ch, gsn_sixth_attack) / 2;

	if (IS_AFFECTED (ch, AFF_SLOW))
		chance = 0;

	if (number_percent () < chance)
	{
		dam = number_range (1, ch->level);
		damage (ch, victim, number_range (dam, (ch->level * 1.5)),
				gsn_kick, DAM_BASH, TRUE, 0);
                check_improve (ch, gsn_sixth_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	}

	return;
}

/*
 Class-Specific Enhanced Damage by Fesdor.  Date: Late '08 (yeah it rhymes)
 */

static int edam_bonus( CHAR_DATA *ch, int damage )
{
	//Overriding values in case the class's code does not modify them.
	int		min=5;
	int		max=30;
	int		bonus=1;
	char buf[MSL/4];
	int cclass = ch->class;
	


	switch (cclass)
	{
		case CLASS_WARRIOR:
		case CLASS_GLADIATOR:
		{
			min = 5;
            max = 60;
            break;
		}
                case CLASS_SWASHBUCKLER:
                {
	                //All Warriors must be berserking.
	                if (IS_AFFECTED (ch, AFF_BERSERK))
	                {
	                //Swashbuckler enhanced cares about hitroll.
	                min = 2;
	                max = 15;
	                min += (min*GET_HITROLL(ch))/250;
	                max += (max*GET_HITROLL(ch))/250;//max=34 with 250 hr
	                break;
                	}
                	else
                	{
	                	min = 3;
	                	max = 15;
	                	break;
                	}
                }
                case CLASS_CRUSADER:
                {
	                //All Warriors must be berserking.
	                if (IS_AFFECTED (ch, AFF_BERSERK))
	                {
	                //Crusader enhanced cares about align
	                if (ch->alignment <= 0)
	                {
		                min = 5;
		                max = 20;
		                max += (max*ch->alignment)/1000;
	                }
	                else if (ch->alignment != 1000)
	                {
		             	min = 10;
		             	max = 15;
		             	max += (max*ch->alignment)/1000;//lim(max) as align->1000 = 30
	                }
	                else
	                {
		             	min = 10;
		             	max = 35;//Small bonus for having true 1000 align (pure good)
	                }
	                break;
	                }
                	else
                	{
	                	min = 3;
	                	max = 15;
	                	break;
                	}
                }
                case CLASS_HIGHLANDER:
                {
	                //All Warriors must be berserking.
	                if (IS_AFFECTED (ch, AFF_BERSERK))
	                {
	                //Highlander enhanced cares about power level.
	                min = 5;
	                max = 22;
	                if (ch->pcdata->power[POWER_LEVEL] >= 3)
	                {
	                max += (max*ch->pcdata->power[POWER_LEVEL])/3;//max=66 at plevel 6.
                	}
                	else
                	{
	                	min = 5;
	                	max = 30;//Let's not cripple the newbies
                	}
	                break;
	                }
                	else
                	{
	                	min = 3;
	                	max = 15;
	                	break;
                	}
                }

		case CLASS_RANGER:
		case CLASS_STRIDER:
		{
			min = 5;
            max = 50;
            break;
        }
                case CLASS_DARKPALADIN:
                {
	                //Darkpaladin cares about black magic rating and align (only if not evil).
	                min = 7;
	                max = 18;
	                max += (max*ch->magic[MAGIC_BLACK])/90;//max=45 for 200 black
	                if (ch->alignment > 0)
	                {
		                max -= (max*ch->alignment)/1000;
	                }
	                break;
                }
                case CLASS_HUNTER:
                {
	                //Nice bonus if in forest, decent bonus if in any other "outdoor" sector.
	                if((ch->in_room->sector_type == SECT_CITY || ch->in_room->sector_type == SECT_INSIDE) && !IS_SET(ch->in_room->affected_by,ROOM_AFF_OVERGROWN))
	                {
		                min = 5;
		                max = 25;
	                }
	                else if (ch->in_room->sector_type != SECT_FOREST && !IS_SET(ch->in_room->affected_by,ROOM_AFF_OVERGROWN))
	                {
		                min = 5;
		                max = 50;
	                }
	                else
	                {
		                min = 10;
		                max = 75;
	                }
	                break;
                }
                case CLASS_PALADIN:
                {
	                //Paladin cares about white magic rating and align (only if not good).
	                //Straight copy of darkpaladin with proper switches made.
	                min = 5;
	                max = 15;
	                max += (max*ch->magic[MAGIC_WHITE])/100;
	                if (ch->alignment < 0)
	                {
		                max += (max*ch->alignment)/1000;
	                }
	                break;
                }
                        

		case CLASS_THIEF:
		case CLASS_MERCENARY:
				min = 5;
				max = 30;
				break;
                case CLASS_ASSASSIN:
                {
	                min = 5;
	                max = 15;
	                if (IS_AFFECTED(ch,AFF_SNEAK))
	                {
	                max += (max*get_skill(ch,gsn_sneak))/50; //45 at 100%
                	}
	                break;
                }
                case CLASS_NINJA:
                {
	                
	                break;
                }
                case CLASS_BARD:
                {
	                
	                break;
                }
				

		case CLASS_SHADE:
		case CLASS_LICH:
				min = 5;
				max = 35;
				break;
				case CLASS_FADE:
				{
	                if ( IS_SET(ch->ability_lich, ABILITY_LICH_NIGHTSTALKER) && (vam_str[time_info.hour] != 0 ))
	                {
						//Daytime
		                if ((time_info.hour > 6) && (time_info.hour < 18))
							{
								min = 5;
								max = 15;
							}
						//Hours 18-23, no fixing needed
						else if ((time_info.hour >= 18) && (time_info.hour <= 23))
							{
								min = 5;
								min += ((min * vam_str[time_info.hour]) / 12);
								max = 30;
								max += ((max * vam_str[time_info.hour]) / 12);
							}
						//Hours 1-6, flip them.
						else if ((time_info.hour >= 1) && (time_info.hour <= 6))
						{
							min = 5;
							min += ((min * (24 - vam_str[time_info.hour])) / 12);
							max = 30;
							max += ((max * (24 - vam_str[time_info.hour])) / 12);
						}
	                }
	                //If not in nightstalker mode, flat rate.
	                else if (!IS_SET(ch->ability_lich, ABILITY_LICH_NIGHTSTALKER))
	                {
		             	min = 5;
		             	max = 25;   
	                }
	                //The Witching Hour: 0
	                else if (vam_str[time_info.hour] == 0)
	                {
		                min = 15;//The maximum minimum\maximum.  Say that 10 times fast.
		                max = 90;
	                }
	                	                
	                break;
                }
                case CLASS_NECROMANCER:
                {
	                
	                break;
                }
                case CLASS_BANSHEE:
                {
	                //See Fade above for structure comments.
	                if ( IS_SET(ch->ability_lich, ABILITY_LICH_NIGHTSTALKER) && (vam_str[time_info.hour] != 0 ))
	                {
		                if ((time_info.hour > 6) && (time_info.hour < 18))
							{
								min = 5;
								max = 15;
							}
						else if ((time_info.hour >= 18) && (time_info.hour <= 23))
							{
								min = 5;
								min += ((min * get_age(ch)) / 300);//Small age factor
								min += ((min * vam_str[time_info.hour]) / 18);
								max = 25;
								min += ((min * get_age(ch)) / 300);
								max += ((max * vam_str[time_info.hour]) / 18);
							}
						else if ((time_info.hour >= 1) && (time_info.hour <= 6))
						{
							min = 5;
							min += ((min * get_age(ch)) / 300);
							min += ((min * (24 - vam_str[time_info.hour])) / 18);
							max = 25;
							min += ((min * get_age(ch)) / 300);
							max += ((max * (24 - vam_str[time_info.hour])) / 18);
						}
	                }
	                else if (!IS_SET(ch->ability_lich, ABILITY_LICH_NIGHTSTALKER))
	                {
		             	min = 5;
		             	max = 25;   
	                }
	                else if (vam_str[time_info.hour] == 0)
	                {
		                //Gotta calculate them this time.
		                min = 5;
						min += ((min * get_age(ch)) / 300);
						min *= 4/3;
						max = 25;
						min += ((min * get_age(ch)) / 300);
						max *= 4/3;
	                }
	                break;
                }
				


		case CLASS_CLERIC:
		case CLASS_PRIEST:
				min = 5;
				max = 30;
				break;
                case CLASS_VOODAN:
                {
	                
	                break;
                }
				case CLASS_MONK:
				{
	                if (is_affected(ch,gsn_martial_arts))
	                {
		                min = 3;
		                max = 10;
	                }
	                else
	                {
		                min = 1;
		                max = 2;
	                }
	                break;
                }
                case CLASS_SAINT:
                {
	                
	                break;
                }
				

 		case CLASS_DRUID:
                case CLASS_SAGE:
                min = 5;
                max = 25;
			 	break;
                case CLASS_WARLOCK:  //Based on saves.
                {
	                min = 3;
	                max = 15;
	                min -= (min*ch->saving_throw)/50; //9 at -100
	                max -= (max*ch->saving_throw)/33; //60 at -100
	                break;
                }
                case CLASS_ALCHEMIST:
                {
	                
	                break;
                }
                case CLASS_SHAMAN:	//Inversely proportional to hp
                {
	                min = 10;
	                max = 20;
	                max += ((max*(ch->max_hit-ch->hit))/ch->max_hit)*2; //max is 50 at 20% hp.
	                break;
                }
                        

		case CLASS_MAGE:
		case CLASS_WIZARD:
				min = 5;
				max = 20;
				break;
                case CLASS_FORSAKEN:
                {	//Bonus for a weapon with a magical damtype.
	                int dt = 0;
	                OBJ_DATA *wield;
	                wield = get_eq_char(ch, WEAR_WIELD);
	                if (wield != NULL && wield->item_type == ITEM_WEAPON)
					{
	                dt += wield->value[3];
					}
	                else
					{
	                dt += ch->dam_type;
                	}
                	//Super ghetto because table in const.c is badly organized.  Make sure if you change the damtype table you change this check.
                	if ((dt==14) || (18<=dt && dt<=20) || (28<=dt && dt<=31) || (dt==33) || (dt==35) || (dt==36) || (dt==38) || (dt==39) || (dt > 40))
                	{
	                	min = 10;
	                	max = (7*number_range(6,10)+10*number_range(1,7));//52 to 140, weighted to the high side 
                	}
                	else
                	{
	                	min = 5;
	                	max = 30;
                	}
	                break;
                }
                case CLASS_CONJURER:
                {
	                
	                break;
                }
                case CLASS_ARCHMAGE:
                {	//Bonus if you have a weapon with a magical damtype, same as forsaken only slightly less varied bonus
	                int dt = 0;
	                OBJ_DATA *wield;
	                wield = get_eq_char(ch, WEAR_WIELD);
	                if (wield != NULL && wield->item_type == ITEM_WEAPON)
					{
	                dt += wield->value[3];
					}
	                else
					{
	                dt += ch->dam_type;
                	}
                	//Make sure if you change the damtype table in const.c you change this check and vise verse.
                	if ((dt==14) || (18<=dt && dt<=20) || (28<=dt && dt<=31) || (dt==33) || (dt==35) || (dt==36) || (dt==38) || (dt==39) || (dt > 40))
                	{
	                	min = 8;
	                	max = (7*number_range(8,10)+10*number_range(1,4));//66 to 110, weighted high 
                	}
                	else
                	{
	                	min = 5;
	                	max = 30;
                	}
	                break;
                }
				
		default:
			min = 5;
			max = 15;
			break;
	}
	bonus = min + (rand()%(max-min));
	if (ch->level > 101)
	{
	sprintf(buf,"{REnhanced Damage: {Wmin:%d {Ymax:%d {Cbonus:%d {Mreturn:%d{x\n\r",min,max,bonus,damage*bonus/90);
	send_to_char(buf,ch);
	}
	return ((damage*bonus)/90);
}

void update_power(CHAR_DATA *highlander)
{
 if(highlander->pcdata->power[POWER_POINTS] <= 1250)
 highlander->pcdata->power[POWER_LEVEL] = 1;
 else if(highlander->pcdata->power[POWER_POINTS] <= 3000)
 highlander->pcdata->power[POWER_LEVEL] = 2;
 else if(highlander->pcdata->power[POWER_POINTS] <= 8500)
 highlander->pcdata->power[POWER_LEVEL] = 3;
 else if(highlander->pcdata->power[POWER_POINTS] <= 17500)
 highlander->pcdata->power[POWER_LEVEL] = 4;
 else if(highlander->pcdata->power[POWER_POINTS] <= 35000)
 highlander->pcdata->power[POWER_LEVEL] = 5;
 else if(highlander->pcdata->power[POWER_POINTS] > 35000)
 highlander->pcdata->power[POWER_LEVEL] = 6;

 return;
}

void falcon_load(CHAR_DATA *ch, CHAR_DATA *victim)
{
 CHAR_DATA *falcon;

 falcon = create_mobile( get_mob_index(MOB_VNUM_FALCON) );
 falcon->alignment = ch->alignment;
 falcon->level = ch->level;
 falcon->hitroll = (ch->level);
 falcon->max_hit = (ch->max_hit * 2)/3;
 falcon->hit = (ch->max_hit/2);
 falcon->damroll = (ch->level);
 falcon->damage[DICE_NUMBER] = (ch->level/5)+1;
 falcon->damage[DICE_TYPE] = dice(6,6);
 SET_BIT(falcon->res_flags, RES_WEAPON);
 SET_BIT (falcon->act, ACT_PET);
 SET_BIT (falcon->affected_by, AFF_CHARM);
 falcon->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
 add_follower (falcon, ch);
 falcon->leader = ch;
 ch->pet = falcon;
 char_to_room( falcon, ch->in_room );
 act( "$N swoops down from the skies above to aid $n.", ch, NULL, falcon, TO_ROOM );
 send_to_char("A large Battle Falcon swoops down from the skies above, coming to your aid.\n\r",ch);
 return;
}

/*
 * Part Creator....for lack of one. -- Skyntil
 */
void part_create(CHAR_DATA *ch, char *argument)
{
    char  arg [MAX_INPUT_LENGTH];
    int vnum;

    argument = one_argument(argument,arg);
    vnum = 0;

    if(IS_SET(ch->act,ACT_NO_BODY))
     return;

    if (arg[0] == '\0') return;
    if (!str_cmp(arg,"head")) vnum = OBJ_VNUM_SEVERED_HEAD;
    else if (!str_cmp(arg,"arm")) vnum = OBJ_VNUM_SLICED_ARM;
    else if (!str_cmp(arg,"leg")) vnum = OBJ_VNUM_SLICED_LEG;
    else if (!str_cmp(arg,"heart")) vnum = OBJ_VNUM_TORN_HEART;
    else if (!str_cmp(arg,"guts")) vnum = OBJ_VNUM_GUTS;
    else if (!str_cmp(arg,"brain")) vnum = OBJ_VNUM_BRAINS;
    else if (!str_cmp(arg,"blood")) vnum = OBJ_VNUM_BLOOD;
    else if (!str_cmp(arg,"eye")) vnum = OBJ_VNUM_EYEBALL;

    // no body parts in arena
    if(!IS_NPC(ch))
    {
     if (IS_SET(ch->in_room->room_flags,ROOM_ARENA))
       vnum = OBJ_VNUM_BLOOD;
    }

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
        if (IS_NPC(ch))   obj->timer = number_range(4,7);
        else              obj->timer = number_range(12,18);

        if (vnum == OBJ_VNUM_BLOOD) obj->timer = number_range(1,4);

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

        sprintf( buf, obj->name, name );
        free_string( obj->name );
        obj->name = str_dup( buf );

        if (IS_NPC (ch))
        {
          obj->value[4] = 0;
        }
        else
        {
          obj->value[4] = 1;
        }

	obj_to_room( obj, ch->in_room );
    }
    return;
}

void
do_sskill (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument (argument, arg);

	if (IS_NPC (ch))
		return;

	if (arg[0] == '\0')
		sprintf (arg, ch->name);

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC (victim))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}

	skillstance (ch, victim);
	return;

}

void
skillstance (CHAR_DATA * ch, CHAR_DATA * victim)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[25];
	char stancename[10];
	int stance;

	if (IS_NPC (victim))
		return;

	stance = victim->stance[0];
	if (stance < 1 || stance > 10)
	{
		send_to_char ("You aren't in a stance!\n\r", ch);
		return;
	}
	if (victim->stance[stance] <= 0)
		sprintf (bufskill, "completely unskilled in");
	else if (victim->stance[stance] <= 25)
		sprintf (bufskill, "an apprentice of");
	else if (victim->stance[stance] <= 50)
		sprintf (bufskill, "a trainee of");
	else if (victim->stance[stance] <= 75)
		sprintf (bufskill, "a student of");
	else if (victim->stance[stance] <= 100)
		sprintf (bufskill, "fairly experienced in");
	else if (victim->stance[stance] <= 125)
		sprintf (bufskill, "well trained in");
	else if (victim->stance[stance] <= 150)
		sprintf (bufskill, "highly skilled in");
	else if (victim->stance[stance] <= 175)
		sprintf (bufskill, "an expert of");
	else if (victim->stance[stance] <= 199)
		sprintf (bufskill, "a master of");
	else if (victim->stance[stance] >= 200)
		sprintf (bufskill, "a grand master of");
	else
	{
		send_to_char ("You aren't in a stance!\n\r", ch);
		return;
	}
	if (stance == STANCE_VIPER)
		sprintf (stancename, "viper");
	else if (stance == STANCE_CRANE)
		sprintf (stancename, "crane");
	else if (stance == STANCE_CRAB)
		sprintf (stancename, "crab");
	else if (stance == STANCE_MONGOOSE)
		sprintf (stancename, "mongoose");
	else if (stance == STANCE_BULL)
		sprintf (stancename, "bull");
	else if (stance == STANCE_MANTIS)
		sprintf (stancename, "mantis");
	else if (stance == STANCE_DRAGON)
		sprintf (stancename, "dragon");
	else if (stance == STANCE_TIGER)
		sprintf (stancename, "tiger");
	else if (stance == STANCE_MONKEY)
		sprintf (stancename, "monkey");
	else if (stance == STANCE_SWALLOW)
		sprintf (stancename, "swallow");
	else
		return;
	if (ch == victim)
		sprintf (buf, "You are %s the %s stance.", bufskill, stancename);
	else
		sprintf (buf, "$N is %s the %s stance.", bufskill, stancename);
	act (buf, ch, NULL, victim, TO_CHAR);
	return;
}

int dambonus (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int stance)
{

	if (dam < 1)
        return 0;

	if (stance < 1 || stance > 10)
         return dam;

	if (dam > 10000)
	 return 10000;

	return dam;
}


bool can_counter (CHAR_DATA * ch)
{
      if (IS_NPC (ch))
              return FALSE;

      if(IS_SET(ch->stance_aff,STANCE_AFF_COUNTER) && number_percent() <
      ch->stance[ch->stance[0]] * 0.5)
        return TRUE;

      return FALSE;
}

bool can_bypass (CHAR_DATA * ch, CHAR_DATA * victim)
{
      if (IS_NPC (ch) || IS_NPC (victim))
             return FALSE;

      if(IS_SET(ch->stance_aff,STANCE_AFF_BYPASS) && number_percent() <
      ch->stance[ch->stance[0]] / 2)
        return TRUE;

      return FALSE;
}



void
improve_stance (CHAR_DATA * ch)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[25];
	char stancename[10];
	int dice1;
	int dice2;
	int dice3;
	int stance;

	dice1 = number_percent ();
	dice2 = number_percent ();
	dice3 = number_percent ();

	if (IS_NPC (ch))
		return;

	stance = ch->stance[0];
	if (stance < 1 || stance > 10)
		return;
	
	if (ch->stance[stance] >= 200)
	{
		ch->stance[stance] = 200;
		return;
	}
	
	if ( ch->stance[stance] < 100 )
	{
		if (((dice1 > ch->stance[stance] && dice2 > ch->stance[stance]) ||
			(dice1 >= 94 || dice2 >= 94)) && dice3 > 50)
			ch->stance[stance] += 1;
		else
			return;
	}
	else
	{
		if (((dice1 > ch->stance[stance] && dice2 > ch->stance[stance]) ||
			(dice1 >= 99 || dice2 >= 99)) && dice3 > 60)
			ch->stance[stance] += 1;
		else
			return;
	}
	
	if (stance == ch->stance[stance])
		return;

	if (ch->stance[stance] == 1)
		sprintf (bufskill, "an apprentice of");
	else if (ch->stance[stance] == 26)
		sprintf (bufskill, "a trainee of");
	else if (ch->stance[stance] == 51)
		sprintf (bufskill, "a student of");
	else if (ch->stance[stance] == 76)
		sprintf (bufskill, "fairly experienced in");
	else if (ch->stance[stance] == 101)
		sprintf (bufskill, "well trained in");
	else if (ch->stance[stance] == 126)
		sprintf (bufskill, "highly skilled in");
	else if (ch->stance[stance] == 151)
		sprintf (bufskill, "an expert of");
	else if (ch->stance[stance] == 176)
		sprintf (bufskill, "a master of");
	else if (ch->stance[stance] == 200)
		sprintf (bufskill, "a grand master of");
	else
		return;
	if (stance == STANCE_VIPER)
		sprintf (stancename, "viper");
	else if (stance == STANCE_CRANE)
		sprintf (stancename, "crane");
	else if (stance == STANCE_CRAB)
		sprintf (stancename, "crab");
	else if (stance == STANCE_MONGOOSE)
		sprintf (stancename, "mongoose");
	else if (stance == STANCE_BULL)
		sprintf (stancename, "bull");
	else if (stance == STANCE_MANTIS)
		sprintf (stancename, "mantis");
	else if (stance == STANCE_DRAGON)
		sprintf (stancename, "dragon");
	else if (stance == STANCE_TIGER)
		sprintf (stancename, "tiger");
	else if (stance == STANCE_MONKEY)
		sprintf (stancename, "monkey");
	else if (stance == STANCE_SWALLOW)
		sprintf (stancename, "swallow");
	else
		return;
	sprintf (buf, "You are now %s the %s stance.\n\r", bufskill, stancename);
	send_to_char (buf, ch);
	return;
}

void do_stance (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int selection;

	argument = one_argument (argument, arg);

	if (IS_NPC (ch))
		return;
	if (arg[0] == '\0')
	{
		if (ch->stance[0] == -1)
		{
			ch->stance[0] = 0;
			send_to_char ("You drop into a fighting stance.\n\r", ch);
			act ("$n drops into a fighting stance.", ch, NULL, NULL, TO_ROOM);
		}
		else
		{
			ch->stance[0] = -1;
			send_to_char ("You relax from your fighting stance.\n\r", ch);
			act ("$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM);
		}
		return;
	}
	if (ch->stance[0] > 0)
	{
		send_to_char ("You cannot change stances until you come up from the one you are currently in.\n\r", ch);
		return;
	}
	if (!str_cmp (arg, "none"))
	{
		selection = STANCE_NONE;
		send_to_char ("You drop into a general fighting stance.\n\r", ch);
		act ("$n drops into a general fighting stance.", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
	}
	else if (!str_cmp (arg, "viper"))
	{
		selection = STANCE_VIPER;
		send_to_char ("You arch your body into the viper fighting stance.\n\r", ch);
		act ("$n arches $s body into the viper fighting stance.", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
                SET_BIT(ch->stance_aff,STANCE_AFF_SPEED);
                SET_BIT(ch->stance_aff,STANCE_AFF_BYPASS);
                if(IS_SET(ch->plyr,PLAYER_THIEF))
                 SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED);
	}
	else if (!str_cmp (arg, "crane"))
	{
		selection = STANCE_CRANE;
		send_to_char ("You swing your body into the crane fighting stance.\n\r", ch);
		act ("$n swings $s body into the crane fighting stance.", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
                SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_BLOCK);
                SET_BIT(ch->stance_aff,STANCE_AFF_BLOCK);
                SET_BIT(ch->stance_aff,STANCE_AFF_COUNTER);
	}
	else if (!str_cmp (arg, "crab"))
	{
		selection = STANCE_CRAB;
		send_to_char ("You squat down into the crab fighting stance.\n\r", ch);
		act ("$n squats down into the crab fighting stance. ", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
                SET_BIT(ch->stance_aff,STANCE_AFF_ROLL);
                SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DEFENSE);
	}
	else if (!str_cmp (arg, "mongoose"))
	{
		selection = STANCE_MONGOOSE;
		send_to_char ("You twist into the mongoose fighting stance.\n\r", ch);
		act ("$n twists into the mongoose fighting stance. ", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
                SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DODGE);
                SET_BIT(ch->stance_aff,STANCE_AFF_ACROBATICS);
	}
	else if (!str_cmp (arg, "bull"))
	{
		selection = STANCE_BULL;
		send_to_char ("You hunch down into the bull fighting stance.\n\r", ch);
		act ("$n hunches down into the bull fighting stance. ", ch, NULL, NULL, TO_ROOM);
                clear_stance_affects(ch);
                SET_BIT(ch->stance_aff,STANCE_AFF_AGGRESSIVE);
                SET_BIT(ch->stance_aff,STANCE_AFF_COUNTER);
                if(IS_SET(ch->plyr,PLAYER_WARRIOR))
                 SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_POWER);
	}
	else
	{
		if (!str_cmp (arg, "mantis") && ch->stance[STANCE_CRANE] >= 200 &&
			ch->stance[STANCE_VIPER] >= 200)
		{
			selection = STANCE_MANTIS;
			send_to_char ("You spin your body into the mantis fighting stance.\n\r", ch);
			act ("$n spins $s body into the mantis fighting stance.", ch, NULL, NULL, TO_ROOM);
                        clear_stance_affects(ch);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ENHANCED_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_SPEED);
                        SET_BIT(ch->stance_aff,STANCE_AFF_BYPASS);
                        if(IS_SET(ch->plyr,PLAYER_THIEF))
                         SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED);
		}
		else if (!str_cmp (arg, "dragon") && ch->stance[STANCE_BULL] >= 200
				 &&
				 ch->stance[STANCE_CRAB] >= 200)
		{
			selection = STANCE_DRAGON;
			send_to_char ("You coil your body into the dragon fighting stance.\n\r", ch);
			act ("$n coils $s body into the dragon fighting stance.", ch, NULL, NULL, TO_ROOM);
                        clear_stance_affects(ch);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ROLL);
                        SET_BIT(ch->stance_aff,STANCE_AFF_AGGRESSIVE);
                        SET_BIT(ch->stance_aff,STANCE_AFF_COUNTER);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DEFENSE);
                        if(IS_SET(ch->plyr,PLAYER_WARRIOR))
                         SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_POWER);
		}
		else if (!str_cmp (arg, "tiger") && ch->stance[STANCE_BULL] >= 200
				 &&
				 ch->stance[STANCE_VIPER] >= 200)
		{
			selection = STANCE_TIGER;
			send_to_char ("You lunge into the tiger fighting stance.\n\r", ch);
			act ("$n lunges into the tiger fighting stance.", ch, NULL, NULL, TO_ROOM);
                        clear_stance_affects(ch);
                        SET_BIT(ch->stance_aff,STANCE_AFF_AGGRESSIVE);
                        SET_BIT(ch->stance_aff,STANCE_AFF_COUNTER);
                        SET_BIT(ch->stance_aff,STANCE_AFF_BYPASS);
                        SET_BIT(ch->stance_aff,STANCE_AFF_SPEED);
                        if(IS_SET(ch->plyr,PLAYER_THIEF))
                         SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_SPEED);
                        if(IS_SET(ch->plyr,PLAYER_WARRIOR))
                         SET_BIT(ch->stance_aff,STANCE_AFF_DOUBLE_POWER);
		}
		else if (!str_cmp (arg, "monkey") && ch->stance[STANCE_CRANE] >= 200
				 &&
				 ch->stance[STANCE_MONGOOSE] >= 200)
		{
			selection = STANCE_MONKEY;
			send_to_char ("You rotate your body into the monkey fighting stance.\n\r", ch);
			act ("$n rotates $s body into the monkey fighting stance.", ch, NULL, NULL, TO_ROOM);
                        clear_stance_affects(ch);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ENHANCED_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_BLOCK);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DODGE);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ACROBATICS);
		}
		else if (!str_cmp (arg, "swallow") && ch->stance[STANCE_CRAB] >= 200
				 &&
				 ch->stance[STANCE_MONGOOSE] >= 200)
		{
			selection = STANCE_SWALLOW;
			send_to_char ("You slide into the swallow fighting stance.\n\r", ch);
			act ("$n slides into the swallow fighting stance.", ch, NULL, NULL, TO_ROOM);
                        clear_stance_affects(ch);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DODGE);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ACROBATICS);
                        SET_BIT(ch->stance_aff,STANCE_AFF_ROLL);
                        SET_BIT(ch->stance_aff,STANCE_AFF_IMPROVED_DEFENSE);
		}
		else
		{
			send_to_char ("Syntax is: stance <style>.\n\r", ch);
			send_to_char ("Stance being one of: None, Viper, Crane, Crab, Mongoose, Bull.\n\r", ch);
			return;
		}
	}
	ch->stance[0] = selection;
	WAIT_STATE (ch, 16);
	return;
}

void clear_stance_affects(CHAR_DATA *ch)
{
 ch->stance_aff = 0;
}

int add_damroll(CHAR_DATA *ch, int diceside)
{
 int value = 0, damroll;

 damroll = GET_MOD_DAMROLL(ch);

 // More on this
 if(diceside == 1)
 {
  if(damroll > 10)
   value++;
  if(damroll > 25)
   value++;
  if(damroll > 40)
   value++;
  if(damroll > 70)
   value++;
  if(damroll > 100)
   value++;
  if(damroll > 150)
   value++;
  if(damroll > 200)
   value++;
  if(damroll > 300)
   value++;
  if(damroll > 400)
   value++;
 }
 // Less on this
 else if(diceside == 2)
 {
  if(damroll > 25)
   value++;
  if(damroll > 60)
   value++;
  if(damroll > 100)
   value++;
  if(damroll > 135)
   value++;
  if(damroll > 200)
   value++;
  if(damroll > 275)
   value++;
  if(damroll > 400)
   value++;
 }

 return value;
}

int check_martial_arts(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int roll;
    int bonus = 0;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];

    if( (roll = number_percent()) < ((get_skill(ch,gsn_martial_arts)*3)/8) )
    {
            switch(number_range(1,5))
            {
                case (1):
                    strcpy( buf1, "You smash your fist into $N's chest!" );
                    strcpy( buf2, "$n smashes $s fist into your chest!" );
                    strcpy( buf3, "$n smashes $s fist into $N's chest!" );
                    WAIT_STATE(victim, 4);
                    bonus = 9;
                    break;
                case (2):
                    strcpy( buf1, "You duck below $N's attack and deliver a chop to $N's shins!" );
                    strcpy( buf2, "$n ducks below your attack and delivers a chop to your shins!" );
                    strcpy( buf3, "$n ducks below $N's attack and delivers a chop to $N's shins!" );
                    bonus = 11;
                    break;
                case (3):
                    strcpy( buf1, "You viciously strike $N's shoulder-blade!" );
                    strcpy( buf2, "$n viciously strikes your shoulder-blade!" );
                    strcpy( buf3, "$n viciously strikes $N's shoulder-blade!" );
                    bonus = 9;
                    break;
                case (4):
                    strcpy( buf1, "You reverse-kick $N in the side!" );
                    strcpy( buf2, "$n reverse-kicks you in the side!" );
                    strcpy( buf3, "$n reverse-kicks $N in the side!" );
                    bonus = 20;
                    WAIT_STATE(victim, 6);
                    break;
                case (5):
                    strcpy( buf1, "You send a swift kick at $N's knees!" );
                    strcpy( buf2, "$n sends a swift kick at your knees!" );
                    strcpy( buf3, "$n sends a swift kick at $N's knees!" );
                    bonus = 12;
                    break;
            }

        act(buf1,ch,0,victim,TO_CHAR);
        act(buf2,ch,0,victim,TO_VICT);
        act(buf3,ch,0,victim,TO_NOTVICT);
        check_improve(ch,gsn_martial_arts,TRUE,6);
    } else{
        bonus = 0;
        check_improve(ch,gsn_martial_arts,FALSE,6);
   }
    if (bonus == 0)
        return 0;
    bonus = ch->level+dice(ch->level/4,bonus);
    return bonus;
}

bool check_open_palming(CHAR_DATA *ch)
{
    int chance = 8;
    if (!is_affected(ch,gsn_palm))
        return FALSE;

    if(ch->pcdata->power[1] >= 400)
      chance = 65;

    if (number_percent() > chance)
        return FALSE;

    return TRUE;
}

/*
 * Check for Block.
 */
bool check_block(CHAR_DATA * ch, CHAR_DATA * victim)
{
        int chance = 0, dam = 0;

	if (!IS_AWAKE (victim))
          return FALSE;

        if(IS_NPC(victim) || !IS_SET(victim->stance_aff,STANCE_AFF_BLOCK))
         return FALSE;

        /* no block if blind */
	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = get_skill( victim, gsn_blind_fighting );
		chance = (chance * 3) / 5;
		chance += 25;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

        chance = get_skill (victim, gsn_hand_to_hand);

	if (!can_see (victim, ch))
		chance /= 2;

        if(!IS_NPC(victim) && victim->class == CLASS_MONK && is_affected(victim,gsn_martial_arts)) {
         if(victim->pcdata->power[1] > number_percent())
          chance *= 2;
        }

        /* STANCES */
        if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

        if(IS_SET(victim->stance_aff,STANCE_AFF_ENHANCED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

	chance -= (GET_HITROLL (ch) * 0.2);

        if (number_percent () >= chance + (victim->level - ch->level)/2)
		return FALSE;

	if (victim->stunned)
		return FALSE;

        // Stance Counter
        if(can_counter(ch))
        {
         if(number_percent() < 15)
         {
            dam = number_range( 20, (ch->level*3) );
            act( "$n counters your block!", ch, NULL, victim, TO_VICT);
            act( "You counter $N's block and strike $N.", ch, NULL, victim, TO_CHAR);
            act( "$n counters $N's block.", ch, NULL, victim, TO_NOTVICT);
            damage(ch,victim,number_range( dam,(ch->level*5) ),gsn_parry,DAM_SLASH,TRUE,0);
            return FALSE;
         }
        }

        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You block $n's attack!{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N blocks your attack!{x", ch, NULL, victim, TO_CHAR);

    return TRUE;
}

/*
 * Check for Acrobatics.
 */
bool check_acrobatics(CHAR_DATA * ch, CHAR_DATA * victim)
{
        int chance = 0, dam = 0;

	if (!IS_AWAKE (victim))
          return FALSE;

        if(IS_NPC(victim) || !IS_SET(victim->stance_aff,STANCE_AFF_ACROBATICS))
         return FALSE;

	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = get_skill( victim, gsn_blind_fighting );
		chance = (chance * 3) / 5;
		chance += 25;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

       if(number_percent() < (victim->stance[victim->stance[0]] * 0.5))
        chance = 45; /* Increased to 45 because stance_aff was almost useless */

	if (!can_see (victim, ch))
		chance /= 2;

        if(!IS_NPC(victim) && victim->class == CLASS_MONK) {
         if(victim->pcdata->power[1] > number_percent())
          chance *= 2;
        }

        chance -= (GET_HITROLL (ch) * 0.1);

        if (number_percent () >= chance + (victim->level - ch->level)/2)
		return FALSE;

	if (victim->stunned)
		return FALSE;

        if(can_bypass(ch,victim))
        {
         if(number_percent() < 15)
         {
            dam = number_range( 20, (ch->level*3) );
            act( "$n follows your flip!", ch, NULL, victim, TO_VICT);
            act( "You follow $N's flip and strike $M.", ch, NULL, victim, TO_CHAR);
            act( "$N follows $n's flip and strikes $M.", victim, NULL, ch, TO_NOTVICT);
            damage(ch,victim,number_range( dam,(ch->level*5) ),gsn_parry,DAM_SLASH,TRUE,0);
            return FALSE;
         }
        }

     if(number_range(1,2) == 1)
     {
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You flip away from $n's attack!{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N flips away from your attack!{x", ch, NULL, victim, TO_CHAR);
     }
     else
     {
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You leap away from $n's attack!{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N leaps away from your attack!{x", ch, NULL, victim, TO_CHAR);
     }

    return TRUE;
}

/*
 * Check for Roll.
 */
bool check_roll(CHAR_DATA * ch, CHAR_DATA * victim)
{
        int chance = 0, dam = 0;

	if (!IS_AWAKE (victim))
          return FALSE;

        if(IS_NPC(victim) || !IS_SET(victim->stance_aff,STANCE_AFF_ROLL))
         return FALSE;

	if (IS_AFFECTED (victim, AFF_BLIND))
	{
		int bfsucc;

		chance = get_skill( victim, gsn_blind_fighting );
		chance = (chance * 3) / 5;
		chance += 25;
		bfsucc= ((number_percent() <= chance));
		check_improve(victim, gsn_blind_fighting, bfsucc, 12);
		if (!bfsucc)
			return FALSE;
	}

        if(number_percent() < victim->stance[victim->stance[0]] * 0.5)
         chance = 45; /* Increased due to uselessness */

	if (!can_see (victim, ch))
		chance /= 2;

        if(!IS_NPC(victim) && victim->class == CLASS_MONK) {
         if(victim->pcdata->power[1] > number_percent())
          chance *= 2;
        }

        chance -= (GET_HITROLL (ch) * 0.1);  /* Added back in */

        if (number_percent () >= chance + (victim->level - ch->level)/2)
		return FALSE;

	if (victim->stunned)
		return FALSE;

        // Stance Counter
        if(can_counter(ch))
        {
         if(number_percent() < 15)
         {
            dam = number_range( 20, (ch->level*3) );
            act( "$n counters your roll!", ch, NULL, victim, TO_VICT);
            act( "You counter $N's roll and strike $N.", ch, NULL, victim, TO_CHAR);
            act( "$n counters $N's roll.", ch, NULL, victim, TO_NOTVICT);
            damage(ch,victim,number_range( dam,(ch->level*5) ),gsn_parry,DAM_SLASH,TRUE,0);
            return FALSE;
         }
        }

        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You roll $n's attack off to the side!{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N rolls your attack away from $M!{x", ch, NULL, victim, TO_CHAR);

    return TRUE;
}

void improve_toughness (CHAR_DATA * ch)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[25];
        int dice1,dice3;
        int dice2,dice4;

	dice1 = number_percent ();
	dice2 = number_percent ();
        dice3 = number_percent ();
        dice4 = number_percent ();

	if (IS_NPC (ch))
		return;

        if (ch->toughness >= 500)
	{
                ch->toughness = 500;
		return;
	}
        if (((dice1 > ch->toughness && dice2 > ch->toughness) ||
        (dice3 > ch->toughness && dice4 > ch->toughness)) ||
        ((dice1 == 100 || dice2 == 100) || (dice3 == 100 || dice4 == 100)))
                ch->toughness += 1;
	else
		return;


        if (ch->toughness == 1)
                sprintf (bufskill, "You feel your skin tingling.");
        else if (ch->toughness == 51)
                sprintf (bufskill, "Your skin feels tougher.");
        else if (ch->toughness == 101)
                sprintf (bufskill, "Your skin begins to harden.");
        else if (ch->toughness == 151)
                sprintf (bufskill, "Your flesh fuses tighter.");
        else if (ch->toughness == 200)
                sprintf (bufskill, "You feel more damage resistant.");
        else if (ch->toughness == 300)
                sprintf (bufskill, "Your skin turns to steel.");
        else if (ch->toughness == 400)
                sprintf (bufskill, "Your muscles feel like iron.");
        else if (ch->toughness == 500)
                sprintf (bufskill, "You develop steel nerves.");
	else
		return;
        sprintf (buf, "\n\r%s\n\r", bufskill);
	send_to_char (buf, ch);
	return;
}

bool check_deflect(CHAR_DATA *ch, CHAR_DATA *victim, int dt,int dam)
{
    int chance;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
   /* char *attack;*/
    int vict_dex;
    int ch_dex;

    if(dt < 1000 || dam <= 0)
    {
	sprintf(buf1,"DT: %d",dt);
	    wiznet(buf1,NULL,NULL,WIZ_DEBUG,0,0);
	    return FALSE;
    }
    
    if( dt == gsn_backstab
     || dt == gsn_ambush
     || dt == gsn_circle
     || dt == gsn_lunge
     || dt == gsn_bladedance
     || dt == gsn_bash
     || dt == gsn_adamantium_palm)
     return FALSE;

    //Wiznet test
    //wiznet("Deflect Calc",NULL,NULL,WIZ_DEBUG,0,0);
    chance = ( get_skill(victim,gsn_deflect) / 8); 

    if(chance == 0)
	    return FALSE;
    
    if (!can_see(victim,ch))
        chance /= 2;

    // Better check here, dex & level range
    vict_dex = get_curr_stat(victim, STAT_DEX);
    ch_dex = get_curr_stat(ch, STAT_DEX);
    
    if(!IS_NPC(ch))
     chance -= (ch_dex - vict_dex);
    else
     chance -= ( (ch_dex*3/4) - vict_dex);
     
    if(ch->level > victim->level+10)
     chance -= 10;
    else if(ch->level > victim->level)
     chance -= 5;
    else if((ch->level + 10) < victim->level)
     chance += 4;
    else if( ch->level < victim->level)
     chance += 8;
     
    if (chance > 25)
        chance = 25;

    if ( number_percent( ) > chance )
    {
        check_improve(victim,gsn_deflect,FALSE,4);
        return FALSE;
    }
    
    wiznet("Deflect Success",NULL,NULL,WIZ_DEBUG,0,0);
    
    //attack = dt_name(dt);

    sprintf(buf2,"You turn $n's %s back on him!","attack");
    sprintf(buf1,"$N turns your %s back on you!","attack");
    act(buf2,ch,NULL,victim,TO_VICT);

    if(!IS_NPC(ch))
     act(buf1,ch,NULL,victim,TO_CHAR);

    check_improve(victim,gsn_deflect,TRUE,5);

    damage(victim,ch,dam,gsn_deflect,DAM_BASH,TRUE, 0);

    return TRUE;

}

bool check_counter_defense(CHAR_DATA *ch,CHAR_DATA *victim,int dt)
{
        int chance;

        if ((chance = get_skill(ch,gsn_counter_defense)) == 0
        || IS_NPC(ch)
        || ch->level < skill_table[gsn_counter_defense].skill_level[ch->class])
                return FALSE;

        chance /= (ch->level/15);
        if (chance > 20)
        chance = 20;

        if(dt == gsn_dodge)
        chance += 5;

        if (number_percent() < chance)
        {
        check_improve(ch,gsn_counter_defense,TRUE,3);
        return TRUE;
        }

        check_improve(ch,gsn_counter_defense,FALSE,2);
        return FALSE;
}


/***************************************************************************
*  PK/Arena Ranking Code - Written Exclusively for Asgardian Nightmare Mud *
*  by Chris Langlois. tas@intrepid.inetsolve.com	 		   *
*  Ranking system is a modification of Professor Arpad Elo's ranking	   *
*  which is also commonly used by FIDE - The International Chess	   *
*  Federation.  More info on the specifics of the system can be found at   *
*  http://www.vogclub.com/ratings/elodetails.phtml			   *
***************************************************************************/

int rank_pk (CHAR_DATA * ch, CHAR_DATA * victim)
{
  double mod = 0;
  double bias = 0;
  double buffer = 0;

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		return 0;
	}

	/* Mod = 1/(1+10^((vrank-chrank)/200)) */
	mod = (victim->pcdata->prank - ch->pcdata->prank)/200;
	mod = pow(10,mod);
	mod = (1/(1+mod));

	/* Bias = 25 + (15*(vtier - chtier)) + (2*(vlvl - plvl)) */
	bias = victim->pcdata->tier - ch->pcdata->tier;
	bias *= 15;
	bias = 25 + bias + (2*(victim->level - ch->level));

	/* Rn = Ro + Bias( W - Mod ) */

	if(ch->class == CLASS_HIGHLANDER && is_pkill(ch))
        {
	   ch->pcdata->power[POWER_POINTS] = ((ch->pcdata->prank-1500)*100)+5000;
	   update_power(ch);
	}
	if(victim->class == CLASS_HIGHLANDER && is_pkill(victim))
        {
	   victim->pcdata->power[POWER_POINTS] = ((victim->pcdata->prank-1500)*100)+5000;
	   update_power(victim);
	}

	buffer = (bias * ( 1 - mod ) );

	return UMAX(buffer, 0);
}

int chance_pk (CHAR_DATA * ch, CHAR_DATA * victim)
{
  double mod = 0;

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		return 0;
	}

	/* Mod = 1/(1+10^((vrank-chrank)/200)) */
	mod = (victim->pcdata->prank - ch->pcdata->prank);
	mod /= 200;
	mod = pow(10,mod);
	mod = (1/(1+mod));
	mod *= 100;

	return ( mod );
}

int rank_arena (CHAR_DATA * ch, CHAR_DATA * victim)
{
  double mod = 0;
  double bias = 0;
  double buffer = 0;

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		return 0;
	}

	/* Mod = 1/(1+10^((vrank-chrank)/200)) */
	mod = (victim->pcdata->arank - ch->pcdata->arank)/200;
	mod = pow(10,mod);
	mod = (1/(1+mod));

	/* Bias = 25 + (15*(vtier - chtier)) + (2*(vlvl - plvl)) */
	bias = victim->pcdata->tier - ch->pcdata->tier;
	bias *= 15;
	bias = 25 + bias + (2*(victim->level - ch->level));

	/* Rn = Ro + Bias( W - Mod ) */

	if(ch->class == CLASS_HIGHLANDER && !is_pkill(ch))
        {
	   ch->pcdata->power[POWER_POINTS] = ((ch->pcdata->arank-1500)*100)+5000;
	   update_power(ch);
	}
	if(victim->class == CLASS_HIGHLANDER && !is_pkill(victim))
        {
	   victim->pcdata->power[POWER_POINTS] = ((victim->pcdata->arank-1500)*100)+5000;
	   update_power(victim);
	}

	buffer = (bias * ( 1 - mod ) );

	return UMAX(buffer, 0);
}

int chance_arena (CHAR_DATA * ch, CHAR_DATA * victim)
{
  double mod = 0;

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		return 0;
	}

	/* Mod = 1/(1+10^((vrank-chrank)/200)) */
	mod = (victim->pcdata->arank - ch->pcdata->arank);
	mod /= 200;
	mod = pow(10,mod);
	mod = (1/(1+mod));
	mod *= 100;

	return ( mod );
}

void do_rank (CHAR_DATA * ch, char *argument)
{
  char arg1 [MIL];
  char arg2 [MIL];
  char buf [MSL];

	smash_tilde( argument );
	argument = one_argument(argument, arg1);
	strcpy(arg2, argument);

	if (!strcmp(arg1, "arena"))
	{
		if (!strcmp(arg2, "list"))
		{

		}
		else
		{
			DESCRIPTOR_DATA *d;
			bool found = FALSE;

			for (d = descriptor_list; d != NULL; d = d->next)
			{
				CHAR_DATA *wch;
				int winmod = 0;
				int losemod = 0;
				int pchance = 0;

				if (d->connected != CON_PLAYING || !can_see (ch, d->character))
					continue;

				wch = (d->original != NULL) ? d->original : d->character;

				if (!can_see (ch, wch))
					continue;

				if (IS_NPC (wch))
					continue;

				if (!str_prefix (arg2, wch->name))
				{
					found = TRUE;

					pchance = chance_arena(ch, wch);
					winmod = rank_arena(ch, wch);
					losemod = rank_arena(wch, ch);

					sprintf ( buf, "%s rank: %d.  Your rank: %d.\n\r",
					  wch->name, wch->pcdata->arank, ch->pcdata->arank);
					send_to_char(buf, ch);
					sprintf ( buf, "Your rank would become -  Win: %d  Loss: %d\n\r",
					  (ch->pcdata->arank + winmod), (ch->pcdata->arank - losemod));
					send_to_char(buf, ch);
					sprintf ( buf, "%s's rank would become -  Win: %d  Loss: %d\n\r",
					  wch->name,
					  (wch->pcdata->arank + losemod), (wch->pcdata->arank - winmod));
					send_to_char(buf, ch);
					sprintf ( buf, "You have a %d%% chance of winning.\n\r", pchance);
					send_to_char(buf, ch);

					return;
				}
			}

			if (!found)
			{
				send_to_char("Syntax:\n\r", ch);
				send_to_char("  rank arena list\n\r", ch);
				send_to_char("  rank arena <name>\n\r", ch);
				send_to_char("  rank pk list\n\r", ch);
				send_to_char("  rank pk <name>\n\r", ch);
				send_to_char("  rank all list\n\r", ch);
				return;
			}
		}
	}
	else if (!strcmp(arg1, "pk"))
	{
		if (!strcmp(arg2, "list"))
		{

		}
		else
		{
			DESCRIPTOR_DATA *d;
			bool found = FALSE;

			for (d = descriptor_list; d != NULL; d = d->next)
			{
				CHAR_DATA *wch;
				int winmod = 0;
				int losemod = 0;
				int pchance = 0;

				if (d->connected != CON_PLAYING || !can_see (ch, d->character))
					continue;

				wch = (d->original != NULL) ? d->original : d->character;

				if (!can_see (ch, wch))
					continue;

				if (IS_NPC (wch))
					continue;

				if (!str_prefix (arg2, wch->name))
				{
					found = TRUE;

					pchance = chance_pk(ch, wch);
					winmod = rank_pk(ch, wch);
					losemod = rank_pk(wch, ch);

					sprintf ( buf, "%s rank: %d.  Your rank: %d.\n\r",
					  wch->name, wch->pcdata->prank, ch->pcdata->prank);
					send_to_char(buf, ch);
					sprintf ( buf, "Your rank would become -  Win: %d  Loss: %d\n\r",
					  (ch->pcdata->prank + winmod), (ch->pcdata->prank - losemod));
					send_to_char(buf, ch);
					sprintf ( buf, "%s's rank would become -  Win: %d  Loss: %d\n\r",
					  wch->name,
					  (wch->pcdata->prank + losemod), (wch->pcdata->prank - winmod));
					send_to_char(buf, ch);
					sprintf ( buf, "You have a %d%% chance of winning.\n\r", pchance);
					send_to_char(buf, ch);


					return;
				}
			}

			if (!found)
			{
				send_to_char("Syntax:\n\r", ch);
				send_to_char("  rank arena list\n\r", ch);
				send_to_char("  rank arena <name>\n\r", ch);
				send_to_char("  rank pk list\n\r", ch);
				send_to_char("  rank pk <name>\n\r", ch);
				send_to_char("  rank all list\n\r", ch);
				return;
			}
		}
	}
	else if (!strcmp(arg1, "all"))
	{
		if (!strcmp(arg2, "list"))
		{
         return;
		}
		else
		{

			send_to_char("Syntax:\n\r",ch);
			send_to_char("  rank arena list\n\r", ch);
			send_to_char("  rank arena <name>\n\r", ch);
			send_to_char("  rank pk list\n\r", ch);
			send_to_char("  rank pk <name>\n\r", ch);
			send_to_char("  rank all list\n\r", ch);
			return;
		}
	}
	else
	{
		send_to_char("Syntax:\n\r", ch);
		send_to_char("  rank arena list\n\r", ch);
		send_to_char("  rank arena <name>\n\r", ch);
		send_to_char("  rank pk list\n\r", ch);
		send_to_char("  rank pk <name>\n\r", ch);
		send_to_char("  rank all list\n\r", ch);
		return;
	}
}

bool check_quick_step(CHAR_DATA *ch, CHAR_DATA *victim) 
{ 
    int chance; 
 
    if (IS_NPC(ch)) 
    return FALSE; 
 
    if ( !IS_AWAKE(ch) ) 
    return FALSE; 
 
    if (ch->pcdata->learned[gsn_quick_step] < 1 
    || !has_skill(ch,gsn_quick_step)) 
    return FALSE; 
 
    if (!is_affected(ch,gsn_martial_arts))
    return FALSE;
    
    chance = ( get_skill(ch,gsn_quick_step) / 10); 
 
    chance += ( ch->level / 10); 
     
    chance -= chance / 3; 
 
    if ( number_percent( ) > chance ) 
    { 
        check_improve(ch,gsn_quick_step,FALSE,4);  
        return FALSE; 
    } 
 
    act("$n begins to move around you very quickly.",ch,0,victim,TO_VICT); 
    act("You start to move around $N with very quick steps.",ch,0,victim,TO_CHAR); 
    act("$n begins to move around $N very quickly.",ch,0,victim,TO_NOTVICT);     
 
    check_improve(ch,gsn_quick_step,TRUE,5); 
 
    return TRUE; 
}
 

