/* These are skills and spells for newly made classes, such as spartan. 
 * All code written by Bree for the exclusive use in Distorted Illusions/Asgardian Nightmare.
 * This code may not be used without permission on any other MUD. */

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
//#include "tables.h"

/* Local Functions Needed */

int focus_dam args ((CHAR_DATA *ch)); 
int focus_hit args ((CHAR_DATA *ch));
bool can_bypass args ((CHAR_DATA * ch, CHAR_DATA * victim));


/* 
  Class: Spartan
  Name: Spartan Shield Bash
  Info: Allows a Spartan to rush a target and bash them with their shield.
  Suggested level: 40
  Extra: Will have a chance to stun.
  Requirements: Must be wearing a shield
  
*/

void do_spartan_bash(CHAR_DATA *ch,char *argument) 
{ 
    CHAR_DATA *victim; 
    int dam = 0, chance, percent, hroll, hrbonus = 0, acbonus = 0, acb; 
    char arg[MAX_INPUT_LENGTH]; 
    one_argument(argument,arg);
    bool SHIELD = FALSE; 
    OBJ_DATA *shieldd;
    int dexbonus = 0;
    
    if ( (chance = get_skill(ch,gsn_spartan_bash)) == 0 
    || !has_skill(ch,gsn_spartan_bash)) 
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
        send_to_char("Cute..shield bash yourself.\n\r",ch); 
        return; 
    }
    
    if (ch->stunned > 0)
    {
	send_to_char ("You're still a little woozy.\n\r", ch);
	return;
    } 
    
    /* Check for wearing a shield */

      shieldd = get_eq_char(ch,WEAR_SHIELD); 
       
      if (shieldd == NULL) 
         
        {     
          send_to_char("You must be wearing a shield to shield bash!.\n\r",ch); 
          return; 
         } 
         else 
         { 
         SHIELD = TRUE; 
         } 
     /* End check for wearing a shield */  
   
        if (is_safe(ch,victim) ) 
        return;
          
    /* If it is a mob give a slightly higher chance bonus */
   if(IS_NPC(victim)) {
	   chance += (ch->level - (victim->level - 3 * 2.5));
	   chance -= get_curr_stat(victim,STAT_DEX)*2/5	; }
   /* If it is a player give a more normal chance bonus*/
	   else {
   	   chance += (ch->level - victim->level * 4); 
       chance -= get_curr_stat(victim,STAT_DEX) + 5;  }
    
    /* Attacker's dexterity */   
    chance += get_curr_stat(ch,STAT_DEX)*3/4; 
    chance = URANGE(5, chance, 90); 
    
    
    if (number_percent() > chance) 
    { 
        dam = 0; 
        check_improve(ch,gsn_spartan_bash,FALSE,3); 
    } 
    
    else 
    { 
        
 /* Less Armor Class Bonus Here (less armor, like a spartan, = bigger dam)  */  
	    acb = ch->armor[2];
	    
	    if ((acb) <= -850)
	      
	    acbonus = number_range (5,12)*2; 
	    
            
/* End Armor Class Bonus Here */

      if (get_curr_stat(ch,STAT_DEX) >=26)
          dexbonus = number_range (2, 6) * 2;
          dam += dexbonus;

	    
	    /* Hitroll bonus damage here */
	    hroll = ch->hitroll; 
 
       if ((hroll) >= 210)
        
       hrbonus = number_range (3,9)*1.5;
       
       if ((hroll) >= 310)
      
       hrbonus = number_range (2,12)*1.9;
     
       //else taken out
       
 /* End hitroll damage bonus here */
	
        dam = hrbonus + acbonus + (ch->level * number_range(6,22));

        act("$n darts at $N! and hit's $N in the face with a shield!",ch,0,victim,TO_NOTVICT);
        act("You dart at $N, striking $N in the face with your shield!",ch,0,victim,TO_CHAR); 
        act("$n rushes at you and strikes you in the face with a shield!",ch,0,victim,TO_VICT); 
 
        dam += (get_curr_stat(ch,STAT_DEX)+str_app[get_curr_stat(ch,STAT_STR)].todam)*8.5; //Was *8
        check_improve(ch,gsn_spartan_bash,TRUE,3); 
        WAIT_STATE(victim,4*PULSE_VIOLENCE);// +2 on the wait lag.
        /* Bonus */
        if ((percent = number_percent()) <= (get_skill(ch,gsn_spartan_bash) / 2.3))
        {
         dam = 2 * dam + (dam * 7 * percent / 50);
         victim->stunned = 1;
         act ("{)You are stunned, and have trouble getting back up!{x", ch, NULL, victim, TO_VICT);
         act ("{.$N is stunned by a shield bash!!{x", ch, NULL, victim, TO_CHAR);
         act ("{($N is having trouble getting back up.{x", ch, NULL, victim, TO_NOTVICT);
        }
               
    } 
 
    damage(ch,victim,dam,gsn_spartan_bash,DAM_BASH,TRUE,0); 
    WAIT_STATE(ch,4*PULSE_VIOLENCE); 

    return; 

} 

/* 
  Class: Spartan
  Name: The Spirit of Sparta
  Info: Allows a Spartan to channel the spirit of Sparta
  Suggested level: 75
  Extra: Gives 30 regen and 15 Hit/Dam for ch->level/3
  Requirements: Must be a Spartan
  
*/
 
void do_spirit_sparta(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_spirit_sparta) <= 0)
        || (ch->level < skill_table[gsn_spirit_sparta].skill_level[ch->class] ) )
    {
        send_to_char("You are not of Sparta!\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_spirit_sparta) )
    {
        send_to_char("The Spirit of Sparta already flows within you.\n\r",ch);
	return;
    }

    if (ch->mana < 200 || ch->move < 200)
    {
        send_to_char("You do not have enough mental power summon the Spirit of Sparta.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_spirit_sparta))
    {
        send_to_char("You concentrate on channeling the Spirit of Sparta, but you lose your concentration.\n\r",ch);
        check_improve(ch,gsn_spirit_sparta,FALSE,1);
        ch->mana -= 100;
        ch->move -= 100;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_spirit_sparta;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.modifier = 15;
    af.duration = ch->level/3;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.location = APPLY_REGEN;
    af.modifier = 30;
    af.duration = ch->level/3;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    
    
    ch->mana -= 200;
    ch->move -= 200;

    act("The Spirit of Sparta flows within $n!",ch,NULL,NULL,TO_ROOM);
    send_to_char("The Spirit of Sparta flows within you!\n\r",ch);
    check_improve(ch,gsn_spirit_sparta,TRUE,1);
 
   return;
}

/* 
  Class: Spartan
  Name Spear Jab
  Info: Jabs at a victim from behind their spear lightning fast. 
  Suggested level: 50
  Extra: Chance to Blind.
  Requirements: Must be wearing a Spear.
  
*/

void do_spear_jab( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam, hrbonus = 0, hroll = 0; 
        char arg[MAX_INPUT_LENGTH]; 
        OBJ_DATA *wield,*second; 
        int multiplier; 
        bool DUAL = FALSE; 
         
        dam = 0; 
        one_argument(argument,arg); 
        if ((chance = get_skill(ch,gsn_spear_jab) ) == 0 
        || !has_skill(ch,gsn_spear_jab) ) 
        { 
        send_to_char("Spear jabbing? What's that?\n\r",ch); 
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
        send_to_char("You can't jab at yourself!\n\r",ch); 
        return; 
        }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt a spear jab, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts a spear jab, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
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
        || (( wield->value[0] != WEAPON_POLEARM) 
        && ( wield->value[0] != WEAPON_SPEAR) ))

        { 
         if ((second == NULL) 
         || (( second->value[0] != WEAPON_POLEARM) 
         && ( second->value[0] != WEAPON_SPEAR) ))
       
         { 
          send_to_char("You must be wielding a spear or polearm to jab.\n\r",ch); 
          return; 
         } 
         else 
         { 
         DUAL = TRUE; 
         } 
        } 
 
        if (is_safe(ch,victim) ) 
        return; 
 
        chance += ch->carry_weight/23; 
        chance -= victim->carry_weight/18; 
        chance += (ch->size - victim->size)*21; 
        chance -= get_curr_stat(victim,STAT_DEX); 
        chance += get_curr_stat(ch,STAT_STR); 
        chance += get_curr_stat(ch,STAT_DEX); 
        if (IS_AFFECTED(ch,AFF_HASTE) ) 
                chance += 15; 
        if (IS_AFFECTED(victim,AFF_HASTE) ) 
                chance -= 9; 
        chance += (ch->level - victim->level)*2/3;

        act("$n's spear jabs out from infront of their shield!",ch,0,victim,TO_NOTVICT); 
        act("Your spear jabs out and you strike at your victim!",ch,0,victim,TO_CHAR); 
        act("$n's spear jabs out from infront of their shield, striking $N!",ch,0,victim,TO_VICT); 
        
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_spear_jab,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_lunge].beats); 
 
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
           dam += dam * (number_range(50,100)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100; 
         } 
        } 
 
        /* Hitroll bonus damage here */
	    hroll = ch->hitroll; 
 
       if ((hroll) >= 210)
        
       hrbonus = number_range (3,9)*1.5;
       
       if ((hroll) >= 310)
      
       hrbonus = number_range (2,13)*2;
        dam += hrbonus;
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_spear_jab]; 
        dam /= 100; 
        multiplier = number_range(ch->level/8, ch->level/4); 
        multiplier /= 10; 
        multiplier += 5/4; 
        dam *= multiplier;
        if(!DUAL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
         dam *= 1.5;
 
        if (dam <= 0) 
                dam = 1; 
        damage(ch,victim,dam,gsn_spear_jab,DAM_PIERCE,TRUE,0); 
        } 
        else 
        { 
        damage(ch,victim,dam,gsn_spear_jab,DAM_PIERCE,TRUE,0); 
        check_improve(ch,gsn_spear_jab,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_spear_jab].beats); 
        } 
        return; 
} 


/* 
  Class: Spartan
  Name Shield Wall
  Info: Forms a Shield Wall with their shield.
  Suggested level: 25
  Extra: Like the shield block skill sort of.
  Requirements: Must be wearing a shield.
  
*/

bool check_shield_wall (CHAR_DATA * ch, CHAR_DATA * victim)
{
        int chance, dam;

	if (!IS_AWAKE (victim))
		return FALSE;

	
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

        chance = get_skill (victim, gsn_shield_wall)*2/3;

        if(IS_SET(victim->stance_aff,STANCE_AFF_ENHANCED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

        if(IS_SET(victim->stance_aff,STANCE_AFF_IMPROVED_BLOCK) &&
         number_percent() < victim->stance[victim->stance[0]] * 0.5)
          chance += 15;

    	chance -= (GET_HITROLL (ch) * 0.15);
	chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 2;
	chance -= (get_curr_stat (ch, STAT_STR) - get_curr_stat (victim, STAT_STR)) * 2;

	if(!IS_NPC(ch))
		chance += victim->level - ch->level*1.5;
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
                 act("$n slides past your shield wall and impales you!",ch,NULL,victim,TO_VICT);
                 act("You counter $N's shield wall and impale $M.",ch,NULL,victim,TO_CHAR);
                 act("$n deftly dances past $N's shield wall and impales $N.",ch,NULL,victim,TO_NOTVICT);
                 damage(ch,victim,number_range(dam,(victim->level*7) ), gsn_coup_de_coup, DAM_PIERCE, TRUE, 0);
		 update_pos(victim);
		 check_improve(ch,gsn_coup_de_coup,TRUE,2);
		 return FALSE;
            }
            else
            {
             check_improve(ch,gsn_coup_de_coup,FALSE,3);
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("){You block $n's attack with your shield wall.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N raises his shield into a wall to block your strike!.{x", ch, NULL, victim,
		 TO_CHAR);
	check_improve (victim, gsn_shield_wall, TRUE, 6);
             return TRUE;
            }
         }

        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)You block $n's attack with your shield wall.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.You block $n's attack with your shield wall.{x", ch, NULL, victim,
		 TO_CHAR);
	check_improve (victim, gsn_shield_wall, TRUE, 6);
	return TRUE;
}


/* 
  Class: Spartan
  Name Rallying Cry
  Info: Rally's the Spartan's group, giving them bonus stats.
  Suggested level: 79
  Extra: Basically it's like Lead for Spartans with a few differances.
  Requirements: None
  
*/


void do_rally ( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int count= 0;
	int members = 0;
	int effect = 0;
	int chance = 0;

	if (is_affected (ch, gsn_rally) || ch->move < 500 || ch->mana < 300)
	{
		send_to_char("You do not feel the strength within you to bellow a rallying cry.\n\r",ch);
		return;
	}

	chance = get_skill(ch,gsn_rally);


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
		send_to_char ("Who are you trying to rally behind you?\n\r", ch);
		return;
	}
	

	
	effect = count / members + 200;

	// reset count to 0 for next step
	count = 0;
	members++;

	// add chance of failure/winning
	effect -= 3*number_percent();
	effect += 10*number_percent();

	effect = effect * chance / 150;

	if (effect < 0)
	{
		send_to_char("You try to bellow a rallying cry but can only manage a whisper.\n\r",ch);
		act("$n begins to bellow a rallying cry but gets drowned out by the bird's chirping..haha.",ch,NULL,gch,TO_NOTVICT);
		count = 0;
	}

	else if(effect < 750)
	{
		send_to_char("Moving with confidence you lift your voice in a roaring rallying cry!.\n\r",ch);
		act("$n stands with confidence and bellows out a powerful rallying cry!",ch,NULL,gch,TO_NOTVICT);
		count = 3;
	}
	else if(effect < 1000)
	{
		send_to_char("With ease, you call for men to rally behind you!.\n\r",ch);
		act("With ease, $n calls in passionate and powerful words for war!",ch,NULL,gch,TO_NOTVICT);
		count = 4;
	}
	else
	{
		send_to_char("You {RBELLOW{x out a rallying cry, {Rinspiring men to fight until death or victory!!{X\n\r",ch);
		act("Infused with power and conviction $n {RBELLOWS{x out a {Rterrifying{x rallying cry!!",ch,NULL,gch,TO_NOTVICT);
		count = 5;
	}

	af.where = TO_AFFECTS;
	af.type = gsn_rally;
	af.level = ch->level;
	af.bitvector = 0;
	af.duration = 25;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group (gch, ch) || IS_NPC (gch))
			continue;
		// messages for the individual need to be moved into the loop
		
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
			send_to_char("Waves of euphoria and adrenaline wash over you, urging you to fight like you've never fought before!\n\r",gch);
			break;
			default:
			send_to_char("Do_lead: count. - Inform an immortal.\n\r",gch);
		}
		af.modifier = number_range(4,14) * effect / 1000;
		af.location = APPLY_CON;
		affect_to_char(gch,&af);
		af.location = APPLY_STR;
		affect_to_char(gch,&af);
		af.location = APPLY_WIS;
		affect_to_char(gch,&af);
		af.location = APPLY_DEX;
		affect_to_char(gch,&af);
		af.modifier = count * 6;
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
	check_improve(ch, gsn_rally, TRUE, 2);
	WAIT_STATE (ch, skill_table[gsn_rally].beats);

	return;
}
/******************************** 
 * End of Spartan Spells/Skills *
 ********************************/
 
/****************************************************************************************************/
 
/************************************* 
 *Start of BladeDancer Skills/Spells*
 *************************************/

/* 
  Class: BladeDancer
  Name: Ancient Dance and it's subfunction dancer_heal  
  Info: Mass Heal skill.
  Suggested level: 80
  Extra: Heals hps and mana of others in the room, including self, cost MOV to use.
  Requirements: Movement Points
*/
void spell_dancer_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;


    victim->hit = UMIN( victim->hit + 200, victim->max_hit );
    victim->mana = UMIN( victim->mana + 200, victim->max_mana ); 
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    send_to_char( "A fuzzy feeling flows through your veins.\n\r", victim );
   
    
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}
void spell_ancient_dance(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num;
  
    if (ch->move >= 275)
    {
    
       heal_num = skill_lookup("heal");
     for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
     {
       if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	  {
	    spell_dancer_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
          ch->move = ( ch->move - 270); 
           update_pos( ch );
	}
	
    }
   }
    else

    send_to_char( "You do not have enough movement to do this dance.\n\r", ch );
      return;
}


/* 
  Class: BladeDancer
  Name: Blood Dance  
  Info: Combination upgraded Haste/Frenzy
  Suggested level: 95
  Extra: Give haste for 101 ticks and an upgraded frenzy for less ticks.
  Requirements: Mana
*/

void do_blood_dance(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_blood_dance) <= 0)
        || (ch->level < skill_table[gsn_blood_dance].skill_level[ch->class] ) )
    {
        send_to_char("Do what now?\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_blood_dance) )
    {
        send_to_char("You must still recover from your last Blood Dance!\n\r",ch);
	return;
    }

    if (ch->mana < 2 || ch->move < 500)
    {
        send_to_char("You do not have enough energy to preform this ritual.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_blood_dance))
    {
        send_to_char("You move around in a large circle dancing furiously, but nothing seems to happen.\n\r",ch);
        check_improve(ch,gsn_blood_dance,FALSE,1);
        ch->mana -= 100;
        ch->move -= 100;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_blood_dance;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.modifier = ch->level*get_skill(ch,gsn_blood_dance)/230;
    af.duration = ch->level/5;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.location = APPLY_DEX;
    af.modifier = 6;
    af.duration = ch->level/5;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    
    
    ch->mana -= 2;
    ch->move -= 500;

    act("$n's dances furiously around in a circle, and {Rblood{x starts to pour from their eyes!.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You start to dance around furiously and your eyes start to bleed!.\n\r",ch);
    check_improve(ch,gsn_blood_dance,TRUE,1);
 
   return;
}

/* 
  Class: BladeDancer
  Name: BladeDance
  Info: Like thieve's circle skill, but has a chance to "surge".
  Suggested level: 45
  Extra: 'Dance' is the keyword for use in combat.
  Requirements: Dagger/Sword/Axe (Edged Weapons for slicing)
*/
void do_bladedance (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int hroll = 0;
	int dam = 0;
	if (get_skill (ch, gsn_bladedance) == 0
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_bladedance].skill_level[ch->class]))
	{
		send_to_char ("You are not adept enough to bladedance\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char ("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
	{
                send_to_char ("You need to wield a primary weapon to bladedance.\n\r", ch);
		return;
	}

        if(IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
        {
         send_to_char("You cannot bladedance with a two-handed weapon!\n\r",ch);
         return;
        }

	
	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 10)) {
		act("You attempt to bladedance, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n sneaks attempts to bladedance, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

	if (ch->stunned)
	{
		send_to_char ("You're still a little woozy.\n\r", ch);
		return;
	}

	if (!can_see (ch, victim))
	{
		
		//Maybe make it not a total lose, give them a chance for 1 or two hits of lesser damage??
		send_to_char ("You stumble blindly into a wall.\n\r", ch);
		return;
	}

	if (ch->move < 25)
	{
                send_to_char ("You're too tired for that.\n\r", ch);
                return;
        }
        
        if (ch->hitroll >= 250)
        
        hroll = ch->hitroll * 3/2;
        dam = hroll;
        
        if (ch->hitroll >= 350)
        hroll = ch->hitroll * 4/2.2;
        dam = hroll;
        
        
        

	WAIT_STATE (ch, 4*PULSE_VIOLENCE);
	if (number_percent () < get_skill (ch, gsn_bladedance)
		|| (get_skill (ch, gsn_bladedance) >= 2 && !IS_AWAKE (victim)))
	{
		check_improve (ch, gsn_bladedance, TRUE, 1);
                act ("{)$n dances insanely fast around you, slicing you with lightning quickness!{x", ch, NULL, victim, TO_VICT);
                act ("{.You dance insanely fast around $N, slicing him with lightning quickness!{x", ch, NULL, victim, TO_CHAR);
                act ("{($n dances insanely fast around $N, slicing him with lightning quickness!.{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 35;
		multi_hit (ch, victim, gsn_bladedance);
	}
	else
	{
		check_improve (ch, gsn_bladedance, FALSE, 1);
                act ("{)$n tries to dance around and strike you but misses!{x", ch, NULL, victim, TO_VICT);
                act ("{.$N foils your attempt to dance around and slice $M.{x", ch, NULL, victim, TO_CHAR);
                act ("{($n fails to quickly dance around $N and slice them!{x", ch, NULL, victim, TO_NOTVICT);
		ch->move -= 20;
		damage (ch, victim, 0 + dam, gsn_bladedance, DAM_NONE, TRUE, 0);

		}


	return;
}
 
 /* 
  Class: BladeDancer
  Name: shadow dance
  Info: conceals the dancer in intense shadows
  Suggested level: 90
  Extra: The person shows up as a shadowy figure in where inside the mud.
         This dance is a perm affect that can be removed by doing the dance
         again.
  Requirements: Mov/Mana
*/
 
void do_shadow_dance(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

  if (get_skill (ch, gsn_shadow_dance) == 0
		|| (!IS_NPC (ch)
			&& ch->level < skill_table[gsn_shadow_dance].skill_level[ch->class]))
	{
		send_to_char ("You are not adept enough to perform this dance\n\r", ch);
		return;
	}
	
	if (ch->move < 750)
	     {
		       send_to_char ("You do not have the energy to preform this dance.\n\r", ch);
	           return;
         }
    
	
	    if (ch->morph_form[0] == MORPH_CONCEAL3) //Conceal 3 is the MOPRH_FORM for shadow dance.
        {
            if(is_affected(ch, gsn_shadow_dance))
            {
                affect_strip(ch,gsn_shadow_dance);
                /* Incase the affect_strip doesn't work */
                if(ch->morph_form[0] > 0)
                 ch->morph_form[0] = 0;
	    }
        
	        send_to_char("You feel the {Ds{wh{Wa{wd{Do{ww{Ds{x begining to leave you.\n\r",ch); 
	    return;
	}
        else if (number_percent() < get_skill(ch,gsn_shadow_dance))
	{
	  
		 af.where    = TO_AFFECTS;
         af.type     = gsn_shadow_dance;
	      af.level    = ch->level;
          af.location = APPLY_MORPH_FORM;
          af.modifier = MORPH_CONCEAL3;  //conceal 3 is the MORPH_FORM for shadow dance.
	      af.bitvector = AFF_SNEAK;
	      af.duration = -1;
	     affect_to_char( ch, &af );

          af.bitvector = 0;
          af.location = APPLY_HITROLL;
          af.modifier = number_range((2+ch->level/20),16);
          affect_to_char(ch,&af);
          af.location = APPLY_DEX;
          af.modifier = 2;
          affect_to_char(ch,&af);   

          check_improve(ch,gsn_shadow_dance,TRUE,6);
          ch->move -= 750;
          send_to_char("You feel the {Ds{wh{Wa{wd{Do{ww{Ds{x begining to envelope you.\n\r",ch);
	  return;
	}
	else
	{
          check_improve(ch,gsn_conceal2, FALSE,3);
          send_to_char("You feel the {Ds{wh{Wa{wd{Do{ww{Ds{x begining to leave you.\n\r",ch);
	  return;
	}

   return;

}
/*********************************
 * End Blade Dancer skills/spells*
 *********************************/
 
 /***********************************************************************************************************/
 
 /* ****************************
  * Start Avenger Skills/Spells*
  ******************************/
 
 

/* 
  Class: Avenger
  Name: 
  Info: 
  Suggested level: 
  Extra: 
  Requirements:
*/ 
  


  
  
  
  
  
  
 /****************************
  * End Avenger Skills/Spells*
  ****************************/ 
  

/* REAVER EVIL ALIGNED CLERIC CLASS*/  
  
  
  
/***********************************************************************************************************/
 

 
/*ideas: Blood Ritual = mana shield or something like diff stasts?, Boiling Blood = haste/frenzy put together
         Blood Strike? = Strong attack spells that drain victim's mana, like ALOT of mana and/or mov.      
*/
/* Start of Blood Magus Spells/Skills */


/* 
  Class: Blood Magus
  Name: Blood Shield
  Info: Trades small -regeneration rate for hp gain/armor gain
  Suggested level: 95
  Extra: None
  Requirements: Hp/Mana/Mov
*/

void spell_blood_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ))
    {
        send_to_char("You are unable to preform this blood ritual so soon.\n\r",ch); 
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_HIT;
    af.modifier  = ch->level * 6.2;
    af.bitvector = 0;
    affect_to_char( ch, &af );
    af.location = APPLY_AC;
    af.modifier  = -75;
   // af.duration  = ch->level * 7.5;
    affect_to_char( ch, &af );
    
    af.location  = APPLY_MANA;
    af.modifier  = ch->level * -13;
    affect_to_char( ch, &af );
    af.location  = APPLY_REGEN;
    af.modifier  = -14;
    affect_to_char( ch, &af );

    act( "$n 's eyes flood {Rred{x and a shimmering {Rred{x aura appears suddenly!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You feel drained of blood, but feel well defended.\n\r", ch );
    return;
}


/* 
  Class: Blood Magus
  Name: Blood Phase
  Info: Magical parry passive skill
  Suggested level: 30
  Extra: None
  Requirements: Hp/Mana
*/

bool check_blood(CHAR_DATA * ch, CHAR_DATA * victim, bool silent)
{
       int chance;

       if (!IS_AWAKE (victim))
               return FALSE;

        if(IS_NPC(victim) && !IS_SET(victim->off_flags,OFF_PHASE))
               return FALSE;

       chance = get_skill (victim, gsn_blood_phase) / 2;

       if(chance == 0) 
        return FALSE;

       if (!can_see (victim, ch))
               chance /= 1.5;

       chance -= (GET_HITROLL (ch) * 0.1);
       
       /*Hitroll bonus */
       if (ch->hitroll >= 275)
          chance += (ch->hitroll)/3.5;
          
       if (number_percent () >= chance + (victim->level -
        ch->level)/2)
               return FALSE;

       if (victim->stunned)
               return FALSE;

       victim->mana -= 5;

     if(!silent)
     {
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
       act ("{)You phase transparent to{Rred{x and absorb $n's attack!{x", ch, NULL, victim, TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
       act ("{.$N phases transparent then {Rred{x and absorbs your attack!{x", ch, NULL, victim, TO_CHAR);
     }
       check_improve (victim, gsn_blood_phase, TRUE, 6);
       return TRUE;
}





/* End of Blood Magus Spells/Skills */
