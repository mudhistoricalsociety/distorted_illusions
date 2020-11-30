/*************************************************************************** 
 *  File: olc.h                                                            *  
 *                                                                         *  
 *  Much time and thought has gone into this software and you are          *  
 *  benefitting.  We hope that you share your changes too.  What goes      *  
 *  around, comes around.                                                  *  
 *                                                                         *  
 *  This code was freely distributed with the The Isles 1.1 source code,   *  
 *  and has been used here for OLC - OLC would not be what it is without   *  
 *  all the previous coders who released their source code.                *  
 *                                                                         *  
 ***************************************************************************/  
/*  
 * This is a header file for all the OLC files.  Feel free to copy it into  
 * merc.h if you wish.  Many of these routines may be handy elsewhere in  
 * the code.  -Jason Dinkel  
 */  
  
  
/*  
 * The version info.  Please use this info when reporting bugs.  
 * It is displayed in the game by typing 'version' while editing.  
 * Do not remove these from the code - by request of Jason Dinkel  
 */  
#define VERSION "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r     Port a ROM 2.4 v1.7\n\r"  
#define AUTHOR  "     By Jason(jdinkel@mines.colorado.edu)\n\r     Modified for use with ROM 2.3\n\r     By Hans Birkeland (hansbi@ifi.uio.no)\n\r     Modificado para uso en ROM 2.4b4a\n\r     Por Ivan Toledo (pvillanu@choapa.cic.userena.cl)\n\r"  
#define DATE    "     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r     (Port a ROM 2.4 - Nov 2, 1996)\n\r    Version actual : 1.7 - Mar 7, 1998\n\r"  
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"  
/*
 *   The definitions for max/min oedit balances to exit editor
 */
long obj_points( OBJ_INDEX_DATA * obj );
long obj_balance( OBJ_INDEX_DATA * obj );  
bool oedit_check( CHAR_DATA * ch );

#define MAX_OEDIT_BALANCE "200"
#define MIN_OEDIT_BALANCE "-200"
  
/*  
 * New typedefs.  
 */  
typedef bool OLC_FUN            args( ( CHAR_DATA *ch, char *argument ) );  
#define DECLARE_OLC_FUN( fun )  OLC_FUN    fun  
  
  
/* Command procedures needed ROM OLC */  
DECLARE_DO_FUN(    do_help    );  
DECLARE_SPELL_FUN( spell_null );  
  
  
  
/*  
 * Connected states for editor.  
 */  
#define ED_NONE         0  
#define ED_AREA         1  
#define ED_ROOM         2  
#define ED_OBJECT       3  
#define ED_MOBILE       4  
#define ED_MPCODE       5  
#define ED_HELP         6  
#define ED_CLAN         7  
  
  
/*  
 * Interpreter Prototypes  
 */  
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );  
void    redit           args( ( CHAR_DATA *ch, char *argument ) );  
void    medit           args( ( CHAR_DATA *ch, char *argument ) );  
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );  
void    mpedit          args( ( CHAR_DATA *ch, char *argument ) );  
void    hedit           args( ( CHAR_DATA *ch, char *argument ) );  
void    cedit           args( ( CHAR_DATA *ch, char *argument ) );  

/*  
 * OLC Constants  
 */  
#define MAX_MOB 1               /* Default maximum number for resetting mobs */  
  
  
  
/*  
 * Structure for an OLC editor command.  
 */  
struct olc_cmd_type  
{  
    char * const        name;  
    OLC_FUN *           olc_fun;  
};  
  
  
  
/*  
 * Structure for an OLC editor startup command.  
 */  
struct  editor_cmd_type  
{  
    char * const        name;  
    DO_FUN *            do_fun;  
};  
  
  
  
/*  
 * Utils.  
 */  
AREA_DATA *get_vnum_area        args ( ( int vnum ) );  
AREA_DATA *get_area_data        args ( ( int vnum ) );  
int flag_value                  args ( ( const struct flag_type *flag_table, char *argument) );  
char *flag_string               args ( ( const struct flag_type *flag_table, int bits ) );  
void add_reset                  args ( ( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index ) );  
  
  
  
/*  
 * Interpreter Table Prototypes  
 */  
extern const struct olc_cmd_type        aedit_table[];  
extern const struct olc_cmd_type        redit_table[];  
extern const struct olc_cmd_type        oedit_table[];  
extern const struct olc_cmd_type        medit_table[];  
extern const struct olc_cmd_type        mpedit_table[];  
extern const struct olc_cmd_type        hedit_table[];
extern const struct olc_cmd_type        cedit_table[];  
  
  
/*  
 * Editor Commands.  
 */  
DECLARE_DO_FUN( do_aedit        );  
DECLARE_DO_FUN( do_redit        );  
DECLARE_DO_FUN( do_oedit        );  
DECLARE_DO_FUN( do_medit        );  
DECLARE_DO_FUN( do_mpedit       );  
DECLARE_DO_FUN( do_hedit        );
DECLARE_DO_FUN( do_cedit        );  
  
/*  
 * General Functions  
 */  
bool show_commands              args ( ( CHAR_DATA *ch, char *argument ) );  
bool show_help                  args ( ( CHAR_DATA *ch, char *argument ) );  
bool edit_done                  args ( ( CHAR_DATA *ch ) );  
bool show_version               args ( ( CHAR_DATA *ch, char *argument ) );  
  
  
  
/*  
 * Area Editor Prototypes  
 */  
DECLARE_OLC_FUN( aedit_show             );  
DECLARE_OLC_FUN( aedit_create           );  
DECLARE_OLC_FUN( aedit_name             );  
DECLARE_OLC_FUN( aedit_file             );  
DECLARE_OLC_FUN( aedit_age              );  
/* DECLARE_OLC_FUN( aedit_recall        );       ROM OLC */  
DECLARE_OLC_FUN( aedit_reset            );  
DECLARE_OLC_FUN( aedit_security         );  
DECLARE_OLC_FUN( aedit_continent	);
DECLARE_OLC_FUN( aedit_builder          );  
DECLARE_OLC_FUN( aedit_vnum             );  
DECLARE_OLC_FUN( aedit_lvnum            );  
DECLARE_OLC_FUN( aedit_uvnum            );  
DECLARE_OLC_FUN( aedit_credits          );  
  
  
/*  
 * Room Editor Prototypes  
 */  
DECLARE_OLC_FUN( redit_show             );  
DECLARE_OLC_FUN( redit_create           );  
DECLARE_OLC_FUN( redit_name             );  
DECLARE_OLC_FUN( redit_desc             );  
DECLARE_OLC_FUN( redit_ed               );  
DECLARE_OLC_FUN( redit_format           );  
DECLARE_OLC_FUN( redit_north            );  
DECLARE_OLC_FUN( redit_south            );  
DECLARE_OLC_FUN( redit_east             );  
DECLARE_OLC_FUN( redit_west             );  
DECLARE_OLC_FUN( redit_up               );  
DECLARE_OLC_FUN( redit_down             );  
DECLARE_OLC_FUN( redit_mreset           );  
DECLARE_OLC_FUN( redit_oreset           );  
DECLARE_OLC_FUN( redit_mshow            );  
DECLARE_OLC_FUN( redit_oshow            );  
DECLARE_OLC_FUN( redit_heal             );  
DECLARE_OLC_FUN( redit_mana             );  
DECLARE_OLC_FUN( redit_clan             );  
DECLARE_OLC_FUN( redit_owner            );  
DECLARE_OLC_FUN( redit_room             );  
DECLARE_OLC_FUN( redit_sector           );  
DECLARE_OLC_FUN( redit_copy             );  
  
  
/*  
 * Object Editor Prototypes  
 */  
DECLARE_OLC_FUN( oedit_show             );  
DECLARE_OLC_FUN( oedit_create           );  
DECLARE_OLC_FUN( oedit_name             );  
DECLARE_OLC_FUN( oedit_short            );  
DECLARE_OLC_FUN( oedit_long             );  
DECLARE_OLC_FUN( oedit_addaffect        );  
DECLARE_OLC_FUN( oedit_addapply         );  
DECLARE_OLC_FUN( oedit_delaffect        );  
DECLARE_OLC_FUN( oedit_value0           );  
DECLARE_OLC_FUN( oedit_value1           );  
DECLARE_OLC_FUN( oedit_value2           );  
DECLARE_OLC_FUN( oedit_value3           );  
DECLARE_OLC_FUN( oedit_value4           );  /* ROM */  
DECLARE_OLC_FUN( oedit_weight           );  
DECLARE_OLC_FUN( oedit_cost             );  
DECLARE_OLC_FUN( oedit_ed               );  
  
DECLARE_OLC_FUN( oedit_extra            );  /* ROM */  
DECLARE_OLC_FUN( oedit_wear             );  /* ROM */  
DECLARE_OLC_FUN( oedit_type             );  /* ROM */  
DECLARE_OLC_FUN( oedit_affect           );  /* ROM */  
DECLARE_OLC_FUN( oedit_material         );  /* ROM */  
DECLARE_OLC_FUN( oedit_level            );  /* ROM */  
DECLARE_OLC_FUN( oedit_condition        );  /* ROM */  
DECLARE_OLC_FUN( oedit_copy             );  
DECLARE_OLC_FUN( oedit_delete           );  
DECLARE_OLC_FUN( oedit_clan             ); 
  
/*  
 * Mobile Editor Prototypes  
 */  
DECLARE_OLC_FUN( medit_show             );  
DECLARE_OLC_FUN( medit_create           );  
DECLARE_OLC_FUN( medit_name             );  
DECLARE_OLC_FUN( medit_short            );  
DECLARE_OLC_FUN( medit_long             );  
DECLARE_OLC_FUN( medit_shop             );  
DECLARE_OLC_FUN( medit_desc             );  
DECLARE_OLC_FUN( medit_level            );  
DECLARE_OLC_FUN( medit_align            );  
DECLARE_OLC_FUN( medit_spec             );  
DECLARE_OLC_FUN( medit_shield		);
  
DECLARE_OLC_FUN( medit_sex              );  /* ROM */  
DECLARE_OLC_FUN( medit_act              );  /* ROM */  
DECLARE_OLC_FUN( medit_affect           );  /* ROM */  
DECLARE_OLC_FUN( medit_ac               );  /* ROM */  
DECLARE_OLC_FUN( medit_form             );  /* ROM */  
DECLARE_OLC_FUN( medit_part             );  /* ROM */  
DECLARE_OLC_FUN( medit_imm              );  /* ROM */  
DECLARE_OLC_FUN( medit_res              );  /* ROM */  
DECLARE_OLC_FUN( medit_vuln             );  /* ROM */  
DECLARE_OLC_FUN( medit_material         );  /* ROM */  
DECLARE_OLC_FUN( medit_off              );  /* ROM */  
DECLARE_OLC_FUN( medit_size             );  /* ROM */  
DECLARE_OLC_FUN( medit_hitdice          );  /* ROM */  
DECLARE_OLC_FUN( medit_manadice         );  /* ROM */  
DECLARE_OLC_FUN( medit_damdice          );  /* ROM */  
DECLARE_OLC_FUN( medit_race             );  /* ROM */  
DECLARE_OLC_FUN( medit_position         );  /* ROM */  
DECLARE_OLC_FUN( medit_gold             );  /* ROM */ 
DECLARE_OLC_FUN( medit_hitroll          );  /* ROM */  
DECLARE_OLC_FUN( medit_damtype          );  /* ROM */  
DECLARE_OLC_FUN( medit_group            );  /* ROM */  
DECLARE_OLC_FUN( medit_addmprog         );  /* ROM */  
DECLARE_OLC_FUN( medit_delmprog         );  /* ROM */  
DECLARE_OLC_FUN( medit_copy             );  
DECLARE_OLC_FUN( medit_delete           );  
DECLARE_OLC_FUN( medit_clan             );  

/* Mobprog editor */  
DECLARE_OLC_FUN( mpedit_create          );  
DECLARE_OLC_FUN( mpedit_code            );  
DECLARE_OLC_FUN( mpedit_show            );  
DECLARE_OLC_FUN( mpedit_list            );

  
  
/*  
 * Macros  
 */  
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))  
  
/* Return pointers to what is being edited. */  
#define EDIT_MOB(Ch, Mob)       ( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )  
#define EDIT_OBJ(Ch, Obj)       ( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )  
#define EDIT_ROOM(Ch, Room)     ( Room = Ch->in_room )  
#define EDIT_AREA(Ch, Area)     ( Area = (AREA_DATA *)Ch->desc->pEdit )  
#define EDIT_MPCODE(Ch, Code)   ( Code = (MPROG_CODE*)Ch->desc->pEdit )  
  
  
/*  
 * Prototypes  
 */  
/* mem.c - memory prototypes. */  
#define ED      EXTRA_DESCR_DATA  
RESET_DATA      *new_reset_data         args ( ( void ) );  
void            free_reset_data         args ( ( RESET_DATA *pReset ) );  
AREA_DATA       *new_area               args ( ( void ) );  
void            free_area               args ( ( AREA_DATA *pArea ) );  
EXIT_DATA       *new_exit               args ( ( void ) );  
void            free_exit               args ( ( EXIT_DATA *pExit ) );  
ED              *new_extra_descr        args ( ( void ) );  
void            free_extra_descr        args ( ( ED *pExtra ) );  
ROOM_INDEX_DATA *new_room_index         args ( ( void ) );  
void            free_room_index         args ( ( ROOM_INDEX_DATA *pRoom ) );  
AFFECT_DATA     *new_affect             args ( ( void ) );  
void            free_affect             args ( ( AFFECT_DATA* pAf ) );  
SHOP_DATA       *new_shop               args ( ( void ) );  
void            free_shop               args ( ( SHOP_DATA *pShop ) );  
OBJ_INDEX_DATA  *new_obj_index          args ( ( void ) );  
void            free_obj_index          args ( ( OBJ_INDEX_DATA *pObj ) );  
MOB_INDEX_DATA  *new_mob_index          args ( ( void ) );  
void            free_mob_index          args ( ( MOB_INDEX_DATA *pMob ) );  
#undef  ED  
  
void            show_liqlist            args ( ( CHAR_DATA *ch ) );  
void            show_damlist            args ( ( CHAR_DATA *ch ) );  
  
char *          mprog_type_to_name      args ( ( int type ) );  
MPROG_LIST      *new_mprog              args ( ( void ) );  
void            free_mprog              args ( ( MPROG_LIST *mp ) );  
MPROG_CODE      *new_mpcode             args ( (void) );  
void            free_mpcode             args ( ( MPROG_CODE *pMcode));  
  
  
/* Editor de helps */
DECLARE_OLC_FUN( hedit_keyword		);
DECLARE_OLC_FUN( hedit_text		);
DECLARE_OLC_FUN( hedit_new		);
DECLARE_OLC_FUN( hedit_level		);
DECLARE_OLC_FUN( hedit_delete		);
DECLARE_OLC_FUN( hedit_show		);
DECLARE_OLC_FUN( hedit_list		);
DECLARE_OLC_FUN( hedit_create           );
DECLARE_OLC_FUN( hedit_newfile          );
  
// #define EDIT_HELP(Ch, Help)     ( Help = (HELP_DATA *)Ch->desc->pEdit )  
  
// HELP_DATA *new_help args ( (void) );  
void free_help  args( ( HELP_DATA * pHelp));  
// void save_helps args( (void) );  
void save_helps( FILE *fp, HELP_AREA *ha );
void save_other_helps( CHAR_DATA *ch );

DECLARE_OLC_FUN( cedit_show );
DECLARE_OLC_FUN( cedit_name );
DECLARE_OLC_FUN( cedit_whoname );
DECLARE_OLC_FUN( cedit_deathroom );
DECLARE_OLC_FUN( cedit_recall );
DECLARE_OLC_FUN( cedit_independent );
DECLARE_OLC_FUN( cedit_pkill );
DECLARE_OLC_FUN( cedit_coclan );
DECLARE_OLC_FUN( cedit_list );
DECLARE_OLC_FUN( cedit_guard_say );
DECLARE_OLC_FUN( cedit_guard_vnum );
DECLARE_OLC_FUN( cedit_clanskill1 );
DECLARE_OLC_FUN( cedit_clanskill2 );
DECLARE_OLC_FUN( cedit_clanskill3 );
DECLARE_OLC_FUN( cedit_clanskill4 );
DECLARE_OLC_FUN( cedit_clanskill5 );
DECLARE_OLC_FUN( cedit_clanskill6 );

extern bool CLANS_CHANGED;
