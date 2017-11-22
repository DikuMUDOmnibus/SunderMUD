/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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

/*
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 */

#include "everything.h"
#include <signal.h>
#include <stdarg.h>





/* command procedures needed */
DECLARE_DO_FUN ( do_help );
DECLARE_DO_FUN ( do_look );
DECLARE_DO_FUN ( do_skills );
DECLARE_DO_FUN ( do_outfit );
DECLARE_DO_FUN ( do_board );

/*
 * Socket and TCP/IP stuff.
 */

#ifdef WIN32
# include <io.h>
# define EWOULDBLOCK WSAEWOULDBLOCK
# define TELOPT_ECHO     1       /* echo */
# define GA      249             /* you may reverse the line */
# define SB      250             /* interpret as subnegotiation */
# define IAC     255             /* interpret as command: */
# define DONT    254             /* you are not to use option */
# define DO      253             /* please, you use option */
# define WONT    252             /* I won't use option */
# define WILL    251             /* I will use option */
#else /* End win32 */
# include <fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/telnet.h>
#endif /* End else (unix) */



#define TELOPT_COMPRESS    85  // MCCP V1
#define TELOPT_COMPRESS2   86  // MCCP V2 - Not Yet Used
#define TELOPT_MSP         90  // Mud Sound Protocol
#define TELOPT_MXP         91  // Mud eXtension Protocol

/* Normal Stuff */

const char echo_off_str         [] = { IAC, WILL, TELOPT_ECHO,     '\0' };
const char echo_on_str          [] = { IAC, WONT, TELOPT_ECHO,     '\0' };
const char go_ahead_str         [] = { IAC, GA, '\0' };

/* MCCP */
const char compress_will        [] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const char compress_do          [] = { IAC, DO,   TELOPT_COMPRESS, '\0' };
const char compress_dont        [] = { IAC, DONT, TELOPT_COMPRESS, '\0' };

/* experimental bits */
const char mxp_will             [] = { IAC, WILL, TELOPT_MXP,      '\0' };  // Challenge
const char mxp_do               [] = { IAC, DO,   TELOPT_MXP,      '\0' };  // Granted
const char mxp_dont             [] = { IAC, DONT, TELOPT_MXP,      '\0' };  // Denied

const char msp_will             [] = { IAC, WILL, TELOPT_MSP,      '\0' };  // Challenge
const char msp_do               [] = { IAC, DO,   TELOPT_MSP,      '\0' };  // Granted
const char msp_dont             [] = { IAC, DONT, TELOPT_MSP,      '\0' };  // Denied

/*
 * OS-dependent declarations.
 */

/*  -- Why I removed much of the "OS-Dependent" stuff.
 * Sorry, this is 2002. Half this stuff isn't even relevant, so I'm taking out
 * support for anything but Linux, FreeBSD and Win32. If you need anythying else, 
 * well piffle on you, you can write it yourself. I don't have access to all 
 * these wierd OS's to keep the code working on them - Lotherius
 */

#ifdef WIN32
# include <process.h>
void gettimeofday(struct timeval *tv, struct timezone *tz)
{
     tv->tv_sec = time (0);
     tv->tv_usec = 0;
}
#else /* end win32 */
int gettimeofday    args ( ( struct timeval * tp, struct timezone * tzp ) );
#endif /* end else ( unix ) */

int close           args ( ( int fd ) );

#if !defined (WIN32)
int select          args ( ( int width, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout ) );
int socket          args ( ( int domain, int type, int protocol ) );
#endif /* end win32 */

/*
 * Global variables.
 */
DESCRIPTOR_DATA    *descriptor_free;	         /* Free list for descriptors    */
DESCRIPTOR_DATA    *descriptor_list;	         /* All open descriptors         */
DESCRIPTOR_DATA    *d_next;		         /* Next descriptor in loop      */
FILE               *fpReserve;		         /* Reserved file handle         */
bool                god;		         /* All new chars are gods!      */
bool                merc_down;		         /* Shutdown                     */
bool                wizlock;		         /* Game is wizlocked            */
bool                newlock;		         /* Game is newlocked            */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              current_time;	         /* time of this pulse           */
bool		    MOBtrigger = TRUE;           /* act() switch                 */
int		    mud_desc; 	                 /* Global for Copyover          */
bool		    fCopyOver = FALSE;           /* Are we recovering from copyover */

/*
 * Low-Level Local functions.
 */

void game_loop_unix 		args ( ( int mud_desc ) );
int  init_socket     		args ( ( int port ) );
void new_descriptor 		args ( ( int mud_desc ) );
bool read_from_descriptor 	args ( ( DESCRIPTOR_DATA * d ) );
bool write_to_descriptor	args ( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool write_to_descriptor_2	args ( ( int desc, char *txt, int length ) );

/*
 * High-Level Locals
 */

bool check_reconnect 	args ( ( DESCRIPTOR_DATA * d, char *name, bool fConn ) );
bool check_playing  	args ( ( DESCRIPTOR_DATA * d, char *name ) );
int  main           	args ( ( int argc, char **argv ) );
void nanny              args ( ( DESCRIPTOR_DATA * d, char *argument ) );
bool process_output 	args ( ( DESCRIPTOR_DATA * d, bool fPrompt ) );
void read_from_buffer 	args ( ( DESCRIPTOR_DATA * d ) );
void stop_idling    	args ( ( CHAR_DATA * ch ) );
void mxp_in             args ( ( DESCRIPTOR_DATA * d, char *argument ) );

#if !defined(WIN32)
# define CLOSESOCK(A) close(A)
# define SIN_FAM PF_INET
#else
# define CLOSESOCK(A) closesocket(A)
# define SIN_FAM AF_INET
#endif

// bool is_sub_super_name 	args ( ( char *newname ) );

int main( int argc, char **argv )
{
     struct timeval      now_time;
     
     
     /*
      * Init time.
      */
     gettimeofday ( &now_time, NULL );
     current_time = ( time_t ) now_time.tv_sec;
     strcpy ( str_boot_time, ctime ( &current_time ) );

     /*
      * Reserve one channel for our use.
      */
     if ( ( fpReserve = fopen ( NULL_FILE, "r" ) ) == NULL )
     {
          perror ( NULL_FILE );
          exit ( 1 );
     }

	 /*
	  * Read the RC file.
	  */

     log_string ( "Getting the RC file Settings." );
     load_rc ( );

     if ( argc > 1 )
     {
          if ( !is_number ( argv[1] ) )
          {
               fprintf ( stderr, "Usage: %s [port #]\n", argv[0] );
               exit ( 1 );
          }
          else if ( atoi ( argv[1] ) >= 1024 )
          {
               mud.port = atoi ( argv[1]);
          }
          else
          {
			   /* Don't log if port is 0 */
               if ( atoi ( argv[1] ) > 0 )
                    fprintf ( stderr, "Specified port may not be under 1024. Using sunder.rc\n" );
          }
          if ( mud.port < 1024 )
          {
               fprintf (stderr, "Invalid port setting in Sunder.RC.\n" );
               exit ( 1 );
          }

		  /* Are we recovering from a copyover? */
          if (argv[2] && argv[2][0])
          {
               fCopyOver = TRUE;
               mud_desc = atoi(argv[3]);
              I3_socket = atoi( argv[4] );
          }
          else
               fCopyOver = FALSE;
     }

	 /*
	  * Run the game.
	  */

#ifdef WIN32
     {
          /* Initialise Windows sockets library */
          
          WORD wVersionRequested = MAKEWORD( 1, 1 );
          WSADATA wsadata;
          int err;
          
          /* Need to include library: wsock32.lib for Windows Sockets */
          err = WSAStartup(wVersionRequested, &wsadata);
          if (err)
          {
               fprintf(stderr, "Error %i on WSAStartup\n", err);
               exit(1);
          }
     }
#endif /* WIN32 */
     
     if (!fCopyOver)
     {
          mud_desc = init_socket ( mud.port );
     }
    
    /* Initialize and connect to Intermud-3 */
    I3_main( FALSE, mud.port, fCopyOver );

    /* Initialize do_count number... */
    max_on = 0;
    
     boot_db (  );
     log_string ( TXT_MUDNAME " is now accepting connections on port %d.", mud.port );
     if (fCopyOver)
          copyover_recover();
     mud.nonotify = FALSE;
     game_loop_unix ( mud_desc );

     CLOSESOCK(mud_desc);

#if defined (WIN32)
     /* Cleanup Winsock */
     WSACleanup();
#endif
     
    /*
     * If you're able to determine the difference between a reboot
     * and shutdown here you can replace the 0 with estimate time (in
     * seconds) it will take until the mud is back.
     */
    
    I3_shutdown( 0 );    
    
	 /*
	  * That's all, folks.
	  */
     log_string ( "Normal termination of game." );
     exit ( 0 );
     return 0;
}

int init_socket ( int port )
{
     static struct       sockaddr_in  sa_zero;
     struct              sockaddr_in  sa;
     int                 x = 1;
     int                 fd;

#if !defined(WIN32)
     if ( ( fd = socket ( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
     {
          perror ( "Init_socket: socket" );
          exit ( 1 );
     }
#else
    WORD wVersionRequested = MAKEWORD( 1, 1 );
    WSADATA wsaData;
    int err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
        perror("No useable WINSOCK.DLL");
        exit(1);
    }

    if ( ( fd = socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "Init_socket: socket" );
        exit( 1 );
    }
#endif
     if ( setsockopt ( fd, SOL_SOCKET, SO_REUSEADDR, ( char * ) &x, sizeof ( x ) ) < 0 )
     {
          perror ( "Init_socket: SO_REUSEADDR" );
          CLOSESOCK(fd);
          exit ( 1 );
     }

	 /* Lotherius - do we need SO_DONTLINGER? Is it relevant? Need to look this up. */
#if defined(SO_DONTLINGER) && !defined(SYSV)
     {
          struct linger       ld;

          ld.l_onoff = 1;
          ld.l_linger = 1000;

          if ( setsockopt ( fd, SOL_SOCKET, SO_DONTLINGER,
                            ( char * ) &ld, sizeof ( ld ) ) < 0 )
          {
               perror ( "Init_socket: SO_DONTLINGER" );
               CLOSESOCK(fd);
               exit ( 1 );
          }
     }
#endif

     sa              = sa_zero;
     sa.sin_family   = SIN_FAM;
     sa.sin_port     = htons( mud.port );
     
     if ( bind ( fd, ( struct sockaddr * ) &sa, sizeof ( sa ) ) < 0 )
     {
          perror ( "Init socket: bind" );
          CLOSESOCK(fd);
          exit ( 1 );
     }

     if ( listen ( fd, 3 ) < 0 )
     {
          perror ( "Init socket: listen" );
          CLOSESOCK(fd);
          exit ( 1 );
     }

     return fd;
}

void game_loop_unix ( int mud_desc )
{
     static struct timeval null_time;
     struct timeval      last_time;
     int                 timera;
     int                 timerb;

#ifndef WIN32
     signal( SIGPIPE, SIG_IGN );
#endif
     
     gettimeofday ( &last_time, NULL );
     current_time = ( time_t ) last_time.tv_sec;

#if defined(DEBUGINFO)
     log_string ( "DEBUG: void game_loop_unix: begin" );
#endif

	 /* Main loop */
     while ( !merc_down )
     {
          fd_set              in_set;
          fd_set              out_set;
          fd_set              exc_set;
          DESCRIPTOR_DATA    *d;
          int                 maxdesc;

          timera = current_time;

		  /*
		   * Poll all active descriptors.
		   */
          FD_ZERO ( &in_set );
          FD_ZERO ( &out_set );
          FD_ZERO ( &exc_set );
          FD_SET ( mud_desc, &in_set );
          maxdesc = mud_desc;
          for ( d = descriptor_list; d; d = d->next )
          {
               maxdesc = UMAX ( maxdesc, d->descriptor );
               FD_SET ( d->descriptor, &in_set );
               FD_SET ( d->descriptor, &out_set );
               FD_SET ( d->descriptor, &exc_set );
          }

          if ( select ( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
          {
               perror ( "Game_loop: select: poll" );
               exit ( 1 );
          }

		  /*
		   * New connection?
		   */
          if ( FD_ISSET ( mud_desc, &in_set ) )
               new_descriptor ( mud_desc );

		  /*
		   * Kick out the freaky folks.
		   */
          for ( d = descriptor_list; d; d = d_next )
          {
               d_next = d->next;
               if ( FD_ISSET ( d->descriptor, &exc_set ) )
               {
                    FD_CLR ( d->descriptor, &in_set );
                    FD_CLR ( d->descriptor, &out_set );
                    if ( d->character )
                         save_char_obj ( d->character );
                    d->outtop = 0;
                    close_socket ( d );
#if defined(DEBUGINFO)
                    log_string ( "DEBUG: Kicking off a freaky descriptor" );
#endif
               }               
          }

		  /*
		   * Process input.
		   */

          for ( d = descriptor_list; d; d = d_next )
          {

               d_next = d->next;
               d->fcommand = FALSE;
               d->multi_comm = FALSE;

               if ( FD_ISSET ( d->descriptor, &in_set ) )
               {
                    if ( d->character )
                         d->character->timer = 0;
                    if ( !read_from_descriptor ( d ) )
                    {
                         FD_CLR ( d->descriptor, &out_set );
                         if ( d->character )
                              save_char_obj ( d->character );
                         d->outtop = 0;
                         close_socket ( d );
                         continue;
                    }
               }

               if ( d->character && d->character->wait > 0 )
               {
                    --d->character->wait;
                    continue;
               }

               read_from_buffer ( d );
               if ( d->incomm[0] != '\0' )
               {
                    d->fcommand = TRUE;
                    stop_idling ( d->character );
                    // Instead of calling smash_tidle 40 billion times inside the mud, we call it here,
                    // ONCE and FOR ALL.... sheesh.. Lotherius
                    smash_codes ( d->incomm );                    
                    if ( d->incomm[0] == '' ) 			// Catch the Secure MXP line response
                    {
                         d->fcommand = FALSE;        			// We don't want it sending a prompt.
                         mxp_in ( d, d->incomm );    			// We are using this for more stuff now
                    }
                    else if ( d->showstr_point ) 			// OLC
                         show_string ( d, d->incomm );
                    else if ( d->pString )
                         string_add ( d->character, d->incomm );   	// OLC
                    else
                         switch ( d->connected )
                         {
                         case CON_PLAYING:
                              if ( !run_olc_editor ( d ) )
                                   interpret ( d->character, d->incomm );
                              break;
                         default:
                              nanny ( d, d->incomm );
                              break;
                         }
                    if ( !d->multi_comm )
                         d->incomm[0] = '\0';
               }
          }
          
         /* Check I3 */
         I3_loop( );
         
          /*
           * Autonomous game motion.
           */
          update_handler (  );

          /*
           * Output.
           */
          for ( d = descriptor_list; d; d = d_next )
          {
               d_next = d->next;
               /* If we have zlib */
#if !defined (NOZLIB)
               if ( ( d->fcommand || d->outtop > 0 || d->out_compress )
                    && FD_ISSET ( d->descriptor, &out_set ) )
               {

                    bool ok = TRUE;

                    if ( d->fcommand || d->outtop > 0 )
                         ok = process_output(d, TRUE );
                    if (ok && d->out_compress)
                         ok = processCompressed(d);
                    if (!ok)
                    {
                         if ( d->character && d->character->level > 0 )
                              save_char_obj ( d->character );
                         d->outtop = 0;
                         close_socket ( d );
                    }
               }
#else
               /* If we don't */
               if ( ( d->fcommand || d->outtop > 0 )
                    &&   FD_ISSET(d->descriptor, &out_set) )
               {
                    if ( !process_output( d, TRUE ) )
                    {                
                        if ( d->character && d->character->level > 1)
                             save_char_obj( d->character );
                        d->outtop   = 0;
                        close_socket( d );
                    }
               }
#endif
          }
	

		  /*
		   * Synchronize to a clock.
		   * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		   * Careful here of signed versus unsigned arithmetic.
		   */
          {
               struct timeval      now_time;
               long                secDelta;
               long                usecDelta;

               gettimeofday ( &now_time, NULL );
               usecDelta =	( ( int ) last_time.tv_usec ) -	( ( int ) now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
               secDelta = ( ( int ) last_time.tv_sec ) - ( ( int ) now_time.tv_sec );
               while ( usecDelta < 0 )
               {
                    usecDelta += 1000000;
                    secDelta -= 1;
               }

               while ( usecDelta >= 1000000 )
               {
                    usecDelta -= 1000000;
                    secDelta += 1;
               }

               if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
               {
                    struct timeval      stall_time;

                    stall_time.tv_usec = usecDelta;
                    stall_time.tv_sec = secDelta;
#ifdef WIN32
                    Sleep( (stall_time.tv_sec * 1000L) + (stall_time.tv_usec / 1000L) );
#else
                    
                    if ( select ( 0, NULL, NULL, NULL, &stall_time ) < 0 )
                    {
                         perror ( "Game_loop: select: stall" );
                         exit ( 1 );
                    }
#endif
               }
          }

          gettimeofday ( &last_time, NULL );
          current_time = ( time_t ) last_time.tv_sec;

          timerb = current_time;

          if ( ( timerb - timera ) > 1 )
          {
               bugf ( "Game_Loop took over 1 second! Players will bitch! (%d)", (timerb - timera) );
          }
     }
     return;
}

void new_descriptor ( int mud_desc )
{
     static DESCRIPTOR_DATA d_zero;
     char                buf[MAX_STRING_LENGTH];
     DESCRIPTOR_DATA    *dnew;
     BAN_DATA           *pban;
     struct sockaddr_in  sock;
     struct hostent     *from;
     int                 desc;
     int                 size;
#ifdef WIN32
     unsigned long arg = 1;
#endif

#if defined(DEBUGINFO)
     log_string ( "DEBUG: void new_descriptor: begin" );
#endif

     size = sizeof ( sock );
     getsockname ( mud_desc, ( struct sockaddr * ) &sock, &size );
     if ( ( desc =accept ( mud_desc, ( struct sockaddr * ) &sock,&size ) ) < 0 )
     {
          perror ( "New_descriptor: accept" );
          return;
     }

#if !defined(FNDELAY)
# define FNDELAY O_NDELAY
#endif
     
#ifdef WIN32
     if ( ioctlsocket(desc, FIONBIO, &arg) == -1 )
#else
     if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
#endif
     {
         perror ( "New_descriptor: fcntl: FNDELAY" );
         return;
     }
     
     /*
      * Cons a new descriptor.
      */
     if ( !descriptor_free )
     {
          dnew = alloc_perm ( sizeof ( *dnew ), "dnew" );
     }
     else
     {
          dnew = descriptor_free;
          descriptor_free = descriptor_free->next;
     }

     *dnew = d_zero;
     dnew->descriptor = desc;

     dnew->connected = CON_CHOOSE_TERM;
     dnew->showstr_head = NULL;
     dnew->showstr_point = NULL;
     dnew->outsize = 2000;
     dnew->pEdit = NULL;		/* OLC */
     dnew->pString = NULL;	/* OLC */
     dnew->editor = 0;		/* OLC */
     dnew->outbuf = alloc_mem ( dnew->outsize, "d->outbuf" );

     size = sizeof ( sock );
     if ( getpeername ( desc, ( struct sockaddr * ) &sock, &size ) < 0 )
     {
          perror ( "New_descriptor: getpeername" );
          dnew->host = str_dup ( "(unknown)" );
     }
     else
     {
		  /*
		   * Would be nice to use inet_ntoa here but it takes a struct arg,
		   * which ain't very compatible between gcc and system libraries.
		   */
          int                 addr;

          addr = ntohl ( sock.sin_addr.s_addr );
          SNP ( buf, "%d.%d.%d.%d",
                    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
                    ( addr >> 8 ) & 0xFF, ( addr ) & 0xFF );
          log_string ( "Sock.sinaddr:  %s", buf );
          from = gethostbyaddr ( ( char * ) &sock.sin_addr,
                                 sizeof ( sock.sin_addr ), AF_INET );
          dnew->host = str_dup ( from ? from->h_name : buf );
     }

	 /*
	  * Swiftest: I added the following to ban sites.  I don't
	  * endorse banning of sites, but Copper has few descriptors now
	  * and some people from certain sites keep abusing access by
	  * using automated 'autodialers' and leaving connections hanging.
	  *
	  * Furey: added suffix check by request of Nickel of HiddenWorlds.
	  */
     for ( pban = ban_list; pban; pban = pban->next )
     {
          if ( !str_suffix ( pban->name, dnew->host ) )
          {
               write_to_descriptor_2 ( desc, "Your site has been banned from this mud.\n\r", 0 );
               CLOSESOCK(desc);
               free_string ( dnew->host );
               free_mem ( dnew->outbuf, dnew->outsize, "d->outbuf" );
               dnew->next = descriptor_free;
               descriptor_free = dnew;
               return;
          }
     }

	 /*
	  * Init descriptor data.
	  */
     dnew->next = descriptor_list;
     descriptor_list = dnew;

	 /*
	  * Send any necessary client challenges.
	  */
     
     // Our telnet negotiation challenges go here.
     // This seems to be buggy on win32 for now, the client ends up with a "U" appended
     // to the next line regardless of the telnet client used. Gotta be something simple,
     // however I have yet to find the cause.
#if !defined (NOZLIB)
     write_to_buffer(dnew, compress_will, 0);
#endif // nozlib
     write_to_buffer(dnew, mxp_will, 0 );
     write_to_buffer(dnew, msp_will, 0 );
     
     write_to_buffer( dnew, "This world is Pueblo 2.50 enhanced.\n\r", 0 );
     
     write_to_buffer ( dnew, TXT_MUDNAME " Version " TXT_MUDVERSION "\n\r", 0 );
     switch ( mud.clogin )
     {
     case NOCOLOR:
          dnew->ansi = FALSE;
          {
               extern char *help_greeting;

               send_to_desc ( dnew, "\n\r" );

               if ( help_greeting[0] == '.' )
                    send_to_desc ( dnew, help_greeting + 1 );
               else
                    send_to_desc ( dnew, help_greeting );
          }
          dnew->connected = CON_GET_NAME;
          break;
     case ASSUME_COLOR:
          dnew->ansi = TRUE;
          {
               extern char *help_greeting;

               send_to_desc ( dnew, "\n\r" );

               if ( help_greeting[0] == '.' )
                    send_to_desc ( dnew, help_greeting + 1 );
               else
                    send_to_desc ( dnew, help_greeting );
          }
          dnew->connected = CON_GET_NAME;
          break;
     default:
          write_to_buffer ( dnew, "This world supports " C_GREEN "C" C_RED "O" C_YELLOW "L" C_BLUE "O" C_MAGENTA "R" CLEAR "!\n\r"
                            "If you see color above, you can use color. Do you wish to use color? (y/n) ", 0 );
          dnew->connected = CON_CHOOSE_TERM;
          break;
     }
     return;
}

void close_socket ( DESCRIPTOR_DATA * dclose )
{
     CHAR_DATA          *ch;
     
#if defined(DEBUGINFO)
     log_string ( "DEBUG: void close_socket: begin" );
#endif
     
     if ( dclose->outtop > 0 )
          process_output ( dclose, FALSE );

     if ( dclose->snoop_by )
     {
          write_to_buffer ( dclose->snoop_by, "Your victim has left the game.\n\r", 0 );
     }

     {
          DESCRIPTOR_DATA    *d;

          for ( d = descriptor_list; d; d = d->next )
          {
               if ( d->snoop_by == dclose )
                    d->snoop_by = NULL;
          }
     }

     /* This is correct, one = here, even though Borland complains about it. */
     if ( ( ch = dclose->character ) )
     {
          log_string( "Closing link to %s.", ch->name );

          if ( ch->pet )
          {
               char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
               stop_follower(ch->pet);
               extract_char( ch->pet, TRUE );
          }
          if ( ( dclose->connected == CON_PLAYING ) ||
               ( ( dclose->connected >= CON_NOTE_TO ) &&
                 ( dclose->connected <= CON_NOTE_FINISH ) ) )
          {
               char lbuf[MSL];
               act ( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
			   /* Zeran - notify message */
               notify_message ( ch, NOTIFY_LOSTLINK, TO_CLAN, NULL );
               SNP ( lbuf, "%s has lost link.", ch->name );
               notify_message ( ch, WIZNET_LINK, TO_IMM, lbuf );
               ch->desc = NULL;
          }
          else
          {
               free_char ( dclose->original ? dclose->original : dclose->character );
          }
     }

     if ( d_next == dclose )
          d_next = d_next->next;

     if ( dclose == descriptor_list )
     {
          descriptor_list = descriptor_list->next;
     }
     else
     {
          DESCRIPTOR_DATA    *d;

          for ( d = descriptor_list; d && d->next != dclose;
                d = d->next )
               ;
          if ( d )
               d->next = dclose->next;
          else
               bugf ( "Close_socket: dclose not found." );
     }

#if !defined (NOZLIB)
	 /* Got some mccp cleanup to do? */
     if (dclose->out_compress)
     {
          deflateEnd(dclose->out_compress);
          free_mem(dclose->out_compress_buf, COMPRESS_BUF_SIZE, "out_compress_buf");
          free_mem(dclose->out_compress, sizeof(z_stream), "zstream");
     }
#endif
     
     CLOSESOCK(dclose->descriptor);
     free_string( dclose->host );
     free_mem ( dclose->outbuf, dclose->outsize, "d->outbuf" );

	 /*    free_string(dclose->showstr_head); */
     dclose->next = descriptor_free;
     descriptor_free = dclose;
     return;
}

bool read_from_descriptor ( DESCRIPTOR_DATA * d )
{
    unsigned int                 iStart, iErr;

	 /* Hold horses if pending command already. */
     if ( d->incomm[0] != '\0' )
          return TRUE;

	 /* Check for overflow. */
     iStart = strlen ( d->inbuf );
     if ( iStart >= sizeof ( d->inbuf ) - 10 )
     {
          log_string( "%s input overflow!", d->host );
          write_to_descriptor ( d, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
          return FALSE;
     }

	 /* Snarf input. */

     for ( ;; )
     {
          int                 nRead;


#if !defined(WIN32)
        nRead = read( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart );
#else
        nRead = recv( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart, 0 );
#endif

#ifdef WIN32
          iErr = WSAGetLastError ();
#else
          iErr = errno;
#endif
          if ( nRead > 0 )
          {
               iStart += nRead;
               if ( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
                    break;
          }
          else if ( nRead == 0 )
          {
               log_string ( "EOF encountered on read." );
               return FALSE;
          }
          else if ( iErr == EWOULDBLOCK || iErr == EAGAIN )
               break;
          else
          {
               perror ( "Read_from_descriptor" );
               return FALSE;
          }
     }

     d->inbuf[iStart] = '\0';
     return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer ( DESCRIPTOR_DATA * d )
{
     int                 i, j, k;

	 /*
	  * Hold horses if pending command already.
	  */
     if ( d->incomm[0] != '\0' )
          return;

	 /*
	  * Look for at least one new line.
	  */
     for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
     {
          if ( d->inbuf[i] == '\0' )
               return;
     }

	 /*
	  * Canonical input processing.
	  */
     for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
     {
          if ( k >= MAX_INPUT_LENGTH - 2 )
          {
               write_to_descriptor ( d, "Line too long.\n\r", 0 );

			   /* skip the rest of the line */
               for ( ; d->inbuf[i] != '\0'; i++ )
               {
                    if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
                         break;
               }
               d->inbuf[i] = '\n';
               d->inbuf[i + 1] = '\0';
               break;
          }

          if ( d->inbuf[i] == '\b' && k > 0 )
               --k;
          // Okay this was smashing the ESC code from MXP secure... what to do what to do....
          //
          else if ( isascii ( d->inbuf[i] ) && isprint ( d->inbuf[i] ) )
               d->incomm[k++] = d->inbuf[i];
          else if ( d->inbuf[i] == '' && i == 0 )  // Allow ESC through only if the first character on a line.
               d->incomm[k++] = d->inbuf[i];
          // We check our telnet option negotiations here.
          else if (d->inbuf[i] == (signed char)IAC)
          {
               if (!memcmp(&d->inbuf[i], compress_do, strlen(compress_do))) // Compress do, V1
               {
#if !defined (NOZLIB)
                    i += strlen(compress_do) - 1;
                    compressStart(d);
#endif
               }
               else if (!memcmp(&d->inbuf[i], compress_dont, strlen(compress_dont))) // Compress dont, V1
               {
#if !defined (NOZLIB)
                    i += strlen(compress_dont) - 1;
                    compressEnd(d);
#endif
               }
               else if (!memcmp(&d->inbuf[i], msp_do, strlen(msp_do))) // Do Mud Sound Protocol
               {
                    i += strlen(msp_do) -1;
                    d->msp = TRUE;
               }
               else if (!memcmp(&d->inbuf[i], msp_dont, strlen(msp_dont))) // Turn off mud sound
               {
                    i += strlen(msp_dont) -1;
                    d->msp = FALSE;
               }
               else if (!memcmp(&d->inbuf[i], mxp_do, strlen(mxp_do))) // Turn on MXP
               {
                    i += strlen(mxp_do) -1;
                    d->mxp = TRUE;
                    mxp_init ( d );
               }
               else if (!memcmp(&d->inbuf[i], mxp_dont, strlen(mxp_dont))) // Turn off MXP
               {
                    i += strlen(mxp_dont) -1;
                    d->mxp = FALSE;
               }
          }
     }

	 /*
	  * Finish off the line.
	  */
     if ( k == 0 )
          d->incomm[k++] = ' ';
     d->incomm[k] = '\0';

	 /*
	  * Deal with bozos with #repeat 1000 ...
	  */

	 /* Zeran - open up the spam checking for testing */
     if ( k > 1 || d->incomm[0] == '!' )
     {
          if ( d->incomm[0] != '!' &&
               strcmp ( d->incomm, d->inlast ) )
          {
               d->repeat = 0;
          }
          else
          {
               if ( ++d->repeat >= 40 )
               {
                    CHAR_DATA          *asshole;
                    char		lbuf[MSL];

                    asshole = d->original ? d->original : d->character;
                    log_string ("%s input spamming with [ %s ]!", asshole->name, d->inlast );
                    notify_message ( asshole, WIZNET_SPAM, TO_IMM, lbuf );
                    write_to_descriptor ( d, "\n\r*** KNOCK OFF THE SPAMMING!!!! ***\n\r", 0 );
                    strcpy ( d->incomm, "quit" );
               }
               else if ( d->repeat == 29 )
                    write_to_descriptor ( d,
                                          "\n\r\n\r******************* Spam Warning Strike 1 ******************.\n\r\n\r",
                                          0 );
               else if ( d->repeat == 35 )
                    write_to_descriptor ( d,
                                          "\n\r\n\r******************* !!! Spam Warning Strike 2 !!! ****************.\n\r\n\r",
                                          0 );
          }
     }

	 /*
	  * Do '!' substitution.
	  */
     if ( d->incomm[0] == '!' )
          strcpy ( d->incomm, d->inlast );
     else
          strcpy ( d->inlast, d->incomm );

	 /*
	  * Shift the input buffer.
	  */
     while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
          i++;
     for ( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
          ;
     return;
}

/*
 * Low level output function.
 */
bool process_output ( DESCRIPTOR_DATA * d, bool fPrompt )
{
     extern bool         merc_down;

	 /*
	  * Bust a prompt.
	  */

     if ( !merc_down )
     {
          if ( d->showstr_point )
               send_to_char ( TXT_CONTINUE, d->character );
          else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
               write_to_buffer ( d, "> ", 2 );

          else if ( fPrompt && d->connected == CON_PLAYING )
          {

               CHAR_DATA          *ch;
               CHAR_DATA          *victim;

               ch = d->character;

			   /* battle prompt */
               if ( ( victim = ch->fighting ) != NULL )
               {
                    int                 percent;
                    char                wound[100];
                    char               *pbuff;
                    char                buf[MAX_STRING_LENGTH];
                    char                buffer[MAX_STRING_LENGTH * 2];

                    if ( victim->max_hit > 0 )
                         percent = victim->hit * 100 / victim->max_hit;
                    else
                         percent = -1;

                    if ( percent >= 100 )
                         SNP ( wound, TXT_COND_A );
                    else if ( percent >= 90 )
                         SNP ( wound, TXT_COND_B );
                    else if ( percent >= 75 )
                         SNP ( wound, TXT_COND_C );
                    else if ( percent >= 50 )
                         SNP ( wound, TXT_COND_D );
                    else if ( percent >= 30 )
                         SNP ( wound, TXT_COND_E );
                    else if ( percent >= 15 )
                         SNP ( wound, TXT_COND_F );
                    else if ( percent >= 0 )
                         SNP ( wound, TXT_COND_G );
                    else
                         SNP ( wound, TXT_COND_H );

                    if (can_see(ch, victim) )
                         SNP ( buf, "%s%s \n\r", IS_NPC ( victim )
                                   ? victim->short_descr : victim->name, wound );
                    else
                         SNP (buf, "Someone%s \n\r", wound);
                    buf[0] = UPPER ( buf[0] );
                    pbuff = buffer;
                    colourconv ( pbuff, buf, d->character );
                    write_to_buffer ( d, buffer, 0 );
               }

               ch = d->original ? d->original : d->character;
               if ( !IS_SET ( ch->comm, COMM_COMPACT ) )
                    write_to_buffer ( d, "\n\r", 2 );

               if ( IS_SET ( ch->comm, COMM_PROMPT ) )
               {

                    char                buffer[MAX_STRING_LENGTH * 2];
                    char                buf[MAX_STRING_LENGTH];
                    char                prompt_st[MAX_STRING_LENGTH];
                    char               *prompt_str = prompt_st;
                    char               *pbuff;
                    int                 count, prompt_len;
                    char                st[512];

                    buf[0] = '\0';
                    ch = d->character;

                    if ( IS_SET ( ch->act, PLR_AFK ) )
                    {
                         strcat ( buf, "{r(AFK){x" );
                    }
                    else if ( d->original != NULL )	/*switched IMM */
                    {
                         SNP ( buf, "< %dhp %dm %dmv >", ch->hit,
                                   ch->mana, ch->move );
                    }
                    else
                    {
                         strcpy ( prompt_str, ch->pcdata->prompt );
                         prompt_len = strlen ( prompt_str );
                         for ( count = 0; count <= prompt_len; count++ )
                         {
                              if ( prompt_str[count] != '$' )
                              {
                                   int temp = strlen ( buf );
                                   buf[temp] = prompt_str[count];
                                   buf[temp + 1] = '\0';
                              }
                              else
                                   switch ( prompt_str[count + 1] )
                                   {
                                   case 'a':
                                        {
                                             SNP ( st, "%d", ch->alignment );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'c':
                                        {
                                             SNP ( st, "\n\r" );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'h':
                                        {
                                             SNP ( st, "%d", ch->hit );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'H':
                                        {
                                             SNP ( st, "%d", ch->max_hit );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'm':
                                        {
                                             SNP ( st, "%d", ch->mana );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'M':
                                        {
                                             SNP ( st, "%d", ch->max_mana );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'v':
                                        {
                                             SNP ( st, "%d", ch->move );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'V':
                                        {
                                             SNP ( st, "%d", ch->max_move );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'l':
                                        {
                                             SNP ( st, "%d", ch->level );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'e':
                                        {
                                             SNP ( st, "%d", ch->exp );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'E':
                                        {
                                             int                 need;
                                             need = ( ( ch->level + 1 ) * exp_per_level ( ch, ch->pcdata->points )- ch->exp );
                                             SNP ( st, "%d", need );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'g':
                                        {
                                             SNP ( st, "%ld", ch->gold );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'w':
                                        {
                                             SNP ( st, "%d", ch->wait );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'R':
                                        {
                                             if ( get_trust ( ch ) >= LEVEL_IMMORTAL )
                                             {
                                                  SNP ( st, "%d", ch->in_room->vnum );
                                                  strcat ( buf, st );
                                                  count += 1;
                                                  break;
                                             }
                                             else
                                             {
                                                  int temp  = strlen ( buf );
                                                  buf[temp] = prompt_str[count];
                                                  buf[temp + 1] = prompt_str[count + 1];
                                                  buf[temp + 2] = '\0';
                                                  count += 1;
                                                  break;
                                             }
                                        }
                                   case 'r':
                                        {
                                             SNP ( st, "%s", ch->in_room->name );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'o':	/* encumbrance */
                                        {
                                             SNP ( st, "%d", total_encumbrance ( ch ) );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'N':
                                        {
                                             SNP ( st, "%s", ch->in_room->area->name );
                                             strcat ( buf, st );
                                             count += 1;
                                             break;
                                        }
                                   case 'S':
                                        {
                                             strcat ( buf, "$" );
                                             count += 1;
                                             break;
                                        }
                                   default:
                                        {
                                             int temp =strlen ( buf );
                                             buf[temp] = prompt_str[count];
                                             buf[temp + 1] = prompt_str[count + 1];
                                             buf[temp + 2] = '\0';
                                             count += 1;
                                        }
                                   }
                              /*end switch */
                         }
                         /*end for */
                    }
                    if ( !IS_NPC ( ch ) && ( IS_SET ( ch->act, PLR_WIZINVIS ) ) )
                    {
                         char                tmpbuf[32];

                         SNP ( tmpbuf, " {r({cWizi %d{r){x", ch->invis_level );
                         strcat ( buf, tmpbuf );
                    }
                    if ( !IS_NPC ( ch ) && ( IS_SET ( ch->act, PLR_CLOAK ) ) )
                    {
                         char                tmpbuf[32];

                         SNP ( tmpbuf, " {r({cCloak %d{r){x",
                                   ch->cloak_level );
                         strcat ( buf, tmpbuf );
                    }
                    if ( !IS_NPC ( ch ) && ( ch->desc->editor ) )
                         strcat ( buf, " {M<OLC mode>{x" );
                    strcat ( buf, " " );
                    pbuff = buffer;
                    colourconv ( pbuff, buf, ch );
                    write_to_buffer ( d, pbuff, 0 );
               }

               if ( IS_SET ( ch->comm, COMM_TELNET_GA ) )
                    write_to_buffer ( d, go_ahead_str, 0 );
          }
     }

	 /* Added to avoid ambiguous else */
	 /*
	  * * Short-circuit if nothing to write.
	  */
     if ( d->outtop == 0 )
          return TRUE;

	 /*
	  * Snoop-o-rama.
	  */
     if ( d->snoop_by )
     {
          if ( d->character )
               write_to_buffer ( d->snoop_by, d->character->name, 0 );
          write_to_buffer ( d->snoop_by, "> ", 2 );
          write_to_buffer ( d->snoop_by, d->outbuf, d->outtop );
     }

	 /*
	  * OS-dependent output.
	  */
     if ( !write_to_descriptor ( d, d->outbuf, d->outtop ) )
     {
          d->outtop = 0;
          return FALSE;
     }
     else
     {
          d->outtop = 0;
          return TRUE;
     }
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer ( DESCRIPTOR_DATA * d, const char *txt, int length )
{
	 /*
	  * Find length in case caller didn't.
	  */
     if ( length <= 0 )
          length = strlen ( txt );

	 /*
	  * Initial \n\r if needed.
	  */
     if ( d->outtop == 0 && !d->fcommand )
     {
          d->outbuf[0] = '\n';
          d->outbuf[1] = '\r';
          d->outtop = 2;
     }

	 /*
	  * Expand the buffer as needed.
	  */
     while ( d->outtop + length >= d->outsize )
     {
          char               *outbuf;

          if ( d->outsize > 32000 )
          {
               bugf ( "Buffer overflow. Closing.\n\r" );
               close_socket ( d );
               return;
          }
          outbuf = alloc_mem ( 2 * d->outsize, "d->outbuf" );
          strncpy ( outbuf, d->outbuf, d->outtop );
          free_mem ( d->outbuf, d->outsize, "d->outbuf" );
          d->outbuf = outbuf;
          d->outsize *= 2;
     }

	 /*
	  * Copy.
	  */
     strncpy ( d->outbuf + d->outtop, txt, length + 1 );
     d->outtop += length;
     return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 * There isn't even an ofind anymore :)
 */

bool write_to_descriptor_2 ( int desc, char *txt, int length )
{
     int                 iStart;
     int                 nWrite;
     int                 nBlock;

     if ( length <= 0 )
          length = strlen ( txt );

     for ( iStart = 0; iStart < length; iStart += nWrite )
     {
          nBlock = UMIN ( length - iStart, 4096 );
          nBlock = UMIN( length - iStart, 2048 );
#if !defined(WIN32)
          if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
#else
               if ( ( nWrite = send( desc, txt + iStart, nBlock , 0) ) < 0 )
#endif
             {
                 if ( errno == EWOULDBLOCK || errno == EAGAIN )
                     break;
                 {
                     perror ( "Write_to_descriptor" );
                     return FALSE;
                 }
             }
     }    
     return TRUE;
}

/* mccp: write_to_descriptor wrapper */
bool write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length)
{
#if !defined (NOZLIB)
     if (d->out_compress)
          return writeCompressed(d, txt, length);
     else
#endif
          return write_to_descriptor_2(d->descriptor, txt, length);
}

/* Nasty define to get around lots of typing. Guess I could've written a void function..... owell */
#define STVIEW SNP (buf, TXT_TRAIN, ch->pcdata->train );   \
     send_to_desc ( d, buf );                      \
     SNP ( buf, TXT_CURSTAT,                       \
                ch->perm_stat[STAT_STR],           \
                ch->perm_stat[STAT_INT],           \
                ch->perm_stat[STAT_WIS],           \
                ch->perm_stat[STAT_DEX],           \
                ch->perm_stat[STAT_CON] );         \
     send_to_desc ( d, buf );                      \
     SNP (buf, TXT_MAXSTAT,                        \
               pc_race_table[pcrace].max_stats[STAT_STR],   \
               pc_race_table[pcrace].max_stats[STAT_INT],   \
               pc_race_table[pcrace].max_stats[STAT_WIS],   \
               pc_race_table[pcrace].max_stats[STAT_DEX],   \
               pc_race_table[pcrace].max_stats[STAT_CON] ); \
     send_to_desc ( d, buf );

/* End of define */

/*
 * I now use send_to_desc during nanny, so that colour can be used during logon
 * if it was selected at CON_CHOOSE_TERM or a sufficient client challenge was
 * received. send_to_desc avoids accessing any ch-> structures since they may not
 * be complete yet.
 */

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny ( DESCRIPTOR_DATA * d, char *argument )
{
     DESCRIPTOR_DATA      *d_old, *d_next;
     char                  buf[MAX_STRING_LENGTH];
     char                  arg[MAX_INPUT_LENGTH];
     CHAR_DATA            *ch;
     struct account_type  *tmp = NULL;
     struct account_type  *last = NULL;
     char                 *pwdnew;
     char                 *p;
     int                   iClass, race, pcrace, i;
     bool                  fOld, acc_found, slot_found;

     acc_found = FALSE;
     slot_found = FALSE;

	 /* Delete leading spaces UNLESS character is writing a note */
     if ( d->connected != CON_NOTE_TEXT )
     {
          while ( isspace ( *argument ) )
               argument++;
     }

     ch = d->character;

     switch ( d->connected )
     {
     default:
          bugf ( "Nanny: bad d->connected %d.", d->connected );
          close_socket ( d );
          return;

     case CON_GET_NAME:
          if ( argument[0] == '\0' )
          {
               close_socket ( d );
               return;
          }
          
         if ( !strncmp (argument, "PUEBLOCLIENT", 12 ) )
         {
             if ( !d->mxp ) // ZMud's pueblo emulation gets confused. Pretend therefore to zmud that we don't do Pueblo.
             {
                 d->ansi = TRUE;
                 d->pueblo = TRUE;
                 write_to_buffer( d, "Pueblo Enabled\n\r", 0 );
             }
             return;
         }

          argument[0] = UPPER ( argument[0] );

          if ( !check_parse_name ( argument ) )
          {
               send_to_desc ( d, TXT_PARSENAME );
               log_string ( "Refusing Name: %s@%s", argument, d->host );
               return;
          }

          fOld = load_char_obj ( d, argument );
          ch = d->character;

          if ( IS_SET ( ch->act, PLR_DENY ) )
          {
               log_string( "Denying access to %s@%s.", argument, d->host );
               write_to_buffer ( d, "You are denied access.\n\r", 0 );
               close_socket ( d );
               return;
          }

          if ( check_reconnect ( d, argument, FALSE ) )
          {
               fOld = TRUE;
          }
          else
          {
               if ( wizlock && !IS_IMMORTAL ( ch ) )
               {
                    send_to_desc ( d, "Sorry, the mud is {RWizLocked{w.\n\r"
                                   "This is only done when the game is too unstable to be played.\n\r"
                                   "The staff is, of course, hot on the tracks of the problem.\n\r"
                                   "Do not despair! Try again later.\n\r" );
                    close_socket ( d );
                    return;
               }
          }

          if ( fOld )
          {
			   /* Old player */
               send_to_desc ( d, "{CPassword:{w " );
               write_to_buffer ( d, echo_off_str, 0 );
               d->connected = CON_GET_OLD_PASSWORD;
               return;
          }
          else
          {
			   /* New player */
               if ( newlock )
               {
                    send_to_desc ( d, "Sorry, no new characters are being accepted at the moment.\n\r"
                                   "Please try again later.\n\r" );
                    close_socket ( d );
                    return;
               }
			   /* check for login duplication cheat */
               {
                    DESCRIPTOR_DATA    *tmp;

                    tmp = descriptor_list;
                    for ( ; tmp != NULL; tmp = tmp->next )
                    {
                         CHAR_DATA          *wch;

                         wch = tmp->original ? tmp->original : tmp->character;
                         if ( wch && ( !str_cmp ( wch->name, argument ) ) && ( wch != ch ) )
                         {
                              send_to_desc ( d, "{RThat character is already in the process of creation.\n\r"
                                             "If you were disconnected and the character is 'hung',\n\r"
                                             "you will need to try another name or contact an imm.{w\n\r");
                              close_socket ( d );
                              return;
                         }
                    }
               }

			   /* Lotherius - sub_super_name was crashing the mud, and the rules weren't working. */
			   /* Left it here for future reference incase want to implement a similar idea.      */
			   /* Zeran - call is_sub_super_name to verify that we don't allow */
			   /* subset/superset names to be generated */

               //if ( is_sub_super_name(argument) )
               //{
               //     SNP (buf, "A player with a similar name already exists...please reconnect and try another.\n");
               //     write_to_buffer( d, buf, 0);
               //     close_socket (d);
               //     return;
               //}
               
               SNP ( buf, TXT_NAMERULE "%s " TXT_YESNO " ", argument );
               send_to_desc ( d, buf );
               d->connected = CON_CONFIRM_NEW_NAME;
               return;
          }
          break;

     case CON_GET_OLD_PASSWORD:

          send_to_desc ( d, "\n\r" );

          if ( ch->pcdata->pwd[0] == '\0' )
          {
               send_to_desc ( d, "{R{&Warning! Null password!\n\r Enter New Password: " );
               bugf ( "Null Password Encountered! (%s)", ch->name );
               d->connected = CON_PW_FIX;
               break;
          }

          if ( strcmp ( crypt ( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
          {
               send_to_desc ( d, "{R{&Wrong password.{w\n\r" );
               ch->pcdata->pwd_tries++;
               switch ( ch->pcdata->pwd_tries )
               {
               case 1:
                    send_to_desc ( d, "Try again: " );
                    return;
                    break;
               case 2:
                    send_to_desc ( d, "{RLast Try:{w " );
                    return;
                    break;
               case 3:
                    log_string ( "[Alert!] Repeated login failures for %s.\n\r", d->character->name );
                    close_socket ( d );
                    return;
               }
          }

          ch->pcdata->pwd_tries = 0;

          write_to_buffer ( d, echo_on_str, 0 );

          if ( check_reconnect ( d, ch->name, TRUE ) )
               return;

          if ( check_playing ( d, ch->name ) )
               return;

		  /* Clear the screen now to hide the password on dumb clients that echo it (ala Zmud) */
          write_to_buffer ( d, VT_CLS, 0 );
          {
               char lbuf[MSL];
               SNP (lbuf, "%s@%s has connected.", ch->name, d->host );
               log_string ( lbuf );
               notify_message ( ch, WIZNET_SITES, TO_IMM, lbuf );
          }

          if ( ch->desc->msp ) // Set the base URL
          {
               send_to_char( "!!SOUND(off U=http://sunder.ath.cx/~sunder/sounds/)", ch );
          }
          if ( IS_HERO ( ch ) )
          {
               do_help ( ch, "nobb introcredit" ); // DO NOT REMOVE THIS!!! You may CHANGE THE COLOURS OR APPEARANCE of the helpfile,
                                                   // BUT YOU MAY NOT REMOVE THE CREDITS CONTENT!!!! (Yes, this means you. Read your licenses.)
               do_help ( ch, "nobb imotd" );
               send_to_char( TXT_CONTINUE, ch);
               d->connected = CON_READ_IMOTD;
          }
          else
          {
               do_help ( ch, "nobb introcredit" ); // DO NOT REMOVE THIS!!! You may CHANGE THE COLOURS OR APPEARANCE of the helpfile,
               									   // BUT YOU MAY NOT REMOVE THE CREDITS CONTENT!!!! (Yes, this means you. Read your licenses.)
               do_help ( ch, "nobb motd" );
               send_to_char( TXT_CONTINUE, ch);
               d->connected = CON_READ_MOTD;
          }
          break;

		  /* RT code for breaking link */

     case CON_BREAK_CONNECT:
          switch ( *argument )
          {
          case 'y':
          case 'Y':
               for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
               {
                    d_next = d_old->next;
                    if ( d_old == d || d_old->character == NULL )
                         continue;
                    if ( str_cmp ( ch->name, d_old->character->name ) )
                         continue;
                    close_socket ( d_old );
               }
               if ( check_reconnect ( d, ch->name, TRUE ) )
                    return;
               send_to_desc ( d, "Reconnect attempt failed.\n\rName: " );
               if ( d->character != NULL )
               {
                    free_char ( d->character );
                    d->character = NULL;
               }
               d->connected = CON_GET_NAME;
               break;
          case 'n':
          case 'N':
               write_to_buffer ( d, "Name: ", 0 );
               if ( d->character != NULL )
               {
                    free_char ( d->character );
                    d->character = NULL;
               }
               d->connected = CON_GET_NAME;
               break;
          default:
               send_to_desc ( d, TXT_PLEASEYN );
               break;
          }
          break;

     case CON_CONFIRM_NEW_NAME:
          switch ( *argument )
          {
          case 'y':
          case 'Y':
               send_to_desc ( d, "Welcome!\n\rPlease enter a new {CPassword:{w " );
               write_to_buffer ( d, echo_off_str, 0 );
               d->connected = CON_GET_NEW_PASSWORD;
               break;
          case 'n':
          case 'N':
               send_to_desc ( d, "Okay, who are you then: " );
               free_char ( d->character );
               d->character = NULL;
               d->connected = CON_GET_NAME;
               break;
          default:
               send_to_desc ( d, TXT_PLEASEYN );
               break;
          }
          break;

     case CON_ACCOUNT_PW_NEW:
          send_to_desc ( d, "\n\r" );

          if ( strlen ( argument ) < 5 )
          {
               send_to_desc ( d, "Passwords must be at least five characters long.\n\r{CMaster Password:{w " );
               return;
          }

          pwdnew = crypt ( argument, ch->pcdata->account->acc_name );

          for ( p = pwdnew; *p != '\0'; p++ )
          {
               if ( *p == '~' )
               {
                    send_to_desc ( d, "New password not acceptable, try again.\n\r{CMaster Password:{w " );
                    return;
               }
          }

          ch->pcdata->account->password = str_dup ( pwdnew );
          send_to_desc ( d, "Please retype your {CMaster Password:{w " );
          d->connected = CON_ACCOUNT_PW_CONFIRM;
          break;

     case CON_GET_NEW_PASSWORD:
          write_to_buffer ( d, "\n\r", 2 );

          if ( strlen ( argument ) < 5 )
          {
               send_to_desc ( d, "Passwords must be at least five characters long.\n\r{CPassword:{w " );
               return;
          }

          pwdnew = crypt ( argument, ch->name );
          for ( p = pwdnew; *p != '\0'; p++ )
          {
               if ( *p == '~' )
               {
                    send_to_desc ( d,  "New password not acceptable, try again.\n\r{CPassword:{w " );
                    return;
               }
          }

          ch->pcdata->pwd = str_dup ( pwdnew );
          send_to_desc ( d, "Please retype {Cpassword:{w " );
          d->connected = CON_CONFIRM_NEW_PASSWORD;
          break;

     case CON_CONFIRM_NEW_PASSWORD:

          write_to_buffer ( d, "\n\r", 2 );

          if ( strcmp ( crypt ( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
          {
               send_to_desc ( d, "Passwords don't match.\n\rRetype {Cpassword:{w " );
               d->connected = CON_GET_NEW_PASSWORD;
               return;
          }

          write_to_buffer ( d, echo_on_str, 0 );
          write_to_buffer ( d, VT_CLS, 0 );

          send_to_desc ( d, "\n\r" TXT_MUDNAME " Requires that all characters be associated with an {Yaccount{w.\n\r"
                         "Your account name will be your {YEMAIL ADDRESS{w which must be a valid address, and not\n\r"
                         "anything else. Our {MPrivacy Policy{w is simple: This email address will never leave our\n\r"
                         "hands, and the ONLY email you'll ever receive from us will be account verification, and\n\r"
                         "a notification in the case your service at " TXT_MUDNAME " is ever interrupted, or\n\r"
                         "requires a new address.\n\r\n\r"
                         "If you already have an account, you must still associate this character with it by logging\n\r"
                         "into your account now.\n\r"
                         "If you do not yet have an account at " TXT_MUDNAME ", then you may create one now.\n\r\n\r" );

          send_to_desc ( d, "Enter your {YEMAIL ADDRESS{w: " );

          d->connected = CON_LOG_ACCOUNT;
          break;

     case CON_PW_FIX:
          write_to_buffer ( d, "\n\r", 2 );

          if ( strlen ( argument ) < 5 )
          {
               send_to_desc ( d, "Password must be at least five characters long.\n\r{CPassword:{w " );
               return;
          }

          pwdnew = crypt ( argument, ch->name );
          for ( p = pwdnew; *p != '\0'; p++ )
          {
               if ( *p == '~' )
               {
                    send_to_desc ( d,  "New password not acceptable, try again.\n\r{CPassword:{w ");
                    return;
               }
          }

          ch->pcdata->pwd = str_dup ( pwdnew );
          send_to_desc ( d, "Please retype {CPassword:{w " );
          d->connected = CON_PW_FIX_CONFIRM;
          break;

     case CON_PW_FIX_CONFIRM:

          write_to_buffer ( d, "\n\r", 2 );

          if ( strcmp ( crypt ( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
          {
               send_to_desc ( d, "{RPasswords don't match.{w\n\rRetype {CPassword:{w " );
               d->connected = CON_GET_NEW_PASSWORD;
               return;
          }

          write_to_buffer ( d, echo_on_str, 0 );

          send_to_desc (d, "\n\rPassword fixed. Now you'll need to enter it to log in.\n\r" );
          send_to_desc (d, "{CPassword:{w " );

          d->connected = CON_GET_OLD_PASSWORD;
          break;

     case CON_ACCOUNT_PW_FIX:
          write_to_buffer ( d, "\n\r", 2 );

          if ( strlen ( argument ) < 5 )
          {
               send_to_desc ( d, "Password must be at least five characters long.\n\r{CMaster Password:{w " );
               return;
          }

          pwdnew = crypt ( argument, ch->name );
          for ( p = pwdnew; *p != '\0'; p++ )
          {
               if ( *p == '~' )
               {
                    send_to_desc ( d,  "New password not acceptable, try again.\n\r{CMaster Password:{w " );
                    return;
               }
          }

          ch->pcdata->account->password = str_dup ( pwdnew );
          send_to_desc ( d, "Please retype {Cpassword:{w " );
          d->connected = CON_ACCOUNT_PW_FIX_VERIFY;
          break;

     case CON_ACCOUNT_PW_FIX_VERIFY:

          write_to_buffer ( d, "\n\r", 2 );

          if ( strcmp ( crypt ( argument, ch->pcdata->account->password ),
                        ch->pcdata->account->password ) )
          {
               send_to_desc ( d, "{RPasswords don't match.{w\n\rRetype {CMaster password:{w " );
               d->connected = CON_GET_NEW_PASSWORD;
               return;
          }

          write_to_buffer ( d, echo_on_str, 0 );
          send_to_desc (d, "\n\rMaster Password fixed.\n\r\n\r[Strike Enter. Hard.]" );

          d->connected = CON_READ_MOTD;
          break;

     case CON_ACCOUNT_PW_CONFIRM:

          write_to_buffer ( d, "\n\r", 2 );

          if ( strcmp ( crypt ( argument, ch->pcdata->account->password ), ch->pcdata->account->password ) )
          {
               send_to_desc ( d, "{RPasswords don't match.{w\n\rRetype {CMaster Password:{w " );
               d->connected = CON_ACCOUNT_PW_NEW;
               return;
          }

          write_to_buffer ( d, echo_on_str, 0 );
          fwrite_accounts();
          write_to_buffer ( d, VT_CLS, 0 );

          send_to_desc ( d, "\n\rTo find out more about how accounts work, after you have logged"
                         "\n\rinto the game, type \"{Chelp account_info{w\" and \"{Chelp account{w\".\n\r\n\r" );
          send_to_desc ( d, TXT_NODEMI );
          send_to_desc ( d, TXT_RACE );
          send_to_desc ( d, TXT_RCFROM );

          ch->pcdata->mortal = TRUE;

          for ( pcrace = 1; pcrace < MAX_PCRACE ; pcrace++ )
          {
               SNP (buf, TXT_RCTEMP, pc_race_table[pcrace].name );
               send_to_desc ( d, buf );
          }
          send_to_desc ( d, TXT_RCPRMP );
          d->connected = CON_GET_NEW_RACE;
          break;

     case CON_LOG_ACCOUNT:
          for ( tmp = account_list; tmp != NULL; tmp = tmp->next )
          {
               if (!str_cmp (argument, tmp->acc_name) )
               {
                    acc_found = TRUE;
                    break;
               }
          }
		  /* Else, new account */

          if (!acc_found)
          {
               if ( (strlen(argument) < 7) || ( strlen(argument) > 55) || ( strstr ( argument, "@" ) == NULL )
                    || (strstr ( argument, "." ) == NULL ) )
               {
                    send_to_desc ( d, "\n\r\n\rI don't believe you. What's your real email address?\n\r");
                    send_to_desc ( d, "\n\rEnter your EMAIL ADDRESS (Required): " );
                    return;
               }

			   /* Move this stuff into parse_email later */
			   /* Probably make it automatic like the sitebans someday */

               if ( (strstr (argument, "hotmail") != NULL) ||
                    ( strstr (argument, "yahoo") !=NULL) ||
                    ( strstr (argument, "excite") != NULL) ||
                    ( strstr (argument, "localhost") != NULL) ||
                    ( strstr (argument, "nospam") != NULL ) ||
                    ( strstr (argument, "127.0.0.1") != NULL) ||
                    ( strstr (argument, "..") != NULL) ||
                    ( strstr (argument, "root" ) != NULL) ||
                    ( strstr (argument, "@@") != NULL) )
               {
                    send_to_desc ( d, TXT_BADEMAIL TXT_MUDEMAIL "\n\r" );
                    send_to_desc ( d, "\n\rEnter your EMAIL ADDRESS (Required): " );
                    return;
               }

			   /* Check for Invalid Characters */
               {
                    char *pc;
                    for ( pc = argument; *pc != '\0'; pc++ )
                    {
                         if (
                             !isalnum ( *pc ) &&
                             (*pc != '@') &&
                             (*pc != '.') &&
                             (*pc != '_') &&
                             (*pc != '-')
                             )
                         {
                              send_to_desc ( d, "\n\rSorry, email addresses may ONLY contain alpha-numeric characters,"
                                             "\n\rand the symbols @ . _ -\n\rPlease try again.\n\r"
                                             "\n\rEnter your EMAIL ADDRESS (Required): " );
                              return;
                         }
                         /* End if statement */
                    }
                    /* End for pc = name */
               }
               /* End check for invalid chars */

               for ( tmp = account_list ; tmp != NULL ; tmp = tmp->next )
               {
                    last = tmp;
               }

               tmp = alloc_perm ( sizeof ( struct account_type ), "account_type::new" );
               tmp->next = NULL;
               tmp->acc_name = str_dup ( argument ); /* Email Address of Account */

               last->next = tmp;

               tmp->status = ACCT_CREATED;
               tmp->permadead = 0;
               tmp->heroes = 0;
               tmp->demigods = 0;
               tmp->vcode = 0;

			   /* Null it all out */

               for (i = 0 ; i < MAX_CHARS ; i++)
               {
                    tmp->char_name[i] = NULL;
               }

			   /* Need a wiznet notify here */

               send_to_desc ( d, "\n\rWelcome to " TXT_MUDNAME " running " TXT_MUDVERSION ".\n\r" );

               send_to_desc( d, "\n\rYou now have a {YMASTER ACCOUNT{w. Any characters you create\n\r"
                             "will be associated with this account. Each person is only allowed\n\r"
                             "one account, but you may create multiple characters within the\n\r"
                             "account. When creating a new character, simply enter your email\n\r"
                             "address again, then enter your {CMaster Password{w.\n\r\n\r" );
               send_to_desc ( d, "You may now choose a {CMaster Password{w. This is different than the \n\r"
                              "password you chose for your character and will ensure that only\n\r"
                              "you may access your account.\n\r\n\r" );

               SNP ( buf, "Give me a {CMaster Password{w for {W%s{w: %s",
                         tmp->acc_name, echo_off_str );
               send_to_desc ( d, buf );

               ch->pcdata->account = tmp;

               d->connected = CON_ACCOUNT_PW_NEW;
               break;
          }

          ch->pcdata->account = tmp;

          if (ch->pcdata->account->status <= ACCT_REJECTED_OTHER)
          {
               send_to_desc ( d, "\n\r{R{&Your account has been DENIED access from the mud.{w\n\r" );
               switch (ch->pcdata->account->status)
               {
               case ACCT_REJECTED_EMAIL:
                    send_to_desc ( d, "REASON: Bad Email Given.\n\r" );
                    send_to_desc ( d, TXT_BADEMAIL TXT_MUDEMAIL "\n\r" );
                    break;
               case ACCT_REJECTED_RULES:
                    send_to_desc (d, "REASON: Rules violation. Shame.\n\r");
                    break;
               }
			   /* Disconnect */
               close_socket ( d );
               return;
          }

	/* Welcome Back */
	/* Check for name, MAX_CHARS */

          for (i = 0 ; i < MAX_CHARS ; i++)
          {
               if (!ch->pcdata->account->char_name[i] )
               {
                    slot_found = TRUE;
                    break;
               }
          }

          if (!slot_found)
          {
               send_to_desc ( d, "Sorry, you have reached the maximum number of characters allowed.\n\r"
                              "Please delete one or more before creating another.\n\r" );
               close_socket ( d );
               return;
          }

          send_to_desc ( d, "{CMaster Password:{w " );
          write_to_buffer ( d, echo_off_str, 0 );

          d->connected = CON_ACCOUNT_PW;
          break;

     case CON_ACCOUNT_PW:

          send_to_desc ( d, "\n\r" );

          if ( strcmp ( crypt ( argument, ch->pcdata->account->password ), ch->pcdata->account->password ) )
          {
               send_to_desc ( d, "{RWrong Master Password.{w\n\r" );
               ch->pcdata->pwd_tries++;
               switch ( ch->pcdata->pwd_tries )
               {
               case 1:
                    send_to_desc ( d, "{YTry again:{w " );
                    return;
                    break;
               case 2:
                    send_to_desc ( d, "{RLast try:{w " );
                    return;
                    break;
               case 3:
                    log_string ( "[Alert!] Repeated Master Password failures for %s:%s.\n\r",
                              d->character->name, d->character->pcdata->account->acc_name );
                    close_socket ( d );
                    return;
               }
          }

          ch->pcdata->pwd_tries = 0;

          write_to_buffer ( d, echo_on_str, 0 );

          if (ch->pcdata->account->status >= ACCT_VERIFIED_DEMISTAT)
          {
               send_to_desc ( d, TXT_YESDEMI );
               send_to_desc ( d, TXT_MORD );
               d->connected = CON_FIND_MORTALS;
          }
          else
          {
               send_to_desc ( d, TXT_NODEMI );
               send_to_desc ( d, TXT_RCFROM );

               ch->pcdata->mortal = TRUE;

               // 0 is null, start at 1
               for ( pcrace = 1; pcrace < MAX_PCRACE ; pcrace++ )
               {
                    SNP (buf, TXT_RCTEMP, pc_race_table[pcrace].name );
                    send_to_desc ( d, buf );
               }
               send_to_desc ( d, TXT_RCPRMP );
               d->connected = CON_GET_NEW_RACE;
          }
          break;

     case CON_FIND_MORTALS:

          one_argument ( argument, arg );

          if ( !strcmp ( arg, "help" ) )
          {
               argument = one_argument ( argument, arg );
               if ( argument[0] == '\0' )
                    do_help ( ch, "demigod" );
               else
                    do_help ( ch, argument );
               send_to_desc ( d, TXT_MORD );
               break;
          }

          switch ( arg[0] )
          {
          case 'm':
          case 'M':
               ch->pcdata->mortal = TRUE;
               break;
          case 'd':
          case 'D':
               ch->pcdata->mortal = FALSE;
               break;
          default:
               send_to_desc ( d, "You may not escape fate. You must make this choice..\n\r" TXT_MORD );
               return;
          }

          if (ch->pcdata->mortal)
          {
               send_to_desc ( d, "You have chosen the dangerous path of the {GMortal{w.\n\r" );
               send_to_desc ( d, "Read 'help formortals' after you begin for information.\n\r" );
          }
          else
          {
               send_to_desc ( d, "You have chosen the path of {YEverlasting Power{w.\n\r" );
               send_to_desc ( d, "Read 'help fordemigods' after you begin for information.\n\r" );
          }

          /* Something in the next few lines breaks win32.... */
          send_to_desc ( d, TXT_RCFROM );

          for ( pcrace = 1; pcrace < MAX_PCRACE ; pcrace++ )
          {
               SNP (buf, TXT_RCTEMP, pc_race_table[pcrace].name );
               send_to_desc ( d, buf );
          }
          send_to_desc ( d, TXT_RCPRMP );
          d->connected = CON_GET_NEW_RACE;
          break;

     case CON_GET_NEW_RACE:
          one_argument ( argument, arg );

          if ( !strcmp ( arg, "help" ) )
          {
               argument = one_argument ( argument, arg );
               if ( argument[0] == '\0' )
                    do_help ( ch, "race help" );
               else
                    do_help ( ch, argument );
               send_to_desc ( d, TXT_RCPRMP );
               break;
          }

          pcrace = pcrace_lookup ( argument );
          race = race_lookup ( argument );

          if ( pcrace == 0 || race == 0 )
          {
               send_to_desc ( d, "That is not a valid race.\n\r\n\r" TXT_RCFROM );
               for ( pcrace = 1; pcrace < MAX_PCRACE; pcrace++ )
               {
                    SNP (buf, TXT_RCTEMP, pc_race_table[pcrace].name );
                    send_to_desc ( d, buf );
               }
               send_to_desc ( d, TXT_RCPRMP );
               break;
          }

          send_to_desc ( d, VT_CLS );

          ch->race = race;
          ch->pcdata->pcrace = pcrace;

          ch->affected_by = ch->affected_by | race_table[race].aff;
          ch->detections = ch->detections | race_table[race].detect;
          ch->imm_flags = ch->imm_flags | race_table[race].imm;
          ch->res_flags = ch->res_flags | race_table[race].res;
          ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
          ch->form = race_table[race].form;
          ch->parts = race_table[race].parts;
          ch->speaking = str_dup ( race_table[race].name );

		  /* add skills */
          for ( i = 0; i < 5; i++ )
          {
               int                 sn;
               if ( pc_race_table[pcrace].skills[i] == NULL )
                    break;
               sn = skill_lookup ( pc_race_table[pcrace].skills[i] );
               if ( sn <= 0 )
                    break;
               ch->pcdata->learned[sn] = 60;
          }

		  /* 4/2/99 Zeran - set racial language skill to 100% knowledge */
          {
               char                buf[128];
               SNP ( buf, "%s tongue", ch->speaking );
               ch->pcdata->learned[skill_lookup ( buf )] = 100;
          }
		  /* add cost */
          ch->pcdata->points = pc_race_table[pcrace].points;
          ch->size = pc_race_table[pcrace].size;

          send_to_desc ( d, "The voice whispers to you again, reminding you of the {rpassions{w of flesh life, and\n\r"
                         "of things which you had long forgotten.\n\r\n\r"
                         "You must now choose sides in the most {Wimportant battle of all time{w.\n\r" );

          send_to_desc ( d, "What is your gender {W({GM{W/{GF{W){w? " );
          d->connected = CON_GET_NEW_SEX;
          break;

     case CON_GET_NEW_SEX:
          switch ( argument[0] )
          {
          case 'm':
          case 'M':
               ch->sex = SEX_MALE;
               ch->pcdata->true_sex = SEX_MALE;
               break;
          case 'f':
          case 'F':
               ch->sex = SEX_FEMALE;
               ch->pcdata->true_sex = SEX_FEMALE;
               break;
          default:
               send_to_desc ( d, "That's not a sex.\n\rWhat IS your gender {W({GM{W/{GF{W){w? " );
               return;
          }

          send_to_desc ( d, VT_CLS );

          send_to_desc ( d, "Wisely chosen.\n\r"
                         "The voices leaves you, and you are born into the world. You forget all about\n\r"
                         "this experience in your youth, but when you come of age the seed planted in your\n\r"
                         "mind awakens, and you feel the {Curge to adventure{w.\n\r\n\r" );
          send_to_desc ( d, "The following guilds vied for your studies:\n\r" );

          send_to_desc ( d, "Class: " );

          for ( iClass = 0; iClass < 7; iClass++ )	/* increase if adding a class */
          {
               SNP ( buf, TXT_RCTEMP, class_table[iClass].name);
               send_to_desc (d, buf );
          }
          send_to_desc (d, TXT_CLASS );
          d->connected = CON_GET_NEW_CLASS;
          break;

     case CON_GET_NEW_CLASS:
          one_argument ( argument, arg );

          if ( !strcmp ( arg, "help" ) )
          {
               write_to_buffer (d, "\n\r", 0);
               argument = one_argument ( argument, arg );
               if ( argument[0] == '\0' )
                    do_help ( ch, "class help" );
               else
                    do_help ( ch, argument );
               send_to_desc ( d, TXT_CLASS );
               break;
          }

          iClass = class_lookup ( argument );

          if ( iClass == -1 )
          {
               send_to_desc ( d, "I'm sorry, I didn't get that.\n\r" TXT_CLASS );
               return;
          }

          ch->pcdata->pclass = iClass;


          {
               char lbuf[MSL];
               SNP ( lbuf, "%s@%s new player.", ch->name, d->host );
               notify_message ( ch, WIZNET_SITES, TO_IMM, lbuf );
               log_string ( lbuf );
          }

          send_to_desc ( d, VT_CLS );

          send_to_desc ( d, "\n\r  What motivates you - what morals do you have?\n\r"
                         "{W({YG{W) {wYou are motivated by the greater needs of mankind.\n\r"
                         "{W({CN{W) {wYou believe that everything must exist in a balance of good versus evil.\n\r"
                         "{W({RE{W) {wYou don't care about all that mumbo-jumbo.\n\r" );
          send_to_desc ( d, "\n\rWhich are you {W({YG{W/{CN{W/{RE{W){w? : " );

          d->connected = CON_GET_ALIGNMENT;
          break;

     case CON_GET_ALIGNMENT:
          switch ( argument[0] )
          {
          case 'g':
          case 'G':
               ch->alignment = 750;
               break;
          case 'n':
          case 'N':
               ch->alignment = 0;
               break;
          case 'e':
          case 'E':
               ch->alignment = -750;
               break;
          default:
               send_to_desc ( d, "I'm not quite sure I understood you.\n\r" );
               send_to_desc ( d, "\n\rWhich are you {W({YG{W/{CN{W/{RE{W){w? : " );
               return;
          }

          send_to_desc ( d, VT_CLS );

		  /* Just stuck in here somewhere odd... heh. */
          ch->pcdata->learned[gsn_recall] = 50;

          send_to_desc ( d, "\n\r{GStat Selection System{w\n\r" );

          send_to_desc ( d, "\n\r   " TXT_MUDNAME " allows you to allocate your stats wherever you want them. You have\n\r"
                         "5 stats - Strength, Intelligence, Wisdom, Dexterity and Constitution, and each of these is \n\r"
                         "abbreviated to its first 3 letters, {WSTR{w, {WINT{w, {WWIS{w, {WDEX{w, and {WCON{w.\n\r\n\r"
                         "{WStrength{w    : Determines your overall muscular strength for physical tasks.\n\r"
                         "{WIntelligence{w: Determines your intellect, in the sense of IQ.\n\r"
                         "{WWisdom{w      : This is your insight and common sense, regardless of book smarts.\n\r"
                         "{WDexterity{w   : Jumping, dodging, swiftness. This is how easily you move.\n\r"
                         "{WConstitution{w: General physical health, resistance to illness and disease.\n\r"
                         "\n\rEach of these stats determines how well you can carry out your skills, as well as\n\r"
                         "your general effectiveness in both physical and academic pursuits.\n\r\n\r"
                         "You will have 30 points to allocate between these five stats. Depending upon your race, you\n\r"
                         "may already have bonuses or penalties to your starting amounts and maximums. What you assign\n\r"
                         "now will remain with you for the rest of your adventuring career.\n\r" );
          if ( !ch->pcdata->mortal )
               send_to_desc ( d, "\n\rSince you are a Demi-God, you get an extra 5 points!\n\r" );
          send_to_desc ( d, "\n\rHit {W[{GENTER{W]{w to begin assigning stats." );
          pcrace = ch->pcdata->pcrace;
          for ( i = 0; i < MAX_STATS; i++ )
          {
               ch->perm_stat[i] = pc_race_table[pcrace].stats[i];
               ch->perm_stat[i] += 10;
          }
          if (ch->pcdata->mortal)   // Careful to never give out too many points here.
               ch->pcdata->train = 30;      // If a player MAXes all his/her stats and has points left
          else                      // he will be stuck in creation. We don't plan for this.
               ch->pcdata->train = 35;
          d->connected = CON_ROLL_STATS;
          break;

     case CON_ROLL_STATS:
          pcrace = ch->pcdata->pcrace;

#if defined(DEBUGINFO)
          log_string ( "DEBUG: CON_ROLL_STATS" );
#endif

          switch ( tolower(argument[0]) )
          {
          default:
               buf[0] = '\0';
               send_to_desc ( d, VT_CLS );
               STVIEW;   /* Macro to view stats. */
               send_to_desc ( d, "\n\rAssign points to stats by typing the first letter of each stat,\n\r"
                              "or Reset all stats, or F when Finished.\n\r\n\r " );
               send_to_desc ( d, TXT_STATPRM );

               break;
          case 's':
               if (ch->pcdata->train < 1)
               {
                    send_to_desc ( d, TXT_STATNOP );
               }
               else if (ch->perm_stat[STAT_STR] >= pc_race_table[pcrace].max_stats[STAT_STR] )
               {
                    send_to_desc ( d, TXT_STATMAX );
               }
               else
               {
                    ch->perm_stat[STAT_STR]++;
                    ch->pcdata->train--;
                    send_to_desc ( d, VT_CLS );
                    STVIEW;
               }
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'i':
               if (ch->pcdata->train < 1)
               {
                    send_to_desc ( d, TXT_STATNOP );
               }
               else if (ch->perm_stat[STAT_INT] >= pc_race_table[pcrace].max_stats[STAT_INT] )
               {
                    send_to_desc ( d, TXT_STATMAX );
               }
               else
               {
                    ch->perm_stat[STAT_INT]++;
                    ch->pcdata->train--;
                    send_to_desc ( d, VT_CLS );
                    STVIEW;
               }
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'w':
               if (ch->pcdata->train < 1)
               {
                    send_to_desc ( d, TXT_STATNOP );
               }
               else if (ch->perm_stat[STAT_WIS] >= pc_race_table[pcrace].max_stats[STAT_WIS] )
               {
                    send_to_desc ( d, TXT_STATMAX );
               }
               else
               {
                    ch->perm_stat[STAT_WIS]++;
                    ch->pcdata->train--;
                    send_to_desc ( d, VT_CLS );
                    STVIEW;
               }
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'd':
               if (ch->pcdata->train < 1)
               {
                    send_to_desc ( d, TXT_STATNOP );
               }
               else if (ch->perm_stat[STAT_DEX] >= pc_race_table[pcrace].max_stats[STAT_DEX] )
               {
                    send_to_desc ( d, TXT_STATMAX );
               }
               else
               {
                    ch->perm_stat[STAT_DEX]++;
                    ch->pcdata->train--;
                    send_to_desc ( d, VT_CLS );
                    STVIEW;
               }
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'c':
               if (ch->pcdata->train < 1)
               {
                    send_to_desc ( d, TXT_STATNOP );
               }
               else if (ch->perm_stat[STAT_CON] >= pc_race_table[pcrace].max_stats[STAT_CON] )
               {
                    send_to_desc ( d, TXT_STATMAX );
               }
               else
               {
                    ch->perm_stat[STAT_CON]++;
                    ch->pcdata->train--;
                    send_to_desc ( d, VT_CLS );
                    STVIEW;
               }
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'r':
               send_to_desc ( d, VT_CLS );
               write_to_buffer ( d, "Starting Over....\n\r\n\r", 0);
               pcrace = ch->pcdata->pcrace;
               for ( i = 0; i < MAX_STATS; i++ )
               {
                    ch->perm_stat[i] = pc_race_table[pcrace].stats[i];
                    ch->perm_stat[i] += 10;
               }
               ch->pcdata->train = 30;
               STVIEW;
               send_to_desc ( d, TXT_STATPRM );
               break;
          case 'f':
               if (ch->pcdata->train > 0)
               {
                    send_to_desc ( d, "You still have points left to spend on stats!\n\r" );
                    send_to_desc ( d, TXT_STATPRM );
                    break;
               }
               send_to_desc ( d, VT_CLS );
               do_help ( ch, "nobb introcredit" ); // DO NOT REMOVE THIS!!! You may CHANGE THE COLOURS OR APPEARANCE of the helpfile,
               // BUT YOU MAY NOT REMOVE THE CREDITS CONTENT!!!! (Yes, this means you. Read your licenses.)
               do_help ( ch, "nobb motd" );
               do_help ( ch, "nobb newbie" );
               send_to_char( TXT_CONTINUE, ch );

               /* Zeran - lets put this here, new guys are ready to play */
               notify_message ( ch, WIZNET_NEWBIE, TO_IMM, NULL );
               /* set some defaults */

               SET_BIT ( ch->comm, COMM_COMBINE );
               // SET_BIT ( ch->comm, COMM_NOFLASHY );              
               SET_BIT ( ch->act, PLR_AUTOEXIT );
               SET_BIT ( ch->act, PLR_AUTOGOLD );
               SET_BIT ( ch->act, PLR_AUTOLOOT );
               SET_BIT ( ch->act, PLR_AUTOSAC );
               SET_BIT ( ch->act, PLR_AUTOSAVE );
               if ( d->ansi == TRUE )
                    SET_BIT ( ch->comm, COMM_COLOUR );
               d->connected = CON_READ_MOTD;
               break;
          }
          break;

     case CON_READ_IMOTD:
          send_to_char ( "\n\r", ch );
          do_help ( ch, "nobb motd" );
          
          send_to_char( TXT_CONTINUE, ch );
          d->connected = CON_READ_MOTD;
          break;

     case CON_CHOOSE_TERM:
          write_to_buffer ( d, "\n\r", 2 );
          if ( !strncmp (argument, "PUEBLOCLIENT", 12 ) )
          {
               if ( !d->mxp ) // ZMud's pueblo emulation gets confused. Pretend therefore to zmud that we don't do Pueblo.
               {
                    d->ansi = TRUE;
                    d->pueblo = TRUE;
                    write_to_buffer( d, "Pueblo Enabled\n\r", 0 );
               }
          }
         else if ( !strncmp ( argument, "#PUEBLO", 7 ) )
         {
             /*
              * ZMud just up and started sending #PUEBLO return to the client challenge one day,
              * quite unexpectedly (not even at a version change) which totally screwed up client
              * negotiation. So we're going to assume by this response that it's ZMud, and properly
              * ignore it.
              * You'd figure if zMud was gonna do Pueblo it would send the proper PUEBLOCLIENT string
              * which is detected above. Blehfuck. It was better when zmud DIDN'T implement pueblo.
              */
             d->ansi = TRUE;
             write_to_buffer ( d, "\n\r", 2 );
             
         }         
          else
          {
               switch ( argument[0] )
               {
               case 'y':
               case 'Y':
                    d->ansi = TRUE;
                    write_to_buffer ( d, "\n\r", 2 );
                    break;
               case 'n':
               case 'N':
                    d->ansi = FALSE;
                    write_to_buffer ( d, "\n\r", 2 );
               break;
               default:
                    write_to_buffer ( d, "Please answer (Y/N)? ", 0);
                    return;
               }
          }
		  /*
		   * Send the greeting.
		   */
          {
               if ( d->pueblo || d->mxp )
               {
                    extern char *help_pueblo_greeting;
                    
                    send_to_desc ( d, "\n\r" );
                    tag_center ( d, TRUE, FALSE );
                    inline_image ( d, "sund-lnxpwr.gif", "middle", FALSE );
                    send_to_desc ( d, "\n\r" );
                    tag_center ( d, FALSE, FALSE );
                    send_to_desc ( d, "\n\r" );
                    if ( help_pueblo_greeting[0] == '.' )
                         send_to_desc ( d, help_pueblo_greeting +1 );
                    else
                         send_to_desc ( d, help_pueblo_greeting );
               }
               else
               {
                    extern char *help_greeting;
                    
                    send_to_desc ( d, "\n\r" );
                    
                    if ( help_greeting[0] == '.' )
                         send_to_desc ( d, help_greeting + 1 );
                    else
                         send_to_desc ( d, help_greeting );
               }
          }
          
          d->connected = CON_GET_NAME;
          break;

     case CON_READ_MOTD:
          // This way, the pfile will save the info for copyover.
          if ( d->pueblo == TRUE )
          {
               SET_BIT      ( ch->comm, COMM_COLOUR );
               SET_BIT     ( ch->comm, COMM_PUEBLO );
          }
          else
               REMOVE_BIT ( ch->comm, COMM_PUEBLO );
          
          if ( d->mxp == TRUE )
               SET_BIT ( ch->comm, COMM_MXP );
          else
               REMOVE_BIT ( ch->comm, COMM_MXP );

          if ( ch->pcdata->account->password[0] == '\0' )
          {
               write_to_buffer ( d, "Warning! Null Account Password!\n\rEnter New Master Password: ", 0 );
               bugf ( "Null Account Password Encountered! (%s)", ch->name );
               d->connected = CON_ACCOUNT_PW_FIX;
               break;
          }

#if !defined (NOZLIB)
          if (ch->desc->out_compress)
               send_to_char("{wMCCP Compression ACTIVE! Thank you for saving us bandwidth.\n\r", ch);
#endif

          send_to_char("\n\rWelcome to {W" TXT_MUDNAME "{w. Please do not romance the mobiles.\n\r", ch);

          send_to_char("Current Game Mode: ", ch );

          switch (mud.mudxp)
          {
          case XP_EASIEST:
               send_to_char ("Easiest with ", ch );
               break;
          case XP_EASY:
               send_to_char ("Easy with ", ch );
               break;
          case XP_NORMAL:
               send_to_char ("Normal with ", ch );
               break;
          case XP_HARD:
               send_to_char ("Hard with ", ch );
               break;
          case XP_VERYHARD:
               send_to_char ("Very Hard with ", ch );
               break;
          case XP_NIGHTMARE:
               send_to_char ("Nightmare with ", ch );
               break;
          default:
               send_to_char ("Unknown with ", ch );
               break;
          }

          switch (mud.death)
          {
          case PERMADEATH:
               send_to_char ("PermaDeath and Full Aging.\n\r", ch );
               break;
          case FULLAGING:
               send_to_char ("Aging Enabled.\n\r", ch );
               break;
          case PARTAGING:
               send_to_char ("Partial Aging.\n\r", ch );
               break;
          case NOAGING:
               send_to_char ("No Aging.\n\r", ch );
               break;
          default:
               send_to_char ("Unknown death rules.\n\r", ch );
               break;
          }

#if defined(DEBUGINFO)
          if ( IS_IMMORTAL ( ch ) )
               write_to_buffer ( d, "\n\r>>> !DEBUG MODE ON! <<<\n\r", 0 );
          log_string ( "DEBUG: CON_READ_MOTD" );
#endif

          ch->next = char_list;
          char_list = ch;
          d->connected = CON_PLAYING;
          ch->pcdata->mode = MODE_NORMAL;
          reset_char ( ch );

          if ( ch->level == 0 )
          {
               ch->perm_stat[class_table[ch->pcdata->pclass].attr_prime] += 2;
               ch->level = 1;
               ch->exp = exp_per_level ( ch, ch->pcdata->points );
               ch->hit = ch->max_hit;
               ch->mana = ch->max_mana;
               ch->move = ch->max_move;
               ch->pcdata->train = 0;
               ch->pcdata->practice = 5;
               ch->pcdata->notify = NOTIFY_ALL;
               SNP ( buf, "the %s", ch->sex == SEX_FEMALE ? "woman" : "man" );
               set_title ( ch, buf );

			   /* Add to Account */
               for (i = 0 ; i < MAX_CHARS ; i++)
               {
                    if (!ch->pcdata->account->char_name[i] )
                    {
                         slot_found = TRUE;
                         break;
                    }
               }

               if (!slot_found)	/* Woops */
               {
                    send_to_char ("\n\rSomehow, your account has reached capacity while you were making\n\r", ch);
                    send_to_char ("your character. Sorry, cancelling creation.\n\r\n\rDisconnecting.\n\r", ch);
                    log_string ( "ACC %s got through creation with too many players!\n\r", ch->pcdata->account->acc_name );
                    close_socket ( d );
                    return;
               }

               ch->pcdata->account->char_name[i] = str_dup ( ch->name );

               if (!ch->pcdata->mortal)
                    ++ch->pcdata->account->demigods;

               if ( !mud.verify ) // Verification not required...
                    ch->pcdata->account->status = ACCT_VERIFIED;

               if (ch->pcdata->account->status < ACCT_UNVERIFIED)
                    ch->pcdata->account->status = ACCT_UNVERIFIED;

			   /* Need to save account data here */

               fwrite_accounts();

			   /* Let's give the character some money to start with */
			   /* This will be burned for the outfitting cost. */

               ch->gold = 25;

               do_outfit ( ch, "" );
               ch->pcdata->learned[get_weapon_sn ( ch, FALSE )] = 60;
               ch->recall_perm = pc_race_table[ch->pcdata->pcrace].recall;
               char_to_room ( ch, get_room_index ( ch->recall_perm ) );

               save_char_obj ( ch );

               send_to_char ( "\n\r", ch );
               send_to_char ( "{YBe sure to read {WHELP RULES{Y!{w\n\r", ch );
               send_to_char ( "\n\r", ch );
          }
          else if ( ch->in_room != NULL )
          {
               char_to_room ( ch, ch->in_room );
          }
          else
          {
               char_to_room ( ch, get_room_index ( ch->recall_perm ) );
          }

          act ( "$n has appeared from nowhere.", ch, NULL, NULL, TO_ROOM );

		  /* Zeran - notify message */
          notify_message ( ch, NOTIFY_LOGIN, TO_ALL, NULL );

          
          
          do_board ( ch, "auto" );	/* Show board status */
          do_count( ch, "" );
          do_look ( ch, "auto" );

          if ( ch->pet != NULL )
          {
               char_to_room ( ch->pet, ch->in_room );
               act ( "$n has entered the game.", ch->pet, NULL, NULL, TO_ROOM );
          }
          break;

     case CON_EDIT_CLAN:
          handle_edit_clan ( d, argument );
          break;
		  /* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
		  /* ch MUST be PC here; have nwrite check for PC status! */
     case CON_NOTE_TO:
          handle_con_note_to ( d, argument );
          break;

     case CON_NOTE_SUBJECT:
          handle_con_note_subject ( d, argument );
          break;			/* subject */

     case CON_NOTE_EXPIRE:
          handle_con_note_expire ( d, argument );
          break;

     case CON_NOTE_TEXT:
          handle_con_note_text ( d, argument );
          break;

     case CON_NOTE_FINISH:
          handle_con_note_finish ( d, argument );
          break;

     }

     return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name ( char *name )
{

     char                buf[MAX_STRING_LENGTH];

#if defined(DEBUGINFO)
     log_string ( "DEBUG: bool check_parse_name" );
#endif

	 /*
	  * Reserved words.
	  */
     buf[0] = '\0';


#if defined(DEBUGINFO)
     log_string ( "DEBUG: after the grep command" );
#endif

     if ( strlen ( name ) < 3 )
          return FALSE;

     if ( strlen ( name ) > 12 )
          return FALSE;
	 /*
	  * Alphanumerics only.
	  * Lock out IllIll twits.
	  */
     {
          char               *pc;
          bool                fIll;

          fIll = TRUE;
          for ( pc = name; *pc != '\0'; pc++ )
          {
               if ( !isalpha ( *pc ) )
                    return FALSE;
               if ( LOWER ( *pc ) != 'i' && LOWER ( *pc ) != 'l' )
                    fIll = FALSE;
          }

          if ( fIll )
               return FALSE;
     }

     // Doing this last now just in case someone has conceived of a string that can
     // screw with the commandline and do wierd stuff, it'll get filtered out by the
     // alphanumeric filter above.

#if !defined (WIN32)
     SNP ( buf, "grep -i -x %s bad.names > /dev/null", name );

     /* Zeran - hey Loth, whats with the nonstandard argument order here?
      * if(0==system(buf))
      * Loth - Hey Zeran... I felt like it? :P It worked didn't it?
      */

     if ( system ( buf ) == 0 )
     {
          buf[0] = '\0';
          return FALSE;
     }
#endif
     
     return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect ( DESCRIPTOR_DATA * d, char *name, bool fConn )
{
     CHAR_DATA          *ch;

#if defined(DEBUGINFO)
     log_string ( "DEBUG: bool check_reconnect: begin" );
#endif

     for ( ch = char_list; ch != NULL; ch = ch->next )
     {
          if ( !IS_NPC ( ch )
               && ( !fConn || ch->desc == NULL )
               && !str_cmp ( d->character->name, ch->name ) )
          {
               if ( fConn == FALSE )
               {
                    free_string ( d->character->pcdata->pwd );
                    d->character->pcdata->pwd =
                         str_dup ( ch->pcdata->pwd );
               }
               else
               {
					/* Zeran - hack to fix lighting problem of reconnecting
					 * characters */
                    char		lbuf[MSL];
                    OBJ_DATA           *obj;

                    if ( ( obj = get_eq_char ( d->character, WEAR_LIGHT ) ) != NULL
                         && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
                         d->character->in_room->light++;
					/* Zeran - set old char_data as quitting to prevent object reset
					 * screwups */
                    d->character->quitting = TRUE;
                    free_char ( d->character );
                    d->character = ch;
                    ch->desc = d;
                    ch->timer = 0;
                    send_to_char ( "Reconnecting.\n\r", ch );
                    act ( "$n eyes lose their glazed appearance.", ch,
                          NULL, NULL, TO_ROOM );
					/* Zeran - notify message */
                    notify_message ( ch, NOTIFY_RECONNECT, TO_CLAN, NULL );
                    SNP ( lbuf, "%s@%s reconnected.", ch->name, d->host );
                    notify_message ( ch, WIZNET_LINK, TO_IMM, lbuf );
                    log_string ( lbuf );
                    d->connected = CON_PLAYING;
					/* Inform the character of a note in progress and the
					 * possbility of continuation! */
                    if ( ch->pcdata->in_progress )
                         send_to_char ( "You have a note in progress. Type {Wnote write{w to continue it.\n\r", ch );
               }
               return TRUE;
          }
     }

     return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing ( DESCRIPTOR_DATA * d, char *name )
{
     DESCRIPTOR_DATA    *dold;

     for ( dold = descriptor_list; dold; dold = dold->next )
     {
          if ( dold != d
               && dold->character != NULL
               && dold->connected != CON_GET_NAME
               && dold->connected != CON_GET_OLD_PASSWORD
               && !str_cmp ( name, dold->original
                             ? dold->original->name : dold->
                             character->name ) )
          {
               send_to_desc ( d, "That character is already playing.\n\r"
                              "Do you wish to connect anyway (Y/N)?" );
               d->connected = CON_BREAK_CONNECT;
               return TRUE;
          }
     }
     return FALSE;
}

void stop_idling ( CHAR_DATA * ch )
{
     if ( !ch
          || !ch->desc
          || ch->desc->connected != CON_PLAYING
          || !ch->was_in_room
          || ch->in_room != get_room_index ( ROOM_VNUM_LIMBO ) )
          return;

     ch->timer = 0;
     char_from_room ( ch );
     char_to_room ( ch, ch->was_in_room );
     ch->was_in_room = NULL;
     act ( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
     return;
}

/*
 * Format text to char.
 */
void form_to_char ( CHAR_DATA *ch, char *fmt, ... )
{
     char buf[MSL*4];     
     va_list args;
     va_start (args, fmt);
     vsnprintf (buf, sizeof(buf)-1, fmt, args);
     va_end (args);     
     send_to_char ( buf, ch );
     return;
}

/*
 * Write to one char.
 */
void send_to_char_bw ( const char *txt, CHAR_DATA * ch )
{
     if ( txt != NULL && ch->desc != NULL )
          write_to_buffer ( ch->desc, txt, strlen ( txt ) );
     return;
}

/* Write to a descriptor, avoids accessing ch */

void send_to_desc ( DESCRIPTOR_DATA *d, const char *txt )
{
     const char		    *point;
     char		        *point2;
     char                buf[MAX_STRING_LENGTH * 4];
     int                 skip = 0;

     buf[0] = '\0';
     point2 = buf;
     if ( txt && d )
     {
          if ( d->ansi || d->mxp )
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         skip = colour ( *point, NULL, point2 );
                         while ( skip-- > 0 )
                              ++point2;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               write_to_buffer ( d, buf, point2 - buf );
          }
          else
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               write_to_buffer ( d, buf, point2 - buf );
          }
     }
     return;
}

/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char ( const char *txt, CHAR_DATA * ch )
{
     const char         *point;
     char               *point2;
     char                buf[MAX_STRING_LENGTH * 4];
     int                 skip = 0;

     buf[0] = '\0';
     point2 = buf;
     if ( txt && ch->desc )
     {
          if ( ch->desc->ansi || ch->desc->mxp )
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         skip = colour ( *point, ch, point2 );
                         while ( skip-- > 0 )
                              ++point2;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               write_to_buffer ( ch->desc, buf, point2 - buf );
          }
          else
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               write_to_buffer ( ch->desc, buf, point2 - buf );
          }
     }
     return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw ( const char *txt, CHAR_DATA * ch )
{
     if ( txt == NULL || ch->desc == NULL )
          return;

     if ( strlen ( txt ) > ( MAX_STRING_LENGTH * 4 ) )
     {
          bugf ( "page_to_char: Overflow, abort." );
          send_to_char ( "{&{RERROR:{x {RError. Page_to_char overflow: Report to imms!{x", ch );
          return;
     }

     ch->desc->showstr_head = alloc_mem ( strlen ( txt ) + 1, "showstr_head" );
     strcpy ( ch->desc->showstr_head, txt );
     ch->desc->showstr_point = ch->desc->showstr_head;
     show_string ( ch->desc, "" );
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char ( const char *txt, CHAR_DATA * ch )
{
     const char         *point;
     char               *point2;
     char                buf[MAX_STRING_LENGTH * 8];
     int                 skip = 0;

     buf[0] = '\0';
     point2 = buf;

     if ( strlen ( txt ) > ( MAX_STRING_LENGTH * 7 ) )
     {
          bugf ( "page_to_char: Overflow, abort." );
          send_to_char ( "{&{RERROR:{x {RError. Page_to_char overflow: Report to imms!{x", ch );
          return;
     }

     if ( txt && ch->desc )
     {
          if ( ch->desc->ansi || ch->desc->mxp )
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         skip = colour ( *point, ch, point2 );
                         while ( skip-- > 0 )
                              ++point2;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               free_string ( ch->desc->showstr_head );
               ch->desc->showstr_head = str_dup ( buf );
               ch->desc->showstr_point = ch->desc->showstr_head;
               show_string ( ch->desc, "" );
          }
          else
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         continue;
                    }
                    *point2 = *point;
                    *++point2 = '\0';
               }
               *point2 = '\0';
               free_string ( ch->desc->showstr_head );
               ch->desc->showstr_head = str_dup ( buf );
               ch->desc->showstr_point = ch->desc->showstr_head;
               show_string ( ch->desc, "" );
          }
     }
     return;
}

/* string pager */
void show_string ( struct descriptor_data *d, char *input )
{
     char                buffer[4 * MAX_STRING_LENGTH];
     char                buf[MAX_INPUT_LENGTH];
     register char      *scan, *chk;
     int                 lines = 0, toggle = 1;
     int                 show_lines;

     one_argument ( input, buf );
     if ( buf[0] != '\0' )
     {
          if ( d->showstr_head )
          {
               free_string ( d->showstr_head );
               d->showstr_head = 0;
          }
          d->showstr_point = 0;
          return;
     }

     if ( d->character )
          show_lines = d->character->lines;
     else
          show_lines = 0;

     for ( scan = buffer;; scan++, d->showstr_point++ )
     {
          if ( ( ( *scan = *d->showstr_point ) == '\n' ||
                 *scan == '\r' ) && ( toggle = -toggle ) < 0 )
               lines++;

          else if ( !*scan ||
                    ( show_lines > 0 && lines >= show_lines ) )
          {
               *scan = '\0';
               write_to_buffer ( d, buffer, strlen ( buffer ) );
               for ( chk = d->showstr_point; isspace ( *chk );
                     chk++ );
               {
                    if ( !*chk )
                    {
                         if ( d->showstr_head )
                         {
                              free_string ( d->showstr_head );
                              d->showstr_head = 0;
                         }
                         d->showstr_point = 0;
                    }
               }
               return;
          }
     }
     return;
}

/* Cursor Manipulation Commands
 * I need to find a good use for this.
 * args: ( ch, line , column ) */

void gotoxy ( CHAR_DATA * ch, int arg1, int arg2 )
{
     if ( !IS_SET ( ch->act, PLR_CURSOR ) )
     {
          bugf ( "Error! GotoXY called when PLR_CURSOR was turned off!" );
          return;
     }
     form_to_char ( ch, "\033[%d;%df", arg1, arg2 );
}

/* quick sex fixer */
void fix_sex ( CHAR_DATA * ch )
{
     if ( ch->sex < 0 || ch->sex > 2 )
          ch->sex = IS_NPC ( ch ) ? 0 : ch->pcdata->true_sex;
}

void act ( const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type )
{
	 /* to be compatible with older code */
     act_new ( format, ch, arg1, arg2, type, POS_RESTING );
}

/*
 * The colour version of the act( ) function, -Lope
 */
void act_new ( const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type, int min_pos )
{
     static char        *const he_she[] = { "it", "he", "she" };
     static char        *const him_her[] = { "it", "him", "her" };
     static char        *const his_her[] = { "its", "his", "her" };

     CHAR_DATA          *to;
     CHAR_DATA          *vch = ( CHAR_DATA * ) arg2;     // VCH is set here.
     OBJ_DATA           *obj1 = ( OBJ_DATA * ) arg1;     // obj1 is set here
     OBJ_DATA           *obj2 = ( OBJ_DATA * ) arg2;     // and obj2 here.
     const char         *str;
     char               *i;
     char               *point;
     char               *pbuff;
     char                buf[MAX_STRING_LENGTH];
     char                buffer[MAX_STRING_LENGTH * 2];
     char                fname[MAX_INPUT_LENGTH];
     bool                fColour = FALSE;

     if ( !format || !*format )
          return;

     if ( !ch || !ch->in_room )
          return;

     to = ch->in_room->people;
     if ( type == TO_VICT )
     {
          if ( !vch )
          {
               bugf ( "Act: null vch with TO_VICT." );
               return;
          }

          if ( !vch->in_room )
               return;

          to = vch->in_room->people;
     }
     
     for ( ; to; to = to->next_in_room )
     {
          if ( (!IS_NPC(to) && !to->desc )
               ||   ( IS_NPC(to) && !HAS_TRIGGER_MOB(to, TRIG_ACT) )
               ||    to->position < min_pos )
               continue;

          if ( type == TO_CHAR && to != ch )
               continue;
          if ( type == TO_VICT && ( to != vch || to == ch ) )
               continue;
          if ( type == TO_ROOM && to == ch )
               continue;
          if ( type == TO_NOTVICT && ( to == ch || to == vch ) )
               continue;
          
          point = buf;
          str = format;
          while ( *str )
          {
               if ( *str != '$' )
               {
                    *point++ = *str++;
                    continue;
               }

               i = NULL;
               switch ( *str )
               {
               case '$':
                    fColour = TRUE;
                    ++str;
                    i = " <@@@> ";
                    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
                    {
                         bugf ( "Act: missing arg2 for code %d.", *str );
                         i = " <@@@> ";
                    }
                    else
                    {
                         switch ( *str )
                         {
                         default:
                              bugf ( "Act: bad code %d.", *str );
                              i = " <@@@> ";
                              break;

                         case 't':
                              i = ( char * ) arg1;
                              break;

                         case 'T':
                              i = ( char * ) arg2;
                              break;

                         case 'n':
                              if ( type == TO_ROOM )	/*check use PERSMASK */
                              {
                                   i = PERSMASK ( ch, to );
                                   break;
                              }
                              i = PERS ( ch, to );
                              break;

                         case 'N':
                              if ( type == TO_ROOM )	/*check use PERSMASK */
                              {
                                   i = PERSMASK ( vch, to );
                                   break;
                              }
                              i = PERS ( vch, to );
                              break;

                         case 'e':
                              i = he_she[URANGE ( 0, ch->sex, 2 )];
                              break;

                         case 'E':
                              i = he_she[URANGE ( 0, vch->sex, 2 )];
                              break;

                         case 'm':
                              i = him_her[URANGE ( 0, ch->sex, 2 )];
                              break;

                         case 'M':
                              i = him_her[URANGE ( 0, vch->sex, 2 )];
                              break;

                         case 's':
                              i = his_her[URANGE ( 0, ch->sex, 2 )];
                              break;

                         case 'S':
                              i = his_her[URANGE ( 0, vch->sex, 2 )];
                              break;

                         case 'p':
                              i = can_see_obj ( to, obj1 )
                                   ? obj1->short_descr : "something";
                              break;

                         case 'P':
                              i = can_see_obj ( to, obj2 )
                                   ? obj2->short_descr : "something";
                              break;

                         case 'd':
                              if ( !arg2 ||
                                   ( ( char * ) arg2 )[0] == '\0' )
                              {
                                   i = "door";
                              }
                              else
                              {
                                   one_argument ( ( char * ) arg2,
                                                  fname );
                                   i = fname;
                              }
                              break;
                             /* Convert $$ into $ ... needed for I3 */
                         case '$':
                             i = "$";
                             break;
                             
                         }
                    }
                    break;

               default:
                    fColour = FALSE;
                    *point++ = *str++;
                    break;
               }

               ++str;
               if ( i )
               {
                    while ( ( *point = *i ) != '\0' )
                    {
                         ++point;
                         ++i;
                    }
               }
          }

          *point++ = '\n';
          *point++ = '\r';
          *point = '\0';
          buf[0] = UPPER ( buf[0] );
          pbuff = buffer;
          colourconv ( pbuff, buf, to );
          if ( to->desc && ( to->desc->connected == CON_PLAYING ) )
               write_to_buffer ( to->desc, buffer, 0 );
          else 
               if ( IS_NPC(to) && MOBtrigger && HAS_TRIGGER_MOB(to, TRIG_ACT) )
                    p_act_trigger( buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT );

     }
     if ( type == TO_ROOM || type == TO_NOTVICT )
     {
          OBJ_DATA *obj, *obj_next;
          CHAR_DATA *tch, *tch_next;
          
          point   = buf;
          str     = format;
          while( *str != '\0' )
          {
               *point++ = *str++;
          }
          *point   = '\0';
          
          for( obj = ch->in_room->contents; obj; obj = obj_next )
          {
               obj_next = obj->next_content;
               if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
                    p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
          }
          
          for( tch = ch; tch; tch = tch_next )
          {
               tch_next = tch->next_in_room;
               
               for ( obj = tch->carrying; obj; obj = obj_next )
               {
                    obj_next = obj->next_content;
                    if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
                         p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
               }
          }
          
          if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
               p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
     }     
     return;
}

// From Lope's colour code (older version )
// Modified by Lotherius to allow some minor configuration

int colour ( char type, CHAR_DATA * ch, char *string )
{
     char                code[20];
     char               *p = '\0';
     bool		 noflashy = FALSE;
     bool	         nodgrey  = FALSE;
     bool	         beep     = FALSE;

     if ( ch )	/* Things to do if we *HAVE* a character. */
     {
          if ( IS_NPC ( ch ) )
               return ( 0 );

          if ( IS_SET ( ch->comm, COMM_DARKCOLOR ) )
          {
               switch ( type )
               {
               case 'R':
               case 'G':
               case 'B':
               case 'M':
               case 'C':
               case 'Y':
               case 'W':
                    type = tolower (type);
                    break;

               default:
                    break;
               }
          }
          if ( IS_SET ( ch->comm, COMM_NODARKGREY) )
               nodgrey = TRUE;
          if ( IS_SET ( ch->comm, COMM_NOFLASHY) )
               noflashy = TRUE;
          if ( IS_SET ( ch->comm, COMM_BEEP) )
               beep = TRUE;
     }

     switch ( type )
     {
     default:
          SNP ( code, CLEAR );
          break;
     case 'x':
          SNP ( code, CLEAR );
          break;
     case 'b':
          SNP ( code, C_BLUE );
          break;
     case 'c':
          SNP ( code, C_CYAN );
          break;
     case 'd':
          if ( !nodgrey ) /* Limit invisible text. */
               SNP ( code, FG_BLACK );
          break;
     case 'g':
          SNP ( code, C_GREEN );
          break;
     case 'm':
          SNP ( code, C_MAGENTA );
          break;
     case 'r':
          SNP ( code, C_RED );
          break;
     case 'w':
          SNP ( code, C_WHITE );
          break;
     case 'y':
          SNP ( code, C_YELLOW );
          break;
     case 'B':
          SNP ( code, C_B_BLUE );
          break;
     case 'C':
          SNP ( code, C_B_CYAN );
          break;
     case 'G':
          SNP ( code, C_B_GREEN );
          break;
     case 'M':
          SNP ( code, C_B_MAGENTA );
          break;
     case 'R':
          SNP ( code, C_B_RED );
          break;
     case 'W':
          SNP ( code, C_B_WHITE );
          break;
     case 'Y':
          SNP ( code, C_B_YELLOW );
          break;
     case 'D':
          if ( !nodgrey )
               SNP ( code, C_D_GREY );
          break;
     case '*':
          if ( !beep )
               SNP ( code, "%c", 007 );
          break;
     case '/':
          SNP ( code, "%c", 012 );
          break;
     case '{':
          SNP ( code, "%c", '{' );
          break;
     case '3':
          if ( !noflashy )
               SNP ( code, MOD_UNDERLINE );
          break;
     case '4':
          if ( !noflashy )
               SNP ( code, MOD_REVERSE );
          break;
     case '&':
          if ( !noflashy )
               SNP ( code, MOD_BLINK );
          break;
     case '#':
          SNP ( code, "%c", '%' );
          break;
     case '-':
          SNP ( code, "%c", '~' );
          break;
     }

     p = code;
     while ( *p != '\0' )
     {
          *string = *p++;
          *++string = '\0';
     }

     return ( strlen ( code ) );
}

void colourconv ( char *buffer, const char *txt, CHAR_DATA * ch )
{
     const char         *point;
     int                 skip = 0;

     if ( ch->desc && txt )
     {
          if ( ch->desc->ansi || ch->desc->mxp )
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         skip = colour ( *point, ch, buffer );
                         while ( skip-- > 0 )
                              ++buffer;
                         continue;
                    }
                    *buffer = *point;
                    *++buffer = '\0';
               }
               *buffer = '\0';
          }
          else
          {
               for ( point = txt; *point; point++ )
               {
                    if ( *point == '{' )
                    {
                         point++;
                         continue;
                    }
                    *buffer = *point;
                    *++buffer = '\0';
               }
               *buffer = '\0';
          }
     }
     return;
}

/* Zeran - ok, here goes a nasty bit of code to prevent players
	from creating chars with names that are subsets or supersets
	of existing names.  ie, player Tyler, Tyl, Tylera.  This is
	to prevent nasty side effects of using abbreviations in the
	game to reference a player */

/* Loth - This was crashing, the rules didn't work well anyway. */

//bool is_sub_super_name ( char *newname )
//{
//	 FILE               *players;
//	 char                tmpstr[128];
//
//	 /* ugly command string to generate a current player list */
//	 char               *command_str =
//		  "ls -l /home/mudadm/mud/player/ | grep -v \"total\" | grep -v \"current_player_list\" | awk '{print $9}' | awk -F\".\" '{print $1}' > /home/mudadm/mud/player/current_player_list";
//
//	 /* the system call blocks interrupts, so if something goes haywire
//	  * here, its gonna be messy */
//	 system ( command_str );
//	 /* Ok, got the current_player_list...now lets parse it */
//	 if ( ( players =
//			fopen ( "/home/mudadm/mud/player/current_player_list",
//					"r" ) ) == NULL )
//	 {
//		  bugf ( "couldn't open current_player_list for reading" );
//		  fclose ( players );
//		  return FALSE;
//	 }
//	 while ( fscanf ( players, "%s", tmpstr ) != EOF )
//	 {
//		  if ( strstr ( tmpstr, newname ) == tmpstr ||
//			   strstr ( newname, tmpstr ) == newname )
//		  {
//			   fclose ( players );
//			   return TRUE;
//		  }
//	 }
//	 fclose ( players );
//	 return FALSE;
//}

/* Begin Copyover Code. */

/*  Copyover - Original idea: Fusion of MUD++ */
/*  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */

#define CH(descriptor)  ((descriptor)->original ? (descriptor)->original : (descriptor)->character)

void do_copyover (CHAR_DATA *ch, char * argument)
{
     FILE *fp;
     CHAR_DATA *gch;
     DESCRIPTOR_DATA *d, *d_next;
     char buf [100], buf2[100], buf3[100];
     extern int mud_desc;

     fp = fopen (COPYOVER_FILE, "w");

     if (!fp)
     {
          send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
          bugf ("Could not write to copyover file.");
          perror ("do_copyover:fopen");
          return;
     }

     SNP (buf, "\n\rCopyover by %s - One moment please.\n\r", ch->name);

	 /* MCCP doesn't handle being "copyover'ed" well. */

     for (d = descriptor_list; d; d = d->next)
     {
          if (d->character != NULL)
               gch = d->character;
          else
               continue;
#if !defined(NOZLIB)
          if (gch->desc->out_compress)
          {
               if (!compressEnd(gch->desc))
                    send_to_char("Could not disable compression, you'll have to reconect in 5 seconds.\n", gch);
          }
#endif
     }

     /* For each playing descriptor, save its state */
     for (d = descriptor_list; d ; d = d_next)
     {
          CHAR_DATA * och = CH (d);
          d_next = d->next; /* We delete from the list , so need to save this */
          
          if (!d->character || d->connected > CON_PLAYING) /* drop those logging on and *sigh* writing notes */
          {
               write_to_descriptor_2(d->descriptor, "\n\rSorry, the mud is rebooting. \n\rCome back in a few minutes.\n\r", 0);
               close_socket (d); /* throw'em out */
          }
          else
          {
               fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
               save_char_obj (och);               
               write_to_descriptor_2 (d->descriptor, buf, 0);
          }
     }
     
     fprintf (fp, "-1\n");
     fclose (fp);

     /*
      * do the saves here so not opening another file at the same time copyover is open and so users see
      * the message that copyover is happening *BEFORE* the big lag. If possible.
      */
     
     // Got tired of having manually done changes forgotten about and overwritten
     //     if ( str_cmp ( argument, "noasave" ) )
     //	 {
     //		  do_asave (NULL, "");
     //	 }
     save_leases ( );
     save_clans ( );
     fwrite_accounts ( );

	 /* Close reserve and other always-open files and release other resources */

     fclose (fpReserve);
    
    if( I3_is_connected() )
    {
        I3_savechanlist();
        I3_savemudlist();
        I3_savehistory();
    }
    

	 /* exec - descriptors are inherited */

     SNP (buf,  "%d", mud.port);
     SNP (buf2, "%d", mud_desc);
     SNP (buf3, "%d", I3_socket );    

     execl(EXE_FILE, " ", buf, "sundermud", buf2, buf3, (char *) NULL);

	 /* Failed - sucessful exec will not return */

     perror ("do_copyover: execl");
     send_to_char ("Copyover FAILED!\n\r",ch);

	 /* Here you might want to reopen fpReserve */
     fpReserve = fopen (NULL_FILE, "r");

}

/* Recover from a copyover - load players */
void copyover_recover ()
{
     static DESCRIPTOR_DATA d_zero;
     DESCRIPTOR_DATA *d;
     FILE *fp;
     char name [100];
     char host[MSL];
     int desc;
     bool fOld;

     log_string ("Copyover recovery initiated");

     fp = fopen (COPYOVER_FILE, "r");

     if (!fp) /* there are some descriptors open which will hang forever then ? */
     {
          perror ("copyover_recover:fopen");
          bugf ("Copyover file not found. Exiting.\n\r");
          exit (1);
     }

     unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading  */

     for (;;)
     {
          fscanf (fp, "%d %s %s\n", &desc, name, host);
          if (desc == -1)
               break;

		  /* Write something, and check if it goes error-free */
          if (!write_to_descriptor_2 (desc, "\n\rRestoring....\n\r",0))
          {
               close (desc); /* nope */
               continue;
          }

          d = alloc_perm (sizeof(DESCRIPTOR_DATA), "descriptor::copyover");

          *d = d_zero;
          d->descriptor    = desc;
          d->character	 = NULL;
          d->showstr_head  = NULL;
          d->showstr_point = NULL;
          d->outsize  	 = 2000;
          d->pEdit         = NULL;                 /* OLC */
          d->pString       = NULL;                 /* OLC */
          d->editor        = 0;                    /* OLC */
          d->outbuf  	 = alloc_mem( d->outsize, "d->outbuf" );
          d->connected	 = CON_COPYOVER_RECOVER;
          d->host = str_dup (host);
          d->next = descriptor_list;
          descriptor_list = d;

		  /* Now, find the pfile */

          fOld = load_char_obj (d, name);

          if (!fOld) /* Player file not found?! */
          {
               write_to_descriptor_2 (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
               close_socket (d);
          }
          else /* ok! */
          {
               write_to_descriptor_2 (desc, "\n\rReady to Rock.\n\r",0);

			   /* Just In Case */
               if (!d->character->in_room)
                    d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

			   /* Insert in the char_list */
               d->character->next = char_list;
               char_list = d->character;

			   /* Need to recheck telnet negotiations */
               write_to_buffer( d, compress_will, 0 );
               write_to_buffer( d, mxp_will, 0 );
               write_to_buffer( d, msp_will, 0 );


               
               char_to_room (d->character, d->character->in_room);
               do_look (d->character, "auto");
               d->connected = CON_PLAYING;

               if (d->character->pet != NULL)
               {
                    char_to_room(d->character->pet,d->character->in_room);
               }
          }

     }
     fclose (fp);
     fCopyOver = FALSE;
     return;
}

/* End copyover code */

void mxp_in ( DESCRIPTOR_DATA * d, char *argument )
{
     char buf[MSL];

     int i = 0;

     // MushClient Response:
     // [1z<VERSION MXP="0.4" CLIENT=MUSHclient VERSION="3.20" REGISTERED=NO> ]
     //
     // zMud Response:
     // [1z<VERSION MXP=0.5 CLIENT=zMUD VERSION=6.30>
     //
     // Smash the ESC [1z at the start.
     //
     for ( ; *argument != '\0'; argument++ )
     {
          if ( *argument == '' )
          {
               argument +=3;
          }
          else
          {
               buf[i] = *argument;
               i++;
          }
     }
     buf[i] = '\0'; // terminate the string

     // I should probably parse the string, but that's a level of complexity to which I will not go.
     // I'm just going to use it to determine the client.
     //
     if ( strstr ( buf, "VERSION" ) )
     {
          if ( d->client[0] != '\0' )
               free_string ( d->client );
          if ( strstr ( buf, "zMUD" ) )
               d->client = str_dup ( "zMUD" );
          else if ( strstr ( buf, "MUSHclient" ) )
               d->client = str_dup ( "MUSHclient" );
          else
          {
               d->client = str_dup ( "Unrecognized MXP Client." );
               bugf( "Unknown MXP Client: %s", buf );
          }
     }
     // Supports needs work to be useful. Currently a query for all supported
     // types results in a truncated response from MushClient due to the list being
     // longer than MAX_INPUT_LENGTH... I suppose one could append to the list...
     // but for now I'm not going to use it.
     else if ( strstr ( buf, "SUPPORTS" ) )
     {
          d->support = str_dup ( buf );
          log_string ( buf );
     }
     else
     {
          // Let's see what else we get in on secure lines.
          // Be careful here, because a player could have sent this, I accidentally
          // sent an escaped command from my xterm.... didn't realize I could do it,
          // so we know now that what we get back from the user isn't secure even if
          // MXP specification thinks it is... Don't trust anyone... - Lotherius
          bugf( "Unknown mxp_in response: %s", argument );
     }
     return;
}

