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
#include <sys/time.h>
#endif
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "clan.h"

long obj_points( OBJ_INDEX_DATA * obj );
long obj_balance( OBJ_INDEX_DATA * obj ); 

/* command procedures needed */
DECLARE_DO_FUN (do_exits);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_affects);
DECLARE_DO_FUN (do_play);
DECLARE_DO_FUN (do_inventory);
DECLARE_DO_FUN (do_mstat);
DECLARE_DO_FUN (do_sense);

int focus_dam args ((CHAR_DATA *ch));
int focus_ac args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch)); 
char * total_length args ((char *input, int length));
char * get_descr_form args((CHAR_DATA *ch,CHAR_DATA *looker, bool get_long));

char *const where_name[] =
{
	"{G<{Cused as light{G>{x     ",
	"{G<{Cworn on finger{G>{x    ",
	"{G<{Cworn on finger{G>{x    ",
	"{G<{Cworn around neck{G>{x  ",
	"{G<{Cworn around neck{G>{x  ",
	"{G<{Cworn on torso{G>{x     ",
	"{G<{Cworn on head{G>{x      ",
	"{G<{Cworn on legs{G>{x      ",
	"{G<{Cworn on feet{G>{x      ",
	"{G<{Cworn on hands{G>{x     ",
	"{G<{Cworn on arms{G>{x      ",
	"{G<{Cworn as shield{G>{x    ",
	"{G<{Cworn about body{G>{x   ",
	"{G<{Cworn about waist{G>{x  ",
	"{G<{Cworn around wrist{G>{x ",
	"{G<{Cworn around wrist{G>{x ",
	"{G<{Cprimary wield{G>{x     ",
	"{G<{Cheld{G>{x              ",
	"{G<{Cfloating nearby{G>{x   ",
	"{G<{Csecondary wield{G>{x   ",
	"{G<{Cworn on face{G>{x      ",
	"{G<{Cworn on ears{G>{x      ",
        "{G<{Cworn on {!bug{G>{x       ",
	"{G<{Cworn on ankle{G>{x     ",
	"{G<{Cworn on ankle{G>{x     ",
	"{G<{Cclan patch{G>{x        ",
	"{G<{Creligion patch{G>{x    ",
	"{G<{Cchest patch{G>{x       ",
	"{G<{Cworn on back{G>{x      "
};

sh_int const where_order[] =
{
	1, 2, 3, 4, 5,
	6, 20, 7, 23, 24,
	8, 9, 10, 11, 12,
	13, 14, 15, 16, 19,
	17, 18, 21, 22, 28,
	0, 25, 26, 27
};


/* for do_count */
int max_on = 0;
bool is_pm = FALSE;


/*
 * Local functions.
 */
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch,
								bool fShort));
BUFFER * show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch,
								 bool fShort, bool fShowNothing));
void show_char_to_char_0 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args ((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args ((CHAR_DATA * ch));



char *
format_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';

	if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
		|| (obj->description == NULL || obj->description[0] == '\0'))
		return buf;

	if (!IS_SET (ch->comm, COMM_LONG))
	{
		strcat (buf, "{x[{y.{R.{B.{M.{Y.{W.{G.{x]");
		if (IS_OBJ_STAT (obj, ITEM_INVIS))
			buf[5] = 'V';
		if (IS_AFFECTED (ch, AFF_DETECT_EVIL)
			&& IS_OBJ_STAT (obj, ITEM_EVIL))
			buf[8] = 'E';
		if (IS_AFFECTED (ch, AFF_DETECT_GOOD)
			&& IS_OBJ_STAT (obj, ITEM_BLESS))
			buf[11] = 'B';
		if (IS_AFFECTED (ch, AFF_DETECT_MAGIC)
			&& IS_OBJ_STAT (obj, ITEM_MAGIC))
			buf[14] = 'M';
		if (IS_OBJ_STAT (obj, ITEM_GLOW))
			buf[17] = 'G';
		if (IS_OBJ_STAT (obj, ITEM_HUM))
			buf[20] = 'H';
		if (IS_OBJ_STAT (obj, ITEM_QUEST))
			buf[23] = 'Q';
		if (!strcmp (buf, "{x[{y.{R.{B.{M.{Y.{W.{G.{x]"))
			buf[0] = '\0';
	}
	else
	{
		if (IS_OBJ_STAT (obj, ITEM_INVIS))
			strcat (buf, "({yInvis{x)");
		if (IS_OBJ_STAT (obj, ITEM_DARK))
			strcat (buf, "({DHidden{x)");
		if (IS_AFFECTED (ch, AFF_DETECT_EVIL)
			&& IS_OBJ_STAT (obj, ITEM_EVIL))
			strcat (buf, "({RRed Aura{x)");
		if (IS_AFFECTED (ch, AFF_DETECT_GOOD)
			&& IS_OBJ_STAT (obj, ITEM_BLESS))
			strcat (buf, "({BBlue Aura{x)");
		if (IS_AFFECTED (ch, AFF_DETECT_MAGIC)
			&& IS_OBJ_STAT (obj, ITEM_MAGIC))
			strcat (buf, "({yMagical{x)");
		if (IS_OBJ_STAT (obj, ITEM_GLOW))
			strcat (buf, "({YGlowing{x)");
		if (IS_OBJ_STAT (obj, ITEM_HUM))
			strcat (buf, "({yHumming{x)");
		if (IS_OBJ_STAT (obj, ITEM_QUEST))
			strcat (buf, "({GQuest{x)");
                if (obj->owner != NULL && ch->name != NULL && !str_cmp(obj->owner, ch->name))
                        strcat (buf, "({#Emblazoned{x)");
	}

	if (buf[0] != '\0')
	{
		strcat (buf, " ");
	}

	if (fShort)
	{
		if (obj->short_descr != NULL)
			strcat (buf, obj->short_descr);
	}
	else
	{
		if (obj->description != NULL)
			strcat (buf, obj->description);
	}
	if (strlen (buf) <= 0)
		strcat (buf, "This object has no description. Please inform Tas.");

	return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
BUFFER *show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count = 0;
    bool fCombine;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( ++count >= 8500 )
	{
	    add_buf( final, "{RToo many objects found.{x\n\r" );
	    return final;
	}
    }

    prgpstrShow = alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int   ) );
    nShow	= 0;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET( ch->comm, COMM_COMBINE ) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET( ch->comm, COMM_COMBINE ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(final,buf);
	    }
	    else
	    {
		add_buf(final,"     ");
	    }
	}
	add_buf(final,prgpstrShow[iShow]);
	add_buf(final,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET( ch->comm, COMM_COMBINE ) )
	    add_buf( final, "     " );
	add_buf(final,"{WNothing.{x\n\r");
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return final;
}

void 
show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

	buf[0] = '\0';

	if (!IS_SET (ch->comm, COMM_LONG))
	{
		strcat (buf, "{x[{y.{D.{c.{b.{w.{C.{r.{B.{D.{R.{Y.{W.{G.{x]");
		if (IS_SHIELDED (victim, SHD_INVISIBLE))
			buf[5] = 'V';
		if (IS_AFFECTED (victim, AFF_HIDE))
			buf[8] = 'H';
		if (IS_AFFECTED (victim, AFF_CHARM))
			buf[11] = 'C';
		if (IS_AFFECTED (victim, AFF_PASS_DOOR))
			buf[14] = 'T';
		if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
			buf[17] = 'P';
		if (IS_SHIELDED (victim, SHD_ICE))
			buf[20] = 'I';
		if (IS_SHIELDED (victim, SHD_FIRE))
			buf[23] = 'F';
		if (IS_SHIELDED (victim, SHD_SHOCK))
			buf[26] = 'L';
		if (IS_SHIELDED (victim, SHD_DEATH))
			buf[29] = 'D';
		if (IS_EVIL (victim)
			&& IS_AFFECTED (ch, AFF_DETECT_EVIL))
			buf[32] = 'E';
		if (IS_GOOD (victim)
			&& IS_AFFECTED (ch, AFF_DETECT_GOOD))
			buf[35] = 'G';
		if (IS_SHIELDED (victim, SHD_SANCTUARY))
			buf[38] = 'S';
		if (victim->on_quest)
		{
			if (!IS_NPC (victim) || IS_IMMORTAL (ch))
				buf[41] = 'Q';
		}
                if (!str_cmp(buf, "{x[{y.{D.{c.{b.{w.{C.{r.{B.{D.{R.{Y.{W.{G.{x]"))
			buf[0] = '\0';
                if (IS_SET (victim->plyr, PLAYER_GHOST))
                        strcat (buf, "[{7G{wH{7O{wS{7T{x]");
		if (IS_SET (victim->comm, COMM_AFK))
			strcat (buf, "[{yAFK{x]");
		if (victim->invis_level >= LEVEL_HERO)
			strcat (buf, "({WWizi{x)");
	}
	else
	{
                if (IS_SET (victim->plyr, PLAYER_GHOST))
                        strcat (buf, "[{7G{wH{7O{wS{7T{x]");
		if (IS_SET (victim->comm, COMM_AFK))
			strcat (buf, "[{yAFK{x]");
		if (IS_SHIELDED (victim, SHD_INVISIBLE))
			strcat (buf, "({yInvis{x)");
		if (victim->invis_level >= LEVEL_HERO)
			strcat (buf, "({WWizi{x)");
		if (IS_AFFECTED (victim, AFF_HIDE))
			strcat (buf, "({DHide{x)");
		if (IS_AFFECTED (victim, AFF_CHARM))
			strcat (buf, "({cCharmed{x)");
		if (IS_AFFECTED (victim, AFF_PASS_DOOR))
			strcat (buf, "({bTranslucent{x)");
		if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
			strcat (buf, "({wPink Aura{x)");
		if (IS_SHIELDED (victim, SHD_ICE))
			strcat (buf, "({DGrey Aura{x)");
		if (IS_SHIELDED (victim, SHD_FIRE))
			strcat (buf, "({rOrange Aura{x)");
		if (IS_SHIELDED (victim, SHD_SHOCK))
			strcat (buf, "({BBlue Aura{x)");
		if (IS_SHIELDED (victim, SHD_DEATH))
			strcat (buf, "({DBlack Aura{x)");
		if (IS_EVIL (victim)
			&& IS_AFFECTED (ch, AFF_DETECT_EVIL))
			strcat (buf, "({RRed Aura{x)");
		if (IS_GOOD (victim)
			&& IS_AFFECTED (ch, AFF_DETECT_GOOD))
			strcat (buf, "({YGolden Aura{x)");
		if (IS_SHIELDED (victim, SHD_SANCTUARY))
			strcat (buf, "({WWhite Aura{x)");
		if (victim->on_quest)
		{
			if (!IS_NPC (victim) || IS_IMMORTAL (ch))
				strcat (buf, "({GQuest{x)");
		}
	}
	if (!IS_NPC (victim) && IS_SET (victim->act, PLR_TWIT))
		strcat (buf, "({rTWIT{x)");
	if (IS_NPC (victim) && ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
		strcat (buf, "{R[TARGET]{x ");

        if (( victim->long_descr[0] != '\0' ) || (victim->morph_form[0] != 0) )
        {
         strcat( buf, get_descr_form(victim,ch,TRUE) );
         send_to_char( buf, ch );
         return;
        }

        if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
                && victim->position == POS_STANDING && ch->on == NULL
                && victim->morph_form[0] == 0)
        strcat(buf,victim->pcdata->pretit);

	if (buf[0] != '\0')
	{
		strcat (buf, " ");
	}

        strcat( buf, get_descr_form(victim,ch,FALSE) );

	if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
		&& victim->position == POS_STANDING && ch->on == NULL
                && victim->morph_form[0] == 0)
		strcat (buf, victim->pcdata->title);

	switch (victim->position)
	{
	case POS_DEAD:
		strcat (buf, " is DEAD!!");
		break;
	case POS_MORTAL:
		strcat (buf, " is mortally wounded.");
		break;
	case POS_INCAP:
		strcat (buf, " is incapacitated.");
		break;
	case POS_STUNNED:
		strcat (buf, " is lying here stunned.");
		break;
	case POS_SLEEPING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], SLEEP_AT))
			{
				sprintf (message, " is sleeping at %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else if (IS_SET (victim->on->value[2], SLEEP_ON))
			{
				sprintf (message, " is sleeping on %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else
			{
				sprintf (message, " is sleeping in %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
		}
		else
			strcat (buf, " is sleeping here.");
		break;
	case POS_RESTING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], REST_AT))
			{
				sprintf (message, " is resting at %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else if (IS_SET (victim->on->value[2], REST_ON))
			{
				sprintf (message, " is resting on %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else
			{
				sprintf (message, " is resting in %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
		}
		else
			strcat (buf, " is resting here.");
		break;
	case POS_SITTING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], SIT_AT))
			{
				sprintf (message, " is sitting at %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else if (IS_SET (victim->on->value[2], SIT_ON))
			{
				sprintf (message, " is sitting on %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else
			{
				sprintf (message, " is sitting in %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
		}
		else
			strcat (buf, " is sitting here.");
		break;
	case POS_STANDING:
		if (victim->on != NULL)
		{
			if (IS_SET (victim->on->value[2], STAND_AT))
			{
				sprintf (message, " is standing at %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else if (IS_SET (victim->on->value[2], STAND_ON))
			{
				sprintf (message, " is standing on %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
			else
			{
				sprintf (message, " is standing in %s.",
						 victim->on->short_descr);
				strcat (buf, message);
			}
		}
		else
			strcat (buf, " is here.");
		break;
	case POS_FIGHTING:
		strcat (buf, " is here, fighting ");
		if (victim->fighting == NULL)
			strcat (buf, "thin air??");
		else if (victim->fighting == ch)
			strcat (buf, "YOU!");
		else if (victim->in_room == victim->fighting->in_room)
		{
			strcat (buf, PERS (victim->fighting, ch));
			strcat (buf, ".");
		}
		else
			strcat (buf, "someone who left??");
		break;
	}

	strcat (buf, "\n\r");
	buf[0] = UPPER (buf[0]);
	send_to_char (buf, ch);
	return;
}



void 
show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int iWear;
	int oWear;
	int percent;
	bool found;
        CHAR_DATA *tch;

/*        if (can_see (victim, ch)
		&& get_trust (victim) >= ch->ghost_level)
	{
		if (ch == victim)
			act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
		else
		{
			act ("$n looks at you.", ch, NULL, victim, TO_VICT);
			act ("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
		}
        } */

	if (ch == victim)
        {
          for (tch = ch->in_room->people; tch != NULL; tch = tch->next_in_room)
          {
            if (can_see(tch,ch) && (ch != victim) && (tch != ch) && get_trust (victim) >= ch->ghost_level)
              act("$n looks at $mself.",ch,NULL,tch,TO_VICT);
          }
        }
	else
	{
          if (can_see(victim,ch) && get_trust (victim) >= ch->ghost_level)
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
          for (tch = ch->in_room->people; tch != NULL; tch = tch->next_in_room)
          {
            if (can_see(tch,ch) && (tch != victim) && get_trust (victim) >= ch->ghost_level)
            {
              sprintf(buf,"$n looks at %s.",get_descr_form(victim,tch,FALSE));
              act(buf,ch,NULL,tch,TO_VICT);
            }
          }
        }

    if(is_affected(victim, gsn_conceal))
    {
        send_to_char("The figure is buried deep within a dark green colored cloak.\n\r",ch);
    }
    else if (victim->description[0] != '\0')
    {
        sprintf (buf, "{C%s{x", victim->description);
        send_to_char(buf,ch);
    }
    else
    {
     act( "{CYou see nothing special about $M.{x", ch, NULL, victim, TO_CHAR );
    }

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	buf[0] = '\0';
        strcpy( buf, get_descr_form(victim,ch,FALSE) );

	if (percent >= 100)
                strcat (buf, " {(is in excellent condition.{x\n\r");
	else if (percent >= 90)
                strcat (buf, " {(has a few scratches.{x\n\r");
	else if (percent >= 75)
                strcat (buf, " {(has small wounds and bruises.{x\n\r");
	else if (percent >= 50)
                strcat (buf, " {(has quite a few wounds.{x\n\r");
	else if (percent >= 30)
                strcat (buf, " {(has some big nasty wounds and scratches.{x\n\r");
	else if (percent >= 15)
                strcat (buf, " {(is pretty hurt.{x\n\r");
	else if (percent >= 0)
                strcat (buf, " {(is in awful condition.{x\n\r");
	else
                strcat (buf, " {(is beyond dead...{x\n\r");

	buf[0] = UPPER (buf[0]);
	send_to_char(buf,ch);

	if (IS_SHIELDED (victim, SHD_ICE))
	{
		sprintf (buf, "%s is surrounded by an {Cicy{x shield.\n\r", PERS (victim, ch));
		buf[0] = UPPER (buf[0]);
		send_to_char(buf,ch);
	}
	if (IS_SHIELDED (victim, SHD_FIRE))
	{
		sprintf (buf, "%s is surrounded by a {Rfiery{x shield.\n\r", PERS (victim, ch));
		buf[0] = UPPER (buf[0]);
		send_to_char(buf,ch);
	}
	if (IS_SHIELDED (victim, SHD_SHOCK))
	{
		sprintf (buf, "%s is surrounded by a {Bcrackling{x shield.\n\r", PERS (victim, ch));
		buf[0] = UPPER (buf[0]);
		send_to_char(buf,ch);
	}
	if (IS_SHIELDED (victim, SHD_DEATH))
	{
		sprintf (buf, "%s is surrounded by a {Ddeath{x shield.\n\r", PERS (victim, ch));
		buf[0] = UPPER (buf[0]);
		send_to_char(buf,ch);
	}

      if(!is_affected(victim,gsn_conceal) || IS_IMMORTAL(ch)
      || (is_affected(victim,gsn_conceal) && is_same_clan(ch,victim)) )
      {
	found = FALSE;
	for (oWear = 0; oWear < MAX_WEAR; oWear++)
	{
		iWear = where_order[oWear];
		if ((obj = get_eq_char (victim, iWear)) != NULL
			&& can_see_obj (ch, obj))
		{
			if (!found)
			{
				send_to_char("\n\r",ch);
				sprintf (buf, "{G%s is using:{x\n\r", victim->name);
				send_to_char(buf,ch);
				found = TRUE;
			}
			sprintf (buf, "%s%s\n\r", where_name[iWear], format_obj_to_char (obj, ch, TRUE));
			send_to_char(buf,ch);
		}
	}
      }

	if (victim != ch
		&& !IS_NPC (ch)
		&& number_percent () < get_skill (ch, gsn_peek)
		&& IS_SET (ch->act, PLR_AUTOPEEK))
	{
		BUFFER *output = show_list_to_char( victim->carrying, ch, TRUE, TRUE );
		send_to_char("\n\r{GYou peek at the inventory:{x\n\r",ch);
		check_improve (ch, gsn_peek, TRUE, 4);
		page_to_char( output->string, ch );
		free_buf( output );
	}
	return;
}



void 
show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
	CHAR_DATA *rch;

	for (rch = list; rch != NULL; rch = rch->next_in_room)
	{
		if (rch == ch)
			continue;

		if (get_trust (ch) < rch->invis_level)
			continue;

		if ((!str_cmp(rch->long_descr, "{x\n\r")) && IS_NPC(rch))
			continue;

		if (get_trust (ch) < rch->ghost_level)
			continue;

		/* if(ch->long_descr == "")
			continue; */

		if (can_see (ch, rch))
		{
			show_char_to_char_0 (rch, ch);
		}
		else if (room_is_dark (ch->in_room)
				 && IS_AFFECTED (rch, AFF_INFRARED))
		{
			send_to_char ("You see {Rglowing red{x eyes watching YOU!\n\r", ch);
		}
	}

	return;
}

void 
do_peek (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument (argument, arg);

	if (IS_NPC (ch))
		return;

	if (arg[0] == '\0')
	{
		send_to_char ("Peek at who?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		do_inventory (ch, "");
		return;
	}

	if (can_see (victim, ch)
		&& get_trust (victim) >= ch->ghost_level)
	{
		act ("$n peers intently at you.", ch, NULL, victim, TO_VICT);
		act ("$n peers intently at $N.", ch, NULL, victim, TO_NOTVICT);
	}

	if (number_percent () < get_skill (ch, gsn_peek))
	{
		BUFFER *output = show_list_to_char( victim->carrying, ch, TRUE, TRUE );
		send_to_char("\n\r{GYou peek at the inventory:{x\n\r",ch);
		check_improve (ch, gsn_peek, TRUE, 4);
		page_to_char( output->string, ch );
		free_buf( output );
	}
	else
	{
		send_to_char("{RYou fail to see anything.{x\n\r",ch);
		check_improve (ch, gsn_peek, FALSE, 2);
	}
	return;
}

bool 
check_blind (CHAR_DATA * ch)
{

	if (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED (ch, AFF_BLIND))
	{
		send_to_char ("You can't see a thing!\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

/* changes your scroll */
void 
do_scroll (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[100];
	int lines;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		if (ch->lines == 0)
			send_to_char ("You do not page long messages.\n\r", ch);
		else
		{
			sprintf (buf, "You currently display %d lines per page.\n\r",
					 ch->lines + 2);
			send_to_char (buf, ch);
		}
		return;
	}

	if (!is_number (arg))
	{
		send_to_char ("You must provide a number.\n\r", ch);
		return;
	}

	lines = atoi (arg);

	if (lines == 0)
	{
		send_to_char ("Paging disabled.\n\r", ch);
		ch->lines = 0;
		return;
	}

	if (lines < 10 || lines > 100)
	{
		send_to_char ("You must provide a reasonable number.\n\r", ch);
		return;
	}

	sprintf (buf, "Scroll set to %d lines.\n\r", lines);
	send_to_char (buf, ch);
	ch->lines = lines - 2;
}

/* RT does socials */
void 
do_socials (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int iSocial;
	int col;

	col = 0;

	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
	{
		sprintf (buf, "%-12s", social_table[iSocial].name);
		send_to_char (buf, ch);
		if (++col % 6 == 0)
			send_to_char ("\n\r", ch);
	}

	if (col % 6 != 0)
		send_to_char ("\n\r", ch);
	return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */

void 
do_motd (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "motd");
}

void 
do_imotd (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "imotd");
}

void 
do_rules (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "rules");
}

void 
do_story (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "story");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("{! ____________________________________ \n\r",ch);
    send_to_char("{!|                                    {!|\n\r",ch);
    send_to_char("{!|    {@action       {@status             {!|\n\r",ch);
    send_to_char("{!|                                    {!|\n\r",ch); 

    send_to_char("{!|  {6autoassist   ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autoexit     ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autogold     ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autoloot     ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autosac      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autosplit    ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autopeek     ",ch);
    if (IS_SET(ch->act,PLR_AUTOPEEK))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6autostore    ",ch);
    if (IS_SET(ch->comm,COMM_STORE))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6compact mode ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6prompt       ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6combine items",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6long flags   ",ch);
    if (IS_SET(ch->comm,COMM_LONG))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
	send_to_char("    {!OFF              {!|{x\n\r",ch);

    send_to_char("{!|  {6noevolve     ", ch);
    if (IS_SET (ch->act,PLR_NOEVOLVE))
        send_to_char("    {@ON               {!|{x\n\r",ch);
    else
        send_to_char("    {!OFF              {!|{x\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("{!|  {$Your corpse is harder to loot.    {!|{x\n\r",ch);
    else 
        send_to_char("{!|  {$Your corpse is fully lootable     {!|\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("{!|  {$You cannot be summoned.           {!|\n\r",ch);
    else
	send_to_char("{!|  {$You can be summoned.              {!|\n\r",ch);            
      
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("{!|  {$You do not welcome followers.     {!|\n\r",ch);
    else
	send_to_char("{!|  {$You accept followers.             {!|\n\r",ch);

    if (IS_SET (ch->act, PLR_NOCANCEL))
	send_to_char("{!|  {$You are immune to cancel.         {!|\n\r", ch);
    else
	send_to_char("{!|  {$Cancel can be cast upon you.      {!|\n\r", ch);

    if (IS_SET (ch->act, PLR_NOTRAN))
	send_to_char("{!|  {$You are immune to transport.      {!|\n\r", ch);
    else
	send_to_char("{!|  {$You can be transported.           {!|\n\r", ch);

    if (IS_SET (ch->plyr, PLAYER_SUBDUE) )
        send_to_char("{!|  {$You subdue your victims.          {!|\n\r",ch);
    else
        send_to_char("{!|  {$You kill your victims.            {!|\n\r",ch);


    send_to_char("{!|                                    {!|\n\r",ch);
    send_to_char("{!|____________________________________|{x\n\r",ch);
}

void 
do_autoassist (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOASSIST))
	{
		send_to_char ("Autoassist removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOASSIST);
	}
	else
	{
		send_to_char ("You will now assist when needed.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOASSIST);
	}
}

void 
do_autoexit (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOEXIT))
	{
		send_to_char ("Exits will no longer be displayed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOEXIT);
	}
	else
	{
		send_to_char ("Exits will now be displayed.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOEXIT);
	}
}

void 
do_autogold (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOGOLD))
	{
		send_to_char ("Autogold removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOGOLD);
	}
	else
	{
		send_to_char ("Automatic gold looting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOGOLD);
	}
}

void 
do_autoloot (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOLOOT))
	{
		send_to_char ("Autolooting removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOLOOT);
	}
	else
	{
		send_to_char ("Automatic corpse looting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOLOOT);
	}
}

void 
do_autosac (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOSAC))
	{
		send_to_char ("Autosacrificing removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOSAC);
	}
	else
	{
		send_to_char ("Automatic corpse sacrificing set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOSAC);
	}
}

void 
do_autosplit (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOSPLIT))
	{
		send_to_char ("Autosplitting removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOSPLIT);
	}
	else
	{
		send_to_char ("Automatic gold splitting set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOSPLIT);
	}
}

void 
do_autopeek (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_AUTOPEEK))
	{
		send_to_char ("Autopeek removed.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_AUTOPEEK);
	}
	else
	{
		send_to_char ("Automatic peek set.\n\r", ch);
		SET_BIT (ch->act, PLR_AUTOPEEK);
	}
}

void do_brief (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_BRIEF))
	{
		send_to_char ("Full descriptions activated.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_BRIEF);
	}
	else
	{
		send_to_char ("Short descriptions activated.\n\r", ch);
		SET_BIT (ch->comm, COMM_BRIEF);
	}
}

void do_short (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->act, PLR_SHORT_COMBAT))
	{
		send_to_char ("Short combat disabled.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_SHORT_COMBAT);
	}
	else
	{
		send_to_char ("Short combat activated.\n\r", ch);
		SET_BIT (ch->act, PLR_SHORT_COMBAT);
	}
}

void 
do_compact (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_COMPACT))
	{
		send_to_char ("Compact mode removed.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_COMPACT);
	}
	else
	{
		send_to_char ("Compact mode set.\n\r", ch);
		SET_BIT (ch->comm, COMM_COMPACT);
	}
}

void 
do_long (CHAR_DATA * ch, char *argument)
{
	if (!IS_SET (ch->comm, COMM_LONG))
	{
		send_to_char ("Long flags activated.\n\r", ch);
		SET_BIT (ch->comm, COMM_LONG);
	}
	else
	{
		send_to_char ("Short flags activated.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_LONG);
	}
}

void 
do_show (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
	{
		send_to_char ("Affects will no longer be shown in score.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_SHOW_AFFECTS);
	}
	else
	{
		send_to_char ("Affects will now be shown in score.\n\r", ch);
		SET_BIT (ch->comm, COMM_SHOW_AFFECTS);
	}
}

void 
do_prompt (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_PROMPT))
		{
			send_to_char ("You will no longer see prompts.\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_PROMPT);
		}
		else
		{
			send_to_char ("You will now see prompts.\n\r", ch);
			SET_BIT (ch->comm, COMM_PROMPT);
		}
		return;
	}

	if (!strcmp (argument, "all"))
		strcpy (buf, "<%hhp %mm %vmv> ");
	else
	{
		if (strlen (argument) > 50)
			argument[50] = '\0';
		strcpy (buf, argument);
		smash_tilde (buf);
		if (str_suffix ("%c", buf))
			strcat (buf, "{x ");

	}

	free_string (ch->prompt);
	ch->prompt = str_dup (buf);
	sprintf (buf, "Prompt set to %s\n\r", ch->prompt);
	send_to_char (buf, ch);
	return;
}

void 
do_combine (CHAR_DATA * ch, char *argument)
{
	if (IS_SET (ch->comm, COMM_COMBINE))
	{
		send_to_char ("Long inventory selected.\n\r", ch);
		REMOVE_BIT (ch->comm, COMM_COMBINE);
	}
	else
	{
		send_to_char ("Combined inventory selected.\n\r", ch);
		SET_BIT (ch->comm, COMM_COMBINE);
	}
}

void 
do_noevolve (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_NOEVOLVE))
	{
		send_to_char ("You now have the possibility of evolving.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_NOEVOLVE);
	}
	else
	{
		send_to_char ("You will no longer evolve.\n\r", ch);
		SET_BIT (ch->act, PLR_NOEVOLVE);
	}
}

void 
do_nofalcon (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->plyr, PLAYER_NOFALCON))
	{
		send_to_char ("You now have the help of your falcon.\n\r", ch);
		REMOVE_BIT (ch->plyr, PLAYER_NOFALCON);
	}
	else
	{
		send_to_char ("You will no longer have the help of your falcon.\n\r", ch);
		SET_BIT (ch->plyr, PLAYER_NOFALCON);
	}
}
void do_noloot (CHAR_DATA * ch, char *argument)
{
 extern bool NOLOOT;
 CHAR_DATA *wch;

 if (IS_NPC (ch))
  return;

 if (ch->level >=109)
 {
	 if(NOLOOT)
	 {
	  send_to_char("Corpse Looting now allowed.\n\r",ch);
	  NOLOOT = FALSE;
          for(wch = char_list; wch != NULL; wch = wch->next)
	  {
	    send_to_char ("Global Loot Flag On: Your corpse may now be looted.\n\r", wch);
	  }
	 }
	 else if(!NOLOOT)
	 {
	  send_to_char("Corpse Looting now NOT allowed.\n\r",ch);
	  NOLOOT = TRUE;
	  for(wch = char_list; wch != NULL; wch = wch->next)
	  {
	    send_to_char ("Global Loot Flag Off: Your corpse is safe from thieves.\n\r", wch);
	  }
	 }
 }
 else
 {
	if (IS_SET (ch->act, PLR_CANLOOT))
	{
		send_to_char ("Your corpse is harder to loot.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_CANLOOT);
	}
	else
	{
		send_to_char ("Your corpse is fully lootable.\n\r", ch);
		SET_BIT (ch->act, PLR_CANLOOT);
	}

 }
 return;
}

void do_nofollow (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

        if(IS_SET(ch->affected_by,AFF_CHARM))
        {
         send_to_char("I don't think so.\n\r",ch);
         return;
        }

	if (IS_SET (ch->act, PLR_NOFOLLOW))
	{
		send_to_char ("You now accept followers.\n\r", ch);
		REMOVE_BIT (ch->act, PLR_NOFOLLOW);
	}
	else
	{
		send_to_char ("You no longer accept followers.\n\r", ch);
		SET_BIT (ch->act, PLR_NOFOLLOW);
		die_follower (ch);
	}
}

void 
do_nosummon (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		if (IS_SET (ch->imm_flags, IMM_SUMMON))
		{
			send_to_char ("You are no longer immune to summon.\n\r", ch);
			REMOVE_BIT (ch->imm_flags, IMM_SUMMON);
		}
		else
		{
			send_to_char ("You are now immune to summoning.\n\r", ch);
			SET_BIT (ch->imm_flags, IMM_SUMMON);
		}
	}
	else
	{
		if (IS_SET (ch->act, PLR_NOSUMMON))
		{
			send_to_char ("You are no longer immune to summon.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOSUMMON);
		}
		else
		{
			send_to_char ("You are now immune to summoning.\n\r", ch);
			SET_BIT (ch->act, PLR_NOSUMMON);
		}
	}
}

void 
do_notran (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		return;
	}
	else
	{
		if (IS_SET (ch->act, PLR_NOTRAN))
		{
			send_to_char ("You are no longer immune to transport.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOTRAN);
		}
		else
		{
			send_to_char ("You are now immune to transport.\n\r", ch);
			SET_BIT (ch->act, PLR_NOTRAN);
		}
	}
}

void 
do_nogoto (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		return;
	}
	else
	{
		if (IS_SET (ch->act, PLR_NOGOTO))
		{
			send_to_char ("You are no longer immune to goto.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOGOTO);
		}
		else
		{
			send_to_char ("You are now immune to goto.\n\r", ch);
			SET_BIT (ch->act, PLR_NOGOTO);
		}
	}
}

void 
do_noviolate (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		return;
	}
	else
	{
		if (IS_SET (ch->act, PLR_NOVIOLATE))
		{
			send_to_char ("You are no longer immune to violate.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOVIOLATE);
		}
		else
		{
			send_to_char ("You are now immune to violate.\n\r", ch);
			SET_BIT (ch->act, PLR_NOVIOLATE);
		}
	}
}



void 
do_nocancel (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
	{
		return;
	}
	else
	{
		if (IS_SET (ch->act, PLR_NOCANCEL))
		{
			send_to_char ("You are no longer immune to cancellation.\n\r", ch);
			REMOVE_BIT (ch->act, PLR_NOCANCEL);
		}
		else
		{
			send_to_char ("You are now immune to cancellation.\n\r", ch);
			SET_BIT (ch->act, PLR_NOCANCEL);
		}
	}
}

void 
do_look (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char *pdesc;
	int door;
	int number, count;

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING)
	{
		send_to_char ("You can't see anything but stars!\n\r", ch);
		return;
	}

	if (ch->position == POS_SLEEPING)
	{
		send_to_char ("You can't see anything, you're sleeping!\n\r", ch);
		return;
	}

	if (!check_blind (ch))
		return;

	if (!IS_NPC (ch)
		&& !IS_SET (ch->act, PLR_HOLYLIGHT)
		&& room_is_dark (ch->in_room))
	{
		send_to_char ("It is pitch black ... \n\r", ch);
		show_char_to_char (ch->in_room->people, ch);
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	number = number_argument (arg1, arg3);
	count = 0;

	if (arg1[0] == '\0' || !str_cmp (arg1, "auto"))
	{
		BUFFER *output;
		/* 'look' or 'look auto' */
                send_to_char ("{<", ch);
		send_to_char (ch->in_room->name, ch);
		send_to_char ("{x", ch);

		if ((IS_IMMORTAL (ch) && (IS_NPC (ch) || IS_SET (ch->act, PLR_HOLYLIGHT)))
			|| IS_BUILDER (ch, ch->in_room->area))
		{
			sprintf (buf, " [Room %d]", ch->in_room->vnum);
			send_to_char (buf, ch);
		}

		send_to_char ("\n\r", ch);

		if (arg1[0] == '\0'
			|| (!IS_NPC (ch) && !IS_SET (ch->comm, COMM_BRIEF)))
		{
			send_to_char ("  ", ch);
			send_to_char (ch->in_room->description, ch);
            send_to_char ("{x", ch);
			if (ch->in_room->vnum == chain)
			{
				send_to_char ("A huge black iron chain as thick as a tree trunk is drifting above the ground\n\r", ch);
				send_to_char ("here.\n\r", ch);
			}
            if(IS_SET(ch->in_room->affected_by, ROOM_AFF_FAERIE))
                send_to_char("This room contains a purple haze.\n\r",ch);

		}
        if(IS_SET(ch->in_room->affected_by, ROOM_AFF_FIRES))
            send_to_char("\n\rThe {!f{1i{!r{1e{!s{x of {r-{RT{Dor{rM{Den{Rt{r-{x burn brightly in the room.\n\r",ch);

		if (!IS_NPC (ch) && IS_SET (ch->act, PLR_AUTOEXIT))
		{
			send_to_char ("\n\r", ch);
			do_exits (ch, "auto");
		}

		output = show_list_to_char (ch->in_room->contents, ch, FALSE, FALSE);
		send_to_char( output->string, ch );
		free_buf( output );
		show_char_to_char (ch->in_room->people, ch);
		return;
	}

	if ( !str_cmp( arg1, "auto1" ) )
	{
                send_to_char( "{<", ch);
		send_to_char( ch->in_room->name, ch );
		send_to_char( "{x\n\r", ch);
		return;
	}

	if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in") || !str_cmp (arg1, "on"))
	{
		BUFFER *final;

		/* 'look in' */
		if (arg2[0] == '\0')
		{
			send_to_char ("Look in what?\n\r", ch);
			return;
		}

		if ((obj = get_obj_here (ch, arg2)) == NULL)
		{
			send_to_char ("You do not see that here.\n\r", ch);
			return;
		}

		switch (obj->item_type)
		{
		default:
			send_to_char ("That is not a container.\n\r", ch);
			break;

		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0)
			{
				send_to_char ("It is empty.\n\r", ch);
				break;
			}

			sprintf (buf, "It's %sfilled with  a %s liquid.\n\r",
					 obj->value[1] < obj->value[0] / 4
					 ? "less than half-" :
					 obj->value[1] < 3 * obj->value[0] / 4
					 ? "about half-" : "more than half-",
					 liq_table[obj->value[2]].liq_color
				);

			send_to_char (buf, ch);
			break;

		case ITEM_CONTAINER:
		case ITEM_PIT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (IS_SET (obj->value[1], CONT_CLOSED))
			{
				send_to_char ("It is closed.\n\r", ch);
				break;
			}

			act ("$p holds:", ch, obj, NULL, TO_CHAR);
			final = show_list_to_char (obj->contains, ch, TRUE, TRUE);
			send_to_char( final->string, ch );
			free_buf( final );
			break;
		}
		return;
	}

	if ((victim = get_char_room (ch, arg1)) != NULL)
	{
		show_char_to_char_1 (victim, ch);
		return;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (can_see_obj (ch, obj))
		{						/* player can see object */
			pdesc = get_extra_descr (arg3, obj->extra_descr);
			if (pdesc != NULL)
			{
				if (++count == number)
				{
					send_to_char (pdesc, ch);
					return;
				}
				else
					continue;
			}
			pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
			if ((pdesc != NULL))
			{
				if (++count == number)
				{
					send_to_char (pdesc, ch);
					return;
				}
				else
					continue;
			}

			if (is_name (arg3, obj->name))
				if (++count == number)
				{
					send_to_char (obj->description, ch);
					send_to_char ("\n\r", ch);
					return;
				}
		}
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if (can_see_obj (ch, obj))
		{
			pdesc = get_extra_descr (arg3, obj->extra_descr);
			if (pdesc != NULL)
				if (++count == number)
				{
					send_to_char (pdesc, ch);
					return;
				}

			pdesc = get_extra_descr (arg3, obj->pIndexData->extra_descr);
			if (pdesc != NULL)
				if (++count == number)
				{
					send_to_char (pdesc, ch);
					return;
				}
			if (is_name (arg3, obj->name))
				if (++count == number)
				{
					send_to_char (obj->description, ch);
					send_to_char ("\n\r", ch);
					return;
				}
		}
	}

	pdesc = get_extra_descr (arg3, ch->in_room->extra_descr);
	if (pdesc != NULL)
	{
		if (++count == number)
		{
			send_to_char (pdesc, ch);
			return;
		}
	}

	if (count > 0 && count != number)
	{
		if (count == 1)
			sprintf (buf, "You only see one %s here.\n\r", arg3);
		else
			sprintf (buf, "You only see %d of those here.\n\r", count);

		send_to_char (buf, ch);
		return;
	}

	if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
		door = 0;
	else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
		door = 1;
	else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
		door = 2;
	else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
		door = 3;
	else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
		door = 4;
	else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
		door = 5;
	else
	{
		send_to_char ("You do not see that here.\n\r", ch);
		return;
	}

	/* 'look direction' */
	if ((ch->alignment < 0)
		&& (pexit = ch->in_room->exit[door + 6]) != NULL)
		door += 6;
	if ((pexit = ch->in_room->exit[door]) == NULL)
	{
		send_to_char ("Nothing special there.\n\r", ch);
		return;
	}

	if (pexit->description != NULL && pexit->description[0] != '\0')
		send_to_char (pexit->description, ch);
	else
		send_to_char ("Nothing special there.\n\r", ch);

	if (pexit->keyword != NULL
		&& pexit->keyword[0] != '\0'
		&& pexit->keyword[0] != ' ')
	{
		if (IS_SET (pexit->exit_info, EX_CLOSED))
		{
			act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
		}
		else if (IS_SET (pexit->exit_info, EX_ISDOOR))
		{
			act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
		}
	}

	return;
}

/* RT added back for the hell of it */
void 
do_read (CHAR_DATA * ch, char *argument)
{
	do_look (ch, argument);
}

void 
do_examine (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Examine what?\n\r", ch);
		return;
	}

	do_look (ch, arg);

	if ((obj = get_obj_here (ch, arg)) != NULL)
	{
		switch (obj->item_type)
		{
		default:
			break;

		case ITEM_JUKEBOX:
			do_play (ch, "list");
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0)
			{
				if (obj->value[1] == 0)
					sprintf (buf, "Odd...there's no coins in the pile.\n\r");
				else if (obj->value[1] == 1)
					sprintf (buf, "Wow. One gold coin.\n\r");
				else
					sprintf (buf, "There are %d gold coins in the pile.\n\r",
							 obj->value[1]);
			}
			else if (obj->value[1] == 0)
			{
				if (obj->value[0] == 1)
					sprintf (buf, "Wow. One silver coin.\n\r");
				else
					sprintf (buf, "There are %d silver coins in the pile.\n\r",
							 obj->value[0]);
			}
			else
				sprintf (buf,
				   "There are %d gold and %d silver coins in the pile.\n\r",
						 obj->value[1], obj->value[0]);
			send_to_char (buf, ch);
			break;

		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_PIT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			sprintf (buf, "in %s", argument);
			do_look (ch, buf);
		}
	}

	return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void 
do_exits (CHAR_DATA * ch, char *argument)
{
	extern char *const dir_name[];
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found;
	bool round;
	bool fAuto;
	int door;
	int outlet;

	fAuto = !str_cmp (argument, "auto");

	if (!check_blind (ch))
		return;

	if (fAuto)
		sprintf (buf, "[Exits:");
	else if (IS_IMMORTAL (ch))
		sprintf (buf, "Obvious exits from room %d:\n\r", ch->in_room->vnum);
	else
		sprintf (buf, "Obvious exits:\n\r");

	found = FALSE;
	for (door = 0; door < 6; door++)
	{
		round = FALSE;
		outlet = door;
		if ((ch->alignment < 0)
			&& (pexit = ch->in_room->exit[door + 6]) != NULL)
			outlet += 6;
		if ((pexit = ch->in_room->exit[outlet]) != NULL
			&& pexit->u1.to_room != NULL
			&& can_see_room (ch, pexit->u1.to_room))
		{
			found = TRUE;
			round = TRUE;
			if (fAuto)
			{
                           if ( !IS_SET(pexit->exit_info,EX_HIDDEN) )
                           {
				strcat (buf, " ");
				if( IS_SET (pexit->exit_info, EX_CLOSED ) && !IS_SET(pexit->exit_info,EX_HIDDEN) )
					strcat (buf, "<");
				strcat (buf, dir_name[outlet]);
				if(IS_SET (pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->exit_info,EX_HIDDEN) )
					strcat (buf, ">");
                           }
			}
			else
			{
				sprintf (buf + strlen (buf), "%-5s - %s",
						 capitalize (dir_name[outlet]),
						 room_is_dark (pexit->u1.to_room)
						 ? "Too dark to tell"
						 : pexit->u1.to_room->name
					);
				if (IS_IMMORTAL (ch))
					sprintf (buf + strlen (buf),
							 " (room %d)\n\r", pexit->u1.to_room->vnum);
				else
					sprintf (buf + strlen (buf), "\n\r");
			}
		}
		if (!round)
		{
			OBJ_DATA *portal;
			ROOM_INDEX_DATA *to_room;

			portal = get_obj_exit (dir_name[door], ch->in_room->contents);
			if (portal != NULL)
			{
				found = TRUE;
				round = TRUE;
				if (fAuto)
				{
					strcat (buf, " ");
					strcat (buf, dir_name[door]);
				}
				else
				{
					to_room = get_room_index (portal->value[0]);
					sprintf (buf + strlen (buf), "%-5s - %s",
							 capitalize (dir_name[door]),
							 room_is_dark (to_room)
							 ? "Too dark to tell"
							 : to_room->name
						);
					if (IS_IMMORTAL (ch))
						sprintf (buf + strlen (buf),
								 " (room %d)\n\r", to_room->vnum);
					else
						sprintf (buf + strlen (buf), "\n\r");
				}
			}
		}
	}

	if (!found)
		strcat (buf, fAuto ? " none" : "None.\n\r");

	if (fAuto)
		strcat (buf, "]\n\r");

	send_to_char (buf, ch);
	return;
}

void 
do_worth (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
	{
		sprintf (buf, "You have %d quest points, %ld platinum, %ld gold and %ld silver.\n\r",
				 ch->qps, ch->platinum, ch->gold, ch->silver);
		send_to_char (buf, ch);
		return;
	}

	sprintf (buf,
			 "You have %ld platinum, %ld gold, %ld silver,\n\rand %ld experience (%ld exp to level).\n\r",
			 ch->platinum, ch->gold, ch->silver, ch->exp,
		(ch->level + 1) * exp_per_level (ch, ch->pcdata->points) - ch->exp);

	send_to_char (buf, ch);

	return;
}


/*
 * do_score modifications by Vengeance and Artema
 */
void do_score (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int i;

	if (IS_NPC (ch))
	{
		do_mstat (ch, "self");
		return;
	}

	sprintf (buf,
			 "{xYou are {G%s{x%s{x\n\r", ch->name, ch->pcdata->title);
	send_to_char(buf,ch);

	sprintf (buf,
			 "{xLevel {B%d{x,  {B%d{x years old.\n\r",
			 ch->level, get_age (ch));
	send_to_char(buf,ch);

	if (get_trust (ch) != ch->level)
	{
		sprintf (buf, "{xYou are trusted at level {B%d{x.\n\r",
				 get_trust (ch));
		send_to_char(buf,ch);
	}

	sprintf (buf, "{xRace: {M%s{x  Sex: {M%s{x  Class: {M%s{x  Size: {M%s{x\n\r",
			 race_table[ch->race].name,
			 ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
			 class_table[ch->class].name, size_table[ch->size].name);
	send_to_char(buf,ch);

	sprintf (buf,
			 "{xYou have {G%d{x/{B%d{x hit, {G%d{x/{B%d{x mana, {G%d{x/{B%d{x movement.\n\r",
			 ch->hit, ch->max_hit,
			 ch->mana, ch->max_mana,
			 ch->move, ch->max_move);
	send_to_char(buf,ch);

	sprintf (buf,
			 "{xYou have {M%d{x practice%s, {Y%d{x train%s, and {G%d{x Iquest point%s.\n\r",
			 ch->practice, ch->practice == 1 ? "" : "s",
			 ch->train, ch->train == 1 ? "" : "s",
			 ch->qps, ch->qps == 1 ? "" : "s");
	send_to_char(buf,ch);

	if (is_clan (ch))
	{
		sprintf (buf,
   "{xYou are in the [{G%s{x] clan, which is a clan.\n\r",clan_table[ch->clan].who_name);
/*      is_pkill(ch) ? "{RPkill{x" : "{WNon-Pkill{x"); */
		send_to_char(buf,ch);
/*  if (is_pkill(ch))
   {
   sprintf( buf,
   "{xPk Rank: {R%d  {xArena Rank: {R%d{x\n\r",
   ch->pcdata->prank, ch->pcdata->arank);
   send_to_char(buf,ch);
   } */
	}

	if (!IS_NPC (ch))
	{
		sprintf (buf, "[{barena stats{x] {gwins{x: {y%d{x  {rlosses{x: {y%d{x\n\r",
				 ch->pcdata->awins, ch->pcdata->alosses);
		send_to_char(buf,ch);
		if (ch->challenger != NULL)
		{
			sprintf (buf, "[{garena{x] You have been {rchallenged{x by {b%s{x.\n\r",
					 ch->challenger->name);
			send_to_char(buf,ch);
		}
		if (ch->challenged != NULL)
		{
			sprintf (buf, "[{garena{x] You have {rchallenged{x {b%s{x.\n\r",
					 ch->challenged->name);
			send_to_char(buf,ch);
		}
		if (ch->gladiator != NULL)
		{
			sprintf (buf, "[{garena bet{x] You have a {y%d{x gold bet on {b%s{x\n\r",
					 ch->pcdata->plr_wager, ch->gladiator->name);
			send_to_char(buf,ch);
		}
	}

	sprintf (buf,
			 "{xYou are carrying {G%d{x/{B%d{x items with weight {G%ld{x/{B%d{x pounds.\n\r",
			 ch->carry_number, can_carry_n (ch),
			 get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
	send_to_char(buf,ch);

	sprintf (buf,
			 "{xStr: {R%d{x({r%d{x)  Int: {R%d{x({r%d{x)  Wis: {R%d{x({r%d{x)  Dex: {R%d{x({r%d{x)  Con: {R%d{x({r%d{x)\n\r",
			 ch->perm_stat[STAT_STR],
			 get_curr_stat (ch, STAT_STR),
			 ch->perm_stat[STAT_INT],
			 get_curr_stat (ch, STAT_INT),
			 ch->perm_stat[STAT_WIS],
			 get_curr_stat (ch, STAT_WIS),
			 ch->perm_stat[STAT_DEX],
			 get_curr_stat (ch, STAT_DEX),
			 ch->perm_stat[STAT_CON],
			 get_curr_stat (ch, STAT_CON));
	send_to_char(buf,ch);

	sprintf (buf,
			 "{xYou have {W%ld platinum{x, {Y%ld gold{x and {w%ld silver{x coins.\n\r",
			 ch->platinum, ch->gold, ch->silver);
	send_to_char(buf,ch);
	for (i = 0; i < 4; i++)
	{
		if (ch->balance[i] > 0)
		{
			sprintf (buf, "{R%-35s{W%6ld platinum.{x",
					 bank_table[i].name, ch->balance[i]);
			send_to_char(buf,ch);
			if (is_compromised (ch, i))
			{
				sprintf (buf, "  {R{zcompromised{x\n\r");
			}
			else
			{
				sprintf (buf, "\n\r");
			}
			send_to_char(buf,ch);
		}
	}
	if (!IS_NPC (ch) && ch->level == LEVEL_HERO)
	{
		sprintf (buf, "{xYou have scored {C%ld exp{x.\n\r", ch->exp);
		send_to_char(buf,ch);
	}
	else if (!IS_NPC (ch) && ch->level < LEVEL_HERO)
	{
		sprintf (buf, "{xYou have scored {C%ld exp{x. You need {C%ld exp{x to level.\n\r",
				 ch->exp, ((ch->level + 1) * exp_per_level (ch, ch->pcdata->points) - ch->exp));
		send_to_char(buf,ch);
	}

	if (ch->wimpy)
	{
		sprintf (buf, "Wimpy set to %d hit points.\n\r", ch->wimpy);
		send_to_char(buf,ch);
	}

	if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	{
		sprintf (buf, "{yYou are drunk.{x\n\r");
		send_to_char(buf,ch);
	}
	if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
	{
		sprintf (buf, "{yYou are thirsty.{x\n\r");
		send_to_char(buf,ch);
	}
	if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
	{
		sprintf (buf, "{yYou are hungry.{x\n\r");
		send_to_char(buf,ch);
	}

	switch (ch->position)
	{
	case POS_DEAD:
		sprintf (buf, "{RYou are DEAD!!{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_MORTAL:
		sprintf (buf, "{RYou are mortally wounded.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_INCAP:
		sprintf (buf, "{RYou are incapacitated.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_STUNNED:
		sprintf (buf, "{RYou are stunned.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_SLEEPING:
		sprintf (buf, "{BYou are sleeping.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_RESTING:
		sprintf (buf, "{BYou are resting.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_STANDING:
		sprintf (buf, "{BYou are standing.{x\n\r");
		send_to_char(buf,ch);
		break;
	case POS_FIGHTING:
		sprintf (buf, "{RYou are fighting.{x\n\r");
		send_to_char(buf,ch);
		break;
	}


	/* print AC values */
	if (ch->level >= 25)
	{
		sprintf (buf, "{xArmor: pierce: {G%d{x  bash: {G%d{x  slash: {G%d{x  exotic: {G%d{x\n\r",
				 GET_AC (ch, AC_PIERCE),
				 GET_AC (ch, AC_BASH),
				 GET_AC (ch, AC_SLASH),
				 GET_AC (ch, AC_EXOTIC));
		send_to_char(buf,ch);
	}

	for (i = 0; i < 4; i++)
	{
		char *temp;

		switch (i)
		{
		case (AC_PIERCE):
			temp = "piercing";
			break;
		case (AC_BASH):
			temp = "bashing";
			break;
		case (AC_SLASH):
			temp = "slashing";
			break;
		case (AC_EXOTIC):
			temp = "exotic";
			break;
		default:
			temp = "error";
			break;
		}

		sprintf (buf, "{xYou are ");
		send_to_char(buf,ch);

		if (GET_AC (ch, i) >= 101)
			sprintf (buf, "{Rhopelessly vulnerable{x to %s.\n\r", temp);
		else if (GET_AC (ch, i) >= 80)
			sprintf (buf, "{Rdefenseless{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= 60)
			sprintf (buf, "{Rbarely protected{x from %s.\n\r", temp);
		else if (GET_AC (ch, i) >= 40)
			sprintf (buf, "{yslightly armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= 20)
			sprintf (buf, "{ysomewhat armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= 0)
			sprintf (buf, "{yarmored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -20)
			sprintf (buf, "{ywell-armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -40)
			sprintf (buf, "{yvery well-armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -60)
			sprintf (buf, "{Bheavily armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -100)
			sprintf (buf, "{Bsuperbly armored{x against %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -200)
			sprintf (buf, "{Ynear invulnerable{x to %s.\n\r", temp);
		else if (GET_AC (ch, i) >= -300)
			sprintf (buf, "{Yalmost invulnerable{x to %s.\n\r", temp);
		else
			sprintf (buf, "{Wdivinely armored{x against %s.\n\r", temp);

		send_to_char(buf,ch);
	}


	/* RT wizinvis and holy light */
	if (IS_IMMORTAL (ch))
	{
		sprintf (buf, "Holy Light: ");
		send_to_char(buf,ch);
		if (IS_SET (ch->act, PLR_HOLYLIGHT))
			sprintf (buf, "{Won{x");
		else
			sprintf (buf, "{Roff{x");

		send_to_char(buf,ch);

		if (ch->invis_level)
		{
			sprintf (buf, "  Invisible: level {W%d{x", ch->invis_level);
			send_to_char(buf,ch);
		}

		if (ch->incog_level)
		{
			sprintf (buf, "  Incognito: level {W%d{x", ch->incog_level);
			send_to_char(buf,ch);
		}

		if (ch->ghost_level)
		{
			sprintf (buf, "  Ghost: level {W%d{x", ch->ghost_level);
			send_to_char(buf,ch);
		}

		sprintf (buf, "\n\r");
		send_to_char(buf,ch);
	}

	if (ch->level >= 15)
	{
		sprintf (buf, "{xHitroll: {G%d{x  Damroll: {G%d{x  Saves: {G%d{x.\n\r",
				 GET_HITROLL (ch), GET_DAMROLL (ch), ch->saving_throw);
		send_to_char(buf,ch);
	}

	if (ch->level >= 10)
	{
		sprintf (buf, "{xAlignment: {B%d{x.  ", ch->alignment);
		send_to_char(buf,ch);
	}

	sprintf (buf, "{xYou are ");
	send_to_char(buf,ch);
	if (ch->alignment > 900)
		sprintf (buf, "{&O{7di{&n{x's chosen{x.\n\r");
	else if (ch->alignment > 700)
		sprintf (buf, "{&O{7di{&n{x's servant{x.\n\r");
	else if (ch->alignment > 500)
		sprintf (buf, "{wsaintly{x.\n\r");
	else if (ch->alignment > 350)
		sprintf (buf, "{wgood{x.\n\r");
	else if (ch->alignment > 225)
		sprintf (buf, "{ykind{x.\n\r");
	else if (ch->alignment > 100)
		sprintf (buf, "{ynice{x.\n\r");
	else if (ch->alignment > -100)
		sprintf (buf, "{yneutral{x.\n\r");
	else if (ch->alignment > -225)
		sprintf (buf, "{yunkind{x.\n\r");
	else if (ch->alignment > -350)
		sprintf (buf, "{ymean{x.\n\r");
	else if (ch->alignment > -500)
		sprintf (buf, "{revil{x.\n\r");
	else if (ch->alignment > -700)
		sprintf (buf, "{rdemonic{x.\n\r");
	else if (ch->alignment > -900)
		sprintf (buf, "{!H{1e{!l{x's servant{x.\n\r");
	else
		sprintf (buf, "{!H{1e{!l{x's chosen{x.\n\r");

	send_to_char(buf,ch);

/*    if (ch->invited)
   {
   sprintf( buf, "{RYou have been invited to join clan {x[{%s%s{x]\n\r",
   clan_table[ch->invited].pkill ? "B" : "M",
   clan_table[ch->invited].who_name);
   send_to_char(buf,ch);
   } */
	if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
		do_affects (ch, "");
}

void do_abilities ( CHAR_DATA *ch, char *argument)
{
    int count = 0;

    if ( IS_NPC(ch) )
       return;

    send_to_char("{D  ._______________________________________________.{x\n\r",ch);
    send_to_char("{D /                   {y[{cABILITIES{y]                   {D\\{x\n\r",ch);
    send_to_char("{D|                                                   {D|{x\n\r",ch);
    
    if ( ch->class == CLASS_SHADE || ch->class == CLASS_LICH || ch->class == CLASS_FADE || ch->class == CLASS_BANSHEE )
    {
        count = ( count == 1 ? 1 : count + 1 );
    
        send_to_char("{D| {mNightstalker{w:",ch);

        if ( IS_SET( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER ) )
           send_to_char(" {GON                                  {D|{x\n\r",ch);
        else
           send_to_char(" {ROFF                                 {D|{x\n\r",ch);
    } 

    /*if ( ch->class == CLASS_CRUSADER )
    {
        count = ( count == 1 ? 1 : count + 1 );   

        send_to_char("{D| {mSunder{w:",ch);
    
        if ( IS_SET( ch->ability_crusader, ABILITY_CRUSADER_SUNDER ) )
           send_to_char(" {GON                                        {D|{x \n\r",ch);
        else
           send_to_char(" {ROFF                                       {D|{x\n\r",ch);
    
    } */   
    if ( count == 0 )
       send_to_char("{D|                  {RTHERE ARE NONE                   {D|{x\n\r",ch);     
    else
       send_to_char("{D|                                                   {D|{x\n\r",ch);    

    send_to_char("{D|___________________________________________________|{x\n\r",ch);
}


void do_affects (CHAR_DATA * ch, char *argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;
	char buf[MAX_STRING_LENGTH];

    if (ch->affected_by)
    {
        sprintf(buf, "Affected by %s\n\r",
            affect_bit_name(ch->affected_by));
        send_to_char(buf,ch);
    }

    if (ch->shielded_by)
    {
        sprintf(buf, "Shielded by %s\n\r",
            shield_bit_name(ch->shielded_by));
        send_to_char(buf,ch);
    }
	if (ch->affected != NULL)
	{
		sprintf (buf, "You are affected by the following spells:\n\r");
		send_to_char(buf,ch);
		if (ch->class == 25)
		{
			sprintf (buf, "{WF{waith{x Modifies: {BH{bitroll {R%d, {BD{bamroll {R%d, {BS{btrength {R%d, {BD{bexterity {R%d{x\n\r",
			 (ch->alignment*ch->level)/2000, (ch->alignment*ch->level)/2000,
			 ch->alignment/300, ch->alignment/300);
			send_to_char(buf,ch);
		}
                if(ch->class == CLASS_MONK)
                {
                 sprintf(buf,"{WC{whi{x Modifies: DamRoll %d HitRoll %d.\n\r",
                 ch->pcdata->power[0]/10+ch->pcdata->power[1]/10, ch->pcdata->power[0]/10);
		 send_to_char(buf,ch);
                }

		for (paf = ch->affected; paf != NULL; paf = paf->next)
		{
			if (paf_last != NULL && paf->type == paf_last->type)
				sprintf (buf, "                      ");
			else
				sprintf (buf, "Spell: {W%-15s{x", skill_table[paf->type].name);

			send_to_char(buf,ch);


			sprintf (buf,
					": modifies {R%s{x by {G%d{x ",
					affect_loc_name (paf->location),
					paf->modifier);
			send_to_char(buf,ch);
			if (paf->duration == -1)
				sprintf (buf, "{Bpermanently{x");
			else
				sprintf (buf, "for {B%d hours{x", paf->duration);
			send_to_char(buf,ch);

			sprintf (buf, "\n\r");
			send_to_char(buf,ch);
			paf_last = paf;
		}
	} 
	else
	{ 
		sprintf (buf, "You are not affected by any spells.\n\r");
		send_to_char(buf,ch);
		if (ch->class == 25)
		{
			sprintf (buf, "{WF{waith{x Modifies: {BH{bitroll {R%d, {BD{bamroll {R%d, {BS{btrength {R%d, {BD{bexterity {R%d{x\n\r",
			 ch->alignment/100, ch->alignment/100,
			 ch->alignment/300, ch->alignment/300);
			send_to_char(buf,ch);
		}
	}
	return;
}


void do_cooldowns (CHAR_DATA * ch, char *argument)
{
	COOLDOWN_DATA *pcd = NULL;
	char buf[MAX_STRING_LENGTH];

	if (ch->cooldowns != NULL)
	{
		sprintf (buf, "Cooldowns:\n\r");
		send_to_char(buf,ch);
   
		for (pcd = ch->cooldowns; pcd != NULL; pcd = pcd->next)
		{
			sprintf (buf, "{W%-15s{x: {B%d rounds{x\n\r", skill_table[pcd->type].name, pcd->duration);
			send_to_char(buf,ch);
		}
	} 
	else
	{ 
		sprintf (buf, "Nothing is on cooldown.\n\r");
		send_to_char(buf,ch);
	}
	return;
}


char *const day_name[] =
{
	"Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
	"Saturday", "Sunday"
};

char *const month_name[] =
{
	"January", "February", "March", "April",
	"May", "June", "July", "August", "September",
	"October", "November", "December"	/*  , "the Dark Shades", "the Shadows",
										   "the Long Shadows", "the Ancient Darkness", "the Great Evil" */
};

void 
do_time (CHAR_DATA * ch, char *argument)
{
	extern char str_boot_time[];
	char buf[MAX_STRING_LENGTH];
	char *suf;
	int day;

	day = time_info.day + 1;

	if (day > 4 && day < 20)
		suf = "th";
	else if (day % 10 == 1)
		suf = "st";
	else if (day % 10 == 2)
		suf = "nd";
	else if (day % 10 == 3)
		suf = "rd";
	else
		suf = "th";

	sprintf (buf,
			 "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
			 (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
			 time_info.hour >= 12 ? "pm" : "am",
			 day_name[day % 7],
			 day, suf,
			 month_name[time_info.month]);
	send_to_char (buf, ch);
	sprintf (buf, "Distorted Illusions started up at %s\n\rThe system time is %s\n\r",
			 str_boot_time,
			 (char *) ctime (&current_time)
		);
	send_to_char (buf, ch);
	return;
}



void 
do_weather (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	static char *const sky_look[4] =
	{
		"cloudless",
		"cloudy",
		"rainy",
		"lit by flashes of lightning"
	};

	if (!IS_OUTSIDE (ch))
	{
		send_to_char ("You can't see the weather indoors.\n\r", ch);
		return;
	}

	sprintf (buf, "The sky is %s and %s.\n\r",
			 sky_look[weather_info.sky],
			 weather_info.change >= 0
			 ? "a warm southerly breeze blows"
			 : "a cold northern gust blows"
		);
	send_to_char (buf, ch);
	return;
}

/* Old help, won't do anything at the moment */
void do_help (CHAR_DATA * ch, char *argument)
{
	HELP_DATA *pHelp;
	char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

	if (argument[0] == '\0')
		argument = "summary";

        /* RaceHelps -- Skyntil */
        if(is_racename(argument))
        {
         do_racehelp(ch,argument);
        }

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (argument[0] != '\0')
	{
		argument = one_argument (argument, argone);
		if (argall[0] != '\0')
			strcat (argall, " ");
		strcat (argall, argone);
	}

	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	{
		if (pHelp->level > get_trust (ch))
			continue;

		if (is_name (argall, pHelp->keyword))
		{
			if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
			{
				send_to_char (pHelp->keyword, ch);
				send_to_char ("\n\r", ch);
			}

			/*
			 * Strip leading '.' to allow initial blanks.
			 */
			if (pHelp->text[0] == '.')
				page_to_char (pHelp->text + 1, ch);
			else
				page_to_char (pHelp->text, ch);

	    /* small hack :) */
	    if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	    &&  		    ch->desc->connected != CON_GEN_GROUPS)
		break;

			return;
		}
	}

	send_to_char ("No help on that word, use the nohelp command to log it.\n\r", ch);
	return;
}

void do_classstat(CHAR_DATA *ch, char *argument)
{
        /* ClassHelps -- Skyntil */
        if(is_classname(argument))
        {
         do_classhelp(ch,argument);
         return;
        }

        send_to_char("That is not a valid class.\n\r",ch);
        return;
}


void do_oldhelp( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int level;

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
         strcat(argall," ");
         strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
    	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

	if (level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    /* add seperator if found */
	    if (found)
		send_to_char(
    "\n\r============================================================\n\r\n\r",ch);
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
            {
		send_to_char(pHelp->keyword,ch);
		send_to_char("\n\r",ch);
            }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		send_to_char(pHelp->text+1,ch);
	    else
		send_to_char(pHelp->text,ch);
	    found = TRUE;
           /*  small hack :)  */
	    if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	    &&  		    ch->desc->connected != CON_GEN_GROUPS)
		break;
	}
    }

    if (!found)
		send_to_char( "No help on that word.\n\r", ch );
    return;
} 



/* whois command */
void do_whois (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	BUFFER *output;
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("You must provide a name.\n\r", ch);
		return;
	}

	output = new_buf ();

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char class[MAX_STRING_LENGTH];
		char sex[MAX_STRING_LENGTH];

		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch))
			continue;

		if (wch->pcdata->pretit == '\0')
			wch->pcdata->pretit = "{x";
    
                if (IS_NPC (wch))
			continue;

		if (!str_prefix (arg, wch->name))
		{
			found = TRUE;

			if (wch->pcdata->tier == 1)
			{
				sprintf (class, "{R%c{r%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}
			else if (wch->pcdata->tier == 2)
			{
				sprintf (class, "{B%c{b%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}
			else if (wch->pcdata->tier >= 3)
			{
				sprintf (class, "{G%c{g%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}

			if (wch->sex == 1)
			{
				sprintf (sex, "{#M{x");
			}
			else if (wch->sex == 2)
			{
				sprintf (sex, "{#F{x");
			}
			else if (wch->sex == 0)
			{
				sprintf (sex, "{#N{x");
			}

			/* a little formatting */
                        sprintf (buf, "[%-3d %-12s %s %s] %s%s%s%s%s%s%s%s%s%s%s%s%s%s{x",
					 wch->level,
				 wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
					 : "            ",
					 class,
					 sex,
					 ((wch->ghost_level >= LEVEL_HERO) && (ch->level >= wch->level)) ? "(Ghost) " : "",
					 wch->incog_level >= LEVEL_HERO ? "({^Incog{x) " : "",
					 wch->invis_level >= LEVEL_HERO ? "({&Wizi{x) " : "",
					 clan_table[wch->clan].who_name,
                                         IS_SET (wch->plyr, PLAYER_GHOST) ? "({7G{wH{7O{wS{7T{x) " : "",
					 IS_SET (wch->comm, COMM_QUIET) ? "({@Quiet{x) " : "",
					 IS_SET (wch->comm, COMM_AFK) ? "{3[{#AFK{3]{x " : "",
					 IS_SET (wch->act, PLR_TWIT) ? "({RTWIT{x) " : "",
					 IS_SET (wch->plyr, PLAYER_NEWBIE) ? "{Y{{{Rn00b{Y}{x " : "",
					 "",
					 is_clan (wch) ? clan_rank_table[wch->rank].rank : "",
					 wch->pcdata->pretit,
					 wch->name,
					 IS_NPC (wch) ? "" : wch->pcdata->title
				);
			add_buf (output, buf);
			sprintf (buf, "\n\r{GPk Rank: {R%d (%d-%d) {GArena Rank: {B%d (%d-%d){x\n\r",
					 wch->pcdata->prank, wch->pcdata->pkills, wch->pcdata->pdeath, wch->pcdata->arank, wch->pcdata->awins, wch->pcdata->alosses);
			/*add_buf (output, buf);
		    sprintf (buf, "\n\r{GPkill:{1 %d - %d	 {GArena:{4 %dW - %dL{x\n\r",
					 wch->pcdata->pkills, wch->pcdata->pdeath, wch->pcdata->awins, wch->pcdata->alosses);*/
			add_buf (output, buf);
			sprintf (buf, "{C%s{x", wch->description);
			add_buf (output, buf);
		}
	}

	if (!found)
	{
		send_to_char ("No one of that name is playing.\n\r", ch);
		return;
	}

	page_to_char (buf_string (output), ch);
	free_buf (output);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who (CHAR_DATA * ch, char *argument)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char questdat[MAX_STRING_LENGTH];
	char whodesc[MAX_STRING_LENGTH];
	char race[MAX_STRING_LENGTH];
	char sex[MAX_STRING_LENGTH];
	char level[MAX_STRING_LENGTH];
	char clan[MAX_STRING_LENGTH];
	char rank[MAX_STRING_LENGTH];
	char *mplayer_name = "";
	char *immplayer_name = "";
	BUFFER *output;
	BUFFER *outputimm;
        BUFFER *ftier;
        BUFFER *stier;
        BUFFER *ttier;
	DESCRIPTOR_DATA *d;
	int iClass;
	int iRace;
	int iClan;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	bool rgfClass[MAX_CLASS];
	bool rgfRace[MAX_PC_RACE];
	bool rgfClan[MAX_CLAN];
	bool fClassRestrict = FALSE;
	bool fClanRestrict = FALSE;
	bool fClan = FALSE;
	bool fRaceRestrict = FALSE;
	bool fImmortalOnly = FALSE;
	bool fPkillOnly = FALSE;
	bool fArenaOnly = FALSE;
	bool fMCCPOnly = FALSE;
	int count;
	int countimm;
	int hour;
	int most_ever = 0;
	int most_hours = 0;
	int immost_hours = 0;

	/*
	 * Set default arguments.
	 */
	iLevelLower = 0;
	iLevelUpper = MAX_LEVEL;
	for (iClass = 0; iClass < MAX_CLASS; iClass++)
		rgfClass[iClass] = FALSE;
	for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
		rgfRace[iRace] = FALSE;
	for (iClan = 0; iClan < MAX_CLAN; iClan++)
		rgfClan[iClan] = FALSE;

	/*
	 * Parse arguments.
	 */
	count = 0;
	countimm = 0;
	nNumber = 0;
	for (;;)
	{
		char arg[MAX_STRING_LENGTH];

		argument = one_argument (argument, arg);
		if (arg[0] == '\0')
			break;

		if (is_number (arg))
		{
			switch (++nNumber)
			{
			case 1:
				iLevelLower = atoi (arg);
				break;
			case 2:
				iLevelUpper = atoi (arg);
				break;
			default:
				send_to_char ("Only two level numbers allowed.\n\r", ch);
				return;
			}
		}
		else
		{

			/*
			 * Look for classes to turn on.
			 */
			if (!str_prefix (arg, "immortals"))
			{
				fImmortalOnly = TRUE;
			}
			else if (!str_prefix (arg, "pkill"))
			{
				fPkillOnly = TRUE;
			}
			else if (!str_prefix (arg, "arena"))
			{
				fArenaOnly = TRUE;
			}
			else if (!str_prefix (arg, "mccp"))
			{
				fMCCPOnly = TRUE;
			}
			else
			{
				iClass = class_lookup (arg);
				if (iClass == -1)
				{
					iRace = race_lookup (arg);

					if (iRace == 0 || iRace >= MAX_PC_RACE)
					{
						if (!str_prefix (arg, "clan"))
							fClan = TRUE;
						else
						{
							iClan = clan_lookup (arg);
							if (iClan)
							{
								fClanRestrict = TRUE;
								rgfClan[iClan] = TRUE;
							}
							else
							{
								send_to_char (
												 "That's not a valid race, class, or clan.\n\r",
												 ch);
								return;
							}
						}
					}
					else
					{
						fRaceRestrict = TRUE;
						rgfRace[iRace] = TRUE;
					}
				}
				else
				{
					fClassRestrict = TRUE;
					rgfClass[iClass] = TRUE;
				}
			}
		}
	}

	/*
	 * Now show matching chars.
	 */
	nMatch = 0;
	buf[0] = '\0';
	output = new_buf ();
	outputimm = new_buf ();
        ftier = new_buf ();
        stier = new_buf ();
        ttier = new_buf ();
        add_buf (outputimm, "\n\r{xVisible {GImmortals{x:\n\r");
        add_buf (ftier, "\n\r{xVisible {GMortals{x:\n\r");
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char class[MAX_STRING_LENGTH];

		/*
		 * Check for match against restrictions.
		 * Don't use trust as that exposes trusted mortals.
		 */
		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch) && IS_SHIELDED (wch, SHD_INVISIBLE))
			continue;

                /* Assassins don't show up */
  //              if(is_affected(wch,gsn_conceal) && ch != wch)
    //              continue;

		if (wch->pcdata->pretit == '\0')
			wch->pcdata->pretit = "{x";

		if (fPkillOnly)
			continue;

		if (fArenaOnly)
			continue;

		if (!can_see (ch, wch) && wch->level >= SQUIRE
			&& ch->level < wch->level)
			continue;

		if (!IS_IMMORTAL (wch))
			continue;

		if (wch->level < iLevelLower
			|| wch->level > iLevelUpper
			|| (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
			|| (fClassRestrict && !rgfClass[wch->class])
			|| (fRaceRestrict && !rgfRace[wch->race])
			|| (fClan && !is_clan (wch))
			|| (fClanRestrict && !rgfClan[wch->clan])
			|| (fMCCPOnly && !d->out_compress))
			continue;

		countimm++;

		nMatch++;

		/*
		 * Figure out what to print for class.
		 */
		if (wch->pcdata->who_descr[0] == '\0')
		{
			sprintf (level, "%-3d ", wch->level);

			if (wch->pcdata->tier == 1)
			{
				sprintf (class, "{R%c{r%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);

			}
			else if (wch->pcdata->tier == 2)
			{
				sprintf (class, "{B%c{b%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}
			else if (wch->pcdata->tier >= 3)
			{
				sprintf (class, "{G%c{g%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}

			if (wch->sex == 1)
			{
				sprintf (sex, " {#M{x");
			}
			else if (wch->sex == 2)
			{
				sprintf (sex, " {#F{x");
			}
			else if (wch->sex == 0)
			{
				sprintf (sex, " {#N{x");
			}

			if (wch->race < MAX_PC_RACE)
			{
				sprintf (race, "%-7s", pc_race_table[wch->race].who_name);
			}
			else
			{
				sprintf (race, "            ");
			}    

			if (is_clan(wch))
			{
				sprintf (clan, "%s", clan_table[wch->clan].who_name);
				sprintf (rank, "%s", clan_rank_table[wch->rank].rank);
			}
			else
			{
				sprintf (clan, "{W  ({yNon-PK{W)  {x");
				sprintf (rank, " ---");
			}

			sprintf (whodesc, "%s%s%s%s %s%s", level, race, class, sex, clan, rank);
		}
		else
		{

			if (is_clan(wch))
			{
				sprintf (clan, "%s", clan_table[wch->clan].who_name);
			}
			else
			{
				sprintf (clan, "{B -{WI{wmmortal{B- {x");
			}
			sprintf (whodesc, "%s %s", wch->pcdata->who_descr, clan);
		}

		/*
		 * Format it up.
		 */
		if (!wch->on_quest)
		{
			questdat[0] = '\0';
		}
		else
		{
			sprintf (questdat, "[{GQ{x] ");
		}

                sprintf (buf, "[%s] %s%s%s%s%s%s%s%s%s%s%s%s%s{x\n\r",
				 whodesc,
				 questdat,
				 ((wch->ghost_level >= LEVEL_HERO) && (ch->level >= wch->level)) ? "(Ghost) " : "",
				 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
				 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                                 IS_SET (wch->plyr, PLAYER_GHOST) ? "({7G{wH{7O{wS{7T{x) " : "",
				 IS_SET (wch->comm, COMM_QUIET) ? "({@Quiet{x)" : "",
				 IS_SET (wch->comm, COMM_AFK) ? "[{yAFK{x] " : "",
				 IS_SET (wch->act, PLR_TWIT) ? "({RTWIT{x) " : "",
				 IS_SET (wch->plyr, PLAYER_NEWBIE) ? "{Y{{{Rn00b{Y}{x " : "",
				 "",
				 wch->pcdata->pretit,
				 wch->name,
				 IS_NPC (wch) ? "" : wch->pcdata->title);

                if ((wch->played/3600) > immost_hours)
                {
                        immplayer_name = wch->name;
                        immost_hours = (wch->played/3600);
                }

		add_buf (outputimm, buf);
	}

	add_buf (outputimm, "\n\r");
	buf[0] = '\0';

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char class[MAX_STRING_LENGTH];

		/*
		 * Check for match against restrictions.
		 * Don't use trust as that exposes trusted mortals.
		 */
		if (d->connected != CON_PLAYING || !can_see (ch, d->character))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see (ch, wch) && IS_SHIELDED (wch, SHD_INVISIBLE))
			continue;

      //         if(is_affected(wch,gsn_conceal) && ch != wch && !IS_IMMORTAL(ch))
       //          continue;

		if (!can_see (ch, wch) && wch->level >= SQUIRE
			&& ch->level < wch->level)
			continue;

		if (wch->pcdata->pretit == '\0')
			wch->pcdata->pretit = "{x";

		if (IS_IMMORTAL (wch))
			continue;

		if (fPkillOnly)
		{ //NEWBIE INSERT
			if (!is_pkill (ch) && !IS_SET (wch->act, PLR_TWIT))
				continue;
			if (IS_SET (wch->act, PLR_TWIT))
				break;
			if (!is_pkill (wch))
				continue;
			if (ch->level > wch->level + 10)
				continue;
			if (ch->level < wch->level - 10)
				continue;
	                if ( ((ch->pcdata->tier == 3) 
			&& (wch->pcdata->tier == 1)) || ((ch->pcdata->tier == 1) 
			&& (wch->pcdata->tier == 3)))
                		continue;
			/*if ( ch->pcdata->prank > (wch->pcdata->prank + 200) )
				continue; 
			if ( ch->pcdata->prank < (wch->pcdata->prank - 200) )
				continue;*/ 
/*      if ((ch->pcdata->tier == 2)
   &&  (wch->pcdata->tier == 1))
   continue;
   if ((ch->pcdata->tier == 1)
   &&  (wch->pcdata->tier == 2))
   continue; */
/*	if (wch->clan == ch->clan && clan_table[ch->clan].name != "LONER")
				continue;*/
		}

		count++;

		if ((wch->in_room->vnum == ROOM_VNUM_CORNER)
			&& (!IS_IMMORTAL (ch)))
			continue;

		if (wch->level < iLevelLower
			|| wch->level > iLevelUpper
			|| (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
			|| (fClassRestrict && !rgfClass[wch->class])
			|| (fRaceRestrict && !rgfRace[wch->race])
			|| (fClan && !is_clan (wch))
			|| (fClanRestrict && !rgfClan[wch->clan])
			|| (fMCCPOnly && !d->out_compress))
			continue;

		nMatch++;

		/*
		 * Figure out what to print for class.
		 */
		if (wch->sex == 1)
		{
			sprintf (sex, " {#M{x");
		}
		else if (wch->sex == 2)
		{
			sprintf (sex, " {#F{x");
		}
		else if (wch->sex == 0)
		{
			sprintf (sex, " {#N{x");
		}

		if (wch->level == MAX_LEVEL - 9)
		{
			if (wch->pcdata->tier >= 3)
			{
				sprintf (level, "{#LGD{x ");
			}
			else if (wch->pcdata->tier == 2)
			{
				sprintf (level, "{!HRO{x ");
			}
			else
			{
                                sprintf (level, "{@LDR{x ");
			}
		}
		else
		{
			sprintf (level, "%-3d ", wch->level);
		}


		if (wch->pcdata->who_descr[0] == '\0')
		{

			if (wch->pcdata->tier ==1)
			{
				sprintf (class, "{R%c{r%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);

			}
			else if (wch->pcdata->tier == 2)
			{
				sprintf (class, "{B%c{b%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}
			else
			{
				sprintf (class, "{G%c{g%c%c{x",
						 class_table[wch->class].who_name[0],
						 class_table[wch->class].who_name[1],
						 class_table[wch->class].who_name[2]);
			}

			if (wch->race < MAX_PC_RACE)
			{
				sprintf (race, "%-7s", pc_race_table[wch->race].who_name);
			}
			else
			{
				sprintf (race, "       ");
			}
			if (is_clan(wch))
			{
				sprintf (clan, "%s", clan_table[wch->clan].who_name);
				sprintf (rank, "%s", clan_rank_table[wch->rank].rank);
			}
			else
			{
				sprintf (clan, "{W  ({yNon-PK{W)  {x");
				sprintf (rank, " ---");
			}

			sprintf (whodesc, "%s%s%s%s %s%s", level, race, class, sex, clan, rank);
		}
		else
		{

			if (is_clan(wch))
			{
				sprintf (clan, "%s", clan_table[wch->clan].who_name);
				sprintf (rank, "%s", clan_rank_table[wch->rank].rank);
			}
			else
			{
				sprintf (clan, "{W  ({yNon-PK{W)  {x");
				sprintf (rank, " ---");
			}
			sprintf (whodesc, "%s%s%s %s%s", level, wch->pcdata->who_descr, sex, clan, rank);
		}


		/*
		 * Format it up.
		 */
		if (!wch->on_quest)
		{
			questdat[0] = '\0';
		}
		else
		{
			sprintf (questdat, "[{GQ{x] ");
		}

                sprintf (buf, "[%s] %s%s%s%s%s%s%s%s%s%s%s%s%s%s{x\n\r",
				 whodesc,
				 questdat,
				 ((wch->ghost_level >= LEVEL_HERO) && (ch->level >= wch->level)) ? "(Ghost) " : "",
				 wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
				 wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
                                 IS_SET (wch->plyr, PLAYER_GHOST) ? "({7G{wH{7O{wS{7T{x) " : "",
				 IS_SET (wch->comm, COMM_QUIET) ? "({@Quiet{x)" : "",
				 IS_SET (wch->comm, COMM_AFK) ? "[{yAFK{x] " : "",
				 IS_SET (wch->act, PLR_TWIT) ? "({RTWIT{x) " : "",
				 IS_SET (wch->plyr, PLAYER_NEWBIE) ? "{Y{{{Rn00b{Y}{x " : "",
				 "",
                                 is_enemy(ch, wch) ? "(ENEMY)" : "",
				 wch->pcdata->pretit,
				 wch->name,
				 IS_NPC (wch) ? "" : wch->pcdata->title);

                if(wch->pcdata->tier == 1)
                add_buf (ftier, buf);
                else if(wch->pcdata->tier == 2)
                add_buf (stier, buf);
                else if(wch->pcdata->tier == 3)
                add_buf (ttier, buf);
                else
		add_buf (output, buf);

		if ((wch->played/3600) > most_hours)
		{
			mplayer_name = wch->name;
			most_hours = (wch->played/3600);
		}
	}

	count += countimm;
	hour = (int) (struct tm *) localtime (&current_time)->tm_hour;
	if (hour < 12)
	{
		if (is_pm)
		{
			is_pm = FALSE;
			/* Wipe Most-on today count at midnight */
		                max_on = 0;
        		        fp = fopen(MAX_FILE,"w");
        		        fprintf(fp, "%d", max_on);
                		fclose(fp);
			//expire_notes ();
			expire_banks ();
			randomize_entrances (ROOM_VNUM_CLANS);
		}
	}
	else
	{
		is_pm = TRUE;
	}

	if ((fp = fopen(MOST_FILE,"r")) != NULL)
 	{
		most_ever = fread_number(fp);
		fclose(fp);
	}
	if ((fp = fopen(MAX_FILE,"r")) != NULL)
	{
		max_on = fread_number(fp);
		fclose(fp);
	}

        if ( count > max_on )
        {
                max_on = UMAX (count, max_on);
                fp = fopen(MAX_FILE,"w");
                fprintf(fp, "%d", max_on);
                fclose(fp);
        }

	if ( max_on > most_ever )
	{
		most_ever = UMAX (max_on, most_ever);
		fp = fopen(MOST_FILE,"w");
		fprintf(fp, "%d", most_ever);
		fclose(fp);
	}


	if (global_quest)
	{
		sprintf (buf2, "\n\r{GThe global quest flag is on.{x");
		add_buf (output, buf2);
	}
	if (dbl_exp )
	{
		sprintf (buf2, "\n\r{YDouble EXP is on.{x");
		add_buf (output, buf2);
	}
        
        if (dbl_exp_lvl )
        {
                sprintf (buf2, "\n\r{YDouble EXP is on for those under 101.{x");
                add_buf (output, buf2);
        }

        if (timed_dbl_exp)
        {
                sprintf (buf2, "\n\r{YDouble EXP is on a timer.{x");
                add_buf (output, buf2);
        }

	if (nMatch != count)
	{
		sprintf (buf2, "\n\r{BMatches found: {W%d{x\n\r", nMatch);
		add_buf (output, buf2);
	}
	else
	{
		sprintf (buf2, "\n\r{BPlayers found: {W%d   {BMost on today: {W%d   {BMost on ever: {W%d{x\n\r", 
		 count, max_on, most_ever);
		add_buf (output, buf2);
	   if (countimm >= 1)
	   {
		sprintf (buf2, "{BImmortal - Most Hours: {W%s {B- {W%d {BHours.{x\n\r", 
		 immplayer_name, immost_hours);
                add_buf (output, buf2); 
	   }
		sprintf (buf2, "{BMortal - Most Hours: {W%s {B- {W%d {BHours.{x\n\r", 
		 mplayer_name, most_hours);
                add_buf (output, buf2); 
	}

	if (countimm >= 1)
	{

                add_buf(ftier, buf_string(stier));
                add_buf(ftier, buf_string(ttier));
                add_buf(ftier, buf_string(output));
                add_buf(outputimm, buf_string (ftier));
		page_to_char (buf_string (outputimm), ch);
	}
	else
	{
                add_buf(ftier, buf_string(stier));
                add_buf(ftier, buf_string(ttier));
                add_buf(ftier, buf_string(output));
                page_to_char (buf_string (ftier), ch);
	}
	free_buf (output);
	free_buf (outputimm);
        free_buf (ftier);
        free_buf (stier);
        free_buf (ttier);
	return;
}

void 
do_count (CHAR_DATA * ch, char *argument)
{
        FILE *fp;
	int count;
	int hour;
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	count = 0;
	hour = (int) (struct tm *) localtime (&current_time)->tm_hour;
	if (hour < 12)
	{
		if (is_pm)
		{
			is_pm = FALSE;
                        /* Wipe Most-on today count at midnight */
                                max_on = 0;
                                fp = fopen(MAX_FILE,"w");
                                fprintf(fp, "%d", max_on);
                                fclose(fp);
			//expire_notes ();
			expire_banks ();
			randomize_entrances (ROOM_VNUM_CLANS);
		}
	}
	else
	{
		is_pm = TRUE;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING && can_see (ch, d->character))
			count++;

        if ( count > max_on )
        {
                max_on = UMAX (count, max_on);
                fp = fopen(MAX_FILE,"w");
                fprintf(fp, "%d", max_on);
                fclose(fp);
        }

	if (max_on == count)
		sprintf (buf, "{BThere are {W%d {Bcharacters on, the most so far today.{x\n\r",
				 count);
	else
		sprintf (buf, "{BThere are {W%d {Bcharacters on, the most on today was {W%d{x.\n\r",
				 count, max_on);

	send_to_char (buf, ch);
}

void 
do_inventory (CHAR_DATA * ch, char *argument)
{
	BUFFER *final = show_list_to_char (ch->carrying, ch, TRUE, TRUE);
	send_to_char ("You are carrying:\n\r", ch);
	page_to_char( final->string, ch );
	free_buf( final );
	return;
}



void 
do_equipment (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	int iWear;
	int oWear;
	bool found;

        send_to_char ("You are using:\n\r", ch);
	found = FALSE;

	for (oWear = 0; oWear < MAX_WEAR; oWear++)
	{
		iWear = where_order[oWear];

		if ((obj = get_eq_char (ch, iWear)) == NULL)
		{
			continue;
		}

		send_to_char (where_name[iWear], ch);
		if (can_see_obj (ch, obj))
		{
			send_to_char (format_obj_to_char (obj, ch, TRUE), ch);
			send_to_char ("\n\r", ch);
		}
		else
		{
			send_to_char ("something.\n\r", ch);
		}
		found = TRUE;
	}

	if (!found)
		send_to_char ("Nothing.\n\r", ch);

	return;
}



void do_compare (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	float value1;
	float value2;
	char *msg;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	if (arg1[0] == '\0')
	{
		send_to_char ("Compare what to what?\n\r", ch);
		return;
	}

	if ((obj1 = get_obj_carry (ch, arg1)) == NULL)
	{
		send_to_char ("You do not have that item.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0')
	{
		for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
		{
			if (obj2->wear_loc != WEAR_NONE
				&& can_see_obj (ch, obj2)
				&& (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
				break;
		}

		if (obj2 == NULL)
		{
			send_to_char ("You aren't wearing anything comparable.\n\r", ch);
			return;
		}
	}

	else if ((obj2 = get_obj_carry (ch, arg2)) == NULL)
	{
		send_to_char ("You do not have that item.\n\r", ch);
		return;
	}

	msg = NULL;
	value1 = 0;
	value2 = 0;

        
        


	if (obj1 == obj2)
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else
	{
		value1 = obj_balance( obj1->pIndexData );
		value2 = obj_balance( obj2->pIndexData );
	}

	if (msg == NULL)
	{
		if (value2 == 0)
			value2 = 1;

		if (value1 == value2)
			msg = "$p and $P look about the same.";
		else if (value1 > value2)
			msg = "$p looks better than $P.";
		else
			msg = "$p looks worse than $P.";
	}

	act (msg, ch, obj1, obj2, TO_CHAR);
	return;
 }



void 
do_credits (CHAR_DATA * ch, char *argument)
{
	do_help (ch, "diku");
	return;
}



void do_where (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	one_argument (argument, arg);
	if (arg[0] == '\0')
	{
		sprintf (buf, "Players near you in %s:\n\r", ch->in_room->area->name);
		send_to_char (buf, ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next)
		{
			char name_buf[MIL];
			if (d->connected == CON_PLAYING
				&& (victim = d->character) != NULL
				&& !IS_NPC (victim)
				&& victim->in_room != NULL
				&& !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
				&& (is_room_owner (ch, victim->in_room)
					|| !room_is_private (ch, victim->in_room))
				&& victim->in_room->area == ch->in_room->area
				&& get_trust (ch) >= victim->ghost_level
				&& can_see (ch, victim)
                                && victim->level < 180 )
			{
				found = TRUE;
                         if (is_affected(victim,gsn_conceal))
                         {
                           if (!is_same_clan(ch,victim) && !IS_IMMORTAL(ch) && number_percent() > 25)
                           sprintf( buf, "%-28s %s\n\r", get_pers(victim,NULL),victim->in_room->name );
                           else /* (IS_IMMORTAL(ch)) */
                           sprintf( buf, "(%s) %-28s %s\n\r", get_name(victim,NULL, name_buf), victim->name,victim->in_room->name);
                           send_to_char (buf, ch);
                         }
                         else
                         {
                           sprintf (buf, "%-28s %s\n\r", victim->name, victim->in_room->name);
                           send_to_char (buf, ch);
                         }

			}
		}
		if (!found)
			send_to_char ("None\n\r", ch);
	}
	else
	{
		found = FALSE;
		for (victim = char_list; victim != NULL; victim = victim->next)
		{
			if (victim->in_room != NULL
				&& victim->in_room->area == ch->in_room->area
/*				&& !IS_AFFECTED (victim, AFF_HIDE)
				&& !IS_AFFECTED (victim, AFF_SNEAK)		*/
				&& get_trust (ch) >= victim->ghost_level
				&& !IS_SET (victim->in_room->room_flags, ROOM_NOWHERE)
                                && can_see (ch, victim)
                                && victim->level < 180 )
			{
				found = TRUE;
                           if(is_affected(victim,gsn_conceal) && number_percent() > 25 && (!str_prefix(arg,"green")
                           || !str_prefix(arg,"cloaked")))
                           {
                                sprintf( buf, "%-28s %s\n\r", get_pers(victim,NULL),victim->in_room->name );
				send_to_char (buf, ch);
				break;
                           }
                           else if (is_name (arg, victim->name) && !is_affected(victim,gsn_conceal))
                           {
				sprintf (buf, "%-28s %s\n\r", //MORPH BREE WHERE
						 PERS (victim, ch), victim->in_room->name);
				send_to_char (buf, ch);
				break;
                           }
			}
		}
		if (!found)
			act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
	}

	return;
}

void 
do_track (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *in_room;
	sh_int track_vnum;
	int door, move, chance, track;

	one_argument (argument, arg);

	if ((chance = get_skill (ch, gsn_track)) == 0)
	{
		send_to_char ("You don't know how to track.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char ("Track whom?\n\r", ch);
		return;
	}

	in_room = ch->in_room;
	track_vnum = in_room->vnum;
	move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)];
	if (ch->move < move)
	{
		send_to_char ("You are too exhausted.\n\r", ch);
		return;
	}

	if (number_percent () < (100 - chance))
	{
		sprintf (buf, "You can find no recent tracks for %s.\n\r", arg);
		send_to_char (buf, ch);
		check_improve (ch, gsn_track, FALSE, 1);
		WAIT_STATE (ch, 1);
		ch->move -= move / 2;
		return;
	}

	for (victim = char_list; victim != NULL; victim = victim->next)
	{
		if (victim->in_room != NULL
			&& !IS_NPC (victim)
			&& can_see (ch, victim)
			&& is_name (arg, victim->name))
		{
			if (victim->in_room->vnum == track_vnum)
			{
				act ("The tracks end right under $S feet.", ch, NULL, victim, TO_CHAR);
				return;
			}
			for (track = 0; track < MAX_TRACK; track++)
			{
				if (victim->track_from[track] == track_vnum)
				{
					for (door = 0; door < 12; door++)
					{
						if ((pexit = in_room->exit[door]) != NULL)
						{
							if (pexit->u1.to_room->vnum == victim->track_to[track])
							{
								sprintf (buf, "Some tracks lead off to the %s.\n\r", dir_name[door]);
								send_to_char (buf, ch);
								check_improve (ch, gsn_track, TRUE, 1);
								WAIT_STATE (ch, 1);
								ch->move -= move;
								return;
							}
						}
					}
					act ("$N seems to have vanished here.", ch, NULL, victim, TO_CHAR);
					check_improve (ch, gsn_track, TRUE, 1);
					WAIT_STATE (ch, 1);
					ch->move -= move;
					return;
				}
			}
			act ("You can find no recent tracks for $N.", ch, NULL, victim, TO_CHAR);
			check_improve (ch, gsn_track, FALSE, 1);
			WAIT_STATE (ch, 1);
			ch->move -= move / 2;
			return;
		}
	}
	for (victim = char_list; victim != NULL; victim = victim->next)
	{
		if ((victim->in_room != NULL)
			&& IS_NPC (victim)
			&& can_see (ch, victim)
			&& (victim->in_room->area == ch->in_room->area)
			&& is_name (arg, victim->name))
		{
			if (victim->in_room->vnum == track_vnum)
			{
				act ("The tracks end right under $S feet.", ch, NULL, victim, TO_CHAR);
				return;
			}
			for (track = 0; track < MAX_TRACK; track++)
			{
				if (victim->track_from[track] == track_vnum)
				{
					for (door = 0; door < 12; door++)
					{
						if ((pexit = in_room->exit[door]) != NULL)
						{
							if (pexit->u1.to_room->vnum == victim->track_to[track])
							{
								sprintf (buf, "Some tracks lead off to the %s.\n\r", dir_name[door]);
								send_to_char (buf, ch);
								check_improve (ch, gsn_track, TRUE, 1);
								WAIT_STATE (ch, 1);
								ch->move -= move;
								return;
							}
						}
					}
					act ("$N seems to have vanished here.", ch, NULL, victim, TO_CHAR);
					check_improve (ch, gsn_track, TRUE, 1);
					WAIT_STATE (ch, 1);
					ch->move -= move;
					return;
				}
			}
		}
	}
	sprintf (buf, "You can find no recent tracks for %s.\n\r", arg);
	send_to_char (buf, ch);
	check_improve (ch, gsn_track, FALSE, 1);
	WAIT_STATE (ch, 1);
	ch->move -= move / 2;
	return;
}

void 
do_consider (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int lvldiff, hpdiff, manadiff;

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Consider killing whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They're not here.\n\r", ch);
		return;
	}

	if (is_safe (ch, victim))
	{
		send_to_char ("Don't even think about it.\n\r", ch);
		return;
	}

	msg = "$N";
	act (msg, ch, NULL, victim, TO_CHAR);
	send_to_char ("----------\n\r", ch);

	lvldiff = victim->level - ch->level;

	send_to_char ("Level: ", ch);

	if (lvldiff <= 0)
		msg = "you wouldn't waste your time with them.";
	else if (lvldiff <= 5)
		msg = "$N is no match for you.";
	else if (lvldiff <= 10)
		msg = "$N cowers in fear from you.";
	else if (lvldiff <= 15)
		msg = "$N smiles nervously at you.";
	else if (lvldiff == 20)
		msg = "the perfect match!";
	else if (lvldiff <= 25)
                msg = "$N says '{_Do you feel lucky, punk?{x'.";
	else if (lvldiff <= 30)
		msg = "$N backs you into a corner.";
	else if (lvldiff <= 35)
		msg = "$N says ' Bring a friend'";
	else if (lvldiff >= 50)
		msg = "eats players like you for breakfast.";
	else
		msg = "this is not supposed to be seen.";

	act (msg, ch, NULL, victim, TO_CHAR);

	hpdiff = (ch->hit - victim->hit);

	send_to_char ("Health:", ch);

	if (hpdiff >= 150)
		msg = " you are currently much healthier than $E.";
	else if (hpdiff >= 50)
		msg = " you are currently healthier than $E.";
	else if (hpdiff >= 25)
		msg = " you are currently slightly healthier than $E.";
	else if (hpdiff >= 1)
		msg = " you are a teensy bit healthier than $E.";
	else if (hpdiff == 0)
		msg = " $E is a perfect match for you.";
	else if (hpdiff <= -1)
		msg = " $E is a teensy bit healthier than you.";
	else if (hpdiff <= -25)
		msg = " $E is slightly healthier than you.";
	else if (hpdiff <= -50)
		msg = " $E is healthier than you.";
	else if (hpdiff <= -150)
		msg = " $E is much healthier than you.";
	else
		msg = " this is not supposed to be seen.";

	act (msg, ch, NULL, victim, TO_CHAR);

	manadiff = (ch->mana - victim->mana);

	send_to_char ("Magic:", ch);

	if (manadiff >= 150)
		msg = " you are currently much more powerful than $E.";
	else if (manadiff >= 50)
		msg = " you are currently more powerful than $E.";
	else if (manadiff >= 25)
		msg = " you are currently slightly more powerful than $E.";
	else if (manadiff >= 1)
		msg = " you are a teensy bit more powerful than $E.";
	else if (manadiff == 0)
		msg = " $E is a perfect match for you.";
	else if (manadiff <= -1)
		msg = " $E is a teensy bit more powerful than you.";
	else if (manadiff <= -25)
		msg = " $E is slightly more powerful than you.";
	else if (manadiff <= -50)
		msg = " $E is more powerful than you.";
	else if (manadiff <= -150)
		msg = " $E is much more powerful than you.";
	else
		msg = " this is not supposed to be seen.";

	act (msg, ch, NULL, victim, TO_CHAR);

	return;
}

void 
set_title (CHAR_DATA * ch, char *title)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC (ch))
	{
		bug ("Set_title: NPC.", 0);
		return;
	}

	if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?')
	{
		buf[0] = ' ';
		strcpy (buf + 1, title);
	}
	else
	{
		strcpy (buf, title);
	}

	free_string (ch->pcdata->title);
	ch->pcdata->title = str_dup (buf);
	ch->pcdata->usr_ttl = TRUE;
	return;
}



void do_title (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (IS_SET (ch->act, PLR_NOTITLE))
		return;

	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
		&& (!IS_IMMORTAL (ch)))
	{
		send_to_char ("Just keep your nose in the corner like a good little player.\n\r", ch);
		return;
	}

        if (argument[0] == '{' && argument[1] == '\0')
                return;

	if (argument[0] == '\0')
	{
		send_to_char ("Change your title to what?\n\r", ch);
		return;
	}

	if (strlen(argument) > 45)
		total_length(argument,45);

	smash_tilde (argument);
	set_title (ch, argument);
	send_to_char ("Ok.\n\r", ch);
}



void 
do_description (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] != '\0')
	{
		buf[0] = '\0';
		smash_tilde (argument);

		if (argument[0] == '-')
		{
			int len;
			bool found = FALSE;

			if (ch->description == NULL || ch->description[0] == '\0')
			{
				send_to_char ("No lines left to remove.\n\r", ch);
				return;
			}

			strcpy (buf, ch->description);

			for (len = strlen (buf); len > 0; len--)
			{
				if (buf[len] == '\r')
				{
					if (!found)	/* back it up */
					{
						if (len > 0)
							len--;
						found = TRUE;
					}
					else
						/* found the second one */
					{
						buf[len + 1] = '\0';
						free_string (ch->description);
						ch->description = str_dup (buf);
						send_to_char ("Your description is:\n\r", ch);
						send_to_char (ch->description ? ch->description :
									  "(None).\n\r", ch);
						return;
					}
				}
			}
			buf[0] = '\0';
			free_string (ch->description);
			ch->description = str_dup (buf);
			send_to_char ("Description cleared.\n\r", ch);
			return;
		}
		if (argument[0] == '+')
		{
			if (ch->description != NULL)
				strcat (buf, ch->description);
			argument++;
			while (isspace (*argument))
				argument++;
		}

		if (strlen (buf) + strlen (argument) >= MAX_STRING_LENGTH - 2)
		{
			send_to_char ("Description too long.\n\r", ch);
			return;
		}

		strcat (buf, argument);
		strcat (buf, "\n\r");
		free_string (ch->description);
		ch->description = str_dup (buf);
	}

	send_to_char ("Your description is:\n\r", ch);
	send_to_char (ch->description ? ch->description : "(None).\n\r", ch);
	return;
}



void 
do_report (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	sprintf (buf,
			 "You say '{SI have %d/%d hp %d/%d mana %d/%d mv %ld xp.{x'\n\r",
			 ch->hit, ch->max_hit,
			 ch->mana, ch->max_mana,
			 ch->move, ch->max_move,
			 ch->exp);

	send_to_char (buf, ch);

	sprintf (buf, "$n says '{SI have %d/%d hp %d/%d mana %d/%d mv %ld xp.{x'",
			 ch->hit, ch->max_hit,
			 ch->mana, ch->max_mana,
			 ch->move, ch->max_move,
			 ch->exp);

	act (buf, ch, NULL, NULL, TO_ROOM);

	return;
}



void do_practice (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int sn;

	if (IS_NPC (ch))
		return;

	if (argument[0] == '\0')
	{
		int col;

		col = 0;
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
                        if (ch->level < ch->pcdata->learnlvl[sn]
				|| ch->pcdata->learned[sn] < 1 /* skill is not known */ )
				continue;

                        sprintf (buf, "{&%-18s{x {$%3d%%{x  ",
					 skill_table[sn].name, ch->pcdata->learned[sn]);
			send_to_char (buf, ch);
			if (++col % 3 == 0)
				send_to_char ("\n\r", ch);
		}

		if (col % 3 != 0)
			send_to_char ("\n\r", ch);

                sprintf (buf, "You have {&%d{x practice sessions left.\n\r",
				 ch->practice);
		send_to_char (buf, ch);
	}
	else
	{
		CHAR_DATA *mob;
		int adept;

		if (!IS_AWAKE (ch))
		{
			send_to_char ("In your dreams, or what?\n\r", ch);
			return;
		}

		for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
		{
			if (IS_NPC (mob) && IS_SET (mob->act, ACT_PRACTICE))
				break;
		}

		if (mob == NULL)
		{
			send_to_char ("You can't do that here.\n\r", ch);
			return;
		}

		if (ch->practice <= 0)
		{
			send_to_char ("You have no practice sessions left.\n\r", ch);
			return;
		}

		if ((sn = find_spell (ch, argument)) < 0
                        || (!IS_NPC (ch)
                                && (ch->level < ch->pcdata->learnlvl[sn]
					|| ch->pcdata->learned[sn] < 1)))
		{
			send_to_char ("You can't practice that.\n\r", ch);
			return;
		}

		adept = IS_NPC (ch) ? 100 : class_table[ch->class].skill_adept;

		if (ch->pcdata->learned[sn] >= adept)
		{
			sprintf (buf, "You are already learned at %s.\n\r",
					 skill_table[sn].name);
			send_to_char (buf, ch);
		}
		else
		{
			ch->practice--;
			ch->pcdata->learned[sn] +=
				int_app[get_curr_stat (ch, STAT_INT)].learn /
				skill_table[sn].rating[ch->class];
			if (ch->pcdata->learned[sn] < adept)
			{
                         sprintf(buf,"You practice $T.{@({&%d{@){x", ch->pcdata->learned[sn]);
                                act (buf, ch, NULL, skill_table[sn].name, TO_CHAR);
                                act ("$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM);
			}
			else
			{
				ch->pcdata->learned[sn] = adept;
				act ("You are now learned at $T.",
					 ch, NULL, skill_table[sn].name, TO_CHAR);
				act ("$n is now learned at $T.",
					 ch, NULL, skill_table[sn].name, TO_ROOM);
			}
		}
	}
	return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void 
do_wimpy (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int wimpy;

	one_argument (argument, arg);

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi (arg);

	if (wimpy < 0)
	{
		send_to_char ("Your courage exceeds your wisdom.\n\r", ch);
		return;
	}

	if (wimpy > ch->max_hit / 2)
	{
		send_to_char ("Such cowardice ill becomes you.\n\r", ch);
		return;
	}

	ch->wimpy = wimpy;
	sprintf (buf, "Wimpy set to %d hit points.\n\r", wimpy);
	send_to_char (buf, ch);
	return;
}



void 
do_password (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;

	if (IS_NPC (ch))
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;
	while (isspace (*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	pArg = arg2;
	while (isspace (*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char ("Syntax: password <old> <new>.\n\r", ch);
		return;
	}

	if (strcmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd))
	{
		WAIT_STATE (ch, 40);
		send_to_char ("Wrong password.  Wait 10 seconds.\n\r", ch);
		return;
	}

	if (strlen (arg2) < 5)
	{
		send_to_char (
			 "New password must be at least five characters long.\n\r", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt (arg2, ch->name);
	for (p = pwdnew; *p != '\0'; p++)
	{
		if (*p == '~')
		{
			send_to_char (
						 "New password not acceptable, try again.\n\r", ch);
			return;
		}
	}

	free_string (ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup (pwdnew);
	save_char_obj (ch);
	send_to_char ("Ok.\n\r", ch);
	return;
}

void do_nscore (CHAR_DATA * ch, char *argument)
{
        char buf[MAX_STRING_LENGTH], buf2[256];
        BUFFER *output = new_buf();
       
	if (IS_NPC (ch))
                sprintf (buf, "\n\r%s\n\r", ch->name);
	else
                sprintf (buf, "\n\r%s%s%s\n\r",ch->pcdata->pretit,ch->name, ch->pcdata->title);
        add_buf(output,buf);

	buf[0] = '\0';
	sprintf (buf, "{y______________________________________________________________________________{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|\\___________________________________________________________________________/|{x\n\r");
        add_buf(output,buf);

	sprintf (buf, "{y||{WLevel{x: %-3d {WAge{x: %-3d  {WRace{x: %-13s {WGender{x: %-6s {WClass{x: %-12s{y||{x\n\r",
			 ch->level, get_age (ch), race_table[ch->race].name, ch->sex == 0 ? "none" : ch->sex == 1 ? "male" : "female", IS_NPC (ch) ? "mobile" : class_table[ch->class].name);
        add_buf(output,buf);
	if (get_trust (ch) > LEVEL_HERO)
	{
		sprintf (buf, "{y||{WTrust{x: %-3d {WHoly Light{x: %-3s {WWizInvis Level{x: %-3d       {WIncognito Level{x: %-3d  {y||{x\n\r",
				 get_trust (ch), IS_SET (ch->act, PLR_HOLYLIGHT) ? "on" : "off", ch->invis_level, ch->incog_level);
                add_buf(output,buf);
	}
	if (!IS_NPC (ch))
	{
		sprintf (buf, "{y||{WPractices{x: %-5d {WTrains{x: %-5d {WExperience{x: %-9ld {WNeeded to level{x: %-5ld{y||{x\n\r",
				 ch->practice, ch->train, ch->exp, IS_HERO (ch) ? (0) : ((ch->level + 1) * exp_per_level (ch, ch->pcdata->points) - ch->exp));
                add_buf(output,buf);
	}
	sprintf (buf, "{y||___________________________________________________________________________||{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|/___________________________________________________________________________\\|{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|\\_______________________________/| |\\_______________________________________/|{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y||{x{WStr{x: {Y%-2d{x({C%2d{x)  {WHit{x: %-2d {WDam{x: %-2d   {y|| ||{x {WHit Points{x: %5d/%5d {WSaves: %5d  {y||\n\r",
			 ch->perm_stat[STAT_STR], get_curr_stat (ch, STAT_STR), str_app[get_curr_stat (ch, STAT_STR)].tohit, str_app[get_curr_stat (ch, STAT_STR)].todam, ch->hit, ch->max_hit, ch->saving_throw);
        add_buf(output,buf);
	sprintf (buf, "{y||{x{WInt{x: {Y%-2d{x({C%2d{x)  {WLearn{x: %-2d%%        {y|| ||{x {WMana{x: %5d/%5d  {WMove{x: %5d/%5d  {y||{x\n\r",
			 ch->perm_stat[STAT_INT], get_curr_stat (ch, STAT_INT), int_app[get_curr_stat (ch, STAT_INT)].learn, ch->mana, ch->max_mana, ch->move, ch->max_move);
        add_buf(output,buf);
	sprintf (buf, "{y||{x{WWis{x: {Y%-2d{x({C%2d{x)  {WPracs{x: %-2d         {y|| ||{x {WHitroll{x: %-3d    {WDamroll{x: %-3d          {y||{x\n\r",
			 ch->perm_stat[STAT_WIS], get_curr_stat (ch, STAT_WIS), wis_app[get_curr_stat (ch, STAT_WIS)].practice, GET_HITROLL (ch), GET_DAMROLL (ch));
        add_buf(output,buf);
	sprintf (buf, "{y||{x{WDex{x: {Y%-2d{x({C%2d{x)  {WACmod{x: %-4d       {y|| ||{x {WACpierce{x: %-5d   {WACbash{x: %-5d       {y||{x\n\r",
			 ch->perm_stat[STAT_DEX], get_curr_stat (ch, STAT_DEX), dex_app[get_curr_stat (ch, STAT_DEX)].defensive, GET_AC (ch, AC_PIERCE), GET_AC (ch, AC_BASH));
        add_buf(output,buf);
	sprintf (buf, "{y||{x{WCon{x: {Y%-2d{x({C%2d{x)  {WHP{x: %-2d  {WSS{x: %-3d%%  {y|| ||{x {WACslash{x: %-8d {WACexotic{x: %-5d     {y||{x\n\r",
			 ch->perm_stat[STAT_CON], get_curr_stat (ch, STAT_CON), con_app[get_curr_stat (ch, STAT_CON)].hitp, con_app[get_curr_stat (ch, STAT_CON)].shock, GET_AC (ch, AC_SLASH), GET_AC (ch, AC_EXOTIC));
        add_buf(output,buf);
	sprintf (buf, "{y||_______________________________|| ||_______________________________________||{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|/_______________________________\\| |/_______________________________________\\|{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|\\___________________________________________________________________________/|{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y||{WPlatinum{x: %-7ld {WGold{x: %-7ld {WSilver{x: %-7ld {WYour Deity{x: %-9s      {y||{x\n\r{y||{WItems{x: %4d/%4d  {WWeight{x: %7ld/%7d   {WAlign{x: %-5d                   {y||{x\n\r",
			 ch->platinum, ch->gold, ch->silver, (god_table[ch->god].name), ch->carry_number, can_carry_n (ch), get_carry_weight (ch) / 10, can_carry_w (ch) / 10, ch->alignment);
        add_buf(output,buf);
	/* if ( ch->alignment >  900 ) send_to_char( "{&O{7di{&n{x's chosen           {x{y||{x\n\r", ch );
	   else if ( ch->alignment >  700 ) send_to_char( "{&O{7di{&n{x's servant          {y||{x\n\r", ch );
	   else if ( ch->alignment >  350 ) send_to_char( "{Ygood{x                    {y||{x\n\r",    ch );
	   else if ( ch->alignment >  100 ) send_to_char( "kind                    {y||{x\n\r",    ch );
	   else if ( ch->alignment > -100 ) send_to_char( "neutral                 {y||{x\n\r", ch );
	   else if ( ch->alignment > -350 ) send_to_char( "mean                    {y||{x\n\r",    ch );
	   else if ( ch->alignment > -700 ) send_to_char( "{Revil                 {x   {y||{x\n\r",    ch );
	   else if ( ch->alignment > -900 ) send_to_char( "{!H{1e{!l{x's servant           {x{y||{x\n\r", ch );
	   else                             send_to_char( "{!H{1e{!l{x's chosen            {x{y||{x\n\r", ch ); */
	sprintf (buf, "{y||{WSize:{x %-9s {WIQuest Points:{x %-8d {WWimpy:{x %-5d {WAQuest Points:{x %-6d {y||{x\n\r", size_table[ch->size].name, ch->qps, ch->wimpy, ch->questpoints);
        add_buf(output,buf);
     if (!IS_NPC(ch))
     {
	if (is_clan (ch))
	{
                sprintf (buf, "{WCLAN:{x %-30s - {WA %-13s {Wclan.                                   {x",
                clan_table[ch->clan].who_name, is_pkill (ch) ? "{RPkill {x" : "{WNon-Pkill {x");
                sprintf (buf2,"{y||%-76s{y||{x\n\r", buf);
                add_buf(output,buf2);
	}
	sprintf (buf, "{y||{WPk Rank:{x %-6d  {WArena Rank:{x %-6d  {WHours: {x%-5d                          {y||{x\n\r",
		ch->pcdata->prank, ch->pcdata->arank, ch->played/3600);
        add_buf(output,buf);
		if (ch->challenger != NULL)
		{
			sprintf (buf, "{y||{x{3[{^Arena{3]{x You have been {rchallenged{x by {b%-10s{x                             {y||{x\n\r",
					 ch->challenger->name);
                        add_buf(output,buf);
		}
		if (ch->challenged != NULL)
		{
			sprintf (buf, "{y||{3[{^Arena{3]{x You have {rchallenged{x {b%-10s{x                                     {y||{x\n\r",
					 ch->challenged->name);
                        add_buf(output,buf);
		}
		if (ch->gladiator != NULL)
		{
			sprintf (buf, "{y||{3[{{^Arena Bet{3]{x You have a {y%-8d{x gold bet on {b%-10s{x             {y||\n\r",
					 ch->pcdata->plr_wager, ch->gladiator->name);
                        add_buf(output,buf);
		}
     }
	if (!IS_NPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	{
		sprintf (buf, "{y|| {xYou are {ydrunk.{x                                                            {y||{x\n\r");
                add_buf(output,buf);
	}
	if (!IS_NPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
	{
		sprintf (buf, "{y|| {xYou are {^thirsty.{x                                                          {y||{x\n\r");
                add_buf(output,buf);
	}
	if (!IS_NPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
	{
		sprintf (buf, "{y|| {xYou are {#hungry.{x                                                           {y||{x\n\r");
                add_buf(output,buf);
	}
        if (IS_SET(ch->plyr,PLAYER_GHOST) && ch->ghost_timer > 0)
        {
                sprintf(buf,"{y||  {xYou are a {7G{wH{7O{wS{7T{x for %d more ticks.                                 {y||{x\n\r",
                ch->ghost_timer);
                add_buf(output,buf);
        }
        if (ch->fight_timer > 0)
        {
                sprintf(buf,"{y|| {xYou have ({R%2d{x) ticks left on your fight timer.                             {y||{x\n\r",
                ch->fight_timer);
                add_buf(output,buf);
        }


	switch (ch->position)
	{
	case POS_DEAD:
		sprintf (buf, "{y||{x You are {z{!DEAD!!{x                                                            {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_MORTAL:
		sprintf (buf, "{y||{x You are {%mortally wounded!!{x                                                {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_INCAP:
		sprintf (buf, "{y||{x You are {1incapacitated.{x                                                    {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_STUNNED:
		sprintf (buf, "{y||{x You are {@stunned.{x                                                          {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_SLEEPING:
		sprintf (buf, "{y||{x You are {$sleeping.{x                                                         {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_RESTING:
		sprintf (buf, "{y||{x You are {4resting.{x                                                          {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_STANDING:
		sprintf (buf, "{y||{x You are {5standing.{x                                                         {y||{x\n\r");
                add_buf(output,buf);
		break;
	case POS_FIGHTING:
		sprintf (buf, "{y||{x You are {&fighting.{x                                                         {y||{x\n\r");
                add_buf(output,buf);
		break;
	}
	
	sprintf (buf, "{y||{WHp Regen Rate: {R%-3d{x {W:: Mana Regen Rate: {R%-3d{x                                 {y||{x\n\r",
	    ch->regen_rate, ch->mana_regen_rate);
	add_buf(output,buf);

	sprintf (buf, "{y||___________________________________________________________________________||{x\n\r");
        add_buf(output,buf);
	sprintf (buf, "{y|/___________________________________________________________________________\\|{x\n\r");
        add_buf(output,buf);
        page_to_char (buf_string (output), ch);
	if (IS_SET (ch->comm, COMM_SHOW_AFFECTS))
		do_affects (ch, "");
}

void do_gods (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int god;
	int blank;

	blank = 0;
	if (IS_NPC (ch))
		return;
	send_to_char ("      ", ch);
	for (god = 0; god < MAX_GOD; god++)
	{
		if ((god_table[god].pc_evil && ch->alignment <= -200)
			|| (god_table[god].pc_neutral && ch->alignment == 0)
			|| (god_table[god].pc_good && ch->alignment >= 200))
		{
			if ((god_table[god].pc_mage && ch->class == 0)
				|| (god_table[god].pc_cleric && ch->class == 1)
				|| (god_table[god].pc_thief && ch->class == 2)
				|| (god_table[god].pc_warrior && ch->class == 3)
				|| (god_table[god].pc_ranger && ch->class == 4)
				|| (god_table[god].pc_druid && ch->class == 5)
				|| (god_table[god].pc_vampire && ch->class == 6)
				|| (god_table[god].pc_wizard && ch->class == 7)
				|| (god_table[god].pc_priest && ch->class == 8)
				|| (god_table[god].pc_mercenary && ch->class == 9)
				|| (god_table[god].pc_gladiator && ch->class == 10)
				|| (god_table[god].pc_strider && ch->class == 11)
				|| (god_table[god].pc_sage && ch->class == 12)
				|| (god_table[god].pc_lich && ch->class == 13)
				|| (god_table[god].pc_forsaken && ch->class == 14)
				|| (god_table[god].pc_conjurer && ch->class == 15)
				|| (god_table[god].pc_archmage && ch->class == 16)
				|| (god_table[god].pc_voodan && ch->class == 17)
				|| (god_table[god].pc_monk && ch->class == 18)
				|| (god_table[god].pc_saint && ch->class == 19)
				|| (god_table[god].pc_assassin && ch->class == 20)
				|| (god_table[god].pc_ninja && ch->class == 21)
				|| (god_table[god].pc_bard && ch->class == 22)
				|| (god_table[god].pc_highlander && ch->class == 23)
				|| (god_table[god].pc_swashbuckler && ch->class == 24)
				|| (god_table[god].pc_crusader && ch->class == 25)
				|| (god_table[god].pc_darkpaladin && ch->class == 26)
				|| (god_table[god].pc_hunter && ch->class == 27)
				|| (god_table[god].pc_paladin && ch->class == 28)
				|| (god_table[god].pc_warlock && ch->class == 29)
				|| (god_table[god].pc_alchemist && ch->class == 30)
				|| (god_table[god].pc_shaman && ch->class == 31)
				|| (god_table[god].pc_fade && ch->class == 32)
				|| (god_table[god].pc_banshee && ch->class == 33))
			{
				sprintf (buf, "%-10s     ", god_table[god].name);
				send_to_char (buf, ch);
			}
			if (++blank % 3 == 0)
				send_to_char ("\n\r      ", ch);

		}
	}
	return;

}

void 
do_identify (CHAR_DATA * ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;

	if ((obj = get_obj_carry (ch, argument)) == NULL)
	{
		send_to_char ("You are not carrying that.\n\r", ch);
		return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC (rch) && rch->pIndexData->vnum == MOB_VNUM_SAGE)
			break;

	if (!rch)
	{
		send_to_char ("No one here seems to know much about that.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL (ch))
		act ("$n cackles and comps you!\n\r", rch, obj, ch, TO_VICT);
	else if (ch->gold < obj->level * 100 + 50 && ch->platinum < obj->level * .25)
	{
		act ("$n resumes tossing bones without looking at $p.",
			 rch, obj, 0, TO_ROOM);
		return;
	}
	else if (ch->gold > obj->level * 100 + 50)
	{
		ch->gold -= obj->level * 100 + 50;
		send_to_char ("Your purse feels lighter.\n\r", ch);
	}
	else if (ch->platinum > obj->level * .25)
	{
		ch->platinum -= obj->level * .25;
		send_to_char ("Your purse feels lighter.\n\r", ch);
	}


	act ("$n fondles $p and tosses some chicken bones to the ground.",
		 rch, obj, 0, TO_ROOM);
	spell_identify (100, 0, ch, obj, 0);
}

void do_subdue(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
	return;

    if(IS_SET(ch->plyr, PLAYER_SUBDUE))
    {
        send_to_char("You will now Kill your victims.\n\r",ch);
        REMOVE_BIT(ch->plyr,PLAYER_SUBDUE);
    }
    else
    {
        send_to_char("You will now subdue your victims.\n\r",ch);
        SET_BIT(ch->plyr,PLAYER_SUBDUE);
    }
}

/* Morph Stuff */
char * get_descr_form(CHAR_DATA *ch,CHAR_DATA *looker, bool get_long)
{
    struct l_form
    {
        char *short_descr;
        char *long_descr;
    };
    char buf[MAX_STRING_LENGTH];
    char *buf2;

    static const struct l_form look_form[] =
    {
    {" ", " "},
    {"a {Dg{wr{We{Dy{X {Dshadow{X", "A {Dg{wr{We{wy{X {Dshadow{X is standing here. \n\r"}, //booya
    {"normal","A huge bat is flying here.\n\r"},
    {"normal","A large grey wolf stands here.\n\r"},
    {"normal","A large, mean looking bear is standing here.\n\r"},
    {"a red dragon","A red dragon is here breathing plumes of smoke.\n\r"},
    {"a black dragon","A black dragon is here spitting burning acids.\n\r"},
    {"a green dragon","A green dragon is here breathing noxious poison.\n\r"},
    {"a white dragon","A white dragon sends out blasts of freezing air at you.\n\r"},
    {"a gold dragon","A gold dragon gazes down at you here.\n\r"},
    {"normal","An odd-looking person stands here.\n\r"},
    {"A giant wolf", "A giant wolf is here, growling.\n\r"},
    {"a gaseous cloud","A cloud of boiling gasses is here.\n\r"},
    {"a cloaked figure", "A cloaked figure stands here.\n\r"},
    {"a shrouded figure", "A shrouded figure stands here.\n\r"},
    {"a cloaked figure", "A cloaked figure stands here.\n\r"},
    {"a goat headed", "A goat headed person stands here.\n\r"},
    {"a lion headed","A lion headed person stands here.\n\r"},
    {"a dragon headed", "A dragon headed person stands here.\n\r"},
    {"",""}
    };

    buf[0] = '\0';
    buf2 = "";

    if (get_long)
    {
        if(ch->morph_form[0] == 0)
         return ch->long_descr;
        else if (ch->morph_form[0] == MORPH_MORPH)
        {
         return ch->long_descr;
        }
        else if (ch->morph_form[0] != MORPH_MORPH)
            return look_form[ch->morph_form[0]].long_descr;
        else
           return ch->long_descr;
    }
    else
    {
        if (ch->morph_form[0] == 0)
        {
            return PERS(ch, looker);
        }
        else
	{
         if (ch->morph_form[0] >= 2 && ch->morph_form[0] <= 4)
         {
	    return PERS(ch, looker);
         }
         else
         {
          if (ch->morph_form[0] == MORPH_CONCEAL)
           return look_form[ch->morph_form[0]].short_descr;
          else if (ch->morph_form[0] >= MORPH_RED && ch->morph_form[0] <= MORPH_GOLD)
           return look_form[ch->morph_form[0]].short_descr;
          else if(ch->morph_form[0] == MORPH_MORPH)
           return ch->morph;

         else if (ch->sex == SEX_MALE)
         sprintf(buf,"%s %s",look_form[ch->morph_form[0]].short_descr,"man");
         else if (ch->sex == SEX_FEMALE)
         sprintf(buf,"%s %s",look_form[ch->morph_form[0]].short_descr,"woman");
         else 
         sprintf(buf,"%s %s",look_form[ch->morph_form[0]].short_descr,"creature");

         buf2 = buf;
         return buf2;
        }
       }
    }
    if (get_long)
        return ch->long_descr;

    return PERS(ch,looker);
}

/*
void update_ranklist(CHAR_DATA *ch)
{
   #define RANK_SHOW  20
   ARANK_DATA *arank;
   PRANK_DATA *prank;

   Linear Sort 
   for (count = 0; count < RANK_SHOW; count++)
   {
	x = count;
	for (index = count + 1; index < RANK_SHOW; index++)
           if (data[index] > data[x])
		x = index;

	if (x > count)
	   swap(data[x], data[count]);
   }

}
*/

void do_unghost(CHAR_DATA *ch)
{
	if (IS_NPC (ch))
		return;

	if (!IS_SET(ch->plyr,PLAYER_GHOST))
	{
		send_to_char("You aren't a ghost.\n\r",ch);
		return;
	}
	else
	{
		ch->ghost_timer = 0;
	        REMOVE_BIT(ch->plyr,PLAYER_GHOST);
         	act("You feel more real.",ch,NULL,NULL,TO_CHAR);
         	act("$n fades into reality.",ch,NULL,NULL,TO_ROOM);
        }
}

/*void do_sunder( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) || ch->class != CLASS_CRUSADER )
    {
        send_to_char("{wIf you want sunder, then go Crusader!\n\r{x",ch);        
        return;
    }

    if ( IS_SET(ch->ability_crusader, ABILITY_CRUSADER_SUNDER ) )
    {
       send_to_char( "{cYou will no longer have the ability to {wsunder{c your opponent.{x\n\r",ch);
       REMOVE_BIT( ch->ability_crusader, ABILITY_CRUSADER_SUNDER );
    }
    else
    {
       if ( ch->mana < ch->level/2 )
       {
          send_to_char("{rYou do not have enough money to turn on this ability!{x\n\r",ch);
          return;
       }
       
       send_to_char( "{mYou will now have the ability to {wsunder{m your opponent.{x\n\r",ch);
       SET_BIT( ch->ability_crusader, ABILITY_CRUSADER_SUNDER );
       ch->mana -= ch->mana / 4;
    }
} */
void do_nightstalker( CHAR_DATA *ch, char *argument )
{
   int count;

   if ( ( ch->class == CLASS_SHADE || ch->class == CLASS_LICH || ch->class == CLASS_FADE || ch->class == CLASS_BANSHEE) && !IS_NPC(ch) )
      count = 1;
   else
   {
      send_to_char("{wYou can already don't get harmed by sunlight!\n\r{x",ch);
      return;
   }
  
   if ( IS_SET( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER ) )
   {
      if ( time_info.hour >= 16 && time_info.hour <= 18 )
      { 
         send_to_char( "{cYou will no longer feel the harmful effects of sunlight, or feel the power of the night.{x\n\r",ch);
         REMOVE_BIT( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER );
      }
      else
      {
         send_to_char( "{rYou must wait until the dusk of the evening to relinquish the {DNightstalker{r!{x\n\r",ch);
         return;
      }
   }
   else
   {
      if ( time_info.hour >= 4 && time_info.hour <= 6)
      {
         send_to_char( "{mYou can now bask in the power of the night, but dread the harmful ray of sunlight!{x\n\r",ch);
         SET_BIT( ch->ability_lich, ABILITY_LICH_NIGHTSTALKER );
         ch->mana = ch->mana/2;
         WAIT_STATE(ch,48);
         act("{R$n {renters a deep trance as they are consumed by the {DNightstalker{r!{x",ch,NULL,NULL,TO_ROOM);
      }
      else
      {
         send_to_char( "{rYou must wait until the dawn of a new day to decide to invoke the {DNightstalker{r!{x\n\r",ch);
         return;
      }
   }
}
//VoOdan sense vitality by fesdor, works with sense vitality spell.
void do_sense( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char hp[MAX_STRING_LENGTH];
    char mana[MSL];
    char move[MSL];
    char buf[MSL];
    int temp = 0;

    if (ch->class != CLASS_VOODAN || get_skill(ch,gsn_sense) == 0)
    	{
	    	send_to_char("You do not know how to sense vitals.\n\r",ch);
	    	return;
    	}
    
    if ( argument[0] == '\0' )
    {
	    send_to_char("Sense who?\n\r",ch);
	    return;
    }
    victim = get_char_world( ch, argument );


    if ( victim == NULL || victim->level > LEVEL_HERO )
    {
        send_to_char("You Failed.\n\r",ch);
        return;
    }

    if (saves_spell(ch->level + 5,victim,DAM_OTHER))
    {
     send_to_char("You Failed.\n\r",ch);
     return;
    }
    
    if ( !is_affected(ch, gsn_sense) )
    {
        send_to_char("You are not sensing anyone right now!\n\r",ch);
        return;
    }
    else if ( !is_affected(victim, gsn_sense) )
    {
        send_to_char("You are not attuned to that player's vitals!\n\r",ch);
        return;
    }
    else
    {
	BUFFER *final;
	final = new_buf();
	if (victim->max_move > 0 )
            temp = ( 100 * victim->move ) / victim->max_move;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( move, "{G%d{x", temp); }
            else if ( temp >= 34 )
              { sprintf( move, "{Y%d{x", temp); }
            else if ( temp <= 33 )
              { sprintf( move, "{R%d{x", temp); }
             else
                sprintf(move,"{R%d{x",temp);
            if (victim->max_hit > 0 )
            temp = ( 100 * victim->hit ) / victim->max_hit;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( hp, "{G%d{x", temp); }
            else if ( temp >= 34 )
              { sprintf( hp, "{Y%d{x", temp); }
            else if ( temp <= 33 )
              { sprintf( hp, "{R%d{x", temp); }
             else
                sprintf(hp,"{R%d{x",temp);

            if (victim->max_mana > 0 )
            temp = ( 100 * victim->mana ) / victim->max_mana;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( mana, "{G%d{x", temp); }
            else if ( temp >= 34 )
              { sprintf( mana, "{Y%d{x", temp); }
            else if ( temp <= 33 )
              { sprintf( mana, "{R%d{x", temp); }
             else
                sprintf(mana,"{R%d{x",temp);
			sprintf(buf,"You sense %s's vitals: %s{c%%hp %s{c%%mp %s{c%%mv{x",victim->name,hp,mana,move);
			add_buf(final,buf); 
	 		page_to_char(buf_string(final),ch);
	 		free_buf( final );
	 		WAIT_STATE(ch, 6);
         return;
    }
    return;
}
