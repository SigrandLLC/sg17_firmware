#ifndef LIB_BUFFERPOOL_H
#define LIB_BUFFERPOOL_H
/******************************************************************************

                               Copyright (c) 2006
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
  WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
  SOFTWARE IS FREE OF CHARGE.

  THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND INFINEON EXPRESSLY DISCLAIMS
  ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
  WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
  OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY THIRD
  PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY INTELLECTUAL
  PROPERTY INFRINGEMENT.

  EXCEPT FOR ANY LIABILITY DUE TO WILFUL ACTS OR GROSS NEGLIGENCE AND EXCEPT
  FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR ANY CLAIM
  OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

******************************************************************************/

/**
   \file lib_bufferpool.h
   The bufferpool provides buffers with preallocated memory.
*/

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */
/* Not supported in kernel space.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
*/
/* ========================================================================== */
/* Global Defines                                                             */
/* ========================================================================== */
#define BUFFERPOOL_ERROR         (-1)
#define BUFFERPOOL_SUCCESS       0

#define  GET_CLEAN_BUFFERS       0
#define  DO_CHECKS_ON_PUT        1
#define  SHOW_MAGIC_ERROR        1
#define  SHOW_INUSE_ERROR        1
#define  SHOW_NOFREEBUF_ERROR    1
#define  SHOW_ERRORS             1
#define  SHOW_PATTERN_WARNING    1
#define  SHOW_WARNINGS           1


/* ========================================================================== */
/* Structure definition.                                                      */
/* ========================================================================== */
typedef struct _BUFFERPOOL
{
    unsigned int    hdrSize;             /* header size (bytes) */
    unsigned int    bufSize;             /* buf size (bytes) */
    unsigned int    ftrSize;             /* footer size (bytes) */
    unsigned int    elements;            /* current number of elements */
    unsigned int    freeElements;        /* current number of free elements */
    unsigned int    incrElements;        /* inc step if buffer size too small */

    struct _BufferInternal   *pHead;     /* buffers are taken from head */
    struct _BufferInternal   *pTail;     /* recycled buffers are added here */
    struct _BufferInternal   *pBlockList;/* list of element blocks allocated */
} BUFFERPOOL;

struct _BufferInternal;

typedef struct _BufferHeader
{
   unsigned int            magic;
   unsigned int            state;
   BUFFERPOOL              *pbp;
   struct _BufferInternal *pNext;
   struct _BufferInternal *pPrev;
   struct _BufferInternal *pNextBlock;
   char dummy[8];
} tBufferHeader;

typedef struct _BufferFooter {
   unsigned int            pattern;
   char dummy[28];
} tBufferFooter;

typedef struct _BufferInternal {
   tBufferHeader           hdr;
} tBufferInternal;

/* ========================================================================== */
/* Exported Functions                                                         */
/* ========================================================================== */
BUFFERPOOL* bufferPoolInit (const unsigned int bufferSize,
                            const unsigned int initialElements,
                            const unsigned int extensionStep);
void* bufferPoolGet        (void *pbp);
int   bufferPoolPut        (void *pb);

int   bufferPoolAvail      (const BUFFERPOOL *pbp);
int   bufferPoolElementSize(BUFFERPOOL *pbp);
int   bufferPoolSize       (const BUFFERPOOL *pbp);

/* -- UNDER CONSTRUCTION / DEBUG functions -- */
int   bufferPoolFree       (BUFFERPOOL *pbp);
int   bufferPoolDump       (const char *whatHappened, const void *pb);
void  printBufferPoolErrors(void);
int   bufferPoolDumpRTP    (const char *whatHappened, const void *pb);

#endif /* LIB_BUFFERPOOL_H */

