/***************************************************************************
*  Random Object Code - Written Exclusively for Asgardian Nightmare Mud    *
*  by Chris Langlois(tas@intrepid.inetsolve.com) and Gabe Volker           *
***************************************************************************/ 

#include "include.h"
#include "recycle.h"

#define LOWER_RANDOM_VNUM	700
#define UPPER_RANDOM_VNUM	825
#define LOWER_UNIQUE_VNUM	850
#define UPPER_UNIQUE_VNUM	936
#define MAX_PREFIX		90
#define MAX_SUFFIX              125
#define MAX_EXCEP_APPLY         13

// Local Functions
void    process_mods         args( ( OBJ_DATA * rand_obj, int mod_number, bool prefix ) );
OBJ_DATA * rand_obj          args( (CHAR_DATA * ch, int mob_level ) );
OBJ_DATA * rand_obj2         args( (CHAR_DATA * ch, int mob_level, char *argument ) );
OBJ_DATA * make_cracked      args( (OBJ_DATA *obj) );
OBJ_DATA * make_exceptional  args( (OBJ_DATA *obj) );
OBJ_DATA * make_special      args( (OBJ_DATA *obj, int mob_level, int rand_type, char *prefix, char *suffix) );
void show_randobj2_types_cmds args(( CHAR_DATA *ch ));
bool is_prefix args((char *word));
bool is_suffix args((char *word));
int prefix_lookup args((char *prefix));
int suffix_lookup args((char *suffix));
void show_prefixes args(( CHAR_DATA *ch ));  
void show_suffixes args(( CHAR_DATA *ch ));

struct randobj2_types
{
    char *type;
    int index;
    char *desc;
};

struct excep_apply_data
{
    int apply_type;
    int min;
    int max;
};

struct prefix_data
{
	char *name;
	int level;
	int align;
	int affect;
	int resist;
	int vulner; 
	int mod_one;
	int min_one;
	int max_one;
	int mod_two;
	int min_two;
	int max_two;
	int mod_three;
	int min_three;
	int max_three;
};

struct suffix_data
{
	char *name;
	int level;
	int align;
	int affect;
	int resist;
	int vulner; 
	int mod_one;
	int min_one;
	int max_one;
	int mod_two;
	int min_two;
	int max_two;
	int mod_three;
	int min_three;
	int max_three;
};

const struct  randobj2_types arg_table    []      =
{
    /* { Name, Index }, */
    { "unique",      0, "Loads a Unique Item" },
    { "normal",      1, "Loads a Normal Item" },
    { "cracked",     2, "Loads a Cracekd Item" },
    { "exceptional", 3, "Loads an Exceptional Item" },
    { "special",     4, "Loads a Special Item" },
    { "cspecial",    5, "Loads a Cracked Special Item" },
    { "especial",    6, "Loads an Exceptional Item" },
    { "list",        7, "Displays this List" },
    {  NULL,         8, NULL}
};

const struct excep_apply_data eapply_table [] =
{
    /* { Apply_Type, Min, Max }, */
    { APPLY_STR, 1,  2 },
    { APPLY_INT, 1,  2 },
    { APPLY_WIS, 1,  2 },
    { APPLY_DEX, 1,  2 },
    { APPLY_CON, 1,  2 },
    { APPLY_AGE, 1,  20 },
    { APPLY_MANA, 1,  35 },
    { APPLY_MOVE, 1,  35 },
    { APPLY_HIT, 1,  35 },
    { APPLY_HITROLL, 1,  10 },
    { APPLY_DAMROLL, 1,  7 },
    { APPLY_SAVES, -1,  -3 },
    { APPLY_AC, -1,  -15 },
    { 0, 0, 0}
};

const	struct	prefix_data		prefix_table	[]	=
{
     /* name,		level,		align,
	affect,			resist,		vulner,
	mod_one,		min_one,	max_one,
	mod_two,		min_two,	max_two,
	mod_three,		min_three,	max_three */
	
	{ "Arcane",		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		75,		150,
	APPLY_SAVES,		-5,		-2,
	APPLY_INT,		1,		3	},
	
	{ "Crippling",	60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		-3,		-3,
	APPLY_STR,		-3,		-3,
	APPLY_INT,		3,		5	},
	
     /* START OF AC ITEMS WITH VARIOUS OTHER TYPES      */

	{ "Altruistic",		 101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-15,		15,
	APPLY_DAMROLL,		-15,		15,
	APPLY_NONE,		0,		0 },
	
	{ "Elders",   	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		0,		5,
	APPLY_AGE,		0,		15,
	APPLY_NONE,		0,		0	},

    	{ "Distressed",   	0,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		1,		10,
	APPLY_DAMROLL,		2,		10,
	APPLY_NONE,		0,		0	},

    	{ "Athletic",   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-5,		-1,
	APPLY_HITROLL,		2,		10,
	APPLY_NONE,		0,		0	},

    	{ "Solid",   		7,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-10,		-6,
	APPLY_DAMROLL,		-10,		-2,
	APPLY_NONE,		0,		0	},

    	{ "Rugged",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-15,		-10,
	APPLY_HITROLL,		-10,		-2,
	APPLY_NONE,		0,		0	},

    	{ "Energetic", 		25,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-15,		-10,
	APPLY_AGE,		-20,		-3,
	APPLY_DAMROLL,		5,		10	},

    	{ "Durable",	   	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-14,		-8,
	APPLY_HIT,		50,		75,
	APPLY_NONE,		0,		0	},
	
	{ "Firm",   		35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-5,		-15,
	APPLY_MANA,		50,		100,
	APPLY_NONE,		0,		0	},

	{ "Robust",	   	40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-12,		-5,
	APPLY_MOVE,		25,		75,
	APPLY_NONE,		0,		0	},

    	{ "Powerful",   	45,	        350,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-13,		-10,
	APPLY_HIT,		15,		60,
	APPLY_MANA,		40,		75	},

     /* START OF NEGATIVE DAMROLL ITEMS                 */

    	{ "Broken",	   	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		-30,		-15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Damaged",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		-15,		-5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},
    
     /* START OF BAD AC ONLY ITEMS                      */

	{ "Vulnerable",   	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		10,		25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Rusted",	   	0,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		1,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF GOOD AC ONLY ITEMS                     */

    	{ "Sturdy",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-5,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Fine",   		10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-10,		-6,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Strong",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-15,		-10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Grand",   		30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-20,		-15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Valiant",	   	40,		100,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-25,		-20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Glorious",   	50,		50,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-30,		-25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Blessed",	   	60,		300,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-35,		-30,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Saintly",	   	70,		500,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-38,		-33,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Awesome",	   	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-42,		-35,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Holy", 	  	90,		700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-45,		-40,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Godly",   		101,		900,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-50,		-42,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE DAMROLL ITEMS                 */

    	{ "Useless", 	  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,	        -30,		-15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Bent",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,	        -15,		-5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Weak",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		-1,		-5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE DAMROLL ITEMS                 */

    	{ "Jagged", 	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		1,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Deadly",	   	20,		-100,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		3,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Heavy",   		40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vicious",  	 	50,		-150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		7,		9,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Brutal",		60,		-200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		9,		11,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Massive",	   	70,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		11,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Savage",	   	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		13,		18,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Ruthless",   	90,		-300,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		15,		22,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Merciless",   	101,		-400,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		17,		24,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE HITROLL ITEMS                 */

     	{ "Tin",	   	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		-30,		-10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Brass",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		-10,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE HITROLL ITEMS                 */

    	{ "Bronze",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		1,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Iron",	   	10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		3,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Steel",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Silver",	   	40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		7,		9,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Gold",   		60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		9,		11,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Platinum",	   	70,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		11,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Mithril",	   	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		15,		20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Meteoric",   	90,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		20,		25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Ethereal",	   	101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HITROLL,		25,		30,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE HR AND DR ITEMS               */

    	{ "Clumsy",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		-10,		-1,
	APPLY_HITROLL,		-20,		-10,
	APPLY_NONE,		0,		0	},

    	{ "Dull",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		-20,		-10,
	APPLY_HITROLL,		-10,		-1,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE HR AND DR ITEMS               */

	{ "Sharp",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		1,		3,
	APPLY_HITROLL,		1,		3,
	APPLY_NONE,		0,		0	},

    	{ "Warrior's",   	10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		3,		5,
	APPLY_HITROLL,		3,		5,
	APPLY_NONE,		0,		0	},

    	{ "Soldier's",   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		5,		7,
	APPLY_HITROLL,		5,		7,
	APPLY_NONE,		0,		0	},

    	{ "Knight's",   	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		7,		9,
	APPLY_HITROLL,		7,		9,
	APPLY_NONE,		0,		0	},

    	{ "Lord's",  	 	60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		9,		11,
	APPLY_HITROLL,		9,		11,
	APPLY_NONE,		0,		0	},

    	{ "Master's",   	70,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		11,		15,
	APPLY_HITROLL,		11,		15,
	APPLY_NONE,		0,		0	},

    	{ "Champion's",   	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		13,		20,
	APPLY_HITROLL,		13,		20,
	APPLY_NONE,		0,		0	},

    	{ "Avatar's",   	90,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		18,		25,
	APPLY_HITROLL,		18,		25,
	APPLY_NONE,		0,		0	},

    	{ "King's",  	 	101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		20,		26,
	APPLY_HITROLL,		20,		26,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE MANA ONLY ITEMS               */

    	{ "Hyena's", 	  	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		-100,		-25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Frog's",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		-25,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE MANA ONLY ITEMS               */

    	{ "Lizard's",   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		1,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Snake's",		10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		10,		20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Raven's",		20,		-100,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		20,		30,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Serpent's",		30,		-150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		30,		50,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Drake's",		40,		-150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		50,		70,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Dragon's",		50,		-200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		70,		100,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Wyrm's",		60,		-200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		100,		130,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Hydra's",		70,		-250,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MANA,		130,		200,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF BAD SAVES ONLY ITEMS                   */

    	{ "Putrid",		60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		5,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vile",		20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		2,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Murky",		1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF GOOD SAVES ONLY ITEMS                  */

    	{ "White",		1,		100,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-1,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Pearl",		10,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-2,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Amber",		20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-2,		-2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Ivory",		30,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-3,		-2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Topaz",		40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-3,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Crystal",		50,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-4,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Jade",		60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-4,		-4,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Diamond",		70,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-5,		-5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Obsidian",		80,		-200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-7,		-5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Emerald",		90,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-10,		-7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF RESIST ITEMS                           */

    	{ "Ruby",		90,		0,
	NO_FLAG,		RES_FIRE,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Coral",		90,		0,
	NO_FLAG,		RES_LIGHTNING,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Viridian",		90,		0,
	NO_FLAG,		RES_POISON,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Sapphire",		90,		0,
	NO_FLAG,		RES_COLD,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Beryl",		90,		0,
	NO_FLAG,		RES_ACID,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Mirrored",		90,		0,
	NO_FLAG,		RES_NEGATIVE,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Dark",		90,		-300,
	NO_FLAG,		RES_HOLY,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Azure",		90,		0,
	NO_FLAG,		RES_MENTAL,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Prismatic",		90,		0,
	NO_FLAG,		RES_SOUND,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Black",		90,		0,
	NO_FLAG,		RES_LIGHT,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Lapis",		90,		0,
	NO_FLAG,		RES_DISEASE,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vibrant",		90,		0,
	NO_FLAG,		RES_CHARM,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},
	
     /* START OF AFFECT ITEMS                           */

    	{ "Veiled",		40,		-100,
	AFF_BLIND,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Pink",		10,		0,
	AFF_FAERIE_FIRE,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Blighted",		25,		-150,
	AFF_POISON,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

	{ "Blackened",		50,		-200,
	AFF_POISON,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Placid",		1,		0,
	AFF_CALM,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Infested",		30,		0,
	AFF_PLAGUE,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Deadened",		15,		0,
	AFF_WEAKEN,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Slow",		20,		0,
	AFF_SLOW,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,      	-5,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ NULL,			0,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	}
 };



const	struct	suffix_data		suffix_table	[]	=
{
    /*  name,		        level,	        align,
	affect,		        resist,	        vulner,
	mod_one,		min_one,	max_one,
	mod_two,		min_two,	max_two,
	mod_three,		min_three,	max_three */
	
     	{ "the Wild",		101,            0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_REGEN,		5,		15,
	APPLY_MANA_REGEN,	5,		15,
	APPLY_NONE,		0,		0       },

	{ "Rancor",		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		1,		3,
	APPLY_DAMROLL,		20,		30,
	APPLY_HIT,		-150,		-75     },
 
     /* START OF BAD AC ONLY RANDOMS                    */
	
        { "Pain",	   	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		10,		25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},         

      	{ "Tears",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		1,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF GOOD AC ONLY RANDOMS                   */
 
        { "Health", 	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-7,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     	{ "Protection",	 	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-15,		-7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Absorbtion",	  	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-20,		-15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Life",   		40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-30,		-20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Osmosis", 	  	60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-40,		-25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE DAMROLL ONLY ITEMS            */

        { "Brittleness",   	35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,	        -30,		-10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Fragility",	   	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,	        -10,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE DAMROLL ONLY ITEMS            */

        { "Craftsmanship",	0,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		1,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Quality",		5,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		3,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSIVITE DAMROLL EVIL ALIGN ONLY ITEMS */

        { "Maiming",		30,		-100,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Slaying",	  	45,		-200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		7,		9,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Gore",  		60,		-300,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		10,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Carnage",		85,		-400,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		15,		20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Slaughter",		101,		-500,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		20,		30,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE NEGATIVE MOVE ONLY ITEMS           */

        { "the Meek",		25,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		-100,		-25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Lethargy", 	 	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		-25,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE MOVE ONLY ITEMS           */

        { "Readiness",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		1,		20,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     	{ "Alacrity",	  	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		20,		50,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     	{ "Swiftness",	  	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		50,		100,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     	{ "Quickness",	  	45,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_MOVE,		100,		250,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE NEGATIVE AGE ONLY ITEMS            */

    	{ "Youth",	  	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AGE,		-10,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE AGE ONLY ITEMS            */

	{ "Age",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AGE,		5,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

	{ "the Ages",	  	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AGE,		5,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

        { "Time",               101,            0,
        NO_FLAG,                NO_FLAG,        NO_FLAG,
        APPLY_AGE,              9,		19,
        APPLY_STR,              -3,		-1,
        APPLY_NONE,		0,		0       }, 

     /* START OF THE NEGATIVE DEX ONLY ITEMS            */

    	{ "Paralysis",  	22,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		-12,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Atrophy",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		-3,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE DEX ONLY ITEMS            */

      	{ "Dexterity",  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Skill",  		10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		2,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Accuracy",	  	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		4,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Precision",  	35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Perfection",  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		7,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE NEGATIVE INT ONLY ITEMS            */

    	{ "the Fool",	  	22,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		-15,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

	{ "Dyslexia",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		-3,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE INT ONLY ITEMS            */

     	{ "Energy",  		1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Mind",	  	10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		2,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Brilliance",  	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		4,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Sorcery",	  	35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Wizardry",		50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		7,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE NEGATIVE STR ONLY ITEMS            */

    	{ "Frailty",  	22,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		-12,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Weakness",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		-3,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE STR ONLY ITEMS            */

    	{ "Strength",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Might",  		10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		2,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Ox",  		20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		4,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Giant",  	35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Titan",  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		7,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE CON ONLY ITEMS                */

	{ "Illness",	  	25,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		-15,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Disease",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		-3,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE CON ONLY ITEMS                */

    	{ "Vitality",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Zest",  		10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		2,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vim",  		20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		4,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vigor",  		40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Balance",  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		7,		10,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF NEGATIVE WIS ONLY ITEMS                */

    	{ "the Idiot",  	25,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		-15,		-3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Dazed",  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		-3,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE POSITIVE WIS ONLY ITEMS            */

    	{ "Wisdom",  		1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		1,		2,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Clarity",	  	10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		2,		3,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Elightenment",  	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		4,		5,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Sage",	  	35,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		5,		7,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Ancients",  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_WIS,		5,		15,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF VARIOUS NEGATIVE ATTRIBUTE ITEMS       */

    	{ "Trouble",	  	60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		-10,		-5,
	APPLY_CON,		-10,		-5,
	APPLY_DEX,		-10,		-5	},

    	{ "the Pit",	  	15,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_CON,		-4,		-1,
	APPLY_INT,		-4,		-1,
	APPLY_WIS,		-4,		-1	},

     /* START OF VARIOUS POSITIVE ATTRIBUTE ITEMS      */

    	{ "the Sky",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		1,		2,
	APPLY_WIS,		1,		2,
	APPLY_CON,		1,		2	},

    	{ "the Moon",	  	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		2,		3,
	APPLY_STR,		2,		3,
	APPLY_CON,		2,		3	},

    	{ "the Stars",  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		3,		4,
	APPLY_WIS,		3,		4,
	APPLY_DEX,		3,		4	},

    	{ "the Heavens",  	70,		200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		4,		5,
	APPLY_STR,		4,		5,
	APPLY_WIS,		4,		5	},

    	{ "the Zodiac",  	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		5,		6,
	APPLY_DEX,		5,		6,
	APPLY_CON,		5,		6	},

     /* START OF NEGATIVE HP ITEMS                      */

    	{ "the Vulture",  	25,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		-100,		-25,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Jackal",  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		-25,		-1,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF POSITIVE HP ITEMS                      */

    	{ "the Fox",	  	1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		25,		55,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Jaguar",  	10,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		35,		55,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Eagle",  	20,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		35,		40,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Wolf"	,  	30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		20,		30,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Tiger",  	40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		30,		50,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Lion",	  	50,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		50,		70,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Mammoth",  	60,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		70,		100,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Whale",  	70,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		100,		130,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Colossus",  	80,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		130,		200,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF GOD TYPE ITEMS                         */

   	{ "Njord",  		1,		-150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		1,		5,
	APPLY_CON,		1,		5,
	APPLY_WIS,		1,		5	},

    	{ "Freya",  		5,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-2,		-1,
	APPLY_MANA,		10,		50,
	APPLY_CON,		1,		3	},

    	{ "Freyr",  		10,		700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		1,		2,
	APPLY_WIS,		2,		3,
	APPLY_MOVE,		100,		200	},

    	{ "Magni",  		15,		150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		5,		15,
	APPLY_MANA,		30,		50,
	APPLY_REGEN,		1,		10	},

    	{ "Heimdall",	  	20,		500,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		40,		100,
	APPLY_MANA,		40,		100,
	APPLY_MOVE,		40,		100	},

    	{ "Tyr",  		25,		700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_STR,		1,		6,
	APPLY_HITROLL,		5,		15,
	APPLY_DAMROLL,		2,		15	},

    	{ "Balder",  		30,		500,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		60,		80,
	APPLY_MANA,		60,		80,
	APPLY_MOVE,		60,		80	},

    	{ "Loki",  		40,		-300,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		2,		5,
	APPLY_SAVES,		-8,		-5,
	APPLY_HITROLL,		20,		35	},

    	{ "the Fenrir Wolf",	50,		-700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		100,		130,
	APPLY_MANA,		100,		130,
	APPLY_DAMROLL,		1,		10	},

    	{ "Thor",  		101,		-150,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		100,		130,
	APPLY_HITROLL,		10,		20,
	APPLY_DAMROLL,		10,		20	},

    	{ "Hel",  		101,		-700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		13,		15,
	APPLY_HITROLL,		13,		15,
	APPLY_SAVES,		-7,		0	},

    	{ "Midgaard Serpent",	70,		-700,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		150,		200,
	APPLY_MANA,		150,		200,
	APPLY_MOVE,		150,		200	},

    	{ "Odin",  		101,		200,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		150,		200,
	APPLY_MANA,		150,		200,
	APPLY_AC,		-50,		-10	},

     /* START OF IMMORTAL ITEMS                         */

    	{ "Doom",		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DAMROLL,		20,		30,
	APPLY_MOVE,		150,		200,
	APPLY_AGE,		15,		30 },

    	{ "Tas",  		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-10,		-5,
	APPLY_HIT,		150,		200,
	APPLY_DAMROLL,		20,		32	},

	{ "Digaaz",  		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_AC,		-20,		-5,
	APPLY_HIT,		150,		200,
	APPLY_HITROLL,		10,		20	},
   
	{ "Fesdor",		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		50,		150,
	APPLY_AC,		-35,		-30,
	APPLY_HITROLL,		20,		30	},
	
	{ "Gavorkian",		101,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_SAVES,		-8,		-5,
	APPLY_AC,		-30,		-15,
	APPLY_HITROLL,		15,		30	},
	
     /* START OF VULN AFFECT ITEMS                      */

	{ "Burning",		50,		0,
	NO_FLAG,		NO_FLAG,	VULN_FIRE,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Shock",		50,		0,
	NO_FLAG,		NO_FLAG,	VULN_LIGHTNING,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Poison",		10,		0,
	NO_FLAG,		NO_FLAG,	VULN_POISON,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Freeze",		40,		0,
	NO_FLAG,		NO_FLAG,	VULN_COLD,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Corrosion",		50,		0,
	NO_FLAG,		NO_FLAG,	VULN_ACID,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Disorder",		10,		0,
	NO_FLAG,		NO_FLAG,	VULN_NEGATIVE,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Disturbed",	20,		-300,
	NO_FLAG,		NO_FLAG,	VULN_HOLY,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Troubled",	15,		0,
	NO_FLAG,		NO_FLAG,	VULN_SOUND,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Shrieking",		10,		0,
	NO_FLAG,		NO_FLAG,	VULN_MENTAL,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Mole",		10,		-150,
	NO_FLAG,		NO_FLAG,	VULN_LIGHT,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Infestation",	15,		0,
	NO_FLAG,		NO_FLAG,	VULN_DISEASE,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Domination",	15,		0,
	NO_FLAG,		NO_FLAG,	VULN_CHARM,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

     /* START OF THE AFFECT ITEMS                       */

    	{ "the Crusader",	10,		300,
	AFF_DETECT_EVIL,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Detection",		40,		0,
	AFF_DETECT_INVIS,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Channeling",		5,		0,
	AFF_DETECT_MAGIC,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Perception",		40,		0,
	AFF_DETECT_MOTION,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Minion",		10,		-300,
	AFF_DETECT_GOOD,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Redeye",		40,		0,
	AFF_INFRARED,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Vision",		30,		0,
	AFF_FARSIGHT,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Chameleon",	60,		0,
	AFF_SNEAK,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Falcon",		20,		0,
	AFF_FLYING,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "the Ghost",		50,		0,
	AFF_PASS_DOOR,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

/*    { "the Cheetah",		70,		0,
	NO_FLAG,		RES_SLOW,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},
*/
    	{ "the Owl",		40,		0,
	AFF_DARK_VISION,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},
	
	{ "Ksavir",  		1,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_INT,		1,		2,
	APPLY_CON,		1,		2,
	APPLY_WIS,		1,		2	},

    	{ "Bree",  		30,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_HIT,		60,		120,
	APPLY_MANA,		60,		120,
	APPLY_DAMROLL,		17,		30	},

    	{ "the Gods",		40,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_DEX,		1,		5,
	APPLY_SAVES,		-15,		-1,
	APPLY_HITROLL,		5,		15	},
	
    	{ "Rage",		30,		0,
	AFF_BERSERK,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

    	{ "Valkyrie",		45,		0,
	AFF_REGENERATION,	NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	},

	{ "Skyntil",	        99,             0,
        NO_FLAG,   		NO_FLAG,        NO_FLAG,
        APPLY_REGEN,    	5,              30,
        APPLY_MORPH_FORM,	MORPH_RED,      MORPH_GOLD,
        APPLY_NONE,         	0,		0  },
        
    	{ NULL,			0,		0,
	NO_FLAG,		NO_FLAG,	NO_FLAG,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0,
	APPLY_NONE,		0,		0	}
 };

OBJ_DATA *rand_obj(CHAR_DATA *ch, int mob_level )
{
  OBJ_DATA *obj;
  int rand_number = 0;
  int rand_number2 = 0;
  int prefix_number = 0;
  int suffix_number = 0;
  int pslevel = 0;
  int align = 0;
  bool roll = FALSE;
  bool add_prefix = FALSE;
  bool add_suffix = FALSE;
  char buf_name[MSL];
  char buf_short[MSL];
  int rand_type = 0; // cracked = 1, normal = 0, excep =2
  int tolerance = 2;

  // 1-100 random
  rand_number = number_percent();

  // Unique Objects - 2%
  if ( rand_number <= 2 && mob_level + 1 >= (number_percent()/2) + 91 )
  {
	//  Pick only objects that are at or below the level of the killed mobile

	rand_number = number_range(LOWER_UNIQUE_VNUM, UPPER_UNIQUE_VNUM);
	obj = create_object( get_obj_index ( rand_number ), 0 );

//	while( obj->level > ( mob_level - 20 ) ) //asdf
	while( obj->level > ( mob_level <= 20 ? 1 : mob_level - 20 ) )
	{
		rand_number = number_range(LOWER_UNIQUE_VNUM, UPPER_UNIQUE_VNUM);
		obj = create_object( get_obj_index ( rand_number ), 0 );
	}	
	
	// Add the word 'unique' to the item name for easy location
	buf_name[0]='\0';
	strcat(buf_name,str_dup(obj->name));
	strcat(buf_name," unique");
  }
  // Magical Objects - 22%
  else if ( rand_number <= 24 )
  {
	pslevel = 0;

	//  Pick only objects that are at or below the level of the killed mobile

	rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
	obj = create_object( get_obj_index ( rand_number ), 0 );

//	while( obj->level > ( mob_level - 20 ) ) //asdf
	while( obj->level > ( mob_level <= 20 ? 1 : mob_level - 20 ) )
	{
		rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
		obj = create_object( get_obj_index ( rand_number ), 0 );
	}

        // Check and see if Quality Changes
        // Exceptional
        if( (rand_number2 = number_percent()) < 15 )
        {
         rand_type = 2;

         obj = make_exceptional(obj);

         tolerance = 7;
        }
        else if (rand_number2 > 85)
        {
         rand_type = 1;

         obj = make_cracked(obj);

         tolerance = -2;
        }

	//  Prefix, Suffix or Both?
	rand_number = dice ( 1, 3 );
	if ( rand_number == 1 )
	{
		add_prefix = TRUE;
	}
	else if ( rand_number == 2 )
	{
		add_suffix = TRUE;		
	}
	else
	{
		add_prefix = TRUE;
		add_suffix = TRUE;
	}

	buf_short[0] = '\0';

	if(mob_level+tolerance < 1)
		tolerance = 1;
	
	// Pick out Prefix/Suffix and be sure that the levels of the two
	// combined is less than or equal to the mob's level
        while ((( pslevel ) > mob_level + tolerance)
		|| roll == FALSE )
	{
		roll = TRUE;
		pslevel = 0;

		if ( add_prefix )
		{
			// Pick Prefix
			prefix_number = number_range ( 0, MAX_PREFIX-1 );
			pslevel = prefix_table[prefix_number].level;
		}
		
		if ( add_suffix )
		{
			// Pick Suffix
			suffix_number = number_range ( 0, MAX_SUFFIX-1 );
			pslevel += suffix_table[suffix_number].level;
		}
	}

	buf_short[0]='\0';

        if( rand_type == 2 ) 
         strcat(buf_short,"Exceptional ");
        else if(rand_type == 1)
         strcat(buf_short,"Cracked ");

	if( add_prefix )
	{
		// Add the Prefix
		strcat(buf_short, prefix_table[prefix_number].name);
		strcat(buf_short, " ");

		// Add the affects of the prefix
		process_mods( obj, prefix_number, TRUE );

		// Compute align of object with prefix modifier
		align += prefix_table[prefix_number].align;
	}

	// Add obj->short_descr to the total buf_short string
	strcat(buf_short, obj->short_descr);	

	if ( add_suffix )
	{
		// Add the Suffix
		strcat(buf_short, " of ");
		strcat(buf_short, suffix_table[suffix_number].name);		

		// Add the affects of the suffix
		process_mods( obj, suffix_number, FALSE );

		// Compute align of object with suffix modifier
		align += suffix_table[suffix_number].align;
	}

	// Add alignment restrictions based on object alignment
	if (align >= 600)
	{
		SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
		SET_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);
	}
	else if (align >=300 && align < 600)
		SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
	else if (align > -600 && align <= -300 )
		SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
	else if (align <= -600)
	{
		SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
		SET_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);
	}

	// If suffix or prefix only, increase suff or pref level to make pslevel
	if ((add_prefix && !add_suffix) || (add_suffix && !add_prefix))
	{
	  pslevel *= 5;
	  pslevel /= 3;
	}

	// Set object level to combined pre/suffix level or 101.. whichever is lower
        obj->level = UMAX ( pslevel, obj->level );
        obj->level = UMIN ( obj->level, 101 );

	// Assign buf_short as the item's short description
	obj->short_descr = str_dup ( buf_short );

	// Add the word 'special' to the item name for easy location
	buf_name[0]='\0';
	strcat(buf_name,str_dup(obj->name));
	if(add_suffix)
	{
		strcat(buf_name, " ");
		strcat(buf_name, suffix_table[suffix_number].name);
	}
	if(add_prefix)
	{
		strcat(buf_name, " ");
		strcat(buf_name, prefix_table[prefix_number].name);
	}

        // Add Quality identifiers
        if( rand_type == 2 ) 
         strcat(buf_name," exceptional");
        else if(rand_type == 1)
         strcat(buf_name," cracked");

	strcat(buf_name," special");
  }
  // Regular Objects - 75%
  else
  {
	//  Pick only objects that are at or below the level of the killed mobile

	rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
	obj = create_object( get_obj_index ( rand_number ), 0 );

//	while( obj->level > ( mob_level - 20 ) ) //asdf
	while( obj->level > ( mob_level <= 20 ? 1 : mob_level - 20 ) )
	{
		rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
		obj = create_object( get_obj_index ( rand_number ), 0 );
	}

        // Exceptional
        if( (rand_number2 = number_percent()) < 15 )
        {
         rand_type = 2;
         obj = make_exceptional(obj);

         buf_short[0]='\0';
         strcat(buf_short,"exceptional ");
         strcat(buf_short,obj->short_descr);
         obj->short_descr = str_dup(buf_short);

         // Add the word 'exceptional' to the item name for easy location
         buf_name[0]='\0';
         strcat(buf_name,str_dup(obj->name));
         strcat(buf_name," exceptional");
        }
        else if (rand_number2 > 85)
        {
         rand_type = 1;
         obj = make_cracked(obj);

         buf_short[0]='\0';
         strcat(buf_short,"cracked ");
         strcat(buf_short,obj->short_descr);
         obj->short_descr = str_dup(buf_short);

         // Add the word 'cracked' to the item name for easy location
         buf_name[0]='\0';
         strcat(buf_name,str_dup(obj->name));
         strcat(buf_name," cracked");
        }
        else
        {
         // Add the word 'normal' to the item name for easy location
         buf_name[0]='\0';
         strcat(buf_name,str_dup(obj->name));
         strcat(buf_name," normal");
        }
  }

  // Add specially flagged name 'normal', 'special', or 'unique'
  obj->name = str_dup( buf_name );

  // Object level is somewhere between the mob and object levels
  // Price depending on the quality
  if(rand_type == 1)
   obj->cost = ( obj->level * 500 );
  else if(rand_type == 2)
   obj->cost = ( obj->level * 1500 );
  else
   obj->cost = ( obj->level * 1000 );

/*  obj->level = number_range( mob_level, obj->level ); */
  if (obj->level > 101)
	obj->level = 101;

  // Return object
  return obj;
}

void process_mods ( OBJ_DATA *rand_obj, int mod_number, bool prefix )
{
  AFFECT_DATA *Af;
  int value = 0;

  if (prefix)
  {
	if ( prefix_table[mod_number].affect != NO_FLAG)
	{
		// Add Affect
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_AFFECTS;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	prefix_table[mod_number].affect;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;
	}

	if ( prefix_table[mod_number].resist != NO_FLAG)
	{
		// Add Resistance
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_RESIST;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	prefix_table[mod_number].resist;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;
	}

	if ( prefix_table[mod_number].vulner != NO_FLAG)
	{
		// Add Vulnerability
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_VULN;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	prefix_table[mod_number].vulner;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;

	}

	if ( prefix_table[mod_number].mod_one != APPLY_NONE)
	{
		// Add First Apply
		value = number_range(prefix_table[mod_number].min_one,
					prefix_table[mod_number].max_one);
	
		Af 			=	new_affect();
		Af->location	=	prefix_table[mod_number].mod_one;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;

	}

	if ( prefix_table[mod_number].mod_two != APPLY_NONE)
	{
		// Add Second Apply
		value = number_range(prefix_table[mod_number].min_two,
					prefix_table[mod_number].max_two);
	
		Af 			=	new_affect();
		Af->location	=	prefix_table[mod_number].mod_two;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}

	if ( prefix_table[mod_number].mod_three != APPLY_NONE)
	{
		// Add Third Apply
		value = number_range(prefix_table[mod_number].min_three,
					prefix_table[mod_number].max_three);
	
		Af 			=	new_affect();
		Af->location	=	prefix_table[mod_number].mod_three;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}
  }
  else
  {
	if ( suffix_table[mod_number].affect != NO_FLAG)
	{
		// Add Affect
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_AFFECTS;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	suffix_table[mod_number].affect;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;
	}

	if ( suffix_table[mod_number].resist != NO_FLAG)
	{
		// Add Resistance
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_RESIST;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	suffix_table[mod_number].resist;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;
	}

	if ( suffix_table[mod_number].vulner != NO_FLAG)
	{
		// Add Vulnerability
		Af             	=	new_affect();
    		Af->location   	=	APPLY_NONE;
   		Af->modifier   	=	0;
    		Af->where      	=	TO_VULN;
    		Af->type       	=	-1;
    		Af->duration   	=	-1;
    		Af->bitvector  	=	suffix_table[mod_number].vulner;
    		Af->level      	=	rand_obj->level;
    		Af->next       	=	rand_obj->affected;
    		rand_obj->affected  =	Af;

	}
    if ( suffix_table[mod_number].mod_one == APPLY_MORPH_FORM || suffix_table[mod_number].mod_two == APPLY_MORPH_FORM || suffix_table[mod_number].mod_three == APPLY_MORPH_FORM)
     {
	     if ( suffix_table[mod_number].mod_one != APPLY_NONE)
	   {
		// Add First Apply
		value = number_range(suffix_table[mod_number].min_one,
					suffix_table[mod_number].max_one);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_one;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	gsn_conceal;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;

	}

	if ( suffix_table[mod_number].mod_two != APPLY_NONE)
	{
		// Add Second Apply
		value = number_range(suffix_table[mod_number].min_two,
					suffix_table[mod_number].max_two);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_two;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	gsn_conceal;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}

	if ( suffix_table[mod_number].mod_three != APPLY_NONE)
	{
		// Add Third Apply
		value = number_range(suffix_table[mod_number].min_three,
					suffix_table[mod_number].max_three);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_three;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	gsn_conceal;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}
}     
	   else
	   {  
	   if ( suffix_table[mod_number].mod_one != APPLY_NONE)
	   {
		// Add First Apply
		value = number_range(suffix_table[mod_number].min_one,
					suffix_table[mod_number].max_one);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_one;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;

	}

	if ( suffix_table[mod_number].mod_two != APPLY_NONE)
	{
		// Add Second Apply
		value = number_range(suffix_table[mod_number].min_two,
					suffix_table[mod_number].max_two);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_two;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}

	if ( suffix_table[mod_number].mod_three != APPLY_NONE)
	{
		// Add Third Apply
		value = number_range(suffix_table[mod_number].min_three,
					suffix_table[mod_number].max_three);
	
		Af 			=	new_affect();
		Af->location	=	suffix_table[mod_number].mod_three;
		Af->modifier	=	value;
		Af->where		=	TO_OBJECT;
		Af->type		=	-1;
		Af->duration   	=	-1;
    		Af->bitvector  	=	0;
    		Af->level      	=	rand_obj->level;
		Af->next		=	rand_obj->affected;
		rand_obj->affected  =   	Af;
	}
  }

}
}
// For Immortal Loading...SPECIFICS :) - Skyntil
OBJ_DATA *rand_obj2(CHAR_DATA *ch, int mob_level, char *argument )
{
  OBJ_DATA *obj;
  char buf_name[MSL/2];
  char buf_short[MSL/2];
  char arg1[MSL/4];
  char arg2[MSL/4];
  char arg3[MSL/4];
  int rand_number = 0;
  char *prefix = NULL;
  char *suffix = NULL;

  argument = one_argument(argument,arg1); // Type
  argument = one_argument(argument,arg2); // Prefix
  argument = one_argument(argument,arg3); // Suffix

  if(arg1[0] == '\0') // No Type submitted..
  {
   send_to_char("Syntax: randobj2 <type> <prefix> <suffix>\n\r",ch);
   send_to_char("Type 'randobj2 list' for a list of types.\n\r",ch);
   send_to_char("Prefix and Suffix only accepted IF type is 4,5, or 6.\n\r",ch);
   send_to_char("If type is 4,5, or 6, if Pre/Suf is left blank, they will be randomly chosen.\n\r",ch);
   send_to_char("If you want Suf only, type 'none' in place of the prefix.\n\r",ch);
   return NULL;
  }
  else if(arg1[0] != '\0') // Picked a Type...
  {
   if(!str_cmp(arg1,"list"))
   {
    show_randobj2_types_cmds(ch);
    return NULL;
   }
   else if(!str_cmp(arg1,"unique"))
   {
    rand_number = number_range(LOWER_UNIQUE_VNUM, UPPER_UNIQUE_VNUM);
    obj = create_object( get_obj_index ( rand_number ), 0 );

    while( obj->level > ( mob_level + 1 ) )
    {
        rand_number = number_range(LOWER_UNIQUE_VNUM, UPPER_UNIQUE_VNUM);
        obj = create_object( get_obj_index ( rand_number ), 0 );
    } 
	
    // Add the word 'unique' to the item name for easy location
    buf_name[0]='\0';
    strcat(buf_name,str_dup(obj->name));
    strcat(buf_name," unique");

    obj->name = str_dup( buf_name );

    obj->cost = ( obj->level * 1000 );

    if (obj->level > 101)
     obj->level = 101;
      
    return obj;
   }
   else
   {
    rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
    obj = create_object( get_obj_index ( rand_number ), 0 );

    while( obj->level > ( mob_level + 1 ) )
    {
        rand_number = number_range(LOWER_RANDOM_VNUM, UPPER_RANDOM_VNUM);
        obj = create_object( get_obj_index ( rand_number ), 0 );
    }

    if(IS_SET(obj->extra_flags,ITEM_NOPURGE))
     REMOVE_BIT(obj->extra_flags,ITEM_NOPURGE);
    if(IS_SET(obj->extra_flags,ITEM_NOSAC))
     REMOVE_BIT(obj->extra_flags,ITEM_NOSAC);

    if(!str_cmp(arg1,"normal"))
    {
     // Add the word 'normal' to the item name for easy location
     buf_name[0]='\0';
     strcat(buf_name,str_dup(obj->name));
     strcat(buf_name," normal");

     obj->name = str_dup( buf_name );

     obj->cost = ( obj->level * 1000 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    }
    else if(!str_cmp(arg1,"cracked"))
    {
     obj = make_cracked(obj);

     buf_short[0]='\0';
     strcat(buf_short,"cracked ");
     strcat(buf_short,obj->short_descr);
     obj->short_descr = str_dup(buf_short);

     // Add the word 'cracked' to the item name for easy location
     buf_name[0]='\0';
     strcat(buf_name,str_dup(obj->name));
     strcat(buf_name," cracked");

     obj->name = str_dup( buf_name );

     obj->cost = ( obj->level * 500 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    }
    else if(!str_cmp(arg1,"exceptional"))
    {
     obj = make_exceptional(obj);

     buf_short[0]='\0';
     strcat(buf_short,"exceptional ");
     strcat(buf_short,obj->short_descr);
     obj->short_descr = str_dup(buf_short);

     // Add the word 'exceptional' to the item name for easy location
     buf_name[0]='\0';
     strcat(buf_name,str_dup(obj->name));
     strcat(buf_name," exceptional");

     obj->name = str_dup( buf_name );

     obj->cost = ( obj->level * 1500 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    }
    else if(!str_cmp(arg1,"special"))
    {
     if(arg2[0] == '\0')
     {
      prefix = NULL;
      suffix = NULL;
     }
     else if(arg2[0] != '\0')
     {
      if(!str_cmp(arg2,"none"))
      {
       prefix = NULL;
      }
      else if(!str_cmp(arg2,"list"))
      {
       show_prefixes(ch);
       return NULL;
      }
      else
      {
       if(is_prefix(arg2))
        prefix = arg2;
       else
       {
        send_to_char("That is not a valid Prefix. Try 'List' for choices.\n\r",ch);
        return NULL;
       }
      }

      if(arg3[0] == '\0')
       suffix = NULL;
      else if(arg3[0] != '\0')
      {
        if(!str_cmp(arg3,"list"))
        {
         show_suffixes(ch);
         return NULL;
        }
        if(is_suffix(arg3))
         suffix = arg3;
        else
        {
         send_to_char("That is not a valid Suffix. Try 'List' for choices.\n\r",ch);
         return NULL;
        }
      }
     }
     
     obj = make_special(obj, mob_level, 0, prefix, suffix);

     obj->cost = ( obj->level * 1000 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    }
    else if(!str_cmp(arg1,"cspecial"))
    {
     if(arg2[0] == '\0')
     {
      prefix = NULL;
      suffix = NULL;
     }
     else if(arg2[0] != '\0')
     {
      if(!str_cmp(arg2,"none"))
      {
       prefix = NULL;
      }
      else if(!str_cmp(arg2,"list"))
      {
       show_prefixes(ch);
       return NULL;
      }
      else
      {
       if(is_prefix(arg2))
        prefix = arg2;
       else
       {
        send_to_char("That is not a valid Prefix. Try 'List' for choices.\n\r",ch);
        return NULL;
       }
      }

      if(arg3[0] == '\0')
       suffix = NULL;
      else if(arg3[0] != '\0')
      {
        if(!str_cmp(arg3,"list"))
        {
         show_suffixes(ch);
         return NULL;
        }
        if(is_suffix(arg3))
         suffix = arg3;
        else
        {
         send_to_char("That is not a valid Suffix. Try 'List' for choices.\n\r",ch);
         return NULL;
        }
      }
     }

     obj = make_cracked(obj);
     obj = make_special(obj, mob_level, 1, prefix, suffix);

     obj->cost = ( obj->level * 500 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    }
    else if(!str_cmp(arg1,"especial"))
    {
     if(arg2[0] == '\0')
     {
      prefix = NULL;
      suffix = NULL;
     }
     else if(arg2[0] != '\0')
     {
      if(!str_cmp(arg2,"none"))
      {
       prefix = NULL;
      }
      else if(!str_cmp(arg2,"list"))
      {
       show_prefixes(ch);
       return NULL;
      }
      else
      {
       if(is_prefix(arg2))
        prefix = arg2;
       else
       {
        send_to_char("That is not a valid Prefix. Try 'List' for choices.\n\r",ch);
        return NULL;
       }
      }

      if(arg3[0] == '\0')
       suffix = NULL;
      else if(arg3[0] != '\0')
      {
        if(!str_cmp(arg3,"list"))
        {
         show_suffixes(ch);
         return NULL;
        }
        if(is_suffix(arg3))
         suffix = arg3;
        else
        {
         send_to_char("That is not a valid Suffix. Try 'List' for choices.\n\r",ch);
         return NULL;
        }
      }
     }
     obj = make_exceptional(obj);
     obj = make_special(obj, mob_level, 2, prefix, suffix);

     obj->cost = ( obj->level * 1500 );

     if (obj->level > 101)
      obj->level = 101;

     return obj;
    } 
    else
    {
     send_to_char("That is not a valid type.\n\r",ch);
     return NULL;
    }
   }
  }
  return NULL;
}

void show_randobj2_types_cmds( CHAR_DATA *ch )  
{  
    char buf  [ MAX_STRING_LENGTH ];  
    char buf1 [ MAX_STRING_LENGTH ];  
    int  cmd;  
    int  col;  
   
    buf1[0] = '\0';  
    col = 0;  
    for (cmd = 0; arg_table[cmd].type != NULL; cmd++)  
    {  
        sprintf( buf, "%d. %-15.15s - %s.\n\r", arg_table[cmd].index, arg_table[cmd].type, arg_table[cmd].desc );  
        strcat( buf1, buf );  
        if ( ++col % 5 == 0 )  
            strcat( buf1, "\n\r" );  
    }  
   
    if ( col % 5 != 0 )  
        strcat( buf1, "\n\r" );  
  
    send_to_char( buf1, ch );  
    return;  
}  

OBJ_DATA *make_cracked(OBJ_DATA *obj)
{
 if(obj->item_type == ITEM_WEAPON)
 {
  obj->value[1] -= number_range(0,1);
  obj->value[2] -= number_range(0,1);
 }
 else if(obj->item_type == ITEM_ARMOR)
 {
  obj->value[1] -= number_range(0,8);
  obj->value[2] -= number_range(0,8);
  obj->value[3] -= number_range(0,8);
  obj->value[4] -= number_range(0,8);
 }
 return obj;
}

OBJ_DATA *make_exceptional(OBJ_DATA *obj)
{
 int rand_weapon_flag = 0;
 int rand_number = 0;
 AFFECT_DATA *Af;
 int value = 0;

 if(obj->item_type == ITEM_WEAPON)
 {
/*  Removed by Tien - No addition of dice from Tas.
   obj->value[1] += number_range(0,2);
   obj->value[2] += number_range(0,2);
*/
   switch( dice(1, 10) )
   {
    case 1: rand_weapon_flag = WEAPON_ACIDIC;  break;
    case 2: rand_weapon_flag = WEAPON_SHOCKING;  break;
    case 3: rand_weapon_flag = WEAPON_VORPAL;  break;
    case 4: rand_weapon_flag = WEAPON_FROST;  break;
    case 5: rand_weapon_flag = WEAPON_POISON;  break;
    case 6: rand_weapon_flag = WEAPON_VAMPIRIC;  break;
    case 7: rand_weapon_flag = WEAPON_FLAMING;  break;
/*
    case 8: obj->value[1] += number_range(0,1); break;
    case 9: obj->value[2] += number_range(0,1); break;
*/
    default: break;
   }
   // Add the random flag
   if (rand_weapon_flag != 0)
    SET_BIT(obj->value[4], rand_weapon_flag);
  }
  else if(obj->item_type == ITEM_ARMOR)
  {
    obj->value[1] += number_range(0,10);
    obj->value[2] += number_range(0,10);
    obj->value[3] += number_range(0,10);
    obj->value[4] += number_range(0,10);

    switch( dice(1, 8) )
    {
     case 1: rand_weapon_flag = ITEM_MAGIC; break;
     case 2: rand_weapon_flag = ITEM_HUM; break;
     case 3: rand_weapon_flag = ITEM_BLESS; break;
     case 4: rand_weapon_flag = ITEM_GLOW; break;
     case 5: rand_weapon_flag = ITEM_EVIL; break;
     default: break;
    }

    if (rand_weapon_flag != 0)
     SET_BIT(obj->value[4], rand_weapon_flag);
  }

  //Now for some random Applies....
  if(number_percent() > 50)
  {
   rand_number = number_range(0, MAX_EXCEP_APPLY);

   if ( eapply_table[rand_number].apply_type != 0)
   {
      // Add First Apply
      value = number_range(eapply_table[rand_number].min, eapply_table[rand_number].max);
	
      Af              =       new_affect();
      Af->location    =       eapply_table[rand_number].apply_type;
      Af->modifier    =       value;
      Af->where       =       TO_OBJECT;
      Af->type        =       -1;
      Af->duration    =       -1;
      Af->bitvector   =       0;
      Af->level       =       obj->level;
      Af->next        =       obj->affected;
      obj->affected   =       Af;
   }
  }

  if(number_percent() < 25)
  {
   rand_number = number_range(0, MAX_EXCEP_APPLY);

   if ( eapply_table[rand_number].apply_type != 0)
   {
      // Add First Apply
      value = number_range(eapply_table[rand_number].min, eapply_table[rand_number].max);
	
      Af              =       new_affect();
      Af->location    =       eapply_table[rand_number].apply_type;
      Af->modifier    =       value;
      Af->where       =       TO_OBJECT;
      Af->type        =       -1;
      Af->duration    =       -1;
      Af->bitvector   =       0;
      Af->level       =       obj->level;
      Af->next        =       obj->affected;
      obj->affected   =       Af;
   }
  }


  return obj;
}

OBJ_DATA *make_special(OBJ_DATA *obj, int mob_level, int rand_type, char *prefix, char *suffix)
{
  int rand_number = 0;
  bool add_prefix = FALSE;
  bool add_suffix = FALSE;
  char buf_short[MSL/2];
  int tolerance = 2;
  int pslevel = 0;
  bool roll = FALSE;
  int prefix_number = 0;
  int suffix_number = 0;
  char buf_name[MSL/2];
  int align = 0;

  //Sent Prefix
  if(prefix != NULL && suffix == NULL)
  {
   prefix_number = prefix_lookup(prefix);

   if(prefix_number == -1)
    prefix_number = number_range ( 0, MAX_PREFIX-1 );

   add_prefix = TRUE;
   add_suffix = FALSE;
   pslevel = prefix_table[prefix_number].level;
  }
  else if(prefix == NULL && suffix != NULL)
  {
   suffix_number = suffix_lookup(suffix);

   if(suffix_number == -1)
    suffix_number = number_range ( 0, MAX_SUFFIX-1 );

   add_suffix = TRUE;
   add_prefix = FALSE;
   pslevel = suffix_table[suffix_number].level;
  }
  else if(prefix != NULL && suffix != NULL)
  {
   prefix_number = prefix_lookup(prefix);

   if(prefix_number == -1)
    prefix_number = number_range ( 0, MAX_PREFIX-1 );

   suffix_number = suffix_lookup(suffix);

   if(suffix_number == -1)
    suffix_number = number_range ( 0, MAX_SUFFIX-1 );

   add_prefix = TRUE;
   add_suffix = TRUE;
   pslevel = prefix_table[prefix_number].level;
   pslevel += suffix_table[suffix_number].level;
  }
  else
  {
   //  Prefix, Suffix or Both?
   rand_number = dice ( 1, 3 );
   if ( rand_number == 1 )
   {
    add_prefix = TRUE;
   }
   else if ( rand_number == 2 )
   {
    add_suffix = TRUE;       
   }
   else
   {
    add_prefix = TRUE;
    add_suffix = TRUE;
   }

   buf_short[0] = '\0';

   // Pick out Prefix/Suffix and be sure that the levels of the two
   // combined is less than or equal to the mob's level
   while ((( pslevel ) > mob_level + tolerance)
            || roll == FALSE )
   {
    roll = TRUE;
    pslevel = 0;

    if ( add_prefix )
    {
      // Pick Prefix
      prefix_number = number_range ( 0, MAX_PREFIX-1 );
      pslevel = prefix_table[prefix_number].level;
    }
       
    if ( add_suffix )
    {
      // Pick Suffix
      suffix_number = number_range ( 0, MAX_SUFFIX-1 );
      pslevel += suffix_table[suffix_number].level;
    }

    // Make suffix or prefix-only items higher level ( level * 3/2 )
    if (!add_suffix || !add_prefix)
    {
      pslevel *=3;
      pslevel /=2;
    }
   }
  }

   buf_short[0]='\0';

   if( rand_type == 2 )
    strcat(buf_short,"Exceptional ");
   else if(rand_type == 1)
    strcat(buf_short,"Cracked ");

  if( add_prefix )
  {
    // Add the Prefix
    strcat(buf_short, prefix_table[prefix_number].name);
    strcat(buf_short, " ");

    // Add the affects of the prefix
    process_mods( obj, prefix_number, TRUE );

    // Compute align of object with prefix modifier
    align += prefix_table[prefix_number].align;
  }

  // Add obj->short_descr to the total buf_short string
  strcat(buf_short, obj->short_descr); 

  if ( add_suffix )
  {
    // Add the Suffix
    strcat(buf_short, " of ");
    strcat(buf_short, suffix_table[suffix_number].name);     

    // Add the affects of the suffix
    process_mods( obj, suffix_number, FALSE );

    // Compute align of object with suffix modifier
    align += suffix_table[suffix_number].align;
  }

  // Add alignment restrictions based on object alignment
  if (align >= 600)
  {
    SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
    SET_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);
  }
  else if (align >=300 && align < 600)
    SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
  else if (align > -600 && align <= -300 )
    SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
  else if (align <= -600)
  {
    SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
    SET_BIT(obj->extra_flags,ITEM_ANTI_NEUTRAL);
  }

  // Set object level to combined pre/suffix level or 101.. whichever is lower
  obj->level = UMIN ( pslevel, 101 );

  // Assign buf_short as the item's short description
  obj->short_descr = str_dup ( buf_short );

  // Add the word 'special' to the item name for easy location
  buf_name[0]='\0';
  strcat(buf_name,str_dup(obj->name));

  if(add_suffix)
  {
      strcat(buf_name, " ");
      strcat(buf_name, suffix_table[suffix_number].name);
  }
  if(add_prefix)
  {
      strcat(buf_name, " ");
      strcat(buf_name, prefix_table[prefix_number].name);
  }

  // Add Quality identifiers
  if( rand_type == 2 ) 
   strcat(buf_name," exceptional");
  else if(rand_type == 1)
   strcat(buf_name," cracked");

  strcat(buf_name," special");

  obj->name = str_dup( buf_name );

  return obj;
}

bool is_prefix(char *word)
{
 int pre;
 bool ispre = FALSE;

 for(pre = 0; prefix_table[pre].name != NULL; pre++)
 {
  if(!str_cmp(prefix_table[pre].name,word))
   ispre = TRUE;
 }

 return ispre;
}

bool is_suffix(char *word)
{
 int suf;
 bool issuf = FALSE;

 for(suf = 0; suffix_table[suf].name != NULL; suf++)
 {
  if(!str_cmp(suffix_table[suf].name,word))
   issuf = TRUE;
 }

 return issuf;
}

void show_prefixes( CHAR_DATA *ch )  
{  
    char buf  [ MAX_STRING_LENGTH ];  
    char buf1 [ MAX_STRING_LENGTH ];  
    int  cmd;  
    int  col;  
   
    buf1[0] = '\0';  
    col = 0;  
    for (cmd = 0; prefix_table[cmd].name != NULL; cmd++)  
    {  
        sprintf( buf, "%-15.15s ", prefix_table[cmd].name);  
        strcat( buf1, buf );  
        if ( ++col % 5 == 0 )  
            strcat( buf1, "\n\r" );  
    }  
   
    if ( col % 5 != 0 )  
        strcat( buf1, "\n\r" );  
  
    send_to_char( buf1, ch );  
    return;  
}  

void show_suffixes( CHAR_DATA *ch )  
{  
    char buf  [ MAX_STRING_LENGTH ];  
    char buf1 [ MAX_STRING_LENGTH ];  
    int  cmd;  
    int  col;  
   
    buf1[0] = '\0';  
    col = 0;  
    for (cmd = 0; suffix_table[cmd].name != NULL; cmd++)  
    {  
        sprintf( buf, "%-15.15s ", suffix_table[cmd].name);  
        strcat( buf1, buf );  
        if ( ++col % 5 == 0 )  
            strcat( buf1, "\n\r" );  
    }  
   
    if ( col % 5 != 0 )  
        strcat( buf1, "\n\r" );  
  
    send_to_char( buf1, ch );  
    return;  
}  

int prefix_lookup(char *prefix)
{
  int number;

  for (number = 0; prefix_table[number].name != NULL; number++)
  {  
   if(!str_cmp(prefix_table[number].name,prefix))
    return number;
  }

  return number;
}

int suffix_lookup(char *suffix)
{
  int number;

  for (number = 0; suffix_table[number].name != NULL; number++)
  {  
   if(!str_cmp(suffix_table[number].name,suffix))
    return number;
  }

  return -1;
}
