#include <sys/types.h>
#include <sys/time.h> 
#include <malloc.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <assert.h> 
#include <ctype.h> 
#include <time.h> 
#include "merc.h" 
 
DECLARE_DO_FUN(do_look     ); 
 
#define LESSER(x,y)             (((x) < (y)) ? (x) : (y)) 
#define GREATER(x,y)    (((x) > (y)) ? (x) : (y)) 
 
#define TEAM_MAX                6 



typedef struct _challenge_t 
{ 
        struct _challenge_t     *next; 
        int                     id; 
        int                     green; 
        int                     flags;    /* CC_???, defined below */ 
        char            *team1[TEAM_MAX]; 
        char            *team2[TEAM_MAX]; 
        int                     status1[TEAM_MAX]; 
        int                     status2[TEAM_MAX]; 
 
} challenge_t; 
 
/* condition flags */ 
#define CC_NOPOTIONS            0x001 
#define CC_NOSCROLLS            0x002 
#define CC_NOWIMPY              0x004 
 
/* status flags */ 
#define CS_ACCEPT               0x001 
#define CS_NORECALL             0x002 
#define CS_AFK                  0x004 
#define CS_CURSE                0x008 
 
// initialize status to CS_INIT 
// challenge can proceed only when everyone's status is CS_GO 
#define CS_INIT                 0 
#define CS_GO                   CS_ACCEPT 
 
challenge_t *chal_head = (challenge_t *)NULL; 
challenge_t  *cur_chal = (challenge_t *)NULL; 
int          challenge_count = 0; 
 
int in_current_challenge( char *pname ); 
static int named_in_challenge( char *pname, challenge_t *pchal ); 
static challenge_t *find_challenge( int cnum ); 
static void cleanup_challenge( challenge_t *pchal ); 
static void arena_message( char *message ); 
static void challenge_message( char *message, challenge_t *pchal ); 
static char *challenge_string( challenge_t *pchal, char *msg, int reverse ); 
static char *challenge_flags( challenge_t *pchal ); 
static int player_challenges( char *pname ); 
static int challenge_update( challenge_t *pchal ); 
static void start_duel( challenge_t *pchal ); 
static void challenge_status( int cnum, CHAR_DATA *ch ); 
static int challenge_finished( challenge_t *pchal ); 
static void noncom_check( void ); 
void arena_update args( ( void ) ); 
 
struct _cond 
{ 
        char    *keyword; 
        int             bit; 
        char    *name; 
        char    *flag; 
}; 
 
struct _cond cond_table[] = { 
        {"np",  CC_NOPOTIONS, "no_potions",   "{^P{x"}, 
        {"ns",  CC_NOSCROLLS, "no_scrolls",   "{!S{x"}, 
        {"nw",  CC_NOWIMPY,   "no_wimpy",     "{$W{x"}, 
        {NULL,  0,            NULL,              '\0'}, 
}; 
 
/*=======================================================================* 
 * function: arena_update 
 * purpose: called every tick from update.c.  check on stuff 
 *=======================================================================*/ 
void arena_update( void ) 
{ 
        challenge_t             *pchal,*pnext; 
        char                    buf[MAX_STRING_LENGTH]; 
        int                             winner,i; // whichteam; 
        CHAR_DATA               *pplayer;
//		CHAR_DATA				*pplayercheck; 
        CHAR_DATA               *rch;  
 
        // update all the individual challenges 
        for (pchal = chal_head; pchal; pchal = pnext ) 
        { 
                pnext = pchal->next; 
                challenge_update( pchal ); 
        } 
 
        // scan the arena and boot mortal non-combatants therefrom 
        noncom_check(); 
 
        // if any challenge is running, see if it's done.  You can tell because 
        // one team will be entirely absent from rooms ARENA_LO to ARENA_HI. 
        if ((cur_chal) && ((winner = challenge_finished(cur_chal)))) 
        { 
                // tell everyone about it 
                sprintf( buf, "[{$ARENA{x] Challenge [%d]:  ", cur_chal->id ); 
                strcat( buf, challenge_string(cur_chal,"DEFEATED ", ((winner == 2)) )); 
                strcat( buf, "{x\n\r" ); 
                arena_message( buf ); 
 
                // send everyone to lounge 
                for( i = 0; i < (2*TEAM_MAX); i++ ) 
                { 
                        if (cur_chal->team1[i] == NULL) 
                                continue; 
 
                        pplayer = get_char_anyone( NULL,cur_chal->team1[i] ); 

 
                        if (pplayer == NULL) 
                                continue; 
         
                for ( rch = pplayer->in_room->people; rch != NULL; rch = rch->next_in_room )  
                {                                                                
                        if ( rch->fighting != NULL )  
                                {                                           
                                stop_fighting( rch, TRUE );                                  
                        }                                                                        
                }
//Dusk
                        affect_strip(pplayer,skill_lookup("cripple"));
                        affect_strip(pplayer,skill_lookup("siphon energy"));
                        affect_strip(pplayer,skill_lookup("siphon life")); 
//JLR
                        affect_strip(pplayer,skill_lookup("poison"));
                        affect_strip(pplayer,skill_lookup("chill touch"));
                        affect_strip(pplayer,skill_lookup("fire breath"));
                        affect_strip(pplayer,skill_lookup("slow"));
                        affect_strip(pplayer,skill_lookup("weaken"));
                        affect_strip(pplayer,skill_lookup("curse"));
                        affect_strip(pplayer,skill_lookup("wither"));
                        affect_strip(pplayer,skill_lookup("blindness"));
                        affect_strip(pplayer,skill_lookup("blindness dust"));
                        affect_strip(pplayer,skill_lookup("plague"));
						affect_strip(pplayer,skill_lookup("shriek"));
                        affect_strip(pplayer,skill_lookup("sleep"));
//Tien
                        affect_strip(pplayer,skill_lookup("feeble mind"));
                        affect_strip(pplayer,skill_lookup("dirt kick"));
                        affect_strip(pplayer,skill_lookup("faerie fire"));
                        affect_strip(pplayer,skill_lookup("headache"));
                        affect_strip(pplayer,skill_lookup("adamantium palm"));
                        affect_strip(pplayer,skill_lookup("call lightning"));
                        affect_strip(pplayer,skill_lookup("prismatic spray"));
                        affect_strip(pplayer,skill_lookup("gouge"));
						affect_strip(pplayer,skill_lookup("charm person"));
/* Stheno/Revye */
						affect_strip(pplayer,skill_lookup("nerve"));				
						affect_strip(pplayer,skill_lookup("garrote"));				
						affect_strip(pplayer,skill_lookup("fear aura"));				
						affect_strip(pplayer,skill_lookup("burning skin"));				
						affect_strip(pplayer,skill_lookup("smokebomb"));				
						affect_strip(pplayer,skill_lookup("dirt kicking"));				
						affect_strip(pplayer,skill_lookup("voodan curse"));
						affect_strip(pplayer,skill_lookup("sense vitality"));
						/* affect_strip(pplayer,skill_lookup(""));				
						*/
						
			
			pplayer->hit = pplayer->max_hit;
                        pplayer->mana = pplayer->max_mana;
                        pplayer->move = pplayer->max_move;

//Jair
					/* This is a hack job to get arena wins and losses working */
 /*					whichteam=0;
					

					for( i = 0; (pchal->team1[i]) && (i<TEAM_MAX); i++ ) 
        			{ 
                		pplayercheck = get_char_anyone( NULL,pchal->team1[i] );
						if (pplayercheck == pplayer)
							whichteam=1;
			        } 

					for( i = 0; (pchal->team1[i]) && (i<TEAM_MAX); i++ ) 
        			{ 
                		pplayercheck = get_char_anyone( NULL,pchal->team1[i] );
						if (pplayercheck == pplayer)
							whichteam=2;
			        } 

						if (whichteam == winner)
							pplayer->pcdata->awins += 1;
						else
							pplayer->pcdata->alosses += 1;

*/

                        char_from_room(pplayer); 
                        char_to_room(pplayer, get_room_index(ROOM_ARENA_LOUNGE)); 
                        act ("$n appears in the room.", pplayer, NULL, NULL, TO_ROOM); 
                        do_look(pplayer, "auto" );
                } 
 
                cleanup_challenge( cur_chal ); 
                cur_chal = (challenge_t *)NULL; 
        } 
 
        // if no challenge is going, look for a green one 
        if (!cur_chal) 
        { 
                for (pchal=chal_head;(pchal) && (!pchal->green);pchal = pchal->next); 
 
                if (pchal) 
                { 
                        // we have a green challenge.  do it 
                        start_duel( pchal ); 
                } 
        } 
} 
 
/*=======================================================================* 
 * function: start_duel 
 * purpose: begin a duel.  transport everyone to the arena, do everything. 
 *=======================================================================*/ 
static void start_duel( challenge_t *pchal ) 
{ 
        challenge_t             *pc; 
        CHAR_DATA               *pplayer; 
        char                    buf[MAX_STRING_LENGTH]; 
        int                             i,start_pos; 
        ROOM_INDEX_DATA *start1,*start2; 
 
        assert(pchal); 
 
        // take pchal off the challenge list 
        if (chal_head == pchal) 
        {       // first entry on list 
                chal_head = pchal->next; 
        } 
        else 
        { 
                for (pc = chal_head; (pc->next) && (pc->next != pchal); pc = pc->next); 
 
                if (pc->next == pchal) 
                { 
                        pc->next = pchal->next; 
                } 
                else 
                {       // not on the list.  Bogus.  Just return. 
                        return; 
                } 
        } 
 
        // write it to cur_chal 
        cur_chal = pchal; 
 
        // tell everyone about it 
        sprintf( buf, "[{$ARENA{x] Challenge [%d]%s:  ", pchal->id, 
                challenge_flags( pchal )); 
        strcat( buf, challenge_string(pchal,"VERSUS ",FALSE )); 
        strcat( buf, "{Rhas begun!{x\n\r\n\r" ); 
        arena_message( buf ); 
 
        start_pos = ROOM_ARENA_LO + 1 + (rand() % (ROOM_ARENA_HI-ROOM_ARENA_LO)); 
        start1 = get_room_index(start_pos); 
        start_pos = ROOM_ARENA_LO + 1 + (rand() % (ROOM_ARENA_HI-ROOM_ARENA_LO)); 
        start2 = get_room_index(start_pos); 
 
        if ((start1 == NULL) || (start2 == NULL)) 
        { 
                bug ("Arena doesn't exist.",0); 
                cleanup_challenge(pchal); 
                cur_chal = NULL; 
                return; 
        } 
 
        // here's where we'd send all the people to the arena and stuff 
        for( i = 0; (pchal->team1[i]) && (i<TEAM_MAX); i++ ) 
        { 
                pplayer = get_char_anyone( NULL,pchal->team1[i] ); 
//		pplayer_orig = pplayer;
                char_from_room(pplayer); 
                char_to_room(pplayer, start1); 
        affect_strip(pplayer,gsn_plague);
        affect_strip(pplayer,gsn_poison);
        affect_strip(pplayer,gsn_blindness);
        affect_strip(pplayer,gsn_sleep);
        affect_strip(pplayer,gsn_curse);
        pplayer->hit    = pplayer->max_hit;
        pplayer->mana   = pplayer->max_mana;
        pplayer->move   = pplayer->max_move;
        update_pos(pplayer);
        send_to_char("You have been Pre-Arena Restored.\n",pplayer);
                act ("$n appears in the room.", pplayer, NULL, NULL, TO_ROOM); 
                do_look(pplayer, "auto" ); 
//              send_to_char( "If the arena were working, you'd be sent to the arena " 
//                      "now.\n\r", pplayer ); 
        } 
 
        for( i = 0; (pchal->team2[i]) && (i<TEAM_MAX); i++ ) 
        { 
                pplayer = get_char_anyone( NULL,pchal->team2[i] ); 
//                pplayer_orig = pplayer;
                char_from_room(pplayer); 
                char_to_room(pplayer, start2); 
         affect_strip(pplayer,gsn_plague);
        affect_strip(pplayer,gsn_poison);
        affect_strip(pplayer,gsn_blindness);
        affect_strip(pplayer,gsn_sleep);
        affect_strip(pplayer,gsn_curse);
        pplayer->hit    = pplayer->max_hit;
        pplayer->mana   = pplayer->max_mana;
        pplayer->move   = pplayer->max_move;
        update_pos(pplayer);
        send_to_char("You have been Pre-Arena Restored.",pplayer);
                act ("$n appears in the room.", pplayer, NULL, NULL, TO_ROOM); 
                do_look(pplayer, "auto" ); 
 
//              send_to_char( "If the arena were working, you'd be sent to the arena " 
//                      "now.\n\r", pplayer ); 
        } 
} 
 
/*=======================================================================* 
 * function: do_challenge 
 * purpose: sends initial arena match query 
 *=======================================================================*/ 
void do_challenge(CHAR_DATA *ch, char *argument) 
{ 
        CHAR_DATA               *victim;  
        char                    buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH]; 
        int                             done = FALSE, i; 
        int                             team = 0; 
        int                             count = 0; 
        int                             reuse = FALSE; 
        int                             hilevel = 0, lowlevel = 0; 
        challenge_t             *pchal, *pc; 
 
        if(IS_NPC(ch)) 
                return;  
 
        if( arena == FIGHT_LOCK ) 
        { 
                send_to_char("Sorry, the arena is currently locked from use.\n\r",ch); 
                return; 
        }
        
        if(ch->hit < ch->max_hit*3/4)
        {
         send_to_char("You are not healthy enough to fight in the arena.\n\r",ch);
         return;
        }

         if (argument[0] == '{' && argument[1] == '\0')
                return;

	 if (argument[0] == '-')
		return;

	 if (argument[0] == '"')
		return;

        // challenge block? 
        if (!strcasecmp(argument, "block")) 
        { 
                if (IS_SET(ch->comm,COMM_NOARENA)) 
                { 
                        REMOVE_BIT(ch->comm,COMM_NOARENA); 
                        send_to_char("You will now receive arena updates and challenges.\n\r",ch); 
                } 
                else 
                { 
                        SET_BIT(ch->comm,COMM_NOARENA); 
                        send_to_char("You will no longer receive arena updates and challenges.\n\r",ch); 
                } 
 
                return; 
        } 
 
        // challenge list? 
        if (!strcasecmp(argument, "list")) 
        { 
                if (cur_chal) 
                { 
                        sprintf( buf, "Current challenge [%d]%s:  ", cur_chal->id, 
                                challenge_flags( cur_chal )); 
                        strcat(buf,challenge_string(cur_chal,"VERSUS ",FALSE)); 
                        strcat(buf,"\n\r"); 
                        send_to_char(buf,ch); 
                } 

                if (chal_head) 
                { 
                        for(pc=chal_head;pc;pc=pc->next) 
                        { 
                                sprintf( buf, "{%cChallenge [%d]%s:{x  ", 
                                        pc->green ? 'G' : 'R', pc->id, challenge_flags( pc )); 
                                strcat(buf,challenge_string(pc,"VERSUS ",FALSE)); 
                                strcat(buf,"\n\r"); 
                                send_to_char(buf,ch); 
                        } 
                } 
                else 
                        send_to_char("No pending challenges.\n\r",ch); 
                return; 
        } 
        // challenge status 
        if (!strncasecmp(argument, "status", 6 )) 
        { 
                argument = one_argument(argument,arg); 
                argument = one_argument(argument,arg); 
 
                if (arg[0] == 0) 
                { 
                        send_to_char("Give a challenge number.\n\r",ch); 
                        return; 
                } 
 
                challenge_status( atoi(arg), ch ); 
                return; 
 
        } 
 
 
        if ( IS_SET(ch->comm,COMM_NOARENA) ) 
        { 
                send_to_char("You're configured to ignore the arena.\n\r", ch ); 
                return; 
        } 

        if (ch->in_room->vnum == ROOM_VNUM_CORNER) 
                return; 

        if(argument[0] == '\0') 
        { 
                send_to_char("Usage: challenge [-conditions] <opponent1> [opponent2] ... [with] [teammate1] [teammate2] ....\n\r",ch); 
                return; 
        } 
 
        // so far so good.  allocate. 
        pchal = (challenge_t *)calloc(1L,sizeof(challenge_t));

        if (pchal == NULL) 
                return; 
 
        // write our name to the head of team1 
        pchal->team1[0] = strdup( ch->name ); 
        count = 0; 
 
        // set challeger accepted 
        pchal->status1[0] |= CS_ACCEPT; 
 
        // initialize hilevel and lowlevel 
        hilevel = ch->level; 
        lowlevel = ch->level; 
 
        // now peel off the names, one at a time 
        do 
        { 
                argument = one_argument(argument,arg); 
 
                // end of args? 
                if (arg[0] == 0) 
                { 
                        done = TRUE; 
                        continue; 
                } 
 
                // look for the team separator 
                if ((!strcmp(arg,"with")) && (team == 0)) 
                { 
                        if (count == 0) 
                        { 
                                send_to_char( "You need at least one player on the other t" 
                                        "eam.\n\r", ch ); 
                                cleanup_challenge( pchal ); 
                                return; 
                        } 
 
                        team++; 
                        count = 1; 
                        continue; 
                } 
 
                // condition flag? 
                if (arg[0] == '-') 
                { 
                        for (i=0; cond_table[i].keyword; i++) 
                        { 
                                if (!strcasecmp(&arg[1], cond_table[i].keyword)) 
                                        pchal->flags |= cond_table[i].bit; 
                        } 
 
                        continue; 
                } 
 
                // match name 
                victim = get_char_world(ch,arg); 
 
                // not found? 
                if (victim == NULL) 
                { 
                        sprintf( buf, "Bad target: %s\n\r", arg ); 
                        send_to_char( buf, ch ); 
                        cleanup_challenge(pchal); 
                        return; 
                } 
 
                // npc, immortal, or self?
                if(IS_NPC(victim) || victim == ch || victim == NULL) 
                { 
                        send_to_char("You cannot challenge NPC's, or yourself.\n\r",ch); 
                        cleanup_challenge(pchal); 
                        return; 
                }

                if(victim->hit < victim->max_hit*2/3)
                {
                 send_to_char("They are not healthy enough to fight in the arena.\n\r",ch);
                 return;
                }
                
                if( victim->fight_timer > 0 )
        	{
         		send_to_char("They cannot fight in the arena right now.\n\r",ch);
         		return;
        	}
 
                for (i=0; (pchal->team1[i]) && (i<TEAM_MAX); i++) 
                        if (!strcmp(victim->name,pchal->team1[i])) 
                                reuse = TRUE; 
 
                for (i=0; (pchal->team2[i]) && (i<TEAM_MAX); i++) 
                        if (!strcmp(victim->name,pchal->team2[i])) 
                                reuse = TRUE; 
 
                // already named? 
                if (reuse) 
                { 
                        send_to_char("Each player can be named only once in a given challenge.\n\r",ch); 
                        cleanup_challenge(pchal); 
                        return; 
                } 
 
                // noarena? 
                if ( IS_SET(victim->comm,COMM_NOARENA) ) 
                { 
                        sprintf( buf, "%s is blocking all challenges.\n\r", victim->name ); 
                        send_to_char( buf, ch ); 
                        cleanup_challenge( pchal ); 
                        return; 
                } 
 
                // named in too many? 
                if (player_challenges(victim->name) >= 4) 
                { 
                        sprintf( buf, "%s is already named in four challenges.  That's" 
                                        " the limit.\n\r", victim->name ); 
                        send_to_char(buf,ch); 
                        return; 
                } 
 
                // team full? 
                if (count >= TEAM_MAX) 
                { 
                        send_to_char("Team size limit exceeded.\n\r",ch); 
                        cleanup_challenge(pchal); 
                        return; 
                } 
 
                hilevel = GREATER(hilevel,victim->level); 
                lowlevel = LESSER(hilevel,victim->level); 
 
                // level range 
                if ((hilevel - lowlevel > 20) && (!IS_IMMORTAL(ch)))
                { 
                        send_to_char("All players in a challenge must be within 20 levels of one another.\n\r",ch); 
                        cleanup_challenge(pchal); 
                        return; 
                } 
 
                if (team) 
                { 
                        pchal->status1[count] = CS_INIT; 
                        pchal->team1[count++] = strdup( victim->name ); 
                } 
                else 
                { 
                        pchal->status2[count] = CS_INIT; 
                        pchal->team2[count++] = strdup( victim->name ); 
                } 
 
        } while ( !done ); 
 
        // valid challenge.  Move on. 
 
        // get an available challenge number 
        pchal->id = ++challenge_count; 
 
        // add challenge to the queue 
        if (chal_head) 
        { 
                // walk list, append to end 
                for (pc = chal_head; pc->next; pc = pc->next); 
                pc->next = pchal; 
        } 
        else 
        { 
                // no entries on list 
                chal_head = pchal; 
        } 
 
        // announce it 
        sprintf( buf, "[{$ARENA{x] Challenge [%d] issued!  ", pchal->id ); 
        strcat( buf, challenge_string(pchal,"VERSUS ",FALSE)); 
        strcat( buf, "\n\r" ); 
        strcat( buf, "Special Conditions:" ); 
        if (pchal->flags) 
        { 
                for( i = 0; cond_table[i].keyword; i++ ) 
                { 
                        if (pchal->flags & cond_table[i].bit) 
                        { 
                                strcat( buf, " " ); 
                                strcat( buf, cond_table[i].name ); 
                        } 
                } 
        } 
        else 
        { 
                strcat( buf, " None" ); 
        } 
        strcat( buf, "\n\r" ); 
        arena_message(buf); 
 
} 
 
/*=======================================================================* 
 * function: do_accept                                                   * 
 * purpose: to accept the arena match, and move the players to the arena * 
 *=======================================================================*/ 
void do_accept(CHAR_DATA *ch, char *argument) 
{ 
        int                     cnum, index; 
        char            buf[MAX_STRING_LENGTH]; 
        challenge_t     *pc; 
 
        if (argument[0] == '\0') 
        { 
                send_to_char("Accept challenges by number.\n\r",ch); 
                return; 
        } 
        
        if( ch->fight_timer > 0 )
        {
         	send_to_char("You cannot fight in the arena until your fight timer expires.\n\r",ch);
         	return;
        }
 
        cnum = atoi( argument ); 
 
        pc = find_challenge( cnum ); 
 
        if (!pc) 
        { 
                send_to_char("No such challenge.\n\r",ch); 
                return; 
        } 
 
        if (!(index = named_in_challenge( ch->name, pc ))) 
        { 
                send_to_char("You're not named in that challenge.\n\r",ch); 
                return; 
        } 
 
        // already accepted? 
        if (pc->status1[index-1] & CS_ACCEPT) 
        { 
                send_to_char("You've already accepted that challenge.\n\r", ch ); 
                return; 
        } 
 
        sprintf(buf,"[{$ARENA{x] Challenge %d accepted by %s.\n\r",cnum,ch->name); 
        arena_message(buf); 
        pc->status1[index-1] |= CS_ACCEPT; 
 
        // if this results in the challenge going green, do an update, so that 
        // the challenge may launch immediately. 
        if (challenge_update( pc )) 
        { 
                arena_update(); 
        } 
 
        return; 
} 
 
/*=======================================================================* 
 * function: do_decline                                                  * 
 * purpose: to chicken out from a sent arena challenge                   * 
 *=======================================================================*/ 
void do_decline(CHAR_DATA *ch, char *argument ) 
{ 
        int                     cnum; 
        char            buf[MAX_STRING_LENGTH]; 
        challenge_t     *pc; 
 
        if (argument[0] == '\0') 
        { 
                send_to_char("Decline challenges by number.\n\r",ch); 
                return; 
        } 
 
        cnum = atoi( argument ); 
 
        pc = find_challenge( cnum ); 
 
        if (!pc) 
        { 
                send_to_char("No such challenge.\n\r",ch); 
                return; 
        } 
 
        if ((!named_in_challenge( ch->name, pc )) && 
                (!IS_IMMORTAL(ch))) 
        { 
                send_to_char("You're not named in that challenge.\n\r",ch); 
                return; 
        } 
 
        sprintf(buf,"[{$ARENA{x] Challenge %d declined by %s.\n\r",cnum,ch->name); 
        arena_message(buf); 
        cleanup_challenge(pc); 
 
        return; 
} 
 
/*======================================================================* 
 * function: do_bet                                                     * 
 * purpose: to allow players to wager on the outcome of arena battles   * 
 *======================================================================*/ 
void do_bet(CHAR_DATA *ch, char *argument) 
{ 
        return;  
} 
 
/*=======================================================================* 
 * function: in_current_challenge 
 * purpose: returns 0 if player isn't in current challenge (or if there 
 * isn't one.)  Returns 1 if he's on team 1, 2 if he's on team 2 
 *=======================================================================*/ 
int in_current_challenge( char *pname ) 
{ 
        int                     index; 
 
        index = named_in_challenge( pname, cur_chal ); 
 
        if (index > TEAM_MAX) 
                return 2; 
 
        if (index) 
                return 1; 
 
        return 0; 
} 
 
/*=======================================================================* 
 * function: named_in_challenge                                          * 
 * purpose: returns TRUE if pname is named in challenge pchal            * 
 *=======================================================================*/ 
static int named_in_challenge( char *pname, challenge_t *pchal ) 
{ 
        int                     i; 
 
        if (pchal == (challenge_t *)NULL) 
                return 0; 
 
        for (i=0;(pchal->team1[i]) && (i<TEAM_MAX);i++) 
                if (!strcasecmp(pname,pchal->team1[i])) 
                        return i+1; 
 
        for (i=0;(pchal->team2[i]) && (i<TEAM_MAX);i++) 
                if (!strcasecmp(pname,pchal->team2[i])) 
                        return i+1+TEAM_MAX; 
 
        return FALSE; 
} 
 
/*=======================================================================* 
 * function: player_challenges 
 * purpose: counts the number of challenges in which a player is named 
 *=======================================================================*/ 
static int player_challenges( char *pname ) 
{ 
        challenge_t             *pc; 
        int                             count = 0; 
 
        for (pc=chal_head;pc;pc = pc->next) 
        { 
                if (named_in_challenge(pname, pc)) 
                        count++; 
        } 
 
        return count; 
} 
 
/*=======================================================================* 
 * function: find_challenge 
 * purpose: returns a pointer to the challenge with id cnum, NULL if none 
 *=======================================================================*/ 
static challenge_t *find_challenge( int cnum ) 
{ 
        challenge_t             *pc; 
 
        for (pc=chal_head;pc;pc = pc->next) 
                if (pc->id == cnum) 
                        return pc; 
 
        return (challenge_t *)NULL; 
} 
 
/*=======================================================================* 
 * function: arena_message 
 * purpose: send message to everyone who isn't NO_ARENA 
 *=======================================================================*/ 
 static void arena_message( char *message ) 
 { 
        DESCRIPTOR_DATA *d; 
 
        for (d=descriptor_list; d; d=d->next) 
        { 
                if ((d->connected == CON_PLAYING) && 
                        (!IS_SET(d->character->comm,COMM_NOARENA))) 
                        send_to_char(message,d->character); 
        } 
 } 
 
/*=======================================================================* 
 * function: challenge_message 
 * purpose: send message to everyone named in a given challenge 
 *=======================================================================*/ 
 static void challenge_message( char *message, challenge_t *pchal ) 
 { 
        int                     team,count; 
        CHAR_DATA       *pplayer; 
        char            **names; 
        int                     *status; 
 
        // loop through the players 
        for ( team = 0; team < 2; team++ ) 
        { 
                names = (team) ? pchal->team2 : pchal->team1; 
                status = (team) ? pchal->status2 : pchal->status1; 
 
                for (count = 0; (count < TEAM_MAX) && (names[count]); count++ ) 
                { 
                        // match name 
                        pplayer = get_char_anyone( NULL,names[count] ); 
 
                        // connected? 
                        if (pplayer != NULL) 
                                send_to_char( message, pplayer ); 
                } 
        } 
 } 
 
/*=======================================================================* 
 * function: challenge_status 
 * purpose: send status of challenge number cnum to player ch 
 *=======================================================================*/ 
static void challenge_status( int cnum, CHAR_DATA *ch ) 
{ 
        challenge_t     *pc; 
        char            buf[MAX_STRING_LENGTH]; 
        CHAR_DATA       *pplayer; 
        int                     i; 
 
        pc = find_challenge( cnum ); 
 
        if (pc == NULL) 
        { 
                send_to_char("No such challenge.\n\r", ch ); 
                return; 
        } 
 
        // print the title line 
        sprintf( buf, "{%cChallenge [%d]:{x  ", 
                pc->green ? 'G' : 'R', pc->id ); 
        strcat(buf,challenge_string(pc, "VERSUS ", FALSE)); 
        strcat(buf,"\n\r"); 
        strcat( buf, "Special Conditions:" ); 
        if (pc->flags) 
        { 
                for( i = 0; cond_table[i].keyword; i++ ) 
                { 
                        if (pc->flags & cond_table[i].bit) 
                        { 
                                strcat( buf, " " ); 
                                strcat( buf, cond_table[i].name ); 
                        } 
                } 
        } 
        else 
        { 
                strcat( buf, " None" ); 
        } 
        strcat( buf, "\n\r" ); 
        send_to_char(buf,ch); 
 
        if (pc->green) 
        { 
                send_to_char("Challenge is {GREADY{x.\n\r", ch ); 
                return; 
        } 
 
        send_to_char("Challenge is {RBLOCKED{x because:\n\r", ch ); 
 
        // now list each problem 
        for (i = 0; i < 2*TEAM_MAX; i++ ) 
        { 
                if (!(pc->team1[i])) 
                        continue; 
 
                // match the player 
                pplayer = get_char_anyone( NULL,pc->team1[i] ); 
 
                // player found? (should never happen) 
                if (pplayer == NULL) 
                { 
                        sprintf( buf, "> %s isn't connected.\n\r", pplayer->name ); 
                        send_to_char( buf, ch ); 
                } 
 
                // hasn't accepted? 
                if (!(pc->status1[i] & CS_ACCEPT)) 
                { 
                        sprintf( buf, "> %s hasn't accepted yet.\n\r", pplayer->name ); 
                        send_to_char( buf, ch ); 
                } 
 
                // cursed? 
                if (pc->status1[i] & CS_CURSE) 
                { 
                        sprintf( buf, "> %s hasn't accepted yet.\n\r", pplayer->name ); 
                        send_to_char( buf, ch ); 
                } 
 
                // afk? 
                if (pc->status1[i] & CS_AFK) 
                { 
                        sprintf( buf, "> %s is AFK.\n\r", pplayer->name ); 
                        send_to_char( buf, ch ); 
                } 
 
                // no-recall? 
                if (pc->status1[i] & CS_NORECALL) 
                { 
                        sprintf( buf, "> %s is in a no-recall area.\n\r", pplayer->name ); 
                        send_to_char( buf, ch ); 
                } 
        } 
} 
 
/*=======================================================================* 
 * function: cleanup_challenge 
 * purpose: frees all memory associated with a given challenge and 
 * removes it from the queue 
 *=======================================================================*/ 
static void cleanup_challenge( challenge_t *pchal ) 
{ 
	char			errmsg[MAX_STRING_LENGTH];
        int                             i; 
        challenge_t             *pc; 
        sprintf(errmsg,"Arena Check 3");
        log_string( errmsg );  
        // free up names 
        for (i=0;(pchal->team1[i]) && (i<TEAM_MAX);i++) 
        { 
                free( pchal->team1[i] ); 
                pchal->team1[i] = NULL; 
        } 
 
        for (i=0;(pchal->team2[i]) && (i<TEAM_MAX);i++) 
        { 
                free( pchal->team2[i] ); 
                pchal->team2[i] = NULL; 
        } 
 
        // take it off the list (if it's on) 
        if (chal_head == pchal) 
        {       // first entry on list 
                chal_head = pchal->next; 
        } 
        else if (chal_head) 
        { 
                for (pc = chal_head; (pc->next) && (pc->next != pchal); pc = pc->next); 
 
                if (pc->next == pchal) 
                        pc->next = pchal->next; 
        } 
 
        // now free the challenge itself 
        free(pchal); 
        return; 
} 
 
 
/*=======================================================================* 
 * function: challenge_string 
 * purpose: write a brief description of a challenge to a static buffer 
 * and return a pointer thereto. 
 *=======================================================================*/ 
static char *challenge_string( challenge_t *pchal, char *msg, int reverse ) 
{ 
        static char buf[MAX_STRING_LENGTH]; 
        int                     i; 
        char            **first, **second; 
 
        if (reverse) 
        { 
                first = pchal->team2; 
                second = pchal->team1; 
        } 
        else 
        { 
                first = pchal->team1; 
                second = pchal->team2; 
        } 
 
        buf[0] = '\0'; 
 
        // announce it 
        for( i = 0; (first[i]) && (i<TEAM_MAX); i++ ) 
        { 
                strcat(buf,first[i]); 
                strcat(buf," "); 
        } 
 
        strcat( buf, msg ); 
 
        for( i = 0; (second[i]) && (i<TEAM_MAX); i++ ) 
        { 
                strcat(buf,second[i]); 
                strcat(buf," "); 
        } 
 
        return buf; 
} 
 
/*=======================================================================* 
 * function: challenge_flags 
 * purpose: write a short string containing challenge flags 
 *=======================================================================*/ 
static char *challenge_flags( challenge_t *pchal ) 
{ 
        static char     buf[MAX_STRING_LENGTH]; 
        int                     i = 0,j; 
 
        sprintf( buf, "{x(" ); 
        for( j=0; cond_table[j].keyword; j++ ) 
        { 
                if (pchal->flags & cond_table[j].bit) 
                { 
                        strcat( buf, cond_table[j].flag ); 
                        i++; 
                } 
        } 
        strcat( buf, ")" ); 
 
        if (i == 0) 
                buf[0] = '\0'; 
 
        return buf; 
} 
 
/*=======================================================================* 
 * function: challenge_update 
 * purpose: updates the status bits of a given challenge 
 * returns TRUE if the challenge goes green as of this update 
 *=======================================================================*/ 
static int challenge_update( challenge_t *pchal ) 
{ 
        static char buf[MAX_STRING_LENGTH]; 
        int                     team,count; 
        CHAR_DATA       *pplayer; 
        char            **names; 
        int                     *status; 
        int                     green, retval = FALSE; 
 
        green = TRUE; 
 
        // loop through the players 
        for ( team = 0; team < 2; team++ ) 
        { 
                names = (team) ? pchal->team2 : pchal->team1; 
                status = (team) ? pchal->status2 : pchal->status1; 
 
                for (count = 0; (count < TEAM_MAX) && (names[count]); count++ ) 
                { 
                        // match name 
                        pplayer = get_char_anyone( NULL,names[count] ); 
 
                        // connected? 
                        if (pplayer == NULL) 
                        { 
                                sprintf( buf, "Challenge [%d] expires - %s has quit.\n", 
                                        pchal->id, names[count] ); 
                                challenge_message( buf, pchal ); 
                                cleanup_challenge(pchal); 
                                return FALSE; 
                        } 
 
                        // no recall? 
                        if (IS_SET(pplayer->in_room->room_flags,ROOM_NO_RECALL)) 
                                status[count] |= CS_NORECALL; 
                        else 
                                status[count] &= ~CS_NORECALL; 
 
                        // afk? 
                        if (IS_SET(pplayer->comm,COMM_AFK)) 
                                status[count] |= CS_AFK; 
                        else 
                                status[count] &= ~CS_AFK; 
 
                        // curse? 
                        if (IS_AFFECTED(pplayer,AFF_CURSE)) 
                                status[count] |= CS_CURSE; 
                        else 
                                status[count] &= ~CS_CURSE; 
 
                        // still green? 
                        if (status[count] != CS_GO) 
                                green = FALSE; 
                } 
        } 
 
        if (green != pchal->green) 
        { 
                sprintf( buf, "Challenge [%d] is now %s.\n", pchal->id, 
                        green ? "{GREADY{x" : "{RBLOCKED{x" ); 
                challenge_message( buf, pchal ); 
                if (green) 
                        retval = TRUE; 
        } 
 
        pchal->green = green; 
 
        return retval; 
} 
 
/*=======================================================================* 
 * function: challenge_finished 
 * purpose: returns 0 if challenge isn't done yet, 1 if team 1 won, 2 if 
 * team 2 won. 
 *=======================================================================*/ 
static int challenge_finished( challenge_t *pchal ) 
{ 
        int                     i,found; 
        CHAR_DATA       *pc; 
 
        found = FALSE; 
 
        // is team 1 entirely absent from rooms ARENA_LO to ARENA_HI? 
        for (i = 0; (pchal->team1[i]) && (i<TEAM_MAX) && (!found); i++) 
        { 
                pc = get_char_anyone( NULL,pchal->team1[i] ); 
 
                if ((pc) && (pc->in_room->vnum >= ROOM_ARENA_LO) && 
                        (pc->in_room->vnum <= ROOM_ARENA_HI)) 
                        found = TRUE; 
        } 
 
        if (!found) 
                return 2; 
         
        found = FALSE; 
 
        // is team 2 entirely absent from rooms ARENA_LO to ARENA_HI? 
        for (i = 0; (pchal->team2[i]) && (i<TEAM_MAX) && (!found); i++) 
        { 
                pc = get_char_anyone( NULL,pchal->team2[i] ); 
 
                if ((pc) && (pc->in_room->vnum >= ROOM_ARENA_LO) && 
                        (pc->in_room->vnum <= ROOM_ARENA_HI)) 
                        found = TRUE; 
        } 
 
        if (!found) 
                return 1; 
 
        return 0; 
         
} 
 
/*=======================================================================* 
 * function: noncom_check 
 * purpose: scan arena for mortals who shouldn't be there & remove them. 
 *=======================================================================*/ 
static void noncom_check( void ) 
{ 
        DESCRIPTOR_DATA *d; 
 
        for (d=descriptor_list; d; d=d->next) 
        { 
                // ignore them if they're disconnected 
                if (d->connected != CON_PLAYING) 
                        continue; 
 
                // ignore them if they're not in the arena 
                if ((d->character->in_room->vnum < ROOM_ARENA_LO) || 
                        (d->character->in_room->vnum > ROOM_ARENA_MORGUE)) 
                        continue; 
 
                // ignore if they're in the arena lounge 
                if (d->character->in_room->vnum == ROOM_ARENA_LOUNGE) 
                        continue; 
 
                // ignore them if they're named in the current challenge 
                if ((cur_chal) && (named_in_challenge(d->character->name,cur_chal))) 
                        continue; 
 
                // let immortals hang around if they want 
                if (IS_IMMORTAL(d->character)) 
                        continue; 
 
                // boot 'em to lounge 
                send_to_char("You've been auto-removed from the arena.\n\r", 
                        d->character ); 
                char_from_room(d->character); 
                char_to_room(d->character, get_room_index(ROOM_ARENA_LOUNGE)); 
                act ("$n appears in the room.", d->character, NULL, NULL, TO_ROOM); 
                do_look(d->character, "auto" ); 
 
        } 
 } 
 
/*=======================================================================* 
 * function: arena_can_quaff 
 * purpose: returns FALSE if ch is in arena during a challenge that forbids 
 * use of potions, true otherwise 
 *=======================================================================*/ 
int arena_can_quaff( CHAR_DATA *ch ) 
{ 
        if (!cur_chal) 
                return TRUE; 
 
        if (!IN_ARENA(ch)) 
                return TRUE; 
 
        if (!(cur_chal->flags & CC_NOPOTIONS)) 
                return TRUE; 
 
        send_to_char( "No potions may be used during this duel.\n\r", ch ); 
 
        return FALSE; 
} 
 
/*=======================================================================* 
 * function: arena_can_recite 
 * purpose: returns FALSE if ch is in arena during a challenge that forbids 
 * use of scrolls, true otherwise 
 *=======================================================================*/ 
int arena_can_recite( CHAR_DATA *ch ) 
{ 
        if (!cur_chal) 
                return TRUE; 
 
        if (!IN_ARENA(ch)) 
                return TRUE; 
 
        if (!(cur_chal->flags & CC_NOSCROLLS)) 
                return TRUE; 
 
        send_to_char( "No scrolls may be used during this duel.\n\r", ch ); 
 
        return FALSE; 
} 
 
/*=======================================================================* 
 * function: arena_can_wimpy 
 * purpose: returns FALSE if ch is in arena during a challenge that forbids 
 * use of wimpy, true otherwise 
 *=======================================================================*/ 
int arena_can_wimpy( CHAR_DATA *ch ) 
{ 
        if (!cur_chal) 
                return TRUE; 
 
        if (!IN_ARENA(ch)) 
                return TRUE; 
 
        if (!(cur_chal->flags & CC_NOWIMPY)) 
                return TRUE; 
 
        return FALSE; 
} 
 
/*=======================================================================*/ 
