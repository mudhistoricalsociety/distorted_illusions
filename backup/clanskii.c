/* Tien
        if(ch->clan != clan_lookup("renshai"))
        {
         send_to_char("Huh?\n\r",ch);
         return;
        }
*/

#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#include <unistd.h>		
#include <sys/time.h>
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "merc.h"
#include "clan.h"
#include "magic.h"

 void do_crit_strike(CHAR_DATA *ch, char *argument)
{

        if(ch->clan != clan_lookup("storm"))
        {
         send_to_char("Huh?\n\r",ch);
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
	if (number_percent () )
	{
		AFFECT_DATA af;

		WAIT_STATE (ch, PULSE_VIOLENCE);
              ch->mana -= 100;

              send_to_char ("{.You focus on the way of the storm!{x\n\r",ch);
              send_to_char ("{.You feel the {!Wrath {xof The {&Storm{x enter your body!{x\n\r", ch);
              act ("{($n concentration increases{x", ch, NULL, NULL, TO_ROOM);
              act ("{($n's eyes turn into shadows.{x", ch, NULL, NULL, TO_ROOM);

               /* check_improve (ch, gsn_crit_strike, TRUE, 2); */

		af.where = TO_AFFECTS;
               af.type = gsn_crit_strike;
		af.level = ch->level;
              af.duration = -1;
                af.bitvector = 0;

		af.location = APPLY_AC;
               af.modifier = -38;
		affect_to_char (ch, &af);
                 

	}

        return;
}
