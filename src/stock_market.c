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
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			                   *
*	ROM has been brought to you by the ROM consortium		               *
*	    Russ Taylor (rtaylor@pacinfo.com)				                   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			                   *
*	    Brian Moore (rom@rom.efn.org)				                       *
*	By using this code, you have agreed to follow the terms of the	       *
*	ROM license, in the file Rom24/doc/rom.license			               *
*   ROT 1.5 is copyright 1996-1997 by Russ Walsh                           * 
*   By using this code, you have agreed to follow the terms of the         * 
*   ROT license, in the file doc/rot.license                               * 
***************************************************************************/
/***************************************************************************
 * This code was written by Bree (gothicbree@hotmail.com)                  *
 * for use exclusively on Distorted Illusions v2.5b                        *
 * (asgarddi.dune.net port:6069). This code may not                        *
 * be used without permission from the author.                             *
 * Copyright(c) 2009, all rights reserved.                                 *
 ***************************************************************************/
/* Special Thanks to Fesdor, without whom this code would probably not exist.
   His invaluable insight and ideas about this code made it what it is today.
   He also had the patience to clean up some of the code to make it compatible
   with GCC4. So Again, thank you Fesdor.*/
/* This code was finished on: / /09
 * As of the date:  / /09 this code is freeware with the understanding that you
 * must send me an email telling me your mud's address and that you are using
 * the code. I appreciate it. (gothicbree@hotmail.com) 
 */
/* This is a code for a stock market for a RoT based mud. It was tested on heavily
 * modified RoT 1.5, but will probably work for any modified version of RoT. It should
 * also be easily converted for Rom 2.4b. You will need to make a new spec named
 * stockman and he will be where your players buy/sell stocks and also see the
 * current stock prices. Stock prices will fluctuate in update.c every 2 real
 * hours. Update simply calls the autoflux() and clanflux() functions every 2 hours.
 * An announcement is made when the stock market fluctuates, along with bonus msg's
 * if the market has a surge or a crash or a stock is in danger of going bankrupt.
 * When or if a stock goes bankrupt it will automatically become worthless due to the
 * bankruptcy funtion which goes through all the players pfiles, changing the value of
 * any stock they have they went bankrupt, making it worthless, and changing the name
 * to a bad stock. There is an immortal command called Fluct which only IMP's should
 * have. This command allows you to either raise or lower the current stock prices by 
 * 15%, reset the stock prices to their base prices, force an autoflux(), or force a
 * clanflux(). This comes in handing for testing and other purposes. The percentages
 * that the stocks rise and decline are decided randomly by a nifty little stockpercent
 * generator that Fesdor wrote. The only stock not affected by this is the Aesir clan
 * stock, which costs alot more, but as a result is more stable and fluctuates at a fixed
 * number range rate independent of the other flucuations in the normal market. It is
 * considered to be a very good long term investment if you can afford it. But this stock
 * does not reset back to it's original value, so get in early. Also the Aesir stock is
 * a more solid investment and has been proven to go up in the long run so the numbers
 * work out, but feel free to change them around, aesir was meant as a solid long term
 * investment for the players. Anyway, enjoy. -Bree
 */ 
 
/*                               INSTRUCTIONS: 
 * The following changes need to be made to the following files:
 *   **Merc.h:**
 *     #define VNUM_OBJ_STOCK_A vnum;  //You must do this for every stock you have A, then B, C, ect.. 
 *     You must also define pulse_market to the desired number of ticks for the market to fluctuate at.
 *   **Update.c:**  
 *   put these with local functions:     
 *     void autoflux();
 *     void clanflux();
 *   put this in the update_handler:
 *     static  int     pulse_market;
 * 
 *     if ( --pulse_market    <= 0 )
 *    {
 *      pulse_market = PULSE_MARKET;
 *      market_update ( );
 *    }
 *   put this at the end of the update_handler under obj_update	( );
 *     market_update   ( );
 *   put this directly under the end of update_handler as a new function:
 *     void market_update(void)
 *   {
 *      autoflux();
 *      clanflux();
 *	 return;
 *   }
 *
 * In **Special.c:**
 *     DECLARE_SPEC_FUN(       spec_stockman           );
 *   put this in the spec_type table:
 *     {   "spec_stockman",                spec_stockman           },
 *   put this under spec_questmaster:
 *      bool spec_stockman (CHAR_DATA *ch)
 *     {  
 *      if (ch->fighting != NULL) return spec_cast_mage( ch);
 *      return FALSE;
 *      }
 *
 *  Then you have to put the usual command for do_fluct in the interp.c and interp.h files
 *  DECLARE_DO_FUN ( do_fluct ); in interp.h
 *  { "fluct",          do_fluct,       POS_DEAD,       ML,  1,  LOG_ALWAYS, 1 }, in interp.c
 *
 * 
 *
 *  Other then that simply put stock_market.c into your src folder, add stock_market.o to the makefile,
 *  and do a clean compile. You will get 3 warnings about truth values, but the code is stable so ignore
 *  them or try to fix it if you wish. If you do use this code on your mud please send me an Email telling
 *  me that you are using it, just so i know. gothicbree@hotmail.com Thank You, and i hope you enjoy the code!
 */
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "interp.h"

/* Local Functions */

void stock  args(( CHAR_DATA *ch, CHAR_DATA *stockman ));
AREA_DATA *get_area_data        args( ( int vnum ) );
void save_area( AREA_DATA *pArea );
void autoflux();
void bankrupt(OBJ_INDEX_DATA *obj);
int stockpercent(OBJ_INDEX_DATA *obj);


bool chance2		args(( int num ));

/* Command Procedures  */

DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_give		);

bool chance2(int num)
{
    if (number_range(40,140) <= num) return TRUE;
    else return FALSE;
}

//For bankrupting a stock.
void bankrupt(OBJ_INDEX_DATA *pObj)
{
	char buf[MAX_STRING_LENGTH];
	if(pObj->cost != 700)
	{
	sprintf(buf,"%s has just gone {DBANKRUPT!{x",pObj->material);
	do_announce(NULL,buf);
	pObj->cost = 700;
	pObj->condition = 0;
	//Remove old certificates from offline players
	sprintf( buf, "grep -rl 'Vnum %d' ../player/ | xargs perl -pi -e 's/Vnum %d/Vnum %d/'", 
	pObj->vnum, pObj->vnum, 47520 );//bad stock
		system( buf );
	}

	//Remove old certificates from the mud
	OBJ_DATA *obj;
	int found = 0;
	for (obj = object_list; obj != NULL; obj = obj->next)
	{
	 if (obj->pIndexData->vnum == pObj->vnum)
	 	{
	 	found = 1;
	 	extract_obj(obj);
	 	break;
 		}
 	 else
 	 	{
	 	found = 0;
	 	continue;
 		}
	}	
	if(found)
	{
	bankrupt(pObj);
	return;
	}
 return;
}

/*
 *Function called in updates that fluctuates stock prices automatically
 */
void autoflux()
{
 log_string("stock market fluctuated.");
 int percent = 0;
 int surge = 0;
 int crash = 0;
 int warning = 0;
 int vnum = 0;

 char buf[MAX_STRING_LENGTH];
 
 OBJ_INDEX_DATA *obj;
 AREA_DATA *area;//Main Port
 AREA_DATA *tarea;//Testport
 area = get_area_data(105);//Main Port
 tarea = get_area_data(28);//Testport
 
 //Calculate chance to surge:
 if (number_range(4,10) >= number_percent())
 {
	 surge = number_range(2,4);//Stocks are now surging!
 }
 /* 
  * Now a chance to crash.  This is higher because it will help
  * counteract the fact that lower prices will fall slower while
  * higher prices rise faster, which is the only flaw with the
  * system as it is, but is counteracted easily.
  */
 else if (number_range(3,8) >= number_percent())
 {
	 crash = number_range(2,4);//Stocks are now crashing!
 }
for (vnum = OBJ_VNUM_STOCK_A; vnum <= OBJ_VNUM_STOCK_E; vnum++)
{
 obj = get_obj_index(vnum);
 percent = stockpercent(obj);//Generate our percentage for this iteration
 if (surge)//Are stocks starting a surge?
 {
	 obj->weight = surge;//Set our surge timer
 }
 if (obj->weight != 0)//Are stocks surging?
 {
	 if(crash)
	 {
		 obj->weight = 0;//Crash stops a surge
		 obj->level = crash;//Set a crash timer
	 }
	 else
	 {
	 	obj->weight -= 1;//Decrement the surge
	 	percent = abs(percent);//Make the percent positive
 	 }
 }
 else if (crash)
 {
	 obj->level = crash;//Set a crash timer
 }
 if (obj->level !=0)//Are stocks crashing?
 {
 	obj->level -= 1;
 	percent = -1*abs(percent);//Make it negative
 }
 obj->cost = obj->cost*(100+percent)/100;//Update cost 
 if(obj->cost <= 100)//Check for Bankrupt stocks
 {
	obj->cost = 100;
	bankrupt(obj);//Bankrupt = true
 }
 else if(obj->cost <= 200)
 {
	warning++;
	if(number_range(1,10) <= 2)
	{
	 obj->cost += number_range(2,7);//DO NOT change this number
	}
	else if(number_range(1,30) == 13)//3.3% chance for instant bankruptcy
	{
	 bankrupt(obj);
	 warning--;
	}
 }
 obj->condition = percent;//Store the most recent % change in a useless spot
}
 save_area(area);
 save_area(tarea);
 
 do_announce(NULL,"Ding Ding Ding! The stock market has just fluctuated!");
 if(surge)
 do_announce(NULL,"The Stock Market has started {GSURGING!{x");
 if(crash)
 do_announce(NULL,"The Stock Market has started {RCRASHING!{x");
 if(warning == 1)
 {
	sprintf(buf,"Warning: 1 company is in danger of going {DBANKRUPT!{x");
 }
 else if(warning != 0)
 {
	sprintf(buf,"Warning: %i companies are in danger of going {DBANKRUPT!{x",warning);
 }
 if(warning)
 do_announce(NULL,buf);
 return;
}

/* This function is for the Aesir clan stock, it fluctuates on a set basis,
 * and has it's own surge built into it independent of the normal market 
 */

void clanflux()
{
  int surge = 0;
  int crash = 0;
  int dam = 0;
  int newcost = 0;
  int percent = 0;
  OBJ_INDEX_DATA *stockindex;
  AREA_DATA *area;
  AREA_DATA *tarea;
  area = get_area_data(105);
  tarea = get_area_data(28);
  	
  dam = number_range (1, 9); 

if (dam <= 4)
 {
         surge = number_range(95,195);
         stockindex = get_obj_index(OBJ_VNUM_STOCK_F);
       	 newcost = stockindex->cost + surge;
         percent = stockpercent(stockindex);
	  stockindex->cost = newcost;
         stockindex->weight -= 1;
         percent = abs(percent);
         stockindex->condition = percent;
         save_area(area);
	  save_area(tarea);
	
	 return; 
}
else if (dam >= 6)
 {
  	 crash = number_range(89,245);
         stockindex = get_obj_index(OBJ_VNUM_STOCK_F);
	 newcost = stockindex->cost - crash;
         percent = stockpercent(stockindex);	
         stockindex->cost = newcost;
         stockindex->level -= 1;
         percent = -1*abs(percent);
         stockindex->condition = percent;
	     save_area(area);
	     save_area(tarea); 

  return;	
 }
else if (dam = 5)
 {
         surge = number_range(299,458);
         stockindex = get_obj_index(OBJ_VNUM_STOCK_F);
         newcost = stockindex->cost + surge;
         percent = stockpercent(stockindex);
         stockindex->cost = newcost;
         stockindex->weight -= 1;
         percent = abs(percent);
         stockindex->condition = percent;
         save_area(area);
         save_area(tarea);
         do_announce(NULL, "Aesir Stock Have {GSURGED{X in PRICE!\n\r");
         return;
 }
	

return;	
}

/*
 *Function for generating a increase/drop percentage called whenever you want to autoflux a stock.
 *For stocks with different risk levels, modify this number AFTER you call the function so the general
 *chances are maintained, you just scale them.
 */
int stockpercent(OBJ_INDEX_DATA *obj)
{
	int per, neg, i;
	if ((neg = number_range(1,2)) == 2)
	{
	 neg = -1;
	}

	i = number_range(1,26);
	switch (i)
	{
	 case 1:
	 {//Big gainers or losers
	  per = neg*17;
	  break;
	 }
	 case 2:
	 {
	  per = neg*10;
	  break;
	 }
	 case 3:
	 case 4:
	 {
	  per = neg*9;
	  break;
	 }
	 case 5:
	 case 6:
	 {
	  per = neg*8;
	  break;
	 }
	 case 7:
	 case 8:
	 {
	  per = neg*7;
	  break;
	 }
	 case 9:
	 case 10:
	 {
	  per = neg*6;
	  break;
	 }
	 case 11:
	 case 12:
	 case 13:
	 {
	  per = neg*5;
	  break;
	 }
	 case 14:
	 case 15:
	 case 16:
	 {
	  per = neg*4;
	  break;
	 }
	 case 17:
	 case 18:
	 case 19:
	 case 20:
	 {
	  per = neg*3;
	  break;
	 }
	 case 21:
	 case 22:
	 case 23:
	 {
	  per = neg*2;
	  break;
	 }
	 case 24:
	 case 25:
	 case 26:
	 {
	  per = neg;
	  break;
	 }
	 default:
	 {
	  per = 0;
	  break;
	 }
	}
 if(obj->cost <= 150)//Stocks in extreme danger of bankruptcy can't fall as hard.
 {
	if(per == -15)//No mega-crashes allowed
	{
	 per = -10;
	}
	per++;//Max decline of -9%
	if(per > 15)
	{
	per = 15;
	}
 }
 return(per);
}

/* Immortal command to manually adjust stock prices by 15% either up or down,
 * reset prices back to normal, or force an autoflux and aesir flux.
 */

void do_fluct( CHAR_DATA *ch, char *argument )
 {
        char arg1[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH]; 
        OBJ_INDEX_DATA *obj; 
        int vnum;
	    AREA_DATA *area;
	    AREA_DATA *tarea;
	    area = get_area_data(105);
	    tarea = get_area_data(28);
	    
	    argument = one_argument(argument, arg1);
          
     if ( arg1[0] == '\0' )
            {
                send_to_char("Syntax: Fluct up/down/reset/now/noaesir(fluct up but not aesir)\n\r",ch);
                send_to_char("This command will raise or lower the cost of all stocks by 15 percent\n\r",ch);
                return;
            }

     if ( !str_prefix( arg1, "up" ) ) 
      {
	   for (vnum = OBJ_VNUM_STOCK_A; vnum <= OBJ_VNUM_STOCK_F; vnum++)
		{
           obj = get_obj_index(vnum);
	       obj->cost = (obj->cost*115)/100;
	       obj->condition = 15;
    	}
        save_area(area);
        save_area(tarea);
        sprintf(buf,"The Stock Market Fluctuates! All prices went {Gup{Y 15 percent!");
        do_announce(ch,buf);
        return;
      }
      if ( !str_prefix( arg1, "noaesir" ) ) 
      {
	   for (vnum = OBJ_VNUM_STOCK_A; vnum <= OBJ_VNUM_STOCK_E; vnum++)
		{
           obj = get_obj_index(vnum);
	       obj->cost = (obj->cost*115)/100;
	       obj->condition = 15;
    	}
        save_area(area);
        save_area(tarea);
        sprintf(buf,"The Stock Market Fluctuates! All prices went {Gup{Y 15 percent! Except Aesir Clan Stock.");
        do_announce(ch,buf);
        return;
      }
	 else if ( !str_prefix( arg1, "down" ) )
      {
       for (vnum = OBJ_VNUM_STOCK_A; vnum <= OBJ_VNUM_STOCK_F; vnum++)
		{
           obj = get_obj_index(vnum);
	       obj->cost = (obj->cost*85)/100;
    	obj->condition = 15;
	       }
        save_area(area);
        save_area(tarea);
        sprintf(buf,"The Stock Market Fluctuates! All prices went {Rdown{Y 15 percent!");
        do_announce(ch,buf);
        return;
      }
     else if ( !str_prefix( arg1, "reset" ) )
      {
	   for (vnum = OBJ_VNUM_STOCK_A; vnum<=OBJ_VNUM_STOCK_F; vnum++)
	    {
           obj = get_obj_index(vnum);
	       obj->cost = 700;
		   obj->condition = 0;
	    
             }
            obj = get_obj_index(OBJ_VNUM_STOCK_F);
            obj->cost = 15000;
            obj->condition = 0;
	       save_area(area);
	       save_area(tarea);  
	       sprintf(buf,"The stock prices have been {Rreset{Y to {W700{Y platinum each.");
	       do_announce(ch,buf);
	       return;
       }
      else if (!str_prefix( arg1, "now" ) )
       {
	       autoflux();
              clanflux();
	       return;
       }
      else if (!str_prefix( arg1, "aesir" ) )
    {
      obj = get_obj_index(OBJ_VNUM_STOCK_F);
      obj->cost = 15000;
      obj->condition = 0;
      send_to_char("Aesir clan stock reset\n\r",ch);
      save_area(area);
      save_area(tarea);
return;
    }
  return;
}



void do_stockmarket(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *stockman; 
    OBJ_DATA *stock;
    OBJ_INDEX_DATA *stockindex;
    char listbuf[MAX_STRING_LENGTH];
    BUFFER *list;
    char buf [MAX_STRING_LENGTH];
    char buf1 [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int new = 1;
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
  
   for ( stockman = ch->in_room->people; stockman != NULL; stockman = stockman->next_in_room )
    {
	if (!IS_NPC(stockman)) continue;
        if (stockman->spec_fun == spec_lookup( "spec_stockman" )) break;
    }

    if (stockman == NULL || stockman->spec_fun != spec_lookup( "spec_stockman" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

             if ( arg1[0] == '\0' )
            {
             send_to_char("Stock Market Commands: List Buy Sell.\n\r",ch);
                return;
            }

    if ( !strcmp( arg1, "list" ) )
	{
	int vnum = 0;
	char name[MAX_STRING_LENGTH];
	list = new_buf();
	sprintf(listbuf,"    {G===============({RStock Market{G)==============={x\n\r");
	add_buf(list,listbuf);
	for(vnum=OBJ_VNUM_STOCK_A;vnum <= OBJ_VNUM_STOCK_E; vnum++)
	{
	 sprintf(name,get_obj_index(vnum)->material);//Get the name of the stock
       if(get_obj_index(vnum)->cost != 0)
	 {
 	    if(get_obj_index(vnum)->condition < 0 && get_obj_index(vnum)->condition > -10)//If it's negative and single digit
	     {
		sprintf(listbuf,"    {W%i{D Plat   {W( {R%i%%{W){D:{C  %s  {D*\n\r",get_obj_index(vnum)->cost,get_obj_index(vnum)->condition,name);
		add_buf(list,listbuf);
	     }
	    else if(get_obj_index(vnum)->condition <= -10)//Double digit negative
	     {
	 	sprintf(listbuf,"    {W%i{D Plat   {W({R%i%%{W){D:{C  %s   {D*\n\r",get_obj_index(vnum)->cost,get_obj_index(vnum)->condition,name);
		add_buf(list,listbuf);
                new = 1;
             }
	    else if(get_obj_index(vnum)->condition >= 0 && get_obj_index(vnum)->condition < 10)//Single digit positive or zero
	     {
		sprintf(listbuf,"    {W%i{D Plat   {W(  {G%i%%{W){D:{C  %s {D*\n\r",get_obj_index(vnum)->cost,get_obj_index(vnum)->condition,name);
                add_buf(list,listbuf);
                new = 2;
	     }
           else//Double digit positive
	     {
	 	sprintf(listbuf,"    {W%i{D Plat   {W( {G%i%%{W){D:{C  %s  {D*\n\r",get_obj_index(vnum)->cost,get_obj_index(vnum)->condition,name);
		add_buf(list,listbuf);
             }

	 }
	 else//Everything else (should never show up if autobankrupt is working properly)
	 {
		sprintf(listbuf,"    {DBANKRUPT       :{R  %s {D*\n\r",name);
		add_buf(list,listbuf);
	 }
	}
	

page_to_char(buf_string(list),ch);
if (new = 2)
{
sprintf(buf2,"    {y============({YImmortal {wClan Stock{y)==========={x\n\r");
sprintf(buf1,"    {W%i{D Plat   {W({Y%i%%{W){D:{C  {GA{Ye{Ws{Gi{Yr{X Clan Stock   {D*\n\r",get_obj_index(OBJ_VNUM_STOCK_F)->cost,get_obj_index(OBJ_VNUM_STOCK_F)->condition);   
}
else if (new = 1)
{
sprintf(buf2,"    {y============({YImmortal {wClan Stock{y)==========={x\n\r");
sprintf(buf1,"    {W%i{D Plat   {W({R-%i%%{W){D:{C  {GA{Ye{Ws{Gi{Yr{X Clan Stock   {D*\n\r",get_obj_index(OBJ_VNUM_STOCK_F)->cost,get_obj_index(OBJ_VNUM_STOCK_F)->condition);
}


send_to_char(buf2,ch);
send_to_char(buf1,ch); 
return;
}
    
    else if ( !strcmp( arg1, "buy" ) )
	{
	    if ( arg2[0] == '\0' )
	    {
		send_to_char( "Stock buy <Stock> (Bought In 5 Bulk Shares)\n\r", ch );
		return;
	    }

	    if ( !str_prefix(arg2, "bree inc.") )
        {
		stockindex = get_obj_index(OBJ_VNUM_STOCK_A);
		if (stockindex->cost == 0)
		{
			send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
			return;
		}
           if (ch->platinum >= stockindex->cost)
            {
                ch->platinum -= stockindex->cost;
                
               stock = create_object( get_obj_index(OBJ_VNUM_STOCK_A),ch->level);  
               obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );//Save their pfile
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough platinum for that.",ch->name);
                do_say(stockman,buf);
                return;
            }
        }
	    else if (!str_prefix(arg2, "fesdor unlimited"))
        {
		stockindex = get_obj_index(OBJ_VNUM_STOCK_B);
		if (stockindex->cost == 0)
		{
			send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
			return;
		}
        if (ch->platinum >= stockindex->cost)
            {

                ch->platinum -= stockindex->cost;
               
             stock = create_object( get_obj_index(OBJ_VNUM_STOCK_B),ch->level); 
             obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough platinum for that.",ch->name);
                do_say(stockman,buf);
                return;
            }
	     return;
	   }
	     else if (!str_prefix(arg2, "distorted illusions of grandeur"))
        {
		stockindex = get_obj_index(OBJ_VNUM_STOCK_C);
		if (stockindex->cost == 0)
		{
			send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
			return;
		}
         if (ch->platinum >= stockindex->cost)
            {
                ch->platinum -= stockindex->cost;
               
                stock = create_object( get_obj_index(OBJ_VNUM_STOCK_C),ch->level); 
                obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough platinum for that.",ch->name);
                do_say(stockman,buf);
                return;
            }
	     return;
	    }
	     else if (!str_prefix(arg2, "asgardian pharmaceuticals"))
        {
		stockindex = get_obj_index(OBJ_VNUM_STOCK_D);
		if (stockindex->cost == 0)
		{
			send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
			return;
		}
         if (ch->platinum >= stockindex->cost)
            {

                ch->platinum -= stockindex->cost;
               
                stock = create_object( get_obj_index(OBJ_VNUM_STOCK_D),ch->level); 
                obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough platinum for that.",ch->name);
                do_say(stockman,buf);
                return;
            }
	     return;
	    }
	     else if (!str_prefix(arg2, "bobs babble bonanza"))
        {
		stockindex = get_obj_index(OBJ_VNUM_STOCK_E);
		if (stockindex->cost == 0)
		{
			send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
			return;
		}
         if (ch->platinum >= stockindex->cost)
            {
                ch->platinum -= stockindex->cost;
               
                stock = create_object(get_obj_index(OBJ_VNUM_STOCK_E),ch->level);
                obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough platinum for that.",ch->name);
                do_say(stockman,buf);
                return;
            }
	     return;
	    }
else if (!str_prefix(arg2, "Aesir Clan Stock"))
        {
                stockindex = get_obj_index(OBJ_VNUM_STOCK_F);
                if (stockindex->cost == 0)
                {
                        send_to_char("You cannot buy stock in a bankrupt company!\n\r",ch);
                        return;
                }
         if (ch->platinum >= stockindex->cost)
            {
                ch->platinum -= stockindex->cost;
                
                stock = create_object(get_obj_index(OBJ_VNUM_STOCK_F),ch->level);
                obj_to_char(stock,ch);

                act( "$N gives $p to $n.", ch, stock, stockman, TO_ROOM );
                act( "$N gives you $p.",   ch, stock, stockman, TO_CHAR );
                save_char_obj( ch );
                return;
            }
}
	    /* These are the Clan Stocks Which will not be subject to the randomness of the normal market
	     * They will be based on clan account balance, number of members, and age. and a small random
	     * multiplier or divider.
	     */
	     else if (!strcmp(arg2, "conquest"))
        {
         
	    return;
        }
             else if (!strcmp(arg2, "kyuubi"))
        {
        
        return;
        }   
          else if (!strcmp(arg2, "enforcer"))
        {
         
	    return;
        } 
            else if (!strcmp(arg2, "audentia"))
        {
         
        return;
        }

      }
else if ( !strcmp( arg1, "sell" ) )
	{
	    if ( arg2[0] == '\0' )
	    {
		send_to_char( "Stock sell <Stock> (Sold In 5 Bulk Shares)\n\r", ch );
		return;
	    }
	    if ( !str_prefix(arg2, "bree inc.") )
        {
	      if ((stock = get_obj_carry (ch, arg2)) == NULL)
		   {
		  	send_to_char( "You do not have that item\n\r", ch );
			return;
		   }	     
		   stockindex = get_obj_index(OBJ_VNUM_STOCK_A);
		   ch->platinum += stockindex->cost;
	       extract_obj (stock);
	       sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );//Save pfile    
	       return;
	    }
        else if ( !str_prefix(arg2, "fesdor unlimited") )
        {
	      if ((stock = get_obj_carry (ch, arg2)) == NULL)
		   {
		  	send_to_char( "You do not have that item\n\r", ch );
			return;
		   }	     
		   stockindex = get_obj_index(OBJ_VNUM_STOCK_B);
		   ch->platinum += stockindex->cost;
	       extract_obj (stock);
	       sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );    
	       return;
	    }
       else if ( !str_prefix(arg2, "distorted illusions of grandeur") )
        {
	      if ((stock = get_obj_carry (ch, arg2)) == NULL)
		   {
		  	send_to_char( "You do not have that item\n\r", ch );
			return;
		   }	     
		   stockindex = get_obj_index(OBJ_VNUM_STOCK_C);
		   ch->platinum += stockindex->cost;
	       extract_obj (stock);
	       sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );    
	       return;
	    }
	       else if ( !str_prefix(arg2, "asgardian pharmaceuticals") )
        {
	      if ((stock = get_obj_carry (ch, arg2)) == NULL)
		   {
		  	send_to_char( "You do not have that item\n\r", ch );
			return;
		   }	     
		   stockindex = get_obj_index(OBJ_VNUM_STOCK_D);
		   ch->platinum += stockindex->cost;
	       extract_obj (stock);
	       sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );    
	       return;
	    }
	    else if ( !str_prefix(arg2, "bobs babble bonanza") )
        {
	      if ((stock = get_obj_carry (ch, arg2)) == NULL)
		   {
		  	send_to_char( "You do not have that item\n\r", ch );
			return;
		   }	     
		   stockindex = get_obj_index(OBJ_VNUM_STOCK_E);
		   ch->platinum += stockindex->cost;
	       extract_obj (stock);
	       sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );    
	       return;
	    }
else if ( !str_prefix(arg2, "Aesir Clan Stock") )
        {
              if ((stock = get_obj_carry (ch, arg2)) == NULL)
                   {
                        send_to_char( "You do not have that item\n\r", ch );
                        return;
                   }
                   stockindex = get_obj_index(OBJ_VNUM_STOCK_F);
                   ch->platinum += stockindex->cost;
               extract_obj (stock);
               sprintf(buf, "Thank you %s!",ch->name);
           do_say(stockman,buf);
           save_char_obj( ch );
               return;
            }
   
     }

}

