/* New Clan System Written For Asgardian Nightmare
 * by Skyntil(Gabe Volker)
 */

#include "include.h"

CLN_DATA *cln_lookup(char *argument);
CLN_DATA *new_cln(void);
void free_cln(CLN_DATA *clan);
MBR_DATA *new_mbr(void);
bool remove_member(char *argument, CLN_DATA *cln);
void add_member(char *argument, CLN_DATA *clan);
void show_clans(CHAR_DATA *ch, char *argument);
void free_mbr(MBR_DATA *mbr);
MBR_DATA *mbr_lookup(char *argument, CLN_DATA *clan);
bool add_clan(char *argument);
bool remove_clan(char *argument);

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
 char buf[256],buf2[256];
 int i;

 buf[0] = '\0';
 buf2[0] = '\0';

 if(cln_first == NULL || cln_list == NULL)
 {
  send_to_char("There are no clans loaded.\n\r",ch);
  return;
 }

 for(i = 0; i < 15; i++)
  strcat(buf,"=");
 strcat(buf,"-NCLANS-");
 for(i = 0; i < 15; i++)
  strcat(buf,"=");

 printf_to_char(ch,"\n\r%-30s\n\r",buf);

 for(clan = cln_first; clan != NULL; clan = clan->next)
 {
  sprintf(buf,"| Name: %s |\n\r",clan->name);
  send_to_char(buf,ch);

  if(clan->member_list == NULL || clan->member_first == NULL)
   send_to_char("|  Members: None |\n\r",ch);
  else
  {
   for(mbr = clan->member_first; mbr != NULL; mbr = mbr->next)
   {
    sprintf(buf2,"|  Member: %s |\n\r",mbr->name);
    send_to_char(buf2,ch);
   }
  }
 }

 buf[0] = '\0';

 for(i = 0; i < 38; i++)
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
 CLN_DATA *nclan;

 if(IS_NPC(ch))
  return;

 argument = one_argument(argument,arg);

 if(!str_cmp(arg,"show"))
 {
  show_clans(ch,arg);
  return;
 }

 if(!str_cmp(arg,"add"))
 {
  one_argument(argument,arg);
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
  one_argument(argument,arg);
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
  argument = one_argument(argument,arg);
  one_argument(argument,arg2);
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
  argument = one_argument(argument,arg);
  one_argument(argument,arg2);
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

 send_to_char("Nclan [show|add|remove|memadd|memremove]\n\r",ch);
 return;
}
