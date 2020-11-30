/*********************************************************************
*                                                                    *
*    000000This file property of Kefka (kefka@netcom.ca) and               *
*                          Skyntil (gvolker98@hotmail.com)           *
*      Code file of Asgardian Nightmare (RoT 1.5/Asgard 0.1b)        *
*      telnet://asgard.betterbox.net:5555                            *
*      http://asgard.betterbox.net                                   *
*********************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
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
#include "interp.h"

void    initialize_spell_types args((void));
int focus_level args ((long total));
long focus_left args ((long total));

int focus_dam args ((CHAR_DATA *ch));
int focus_str args ((CHAR_DATA *ch));
int focus_ac args ((CHAR_DATA *ch));
int focus_move args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch));
int focus_dex args ((CHAR_DATA *ch));
int focus_hp args ((CHAR_DATA *ch));
int focus_con args ((CHAR_DATA *ch));
int focus_sorc args ((CHAR_DATA *ch));
int focus_sorcbonus args ((CHAR_DATA *ch));
int focus_mana args ((CHAR_DATA *ch));
int focus_int args ((CHAR_DATA *ch));
int focus_save args ((CHAR_DATA *ch));
int focus_wis args ((CHAR_DATA *ch));

DECLARE_DO_FUN( do_decapitate );
void raw_kill args ((CHAR_DATA * victim, CHAR_DATA * killer));

void 
do_pretitle (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int value;

	if (IS_SET (ch->act, PLR_NOTITLE))
		return;

	if (IS_NPC (ch))
	{
		send_to_char ("Not on NPC's.\n\r", ch);
		return;
	}


	if ((ch->in_room->vnum == ROOM_VNUM_CORNER)
		&& (!IS_IMMORTAL (ch)))
	{
		send_to_char ("Just keep your nose in the corner like a good little player.\n\r", ch);
		return;
	}

	if (ch->pcdata->pretit == '\0')
		ch->pcdata->pretit = "{x";

	if (argument[0] == '\0')
	{
		sprintf (buf, "Your current pretitle is '%s'.{x\n\r", ch->pcdata->pretit);
		send_to_char (buf, ch);
		return;
	}


	if (strlen (argument) > 45)
	{
		argument[45] = '{';
		argument[46] = 'x';
		argument[47] = '\0';
	}
	else
	{
		value = strlen (argument);
		argument[value] = '{';
		argument[value + 1] = 'x';
		argument[value + 2] = '\0';
	}


	ch->pcdata->pretit = str_dup (argument);
	send_to_char ("Done.\n\r", ch);
	return;

}

void do_stances(CHAR_DATA *ch, char *argument)
{
 char buf[MSL];

 send_to_char("\n\r{b========={x{ySTANCES{x{b========={x\n\r",ch);
 sprintf(buf,"{b|>{x {gBull{x    : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_BULL]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gViper{x   : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_VIPER]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gCrab{x    : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_CRAB]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gCrane{x   : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_CRANE]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMongoose{x: {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_MONGOOSE]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gTiger{x   : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_TIGER]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMantis{x  : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_MANTIS]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gDragon{x  : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_DRAGON]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMonkey{x  : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_MONKEY]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gSwallow{x : {w%-3d{x/200   {b<|{x\n\r",ch->stance[STANCE_SWALLOW]);
 send_to_char(buf,ch);
 send_to_char("{b========={x{y*******{x{b========={x\n\r",ch);

 return;
}

void do_power(CHAR_DATA *ch, char *argument)
{
 char buf[MSL];
 extern bool clantoggle;

 if(IS_IMMORTAL(ch))
 {
  if(!str_cmp(argument,"toggle"))
  {
   if(clantoggle)
   clantoggle = FALSE;
   else if(!clantoggle)
   clantoggle = TRUE;

   printf_to_char(ch,"Clantoggle is now %d.\n\r",clantoggle);
   return;
  }
  if(!str_cmp(argument,"reset"))
  {
   ch->pcdata->power[0] = 0;
   ch->pcdata->power[1] = 0;
   send_to_char("power reset.\n\r",ch);
  }
  if(!str_cmp(argument,"nomonk"))
  {
   NOMONKS = TRUE;
   printf_to_char(ch,"NOMONK is now %d.\n\r",NOMONKS);
  }
  if(!str_cmp(argument,"yesmonk"))
  {
   NOMONKS = FALSE;
   printf_to_char(ch,"NOMONK is now %d.\n\r",NOMONKS);
  }
  if(!str_cmp(argument,"noninja"))
  {
   printf_to_char(ch,"NONINJA is now %d.\n\r",NONINJA);
   NONINJA = TRUE;
  }
  if(!str_cmp(argument,"yesninja"))
  {
   NONINJA = FALSE;
   printf_to_char(ch,"NONINJA is now %d.\n\r",NONINJA);
  }
 }

 if(ch->class != CLASS_HIGHLANDER && ch->class != CLASS_MONK)
 {
  send_to_char("You pray for power....the gods laugh at your display.\n\r",ch);
  return;
 }

 if(ch->class == CLASS_HIGHLANDER) {
 send_to_char("\n\r{b======{x{cHIGHLAND-POWER{x{b======{x\n\r",ch);
/* sprintf(buf,"{b|>{x {gKills{x    : {r%-3d{x       {b<|{x\n\r",ch->pcdata->power[POWER_KILLS]);
 send_to_char(buf,ch); */
 sprintf(buf,"{b|>{x {gPoints{x   : {r%-3d{x       {b<|{x\n\r",ch->pcdata->power[POWER_POINTS]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gLevel{x    : {r%-3d{x       {b<|{x\n\r",ch->pcdata->power[POWER_LEVEL]);
 send_to_char(buf,ch);
 send_to_char("{b========={x{c********{x{b========={x\n\r",ch);
 }
 else {
 send_to_char("\n\r{b======{x{c   CHI-POWER  {x{b======{x\n\r",ch);
 sprintf(buf,"{b|>{x {gCombat{x    : {r%-3d{x      {b<|{x\n\r",ch->pcdata->power[0]);
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gDefense{x   : {r%-3d{x      {b<|{x\n\r",ch->pcdata->power[1]);
 send_to_char(buf,ch);
 send_to_char("{b========={x{c********{x{b========={x\n\r",ch);
 }
 return;
}

void do_focus(CHAR_DATA *ch, char *argument)
{
 char buf[MSL];

 if(ch->level < LEVEL_HERO)
 {
  send_to_char("You just aren't legendary enough to focus.\n\r",ch);
  return;
 }
 else
 {
 send_to_char("\n\r{b-----------======================{x{cFOCUS{x{b======================-----------{x\n\r",ch);
 send_to_char("{b|>{x                   {RLvl  Totl Pnts  Pnts Left  Modifier1  Modifier2 {b<|{x\n\r", ch);
 send_to_char("{b|>{x                   {r---  ---------  ---------  ---------  --------- {b<|{x\n\r", ch);
 sprintf(buf,"{b|>{x {gCombat Power{x     : {r%-2d  %-9ld  %-9ld  Dam: %4d  Str: %4d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[COMBAT_POWER]),
	ch->pcdata->focus[COMBAT_POWER],
	focus_left(ch->pcdata->focus[COMBAT_POWER]),
	focus_dam(ch), focus_str(ch) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gCombat Defense{x   : {r%-2d  %-9ld  %-9ld  AC: %5d  Mv: %5d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[COMBAT_DEFENSE]),
	ch->pcdata->focus[COMBAT_DEFENSE],
	focus_left(ch->pcdata->focus[COMBAT_DEFENSE]),
        focus_ac(ch), focus_move(ch)*focus_level(ch->pcdata->focus[COMBAT_DEFENSE]) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gCombat Agility{x   : {r%-2d  %-9ld  %-9ld  Hit: %4d  Dex: %4d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[COMBAT_AGILITY]),
	ch->pcdata->focus[COMBAT_AGILITY],
	focus_left(ch->pcdata->focus[COMBAT_AGILITY]),
	focus_hit(ch), focus_dex(ch) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gCombat Toughness{x : {r%-2d  %-9ld  %-9ld  Hp: %5d  Con: %4d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[COMBAT_TOUGHNESS]),
	ch->pcdata->focus[COMBAT_TOUGHNESS],
	focus_left(ch->pcdata->focus[COMBAT_TOUGHNESS]),
        focus_hp(ch)*focus_level(ch->pcdata->focus[COMBAT_TOUGHNESS]), focus_con(ch) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMagic Power{x      : {r%-2d  %-9ld  %-9ld  Sor: %4d  Max: +%3d%%{b<|{x\n\r",
	focus_level(ch->pcdata->focus[MAGIC_POWER]),
	ch->pcdata->focus[MAGIC_POWER],
	focus_left(ch->pcdata->focus[MAGIC_POWER]),
	focus_sorc(ch),focus_sorcbonus(ch) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMagic Ability{x    : {r%-2d  %-9ld  %-9ld  Ma: %5d  Int: %4d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[MAGIC_ABILITY]),
	ch->pcdata->focus[MAGIC_ABILITY],
	focus_left(ch->pcdata->focus[MAGIC_ABILITY]),
        focus_mana(ch)*focus_level(ch->pcdata->focus[MAGIC_ABILITY]), focus_int(ch) );
 send_to_char(buf,ch);
 sprintf(buf,"{b|>{x {gMagic Defense{x    : {r%-2d  %-9ld  %-9ld  Svs: %4d  Wis: %4d {b<|{x\n\r",
	focus_level(ch->pcdata->focus[MAGIC_DEFENSE]),
	ch->pcdata->focus[MAGIC_DEFENSE],
	focus_left(ch->pcdata->focus[MAGIC_DEFENSE]),
	focus_save(ch), focus_wis(ch) );
 send_to_char(buf,ch);
 send_to_char("{b-----------===================={x{c********{x{b=====================-----------{x\n\r",ch);
 
 if (ch->pcdata->focus[CURRENT_FOCUS] == COMBAT_POWER)
	send_to_char("You current focus is Combat Power.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == COMBAT_DEFENSE)
	send_to_char("You current focus is Combat Defense.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == COMBAT_AGILITY)
	send_to_char("You current focus is Combat Agility.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == COMBAT_TOUGHNESS)
	send_to_char("You current focus is Combat Toughness.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == MAGIC_POWER)
	send_to_char("You current focus is Magic Power.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == MAGIC_ABILITY)
	send_to_char("You current focus is Magic Ability.\n\r",ch);
 else if (ch->pcdata->focus[CURRENT_FOCUS] == MAGIC_DEFENSE)
	send_to_char("You current focus is Magic Defense.\n\r",ch);

 }
 return;
}

void do_rubeyes(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int burnt = 0;
    AFFECT_DATA af, *paf, *fire;
    bool found = 0;

    one_argument( argument, arg );

    if (IS_NPC( ch ))
    return;

    if(ch->fighting != NULL)
    {
      send_to_char("Not while fighting!\n\r",ch);
      return;
    }

    if(!IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char( "But your eyes aren't blinded!\n\r", ch );
	return;
    }
    
    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
        switch( paf->location )
        {
              case APPLY_CRIPPLE_HEAD:
              found = 1;
              break;
        }
        if(found)
        {
          send_to_char("You can't rub out the crippling effects!\n\r",ch);
          return; 
        }
    }

    if ( number_percent() > 60 )
    {
        act( "You rub your eyes till they bleed but remain blind.", ch, NULL, NULL,TO_CHAR);
        act( "$n rubs $s eyes violently.",  ch, NULL, NULL, TO_ROOM );
        WAIT_STATE(ch,4);
	return;
    }
   
    if ( is_affected(ch,skill_lookup("fire breath")))
    {
       int fchance = 0;

       fchance = number_percent();

       if ( fchance < 5)
       {
	  act("You start to rub your eyes but fail.",ch,NULL,NULL,TO_CHAR);
	  act("You double over as the smoke in your lungs sends you into a coughing fit!",ch,NULL,NULL,TO_CHAR);
	  act("$n suddenly doubles over and begins to cough uncontrollably!",ch,NULL,NULL,TO_ROOM);
	  WAIT_STATE(ch,12);
	  act("Your coughing fit subsides.",ch,NULL,NULL,TO_CHAR);
	  return;
       }
	    
       if ( fchance < 10)
       {
          act("You try to rub the smoke out of your eyes but fail.",ch,NULL,NULL,TO_CHAR);
          act("$n rubs $s eyes violently.",ch,NULL,NULL,TO_ROOM);
          WAIT_STATE(ch,3);
          return;
       }

       for ( fire = ch->affected; fire != NULL; fire = fire->next)
       {
           if ( fire->type == skill_lookup("fire breath"))
           {
              burnt = fire->duration;
              break;
           }
       }

       if ( burnt > 1 && fchance < 50)
       {
          affect_strip(ch,skill_lookup("fire breath"));   

          af.where     = TO_AFFECTS;
          af.location  = APPLY_HITROLL;
          af.modifier  = -25;
          af.level     = fire->level;
          af.bitvector = AFF_BLIND;
          af.type      = skill_lookup("fire breath");
          af.duration  = 1;

          affect_to_char(ch,&af);

          WAIT_STATE( ch, 6 );

          act("You manage to rub some smoke out of your eyes.",ch,NULL,NULL,TO_CHAR);
          act("Smoke flows out from $n's eyes.",ch,NULL,NULL,TO_ROOM);
	  return;
       }
       else if ( burnt > 0 && fchance < 75 )
       {
            affect_strip(ch,skill_lookup("fire breath"));

            af.where     = TO_AFFECTS;
            af.location  = APPLY_HITROLL;
            af.duration  = 0;
            af.type      = skill_lookup("fire breath");
            af.level     = 101;
            af.bitvector = AFF_BLIND;
            af.modifier  = -25;

            affect_to_char(ch,&af);

            if (ch->move > 1)
               ch->move = ch->move*9/10;

            act("Your eyes begin to clear, the smoke dissipating",ch,NULL,NULL,TO_CHAR);
            act("$n's eyes begin to water.",ch,NULL,NULL,TO_ROOM);

            WAIT_STATE(ch,4);
	    return;
       }	     
    }

    if ( is_affected(ch,skill_lookup("blindness")))
    {
     //act("Try as you might, you the magic will not rub away!",ch,NULL,NULL,TO_CHAR);	
     //WAIT_STATE(ch,2);
     //return;
     	
     if(number_percent() < 50)
     {
      act("You try to rub the darkness from your eyes but fail.",ch,NULL,NULL,TO_CHAR);
      act("$n rubs $s eyes violently.",ch,NULL,NULL,TO_ROOM);
     }
     for(fire = ch->affected; fire != NULL; fire = fire->next)
     {
      if(fire->type == skill_lookup("blindness"))
      {
       burnt = fire->duration;
       break;
      }
     }
     if(burnt > 1)
     {
     affect_strip(ch,skill_lookup("blindness"));   
     af.where = TO_AFFECTS;
     af.location = APPLY_HITROLL;
     af.modifier = -25;
     af.level = fire->level;
     af.bitvector = AFF_BLIND;
     af.type = skill_lookup("blindness");
     af.duration = (burnt-2);
     affect_to_char(ch,&af);
     WAIT_STATE(ch, 36);
      act("You manage to rub some of the darkness from your eyes.",ch,NULL,NULL,TO_CHAR);
      act("Darkness flows out from $n's eyes.",ch,NULL,NULL,TO_ROOM);
	return;
     }
     else if (burnt > 0)
     {
      affect_strip(ch,skill_lookup("blindness"));
      af.where=TO_AFFECTS;
      af.location = APPLY_HITROLL;
      af.duration = 0;
      af.type = skill_lookup("blindness");
      af.level = 101;
      af.bitvector = AFF_BLIND;
      af.modifier = -25;
      affect_to_char(ch,&af);
      if(ch->move > 1)
       ch->move = ch->move/2;
        act("Your eyes begin to clear, the darkness dissapating",ch,NULL,NULL,TO_CHAR);
	act("$n's eyes begin to water.",ch,NULL,NULL,TO_ROOM);
	WAIT_STATE(ch,4);
	return;
     }	     
    }

    
    act( "You can see again!!",  ch, NULL, ch, TO_CHAR );
    act( "$n looks around with googly eyes!", ch, NULL, ch, TO_ROOM );
    affect_strip(ch,gsn_dirt);
    affect_strip(ch,gsn_blindness_dust);
    affect_strip(ch,gsn_solar_flare);
    affect_strip(ch,skill_lookup("blindness"));
    affect_strip(ch,skill_lookup("smokebomb"));
    affect_strip(ch,skill_lookup("solar"));
    affect_strip(ch,skill_lookup("fire breath"));
    if(IS_SET(ch->affected_by,AFF_BLIND))
    	REMOVE_BIT(ch->affected_by,AFF_BLIND);
    
    WAIT_STATE(ch,4);
    return;
}
void do_smother(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC( ch ))
    return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Smother who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You cannot smother yourself.\n\r", ch );
	return;
    }

    if (!is_affected(victim, gsn_on_fire))
    {
	send_to_char( "But they are not on fire!\n\r", ch );
	return;
    }

    if ( number_percent() > 40 && number_bits(2) != 0)
    {
	act( "You try to smother the flames around $N but fail!",  ch, NULL, victim, TO_CHAR    );
	act( "$n tries to smother the flames around you but fails!", ch, NULL, victim, TO_VICT    );
	act( "$n tries to smother the flames around $N but fails!",  ch, NULL, victim, TO_NOTVICT );
	return;
    }

    act( "You manage to smother the flames around $M!",  ch, NULL, victim, TO_CHAR    );
    act( "$n manages to smother the flames around you!", ch, NULL, victim, TO_VICT    );
    act( "$n manages to smother the flames around $N!",  ch, NULL, victim, TO_NOTVICT );
    affect_strip(victim,gsn_on_fire);
    return;
}

void do_rolldirt(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC( ch ))
    return;

    if (!is_affected(ch, gsn_on_fire))
    {
        send_to_char( "But you are not on fire!\n\r", ch );
	return;
    }

    if ( number_percent() > 40 && number_bits(2) != 0)
    {
        act( "You roll in the dirt but cannot suppress the flames surrounding you.", ch, NULL, NULL,TO_CHAR);
        act( "$n rolls in the dirt trying to smother the flames around $m!",  ch, NULL, NULL, TO_ROOM );
	return;
    }

    act( "The flames around you vanish in the dirt!",  ch, NULL, ch, TO_CHAR );
    act( "$n manages to put out $s flames!", ch, NULL, ch, TO_ROOM );
    affect_strip(ch,gsn_on_fire);
    return;
}

/*
 * This code is written for Asgardian NightMare
 * by Skyntil(Gabe Volker) 4/20/00
 * Can be expanded to write a whole file if so desired.
 *
 * This Function written to write the magtype.c file from AN values.
 * Basically Declares each spell to be of type MAGIC_NONE
 * Then is to be edited with correct types
 *
 */
void write_spell_file( )
{
 FILE *fp;
 char buf[MAX_STRING_LENGTH/4];
 int i;

 //Open file

 sprintf(buf,"%s","magtype.c");
 if((fp = fopen(buf,"w")) == NULL)
 {
  perror(buf);
  exit(1);
 }

 // Write Credits
 fprintf(fp,"/*\n * Initialize Spell Types -- Do not forget to add new spells to here.\n");
 fprintf(fp," * This File Can be Generated on Bootup If so Desired.\n");
 fprintf(fp," * It could be also done online. This would avoid having to add\n");
 fprintf(fp," * new spells manually.\n");
 fprintf(fp," */\n");

 // Write Includes
 fprintf(fp,"#include \"include.h\"\n");

 // Write Declarations
 fprintf(fp,"#define SPELL_TYPE(name,type) magic_table[skill_lookup(name)] = type;\n");

 // Write Function
 fprintf(fp,"void initialize_spell_types( )\n");
 fprintf(fp,"{\n");
 fprintf(fp,"log_string( \"Initializing Magic Types.\" );");

 // Now the spells
 for(i = 0; i < MAX_SKILL ; i++)
 {
  if(skill_table[i].spell_fun != spell_null)
   fprintf(fp,"SPELL_TYPE( \"%s\", MAGIC_NONE );\n",skill_table[i].name);
 }

 // Close it off
 fprintf(fp,"\nreturn;\n}\n");

 fclose(fp);

 // Close file
 return;
}

void do_learned(CHAR_DATA *ch, char *argument)
{
 char buf[MSL];

 send_to_char("\n\r{b======={x{yLEARNED MAGIC{x{b======={x\n\r",ch);
      sprintf(buf,"{b|>{x {gWhite{x  : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_WHITE]);
 send_to_char(buf,ch);
      sprintf(buf,"{b|>{x {gRed{x    : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_RED]);
 send_to_char(buf,ch);
      sprintf(buf,"{b|>{x {gBlue{x   : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_BLUE]);
 send_to_char(buf,ch);
      sprintf(buf,"{b|>{x {gGreen{x  : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_GREEN]);
 send_to_char(buf,ch);
      sprintf(buf,"{b|>{x {gBlack{x  : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_BLACK]);
 send_to_char(buf,ch);
      sprintf(buf,"{b|>{x {gOrange{x : {w%-3d{x/200      {b<|{x\n\r",ch->magic[MAGIC_ORANGE]);
 send_to_char(buf,ch);
     send_to_char("{b========{x{y***********{x{b========{x\n\r",ch);

 return;
}

void do_combat(CHAR_DATA *ch, char *argument)
{
 char buf[MSL];
 float damred;

 if(!is_warrior_class(ch->class))
 {
  send_to_char("You do not have any special combat abilities.\n\r",ch);
  return;
 }

 damred = ((((float)ch->toughness/5)/34)/10);

 send_to_char("\n\r{b========={x{yCOMBAT{x{b=========={x\n\r",ch);
 sprintf(buf,"{b|>{x {gToughness{x : {w%-3d{x/500 {b<|{x\n\r",ch->toughness);
 send_to_char(buf,ch);
 sprintf(buf,"{b|> {gDam Reduce{& : {w%.3f{&%s  {b<|\n\r{x", damred*100, "%");
 send_to_char(buf,ch);
 send_to_char("{b========={y*******{b========={x\n\r",ch);

 if(!str_cmp(argument,"all"))
  do_stances(ch,"");
 return;
}

void do_decapitate(CHAR_DATA *ch, char *argument)
{
  char arg[MSL]; 
  CHAR_DATA *victim;
 
  one_argument( argument, arg ); 
 
  if ( arg[0] == '\0' ) 
  { 
        send_to_char( "Decapitate whom?\n\r", ch ); 
        return; 
  } 
 
  if( IS_NPC(ch))
  return;

  if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
  return; 

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  { 
        send_to_char("They aren't here to be decapitated.\n\r",ch);
        return; 
  } 
 
  if ( victim == ch ) 
  { 
     send_to_char( "You can't do that.\n\r", ch ); 
     return; 
  }

  if(ch->fighting != NULL)
  {
   send_to_char("Finish combat before decapitating!\n\r",ch);
   return;
  }

 if(victim->position > POS_INCAP)
 {
  send_to_char("Your victim needs to be mortally wounded for you to kill them.\n\r",ch);
  return;
 }

 if(!IS_SET(ch->plyr,PLAYER_SUBDUE))
 {
  send_to_char("You must be Subduing to decapitate your victim.\n\r",ch);
  return;
 }

  act("$n Decapitates $N bruttaly!! A fountain of {!GORE{0 flows out of $N ",ch,NULL,victim,TO_NOTVICT);
  act("$n Decapitates you! You feel numb.",ch,NULL,victim,TO_VICT);
  act("You Decapitate $N bruttaly!! You Are Covered In {!GORE{0!",ch,NULL,victim,TO_CHAR);
  REMOVE_BIT(ch->plyr,PLAYER_SUBDUE);
  victim->position = POS_DEAD;
  act( "$n is DEAD!!", victim, 0, 0, TO_ROOM ); 
  send_to_char( "You Gaze At You're Corpse From  Distance As All Goes Dim!!\n\r\n\r", victim ); 

 
    /* 
     * Payoff for killing things. 
     */ 
    if ( victim->position == POS_DEAD ) 
     raw_kill(victim,ch); 

 SET_BIT(ch->plyr,PLAYER_SUBDUE);

 sprintf( log_buf, "%s was DECAPITATED by %s at %s [room %d]", 
        victim->name,ch->name,ch->in_room->name,ch->in_room->vnum); 
 wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0); 

 tail_chain();
 return;
}
