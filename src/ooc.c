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
 **************************************************************************/

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

void do_boo(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_CHANNEL_LENGTH];
    DESCRIPTOR_DATA *d;
    int wtime;

    if (argument[0] == '\0')
         {
         if(IS_SET(ch->comm,COMM_NOCHAT))
         {
            send_to_char("boo channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOCHAT);
         }
         else
         {
        send_to_char("boo channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCHAT);
      }
    }
    else  /* CHAT message sent, turn CHAT on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

        if (IS_SET(ch->comm,COMM_NOCHANNELS) || IS_SET(ch->comm,COMM_NOPUBCHAN))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }
      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
        send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
        return;
      }

      REMOVE_BIT(ch->comm,COMM_NOCHAT);

      sprintf( buf, "{3[{=BOO{3]{x You '{=%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
        int pos;
        bool found = FALSE;

        victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOCHAT) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
            for (pos = 0; pos < MAX_FORGET; pos++)
            {
                if (IS_NPC(victim))
                    break;
                if (victim->pcdata->forget[pos] == NULL)
                    break;
                if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
                    found = TRUE;
            }
            if (!found)
            {
                act_new( "{3[{=BOO{3]{x $n '{=$t{x'",
                    ch,argument, d->character, TO_VICT,POS_SLEEPING );
            }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}





void do_chat(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_CHANNEL_LENGTH];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (argument[0] == '\0')
         {
         if(IS_SET(ch->comm,COMM_NOCHAT))
         {
            send_to_char("ooc channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOCHAT);
         }
         else
         {
        send_to_char("ooc channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCHAT);
      }
    }
    else  /* CHAT message sent, turn CHAT on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS) || IS_SET(ch->comm, COMM_NOPUBCHAN))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }
 /*
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 
*/
      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

      REMOVE_BIT(ch->comm,COMM_NOCHAT);
 
      sprintf( buf, "{3[{=OOC{3]{x You '{=%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOCHAT) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
                if (IS_NPC(victim))
                    break; 
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
                act_new( "{3[{=OOC{3]{x $n '{=$t{x'",
		    ch,argument, d->character, TO_VICT,POS_SLEEPING );
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}

void do_ic(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_CHANNEL_LENGTH];
    DESCRIPTOR_DATA *d;
    int wtime;
 
    if (argument[0] == '\0')
         {
         if(IS_SET(ch->comm,COMM_NOCHAT))
         {
            send_to_char("ic channel is now ON.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOCHAT);
         }
         else
         {
        send_to_char("ic channel is now OFF.\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCHAT);
      }
    }
    else  /* CHAT message sent, turn CHAT on if it isn't already */
    {
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }
 
        if (IS_SET(ch->comm,COMM_NOCHANNELS) || IS_SET(ch->comm, COMM_NOPUBCHAN))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }
 /*
        if ((!str_infix(" fuck ", argument))
        ||  (!str_prefix("fuck ", argument))
        ||  (!str_suffix(" fuck", argument))
        ||  (!str_suffix(" fuck.", argument))
        ||  (!str_suffix(" fuck!", argument))
        ||  (!str_cmp("fuck", argument))
        ||  (!str_cmp("fuck.", argument))
        ||  (!str_cmp("fuck!", argument))
        ||  (!str_cmp("shit", argument))
        ||  (!str_cmp("shit.", argument))
        ||  (!str_cmp("shit!", argument))
        ||  (!str_infix(" shit ", argument))
        ||  (!str_prefix("shit ", argument))
        ||  (!str_suffix(" shit", argument))
        ||  (!str_suffix(" shit.", argument))
        ||  (!str_suffix(" shit!", argument)))
        { 
            if (!IS_IMMORTAL(ch)) {
                send_to_char("Don't let the IMM's catch you talking like that!\n\r",ch);
                return; 
            } 
        } 
*/
      if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
      && (!IS_IMMORTAL(ch)))
      {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
      }

      REMOVE_BIT(ch->comm,COMM_NOCHAT);
 
      sprintf( buf, "{3[{=IC{3]{x You '{=%s{x'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;
	int pos;
	bool found = FALSE;

        victim = d->original ? d->original : d->character;
 
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
             !IS_SET(victim->comm,COMM_NOCHAT) &&
             !IS_SET(victim->comm,COMM_QUIET) )
        {
	    for (pos = 0; pos < MAX_FORGET; pos++)
	    {
                if (IS_NPC(victim))
                    break; 
		if (victim->pcdata->forget[pos] == NULL)
		    break;
		if (!str_cmp(ch->name,victim->pcdata->forget[pos]))
		    found = TRUE;
	    }
	    if (!found)
	    {
                act_new( "{3[{=IC{3]{x $n '{=$t{x'",
		    ch,argument, d->character, TO_VICT,POS_SLEEPING );
	    }
        }
      }
    }
    wtime = UMAX(2, 9-(ch->level));
    WAIT_STATE( ch, wtime );
}






























