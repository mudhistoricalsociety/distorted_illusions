#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

bool char_exists args(( bool backup, char *argument ));
bool    check_parse_name        args( ( char *name ) );

void do_pload( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],name[MAX_STRING_LENGTH/4];
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *in_room;
    bool fOld;

    if ( IS_NPC(ch) || ch->desc == NULL || ch->in_room == NULL ) return;

    if ( argument[0] == '\0' )
    {
   	send_to_char( "Syntax: pload <name>.\n\r", ch );
	   return;
    }

    if (!check_parse_name( argument ))
    {
	   send_to_char( "Thats an illegal name.\n\r", ch );
	   return;
    }

    if(get_char_world(ch,argument) != NULL)
    {
     send_to_char("You cannot load a person who is online.\n\r",ch);
     return;
    }

    if ( !char_exists(TRUE,argument) )
    {
	   send_to_char( "That player doesn't exist.\n\r", ch );
	   return;
    }

    argument[0] = UPPER(argument[0]);
    save_char_obj(ch);
    sprintf(buf,"You transform into %s.\n\r",argument);
    send_to_char(buf,ch);
    sprintf(buf,"$n transforms into %s.",argument);
    act(buf,ch,NULL,NULL,TO_ROOM);

    sprintf(name,"%s",ch->name);
   // send_to_char(name,ch);

    d = ch->desc; /* Assign Current desc to D */
    in_room = ch->in_room; /* in_room where you are originaly */
    extract_char(ch, TRUE); /* Remove your character */
    d->character = NULL;  /* Make your character point to nothing */

    fOld = load_char_obj( d, argument );  /* Load the new character into D */

    ch = d->character;  /* Character = New Character */
    ch->next = char_list;  /* Add ch to stack */
    char_list = ch;        /* Mark person as last in stack */
    char_to_room(ch,in_room); /* Put person in room where old char was */
    ch->pload = str_dup(name);
    sprintf(buf,"Your Pload is %s.\n\r",ch->pload);
    send_to_char(buf,ch);
    return;
}

void do_preturn( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool fOld;

    if(!(ch != NULL && ch->desc != NULL))
    {
     bug("Preturn, null char or desc",0);
     return;
    }

    if (IS_NPC(ch)) {send_to_char("Huh?\n\r",ch);return;}

    if (ch->pload == NULL) {send_to_char("Huh?\n\r",ch);return;}

    sprintf(arg,ch->pload);

    if (strlen(arg) < 3 || strlen(arg) > 10)
	{send_to_char("Huh?(STRLEN)\n\r",ch);return;}

//   if (!str_cmp(ch->pload,arg)) {send_to_char("Huh?\n\r",ch);return;}

    sprintf(buf,"You transform back into %s.\n\r",capitalize(ch->pload));
    send_to_char(buf,ch);
    sprintf(buf,"$n transforms back into %s.",capitalize(ch->pload));
    act(buf,ch,NULL,NULL,TO_ROOM);

    d = ch->desc;

    if (ch != NULL && ch->desc != NULL)
	  	 extract_char(ch,TRUE);
    else if (ch != NULL)
		 extract_char(ch,TRUE);

    if (ch->desc)
    	ch->desc->character = NULL;
/*
    ch->next = char_list;
    char_list = ch;
*/
    fOld = load_char_obj(d, capitalize(arg));

    ch = d->character;
    ch->next = char_list;
    char_list = ch;

    if (ch->in_room != NULL)
    	char_to_room(ch,ch->in_room);
    else
    	char_to_room(ch,get_room_index(3001));
    free_string(ch->pload);
    ch->pload = str_dup("");
    return;
}

bool char_exists( bool backup, char *argument )
{
    FILE *fp;
    char buf [MAX_STRING_LENGTH];
    bool found = FALSE;

    sprintf( buf, "%s%s%s%s", PLAYER_DIR, initial( argument ),
           "/", capitalize( argument ) );

    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
	found = TRUE;
	fclose( fp );
    }
    return found;
}

