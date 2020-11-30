#define INDEP           0
#define LONER           1
#define OUTCAST         2
#define DEMON           9
 
#define MEMBER          0
#define JUNIOR          1
#define SENIOR          2
#define RANK_LIEUTENANT 3
#define DEPUTY          4
#define RANK_ELITE      5
#define SECOND          6 
#define LEADER          7

/* Clan Flags */
#define CLAN_DOWN      (A)

 
int clan_lookup         (const char *name); 
//extern  struct  clan_type               clan_table[MAX_CLAN]; 
//extern  const   struct  clan_titles             clan_rank_table[MAX_RANK]; 
extern  const   char *  lookup_rank(int);
typedef struct clan_ranks CLAN_RANK;
 
 
struct clan_type 
{
        char *  name;                   /* name                                 */
        char *  who_name;               /* who entry                    */ 
        char *  god;                    /* sacrifice god                */ 
        sh_int  deathroom;              /* death-transfer room  */ 
        sh_int  recall;                 /* recall room                  */ 
        bool    independent;    /* true for loners              */ 
        bool    pkill;                  /* true for pkill clans */
        sh_int  kills;
        sh_int  deaths;
        sh_int  coclan;
        sh_int  guardian_vnum;
        sh_int  statue_vnum;
        sh_int  key_vnum;
        sh_int  altar_vnum;
        sh_int  clan_item_vnum;
        char *  guardian_say;
        char *  statue_say;
        sh_int  altar_room;
        char *  clan_skill1;
        char *  clan_skill2;
        char *  clan_skill3;
        char *  clan_skill4;
        char *  clan_skill5;
        char *  clan_skill6;
        CLAN_RANK *ranks;
}; 
 extern  struct  clan_type               clan_table[MAX_CLAN];
struct clan_titles 
{ 
        char *rank; 
};
extern  const   struct  clan_titles             clan_rank_table[MAX_RANK];
struct clan_ranks
{
   char *leader;
   char *vice;
   char *military;
   char *economy;
   char *rank1;
   char *rank2;
   char *rank3;
};

void save_clan_list args((void));
void load_new_clans_list args((void));
void save_clans args(( void ));
