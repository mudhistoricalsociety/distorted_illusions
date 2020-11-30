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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_handle        );
DECLARE_DO_FUN(do_side_kick     );
DECLARE_DO_FUN(do_spin_kick     );
DECLARE_DO_FUN(do_blindness_dust);
DECLARE_DO_FUN(do_will_of_iron  );
//DECLARE_DO_FUN(do_spinkick      );
DECLARE_DO_FUN(do_purify_body  );
DECLARE_DO_FUN(do_healing_trance  );
DECLARE_DO_FUN(do_yell  );
DECLARE_DO_FUN(do_rest  );
DECLARE_DO_FUN(do_sleep  );

int handling_improve args((CHAR_DATA *ch, sh_int sn));
void check_ground_stomp args((CHAR_DATA *ch,CHAR_DATA *victim,int chance,int dam));
void check_follow_through args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));
bool check_dispel args(( int dis_level, CHAR_DATA *victim, int sn));
bool is_safe_quiet args ((CHAR_DATA * ch, CHAR_DATA * victim));
void improve_toughness args ((CHAR_DATA *ch));

/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }
	
    strcpy(arg2,argument);
    argument = one_argument(argument,arg);
    strcpy(arg1,argument);

    if (arg[0] == '\0')
    {
        do_say(trainer,"{_Pardon me?{x");
	return;
    }

    if (!str_prefix(arg,"list"))
    {
	int col;
	

	col = 0;

        sprintf(buf, "%-18s %-5s %-18s %-5s %-18s %-5s\n\r",
                     "skill","cost","skill","cost","skill","cost");
        send_to_char(buf,ch);
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
 
            if (!ch->pcdata->learned[sn]
            &&  skill_table[sn].skill_level[ch->class] <= LEVEL_HERO)
            {
                sprintf(buf,"{&%-18s {8%-5d{x ",
                    skill_table[sn].name,skill_table[sn].rating[ch->class]);
                send_to_char(buf,ch);
                if (++col % 3 == 0)
                    send_to_char("\n\r",ch);
            }
        }
        if (col % 3 != 0)
            send_to_char("\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"convert"))
    {
	if (ch->practice < 6)
	{
            act("$N tells you 'You are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}
	if (!str_cmp(arg1,"all"))
	{
		act("$N converts all your practices to trains.",ch,NULL,trainer,TO_CHAR);
		while(ch->practice >= 6)
		{	//Do it.  All your practice are belong to train.
			ch->practice -= 6;
			ch->train += 1;
		}
		return;
	}
	else
	{
		act("$N helps you apply your practice to training",
		ch,NULL,trainer,TO_CHAR);
		ch->practice -= 6;
		ch->train +=1 ;
		return;
	}
	}

    if (!str_prefix(arg,"study"))
    {
	if (ch->train < 1)
	{
            act("$N tells you 'You are not yet ready.{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
    }
	if (!str_cmp(arg1,"all"))
	{
		act("$N converts all your trains to practices.",ch,NULL,trainer,TO_CHAR);
		while(ch->train > 0)
		{	//Do it.  All your trains are belong to practice.
			ch->practice += 6;
			ch->train -= 1;
		}
		return;
	}
	else
	{
        act("$N helps you apply your training to practice",
                ch,NULL,trainer,TO_CHAR);
        ch->train -= 1;
	ch->practice += 6;
	return;
    }
}

    if (!str_prefix(arg,"points"))
    {
	if (ch->train < 2)
	{
            act("$N tells you '{_You are not yet ready.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->points <= 40)
	{
            act("$N tells you '{_There would be no point in that.{x'",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("$N trains you, and you feel more at ease with your skills.",
	    ch,NULL,trainer,TO_CHAR);

	ch->train -= 2;
	ch->pcdata->points -= 1;
	ch->exp = (long)exp_per_level(ch,ch->pcdata->points) * ch->level;
	return;
    }

    /* else add a group/skill */

    sn = skill_lookup(arg2);

    if (sn == skill_lookup("conceal"))
    {
      send_to_char("You cannot gain that.\n\r",ch);
      return;
    }

    if (sn > -1)
    {	    

        if (ch->pcdata->learned[sn])
        {
            act("$N tells you '{_You already know that skill!{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        if (skill_table[sn].skill_level[ch->class] > LEVEL_HERO)
        {
            act("$N tells you '{_That skill is beyond your powers.{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        if (ch->train < skill_table[sn].rating[ch->class])
        {
            act("$N tells you '{_You are not yet ready for that skill.{x'",
                ch,NULL,trainer,TO_CHAR);
            return;
        }
 
        /* add the skill */
	ch->pcdata->learned[sn] = 1;
        ch->pcdata->learnlvl[sn] = skill_table[sn].skill_level[ch->class];
        act("$N trains you in the art of $t",
            ch,skill_table[sn].name,trainer,TO_CHAR);
        ch->train -= skill_table[sn].rating[ch->class];
        return;
    }

    act("$N tells you '{_I do not understand...{x'",ch,NULL,trainer,TO_CHAR);
}
    



/* RT spells and skills show the players spells (or skills) */
void do_garrote( CHAR_DATA *ch, char *argument ) 
{ 
char arg[MAX_INPUT_LENGTH]; 
CHAR_DATA *victim; 
int chance; 

one_argument( argument, arg ); 

if ( (chance = get_skill(ch,gsn_garrote)) == 0 
|| (!IS_NPC(ch) 
&& ch->level < skill_table[gsn_garrote].skill_level[ch->class])) 
{ 
send_to_char("Garrote? What's that?{x\n\r",ch); 
return; 
} 

if (arg[0] == '\0') 
{ 
send_to_char("Garrote whom?\n\r",ch); 
return; 
} 

if (ch->fighting != NULL) 
{ 
send_to_char("{hYou're facing the wrong end.{x\n\r",ch); 
return; 
} 

else if ((victim = get_char_room(ch,arg)) == NULL) 
{ 
send_to_char("They aren't here.\n\r",ch); 
return; 
} 

if ( victim == ch ) 
{ 
send_to_char( "How can you sneak up on yourself?\n\r", ch ); 
return; 
} 

if ( is_safe( ch, victim ) ) 
return; 

if (IS_NPC(victim) && 
victim->fighting != NULL && 
!is_same_group(ch,victim->fighting) 
&& !IS_SET(victim->in_room->room_flags, ROOM_ARENA)) 
{ 
send_to_char("Kill stealing is not permitted.\n\r",ch); 
return; 
} 
if ( victim->hit < victim->max_hit / 4) 
{
	act( "$N is hurt and suspicious ... you can't sneak up.", 
ch, NULL, victim, TO_CHAR ); 
return; 
} 

if ( ( ch->fighting == NULL ) 
&& ( !IS_NPC( ch ) ) 
&& ( !IS_NPC( victim ) ) ) 
{ 
ch->attacker = TRUE; 
victim->attacker = FALSE; 
} 

/* dexterity */ 
chance += get_curr_stat(ch,STAT_DEX); 
chance -= get_curr_stat(victim,STAT_DEX)/2; 

/* speed */ 
if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE)) 
chance += 25; 
if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE)) 
chance -= 10; 

/* level */ 
chance += (ch->level - victim->level) * 4; 

/* sloppy hack to prevent false zeroes */ 
if (chance % 5 == 0) 
chance += 1; 

/* now the attack */ 
if (number_percent() * 2  < chance) 
{ 
AFFECT_DATA af; 
act("{k$n is choked by a wire around the neck!{x",victim,NULL,NULL,TO_ROOM); 
act("{i$n slips a wire around your neck!{x",ch,NULL,victim,TO_VICT); 
multi_hit (ch, victim, gsn_garrote);
send_to_char("{7You choke and gag!{x\n\r",victim); 
check_improve(ch,gsn_garrote,TRUE,2); 
WAIT_STATE(ch,skill_table[gsn_garrote].beats); 

/* Leave the garrote on the choke the shit out of em */ 

af.where = TO_AFFECTS2; 
af.type = gsn_garrote; 
af.level = ch->level; 
af.duration = 3; 
af.location = APPLY_HITROLL; 
af.modifier = -150; 
af.bitvector = AFF2_GARROTE; 

affect_to_char(victim,&af); 
} 
else 
{ 
damage(ch,victim,0,gsn_garrote,DAM_NONE,TRUE, 0); 
check_improve(ch,gsn_garrote,FALSE,2); 
WAIT_STATE(ch,skill_table[gsn_garrote].beats); 
} 
} 

void do_spells(CHAR_DATA *ch, char *argument)
{
    char spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char spell_columns[LEVEL_HERO];
    int sn,lev,mana;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    bool ShowColors = TRUE;
    bool fWhite = FALSE;
    bool fRed = FALSE;
    bool fBlue = FALSE;
    bool fGreen = FALSE;
    bool fOrange = FALSE;
    bool fBlack = FALSE;

    if (IS_NPC(ch))
      return;

    argument = one_argument(argument,arg);

    if(arg[0] != '\0')
    {
     if(!str_cmp(arg,"white"))
     {
      fWhite = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {7White{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_WHITE]);
     }
     if(!str_cmp(arg,"red"))
     {
      fRed = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {!Red{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_RED]);
     }
     if(!str_cmp(arg,"blue"))
     {
      fBlue = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {$Blue{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_BLUE]);
     }
     if(!str_cmp(arg,"green"))
     {
      fGreen = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {@Green{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_GREEN]);
     }
     if(!str_cmp(arg,"black"))
     {
      fBlack = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {8Black{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_BLACK]);
     }
     if(!str_cmp(arg,"orange"))
     {
      fOrange = TRUE;
      ShowColors = FALSE;
      printf_to_char(ch,"Proficiecy in {3Orange{x Magic: {&%d{x.\n\r",ch->magic[MAGIC_ORANGE]);
     }
    }


    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
	spell_columns[lev] = 0;
	spell_list[lev][0] = '\0';
    }
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
        break;

/*      if (skill_table[sn].skill_level[ch->class] < LEVEL_HERO &&
          skill_table[sn].spell_fun != spell_null && 
          ch->pcdata->learned[sn] > 0)
      {
	found = TRUE;
        lev = skill_table[sn].skill_level[ch->class]; */

        lev = ch->pcdata->learnlvl[sn];

      if (lev < LEVEL_HERO &&
          skill_table[sn].spell_fun != spell_null && 
	  ch->pcdata->learned[sn] > 0)
      {
        found = TRUE;
  
	if (ch->level < lev)
        {

      if(!ShowColors)
      {
       if(fWhite && !IS_SPELL_WHITE(sn))
        continue;
       else if(fRed && !IS_SPELL_RED(sn))
        continue;
       else if(fBlue && !IS_SPELL_BLUE(sn))
        continue;
       else if(fGreen && !IS_SPELL_GREEN(sn))
        continue;
       else if(fOrange && !IS_SPELL_ORANGE(sn))
        continue;
       else if(fBlack && !IS_SPELL_BLACK(sn))
        continue;
      }

         if(IS_SPELL_WHITE(sn))
          sprintf(buf,"{&%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else if(IS_SPELL_RED(sn))
          sprintf(buf,"{!%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else if(IS_SPELL_BLUE(sn))
          sprintf(buf,"{$%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else if(IS_SPELL_GREEN(sn))
          sprintf(buf,"{@%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else if(IS_SPELL_ORANGE(sn))
          sprintf(buf,"{3%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else if(IS_SPELL_BLACK(sn))
          sprintf(buf,"{8%-18s{x  {#n/a{x      ", skill_table[sn].name);
         else
          sprintf(buf,"{C%-18s{x  {#n/a{x      ", skill_table[sn].name);
        }
	else
	{

      if(!ShowColors)
      {
       if(fWhite && !IS_SPELL_WHITE(sn))
        continue;
       else if(fRed && !IS_SPELL_RED(sn))
        continue;
       else if(fBlue && !IS_SPELL_BLUE(sn))
        continue;
       else if(fGreen && !IS_SPELL_GREEN(sn))
        continue;
       else if(fOrange && !IS_SPELL_ORANGE(sn))
        continue;
       else if(fBlack && !IS_SPELL_BLACK(sn))
        continue;
      }

	  mana = UMAX(skill_table[sn].min_mana,
		      100/(2 + ch->level - lev));
         if(IS_SPELL_WHITE(sn))//maybe put * 3 or 2 after [sn] --bree learned
          sprintf(buf,"{&%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else if(IS_SPELL_BLUE(sn))
          sprintf(buf,"{$%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else if(IS_SPELL_RED(sn))
          sprintf(buf,"{!%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else if(IS_SPELL_GREEN(sn))
          sprintf(buf,"{@%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else if(IS_SPELL_ORANGE(sn))
          sprintf(buf,"{3%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else if(IS_SPELL_BLACK(sn))
          sprintf(buf,"{8%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);
         else
          sprintf(buf,"{C%-18s  {#%3d{x mana {@({&%d{@){x ",skill_table[sn].name,mana,ch->pcdata->learned[sn]);

	}
	
	if (spell_list[lev][0] == '\0')
          sprintf(spell_list[lev],"\n\r{BLevel{x {&%2d{x: %s",lev,buf);
        else /* append */
	{
	  if ( ++spell_columns[lev] % 2 == 0)
            strcat(spell_list[lev],"\n\r          ");
	  strcat(spell_list[lev],buf);
        }
      }
    }

    /* return results */
 
    if (!found)
    {
      send_to_char("You know no spells.\n\r",ch);
      return;
    }
    
    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (spell_list[lev][0] != '\0')
	send_to_char(spell_list[lev],ch);
    send_to_char("\n\r",ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
    char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
    char skill_columns[LEVEL_HERO];
    int sn,lev;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;
 
    /* initilize data */
    for (lev = 0; lev < LEVEL_HERO; lev++)
    {
        skill_columns[lev] = 0;
        skill_list[lev][0] = '\0';
    }

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL )
        break;

      lev = ch->pcdata->learnlvl[sn];
 
/*      if (skill_table[sn].skill_level[ch->class] < LEVEL_HERO && */

      if (lev < LEVEL_HERO &&
          skill_table[sn].spell_fun == spell_null && 
	  ch->pcdata->learned[sn] > 0)
      {
        found = TRUE;
//      lev = skill_table[sn].skill_level[ch->class];

        if (ch->level < lev)
          sprintf(buf,"{R%-18s{x {@n/a{x      ", skill_table[sn].name);
        else
          sprintf(buf,"{R%-18s{x {@%3d%%{x      ",skill_table[sn].name,
					 ch->pcdata->learned[sn]);
 
        if (skill_list[lev][0] == '\0')
          sprintf(skill_list[lev],"\n\r{yLevel {&%2d{x: %s",lev,buf);
        else /* append */
        {
          if ( ++skill_columns[lev] % 2 == 0)
            strcat(skill_list[lev],"\n\r          ");
          strcat(skill_list[lev],buf);
        }
      }
    }

    /* return results */
 
    if (!found)
    {
      send_to_char("You know no skills.\n\r",ch);
      return;
    }

    for (lev = 0; lev < LEVEL_HERO; lev++)
      if (skill_list[lev][0] != '\0')
        send_to_char(skill_list[lev],ch);
    send_to_char("\n\r",ch);
}


/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
    char buf[100];
    int sn,col;

    if (IS_NPC(ch))
	return;

    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (!ch->gen_data->skill_chosen[sn] 
        &&  ch->pcdata->learned[sn] < 1
	&&  skill_table[sn].skill_level[ch->class] < LEVEL_HERO )
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
 	    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
            /* Bad Hack but should work
             * Sets class gainable skills to 0
             * leaves ungainables at -1
             */
            ch->pcdata->learned[sn] = 0;
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);

    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %ld\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen(CHAR_DATA *ch)
{
    char buf[100];
    int sn,col;
 
    if (IS_NPC(ch))
        return;
 
    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if (ch->gen_data->skill_chosen[sn] 
	&&  skill_table[sn].skill_level[ch->class] <= LEVEL_HERO)
        {
            sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
                                    skill_table[sn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r",ch);
 
    sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"Experience per level: %ld\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}

long exp_per_level(CHAR_DATA *ch, int points)
{
    long expl,inc,epl;

    if (IS_NPC(ch))
	return 1000; 

    expl = 1000;
    inc = 500;

    if (points < 40)
	return 1000 * pc_race_table[ch->race].class_mult[ch->class]/100;

    /* processing */
    points -= 40;

    while (points > 9)
    {
	expl += (long)inc;
        points -= 10;
        if (points > 9)
	{
	    expl += (long)inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += (long)points * (long)inc / 10;  
    
    epl = (long)expl * (long)pc_race_table[ch->race].class_mult[ch->class]/100;
    
    if(ch->pcdata->tier > 2)
    {	    
    /* Temporary check */
    if(epl < 3000)
    epl += (long)(epl * 8 / 100);
    else if(epl < 6000)
    epl += (long)(epl * 4 / 100);
    else
    epl += (long)(epl * 2 / 200);
    }
    else if(ch->pcdata->tier > 1)
    {
    	epl += (long)(epl * 4 / 100);
    }
   
    return epl;
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int sn;
 
    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_help(ch,"group help");
	    return TRUE;
	}

        do_help(ch,argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("You must provide a skill name.\n\r",ch);
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > 0)
	    {
		send_to_char("You already know that skill!\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].skill_level[ch->class] > LEVEL_HERO)
	    {
		send_to_char("That skill is not available.\n\r",ch);
		return TRUE;
	    }
	    sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 1;
	    ch->pcdata->points += skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("No skills or groups by that name...\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {
	if (argument[0] == '\0')
  	{
	    send_to_char("You must provide a skill to drop.\n\r",ch);
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("Skill dropped.\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("You haven't bought any such skill or group.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_help(ch,"premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }
    return FALSE;
}
        

int spell_avail( CHAR_DATA *ch, const char *name )
{
    /* checks to see if a spell is available to a class */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if (skill_table[sn].skill_level[ch->class] <= LEVEL_HERO)
		    return sn;
	}
    }
    return found;
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];

    if (IS_NPC(ch))
	return;

    sn = handling_improve(ch,sn);

    if (ch->level < skill_table[sn].skill_level[ch->class]
    ||  skill_table[sn].skill_level[ch->class] > LEVEL_HERO
    ||  ch->pcdata->learned[sn] == 0
    ||  ch->pcdata->learned[sn] == 100)
	return;  /* skill is not known */ 

    /* check to see if the character has a chance to learn */
    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance /= (		multiplier
		*	skill_table[sn].rating[ch->class] 
		*	4)+1;
    chance += ch->level;

    if (number_range(1,1000) > chance)
	return;

    /* now that the character has a CHANCE to learn, see if they really have */	

    if (success)
    {
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
	    ch->pcdata->learned[sn]++;
	    sprintf(buf,"{GYou have become better at {R%s{G! ({R%d{G%%){x\n\r",
		    skill_table[sn].name, ch->pcdata->learned[sn]);
	    send_to_char(buf,ch);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }

    else
    {
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
	    ch->pcdata->learned[sn] += number_range(1,3);
	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
	    sprintf(buf,
		"{GYou learn from your mistakes, and your {R%s {Gskill improves. ({R%d{G%%){x\n\r",
		skill_table[sn].name, ch->pcdata->learned[sn]);
	    send_to_char(buf,ch);
	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	    if (deduct)
	   	ch->pcdata->points += skill_table[sn].rating[ch->class]; 
	}
	return;
    }
	
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }

}

void do_showclass(CHAR_DATA *ch, char* argument)
{
    char buf[100];
    int sn,col,class;

    if (IS_NPC(ch))
	return;

    if ( (class = class_lookup(argument)) == -1)
    {
	send_to_char("That's not a valid class",ch);
	return;
    }

    col = 0;
 
    sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","skill","cp","skill","cp","skill","cp");
    send_to_char(buf,ch);
 
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
 
        if ( skill_table[sn].skill_level[class] < LEVEL_HERO )
        {
            sprintf(buf,"%-18s %-2d(%-3d) ",skill_table[sn].name,
            skill_table[sn].rating[class],skill_table[sn].skill_level[class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
        send_to_char( "\n\r", ch );
    send_to_char("\n\r", ch );
    return;
}

void do_showskill(CHAR_DATA *ch, char* argument)
{
	int class;
	int skill;
        char buf[MAX_STRING_LENGTH];

        skill = skill_lookup(argument);
        if ( skill == -1 )
	{
		send_to_char("That's not a skill or spell.\n\r",ch);
		return;
	}

	sprintf(buf,"The following classes get '%s':\n\r",argument);
        for ( class = 0; class < MAX_CLASS; class++ )
        {
	    if(skill_table[skill].skill_level[class] < LEVEL_HERO)
	    {
            	if ( class > 0 )
            	    strcat( buf, " " );
            	strcat( buf, class_table[class].name );
	    }
        }
    strcat( buf, ".\n\r" );

    send_to_char(buf,ch);
    return;
}

/* Monk Stuff -- Skyntil */
void do_open_palming(CHAR_DATA *ch,char *argument)
{
    AFFECT_DATA af;
    int chance;

    if ( (chance = get_skill(ch,gsn_palm)) == 0
    || ch->pcdata->learned[gsn_palm] < 1)
    {
        send_to_char("You don't have the ability to use open palm striking martial arts.\n\r",ch);
        return;
    }

    if (IS_NPC(ch) && IS_AFFECTED(ch,AFF_CHARM))
        return;

    if (is_affected(ch,gsn_palm))
    {
        send_to_char("Your palms are already tense.\n\r",ch);
        return;
    }

    if (get_eq_char(ch,WEAR_WIELD) != NULL
        || get_eq_char(ch,WEAR_SECONDARY) != NULL)
	{
	send_to_char("You can't start using open palm striking while using weapons.\n\r",ch);
	return;
	}

    if (number_percent() >= chance)
    {
        send_to_char("Your try to tense the nerves in your hands, but fail.\n\r",ch);
        check_improve(ch,gsn_palm,FALSE,2);
        return;
    }

    act("$n goes into a trance and $s hands become deadly weapons.",ch,0,0,TO_ROOM);
    send_to_char("You feel your mind relax as the nerves in your hands tense.\n\r",ch);
    af.where = TO_AFFECTS;
    af.type = gsn_palm;
    af.location = 0;
    af.modifier = 0;
    af.duration = (ch->level/4);
    af.level = ch->level;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    check_improve(ch,gsn_palm,TRUE,2);
	WAIT_STATE(ch,6);
    return;
}

void do_martial_arts(CHAR_DATA *ch,char *argument)
{
    AFFECT_DATA af;
    int chance;

    if ( (chance = get_skill(ch,gsn_martial_arts)) == 0
    || ch->pcdata->learned[gsn_martial_arts] < 1 )
    {
        send_to_char("You don't know any martial arts.\n\r",ch);
        return;
    }

    if (ch->class == CLASS_MONK)
    {
	    if (is_affected(ch,gsn_martial_arts))
	    {
	    	if (ch->fighting == NULL)
	    	{
        	send_to_char("You stop focusing on the art of combat and find inner peace.\n\r",ch);
        	affect_strip(ch,gsn_martial_arts);
        	WAIT_STATE(ch,18);
        	return;
    		}
    		else
    		{
	    	send_to_char("You are too busy fighting to do that!\n\r",ch);
	    	return;
    		}
		}
    }
    else if (is_affected(ch,gsn_martial_arts))
    {
	    send_to_char("You are already focused on combat.\n\r",ch);
	    return;
    }

    if ((get_eq_char(ch,WEAR_WIELD) != NULL
        || get_eq_char(ch,WEAR_SECONDARY) != NULL) && ch->class == CLASS_MONK)
	{
        send_to_char("You cannot practice this skill using weapons.\n\r",ch);
	return;
	}

    if (number_percent() >= chance)
    {
        send_to_char("You build up intense focus but lose concentration.\n\r",ch);
        check_improve(ch,gsn_martial_arts,FALSE,2);
        return;
    }

    act("$n's face clouds with a look of deadly concentration.",ch,0,0,TO_ROOM);
    send_to_char("You feel your body's focus increase toward mastery.\n\r",ch);
    af.where = TO_AFFECTS;
    af.type = gsn_martial_arts;
    af.location = 0;
    af.modifier = 0;
    if (ch->class == CLASS_MONK)
    {
    af.duration = -1;
	}
	else
	{
	af.duration = ch->level/5;
	}
    af.level = ch->level;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    WAIT_STATE(ch,6);

    return;
}

void lose_skill(CHAR_DATA *ch, int skill)
{
 int gain = 0; /* Gain from losing */
 int prac_gain = 0;
 char log_name[MSL];
 char log_buf[100];

 // If your class loses a skill
 if(!IS_IMMORTAL(ch) && !class_has_skill(ch->class, skill))
 {
  gain = skill_table[skill].rating[ch->class];
  if(ch->pcdata->learned[skill] >= 75)
  	prac_gain = 3;
  else if(ch->pcdata->learned[skill] >= 50)
  	prac_gain = 2;
  else if(ch->pcdata->learned[skill] >= 25)
  	prac_gain = 1;
  else
  	prac_gain = 0;	
  ch->train += gain;
  ch->practice += prac_gain;
  printf_to_char(ch,"You lose the %s skill and gain %d trains.\n\r",
  skill_table[skill].name,gain);
  ch->pcdata->learned[skill] = -1;
  ch->pcdata->learnlvl[skill] = 102;
  sprintf(log_name,"%s",ch->name);
  sprintf(log_buf," has lost the %s skill.", skill_table[skill].name);
  strcat(log_name, log_buf);
  log_string(log_name);  
 }
 return;
}

void gain_skill(CHAR_DATA *ch, int skill)
{
 // You figure it out
 if(class_has_skill(ch->class, skill) && ch->pcdata->learned[skill] < 1)
 {
  if(ch->level > skill_table[skill].skill_level[ch->class])
  {
   ch->pcdata->learned[skill] = 75;
   ch->pcdata->learnlvl[skill] = skill_table[skill].skill_level[ch->class];
  }
  else
  {
   ch->pcdata->learned[skill] = 1;
   ch->pcdata->learnlvl[skill] = skill_table[skill].skill_level[ch->class];
  }
  printf_to_char(ch,"You gain the %s skill at %d percent.\n\r",
  skill_table[skill].name,ch->pcdata->learned[skill]);
 }
 return;
}

/* Gain a skill into the gain list */
void gain_skill_gain(CHAR_DATA *ch, int skill)
{
 char log_name[MSL];
 char log_buf[100];
 
 if(class_has_skill(ch->class, skill) && ch->pcdata->learned[skill] < 0)
 {
   ch->pcdata->learned[skill] = 0;
   printf_to_char(ch,"You are now able to gain the %s skill.\n\r",
   skill_table[skill].name);
   sprintf(log_name,"%s",ch->name);
   sprintf(log_buf," can now gain the %s skill.", skill_table[skill].name);
   strcat(log_name, log_buf);
   log_string(log_name);
 }
 return;
}


bool has_skill(CHAR_DATA *ch,int skill)
{
  return IS_NPC(ch) ? TRUE : (ch->level >= ch->pcdata->learnlvl[skill]); 
}

bool class_has_skill(int class, int skill)
{
 return (skill_table[skill].skill_level[class] < 102);
}

bool has_learned_skill(CHAR_DATA *ch,int skill)
{
  return IS_NPC(ch) ? TRUE : (ch->pcdata->learned[skill] > 0); 
}

bool has_gain(CHAR_DATA *ch,int skill)
{
  if(ch->pcdata->learned[skill] == 0)
   return TRUE;
  else
   return FALSE;
}

int handling_improve(CHAR_DATA *ch, sh_int sn)
{
  sh_int wsn = -1;

  if(ch->pcdata->learned[sn] < 100)
  return sn;

  if(sn == gsn_sword)
  wsn = gsn_sword2;
  else if(sn == gsn_axe)
  wsn = gsn_axe2;
  else if(sn == gsn_hand_to_hand)
  wsn = gsn_hand2;
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
  return sn;
  
  return wsn;

}

void do_handle(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    sh_int sn,wsn = -1;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && (IS_SET(trainer->act,ACT_GAIN)
            || IS_SET(trainer->act,ACT_TRAIN)) )
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("You can't do that here.\n\r",ch);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        do_say(trainer,"What do you wish to learn?");
        do_say(trainer,"I can teach you how to handle your weapon better perhaps?");
	return;
    }

    if(str_cmp(arg,"sword")
    && str_cmp(arg,"dagger")
    && str_cmp(arg,"polearm")
    && str_cmp(arg,"spear")
    && str_cmp(arg,"flail")
    && str_cmp(arg,"mace")
    && str_cmp(arg,"whip")
    && str_cmp(arg,"axe")
    && str_cmp(arg,"dual wield") )
    {
     do_say(trainer,"Pardon me?");
     return;
    }

    sn = skill_lookup(arg);

    if (ch->pcdata->learned[sn] < 100 && sn != gsn_dual_wield)
    {
       act("$N tells you 'You are not yet ready to learn with that weapon.'",ch,NULL,trainer,TO_CHAR);
       return;
    }
    else if(ch->pcdata->learned[sn] < 100 && sn == gsn_dual_wield)
    {
       act("$N tells you 'You are not yet ready to learn dual weapon mastery.'",ch,NULL,trainer,TO_CHAR);
       return;
    }
    
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
   return;
   
  if(wsn == gsn_dual_wield2)
  {
  	ch->pcdata->learned[wsn] = 1;
  	ch->pcdata->learnlvl[wsn] = skill_table[wsn].skill_level[ch->class];
   	act("$N teaches how to better handle your weapon!",ch,NULL,trainer,TO_CHAR);
   	return;
  }

  if(ch->class == CLASS_ASSASSIN && !IS_IMMORTAL(ch) && wsn == gsn_dagger2)
  {
   ch->pcdata->learned[wsn] = 1;
   act("$N teaches how to better handle your weapon!",ch,NULL,trainer,TO_CHAR);
   return;
  }

  if(ch->class != CLASS_CRUSADER && !IS_IMMORTAL(ch))
  {
    do_say(trainer,"You cannot learn to better handle that weapon.");
    return;
  }
  ch->pcdata->learned[wsn] = 1;

  act("$N teaches how to better handle your weapon!",ch,NULL,trainer,TO_CHAR);

  /*if(ch->level == LEVEL_HERO)
   ch->practice += 2;*/

  return;
}

// New Stuff 01/2005 -- Skyn.

// Ground Stomp
void check_ground_stomp(CHAR_DATA *ch,CHAR_DATA *victim,int chance,int dam)
{
	if (chance < 10)
		return;
	if (dam == 0)
		return;

        chance = get_skill(ch,gsn_ground_stomp); 
        chance /= 2; 

  	chance -= number_range(5,15);
        chance = URANGE(5,chance,50);

        if (!has_skill(ch,gsn_ground_stomp))
        return;

        if (number_percent() > chance)
	{
                check_improve(ch,gsn_ground_stomp,FALSE,3); 
                return; 
	}

        act("You land your foot hard on $N's body.",ch,0,victim,TO_CHAR); 
        act("$n stomps $N's body with brutal force.",ch,0,victim,TO_NOTVICT); 
        act("$n lands his foot hard against your body.",ch,0,victim,TO_VICT); 
          
        check_improve(ch,gsn_ground_stomp,TRUE,2);

        dam = dam * 4; 
        dam = dam / 5;

        if(number_percent() < 10)
        {
         act("You hit $N critically!",ch,0,victim,TO_CHAR);
         dam *= 3/2;
        }

        damage(ch,victim,dam,gsn_ground_stomp,DAM_BASH,TRUE, 0);
        return; 
}

// Side Kick
void do_side_kick( CHAR_DATA *ch, char *argument )
{
    int dam;
    CHAR_DATA *victim;

    if(IS_NPC(ch))
    return;

    if ( !IS_NPC(ch) && !has_skill(ch,gsn_sidekick))
    {
        send_to_char("Huh?\n\r", ch );
        return;
    }
    
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }
    
    if( ch->stunned )
    {
	send_to_char("You're still a bit woozy.\n\r",ch);
	return;
    }

    if ( get_skill(ch,gsn_sidekick) > number_percent())
    {
	dam = number_range (1, ch->level);
	dam += number_range(0,(ch->level)/3);
        dam += number_range(0,(ch->level)/3);
        dam += number_range(0,(ch->level)/3);
        dam += number_range(0,(ch->level)/4);
        dam += number_range(0,(ch->level)/4);
        dam += number_range(0,(ch->level)/4);
        dam += number_range(0,(ch->level)/4);
	dam += number_range(ch->level/3,ch->level/2);

        act("You kick $N brutally in the side!",ch,0,victim,TO_CHAR);
        act("$n kicks you brutally in the side!",ch,0,victim,TO_VICT);
        
        if(has_skill(ch,gsn_vital_hit)) 
        { 
        	if( (get_skill(ch,gsn_vital_hit) / 12) > number_percent( )) 
         	{ 
         		check_improve(ch,gsn_vital_hit,TRUE, 3); 
         		dam = (dam * 8) / 5; 
         	} 
        }
        
        damage(ch,victim,dam, gsn_sidekick,DAM_BASH,TRUE, 0);
        check_improve(ch,gsn_sidekick,TRUE,1);
	check_follow_through(ch,victim,dam);
    }
    else
    {
        damage( ch, victim, 0, gsn_sidekick,DAM_BASH,TRUE, 0);
        check_improve(ch,gsn_sidekick,FALSE,1);
    }
    
    WAIT_STATE( ch, skill_table[gsn_sidekick].beats );

    return;
}

// Spin Kick
void do_spin_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;
    int skill;

    if (((get_skill(ch,gsn_spin_kick)) <= 0)
    || !has_skill(ch,gsn_spin_kick))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if (!is_affected(ch,gsn_martial_arts))
    {
	    send_to_char("You must be focused on the martial arts to use this skill.\n\r",ch);
	    return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }
    
    if( ch->stunned )
    {
	send_to_char("You're still a bit woozy.\n\r",ch);
	return;
    }
    
    if (number_percent() > get_skill(ch,gsn_spin_kick))
    {
        send_to_char("You slip and fail your kick.\n\r",ch);
        check_improve(ch,gsn_spin_kick, FALSE,1);
        WAIT_STATE(ch, skill_table[gsn_spin_kick].beats);
	return;
    }

    act("You deftly spin around and kick $N in the chest!",ch,NULL,victim,TO_CHAR);
    act("$n spins around and kicks you in the chest!",ch,NULL,victim,TO_VICT);
    act("$n spins around deftly and kicks $N hard!",ch,NULL,victim,TO_NOTVICT);

	skill = get_skill(ch,gsn_spin_kick);

    if(ch->level <= 80)
    dam = ch->level * 2 + (number_range(0, skill)) * 3 / 2;
    else if(ch->level <= 90)
    dam = ch->level * 3 + (number_range((skill/3), skill)) * 3 / 2;
    else if(ch->level <= 100)
    dam = ch->level * 3 + (number_range((skill/2), skill)) * 9 / 5;
    else
    dam = ch->level * 3 + (number_range((skill*3/4), skill)) * 9 / 4;
    
    dam += -2*(dex_app[get_curr_stat(ch,STAT_DEX)].defensive);
    
    if (number_percent() < get_skill(ch,gsn_spin_kick) / 8)
    {
    	act("Your kick knocks $N's breath away!",ch,NULL,victim,TO_CHAR);
    	act("$n's kick knocks your breath away!",ch,NULL,victim,TO_VICT);
    	act("$n's kick sends $N sprawling!",ch,NULL,victim,TO_NOTVICT);
    	WAIT_STATE(victim,2*PULSE_VIOLENCE); 
    }
    
    	if(has_skill(ch,gsn_vital_hit)) 
        { 
        	if( (get_skill(ch,gsn_vital_hit) / 12) > number_percent( )) 
         	{ 
         		check_improve(ch,gsn_vital_hit,TRUE, 3); 
         		dam = (dam * 8) / 5; 
         	} 
        }
        
    WAIT_STATE(ch, skill_table[gsn_spin_kick].beats);
    
    damage(ch,victim,dam, gsn_spin_kick,DAM_SLASH,TRUE, 0);
    check_improve(ch,gsn_spin_kick, TRUE,1);
    check_follow_through(ch,victim,dam);
    
    return;
}

// Follow through
void check_follow_through(CHAR_DATA *ch, CHAR_DATA *victim, int dam) 
{ 
    int chance;
    OBJ_DATA *wield;
	wield = get_eq_char(ch,WEAR_WIELD);
 
    chance = get_skill(ch,gsn_follow_through); 
 
    if (wield != NULL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
    {
	chance=chance*(get_curr_stat(ch,STAT_DEX)/12);
	}
	chance=chance/2;
    chance -= number_range(0,7); 
    
    if (!has_skill(ch,gsn_follow_through)) 
        return; 
 
    if (number_percent() < chance) 
    { 
	    if (ch->class == CLASS_HIGHLANDER)
	    {
		    if (wield != NULL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
		    {
		    act("The momentum of $n's weapon brings them around for another attack!",ch,0,0,TO_ROOM); 
        	act("The momentum of your swing brings your two-handed weapon around for another attack!",ch,0,0,TO_CHAR);
        	dam = dam * 7/4;
        	damage(ch, victim, dam, gsn_follow_through, DAM_SLASH, TRUE, 0); 
        	check_improve(ch,gsn_follow_through,TRUE,2);
    		}
    		else
    		{
	    	act("The momentum of $n's weapon brings them around for another attack!",ch,0,0,TO_ROOM); 
        	act("The momentum of your swing brings your weapon around for another attack!",ch,0,0,TO_CHAR);
        	dam = dam * 2/3;
        	damage(ch, victim, dam, gsn_follow_through, DAM_SLASH, TRUE, 0); 
        	check_improve(ch,gsn_follow_through,TRUE,2);
    		}
       } 
       else if (is_affected(ch,gsn_martial_arts))
       {
        act("$n's leg sweeps around and strikes with a follow through!",ch,0,0,TO_ROOM); 
        act("Your leg sweeps around to land a follow through kick!",ch,0,0,TO_CHAR);
        dam = dam * 4; 
        dam = dam / 5;
        damage(ch, victim, dam, gsn_follow_through, DAM_BASH, TRUE, 0); 
        check_improve(ch,gsn_follow_through,TRUE,2); 
    }
    } 
    else 
        check_improve(ch,gsn_follow_through,FALSE,3); 
 
    return; 
} 

// Blindness dust  
void do_blindness_dust(CHAR_DATA *ch,char *argument) 
{ 
        CHAR_DATA *vch; 
        CHAR_DATA *vch_next; 
        AFFECT_DATA af;    
        int chance; 
        bool fighting = FALSE; 
        char buf[MAX_STRING_LENGTH];  
 
        if ( (chance = get_skill(ch,gsn_blindness_dust)) == 0 
        || !has_skill(ch,gsn_blindness_dust)) 
        { 
        send_to_char("You don't know how to make blindness dust to throw.\n\r",ch); 
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
        
        if (number_percent() + 10 > chance) 
        { 
        act("$n hurls some dust into the air but it is blown away.",ch,0,0,TO_ROOM); 
        send_to_char("You throw out some dust but it is blown away.\n\r",ch); 
        ch->mana -= 18; 
        check_improve(ch,gsn_blindness_dust,FALSE,2); 
        WAIT_STATE(ch,12); 
        return; 
        } 
        
        act("$n hurls a handful of dust into the room!",ch,0,0,TO_ROOM); 
        send_to_char("You throw a handful of blindness dust into the room!\n\r",ch); 
        check_improve(ch,gsn_blindness_dust,TRUE,2); 
        if (ch->fighting != NULL) 
                fighting = TRUE; 
 
        af.where = TO_AFFECTS; 
        af.type = gsn_blindness_dust; 
        af.level = ch->level; 
        af.duration = (ch->level)/25; 
        af.bitvector = AFF_BLIND; 
        af.location = APPLY_HITROLL; 
        af.modifier = -4; 
        ch->mana -= 36; 
        WAIT_STATE(ch,12); 
        
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next) 
        { 
        	vch_next = vch->next_in_room; 
        	if (is_safe_quiet(ch,vch))    
                	 continue; 
        	if (is_same_group(ch,vch))       
	                continue; 
        	if (!IS_AFFECTED(vch,AFF_BLIND) && !saves_spell( ch->level,vch,DAM_OTHER)) 
        	{ 
        		act("$n appears blinded.",vch,0,0,TO_ROOM); 
        		send_to_char("You get dust in your eyes.\n\r",vch); 
                	affect_to_char(vch,&af); 
        	} 
        	
        	if (!IS_NPC(vch) && !IS_NPC(ch) && (vch->fighting == NULL || (!fighting))) 
                { 
        		sprintf(buf,"Help! %s just threw dust in my eyes!",PERS(ch,vch)); 
                	do_yell(vch,buf); 
                } 
 
                if (vch->fighting == NULL) 
                        multi_hit(vch,ch,TYPE_UNDEFINED); 
         } 
 
        return; 
} 

void do_will_of_iron(CHAR_DATA *ch,char *argument) 
{
   AFFECT_DATA af;
  // int counter;
 
   if (((get_skill(ch,gsn_will_of_iron)) <= 0)
    || !has_skill(ch,gsn_will_of_iron) || IS_NPC(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
  
   if (is_affected(ch,gsn_will_of_iron))
   {
        send_to_char("You are already focusing your will!\n\r",ch);
	return;
   }

   send_to_char("You focus your energy, and your will becomes strong as steel.\n\r",ch);

   //check_improve(ch,gsn_will_of_iron,TRUE,1);

   af.where     = TO_AFFECTS;
   af.type      = gsn_will_of_iron;
   af.level     = ch->level;
   af.duration  = ch->level/5 + 5;
   af.location  = APPLY_AC;
   af.modifier  = -1*(ch->level*5/8);
   af.bitvector = 0;
   affect_to_char( ch, &af );
   af.modifier  = -25;
   af.location  = APPLY_HITROLL;
   affect_to_char( ch, &af );
   af.location = APPLY_REGEN;
   af.modifier = 100;
   affect_to_char( ch, &af );
//   for(counter = 0; counter < 3250; counter++)
   	improve_toughness(ch);
  check_improve(ch,gsn_will_of_iron,TRUE,1);
   return;
}

void do_healing_trance(CHAR_DATA *ch,char *argument)
{
    AFFECT_DATA af;

    if (((get_skill(ch,gsn_healing_trance)) <= 0)
    || !has_skill(ch,gsn_healing_trance) || IS_NPC(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if (ch->fighting != NULL)
    {
	send_to_char("You are too busy fighting to meditate.\n\r",ch);
	return;
    }
    
    if (is_affected(ch,gsn_healing_trance))
    {
        send_to_char("You are already at an inner peace.\n\r",ch);
	return;
    }

    do_rest(ch,"");

    send_to_char("You relax and concentrate on healing your wounds.\n\r",ch);
    
    check_improve(ch,gsn_healing_trance,TRUE,1);
  
    af.where = TO_AFFECTS;
    af.level = ch->level;
    af.type  = gsn_healing_trance;
    af.location = APPLY_REGEN;
    af.modifier = (ch->level-1)*2.0; 
    af.bitvector = 0;
    af.duration = 4;
    affect_to_char(ch,&af);
    af.location = APPLY_MANA_REGEN;
    af.modifier = (ch->level-1)*.1;
    affect_to_char(ch,&af);
    ch->stunned = 4;
    
    return;
}

void do_trance(CHAR_DATA *ch,char *argument)
{

	AFFECT_DATA af;

	if(((get_skill(ch,gsn_meditation)) <= 0)
			|| !has_skill(ch,gsn_meditation) || IS_NPC(ch))
	{
		send_to_char( "Huh?\n\r",ch);
		return;
	}

	if(ch->fighting != NULL)
	{
		send_to_char("You are too busy fighting to meditate.\n\r",ch);
		return;
	}

	if (is_affected(ch,gsn_meditation))
	{
        	send_to_char("You are already meditating.\n\r",ch);
		return;
	}

	do_rest(ch,"");

	send_to_char("You relax and focus your mind within.\n\r",ch);
    
	check_improve(ch,gsn_meditation,TRUE,1);
  
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.type  = gsn_meditation;
	af.location = APPLY_REGEN;
	af.modifier = (ch->level-1)*1; 
	af.bitvector = 0;
	af.duration = 10;
	affect_to_char(ch,&af);
	af.location = APPLY_MANA_REGEN;
	af.modifier = (ch->level-1)*2.5;
	affect_to_char(ch,&af);
	//ch->stunned = 4;
    
	return;
}


void do_purify_body(CHAR_DATA *ch,char *argument)
{
    AFFECT_DATA *af;
    sh_int af_type;
    sh_int sn;
    
    sn = gsn_purify_body;

    if (((get_skill(ch,gsn_purify_body)) <= 0)  || !has_skill(ch,gsn_purify_body) || IS_NPC(ch))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    if (is_affected(ch,gsn_martial_arts))
    {
	    send_to_char( "Not while your mind focuses on the art of war.\n\r",ch);
	    return;
    }

    send_to_char("You go into a deep trance and purify your body.\n\r",ch);
    
    check_improve(ch,gsn_purify_body,TRUE,1);

        for ( af = ch->affected; af != NULL; af = af->next )
        {
            af_type = af->type;

            if( af_type < 0 || af_type > MAX_SKILL)
            {
            bug("Problem: Monk :: Purify Body :: Affect remove",0);
            return;
            }

	    if( af_type != gsn_healing_trance
             && skill_table[af_type].slot != 999 )
		check_dispel(ch->level+35,ch,af->type);
	}

	do_sleep(ch,"");
	ch->stunned = 2;
	WAIT_STATE(ch,24);

    return;
}    

/*void do_spinkick(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    CHAR_DATA *victim,*vch,*vch_next;
    int attempt, chance, dam, level;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    one_argument(argument,arg1);

    if(IS_NPC(ch))
    return;

    if ( (chance = get_skill(ch,gsn_spinkick)) == 0
    || !has_skill(ch,gsn_spinkick))
    {
        send_to_char("Huh?\n\r",ch);
        return;
    }

    if(ch->fighting != NULL)
    victim = ch->fighting;
    else
    {
     if( (victim = get_char_room(ch,arg1)) == NULL )
     {
    	send_to_char("They're not here.\n\r",ch);
    	return;
     }
    }

    if(victim == ch)
    { 
     send_to_char("Kicking yourself? get real.\n\r",ch);
     return;
    }
    if (!IS_NPC(victim) && !can_pkill(ch,victim)) {
     send_to_char("You can't seem to kick them.",ch);
     return;
    }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt to spin kick, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to spin kick, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }
      
    if(ch->stunned)
    {
     send_to_char("You are still a bit woozy.\n\r",ch);
     return;
    }

    level = ch->level + 5;
    // chance = (ch->level - victim->level) * 2;
    chance += number_range(20,50);
    chance -= get_curr_stat(victim,STAT_DEX)/2;
    chance -= get_curr_stat(victim,STAT_STR)/3;
    chance += get_curr_stat(ch,STAT_STR)/3;
    chance += get_curr_stat(ch,STAT_DEX);

    if (IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;
    if (IS_AFFECTED(ch,AFF_HASTE))
        chance += 25;
    if (IS_AFFECTED(victim,AFF_SLOW))
        chance += 20;
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance -= 30;
    if (IS_AFFECTED(victim,AFF_FLYING))
        chance += 5;
    chance += (ch->size - victim->size)*10;

   if (!IS_NPC(victim) && !IS_NPC(ch) && (victim->hit > 1))
   {
      sprintf(buf,"Help! %s just tried to kick me out of the room!",PERS(ch,victim));

      if(ch->fighting == NULL)
      {
      do_yell(victim,buf);
      }
   }


    if (number_percent() > chance)
    {
        act("You spin your leg around but miss $N.",ch,0,victim,TO_CHAR);
        act("$n spins his leg around but misses you.",ch,0,victim,TO_VICT);
        act("$n spins his leg visciously at $N but misses.",ch,0,victim,TO_NOTVICT);
        check_improve(ch,gsn_spinkick,FALSE,3);
        WAIT_STATE(ch,10);
        if(victim->fighting == NULL)
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    was_in = victim->in_room;
    dam = dice(level,8);
    
    // Stance modifier
    if (!IS_NPC(ch) && ch->stance[0] > 0 && number_percent() < 3)
    {
	int stance = ch->stance[0];
        if (ch->stance[stance] >= 200)
	    dam += dice(ch->stance[stance],2);
	else if(ch->stance[stance] > 0)
 	    dam += dice(ch->stance[stance],1);
    }

   if(number_percent() < 25)
   {

    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;
        char dir[MAX_STRING_LENGTH/4];

	door = number_door( );

        if (door == 0)
        (strcpy(dir,"north"));
        else if (door == 1)
        (strcpy(dir,"east"));
        else if (door == 2)
        (strcpy(dir,"south"));
        else if (door == 3)
        (strcpy(dir,"west"));
        else if (door == 4)
        (strcpy(dir,"up"));
        else if (door == 5)
        (strcpy(dir,"down"));
        else
        strcpy(dir,"none");

	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
        ||  ( IS_SET(pexit->exit_info, EX_CLOSED) )
	|| ( IS_NPC(victim)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

        if ( ( was_in->sector_type == SECT_AIR)
        ||  ( (pexit->u1.to_room->sector_type == SECT_AIR )
        &&  ( !IS_AFFECTED(victim, AFF_FLYING) ) ) )
            continue;

        if (IS_SET(pexit->exit_info,EX_CLOSED))
        {
        act("You spin around and kick $N through the door with incredible force!",ch,0,victim,TO_CHAR);
        act("$n's spin kick sends $N crashing through the door with incredible force!",ch,0,victim,TO_NOTVICT);
        act("$n spins around and kicks you through the door with incredible force!",ch,0,victim,TO_VICT);
        }
        else
        {
        act("You spin your leg visciously at $N and kick $M from the room!",ch,0,victim,TO_CHAR);
        act("$n spins his leg visciously at $N and kicks $M from the room!",ch,NULL,victim,TO_NOTVICT);
        act("$n spins his leg visciously at you and kicks you from the room!",ch,0,victim,TO_VICT);
        dam = dice(level,5);
        }

        for(vch = victim->in_room->people; vch != NULL; vch = vch_next)
        {
         vch_next = vch->next_in_room;

         if(IS_IMMORTAL(vch))
         {
         sprintf(buf,"\n\r%s is kicked %s.",victim->name,dir);
         send_to_char(buf,vch);
         }
        }

        char_from_room(victim);
        char_to_room(victim, pexit->u1.to_room);
	do_look(victim,"auto");

        if (IS_SET(pexit->exit_info,EX_CLOSED))
        {
        REMOVE_BIT(pexit->exit_info,EX_CLOSED);
        act("With a deafening crash the door explodes open and $n comes flying into the room!",victim,0,0,TO_ROOM);
        }
        else
        {
        act("$n comes flying into the room with incredible force!",victim,0,0,TO_ROOM);
        }

        if (saves_spell(ch->level,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_spinkick,DAM_BASH,TRUE,0);
        WAIT_STATE(victim,36);
        WAIT_STATE(ch,24);
        stop_fighting( ch, TRUE );
        return;
    }

    if( attempt == 6 )
    {
	dam += dice(level,4);
	act("You spin your leg viscously at $N and kick $M brutally into a wall!",ch,0,victim,TO_CHAR);
    	act("$n spins his leg viscously at $N and kicks $M brutally into a wall!",ch,0,victim,TO_NOTVICT);
    	act("$n spins his leg viscously at you and kicks you brutally into a wall!",ch,0,victim,TO_VICT);

        if (saves_spell(ch->level,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_spinkick,DAM_BASH,TRUE,0);
        WAIT_STATE(ch,24);
        WAIT_STATE(victim,24);
    }
   }
   else
   {
    act("You spin your leg viscously at $N and kick $M brutally!",ch,0,victim,TO_CHAR);
    act("$n spins his leg viscously at $N and kicks $M brutally!",ch,0,victim,TO_NOTVICT);
    act("$n spins his leg viscously at you and kicks you brutally!",ch,0,victim,TO_VICT);

        if (saves_spell(ch->level-4,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_spinkick,DAM_BASH,TRUE,0);

        if(number_range(1,2) == 2)
        WAIT_STATE(victim,24);
        else
        WAIT_STATE(victim,12);

        WAIT_STATE(ch,12);

   }
   return; 
}*/


/*void do_chop( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam; 
        char arg[MAX_INPUT_LENGTH]; 
        OBJ_DATA *wield,*second; 
        int multiplier; 
        bool DUAL = FALSE; 
         
        dam = 0; 
        one_argument(argument,arg); 
        if ((chance = get_skill(ch,gsn_chop) ) == 0 
        || !has_skill(ch,gsn_chop) ) 
        { 
        send_to_char("Chopping? What's that?\n\r",ch); 
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
        send_to_char("You can't chop at yourself!\n\r",ch); 
        return; 
        }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt to chop, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to chop, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
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
          send_to_char("You must be wielding a sword or axe to chop.\n\r",ch); 
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
                chance -= 10; 
        chance += (ch->level - victim->level);

        act("$n attempts to split open $N with a quick chop!",ch,0,victim,TO_NOTVICT); 
        act("You attempt to split open $N with a quick chop!",ch,0,victim,TO_CHAR); 
        act("$n attempts to split you open with a quick chop!",ch,0,victim,TO_VICT); 
 
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_lunge,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_chop].beats); 
 
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
 
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_chop]; 
        dam /= 100; 
        multiplier = number_range(ch->level/8, ch->level/4); 
        multiplier /= 10; 
        multiplier += 5/4; 
        dam *= multiplier;
        if(!DUAL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
         dam *= 1.5;
 
        if (dam <= 0) 
                dam = 1; 
        damage(ch,victim,dam,gsn_chop,DAM_PIERCE,TRUE,0); 
        } 
        else 
        { 
        damage(ch,victim,dam,gsn_chop,DAM_PIERCE,TRUE,0); 
        check_improve(ch,gsn_chop,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_chop].beats); 
        } 
        return; 
} 
*/
