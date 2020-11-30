/******** HEDIT ------ Modified Version ------ by Skyntil ********
*         ============ Asgardian Nightmare =============         *
************------------*****************-------------***********/ 

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "lookup.h"
#include "recycle.h"

#define HEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)
#define EDIT_HELP(ch, help)	( help = (HELP_DATA *) ch->desc->pEdit )

extern HELP_AREA * had_list;
void save_area_list args((void)); 

const struct olc_cmd_type hedit_table[] =
{
/*	{	command		function	}, */

	{	"keyword",	hedit_keyword	},
	{	"text",	hedit_text	},
	{	"level",	hedit_level	},
	{	"commands",	show_commands	},
	{	"delete",	hedit_delete	},
	{	"list",		hedit_list	},
	{	"show",		hedit_show	},
	{	"?",		show_help	},
        {       "create",       hedit_create    },
        {       "newfile",      hedit_newfile   },

	{	NULL,		0		}
};

HELP_AREA * get_help_area( HELP_DATA *help )
{
        HELP_AREA * area;
        HELP_DATA * thishelp;

        /* Scroll though Help_area_data list */
        for ( area = had_list; area; area = area->next )
        {
          /* Scroll Through Helps */
          for ( thishelp = area->first; thishelp; thishelp = thishelp->next_area )
          {
            if ( thishelp == help )
                return area;
          }
        }
	return NULL;
}

HEDIT(hedit_show)
{
	HELP_DATA * help;
	char buf[MSL*2];

	EDIT_HELP(ch, help);

	sprintf( buf, "Keyword : [%s]\n\r"
				"Level   : [%d]\n\r"
				"Text   :\n\r"
				"%s-FIN-\n\r",
				help->keyword,
				help->level,
				help->text );
	send_to_char( buf, ch );

	return FALSE;
}

HEDIT(hedit_level)
{
	HELP_DATA *help;
	int lev;

	EDIT_HELP(ch, help);

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char( "Syntax : Level [-1..MAX_LEVEL]\n\r", ch );
		return FALSE;
	}

	lev = atoi(argument);

	if ( lev < -1 || lev > MAX_LEVEL )
	{
		printf_to_char( ch, "HEdit : Level must be between -1 and %d.\n\r", MAX_LEVEL );
		return FALSE;
	}

	help->level = lev;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT(hedit_keyword)
{
	HELP_DATA *help;

	EDIT_HELP(ch, help);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax : keyword [keywords]\n\r", ch );
		return FALSE;
	}

	free_string(help->keyword);
	help->keyword = str_dup(argument);

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT( hedit_text )
{
	HELP_DATA *help;

	EDIT_HELP(ch, help);

	if ( !IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax : text\n\r", ch );
		return FALSE;
	}

	string_append( ch, &help->text );

	return TRUE;
}

void hedit( CHAR_DATA *ch, char *argument)
{
	 HELP_DATA * pHelp;
	 HELP_AREA *had;
	 char arg[MAX_INPUT_LENGTH];
	 char command[MAX_INPUT_LENGTH];
	 int cmd;

	 smash_tilde(argument);
	 strcpy(arg, argument);
	 argument = one_argument( argument, command);

	 EDIT_HELP(ch, pHelp);

	 had = get_help_area(pHelp);

	 if (had == NULL)
	 {
		bugf( "hedit : help for %s is NULL", pHelp->keyword );
		edit_done(ch);
		return;
	 }

         if (ch->pcdata->security < 4)
	 {
                send_to_char("HEdit: Insufficient security to edit help.\n\r",ch);
                edit_done(ch);
                return;
	 }

	 if (command[0] == '\0')
	 {
                hedit_show(ch, argument);
		  return;
	 }

	 if (!str_cmp(command, "done") )
	 {
		  edit_done(ch);
		  return;
	 }

	 for (cmd = 0; hedit_table[cmd].name != NULL; cmd++)
	 {
                if (!str_prefix(command, hedit_table[cmd].name) )
                {
                        if ((*hedit_table[cmd].olc_fun) (ch, argument))
                                had->changed = TRUE;
                        return;
                }
	 }

	 interpret(ch, arg);
	 return;
}

void do_hedit(CHAR_DATA *ch, char *argument)
{
	HELP_DATA * pHelp;

	if ( IS_NPC(ch) )
		return;
    if (!oedit_check(ch))
    {
        send_to_char("OEdit: You must balance this eq before you are done.\n\r",ch);
        return;
    }

        if (argument[0] == '\0')
        {
         send_to_char("You must specify a help to edit.\n\r",ch);
         send_to_char("If in despair...type Hedit summary, then list all once in Hedit",ch);
         return;
        }

        if ( (pHelp = help_lookup( argument )) == NULL )
	{
		send_to_char( "HEdit : Help does not exist.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *) pHelp;
	ch->desc->editor	= ED_HELP;

	return;
}

HEDIT(hedit_delete)
{
	HELP_DATA * pHelp, * temp;
	HELP_AREA * had;
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	EDIT_HELP(ch, pHelp);

	for ( d = descriptor_list; d; d = d->next )
		if ( d->editor == ED_HELP && pHelp == (HELP_DATA *) d->pEdit )
			edit_done(d->character);

	if (help_first == pHelp)
		help_first = help_first->next;
	else
	{
		for ( temp = help_first; temp; temp = temp->next )
			if ( temp->next == pHelp )
				break;

		if ( !temp )
		{
			bugf( "hedit_delete : help %s not found in help_first", pHelp->keyword );
			return FALSE;
		}

		temp->next = pHelp->next;
	}

	for ( had = had_list; had; had = had->next )
		if ( pHelp == had->first )
		{
			found = TRUE;
			had->first = had->first->next_area;
		}
		else
		{
			for ( temp = had->first; temp; temp = temp->next_area )
				if ( temp->next_area == pHelp )
					break;

			if ( temp )
			{
				temp->next_area = pHelp->next_area;
				found = TRUE;
				break;
			}
		}

	if ( !found )
	{
		bugf( "hedit_delete : help %s not found in had_list", pHelp->keyword );
		return FALSE;
	}

	free_help(pHelp);

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT(hedit_list)
{
        char buf[MIL/10];
	int cnt = 0;
	HELP_DATA *pHelp;
	BUFFER *buffer;

	EDIT_HELP(ch, pHelp);

	if ( IS_NULLSTR(argument) )
	{
                send_to_char( "Syntax: list all\n\r", ch );
                send_to_char( "        list area\n\r", ch );
                send_to_char( "        list <file> (i.e. help.are)\n\r",ch);
		return FALSE;
	}

	if ( !str_cmp( argument, "all" ) )
	{
                buffer = new_buf();

		for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
		{
			sprintf( buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
				cnt % 4 == 3 ? "\n\r" : " " );

			add_buf( buffer, buf );
			cnt++;
		}

                if ( cnt % 4 != 0)
			add_buf( buffer, "\n\r" );

		page_to_char( buf_string(buffer), ch );
		return FALSE;
	}

	if ( !str_cmp( argument, "area" ) )
	{
		if ( ch->in_room->area->helps == NULL )
		{
			send_to_char( "There are no helps in this area.\n\r", ch );
			return FALSE;
		}

		buffer = new_buf();

		for ( pHelp = ch->in_room->area->helps->first; pHelp; pHelp = pHelp->next_area )
		{
			sprintf( buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
				cnt % 4 == 3 ? "\n\r" : " " );
			add_buf( buffer, buf );
			cnt++;
		}

		if ( cnt % 4 )
			add_buf( buffer, "\n\r" );

		page_to_char( buf_string(buffer), ch );
		return FALSE;
	}

     {
       HELP_AREA *had;
       char arg[MSL/4];

        one_argument(argument,arg);

        had = had_lookup(arg);

        if ( had )
	{
                if ( (pHelp = had->first) == NULL )
		{
                        send_to_char( "No helps in this file.\n\r", ch );
			return FALSE;
		}

		buffer = new_buf();

                for ( pHelp = had->first; pHelp; pHelp = pHelp->next_area )
		{
			sprintf( buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
				cnt % 4 == 3 ? "\n\r" : " " );
			add_buf( buffer, buf );
			cnt++;
		}

		if ( cnt % 4 )
			add_buf( buffer, "\n\r" );

		page_to_char( buf_string(buffer), ch );
		return FALSE;
	}
        else if(!had)
        {
         send_to_char("That is not a valid help-file(non-area)\n\r",ch);
         return FALSE;
        }
      }

	return FALSE;
}

HEDIT(hedit_create)
{
	char arg[MIL], fullarg[MIL];
        HELP_AREA *had;
	HELP_DATA *help;
	extern HELP_DATA *help_last;

	if ( IS_NULLSTR(argument) )
	{
                send_to_char( "Syntax : create [help area] [keyword]\n\r", ch );
		return FALSE;
	}

	strcpy( fullarg, argument );
        argument = one_argument( argument, arg ); /* argument now holds keyword */

        if ( !(had = had_lookup(arg)))
	{
         send_to_char("Help File not found.\n\r",ch);
         send_to_char("Choices are:\n\r",ch);

         for ( had = had_list; had; had = had->next )
         {
           if ( had->area == NULL )    
              printf_to_char( ch, "%s\n\r", had->filename ); 
         }

/*         had = had_lookup("rot.are");
         printf_to_char(ch,"%s\n\r",had->filename);
         had = had_lookup("help.are");
         printf_to_char(ch,"%s\n\r",had->filename);
         had = had_lookup("olc.hlp");
         printf_to_char(ch,"%s\n\r",had->filename); */

         send_to_char("*** END_LIST ***\n\r",ch);
         return TRUE;
	}
        else if(had)
        {
         printf_to_char(ch,"%s Found. Creating new Entry.\n\r",had->filename);
        }

        if (!had) 
	{
         send_to_char("You must specify a file to save this help in.\n\r",ch);
         return TRUE;
        }

	help		= new_help();
	help->level	= 0;
	help->keyword	= str_dup(argument);
	help->text	= str_dup( "" );

	if (help_last)
            help_last->next = help;

	if (help_first == NULL)
             help_first = help;

	help_last	= help;
	help->next	= NULL;

        if ( !had->first )
                had->first      = help;
        if ( !had->last )
                had->last       = help;

        had->last->next_area    = help;
        had->last               = help;
	help->next_area		= NULL;

	ch->desc->pEdit		= (HELP_DATA *) help;
	ch->desc->editor	= ED_HELP;

	send_to_char( "Ok.\n\r", ch );
	return FALSE;
}

HEDIT(hedit_newfile)
{
	char arg[MIL], fullarg[MIL];
        HELP_AREA *had = NULL,*ha;
	HELP_DATA *help;
	extern HELP_DATA *help_last;

        if(ch->pcdata->security < 8)
        {
         send_to_char("You need level 8 security to create new files.\n\r",ch);
         return TRUE;
        }

        if ( IS_NULLSTR(argument) )
	{
                send_to_char( "Syntax : newfile [name] Do not attach .are or .hlp to name.\n\r", ch );
		return FALSE;
	}

	strcpy( fullarg, argument );
        one_argument( argument, arg );

        for ( ha = had_list; ha; ha = ha->next )
        {
         if ( !str_cmp(arg, ha->filename) )
         continue;
         else
         {  
                had                     = new_had ();
                strcat(arg,".hlp");
                had->filename           = str_dup( arg ); 
                had->area               = NULL;
                had->next               = had_list;   
                had_list                = had;
                break;
         }
        }

        if(had)
        {
        printf_to_char(ch,"%s created.\n\r",had->filename);
        save_area_list();
        }

        if (!had)
	{
         send_to_char("Something is wrong, contact Skyntil.\n\r",ch);
         return TRUE;
        }

	help		= new_help();
	help->level	= 0;
	help->keyword	= str_dup(argument);
	help->text	= str_dup( "" );

	if (help_last)
            help_last->next = help;

	if (help_first == NULL)
             help_first = help;

	help_last	= help;
	help->next	= NULL;

        if ( !had->first )
                had->first      = help;
        if ( !had->last )
                had->last       = help;

        had->last->next_area    = help;
        had->last               = help;
	help->next_area		= NULL;

        top_help++;

	ch->desc->pEdit		= (HELP_DATA *) help;
	ch->desc->editor	= ED_HELP;

	send_to_char( "Ok.\n\r", ch );
        return FALSE;
}

