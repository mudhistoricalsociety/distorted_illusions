/* This file is the Clan-Skills of the PK Clan DarkSide && Renshai */
/* Copyright 2000, Gabe Volker */
/* To be used only on Asgardian Nightmare unless permission is granted */
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

DECLARE_DO_FUN( do_announce );
int focus_dam args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch));
int focus_ac args ((CHAR_DATA *ch));

void do_battle_fury(CHAR_DATA *ch, char *argument)
{
	int chance, hp_percent;
	int sn;

        if ((chance = get_skill (ch, gsn_battle_fury)) == 0
        || !has_skill(ch,gsn_battle_fury))
	{
                send_to_char ("{.You do not know how to battlefury.{x\n\r", ch);
		return;
	}

        if (is_affected (ch, gsn_battle_fury))
	{
                send_to_char ("{.You are already embracing the fury of the DarkSide.{x\n\r", ch);
		return;
	}

	if (IS_AFFECTED (ch, AFF_CALM))
	{
                send_to_char ("{.You're feeling too mellow to battlefury.{x\n\r", ch);
		return;
	}

        if (ch->mana < 100)
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
	chance += 25 - hp_percent / 2;

	if (number_percent () < chance)
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
                ch->mana -= 150;
		ch->move = UMAX(1,ch->move - 50);

		/* heal a little damage */
                ch->hit += ch->level * 4;
		ch->hit = UMIN (ch->hit, ch->max_hit);

                send_to_char ("{.Your eyes invert for a moment as you are consumed by {rfury!{x\n\r", ch);
                act ("{($n gets a {cw{gi{rc{yk{ce{gd{k look in $s eyes.{x", ch, NULL, NULL, TO_ROOM);
                check_improve (ch, gsn_battle_fury, TRUE, 2);

		af.where = TO_AFFECTS;
                af.type = gsn_battle_fury;
		af.level = ch->level;
                af.duration = number_fuzzy (ch->level / 10);
                af.bitvector = 0;

		af.location = APPLY_DAMROLL;
                af.modifier = UMAX (1, ch->level / 4);
		affect_to_char (ch, &af);

                if(!IS_SHIELDED(ch,SHD_STEEL))
                {
				 sn = skill_lookup("steel flesh");
                
				 af.where = TO_SHIELDS;
				 af.type = sn;
				 af.level = ch->level;
				 af.duration = ch->level/2;
                 af.location = APPLY_AC;
				 af.modifier = UMAX (10, 5 * (ch->level / 20)) * (-1);
                 af.bitvector = SHD_STEEL;
                 affect_to_char (ch, &af);
                 
				 send_to_char("Your skin hardens into steel-like material.\n\r",ch);
                }
	}
	else
	{
		WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
		ch->mana -= 25;
		ch->move = UMAX(1,ch->move-75);

                send_to_char ("{.Your pulse speeds up, but nothing happens.{x\n\r", ch);
                check_improve (ch, gsn_battle_fury, FALSE, 2);
	}
}


/* Renshai Clan Skills */

/* BattleCry that channels' godly force at opponents */
void do_modis_anger(CHAR_DATA *ch, char *argument)
{
        
           if(ch->clan != clan_lookup("renshai"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_modis_anger))
	{
                send_to_char ("{.Your anger begins to subside.{x\n\r",ch);
		affect_strip(ch,gsn_modis_anger);
                return;
        }

	if (number_percent () )
	{
		AFFECT_DATA af;

                send_to_char ("{.You release your stored energy and bellow a mighty battlecry!{x\n\r",ch);
                send_to_char ("{.You feel the {!Wrath {xof {DShadows{x enter your body!{x\n\r", ch);
                act ("{($n bellows a mighty battlecry and begins frothing at the mouth.{x", ch, NULL, NULL, TO_ROOM);
                act ("{($n's eyes turn blood red.{x", ch, NULL, NULL, TO_ROOM);

                /* check_improve (ch, gsn_modis_anger, TRUE, 2); */

		af.where = TO_AFFECTS;
                af.type = gsn_modis_anger;
		af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

		af.location = APPLY_DAMROLL;
                af.modifier = 15;
		affect_to_char (ch, &af);
	        
        }

        return;
}

/* Increased Accuracy resulting in Double Strike */
void do_concentration(CHAR_DATA *ch, char *argument)
{
        int chance;

        if ((chance = get_skill (ch, gsn_concentration)) == 0
        || !has_skill(ch,gsn_concentration))
	{
                send_to_char ("{.You know not the way of the Rensahi.{x\n\r", ch);
		return;
	}

        if (is_affected (ch, gsn_concentration))
	{
                send_to_char ("{.You are already concentrating on your accuracy!{x\n\r", ch);
		return;
	}

        if( ch->move < 251 )
        {
         send_to_char("You do not have the stamina to do this. Gather your energy.\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_modis_anger))
        {
                send_to_char("{.Your anger fades, you start to cool down.{x\n\r",ch);
                affect_strip(ch, gsn_modis_anger);
        }

	if (number_percent () < chance)
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
                ch->move -= 250;

                send_to_char ("{.You concentrate on the determination of Modi and your aim improves!{x\n\r", ch);
                act ("{($n's eyes turn light blue with white steaks.{x", ch, NULL, NULL, TO_ROOM);

                /* check_improve (ch, gsn_concentration, TRUE, 2); */

		af.where = TO_AFFECTS;
                af.type = gsn_concentration;
		af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

                af.location = APPLY_HITROLL;
                af.modifier = UMAX (1, ch->level / 3);
		affect_to_char (ch, &af);
	}

        return;

}

void do_calm(CHAR_DATA *ch, char *argument)
{
/* Tien
        if(ch->clan != clan_lookup("renshai"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
*/
        if (is_affected (ch, gsn_concentration))
        {
                send_to_char("{.Your concentration fades, your temperature starts to rise.{x\n\r",ch);
                affect_strip(ch, gsn_concentration);
		return;
        }

        if (is_affected (ch, gsn_modis_anger))
        {
                send_to_char("{.Your anger fades, you start to cool down.{x\n\r",ch);
                affect_strip(ch, gsn_modis_anger);
		return;
 	}
	send_to_char("{.You feel cool and collected.{x\n\r",ch);
        return;
}

void do_faith(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int chance = 0;
    int dam;
    char arg[MSL];

    one_argument (argument, arg);

    if ((chance = get_skill (ch, gsn_blind_faith)) == 0
        || !has_skill(ch,gsn_blind_faith))
    {
         send_to_char ("{!You do not have enough {&faith!{x\n\r", ch);
        return;
    }
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
        send_to_char ("You know how much faith you have!\n\r", ch);
        return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
        act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
        return;
    }

    chance = get_skill(ch,gsn_blind_faith);

    if (number_percent() <= chance)
    {
    	if (ch->pcdata->tier == 1) 
	    	dam = dice( ch->level, 10 );
    	else if (ch->pcdata->tier == 2)
	    	dam = dice( ch->level*2, 12 );
    	else
	    	dam = dice( ch->level*3, 13 );

    	if (ch->mana < 50)
    		ch->hit -= 50;
    	else
    		ch->mana -= 50;
   
    	if (victim != ch)
    	{
    	    act("$n utters words of his faith, suddenly the room erupts in blinding light!\n\r",
    	        ch,NULL,NULL,TO_ROOM);
    	    send_to_char("You call upon your faith and suddenly the room erupts in blinding light.\n\r",ch);
    	}

    	if ( ( ch->fighting == NULL )
    	&& ( !IS_NPC( ch ) )
    	&& ( !IS_NPC( victim ) ) )
    	{
    		ch->attacker = TRUE;
    		victim->attacker = FALSE;
    	}

    	if ( saves_spell( ch->level, victim, DAM_HOLY) )
    	    dam *= 0.75;

    	damage( ch, victim, dam, gsn_blind_faith, DAM_LIGHT , TRUE, 0);
    	spell_blindness(gsn_blindness, 3 * ch->level / 2, ch, (void *) victim,TARGET_CHAR);
    	WAIT_STATE (ch, PULSE_VIOLENCE);
    }
    
    else
    {
	    send_to_char("You lose faith momentarily and feel slightly weaker.\n\r",ch);
	    act("$n utters words of his faith, but falters and nothing happens.\n\r",
	            ch,NULL,NULL,TO_ROOM);    

	    if (ch->mana < 25)
		    ch->hit -= 25;
	    else
		    ch->mana -= 25;

	    WAIT_STATE (ch, PULSE_VIOLENCE);
    }
}

// void do_crit_strike(CHAR_DATA *ch, char *argument)
//{
//      int chance;
//
//       if ((chance = get_skill (ch, gsn_crit_strike)) == 0
//        || !has_skill(ch,gsn_crit_strike))
//	{
//               send_to_char ("{.You know not the way of the Storm.{x\n\r", ch);
//		return;
//	}
//       if (is_affected (ch, gsn_crit_strike))
//	{
//              send_to_char ("{.Your fury begins to subside.{x\n\r", ch);
//		affect_strip(ch, gsn_crit_strike);
//              return;
//        }
//
//      if( ch->mana < 100 )
//    {
//     send_to_char("You do not have the willpower to do this. Gather your energy.\n\r",ch);
//         return;
//        }
//
//       if (is_affected (ch, gsn_crit_strike))
//        {
//               send_to_char("{.Your fury begins to subside.{x\n\r",ch);
//              affect_strip(ch, gsn_crit_strike);
//        }
//
//	if (number_percent () < chance)
//	{
//		AFFECT_DATA af;
//
//		WAIT_STATE (ch, PULSE_VIOLENCE);
//              ch->mana -= 100;
//
//              send_to_char ("{.You focus on crital striking!{x\n\r",ch);
//              send_to_char ("{.You feel the {!Wrath {xof The {&Storm{x enter your body!{x\n\r", ch);
//              act ("{($n concentration increases{x", ch, NULL, NULL, TO_ROOM);
//              act ("{($n's eyes turn into shadows.{x", ch, NULL, NULL, TO_ROOM);
//
//               /* check_improve (ch, gsn_crit_strike, TRUE, 2); */
//
//		af.where = TO_AFFECTS;
//               af.type = gsn_crit_strike;
//		af.level = ch->level;
//              af.duration = -1;
//                af.bitvector = 0;
//
//		af.location = APPLY_DAMROLL;
//               af.modifier = UMAX (1, ch->level / 5);
//		affect_to_char (ch, &af);
//	}
//
//        return;
//}




/** Alliance clanskill removed - Quintalis 9-28-04 **

int alliance_damroll(CHAR_DATA *ch)
{
    int damroll;
    int dam;
    damroll = 0;
    if (IS_NPC(ch)
        || ch->clan != clan_lookup("alliance") )
    return damroll;
    dam = ch->damroll;
    dam += str_app[get_curr_stat(ch,STAT_STR)].todam;
    dam += focus_dam(ch);
    if (IS_CRUSADER(ch))
    dam += (ch->alignment*ch->level)/2000;

	if (ch->class == CLASS_MONK)
	dam += ch->pcdata->power[0]/10+ch->pcdata->power[1]/10;

    if (ch->fighting == NULL)
	damroll = dam/20;
    else if (IS_NPC(ch->fighting))
	damroll = dam/20;
    else if (IN_ARENA(ch))
	damroll = dam/20;
    else if (number_on(ch->clan) == number_on(ch->fighting->clan))
        damroll = dam/8;
    else if (number_on(ch->clan)+1 < number_on(ch->fighting->clan))
	damroll = dam/4;
    else if (number_on(ch->clan) < number_on(ch->fighting->clan))
	damroll = dam/6;
    else
	damroll = dam/20;

    if (damroll < 0)
	damroll = (damroll*damroll)/2;
    return damroll;
}
int alliance_hitroll(CHAR_DATA *ch)
{
    int hitroll;
    int hit;
    hitroll = 0;
    if (IS_NPC(ch)
        || ch->clan != clan_lookup("alliance") )
    return hitroll;
    hit = ch->hitroll;
    hit += str_app[get_curr_stat(ch,STAT_STR)].tohit;
    hit += focus_hit(ch);
    if (IS_CRUSADER(ch))
    hit += (ch->alignment*ch->level)/2000;

    if (ch->fighting == NULL)
	hitroll = hit/20;
    else if (IS_NPC(ch->fighting))
	hitroll = hit/20;
    else if (IN_ARENA(ch))
	hitroll = hit/20;
    else if (number_on(ch->clan) == number_on(ch->fighting->clan))
        hitroll = hit/8;
    else if (number_on(ch->clan)+1 < number_on(ch->fighting->clan))
	hitroll = hit/4;
    else if (number_on(ch->clan) < number_on(ch->fighting->clan))
	hitroll = hit/6;
    else
	hitroll = hit/20;


    if (hitroll < 0)
	hitroll = (hitroll*hitroll)/2;
    return hitroll;
}
int alliance_ac(CHAR_DATA *ch, int type)
{
    int armor;
    int ac;
    armor = 0;
    if (IS_NPC(ch)
        || ch->clan != clan_lookup("alliance") )
    return armor;
    ac = ch->armor[type];
    if (IS_AWAKE(ch))
    ac += dex_app[get_curr_stat(ch,STAT_DEX)].defensive;
    ac += focus_ac(ch);

    if (ch->fighting == NULL)
	armor = ac/20;
    else if (IS_NPC(ch->fighting))
	armor = ac/20;
    else if (IN_ARENA(ch))
	armor = ac/20;
    else if (number_on(ch->clan) == number_on(ch->fighting->clan))
        armor = ac/8;
    else if (number_on(ch->clan)+1 < number_on(ch->fighting->clan))
	armor = ac/4;
    else if (number_on(ch->clan) < number_on(ch->fighting->clan))
	armor = ac/6;
    else
	armor = ac/20;
	
    if(armor > 0)
	armor = 0-armor;
    return armor;
}**/
