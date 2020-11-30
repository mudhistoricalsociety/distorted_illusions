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
#include "recycle.h"
#include "tables.h"
#include "clan.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_wear		);
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_yell          );

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );

bool check_channel args( (CHAR_DATA *ch,CHAR_DATA *victim,int dt) );
int     sorcery_dam  args( ( int num, int dice, CHAR_DATA *ch) );
int 	sorcery_dam2 args( (int dam) );
bool    check_sorcery args( (CHAR_DATA *ch, int sn) );
bool saves_spell_real( int level, CHAR_DATA *victim, int dam_type, bool show,CHAR_DATA *ch );
int focus_sorc args ((CHAR_DATA *ch));
int focus_level args ((long total));

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
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
	    if ( found == -1)
		found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}

/*
 * Warlocks -- Possibly other classes
 */
bool check_channel(CHAR_DATA *ch,CHAR_DATA *victim,int dt)
{
    int mana;

    if (dt >= 0 && dt < MAX_SKILL && skill_table[dt].spell_fun != NULL)
    {
	if (skill_table[dt].target != TAR_CHAR_OFFENSIVE
        || !is_affected(victim,gsn_channel)) 
		return FALSE;

        mana = skill_table[dt].min_mana + 25;

        if (victim == ch)
         mana /= 2;
   
        if (number_percent() < (victim->level/2 + (victim->pcdata->learned[gsn_channel]) / 2)*1/3)
        {
         if(skill_table[dt].target == TAR_CHAR_OFFENSIVE
         && number_percent( ) < 50 && victim != ch)
         {
          act("$N channels $n's spell back at $m!",ch,NULL,victim,TO_NOTVICT);
          act("You channel $n's spell and reflect pure energy back on $m!",ch,NULL,victim,TO_VICT);
          act("$N channels your spell and reflects pure energy at you!",ch,NULL,victim,TO_CHAR);
          damage(victim,ch,dice(ch->level,4),gsn_channel,DAM_ENERGY,TRUE,DF_NOPARRY |DF_NODODGE);
          return TRUE;
         }
         else
         {
                act("$N channels $n's spell.",ch,NULL,victim,TO_NOTVICT);
                act("You channel $n's spell.",ch,NULL,victim,TO_VICT);
                act("$N channels your spell.",ch,NULL,victim,TO_CHAR);
		victim->mana += mana;
                send_to_char("You feel energy surge up through your body!\n\r",victim);
		return TRUE;
         }
        }
       }
       return FALSE;
}

/*
 * All Magic-Users get a chance for enhanced spell damage
 */
bool check_sorcery(CHAR_DATA *ch,int sn)
{
        int chance;
        chance = number_range(60,75);

        if(IS_NPC(ch))
         return FALSE;

        if (!class_table[ch->class].fMana)
          return FALSE;
	
        chance /= 5;

        if ( sn == skill_lookup("acid rain")
         || sn == skill_lookup("ray of truth")  //begins init
         || sn == skill_lookup("energy drain")
         || sn == skill_lookup("demonfire")
         || sn == skill_lookup("acid blast"))
                chance -= 10;
        
        if (sn == skill_lookup("magic missle")
         || sn == skill_lookup("chill touch")
         || sn == skill_lookup("burning hands")
         || sn == skill_lookup("lightning bolt")
         || sn == skill_lookup("flamestrike")
         || sn == skill_lookup("harm")
         || sn == skill_lookup("color spray"))
                 chance += 10;

         if((sn == skill_lookup("entity")) || (sn == skill_lookup("unite")) || (sn == skill_lookup("blind faith")) || (sn == skill_lookup("kick")))
          return FALSE;

         chance += (focus_sorc(ch));

        if (number_percent() > chance)
         return FALSE;

     //   if (IS_IMMORTAL(ch))
     //   send_to_char("Sorcery Enhanced Damage!\n\r",ch);

        return TRUE;
}

void do_sorcery(CHAR_DATA *ch, char *argument)
{
 char arg[MSL/10];
 char arg2[MSL/10];
 int dam;
 int level;
 int result;

 argument = one_argument(argument,arg);
 argument = one_argument(argument,arg2);

 if(arg[0] == '\0')
 {
  send_to_char("Syntax: sorcery dam level\n\r",ch);
  return;
 }

 dam = atoi(arg);
 level = atoi(arg2);

 result = sorcery_dam(dam, level+1, ch);
 printf_to_char(ch,"Sorcery=%d",result);
 return;
}

/*
 * Find the Enhanced Damage
 */
int sorcery_dam(int num,int die,CHAR_DATA *ch)
{
	return (dice(num,die)*(100+focus_sorc(ch)))/100;
}

// add 10-25% -- non-dice version
int sorcery_dam2(int dam)
{
	return (dam * ( number_range(110,125) / 100 ) );
}

/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( ch->class==rch->class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}

/* Start Insert */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
 return saves_spell_real( level, victim, dam_type, FALSE,NULL);
}

void do_testsave(CHAR_DATA *ch, char *argument)
{
 STRING( arg )
 STRING( arg1 )
 STRING( arg2 )
 CHAR_DATA *victim;
 int value,type;
 bool SHOW = FALSE;

 argument = one_argument(argument,arg);
 argument = one_argument(argument,arg1);
 argument = one_argument(argument,arg2);

 if( arg[0] == '\0' || arg1[0] == '\0')
 {
  send_to_char("Usage: testsave <victim> <level> <dam_type>\n\r",ch);
  return;
 }

 if ( ( victim = get_char_world( ch, arg ) ) == NULL )
 {
   send_to_char( "They aren't here.\n\r", ch );
   return;
 }

 if(!is_number(arg1))
 {
  send_to_char("Second argument must be a number.\n\r",ch);
  return;
 }

 value = atoi(arg1);

 if(!is_number(arg2))
 {
  send_to_char("Third argument must be a number.\n\r",ch);
  return;
 }

 type = atoi(arg2);

 SHOW = saves_spell_real(value, victim, type, TRUE,ch);

 if(SHOW)
 send_to_char("Saving Throw Test completed SuccessFully.\n\r",ch);
 else
 send_to_char("Bummer, didn't save...heh.\n\r",ch);

 return;
}

        
/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell_real( int level, CHAR_DATA *victim, int dam_type, bool show,CHAR_DATA *ch )
{
    int saves,chance;
    int victim_saves,lvlsave;
    char buf[MSL/4];

    if(ch == NULL)
     show = FALSE;

    if(victim == NULL)
    {
     bug("Null victim in Saves_Spell_real",0);
     return FALSE;
    }
    
    victim_saves = victim->saving_throw; //Sorn + -1*focus_save(ch);
    
    //rand = number_range(-100,-80);

    //if (victim_saves < -80)
      //  victim_saves = UMAX(victim_saves,rand);

    if (victim_saves)
    if(show)
    {
     sprintf(buf,"Simulated Saving Throw:\n\rSpell Level: %d\n\rVictim Level: %d victim Saves: %d\n\r",
     level,victim->level, victim_saves);
     send_to_char(buf,ch);
    }

    /*saves = (victim->level - level)*2;
    
    if(show)
    {
     sprintf(buf,"Saves #1(level): %d\n\r", saves);
     send_to_char(buf,ch);
    }
    
    saves = saves - (victim_saves);
    
    if(show)
    {
     sprintf(buf,"Saves #2(after saves): %d\n\r", saves);
     send_to_char(buf,ch);
    } */
    
    saves = victim_saves*80/100;
    saves *= -1;
    //saves += 15; 
    
    if(show)
    {
     sprintf(buf,"Saves #1(onscale): %d\n\r", saves);
     send_to_char(buf,ch);
    }
    
    lvlsave = level - victim->level;
    lvlsave = lvlsave*9/60; // Puts it on a scale of 1 - 15.
        
    saves -= lvlsave;
    
    if(show)
    {
    	sprintf(buf,"Saves #1.5(level): %d\n\r", saves);
     	send_to_char(buf,ch);
    }

	//check if resistant/vuln/imm
	switch(check_immune(victim,dam_type))
    {
        case IS_IMMUNE:         return TRUE;
        case IS_RESISTANT:      saves += victim->level/10;     break;
        case IS_VULNERABLE:     saves -= victim->level/10;     break;
    }

    if(show)
    {
     sprintf(buf,"Saves #2(immunities): %d\n\r", saves);
     send_to_char(buf,ch);
    }


    if (IS_AFFECTED(victim,AFF_BERSERK))
        saves -= number_range(5,victim->level/7);

    if(show)
    {
     sprintf(buf,"Saves #3(berserk): %d\n\r", saves);
     send_to_char(buf,ch);
    }

    //lower chance if mobile
    if (IS_NPC(victim))
    {
        saves -= victim->level/4;

    	if(show)
    	{
     		sprintf(buf,"Saves #4(npc): %d\n\r", saves);
     		send_to_char(buf,ch);
    	}
    }

    //magic classes save a bit more
    if (!IS_NPC(victim) && class_table[victim->class].fMana)
		saves = saves*12/10;

    if(show)
    {
     sprintf(buf,"Saves #5(fmana): %d\n\r", saves);
     send_to_char(buf,ch);
    }
    // Returned to 0 to 75 by Fesdor to decrease general effectiveness.
    saves = URANGE(0, saves, 75); 
    
    if(show)
    {
     sprintf(buf,"Saves #6(W/URANGE): %d\n\r",saves);
     send_to_char(buf,ch);
    }

    //random percent
    chance = number_percent( );
    
    if(show)
    {
    sprintf(buf,"CHANCE: %d\n\rSAVED: %s\n\r",
    chance, (saves > chance ? "TRUE": "FALSE"));
    send_to_char(buf,ch);
    }

    //if your saves is lower than chance, you save against the spell
    return saves > chance;

 /* Old Saves

    save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;

    if (IS_AFFECTED(victim,AFF_BERSERK))
        save += (victim->level)/20;

    if(show)
    {
     sprintf(buf,"W/Berserk: %d\n\r",save);
     send_to_char(buf,ch);
    }

    if (IS_NPC(victim))
        save -= victim->level/4;        // simulate npc saving throw 

    if(show)
    {
    sprintf(buf,"W/NPC: %d\n\r",save);
    send_to_char(buf,ch);
    }

    if (!IS_NPC(victim) && class_table[victim->class].fMana)
	save = 9 * save / 10;

    if(show)
    {
    sprintf(buf,"w/fMana: %d\n\r",save);
    send_to_char(buf,ch);
    }

    save = ( save * 7 ) / 10;

    if(show)
    {
    sprintf(buf,"Save * 7/10: %d\n\r",save);
    send_to_char(buf,ch);
    }

    switch(check_immune(victim,dam_type))
    {
        case IS_IMMUNE:         return TRUE;
        case IS_RESISTANT:      save += save/3;     break;
        case IS_VULNERABLE:     save -= save/3;     break;
    }

    if(show)
    {
    sprintf(buf,"w/Res/Vuln: %d\n\r",save);
    send_to_char(buf,ch);
    }

    save = URANGE( 15, save, 95 );


    if(show)
    {
    sprintf(buf,"W/URANGE: %d\n\r",save);
    send_to_char(buf,ch);
    }

    save = number_range( save / 3, save);


    if(show)
    {
    sprintf(buf,"W/Randomize Range (save/3 - save): %d\n\r",save);
    send_to_char(buf,ch);
    }

    chance = number_percent( );

    if(show)
    {
    sprintf(buf,"CHANCE: %d\n\rSAVED: %s\n\r",
    chance, (chance < save ? "TRUE": "FALSE"));
    send_to_char(buf,ch);
    }
    
    return chance < save;

*/
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;

    save = 50 + (spell_level - dis_level) * 5;
    if(duration==-1)
	return TRUE;
    save = URANGE( 5, save, 99 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (saves_spell(dis_level,victim,DAM_OTHER))
		return FALSE;

    if (sn < 0 || sn > MAX_SKILL){
    bug("Magic Sector :: Check_dispel :: %d",sn);
    return TRUE;
    }

    /* This is new way of non-dispellable gsn's
     */
    if(skill_table[sn].slot == 999)
     return FALSE;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if(!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    if(af->duration!=-1)	
		        af->level--;
            }
        }
    }
    return FALSE;
}

/* End Insert */



/*
 * OLD SAVING THROWS
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    save = 10 + ( victim->level - level) * 2 - victim->saving_throw * 2;

    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/10;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save *= 2;	break;
	case IS_VULNERABLE:	save /= 2;	break;
    }

    if (!IS_NPC(victim) && class_table[victim->class].fMana)
	save = 4 * save / 5;
    save = URANGE( 15, save, 95 );

    save = dice(1,save+10);

    return number_percent( ) < save;
}
*/

bool saves_dispel_magic( int level, CHAR_DATA *victim, int dam_type )
{
/*    int save;
    int minnum; */

    return saves_spell(level,victim,dam_type);
/*
    save = 10 + ( victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/10;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save *= 2;	break;
	case IS_VULNERABLE:	save /= 2;	break;
    }

    if (!IS_NPC(victim) && class_table[victim->class].fMana)
	save = 4 * save / 5;
    minnum = level / 2;
    minnum += ( ( level - victim->level ) / 5 ) * 2;
    minnum = URANGE( 1, minnum, 75 );
    save = URANGE( minnum, save, 75 );
    return number_percent( ) < save; */
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *third_name;
char *type_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target, magtype;
    bool channeled = FALSE;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) && ch->desc == NULL)
	return;

    REMOVE_BIT( ch->affected_by, AFF_HIDE ); 

    if ( !str_cmp(class_table[ch->class].name,"bard") )
    {
	send_to_char( "Try singing instead.\n\r", ch );
	return;
    }

    if(IS_SET(ch->in_room->room_flags,ROOM_SILENCED) && !IS_IMMORTAL(ch))
    {
     send_to_char("You cannot find the power to cast.\n\r",ch);
     return;
    }

    target_name = one_argument( argument, arg1 );
    third_name = one_argument( target_name, arg2 );
    strcpy( target_name, arg2 );
    one_argument( third_name, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
	    act("You attempt to cast, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
	    act("$n attempts to cast, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
	    WAIT_STATE(ch,DEF_FEAR_WAIT);
	    return; }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( sn = find_spell( ch,arg1 ) ) < 0
    || ( !IS_NPC(ch) && ((ch->level < ch->pcdata->learnlvl[sn]
    && ch->level < LEVEL_HERO+1)
    ||   		 ch->pcdata->learned[sn] == 0)))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }
 
	if(skill_table[sn].spell_fun == spell_null){
   	send_to_char("You don't know any spells of that name.\n\r",ch);
        return;
	}
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    if(is_affected(ch,skill_lookup("headache")) && number_percent() < 20 )
    {
     act("Your head throbs in pain and you muddle your words.\n\r",ch,NULL,NULL,TO_CHAR);
     act("$n's words turn to gibberish.",ch,NULL,NULL,TO_ROOM);
     WAIT_STATE(ch,12);
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
            do_say( keeper, "{_No magic in here, kid.{x" );
            ch->mana -= mana / 3;
            return;
        }
     }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
       {
          char buf[MAX_STRING_LENGTH];
          sprintf( buf,
             "%s tries to cast a spell, which fizzles and dies.", ch->name );
          act(buf,ch,NULL,NULL,TO_ROOM);
          sprintf( buf, "Your spell fizzles and dies." );
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
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
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
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


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

/*        if (victim->fighting != NULL &&
                !is_same_group (ch, victim->fighting))
        {
                send_to_char ("Kill stealing is not permitted.\n\r", ch);
                return;
        }   */

/*
	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}
 */

        if (!IS_NPC(ch) && !IS_NPC(victim) && !is_safe(ch,victim)) {
	 if(!IN_ARENA(ch) && !IN_ARENA(victim))
	 {
	         ch->fight_timer = pktimer;
	         victim->fight_timer = pktimer;
	 }
        }

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    if (ch->class == CLASS_MONK) 
    {
    	if (is_affected(ch,gsn_martial_arts) && //Only giant/haste/frenzy available to martial arts monks. 
    	(str_cmp(skill_table[sn].name, "giant strength") && str_cmp(skill_table[sn].name, "frenzy") && str_cmp(skill_table[sn].name, "haste")))
    	{
	    send_to_char("You must put your mind at peace before casting this spell.\n\r",ch);
	    return;
    	}
	}
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

/*
	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}
*/

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
    if (is_affected(ch,gsn_martial_arts) && (ch->class == CLASS_MONK))
    {
	    send_to_char("You must put your mind at peace before casting this spell.\n\r",ch);
	    return;
    }
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
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
		send_to_char("Cast the spell on whom or what?\n\r",ch);
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

/*            if (victim->fighting != NULL &&
                !is_same_group (ch, victim->fighting))
            {
                send_to_char ("Kill stealing is not permitted.\n\r", ch);
                return;
            } */
  
/*
	    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	    && ( !IS_IMMORTAL( ch ) )
	    && ( !IS_IMMORTAL( victim ) )
	    && ( ch != victim )
	    && ( !skill_table[sn].socket )
	    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	    {
		send_to_char("Spell failed.\n\r",ch);
		return;
	    }
*/

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
    	//No removing curses while martial arting for monks.
    	if ((is_affected(ch,gsn_martial_arts)) && (ch->class == CLASS_MONK) && (!strcmp(skill_table[sn].name, "remove curse")))
    	{
	    	send_to_char("You must put your mind at peace before casting this spell.\n\r",ch);
	    	return;
    	}
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {

/*
	    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	    && ( !IS_IMMORTAL( ch ) )
	    && ( !IS_IMMORTAL( victim ) )
	    && ( ch != victim )
	    && ( !skill_table[sn].socket )
	    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	    {
		send_to_char("Spell failed.\n\r",ch);
		return;
	    }
*/

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
/*
	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}
*/

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

        if(IS_SET(victim->in_room->room_flags,ROOM_REMORT) && !IS_IMMORTAL(ch))
        {
         send_to_char("They are in remort, you cannot transport anything to them.\n\r",ch);
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

        send_to_char("Your eyes flash {ggreen{x.\n\r",ch);
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
	say_spell( ch, sn );
      
    WAIT_STATE( ch, skill_table[sn].beats );

    switch(magic_table[sn])
    {
     case MAGIC_NONE: magtype = MAGIC_NONE; break;
     case MAGIC_WHITE: magtype = MAGIC_WHITE; improve_magic(MAGIC_WHITE,ch);
       send_to_char("Your eyes flash {wwhite{x.\n\r",ch); break;
     case MAGIC_RED:   magtype = MAGIC_RED; improve_magic(MAGIC_RED,ch);
       send_to_char("Your eyes flash {rred{x.\n\r",ch); break;
     case MAGIC_BLUE:  magtype = MAGIC_BLUE; improve_magic(MAGIC_BLUE,ch);
       send_to_char("Your eyes flash {bblue{x.\n\r",ch); break;
     case MAGIC_GREEN: magtype = MAGIC_GREEN; improve_magic(MAGIC_GREEN,ch);
       send_to_char("Your eyes flash {ggreen{x.\n\r",ch); break;
     case MAGIC_BLACK: magtype = MAGIC_BLACK; improve_magic(MAGIC_BLACK,ch);
       send_to_char("Your eyes flash black.\n\r",ch); break;
     case MAGIC_ORANGE: magtype = MAGIC_ORANGE; improve_magic(MAGIC_ORANGE,ch);
       send_to_char("Your eyes flash {yorange{x.\n\r",ch); break;
     default: magtype = MAGIC_NONE; break;
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

    /*
     *  OLD CALCULATIONS
     *  if (IS_NPC(ch) || class_table[ch->class].fMana)
     *   class has spells 
     *  (*skill_table[sn].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
     */

       channeled = check_channel(ch,victim,sn);

       if ((!channeled))
       {
        if (IS_NPC(ch)) // MOBILES Use level
          (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        else if(magtype != MAGIC_NONE)
        {
         if(ch->magic[magtype] < 75) // 1 - 48 // 46 for 1 - 34 // old 62
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*46)/100, ch, vo,target);
         else if(ch->magic[magtype] < 105) // 47 - 65 // 47 for 35 - 49 // old 61
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*47)/100, ch, vo,target);
         else if(ch->magic[magtype] < 135) // 65 - 83 // 48 for 50 - 66 // old 60
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*48)/100, ch, vo,target);
         else if(ch->magic[magtype] < 170) // 82 - 103 // 50 for 67 - 86 // old 59
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*50)/100, ch, vo,target);
         else if(ch->magic[magtype] < 190) // 103 - 110 // 51 for 87 - 96 // old 58
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*51)/100, ch, vo,target);
         else     // 112 - 118 // 51 for 97 - 102 // old 57
          (*skill_table[sn].spell_fun) (sn, (ch->magic[magtype]*51)/100, ch, vo,target);
        }
        else
          (*skill_table[sn].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
        check_improve(ch,sn,TRUE,1);
       }
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
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo = NULL;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    if (ch->fighting != NULL)
    {
	WAIT_STATE( ch, skill_table[sn].beats );
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
	{
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
        }
	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
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
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
/*
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    dam = dice( level, 14 );

    if ( ch->class == 0 ) // 0 is the class number for Mage
    {
        dam = dice( level * 3, 14 );
     }
    if ( ch->class == 7 ) // 7 is the class number for wizard
    {
        dam = dice( level * 4, 14 );
    }
    if ( ch->class >= 14 && ch->class <= 16 )
    {
        dam = dice( level * 5, 14 );
    }

    dam /= 1.7;

    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage_old( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
*/
//
 CHAR_DATA *victim = (CHAR_DATA *) vo;
 int dam;

 /*if ((ch->fighting == NULL) && (!IS_NPC(ch)) && (!IS_NPC(victim))) {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
 } */

 	if (IS_NPC(ch)) 
 	{
   		dam = dice(level*4,14);
   		dam /= 1.6;
   		if (saves_spell(level,victim,DAM_ACID))
		    dam /=2;
   		damage_old(ch,victim,dam,sn,DAM_ACID,TRUE);
   		return; 
   	}

	if (check_sorcery(ch,sn))
        	dam = sorcery_dam(level*3,14,ch);
    	else
		dam = dice(level*3,14);
   
   	if(ch->pcdata->tier == 3) 
   	{
	dam *=1.4; 
	}
   	else if(ch->pcdata->tier == 2) 
   	{
	dam *=1.2; 
	}

   	if((ch->class == 0) || (ch->class == 7) || ((ch->class >= 14) && (ch->class <=16)))
		dam *=1.1;

   	dam/=1.6;
   	
   	if (saves_spell(level,victim,DAM_ACID))
		dam /= 2;
   	
   	damage_old(ch, victim, dam, sn, DAM_ACID, TRUE);
   	return;

}

void spell_acid_rain(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch,hitnum;

    act("$n calls forth acid from the sky $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n calls forth acid from the sky upon you!",ch,NULL,victim,TO_VICT);
    act("You call acid from the sky upon $N.",ch,NULL,victim,TO_CHAR);
  
  for ( hitnum = 1; hitnum < (dice(1,3)+1); hitnum++ ) 
  {
    hpch = UMAX(11,ch->hit);
    hpch = UMIN(5500,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/3.5);
    
    	if (check_sorcery(ch,sn))
        	dice_dam = sorcery_dam(4.8,22,ch);
    	else
		dice_dam = dice(4,22);

    dam = UMAX(hp_dam + dice_dam/15,dice_dam + hp_dam/14);

    dam = dice(ch->level/18,(dam*2)/2)*1.3;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if ( saves_spell(level,victim,DAM_ACID) )
    {
	damage_old(ch,victim,dam/3,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_ACID,TRUE); 
    }
  }
}

void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int affected;

    if ( is_affected( victim, sn ) )
    {
       affected = 1;
       affect_strip(victim, skill_lookup("armor") );
	/*if (victim == ch)
	  send_to_char("You are already armored.\n\r",ch);
	else
	  act("$N is already armored.",ch,NULL,victim,TO_CHAR);
	return; */
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -30;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( affected == 1)
       send_to_char("Your armor is renewed!\n\r",victim);
    else
      send_to_char( "You feel someone protecting you.\n\r", victim );
    
    if ( ch != victim )
	act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 10*(6 + level);
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_OTHER))
	return;


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = dice (1,3);
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 14, 15,
	17, 18, 20, 21, 23,	24, 26, 27, 28, 29,
	29, 29, 29, 29, 30,	30, 30, 30, 31, 31,
	31, 31, 32, 32, 32,	32, 33, 33, 33, 33,
	34, 34, 34, 34, 35,	35, 35, 35, 36, 36,
	36, 36, 37, 37, 37,	37, 38, 38, 38, 38,
	39, 39, 39, 39, 40,	40, 40, 40, 41, 41,
	41, 41, 42, 42, 42,	42, 43, 43, 43, 43,
	44, 44, 44, 44, 45,	45, 45, 45, 46, 46,
	46, 46, 47, 47, 47,	47, 48, 48, 48, 48
    };
    int dam;
    
   
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 ) + 10;
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);
        
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_FIRE,TRUE);
    return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return;
    }

    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level/2, 8,ch);
    else    
    	dam = dice(level/2, 8);

    act( "$g's {Ylightning{x strikes your foes!", ch, NULL, NULL, TO_CHAR );
    act( "$n calls $g's {Ylightning{x to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
	    {
		if ( ( ch->fighting == NULL )
		&& ( !IS_NPC( ch ) )
		&& ( !IS_NPC( vch ) ) )
		{
		    ch->attacker = TRUE;
		    vch->attacker = FALSE;
		}
		damage_old( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "{z{YLightning{x flashes in the sky.\n\r", vch );
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    
    
    
    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if(!is_pkill(ch) && is_pkill(vch))
  {
     send_to_char("If you want to pkill become pk!\n\r", ch);
     return;
  }
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
    chance = 8 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(1, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\n\r",vch);

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


void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
    AFFECT_DATA *af;
 
    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }

    if ((!IS_NPC(victim) && victim != ch && !IS_IMMORTAL(ch) && !is_same_group(ch,victim)) || IS_SET(victim->act,PLR_NOCANCEL))
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */

        for ( af = victim->affected; af != NULL; af = af->next )
        {
                if(check_dispel(level,victim,af->type))
                {
                 /* Insert Dispel Room Acts here */
                    found = TRUE;
		}
        }

   /*

    if (check_dispel(level,victim,skill_lookup("armor")))
	{
		act("$n's shield fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
	}
 
    if (check_dispel(level,victim,skill_lookup("bless")))
    {
		act("$n is no longer blessed.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
		found = TRUE;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
    {
	    found = TRUE;
 		act("$n's looks more confortable.",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect motion")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect motion")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
    {
	    found = TRUE;
 		act("The red in $n's eyes disappears.",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
	{
		act("$n can no longer walk through walls",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
	}

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE; 
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("sleep")))
    {
		act("$n doesn't look as drousy.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    */
 
    if (found)
        send_to_char("Victim Cancelled.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

/* Siphon Life spell for warlocks/voodans by Dusk */

void spell_siphon_life( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
    int dam, mod, pbonus, fbonus, dbonus;    
 
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    } 

    pbonus = ( !IS_NPC( ch ) && !IS_NPC( victim ) ? focus_level(ch->pcdata->focus[MAGIC_POWER]) - focus_level(victim->pcdata->focus[MAGIC_POWER]) : 0 );
    fbonus = ( !IS_NPC( ch ) ? ch->pcdata->focus[MAGIC_ABILITY] : 0 );
    dbonus = ( !IS_NPC( victim ) ? victim->pcdata->focus[MAGIC_DEFENSE] : 0 );

    fbonus = ( ( fbonus - dbonus < 0 ) ? 0 : fbonus );
    
    pbonus =  ( pbonus < 0 ? -1 : pbonus / 2 );
    pbonus =  ( pbonus > 5 ? 5 : pbonus );   

    mod = ( ( ch->level + get_curr_stat( ch , STAT_INT ) + fbonus ) - ( victim->saving_throw * -1 + get_curr_stat( victim , STAT_WIS ) ) );
    dam = ch->level + get_curr_stat( ch, STAT_INT ) * 8 - victim->saving_throw * -1;
    dam = ( check_sorcery( ch, sn ) ? sorcery_dam2(dam*1.15) : dam );
    
    if ( victim == ch )
    {
       send_to_char("{wYou can't siphon your own life!{x\n\r",ch);
       return;
    }   
 
    if ( is_affected( victim , gsn_siphon ) || is_affected( ch , gsn_siphon)  )
    {
       send_to_char("{mYou draw on your siphoned link to gain more life!{x\n\r",ch);
       send_to_char("{RYou feel your lifeforce fading fast as it is being drawn on!{x\n\r",victim);
       
       victim->hit -= dam;
       ch->hit += dam;      
    }
    else
    {

       af.where            = TO_AFFECTS;
       af.type             = gsn_siphon;
       af.level            = ch->level;
       af.duration         = 2 + pbonus;
       af.location         = APPLY_REGEN;
       af.modifier         = mod;
       af.bitvector        = 0; 
    
       affect_to_char(ch, &af);
    
       af.modifier         = mod * -1;
    
       affect_to_char(victim, &af);
    
       send_to_char("{RYou feel your life being siphoned away!{x\n\r",victim);
       send_to_char("{mYou feel your life being restored by your siphon!{x\n\r",ch);
    }
        
    damage_old( ch, victim, dam, sn,DAM_HARM,TRUE);
    return;
}

/* End of Siphon Life Spell by Dusk */
/* Siphon Energy Spell by Dusk */

void spell_siphon_energy( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
    int dam, mod, pbonus, fbonus, dbonus;    
 
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    } 

    pbonus = ( !IS_NPC( ch ) && !IS_NPC( victim ) ? focus_level(ch->pcdata->focus[MAGIC_POWER]) - focus_level(victim->pcdata->focus[MAGIC_POWER]) : 0 );
    fbonus = ( !IS_NPC( ch ) ? ch->pcdata->focus[MAGIC_ABILITY] : 0 );
    dbonus = ( !IS_NPC( victim ) ? victim->pcdata->focus[MAGIC_DEFENSE] : 0 );

    fbonus = ( ( fbonus - dbonus < 0 ) ? 0 : fbonus );
    
    pbonus =  ( pbonus < 0 ? -1 : pbonus / 2 );
    pbonus =  ( pbonus > 5 ? 5 : pbonus );   

    mod = ( ( ch->level + get_curr_stat( ch , STAT_INT ) + fbonus ) - ( victim->saving_throw * -1 + get_curr_stat( victim , STAT_WIS ) ) );
    dam = ch->level + get_curr_stat( ch, STAT_INT ) * 8 - victim->saving_throw * -1;
    dam = ( check_sorcery( ch, sn ) ? sorcery_dam2(dam*1.15) : dam );
    
    if ( victim == ch )
    {
       send_to_char("{wYou can't siphon your own energy!{x\n\r",ch);
       return;
    }   
 
    if ( is_affected( victim , gsn_siphon ) || is_affected( ch , gsn_siphon)  )
    {
       send_to_char("{mYou draw on your siphoned link to gain more energy!{x\n\r",ch);
       send_to_char("{RYou feel your energy supply fading fast as it is being drawn on!{x\n\r",victim);
       
       victim->mana -= dam;
       if ( victim-> mana < 0 )
          victim->mana = 0;
       else
          ch->mana += dam;      
    }
    else
    {

       af.where            = TO_AFFECTS;
       af.type             = gsn_siphon;
       af.level            = ch->level;
       af.duration         = 2 + pbonus;
       af.location         = APPLY_MANA_REGEN;
       af.modifier         = mod;
       af.bitvector        = 0; 
    
       affect_to_char(ch, &af);
    
       af.modifier         = mod * -1;
    
       affect_to_char(victim, &af);
    
       send_to_char("{RYou feel your energy being siphoned away!{x\n\r",victim);
       send_to_char("{mYou feel your energy being restored by your siphon!{x\n\r",ch);
    }
        
    damage_old( ch, victim, dam, sn,DAM_HARM,TRUE);
    return;
}

void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    if (check_sorcery(ch,sn))
        dam = sorcery_dam(6,13,ch);
    else        
    	dam = dice(6, 13);

    damage_old( ch, victim, dam + level / 2, sn,DAM_HARM,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam, count=0;

    /* first strike */

     act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level,6,ch);
    else        
    	dam = dice(level,6);

    if (saves_spell(level,victim,DAM_LIGHTNING))
 	dam /= 3;
 	
    damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 6;   /* decrement damage */
    count++;

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
          if (!is_same_group(ch,tmp_vict) && tmp_vict != last_vict && (!IS_IMMORTAL(tmp_vict) || can_see(ch,tmp_vict)))
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    if (last_vict == ch)
	    {
             if (number_percent() >= level/3)
	      {
	       act("The bolt arcs back to $n, and it stabilizes!",tmp_vict,NULL,NULL,TO_ROOM);
	       act("The bolt arcs back to you and you stabilize its energy!",tmp_vict,NULL,NULL,TO_ROOM);
	       level+=8;
	      }
	     else
	      {
	       act("The bolt arcs back to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	       act("The bolt arcs back to you!",tmp_vict,NULL,NULL,TO_ROOM);
	      }
	    }
	    else
	    {
             if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
              act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
             if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
              act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,6);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	    damage_old(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 6;  /* decrement damage */
            count++;
	    }
	  }
        }

	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	  {
            act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
            act("The bolt grounds out through your body.",ch,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
          if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
          if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
          send_to_char("You ground out the lightning bolt.\n\r",ch);
	  level -= 6;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim,DAM_OTHER))
	return;	
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/5;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo, *wch;
    AFFECT_DATA af;
    int count = 0;

    if (is_safe(ch,victim)) return;
    if (!IS_NPC(victim)) return;
  	if(IS_SET(ch->plyr,PLAYER_GHOST))
	{ send_to_char("You are a ghost!, you cannot charm right now.\n\r",ch);
	return; }

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   ch->fighting != NULL
    ||   victim->fighting != NULL
    ||   dice(1,100) < 50
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level+5 < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_CHARM)
    ||   saves_spell( level - 5, victim,DAM_CHARM)
    ||   saves_spell( level + 5, victim,DAM_CHARM))
    {
	send_to_char("Spell Failed.\n\r",ch);
	multi_hit( victim, ch, TYPE_UNDEFINED );
	return;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( IS_AFFECTED(wch,AFF_CHARM)
	&&   wch->master == ch )
	{
	    if ( ++count >= 3 )
	    {
		send_to_char( "You can not control that many monsters.\n\r", ch );
		return;
	    }
	}
    }

 /*   if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    } */
  
    if ( victim->master != NULL )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level * 3 / 2;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  6,	 6,  7,  7,  8,  8,
	 9, 10, 12, 12, 13,	13, 13, 13, 13, 13,
	14, 14, 14, 14, 14,	14, 15, 15, 15, 15,
	15, 15, 16, 16, 16,	16, 16, 16, 17, 17,
	17, 17, 17, 17, 18,	18, 18, 18, 18, 18,
	19, 19, 19, 19, 19,	19, 20, 20, 20, 20,
	20, 20, 21, 21, 21,	21, 21, 21, 22, 22,
	22, 22, 22, 22, 23,	23, 23, 23, 23, 23,
	24, 24, 24, 24, 24,	24, 25, 25, 25, 25,
	25, 25, 26, 26, 26,	26, 26, 26, 27, 27
    };
    AFFECT_DATA af;
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

       dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
       
       if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);

    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = level / 10;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    damage_old( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 32, 35, 37, 40,	42, 45, 47, 50, 52,
	55, 55, 55, 55, 55,	55, 56, 56, 57, 57,
	58, 58, 58, 58, 59,	59, 60, 60, 61, 61,
	61, 61, 62, 62, 63,	63, 64, 64, 64, 64,
	65, 65, 66, 66, 67,	67, 67, 67, 68, 68,
	69, 69, 70, 70, 70,	70, 71, 71, 72, 72,
	73, 73, 73, 73, 74,	74, 75, 75, 76, 76,
	76, 76, 77, 77, 78,	78, 79, 79, 79, 79
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

    dam = number_range( dam_each[level] / 2,  dam_each[level] * 2 ) + 2*level;
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);

    if ( saves_spell( level, victim,DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_LIGHT,TRUE );
    return;
}

void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}



void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = level / 4;
    mushroom->value[1] = level / 4;
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful {Rred rose{x.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful {Rred rose{x.\n\r",ch);
    obj_to_char(rose,ch);
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(4, 14) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 10) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(3, 9) + level ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR);
                return;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = level/10;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/10;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 7);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, dam_mod, chance;

    chance = 100;
    dam_mod = 10;
    if (IS_GOOD(ch))
    {
	dam_mod = 5;
        chance = 0;
    }
    else if (IS_NEUTRAL(ch))
    {
        dam_mod = 7;
        chance = 50;
    }

    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level*4,10,ch);
    else 
    	dam = dice( level*4, 10 );

    if ( ch->class == 1 ) /* 1 is the class number for Cleric */
    {
       if (check_sorcery(ch,sn))
        	dam = sorcery_dam(level*3,12,ch);
       else  
       		dam = dice( level*3, 12 );
    }
    if ( ch->class == 8 ) /* 8 is the class number for Priest */
    {
       if (check_sorcery(ch,sn))
        dam = sorcery_dam(level*4,11,ch);
       else 
       	dam = dice( level*4, 11 );
    }
    if ( ch->class == CLASS_VOODAN || ch->class == CLASS_FADE
    || ch->class == CLASS_SHAMAN || ch->class == CLASS_BANSHEE
    || ch->class == CLASS_NECROMANCER)
    {
       if (check_sorcery(ch,sn))
        dam = sorcery_dam(level*5,10,ch);
       else 
        dam = dice( level*5, 10 );
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);
    if ( ch->pet != NULL )
	ch->pet->alignment = ch->alignment;

    if (number_percent() > chance)
    {
	act("$n's demonfire turns on $m!",ch,0,0,TO_ROOM);
	act("Your demonfire turns on you!",ch,0,0,TO_CHAR);
        dam *= dam_mod;
        dam /= 10;
		if (saves_spell(level,ch,DAM_NEGATIVE))
			dam /= 2;
        damage_old(ch,ch,dam,sn,DAM_NEGATIVE,TRUE);
        return;
    }

    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
	dam *= 3/2;

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;

    dam = (dam * (ch->alignment * -1) ) / 1500;

    if (!IS_GOOD(victim))
      dam /= 2;

    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_motion(int sn,int level,CHAR_DATA *ch,void *vo,int
target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MOTION) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense movement.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_MOTION;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "{&O{7di{&n{x protects $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (victim->hit > (ch->level * 4)) {
       dam = dice( level, 4 ) + level/3;
    }
    else {
       dam = UMAX(victim->hit, dice(level,4)) + level/3;
    }
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);

    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;
 
    if ( IS_EVIL(victim) )
    {
        act( "{!H{1e{!l{x protects $N.", ch, NULL, victim, TO_ROOM );
        return;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return;
    }
 
    if (victim->hit > (ch->level * 4)) {
       dam = dice( level, 4 ) + level/3;
    }
    else {
       dam = UMAX(victim->hit, dice(level,4)) + level/3;
    }
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    return;
}


void spell_dispel_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target ){
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (!IS_SET(obj->extra_flags, ITEM_INVIS))
    {
        act("$p is not invisible!",ch,obj,NULL,TO_CHAR);
        return;
    }

   REMOVE_BIT(obj->extra_flags, ITEM_INVIS);

   act("$p fades into sight.",ch,obj,NULL,TO_ALL);
   return;
}



/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if ((IS_NPC(ch) || IS_NPC(victim))
    && (saves_spell(level, victim,DAM_OTHER)))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    } else if (!IS_NPC(ch) && !IS_NPC(victim))
    {
	if ( is_clan( victim )
	&& !is_clan( ch ) )
	{
	    send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	    send_to_char( "You failed.\n\r", ch);
	    return;
	} else if ( saves_dispel_magic( level, victim,DAM_OTHER ) )
	{
	    send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	    send_to_char( "You failed.\n\r", ch);
	    return;
	} else if ( is_clan( victim )
	&& !is_same_clan( ch, victim )
	&& is_safe_spell( ch, victim, FALSE ) )
	{
	    send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	    send_to_char( "You failed.\n\r", ch);
	    return;
	}
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
    {
	    found = TRUE;
 		act("$n's body armor diminishes.",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("bless")))
    {
	    found = TRUE;
 		act("$n does not look as blessed anymore.",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
        found = TRUE;
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
    {
	    found = TRUE;
 		act("$n looks more comfortable.",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect motion")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect motion")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
/*
    if (IS_SHIELDED(victim,SHD_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
        && !is_affected(victim,skill_lookup("globe of invulnerability")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_SHIELDED(victim,SHD_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
        && !is_affected(victim,skill_lookup("globe of invulnerability")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
*/
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
    {
	    found = TRUE;
		act("$n does not look as drousy",victim,NULL,NULL,TO_ROOM);
	}

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);

    if (!IS_NPC(ch) && !IS_NPC(victim))
    {
	if ( is_clan( victim )
	&& !is_same_clan( ch, victim ) )
	{
	    if ( ch->fighting == NULL )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    multi_hit( victim, ch, TYPE_UNDEFINED );
	}
    }
    return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    dam = dice( 5, 8 ) + level / 3;
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam2(dam);

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
        if ( vch->in_room == NULL || is_same_group(ch,vch) || IS_IMMORTAL(vch))
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if ( ( ch->fighting == NULL )
		&& ( !IS_NPC( ch ) )
		&& ( !IS_NPC( vch ) ) )
		{
		    ch->attacker = TRUE;
		    vch->attacker = FALSE;
		}
		if (IS_AFFECTED(vch,AFF_FLYING))
		    damage_old(ch,vch,0,sn,DAM_BASH,TRUE);
		else
                    damage_old( ch,vch,level + dam, sn, DAM_BASH,TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_char("That isn't an armor.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }
/* new noenchant code by bree */
     if (IS_OBJ_STAT(obj,ITEM_NOENCHANT) || IS_OBJ_STAT(obj,ITEM_NOENCHANT))
        {
            if (!IS_OBJ_STAT(obj,ITEM_NOENCHANT)
            &&  !saves_dispel(level + 2,obj->level,0))
            {
                return;
            }

            act("This item cannot be enchanted.",ch,obj,NULL,TO_CHAR);

            return;
        } 
/* end no enchant code */


    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 15;	/* base 15% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_AC )
            {
	    	ac_bonus = paf->modifier;
		ac_found = TRUE;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus + 1);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (90 - level/5))  /* success! */
    {
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
 	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_AC;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

}




void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }
/* new noenchant code by bree */
    if (IS_OBJ_STAT(obj,ITEM_NOENCHANT) || IS_OBJ_STAT(obj,ITEM_NOENCHANT))
        {
            if (!IS_OBJ_STAT(obj,ITEM_NOENCHANT)
            &&  !saves_dispel(level + 2,obj->level,0))
            {
                return;
            }

            act("This item cannot be enchanted.",ch,obj,NULL,TO_CHAR);
        
            return;
        }
/* end no enchant code */


    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
		hit_found = TRUE;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
		dam_found = TRUE;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
	{
	    af_new = new_affect();
	
	    af_new->next = obj->affected;
	    obj->affected = af_new;

	    af_new->where	= paf->where;
	    af_new->type 	= UMAX(0,paf->type);
	    af_new->level	= paf->level;
	    af_new->duration	= paf->duration;
	    af_new->location	= paf->location;
	    af_new->modifier	= paf->modifier;
	    af_new->bitvector	= paf->bitvector;
	}
    }

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where	= TO_OBJECT;
	paf->type	= sn;
	paf->level	= level;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	=  added;
	paf->bitvector  = 0;
    	paf->next	= obj->affected;
    	obj->affected	= paf;
    }

    if (hit_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
            if ( paf->location == APPLY_HITROLL)
            {
		paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX(paf->level,level);
                if (paf->modifier > 4)
                    SET_BIT(obj->extra_flags,ITEM_HUM);
            }
	}
    }
    else /* add a new affect */
    {
        paf = new_affect();
 
        paf->type       = sn;
        paf->level      = level;
        paf->duration   = -1;
        paf->location   = APPLY_HITROLL;
        paf->modifier   =  added;
        paf->bitvector  = 0;
        paf->next       = obj->affected;
        obj->affected   = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
        ch->alignment = UMAX(-1000, ch->alignment - 50);
	if ( ch->pet != NULL )
	    ch->pet->alignment = ch->alignment;
    }

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }


    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
        gain_exp( victim, (0 - number_range( level/3, 4 * level / 3 ))/2 );
        victim->mana   = ((victim->mana * 5)/6);
        victim->move   = ((victim->move * 5)/6);

        dam = dice( level, 5 );

	ch->hit		+= dam;
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);

    return;
}

void spell_entity(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *held;

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    held = get_eq_char (ch, WEAR_HOLD);

    if ( !held )
    {
	send_to_char( "You aren't holding anything.",ch );
	return;
    }

    if (is_name("titan",held->name))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      int dam;

      dam = dice(5*level, 8) + dice(1, level/2);

      send_to_char( "A {Rbloodthirsty{x titan rises from the {yground{x in front of you!\n\r", ch );
      act( "$n summons a {Rbloodthirsty{x titan, who rises from the ground in a deafening roar.", ch, NULL, NULL, TO_ROOM );

      for ( vch = char_list; vch != NULL; vch = vch_next )
      {
  	  vch_next	= vch->next;
	  if ( vch->in_room == NULL )
	      continue;
	  if ( vch->in_room == ch->in_room )
	  {
	      if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	      {
	  	  if ( ( ch->fighting == NULL )
		  && ( !IS_NPC( ch ) )
		  && ( !IS_NPC( vch ) ) )
		  {
		      ch->attacker = TRUE;
		      vch->attacker = FALSE;
		  }
		  if (IS_AFFECTED(vch,AFF_FLYING))
		      damage_old(ch,vch,0,sn,DAM_BASH,TRUE);
		  else
                      damage_old( ch,vch, dam, sn, DAM_BASH,TRUE);
	      }
	      continue;
	  }

	  if ( vch->in_room->area == ch->in_room->area )
	      send_to_char( "The earth cracks and shivers.\n\r", vch );
      }

      return;
    }
    else if (is_name("dragon",held->name))
    {
      int dam;
     
      act("A {WP{wlatinum{x dragon swoops down from somewhere unknown.",ch,NULL,victim,TO_ROOM);
      act("The dragon breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
      act("The dragon breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
      act("The dragon breathes a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

	  if (ch->mana > 10000)
      {
       			dam = dice(level, 20)*10.5;
	  }
	  else
	  {
       			dam = dice(level, 20)*10.5;
	  }

	/*NPCs don't get the advantage of everything*/
	  if ( IS_NPC( ch) )
		dam /= 3;


      if ( ( ch->fighting == NULL )
      && ( !IS_NPC( ch ) )
      && ( !IS_NPC( victim ) ) )
      {
  	  ch->attacker = TRUE;
	  victim->attacker = FALSE;
      }
      if (saves_spell(level,victim,DAM_LIGHTNING))
      {
	  damage_old(ch,victim,dam/3,sn,DAM_LIGHTNING,TRUE);
      }
      else
      {
  	  shock_effect(victim,level,dam,TARGET_CHAR);
	  damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
      }
    }
    else if (is_name("snake",held->name))
    {
      int dam;
     
      act("The ground opens beneath you and a huge venomous {Gs{gnake{x appears!",ch,NULL,victim,TO_ROOM);
      act("The snake spews acid at  $N.",ch,NULL,victim,TO_NOTVICT);
      act("The snake spews acid at you!",ch,NULL,victim,TO_VICT);
      act("The snake spews acid at $N.",ch,NULL,victim,TO_CHAR);

	  if (ch->mana > 10000)
      {
       			dam = dice(level, 20)*11;
	  }
	  else
	  {
       			dam = dice(level, 20)*11;
	  }

	/*NPCs don't get the advantage of everything*/
	  if ( IS_NPC( ch) )
		dam /= 3;


      if ( ( ch->fighting == NULL )
      && ( !IS_NPC( ch ) )
      && ( !IS_NPC( victim ) ) )
      {
  	  ch->attacker = TRUE;
	  victim->attacker = FALSE;
      }
      if (saves_spell(level,victim,DAM_ACID))
      {
	  damage_old(ch,victim,dam/3,sn,DAM_ACID,TRUE);
      }
      else
      {
  	  shock_effect(victim,level,dam,TARGET_CHAR);
	  damage_old(ch,victim,dam,sn,DAM_ACID,TRUE); 
      }
    }
    else
    {
      send_to_char( "You raise your fists to the air with no effect, perhaps you are missing something.\n\r", ch );
      act( "$n raises both fists to the sky with no visible effect.", ch, NULL, NULL, TO_ROOM );

      return;
    }
}


void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,	  0,   0,   0,  30,  32,
	 35,  37,  40,  42,  45,	 47,  50,  52,  55,  57,
	 60,  62,  65,  67,  70,	 72,  75,  77,  80,  81,
	 82,  83,  84,  85,  86,	 87,  88,  89,  90,  91,
	 92,  93,  94,  95,  96,	 97,  98,  99, 100, 101,
	102, 103, 104, 105, 106,	107, 108, 109, 110, 111,
	112, 113, 114, 115, 116,	117, 118, 119, 120, 121,
	122, 123, 124, 125, 126,	127, 128, 129, 130, 131
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );

    dam = number_range( dam_each[level] , dam_each[level] * 3 );

    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = level*10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    dam = dice(6 + level / 2, 9);

    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/10;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;
    AFFECT_DATA af;

    if(IS_SET(ch->in_room->affected_by,ROOM_AFF_FAERIE) )
    {
     send_to_char("This room is already surrounded in faerie fog.\n\r",ch);
     return;
    }

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    af.where = TO_ROOM_AFF;
    af.type = sn;
    af.duration = level/30;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.level = level;
    af.bitvector = ROOM_AFF_FAERIE;
    affect_to_room( ch->in_room, &af);

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if ((ich->invis_level > 0) || (ich->ghost_level > 0))
	    continue;

        if ( ich == ch || saves_spell( level, ich, DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->shielded_by, SHD_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
        affect_strip ( ich, gsn_forest_blend            );
        affect_strip ( ich, gsn_earthmeld               );
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= ch->level * 15; /* 15 pounds per level capacity */
    disc->value[3]	= ch->level * 10; /* 10 pounds per level max per item */
    disc->timer		= ch->level * 5 - number_range(0,level / 2); 

    act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You create a floating disc.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE);
    return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_fortify_strength( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int count = 0;
 
    if ( is_affected(ch, skill_lookup( "fortify intelligence") ) )
        count ++;
    if ( is_affected(ch, skill_lookup( "fortify wisdom") ) )
        count ++;
    if ( is_affected(ch, skill_lookup( "fortify dexterity") ) )
        count ++;
    if ( is_affected(ch, skill_lookup( "fortify constitution") ) )
        count ++;

    if ( count >= 3 )
    {
       send_to_char("You can only withstand fortifying three attributes!.\n\r",ch);
       return;
    }      

    if ( is_affected(ch, skill_lookup( "fortify strength") ) )
    {
       send_to_char("You prolong your fortification to its max!\n\r",ch);
       affect_strip( ch, skill_lookup( "fortify strength" ) );
       count = 6;
    }
	   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_STR;
    af.modifier  = (get_curr_stat(ch,STAT_INT)*4)/10;
    af.bitvector = 0;
    
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = ch->level/2;

    affect_to_char( victim, &af);
    
    if ( count != 6 )
    {
       send_to_char( "Your strength has been fortified!\n\r", victim );
       act( "$n's muscles twitch, as they become fortified.", victim, NULL, NULL, TO_ROOM );
    }
    return;
}

void spell_fortify_intelligence( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int count = 0;
 
    if ( is_affected(victim, skill_lookup( "fortify strength") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify wisdom") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify dexterity") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify constitution") ) )
        count ++;

    if ( count >= 3 )
    {
       send_to_char("You can only withstand fortifying three attributes!.\n\r",ch);
       return;
    }      

    if ( is_affected(victim, skill_lookup( "fortify intelligence") ) )
    {
       send_to_char("You prolong your fortification to its max!\n\r",ch);
       affect_strip( ch, skill_lookup( "fortify intelligence" ) );
       count = 6;
    }
	   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	   = level;
    af.duration  = level/4;
    af.location  = APPLY_INT;
    af.modifier  = (get_curr_stat(ch,STAT_INT)*4)/10;
    af.bitvector = 0;
    
    affect_to_char( victim, &af );

    af.location  = APPLY_MANA;
    af.modifier  = ch->level*5;

    affect_to_char(victim, &af );
   
    if ( count != 6 )
    {
       send_to_char( "Your intelligence has been fortified!\n\r", victim );
       act( "$n's mind expands, as they become fortified.", victim, NULL, NULL, TO_ROOM );
    }
    return;
}

void spell_fortify_wisdom( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int count = 0;
 
    if ( is_affected(victim, skill_lookup( "fortify intelligence") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify strength") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify dexterity") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify constitution") ) )
        count ++;

    if ( count >= 3 )
    {
       send_to_char("You can only withstand fortifying three attributes!.\n\r",ch);
       return;
    }      

    if ( is_affected(victim, skill_lookup( "fortify wisdom") ) )
    {
       send_to_char("You prolong your fortification to its max!\n\r",ch);
       affect_strip( ch, skill_lookup( "fortify wisdom" ) );
       count = 6;
    }
	   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	   = level;
    af.duration  = level/4;
    af.location  = APPLY_WIS;
    af.modifier  = (get_curr_stat(ch,STAT_INT)*4)/10;
    af.bitvector = 0;
    
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVES;
    af.modifier  = ch->level/6*-1;
 
    affect_to_char( victim, &af );
 
    if ( count != 6 )
    {
       send_to_char( "Your wisdom has been fortified!\n\r", victim );
       act( "$n's knowledge expands, as they become fortified.", victim, NULL, NULL, TO_ROOM );
    }
    return;
}

void spell_fortify_dexterity( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int count = 0;
 
    if ( is_affected(victim, skill_lookup( "fortify intelligence") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify wisdom") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify strength") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify constitution") ) )
        count ++;

    if ( count >= 3 )
    {
       send_to_char("You can only withstand fortifying three attributes!.\n\r",ch);
       return;
    }      

    if ( is_affected(victim, skill_lookup( "fortify dexterity") ) )
    {
       send_to_char("You prolong your fortification to its max!\n\r",ch);
       affect_strip( ch, skill_lookup( "fortify dexterity" ) );
       count = 6;
    }
	   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = (get_curr_stat(ch,STAT_INT)*4)/10;
    af.bitvector = 0;
    
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = ch->level/2;

    affect_to_char( victim, &af );

    if ( count != 6 )
    {
       send_to_char( "Your dexterity has been fortified!\n\r", victim );
       act( "$n's prowless increases, as they become fortified.", victim, NULL, NULL, TO_ROOM );
    }
    return;
}

void spell_fortify_constitution( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int count = 0;
 
    if ( is_affected(victim, skill_lookup( "fortify intelligence") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify wisdom") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify dexterity") ) )
        count ++;
    if ( is_affected(victim, skill_lookup( "fortify strength") ) )
        count ++;

    if ( count >= 3 )
    {
       send_to_char("You can only withstand fortifying three attributes!.\n\r",ch);
       return;
    }      

    if ( is_affected(victim, skill_lookup( "fortify constitution") ) )
    {
       send_to_char("You prolong your fortification to its max!\n\r",ch);
       affect_strip( ch, skill_lookup( "fortify constitution" ) );
       count = 6;
    }
	   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/4;
    af.location  = APPLY_CON;
    af.modifier  = (get_curr_stat(ch,STAT_INT)*4)/10;
    af.bitvector = 0;
    
    affect_to_char( victim, &af );
    
    af.location  = APPLY_HIT;
    af.modifier  = ch->level*5;

    affect_to_char( victim, &af );

    if ( count != 6 )
    {
       send_to_char( "Your constitution has been fortified!\n\r", victim );
       act( "$n's body hardens, as they become fortified.", victim, NULL, NULL, TO_ROOM );
    }
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
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
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = AFF_BERSERK;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 30);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if( ch->fighting != NULL)
    {
     send_to_char("Your arcane words are lost in the heat of battle.\n\r",ch);
     return;
    }

    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 

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
    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch,victim))
    ||   clan_table[victim->clan].independent)))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
    if (global_quest && IS_NPC(victim) && victim->on_quest)
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }
    if (victim->in_room->area->continent != ch->in_room->area->continent) {
	send_to_char("You reach out, but the destination is too far.\n\r", ch);
    	return; }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
}

void spell_forestwalk( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if( ch->fighting != NULL)
    {
     send_to_char("You cannot escape into the forest during battle.\n\r",ch);
     return;
    }

    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 
/*
    if (victim == ch)
    {
	    if (has_skill(ch,skill_lookup("blaze"))
	    {
	    		if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    			||   ch->pcdata->recall == 0
   			    ||   !can_see_room(ch,ch->pcdata->recall) 
  			    ||   IS_SET(ch->pcdata->recall->room_flags, ROOM_SAFE)
    			||   IS_SET(ch->pcdata->recall->room_flags, ROOM_PRIVATE)
    			||   IS_SET(ch->pcdata->recall->room_flags, ROOM_SOLITARY)
    			||   IS_SET(ch->pcdata->recall->room_flags, ROOM_NO_RECALL)
    			||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    			||   victim->level >= level + 4
    			||   IS_SET(ch->pcdata->recall->area->area_flags, AREA_RESTRICTED)
    			{
        			send_to_char( "You cannot find the path.\n\r", ch );
        			return;
    			}
    			else
    			if ( strstr( ch->pcdata->recall->area->builders, "Unlinked" ) )
    			{
				send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
				return;
    			}
    			if (ch->pcdata->recall->area->continent != ch->in_room->area->continent) {
	    			send_to_char( "You begin to travel but the distance is too great.\n\r",ch);
	    			return; }

    			if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
				gate_pet = TRUE;
    			else
				gate_pet = FALSE;
    
    			act("$n steps into the surrounding foliage and dissappears.",ch,NULL,NULL,TO_ROOM);
    			send_to_char("You step into the forest and travel to your destination.\n\r",ch);
    			char_from_room(ch);
    			char_to_room(ch,ch->pcdata->recall);

    			if(!IS_SET(ch->pcdata->recall->sector_type, SECT_FOREST))
     			act("$n steps out from behind a tree.",ch,NULL,NULL,TO_ROOM);
    			do_look(ch,"auto");

    			if (gate_pet)
    			{
        			act("$n follows $s master into the forest.",ch->pet,NULL,NULL,TO_ROOM);
				send_to_char("You step through a gate and vanish.\n\r",ch->pet);
				char_from_room(ch->pet);
				char_to_room(ch->pet,ch->pcdata->recall);
        			if(!IS_SET(ch->pcdata->recall->sector_type, SECT_FOREST))
         			act("$n rolls out from behind a bush.",ch->pet,NULL,NULL,TO_ROOM);
				do_look(ch->pet,"auto");
    			}
		}
		else
		{
		return;
		}
	}
	else
	{
*/    
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 4
    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch,victim))
    ||   clan_table[victim->clan].independent)))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "You cannot find the path.\n\r", ch );
        return;
    }
    
    if (global_quest && IS_NPC(victim) && victim->on_quest)
    {
        send_to_char( "You cannot create a path to someone on a quest.\n\r", ch );
        return;
    }
    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }
    if (victim->in_room->area->continent != ch->in_room->area->continent) {
	    send_to_char( "You begin to travel but the distance is too great.\n\r",ch);
	    return; }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps into the surrounding foliage and dissappears.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step into the forest and travel to your destination.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    if(!IS_SET(victim->in_room->sector_type, SECT_FOREST))
     act("$n steps out from behind a tree.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
        act("$n follows $s master into the forest.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
        if(!IS_SET(victim->in_room->sector_type, SECT_FOREST))
         act("$n rolls out from behind a bush.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
}

void spell_shadowwalk( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    CHAR_DATA *fch, *fch_next; 
    bool gate_pet;

    if( ch->fighting != NULL)
    {
     send_to_char("You cannot escape into the darkness during battle.\n\r",ch);
     return;
    }

    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 4
    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch,victim))
    ||   clan_table[victim->clan].independent)))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "You cannot find the path.\n\r", ch );
        return;
    }
    if (global_quest && IS_NPC(victim) && victim->on_quest)
    {
        send_to_char( "You cannot create a path to someone on a quest.\n\r", ch );
        return;
    }
    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }
    if (victim->in_room->area->continent != ch->in_room->area->continent) {
	    send_to_char( "You begin to travel but the destination is too far.\n\r",ch);
	    return; }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps into the surrounding shadows and dissappears.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step into the shadows and travel to your destination.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    if(!IS_SET(victim->in_room->room_flags, ROOM_DARK))
     act("$n steps out of the shadows.",ch,NULL,NULL,TO_ROOM);

    do_look(ch,"auto");

    if (gate_pet)
    {
        act("$n follows $s master into the shadows.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
      if(!IS_SET(victim->in_room->room_flags, ROOM_DARK))
        act("$n steps out of the shadows.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
    
    if(ch->class == CLASS_NECROMANCER)
    {
    
                for (fch = ch->in_room->people; fch != NULL; fch = fch_next) 
                { 
                        fch_next = fch->next_in_room; 
 
                        if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM) 
                                && fch->position < POS_STANDING) 
                                fch->position = POS_STANDING; 
 
                        if (fch->master == ch && fch->position == POS_STANDING) 
                        { 
                        	act("$n follows $s master into the shadows.",fch,NULL,NULL,TO_ROOM);
				send_to_char("You step through a gate and vanish.\n\r",fch);
				char_from_room(fch);
				char_to_room(fch,victim->in_room);
      				if(!IS_SET(victim->in_room->room_flags, ROOM_DARK))
        			act("$n steps out of the shadows.",ch->pet,NULL,NULL,TO_ROOM);
				do_look(fch,"auto");
                	}
               }	
    }
}



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_AFFECTED(victim,AFF_WEAKEN))
    {
        if (!check_dispel(level,victim,skill_lookup("weaken")))
        {
            if (victim != ch)
                send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily stronger.\n\r",victim);
            return;
        }
        act("$n is looking a bit stronger.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 3 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}

/*
 * Sorcery on this? Maybe
 */
void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = UMAX(  20, victim->hit - dice(1,4) );

    if ( saves_spell( level, victim,DAM_HARM) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage_old( ch, victim, dam, sn, DAM_HARM ,TRUE);
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 3 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    /* Saints get double heal */
    if(ch->class == CLASS_SAINT)
    {
      if (dice (1,3) == 1)
      {
	victim->hit = UMIN( victim->hit + dice(50,8), victim->max_hit );
	send_to_char( "Torrents of vitality surge through your body!\n\r",victim );
      }
    victim->hit = UMIN( victim->hit +350, victim->max_hit );
    update_pos( victim );
    send_to_char( "A hot feeling fills your body.\n\r", victim );
    return;
    }

    victim->hit = UMIN( victim->hit + 200, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;

   if ( IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
     {
     send_to_char("That spell is illegal in the Arena.", ch);
     return;
     }
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2.5 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL))
            {
                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
		        act("$n yelps and throws $p!",
			    victim,obj_lose,NULL,TO_ROOM);
		        act("You remove $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level*3));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
                        act("$n yelps and throws $p!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You remove $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon!\n\r",
			    victim);
			dam += 1;
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose))
                    {
                        act("$n throws a burning hot $p!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You remove $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    } 
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if ( ( ch->fighting == NULL )
	&& ( !IS_NPC( ch ) )
	&& ( !IS_NPC( victim ) ) )
	{
	    ch->attacker = TRUE;
	    victim->attacker = FALSE;
	}
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage_old(ch,victim,dam*3/2,sn,DAM_FIRE,TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((is_clan(ch) && is_same_clan(ch,vch)) ||
	    (IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
 	  send_to_char("You feel full more powerful.\n\r",vch);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
	  spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) ||
 		 (is_clan(ch) && !is_same_clan(ch,vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( vch ) ) )
	    {
		ch->attacker = TRUE;
		vch->attacker = FALSE;
	    }
            spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);

            dam = dice(level,15);

            damage_old(ch,vch,dam*3/2,sn,DAM_ENERGY,TRUE);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( vch ) ) )
	    {
		ch->attacker = TRUE;
		vch->attacker = FALSE;
	    }
            spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);

            dam = dice(level,10);

	    damage_old(ch,vch,dam*3/2,sn,DAM_ENERGY,TRUE);
   	  }
	}
    }  
    
    send_to_char("You feel drained.\n\r",ch);
    ch->move -= 200;
    if (ch->move <= 100)
	ch->move = 100;
    ch->hit -= 150;
    if (ch->hit <= 50)
	ch->hit = 50;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    sprintf( buf,
	"Object '%s' is type %s.\n\rWear flags %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
	obj->name,
	item_type_name( obj ),
        wear_bit_name(obj->wear_flags),
	extra_bit_name( obj->extra_flags ),
	obj->weight / 10,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
        sprintf(buf,"It holds %s-colored %s.\n\r",
            liq_table[obj->value[2]].liq_color,
            liq_table[obj->value[2]].liq_name);
        send_to_char(buf,ch);
        break;

    case ITEM_CONTAINER:
    case ITEM_PIT:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);
	if (obj->value[4] != 100)
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
		
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("spear/staff.\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("mace/club.\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("axe.\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("flail.\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);	break;
	    default		: send_to_char("unknown.\n\r",ch);	break;
 	}
	if (obj->clan)
	{
	    sprintf( buf, "Damage is variable.\n\r");
	} else
	{
	    if (obj->pIndexData->new_format)
		sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	    else
		sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	}
	send_to_char( buf, ch );
        if (obj->value[4])  /* weapon flags */
        {
            sprintf(buf,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
        }
	break;

    case ITEM_ARMOR:
	if (obj->clan)
	{
	    sprintf( buf, "Armor class is variable.\n\r");
	} else
	{
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	}
	send_to_char( buf, ch );
	break;
    }
    if (obj->owner != NULL)
    {
     sprintf( buf, "This object is owned by %s{x.\n\r",obj->owner);
     send_to_char( buf, ch );
    }
    if (is_clan_obj(obj))
    {
	sprintf( buf, "This object is owned by the [%s{x] clan.\n\r",
	    clan_table[obj->clan].who_name
	    );
	send_to_char( buf, ch );
    }
    if (is_class_obj(obj))
    {
	sprintf( buf, "This object may only be used by a %s.\n\r",
	    class_table[obj->class].name
	    );
	send_to_char( buf, ch );
    }
    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_SHIELDS:
                        sprintf(buf,"Adds %s shield.\n",
                            shield_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
            if ( paf->duration > -1)
                sprintf(buf,", %d hours.\n\r",paf->duration);
            else
                sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_SHIELDS:
                        sprintf(buf,"Adds %s shield.\n",
                            shield_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
    }
    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;	

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_SHIELDED(victim, SHD_INVISIBLE) )
	return;

    if ( IS_NPC( victim )
    &&  IS_SET( victim->off_flags, OFF_CLAN_GUARD ) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,	 0, 25, 26, 28, 29,
	31, 32, 34, 35, 37,	38, 39, 40, 40, 40,
	41, 41, 42, 42, 42,	43, 43, 43, 44, 44,
	44, 45, 45, 45, 46,	46, 46, 47, 47, 47,
	48, 48, 48, 49, 49,	49, 50, 50, 50, 51,
	51, 51, 52, 52, 52,	53, 53, 53, 54, 54,
	54, 55, 55, 55, 56,	56, 56, 57, 57, 57,
	58, 58, 58, 59, 59,	59, 60, 60, 60, 61,
	61, 61, 62, 62, 62,	63, 63, 63, 64, 64
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );

    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	||   ch->level < obj->level)
	    continue;


	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    if (IS_IMMORTAL(ch) || !IS_NPC(in_obj->carried_by)
	    || !global_quest || !in_obj->carried_by->on_quest)
	    {
		found = TRUE;
		number++;
		sprintf( buf, "one is carried by %s\n\r",
		    PERS(in_obj->carried_by, ch) );
		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);
	    }
	}
	else
	{
	    found = TRUE;
	    number++;
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "one is in %s [Room %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
	    	sprintf( buf, "one is in %s\n\r",
		    in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	    buf[0] = UPPER(buf[0]);
	    add_buf(buffer,buf);
	}

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int count = 0, i, extra=0;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  3,  3,  4,	 4,  4,  4,  5,  5,
	 6,  6,  6,  6,  6,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 7,  7,  7,  7,  7,
	 8,  8,  8,  8,  8,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	 9,  9,  9,  9,  9,
	10, 10, 10, 10, 10,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	11, 11, 11, 11, 11,
	12, 12, 12, 12, 12,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	13, 13, 13, 13, 13,
	14, 14, 14, 14, 14,	14, 14, 14, 14, 14
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);

    if(ch->level < 20)
     count = 1;
    else if(ch->level < 40)
     count = 2;
    else if(ch->level < 60)
     count = 3;
    else if(ch->level < 80)
     count = 4;
    else if(ch->level < 100)
     count = 5;
    else
     count = 6;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    for(i = 0; i < count; i++)
    {
     dam = number_range( dam_each[level] * 2, dam_each[level] * 2 ) + extra;

     if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;

     damage_old( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
     if(i > 0)
      extra = dice(10,i);
    }
    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_SHIELDED(gch, SHD_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_SHIELDS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = SHD_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }
	
    if ((victim->level > 100) && (!IS_AFFECTED(victim,AFF_PLAGUE)) && !IS_NPC(victim))
	{
		victim->move -= 300; //Legends lose move upon the first affliction
	}
    
    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level * 3/4;
    af.duration  = level;
    af.location  = APPLY_CON;
    af.modifier  = -1*number_range(1,5); 
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}
void spell_voodan_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim->class == CLASS_VOODAN)
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }
if ((victim->level > 100) && (!is_affected(victim,skill_lookup("voodan curse"))) && !IS_NPC(victim) && (victim->mana > 0) && (victim->move > 0))
	{	//Legends lose move and mana upon the first affliction
		if (victim->move > victim->max_move*2/3)
		{
			victim->move /= 2;
		}
		else
		{
			victim->move -= 300;
		}
		if (victim->mana > victim->max_mana*2/3)
		{
			victim->mana /= 2;
		}
		else
		{
			victim->mana -= 400;
		}
	}
if (!is_affected(victim,skill_lookup("voodan curse")))
{
    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level;
    af.duration  = level/6;
    af.location  = APPLY_CON;
    af.modifier  = -1*number_range(5,20); 
    af.bitvector = AFF_CURSE;
    affect_to_char(victim,&af);
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_STR;
    af.modifier  = -1 * number_range(5,20);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );
}
    
if (victim->hitroll > 125)
{
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * number_range((level/10),(level/5));
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );
}
       send_to_char
      ("You scream in agony as the {Dv{Wo{Dod{wa{Dn {Dcurse{x runs rampant through your body!\n\r",victim);
    act("$n screams in agony as the {Dv{Wo{Dod{wa{Dn {Dcurse{x does its work.",
	victim,NULL,NULL,TO_ROOM);
}
void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	 //   ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level;
	    af.duration	 = level/2;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_STR;
    af.modifier  = -1 * number_range(1,4);
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_SHIELDED(victim, SHD_PROTECT_EVIL) 
    ||   IS_SHIELDED(victim, SHD_PROTECT_GOOD))
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
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = SHD_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_SHIELDED(victim, SHD_PROTECT_GOOD) 
    ||   IS_SHIELDED(victim, SHD_PROTECT_EVIL))
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
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = SHD_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, dam_mod, chance/*, align*/;
    chance = 100;
    dam_mod = 10;

    if (IS_EVIL(ch))
    {
	dam_mod = 7;
        chance = 0;
    }
    else if (IS_NEUTRAL(ch))
    {
        dam_mod = 5;
        chance = 50;
    }
 
    dam = dice( level*2, 10 );

    if ( ch->class == CLASS_CLERIC ) 
    {
       dam = dice( level*3, 10 );
    }
    if ( ch->class == CLASS_PRIEST ) 
    {
       dam = dice( level*4, 10 );
    }
    if ( ch->class == CLASS_SAINT || ch->class == CLASS_MONK
    || ch->class == CLASS_SHAMAN || ch->class == CLASS_VOODAN)
    {
       dam = dice( level*5, 10 );
    }

    if (number_percent() > chance)
    {
        act("$n's holy energy explodes within $m!",ch,0,0,TO_ROOM);
        act("Your holy energy explodes within you!",ch,0,0,TO_CHAR);
        dam *= dam_mod;
        dam /= 10;
        if (saves_spell(level,ch,DAM_HOLY))
		dam /= 2;
        damage_old(ch,ch,dam,sn,DAM_HOLY,TRUE);
        return;
    }

    if (victim != ch)
    {
        act("$n raises $s hand, and a blinding ray of light shoots forth!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char("You raise your hand and a blinding ray of light shoots forth!\n\r",ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    if ( saves_spell( level, victim, DAM_HOLY) )
        dam /= 2;

    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    spell_blindness(gsn_blindness, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}
void spell_soul_burn (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    

    dam = dice( level*5, 10 );

    if (victim != ch)
    {
        act("$n begins to chant and a torrent of black flames come forth!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char("You begin to chant and a torrent of black flames come forth!!\n\r",ch);
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    damage_old( ch, victim, dam, sn, DAM_HARM ,TRUE);
    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
    }

void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if ((obj->item_type != ITEM_WAND) & (obj->item_type != ITEM_STAFF))
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level*2, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	else
	{
	    act("There is no curse on $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_restore_mana( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->mana = UMIN( victim->mana + level*2, victim->max_mana );
    if (victim->max_mana == victim->mana)
        send_to_char("You feel fully focused!\n\r",victim);
    else
        send_to_char( "You feel more focused.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

/* Now called Globe of Invulnerability -- Sanctuary for old purposes */
void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(!IS_SHIELDED(victim,SHD_SANCTUARY) && is_warrior_class(victim->class))
    {
     send_to_char("A white aura surrounds you but fades away.\n\r",ch);
     return;
    }

    if ( IS_SHIELDED(victim, SHD_SANCTUARY) )
    {
     if(victim->class == CLASS_MAGE || victim->class == CLASS_WIZARD ||
        (victim->class >= CLASS_FORSAKEN && victim->class <= CLASS_ARCHMAGE))
     {
       send_to_char("You are already in a globe of invulnerability.\n\r",ch);
     }
     else if(victim->class == CLASS_SHADE || victim->class == CLASS_LICH ||
        (victim->class >= CLASS_FADE && victim->class <= CLASS_BANSHEE))
     {
       send_to_char("You are already protected by a darkshield.\n\r",ch);
     }
     else
     {
       send_to_char("You are already protected.\n\r",ch);
     }
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 4;
    af.location  = APPLY_AC;
    af.modifier  = -10;
    af.bitvector = SHD_SANCTUARY;
    affect_to_char( victim, &af );

    if(victim->class == CLASS_MAGE || victim->class == CLASS_WIZARD ||
       (victim->class >= 15 && victim->class <= 17))
    {
    act( "$n is surrounded by a white globe of energy.\n\r",victim,NULL,NULL,TO_ROOM);
    send_to_char("You are surrounded by a globe of invulnerability.\n\r",victim);
    }
    else if(victim->class == CLASS_SHADE || victim->class == CLASS_LICH ||
        (victim->class >= CLASS_FADE && victim->class <= CLASS_BANSHEE))
    {
     act( "$n is surrounded by a darkened shield.\n\r",victim,NULL,NULL,TO_ROOM);
     send_to_char("You are surrounded by a darkened shield.\n\r",victim);
    }
    else
    {
    act( "$n is surrounded by a white aura.\n\r", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    }
    return;
}

/* Creates a safe-area for 200 mana / tick */
void spell_room_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
	{
    AFFECT_DATA af;
/*
    if(ch->fight_timer > 0)
    {
     send_to_char("The Aesir do not grant this room sanctuary during battle.\n\r",ch);
     return;
    }
*/
    if(is_affected(ch,gsn_room_sanctuary))
    {
     send_to_char("You cannot muster the energy to call upon the Aesir yet.\n\r",ch);
     ch->mana += 300;
     return;
    }

    if ( IS_SET(ch->in_room->affected_by, ROOM_AFF_SAFE) ||
    IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("This Room is already a sanctuary.\n\r",ch);
        ch->mana += 300;
        return;
    }

    af.where     = TO_ROOM_AFF;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ROOM_AFF_SAFE;
    affect_to_room( ch->in_room, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    act( "$n blesses the surroundings. You feel safe.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You create a temporary sanctuary.\n\r", ch );
    return;
}




void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = - level/5;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	 0,  0, 20, 22, 25,	27, 29, 31, 33, 34,
	36, 37, 38, 39, 39,	39, 39, 39, 40, 40,
	40, 40, 41, 41, 41,	41, 42, 42, 42, 42,
	43, 43, 43, 43, 44,	44, 44, 44, 45, 45,
	45, 45, 46, 46, 46,	46, 47, 47, 47, 47,
	48, 48, 48, 48, 49,	49, 49, 49, 50, 50,
	50, 50, 51, 51, 51,	51, 52, 52, 52, 52,
	53, 53, 53, 53, 54,	54, 54, 54, 55, 55,
	55, 55, 56, 56, 56,	56, 57, 57, 57, 57
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 ) + level;

    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   ch->fighting != NULL
    ||   victim->fighting != NULL
    ||   IS_SET(ch->plyr,PLAYER_GHOST)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   ch->level+10 < victim->level
    ||   saves_spell( level, victim, DAM_CHARM))
/*    ||   saves_spell( level-5, victim,DAM_CHARM)
    ||   saves_spell( level+5, victim,DAM_CHARM))


    ||   saves_spell( level-7, victim,DAM_CHARM)
    ||   saves_spell( level+7, victim,DAM_CHARM)) */

    {
     send_to_char("You failed.\n\r",ch);
     multi_hit( victim, ch, TYPE_UNDEFINED );
     return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
	affect_strip(victim,gsn_hide);
	REMOVE_BIT(victim->affected_by, AFF_HIDE );
	
	if(!IS_NPC(victim) && !IS_NPC(ch) && !IN_ARENA(ch) && !IN_ARENA(victim))
	{
	     ch->fight_timer = pktimer;
	     victim->fight_timer = pktimer;
	}
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
             send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    } 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}




void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
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
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -level / 3;
    af.bitvector = SHD_STONE;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(ch->in_room->room_flags, ROOM_ARENA)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch, victim))
    ||   clan_table[victim->clan].independent)))
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (global_quest && IS_NPC(victim) && victim->on_quest)

    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
    
    if(victim->fight_timer > 0) 
    {
	send_to_char("You try to summon them, but something blocks their arrival.\n\r",ch);
	return;
    }

    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    do_look( victim, "auto" );
    return;
}



void spell_transport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	return;
}

void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if(ch->fight_timer > 0) 
    {
	send_to_char("In some kind of hurry?\n\r",ch);
	return;
    }

    if (is_safe(ch,victim) && victim != ch)
    {
        send_to_char("Not on that target.\n\r",ch);
        return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '{S%s{x'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '{S%s{x'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) )
	    send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,skill_lookup("giant strength")))
    {
        if (!check_dispel(level,victim,skill_lookup("giant strength")))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily weaker.\n\r",victim);
            return;
        }

        act("$n is a bit weaker.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = dice (2,8);
    af.location  = APPLY_STR;
    af.modifier  = -1 * (dice(2,5));
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}



/* RT recall spell is back */
/* RW recall spell is gone again */

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    
    if (IS_NPC(victim))
      return;

    return;

    if ( ch->alignment < 0 )
    {
	if ((location = get_room_index( ROOM_VNUM_TEMPLEB)) == NULL)
	{
	    send_to_char("You are completely lost.\n\r",victim);
	    return;
	}
    }
    else
    {
	if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
	{
	    send_to_char("You are completely lost.\n\r",victim);
	    return;
	} 
    }

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);
    
    ch->move *= .75;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_look(victim,"auto");
}

/*
 * Breath Spells - Used by NPCs so be careful!
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
    act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

	//putting a limit on amount of hps
	if (ch->hit > 15000)
		hpch = UMAX(14, 15000);
    else
		hpch = UMAX(14,ch->hit);

    hp_dam = number_range(hpch/11 + 1, hpch/6);

    dice_dam = dice(level*3/2,22);

    dam = UMAX(hp_dam + dice_dam/10 + 4,dice_dam + hp_dam/10 + 4) + ch->level*5;
	dam *= 2.2;
	
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if (saves_spell(level,victim,DAM_ACID))
    {
	acid_effect(victim,level/3,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_ACID,TRUE);
    }
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

	//putting a limit on amount of hps
	if (ch->hit > 15000)
		hpch = UMAX(17, 15000);
    else
		hpch = UMAX(17,ch->hit);

    hp_dam  = number_range( hpch/9+1, hpch/5 );

    dice_dam = dice(level,22);

    dam = UMAX(hp_dam + dice_dam / 10 + 4, dice_dam + hp_dam / 10 + 4) + ch->level*5;
	fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);
	dam *= number_range(3,40)/10;
	
    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if ((is_safe_spell(ch,vch,TRUE)	||  (IS_NPC(vch) && IS_NPC(ch)) || (is_same_group(ch,vch))) 
	&& (ch->fighting != vch || vch->fighting != ch))
	continue;

	if (vch == victim) /* full damage */
	{
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( victim ) ) )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	if (vch == ch) /* less damage to caster */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		//fire_effect(vch,level/4,dam/40,TARGET_CHAR);
		damage_old(ch,vch,dam/16,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		//fire_effect(vch,level/2,dam/12,TARGET_CHAR);
		damage_old(ch,vch,dam/8,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage_old(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
  }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam, hpch;

    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

	//putting a limit on amount of hps
	if (ch->hit > 15000)
		hpch = UMAX(15, 15000);
    else
		hpch = UMAX(15,ch->hit);

    hp_dam = number_range(hpch/11 + 1, hpch/6);

    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10 + 4,dice_dam + hp_dam/10 + 4) + ch->level*5;
	cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 
	dam *= number_range(21,28)/10;
	
    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	
	//Don't hit groupies and friends and pets and stuff.  Copied into other room breaths as well.
	if ((is_safe_spell(ch,vch,TRUE)	||  (IS_NPC(vch) && IS_NPC(ch)) || (is_same_group(ch,vch))) 
	&& (ch->fighting != vch || vch->fighting != ch))
	continue;

	if (vch == victim) /* full damage */
	{
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( victim ) ) )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else if (vch == ch) 
	{
		if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		//cold_effect(vch,level/4,dam/20,TARGET_CHAR);
		damage_old(ch,vch,dam/12,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		//cold_effect(vch,level/2,dam/16,TARGET_CHAR);
		damage_old(ch,vch,dam/6,sn,DAM_COLD,TRUE);
		}
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage_old(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
	}
}

    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

	//putting a limit on amount of hps
	if (ch->hit > 15000)
		hpch = UMAX(12, 15000);
    else
		hpch = UMAX(12,ch->hit);
    
    hp_dam = number_range(hpch/15+1,8);

    dice_dam = dice(level,18);

    dam = UMAX(hp_dam + dice_dam/10 + 4,dice_dam + hp_dam/10 + 4) + ch->level*5;
	poison_effect(ch->in_room,level,dam,TARGET_ROOM);
	dam *= number_range(15,30)/10;
	
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;
	if ((is_safe_spell(ch,vch,TRUE)	||  (IS_NPC(vch) && IS_NPC(ch)) || (is_same_group(ch,vch))) 
	&& (ch->fighting != vch || vch->fighting != ch))
	continue;

	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( vch ) ) )
	    {
		ch->attacker = TRUE;
		vch->attacker = FALSE;
	    }
	if (vch == ch)
	{
		//poison_effect(vch,level/2,dam/12,TARGET_CHAR);
	    damage_old(ch,vch,dam/10,sn,DAM_POISON,TRUE);
	}
	else if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage_old(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage_old(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
    }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

	//putting a limit on amount of hps
	if (ch->hit > 15000)
		hpch = UMAX(16, 15000);
    else
		hpch = UMAX(16,ch->hit);

    hp_dam = number_range(hpch/9+1,hpch/5);

      dice_dam = dice(level,21);

    dam = UMAX(hp_dam + dice_dam/10 + 4,dice_dam + hp_dam/10 + 4) + ch->level*5;
	dam *= number_range(25,35)/10;
	
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level+40,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 * Sorcery?
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

/*
 * Sorcery?
 */
void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 600, 800 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 1;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam*10, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_guardian( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;


    victim=create_mobile( get_mob_index( MOB_VNUM_GUARDIAN ) );
    victim->level = level;
    victim->hit = 100*level;
    victim->max_hit = 100*level;
    victim->hitroll = level;
    victim->damroll = level;
    victim->armor[0] = 100 - (level*7);
    victim->armor[1] = 100 - (level*7);
    victim->armor[2] = 100 - (level*7);
    victim->armor[3] = 100 - (level*7);

    strcpy(buf,"Come forth, creature of darkness, and do my bidding!");
    do_say( ch, buf );

    send_to_char( "A demon bursts from the ground and bows before you.\n\r",ch );
    act( "$N bursts from the ground and bows before $n.", ch, NULL, victim, TO_ROOM );

    char_to_room( victim, ch->in_room );

    add_follower( victim, ch );
    af.type      = sn;
    af.duration  = 666;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    return;
}

void spell_stomp( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

      dam = 3*level + dice (15, 20);

    send_to_char( "The earth splits open beneath your feet!\n\r", ch );
    act( "$n makes the earth split and crumble.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if ( ( ch->fighting == NULL )
		&& ( !IS_NPC( ch ) )
		&& ( !IS_NPC( vch ) ) )
		{
		    ch->attacker = TRUE;
		    vch->attacker = FALSE;
		}
		if (IS_AFFECTED(vch,AFF_FLYING))
		    damage_old(ch,vch,0,sn,DAM_SOUND,TRUE);
		else
                    damage_old( ch,vch,dam, sn, DAM_SOUND,TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth splits and crumbles.\n\r", vch );
    }

    return;
}

/*
 * MagType System written for Asgardian NightMare
 * by Skyntil(Gabe Volker) 4/20/00
 */
void improve_magic(int type, CHAR_DATA *ch)
{
 // int dice1,dice2,dice3;
 int dice1;
 char buf[MAX_STRING_LENGTH/4];
 bool send_message = FALSE;

 /* dice1 = number_percent();
 dice2 = number_percent();
 dice3 = number_percent(); */

 buf[0] = '\0';

 if(ch->magic[type] > 200)
  ch->magic[type] = 200;
 if(ch->magic[type] == 200)
   return;

 // Improve?
 dice1 = ((300 - ch->magic[type]) * 2);
 if((number_percent() * 1500000) < (dice1 * dice1 * dice1)) {
   ch->magic[type] += 1;
   send_message = TRUE; }

 // Now send the Messages
 if(ch->magic[type] == 2)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 35)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 76)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 100)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 134)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 168)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 190)
   sprintf(buf,"You have advanced in your mastery of %s magic!\n\r", get_magtype_name(type));
 else if(ch->magic[type] == 200)
   sprintf(buf,"You are now a master of %s magic!\n\r", get_magtype_name(type));

 if(buf[0] != '\0' && send_message)
  send_to_char(buf,ch);

 return;
}

char *get_magtype_name(int type)
{
    static char buf[512];

    buf[0] = '\0';


    switch(type)
    {
     case MAGIC_WHITE: strcpy(buf," {wwhite{x"); break;
     case MAGIC_RED:   strcpy(buf," {rred{x"); break;
     case MAGIC_BLUE:  strcpy(buf," {bblue{x"); break;
     case MAGIC_GREEN: strcpy(buf," {ggreen{x"); break;
     case MAGIC_BLACK: strcpy(buf," black"); break;
     case MAGIC_ORANGE: strcpy(buf," {yorange{x"); break;
     default: break;
    }

     return ( buf[0] != '\0' ) ? buf+1 : "unknown";
}

void spell_conjure_wrath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *held;

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    held = get_eq_char (ch, WEAR_HOLD);

    if ( !held )
    {
	send_to_char( "You aren't holding anything.\n\r",ch );
	return;
    }

    if ( is_name( "light", held->name ) )
    {
	CHAR_DATA *vch, *vch_next;
	int dam;

	send_to_char( "You conjure several small balls of pulsing energy!\n\r", ch );
	act( "Several small globes of energy appear in $n's hands.", ch, NULL, NULL, TO_ROOM );

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( vch->in_room == ch->in_room
	    &&   vch != ch
	    &&   !is_same_group( ch, vch )
	    &&   !is_safe_spell( ch, vch, TRUE ) )
	    {
		if ( IS_AFFECTED( vch, AFF_HASTE ) )
		    dam = dice( 4 * level, 9 ) + dice( 1, level / 2 );
		else
		    dam = dice( 4 * level, 10 );

		damage_old( ch,vch, dam, sn, DAM_BASH, TRUE );
		continue;
	    }

	    if ( vch->in_room && vch->in_room->area == ch->in_room->area )
		send_to_char( "You see a bright flash in the sky.\n\r", vch );
	}

	return;
    }

    else if (is_name("powder",held->name) || is_name("herb",held->name))
    {
      int dam=0,hp_dam,dice_dam,hpch,weapon = 0, dam_type=DAM_NONE;

      weapon = number_range(0,3);

      if (weapon == 3)
       weapon = number_range(0,3);
      if (weapon == 3)
       weapon = number_range(0,3);

      switch(weapon) {
      case 0:
       act("$n conjures forth sickly knives from Hell's Kitchen!",ch,NULL,victim,TO_ROOM);
       act("The knives impale $N.",ch,NULL,victim,TO_NOTVICT);
       act("You are impaled by a dozen knives!",ch,NULL,victim,TO_VICT);
       act("You conjure knives to impale $N.",ch,NULL,victim,TO_CHAR);

	   if (ch->hit > 15000)
		  hpch = UMAX(10,15000);
       else
		  hpch = UMAX(10,ch->hit);
       hp_dam = number_range(hpch/9+1,hpch/5);

        dice_dam = dice(level, 18);

       dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
       dam *= 3/2;
       dam += dice(2,level);
       dam_type = DAM_PIERCE;
       break;
      case 1:
       act("$n conjures forth razor shuriken from the Ninja Training Grounds!",ch,NULL,victim,TO_ROOM);
       act("The razor shuriken cut into $N.",ch,NULL,victim,TO_NOTVICT);
       act("Several shuriken cut into your flesh!",ch,NULL,victim,TO_VICT);
       act("You conjure shuriken to cut $N.",ch,NULL,victim,TO_CHAR);

	   if (ch->hit > 15000)
		  hpch = UMAX(10,15000);
       else
		  hpch = UMAX(10,ch->hit);
       hp_dam = number_range(hpch/9+1,hpch/5);

       dice_dam = dice(level, 21);

       dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
       dam *= 3;
       dam += dice(2,level);
       dam_type = DAM_SLASH;
       break;
      case 2:
       act("$n conjures forth fiery boulders from the Cliffs of Doom!",ch,NULL,victim,TO_ROOM);
       act("The boulders smash and incinerate $N.",ch,NULL,victim,TO_NOTVICT);
       act("You are crushed by burning rocks!",ch,NULL,victim,TO_VICT);
       act("You conjure fiery boulders to crush $N.",ch,NULL,victim,TO_CHAR);

	   if (ch->hit > 15000)
		  hpch = UMAX(10,15000);
       else
		  hpch = UMAX(10,ch->hit);
       hp_dam = number_range(hpch/9+1,hpch/5);

       dice_dam = dice(level, 16);

       dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
       dam *= 2;
       dam += dice(2,level);
       dam_type = DAM_BASH;
       break;
      case 3:
       act("$n conjures forth a rubber chicken? Whoops!",ch,NULL,victim,TO_ROOM);
       //act("The chicken turns on $n! Squaak!.",victim,NULL,ch,TO_NOTVICT);
       act("You conjure forth a rubber chicken by accident!",victim,NULL,ch,TO_VICT);
       //act("The rubber chicken pecks at your leg.",ch,NULL,victim,TO_CHAR);

       if (ch->hit > 15000)
		  hpch = UMAX(10,15000);
       else
		  hpch = UMAX(10,ch->hit);
       hp_dam = number_range(hpch/9+1,hpch/5);

        dice_dam = dice(level, 1);

       dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
       dam *= 1/2;
       dam_type = DAM_OTHER;
       break;
      }

       if ( ( ch->fighting == NULL )
       && ( !IS_NPC( ch ) )
       && ( !IS_NPC( victim ) ) )
       {
  	  ch->attacker = TRUE;
	  victim->attacker = FALSE;
       }

       if (dam_type == DAM_BASH)
       {
          fire_effect(victim,level,dam,TARGET_CHAR);
          damage_old(ch,victim,dam,sn,dam_type,TRUE);
       }
       else if (dam_type == DAM_OTHER)
       {
          damage_old(ch,victim,dam/2,sn,dam_type,TRUE); 
       }
       else 
       {
          damage_old(ch,victim,dam,sn,dam_type,TRUE); 
       }
    }
    else
    {
      send_to_char( "You raise your hands and begin to chant with no effect, perhaps you are missing something.\n\r", ch );
      act( "$n raises both hands and begins to chant with no visible effect.", ch, NULL, NULL, TO_ROOM );

      return;
    }
}

/*
 * Block of Sage and 3rd tier Sage class spells
 */
void spell_channel( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;

    if (is_affected(ch,gsn_channel) )
    {
        send_to_char("You are already channeling spells.\n\r",ch);
	return;
    }

    af.where 	= TO_AFFECTS;
    af.type     = gsn_channel;
    af.level 	= level;
    af.duration = 3 + level/5;
    af.modifier = -level/12;
    af.location = APPLY_SAVES;
    af.bitvector = 0;
    affect_to_char(ch, &af);
    send_to_char("A shimmering field of energy surrounds you.\n\r",ch);
    act("A blue force field sizzles into existence around $n.",ch,NULL,NULL,TO_ROOM);
    return;
}

void spell_wither(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        int dam, chance;
        AFFECT_DATA af;

        chance = ch->pcdata->learned[sn] + (ch->level - victim->level)*3;
        chance += ch->magic[MAGIC_BLACK];
        chance -= victim->magic[MAGIC_BLACK];
        
         if (check_sorcery(ch,sn))
        	dam = sorcery_dam(level,15,ch);
         else
         	dam = dice(level,15);

        if (is_affected(victim,sn))
                chance = 0;
        chance = URANGE(5,chance,90);
        if ((number_percent() > chance)
        || saves_spell(level,victim,DAM_NEGATIVE)
        || is_affected(victim,sn))
        {
        	send_to_char("You feel an intense pain in your body.\n\r",victim);
        	act("$n jerks in sudden pain.",victim,0,0,TO_ROOM);
        	if (saves_spell(level,victim,DAM_HARM) )
			{
        		dam *= 3;
				dam /= 4;
			}
        	damage_old(ch,victim,dam,sn,DAM_HARM,TRUE);
        	return;
        }

        af.where = TO_AFFECTS;
        af.level = level;
        af.duration = level/7;
        af.type = sn;
        af.bitvector = 0;

        switch(number_range(0,9))
        {
        case (0):
        case (1):
        case (2):
        case (3): /* arms */
        send_to_char("You feel a sudden intense pain as your arms wither!\n\r",victim);
        act("$n screams in agony as $s arms seem to shrivel up!",victim,0,0,TO_ROOM);
        af.location = APPLY_STR;
        af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_HITROLL;
        af.modifier = -8;
        affect_to_char(victim,&af);
        af.location = APPLY_DAMROLL;
        af.modifier = -10;
        affect_to_char(victim,&af);
        break;
        case (4):
        case (5):
        case (6):
        case (7): /* legs */
        send_to_char("You feel a sudden intense pain as your legs wither!\n\r",victim);
        act("$n screams in agony as $s legs crumple beneath $m!",victim,0,0,TO_ROOM);
        af.location = APPLY_STR;
        af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_DEX;
        af.modifier = -7;
        affect_to_char(victim,&af);
        af.location = APPLY_HITROLL;
        af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_DAMROLL;
        af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_MOVE;
        af.modifier = -3*level;
        affect_to_char(victim,&af);
        break;
        case(8): /* body */
        act("$n's body suddenly seems to crumple up and wither!",victim,0,0,TO_ROOM);
        send_to_char("You feel a sudden intense pain as your body gives out and withers up!\n\r",victim);
        af.location = APPLY_STR;
        af.modifier = -8;
        affect_to_char(victim,&af);
        af.location = APPLY_DEX;
        af.modifier = -5;
		affect_to_char(victim,&af);
		af.location = APPLY_CON;
		af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_HITROLL;
        af.modifier = -6;
        affect_to_char(victim,&af);
        af.location = APPLY_DAMROLL;
        af.modifier = -10;
		af.bitvector = AFF_WEAKEN;
        affect_to_char(victim,&af);
        dam *= 3;
        break;
        case (9): /* head */
        send_to_char("Your head ruptures and then shrivels as it undergoes a sudden withering!\n\r",victim);
        act("$n's skull seems to just wither and shrivel up!",victim,0,0,TO_ROOM);
        dam *= 4;
        af.location = APPLY_STR;
        af.modifier = -8;
        affect_to_char(victim,&af);
        af.location = APPLY_DEX;
        af.modifier = -5;
        affect_to_char(victim,&af);
        af.location = APPLY_HITROLL;
        af.modifier = -6;
        affect_to_char(victim,&af);
        af.location = APPLY_DAMROLL;
        af.modifier = -10;
        af.bitvector = AFF_BLIND;
        affect_to_char(victim,&af);
        send_to_char("Your eyes are desicated...you are blinded!\n\r",victim);
        break;
        }

        damage_old(ch,victim,dam,sn,DAM_HARM,TRUE);
        return;
}

void spell_soulbind(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    CHAR_DATA *check;

    if (!IS_NPC(victim))
    {
        send_to_char("You can't bind them to your soul.\n\r",ch);
        return;
    }

    if (victim->master != ch)
    {
        send_to_char("You can't bind them to your soul.\n\r",ch);
        return;
    }

    if (IS_SET(victim->imm_flags,IMM_CHARM)) {
	    send_to_char("You can't bind them to your soul.\n\r",ch);
	    return; }

    if (is_affected(victim,sn))
    {
        send_to_char("That zombie is already soulbound.\n\r",ch);
        return;
    }
    for (check = char_list; check != NULL; check = check->next)
    {
        if (!IS_NPC(check))
            continue;

        if (check->master == ch && is_affected(check,sn))
        {
            send_to_char("You already have a zombie bound to your soul.\n\r",ch);
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
    act("$N's eyes burn bright red as it's energy is bound to your soul.",ch,0,victim,TO_CHAR);
    act("$N's eyes burn bright red for a moment.",ch,0,victim,TO_NOTVICT);
    return;
}

/* 
 * Makes an aggressive Mobile. Also Makes it subordinate to caster.
 */ 
void spell_domineer(int sn, int level, CHAR_DATA * ch, void * vo, int target) 
{ 
        CHAR_DATA * victim; 
        AFFECT_DATA af; 
        char arg[MAX_INPUT_LENGTH]; 
         
        target_name = one_argument(target_name, arg); 
        if(arg[0] == '\0') 
        { 
                send_to_char("Cast the spell on whom?\n\r", ch); 
                return; 
        } 
        if((victim = get_char_room(ch, arg)) == NULL) 
        { 
                send_to_char("They aren't here.\n\r", ch); 
                return; 
        } 
        if(!IS_NPC(victim) || IS_SET(victim->imm_flags,IMM_CHARM)) 
        { 
                send_to_char("They cannot be affected by your spell.\n\r", ch); 
                return; 
        } 
 
        if(saves_spell(level, victim, DAM_MENTAL)) 
        {
                char buf[100]; 
                act("You invade $N's mind, but $E is able to fight you off.", ch, 0, victim, TO_CHAR); 
                act("$n invades your mind, but you fight $m off.", ch, 0, victim, TO_VICT); 
                sprintf(buf, "How dare you try to influence my mind %s?! Prepare to die!", PERS(ch, victim)); 
                do_yell(victim, buf); 
                multi_hit(victim, ch, TYPE_UNDEFINED); 
                return; 
        } 
		
		/* This is to stop people from being able to control mobs that are way too high */
		if(IS_NPC(victim) && victim->level > ch->level + 45)
		{
			    char buf[100]; 
                act("You invade $N's mind, but $E is able to fight you off.", ch, 0, victim, TO_CHAR); 
                act("$n invades your mind, but you fight $m off.", ch, 0, victim, TO_VICT); 
                sprintf(buf, "How dare you try to influence my mind %s?! Prepare to die!", PERS(ch, victim)); 
                do_yell(victim, buf); 
                multi_hit(victim, ch, TYPE_UNDEFINED); 
                return; 
		}

        af.type = sn; 
        af.level = level; 
        af.where = TO_AFFECTS; 
        af.location = 0; 
        af.duration = 15; 
        af.modifier = 0; 
        af.bitvector = 0; 
        affect_to_char(victim, &af); 
 
        act("You invade $N's mind and make them more ill-tempered.", ch, 0, victim, TO_CHAR); 
        act("You suddenly feel more hateful of everyone.", ch, 0, victim, TO_VICT); 
        act("$N looks around and scowls.", ch, 0, victim, TO_NOTVICT); 
        SET_BIT(victim->act, ACT_AGGRESSIVE);
        victim->master = ch;
        return; 
} 

/*
 * Give some people a headache, can't cast spells right.
 */ 
void spell_headache(int sn, int level, CHAR_DATA * ch, void * vo, int target) 
{ 
        CHAR_DATA * victim; 
        AFFECT_DATA af; 
        int counter, saves = 0; 
 
        victim = (CHAR_DATA *)vo; 
 
        if(is_affected(victim, sn)) 
        { 
                send_to_char("They already have a headache.\n\r", ch); 
                return; 
        }

        for(counter = 0;counter < 2;counter++) 
                if(saves_spell(level, victim, DAM_MENTAL)) 
                        saves++; 
 
        if(saves) 
        { 
          act("You invade $N's mind, but $E is able to fight you off.", ch, 0, victim, TO_CHAR); 
          act("$n invades your mind, but you are able to fight $m off.", ch, 0, victim, TO_VICT); 
           return; 
        } 
        act("You invade $N's mind and cause it to throb with pain.", ch, 0, victim, TO_CHAR); 
        act("$n invades your mind and you suddenly feel a blinding pain in your head.", ch, 0, victim, TO_VICT); 
        act("$N staggers, and brings his hand up to massage his temples.", ch, 0, victim, TO_NOTVICT); 
         
        af.type = sn; 
        af.level = level; 
        af.where = TO_AFFECTS; 
        af.location = APPLY_HITROLL; 
        af.modifier = - level/17 - 2; 
        af.duration = level/17 + 2; 
        af.bitvector = 0; 
        affect_to_char(victim, &af); 
         
        return; 
} 

void spell_dimension_walk(int sn, int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim;
    CHAR_DATA *group;
    CHAR_DATA *arrival;
    CHAR_DATA *g_next;
    CHAR_DATA *last_to_venue;
    int numb;
    bool gate_pet;

    if (ch->fighting != NULL) 
    {
      send_to_char("You can't concentrate enough.\n\r",ch);
      return;
    } 
    
    if(ch->fight_timer > 0)
    {
        send_to_char("In some kind of hurry?\n\r",ch);
        return;
    } 

    last_to_venue = ch;
    victim = get_char_world( ch, target_name );

    if (!(victim==NULL)) 
    {
    if ((IS_NPC(victim)) && ( IS_SET(victim->res_flags,RES_SUMMON) ))
	level -= 5;
    if ((IS_NPC(victim)) && ( IS_SET(victim->vuln_flags,VULN_SUMMON) ))
        level += 5;
    }

    if ( victim == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch,victim))
    ||   clan_table[victim->clan].independent)))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   IS_SET(victim->in_room->area->area_flags, AREA_RESTRICTED)
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
        send_to_char( "You failed to bring yourself to them.\n\r", ch );
        return;
    }	
    if (victim->in_room->clan != 0
        && victim->in_room->clan != victim->clan)
    {
	send_to_char("You failed.\n\r",victim);
	return;
    }
    if ( strstr( victim->in_room->area->builders, "Unlinked" ) )
    {
	send_to_char( "You can't gate to areas that aren't linked!\n\r",ch );
	return;
    }
    if (victim->in_room->area->continent != ch->in_room->area->continent) {
	    send_to_char( "You begin to travel but the destination is too far.\n\r",ch);
	    return; }

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;

    numb = 1;
    for (group = ch->in_room->people; group != NULL; group = g_next)
    {
	g_next = group->next_in_room;
	if( group->fighting != NULL )
		continue;
	if (group == ch )
		continue;
        if( !IS_NPC(group) 
         ||  group->master == NULL
         ||  group->master != ch)
                continue;

	numb++;

        if(IS_SET(group->affected_by,AFF_HIDE))
         REMOVE_BIT(group->affected_by,AFF_HIDE);

        send_to_char("You focus your mind, and step through the barrier of space.\n\r", ch); 
        act("$n concentrates, begins walking, and gradually fades away!", ch, 0, 0, TO_ROOM); 
	char_from_room(group);
	char_to_room(group,victim->in_room);
        act("Suddenly, $n seems to step out of thin air!", ch, 0, 0, TO_ROOM); 
	last_to_venue = group;
   }

   if (gate_pet)
   {
	group = ch->pet;

        if(IS_SET(group->affected_by,AFF_HIDE))
         REMOVE_BIT(group->affected_by,AFF_HIDE);

        send_to_char("You focus your mind, and step through the barrier of space.\n\r", ch); 
        act("$n concentrates, begins walking, and gradually fades away!", ch, 0, 0, TO_ROOM); 
	char_from_room(group);
	char_to_room(group,victim->in_room);
        act("Suddenly, $n seems to step out of thin air!", ch, 0, 0, TO_ROOM); 
	last_to_venue = group;
        numb++;
   }

   if(IS_SET(ch->affected_by,AFF_HIDE))
    REMOVE_BIT(ch->affected_by,AFF_HIDE);

   send_to_char("You focus your mind, and step through the barrier of space.\n\r", ch);
   act("$n concentrates, begins walking, and gradually fades away!", ch, 0, 0, TO_ROOM); 
   char_from_room(ch);
   char_to_room(ch,victim->in_room);
   act("Suddenly, $n seems to step out of thin air!", ch, 0, 0, TO_ROOM); 

   if (last_to_venue == ch)
    {
	do_look(ch,"auto");
	return;
    }


   for (arrival = ch->in_room->people; arrival != NULL; arrival = arrival->next_in_room)
    {
	do_look(arrival,"auto");
	if (--numb == 0)
	break;
    }

  return;
}

void spell_earthmeld(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    AFFECT_DATA af;
    if (is_affected(ch,sn))
    {
    send_to_char("You are already melded with the ground.\n\r",ch);
    return;
    }
    if (ch->in_room->sector_type == SECT_WATER_SWIM
    || ch->in_room->sector_type == SECT_WATER_NOSWIM
    || ch->in_room->sector_type == SECT_AIR)
    {
    send_to_char("There isn't enough natural earth to meld with here.\n\r",ch);
    return;
    }

    act("$n vanishes into the ground!",ch,0,0,TO_ROOM);
    send_to_char("You vanish into the ground!\n\r",ch);

    af.where = TO_AFFECTS;
    af.type = gsn_earthmeld;
    af.level = level;
    af.modifier = 0;
    af.bitvector = 0;
    af.duration = (level/5);
    af.location = 0;
    affect_to_char(ch,&af);

    return;
}
//Forskane Spell - Torrents of Flame by Fesdor
void spell_torrents_flame(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch,hitnum;

    act("$n unleashes a torrent of flame at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n raises their hands and torrents of flame burst forth!",ch,NULL,victim,TO_VICT);
    act("You unleash torrents of flame at $N.",ch,NULL,victim,TO_CHAR);
  
  for ( hitnum = 0; hitnum < ((dice(1,6)+1)/2); hitnum++ ) 
  {
    hpch = number_range(3800,ch->hit);
    hp_dam = number_range(hpch/10,hpch/5);
    
    	if (check_sorcery(ch,sn))
        	dice_dam = sorcery_dam(4.8,22,ch);
    	else
		dice_dam = dice(4,20);

    dam = UMAX(hp_dam + dice_dam/15,dice_dam + hp_dam/14);

    dam = dice(ch->level/15,(dam*2.5)/2);
    
    dam = number_range(dam*2/3,dam*4/3);

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if ( saves_spell(level,victim,DAM_FIRE) )
    {
	fire_effect(victim,level/2,dam/5,TARGET_CHAR);
	damage_old(ch,victim,dam/3,sn,DAM_FIRE,TRUE);
    }
    else
    {
	fire_effect(victim,level+3,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_FIRE,TRUE); 
    }
  }
}

void spell_barrier(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    AFFECT_DATA af;
    if (is_affected(ch,sn))
    {
    send_to_char("You are already surrounded by a barrier.\n\r",ch);
    return;
    }

    act("$n is surrounded by a shimmering barrier",ch,0,0,TO_ROOM);
    send_to_char("A shimmering barrier materializes around you!\n\r",ch);

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.modifier = -33;
    af.bitvector = 0;
    af.duration = 20;
    af.location = APPLY_AC;
    affect_to_char(ch,&af);

    return;
}

void spell_conjure_blizzard(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam, count=0;

    /* first strike */

    act("$n murmers arcane words as a blizzard rages about $N!",
        ch,NULL,victim,TO_ROOM);
    act("You murmer arcane words as a blizzard rages about $N!",
	ch,NULL,victim,TO_CHAR);
    act("$n conjures an icy blizzard all around you!",
	ch,NULL,victim,TO_VICT);  

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    
    if (check_sorcery(ch,sn))
        dam = sorcery_dam(level,30,ch);
    else        
    	dam = dice(level,24);

    if (saves_spell(level,victim,DAM_COLD))
 	dam /= 4;
 	
    damage_old(ch,victim,dam,sn,DAM_COLD,TRUE);
    cold_effect(victim,level,dam,TARGET_CHAR);
    last_vict = victim;
    level -= 10;   /* decrement damage */
    count++;

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
          if (!is_same_group(ch,tmp_vict) && tmp_vict != last_vict && (!IS_IMMORTAL(tmp_vict) || can_see(ch,tmp_vict)))
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    if (last_vict == ch)
	    {
             if (number_percent() >= level/3)
	      {
	       act("The blizzard rages on!",NULL,NULL,NULL,TO_ROOM);
	       act("The blizzard rages on!",NULL,NULL,NULL,TO_ROOM);
	       level+=20;
	      }
	     else
	      {
	       act("$n diverts the blizzard away from them!",tmp_vict,NULL,NULL,TO_ROOM);
	       act("The blizzard surrounds you briefly before you divert it away!",NULL,NULL,NULL,TO_ROOM);
	      }
	    }
	    else
	    {
             if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
              act("The blizzard surrounds $n!",tmp_vict,NULL,NULL,TO_ROOM);
             if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
              act("The freezing blizzard bites at your flesh!",NULL,NULL,NULL,TO_CHAR);
        dam = dice(level,27);
	    if (saves_spell(level,tmp_vict,DAM_COLD))
		dam /= 2;
	    damage_old(ch,tmp_vict,dam,sn,DAM_COLD,TRUE);
	    cold_effect(tmp_vict,level,dam,TARGET_CHAR);
	    level -= 10;  /* decrement damage */
            count++;
	    }
	  }
        }

	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	  {
            act("The raging blizzard slowly dies down.",NULL,NULL,NULL,TO_ROOM);
            act("You lose control of the blizzard as it dies away.",NULL,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
          if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
	  act("The blizzard surrounds $n then fades away.",ch,NULL,NULL,TO_ROOM);
          if(!IS_SET(ch->act, PLR_SHORT_COMBAT))
          send_to_char("The blizzard surrounds you then fades away.\n\r",ch);
	  level -= 10;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}

void spell_mana_blast (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int dam_type;
    int marker = 0;
    //Log-ish scale for damage based on mana %
    dam = dice( level*5, (ch->mana*10/ch->max_mana)*8/5 );

    if (victim != ch)
    {
        act("$n raises their hand, releasing a blast of pure {Ce{Dn{Ce{cr{Wg{cy{x!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char("You channel your {Ce{Dn{Ce{cr{Wg{cy{x and try to expose their weakness!\n\r",ch);
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    //If you add a damtype to the game, increase the loop max by 1 so your new one is checked.
    for (dam_type=0;dam_type <= 17;dam_type++)
    {
    	switch(check_immune(victim,dam_type))
    	{
			case IS_IMMUNE:
			case IS_RESISTANT:
			case IS_VULNERABLE:	
			{
				marker = 1;
				break;
			}
		}
		if (marker)
		{	//Hit 'em in the weak spot and stop checking.
			dam *= 2.5;
			break;
		}
	}
	if (saves_spell(level-10,victim,DAM_OTHER))
	{
		dam /= 3;
	}
	
    damage_old( ch, victim, dam, sn, DAM_NONE ,TRUE);
}

/*
 *Transmute damnoun-changing spell for Alchemist. By Fesdor
 *Since v3 is type int, dt must be type int and thus the player must input an integer
 *If you figure out how to take strings from the player and convert them to ints using attack_table, do it.
 *Until then, include table of integer values for damtypes in help transmute
 */
void spell_transmute ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	OBJ_DATA *weapon;
	OBJ_DATA *held;
	weapon=(OBJ_DATA *) vo;
	held = get_eq_char(ch,WEAR_HOLD);
	int dt = atoi(third_name);//Prevent anything but integers moving forward.  Any string will become 0.
	
	//For Immortal Debugging Purposes
	if(ch->level > 101)
	{
	char buf[MAX_STRING_LENGTH];
    sprintf(buf,"Damtype is %d\n\r",dt);
    send_to_char(buf,ch);
	}
	
    if (held == NULL || (str_cmp(held->name,"philosopher stone")))//Be careful making any other items with these keywords (aka don't do it).
    {
        send_to_char("You need {YT{Ch{Ye Ph{Ci{Yl{Co{cs{yop{Yh{Ce{Yr's S{yt{Co{yn{Ye{x to transmute a weapon.\n\r",ch);
        return;
    }
    //If the attack table changes in const.c, change the upper number for this check.
    if (0<dt && dt<=46)
    {
		if(weapon->item_type != ITEM_WEAPON)
		{
			send_to_char("You can only target weapons.\n\r",ch);
			return ;
		}
		else
		{
			char buf2[MAX_STRING_LENGTH/4];
			sprintf(buf2,"You focus your power and change %s's damtype to %s.\n\r",weapon->short_descr,attack_table[dt].noun);
			send_to_char(buf2,ch);
			weapon->value[3] = dt;
	    	extract_obj(held);
	    	return;
		}
	}
	else
	{
		send_to_char("Invalid damtype number, please see {C'{Whelp transmute{C'{x.\n\r",ch);
		send_to_char("Syntax: cast transmute <weapon> <damtype number>\n\r",ch);
		return ;
	}
	return;
}
