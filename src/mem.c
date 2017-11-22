/***************************************************************************
 *  File: mem.c                                                            *
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

#include "everything.h"

/*
 * Globals
 */
extern int          top_reset;
extern int          top_area;
extern int          top_exit;
extern int          top_ed;
extern int          top_room;
extern int          top_mprog_index; /* MProgs */
extern int          top_oprog_index;
extern int          top_rprog_index;
extern struct       lease_data *lease_list;

AREA_DATA          *area_free;
EXTRA_DESCR_DATA   *extra_descr_free;
EXIT_DATA          *exit_free;
ROOM_INDEX_DATA    *room_index_free;
OBJ_INDEX_DATA     *obj_index_free;
SHOP_DATA          *shop_free;
MOB_INDEX_DATA     *mob_index_free;
RESET_DATA         *reset_free;
HELP_DATA          *help_free;

HELP_DATA          *help_last;
static struct       char_group *     group_free;
PROG_LIST         *mprog_free;
PROG_LIST         *oprog_free;
PROG_LIST         *rprog_free;
PROG_CODE         *mpcode_free;
PROG_CODE         *opcode_free;
PROG_CODE         *rpcode_free;

/* MProgs */

void	free_mprog  args ( ( PROG_LIST *mp ) );
void    free_oprog  args ( ( PROG_LIST *op ) );
void    free_rprog  args ( ( PROG_LIST *rp ) );

RESET_DATA         *new_reset_data ( void )
{
     RESET_DATA         *pReset;

     if ( !reset_free )
     {
          pReset = alloc_perm ( sizeof ( *pReset ), "new_reset_data" );
          top_reset++;
     }
     else
     {
          pReset = reset_free;
          reset_free = reset_free->next;
     }

     pReset->next = NULL;
     pReset->command = 'X';
     pReset->arg1 = 0;
     pReset->arg2 = 0;
     pReset->arg3 = 0;

     return pReset;
}

void free_reset_data ( RESET_DATA * pReset )
{
     pReset->next = reset_free;
     reset_free = pReset;
     return;
}

AREA_DATA          *new_area ( void )
{
     AREA_DATA          *pArea;
     char                buf[MAX_INPUT_LENGTH];

     if ( !area_free )
     {
          pArea = alloc_perm ( sizeof ( *pArea ), "new_area" );
          top_area++;
     }
     else
     {
          pArea = area_free;
          area_free = area_free->next;
     }

     pArea->next = NULL;
     pArea->name = str_dup ( "New area" );
     pArea->area_flags = AREA_ADDED;
     pArea->security = 1;
     pArea->builders = str_dup ( "None" );
     pArea->soundfile = str_dup ( "None" );
     pArea->credits = str_dup ( "None" );
     pArea->lvnum = 0;
     pArea->uvnum = 0;
     pArea->age = 0;
     pArea->nplayer = 0;
     pArea->empty = TRUE;	/* ROM patch */
     SNP ( buf, "area%d.are", pArea->vnum );
     pArea->filename = str_dup ( buf );
     pArea->vnum = top_area - 1;

     return pArea;
}

void free_area ( AREA_DATA * pArea )
{
     free_string ( pArea->name );
     free_string ( pArea->filename );
     free_string ( pArea->builders );
     free_string ( pArea->soundfile );
     free_string ( pArea->credits );

     pArea->next = area_free->next;
     area_free = pArea;
     return;
}

EXIT_DATA          *new_exit ( void )
{
     EXIT_DATA          *pExit;

     if ( !exit_free )
     {
          pExit = alloc_perm ( sizeof ( *pExit ), "new_exit" );
          top_exit++;
     }
     else
     {
          pExit = exit_free;
          exit_free = exit_free->next;
     }

     pExit->u1.to_room = NULL;	/* ROM OLC */
     pExit->next = NULL;

	 /*  pExit->vnum         =   0;                        ROM OLC */
     pExit->exit_info = 0;
     pExit->key = 0;
     pExit->keyword = &str_empty[0];
     pExit->description = &str_empty[0];
     pExit->rs_flags = 0;

     return pExit;
}

void free_exit ( EXIT_DATA * pExit )
{
     free_string ( pExit->keyword );
     free_string ( pExit->description );

     pExit->next = exit_free;
     exit_free = pExit;
     return;
}

EXTRA_DESCR_DATA   *new_extra_descr ( void )
{
     EXTRA_DESCR_DATA   *pExtra;

     if ( !extra_descr_free )
     {
          pExtra = alloc_perm ( sizeof ( *pExtra ), "new_ed" );
          top_ed++;
     }
     else
     {
          pExtra = extra_descr_free;
          extra_descr_free = extra_descr_free->next;
     }

     pExtra->keyword = NULL;
     pExtra->description = NULL;
     pExtra->next = NULL;

     return pExtra;
}

void free_extra_descr ( EXTRA_DESCR_DATA * pExtra )
{
     free_string ( pExtra->keyword );
     free_string ( pExtra->description );

     pExtra->next = extra_descr_free;
     extra_descr_free = pExtra;
     return;
}

ROOM_INDEX_DATA    *new_room_index ( void )
{
     ROOM_INDEX_DATA    *pRoom;
     int                 door;

     if ( !room_index_free )
     {
          pRoom = alloc_perm ( sizeof ( *pRoom ), "new_room_ind" );
          top_room++;
     }
     else
     {
          pRoom = room_index_free;
          room_index_free = room_index_free->next;
     }

     pRoom->next = NULL;
     pRoom->people = NULL;
     pRoom->contents = NULL;
     pRoom->extra_descr = NULL;
     pRoom->area = NULL;

     for ( door = 0; door < MAX_DIR; door++ )
          pRoom->exit[door] = NULL;

     pRoom->name = &str_empty[0];
     pRoom->description = &str_empty[0];
     pRoom->notes = &str_empty[0];
     pRoom->vnum = 0;
     pRoom->room_flags = 0;
     pRoom->light = 0;
     pRoom->lease = NULL;
     pRoom->sector_type = 0;

     return pRoom;
}

void free_room_index ( ROOM_INDEX_DATA * pRoom )
{
     int                 door;
     EXTRA_DESCR_DATA   *pExtra;
     RESET_DATA         *pReset;

     free_string ( pRoom->name );
     free_string ( pRoom->description );
     free_string ( pRoom->notes );

     for ( door = 0; door < MAX_DIR; door++ )
     {
          if ( pRoom->exit[door] )
               free_exit ( pRoom->exit[door] );
     }

     for ( pExtra = pRoom->extra_descr; pExtra;
           pExtra = pExtra->next )
     {
          free_extra_descr ( pExtra );
     }

     for ( pReset = pRoom->reset_first; pReset;
           pReset = pReset->next )
     {
          free_reset_data ( pReset );
     }

     free_rprog( pRoom->rprogs );

     pRoom->next = room_index_free;
     room_index_free = pRoom;
     return;
}

AFFECT_DATA        *new_affect ( void )
{
     AFFECT_DATA        *pAf;

     if ( !affect_free )
     {
          pAf = alloc_perm ( sizeof ( *pAf ), "new_affect" );
          top_affect++;
     }
     else
     {
          pAf = affect_free;
          affect_free = affect_free->next;
     }

     pAf->next = NULL;
     pAf->where = TO_AFFECTS;
     pAf->location = 0;
     pAf->modifier = 0;
     pAf->type = 0;
     pAf->duration = 0;
     pAf->bitvector = 0;

     return pAf;
}

void free_affect ( AFFECT_DATA * pAf )
{
     pAf->next = affect_free;
     affect_free = pAf;
     free_string (pAf->caster);
     return;
}

SHOP_DATA          *new_shop ( void )
{
     SHOP_DATA          *pShop;
     int                 buy;

     if ( !shop_free )
     {
          pShop = alloc_perm ( sizeof ( *pShop ), "new_shop" );
          top_shop++;
     }
     else
     {
          pShop = shop_free;
          shop_free = shop_free->next;
     }

     pShop->next = NULL;
     pShop->keeper = 0;

     for ( buy = 0; buy < MAX_TRADE; buy++ )
          pShop->buy_type[buy] = 0;

     pShop->profit_buy = 100;
     pShop->profit_sell = 100;
     pShop->open_hour = 0;
     pShop->close_hour = 23;

     return pShop;
}

void free_shop ( SHOP_DATA * pShop )
{
     pShop->next = shop_free;
     shop_free = pShop;
     return;
}

OBJ_INDEX_DATA     *new_obj_index ( void )
{
     OBJ_INDEX_DATA     *pObj;
     int                 value;

     if ( !obj_index_free )
     {
          pObj = alloc_perm ( sizeof ( *pObj ), "new_obj_ind" );
          top_obj_index++;
     }
     else
     {
          pObj = obj_index_free;
          obj_index_free = obj_index_free->next;
     }

     pObj->next = NULL;
     pObj->extra_descr = NULL;
     pObj->affected = NULL;
     pObj->area = NULL;
     pObj->name = str_dup ( "no name" );
     pObj->short_descr = str_dup ( "(no short description)" );
     pObj->description = str_dup ( "(no description)" );
     pObj->notes = str_dup ("");
     pObj->vnum = 0;
     pObj->item_type = ITEM_TRASH;
     pObj->extra_flags = 0;
     pObj->wear_flags = 0;
     pObj->vflags = 0;
     pObj->count = 0;
     pObj->weight = 0;
     pObj->cost = 0;
     pObj->material = material_lookup ( "" );	/* ROM */
     pObj->condition = 100;	/* ROM */
     for ( value = 0; value < 5; value++ )	/* 5 - ROM */
          pObj->value[value] = 0;

     pObj->new_format = TRUE;	/* ROM */

     return pObj;
}

void free_obj_index ( OBJ_INDEX_DATA * pObj )
{
     EXTRA_DESCR_DATA   *pExtra;
     AFFECT_DATA        *pAf;

     free_string ( pObj->name );
     free_string ( pObj->short_descr );
     free_string ( pObj->description );
     free_string ( pObj->notes );
     free_oprog( pObj->oprogs );

     for ( pAf = pObj->affected; pAf; pAf = pAf->next )
     {
          free_affect ( pAf );
     }

     for ( pExtra = pObj->extra_descr; pExtra;
           pExtra = pExtra->next )
     {
          free_extra_descr ( pExtra );
     }

     pObj->next = obj_index_free;
     obj_index_free = pObj;
     return;
}

MOB_INDEX_DATA     *new_mob_index ( void )
{
     MOB_INDEX_DATA     *pMob;

     if ( !mob_index_free )
     {
          pMob = alloc_perm ( sizeof ( *pMob ), "new_mob_ind" );
          top_mob_index++;
     }
     else
     {
          pMob = mob_index_free;
          mob_index_free = mob_index_free->next;
     }

     pMob->next = NULL;
     pMob->spec_fun = NULL;
     pMob->pShop = NULL;
     pMob->area = NULL;
     pMob->player_name = str_dup ( "no name" );
     pMob->short_descr = str_dup ( "(no short description)" );
     pMob->long_descr = str_dup ( "(no long description)\n\r" );
     pMob->notes = str_dup ( "");
     pMob->description = &str_empty[0];
     pMob->vnum = 0;
     pMob->count = 0;
     pMob->killed = 0;
     pMob->sex = 0;
     pMob->level = 0;
     pMob->act = ACT_IS_NPC;
     pMob->affected_by = 0;
     pMob->detections = 0;
     pMob->protections = 0;
     pMob->alignment = 0;
     pMob->hitroll = 0;
     pMob->race = race_lookup ( "human" );	/* - Hugin */
     pMob->form = 0;		/* ROM patch -- Hugin */
     pMob->parts = 0;		/* ROM patch -- Hugin */
     pMob->imm_flags = 0;	/* ROM patch -- Hugin */
     pMob->res_flags = 0;	/* ROM patch -- Hugin */
     pMob->vuln_flags = 0;	/* ROM patch -- Hugin */
     pMob->material = material_lookup ( "" );	/* -- Hugin */
     pMob->off_flags = 0;	/* ROM patch -- Hugin */
     pMob->size = SIZE_MEDIUM;	/* ROM patch -- Hugin */
     pMob->hit[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
     pMob->hit[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
     pMob->hit[DICE_BONUS] = 0;	/* ROM patch -- Hugin */
     pMob->mana[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
     pMob->mana[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
     pMob->mana[DICE_BONUS] = 0;	/* ROM patch -- Hugin */
     pMob->damage[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
     pMob->damage[DICE_TYPE] = 0;	/* ROM patch -- Hugin */
     pMob->damage[DICE_NUMBER] = 0;	/* ROM patch -- Hugin */
     pMob->start_pos = POS_STANDING;	/*  -- Hugin */
     pMob->default_pos = POS_STANDING;	/*  -- Hugin */
     pMob->gold = 0;

     pMob->new_format = TRUE;	/* ROM */

     return pMob;
}

void free_mob_index ( MOB_INDEX_DATA * pMob )
{
     free_string ( pMob->player_name );
     free_string ( pMob->short_descr );
     free_string ( pMob->long_descr );
     free_string ( pMob->description );
     free_string ( pMob->notes );
     free_mprog( pMob->mprogs );

     free_shop ( pMob->pShop );

     pMob->next = mob_index_free;
     mob_index_free = pMob;
     return;
}

HELP_DATA          *new_help ( void )
{
     HELP_DATA          *NewHelp;

     if ( help_free == NULL )
     {
          NewHelp = alloc_perm ( sizeof ( *NewHelp ), "new_help" );
          top_help++;
     }
     else
     {
          NewHelp = help_free;
          help_free = help_free->next;
     }

     NewHelp->next = NULL;
     NewHelp->level = 0;
     NewHelp->keyword = &str_empty[0];
     NewHelp->text = &str_empty[0];

     return NewHelp;
}

void free_help ( HELP_DATA * pHelp )
{
     if ( pHelp->keyword != NULL )
          free_string ( pHelp->keyword );
     if ( pHelp->text != NULL )
          free_string ( pHelp->text );

     pHelp->next = help_free;
     help_free = pHelp;
     return;
}

LEASE *new_lease ( void )
{
     LEASE        *lease = NULL;
     LEASE        *previous = NULL;

	 /* Very important. Must get previous. Must get previous. */
     for ( lease = lease_list; lease != NULL; lease = lease->next )
          previous = lease;

     lease = alloc_mem ( sizeof ( struct lease_data ), "lease_data" );
     lease->next = NULL;

     if ( lease_list == NULL )
          lease_list = lease;
     else
          previous->next = lease;
     previous=lease;

     lease->room_rent = 50000;
     lease->rented_by = str_dup ("");
     lease->paid_month = 0;
     lease->paid_day = 0;
     lease->paid_year = 0;
     lease->lease_descr = str_dup ("");
     lease->lease_name = str_dup("");
     lease->owner_only = FALSE;
     lease->shop_type = 0;
     lease->shop_gold = 0;
     lease->room = NULL;
	 /* We don't consider this valid until things are set */
     INVALIDATE ( lease );
     return lease;
}

struct char_group *newgroup (void)
{
     static struct char_group groupZero;
     struct char_group *group;
     if ( group_free == NULL )
     {
          group = alloc_perm ( sizeof ( *group ), "newgroup" );
     }
     else
     {
          group = group_free;
          group_free = group_free->next;
     }

     *group = groupZero;

     return group;
}

/* This doesn't clear the pointer in the array,
 * the calling function should do that specifically.
 */
void free_group ( struct char_group *group )
{
    group->next = group_free;
    group->gch = NULL;          /* Don't wanna free this as it could be an existing char */
    group_free = group;
}

PROG_CODE *new_mpcode(void)
{
     PROG_CODE *NewCode;

     if (!mpcode_free)
     {
          NewCode = alloc_perm(sizeof(*NewCode), "new_mpcode" );
          top_mprog_index++;
     }
     else
     {
          NewCode     = mpcode_free;
          mpcode_free = mpcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_mpcode(PROG_CODE *pMcode)
{
     free_string(pMcode->code);
     pMcode->next = mpcode_free;
     mpcode_free  = pMcode;
     return;
}

PROG_LIST *new_mprog(void)
{
     static PROG_LIST mp_zero;
     PROG_LIST *mp;

     if (mprog_free == NULL)
          mp = alloc_perm(sizeof(*mp), "new_mprog");
     else
     {
          mp = mprog_free;
          mprog_free=mprog_free->next;
     }

     *mp = mp_zero;
     mp->vnum             = 0;
     mp->trig_type        = 0;
     mp->code             = str_dup("");
     VALIDATE(mp);
     return mp;
}

void free_mprog(PROG_LIST *mp)
{
     if (!IS_VALID(mp))
          return;

     INVALIDATE(mp);
     mp->next = mprog_free;
     mprog_free = mp;
}

PROG_CODE *new_opcode(void)
{
     PROG_CODE *NewCode;

     if (!rpcode_free)
     {
          NewCode = alloc_perm(sizeof(*NewCode), "opcode" );
          top_oprog_index++;
     }
     else
     {
          NewCode     = opcode_free;
          opcode_free = opcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

PROG_CODE *new_rpcode(void)
{
     PROG_CODE *NewCode;

     if (!rpcode_free)
     {
          NewCode = alloc_perm(sizeof(*NewCode), "rpcode" );
          top_rprog_index++;
     }
     else
     {
          NewCode     = rpcode_free;
          rpcode_free = rpcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_opcode(PROG_CODE *pOcode)
{
     free_string(pOcode->code);
     pOcode->next = opcode_free;
     opcode_free  = pOcode;
     return;
}

void free_rpcode(PROG_CODE *pRcode)
{
     free_string(pRcode->code);
     pRcode->next = rpcode_free;
     rpcode_free  = pRcode;
     return;
}

PROG_LIST *new_oprog(void)
{
     static PROG_LIST op_zero;
     PROG_LIST *op;

     if (oprog_free == NULL)
          op = alloc_perm(sizeof(*op), "oprog" );
     else
     {
          op = oprog_free;
          oprog_free=oprog_free->next;
     }

     *op = op_zero;
     op->vnum             = 0;
     op->trig_type        = 0;
     op->code             = str_dup("");
     VALIDATE(op);
     return op;
}

void free_oprog(PROG_LIST *op)
{
     if (!IS_VALID(op))
          return;

     INVALIDATE(op);
     op->next = oprog_free;
     oprog_free = op;
}

PROG_LIST *new_rprog(void)
{
     static PROG_LIST rp_zero;
     PROG_LIST *rp;

     if (rprog_free == NULL)
          rp = alloc_perm(sizeof(*rp), "rprog");
     else
     {
          rp = rprog_free;
          rprog_free=rprog_free->next;
     }

     *rp = rp_zero;
     rp->vnum             = 0;
     rp->trig_type        = 0;
     rp->code             = str_dup("");
     VALIDATE(rp);
     return rp;
}

void free_rprog(PROG_LIST *rp)
{
     if (!IS_VALID(rp))
          return;

     INVALIDATE(rp);
     rp->next = rprog_free;
     rprog_free = rp;
}
