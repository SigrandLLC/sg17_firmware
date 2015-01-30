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

 ****************************************************************************
   Module      : drv_tapi_tone.c
   Desription  : Contains TAPI Tone Services.
*******************************************************************************/


/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_ll_interface.h"
#include "drv_tapi_errno.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* tone coefficient maximums */
#define MAX_TONE_TIME          64000
#define MAX_TONE_FREQ          32768
#define MIN_TONE_POWER         -300
#define MAX_TONE_POWER         0
#define MAX_CADENCE_TIME      64000   /* ms */
#define MAX_VOICE_TIME        64000   /* ms */

#define MAX_CMD_WORD             31

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* Predefined tones frequencies */
static
const IFX_uint32_t TAPI_PredefinedToneFreq [IFX_TAPI_TONE_INDEX_MIN - 1][2] =
{
    /* Freq1/Freq2(Hz) *  Freq1/Freq2(Hz) - Index - DTMF Digit */
    {697, 1209},   /*   697/1209 Hz,   1,       1 */
    {697, 1336},   /*   697/1336 Hz,   2,       2 */
    {697, 1477},   /*   697/1477 Hz,   3,       3 */
    {770, 1209},   /*   770/1209 Hz,   4,       4 */
    {770, 1336},   /*   770/1336 Hz,   5,       5 */
    {770, 1477},   /*   770/1477 Hz,   6,       6 */
    {852, 1209},   /*   852/1209 Hz,   7,       7 */
    {852, 1336},   /*   852/1336 Hz,   8,       8 */
    {852, 1477},   /*   852/1477 Hz,   9,       9 */
    {941, 1209},   /*   941/1209 Hz,   10,      * */
    {941, 1336},   /*   941/1336 Hz,   11,      0 */
    {941, 1477},   /*   941/1477 Hz,   12,      # */
    {800,    0},   /*   800/0    Hz,   13,      - */
    {1000,   0},   /*   1000/0   Hz,   14,      - */
    {1250,   0},   /*   1250/0   Hz,   15,      - */
    {950,    0},   /*   950/0    Hz,   16,      - */
    {1100,   0},   /*   1100/0   Hz,   17,      - */ /* --> CNG Tone */
    {1400,   0},   /*   1400/0   Hz,   18,      - */
    {1500,   0},   /*   1500/0   Hz,   19,      - */
    {1600,   0},   /*   1600/0   Hz,   20,      - */
    {1800,   0},   /*   1800/0   Hz,   21,      - */
    {2100,   0},   /*   2100/0   Hz,   22,      - */ /* --> CED Tone */
    {2300,   0},   /*   2300/0   Hz,   23,      - */
    {2450,   0},   /*   2450/0   Hz,   24,      - */
    {350,  440},   /*   350/440  Hz,   25,      - */  /* --> Dial Tone */
    {440,  480},   /*   440/480  Hz,   26,      - */  /* --> Ring Back */
    {480,  620},   /*   480/620  Hz,   27,      - */  /* --> Busy Tone */
    {697, 1633},   /*   697/1633 Hz,   28,      A */
    {770, 1633},   /*   770/1633 Hz,   29,      B */
    {852, 1633},   /*   852/1633 Hz,   30,      C */
    {941, 1633},   /*   941/1633 Hz,   31,      D */
};


/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t ComplexTone_Conf (TAPI_TONE_DATA_t *pData,
                                     COMPLEX_TONE *toneCoeffs,
                                     COMPLEX_TONE const *pTone);
static IFX_void_t Event_SimpleTone   (TAPI_CHANNEL *pChannel,
                                      IFX_uint8_t nResID);
static IFX_void_t Event_ComposedTone (TAPI_CHANNEL *pChannel,
                                      IFX_uint8_t nResID);
static IFX_int32_t TAPI_DECT_Tone_Stop_Unprot(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                              TAPI_CHANNEL *pChannel);
IFX_int32_t TAPI_Phone_Tone_Stop_Unprot (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                         TAPI_CHANNEL *pChannel,
                                         IFX_int32_t nToneIndex,
                                         TAPI_TONE_DST nDirection);


/**
   If the LL function for resource information is not available
   this function is called to set the resource ID by using the parameters
   src and dst.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param src Tone source as defined in IFX_TAPI_TONE_SRC_t. Must be given
   by the calling function. If set to 0 a signaling module connected to
   a coder is taken as source.
   by the calling function
   \param dst Value defined in TAPI_TONE_DST. Direction where to play the
              tone: local or network

   \param pRes The resource information of the tone generator. The structure
               is filled in this function.
   \return
   - TAPI_statusInvalidToneRes If the resource can not support a requested
     feature. This is the case when a resource supports only local tones and
     the destination TAPI_TONE_DST_NET was requested.
   - TAPI_statusLLNotSupp LL driver does not support any tone playing
   - TAPI_statusOk if the resource was successfully specified.

   \remarks
   This function is called when protection is needed for
   TAPI_Phone_Tone_Play_Unprot
*/
static IFX_int32_t TAPI_Tone_ResIdGet (IFX_TAPI_DRV_CTX_t *pDrvCtx,
               TAPI_CHANNEL* pChannel, IFX_int32_t src, TAPI_TONE_DST dst,
               IFX_TAPI_TONE_RES_t *pRes)
{
   /* select which TAPI data storage to use */
   /* current assignment is still a mix of destination and resource
      default is 0 */
   if ((src & IFX_TAPI_TONE_SRC_TG) &&
       (pDrvCtx->ALM.TG_Play != IFX_NULL))
   {
      pRes->nResID = 1;
      pRes->sequenceCap = IFX_TAPI_TONE_RESSEQ_FREQ;
   }
   else if (src & IFX_TAPI_TONE_SRC_DECT)
   {
      pRes->nResID = 2;
      pRes->sequenceCap = IFX_TAPI_TONE_RESSEQ_SIMPLE;
   }
   else
   {
      pRes->nResID = 0;
      pRes->sequenceCap = IFX_TAPI_TONE_RESSEQ_SIMPLE;
      if (pDrvCtx->SIG.UTG_Start != IFX_NULL)
      {
         /* default res = 0 is used or different one if 2 UTGs availalble */
         if ((pDrvCtx->SIG.UTG_Count_Get != IFX_NULL) &&
              pDrvCtx->SIG.UTG_Count_Get (pChannel->pLLChannel) == 2)
         {
            /* When there are two UTG per channel we bind them fix to directions */
            if (dst != TAPI_TONE_DST_NET)
               /* use UTG 2 to play tones in local direction  */
               pRes->nResID = 1;
         }
      }
      else if (pDrvCtx->ALM.TG_Play != IFX_NULL)
      {
         /* Because the source for playing the tone might not be coded in
            nToneIndex, add an additional check here if the tone generator is
            the source */
         switch (dst)
         {
            case TAPI_TONE_DST_LOCAL:
               pRes->nResID      = 1;
               pRes->sequenceCap = IFX_TAPI_TONE_RESSEQ_FREQ;
               break;
            default:
               RETURN_STATUS (TAPI_statusInvalidToneRes, 0);
         }
      }
      else
      {
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);
      }
   }
   return TAPI_statusOk;
}

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Configures predefined tones of Index 1 to 31
\param pToneTable  Handle to COMPLEX_TONE structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   Not a low level function
   Constant Table TAPI_PredefinedToneFreq defines the frequencies used for
   the predefined tones in index range 1 to 31. In case this table is changed
   in the future, this function must be adapted accordingly.
   The actual table status is :
      - Either Freq2 > Freq 1 => DTMF,
      - or Freq2 = 0 (e.g CNG tone)
\todo This is not a low level function. The frequencies are not
      related to the device.
*/
IFX_return_t TAPI_Phone_Tone_Predef_Config (COMPLEX_TONE* pToneTable)
{
   IFX_TAPI_TONE_SIMPLE_t predef_tone;
   IFX_uint32_t i;


   /* setup tone structure for predefined tone */
   memset (&predef_tone, 0, sizeof (predef_tone));
   /* add all predefined tones to tone table */
   for (i = 0; i < (IFX_TAPI_TONE_INDEX_MIN - 1); i++)
   {
      /* read frequencies from contant table */
      predef_tone.freqA       = TAPI_PredefinedToneFreq[i][0];
      predef_tone.freqB       = TAPI_PredefinedToneFreq[i][1];
      /* tone indexes start from 1 on */
      predef_tone.index       = i + 1 ;
      /* set frequencies to play and levels */
      switch (predef_tone.freqB)
      {
         case  0:
            /* play only freqA */
            predef_tone.frequencies [0] = IFX_TAPI_TONE_FREQA;
            /* -9 dB = -90 x 0.1 dB for the higher frequency */
            predef_tone.levelA          = -90;
            predef_tone.levelB          = 0;
            break;
         default:
            /* In this case, freqB > freqA according to table
               TAPI_PredefinedToneFreq : Play both frequencies as dtmf  */
            predef_tone.frequencies [0] = (IFX_TAPI_TONE_FREQA |
                                           IFX_TAPI_TONE_FREQB);
            /* -11 dB = -110 x 0.1 dB for the lower frequency ,
               -9  dB = -90  * 0.1 dB for the higher frequency
                        (to attenuate the higher tx loss) */
            predef_tone.levelA          = -110;
            predef_tone.levelB          = -90;
            break;
      }
      /* set cadences and loop */
      switch (predef_tone.index)
      {
         case 17:  /* CNG */
         case 22:  /* CED */
            predef_tone.cadence [0] = 1000;
            predef_tone.cadence [1] = 0;
            /* tone played continuously */
            predef_tone.loop = 0;
            break;
         case 25:  /* (dialtone) */
            predef_tone.cadence [0] = 1000;
            predef_tone.cadence [1] = 0;
            /* tone played continuously */
            predef_tone.loop = 0;
            break;
         case 26:  /* (ringback tone) */
            predef_tone.cadence [0] = 1000;
            predef_tone.cadence [1] = 4000;
            /* tone played continuously */
            predef_tone.loop = 0;
            break;
         case 27:  /* (busy tone) */
            predef_tone.cadence [0] = 500;
            predef_tone.cadence [1] = 500;
            /* tone played continuously */
            predef_tone.loop = 0;
            break;
         default:
            /* all other tones will be played only once for 100 ms */
            predef_tone.cadence [0] = 100;
            predef_tone.cadence [1] = 0;
            /* tone played only once */
            predef_tone.loop = 1;
            break;
      }
      if (TAPI_Phone_Add_SimpleTone(pToneTable, &predef_tone) == IFX_ERROR)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("Predefined tone configuration"
                " failed for tone index %d\n\r", predef_tone.index));
         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}

/**
   Sets the on-time for a tone with a specified duration.

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param nTime           - Time in 0.25ms

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Set_On_Time(TAPI_CHANNEL *pChannel,
                                        IFX_uint32_t nTime)
{
   /* set time only when no tone is played */
   if (pChannel->TapiTgToneData.nToneState != 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: could not set on-time "
          "for channel %d\n\r", pChannel->nChannel));
      return IFX_ERROR;
   }

   pChannel->TapiTgToneData.nOnTime = nTime;

   return IFX_SUCCESS;
}

/**
   Sets the off-time for a tone with a specified duration.

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param nTime           - Time in 0.25ms

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Set_Off_Time(TAPI_CHANNEL *pChannel,
                                         IFX_uint32_t nTime)
{
   /* set time only when no tone is played */
   if (pChannel->TapiTgToneData.nToneState != 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: could not set off-time "
          "for channel %d\n\r", pChannel->nChannel));
      return IFX_ERROR;
   }

   pChannel->TapiTgToneData.nOffTime = nTime;

   return IFX_SUCCESS;
}

/**
   Gets the on-time.

   \param pChannel        - handle to TAPI_CHANNEL
   \param pOnTime         - Tone On Time in 0.25ms

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_Tone_Get_On_Time(TAPI_CHANNEL *pChannel,
                                        IFX_uint32_t *pOnTime)
{
   *pOnTime = pChannel->TapiTgToneData.nOnTime;
   return IFX_SUCCESS;
}


/**
   Gets the off-time.

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pOffTime        - Tone Off Time in 0.25ms

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_Tone_Get_Off_Time(TAPI_CHANNEL *pChannel,
                                         IFX_uint32_t *pOffTime)
{
   *pOffTime = pChannel->TapiTgToneData.nOffTime;
   return IFX_SUCCESS;
}

/**
   Sets the tone level of tone currently played

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel        - handle to TAPI_CHANNEL structure
   \param nToneIndex      - index in the tone table

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Set_Level(IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel,
                                      IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel)
{
   IFX_int32_t ret = IFX_SUCCESS;

   if (pDrvCtx->SIG.UTG_Level_Set != NULL)
   {
      ret = pDrvCtx->SIG.UTG_Level_Set (pChannel->pLLChannel, pToneLevel);
   }
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("DRV_ERROR: Tone Set Level failed low level support not there!! \n\r"));
      ret = IFX_ERROR;
   }

   return ret;
}

/**
   Gets the tone playing state.

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pToneState      - 0: no tone is played
                     1: tone is played (on-time)
                     2: silence (off-time)
   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_Tone_Get_State(TAPI_CHANNEL *pChannel,
                                      IFX_uint32_t *pToneState)
{
   *pToneState = pChannel->TapiTgToneData.nToneState;
   return IFX_SUCCESS;
}

/**
   Add simple tone to internal tone coefficients table

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pSimpleTone     - entry for internal tone table

   \return:
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Add_SimpleTone(COMPLEX_TONE* toneCoefficients,
                                      IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone)
{
   IFX_TAPI_TONE_SIMPLE_t *pToneCfg = IFX_NULL;
   IFX_uint32_t i;

   if (pSimpleTone->index >= TAPI_MAX_TONE_CODE || pSimpleTone->index == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Simple tone code out of range\n\r"));
      return IFX_ERROR;
   }
   /* check on time to set */
   for (i=0; i < IFX_TAPI_TONE_STEPS_MAX; ++i)
   {
      if (pSimpleTone->cadence[i] > MAX_TONE_TIME)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Max on time "
               "time is %d ms\n\r", MAX_TONE_TIME));
         return IFX_ERROR;
      }
   }
   /* check for zero time on frequency A */
   if (pSimpleTone->cadence[0] == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Max off/on time "
            "for first step may not be zero\n\r"));
      return IFX_ERROR;
   }
   /* check pause time to set */
   if (pSimpleTone->pause > MAX_CADENCE_TIME)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Max pause time "
            "is %d ms\n\r", MAX_CADENCE_TIME));
      return IFX_ERROR;
   }

   /* check frequencies to set */
   if (pSimpleTone->freqA > MAX_TONE_FREQ ||
       pSimpleTone->freqB > MAX_TONE_FREQ ||
       pSimpleTone->freqC > MAX_TONE_FREQ ||
       pSimpleTone->freqD > MAX_TONE_FREQ)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Max frequency "
            "is %d Hz\n\r", MAX_TONE_FREQ));
      return IFX_ERROR;
   }
   /* check power level to set */
   if (pSimpleTone->levelA < MIN_TONE_POWER ||
       pSimpleTone->levelA > MAX_TONE_POWER ||
       pSimpleTone->levelB < MIN_TONE_POWER ||
       pSimpleTone->levelB > MAX_TONE_POWER ||
       pSimpleTone->levelD < MIN_TONE_POWER ||
       pSimpleTone->levelD > MAX_TONE_POWER ||
       pSimpleTone->levelC < MIN_TONE_POWER ||
       pSimpleTone->levelC > MAX_TONE_POWER)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Min power "
            "level is %d Max power level is %d\n\r", MIN_TONE_POWER,
            MAX_TONE_POWER));
      return IFX_ERROR;
   }
   /* Set ptr to tone to be configured */
   pToneCfg = &toneCoefficients[pSimpleTone->index].tone.simple;
   /* Add entry to internal tone table */
   toneCoefficients[pSimpleTone->index].type = TAPI_TONE_TYPE_SIMPLE;
   /* Is the tone out of predefined tone range ? */
   if (pSimpleTone->index >= IFX_TAPI_TONE_INDEX_MIN)
   {
      *pToneCfg = *pSimpleTone;
   }
   else
   {
      /* add tone to tone table in case the tone if configured the first time */
      if (pToneCfg->frequencies [0] == IFX_TAPI_TONE_FREQNONE)
      {
         *pToneCfg = *pSimpleTone;
      }
      else
      {
         /* for predefined tones, adapt only cadence and level */
         pToneCfg->levelA      = pSimpleTone->levelA;
         pToneCfg->levelB      = pSimpleTone->levelB;
         /* tone on time */
         pToneCfg->cadence [0] = pSimpleTone->cadence [0];
         /* tone off time */
         pToneCfg->cadence [1] = pSimpleTone->cadence [1];
         /* set loop. 0 means continuous tone */
         pToneCfg->loop        = pSimpleTone->loop;
      }
   }

   return IFX_SUCCESS;
}

/**
   Add composed tone to internal tone coefficients table

   \param toneCoefficients - Pointer to Complex tone structure
   \param pComposedTone   - entry for internal tone table

   \return:
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Add_ComposedTone (COMPLEX_TONE* toneCoefficients,
                                         IFX_TAPI_TONE_COMPOSED_t const *pComposedTone)
{
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint32_t i;

   if (pComposedTone->index >= TAPI_MAX_TONE_CODE || pComposedTone->index == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Composed tone code out of range\n\r"));
      return IFX_ERROR;
   }

   /* check alternate voice time to set */
   if (pComposedTone->alternatVoicePath > MAX_VOICE_TIME)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Max alternate voice "
            "time is %d ms\n\r", MAX_VOICE_TIME));
      return IFX_ERROR;
   }

   /* check loop count against alternate voice time */
   if (pComposedTone->loop == 1 && pComposedTone->alternatVoicePath != 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Single repetition "
            "not allowed when alternate voice time is non-zero\n\r"));
      return IFX_ERROR;
   }

   /* Validate the number of simple tones allowed within composed tone */
   if (pComposedTone->count == 0 ||
       pComposedTone->count > IFX_TAPI_TONE_SIMPLE_MAX)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Number of simple tone fields is out of range\n\r"));
      return IFX_ERROR;
   }

   /* Validate if all simple tone codes are configured within the composed tone */
   for (i = 0; i < pComposedTone->count; i++)
   {
      if (pComposedTone->tones[i] >= TAPI_MAX_TONE_CODE ||
         pComposedTone->tones[i] == 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Simple tone code within composed tone is out of range\n\r"));
         return IFX_ERROR;
      }
      if (toneCoefficients[pComposedTone->tones[i]].type == TAPI_TONE_TYPE_NONE)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Simple tone code %d within composed tone not configured\n\r",
            pComposedTone->tones[i]));
         return IFX_ERROR;
      }
   }

   /* Add entry to internal tone table */
   toneCoefficients[pComposedTone->index].type          = TAPI_TONE_TYPE_COMP;
   memcpy ((char *)&toneCoefficients[pComposedTone->index].tone.composed,
           (char *)pComposedTone, sizeof(IFX_TAPI_TONE_COMPOSED_t));

   return ret;
}

/**
   Add tone to internal tone coefficients table

   \param toneCoefficients - pointer to Complex tone structure
   \param pTone    - entry for internal tone table

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_TableConf (COMPLEX_TONE* toneCoefficients,
                                       IFX_TAPI_TONE_t const *pTone)
{
   IFX_int32_t ret = IFX_ERROR;
   switch (pTone->simple.format)
   {
      case  IFX_TAPI_TONE_TYPE_SIMPLE:
         ret = TAPI_Phone_Add_SimpleTone(toneCoefficients, &pTone->simple);
      break;
      case  IFX_TAPI_TONE_TYPE_COMPOSED:
         ret = TAPI_Phone_Add_ComposedTone(toneCoefficients, &pTone->composed);
      break;
      default:
         /* error */
         break;
   }
   return ret;
}

/**
   Play complex (simple or composed) tone

   \param pChannel         - handle to TAPI_CHANNEL structure
   \param pToneCoeffTable  - reference to TAPI complex tone table
   \param nToneCodeCoeff   - reference to tone to be played

   \return:
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t ComplexTone_Conf(TAPI_TONE_DATA_t * pData,
                                    COMPLEX_TONE *pToneCoeffTable,
                                    COMPLEX_TONE const *pToneCoeff)
{
   IFX_uint32_t firstToneCode = 0;

   /* Setup the tone code type */
   pData->nType = pToneCoeff->type;

   /* This is a composed tone so get the first simple tone sequence to play */
   if (pToneCoeff->type == TAPI_TONE_TYPE_COMP)
   {
      /* Retrieve composed configuration from internal tone coefficients table */
      pData->nComposedMaxReps    = pToneCoeff->tone.composed.loop;
      pData->nComposedCurrReps   = 0;
      pData->nAlternateVoiceTime = pToneCoeff->tone.composed.alternatVoicePath;
      pData->nComposedToneCode   = pToneCoeff->tone.composed.index;
      pData->nMaxToneCount       = pToneCoeff->tone.composed.count;
      pData->nToneCounter        = 0;

      /* First simple tone sequence to play */
      firstToneCode = pToneCoeff->tone.composed.tones[0];

      /* Use the simple tone code as an index into internal tone coefficients table */
      pToneCoeff = &pToneCoeffTable[firstToneCode];
   }

   /* Retrieve simple tone configuration from internal tone coefficients table */
   pData->nSimpleMaxReps      = pToneCoeff->tone.simple.loop;
   pData->nSimpleCurrReps     = 0;
   pData->nPauseTime          = pToneCoeff->tone.simple.pause;
   pData->nSimpleToneCode     = pToneCoeff->tone.simple.index;

   return IFX_SUCCESS;
}

/**
   Detects a tone based on the complex (simple or composed) tone

   \param pDrvCtx     - pointer to low-level driver context
   \param pChannel    - handle to TAPI_CHANNEL structure
   \param signal      - tone code reference for internal tone table

   \return:
   IFX_SUCCESS or IFX_ERROR
   \remark
   The function returns an error, when the tone is not previously defined
*/
IFX_int32_t TAPI_Phone_DetectToneStart (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                        TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_TONE_CPTD_t const *signal)
{
   COMPLEX_TONE *pToneCoeff;
   IFX_int32_t ret = IFX_SUCCESS;
   TAPI_DEV* pTapiDev = pChannel->pTapiDevice;

   /* Use the tone code as an index into internal tone coefficients table */
   pToneCoeff = &(pTapiDev->pToneTbl[signal->tone]);

   /* check if tone code is configured */
   if (pToneCoeff->type == TAPI_TONE_TYPE_NONE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Reference to unconfigured tone code entry\n\r"));
      return IFX_ERROR;
   }
   if (signal->tone >= TAPI_MAX_TONE_CODE || signal->tone <= 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Detect complex tone code out of range\n\r"));
      return IFX_ERROR;
   }
   if (pToneCoeff->type == TAPI_TONE_TYPE_COMP)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Reference to unsupported tone code entry\n\r"));
      return IFX_ERROR;
   }

   if (ptr_chk(pDrvCtx->SIG.CPTD_Start, "pDrvCtx->SIG.CPTD_Start"))
      ret = pDrvCtx->SIG.CPTD_Start (pChannel->pLLChannel,
                                     &pToneCoeff->tone.simple,
                                     signal->signal);

   if (ret == IFX_SUCCESS)
   {
      /* store it to the signals */
      if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Enable,
                 "pDrvCtx->SIG.MFTD_Signal_Enable"))
         pDrvCtx->SIG.MFTD_Signal_Enable (pChannel->pLLChannel,
                                          IFX_TAPI_SIG_CPTD);
   }
   return ret;
}

/**
   Stops the detection a tone based on the complex (simple or composed) tone

   \param pDrvCtx     - pointer to low-level driver context
   \param pChannel    - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS
*/
IFX_int32_t TAPI_Phone_DetectToneStop (IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel)
{
   /* removed it from the signals */
   if (ptr_chk(pDrvCtx->SIG.MFTD_Signal_Disable,
              "pDrvCtx->SIG.MFTD_Signal_Disable"))
      pDrvCtx->SIG.MFTD_Signal_Disable (pChannel->pLLChannel, IFX_TAPI_SIG_CPTD);

   if (ptr_chk(pDrvCtx->SIG.CPTD_Stop, "pDrvCtx->SIG.CPTD_Stop"))
      pDrvCtx->SIG.CPTD_Stop (pChannel->pLLChannel);

   return IFX_SUCCESS;
}

/**
   Plays a dial tone.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Dial(IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret;

   /* set times for dial tone */
   pChannel->TapiTgToneData.nOnTime  = 0xFFFFFF;
   pChannel->TapiTgToneData.nOffTime = 0;

   /* play dial tone */
   ret = TAPI_Phone_Tone_Play(pDrvCtx, pChannel, 25, TAPI_TONE_DST_LOCAL);

   return ret;
}

/**
   Plays a ringback tone.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Ringback(IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret;

   /* set times for dial tone */
   pChannel->TapiTgToneData.nOnTime  = 4000;
   pChannel->TapiTgToneData.nOffTime = 16000;
   ret = TAPI_Phone_Tone_Play (pDrvCtx, pChannel, 26, TAPI_TONE_DST_LOCAL);

   return ret;
}

/**
   Plays a busy tone.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure

   \return:
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Busy(IFX_TAPI_DRV_CTX_t *pDrvCtx, TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret;

   /* set times for busy tone */
   pChannel->TapiTgToneData.nOnTime  = 2000;
   pChannel->TapiTgToneData.nOffTime = 2000;

   ret = TAPI_Phone_Tone_Play (pDrvCtx, pChannel, 27, TAPI_TONE_DST_LOCAL);

   return ret;
}

/**
   Does the protection before the function for playing a tone is called.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param nToneIndex   - index in the tone table
   \param dst          - direction where to play the tone: local or network

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   This function is called when protection is needed for
   TAPI_Phone_Tone_Play_Unprot
*/
IFX_int32_t TAPI_Phone_Tone_Play(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                      TAPI_CHANNEL *pChannel,
                                      IFX_int32_t nToneIndex,
                                      TAPI_TONE_DST dst)
{
   IFX_int32_t    err = IFX_SUCCESS;

   /* make sure the resource is not in use before allocating it */
   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   err = TAPI_Phone_Tone_Play_Unprot (pDrvCtx, pChannel, nToneIndex, dst);
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return err;
}

/**
   Plays a tone from the tone table.

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param nToneIndex   - index in the tone table
   \param dst          - direction where to play the tone: local or network

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Play_Unprot (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                         TAPI_CHANNEL *pChannel,
                                         IFX_int32_t nToneIndex,
                                         TAPI_TONE_DST dst)
{
   TAPI_DEV *pTapiDev = pChannel->pTapiDevice;
   IFX_int32_t ret = IFX_SUCCESS;
   COMPLEX_TONE *pToneCoeff;
   IFX_TAPI_TONE_SIMPLE_t *pToneSimple;
   IFX_int32_t src;
   /*IFX_TAPI_LL_TONE_DIR_t toneDir;*/
   IFX_TAPI_TONE_RES_t ToneRes;
   IFX_int32_t nSimpleToneIndex;
   TAPI_TONE_DATA_t *pData;

   memset (&ToneRes, 0, sizeof(IFX_TAPI_TONE_RES_t ));
   /* Filter out the source for playing out the tone from parameter nToneIndex.
      Note: The parameter nToneIndex is not well documented in TAPI document for
      this purpose. Therefore it is also checked if the LL exclusively provides
      a function pointer for tone generator usage. In this case the appropriate
      ressource is selected.  */
   src = nToneIndex & (IFX_TAPI_TONE_SRC_TG |
                       IFX_TAPI_TONE_SRC_DSP | IFX_TAPI_TONE_SRC_DECT);
   nToneIndex &=     ~(IFX_TAPI_TONE_SRC_TG |
                       IFX_TAPI_TONE_SRC_DSP | IFX_TAPI_TONE_SRC_DECT);

   /* check tone index parameter */
   if (nToneIndex >= TAPI_MAX_TONE_CODE || nToneIndex < 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Play tone code out of range\n\r"));
      return IFX_ERROR;
   }

   /* Tone index == 0 is stop */
   if (nToneIndex == 0)
   {
      /* if index is 0 stop all tone generators in this direction */
      if (src & IFX_TAPI_TONE_SRC_DECT)
         ret = TAPI_DECT_Tone_Stop_Unprot (pDrvCtx, pChannel);
      else
         ret = TAPI_Phone_Tone_Stop_Unprot (pDrvCtx, pChannel, 0, dst);

      return ret;
   }

   /**\todo Shift to LL */
   /* gets a resource ID and sets the module type for the resource */
   ret = TAPI_Tone_ResIdGet (pDrvCtx, pChannel, src, dst, &ToneRes);
   /* get the right resource from LL for playing out this tone if possible */
   if (pDrvCtx->SIG.ToneGen_ResIdGet != IFX_NULL)
   {
      ret = pDrvCtx->SIG.ToneGen_ResIdGet (pChannel->pLLChannel,
               0, dst, &ToneRes);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusToneNoRes, ret);
   }

   /*ret = TAPI_Tone_Start (pDrvCtx, pChannel, nResID, &tone);*/

   /* Use the tone code as an index into internal tone coefficients table */
   pToneCoeff = &(pTapiDev->pToneTbl[nToneIndex]);
   /* Check the kind of tone and get the index of the first simple tone */
   switch (pToneCoeff->type)
   {
   case TAPI_TONE_TYPE_COMP:
      /* For composed tones fetch the first simple tone */
      nSimpleToneIndex = pToneCoeff->tone.composed.tones[0];
      break;
   case TAPI_TONE_TYPE_SIMPLE:
      /* For simple tones the index already points to a simple tone */
      nSimpleToneIndex = nToneIndex;
      break;
   case TAPI_TONE_TYPE_NONE:
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Reference to unconfigured tone code entry\n\r"));
      return IFX_ERROR;
   default:
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Unsupported tone entry on index:%d\n\r", nToneIndex));
      return IFX_ERROR;
   }


   /* retrieve simple tone from internal tone coefficients table */
   pToneSimple = &(pTapiDev->pToneTbl[nSimpleToneIndex].tone.simple);

   pData = &pChannel->TapiComplexToneData[ToneRes.nResID];
   if ((pData->nToneState == TAPI_CT_IDLE) ||
       (pData->nToneState == TAPI_CT_DEACTIVATED))
   {
      /* preparation for complex tones */
      ComplexTone_Conf (pData, pTapiDev->pToneTbl, pToneCoeff);

      /* Try to play the first simple tone */
      if ((src & IFX_TAPI_TONE_SRC_DECT) &&
          ptr_chk(pDrvCtx->DECT.UTG_Start, "pDrvCtx->DECT.UTG_Start"))
      {
         /* Play the first simple tone */
         ret = pDrvCtx->DECT.UTG_Start (pChannel->pLLChannel,
                                        pToneSimple, dst, 0);
      }
      else if (ptr_chk(pDrvCtx->SIG.UTG_Start, "pDrvCtx->SIG.UTG_Start"))
      {
         /* Play the first simple tone */
         ret = pDrvCtx->SIG.UTG_Start (pChannel->pLLChannel,
                                       pToneSimple, dst, ToneRes.nResID);
      }
      /* if no UTG is available play tone on TG (if available) */
      else if (ptr_chk(pDrvCtx->ALM.TG_Play, "pDrvCtx->ALM.TG_Play"))
      {
         /* Play the first simple tone */
         ret = pDrvCtx->ALM.TG_Play (pChannel->pLLChannel,
                                     ToneRes.nResID, pToneSimple, dst);
         /*
            start the timer for the first cadence only when it is not a
            continuously played tone (loop = 0 and cadence[1] = 0)
         */
         if ((ret == IFX_SUCCESS) &&
            ((pToneSimple->cadence[1] != 0) ||
            (pToneSimple->loop != 0)))
         {
            /* start the timer for the first cadence of the simple tone. */
            TAPI_SetTime_Timer(pChannel->pToneRes[ToneRes.nResID].Tone_Timer,
                               pToneSimple->cadence[0],
                               IFX_FALSE, IFX_TRUE);
         }
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("DRV_ERROR: No low level support for playing tone\n\r"));
         ret = IFX_ERROR;
      }


      if (TAPI_SUCCESS(ret))
      {  /* store the information */
         pData->dst = dst;
         pData->sequenceCap = ToneRes.sequenceCap;
         pData->nToneIndex = nToneIndex;
         pData->nToneState = TAPI_CT_ACTIVE;
      }
      else
      {
         /* errmsg: Playing tone in LL driver failed */
         RETURN_STATUS (TAPI_statusTonePlayLLFailed, ret);
      }
   }
   else
   {
      /* tone is already playing */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Tone resource is already playing a tone\n\r"));
      ret = IFX_ERROR;
   }

   if (ret != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("DRV_ERROR: Tone play failed\n\r"));
   }

   return ret;
}

/**
   Stops Tone Generation, only in connection with "IFX_TAPI_TONE_STOP"

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param nToneIndex   - tone to be stopped
   \param nDirection   - direction into which tone is played

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_Phone_Tone_Stop(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                 TAPI_CHANNEL *pChannel,
                                 IFX_int32_t nToneIndex,
                                 TAPI_TONE_DST nDirection)
{
   IFX_int32_t ret = IFX_ERROR;

   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   ret = TAPI_Phone_Tone_Stop_Unprot(pDrvCtx,pChannel,nToneIndex,nDirection);
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return (ret);
}

/**
   Stops Tone Generation, only in connection with "IFX_TAPI_TONE_STOP"

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param nToneIndex   - tone to be stopped
   \param nDirection   - direction into which tone is played

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks No protection against concurrent access. If you need protection
            TAPI_Phone_Tone_Stop must be called.
*/
IFX_int32_t TAPI_Phone_Tone_Stop_Unprot (IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                         TAPI_CHANNEL *pChannel,
                                         IFX_int32_t nToneIndex,
                                         TAPI_TONE_DST nDirection)
{
   TAPI_TONE_DATA_t *pData;
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_uint8_t nResID = 0;
   IFX_int32_t ret = IFX_ERROR;

   while (nResID < TAPI_TONE_MAXRES)
   {
      /* Skip DECT resouce as it is handled in a separate function below */
      if (nResID == 2)
      {
         nResID++;
         continue;
      }
      /* get tone definition */
      pData = &pChannel->TapiComplexToneData[nResID];
      /* stop given tone index played into the given direction
         a direction of TAPI_TONE_DST_DEFAULT means don't care for the direction
         if no index is specified every tone currently played is stopped */
      if ((nToneIndex != 0 && pData->nToneIndex == nToneIndex &&
           (nDirection == TAPI_TONE_DST_DEFAULT || pData->dst == nDirection)) ||
          (nToneIndex == 0 && pData->nToneState != TAPI_CT_IDLE &&
           (nDirection == TAPI_TONE_DST_DEFAULT || pData->dst == nDirection)))
      {
         if (pData->nToneState == TAPI_CT_IDLE)
            return TAPI_statusOk;
         /* Set tone generation back to initial state */
         pData->nToneState = TAPI_CT_IDLE;

         /* Stop any voice path teardowm timer that may be running */
         TAPI_Stop_Timer (pChannel->pToneRes[nResID].Tone_Timer);

         if (ptr_chk(pDrvCtx->SIG.UTG_Stop, "pDrvCtx->SIG.UTG_Stop"))
         {
            ret = pDrvCtx->SIG.UTG_Stop(pChannel->pLLChannel, nResID);
         }
         else if (ptr_chk(pDrvCtx->ALM.TG_Stop, "pDrvCtx->ALM.TG_Stop"))
         {
            ret = pDrvCtx->ALM.TG_Stop(pChannel->pLLChannel, nResID);
         }
         if (!(TAPI_SUCCESS (ret)))
            RETURN_STATUS (TAPI_statusToneStop, ret);

         /* issue TAPI event to the application */
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_TONE_GEN_END;
         switch (pData->dst)
         {
            case TAPI_TONE_DST_DEFAULT:
            case TAPI_TONE_DST_LOCAL:
               tapiEvent.data.tone_gen.local = 1;
               break;
            case TAPI_TONE_DST_NET:
               tapiEvent.data.tone_gen.network = 1;
               break;
            case TAPI_TONE_DST_NETLOCAL:
               tapiEvent.data.tone_gen.local = 1;
               tapiEvent.data.tone_gen.network = 1;
               break;
         }
         tapiEvent.data.tone_gen.index = pData->nToneIndex;
         IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);

         /* reset index afterwards anyway */
         pData->nToneIndex = 0;
      }
      nResID++;
   }

   return (ret);
}

/**
   Stops Tone Generation on a DECT channel

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t TAPI_DECT_Tone_Stop(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = IFX_ERROR;

   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   ret = TAPI_DECT_Tone_Stop_Unprot(pDrvCtx,pChannel);
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);

   return (ret);
}

/**
   Stops Tone Generation on a DECT channel

   \param pDrvCtx      - pointer to low-level device driver context
   \param pChannel     - handle to TAPI_CHANNEL structure

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks No protection against concurrent access. If you need protection
            TAPI_DECT_Tone_Stop must be called.
*/
static IFX_int32_t TAPI_DECT_Tone_Stop_Unprot(IFX_TAPI_DRV_CTX_t *pDrvCtx,
                                              TAPI_CHANNEL *pChannel)
{
   TAPI_TONE_DATA_t *pData;
   IFX_int32_t res = 2; /* DECT is always resource 2 by definition */
   IFX_int32_t ret = IFX_ERROR;

   /* Stop any voice path teardowm timer that may be running */
   TAPI_Stop_Timer (pChannel->pToneRes[res].Tone_Timer);

   if (ptr_chk(pDrvCtx->DECT.UTG_Stop, "pDrvCtx->DECT.UTG_Stop"))
   {
      ret = pDrvCtx->DECT.UTG_Stop(pChannel->pLLChannel, 0);
   }

   /* Get tone definition */
   pData = &pChannel->TapiComplexToneData[res];
   /* Set tone generation back to initial state */
   pData->nToneState = TAPI_CT_IDLE;
   /* Reset index afterwards anyway */
   pData->nToneIndex = 0;

   return (ret);
}
/*
 * Tone State machine
 */
/**
   Function called from the teardown voice path timer.
   The timer expiry indicates the following:
     - Stop the voice path recording(teardown) in order for the next
       tone playing sequence step to be executed on that channel
\param
   Timer  - TimerID of timer that exipres
\param
   nArg   - Argument of timer including the VINETIC_CHANNEL structure
           (as integer pointer)
\return
*/
IFX_void_t Tone_OnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_TONERES *pRes = (TAPI_TONERES*)nArg;
   TAPI_CHANNEL *pChannel = pRes->pTapiCh;
   TAPI_DEV     *pTapiDev = pChannel->pTapiDevice;
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = pTapiDev->pDevDrvCtx;

   IFX_uint32_t toneCode           = 0;
   IFX_TAPI_TONE_SIMPLE_t *pToneSimple;
   TAPI_TONE_DATA_t *pData;

   IFX_int8_t  nToneStep;
   IFX_int32_t ret = IFX_ERROR;

   IFXOS_MutexLock(pChannel->semTapiChDataLock);
   pData = &pChannel->TapiComplexToneData[pRes->nRes];

   /* check if mailbox supports all commands, otherwise return immediately */
   if (pData->sequenceCap == IFX_TAPI_TONE_RESSEQ_SIMPLE)
   {
      IFX_uint8_t nCmdMbxSize;
      IFX_TAPI_LL_DEV_t *pDev = pTapiDev->pLLDev;

      if (ptr_chk(pDrvCtx->GetCmdMbxSize, "pDrvCtx->GetCmdMbxSize"))
      {
         if(pDrvCtx->GetCmdMbxSize (pDev, &nCmdMbxSize) == IFX_SUCCESS)
         {
            if (nCmdMbxSize < MAX_CMD_WORD)
            {
               TAPI_SetTime_Timer(pChannel->pToneRes[pRes->nRes].Tone_Timer, 1,
                                  IFX_FALSE, IFX_FALSE);
               goto exit;
            }
         }
      }
   }

   /* Get the simple tone code */
   if (pData->nType == TAPI_TONE_TYPE_SIMPLE)
   {
      toneCode = pData->nSimpleToneCode;
   }
   else if (pData->nType == TAPI_TONE_TYPE_COMP)
   {
      /* Get the simple tone to play within the composed tone */
      IFX_TAPI_TONE_COMPOSED_t *pComposedTone;

      pComposedTone = &pTapiDev->pToneTbl[pData->nComposedToneCode].
                      tone.composed;
      toneCode = pComposedTone->tones[pData->nToneCounter];
   }

   /* Any errors on the simple tone code index is taken care of here */
   if (toneCode >= TAPI_MAX_TONE_CODE || toneCode == 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Repeat complex tone code out of range\n\r"));
      goto exit;
   }

   /* Retrieve simple tone configuration from internal tone coefficients table */
   pToneSimple = &(pTapiDev->pToneTbl[toneCode].tone.simple);
   pData->nSimpleMaxReps      = pToneSimple->loop;
   pData->nPauseTime          = pToneSimple->pause;
   pData->nSimpleToneCode     = pToneSimple->index;

   if (pData->sequenceCap == IFX_TAPI_TONE_RESSEQ_FREQ)
   {
      /* when the next simple tone in a composed tone must be played, call
         the low level function to set up the tone */
      if ((pData->nToneState == TAPI_CT_IDLE) &&
          (pData->nType == TAPI_TONE_TYPE_COMP))
      {
         /* tone has finished */
         if (ptr_chk(pDrvCtx->ALM.TG_Play, "pDrvCtx->ALM.TG_Play"))
            ret = pDrvCtx->ALM.TG_Play (pChannel->pLLChannel, pRes->nRes,
                                  pToneSimple, TAPI_TONE_DST_LOCAL);

         if (ret == IFX_SUCCESS)
         {  /* store the information */
            pData->dst = TAPI_TONE_DST_LOCAL;   /* must be TAPI_TONE_DST_LOCAL */
            pData->nToneIndex = toneCode;
            pData->nToneState = TAPI_CT_ACTIVE;
            /* In case of a simple tone start the timer for the first cadence.*/
            TAPI_SetTime_Timer(pChannel->pToneRes[pRes->nRes].Tone_Timer,
                               pToneSimple->cadence[0], IFX_FALSE,
                               IFX_TRUE);
         }
         else
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Setup of next simple\
                  tone failed\n\r"));
         }
      }
      else
      {
         /* tone has finished */
         if (ptr_chk(pDrvCtx->ALM.TG_ToneStep, "pDrvCtx->ALM.TG_ToneStep\n\r"))
            ret = pDrvCtx->ALM.TG_ToneStep (pChannel->pLLChannel, pToneSimple,
                                            pRes->nRes, &nToneStep);
         if (ret == IFX_SUCCESS)
         {
            pData->nToneState = TAPI_CT_ACTIVE;
            /* set tone timer */
            TAPI_SetTime_Timer (pChannel->pToneRes[pRes->nRes].Tone_Timer,
                                pToneSimple->cadence[nToneStep],
                                IFX_FALSE, IFX_TRUE);
         }
      }
   }
   else /* UTG */
   {
      /* Activate the universal tone generator with simple tone sequence */
      if (pRes->nRes == 2)
      {
         /* Play tone on DECT UTG */
         if (ptr_chk(pDrvCtx->DECT.UTG_Start, "pDrvCtx->DECT.UTG_Start"))
            pDrvCtx->DECT.UTG_Start(pChannel->pLLChannel,
                                    pToneSimple, pData->dst, 0);
      }
      else
      {
         /* Play tone on SIG UTGs */
         if (ptr_chk(pDrvCtx->SIG.UTG_Start, "pDrvCtx->SIG.UTG_Start"))
            pDrvCtx->SIG.UTG_Start(pChannel->pLLChannel,
                                   pToneSimple, pData->dst, pRes->nRes);
      }
      pChannel->TapiComplexToneData[pRes->nRes].nToneState = TAPI_CT_ACTIVE;
   }

exit:
   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   return;
}

/**
   UTG deactivated event handling function. Is called upon event.
   \param pChannel     - handle to TAPI_CHANNEL structure
   \param utgNum       - index of resource which is reporting this

   \return none
*/
IFX_void_t TAPI_Tone_Step_Completed (TAPI_CHANNEL *pChannel, IFX_uint8_t nResID)
{
   IFX_TAPI_DRV_CTX_t   *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_TONE_DATA_t     *pData;
   IFX_TAPI_EVENT_t     tapiEvent;

   /* handle firmware related stuff here */
   if ((nResID < 2) &&
       ptr_chk(pDrvCtx->SIG.UTG_Event_Deactivated,
              "pDrvCtx->SIG.UTG_Event_Deactivated"))
   {
      pDrvCtx->SIG.UTG_Event_Deactivated(pChannel->pLLChannel, nResID);
   }

   IFXOS_MutexLock(pChannel->semTapiChDataLock);

   /* get pointer to TAPI data storage for this resource  */
   pData = &pChannel->TapiComplexToneData[nResID];

   /* Ignore events when state is not active.
      May be caused by stop after the event was already dispatched. */
   if (pData->nToneState != TAPI_CT_ACTIVE)
   {
      IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
      return;
   }

  /* Tone generation is in deactivated state */
   pData->nToneState = TAPI_CT_DEACTIVATED;
   /* This is a simple tone so prepare for next repetition (if any)
      of a simple tone sequence to play    */
   if (pData->nType == TAPI_TONE_TYPE_SIMPLE)
   {
      /* Set tone generation state */
      Event_SimpleTone(pChannel, nResID);
   }
   else if (pData->nType == TAPI_TONE_TYPE_COMP)
   {
      /* This is a composed tone so retrieve the next (if any) simple tone
      sequence to play */
      /* Prepare for the for next repetition (if any) of a simple tone */
      Event_SimpleTone(pChannel, nResID);
      /* check if no more simple tone is played out. */
      if (pData->nToneState == TAPI_CT_DEACTIVATED)
      {
         /* Init the simple tone repetition counter again */
         pData->nSimpleCurrReps = 0;
         /* Setup the next simple tone to play within the composed tone */
         pData->nToneCounter++;
         /* There are no more simple tones to play within the composed tone */
         if (pData->nToneCounter >= pData->nMaxToneCount)
         {
            /* Init the tone counter again to play repetative composed tones */
            pData->nToneCounter = 0;
            /* Prepare for the for next repetition (if any) of a composed tone */
            Event_ComposedTone(pChannel, nResID);

            if ((pData->nComposedMaxReps != 0) &&
                (pData->nComposedCurrReps >= pData->nComposedMaxReps))
            {
               /* This was the last composed tone sequence to be played */
               /* Set tone generation back to initial state */
               pData->nToneState = TAPI_CT_DEACTIVATED;
            }
         }
         /* Kick off the next simple tone sequence to play */
         else
         {
            if (pData->nPauseTime == 0)
            {
               IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
               Tone_OnTimer (0, (IFX_uint32_t)&pChannel->pToneRes [nResID]);
               return;
            }
            else
            {
               pData->nToneState = TAPI_CT_ACTIVE_PAUSE;
               /* Start the voice path teardown timer */
               TAPI_SetTime_Timer (pChannel->pToneRes[nResID].Tone_Timer,
                                pData->nPauseTime, IFX_FALSE, IFX_FALSE);
            }
         }
      }
   }
   if (pData->nToneState == TAPI_CT_DEACTIVATED)
   {
      /* Stop the voice path teardowm timer */
      TAPI_Stop_Timer(pChannel->pToneRes[nResID].Tone_Timer);
      pData->nToneState = TAPI_CT_IDLE;
      /* issue TAPI event to the application */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_TONE_GEN_END;
      switch (pData->dst)
      {
         case TAPI_TONE_DST_DEFAULT:
         case TAPI_TONE_DST_LOCAL:
            tapiEvent.data.tone_gen.local = 1;
            break;
         case TAPI_TONE_DST_NET:
            tapiEvent.data.tone_gen.network = 1;
            break;
         case TAPI_TONE_DST_NETLOCAL:
            tapiEvent.data.tone_gen.local = 1;
            tapiEvent.data.tone_gen.network = 1;
            break;
      }
      tapiEvent.data.tone_gen.index = pData->nToneIndex;
      IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
   }

   IFXOS_MutexUnlock(pChannel->semTapiChDataLock);
   return;
}

TAPI_CMPLX_TONE_STATE_t TAPI_ToneState (TAPI_CHANNEL *pChannel, IFX_uint8_t nResId)
{
   return pChannel->TapiComplexToneData[nResId].nToneState;
}


/**
   Simple tone deactivated event handling function.
\param
   pChannel     - handle to TAPI_CHANNEL structure
   nResID       - tone resource 0 or 1
\remark
   This function is called from interrupt level. Is handles
   tone resource 0 or 1
\return
   none
*/
static IFX_void_t Event_SimpleTone(TAPI_CHANNEL *pChannel, IFX_uint8_t nResID)
{
   TAPI_TONE_DATA_t *pData;

   /* get context for this resource */
   pData = &pChannel->TapiComplexToneData[nResID];

   /* One simple tone sequence has been applied on this channel */
   pData->nSimpleCurrReps++;

   if (pData->nPauseTime == 0)
   {
   /* Special case: A zero pause time was specified on this channel,
     therefore ignore the repetition count.
     The simple tone sequence must be applied only once.    */
      return;
   }
   /* An infinite tone sequence repetition was requested
      on this channel or there are still simple tone sequences to repeat.
      Repeat tone sequence again after the voice path timer expires. */
   if ((pData->nSimpleMaxReps == 0) ||
            (pData->nSimpleCurrReps < pData->nSimpleMaxReps))
   {
      /* Tone generation is in pause state */
      pData->nToneState = TAPI_CT_ACTIVE_PAUSE;
      /* Start the voice path teardown timer */
      TAPI_SetTime_Timer(pChannel->pToneRes[nResID].Tone_Timer,
                         pData->nPauseTime, IFX_FALSE, IFX_FALSE);
   }
}

/**
   Composed tone deactivated event handling function.
\param
   pChannel     - handle to TAPI_CHANNEL structure
\return
   none
*/
static IFX_void_t Event_ComposedTone(TAPI_CHANNEL *pChannel, IFX_uint8_t nResID)
{
   TAPI_TONE_DATA_t *pData;

   pData = &pChannel->TapiComplexToneData[nResID];
   /* One composed tone sequence has been applied on this channel */
   pData->nComposedCurrReps++;
   if (pData->nAlternateVoiceTime == 0)
   {
   /* Special case: A zero alternate voice time was specified on this channel,
     therefore ignore the repetition count.
     The composed tone sequence must be applied only once.   */

      /* Set tone generation back to initial state */
   }
   /* An infinite tone sequence repetition was requested
      on this channel or there are still composed tone sequences to repeat.
      Repeat tone sequence again after the voice path timer expires.   */
   if ((pData->nComposedMaxReps == 0) ||
       (pData->nComposedCurrReps < pData->nComposedMaxReps))
   {
      /* Tone generation is in pause state */
      pData->nToneState = TAPI_CT_ACTIVE_PAUSE;
      TAPI_SetTime_Timer(pChannel->pToneRes[nResID].Tone_Timer,
                         pData->nAlternateVoiceTime,
                         IFX_FALSE, IFX_FALSE);
   }
}


/* Note: this function shall be replaced by TAPI_Tone_ResIdGet */
IFX_void_t TAPI_Tone_Set_Source (TAPI_CHANNEL *pChannel, IFX_uint8_t res,
                                 IFX_int32_t src)
{
   pChannel->TapiComplexToneData[res].sequenceCap = src;
}
