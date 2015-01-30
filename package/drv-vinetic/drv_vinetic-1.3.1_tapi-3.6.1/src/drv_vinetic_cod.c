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

*******************************************************************************
   Module      : drv_vinetic_cod.c
   Description : This file implements the Coder module
******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_cod_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_con.h"
#include "drv_vinetic_sig.h"
#include "drv_vinetic_stream.h"
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL)
#include "drv_vinetic_cod_aal.h"
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL) */
#ifdef HAVE_CONFIG_H
#include "drv_config.h"
#endif /* HAVE_CONFIG_H */


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Frame lengths in ms */
#define FL_5MS                5
#define FL_5_5MS              6
#define FL_10MS               10
#define FL_20MS               20
#define FL_30MS               30
#define FL_11MS               11
#define FL_40MS               40
#define FL_60MS               60

/* Translation table for coder values   TAPI enum -> FW value */
static IFX_int8_t TranslateCoderTable[] =
{
   (IFX_int8_t)IFX_ERROR,              /*  0: No coder */
   COD_CH_G723_63_ENC,                 /*  1: G723, 6.3 kBit/s */
   COD_CH_G723_53_ENC,                 /*  2: G723, 5.3 kBit/s */
   (IFX_int8_t)IFX_ERROR,              /*  3: No coder */
   (IFX_int8_t)IFX_ERROR,              /*  4: No coder */
   (IFX_int8_t)IFX_ERROR,              /*  5: No coder */
   COD_CH_G728_ENC,                    /*  6: G728, 16 kBit/s */
   COD_CH_G729_ENC,                    /*  7: G729 A and B, 8 kBit/s */
   COD_CH_G711_ULAW_ENC,               /*  8: G711 u-Law, 64 kBit/s */
   COD_CH_G711_ALAW_ENC,               /*  9: G711 A-Law, 64 kBit/s */
   COD_CH_G711_ULAW_VBD_ENC,           /* 10: G711 u-law VBD, 64 kBit/s */
   COD_CH_G711_ALAW_VBD_ENC,           /* 11: G711 A-law VBD, 64 kBit/s */
   COD_CH_G726_16_ENC,                 /* 12: G726, 16 kBit/s */
   COD_CH_G726_24_ENC,                 /* 13: G726, 24 kBit/s */
   COD_CH_G726_32_ENC,                 /* 14: G726, 32 kBit/s */
   COD_CH_G726_40_ENC,                 /* 15: G726, 40 kBit/s */
   COD_CH_G729_E_ENC,                  /* 16: G729 E, 11.8 kBit/s */
   COD_CH_ILBC_133,                    /* 17: iLBC, 13.3 kBit/s */
   COD_CH_ILBC_152,                    /* 18: iLBC, 15.2 kBit/s */
   (IFX_int8_t)IFX_ERROR,              /* 19: No coder */
   (IFX_int8_t)IFX_ERROR,              /* 20: No coder */
   COD_CH_AMR_4_75_ENC,                /* 21: AMR, 4.75 kBit/s */
   COD_CH_AMR_5_15_ENC,                /* 22: AMR, 5.15 kBit/s */
   COD_CH_AMR_5_9_ENC,                 /* 23: AMR, 5.9 kBit/s */
   COD_CH_AMR_6_7_ENC,                 /* 24: AMR, 6.7 kBit/s */
   COD_CH_AMR_7_4_ENC,                 /* 25: AMR, 7.4 kBit/s */
   COD_CH_AMR_7_95_ENC,                /* 26: AMR, 7.95 kBit/s */
   COD_CH_AMR_10_2_ENC,                /* 27: AMR, 10.2 kBit/s */
   COD_CH_AMR_12_2_ENC,                /* 28: AMR, 12.2 kBit/s */
   (IFX_int8_t)IFX_ERROR,              /* 29: No coder */
   (IFX_int8_t)IFX_ERROR,              /* 30: No coder */
   (IFX_int8_t)IFX_ERROR,              /* 31: G.722 (wideband), 64 kBit/s */
   (IFX_int8_t)IFX_ERROR,              /* 32: G.722.1 (wideband), 24 kBit/s */
   (IFX_int8_t)IFX_ERROR,              /* 33: G.722.1 (wideband), 32 kBit/s */
};

/* Translation table for frame length values   TAPI enum -> FW value */
static IFX_int8_t TranslateFrameLengthTable[] =
{
   (IFX_int8_t)IFX_ERROR,              /*  0: Not supported. */
   (IFX_int8_t)IFX_ERROR,              /*  1: 2.5 ms packetization length. */
   COD_CH_PTE_5MS,                     /*  2: 5 ms packetization length.   */
   COD_CH_PTE_5_5MS,                   /*  3: 5.5 ms packetization length. */
   COD_CH_PTE_10MS,                    /*  4: 10 ms packetization length. */
   COD_CH_PTE_11MS,                    /*  5: 11 ms packetization length. */
   COD_CH_PTE_20MS,                    /*  6: 20 ms packetization length. */
   COD_CH_PTE_30MS,                    /*  7: 30 ms packetization length. */
   COD_CH_PTE_40MS,                    /*  8: 40 ms packetization length. */
   (IFX_int8_t)IFX_ERROR,              /*  9: 50 ms packetization length. */
   COD_CH_PTE_60MS                     /* 10: 60 ms packetization length. */
};

/* define for encoder packet time not supported, invalid payload type */
#define PT_INVAL                  1000

/* encoder packet time table : PT_INVAL means not supported */
static const IFX_int32_t VINETIC_PteVal [IFX_TAPI_ENC_TYPE_MAX][7] =
{
   /*                          PTE values                                     */
   {0,         1,         2,         3,         4,         5,         6       },
   /*           IFX_TAPI_ENC_TYPE_G723_63, us - index 1                       */
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  30000,     PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G723_53, us - index 2                       */
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  30000,     PT_INVAL,  PT_INVAL,  PT_INVAL},
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G728   , us - index 6                       */
   {5000,      10000,     20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G729   , us - index 7                       */
   {PT_INVAL,  10000,     20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  60000   },
   /*           IFX_TAPI_ENC_TYPE_MLAW   , us - index 8                       */
   {5000,      10000,     20000,     PT_INVAL,  5500,      11000,     60000   },
   /*           IFX_TAPI_ENC_TYPE_ALAW   , us - index 9                       */
   {5000,      10000,     20000,     PT_INVAL,  5500,      11000,     60000   },
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G726_16, us - index 12                      */
   {5000,      10000,     20000,     PT_INVAL,  5500,      11000,     PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G726_24, us - index 13                      */
   {5000,      10000,     20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G726_32, us - index 14                      */
   {5000,      10000,     20000,     PT_INVAL,  5500,      11000,     PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G726_40, us - index 15                      */
   {5000,      10000,     20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_G729_E , us - index 16                      */
   {PT_INVAL,  10000,     20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  60000   },
   /*           IFX_TAPI_ENC_TYPE_ILBC_133 , us - index 17                    */
   {PT_INVAL,  PT_INVAL,  PT_INVAL,  30000,     PT_INVAL,  PT_INVAL,  PT_INVAL},
   /*           IFX_TAPI_ENC_TYPE_ILBC_152 , us - index 18                    */
   {PT_INVAL,  PT_INVAL,  20000,     PT_INVAL,  PT_INVAL,  PT_INVAL,  PT_INVAL}
};


/* ---------------------------------------------------------------------------
                        AGC related variables - BEGIN
 */

/** Parameter limits for AGC (Automated Gain Control). */

/** "Compare Level", this is the target level in 'dB', MAX is 0dB. */
const IFX_int32_t AGC_CONFIG_COM_MAX = 0;

/** "Compare Level", this is the target level in 'dB', MIN is -50dB. */
const IFX_int32_t AGC_CONFIG_COM_MIN = -50;

/** Used to get right table index for dB -> HEX conversion. */
const IFX_int32_t AGC_CONFIG_COM_OFFSET = 50;

/** "Maximum Gain", maximum gain that we'll be applied to the signal in
    'dB', MAX is 48dB. */
const IFX_int32_t AGC_CONFIG_GAIN_MAX = 48;

/** "Maximum Gain", maximum gain that we'll be applied to the signal in
    'dB', MIN is 0dB. */
const IFX_int32_t AGC_CONFIG_GAIN_MIN = 0;

/** Used to get right table index for dB -> HEX conversion. */
const IFX_int32_t AGC_CONFIG_GAIN_OFFSET = 0;

/** "Maximum Attenuation for AGC", maximum attenuation that we'll be applied
    to the signal in 'dB', MAX is 0dB. */
const IFX_int32_t AGC_CONFIG_ATT_MAX = 0;

/** "Maximum Attenuation for AGC", maximum attenuation that we'll be applied
    to the signal in 'dB', MIN is -42dB. */
const IFX_int32_t AGC_CONFIG_ATT_MIN = -42;

/** Used to get right table index for dB -> HEX conversion. */
const IFX_int32_t AGC_CONFIG_ATT_OFFSET = 42;

/** "Minimum Input Level", signals below this threshold won't be processed
    by AGC in 'dB', MAX is -25 dB. */
const IFX_int32_t AGC_CONFIG_LIM_MAX = -25;

/** "Minimum Input Level", signals below this threshold won't be processed
    by AGC in 'dB', MIN is -60 dB. */
const IFX_int32_t AGC_CONFIG_LIM_MIN = -60;

/** Used to get right table index for dB -> HEX conversion. */
const IFX_int32_t AGC_CONFIG_LIM_OFFSET = 60;


/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_COM[] =
{
   /* conversion table from 'dB# to HEX register values */
   /* -50   -49   -48   -47   -46   -45   -44   -43   -42   -41   -40   */
      132,  133,  133,  134,  134,  135,  136,  137,  138,  140,  141,
   /* -39   -38   -37   -36   -35   -34   -33   -32   -31   -30   -29   */
      142,  144,  146,  148,  151,  154,  157,  160,  164,  169,  174,
   /* -28   -27   -26   -25   -24   -23   -22   -21   -20   -19   -18   */
      179,  186,  193,  7,    8,    9,    10,   11,   13,   14,   16,
   /* -17   -16   -15   -14   -13   -12   -11   -10   -9    -8    -7    */
      18,   20,   23,   26,   29,   32,   36,   40,   45,   51,   57,
   /* -6    -5    -4    -3    -2    -1    0  */
      64,   72,   81,   91,   102,  114,  128
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_GAIN[] =
{
   /* 0     1     2     3     4     5     6     7     8     9     10    */
      136,  137,  138,  139,  141,  142,  144,  146,  148,  151,  153,
   /* 11    12    13    14    15    16    17    18    19    20    21    */
      156,  160,  164,  168,  173,  178,  185,  4,    199,  208,  218,
   /* 22    23    24    25    26    27    28    29    30    31    32    */
      229,  241,  32,   36,   40,   45,   50,   56,   63,   71,   80,
   /* 33    34    35    36    37    38    39    40    41    42    43    */
      89,   100,  112,  126,  142,  159,  178,  200,  224,  252,  255,
   /* 44    45    46    47    48    */
      255,  255,  255,  255,  255
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_ATT[] =
{
   /* -42   -41   -40   -39   -38   -37   -36   -35   -34   -33   -32   */
      1,    1,    1,    1,    2,    2,    2,    2,    3,    3,    3,
   /* -31   -30   -29   -28   -27   -26   -25   -24   -23   -22   -21   */
      4,    4,    5,    5,    6,    6,    7,    8,    9,    10,   11,
   /* -20   -19   -18   -17   -16   -15   -14   -13   -12   -11   -10   */
      13,   14,   16,   18,   20,   23,   26,   29,   32,   36,   40,
   /* -9    -8    -7    -6    -5    -4    -3    -2    -1    0  */
      45,   51,   57,   64,   72,   81,   91,   102,  114,  127
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_LIM[] =
{
   /* -60   -59   -58   -57   -56   -55   -54   -53   -52   -51   -50   */
      161,  165,  169,  174,  180,  186,  193,  201,  210,  220,  232,
   /* -49   -48   -47   -46   -45   -44   -43   -42   -41   -40   -39   */
      7,    8,    9,    10,   12,   13,   14,   16,   18,   20,   23,
   /* -38   -37   -36   -35   -34   -33   -32   -31   -30   -29   -28   */
      26,   29,   32,   36,   41,   46,   51,   58,   65,   73,   81,
   /* -27   -26   -25   */
      91,   103,  115
};

static IFX_uint8_t VadCoeff_Reg_LIM[] =
{
   /*   0    -3    -6    -9   -12   -15   -18   -21   -24   -27   */
      127,  124,  120,  116,  112,  108,  104,  100,   96,   92,
   /* -30   -33   -36   -39   -42   -45   -48   -51   -54   -57   */
       88,   84,   80,   76,   72,   68,   64,   60,   56,   52,
   /* -60   -63   -66   -69   -72   -75   -78   -81   -84   -87   */
       48,   44,   40,   36,   32,   28,   24,   20,   16,   12,
   /* -90   -93   -96   */
        8,    4,    1
};

/* ---------------------------------------------------------------------------
                        AGC related variables - END
 */


/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t getFrameLength              (IFX_int32_t nPTE);

static IFX_int32_t getPTE                      (IFX_int32_t nFrameLength);

static IFX_int32_t getConvPTE (IFX_int32_t nPrePTE, IFX_int32_t nCodec);

static IFX_TAPI_COD_TYPE_t vinetic_cod_trans_cod_fw2tapi (IFX_int8_t nCoder);
static IFX_int8_t vinetic_cod_trans_fl_tapi2fw (IFX_TAPI_COD_LENGTH_t nFL);
static IFX_boolean_t vinetic_cod_codec_support_check (VINETIC_DEVICE *pDev,
                                                      IFX_int8_t nCodec);


static IFX_int32_t Dsp_SetDPDemod  (VINETIC_CHANNEL *pCh, IFX_uint8_t nSt1,
                                    IFX_uint8_t nSt2, IFX_uint8_t nEq,
                                    IFX_uint8_t nTr);
static IFX_int32_t Dsp_SetDPMod    (VINETIC_CHANNEL *pCh, IFX_uint8_t nSt,
                                    IFX_uint16_t nLen, IFX_uint8_t nDbm,
                                    IFX_uint8_t nTEP, IFX_uint8_t nTr);
static IFX_int32_t Dsp_SetDatapump (VINETIC_CHANNEL *pCh, IFX_boolean_t bEn,
                                    IFX_boolean_t bMod, IFX_uint16_t gain,
                                    IFX_uint16_t mod_start,
                                    IFX_uint16_t mod_req, IFX_uint16_t demod_send);
static IFX_int32_t Dsp_SetRTPPayloadType(VINETIC_CHANNEL *pCh,
                                    IFX_boolean_t bUp, IFX_uint8_t *pPTvalues);

static IFX_TAPI_LL_ERR_t IFX_TAPI_LL_COD_DEC_Chg_Detail_Req (IFX_TAPI_LL_CH_t *pLLCh,
                                                             IFX_TAPI_DEC_DETAILS_t *pDec);
static IFX_return_t IFX_TAPI_LL_COD_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_TAPI_PKT_VOLUME_t const *pVol);
static IFX_return_t IFX_TAPI_LL_COD_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_boolean_t bHp);

static IFX_int32_t VINETIC_COD_RTP_Cfg (VINETIC_CHANNEL *pCh,
                                       IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   function called to get frame length in ms according to PTE value
\param nPTE PTE value
\return
    encoder packet time in ms or IFX_ERROR
Remarks
*/
static IFX_int32_t getFrameLength (IFX_int32_t nPTE)
{
   switch (nPTE)
   {
   case COD_CH_PTE_5MS:
      return (FL_5MS);
   case COD_CH_PTE_5_5MS:
      return (FL_5_5MS);
   case COD_CH_PTE_10MS:
      return (FL_10MS);
   case COD_CH_PTE_20MS:
      return (FL_20MS);
   case COD_CH_PTE_30MS:
      return (FL_30MS);
   case COD_CH_PTE_11MS:
      return (FL_11MS);
   case COD_CH_PTE_40MS:
      return (FL_40MS);
   case COD_CH_PTE_60MS:
      return (FL_60MS);
   default:
      return IFX_ERROR;
   }
}

/**
   decode the frame length coefficient to the IFX_TAPI value
\param nPTE frame length (packetisation time) coefficients
\return IFX_TAPI value for this packetisation time
*/
static IFX_TAPI_COD_LENGTH_t getCOD_TAPI_Frame_Length(IFX_int16_t nPTE)
{
   switch (nPTE)
   {
   case 10:
      return (IFX_TAPI_COD_LENGTH_5);
   case 11:
      return (IFX_TAPI_COD_LENGTH_5_5);
   case 20:
      return (IFX_TAPI_COD_LENGTH_10);
   case 40:
      return (IFX_TAPI_COD_LENGTH_20);
   case 60:
      return (IFX_TAPI_COD_LENGTH_30);
   case 22:
      return (IFX_TAPI_COD_LENGTH_11);
   case 80:
      return (IFX_TAPI_COD_LENGTH_40);
   case 120:
      return (IFX_TAPI_COD_LENGTH_60);
   default:
      return IFX_TAPI_COD_LENGTH_ZERO;
   }
}

/**
   function called to get PTE value according to frame length in ms
\param nFrameLength    encoder packet time in ms
\return
   PTE value or IFX_ERROR
Remarks
*/
static IFX_int32_t getPTE (IFX_int32_t nFrameLength)
{
   switch (nFrameLength)
   {
   case FL_5MS:
      return (COD_CH_PTE_5MS);
   case FL_5_5MS:
      return COD_CH_PTE_5_5MS;
   case FL_10MS:
      return (COD_CH_PTE_10MS);
   case FL_20MS:
      return (COD_CH_PTE_20MS);
   case FL_30MS:
      return (COD_CH_PTE_30MS);
   case FL_11MS:
      return (COD_CH_PTE_11MS);
   case FL_40MS:
      return (COD_CH_PTE_40MS);
   case FL_60MS:
      return (COD_CH_PTE_60MS);
   default:
      return IFX_ERROR;
   }
}

/**
   function to translate coder value from TAPI enum to FW encoding
\param nCoder    Coder setected
\return
   ENC value or IFX_ERROR
*/
IFX_uint8_t VINETIC_COD_trans_cod_tapi2fw (IFX_TAPI_COD_TYPE_t nCoder)
{
   /* range check neccessary because this function is called from extern */
   /* sizeof is sufficient because data type in array is uint8 */
   if( (nCoder < 0) || (nCoder >= sizeof(TranslateCoderTable)) )
   {
      TRACE(VINETIC, DBG_LEVEL_HIGH,
            ("\n\rDRV_ERROR: Parameter coder value %d out of range [0 to %d]\n",
             nCoder, sizeof(TranslateCoderTable)));
      return((IFX_int8_t)IFX_ERROR);
   }

   return TranslateCoderTable[nCoder];
}

/**
   function to translate coder value from FW encoding to TAPI enum
\param  nCoder fw coder value
\return IFX_TAPI value for this coder type or IFX_TAPI_COD_TYPE_UNKNOWN
*/
static IFX_TAPI_COD_TYPE_t vinetic_cod_trans_cod_fw2tapi (IFX_int8_t nCoder)
{
   IFX_TAPI_COD_TYPE_t  i;

   /* Lookup FW value in the translation table. Index in this table is the
      enum used by tapi. Index 0 is coder type unknown and is left out of
      the check. If FW value cannot be found leave and exit below. */
   for (i=IFX_TAPI_COD_TYPE_UNKNOWN+1; i < sizeof(TranslateCoderTable); i++)
   {
      if (TranslateCoderTable[i] == nCoder)
      {
         return i;
      }
   }

   return IFX_TAPI_COD_TYPE_UNKNOWN;
}

/**
   Function to perform check whether the requested codec is supported by
   the firmware version in use.

\param pDev       pointer to VINETIC device structure
\param nCodec     Encoder identifier

\return
   IFX_TRUE for supported encoder, else IFX_FALSE
*/
static IFX_boolean_t vinetic_cod_codec_support_check (VINETIC_DEVICE *pDev,
                                                      IFX_int8_t nCodec)
{
   IFX_boolean_t bRet = IFX_FALSE;

   switch (nCodec)
   {
      /* inactive encoder */
      case (IFX_int8_t)IFX_ERROR:
         bRet = IFX_TRUE;
         break;
      /* G.711, alaw and mlaw */
      case COD_CH_G711_ULAW_ENC:
      case COD_CH_G711_ALAW_ENC:
      case COD_CH_G711_ULAW_VBD_ENC:
      case COD_CH_G711_ALAW_VBD_ENC:
         if ((pDev->caps.CODECS & CODEC_G711))
         bRet = IFX_TRUE;
         break;
      /* G.726, 16, 24, 32, and 40 kbit/s */
      case COD_CH_G726_16_ENC:
      case COD_CH_G726_24_ENC:
      case COD_CH_G726_32_ENC:
      case COD_CH_G726_40_ENC:
         if ((pDev->caps.CODECS & CODEC_G726))
         bRet = IFX_TRUE;
         break;
      /* AMR with all data rates, 4.75, 5.15, 5.9, 6.7, 7.4,
                                  7.95, 10.2, 12.2 bit/s */
      case COD_CH_AMR_4_75_ENC:
      case COD_CH_AMR_5_15_ENC:
      case COD_CH_AMR_5_9_ENC:
      case COD_CH_AMR_6_7_ENC:
      case COD_CH_AMR_7_4_ENC:
      case COD_CH_AMR_7_95_ENC:
      case COD_CH_AMR_10_2_ENC:
      case COD_CH_AMR_12_2_ENC:
         if ((pDev->caps.CODECS & CODEC_AMR))
         bRet = IFX_TRUE;
         break;
      /* G.728, 16 kbit/s */
      case COD_CH_G728_ENC:
         if ((pDev->caps.CODECS & CODEC_G728))
         bRet = IFX_TRUE;
         break;
      /* G.729A/B, 8 and 11.8 kbit/s */
      case COD_CH_G729_ENC:
         /* This check is disabled because FW does not correctly indicate this
            coder type in some versions. So to avoid not beeing able to use
            this encoder we print a warning and assume that we support it.
            If FW support is not available it we get a command error when using
            this encoder type. */
         if (!(pDev->caps.CODECS & CODEC_G729AB))
         {
            TRACE (VINETIC, DBG_LEVEL_NORMAL, (
                   "Info: G.729 A&B support of FW cannot be determined.\n"
                   "Assuming that it is there but command error is possible\n"));
         }
#if 0
         if ((pDev->caps.CODECS & CODEC_G729AB))
#endif
         bRet = IFX_TRUE;
         break;
      /* ILBC, 13.3 and 15.2 kbit/s */
      case COD_CH_ILBC_133:
      case COD_CH_ILBC_152:
         if ((pDev->caps.CODECS & CODEC_ILBC))
         bRet = IFX_TRUE;
         break;
      /* G723.1, 5.3 and 6.3 kbit/s */
      case COD_CH_G723_63_ENC:
      case COD_CH_G723_53_ENC:
         if ((pDev->caps.CODECS & CODEC_G723_1))
         bRet = IFX_TRUE;
         break;
      /* G.729E, 11.8 kbit/s */
      case COD_CH_G729_E_ENC:
         if ((pDev->caps.CODECS & CODEC_G729E))
         bRet = IFX_TRUE;
         break;
      default:
         bRet = IFX_FALSE;
   }

   return bRet;
}

/**
   function to translate frame length value from TAPI enum to FW encoding
\param nFL    Frame length setected
\return
   Frame length value or IFX_ERROR
*/
static IFX_int8_t vinetic_cod_trans_fl_tapi2fw (IFX_TAPI_COD_LENGTH_t nFL)
{
   /* range check not neccessary as long as enum and array stay in sync
      and nobody casts any out of range values to the parameter */
   return TranslateFrameLengthTable[nFL];
}

/**
   function called to determine the convenient PTE value according to the pre
   existing value in case this one doesn't match with the selected encoder
\param nPrePTE         pre existing PTE
\param nCodec          encoder algorithm value
\return
   IFX_SUCCESS or IFX_ERROR
Remarks
   The tables of PTE values supported per encoder look as follows :
   - G.711 Alaw         : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.711 Mulaw        : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.723 5.3 KBps     : 30 ms
   - G.723 6.3 KBps     : 30 ms
   - G.728              : 5ms, 10 ms, 20 ms
   - G.729_AB           : 10 ms, 20 ms
   - G.729_E            : 10 ms, 20 ms
   - G.726 16 KBps      : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.726 24 kBps      : 5ms, 10 ms, 20 ms
   - G.726 32 KBps      : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.726 40 KBps      : 5ms, 10 ms, 20 ms
   - ILBC  13.3 KBps    : 30 ms - no other choice possible
   - ILBC  15.2 KBps    : 20 ms - no other choice possible
*/
static IFX_int32_t getConvPTE (IFX_int32_t nPrePTE, IFX_int32_t nCodec)
{
   IFX_int32_t *pTemp    = NULL;
   IFX_int32_t  size, pre_time = 0, abs_time = 0, i = 0, pos = 0, nConvPte;

   /* set size */
   size = sizeof (VINETIC_PteVal [nCodec]) / sizeof (IFX_int32_t);
   /* look in vertical to determine the actual packet time */
   for (i = 1; i < IFX_TAPI_ENC_TYPE_MAX; i++)
   {
      pTemp = (IFX_int32_t *)VINETIC_PteVal [i];
      if (pTemp [nPrePTE] != PT_INVAL)
      {
         pre_time = pTemp [nPrePTE];
         break;
      }
   }
   /* find first value for comparisons */
   for (i = 0; i < size; i ++)
   {
      pTemp = (IFX_int32_t *)&VINETIC_PteVal [nCodec][i] ;
      if (*pTemp != PT_INVAL)
      {
         abs_time = ABS (*pTemp - pre_time);
         pos = i;
         break;
      }
   }
   /* loop horizontally to determine best match */
   for (i = 0; i < size; i++)
   {
      pTemp = (IFX_int32_t *)&VINETIC_PteVal [nCodec][i];
      if (*pTemp != PT_INVAL && (abs_time > ABS (*pTemp - pre_time)))
      {
         abs_time = ABS (*pTemp - pre_time);
         pos = i;
      }
   }
   /* set new pte value */
   nConvPte = VINETIC_PteVal [0][pos];

   /* trace out some important info */
   TRACE (VINETIC, DBG_LEVEL_HIGH, ("Not matching encoder packet time of "
         "%d us is set now.\n\r", pre_time));
   TRACE (VINETIC, DBG_LEVEL_HIGH, ("This will be replaced by the best "
         "matching encoder packet time of %d us.\n\r",
         VINETIC_PteVal [nCodec][pos]));

   return nConvPte;
}

/**
   function called to check the validity of the co-existence of the requested
   PTE value and encoder algorithm value
\param nFrameLength    encoder packet time (PTE) value
\param nCodec          encoder algorithm value
\return
   IFX_SUCCESS or IFX_ERROR
Remarks
   Not every encoder packet time are allowed for each encoder algorithm.
   So the setting of encoder packet time has to meet following requirements:

   - G.711 Alaw         : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms, 60 ms
   - G.711 Mulaw        : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms, 60 ms
   - G.723 5.3 Kbps     : 30 ms
   - G.723 6.3 Kbps     : 30 ms
   - G.728              : 5ms, 10 ms, 20 ms
   - G.729_AB           : 10 ms, 20 ms, 60 ms
   - G.729_E            : 10 ms, 20 ms, 60 ms
   - G.726 16 Kbps      : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.726 24 kbps      : 5ms, 10 ms, 20 ms
   - G.726 32 Kbps      : 5ms, 10 ms, 20 ms, 5.5 ms, 11 ms
   - G.726 40 Kbps      : 5ms, 10 ms, 20 ms
*/
static IFX_int32_t CheckENCnPTE (IFX_uint8_t nFrameLength, IFX_uint8_t nCodec)
{
   IFX_int32_t ret = IFX_SUCCESS;

   switch (nFrameLength)
   {
      /* 5ms, not possible for AMR, G.729, G.723.1 and ILBCx */
      case COD_CH_PTE_5MS :
         if ((nCodec == COD_CH_AMR_4_75_ENC) ||
             (nCodec == COD_CH_AMR_5_15_ENC) ||
             (nCodec == COD_CH_AMR_5_9_ENC)  ||
             (nCodec == COD_CH_AMR_6_7_ENC)  ||
             (nCodec == COD_CH_AMR_7_4_ENC)  ||
             (nCodec == COD_CH_AMR_7_95_ENC) ||
             (nCodec == COD_CH_AMR_10_2_ENC) ||
             (nCodec == COD_CH_AMR_12_2_ENC) ||
             (nCodec == COD_CH_G729_ENC)    ||
             (nCodec == COD_CH_G729_E_ENC)  ||
             (nCodec == COD_CH_G723_53_ENC) ||
             (nCodec == COD_CH_G723_63_ENC) ||
             (nCodec == COD_CH_ILBC_152)    ||
             (nCodec == COD_CH_ILBC_133))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 10ms, not possible for AMR, G.723x and ILBCx */
      case COD_CH_PTE_10MS :
         if ((nCodec == COD_CH_AMR_4_75_ENC) ||
             (nCodec == COD_CH_AMR_5_15_ENC) ||
             (nCodec == COD_CH_AMR_5_9_ENC)  ||
             (nCodec == COD_CH_AMR_6_7_ENC)  ||
             (nCodec == COD_CH_AMR_7_4_ENC)  ||
             (nCodec == COD_CH_AMR_7_95_ENC) ||
             (nCodec == COD_CH_AMR_10_2_ENC) ||
             (nCodec == COD_CH_AMR_12_2_ENC) ||
             (nCodec == COD_CH_G723_53_ENC) ||
             (nCodec == COD_CH_G723_63_ENC) ||
             (nCodec == COD_CH_ILBC_152)    ||
             (nCodec == COD_CH_ILBC_133))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 20ms, not possible for G.723x and ILBC 13.3  */
      case COD_CH_PTE_20MS :
         if ((nCodec == COD_CH_G723_53_ENC) ||
             (nCodec == COD_CH_G723_63_ENC) ||
             (nCodec == COD_CH_ILBC_133))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 30ms, allowed for all codecs, except AMR and ILBC 15.2 */
      case COD_CH_PTE_30MS :
         if ((nCodec == COD_CH_AMR_4_75_ENC) ||
             (nCodec == COD_CH_AMR_5_15_ENC) ||
             (nCodec == COD_CH_AMR_5_9_ENC)  ||
             (nCodec == COD_CH_AMR_6_7_ENC)  ||
             (nCodec == COD_CH_AMR_7_4_ENC)  ||
             (nCodec == COD_CH_AMR_7_95_ENC) ||
             (nCodec == COD_CH_AMR_10_2_ENC) ||
             (nCodec == COD_CH_AMR_12_2_ENC) ||
             (nCodec == COD_CH_ILBC_152))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 5.5ms and 11ms, only allowed for G.711, G.726-16kbps, G.726-32kbps */
      case COD_CH_PTE_5_5MS :
      case COD_CH_PTE_11MS  :
         if ((nCodec != COD_CH_G711_ALAW_ENC) &&
             (nCodec != COD_CH_G711_ULAW_ENC) &&
             (nCodec != COD_CH_G726_16_ENC)   &&
             (nCodec != COD_CH_G726_32_ENC))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 40ms, not possible for G.723 and iLBC 13.3KBps */
      case COD_CH_PTE_40MS :
         if ((nCodec == COD_CH_G723_53_ENC) ||
             (nCodec == COD_CH_G723_63_ENC) ||
             (nCodec == COD_CH_ILBC_133))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 60ms,  allowed for all codecs */
      case COD_CH_PTE_60MS :
         /* the default is supported - so do nothing here */
         break;
      default:
         break;
   }

   return ret;
}


/**
   Configures the fax datapump demodulator

   \param  pCh        - pointer to VINETIC channel structure
   \param  nSt1       - standard 1
   \param  nSt2       - Standard 2
   \param  nEq        - Equalizer
   \param  nTr        - Training

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t Dsp_SetDPDemod (VINETIC_CHANNEL *pCh, IFX_uint8_t nSt1,
                                   IFX_uint8_t nSt2, IFX_uint8_t nEq,
                                   IFX_uint8_t nTr)
{
   IFX_uint16_t pCmd [3] = {0};
   VINETIC_DEVICE *pDev = pCh->pParent;

   pCmd [0] = (CMD1_EOP | (pCh->nChannel - 1));
   pCmd [1] = ECMD_COD_CHFAXDEMOD;
   pCmd [2] = ((nSt1 & COD_DEMOD_ST1)        |
               ((nSt2 << 5) & COD_DEMOD_ST2) |
               ((nEq << 14) & COD_DEMOD_EQ)  |
               ((nTr << 15) & COD_DEMOD_TRN));

   return CmdWrite (pDev, pCmd, 1);
}


/**
   Configures the Datapump modulator

   \param   pCh        - pointer to VINETIC channel structure
   \param   nSt        - standard
   \param   nLen       - signal length
   \param   nDbm       - level
   \param   nTEP       - TEP
   \param   nTr        - Training

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t Dsp_SetDPMod (VINETIC_CHANNEL *pCh, IFX_uint8_t nSt,
                                 IFX_uint16_t nLen, IFX_uint8_t nDbm,
                                 IFX_uint8_t nTEP, IFX_uint8_t nTr)
{
   IFX_uint16_t pCmd [4] = {0};
   VINETIC_DEVICE *pDev = pCh->pParent;

   /* set command to configure the  modulator */
   pCmd [0] = (CMD1_EOP | (pCh->nChannel - 1));
   pCmd [1] = ECMD_COD_CHFAXMOD;
   pCmd [2] = (nSt | (nTr << 5) | (nTEP << 6) |
               ((nDbm << 8) & COD_MOD_DBM));
   if (nLen <= COD_CHFDP_MODMAXSGLEN)
      pCmd [3] = (nLen & COD_MOD_SGLEN);
   else
      pCmd [3] = COD_CHFDP_MODMAXSGLEN;

   return CmdWrite (pDev, pCmd, 2);
}


/**
   Set data pump according to input parameters

   \param   pCh        - pointer to VINETIC channel structure
   \param   bEn        - IFX_FALSE : disable / IFX_TRUE : enable datapump
   \param   bMod       - 0 = Modulator / 1 = Demodulator
   \param   gain       - gain to be applied, 0x60 is 0 dB
   \param   mod_start  - level for start modulation
   \param   mod_req    - level for request more data
   \param   demod_send - level for send data

   \return
      IFX_SUCCESS or IFX_ERROR
   \remark
      This function disables datapump if bEn is IFX_FALSE. Otherwise it enables the
      datapump and set modulator or demodulator according to bMod
*/
static IFX_int32_t Dsp_SetDatapump (VINETIC_CHANNEL *pCh, IFX_boolean_t bEn,
                                    IFX_boolean_t bMod, IFX_uint16_t gain,
                                    IFX_uint16_t mod_start, IFX_uint16_t mod_req,
                                    IFX_uint16_t demod_send)
{
   IFX_uint16_t    pCmd [7] = {0};
   VINETIC_DEVICE *pDev     = pCh->pParent;
   IFX_uint8_t     ch       = (pCh->nChannel - 1), count;

   /* setup cmd 1 */
   pCmd [0] = (CMD1_EOP | ch);
   /* set Coder Channel Fax data Pump */
   pCmd [1] = ECMD_COD_CHFAXDP;

   /** \todo  To check: Because the ALM isn't directly connected to
     CODER anymore, how shall we connect the datapump input?
     Where is coming the signal from? Now we use the coder configuration
     on this channel, as the coder is disabled.       */
   pCmd [2] = (ch << 8) | pDev->pChannel[ch].pCOD->fw_cod_ch.bit.i1;

   /* Disable Datapump if  EN = 1 */
   if (bEn == IFX_FALSE)
   {
      count =1;
   }
   else
   {
      count = 5;
      /* Set DPNR = Ch , input 1 = Coder input I1 setting */
      /* Note: The coder channel should be disabled to do this, so that
               we can use this resource. */
      pCmd [2] |= COD_CHFDP_EN;

      switch (bMod)
      {
         /* configure data for modulation */
         case IFX_FALSE:
            /* Modulation gain */
            pCmd [3] = 0x6000 | gain;
            /* MOBSM: level for start modulation */
            pCmd [4] = mod_start;
            /* MOBRD: level for request more data */
            pCmd [5] = mod_req;
            /* set unused fields to valid values */
            pCmd [6] = 1;
            break;
         /* configure data for demodulation */
         case IFX_TRUE:
            /* MD = 1, */
            pCmd [2] |= COD_CHFDP_MD;
            /* Demodulation gain */
            pCmd [3] = 0x60 | ((IFX_uint16_t)gain << 8);
            /* set unused fields to valid values */
            pCmd [4] = 1;
            pCmd [5] = 1;
            /* DMBSD: send data level */
            pCmd [6] = demod_send;
            break;
      }
   }

   return (CmdWrite (pDev, pCmd, count));
}


/**
   Sets Coder payload type (RTP) in the streaming direction given

   \param  pCh    - pointer to VINETIC channel structure
   \param  bUp    - IFX_TRUE : upstream / IFX_FALSE : downstream

   \return
      IFX_SUCCESS or IFX_ERROR
   \remark
      This function assumes that the array pPTvalues is at least
      IFX_TAPI_ENC_TYPE_MAX big and supports all coders specified in
      \ref IFX_TAPI_ENC_TYPE_t which is used as index.
*/
static IFX_int32_t Dsp_SetRTPPayloadType (VINETIC_CHANNEL *pCh,
                                          IFX_boolean_t bUp,
                                          IFX_uint8_t *pPTvalues)
{
   IFX_int32_t      ret = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t      ch  = pCh->nChannel - 1, count = 0, i;
   IFX_uint16_t    pData [20] = {0}, *pPtype = IFX_NULL;

   /* downstraem direction (TX) */
   if (bUp == IFX_FALSE)
   {
      /* set commands data words using cached value to avoid reading
         the coder channel configuration */
      pData [0] = (CMD1_EOP | ch);
      pData [1] = ECMD_COD_CHRTP_TX;
      pData [2] = HIGHWORD (pCh->pCOD->nSsrc);
      pData [3] = LOWWORD  (pCh->pCOD->nSsrc);
      pData [4] = pCh->pCOD->nSeqNr;
      /* set ptr to right place in RTP config data buffer */
      pPtype = &pData [5];
      /* set number of data to write */
      count  = 18;
      /* set SID bits for TX direction only */
      pPtype [0] = 0x8080;
      pPtype [1] = 0x8080;
      pPtype [2] = 0x8080;
   }
   else /* upstream direction (RX) */
   {
      /* set command to write the whole message */
      pData [0] = (CMD1_EOP | ch);
      pData [1] = ECMD_COD_CHRTP_RX;
      /* set ptr to right place in RTP config data buffer */
      pPtype    = &pData [2];
      /* set number of data to write */
      count     = 15;
   }

   /* range check and check for conflicts with the event PT */
   for (i=0 ; i<IFX_TAPI_ENC_TYPE_MAX; i++)
   {
      /* FW accepts payload types 0x00 - 0x7F,
         bit 0x80 is reserved for the SID bit */
      if (pPTvalues[i] & 0x80)
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH,
         ("ERROR RTP payload type (0x%X) out of range 0x00-0x7F\n\r",
          pPTvalues[i]));
          return IFX_ERROR;
      }

      /* check for conflicts with the event payload type */
      if (pPTvalues[i] == pCh->nEvtPT)
      {
         TRACE(VINETIC, DBG_LEVEL_HIGH, ("ERROR conflict in RTP PT table,"
               "redefinition of the event PT (0x%X)\n\r", pCh->nEvtPT));
         /* errmsg: payloadtype redefinition (evt pt) */
         /*RETURN_STATUS (VIN_statusRtpEvtPtRedefinition, IFX_NULL);*/
         return IFX_ERROR;
      }
   }

   /* G711, IFX_TAPI_ENC_TYPE_ALAW, 64 KBps */
   pPtype [0]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_ALAW] << 8;
   /* G711, uLAW, 64 KBps */
   pPtype [0]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_MLAW];
   /* G726, 16 KBps */
   pPtype [1]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_G726_16] << 8;
   /* G726, 24 KBps */
   pPtype [1]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_G726_24];
   /* G726, 32 KBps */
   pPtype [2]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_G726_32] << 8;
   /* G726, 40 KBps */
   pPtype [2]   |=  pPTvalues [IFX_TAPI_ENC_TYPE_G726_40];

   /* Write AMR PT values only if FW reports AMR support because
      AMR is only avaliable in special versions. */
   if (vinetic_cod_codec_support_check(pDev, COD_CH_AMR_4_75_ENC))
   {
      /* AMR 4.75, KBps | AMR, 5.15 KBps */
      pPtype [3]   |=  (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_4_75] << 8) |
                       (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_5_15]);
      /* AMR 5.9, KBps | AMR, 6.7 KBps */
      pPtype [4]   |=  (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_5_9] << 8) |
                       (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_6_7]);
      /* AMR 7.4, KBps | AMR, 7.95 KBps */
      pPtype [5]   |=  (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_7_4] << 8) |
                       (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_7_95]);
      /* AMR 10.2, KBps | AMR, 12.2 KBps */
      pPtype [6]   |=  (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_10_2] << 8) |
                       (pPTvalues[IFX_TAPI_ENC_TYPE_AMR_12_2]);
   }

   /* IFX_TAPI_ENC_TYPE_G728, 16 KBps */
   pPtype [7]   |=  pPTvalues[IFX_TAPI_ENC_TYPE_G728] << 8;
   /* IFX_TAPI_ENC_TYPE_G729, 8KBps */
   pPtype [8]   |=  (pPTvalues[IFX_TAPI_ENC_TYPE_G729] << 8) |
                     pPTvalues[IFX_TAPI_ENC_TYPE_G729_E];
   /** iLBC, 15.2 kBit/s */
   pPtype [12]  |=  (pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_152] << 8);
   /** iLBC, 13.3 kBit/s */
   pPtype [12]  |=  pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_133];
   /* G723, 5,3 KBps */
   pPtype [13]  |=  (pPTvalues [IFX_TAPI_ENC_TYPE_G723_53] << 8);
   /* G723, 6,3 KBps */
   pPtype [13]  |=  pPTvalues [IFX_TAPI_ENC_TYPE_G723_63];
   /* G723, 5,3 KBps */
   pPtype [14]  |=  (pPTvalues [IFX_TAPI_ENC_TYPE_ALAW_VBD] << 8);
   /* G723, 6,3 KBps */
   pPtype [14]  |=  pPTvalues [IFX_TAPI_ENC_TYPE_MLAW_VBD];

   ret = CmdWrite (pDev, pData, count);

   /* Some FW versions are not able to handle different PT types for the two
      iLBC rates in decoding direction. Print a warning for the user. */
   if ((bUp == IFX_FALSE) &&
       (pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_133] !=
        pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_152]))
   {
      TRACE(VINETIC, DBG_LEVEL_NORMAL, ("WARNING: payload types for iLBC 13.3 "
            "and iLBC 15.2 downstream should be identical.\n"));
   }

   return ret;
}


/* ============================= */
/* Global function definition    */
/* ============================= */


/**
  Enables or Disables voice coder channel according to nMode
\param pChannel        Handle to TAPI_CHANNEL structure
\param nMode           0: off, 1: on
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function is not a TAPI low level function because it is not needed
   by TAPI.
   To succesfully modify the coder channel settings, at least the two first
   command Words should be programed.
   Function should protect access to fw messages
   After activating the coder activate stored ET bits: DTMF receiver and
   generator, ATD, UTD. For FW_ETU Sig-Ch and no generator is touched.
   Remember all ET bits and switch ET bits to zero: DTMF receiver and
   generator, ATD, UTD. For ETU Sig-Ch and no generator.
   Reset the signaling channel with VINETIC_SIG_SigReset.
   This is neccessary to initialze the event playout unit. Otherwise the
   sequence number may not match for the new connection and events could
   be played out.
*/
IFX_return_t VINETIC_COD_Voice_Enable (VINETIC_CHANNEL *pCh, IFX_uint8_t nMode)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t ch = pCh->nChannel - 1;
   FWM_COD_CH *pCodCh = &pCh->pCOD->fw_cod_ch;
   IFX_int32_t ret = IFX_SUCCESS, bSet = IFX_FALSE, i;

   /* protect fwmsg against concurrent tasks */
   IFXOS_MutexLock (pDev->memberAcc);

   /* for VIP some special handling is needed */
   if ((pDev->nChipMajorRev == VINETIC_V1x) &&
       (pCh->nChannel - 1 == 3) && (nMode == 1))
   {
      /* check if any CPTD is activated */
      for (i = 0; i < pDev->caps.nCPTD; i++)
      {
         if ( (VINETIC_SIG_CPTD_Get_Status (pDev, i)) == 1)
         {
            /* unlock */
            IFXOS_MutexUnlock (pDev->memberAcc);
            SET_ERROR(VIN_ERR_NORESOURCE);
            return IFX_ERROR;
         }
      }
   }
   /* switch on */
   if ((nMode == 1) && !(pCodCh->bit.en))
   {
      pCodCh->bit.en = 1;
      bSet = IFX_TRUE;
      /* switch signaling channel on */
      ret = Dsp_SigActStatus (pDev, ch, IFX_TRUE, CmdWrite);
#ifndef FW_ETU
      /* always activate the DTMF generator */
      if (ret == IFX_SUCCESS &&
          VINETIC_SIG_DTMFG_Get_Status (pDev,ch) == 0)
      {
         ret = VINETIC_SIG_DTMFG_Enable (pDev,ch);
      }
#endif /* FW_ETU */
   }
   /* switch off */
   else if ((nMode == 0) && (pCodCh->bit.en))
   {
      SIG_MOD_STATE state;
      state.value = 0x0;
      pCodCh->bit.en = 0;
      bSet = IFX_TRUE;

      /***********************************************************************
       switch off ET of every algorithm
       ***********************************************************************/
      ret = VINETIC_SIG_UpdateEventTrans (pDev, ch, &state, IFX_NULL);

#ifndef FW_ETU
      if (ret == IFX_SUCCESS &&
          VINETIC_SIG_DTMFG_Get_Status (pDev, ch) == 1)
      {
         VINETIC_SIG_DTMFG_Disable (pDev, ch);
         /*ret = CmdWrite (pDev, pDev->pSigCh[ch].sig_dtmfgen.value,*/
         /*                CMD_SIG_DTMFGEN_LEN);*/
      }
#endif /* FW_ETU */
      if (ret == IFX_SUCCESS)
         ret = Dsp_SigActStatus (pDev, ch, IFX_FALSE, CmdWrite);
   }

   if (ret == IFX_SUCCESS && bSet)
      ret = CmdWrite (pDev, (IFX_uint16_t *) pCodCh, 2);

   /* activate the event transmission if configured */
   if (ret == IFX_SUCCESS && nMode == 1 &&
       VINETIC_SIG_Event_Tx_Status_Get (pDev, ch) != 0)
   {
      ret = VINETIC_SIG_UpdateEventTrans (pDev, ch, IFX_NULL, IFX_NULL);
   }

   /* unlock */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}

/**
   sets the Voice Activation Detection mode.
\param pChannel        Handle to TAPI_CHANNEL structure
\param nVAD            IFX_TRUE: VAD switched on
                     IFX_FALSE: VAD is off
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function sets the SC bit (Silence Compression) of Coder Channel's
   cofiguration if VAD is on. Otherwise, it sets it to zero. Recording Codec
   G.729E (11.8kbps) does not support VAD.
   In Case of VAD with comfort noise generation, CNG bit will be set only if
   Codec is G711.
   SIC maybe modified in G.711, G.723 and ILBC on fly.
   The SIC bit is don't care for G728, G729E.
   For G.729 A/B coder must be stopped and started (ENC =0), which resets
   the statistics -> recommend to user to do it before coder start.
*/
IFX_return_t IFX_TAPI_LL_COD_VAD_Cfg (IFX_TAPI_LL_CH_t *pLLChannel, IFX_int32_t nVAD)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_COD_CH      *pCodCh = &pCh->pCOD->fw_cod_ch;
   IFX_int32_t      ret    = IFX_SUCCESS;
   IFX_uint32_t sc_bit, cng_bit, ns_bit;

   /* protect fw messages */
   IFXOS_MutexLock (pDev->memberAcc);
   /* do appropriate VAD setting */

   /* store the current register bits into local variables.
      Only these bit are getting modified in this function */
   ns_bit = pCodCh->bit.ns;
   cng_bit = pCodCh->bit.cng;
   sc_bit = pCodCh->bit.sc;
      /* set Coder Channel write command */

   ns_bit = 0;
   switch (nVAD)
   {
   case  IFX_TAPI_ENC_VAD_NOVAD:
      /* deactivate silence compression, deactivate comfort noise generation */
      sc_bit = 0;
      cng_bit = 0;
      break;
   case IFX_TAPI_ENC_VAD_CNG_ONLY:
      /* deactivate silence compression, activate comfort noise generation */
      sc_bit = 0;
      cng_bit = 1;
      break;
   case IFX_TAPI_ENC_VAD_SC_ONLY:
      /* activate silence compression, deactivate comfort noise generation */
      sc_bit = 1;
      ns_bit = 1;
      cng_bit = 0;
      break;
   default:
      /* activate silence compression */
      sc_bit = 1;
      ns_bit = 1;
      cng_bit = 1;
      break;
   }

   /* check if there is a need to send an update firmware message.
      only send a message if at least one bit has changed */

   if ((ns_bit != pCodCh->bit.ns) ||
       (cng_bit != pCodCh->bit.cng) ||
       (sc_bit != pCodCh->bit.sc))
   {
      /* it is only allowed to change the SC bit
         if the encoder is running G.729A or G.729B */
      if ((pCodCh->bit.enc == COD_CH_G729_ENC) &&
         (sc_bit != pCodCh->bit.sc))
      {
         /* firmware bits have changed and we want to
            write a firmware message. This is not allowed
            when the encoder is currently running. */
         SET_ERROR (VIN_ERR_COD_RUNNING);
         ret = IFX_ERROR;
      }

      /* disallow setting while room-noise detection is running */
      if (pCh->pCOD->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
      {
         ret = IFX_ERROR;
      }

      if (ret == IFX_SUCCESS)
      {
         /* write the modified bits to the firmware message
         and send it to the command mailbox */
         pCodCh->bit.ns = ns_bit;
         pCodCh->bit.cng = cng_bit;
         pCodCh->bit.sc = sc_bit;

         /*  set the requested VAD mode*/
         ret = CmdWrite (pDev, pCodCh->value, 2);
      }
   }

   /* release locks */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}


#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
/** Turns the room noise detection mode on or off
\param pLLCh        Pointer to Low-level channel structure
\param bEnable      IFX_TRUE to enable or IFX_FALSE to disable
\param nThreshold   detection level in minus dB
\param nVoicePktCnt    count of consecutive voice packets required for event
\param nSilencePktCnt  count of consecutive silence packets required for event
\remarks
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_RoomNoise (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_boolean_t bEnable,
                                            IFX_uint32_t nThreshold,
                                            IFX_uint8_t nVoicePktCnt,
                                            IFX_uint8_t nSilencePktCnt)
{

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_COD_CH   fwm_cod_ch;
   IFX_uint16_t pCmd [6], nTmpWord;
   IFX_int32_t ret = IFX_SUCCESS;

   if (bEnable)
   {
      /* Enable room-noise-detection */

      /* Make sure encoder is in idle mode */
      IFXOS_MutexLock (pCh->chAcc);
      if (pCh->pCOD->nOpMode != vinetic_cod_OPMODE_IDLE)
      {
         /* if room-noise is running just return success otherwise error */
         if (pCh->pCOD->nOpMode != vinetic_cod_OPMODE_ROOMNOISE)
         {
            SET_ERROR (VIN_ERR_COD_RUNNING);
            ret = IFX_ERROR;
         }
         IFXOS_MutexUnlock (pCh->chAcc);
         return ret;
      }

      /* Limit the threshold level to a maximum of -96 dB */
      if (nThreshold > 96)
      {
         nThreshold = 96;
      }
      /* Store the count values for use by the irq routine */
      pCh->pCOD->nSilencePktCnt = nSilencePktCnt;
      pCh->pCOD->nVoicePktCnt = nVoicePktCnt;

      /* Get the PT table for upstream and set PT for G.711 aLaw */
      pCh->pCOD->pRtpPtCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
      pCh->pCOD->pRtpPtCmd [1] = ECMD_COD_CHRTP_TX;

      if (CmdRead(pDev, pCh->pCOD->pRtpPtCmd,
                        pCh->pCOD->pRtpPtCmd, 18) != IFX_SUCCESS)
      {
         IFXOS_MutexUnlock (pCh->chAcc);
         return IFX_ERROR;
      }

      nTmpWord = pCh->pCOD->pRtpPtCmd [5];
      pCh->pCOD->pRtpPtCmd [5] = 0x8880;
      if (CmdWrite(pDev, pCh->pCOD->pRtpPtCmd, 18) != IFX_SUCCESS)
      {
         IFXOS_MutexUnlock (pCh->chAcc);
         return IFX_ERROR;
      }
      pCh->pCOD->pRtpPtCmd [5] = nTmpWord;

      /* Set the VAD coefficients with a different limiter level */
      /* Command is sent by broadcast so channel is 0 */
      pCmd [0] = CMD1_BC | CMD1_EOP;
      pCmd [1] = ECMD_VAD_COEFF;
      pCmd [2] = 0x1000 | (VadCoeff_Reg_LIM[nThreshold / 3]);
      pCmd [3] = 0x7F06;
      pCmd [4] = 0x8882;
      pCmd [5] = 0x8128;

      if (CmdWrite(pDev, pCmd, 4) != IFX_SUCCESS)
      {
         IFXOS_MutexUnlock (pCh->chAcc);
         return IFX_ERROR;
      }

      /* Enable the coder */

      /* copy the current coder channel configuration */
      IFXOS_MutexLock (pDev->memberAcc);
      fwm_cod_ch = pCh->pCOD->fw_cod_ch;
      IFXOS_MutexUnlock (pDev->memberAcc);

      /* change the configuration and write it */
      fwm_cod_ch.bit.en = 1;
      fwm_cod_ch.bit.ns = 1;
      fwm_cod_ch.bit.em = 0;
      fwm_cod_ch.bit.im = 0;
      fwm_cod_ch.bit.pst = 1;
      fwm_cod_ch.bit.sc = 1;
      fwm_cod_ch.bit.pte = COD_CH_PTE_10MS;
      fwm_cod_ch.bit.enc = COD_CH_G711_ALAW_ENC;

      ret = CmdWrite (pDev, (IFX_uint16_t *) &fwm_cod_ch, 3);

      if (ret == IFX_SUCCESS)
      {
         pCh->pCOD->nOpMode = vinetic_cod_OPMODE_ROOMNOISE;
         pCh->pCOD->nLastEventPt = 0;
      }

      /* Set the VAD coefficients back to the reset values */
      pCmd [2] = 0x102A;
      pCmd [5] = 0x8528;

      if (CmdWrite(pDev, pCmd, 4)  != IFX_SUCCESS)
      {
         ret = IFX_ERROR;
      }

      IFXOS_MutexUnlock (pCh->chAcc);
   }
   else
   {
      /* Disable room-noise-detection */

      /* Make sure encoder is in room-noise-detection mode */
      IFXOS_MutexLock (pCh->chAcc);
      if (pCh->pCOD->nOpMode != vinetic_cod_OPMODE_ROOMNOISE)
      {
         /* if already stopped just return success otherwise error */
         if (pCh->pCOD->nOpMode != vinetic_cod_OPMODE_IDLE)
         {
            SET_ERROR (VIN_ERR_COD_RUNNING);
            ret = IFX_ERROR;
         }
         IFXOS_MutexUnlock (pCh->chAcc);
         return ret;
      }

      /* Stop the coder */
      ret = CmdWrite (pDev, (IFX_uint16_t *) &pCh->pCOD->fw_cod_ch, 2);

      /* restore the PT table */
      if (ret == IFX_SUCCESS)
         ret = CmdWrite(pDev, pCh->pCOD->pRtpPtCmd, 18);

      pCh->pCOD->nOpMode = vinetic_cod_OPMODE_IDLE;
      IFXOS_MutexUnlock (pCh->chAcc);
   }

   return ret;
}


/** Analyse the packet stream for room noise detection and send events
\param pCh          Pointer to VINETIC channel structure
\param nPT          Payload type of the packet
\remarks
This function requires that the payload type only has two values. It should
indicate a SID packet (0x0D) or any voice packet. The PT value for the voice
packet is irrelevant as long as it stays the same over the entire detection
period. This is normally not a problem since the coder will be set to only
one codec during room-noise detection operation.
*/
void irq_VINETIC_COD_RoomNoise_PktAnalysis (VINETIC_CHANNEL *pCh,
                                            IFX_uint8_t nPT)
{
   IFX_TAPI_EVENT_t tapiEvent;

   /* set threshold counter on changes of PT */
   if (nPT != pCh->pCOD->nLastPktPt)
   {
      if (nPT == PKT_VOP_PT_SID)
      {
         pCh->pCOD->nThresholdCtr = pCh->pCOD->nSilencePktCnt;
      }
      else
      {
         pCh->pCOD->nThresholdCtr = pCh->pCOD->nVoicePktCnt;
      }
   }
   /* store the last PT */
   pCh->pCOD->nLastPktPt = nPT;

   /* send event on 1 - counter continues until 0
      this way the event gets only sent once while packets arrive
      There is also a special condition to send one packet at start of
      detection when the nLastEventPt is zero. */
   if (((pCh->pCOD->nThresholdCtr == 1) && (pCh->pCOD->nLastEventPt != nPT)) ||
       (pCh->pCOD->nLastEventPt == 0) )
   {
      /* send TAPI event */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      if (nPT == PKT_VOP_PT_SID)
      {
         tapiEvent.id = IFX_TAPI_EVENT_COD_ROOM_SILENCE;
      }
      else
      {
         tapiEvent.id = IFX_TAPI_EVENT_COD_ROOM_NOISE;
      }
      IFX_TAPI_Event_Dispatch((TAPI_CHANNEL *)pCh->pTapiCh, &tapiEvent);
      pCh->pCOD->nLastEventPt = nPT;
   }
   /* count until threshold is 0 */
   if (pCh->pCOD->nThresholdCtr > 0)
   {
      pCh->pCOD->nThresholdCtr--;
   }

   return;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */


/**
   Configures a set of parameters related to the AGC (Automatic Gain Control)
   block, available in the Encoder direction only. AGC functional block is part
   of the Coder module. Not all configurable parameters can be configured with
   this interface function. See below for a list of parameters which retain their
   reset values.

   \param
     - pLLChannel  handle to VINETIC_CONNECTION structure
     - pAGC_Cfg    handle to IFX_TAPI_ENC_AGC_CFG_t structure

   \return
      - IFX_ERROR if error
      - IFX_SUCCESS if successful

   \remarks
   The parameters passed in IFX_TAPI_ENC_AGC_CFG_t data structure are in dB,
   so mapping tables are used to convert them into hex value suitable for
   programming.
*/
IFX_return_t IFX_TAPI_LL_COD_AGC_Cfg(IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_ENC_AGC_CFG_t *pAGC_Cfg)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t pCmd [2], pData [7];
   IFX_uint8_t  regCOM, regGAIN, regATT, regLIM;


   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_AGC_COEF;

   if (CmdRead(pDev, pCmd, pData, 5) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   /* Check if given parameter are in range */
   if ((pAGC_Cfg->com > AGC_CONFIG_COM_MAX)    ||
       (pAGC_Cfg->com < AGC_CONFIG_COM_MIN)    ||
       (pAGC_Cfg->gain > AGC_CONFIG_GAIN_MAX)  ||
       (pAGC_Cfg->gain < AGC_CONFIG_GAIN_MIN)  ||
       (pAGC_Cfg->att > AGC_CONFIG_ATT_MAX)    ||
       (pAGC_Cfg->att < AGC_CONFIG_ATT_MIN)    ||
       (pAGC_Cfg->lim > AGC_CONFIG_LIM_MAX)    ||
       (pAGC_Cfg->lim < AGC_CONFIG_LIM_MIN))
   {
      return IFX_ERROR;
   }

   /* Get the register values from the 'dB' values */
   regCOM   = AgcConfig_Reg_COM[pAGC_Cfg->com + AGC_CONFIG_COM_OFFSET];
   regGAIN  = AgcConfig_Reg_GAIN[pAGC_Cfg->gain + AGC_CONFIG_GAIN_OFFSET];
   regATT   = AgcConfig_Reg_ATT[pAGC_Cfg->att + AGC_CONFIG_ATT_OFFSET];
   regLIM   = AgcConfig_Reg_LIM[pAGC_Cfg->lim + AGC_CONFIG_LIM_OFFSET];

   /* Update with the given parameter and write the message back to the
      device */

   /* Set the 'COM' value in the message and keep the 'GAIN-INT' value */
   pData [2] = (regCOM << 8) | (pData [2] & 0xFF);

   /* Set the 'GAIN' value and the 'ATT' value in the message */
   pData [4] = (regGAIN << 8) | regATT;

   /* Set the 'LIM' value in the message and keep the 'DEC' value */
   pData [5] = (pData [5] & 0xFF00) | regLIM;

   if (CmdWrite(pDev, pData, 5) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Enable/Disable the AGC (Automatic Gain Control) block

   \param pLLChannel Handle to VINETIC_CONNECTION structure
   \param agcMode
         IFX_TAPI_ENC_AGC_MODE_ENABLE  - to enable AGC
         IFX_TAPI_ENC_AGC_MODE_DISABLE - to disable AGC

   \return Return value according to IFX_return_t
            - IFX_ERROR if an error occured
            - IFX_SUCCESS if successful

   \remarks
   This implementation assumes that the index of the AGC resource is fixed
   assigned to the related index of the Coder-Module.
   If the AGC resource is enable or disable, it depends on 'agcMode'.
*/
IFX_return_t IFX_TAPI_LL_COD_AGC_Enable(IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_ENC_AGC_MODE_t agcMode)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t pData [3];


   pData [0] = CMD1_EOP | (pCh->nChannel - 1);
   pData [1] = ECMD_COD_AGC;

   pData [2] = (agcMode ? 0x8000 : 0) | (pCh->nChannel - 1);

   if (CmdWrite (pDev, pData, 1) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
#else
   return IFX_ERROR;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
}


/**
   Starts the Playing.
\param pChannel        Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
  This function does the same as TAPI_LL_Phone_Start_Recording :
  start Recording / playing are done by enabling the coder channel.
*/

IFX_return_t IFX_TAPI_LL_COD_DEC_Start(IFX_TAPI_LL_CH_t *pLLChannel)
{
   IFX_int32_t ret = IFX_ERROR;

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pDev->pChannel[pCh->nChannel - 1].pCOD;

   if (pCodCh->fw_cod_ch.bit.dec == 0)
   {
      /* protect fwmsg against concurrent tasks */
      IFXOS_MutexLock (pDev->memberAcc);
      pCodCh->fw_cod_ch.bit.dec = 1;
      /* unlock */
      IFXOS_MutexUnlock (pDev->memberAcc);

      if (pCodCh->fw_cod_ch.bit.en == 0)
      {
         if (pDev->pChannel[pCh->nChannel - 1].pCOD->fw_cod_ch.bit.en == 1)
         {
            SET_ERROR (VIN_ERR_T38_RUNNING);
            return IFX_ERROR;
         }
         /* The 'en' bit has to be set too. function
          * 'VINETIC_COD_Voice_Enable'
            writes the firmware message to the device */

         ret = VINETIC_COD_Voice_Enable (pCh, 1);

         if (ret != IFX_SUCCESS)
         {
            /* enabling the coder-module failed, mark this in the 'dec'
               parameter */
            /* protect fwmsg against concurrent tasks */
            IFXOS_MutexLock (pDev->memberAcc);
            pCodCh->fw_cod_ch.bit.dec = 0;
            /* unlock */
            IFXOS_MutexUnlock (pDev->memberAcc);
         }
      }
      else
      {
         /* The 'en' bit was already set in the firmware message but the
            parameter 'dec' has changed. Only if the 'dec' bit is zero, the
            firmware message has to be sent to the device */

         /* protect fwmsg against concurrent tasks */
         IFXOS_MutexLock (pDev->memberAcc);
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 4);
         /* unlock */
         IFXOS_MutexUnlock (pDev->memberAcc);
      }
   }
   else
      ret = IFX_SUCCESS;

#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

   return ret;
}


/**
   Stops the Playing.
\param pChannel        Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
  This function does the same as TAPI_LL_Phone_Stop_Recording :
  stop Recording / playing are done by disabling the coder channel.
*/
IFX_return_t IFX_TAPI_LL_COD_DEC_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   IFX_int32_t ret = IFX_ERROR;

#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pDev->pChannel[pCh->nChannel - 1].pCOD;

   if (pCodCh->fw_cod_ch.bit.dec)
   {
      /* decoder has to be updated */
      /* protect fwmsg against concurrent tasks */
      IFXOS_MutexLock (pDev->memberAcc);
      pCodCh->fw_cod_ch.bit.dec = 0;
      /* unlock */
      IFXOS_MutexUnlock (pDev->memberAcc);

      if (pCodCh->fw_cod_ch.bit.enc == 0)
      {
         /* encoder is not running -> disable the whole coder-module */
         ret = VINETIC_COD_Voice_Enable (pCh, 0);
         if (ret != IFX_SUCCESS)
            return ret;
         pCh->bVoiceConnect = IFX_FALSE;
      }
      else
      {
         /* encoder is still running -> do not disable the whole coder-module
            send the updated firmware message to the device */
         IFXOS_MutexLock (pDev->memberAcc);
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 2);
         /* unlock */
         IFXOS_MutexUnlock (pDev->memberAcc);
      }
   }
   else
      ret = IFX_SUCCESS;

#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

   return ret;
}


/**
   Switches on/off the HP filter in the decoder path of the COD module

\param pLLChannel Handle to TAPI low level channel structure
\param bHp        IFX_FALSE to switch HP filter off
                  IFX_TRUE  to switch HP filter on
\return
   IFX_ERROR on success, IFX_ERROR on error
*/
IFX_return_t IFX_TAPI_LL_COD_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_boolean_t bHp)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   FWM_COD_CH *p_fw_cod_ch = &pCh->pCOD->fw_cod_ch;
   IFX_int32_t ret = IFX_SUCCESS;
   unsigned newHP = (bHp == IFX_TRUE) ? 1 : 0;

   /* protect fwmsg access against concurrent tasks */
   IFXOS_MutexLock (pDev->memberAcc);

   if (p_fw_cod_ch->bit.hp != newHP)
   {
      /* store new HP setting */
      p_fw_cod_ch->bit.hp = newHP;

      /* if decoder is currently running write new setting to fw */
      if ((p_fw_cod_ch->bit.en != 0) && (p_fw_cod_ch->bit.dec != 0))
      {
         ret = CmdWrite (pDev, (IFX_uint16_t *) p_fw_cod_ch, 2);
      }
   }

   /* unlock protection */
   IFXOS_MutexUnlock (pDev->memberAcc);

   return ret;
}


#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
/**
   Starts the coder in upstream / record data
\param pChannel        Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
  This function enables the coder channel with the existing
 configuration.
  It resets, also, the members of the VINETIC_CHANNEL structure concerning the
  streaming and clears the channel Fifo, dedicated for incoming voice packets.
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_Start (IFX_TAPI_LL_CH_t *pLLChannel)
{
   IFX_int32_t ret = IFX_SUCCESS;

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;

   /* disallow setting while room-noise detection is running */
   if (pCh->pCOD->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
   {
      ret = IFX_ERROR;
   }
   else
   if (pCodCh->fw_cod_ch.bit.enc != pCodCh->enc_conf)
   {
#if 0
#if (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38)
      if (pCh->pCOD->fw_cod_fax_ctrl.EN == 1)
      {
         SET_ERROR (VIN_ERR_T38_RUNNING);
         return IFX_ERROR;
      }
#endif /* (VMMC_CFG_FEATURES& VMMC_FEAT_FAX_T38) */
#endif
      /* encoder has to be updated */
      /* protect fwmsg against concurrent tasks */
      IFXOS_MutexLock (pDev->memberAcc);
      pCodCh->fw_cod_ch.bit.enc = pCodCh->enc_conf;
      IFXOS_MutexUnlock (pDev->memberAcc);

      if (pCodCh->fw_cod_ch.bit.en == 0)
      {
         /* The 'en' bit has to be set too. function
          * 'VINETIC_COD_Voice_Enable'
            writes the firmware message to the device */
         ret = VINETIC_COD_Voice_Enable (pCh, 1);

         if (ret != IFX_SUCCESS)
         {
            /* enabling the coder-module failed, mark this in the 'enc'
               parameter */
            /* protect fwmsg against concurrent tasks */
            IFXOS_MutexLock (pDev->memberAcc);
            pCodCh->fw_cod_ch.bit.enc = 0;
            IFXOS_MutexUnlock (pDev->memberAcc);
         }
         else
         {
            pCh->pCOD->nOpMode = vinetic_cod_OPMODE_VOICE;
         }
      }
      else
      {
         /* The 'en' bit was already set in the firmware message but the
            parameter 'enc' has changed. The firmware message has to be sent to
            the device */

         /* protect fwmsg against concurrent tasks */
         IFXOS_MutexLock (pDev->memberAcc);
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 4);
         IFXOS_MutexUnlock (pDev->memberAcc);
      }
   }
   else
   {
      /* nothing to do. Encoder is already running */
      return (IFX_SUCCESS);
   }

   if (ret == IFX_SUCCESS)
   {
      /* set voice connection flag */
      pCh->bVoiceConnect = IFX_TRUE;
      /* Clear fifo for this channel */
      Vinetic_IrqLockDevice (pDev);

      TAPI_ClearFifo(TAPI_UpStreamFifo_Get(pCh->pTapiCh));

      Vinetic_IrqUnlockDevice (pDev);
   }

   return ret;
}


/**
   Stops the Recording.
\param pChannel        Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function disables the Coder Channel module.
   flag of the VINETIC_CHANNEL structure and wakes up processes to read
   the last arrived data.
   The function calls VINETIC_COD_Voice_Enable which also switches the event
  transmission after the coder is stopped. It is switched on from the
  cached structure in TAPI_LL_Start_Recording. Modifing on while the
  coder is stopped is done in the cached strucutre

*/
IFX_return_t IFX_TAPI_LL_COD_ENC_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   IFX_int32_t ret = IFX_ERROR;

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;

   if (pCodCh->fw_cod_ch.bit.enc)
   {
      /* encoder has to be updated */
      /* protect fwmsg against concurrent tasks */
      IFXOS_MutexLock (pDev->memberAcc);
      pCodCh->fw_cod_ch.bit.enc = 0;
      IFXOS_MutexUnlock (pDev->memberAcc);

      if (pCodCh->fw_cod_ch.bit.dec == 0)
      {
         /* decoder is not running -> disable the whole coder-module */
         /* protect fwmsg against concurrent tasks */

         ret = VINETIC_COD_Voice_Enable (pCh, 0);
         if (ret != IFX_SUCCESS)
            return ret;
         pCh->bVoiceConnect = IFX_FALSE;
         pCh->pCOD->nOpMode = vinetic_cod_OPMODE_IDLE;
      }
      else
      {
         /* decoder is still running -> do not disable the whole coder-module
            send the updated firmware message to the device */
         IFXOS_MutexLock (pDev->memberAcc);
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 2);
         pCh->pCOD->nOpMode = vinetic_cod_OPMODE_IDLE;
         IFXOS_MutexUnlock (pDev->memberAcc);
      }
   }
   else
      ret = IFX_SUCCESS;

   /* do not try to wakeup or reset the fifos here (!) */
   return ret;
}


/**
   Put encoder in hold/unhold state.

   \param pLLChannel        Handle to VINETIC_CHANNEL structure
   \param nOnHold           IFX_ENABLE - encoder is put to hold
                            IFX_DISABLE - encoder is put to unhold

   \return Return value according to IFX_return_t
           - IFX_ERROR if an error occured
           - IFX_SUCCESS if successful

   \remarks
   This function temporarily stop and restart packet encoding, for example to
   hold/unhold the remote VoIP party.
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_Hold(IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_operation_t nOnHold)
{
   IFX_return_t ret = IFX_ERROR;

   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: IFX_TAPI_LL_COD_ENC_Hold, called\n"));

   /* disallow setting while room-noise detection is running */
   if (pCodCh->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
   {
      return IFX_ERROR;
   }

   switch (nOnHold)
   {
      case IFX_DISABLE:
         TRACE(VINETIC, DBG_LEVEL_LOW,
               ("INFO: IFX_TAPI_LL_COD_ENC_Hold: Dev%d,Ch%d: unset encoder hold\n",
               pDev->nDevNr, pCh->nChannel - 1));
         pCodCh->fw_cod_ch.bit.em = 0;
         break;

      case IFX_ENABLE:
         TRACE(VINETIC, DBG_LEVEL_LOW,
               ("INFO: IFX_TAPI_LL_COD_ENC_Hold: Dev%d,Ch%d: set encoder hold\n",
               pDev->nDevNr, pCh->nChannel - 1));
         pCodCh->fw_cod_ch.bit.em = 1;
         break;

      default:
         return IFX_ERROR;
   }

   ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 2);

   return ret;
}


/**
   Sets the Recording Codec and packet length
\param pChannel        Handle to VINETIC_CHANNEL structure
\param nCoder          Selected coder type
\param nFrameLength    Length of frames to be generated by the coder
\param nBitPack        selecting RTP default or ITU-T I366.2 Bit Alignment
                       for G.726 codecs
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_Cfg_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_COD_TYPE_t nCoder,
                                          IFX_TAPI_COD_LENGTH_t nFrameLength,
                                          IFX_TAPI_COD_AAL2_BITPACK_t nBitPack)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_return_t ret = IFX_SUCCESS;
   IFX_int8_t  nCod, nPte;

   /* Translate parameters from TAPI to FW values */
   nCod = VINETIC_COD_trans_cod_tapi2fw (nCoder);
   nPte = vinetic_cod_trans_fl_tapi2fw(nFrameLength);

   /* check for invalid parameter values */
   if ((nCod == IFX_ERROR) || (nPte == IFX_ERROR))
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: Parameter values not supported.\n\r"));
      ret = IFX_ERROR;
   }

   /* check parameters for invalid combinations
      only specific coder/frame length combinations are supported */
   if (ret == IFX_SUCCESS)
   {
      if ((ret = CheckENCnPTE((IFX_uint8_t)nPte,
                              (IFX_uint8_t)nCod)) != IFX_SUCCESS)
      {
         TRACE (VINETIC,DBG_LEVEL_HIGH,
             ("\n\rDRV_ERROR: Parameter combination not supported.\n\r"));
      }
   }

   /* check whether the requested encoder type is supported by firmware */
   if (ret == IFX_SUCCESS)
   {
      if (vinetic_cod_codec_support_check(pDev, nCod) == IFX_FALSE)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("\n\rDRV_ERROR: Requested encoder type (0x%02x) not supported "
                 "by this firmware.\n\r", nCoder));
         SET_ERROR(VIN_ERR_CODCONF_NOTVALID);
         ret = IFX_ERROR;
      }
   }

   if (ret == IFX_SUCCESS)
   {
      IFXOS_MutexLock (pDev->memberAcc);

      /* store the values in the cached variable/fw-message */
      pCodCh->enc_conf = (IFX_uint8_t)nCod;
      pCodCh->fw_cod_ch.bit.pte = (IFX_uint8_t)nPte;

      if (!((nBitPack == IFX_TAPI_COD_AAL2_BITPACK) &&
           ((nCoder == IFX_TAPI_COD_TYPE_G726_16) ||
            (nCoder == IFX_TAPI_COD_TYPE_G726_24) ||
            (nCoder == IFX_TAPI_COD_TYPE_G726_32) ||
            (nCoder == IFX_TAPI_COD_TYPE_G726_40))))
      {
         pCodCh->fw_cod_ch.bit.bitalign_enc_aal = 0;
      }
      else
      {
         pCodCh->fw_cod_ch.bit.bitalign_enc_aal = 1;
      }

      /* if the encoder is running send the message to fw
         running state is determined by the enc field of the fw message
         because the enable bit also controls the decoder which might be
         running independently of the encoder. */
      if (pCodCh->fw_cod_ch.bit.enc)
      {
         /* encoder is enabled, write the values to the device */
         pCodCh->fw_cod_ch.bit.enc = (IFX_uint8_t)nCod;
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 4);
      }

      IFXOS_MutexUnlock (pDev->memberAcc);
   }

   return ret;
}

/**
   Configures the Decoder
\param pChannel        handle to VINETIC_CHANNEL structure
\param nBitPack        selecting RTP default or ITU-T I366.2 Bit Alignment
                       for G.726 codecs
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_return_t IFX_TAPI_LL_COD_DEC_Cfg_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_COD_AAL2_BITPACK_t nBitPack)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_int32_t     ret = IFX_SUCCESS;

   IFXOS_MutexLock (pDev->memberAcc);
   switch (nBitPack)
   {
      case IFX_TAPI_COD_RTP_BITPACK:
         pCh->pCOD->fw_cod_ch.bit.bitalign_dec_aal = 0;
         break;
      case IFX_TAPI_COD_AAL2_BITPACK:
         pCh->pCOD->fw_cod_ch.bit.bitalign_dec_aal = 1;
         break;
   }
   IFXOS_MutexUnlock (pDev->memberAcc);

   /* if the decoder is running send the message to fw
      running state is determined by the dec field of the fw message */
   if (pCodCh->fw_cod_ch.bit.dec)
   {
      ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 4);
   }
   return ret;
}

/**
   Sets the Recording Codec
   OBSOLETED by IFX_TAPI_LL_COD_ENC_Cfg_Set
\param pChannel        Handle to TAPI_CHANNEL structure
\param nCodec          the codec for the record audio channel
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function sets the recording codec and a suitable PTE value.
   Coder channel module remains disabled after the completion of
   this function. If the user needs another PTE, one can program this
   through the appropriate interface.

   The PTE value set as default are shown below

   - G.711 Alaw         : 10 ms
   - G.711 Mulaw        : 10 ms
   - G.723 5.3 Kbps     : 30 ms - no other choice possible
   - G.723 6.3 Kbps     : 30 ms - no other choice possible
   - G.728              : 10 ms
   - G.729_AB           : 10 ms
   - G.729_E            : 10 ms
   - G.726 16 Kbps      : 10 ms
   - G.726 24 kbps      : 10 ms
   - G.726 32 Kbps      : 10 ms
   - G.726 40 Kbps      : 10 ms
   - iLBC  13.3Kbps     : 30 ms - no other choice possible
   - iLBC  15.2Kbps     : 20 ms - no other choice possible

   This function also caches the actual codec set.
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_CoderType_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_int32_t nCoder)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_return_t ret = IFX_SUCCESS;
   IFX_int8_t nCod = 0, nPte = -1;

   /* check coder module */
   if (pDev->nDevState & DS_COD_EN)
   {
      /* map user selected coder to appropriate FW coder identifier */
      nCod = VINETIC_COD_trans_cod_tapi2fw (nCoder);

      /* check whether the requested encoder type is supported by firmware */
      if (vinetic_cod_codec_support_check(pDev, nCod) == IFX_FALSE)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("\n\rDRV_ERROR: Requested encoder type (0x%02x) not supported "
                 "by this firmware.\n\r", (IFX_uint16_t)nCoder));
         SET_ERROR(VIN_ERR_CODCONF_NOTVALID);
         ret = IFX_ERROR;
      }
   }
   else
   {
      SET_ERROR(VIN_ERR_CODCONF_NOTVALID);
      ret = IFX_ERROR;
   }

   /* PTE value suitable ? if not, search a suitable pte  */
   if ((ret == IFX_SUCCESS) && (nCod != IFX_ERROR))
   {
      /* get matching PTE to set */
      if (CheckENCnPTE ((IFX_uint8_t)pCodCh->fw_cod_ch.bit.pte,
                        (IFX_uint8_t)nCod) == IFX_ERROR)
      {
         /* in this case where the current frame-length is not supported by
            the selected coder, a best match is being found */
         nPte = getConvPTE ((IFX_uint8_t)pCodCh->fw_cod_ch.bit.pte, nCoder);
      }
   }
   else
   {
      SET_ERROR(VIN_ERR_CODCONF_NOTVALID);
      ret = IFX_ERROR;
   }

   /* set Encoder: The coder channel will be enabled at recording start.
      Suitable Pte from pte table is set. The settings must be protected
      against concurrent access
   */
   if (ret == IFX_SUCCESS)
   {
      IFX_boolean_t bUpdate = IFX_FALSE;

      IFXOS_MutexLock (pDev->memberAcc);

      /* store the given encoder parameter in the driver parameter */
      if (pCodCh->enc_conf != (IFX_uint8_t)nCod)
      {
         pCodCh->enc_conf = (IFX_uint8_t) nCod;
         if (pCodCh->fw_cod_ch.bit.enc)
         {
            /* encoder is enable, write the changed codec to the device */
            pCodCh->fw_cod_ch.bit.enc = (IFX_uint8_t) nCod;
            /* it is required to send a firmware message */
            bUpdate = IFX_TRUE;
         }
      }

      if (nPte != -1)
      {
         pCodCh->fw_cod_ch.bit.pte = (IFX_uint8_t) nPte;
         /* it is required to send a firmware message */
         bUpdate = IFX_TRUE;
      }

      /* do not update when room-noise detection is running */
      if (pCodCh->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
      {
         bUpdate = IFX_FALSE;
         ret = IFX_ERROR;
      }

      if (bUpdate != IFX_FALSE)
      {
         /* it is required to send a firmware message */
         ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 2);
      }

      IFXOS_MutexUnlock (pDev->memberAcc);
   }

   return ret;
}


/**
   Sets the frame length for the audio packets
   OBSOLETED by IFX_TAPI_LL_COD_ENC_Cfg_Set
\param pChannel        Handle to TAPI_CHANNEL structure
\param nFrameLength    length of frames in milliseconds
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function sets the frame length only when the requested value
   is valid in relation to the pre-existing Codec value of the Coder Channel
   configuration. Coder channel module remains either disabled or enabled
   after the completion of this function, depending on its previous state.
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_FrameLength_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_int32_t nFrameLength)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_int32_t      ret    = IFX_SUCCESS, nPte = 0;

   /* check coder module */
   if (pDev->nDevState & DS_COD_EN)
   {
      /* map frame length value in ms provided by the user in ms to FW value */
      nPte = getPTE (nFrameLength);
   }
   else
   {
      ret = IFX_ERROR;
      SET_ERROR(VIN_ERR_CODCONF_NOTVALID);
   }

   /* disallow setting while room-noise detection is running */
   if (pCh->pCOD->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
   {
      ret = IFX_ERROR;
   }

   /* check if PTE and pre existing ENC match together */
   if ((ret == IFX_SUCCESS) && (nPte != IFX_ERROR))
      ret = CheckENCnPTE ((IFX_uint8_t)nPte,
                          (IFX_uint8_t)pCodCh->fw_cod_ch.bit.enc);
   else
      ret = IFX_ERROR;
   /*  set the requested PTE value*/
   if (ret == IFX_SUCCESS)
   {
      IFXOS_MutexLock (pDev->memberAcc);
      pCodCh->fw_cod_ch.bit.pte = (IFX_uint32_t) nPte;
      ret = CmdWrite (pDev, (IFX_uint16_t *) & pCodCh->fw_cod_ch, 2);
      IFXOS_MutexUnlock (pDev->memberAcc);
   }

   return ret;
}


/**
   Retrieves the saved frame length for the audio packets, as saved when applied
   to the channel
\param pChannel        Handle to TAPI_CHANNEL structure
\param pFrameLength    Handle to length of frames in milliseconds
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_return_t IFX_TAPI_LL_COD_ENC_FrameLength_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_int32_t *pFrameLength)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_int32_t      nFrameLength;

   /* set frame length according to stored PTE */
   nFrameLength = getFrameLength (pCodCh->fw_cod_ch.bit.pte);
   if (nFrameLength == IFX_ERROR)
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: PTE value has no corresponding frame length.\n\r"));
      return IFX_ERROR;
   }
   *pFrameLength = nFrameLength;
   return IFX_SUCCESS;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

/**
   Sets the COD interface volume.

   Gain Parameter are given in 'dB'. The range is -24dB ... 24dB.

\param pLLChannel Handle to TAPI low level channel structure
\param pVol     Handle to IFX_TAPI_LINE_VOLUME_t structure
\return
*/
IFX_return_t IFX_TAPI_LL_COD_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_PKT_VOLUME_t const *pVol)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t  ret = IFX_SUCCESS;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD != IFX_NULL)
   {
      /* range check, because gain var is integer */
      if ((pVol->nEnc < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
          (pVol->nEnc > IFX_TAPI_LINE_VOLUME_MAX_GAIN) ||
          (pVol->nDec < IFX_TAPI_LINE_VOLUME_MIN_GAIN) ||
          (pVol->nDec > IFX_TAPI_LINE_VOLUME_MAX_GAIN))
      {
         /* parameter are out of supported range */
         TRACE(VINETIC, DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Volume Gain out of range for COD, "
               "(Enc=%ddB, Dec=%ddB, allowed:%d..%ddB)!\n\r",
               pVol->nEnc, pVol->nDec,
               IFX_TAPI_LINE_VOLUME_MIN_GAIN, IFX_TAPI_LINE_VOLUME_MAX_GAIN));
         ret = IFX_ERROR;
      }

      if (ret == IFX_SUCCESS)
      {
         /* protect fw msg */
         IFXOS_MutexLock (pDev->memberAcc);

         /* store actual settings into message cache */
         pCh->pCOD->fw_cod_ch.bit.gain1 =
            (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nEnc + 24];
         pCh->pCOD->fw_cod_ch.bit.gain2 =
            (IFX_uint32_t)VINETIC_AlmPcmGain [pVol->nDec + 24];

         /* if channel is enabled write to fw */
         if (pCh->pCOD->fw_cod_ch.bit.en != 0)
         {
            /* channel is enabled, write the updated msg to the device */
            ret = CmdWrite (pDev, (IFX_uint16_t *) & pCh->pCOD->fw_cod_ch, 3);
         }

         /* release lock */
         IFXOS_MutexUnlock (pDev->memberAcc);
      }
   }
   else
   {
      /* pCOD is not initialised in this channel */
      ret = IFX_ERROR;
   }

   return ret;
}

/**
   configures the jitter buffer
\param pChannel   Handle to TAPI_CHANNEL structure
\param pJbConf    Handle to IFX_TAPI_JB_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function caches the actual Jitter buffer configuration set.
*/
IFX_return_t IFX_TAPI_LL_COD_JB_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_JB_CFG_t const *pJbConf)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE  *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t    pData [6] = {0};
   IFX_int32_t     ret;

   /* setup command for jb configuration */
   pData [0] = CMD1_EOP | (pCh->nChannel - 1);
   pData [1] = ECMD_COD_CHJB;

   /* do JB settings */
   switch (pJbConf->nJbType)
   {
      case IFX_TAPI_JB_TYPE_FIXED:
         pData[2] &= ~COD_CHJB_ADAPT;
         break;
      case IFX_TAPI_JB_TYPE_ADAPTIVE:
         /* set adaptive type */
         pData[2] |= COD_CHJB_ADAPT;
         /* do settings for local adaptation and sample interpolation */
         switch (pJbConf->nLocalAdpt)
         {
            case IFX_TAPI_JB_LOCAL_ADAPT_OFF:
               pData [2] &= ~(COD_CHJB_LOC | COD_CHJB_SI);
               break;
            case IFX_TAPI_JB_LOCAL_ADAPT_ON:
               pData [2] |= COD_CHJB_LOC;
               break;
            case IFX_TAPI_JB_LOCAL_ADAPT_SI_ON:
               pData [2] |= (COD_CHJB_LOC | COD_CHJB_SI);
               break;
            default:
               TRACE (VINETIC,DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Wrong Parameter."
                     " Can't set Jitter Buffer\n\r"));
               return IFX_ERROR;
         }
         break;

      default:
         TRACE (VINETIC,DBG_LEVEL_HIGH,
               ("\n\rDRV_ERROR: Wrong Parameter. Can't set Jitter Buffer\n\r"));
         return IFX_ERROR;
   }
   switch (pJbConf->nPckAdpt)
   {
      case  0:
      case  1:
         /* not supported */
         break;

      case  IFX_TAPI_JB_PKT_ADAPT_VOICE:
         /* Packet adaption is optimized for voice. Reduced adjustment speed and
         packet repetition is off */
         /* ADAP = 1, ->see above PJE = 1 SF = default ->see below */
         pData [2] &= ~(COD_CHJB_RAD | COD_CHJB_PRP | COD_CHJB_DVF);
         break;

      case  IFX_TAPI_JB_PKT_ADAPT_DATA:
         /* Packet adaption is optimized for data */
         pData [2] |= COD_CHJB_RAD | COD_CHJB_PRP | COD_CHJB_DVF;
         break;

      default:
         SET_ERROR(VIN_ERR_FUNC_PARM);
         return IFX_ERROR;
   }

   pData [2] |= COD_CHJB_PJE;
   /* scaling factor */
   pData [2] |= ((IFX_uint8_t)pJbConf->nScaling << 8);
   /* Initial Size of JB */
   pData [3] = pJbConf->nInitialSize;
   /* minimum Size of JB */
   pData [4] = pJbConf->nMinSize;
   /* maximum Size of JB  */
   pData [5] = pJbConf->nMaxSize;
   ret = CmdWrite (pDev, pData, 4);

   return ret;
}


/**
   query jitter buffer statistics
\param pChannel          Handle to TAPI_CHANNEL structure
\param pJbData           Handle to IFX_TAPI_JB_STATISTICS_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_COD_JB_Stat_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_JB_STATISTICS_t *pJbData)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t    pCmd [2] = {0}, pData[30] = {0};
   IFX_uint32_t    val, sum;
   IFX_int32_t     ret = IFX_SUCCESS;

   /* setup command to read jb configuration */
   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_COD_CHJB;
   ret = CmdRead (pDev, pCmd, pData, 1);
   /* get type of configured jitter buffer */
   if (ret == IFX_SUCCESS)
   {
      switch (pData [2] & COD_CHJB_ADAPT)
      {
         case 0:
            /* jitter buffer type */
            pJbData->nType = IFX_TAPI_JB_TYPE_FIXED;
            break;
         case 1:
         default:
            /* jitter buffer type */
            pJbData->nType = IFX_TAPI_JB_TYPE_ADAPTIVE;
            break;
      }
      /* read Coder Channel JB Statistics */
      /* When extended JB statistic is available a length of 28 can be read
         otherwise only a length of 26 may be read. */
      pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
      pCmd [1] = ECMD_COD_CHJBSTAT;
      if (pDev->caps.bExtendedJBsupported)
         ret = CmdRead (pDev, pCmd, pData, 28);
      else
         ret = CmdRead (pDev, pCmd, pData, 26);
   }
   /* assign the JB statistics values to the corresponding members of the
      IFX_TAPI_JB_STATISTICS_t structure */
   if (ret == IFX_SUCCESS)
   {
      /* incoming time, not supported anymore */
      pJbData->nInTime           = 0;
      /* Comfort Noise Generation, not supported anymore */
      pJbData->nCNG              = 0;
      /* Bad Frame Interpolation, not supported anymore */
      pJbData->nBFI              = 0;
      /* max packet delay, not supported anymore */
      pJbData->nMaxDelay         = 0;
      /* minimum packet delay, not supported anymore */
      pJbData->nMinDelay         = 0;
      /* network jitter value, not supported anymore */
      pJbData->nNwJitter         = 0;
      /* play out delay */
      pJbData->nPODelay          = pData [2];
      /* max play out delay */
      pJbData->nMaxPODelay       = pData [3];
      /* min play out delay */
      pJbData->nMinPODelay       = pData [4];
      /* current jitter buffer size */
      pJbData->nBufSize          = pData [5];
      pJbData->nMaxBufSize       = pData [6];
      pJbData->nMinBufSize       = pData [7];
      /* received packet number  */
      pJbData->nPackets          = ((pData [8] << 16) | pData [9]);
      /* lost packets number, not supported anymore */
      pJbData->nLost             = 0;
      /* invalid packet number (discarded) */
      pJbData->nInvalid          = pData [10];
      /* duplication packet number, not supported anymore */
      pJbData->nDuplicate        =  0;
      /* late packets number */
      pJbData->nLate             = pData [11];
      /* early packets number */
      pJbData->nEarly            = pData [12];
      /* resynchronizations number */
      pJbData->nResync           = pData [13];
      /* new support */
      pJbData->nIsUnderflow      = ((pData [14] << 16) | pData [15]);
      pJbData->nIsNoUnderflow    = ((pData [16] << 16) | pData [17]);
      pJbData->nIsIncrement      = ((pData [18] << 16) | pData [19]);
      pJbData->nSkDecrement      = ((pData [20] << 16) | pData [21]);
      pJbData->nDsDecrement      = ((pData [22] << 16) | pData [23]);
      pJbData->nDsOverflow       = ((pData [24] << 16) | pData [25]);
      pJbData->nSid              = ((pData [26] << 16) | pData [27]);

      /* This data is only provided when extended JB statistics are supported */
      if (pDev->caps.bExtendedJBsupported)
      {
         pJbData->nRecBytesL     = ((pData [28] << 16) | pData [29]);
         /* Because the firmware does not provide the high DWORD part
            the driver has to handle the overflow. This is done by comparing the
            new and the old value. If the new value is smaller, there is
            an overflow and high is increased */
         if (pJbData->nRecBytesL < pCh->pCOD->nRecBytesL)
            pCh->pCOD->nRecBytesH++;
         pCh->pCOD->nRecBytesL = pJbData->nRecBytesL;
         /* apply the high DWORD part */
         pJbData->nRecBytesH = pCh->pCOD->nRecBytesH;
      }
   }
   if (pDev->caps.bExtendedJBsupported)
   {
      if (ret == IFX_SUCCESS)
      {
         pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
         pCmd [1] = ECMD_SIG_EVTSTAT;
         ret = CmdRead (pDev, pCmd, pData, 9);
      }
      if (ret == IFX_SUCCESS)
      {
         val = ((pData [10] << 16) | pData [11]);

         /* event packet byte counter overflow check */
         if (val < pCh->pCOD->nRecEvtBytesL)
            pCh->pCOD->nRecEvtBytesH++;
         /* cache L value for next overflow check */
         pCh->pCOD->nRecEvtBytesL = val;

         /* addition of packet and event packet byte counters */
         sum = pJbData->nRecBytesL + val;
         /* overflow check of addition,
            if we have an overflow in the addition, we have to
            increase high DWord reported to the application. */
         if ((sum < pJbData->nRecBytesL) || (sum < val))
            pJbData->nRecBytesH++;

         /* prepare final statistics for the application */
         pJbData->nRecBytesL = sum;
         pJbData->nRecBytesH += pCh->pCOD->nRecEvtBytesH;
      }
   }

   return ret;
}

/**
   resets jitter buffer statistics
\param pChannel          Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_COD_JB_Stat_Reset (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t    pCmd [2] = {0};

   /* reset JB Statistics by sending a Coder Channel JB Statistics write
      command of length 0 */
   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_COD_CHJBSTAT;

   return CmdWrite (pDev, pCmd, 0);
}


/**
   configure rt(c)p for a new connection
\param pChannel        Handle to TAPI_CONNECTION structure
\param pRtpConf        Handle to IFX_TAPI_PKT_RTP_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_COD_RTP_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   IFX_int32_t ret = IFX_SUCCESS;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint8_t ch = pCh->nChannel - 1;

   if (ch < pDev->caps.nCOD)
   {
      /* set ssrc, sequence nr and timestamp for coder channel */
      ret = VINETIC_COD_RTP_Cfg (pCh, pRtpConf);
   }

   if ((ch < pDev->caps.nSIG) && (ch < pDev->caps.nCOD))
   {
      /* sanity check for proper event payload types */
      if ((pCh->pCOD->fw_cod_ch.bit.en) && (pCh->nEvtPT != pRtpConf->nEventPT))
      {
         SET_ERROR (VIN_ERR_COD_RUNNING);
         return IFX_ERROR;
      }

      if (pRtpConf->nEventPT != 0)
      {
         /* set SSRC for the signaling channel and configure OOB signalling */
         ret = VINETIC_SIG_RTP_OOB_Cfg (pCh, pRtpConf);
      }

      /* error case */
      if (ret != IFX_SUCCESS)
      {
         TRACE (VINETIC,DBG_LEVEL_HIGH,
             ("\n\rDRV_ERROR: RTP Configuration failed\n\r"));
      }
      else
      {
         pCh->pCOD->nSeqNr = pRtpConf->nSeqNr;
         pCh->pCOD->nSsrc  = pRtpConf->nSsrc;
      }
   }
   return ret;
}


/**
   configure a new payload type
\param pChannel          Handle to TAPI_CHANNEL structure
\param pRtpPTConf        Handle to IFX_TAPI_PKT_RTP_PT_CFG_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_return_t IFX_TAPI_LL_COD_RTP_PayloadTable_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf)
{
   IFX_int32_t ret = IFX_SUCCESS;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL)
   /* In case of AAL we must not write the RTP PT table to avoid conflicts,
      as we currently don't have a return value for "not supported", we
      return IFX_SUCCESS because of the automatic RTP PT table configuration
      done by drv_tapi. Once we have a return value for not supported
      we could modify this code.
      An alternative would be to remove the function pointers for RTP
      from the drvCty in case we download an AAL firmware - but this would
      affect all devices at once. So this is no alternative. */
   if (pCh->pParent->nProtocolType != IFX_TAPI_PRT_TYPE_RTP)
      return IFX_SUCCESS;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL) */

   /* setting while room-noise detection is running has no effect - abort */
   if (pCh->pCOD->nOpMode == vinetic_cod_OPMODE_ROOMNOISE)
      ret = IFX_ERROR;

#ifdef VIN_2CPE
   /* Set payload types in downstream direction (TX) first -
      Note: overwrites upstream PT config (RX) */
   if (ret == IFX_SUCCESS)
      ret = Dsp_SetRTPPayloadType (pCh, IFX_FALSE,
                                   (IFX_uint8_t*)pRtpPTConf->nPTdown);
#endif /* VIN_2CPE*/

   /* Set payload types in upstream direction (RX) */
   if (ret == IFX_SUCCESS)
      ret = Dsp_SetRTPPayloadType (pCh, IFX_TRUE,
                                   (IFX_uint8_t*)pRtpPTConf->nPTup);

   if (ret != IFX_SUCCESS)
      TRACE (VINETIC,DBG_LEVEL_HIGH,
             ("\n\rDRV_ERROR: Coder Channel RTP configuration.\n\r"));

   return ret;
}


/** Start or stop generation of RTP event packets
   \param nEvent          Event code as defined in RFC2833
   \param nStart          Start (true) or stop (false)
   \param nDuration       Duration of event in units of 10 ms (0 = forever)
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured
\remarks
   Event duration below 50ms are automatically increased to 50ms.
\remarks
   Even though the FW message belongs to the SIG module it is sent from here
   because the RTP generation is originally a function of the COD module.
*/
IFX_return_t IFX_TAPI_LL_COD_RTP_EventGenerate (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_uint8_t nEvent,
                                                IFX_boolean_t bStart,
                                                IFX_uint8_t nDuration)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) (pCh->pParent);
   IFX_uint16_t pCmd [4] = {0};

   /* valid values for duration are 0x05 - 0xFF (50ms - 2550ms) */
   if ( (nDuration != 0) && (nDuration < 0x05) )
   {
      TRACE (VINETIC, DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: Duration for event generation "
           "increased to 50ms minimum\n\r"));
      nDuration = 0x05;
   }

   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_SIG_EV_GEN;
   pCmd [2] = (bStart ? 0x8000 : 0x0000) | (nDuration & 0x00FF);
   pCmd [3] = nEvent & 0x00FF;

   return CmdWrite (pDev, pCmd, 2);
}


/**
   gets the RTCP statistic information for the addressed channel
\param pChannel        Handle to TAPI_CHANNEL structure
\param pRTCP           Handle to IFX_TAPI_PKT_RTCP_STATISTICS_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   RTCP statistics for the specified channel are reset after the completion of
   the read request. A Coder Channel Statistics write command is issued on this
   purpose.
*/
IFX_return_t IFX_TAPI_LL_COD_RTCP_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t pCmd [2] = {0}, pData[16] = {0};
   IFX_int32_t ret = IFX_SUCCESS;

   /*EOP command */
   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   /* Read Coder Channel Statistics */
   pCmd [1] = ECMD_COD_CHRTCP;
   ret = CmdRead (pDev, pCmd, pData, 14);
   if (ret == IFX_SUCCESS)
   {
      /* assign the RTCP values to the corresponding members of the
         IFX_TAPI_PKT_RTCP_STATISTICS_t structure */
      pRTCP->rtp_ts        = (pData[2] << 16) | pData[3];
      pRTCP->psent         = (pData[4] << 16) | pData[5];
      pRTCP->osent         = (pData[6] << 16) | pData[7];
      pRTCP->rssrc         = (pData[8] << 16) | pData[9];
      pRTCP->fraction      = ((pData[10] & 0xFF00)>> 8);
      pRTCP->lost          = ((pData[10] & 0xFF) << 16) | pData[11];
      pRTCP->last_seq      = (pData[12] << 16) | pData [13];
      pRTCP->jitter        = (pData[14] << 16) | pData[15];
      /* return stored SSRC */
      pRTCP->ssrc          = pCh->pCOD->nSsrc;
      /* The calling control task will set the parameters lsr and dlsr */
   }
   else
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
          ("\n\rDRV_ERROR: can't read Coder Channel RTCP statistics.\n\r"));
   }

   return ret;
}

/**
   resets  RTCP statistics
\param pChannel          Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
*/
IFX_return_t IFX_TAPI_LL_COD_RTCP_Reset (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t    pCmd [2] = {0};

   /* reset RTCP Statistics by sending a Coder Channel
      Statistics (RTCP command) write command of length 0 */
   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_COD_CHRTCP;

   return CmdWrite (pDev, pCmd, 0);
}

#ifdef ASYNC_CMD
/**
   Prepare the RTCP statistic information for the addressed channel
   (in ISR context)
\param pChannel        Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   Together with TAPI_LL_Phone_RTCP_GatherStatistics, this function provides
   non-blocking and non-OS-dependent access to RTCP statistics.  The client
   calls TAPI_LL_Phone_RTCP_PrepareStatistics, performs other time-consuming
   tasks, and then calls TAPI_LL_Phone_RTCP_GatherStatistics to read the
   command mailbox. This function must execute in the context of a
   non-interruptible ISR, hence we are not concerned with protection from
   interrupts.
   Note that it is the responsibility of the client to gather statistics in the
   same order that they are requested (since there's a single command outbox
   per VINETIC device).
*/
IFX_return_t IFX_TAPI_LL_COD_RTCP_Prepare_Unprot (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_uint16_t          pCmd[2];
   IFX_int32_t           ret     = IFX_SUCCESS;

   /* EOP command with read bit set (the Short Command bit remains 0) */
   pCmd[0] = CMD1_EOP | CMD1_RD | (pCh->nChannel - 1);
   /* Read Coder Channel Statistics (the length field is 0, see VINETIC User's
      Manual) */
   pCmd[1] = ECMD_COD_CHRTCP | CMD_COD_RTCP_LEN;
   /* write read-request using ISR-specific write function */
   ret = CmdWriteIsr (pDev, pCmd, 0);
   if (ret != IFX_SUCCESS)
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH,
           ("\n\rDRV_ERROR: can't request Coder Channel RTCP statistics.\n\r"));
   }

   return ret;
}

/**
   Gathers previously requested RTCP statistic information for the addressed
   channel
\param pChannel        Handle to TAPI_CHANNEL structure
\param pRTCP           Handle to the returned data.
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   See description under TAPI_LL_Phone_RTCP_PrepareStatistics.
*/
IFX_return_t IFX_TAPI_LL_COD_RTCP_Prepared_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   VINETIC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_uint16_t    *pData;

   if (pCodCh.rtcp_update == IFX_TRUE)
   {
      pData = pCodCh.rtcp;
      /* assign the RTCP values to the corresponding members
         of the IFX_TAPI_PKT_RTCP_STATISTICS_t structure */
      pRTCP->rtp_ts        = (pData[0] << 16) | pData[1];
      pRTCP->psent         = (pData[2] << 16) | pData[3];
      pRTCP->osent         = (pData[4] << 16) | pData[5];
      pRTCP->ssrc          = (pData[6] << 16) | pData[7];
      pRTCP->fraction      = ((pData[8] & 0xFF00)>> 8);
      pRTCP->lost          = ((pData[8] & 0xFF) << 16) | pData[9];
      pRTCP->last_seq      = (pData[10] << 16) | pData [11];
      pRTCP->jitter        = (pData[12] << 16) | pData[13];
      pCodCh.rtcp_update = IFX_FALSE;

      return IFX_SUCCESS;
   }
   else
      return IFX_ERROR;
}

#endif /* ASYNC_CMD */

/**
   callback function used by the TAPI event dispatcher in case of a decoder change
   event to retrieve details of the new decoder (decoder type and packetisation time)
   The call of the function is important as reading of the decoder details acknowledges
   the decoder change interrupt in firmware, i.e. no further decoder change will be
   reported.
   \param pLLCh   pointer to the low level channel context
   \param pDec    pointer to a structure to retrieve the decoder details
   \return IFX_SUCCESS or IFX_ERROR
*/
static IFX_TAPI_LL_ERR_t IFX_TAPI_LL_COD_DEC_Chg_Detail_Req (IFX_TAPI_LL_CH_t *pLLCh,
                                                             IFX_TAPI_DEC_DETAILS_t *pDec)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLCh;
   IFX_uint16_t    pCmd[4] = {0};
   int err;

   pCmd [0] = CMD1_EOP | (pCh->nChannel - 1);
   pCmd [1] = ECMD_COD_CHDECSTAT;
   err = CmdRead (pCh->pParent, &pCmd[0], &pCmd[0], 1);

   pDec->dec_type = vinetic_cod_trans_cod_fw2tapi((IFX_int8_t)(pCmd[2] & COD_CH_ENC));
   pDec->dec_framelength = getCOD_TAPI_Frame_Length((pCmd[2] & 0xFF00) >> 8);

   return err;
}


#if (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38)
/**
   Configures the Datapump for Modulation
\param pChannel          Handle to TAPI_CHANNEL structure
\param pFaxMod           Handle to IFX_TAPI_T38_MOD_DATA_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This function has also to map the Fax_Downstream ptr to the write
 interface
   -  The configuration of the Modulator requires that the datapump is disable.
      So the followings are done here :
      1- Disable datapump if enable
      2- Set Modulator
      3- Map read interface to Fax_Downstream
      4- Activate datapump for Modulation and
         set other datapump parameters related to Modulation
   - This operation uses a data channel. Any reference to phone channel should
     be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_Mod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_T38_MOD_DATA_t const *pFaxMod)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t ret;
   /* Event handler if error. */
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_return_t ret_evt;

   if (pDev->pChannel[pCh->nChannel - 1].pCOD->fw_cod_ch.bit.en)
   {
      SET_ERROR (VIN_ERR_COD_RUNNING);
      return IFX_ERROR;
   }
   /* disable datapump */
   ret = Dsp_SetDatapump (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);

   /* set command to configure the modulator */
   if (ret == IFX_SUCCESS)
      ret = Dsp_SetDPMod (pCh, pFaxMod->nStandard, pFaxMod->nSigLen,
                          pFaxMod->nDbm, pFaxMod->nTEP, pFaxMod->nTraining);
   else
   {
      pCh->TapiFaxStatus.nStatus &= ~ (IFX_TAPI_FAX_T38_TX_ON |
                                            IFX_TAPI_FAX_T38_DP_ON);
      /* set datapump failed, no further action required */
      return ret;
   }
   if (ret == IFX_SUCCESS)
   {
      /* map write interface for fax support */
      pCh->if_write = Fax_DownStream;
      /* set fax state as running */
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_TX_ON;
      /* configure and start the datapump for modulation */
      ret = Dsp_SetDatapump (pCh, IFX_TRUE, IFX_FALSE, pFaxMod->nGainTx,
                             pFaxMod->nMobsm, pFaxMod->nMobrd, 0);
   }
   /* handle error case: Continue with voice */
   if (ret == IFX_ERROR)
   {
      ret = Dsp_SetDatapump (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
      pCh->if_write   = VoIP_DownStream;
      pCh->TapiFaxStatus.nStatus &= ~IFX_TAPI_FAX_T38_TX_ON;
      /* set error and issue tapi exception */
       /* Fill event structure. */
      /* FIXME, need protection. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_MODON;
      ret_evt = IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
      if(ret_evt == IFX_ERROR)
      {
         /* \todo if dispatcher error?? */
      }
   }
   else
   {
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_DP_ON;
   }

   return ret;
}

/**
   Configures the Datapump for Demodulation
\param pChannel          Handle to TAPI_CHANNEL structure
\param pFaxDemod         Handle to IFX_TAPI_T38_DEMOD_DATA_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This function has also to map the Fax_UpStream ptr to the read interface
      and reset the data fifo
   -  The configuration of the Demodulator requires that the datapump is
 disable.
      So the followings are done here :
         1- Disable datapump if enable
         2- Set Demodulator
         3- Map read interface to Fax_UpStream
         4- Activate datapump for Demodulation and
            set other datapump parameters related to Demodulation
   - This operation uses a data channel. Any reference to phone channel should
     be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_DeMod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t ret;
   /* Event handler if error. */
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_return_t ret_evt;

   if (pDev->pChannel[pCh->nChannel - 1].pCOD->fw_cod_ch.bit.en)
   {
      SET_ERROR (VIN_ERR_COD_RUNNING);
      return IFX_ERROR;
   }
   /* Disable Channel Fax datapump */
   ret = Dsp_SetDatapump (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);

   /* set command to configure the demodulator */
   if (ret == IFX_SUCCESS)
      ret = Dsp_SetDPDemod (pCh, pFaxDemod->nStandard1, pFaxDemod->nStandard2,
                            pFaxDemod->nEqualizer, pFaxDemod->nTraining);
   else
      /* set datapump failed, no further action required */
      return ret;

   if (ret == IFX_SUCCESS)
   {
      /* Clear fifo for this channel to record valid fax data */
      Vinetic_IrqLockDevice (pDev);

      fifoReset(TAPI_UpStreamFifo_Get(pCh->pTapiCh));

      /* map write interface for fax support */
      pCh->if_read = Fax_UpStream;
      /* set fax state as running */
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_TX_ON;
      Vinetic_IrqUnlockDevice (pDev);
      /* configure the datapump for demodulation */
      ret = Dsp_SetDatapump (pCh, IFX_TRUE, IFX_TRUE, pFaxDemod->nGainRx,
                             0, 0, pFaxDemod->nDmbsd);
   }
   else
   {
      pCh->TapiFaxStatus.nStatus &= ~ (IFX_TAPI_FAX_T38_TX_ON |
                                            IFX_TAPI_FAX_T38_DP_ON);
      /* SetDPDemod failed, no further action required */
      return ret;
   }
   /* configure the datapump for demodulation */
   if (ret == IFX_ERROR)
   {
      /* handle error case: continue with voice */
      ret = Dsp_SetDatapump (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
      Vinetic_IrqLockDevice (pDev);
      pCh->if_read   = VoIP_UpStream;
      /* from now on, fax is running */
      pCh->TapiFaxStatus.nStatus = 0;
      Vinetic_IrqUnlockDevice (pDev);
      /* set error and issue tapi exception */
      /* Fill event structure. */
      /* FIXME, need protection. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_DEMODON;
      ret_evt = IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
      if(ret_evt == IFX_ERROR)
      {
         /* \todo if dispatcher error?? */
      }
   }
   else
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_DP_ON;

   return ret;
}

/**
   disables the Fax datapump
\param pChannel          Handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This function has to remap the VoIp_UpStream and VoIP_Downstream to
      the driver read and write interfaces
   -  This operation uses a data channel. Any reference to phone channel should
      be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_Datapump_Disable (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VINETIC_CHANNEL  *pCh = (VINETIC_CHANNEL *)pLLChannel;
   IFX_int32_t ret = IFX_SUCCESS;
   /* Event handler if error. */
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_return_t ret_evt;

   /* Disable Channel Fax datapump */
   ret = Dsp_SetDatapump (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
   /* activate voice path and make driver ready for a next fax connection */
   if (ret == IFX_SUCCESS)
   {
      /* reset status /error flags */
      pCh->TapiFaxStatus.nStatus = 0;
      pCh->TapiFaxStatus.nError = 0;
      /* remap read/write interfaces */
      pCh->if_write = VoIP_DownStream;
      pCh->if_read  = VoIP_UpStream;
   }
   else
   {
      /* set error and issue tapi exception */
      /* Fill event structure. */
      /* FIXME, need protection. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_DPOFF;
      /* FIXME: */
      tapiEvent.data.value = 0;
      ret_evt = IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
      if(ret_evt == IFX_ERROR)
      {
         /* \todo if dispatcher error?? */
      }
      TRACE (VINETIC, DBG_LEVEL_HIGH,
             ("DRV_ERR: Disable Datapump" " failed\n\r"));
   }

   return ret;
}

/**
   query Fax Status
\param pChannel          Handle to TAPI_CHANNEL structure
\param pFaxStatus        Handle to IFX_TAPI_T38_STATUS_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This operation uses a data channel. Any reference to phone channel should
      be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_Status_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_T38_STATUS_t *pFaxStatus)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* read status from cache structure */
   *pFaxStatus = pCh->TapiFaxStatus;

   return IFX_SUCCESS;
}

/**
   set Fax Error Status
\param pChannel          Handle to TAPI_CHANNEL structure
\param pFaxStatus        Handle to IFX_TAPI_T38_STATUS_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This operation uses a data channel. Any reference to phone channel should
      be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_Error_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                      unsigned char error)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* read status from cache structure */
   pCh->TapiFaxStatus.nError = error;

   return IFX_SUCCESS;
}

/**
   query Fax Status
\param pChannel          Handle to TAPI_CHANNEL structure
\param pFaxStatus        Handle to IFX_TAPI_T38_STATUS_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   -  This operation uses a data channel. Any reference to phone channel should
      be done with an instance of a phone channel pointer
*/
IFX_return_t IFX_TAPI_LL_COD_T38_Status_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                      unsigned char status)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *)pLLChannel;

   /* read status from cache structure */
   pCh->TapiFaxStatus.nStatus = status;

   return IFX_SUCCESS;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38) */


#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE))
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
/**
   RTP settings for Coder channel
\param
   pCh         - handle to VMMC channel
   pRtpPTConf  - RTP configuration data passed from TAPI
*/
static IFX_int32_t VINETIC_COD_RTP_Cfg (VINETIC_CHANNEL *pCh,
                                        IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   IFX_int32_t  ret = IFX_SUCCESS;
   IFX_uint16_t cmd[5];
   IFX_uint16_t *pCmd = &cmd[0];
   IFX_uint16_t ch = pCh->nChannel-1;

   /* set SSRC and Sequence Number (RTP Upstream Configuration/PTs)
                                                   / if different */
   if ((pCh->pCOD->nSsrc  != pRtpConf->nSsrc) ||
       (pCh->pCOD->nSeqNr != pRtpConf->nSeqNr))
   {
      /* do rtp settings for coder channel */
      pCmd[0] = CMD1_EOP | ch;
      pCmd[1] = ECMD_COD_CHRTP_TX;
      if (pCmd[2] != HIGHWORD (pRtpConf->nSsrc) ||
          pCmd[3] != LOWWORD (pRtpConf->nSsrc) ||
          pCmd[4] != pRtpConf->nSeqNr)
      {
         /* write only if needed */
         pCmd[2] = HIGHWORD (pRtpConf->nSsrc);
         pCmd[3] = LOWWORD (pRtpConf->nSsrc);
         pCmd[4] = pRtpConf->nSeqNr;
         ret = CmdWrite (pCh->pParent, pCmd, 3);
      }
   }

   /* set time stamp (RTP Timestamp Configuration) / if different */
   if (ret == IFX_SUCCESS)
   {
      /* set the RTP time stamp */
      pCmd[0] = CMD1_EOP | ch;
      pCmd[1] = ECMD_COD_RTP;
      pCmd[2] = HIGHWORD (pRtpConf->nTimestamp);
      pCmd[3] = LOWWORD (pRtpConf->nTimestamp);

      ret = CmdWrite (pCh->pParent, pCmd, 2);
   }

   return ret;
}


/**
  get the coder channel status (coder enabled or not)
  \param   pCh   - pointer to the device structure
  \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
*/
IFX_boolean_t VINETIC_COD_ChStatusGet(VINETIC_CHANNEL *pCh)
{
   return pCh->pCOD->fw_cod_ch.bit.en;
}


/**
   base CODER Module configuration
\param
   pDev  - pointer to the device interface
\return
    IFX_SUCCESS if no error, otherwise IFX_ERROR
Remark:
   Use this function where needed to set the base configuration
   of the Coder Module. This function isn't an IOCTL function
   This function configures:
      - CODER Module
      - all CODER Channels
      - all CODER Jitter Buffers
      - Coder Channel decoder status and Coder Channel profiles in case of AAL
*/
IFX_int32_t VINETIC_COD_baseConf (VINETIC_CHANNEL *pCh)
{
   VINETIC_DEVICE *pDev   = pCh->pParent;
   IFX_uint16_t pCmd [14] = {0};
   IFX_int32_t  err       = IFX_SUCCESS, i;
   IFX_uint8_t  ch        = (pCh->nChannel - 1);

   /* Configure Coder Channels RTP or AAL *********************************** */
   if ((pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT) == ECMD_VERS_EDSP_PRT_AAL)
   {
      pCmd [0] = CMD1_EOP | ch;

      /* configure AAL profile */
      pCmd [1] = ECMD_COD_CHAAL;
      /* CID = 0x3E */
      pCmd [2] = 0x3E00;
      /* SQNR_INTV = 0x04 for 5.5 ms
         (G711, ALaw/ULaw - G726, 16Kbps/32 Kbps */
      pCmd [3] = COD_CHAAL_SQNR_INTV_5_5MS;
      /* Formula : LI = (bitrate[Kbps]*pte)/8 - 1 */

      /* LI = 0x2B, ENC = 0x03 : G711 ULaw, 64 Kbps */
      pCmd [4] = 0xAC03;
      /* LI = 0x0A, ENC = 0x04 : G726, 16 Kbps, 5.5 ms */
      pCmd [5] = 0x2804;
      /* LI = 0x0x15, ENC = 0x06: G726, 32 Kbps */
      pCmd [6] = 0x5406;
      for (i = 7; i <= 13; i++)
         pCmd [i] = 0;
      err = CmdWrite (pDev, pCmd, 12);
   }

   /* Configure Coder Channels ********************************************** */
   if (err == IFX_SUCCESS)
   {
      /* reset all data fields/bits in the fw message
         "Coder Channel Speech Compression" to zero.
         Explicit set to zero is not necessary */
      memset (&pCh->pCOD->fw_cod_ch.value[CMD_HEADER_CNT], 0, (CMD_COD_CH_LEN * 2));
      /* BFI = 1, DEC = 0 */
      pCh->pCOD->fw_cod_ch.value[3]      = 0xF700;
      pCh->pCOD->fw_cod_ch.bit.dec       = 1; /* decoder path active */
      pCh->pCOD->fw_cod_ch.bit.codnr     = ch;
      pCh->pCOD->fw_cod_ch.bit.gain1     = COD_GAIN_0DB;
      pCh->pCOD->fw_cod_ch.bit.gain2     = COD_GAIN_0DB;
      pCh->pCOD->fw_cod_ch.bit.ns        = 1;

      /* store the encoder value local till the coder-module gets enabled and
         is not muted */
      pCh->pCOD->enc_conf                = COD_CH_G711_ULAW_ENC;
      switch (pDev->nEdspVers[0] & ECMD_VERS_EDSP_PRT)
      {
         case ECMD_VERS_EDSP_PRT_AAL:
            pCh->pCOD->fw_cod_ch.bit.pte = COD_CH_PTE_5_5MS;
            break;
         default:
            pCh->pCOD->fw_cod_ch.bit.pte = COD_CH_PTE_10MS;
      }

      /* Set the inputs in the cached message and write it */
      err = VINETIC_COD_Set_Inputs(pCh);
   }

   /* Configure coder channel decoder status to issue interrupts on change of
      Decoder or Packet time */
   if (err == IFX_SUCCESS)
   {
      pCmd [0] = CMD1_EOP | ch;
      pCmd [1] = ECMD_COD_CHDECSTAT;
      /* PTE = 1 (to set with profile), DC = 1 */
      pCmd [2] = (COD_CHDECSTAT_PTC | COD_CHDECSTAT_DC);
      err = CmdWrite (pDev, pCmd, 1);
   }
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
   /* set up little endian mode in firmware */
   pCmd[0] = CMD1_EOP;
   pCmd[1] = ECMD_ENDIAN_CTL;
   pCmd[2] = ECMD_ENDIAN_LE;
   err = CmdWrite(pDev, pCmd, 1);
#endif /* (__BYTE_ORDER == __LITTLE_ENDIAN) */
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
   if (err != IFX_SUCCESS)
   {
      TRACE (VINETIC,DBG_LEVEL_HIGH, ("CODER configuration failed\n\r"));
   }
   return err;
}
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */
#endif /* (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M | VIN_FEAT_VIN_2CPE ) */


/**
   Initalize the coder module and the cached firmware messages and variables

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_COD_Init_Ch (VINETIC_CHANNEL *pCh)
{
   VINETIC_CODCH_t *pCOD = pCh->pCOD;
   IFX_uint8_t     ch    = pCh->nChannel - 1;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: VINETIC_COD_Init_Ch called\n\r"));

   VINETIC_CON_Init_CodCh (pCh);

   /* COD CH */
   memset (pCOD->fw_cod_ch.value, 0, sizeof(pCOD->fw_cod_ch));
   pCOD->fw_cod_ch.value[0] = CMD1_EOP | ch;
   pCOD->fw_cod_ch.value[1] = ECMD_COD_CH;

   /* at startup the coder is idle */
   pCOD->nOpMode = vinetic_cod_OPMODE_IDLE;
}


/**
   Set the signal inputs of the cached fw message for the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_return_t VINETIC_COD_Set_Inputs (VINETIC_CHANNEL *pCh)
{
   FWM_COD_CH        *p_fw_cod_ch;
   IFX_return_t      ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_cod_ch = &pCh->pCOD->fw_cod_ch;

   IFXOS_MutexLock (pCh->chAcc);
   p_fw_cod_ch->bit.i1 = VINETIC_CON_Get_COD_SignalInput (pCh, 0);
   p_fw_cod_ch->bit.i2 = VINETIC_CON_Get_COD_SignalInput (pCh, 1);
   p_fw_cod_ch->bit.i3 = VINETIC_CON_Get_COD_SignalInput (pCh, 2);
   p_fw_cod_ch->bit.i4 = VINETIC_CON_Get_COD_SignalInput (pCh, 3);
   p_fw_cod_ch->bit.i5 = VINETIC_CON_Get_COD_SignalInput (pCh, 4);

   ret = CmdWrite (pCh->pParent, p_fw_cod_ch->value, CMD_COD_CH_LEN);

   IFXOS_MutexUnlock (pCh->chAcc);

   return ret;
}


/**
  Allocate data structures of the COD module in the given channel

  \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
  \return  IFX_SUCCESS or IFX_ERROR in case the stucture could not be created
  \remarks The channel parameter is not checked because the calling
           function assures correct values.
*/
IFX_return_t VINETIC_COD_Allocate_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   VINETIC_COD_Free_Ch_Structures (pCh);

   pCh->pCOD = IFXOS_MALLOC(sizeof(VINETIC_CODCH_t));
   if (pCh->pCOD == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset(pCh->pCOD, 0, sizeof(VINETIC_CODCH_t));

   return IFX_SUCCESS;
}


/**
   Free data structures of the COD module in the given channel

   \param   VINETIC_CHANNEL*   Pointer to the VINETIC channel structure
   \return  none
*/
IFX_void_t VINETIC_COD_Free_Ch_Structures (VINETIC_CHANNEL *pCh)
{
   if (pCh->pCOD != IFX_NULL)
   {
      IFXOS_FREE(pCh->pCOD);
   }
}


/**
   Function called by the init_module of device, fills up COD module
   function pointers which are passed to HL TAPI during registration

   \param   pCOD   handle to the coder channel driver context structure
   \return  none
*/
void VINETIC_COD_Func_Register (IFX_TAPI_DRV_CTX_COD_t *pCOD)
{
#if (VIN_CFG_FEATURES & VIN_FEAT_VOICE)
   pCOD->ENC_Start               = IFX_TAPI_LL_COD_ENC_Start;
   pCOD->ENC_Stop                = IFX_TAPI_LL_COD_ENC_Stop;
   pCOD->ENC_Hold                = IFX_TAPI_LL_COD_ENC_Hold;
   pCOD->ENC_Cfg                 = IFX_TAPI_LL_COD_ENC_Cfg_Set;
   pCOD->DEC_Cfg                 = IFX_TAPI_LL_COD_DEC_Cfg_Set;
   pCOD->ENC_CoderType_Set       = IFX_TAPI_LL_COD_ENC_CoderType_Set;
   pCOD->ENC_FrameLength_Set     = IFX_TAPI_LL_COD_ENC_FrameLength_Set;
   pCOD->ENC_FrameLength_Get     = IFX_TAPI_LL_COD_ENC_FrameLength_Get;
   pCOD->ENC_RoomNoise           = IFX_TAPI_LL_COD_ENC_RoomNoise;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_VOICE) */

   pCOD->DEC_Start               = IFX_TAPI_LL_COD_DEC_Start;
   pCOD->DEC_Stop                = IFX_TAPI_LL_COD_DEC_Stop;

   pCOD->VAD_Cfg                 = IFX_TAPI_LL_COD_VAD_Cfg;

   pCOD->AGC_Cfg                 = IFX_TAPI_LL_COD_AGC_Cfg;
   pCOD->AGC_Enable              = IFX_TAPI_LL_COD_AGC_Enable;

   pCOD->JB_Cfg                  = IFX_TAPI_LL_COD_JB_Cfg;
   pCOD->JB_Stat_Reset           = IFX_TAPI_LL_COD_JB_Stat_Reset;
   pCOD->JB_Stat_Get             = IFX_TAPI_LL_COD_JB_Stat_Get;

#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL)
   pCOD->AAL_Cfg                 = IFX_TAPI_LL_COD_AAL_Cfg;
   pCOD->AAL_Profile_Cfg         = IFX_TAPI_LL_COD_AAL_Profile_Cfg;
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET_AAL) */

   pCOD->RTCP_Get                = IFX_TAPI_LL_COD_RTCP_Get;
   pCOD->RTCP_Reset              = IFX_TAPI_LL_COD_RTCP_Reset;
   pCOD->RTP_PayloadTable_Cfg    = IFX_TAPI_LL_COD_RTP_PayloadTable_Cfg;
   pCOD->RTP_Cfg                 = IFX_TAPI_LL_COD_RTP_Cfg;
   pCOD->RTP_EV_Generate         = IFX_TAPI_LL_COD_RTP_EventGenerate;

#ifdef ASYNC_CMD
   pCOD->RTCP_Prepared_Get       = IFX_TAPI_LL_COD_RTCP_Prepared_Get;
   pCOD->RTCP_Prepare_Unprot     = IFX_TAPI_LL_COD_RTCP_Prepare_Unprot;
#endif

#if (VIN_CFG_FEATURES & VIN_FEAT_FAX_T38)
   pCOD->T38_Mod_Enable          = IFX_TAPI_LL_COD_T38_Mod_Enable;
   pCOD->T38_DeMod_Enable        = IFX_TAPI_LL_COD_T38_DeMod_Enable;
   pCOD->T38_Datapump_Disable    = IFX_TAPI_LL_COD_T38_Datapump_Disable;
   pCOD->T38_Status_Get          = IFX_TAPI_LL_COD_T38_Status_Get;
   pCOD->T38_Status_Set          = IFX_TAPI_LL_COD_T38_Status_Set;
   pCOD->T38_Error_Set           = IFX_TAPI_LL_COD_T38_Error_Set;
#endif

   pCOD->DEC_Chg_Evt_Enable      = IFX_NULL;
   pCOD->DEC_Chg_Evt_Detail_Req  = IFX_TAPI_LL_COD_DEC_Chg_Detail_Req;
   pCOD->Volume_Set              = IFX_TAPI_LL_COD_Volume_Set;
   pCOD->DEC_HP_Set              = IFX_TAPI_LL_COD_DEC_HP_Set;
}
