/* These are new additions for unused classes in an attempt to make people want to play them.
   These skills/spells are for use in Asgardian Nightmare only, and written by Bree.  */

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
#include "tables.h"

int focus_dam args ((CHAR_DATA *ch)); 

/* 
   Make changes for new spells in:
               Const.c
               Magic.h
               Magtype.c
*/

/* 
   Name: Madness
   Type: Spell - Orange Magic
   Information: This is a modified berserk/frenzy with
                better stats for weaker magic classes.
   Class: Conjurer
*/

void spell_madness(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already raging around in madness.\n\r",ch);
	else
	  act("$N is already raging around in madness.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 2;
    af.modifier  = level / 4;
    af.bitvector = AFF_BERSERK;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 36);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with insane madness!\n\r",victim);
    act("$n gets an insane look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* 
   Name: Fireward
   Type: Spell - Red Magic
   Information: Add's resist fire to a player.
   Class: Voodan
*/

void spell_fireward( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already resisting the flames.\n\r",ch); 
	else
	  act("$N begins to ward away the flames.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_FIRE;
    affect_to_char( victim, &af );
    
    act( "$n looks stronger against the flame.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin warding off the flames.\n\r", victim );
 
   return;

}

/* 
   Name: Divine Intervention
   Type: Spell - White
   Information: 
   Class: Paladin 
*/

void spell_interven( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already channeling your divine intervention.\n\r",ch); 
	else
	  act("$N glows white and suddenly looks more divine!.",ch,NULL,victim,TO_CHAR);
	return;
    }

                af.where = TO_AFFECTS;
                af.type = sn;
                af.level = ch->level;
                af.duration = level/3.5;
                af.bitvector = 0;

                af.location = APPLY_DAMROLL;
                af.modifier = 10;
                af.duration = level/3.5;
                af.bitvector = 0;
                affect_to_char(ch,&af);

                af.location = APPLY_REGEN;
                af.modifier = 25;
                af.duration = level/3.5;
                affect_to_char(ch,&af);
                af.location = APPLY_HIT;
                af.modifier = 100;
                af.duration = level/3.5;

                affect_to_char(ch,&af);

                
    act( "$n prays for divine intervention!", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You gain the help of your god!\n\r", victim );

 
   return;

}

/* 
   Name: acid ward
   Type: Spell - Red Magic
   Information: Add's resist acid to a player.
   Class: Alchemist
*/

void spell_acidward( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already resisting acid.\n\r",ch); 
	else
	  act("$N begins to ward away acid.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_ACID;
    affect_to_char( victim, &af );
    
    act( "$n looks stronger against acid.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin warding off acid.\n\r", victim );
 
   return;

}

/* 
   Name: ice ward
   Type: Spell - Blue Magic
   Information: Add's resist ice to a player.
   Class: Warlock
*/

void spell_iceward( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already resisting ice.\n\r",ch); 
	else
	  act("$N begins to ward away ice.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_COLD;
    affect_to_char( victim, &af );
    
    act( "$n looks stronger against ice.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You begin warding off ice.\n\r", victim );
 
   return;
}
//Voodan sense vitality by fesdor, works with do_sense.
void spell_sense_vitality( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;    
    int chance = 100;
    
    chance -= (get_curr_stat(victim, STAT_CON)*3);
    chance += (get_curr_stat(ch, STAT_CON)*2);
    
    if ( victim == ch )
    {
       send_to_char("You can't establish a link with yourself!\n\r",ch);
       return;
    }
    
    if (IS_NPC(victim))
    {
	    send_to_char("You can only use this spell on players.\n\r",ch);   
	    return;
    }
    
    if ( ( ch->fighting == NULL ) && ( !IS_NPC( ch ) ) && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    } 
    
    if (is_affected( victim , gsn_sense ))
    {
	    send_to_char("Someone is already linked to that person.\n\r",ch);
	    return;
    }
 
    else if ( is_affected( ch , gsn_sense)  )
    {
       send_to_char("You are already attuned to someone's vitals!\n\r",ch);
       return;            
    }
    else if (number_percent() < chance)
    {

       af.where            = TO_AFFECTS;
       af.type             = gsn_sense;
       af.level            = ch->level;
       af.duration         = ch->level/5;
       af.location         = APPLY_REGEN;
       af.modifier         = victim->hit/100;
       af.bitvector        = 0; 
    
       affect_to_char(ch, &af);
    
       af.modifier         = 0;
    
       affect_to_char(victim, &af);
    
       send_to_char("{cYou feel a chill as some strange force invades your very soul!{x\n\r",victim);
       send_to_char("{RYou feel a second pulse in your fingertips as you become attuned to their vitals!{x\n\r",ch);
       return;
    }
    else
    {
       send_to_char("{CYou feel slightly disturbed but it fades.{x\n\r",victim);
       send_to_char("{rYour heart begins to beat for two, but the feeling subsides.{x\n\r",ch);
    return;
	}
	return;
}
