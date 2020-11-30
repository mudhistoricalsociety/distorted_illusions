/* Pfile Parser exclusively for Asgardian Nightmares
 * by Tien (Robert Yearwood)
 */
#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>      
#include <unistd.h>     
#include <sys/time.h>
#endif
#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
#include "clan.h"

#define MAX_NEST        150
static  OBJ_DATA *  rgObjNest   [MAX_NEST];

void scan_pfiles (void) 
{
    char strsave[MAX_STRING_LENGTH];
    char currfile[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    char period[MAX_STRING_LENGTH];
    char period2[MAX_STRING_LENGTH];
    DIR * dir_p;
    struct dirent * dir_entry_p;
     sprintf(period,".");
     sprintf(period2,"..");
  sprintf(strsave, "%sa", PLAYER_DIR);
  dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {   
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
    sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);

   sprintf(strsave, "%sb", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
    sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sc", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
    sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sd", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
    sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%se", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sf", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
   if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sg", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sh", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%si", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sj", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sk", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sl", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sm", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sn", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%so", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sp", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sq", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sr", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%ss", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%st", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%su", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sv", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sw", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sx", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sy", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
   sprintf(strsave, "%sz", PLAYER_DIR);
    dir_p = opendir(strsave);
    while( NULL != (dir_entry_p = readdir(dir_p)))
  {
    if (strcmp(dir_entry_p->d_name,period) && strcmp(dir_entry_p->d_name,period2))
   {
    sprintf(currfile,"%s/%s",strsave,dir_entry_p->d_name);
        sprintf(name,"%s",dir_entry_p->d_name);
    scan_file(currfile, name);
   }
  }
  closedir(dir_p);
}
void scan_file( char *file , char *name )
{
    char strsave[MAX_STRING_LENGTH];
  /*  char buf[MAX_STRING_LENGTH]; */
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

    sprintf(strsave,file);
    ch = new_char();
    ch->pcdata = new_pcdata();

    ch->desc                = NULL;
    ch->name                = str_dup( name );
    ch->id              = get_pc_id();
    ch->race                = race_lookup("human");
    ch->act             = PLR_NOSUMMON;
    ch->comm                = COMM_COMBINE 
                    | COMM_PROMPT
                    | COMM_STORE;
    ch->prompt              = str_dup("<%hhp %mm %vmv> ");
    ch->pcdata->confirm_delete      = FALSE;
    ch->pcdata->pwd         = str_dup( "" );
    ch->pcdata->bamfin          = str_dup( "" );
    ch->pcdata->bamfout         = str_dup( "" );
    ch->pcdata->identity                = str_dup( "" );
    ch->pcdata->immskll                 = str_dup( "" );
    ch->pcdata->who_descr       = str_dup( "" );
    ch->pcdata->title           = str_dup( "" );
    ch->pcdata->pretit                  = str_dup( "" ); 
/*    ch->pcdata->colour_name             = str_dup( "" ); */
    ch->pcdata->usr_ttl                 = FALSE;
    ch->gladiator                       = NULL;
    ch->pcdata->tier                    = 1;
    for (stat =0; stat < MAX_STATS; stat++)
    ch->perm_stat[stat]     = 13;
    ch->pcdata->condition[COND_THIRST]  = 48; 
    ch->pcdata->condition[COND_FULL]    = 48;
    ch->pcdata->condition[COND_HUNGER]  = 48;
    ch->pcdata->security        = 0;    /* OLC */
    ch->pload = NULL;                          /* Skyn */
    ch->pcdata->power[POWER_KILLS] = 0;     /* Highlander */
    ch->pcdata->power[POWER_POINTS] = 0;
    ch->pcdata->power[POWER_LEVEL] = 0;
    ch->ghost = str_dup( "" );
    ch->ghost_timer = -1;
    ch->regen_rate = 0;
    ch->mana_regen_rate = 0;
    ch->toughness = 0;
    ch->pcdata->status = 0;
    for(stat =0; stat < 6; stat++)
     ch->magic[stat] = 1;

    for(stat =1; stat < 11; stat++)
     ch->stance[stat]=1;

    /* Sets all skills to not learned before read of what they know */
    for(stat = 0; stat < MAX_SKILL; stat++)
     ch->pcdata->learned[stat] = -1;

    ch->morph = NULL;
    ch->pcdata->target = NULL;
    ch->morph_form[0] = 0;
  
    found = FALSE;
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
    int iNest;

    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
        rgObjNest[iNest] = NULL;

    found = TRUE;
    for ( ; ; )
    {
        char letter;
        char *word;

        letter = fread_letter( fp );
        if ( letter == '*' )
        {
        fread_to_eol( fp );
        continue;
        }

        if ( letter != '#' )
        {
        bug( "Load_char_obj: # not found.", 0 );
        break;
        }

        word = fread_word( fp );
        if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
        else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
        else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
        else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
        else if ( !str_cmp( word, "END"    ) ) break;
        else
        {
        bug( "Load_char_obj: bad section.", 0 );
        break;
        }
    }
    fclose( fp );
    }

    if (ch->pcdata->prank <= 0)
    {
        ch->pcdata->prank = 1500;
        ch->pcdata->prank += ch->pcdata->pkills * 2;
        ch->pcdata->prank -= ch->pcdata->pdeath * 2;
    }

    if (ch->pcdata->arank <= 0)
        ch->pcdata->arank = 1500;

    /* initialize race */
    if (found)
    {
    int i;

    if (ch->race == 0)
        ch->race = race_lookup("human");

    ch->size = pc_race_table[ch->race].size;
    ch->dam_type = 17; /*punch */

    for (i = 0; i < 5; i++)
    {
        if (pc_race_table[ch->race].skills[i] == NULL)
        break;
        group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
    }
    ch->affected_by = ch->affected_by|race_table[ch->race].aff;
    ch->shielded_by = ch->shielded_by|race_table[ch->race].shd;
    ch->imm_flags   = ch->imm_flags | race_table[ch->race].imm;
    ch->res_flags   = ch->res_flags | race_table[ch->race].res;
    ch->vuln_flags  = ch->vuln_flags | race_table[ch->race].vuln;
    ch->form    = race_table[ch->race].form;
    ch->parts   = race_table[ch->race].parts;
    }

    
    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
    ch->pcdata->learned[gsn_recall] = 50;
    }
 
    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35))
    {
    switch (ch->level)
    {
        case(40) : ch->level = 60;  break;  /* imp -> imp */
        case(39) : ch->level = 58;  break;  /* god -> supreme */
        case(38) : ch->level = 56;  break;  /* deity -> god */
        case(37) : ch->level = 53;  break;  /* angel -> demigod */
    }

        switch (ch->trust)
        {
            case(40) : ch->trust = 60;  break;  /* imp -> imp */
            case(39) : ch->trust = 58;  break;  /* god -> supreme */
            case(38) : ch->trust = 56;  break;  /* deity -> god */
            case(37) : ch->trust = 53;  break;  /* angel -> demigod */
            case(36) : ch->trust = 51;  break;  /* hero -> hero */
        }
    }

    /* ream gold */
    if (found && ch->version < 4)
    {
    ch->gold   /= 100;
    }

    // Do the testing stuff here.
    reset_char(ch);
    save_char_obj(ch);
    free_char(ch);
    return;
}
