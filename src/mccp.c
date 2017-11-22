/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

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

#if !defined (NOZLIB)
#include <arpa/telnet.h>

char compress_start  [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };

bool processCompressed(DESCRIPTOR_DATA *desc);
bool write_to_descriptor args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );

int mccp_mem_usage = 0;
int mccp_mem_freed = 0;

/*
 * Memory management - zlib uses these hooks to allocate and free memory
 * it needs
 */

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    mccp_mem_usage += size;
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    mccp_mem_freed += sizeof(*address);
    free(address);
}

/*
 * Begin compressing data on `desc'
 */

bool compressStart(DESCRIPTOR_DATA *desc)
{
    z_stream *s;
   
    if (desc->out_compress) /* already compressing */
        return TRUE;

    /* allocate and init stream, buffer */
    s = (z_stream *)alloc_mem(sizeof(*s), "zstream");
    desc->out_compress_buf = (unsigned char *)alloc_mem(COMPRESS_BUF_SIZE, "out_compress_buf");
   
    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = desc->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK) {
        /* problems with zlib, try to clean up */
        free_mem(desc->out_compress_buf, COMPRESS_BUF_SIZE, "out_compress_buf");
        free_mem(s, sizeof(z_stream), "zstream");
        return FALSE;
    }

    write_to_descriptor(desc, compress_start, strlen(compress_start));

    /* now we're compressing */
    desc->out_compress = s;
    return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd(DESCRIPTOR_DATA *desc)
{
    unsigned char dummy[1];
   
    if (!desc->out_compress)
        return TRUE;

    desc->out_compress->avail_in = 0;
    desc->out_compress->next_in = dummy;

    /* No terminating signature is needed - receiver will get Z_STREAM_END */
   
    if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;

    if (!processCompressed(desc)) /* try to send any residual data */
        return FALSE;

    deflateEnd(desc->out_compress);
    free_mem(desc->out_compress_buf, COMPRESS_BUF_SIZE, "out_compress_buf");
    free_mem(desc->out_compress, sizeof(z_stream), "zstream");
    desc->out_compress = NULL;
    desc->out_compress_buf = NULL;

    return TRUE;
}

/* Try to send any pending compressed-but-not-sent data in `desc' */
/* This function seems very OS-Dependent */
bool processCompressed(DESCRIPTOR_DATA *desc)
{
    int iStart, nBlock, nWrite, len;

    if (!desc->out_compress)
        return TRUE;
   
    /* Try to write out some data.. */
    len = desc->out_compress->next_out - desc->out_compress_buf;
    if (len > 0) {
        /* we have some data to write */

        for (iStart = 0; iStart < len; iStart += nWrite)
        {
            nBlock = UMIN (len - iStart, 4096);
#if !defined(WIN32)
            if ((nWrite = write (desc->descriptor, desc->out_compress_buf + iStart, nBlock)) < 0)
#else
            if ((nWrite = write (desc->descriptor, desc->out_compress_buf + iStart, nBlock, 0)) < 0)
#endif
            {
                 /* FreeBSD doesn't include ENOSR */
#ifdef __linux__                 
                 if ( errno == ENOSR)
                      break;
#endif
                 if ( errno == EAGAIN )
                      break;

                return FALSE; /* write error */
            }

            if (nWrite <= 0)
                break;
        }

        if (iStart) {
            /* We wrote "iStart" bytes */
            if (iStart < len)
                memmove(desc->out_compress_buf, desc->out_compress_buf+iStart, len - iStart);

            desc->out_compress->next_out = desc->out_compress_buf + len - iStart;
        }
    }

    return TRUE;
}

/* write_to_descriptor, the compressed case */
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length)
{
    z_stream *s = desc->out_compress;
   
    s->next_in = (unsigned char *)txt;
    s->avail_in = length;

    while (s->avail_in) {
        s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - desc->out_compress_buf);
           
        if (s->avail_out) {
            int status = deflate(s, Z_SYNC_FLUSH);

            if (status != Z_OK) {
                /* Boom */
                return FALSE;
            }
        }

        /* Try to write out some data.. */
        if (!processCompressed(desc))
            return FALSE;

        /* loop */
    }
   
    /* Done. */
    return TRUE;
}
#endif

void do_showcompress( CHAR_DATA *ch, char *argument )
{

#if !defined(NOZLIB)
  DESCRIPTOR_DATA *d;
  BUFFER *buffer;
  CHAR_DATA *gch;
  int count1 = 0;
  int count2 = 0;

  if (IS_NPC(ch)) return;

  buffer = buffer_new(512);

  bprintf ( buffer, "Compression Usage: \n\r" );

  for (d = descriptor_list; d != NULL; d = d->next)
  {
    if (d->connected != CON_PLAYING) 
    	continue;
    if (d->character != NULL) 
    	gch = d->character;
    else continue;
    if (gch->desc->out_compress)
    {
      bprintf(buffer, "{GMCCP: {W%s{w\n\r", gch->name);
      count1++;
    }
    else
    {
      bprintf(buffer, "{RNONE: {W%s{w.\n\r", gch->name);
      count2++;
    }
  }
  bprintf(buffer,"\n\r%d out of %d players currently compressing.\n\r", count1, count2 + count1);
  page_to_char(buffer->data, ch);
  buffer_free (buffer);
#endif
  return;               
}


