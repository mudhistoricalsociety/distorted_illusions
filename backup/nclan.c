/* New Clan System Written For Asgardian Nightmare
 * by Skyntil(Gabe Volker)
 */

#include "include.h"
#include "clan.h"

CLN_DATA *cln_lookup(char *argument);
CLN_DATA *new_cln(void);
void free_cln(CLN_DATA *clan);
MBR_DATA *new_mbr(void);
bool remove_member(char *argument, CLN_DATA *cln);
void add_member(char *argument, CLN_DATA *clan);
void show_clans(CHAR_DATA *ch, char *argument);  //Multiple
void show_clan(CHAR_DATA *ch, char *argument);   //Single
void free_mbr(MBR_DATA *mbr);
MBR_DATA *mbr_lookup(char *argument, CLN_DATA *clan);
bool add_clan(char *argument);
bool remove_clan(char *argument);
bool write_clan(CLN_DATA *clan);
void write_clans(void);
void add_mbr_rank(char *name, CLN_DATA *cln, int rank);
CLN_DATA * read_nclan(FILE *fp);
bool add_clan_manual( CLN_DATA *cln);
void read_nclans(void);
void write_nclan_list(void);
void sbug( const char *str, char *param );
extern const struct clan_titles clan_rank_table[MAX_RANK];
void find_mbr_rank(CLN_DATA *nclan, int rank, char *argument);
void check_cln_member(CHAR_DATA *victim, CLN_DATA *cln, int rank);
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_quit );
bool is_nclan(CHAR_DATA *ch);
bool is_same_nclan(CHAR_DATA *ch, CHAR_DATA *victim);

CLN_DATA *cln_list;   // The Clan List
CLN_DATA *cln_first;  // First Clan

/* Think of it like this
 *  CLAN1 -> CLAN2 -> CLAN3 -> NULL(end)
 *   |        |        |
 *  MBR1     MBR1     MBR1
 *   |        |        |
 *  MBR2     MBR2     MBR2
 *   |        |        |
 *  NULL(end)NULL(end)NULL(end)
 */

CLN_DATA *cln_data_free;

CLN_DATA *new_cln(void)
{
 CLN_DATA *nclan;

 if (cln_data_free == NULL) 
  nclan = alloc_mem(sizeof(*nclan));
 else 
 { 
   nclan = cln_data_free; 
   cln_data_free = cln_data_free->next; 
 } 

 nclan->next = NULL;
 nclan->name = str_dup("");
 nclan->members = 0;
 nclan->member_first = NULL;
 nclan->member_list = new_mbr();
 nclan->kills = 0;
 nclan->deaths = 0;
 nclan->atwar = 0;

 nclan->who_name = str_dup("");
 nclan->god = str_dup("");
 nclan->deathroom = ROOM_VNUM_ALTAR;
 nclan->recall = ROOM_VNUM_ALTAR;
 nclan->independent = FALSE;
 nclan->pkill = TRUE;

 VALIDATE(nclan); 
 
 return nclan; 
}

void free_cln(CLN_DATA *clan)
{
 MBR_DATA *remove_mbr;

 if (!IS_VALID(clan))
     return; 
 
 if(clan->member_list == NULL || clan->member_first == NULL)
 {
  free_string(clan->name);
  free_string(clan->who_name);
  free_string(clan->god);
  clan->next = cln_data_free;
  cln_data_free = clan; 
  INVALIDATE(clan);
  return;
 }
 else  // With Members
 {
  for(remove_mbr = clan->member_first; remove_mbr != NULL; remove_mbr = remove_mbr->next)
    free_mbr(remove_mbr);

  free_mbr(clan->member_first);

  free_string(clan->name);
  free_string(clan->who_name);
  free_string(clan->god);

  clan->next = cln_data_free;
  cln_data_free = clan; 
  INVALIDATE(clan);
  return;
 }
 return;
}

MBR_DATA *mbr_data_free;

MBR_DATA *new_mbr(void)
{
 MBR_DATA *nmbr;

 if (mbr_data_free == NULL) 
  nmbr = alloc_mem(sizeof(*nmbr));
 else 
 { 
   nmbr = mbr_data_free; 
   mbr_data_free = mbr_data_free->next; 
 } 

 nmbr->next = NULL;
 nmbr->name = str_dup("");

 VALIDATE(nmbr); 
 
 return nmbr; 
}

void free_mbr(MBR_DATA *mbr)
{
    if (!IS_VALID(mbr)) 
        return; 
 
    mbr->next = mbr_data_free; 
    mbr_data_free = mbr; 
    INVALIDATE(mbr); 
}

bool add_clan( char *argument)
{
 CLN_DATA *new_clan;

 new_clan = new_cln();
 new_clan->name = str_dup(argument);

 // Check if First Clan
 if(cln_list == NULL || cln_first == NULL)
 {
   cln_list = new_clan; // Put into position
   cln_first = cln_list; // Make it the first
 }
 else
 {
   cln_list->next = new_clan; // Point to new node
   /* CLN_LIST -> NEW_CLAN */
   cln_list = new_clan; // Set cln_list to current position.
   /* OLD_NODE -> CLN_LIST */
 }

 return TRUE;
}

bool remove_clan(char *argument)
{
 CLN_DATA *cln_find, *cln_found;

 if((cln_found = cln_lookup(argument)) == NULL)
  return FALSE;

 if(cln_found == cln_first)
 {
  /* CLN_FND -> NEXT
   *       Cut found out and release it
   * NEXT
   */

  cln_first = cln_first->next; // Assign first to next
  free_cln(cln_found);
  return TRUE;
 }
 else if(cln_found->next == NULL) // Last
 {
  /* CLN_FIND -> CLN_FOUND -> NULL
   *       Cut found out and release it
   * CLN_FIND -> NULL
   */

  for(cln_find = cln_first; cln_find != NULL; cln_find = cln_find->next)
  {
   if(cln_find->next == cln_found)
   break;
  }

  cln_list = cln_find; // Assign End pointer to new last
  cln_list->next = NULL; // Don't point to something thats gone
  free_cln(cln_found);
  return TRUE;
 }
 else // Not the first clan
 {

 /* CLN_FIND -> CLN_FOUND -> NEXT
  *         Cut found out and release it
  * CLN_FIND -> NEXT
  */

  for(cln_find = cln_first; cln_find != NULL; cln_find = cln_find->next)
  {
   if(cln_find->next == cln_found)
   break;
  }

  cln_find->next = cln_found->next; // Assign prev to next
  free_cln(cln_found);
  return TRUE;
 }

 return TRUE;
}

void show_clans(CHAR_DATA *ch, char *argument)
{
 CLN_DATA *clan;
 MBR_DATA *mbr;
 char buf[256],buf2[256],buf3[256];
 int i;

 buf[0] = '\0';
 buf2[0] = '\0';
 buf3[0] = '\0';

 if(cln_first == NULL || cln_list == NULL)
 {
  send_to_char("There are no clans loaded.\n\r",ch);
  return;
 }

 for(i = 0; i < 12; i++)
  strcat(buf,"=");
 strcat(buf,"-NCLANS-");
 for(i = 0; i < 12; i++)
  strcat(buf,"=");

 printf_to_char(ch,"\n\r%-30s\n\r",buf);

 for(clan = cln_first; clan != NULL; clan = clan->next)
 {
  sprintf(buf3," Name: %s",clan->name);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3," Member Total: %d",clan->members);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  if(clan->member_list == NULL || clan->member_first == NULL)
  {
    sprintf(buf3,"  Members: %s","None");
    sprintf(buf2,"|%-30s|\n\r",buf3);
    send_to_char(buf2,ch);
  }
  else
  {
   for(mbr = clan->member_first; mbr != NULL; mbr = mbr->next)
   {
    sprintf(buf3,"  Member: %s Rank: %s",mbr->name, clan_rank_table[mbr->rank].rank);
    sprintf(buf2,"|%-30s|\n\r",buf3);
    send_to_char(buf2,ch);
   }
  }

  sprintf(buf3,"  Kills: %d Deaths: %d",clan->kills, clan->deaths);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"  War Flags: %s","At Peace");
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);
 }

 buf[0] = '\0';

 for(i = 0; i < 32; i++)
  strcat(buf,"=");

 printf_to_char(ch,"%-30s\n\r",buf);

 return;
}

void show_clan(CHAR_DATA *ch, char *argument)
{
 CLN_DATA *clan;
 MBR_DATA *mbr;
 char buf[256],buf2[256],buf3[256];
 int i, length, flength;

 buf[0] = '\0';
 buf2[0] = '\0';
 buf3[0] = '\0';

 if(cln_first == NULL || cln_list == NULL)
 {
  send_to_char("There are no clans loaded.\n\r",ch);
  return;
 }

 if((clan = cln_lookup(argument)) == NULL)
 {
  send_to_char("That clan does not exist.\n\r",ch);
  return;
 }

 flength = 30 - strlen(clan->name)+2;

 for(i = 0; i < flength/2; i++)
  strcat(buf,"=");
 sprintf(buf2,"-%s-",capitalize(clan->name));
 strcat(buf,buf2);
 flength = 30 - strlen(buf);
 for(i = 0; i < flength; i++)
  strcat(buf,"=");
 length = strlen(buf);

 printf_to_char(ch,"\n\r%-30s\n\r",buf);

  sprintf(buf3," Name: %s",clan->name);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"  Member Total: %d",clan->members);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  if(clan->member_list == NULL || clan->member_first == NULL)
  {
    sprintf(buf3,"  Members: %s","None");
    sprintf(buf2,"|%-30s|\n\r",buf3);
    send_to_char(buf2,ch);
  }
  else
  {
   for(mbr = clan->member_first; mbr != NULL; mbr = mbr->next)
   {
    sprintf(buf3,"  Member: %s Rank: %s",mbr->name, clan_rank_table[mbr->rank].rank);
    sprintf(buf2,"|%-30s|\n\r",buf3);
    send_to_char(buf2,ch);
   }
  }

  sprintf(buf3,"  Kills: %d Deaths: %d",clan->kills, clan->deaths);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"  War Flags: %s","At Peace");
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"WHO: %s",clan->who_name);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"GOD: %s",clan->god);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"RECALL: %d",clan->recall);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"DEATHROOM: %d",clan->deathroom);
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);


  sprintf(buf3,"INDEPENDENT: %s",(clan->independent==TRUE) ? "TRUE":"FALSE");
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  sprintf(buf3,"PKILL: %s",(clan->pkill==TRUE) ? "TRUE":"FALSE");
  sprintf(buf,"|%-30s|\n\r",buf3);
  send_to_char(buf,ch);

  buf[0] = '\0';

  for(i = 0; i < length; i++)
   strcat(buf,"=");

  printf_to_char(ch,"%-30s\n\r",buf);

  return;
}


void add_member(char *argument, CLN_DATA *clan)
{
 MBR_DATA *new_mebr;

 new_mebr = new_mbr();
 new_mebr->name = str_dup(argument);

 if(clan->member_list == NULL || clan->member_first == NULL)
 {
   clan->member_list = new_mebr; // Put into position
   clan->member_first = clan->member_list; // Set as First
 }
 else
 {
   clan->member_list->next = new_mebr; // Point to next node
   /* MBR_LIST -> NEW_MBR */
   clan->member_list = new_mebr; // Set cln_list to current position.
   /* OLD_NODE -> MBR_LIST */
 }
 clan->members += 1;

 return;
}

bool remove_member(char *argument, CLN_DATA *cln)
{
 MBR_DATA *mbr_find, *mbr_found;

 if((mbr_found = mbr_lookup(argument,cln)) == NULL)
  return FALSE;

 if(mbr_found == cln->member_first)
 {
  /* mbr_FND -> NEXT
   *       Cut found out and release it
   * NEXT
   */

  cln->member_first = mbr_found->next; // Assign first to next
  cln->members -= 1;
  free_mbr(mbr_found);
  return TRUE;
 }
 else if(mbr_found->next == NULL) // Last
 {
  /* mbr_FIND -> mbr_FOUND -> NULL
   *       Cut found out and release it
   * mbr_FIND -> NULL
   */

  for(mbr_find = cln->member_first; mbr_find != NULL; mbr_find = mbr_find->next)
  {
   if(mbr_find->next == mbr_found)
   break;
  }

  cln->member_list = mbr_find; // Assign End pointer to new last
  cln->member_list->next = NULL; // Don't point to something thats gone
  cln->members -= 1;
  free_mbr(mbr_found);
  return TRUE;
 }
 else // Not the first clan
 {

 /* mbr_FIND -> mbr_FOUND -> NEXT
  *         Cut found out and release it
  * mbr_FIND -> NEXT
  */

  for(mbr_find = cln->member_first; mbr_find != NULL; mbr_find = mbr_find->next)
  {
   if(mbr_find->next == mbr_found)
   break;
  }

  mbr_find->next = mbr_found->next; // Assign prev to next
  cln->members -= 1;
  free_mbr(mbr_found);
  return TRUE;
 }

 return TRUE;
}

CLN_DATA *cln_lookup(char *argument)
{
  CLN_DATA *nclan;

  for (nclan = cln_first; nclan != NULL; nclan = nclan->next)
  {
     if (LOWER (argument[0]) == LOWER (nclan->name[0])
          && !str_prefix (argument, nclan->name))
             return nclan;
  }

   return NULL;
}

MBR_DATA *mbr_lookup(char *argument, CLN_DATA *clan)
{
  MBR_DATA *nmbr;

  for (nmbr = clan->member_first; nmbr != NULL; nmbr = nmbr->next)
  {
     if (LOWER (argument[0]) == LOWER (nmbr->name[0])
          && !str_prefix (argument, nmbr->name))
             return nmbr;
  }

   return NULL;
}

void do_nclan(CHAR_DATA *ch, char *argument)
{
 char arg[MSL/10];
 char arg2[MSL/10];

 if(IS_NPC(ch))
  return;

 argument = one_argument(argument,arg);

 if(!str_cmp(arg,"list"))
 {
  show_clans(ch,argument);
  return;
 }

 if(!str_cmp(arg,"show"))
 {
  one_argument(argument,arg2);
  if(!arg2[0])
  {
   send_to_char("Which clan would you like to show?\n\r",ch);
   return;
  }
  show_clan(ch,arg2);
  return;
 }


 if(!str_cmp(arg,"add"))
 {
  grab_argument(argument,arg);
  if(!arg[0])
  {
   send_to_char("You must include a nclan name.\n\r",ch);
   return;
  }
  add_clan(arg);
  printf_to_char(ch,"NCLAN: %s Added.\n\r",arg);
  return;
 }

 if(!str_cmp(arg,"remove"))
 {
  grab_argument(argument,arg);
  if(!arg[0])
  {
   send_to_char("You must include a nclan name.\n\r",ch);
   return;
  }
  if(!remove_clan(arg))
  {
   send_to_char("There is no such clan.\n\r",ch);
   return;
  }
  else
  { 
   printf_to_char(ch,"NCLAN: %s Removed.\n\r",arg);
   return;
  }
 }

 if(!str_cmp(arg,"memadd"))
 {
  CLN_DATA *nclan;
  argument = one_argument(argument,arg);
  grab_argument(argument,arg2);
  if(!arg[0])
  {
   send_to_char("You must include a NCLAN name.\n\r",ch);
   return;
  }
  if((nclan = cln_lookup(arg)) == NULL)
  {
   send_to_char("That clan does not exist.\n\r",ch);
   return;
  }
  if(!arg2[0])
  {
   send_to_char("You must include a member name.\n\r",ch);
   return;
  }
  add_member(arg2,nclan);
  printf_to_char(ch,"NCLAN: %s MEMBER: %s Added.\n\r",arg,arg2);
  return;
 }

 if(!str_cmp(arg,"memremove"))
 {
  CLN_DATA *nclan;
  argument = one_argument(argument,arg);
  grab_argument(argument,arg2);
  if(!arg[0])
  {
   send_to_char("You must include a NCLAN name.\n\r",ch);
   return;
  }
  if((nclan = cln_lookup(arg)) == NULL)
  {
   send_to_char("That clan does not exist.\n\r",ch);
   return;
  }
  if(!arg2[0])
  {
   send_to_char("You must include a member name.\n\r",ch);
   return;
  }
  remove_member(arg2,nclan);
  printf_to_char(ch,"NCLAN: %s MEMBER: %s Removed.\n\r",arg,arg2);
  return;
 }

 if(!str_cmp(arg,"save"))
 {
  // CLN_DATA *nclan;
  argument = one_argument(argument,arg2);
  if(!arg2[0])
   strcpy(arg2,"all");
/*  else if(arg2[0])
  {
   if((nclan = cln_lookup(arg2)) == NULL)
   {
    send_to_char("That clan doesn't exits.\n\r",ch);
    return;
   }
   write_clan(nclan);
   printf_to_char(ch,"%s saved.\n\r",nclan->name);
   return;
  } */

  if(!str_cmp(arg2,"all"))
  {
   write_clans();
   send_to_char("Clans saved.\n\r",ch);
   return;
  }
  send_to_char("Only option is all. If you omit any string, it will default to it.\n\r",ch);
  return;
 }

 if(!str_cmp(arg,"load"))
 {
   read_nclans();
   send_to_char("Clans loaded.\n\r",ch);
   return;
 }

 send_to_char("Nclan [list|add|remove|memadd|memremove|show|load|save]\n\r",ch);
 return;
}

void do_ncset(CHAR_DATA *ch, char *argument)
{
 char arg[MSL/10];
 char arg2[MSL/10];
 char arg3[MSL/10];
 CLN_DATA *nclan;
 int value;

 if(IS_NPC(ch))
  return;

 argument = one_argument(argument,arg);
 argument = one_argument(argument,arg2);
 grab_argument(argument,arg3);

 if(!arg[0])
 {
  send_to_char("NCSET: <clanname> [who|god|death|recall|indep|pk]\n\r",ch);
  return;
 }

 if((nclan = cln_lookup(arg)) == NULL)
 {
  send_to_char("That clan does not exist.\n\r",ch);
  return;
 }

 if(!str_cmp(arg2,"who"))
 {
   if(!arg3[0])
   {
    send_to_char("You need to provide a string for the clan's who_name.\n\r",ch);
    return;
   }
   nclan->who_name = str_dup(arg3);
   printf_to_char(ch,"NCSET: %s WHO_NAME: %s\n\r",arg,arg3); 
   return;
 }

 if(!str_cmp(arg2,"god"))
 {
   if(!arg3[0])
   {
    send_to_char("You need to provide a string for the clan's god.\n\r",ch);
    return;
   }
   nclan->god = str_dup(arg3);
   printf_to_char(ch,"NCSET: %s GOD: %s\n\r",arg,arg3); 
   return;
 }

 if(!str_cmp(arg2,"death"))
 {
   if(!arg3[0])
   {
    send_to_char("You need to provide a value for the clan's deathroom.\n\r",ch);
    return;
   }
   if(!is_number(arg3))
   {
    send_to_char("Room must be a numeric value.\n\r",ch);
    return;
   }
   value = atoi(arg3);
   nclan->deathroom = value;
   printf_to_char(ch,"NCSET: %s DEATHROOM: %d\n\r",arg,value); 
   return;
 }

 if(!str_cmp(arg2,"recall"))
 {
   if(!arg3[0])
   {
    send_to_char("You need to provide a value for the clan's recall.\n\r",ch);
    return;
   }
   if(!is_number(arg3))
   {
    send_to_char("Room must be a numeric value.\n\r",ch);
    return;
   }
   value = atoi(arg3);
   nclan->recall = value;
   printf_to_char(ch,"NCSET: %s RECALL: %d\n\r",arg,value); 
   return;
 }

 if(!str_cmp(arg2,"indep"))
 {
   if(!arg3[0])
   {
    send_to_char("True of False?\n\r",ch);
    return;
   }
   if(!str_cmp(arg3,"true"))
    nclan->independent = TRUE;
   else if(!str_cmp(arg3,"false"))
    nclan->independent = FALSE;
   else
   {
    send_to_char("Type true or false for the third argument.\n\r",ch);
    return;
   }
   printf_to_char(ch,"NCSET: %s INDEPENDENT: %s\n\r",arg,capitalize(arg3));
   return;
 }

 if(!str_cmp(arg2,"pk"))
 {
   if(!arg3[0])
   {
    send_to_char("True of False?\n\r",ch);
    return;
   }
   if(!str_cmp(arg3,"true"))
    nclan->pkill = TRUE;
   else if(!str_cmp(arg3,"false"))
    nclan->pkill = FALSE;
   else
   {
    send_to_char("Type true or false for the third argument.\n\r",ch);
    return;
   }
   printf_to_char(ch,"NCSET: %s PKILL: %s\n\r",arg,capitalize(arg3));
   return;
 }

 send_to_char("NCSET: <clanname> [who|god|death|recall|indep|pk]\n\r",ch);
 return;
}

#define CLN_DIR "../data/CLN"

void write_clans(void)
{
 CLN_DATA *nclan;

 write_nclan_list();

 for(nclan = cln_first; nclan != NULL; nclan = nclan->next)
 {
  if(!write_clan(nclan))
   sbug("Error writing %s to file.",nclan->name);
 }
 return;
}

void write_nclan_list()
{
 FILE *fp;
 char buf[256];
 CLN_DATA *nclan;

 sprintf(buf,"%s/nclans.lst",CLN_DIR);

 if((fp = fopen(buf,"w"))==NULL)
 {
  bug("Couldn't open nclans.lst for write.",0);
  return;
 }

 for(nclan = cln_first; nclan != NULL; nclan = nclan->next)
 {
  fprintf(fp,"%s\n",nclan->name);
 }
 fprintf(fp,"%s\n","End");
 fclose(fp);
 return;
}

bool write_clan(CLN_DATA *clan)
{
 FILE *fp;
 char buf[256];
 MBR_DATA *mbr;

 sprintf(buf,"%s/%s.cln",CLN_DIR,clan->name);

 if((fp = fopen(buf,"w")) == NULL)
 {
  bug("Error in Write Clan",0);
  return FALSE;
 }

 fprintf(fp,"NAME %s~\n",clan->name);
 fprintf(fp,"KILLS %d\n",clan->kills);
 fprintf(fp,"DEATHS %d\n",clan->deaths);
 fprintf(fp,"WAR %s\n", print_flags(clan->atwar));
 fprintf(fp,"WHO %s~\n", clan->who_name);
 fprintf(fp,"GOD %s~\n", clan->god);
 fprintf(fp,"DTH %d\n", clan->deathroom);
 fprintf(fp,"REC %d\n", clan->recall);
 fprintf(fp,"IND %s~\n", (clan->independent==TRUE)?"TRUE":"FALSE");
 fprintf(fp,"PKL %s~\n", (clan->pkill==TRUE)?"TRUE":"FALSE");
 if(clan->member_first != NULL)
 {
  fprintf(fp,"%s","MEMBERS\n");
  for(mbr = clan->member_first; mbr != NULL; mbr = mbr->next)
  {
   fprintf(fp,"%s~\n",mbr->name);
   fprintf(fp,"%d\n",mbr->rank);
  }
  fprintf(fp,"%s~\n","EndMbr");
 }
 fprintf(fp,"%s\n","End");

 fclose(fp);
 return TRUE;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value ) \
				if ( !str_cmp( word, literal ) ) \
				{ \
				    field  = value; \
				    fMatch = TRUE; \
				    break; \
				}

CLN_DATA * read_nclan(FILE *fp)
{
 CLN_DATA *nclan = new_cln();
 char *word;
 bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

        case 'D':
            KEY( "DEATHS",   nclan->deaths,  fread_number( fp ) );
            KEY( "DTH",   nclan->deathroom,  fread_number( fp ) );
            break;
        case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
              return nclan;
	    }
          break;
        case 'G':
            KEY( "GOD", nclan->god, fread_string( fp ) );
            break;
        case 'I':
            if(!str_cmp(word,"IND"))
            {
             char *nword;
             nword = fread_string( fp );
             if(!str_cmp(nword,"TRUE"))
              nclan->independent = TRUE;
             else if(!str_cmp(nword,"FALSE"))
              nclan->independent = FALSE;
             else
              bug("NCLAN_READ: INDEP not recognized.",0);
             fMatch = TRUE;
             break;
            }
            break;
        case 'K':
            KEY( "KILLS",    nclan->kills,  fread_number( fp ) );
            break;
        case 'M':
         if(!str_cmp(word,"MEMBERS"))
         {
          char *nword;
          int rank;

          for( ; ; )
          {
           nword = fread_string( fp );
           if(str_cmp(nword,"EndMbr"))
           {
            add_member(nword,nclan);
            rank = fread_number( fp );
            add_mbr_rank(nword,nclan,rank);
           }
           else
            break;
          }
          fMatch = TRUE;
          break;
         }
         break;
        case 'N':
          KEY("NAME",nclan->name, fread_string( fp )); 
          break;
        case 'P':
            if(!str_cmp(word,"PKL"))
            {
             char *nword;
             nword = fread_string( fp );
             if(!str_cmp(nword,"TRUE"))
              nclan->pkill = TRUE;
             else if(!str_cmp(nword,"FALSE"))
              nclan->pkill = FALSE;
             else
              bug("NCLAN_READ: PKILL not recognized.",0);
             fMatch = TRUE;
             break;
            }
            break;
        case 'R':
            KEY( "REC",   nclan->recall,  fread_number( fp ) );
            break;
        case 'W':
            KEY( "WAR",    nclan->atwar,  fread_flag( fp ) );
            KEY( "WHO",    nclan->who_name,  fread_string( fp ) );
            break;
        default: break;
        }
    }
    return nclan;
}

void read_nclans(void)
{
 FILE *fp, *fp2;
 char buf[256],name[50];
 char *word;
 CLN_DATA *nclan;

 sprintf(buf,"%s/nclans.lst",CLN_DIR);

 if((fp = fopen(buf,"r")) == NULL)
 {
  bug("Could not open the nclan list.",0);
  return;
 }

 for ( ; ; )
 {
  word = feof(fp) ? "End" : fread_word( fp );

  if(!str_cmp(word,"End"))
   break;

  strcpy(name,word);  // For the name...do not remove this

  sprintf(buf,"%s/%s.cln",CLN_DIR,word);

  if((fp2 = fopen(buf,"r")) == NULL)
  {
   bug("Could not open a nclan.",0);
   return;
  }

  nclan = read_nclan(fp2);

  sbug("Loading nClan: %s",nclan->name);

  if(!add_clan_manual(nclan))
  {
   sbug("Could not load %s.",nclan->name);
   exit(0);
  }
  fclose(fp2);
 }
 fclose(fp);

}

void add_mbr_rank(char *name, CLN_DATA *cln, int rank)
{
 MBR_DATA *mbr;
 for(mbr = cln->member_first; mbr != NULL; mbr = mbr->next)
 {
  if(!str_cmp(mbr->name,name))
  {
   mbr->rank = rank;
   break;
  }
 }
 return;
}

bool add_clan_manual( CLN_DATA *cln)
{

 // Check if First Clan
 if(cln_list == NULL || cln_first == NULL)
 {
   cln_list = cln; // Put into position
   cln_first = cln_list; // Make it the first
 }
 else
 {
   cln_list->next = cln; // Point to new node
   /* CLN_LIST -> NEW_CLAN */
   cln_list = cln; // Set cln_list to current position.
   /* OLD_NODE -> CLN_LIST */
 }
 return TRUE;
}

/* This checks if a player's rank in a clan has changed, or
   If they were totally removed. */
void check_cln_member(CHAR_DATA *victim, CLN_DATA *cln, int rank)
{
  bool CHANGED = FALSE;

  if(rank == -1) // Booted
  {
   remove_member(victim->name,cln_lookup(victim->cln));
   victim->cln = str_dup("Loner");
   victim->rank = rank;
   CHANGED = TRUE;
  }
  else
  {
   // If in another Clan, assign new one
   if(victim->cln != NULL && str_cmp(victim->cln,cln->name))
   {
    remove_member(victim->name,cln_lookup(victim->cln));
    add_member(victim->name,cln);   /* make member of the clan */
    strcpy(victim->cln,cln->name); // Safer
    CHANGED = TRUE;
   }
   // If rank isn't the same, make it
   if(victim->rank != rank)
   {
    victim->rank = rank;
    add_mbr_rank(victim->name, cln_lookup(victim->cln), victim->rank);
    CHANGED = TRUE;
   }
  }
  if(CHANGED)
   write_clans();
  return;
}

/* New Rosters */
void do_nroster (CHAR_DATA * ch, char *argument) 
{ 
        static char arg[MAX_INPUT_LENGTH]; 
        static char buf[2 * MAX_STRING_LENGTH];
        CLN_DATA *nclan;
 
        argument = one_argument (argument, arg); 
 
        if ((arg[0] == '\0')) 
        { 
                send_to_char ("Syntax:  roster <clan>\n\r", ch); 
                return; 
        } 
 
        if((nclan = cln_lookup(arg)) == NULL)
        {
                send_to_char ("That's not a clan!\n\r", ch); 
                return; 
        } 
 
   sprintf (buf, "The following list displays the current membership of %s:\n\r",nclan->name); 
   send_to_char (buf, ch); 
 
   sprintf (buf, "{&Members:{x\n\r"); 
   send_to_char (buf, ch); 
 
   sprintf (buf, "      {&Leader{x:"); 
   find_mbr_rank(nclan,7,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
 
   sprintf (buf, "      {^Vice Leaders:{x"); 
   find_mbr_rank(nclan,6,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
 
   sprintf (buf, "      {#Elite Captains{x:"); 
   find_mbr_rank(nclan,5,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 

   sprintf (buf, "      {#Commanders{x:");
   find_mbr_rank(nclan,4,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
 
   sprintf (buf, "      {#Lieutenants{x:"); 
   find_mbr_rank(nclan,3,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 

   sprintf (buf, "      {@Soldiers{x:");
   find_mbr_rank(nclan,2,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
 
   sprintf (buf, "      {!Recruits{x:"); 
   find_mbr_rank(nclan,1,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
 
   sprintf (buf, "Supporters:\n\r "); 
   find_mbr_rank(nclan,0,buf);
   send_to_char (buf, ch); 
   send_to_char ("\n\r", ch); 
}

void find_mbr_rank(CLN_DATA *nclan, int rank, char *argument)
{
 MBR_DATA *mbr;
 char buf[50],buf2[40];
 int i=0;

 buf[0] = '\0';
 buf2[0] = '\0';

 for(mbr = nclan->member_first; mbr != NULL; mbr = mbr->next)
 {
  if(mbr->rank == rank)
  {
   i += 1;
   if(i > 1)
    strcat(buf,",");
   sprintf(buf2," %s",mbr->name);
   strcat(buf,buf2);
   strcat(argument,buf);
  }
 }
 return;
}
 
void do_nrosterpurge (CHAR_DATA * ch, char *argument) 
{ 
        static char arg[MAX_INPUT_LENGTH]; 
        static char buf[MAX_INPUT_LENGTH]; 
        FILE *in;
        CLN_DATA *nclan;
        MBR_DATA *mbr;
 
        one_argument (argument, arg); 
 
        if ((arg[0] == '\0')) 
        { 
                send_to_char ("Syntax:  rosterpurge <clan>\n\r", ch); 
                return; 
        } 
 
        if((nclan = cln_lookup(arg)) == NULL)
        {
                send_to_char ("That's not a clan!\n\r", ch); 
                return; 
        } 

        // Cycle Through Members
        for (mbr = nclan->member_first; mbr != NULL; mbr = mbr->next)
        { 
         sprintf (buf, "%s%s%s%s", PLAYER_DIR, initial (mbr->name),"/", capitalize (mbr->name)); 

         if ((in = fopen (buf, "r")) == NULL) 
         { 
            remove_member(mbr->name,nclan);  // Junk 'Em!
         } 
         else
           fclose (in);

        }
        send_to_char ("Roster purged of nonexistent players.\n\r", ch); 
} 

/* New Clan Leader */

void do_ncleader (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
        CLN_DATA *nclan;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	/* Lists all possible clans */
	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
           send_to_char ("{wClan list:{x\n\r", ch);
           /*
            * Won't print the independent
            */
           for (nclan = cln_first; nclan != NULL; nclan = nclan->next)
           {
               if (nclan->independent == FALSE)
               {
                   sprintf (buf, "   {G%s{x\n\r", nclan->name);
                   send_to_char (buf, ch);
               }
           }

           send_to_char ("\n\rSyntax: {Gcleader {c<{wchar{c> <{wclan name{c>{x\n\r", ch);
           send_to_char ("If {c<{wclan name{c>{x is {r'{wnone{r'{x clan leadership has been removed.\n\r", ch);
           return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("No character by that name exists.\n\r", ch);
		return;
	}

	if (IS_NPC (victim))
	{
                send_to_char ("Obviously you have issues.\n\r", ch);
		return;
	}

	if (!str_cmp (arg2, "none"))
	{
          if (victim->rank == LEADER)
          {
            if((nclan = cln_lookup(victim->cln))==NULL)
            {
             printf_to_char(ch,"%s's clan does not exist. Please remove their membership via guild.\n\r",victim->name);
             return;
            }

            sprintf (buf, "You remove the leadership from %s.\n\r", victim->name);
            send_to_char (buf, ch);
            sprintf (buf, "You aren't the leader of {G%s{x clan anymore!\n\r", nclan->name);
            send_to_char (buf, victim);
            // This one does it all
            check_cln_member(victim, nclan, MEMBER);
            return;
          }
          else
          {
               sprintf (buf, "%s isn't the leader of any clan.\n\r", victim->name);
               send_to_char (buf, ch);
               return;
          }
	}

        if((nclan = cln_lookup(arg2)) == NULL)
        {
             send_to_char ("No such clan exists.\n\r", ch);
             return;
        }

        if(nclan->independent)
        {
         send_to_char("That clan is independent and has no structure.\n\r",ch);
         return;
        }
    	else
	{
          if(!str_cmp(victim->cln, nclan->name) && victim->rank == LEADER)
          {
             sprintf (buf, "They already hold the leadership of %s.\n\r", nclan->name);
             send_to_char (buf, ch);
             return;
          }

          if (victim->level < 50)
          {
              sprintf (buf, "%s's level is too low to be leader of %s clan.\n\r"
                            "All {GLEADERS{x must be {wlevel {r50{x or higher.\n\r",
                            victim->name, nclan->name);
              send_to_char (buf, ch);
              return;
          }
          else
          {
              sprintf (buf, "%s is now the {GLEADER{x of %s.\n\r", victim->name, nclan->name);
              send_to_char (buf, ch);
              sprintf (buf, "You are now the {GLEADER{x of %s.\n\r", nclan->name);
              send_to_char (buf, victim);

              // This one does it all
              check_cln_member(victim, nclan, LEADER);
          }
	}

        return;
}

void do_nclanlist (CHAR_DATA * ch, char *argument)
{
        char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH/10];
        CLN_DATA *nclan;

        // IF in clan....show kills & deaths
        // IF not in clan ... show kills only
        // IF in clan....show War status with other clans

        send_to_char ("{c***********************************************{x\n\r", ch);
        send_to_char ("{c*      {w The Clans of {1A{!s{3g{#a{&r{#d{3i{!a{1n{x {wN{7ig{&htm{7ar{we{c      *{x\n\r", ch);
        send_to_char ("{c***********************************************{x\n\r", ch);
        send_to_char ("{c Clan name   Status   Members   Kills   Deaths {x\n\r", ch);
        send_to_char ("{w-----------------------------------------------{x\n\r", ch);

        for (nclan = cln_first; nclan != NULL; nclan = nclan->next)
	{
         sprintf (buf, " %-12s", nclan->name);
         sprintf (buf2, "%-9s", (nclan->independent == TRUE) ? "TRUE" : "FALSE");
         strcat(buf, buf2);
         sprintf(buf2,"%-10d", nclan->members);
         strcat(buf, buf2);
         sprintf(buf2,"%-8d",nclan->kills);
         strcat(buf, buf2);
         sprintf(buf2,"%-7d\n\r",nclan->deaths);
         strcat(buf, buf2);
         send_to_char (buf, ch);
	}
	return;
}

bool is_nclan(CHAR_DATA *ch)
{
 CLN_DATA *cln;

 if(ch->cln[0] == '\0')
  return FALSE;
 if((cln = cln_lookup(ch->cln))==NULL)
 {
  send_to_char("Your clan does not exist! Inform the Imms.\n\r",ch);
  return FALSE;
 }
 if(!str_cmp(ch->cln,"Outcast"))
  return FALSE;
 if(!str_cmp(ch->cln,"Loner"))
  return FALSE;
 if(cln->independent == TRUE)
  return FALSE;

  return TRUE;
}

bool is_same_nclan(CHAR_DATA *ch, CHAR_DATA *victim)
{
 if(!str_cmp(ch->cln,victim->cln))
  return TRUE;

 return FALSE;
}

void do_nclantalk (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

        if (!is_nclan(ch)) // Takes care of Independent clans too.
	{
		send_to_char ("You aren't in a clan.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_NOCLAN))
		{
			send_to_char ("{RClan{x channel is now {GON{x\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_NOCLAN);
		}

		else
		{
			send_to_char ("{RClan{x channel is now {ROFF{x\n\r", ch);
			SET_BIT (ch->comm, COMM_NOCLAN);
		}
		return;
	}

	if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
		send_to_char ("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}

        if(IS_SET(ch->in_room->room_flags,ROOM_SILENCED) && !IS_IMMORTAL(ch))
        {
         send_to_char("You have lost your powers of speech in this room!\n\r",ch);
         return;
        }
        
	REMOVE_BIT (ch->comm, COMM_NOCLAN);
	sprintf (buf, "{3[{RCLAN{3]{x You '{w%s{x'\n\r", argument);
	send_to_char (buf, ch);

	if (ch->rank >= JUNIOR)
		sprintf (buf, "{3[{x%s - $c{3]{x %s '{w%s{x'{x\n\r", clan_rank_table[ch->rank].rank, ch->name, argument);

	else
		sprintf (buf, "$c{x %s '{w%s{x'{x\n\r", ch->name, argument);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->connected == CON_PLAYING
			&& d->character != ch
                        && d->character->level >= 108
			&& !IS_SET (d->character->comm, COMM_NOCLAN)
			&& !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new ("$c $n '{w$t{x'", ch, argument, d->character, TO_VICT, POS_DEAD);
		}
		else if (d->connected == CON_PLAYING
				 && d->character != ch
                                 && is_same_nclan (ch, d->character)
				 && !IS_SET (d->character->comm, COMM_NOCLAN)
				 && !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
		}
	}

	return;
}

void do_ncgossip (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

        if (!is_nclan(ch))
	{
		send_to_char ("You aren't in a clan.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		if (IS_SET (ch->comm, COMM_NOCLAN))
		{
			send_to_char ("{RClan{x channels are now {GON{x\n\r", ch);
			REMOVE_BIT (ch->comm, COMM_NOCLAN);
		}

		else
		{
			send_to_char ("{RClan{x channels are now {ROFF{x\n\r", ch);
			SET_BIT (ch->comm, COMM_NOCLAN);
		}
		return;
	}

	if (IS_SET (ch->comm, COMM_NOCHANNELS))
	{
		send_to_char ("The gods have revoked your channel priviliges.\n\r", ch);
		return;
	}

	REMOVE_BIT (ch->comm, COMM_NOCLAN);
	sprintf (buf, "{3[{RCGOSSIP{3]{x You '{w%s{x'\n\r", argument);
	send_to_char (buf, ch);

	if (IS_IMMORTAL(ch))
		sprintf (buf, "{W[{rCGossip]-{x{3[{WIMMORTAL{3]{x %s '{w%s{x'{x\n\r", ch->name, argument);
	else if (ch->rank >= JUNIOR)
		sprintf (buf, "{W[{rCGossip]-{x{3[{x%s - $c{3]{x %s '{w%s{x'{x\n\r", clan_rank_table[ch->rank].rank, ch->name, argument);
	else
		sprintf (buf, "{W[{rCGossip]-{x$c{x %s '{w%s{x'{x\n\r", ch->name, argument);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->connected == CON_PLAYING
			&& d->character != ch
                        && d->character->level >= 108
			&& !IS_SET (d->character->comm, COMM_NOCLAN)
			&& !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new ("$c $n '{w$t{x'", ch, argument, d->character, TO_VICT, POS_DEAD);
		}
		else if (d->connected == CON_PLAYING
				 && d->character != ch
                                 && (is_nclan(d->character)) 
                                 && !IS_SET (d->character->comm, COMM_NOCLAN)
				 && !IS_SET (d->character->comm, COMM_QUIET))
		{
                        act_new (buf, ch, argument, d->character, TO_VICT, POS_DEAD);
		}
	}

	return;
}

void do_npromote (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
        CLN_DATA *nclan;

	argument = one_argument (argument, arg1);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not promote someone.\n\r", ch);
		return;
	}

	if ((ch->rank != LEADER) && (!IS_IMMORTAL (ch)))
	{
         if ((ch->rank != (LEADER - 1)) && (!IS_IMMORTAL (ch)))
         {
             send_to_char ("You must be a clan Leader or Vice Leader to promote someone.\n\r", ch);
             return;
         }
	}

	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: {Gpromote {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They must be playing to be promoted.\n\r", ch);
		return;
	}

	if (IS_NPC (victim) )
	{
		send_to_char ("You must be mad.\n\r", ch);
		return;
	}

        if (!is_same_nclan(ch,victim) && (!IS_IMMORTAL(ch)))
	{
		send_to_char ("You can not promote a player who is not in your clan.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not promote yourself.\n\r", ch);
		return;
	}

	if (victim->rank >= SECOND)
	{
		send_to_char ("You can not promote this player anymore.\n\r", ch);
		return;
	}

        if((nclan = cln_lookup(victim->cln))==NULL)
        {
          printf_to_char(ch,"%s doesn't have a clan!.\n\r",victim->name);
          return;
        }

        check_cln_member(victim, nclan, victim->rank + 1);

	sprintf (buf, "They are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, ch);
	sprintf (buf, "You are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, victim);
	return;
}

void do_ndemote (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
        CLN_DATA *nclan;

	argument = one_argument (argument, arg1);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not demote someone.\n\r", ch);
		return;
	}

	if ((ch->rank != LEADER) && (!IS_IMMORTAL (ch)))
	{
         if ((ch->rank != (LEADER-1)) && (!IS_IMMORTAL (ch)))
         {
            send_to_char ("You must be a clan Leader or Vice-Leader to demote someone.\n\r", ch);
            return;
         }
	}

	if (arg1[0] == '\0')
	{
		send_to_char ("Syntax: {Gdemote {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg1)) == NULL)
	{
		send_to_char ("They must be playing to be demoted.\n\r", ch);
		return;
	}

        if (IS_NPC (victim))
	{
		send_to_char ("You must be mad.\n\r", ch);
		return;
	}

        if (!is_same_nclan(ch,victim) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You can not demote a player who is not in your clan.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not demote yourself.\n\r", ch);
		return;
	}

	if (victim->rank <= MEMBER)
	{
		send_to_char ("You can not demote this player anymore.\n\r", ch);
		return;
	}

        if((nclan = cln_lookup(victim->cln))==NULL)
        {
          printf_to_char(ch,"%s doesn't have a clan!.\n\r",victim->name);
          return;
        }

        check_cln_member(victim, nclan, victim->rank - 1);

	sprintf (buf, "They are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, ch);
	sprintf (buf, "You are now {G%s{x of the clan.\n\r", lookup_rank (victim->rank));
	send_to_char (buf, victim);
	return;
}

void do_nexile (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
        CLN_DATA *nclan;

	argument = one_argument (argument, arg);

	if (IS_NPC (ch))
	{
		send_to_char ("NPC's can not demote someone.\n\r", ch);
		return;
	}

	if (ch->rank != SECOND && ch->rank != LEADER && !IS_IMMORTAL (ch))
	{
		send_to_char ("You must be a clan Leader or Auxiliary to exile someone.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char ("Syntax: {Gexile {c<{wchar{c>{x\n\r", ch);
		return;
	}

	if ((victim = get_char_world (ch, arg)) == NULL)
	{
          send_to_char ("They aren't playing.\n\r", ch);
          return;
	}

        if (IS_IMMORTAL(victim))
	{
		send_to_char ("You can't outcast an immortal.", ch);
		return;
	}

        if (IS_NPC(victim))
	{
                send_to_char ("You can not exile an NPC.\n\r", ch);
		return;
	}

        if (!is_same_clan(ch,victim) && (!IS_IMMORTAL (ch)))
	{
		send_to_char ("You can not exile a player who is not in your clan.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char ("You can not exile yourself.\n\r", ch);
		return;
	}

        if((nclan = cln_lookup(victim->cln))==NULL)
        {
          printf_to_char(ch,"%s doesn't have a clan!.\n\r",victim->name);
          return;
        }
        // Remove clan totally
        check_cln_member(victim, nclan, -1);

        /* leader outcasts victim */
	send_to_char ("They are now {GOUTCAST{x from the clan.\n\r", ch);
        sprintf (buf, "You have been {GOUTCAST{x from %s clan!\n\r", nclan->name);
	send_to_char (buf, victim);
	send_to_char ("Type {r'{Ghelp outcast{r'{x for more information.\n\r", victim);

        if((nclan = cln_lookup("Outcast"))==NULL)
        {
          printf_to_char(ch,"No Clan to be outcasted to, %s will now have no clan.\n\r",victim->name);
          return;
        }
        // Re-Instate as OutCast
        check_cln_member(victim, nclan, MEMBER);
	return;
}

void do_nloner (CHAR_DATA * ch, char *argument)
{
        CLN_DATA *nclan;
	if (IS_NPC (ch))
		return;

	if (ch->level > 20 || ch->pcdata->tier > 1)
	{
		send_to_char ("You should have decided that sooner, after level 20, first tier, it's too late.\n\r",ch);
		return;
	}

	if (ch->level < 6)
	{
		send_to_char ("You are still a {GNEWBIE{x, wait until {wlevel {r6{x.\n\r", ch);
		return;
	}

        if (!str_cmp(ch->cln,"Outcast"))        /* IS_OUTCAST */
	{
		send_to_char ("You are an {GOUTCAST{x!  You can't join a clan."
		   "\n\rType {r'{Ghelp outcast{r'{x for more information.\n\r", ch);
		return;
	}

        if (!str_cmp(ch->cln,"Loner"))     
	{
		send_to_char ("You are already a {GLONER{x.\n\r", ch);
		return;
	}

        if (is_nclan (ch))
	{
		send_to_char ("You are already in a Clan.\n\r", ch);
		return;
	}

	if (ch->pcdata->confirm_loner)
	{
		if (argument[0] != '\0')
		{
			send_to_char ("{GLONER{x status removed.\n\r", ch);
			ch->pcdata->confirm_loner = FALSE;
			return;
		}

		else
		{
                   if((nclan = cln_lookup("Loner")) == NULL)
                   {
                    send_to_char("You can't become a Loner right now.\n\r",ch);
                    return;
                   }
                	send_to_char ("{*{wYou are now a brave {GLONER{x!!\n\r", ch);
			ch->pcdata->confirm_loner = FALSE;
                        check_cln_member(ch,nclan,MEMBER);
			return;
		}
	}

	if (argument[0] != '\0')
	{
		send_to_char ("Just type {GLONER{x. No argument.\n\r", ch);
		return;
	}

	do_help (ch, "pkill");
	send_to_char ("\n\r", ch);
	send_to_char ("Type {GLONER{x again to confirm this command.\n\r", ch);
	send_to_char ("{RWARNING:{x this command is irreversible.\n\r", ch);
	send_to_char ("Typing {GLONER{x with an argument will undo delete status.\n\r", ch);
	ch->pcdata->confirm_loner = TRUE;
}

void do_npetition_list (CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;
	bool flag = FALSE;
	char buf[MAX_STRING_LENGTH];
        CLN_DATA *nclan;

	for (d = descriptor_list; d; d = d->next)
	{
		CHAR_DATA *victim;
		victim = d->original ? d->original : d->character;

                if((nclan = cln_lookup(victim->cln))==NULL)
                  continue;

		if (d->connected == CON_PLAYING)
		{
                        if (!str_cmp(victim->npetition,ch->cln))
			{
				if (!flag)
				{
					flag = TRUE;
					send_to_char ("The following characters have petitioned your clan:\n\n\r", ch);
				}

				sprintf (buf, "{c[{w%3d %5s %s{c][{w%-10s%-16s{c] ({w%s{c){x\n\r",
						 victim->level,
						 victim->race < MAX_PC_RACE ? pc_race_table[victim->race].who_name : "     ",
						 class_table[victim->class].who_name,
						 victim->name,
						 IS_NPC (victim) ? "" : victim->pcdata->title,
                                                 nclan->who_name);
				send_to_char (buf, ch);
			}
		}
	}

	if (!flag)
		send_to_char ("No-one has petitioned your clan.\n\r", ch);
}

void do_npetition (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
        short status;
        CLN_DATA *nclan, *yclan;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

        status = (ch->cln[0] != '\0') ? ch->rank : 0;

	if (arg1[0] == 0)
	{
		if (status >= SECOND)
		{
                        do_npetition_list(ch);
			return;
		}

                if (!ch->npetition[0])
		{
			send_to_char ("Syntax: {Gpetition {c<{wclan name{c>{x.\n\r", ch);
			return;
		}

                if (ch->npetition)
		{
                        ch->npetition = NULL;
			send_to_char ("You withdraw your petition.\n\r", ch);
			return;
		}
	}

	if (arg2[0] == 0 && status < SECOND)
	{
                if ((nclan = cln_lookup (arg1)) == NULL)
		{
			send_to_char ("There is no clan by that name.\n\r", ch);
			return;
		}

                if (nclan->independent)
		{
			send_to_char ("{GOUTCAST{x and {GLONER{x aren't clans!\n\r", ch);
			return;
		}

                if ((yclan = cln_lookup(ch->cln)) == NULL)
                {
                 send_to_char("You aren't in a clan.\n\r",ch);
                 return;
                }

                if(nclan == yclan)
                {
                        sprintf (buf, "You are already a member of {G%s{x clan.\n\r", yclan->who_name);
			send_to_char (buf, ch);
			return;
		}

                if (yclan->independent == TRUE && nclan->pkill == TRUE)
		{
			send_to_char ("You must be at least a {GLONER{x to join a clan.\n\r", ch);
			return;
		}
		else
		{
                        ch->npetition = str_dup(nclan->name);
			sprintf (buf, "You have petitioned {G%s{x clan for membership.\n\r",
                                         nclan->name);
			send_to_char (buf, ch);
			return;
		}
	}

	if (status >= SECOND)
	{
                if ((yclan = cln_lookup(ch->cln)) == NULL)
                {
                 send_to_char("You aren't in a clan.\n\r",ch);
                 return;
                }

		if (!str_prefix (arg1, "accept"))
		{

			if ((victim = get_char_world (ch, arg2)) == NULL)
			{
				send_to_char ("They are not playing.\n\r", ch);
				return;
			}

                        if (str_cmp(victim->npetition,ch->cln))
			{
				send_to_char ("They have not petitioned your clan.\n\r", ch);
				return;
			}

                        check_cln_member(victim, yclan, MEMBER);
                        victim->npetition = NULL;

			send_to_char ("You have accepted them into your clan.\n\r", ch);
			send_to_char ("Your clan application was successful.\n\r", victim);
			sprintf (buf, "You are now a proud member of clan {G%s{x.\n\r",
                                         yclan->who_name);
			send_to_char (buf, victim);
			return;
		}
		else if (!str_prefix (arg1, "reject"))
                {
                 if ((yclan = cln_lookup(ch->cln)) == NULL)
                 {
                  send_to_char("You aren't in a clan.\n\r",ch);
                  return;
                 } 

                        if ((victim = get_char_world (ch, arg2)) == NULL)
			{
				send_to_char ("They are not playing.\n\r", ch);
				return;
			}

                        if (str_cmp(victim->npetition,ch->cln))
			{
				send_to_char ("They have not petitioned your clan.\n\r", ch);
				return;
			}

                        victim->npetition = NULL;
			send_to_char ("You have rejected there application.\n\r", ch);
			send_to_char ("Your clan application has been rejected.\n\r", victim);
			return;
		}

		send_to_char ("Syntax: {Gpetition accept {c<{wplayer{c>{x\n\r"
					  "        {Gpetition reject {c<{wplayer{c>{x\n\r", ch);
		return;
	}

	send_to_char ("Syntax: {Gpetition {c<{wclan name{c>{x.\n\r", ch);
	return;
}

/* This is temp -- to set to new clans */
void do_reclan(CHAR_DATA *ch, char *argument)
{
 CLN_DATA *nclan;
 int clan;

 if(IS_IMMORTAL(ch))
 {          
  if(!str_cmp(argument,"copyclans"))
  {
   for(clan = 0; clan < MAX_CLAN; clan++)
   {
    if((nclan = cln_lookup(clan_table[clan].name)) == NULL)
    {
     nclan = new_cln();
     nclan->name = str_dup(capitalize(clan_table[clan].name));
     if(nclan->name[0] == '\0' || nclan->name[0] == ' ')
      continue;
     nclan->who_name = str_dup(clan_table[clan].who_name);
     nclan->recall = clan_table[clan].recall;
     nclan->pkill = clan_table[clan].pkill;
     nclan->independent = clan_table[clan].independent;
     if(!add_clan_manual(nclan))
     {
      send_to_char("clan add failed.\n\r",ch);
      return;
     }
     printf_to_char(ch,"New clan %s added.\n\r",nclan->name);
    }
   }
   return;
  }
 }

 if(ch->cln[0] == '\0')
 {
  send_to_char("You have been saved. Please Reconnect.\n\r",ch);
  do_quit(ch,"");
 }

 if((nclan = cln_lookup(ch->cln)) == NULL)
 {
  send_to_char("Your Clan does not exist yet in the new setup. Try again later.\n\r",ch);
  return;
 }

 check_cln_member(ch,nclan,ch->rank);
 ch->clan = -1;
 send_to_char("You have been RECLANNED!\n\r",ch);
 send_to_char("Temporarily all known commands are now preceeded with an N.\n\r",ch);
 send_to_char("For Example: clantalk is now nclantalk and clist is nclist.\n\r",ch);
 send_to_char("Other commands affected: petition, exile, promote, demote, cgossip.\n\r",ch);
 return;
}

