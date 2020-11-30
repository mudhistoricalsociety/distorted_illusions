/***************************************************************************
 *          This snippet was written by Donut for the Khrooon Mud.         *
 *            Original Coded by Yago Diaz <yago@cerberus.uab.es>           *
 *        	(C) June 1997	                                           *
 *        	(C) Last Modification October 1997			   *
 *		Major enhancement/neatening done by	                   *
 *					Kharas (root@fading.tcimet.net)    *
 *		(eg: Made sum_dat table, shortened function, etc)          *
 *		Rewrite summoner_data, summoner_table, ... by Yago         *
 *			November 1997					   *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "clan.h"
#define MONEY "Gold"


struct summoner_data
{
    char *name;
    int cost;
    int vnum;
};


const	struct	summoner_data	summoner_table	[]	=
{
    /* Name */		    /* Cost */	    /* Vnum */
    { "The Mutant Dump",	   		150,		11800 },
    { "City Of Annonymous",		200,	       12500 },
    { "Hell",		       100,	      10416 },
    { "Troy",                 9000,           10983 },
    { "Divided Souls",		75,	      11600 },
    { "Drakyri Isle",	       100,	      11700 },
    { "Sanuba Desert",       900,            42900 },
    { "Shadow Grove",           50,            1300 },
    { "Mysteria",        850,            1951 },
    { "Githyanki Keep",        300,            7750 },
    { "Keep of Mahntor",        75,            2361 },
    { "Black Water Park",         7500,            17527 },
    { "Kingdoms",               10000,            17542 },
//    { "Remort Challenge",     1000,            9400 },
    { NULL,			 0,		  0 }
 };

DECLARE_DO_FUN( do_look );

CHAR_DATA * find_summoner	args( ( CHAR_DATA *ch ) );

CHAR_DATA *find_summoner ( CHAR_DATA *ch )
{
    CHAR_DATA * summoner;

    for ( summoner = ch->in_room->people; summoner != NULL; summoner = summoner->next_in_room )
    {
	if (!IS_NPC(summoner))
	    continue;

	if (summoner->spec_fun == spec_lookup( "spec_summoner" ))
	    return summoner;
    }

   if (summoner == NULL || summoner->spec_fun != spec_lookup( "spec_summoner" ))
   {
	send_to_char("You can't do that here.\n\r",ch);
	return NULL;
   }

   if ( summoner->fighting != NULL)
   {
	send_to_char("Wait until the fighting stops.\n\r",ch);
	return NULL;
   }

   return NULL;
}


void do_travel(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *summoner,*pet;
    ROOM_INDEX_DATA *room;
    int i;
    char  buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
    char  arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg );
    argument = one_argument(argument, arg1);

    summoner = find_summoner (ch);

    if (!summoner)
        return;

    if (arg[0] == '\0')
    {
	sprintf(buf, "You must tell me what travel you want to do:\n\r"
		     "\tTRAVEL list - shows possible locations to travel to.\n\r"
		     "\tTRAVEL buy <name> - Travel to selected location.");
	act("$N says '$t'", ch, buf, summoner, TO_CHAR);
	return;
    }

    if (!str_cmp( arg, "list"))
    {
	sprintf(buf, "%s says you may travel to the following locations:\n\r", PERS(ch,summoner));
	send_to_char(buf,ch);
	for( i=0; summoner_table[i].name != NULL; i++ )
	{
	    sprintf(buf,"\t%-15s - %-3d %s\n\r",
		summoner_table[i].name, summoner_table[i].cost, MONEY);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_cmp( arg, "buy"))
    {
	if (arg1[0] == '\0')
	{
	    sprintf(buf, "You must tell me where you wish to travel");
	    act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
	    return;
	}

	for( i=0; summoner_table[i].name != NULL; i++ )
	    if(is_name(arg1,summoner_table[i].name))
		break;

	    if(summoner_table[i].name == NULL)
	    {
		sprintf(buf, "That location of travel is not on the list");
		act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
       		return;
            }

	    if ( ( ch->silver / 100 + ch->gold + ch->platinum * 100 ) >= summoner_table[i].cost )  
	       send_to_char("Splendid, you have the required funds.\n\r",ch);
            else
            {
		sprintf(buf, "You don't have enough %s for my services", MONEY);
		act("$N says '$t'.", ch, buf, summoner, TO_CHAR);
		return;
	    }

	    room = get_room_index(summoner_table[i].vnum);

            if( !room )
	    {
		send_to_char("That location's room doesn't seem to exist,"
		    "Please inform your Imp.\n\r",ch);
	        return;
 	    }

	    if ( (pet = ch->pet) != NULL && pet->in_room == ch->in_room)
	    {
		char_from_room( pet );
		char_to_room( pet, get_room_index(summoner_table[i].vnum) );
	    }

	    char_from_room( ch );
	    char_to_room( ch, get_room_index(summoner_table[i].vnum) );

	    deduct_cost(ch,summoner_table[i].cost,VALUE_GOLD);
	    sprintf(buf, "%s utters the words 'hasidsindsad'\n\rYou are surrounded by a violet fog.\n\r", summoner->short_descr);
	    send_to_char(buf, ch);
	    do_look (ch, "");
	    return;
    }
    else
	do_travel(ch,"");
}
