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

/************************************************************************
*	Asgard implemented modifications on ROT 1.4, code by            *
*	Chris Langlois.  Copyright 1999.				*
************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "clan.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_wear		);
DECLARE_DO_FUN(do_say           );

/*
 * Local functions.
 */
void	sing_song	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
int skill_lookup( const char *name );
int find_spell( CHAR_DATA *ch, const char *name );
int slot_lookup( int slot );
int mana_cost (CHAR_DATA *ch, int min_mana, int level);

void sing_song( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    CHAR_DATA *rch;

    sprintf( buf,  "$n begins singing, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( buf, ch, NULL, rch, TO_VICT );
    }

    return;
}

char *target_name;
char *third_name;

void do_sing( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    if ( IS_NPC(ch) )
	return;

    if ( str_cmp(class_table[ch->class].name,"bard") )
    {
	send_to_char( "You aren't a bard, what are you thinking?\n\r",ch);
	return;
    }

    target_name = one_argument( argument, arg1 );
    third_name = one_argument( target_name, arg2 );
    strcpy( target_name, arg2 );
    one_argument( third_name, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Sing what?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
	    act("You attempt to sing, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
	    act("$n attempts to sing, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
	    WAIT_STATE(ch,DEF_FEAR_WAIT);
	    return; }

    if (ch->stunned)
    {
        send_to_char("You're in no condition to be singing.\n\r",ch);
        return;
    }

    if ( ( sn = find_spell( ch,arg1 ) ) < 0
    || ( !IS_NPC(ch) && ((ch->level < skill_table[sn].skill_level[ch->class]
    && ch->level < LEVEL_HERO+1)
    ||   		 ch->pcdata->learned[sn] == 0)))
    {
	send_to_char( "You don't know any songs of that name.\n\r",
	 ch );
	return;
    }
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You're a bit busy for that.\n\r", ch );
	return;
    }

    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if ( IS_SET(ch->in_room->room_flags, ROOM_SHOP) )
     {
        CHAR_DATA *keeper;
        SHOP_DATA *pShop;
        pShop = NULL;

        for ( keeper = ch->in_room->people; keeper; 
                 keeper = keeper->next_in_room )
        {
            if ( IS_NPC(keeper) && 
                (pShop = keeper->pIndexData->pShop) != NULL )
            break;
        }


        if ( keeper != NULL )
        {
            do_say( keeper, "{_I'll have none of that racket in here.{x" );
            ch->mana -= mana / 3;
            return;
        }
     }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
       {
          char buf[MAX_STRING_LENGTH];
          sprintf( buf,
             "%s gurgles some unrecognizable noise.", ch->name );
          act(buf,ch,NULL,NULL,TO_ROOM);
          sprintf( buf, "You gurgle some unrecognizable noise." );
          act(buf,ch,NULL,NULL,TO_CHAR);
          ch->mana -= mana;
          return;
       }

    if (!strcmp(skill_table[sn].name, "restore mana") )
	mana = 1;

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_sing: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Sing about who?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	if ( !IS_NPC(ch) )
	{

            if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}


	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot sing this song about another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the song be sung about?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Sing about whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }


	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {


            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

    case TAR_OBJ_TRAN:
	if (arg2[0] == '\0')
	{
	    send_to_char("Transport what to whom?\n\r",ch);
	    return;
	}
	if (arg3[0] == '\0')
	{
	    send_to_char("Transport it to whom?\n\r",ch);
	    return;
	}
        if ( ( obj = get_obj_carry( ch, target_name ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return;
        }
	if ( ( victim = get_char_world( ch, third_name ) ) == NULL
	|| IS_NPC(victim) )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if ( !IS_NPC(ch) && ch->mana < mana )
	{
	    send_to_char( "You don't have enough mana.\n\r", ch );
	    return;
	}
	if ( obj->wear_loc != WEAR_NONE )
	{
	    send_to_char( "You must remove it first.\n\r", ch );
	    return;
	}
	if (IS_SET(victim->act,PLR_NOTRAN)
	&& ch->level < SQUIRE )
	{
	    send_to_char( "They don't want it.\n\r", ch);
	    return;
	}
	if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
	{
	    act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	    return;
	}
	if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
	{
	    act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	    return;
	}
	if ( !can_see_obj( victim, obj ) )
	{
	    act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	    return;
	}

	WAIT_STATE( ch, skill_table[sn].beats );
	if ( !can_drop_obj( ch, obj ) || IS_OBJ_STAT(obj,ITEM_QUEST)
	|| ( obj->item_type == ITEM_PASSBOOK ) )
	{
	    send_to_char( "It seems happy where it is.\n\r", ch);
	    check_improve(ch,sn,FALSE,1);
	    ch->mana -= mana / 3;
	    return;
	}
	if ((obj->pIndexData->vnum == OBJ_VNUM_VOODOO)
	&& (ch->level <= HERO))
	{
	    send_to_char( "You can't transport voodoo dolls.\n\r",ch);
	    check_improve(ch,sn,FALSE,1);
	    ch->mana -= mana / 3;
	    return;
	}

    if ((obj->item_type == ITEM_CONTAINER) ||
        (obj->item_type == ITEM_CORPSE_NPC) ||
        (obj->item_type == ITEM_CORPSE_PC))
    {
        if (find_voodoo( NULL, obj->contains))
        {
            if (ch->level < SUPREME)
            {
                send_to_char( "You can't transport a container that holds voodoo dolls.\n\r", ch);
                return;
            }
            else
                send_to_char( "Warning! You just transported an object containing one or more voodoo dolls.\n\r", ch);
        }
    }

	if ( number_percent( ) > get_skill(ch,sn) )
	{
	    send_to_char( "You lost your concentration.\n\r", ch );
	    check_improve(ch,sn,FALSE,1);
	    ch->mana -= mana / 2;
	}
	else
	{
	    ch->mana -= mana;
	    obj_from_char( obj );
	    obj_to_char( obj, victim );
	    act( "$p glows {Ggreen{x, then disappears.", ch, obj, victim, TO_CHAR);
	    act( "$p suddenly appears in your inventory.", ch, obj, victim, TO_VICT);
	    act( "$p glows {Ggreen{x, then disappears from $n's inventory.", ch, obj, victim, TO_NOTVICT);
	    check_improve(ch,sn,TRUE,1);
	    if (IS_OBJ_STAT(obj,ITEM_FORCED)
	    && (victim->level <= HERO) ) {
		do_wear(victim, obj->name);
	    }
	}
	return;
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
	return;
    }
      
    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	sing_song( ch, sn );
      
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( number_percent( ) > get_skill(ch,sn) )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
        ch->mana -= mana;
        if (IS_NPC(ch) || class_table[ch->class].fMana) 
	/* class has spells */
            (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        else
            (*skill_table[sn].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
        check_improve(ch,sn,TRUE,1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
    return;
}

/*
 * Song functions.
 */

void song_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{  
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    CHAR_DATA *check;

    if (!IS_NPC(victim))
    {
        send_to_char("You can't seem to charm them.\n\r",ch);
        return;
    }

    /*  

    if (victim->master != ch)
    {
        send_to_char("You can't seem to charm them.\n\r",ch);
        return;
    }

    */

    if (IS_SET(victim->imm_flags,IMM_CHARM))
    {
       send_to_char("You can't seem to charm them.\n\r",ch);
       return;
    }
    
    if (level+31 < victim->level)
    {
       send_to_char("They Laugh at your song mercilessly.\n\r",ch);
       return;
    }

    if (is_affected(victim,sn))
    {
        send_to_char("They are already charmed by a song.\n\r",ch);
        return;
    }

    for (check = char_list; check != NULL; check = check->next)
    {
        if (!IS_NPC(check))
            continue;

        if (check->master == ch && is_affected(check,sn))
        {
            send_to_char("You have already Charmed someone with your music.\n\r",ch);
            return;
        }
    }

    af.where = TO_AFFECTS;
    af.location = APPLY_DAMROLL;
    af.modifier = 50;
    af.duration = 50;
    af.bitvector = AFF_CHARM;
    af.level = level;
    af.type = sn;
    affect_to_char(victim,&af);

    SET_BIT(victim->act,ACT_PET);
    ch->pet = victim;
    victim->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;
    add_follower(victim, ch);
    victim->leader = ch;
    act("$N is lulled into submission by your singing.",ch,0,victim,TO_CHAR);
    act("$N seems to be in a trance.",ch,0,victim,TO_NOTVICT);
    return;  

                   
    /*
       COMMENTED OUT SECTION, POSSIBLY OLDER OR FIREST VERSION OF IT
       DUSK
    */

    /*

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch,victim)) return;

    if ( victim == ch )
    {
        send_to_char( "You sway with the melody of your song.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level+20 < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level+10, victim,DAM_CHARM) )
    {
        act("You sing sweetly to $N but they resist your charm.",ch,NULL,victim,TO_CHAR);
	return;
    }
  
    if ( victim->master != NULL )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level+10;
    af.duration  = number_fuzzy( level / 10 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    act( "You are lulled into submission by $n's singing.", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
        act("$N is lulled into submission by your singing.",ch,NULL,victim,TO_CHAR);
    return;

    */
}

void song_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    act("You start singing a calming melody....",ch,NULL,NULL,TO_CHAR);

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;
	    
            act("$n's singing calms you.\n\r",ch,NULL,vch,TO_VICT);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
  	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void song_sonic_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int bonus;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    act("You start singing in a very high pitch.",ch,NULL,NULL,TO_CHAR);
    act("$n starts singing in a very high pitch.",ch,NULL,NULL,TO_ROOM);

    bonus = ch->magic[MAGIC_BLUE]/75;

    if(number_bits(1) == 0)
     dam = dice( level * 5, 13 )* bonus;
    else
     dam = dice( level * 3, 13 );

    if ( saves_spell( level+10, victim, DAM_SOUND ) )
	dam /= 2;

    damage( ch, victim, dam, sn, DAM_SOUND,TRUE,0);

    if(number_bits(2) == 0)
    {
     act("Your eardrums burst!!!",ch,NULL,victim,TO_VICT);
     dam = dice( level * 4, 15 );
     damage( ch, victim, dam, sn, DAM_SOUND,TRUE,0);
    }
     improve_magic (2,ch);
    return;
}

//rage song by Bree
void song_rage( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, chance;
    AFFECT_DATA af;
    
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    act("You start singing in a very angry tone!",ch,NULL,NULL,TO_CHAR);
    act("$n starts singing in a very angry tone!",ch,NULL,NULL,TO_ROOM);
    dam = 1;
    chance = 80;
    if ( saves_spell( level+10, victim, DAM_SOUND ) )
    {
	    chance = chance/2;
    }

    
    if(number_bits(1) == 0)
    {
     dam = dice( level * 2, 12 );
	}
    else
    {
     dam = dice( level * 2, 10 );
	}
     
    if ( saves_spell( level+10, victim, DAM_SOUND ) )
    {
	dam /= 2;
	}
    damage( ch, victim, dam, sn, DAM_SOUND,TRUE,0);

    if(number_bits(2) == 0)
    {
     act("Your eyes start to bleed!!!",ch,NULL,victim,TO_VICT);
     dam = dice( level * 8, 18 );
        if (number_percent() < chance)
            {
                 act("Your eyes start to bleed!!!",ch,NULL,victim,TO_VICT);
                 af.where     = TO_AFFECTS;
                 af.type      = gsn_blindness;
                 af.level     = ch->level;
                 af.bitvector = AFF_BLIND;
                 af.duration  = 1;
                 af.modifier  = -5;
                 af.location  = APPLY_DEX;

                 affect_to_char( victim, &af );
      		}
     	else
      		{
	        		act("They appear unaffected by your rage!",ch,NULL,NULL,TO_CHAR);
       		}
    }

    WAIT_STATE(victim,PULSE_VIOLENCE);
    return;
}
