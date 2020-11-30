#if defined(macintosh)
#include <types.h> 
#include <time.h> 
#else 
#include <sys/types.h> 
#include <sys/time.h> 
#endif 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include "merc.h" 
#include "clan.h"
#include "tables.h"
#include "olc.h"
 
extern char str_empty[1]; 
#define IS_VALID(data)          ((data) != NULL && (data)->valid) 
#define VALIDATE(data)          ((data)->valid = TRUE) 
#define INVALIDATE(data)        ((data)->valid = FALSE)
#define CEDIT( fun )           bool fun(CHAR_DATA *ch, char *argument)

bool CLANS_CHANGED = FALSE;

bool oedit_check (CHAR_DATA *ch)
{
    OBJ_INDEX_DATA *pObj;
    if (ch->desc->editor != ED_OBJECT )
     {
      return TRUE;
     }
    if (ch->desc->editor == ED_OBJECT )
    {
        pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
        if (!IS_TRUSTED(ch,CREATOR) && ch->pcdata->security < 9 && 
            (obj_balance(pObj) > atoi(MAX_OEDIT_BALANCE) || obj_balance(pObj) < atoi(MIN_OEDIT_BALANCE) ))
        { 
            return FALSE; 
        } 
        else if (!is_clan_obj_ind(pObj) && ch->pcdata->security < 5 &&
	    (obj_balance(pObj) > atoi(MAX_OEDIT_BALANCE) || obj_balance(pObj) < atoi(MIN_OEDIT_BALANCE) )) 
        { 
           return FALSE; 
        }
	else
	{
	   return TRUE;
	}
   }
  return FALSE;
}
const struct olc_cmd_type cedit_table[] =
{
/*      {       command         function          }, */

        {       "name",         cedit_name        },
        {       "whoname",      cedit_whoname     },
        {       "deathroom",    cedit_deathroom   },
        {       "recall",       cedit_recall      },
        {       "independent",  cedit_independent },
        {       "pkill",        cedit_pkill       },
        {       "list",         cedit_list        },
        {       "guard_vnum",   cedit_guard_vnum  },
        {       "guard_say",    cedit_guard_say   },
        {       "coclan",       cedit_coclan      },
        {       "clan_skill1",   cedit_clanskill1 },
        {       "clan_skill2",   cedit_clanskill2 },
        {       "clan_skill3",   cedit_clanskill3 },
        {       "clan_skill4",   cedit_clanskill4 },
        {       "clan_skill5",   cedit_clanskill5 },
        {       "clan_skill6",   cedit_clanskill6 },
        {       "show",         cedit_show        },
        {       "commands",     show_commands     },
        {       NULL,           0                 }
};


NEW_CLAN_DATA *new_clan_list[MAX_CLAN];
/* Members -- Just calculated from saved member on loadup.
   Needs to be updated when a person joins or is outcasted
   NOTE: This is only temporary....but we need it */
sh_int clan_members[MAX_CLAN];

void read_clan args((char *clanname, int position));
 
/* stuff for recycling new_clan structures */ 
NEW_CLAN_DATA *new_clan_free; 
NEW_CLAN_DATA * new_new_clan (void)
{ 
        static NEW_CLAN_DATA new_clan_zero; 
        NEW_CLAN_DATA *new_clan; 
 
        if (new_clan_free == NULL) 
        { 
                new_clan = alloc_perm (sizeof (*new_clan)); 
        } 
        else 
        { 
                new_clan = new_clan_free; 
                new_clan_free = new_clan_free->next; 
        } 
 
        *new_clan = new_clan_zero; 
        VALIDATE (new_clan); 
        new_clan->name = &str_empty[0]; 
        return new_clan; 
} 
 
void  
free_new_clan (NEW_CLAN_DATA * new_clan) 
{ 
        if (!IS_VALID (new_clan)) 
                return; 
 
        free_string (new_clan->name); 
        INVALIDATE (new_clan); 
 
        new_clan->next = new_clan_free; 
        new_clan_free = new_clan; 
} 

void save_clan_list(void)
{
 FILE *fp;
 char buf[MAX_INPUT_LENGTH];
 int clan;

 /* filename */
 sprintf (buf, "%s%s", CLAN_DIR, CLAN_FILE); 
 
 if ((fp = fopen (buf, "w")) == NULL) 
 { 
     perror (buf); 
 }

 for(clan = 1; clan < MAX_CLAN; clan++)
 {
  fprintf(fp,"%s\n",clan_table[clan].name);
 }
 fprintf(fp,"End\n");
 
 fclose (fp);
 return;
}

void save_new_clan (int new_clan) 
{ 
        FILE *fp; 
        char buf[MAX_INPUT_LENGTH]; 
        NEW_CLAN_DATA *out_list;
     //   CLN_DATA *cln_stats;
 
#ifdef DEBUG 
        Debug ("save_new_clan"); 
#endif 
        if (new_clan < 1 || new_clan >= MAX_CLAN) 
         return; 

        save_clan_list();

        /* filename */
        sprintf (buf, "%s%s.dat", CLAN_DIR, clan_table[new_clan].name); 
 
        if ((fp = fopen (buf, "w")) == NULL) 
        { 
                perror (buf); 
        }

          fprintf(fp, "Name %s~\n",clan_table[new_clan].name);
          fprintf(fp, "Who %s~\n",clan_table[new_clan].who_name); 
          fprintf(fp, "God %s~\n",clan_table[new_clan].god);      
          fprintf(fp, "Death %d\n",clan_table[new_clan].deathroom);
          fprintf(fp, "Recall %d\n",clan_table[new_clan].recall);
          fprintf(fp, "Ind %d\n",(clan_table[new_clan].independent == FALSE) ? 0 : 1);
          fprintf(fp, "Pkill %d\n",(clan_table[new_clan].pkill == FALSE) ? 0 : 1);
          fprintf(fp, "Kills %d\n",clan_table[new_clan].kills);
          fprintf(fp, "Deaths %d\n",clan_table[new_clan].deaths);
          fprintf(fp, "Cocln %d\n",clan_table[new_clan].coclan);
          /* New things */
          fprintf(fp, "GrdVnm %d\n",clan_table[new_clan].guardian_vnum);
          fprintf(fp, "GrdSay %s~\n",clan_table[new_clan].guardian_say);
          fprintf(fp, "Sk1 %s~\n", clan_table[new_clan].clan_skill1);
          fprintf(fp, "Sk2 %s~\n", clan_table[new_clan].clan_skill2);
          fprintf(fp, "Sk3 %s~\n", clan_table[new_clan].clan_skill3);
          fprintf(fp, "Sk4 %s~\n", clan_table[new_clan].clan_skill4);
          fprintf(fp, "Sk5 %s~\n", clan_table[new_clan].clan_skill5);
          fprintf(fp, "Sk6 %s~\n", clan_table[new_clan].clan_skill6);

          fprintf(fp,"Members~\n");

        for (out_list = new_clan_list[new_clan]; out_list != NULL; out_list = out_list->next) 
        { 
          fprintf (fp, "%d %s~\n", out_list->rank, out_list->name);
        } 

        fprintf (fp, "999");

    /*    for (cln_stats = clan_stat_list[new_clan]; cln_stats != NULL; cln_stats = cln_stats->next)
          {
           fprintf (fp,"Kills %d\n", cln_stats->kills);
           fprintf (fp,"Deaths %d\n", cln_stats->deaths);
           fprintf (fp,"War %d\n", print_flags(cln_stats->atwar));
          }

          fprintf(fp,"#!");
     */

        fclose (fp); 
}

void save_clans( void )
{
 int clan;
 char buf[MSL/10];
 for(clan = 1; clan < MAX_CLAN; clan++)
 {
  sprintf(buf,"Saving Clan %s", clan_table[clan].name);
  log_string( buf );
  save_new_clan( clan );
 }
 return;
}
 
void  
load_new_clans_list (void) 
{ 
        FILE *fp; 
        char buf[MAX_INPUT_LENGTH]; 
        char *word;
        int position;
        char name[MAX_CLAN][MAX_INPUT_LENGTH/4];
 
#ifdef DEBUG 
        Debug ("load_new_clans"); 
#endif 
        log_string ("Loading new_clans");

        /* Clan List */
        sprintf (buf, "%s%s", CLAN_DIR,CLAN_FILE);
 
        if ((fp = fopen (buf, "r")) == NULL) 
        { 
          perror(buf);
        }

  name[0][0] = '\0';
  position = 1;

  for( ; ; )
  {
   if (feof (fp))
   { 
     break; 
   }

   word = fread_word( fp );

   strcpy(name[position],word);

   if(!str_cmp(word,"End"))
   {
    break;
   }
     
   position++;
  }

  fclose (fp);

  for(position = 1; position < MAX_CLAN; position++)
  {
    read_clan(name[position],position);
  }
  return; 
}

void read_clan(char *clanname, int position)
{
  FILE *fp; 
  NEW_CLAN_DATA *in_list;
  int new_clan, temprank;
  char buf[MAX_INPUT_LENGTH];
  char *word;
  int number;
  // CLN_DATA *cln_in;

  new_clan = position;
  new_clan_list[new_clan] = NULL;
  clan_members[new_clan] = 0;
  // clan_stat_list[new_clan] == NULL;
  // cln_in = new_new_clan();

  sprintf (buf, "Loading %s",clanname);
  log_string (buf);

  /* Clan file */
  sprintf (buf, "%s%s.dat",CLAN_DIR,clanname);
 
  if ((fp = fopen (buf, "r")) == NULL) 
  { 
   perror(buf);
  }

     /* Now reading in the clan file */
     for( ; ; )
     {
      if (feof (fp))
       break; 
        
       word = fread_word( fp );

       if(!str_cmp(word,"Members~"))
       {
         break;
       }

       else if(!str_cmp(word,"Name"))
        clan_table[new_clan].name = fread_string( fp );
       else if(!str_cmp(word,"Who"))
        clan_table[new_clan].who_name = fread_string( fp );
       else if(!str_cmp(word,"God"))
        clan_table[new_clan].god = fread_string( fp );
       else if(!str_cmp(word,"Death"))
        clan_table[new_clan].deathroom = fread_number( fp );
       else if(!str_cmp(word,"Recall"))
        clan_table[new_clan].recall = fread_number( fp );
       else if(!str_cmp(word,"Ind"))
       {
        number = fread_number( fp );
        if(number == 0)
        clan_table[new_clan].independent = FALSE;
        else if(number == 1)
        clan_table[new_clan].independent = TRUE;
        else
        clan_table[new_clan].independent = TRUE;
       }
       else if(!str_cmp(word,"Pkill"))
       {
        number = fread_number( fp );
        if(number == 0)
        clan_table[new_clan].pkill = FALSE;
        else if(number == 1)
        clan_table[new_clan].pkill = TRUE;
        else
        clan_table[new_clan].pkill = FALSE;
       }
       else if(!str_cmp(word,"Kills"))
        clan_table[new_clan].kills = fread_number( fp );
       else if(!str_cmp(word,"Deaths"))
        clan_table[new_clan].deaths = fread_number( fp );
       else if(!str_cmp(word,"Cocln"))
        clan_table[new_clan].coclan = fread_number( fp );
       else if(!str_cmp(word,"GrdVnm"))
        clan_table[new_clan].guardian_vnum = fread_number( fp );
       else if(!str_cmp(word,"GrdSay"))
        clan_table[new_clan].guardian_say = fread_string( fp );
       else if(!str_cmp(word,"Sk1"))
        clan_table[new_clan].clan_skill1 = fread_string( fp );
       else if(!str_cmp(word,"Sk2"))
        clan_table[new_clan].clan_skill2 = fread_string( fp );
       else if(!str_cmp(word,"Sk3"))
        clan_table[new_clan].clan_skill3 = fread_string( fp );
       else if(!str_cmp(word,"Sk4"))
        clan_table[new_clan].clan_skill4 = fread_string( fp );
       else if(!str_cmp(word,"Sk5"))
        clan_table[new_clan].clan_skill5 = fread_string( fp );
       else if(!str_cmp(word,"Sk6"))
        clan_table[new_clan].clan_skill6 = fread_string( fp );
     } 
       /* Now after that stuff...come the members */
      for (;;)
      { 
        if (feof (fp))
        { 
         break; 
        }
      
        temprank = fread_number (fp); 
        if (temprank < 999) 
        { 
          in_list = new_new_clan (); 
          in_list->rank = temprank; 
          in_list->name = str_dup (fread_string (fp)); 
          in_list->next = new_clan_list[new_clan]; 
          new_clan_list[new_clan] = in_list;
          clan_members[new_clan] += 1;  // Increment Members
        } 
        else 
        { 
         break; 
        } 
      }
     if (!clan_table[new_clan].coclan)
        clan_table[new_clan].coclan = 0;
     fclose (fp); 
 return;
}

 
void  
check_new_clan (char *name, int new_clan, int rank) 
{ 
        int i; 
        bool changed = FALSE; 
        bool found = FALSE; 
        NEW_CLAN_DATA *temp1; 
        NEW_CLAN_DATA *temp2; 
#ifdef DEBUG 
        Debug ("check_new_clan"); 
#endif 
        for (i = 0; i < MAX_CLAN; i++) 
        { 
                temp1 = new_clan_list[i]; 
                if (temp1 == NULL)              /* Need to look-ahead 1 so do this check first */ 
                { 
                        if (i == new_clan) 
                        { 
                                temp1 = new_new_clan (); 
                                temp1->name = str_dup (name); 
                                temp1->rank = rank; 
                                temp1->next = new_clan_list[i]; 
                                new_clan_list[i] = temp1; 
                                changed = TRUE; 
                                found = TRUE; 
                                save_new_clan (new_clan); 
                                break; 
                        } 
                } 
                else if (!str_cmp (temp1->name, name))  /* compare with first on list */ 
                { 
                        found = TRUE; 
                        if (i == new_clan) 
                        { 
                                if (temp1->rank != rank) 
                                { 
                                        temp1->rank = rank; 
                                        changed = TRUE; 
                                } 
                        } 
                        else 
                        { 
                                temp2 = new_clan_list[i];       /* placeholder */ 
                                new_clan_list[i] = new_clan_list[i]->next;      /* drop from list */ 
                                free_new_clan (temp2);  /* recycle the memory */ 
                                changed = TRUE; 
                        } 
                } 
                else 
                        for (; temp1->next != NULL; temp1 = temp1->next) 
                        { 
                                if (i == new_clan) 
                                {                               /* in new_clan, make sure in list */ 
                                        if (!str_cmp (temp1->next->name, name)) 
                                        { 
                                                if (temp1->next->rank != rank) 
                                                { 
                                                        temp1->next->rank = rank; 
                                                        changed = TRUE; 
                                                } 
                                                found = TRUE; 
                                                break; 
                                        } 
                                } 
                                else 
                                {                               /* not in new_clan, make sure not in list */ 
                                        if (!str_cmp (temp1->next->name, name)) 
                                        { 
                                                temp2 = temp1->next;    /* placeholder */ 
                                                temp1->next = temp2->next;      /* drop from list */ 
                                                free_new_clan (temp2);  /* recycle the memory */ 
                                                changed = TRUE; 
                                                break; 
                                        } 
                                } 
                        } 
                if (!found && (i == new_clan))  /* need to add */ 
                { 
                        temp1 = new_new_clan (); 
                        temp1->name = str_dup (name); 
                        temp1->rank = rank; 
                        temp1->next = new_clan_list[i]; 
                        new_clan_list[i] = temp1; 
                        changed = TRUE; 
                } 
                if (changed) 
                        save_new_clan (i); 
        } 
 
} 
 
void  
members_of_new_clan (char *members, int new_clan, int rank, int max_members) 
/* returns the first max_members players of rank "rank" in new_clan "new_clan" 
   and stores in a variable called members */ 
/* members must be initialized prior to calling this function, it will strcat 
   onto the passed variable */ 
{ 
        NEW_CLAN_DATA *temp1; 
        int count = 0; 
#ifdef DEBUG 
        Debug ("members_of_new_clan"); 
#endif 
        temp1 = new_clan_list[new_clan]; 
        for (; temp1 != NULL; temp1 = temp1->next) 
        { 
                if (strlen (members) > MAX_STRING_LENGTH - 20) 
                        break; 
                if (temp1->rank == rank) 
                { 
                        strcat (members, temp1->name); 
                        strcat (members, ", "); 
                        if (++count >= max_members) 
                                break; 
                } 
        } 
        if (strlen (members) > 2 && members[strlen (members) - 2] == ',') 
                members[strlen (members) - 2] = '\0'; 
} 
 
 
void  
do_roster (CHAR_DATA * ch, char *argument) 
{ 
        static char arg[MAX_INPUT_LENGTH]; 
        static char buf[2 * MAX_STRING_LENGTH]; 
        int i; 
 
#ifdef DEBUG 
        Debug ("do_roster"); 
#endif 
 
        argument = one_argument (argument, arg); 
 
        if ((arg[0] == '\0')) 
        { 
                send_to_char ("Syntax:  roster <new_clan>\n\r", ch); 
                return; 
        } 
 
        for (i = 0; i < MAX_CLAN; i++) 
        { 
                if ((!str_prefix (arg, clan_table[i].name))) 
                { 
                        break; 
                } 
        } 
 
        if (i >= MAX_CLAN) 
        { 
                send_to_char ("That's not a clan!\n\r", ch); 
                return; 
        } 
 
        sprintf (buf, "The following list displays the current membership of %s:\n\r", clan_table[i].who_name); 
        send_to_char (buf, ch); 
 
        sprintf (buf, "{&Members:{x\n\r"); 
        send_to_char (buf, ch); 
 
        sprintf (buf, "      {&L{7eader{x:"); 
        members_of_new_clan (buf, i, 7, 10); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
 
        sprintf (buf, "      {^V{6ice {^L{6eaders:{x"); 
        members_of_new_clan (buf, i, 6, 10); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
 
        sprintf (buf, "      {#Elite Captains{x:"); 
        members_of_new_clan (buf, i, 5, 10); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 

        sprintf (buf, "      {#Commanders{x:");
        members_of_new_clan (buf, i, 4, 999); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
 
        sprintf (buf, "      {#Lieutenants{x:"); 
        members_of_new_clan (buf, i, 3, 999); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 

        sprintf (buf, "      {@Soldiers{x:");
        members_of_new_clan (buf, i, 2, 999); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
 
        sprintf (buf, "      {!Recruits{x:"); 
        members_of_new_clan (buf, i, 1, 999); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
 
        sprintf (buf, "Members:\n\r "); 
        members_of_new_clan (buf, i, 0, 999); 
        send_to_char (buf, ch); 
        send_to_char ("\n\r", ch); 
} 
 
void do_rosterpurge (CHAR_DATA * ch, char *argument) 
{ 
        static char arg[MAX_INPUT_LENGTH]; 
        static char buf[MAX_INPUT_LENGTH]; 
        NEW_CLAN_DATA *temp1; 
        int i; 
        FILE *in; 
 
#ifdef DEBUG 
        Debug ("do_rosterpurge"); 
#endif 
 
        one_argument (argument, arg); 
 
        if ((arg[0] == '\0')) 
        { 
                send_to_char ("Syntax:  rosterpurge <new_clan>\n\r", ch); 
                return; 
        } 
 
        for (i = 0; i < MAX_CLAN; i++)
        { 
                if ((!str_prefix (arg, clan_table[i].name))) 
                { 
                        break; 
                } 
        } 
 
        if (i >= MAX_CLAN) 
        { 
                send_to_char ("That's not a new_clan!\n\r", ch); 
                return; 
        } 

        for (temp1 = new_clan_list[i] ; temp1 != NULL; temp1 = temp1->next) 
        { 
                sprintf (buf, "%s%s%s%s", PLAYER_DIR, initial (ch->name), 
                                 "/", capitalize (ch->name)); 
 
/* 
 *  Original Version. (Not parsed by letter directories) 
 *      sprintf(buf, "../player/%s", capitalize(temp1->name)); 
 */
                if ((in = fopen (buf, "r")) == NULL) 
                { 
                  check_new_clan (temp1->name, -1, 0); 
                } 
                else
                   fclose (in);

        }
        send_to_char ("Roster purged of nonexistent players.\n\r", ch); 
} 

/* CEDIT from here down - Skyntil - */
bool can_coclan(int clan, int coclan)
{
 if (clan == coclan)
    return FALSE;
 if (clan_table[clan].pkill == TRUE && clan_table[coclan].pkill == TRUE)
    return FALSE;
 if (clan_table[clan].pkill == FALSE && clan_table[coclan].pkill == FALSE)
    return FALSE;
 if (clan_table[clan].independent == TRUE || clan_table[clan].independent == TRUE)
    return FALSE;
 return TRUE;
}

CEDIT(cedit_show)
{
		char clanname[MSL];
        char buf[MSL*2];
        int clan;
		int coclan;
        if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
        {
          send_to_char("That clan doesn't exist.\n\r",ch);
          return FALSE;
        }
		coclan = clan_table[clan].coclan;        
        sprintf(clanname,"%s",clan_table[clan].name);
		sprintf(buf,"{$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{@({^Cedit for clan:{7 %-12s {@){$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-\n\r",capitalize(clanname));
		send_to_char(buf,ch);
		sprintf(buf,"{$|                                                                           |\n\r{$| {7Name: {@[{^%-12s{@]   {7WhoName {@[{^",clanname);
		send_to_char(buf,ch);
		sprintf(buf,"%-35s",clan_table[clan].who_name);
		send_to_char_bw(buf,ch);
		sprintf(buf,"{@]      {$|\n\r{$|---------------------------------------------------------------------------|{x\n\r{$|            {#Room Vnum              {$|            {#Clan Status                {$|{x\n\r");
        send_to_char(buf,ch);
        sprintf(buf,"{$|  {7Death: {@[{^%-5d{@]   {7Recall: {@[{^%-5d{@] {$|  ",clan_table[clan].deathroom,clan_table[clan].recall);
		send_to_char(buf,ch);
        sprintf(buf,"{7Pkill: {@[%-7s{@] {7Independent: {@[%-7s{@]  {$|\n\r",((clan_table[clan].pkill == FALSE) ? "{3FALSE" : "{!TRUE"),((clan_table[clan].independent == FALSE) ? "{3FALSE" : "{!TRUE"));
		send_to_char(buf,ch);
    	sprintf(buf,"{$|---------------------------------------------------------------------------|{x\n\r{$|                       {#Misc. Settings                                      {$|\n\r{$|    {#Clan God                            {#No. {6-  {#CoClan Name                 {$|\n\r");
        send_to_char(buf,ch);
        sprintf(buf,"{$| {7God: {@[{^%-7s{@]                              {7CoClan: {@[{^%-2d {@- %-12s{@]   {$|\n\r",clan_table[coclan].god,coclan,clan_table[coclan].who_name);
		send_to_char(buf,ch);
        sprintf(buf,"{$| {7Guard_vnum {@[{^%-5d{@] {7Guard_say: {@[{^Greetings, %-30s{@] {$|\n\r",clan_table[clan].guardian_vnum,clan_table[clan].guardian_say);
		send_to_char(buf,ch);
		sprintf(buf,"{$|---------------------------------------------------------------------------|\n\r{$|  {#ClanSkills                                                               {$|\n\r");
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill1 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill1);
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill2 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill2);
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill3 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill3);
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill4 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill4);
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill5 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill5);
		send_to_char(buf,ch);
		sprintf(buf,"{$|              {7Clanskill6 {@[{^%-30s{@]                  {$|\n\r",clan_table[clan].clan_skill6);
		send_to_char(buf,ch);
        sprintf(buf,"{$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{@({^Cedit for clan: {7%-12s {@){$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{4={$-{x\n\r",capitalize(clanname));
		send_to_char(buf,ch);

  return FALSE;
}

CEDIT(cedit_list)
{
        char buf[MSL*2];
        int clan;

  for(clan = 1; clan < MAX_CLAN; clan++)
  {
   sprintf(buf,"Name:    [%s] WhoName: [%s] God:     [%s] Death:   [%d]\n\rRecall:  [%d] Indep:   [%s] Pkill:   [%s]\n\r",
   clan_table[clan].name, clan_table[clan].who_name,clan_table[clan].god,clan_table[clan].deathroom,
   clan_table[clan].recall, ((clan_table[clan].independent == FALSE) ? "FALSE" : "TRUE"),
   ((clan_table[clan].pkill == FALSE) ? "FALSE" : "TRUE"));
   send_to_char_bw(buf,ch);
  }

   return FALSE;
}

void do_cedit(CHAR_DATA *ch, char *argument)
{
  int clan;

	if ( IS_NPC(ch) )
		return;

    if (!oedit_check(ch))
    {
        send_to_char("OEdit: You must balance this eq before you are done.\n\r",ch);
        return;
    }
        if (argument[0] == '\0')
        {
         send_to_char("You must specify a clan to edit.\n\r",ch);
         return;
        }

        if((clan = clan_lookup(argument)) == -1)
        {
          send_to_char("That clan doesn't exist.\n\r",ch);
          return;
        }

        ch->desc->clanEdit      = clan_table[clan].name;
        ch->desc->editor        = ED_CLAN;

        send_to_char("Entering Clan Editor.\n\r",ch);

	return;
}

void cedit( CHAR_DATA *ch, char *argument)
{
	 char arg[MAX_INPUT_LENGTH];
	 char command[MAX_INPUT_LENGTH];
         int cmd;

	 smash_tilde(argument);
	 strcpy(arg, argument);
	 argument = one_argument( argument, command);

         if (ch->pcdata->security < 4)
	 {
                send_to_char("CEdit: Insufficient security to edit clans.\n\r",ch);
                edit_done(ch);
                return;
	 }

	 if (command[0] == '\0')
	 {
                cedit_show(ch, argument);
                return;
	 }

	 if (!str_cmp(command, "done") )
	 {
		  edit_done(ch);
		  return;
	 }

         for (cmd = 0; cedit_table[cmd].name != NULL; cmd++)
	 {
                if (!str_prefix(command, cedit_table[cmd].name) )
                {
                         if ((*cedit_table[cmd].olc_fun) (ch, argument))
                         {
                            save_clans();
                            send_to_char( "Clans Saved.\n\r", ch);
                            load_new_clans_list();
                            send_to_char( "Clans Loaded.\n\r",ch);
                            CLANS_CHANGED = FALSE;
                         }
                         return;
                }
	 }

	 interpret(ch, arg);
	 return;
}

CEDIT( cedit_name )
{
 int clan;
 char arg[MSL/4];
 char buf[MSL/4];

 /* Wait till actual online CLAN CREATION -- Fuck that
 send_to_char("This function isn't enabled yet.\n\r",ch);
 return FALSE; */
 one_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }
 sprintf(buf, "%s", clan_table[clan].name);
 clan_table[clan].name = str_dup(arg);
 ch->desc->clanEdit = clan_table[clan].name;
 printf_to_char(ch,"Clan %s, Name changed to %s.\n\r", buf, clan_table[clan].name);
 return TRUE;
}

CEDIT( cedit_whoname )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 clan_table[clan].who_name = str_dup(arg);
 printf_to_char(ch,"Clan %s, Who_name changed to %s.\n\r",clan_table[clan].name, clan_table[clan].who_name);
 return TRUE;

}

CEDIT( cedit_deathroom )
{
 int clan, room;

 char arg[MSL/4];

 one_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(!is_number(arg))
 {
  send_to_char("Argument must be a number[vnum].\n\r",ch);
  return FALSE;
 }

 room = atoi(arg);

 clan_table[clan].deathroom = room;
 printf_to_char(ch,"Clan %s, DeathRoom changed to %d.\n\r",capitalize(ch->desc->clanEdit), clan_table[clan].deathroom);

 return TRUE;
}

CEDIT( cedit_recall )
{
 int clan, recall;
 char arg[MSL/4];

 one_argument(argument,arg);
 
 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(!is_number(arg))
 {
  send_to_char("Argument must be a number[vnum].\n\r",ch);
  return FALSE;
 }

 recall = atoi(arg);

 clan_table[clan].recall = recall;
 printf_to_char(ch,"Clan %s, Recall changed to %d.\n\r",capitalize(ch->desc->clanEdit), clan_table[clan].recall);

 return TRUE;
}

CEDIT( cedit_independent )
{
 int clan;
 char arg[MSL/4];

 one_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(!str_cmp(arg,"true"))
 {
  clan_table[clan].independent = TRUE;
  send_to_char("Clan is now Independent.\n\r",ch);
  send_to_char(arg,ch);
 }
 else if(!str_cmp(arg,"false"))
 {
  clan_table[clan].independent = FALSE;
  send_to_char("Clan is now NON-Independent.\n\r",ch);
 }
 else
 {
  send_to_char("Syntax: independent [TRUE|FALSE]\n\r",ch);
  return FALSE;
 }

 return TRUE;
}

CEDIT( cedit_pkill )
{
 int clan;
 char arg[MSL/4];

 one_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(!str_cmp(arg,"true"))
 {
  clan_table[clan].pkill = TRUE;
  send_to_char("Clan is now a PKILL clan.\n\r",ch);
 }
 else if(!str_cmp(arg,"false"))
 {
  clan_table[clan].pkill = FALSE;
  send_to_char("Clan is now a NON-PKILL clan.\n\r",ch);
 }
 else
 {
  send_to_char("Syntax: pkill [TRUE|FALSE]\n\r",ch);
  return FALSE;
 }

 return TRUE;
}
CEDIT( cedit_guard_vnum )
{
 int clan, vnum;

 char arg[MSL/4];

 one_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(!is_number(arg))
 {
  send_to_char("Argument must be a number[vnum].\n\r",ch);
  return FALSE;
 }

 vnum = atoi(arg);

 clan_table[clan].guardian_vnum = vnum;
 printf_to_char(ch,"Clan %s, Guardian_Vnum changed to %d.\n\r",capitalize(ch->desc->clanEdit), clan_table[clan].guardian_vnum);

 return TRUE;
}

CEDIT( cedit_coclan )
{
 int coclan, clan;
 char arg[MSL/4];
 char name[MSL];
 char cname[MSL];
 char buf[MSL];
 one_argument(argument,arg);

 coclan = 0;
 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }
 if(!is_number(arg))
 {
  if(clan_lookup(arg) == -1)
    {
        send_to_char("That is not a valid coclan",ch);
        return FALSE;
    }
  else
    coclan = clan_lookup(arg);
 }
 if (is_number(arg))
    {
     if (atoi(arg) > MAX_CLAN || atoi(arg) < 0)
        {
        send_to_char("That is not a valid clan",ch);
        return FALSE;
        }
     else
       coclan = atoi(arg);
 }
    sprintf(name,"%s",capitalize(clan_table[clan].name));
    sprintf(cname,"%s",capitalize(clan_table[coclan].name));
    if (!can_coclan(clan,coclan))
    {
        sprintf(buf,"%s and %s cannot be co-clanned, they must be a non-pk and pk clan.\n\r",name,cname);
        send_to_char(buf,ch);        
        return FALSE;
    }
    clan_table[clan].coclan = coclan;
    clan_table[coclan].coclan = clan; 
    printf_to_char(ch,"Clan %s and %s are now coclanned.\n\r",name, cname);
    return TRUE;
}

CEDIT( cedit_guard_say )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 clan_table[clan].guardian_say = str_dup(arg);
 printf_to_char(ch,"Clan %s, Guardian_Say changed to %s.\n\r",clan_table[clan].name, clan_table[clan].guardian_say);
 return TRUE;

}
CEDIT( cedit_clanskill1 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill1 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill1 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill1);
 return TRUE;

}

CEDIT( cedit_clanskill2 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill2 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill2 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill2);
 return TRUE;

}

CEDIT( cedit_clanskill3 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill3 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill3 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill3);
 return TRUE;

}

CEDIT( cedit_clanskill4 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill4 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill4 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill4);
 return TRUE;

}

CEDIT( cedit_clanskill5 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill5 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill5 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill5);
 return TRUE;

}

CEDIT( cedit_clanskill6 )
{
 int clan;

 char arg[MSL/4];

 grab_argument(argument,arg);

 if((clan = clan_lookup(ch->desc->clanEdit)) == -1)
 {
  send_to_char("That clan doesn't exist.\n\r",ch);
  return FALSE;
 }

 if(is_number(arg))
 {
  send_to_char("Argument must be a string[no spaces].\n\r",ch);
  return FALSE;
 }

 if(skill_lookup(arg) == -1)
 {
  send_to_char("That is not a Skill!\n\r",ch);
  return FALSE;
 }

 clan_table[clan].clan_skill6 = str_dup(arg);
 printf_to_char(ch,"Clan %s, Clan_Skill6 changed to %s.\n\r",clan_table[clan].name, clan_table[clan].clan_skill6);
 return TRUE;

}
