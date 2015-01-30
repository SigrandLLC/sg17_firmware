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
   Module      : drv_vinetic_init_cap.c
******************************************************************************/

/* ============================= */
/* includes                      */
/* ============================= */
#include "drv_vinetic_init.h"


/* ============================= */
/* Global Declarations           */
/* ============================= */
/**
   This command reads out the capabilities of the firmware variant. The command
   provides information about the features, which are implemented. It does not
   give information about the resource requirements regarding processing power
   (MIPs), which are consumed by each feature.
   The version and the length of the capability message can be read out by
   reading the first 32-bit word of the command.
*/
typedef union
{
   IFX_uint16_t value[20];
   struct
   {
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      /* cmd 1 */
      unsigned ch                            : 4;
      unsigned res                           : 4;
      unsigned cmd                           : 5;
      unsigned bc                            : 1;
      unsigned res0                          : 1;
      unsigned rw                            : 1;
      /* cmd 2 */
      unsigned length                        : 8;
      unsigned ecmd                          : 5;
      unsigned mod                           : 3;
      /* data */
      unsigned BLEN                          : 8;
      unsigned VERS                          : 8;
      unsigned NALI                          : 8;
      unsigned NPCM                          : 8;
      unsigned NCOD                          : 8;
      unsigned NSIG                          : 8;
      unsigned NEQ                           : 8;
      unsigned NAGC                          : 8;
      unsigned NWLEC                         : 8;
      unsigned NNLEC                         : 8;
      unsigned NWWLEC                        : 8;
      unsigned NNWLEC                        : 8;
      unsigned NDTMFG                        : 8;
      unsigned NUTG                          : 8;
      unsigned NCIDR                         : 8;
      unsigned NCIDS                         : 8;
      unsigned NMFTD                         : 8;
      unsigned NCPTD                         : 8;
      unsigned Res01                         : 8;
      unsigned NFAX                          : 8;
      unsigned Res02                         : 16;
      unsigned CODECS                        : 16;
      unsigned CMID                          : 8;
      unsigned CLOW                          : 8;
      unsigned PCOD                          : 8;
      unsigned CMAX                          : 8;
      unsigned Res03                         : 12;
      unsigned MFTDV                         : 4;
      unsigned TONES                         : 16;
      unsigned OVL                           : 8;
      unsigned FEAT                          : 8;
      unsigned ETC                           : 8;
      unsigned EPC                           : 8;
#endif /* LITTLE_ENDIAN */
#if (__BYTE_ORDER == __BIG_ENDIAN)
      /* cmd 1 */
      unsigned rw                            : 1;
      unsigned res0                          : 1;
      unsigned bc                            : 1;
      unsigned cmd                           : 5;
      unsigned res                           : 4;
      unsigned ch                            : 4;
      /* cmd 2 */
      unsigned mod                           : 3;
      unsigned ecmd                          : 5;
      unsigned length                        : 8;
      /* data */
      unsigned VERS                          : 8;
      unsigned BLEN                          : 8;
      unsigned NPCM                          : 8;
      unsigned NALI                          : 8;
      unsigned NSIG                          : 8;
      unsigned NCOD                          : 8;
      unsigned NAGC                          : 8;
      unsigned NEQ                           : 8;
      unsigned NNLEC                         : 8;
      unsigned NWLEC                         : 8;
      unsigned NNWLEC                        : 8;
      unsigned NWWLEC                        : 8;
      unsigned NUTG                          : 8;
      unsigned NDTMFG                        : 8;
      unsigned NCIDS                         : 8;
      unsigned NCIDR                         : 8;
      unsigned NCPTD                         : 8;
      unsigned NMFTD                         : 8;
      unsigned NFAX                          : 8;
      unsigned Res01                         : 8;
      unsigned Res02                         : 16;
      unsigned CODECS                        : 16;
      unsigned CLOW                          : 8;
      unsigned CMID                          : 8;
      unsigned CMAX                          : 8;
      unsigned PCOD                          : 8;
      unsigned MFTDV                         : 4;
      unsigned Res03                         : 12;
      unsigned TONES                         : 16;
      unsigned FEAT                          : 8;
      unsigned OVL                           : 8;
      unsigned EPC                           : 8;
      unsigned ETC                           : 8;
#endif /* BIG_ENDIAN */
   } bit;
} FWM_CAPS;


/* ============================= */
/* Local function declaration    */
/* ============================= */
IFX_LOCAL IFX_void_t AddCapability (IFX_TAPI_CAP_t* CapList,
                                    IFX_uint32_t *pnCap,
                                    IFX_char_t const * description,
                                    IFX_int32_t type, IFX_int32_t value);


/* ============================= */
/* Function definitions          */
/* ============================= */

/**
  set all vinetic capabilities
\param pDev ptr to actual Vinetic Device
\return
   IFX_SUCCESS or IFX_ERROR if no memory is available
Remark:
   Macro MAX_CAPS must match with the capabilities number. So adapt this macro
   accordingly if new capabilities are added.
*/
IFX_int32_t VINETIC_AddCaps (VINETIC_DEVICE *pDev)
{
   /* capability list */
   IFX_TAPI_CAP_t *CapList;

   /* start adding the capability with coders */
   IFX_uint32_t nCap = 0;

   /* holds the maximum amount of capabilities */
   if (pDev->CapList == NULL)
   {
      pDev->CapList = (IFX_TAPI_CAP_t*)
                      IFXOS_MALLOC(MAX_CAPS * sizeof(IFX_TAPI_CAP_t));
      if (pDev->CapList == NULL)
      {
         SET_DEV_ERROR (VIN_ERR_NO_MEM);
         return IFX_ERROR;
      }
      /*FIXME: Rescheduling required for malloc*/
      IFXOS_Wait (1);
   }

   CapList = pDev->CapList;

   AddCapability (CapList, &nCap,"INFINEON TECHNOLOGIES",IFX_TAPI_CAP_TYPE_VENDOR,0);
#ifdef VIN_2CPE
   AddCapability (CapList, &nCap, "VINETIC-CPE", IFX_TAPI_CAP_TYPE_DEVICE, 0);
#else
   AddCapability (CapList, &nCap, "VINETIC", IFX_TAPI_CAP_TYPE_DEVICE, 0);
#endif /* VIN_2CPE */

   AddCapability (CapList, &nCap, "POTS", IFX_TAPI_CAP_TYPE_PORT, IFX_TAPI_CAP_PORT_POTS);
   AddCapability (CapList, &nCap, "PSTN", IFX_TAPI_CAP_TYPE_PORT, IFX_TAPI_CAP_PORT_PSTN);
   if (pDev->nDevState & DS_BASIC_INIT)
   {
      switch (pDev->nChipRev)
      {
         case  VINETIC_2CPE_V21:
            AddCapability (CapList, &nCap, "DEVICE VERSION", IFX_TAPI_CAP_TYPE_DEVVERS, 0x0201);
            break;
         case  VINETIC_V14:
            AddCapability (CapList, &nCap, "DEVICE VERSION", IFX_TAPI_CAP_TYPE_DEVVERS, 0x0104);
            break;
         default:
         case  VINETIC_2CPE_V22:
            AddCapability (CapList, &nCap, "DEVICE VERSION", IFX_TAPI_CAP_TYPE_DEVVERS, 0x0202);
            break;
      }
      switch (pDev->nChipType)
      {
         default:
         case  VINETIC_TYPE_2CPE:
            AddCapability (CapList, &nCap, "DEVICE TYPE",
                           IFX_TAPI_CAP_TYPE_DEVTYPE,
                           IFX_TAPI_DEV_TYPE_VINETIC_CPE);
            break;
         case  VINETIC_TYPE_VIP:
         case  VINETIC_TYPE_C:
         case  VINETIC_TYPE_M:
            AddCapability (CapList, &nCap, "DEVICE TYPE",
                           IFX_TAPI_CAP_TYPE_DEVTYPE,
                           IFX_TAPI_DEV_TYPE_VINETIC);
            break;
         case  VINETIC_TYPE_S:
            AddCapability (CapList, &nCap, "DEVICE TYPE",
                           IFX_TAPI_CAP_TYPE_DEVTYPE,
                           IFX_TAPI_DEV_TYPE_VINETIC_S);
            break;
      }
   }
#if (VIN_CFG_FEATURES & VIN_FEAT_PACKET)
   if ((pDev->nDevState & DS_FW_DLD) != 0)
   {
      /* coder features of M and VIP versions */
      AddCapability (CapList, &nCap, "DSP", IFX_TAPI_CAP_TYPE_DSP, 0);
      if ((!((pDev->nEdspVers[0] & ECMD_VERS_FEATURES)
              == ECMD_VERS_FEATURES_C)                          ||
            ((pDev->nEdspVers[0] & ECMD_VERS_FEATURES)
              == ECMD_VERS_FEATURES_S))                         &&
            (pDev->nChipType != VINETIC_TYPE_S))
      {
         /* add the defaults */
         AddCapability (CapList, &nCap, "G.726 16 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_16);
         AddCapability (CapList, &nCap, "G.726 24 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_24);
         AddCapability (CapList, &nCap, "G.726 32 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_32);
         AddCapability (CapList, &nCap, "G.726 40 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_40);
         AddCapability (CapList, &nCap, "u-LAW",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_MLAW);
         AddCapability (CapList, &nCap, "A-LAW",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ALAW);
         if ((pDev->nEdspVers[0] & ECMD_VERS_MV) == ECMD_VERS_MV_4)
         {
            switch (pDev->nEdspVers[0] & ECMD_VERS_FEATURES)
            {
            case ECMD_VERS_FEATURES_729ABE_G723:
               AddCapability (CapList, &nCap, "G.723 6.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_63);
               AddCapability (CapList, &nCap, "G.723 5.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_53);
               AddCapability (CapList, &nCap, "G.729",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
               AddCapability (CapList, &nCap, "G.729E",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729_E);
#ifdef VIN_2CPE
               AddCapability (CapList, &nCap, "iLBC 13.3 kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ILBC_133);
               AddCapability (CapList, &nCap, "iLBC 15.2 kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ILBC_152);
               AddCapability (CapList, &nCap, "T.38", IFX_TAPI_CAP_TYPE_T38, 1);
#endif /* VIN_2CPE */
               break;
            case ECMD_VERS_FEATURES_G729AB_G723:
               AddCapability (CapList, &nCap, "G.729",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
               AddCapability (CapList, &nCap, "G.729E",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729_E);
#ifdef VIN_2CPE
               AddCapability (CapList, &nCap, "T.38", IFX_TAPI_CAP_TYPE_T38, 1);
#else
               AddCapability (CapList, &nCap, "G.723 6.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_63);
               AddCapability (CapList, &nCap, "G.723 5.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_53);
#endif /* VIN_2CPE */
               break;
            case ECMD_VERS_FEATURES_G728_G723:
               AddCapability (CapList, &nCap, "G.728",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G728);
               AddCapability (CapList, &nCap, "G.723 6.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_63);
               AddCapability (CapList, &nCap, "G.723 5.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_53);
               break;
            case ECMD_VERS_FEATURES_G728_G729ABE:
               AddCapability (CapList, &nCap, "G.728",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G728);
               AddCapability (CapList, &nCap, "G.729",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
               AddCapability (CapList, &nCap, "G.729E",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729_E);
               break;
            case ECMD_VERS_FEATURES_729ABE_T38:
               AddCapability (CapList, &nCap, "G.729",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
               AddCapability (CapList, &nCap, "G.729E",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729_E);
               AddCapability (CapList, &nCap, "T.38",
                              IFX_TAPI_CAP_TYPE_T38, 1);
               break;
            case ECMD_VERS_FEATURES_G729AB_G728_T38:
               AddCapability (CapList, &nCap, "G.729",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
               AddCapability (CapList, &nCap, "G.728",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G728);
               AddCapability (CapList, &nCap, "T.38",
                              IFX_TAPI_CAP_TYPE_T38, 1);
               break;
            case ECMD_VERS_FEATURES_G723_G728_T38:
               AddCapability (CapList, &nCap, "G.723 6.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_63);
               AddCapability (CapList, &nCap, "G.723 5.3kbps",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_53);
               AddCapability (CapList, &nCap, "G.728",
                              IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G728);
               AddCapability (CapList, &nCap, "T.38",
                              IFX_TAPI_CAP_TYPE_T38, 1);
               break;
            default:
               TRACE (VINETIC,DBG_LEVEL_NORMAL, ("Warning: Unknown feature set\n\r"));
               break;
            }
         }
         /* set coder channels */
         switch (pDev->nEdspVers[0] & ECMD_VERS_MV)
         {
         case ECMD_VERS_MV_8:
            AddCapability (CapList, &nCap, "Coder",
                           IFX_TAPI_CAP_TYPE_CODECS, 8);
            break;
         case 0x6000:
            AddCapability (CapList, &nCap, "Coder",
                           IFX_TAPI_CAP_TYPE_CODECS, 6);
            break;
         default:
            AddCapability (CapList, &nCap, "Coder",
                           IFX_TAPI_CAP_TYPE_CODECS, 4);
         }
      }
   }
#endif /* (VIN_CFG_FEATURES & VIN_FEAT_PACKET) */
#if (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M))
   switch (pDev->nChipType)
   {
   case VINETIC_TYPE_S:
      AddCapability (CapList, &nCap, "PCM", IFX_TAPI_CAP_TYPE_PCM, 4);
      break;
   default:
      AddCapability (CapList, &nCap, "PCM", IFX_TAPI_CAP_TYPE_PCM, 8);
      AddCapability (CapList, &nCap, "CED",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, IFX_TAPI_CAP_SIG_DETECT_CED);
      AddCapability (CapList, &nCap, "CNG",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, IFX_TAPI_CAP_SIG_DETECT_CNG);
      AddCapability (CapList, &nCap, "DIS",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, IFX_TAPI_CAP_SIG_DETECT_DIS);
      AddCapability (CapList, &nCap, "POWER",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, IFX_TAPI_CAP_SIG_DETECT_POWER);
      AddCapability (CapList, &nCap, "CPT",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, IFX_TAPI_CAP_SIG_DETECT_CPT);
      break;
   }
#endif /* (VIN_CFG_FEATURES & (VIN_FEAT_VIN_C | VIN_FEAT_VIN_M)) */

   AddCapability (CapList, &nCap, "Phones",
                  IFX_TAPI_CAP_TYPE_PHONES, pDev->caps.nALI);
   /* check if no array out of bounds */
   IFXOS_ASSERT (nCap <= MAX_CAPS);
   pDev->nMaxCaps = (IFX_uint8_t)nCap;

   return IFX_SUCCESS;
}



/**
  Add capability to given list
\param pnCap        -
\param description  -
\param type         -
\param value        -
\return
   none
\remarks
   add a capability to the existing ones and increase the counter nCap
*/
IFX_LOCAL IFX_void_t AddCapability (IFX_TAPI_CAP_t* CapList,
                                    IFX_uint32_t *pnCap, IFX_char_t const * description,
                                    IFX_int32_t type, IFX_int32_t value)
{
   IFX_uint32_t capnr;

   if (pnCap  == NULL)
      return;

   if (*pnCap >= MAX_CAPS)
      return;

   capnr = (*pnCap);
   /* Note: strncpy will fill the target string with \0 if the source string is
    * shorter than length parameter */
   strncpy (CapList[capnr].desc, description, sizeof (CapList[0].desc));
   CapList[capnr].captype = (IFX_TAPI_CAP_TYPE_t)type;
   CapList[capnr].cap = value;
   CapList[capnr].handle = (int)capnr;
   (*pnCap) = capnr + 1;
   TRACE (VINETIC,DBG_LEVEL_LOW, ("Cap: %s , type %d = %d size=%d\n\r",
          description, type, value,strlen(description)));
}



/**
   Checks if a specific capability is supported from VINETIC
\param pLLDev      Handle to VINETIC_DEVICE structure
\param pCapList    Handle to IFX_TAPI_CAP_t structure
\return Support status of the capability
   - 0 if not supported
   - 1 if supported
\remarks
   This function compares only the captype and the cap members of the given
   IFX_TAPI_CAP_t structure with the ones of the VINETIC_CAP_Table.
*/
IFX_int32_t TAPI_LL_Phone_Check_Capability (IFX_TAPI_LL_DEV_t *pLLDev,
                                            IFX_TAPI_CAP_t *pCapList)
{
   VINETIC_DEVICE *pDev = pLLDev;
   IFX_int32_t cnt;
   IFX_int32_t ret = 0;

   /* do checks */
   for (cnt = 0; cnt < pDev->nMaxCaps; cnt++)
   {
      if (pCapList->captype == pDev->CapList[cnt].captype)
      {
         switch (pCapList->captype)
         {
         /* Handle number counters, cap is returned */
         case IFX_TAPI_CAP_TYPE_PCM:
         case IFX_TAPI_CAP_TYPE_CODECS:
         case IFX_TAPI_CAP_TYPE_PHONES:
         case IFX_TAPI_CAP_TYPE_T38:
         case IFX_TAPI_CAP_TYPE_DEVVERS:
         case IFX_TAPI_CAP_TYPE_DEVTYPE:
            pCapList->cap = pDev->CapList[cnt].cap;
            ret = 1;
            break;
         case IFX_TAPI_CAP_TYPE_DEVICE:
         case IFX_TAPI_CAP_TYPE_VENDOR:
            strncpy (pCapList->desc, pDev->CapList[cnt].desc, sizeof (pCapList->desc));
            ret = 1;
            break;
         default:
            /* default IFX_TRUE or IFX_FALSE capabilities */
            if (pCapList->cap == pDev->CapList[cnt].cap)
               ret = 1;
            break;
         }
      }
   }
   return ret;
}



/**
   Returns Vinetic's number of capability lists
\return
   The amount of capabilitie entries
\remarks
   VINETIC_CAP_Table can be updated without any changes in TAPI functions
   concerning capabilities' services.
*/
IFX_uint32_t TAPI_LL_Phone_Get_Capabilities (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VINETIC_DEVICE *pDev = pLLDev;
   return (pDev->nMaxCaps);
}



/**
   returns Vinetic's capability lists
\param pLLDev          Handle to VINETIC_DEVICE structure
\param pCapList        Handle to IFX_TAPI_CAP_t structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   The whole set of VINETIC capabilities is copied to the passed pointer
   argument.
*/
IFX_return_t TAPI_LL_Phone_Get_Capability_List (IFX_TAPI_LL_DEV_t *pLLDev,
                                                IFX_TAPI_CAP_t *pCapList)
{
   VINETIC_DEVICE *pDev = pLLDev;

   memcpy(pCapList, pDev->CapList,
          ((IFX_uint32_t)pDev->nMaxCaps * sizeof(IFX_TAPI_CAP_t)));
   return IFX_SUCCESS;
}



/**
  Read out the Capabilities from firmware.
\param VINETIC_DEVICE *pDev
\return
   Success or Error
\remarks
   none.
*/

IFX_return_t VINETIC_Get_FwCap (VINETIC_DEVICE *pDev)
{
   IFX_return_t  ret = IFX_ERROR;
   FWM_CAPS      capCmd;

   pDev->bCapsRead = IFX_FALSE;

   if (pDev->nDevState & DS_FW_DLD)
   {
#if 0
      if (pDev->nEdspVers[2] & ECMD_VERS_CAP)
#else
      /* Current static configuration for all V2.2 versions */
      /* Capability bit in version register is incorrect in RTP 0.17.32. */
      if (1)
#endif
      {
         /* Initialise the firmware command for reading the capabilities */
         memset((IFX_void_t *)&capCmd, 0, sizeof(FWM_CAPS));

         /* Command Header */
         capCmd.value[0]   = CMD1_EOP;
         capCmd.value[1]   = ECMD_CAPS;
         capCmd.bit.length = 18;

         ret = CmdRead(pDev, (IFX_uint16_t *)&capCmd,
                       (IFX_uint16_t *)&capCmd, capCmd.bit.length);
      }
      else
      {
         /* According to FW version register the capability command is not
            supported. So do not try this and abort. */
         ret = IFX_ERROR;
      }

      if (ret != IFX_SUCCESS)
      {
         TRACE (VINETIC, DBG_LEVEL_LOW,
                ("INFO: Firmware capabilities-message not supported\n\r"));
      }
      else /* Store the capabilities*/
      {
         TRACE (VINETIC, DBG_LEVEL_LOW,
                ("INFO: Capabilities where read with firmware message\n\r"));
         pDev->bCapsRead = IFX_TRUE;

         /* Out internal structure is different from the firmware message to
            be more flexible. So when the firmware message changes we can
            handle different versions of the message by first looking to the
            version field before copying the values. Also some different
            interpretation of values is possible. */

         /* Number of PCM Channels */
         pDev->caps.nPCM = capCmd.bit.NPCM;
         /* Number of Analog Line Channels */
         pDev->caps.nALI = capCmd.bit.NALI;
         /* Number of Signaling Channels */
         pDev->caps.nSIG = capCmd.bit.NSIG;
         /* Number of Coder Channels */
         pDev->caps.nCOD = capCmd.bit.NCOD;
         /* Number of AGCs */
         pDev->caps.nAGC = capCmd.bit.NAGC;
         /* Number of Equalizers */
         pDev->caps.nEQ = capCmd.bit.NEQ;
         /* Number of Near-End LECs */
         pDev->caps.nNLEC = capCmd.bit.NNLEC;
         /* Number of Combined Near-End/Far-End LECs */
         pDev->caps.nWLEC = capCmd.bit.NWLEC;
         /* Number of Near-End Wideband LECs */
         pDev->caps.nNWLEC = capCmd.bit.NNWLEC;
         /* Number of Combined Near-End/Far-End Wideband LECs */
         pDev->caps.nWWLEC = capCmd.bit.NWWLEC;
         /* Number of Universal Tone Generators */
         pDev->caps.nUTG = capCmd.bit.NUTG;
         /* Number of DTMF Generators */
         pDev->caps.nDTMFG = capCmd.bit.NDTMFG;
         /* Number of Caller ID Senders */
         pDev->caps.nCIDS = capCmd.bit.NCIDS;
         /* Number of Caller ID Receivers */
         pDev->caps.nCIDR = capCmd.bit.NCIDR;
         /* Number of Call Progress Tone Detectors */
         pDev->caps.nCPTD = capCmd.bit.NCPTD;
         /* Number of Modem and Fax Tone Discriminators (MFTDs) */
         pDev->caps.nMFTD = capCmd.bit.NMFTD;
         /* Number of FAX Channels with FAX Relay (T.38) Support */
         pDev->caps.nFAX = capCmd.bit.NFAX;
         /* Codecs */
         pDev->caps.CODECS = capCmd.bit.CODECS;
         /* Maximum Number of Low Complexity Coders for the Coder Channel */
         pDev->caps.CLOW = capCmd.bit.CLOW;
         /* Maximum Number of Mid Complexity Coders for the Coder Channel */
         pDev->caps.CMID = capCmd.bit.CLOW;
         /* Maximum Number of High Complexity Coders for the Coder Channel*/
         pDev->caps.CMAX = capCmd.bit.CMAX;
         /* PCM Channel Coders */
         pDev->caps.PCOD = capCmd.bit.PCOD;
         /* MFTD Version */
         pDev->caps.MFTDV = capCmd.bit.MFTDV;
         /* Tone Detection Capabilities */
         pDev->caps.TONES = capCmd.bit.TONES;
         /* Features */
         pDev->caps.FEAT = capCmd.bit.FEAT;
         /* Overlays */
         pDev->caps.OVL = capCmd.bit.OVL;
         /* Event Playout Capabilities */
         pDev->caps.EPC = capCmd.bit.EPC;
         /* Event Transmission Capabilities */
         pDev->caps.ETC = capCmd.bit.ETC;

         /* Number of UTG resources per channel (== SIG module), either 1 or 2 */
         pDev->caps.nUtgPerCh = (capCmd.bit.FEAT & EDSP_CAP_FEAT_UTGUD) ? 2 : 1;
         /* Support for extended jitter buffer statistics is yes by default */
         pDev->caps.bExtendedJBsupported = IFX_TRUE;
         /* set the number of NLEC equal to the number of WLEC when supported */
         if (pDev->caps.nWLEC > 0)
         {
            pDev->caps.nNLEC = pDev->caps.nWLEC;
         }
#if 0
         /* Support for two UTDs per SIG module */
         IFX_uint32_t  bUtd2supported : 1;
#endif
      }
   }

   return ret;
}
