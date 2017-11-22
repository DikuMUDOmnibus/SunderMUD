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


/*** buffer.h ****/

#ifndef _BUFFER_H
# define _BUFFER_H   1

# define BUFFER_DEBUG

typedef struct buffer_type BUFFER;

struct buffer_type
{
     char *data; /* The data */

     int len;	/* The current len of the buffer */
     int size;	/* The allocated size of data */

     bool overflowed; /* Is the buffer overflowed? */
};

# ifdef BUFFER_DEBUG /* Debugged version */

#  define buffer_new(size)           __buffer_new (size, __FILE__, __LINE__)
#  define buffer_strcat(buffer,text) __buffer_strcat (buffer, text, __FILE__, __LINE__)

BUFFER * __buffer_new (int size, const char *file, unsigned line);
void __buffer_strcat (BUFFER *buffer, const char *text, const char *file, unsigned line);

# else  /* not debugged version */

#  define buffer_new(size)           __buffer_new (size)
#  define buffer_strcat(buffer,text) __buffer_strcat (buffer,text)

BUFFER * __buffer_new (int size);
void __buffer_strcat (BUFFER *buffer, const char *text);

# endif

void buffer_free (BUFFER *buffer);
void buffer_clear (BUFFER *buffer);
int find_mem_size (int min_size);
int bprintf (BUFFER *buffer, char *fmt, ...);

#endif // _BUFFER_H
