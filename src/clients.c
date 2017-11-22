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
 * In this file, always check for MSP/MXP before checking for Pueblo.
 * 
 * The reason? Newer ZMuds "support" pueblo. However, if you use Pueblo commands,
 * most of them are broken and behave very badly. zMud returns a Puebloclient string,
 * however, which results in lots of broken things that would've worked just fine with
 * the MXP code. Lesson: Check for MXP before checking for Pueblo.
 */

/*
 * Function to strip linefeed. Because ZMud barfs with <SEND></SEND> on a simple linefeed.
 */

char *strip_lf ( char *str )
{
     static char         newstr[MAX_STRING_LENGTH];
     int                 i, j;
     
     for ( i = j = 0; str[i] != '\0'; i++ )
          if ( str[i] != '\n' )
          {
               newstr[j++] = str[i];
          }
     newstr[j] = '\0';
     return newstr;
}


/* Sound Stuff */

void do_sound ( CHAR_DATA * ch, char *argument )
{
     char arg[MAX_STRING_LENGTH];

     if ( IS_NPC(ch) || !ch->desc )
          return;

     argument = one_argument ( argument, arg );
          
     if ( !ch->desc->msp && !ch->desc->mxp && !ch->desc->pueblo )
     {
          ch->desc->msp = TRUE;
          send_to_char ( "Attempting to enable MSP sound. Note: Pueblo and MXP clients do not need to enable sound via this command.\n\r\n\r", ch );
          send_to_char ( "!!SOUND(off U=" TXT_SOUNDURL ")",ch );
          send_to_char ( "!!MUSIC(off U=" TXT_SOUNDURL ")",ch );
          send_to_char ( "\n\rIf you have an older version of Zmud you must read {Whelp MSP{w\n\r", ch);
          send_to_char ( "for information on how to install sounds. Other clients may vary.\n\r", ch);
          send_to_char ( "{WMSP sound is on. You must have an MSP client. (Help MSP)\n\r", ch );
          send_to_char ( "Now showing the sound control panel:\n\r\n\r", ch);
          ch->pcdata->svolume = 75;
          ch->pcdata->mvolume = 50;
     }     
     if ( !str_cmp ( arg, "off" ) )
     {
          if ( ch->desc->mxp )
          {
               send_to_char ( "Cannot disable sound in MXP completely, but we will set the volume to Zero.\n\r", ch );
               ch->pcdata->mvolume = 0;
               ch->pcdata->svolume = 0;
          }
          else if ( ch->desc->pueblo )
          {
               send_to_char ( "Please disable sound in your client with Pueblo.\n\r", ch );
          }
          else if ( ch->desc->msp )
          {
               send_to_char ( "Sound is now off.\n\r", ch );
               send_to_char ( "!!SOUND(off)",ch );
               send_to_char ( "!!MUSIC(off)",ch );
               ch->desc->msp = FALSE;
          }
          else
          {
               send_to_char ( "You don't have sounds enabled to begin with.\n\r", ch );
               return;
          }
          send_to_char ( "\n\rIf you prefer, the \"stop\" command will stop the current sound and leave sound on.\n\r", ch);
          return;
     }

     if ( !str_cmp ( arg, "m" ) || !str_cmp (arg, "music") )
     {
          if ( atoi(argument) < 0 || atoi(argument) > 100)
          {
               send_to_char ("Volume values range from 0 (Off) to 100\%\n\r", ch);
               return;
          }
          form_to_char ( ch, "Music volume: %d\n\r", atoi(argument) );
          ch->pcdata->mvolume = atoi(argument);

          return;
     }

     if ( !str_cmp ( arg, "s" ) || !str_cmp (arg, "sounds") )
     {
          if ( atoi(argument) < 0 || atoi(argument) > 100)
          {
               send_to_char ("Volume values range from 0 (Off) to 100\%\n\r", ch);
               return;
          }
          form_to_char ( ch, "Sound Effects volume: %d\n\r", atoi(argument) );
          ch->pcdata->svolume = atoi(argument);
          return;
     }
     
     form_to_char ( ch, "{G[{W---{G]{w Music              {G[{W---{G]{w Sound Effects\n\r");
     send_to_char (     "----------------------------------------------\n\r", ch);
     form_to_char ( ch, "{G[{W%3s{G]{w (m) Music Volume   {G[{W%3s{G]{w (s) Sounds Volume\n\r",
                    ( ch->pcdata->mvolume > 0 ? itos(ch->pcdata->mvolume) : "OFF" ),
                    ( ch->pcdata->svolume > 0 ? itos(ch->pcdata->svolume) : "OFF" ) );
     send_to_char (     "----------------------------------------------\n\r", ch);
     send_to_char("Syntax: sound <type> <vol>\n\r    (Enter 0 for \"OFF\", type is \"m\" or \"s\")\n\r", ch);
     if ( ch->desc->msp )
          send_to_char("Use \"sound off\" to turn MSP off completely.\n\r", ch);
     return;
}

void do_stop ( CHAR_DATA *ch)
{
     if (IS_NPC(ch) || !ch->desc)
          return;
     else if ( ch->desc->pueblo )
     {
          send_to_char ( "<img xch_sound=stop>", ch );
          send_to_char ( "Currently playing sounds and music halted.\n\r", ch);               
     }
     else if ( ch->desc->mxp )
     {
          send_to_char ( MXP_SECURE "<SOUND off><MUSIC off>" MXP_LLOCK, ch );
          send_to_char ( "Currently playing sounds and music halted.\r\n", ch );
     }
     else if ( ch->desc->msp )
     {
          /* We're not going to clear "playing" because we don't want to restart the song yet. */
          send_to_char ( "!!SOUND(off)",ch );
          send_to_char ( "!!MUSIC(off)",ch );
          send_to_char ( "Currently playing sounds and music halted.\n\r", ch);
     }
     else     
          send_to_char("You don't have sound enabled...\n\r", ch);     
     return;
}

// Stops music on a desc with no visible output.
void stop_music ( DESCRIPTOR_DATA *d )
{
     if ( d->pueblo )
     {
          send_to_desc ( d, "<img xch_sound=stop>" );
          return;
     }
     else if ( d->mxp )
     {
          send_to_desc ( d, MXP_SECURE "<MUSIC off>" MXP_LLOCK );
          return;
     }
     else if ( d->msp )
     {
          send_to_desc ( d, "!!MUSIC(off)" );
          return;
     }
     return;
}

/* Call this with: filename, ch */
/* eventually add sound classes */

void sound ( const char *fname, CHAR_DATA *ch )
{
     if ( IS_NPC(ch) || !ch->desc )
          return;

     if ( ch->desc->mxp )
     {
          form_to_char ( ch, MXP_SECURE "<SOUND %s V=%d U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->svolume );
     }
     else if ( ch->desc->pueblo )
     {
          form_to_char ( ch, "</xch_mudtext><img xch_volume=%d xch_device=wave>", ch->pcdata->svolume );
          form_to_char ( ch, "<img xch_sound=play href=\"" TXT_SOUNDURL "%s\"><xch_mudtext>", fname );
     }
     else if ( ch->desc->msp )
     {
          // URL Tag is from V.3 specification. V.2 spec doesn't have it.
          form_to_char ( ch, "!!SOUND(%s V=%d U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->svolume, fname );
     }

     return;
}

void music ( const char *fname, CHAR_DATA *ch, bool repeat )
{
     if ( IS_NPC(ch) || !ch->desc )
          return;
     
     if ( !ch->desc->msp && !ch->desc->pueblo && !ch->desc->mxp )
          return;

     /* Let's not repeat the same thing over and over. */
     /* This saves bandwidth, and keeps a song that has been stopped from playing */
     
     if ( !str_cmp ( fname, ch->pcdata->mplaying ) )
          return;
     
     free_string ( ch->pcdata->mplaying );
     ch->pcdata->mplaying = str_dup ( fname );
     
     if (repeat)
     {
          if ( ch->desc->mxp )          
               form_to_char ( ch, MXP_SECURE "<MUSIC %s V=%d L=-1 C=1 U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->mvolume );
          else if ( ch->desc->pueblo )
          {
               form_to_char ( ch, "</xch_mudtext><img xch_volume=%d xch_device=midi>", ch->pcdata->mvolume );
               form_to_char ( ch, "<img xch_sound=loop href=\"" TXT_SOUNDURL "%s\"><xch_mudtext>", fname );
          }
          else if ( ch->desc->msp )
               form_to_char ( ch, "!!MUSIC(%s V=%d L=-1 C=1 U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->mvolume, fname );
          
     }
     else
     {
          if ( ch->desc->mxp )
               form_to_char ( ch, MXP_SECURE "<MUSIC %s V=%d C=1 U=" TXT_SOUNDURL ">" MXP_LLOCK, fname, ch->pcdata->mvolume );
          else if ( ch->desc->pueblo )
          {
               form_to_char ( ch, "</xch_mudtext><img xch_volume=%d xch_device=midi>", ch->pcdata->mvolume );
               form_to_char ( ch, "<img xch_sound=play href=\"" TXT_SOUNDURL "%s\"><xch_mudtext>", fname );
          }
          else if ( ch->desc->msp )
               form_to_char ( ch, "!!MUSIC(%s V=%d C=1 U=" TXT_SOUNDURL "%s)", fname, ch->pcdata->mvolume, fname );

     }
     return;
}

// Sets up MXP
//
// If you have any entities to declare, put them here.
// But keep in mind, colors don't translate in entities correctly... Grr...
//

void mxp_init ( DESCRIPTOR_DATA *d )
{
    char buf[MSL];
    
    SNP ( buf, MXP_SECURE
           "<!ELEMENT RName '<FONT \"Comic Sans MS\" COLOR=CYAN> <B>' FLAG=\"RoomName\">"
           "<!ELEMENT RDesc FLAG='RoomDesc'>"
           "<!ELEMENT RExits FLAG='RoomExit'>"
           "<!ELEMENT Ex '<SEND>'>" );
    write_to_buffer ( d, buf, 0 );
    SNP ( buf, MXP_SECURE "<SUPPORT image send frame stat>" );
    write_to_buffer ( d, buf, 0 );
    SNP ( buf, MXP_LLOCK ); // Locked mode, no player MXP tags..... 
    //     if ( d->pueblo )
    //          write_to_buffer ( d, "<xch_mudtext>", 0 );
    //  
    write_to_buffer ( d, buf, 0 );
    
    return;
}

/*
 * Argument is the pathname of the image to display.
 * 
 * Bitmap or GIF (why no PNG support in the clients???)
 *
 * if pageit is true, the text will be paged rather than sent.
 * 
 * Argh.... ZMud displays PUEBLO images just fine, but fuxors its own format!
 * 
 */

void inline_image ( DESCRIPTOR_DATA *d, char *image, char *align, bool pageit )
{
     char buf[MSL];

     if ( d->mxp )
     {
          SNP ( buf, "<IMAGE FName=\"%s\" URL=\"" TXT_IMAGEURL "\" ALIGN=\"%s\">",
                strip_cr(strip_lf(image)), align );
          
          if ( pageit )
          {
               if ( !d->character )
               {
                    bugf( "No character on inline_image paged call." );
                    return;
               }
               page_to_char ( MXP_SECURE, d->character );
               page_to_char ( buf, d->character );
               page_to_char ( MXP_LLOCK, d->character );
          }
          else
          {
               send_to_desc ( d, MXP_SECURE );
               send_to_desc ( d, buf );
               send_to_desc ( d, MXP_LLOCK );
          }
     }
     else if ( d->pueblo && !d->mxp ) // VEry careful NOT to use Pueblo if we have MXP since ZMUD Barfs on Pueblo "emulation"
     {
          SNP ( buf, "<img src=\"" TXT_IMAGEURL "%s\" align=%s>", image, align );
          if ( pageit )
          {
               if ( !d->character )
               {
                    bugf( "No character on inline_image paged call." );
                    return;
               }
               page_to_char ( "</xch_mudtext>", d->character );
               page_to_char ( buf, d->character );
               page_to_char ( "<xch_mudtext>", d->character );
          }
          else
          {
               send_to_desc ( d, "</xch_mudtext>" );
               send_to_desc ( d, buf );
               send_to_desc ( d, "<xch_mudtext>" );
          }
     }
     return;
}

// Allows user to specify an image to display to self from the default location.

void do_image ( CHAR_DATA *ch, char *argument )
{
     inline_image ( ch->desc, argument, "bottom", FALSE );
}

/*
 * Sends an appropriate client tag to center text.
 *
 * Not only does MXP not have a <center> tag that I can find,
 * it also refuses to properly display the Pueblo tag through
 * its compatibility mode. So, no MXP clients get the tag.
 */

void tag_center ( DESCRIPTOR_DATA *d, bool onoff, bool pageit )
{
     char buf[MSL];

     if ( !d->pueblo )
          return;
     
     if ( onoff ) // True = turn centering on.
     {
          if ( d->pueblo && !d->mxp )
          {
               SNP ( buf, "</xch_mudtext><center><xch_mudtext>" );
          }
     }
     else // False = turn centering off.
     {
          if ( d->pueblo && !d->mxp )
          {
               SNP ( buf, "</xch_mudtext></center><xch_mudtext>" );
          }
     }
     if ( pageit ) // Can only page to a char
     {
          if ( !d->character )
          {
               bugf ( "No character on tag_center paged call." );
               return;
          }
          page_to_char ( buf, d->character );
     }
     else
          send_to_desc ( d, buf );
     
     return;
}

/*
 * Opens a "secure" channel for tags.
 * Implemented as char so it can be embedded in other text.
 */
char                         *tag_secure ( DESCRIPTOR_DATA *d )
{
    static char                   buf[128];
    
    buf[0] = '\0';
    
    if ( d->mxp )
        SLCAT ( buf, MXP_SECURE );
    else if ( d->pueblo )
        SLCAT ( buf, "</xch_mudtext>" );
    else
        SLCAT ( buf, "" );
    
    return buf;
}

/*
 * Closes secure tag channel
 * Implemented as char so it can be embedded in other text.
 */
char                         *tag_close ( DESCRIPTOR_DATA *d )
{
    static char                   buf[128];
    
    buf[0] = '\0';
    
    if ( d->mxp )
        SLCAT ( buf, MXP_LOPEN ); /* We're going to use locked open mode now. */
    else if ( d->pueblo )
        SLCAT ( buf, "<xch_mudtext>" );
    else
        SLCAT ( buf, "" );
    return buf;
}



/*
 * Returns a wrapped string that is clickable.
 * 
 * Unfortunately, Pueblo doesn't support a drop-down menu like MXP so this only allows one item.
 */

char *click_cmd ( DESCRIPTOR_DATA *d, char *text, char *command, char *mouseover )
{
     static char buf[MSL];
     
     buf[0] = '\0';

     if ( d->mxp )
     {
          SNP ( buf, MXP_SECURE "<send \"%s\" hint=\"%s\">%s</SEND>" MXP_LLOCK, command, mouseover, strip_cr(strip_lf(text)) );
     }
     else if ( d->pueblo )
     {
          SNP ( buf, "</xch_mudtext><a xch_cmd=\"%s\" xch_hint=\"%s\">%s</a><xch_mudtext>", command, mouseover, text );
     }
     else
     {
          SNP ( buf, text );
     }

     return buf;
}






















