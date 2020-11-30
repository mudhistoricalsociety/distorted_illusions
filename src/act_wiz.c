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
#include <time.h>
#else
#include <sys/types.h>
#include <time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "merc.h"
#include "db.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "clan.h"
#include "interp.h"
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>


/* command procedures needed */
DECLARE_DO_FUN(do_at		);
DECLARE_DO_FUN(do_aset          );
DECLARE_DO_FUN(do_rstat		);
DECLARE_DO_FUN(do_mstat		);
DECLARE_DO_FUN(do_ostat		);
DECLARE_DO_FUN(do_rset		);
DECLARE_DO_FUN(do_mset		);
DECLARE_DO_FUN(do_oset		);
DECLARE_DO_FUN(do_sset		);
DECLARE_DO_FUN(do_lset          );
DECLARE_DO_FUN(do_mfind		);
DECLARE_DO_FUN(do_ofind		);
DECLARE_DO_FUN(do_slookup	);
DECLARE_DO_FUN(do_mload		);
DECLARE_DO_FUN(do_oload		);
DECLARE_DO_FUN(do_vload		);
DECLARE_DO_FUN(do_robjload	);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_save		);
DECLARE_DO_FUN(do_transfer	);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_force		);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_disconnect	);
DECLARE_DO_FUN(do_restore	);
DECLARE_DO_FUN(do_allpeace	);
DECLARE_DO_FUN(do_remaffect	);
DECLARE_DO_FUN(do_robjload2     );
DECLARE_DO_FUN( do_echo		);

long obj_points( OBJ_INDEX_DATA * obj );
long obj_balance( OBJ_INDEX_DATA * obj );
int focus_ac args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch));
int focus_dam args ((CHAR_DATA *ch));

/*
 * Local functions.
 */
bool    write_to_descriptor     args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool	write_to_descriptor_2	args( ( int desc, char *txt, int length ) );
void sig_handler args((int sig));
OBJ_DATA * rand_obj args( ( CHAR_DATA *ch, int mob_level ));
OBJ_DATA * rand_obj2 args( ( CHAR_DATA *ch, int mob_level, char *argument ));
void grant_command args((CHAR_DATA *ch, char *argument));

/*
MYSQL *conn;
*/


/* Override command for Imp's. Change 'Bree' to the owners name */
bool can_over_ride( CHAR_DATA *ch, CHAR_DATA *victim, bool equal )
{
    if ( ch == NULL
    ||   victim == NULL
    ||   victim == ch
    ||   IS_NPC( victim )
    ||   !str_cmp( ch->name, "Bree" ) )
	return TRUE;

    if ( !equal && !str_cmp( victim->name, "Bree" ) )
	return FALSE;

    if ( get_trust( ch ) > get_trust( victim )
    ||   ( equal && get_trust( ch ) >= get_trust( victim ) ) )
	return TRUE;

    return FALSE;
}

const   struct  pair_type        pair_table       [] =
{ 
  {"switch", "return",FALSE},
  {"reboo", "reboot",FALSE},
  {"shutdow", "shutdown",FALSE},
  {"sla", "slay",FALSE},
  {"", "",FALSE}
};

void do_wiznet( CHAR_DATA *ch, char *argument )
{
   int flag;
   int col = 0;
   char buf[MAX_STRING_LENGTH];

   if ( argument[0] == '\0' )
    /* Show wiznet options - just like channel command */
   {
        send_to_char("WELCOME TO WIZNET!!!\n\r", ch);
        send_to_char("   Option      Status\n\r",ch);
        send_to_char("---------------------\n\r",ch);
        /* list of all wiznet options */
        buf[0] = '\0';

        for (flag = 0; wiznet_table[flag].name != NULL; flag++)
        {
            if (wiznet_table[flag].level <= get_trust(ch))
            {
                sprintf( buf, "%-14s %s\t", wiznet_table[flag].name,
		    IS_SET(ch->wiznet,wiznet_table[flag].flag) ? "{GON{0" : "{ROFF{0" );
                send_to_char(buf, ch);
                col++;
                if (col==3)
                {
                   send_to_char("\n\r",ch);
                  col=0;
                }
            }
        }
/* To avoid color bleeding */
     send_to_char("{0",ch);
     return;
   }

   if (!str_prefix(argument,"on"))
   {
        send_to_char("Welcome to Wiznet!\n\r",ch);
        SET_BIT(ch->wiznet,WIZ_ON);
        return;
   }

   if (!str_prefix(argument,"off"))
   {
        send_to_char("Signing off of Wiznet.\n\r",ch);
        REMOVE_BIT(ch->wiznet,WIZ_ON);
        return;
   }

   flag = wiznet_lookup(argument);

   if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
   {
        send_to_char("No such option.\n\r",ch);
        return;
   }

   if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
   {
        sprintf(buf,"You will no longer see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
   else
   {
        sprintf(buf,"You will now see %s on wiznet.\n\r",
                wiznet_table[flag].name);
        send_to_char(buf,ch);
        SET_BIT(ch->wiznet,wiznet_table[flag].flag);
        return;
   }
}


void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "{V%s{x", string);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
/*    if(!d->character || !d || !d->connected)
      return; */
        if (d->connected == CON_PLAYING
	&&  !IS_NPC(d->character)
	&&  (IS_HERO(d->character)
	||  (d->character->pcdata->tier == 2))
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch )
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{Y-->{x ",d->character);
            act_new(buf,d->character,obj,ch,TO_CHAR,POS_DEAD);
        }
    }
 
    return;
}

/* void do_gamestatus( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    bool status = FALSE;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	sprintf(buf,"{GCapslock{y:  %s\n\r",
	    mud_stat.capslock ? "{RON" : "{gOFF");
	send_to_char(buf,ch);

	sprintf(buf,"{GColorlock{y: %s\n\r",
	    mud_stat.colorlock ? "{RON" : "{gOFF");
	send_to_char(buf,ch);

	sprintf(buf,"{GMultilock{y: %s\n\r",
	    mud_stat.multilock ? "{RON" : "{gOFF");
	send_to_char(buf,ch);

	sprintf(buf,"{GNewlock{y:   %s\n\r",
	    mud_stat.newlock ? "{RON" : "{gOFF");
	send_to_char(buf,ch);

	sprintf(buf,"{GWizlock{y:   %s{x\n\r",
	    mud_stat.wizlock ? "{RON" : "{gOFF");
	send_to_char(buf,ch);

	sprintf(buf,"{GGood_God{y:  %s\n\r",
	    mud_stat.good_god_string);
	send_to_char( buf, ch );

	sprintf(buf,"{GEvil_God{y:  %s\n\r",
	    mud_stat.evil_god_string);
	send_to_char( buf, ch );

	sprintf(buf,"{GNeut_God{y:  %s\n\r",
	    mud_stat.neut_god_string);
	send_to_char( buf, ch );

	sprintf(buf,"{GMud Name{y:  %s\n\r",
	    mud_stat.mud_name_string);
	send_to_char( buf, ch );

	return;
    }

    if ( !str_prefix( arg, "good_god" ) )
    {
	if ( argument[0] == '\0' )
	{
	    sprintf( buf, "Good God: %s\n\r", mud_stat.good_god_string );
	    send_to_char( buf, ch );
	} else {
	    free_string( mud_stat.good_god_string );
	    mud_stat.good_god_string = str_dup( argument );
	    mud_stat.changed = TRUE;
	}
	return;
    }

    else if ( !str_prefix( arg, "evil_god" ) )
    {
	if ( argument[0] == '\0' )
	{
	    sprintf( buf, "Evil God: %s\n\r", mud_stat.evil_god_string );
	    send_to_char( buf, ch );
	} else {
	    free_string( mud_stat.evil_god_string );
	    mud_stat.evil_god_string = str_dup( argument );
	    mud_stat.changed = TRUE;
	}
	return;
    }

    else if ( !str_prefix( arg, "neut_god" ) )
    {
	if ( argument[0] == '\0' )
	{
	    sprintf( buf, "Neut God: %s\n\r", mud_stat.neut_god_string );
	    send_to_char( buf, ch );
	} else {
	    free_string( mud_stat.neut_god_string );
	    mud_stat.neut_god_string = str_dup( argument );
	    mud_stat.changed = TRUE;
	}
	return;
    }

    else if ( !str_prefix( arg, "name" ) )
    {
	if ( argument[0] == '\0' )
	{
	    sprintf( buf, "Mud Name: %s\n\r", mud_stat.mud_name_string );
	    send_to_char( buf, ch );
	} else {
	    free_string( mud_stat.mud_name_string );
	    mud_stat.mud_name_string = str_dup( argument );
	    mud_stat.changed = TRUE;
	}
	return;
    }

    if ( !str_cmp(argument,"on") )
	status = TRUE;
    else if ( !str_cmp(argument,"off") )
	status = FALSE;
    else
    {
	send_to_char("Syntax: gamestatus capslock  <on|off>.\n\r"
		     "        gamestatus colorlock <on|off>.\n\r"
		     "        gamestatus multilock <on|off>.\n\r"
		     "        gamestatus newlock   <on|off>.\n\r"
		     "        gamestatus wizlock   <on|off>.\n\r"
		     "        gamestatus good_god  <name>.\n\r"
		     "        gamestatus evil_god  <name>.\n\r"
		     "        gamestatus neut_god  <name>.\n\r"
		     "        gamestatus name      <name>.\n\r", ch);
	return;
    }

    if ( !str_prefix(arg,"capslock") )
    {
	mud_stat.capslock = status;

	if ( mud_stat.capslock )
	{
	    wiznet("$N has capslocked the game.",ch,NULL,0,0,0);
	    send_to_char( "Game capslocked.\n\r", ch );
	} else {
	    wiznet("$N removes capslock.",ch,NULL,0,0,0);
	    send_to_char( "Game un-capslocked.\n\r", ch );
	}
    }

    else if ( !str_prefix(arg,"colorlock") )
    {
	mud_stat.colorlock = status;

	if ( mud_stat.colorlock )
	{
	    wiznet("$N has colorlocked the game.",ch,NULL,0,0,0);
	    send_to_char( "Game colorlocked.\n\r", ch );
	} else {
	    wiznet("$N removes colorlock.",ch,NULL,0,0,0);
	    send_to_char( "Game un-colorlocked.\n\r", ch );
	}
    }

    else if ( !str_prefix(arg,"multilock") )
    {
	mud_stat.multilock = status;

	if ( mud_stat.multilock )
	{
	    CHAR_DATA *victim, *wch;
	    wiznet("$N has multilocked the game.",ch,NULL,0,0,0);
	    send_to_char( "Game multilocked.\n\r", ch );

	    for ( victim = player_list; victim != NULL; victim = victim->pcdata->next_player )
	    {
		if ( IS_NPC(victim)
		||   IS_IMMORTAL(victim)
		||   !str_cmp(victim->name,"Tester")
		||   !str_cmp(victim->name,"Testguy")
		||   victim->pcdata->socket == NULL )
		    continue;

		for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
		{
		    if ( !IS_IMMORTAL(wch)
		    &&   str_cmp(wch->name,"Tester")
		    &&   str_cmp(wch->name,"Testguy")
		    &&   str_cmp(victim->name, wch->name)
		    &&   wch->pcdata->socket != NULL
		    &&   !check_allow(wch->pcdata->socket,ALLOW_ITEMS)
		    &&   !check_allow(wch->pcdata->socket,ALLOW_CONNECTS)
		    &&   !strcmp(victim->pcdata->socket, wch->pcdata->socket) )
                    {
			send_to_char("Sorry, multiplaying has been terminated.\n\r",wch);
			force_quit(wch,"");
			continue;
		    }
		}
	    }
	} else {
	    wiznet("$N removes multilock.",ch,NULL,0,0,0);
	    send_to_char( "Game un-multilocked.\n\r", ch );
	}
    }

    else if ( !str_prefix(arg,"newlock") )
    {
	mud_stat.newlock = status;

	if ( mud_stat.newlock )
	{
	    wiznet("$N has newlocked the game.",ch,NULL,0,0,0);
	    send_to_char( "Game newlocked.\n\r", ch );
	} else {
	    wiznet("$N removes newlock.",ch,NULL,0,0,0);
	    send_to_char( "Game un-newlocked.\n\r", ch );
	}
    }

    else if ( !str_prefix(arg,"wizlock") )
    {
	mud_stat.wizlock = status;

	if ( mud_stat.wizlock )
	{
	    wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	    send_to_char( "Game wizlocked.\n\r", ch );
	} else {
	    wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	    send_to_char( "Game un-wizlocked.\n\r", ch );
	}
    }

    else
    {
	do_gamestatus(ch,"");
	return;
    }

    mud_stat.changed = TRUE;
}
*/

void do_remaffect ( CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int counter;
    int aff_num;

    counter = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if(arg1[0]=='\0')
    {
      send_to_char("Syntax: remaffect <affectname> [<target>]\r\n",ch);
      return;
    }
    else
    {
      if(arg2[0]=='\0')
      {
        aff_num = skill_lookup(arg1);
        while(1)
        {
          paf = affect_find(ch->affected,aff_num);
          if(paf != NULL)
          {
            counter++;
            affect_remove(ch, paf);
            continue;
          }
          else 
          {  
            if(counter == 0)
            {
              send_to_char("You aren't affected by that.\r\n",ch);
              return;
            }
            else
            {   
              send_to_char("Affect removed.\r\n",ch);
              break;
            }
          }
        }
      }
      else
      {
        victim = get_char_world(ch, arg2);
        if (victim == NULL)
        {
          send_to_char( "They aren't here.\n\r", ch );
          return;
        }
        else
        {
          aff_num = skill_lookup(arg1); 
          while(1)
          {
            paf = affect_find(victim->affected,aff_num);
            if(paf!=NULL)
            {
              counter++;
              affect_remove(victim, paf);
              continue;
            }
            else
            {  
              if(counter == 0)
              {  
                send_to_char("They aren't affected by that.\r\n",ch);
                return;
              }
              else
              {   
                send_to_char("Affect removed.\r\n",ch);
                break;
              }
            }
          }
        }
      }
    }
}



/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if (ch->level > 9 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	if (ch->carry_number + 1 > can_carry_n(ch))
	{
	    send_to_char("You can't carry any more items.\n\r",ch);
	    return;
	}
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
	act("$G gives you a light.",ch,NULL,NULL,TO_CHAR);
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
        act("$G gives you a vest.",ch,NULL,NULL,TO_CHAR);
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
    {
    	sn = 0; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->pcdata->learned[sn] < 
		ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

    	obj = create_object(get_obj_index(vnum),0);
     	obj_to_char(obj,ch);
    	equip_char(ch,obj,WEAR_WIELD);
        act("$G gives you a weapon.",ch,NULL,NULL,TO_CHAR);
    }

    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
    ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
    &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        if (ch->carry_number + 1 > can_carry_n(ch))
        {
            send_to_char("You can't carry any more items.\n\r",ch);
            return;
        }
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
        act("$G gives you a shield.",ch,NULL,NULL,TO_CHAR);
    }
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    bool fAll = TRUE;
 
    argument = one_argument( argument, arg );
    one_argument( argument, arg1 );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom? -- Usage: nochannel <victim> <public|or leave blank>", ch );
        return;
    }
    
    if ( arg1[0] == '\0' )
    {
    	fAll = TRUE;	
    }
    else if(!str_prefix(arg1,"public"))
    {
    	 fAll = FALSE;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust(victim) >= get_trust(ch) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if(fAll)
    {
    	if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    	{
        	REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        	send_to_char( "The gods have restored your channel priviliges.\n\r", victim );
        	send_to_char( "NOCHANNELS removed.\n\r", ch );
		sprintf(buf,"$N restores channels to %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    	}
    	else
    	{
        	SET_BIT(victim->comm, COMM_NOCHANNELS);
        	send_to_char( "The gods have revoked your channel priviliges.\n\r", victim );
        	send_to_char( "NOCHANNELS set.\n\r", ch );
		sprintf(buf,"$N revokes %s's channels.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    	}
    }
    else
    {
    	if ( IS_SET(victim->comm, COMM_NOPUBCHAN) )
    	{
        	REMOVE_BIT(victim->comm, COMM_NOPUBCHAN);
        	send_to_char( "The gods have restored your public channel priviliges.\n\r", victim );
        	send_to_char( "NOPUBCHAN removed.\n\r", ch );
		sprintf(buf,"$N restores public channels to %s",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    	}
    	else
    	{
        	SET_BIT(victim->comm, COMM_NOPUBCHAN);
        	send_to_char( "The gods have revoked your public channel priviliges.\n\r", victim );
        	send_to_char( "NOPUBCHAN set.\n\r", ch );
		sprintf(buf,"$N revokes %s's public channels.",victim->name);
		wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    	}
    }
    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_quit( victim, "" );

    return;
}

void do_wipe( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Wipe whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->comm, COMM_WIPED);
    sprintf(buf,"$N wipes access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_disconnect( ch, victim->name);

    return;
}


void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	if (get_trust( ch ) < MAX_LEVEL)
	{
	    return;
	}

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || (get_trust( victim ) > get_trust( ch ) /* && get_trust( victim ) == MAX_LEVEL */ ))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	 send_to_char("{RDenied.{x\n\r",ch);
	 victim = ch;
    }

    if (IS_SET(victim->act, PLR_KEY)
    && (ch->level < MAX_LEVEL) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d == ch->desc )
	    {
		close_socket( d );
		return;
	    }
	}
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_newbie( CHAR_DATA *ch, char *argument ) {
    CHAR_DATA *victim;

    if(argument[0]=='\0') {
	send_to_char("Syntax: newbie <character>.\n\r",ch);
	return; }

    if((victim=get_char_world(ch,argument))==NULL) {
	send_to_char("They aren't here.\n\r",ch);
	return; }

    if(IS_NPC(victim)) {
	send_to_char("Not on NPC's.\n\r",ch);
	return; }

    /*if((get_trust(victim)>=get_trust(ch)) && (victim!=ch)) {
	send_to_char("Your command backfires!\n\r",ch);
	send_to_char("You are now a NEWB!\n\r",ch);
	SET_BIT(ch->plyr,PLAYER_NEWBIE);
	return; }*/

    if(IS_SET(victim->plyr,PLAYER_NEWBIE) && (ch == victim )) {
	send_to_char("Newbie flag removed.\n\r",ch);
	send_to_char("You are no longer a newbie!.\n\r",victim); 
	REMOVE_BIT(victim->plyr,PLAYER_NEWBIE); }
	
	else if(IS_SET(victim->plyr,PLAYER_NEWBIE) && (ch->level > 101)) {
	send_to_char("Newbie flag removed.\n\r",ch);
	send_to_char("You are no longer a newbie!.\n\r",victim); 
	REMOVE_BIT(victim->plyr,PLAYER_NEWBIE); }
    
	else if(!IS_SET(victim->plyr,PLAYER_NEWBIE) && (ch->level > 101)){
	send_to_char("Newbie flag set.\n\r",ch);
	send_to_char("You join the ranks of the newbies.\n\r",victim);
	SET_BIT(victim->plyr,PLAYER_NEWBIE); }
   
	else
	{
		send_to_char("You arent a Newbie.\n\r",ch);
		return;
	}
    return;
}

void do_twit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;


    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: twit <character>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( (get_trust( victim ) >= get_trust( ch )) && (victim != ch) )
    {
	send_to_char( "Your command backfires!\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", ch );
	SET_BIT( ch->act, PLR_TWIT );
	return;
    }

    if ( IS_SET(victim->act, PLR_TWIT) )
    {
	send_to_char( "Someone beat you to it.\n\r", ch );
    } else
    {
	SET_BIT( victim->act, PLR_TWIT );
	send_to_char( "Twit flag set.\n\r", ch );
	send_to_char( "You are now considered a TWIT.\n\r", victim );
    }
    return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;


    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character>.\n\r", ch );
        return;
    }  

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }  

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_TWIT) )
    {
        REMOVE_BIT( victim->act, PLR_TWIT );
        send_to_char( "Twit flag removed.\n\r", ch );
        send_to_char( "You are no longer a TWIT.\n\r", victim );
    }

    if ( IS_SET(victim->plyr, PLAYER_ANCIENT_ENEMY) )
    {
        REMOVE_BIT( victim->plyr, PLAYER_ANCIENT_ENEMY );
        send_to_char( "ENEMY flag removed.\n\r", ch );
        send_to_char( "You are no longer an Enemy of Midgaard.\n\r", victim );
    }

    return;
}

void do_announce( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
   send_to_char( "Announce What?\n\r", ch );
   return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
   if ( d->connected == CON_PLAYING && !IS_SET(d->character->comm, COMM_QUIET))
   {
       send_to_char( "{3[{#INFO{3]:{# ",d->character);
       send_to_char( argument, d->character );
       send_to_char( "{x\n\r",   d->character );
   }
    }

    return;
}


void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= ch->level)
            {
		sprintf( buf, "G %s > ", ch->name );
                send_to_char( buf, d->character );
            }
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_wecho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];


    if ( argument[0] == '\0' )
    {
	send_to_char( "Warn echo what?\n\r", ch );
	return;
    }

    sprintf(buf, "{z{#***{x {^%s{x {z{#***{x", argument);
    do_echo(ch, buf);
    do_echo(ch, buf);
    do_echo(ch, buf);
    return;
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= ch->level)
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;


    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= ch->level)
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= ch->level)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
//    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

void do_corner( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char corner[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Corner whom?\n\r", ch);
	return;
    }

    sprintf(corner, "%d", ROOM_VNUM_CORNER);

    location = find_location( ch, corner );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (( victim->level > ch->level
    && !IS_SET(ch->act, PLR_KEY)
    && (victim->level != MAX_LEVEL))
    || ((IS_SET(victim->act, PLR_KEY))
    && (ch->level != MAX_LEVEL)))
    {
        send_to_char( "You failed!\n\r", ch);
        return;
    }

    if ( victim->in_room == NULL )
    {
        send_to_char( "They are in limbo.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );

    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if ( ch != victim )
        act( "$n has transferred you.", ch, NULL, victim, TO_VICT );

    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );

    return;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) && (ch->level >= CREATOR))
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   ch->level >= d->character->ghost_level
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( ch,location ) 
	&&  ch->level < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (( victim->level > ch->level
    && !IS_SET(ch->act, PLR_KEY) 
    && (victim->level != MAX_LEVEL))
    || ((IS_SET(victim->act, PLR_KEY)) 
    && (ch->level != MAX_LEVEL)))
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_allpeace( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;


    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   ch->level >= d->character->ghost_level
	&&   can_see( ch, d->character ) )
	{
	    char buf[MAX_STRING_LENGTH];
	    sprintf( buf, "%s peace", d->character->name );
	    do_at( ch, buf );
	}
    }
}

void do_wedpost( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: wedpost <char>\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }
    
    if (victim->wedpost)
    {
        send_to_char("They are no longer allowed to post wedding announcements.\n\r",ch);
        victim->wedpost = FALSE;
    }
    else
    {
        send_to_char("They are now allowed to post wedding announcements.\n\r",ch);
        victim->wedpost = TRUE;
    }
}

void do_recover( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;


    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Recover whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && !IS_SET(ch->act,ACT_PET) )
    {
	send_to_char( "You can't recover NPC's.\n\r", ch );
	return;
    }

    if (( get_trust( victim ) > get_trust( ch )
    && !IS_SET(ch->act, PLR_KEY) 
    && (victim->level != MAX_LEVEL))
    || ((IS_SET(victim->act, PLR_KEY)) 
    && (ch->level != MAX_LEVEL)))
    {
        send_to_char( "You failed!\n\r", ch);
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
    {
	send_to_char( "They are fighting.\n\r", ch );
	return;
    }

    if ( victim->alignment < 0 )
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLEB ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
	{
	    send_to_char( "The recall point seems to be missing.\n\r", ch );
	    return;
	}
    }

    if (is_clan(victim)
    && (clan_table[victim->clan].recall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->act, PLR_TWIT))
	location = get_room_index( clan_table[victim->clan].recall );

    if (IS_NPC(victim) && IS_SET(ch->act,ACT_PET)
    && is_clan(victim->master)
    && (clan_table[victim->master->clan].recall != ROOM_VNUM_ALTAR)
    && !IS_SET(victim->master->act, PLR_TWIT)) 
        location = get_room_index( clan_table[victim->master->clan].recall );

    if ( victim->in_room == location )
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    &&   !IS_AFFECTED(victim, AFF_CURSE))
    {
	act( "$N does not need recovering.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a flash.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a flash of light.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has recovered you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    act( "$N has been recovered.", ch, NULL, victim, TO_CHAR);
    if (victim->pet != NULL)
	do_recover(victim->pet,"");
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;

    
    if (IS_NPC(ch))
    {
	send_to_char( "NPC's cannot use this command.\n\r", ch);
	return;
    }
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( ch,location ) 
    &&  ch->level < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int count = 0;

    if ( ( argument[0] == '\0' )
	&& ( IS_NPC(ch) ) )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }
    if ( ( argument[0] == '\0' )
	&& ( !ch->pcdata->recall ) ) 
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }
    if ( ( argument[0] == '\0' )
      && (ch->pcdata->recall) )
    {
	sprintf(arg, "%d", ch->pcdata->recall);
    }
    else
    {
	sprintf(arg, "%s", argument);
    }
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(ch,location) 
    &&  (count > 1 || ch->level < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

  /*  if (IS_SET(victim->act,PLR_NOGOTO))
    {
      send_to_char( "Nice try. Try asking.\n\r",ch);
      return;
    } 
*/
    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch) >= ch->ghost_level))
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch)  >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
    if ( ( argument[0] == '\0' )
      && (ch->pet != NULL) )
    {
	char_from_room( ch->pet );
	char_to_room( ch->pet, location );
    }
    do_look( ch, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
/*
    if (IS_SET(victim->act,PLR_NOVIOLATE) && ch->level < victim->level)
    {
      send_to_char( "Nice try. Try asking.\n\r",ch);
      return;
    } */
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( ch,location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch) >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if ((get_trust(rch) >= ch->invis_level)
	&& (get_trust(rch) >= ch->ghost_level))
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
}

void do_ftick ( CHAR_DATA *ch, char *argument )
{

    update_handler( TRUE );
    return;
}

/* RT to replace the 3 stat commands */

/* MYSQL
void do_mysql ( CHAR_DATA *ch, char *argument )
{
   char arg[MIL];
   char *string;

   string = one_argument(argument, arg);

   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  mysql status\n\r",ch);
	send_to_char("  mysql connect\n\r",ch);
	send_to_char("  mysql disconnect\n\r",ch);
 	send_to_char("  mysql ping\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"status"))
   {
	if (conn == NULL)
	{
		send_to_char( "No current connection detected.\n\r", ch);
		return;
	}

	send_to_char( sql_status(), ch);
	send_to_char( "\n\r", ch);
	return;
   }

   if (!str_cmp(arg,"connect"))
   {

	conn = sql_connect();

	if (conn == NULL)
	{
		send_to_char( "Connection Failed.\n\r", ch);
	}

	send_to_char ( "MySQL Connection Established.\n\r", ch );
	return;
   }

   if (!str_cmp(arg,"disconnect"))
   {
	if (conn == NULL)
	{
		send_to_char( "No current connection detected.\n\r", ch);
		return;
	}

	sql_disconnect(conn);
	send_to_char( "MySQL Connection Terminated.\n\r", ch);
	conn = NULL;
	return;
   }

   if (!str_cmp(arg,"ping"))
   {
	if ( sql_ping() )
	{
		send_to_char ( "MySQL Null connection detected, Ping sent.\n\r", ch );
		return;
	}

	send_to_char( "MySQL Connection already present.\n\r", ch);
	return;
   }

}

*/

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;


   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;
    AFFECT_DATA *paf;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( ch,location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
        "Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
        sector_bit_name(location->sector_type),
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %s.\n\rDescription:\n\r%s",
	room_bit_name(location->room_flags),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (( get_trust(ch) >= rch->ghost_level)
	&& (can_see(ch,rch)))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( "Affected:", ch );
    for ( paf = location->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
	    "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
            paf->where == TO_ROOM_AFF ?
            room_affect_name( paf->bitvector) :
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	send_to_char(buf,ch);
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *sameobj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }
    if (( sameobj = get_obj_index(obj->pIndexData->vnum)) == NULL ) {
	    send_to_char( "Uhh, it just worked for OBJ_DATA, why not OBJ_INDEX_DATA?\n\r",ch);
	    return; }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_type_name(obj), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d  Obj_Balance: %5ld\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4], obj_balance(sameobj) );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
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
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
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
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->clan)
	    {
		sprintf( buf, "Damage is variable.\n\r");
	    } else
	    {
		if (obj->pIndexData->new_format)
		    sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
		else
		    sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    }
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		attack_table[obj->value[3]].noun);
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
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
		"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    }
	    send_to_char( buf, ch );
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
    if (is_clan_obj(obj))
    {
	sprintf( buf, "This object is owned by the [%s{x] clan.\n\r",
	    clan_table[obj->clan].who_name
	    );
	send_to_char( buf, ch );
    }

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
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
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
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
            send_to_char(buf,ch);
        }
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL 
    || (victim->level > get_trust(ch) && victim->level == MAX_LEVEL))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	if (!str_cmp(victim->name, "Raos"))
                sprintf( buf, "Name: %s\n\rIdentity: %s\n\rSocket: k17b4.AD7.spar.edu\n\r",
                    victim->name, victim->pcdata->identity);
	else if (!str_cmp(victim->name, "Corlan"))
                sprintf( buf, "Name: %s\n\rIdentity: %s\n\rSocket: icenet.pp7.ibf.au\n\r",
                    victim->name, victim->pcdata->identity);
	else if (!str_cmp(victim->name, "Electrum"))
                sprintf( buf, "Name: %s\n\rIdentity: %s\n\rSocket: 176.83.32.7\n\r",
                    victim->name, victim->pcdata->identity);
	else
                sprintf( buf, "Name: %s\n\rIdentity: %s\n\rSocket: %s Morph_Form: %d\n\r",
                    victim->name, victim->pcdata->identity, victim->pcdata->socket, victim->morph_form[0]);
    } else
    {
	sprintf( buf, "Name: %s\n\rSocket: <mobile>\n\r",
	    victim->name);
    }
    send_to_char(buf,ch);

    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char(buf,ch);

    if (IS_NPC(victim))
    {
        if(victim->pIndexData->clan != 0)
        {
         sprintf(buf,"Clan: %s ",clan_table[victim->pIndexData->clan].who_name);
         send_to_char(buf,ch);
        }
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
   	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char(buf,ch);

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char(buf,ch);
	
    sprintf( buf,
        "Lv: %d  Class: %s  Align: %d  Exp: %ld Tier: %d\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
	victim->alignment,
        victim->exp, IS_NPC(victim) ? -1 : victim->pcdata->tier );
    send_to_char(buf,ch);

    sprintf( buf,
        "Platinum: %ld  Gold: %ld  Silver: %ld Bounty: %ld\n\r",
        victim->platinum, victim->gold, victim->silver, (IS_NPC(victim) ? -1 : victim->pcdata->bounty) );
    send_to_char(buf,ch);

    if (!IS_NPC(victim))
    {
	sprintf( buf,
	    "Bank-0: %ld  Bank-1: %ld  Bank-2: %ld  Bank-3: %ld\n\r",
	    victim->balance[0] > 0 ? victim->balance[0] : 0,
	    victim->balance[1] > 0 ? victim->balance[1] : 0,
	    victim->balance[2] > 0 ? victim->balance[2] : 0,
	    victim->balance[3] > 0 ? victim->balance[3] : 0 );
	send_to_char(buf,ch);
    }

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char(buf,ch);

    if (is_clan(victim))
	{
	sprintf(buf, "{cClan: {w%s  {cRank: %s  {cPetition: {w%s\n\r",
	clan_table[victim->clan].name,
	victim->rank == MEMBER ? "{c({wnone{c){x" : clan_rank_table[victim->rank].rank,
	victim->petition == INDEP ? "{c({wnone{c){x" : clan_table[victim->petition].name);
	send_to_char(buf,ch);
	}

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char(buf,ch);

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
         "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  ImmQuest: %d  AutoQuest: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->qps, victim->questpoints );
	send_to_char(buf,ch);
    }

    sprintf( buf, "Carry number: %d  Carry weight: %ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char(buf,ch);

    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
	send_to_char(buf,ch);
    }

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    if (victim->comm)
    {
    	sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }
    if (victim->affected2_by)
    {
         sprintf(buf, "Also affected by %s\n\r",
            affect2_bit_name(victim->affected2_by));
           send_to_char (buf,ch);
    }
    if (victim->shielded_by)
    {
	sprintf(buf, "Shielded by %s\n\r", 
	    shield_bit_name(victim->shielded_by));
	send_to_char(buf,ch);
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char(buf,ch);

    if (!IS_NPC(victim))
    {
	sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );	/* OLC */
	send_to_char( buf, ch );					/* OLC */
    }

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char(buf,ch);

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Mobile has special procedure %s.\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    char show_vect[MSL];
    char bit_type[MSL];

    for (paf = victim->affected; paf != NULL; paf = paf->next)
    {
	    switch(paf->where)
	    {	default:
		case TO_AFFECTS:
		sprintf(show_vect, "%s", affect_bit_name (paf->bitvector));
		sprintf(bit_type, "aff");
		break;

		case TO_AFFECTS2:
		sprintf(show_vect, "%s", affect2_bit_name (paf->bitvector));
		sprintf(bit_type, "aff2");
		break;
	   }

        sprintf (buf,
                 "Spell: '%s' modifies %s by %d for %d hours with bits(%s) %s, level %d.\n\r",
                 skill_table[(int) paf->type].name,
                 affect_loc_name (paf->location),
                 paf->modifier,
                 paf->duration, bit_type, show_vect, paf->level);
        send_to_char (buf, ch);
    }


    printf_to_char(ch,"Hp_Regen_Rate: %d, Mana_Regen_Rate: %d", victim->regen_rate, victim->mana_regen_rate);
    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;
    char *string;


    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	send_to_char("  vnum areas\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }

    if (!str_cmp(arg,"areas") || !str_cmp(arg,"area"))
    {
	output = new_buf();
	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	    pArea2 = pArea2->next;

	for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	{
	    sprintf( buf, "%-26s {R%5d %5d{x  %-26s {R%5d %5d{x\n\r",
		pArea1->name, pArea1->min_vnum, pArea1->max_vnum,
		(pArea2 != NULL) ? pArea2->name : "",
		(pArea2 != NULL) ? pArea2->min_vnum : 0,
		(pArea2 != NULL) ? pArea2->max_vnum : 0);
	    add_buf(output,buf);
	    pArea1 = pArea1->next;
	    if ( pArea2 != NULL )
		pArea2 = pArea2->next;
	}
	page_to_char( buf_string(output), ch );
	free_buf(output);
 	return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    int count;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;
    count	= 0;
    output	= new_buf();

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		count++;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		add_buf(output,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
    {
	send_to_char( "No mobiles by that name.\n\r", ch );
    }
    else
    {
	page_to_char( buf_string(output), ch );
    }
    free_buf(output);
    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    int count;
    bool fAll;
    bool found;
    bool unbal=FALSE;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "unbal" ) )
	unbal = TRUE;

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;
    count	= 0;
    output	= new_buf();

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) ||
		(unbal && (obj_balance(pObjIndex) > 1000 )))
	    {
		found = TRUE;
		count++;
		if ( obj_balance(pObjIndex) <= -25 )
			sprintf( buf, "[{B%5d{x] [%5ld/%5ld]  %s\n\r",
			    pObjIndex->vnum, obj_points(pObjIndex),
			    obj_balance(pObjIndex), pObjIndex->short_descr );
		else if ( obj_balance(pObjIndex) <= 25 && 
		     obj_balance(pObjIndex) > -25 )
			sprintf( buf, "[{G%5d{x] [%5ld/%5ld]  %s\n\r",
			    pObjIndex->vnum, obj_points(pObjIndex),
			    obj_balance(pObjIndex), pObjIndex->short_descr );
		else if ( obj_balance(pObjIndex) <= 300 &&
		     obj_balance(pObjIndex) > 25 )
			sprintf( buf, "[{Y%5d{x] [%5ld/%5ld]  %s\n\r",
			    pObjIndex->vnum, obj_points(pObjIndex),
			    obj_balance(pObjIndex), pObjIndex->short_descr );
		else if ( obj_balance(pObjIndex) <= 800 &&
		     obj_balance(pObjIndex) > 25 )
			sprintf( buf, "[{C%5d{x] [%5ld/%5ld]  %s\n\r",
			    pObjIndex->vnum, obj_points(pObjIndex),
			    obj_balance(pObjIndex), pObjIndex->short_descr );
		else
			sprintf( buf, "[{R%5d{x] [%5ld/%5ld]  %s\n\r",
			    pObjIndex->vnum, obj_points(pObjIndex),
			    obj_balance(pObjIndex), pObjIndex->short_descr );

		add_buf(output,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
    {
	send_to_char( "No objects by that name.\n\r", ch );
    }
    else
    {
	page_to_char( buf_string(output), ch );
    }
    free_buf(output);
    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();


    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
        ||   ch->level < obj->level
	||   (obj->carried_by != NULL && !can_see(ch,obj->carried_by)))
            continue;
 
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}
void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MSL];
    char hp[MSL];
    char mana[MSL];
    char move[MSL];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;
    int temp = 0;

    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */
    send_to_char("Players currently logged on to Distorted Illusions.\n\r\n\r",ch);
	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		if ((victim->level <= ch->level)
		    || get_trust(ch) > get_trust(victim))
		{
		    count++;
		    if (d->original != NULL)
			sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		    else
            {
            if (victim->max_move > 0 )
            temp = ( 100 * victim->move ) / victim->max_move;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( move, "{G%d{x", victim->move); }
            else if ( temp >= 34 )
              { sprintf( move, "{Y%d{x", victim->move); }
            else if ( temp <= 33 )
              { sprintf( move, "{R%d{x", victim->move); }
             else
                sprintf(move,"{R%d{x",victim->move);
            if (victim->max_hit > 0 )
            temp = ( 100 * victim->hit ) / victim->max_hit;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( hp, "{G%d{x", victim->hit); }
            else if ( temp >= 34 )
              { sprintf( hp, "{Y%d{x", victim->hit); }
            else if ( temp <= 33 )
              { sprintf( hp, "{R%d{x", victim->hit); }
             else
                sprintf(hp,"{R%d{x",victim->hit);

            if (victim->max_mana > 0 )
            temp = ( 100 * victim->mana ) / victim->max_mana;
            else
            temp = 0;
            if ( temp >= 68 )
              { sprintf( mana, "{G%d{x", victim->mana); }
            else if ( temp >= 34 )
              { sprintf( mana, "{Y%d{x", victim->mana); }
            else if ( temp <= 33 )
              { sprintf( mana, "{R%d{x", victim->mana); }
             else
                sprintf(mana,"{R%d{x",victim->mana);

            if (victim->fighting) 
            {

           		if (IS_NPC(victim->fighting))
				sprintf(buf2,"{*Fight{x: %s{x",victim->fighting->short_descr);
            	else
        		sprintf(buf2,"{!PK {*Fight{x:{# %s{x",victim->fighting->name );

            }
            else if (victim->fight_timer > 0)
            {
	            sprintf(buf2,"{RPK Timer{D: {r%d{x",victim->fight_timer);
            }
            else
                sprintf(buf2,"{$Resting{x");
			sprintf(buf,"{@%3d{x: {6%s {7(%s %s %s{7) {6%s {3[{^%5d{3] {*in {&%s {^(%s{^){x\n\r",
			count, victim->name,
            hp,mana,move,victim->in_room->name,
			victim->in_room->vnum,victim->in_room->area->name,buf2);
            }
		    add_buf(buffer,buf);
		}
	    }
	}
    page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }
    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL &&   is_name( argument, victim->name ) )
	{
	    if ((victim->level <= CREATOR && ch->level <= CREATOR)
		|| get_trust(ch) > CREATOR)
	    {
		found = TRUE;
		count++;
		sprintf( buf, "%3d) [%5d] %-28s [%5d] %s{x\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
		add_buf(buffer,buf);
	    }
	}
	if ( count >= 200 )
	    break;
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{

    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}

void do_reboot( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);
  argument = one_argument(argument,arg2);    

    if (arg[0] == '\0') 
    {
        send_to_char("Usage: reboot now <message>\n\r",ch);
        send_to_char("Usage: reboot <ticks to reboot>\n\r",ch);
        send_to_char("Usage: reboot cancel\n\r",ch);
        send_to_char("Usage: reboot status\n\r",ch);
        return;
    }

    if (is_name(arg,"cancel")) 
    {
      reboot_counter = -1;
      send_to_char("Reboot canceled.\n\r",ch);
      return;
    }

    if (is_name(arg, "now")) 
    {
      reboot_system(arg2);
      return;
    }

    if (is_name(arg, "status")) 
    {
      if (reboot_counter == -1) 
	sprintf(buf, "Automatic rebooting is inactive.\n\r");
      else
	sprintf(buf,"Reboot in %i minutes.\n\r",reboot_counter);
      send_to_char(buf,ch);
      return;
    }

    if (is_number(arg))
    {
     reboot_counter = atoi(arg);
     sprintf(buf,"System will reboot in %i ticks.\n\r",reboot_counter);
     send_to_char(buf,ch);
     return;
    }

 do_reboot(ch,"");   
}

void reboot_system( char *message )
{
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    FILE *fp;


    if(message[0] == '\0')
     message = str_dup("System is going down for reboot NOW!");

    sprintf( log_buf, "Rebooting System.");
    log_string(log_buf);
    if((fp = fopen(LAST_COMMAND,"a")) == NULL)
      bug("Error in do_auto_save opening last_command.txt",0);
   
      fprintf(fp,"Last Command: %s\n",
            last_command);

    fclose( fp );

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
        send_to_desc(message,d);
        if (d->character != NULL)
	   save_char_obj(d->character);
    	close_socket(d);
    }
    merc_down = TRUE;    
    return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{

    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);    

    if (arg[0] == '\0') 
    {
	if (!str_cmp(ch->name,"Callin"))
	{
	    do_shutdown(ch,"now");
	    return;
	}
        send_to_char("Usage: shutdown now\n\r",ch);
        send_to_char("Usage: shutdown <ticks to reboot>\n\r",ch);
        send_to_char("Usage: shutdown cancel\n\r",ch);
        send_to_char("Usage: shutdown status\n\r",ch);
        return;
    }

    if (is_name(arg,"cancel")) 
    {
      shutdown_counter = -1;
      send_to_char("Shutdown canceled.\n\r",ch);
      return;
    }

     if (is_name(arg, "now")) 
     {
	 if (ch->invis_level < LEVEL_HERO)
	 sprintf( buf, "Shutdown by %s.", ch->name );

	 append_file( ch, SHUTDOWN_FILE, buf );
	 strcat( buf, "\n\r" );

	 if (ch->invis_level < LEVEL_HERO)
	 {
                do_echo(ch, buf );
	 }

      shutdown_system();
      return;
     }

    if (is_name(arg, "status")) 
    {
      if (shutdown_counter == -1) 
        sprintf(buf, "No shutdown is scheduled.\n\r");
      else
        sprintf(buf,"Shutdown in %i minutes.\n\r",shutdown_counter);
      send_to_char(buf,ch);
      return;
    }

    if (is_number(arg))
    {
     shutdown_counter = atoi(arg);
     sprintf(buf,"System will shutdown in %i ticks.\n\r",shutdown_counter);
     send_to_char(buf,ch);
     return;
    }

 do_shutdown(ch,"");   
}

void shutdown_system( void )
{
         extern bool merc_down;
	 DESCRIPTOR_DATA *d,*d_next;

         sprintf( log_buf, "SHUTDOWN.");
         log_string(log_buf);

         for ( d = descriptor_list; d != NULL; d = d_next )
         {
          d_next = d->next;
          send_to_desc("System is SHUTING DOWN for update NOW!",d);
          if (d->character != NULL)
	   save_char_obj(d->character);
          close_socket(d);
         }  
         merc_down = TRUE;    
         return;

}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;


    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );

	if ( !IS_TRUSTED( ch, CREATOR ) )
	    wiznet( "$N stops being such a snoop.",
		ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust( ch ) );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE )
    ||   ( IS_SET( victim->comm, COMM_SNOOP_PROOF ) && !IS_TRUSTED( ch, CREATOR ) ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
   
    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;

    if ( !IS_TRUSTED( ch, CREATOR ) )
    {
	sprintf( buf, "$N starts snooping on %s",
	    ( IS_NPC( ch ) ? victim->short_descr : victim->name ) );
	wiznet( buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust( ch ) );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }
/*
    if (victim->level > ch->level)
    {
	send_to_char("That character is too powerful for you to handle.\n\r",ch);
	return;
    }
*/
    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(ch,victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"{^$N{V switches into {#%s{V",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;

    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    if (IS_SET(ch->act, PLR_COLOUR))
        SET_BIT(victim->act,PLR_COLOUR);
    send_to_char( "Ok.\n\r", victim );
    return;
}


void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( "You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"{^$N returns from {#%s{V.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    REMOVE_BIT(ch->act,PLR_COLOUR);
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 105)
	|| (IS_TRUSTED(ch,DEMI)	    && obj->level <= 100)
	|| (IS_TRUSTED(ch,KNIGHT)    && obj->level <= 20)
	|| (IS_TRUSTED(ch,SQUIRE)   && obj->level ==  5))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;


    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}
	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot clone an exit object.\n\r",ch);
	    return;
	}
	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 100 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 90 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level > 85 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,KNIGHT))
	||  !IS_TRUSTED(ch,SQUIRE))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];


    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	if (ch->level >= CREATOR)
	    send_to_char("  load voodoo <player>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"voodoo") && (ch->level >= CREATOR))
    {
	do_vload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"randobj") && (ch->trust >= 110))
    {
	do_robjload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"randobj2") && (ch->trust >= 110))
    {
        do_robjload2(ch,argument);
	return;
    }


    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    sprintf(buf,"You load %s.",victim->short_descr);
    send_to_char( buf, ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    if (pObjIndex->item_type == ITEM_EXIT)
    {
	send_to_char("You cannot load an exit object.\n\r",ch);
	return;
    }

    if ( pObjIndex->area && ch->pcdata
    &&   !IS_BUILDER( ch, pObjIndex->area ) )
    {
	send_to_char( "Worry about your own areas for now...\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    obj->maker = ch->name;
    obj->made = (char *) ctime (&current_time);
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    act( "You have created $p!", ch, obj, NULL, TO_CHAR );
    return;
}

void do_vload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char *name;
   
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0')
    {
        send_to_char( "Syntax: load voodoo <player>\n\r", ch );
        return;
    }
    
    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (wch->level > ch->level)
		continue;

	    found = TRUE;
 
	    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_VOODOO ) ) == NULL )
	    {
		send_to_char( "Cannot find the voodoo doll vnum.\n\r", ch );
		return;
	    }
	    obj = create_object( pObjIndex, 0 );
	    name = wch->name;
	    sprintf( buf, obj->short_descr, name );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( buf );
	    sprintf( buf, obj->description, name );
	    free_string( obj->description );
	    obj->description = str_dup( buf );
	    sprintf( buf, obj->name, name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	    if ( CAN_WEAR(obj, ITEM_TAKE) )
		obj_to_char( obj, ch );
	    else
		obj_to_room( obj, ch->in_room );
	    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
	    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	    sprintf(buf, "You load %s.", obj->short_descr);
	    send_to_char( buf, ch );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }
    send_to_char("No one of that name is playing.\n\r",ch);
    return;
}

void do_robjload( CHAR_DATA *ch , char *argument )
{
	int level;
	char arg1[MSL];
	OBJ_DATA *obj;
	char buf[MSL];
	
	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' || !is_number(arg1))
	{
        	send_to_char( "Syntax: load randobj <level>.\n\r", ch );
        	return;
	}

        level = atoi(arg1);
        if (level < 0 || level > 200)
        {
          send_to_char( "Level must be be between 0 and 200.\n\r",ch);
          return;
        }

	obj = rand_obj(ch,level );

        obj->maker = str_dup(ch->name);

        if ( CAN_WEAR(obj, ITEM_TAKE) )
		obj_to_char( obj, ch );
	else
		obj_to_room( obj, ch->in_room );
	act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
	wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	sprintf( buf, "You load %s.", obj->short_descr);
	send_to_char( buf, ch );
	send_to_char( "Ok.\n\r", ch );
	return;
}

void do_robjload2( CHAR_DATA *ch , char *argument )
{
	int level;
	char arg1[MSL];
	OBJ_DATA *obj;
	char buf[MSL];
	
	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' || !is_number(arg1))
	{
                send_to_char( "Syntax: load randobj2 <level>.\n\r", ch );
        	return;
	}

        level = atoi(arg1);
        if (level < 0 || level > 200)
        {
          send_to_char( "Level must be be between 0 and 200.\n\r",ch);
          return;
        }

        obj = rand_obj2(ch, level, argument);

        if(obj == NULL)
        {
         send_to_char("NULL OBJECT.\n\r",ch);
         return;
        }

        obj->maker = str_dup(ch->name);

        if ( CAN_WEAR(obj, ITEM_TAKE) )
		obj_to_char( obj, ch );
	else
		obj_to_room( obj, ch->in_room );
	act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
	wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	sprintf( buf, "You load %s.", obj->short_descr);
	send_to_char( buf, ch );
	send_to_char( "Ok.\n\r", ch );
	return;
}


void do_randclan( CHAR_DATA *ch, char *argument )
{

    randomize_entrances( ROOM_VNUM_CLANS );
    send_to_char("Clan entrances have been moved.\n\r",ch);
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	if (get_trust(ch) <= DEITY)
	{
	  send_to_char("Not against PC's!\n\r",ch);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 110 )
    {
	send_to_char( "Level must be 1 to 110.\n\r", ch );
	return;
    }

    if ( level > ch->level )
    {
	send_to_char( "Limited to your level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "{R******** {GOOOOHHHHHHHHHH  NNNNOOOO {R*******{x\n\r", victim );
	sprintf(buf, "{R**** {WYou've been demoted to level %d {R****{x\n\r", level );
	send_to_char(buf, victim);
	if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 100;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level_quiet( victim );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "{B******* {GOOOOHHHHHHHHHH  YYYYEEEESSS {B******{x\n\r", victim );
	sprintf(buf, "{B**** {WYou've been advanced to level %d {B****{x\n\r", level );
	send_to_char(buf, victim);
	if ((victim->level > HERO) || (level > HERO))
	{
	    update_wizlist(victim, level);
	}
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level_quiet( victim );
    }
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    save_char_obj(victim);
    return;
}

void do_trust( CHAR_DATA *ch, char *argument ) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 111 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 111.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;


    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
	    if ( IS_SET(vch->act, PLR_NORESTORE)  
	    || (vch->in_room != NULL && IS_SET(vch->in_room->room_flags, ROOM_ARENA))
	    || (vch->fight_timer > 0))
	    {
		act("$n attempts to restore you, but fails.",ch,NULL,vch,TO_VICT);
	    } else {
		affect_strip(vch,gsn_plague);
		affect_strip(vch,gsn_poison);
		affect_strip(vch,gsn_blindness);
		affect_strip(vch,gsn_sleep);
		affect_strip(vch,gsn_curse);

		vch->hit 	= vch->max_hit;
		vch->mana	= vch->max_mana;
		vch->move	= vch->max_move;
		update_pos( vch);
		act("$n has restored you.",ch,NULL,vch,TO_VICT);
	    }
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 2 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            if ( IS_SET(victim->act, PLR_NORESTORE)
	    	||   (victim->in_room != NULL && IS_SET(victim->in_room->room_flags, ROOM_ARENA)) 
	    	|| (victim->fight_timer > 0))
            {
                act("$n attempts to restore you, but fails.",ch,NULL,victim,TO_VICT);          
            } else {
		affect_strip(victim,gsn_plague);
		affect_strip(victim,gsn_poison);
		affect_strip(victim,gsn_blindness);
		affect_strip(victim,gsn_sleep);
		affect_strip(victim,gsn_curse);
            
		victim->hit 	= victim->max_hit;
		victim->mana	= victim->max_mana;
		victim->move	= victim->max_move;
		update_pos( victim);
		if (victim->in_room != NULL)
		    act("$n has restored you.",ch,NULL,victim,TO_VICT);
	    }
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NORESTORE)
    ||   (victim->in_room != NULL && IS_SET(victim->in_room->room_flags, ROOM_ARENA)) )
    {
	act("$n attempts to restore you, but fails.",ch,NULL,victim,TO_VICT);
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_immkiss( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );
    if (arg[0] == '\0')
    {
	send_to_char( "Who do you want to kiss?\n\r", ch );
	return;
    }
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( ch->in_room != victim->in_room )
    {
	send_to_char( "Your lips aren't that long!\n\r", ch);
	return;
    }
    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n kisses you, and you feel a sudden rush of adrenaline.", ch, NULL, victim, TO_VICT );
    send_to_char( "You feel MUCH better now!\n\r", victim);
    send_to_char( "They feel MUCH better now!\n\r", ch);
    sprintf(buf,"$N immkissed %s",
        IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}

void do_norestore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Norestore whom?\n\r", ch );
        return;
    }
       
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
       
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_NORESTORE) )
    {
        REMOVE_BIT(victim->act, PLR_NORESTORE);
        send_to_char( "NORESTORE removed.\n\r", ch );
        sprintf(buf,"$N allows %s restores.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->act, PLR_NORESTORE);
        send_to_char( "NORESTORE set.\n\r", ch );
        sprintf(buf,"$N denys %s restores.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
       
    save_char_obj( victim );
 
    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }
       
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
       
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
       
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
       
    if ( IS_SET(victim->act, PLR_NOTITLE) )
    {
        REMOVE_BIT(victim->act, PLR_NOTITLE);
        send_to_char( "NOTITLE removed.\n\r", ch );
        sprintf(buf,"$N allows %s title.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->act, PLR_NOTITLE);
        send_to_char( "NOTITLE set.\n\r", ch );
        sprintf(buf,"$N denys %s title.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
       
    save_char_obj( victim );
 
    return;
}

void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

	if (ch == victim)
    {
	    send_to_char( "{RNot on yourself. Sorry.{x\n\r", ch);
	return;
	}
    
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "{BLOG has been removed.{x\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "{RLOG has been set.{x\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;


    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL ) {
	    stop_fighting( rch, TRUE );
	    if (!IS_NPC(rch) ) {
		send_to_char( "Ok.\n\r", ch );
	    }
	}
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;


    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 

    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];


    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	send_to_char("  set char  <name> <field> <value>\n\r",ch);
        send_to_char("  set ability <name> <field> <value>\n\r",ch);
        send_to_char("  set learnlvl <name> <field> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"learnlvl"))
    {
        do_lset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"ability"))
    {
        do_aset(ch,argument);
	return;
    }

    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
            {
                victim->pcdata->learned[sn] = value;
                victim->pcdata->learnlvl[sn] = skill_table[sn].skill_level[victim->class];
            }
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
        victim->pcdata->learnlvl[sn] = skill_table[sn].skill_level[victim->class];
    }

    return;
}

void do_lset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
        send_to_char( "  set learnlvl <name> <spell or skill> <value>\n\r", ch);
        send_to_char( "  set learnlvl <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
        send_to_char( "Value range is -1 to 110.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
                victim->pcdata->learnlvl[sn]     = value;
	}
    }
    else
    {
        victim->pcdata->learnlvl[sn] = value;
    }

    return;
}


void do_aset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
        send_to_char( "  set ability <name> <field> <value>\n\r", ch);
        send_to_char( "  set ability <name> all <value>\n\r",ch);  
        send_to_char("   viper crab crane mongoose bull\n\r",ch);
        send_to_char("   tiger dragon monkey swallow mantis\n\r",ch);
        send_to_char("   white black orange red blue green\n\r",ch);
        send_to_char("   toughness\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );

    if ( fAll )
    {
     for(sn =0; sn < 6; sn++)
     victim->magic[sn] = value;

     for(sn =1; sn < 11; sn++)
     victim->stance[sn] = value;

    }
    else /* Look for the argument */
    {
     if ( !str_cmp( arg2, "viper" ) )
     {
 	if ( IS_NPC(victim) )
 	{
 	    send_to_char( "Not on NPC's.\n\r", ch );
 	    return;
 	}
 
	if ( value < 0 || value > 200 )
 	{
 	    send_to_char( "Stance Viper range is 0 to 200.\n\r", ch );
 	    return;
 	}

        victim->stance[STANCE_VIPER] = value;
        send_to_char("Ok.\n\r",ch);
   	return;
    }
    if ( !str_cmp( arg2, "crane" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Crane range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_CRANE] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "crab" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Crab range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_CRAB] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "mongoose" ) )
    {
	if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
  	    return;
  	}
  
  	if ( value < 0 || value > 200 )
  	{
  	    send_to_char( "Stance Mongoose range is 0 to 200.\n\r", ch );
  	    return;
  	}
        victim->stance[STANCE_MONGOOSE] = value;
        send_to_char("Ok.\n\r",ch);
  	return;
    }
    if ( !str_cmp( arg2, "bull" ) )
    {
  	if ( IS_NPC(victim) )
  	{
  	    send_to_char( "Not on NPC's.\n\r", ch );
  	    return;
  	}

	if ( value < 0 || value > 200 )
  	{
  	    send_to_char( "Stance Bull range is 0 to 200.\n\r", ch );
  	    return;
  	}
        victim->stance[STANCE_BULL] = value;
        send_to_char("Ok.\n\r",ch);
  	return;
    }
    if ( !str_cmp( arg2, "mantis" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
        }

	if ( value < 0 || value > 200 )
 	{
 	    send_to_char( "Stance Mantis range is 0 to 200.\n\r", ch );
 	    return;
        }
        victim->stance[STANCE_MANTIS] = value;
        victim->stance[STANCE_CRANE]  = 200;
        victim->stance[STANCE_VIPER]  = 200;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "dragon" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Dragon range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_DRAGON] = value;
        victim->stance[STANCE_CRAB]   = 200;
        victim->stance[STANCE_BULL]   = 200;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "tiger" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Tiger range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_TIGER] = value;
        victim->stance[STANCE_BULL]  = 200;
        victim->stance[STANCE_VIPER] = 200;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "monkey" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Monkey range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_MONKEY] = value;
        victim->stance[STANCE_MONGOOSE]  = 200;
        victim->stance[STANCE_CRANE]  = 200;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "swallow" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
	if ( value < 0 || value > 200 )
	{
	    send_to_char( "Stance Swallow range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->stance[STANCE_SWALLOW] = value;
        victim->stance[STANCE_CRAB]  = 200;
        victim->stance[STANCE_MONGOOSE]  = 200;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "white" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 1 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
 	    return;
 	}
        victim->magic[MAGIC_WHITE] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "red" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->magic[MAGIC_RED] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "blue" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->magic[MAGIC_BLUE] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "green" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->magic[MAGIC_GREEN] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "black" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->magic[MAGIC_BLACK] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "orange" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 200 )
	{
            send_to_char( "Spell range is 0 to 200.\n\r", ch );
	    return;
	}
        victim->magic[MAGIC_ORANGE] = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
    if ( !str_cmp( arg2, "toughness" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
        if ( value < 0 || value > 500 )
	{
            send_to_char( "Toughness range is 0 to 500.\n\r", ch );
	    return;
	}
        victim->toughness = value;
        send_to_char("Ok.\n\r",ch);
	return;
    }
   }
    /*
     * Generate usage message.
     */
    do_aset( ch, "" );
    return;
}




void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex class level god\n\r",	ch );
	send_to_char( "    race group platinum gold silver hp\n\r",	ch );
        send_to_char( "    mana move prac align train thirst bounty\n\r",      ch );
	send_to_char( "    hunger drunk full quest aquest security\n\r",	        ch );
        send_to_char( "    pdeath pkill prank arank qtimer pktimer\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL 
    || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "Done.\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity ranges is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > ch->level )
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", ch->level );
	    send_to_char(buf,ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if (!str_prefix(arg2,"god"))
    {
        int god;

        god = god_lookup(arg3);

        if( god == 0 )
        {
            send_to_char("That is not a valid god for that character.\n\r",ch);
            return;
        }

        victim->god = god;
        return;
    }
    
    if ( !str_prefix ( arg2, "qtimer" ) )
    {
       victim->nextquest = value;
       return;
    }

    if ( !str_prefix( arg2, "platinum" ) )
    {
	victim->platinum = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 50000 )
	{
	    send_to_char( "Hp range is -10 to 50,000 hit points.\n\r", ch );
	    victim->max_hit = 50000;
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 50000 )
	{
	    send_to_char( "Mana range is 0 to 50,000 mana points.\n\r", ch );
	    victim->max_mana=50000;
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 50000 )
	{
	    send_to_char( "Move range is 0 to 50,000 move points.\n\r", ch );
	    victim->max_move = 50000;
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if ( victim->pet != NULL )
	    victim->pet->alignment = victim->alignment;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if ( !str_prefix( arg2, "quest" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "NPC's don't need quest points.\n\r", ch );
	    return;
	}

	victim->qps = value;
	return;
    }

    if ( !str_prefix( arg2, "aquest" ) )
    {
        if ( IS_NPC(victim) )
        {
          send_to_char( "NPC's don't need auto-quest points.\n\r", ch );
          return;
        }
        victim->questpoints = value;
        return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if ( !str_prefix( arg2, "bounty" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 || value > 1000 )
	{
            sprintf(buf, "Bounty range is 0 to %d.\n\r", 1000 );
	    send_to_char(buf,ch);
	    return;
	}
        victim->pcdata->bounty = value;
	return;
    }

    if ( !str_prefix( arg2, "pdeath" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 )
	{
            send_to_char("Pdeath must be 0 or greater.\n\r", ch);
	    return;
	}
        victim->pcdata->pdeath = value;
        sprintf(buf,"%s's pDeaths set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "pkill" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 )
	{
            send_to_char("Pkill must be 0 or greater.\n\r", ch);
	    return;
	}
        victim->pcdata->pkills = value;
        sprintf(buf,"%s's Pkills set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
	return;
    }
    if ( !str_prefix( arg2, "arank" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 )
	{
            send_to_char("Arena Rank must be 0 or greater.\n\r", ch);
	    return;
	}
        victim->pcdata->arank = value;
        sprintf(buf,"%s's Arena Rank set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
	return;
    }
    if ( !str_prefix( arg2, "prank" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 )
	{
            send_to_char("PK Rank must be 0 or greater.\n\r", ch);
	    return;
	}
        victim->pcdata->prank = value;
        sprintf(buf,"%s's PK Rank set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
	return;
    }
    if ( !str_prefix( arg2, "pktimer" ) )
    {
        if ( IS_NPC(victim) )
	{
            send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

        if ( value < 0 )
	{
            send_to_char("PK Timer must be 0 or greater.\n\r", ch);
	    return;
	}
        victim->fight_timer = value;
        sprintf(buf,"%s's PK Timer set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
	return;
    }





    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_rename( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    char name [MAX_INPUT_LENGTH];
    char oldname [MAX_INPUT_LENGTH];
    char strsave [MAX_STRING_LENGTH];
    char errmsg[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;
    int clead;


    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );
    sprintf(errmsg,"Rename check1");
    log_string( errmsg );
    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  rename <char> <new name>\n\r",ch);
	return;
    }
    sprintf(errmsg,"Rename check2");
    log_string( errmsg );
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    sprintf(errmsg,"Rename check3");
    log_string( errmsg );
    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    sprintf(errmsg,"Rename check4");
    log_string( errmsg );
    if (victim->level >= ch->level)
    {
	send_to_char( "Nope, ain't gonna do it.\n\r", ch );
	return;
    }
    sprintf(errmsg,"Rename check5");
    log_string( errmsg );
    if ( check_char_exist( arg2 ) )
    {
	send_to_char( "That name is already in use, or is illegal.\n\r", ch );
	return;
    }
        sprintf(errmsg,"Rename check6");
    log_string( errmsg );
    sprintf(name, "%s", str_dup(capitalize(arg2)));
    sprintf(errmsg,"Rename check7");
    log_string( errmsg );
    sprintf(oldname, "%s", str_dup(victim->name));
        sprintf(errmsg,"Rename check8");
    log_string( errmsg );
    sprintf(strsave, "%s/%s/%s", PLAYER_DIR, initial (victim->name), capitalize( victim->name ) );
        sprintf(errmsg,"Rename check9");
    log_string( errmsg );
    clan = 0;
    clead = 0;
    if (is_clead(victim))
    {
	clead = victim->clead;
    }
    if (is_clan(victim))
    {
	clan = victim->clan;
     check_new_clan(oldname, 999, 0);
    }
    sprintf(errmsg,"Rename check10");
    log_string( errmsg );
    victim->name = str_dup(name);
    victim->clead = clead;
    victim->clan = clan;
    sprintf(errmsg,"Rename check11");
    log_string( errmsg );
    check_new_clan(victim->name, victim->clan, victim->rank);
    sprintf(errmsg,"Rename check12");
    log_string( errmsg );
    do_save(victim,"");
    sprintf(errmsg,"Rename check13");
    log_string( errmsg );
    sprintf(buf, "Your name has been changed to '%s'.\n\r", name );
    send_to_char( buf, victim );
    send_to_char( "Ok.\n\r", ch);
    sprintf(errmsg,"Rename check14");
    log_string( errmsg );
    unlink(strsave);
    sprintf(errmsg,"Rename check15");
    log_string( errmsg );
    return;
}


void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_STRING_LENGTH];
    char buf [MAX_STRING_LENGTH];
    char buf2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int cnt, plc;


    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
        send_to_char("    fields: name short long title who spec pretitle\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL 
	   || (get_trust( victim ) > get_trust( ch) /* && get_trust( victim ) == MAX_LEVEL */ ))
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

	if ((victim->level >= ch->level) && (ch != victim))
	{
	    send_to_char("That will not be done.\n\r", ch);
	    return;
	}
    	
    	if ( !str_prefix( arg2, "who" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }
	    if ((ch->level < CREATOR)
	    && (victim->level < HERO)
	    && (victim->pcdata->tier == 1))
	    {
		send_to_char( "Not on 1st tier mortals.\n\r", ch);
		return;
	    }
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    victim->pcdata->who_descr = str_dup( "" );
	    if (arg3[0] == '\0')
	    {
		return;
	    }
	    cnt = 0;
	    for(plc = 0; plc < strlen(arg3); plc++)
	    {
		if (arg3[plc] != '{')
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "%c", arg3[plc]);
		    } else
		    {
			sprintf(buf2, "%s%c", buf, arg3[plc]);
		    }
		    sprintf(buf, "%s", buf2);
		    cnt++;
		} else if (arg3[plc+1] == '{')
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "{{");
		    } else
		    {
			sprintf(buf2, "%s{{", buf);
		    }
		    sprintf(buf, "%s", buf2);
		    cnt++;
		    plc++;
		} else
		{
		    if (buf[0] == '\0')
		    {
			sprintf(buf2, "{%c", arg3[plc+1]);
		    } else
		    {
			sprintf(buf2, "%s{%c", buf, arg3[plc+1]);
		    }
		    sprintf(buf, "%s", buf2);
		    plc++;
		}
            if (IS_IMMORTAL(victim))
            {
                if (cnt >= 20)
		{
		    plc = strlen(arg3);
		}
            }
            else
            {
            if (cnt >=10)
            {
		    plc = strlen(arg3);
		}
            }
	    }
	    sprintf(buf2, "%s{0", buf);
	    sprintf(buf, "%s", buf2);
          if (IS_IMMORTAL(victim))
          {
            while (cnt < 20)
	    {
		sprintf(buf2, "%s ", buf);
		sprintf(buf, "%s", buf2);
		cnt++;
	    }
          }
          else
          {
          while (cnt < 10)
	    {
		sprintf(buf2, "%s ", buf);
		sprintf(buf, "%s", buf2);
		cnt++;
	    }
          }
    	    victim->pcdata->who_descr = str_dup(buf);
	    buf[0] = '\0';
	    buf2[0] = '\0';
    	    return;
    	}

	if (arg3[0] == '\0')
	{
	    do_string(ch,"");
	    return;
	}

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

/*        if ( !str_prefix( arg2, "pretitle" ) )
        {
            if ( IS_NPC(victim) )
            {
                send_to_char( "Not on NPC's.\n\r", ch );
                return;
            }
            victim->pcdata->pretit = str_dup( arg3 );
            return;
        } */

    	if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (arg3[0] == '\0')
    {
	do_string(ch,"");
	return;
    }
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	if (obj->item_type == ITEM_EXIT)
	{
	    send_to_char("You cannot modify exit objects.\n\r",ch);
	    return;
	}
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;
    int clan;
    int class;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    level weight cost timer clan guild\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }
    if (obj->item_type == ITEM_EXIT)
    {
	send_to_char( "You cannot modify exit objects.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "clan" ) )
    {
	if (!str_prefix(arg3,"none"))
	{
	    obj->clan = 0;
	    return;
	}
	if ((clan = clan_lookup(arg3)) == 0)
	{
	    send_to_char("No such clan exists.\n\r",ch);
	    return;
	}
	obj->clan = clan;
	return;
    }
    if ( !str_prefix( arg2, "guild" ) )
    {
	if (!str_prefix(arg3,"none"))
	{
	    obj->class = 0;
	    return;
	}
	if ((class = class_lookup(arg3)) == 0)
	{
	    send_to_char("No such guild exists.\n\r",ch);
	    return;
	}
	obj->class = class;
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    obj->value[0] = UMIN(MAX_WEAPON,value);
	    obj->value[0] = UMAX(0,obj->value[0]);
	    return;
	}
	if ((obj->item_type == ITEM_WAND)
	||  (obj->item_type == ITEM_STAFF)
	||  (obj->item_type == ITEM_POTION)
	||  (obj->item_type == ITEM_SCROLL)
	||  (obj->item_type == ITEM_PILL))
	{
	    obj->value[0] = UMIN(MAX_LEVEL,value);
	    obj->value[0] = UMAX(0,obj->value[0]);
	    return;
	}
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	if ((obj->item_type == ITEM_FOUNTAIN)
	||  (obj->item_type == ITEM_DRINK_CON))
	{
	    obj->value[2] = UMIN(MAX_LIQUID,value);
	    obj->value[2] = UMAX(0,obj->value[2]);
	    return;
	}
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	if (obj->item_type == ITEM_WEAPON)
	{
	    obj->value[3] = UMIN(MAX_DAMAGE_MESSAGE,value);
	    obj->value[3] = UMAX(0,obj->value[3]);
	    return;
	}
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( (get_trust( ch ) < CREATOR && (obj->pIndexData->level - 5) > value)
	&& !IS_SET(ch->act, PLR_KEY) )
	{
	    send_to_char("You may not lower an item more than 5 levels!\n\r",ch);
	    return;
	}
        if ( (get_trust( ch ) == CREATOR && (obj->pIndexData->level - 10) > value)
	&& !IS_SET(ch->act, PLR_KEY) )
        { 
            send_to_char("You may not lower an item more than 10 levels!\n\r",ch);
            return; 
        }
	obj->level = UMIN(MAX_LEVEL,value);
	obj->level = UMIN(0,obj->level);
        if ( obj->level == 0 ) obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(ch,location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	send_to_char( "Use the flag command instead.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}

/* Written by Stimpy, ported to rom2.4 by Silverhand 3/12
 *
 *	Added the other COMM_ stuff that wasn't defined before 4/16 -Silverhand
 */
void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    char *          st;
    char            s[100];
    char            idle[10];
    bool            showIP = FALSE;

    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    if(!str_cmp(argument,"showip"))
     showIP = TRUE;

    if(!showIP)
    strcat( buf2, "\n\r[Num Connected_State Login@ Idl] Player Name Host\n\r" );
    else
    strcat( buf2, "\n\r[Num Connected_State Login@ Idl] Player Name HostIP\n\r" );

    strcat( buf2,
"--------------------------------------------------------------------------\n\r");  
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->character && can_see( ch, d->character ) 
	&& get_trust(ch) >= get_trust(d->character))
        {
           /* NB: You may need to edit the CON_ values */
           switch( d->connected )
           {
              case CON_PLAYING:              st = "    PLAYING    ";    break;
              case CON_GET_NAME:             st = "   Get Name    ";    break;
              case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
              case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
              case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
              case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
              case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
              case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
              case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
              case CON_GET_ALIGNMENT:        st = " Get New Align ";    break;
              case CON_DEFAULT_CHOICE:       st = " Choosing Cust ";    break;
              case CON_GEN_GROUPS:           st = " Customization ";    break;
              case CON_PICK_WEAPON:          st = " Picking Weapon";    break;
              case CON_READ_IMOTD:           st = " Reading IMOTD ";    break;
              case CON_BREAK_CONNECT:        st = "   LINKDEAD    ";    break;
              case CON_READ_MOTD:            st = "  Reading MOTD ";    break;
              default:                       st = "   !UNKNOWN!   ";    break;
           }
           count++;
           
           /* Format "login" value... */
           vch = d->original ? d->original : d->character;
           strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
           
           if ( vch->timer > 0 )
              sprintf( idle, "%-2d", vch->timer );
           else
              sprintf( idle, "  " );

	if (!str_cmp(d->character->name, "Alina") )
           sprintf( buf, "[%3d %s %7s %2s] %-12s k17b4.AD7.spar.edu\n\r",
              d->descriptor,
              st,
              s,
              idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)"  );
	else if (!str_cmp(d->character->name, "Corlan") )
           sprintf( buf, "[%3d %s %7s %2s] %-12s icenet.pp7.ibf.au\n\r",
              d->descriptor,
              st,
              s,
              idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)"  );
	else if (!str_cmp(d->character->name, "Electrum") )
           sprintf( buf, "[%3d %s %7s %2s] %-12s 176.83.32.7\n\r",
              d->descriptor,
              st,
              s,
              idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)"  );
	else
           sprintf( buf, "[%3d %s %7s %2s] %-12s %-32.32s\n\r",
              d->descriptor,
              st,
              s,
              idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)",
              showIP ? d->hostip : d->host );

              
           strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;
}

/*
void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    count	= 0;
    buf[0]	= '\0';

    one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL && can_see( ch, d->character ) 
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name)))
	&& !IS_NPC(d->character)
	&& (d->character->level <= ch->level))
	{
	    count++;
	    sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host
		);
	}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}
*/


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char wizbuf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2,"reroll"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if (!IS_TRUSTED(ch,IMPLEMENTOR))
    {
       sprintf( wizbuf, "$N forces %s to %s.", arg, argument );
       wiznet( wizbuf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && can_over_ride( ch, vch, FALSE ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && can_over_ride( ch, vch, FALSE )
	    &&   vch->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) && can_over_ride( ch, vch, FALSE )
            &&   vch->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;
//booya

	if ( ( victim = get_char_world( ch, arg ) ) == NULL 
	|| (victim->level >= ch->level && victim->level == MAX_LEVEL))
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(ch,victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if ( !can_over_ride( ch, victim, FALSE ) )
        {
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( ch->invis_level)
      {
          ch->invis_level = 0;
          if (!IS_TRUSTED(ch,IMPLEMENTOR))
          {
	     act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
          }
          else 
          {
             act( "$n appears in a blinding {z{Wflash{x!", ch, NULL, NULL, TO_ROOM );
	  }
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
          if (!IS_TRUSTED(ch,IMPLEMENTOR)) 
          {
	     act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  }
          else 
          { 
             act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM );
          }
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
          ch->invis_level = get_trust(ch);
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  if (!IS_TRUSTED(ch,IMPLEMENTOR))
	  {
             act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  }
	  else
	  {
             act( "A {Wblinding white light{x envelops $n, then {z{Dvanishes{x.", ch, NULL, NULL, TO_ROOM );
	  }
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
          ch->reply = NULL;
          ch->invis_level = level;
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = get_trust(ch);
          ch->ghost_level = 0;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          ch->ghost_level = 0;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}

void do_ghost( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->ghost_level)
      {
          ch->ghost_level = 0;
          act( "$n steps out from the mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You step out from the mist.\n\r", ch );
      }
      else
      {
          ch->ghost_level = get_trust(ch);
          ch->incog_level = 0;
          act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You vanish into a mist.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Ghost level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->ghost_level = level;
          ch->incog_level = 0;
          act( "$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You vanish into a mist.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_mquest (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;


    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a quest item of what?\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_QUEST))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUEST);
	act("$p is no longer a quest item.",ch,obj,NULL,TO_CHAR);
    }
    else
    {
	SET_BIT(obj->extra_flags,ITEM_QUEST);
	act("$p is now a quest item.",ch,obj,NULL,TO_CHAR);
    }

    return;
}

void do_mpoint (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;


    if ( argument[0] == '\0' )
    {
        send_to_char( "Make a questpoint item of what?\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_QUESTPOINT))
    {
	REMOVE_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is no longer a questpoint item.",ch,obj,NULL,TO_CHAR);
    }
    else
    {
	SET_BIT(obj->extra_flags,ITEM_QUESTPOINT);
	act("$p is now a questpoint item.",ch,obj,NULL,TO_CHAR);
    }

    return;
}

void do_gset (CHAR_DATA *ch, char *argument)
{
    if ( IS_NPC(ch) )
	return;

    if ( ( argument[0] == '\0' ) || !is_number( argument ) )
    {
        send_to_char( "Goto point cleared.\n\r", ch );
	ch->pcdata->recall = 0;
        return;
    }

    ch->pcdata->recall = atoi(argument);

    send_to_char( "Ok.\n\r", ch );

    return;
}

void do_wizslap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    AFFECT_DATA af;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "WizSlap whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level >= ch->level && IS_IMMORTAL(ch))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    pRoomIndex = get_random_room(victim);

    act( "$n slaps you, sending you reeling through time and space!", ch, NULL, victim, TO_VICT);
    act( "$n slaps $N, sending $M reeling through time and space!", ch, NULL, victim, TO_NOTVICT );
    act( "You send $N reeling through time and space!", ch, NULL, victim, TO_CHAR );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n crashes to the ground!", victim, NULL, NULL, TO_ROOM );
    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("weaken");
    af.level     = 110;
    af.duration  = 5;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (105 / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your god's might compare to yours.\n\r", victim );

    af.where     = TO_AFFECTS;
    af.type      = skill_lookup("curse");
    af.level     = 115;
    af.duration  = 5;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (105 / 8);
    af.bitvector = AFF_CURSE; 
    affect_to_char( victim, &af );
    
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 25; 
    affect_to_char( victim, &af );

    send_to_char( "You feel the curse of the gods upon you.\n\r", victim);
    do_look( victim, "auto" );
    return;
}
void do_pandaslap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "PandaSlap whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level >= ch->level && IS_IMMORTAL(ch))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
   // pRoomIndex = get_random_room(victim);
      pRoomIndex = get_room_index (41000);

    act( "$n appears out from behind some bamboo and slaps you, sending you reeling through the jungle!", ch, NULL, victim, TO_VICT);
    act( "$n appears out from behind some bamboo and slaps $N, sending $M reeling through the jungle!", ch, NULL, victim, TO_NOTVICT );
    act( "You send $N reeling through the jungle!", ch, NULL, victim, TO_CHAR );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n crashes to the ground!", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}
void do_pack ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *pack;
    OBJ_DATA *obj;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Send a survival pack to whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (victim->level >= 10) && (ch->level < DEMI) )
    {
	send_to_char("They don't need one at thier level.\n\r", ch);
	return;
    }

    if (!can_pack(victim) )
    {
	send_to_char("They already have a survival pack.\n\r",ch);
	return;
    }

    pack = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_PACK), 0 );
    pack->level = 5;

    for (i = 0; i < 7; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_A), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_B), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_C), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_D), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_E), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_F), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_G), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_H), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_I), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_J), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_K), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_L), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_M), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_N), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_O), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_P), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_Q), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_R), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_S), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_T), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_U), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    for (i = 0; i < 2; i++)
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_V), 0 );
	obj->level = 5;
	obj_to_obj( obj, pack );
    }
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_W), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );
    obj = create_object( get_obj_index(OBJ_VNUM_SURVIVAL_X), 0 );
    obj->level = 5;
    obj_to_obj( obj, pack );

    obj_to_char( pack, victim );

    send_to_char("Ok.\n\r", ch);
    act( "$p suddenly appears in your inventory.", ch, pack, victim, TO_VICT);
    return;
}


bool can_pack(CHAR_DATA *ch)
{
    OBJ_DATA *object;
    bool found;
 
    if ( ch->desc == NULL )
        return TRUE;
 
    if ( ch->level > HERO )
	return TRUE;

    /*
     * search the list of objects.
     */
    found = TRUE;
    for ( object = ch->carrying; object != NULL; object = object->next_content )
    {
    	if (object->pIndexData->vnum == OBJ_VNUM_SURVIVAL_PACK)
	    found = FALSE;
    }
    if (found)
	return TRUE;
 
    return FALSE;
}

void do_dupe(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH],arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int pos;
    bool found = FALSE;

    if (IS_NPC(ch))
	return;

    smash_tilde( argument );

    argument = one_argument(argument,arg);
    one_argument(argument,arg2);
    
    if (arg[0] == '\0')
    {
	send_to_char("Dupe whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	if (victim->pcdata->dupes[0] == NULL)
	{
	    send_to_char("They have no dupes set.\n\r",ch);
	    return;
	}
	send_to_char("They currently have the following dupes:\n\r",ch);

	for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (victim->pcdata->dupes[pos] == NULL)
		break;

	    sprintf(buf,"    %s\n\r",victim->pcdata->dupes[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_DUPES; pos++)
    {
	if (victim->pcdata->dupes[pos] == NULL)
	    break;

	if (!str_cmp(arg2,victim->pcdata->dupes[pos]))
	{
	    found = TRUE;
	}
    }

    if (found)
    {
	found = FALSE;
	for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (victim->pcdata->dupes[pos] == NULL)
		break;

	    if (found)
	    {
		victim->pcdata->dupes[pos-1]		= victim->pcdata->dupes[pos];
		victim->pcdata->dupes[pos]		= NULL;
		continue;
	    }

	    if(!strcmp(arg2,victim->pcdata->dupes[pos]))
	    {
		send_to_char("Dupe removed.\n\r",ch);
		free_string(victim->pcdata->dupes[pos]);
		victim->pcdata->dupes[pos] = NULL;
		found = TRUE;
	    }
	}
	return;
    }

    for (pos = 0; pos < MAX_DUPES; pos++)
    {
	if (victim->pcdata->dupes[pos] == NULL)
	    break;
     }

     if (pos >= MAX_DUPES)
     {
	send_to_char("Sorry, they've reached the limit for dupes.\n\r",ch);
	return;
     }
  
     /* make a new dupe */
     victim->pcdata->dupes[pos]		= str_dup(arg2);
     sprintf(buf,"%s now has the dupe %s set.\n\r",victim->name,arg2);
     send_to_char(buf,ch);
}

void do_astat( CHAR_DATA *ch, char *argument )
{
}

void do_aclear( CHAR_DATA *ch, char *argument )
     {
     }

void do_abusy( CHAR_DATA *ch, char *argument )
     {
     }

void do_alock( CHAR_DATA *ch, char *argument )
     {
     }

void do_awipe (CHAR_DATA* ch, char *argument) 

{
}

void do_guild( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int clan;
        extern sh_int clan_members[MAX_CLAN];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
	send_to_char( "Syntax: {Gguild {c<{wchar{c> <{wclan name{c>{x\n\r",ch);
	return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
	send_to_char( "They aren't playing.\n\r", ch );
	return;
	}

      

	if (!str_prefix(arg2,"none"))
	{
		/* Remove Outcast flag */
		if(victim->clan == OUTCAST)
		{
                clan_members[victim->clan] -= 1;
                sprintf(buf,"You remove the {GOUTCAST{x flag from %s\n\r",victim->name);
		send_to_char(buf,ch);
		send_to_char("Your shame as an {GOUTCAST{x has ended.\n\r", victim);
		victim->clan = INDEP;
		victim->rank = MEMBER;
		return;
		}

	send_to_char("They are now clanless.\n\r",ch);
	send_to_char("You are now a member of no clan!\n\r",victim);

        if(victim->clan != INDEP)   // Do member reduction first
        clan_members[victim->clan] -= 1;

	victim->clan = INDEP;
	victim->rank = MEMBER;
        check_new_clan(victim->name, victim->clan, victim->rank);
	return;
	}

        if ((clan = clan_lookup(arg2)) == INDEP)
	{
	send_to_char("No such clan exists.\n\r",ch);
	return;
	}

	if (clan_table[clan].independent)
	{
	sprintf(buf,"They are now a {G%s{x.\n\r", clan_table[clan].name);
	send_to_char(buf,ch);
	sprintf(buf,"You are now a {G%s{x.\n\r", clan_table[clan].name);
	send_to_char(buf,victim);
	}

	else
	{
	sprintf(buf,"They are now a proud member of clan {G%s{x.\n\r", clan_table[clan].name);
	send_to_char(buf,ch);
	sprintf(buf,"You are now a proud member of clan {G%s{x.\n\r", clan_table[clan].name);
	send_to_char(buf,victim);
	}
        
        if(is_clan(victim)) // Reduce old clan first
         clan_members[victim->clan] -= 1;

        victim->clan = clan;
	victim->rank = MEMBER; /* make victim a member of the CLAN */
     check_new_clan(victim->name, victim->clan, victim->rank);
        clan_members[victim->clan] += 1;

}


/*
 * used by imms to reset, open, or close the arena
 */
void do_arena(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	switch (arg1[0])
	{
		case 'r':	case 'R':
			// reset: clear everyone out, reset to closed
			if (arena == FIGHT_LOCK)
			{
				send_to_char("Arena is already closed.\n\r", ch );
				return;
			}
			break;

		case 'c':	case 'C':
			// close: set the arena to closed
			if (arena == FIGHT_LOCK)
			{
				send_to_char("Arena is already closed.\n\r", ch );
				return;
			}

			if ((arena == FIGHT_BUSY) || (arena == FIGHT_START))
			{
				send_to_char("Arena is busy.  Use 'arena reset' instead.\n\r", ch );
				return;
			}

			arena = FIGHT_LOCK;
			send_to_char("Arena closed.\n\r", ch );

			break;

		case 'o':	case 'O':
			// open: set arena to open
			if (arena == FIGHT_OPEN)
			{
				send_to_char("Arena is already open.\n\r", ch );
				return;
			}

			if ((arena == FIGHT_BUSY) || (arena == FIGHT_START))
			{
				send_to_char("Arena is already open.\n\r", ch );
				return;
			}

			arena = FIGHT_OPEN;
			send_to_char("Arena opened.\n\r", ch );

			break;

		default:
			send_to_char("arena <reset/open/close>\n\r", ch );
			return;
			break;
	}
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		send_to_char("addlag to who?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They're not here.", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("That makes a LOT of sense.", ch);
		return;
	}

      if (victim->level >= ch->level)
      {
            send_to_char("NO!\n\r",ch);
            return;
      }

        if (x > 5000)
	{
		send_to_char("There's a limit to cruel and unusual punishment", ch);
		return;
	}

	send_to_char("Somebody REALLY didn't like you", victim);
	WAIT_STATE(victim, x);
	send_to_char("Adding lag now...", ch);
	return;
}

void do_backu( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to BACKUP, spell it out.\n\r", ch
);
    return;
}

void do_backup( CHAR_DATA *ch, char *argument )
{
 FILE *fp;
 DESCRIPTOR_DATA *d,*d_next;
 char arg[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH];
 char strsave[MAX_STRING_LENGTH];
 char *strfl;
 int value;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if (arg[0] == '\0')
  {
    send_to_char("Syntax:\n\n\r",ch);
    send_to_char("backup list\n\rbackup <1-3> write\n\r", ch);
    send_to_char("backup <1-3> erase\n\n\r",ch);
    send_to_char(
     "Writing to a backup lags the mud for a period, so all players ",ch);
    send_to_char("automatically informed that one is taking place. ",ch);
    send_to_char("Backup list simply displays the status of all ",ch);
    send_to_char("backups in the backup directory.  Erase wipes ",ch);
    send_to_char("the slot but is unecessary if you are simply ",ch); 
    send_to_char("going to make a new one in the slot because write ",ch);
    send_to_char("erases the slot before making a new backup.\n\r",ch);

    return;
  }

  if (!str_cmp(arg,"list"))
  {
    send_to_char("Backup #   Name       Time of Backup\n\r",ch);
    send_to_char("--------   ---------- ------------------------\n\r",ch);
    for (value = 1; value < 4; value++)
    {
     strsave[0]='\0';
     sprintf(strsave, "%s/backup%d.txt",BACKUP_DIR,value);
     fp = fopen(strsave,"r");
     if (!fp)
     {
        sprintf(buf, "Backup#%d - {REmpty       N/A{0\n\r",value); 
        send_to_char(buf,ch);
        continue;
     }
     strfl = fread_string_eol(fp);

     sprintf(buf, "Backup#%d - %s{0\n\r",value,strfl);
     send_to_char(buf,ch);
     fclose(fp);
    }

       return;
   }



    value = atoi( arg );
    if ((value >= 1 && value <= 3) && (str_cmp(arg2,"erase")))
    {
	   for (d = descriptor_list; d ; d = d_next)
	   {
	    d_next = d->next;
	    write_to_descriptor( d, "Backing up Distorted Illusions - Standby, this may take a moment..\n\n\r",0);
	   }

	buf[0]='\0';
        sprintf(buf, "Archiving to slot #%d of 3...\n\r", value);
        write_to_descriptor( ch->desc, buf ,0 );
	buf[0]='\0';
        sprintf(buf,"rm -fr %s/backup%d*",BACKUP_DIR,value);
        system(buf);
	buf[0]='\0';
	sprintf(buf,"tar -cf %s/backup%d.tar %s"
	  ,BACKUP_DIR,value,ROT_DIR);
        system(buf);
	buf[0]='\0';
        sprintf(buf,"Gzipping as backup%d.tar.gz ...\n\r",
          value);
        write_to_descriptor( ch->desc, buf ,0);
	buf[0]='\0';
	sprintf(buf,"gzip -f %s/backup%d.tar &"
	  ,BACKUP_DIR,value);
        system(buf);

	buf[0]='\0';
        sprintf(buf,"Creating backup%d.txt file...\n\r", value);
        write_to_descriptor( ch->desc, buf ,0);
	buf[0]='\0';
        sprintf(buf,"        By: %-10s Time: %s\n\r",
            ch->name,
            (char *) ctime( &current_time ) );
        write_to_descriptor( ch->desc, buf , 0);

	strsave[0]='\0';
        sprintf(strsave, "%s/backup%d.txt",BACKUP_DIR,value);

        if ((fp = fopen(strsave,"w")) == NULL)
        {
            bug("Backup: fopen",0);
            perror(strsave);
        }

        fprintf(fp,"{B%-10s {G%s\n",
            ch->name,
            (char *) ctime( &current_time ));
        fclose( fp );
	buf[0]='\0';
        sprintf(buf,"{BB{backup {BC{bomplete!{0\n\r");
        do_echo(ch, buf );
        return;
    }
    else if ((value >= 1 && value <= 3) && (!str_cmp(arg2,"erase")))
    {
        sprintf(buf,"Erased slot #%d of 3...\n\r",value);
        send_to_char(buf,ch);
        sprintf(buf,"rm -fr %s/backup%d*",BACKUP_DIR,value);
        system(buf);
        return;
    }
    else
    {
        send_to_char("Invalid argument.. (1-3)\n\r",ch);
        return;
    }
} 

void print_log_watch(char *argument)
{
  /*
      for (d = descriptor_list; d != NULL; d = d->next)
      {
        if(IS_SET(d->character->plyr,PLAYER_LOG_WATCH))
        {
         sprintf(buf,"[LOG WATCH]:\n\r");
         strcat(buf,argument);
        }
      }
   */
   return;
}

void 
do_watch (CHAR_DATA * ch, char *argument)
{
        if (IS_SET (ch->plyr, PLAYER_LOG_WATCH))
	{
                send_to_char ("Log Watch mode removed.\n\r", ch);
                REMOVE_BIT (ch->plyr, PLAYER_LOG_WATCH);
	}
	else
	{
                send_to_char ("Log Watch mode on.\n\r", ch);
                SET_BIT (ch->plyr, PLAYER_LOG_WATCH);
	}
}

void do_iquest( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH/4];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );

    if ( ( arg1[0] == '\0' ) || ( !IS_IMMORTAL(ch) ) )
    {
	if (!global_quest)
	{
	    send_to_char("There is no quest in progress.\n\r",ch);
	    return;
	}

        if (global_quest_closed)
        {
         send_to_char("The current quest is closed..sorry.\n\r",ch);
         return;
        }

	if (ch->on_quest)
	{
	    send_to_char("You'll have to wait till the quest is over.\n\r",ch);
	    return;
	}
	ch->on_quest = TRUE;
	send_to_char("Your quest flag is now on.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1, "mob"))
    {
	argument = one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char("Quest which mobile?\n\r",ch);
	    return;
	}
	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if (!IS_NPC(victim))
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}
	if (victim->on_quest)
	{
	    send_to_char( "Quest flag removed.\n\r", ch );
	    victim->on_quest = FALSE;
	} else
	{
	    send_to_char( "Quest flag set.\n\r", ch );
	    victim->on_quest = TRUE;
	}
	return;
    }
    if (!str_cmp(arg1, "on"))
    {
	if (global_quest)
	{
	    send_to_char("The global quest flag is already on.\n\r",ch);
	    return;
	}
	global_quest = TRUE;
	send_to_char("The global quest flag is now on.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1, "off"))
    {
	if (!global_quest)
	{
	    send_to_char("The global quest flag is not on.\n\r",ch);
	    return;
	}
	global_quest = FALSE;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING )
	    {
		d->character->on_quest = FALSE;
	    }
	}
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if (!IS_NPC(victim))
		continue;
	    if (!victim->on_quest)
		continue;
	    victim->on_quest = FALSE;
	}
	send_to_char("The global quest flag is now off.\n\r",ch);
	return;
    }
    if (!str_cmp(arg1, "closed"))
    {
        if (global_quest_closed)
	{
            send_to_char("The quest is already closed.\n\r",ch);
	    return;
	}

        global_quest_closed = TRUE;
        send_to_char("The global quest is now closed.\n\r",ch);
	return;
    }

    if (!str_cmp(arg1, "open"))
    {
        if (!global_quest_closed)
	{
            send_to_char("The quest is already open.\n\r",ch);
	    return;
	}

        global_quest_closed = FALSE;
        send_to_char("The global quest is now open.\n\r",ch);
	return;
    }

    if (!str_cmp(arg1, "restore"))
    {
        if (!global_quest)
	{
            send_to_char("There is no quest running.\n\r",ch);
	    return;
	}

	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
            if (IS_NPC(victim))
		continue;
	    if (!victim->on_quest)
		continue;
            do_restore(ch,victim->name);
	}

        send_to_char("All quest participants restored.\n\r",ch);
	return;
    }

    if (!str_cmp(arg1, "begin"))
    {
        if (!global_quest)
	{
            send_to_char("There is no quest to begin!\n\r",ch);
	    return;
	}

        for ( victim = char_list; victim != NULL; victim = victim->next )
	{
            if (IS_NPC(victim))
		continue;
	    if (!victim->on_quest)
		continue;
            if (IS_IMMORTAL(victim))
            continue;
            sprintf(buf,"%s %s",victim->name,ch->name);
            do_transfer(ch,buf);
	}

        send_to_char("All quest participants placed in the quest room.\n\r",ch);
	return;
    }

    if (!str_cmp(arg1, "allpeace"))
    {
        if (!global_quest_closed)
	{
            send_to_char("The quest is already open.\n\r",ch);
	    return;
	}

        for ( victim = char_list; victim != NULL; victim = victim->next )
	{
            if (IS_NPC(victim))
		continue;
	    if (!victim->on_quest)
		continue;
            do_peace(victim,"");
	}

        send_to_char("All quest participants are at peace.\n\r",ch);
	return;
    }

    do_iquest(ch, "");
    return;
}

void do_ident( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );

        if (argument[0] == '\0')
        {
            sprintf(buf,"Your identity is known as: %s\n\r",ch->pcdata->identity);
            send_to_char(buf,ch);
            return;
        }

        if ( IS_SET( ch->comm, COMM_NOEMOTE ) )
        {
            send_to_char( "You have been restricted from using this command.\n\r",ch);
            return;
        }

        strcat( argument, "{x" );
        free_string( ch->pcdata->identity );
        ch->pcdata->identity = str_dup( argument );

        sprintf(buf,"Your identity is now known as: %s\n\r",ch->pcdata->identity);
        send_to_char(buf,ch);
    }
    return;
}

/* Grant/Revoke system for immortal command usage */
/*void do_grant(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int i,j, level;
    bool toomany = TRUE;
    bool found = FALSE;
    char arg1[MIL];
    char arg2[MIL];
    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if(arg1[0] == '\0' 
    || arg2[0] == '\0'
    || !(victim = get_char_world(ch,arg1))
    || IS_NPC(victim))
    {
	send_to_char("Syntax: grant <character> <command>\n\r",ch);
	return;
    }

    for(i = 0; i < MAX_GRANT; i++)
    {
	if(victim->pcdata->grant[i] == NULL)
	{
	    toomany = FALSE;
	    break;
	}
    }
    
    if(toomany)
    {
	send_to_char("That character has too many commands granted.\n\r",ch);
	return;
    }

    // Check to see if it is by level
    if(is_number(arg2))
    {
    	level = atoi( arg2 );
    	
        if( level < 102 || level > 110 )
    	{
		send_to_char( "Level must be 102 to 110.\n\r", ch );
		return;
    	}
    	else // we have a valid number
    	{
    		for(j = 0; cmd_table[j].name[0] != '\0'; j++)
    		{
    			if(cmd_table[j].level > 101 && cmd_table[j].level <= level)
    			{
    				grant_command(victim,cmd_table[j].name);
    			}
    		}
    	}
    	return;
    }	

    for(j = 0; cmd_table[j].name[0] != '\0'; j++)
    {
	if(!str_prefix(arg2, cmd_table[j].name))
	{
	    found = TRUE;
	    break;
	}
    }
    if(!found)
    {
	send_to_char("Command not found.\n\r",ch);
	return;
    }
    if(check_grant(victim,cmd_table[j].name) != -1)
    {
	send_to_char("That command has already been granted to them.\n\r",ch);
	return;
    }
    victim->pcdata->grant[i] = str_dup(cmd_table[j].name);
    printf_to_char(victim,"You have been granted %s.\n\r",victim->pcdata->grant[i]);
    printf_to_char(ch,"You have granted %s %s.\n\r",victim->name, victim->pcdata->grant[i]);
    return;
}

void do_revoke(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int i;
    char arg1[MIL];
    char arg2[MIL];
    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if(arg1[0] == '\0' 
    || arg2[0] == '\0'
    || !(victim = get_char_world(ch,arg1))
    || IS_NPC(victim))
    {
	send_to_char("Syntax: revoke <victim> <command>\n\r",ch);
	return;
    }
    if((i = check_grant(victim, arg2)) == -1)
    {
	send_to_char("That character has not been granted that command.\n\r",ch);
	return;
    }
    printf_to_char(victim,"The %s command has been revoked.\n\r", victim->pcdata->grant[i]);
    printf_to_char(ch,"The %s command has been revoked from %s.\n\r",victim->pcdata->grant[i],victim->name);
    free_string(victim->pcdata->grant[i]);
    victim->pcdata->grant[i] = NULL;
    return;
}

void grant_command(CHAR_DATA *ch, char *argument)
{
    char arg1[MIL];
    int i, j;
    bool toomany = TRUE;
    argument = one_argument(argument,arg1);
    
    for(i = 0; i < MAX_GRANT; i++)
    {
	if(ch->pcdata->grant[i] == NULL)
	{
	    toomany = FALSE;
	    break;
	}
    }
    
    if(toomany)
     return;
    
    if(check_grant(ch,arg1) != -1)
    	return;
    
    ch->pcdata->grant[i] = str_dup(arg1);
    printf_to_char(ch,"You have been granted %s.\n\r",ch->pcdata->grant[i]);
    
    return;
}

void do_viewgrant( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA* victim;
    int i;
    bool match = FALSE;
        if ( IS_NPC(ch))
    {
	send_to_char( "Sorry, no NPC's.\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "These are your specifically granted commands:\n\r", ch );
	for ( i = 0; i < MAX_GRANT; i++ )
	{
	    if (( ch->pcdata->grant[i] != NULL ) && ( ch->pcdata->grant[i][0] != '\0' ))
	    {
		match = TRUE;
		printf_to_char( ch, "%s\n\r", ch->pcdata->grant[i] );
	    }
	}
	if ( !match )
	    send_to_char( "None.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( IS_NPC(victim))
    {
	send_to_char( "Sorry, no NPC's.\n\r", ch );
	return;
    }

    if ( ch->trust < victim->trust )
    {
	send_to_char( "Your powers aren't strong enough to find their granted commands.\n\r", ch );
	return;
    }

    printf_to_char( ch, "These are %s's specifically granted commands:\n\r", victim->name );
    for ( i = 0; i < MAX_GRANT; i++ )
    {
	if (( victim->pcdata->grant[i] != NULL ) && ( victim->pcdata->grant[i][0] != '\0' ))
	{
	    match = TRUE;
	    printf_to_char( ch, "%s\n\r", victim->pcdata->grant[i] );
        }
    }
    if ( !match )
	send_to_char( "None.\n\r", ch );
    return;
}*/

bool is_granted( CHAR_DATA *ch, DO_FUN *do_fun)
{
	GRANT_DATA *gran;
 
	if (ch->desc == NULL) 
		return FALSE;
 
	if (ch->desc->original != NULL) 
		ch = ch->desc->original;

	for (gran=ch->pcdata->granted; gran != NULL; gran=gran->next)
  		if (do_fun == gran->do_fun)
    		return TRUE;

	return FALSE;
}

bool is_granted_name( CHAR_DATA *ch, char *name)
{
	GRANT_DATA *gran; 

	if (ch->desc == NULL)
		return FALSE;

	if (ch->desc->original != NULL) 
		ch = ch->desc->original;

	for (gran=ch->pcdata->granted; gran != NULL; gran=gran->next)
  		if (is_exact_name(gran->name,name))
    		return TRUE;   

	return FALSE;
} 
  
int grant_duration(CHAR_DATA *ch, DO_FUN *do_fun)
{
	GRANT_DATA *gran;
  
	if (ch->desc->original != NULL) 
		ch=ch->desc->original;

/*  Replace the x's in the line below with the name of
    a character that is allowed to grant commands to
    anyone, even if they don't have the command
    themselves.  This is useful when you add new
    imm commands, and need to give them to yourself.
    Additional names can be added as needed and
    should be seperated by spaces.  */

	if (is_exact_name(ch->name,"Skyntil"))
  	return -1;
    
	for (gran=ch->pcdata->granted; gran != NULL; gran=gran->next)
  		if (gran->do_fun == do_fun)
    			return gran->duration;

	return 0;
}

void grant_add(CHAR_DATA *ch, char *name, DO_FUN *do_fun, int duration, int level)
{
	GRANT_DATA *gran;

	if (ch->desc->original != NULL) ch=ch->desc->original;
  
	gran = alloc_mem(sizeof(*gran));
	gran->name = str_dup(name);
	gran->do_fun = do_fun;
	gran->duration = duration;
	gran->level = level;

	gran->next = ch->pcdata->granted;
	ch->pcdata->granted = gran;

	return;
}

void grant_remove(CHAR_DATA *ch, DO_FUN *do_fun, bool mshow)
{
	GRANT_DATA *p,*gran;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *rch;
    
	rch = ch->desc->original ? ch->desc->original : ch;

	p=NULL;
	gran=rch->pcdata->granted;
	if (gran->do_fun == do_fun)
  		rch->pcdata->granted=gran->next;
	else
	{
  		for (gran=rch->pcdata->granted; gran != NULL; gran=gran->next)
  		{
    			if (gran->do_fun == do_fun) break;
    			p=gran;
  		}
        }
               
	if (p != NULL) p->next=gran->next;
		sprintf(buf,"You have lost access to the %s command.\n\r",gran->name);
	
	if (mshow) 
		send_to_char(buf,ch);
	
	free_string(gran->name);
	free_mem(gran,sizeof(*gran));
	return;
}

void grant_level( CHAR_DATA *ch, CHAR_DATA *victim, int level, int duration )
{
	int cmd;

	for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
  		if ( cmd_table[cmd].level == level
  		&& !is_granted(victim,cmd_table[cmd].do_fun)
  		&& grant_duration(ch,cmd_table[cmd].do_fun) == -1)
    		grant_add(victim,cmd_table[cmd].name,cmd_table[cmd].do_fun, duration, cmd_table[cmd].level);
	
	return;
}

void revoke_level( CHAR_DATA *ch, CHAR_DATA *victim, int level )
{
	int cmd;

	for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
  		if ( cmd_table[cmd].level == level
  		&& is_granted(victim,cmd_table[cmd].do_fun)
  		&& grant_duration(ch,cmd_table[cmd].do_fun) == -1)
    			grant_remove(victim,cmd_table[cmd].do_fun,FALSE);
	
	return;
}   
    
void do_grant( CHAR_DATA *ch, char *argument )
{  
char buf[MAX_STRING_LENGTH];
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
CHAR_DATA *victim=NULL,*rch,*rvictim=NULL;
int  dur,cmd,x;
bool found=FALSE;
DESCRIPTOR_DATA *d;

argument = one_argument(argument,arg1);
argument = one_argument(argument,arg2);
one_argument(argument,arg3);

rch = ch->desc->original?ch->desc->original:ch;
  
if (arg1[0] == '\0')
  { 
    send_to_char("Grant who, what?\n\r",ch);
    return;
  }
 
for (d = descriptor_list; d != NULL; d = d->next)
{
  rvictim = d->original?d->original:d->character;
  
  if (rvictim == NULL) continue;

  if (!str_cmp(rvictim->name,arg1)) 
  {
     victim = d->character;
     break;
  }
}

if (victim == NULL && !str_cmp("self",arg1))  
{

   rvictim = rch;
   victim = ch;
}

if (victim==NULL)
  {
    send_to_char("Victim not found.\n\r",ch);
    return;
  }

if (arg2[0] == '\0')
  { 
    int col=0;
    int lvl;

    sprintf(buf,"%s has not been granted the following commands:\n\r",
            rvictim->name);
    send_to_char(buf,ch);
    
    for ( lvl = IM; lvl <= ( L1 + 1 ) ; lvl++ )
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      if (cmd_table[cmd].level >= LEVEL_IMMORTAL 
      && !is_granted(victim,cmd_table[cmd].do_fun)
      && cmd_table[cmd].level == lvl)
      {
        sprintf( buf,"[L%3d] %-12s", cmd_table[cmd].level, cmd_table[cmd].name );
        send_to_char(buf,ch);
        if ( ++col % 4 == 0 )
          send_to_char( "\n\r", ch );
      }
    if ( col % 4 != 0 )
      send_to_char( "\n\r", ch);
    return;
  }

dur = arg3[0]=='\0' ? -1 : is_number(arg3) ? atoi(arg3) : 0;
   
if (dur<1 && dur != -1)
  { 
    send_to_char("Invalid duration!\n\r",ch);
    return;
  } 
    
  if (is_number(arg2))
  { 
    if (atoi(arg2) < LEVEL_IMMORTAL || atoi(arg2) > MAX_LEVEL)
    {
      send_to_char("Invalid grant level.\n\r",ch);
      return; 
    }
    grant_level(ch, victim, atoi(arg2), dur );
    sprintf(buf, "You have been granted level %d commands.\n\r", atoi(arg2));
    send_to_char("Ok.\n\r",ch);
    send_to_char(buf,victim);
    return;
  } 
    
for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
  if ( arg2[0] == cmd_table[cmd].name[0]
  &&   is_exact_name( arg2, cmd_table[cmd].name ) )
  {  
     found = TRUE;
      break;
  }
          
if (found)
  { 
    if (cmd_table[cmd].level < LEVEL_IMMORTAL)
    {
      send_to_char("You can only grant immortal commands.\n\r",ch);
      return;
    }

    if (grant_duration(ch,cmd_table[cmd].do_fun) != -1)
    { 
      send_to_char("You can't grant that!\n\r",ch);
      return;
    }

    if (is_granted(victim,cmd_table[cmd].do_fun))
    { 
      send_to_char("They already have that command!\n\r",ch);
      return;
    }

    grant_add(victim,cmd_table[cmd].name,cmd_table[cmd].do_fun,
              dur, cmd_table[cmd].level);

    sprintf(buf,"%s has been granted the %s command.\n\r",rvictim->name,
            cmd_table[cmd].name);
    send_to_char(buf,ch);
    sprintf(buf,"%s has granted you the %s command.\n\r",rch->name,
            cmd_table[cmd].name);
    send_to_char(buf,victim);

    for (x=0; pair_table[x].first[0] != '\0'; x++)
      if (!str_cmp(arg2,pair_table[x].first)
      && !is_granted_name(victim,pair_table[x].second))
      { 
        sprintf(buf,"%s %s %s",rvictim->name, pair_table[x].second, arg3);
        do_grant(ch,buf);
      }
      else if (!str_cmp(arg2,pair_table[x].second)
           && pair_table[x].one_way != TRUE
           && !is_granted_name(victim,pair_table[x].first))
      { 
        sprintf(buf,"%s %s %s",rvictim->name,pair_table[x].first, arg3);
        do_grant(ch,buf);
 }
    
    return;
  }
  send_to_char("Command not found!\n\r",ch);
  return;
}   
    
void do_revoke( CHAR_DATA *ch, char *argument )
{
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
CHAR_DATA *victim=NULL,*rvictim=NULL;
DESCRIPTOR_DATA *d;
int cmd,x;
bool had_return, found=FALSE;
    
argument = one_argument(argument,arg1);
one_argument(argument,arg2);
    
if (arg1[0] == '\0' )
  { 
    send_to_char("Revoke who, what?\n\r",ch);
    return;
  }   
      
for (d = descriptor_list; d != NULL; d = d->next)
{ 
  rvictim = d->original?d->original:d->character;
       
  if (rvictim == NULL) continue;
           
  if (!str_cmp(rvictim->name,arg1))
  {  
     victim = d->character;
     break;
 }
}   
    
if (victim == NULL && !str_cmp("self",arg1))
{  
  rvictim = ch->desc->original ? ch->desc->original : ch;
  victim = ch;
}

if (victim==NULL)
  {
    send_to_char("Victim not found.\n\r",ch);
    return;
  }

had_return = is_granted_name(victim,"return");

if (arg2[0] == '\0')
  { 
    int col=0,lvl;
    char buf[MAX_STRING_LENGTH];
     
    sprintf(buf,"%s has been granted the following commands:\n\r",
            rvictim->name);
    send_to_char(buf,ch);

    for ( lvl = IM; lvl <= ( L1 + 1 ) ; lvl++ )  
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      if (cmd_table[cmd].level >= LEVEL_IMMORTAL 
      && is_granted(victim,cmd_table[cmd].do_fun)
      && cmd_table[cmd].level == lvl )
      {
        sprintf( buf,"[L%3d] %-12s", cmd_table[cmd].level, cmd_table[cmd].name );
        send_to_char(buf,ch);
        if ( ++col % 4 == 0 )
          send_to_char( "\n\r", ch );
   }
    if ( col % 4 != 0 )
      send_to_char( "\n\r", ch);
    return;
  }
  
if (is_number(arg2))
  { 
    char buf[MAX_STRING_LENGTH];
  
    if (atoi(arg2)<LEVEL_IMMORTAL || atoi(arg2)>MAX_LEVEL)
    {
      send_to_char("Invalid revoke level.\n\r",ch);
      return;
    }
    revoke_level(ch, victim, atoi(arg2));
    sprintf(buf, "You have lost acess to level %d commands.\n\r", atoi(arg2));
    send_to_char("Ok.\n\r",ch);
    send_to_char(buf,victim);
    
    if (had_return && !is_granted_name(victim,"return") &&
        rvictim != victim) do_return(victim,"");
    
    return;
  }

for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
  if ( arg2[0] == cmd_table[cmd].name[0]
  &&   is_exact_name( arg2, cmd_table[cmd].name ) )
  {   
      found = TRUE;
      break;
  }
        
if (found)
  {
 char buf[MAX_STRING_LENGTH];
    
    if (grant_duration(ch,cmd_table[cmd].do_fun) != -1)
    {
      send_to_char("You can't revoke that!\n\r",ch);
      return;
    }
    
    if (!is_granted(victim,cmd_table[cmd].do_fun))
    {
      send_to_char("They don't have that command!\n\r",ch);
      return;
    } 
      
    grant_remove(victim,cmd_table[cmd].do_fun,TRUE);
    
    sprintf(buf,"%s has lost access to the %s command.\n\r",
             rvictim->name,cmd_table[cmd].name);
    send_to_char(buf,ch);

    for (x=0; pair_table[x].first[0] != '\0'; x++)
      if (!str_cmp(arg2,pair_table[x].first)
      && is_granted_name(victim,pair_table[x].second))
      {
        sprintf(buf,"%s %s",rvictim->name,pair_table[x].second);
        do_revoke(ch,buf);
      }
      else if (!str_cmp(arg2,pair_table[x].second)
           && pair_table[x].one_way != TRUE
           && is_granted_name(victim,pair_table[x].first))
      {
        sprintf(buf,"%s %s",rvictim->name,pair_table[x].first);
        do_revoke(ch,buf);
      }
  
    if (had_return && !is_granted_name(victim,"return") && rvictim != victim) do_return(victim,"");

    return;
  }   
  send_to_char("Command not found!\n\r",ch);
  return;
}   

    
void do_gstat( CHAR_DATA *ch, char *argument )
{
char arg[MAX_INPUT_LENGTH];
char buf[MAX_STRING_LENGTH];
BUFFER *buffer;
GRANT_DATA *grant;
CHAR_DATA *victim;
int col=0;   
             
one_argument(argument,arg);

if (arg[0] == '\0')
  { 
    send_to_char("Gstat who?\n\r",ch);
    return;
  }
        
if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  { 
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
        
if (IS_NPC(victim))
  { 
    send_to_char("Not on mobs.\n\r",ch);
    return;
  }

if (get_trust(ch)<get_trust(victim))
  {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }

buffer=new_buf();

sprintf(buf,"Grant status for %s:\n\r\n\r",victim->name );

add_buf(buffer,buf);

for (grant=victim->pcdata->granted; grant!=NULL; grant=grant->next)
  {
    char ds[50],ss[25],s2[25];
    int x,sl;
    
    sprintf(ds,"%d",grant->duration);
    ss[0]='\0';
    sl = (int)((6-strlen(ds)) / 2);
    
    for (x=0; x<sl ;x++)
      strcat(ss," ");

    strcpy(s2,ss);
    
    if ((strlen(ss)+strlen(ds)) % 2 == 1) strcat(s2," ");
    
    if (grant->duration==-1)
      sprintf(buf,"[ perm ] %-11s",grant->name);
    else
      sprintf(buf,"[%s%d%s] %-11s",ss, grant->duration, s2, grant->name);
    
    add_buf(buffer,buf);
  col++;
    col %= 4;
    
    if (col==0) add_buf(buffer,"\n\r");
  }
   
if (col != 0) add_buf(buffer,"\n\r");

page_to_char(buf_string(buffer),ch);

free_buf(buffer);
return;
}

void do_smite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int value;

    if ( IS_NPC(ch) )
    {
	send_to_char( "Mobs can't smite.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
        send_to_char("Syntax: smite <char> <type> <divisor>\n\r", ch);
        send_to_char("<Type> can be hp, mana, move, or all.\n\r",ch);
        send_to_char("<Divisor> can be any number from 2 - 100.\n\r",ch);
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Trying to smite a mob?\n\r", ch );
	return;
    }

    if (ch == victim)
    {
        send_to_char( "Trying to smite yourself?\n\r", ch );
        return;
    }

    value = atoi(arg3);
    if(!is_number(arg3))
    value = 2;

    act( "A bolt from the heavens smites $N!", ch, NULL, victim, TO_NOTVICT );
    act( "A bolt from the heavens smites you!", ch, NULL, victim, TO_VICT );
    act( "You smite $N!", ch, NULL, victim, TO_CHAR );

    if(!str_cmp(arg2,"hp"))
    {
     victim->hit /= value;
    }
    else if(!str_cmp(arg2,"mana"))
    {
     victim->mana /= value;
    }
    else if(!str_cmp(arg2,"move"))
    {
     victim->move /= value;
    }
    else if(!str_cmp(arg2,"all"))
    {
        victim->hit /= value;
        victim->mana /= value;
        victim->move /= value;
    }

    return;
}

/* Funprogs! implement this soon.
void do_funprog( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int value, value1;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    extern bool Funprog_running;
    extern sh_int Xp_mult;
    extern sh_int Dam_mult;
    extern sh_int Skill_mult;
    extern sh_int Funprog_time;
    char type[MAX_INPUT_LENGTH];

    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
        
    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r",ch);
        send_to_char( "  set funprog <type> <value> <time>\n\r", ch);
        send_to_char( "  Time is calculated in Minutes.\n\r",ch);
        return;
    }
   
    if( is_number( type ) )
    {
     send_to_char( "Type must be xp,dam, or skill, or auto.\n\r", ch );
     return;
    }

    if(!str_cmp(type, "auto"))
    {
     send_to_char("Auto-Funprog toggled.\n\r",ch);
     if(Auto_funprog)
     Auto_funprog = FALSE;
     else
     Auto_funprog = TRUE;
     return;
    }

    if ( !is_number( arg1 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }
    else if ( !is_number( arg2 ) )
    {
        send_to_char( "Time Value must be numeric.\n\r", ch );
        return;
    }
        
    value = atoi( arg1 );
    value1 = atoi( arg2 );

    if ( value1 < 0 || value1 > 120)
    {
        send_to_char( "Time range is 0 to 120(2 hours ).\n\r", ch );
        return;
    }
    
    if(!str_cmp(type,"xp"))
    {
     if ( value < 1 || value > 10 )
     {
        send_to_char( "Value range is 1 to 10.\n\r", ch );
        return;
     }

     if(ch->level < 58 && value > 5)
     {
      send_to_char("You cannot raise the multiple past 5.\n\r",ch);
      return;
     }

        if(value > 1 && value1 > 0)
        {
        Funprog_running = TRUE;
        Xp_mult = value;
        Funprog_time = value1;
        }
        else if (value == 1 || value1 == 0)
        {
         Funprog_running = FALSE;
         Xp_mult = value;
         Funprog_time = value1;
        }

    }
    if(!str_cmp(type,"dam"))
    {
     if ( value < 1 || value > 10 )
     {
        send_to_char( "Value range is 1 to 10.\n\r", ch );
        return;
     }
        if(value > 1 && value1 > 0)
        {
        Funprog_running = TRUE;
        Dam_mult = value;
        Funprog_time = value1;
        }
        else if (value == 1 || value1 == 0)
        {
         Funprog_running = FALSE;
         Dam_mult = value;
         Funprog_time = value1;
        }
    }
    if(!str_cmp(type,"skill"))
    {
     if ( value < -10 || value > 10 )
     {
        send_to_char( "Value range is -10 to 10.\n\r", ch );
        return;
     }
        if(value != 0 && value1 > 0)
        {
        Funprog_running = TRUE;
        Skill_mult = value;
        Funprog_time = value1;
        }
        else if (value == 0 || value1 == 0)
        {
         Funprog_running = FALSE;
         Skill_mult = value;
         Funprog_time = value1;
        }
    }


if(Funprog_running)
{
    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->connected == CON_PLAYING )
      {
        vch = d->original ? d->original : d->character;
        do_announce_funprog(vch,type);
      }

    }
}    
else if(!Funprog_running)
{
    for ( d = descriptor_list; d; d = d->next )
    {
      if ( d->connected == CON_PLAYING )
      {
        vch = d->original ? d->original : d->character;
        send_to_char("FUNPROG prematurely ended!\n\r",vch);
      }

    }

}
        
    return;
}*/

/*
void do_avator( CHAR_DATA *ch, char *argument )
{
	 char buf[MAX_STRING_LENGTH];	
	 char arg1[MAX_INPUT_LENGTH];	
	 int level;				
	 int iLevel;				

	 argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' || !is_number( arg1 ) )
	 {
		  send_to_char( "Syntax: avator <level>.\n\r", ch );
		  return;
	 }

	 if ( IS_NPC(ch) )
	 {
		  send_to_char( "Not on NPC's.\n\r", ch );
		  return;
	 }

		  if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
		  {
		 sprintf(buf, "Level must be 1 to %d.\n\r", MAX_LEVEL );
		  send_to_char( buf, ch );
		return;
	 }

	 if ( level > get_trust( ch ) )
	 {
		  send_to_char( "Limited to your trust level.\n\r", ch );
		  sprintf(buf, "Your Trust is %d.\n\r",ch->trust);
		  send_to_char(buf,ch);
		  return;
	 }
// Your trust stays so you will have all immortal command

		  if(ch->trust == 0) {
		  ch->trust = ch->level;
		  }

	if ( level <= ch->level )
	 {
		 int temp_prac;

		  send_to_char( "Lowering a player's level!\n\r", ch );
		  send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r",ch );
		  temp_prac = ch->practice;
		  ch->level    = 1;
                  ch->exp      = exp_per_level(ch);
		  ch->max_hit  = 20;
		  ch->max_mana = 100;
		  ch->max_move = 100;
		  ch->practice = 0;
		  ch->hit      = ch->max_hit;
		  ch->mana     = ch->max_mana;
		  ch->move     = ch->max_move;
		  advance_level( ch, TRUE );
		  ch->practice = temp_prac;
	 }
	 else
	 {
		  send_to_char( "Raising a player's level!\n\r", ch );
		  send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", ch);
	 }

	 for ( iLevel = ch->level ; iLevel < level; iLevel++ )
	 {
		  ch->level += 1;
		  advance_level( ch,TRUE);
	 }
	 sprintf(buf,"You are now level %d.\n\r",ch->level);
	 send_to_char(buf,ch);
			ch->exp   = exp_per_level(ch)
						* UMAX( 1, ch->level );
    if (ch->pcdata->condition[COND_DEHYDRATED] > 40)
          ch->pcdata->condition[COND_DEHYDRATED] = 0;

	 save_char_obj(ch);
	 return;
}*/
void do_award(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_STRING_LENGTH];
  char arg2 [MAX_STRING_LENGTH];
  char arg3 [MAX_STRING_LENGTH];
  int value;

  DESCRIPTOR_DATA *d;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg3 ) )
  {
    send_to_char( "{GS{gynta{Gx{y: {Yaward {y<{Ychar{y> <{Ytype{y> <{Yvalue{y>.{x\n\r", ch);
    send_to_char( "{GT{gype{Gs a{gr{Ge{y: {Wplatinum{w, {Wexperience{w,{x\n\r",ch);
    send_to_char( "           {Wpractice{w,{W train{w, {Wqps{w, {Wiqps{x\n\r",ch);
    send_to_char( "{wFor more information, type {W'{DHELP AWARD{W'{w.{x\n\r",ch);
    return;
  }

  if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
  {
    send_to_char( "{RCannot find {rtarget {Rplayer{R.{x\n\r", ch);
    return;
  }
  if ( !str_cmp( arg1, "all" ) && (ch->level >= IMMORTAL))
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   d->character->level < IMMORTAL 
            &&   ch->level >= d->character->ghost_level
            &&   can_see( ch, d->character ) )
            {                                                      
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s %s", d->character->name, arg2, arg3 );
                do_award( ch, buf );
            }
}
       return;
    }
  if ( IS_NPC( victim ) )
  {
    send_to_char( "{RNot on {rNPC's{R!{x\n\r", ch);
    return;
  }

//Do not limit my immortal powerrsssss!!!!
 
/*  if ( ch == victim )
  {
    send_to_char( "{RSorry, no awarding {ryourself{R!{x", ch );
    return;
  }

  if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
  {
    send_to_char( "{RImmortals cannot be {rawarded{R!{x\n\r", ch );
    return;
  }*/
  
  if (is_name(arg2, "platinum"))
  {
    value = atoi( arg3 );

    if (value < -10000 || value > 10000 )
    {
      send_to_char( "{wAcceptable values are between {D-10,000 {wand {W10,000{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThat is fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    victim->platinum += value;
    sprintf( buf,"{cYou have awarded {B%s{C %d {cplatinum.{x\n\r", victim->name, value );
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded {c%d {Cplatinum!!{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{cYou have been penalized {C%d {cplatinum!!{x.\n\r", value );
      send_to_char( buf, victim );
      return;
      }
    }

  else if (is_name(arg2, "experience") )
  {
    value = atoi( arg3 );

    if (value < -100000 || value > 100000 )
    {
      send_to_char( "{wAcceptable values are between {D-100,000 {wand {W100,000{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThat is fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    gain_exp(victim, value);
    sprintf( buf,"{cYou have awarded {B%s {C%d{c experience points.{x\n\r", victim->name, value);
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded {c%d {Cexperience points.{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{cYou have been penalized {C%d {cexperience points.{x\n\r", value );
      send_to_char( buf, victim );
      return;
      }
    }

  else if (is_name(arg2, "qps" ) )
  {
    value = atoi( arg3 );

    if (value < -10000 || value > 10000 )
    {
      send_to_char( "{wAcceptable values are between {D-10,000 {wand {W10,000{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThat is fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    victim->questpoints += value;
    sprintf( buf,"{cYou have awarded {B%s {C%d {cquest points.{x\n\r", victim->name, value);
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded{c %d {Cquest points.{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{cYou have been penalized {C%d{c quest points!{x\n\r", value );
      send_to_char( buf, victim );
      return;
      }
    }

  else if (is_name(arg2, "iqps" ) )
  {
    value = atoi( arg3 );

    if (value < -100 || value > 100 )
    {
      send_to_char( "{wAcceptable values are between {D-100 {wand {W100{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThat is fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    victim->qps += value;
    sprintf( buf,"{cYou have awarded {B%s {C%d {cImm quest points.{x\n\r", victim->name, value);
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded{c %d {CImm quest points.{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{cYou have been penalized {C%d{c Imm quest points!{x\n\r", value );
      send_to_char( buf, victim );
      return;
      }
    }

  else if (is_name(arg2, "prac practice" ) )
  {
    value = atoi( arg3 );

    if (value < -1000 || value > 1000 )
    {
      send_to_char( "{wAcceptable values are between {D-1,000 {wand {W1,000{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThat is fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    victim->practice += value;
    sprintf( buf,"{cYou have awarded {B%s {C%d {cpractices.{x\n\r", victim->name, value);
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded {c%d {Cpractices.{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{RYou have been penalized %d prac(s).{x\n\r", value );
      send_to_char( buf, victim );
      return;
    }
  }

  else if (is_name(arg2, "train" ) )
  {
    value = atoi( arg3 );

    if (value < -1000 || value > 1000 )
    {
      send_to_char( "{wAcceptable values are between {D-1,000 {wand {W1,000{w.{x\n\r", ch );
      return;
    }

    if (value == 0 )
    {
      send_to_char( "{WThats fairly trivial, isn't it?{x\n\r", ch );
      return;
    }

    victim->train += value;
    sprintf( buf,"{cYou have awarded{B %s {C%d {ctrains.{x\n\r", victim->name, value);
    send_to_char(buf, ch);

    if ( value > 0 )
    {
      sprintf( buf,"{CYou have been awarded {c%d {Ctrains.{x\n\r", value );
      send_to_char( buf, victim );
      }
      else
      {
      sprintf( buf,"{cYou have been penalized{C %d {ctrains.{x\n\r", value );
      send_to_char( buf, victim );
      return;
    }
  }
  return;
}
void do_gcast(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *vch;
  char arg[MAX_INPUT_LENGTH];
  
   DESCRIPTOR_DATA *d;

  argument = one_argument(argument,arg);
  
  
  if (IS_NPC(ch))
    return;                                  
 if (arg[0] == '\0')
  { send_to_char("Spellup whom?\n\r",ch);
  return; }
 

    if ( !str_cmp( arg, "world" ) && (ch->level >= IMMORTAL))
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   ch->level >= d->character->ghost_level
            &&   can_see( ch, d->character ) )
            {                                                      
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg );
                do_gcast( ch, buf );
            }
}
       return;
    }
  if (str_cmp("room",arg))
  { if ((vch = get_char_world(ch,arg)) == NULL)
    { send_to_char("There is noone here to cast on.\n\r",ch);
    return; }}
  else
    vch = ch; 

if (!str_cmp("room",arg))
  for (vch = ch->in_room->people;vch;vch = vch->next_in_room)
  { if (vch == ch)
    continue;
    if (IS_NPC(vch))
      return;
    spell_shockshield(skill_lookup("shockshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_fireshield(skill_lookup("fireshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_iceshield(skill_lookup("iceshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_shield(skill_lookup("shield"),ch->level,ch,vch,TARGET_CHAR);
    spell_armor(skill_lookup("armor"),ch->level,ch,vch,TARGET_CHAR);
    spell_fly(skill_lookup("fly"),ch->level,ch,vch,TARGET_CHAR);
    spell_frenzy(skill_lookup("frenzy"),ch->level,ch,vch,TARGET_CHAR);
    spell_giant_strength(skill_lookup("giant strength"),ch->level,ch,vch,TARGET_CHAR);
    spell_bless(skill_lookup("bless"),ch->level,ch,vch,TARGET_CHAR);
    spell_haste(skill_lookup("haste"),ch->level,ch,vch,TARGET_CHAR);
  }
  else
  { spell_shockshield(skill_lookup("shockshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_fireshield(skill_lookup("fireshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_iceshield(skill_lookup("iceshield"),ch->level,ch,vch,TARGET_CHAR);
    spell_shield(skill_lookup("shield"),ch->level,ch,vch,TARGET_CHAR);
    spell_armor(skill_lookup("armor"),ch->level,ch,vch,TARGET_CHAR);
    spell_fly(skill_lookup("fly"),ch->level,ch,vch,TARGET_CHAR);
    spell_frenzy(skill_lookup("frenzy"),ch->level,ch,vch,TARGET_CHAR);
    spell_giant_strength(skill_lookup("giant strength"),ch->level,ch,vch,TARGET_CHAR);
    spell_bless(skill_lookup("bless"),ch->level,ch,vch,TARGET_CHAR);
    spell_haste(skill_lookup("haste"),ch->level,ch,vch,TARGET_CHAR);
  }
  return;
}

void do_dblexp( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
     
    int value;
    DESCRIPTOR_DATA *d;

    
    argument = one_argument( argument, arg1 );
//    vch = d->original ? d->original : d->character;
	vch = d-> character;

	if ( arg1[0] == '\0' )
    {
       send_to_char("{WSyntax{D: {cdblexp on{D, {cdblexp off{D, {cdblexp level{D, {cdblexp shutdown{D.{x\n\r",ch);
       send_to_char(".\n\r",ch);
       send_to_char("{wLevel{D: {cWill enable dblexp for those under lvl 101{D.{x\n\r",ch);
       return;
    }

    if ( !str_cmp(arg1, "on") )
    {
	if (dbl_exp)
	{
	    send_to_char("Double EXP is already on.\n\r",ch);
	    return;
	}
 
	if (timed_dbl_exp)
	{
            timed_dbl_exp = FALSE;
            dbl_exp = TRUE;
	    send_to_char("Double EXP changed from Timer to On.\n\r",ch);
	    return;
	}
        
        if (dbl_exp_lvl)
        {
           dbl_exp_lvl = FALSE;
           dbl_exp = TRUE;
           send_to_char("Double EXP changed from under 101, to All.\n\r",ch);
           return;
        }

	dbl_exp = TRUE;
	send_to_char("Double EXP is now on.\n\r",ch);

	for ( d = descriptor_list; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING && !IS_SET(d->character->comm, COMM_QUIET))
            {
               send_to_char( "{3[{#INFO{3]:{# Double EXP has been Enabled!!!!!{x\n\r",d->character);
            }
        }

	return;
    }
    
    if ( !str_cmp(arg1, "level") )
    {
       int l = 0;

       if (dbl_exp)
       {
          l++;
          dbl_exp = FALSE;
          dbl_exp_lvl = TRUE;
          send_to_char("Double EXP changed from all to only under 101.\n\r",ch);
       }

       if (timed_dbl_exp)
       {
          l++;
          timed_dbl_exp = FALSE;
          dbl_exp_lvl = TRUE;
          send_to_char("Double EXP changed from on a Timer, to under 101.\n\r",ch);
       }
       
       if (dbl_exp_lvl)
       {
          
          send_to_char("Double EXP is already on for those under level 101.\n\r",ch);
          return;
       } 

       if ( l == 0 )
       {
           dbl_exp_lvl = TRUE;
           send_to_char("Double EXP for all those under level 101 is now on.\n\r",ch);
       }
         
       for ( d = descriptor_list; d; d = d->next )
       {
           if ( d->connected == CON_PLAYING && !IS_SET(d->character->comm, COMM_QUIET) )
           {
              send_to_char( "{3[{#INFO{3]:{# Double EXP has been Enabled for all below 101!!!!!!!{x\n\r",d->character);
           }
       }
   
       return;
    }

    if ( !str_cmp(arg1, "off") )
    {
	if (timed_dbl_exp)
	{
        
	    send_to_char("Double EXP is on a Timer it will shutdown automaticly.\n\r",ch);
            send_to_char("If wish to still turn off, use dblexp shutdown.\n\r",ch);
	    return;
	}

	dbl_exp = FALSE;
        dbl_exp_lvl = FALSE;
	send_to_char("Double EXP is now off.\n\r",ch);

	for ( d = descriptor_list; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING && !IS_SET(d->character->comm, COMM_QUIET))
            {
               send_to_char( "{3[{#INFO{3]:{# Awww Double EXP has been Disabled.\n\r{x",d->character);
            }
        }

	return;
    }

    if (!str_cmp(arg1, "shutdown"))
    {
       dbl_exp = FALSE;
       dbl_exp_lvl = FALSE;
       timed_dbl_exp = FALSE;
       send_to_char("Double EXP has been SHUTDOWN.\n\r",ch);
       return;
    }

    if (is_number(arg1))
    {
       value = atoi(arg1);
       sprintf(buf,"{3[{#INFO{3]:{# Double EXP has been Enabled for %i Ticks {x\n\r",value);
   
       for (d = descriptor_list; d != NULL; d = d->next) 
           send_to_desc(buf,d);
      
       if (value > 0)
       {
         timed_dbl_exp = TRUE;
       }
       else if (value == 0)
       {
               timed_dbl_exp = FALSE;
       }     
    }

    do_dblexp(ch, "");
    return;
}

int days_since_last_file_mod( char *filename )
{
    int days;
    struct stat buf;
    extern time_t current_time;

    if (!stat(filename, &buf))
    {
	days = (current_time - buf.st_mtime)/86400;
    }
    else
	days = 0;

    return days;
}

/* Modifications to Plist by Bree -- Complete re-write by Xorax */
void do_plist( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    DIR *dp = NULL;
    struct dirent *ep;
    char buf[80];
    int days;

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
    {
	sprintf( buf, "%s*", PLAYER_DIR );
	dp = opendir( buf );
    }

    else if ( !str_prefix( argument, "immortal" ) )
	dp = opendir( "../gods" );

    else
    {
	if ( !isalpha( LOWER( *argument ) ) )
	{
	    send_to_char( "Syntax: PLIST [IMMORTAL/LETTER exmp.[Plist a]\n\r", ch );
	    return;
	}

	sprintf( buf, "%s%c", PLAYER_DIR, LOWER( *argument ) );
	dp = opendir( buf );
    }

    if ( dp == NULL )
    {
	perror( "Couldn't open the directory" );
	return;
    }

    final = new_buf( );

    while( ( ep = readdir( dp ) ) )
    {
	if ( ep->d_name[0] == '.' )
	    continue;

	/*
	 * change the string literals to fit your needs like above
	 */
//	sprintf( buf, "%s%s", fImmortal ? "../gods/" : "../player/", ep->d_name );

	days = days_since_last_file_mod( buf );

	/*
	 * color coding using Lopes comaptable color to highlight
	 * inactivity. green = active, red = innactive
	 * Just remove the colors if they cause problems.
	 */
	sprintf( buf, "%-15s %s%-3d{x days\n\r",
	    ep->d_name, days > 30 ? "{r" : days > 20 ? "{Y" : days > 10 ? "{g" : "{G", days );
	add_buf( final, buf );
    }

    closedir( dp );
    page_to_char( final->string, ch );
    free_buf( final );
}

void do_nuke( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: nuke <name> \n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on mobiles!\n\r", ch );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You failed!\n\r", ch );
	return;
    }

    stop_fighting( victim, TRUE );

    sprintf( strsave, "%s%s/%s", PLAYER_DIR, initial( victim->name ),
	capitalize( victim->name ) );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    sprintf( buf, "{C%s {csends a {yl{Yightnin{yg {Yb{yol{Yt{c flying through the heavens.\n\r"
			  "It streaks down towards Earth, striking {C%s {cin the {rh{Rear{rt{c.\n\r"
			  "A {rf{Rir{reb{Ral{rl{c flares up, and a pile of {wash{c is all that remains.{x\n\r",
		PERS( ch, d->character ), PERS( victim, d->character ) );
	    send_to_char( buf, d->character );
	}
    }

    if ( is_clead( victim ) )
	update_clanlist( victim, victim->clead, FALSE, TRUE );
    if ( is_clan( victim ) )
	update_clanlist( victim, victim->clead, FALSE, FALSE );

    check_new_clan( victim->name, 999, victim->rank );

    if ( victim->level > HERO )
	update_wizlist( victim, 1 );

//    sprintf( buf, "vapes %s.", victim->name );
//    parse_logs( ch, "immortal", buf );

    force_quit( victim, "" );

    unlink( strsave );

    return;
}

void do_doas( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *orig;
    char arg1[MAX_INPUT_LENGTH];
    bool is_afk;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Doas <victim> <command>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "delete" ) || !str_cmp( argument, "reroll" )
    ||   !str_cmp( argument, "quit" ) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	act( "$t not found.", ch, arg1, NULL, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Do it yourself!\n\r", ch );
	return;
    }

    if ( !can_over_ride( ch, victim, FALSE ) )
    {
	send_to_char( "You can not impersonate your superior!\n\r", ch );
	return;
    }

//    if ( !check_builder( ch, victim->in_room, TRUE ) )
//	return;

    orig = victim->desc;
	
    victim->desc = ch->desc;
    ch->desc = NULL;

    if ( IS_SET( victim->comm, COMM_AFK ) )
	is_afk = TRUE;
    else
	is_afk = FALSE;

    interpret( victim, argument );

    ch->desc = victim->desc;	
    victim->desc = orig;

    if ( is_afk )
	SET_BIT( victim->comm, COMM_AFK );
}

void append_todo( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
    char *strtime;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-9] = '\0';
    strncpy( buf, strtime+4, 6 );

    buf[6] = '\0';

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    } else {
	if( !str_cmp( file, BUG_FILE ) || !str_cmp( file, TYPO_FILE ) )
	{
	    int vnum;

	    vnum = ch->in_room ? ch->in_room->vnum : 0;
	    fprintf( fp, "[{w%-12s {B- {w%5d{B] {y%s: {w%s\n",
		ch->name, vnum, buf, str );
	}

	else
	    fprintf( fp, "[{w%-12s{B] {y%s: {w%s\n", ch->name, buf, str );

	fclose( fp );
    }
}

void parse_todo( CHAR_DATA *ch, char *argument, char *command, char *file )
{
    FILE *fp;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	printf_to_char( ch, "Syntax:  %s add <item>\n\r", command );
	printf_to_char( ch, "         %s show\n\r", command );
	printf_to_char( ch, "         %s delete <line no>\n\r", command );
	printf_to_char( ch, "         %s clear\n\r", command );
	return;
    }

    argument = one_argument( argument, arg );

    if ( !str_prefix( arg, "show" ) || !str_prefix( arg, "list" ) )
    {
	BUFFER *output;
	char *line;
	char c = '[';
	int i = 1;

	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	output = new_buf( );

	while( !feof( fp ) )
	{
	    line = fread_string_eol( fp );
	    sprintf( buf, "{y%3d{B) %s\n\r", i++, line );
	    add_buf( output, buf );
	    free_string(line);

	    while( !feof( fp ) && ( c = fread_letter( fp ) ) != '[' )
		;

	    if ( !feof( fp ) )
		ungetc( c, fp );
	}

	add_buf( output, "{x" );

	page_to_char( output->string, ch );
	free_buf( output );
	fclose( fp );
	return;
    }

    else if ( !str_prefix( arg, "add" ) )
    {
	append_todo( ch, file, argument );
	printf_to_char( ch, "Line added to the %s list.\n\r", command );
	return;
    }

    else if ( !str_cmp( arg, "delete" ) )
    {
	FILE *fout;
	char *line, c = '[';
	bool empty = TRUE, found = FALSE;
	int i = 1, num;

	one_argument( argument, arg );

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    printf_to_char( ch, "Syntax: %s delete <line number>\n\r", command );
	    return;
	}

	num = atoi( arg );

	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	if ( ( fout = fopen( "../data/temp.dat", "w" ) ) == NULL )
	{
	    fclose( fp );
	    send_to_char( "Error opening temporary file.\n\r", ch );
	    return;
	}

	while( !feof( fp ) )
	{
	    line = fread_string_eol( fp );

	    if ( i++ != num )
	    {
		empty = FALSE;
		fprintf( fout, "%s\n\r", line );
	    } else
		found = TRUE;

	    free_string( line );

	    while( !feof( fp ) && ( c = fread_letter( fp ) ) != '[' )
		;

	    if ( !feof( fp ) )
		ungetc(c,fp);
	}

	fclose( fout );
	fclose( fp );

	if ( empty )
	{
	    printf_to_char( ch, "The %s list has been cleared.\n\r", command );
	    unlink( file );
	    return;
	}

	if ( !found )
	    send_to_char( "That line number does not exist.\n\r", ch );
	else
	{
	    rename( "../data/temp.dat", file );
	    printf_to_char( ch, "Line deleted from %s list.\n\r", command );
	}

	return;
    }

    else if ( !str_cmp( arg, "clear" ) )
    {
	if ( ( fp = fopen( file, "r" ) ) == NULL )
	{
	    printf_to_char( ch, "The %s list is empty.\n\r", command );
	    return;
	}

	fclose( fp );
	printf_to_char( ch, "The %s list has been cleared.\n\r", command );
	unlink( file );
	return;
    }

    parse_todo( ch, "", command, file );
    return;
}

void do_changed( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
	parse_todo( ch, argument, "changed", CHANGED_FILE );
    else
	parse_todo( ch, "show", "changed", CHANGED_FILE );

    return;
}

void do_todo( CHAR_DATA *ch, char *argument )
{
    parse_todo( ch, argument, "todo", TO_DO_FILE );
    return;
}

void do_tocode( CHAR_DATA *ch, char *argument )
{
    parse_todo( ch, argument, "tocode", TO_CODE_FILE );
    return;
}

void do_bug( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "bug", BUG_FILE );
	return;
    }

    append_todo( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "typo", TYPO_FILE );
	return;
    }

    append_todo( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

/* void do_buildvance (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MIL];
    char arg2[MIL];
    char arg3[MIL];
    int  stat;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy(arg3, argument);

    stat = is_number(arg3) ? atoi(arg3) : -1;

    if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
	send_to_char ("Syntax: buildvance <char name> <promote|demote|security> <value>", ch);
	return;
    }

    if (!str_cmp(arg2, "promote"))
    {
    if (!str_cmp(ch->name, "Eragon"))
    {
        if (IS_NPC(ch))
        {
		send_to_char ("Mobs dont build, well.\n\r", ch);
		return;
    	}

        if ((victim = get_char_world(ch, arg1)) == NULL)
    	{
		send_to_char( "That player is not here.\n\r", ch);
		return;
 		}

        if (IS_SET(victim->act, PLR_IS_BUILDER))
        {
          if (victim->level >= stat)
	      {
		    send_to_char ("Use the demote option to do that.\n\r", ch);
            return;
          }
          if (stat  >= victim->level)
	      {
		    send_to_char ("You have promoted them.\n\r", ch);
	 	    send_to_char ("You have been promoted.\n\r", victim);
		    victim->level = stat;
            return;
	      }

        }

    	if (!IS_SET(victim->act, PLR_IS_BUILDER))
    	{
	      if (stat < victim->level)
          {
	    	send_to_char ("You cant demote them.\n\r", ch);
	    	return;
	      }
	      if (202 >= stat || stat > 207)
	      {
	        send_to_char ("They gotta be immortal to build.\n\r", ch);
	        return;
	      }
          send_to_char ("You are now a Builder of Equanox.\n\r", victim);
      	  send_to_char ("They are now a Builder of Equanox.\n\r", ch);
	      victim->level = stat;
          SET_BIT (victim->act, PLR_IS_BUILDER);
	      return;
 	    }
 	}
 	send_to_char ("Only the Head Builder can advance builders", ch);
 	return;
    }

    if (!str_cmp(arg2, "demote"))
    {
    if (!str_cmp(ch->name, "Eragon"))  // || IS_IMP(ch))
    {
        if (IS_NPC(ch))
        {
		send_to_char ("Mobs dont build, well.\n\r", ch);
		return;
    	}

        if ((victim = get_char_world(ch, arg1)) == NULL)
    	{
		send_to_char( "That player is not here.\n\r", ch);
		return;
 		}

        if (IS_SET(victim->act, PLR_IS_BUILDER))
        {
            if (victim->level >= stat)
	        {
		    send_to_char ("You have demoted them.\n\r", ch);
		    send_to_char ("You have been demoted.\n\r", victim);
		    victim->level = stat;
		    if (202 >= stat)
		    {
        	REMOVE_BIT (victim->act, PLR_IS_BUILDER);
		    return;
		    }
        	return;
	    }

        if (stat  >= victim->level)
	    {
		send_to_char ("Use the promote option for this.\n\r", ch);
        return;
	    }

        }

    	if (!IS_SET(victim->act, PLR_IS_BUILDER))
    	{
        send_to_char ("They must be a builder to fire them.\n\r", victim);
	    return;
 	    }
 	}
 	send_to_char ("Only the Head Builder can demote builders", ch);
 	return;
 	}

 	if (!str_cmp(arg2, "security"))
 	{
     if (!str_cmp(ch->name, "Eragon")) // || IS_IMP(ch))
     {
	    if (IS_NPC(ch))
        {
		  send_to_char ("Mobs dont need security.\n\r", ch);
		  return;
    	}
   	    if ((victim = get_char_world(ch, arg1)) == NULL)
    	{
	      send_to_char( "That player is not here.\n\r", ch);
		  return;
   	    }

	    if (IS_SET(victim->act, PLR_IS_BUILDER) && (9 >= stat >= 0))
    	{
		  send_to_char ("You changed there status.\n\r", ch);
		  send_to_char ("Your trust has changed.\n\r", victim);
		  victim->pcdata->security = stat;
          return;
	    }

    	if (!IS_SET(victim->act, PLR_IS_BUILDER))
    	{
	    send_to_char ("They have to be a buider first.\n\r", ch);
	    return;
	    }
	 }
	send_to_char ("Only the Head Builder can edit security", ch);
	return;
 	}
	send_to_char ("Syntax: buildvance <char name> <promote|demote|security> <value>", ch);
	return;
}
*/


void do_nohelp( CHAR_DATA *ch, char *argument )
{
    if ( IS_IMMORTAL( ch ) )
    {
	parse_todo( ch, argument, "nohelp", NO_HELP_FILE );
	return;
    }

    append_todo( ch, NO_HELP_FILE, argument );
    send_to_char( "Missing help file logged.\n\r", ch );
    return;
}


