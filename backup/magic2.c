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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "tables.h"
#include "clan.h"
#include "recycle.h"

DECLARE_DO_FUN(do_scan			);
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_get );
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_flee);

BUFFER *    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    bool fShort, bool fShowNothing ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) );

extern char *target_name;
extern char *third_name;
bool check_dispel args ( ( int dis_level, CHAR_DATA *victim, int sn) );
extern void raw_kill args ((CHAR_DATA * victim, CHAR_DATA * killer));
extern void group_gain args ((CHAR_DATA * ch, CHAR_DATA * victim));
int     sorcery_dam  args( ( int num, int dice, CHAR_DATA *ch) );
int 	sorcery_dam2 args( (int dam) );
bool    check_sorcery args( (CHAR_DATA *ch, int sn) );


void spell_farsight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FARSIGHT) )
    {
        if (victim == ch)
          send_to_char("Your eyes are already as good as they get.\n\r",ch);
        else
          act("$N can see just fine.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FARSIGHT;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes jump into focus.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_protection_voodoo(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, SHD_PROTECT_VOODOO) || is_affected(victim, sn) )
    {
	send_to_char("They are already protected from voodoo magic.\n\r",ch);
	return;
    }
    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.duration	= level;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector = SHD_PROTECT_VOODOO;
    affect_to_char( victim, &af );
    return;
}

void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||	(is_clan(victim) && (!is_same_clan(ch,victim)
    ||  clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 

    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }
    if (victim->in_room->area->continent != ch->in_room->area->continent) {
	    send_to_char( "You fail.\n\r",ch);
	    return; }

    printf( "Gating to mob in area %s, flags %d\n",
        victim->in_room->area->name, victim->in_room->area->area_flags );

    if (global_quest && IS_NPC(victim) && victim->on_quest)
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;
 
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) )
    ||	 is_affected(ch,gsn_martial_arts)//Screw you monks! 
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||	 (is_clan(victim) && (!is_same_clan(ch,victim)
    ||   clan_table[victim->clan].independent)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   
 
    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 

    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
 
    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        extract_obj(stone);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}

void spell_empower( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *object;
    char buf[MAX_STRING_LENGTH];
    char *name;
    int new_sn;
    int mana;
    int newmana;
    int newtarget;

    if ( ( new_sn = find_spell( ch,target_name ) ) < 0
    || ( !IS_NPC(ch) && (ch->level < skill_table[new_sn].skill_level[ch->class]
    ||                   ch->pcdata->learned[new_sn] == 0)))
    {
        send_to_char( "What spell do you wish to bind?\n\r", ch );
        return;
    }

    name = skill_table[new_sn].name;
    if (!strcmp(name, "empower") )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    if (skill_table[new_sn].spell_fun == spell_null)
    {
	send_to_char( "That's not a spell.\n\r", ch );
	return;
    }
    newtarget = skill_table[new_sn].target;
    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if (ch->level + 2 == skill_table[new_sn].skill_level[ch->class])
	newmana = 50;
    else
    	newmana = UMAX(
	    skill_table[new_sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[new_sn].skill_level[ch->class] ) );

    if ( (ch->mana - mana - newmana) < 0)
    {
        send_to_char( "You do not have enough mana.\n\r", ch );
        return;
    }
    ch->mana -= newmana;

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	object = create_object(get_obj_index(OBJ_VNUM_POTION), 0);
    }
    else
    {
	object = create_object(get_obj_index(OBJ_VNUM_SCROLL), 0);
    }
    object->value[0] = ch->level;
    object->value[1] = new_sn;
    object->level = ch->level-5;

    sprintf( buf, "%s %s", object->name, name);
    free_string(object->name);
    object->name = str_dup(buf);

    sprintf( buf, "%s%s", object->short_descr, name);
    free_string(object->short_descr);
    object->short_descr = str_dup(buf);

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	sprintf( buf, "$n has created a potion of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a potion of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    else
    {
	sprintf( buf, "$n has created a scroll of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a scroll of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    obj_to_char(object,ch);
    return;
}

//	Quench Spell by Bree
/*
	void spell_quench( int sn, int level, CHAR_DATA *ch, void *vo )
	{

    if ( IS_NPC(ch) )
	return;
  
    ch->pcdata->condition[COND_THIRST] = 30;
    send_to_char( "You have quenched your thirst.\n\r", ch );
    return;

 }

//	Sate spell by Bree
 
	void spell_sate( int sn, int level, CHAR_DATA *ch, void *vo ) 
	{
    
		if ( IS_NPC(ch) )
		return;
  
    ch->pcdata->condition[COND_HUNGER] = 24;
    send_to_char( "You have sated your hunger.\n\r", ch );
    return;
 }
*/
 
//Betray Spell by Bree

/* void spell_betray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   if ( !ch->fighting )
   {
	send_to_char( "You may only cast betray during combat.", ch );

    if ( victim == ch )
    {
	send_to_char( "Betray yourself?  You're weird.\n\r", ch );
	return;
    }

    if (   IS_AFFECTED( victim, AFF_CHARM )
	|| IS_AFFECTED( ch,     AFF_CHARM )
	|| level < victim->level
	|| saves_spell( level, victim ) )
        return;

    if ( victim->fighting == ch )
	stop_fighting( victim, TRUE );
    if ( victim->master )
        stop_follower( victim );
    add_follower( victim, ch );

    af.type	 = sn;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    act( "$N has betrayed!", ch, NULL, victim, TO_CHAR );
    act( "You now follow $n!", ch, NULL, victim, TO_VICT );
    return;
}
*/
void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj;
    OBJ_DATA *cobj;
    OBJ_DATA *obj_next;
    CHAR_DATA *pet;
    int	length;

    if ( ( obj = get_obj_here( ch, "corpse" ) ) == NULL )
    {
	send_to_char( "There's no corpse here.\n\r", ch );
	return;
    }
    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }
    pMobIndex = get_mob_index( MOB_VNUM_CORPSE );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) ) 
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) ) 
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
	pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( str_replace(obj->short_descr, "corpse", "zombie") );
    sprintf( buf, "%s", str_replace(obj->description, "corpse", "zombie") );
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s standing here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = UMAX(1, UMIN(109, ((ch->level/2)+(obj->level))));
    pet->max_hit = pet->level * 20;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level;
    pet->armor[1] = pet->level;
    pet->armor[2] = pet->level;
    pet->armor[3] = pet->level;
    for ( cobj = obj->contains; cobj != NULL; cobj = obj_next )
    {
	obj_next = cobj->next_content;
	obj_from_obj( cobj );
	obj_to_room( cobj, ch->in_room );
    }
    extract_obj( obj );
    sprintf( buf, "%s stands up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s stands up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_conjure( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *stone;
    
 
    if (IS_NPC(ch))
	return;

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_DEMON_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }

    /*if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
	if (stone->value[0] < 1)
	{
	    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
	    act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_CHAR);
	    act("It flares brightly and explodes into dust.",ch,stone,NULL,TO_ROOM);
	    extract_obj( stone );
	    return;
	}
    }*/
     if(ch->class == CLASS_VOODAN || ch->class == CLASS_FORSAKEN
      || ch->class == CLASS_WIZARD || ch->class == CLASS_MAGE)
      {
	       CHAR_DATA *pet;
    pMobIndex = get_mob_index( MOB_VNUM_DEMON );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        /*stone->value[0] = UMAX(0, stone->value[0]-1);*/
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
        extract_obj( stone );
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level*3/2;
    pet->damroll = ch->level*4;
    pet->max_hit = pet->level * 80;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level;
    pet->armor[1] = pet->level;
    pet->armor[2] = pet->level;
    pet->armor[3] = pet->level;
}    
if(ch->class == CLASS_ALCHEMIST || ch->class == CLASS_SHAMAN || ch->class == CLASS_SAGE)
      {
	       CHAR_DATA *pet;
    pMobIndex = get_mob_index( 3195 );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        /*stone->value[0] = UMAX(0, stone->value[0]-1);*/
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
        extract_obj( stone );
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level*3/2;
    pet->damroll = ch->level*4;
    pet->max_hit = pet->level * 80;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level;
    pet->armor[1] = pet->level;
    pet->armor[2] = pet->level;
    pet->armor[3] = pet->level;
} 
if(ch->class == CLASS_SAINT || ch->class == CLASS_PRIEST )
      {
	       CHAR_DATA *pet;
    pMobIndex = get_mob_index( 3196 );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        /*stone->value[0] = UMAX(0, stone->value[0]-1);*/
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
        extract_obj( stone );
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level*3/2;
    pet->damroll = ch->level*4;
    pet->max_hit = pet->level * 80;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level;
    pet->armor[1] = pet->level;
    pet->armor[2] = pet->level;
    pet->armor[3] = pet->level;
}
if(ch->class == CLASS_CONJURER)
      {
	       CHAR_DATA *pet;
    pMobIndex = get_mob_index( 3197 );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        /*stone->value[0] = UMAX(0, stone->value[0]-1);*/
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
        extract_obj( stone );
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level*3/2;
    pet->damroll = ch->level*4;
    pet->max_hit = pet->level * 95;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level;
    pet->armor[1] = pet->level;
    pet->armor[2] = pet->level;
    pet->armor[3] = pet->level;
}           
    

    return;
}



void spell_animate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *pet;
    int	length;

    if  ((obj->pIndexData->vnum > 17)
	|| (obj->pIndexData->vnum < 12))
    {
	send_to_char( "That's not a body part!\n\r", ch );
	return;
    }

    pMobIndex = get_mob_index( MOB_VNUM_ANIMATE );
    pet = create_mobile( pMobIndex );
    SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GIt's branded with the mark of %s.{x.\n\r",
	obj->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( obj->short_descr );
    free_string( pet->name );
    pet->name = str_dup( obj->name );
    sprintf( buf, "%s", obj->description);
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s floating here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    obj_from_char( obj );
    sprintf( buf, "%s floats up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s floats up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_iceshield( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_ICE))
    {
	if(victim == ch)
	    send_to_char("You are already surrounded by an {Cicy{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by an {Cicy{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
	act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Cicy{x shield quickly melts away.\n\r", victim);
	act("$n's {Cicy{x shield quickly melts away.",victim, NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ICE;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
   act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM);
   return;
}

void spell_fireshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
   
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_FIRE))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a {Rfirey{x shield.\r\n", ch);
	else
	    act("$N is already surrounded by a {Rfiery{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
	act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Rfirey{x shield gutters out.\n\r", victim);
	act("$n's {Rfirey{x shield gutters out.",victim, NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_FIRE;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
    act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM);
    return;

}

void spell_shockshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_SHOCK))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded in a {Bcrackling{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by a {Bcrackling{x shield.",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by a {Bcrackling{x shield.\n\r", victim);
	act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Bcrackling{x shield sizzles and fades.\n\r", victim);
	act("$n's {Bcrackling{x shield sizzles and fades.",victim, NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHOCK;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Bcrackling{x field.\n\r",victim);
    act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_deathshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_DEATH))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded in a {Ddeath{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by a {Ddeath{x shield.",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by a {Ddeath{x shield.\n\r", victim);
	act("$n is surrounded by a {Ddeath{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Ddeath{x shield sizzles and fades.\n\r", victim);
	act("$n's {Ddeath{x shield sizzles and fades.",victim, NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_DEATH;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Ddeath{x field.\n\r",victim);
    act("$n is surrounded by a {Ddeath{x shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_quest_pill( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_NPC(victim))
	return;

    victim->qps++;
    send_to_char( "{YYou've gained a {RQuest Point{Y!{x\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_voodoo( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *bpart;
    OBJ_DATA *doll;

    bpart = get_eq_char(ch,WEAR_HOLD);
    if  ((bpart == NULL)
    ||   (bpart->pIndexData->vnum < 12)
    ||   (bpart->pIndexData->vnum > 17))
/*    || bpart->pIndexData->vnum != 30) */
    {
	send_to_char("You are not holding a body part.\n\r",ch);
	return;
    }
    if (bpart->value[4] == 0)
    {
	send_to_char("This body part is from a mobile.\n\r",ch);
	return;
    }
    one_argument(bpart->name, name);
    doll = create_object(get_obj_index(OBJ_VNUM_VOODOO), 0);
    sprintf( buf, doll->short_descr, name );
    free_string( doll->short_descr );
    doll->short_descr = str_dup( buf );
    sprintf( buf, doll->description, name );
    free_string( doll->description );
    doll->description = str_dup( buf );
    sprintf( buf, doll->name, name );
    free_string( doll->name );
    doll->name = str_dup( buf );
    act( "$p morphs into a voodoo doll",ch,bpart,NULL,TO_CHAR);
    obj_from_char( bpart );
    obj_to_char(doll,ch);
    equip_char(ch,doll,WEAR_HOLD);
    act( "$n has created $p!", ch, doll, NULL, TO_ROOM );
    return;
}


/* by Airius WWW */
void send_hue_mess(char *clmess, char *clcode, CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf[MAX_STRING_LENGTH];
        CHAR_DATA   *vch;
    CHAR_DATA   *vch_next;

    if(IS_SET(ch->act, PLR_COLOUR))
        sprintf(buf, "%sA %s hue strikes you!%s\n\r", clcode, clmess, CLEAR);
    else
        sprintf(buf,"A %s hue strikes you!\n\r", clmess);
    send_to_char(buf, victim);

    for ( vch = char_list; vch!=NULL; vch = vch_next )
    {
    vch_next=vch->next;
    if (vch->in_room == NULL)
        continue ;
    if ( vch->in_room == ch->in_room )
    {
        if ( vch != victim )
        {
            if(IS_SET(vch->act, PLR_COLOUR))
              sprintf(buf, "%sA %s hue strikes %s!%s\n\r", clcode, clmess, victim->short_descr, CLEAR);
            else
              sprintf(buf, "A %s hue strikes0%s!Ln\r", clmess, victim->short_descr);
            send_to_char(buf, vch);
        }
        continue ;
    }
    }

}

/* by Airius WWW */
void strike_with_hue( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim)
{
    int dtoss;
    ROOM_INDEX_DATA *pRoomIndex;
    AFFECT_DATA af;

    dtoss=dice(1,7);
    switch (dtoss)
    {
    case 1 :
      send_hue_mess("red", C_B_RED, ch, victim);
      damage_old(ch, victim, saves_spell( level, victim, DAM_FIRE) ? 50 : 25, sn, DAM_FIRE, TRUE);
     break;
    case 2 :
      send_hue_mess("cyan", C_B_CYAN, ch, victim);
      damage_old(ch, victim, saves_spell( level, victim, DAM_ENERGY) ? 100 : 50, sn, DAM_ENERGY, TRUE);
     break;
    case 3 :
      send_hue_mess("yellow", C_B_YELLOW, ch, victim);
      damage_old(ch, victim, saves_spell( level, victim, DAM_LIGHT) ? 150 : 75, sn, DAM_LIGHT, TRUE);
     break;
    case 4 :
      send_hue_mess("green", C_B_GREEN, ch, victim);
     if(!saves_spell(level, victim, DAM_POISON))
     {
     af.where     = TO_AFFECTS;
     af.type      = sn;
     af.level     = level;
     af.duration  = level;
     af.location  = APPLY_STR;
     af.modifier  = -4;
     af.bitvector = AFF_POISON;
     affect_join( victim, &af );
     send_to_char( "You feel very sick.\n\r", victim );
     act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
     }
     else
      damage_old(ch, victim, 20, sn, DAM_POISON, TRUE);
     break;
    case 5 :
      send_hue_mess("blue", C_B_BLUE, ch, victim);
     if ( victim->in_room == NULL
     || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
     || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
     || ( victim != ch && ( saves_spell( level - 10, victim,DAM_OTHER))))
     {
        break;
     }

     pRoomIndex = get_random_room(victim);

     if (victim != ch)
    send_to_char("You have been teleported!\n\r",victim);

     act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
     char_from_room( victim );
     char_to_room( victim, pRoomIndex );
     act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
     do_look( victim, "auto" );
     break;
    case 6 :
      send_hue_mess("indigo", C_MAGENTA, ch, victim);
      if (saves_spell(level,victim,DAM_OTHER)
     ||  IS_SET(victim->imm_flags,IMM_MAGIC))
     {
    if (victim != ch)
                send_to_char("Nothing seemed to happen.\n\r",ch);
          send_to_char("You feel momentarily lethargic.\n\r",victim);
          break;
     }

     if (IS_AFFECTED(victim,AFF_HASTE))
     {
          if (!check_dispel(level,victim,skill_lookup("haste")))
          {
         if (victim != ch)
                    send_to_char("Spell failed.\n\r",ch);
     affect_to_char( victim, &af );
     send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
     act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
              send_to_char("You feel momentarily slower.\n\r",victim);
                break;
          }

          act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
          break;
     }


     af.where     = TO_AFFECTS;
     af.type      = sn;
     af.level     = level;
     af.duration  = level/2;
     af.location  = APPLY_DEX;
     af.modifier  = -1 - (level/5);
     af.bitvector = AFF_SLOW;
     affect_join( victim, &af );
     send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
     act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
     break;
    case 7 :
      send_hue_mess("violet", C_B_MAGENTA, ch, victim);
      damage_old(ch, victim, saves_spell( level, victim, DAM_NEGATIVE) ? 200 : 150, sn, DAM_NEGATIVE, TRUE);
      send_to_char( "You feel your life slipping away\n\r", victim );
      act("$n seems slightly weaker.",victim,NULL,NULL,TO_ROOM);
     break;
    }
    return ;
}

/* by Airius WWW */
void spell_prismatic_spray( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch, *vch_next;
    int i, number_hits;

    send_to_char( "You put out your hands and send forth a dazzling pristmatic spray!\n\r", ch );
    act( "$n raises $s hands and sends out a dazzling prismatic spray!", ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;

	if ( vch == ch
	||   is_same_group( ch, vch )
	||   is_safe_spell( ch, vch, TRUE ) )
	    continue;

	number_hits  = dice( 1, 8 ) == 8 ? 2 : 1;
	number_hits += dice( 1, 8 ) == 8 ? 2 : 1;

	for ( i = 1; i <= number_hits; i++ )
	    strike_with_hue( sn, level, ch, vch );
    }
}

void spell_wizard_eye( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    EXIT_DATA *pexit;
    int door=0;
    int attempt;
    CHAR_DATA *victim;

    victim = get_char_world( ch, target_name );

    if (IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char("Maybe it would help if you could see?\n\r",ch);
        return;
    }

    if ( victim == NULL ||
           victim->level > LEVEL_HERO )
    {
        send_to_char("Failed.\n\r",ch);
        return;
    }

    /*Unsaveable. Let's do it.
    if (saves_spell(level + 5,victim,DAM_OTHER))
    {
     send_to_char("Failed.\n\r",ch);
     return;
    }*/

    if ( victim == ch )
    {
        send_to_char("You are here!\n\r",ch);
        return;
    }
    if ( IS_AFFECTED(victim, AFF_DETECT_LOCATION) && (saves_spell( victim->level, ch, DAM_OTHER ) || number_percent() < 30))
        act("$N just located you!", victim, NULL, ch, TO_CHAR);

    if ( !IS_AFFECTED(victim, AFF_DISPLACE) || saves_spell( victim->level, ch, DAM_OTHER ) )
    {
	BUFFER *final;
         send_to_char( victim->in_room->name, ch );
         send_to_char( "\n\r  ", ch );
         send_to_char( victim->in_room->description, ch );
         final = show_list_to_char( victim->in_room->contents, ch, FALSE, FALSE );
	 send_to_char( final->string, ch );
	 free_buf( final );
         show_char_to_char( victim->in_room->people,   ch );
         return;
    }

    /* ok. now the hard bit. */

        for ( attempt = 0; attempt < 6; attempt++ )
        {
             door = attempt;
		pexit = victim->in_room->exit[door];
             if ( pexit == 0
               ||   pexit->u1.to_room == NULL
               ||   IS_SET(pexit->exit_info, EX_CLOSED) )
                    door = 7;
               else
                    break;
        }

        if ( door < 7 && door >= 0 )
        {
	   BUFFER * final;
           send_to_char( pexit->u1.to_room->name, ch );
           send_to_char( "\n\r  ", ch );
           send_to_char( pexit->u1.to_room->description, ch );
           final = show_list_to_char( pexit->u1.to_room->contents, ch, FALSE, FALSE );
	   send_to_char( final->string, ch );
	   free_buf( final );
           show_char_to_char( pexit->u1.to_room->people,   ch );

           return;
        }
        else
        {
            send_to_char("Failed.\n\r",ch);
            return;
        }

    return;
}

void spell_detect_location( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_DETECT_LOCATION) )

    {
        send_to_char("You already have a sixth sense.\n\r",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_LOCATION;
    affect_to_char( ch, &af );
    send_to_char("You feel your body becoming more aware of its surroundings.\n\r",ch);
    return;
}

void spell_displace( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_DISPLACE) )
    {
        send_to_char("You are already displaced.\n\r",ch);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DISPLACE;

    affect_to_char( ch, &af );
	send_to_char("You begin to distort the world around you.\n\r",ch);
    
    return;
}

void spell_feeble_mind(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
    if (victim == ch)
      send_to_char("You stare and are not sure quite whats going on!\n\r",ch);
    else
      act("$N already looks pretty stupid.",ch,NULL,victim,TO_CHAR);
    return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_INT;
    af.modifier  = 0 - 8 - (level >= 10) - (level >= 15) - (level >= 20) - (level >= 25) - (level >= 30) - (level >= 35) - (level >= 40);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if(victim->fighting != NULL)
     stop_fighting(victim,TRUE);

    send_to_char( "You forget what you were supposed to be doing!\n\r", victim );
    act("$n's face goes blank and he begins to drool.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_unite(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int totalhits = 0;
    int dam = 0;
    int idice;
    char buf[MSL];

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
        ch->attacker = TRUE;
        victim->attacker = FALSE;
    } 
   send_to_char( "You unite the world under your banner.. CHARGE!\n\r", ch );
   act( "$n raises $s hands to the sky, uniting a global army to serve his ends!",
   ch, NULL, NULL, TO_ROOM );

   for ( vch = char_list; vch != NULL; vch = vch_next )
   {
	vch_next = vch->next; 

	if (ch->mana <= 0)
	{
		ch->mana = 0; // Reset to 0, no negatives.
		continue;
	}	
	
	if (totalhits >= 7)
	 continue;
	 
	if ( vch->level <= LEVEL_HERO
	  && vch != ch
	  && vch != victim
	  && !IS_NPC(vch) )
	{
		idice = number_range(12,18);
		
		if (check_sorcery(ch,sn))
        		dam = sorcery_dam(vch->level,idice,ch);
		else        		
			dam = dice(vch->level,idice);

		sprintf(buf,"The illusionary arm of %s claws madly at %s!",
		 capitalize(vch->name),victim->name);
		act(buf,ch,NULL,victim,TO_CHAR);
		act(buf,ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"The illusionary arm of %s claws madly at you!",
		 capitalize(vch->name));
		act(buf,ch,NULL,victim,TO_VICT);
		
		damage_old (ch, victim, dam*1.3, sn, DAM_MENTAL, TRUE);
		
		ch->mana -= vch->level/5;
		totalhits++;
	}
    }
    WAIT_STATE( ch, ((totalhits+3)*2) );
}

void spell_wrath_of_god (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    char *god = god_table[ch->god].name;
    char buf[MAX_STRING_LENGTH/8];

    if (victim != ch)
    {
        sprintf(buf,"$n calls the wrath of %s down upon $N!",god);
        act(buf,ch,NULL,victim,TO_ROOM);
        sprintf(buf,"You call down the wrath of %s upon $N!",god);
        act(buf,ch,NULL,victim,TO_CHAR);
    }

    /*if ((IS_GOOD(victim) && IS_GOOD(ch)) || (IS_EVIL(victim) && IS_EVIL(ch)))
    {
        act("$n seems unharmed by the wrath.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }*/

    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level,8,ch);
    else
        dam = dice(level, 8);

    if (level <= 25) 
        dam += dice(level, 2);
   else if (level <= 45)
        dam += dice(level, 6);
    else if (level < 65)
        dam += dice(level, 10);
    else if (level < 80)
        dam += dice(level, 14);
    else if (level < 100)
        dam += dice(level*2, 10);
    else
    {
     if(ch->class == CLASS_SAINT)
     {
      if (check_sorcery(ch,sn))
        dam += sorcery_dam(level * 6,12,ch);
      else
        dam += dice(level * 5, 12);
     }
     else
      dam += dice(level * 2, 12);
    }

    if (saves_spell(level,victim,DAM_HOLY) || saves_spell(level + 5, victim,DAM_HOLY))
	dam /= 2;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old(ch,victim,dam,sn,DAM_ENERGY,TRUE);

    if (number_range(0,2) != 0)
	return;
    spell_curse(gsn_curse,level,ch,(void *) victim, TARGET_CHAR);
}

/* Saint Spell  - Skyntil - */
void spell_divine_protection (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
 char *god = god_table[ch->god].name;
 char buf[MAX_STRING_LENGTH/10];
 int resistance = -1;
 AFFECT_DATA af;

 if(is_affected(ch,gsn_divine_protection))
 {
  sprintf(buf,"%s has already given your divine protection.\n\r",god);
  send_to_char(buf,ch);
  return;
 }

 if (!str_cmp(third_name, "bash"))
        resistance = RES_BASH;
 if (!str_cmp(third_name, "pierce"))
        resistance = RES_PIERCE;
 if (!str_cmp(third_name, "slash"))
        resistance = RES_SLASH;
 if (!str_cmp(third_name, "fire"))
        resistance = RES_FIRE;
 if (!str_cmp(third_name, "cold"))
        resistance = RES_COLD;
 if (!str_cmp(third_name, "lightning"))
        resistance = RES_LIGHTNING;
 if (!str_cmp(third_name, "acid"))
        resistance = RES_ACID;
 if (!str_cmp(third_name, "poison"))
        resistance = RES_POISON;
 if (!str_cmp(third_name, "negative"))
        resistance = RES_NEGATIVE;
 if (!str_cmp(third_name, "energy"))
        resistance = RES_ENERGY;
 if (!str_cmp(third_name, "mental"))
        resistance = RES_MENTAL;
 if (!str_cmp(third_name, "disease"))
        resistance = RES_DISEASE;
 if (!str_cmp(third_name, "drowning"))
        resistance = RES_DROWNING;
 if (!str_cmp(third_name, "charm"))
        resistance = RES_CHARM;
 if (!str_cmp(third_name, "sound"))
        resistance = RES_SOUND;
 if (resistance == -1)
 {
   sprintf(buf,"%s doesn't seem to understand your prayer.\n\r",capitalize(god));
   send_to_char(buf,ch);
   return;
 }

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/8;
    af.location  = APPLY_NONE;
    af.modifier  = 1;
    af.bitvector = resistance;
    affect_to_char( ch, &af );
    sprintf(buf,"$n kneels down and utters a prayer to %s.",capitalize(god));
    act( buf, ch, NULL, NULL,TO_ROOM);
    sprintf(buf,"%s protects your from %s.\n\r",capitalize(god),third_name);
    send_to_char( buf, ch );

 return;
}

void spell_hold_align(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
 CHAR_DATA *victim = (CHAR_DATA *) vo;
 AFFECT_DATA af;

 if(victim != ch)
 {
  send_to_char("You cannot cast this on another.\n\r",ch);
  return;
 }

 if(is_affected(victim,gsn_hold_align))
 {
   send_to_char("You are already constant in your worship.",ch);
   return;
 }
 
 if(is_affected(victim,gsn_martial_arts) && (ch->class == CLASS_MONK))
 {
	 send_to_char("You must first put your mind at peace.\n\r",ch);
	 return;
 }

 act("$n looks more sullen.",victim,NULL,NULL,TO_ROOM);
 act("You feel closer to $g.",victim,NULL,NULL,TO_CHAR);

/* Alignment Changes */

 if(victim->alignment > 200)
  if(god_table[ch->god].pc_good)
   victim->alignment = 1000;

 if(victim->alignment < -200 )
  if(god_table[ch->god].pc_evil)
   victim->alignment = -1000;

 if(victim->alignment < 200 && victim->alignment > -200)
  if(god_table[ch->god].pc_neutral)
   victim->alignment = 0;

 af.where = TO_AFFECTS;
 af.location = APPLY_NONE;
 af.type = gsn_hold_align;
 af.modifier = 0;
 af.bitvector = 0;
 af.duration = victim->level/12;
 affect_to_char(victim, &af);

 return;
}
void spell_turn_undead(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
 CHAR_DATA *victim = (CHAR_DATA *) vo;
 int chance,dam;
 COOLDOWN_DATA cd;

 if(victim == ch)
 {
  send_to_char("You feel a zap from above.\n\r",ch);
  return;
 }

 if(victim == NULL)
 return;

 if(IS_NPC(victim))
 {
  if(!IS_SET(victim->act,ACT_UNDEAD))
  {
	send_to_char("Use this on undead.\n\r",ch);
	return;
  }
  chance = get_skill(ch,gsn_turn_undead);
  chance += get_curr_stat(ch,STAT_WIS) * 4;

  if(ch->alignment < 0)
  return;

/*	Cooldown check	*/
	if (ch->fighting != NULL && on_cooldown(ch, gsn_turn_undead))
	{
                send_to_char ("Turn Undead is still on cooldown.\n\r", ch);
                return;
       }


/*	Apply cooldown	*/
	cd.type      = skill_lookup("turn_undead");
	cd.duration	 = skill_table[gsn_turn_undead].cooldown;
        cooldown_on( ch, &cd );

  chance += ch->alignment/10;

  act("$n intones holy words and shadows disperse!",ch,NULL,NULL,TO_ROOM);
  act("You chant holy words and the undead falter.",ch,NULL,NULL,TO_CHAR);

  /* Max of 300 normal around 250 */
  if(number_percent() < chance/8) /* 30-40% */
  {
   act("$N screams as $E crumbles into dust!",ch,NULL,victim,TO_ROOM);
   raw_kill(victim,ch);
   group_gain(ch, victim);

		if (!IS_NPC (ch) && IS_NPC (victim))
		{
                        OBJ_DATA *coins, *corpse;

			corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

			if (IS_SET (ch->act, PLR_AUTOLOOT) &&
				corpse && corpse->contains)		/* exists and not empty */
				do_get (ch, "all corpse");

			if (IS_SET (ch->act, PLR_AUTOGOLD) &&
				corpse && corpse->contains &&	/* exists and not empty */
				!IS_SET (ch->act, PLR_AUTOLOOT))
				if ((coins = get_obj_list (ch, "gcash", corpse->contains))
					!= NULL)
					do_get (ch, "all.gcash corpse");

			if (IS_SET (ch->act, PLR_AUTOSAC)) {
				if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)
                                {
                                        return;
                                }
				else
					do_sacrifice (ch, "corpse");
			}
		}
  }
  else
  {
   dam = dice(20,ch->level);
   damage(ch,victim,dam,gsn_turn_undead,DAM_HOLY,TRUE,0);
  }
 }

 if(!IS_NPC(victim))
 {

/*
  if(!IS_SET(victim->act,ACT_UNDEAD))
  {
	send_to_char("Use this on undead.\n\r",ch);
	return;
  }
*/

  if (!(	victim->class == CLASS_FADE
	    ||	victim->class == CLASS_LICH
		||	victim->class == CLASS_NECROMANCER
		||	victim->class == CLASS_SHADE
		||	victim->class == CLASS_BANSHEE
		||	IS_SET(victim->act,ACT_UNDEAD)))
  {
	send_to_char("Use this on undead.\n\r",ch);
	return;
  }



  chance = get_skill(ch,gsn_turn_undead);
  chance += get_curr_stat(ch,STAT_WIS) * 4;
  chance -= get_curr_stat(victim,STAT_WIS) * 3;

  if(ch->alignment < 0)
  return;

  chance += ch->alignment/10;

  if(victim->alignment > -1)
  return;

  chance -= victim->alignment/20; /* The more evil, the better chance */

  act("$n intones holy words and shadows disperse!",ch,NULL,NULL,TO_ROOM);
  act("You chant holy words and the undead falter.",ch,NULL,NULL,TO_CHAR);

  /* Max of 275 normal around 200 */
  if(number_percent() < chance/20) /* 20-27%   lower now, don't know the %, was /10 now /20*/
  {
   act("You scream in horror as $n turns you back to the grave.",ch,NULL,victim,TO_VICT);
   act("$N screams as $E crumbles into dust!",ch,NULL,victim,TO_NOTVICT);
   raw_kill(victim,ch);

		if (!IS_NPC (victim))
		{
			sprintf (log_buf, "%s killed by %s at %d",
					 victim->name,
					 (IS_NPC (ch) ? ch->short_descr : ch->name),
					 ch->in_room->vnum);
			log_string (log_buf);
		}
  }
  else
  {
   dam = dice(20,ch->level);
   damage(ch,victim,dam,gsn_turn_undead,DAM_HOLY,TRUE,0);
  }
 }
 return;
}
void spell_protective_aura(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
/*  AFFECT_DATA af; */

  if(victim != ch)
  {
   send_to_char("You cannot cast this on another.\n\r",ch);
   return;
  }

  if(is_affected(victim,gsn_protective_aura))
  {
   send_to_char("You are already surrounded by a protective aura",victim);
   return;
  }

  act("$n is surrounded by a golden aura.",victim,NULL,NULL,TO_ROOM);
  act("You are granted protection by $g.",victim,NULL,NULL,TO_CHAR);

  SET_BIT(ch->shielded_by,SHD_PROTECT_AURA);
  ch->shd_aura_timer = 5;
  return;
}

void spell_enforcer(int sn,int level, CHAR_DATA *ch,void *vo,int target)
{
        CHAR_DATA *guard;
        AFFECT_DATA af;
        CHAR_DATA *check;
        int i;

        if (is_affected(ch,sn))
        {
         send_to_char("You cannot call for more enforcers yet.\n\r",ch);
         return;
        }

        if(ch->in_room->sector_type != SECT_CITY)
        {
         send_to_char("You can only summon enforcers within a city.\n\r",ch);
         return;
        }

        for (check = char_list; check != NULL; check = check->next)
        {
          if (IS_NPC(check))     
           if ( (check->master == ch) && (check->spec_fun == spec_lookup("spec_cloaked_enforcer")) )
           {
            send_to_char("You still have guards under your command!\n\r",ch);
            return;
           }
	}

        af.where = TO_AFFECTS;
        af.level = level;
        af.location = 0;
        af.modifier = 0;
        af.duration = 10;
        af.bitvector = 0;
        af.type = sn;
        affect_to_char(ch,&af);
        act("$n shouts, 'Aid me Citizens!'",ch,0,0,TO_ROOM);
        send_to_char("You call the citizens to arms!\n\r",ch);
        send_to_char("Enforcer guards arrive to aid you.\n\r",ch);

        for ( i = 0; i < 2; i++)
        {
         guard = create_mobile(get_mob_index(41098) );
         guard->level = number_range(ch->level,ch->level*4/4);
         guard->hitroll += (level*5/10);
         guard->damroll += (level*9/10);
         guard->max_hit = number_range(ch->max_hit/8,(ch->max_hit*3/8));
         guard->hit = guard->max_hit;
         guard->max_move = ch->max_move;
         guard->move = guard->max_move;
         char_to_room(guard,ch->in_room);
         add_follower(guard, ch);
         guard->leader = ch;
         guard->spec_fun = spec_lookup("spec_cloaked_enforcer");
         SET_BIT(guard->affected_by, AFF_CHARM);
         SET_BIT(guard->affected_by, AFF_DETECT_INVIS);
         guard->name = str_dup("order enforcer guard");
         guard->short_descr = str_dup("an enforcer of order");
         guard->long_descr = str_dup("A guard has come to aid the law.\n\r");

         af.where = TO_AFFECTS;
         af.level = level;
         af.location = 0;
         af.modifier = 0;
         af.duration = -1;
         af.bitvector = AFF_DETECT_MOTION;
         af.type = gsn_acute_vision;
         affect_to_char(guard,&af);
        }
        act("Enforcers of Order respond to $n's call.",ch,0,0,TO_ROOM);
        return;
}

/*
 * New Dead Animation -- Skyntil
 */
void spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *zombie;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int chance;
    int z_level;
    int control, extra = 0;
    int l_chance = 20;
   
    if ( is_affected(ch,sn) )
    {
	send_to_char("You have not yet regained your powers over the dead.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
        && search->spec_fun == spec_lookup("spec_necro_zombie"))
        control += 6;
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_skeleton"))
        control += 4;
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_mummy"))
        control += 12;
    }
    
    if (ch->level < 30 && control > 12)
    {
    	send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
    }
    else if (ch->level < 55 && control > 17)
	{
		send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
	}
	else if (ch->level < 70 && control > 24)
	{
		send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
	}
	else if (ch->level < 92 && control > 35)
	{
		send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
	}
	else if (ch->level <= 101 && control >= 42)
	{
		send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
	}
    if (target_name[0] == '\0')
    {
        send_to_char("Which corpse do you wish to animate?\n\r",ch);
	return;
    }
    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,obj_name);

    if (corpse == NULL)
    {
	send_to_char("You can't animate that.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) || (corpse->item_type == ITEM_CORPSE_PC) )
    {
	send_to_char("You can't animate that.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj_to_room(obj,ch->in_room);
    }

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
	chance += (3*ch->level);
	chance -= (3*corpse->level);
    }
    
    if(is_obj_affected(corpse,gsn_embalm))
    {
    	l_chance = 25;
    	chance += 5;
    }

    chance = URANGE(l_chance,chance,90);
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = ch->level/14;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    
    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
        act("$n chants dark words, $p is destroyed.",ch,NULL,NULL,TO_ROOM);
	extract_obj(corpse);
	af.duration = ch->level/25;
	affect_to_char(ch,&af);
	return;
    }

    affect_to_char(ch,&af);

    act("$n chants dark words and life seeps back into $p.",ch, corpse, NULL,TO_ROOM);
    act("$p's eyes glow red. $p comes to life!",ch, corpse, NULL,TO_ROOM);
    act("You call upon the powers of the dark to give life to $p.",ch, corpse, NULL,TO_CHAR);

    zombie = create_mobile(get_mob_index(MOB_VNUM_ANIMATE));
    char_to_room(zombie,ch->in_room);
    
    z_level = UMAX(1,corpse->level - number_range(10,18));
    if(is_obj_affected(corpse,gsn_embalm))
    {
    	z_level += number_range(8,16);
    	extra = number_range(1,4);
    }
    zombie->level = z_level;
    zombie->max_hit = (dice(z_level, (22+extra)));
    zombie->max_hit += (z_level * (22+extra));
    zombie->hit = zombie->max_hit;
    zombie->damroll += (z_level*4/4)*2+(extra*10);
    zombie->hitroll += (z_level*4/4)+(extra*8);
    zombie->alignment = -1000;
    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the zombie of %s", name);
    sprintf( buf2, "A zombie of %s is standing here.\n\r", name);
    free_string(zombie->short_descr);
    free_string(zombie->long_descr);
    zombie->short_descr = str_dup(buf1);
    zombie->long_descr = str_dup(buf2);
    zombie->name = str_dup(buf1);

    add_follower(zombie,ch);
    zombie->leader = ch;
    
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(zombie,&af);
    zombie->spec_fun = spec_lookup("spec_necro_zombie");

    return;
}

/*
 * Skeletons -- More fun for necros.
 */
void spell_animate_skeleton( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *skeleton;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int chance;
    int z_level;
    int control;
   

    if ( is_affected(ch,sn) )
    {
        send_to_char("You have not yet regained your powers to animate bones.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
        && search->spec_fun == spec_lookup("spec_necro_zombie"))
        control += 6;
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_skeleton"))
        control += 4;
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_mummy"))
        control += 12;
    }
    
    if ((ch->level < 30 && control > 12)
    || (ch->level < 55 && control > 18)
    || (ch->level < 70 && control > 24)
    || (ch->level < 92 && control > 30)
    || (ch->level <= 101 && control >= 36))
    {
    send_to_char("You already control as many undead as you can.\n\r",ch);
	return;
    }

    if (target_name[0] == '\0')
    {
        send_to_char("Which skeleton do you wish to animate?\n\r",ch);
	return;
    }
    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,obj_name);

    if (corpse == NULL || corpse->item_type != ITEM_SKELETON)
    {
	send_to_char("You can't animate that.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj_to_room(obj,ch->in_room);
    }

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
        chance += (4*ch->level);
	chance -= (3*corpse->level);
    }

    chance = URANGE(10,chance,95);
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = ch->level/12;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    
    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
        act("$n utters dark words. $p is destroyed.",ch,NULL,NULL,TO_ROOM);
	extract_obj(corpse);
	af.duration = ch->level/24;
	affect_to_char(ch,&af);
	return;
    }
    
    affect_to_char(ch,&af);
    
    act("$n chants dark words and $p slowly rises to it's feet.",ch, corpse,NULL,TO_ROOM);
    act("You chant dark words of death and $p slowly rises to it's feet.",ch, corpse, NULL,TO_CHAR);

    skeleton = create_mobile(get_mob_index(MOB_VNUM_ANIMATE));
    char_to_room(skeleton,ch->in_room);

    z_level = UMAX(1,corpse->level - number_range(16,26));
    skeleton->level = z_level;
    skeleton->max_hit = (dice(z_level, 18));
    skeleton->max_hit += (z_level * 18);
    skeleton->hit = skeleton->max_hit;
    skeleton->damroll += (z_level*3/4)*2;
    skeleton->hitroll += (z_level*3/4);
    skeleton->alignment = -1000;
    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the skeleton of %s", name);
    sprintf( buf2, "A skeleton of %s is standing here.\n\r", name);
    free_string(skeleton->short_descr);
    free_string(skeleton->long_descr);
    skeleton->short_descr = str_dup(buf1);
    skeleton->long_descr = str_dup(buf2);
    skeleton->name = str_dup(buf1);

    add_follower(skeleton,ch);
    skeleton->leader = ch;

    af.type = skill_lookup("animate skeleton");
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(skeleton,&af);
    skeleton->spec_fun = spec_lookup("spec_necro_skeleton");

    return;
}

/*
 * More powerful zombie
 */
void spell_mummify( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *mummy;
    OBJ_DATA *corpse;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj;
    CHAR_DATA *search;
    AFFECT_DATA af;
    char *name;
    char *last_name;
    char *obj_name;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int chance, l_chance;
    int z_level;
    int control;
    int extra = 0;
   

    if ( is_affected(ch,sn) )
    {
    	send_to_char("You have not yet regained your powers to over the dead.\n\r",ch);
	return;
    }

    control = 0;

    for (search = char_list; search != NULL; search = search->next)
    {
	if (IS_NPC(search) && (search->master == ch) 
        && search->spec_fun == spec_lookup("spec_necro_zombie"))
        {
         	control += 6;
         	//sprintf(buf1,"Control1: %d",control);
         	//wiznet(buf1,NULL,NULL,WIZ_DEBUG,0,0);
	}
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_skeleton"))
        {
        	control += 4;
        	//sprintf(buf1,"Control2: %d",control);
         	//wiznet(buf1,NULL,NULL,WIZ_DEBUG,0,0);
	}
        else if (IS_NPC(search) && (search->master == ch)
        && search->spec_fun == spec_lookup("spec_necro_mummy"))
        {
        	control += 12;
        	//sprintf(buf1,"Control3: %d",control);
         	//wiznet(buf1,NULL,NULL,WIZ_DEBUG,0,0);
	}
    }
    
    //sprintf(buf1,"ControlT: %d",control);
    //wiznet(buf1,NULL,NULL,WIZ_DEBUG,0,0);
    
    if ((ch->level < 30 && control > 12)
    || (ch->level < 55 && control > 18)
    || (ch->level < 70 && control > 24)
    || (ch->level < 92 && control > 30)
    || (ch->level <= 101 && control >= 36))
    {
    	send_to_char("You already control as many undead as you can.\n\r",ch);
		return;
    }

    if (target_name[0] == '\0')
    {
        send_to_char("Which corpse do you wish to Mummify?\n\r",ch);
	return;
    }
    
    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,obj_name);

    if (corpse == NULL)
    {
        send_to_char("You can't mummify that.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) || (corpse->item_type == ITEM_CORPSE_PC) )
    {
    	send_to_char("You can't mummify or animate that.\n\r",ch);
	return;
    }
    
    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj_to_room(obj,ch->in_room);
    }

    chance = get_skill(ch,sn);

    if (ch->level < corpse->level)
    {
    chance += (2*ch->level);
    chance -= (2*(corpse->level+5));
    }
    
    l_chance = 10;
    
    if(is_obj_affected(corpse,gsn_embalm))
    {
    	l_chance = 15;
    	chance += 5;
    }
    chance = URANGE(10,chance,85);
    
    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = ch->level/6;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    
    if (number_percent() > chance)
    {
	act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
        act("$n chants dark words, $p is destroyed.",ch,corpse,NULL,TO_ROOM);
	extract_obj(corpse);
	af.duration = ch->level/12;
	affect_to_char(ch,&af);
	return;
    }
    
    affect_to_char(ch,&af);
    
    act("$n chants dark words and $p slowly rises to it's feet.",ch,corpse,NULL,TO_ROOM);
    act("You chant evil words and $p slowly rises to it's feet.",ch,corpse,NULL,TO_CHAR);

    mummy = create_mobile(get_mob_index(MOB_VNUM_ANIMATE));
    char_to_room(mummy,ch->in_room);

    z_level = UMAX(1,corpse->level - number_range(8,15));
    if(is_obj_affected(corpse,gsn_embalm))
    {
    	z_level += number_range(8,15);
    	extra = number_range(2,6);
    }	
    mummy->level = z_level;
    mummy->max_hit = (dice(z_level, (25 + extra)));
    mummy->max_hit += (z_level * (25 + extra));
    mummy->hit = mummy->max_hit;
    mummy->damroll += (z_level*5/4)*2 + (extra*10);
    mummy->hitroll += (z_level*5/4) + (extra*8);
    mummy->alignment = -1000;
    last_name = name;
    
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;

    extract_obj(corpse);
    
    sprintf( buf1, "the mummy of %s", name);
    sprintf( buf2, "A torn and shredded mummy of %s is standing here.\n\r", name);
    free_string(mummy->short_descr);
    free_string(mummy->long_descr);
    mummy->short_descr = str_dup(buf1);
    mummy->long_descr = str_dup(buf2);
    mummy->name = str_dup(buf1);

    add_follower(mummy,ch);
    mummy->leader = ch;
    
    af.type = skill_lookup("mummify");
    af.duration = -1;
    af.bitvector = AFF_CHARM;
    affect_to_char(mummy,&af);
    mummy->spec_fun = spec_lookup("spec_necro_mummy");

    return;
}

/*
 * Alrighty...Lets make skeletons
 */
void spell_decay_corpse(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *skeleton;
    char *obj_name;
    char *name;
    char *last_name;
    int chance;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (target_name[0] == '\0')
    {
        send_to_char("Which corpse would you like to decay?\n\r",ch);
	return;
    }

    obj_name = str_dup(target_name);

    corpse = get_obj_here(ch,obj_name);

    if (corpse == NULL)
    {
    send_to_char("You can't find that object.\n\r",ch);
  	return;
    }

    if ((corpse->item_type != ITEM_CORPSE_NPC) && (corpse->item_type != ITEM_CORPSE_PC) )
    {
    send_to_char("You can't decay that.\n\r",ch);
	return;
    }

    name = corpse->short_descr;

    for (obj = corpse->contains; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	obj_from_obj(obj);
	obj_to_room(obj,ch->in_room);
    }

    chance = get_skill(ch,sn);

    if (number_percent() > chance)
    {
        act("You chant dark words of decay but destroy $p.",ch,corpse,NULL,TO_CHAR);
        act("$n chants dark words, $p is reduced to ashes.",ch,corpse,NULL,TO_ROOM);
	extract_obj(corpse);
	return;
    }

    act("$n decays the flesh off $p.",ch,corpse,NULL,TO_ROOM);
    act("You decay the flesh off $p and are left with a skeleton.",ch,corpse,NULL,TO_CHAR);

    skeleton = create_object(get_obj_index(OBJ_VNUM_GENERIC), 1);
    obj_to_room(skeleton,ch->in_room);

    skeleton->level = corpse->level;

    last_name = name;
    last_name = one_argument(corpse->short_descr, name);
    last_name = one_argument(last_name,name);
    last_name = one_argument(last_name,name);
    name = last_name;
    extract_obj(corpse);
    
    sprintf( buf1, "the skeleton of %s", name);
    sprintf( buf2, "A skeleton of %s is lying here in a puddle of decayed flesh.", name);
    free_string(skeleton->short_descr);
    free_string(skeleton->description);
    skeleton->name = str_dup(buf1);
    skeleton->short_descr = str_dup(buf1);
    skeleton->description = str_dup(buf2);
    skeleton->item_type = ITEM_SKELETON;
    SET_BIT(skeleton->wear_flags, ITEM_TAKE);

    return;
}

void spell_steel_flesh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (IS_SHIELDED(victim, SHD_STEEL))
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = SHD_STEEL;
    affect_to_char( victim, &af );
    act( "$n's flesh turns to steel.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your flesh turns to steel.\n\r", victim );
    return;
}

void spell_mana_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ))
    {
        send_to_char("You cannot handle the mental pressure of transfering mana yet.\n\r",ch); 
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_HIT;
    af.modifier  = ch->level * 10;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    af.location  = APPLY_MANA;
    af.modifier  = ch->level * -15;
    affect_to_char( ch, &af );

    act( "$n concentrates and looks healthier.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You feel life surge through your limbs.\n\r", ch );
    return;
}

void spell_darken_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(!IS_EVIL(ch))
	{
		send_to_char("You are not quite wicked enough to do that.\n\r",ch);
		return ;
	}

	if(obj->item_type != ITEM_WEAPON)
	{
                send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}
	else
	{
                if(obj->value[0]!= WEAPON_SWORD && obj->value[0]!= WEAPON_DAGGER &&
                        obj->value[0]!= WEAPON_SPEAR && obj->value[0]!= WEAPON_AXE &&
                        obj->value[0]!= WEAPON_POLEARM && obj->value[0]!= WEAPON_EXOTIC)
                {
                  send_to_char("You must target a sharp weapon.\n\r",ch);
                               return;
                }
		else
		{

		  if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC))
		  {
			send_to_char("That weapon is already quite evil.\n\r", ch);
			return ;
		  }

		  if(IS_OBJ_STAT(obj,ITEM_BLESS))
		  {
                        send_to_char("That weapon is too holy to be touched by your magic.\n\r", ch);
			return ;
		  }

                  if(!IS_OBJ_STAT(obj,ITEM_EVIL))
			SET_BIT(obj->extra_flags, ITEM_EVIL);
		  if(!IS_OBJ_STAT(obj,ITEM_ANTI_GOOD))
			SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		  if(!IS_OBJ_STAT(obj,ITEM_ANTI_NEUTRAL))
			SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);

		  af.where    = TO_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level/2;
		  af.location = 0;
		  af.modifier = 0;
		  af.bitvector= WEAPON_VAMPIRIC;
		  affect_to_obj(obj, &af);
                  af.location = APPLY_DAMROLL;
                  af.modifier = ch->level/10;
                  affect_to_obj(obj, &af);
                  af.location = APPLY_HITROLL;
                  af.modifier = ch->level/10;
                  affect_to_obj(obj, &af);

		  act("$p becomes dark and evil.",ch,obj,NULL,TO_ALL);
                  return;
		}
	}
	return ;
}

void spell_empower_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}
	else
	{
		if(obj->value[0]!=WEAPON_SWORD && obj->value[0]!=WEAPON_DAGGER &&
			obj->value[0]!=WEAPON_SPEAR && obj->value[0]!=WEAPON_AXE &&
			obj->value[0]!=WEAPON_POLEARM && obj->value[0]!=WEAPON_EXOTIC)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}

		else
		{
		  if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING))
		  {
			send_to_char("That weapon is already imbued with power.\n\r", ch);
			return ;
		  }

		  af.where    = TO_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level;
		  af.location = 0;
		  af.modifier = 0;
		  af.bitvector= WEAPON_SHOCKING;
		  affect_to_obj(obj, &af);
                  af.location = APPLY_DAMROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);
                  af.location = APPLY_HITROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);

		  act("$p sparks with electricity.",ch,obj,NULL,TO_ALL);
		  return ;
		}
	}
	return ;
}

void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}
	else
	{
		if(obj->value[0]!=WEAPON_SWORD && obj->value[0]!=WEAPON_DAGGER &&
			obj->value[0]!=WEAPON_SPEAR && obj->value[0]!=WEAPON_AXE &&
			obj->value[0]!=WEAPON_POLEARM && obj->value[0]!=WEAPON_EXOTIC)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}

		else
		{
		  if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
		  {
			send_to_char("That weapon is already flaming.\n\r", ch);
			return ;
		  }
		  if(IS_WEAPON_STAT(obj,WEAPON_FROST))
		  {
			send_to_char("That weapon is too cold to accept the magic.\n\r", ch);
			return ;
		  }

		  af.where    = TO_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level*2;
		  af.location = 0;
		  af.modifier = 0;
		  af.bitvector= WEAPON_FLAMING;
		  affect_to_obj(obj, &af);
                  af.location = APPLY_DAMROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);
                  af.location = APPLY_HITROLL;
                  af.modifier = 5;
                  affect_to_obj(obj, &af);

		  act("$p gets a fiery aura.",ch,obj,NULL,TO_ALL);
		  return ;
		}
	}
	return ;
}

void spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}
	else
	{
		if(obj->value[0]!=WEAPON_SWORD && obj->value[0]!=WEAPON_DAGGER &&
			obj->value[0]!=WEAPON_SPEAR && obj->value[0]!=WEAPON_AXE &&
			obj->value[0]!=WEAPON_POLEARM && obj->value[0]!=WEAPON_EXOTIC)
		 return ;
		else
		{
		  if(IS_WEAPON_STAT(obj,WEAPON_FROST))
		  {
			send_to_char("That weapon is already wickedly cold.\n\r", ch);
			return ;
		  }
                  if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
		  {
			send_to_char("That weapon is too warm to accept the magic.\n\r", ch);
			return ;
		  }

		  af.where    = TO_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level*2;
		  af.location = 0;
		  af.modifier = 0;
		  af.bitvector= WEAPON_FROST;
		  affect_to_obj(obj, &af);
                  af.location = APPLY_DAMROLL;
                  af.modifier = 5;
                  affect_to_obj(obj, &af);
                  af.location = APPLY_HITROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);

		  act("$p grows wickedly cold.",ch,obj,NULL,TO_ALL);
		  return ;
		}
	}
	return ;
}

void spell_acid_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	obj=(OBJ_DATA *) vo;

	if(obj->item_type != ITEM_WEAPON)
	{
		send_to_char("You can only target sharp weapons.\n\r",ch);
		return ;
	}
	else
	{
		if(obj->value[0]!=WEAPON_SWORD && obj->value[0]!=WEAPON_DAGGER &&
			obj->value[0]!=WEAPON_SPEAR && obj->value[0]!=WEAPON_AXE &&
			obj->value[0]!=WEAPON_POLEARM && obj->value[0]!=WEAPON_EXOTIC)
		 return ;
		else
		{
                  if(IS_WEAPON_STAT(obj,WEAPON_ACIDIC))
		  {
                        send_to_char("That weapon is already corrosive.\n\r", ch);
			return ;
		  }
                  if(IS_WEAPON_STAT(obj,WEAPON_FLAMING))
		  {
			send_to_char("That weapon is too warm to accept the magic.\n\r", ch);
			return ;
		  }

		  af.where    = TO_WEAPON;
		  af.type     = sn;
		  af.level    = level/2;
		  af.duration = level;
		  af.location = 0;
		  af.modifier = 0;
                  af.bitvector= WEAPON_ACIDIC;
		  affect_to_obj(obj, &af);
                  af.location = APPLY_DAMROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);
                  af.location = APPLY_HITROLL;
                  af.modifier = 1;
                  affect_to_obj(obj, &af);

                  act("$p grows wickedly corrosive.",ch,obj,NULL,TO_ALL);
		  return ;
		}
	}
	return ;
}


void spell_illusion_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	 CHAR_DATA *victim = (CHAR_DATA *) vo;
	 AFFECT_DATA af;

	 if(!is_affected(victim,sn))
	 {
         af.where = TO_AFFECTS;
         af.type = sn;
         af.level = level;
         af.duration = level/6;
         af.location = APPLY_HIT;
         af.modifier = level*8;
         af.bitvector = 0;
	 affect_to_char( victim, &af );
         af.modifier = level*5;
         af.location = APPLY_MANA;
	 affect_to_char( victim, &af );

	 send_to_char( "You appear mightier then you are!\n\r", victim );
         act( "$n grows in stature and appears very mighty!\n\r", victim, NULL, NULL, TO_ROOM );
	 }
	 else
         send_to_char("You are already affected by the illusion.\n\r", victim );

	 return ;
}

/* Necromancer -- preserve a limb for use later with making golems -Skyn */
void spell_preserve_limb(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int vnum, chance;

    vnum = obj->pIndexData->vnum;
    
    if (vnum != OBJ_VNUM_SEVERED_HEAD
	&& vnum != OBJ_VNUM_TORN_HEART
	&& vnum != OBJ_VNUM_SLICED_ARM
	&& vnum != OBJ_VNUM_SLICED_LEG
	&& vnum != OBJ_VNUM_GUTS
	&& vnum != OBJ_VNUM_BRAINS)
	{
    		send_to_char("You cannot preserve that item.\n\r",ch);
		return;
    	}

    if (obj->timer > 10)
    {
        send_to_char("It's already in very well preserved condition.\n\r",ch);
        return;
    }
    
    chance = get_skill(ch,sn);
    chance = (chance * 9) / 10;

    if (number_percent() > chance)
    {
        act("$n destroys $p.",ch,obj,NULL,TO_ROOM);
        act("You fail and destroy $p.",ch,obj,NULL,TO_CHAR);
        extract_obj(obj);
        return;
    }

    act("You cover $p in a magical shell to slow it's decay.",ch,obj,NULL,TO_CHAR);
    act("$p glows with a sickly green aura.",ch,obj,NULL,TO_ROOM);
    
    obj->timer += number_range(level/4,level/3);
    return;
}

/* Vampire/Necro classes -- Scare someone stiff? -Skyn */
void spell_fear(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    bool bad_fail, utter_fail;
    int range;

    bad_fail = FALSE;
    utter_fail = FALSE;

    if (victim == NULL)
    {
    	send_to_char("Bug -- null victim.\n\r",ch);
        return;
    }
    
    if (victim == ch)
    {
        send_to_char("Want to scare yourself? Go look in a mirror.\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	    send_to_char("You can't concentrate enough.\n\r",ch);
	    return;
    }

    act("$n points at $N and intones loudly 'Furcht!'",ch,0,victim,TO_NOTVICT);
    act("$n points at you and intones loudly 'Furcht!'",ch,0,victim,TO_VICT);
    act("You point at $N and intone loudly 'Furcht!'",ch,0,victim,TO_CHAR);

    if (!IS_AWAKE(victim))
    {
        act("$n shivers for a moment.",victim,0,0,TO_ROOM);
        send_to_char("You feel an icy hand brush your soul, but fall back into a deep dream.\n\r",victim);
        return;
    }
    
    if (is_affected(victim,sn))
    { 	
	send_to_char("They are already fear you!\n\r",ch);
	send_to_char("You feel a small shiver pass through you.\n\r",victim);
	return;
    }

    // No affecting clan guards
    if (IS_NPC(victim))
    {
	if ( IS_SET(victim->off_flags,OFF_CLAN_GUARD) )
	{
        	act("$n shivers for a moment.",victim,0,0,TO_ROOM);
        	send_to_char("You feel the chill of terror for a moment.\n\r", victim);
        	return;
        }
    }

    if (saves_spell(level,victim,DAM_MENTAL))
    {
    	act("$n shivers for a moment.",victim,0,0,TO_ROOM);
    	send_to_char("You feel the chill of terror for a moment.\n\r", victim);
    	return;
    }

    if (!saves_spell(level - 2,victim,DAM_OTHER))
    {
        bad_fail = TRUE;
        if (!saves_spell(level - 5,victim,DAM_OTHER))
          if (!saves_spell(level,victim,DAM_OTHER))
              utter_fail = TRUE;
    }

    /* Insta-kill */
    if (ch->level - victim->level + IS_NPC(ch) ? 50 : 10 > number_percent()) {
    if (utter_fail && ((!IS_NPC(victim) && number_percent() > 90) || IS_NPC(victim)))
    {
    	act("$n's eyes widen and $s heart ruptures from shock!",victim,0,0,TO_ROOM);
    	send_to_char("You feel a terror so intense your heart stops dead!\n\r",victim);
        raw_kill(victim,ch);
	group_gain(ch,victim);
        return;
    } }
    
    act("$n's eyes widen in shock and $s entire body freezes in momentary terror.",victim,NULL,NULL,TO_ROOM);
    send_to_char("You feel an overwhelming terror and you shudder in momentary shock.\n\r",victim);

    range = level/12;

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.bitvector = 0;
    af.duration = (number_range(1,5) + range);

    af.location = APPLY_CON;
    af.modifier = -number_range(2,range + 1);
    affect_to_char(victim,&af);
    af.location = APPLY_STR;
    af.modifier = -number_range(2,range + 1);
    affect_to_char(victim,&af);
    af.location = APPLY_DEX;
    af.modifier = -number_range(1,range);
    affect_to_char(victim,&af);
    af.location = APPLY_HIT;
    af.modifier = -number_range(2,range);
    affect_to_char(victim,&af);
    af.location = APPLY_DAMROLL;
    af.modifier = -number_range(2,range);
    affect_to_char(victim,&af);

    if (victim->position == POS_FIGHTING)
        do_flee(victim,"");

    if (victim->position == POS_FIGHTING)
        do_flee(victim,"");
    
    if (victim->position == POS_FIGHTING)
        do_flee(victim,"");
    
    if( bad_fail )
    {
        WAIT_STATE(victim,12);
    }
    
    return;
}

/* Necromancer -- create a more advanced golem -Skyn */
void spell_greater_golem(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
        CHAR_DATA *golem;
        AFFECT_DATA af;
        OBJ_DATA *part;
        OBJ_DATA *part_next;
        int parts = 0;
        CHAR_DATA *check;
        int z_level;

        if (is_affected(ch,sn))
        {
        	send_to_char("You are not rested enough to make another golem yet.\n\r",ch);
        	return;
        }

        for (check = char_list; check != NULL; check = check->next)
        {
		if (IS_NPC(check))     
   			if ( (check->master == ch) && (check->spec_fun == spec_lookup("spec_necro_igolem")) )
        		{
				send_to_char("You already have a golem under your command.\n\r",ch);
        			return;
        		}
	}
	
	for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		
    		if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
    		&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
    		&& part->pIndexData->vnum != OBJ_VNUM_GUTS
    		&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
        	continue;
        	
    		parts++;
    	}

    	if (parts == 0)
    	{
        	send_to_char("You don't have any body parts to create a golem with!\n\r",ch);
        	return;
    	}
    	else if (parts <= 2)
    	{
    		send_to_char("You don't have enough limbs to make a golem.\n\r",ch);
    		return;
    	}

    	for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
    		&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
    		&& part->pIndexData->vnum != OBJ_VNUM_GUTS
    		&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
        	continue;
    		
    		extract_obj(part);
    	}
    	
    	/*for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		
    		if (part->pIndexData->item_type != ITEM_ARMOR)
        	continue;
        	
    		parts++;
    	}
    	
    	if (parts < 1)
    	{
    		send_to_char("You must have some metal armor to make the golem.\n\r",ch);
    		return;
    	}
    	
    	for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		
    		if (part->pIndexData->item_type != ITEM_ARMOR)
        	continue;
        	
    		extract_obj(part);
    	}*/
    	
	
        af.where = TO_AFFECTS;
        af.level = level;
        af.location = 0;
        af.modifier = 0;
        af.duration = 35;
        af.bitvector = 0;
        af.type = sn;
        affect_to_char(ch,&af);
        act("$n takes some spare body parts and creates an iron golem!",ch,0,0,TO_ROOM);
        send_to_char("You use some preserved limbs to fashion an iron golem to serve you!\n\r",ch);
        
        golem = create_mobile(get_mob_index(MOB_VNUM_IGOLEM));
    	
	z_level = ch->level + number_range(-10,15);
    	golem->level = z_level;
    	golem->max_hit = (dice(z_level, 30));
    	golem->max_hit += (z_level * 30);
    	golem->max_move = ch->max_move;
        golem->move = golem->max_move;
    	golem->hit = golem->max_hit;
    	golem->damroll += (z_level*5/4)*2;
    	golem->hitroll += (z_level*11/8);
    	golem->alignment = -1000;
    	
    	char_to_room(golem,ch->in_room);
    	
    	add_follower(golem,ch);
    	golem->leader = ch;
    
    	af.type = sn;
    	af.duration = -1;
    	af.bitvector = AFF_CHARM;
    	affect_to_char(golem,&af);
    	golem->spec_fun = spec_lookup("spec_necro_igolem");
    	
        return;
}

/* Necromancer -- create a basic golem -Skyn */
void spell_lesser_golem(int sn,int level, CHAR_DATA *ch,void *vo,int target)
{
        CHAR_DATA *golem;
        AFFECT_DATA af;
        CHAR_DATA *check;
        OBJ_DATA *part;
        OBJ_DATA *part_next;
        int parts = 0;
        int z_level;

        if (is_affected(ch,sn))
        {
	        send_to_char("You are not ready to build another golem yet.\n\r",ch);
	        return;
        }

        for (check = char_list; check != NULL; check = check->next)
        {
		if (IS_NPC(check))     
   			if ( (check->master == ch) && (check->spec_fun == spec_lookup("spec_necro_fgolem")) )
        		{
				send_to_char("You already have a flesh golem under your command.\n\r",ch);
        			return;
        		}
	}

    	for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		
    		if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
    		&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
    		&& part->pIndexData->vnum != OBJ_VNUM_GUTS
    		&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
        	continue;
        	
    		parts++;
    	}


    	if (parts == 0)
    	{
        	send_to_char("You don't have any body parts to create a golem with!\n\r",ch);
        	return;
    	}
    	else if (parts <= 2)
    	{
    		send_to_char("You don't have enough limbs to make a golem.\n\r",ch);
    		return;
    	}

    	for (part = ch->carrying; part != NULL; part = part_next)
    	{
    		part_next = part->next_content;
    		if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
    		&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
    		&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
    		&& part->pIndexData->vnum != OBJ_VNUM_GUTS
    		&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
        	continue;
    		
    		extract_obj(part);
    	}
    	
    	if (parts >= ch->level/10)
        	parts = ch->level/10;

    	z_level = ch->level + number_range(-6,8);
    	z_level += parts;
    	
        af.where = TO_AFFECTS;
        af.level = level;
        af.location = 0;
        af.modifier = 0;
        af.duration = 35;
        af.bitvector = 0;
        af.type = sn;
        affect_to_char(ch,&af);
        
        act("$n pieces some severed limbs and dead flesh together and creates a flesh golem!",ch,0,0,TO_ROOM);
        send_to_char("You build a flesh golem to serve you!\n\r",ch);

        golem = create_mobile(get_mob_index(MOB_VNUM_FGOLEM));
    	
    	golem->level = z_level;
    	golem->max_hit = (dice(z_level, (18+parts/2) ));
    	golem->max_hit += (z_level * (18+parts/2));
    	golem->max_move = ch->max_move;
        golem->move = golem->max_move;
    	golem->hit = golem->max_hit;
    	golem->damroll += (z_level*4/4)*2 + parts * 3;
    	golem->hitroll += (z_level*4/4) + parts;
    	golem->alignment = -1000;
        
        char_to_room(golem,ch->in_room);
        add_follower(golem, ch);
        golem->leader = ch;
    
    	af.type = sn;
    	af.duration = -1;
    	af.bitvector = AFF_CHARM;
    	affect_to_char(golem,&af);
    	golem->spec_fun = spec_lookup("spec_necro_fgolem");
        
        
        return;
}

/* Necromancer -- light a fire? :) -Skyn */
void spell_cremate(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level,19,ch);
    else
	dam = dice(level,21);

    act("$n is enveloped by searing fire!",victim,0,0,TO_ROOM);
    send_to_char("You are enveloped in a searing fire!\n\r",victim);

    if ( saves_spell( level, victim, DAM_FIRE ) )
	dam /= 2;
    
    damage_old( ch, victim, dam, sn, DAM_FIRE, TRUE);

    if( number_percent( ) < 10 )
    {
    	dam = number_range(1,level / 5 + 6);
    	act("$n is covered in flames!",victim,NULL,NULL,TO_ROOM);
    	act("You are enveloped in flames!",victim,NULL,NULL,TO_CHAR);
    	fire_effect( (void *) victim,level/1.5,dam,TARGET_CHAR);
    	damage_old(ch,victim,dam,0,DAM_FIRE,FALSE);
    }

    return;
}

void spell_courage( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
		if (victim == ch)
			send_to_char("You are already envigorated with courage.\n\r",ch);
		else
			act("$N is already courageous.",ch,NULL,victim,TO_CHAR);
		return;
	}
	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 10; //to use wisdom
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char( victim, &af );
	send_to_char("You lift your face with renewed vigor ready to face all obstaces to come.\n\r",victim);
	affect_strip(victim,skill_lookup("fear aura"));
	if ( ch != victim )
		act("$N shifts their stance settling into a pose implying strength and resolve.",ch,NULL,victim,TO_CHAR);
	return;
}

void spell_imbue_weapon(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	return;
}

void spell_restore(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	return;
}

//Martyr spell for Saints.  Automatically aggies everything in the room on them.  Fesdor 1-20-09
void spell_martyr(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int marker = 0;
	
	if (victim != ch)
	{	//TAR_CHAR_DEFENSIVE would normally allow casting on another.  I chose it so they can still use this during combat.
		send_to_char("Sacrifice someone else and take the credit? Yeah right.\n\r",ch);
		return;
	}
	
	if (is_affected(ch,sn))
	{
		send_to_char("You cannot martyr yourself again so soon!\n\r",ch);
		return;
	}
	
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	if ((is_safe_spell(ch,vch,TRUE)	|| (is_same_group(ch,vch)) || (vch == ch))	&& (ch->fighting != vch || vch->fighting != ch))
	continue;
	if ( ( ch->fighting == NULL ) && ( !IS_NPC( ch ) ) && ( !IS_NPC( vch ) ) )
	    {
		ch->attacker = FALSE;
		vch->attacker = TRUE;
	    }
	    char buf[MAX_STRING_LENGTH];
	    sprintf(buf,"%s draws your attention, forcing you to attack them!\n\r",ch->short_descr);
	    send_to_char(buf,vch);
		vch->fighting = ch;
		marker = 1;//We have a taker!
    }
    if(marker)
    {
	send_to_char("You sacrifice yourself for the good of the cause!\n\r",ch);
    af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 1;//Once every 2 ticks.
	af.bitvector= 0;
	af.modifier	= -100;//A little defense boost with all the tanking they're about to do
	af.location	= APPLY_AC;
	affect_to_char( ch, &af );
	}
	return;
}
