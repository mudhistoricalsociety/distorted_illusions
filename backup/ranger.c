#include "include.h"

bool check_dispel args ( ( int dis_level, CHAR_DATA *victim, int sn) );
bool check_tumble args ((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void check_ground_stomp args((CHAR_DATA *ch,CHAR_DATA *victim,int chance,int dam));
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_monk_nerve );
int focus_ac args ((CHAR_DATA *ch));
int focus_hit args ((CHAR_DATA *ch));
int focus_dam args ((CHAR_DATA *ch));
void improve_toughness args((CHAR_DATA * ch));

void do_chreset(CHAR_DATA *ch, char *argument)
{
 send_to_char("Character Reset.\n\r",ch);
 reset_char(ch);
 return;
}

void do_herb(CHAR_DATA *ch, char *argument)
{
 char arg[100];
 CHAR_DATA *victim;
 AFFECT_DATA af;

 argument = one_argument(argument,arg);

 if (get_skill (ch, gsn_herb) == 0 ||
      (ch->level < skill_table[gsn_herb].skill_level[ch->class]))
 {
  send_to_char("You find some lovely herbs in the bushes.\n\r",ch);
  return;
 }


/*   if ((ch->in_room->sector_type != SECT_FOREST)
   && (ch->in_room->sector_type != SECT_HILLS) 
   && (ch->in_room->sector_type != SECT_MOUNTAIN) ) */

   if ((ch->in_room->sector_type == SECT_CITY)
   || (ch->in_room->sector_type == SECT_INSIDE))
   { 
     send_to_char("You must be in a wilderness environment to find herbs.\n\r",ch);
     return; 
    } 

   if (arg[0] == '\0') 
      victim = ch;
   else   if ((victim = get_char_room(ch,arg)) == NULL)
   { 
      send_to_char("They aren't here to be healed.\n\r",ch);
      return; 
   } 

   if (number_percent() > ch->pcdata->learned[gsn_herb]) 
   { 
       act("You search around the area but cannot find any herbs.",ch,NULL,NULL,TO_CHAR);
       act("$n looks in the bushes intently.",ch,NULL,NULL,TO_ROOM);
       check_improve(ch,gsn_herb,FALSE,2); 
       return; 
   }

   if(number_bits(2) == 0)
   {
    act("You search around the area but cannot find any herbs.",ch,NULL,NULL,TO_CHAR);
    act("$n looks in the bushes intently.",ch,NULL,NULL,TO_ROOM);
    check_improve(ch,gsn_herb,FALSE,2); 
    return;
   }

   if(is_affected(victim, gsn_herb))
   {
    if(victim == ch)
     send_to_char("You already have herbs applied to your wounds.\n\r",ch);
    else
     send_to_char("They already have herbs on their wounds.\n\r",ch);
    return;
   }

 
   if (victim != ch) 
   { 
    act("You find some herbs and apply them to $N's wounds.",ch,NULL,victim,TO_CHAR);
    act("$n searches in a few nearby bushes and emerges with some herbs.",ch,NULL,NULL,TO_ROOM);
    act("$n applies the herbs to your wounds.",ch,NULL,victim,TO_VICT);
   } 
 
   if (victim == ch) 
   { 
    act("$n searches in a few nearby bushes and emerges with some herbs.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You find some herbs and apply them to your wounds.\n\r",ch);
   } 

   if (IS_AFFECTED(victim,AFF_PLAGUE) && number_percent() > 30) 
   { 
     affect_strip(victim,gsn_plague); 
     act("The sores on $n's body vanish.\n\r",victim,0,0,TO_ROOM); 
     send_to_char("The sores on your body vanish.\n\r",victim); 
   }

   if (IS_AFFECTED(victim,AFF_POISON) && number_percent() > 30) 
   { 
     affect_strip(victim,gsn_poison); 
     act("$n looks a bit healthier.\n\r",victim,0,0,TO_ROOM); 
     send_to_char("The poison in your body is nullified.\n\r",victim); 
   } 

   check_improve(ch,gsn_herb,TRUE,4);

   if(victim->hit >= victim->max_hit)
   {
    if(ch == victim)
     send_to_char("But you are not wounded!\n\r",ch);
    else
     act("But $N is not wounded!",ch,NULL,victim,TO_CHAR);
    return;
   }

    af.where = TO_AFFECTS; 
    af.type = gsn_herb; 
    af.location = APPLY_REGEN; 
    af.duration = ch->level/10;
    af.modifier = ch->level < 51 ? ch->level/2 : ch->level;
    af.bitvector = 0; 
    af.level = ch->level; 
    affect_to_char(victim,&af); 
        return; 
}

void do_barkskin( CHAR_DATA *ch,char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_barkskin) <= 0)
        || (ch->level < skill_table[gsn_barkskin].skill_level[ch->class] ) )
    {
	send_to_char("You do not know how to turn your skin to bark.\n\r",ch);
	return;
    }

    if (is_affected(ch,gsn_barkskin) )
    {
	send_to_char("Your skin is already covered in bark.\n\r",ch);
	return;
    }

    if (ch->mana < 60)
    {
        send_to_char("You do not have enough mental power to concentrate.\n\r",ch);
	return;
    }

    if (number_percent() > get_skill(ch,gsn_barkskin))
    {
        send_to_char("Your skin begins to harden but grows soft again.\n\r",ch);
        check_improve(ch,gsn_barkskin,FALSE,1);
        ch->mana -= 30;
        return;
    }

    af.where = TO_AFFECTS;
    af.type = gsn_barkskin;
    af.level = ch->level;
    af.location = APPLY_AC;
    af.modifier = -(ch->level * 2/3);
    af.duration = ch->level/5;
    af.bitvector = 0;
    affect_to_char(ch,&af);

    ch->mana -= 60;

    act("$n's brow furrows and $s skin turns into bark.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your skin hardens into bark.\n\r",ch);
    check_improve(ch,gsn_barkskin,TRUE,1);
    return;
}

void spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
/*    if(ch->in_room->sector_type != SECT_FOREST &&
       ch->in_room->sector_type != SECT_MOUNTAIN)*/

       if ((ch->in_room->sector_type == SECT_CITY)
       || (ch->in_room->sector_type == SECT_INSIDE))
       {
        send_to_char("You must be in the wilderness to entangle someone.\n\r",ch);
        return;
       }

    if (victim == ch)
    {
      send_to_char("You cannot entangle yourself!\n\r",ch);
      return;
    }

    if ( is_affected( victim, gsn_entangle ))
    {
          act("$N is already entangled in vines.",
              ch,NULL,victim,TO_CHAR);
        return;
    }

    if ( IS_AFFECTED(victim,AFF_SLOW))
    {
          act("$N is moving sluggishly already.",
              ch,NULL,victim,TO_CHAR);
        return;
    }

    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
        act("Vines from the surrounding forest lunge at $N but miss.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
            act("Vines from the surrounding forest grab $N but $E struggles free.",ch,NULL,victim,TO_CHAR);
            return;
        }

        act("$n is slowed by entagling vines.",victim,NULL,NULL,TO_ROOM);

        if(is_affected(victim,skill_lookup("haste")))
         affect_strip(victim,skill_lookup("haste"));
        else
        REMOVE_BIT(victim->affected_by,AFF_HASTE);

        return;
    } 

    af.where     = TO_AFFECTS;
    af.type      = gsn_entangle;
    af.level     = level;
    af.duration  = level/3;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32) - (level >= 48);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "Long forest vines reach out and entangle you!\n\r", victim );
    act("$n is entangled by long green vines!",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_crevice(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, save_num = 0;
    int count;

    if ( (ch->in_room->sector_type != SECT_FOREST)
    && ( ch->in_room->sector_type != SECT_MOUNTAIN)
    && ( ch->in_room->sector_type != SECT_FIELD) )
    {
        send_to_char("You do not control the elements in this environment.\n\r",ch);
        return;
    }

    act("$n opens a crevice in the ground beneath $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n opens a crevice beneath you.",ch,NULL,victim,TO_VICT);
    act("You call upon the magic of the wilderness and create a crevice beneath $N.",ch,NULL,victim,TO_CHAR);

    if(number_bits(2)==0)
     dam = dice(level*3, 18);
    else
     dam = dice(level*2, 14);

    dam += dice(level,12);

    for (count = 0; count < 2; count++)
    {
        if (saves_spell(level, victim, DAM_BASH) )
            save_num++;
    }

    if (save_num == 0)
    {
        act("$n cries out as $s the crevice closes in on $m!",victim,NULL,NULL,TO_ROOM);
        send_to_char("You fall into the crevice and scream in agony as it closes in on you!\n\r",victim);
        damage(ch,victim,dam,gsn_crevice,DAM_BASH,TRUE,0);
        return;
    }

    dam /= save_num * 2;   // Up to divided by 4

    damage(ch,victim,dam,gsn_crevice,DAM_BASH,TRUE,0);

    return;
}

void do_detect_terrain( CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    if ( (get_skill(ch,gsn_detect_terrain) <= 0)
        || (ch->level < skill_table[gsn_detect_terrain].skill_level[ch->class] ) )
    {
	send_to_char("You do not know how to detect the environment.\n\r",ch);
	return;
    }


    if ( is_affected(ch, gsn_detect_terrain) )
    {
        send_to_char("You are already identifying the environment. \n\r",ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_detect_terrain;
    af.level     = ch->level;
    af.duration  = ch->level/2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    send_to_char( "Your awareness of the environment improves.\n\r", ch );
    return;
}

/*-------------- MISCELLANEOUS -------------------*/
void do_throw(CHAR_DATA *ch,char *argument) 
{ 
    CHAR_DATA *victim; 
    int dam, chance, percent; 
    char arg[MAX_INPUT_LENGTH]; 
 
    if ( ( chance = get_skill(ch,gsn_throw) ) == 0 
       || !has_skill(ch,gsn_throw) ) 
    { 
       send_to_char("Throwing? What's that?\n\r",ch); 
       return; 
    } 
 
    one_argument(argument,arg); 

    victim = ( arg[0] == '\0' ? ch->fighting : get_char_room(ch,arg) ); 
 
    if ( victim == NULL ) 
    { 
        send_to_char("They aren't here.\n\r",ch); 
        return; 
    }
 
    if ( victim == ch ) 
    { 
        send_to_char("You can't throw yourself dummy.\n\r",ch); 
        return; 
    }
    
    if (!is_affected(ch,gsn_martial_arts))
    {
	    send_to_char("You must be focused on the martial arts to use this skill.\n\r",ch);
	    return;
    }
    
    if ( ch->stunned > 0 )
    {
	send_to_char ("You're still a little woozy.\n\r", ch);
	return;
    } 

    if ( ( victim->fighting != ch ) && ( ch->fighting != victim ) )
    { 
        send_to_char("But you aren't engaged in combat with them.\n\r",ch); 
        return; 
    } 
 
    if ( get_eq_char(ch,WEAR_WIELD) != NULL || get_eq_char(ch,WEAR_SECONDARY) != NULL )
    { 
        send_to_char("You cannot throw with items in your hands.\n\r",ch); 
        return; 
    }
    
    if ( IS_NPC(victim) )
    {
       chance += (ch->level - (victim->level - 20));
       chance -= get_curr_stat(victim,STAT_DEX)*2/3;
    }
    else
    {
   	   chance += (ch->level - victim->level); 
    	   chance -= get_curr_stat(victim,STAT_DEX);
    }
    
    chance += get_curr_stat(ch,STAT_DEX)*2/3; 
    chance = URANGE(5, chance, 80); 
 
    if (is_affected(ch,AFF_BLIND))
    {
	    chance /=3; //Harder to land while blind.
    }
    
    if (number_percent() > chance) 
    { 
        dam = 0; 
        check_improve(ch,gsn_throw,FALSE,3); 
    } 
    else 
    { 
        if (check_tumble(ch,victim,gsn_throw)) 
        { 
           check_improve(ch,gsn_throw,TRUE,2); 
           act("$N rolls free of $n's throw.",ch,0,victim,TO_NOTVICT); 
           act("You roll out of $n's throw attempt.",ch,0,victim,TO_VICT); 
           act("$N rolls free of your throw attempt.",ch,0,victim,TO_CHAR);
           WAIT_STATE(ch,skill_table[gsn_throw].beats);
           return; 
        } 
 
	dam = (ch->level * number_range(1,5));

        act("$n grabs $N and throws $M to the ground with stunning force!",ch,0,victim,TO_NOTVICT);
        act("You grab $N and throw $M to the ground with stunning force!",ch,0,victim,TO_CHAR); 
        act("$n grabs you and throws you to the ground with stunning force!",ch,0,victim,TO_VICT); 
 
        dam += (get_curr_stat(ch,STAT_DEX)+str_app[get_curr_stat(ch,STAT_STR)].todam)*2; 
        check_improve(ch,gsn_throw,TRUE,3);
        
        if ( !is_affected( victim, gsn_throw ) )
        {
           AFFECT_DATA af;
  
           WAIT_STATE(victim,1*PULSE_VIOLENCE);
            
            af.where      = TO_AFFECTS;
            af.type       = gsn_throw;
            af.level      = ch->level;
            af.duration   = 1;
            af.location   = APPLY_CON;
            af.modifier   = -5;
            af.bitvector  = 0;
        
            affect_to_char( victim, &af );
        }


        /* A little extra kick */
        if ( ( percent = number_percent() ) <= ( get_skill(ch,gsn_throw) / 6 ) )
        {
           dam = 2 * dam + (dam * 2 * percent / 100);
           victim->stunned = 1;
           act ("{)You are stunned, and have trouble getting back up!{x", ch, NULL, victim, TO_VICT);
           act ("{.$N is stunned by the force of your throw!{x", ch, NULL, victim, TO_CHAR);
           act ("{($N is having trouble getting back up.{x", ch, NULL, victim, TO_NOTVICT);
        }
        
        
    } 
 
    damage(ch,victim,dam,gsn_throw,DAM_BASH,TRUE,0); 
    WAIT_STATE(ch,1*PULSE_VIOLENCE); 
    
    if ( ch->fighting == victim ) 
        check_ground_stomp(ch,victim,chance,dam);  
 
    return; 
} 

void do_nerve(CHAR_DATA *ch,char *argument) 
{ 
    char buf[MAX_STRING_LENGTH]; 
    CHAR_DATA *victim; 
    char arg[MAX_INPUT_LENGTH]; 
    AFFECT_DATA af; 
    int chance; 
 
    if(ch->class == CLASS_MONK)
    {
     do_monk_nerve(ch,argument);
     return;
    }
    
    if ( (chance = get_skill(ch,gsn_nerve)) == 0 
    || !has_skill(ch,gsn_nerve)) 
    { 
        send_to_char("You don't know how to stun opponents.\n\r",ch); 
        return; 
    } 
 
    one_argument(argument,arg); 
    if (arg[0] == '\0') 
        victim = ch->fighting; 
    else 
        victim = get_char_room(ch,arg); 
 
    if (victim == NULL) 
    { 
        send_to_char("Attempt to stun who?\n\r",ch); 
        return; 
    } 
    if (victim == ch) 
    { 
        send_to_char("You can't do that.\n\r",ch); 
        return; 
    } 
    
    if (ch->stunned > 0)
    {
	send_to_char ("You're still a little woozy.\n\r", ch);
	return;
    }
    
    if (is_safe(ch,victim)) 
        return; 
 
    if (is_affected(victim,gsn_nerve)) 
    { 
        send_to_char("They have already been weakened.\n\r",ch);  
        return; 
    } 
 
      
    chance += (ch->level - victim->level)*2; 
    chance -= get_curr_stat(victim,STAT_DEX)/3; 
    chance += get_curr_stat(ch,STAT_DEX)/2; 
    chance -= get_curr_stat(victim,STAT_CON)/3;
    chance += get_curr_stat(ch,STAT_CON)/2;
    if(IS_NPC(victim))
          chance+=(victim->level/5);
    if (number_percent() > chance) 
    { 
        act("$n grasps $N's neck but fails to stun them.",ch,0,victim,TO_NOTVICT); 
        act("You grasp $N's neck but fail to stun them.",ch,0,victim,TO_CHAR); 
        act("$n grasps your neck but fails to stun you.",ch,0,victim,TO_VICT); 
        check_improve(ch,gsn_nerve,FALSE,3); 
        WAIT_STATE(ch,PULSE_VIOLENCE); 
        return; 
    } 
    else 
    { 
    act("$n grasps $N's neck and weakens $m with pressure points.",ch,0,victim,TO_NOTVICT); 
    act("You grasp $N's neck and weaken $m with pressure points.",ch,0,victim,TO_CHAR); 
    act("$n grasps your neck and weakens you with pressure point.",ch,0,victim,TO_VICT); 
    check_improve(ch,gsn_nerve,TRUE,3); 
    af.where = TO_AFFECTS; 
    af.type = gsn_nerve; 
    af.location = APPLY_STR; 
    af.duration = 1; 
    af.bitvector = 0; 
    af.modifier = -3; 
    af.level = ch->level; 
    affect_to_char(victim,&af); 
    WAIT_STATE(ch,PULSE_VIOLENCE);
    victim->stunned = 1;
     act ("{)You are stunned, and have trouble getting back up!{x", ch, NULL, victim, TO_VICT);
     act ("{.$N is stunned by your nerve strike!{x", ch, NULL, victim, TO_CHAR);
     act ("{($N is having trouble getting back up.{x", ch, NULL, victim, TO_NOTVICT);
    } 
 
        if (!IS_NPC(ch) && !IS_NPC(victim) 
        && (ch->fighting == NULL 
        || victim->fighting == NULL) ) 
        { 
         sprintf(buf,"Help, %s is attacking me!",PERS(ch,victim)); 
         do_yell(victim,buf); 
        } 
        if (victim->fighting == NULL) 
        { 
         multi_hit(victim,ch,TYPE_UNDEFINED); 
        } 
                 
    return; 
}

bool check_tumble(CHAR_DATA *ch, CHAR_DATA *victim, int dt) 
{ 
        int chance; 
 
        if ( (chance = get_skill(victim,gsn_roll)) == 0 
        || !has_skill(victim,gsn_roll)) 
                return FALSE; 
                 
        chance *= 4; 
        chance /= 5; 
        chance += get_curr_stat(victim,STAT_DEX); 
        chance -= victim->carry_weight/100;
        chance -= get_curr_stat(ch,STAT_DEX);
        if (number_percent() > chance) 
        { 
           check_improve(victim,gsn_roll,FALSE,2); 
           return FALSE; 
        } 
 
        check_improve(victim,gsn_roll,TRUE,2); 
        return TRUE; 
} 

void do_lunge( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam; 
        char arg[MAX_INPUT_LENGTH]; 
        OBJ_DATA *wield,*second; 
        int multiplier; 
        bool DUAL = FALSE; 
         
        dam = 0; 
        one_argument(argument,arg); 
        if ((chance = get_skill(ch,gsn_lunge) ) == 0 
        || !has_skill(ch,gsn_lunge) ) 
        { 
        send_to_char("Lunging? What's that?\n\r",ch); 
        return; 
        } 
        if (arg[0] == '\0') 
        { 
        victim = ch->fighting; 
         if (victim == NULL) 
         { 
            send_to_char("But you aren't fighting anyone!\n\r",ch); 
            return; 
         } 
        } 
        else if ( (victim = get_char_room(ch,arg)) == NULL ) 
        { 
                send_to_char("They aren't here.\n\r",ch); 
                return; 
        } 
        if (victim == ch) 
        { 
        send_to_char("You can't lunge at yourself!\n\r",ch); 
        return; 
        }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt to lunge, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to lunge, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

        if(ch->stunned)
        {
         send_to_char("You are still a bit woozy.\n\r",ch);
         return;
        }
     
        wield = get_eq_char(ch,WEAR_WIELD); 
        second = get_eq_char(ch,WEAR_SECONDARY); 
 
        if ((wield == NULL) 
        || (( wield->value[0] != WEAPON_SWORD) 
        && ( wield->value[0] != WEAPON_SPEAR) 
        && ( wield->value[0] != WEAPON_POLEARM) )) 
        { 
         if ((second == NULL) 
         || (( second->value[0] != WEAPON_SWORD) 
         && ( second->value[0] != WEAPON_SPEAR) 
         && ( second->value[0] != WEAPON_POLEARM) )) 
         { 
          send_to_char("You must be wielding a sword, spear or polearm to lunge.\n\r",ch); 
          return; 
         } 
         else 
         { 
         DUAL = TRUE; 
         } 
        } 
 
        if (is_safe(ch,victim) ) 
        return; 
 
        chance += ch->carry_weight/25; 
        chance -= victim->carry_weight/20; 
        chance += (ch->size - victim->size)*20; 
        chance -= get_curr_stat(victim,STAT_DEX); 
        chance += get_curr_stat(ch,STAT_STR)/3; 
        chance += get_curr_stat(ch,STAT_DEX)/2; 
        if (IS_AFFECTED(ch,AFF_HASTE) ) 
                chance += 10; 
        if (IS_AFFECTED(victim,AFF_HASTE) ) 
                chance -= 10; 
        chance += (ch->level - victim->level);

        act("$n attempts to impale $N with a quick lunge!",ch,0,victim,TO_NOTVICT); 
        act("You attempt to impale $N with a quick lunge!",ch,0,victim,TO_CHAR); 
        act("$n attempts to impale you with a quick lunge!",ch,0,victim,TO_VICT); 
 
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_lunge,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_lunge].beats); 
 
        if(DUAL) 
        wield = second; 
 
        if (wield->pIndexData->new_format) 
                dam = dice(wield->value[1],wield->value[2]); 
        else 
                dam = number_range(wield->value[1],wield->value[2]); 
 
        if (get_skill(ch,gsn_enhanced_damage) > 0 ) 
        { 
         if (number_percent() <= get_skill(ch,gsn_enhanced_damage) ) 
         { 
           check_improve(ch,gsn_enhanced_damage,TRUE,1); 
           dam += dam * (number_range(50,100)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100; 
         } 
        } 
 
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_lunge]; 
        dam /= 100; 
        multiplier = number_range(ch->level/8, ch->level/4); 
        multiplier /= 10; 
        multiplier += 5/4; 
        dam *= multiplier;
        if(!DUAL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
         dam *= 1.5;
 
        if (dam <= 0) 
                dam = 1; 
        damage(ch,victim,dam,gsn_lunge,DAM_PIERCE,TRUE,0); 
        } 
        else 
        { 
        damage(ch,victim,dam,gsn_lunge,DAM_PIERCE,TRUE,0); 
        check_improve(ch,gsn_lunge,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_lunge].beats); 
        } 
        return; 
} 

void special_move( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int dam = number_range(5,10) + GET_DAMROLL(ch);

    if (dam < 10) dam = 10;

    switch (number_range(1,7))
    {
    default:
	return;
    case 1: /* Punch */
	act("You pull your hands into your waist then snap them into $N's stomach.",ch,NULL,victim,TO_CHAR);
	act("$n pulls $s hands into $s waist then snaps them into your stomach.",ch,NULL,victim,TO_VICT);
	act("$n pulls $s hands into $s waist then snaps them into $N's stomach.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_punch,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You double over in agony, and fall to the ground gasping for breath.",victim,NULL,NULL,TO_CHAR);
	act("$n doubles over in agony, and falls to the ground gasping for breath.",victim,NULL,NULL,TO_ROOM);
        victim->stunned = 2;
	break;
    case 2: /* Spinkick */
	act("You spin in a low circle, catching $N behind $S ankle.",ch,NULL,victim,TO_CHAR);
	act("$n spins in a low circle, catching you behind your ankle.",ch,NULL,victim,TO_VICT);
	act("$n spins in a low circle, catching $N behind $S ankle.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_spinkick,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You crash to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n crashes to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
        victim->stunned = 2;
	break;
    case 3: /* Elbow */
	act("You roll between $N's legs and flip to your feet.",ch,NULL,victim,TO_CHAR);
	act("$n rolls between your legs and flips to $s feet.",ch,NULL,victim,TO_VICT);
	act("$n rolls between $N's legs and flips to $s feet.",ch,NULL,victim,TO_NOTVICT);
	act("You spin around and smash your elbow into the back of $N's head.",ch,NULL,victim,TO_CHAR);
	act("$n spins around and smashes $s elbow into the back of your head.",ch,NULL,victim,TO_VICT);
	act("$n spins around and smashes $s elbow into the back of $N's head.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_elbow,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You fall to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n falls to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
        victim->stunned = 2;
	break;
    case 4: /* Kick */
	act("You somersault over $N's head and land lightly on your toes.",ch,NULL,victim,TO_CHAR);
	act("$n somersaults over your head and lands lightly on $s toes.",ch,NULL,victim,TO_VICT);
	act("$n somersaults over $N's head and lands lightly on $s toes.",ch,NULL,victim,TO_NOTVICT);
	act("You roll back onto your shoulders and kick both feet into $N's back.",ch,NULL,victim,TO_CHAR);
	act("$n rolls back onto $s shoulders and kicks both feet into your back.",ch,NULL,victim,TO_VICT);
	act("$n rolls back onto $s shoulders and kicks both feet into $N's back.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_kick, DAM_BASH, TRUE, 0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You fall to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n falls to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
	act("You flip back up to your feet.",ch,NULL,NULL,TO_CHAR);
	act("$n flips back up to $s feet.",ch,NULL,NULL,TO_ROOM);
	stop_fighting(victim, TRUE);
        victim->stunned = 2;
	break;
    case 5: /* Headbutt */
	act("You grab $N by the neck and slam your head into $S face.",ch,NULL,victim,TO_CHAR);
	act("$n grabs $N by the neck and slams $s head into $S face.",ch,NULL,victim,TO_NOTVICT);
	act("$n grabs you by the neck and slams $s head into your face.",ch,NULL,victim,TO_VICT);
        damage(ch,victim,dam,gsn_headbutt,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You crash to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n crashes to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
        victim->stunned = 2;
	break;
    case 6: /* Knee */
	act("You slam your fist into $N's stomach, who doubles over in agony.",ch,NULL,victim,TO_CHAR);
	act("$n slams $s fist into your stomach, and you double over in agony.",ch,NULL,victim,TO_VICT);
	act("$n slams $s fist into $N's stomach, who doubles over in agony.",ch,NULL,victim,TO_NOTVICT);
	act("You grab $N by the head and slam $S face into your knee.",ch,NULL,victim,TO_CHAR);
	act("$n grabs you by the head and slams your face into $s knee.",ch,NULL,victim,TO_VICT);
	act("$n grabs $N by the head and slams $S face into $s knee.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_knee,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You roll onto your back and smash your feet into $N's chest.",ch,NULL,victim,TO_CHAR);
	act("$n rolls onto $s back and smashes $s feet into your chest.",ch,NULL,victim,TO_VICT);
	act("$n rolls onto $s back and smashes $s feet into $N's chest.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_kick,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You crash to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n crashes to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
	act("You flip back up to your feet.",ch,NULL,NULL,TO_CHAR);
	act("$n flips back up to $s feet.",ch,NULL,NULL,TO_ROOM);
	stop_fighting(victim, TRUE);
        victim->stunned = 2;
	break;
    case 7: /* Punch again */
	act("You duck under $N's attack and pound your fist into $S stomach.",ch,NULL,victim,TO_CHAR);
	act("$n ducks under your attack and pounds $s fist into your stomach.",ch,NULL,victim,TO_VICT);
	act("$n ducks under $N's attack and pounds $s fist into $N's stomach.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_punch,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You double over in agony.",victim,NULL,NULL,TO_CHAR);
	act("$n doubles over in agony.",victim,NULL,NULL,TO_ROOM);

	act("You grab $M by the head and smash your knee into $S face.",ch,NULL,victim,TO_CHAR);
	act("$n grabs you by the head and smashes $s knee into your face.",ch,NULL,victim,TO_VICT);
	act("$n grabs $M by the head and smashes $s knee into $N's face.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_knee,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;

	act("You stamp on the back of $N's leg, forcing $M to drop to one knee.",ch,NULL,victim,TO_CHAR);
	act("$n stamps on the back of your leg, forcing you to drop to one knee.",ch,NULL,victim,TO_VICT);
	act("$n stamps on the back of $N's leg, forcing $M to drop to one knee.",ch,NULL,victim,TO_NOTVICT);

	act("You grab $N by the hair and yank $S head back.",ch,NULL,victim,TO_CHAR);
	act("$n grabs you by the hair and yank your head back.",ch,NULL,victim,TO_VICT);
	act("$n grabs $N by the hair and yank $S head back.",ch,NULL,victim,TO_NOTVICT);

	act("You hammer your elbow down into $N's face.",ch,NULL,victim,TO_CHAR);
	act("$n hammers $s elbow down into your face.",ch,NULL,victim,TO_VICT);
	act("$n hammers $s elbow down into $N's face.",ch,NULL,victim,TO_NOTVICT);
        damage(ch,victim,dam,gsn_elbow,DAM_BASH,TRUE,0);
	if (victim == NULL || victim->position == POS_DEAD) return;
	act("You crash to the ground, stunned.",victim,NULL,NULL,TO_CHAR);
	act("$n crashes to the ground, stunned.",victim,NULL,NULL,TO_ROOM);
        victim->stunned = 2;
	break;
    }
    return;
}

/* Hara Kiri -- Skyntil */
/*
void do_hara_kiri(CHAR_DATA *ch, char *argument)
{
 char arg1[MSL/4];
 AFFECT_DATA af;
 int counter;

 one_argument(argument,arg1);

 if(!is_warrior_class(ch->class) || ch->class == CLASS_MONK)
 {
  send_to_char("Huh?\n\r",ch);
  return;
 }

 if(is_affected(ch,gsn_hara_kiri))
 {
  send_to_char("You must recover from your previous attempt first.\n\r",ch);
  return;
 }

 if(ch->hit < ch->max_hit/2)
 {
  send_to_char("You do not have the health to perform this ritual.\n\r",ch);
  return;
 }
*/
 /* Full Hara */
 /* if(!str_cmp(arg1,"full"))
 {
  act("You tense your body incredibly.",ch,NULL,NULL,TO_CHAR);
  act("$N looks very pale all of a sudden.",NULL,NULL,ch,TO_NOTVICT);
  act("Your veins pop up from beneath your skin.",ch,NULL,NULL,TO_CHAR);
  act("$N slits his wrists!",NULL,NULL,ch,TO_NOTVICT);
  act("You slit your wrists and let the blood drain out.",ch,NULL,NULL,TO_CHAR);
  act("Blood pours onto the floor. $N faints.",NULL,NULL,ch,TO_NOTVICT);
  act("You faint.",ch,NULL,NULL,TO_CHAR);
  ch->hit = 1;
  ch->stunned = 2;

  af.where = TO_AFFECTS;
  af.type = gsn_hara_kiri;
  af.level = ch->level;
  af.duration = ch->level/4;
  af.modifier = (ch->level-1)*2.75;
  af.location = APPLY_REGEN;
  af.bitvector = 0;
  affect_to_char(ch,&af);
  act("You feel slightly disorientated.",ch,NULL,NULL,TO_CHAR);
  for(counter = 0; counter < 4000; counter++)
   improve_toughness(ch);

  return;
 }
*/
//changes here -bree
 /* if(!str_cmp(arg1,""))
 {
  act("You must use 'hara full, hara half, or hara minor.",ch,NULL,NULL,TO_CHAR);
  
  return;
 }
*/


 /* Half Hara */
 /* if(!str_cmp(arg1,"half"))
 {
  act("You tense your body incredibly.",ch,NULL,NULL,TO_CHAR);
  act("$N looks very pale all of a sudden.",NULL,NULL,ch,TO_NOTVICT);
  act("Your veins pop up from beneath your skin.",ch,NULL,NULL,TO_CHAR);
  act("$N slits his wrists!",NULL,NULL,ch,TO_NOTVICT);
  act("You slit your wrists and let the blood drain out.",ch,NULL,NULL,TO_CHAR);
  act("Blood pours onto the floor. $N faints.",NULL,NULL,ch,TO_NOTVICT);
  act("You faint.",ch,NULL,NULL,TO_CHAR);
  ch->hit /= 2;
  ch->stunned = 1;

  af.where = TO_AFFECTS;
  af.type = gsn_hara_kiri;
  af.level = ch->level/2;
  af.duration = ch->level/7;
  af.modifier = (ch->level-1)*2.25;
  af.location = APPLY_REGEN;
  af.bitvector = 0;
  affect_to_char(ch,&af);
  act("You feel invigorated.",ch,NULL,NULL,TO_CHAR);
  for(counter = 0; counter < 2500; counter++)
   improve_toughness(ch);

  return;
 }
*/
 /* Minor Hara */
 /* if(arg1[0] == '\0' || !str_cmp(arg1,"minor"))
 {
  act("You tense your body incredibly.",ch,NULL,NULL,TO_CHAR);
  act("$N looks very pale all of a sudden.",NULL,NULL,ch,TO_NOTVICT);
  act("Your veins pop up from beneath your skin.",ch,NULL,NULL,TO_CHAR);
  act("$N slits his wrists!",NULL,NULL,ch,TO_NOTVICT);
  act("You slit your wrists and let the blood drain out.",ch,NULL,NULL,TO_CHAR);
  act("Blood pours onto the floor. $N faints.",NULL,NULL,ch,TO_NOTVICT);
  act("You faint.",ch,NULL,NULL,TO_CHAR);
  ch->hit -= ch->hit/3;
  WAIT_STATE(ch,6);

  af.where = TO_AFFECTS;
  af.type = gsn_hara_kiri;
  af.level = ch->level;
  af.duration = ch->level/9;
  af.modifier = (ch->level-1)*1.75;
  af.location = APPLY_REGEN;
  af.bitvector = 0;
  affect_to_char(ch,&af);
  act("You feel invigorated.",ch,NULL,NULL,TO_CHAR);
  for(counter = 0; counter < 1500; counter++)
   improve_toughness(ch);

  return;
 }

 return;
}
*/

//hara fix for GCC 4 by Bree

void do_hari(CHAR_DATA *ch, char *argument)
{
    char arg1[MSL/4];
    AFFECT_DATA af;
    
    one_argument(argument,arg1);

    if(!is_warrior_class(ch->class) || ch->class == CLASS_MONK)
    {
     send_to_char("Huh?\n\r",ch);
     return;
    }
   
    if (is_affected(ch,gsn_hari) )
    {
        send_to_char("You are already affected by hara!\n\r",ch);
	return;
    }

 if(!str_cmp(arg1,"full"))
 {
  act("You tense your body incredibly.",ch,NULL,NULL,TO_CHAR);
  act("$N looks very pale all of a sudden.",NULL,NULL,ch,TO_NOTVICT);
  act("Your veins pop up from beneath your skin.",ch,NULL,NULL,TO_CHAR);
  act("$N slits his wrists!",NULL,NULL,ch,TO_NOTVICT);
  act("You slit your wrists and let the blood drain out.",ch,NULL,NULL,TO_CHAR);
  act("Blood pours onto the floor. $N faints.",NULL,NULL,ch,TO_NOTVICT);
  act("You faint.",ch,NULL,NULL,TO_CHAR);
  ch->hit = 1;
  ch->stunned = 2;

    af.where = TO_AFFECTS;
    af.type = gsn_hari;
    af.level = ch->level;
    af.location = APPLY_REGEN;
    af.modifier = (ch->level-1)*2.75;
    af.duration = ch->level/4;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    improve_toughness(ch);
     ch->toughness = (ch->level)*3.5;
   return;
 }

if(!str_cmp(arg1,"half"))
 {
  act("You tense your body incredibly.",ch,NULL,NULL,TO_CHAR);
  act("$N looks very pale all of a sudden.",NULL,NULL,ch,TO_NOTVICT);
  act("Your veins pop up from beneath your skin.",ch,NULL,NULL,TO_CHAR);
  act("$N slits his wrists!",NULL,NULL,ch,TO_NOTVICT);
  act("You slit your wrists and let the blood drain out.",ch,NULL,NULL,TO_CHAR);
  act("Blood pours onto the floor. $N faints.",NULL,NULL,ch,TO_NOTVICT);
  act("You faint.",ch,NULL,NULL,TO_CHAR);
  ch->hit /= 2;
  ch->stunned = 1;

 af.where = TO_AFFECTS;
    af.type = gsn_hari;
    af.level = ch->level;
    af.location = APPLY_REGEN;
    af.modifier = (ch->level-1)*2.25;
    af.duration = ch->level/7;
    af.bitvector = 0;
    affect_to_char(ch,&af);
    improve_toughness(ch);
  act("You feel invigorated.",ch,NULL,NULL,TO_CHAR);

improve_toughness(ch);
 ch->toughness = (ch->level)*3.5;
   return;
 }



return;
}

void do_adamantium_palm( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    AFFECT_DATA af;
    int chance;
    int dam;
    bool NOHIT = FALSE;

    if( (chance = get_skill(ch,gsn_adamantium_palm)) < 1
     || !has_skill(ch,gsn_adamantium_palm) )
    {
        send_to_char("Huh?\n\r",ch);
    	return;
    }
    if (!is_affected(ch,gsn_martial_arts))
    {
	 	send_to_char("You must be focused on the martial arts to use this skill.\n\r",ch);
	    return;
    }
    
    one_argument( argument, arg );
    
    if( arg[0] == '\0' )
    {
        NOHIT = TRUE;
        send_to_char("Adamantium Palm who?\n\r",ch);
        return;
    }
    
    if( ch->fighting )
    {
      NOHIT = TRUE;
    }
    else if( (victim = get_char_room(ch,arg)) == NULL )
    {
    	send_to_char("They're not here.\n\r",ch);
    	return;
    }
    else if( victim->hit < victim->max_hit/2 )
    {
        send_to_char("They're too aware to be attacked.\n\r",ch);
    	return;
    }
    else if( is_affected(victim,gsn_adamantium_palm) )
    {
        send_to_char("They're already suffering from being struck!\n\r",ch);
    	return;
    }
    else if( ch->mana < 75 )
    {
        send_to_char("You do not have the mental energy.\n\r",ch);
    	return;
    }

    if( number_percent() > UMIN(95, chance) && !NOHIT)
    {
    	act("Your concentration slips as you strike at $N!",ch,0,victim,TO_CHAR);
    	act("$n strikes at you, but loses $s concentration!",ch,0,victim,TO_VICT);
    	act("$n strikes at $N, but loses $s concentration!",ch,0,victim,TO_NOTVICT);
        damage(ch,victim,0,gsn_adamantium_palm,DAM_BASH,TRUE,0);
        ch->mana = UMAX( 0, ch->mana-35 );
        check_improve(ch,gsn_adamantium_palm,FALSE,1);
        WAIT_STATE(ch, skill_table[gsn_adamantium_palm].beats/2);
	return;
    }
    else if(!NOHIT)
    {
    act("You channel your will through your palm as you strike $N!",ch,0,victim,TO_CHAR);
    act("$n viciously strikes you with a hardened palm!",ch,0,victim,TO_VICT);
    act("$n viciously strikes $N with a hardened palm!",ch,0,victim,TO_NOTVICT);

    ch->mana = UMAX( 0, ch->mana - 70 );
    WAIT_STATE(ch, skill_table[gsn_adamantium_palm].beats);

    /* Cut damage down a bit */
    dam = dice((ch->level)/4,13);
    dam += dice(9,chance/2);
    
    if( damage(ch,victim,dam,gsn_adamantium_palm,DAM_BASH,TRUE,0) )
    {
    	af.where = TO_AFFECTS;
        af.level = ch->level;
    	af.location = APPLY_AC;
        /* Changed */
        af.modifier = ch->level;
    	af.bitvector = 0;
        af.type = gsn_adamantium_palm;
        af.duration = ch->level / 12;
    
    	affect_to_char(victim,&af);
    }
        check_improve(ch,gsn_adamantium_palm,TRUE,1);
        return;
   }
  
    return;
}

void do_roundhouse(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *was_in;
    CHAR_DATA *victim,*vch,*vch_next;
    int attempt, chance, dam, level;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    one_argument(argument,arg1);

    if(IS_NPC(ch))
    return;

    if ( (chance = get_skill(ch,gsn_roundhouse)) == 0
    || !has_skill(ch,gsn_roundhouse))
    {
        send_to_char("Huh?\n\r",ch);
        return;
    }
    
    if (!is_affected(ch,gsn_martial_arts))
    {
	    send_to_char("You must be focused on the martial arts to use this skill.\n\r",ch);
	    return;
    }

    if(ch->fighting != NULL)
    victim = ch->fighting;
    else
    {
     if( (victim = get_char_room(ch,arg1)) == NULL )
     {
    	send_to_char("They're not here.\n\r",ch);
    	return;
     }
    }

    if(victim == ch)
    { 
     send_to_char("Kicking yourself? get real.\n\r",ch);
     return;
    }
    
    if (!IS_NPC(victim) && !can_pkill(ch,victim)) {
     send_to_char("You can't seem to kick them.",ch);
     return;
    }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt to roundhouse, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to roundhouse, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }
      
    if(ch->stunned)
    {
     send_to_char("You are still a bit woozy.\n\r",ch);
     return;
    }

    level = ch->level + 5;
    // chance = (ch->level - victim->level) * 2;
    chance += number_range(20,50);
    chance -= get_curr_stat(victim,STAT_DEX)/2;
    chance -= get_curr_stat(victim,STAT_STR)/3;
    chance += get_curr_stat(ch,STAT_STR)/3;
    chance += get_curr_stat(ch,STAT_DEX);

    if (IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;
    if (IS_AFFECTED(ch,AFF_HASTE))
        chance += 25;
    if (IS_AFFECTED(victim,AFF_SLOW))
        chance += 20;
    if (IS_AFFECTED(ch,AFF_SLOW))
        chance -= 30;
    if (IS_AFFECTED(victim,AFF_FLYING))
        chance += 5;
    chance += (ch->size - victim->size)*10;

   if (!IS_NPC(victim) && !IS_NPC(ch) && (victim->hit > 1))
   {
      sprintf(buf,"Help! %s just tried to kick me out of the room!",PERS(ch,victim));

      if(ch->fighting == NULL)
      {
      do_yell(victim,buf);
      }
   }


    if (number_percent() > chance)
    {
        act("You spin your leg around but miss $N.",ch,0,victim,TO_CHAR);
        act("$n spins his leg around but misses you.",ch,0,victim,TO_VICT);
        act("$n spins his leg visciously at $N but misses.",ch,0,victim,TO_NOTVICT);
        check_improve(ch,gsn_roundhouse,FALSE,3);
        WAIT_STATE(ch,12);
        if(victim->fighting == NULL)
        multi_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }

    was_in = victim->in_room;
    dam = dice(level,8);
    
    // Stance modifier
    if (!IS_NPC(ch) && ch->stance[0] > 0 && number_percent() < 3)
    {
	int stance = ch->stance[0];
        if (ch->stance[stance] >= 200)
	    dam += dice(ch->stance[stance],2);
	else if(ch->stance[stance] > 0)
 	    dam += dice(ch->stance[stance],1);
    }

   if(number_percent() < 25)
   {

    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;
        char dir[MAX_STRING_LENGTH/4];

	door = number_door( );

        if (door == 0)
        (strcpy(dir,"north"));
        else if (door == 1)
        (strcpy(dir,"east"));
        else if (door == 2)
        (strcpy(dir,"south"));
        else if (door == 3)
        (strcpy(dir,"west"));
        else if (door == 4)
        (strcpy(dir,"up"));
        else if (door == 5)
        (strcpy(dir,"down"));
        else
        strcpy(dir,"none");

	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
        ||  ( IS_SET(pexit->exit_info, EX_CLOSED) )
	|| ( IS_NPC(victim)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

        if ( ( was_in->sector_type == SECT_AIR)
        ||  ( (pexit->u1.to_room->sector_type == SECT_AIR )
        &&  ( !IS_AFFECTED(victim, AFF_FLYING) ) ) )
            continue;

        if (IS_SET(pexit->exit_info,EX_CLOSED))
        {
        act("You spin around and kick $N through the door with incredible force!",ch,0,victim,TO_CHAR);
        act("$n's roundhouse sends $N crashing through the door with incredible force!",ch,0,victim,TO_NOTVICT);
        act("$n spins around and kicks you through the door with incredible force!",ch,0,victim,TO_VICT);
        }
        else
        {
        act("You spin your leg visciously at $N and kick $M from the room!",ch,0,victim,TO_CHAR);
        act("$n spins his leg visciously at $N and kicks $M from the room!",ch,NULL,victim,TO_NOTVICT);
        act("$n spins his leg visciously at you and kicks you from the room!",ch,0,victim,TO_VICT);
        dam = dice(level,5);
        }

        for(vch = victim->in_room->people; vch != NULL; vch = vch_next)
        {
         vch_next = vch->next_in_room;

         if(IS_IMMORTAL(vch))
         {
         sprintf(buf,"\n\r%s is kicked %s.",victim->name,dir);
         send_to_char(buf,vch);
         }
        }

        char_from_room(victim);
        char_to_room(victim, pexit->u1.to_room);
	do_look(victim,"auto");

        if (IS_SET(pexit->exit_info,EX_CLOSED))
        {
        REMOVE_BIT(pexit->exit_info,EX_CLOSED);
        act("With a deafening crash the door explodes open and $n comes flying into the room!",victim,0,0,TO_ROOM);
        }
        else
        {
        act("$n comes flying into the room with incredible force!",victim,0,0,TO_ROOM);
        }

        if (saves_spell(ch->level,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_roundhouse,DAM_BASH,TRUE,0);
        WAIT_STATE(victim,36);
        WAIT_STATE(ch,24);
        stop_fighting( ch, TRUE );
        return;
    }

    if( attempt == 6 )
    {
	dam += dice(level,4);
	act("You spin your leg viscously at $N and kick $M brutally into a wall!",ch,0,victim,TO_CHAR);
    	act("$n spins his leg viscously at $N and kicks $M brutally into a wall!",ch,0,victim,TO_NOTVICT);
    	act("$n spins his leg viscously at you and kicks you brutally into a wall!",ch,0,victim,TO_VICT);

        if (saves_spell(ch->level,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_roundhouse,DAM_BASH,TRUE,0);
        WAIT_STATE(ch,24);
        WAIT_STATE(victim,16);
    }
   }
   else
   {
    act("You spin your leg viscously at $N and kick $M brutally!",ch,0,victim,TO_CHAR);
    act("$n spins his leg viscously at $N and kicks $M brutally!",ch,0,victim,TO_NOTVICT);
    act("$n spins his leg viscously at you and kicks you brutally!",ch,0,victim,TO_VICT);

        if (saves_spell(ch->level-4,victim,DAM_BASH))
                dam /= 2;
        damage(ch,victim,dam,gsn_roundhouse,DAM_BASH,TRUE,0);

        if(number_range(1,2) == 2)
        WAIT_STATE(victim,16);
        else
        WAIT_STATE(victim,12);

        WAIT_STATE(ch,16);

   }
   return; 
}

void do_fists(CHAR_DATA *ch, char *argument)
{
  AFFECT_DATA af;
  int sn = -1;
  char arg[MSL/10];
  bool affected = FALSE;

  one_argument(argument,arg);

  if (IS_NPC(ch) || !has_skill(ch,sn))
  {
	send_to_char("Huh?\n\r",ch);
	return;
  }

  if(is_affected(ch,gsn_fists_fury) || is_affected(ch,gsn_fists_flame) ||
  is_affected(ch,gsn_fists_ice) || is_affected(ch,gsn_fists_fang) ||
  is_affected(ch,gsn_fists_acid) || is_affected(ch,gsn_fists_divinity) ||
  is_affected(ch,gsn_fists_claw) || is_affected(ch,gsn_fists_darkness) ||
  is_affected(ch,gsn_fists_liquid) )
   affected = TRUE;

  if(affected && !str_cmp(arg,"none"))
  {
  	if(is_affected(ch,gsn_fists_fury))
  		sn = gsn_fists_fury;	
  	else if(is_affected(ch,gsn_fists_flame))	
  		sn = gsn_fists_flame;
	else if(is_affected(ch,gsn_fists_ice))	  		
  		sn = gsn_fists_ice;
  	else if(is_affected(ch,gsn_fists_acid))	
  		sn = gsn_fists_acid;
  	else if(is_affected(ch,gsn_fists_claw))	
  		sn = gsn_fists_claw;
  	else if(is_affected(ch,gsn_fists_liquid))	
  		sn = gsn_fists_liquid;
  	else if(is_affected(ch,gsn_fists_fang))	
  		sn = gsn_fists_fang;
  	else if(is_affected(ch,gsn_fists_divinity))	
  		sn = gsn_fists_divinity;
  	else if(is_affected(ch,gsn_fists_darkness))	
  		sn = gsn_fists_darkness;
  		
  	if(sn != -1)
  	{	
		send_to_char( skill_table[sn].msg_off, ch );
		send_to_char( "\n\r", ch );  		
		affect_strip(ch,sn);			
	}
  	return;
  }
  else if (affected)
  {
        send_to_char("You are already concentrating on your fighting technique!\n\r",ch);
	return;
  }

  if(!str_cmp(arg,"flame"))
   sn = gsn_fists_flame;
  else if(!str_cmp(arg,"ice"))
   sn = gsn_fists_ice;
  else if(!str_cmp(arg,"acid"))
   sn = gsn_fists_acid;
  else if(!str_cmp(arg,"darkness"))
   sn = gsn_fists_darkness;
  else if(!str_cmp(arg,"fang"))
   sn = gsn_fists_fang;
  else if(!str_cmp(arg,"claw"))
   sn = gsn_fists_claw;
  else if(!str_cmp(arg,"liquid"))
   sn = gsn_fists_liquid;
  else if(!str_cmp(arg,"fury"))
   sn = gsn_fists_fury;
  else if(!str_cmp(arg,"divinity"))
   sn = gsn_fists_divinity;

  if(sn == -1)
  {
	send_to_char("You clench your fists to no avail.\n\r",ch);
	return;
  }

  if (number_percent() > get_skill(ch,sn))
  {
        send_to_char("You fail to focus on your fighting technique.\n\r",ch);
	check_improve(ch,sn,FALSE,1);
	return;
  }
  send_to_char("You focus on your fighting technique!\n\r",ch);

  check_improve(ch,sn,TRUE,1);

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = ch->level;
  af.duration  = ch->level/10;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = 0;
  affect_to_char( ch, &af );

  if (sn == gsn_fists_fury)
  {
        af.duration = ch->level/10;
        af.modifier = ch->level/4 + str_app[get_curr_stat(ch,STAT_STR)].todam;
        af.location = APPLY_DAMROLL;
	affect_to_char(ch,&af);
	//af.modifier = ch->level/4 + str_app[get_curr_stat(ch,STAT_STR)].tohit;
	//af.location = APPLY_HITROLL;
        //affect_to_char( ch, &af );
  }

  return;
}

void do_monk_nerve(CHAR_DATA *ch,char *argument) 
{ 
    char buf[MAX_STRING_LENGTH]; 
    CHAR_DATA *victim; 
    char arg[MAX_INPUT_LENGTH]; 
    AFFECT_DATA af; 
    int chance; 
     
    if ( (chance = get_skill(ch,gsn_nerve)) == 0 
    || !has_skill(ch,gsn_nerve)) 
    { 
        send_to_char("You aren't trained in the use of pressure points.\n\r",ch); 
        return; 
    } 
 
    one_argument(argument,arg); 
    if (arg[0] == '\0') 
        victim = ch->fighting; 
    else 
        victim = get_char_room(ch,arg); 
 
    if (victim == NULL) 
    { 
        send_to_char("Place pressure on who?\n\r",ch); 
        return; 
    } 
    
    if (victim == ch) 
    { 
        send_to_char("You can't do that.\n\r",ch); 
        return; 
    } 
    
    if (ch->stunned > 0)
    {
	send_to_char ("You're still a little woozy.\n\r", ch);
	return;
    }
    
    if (is_safe(ch,victim)) 
        return; 
 
    if (is_affected(victim,gsn_nerve)) 
    { 
        send_to_char("They have already been weakened.\n\r",ch);  
        return; 
    } 
       
    chance += (ch->level - victim->level)*2; 
    chance -= get_curr_stat(victim,STAT_DEX)/3; 
    chance += get_curr_stat(ch,STAT_DEX)/2; 
    chance -= get_curr_stat(victim,STAT_CON)/3;
    chance += get_curr_stat(ch,STAT_CON)/2;
    if(IS_NPC(victim))
          chance+=(victim->level/5);
    if (number_percent() > chance) 
    { 
        act("$n grasps $N's neck but fails to weaken them.",ch,0,victim,TO_NOTVICT); 
        act("You grasp $N's neck but fail to weaken them.",ch,0,victim,TO_CHAR); 
        act("$n grasps your neck but fails to weaken you.",ch,0,victim,TO_VICT); 
        check_improve(ch,gsn_nerve,FALSE,3); 
        WAIT_STATE(ch,PULSE_VIOLENCE); 
        return; 
    } 
    else 
    { 
    act("$n grasps $N's neck and weakens $m with pressure points.",ch,0,victim,TO_NOTVICT); 
    act("You grasp $N's neck and weaken $m with pressure points.",ch,0,victim,TO_CHAR); 
    act("$n grasps your neck and weakens you with pressure point.",ch,0,victim,TO_VICT); 
    check_improve(ch,gsn_nerve,TRUE,3); 
    af.where = TO_AFFECTS; 
    af.type = gsn_nerve; 
    af.location = APPLY_STR; 
    af.duration = 1; 
    af.bitvector = 0; 
    af.modifier = -3; 
    af.level = ch->level; 
    affect_to_char(victim,&af); 
    WAIT_STATE(ch,PULSE_VIOLENCE);
    }
     
        if (!IS_NPC(ch) && !IS_NPC(victim) 
        && (ch->fighting == NULL 
        || victim->fighting == NULL) ) 
        { 
         sprintf(buf,"Help, %s is attacking me!",PERS(ch,victim)); 
         do_yell(victim,buf); 
        } 
        if (victim->fighting == NULL) 
        { 
         multi_hit(victim,ch,TYPE_UNDEFINED); 
        } 
                 
    return; 
}

void do_chop( CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA *victim; 
        int chance, dam; 
        char arg[MAX_INPUT_LENGTH]; 
        OBJ_DATA *wield,*second; 
        int multiplier; 
        bool DUAL = FALSE; 
         
        dam = 0; 
        one_argument(argument,arg); 
        if ((chance = get_skill(ch,gsn_chop) ) == 0 
        || !has_skill(ch,gsn_chop) ) 
        { 
        send_to_char("Chopping? What's that?\n\r",ch); 
        return; 
        } 
        if (arg[0] == '\0') 
        { 
        victim = ch->fighting; 
         if (victim == NULL) 
         { 
            send_to_char("But you aren't fighting anyone!\n\r",ch); 
            return; 
         } 
        } 
        else if ( (victim = get_char_room(ch,arg)) == NULL ) 
        { 
                send_to_char("They aren't here.\n\r",ch); 
                return; 
        } 
        if (victim == ch) 
        { 
        send_to_char("You can't chop at yourself!\n\r",ch); 
        return; 
        }

	if (IS_AFFECTED(ch,AFF_FEAR) && (number_percent() <= 25)) {
		act("You attempt to chop, but begin crying instead.",ch,NULL,NULL,TO_CHAR);
		act("$n attempts to chop, but begins crying instead.",ch,NULL,NULL,TO_ROOM);
		WAIT_STATE(ch,DEF_FEAR_WAIT);
		return; }

        if(ch->stunned)
        {
         send_to_char("You are still a bit woozy.\n\r",ch);
         return;
        }
     
        wield = get_eq_char(ch,WEAR_WIELD); 
        second = get_eq_char(ch,WEAR_SECONDARY); 
 
        if ((wield == NULL) 
        || (( wield->value[0] != WEAPON_SWORD) 
        && ( wield->value[0] != WEAPON_AXE) 
        && ( wield->value[0] != WEAPON_DAGGER) )) 
        { 
         if ((second == NULL) 
         || (( second->value[0] != WEAPON_SWORD) 
         && ( second->value[0] != WEAPON_AXE) 
         && ( second->value[0] != WEAPON_DAGGER) )) 
         { 
          send_to_char("You must be wielding a sword or axe to chop.\n\r",ch); 
          return; 
         } 
         else 
         { 
         DUAL = TRUE; 
         } 
        } 
 
        if (is_safe(ch,victim) ) 
        return; 
 
        chance += ch->carry_weight/25; 
        chance -= victim->carry_weight/20; 
        chance += (ch->size - victim->size)*20; 
        chance -= get_curr_stat(victim,STAT_DEX); 
        chance += get_curr_stat(ch,STAT_STR)/3; 
        chance += get_curr_stat(ch,STAT_DEX)/2; 
        if (IS_AFFECTED(ch,AFF_HASTE) ) 
                chance += 10; 
        if (IS_AFFECTED(victim,AFF_HASTE) ) 
                chance -= 10; 
        chance += (ch->level - victim->level);

        act("$n attempts to split open $N with a quick chop!",ch,0,victim,TO_NOTVICT); 
        act("You attempt to split open $N with a quick chop!",ch,0,victim,TO_CHAR); 
        act("$n attempts to split you open with a quick chop!",ch,0,victim,TO_VICT); 
 
        if (number_percent() < chance) 
        { 
        check_improve(ch,gsn_lunge,TRUE,1); 
        WAIT_STATE(ch,skill_table[gsn_chop].beats); 
 
        if(DUAL) 
        wield = second; 
 
        if (wield->pIndexData->new_format) 
                dam = dice(wield->value[1],wield->value[2]); 
        else 
                dam = number_range(wield->value[1],wield->value[2]); 
 
        if (get_skill(ch,gsn_enhanced_damage) > 0 ) 
        { 
         if (number_percent() <= get_skill(ch,gsn_enhanced_damage) ) 
         { 
           check_improve(ch,gsn_enhanced_damage,TRUE,1); 
           dam += dam * (number_range(50,100)/100) * ch->pcdata->learned[gsn_enhanced_damage]/100; 
         } 
        } 
 
        dam += GET_DAMROLL(ch); 
        dam *= ch->pcdata->learned[gsn_chop]; 
        dam /= 100; 
        multiplier = number_range(ch->level/8, ch->level/4); 
        multiplier /= 10; 
        multiplier += 5/4; 
        dam *= multiplier;
        if(!DUAL && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))
         dam *= 1.5;
 
        if (dam <= 0) 
                dam = 1; 
        damage(ch,victim,dam,gsn_chop,DAM_PIERCE,TRUE,0); 
        } 
        else 
        { 
        damage(ch,victim,dam,gsn_chop,DAM_PIERCE,TRUE,0); 
        check_improve(ch,gsn_chop,FALSE,1); 
        WAIT_STATE(ch,skill_table[gsn_chop].beats); 
        } 
        return; 
} 

/*  void do_crit_strike(CHAR_DATA *ch, char *argument)
{
        int chance;

        if (is_affected (ch, gsn_crit_strike))
	{
                send_to_char ("{.Your fury begins to subside.{x\n\r", ch);
		affect_strip(ch, gsn_crit_strike);
                return;
	}

        if( ch->mana < 100 )
        {
         send_to_char("You do not have the willpower to do this. Gather your energy.\n\r",ch);
         return;
        }

        if (is_affected (ch, gsn_crit_strike))
        {
                send_to_char("{.Your fury begins to subside.{x\n\r",ch);
                affect_strip(ch, gsn_crit_strike);
        }

	if (number_percent () < chance)
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
                ch->mana -= 100;

                send_to_char ("{.You focus on crital striking!{x\n\r",ch);
                send_to_char ("{.You feel the {!Wrath {xof The {&Storm{x enter your body!{x\n\r", ch);
                act ("{($n concentration increases{x", ch, NULL, NULL, TO_ROOM);
                act ("{($n's eyes turn into shadows.{x", ch, NULL, NULL, TO_ROOM);


		af.where = TO_AFFECTS;
                af.type = gsn_crit_strike;
		af.level = ch->level;
                af.duration = -1;
                af.bitvector = 0;

		af.location = APPLY_DAMROLL;
                af.modifier = UMAX (1, ch->level / 5);
		affect_to_char (ch, &af);
	}

        return;
}
*/
