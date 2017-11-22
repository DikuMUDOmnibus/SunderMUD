/**********************************************************
 *************** S U N D E R M U D *** 2 . 0 **************
 **********************************************************
 * The unique portions of the SunderMud code as well as   *
 * the integration efforts for code from other sources is *
 * based primarily on the efforts of:                     *
 *                                                        *
 * Lotherius <aelfwyne@operamail.com> (Alvin W. Brinson)  *
 *    and many others, see "help sundermud" in the mud.   *
 **********************************************************/

/*
 * Purpose of this file: Contain global prototypes for functions
 */

#ifndef _PROTO_H
# define _PROTO_H   1

/* 
 * OS-Dependent: 
 */

# if defined(linux)
char *  crypt           args( ( const char *key, const char *salt ) );
# endif

# ifdef WIN32
void gettimeofday(struct timeval *tv, struct timezone *tz);
# endif

/* 
 * Non-OS-Dependent:
 */

/*
 * Some function prototype shortcuts
 */
# define CD     CHAR_DATA
# define MID    MOB_INDEX_DATA
# define OD     OBJ_DATA
# define OID    OBJ_INDEX_DATA
# define RID    ROOM_INDEX_DATA
# define SF     SPEC_FUN
# define PC     PROG_CODE

/* act_comm.c */
void    check_sex                       args ( ( CHAR_DATA *ch) );
void    add_follower                    args ( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void    stop_follower                   args ( ( CHAR_DATA *ch ) );
void    nuke_pets                       args ( ( CHAR_DATA *ch ) );
void    die_follower                    args ( ( CHAR_DATA *ch ) );
void    leave_group                     args ( ( CHAR_DATA *ch ) );
bool    is_same_group                   args ( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
char    *scramble                       args ( ( char *argument, int modifier ) );
char    *drunk_speech                   args ( ( const char *argument, CHAR_DATA *ch ) );
void    real_delete                     args ( ( CHAR_DATA *ch ) );
void    do_fastquit                     args ( ( CHAR_DATA *ch ) );
void    do_quote                        args ( ( CHAR_DATA *ch ) );
void    offline_delete                  args ( ( char *argument ) );
char    *csc_translate                  args ( ( CHAR_DATA *ch, const char *argument, CHAR_DATA *vch, OBJ_DATA *vob, ROOM_INDEX_DATA *vrm ) );

/* act_info.c */
void    do_count                        args( ( CHAR_DATA *ch, char *argument ) );
void    set_title                       args( ( CHAR_DATA *ch, char *title ) );
bool    is_outside                      args( ( CHAR_DATA *ch ) );
int     score_calc                      args( ( CHAR_DATA *ch ) );

/* act_magic.c */
int     mana_cost               	args ( ( CHAR_DATA *ch, int min_mana, int level) );
bool    saves_spell             	args ( ( int level, CHAR_DATA *victim ) );
void    obj_cast_spell          	args ( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );


/* act_move.c */
void    move_char                       args( ( CHAR_DATA *ch, int door, bool follow ) );
int     total_encumbrance               args( ( CHAR_DATA *ch ) );

/* act_obj.c */
void    do_pay                          args( ( CHAR_DATA *ch, int amount ) );
bool    can_loot                        args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void    get_obj                         args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container ) );
bool    wear_obj_size                   args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int     item_lookup                     args( ( const char *name ) );
void    wear_obj                        args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace) );
bool    belongs                         args( (CHAR_DATA *ch, OBJ_DATA *obj) );

/* act_skill.c */
bool    skill_available                 args ( ( int sn, CHAR_DATA *ch, int reason, CHAR_DATA *mob) );
void    show_current_prac               args ( ( CHAR_DATA *ch ) );
bool    is_skillmaster_skill            args ( ( CHAR_DATA *mob, int sn ) );
void    show_skillmaster_skills         args ( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
int     exp_per_level                   args ( ( CHAR_DATA *ch, int points ) );
void    check_improve                   args ( ( CHAR_DATA *ch, int sn, bool success, int multiplier ) );
int     skill_lookup                    args ( ( const char *name ) );
int     slot_lookup                     args ( ( int slot ) );

/* act_wiz.c */
RID     *find_location          	args ( ( CHAR_DATA *ch, char *arg ) );

/* bit.c */
int     position_lookup         	args ( ( const char *name) );

/* calcfunc.c */
int     c_base_ac                       args ( ( OBJ_DATA *obj, int actype ) );
int	c_current_ac			args ( ( CHAR_DATA *ch, int where, int actype ) );

/* board.c */
void 	finish_note      		args ( ( BOARD_DATA *board, NOTE_DATA *note ) ); /* attach a note to a board */
void 	free_note        		args ( ( NOTE_DATA *note) );                /* deallocate memory used by a note */
void 	load_boards      		args ( ( void ) );                          /* load all boards */
int  	board_lookup     		args ( ( const char *name ) );              /* Find a board with that name */
bool 	is_note_to       		args ( ( CHAR_DATA *ch, NOTE_DATA *note) ); /* is tha note to ch? */
void 	personal_message 		args ( ( const char *sender, const char *to, const char *subject, 
                                                 const int expire_days, const char *text) );
void 	make_note 			args ( ( const char* board_name, const char *sender, const char *to, 
                                                 const char *subject, const int expire_days, const char *text) );
void 	save_notes 			args ( ( void ) );
void 	handle_con_note_to         	args ( ( DESCRIPTOR_DATA *d, char * argument) );
void 	handle_con_note_subject    	args ( ( DESCRIPTOR_DATA *d, char * argument) );
void 	handle_con_note_expire     	args ( ( DESCRIPTOR_DATA *d, char * argument) );
void 	handle_con_note_text       	args ( ( DESCRIPTOR_DATA *d, char * argument) );
void 	handle_con_note_finish          args ( ( DESCRIPTOR_DATA *d, char * argument) );

/* channels.c */
void    channel_message         	args ( ( CHAR_DATA *ch, char *argument, char *channel) );

/* clan.c */
void    boot_clans                      args ( ( void ) );
void    list_clans                      args ( ( CHAR_DATA *ch, char *argument ) );
void    save_clans                      args ( ( void ) );
void    save_one_clan                   args ( ( struct clan_main_type *tmp ) );
struct  clan_main_type *clan_by_short   args ( ( char * argument ) );
bool    is_same_clan                    args ( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void    do_clan_tell                    args ( ( CHAR_DATA *ch, char *argument ) );
void    do_setclan                      args ( ( CHAR_DATA *ch, char *argument ) );
void    do_declan                       args ( ( CHAR_DATA *ch, char *argument ) );
void    do_cedit                        args ( ( CHAR_DATA *ch, char *argument ) );
bool    clan_war                        args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* clients.c */
void    sound                           args ( ( const char *fname, CHAR_DATA *ch ) );
void    music                           args ( ( const char *fname, CHAR_DATA *ch, bool repeat ) );
void    inline_image                    args ( ( DESCRIPTOR_DATA *d, char *image, char *align, bool pageit ) );
void    mxp_init                        args ( ( DESCRIPTOR_DATA *d ) );
void    tag_center                      args ( ( DESCRIPTOR_DATA *d, bool onoff, bool pageit ) );
char   *tag_secure 						args ( ( DESCRIPTOR_DATA *d ) );
char   *tag_close 						args ( ( DESCRIPTOR_DATA *d ) );
char   *click_cmd                       args ( ( DESCRIPTOR_DATA *d, char *text, char *command, char *mouseover ) );
void    stop_music                      args ( ( DESCRIPTOR_DATA *d ) );

/* comm.c */
void    show_string                     args ( ( struct descriptor_data *d, char *input) );
void    close_socket                    args ( ( DESCRIPTOR_DATA *dclose ) );
void    write_to_buffer                 args ( ( DESCRIPTOR_DATA *d, const char *txt, int length ) );
void    form_to_char                    args ( ( CHAR_DATA *ch, char *fmt, ... ) );
void    send_to_char                    args ( ( const char *txt, CHAR_DATA *ch ) );
void    send_to_desc                    args ( ( DESCRIPTOR_DATA *d, const char *txt ) );
void    page_to_char                    args ( ( const char *txt, CHAR_DATA *ch ) );
void    act                             args ( ( const char *format, CHAR_DATA *ch, const void 
                                                *arg1, const void *arg2, int type ) );
void    act_new                         args ( ( const char *format, CHAR_DATA *ch, const void 
                                                *arg1, const void *arg2, int type, int min_pos) );
int     colour                          args ( ( char type, CHAR_DATA *ch, char *string ) );
void    colourconv                      args ( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
int     cstrlen                         args ( ( const char *str ) );
void    send_to_char_bw                 args ( ( const char *txt, CHAR_DATA *ch ) );
void    page_to_char_bw                 args ( ( const char *txt, CHAR_DATA *ch ) );
void    gotoxy                          args ( ( CHAR_DATA *ch, int arg1, int arg2) );
void    copyover_recover                args ( (void));
bool    check_parse_name                args ( ( char *name ) );
void    fwrite_disable                  args ( ( void ) );
void    fwrite_crier                    args ( ( void ) );
void    fwrite_accounts                 args ( ( void ) );
void    boot_db                         args ( ( void ) );
void    area_update                     args ( ( void ) );
void    random_apply                    args ( ( OBJ_DATA *obj, CHAR_DATA *mob ) ); /* random obj */
int     random_spell                    args ( ( int level, int mask, sh_int *type ) ); /* random obj */
void    wield_random_magic              args ( ( CHAR_DATA *mob ) ); /* random obj */
void    wield_random_armor              args ( ( CHAR_DATA *mob ) ); /* random obj */
CD *    create_mobile                   args ( ( MOB_INDEX_DATA *pMobIndex ) );
void    clone_mobile                    args ( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *    create_object                   args ( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void    clone_object                    args ( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void    clear_char                      args ( ( CHAR_DATA *ch ) );
void    free_char                       args ( ( CHAR_DATA *ch ) );
char *  get_extra_descr                 args ( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *   get_mob_index                   args ( ( int vnum ) );
OID *   get_obj_index                   args ( ( int vnum ) );
RID *   get_room_index                  args ( ( int vnum ) );
PC *    get_prog_index                  args ( ( int vnum, int type ) );
char    fread_letter                    args ( ( FILE *fp ) );
int     fread_number                    args ( ( FILE *fp ) );
long    fread_flag                      args ( ( FILE *fp ) );
char *  fread_string                    args ( ( FILE *fp ) );
char *  fread_string_eol                args ( ( FILE *fp ) );
void    fread_to_eol                    args ( ( FILE *fp ) );
char *  fread_word                      args ( ( FILE *fp ) );
long    flag_convert                    args ( ( char letter) );
void *  alloc_mem                       args ( ( int sMem, char * identifier ) );
void *  alloc_perm                      args ( ( int sMem, char * identifier ) );
void    free_mem                        args ( ( void *pMem, int sMem, char * identifier ) );
char *  str_dup                         args ( ( const char *str ) );
void    free_string                     args ( ( char *pstr ) );
int     number_fuzzy                    args ( ( int number ) );
int     number_range                    args ( ( int from, int to ) );
int     number_percent                  args ( ( void ) );
int     number_door                     args ( ( void ) );
int     number_bits                     args ( ( int width ) );
int     number_mm                       args ( ( void ) );
int     dice                            args ( ( int number, int size ) );
int     interpolate                     args ( ( int level, int value_00, int value_32 ) );
void    smash_codes                     args ( ( char *str ) );
bool    str_cmp                         args ( ( const char *astr, const char *bstr ) );
bool    str_prefix                      args ( ( const char *astr, const char *bstr ) );
bool    str_infix                       args ( ( const char *astr, const char *bstr ) );
bool    str_suffix                      args ( ( const char *astr, const char *bstr ) );
char *  capitalize                      args ( ( const char *str ) );
void    append_file                     args ( ( CHAR_DATA *ch, char *file, char *str ) );
void    bugf                            args ( ( const char *fmt, ... ) );
void    log_string                      args ( ( const char *str, ... ) );
void    tail_chain                      args ( ( void ) );
void    reset_area                      args ( ( AREA_DATA * pArea, bool force ) );
void    reset_room                      args ( ( ROOM_INDEX_DATA *pRoom, bool force ) );
void    save_skills                     args ( ( void ) );
void    load_skills                     args ( ( void ) );
void    memlog                          args ( ( char *identifier, int action, int amount ) );                                                                                              

/* db2.c */
void    load_rc                         args ( ( void ) );

/* fight.c */
bool    is_safe                         args ( ( CHAR_DATA *ch, CHAR_DATA *victim, bool backtalk ) );
bool    is_safe_spell                   args ( ( CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void    violence_update                 args ( ( void ) );
void    multi_hit                       args ( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool    damage                          args ( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int class, 
                                                 bool show ) );
bool    new_damage                      args ( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int class,
                                                 bool show, bool dual, int where ) );
void    update_pos                      args ( ( CHAR_DATA *victim ) );
void    raw_kill                        args ( ( CHAR_DATA *victim ) );
void    stop_fighting                   args ( ( CHAR_DATA *ch, bool fBoth ) );
void    death_cry                       args ( ( CHAR_DATA *ch ) );
void    drop_level                      args ( ( CHAR_DATA *ch ) );
bool    check_material_vuln             args ( ( OBJ_DATA *obj, CHAR_DATA *victim ) );

/* handler.c */

bool    is_admin                	args ( ( CHAR_DATA *ch) );
int     count_users             	args ( ( OBJ_DATA *obj) );
int     check_immune            	args ( ( CHAR_DATA *ch, int dam_type) );
int     material_lookup         	args ( ( const char *name) );
char *  material_name           	args ( ( sh_int num ) );
long    material_vuln           	args ( ( sh_int num ) );
long    material_dura           	args ( ( sh_int num ) );
long    material_repa           	args ( ( sh_int num ) );
bool    is_material             	args ( ( sh_int num, long material_flag ) );
int     race_lookup             	args ( ( const char *name) );
int     pcrace_lookup           	args ( ( const char *name) );
int     class_lookup            	args ( ( const char *name) );
int     get_skill               	args ( ( CHAR_DATA *ch, int sn ) );
int     get_weapon_sn           	args ( ( CHAR_DATA *ch, bool dual) );
int     get_weapon_skill       		args ( ( CHAR_DATA *ch, int sn ) );
int     get_age                 	args ( ( CHAR_DATA *ch ) );
void    reset_char              	args ( ( CHAR_DATA *ch )  );
int     get_trust               	args ( ( CHAR_DATA *ch ) );
int     get_curr_stat           	args ( ( CHAR_DATA *ch, int stat ) );
int     can_carry_n             	args ( ( CHAR_DATA *ch ) );
int     can_carry_w             	args ( ( CHAR_DATA *ch ) );
bool    is_name                 	args ( ( char *str, char *namelist ) );
bool    is_full_name            	args ( ( const char *str, char *namelist ) );
bool    is_name_abbv            	args ( ( char *str, char *namelist ) );
void    set_affect              	args ( ( AFFECT_DATA *paf, sh_int type, sh_int level, sh_int duration,
                                                 sh_int location, sh_int modifier, sh_int where,
                                                 int bitvector, char *caster ) );
void    affect_to_char          	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_obj           	args ( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_remove           	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove_obj       	args ( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_strip            	args ( ( CHAR_DATA *ch, int sn ) );
bool    is_affected             	args ( ( CHAR_DATA *ch, int sn ) );
void    affect_join             	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    char_from_room          	args ( ( CHAR_DATA *ch ) );
void    char_to_room            	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void    obj_to_char             	args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_char           	args ( ( OBJ_DATA *obj ) );
OD *    get_eq_char             	args ( ( CHAR_DATA *ch, int iWear ) );
bool    equip_char              	args ( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
bool    unequip_char            	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void    obj_from_room           	args ( ( OBJ_DATA *obj ) );
void    obj_to_room             	args ( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void    obj_to_obj              	args ( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void    obj_from_obj            	args ( ( OBJ_DATA *obj ) );
void    extract_obj             	args ( ( OBJ_DATA *obj ) );
void    extract_char            	args ( ( CHAR_DATA *ch, bool fPull ) );
CD *    get_char_room           	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
CD *    get_char_world          	args ( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_type            	args ( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *    get_obj_list            	args ( ( CHAR_DATA *ch, char *argument, OBJ_DATA *list ) );
OD *    get_obj_carry           	args ( ( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer ) );
OD *    get_obj_wear            	args ( ( CHAR_DATA *ch, char *argument, bool character ));
OD *    get_obj_here            	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
OD *    get_obj_world           	args ( ( CHAR_DATA *ch, char *argument ) );
OD *    create_money            	args ( ( int amount ) );
int     get_obj_number          	args ( ( OBJ_DATA *obj ) );
int     get_obj_weight          	args ( ( OBJ_DATA *obj ) );
bool    room_is_dark            	args ( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    room_is_private         	args ( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    can_see                 	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    can_see_obj             	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_see_room            	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool    can_drop_obj            	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *  item_type_name          	args ( ( OBJ_DATA *obj ) );
char *  affect_loc_name         	args ( ( int location ) );
char *  affect_bit_name         	args ( ( int vector ) );
char *  detect_bit_name         	args ( ( int vector ) );
char *  protect_bit_name        	args ( ( int vector ) );
char *  extra_bit_name          	args ( ( int extra_flags ) );
char *  wear_bit_name           	args ( ( int wear_flags ) );
char *  act_bit_name            	args ( ( int act_flags ) );
char *  off_bit_name            	args ( ( int off_flags ) );
char *  imm_bit_name            	args ( ( int imm_flags ) );
char *  form_bit_name           	args ( ( int form_flags ) );
char *  part_bit_name           	args ( ( int part_flags ) );
char *  weapon_bit_name         	args ( ( int weapon_flags ) );
char *  comm_bit_name           	args ( ( int comm_flags ) );

/* interp.c */
void    interpret               	args ( ( CHAR_DATA *ch, char *argument ) );
bool    is_number               	args ( ( char *arg ) );
int     number_argument         	args ( ( char *argument, char *arg ) );
int     mult_argument           	args ( ( char *argument, char *arg) );
char *  one_argument            	args ( ( char *argument, char *arg_first ) );
char *  one_argument_nl         	args ( ( char *argument, char *arg_first ) );

/* lease.c */
void    save_leases     		args ( ( void ) );

/* mccp.c */
bool    compressStart           	args ( (DESCRIPTOR_DATA *desc) );
bool    compressEnd             	args ( (DESCRIPTOR_DATA *desc) );
bool    processCompressed       	args ( (DESCRIPTOR_DATA *desc) );
bool    writeCompressed         	args ( (DESCRIPTOR_DATA *desc, char *txt, int length) );

/* mem.c */
struct  char_group *newgroup            args ( ( void ) );
void    free_group                      args ( ( struct char_group *group ) );
LEASE   *new_lease                      args ( ( void ) );

/* mob_cmds.c */
void    mob_interpret           	args ( ( CHAR_DATA *ch, char *argument ) );
void    obj_interpret           	args ( ( OBJ_DATA *obj, char *argument ) );
void    room_interpret          	args ( ( ROOM_INDEX_DATA *room, char *argument ) );

/* mob_prog.c */
void    program_flow            	args ( ( int vnum, char *source, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA 
                                                 *room, CHAR_DATA *ch, const void *arg1, const void *arg2, int ptype ) );
void    p_act_trigger           	args ( ( char *argument, CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room,
                                                 CHAR_DATA *ch, const void *arg1, const void *arg2, int type ) );
bool    p_percent_trigger       	args ( ( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
                                                 const void *arg1, const void *arg2, int type ) );
void    p_bribe_trigger         	args ( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool    p_exit_trigger          	args ( ( CHAR_DATA *ch, int dir, int type ) );
void    p_give_trigger          	args ( ( CHAR_DATA *mob, OBJ_DATA *obj, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
                                                 OBJ_DATA *dropped, int type ) );
void    p_greet_trigger        		args ( ( CHAR_DATA *ch, int type ) );
void    p_hprct_trigger         	args ( ( CHAR_DATA *mob, CHAR_DATA *ch ) );

/* notify.c */
void    notify_message			args ( ( CHAR_DATA *ch, long type, long to, char *extra_name ) );

/* obj_cond.c */
char    *obj_cond                       args ( (OBJ_DATA *obj) );
void    check_damage_obj                args ( (CHAR_DATA *ch, OBJ_DATA *obj, int chance, int damtype) );
void    damage_obj                      args ( (CHAR_DATA *ch, OBJ_DATA *obj, int damage, int damtype) );
void    set_obj_cond                    args ( (OBJ_DATA *obj, int condition) );

/* olc.c */
char *  strip_cr                        args ( ( char *str  ) );
bool    run_olc_editor                  args ( ( DESCRIPTOR_DATA *d ) );
char    *olc_ed_name                    args ( ( CHAR_DATA *ch ) );
char    *olc_ed_vnum                    args ( ( CHAR_DATA *ch ) );
int     flag_value                      args ( ( const struct flag_type *flag_table, char *argument) );
char *  flag_string                     args ( ( const struct flag_type *flag_table, int bits ) );
void    save_races                      args ( ( void ) );

/* pfile.c */
void    save_char_obj           	args ( ( CHAR_DATA *ch ) );
bool    load_char_obj           	args ( ( DESCRIPTOR_DATA *d, char *name ) );

/* quest.c */
bool    chance                  	args ( ( int num ) );

/* special.c */
SF *    spec_lookup             	args ( ( const char *name ) );
char *  spec_string             	args ( ( SPEC_FUN *fun ) );      /* OLC */

/* ssm.c */
int     defrag_heap                     args ( ( void ) );

/* string.c */                    
void    string_edit                     args( ( CHAR_DATA *ch, char **pString ) );
void    string_append                   args( ( CHAR_DATA *ch, char **pString ) );
char *  string_replace                  args( ( char * orig, char * old, char * new ) );
void    string_add                      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string                   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg                       args( ( char *argument, char *arg_first, bool fCase ) );
char *  string_unpad                    args( ( char * argument ) );
char *  string_proper                   args( ( char * argument ) );
char *  itos                            args( ( int num ) );

/* update.c */
void    advance_level           	args ( ( CHAR_DATA *ch ) );
void    gain_exp                	args ( ( CHAR_DATA *ch, int gain ) );
void    gain_condition          	args ( ( CHAR_DATA *ch, int iCond, int value ) );
void    update_handler          	args ( ( void ) );
void    undo_mask               	args ( ( CHAR_DATA *ch) );

# undef  CD
# undef  MID
# undef  OD
# undef  OID
# undef  RID
# undef  SF
# undef  PC

#endif /* PROTO_H */
