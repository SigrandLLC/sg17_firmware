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
   \file lib_bufferpool.c
   The bufferpool provides buffers with preallocated memory.

   \verbatim
                                                           NULL
                                                           ^
                                                           |pPrev
             +----------------+   pHead----->+---------------+
             + bufferPool     +   GET        + bufferElement +
             +----------------+              +---------------+
             + pHead          +               |pNext       ^
             + pTail          +               V            |pPrev
             + ...            +              +---------------+
             +----------------+              + bufferElement +
                                             +---------------+
                                  pTail----->+---------------+
                                  PUT        + bufferElement +
                                             +---------------+
                                              |pNext
                                              V
                                              NULL


           The internal buffer layout differs depending on the
           current state of the buffer. While the buffer is
           free and in the buffer pool, the first two DWORDS are
           used for the pointers to the next/previous buffer
           as depicted below.

        a) internal buffer layout and pointers for used buffers

                    31               0
                    +-----------------+
            pbi-->  |  MAGIC Pattern  |   pointer to buffer internally
                    |      state      |   (points to the buffer head)
                    |   ptr to pool   |
                    +-----------------+
            pb -->  .                 .   pointer to buffer used
                    .      data       .   externally
                    .                 .
                    +-----------------+
            pf -->  |  CHECK Pattern  |   pointer to buffer footer
                    +-----------------+

        b) internal buffer layout and pointers for free buffers
           in the pool

                    31               0
                    +-----------------+
            pbi-->  |  MAGIC Pattern  |   pointer to buffer internally
                    |      state      |   (points to the buffer head)
                    |   ptr to pool   |
                    +-----------------+
            pb -->  .     pNext       .   pointer to buffer used
                    .     pPrev       .   externally
                    .                 .
                    +-----------------+
            pf -->  |  CHECK Pattern  |   pointer to buffer footer
                    +-----------------+
   \endverbatim

*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include <lib_bufferpool.h>
#include <sys_drv_ifxos.h>
#ifdef LINUX
/* if linux/slab.h is not available, use the precessor linux/malloc.h */
#include <linux/slab.h>
#endif /* LINUX */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
#define MAGIC_PATTERN   0x24101974
#define CHECK_PATTERN   0xAAAAAAAA

#define GET_HDR_PTR(pb) \
   (((tBufferHeader *)pb) - 1);\

#define GET_DATA_PTR(pbi) \
   (((tBufferHeader *)pbi) + 1)

#define GET_FTR_PTR(pbi) \
   (void*) ((((char *)pbi)+ ((tBufferHeader*)pbi)->pbp->hdrSize) + \
                               ((tBufferHeader*)pbi)->pbp->bufSize)

#define GET_DATA_SIZE(pbi) \
   ((unsigned int) (((tBufferHeader*)pbi)->pbp->bufSize))

#ifdef LINUX
#define printf printk
#endif /* LINUX */

typedef enum {
   BUFFER_STATE_FREE,
   BUFFER_STATE_IN_USE,
   BUFFER_STATE_END_OF_STATES
} BUFFER_STATES;

/* ============================= */
/* Local variable definition     */
/* ============================= */
static unsigned int   errorCnt_MAGIC   = 0;
static unsigned int   errorCnt_Pattern = 0;
static unsigned int   errorCnt_PutNULL = 0;
static unsigned int   errorCnt_PutFree = 0;

/* ============================= */
/* Local function definition     */
/* ============================= */

#if (DO_CHECKS_ON_PUT == 1)
static int checkMagic(const tBufferHeader *ph) {
   return ((ph->magic == MAGIC_PATTERN) ? 1 : 0);
}

static int checkPattern(const tBufferHeader *ph) {
   unsigned int *pFtr;
   pFtr = GET_FTR_PTR(ph);
   return ((*pFtr == CHECK_PATTERN) ? 1 : 0);
}

static int checkInUse (const tBufferHeader *ph) {
   return ((ph->state == BUFFER_STATE_IN_USE) ? 1 : 0);
}
#endif


/*******************************************************************************
Description:
   Add <count> new buffers to the bufferpool (pbp).
Arguments:
   pbp   -  bufferpool handle
   count -  number of buffers to be added to the bufferpool
Return:
   BUFFERPOOL_SUCCESS or BUFFERPOOL_ERROR if malloc failed
*******************************************************************************/
static int initBuffer(BUFFERPOOL *pbp, const unsigned int count) {
   tBufferInternal    *pbi;    /* internal handle pointing to buffer header */
   tBufferFooter      *pf;     /* buffer footer */
   unsigned int        elementSize = 0;
   unsigned int        initialSize = 0;
   unsigned int        i;

   elementSize = (sizeof(tBufferHeader) + pbp->bufSize + sizeof(tBufferFooter));
   initialSize = count * elementSize;

   /* Sanity Check */
   if (pbp->pTail != NULL)
   {
       printf("ERROR initBuffer pTail != NULL\n");
       return(BUFFERPOOL_ERROR);
   }
   /* Allocate memory and add the new buffers to the bufferpool */
   pbp->pTail           = (tBufferInternal *) IFXOS_MALLOC(initialSize);
   if (pbp->pTail == NULL)
      return (BUFFERPOOL_ERROR);
   pbp->pHead           = (tBufferInternal *)
                      (((unsigned int) pbp->pTail) + initialSize - elementSize);
   pbi                  = pbp->pTail;
   pbp->elements       += count;
   pbp->freeElements   += count;
   /* link this memory block into a list needed for buffer pool free */
   pbi->hdr.pNextBlock  = pbp->pBlockList;
   pbp->pBlockList      = pbi;

   /* Initialize the new buffers */
   for (i=0; i < count ; i++)
   {
      /* add header */
      pbi->hdr.magic = MAGIC_PATTERN;
      pbi->hdr.pbp   = pbp;
      pbi->hdr.state = BUFFER_STATE_FREE;

      /* add footer */
      pf = GET_FTR_PTR (pbi);
      pf->pattern    = CHECK_PATTERN;

      /* set pPrev and pNext for each buffer */
      if (i < count-1)
         pbi->hdr.pPrev = (tBufferInternal*)(((unsigned int) pbi) + elementSize);
      else
         pbi->hdr.pPrev = NULL;
      if (i > 0)
         pbi->hdr.pNext = (tBufferInternal*)(((unsigned int) pbi) - elementSize);
      else
         pbi->hdr.pNext = NULL;

      pbi = pbi->hdr.pPrev;
   }
   return BUFFERPOOL_SUCCESS;
}

/**
   Create a new bufferpool and initalise it.

   Initialize a new bufferpool with "initialElements" buffers of "buffersize"
   bytes. Define the "extensionStep" of buffers to add to the bufferpool when
   the last buffer is taken from the pool. If the dynamic extension of the
   bufferpool is not desired set "extensionStep" to 0.

   \param  bufferSize        Size of one buffer in bytes.
   \param  initialElements   Number of empty buffers created at initialisation.
   \param  extensionStep     Number of empty buffers to be added when the last 
                             buffer is taken out of the bufferpool. Set to 0
                             to prevent dynamic growing of the bufferpool.

   \return
   The return value is a handle (pointer) to the bufferpool or a NULL pointer
   if the initialisation failed.
*/
BUFFERPOOL* bufferPoolInit (const unsigned int bufferSize,
                            const unsigned int initialElements,
                            const unsigned int extensionStep)
{
   BUFFERPOOL        *pbp;    /* buffer pool */

   /* Allocate memory for the bufferpool management structure */
   pbp                = (BUFFERPOOL *) IFXOS_MALLOC(sizeof(BUFFERPOOL));
   if (NULL == pbp)
      return (NULL);

   /* Initalize the bufferpool management structure */
   pbp->hdrSize       = sizeof(tBufferHeader);
   pbp->bufSize       = bufferSize;
   pbp->ftrSize       = sizeof(tBufferFooter);
   pbp->incrElements  = extensionStep;

   pbp->elements      = 0;
   pbp->freeElements  = 0;
   pbp->pHead         = NULL;
   pbp->pTail         = NULL;
   pbp->pBlockList    = NULL;

   if (BUFFERPOOL_ERROR == initBuffer (pbp, initialElements))
      return (NULL);

   return pbp;
}

/**
   Unconditionally free an existing bufferpool.

   Free all memory allocated for the elements of the buffer pool and also
   free the bufferpool management structure. No check is done if all buffer
   elements are unused.

   \param   pbp         Pointer to a bufferpool management structure.

   \return
     - \ref BUFFERPOOL_SUCCESS: if successful
     - \ref BUFFERPOOL_ERROR: when pointer to bufferpool is incorrect
*/
int bufferPoolFree(BUFFERPOOL *pbp)
{
   tBufferInternal   *pbi;

   if (pbp == NULL || pbp->pHead == NULL) {
      printf ("\nERROR bufferPoolFree, no valid buffer pool\n");
      return BUFFERPOOL_ERROR;
   }

   /* Free all allocated memory blocks. */
   while (pbp->pBlockList != NULL)
   {
      pbi = pbp->pBlockList->hdr.pNextBlock;
      IFXOS_FREE(pbp->pBlockList);
      pbp->pBlockList = pbi;
   }

   /* Free the bufferpool management structure */
   IFXOS_FREE(pbp);

   return BUFFERPOOL_SUCCESS;
}

/*******************************************************************************
Description:
   Request a buffer from the bufferpool.
Arguments:
   pbp   - a handle (pointer) to the bufferpool
Return:
   The return value is a void pointer to the provided buffer or a NULL pointer
   if no more buffers are available in the bufferpool.
*******************************************************************************/
void* bufferPoolGet(void *pbp)
{
   tBufferInternal *pbi;
   void            *pd;

   if (NULL != ((BUFFERPOOL *)pbp)->pHead)
   {
      pbi = ((BUFFERPOOL *)pbp)->pHead;
      if(pbi->hdr.state == BUFFER_STATE_IN_USE)
      {
         return NULL;
      }
      ((BUFFERPOOL *)pbp)->freeElements--;
      pd = GET_DATA_PTR(((BUFFERPOOL *)pbp)->pHead);

      if (((BUFFERPOOL *)pbp)->pHead->hdr.pNext != NULL)
      {
         ((BUFFERPOOL *)pbp)->pHead->hdr.pNext->hdr.pPrev = NULL;
         ((BUFFERPOOL *)pbp)->pHead = ((BUFFERPOOL *)pbp)->pHead->hdr.pNext;
      }
      else
      {
         ((BUFFERPOOL *)pbp)->pHead               = NULL;
         ((BUFFERPOOL *)pbp)->pTail               = NULL;
         if (((BUFFERPOOL *)pbp)->incrElements != 0)
         {
            initBuffer((BUFFERPOOL*)pbp, ((BUFFERPOOL*)pbp)->incrElements);
         }
      }
      #if (GET_CLEAN_BUFFERS == 1)
      memset(pd, 0, GET_DATA_SIZE(pbi));
      #endif
      pbi->hdr.state = BUFFER_STATE_IN_USE;
      return pd;
   }
   else
   {
      #if (SHOW_NOFREEBUF_ERROR == 1)
      printf("ERROR bufferPoolGet - no buffer free\n");
      #endif
      return NULL;
   }
}

/*******************************************************************************
Description:
   Return a buffer to the bufferpool.
Arguments:
   pb   - a pointer to the buffer to be returned
Return:
   BUFFERPOOL_SUCCESS or BUFFERPOOL_ERROR
*******************************************************************************/
/* FIXME: return value should use IFX_SUCCESS or IFX_ERROR. */
int bufferPoolPut(void *pb)
{
   tBufferHeader *ph  = NULL;
   BUFFERPOOL    *pbp = NULL;
   #if (DO_CHECKS_ON_PUT == 1)
   tBufferFooter *pf  = NULL;
   #endif

   if (pb != NULL)
   {
      ph = GET_HDR_PTR(pb);

      #if (DO_CHECKS_ON_PUT == 1) /* ********************************checks****/
      if (!checkPattern(ph))
      {
         #if (SHOW_PATTERN_WARNING == 1)
         bufferPoolDump("Pattern failed", pb);
         printf("\nWARNING bufferPoolPut Pattern failure\n");
         #endif
         /* fix footer */
         pf = GET_FTR_PTR(ph);
         pf->pattern = CHECK_PATTERN;
         errorCnt_Pattern++;
      }
      if (!checkMagic(ph))
      {
         #if (SHOW_MAGIC_ERROR == 1)
         printf("\nERROR bufferPoolPut Magic   failure, <%04X>\n", ph->magic);
         #endif
         errorCnt_MAGIC++;
         return BUFFERPOOL_ERROR;
      }
      if (!checkInUse(ph))
      {
         #if (SHOW_INUSE_ERROR == 1)
         printf("\nERROR bufferPoolPut, buffer has been returned already\n");
         #endif
         errorCnt_PutFree++;
         return BUFFERPOOL_ERROR;
      }
      #endif                      /* ********************************checks****/

      /* Get relevant bufferpool */
      pbp                                  = ph->pbp;
      /* Set the bufferstate to free and add it to the bufferpool */
      ph->state                            = BUFFER_STATE_FREE;
      ((tBufferInternal*)ph)->hdr.pPrev        = pbp->pTail;
      ((tBufferInternal*)ph)->hdr.pNext        = NULL;

      /* Add the link from the tail element to the new element (if available) */
      if (pbp->pTail != NULL)
         pbp->pTail->hdr.pNext                 = (tBufferInternal *) ph;
      /* If the bufferpool was empty, set the pHead pointer to the new buffer */
      if (pbp->pHead == NULL)
         pbp->pHead                        = (tBufferInternal *) ph;
      /* Move the pTail pointer to the new buffer and increase  freeElements  */
      pbp->pTail                           = (tBufferInternal *) ph;
      pbp->freeElements++;
   }
   else
   {
      #if (SHOW_WARNINGS == 1)
      printf("\nERROR bufferPoolPut NULL\n");
      #endif
      errorCnt_PutNULL++;
      return BUFFERPOOL_ERROR;
   }
   return BUFFERPOOL_SUCCESS;
}


/*******************************************************************************
Description:
   Return the size of a buffer element managed by the bufferpool. All buffer
   in a buffer pool have the same size. This is the maximum size and not the
   used size of the buffer elements. This size is given by the 'bufferPoolInit'
   routine.
Arguments:
   pbp   - a handle (pointer) to the bufferpool
Return:
   The return value is the overall number of buffers managed by the bufferpool.
*******************************************************************************/
int bufferPoolElementSize(BUFFERPOOL *pbp)
{
   return(pbp->bufSize);
}

/*******************************************************************************
Description:
   Return the overall number of buffers managed by the bufferpool.
Arguments:
   pbp   - a handle (pointer) to the bufferpool
Return:
   The return value is the overall number of buffers managed by the bufferpool
   or BUFFERPOOL_ERROR if pbp is a NULL pointer.
*******************************************************************************/
int bufferPoolSize(const BUFFERPOOL *pbp)
{
   if (pbp == NULL)
      return BUFFERPOOL_ERROR;

   return(pbp->elements);
}

/*******************************************************************************
Description:
   Return the number of currently free buffers
Arguments:
   pbp   - a handle (pointer) to the bufferpool
Return:
   The return value is the number of free buffers in the bufferpool
   or BUFFERPOOL_ERROR if pbp is a NULL pointer.
*******************************************************************************/
int bufferPoolAvail(const BUFFERPOOL *pbp)
{
   if (pbp == NULL)
      return BUFFERPOOL_ERROR;

   return(pbp->freeElements);
}

/*******************************************************************************
Description:
   Debug function -- under construction --
   Trace of the buffer content
Arguments:
   whatHappened   - string to be printed together with the buffer contents
   pb             - a handle (pointer) to a buffer
Return:
   BUFFERPOOL_SUCCESS or BUFFERPOOL_ERROR
*******************************************************************************/
int bufferPoolDump(const char *whatHappened, const void *pb)
{
   unsigned int   i;
   tBufferHeader *ph;
   tBufferFooter *pf;

   if (pb == NULL) {
      printf("ERROR no buffer to dump\n");
      return BUFFERPOOL_ERROR;
   }
   /*printf("bufferPoolDump @%p\n", pb);*/
   ph = GET_HDR_PTR(pb);
   pf = GET_FTR_PTR(ph);

   /*printf("%s\n", whatHappened);*/
   printf("HDR @[%lXh]  <", (unsigned long) ph);
   for (i=0; i < ph->pbp->hdrSize; i++)
   {
      printf("%02X", ((unsigned char *)ph)[i]);
      if (i < ph->pbp->hdrSize -1) printf(" ");
   }
   printf("> [%d] Bytes\n", i);

   printf("BUF @[%lXh]  <", (unsigned long) pb);
   for (i=0; i < GET_DATA_SIZE(ph); i++)
   {
      printf("%02X", ((unsigned char *)pb)[i]);
      if (i < GET_DATA_SIZE(ph) -1) printf(" ");
   }
   printf("> [%d] DataSize\n", GET_DATA_SIZE(ph));

   printf("FTR @[%lXh]  <", (unsigned long) pf);
   for (i=0; i < ph->pbp->ftrSize; i++)
   {
      printf("%02X", ((unsigned char *)pf)[i]);
      if (i < ph->pbp->ftrSize -1) printf(" ");
   }
   printf("> [%d] Bytes\n", i);

   return BUFFERPOOL_SUCCESS;
}

/*******************************************************************************
Description:
   Debug function -- under construction --
Arguments:
   whatHappened   - string to be printed together with the buffer contents
   pb             - a handle (pointer) to a buffer
Return:
   BUFFERPOOL_SUCCESS or BUFFERPOOL_ERROR
*******************************************************************************/
int bufferPoolDumpRTP(const char *whatHappened, const void *pb)
{
   unsigned int   i;
   unsigned int   dataSizeWords;
   tBufferHeader *ph;

   if (pb == NULL) {
      printf("ERROR no buffer to dump\n");
      return(-1);
   }
   ph = GET_HDR_PTR(pb);

   printf("CMD/RTP @[%lXh]  <", (unsigned long) pb);
   dataSizeWords = GET_DATA_SIZE(ph)>>1; /* /2 */
   for (i=0; i < (dataSizeWords); i++)
   {
      if (i < dataSizeWords && i!=0 && i%20 == 0)
         printf("\n                     ");
      printf("%04X", ((unsigned short *)pb)[i]);
      if (i < dataSizeWords)
         printf(" ");
   }
   printf("> [%d] Words\n", dataSizeWords);
   return BUFFERPOOL_SUCCESS;
}

/*******************************************************************************
Description:
   Debug function -- under construction --
   Prints all error counters
Arguments:
   none
Return:
   none
*******************************************************************************/
void printBufferPoolErrors(void)
{
   printf("\nerrMagic         %5d"
          "\nerrPattern       %5d"
          "\nerrPutNULL       %5d"
          "\nerrPutFree       %5d\n",
          errorCnt_MAGIC, errorCnt_Pattern, errorCnt_PutNULL, errorCnt_PutFree);
}

