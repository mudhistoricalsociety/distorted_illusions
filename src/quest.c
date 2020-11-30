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
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  * 
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_wizslap );

BUFFER * display_stats  args( ( OBJ_DATA *obj, CHAR_DATA *ch, bool contents ) );

/* Object vnums for Quest Rewards


Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 9825
#define QUEST_OBJQUEST2 9826
#define QUEST_OBJQUEST3 9827
#define QUEST_OBJQUEST4 9828
#define QUEST_OBJQUEST5 9829
/* Local functions */

void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool chance		args(( int num ));
bool god;

struct quest_type
{
    char	*keyword;
    int		cost;
    bool	iq;
    int		vnum;
};

const struct quest_type quest_table [] =
{
//  { "KEYWORD",					COST,	IQUEST?,	VNUM	},

    { "pill",						1,		TRUE,		50		},
    { "blue diamond",               650,    FALSE,		2660,   }, 
    { "flag token",					1500,	FALSE,		9927	},
    { "freya sight",                2650,   FALSE, 		9853 	},
    { "healing pod portable",		2800,	FALSE,		9851	},
    { "philosopher stone",          3000,   FALSE,  	9926,   },
    { "pack aesir",					3400,	FALSE,		9905	},
    { "guidance fenrir",			3400,	FALSE,		9852	},
    //Aesir
    { "sword",						45,		TRUE,		9873	},
    { "dagger",						45,		TRUE,		9874	},
    { "mace",						45,		TRUE,		9875	},
    { "flail",						45,		TRUE,		9876	},
    { "ranseur",					45,		TRUE,		9877	},
    { "staff",						45,		TRUE,		9878	},
    { "axe",						45,		TRUE,		9879	},
    { "whip",						45,		TRUE,		9880	},
    { "platemail",					45,		TRUE,		9881	},
    { "helmet",						45,		TRUE,		9882	},
    { "legplates",					45,		TRUE,		9891	},
    { "boots",						45,		TRUE,		9883	},
    { "gauntlets",					45,		TRUE,		9884	},
    { "armplates",					45,		TRUE,		9885	},
    { "shield",						45,		TRUE,		9886	},
    { "cape cloak",					45,		TRUE,		9887	},
    { "bracer",						45,		TRUE,		9888	},
    { "girth",						45,		TRUE,		9889	},
    { "mask",						45,		TRUE,		9890	},
    //Vinir	
    { "vdagger",					10,		TRUE,		9921	},
    { "vmace",						10,		TRUE,		9918	},
    { "vflail",						10,		TRUE,		9922	},
    { "vpolearm",					10,		TRUE,		9920	},
    { "vspear",						10,		TRUE,		9924	},
    { "vaxe",						10,		TRUE,		9919	},
    { "vsword",						10,		TRUE,		9917	},
    { "vwhip",						10,		TRUE,		9923	},
    { "vplatemail",					10,		TRUE,		9912	},
    { "vhelmet",					10,		TRUE,		9913	},
    { "vlegplates",					10,		TRUE,		9907	},
    { "vboots",						10,		TRUE,		9911	},
    { "vgauntlets",					10,		TRUE,		9909	},
    { "varmplates",					10,		TRUE,		9908	},
    { "vshield",					10,		TRUE,		9910	},
    { "vcape vcloak",				10,		TRUE,		9914	},
    { "vbracer",					10,		TRUE,		9906	},
    { "vgirth",						10,		TRUE,		9916	},
    { "vexotic",					10,		TRUE,		9925	},
    { "vmask",						10,		TRUE,		9915	},
    //Leave this blank line!
    { "",				0,	FALSE,		0	}
};


int find_table_pos( char *argument )
{
    int pos;

    for ( pos = 0; quest_table[pos].keyword[0] != '\0'; pos++ )
    {
	if ( is_name( argument, quest_table[pos].keyword ) )
	    return pos;
    }

    return -1;
}

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
  char arg4 [MAX_INPUT_LENGTH];
    int i;
 //   char aesir [MAX_INPUT_LENGTH] = "aesir";
    

    
    smash_tilde( argument );
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    strcpy(arg3, argument);
    
    int expreward;
    expreward = number_range(300,3000);    

    if (!strcmp(arg1, "info"))
    {
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
	    {
		sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
		send_to_char(buf, ch);
	    }
	    else if (ch->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL)
		{
		    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r",questinfoobj->name);
		    send_to_char(buf, ch);
		}
		else send_to_char("You aren't currently on a quest.\n\r",ch);
		return;
	    }
	    else if (ch->questmob > 0)
	    {
                questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL)
		{
	            sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
		    send_to_char(buf, ch);
		}
		else send_to_char("You aren't currently on a quest.\n\r",ch);
		return;
	    }
	}
	else
	    send_to_char("You aren't currently on a quest.\n\r",ch);
	return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d Aquest points.\n\r",ch->questpoints);
	send_to_char(buf, ch);
	return;
    }
    else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char("You aren't currently on a quest.\n\r",ch);
	    if (ch->nextquest > 1)
	    {
		sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
		send_to_char(buf, ch);
	    }
	    else if (ch->nextquest == 1)
	    {
		sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(buf, ch);
	    }
	}
        else if (ch->countdown > 0)
        {
	    sprintf(buf, "Time left for current quest: %d\n\r",ch->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an 
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
	if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

	if (!strcmp(arg1, "string"))
        {
            if (ch->questpoints >= 300)
            {
	       if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
    	   {
	       send_to_char( "You dont have that item.\n\r", ch );
	       return;
    	    }
		    if (arg3[0] == '\0')
	            {
			send_to_char("You have to provide string to restring something.",ch);
	                return;
	            }
    	     obj = get_obj_carry( ch, arg2 );
    	     free_string( obj->short_descr );
	         obj->short_descr = str_dup( arg3 );
	         ch->questpoints -= 300;
	         send_to_char("Thank you, your restring was successful!\n\r",ch);
	    	return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }

	else if ( !strcmp( arg1, "list" ) )
	{
	    OBJ_INDEX_DATA *pObj;

	    send_to_char( "    {D***************({RQuest Services{D)***************{x\n\r", ch );
	    send_to_char( "    {D* {cImmunities{C({W1500{C){D:{B  {r(buy hunger/thirst)     {D*\n\r", ch );
	    send_to_char( "    {D* {cPretitle{C({W7500{C){D:    {r(buy pretitle {Y'{rtitle{Y'{r){D  *\n\r", ch );
		send_to_char( "    {D* {cWhotitle{C({W10000{C){D:   {r(buy whotitle {Y'{rtitle{Y'{r){D  *\n\r", ch );
	    send_to_char( "    {D* {cGod Change{C({W650{C){D:   {r(buy god {Y'{rnew god{Y'{r){D     *\n\r", ch );
	    send_to_char( "    {D*    {ruse {Y'{Whelp {Bnewgod{Y' {rfor this list.{D        *\n\r", ch );
	    send_to_char( "    {D* {cRestring{C({W300{C){D:     {r(string {W<{robj{W><{rstring{W>{r){D  *\n\r", ch );
	    send_to_char( "    {D* {cQuest Points{C({Wbuy {caquest{W/{ciquest{C){D:{r1iq {w={r 300aq{D*\n\r", ch );	     
	    send_to_char( "    {D* {cCorpse Retrieval{C({c1 Iq{W/{C300 Aq{W/600 Plat{C){D:    *\n\r", ch );
	    send_to_char( "    {D*    {r(buy retrieval {ciquest/aquest/plat{r ){D     *\n\r", ch );
	    send_to_char( "    {D*                                            *\n\r", ch );
	    send_to_char( "    {D*****************({RQuest Items{D)****************\n\n\r", ch );
	    send_to_char( "   {W[{wCost{W] {Y[{yKeywords{Y]               {C[{cObject{C]{x\n\r", ch );

	    for ( i = 0; quest_table[i].keyword[0] != '\0'; i++ )
	    {
		if ( ( pObj = get_obj_index( quest_table[i].vnum ) ) != NULL )
		{
		    sprintf( buf, "%6d %s %-25.25s %s\n\r",
			quest_table[i].cost,
			quest_table[i].iq ? "IQ" : "AQ",
			quest_table[i].keyword,
			pObj->short_descr );
		    send_to_char( buf, ch );
		}
	    }

	    return;
	}

	else if ( !strcmp( arg1, "browse" ) )
	{
	    BUFFER *stats;

	    if ( arg2[0] == '\0' )
	    {
		send_to_char( "Quest Browse <item>\n\r", ch );
		return;
	    }

	    if ( ( i = find_table_pos( arg2 ) ) == -1 )
	    {
		send_to_char( "That is not a quest item.\n\r", ch );
		return;
	    }

	 if ( ( obj = create_object( get_obj_index( quest_table[i].vnum ), 0 ) ) == NULL )
	    {
		send_to_char( "Error, can not create object!\n\r", ch );
		return;
	    }

	    stats = display_stats( obj, ch, FALSE );
	    page_to_char( stats->string, ch );
	    free_buf( stats );
	    extract_obj( obj );
	    return;
	}
	
	/*else if ( !strcmp( arg1, "sell") )
	{
		if ( arg2[0] == '\0' )
	    {
		send_to_char( "Quest sell <item>\n\r", ch );
		send_to_char( "Vinir items sell for 8 iqp, Aesir for 40 iqp\n\r", ch );
		return;
	    }
	    
		if ((obj = get_obj_carry (ch, arg2)) == NULL)
		{
			send_to_char( "You do not have that item\n\r", ch );
			return;
		}
		
	    
	    i = find_table_pos( arg2 );

	    if ( !quest_table[i].iq)
	    {
		    send_to_char( "You can only sell Aesir or Vinir items\n\r", ch );
		    return;
	    }

	    obj=get_obj_carry(ch,arg2);
	    if ( quest_table[i].iq )
	    {
		    if ((quest_table[i].cost) == 45)
		    {
				ch->qps += 40;
				extract_obj (obj);
			}
			else if ((quest_table[i].cost) == 10)
			{
				if (obj->name == aesir)
				{
					send_to_char("You used the wrong keywords.  Please use the keyword from quest list for your item.\n\r",ch);
					return;
				}
				else
				{
				ch->qps += 8;
				extract_obj (obj);
				}
			}
			else
			{
				ch->qps +=1;
				extract_obj (obj);
			}
			sprintf(buf, "Thank you %s!",ch->name);
        	do_say(questman,buf);
	    return;
    	}
    return;
	}
*/
	else if ( !strcmp( arg1, "buy" ) )
	{
	    if ( arg2[0] == '\0' )
	    {
		send_to_char( "Quest buy <item>\n\r", ch );
		return;
	    }

	    if ( ( i = find_table_pos( arg2 ) ) == -1 )
	    {
	    if ( !strcmp(arg2, "retrieval") )
        {
            OBJ_DATA *obj;
            int plat = 0, found = 0;
            
            for ( obj = object_list; obj != NULL; obj = obj->next )
            {
                if ( obj->item_type == ITEM_CORPSE_PC && !str_cmp(obj->owner, ch->name) )
                   found = 1;
            }

            if ( found == 0 )
            {
               sprintf(buf,"I'm sorry %s,but there is no corpse of yours out there.",ch->name);
               do_say(questman,buf);
               sprintf(buf,"Are you sure you died?");
               do_say(questman,buf);
               return;
            }

            if ( arg3[0] == '\0' || !is_name( arg3, "platinum iquest aquest" ) )
            {           
               send_to_char("{cYou need to specifiy how you want to pay for this.{x\n\r",ch);
               send_to_char("{ySyntax{D: {cquest buy retrieval {mplatinum{w/{miquest{w/{maquest{x\n\r",ch);
               return;
            }
           
            plat = ch->platinum + ( ch->gold / 100 ) + ( ch->silver / 10000);            

            if ( ( is_name(arg3, "platinum" ) && plat < 600 ) ||  ( is_name(arg3, "iquest" ) && ch->qps < 1 )
               || ( is_name(arg3, "aquest") && ch->questpoints < 300 ) )
            {
               send_to_char("{cYou do not have the required amount. Either,{x\n\r",ch);
               send_to_char("{cspecify a different payment or collect more.{x\n\r",ch);
               return;
            }
            else if ( is_name(arg3, "platinum" ) )
               ch->platinum -= 600;
            else if ( is_name(arg3, "iquest" ) )
               ch->qps -= 1;
            else if ( is_name(arg3, "aquest" ) )
               ch->questpoints -= 300;

            for ( obj = object_list; obj != NULL; obj = obj->next )
            {
                if ( obj->item_type == ITEM_CORPSE_PC && !str_cmp( obj->owner,ch->name) && (obj->in_room != NULL))
                {
                   obj_from_room(obj);
                   obj_to_char(obj, ch);
                }
            }

            sprintf(buf,"Your belongings appear in your arms!",arg3);
            do_say(questman,buf);
            return;
        }

        else if (!strcmp(arg2, "god"))
        {
            if (ch->questpoints >=650)
            {
              int god;

              god = god_lookup(arg3);

              if( god == 0 )
              {
              send_to_char("That is not a valid god for you.\n\r",ch);
              return;
              }
              ch->questpoints -= 650;
              ch->god = god;
              send_to_char("Thank you, your god has been changed.\n\r",ch);
              return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points to change your god.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (!strcmp(arg2, "pretitle"))
        {
            if (ch->questpoints >=7500)
            {
	        int value;
	
	        if (IS_SET (ch->act, PLR_NOTITLE))
	                return;
	
	        if (ch->pcdata->pretit == '\0')
	                ch->pcdata->pretit = "{x";

	        if (arg3[0] == '\0')
	        {
	                send_to_char ("You must pick a pretitle, choose wisely..", ch);
	                return;
	        }


	        if (strlen (arg3) > 45)
	        {
	                arg3[45] = '{';
	                arg3[46] = 'x';
	                arg3[47] = '\0';
	        }
	        else
	        {
	                value = strlen (arg3);
	                arg3[value] = '{';
	                arg3[value + 1] = 'x';
	                arg3[value + 2] = '\0';
	        }

	        ch->pcdata->pretit = str_dup (arg3);
	        send_to_char ("Pretitle Set.\n\r", ch);
                ch->questpoints -= 7500;
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points to do that.", ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (!strcmp(arg2, "whotitle"))
        {
            if (ch->questpoints >=10000)
            {
		    char buf2[MAX_STRING_LENGTH];
		    int plc = 0;
		    int cnt = 0;
	            buf[0] = '\0';
	            buf2[0] = '\0';
	            ch->pcdata->who_descr = str_dup( "" );
	            if (arg3[0] == '\0')
	            {
			send_to_char("You must choose a Whotitle, please choose Wisely.",ch);
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
	            if (IS_IMMORTAL(ch))
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
	          if (IS_IMMORTAL(ch))
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
	            ch->pcdata->who_descr = str_dup(buf);
	            buf[0] = '\0';
	            buf2[0] = '\0';
		send_to_char("Whotitle Set.",ch);
              	ch->questpoints -= 10000;
              	return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points to do that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (!strcmp(arg2, "thirst"))
        {
            if (ch->questpoints > 1500)
            {
              if( ch->pcdata->condition[COND_THIRST] == -1 )
              {
              send_to_char("You are already immune to thirst.\n\r",ch);
              return;
              }
              else {
              ch->questpoints -= 1500;
              ch->pcdata->condition[COND_THIRST] = -1;
              send_to_char("Your thirst has been permanently quenched.\n\r",ch);
              return;
            	}
        	}
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points to make yourself immune to that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (!strcmp(arg2, "hunger"))
        {
            if (ch->questpoints > 1500)
            {
              if(  ch->pcdata->condition[COND_HUNGER] == -1 )
              {
              send_to_char("You are already immune to hunger.\n\r",ch);
              return;
              }
              else {
              ch->questpoints -= 1500;
              ch->pcdata->condition[COND_HUNGER] = -1;
              send_to_char("You feel perpetually full.\n\r",ch);
              return;
          		}
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points to make yourself immune to that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (!strcmp(arg2, "iquest"))
		{
	    if(ch->questpoints < 300)
	    {
		sprintf(buf,"I'm sorry %s, you don't have enough aquest points for that",ch->name);
		do_say(questman,buf);
		return;
	    }
	    ch->qps = ch->qps + 1;
	    ch->questpoints = ch->questpoints - 300;
	    sprintf(buf,"Thank you!");
	    do_say(questman,buf);
	    return;
		}
		else if (!strcmp(arg2, "aquest"))
		{
	    if(ch->qps < 1)
	    {
		sprintf(buf,"I'm sorry %s, you don't have enough quest points for that",ch->name);
		do_say(questman,buf);
		return;
	    }
	    ch->qps = ch->qps - 1;
	    ch->questpoints = ch->questpoints + 300;
	    sprintf(buf,"Thank you!");
	    do_say(questman,buf);
	    return;
		}
        else if (!strcmp(arg2, "blue"))
        {
            if (ch->questpoints >= 650)
            {
                OBJ_DATA *blue;

                ch->questpoints -= 650;

                blue = create_object(get_obj_index(2660),ch->level);
                obj_to_char(blue,ch);

                act( "$N gives $p to $n.", ch, blue, questman, TO_ROOM );
                act( "$N gives you $p.",   ch, blue, questman, TO_CHAR );
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
    	else{
	    send_to_char( "That is not a quest item.\n\r", ch );
		return;
		}
	}
	

	    if ( quest_table[i].iq && ch->qps < quest_table[i].cost )
	    {
		sprintf( buf, "Sorry, %s, but you don't have enough iquest points for that.", ch->name );
		do_say( questman, buf );
		return;
	    }

	    if ( !quest_table[i].iq && ch->questpoints < quest_table[i].cost )
	    {
		sprintf( buf, "Sorry, %s, but you don't have enough aquest points for that.", ch->name );
		do_say( questman, buf );
		return;
	    }

	    if ( ( obj = create_object( get_obj_index( quest_table[i].vnum ), 0 )) == NULL )
	    {
		send_to_char( "Error, can not create object!\n\r", ch );
		return;
	    }

	    if ( quest_table[i].iq )
		ch->qps -= quest_table[i].cost;
	    else
		ch->questpoints -= quest_table[i].cost;

	    obj_to_char( obj, ch );

            act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
            act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );

	    return;
	}     

    else if (!strcmp(arg1, "quit"))
    {
	act("$n informs $N that he is giving up on his quest.",ch,NULL,questman,TO_ROOM);
	act("You inform $N that you are giving up on your quest.",ch,NULL,questman,TO_CHAR);

	if (IS_NPC(ch))
	{
	   return;
	}
	if(!IS_SET(ch->act, PLR_QUESTOR))
	{
	   sprintf(buf,"%s you idiot, you aren't on a quest!", ch->name);
	   do_say(questman,buf);
	   return;
	}
	REMOVE_BIT(ch->act, PLR_QUESTOR);
	ch->questgiver = NULL;
	ch->countdown = 0;
	ch->questmob = 0;
	ch->questobj = 0;
	ch->questroom = 0;
	ch->nextquest = 15;
	sprintf(buf,"That's too bad %s, now you have to wait.",ch->name);
	do_say(questman,buf);
	return;
   }
	
	
    else if (!strcmp(arg1, "request"))
    {
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM); 
	act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);
      if (IS_NPC(ch))
      {
        act( "$N bonks $n on the head for being a total moron.", ch, NULL, questman, TO_ROOM);
        act( "$N bonks you on the head for being such a total moron.", ch, NULL, questman, TO_CHAR);
        sprintf(buf, "Quests are for kids, silly NPC!");
        do_say(questman, buf);
      }
      if (ch->level < 15)
      {
	    sprintf(buf, "Brave %s, you are too inexperienced to quest now.",ch->name);
          do_say(questman, buf);
          sprintf(buf, "Come back when you've learned more.");
          do_say(questman, buf);
          return;
      }
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    sprintf(buf, "But you're already on a quest!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0)
	{
	    sprintf(buf, "You're very brave, %s, but let someone else have a chance.",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Come back later.");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "Thank you, brave %s!",ch->name);
	do_say(questman, buf);

	generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
	{
            ch->countdown = number_range(10,30);
	    SET_BIT(ch->act, PLR_QUESTOR);
	    sprintf(buf, "You have %d minutes to complete this quest.",ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "May the gods go with you!");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM); 
	act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
	if (ch->questgiver != questman)
	{
	    sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    do_say(questman,buf);
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->countdown > 0)
	    {
		int reward, pointreward, pracreward;

		reward = number_range(300,900);
                pointreward = number_range(10,40);
if (dbl_exp || timed_dbl_exp)
        {    
		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.{#(Double EXP/AQP Bonus){x",pointreward*2,reward*2);
		do_say(questman,buf);
		if (chance(3))
		{
            pracreward = number_range(1,5);
            if (pracreward*2 >= 10)
            {
            pracreward = 5;
        }
		    sprintf(buf, "You gain %d practices!{x\n\r",pracreward*2);
		    send_to_char(buf, ch);
		    ch->practice += pracreward*2;
		}

	        REMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
                ch->questroom = 0;
	        ch->nextquest = 10;
//		ch->gold += reward;
                add_cost(ch,reward*2,VALUE_GOLD);
		ch->questpoints += pointreward*2;
}
else
{
	    sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
	    sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		do_say(questman,buf);
		if (chance(3))
		{
                    pracreward = number_range(1,5);
		    sprintf(buf, "You gain %d practices!\n\r",pracreward);
		    send_to_char(buf, ch);
		    ch->practice += pracreward;
		}

	        REMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
                ch->questroom = 0;
	        ch->nextquest = 10;
//		ch->gold += reward;
                add_cost(ch,reward,VALUE_GOLD);
		ch->questpoints += pointreward;
	}
	        return;
	    }
	    else if (ch->questobj > 0 && ch->countdown > 0)
	    {
		bool obj_found = FALSE;

    		for (obj = ch->carrying; obj != NULL; obj= obj_next)
    		{
        	    obj_next = obj->next_content;
        
		    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward, pracreward;

		    reward = number_range(300,700);
                    pointreward = number_range(15,40);

		    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
		    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

if (dbl_exp || timed_dbl_exp)
        {    
		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.{#(Double EXP/AQP Bonus){x",pointreward*2,reward*2);
		do_say(questman,buf);
		    if (chance(15))
		    {
		        pracreward = number_range(1,10);
				if (pracreward*2 >= 10)
            	{
            	pracreward = 5;
        		}
		        sprintf(buf, "You gain %d practices!{x\n\r",pracreward*2);
		        send_to_char(buf, ch);
		        ch->practice += pracreward*2;
		    }
                    if ( chance(25) )
                    {
                       if ( ch->level < 101 )
                          gain_exp(ch,expreward);
                       else
                          gain_exp(ch,expreward/2);
                     
                       sprintf(buf,"{mWith completing that quest you gained {Y%d{m experience points!",expreward);
                       do_say(questman,buf);
                    }
                    if ( chance(5) )
                    {
                       ch->qps += 1;
                       sprintf(buf,"{CWith time comes rewards, you earned 1 {YIQ{C!{x");
                       do_say(questman,buf);
                    }  

	            REMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
	            ch->nextquest = 10;
//		    ch->gold += reward;
                    add_cost(ch,reward*2,VALUE_GOLD);
		    ch->questpoints += pointreward*2;
		    extract_obj(obj);
	    }
	    else
	    {
		    sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		do_say(questman,buf);
		    if (chance(15))
		    {
		        pracreward = number_range(1,8);
		        sprintf(buf, "You gain %d practices!\n\r",pracreward);
		        send_to_char(buf, ch);
		        ch->practice += pracreward;
		    }
		        if ( chance(25) )
                    {
                       if ( ch->level < 101 )
                       {
	                   sprintf(buf,"{mWith completing that quest you gained {Y%d{m experience points!",expreward);
                       do_say(questman,buf);
                       gain_exp(ch,expreward);
                       }
                       else
                       {
	                   sprintf(buf,"{mWith completing that quest you gained {Y%d{m experience points!",expreward/2);
                       do_say(questman,buf);
                       gain_exp(ch,expreward/2);
                       }
                     

                    }
                    if ( chance(5) )
                    {
                       ch->qps += 1;
                       sprintf(buf,"{CWith time comes rewards, you earned 1 {YIQ{C!{x");
                       do_say(questman,buf);
                    }  

	            REMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
	            ch->nextquest = 10;
//		    ch->gold += reward;
                    add_cost(ch,reward,VALUE_GOLD);
		    ch->questpoints += pointreward;
		    extract_obj(obj);
	    }
		    return;
		}
		else
		{
		    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
	    {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf,"But you didn't complete your quest in time!");
	else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
	do_say(questman, buf);
	return;
    }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
    long mcounter;
    int level_diff, mob_vnum;

    /*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */

    for (mcounter = 0; mcounter < 99999; mcounter ++)
    {
	mob_vnum = number_range(50, 32600);

	if ( (vsearch = get_mob_index(mob_vnum) ) != NULL )
	{
	    level_diff = vsearch->level - ch->level;

		/* Level differences to search for. Moongate has 350
		   levels, so you will want to tweak these greater or
		   less than statements for yourself. - Vassago */
		
	    if (((level_diff < 70 && level_diff > -25)
		|| (ch->level > 20 && ch->level < 50 && vsearch->level > 10 && vsearch->level < 40)
		|| (ch->level > 50 && vsearch->level > 101))
		&& IS_EVIL(vsearch)
		&& vsearch->pShop == NULL
		&& !IS_SET(vsearch->imm_flags, IMM_SUMMON)
    		&& !IS_SET(vsearch->act,ACT_TRAIN)
    		&& !IS_SET(vsearch->act,ACT_PRACTICE)
    		&& !IS_SET(vsearch->act,ACT_IS_HEALER)
                && vsearch->clan == 0
		&& ( !( strstr( vsearch->area->builders, "Noquest" ) )) 
		&& (ch->in_room->area->continent == vsearch->area->continent) 
		&& chance(35)) break;
		else vsearch = NULL;
	}
    }
    if ( mob_vnum < 21000 && mob_vnum > 19999 )
    {
        vsearch = NULL;
    }

    if ( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL )
    {
	/*sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 2;*/
		generate_quest(ch, questman);
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
/*	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 2;*/
		generate_quest(ch, questman);
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40))
    {
	int objvnum = 0;

	switch(number_range(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}

        questitem = create_object( get_obj_index(objvnum), ch->level );
	obj_to_room(questitem, room);
	ch->questobj = objvnum;
        ch->questroom = room->vnum;

	sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",questitem->short_descr);
	do_say(questman, buf);
	do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
	do_say(questman, buf);
	return;
    }

    /* Quest to kill a mob */

    else 
    {
    switch(number_range(0,1))
    {
	case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
	break;

	case 1:
        sprintf(buf, "The Realms's most heinous criminal, %s, has escaped from the dungeon!",victim->short_descr);
	do_say(questman, buf);
	sprintf(buf, "Since the escape, %s has murdered %d civillians!",victim->short_descr, number_range(2,20));
	do_say(questman, buf);
	do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
	break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
        do_say(questman, buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "That location is in the general area of %s.",room->area->name);
	do_say(questman, buf);
    }
    ch->questmob = victim->pIndexData->vnum;
    }
    return;
}

/* Called from update_handler() by pulse_area */

/*
void quest_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (IS_NPC(ch)) continue;

	if (ch->nextquest > 0) 
	{
	    ch->nextquest--;

	    if (ch->nextquest == 0)
	    {
	        send_to_char("You may now quest again.\n\r",ch);
	        return;
	    }
	}
        else if (IS_SET(ch->act,PLR_QUESTOR))
        {
	    if (--ch->countdown <= 0)
	    {
    	        char buf [MAX_STRING_LENGTH];

	        ch->nextquest = 10;
	        sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
	        send_to_char(buf, ch);
	        REMOVE_BIT(ch->act, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
	    }
	    if (ch->countdown > 0 && ch->countdown < 6)
	    {
	        send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
	        return;
	    }
        }
    }
    return;
}
*/

