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

******************************************************************************
   Module      : drv_vinetic_alm_cpe.c
   Date        : 2003-10-28
   Description : This file contains the implementations of vinetic 2CPE related
                 tapi low level functions for ALM module.
*******************************************************************************/

#include "drv_vinetic_api.h"


extern   IFX_void_t serve_LineX_Int(VINETIC_CHANNEL *pCh,
                                    IFX_uint16_t nLineX_Int);


/*******************************************************************************
Description:
   Switch polarity.
Arguments:
   pChannel        - handle to TAPI_CHANNEL structured
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Polarity_Set (IFX_TAPI_LL_CH_t *pLLChannel)
{
   TRACE (VINETIC, DBG_LEVEL_NORMAL,
         ("VINETIC WARN: Polarity feature not supported\r\n"));
   return IFX_SUCCESS;
}


/*******************************************************************************
Description:
   Enable / disable the automatic battery switch.
Arguments:
   pChannel        - handle to TAPI_CHANNEL structured
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_AutoBatterySwitch(IFX_TAPI_LL_CH_t *pLLChannel)
{
   TRACE (VINETIC, DBG_LEVEL_NORMAL,
         ("VINETIC WARN: AutoBatterySwitch feature not supported\r\n"));
   return IFX_SUCCESS;
}

/*******************************************************************************
Description:
   Prepare parameters and call the Target Configuration Function to switch the
   line mode.
Arguments:
   pChannel        - handle to TAPI_CHANNEL structure
   nMode           - linefeed mode
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Mode_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_int32_t nMode,
                                               IFX_uint8_t nTapiLineMode)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t      ret;
   CMD_OpMode_t     opmod;

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: IFX_TAPI_LL_ALM_CPE_Line_Mode_Set\n"));

   /* sanity check */
   if (pCh->nChannel - 1 >= pDev->caps.nALI)
   {
      return(IFX_SUCCESS);
   }

   memset (&opmod, 0, sizeof (CMD_OpMode_t));
   opmod.RW      = VIN_CMD_RD;
   opmod.CMD     = VIN_CMD_ALM;
   opmod.MOD     = VIN_MOD_DCCTL;
   opmod.ECMD    = VIN_ECMD_OPMOD;
   opmod.CH      = pCh->nChannel -1;
   opmod.LENGTH  = 1;

   /* check current configuration */
   ret = CmdRead (pDev, (IFX_uint16_t *)((IFX_void_t *) &opmod),
                  (IFX_uint16_t *)((IFX_void_t *) &opmod), 1);
   if (ret != IFX_SUCCESS)
      goto error;

   /* check if transition is valid, prepare command */
   opmod.RW      = VIN_CMD_WR;

   switch (nMode)
   {
      case IFX_TAPI_LINE_FEED_ACTIVE:
      case IFX_TAPI_LINE_FEED_RING_PAUSE:
      /* for backward compatibility only */
      case IFX_TAPI_LINE_FEED_NORMAL_AUTO:
      case IFX_TAPI_LINE_FEED_ACTIVE_LOW:
      case IFX_TAPI_LINE_FEED_ACTIVE_BOOSTED:
         switch (opmod.OP_MODE)
         {
            case VIN_OPMOD_ACT:
            case VIN_OPMOD_RING:
            case VIN_OPMOD_ONHOOK:
               opmod.OP_MODE = VIN_OPMOD_ACT;
               /* in case of ring pause, leave reverse
                  polarity bit as is */
               if (nMode != IFX_TAPI_LINE_FEED_RING_PAUSE)
                  opmod.REV_POL = VIN_OPMOD_POL_NORMAL;
               break;
            default:
               ret = IFX_ERROR;
               goto error;
         }
         break;
      case IFX_TAPI_LINE_FEED_ACTIVE_REV:
      /* for backward compatibility only */
      case IFX_TAPI_LINE_FEED_REVERSED_AUTO:
      case IFX_TAPI_LINE_FEED_PARKED_REVERSED:
         switch (opmod.OP_MODE)
         {
            case VIN_OPMOD_ACT:
            case VIN_OPMOD_RING:
            case VIN_OPMOD_ONHOOK:
               opmod.OP_MODE = VIN_OPMOD_ACT;
               opmod.REV_POL = VIN_OPMOD_POL_REVERSE;
               break;
            default:
               ret = IFX_ERROR;
               goto error;
         }
         break;
      case IFX_TAPI_LINE_FEED_STANDBY:
         switch (opmod.OP_MODE)
         {
            case VIN_OPMOD_ACT:
            case VIN_OPMOD_RING:
            case VIN_OPMOD_ONHOOK:
            case VIN_OPMOD_PDH:
               opmod.OP_MODE = VIN_OPMOD_ONHOOK;
               opmod.REV_POL = VIN_OPMOD_POL_NORMAL;
               break;
            default:
               ret = IFX_ERROR;
               goto error;
         }
         break;
      case IFX_TAPI_LINE_FEED_DISABLED:
         opmod.OP_MODE = VIN_OPMOD_PDH;
         opmod.REV_POL = VIN_OPMOD_POL_NORMAL;
         break;
      case IFX_TAPI_LINE_FEED_RING_BURST:
         switch (opmod.OP_MODE)
         {
            case VIN_OPMOD_ACT:        /* check for hook in case of active? */
            case VIN_OPMOD_RING:
            case VIN_OPMOD_ONHOOK:
               /* leave reversal bit as is */
               opmod.OP_MODE = VIN_OPMOD_RING;
               break;
            default:
               ret = IFX_ERROR;
               goto error;
         }
         break;
      /* unsupported linemodes */
      case IFX_TAPI_LINE_FEED_HIGH_IMPEDANCE:
      case IFX_TAPI_LINE_FEED_GROUND_START:
      case IFX_TAPI_LINE_FEED_METER:
      case IFX_TAPI_LINE_FEED_ACT_TEST:
      case IFX_TAPI_LINE_FEED_ACT_TESTIN:
      case IFX_TAPI_LINE_FEED_DISABLED_RESISTIVE_SWITCH:
      default:
         SET_ERROR (VIN_ERR_NOTSUPPORTED);
         return IFX_ERROR;
   }

   ret = CmdWrite(pDev, (IFX_uint16_t *)((IFX_void_t *) &opmod), 1);

error:
   return ret;
}

/*******************************************************************************
Description:
   Set line type.
Arguments:
   pLLChannel      - handle to VINETIC_CHANNEL structure
   nType           - new line type, can be IFX_TAPI_LINE_TYPE_FXS
                                        or IFX_TAPI_LINE_TYPE_FXO

Return:
   IFX_SUCCESS or IFX_ERROR

Remarks:
   In case switching to FXS line type, the line mode used is Power Down High
   Impedance (PDH).
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Line_Type_Set(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_LINE_TYPE_t nType)
{
   VINETIC_CHANNEL *pCh  = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);
   IFX_int32_t ret;
   CMD_OpMode_t opmod;

   Vinetic_IrqLockDevice(pDev);

   TRACE(VINETIC, DBG_LEVEL_LOW, ("INFO: TAPI_LL_Phone_SwitchLine called\n"));

   memset (&opmod, 0, sizeof (CMD_OpMode_t));
   opmod.RW      = VIN_CMD_WR;
   opmod.CMD     = VIN_CMD_ALM;
   opmod.MOD     = VIN_MOD_DCCTL;
   opmod.ECMD    = VIN_ECMD_OPMOD;
   opmod.CH      = pCh->nChannel - 1;
   opmod.LENGTH  = 1;

   switch (nType)
   {
      case IFX_TAPI_LINE_TYPE_FXS:
         TRACE(VINETIC, DBG_LEVEL_NORMAL,
               ("INFO: TAPI_LL_Phone_SwitchLine: Dev%d,Ch%d: "
                "switch to FXS line type\n",
                pDev->nDevNr, pCh->nChannel - 1));

         opmod.OP_MODE = VIN_OPMOD_PDH;
         break;

      case IFX_TAPI_LINE_TYPE_FXO:
         TRACE(VINETIC, DBG_LEVEL_NORMAL,
               ("INFO: TAPI_LL_Phone_SwitchLine: Dev%d,Ch%d: "
                "switch to FXO line type\n",
                pDev->nDevNr, pCh->nChannel - 1));
         /* switching to linemode FXO requires to switch to PDH first... */
         IFXOS_MutexLock (pDev->mbxAcc);
         opmod.OP_MODE = VIN_OPMOD_PDH;
         ret = CmdWriteIsr (pDev, (IFX_uint16_t *)((IFX_void_t *) &opmod), 1);
         IFXOS_MutexUnlock (pDev->mbxAcc);
         opmod.OP_MODE = VIN_OPMOD_FXO;
         break;

      default:
         Vinetic_IrqUnlockDevice(pDev);
         return IFX_ERROR;
   }

   IFXOS_MutexLock (pDev->mbxAcc);
   ret = CmdWriteIsr (pDev, (IFX_uint16_t *)((IFX_void_t *) &opmod), 1);
   IFXOS_MutexUnlock (pDev->mbxAcc);

   Vinetic_IrqUnlockDevice(pDev);

   return ret;
}

/*********************************************************************************
Description:
   This service enables or disables a high level path of a phone channel.
Arguments:
   pChannel - handle to TAPI_CHANNEL structure
   bEnable  - The parameter represent a boolean value of \ref IFX_TAPI_LINE_LEVEL_t.
               - 0: IFX_TAPI_LINE_LEVEL_DISABLE, disable the high level path.
               - 1: IFX_TAPI_LINE_LEVEL_ENABLE, enable the high level path.
Return:
   IFX_SUCCESS or IFX_ERROR
Remarks:
   This service is intended for phone channels only and must be used in
   combination with IFX_TAPI_PHONE_VOLUME_SET or IFX_TAPI_PCM_VOLUME_SET
   to set the max. level (IFX_TAPI_LINE_VOLUME_HIGH) or to restore level
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Volume_High_Level (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_int32_t bEnable)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE*) (pCh->pParent);

   IFX_int32_t      ret;

   CMD_OpMode_t     opmod;

   /* sanity check */
   if (pCh->nChannel - 1 >= pDev->caps.nALI)
   {
      return(IFX_SUCCESS);
   }

   memset (&opmod, 0, sizeof (CMD_OpMode_t));
   opmod.RW      = VIN_CMD_RD;
   opmod.CMD     = VIN_CMD_ALM;
   opmod.MOD     = VIN_MOD_DCCTL;
   opmod.ECMD    = VIN_ECMD_OPMOD;
   opmod.CH      = pCh->nChannel -1;
   opmod.LENGTH  = 1;

   /* check current configuration */
   ret = CmdRead (pDev,
                 (IFX_uint16_t *)((IFX_void_t *)&opmod),
                 (IFX_uint16_t *)((IFX_void_t *)&opmod), 1);
   if (ret != IFX_SUCCESS)
      goto error;

   Vinetic_IrqLockDevice(pDev);
   opmod.RW      = VIN_CMD_WR;
   opmod.HOWLER  = bEnable ? VIN_OPMOD_HOWLER_ON : VIN_OPMOD_HOWLER_OFF;

   /*FIXME: FPT:*/
#if 0
   /* check if linemode was possibly set in interrupt context after reading
      the current status (onOvertemp it might have been changed to power down */
   if ((pChannel->TapiOpControlData.nLineMode
        == IFX_TAPI_LINE_FEED_DISABLED) &&
       (opmod.OP_MODE != VIN_OPMOD_PDH))
   {
      /* line has been switched to power down in the meantime */
      ret = IFX_ERROR;
   }
   else
#endif
   {
      IFXOS_MutexLock (pDev->mbxAcc);
      ret = CmdWriteIsr (pDev, (IFX_uint16_t *)((IFX_void_t *) &opmod), 1);
      IFXOS_MutexUnlock (pDev->mbxAcc);
   }
   Vinetic_IrqUnlockDevice(pDev);

error:
   return ret;
}


/*******************************************************************************
Description:
   Prepare parameters and call the Target Configuration Function to set
   the ring configuration.
Arguments:
   pChannel        - handle to TAPI_CHANNEL structure
   pRingConfig     - pointer to ring config structure
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Ring_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_RING_CFG_t const *pRingConfig)
{
   /* not supported - fast ring trip is configured via bbd download
      because this setting affects also the hook thresholds */
   TRACE (VINETIC, DBG_LEVEL_HIGH,
         ("VINETIC ERROR: Ring Config supported only via download\r\n"));

   return IFX_ERROR;
}


/*******************************************************************************
Description:
   Configure metering mode of chip
Arguments:
   pChannel    - handle to TAPI_CHANNEL structure
   nMode       - use TTX (0) or reverse polarity (1)
   nFreq       - Default/12 KHz/16 KHz
Return:
   IFX_SUCCESS or IFX_ERROR
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Metering_Cfg(IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_uint8_t nMode,
                                             IFX_uint8_t nFreq)
{
   TRACE (VINETIC, DBG_LEVEL_HIGH,
         ("VINETIC ERROR: Metering feature not supported\r\n"));

   return IFX_ERROR;
}

/*******************************************************************************
Description:
   Restores the line state back after fault
Arguments:
   pChannel - handle to TAPI_CHANNEL structure
Return:
   IFX_SUCCESS or IFX_ERROR
Remarks:
   On fault the line state is stored and with this function reapplied again
*******************************************************************************/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_FaultLine_Restore (IFX_TAPI_LL_CH_t *pLLChannel)
{

   /** \todo Implement this function */

   return IFX_ERROR;
}

/**
This service generates an on or off hook event for the low level driver.
\param pChannel Handle to TAPI_CONNECTION structure
\param bHook - if 1 generate off hook, if 0 generate on hook
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
   The hook event is triggered by calling the irq handler directly from
   here. The hook event then gets to the hook state machine for
   validation. Depending on the timing of calling this interface
   also hook flash and pulse dialing can be verified. */
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Test_HookGen (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_boolean_t bHook)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) (pCh->pParent);

   if (pCh->nChannel <= pDev->caps.nALI)
   {
      serve_LineX_Int(pCh, bHook ? V2CPE_LINE1_INT_OFFHOOK :
                                   V2CPE_LINE1_INT_ONHOOK);
      return IFX_SUCCESS;
   }
   else
   {
      TRACE (VINETIC, DBG_LEVEL_NORMAL, ("Cannot generate Hook event "
             "on non existing analog channel %d\r\n", pCh->nChannel));
      return IFX_ERROR;
   }
}

/**
This service turns on the ALM loop in the 8kHz domain for testing.
As a result signals from I1-I5 are looped back to the output of the ALM.
Additionally the signals will pass through to the line undisturbed.
\param pChannel - handle to TAPI_CHANNEL structure
\return Return value according to IFX_return_t
   - IFX_ERROR if an error occured
   - IFX_SUCCESS if successful
\remarks
*/
IFX_int32_t IFX_TAPI_LL_ALM_CPE_Test_Loop (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_TAPI_TEST_LOOP_t* pLoop)
{
   VINETIC_CHANNEL *pCh = (VINETIC_CHANNEL *) pLLChannel;
   VINETIC_DEVICE *pDev = (VINETIC_DEVICE *) (pCh->pParent);
   IFX_int32_t      err = IFX_SUCCESS;
   CMD_DCCTL_Debug_t  debugCfg;

   memset (&debugCfg, 0, sizeof (CMD_DCCTL_Debug_t));
   debugCfg.CMD     = VIN_CMD_ALM;
   debugCfg.MOD     = VIN_MOD_DCCTL;
   debugCfg.ECMD    = CMD_DCCTL_DEBUG;

   /* read out first */
   debugCfg.RW      = VIN_CMD_RD;
   debugCfg.CH      = pCh->nChannel -1;
   debugCfg.LENGTH  = 5;
   err = CmdRead (pDev, (IFX_uint16_t *)((IFX_void_t *)&debugCfg),
                        (IFX_uint16_t *)((IFX_void_t *)&debugCfg), 5);

   if (err != IFX_SUCCESS)
      goto error;

   /* enable or disable testloop */
   debugCfg.RW     = VIN_CMD_WR;

   if (pLoop->bAnalog)
   {
      debugCfg.data1  = 0x0001;   /* enable debug mode */
      debugCfg.data3 |= 0x8040;   /* set 8kHz loopback and AC enable bits */
   }
   else
   {
      debugCfg.data1  = 0x0000;   /* enable normal mode */
      debugCfg.data3 &= ~0x0040;  /* clear 8KHz loopback bit */
   }
   err = CmdWrite (pDev, (IFX_uint16_t *)((IFX_void_t *)&debugCfg), 5);

error:
   return (err == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}


void VINETIC_ALM_CPE_Func_Register (IFX_TAPI_DRV_CTX_ALM_t *pAlm)
{
   /* Fill the function pointers of signaling module */
   pAlm->Line_Polarity_Set = IFX_NULL;
   pAlm->Line_Mode_Set     = IFX_TAPI_LL_ALM_CPE_Line_Mode_Set;
   pAlm->Line_Type_Set     = IFX_TAPI_LL_ALM_CPE_Line_Type_Set;
   pAlm->AutoBatterySwitch = IFX_TAPI_LL_ALM_CPE_AutoBatterySwitch;
   pAlm->Volume_High_Level = IFX_TAPI_LL_ALM_CPE_Volume_High_Level;
   pAlm->Ring_Cfg          = IFX_TAPI_LL_ALM_CPE_Ring_Cfg;
   pAlm->Metering_Cfg      = IFX_TAPI_LL_ALM_CPE_Metering_Cfg;
   pAlm->FaultLine_Restore = IFX_TAPI_LL_ALM_CPE_FaultLine_Restore;
   pAlm->TestLoop          = IFX_TAPI_LL_ALM_CPE_Test_Loop;
   pAlm->TestHookGen       = IFX_TAPI_LL_ALM_CPE_Test_HookGen;
}
