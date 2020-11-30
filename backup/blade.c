/* This file is the Clan-Skills of the PK Clan -=GreenBlade=- */
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

DECLARE_DO_FUN( do_announce );
DECLARE_DO_FUN( do_yell );

/* Place a bounty upon a player's head for assassination
 * Restricts:
 * - Victim must be Pk
 * - No more than 250 Plat per Person
 * - No less than 25 plat addition
 * Benefits:
 * - Higher the bounty, more power the GreenBlades have
 * Functionality:
 * - Remove bounty from doner
 * - Add bounty to victim's pcdata
 */
void do_bounty(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *victim;
 DESCRIPTOR_DATA *d;
 int bounty;
 char arg[MAX_INPUT_LENGTH/4], amount[MAX_INPUT_LENGTH/4];
 char buf[MAX_INPUT_LENGTH/4], buf2[MAX_INPUT_LENGTH/4];

 if(IS_NPC(ch))
  return;

 argument = one_argument(argument,arg);
 argument = one_argument(argument,amount);

 if(arg[0] == '\0' || amount[0] == '\0')
 {
   if(ch->pcdata->bounty > 0)
    printf_to_char(ch, "You have a bounty of %ld platinum on your head!\n\r", ch->pcdata->bounty);

   send_to_char("Place a bounty of how much on who?\n\r",ch);
   return;
 }

 if( (victim = get_char_world(ch,arg)) == NULL)
 {
  send_to_char("There is nobody in the lands by that name.\n\r",ch);
  return;
 }

  if(IS_NPC(victim))
  {
   send_to_char("Not on NPCs.\n\r",ch);
   return;
  }


 if(victim->clan == clan_lookup("fortune") || IS_IMMORTAL(victim)
 || !is_pkill(victim))
 {
  send_to_char("You cannot place a bounty upon their head.\n\r",ch);
  return;
 }

 if(ch->clan == clan_lookup("fortune") || ch->clan == victim->clan)
 {
  send_to_char("I dont think so.\n\r",ch);
  return;
 }

 if(!is_number(amount))
 {
  send_to_char("How much platinum do you wish to bounty?\n\r",ch);
  return;
 }

 bounty = atoi(amount);

 if(bounty < 50)
 {
  send_to_char("Minimum bounty is 50 platinum.\n\r",ch);
  return;
 }

 if(ch->platinum < bounty)
 {
  send_to_char("You do not have that much platinum.\n\r",ch);
  return;
 }

 /* Now we have the victim and the amount */
 victim->pcdata->bounty += bounty;
 ch->platinum -= bounty;

 /* Alert any GreenBlades */

 sprintf(buf,"A bounty of %d has been placed on %s's head!\n\r", bounty, PERS(victim,ch));
 sprintf(buf2,"The bounty in full is %ld platinum.\n\r", victim->pcdata->bounty);
 for (d = descriptor_list; d != NULL; d = d->next)
 {
   if (d->connected == CON_PLAYING
       && d->character != ch
       && is_pkill(d->character)
       /*&& (d->character->clan == clan_lookup("fortune"))*/
       && !IS_SET (d->character->comm, COMM_NOCLAN)
       && !IS_SET (d->character->comm, COMM_QUIET))
       {
        send_to_char(buf,d->character);
        send_to_char(buf2,d->character);
       }
 }

 /* Alert the Mud */
 do_announce(ch,buf);

 return;
}

/* Hides the GreenBlade from mortal eyes
 * GreenBlade appears as a green cloaked figure
 * Only Immortals and Clan Members see the real name when wwconcealed
 * Restricts:
 * -None
 * Benefits:
 * - General, lower Ac, small hit/dam, bit of regen.
 * - Overall vanishment
 * Functionality:
 * - Employ Identity or Morph
 * - Use gsn_conceal
 */
void do_conceal(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;

  if(ch->clan != clan_lookup("aesir"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }

  if(str_cmp(clan_table[ch->clan].name,"aesir"))
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
        if (ch->morph_form[0] == MORPH_CONCEAL)
        {
            if(is_affected(ch, gsn_conceal))
            {
                affect_strip(ch,gsn_conceal);
                /* Incase the affect_strip doesn't work */
                if(ch->morph_form[0] > 0)
                 ch->morph_form[0] = 0;
	    }
           /* if (ch->long_descr != NULL && !str_cmp(ch->long_descr,"A dark assassin stands here.\n\r"))
            {
      free_string(ch->long_descr);
	      ch->long_descr = "\0";
            }*/
            send_to_char("You expose yourself to the lands.\n\r",ch); 
	    return;
	}
        else if (number_percent() < get_skill(ch,gsn_third_attack))
	{
	  af.where    = TO_AFFECTS;
          af.type     = gsn_conceal;
	  af.level    = ch->level;
          af.location = APPLY_MORPH_FORM;
          af.modifier = MORPH_CONCEAL;
	  af.bitvector = AFF_SNEAK;
	  af.duration = -1;
	  affect_to_char( ch, &af );

          af.bitvector = 0;
          af.location = APPLY_AC;
          af.modifier = -20;
          affect_to_char(ch,&af);
          af.modifier = 10;
          af.location = APPLY_DAMROLL;
          affect_to_char(ch,&af);   
          af.location = APPLY_HIT;    
          af.location = APPLY_SAVES;
          af.modifier = -10;
          affect_to_char(ch,&af);   

          check_improve(ch,gsn_conceal,TRUE,6);
          send_to_char("You conceal your presence.\n\r",ch);
	  return;
	}
	else
	{
          check_improve(ch,gsn_conceal, FALSE,3);
          send_to_char("You fail to conceal yourself.\n\r",ch);
	  return;
	}

   return;
}

/* Targets a bountied person
 * Restricts:
 * - Must be a bountied person online
 * - Must be concealed
 * Benefits:
 * - The higher the bounty on the selected victim,
 *   the more power the GreenBlade receives
 * - Increased Battle Skills depending on bounty on victim
 * Functionality:
 * - With no argument, all available victims are displayed
 * - Select a victim, assassin is surrounded by emerald aura of power
 */
void do_target(CHAR_DATA *ch, char *argument)
{
 DESCRIPTOR_DATA *d;
 char buf[MSL/10], arg[MSL/10];
 CHAR_DATA *victim;
// AFFECT_DATA af;
/*
 * Clan specific code removed on 12/30/2004 
 */
 /*
  if (get_skill(ch,gsn_target) <= 0
  || !has_skill(ch,gsn_target))
  {
     send_to_char("Huh?\n\r",ch);
     return;
  }

  if(str_cmp(clan_table[ch->clan].name,"fortune") && !IS_IMMORTAL(ch))
  {
   send_to_char("Huh?\n\r",ch);
   return;
  }
*/
  if(!is_pkill(ch) || IS_NPC(ch))
  {
     send_to_char("Huh?\n\r", ch);
     return;
  }
  argument = one_argument(argument,arg);

  if(arg[0] == '\0')
  {
   if(ch->pcdata->target != NULL)
    printf_to_char(ch, "You are currently targeting %s.\n\r", ch->pcdata->target);

   send_to_char("The following persons have a bounty in the lands:\n\r",ch);
   for (d = descriptor_list; d != NULL; d = d->next)
   {
    if (d->connected == CON_PLAYING
       && d->character != ch
       && !IS_IMMORTAL(d->character)
       && d->character->pcdata->bounty > 0)
       {
        sprintf(buf,"%s - %ld Platinum\n\r", capitalize(d->character->name), d->character->pcdata->bounty);
        send_to_char(buf,ch);
       }
   }

   /* check for PK to be targetable */
   send_to_char("You may target:\n\r",ch);
   for (d = descriptor_list; d != NULL; d = d->next)
   {
    if (d->connected == CON_PLAYING
       && d->character != ch
       && !IS_IMMORTAL(d->character)
       && can_pkill(ch,d->character)
       && d->character->pcdata->bounty > 0)
       {
        sprintf(buf,"%s - %ld Platinum\n\r", capitalize(d->character->name), d->character->pcdata->bounty);
        send_to_char(buf,ch);
       }
   }
   return;
  }

  /* Else...with an argument */
  if( (victim = get_char_world(ch,arg)) == NULL)
  {
   send_to_char("There is nobody in the lands by that name.\n\r",ch);
   return;
  }

  if(IS_NPC(victim))
  {
   send_to_char("Not on NPCs.\n\r",ch);
   return;
  }

  if(victim->pcdata->bounty == 0 || victim->pcdata->bounty < 0)
  {
   send_to_char("You cannot target them.\n\r",ch);
   return;
  }

  if(ch->clan == victim->clan)
  {
    send_to_char("I don't think so.\n\r", ch);
    return;
  }

  if(IS_IMMORTAL(victim) || !can_pkill(ch,victim))
  {
   send_to_char("You cannot target them.\n\r",ch);
   return;
  }

  /* Ok..now we have a target */
  sprintf(buf,"{&You are now {!targeting {#%s{&.{x\n\r",victim->name);
  send_to_char(buf,ch);
  ch->pcdata->target = str_dup(victim->name);

  /* Make sure the affect doesn't happen more than once */
/*
  if(is_affected(ch,gsn_target))
   affect_strip(ch,gsn_target);

  af.where    = TO_AFFECTS;
  af.type     = gsn_target;
  af.level    = ch->level;
  af.location = APPLY_REGEN;
  if (victim->pcdata->bounty/5 > 50)
	af.modifier = 50;
  else
	  af.modifier = number_range(50,victim->pcdata->bounty/5);
  af.bitvector = 0;
  af.duration = -1;
  affect_to_char( ch, &af );

  af.location = APPLY_AC;
  if (victim->pcdata->bounty/5 > 50)
  af.modifier = -(50);
  else
    af.modifier = -(number_range(victim->pcdata->bounty/5,50));
  affect_to_char(ch,&af);
*/
  send_to_char("You feel prepared to make an assassination.\n\r",ch);

  return;
}

/* This one is classic....
 * Lunge at your victim and rip their coin purse
 * Grab the money they drop if successful
 * Restricts:
 * - Victim does not need to be targeted
 * - Must use dagger/sword/axe within a weight limit, must be sharp or vorpal
 * - Will only work in PK once Looting is enabled
 * Benefits:
 * - Duh! Money
 * Functionality:
 * - Assassin lunges at coin purse...attempts to rip open
 * - If they fail at picking up the money it goes onto the ground
 */
void do_rip(CHAR_DATA *ch, char *argument)
{

 return;
}

/* To take a token of a murdered victim
 * Restricts:
 * - None
 * Benefits:
 * - Trophy to prove assassination
 * - Receive bounty money once victim is killed
 * Functionality:
 * - Automatically done when victim is killed
 * - Chance of several bodyparts..will be preserved
 * - Cannot be used for voodoo
 */
void do_mutilate(CHAR_DATA *ch, CHAR_DATA *victim)
{
// OBJ_DATA *obj;
// int vnum;
// char *name;
// char buf[MSL/10];
 if(IS_NPC(victim))
	 return;
 /*
  * Clan specific code.  Removed on 12/30/2004
  */
 /*
 if(str_cmp(clan_table[ch->clan].name,"fortune"))
  return;

 if(IS_NPC(victim))
  return;

 act("$n violently mutilates $N and severs a bodypart.",ch,NULL,victim,TO_NOTVICT);
 act("You violently mutilate $N and sever a bodypart.",ch,NULL,victim,TO_CHAR);
 act("$n violently mutilates you and severs a bodypart.",ch,NULL,victim,TO_VICT);

 vnum       = OBJ_VNUM_SEVERED_HEAD;
 obj        = create_object( get_obj_index( vnum ), 0 );
 obj->timer = 999;
 name       = victim->name;

 sprintf( buf, obj->short_descr, name );
 free_string( obj->short_descr );
 obj->short_descr = str_dup( buf );

 sprintf( buf, obj->description, name );
 free_string( obj->description );
 obj->description = str_dup( buf );

 sprintf( buf, obj->name, name );
 free_string( obj->name );
 obj->name = str_dup( buf );  

 obj->value[4] = 0; // Make sure you can't make a voodoo
 obj_to_char( obj, ch);

 send_to_char("You safely hide your trophy.\n\r",ch);

 affect_strip(ch,gsn_target);
 */
 send_to_char("Target successfully Assassinated.\n\r",ch);
 ch->platinum += victim->pcdata->bounty;
 ch->pcdata->target = NULL;
 victim->pcdata->bounty = 0;
 send_to_char("You no longer have a bounty on your head.\n\r",victim);
 return;
}

/* Cheap Shot
 * Aim at a bodypart that can be broken, thus slowing and hurting opponent
 * Restricts:
 * - Can only use once in a fight successfully
 * - Assassin becomes vulnerable for one round after if miss occurs
 * Benefits:
 * - Opponent is disabled, possible stun
 * Functionality:
 * - Grab your victim..random bodypart, attempt to break
 * - Check victim constitution, assassin's strength, weapon weight
 * - Check stance, speed of both
 */

/* ADJUSTED BY DUSK TO BE USEABLE AND BALANCED */

void do_cripple(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA  af;
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    int chance, dam,hit,dammod;
    
    dammod = get_curr_stat( ch, STAT_STR ) + ch->level;

    one_argument(argument,arg);

    if ( ( chance = get_skill(ch,gsn_cripple) ) <= 0
           || ( !IS_NPC(ch) && !has_skill(ch,gsn_cripple) ) )
    {
        send_to_char("You do not possess the knowledge of crippling someone\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' )
    {
        victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char("But you aren't fighting anyone!\n\r",ch);
            return;
        }
    }
    else if ( ( victim = get_char_room(ch,arg) ) == NULL )
    {
       send_to_char("They aren't here.\n\r",ch);
       return;
    }

    if ( is_safe(ch,victim) )
       return;

    if ( is_affected(victim, gsn_cripple) )
    {
       send_to_char("They are still suffering from being recently crippled.\n\r",ch);
       return;
    }

    if ( victim == ch )
    {
       send_to_char("Amusing, but not possible.\n\r",ch);
       return;
    }

    if ( IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim )
    {
       act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
       return;
    }

    /* modifiers */

    chance = 50;
    
    /* speed */

    if ( IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE) )
        chance += 20;
    if ( IS_SET(ch->stance_aff, STANCE_AFF_SPEED) )
        chance += 15;

    chance = number_fuzzy(chance);

    /* now the attack */

    if ( number_percent() < chance )
    {
       hit = number_percent();
 
       if ( hit < 33 )
          hit = 1;
       else if ( hit < 66 )
          hit = 2;
       else
          hit = 3;

       switch ( hit )
       {
              /* Skull Shot */
              case 1:

              act("$n strikes you in the back of the skull!",ch,NULL,victim,TO_VICT);
              act("You strike $N in the back of the skull!",ch,NULL,victim,TO_CHAR);
              act("$n strikes $N in the back of the skull!",ch,NULL,victim,TO_NOTVICT);

              dam = ( ch->level );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( dammod, ch->level * 2);
              dam += number_range( dammod, ch->level * 2);
              dam += number_range( dammod, ch->level * 2);
              dam += number_range( dammod, ch->level * 2);
              dam += number_range( dammod, ch->level * 2);
              dam += number_range( ch->level/2, ch->level );
              
              if ( (get_curr_stat( victim, STAT_DEX ) + (get_curr_stat( victim, STAT_CON ) *3/2))
                   < number_percent() + ( get_curr_stat( ch, STAT_DEX ) + get_curr_stat( ch, STAT_STR ) ) / 2 )
              {      
                 damage( ch,victim,dam,gsn_cripple,DAM_BASH,TRUE,0 );
             
                 af.where     = TO_AFFECTS;
                 af.type      = gsn_cripple;
                 af.level     = ch->level;
                 af.bitvector = AFF_BLIND;
                 af.duration  = 1;
                 af.modifier  = -5;
                 af.location  = APPLY_DEX;

                 affect_to_char( victim, &af );

                 af.modifier  = 0;
                 af.location  = APPLY_CRIPPLE_HEAD;

                 affect_to_char( victim, &af );

                 send_to_char("{RYou are blinded by the crippling effects on your skull{x!\n\r",victim);
              }
              else
                 damage( ch, victim, dam/2, gsn_cripple,DAM_BASH,TRUE,0 );

              if ( !IS_NPC(ch) && !IS_NPC(victim)
                  && ( ch->fighting == NULL || victim->fighting == NULL ) )
              {
                 sprintf(buf,"Help! %s is trying to break my skull!",PERS(ch,victim));
                 do_yell( victim, buf );
              }
              break;

              /* Leg Shot */

              case 2:

              act("$n strikes and fractures your leg!",ch,NULL,victim,TO_VICT);
              act("You strike $N and fracture their leg!",ch,NULL,victim,TO_CHAR);
              act("$n strikes $N and fractures their leg!",ch,NULL,victim,TO_NOTVICT);

              dam = ( ch->level );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( ch->level/2, ch->level );

              damage( ch,victim,dam,gsn_cripple,DAM_BASH,TRUE,0 );

              af.where     = TO_AFFECTS;
              af.type      = gsn_cripple;
              af.level     = ch->level;
              af.bitvector = 0;
              af.duration  = 1;
              af.modifier  = -9;
              af.location  = APPLY_DEX;

              affect_to_char( victim, &af );

              af.modifier  = 0;
              af.location  = APPLY_CRIPPLE_LEG;

              affect_to_char( victim, &af );

              victim->move -= number_range(victim->move / 2,victim->move);

              if ( victim->move < 0 )
                 victim->move = 0;

              if ( !IS_NPC(ch) && !IS_NPC(victim)
                  && ( ch->fighting == NULL || victim->fighting == NULL ) )
              {
                 sprintf(buf,"Help! %s is trying to break my leg!",PERS(ch,victim));
                 do_yell( victim, buf );
              }

              break;
               
              /* Arm Shot */

              case 3: 
 
              act("$n strikes and fractures your arm!",ch,NULL,victim,TO_VICT);
              act("You strike $N and fracture their arm!",ch,NULL,victim,TO_CHAR);
              act("$n strikes $N and fractures their arm!",ch,NULL,victim,TO_NOTVICT);

              dam = ( ch->level );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( dammod, ch->level * 2 );
              dam += number_range( ch->level/5, ch->level/2 );

              damage( ch,victim,dam,gsn_cripple,DAM_BASH,TRUE,0 );

              af.where     = TO_AFFECTS;
              af.type      = gsn_cripple;
              af.level     = ch->level;
              af.bitvector = 0;
              af.duration  = 1;
              af.modifier  = -5;
              af.location  = APPLY_STR;

              affect_to_char( victim, &af);
            
              af.modifier  = 0;
              af.location  = APPLY_CRIPPLE_ARM;

              affect_to_char( victim, &af );

              if ( get_curr_stat( victim, STAT_STR ) + get_curr_stat( victim, STAT_CON ) * 2
                   < get_curr_stat( ch, STAT_STR ) + number_percent() )
              {
                 OBJ_DATA *obj, *secondary;

                 if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) != NULL
                        && !IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
                 {
                    obj_from_char( obj );

                    act("Your weapon falls from your grasp!",ch,NULL,victim,TO_VICT);
                    act("$N's weapon falls from $m grasp!",ch,NULL,victim,TO_NOTVICT);
                 
                    obj_to_char( obj, victim ); // All wpns go to inventory till looting in pk Dusk

                    /*  TOOK OUT DROPPING TO GROUND CAUSE OF NOLOOTING DUSK 

                    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) || IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
                       obj_to_char( obj, victim );
                    else
                    {
                       obj_to_room( obj, victim->in_room );
                   
                       if ( IS_NPC(victim) && victim->wait == 0 && can_see_obj( victim, obj ) )
                          get_obj( victim,obj,NULL );
                    }
                        END OF PART OF BEING TAKEN OUT DUE TO NO LOOTING DUSK */

                    if ( ( secondary = get_eq_char( victim, WEAR_SECONDARY ) ) != NULL )
                    {
                       unequip_char( victim, secondary );
                       equip_char( victim, secondary, WEAR_WIELD );
                    }
                 }
              }

              if ( !IS_NPC(ch) && !IS_NPC(victim) 
                   && ( ch->fighting == NULL || victim->fighting == NULL ) )
              {
                 sprintf(buf,"Help! %s is breaking my arm!",PERS(ch,victim));
                 do_yell( victim, buf );
              }
              break;

              default:
 
              send_to_char("You strike trying to break a bone but miss.\n\r",ch);
              check_improve( ch,gsn_cripple,FALSE,1 );
              return;
       }
       check_improve( ch,gsn_cripple,TRUE,1 );
       WAIT_STATE( ch,skill_table[gsn_cripple].beats );
    }
    else
    {
	if ( !IS_NPC(ch) && !IS_NPC(victim) 
	    && ( ch->fighting == NULL || victim->fighting == NULL ) )
	{
           sprintf(buf,"Help! %s just tried to break my bones!",PERS(ch,victim));
           do_yell( victim, buf );
	}

        damage( ch,victim,0,gsn_cripple,DAM_BASH,TRUE,0 );

        WAIT_STATE( ch,skill_table[gsn_cripple].beats*2/3 );
        check_improve( ch,gsn_cripple,FALSE,1 );
    } 

    return;
}

void do_fires(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *ich;
  AFFECT_DATA af;

  if (get_skill(ch,gsn_fires) <= 0
  || !has_skill(ch,gsn_fires))
  {
     send_to_char("Huh?\n\r",ch);
     return;
  }

  if(str_cmp(clan_table[ch->clan].name,"torment"))
  {
   send_to_char("Huh?\n\r",ch);
   return;
  }
    if(IN_ARENA(ch))
    {
    send_to_char("No crashy crashy the mud!",ch);
    return;
    }
    if(IS_SET(ch->in_room->affected_by,ROOM_AFF_FIRES) )
    {
     send_to_char("This room is already engulfed in flames.\n\r",ch);
     return;
    }

    act( "{7$n calls forth the {!f{1i{!r{1e{!s{7 of {r-{RT{Dor{rM{Den{Rt{r-{7, flames engulf the room.{x", ch, NULL, NULL, TO_ROOM );
    send_to_char( "{7You call forth the {!f{1i{!r{1e{!s{7 of {r-{RT{Dor{rM{Den{Rt{r-{7, flames engulf the room.{x\n\r", ch );

    af.where = TO_ROOM_AFF;
    af.type = skill_lookup("fires");
    af.duration = ch->pcdata->clan_rank+1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.level = ch->level;
    af.bitvector = ROOM_AFF_FIRES;
    affect_to_room( ch->in_room, &af);

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
        if (ich->invis_level > 0)
                continue;

	if(!str_cmp(clan_table[ich->clan].name,"torment"))
		continue;

	if (IS_NPC(ich) || !is_pkill(ich) || is_safe(ch,ich))
		continue;

        if ( saves_spell( ch->level, ich, DAM_OTHER) )
                continue;

	fire_effect(ich,ch->level,ch->pcdata->clan_rank*100,TARGET_CHAR);
	damage_old( ch, ch, ch->pcdata->clan_rank*100, gsn_on_fire,DAM_FIRE,FALSE);
    }

    return;

}
