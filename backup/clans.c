/****************************************************
 * Clan system implemented and fixed by Jeremias    *
 *      This source is free of any bug !!!          *
 ****************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/timeb.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "clan.h"

DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_say);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN(do_look);

char * total_length args ((char *input, int length));

void clan_entry_trigger args((CHAR_DATA *ch, sh_int clan));
/* OLD-OLD CLAN INFO
 * 
 * CLAN NAME   CLAN NUMBER
 * -----------------------
 * Independent  clan = 0
 * Loner        clan = 1
 * Outcast      clan = 2
 * Seekers      clan = 3
 * Justice      clan = 4
 * GrimDragon   clan = 5
 * SeveN        clan = 6

 * OLD CLAN INFO
 *
 * #   CLAN
 * ---------
 * 0 = NOONE
 * 1 = LONER
 * 2 = OUTCAST
 * 3 = ANGELS
 * 4 = ENTROPY
 * 5 = ANCIENTS
 * 6 = NIGHTSHADE
 * 7 = LEGIONS
 * 8 = ADIUVO

 * NEW CLAN INFO
 *
 * #   CLAN
 * ---------
 * 0 = NOONE
 * 1 = LONER
 * 2 = OUTCAST
 * 3 = SEEKERS
 * 4 = KNIGHTS 
 * 5 = NIGHTSHADE
 * 6 = ADIUVO
 * 7 = NULL
 * 8 = NULL

 */

// This from CLAN_LIST.C -- Just a calculated number from Clan Members
extern sh_int clan_members[MAX_CLAN];

// Now is read in on bootup from Files
struct clan_type clan_table[MAX_CLAN] =
{
	/* independent should be FALSE if is a real clan */
	/* name
	   who entry
	   deathroom, recall
           indep?, pk? */
	{"",
	 "",
	 "Noone",
	 ROOM_VNUM_MORGUE, ROOM_VNUM_TEMPLE,
         TRUE, FALSE,0,0},

	{"loner",
	 "({!PK{x)",
	 "Noone",
	 ROOM_VNUM_MORGUE, ROOM_VNUM_TEMPLE,
         TRUE, TRUE,0,0},

	{"outcast",
	 "({!OUTCAST{x)",
	 "Noone",
	 ROOM_VNUM_MORGUE, ROOM_VNUM_TEMPLE,
         TRUE, TRUE,0,0},

	{"seekers",
	 "{3[{&S{7e{8e{!k{8e{7r{&s{3]{x",
	 "Noone",
	 20050, 20053,
         FALSE, TRUE,0,0},
};

const struct clan_titles clan_rank_table[MAX_RANK] =
{
    	{" ---"},
        {" {!Rcr{x"},
        {" {@Sdr{x"},
        {" {cLnt{x"},
    	{" {#Cdr{x"},
        {" {bElt{x"},
    	{" {^VLr{x"},
        {" {&Ldr{x"}
};

const char *
lookup_rank (int cs)
{
	switch (cs)
	{
	case LEADER:
		return "the Leader";
	case SECOND:
                return "the Vice-Leader";
        case RANK_ELITE:
                return "an Elite member";
	case DEPUTY:
                return "a Commander";
        case RANK_LIEUTENANT:
                return "a Lieutenant";
	case SENIOR:
                return "a Soldier";
	case JUNIOR:
                return "a Recruit";
	}
	return "a member";
}

/* returns clan number */
int clan_lookup (const char *name)
{
	int clan;

	for (clan = 0; clan < MAX_CLAN; clan++)
	{

		if (LOWER (name[0]) == LOWER (clan_table[clan].name[0])
			&& !str_prefix (name, clan_table[clan].name))
			return clan;
	}

        return INDEP;    /* none found return independent */
}

void do_cleader (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int clan;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	/* Lists all possible clans */
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
                send_to_char ("{wClan list:{x\n\r", ch);
		/*
		 * Won't print the independent
		 */
		for (clan = 0; clan < MAX_CLAN; clan++)
		{
			if (!clan_table[clan].independent)
			{
				sprintf (buf, "   {G%10s{x", clan_table[clan].name);
				send_to_char (buf, ch);
                sprintf (buf, "On now: {G%d{x\n\r", number_on(clan));
                send_to_char (buf, ch);
			}
		}

		send_to_char ("\n\rSyntax: {Gcleader {c<{wchar{c> <{wclan name{c>{x\n\r", ch);
		send_to_char ("If {c<{wclan name{c>{x is {r'{wnone{r'{x clan leadership has been removed.\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("No character by that name exists.\n\r", ch);
		return;
	}

	if (IS_NPC (victim))
	{
		send_to_char ("You must be mad.\n\r", ch);
		return;
	}

	if (!str_cmp (arg2, "none"))
	{
		if (victim->rank == LEADER)
		{
			sprintf (buf, "You remove the leadership from %s.\n\r", victim->name);
			send_to_char (buf, ch);
			sprintf (buf, "You aren't the leader of {G%s{x clan anymore!\n\r", clan_table[victim->clan].name);
			send_to_char (buf, victim);
			victim->rank = MEMBER;	/* make victim a member of the CLAN */
			check_new_clan (victim->name, victim->clan, victim->rank);
			return;
		}

		else
		{
			sprintf (buf, "%s isn't the leader of any clan.\n\r", victim->name);
			send_to_char (buf, ch);
			return;
		}
	}

	if ((clan = clan_lookup (arg2)) == INDEP)
	{
		send_to_char ("No such clan exists.\n\r", ch);
		return;
	}

	if (clan_table[clan].independent)
	{
		send_to_char ("{GOUTCAST{x and {GLONER{x aren't clans!\n\r", ch);
		return;
	}

	else
		/* is a TRUE clan */
	{
		if (victim->rank == LEADER)
		{
			sprintf (buf, "They already hold the leadership of %s clan.\n\r", clan_table[victim->clan].name);
			send_to_char (buf, ch);
			return;
		}

		if (victim->level < 1)
		{
			sprintf (buf, "%s's level is too low to be leader of %s clan.\n\r"
					 "All {GLEADERS{x must be {wlevel {r50{x or higher.\n\r",
					 victim->name, clan_table[clan].name);
			send_to_char (buf, ch);
			return;
		}

		else
		{
			sprintf (buf, "%s is now the {GLEADER{x of %s.\n\r", victim->name, clan_table[clan].name);
			send_to_char (buf, ch);
			sprintf (buf, "You are now the {GLEADER{x of %s.\n\r", clan_table[clan].name);
			send_to_char (buf, victim);
		}
	}

        if(victim->clan != clan)
        {
        victim->clan = clan;		/* make member of the clan */
        clan_members[clan] += 1;
        }
	victim->rank = LEADER;		/* make victim LEADER of the CLAN */
	check_new_clan (victim->name, victim->clan, victim->rank);

       /* Skills */
       {
        int skill;
        if(clan_table[victim->clan].clan_skill1 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill1);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 10;
         }
        }
        if(clan_table[victim->clan].clan_skill2 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill2);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 20;
         }
        }
        if(clan_table[victim->clan].clan_skill3 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill3);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 30;
         }
        }
        if(clan_table[victim->clan].clan_skill4 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill4);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 40;
         }
        }
        if(clan_table[victim->clan].clan_skill5 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill5);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 50;
         }
        }
        if(clan_table[victim->clan].clan_skill6 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill6);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 60;
         }
        }

       }
}

void do_clanlist (CHAR_DATA * ch, char *argument)
{
        char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH/10];
	int clan;

        // IF in clan....show kills & deaths
        // IF not in clan ... show kills only
        // IF in clan....show War status with other clans

        send_to_char ("{c***********************************************{x\n\r", ch);
        send_to_char ("{c*      {w The Clans of {$D{&i{8s{$t{&o{8r{$t{&e{8d {wIllusions      {c*\n\r", ch);
        send_to_char ("{c***********************************************{x\n\r", ch);
        send_to_char ("{c Clan Name  Status     Whoname         Coclan   {x\n\r", ch);
        send_to_char ("{w-----------------------------------------------{x\n\r", ch);

	for (clan = 1; clan < MAX_CLAN; clan++)
	{
	if ( !str_cmp( clan_table[clan].name, "unused" ) )
	    break;

         sprintf (buf, "{@%-12s{x", clan_table[clan].name );
         sprintf (buf2, "%-9s{x", (clan_table[clan].pkill == TRUE) ? "{!PKill" : "{3Non-PK");
         strcat(buf, buf2);
         sprintf(buf2," %s{x", total_length( clan_table[clan].who_name, 13 ) );
         strcat(buf, buf2);
         if (clan_table[clan].coclan)
         {
	     sprintf(buf2,"%s{x\n\r",clan_table[clan_table[clan].coclan].who_name);
         strcat(buf, buf2);
         }
         if (!clan_table[clan].coclan)
         {
         sprintf(buf2,"       {8None{x\n\r");
         strcat(buf, buf2);
         }
         /*
         sprintf(buf2,"%-7d\n\r",clan_table[clan].deaths);
         strcat(buf, buf2);
         */
	     send_to_char (buf, ch);
	}
        return;
}

void do_clantalk (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
        bool IMMTALK = FALSE;
        int temp_clan = 0;

	if (!is_clan (ch) || clan_table[ch->clan].independent)
	{
		send_to_char ("You aren't in a clan.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_NOCLAN))
		{
			send_to_char ("{RClan{x channel is now {GON{x\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_NOCLAN);
		}

		else
		{
			send_to_char ("{RClan{x channel is now {ROFF{x\n\r", ch);
			SET_BIT (ch->comm, COMM_NOCLAN);
		}
		return;
	}

	/*if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
		send_to_char ("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}*/
	
    if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
           && (!IS_IMMORTAL (ch)))
        {
           send_to_char ("Just keep your nose in the corner like a good little player.\n\r", ch);
           return;
        }

        if(IS_SET(ch->in_room->room_flags,ROOM_SILENCED) && !IS_IMMORTAL(ch))
        {
         send_to_char("You have lost your powers of speech in this room!\n\r",ch);
         return;
        }
        
        if(IS_IMMORTAL(ch))
        {
         char arg[MSL/4];
         int sendclan;

         argument = one_argument(argument,arg);
         temp_clan = ch->clan;

         if((sendclan = clan_lookup(arg)) != 0 )
         {
          ch->clan = sendclan;
          IMMTALK = TRUE;
         }
         else
         {
          strcat(arg," "); // Fill in that damn space
          strcat(arg,argument);
          argument = arg;    /* redistribute */
         }
        }

	REMOVE_BIT (ch->comm, COMM_NOCLAN);
        sprintf (buf, "{3[{RCLAN{3]{x You '{9%s{x'\n\r", argument);
	send_to_char (buf, ch);

//        if (ch->rank >= JUNIOR)
                sprintf (buf, "{3[{x%s - $c{3]{x %s '{9%s{x'{x\n\r", clan_rank_table[ch->rank].rank, PERS(ch,ch), argument);

//        else
//                sprintf (buf, "$c{x %s '{9%s{x'{x\n\r", ch->name, argument);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->connected == CON_PLAYING
			&& d->character != ch
                        && d->character->trust >= 110
			&& !IS_SET (d->character->comm, COMM_NOCLAN)
			&& !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new ("$c $n '{9$t{x'", ch, argument, d->character, TO_VICT, POS_DEAD);
		}
		else if (d->connected == CON_PLAYING
				 && d->character != ch
				 && is_same_clan (ch, d->character)
				 && !IS_SET (d->character->comm, COMM_NOCLAN)
				 && !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
		}
	}

        if(IMMTALK)
        ch->clan = temp_clan;

	return;
}

void do_cgossip (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	if (!is_clan (ch) )
	{
		send_to_char ("You aren't in a clan.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_NOCLAN))
		{
			send_to_char ("{RClan{x channels are now {GON{x\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_NOCLAN);
		}

		else
		{
			send_to_char ("{RClan{x channels are now {ROFF{x\n\r", ch);
			SET_BIT (ch->comm, COMM_NOCLAN);
		}
		return;
	}

	if (IS_SET (ch->comm, COMM_NOCHANNELS) || IS_SET(ch->comm, COMM_NOPUBCHAN))
	{
		send_to_char ("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}

        if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
           && (!IS_IMMORTAL (ch)))
        {
           send_to_char ("Just keep your nose in the corner like a good little player.\n\r", ch);
           return;
        }

	REMOVE_BIT (ch->comm, COMM_NOCLAN);
	sprintf (buf, "{3[{RCGOSSIP{3]{x You '{w%s{x'\n\r", argument);
	send_to_char (buf, ch);

	if (IS_IMMORTAL(ch))
                sprintf (buf, "{W[{rCGossip]-{x{3[{WIMMORTAL{3]{x %s '{,%s{x'{x\n\r", ch->name, argument);
	else if (ch->rank >= JUNIOR)
                sprintf (buf, "{W[{rCGossip]-{x{3[{x%s - $c{3]{x %s '{,%s{x'{x\n\r", clan_rank_table[ch->rank].rank, ch->name, argument);
	else
                sprintf (buf, "{W[{rCGossip]-{x$c{x %s '{,%s{x'{x\n\r", ch->name, argument);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->connected == CON_PLAYING
			&& d->character != ch
                        && d->character->level >= 108
			&& !IS_SET (d->character->comm, COMM_NOCLAN)
			&& !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new ("{W[{rCGossip]-{x$c $n '{,$t{x'", ch, argument, d->character, TO_VICT, POS_DEAD);
		}
		else if (d->connected == CON_PLAYING
				 && d->character != ch
				 && (is_clan (d->character) 
				  || !clan_table[d->character->clan].independent)
				 && !IS_SET (d->character->comm, COMM_NOCLAN)
				 && !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
		}
	}

	return;
}

void 
do_promote (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument (argument, arg1);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not promote someone.\n\r", ch);
		return;
	}

	if ((ch->rank != LEADER) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You must be a clan Leader to promote someone.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: {Gpromote {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They must be playing to be promoted.\n\r", ch);
		return;
	}

	if (IS_NPC (victim) )
	{
		send_to_char ("You must be mad.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not promote yourself.\n\r", ch);
		return;
	}

    if ((victim->clan != ch->clan) && (!IS_IMMORTAL (ch)))
    {
        send_to_char ("You can not promote a player who is not in your clan.\n\r", ch);
        return;
    }

	if (victim->rank >= SECOND)
	{
		send_to_char ("You can not promote this player anymore.\n\r", ch);
		return;
	}

	victim->rank += 1;
	check_new_clan (victim->name, victim->clan, victim->rank);

	sprintf (buf, "They are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, ch);
	sprintf (buf, "You are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, victim);
	return;
}

void 
do_demote (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument (argument, arg1);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not demote someone.\n\r", ch);
		return;
	}

	if ((ch->rank != LEADER) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You must be a clan Leader to demote someone.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: {Gdemote {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They must be playing to be demoted.\n\r", ch);
		return;
	}

	if (IS_NPC (victim) || (victim->clan <= OUTCAST))
	{
		send_to_char ("You must be mad.\n\r", ch);
		return;
	}

	if ((victim->clan != ch->clan) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You can not demote a player who is not in your clan.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not demote yourself.\n\r", ch);
		return;
	}

	if (victim->rank <= MEMBER)
	{
		send_to_char ("You can not demote this player anymore.\n\r", ch);
		return;
	}

	victim->rank -= 1;
	check_new_clan (victim->name, victim->clan, victim->rank);

	sprintf (buf, "They are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, ch);
	sprintf (buf, "You are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, victim);
	return;
}

void do_exile (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument (argument, arg);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not demote someone.\n\r", ch);
		return;
	}

	if (ch->rank != SECOND && ch->rank != LEADER && !IS_IMMORTAL (ch))
	{
		send_to_char ("You must be a clan Leader or Auxiliary to exile someone.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char ("Syntax: {Gexile {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg)) == NULL)
	{
		if (IS_IMMORTAL(ch))
		{
			send_to_char ("Attempting to exile logged-out player.\n\r", ch );
			check_new_clan (arg, OUTCAST, MEMBER);
			return;
		}
		else
		{
			send_to_char ("They aren't playing.\n\r", ch);
			return;
		}
	}

	if (IS_IMMORTAL (victim))
	{
		send_to_char ("You can't outcast an immortal.", ch);
		return;
	}

        if(!IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
        {
         if(ch->rank == SECOND && victim->rank == LEADER)
         {
          send_to_char("You can't outcast your leader.\n\r",ch);
          printf_to_char(victim,"ALERT! %s just tried to exile you.\n\r",ch->name);
          return;
         }
        }

	if (IS_NPC (victim) || (victim->clan <= OUTCAST))
	{
		send_to_char ("You can not exile an NPC or player who is not in your clan.\n\r", ch);
		return;
	}

	if ((victim->clan != ch->clan) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You can not exile a player who is not in your clan.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not exile yourself.\n\r", ch);
		return;
	}

	/* leader outcasts victim */
	send_to_char ("They are now {GOUTCAST{x from the clan.\n\r", ch);
	sprintf (buf, "You have been {GOUTCAST{x from %s clan!\n\r", clan_table[victim->clan].name);
	send_to_char (buf, victim);
	send_to_char ("Type {r'{Ghelp outcast{r'{x for more information.\n\r", victim);
        if(is_pkill(victim))
         victim->clan = clan_lookup("outcast");         /* OUTCAST GROUP */
        else
         victim->clan = 0;
        clan_members[ch->clan] -= 1;
	victim->rank = MEMBER;
	check_new_clan (victim->name, victim->clan, victim->rank);
	return;
}

void do_loner (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

        if (ch->level > 75 || ch->pcdata->tier > 2)
	{
                send_to_char ("You should have decided that sooner, after level 75, second tier, it's too late.\n\r",ch);
		return;
	}

	if (ch->level < 6)
	{
		send_to_char ("You are still a {GNEWBIE{x, wait until {wlevel {r6{x.\n\r", ch);
		return;
	}

	if (ch->clan == OUTCAST)	/* IS_OUTCAST */
	{
		send_to_char ("You are an {GOUTCAST{x!  You can't join a clan."
		   "\n\rType {r'{Ghelp outcast{r'{x for more information.\n\r", ch);
		return;
	}

	if (ch->clan == LONER)
	{
		send_to_char ("You are already a {GLONER{x.\n\r", ch);
		return;
	}

	if (is_clan (ch) && ch->clan != clan_lookup("squire"))
	{
		send_to_char ("You are already in a Clan.\n\r", ch);
		return;
	}

        if(ch->clan != clan_lookup("squire"))
        {
         send_to_char("You must go {GPK{x first.\n\r",ch);
         send_to_char("Type PK, twice to do so.\n\r",ch);
         return;
        }

	if (ch->pcdata->confirm_loner)
	{
		if (argument[0] != '\0')
		{
			send_to_char ("{GLONER{x status removed.\n\r", ch);
			ch->pcdata->confirm_loner = FALSE;
			return;
		}

		else
		{
			send_to_char ("{*{wYou are now a brave {GLONER{x!!\n\r", ch);
			ch->clan = LONER;
			ch->rank = MEMBER;
			ch->pcdata->confirm_loner = FALSE;
			check_new_clan (ch->name, ch->clan, ch->rank);
			return;
		}
	}

	if (argument[0] != '\0')
	{
		send_to_char ("Just type {GLONER{x. No argument.\n\r", ch);
		return;
	}

	do_help (ch, "pkill");
	send_to_char ("\n\r", ch);
	send_to_char ("Type {GLONER{x again to confirm this command.\n\r", ch);
	send_to_char ("{RWARNING:{x this command is irreversible.\n\r", ch);
	send_to_char ("Typing {GLONER{x with an argument will undo delete status.\n\r", ch);
	ch->pcdata->confirm_loner = TRUE;
}

/* Puts into the Squire clan */
void do_pk (CHAR_DATA * ch, char *argument)
{
	if (IS_NPC (ch))
		return;

	if (ch->level > 25 && ch->pcdata->tier == 1)
	{
		send_to_char ("You have the option to go pk next tier before level 25. It will be your last chance.\n\r",ch);
		return;
	}

        if (ch->level > 25 || ch->pcdata->tier > 2)
	{
                send_to_char ("You should have decided that sooner, after level 25, second tier, it's too late.\n\r",ch);
		return;
	}

	if (ch->level < 6)
	{
		send_to_char ("You are still a {GNEWBIE{x, wait until {wlevel {r6{x.\n\r", ch);
		return;
	}

	if (ch->clan == OUTCAST)	/* IS_OUTCAST */
	{
		send_to_char ("You are an {GOUTCAST{x!  You can't join a clan."
		   "\n\rType {r'{Ghelp outcast{r'{x for more information.\n\r", ch);
		return;
	}

        if (ch->clan == clan_lookup("squire"))
	{
                send_to_char ("You are already {GPK{x.\n\r", ch);
		return;
	}

	if (is_clan (ch))
	{
		send_to_char ("You are already in a Clan.\n\r", ch);
		return;
	}

	if (ch->pcdata->confirm_loner)
	{
		if (argument[0] != '\0')
		{
                        send_to_char ("{GPK{x status removed.\n\r", ch);
			ch->pcdata->confirm_loner = FALSE;
			return;
		}

		else
		{
                        send_to_char ("{*{wYou are now a brave {GPK{x!!\n\r", ch);
                        ch->clan = clan_lookup("squire");
			ch->rank = MEMBER;
			ch->pcdata->confirm_loner = FALSE;
			check_new_clan (ch->name, ch->clan, ch->rank);
			SET_BIT(ch->plyr,PLAYER_NEWBIE);
			return;
		}
	}

	if (argument[0] != '\0')
	{
                send_to_char ("Just type {GPK{x. No argument.\n\r", ch);
		return;
	}

	do_help (ch, "pkill");
	send_to_char ("\n\r", ch);
        send_to_char ("Type {GPK{x again to confirm this command.\n\r", ch);
	send_to_char ("{RWARNING:{x this command is irreversible.\n\r", ch);
        send_to_char ("Typing {GPK{x with an argument will undo delete status.\n\r", ch);
	ch->pcdata->confirm_loner = TRUE;
}


void 
do_petition_list (int clan, CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;
	bool flag = FALSE;
	char buf[MAX_STRING_LENGTH];

	for (d = descriptor_list; d; d = d->next)
	{
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING)
		{
			if (victim->petition == clan)
			{
				if (!flag)
				{
					flag = TRUE;
					send_to_char ("The following characters have petitioned your clan:\n\n\r", ch);
				}

				sprintf (buf, "{c[{w%3d %5s %s{c][{w%-10s%-16s{c] ({w%s{c){x\n\r",
						 victim->level,
						 victim->race < MAX_PC_RACE ? pc_race_table[victim->race].who_name : "     ",
						 class_table[victim->class].who_name,
						 victim->name,
						 IS_NPC (victim) ? "" : victim->pcdata->title,
						 clan_table[victim->clan].name);
				send_to_char (buf, ch);
			}
		}
	}

	if (!flag)
		send_to_char ("No-one has petitioned your clan.\n\r", ch);
}

void 
do_petition (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	short status;
        int clan = INDEP;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	status = ch->clan ? ch->rank : 0;

	if (IS_SET(ch->plyr, PLAYER_NEWBIE)) {
		send_to_char("Silly newbie, clans are for real people.\n\r", ch);
		return; }

	if (arg1[0] == 0)
	{
		if (status >= SECOND)
		{
			do_petition_list (ch->clan, ch);
			return;
		}

		if (!ch->petition)
		{
			send_to_char ("Syntax: {Gpetition {c<{wclan name{c>{x.\n\r", ch);
			return;
		}

		if (ch->petition)
		{
			ch->petition = 0;
			send_to_char ("You withdraw your petition.\n\r", ch);
			return;
		}
	}

	if (arg2[0] == 0 && status < SECOND)
	{
		if ((clan = clan_lookup (arg1)) == INDEP)
		{
			send_to_char ("There is no clan by that name.\n\r", ch);
			return;
		}

		if (clan_table[clan].independent)
		{
			send_to_char ("{GOUTCAST{x and {GLONER{x aren't clans!\n\r", ch);
			return;
		}

		if (clan == ch->clan)
		{
			sprintf (buf, "You are already a member of {G%s{x clan.\n\r", clan_table[clan].name);
			send_to_char (buf, ch);
			return;
		}

		if (ch->clan == INDEP && clan_table[clan].pkill == TRUE)
		{
			send_to_char ("You must be at least a {GLONER{x to join a clan.\n\r", ch);
			return;
		}

		else
		{
			if(ch->clan == clan_lookup("outcast"))
			{
				send_to_char("No clan wants you as a member right now!\n\r",ch);
				return;
			}
			ch->petition = clan;
			sprintf (buf, "You have petitioned {G%s{x clan for membership.\n\r",
					 clan_table[clan].name);
			send_to_char (buf, ch);
			return;
		}
	}

	if (status >= SECOND)
	{
		if (!str_prefix (arg1, "accept"))
		{

			if ((victim = get_char_world (ch, arg2)) == NULL)
			{
				send_to_char ("They are not playing.\n\r", ch);
				return;
			}

			if (victim->petition != ch->clan)
			{
				send_to_char ("They have not petitioned your clan.\n\r", ch);
				return;
			}
			
			if(victim->clan == clan_lookup("outcast"))
			{
				send_to_char("Outcasts cannot be clanned!\n\r",ch);
				victim->petition = 0;
				return;
			}

                        clan_members[clan] += 1;
                        victim->clan = ch->clan;
			victim->rank = MEMBER;
			check_new_clan (victim->name, victim->clan, victim->rank);
			victim->petition = 0;
			send_to_char ("You have accepted them into your clan.\n\r", ch);
			send_to_char ("Your clan application was successful.\n\r", victim);
			sprintf (buf, "You are now a proud member of clan {G%s{x.\n\r",
					 clan_table[victim->clan].name);
			send_to_char (buf, victim);

       /* Skills */
       {
        int skill;
        if(clan_table[victim->clan].clan_skill1 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill1);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 10;
         }
        }
        if(clan_table[victim->clan].clan_skill2 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill2);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 20;
         }
        }
        if(clan_table[victim->clan].clan_skill3 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill3);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 30;
         }
        }
        if(clan_table[victim->clan].clan_skill4 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill4);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 40;
         }
        }
        if(clan_table[victim->clan].clan_skill5 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill5);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 50;
         }
        }
        if(clan_table[victim->clan].clan_skill6 != '\0')
        {
         skill = skill_lookup(clan_table[victim->clan].clan_skill6);
         if(skill != -1)
         {
          victim->pcdata->learned[skill] = 75;
          victim->pcdata->learnlvl[skill] = 60;
         }
        }

       }
			return;
		}

		else if (!str_prefix (arg1, "reject"))
		{

			if ((victim = get_char_world (ch, arg2)) == NULL)
			{
				send_to_char ("They are not playing.\n\r", ch);
				return;
			}

			if (victim->petition != ch->clan)
			{
				send_to_char ("They have not petitioned your clan.\n\r", ch);
				return;
			}

			victim->petition = 0;
			send_to_char ("You have rejected there application.\n\r", ch);
			send_to_char ("Your clan application has been rejected.\n\r", victim);
			return;
		}

		send_to_char ("Syntax: {Gpetition accept {c<{wplayer{c>{x\n\r"
					  "        {Gpetition reject {c<{wplayer{c>{x\n\r", ch);
		return;
	}

	send_to_char ("Syntax: {Gpetition {c<{wclan name{c>{x.\n\r", ch);
	return;
}

void update_clanlist (CHAR_DATA * ch, int clannum, bool add, bool clead)
{ /*
  MBR_DATA *prev;
  MBR_DATA *curr;
  CLN_DATA *pcln;

  if (IS_NPC(ch))
  {
   return;
  }

  for (pcln = cln_list; pcln != NULL; pcln = pcln->next)
  {
   if (pcln->clan == clannum)
   {
    if (clead)
    {
     if (!add)
     {
      prev = NULL;

      for ( curr = pcln->list; curr != NULL; prev = curr, curr = curr->next )
      {
       if ( !str_cmp( ch->name, curr->name ) )
       {
        if ( prev == NULL )
         pcln->list = pcln->list->next;
        else
         prev->next = curr->next;

        free_mbr(curr);
        save_clanlist(clannum);
       }
      }
     return;
    }
    else
    {
     curr = new_mbr();
     curr->name = str_dup(ch->name);
     curr->next = pcln->list;
     pcln->list = curr;
     save_clanlist(clannum);
     return;
    }
   }

   if (add)
    pcln->members++;
   else
    pcln->members--;

   if (pcln->members < 0)
    pcln->members = 0;

   save_clanlist(clannum);
  }
 }
 return */
}

/*
 * New Sections -- Skyntil
 */
void do_mark(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int clan;

    argument = one_argument(argument, arg1);

    if(IS_NPC(ch))
     return;

    clan = clan_lookup("ancients");

    if(ch->clan != clan && !IS_IMMORTAL(ch))
    {
     send_to_char("Huh?\n\r",ch);
     return;
    }

    if(arg1[0]=='\0')
    {
      send_to_char("Syntax: mark [<victim>]\r\n",ch);
      return;
    }

    victim = get_char_world(ch, arg1);

    if (victim == NULL || !can_see(ch,victim))
    {
       send_to_char( "They cannot be found.\n\r", ch );
       return;
    }

    if(!is_pkill(victim))
    {
     send_to_char("You cannot mark a NON-PK player.\n\r",ch);
     return;
    }

    // Assuming that victim is found.
    if(!IS_SET(victim->plyr,PLAYER_ANCIENT_ENEMY))
    {
     SET_BIT(victim->plyr,PLAYER_ANCIENT_ENEMY);
     sprintf(buf,"%s has been marked as an enemy of Ancients.",
     capitalize(victim->name));
     send_to_char(buf,ch);
     send_to_char("You are now an enemy of Midgaard.\n\r",victim);
    }
    else
    {
     REMOVE_BIT(victim->plyr,PLAYER_ANCIENT_ENEMY);
     sprintf(buf,"%s is no longer marked as an enemy of Ancients.",
     capitalize(victim->name));
     send_to_char(buf,ch);
     send_to_char("You are no longer an enemy of Midgaard.\n\r",victim);
    }

    return;
}

bool is_enemy(CHAR_DATA *ch, CHAR_DATA *victim)
{
 int clan;

 clan = clan_lookup("ancients");

 if(!IS_IMMORTAL(ch) && ch->clan != clan)
  return FALSE;

 if(ch->clan == clan && IS_SET(victim->plyr,PLAYER_ANCIENT_ENEMY))
  return TRUE;

 return FALSE;
}


/* Clan Guards/Entrances/Triggers/etc */
void clan_entry_trigger(CHAR_DATA *ch, sh_int clan)
{
    char buf[MAX_STRING_LENGTH];
    char *message;
    CHAR_DATA *guardian;

    if (IS_NPC(ch)) return;

    if (IS_IMMORTAL(ch)) return;

    for (guardian = ch->in_room->people; guardian != NULL; guardian = guardian->next_in_room)
    {
        if (!IS_NPC(guardian)) continue;
        if (IS_SET(guardian->off_flags,OFF_CLAN_GUARD)
        && guardian->pIndexData->vnum == clan_table[clan].guardian_vnum)
        break;
    }

    if (guardian == NULL)
    return;

    if (is_same_clan(ch,guardian)||IS_IMMORTAL(ch))
    {
        sprintf(buf,"Greetings, %s", clan_table[clan].guardian_say);
        do_say(guardian, buf);
        return;
    }

    if (ch->level <= 75  || IS_AFFECTED(ch, AFF_CHARM))
    {
        /* say something, then move em out */
                switch (number_range(0,6))
                {
                default : message = NULL;
                        break;
                case 0: message = "$n says 'This area is restricted friend.'\n\r";
                        break;
                case 1: message = "$n looks at you and gestures silently.\n\r";
                        break;
                case 2: message = "$n asks 'Ever dance with the devil in the pale moon light?'\n\r";
                        break;
                case 3: message = "$n says 'Begone from this place.'\n\r";
                        break;
                case 4: message = "$n says 'Leave now or suffer.'\n\r";
                        break;
                case 5: message = "$n says 'Welcome, too bad you cannot stay.'\n\r";
                        break;
                case 6: message = "$n utters the words 'word of recall'\n\r";
                        break;
                }

                if (message != NULL)
                act(message,guardian,NULL,ch,TO_ROOM);
                        
                char_from_room(ch);
                char_to_room(ch, (get_room_index(ROOM_VNUM_TEMPLE)));
                do_look( ch, "auto" );
                return;  
    }

    sprintf(buf,"Help! I'm being attacked by %s!",guardian->short_descr);
    do_yell(ch,buf);
    do_clantalk(guardian,"Intruder! Intruder!");
    multi_hit(guardian,ch,TYPE_UNDEFINED);
    /* Stop bug abuse :) */
    if(IS_SET(ch->plyr,PLAYER_RUNNING))
     REMOVE_BIT(ch->plyr,PLAYER_RUNNING);

    return;
}

int number_on(int clan)
{
    int ison;
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    ison = 0;
    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->connected != CON_PLAYING)
            continue;
        wch = d->character;
        if (wch->clan == clan)
        ison++;
    }
    return ison;
}
void do_colist (CHAR_DATA * ch)
{
    int clan,coclan;
    char name[MSL];
    char cname[MSL];
    char buf[MSL];
    send_to_char("The clans of Distorted Illusions are allied in the following manner:\n\r",ch);
    for (clan = 1; clan < MAX_CLAN; clan++)
    {
    coclan = clan_table[clan].coclan;
    sprintf(name,"%s",capitalize(clan_table[clan].name));
    sprintf(cname,"%s",capitalize(clan_table[coclan].name));
    if (!str_cmp(name,"Unused"))
    continue;
    if (coclan == 0)
    {
    sprintf(buf, "%s has no allies.\n\r",capitalize(clan_table[clan].name));
    send_to_char(buf,ch);
    continue;
    }
    if (clan_table[clan].coclan < clan && clan_table[clan].coclan != 0)
    continue;

    sprintf(buf, "%s and %s are allied.\n\r",name,cname);
    send_to_char(buf,ch);
    }
    return;
}
