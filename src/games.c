/*
Slot code by Jair for Asgardian Nightmares
Written for Rot
*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN(do_announce    );











/***********************************************************
  fp = fopen(SLOT_FILE,"r");
  slotMachine->value[1] = fread_number(fp);
  fclose(fp);
************************************************************/
//no idea what this does










/*****Functions*******/
int Roll_Slots();
void Update_Player_Money(CHAR_DATA *ch, int silver);
int Slots_Win(int slot_roll1, int slot_roll2, int slot_roll3, int bet, int Slots_Jackpot, OBJ_DATA *slotMachine);




void do_slots(CHAR_DATA *ch, char *argument)
{
        char *bar_messages[] = {
        "<------------>",
        "{Y  Aegir  {x",    /* 1 */
        "{R  Odin   {x",        /* 2 */
        "{M   Hel   {x",    /* 3 */
        "{c  Loki   {x",        /* 4 */
        "{C  Thor   {x",    /* 5 */
        "{y  Freya  {x",        /* 6 */
        "{r Forseti {x",        /* 7 */
        "{G   Tyr   {x",        /* 8 */
        "{W   Sif   {x",        /* 9 */
        "{B  Jord   {x",        /* 10*/
        };


	//FILE *fp;
	OBJ_DATA *slotMachine;
	char buf[MAX_STRING_LENGTH];
	//int winArray[11];
	int Slots_Jackpot, cost;	
	char arg[MSL/10];
	int bet, total_silver, slot_roll1, slot_roll2, slot_roll3, money_won;
	bool foundSlot;

	//check if arguments are correct
	argument = one_argument(argument,arg); // the players bet

	if(arg[0] == '\0')
	{
		send_to_char("{@Syntax: {#slots {$<{&amount{$>{x\n\r",ch);
		return;
	}

	/*if(!is_number(arg))
	{
		send_to_char("{@Syntax: {#slots {$<{&amount{$>{^ amount in silver{x\n\r",ch);
		return;
	}*/

	bet = atoi(arg);



	//First, check if the character is an npc
	if(IS_NPC(ch))
	{
		send_to_char("Sorry, only player characters may use the slot machine.\n\r", ch);
		return;
	}


	//Second, check if there is a slot machine here, if it is working


	foundSlot = FALSE; 

	for (slotMachine=ch->in_room->contents;slotMachine!=NULL; slotMachine=slotMachine->next_content)
	{
		if ( (slotMachine->item_type == ITEM_SLOT_MACHINE) && (can_see_obj(ch,slotMachine)))
 		{
			foundSlot = TRUE;
			break;
		}
	}
  
	if(foundSlot == FALSE)
	{
		send_to_char("There is no slot machine here.\n\r", ch);
		return;
	}
	
	cost = slotMachine->value[0];

	if(cost <= 0)
	{
		send_to_char("This slot machine seems to be broken.\n\r", ch);
		return;
	}




	//bet needs to be in silver, so get total money character has

	
	total_silver = ch->silver + (ch->gold * 100) + (ch->platinum * 10000);


	//Check if bet is greater than the money the character has

	if ( bet > total_silver )
	{
		send_to_char("You can't bet more than you have.\n\r",ch);
		return;
	}

	if ( bet > 100000 )
	{
		send_to_char("{@You can't bid more than 10 platinum.{0\n\r",ch);
		return;
	}

	if ( bet < 0 )
	{
		send_to_char("{@You can't bid a negative number.{0\n\r",ch);
		return;
	}	

	//Backup Slots
/*  This didn't like me, so I took it out. */

/*  
  	fp = fopen(SLOT_FILE,"r");
  	slotMachine->value[1] = fread_number(fp);
  	fclose(fp);
*/

	//so, the character has enough money to cover its bet, slots on
	Slots_Jackpot = slotMachine->value[1];


	//subtract the bet to start
	total_silver = total_silver - bet;






	//run slot code
	if (!str_cmp(ch->name,"Callin"))
	{
	slot_roll1 = 1;
	slot_roll2 = 1;
	slot_roll3 = 1;
	}
	else
	{
	slot_roll1 = Roll_Slots();
	slot_roll2 = Roll_Slots();
	slot_roll3 = Roll_Slots();
}
//Tien's Code

	send_to_char("{x\n\r{x     {3_______________________________________    {!_{x\n\r", ch);
	send_to_char("{x    {3/{* ------------------------------------- {3\\  {!(_){x\n\r", ch);
	send_to_char("{x   {3|       Distorted Illusions {@Slots  {7-{*|{7-    {3| {7/{7/{x\n\r", ch);
	send_to_char("{x   {3|{!> {@************************************* {!<{3|{7/{7/{x\n\r", ch);
	sprintf(buf, "{x   {3|{!> {@[ %s {@| %s {@| %s {@] {!<{3|{7/{x\n\r", bar_messages[slot_roll1], bar_messages[slot_roll2], bar_messages[slot_roll3]);
	send_to_char(buf, ch);
	send_to_char("{x   {3|{!> {@************************************* {!<{3|{x\n\r", ch);
	send_to_char("{x    {3\\{*---------------------------------------{3/{x\n\r", ch);
	sprintf( buf, "{x     {#Current Jackpot:{& %d {7silver{x\n\r{x\n\r", Slots_Jackpot );
	send_to_char(buf, ch);


	//check if the player won
	if (slot_roll1 != slot_roll2 && slot_roll1 != slot_roll3 && slot_roll2 != slot_roll3)
	{
		//update jackpot
		Slots_Jackpot = Slots_Jackpot + bet;

		if (Slots_Jackpot > 10000000)
			Slots_Jackpot = 10000000;

		//player did not win
      	sprintf(buf, "Sorry you didn't win anything. The jackpot is now worth %d silver.\n\r",
	      Slots_Jackpot);
	
  
    	send_to_char(buf, ch);		
		slotMachine->value[1]=Slots_Jackpot;
		Update_Player_Money(ch, total_silver);

		return;
	}


	//the player has won something, find how much they won.


	//money won is the amount they won total, to be added to their money
	
	money_won = Slots_Win (slot_roll1, slot_roll2, slot_roll3, bet, Slots_Jackpot, slotMachine);


	//update players total_silver
	//Check if the slot machine can cover the win
	if (Slots_Jackpot < money_won)
	{
		//slots cannot handle the amount, give over as much as possible
		total_silver = total_silver + Slots_Jackpot;
		
      	sprintf(buf, "The slot machine cannot cover your winnings.  You get %d silver.\n\r", Slots_Jackpot);
      	send_to_char(buf, ch);	

		Slots_Jackpot = 0;
		slotMachine->value[1]=Slots_Jackpot;

		
		Update_Player_Money(ch, total_silver);
		return;
	}


	total_silver = total_silver + money_won + bet;
	Slots_Jackpot = Slots_Jackpot - money_won;
	slotMachine->value[1]=Slots_Jackpot;


	if( (slot_roll1 == slot_roll2) && (slot_roll2 == slot_roll3) )
	{
      sprintf(buf, "You won the jackpot worth %d silver!! The jackpot now stands at %d silver.\n\r",
              money_won, slotMachine->value[1]);
      send_to_char(buf, ch);
	}
	else
	{
      sprintf(buf, "You matched 2 bars and won %d silver! The jackpot is now worth %d silver.\n\r",
	      money_won, slotMachine->value[1]);
	  send_to_char(buf, ch);
	}



	Update_Player_Money(ch, total_silver);

	return;
}


int Roll_Slots()
{
	int slot = 0;

	slot = number_range(1,49);

	if (slot == 1)
	return 1;				//tas		1/49
	else if (slot == 2 || slot == 3)
	return 2;				//skyntil	2/49
	else if (slot >=4 && slot <=6)
	return 3;				//lucifer	3/49
	else if (slot >=7 && slot <=10)
	return 4;				//raolin	4/49
	else if (slot >=11 && slot <= 15)
	return 5;				//kefka		5/49
	else if (slot >=16 && slot <= 21)
	return 6;				//jasrags	6/49
	else if (slot >=22 && slot <= 28)
	return 7;				//deblin	7/49
	else if (slot >=29 && slot <= 36)
	return 8;				//anonymous	8/49
	else if (slot >=37 && slot <= 45)
	return 9;				//gavorkian	9/49
	else
	return 10;				//jair		10/49
}



int Slots_Win(int bar1, int bar2, int bar3, int bet, int jackpot, OBJ_DATA *slotMachine)
{
	int winnings;

	//Check if all 3 are the same, declare jackpot
	winnings = 0;

	if( (bar1 == bar2) && (bar2 == bar3) )
	{
 		if(bar1 <= 5)
		{
			winnings = bet * (11-bar1) * 1.5;  /* they won the jackpot, make it */
					          				 /* worth their while!            */
		}
		else
			winnings = bet * (11-bar1);
	
		slotMachine->value[1] -= winnings;   /* put it back to something */
		slotMachine->value[1] += bet;
		return winnings;
    }
	if(bar1 == bar2 || bar1 == bar3)
	{
 		winnings += ((bet*12)/(bar1));
		return winnings;
	}
	if(bar2 == bar3)
	{
		winnings += ((bet*12)/(bar2));
		return winnings;
	}
	return 0;
}


void Update_Player_Money(CHAR_DATA *ch, int total_silver)
{
	int plat, gold, silver;

	plat = total_silver / 10000;
	total_silver -= plat*10000;
	gold = total_silver / 100;
	total_silver -= gold*100;
	silver = total_silver;
	
	ch->silver = silver;
	ch->gold = gold;
	ch->platinum = plat;
}

