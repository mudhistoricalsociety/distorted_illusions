/* This file written for the use of Asgardian Nightmare by Gabe Volker */

#include "include.h"

/* Claim Ownership */
void do_emblazon(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *stone;

    one_argument( argument, arg );

    if (IS_NPC(ch))
    {
     send_to_char("Nice Try.\n\r",ch);
     return;
    }

    /* Restricts here */
    stone = get_eq_char(ch,WEAR_HOLD);

    if( stone == NULL)
    {
     send_to_char("You must be holding a Blood Stone.\n\r",ch);
     return;
    }

    if( stone->pIndexData->vnum != 9950 )
    {
     send_to_char("You must be holding a Blood Stone.\n\r",ch);
     return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "What object do you wish to emblazon?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry(ch, arg) ) == NULL )
	{
         send_to_char("You are not carrying that item.\n\r",ch);
         return;
        }

    if ( obj->owner != NULL && strlen(obj->owner) > 1 )
    {
        if (!str_cmp(ch->name,obj->owner))
            send_to_char("It already bears your mark!\n\r",ch);
	else
            send_to_char("Someone else owns this item.\n\r",ch);
	return;
    }

    /* Reductions here */
    obj_from_char(stone);

    if (obj->owner != NULL) free_string(obj->owner);
    obj->owner = str_dup(ch->name);
    act("The Blood Stone fuses into a fiery dagger.",ch,obj,NULL,TO_CHAR);
    act("You emblazon your mark onto $p.",ch,obj,NULL,TO_CHAR);
    act("$n writes runes onto $p with a fiery dagger.",ch,obj,NULL,TO_ROOM);
    return;
}

/* Disown */
void do_scratch(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_NPC(ch))
    {
     send_to_char("Nice Try.\n\r",ch);
     return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "What object do you wish to scratch your emblem off of?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry(ch, arg) ) == NULL )
	{
         send_to_char("You are not carrying that item.\n\r",ch);
         return;
        }

    if (obj->owner == NULL)
    {
       send_to_char("You do not own this item!\n\r",ch);
       return;
    }
    if (str_cmp(ch->name,obj->owner))
    {
       send_to_char("You do not own this item!\n\r",ch);
       return;
    }

    if (obj->owner != NULL) free_string(obj->owner);

    act("You scratch your mark off of $p.",ch,obj,NULL,TO_CHAR);
    act("$n scratches the marks of ownership off of $p.",ch,obj,NULL,TO_ROOM);
    return;
}

