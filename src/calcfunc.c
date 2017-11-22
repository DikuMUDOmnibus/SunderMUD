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
 * Author: Lotherius (aelfwyne@operamail.com)
 * Purpose:
 * Functions in this file provide standard comparisons for various
 * totals used in-game. The functions here will calculate these totals
 * based on rulesets defined in one place - here, not all over the
 * code.
 */

#include "everything.h"

/*
 * int function c_base_ac: 
 * Parameters: 
 * 1. object *obj 
 * 2. actype = AC_PIERCE, AC_BASH, AC_SLASH, AC_EXOTIC
 * Returns:
 * base AC with no modifiers (except condition) for object.
 */

int c_base_ac ( OBJ_DATA *obj, int actype )
{
     int rval;
     int i;

     if ( !obj )
          return 0;
     
     if ( obj->condition <= 0 ) // Useless objects are.. useless.
          return 0;
     
     if ( obj->item_type != ITEM_ARMOR ) // This should've been checked elsewhere, but doublecheck here.
          return 0;
     
     /* Since we can't count on type being same as [i] */
     for ( i = 0 ; material_table[i].type != obj->material; i++ )
          ;         /* nothing */
     
     switch ( actype )
     {
     case AC_PIERCE:
          rval = material_table[i].pierce;
          break;
     case AC_BASH:
          rval = material_table[i].bash;
          break;
     case AC_SLASH:
          rval = material_table[i].slash;
          break;
     case AC_EXOTIC:
          rval = material_table[i].exotic;
          break;
     default:
          bugf ( "Invalid AC_Type: %d in c_base_ac", actype );
          rval = 0;
     }
     
     /* the metals */
     if ( IS_SET ( material_table[i].flags, MAT_METAL ) )
     {
          if ( IS_SET ( obj->vflags, ARMOR_BANDED ) )		/* 10% bonus */
               rval += ( rval *10 / 100 );
          else if ( IS_SET ( obj->vflags, ARMOR_RING ) )        /* 20% bonus exc piercing, which is -10% */
          {
               if ( actype != AC_PIERCE )
                    rval += ( rval * 20 / 100 );
               else
                    rval -= ( rval * 10 / 100 );
          }
          else if ( IS_SET ( obj->vflags, ARMOR_SCALE ) )	/* 20% bonus all around */
               rval += ( rval * 20 / 100 );
          else if ( IS_SET ( obj->vflags, ARMOR_PLATE ) )	/* 45% bonus */
               rval += ( rval * 45 / 100 );
          
          /* Now check for cast / forged */
          /* Default is considered here to be a combination of the two */
          if ( IS_SET ( obj->vflags, ARMOR_CAST ) )
               rval -= ( rval * 15 / 100 );
          else if ( IS_SET ( obj->vflags, ARMOR_FORGED ) )
               rval += ( rval * 15 / 100 );
          
          /* If an armor is tempered, it gets better against pierce and slash */
          if ( IS_SET ( obj->vflags, ARMOR_TEMPERED ) )
          {
               if ( actype == AC_PIERCE || actype == AC_SLASH )
                    rval += ( rval * 15 / 100 );
          }
     }
     /* The leathers */
     else if ( obj->material == MATERIAL_LEATHER )
     {
          if ( IS_SET ( obj->vflags, ARMOR_HARDENED ) )
               rval += ( rval * 25 / 100 );
          else if ( IS_SET ( obj->vflags, ARMOR_SOFTENED ) )
               rval -= ( rval * 25 / 100 );
          if ( IS_SET ( obj->vflags, ARMOR_STUDDED ) )
               rval += ( rval * 20 / 100 );
     }
     
     if ( IS_SET ( obj->vflags, ARMOR_THICKENED ) )	// Thickened
          rval += ( rval * 10 / 100 );

     /* Fudge it for quality. All default items are not modified here */
     if ( IS_SET ( obj->vflags, ARMOR_BAD_QUALITY ) )
          rval = ( rval * 50 / 100 );				/* 50% quality */
     else if ( IS_SET ( obj->vflags, ARMOR_LOW_QUALITY ) )
          rval = ( rval * 75 / 100 );				/* 75% quality */
     else if ( IS_SET ( obj->vflags, ARMOR_HIGH_QUALITY ) )
          rval += ( rval * 25 / 100 );  			/* 25% BETTER quality */

     /* Do some fudging for item level */
     if ( obj->level <= 99 )
     {
          rval -= ( 100 - obj->level ) / 3;
     }
     
     if ( rval < 0 )
          rval = 0;
     
     rval = (rval * obj->condition / 100 ); // Reduce for condition
     rval = (rval/2);
     
     if ( rval > 99 )	/* Drop to 99 */
          rval = 99;

     return ( rval );
}

/*
 * int fuction c_current_ac
 * Parameters:
 * 1. CHAR_DATA ch : obvious.
 * 2. int where    : body location via wear_info
 * 3. int actype   : = AC_PIERCE, AC_BASH, AC_SLASH, AC_EXOTIC
 * Returns:
 * AC of location with any applicable modifiers ( spells, etc )
 */

int c_current_ac ( CHAR_DATA *ch, int where, int actype )
{
     OBJ_DATA    *obj = NULL;
     OBJ_DATA	 *sobj = NULL;
     int	  k, j;
     
     if ( where == -1 )
          return 0;
     if ( wear_info[where].has_ac == FALSE )
          return 0;

     obj = get_eq_char ( ch, where );
     if ( wear_info[where].supercede > 0 )
          sobj = get_eq_char ( ch, wear_info[where].supercede );

     if ( ch->armor > 0 )
          k = ( ch->armor / 10 );	// 1/10th of magical bonus goes to this location.
     else
          k = 0;

     /* NPC's get some free armor, but less if the actually have armor */
     
     if ( IS_NPC ( ch ) )
          j = ch->level / 3;
     else
          j = 0;

     if ( obj && sobj )	// Got both
     {
          if ( IS_NPC ( ch ) )
               return ( ( c_base_ac ( obj, actype ) ) + ( c_base_ac ( sobj, actype ) / 2 ) + k + ( j/3 ) );
          else
               return ( ( c_base_ac ( obj, actype ) ) + ( c_base_ac ( sobj, actype ) / 2 ) + k );
     }
     else if ( obj && !sobj )	// Got primary only
     {
          if ( IS_NPC ( ch ) )
               return ( c_base_ac ( obj, actype ) + k + ( j / 2 ) );
          else               
               return ( c_base_ac ( obj, actype ) + k );
     }
     else if ( !obj && sobj )	// Got supercede only
     {
          if ( IS_NPC ( ch ) )
               return ( ( c_base_ac ( obj, actype ) / 2 ) + k + ( j / 2 ) );
          else
               return ( ( c_base_ac ( sobj, actype ) / 2 ) + k );
     }
     else			// Got neither
     {
          if ( IS_NPC ( ch ) )
               return ( k+j );
          else
               return ( k );
     }
}

/*
 * int function c_rollweapon:
 * Parameters:
 * 1. object *obj
 * 2. Boolean, return roll ( FALSE ), or return Average ( TRUE )
 * Returns:
 * A rolled damage for the weapon if average is FALSE,
 * An average damage for the weapon if average is TRUE.
 * Adds no modifiers.
 */

//int c_rollweapon ( OBJ_DATA *obj, bool average )
//{
//     
//}
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
