#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "clan.h"
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

extern  NOTE_DATA *note_list;
extern  NOTE_DATA *idea_list;
extern  NOTE_DATA *news_list;
extern  NOTE_DATA *changes_list;
extern  NOTE_DATA *weddings_list;
extern  NOTE_DATA *quotes_list;


void col2html (char buf[MAX_INPUT_LENGTH*2], char arg[MAX_INPUT_LENGTH*2], int len)
{

  char buf2[22];
  char buf3[1];
  
  int i;
  for (i=0; i < MAX_INPUT_LENGTH*2; ++i)
    {
        buf[i]= 'G';
    }

  for (i=0; i < 22; ++i)
    {
        buf2[i] = 'G';
    }
    buf[0] = '\0';
    buf2[0] = '\0';

  for (i=0; i <= len; ++i) 
    {
      switch (arg[i])
        {
       default: buf3[0] = arg[i]; strcat(buf, buf3); break;
        {
        case '{':
          buf2[0] = '\0';
          i++;
       switch (arg[i])
       {
         
           {
                   case 'm':           
                     strcat(buf2,"<font color=\"#8B008B\">");
                     break;
                   case '5':
                     strcat(buf2,"<font color=\"#8B008B\">");
                     break;
                   case 'M':
                     strcat(buf2, "<font color=\"#FF00FF\">");
                     break;
                   case '%':
                     strcat(buf2, "<font color=\"#FF00FF\">");
                     break;
                   case 'r':
                     strcat(buf2, "<font color=\"#8B0000\">");
                     break;
                   case '1':
                     strcat(buf2, "<font color=\"#8B0000\">");
                     break;
                   case 'R':
                     strcat(buf2, "<font color=\"#FF0000\">");
                     break;
                   case '!':
                     strcat(buf2, "<font color=\"#FF0000\">");
                     break;
                   case 'g':
                     strcat(buf2, "<font color=\"#006400\">");
                     break;
                   case '2':
                     strcat(buf2, "<font color=\"#006400\">");
                     break;
                   case 'G':
                     strcat(buf2, "<font color=\"#00FF00\">");
                     break;
                   case '@':
                     strcat(buf2, "<font color=\"#00FF00\">");
                     break;
                   case 'c':
                     strcat(buf2, "<font color=\"#008B8B\">");
                     break;
                   case '6':
                     strcat(buf2, "<font color=\"#008B8B\">");
                     break;
                   case 'C':
                     strcat(buf2, "<font color=\"#00FFFF\">");
                     break;
                   case '^':
                     strcat(buf2, "<font color=\"#00FFFF\">");
                     break;
                   case 'y':
                     strcat(buf2, "<font color=\"#808000\">");
                     break;
                   case '3':
                     strcat(buf2, "<font color=\"#808000\">");
                     break;
                   case 'Y':
                     strcat(buf2, "<font color=\"#FFFF00\">");
                     break;
                   case '#':
                     strcat(buf2, "<font color=\"#FFFF00\">");
                     break;
                   case 'w':
                     strcat(buf2, "<font color=\"#808080\">");
                     break;
                   case '7':
                     strcat(buf2, "<font color=\"#808080\">");
                     break;
                   case 'W':
                     strcat(buf2, "<font color=\"#FFFFFF\">");
                     break;
                   case '&':
                     strcat(buf2, "<font color=\"#FFFFFF\">");
                     break;
                   case 'D':
                     strcat(buf2, "<font color=\"#636363\">");
                     break;
                   case '8':
                     strcat(buf2, "<font color=\"#636363\">");
                     break;
                   case '*':
                     strcat(buf2, "<font color=\"#636363\">");
                     break;
                   case 'b':
                     strcat(buf2, "<font color=\"#00008B\">");
                     break;
                   case '4':
                     strcat(buf2, "<font color=\"#00008B\">");
                     break;
                   case 'B':
                     strcat(buf2, "<font color=\"#0000FF\">");
                     break;
                   case '$':
                     strcat(buf2, "<font color=\"#0000FF\">");
                     break;
                   case '{':
                     strcat(buf2, "{");
                     break;
                   case 'x':
                     strcat(buf2, "<font color=\"#006400\">");
                     break;
          }
          default: strcat(buf2, "");break;
        }      
        strcat(buf, buf2);
        break;
      case '\0':
      buf3[0] = arg[i];
      strcat(buf, buf3);
      break;
      } 
  }
}
 return;
 }


void who_html_update (void)
{
/*  FILE *fp; 
  DESCRIPTOR_DATA *d;
  char buf[MAX_INPUT_LENGTH*2];
  char buf2[MAX_INPUT_LENGTH*2];
  char clan_name[MAX_INPUT_LENGTH*2];
  int most_ever = 0;
  int max_on = 0;
  int total = 0;
  int clan;
  buf[0] = '\0';
  buf2[0] = '\0';

  if ((fp = fopen(MOST_FILE,"r")) != NULL)
  {
          most_ever = fread_number(fp);
  }
  fclose(fp);

  if ((fp = fopen(MAX_FILE,"r")) != NULL)
  {
          max_on = fread_number(fp);
  }
  fclose(fp);

//  File Must Exist
  if ( !(fp = fopen( PLAYERS_HTML, "w") ))
  {
     bug( "players.html: fopen", 0 );
     perror( "players.html" );
     return;
  }
  else
  {
  fprintf(fp, "<html>\n");
  fprintf(fp, "<head>\n");
  fprintf(fp, "<title>\n");
  fprintf(fp, "Asgardian Nightmare - Player List\n");
  fprintf(fp, "</title>\n");

  // meta for refresh
  fprintf(fp, "<meta http-equiv=\"refresh\" content=\"15;players.html\">\n");

  fprintf(fp, "</head>\n");
  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

  fprintf(fp, "<h1><font color=\"#FFFFCC\">\n");

  fprintf(fp, "<center>Players On Asgardian Nightmare:</center>");

  fprintf(fp, "</font></h1>\n");
  
  fprintf(fp, "<pre>\n");

  fprintf(fp, "<font color=\"#FF00FF\"> [");
  fprintf(fp, "<font color=\"#00FFFF\">Lvl");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\"> Race ");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\">Class");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\">    Clan    ");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\">PK?");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\">Rank");
  fprintf(fp, "<font color=\"#FF00FF\">] [");
  fprintf(fp, "<font color=\"#00FFFF\">Name");
  fprintf(fp, "<font color=\"#FF00FF\">]\n");
  fprintf(fp, "<font color=\"#0000FF\">___________________________________________________________________________\n");
  
  for ( d = descriptor_list; d != NULL ; d = d->next )
  {
    CHAR_DATA *wch;
    char class[5];
    
    if ( d->connected != CON_PLAYING)
        continue;

    wch   = ( d->original != NULL ) ? d->original : d->character;
    class[0] = '\0';

    if (wch->invis_level || wch->incog_level || wch->ghost_level)
	continue;

    total++;

    if (!(wch->invis_level > LEVEL_HERO))
    {
      switch ( wch->level )
        {
          {
          case MAX_LEVEL - 0 : strcat(class, "{x[{&C{7RE{x]");    break;
          case MAX_LEVEL - 1 : strcat(class, "{x[{&I{7MP{x]");    break;
          case MAX_LEVEL - 2 : strcat(class, "{x[{!H{1BR{x]");    break;
          case MAX_LEVEL - 3 : strcat(class, "{x[{!S{1BR{x]");    break;
          case MAX_LEVEL - 4 : strcat(class, "{x[{!B{1DR{x]");    break;
          case MAX_LEVEL - 5 : strcat(class, "{x[{!J{1BR{x]");    break;
          case MAX_LEVEL - 6 : strcat(class, "{x[{$Q{4ST{x]");    break;
          case MAX_LEVEL - 7 : strcat(class, "{x[{^A{6DM{x]");    break;
          case MAX_LEVEL - 8 : strcat(class, "{x[{^J{6AD{x]");    break;
          }
        default: //strcat(class, "[PLR]");    break;

         if (wch->pcdata->tier == 1)
         {
          sprintf (class, "[{R%c{r%c%c{x]",
           class_table[wch->class].who_name[0],
           class_table[wch->class].who_name[1],
           class_table[wch->class].who_name[2]);
         }
         else if (wch->pcdata->tier == 2)
         {
          sprintf (class, "[{B%c{b%c%c{x]",
           class_table[wch->class].who_name[0],
           class_table[wch->class].who_name[1],
           class_table[wch->class].who_name[2]);
         }
         else if (wch->pcdata->tier >= 3)
         {
          sprintf (class, "[{G%c{g%c%c{x]",
           class_table[wch->class].who_name[0],
           class_table[wch->class].who_name[1],
           class_table[wch->class].who_name[2]);
         }
         break;

        }

      fprintf(fp, "<font color=\"#006400\"> [");
      fprintf(fp, "<font color=\"#00FF00\">");
      sprintf(buf, "%3d", wch->level);
      fprintf(fp, buf);
   if (wch->level <= 101) 
   { 
      fprintf(fp, "<font color=""#006400"">] [");
      fprintf(fp, "<font color=""#0000FF"">");

      buf2[0] = '\0';
      sprintf(buf2, "%6s", pc_race_table[wch->race].who_name);
      col2html(buf, buf2, strlen(buf2));
      fprintf(fp, buf);
   }
   else
   {
      fprintf(fp, "<font color=""#006400"">] [");
      fprintf(fp, "<font color=""#FFFFFF"">Staff ");
   }
   fprintf(fp, "<font color=""#006400"">]  ");

   buf2[0] = '\0';
   col2html(buf2, class, strlen(class));
   fprintf(fp, "%s", buf2);
   fprintf(fp, "  ");
   // Added Clan, PK or not, Arena Rank or PK rank depending on if PK or if NonPK 
   clan = wch->clan;
   if(clan_lookup(clan_table[clan].name) == -1 || clan == 0)
   {
       if (wch->level > LEVEL_HERO)
       {
         buf2[0] = '\0';
         sprintf(buf2, "{B -{WI{wmmortal{B- {x");
         col2html(buf, buf2, strlen(buf2));
         sprintf(clan_name, "%s", buf);
       }
       if (wch->level <= LEVEL_HERO)
       {
         buf2[0] = '\0';
         sprintf(buf2, "{W  ({yNon-PK{W)  {x");
         col2html(buf, buf2, strlen(buf2));
         sprintf(clan_name, "%s", buf);
       }
   }
   if (clan_lookup(clan_table[clan].name) > 0)
   { 
//     buf2[0] = '\0';
     sprintf(buf, "%s", clan_table[clan].who_name);
//     col2html(buf, buf2, strlen(buf2));
     sprintf(clan_name, "%s", buf);
   }
   fprintf(fp, "\[");
   fprintf(fp, "%s", clan_name);
   fprintf(fp, "] ");
   if (clan_table[clan].pkill)
        {
         buf2[0] = '\0';
         sprintf (buf2, "\[{!Yes{x] ");
         col2html(buf, buf2, strlen(buf2)); 
         fprintf(fp, "%s", buf);
         buf2[0] = '\0';
         sprintf (buf2, "\[{!%i{x] ", wch->pcdata->prank);
         col2html(buf, buf2, strlen(buf2)); 
         fprintf(fp, "%s", buf);
        }
   else if (clan == 0 || !clan_table[clan].pkill)
        {
         buf2[0] = '\0';
         sprintf (buf2, "\[{3No {x] ");
         col2html(buf, buf2, strlen(buf2)); 
         fprintf(fp, "%s", buf);
         buf2[0] = '\0';
         sprintf (buf2, "\[{3%i{x] ", wch->pcdata->arank);
         col2html(buf, buf2, strlen(buf2)); 
         fprintf(fp, "%s", buf);
        }

//      fprintf(fp, "<font color=""#00FF00"">");
      fprintf(fp, "<font color=""#FFFFCC"">");

    // Lets test this
      if (wch->pcdata->pretit != '\0') 
      {
        buf2[0] = '\0';
        sprintf(buf2, "%s", wch->pcdata->pretit );
        col2html(buf, buf2, strlen(buf2));
        fprintf(fp, "%s", buf );
      }
      fprintf(fp, "<font color=""#006400"">");
      fprintf(fp, " %s", wch->name);
//      buf2[0] = '\0';
      sprintf(buf, "%s", wch->pcdata->title );
//      col2html(buf, buf2, strlen(buf2));
      fprintf(fp, "%s", buf);
      fprintf(fp, "\n");
     }  
  }
  fprintf(fp, "<br>\n");
  fprintf(fp, "<font color=\"#0000FF\">Players found: ");
  fprintf(fp, "<font color=\"#FFFFFF\">");
  fprintf(fp, "%d", total);
  fprintf(fp, "<font color=\"#0000FF\">   Most on today: ");
  fprintf(fp, "<font color=\"#FFFFFF\">");
  fprintf(fp, "%d", max_on);
  fprintf(fp, "<font color=\"#0000FF\">   Most on ever: ");
  fprintf(fp, "<font color=\"#FFFFFF\">");
  fprintf(fp, "%d", most_ever);
  fprintf(fp, "<br>\n");
  
  fprintf(fp, "</font>\n");
  fprintf(fp, "</pre><br>\n");

  fprintf(fp, "<font color=\"#FFFFFF\" face=\"Times New Roman\">\n");
  fprintf(fp, "This page is updated every 15 seconds.\n");
  sprintf(buf, "This page last updated at %s Central Time.\n", ((char *) ctime( &current_time )));
  fprintf(fp, buf);

  fprintf(fp, "<br>\n");
  fprintf(fp, "<br>\n");
  fprintf(fp, "<hr>\n");

  fprintf(fp, "</p>\n");
  fprintf(fp, "</body>\n");
  fprintf(fp, "</html>\n");
  fclose( fp ); 
  } 
  buf[0] = '\0';
  buf2[0] = '\0';
  return;*/
}

void note_html_update()
{
/*  FILE *fp;
  char buf[MSL];
  char name[MIL]; 
  char *list_name;
  NOTE_DATA *pnote;
  NOTE_DATA **list;
  BUFFER *output;

  int vnum;
  int type;

    list = &note_list;
    list_name = "notes";
    type = NOTE_NOTE;

    vnum = 0;
    output = new_buf();

    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>Notes</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }

    list = &idea_list;
    list_name = "ideas";
    type = NOTE_IDEA;

    vnum = 0;
    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>Ideas</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"/note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }

    list = &news_list;
    list_name = "news";
    type = NOTE_NEWS;

    vnum = 0;
    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>News</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"/note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }

    list = &changes_list;
    list_name = "changes";
    type = NOTE_CHANGES;

    vnum = 0;
    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>Changes</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"/note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }

    list = &weddings_list;
    list_name = "weddings";
    type = NOTE_WEDDINGS;

    vnum = 0;
    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>Weddings</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"/note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }

    list = &quotes_list;
    list_name = "quotes";
    type = NOTE_QUOTES;

    vnum = 0;
    sprintf(buf, "<h2><font color=\"#FFFFCC\">\n");
    add_buf(output, buf);
    sprintf(buf, "<center>Quotes</center><BR>");
    add_buf(output, buf);
    sprintf(buf, "</font></h2>\n");  
    add_buf(output, buf);
    for ( pnote = *list; pnote != NULL; pnote = pnote->next )
    {
	if ( is_name( "all", pnote->to_list ) )
 	{
	    sprintf( buf, "[<A HREF = \"/note/%dn%d.html\">%3d</A>] %s: %s<BR>\n",
              vnum, type, vnum, pnote->sender, pnote->subject );
            add_buf(output,buf);
	    sprintf( name, "%s/note/%dn%d.html", WWW_DIR, vnum, type);

	    if ((fp = fopen( name ,"w")) != NULL)
	    {
		  fprintf(fp, "<html>\n");
		  fprintf(fp, "<head>\n");
		  fprintf(fp, "<title>\n");
		  fprintf(fp, "</title>\n");
		  fprintf(fp, "</head>\n");
		  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

                  sprintf( buf, "[%3d] %s: %s<BR>\n",vnum, pnote->sender, pnote->subject);
		  fprintf(fp, buf);
		  sprintf( buf, "%s<BR>\n", pnote->date);
		  fprintf(fp, buf);
		  sprintf( buf, "To: %s<BR>\n", pnote->to_list);
		  fprintf(fp, buf);
                  fprintf(fp, pnote->text);
		  fprintf(fp, "</body>\n");
		  fprintf(fp, "</html>\n");
	    }
	    fclose(fp);

            vnum++;
        }
    }
    
  if ( !(fp = fopen(NOTELIST_HTML, "w") ))
  {
     bug( "notelist.html: fopen", 0 );
     perror( "notelist.html" );
     return;
  }
  else
  {
  fprintf(fp, "<html>\n");
  fprintf(fp, "<head>\n");
  fprintf(fp, "<title>\n");
  fprintf(fp, "Note Boards\n");
  fprintf(fp, "</title>\n");

  fprintf(fp, "</head>\n");
  fprintf(fp, "<body text = \"#0000FF\" bgcolor=BLACK link=\"#FF0000\" vlink=\"#FFA500\" alink=\"#FFFF00\">\n ");

  fprintf(fp, "<h1><font color=\"#FFFFCC\">\n");
  fprintf(fp, "<center>Note Boards</center><BR>");
  fprintf(fp, "</font></h1>\n");  

  fprintf(fp, buf_string(output));
  free_buf(output);
  fprintf(fp, "</body>\n");
  fprintf(fp, "</html>\n");
  fclose( fp ); 
  } */
}

