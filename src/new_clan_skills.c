#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#include <unistd.h>		
#include <sys/time.h>
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "merc.h"
#include "clan.h"
#include "magic.h"

int focus_dam args ((CHAR_DATA *ch)); 

/* New Swashbuckler Skill
*Head Cut by Bree
*/

void do_head_cut( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam; 
        char arg[MAX_INPUT_LENGTH]; 
        OBJ_DATA *wield,*second; 
        int multiplier; 
        bool DUAL = FALSE; 
int hroll = 0; 
        dam = 0; 
AFFECT_DATA af;
        one_argument(argument,arg); 
        


        if ((chance = get_skill(ch,gsn_head_cut) ) == 0 
        || !has_skill(ch,gsn_head_cut) ) 
        { 
        send_to_char("You do not know how to head cut\n\r",ch); 
        return; 
        } 
        if (arg[0] == '\0') 
        { 
        victim = ch->fighting; 
         if (victim == NULL) 
         { 
            send_to_char("But you aren't fighting anyone!\n\r",ch); 
            return; 
         } 
        } 
        else if ( (victim = get_char_room(ch,arg)) == NULL ) 
        { 
                send_to_char("They aren't here.\n\r",ch); 
                return; 
        } 
        if (victim == ch) 
        { 
        send_to_char("Cut at your own head? Cute.\n\r",ch); 
        return; 
        }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt a head cut, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts a head cut, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

        if(ch->stunned)
        {
         send_to_char("You are still a bit woozy.\n\r",ch);
         return;
        }
     
        wield = get_eq_char(ch,WEAR_WIELD); 
        second = get_eq_char(ch,WEAR_SECONDARY); 
 
        if ((wield == NULL) 
        || (( wield->value[0] != WEAPON_SWORD) 
        && ( wield->value[0] != WEAPON_AXE) 
        && ( wield->value[0] != WEAPON_DAGGER) )) 
        { 
         if ((second == NULL) 
         || (( second->value[0] != WEAPON_SWORD) 
         && ( second->value[0] != WEAPON_AXE) 
         && ( second->value[0] != WEAPON_DAGGER) )) 
         { 
          send_to_char("You must be wielding a sword or axe or dagger to attempt a head cut.\n\r",ch); 
          return; 
         } 
         else 
         { 
         DUAL = TRUE; 
         } 
        } 
 
        if (is_safe(ch,victim) ) 
        return; 
 
        chance += ch->carry_weight/25; 
        chance -= victim->carry_weight/20; 
        chance += (ch->size - victim->size)*20; 
        chance -= get_curr_stat(victim,STAT_DEX); 
        chance += get_curr_stat(ch,STAT_STR)/3; 
        chance += get_curr_stat(ch,STAT_DEX)/2; 
        if (IS_AFFECTED(ch,AFF_HASTE) ) 
                chance += 10; 
        if (IS_AFFECTED(victim,AFF_HASTE) ) 
                chance -= 15; 
        chance += (ch->level - victim->level);

        act("$n attempts a cut at $N's head!",ch,0,victim,TO_NOTVICT); 
        act("You attempt a head cut on $N",ch,0,victim,TO_CHAR); 
        act("$n attempts to cut at your head!",ch,0,victim,TO_VICT); 
 
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_head_cut,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_head_cut].beats + 4); 
 
        if(DUAL) 
        wield = second; 
 
        if (wield->pIndexData->new_format) 
                dam = dice(wield->value[1],wield->value[2]); 
        else 
                dam = number_range(wield->value[1],wield->value[2]); 
 
        if (get_skill(ch,gsn_enhanced_damage) > 0 ) 
        { 
         if (number_percent() <= get_skill(ch,gsn_enhanced_damage) ) 
         { 
           check_improve(ch,gsn_enhanced_damage,TRUE,1); 
           dam += dam * (number_range(25,75)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100; 
         } 
        } 
 
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_head_cut]; 
        dam /= 100; 
        multiplier = number_range(ch->level/3, ch->level/2); 
        multiplier /= 10; 
        multiplier += 5/4; 
        dam *= multiplier;
        
     /*    spell_blind(gsn_blind, 2 * level / 6, ch, (void *) victim,TARGET_CHAR); */
        
        if(!DUAL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
         dam *= 1.25;
 		
       hroll = ch->hitroll; 
 
       if ((hroll) >= 250)
        
       dam =+ dam*2;
       
       if ((hroll) >= 350)
      
       dam =+ dam*4/2.3;
                 
 


       if (!IS_NPC(victim))
        {
	        dam = dam*2/3;
        }
         

        if (dam <= 0) 
                dam = 1; 


        damage(ch,victim,dam,gsn_head_cut,DAM_PIERCE,TRUE,0); 
        } 
        else
          if (number_range(1, 10) >= 8)
        {
                 af.where     = TO_AFFECTS;
                 af.type      = gsn_head_cut;
                 af.level     = ch->level;
                 af.bitvector = AFF_BLIND;
                 af.duration  = 1;
                 af.modifier  = -4;
                 af.location  = APPLY_DEX;
         act( "$n scores a cut above the eye and {Rblood{x starts to pour into their eyes!", victim, NULL, NULL, TO_ROOM );
        damage(ch,victim,dam,gsn_head_cut,DAM_PIERCE,TRUE,0);
           check_improve(ch,gsn_head_cut,FALSE,1);
           WAIT_STATE(ch,skill_table[gsn_head_cut].beats + 4); 
     }
     else
{
       damage(ch,victim,dam,gsn_head_cut,DAM_PIERCE,TRUE,0); 
       check_improve(ch,gsn_head_cut,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_head_cut].beats + 4); 
        } 
        return; 
} 

void do_grenado( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam; 
        char arg[MAX_INPUT_LENGTH]; 
        int multiplier; 
        int hroll = 0; 
        dam = 0; 
        AFFECT_DATA af;
        
        one_argument(argument,arg); 

        if ((chance = get_skill(ch,gsn_grenado) ) == 0 
        || !has_skill(ch,gsn_grenado) ) 
        { 
        send_to_char("You do not know how to make or use a grenado!\n\r",ch); 
        return; 
        } 
        
        
        /* if (arg[0] == '\0') 
        { 
        victim = ch->fighting; 
         if (victim == NULL) 
         { 
            send_to_char("But you aren't fighting anyone!\n\r",ch); 
            return; 
         } 
        } */
        else if ( (victim = get_char_room(ch,arg)) == NULL ) 
        { 	
                send_to_char("They aren't here.\n\r",ch); 
                return; 
        } 

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You try to toss a grenado, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n tries to toss a grenado at you, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

        if(ch->stunned)
        {
         send_to_char("You are still a bit woozy.\n\r",ch);
         return;
        }
     
     
 
        if (is_safe(ch,victim) ) 
        return; 
 
        chance += ch->hitroll/45; 
        chance -= victim->hitroll/42; 
        chance += (ch->size - victim->size)*20; 
        chance -= get_curr_stat(victim,STAT_DEX)/2; 
        chance += get_curr_stat(ch,STAT_STR)/2 + 1; 
        chance += get_curr_stat(ch,STAT_DEX) + 2; 
        
        
        if (IS_AFFECTED(ch,AFF_HASTE) ) 
                chance += 16; 
        if (IS_AFFECTED(victim,AFF_HASTE) ) 
                chance -= 12; 
        
        chance += (ch->level - victim->level);

        act("$n tosses a {gg{Gr{Rena{Gd{go{x at $N!",ch,0,victim,TO_NOTVICT); 
        act("You tosses a {gg{Gr{Rena{Gd{go{x at $N",ch,0,victim,TO_CHAR); 
        act("$n tosses a {gg{Gr{Rena{Gd{go{x at you!",ch,0,victim,TO_VICT); 
 
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_grenado,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_grenado].beats + 8); 
 
        if (get_skill(ch,gsn_enhanced_damage) > 0 ) 
        { 
         if (number_percent() <= get_skill(ch,gsn_enhanced_damage) ) 
         { 
           check_improve(ch,gsn_enhanced_damage,TRUE,1); 
           dam += dam * (number_range(25,80)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100; 
         } 
        } 
 
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_grenado]; 
        dam /= 100; 
        multiplier = number_range(ch->level/3, ch->level/2); 
        multiplier /= 8.3; 
        multiplier += 5/2.5; 
        dam *= multiplier;
        
       hroll = ch->hitroll; 
 
       if ((hroll) >= 230)
        
       dam =+ dam*2;
       
       if ((hroll) >= 300)
      
       dam =+ dam*4.2/2.4;
       
       if ((hroll) >= 360)
       
       dam =+ dam*5/4.5;

       if (number_range(1, 10) >= 3)
        {
                 af.where     = TO_AFFECTS;
                 af.type      = gsn_grenado;
                 af.level     = ch->level;
                 af.bitvector = AFF_BLIND;
                 af.duration  = 1;
                 af.modifier  = -2;
                 af.location  = APPLY_DEX;
         act( "$n's {gg{Gr{Rena{Gd{go{x explodes and the target appears to be blind!", victim, NULL, NULL, TO_ROOM );
         damage(ch,victim,dam,gsn_grenado,DAM_SOUND,TRUE,0);
           check_improve(ch,gsn_grenado,FALSE,1);
           WAIT_STATE(ch,skill_table[gsn_grenado].beats + 10); 
        }
                           
               damage(ch,victim,dam,gsn_grenado,DAM_SOUND,TRUE,0);  
       }  
        
     else
{
       damage(ch,victim,dam,gsn_grenado,DAM_SOUND,TRUE,0); 
       check_improve(ch,gsn_grenado,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_grenado].beats + 10); 
       } 
        return; 
} 


/* Holy Aura spell by Bree.
This spell gives a Crusader roughly -38 ac at
101 by praying to their god for protection. */

void spell_holy_aura( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already protected.\n\r",ch); 
	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -level / 4;
    af.bitvector = SHD_STONE;
    affect_to_char( victim, &af );
    act( "$n is protected by a holy aura", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a holy aura.\n\r", victim );
 
   return;

}


          
/* New Shaman Spell By Bree 
This spell allows the Shaman to channel their 
ancestors, thus giving them more knowledge which
translates into -ac and small hitroll */

 void spell_generations( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already channeling the knowledge of your ancestors.\n\r",ch); 
	else
	  act("$N is already channeling the knowledge of their ancestors.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 30;
    af.location  = APPLY_AC;
    af.modifier  = -39;
    af.bitvector = SHD_STONE;
    affect_to_char( victim, &af );
   
    af.location = APPLY_HITROLL;
    af.modifier = 20;
    af.duration = 30;
    affect_to_char( victim, &af );
    
    act( "$n looks more knowledgable.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin channeling your ancestors.\n\r", victim );
 
   return;

}


/* New Ninja michi/fast heal combination By Bree */
void do_inner_str(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_inner_str) <= 0)
        || (ch->level < skill_table[gsn_inner_str].skill_level[ch->class] ) )
    {
        send_to_char("Do what now?\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_inner_str) )
    {
        send_to_char("You are already channeling your inner strength\n\r",ch);
	return;
    }

    if (ch->mana < 200 || ch->move < 200)
    {
        send_to_char("You do not have enough mental power to concentrate.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_inner_str))
    {
        send_to_char("You concentrate on channeling your inner strength, but cannot acheive it.\n\r",ch);
        check_improve(ch,gsn_inner_str,FALSE,1);
        ch->mana -= 100;
        ch->move -= 100;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_inner_str;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.modifier = ch->level*get_skill(ch,gsn_inner_str)/225;
    af.duration = ch->level/6;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.location = APPLY_REGEN;
    af.modifier = 90;
    af.duration = ch->level/6;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    
    
    ch->mana -= 200;
    ch->move -= 200;

    act("$n's begins channeling their inner strength.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You focus on your inner strength.\n\r",ch);
    check_improve(ch,gsn_inner_str,TRUE,1);
 
   return;
}



/* Saskia Clan Skill By Bree
This basically is like blindness dust, a room blind */

 
void do_solar_flare(CHAR_DATA *ch,char *argument) 
{ 

        bool is_safe_quiet args ((CHAR_DATA * ch, CHAR_DATA * victim));
        CHAR_DATA *vch; 
        CHAR_DATA *vch_next; 
        AFFECT_DATA af;    
       // int chance; 
        bool fighting = FALSE; 
      //  char buf[MAX_STRING_LENGTH];  
 
           if(ch->clan != clan_lookup("saskia"))
        {
         send_to_char("Are you high?\n\r",ch);
         return;
        }
        
       
        if( ch->stunned )
    	{
		send_to_char("You're still a bit woozy.\n\r",ch);
		return;
    	}
    	
        if (ch->mana < 36) 
        { 
        send_to_char("You don't have the mana.\n\r",ch); 
        return; 
        } 
        

        if (number_percent() < 5 ) 
        { 
        act("$n launches a solar flare into the air but it doesn't burst.",ch,0,0,TO_ROOM); 
        send_to_char("You throw out a solar flare but it doesn't burst!\n\r",ch); 
        ch->mana -= 18; 
        check_improve(ch,gsn_solar_flare,FALSE,2); 
        WAIT_STATE(ch,9); 
        return; 
        } 
        
        act("$n hurls a solar flare into the room!",ch,0,0,TO_ROOM); 
        send_to_char("You throw a solar flare into the room!\n\r",ch); 
        check_improve(ch,gsn_solar_flare,TRUE,2); 
        if (ch->fighting != NULL) 
                fighting = TRUE; 
 
        af.where = TO_AFFECTS; 
        af.type = gsn_solar_flare; 
        af.level = ch->level; 
        af.duration = (ch->level)/10; 
        af.bitvector = AFF_BLIND; 
        af.location = APPLY_HITROLL; 
        af.modifier = -4; 
        ch->mana -= 36; 
        WAIT_STATE(ch,9); 
        
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next) 
        { 
        	vch_next = vch->next_in_room; 
        	if (is_safe_quiet(ch,vch))    
                	 continue; 
        	if (is_same_group(ch,vch))       
	                continue; 
          	if(!str_cmp(clan_table[vch->clan].name,"saskia"))
                  continue;


                if (!IS_AFFECTED(vch,AFF_BLIND) && !saves_spell( ch->level,vch,DAM_OTHER)) 
        	{ 
        		act("$n appears blinded.",vch,0,0,TO_ROOM); 
        		send_to_char("You get light out of your eyes.\n\r",vch); 
                	affect_to_char(vch,&af); 
        	} 
        	
 
                if (vch->fighting == NULL) 
                        multi_hit(vch,ch,TYPE_UNDEFINED); 
         } 
 
        return; 

} 

/* Prayer (New Saint Skill) By Bree
Prayer basiaclly allows for the saint to pray for
the intervention of the Gods. Since you know, they
spend most of their time praying and shit.
This spell will have a less chance then most of actually
landing. Damage is extremely spikey, from 150-2k, but
mostly avgs around 600-900 */

void do_prayer(CHAR_DATA *ch,char *argument) 
{ 
    CHAR_DATA *victim; 
    int dam, chance, percent; 
    char arg[MAX_INPUT_LENGTH]; 
 
    if ( (chance = get_skill(ch,gsn_prayer)) == 0 
    || !has_skill(ch,gsn_prayer)) 
    { 
        send_to_char("Huh?\n\r",ch); 
        return; 
    } 
 
    one_argument(argument,arg); 
    if (arg[0] == '\0') 
        victim = ch->fighting; 
    else 
        victim = get_char_room(ch,arg); 
 
    if (victim == NULL) 
    { 
        send_to_char("They aren't here.\n\r",ch); 
        return; 
    } 
    if (victim == ch) 
    { 
        send_to_char("You can't pray for the gods to smite YOU, dummy!\n\r",ch); 
        return; 
    }
    
    if (ch->stunned > 0)
    {
	send_to_char ("You're still a little woozy.\n\r", ch);
	return;
    } 

    if ((victim->fighting != ch) && (ch->fighting != victim))
    { 
        send_to_char("But you aren't engaged in combat with them.\n\r",ch); 
        return; 
    } 
 
/*    if (get_eq_char(ch,WEAR_WIELD) != NULL ||
    get_eq_char(ch,WEAR_SECONDARY) != NULL)
    { 
        send_to_char("You cannot pray with items in your hands.\n\r",ch); 
        return; 
    }*/
    
   if(IS_NPC(victim)) {
	   chance += (ch->level - (victim->level - 12 * 2));
	   chance -= get_curr_stat(victim,STAT_DEX)*2/3	; }
   else {
   	   chance += (ch->level - victim->level * 4); 
    	   chance -= get_curr_stat(victim,STAT_DEX);  }
    
    chance += get_curr_stat(ch,STAT_DEX)*3/4; 
    chance = URANGE(5, chance, 90); 
 
    if (number_percent() > chance) 
    { 
        dam = 0; 
        check_improve(ch,gsn_prayer,FALSE,3); 
    } 
    
    else 
    { 

 
	dam = (ch->level * number_range(8,18));

        act("$n calls down the power of the Gods upon $N!",ch,0,victim,TO_NOTVICT);
        act("You pray to the Gods for help!",ch,0,victim,TO_CHAR); 
        act("$n calls down the power of the Gods to smite you!",ch,0,victim,TO_VICT); 
 
        dam += (get_curr_stat(ch,STAT_DEX)+str_app[get_curr_stat(ch,STAT_STR)].todam)*8; 
        check_improve(ch,gsn_prayer,TRUE,3); 
        WAIT_STATE(victim,PULSE_VIOLENCE);
        /* A little extra kick */
        if ((percent = number_percent()) <= (get_skill(ch,gsn_prayer) / 3))  // changed from divided by 5
        {
         dam = 2 * dam + (dam * 6 * percent / 40); //changed from 100
         victim->stunned = 1;
         act ("{)You are stunned, and have trouble getting back up!{x", ch, NULL, victim, TO_VICT);
         act ("{.$N is stunned by the gods!{x", ch, NULL, victim, TO_CHAR);
         act ("{($N is having trouble getting back up.{x", ch, NULL, victim, TO_NOTVICT);
        }
        
        
    } 
 
    damage(ch,victim,dam,gsn_prayer,DAM_BASH,TRUE,0); 
    WAIT_STATE(ch,PULSE_VIOLENCE); 

 
    return; 


} 
/* Insomnia Spell By Bree
Basically this spell makes it more difficult for a 
magic users to land the sleep spell on someone. Very
few classes should get this spell, i am designing it
for 2 classes, Archmage and Saint. Check for sleep and affect_strip it*/
//add to update.c to check for sleep and strip it if insomnia affected.
/* void spell_insomnia( int sn, int level, CHAR_DATA *ch, void *vo,int 
target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already wide awake.\n\r",ch); 
	else
	  act("$N is already wide awake",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    affect_to_char( victim, &af );
   
    act( "$n looks less sleepy", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin to need less sleep.\n\r", victim );
 
   return;

} */

/***** START OF DREAMS ******************/
//if (!IS_NPC(ch) && ch->position == POS_SLEEPING)
//{
//
//if (dream <=186)
//{
//send_to_char("zzzzzzzzzzzzzzzzzzzzzzzzz.\n\r",ch);
//}
// if (is_affected(ch, gsn_insomnia) && ch != NULL)
//
//{   affect_strip(vch,gsn_sleep); 
//    return;
//}

/* silverwolves clan skill by Bree. Basically just like
conceal, turns the player into a wolf.*/

void do_conceal2(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

  if(ch->clan != clan_lookup("silverwolves"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

  if(str_cmp(clan_table[ch->clan].name,"silverwolves"))
  {
   send_to_char("Huh?\n\r",ch);
   return;
  }
    /*
	if (ch->morph_form[0] > 0 && ch->morph_form[0] != MORPH_CONCEAL) 
	  { 
            send_to_char("You cannot conceal yourself while in another morphed form .\n\r",ch);
	    return;
	  }
        else */ 
        if (ch->morph_form[0] == MORPH_CONCEAL2)
        {
            if(is_affected(ch, gsn_conceal2))
            {
                affect_strip(ch,gsn_conceal2);
                /* Incase the affect_strip doesn't work */
                if(ch->morph_form[0] > 0)
                 ch->morph_form[0] = 0;
	    }
           /* if (ch->long_descr != NULL && !str_cmp(ch->long_descr,"A dark assassin stands here.\n\r"))
            {
      free_string(ch->long_descr);
	      ch->long_descr = "\0";
            }*/
            send_to_char("You feel more human again.\n\r",ch); 
	    return;
	}
        else if (number_percent() < get_skill(ch,gsn_third_attack))
	{
	  af.where    = TO_AFFECTS;
          af.type     = gsn_conceal2;
	  af.level    = ch->level;
          af.location = APPLY_MORPH_FORM;
          af.modifier = MORPH_CONCEAL2;
	  af.bitvector = AFF_SNEAK;
	  af.duration = -1;
	  affect_to_char( ch, &af );

          af.bitvector = 0;
          af.location = APPLY_HITROLL;
          af.modifier = number_range((2+ch->level/20),16);
          affect_to_char(ch,&af);
          af.modifier = number_range((2+ch->level/20),16);
          af.location = APPLY_DAMROLL;
          affect_to_char(ch,&af);   
          af.location = APPLY_HIT;    
          af.location = APPLY_DEX;
          af.modifier = 2;
          affect_to_char(ch,&af);   

          check_improve(ch,gsn_conceal2,TRUE,6);
          send_to_char("You transform into a wolf!.\n\r",ch);
	  return;
	}
	else
	{
          check_improve(ch,gsn_conceal2, FALSE,3);
          send_to_char("You become human again.\n\r",ch);
	  return;
	}

   return;

}

/* NighTshade clan skill by Bree */
 void do_snattack(CHAR_DATA *ch, char *argument)
{
/*
        if(ch->clan != clan_lookup("audentia") || ch->clan == clan_lookup("audentia") )
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
*/
if(ch->clan != clan_lookup("audentia"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

  if(str_cmp(clan_table[ch->clan].name,"audentia"))
  {
   send_to_char("Huh?\n\r",ch);
   return;
  }


      if( ch->mana < 100 )
    {
     send_to_char("You do not have the willpower to do this. Gather your energy.\n\r",ch);
         return;
        }
      if (is_affected (ch, gsn_snattack))
        {
               send_to_char("{.You feel less sneaky.{x\n\r",ch);
              affect_strip(ch, gsn_snattack);
        }
	if (number_percent () )
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
              ch->mana -= 100;

              send_to_char ("{.You focus on the arts of NighTshade{x\n\r", ch);
              act ("{($n concentration increases{x", ch, NULL, NULL, TO_ROOM);
              act ("{($n looks more stealth{x", ch, NULL, NULL, TO_ROOM);

               /* check_improve (ch, gsn_crit_strike, TRUE, 2); */

		  		af.where = TO_AFFECTS;
                af.type = gsn_snattack;
				af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

				af.location = APPLY_DAMROLL;
                af.modifier = 5;
				affect_to_char (ch, &af);

        }

        return;
}

void do_snattacktwo(CHAR_DATA *ch, char *argument)
{
/*
        if(ch->clan != clan_lookup("audentia") || ch->clan == clan_lookup("audentia") )
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
*/
if(ch->clan != clan_lookup("iniquity"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

  if(str_cmp(clan_table[ch->clan].name,"iniquity"))
  {
   send_to_char("Huh?\n\r",ch);
   return;
  }


      if( ch->mana < 100 )
    {
     send_to_char("You do not have the willpower to do this. Gather your energy.\n\r",ch);
         return;
        }
      if (is_affected (ch, gsn_snattacktwo))
        {
               send_to_char("{.You feel less sneaky.{x\n\r",ch);
              affect_strip(ch, gsn_snattacktwo);
        }
	if (number_percent () )
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
              ch->mana -= 100;

              send_to_char ("{.You focus on the arts of Iniquity{x\n\r", ch);
              act ("{($n concentration increases{x", ch, NULL, NULL, TO_ROOM);
              act ("{($n looks more stealth{x", ch, NULL, NULL, TO_ROOM);

               /* check_improve (ch, gsn_crit_strike, TRUE, 2); */

		  		af.where = TO_AFFECTS;
                af.type = gsn_snattacktwo;
				af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

				af.location = APPLY_DAMROLL;
                af.modifier = 5;
				affect_to_char (ch, &af);

        }

        return;
}

void do_nightwish(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

        if(ch->clan != clan_lookup("storm"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
           if (is_affected (ch, gsn_nightwish))
	{
                send_to_char ("{.The night begins to subside.{x\n\r", ch);
                affect_strip(ch,gsn_nightwish);
		return;
	}
         send_to_char ("{.The night builds rapidly, bending to your will..{x\n\r", ch);
         act ("{($n's body blends in with the night.{x", ch, NULL, NULL, TO_ROOM);

          

          af.where    = TO_AFFECTS;
          af.type     = gsn_nightwish;
	  af.level    = ch->level;
          af.duration = -1;
          
          af.location = APPLY_DAMROLL;
          af.modifier = 15;
          af.duration = -1;
          affect_to_char(ch,&af);

          return;
    
}

 void do_vengeance(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

        if(ch->clan != clan_lookup("vengeance"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
           if (is_affected (ch, gsn_vengeance))
        {
                send_to_char ("{.Your fury subsides{x\n\r", ch);
                affect_strip(ch,gsn_vengeance);
                return;
        }
         send_to_char ("{.You begin to feel your fury rising.{x\n\r", ch);
         act ("{($n's eye's turn {!red{X, and roll back into their skull!{x", ch, NULL, NULL, TO_ROOM);



          af.where    = TO_AFFECTS;
          af.type     = gsn_vengeance;
              af.level    = ch->level;
          af.duration = -1;
          af.bitvector = 0;

          af.location = APPLY_DAMROLL;
          af.modifier = 15;
          af.duration = -1;
          affect_to_char(ch,&af);
   
          return;
}

 void do_divide(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

        if(ch->clan != clan_lookup("builder"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
           if (is_affected (ch, gsn_divide))
	{
                send_to_char ("{.The divide closes slowly.{x\n\r", ch);
                affect_strip(ch,gsn_divide);
		return;
	}
         send_to_char ("{.A great divide engulfs you!{x\n\r", ch);
         act ("{($n's body becomes one with the divide!{x", ch, NULL, NULL, TO_ROOM);

          

          af.where    = TO_AFFECTS;
          af.type     = gsn_divide;
	      af.level    = ch->level;
          af.duration = -1;
          af.bitvector = 0;
	  
          af.location = APPLY_DAMROLL;
          af.modifier = 15;
          af.duration = -1;
          affect_to_char(ch,&af);
          return;
}

void do_shadow_form(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

        if(ch->clan != clan_lookup("shadows"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
           if (is_affected (ch, gsn_shadow_form))
	{
                send_to_char ("{.The shadows subside.{x\n\r", ch);
                affect_strip(ch,gsn_shadow_form);
		return;
	}
         send_to_char ("{.The shadows envelop you!{x\n\r", ch);
         act ("{($n's body is enveloped in shadows!{x", ch, NULL, NULL, TO_ROOM);

          

          af.where    = TO_AFFECTS;
          af.type     = gsn_shadow_form;
	      af.level    = ch->level;
          af.duration = -1;
          af.bitvector = 0;
	  
          af.location = APPLY_DAMROLL;
          af.modifier = 15;
          af.duration = -1;
          affect_to_char(ch,&af);
/*
          af.location  = APPLY_AC;
          af.modifier  = -34;
          af.bitvector = SHD_STONE;
          affect_to_char(ch,&af);  
          
          af.location = APPLY_REGEN;
          af.modifier = 75;
          af.duration = -1;
          affect_to_char(ch,&af);
*/
          return;
}

 void do_greed(CHAR_DATA *ch, char *argument)
{

           if(ch->clan != clan_lookup("profit"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_greed))
        {
                send_to_char ("{.Your greed begins to subside.{x\n\r",
ch);
                affect_strip(ch,gsn_greed);
                return;
        }

        if( ch->mana < 251 )

        {
         send_to_char("You are not greedy enough to do this.\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_concentration))
        {
                send_to_char("{.Lets get greedy!.{x\n\r",ch);
                affect_strip(ch, gsn_concentration);
        }

        if (number_percent () )
        {
                AFFECT_DATA af;

                WAIT_STATE (ch, PULSE_VIOLENCE);
                ch->mana -= 250;


                send_to_char ("{.Lets get greedy!{x\n\r",ch);
                send_to_char ("{.Greed overwhelms you as your eyes turn {@green!{X{x\n\r", ch);
                act ("{($n's eyes turn green as the greed washes over them.{x", ch, NULL, NULL, TO_ROOM);
                act ("{($n's eyes turn emerald green.{x", ch, NULL, NULL, TO_ROOM);

                /* check_improve (ch, gsn_greed, TRUE, 2); */

                af.where = TO_AFFECTS;
                af.type = gsn_greed;
                af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

                af.location = APPLY_DAMROLL;
                af.modifier = 15;
                af.duration = -1;
                af.bitvector = 0;
                affect_to_char (ch, &af);
                
        }

        return;
}

/* wolfpack clan skill by bree */
void do_bloodlust(CHAR_DATA *ch, char *argument)
{

           if(ch->clan != clan_lookup("chosen"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_bloodlust))
        {
                send_to_char ("{.You calm down as the adrenaline leaves your body.{x\n\r",ch);
                affect_strip(ch,gsn_bloodlust);
                return;
        }
        
	{
     AFFECT_DATA af;
		
               
                af.where = TO_AFFECTS;
                af.type = gsn_bloodlust;
                af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

                af.location = APPLY_DAMROLL;
                af.modifier = 15;
                af.duration = -1;
                af.bitvector = 0;
                affect_to_char (ch, &af);
                send_to_char ("{BYou feel empowered as adrenaline pumps through your veins!{x\n\r",ch);
       			act ("{B$n begins to breathe rapidly as adrenaline courses through them!{x", ch, NULL, NULL, TO_ROOM);

   			}
        return;
}

/* Second shinigami clan skill by bree */
void do_vmight(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

        if(ch->clan != clan_lookup("builder"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
           if (is_affected (ch, gsn_vmight))
	{
                send_to_char ("{.The Might of the Vanir fades from your mind.{x\n\r", ch);
                affect_strip(ch,gsn_vmight);
		return;
	}
         send_to_char ("{.You begin to feel the might of the Vanir all around you!{x\n\r", ch);
         act ("{($n begins channeling the Might of the Vanir!{x", ch, NULL, NULL, TO_ROOM);
         act ("{($n's eyes fade to black!{x", ch, NULL, NULL, TO_ROOM);

          

        	af.where    = TO_AFFECTS;
        	af.type     = gsn_vmight;
			af.level    = ch->level;
          	af.duration = -1;
          	af.bitvector = 0;
	  
          	af.location = APPLY_DAMROLL;
          	af.modifier = 5;
          	af.duration = -1;
          	affect_to_char(ch,&af);
          return;
}

void do_phsyco(CHAR_DATA *ch,char *argument) 
{ 
    CHAR_DATA *victim; 
    int dam; 
    char arg[MAX_INPUT_LENGTH]; 
 
     if(ch->clan != clan_lookup("IMPLEMENTORS"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
 
    one_argument(argument,arg); 
    if (arg[0] == '\0') 
        victim = ch->fighting; 
    else 
        victim = get_char_room(ch,arg); 
 
    if (victim == NULL) 
    { 
        send_to_char("They aren't here.\n\r",ch); 
        return; 
    } 
    if (victim == ch) 
    { 
        send_to_char("You can't do that.\n\r",ch); 
        return; 
    }
     
    { 

 
	dam = (ch->level * number_range(300,400));

        act("$n goes starts to go {RPhsycotic!",ch,0,victim,TO_NOTVICT);
        act("$n goes into a {RPhsycotic {DR{Ra{Dg{Re{X!!",ch,0,victim,TO_VICT); 
 
        dam += (get_curr_stat(ch,STAT_DEX)+str_app[get_curr_stat(ch,STAT_STR)].todam)*8; 
        check_improve(ch,gsn_phsyco,TRUE,3); 
     
        
        {
         dam = 2 * dam + (dam * 6 * 5 / 1); //changed from 100
         victim->stunned = 1;
         act ("{)You are stunned, and have trouble getting back up!{x", ch, NULL, victim, TO_VICT);
         act ("{.$N is stunned by the {RRaging Phsyco!{x", ch, NULL, victim, TO_CHAR);
         act ("{($N is having trouble getting back up.{x", ch, NULL, victim, TO_NOTVICT);
        }
        
        
    } 
 
    damage(ch,victim,dam,gsn_phsyco,DAM_BASH,TRUE,0); 


 
    return; 
}
