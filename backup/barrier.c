/*
 * Block check for archmage barrier spell by Fesdor
 * Do not include in compiler, this is for fight.c and has already been put in.
 * This code is OUTDATED.  Use only for backup purposes should something horrific go wrong.
 */
bool check_barrier (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int chance;
	if (is_affected(victim,AFF_BARRIER))
	{

	if (!IS_AWAKE (victim))
		return FALSE;

        chance = 40;

        if (get_curr_stat (victim, STAT_CON) > 19)
        chance += 10;
   
        if (get_curr_stat (victim, STAT_INT) > 23)
        chance += 15;
        
        
    	chance -= (GET_HITROLL (ch) * 0.15);
		chance -= (get_curr_stat (ch, STAT_DEX) - get_curr_stat (victim, STAT_DEX)) * 2;
		chance -= (get_curr_stat (ch, STAT_INT) - get_curr_stat (victim, STAT_INT)) * 3;

	if(!IS_NPC(ch))
	{	
	chance += victim->level - ch->level;
	}
	else
	{
		chance += victim->level - (ch->level*2/3);
	}
	
	if (victim->stunned)
		chance=chance/4
			
	if (number_range(1,100) >= chance)
		return FALSE;
	else
        if(!IS_SET(victim->act,PLR_SHORT_COMBAT))
        act ("{)Your barrier blocks $n's attack.{x", ch, NULL, victim,
		 TO_VICT);
        if(!IS_SET(ch->act,PLR_SHORT_COMBAT))
        act ("{.$N's personal barrier blocks your attack!{x", ch, NULL, victim,
		 TO_CHAR);
	return TRUE;
	}
	else
	{
	return FALSE;
   }
}