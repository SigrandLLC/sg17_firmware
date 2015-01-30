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
   Module      : drv_vinetic_cid.c
   Description : This file contains the implementation of the functions
                 for CID operations.
******************************************************************************/

/* ============================= */
/* Check if feature is enabled   */
/* ============================= */
#ifdef HAVE_CONFIG_H
#include <drv_config.h>
#endif

#ifdef TAPI_CID

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vinetic_sig_priv.h"
#include "drv_vinetic_api.h"
#include "drv_vinetic_sig_cid.h"
#include "drv_vinetic_dspconf.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/* DTMF max digt/inter digit time */
#define MAX_DIGIT_TIME        127 /* ms */
#define MAX_INTERDIGIT_TIME   127 /* ms */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Extern function declaration   */
/* ============================= */
extern IFX_TAPI_CID_RX_DATA_t *TAPI_Phone_GetCidRxBuf (TAPI_CHANNEL *pChannel,
                                                       IFX_uint32_t nLen);

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_int32_t PrepareForCid      (VINETIC_CHANNEL *pCh);
static IFX_int32_t OnCidRequest       (VINETIC_CHANNEL *pCh);
static IFX_int32_t StopCid            (VINETIC_CHANNEL *pCh);
static IFX_int32_t SetupAfterCid      (VINETIC_CHANNEL *pCh);

static IFX_int32_t vinetic_sig_SetCidSender (VINETIC_CHANNEL *pCh,
                                         IFX_boolean_t bEn);
static IFX_int32_t vinetic_sig_SetCIDRec (VINETIC_CHANNEL const *pCh,
                                         IFX_TAPI_CID_HOOK_MODE_t  cidHookMode,
                                         IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                         IFX_boolean_t bEn);
static IFX_int32_t vinetic_sig_SetCIDCoeff (VINETIC_CHANNEL *pCh,
                                         IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                         IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                         IFX_uint8_t nSize);

/* ============================= */
/* Local variable definition     */
/* ============================= */
/* val,0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175,180,185,190,195,200,205,210,215,220,225,230,235,240,245,250,255,260,265,270,275,280,285,290,295,300,305,310,315,320,325,330,335,340,345,350,355,360,365,370,375,380,385,390,395,400,405,410,415,420,425,430,435,440,445,450,455,460,465,470,475,480,485,490,495,500,505,510,515,520,525,530,535,540,545,550,555,560,565,570,575,580,585,590,595,600,605,610,615,620,625,630,635,640,645,650,655,660,665,670,675,680,685,690,695,700,705,710,715,720,725,730,735,740,745,750,755,760,765,770,775,780,785,790,795,800,805,810,815,820,825,830,835,840,845,850,855,860,865,870,875,880,885,890,895,900,905,910,915,920,925,930,935,940,945,950,955,960 */
/* dB,0,-0,5,-1,-1,5,-2,-2,5,-3,-3,5,-4,-4,5,-5,-5,5,-6,-6,5,-7,-7,5,-8,-8,5,-9,-9,5,-10,-10,5,-11,-11,5,-12,-12,5,-13,-13,5,-14,-14,5,-15,-15,5,-16,-16,5,-17,-17,5,-18,-18,5,-19,-19,5,-20,-20,5,-21,-21,5,-22,-22,5,-23,-23,5,-24,-24,5,-25,-25,5,-26,-26,5,-27,-27,5,-28,-28,5,-29,-29,5,-30,-30,5,-31,-31,5,-32,-32,5,-33,-33,5,-34,-34,5,-35,-35,5,-36,-36,5,-37,-37,5,-38,-38,5,-39,-39,5,-40,-40,5,-41,-41,5,-42,-42,5,-43,-43,5,-44,-44,5,-45,-45,5,-46,-46,5,-47,-47,5,-48,-48,5,-49,-49,5,-50,-50,5,-51,-51,5,-52,-52,5,-53,-53,5,-54,-54,5,-55,-55,5,-56,-56,5,-57,-57,5,-58,-58,5,-59,-59,5,-60,-60,5,-61,-61,5,-62,-62,5,-63,-63,5,-64,-64,5,-65,-65,5,-66,-66,5,-67,-67,5,-68,-68,5,-69,-69,5,-70,-70,5,-71,-71,5,-72,-72,5,-73,-73,5,-74,-74,5,-75,-75,5,-76,-76,5,-77,-77,5,-78,-78,5,-79,-79,5,-80,-80,5,-81,-81,5,-82,-82,5,-83,-83,5,-84,-84,5,-85,-85,5,-86,-86,5,-87,-87,5,-88,-88,5,-89,-89,5,-90,-90,5,-91,-91,5,-92,-92,5,-93,-93,5,-94,-94,5,-95,-95,5,-96 */
/* rx Val, 32768,30935,29205,27571,26029,24573,23198,21900,20675,19519,18427,17396,16423,15504,14637,13818,13045,12315,11627,10976,10362,9783,9235,8719,8231,7771,7336,6925,6538,6172,5827,5501,5193,4903,4629,4370,4125,3894,3677,3471,3277,3093,2920,2757,2603,2457,2320,2190,2068,1952,1843,1740,1642,1550,1464,1382,1305,1232,1163,1098,1036,978,924,872,823,777,734,693,654,617,583,550,519,490,463,437,413,389,368,347,328,309,292,276,260,246,232,219,207,195,184,174,164,155,146,138,130,123,116,110,104,98,92,87,82,78,73,69,65,62,58,55,52,49,46,44,41,39,37,35,33,31,29,28,26,25,23,22,21,20,18,17,16,16,15,14,13,12,12,11,10,10,9,9,8,8,7,7,7,6,6,6,5,5,5,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 */
static const IFX_uint16_t cid_rx_levels[] = {0x8000, 0x78D6, 0x7214, 0x6BB2, 0x65AC, 0x5FFC, 0x5A9D, 0x558C, 0x50C3, 0x4C3E, 0x47FA, 0x43F4, 0x4026, 0x3C90, 0x392C, 0x35FA, 0x32F5, 0x301B, 0x2D6A, 0x2AE0, 0x287A, 0x2636, 0x2413, 0x220E, 0x2026, 0x1E5A, 0x1CA7, 0x1B0D, 0x198A, 0x181C, 0x16C3, 0x157D, 0x1449, 0x1326, 0x1214, 0x1111, 0x101D, 0x0F36, 0x0E5C, 0x0D8E, 0x0CCC, 0x0C15, 0x0B68, 0x0AC5, 0x0A2A, 0x0999, 0x090F, 0x088E, 0x0813, 0x079F, 0x0732, 0x06CB, 0x066A, 0x060E, 0x05B7, 0x0565, 0x0518, 0x04CF, 0x048A, 0x0449, 0x040C, 0x03D2, 0x039B, 0x0367, 0x0337, 0x0309, 0x02DD, 0x02B4, 0x028D, 0x0269, 0x0246, 0x0226, 0x0207, 0x01EA, 0x01CE, 0x01B4, 0x019C, 0x0185, 0x016F, 0x015B, 0x0147, 0x0135, 0x0124, 0x0113, 0x0104, 0x00F5, 0x00E7, 0x00DB, 0x00CE, 0x00C3, 0x00B8, 0x00AD, 0x00A4, 0x009B, 0x0092, 0x008A, 0x0082, 0x007B, 0x0074, 0x006D, 0x0067, 0x0061, 0x005C, 0x0057, 0x0052, 0x004D, 0x0049, 0x0045, 0x0041, 0x003D, 0x003A, 0x0037, 0x0033, 0x0031, 0x002E, 0x002B, 0x0029, 0x0026, 0x0024, 0x0022, 0x0020, 0x001E, 0x001D, 0x001B, 0x001A, 0x0018, 0x0017, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010, 0x000F, 0x000E, 0x000D, 0x000D, 0x000C, 0x000B, 0x000A, 0x000A, 0x0009, 0x0009, 0x0008, 0x0008, 0x0007, 0x0007, 0x0006, 0x0006, 0x0006, 0x0005, 0x0005, 0x0005, 0x0004, 0x0004, 0x0004, 0x0004, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
/*tx Val,48300,45598,43048,40640,38366,36220,34194,32281,30475,28771,27161,25642,24207,22853,21575,20368,19229,18153,17138,16179,15274,14419,13613,12851,12132,11454,10813,10208,9637,9098,8589,8109,7655,7227,6823,6441,6081,5741,5419,5116,4830,4560,4305,4064,3837,3622,3419,3228,3048,2877,2716,2564,2421,2285,2157,2037,1923,1815,1714,1618,1527,1442,1361,1285,1213,1145,1081,1021,964,910,859,811,766,723,682,644,608,574,542,512,483,456,430,406,384,362,342,323,305,288,272,256,242,229,216,204,192,182,171,162,153,144,136,129,121,115,108,102,96,91,86,81,77,72,68,64,61,57,54,51,48,46,43,41,38,36,34,32,30,29,27,26,24,23,22,20,19,18,17,16,15,14,14,13,12,11,11,10,10,9,9,8,8,7,7,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1*/
static const IFX_uint16_t cid_tx_levels[] = {0xBCAC, 0xB21E, 0xA827, 0x9EBF, 0x95DE, 0x8D7C, 0x8592, 0x7E19, 0x770B, 0x7062, 0x6A19, 0x6429, 0x5E8F, 0x5945, 0x5446, 0x4F90, 0x4B1C, 0x46E9, 0x42F1, 0x3F32, 0x3BA9, 0x3853, 0x352C, 0x3233, 0x2F64, 0x2CBD, 0x2A3D, 0x27E0, 0x25A5, 0x238A, 0x218D, 0x1FAC, 0x1DE7, 0x1C3A, 0x1AA6, 0x1928, 0x17C0, 0x166C, 0x152B, 0x13FC, 0x12DE, 0x11CF, 0x10D0, 0x0FDF, 0x0EFC, 0x0E26, 0x0D5B, 0x0C9C, 0x0BE7, 0x0B3D, 0x0A9C, 0x0A04, 0x0974, 0x08ED, 0x086D, 0x07F4, 0x0782, 0x0717, 0x06B1, 0x0651, 0x05F7, 0x05A1, 0x0551, 0x0505, 0x04BD, 0x0479, 0x0439, 0x03FC, 0x03C3, 0x038D, 0x035A, 0x032A, 0x02FD, 0x02D2, 0x02AA, 0x0284, 0x0260, 0x023E, 0x021D, 0x01FF, 0x01E3, 0x01C7, 0x01AE, 0x0196, 0x017F, 0x016A, 0x0155, 0x0142, 0x0130, 0x011F, 0x010F, 0x0100, 0x00F2, 0x00E4, 0x00D7, 0x00CB, 0x00C0, 0x00B5, 0x00AB, 0x00A1, 0x0098, 0x0090, 0x0088, 0x0080, 0x0079, 0x0072, 0x006C, 0x0066, 0x0060, 0x005A, 0x0055, 0x0051, 0x004C, 0x0048, 0x0044, 0x0040, 0x003C, 0x0039, 0x0036, 0x0033, 0x0030, 0x002D, 0x002B, 0x0028, 0x0026, 0x0024, 0x0022, 0x0020, 0x001E, 0x001C, 0x001B, 0x0019, 0x0018, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010, 0x000F, 0x000E, 0x000D, 0x000C, 0x000C, 0x000B, 0x000A, 0x000A, 0x0009, 0x0009, 0x0008, 0x0008, 0x0007, 0x0007, 0x0006, 0x0006, 0x0006, 0x0005, 0x0005, 0x0005, 0x0004, 0x0004, 0x0004, 0x0004, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Disables or Enables CID Sender according to bEn

   \param   pCh  - pointer to VINETIC channel structure
   \param   bEn  - IFX_TRUE : enable / IFX_FALSE : disable

   \return
      IFX_SUCCESS or IFX_ERROR

*/
IFX_int32_t vinetic_sig_SetCidSender (VINETIC_CHANNEL *pCh, IFX_boolean_t bEn)
{
   IFX_int32_t    ret   = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t    ch    = (pCh->nChannel - 1);

   pDev->pChannel[ch].pSIG->cid_sender[0] &= ~CMD1_RD;
   if ((bEn == IFX_TRUE) && !(pDev->pChannel[ch].pSIG->cid_sender[2] & SIG_CID_EN))
   {
      pDev->pChannel[ch].pSIG->cid_sender[2] |= SIG_CID_EN;
      ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->cid_sender, 1);
   }
   else if ((bEn == IFX_FALSE) && (pDev->pChannel[ch].pSIG->cid_sender[2] & SIG_CID_EN))
   {
      pDev->pChannel[ch].pSIG->cid_sender[2] &= ~SIG_CID_EN;
      ret = CmdWrite (pDev, pDev->pChannel[ch].pSIG->cid_sender, 1);
   }

   return ret;
}

/**
  Disables or Enables Cid Receiver according to bEn.

\param
   pCh  - pointer to VINETIC channel structure
\param
   bEn  - IFX_TRUE : enable / IFX_FALSE : disable cid receiver

\return
   IFX_SUCCESS or IFX_ERROR
\remark
   The signalling channel must be enabled before this command is issued.
*******************************************************************************/
IFX_int32_t vinetic_sig_SetCIDRec (VINETIC_CHANNEL const *pCh,
                                   IFX_TAPI_CID_HOOK_MODE_t  cidHookMode,
                                   IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                   IFX_boolean_t bEn)
{
   IFX_int32_t     err      = IFX_SUCCESS,
                   ch = pCh->nChannel - 1;
   IFX_uint16_t    pCmd [5] = {0};
   VINETIC_DEVICE *pDev     = pCh->pParent;

   if (pDev->nChipMajorRev == VINETIC_V1x && bEn == IFX_TRUE)
   {
      err = VINETIC_SIG_SetDtmfRec (pCh, IFX_FALSE, VIN_SIG_TX);
   }
   /* EOP Cmd */
   pCmd [0] = (CMD1_EOP | ch);
   /* CID RX */
   if (err == IFX_SUCCESS)
   {
      if (bEn == IFX_TRUE)
      {
         pCmd[1] = (CMD2_MOD_RES | 0x0700);
         /* coefficients for CID rx */
         pCmd[2] = cid_rx_levels[ (-1)*pFskConf->levelRX / 5 ];
         /* set Seizure */
         pCmd[3] = pFskConf->seizureRX;
         /* set Mark according to CID type */
         switch (cidHookMode)
         {
         case  IFX_TAPI_CID_HM_OFFHOOK:
            pCmd[4] = pFskConf->markRXOffhook;
            break;
         default:
            /* fallthrough to onhook case */
         case IFX_TAPI_CID_HM_ONHOOK:
            pCmd[4] = pFskConf->markRXOnhook;
            break;
         }
         err = CmdWrite (pDev, pCmd, 3);
         if (err == IFX_SUCCESS)
         {
            pCmd [1] = ECMD_CIDRX;
            pCmd [2] = SIG_CIDRX_EN | ch;
            err = CmdWrite (pDev, pCmd, 1);
         }
      }
      else
      {
         pCmd [1] = ECMD_CIDRX;
         pCmd [2] = ch;
         err = CmdWrite (pDev, pCmd, 1);
      }
   }
   if (err == IFX_SUCCESS &&
       pDev->nChipMajorRev == VINETIC_V1x && bEn == IFX_FALSE)
   {
      /* for the 4VIP V1.4 the DTMF must be reconfigured cause of the overlay */
      pCmd[1] = ECMD_DTMF_REC_COEFF;
      /* Level= -56dB, Twist= 9.1dB */
      pCmd[2] = 0xC81C;
      pCmd[3] = 0x0060;
      err = CmdWrite (pDev, pCmd, 2);
      if (err == IFX_SUCCESS)
         err = VINETIC_SIG_SetDtmfRec ((VINETIC_CHANNEL const *) pCh,
                                IFX_TRUE, VIN_SIG_TX);
   }

   return err;
}

/**
   Set the CID sender coefficients
\param
   pCh         - pointer to VINETIC channel structure
\param
   cidHookMode - cid hook mode as specified in IFX_TAPI_CID_HOOK_MODE_t
\param
   pFskConf    - handle to IFX_TAPI_CID_FSK_CFG_t structure
\param
   nSize       - size of CID data to send, used to set the BRS level.
\return
   IFX_SUCCESS or Error Code
\remark
   - The CID sender must be disabled before programming the coefficients, what
      is done in this function
   - According to CID type (onhook=Type1, Offhook=Type2), different
      Mark/Seizure configuration apply according to specifications:
      GR-30-CORE : Type 1 (Onhook)  => Seizure = 300 bits, Mark = 180 bits
                   Type 2 (Offhook) => Seizure = 0 bits, Mark = 80 bits
      ETSI       : Type 1 (Onhook)  => Seizure = 300 bits, Mark = 180+/-25 bits
                   Type 2 (Offhook) => Seizure = 0 bits, Mark =  80+/-25bits
      SIN227     : Type 1 (Onhook)  => Seizure = 96 to 315 bits, Mark >= 55 bits
                   Type 2 (Offhook) => Seizure = 0 bits, Mark >= 55 bits

      To meet all these specifications, following is set as default:
         Type 1 (Onhook)  => Seizure = 300 bits, Mark = 180 bits
         Type 2 (Offhook) => Seizure =   0 bits, Mark = 80 bits
   - Here are some dB levels for reference :
      (formula: level=32768*10^(dBval / 20)
      -5dB = 0x47FA, -10 dB = 0x287A, - 15 dB = 0x16C3, -20dB = 0xCCC ,
      -25 dB = 0x732, -30 dB = 0x40C , -35 dB = 0x246, -40 dB = 0x147
*/
IFX_int32_t vinetic_sig_SetCIDCoeff (VINETIC_CHANNEL          *pCh,
                                     IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                     IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                     IFX_uint8_t nSize)
{
   IFX_int32_t    err = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint16_t   ch = (pCh->nChannel - 1);
   IFX_uint16_t   pCidCoefs [6] = {0};

   /* not allowed while cid sender is active */
   err = vinetic_sig_SetCidSender (pCh, IFX_FALSE);
   if (err == IFX_SUCCESS)
   {
      /* set the new coefficients */
      pCidCoefs[0] = (CMD1_EOP | ch);
      pCidCoefs[1] = ECMD_CID_COEF;
      /* calculate level value from pFskConf->levelTX */
      pCidCoefs[2] = cid_tx_levels[ (-1)*pFskConf->levelTX / 5 ];
      /* set seizure and mark according to CID type */
      switch (cidHookMode)
      {
      /* offhook CID, called CID type 2, NTT inclusive */
      case  IFX_TAPI_CID_HM_OFFHOOK:
         /* set Seizure - off hook CID has always 0 seizure bits */
         pCidCoefs[3] = 0;
         /* set Mark */
         pCidCoefs[4] = pFskConf->markTXOffhook;
         break;
      /* onhook CID, called CID type 1, NTT inclusive */
      case IFX_TAPI_CID_HM_ONHOOK:
         /* set Seizure */
         pCidCoefs[3] = pFskConf->seizureTX;
         /* set Mark */
         pCidCoefs[4] = pFskConf->markTXOnhook;
         break;
      }
      /* BRS <= MAX_CID_LOOP_DATA*/
      if (nSize >= MAX_CID_LOOP_DATA)
         pCidCoefs [5] = MAX_CID_LOOP_DATA;
      else
         pCidCoefs [5] = nSize;

      err = CmdWrite (pDev, pCidCoefs, 4);
   }
   if (err != IFX_SUCCESS)
      TRACE (VINETIC,DBG_LEVEL_HIGH, ("CID Coefficients setting failed\n\r"));

   return err;
}

/******************************************************************************
Description:
   setup to be done before a cid transmission.
Arguments:
   pCh      - channel pointer
Return:
   IFX_SUCCESS or IFX_ERROR
Remark:
   It is assumed that the CID coefficients (mark, seizure, brs, level) were set
   already accordingly by the caller of this function.
******************************************************************************/
static IFX_int32_t PrepareForCid (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t     err  = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t     ch   = (pCh->nChannel - 1);

   /* set the state to prepare to protect channel from a new cid sending while
      an ongoing cid sending is on. */
   CIDTX_SET_STATE (CID_PREPARE);
   /* store data into channel structure */
   pCh->cidSend.nPos     = 0;
   pCh->cidSend.pCmd [0] = (CMD1_EOP | (pCh->nChannel - 1));
   pCh->cidSend.pCmd [1] = ECMD_CID_DATA;
   pCh->cidSend.pCmd [2] = 0;
   /* Setup CID interrupt masks.
      Note : allow rising edge interrupts for CIS_BUF, CIS_REQ.
             allow falling edge interrupts for CIS_ACT */
   err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_CIS, IFX_FALSE,
                                       VIN_CIS_BUF_REQ_MASK, VIN_CIS_ACT_MASK);
   if (err == IFX_SUCCESS)
   {
      IFXOS_MutexLock   (pDev->memberAcc);
      err = Dsp_SigActStatus (pDev, ch, IFX_TRUE, CmdWrite);
      IFXOS_MutexUnlock   (pDev->memberAcc);
   }

   /* Set adders and enable CID Sender */
   if (err == IFX_SUCCESS)
   {
       CIDTX_SET_STATE (CID_TRANSMIT);
      /* Add signal only to Adder 2 and mute voice there (A1 = 00, A2 = 10) */
      pDev->pChannel[ch].pSIG->cid_sender[CMD_HEADER_CNT] &=
         ~(SIG_CID_A1 | SIG_CID_A2);
      pDev->pChannel[ch].pSIG->cid_sender[CMD_HEADER_CNT] |=
         SIG_CID_A2_VOICE;
      err = vinetic_sig_SetCidSender(pCh, IFX_TRUE);
   }

   if (err == IFX_ERROR)
   {
      /* allow CID setup on the next run */
      CIDTX_SET_STATE (CID_SETUP);
      pCh->cidSend.nCidCnt = 0;
      /* mask CID interrupts */
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_CIS, IFX_FALSE, 0, 0);
   }

   return err;
}

/******************************************************************************
Description:
   Does the needed settings at end of CID due to an error, an offhook event
   or the end of transmission
Arguments:
   pCh      - channel pointer
Return:
   IFX_SUCCESS or IFX_ERROR
Remarks :
   This function can be called from Interrupt routine and therefore should not
   be blocking.
   This function does not deactivate the global signaling channel, because
   otherwise the switching function Dsp_SigActStatus must be protected
   against interrupts. Switching of signaling channel is not a must, but
   only a recommendation. In a typical scenario more sig resources would be
   used anyway, that the switching is not mandatory.
******************************************************************************/
static IFX_int32_t StopCid (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t     err  = IFX_SUCCESS;
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t     ch   = (pCh->nChannel - 1);

   if (pCh->cidSend.nState == CID_OFFHOOK)
   {
      /* Disable Cid Sender now */
      pDev->pChannel[ch].pSIG->cid_sender[0] &= ~CMD1_RD;
      pDev->pChannel[ch].pSIG->cid_sender[2] &= ~SIG_CID_EN;
      pDev->pChannel[ch].pSIG->cid_sender[2] &= ~SIG_CID_AD;
      err = CmdWriteIsr (pDev, pDev->pChannel[ch].pSIG->cid_sender, 1);
#ifndef VIN_2CPE
#ifdef EVALUATION
   if (err == IFX_SUCCESS)
   {
      if ((((VINCH_EVAL *)pCh->pEval)->cidEval.bEvalCid == IFX_TRUE) &&
          (((VINCH_EVAL *)pCh->pEval)->cidEval.nCidOp != CIDEVAL_STOP))
      {
         ((VINCH_EVAL *)pCh->pEval)->cidEval.nCidOp = CIDEVAL_STOP;
         Eval_CidCallback ((IFX_uint32_t)pCh);
         err = ((pDev->err == VIN_ERR_CID_TRANSMIT) ? IFX_ERROR : IFX_SUCCESS);
      }
   }
#endif /* EVALUATION */
#endif /* VIN_2CPE */
   }
   /* Reset CID interrupt masks.
      Note : Mask rising edge interrupts for CIS_BUF, CIS_REQ, CIS_ACT.
             Mask falling edge interrupts for CIS_BUF, CIS_REQ, CIS_ACT. */
   if (err == IFX_SUCCESS)
   {
      err = VINETIC_Host_Set_EdspIntMask (pCh, VIN_EDSP_CIS, IFX_TRUE, 0, 0);
   }
   /* reset CID state */
   if (err == IFX_SUCCESS)
   {
      CIDTX_SET_STATE(CID_SETUP);
      pCh->cidSend.nCidCnt = 0;
   }

   return err;
}

/******************************************************************************
Description:
   setup to be done after succesfull cid transmission
Arguments:
   pCh      - channel pointer
Return:
   IFX_SUCCESS or IFX_ERROR
Remarks :
   This function can be called from Interrupt routine and therefore should not
   be blocking.
******************************************************************************/
static IFX_int32_t SetupAfterCid (VINETIC_CHANNEL *pCh)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   IFX_uint8_t    ch    = (pCh->nChannel - 1);
   IFX_int32_t    err   = IFX_SUCCESS;

   /* safety check. Function sould be called only at
      end of successfull transmission */
   if (pCh->cidSend.nState != CID_TRANSMIT)
      return IFX_SUCCESS;
   /* disable the CID sender : EN = 0 */
   pDev->pChannel[ch].pSIG->cid_sender[0] &= ~CMD1_RD;
   pDev->pChannel[ch].pSIG->cid_sender[2] &= ~SIG_CID_EN;
   /* to succesfully end the transmission, set autodeactivation : AD = 1*/
   pDev->pChannel[ch].pSIG->cid_sender[2] |= SIG_CID_AD;
   err = CmdWriteIsr (pDev, pDev->pChannel[ch].pSIG->cid_sender, 1);
#ifndef VIN_2CPE
#ifdef EVALUATION
   if (err == IFX_SUCCESS)
   {
      if ((((VINCH_EVAL *)pCh->pEval)->cidEval.bEvalCid == IFX_TRUE)  &&
          (((VINCH_EVAL *)pCh->pEval)->cidEval.nCidOp == CIDEVAL_PAUSE2))
      {
         Eval_CidCallback ((IFX_uint32_t)pCh);
         err = ((pDev->err == VIN_ERR_CID_TRANSMIT) ? IFX_ERROR : IFX_SUCCESS);
      }
   }
#endif /* EVALUATION */
#endif /* VIN_2CPE */

   return err;
}

/******************************************************************************
Description:
   operations to be done on CID request (ISRE1 [CIS_REQ] = 1)
Arguments:
   pCh      - channel pointer
Return:
   IFX_SUCCESS or IFX_ERROR
Remark :
   This function is called from Interrupt routine and therefore should not
   be blocking.
******************************************************************************/
static IFX_int32_t OnCidRequest (VINETIC_CHANNEL *pCh)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   VINETIC_CID    *pChCid  = &pCh->cidSend;
   IFX_uint8_t    i, len;
   IFX_int32_t    err;

   if (pChCid->nCidCnt - pChCid->nPos > MAX_CID_LOOP_DATA)
      len = MAX_CID_LOOP_DATA;
   /* last Cid data */
   else
   {
      len = (pChCid->nCidCnt - pChCid->nPos);
      pChCid->pCmd [2] = pCh->cidSend.nOdd;
   }
   if (len == 0)
   {
      /* no more data to send so ignore this interrupt */
      return IFX_SUCCESS;
   }
   /* code length, counting Odd word */
   pChCid->pCmd [1] = (pChCid->pCmd [1] & ~CMD2_LEN) | (len + 1);
   /* fill command buffer with new cid data */
   for (i = 0; i < len; i++)
      pChCid->pCmd [3 + i] = pChCid->pCid [pChCid->nPos + i];
   /* send cid */
   err = CmdWriteIsr (pDev, pChCid->pCmd, len + 1);
   if (err == IFX_SUCCESS)
   {
      /* advance the pos variable */
      pChCid->nPos += len;
      /* what to do at end of CID. */
      if (pChCid->nPos >= pChCid->nCidCnt)
      {
         err = SetupAfterCid (pCh);
      }
   }

   return err;
}

/* ============================= */
/* Global function definition    */
/* ============================= */

/*****************************************************************************/
/**
   CID FSK State Machine
\param
   pCh          - channel pointer
\return
   IFX_SUCCESS/IFX_ERROR
\remarks
   This function manages all CID states :
   - CID_SETUP        : settings to do before sending CID data
   - CID_TRANSMIT     : sending of CID data
   - CID_OFFHOOK      : stop Cid sender on Offhook
   - CID_TRANSMIT_ERR : settings after CID data send error (Buffer underrun i.e)

   This function can be called from Interrupt routine and therefore should not
   be blocking.
   CID baudrate = 150 Bytes/sec without Pause.
   1 CID Word = ~ 20 ms / Pause before request : 300 ms
*/
/*****************************************************************************/
IFX_int32_t VINETIC_CidFskMachine (VINETIC_CHANNEL *pCh)
{
   IFX_int32_t    err   = IFX_SUCCESS;

   switch (pCh->cidSend.nState)
   {
      case CID_SETUP:
         /* prepare for cid only when new cid data */
         if (pCh->cidSend.nCidCnt)
            err = PrepareForCid (pCh);
         break;
      case CID_TRANSMIT:
         err = OnCidRequest (pCh);
         break;
      case CID_OFFHOOK:
      case CID_TRANSMIT_END:
      case CID_TRANSMIT_ERR:
         err = StopCid (pCh);
         break;
      default:
         TRACE (VINETIC, DBG_LEVEL_HIGH,
                ("ERR: Unhandled Cid machine state\r\n"));
         err = IFX_ERROR;
         break;
   }
   if (err == IFX_ERROR)
   {
      SET_ERROR (VIN_ERR_CID_TRANSMIT);
      /* allow CID setup on the next run */
      CIDTX_SET_STATE(CID_SETUP);
      pCh->cidSend.nCidCnt = 0;
      /* don't allow forthcoming ringing */
      TAPI_Cid_Abort (pCh->pTapiCh);
      /** \todo send cid transmission runtime error here */
   }

   return err;
}

/******************************************************************************
Description:
   DTMF CID state machine
Arguments:
   pCh      - channel pointer
Return:
   IFX_SUCCESS or IFX_ERROR
Remark :
   This function translates the DTMF to CID states. This function is called
   from DTMF module in ISR context.
   In case the dtmf transmission is finished or aborted, the cid flag must be
   reset for an eventual subsequent sending of CID FSK on this channel.
******************************************************************************/
IFX_void_t VINETIC_CidDtmfMachine (VINETIC_CHANNEL *pCh)
{
   IFX_TAPI_EVENT_t tapiEvent;
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) pCh->pTapiCh;
   IFX_return_t ret = IFX_SUCCESS;

   switch (pCh->dtmfSend.state)
   {
      case DTMF_READY:
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_CID_TX_INFO_END;
         ret = IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
         break;
      case DTMF_START:
      case DTMF_TRANSMIT:
         break;
      case DTMF_ABORT:
         pCh->dtmfSend.state = DTMF_READY;
         /* don't allow forthcoming ringing */
         TAPI_Cid_Abort (pCh->pTapiCh);
         /** \todo send cid transmission runtime error here */
         break;
      default:
         break;
   }
}

/**
   reset FW message cache for CID sender
   temporary

*/
IFX_return_t VINETIC_SIG_CID_Sender_MSG_Reset (VINETIC_CHANNEL *pCh)
{
   VINETIC_DEVICE *pDev = pCh->pParent;
   int ch = pCh->nChannel -1;
   pDev->pChannel[ch].pSIG->cid_sender[2] = 0;
   return IFX_SUCCESS;
}

/**
   Start the CID receiver
\param pChannel    Handle to TAPI_CHANNEL structure
\param pCidFskCid      Pointer to Cid Fsk Configuration structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   It is checked if the needed resource is available. Otherwise it returns an
   error.
*/
IFX_return_t IFX_TAPI_LL_SIG_CID_RX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                           IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg)
{
   IFX_int32_t ret = IFX_ERROR;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;

   /* CID receiver can only be started when the signalling channel is active */
   if (pCh->pSIG->fw_sig_ch.bit.en == IFX_TRUE)
   {
      ret = vinetic_sig_SetCIDRec (pCh, cidHookMode, pCidFskCfg, IFX_TRUE);
   }

   return ret;
}

/**
   Stop CID receiver
\param pChannel        Handle to TAPI_CHANNEL structure
\param pCidFskCid      Pointer to Cid Fsk Configuration structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   It is checked if the resource is activated. Otherwise it returns an
   error.
*/
IFX_return_t IFX_TAPI_LL_SIG_CID_RX_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_CID_FSK_CFG_t *pCidFskCfg)
{
   IFX_int32_t ret = IFX_ERROR;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;

   /* CID receiver can only be stopped when the signalling channel is active */
   if (pCh->pSIG->fw_sig_ch.bit.en == IFX_TRUE)
   {
      /* hookmode and pCidFskCfg are ignored but must be given */
      ret = vinetic_sig_SetCIDRec (pCh, IFX_TAPI_CID_HM_ONHOOK,
                                   pCidFskCfg, IFX_FALSE);
   }

   return ret;
}

/**
   Send CID data
\param pChannel        Handle to TAPI_CHANNEL structure
\param pCid            contains CID data
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   This function is non blocking. It handles all necessary steps to transmit a
   CID. It returns an error if a CID transmission is already running.
*/
IFX_int32_t IFX_TAPI_LL_SIG_CID_TX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                          TAPI_CID_DATA_t const *pCid,
                                          TAPI_CID_CONF_t *pCidConf,
                                          IFX_TAPI_CID_DTMF_CFG_t *pDtmfConf,
                                          IFX_TAPI_CID_FSK_CFG_t *pCidFskConf)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   IFX_int32_t      ret = IFX_SUCCESS;

   switch (pCidConf->nStandard)
   {
   case IFX_TAPI_CID_STD_TELCORDIA:
   case IFX_TAPI_CID_STD_ETSI_FSK:
   case IFX_TAPI_CID_STD_SIN:
   case IFX_TAPI_CID_STD_NTT:
      /* check whether UTG2 is active,
      because the status bits for CIS and UTG2 are overlaid
      => CIS cannot be used while UTG2 is active! */
      if (pCh->pSIG->fw_sig_utg[1].bit.en == 1)
      {
         TRACE (VINETIC, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Cid Sender "
                 "cannot be used while UTG2 is active! \n\r"));
         return (IFX_ERROR);
      }
      if (pCh->cidSend.nState == CID_SETUP)
      {
         /* prevent data buffer overrun */
         if ( pCid->nCidParamLen > 258 )
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: "
                   "Too much data for CID sender buffer - aborting\n\r"));
            return IFX_ERROR;
         }
         /* odd num (lowest bit it set) */
         pCh->cidSend.nOdd     = (pCid->nCidParamLen & 1);
         /* get length in words (round up) */
         pCh->cidSend.nCidCnt  = (pCid->nCidParamLen>>1) + pCh->cidSend.nOdd;
         cpb2w(pCh->cidSend.pCid, (IFX_uint8_t *)pCid->cidParam,
               ((IFX_uint32_t)pCh->cidSend.nCidCnt << 1));
         ret = vinetic_sig_SetCIDCoeff(pCh, pCid->txHookMode,
                               pCidFskConf,
                               pCh->cidSend.nCidCnt);
         /* now start non blocking low level machine */
         if (ret == IFX_SUCCESS)
         {
            /* Bellcore specification shall be used */
            if (pCidConf->nStandard == IFX_TAPI_CID_STD_TELCORDIA)
            {
               pCh->pSIG->cid_sender[CMD_HEADER_CNT] &= ~SIG_CID_V23;
            }
            /* ITU-T V.23 specification shall be used */
            else
            {
               pCh->pSIG->cid_sender[CMD_HEADER_CNT] |= SIG_CID_V23;
            }
            pCh->cidSend.nCidType = pCid->txHookMode;
            ret = VINETIC_CidFskMachine (pCh);
         }
      }
      else
      {
         ret = IFX_ERROR;
         SET_ERROR (VIN_ERR_CID_RUNNING);
      }
      break;
   case IFX_TAPI_CID_STD_ETSI_DTMF:
      if ((pCh->dtmfSend.state == DTMF_READY) ||
          (pCh->dtmfSend.state == DTMF_ABORT)    )
      {
         IFX_uint16_t  i;

         /* check digit and interdigit times */
         if ((pDtmfConf->digitTime > MAX_DIGIT_TIME) ||
             (pDtmfConf->interDigitTime > MAX_INTERDIGIT_TIME))
         {
            TRACE (VINETIC, DBG_LEVEL_HIGH, ("\n\rDRV_ERROR: Digit time or "
                 "inter digit time exceeds limit of 127 ms\n\r"));
            return IFX_ERROR;
         }
         /* Write DTMF/AT generator coefficients */
         ret = VINETIC_SIG_SetDtmfCoeff (pCh, (pDtmfConf->digitTime * 2),
                                         (pDtmfConf->interDigitTime * 2));
         if (ret == IFX_SUCCESS)
         {
            /* not using cpb2w, byteorder is corrected in VINETIC_CidDtmfMachine */
            memcpy (pCh->cidSend.pCid, pCid->cidParam, pCid->nCidParamLen);
            /* transcode Characters A-D, # and * to FW specific setting */
            /* errors may occur if input string contains invalid characters */
            for (i=0; ret == IFX_SUCCESS && i<pCid->nCidParamLen; i++)
               ret = VINETIC_SIG_DTMF_encode_ascii2fw(
                        *(((IFX_char_t *)pCh->cidSend.pCid) + i),
                        ((IFX_uint8_t *)pCh->cidSend.pCid) + i);
            /* finally start the DTMF generator */
            if (ret == IFX_SUCCESS)
               ret = VINETIC_SIG_DtmfStart (pCh, pCh->cidSend.pCid,
                                            pCid->nCidParamLen, 1,
                                            VINETIC_CidDtmfMachine, IFX_TRUE);
         }
      }
      else
      {
         ret = IFX_ERROR;
         TRACE (VINETIC, DBG_LEVEL_HIGH,
              ("\n\rDRV_ERROR: Cannot start DTMF transmission while DTMF "
               "generator is already in use on this channel\n\r"));
      }
      break;
   default:
      ret = IFX_ERROR;
      break;
   }

   return ret;
}

/**
   Stop CID data transmission
\param pLLChannel      Handle to TAPI low level channel structure
\param pCidConf        Pointer to the global CID configuration
\return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
\remarks
*/
IFX_int32_t IFX_TAPI_LL_SIG_CID_TX_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                         TAPI_CID_CONF_t *pCidConf)
{
   IFX_int32_t ret = IFX_ERROR;
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;

   switch (pCidConf->nStandard)
   {
   case IFX_TAPI_CID_STD_TELCORDIA:
   case IFX_TAPI_CID_STD_ETSI_FSK:
   case IFX_TAPI_CID_STD_SIN:
   case IFX_TAPI_CID_STD_NTT:
      if (pCh->cidSend.nState == CID_TRANSMIT)
      {
         /* flag a fake offhook to stop the FSK state machine */
         pCh->cidSend.nState = CID_OFFHOOK;
         ret = VINETIC_CidFskMachine (pCh);
      }
   case IFX_TAPI_CID_STD_ETSI_DTMF:
      if ((pCh->dtmfSend.state == DTMF_START) ||
          (pCh->dtmfSend.state == DTMF_TRANSMIT))
      {
         ret = irq_VINETIC_SIG_DtmfStop(pCh, IFX_FALSE);
      }
      break;
   default:
      ret = IFX_ERROR;
      break;
   }

   return ret;
}

/**
   CID Receiver data collection.
\param pChannel        Handle to TAPI_CHANNEL structure
\remarks
   This function is called by the function handling data packets in
   interrupt level. It collects the data given and stores it in a buffer
   which it gets from TAPI.
*/
IFX_return_t VINETIC_SIG_CID_RX_Data_Collect (TAPI_CHANNEL *pChannel)
{
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pChannel->pTapiDevice->pLLDev);
   VINETIC_CHANNEL *pCh = &pDev->pChannel [pChannel->nChannel];
   IFX_TAPI_CID_RX_DATA_t *pBuf = NULL;
   PACKET *pPacket = &pCh->cidRxPacket;
   IFX_uint32_t nLength = pPacket->cmd2 & CMD2_LEN;
   IFX_int32_t nOdd = pPacket->cmd2 & CMD2_ODD;
   IFX_uint32_t nLen;

   /* stop processing on illegal packet length */
   if (nLength == 0 || nLength > 2)
      return IFX_ERROR;

   /* packet contains fsk payload data if length is 2 words */
   if (nLength == 2)
   {
      /* packet contains fsk payload data */

      /* determine the length of data in bytes */
      nLen = nOdd ? 1 : 2;

      /* get a buffer with at least the length to store the received data
         This implicitely sets the HL status to IFX_TAPI_CID_RX_STATE_ONGOING
         and on no buffers sets the HL error to IFX_TAPI_CID_RX_ERROR_READ */
      pBuf = (IFX_TAPI_CID_RX_DATA_t *)TAPI_Phone_GetCidRxBuf (pChannel, nLen);
      if (pBuf != NULL)
      {
         /* at least the high byte is valid */
         pBuf->data [pBuf->nSize++] = HIGHBYTE (ntohs(pPacket->pData [1]));
         /* if odd bit isn't set, then the low byte is the second data */
         if (nOdd == 0)
            pBuf->data [pBuf->nSize++] = LOWBYTE (ntohs(pPacket->pData [1]));
      }
   }

   /* evaluate from the carrier detect flag if receiver has just finished  */
   /* if CD = 1, caller id receiving is still running */
   /* if CD = 0, carrier lost. Reception of block finished */
   if ( !(ntohs(pPacket->pData[0]) & 0x8000) )
   {
      /* event for the user : fsk reception ended */
      IFX_TAPI_EVENT_t tapiEvent;

      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_CID_RX_END;
      /* no direction in event - always local due to coding */
      IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
   }

   return IFX_SUCCESS;
}

#endif /* TAPI_CID */
