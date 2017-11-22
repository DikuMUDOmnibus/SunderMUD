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

/* db2.c */

#include "everything.h"
#include "db.h"

extern int flag_lookup args((const char *name, const struct flag_type *flag_table));

/* values for db2.c */
struct       social_type     social_table[MAX_SOCIALS];
int          social_count            = 0;

/* Yeah I do something different here, so shoot me. */

void load_rc ( )
{
     FILE    *fpt;
     char    key[256];
     char    value[256];
     int     done = FALSE;

     if ( ( fpt = fopen ( "../bin/sunder.rc", "r" ) ) == NULL )
     {
          bugf ( "Unable to load bin/sunder.rc. Run autosetup, or copy sunder.rc.autoback to sunder.rc and restart." );
          exit ( 1 );
     }

     while (!done)
     {
          fscanf ( fpt, "%s %s", key, value );

          switch ( UPPER (key[0]))
          {
          case 'C':
               if ( !str_cmp ( key, "COLORLEVEL" ) )
                    mud.clogin = atoi (value);
               break;
          case 'D':
               if ( !str_cmp ( key, "DEATHLEVEL" ) )
                    mud.death = atoi (value);
               else if ( !str_cmp ( key, "DIFFICULTY" ) )
                    mud.mudxp = atoi (value);
               break;
          case 'E':
               if ( !str_cmp (key, "End") )
               {
                    done = TRUE;
                    break;
               }
               break;
          case 'M':
               if ( !str_cmp ( key, "MAKELEVEL" ) )
                    mud.makelevel = atoi(value);
               else if ( !str_cmp ( key, "MAKECOST" ) )
                    mud.makecost = atoi(value);
               else if ( !str_cmp ( key, "MAKEQP" ) )
                    mud.makeqp = atoi(value);
               break;
          case 'N':
               if ( !str_cmp ( key, "NEED_FORDEMI" ) )
                    mud.fordemi = atoi (value);
               else if ( !str_cmp (key, "NOVERIFY") )
                    mud.verify = FALSE;
               break;
          case 'P':
               if ( !str_cmp ( key, "PORT_NUM") )
                    mud.port = atoi (value);
               break;
          case 'V':
               if ( !str_cmp (key, "VERIFY_EMAIL") )
                    mud.verify = TRUE;
               break;
          }
     }
     fclose ( fpt );
     return;
}

/* 
 * snarf a socials file 
 * Modified below to use ~ at the end of the lines.
 * Now loads socials from a set non-area based file.
 */

void load_socials ( void )
{
     SNP ( strArea, "%s%s", DATA_DIR, SOCIAL_FILE );

     if ( !( fpArea = fopen ( strArea, "r" ) ) )
     {
          bugf ( "load_socials: Unable to open %s for reading!", strArea);
          perror ( strArea );
          return;
     }

     for ( ;; )
     {
          struct social_type  social;
          char               *temp;

          /* clear social */
          social.char_no_arg 	= NULL;
          social.others_no_arg 	= NULL;
          social.char_found 	= NULL;
          social.others_found 	= NULL;
          social.vict_found 	= NULL;
          social.char_not_found = NULL;
          social.char_auto 	= NULL;
          social.others_auto 	= NULL;

          temp 			= fread_string ( fpArea );
          if ( !strcmp ( temp, "#0" ) )
               break;;		        /* done - # by itself means a social has ended before all lines were read. */
          				/* $ by itself means that line is blank but more exist. */
          SLCPY ( social.name, temp );

          temp = fread_string ( fpArea );   	/* Read next line */
          if ( !strcmp ( temp, "$" ) )      	/* If it is blank, set char_no_arg null */
               social.char_no_arg = NULL;
          else if ( !strcmp ( temp, "#" ) ) 	/* If it is a #, complete this social and go to the next. */
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.char_no_arg = temp;   // Else, set the field on the social and look for the next.

          temp = fread_string ( fpArea );   // Same thing, for others_no_arg
          if ( !strcmp ( temp, "$" ) )
               social.others_no_arg = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.others_no_arg = temp;

          temp = fread_string ( fpArea );   // Same thing, for char_found
          if ( !strcmp ( temp, "$" ) )
               social.char_found = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.char_found = temp;

          temp = fread_string ( fpArea );  // Same thing, for others_found
          if ( !strcmp ( temp, "$" ) )
               social.others_found = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.others_found = temp;

          temp = fread_string ( fpArea );    // Same thing, for vict_found
          if ( !strcmp ( temp, "$" ) )
               social.vict_found = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.vict_found = temp;

          temp = fread_string ( fpArea );   // Same thing for char_not_found
          if ( !strcmp ( temp, "$" ) )
               social.char_not_found = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.char_not_found = temp;

          temp = fread_string ( fpArea );   // Same thing for char_auto
          if ( !strcmp ( temp, "$" ) )
               social.char_auto = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.char_auto = temp;

          temp = fread_string ( fpArea );    // Same thing for others_auto
          if ( !strcmp ( temp, "$" ) )
               social.others_auto = NULL;
          else if ( !strcmp ( temp, "#" ) )
          {
               social_table[social_count] = social;
               social_count++;
               continue;
          }
          else
               social.others_auto = temp;    // And we're going to assume this social is done without checking :)

          social_table[social_count] = social;
          social_count++;
     }
     fclose ( fpArea );
     strArea[0] = '\0';
     return;
}

