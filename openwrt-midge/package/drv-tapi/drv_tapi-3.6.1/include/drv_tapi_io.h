#ifndef DRV_TAPI_IO_H
#define DRV_TAPI_IO_H
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
   \file drv_tapi_io.h
   Contains TAPI I/O defines, enums and structures according to LTAPI
   specification.
   TAPI ioctl defines
      The file is divided in sections ioctl commands, constants, enumerations,
      structures for the following groups :
      - TAPI_INTERFACE_INIT
      - TAPI_INTERFACE_OP
      - TAPI_INTERFACE_METER
      - TAPI_INTERFACE_TONE
      - TAPI_INTERFACE_DIAL
      - TAPI_INTERFACE_SIGNAL
      - TAPI_INTERFACE_CID
      - TAPI_INTERFACE_CON
      - TAPI_INTERFACE_MISC
      - TAPI_INTERFACE_EVENT
      - TAPI_INTERFACE_RINGING
      - TAPI_INTERFACE_PCM
      - TAPI_INTERFACE_FAX
      - TAPI_INTERFACE_AUDIO
      - TAPI_INTERFACE_DECT
   \note Changes:
   IFX_TAPI_MAP_DATA_t nPhoneCh changed to nDstCh because it can also be a PCM
   and two more enum entries for chnl type are added for audio channel.
   IFX_TAPI_MAP_DATA_t nDataChType changed to nChType for destination type
   IFX_TAPI_MAP_PCM_ADD and REMOVE added
*/

#include "ifx_types.h"

#ifndef TAPI_VERSION3
#define TAPI_VERSION3
#endif

/** \defgroup TAPI_INTERFACE TAPI ioctl and Functions Reference
    This chapter describes the entire interfaces to the TAPI. The ioctl
    commands are explained by mentioning the return values for each function.
    The organization is as follows: */
/*@{*/

/** \defgroup TAPI_INTERFACE_AUDIO Audio Services
    Contains services for audio channel like enable/disable
    mode (handset/handsfree/headset/mute) volume set etc.*/

/** \defgroup TAPI_INTERFACE_CID CID Features Services
      Contains services for configuration, sending and receiving Caller ID. */

/** \defgroup TAPI_INTERFACE_CON Connection Control Services
    Contains all services used for RTP or AAL connection. It also contains
    services for conferencing.
    Applies to data channels unless otherwise stated.  */

/** \defgroup TAPI_INTERFACE_DECT DECT Services
    Contains services provided by the DECT channels */

/** \defgroup TAPI_INTERFACE_DIAL Dial Services
    Contains services for dialing. All dial services apply to phone channels
    unless otherwise stated. */

/** \defgroup TAPI_INTERFACE_EVENT Event Reporting Services
    Contains services for event reporting. This is applicable to device file
    descriptors unless otherwise stated. */

/** \defgroup TAPI_INTERFACE_FAX Fax T.38 Services
   The FAX services switch the corresponding data channel from voice to T.38
   data pump. All fax services apply to data channels unless otherwise stated.*/

/** \defgroup TAPI_INTERFACE_FXO FXO Services
    All FXO services apply to channel file descriptors unless otherwise stated.*/

/** \defgroup TAPI_INTERFACE_INIT Initialization Services
   This service sets the default initialization of device and hardware.*/

/** \defgroup TAPI_INTERFACE_METER Metering Services
    Contains services for metering.
    All metering services apply to phone channels unless otherwise stated. */

/** \defgroup TAPI_INTERFACE_MISC Miscellaneous Services
    Contains services for status and version information. This is applicable to
    phone channels unless otherwise stated.  */

/** \defgroup TAPI_INTERFACE_OP Operation Control Services
    Modifies the operation of the device.
    All operation control services apply to phone channels unless otherwise
    stated.  */

/** \defgroup TAPI_INTERFACE_PCM PCM Services
    Contains services for PCM configuration.
    Applies to phone channels unless
    otherwise stated. */

/** \defgroup TAPI_POLLING_SERVICE Polling Services
    All polling services apply to device file descriptors unless otherwise stated.*/

/** \defgroup TAPI_INTERFACE_RINGING Power Ringing Services
      Ringing on FXS interfaces. */

/** \defgroup TAPI_INTERFACE_SIGNAL Signal Detection Services
   The application handles the different states of the detection status. */

/** \defgroup TAPI_INTERFACE_TEST Testing Services
    Contains services for system tests like hook generation and loops. */

/** \defgroup TAPI_INTERFACE_TONE Tone Control Services
    All tone services apply to phone channels unless otherwise stated. */

/*@}*/

/* ========================================================================== */
/*                     TAPI Interface Ioctl Commands                          */
/* ========================================================================== */

/** Magic number for ioctls.*/
#define IFX_TAPI_IOC_MAGIC 'q'


/* ======================================================================== */
/* TAPI Initialization Services, ioctl commands (Group TAPI_INTERFACE_INIT) */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_INIT */
/*@{*/

/** This service sets the default initialization of the device and of the
      specific channel. It applies to channel file descriptors.

   \param IFX_TAPI_CH_INIT_t* The parameter points to a
    \ref IFX_TAPI_CH_INIT_t structure.

   \return No return value.

   \code
   IFX_TAPI_CH_INIT_t Init;
   Init.nMode = 0;
   Init.nCountry = 2;
   Init.pProc = &DevInitStruct;
   ioctl(fd, IFX_TAPI_CH_INIT, &Init);
   \endcode */
#define  IFX_TAPI_CH_INIT                          _IO(IFX_TAPI_IOC_MAGIC, 0x0F)

/*@}*/ /* TAPI_INTERFACE_INIT */

/* ========================================================================= */
/* TAPI Operation Control Services, ioctl commands (Group TAPI_INTERFACE_OP) */
/* ========================================================================= */
/** \addtogroup TAPI_INTERFACE_OP */
/*@{*/

/** This service sets the line feeding mode. The file descriptor is applicable
    to phone channel file descriptors containing an analog interface.

   \param IFX_TAPI_LINE_FEED_t* The parameter points to a
          \ref IFX_TAPI_LINE_FEED_t structure.

   \remarks For all battery switching modes the hardware must be able to
   support it for example by programming coefficients.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \code
   // set line mode to power down
   ioctl(fd, IFX_TAPI_LINE_FEED_SET, IFX_TAPI_LINE_FEED_DISABLED);
   \endcode
*/
#define  IFX_TAPI_LINE_FEED_SET                    _IO(IFX_TAPI_IOC_MAGIC, 0x01)

/** This service sets the line type (FXS, FXO) of analog channel. The file
    descriptor is applicable to phone channel file descriptors containing an
     analog interface.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_LINE_TYPE_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \code
   IFX_int32_t fd;

   // set line type to FXO
   ioctl(fd, IFX_TAPI_LINE_TYPE_SET, IFX_TAPI_LINE_TYPE_FXO);
   \endcode
*/
#define  IFX_TAPI_LINE_TYPE_SET              _IOW(IFX_TAPI_IOC_MAGIC, 0x47, int)

/** Sets the line echo canceller (LEC) configuration. The file descriptor is
    applicable to phone channel file descriptors.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_LEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks
   Attention: It is strongly recommended to use the new IFX_TAPI_WLEC_*
   interfaces. This interface is going to be discontinued in a next TAPI
   release.

   \code
   IFX_TAPI_LEC_CFG_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LEC_CFG_t));
   param.nType   = IFX_TAPI_LEC_TYPE_NLEC;
   param.nGainTx = IFX_TAPI_LEC_GAIN_MEDIUM;
   param.nGainRx = IFX_TAPI_LEC_GAIN_MEDIUM;
   param.nLen = 16; // 16ms
   param.bNlp = IFX_TAPI_LEC_NLP_DEFAULT;
   ret = ioctl(fd, IFX_TAPI_LEC_PHONE_CFG_SET, &param)
   \endcode
*/
#define  IFX_TAPI_LEC_PHONE_CFG_SET          _IOW(IFX_TAPI_IOC_MAGIC, 0x28, int)

/** Retrieves the LEC configuration. The file descriptor is applicable to phone
    channel file descriptors.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_LEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks
    Attention: It is strongly recommended to use the new IFX_TAPI_WLEC_*
    interfaces. This interface is going to be discontinued in a next TAPI
    release.
*/
#define  IFX_TAPI_LEC_PHONE_CFG_GET          _IOR(IFX_TAPI_IOC_MAGIC, 0x29, int)


/** Sets the line echo canceller (LEC) configuration for PCM. The file
    descriptor is applicable to phone channel file descriptors.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_LEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks
   Attention: It is strongly recommended to use the new IFX_TAPI_WLEC_*
   interfaces. This interface is going to be discontinued in a next TAPI
   release.

   \code
   IFX_TAPI_LEC_CFG_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LEC_CFG_t));
   param.nType   = IFX_TAPI_LEC_TYPE_NLEC;
   param.nGainTx = IFX_TAPI_LEC_GAIN_MEDIUM;
   param.nGainRx = IFX_TAPI_LEC_GAIN_MEDIUM;
   param.nLen = 16; // 16ms
   param.bNlp = IFX_TAPI_LEC_NLP_DEFAULT;
   ioctl(fd, IFX_TAPI_LEC_PCM_CFG_SET, &param)
   \endcode
*/
#define  IFX_TAPI_LEC_PCM_CFG_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0x50, int)

/** Retrieves the LEC configuration for PCM. The file descriptor is applicable
    to phone channel file descriptors.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_LEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks It is strongly recommended to use
    the new IFX_TAPI_WLEC_* interfaces. This interface is going to be
    discontinued in a next TAPI release.
*/
#define  IFX_TAPI_LEC_PCM_CFG_GET            _IOR(IFX_TAPI_IOC_MAGIC, 0x51, int)

/** Sets the line echo canceller (LEC) configuration. The file descriptor is
   applicable to phone channel file descriptors.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_WLEC_CFG_t structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_WLEC_CFG_t param;
   memset (&param, 0, sizeof(IFX_TAPI_WLEC_CFG_t));
   param.nType   = IFX_TAPI_LEC_TYPE_NLEC;
   param.bNlp = IFX_TAPI_LEC_NLP_DEFAULT;
   ret = ioctl(fd, IFX_TAPI_WLEC_PHONE_CFG_SET, &param)
   \endcode

   \note The ioctl IFX_TAPI_LEC_PHONE_CFG_SET is obsolete and has been replaced
    by this ioctl.

   \remarks WLEC mode is automatically mapped to NLEC mode when wideband is
            in use.
*/
#define  IFX_TAPI_WLEC_PHONE_CFG_SET         _IOW(IFX_TAPI_IOC_MAGIC, 0x30, int)

/** Retrieves the LEC configuration. The file descriptor is applicable to phone
    channel file descriptors.


   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_WLEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \note The ioctl IFX_TAPI_LEC_PHONE_CFG_GET is obsolete and has been replaced
    by this ioctl.

   \remarks WLEC mode is automatically mapped to NLEC mode when wideband
            is in use. However, this interface still reports the configured
            WLEC mode in this case.
*/
#define  IFX_TAPI_WLEC_PHONE_CFG_GET         _IOR(IFX_TAPI_IOC_MAGIC, 0x31, int)


/** Sets the line echo canceller (LEC) configuration for PCM. The file
    descriptor is applicable to phone channel file descriptors.


   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_WLEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \note The ioctl IFX_TAPI_LEC_PCM_CFG_SET is obsolete and has been replaced
    by this ioctl.

   \remarks Note: WLEC mode is automatically mapped to NLEC mode when wideband
            is in use.
*/
#define  IFX_TAPI_WLEC_PCM_CFG_SET           _IOW(IFX_TAPI_IOC_MAGIC, 0x32, int)

/** Sets the line echo canceller (LEC) configuration. The file descriptor is
   applicable to phone channel file descriptors.


   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_WLEC_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \note The ioctl IFX_TAPI_LEC_PCM_CFG_GET is obsolete and has been replaced
    by this ioctl.

   \remarks Note: WLEC mode is automatically mapped to NLEC mode when wideband
            is in use. However, this interface still reports the configured
            WLEC mode in this case.
*/
#define  IFX_TAPI_WLEC_PCM_CFG_GET           _IOR(IFX_TAPI_IOC_MAGIC, 0x33, int)

/** Specifies the time for hook, pulse digit and hook flash validation.
   The file descriptor is applicable to phone channel file descriptors
   containing an analog interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_LINE_HOOK_VT_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks
   The following conditions must be met:
      - IFX_TAPI_LINE_HOOK_VT_DIGITLOW_TIME
        min and max < IFX_TAPI_LINE_HOOK_VT_HOOKFLASH_TIME min and max
      - IFX_TAPI_LINE_HOOK_VT_HOOKFLASH_TIME
        min and max < IFX_TAPI_LINE_HOOK_VT_HOOKON_TIME min and max

   \code
   IFX_TAPI_LINE_HOOK_VT_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LINE_HOOK_VT_t));
   // set pulse dialing
   param.nType = IFX_TAPI_LINE_HOOK_VT_DIGITLOW_TIME;
   param.nMinTime = 40;
   param.nMaxTime = 60;
   ioctl(fd, IFX_TAPI_LINE_HOOK_VT_SET, (IFX_int32_t) &param);
   param.nType = IFX_TAPI_LINE_HOOK_VT_DIGITHIGH_TIME;
   param.nMinTime = 40;
   param.nMaxTime = 60;
   ioctl(fd, IFX_TAPI_LINE_HOOK_VT_SET, (IFX_int32_t) &param);
   \endcode */
#define  IFX_TAPI_LINE_HOOK_VT_SET           _IOW(IFX_TAPI_IOC_MAGIC, 0x2E, int)


/** Sets the speaker phone and microphone volume settings. The file descriptor
    is applicable to phone channel file descriptors containing an analog interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_LINE_VOLUME_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_LINE_VOLUME_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LINE_VOLUME_t));
   param.nTx = 24;
   param.nRx = 0;
   ioctl(fd, IFX_TAPI_PHONE_VOLUME_SET, &param);
   \endcode
*/
#define  IFX_TAPI_PHONE_VOLUME_SET           _IOW(IFX_TAPI_IOC_MAGIC, 0x42, int)


/** Sets the PCM interface volume settings. The file descriptor is aplicable to
   phone channel file descriptors containing a PCM interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_LINE_VOLUME_t structure.

   This interface expects a structure of type \ref IFX_TAPI_LINE_VOLUME_t.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_LINE_VOLUME_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LINE_VOLUME_t));
   param.nTx = 24;
   param.nRx = 0;
   ioctl(fd, IFX_TAPI_PCM_VOLUME_SET, &param);
   \endcode
*/
#define  IFX_TAPI_PCM_VOLUME_SET             _IOW(IFX_TAPI_IOC_MAGIC, 0x45, int)

/** This service enables or disables a high level path of a phone channel.
   The high level path might be required to play howler tones. The file
   descriptor is applicable to phone channel file descriptors containing an
   analog interface.

   \param IFX_int32_t The parameter represent a boolean value of
          \ref IFX_TAPI_LINE_LEVEL_t.

    \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks This service is intended for phone channels only and must be used
   in combination with \ref IFX_TAPI_PHONE_VOLUME_SET or
   \ref IFX_TAPI_PCM_VOLUME_SET to set the max. level
   (\ref IFX_TAPI_LINE_VOLUME_HIGH) or to restore level
    Only the order of calls with the parameters
    IFX_TAPI_LINE_LEVEL_ENABLE and then IFX_TAPI_LINE_LEVEL_DISABLE
    is supported.

   \code
   IFX_int32_t fd;

   ioctl(fd, IFX_TAPI_LINE_LEVEL_SET, IFX_TAPI_LINE_LEVEL_ENABLE);
   // play out some high level tones or samples
   ioctl(fd, IFX_TAPI_LINE_LEVEL_SET, IFX_TAPI_LINE_LEVEL_DISABLE);
   \endcode
*/
#define  IFX_TAPI_LINE_LEVEL_SET             _IOW(IFX_TAPI_IOC_MAGIC, 0x46, int)

/**  This service reads the hook status from the driver. The file descriptor is
    applicable to phone channel file descriptors containing an analog interface.

   \param IFX_int32_t The parameter is a pointer to the status.
   - 0: no hook detected
   - 1: hook detected

       \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \note "off hook" corresponds to "ground start" depending on line feed mode.

   \code
   IFX_int32_t fd;
   IFX_int32_t nHook;

   ioctl(fd, IFX_TAPI_LINE_HOOK_STATUS_GET, &nHook);
   switch(nHook)
   {
      case 0:
      // on hook
      break;

      case 1:
      // off hook
      break;

      default:
      // unknown state
      break;
   \endcode
*/
#define  IFX_TAPI_LINE_HOOK_STATUS_GET            _IO (IFX_TAPI_IOC_MAGIC, 0x84)

/*@}*/ /* TAPI_INTERFACE_OP */

/* =================================================================== */
/* TAPI Metering Services, ioctl commands (Group TAPI_INTERFACE_METER) */
/* =================================================================== */
/** \addtogroup TAPI_INTERFACE_METER */
/*@{*/

/** This service sets the characteristics for the metering service. The file
    descriptor is applicable to phone channel file descriptors.

   \param IFX_TAPI_METER_CFG_t* The parameter points to a
   \ref IFX_TAPI_METER_CFG_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks If burst_cnt is not zero the burst number is defined by burst_cnt
    and stopping is not necessary. If burst_cnt is set to zero the bursts are
    sent out until a \ref IFX_TAPI_METER_STOP ioctl is called.

   \code
   IFX_TAPI_METER_CFG_t Metering;
   memset (&Metering, 0, sizeof (IFX_TAPI_METER_CFG_t));
   // Metering mode is already set to 0
   // 100ms burst length
   Metering.burst_len = 100;
   // 2 min. burst distance
   Metering.burst_dist = 120;
   // send out 2 bursts
   Metering.burst_cnt = 2;
   // set metering characteristic
   ioctl(fd, IFX_TAPI_METER_CFG_SET, &Metering);
   \endcode   */
#define  IFX_TAPI_METER_CFG_SET                    _IO(IFX_TAPI_IOC_MAGIC, 0x0C)

/** This service starts the metering. The file descriptor is applicable to phone
    channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Before this service can be used, the metering characteristic must
    be set (\ref IFX_TAPI_METER_CFG_SET) and the line mode must be set
    to normal (\ref IFX_TAPI_LINE_FEED_SET).

   \code
   ioctl(fd, IFX_TAPI_METER_START, 0);
   \endcode   */
#define  IFX_TAPI_METER_START                      _IO(IFX_TAPI_IOC_MAGIC, 0x0D)

/** This service stops the metering. The file descriptor is applicable to phone
   channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks If the metering has not been started before this service returns
    an error.

   \code
   ioctl(fd, IFX_TAPI_METER_STOP, 0);
   \endcode   */
#define  IFX_TAPI_METER_STOP                       _IO(IFX_TAPI_IOC_MAGIC, 0x0E)

/*@}*/ /* TAPI_INTERFACE_METER */

/* ======================================================================= */
/* TAPI Tone Control Services, ioctl commands (Group TAPI_INTERFACE_TONE)  */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_TONE */
/*@{*/

/**
   This service sets the tone level of the tone currently played.
   \param IFX_TAPI_PREDEF_TONE_LEVEL_t* The parameter points to a
   \ref IFX_TAPI_PREDEF_TONE_LEVEL_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PREDEF_TONE_LEVEL_t Tone;
   memset (&Tone, 0, sizeof(IFX_TAPI_PREDEF_TONE_LEVEL_t));
   Tone.generator = 0xFF;
   // all tone generators
   Tone.level = 0x100;
   // -38.97 dB level
   // playlevel in dB can be calculated via formula:
   // playlevel_in_dB = +3.17 + 20 log10 (nPlayLevel/32767)
   ioctl(fd, IFX_TAPI_TONE_LEVEL_SET, &Tone);
   \endcode
*/
#define  IFX_TAPI_TONE_LEVEL_SET                   _IO(IFX_TAPI_IOC_MAGIC, 0x08)

/** Configures a tone based on simple or composed tones. The tone is also added
    to the tone table. The file descriptor is applicable to channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_TONE_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   A simple tone specifies a tone sequence composed of several single frequency
   tones or dual frequency tones. The sequence can be transmitted only one
   time, several times or until the transmission is stopped by client.
   This interface can add a simple tone to the internal table with maximum
   222 entries, starting from IFX_TAPI_TONE_INDEX_MIN (32).
   At least on cadence must be defined otherwise this interface returns an
   error. The tone table provides all tone frequencies in 5 Hz steps with a 5%
   tolerance and are defined in RFC 2833.
   For composed tones the loopcount of each simple tone must be different from 0.

   \code

   IFX_TAPI_CH_INIT_t tapi;
   IFX_TAPI_TONE_t tone;
   IFX_int32_t fd;

   //Open channel file descriptor for channel 0
   fd = open("/dev/vin11", O_RDWR, 0x644);

   memset (&tapi,0, sizeof(IFX_TAPI_CH_INIT_t));
   memset (&tone,0, sizeof(IFX_TAPI_TONE_t));
   ioctl(fd, IFX_TAPI_CH_INIT, (IFX_int32_t) &tapi);
   tone.simple.format = IFX_TAPI_TONE_TYPE_SIMPLE;
   tone.simple.index = 71;
   tone.simple.freqA = 480;
   tone.simple.freqB = 620;
   tone.simple.levelA = -300;
   tone.simple.cadence[0] = 2000;
   tone.simple.cadence[1] = 2000;
   tone.simple.frequencies[0] = IFX_TAPI_TONE_FREQA | IFX_TAPI_TONE_FREQB;
   tone.simple.loop = 2;
   tone.simple.pause = 200;
   ioctl(fd, IFX_TAPI_TONE_TABLE_CFG_SET, (IFX_int32_t) &tone);
   memset (&tone,0, sizeof(IFX_TAPI_TONE_t));
   tone.composed.format = IFX_TAPI_TONE_TYPE_COMPOSED;
   tone.composed.index = 100;
   tone.composed.count = 2;
   tone.composed.tones[0] = 71;
   tone.composed.tones[1] = 71;
   ioctl(fd, IFX_TAPI_TONE_TABLE_CFG_SET, (IFX_int32_t) &tone);
   //Now start playing the simple tone
   ioctl(fd, IFX_TAPI_TONE_LOCAL_PLAY, (IFX_int32_t) 71);
   //Stop playing tone
   ioctl(fd, IFX_TAPI_TONE_STOP, (IFX_int32_t) 71);

   //Close all open fds
   close(fd);

   \endcode
*/
#define  IFX_TAPI_TONE_TABLE_CFG_SET         _IOW(IFX_TAPI_IOC_MAGIC, 0x36, int)

   /** Starts/stops generation of a tone towards local port, the parameter
   (if greater than 0) gives the tone table index of the tone to be played.
   If the parameter is equal to zero, the current tone generation stops.
    The file descriptor is applicable to channel descriptors.

   \param IFX_int32_t The parameter is the index of the tone to the
   predefined tone table (range 1 - 31) or custom tones added previously
   (index 32 - 255).Index 0 means tone stop. Using the upper bits modify the
   default tone playing source.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   This can be a predefined, a simple or a composed tone. The tone codes are
   assigned previously upon system start.
   Index 1 - 31 is predefined by the driver and covers the original TAPI.
   A timing must be specified with \ref IFX_TAPI_TONE_ON_TIME_SET and
   \ref IFX_TAPI_TONE_OFF_TIME_SET before.

   \code
   IFX_TAPI_LINE_VOLUME_t param;
   IFX_int32_t fd;

   // play tone index 34
   ioctl(fd, IFX_TAPI_TONE_LOCAL_PLAY, 34);
   \endcode   */
#define  IFX_TAPI_TONE_LOCAL_PLAY          _IOW (IFX_TAPI_IOC_MAGIC, 0x9B, char)

   /** Starts/stops generation of a tone towards network, the parameter
   (if greater than 0) gives the tone table index of the tone to be played.
   If the parameter is equal to zero, current tone generation stops. The file
   descriptor is applicable to channel descriptors.

   \param IFX_int32_t The parameter is the index of the tone to the predefined
   tone table (range 1 - 31) or custom tones added previously
   (index 32 - 255). Index 0 means tone stop. Using the upper bits modify the
   default tone playing source.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   This can be a predefined, a simple or a composed tone. The tone codes are
   assigned previously upon system start.
   Index 1 - 31 is predefined by the driver and covers the original TAPI.
   A timing must be specified with \ref IFX_TAPI_TONE_ON_TIME_SET and
    \ref IFX_TAPI_TONE_OFF_TIME_SET before.
   All other indices can be custom defined by \ref IFX_TAPI_TONE_TABLE_CFG_SET.

   \code
   // play tone index 34
   ioctl(fd, IFX_TAPI_TONE_NET_PLAY, 34);
   \endcode   */
#define  IFX_TAPI_TONE_NET_PLAY            _IOW (IFX_TAPI_IOC_MAGIC, 0xC5, char)

/**
   This service sets the on-time for a tone with a specified duration.
   After setting tone on/off time, the tone is started with service
   \ref IFX_TAPI_TONE_LOCAL_PLAY.

   \param int The parameter is equal to the duration of the tone in
   0.25 ms steps. The maximum value is 65535.

   - 0: not valid (default)
   - 1: 0.25 ms
   - 2: 0.5 ms
   - ...
   - 65535: 16383.75 ms

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This interface must be called before using tone services.

   \code
   int nDuration = 0x190;
   // set tone on-time with a 100 ms duration
   ioctl(fd, IFX_TAPI_TONE_ON_TIME_SET, nDuration);
   \endcode   */
#define  IFX_TAPI_TONE_ON_TIME_SET          _IOW (IFX_TAPI_IOC_MAGIC, 0x9C, int)

/**
   This service sets the off-time for a tone with a specified duration.
   After setting tone on/off time, the tone is started with service
   \ref IFX_TAPI_TONE_LOCAL_PLAY.

   \param int The parameter is equal to the duration of the tone in
   0.25 ms steps.

   - 0: not valid (default)
   - 1: 0.25 ms
   - 2: 0.5 ms
   - ...
   - 65535: 16383.75 ms

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nDuration = 0x190;
   // set tone off-time with a 100 ms duration
   ioctl(fd, IFX_TAPI_TONE_OFF_TIME_SET, nDuration);
   \endcode   */
#define  IFX_TAPI_TONE_OFF_TIME_SET         _IOW (IFX_TAPI_IOC_MAGIC, 0x9D, int)

/**
   This service gets the current on-time duration which was set by a
   former \ref IFX_TAPI_TONE_ON_TIME_SET

   \param int The parameter points to the  duration of the tone
   in 0.25 ms steps.

   - 0: not valid (default)
   - 1: 0.25 ms
   - 2: 0.5 ms
   - ...
   - 65535: 16383.75 ms

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nDuration, ret;
   // get on-time duration
   ret = ioctl(fd, IFX_TAPI_TONE_ON_TIME_GET, &nDuration);
   \endcode   */
#define  IFX_TAPI_TONE_ON_TIME_GET                _IO (IFX_TAPI_IOC_MAGIC, 0x9E)

/**
   This service gets the current off-time duration which was set by a
   former \ref IFX_TAPI_TONE_OFF_TIME_SET

   \param int The parameter is equal to the duration of the tone in
   0.25 ms steps. The maximum value is 65535.

   - 0: not valid (default)
   - 1: 0.25 ms
   - 2: 0.5 ms
   - ...
   - 65535: 16383.75 ms

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nDuration, ret;
   // get on-time duration
   ret = ioctl(fd, IFX_TAPI_TONE_OFF_TIME_GET, &nDuration);
   \endcode   */
#define  IFX_TAPI_TONE_OFF_TIME_GET               _IO (IFX_TAPI_IOC_MAGIC, 0x9F)

/** This service retrieves the tone playing state. The file descriptor is
    applicable to data channel file descriptors.

   \param IFX_int32_t* The parameter points to the tone state
   - 0: no tone is played
   - 1: tone is played (tone is within on-time)
   - 2: silence (tone is within off-time)

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nToneState;
   // get the tone state
   ret = ioctl(fd, IFX_TAPI_TONE_STATUS_GET, &nToneState);
   \endcode   */
#define  IFX_TAPI_TONE_STATUS_GET                 _IO (IFX_TAPI_IOC_MAGIC, 0xA0)

/** Pre-defined tone services for busy tone.

   \param int This interface expects no parameter. It should be set to 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks These tones are defined by frequencies inside the TAPI for USA.

   \code
   ioctl(fd, IFX_TAPI_TONE_BUSY_PLAY, 0);
   \endcode   */
#define  IFX_TAPI_TONE_BUSY_PLAY                  _IO (IFX_TAPI_IOC_MAGIC, 0xA1)

/** Pre-defined tone services for ring back tone.

   \param int This interface expects no parameter. It should be set to 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks These tones are defined by frequencies inside the TAPI for USA.

   \code
   ioctl(fd, IFX_TAPI_TONE_RINGBACK_PLAY, 0);
   \endcode   */
#define  IFX_TAPI_TONE_RINGBACK_PLAY              _IO (IFX_TAPI_IOC_MAGIC, 0xA2)

/**
   Pre-defined tone services for dial tone.

   \param int This interface expects no parameter. It should be set to 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks These tones are defined by frequencies inside the TAPI for USA.

   \code
   ioctl(fd, IFX_TAPI_TONE_BUSY_PLAY, 0);
   \endcode   */
#define  IFX_TAPI_TONE_DIALTONE_PLAY              _IO (IFX_TAPI_IOC_MAGIC, 0xA3)

/** Stops playback of a specified tone.

   \param int Index of the tone to be stoppped.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Stopping all tones played into this direction can be done with
   index 0. Passing index 0 to \ref IFX_TAPI_TONE_LOCAL_PLAY has the same effect.

   \code
   // stop playing tone index 34 in local direction
   ioctl(fd, IFX_TAPI_TONE_LOCAL_STOP, 34);
   \endcode   */
#define  IFX_TAPI_TONE_LOCAL_STOP          _IOW (IFX_TAPI_IOC_MAGIC, 0xAB, char)

/** Stops playback of a specified tone.

   \param int Index of the tone to be stoppped.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Stopping all tones played into this direction can be done with
   index 0. Passing index 0 to \ref IFX_TAPI_TONE_NET_PLAY has the same effect.

   \code
   // stop playing tone index 34 in network direction
   ioctl(fd, IFX_TAPI_TONE_NET_STOP, 34);
   \endcode   */
#define  IFX_TAPI_TONE_NET_STOP            _IOW (IFX_TAPI_IOC_MAGIC, 0xAC, char)

/** Stops tone generation. The file descriptor is applicable to data channel
    file descriptors.

   \param IFX_int32_t Tone table index of the tone to be stopped or index = 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Giving an index of 0 will stop playback of all tones in all
   directions. Stopping tones can also be done with
	\ref IFX_TAPI_TONE_LOCAL_PLAY or IFX_TAPI_TONE_NET_PLAY and index 0

   \code
   // stop playing tone index 34
   ioctl(fd, IFX_TAPI_TONE_STOP, 34);
   \endcode   */
#define  IFX_TAPI_TONE_STOP                       _IO (IFX_TAPI_IOC_MAGIC, 0xA4)

/*@}*/ /* TAPI_INTERFACE_TONE */

/* ======================================================================= */
/* TAPI Dial Services, ioctl commands (Group TAPI_INTERFACE_DIAL)          */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_DIAL */
/*@{*/

/** This service controls the DTMF sending mode of out-of-band (OOB)
    information: RFC 2833 packets. The File descriptor is applicable to data
     channel file descriptors.

   \param int The parameter must be selected from \ref IFX_TAPI_PKT_EV_OOB_t
    enumerator.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   // Set the DTMF sending mode to out of band
   ioctl(fd, IFX_TAPI_PKT_EV_OOB_SET, IFX_TAPI_PKT_EV_OOB_ONLY);
   \endcode   */
#define  IFX_TAPI_PKT_EV_OOB_SET             _IOW (IFX_TAPI_IOC_MAGIC, 0x99,int)

/*@}*/ /* TAPI_INTERFACE_DIAL */

/* ======================================================================= */
/* TAPI Signal Detection Services, ioctl commands                          */
/* (Group TAPI_INTERFACE_SIGNAL)                                           */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_SIGNAL */
/*@{*/

/** Enables the signal detection for fax/modem signals. The file descriptor
    is applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_SIG_DETECTION_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks For firmware providing ATD/UTD please check the capability list for
    available detection services and detectors amount.
   Signal events are reported via exception. The information is queried with
   \ref IFX_TAPI_EVENT_GET. If AM is detected the driver may automatically switch
	to modem coefficients after the ending of CED. Not every combination is possible.
 	It depends on the underlying device and firmware. For firmware providing MFTD
	there are no limitations in the combination of signals that can be detected.
	When reprogramming the detectors the driver has to stop the detector for a short moment.

   \code
   IFX_TAPI_SIG_DETECTION_t detect;
   IFX_TAPI_CH_STATUS_t param;
   memset (&param, 0, sizeof(IFX_TAPI_CH_STATUS_t));
   memset (&detect, 0, sizeof(IFX_TAPI_SIG_DETECTION_t));
   detect.sig = IFX_TAPI_SIG_CEDTX | IFX_TAPI_SIG_PHASEREVRX;
   ioctl (fd, IFX_TAPI_SIG_DETECT_ENABLE, &detect);
   \endcode  */
#define  IFX_TAPI_SIG_DETECT_ENABLE          _IOW(IFX_TAPI_IOC_MAGIC, 0x40, int)

/** Disables the signal detection for Fax or modem signals. The file descriptor
    is applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_SIG_DETECTION_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

\note It is strongly recommended to use the new event reporting interfaces.
      This interface is going to be discontinued in a next TAPI release.
*/

#define  IFX_TAPI_SIG_DETECT_DISABLE         _IOW(IFX_TAPI_IOC_MAGIC, 0x41, int)

/** This service is used to set DTMF Receiver coefficients. The file descriptor
    is applicable to data channel file descriptors.

   \param IFX_int32_t The parameter points to a
    \ref IFX_TAPI_DTMF_RX_CFG_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks If enabled, the DTMF receiver will be temporarily disabled during
    the writing of the coefficients.

   \code
   IFX_TAPI_DTMF_RX_CFG_t DtmfCoef;

   DtmfCoef.nLevel   = -56;   // dB
   DtmfCoef.nTwist   = 9;     // dB
   DtmfCoef.nGain    = 0;     // dB

   ioctl(fd, IFX_TAPI_DTMF_RX_CFG_SET, (int)&DtmfCoef);
   \endcode
*/
#define  IFX_TAPI_DTMF_RX_CFG_SET           _IOW(IFX_TAPI_IOC_MAGIC, 0x34, int)

/** Retrieves DTMF receiver coefficients.

   \param IFX_TAPI_DTMF_RX_CFG_t* The parameter points to a
          \ref IFX_TAPI_DTMF_RX_CFG_t structure.

   \code
   IFX_TAPI_DTMF_RX_CFG_t DtmfCoef;

   ioctl(fd, IFX_TAPI_DTMF_RX_CFG_SET, (int)&DtmfCoef);
   \endcode

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_DTMF_RX_CFG_GET            _IOR(IFX_TAPI_IOC_MAGIC, 0x35, int)

/** Starts the call progress tone detection based on a previously defined
   simple tone. The file descriptor is applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_TONE_CPTD_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_TONE_CPTD_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_TONE_CPTD_t));
   param.tone   = 74;
   param.signal = IFX_TAPI_TONE_CPTD_DIRECTION_TX;
   ioctl (fd, IFX_TAPI_TONE_CPTD_START, &param);
   \endcode   */
#define  IFX_TAPI_TONE_CPTD_START            _IOW(IFX_TAPI_IOC_MAGIC, 0xC3, int)

/** Stops the call progress tone detection. The file descriptor is applicable
    to data channel descriptors.

   \param IFX_int32_t This interface expects no parameter. It should be set to 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
   */
#define  IFX_TAPI_TONE_CPTD_STOP             _IOW(IFX_TAPI_IOC_MAGIC, 0xC4, int)

/** This service is used to configure fax/modem detectors.

   \param IFX_int32_t The parameter points to a
    \ref IFX_TAPI_MFTD_CFG_t structure.

      \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks If enabled, the fax/modem detectors will be temporarily disabled during
    the writing of the coefficients.

   \code
   IFX_TAPI_MFTD_CFG_t FaxModem;

   FaxModem.nHoldMinLevel = -32;  // dB
   FaxModem.nHoldGapTime  = 120;  // ms

   ioctl(fd, IFX_TAPI_MFTD_CFG_SET, (int)&FaxModem);
   \endcode
*/

#define  IFX_TAPI_MFTD_CFG_SET             _IOW(IFX_TAPI_IOC_MAGIC, 0xXX, int)

/*@}*/ /* TAPI_INTERFACE_SIGNAL */

/* ======================================================================= */
/* TAPI CID Features Service, ioctl commands  (Group TAPI_INTERFACE_CID)   */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_CID */
/*@{*/

/** Sets up the CID receiver to start receiving CID Data. The file descriptor is
    applicable to data channel file descriptors.

   \param IFX_TAPI_CID_HOOK_MODE_t  Hook mode for the reception to be choosen
   from IFX_TAPI_CID_HOOK_MODE_t enumerator.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This command must be sent so that the driver can start collecting
    CID Data.

   \code
   ioctl(fd, IFX_TAPI_CID_RX_START, (IFX_int32_t) IFX_TAPI_CID_HM_ONHOOK);
   \endcode
*/

#define  IFX_TAPI_CID_RX_START                     _IO(IFX_TAPI_IOC_MAGIC, 0x3A)

/** Stops the CID receiver. The file descriptor is applicable to data channel
    file descriptors.
   \param int This interface expects no parameter. It should be set to 0.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_CID_RX_STOP, 0);
   \endcode   */

#define  IFX_TAPI_CID_RX_STOP                      _IO(IFX_TAPI_IOC_MAGIC, 0x3B)

/** Retrieves the current status information of the CID receiver. The file
    descriptor is applicable to data channel file descriptors.

   \param IFX_TAPI_CID_RX_STATUS_t* The parameter points to a
   \ref IFX_TAPI_CID_RX_STATUS_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Once the CID receiver is activated it signals the status with an
   event. The event is raised if data has been received completely or
   an error occurred. Afterwards this interface is used to determine if data
   has been received. In that case it can be read with the interface
   \ref IFX_TAPI_CID_RX_DATA_GET. This interface can also be used without
   exception to determine the status of the receiver during reception.

   \code
   IFX_TAPI_CID_RX_STATUS_t cidStatus;
   IFX_int32_t fd;
   IFX_return_t ret;
   memset(&cidStatus, 0, sizeof (IFX_TAPI_CID_RX_STATUS_t));
   ret = ioctl(fd, IFX_TAPI_CID_RX_STATUS_GET, (IFX_int32_t) &cidStatus);
   // Check if cid information are available for reading
   if ((IFX_SUCCESS == ret)
   && (IFX_TAPI_CID_RX_ERROR_NONE == cidStatus.nError)
   && (IFX_TAPI_CID_RX_STATE_DATA_READY == cidStatus.nStatus))
   {
   printf ( "Data are ready for reading\n\r" );
   }
   \endcode
*/

#define  IFX_TAPI_CID_RX_STATUS_GET          _IOR(IFX_TAPI_IOC_MAGIC, 0x3C, int)

/** Reads CID Data collected by the Caller ID receiver. The file descriptor is
    applicable to data channel file descriptors.

   \param IFX_TAPI_CID_RX_DATA_t* The parameter points to a
   \ref IFX_TAPI_CID_RX_DATA_t structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks This request can be sent after the CID receiver status signaled
    that data is ready for reading or that data collection is ongoing. In the
    last case, the number of data read correspond to the number of data
    received since CID receiver was started. Once the data is read after the
    status signaled that data is ready, new data can be read only if CID
    receiver detects new data.

   \code
   IFX_TAPI_CID_RX_STATUS_t cidStatus;
   IFX_TAPI_CID_RX_DATA_t cidData;
   IFX_int32_t fd;
   IFX_return_t ret;
   // Open channel file descriptor for channel 0
   fd = open("/dev/vin11", O_RDWR, 0x644);
   memset (&cidStatus, 0, sizeof (IFX_TAPI_CID_RX_STATUS_t));
   memset (&cidData, 0, sizeof (IFX_TAPI_CID_RX_DATA_t));
   // Read actual status
   ret = ioctl(fd, IFX_TAPI_CID_RX_STATUS_GET, ( IFX_int32_t )&cidStatus);
   // Check if cid data are available for reading
   if ((IFX_SUCCESS == ret)
   && (IFX_TAPI_CID_RX_ERROR_NONE == cidStatus.nError)
   && (IFX_TAPI_CID_RX_STATE_DATA_READY == cidStatus.nStatus))
   {
   ret = ioctl(fd, IFX_TAPI_CID_RX_DATA_GET, (IFX_int32_t) &cidData);
   }
   // Close all open fds
   close(fd);
   \endcode
*/

#define  IFX_TAPI_CID_RX_DATA_GET            _IOR(IFX_TAPI_IOC_MAGIC, 0x3D, int)

/** This interface transmits CID message. The file descriptor is applicable to
    data channel file descriptors.

   \param  IFX_TAPI_CID_MSG_t*  Pointer to a \ref IFX_TAPI_CID_MSG_t,
   containing the CID / MWI information to be transmitted.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Before issuing this service, the CID engine must be configured with
   IFX_TAPI_CID_CFG_SET at least one time after boot. This is required to
   configure country specific settings.

   \code
   IFX_TAPI_CID_MSG_t Cid_Info;
   IFX_TAPI_CID_MSG_ELEMENT_t Msg_El[2];
   IFX_int32_t fd;
   IFX_return_t ret;
   IFX_char_t* number = "12345";
   // Open channel file descriptor for channel 0
   fd = open("/dev/vin11", O_RDWR, 0x644);
   // Reset and fill the caller id structure
   memset(&Cid_Info, 0, sizeof(Cid_Info));
   memset(&Msg_El, 0, sizeof(Msg_El));
   Cid_Info.txMode = IFX_TAPI_CID_HM_ONHOOK;
   // Message Waiting
   d_Info.messageType = IFX_TAPI_CID_MT_MWI;
   Cid_Info.nMsgElements = 2;
   Cid_Info.message = Msg_El;
   // Mandatory for Message Waiting: set Visual Indicator on
   Msg_El[0].value.elementType = IFX_TAPI_CID_ST_VISINDIC;
   Msg_El[0].value.element = IFX_TAPI_CID_VMWI_EN;
   // Add optional CLI (number) element
   Msg_El[1].string.elementType = IFX_TAPI_CID_ST_CLI;
   Msg_El[1].string.len = ret;
   strncpy(Msg_El[1].string.element, number, sizeof(Msg_El[1].string.element));
   // Transmit the caller id
   ioctl(fd, IFX_TAPI_CID_TX_INFO_START, &Cid_Info);
   // close all open fds
   close(fd);
   \endcode
*/

#define  IFX_TAPI_CID_TX_INFO_START         _IOW (IFX_TAPI_IOC_MAGIC, 0xB1, int)

/** This service stops an ongoing transmission of a CID message that was
   started with \ref IFX_TAPI_CID_TX_INFO_START.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ret = ioctl(fd, IFX_TAPI_CID_TX_INFO_STOP, 0);
   \endcode
  */
#define  IFX_TAPI_CID_TX_INFO_STOP          _IOW (IFX_TAPI_IOC_MAGIC, 0xB5, int)

/** This service starts a pre-programmed CID sequence driven by TAPI. This is a
   non-blocking service, the driver will signal the end of the CID sequence with an event.
   Before issuing this service, CID engine must be configured with
   IFX_TAPI_CID_CFG_SET at least one time after boot. This is required to
   configure country specific settings.

   \param  IFX_TAPI_CID_MSG_t*  Pointer to a \ref IFX_TAPI_CID_MSG_t,
   defining the CID/MWI type and containing the information to be transmitted.

   \remarks For FSK transmission, decision of seizure and mark length is based
    on configured standard and CID transmission type.
*/
#define  IFX_TAPI_CID_TX_SEQ_START          _IOW (IFX_TAPI_IOC_MAGIC, 0xB2, int)

/** Configures the CID transmitter. The file descriptor is applicable to data
    channel file descriptors.

   \param  IFX_TAPI_CID_CFG_t*  Pointer to a \ref IFX_TAPI_CID_CFG_t,
   containing CID / MWI configuration information.

     \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

  \remarks The delay must be programmed so that the CID data would still fit
   between the ring burst in case of appearancemode 1 and 2, otherwise the
   ioctl \ref IFX_TAPI_RING_START may return an error. CID is stopped when the
   ringing is stopped or the phone goes off-hook.

   \code
      IFX_TAPI_CID_CFG_t param;
      IFX_int32_t fd;
      memset(&param, 0, sizeof(IFX_TAPI_CID_CFG_t));
      // Set CID standard to Telcordia/Bellcore default values
      param.nStandard = IFX_TAPI_CID_STD_TELCORDIA;
      ioctl(fd, IFX_TAPI_CID_CFG_SET, &param);
   \endcode
*/

#define  IFX_TAPI_CID_CFG_SET               _IOW (IFX_TAPI_IOC_MAGIC, 0xB0, int)

/*@}*/ /* TAPI_INTERFACE_CID */

/* ======================================================================= */
/* TAPI Connection Services, ioctl commands (Group TAPI_INTERFACE_CON)     */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_CON */
/*@{*/

/** This interface configures AAL fields for a new connection. The file
    descriptor is applicable to phone channel file descriptors containing an
    analog interface.

   \param IFX_TAPI_PCK_AAL_CFG_t* The parameter points to a
   \ref IFX_TAPI_PCK_AAL_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PKT_AAL_CFG_SET param;
   memset (&param, 0, sizeof (IFX_TAPI_PKT_AAL_CFG_SET));
   param.nTimestamp = 0x1234;
   ret = ioctl(fd, IFX_TAPI_PKT_AAL_CFG_SET, &param)
   \endcode   */
#define  IFX_TAPI_PKT_AAL_CFG_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0x1E, int)

/** AAL profile configuration. The file descriptor is applicable to phone channel
   file descriptors containing an analog interface.

   \param IFX_TAPI_PCK_AAL_PROFILE_t* The parameter points to a
   \ref IFX_TAPI_PCK_AAL_PROFILE_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
    Sets each row in a profile. A maximum number of 10 rows can be set up. This
    interface is called on initialization to program the selected AAL profile.
    The following example programs the ATMF profiles 3, 4 and 5.

   \code
   IFX_TAPI_PCK_AAL_PROFILE_t Profile;
   switch (nProfile)
   {
      case  3:
      case  4:
      // ATMF profile 3 and 4
      Profile.rows = 1;
      Profile.codec[0] = IFX_TAPI_COD_TYPE_MLAW;
      Profile.len[0] = 43;
      Profile.nUUI[0] = IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_7;
      break;
      case  5:
      // ATMF profile 5
      Profile.rows = 2;
      Profile.codec[0] = IFX_TAPI_COD_TYPE_MLAW;
      Profile.len[0] = 43;
      Profile.nUUI[0] = IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_7;
      Profile.codec[1] = IFX_TAPI_COD_TYPE_G726_32;
      Profile.len[1] = 43;
      Profile.nUUI[1] = IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_15;
      break;
   }
   ret = ioctl (fd, IFX_TAPI_PKT_AAL_PROFILE_SET, (INT)&Profile);
   \endcode */
#define  IFX_TAPI_PKT_AAL_PROFILE_SET        _IOW(IFX_TAPI_IOC_MAGIC, 0x23, int)

/** This interface configures RTP and RTCP fields for a new connection. The file
    descriptor is applicable to data channel file descriptors.

   \param IFX_TAPI_PKT_RTP_CFG_t* The parameter points to a
   \ref IFX_TAPI_PKT_RTP_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PKT_RTP_CFG_t param;
   IFX_int32_t fd;
   memset (&param, 0, sizeof(IFX_TAPI_PKT_RTP_CFG_t));
   param.nSeqNr = 0x1234;
   ioctl(fd, IFX_TAPI_PKT_RTP_CFG_SET, &param);
   \endcode   */
#define  IFX_TAPI_PKT_RTP_CFG_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0x13, int)

/** Configures the payload type table. The file descriptor is applicable to data
   channel file descriptors.

   \param IFX_TAPI_PKT_RTP_PT_CFG_t* The parameter points to a
   \ref IFX_TAPI_PKT_RTP_PT_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   The requested payload type should have been negotiated with the peer prior
   to the connection set-up. Event payload types are negotiated during
   signaling phase and the driver and the device can be programmed accordingly
   at the time of starting the media session. Event payload types shall not
   be modified when the session is in progress.

   \code
   IFX_TAPI_PKT_RTP_PT_CFG_t param;
IFX_int32_t fd;
memset(&param, 0, sizeof(IFX_TAPI_PKT_RTP_PT_CFG_t));
param.nPTup[6D] = 0x5;
param.nPTdown[12D] = 0x4;
ioctl(fd, IFX_TAPI_PKT_RTP_PT_CFG_SET, &param);
   \endcode   */
#define  IFX_TAPI_PKT_RTP_PT_CFG_SET         _IOW(IFX_TAPI_IOC_MAGIC, 0x14, int)

/** Retrieves RTCP statistics. The file descriptor is applicable to data
   channel file descriptors.

   \param IFX_TAPI_PKT_RTCP_STATISTICS_t* Pointer to a
   \ref IFX_TAPI_PKT_RTCP_STATISTICS_t structure according to RFC 3550/3551
    for a sender report.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Not all statistics may be supported by the Infineon device.

   \code
   IFX_TAPI_PKT_RTCP_STATISTICS_t param;
   IFX_int32_t fd;
   ioctl(fd, IFX_TAPI_PKT_RTCP_STATISTICS_GET, (IFX_int32_t) &param);
   \endcode   */
#define  IFX_TAPI_PKT_RTCP_STATISTICS_GET    _IOR(IFX_TAPI_IOC_MAGIC, 0x15, int)

/** Resets the RTCP statistics. The file descriptor is applicable to data
   channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_PKT_RTCP_STATISTICS_RESET, 0);
   \endcode   */
#define  IFX_TAPI_PKT_RTCP_STATISTICS_RESET  _IOW(IFX_TAPI_IOC_MAGIC, 0x16, int)

/** This service is used to generate RFC2833 event from the application software.
   The file descriptor is a channel file descriptor.

   \param IFX_TAPI_PKT_EV_GENERATE_t* The parameter points to a
          \ref IFX_TAPI_PKT_EV_GENERATE_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PKT_EV_GENERATE_t param;
   memset (&param, 0, sizeof(IFX_TAPI_PKT_EV_GENERATE_t));
   param.action = IFX_TAPI_EV_GEN_START;
   param.event = 64;
   param.duration = 5;
   ret = ioctl(fd, IFX_TAPI_PKT_EV_GENERATE, &param);
   \endcode   */
#define  IFX_TAPI_PKT_EV_GENERATE            _IOW(IFX_TAPI_IOC_MAGIC, 0x77, int)


/** This service is used to configure the generation of RFC2833 events from
   the application software. The file descriptor is a channel file descriptor.

   \param IFX_TAPI_PKT_EV_GENERATE_CFG_t* The parameter points to a
          \ref IFX_TAPI_PKT_EV_GENERATE_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PKT_EV_GENERATE_CFG_t param;
   memset (&param, 0, sizeof(IFX_TAPI_PKT_EV_GENERATE_CFG_t));
   param.local = IFX_FALSE;
   ret = ioctl(fd, IFX_TAPI_PKT_EV_GENERATE_CFG, &param);
   \endcode   */
#define  IFX_TAPI_PKT_EV_GENERATE_CFG        _IOW(IFX_TAPI_IOC_MAGIC, 0x78, int)

/**
   Flush the upstream packet fifo on a channel.
   Note: You might create race conditions where you are woken up already,
         then you flush the packets and wait blocking in read for packets.
*/
#define  IFX_TAPI_PKT_FLUSH                  _IOW(IFX_TAPI_IOC_MAGIC, 0x8C, int)

/** Configures the jitter buffer. The file descriptor is applicable to data
    channel file descriptors.

   \param IFX_TAPI_JB_CFG_t* The parameter points to a
   \ref IFX_TAPI_JB_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks Selects fixed or adaptive Jitter Buffer and sets parameters. Modifies
    the Jitter Buffer settings during run-time.

   \code
   IFX_TAPI_JB_CFG_t param;
   IFX_int32_t fd;
   memset (&param, 0, sizeof(IFX_TAPI_JB_CFG_t));
   // Set to adaptive jitter buffer type
   param.nJbType = IFX_TAPI_JB_TYPE_ADAPTIVE;
   ret = ioctl(fd, IFX_TAPI_JB_CFG_SET, param);
   \endcode */
#define  IFX_TAPI_JB_CFG_SET                 _IOW(IFX_TAPI_IOC_MAGIC, 0x17, int)

/** Reads out Jitter Buffer statistics. The file descriptor is applicable to
    data channel file descriptors.

   \param IFX_TAPI_JB_STATISTICS_t* The parameter points to a
   \ref IFX_TAPI_JB_STATISTICS_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_JB_STATISTICS_t param;
   IFX_int32_t fd;
   memset (&param, 0, sizeof(IFX_TAPI_JB_STATISTICS_t));
   ioctl(fd, IFX_TAPI_JB_STATISTICS_GET, param);
   \endcode */
#define  IFX_TAPI_JB_STATISTICS_GET          _IOR(IFX_TAPI_IOC_MAGIC, 0x18, int)

/** Resets the jitter buffer statistics. The file descriptor is applicable to
    data channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_JB_STATISTICS_RESET, 0);
   \endcode   */
#define  IFX_TAPI_JB_STATISTICS_RESET        _IOW(IFX_TAPI_IOC_MAGIC, 0x19, int)

/** This interface connects a phone channel to another phone channel. See also
    description in Chapter 2.9. The file descriptor is applicable to phone
    channel file descriptors containing an analog interface.

   \param IFX_TAPI_MAP_PHONE_t* The parameter points to a
   \ref IFX_TAPI_MAP_PHONE_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The host has to take care about the resource management.

   \code
   IFX_TAPI_MAP_PHONE_t param;
   IFX_int32_t fd;
   memset(&param, 0, sizeof(IFX_TAPI_MAP_PHONE_t));
   param.nPhoneCh = 1;
   ioctl(fd, IFX_TAPI_MAP_PHONE_ADD, &param);
   \endcode */
#define  IFX_TAPI_MAP_PHONE_ADD              _IOW(IFX_TAPI_IOC_MAGIC, 0x2A, int)

/** This interface removes a phone channel from a phone channel. See also
    description in Chapter 2.9. The file descriptor is applicable to phone
    channel file descriptors containing an analog interface.

   \param IFX_TAPI_MAP_PHONE_t* The parameter points to a
   \ref IFX_TAPI_MAP_PHONE_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_MAP_PHONE_t param;
   IFX_int32_t fd;
   memset (&param, 0, sizeof(IFX_TAPI_MAP_PHONE_t));
   param.nPhoneCh = 1;
   ioctl(fd, IFX_TAPI_MAP_PHONE_REMOVE, &param);
   \endcode */
#define  IFX_TAPI_MAP_PHONE_REMOVE           _IOW(IFX_TAPI_IOC_MAGIC, 0x2B, int)

/** This interface connects the data channel to a phone channel. See also
   description in Chapter 2.9. The file descriptor is applicable to data channel
   file descriptors.

   \note It is recommended to choose the destination using
   \ref IFX_TAPI_MAP_TYPE_t enumerator. TAPI includes IFX_TAPI_MAP_DATA_TYPE_t
    enumerator for backwards compatibility reasons only.

   \param IFX_TAPI_MAP_DATA_t* The parameter points to a
   \ref IFX_TAPI_MAP_DATA_t structure

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The host has to take care about the resource management. It has to
    count the data channel (codec) resource usage and maintains a list, which
    data channel is mapped to which phone channel. The available resources can
    be queried by \ref IFX_TAPI_CAP_CHECK.

   \code
   IFX_TAPI_MAP_DATA_t datamap;
   IFX_int32_t fd;
   memset(&datamap, 0, sizeof (IFX_TAPI_MAP_DATA_t));
   // Add phone 0 to data 0
   ioctl(fd, IFX_TAPI_MAP_DATA_ADD, &datamap);
   datamap.nDstCh = 1;
   // Add phone 1 to data 0
   ioctl(fd, IFX_TAPI_MAP_DATA_ADD, &datamap);
   \endcode   */
#define  IFX_TAPI_MAP_DATA_ADD               _IOW(IFX_TAPI_IOC_MAGIC, 0x24, int)

/** This interface removes a data channel from an analog phone device.
 \note It is recommended to choose the destination using IFX_TAPI_MAP_TYPE_t
 enumerator! TAPI includes the IFX_TAPI_MAP_DATA_TYPE_t enumerator for
   backwards compatibility reasons only. The file descriptor is applicable to
   data channel file descriptors.

   \param IFX_TAPI_MAP_DATA_t* The parameter points to a
   \ref IFX_TAPI_MAP_DATA_t structure

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The host has to take care about the resource management. It has to
   count the data channel (codec) resource usage and maintains a list, which
   data channel is mapped to which phone channel. The available resources can
   be queried by \ref IFX_TAPI_CAP_CHECK.

   \code
   IFX_TAPI_MAP_DATA_t datamap;
   IFX_int32_t fd;
   memset(&datamap, 0, sizeof(IFX_TAPI_MAP_DATA_t));
   // Do something ....
   // Remove connection between phone channel 1 (nDstCh=1)
   and data channel 1 (fd)
   datamap.nDstCh = 1;
   ioctl(fd, IFX_TAPI_MAP_DATA_REMOVE, &datamap);
   // Now phone and data resources are unmapped
   \endcode   */
#define  IFX_TAPI_MAP_DATA_REMOVE            _IOW(IFX_TAPI_IOC_MAGIC, 0x25, int)

/** This interface connects the PCM channel to a phone channel. See also
   description in Chapter 2.9. The File descriptor is applicable to phone channel
   file descriptors containing a PCM interface.

   \param IFX_TAPI_MAP_PCM_t* The parameter points to a
   \ref IFX_TAPI_MAP_PCM_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_MAP_PCM_t pcmMap;
   IFX_int32_t fd;
   memset(&pcmMap, 0, sizeof(IFX_TAPI_MAP_PCM_t));
   // Connect PCM2 (addressed by fd) to phone channel 1 (analog)
   pcmMap.nDstCh = 1;
   ioctl(fd, IFX_TAPI_MAP_PCM_ADD, &pcmMap);
   \endcode   */
#define  IFX_TAPI_MAP_PCM_ADD                _IOW(IFX_TAPI_IOC_MAGIC, 0x43, int)

/** This interface removes the PCM channel from the phone channel. See also
   description in Chapter 2.9. File descriptor. It is applicable to phone channel
   file descriptors containing a PCM interface.

   \param IFX_TAPI_MAP_PCM_t* The parameter points to a
   \ref IFX_TAPI_MAP_PCM_t structure.

      \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_MAP_PCM_t param;
   IFX_int32_t fd;
   memset(&param, 0, sizeof (IFX_TAPI_MAP_PCM_t));
   // PCM channel 1 is removed from the phone channel 2
   param.nDstCh = 2;
   // fd1 represents PCM channel 1
   ioctl(fd, IFX_TAPI_MAP_PCM_REMOVE, &param);
   \endcode   */
#define  IFX_TAPI_MAP_PCM_REMOVE             _IOW(IFX_TAPI_IOC_MAGIC, 0x44, int)

/** This interface connects a DECT channel to another ALM, PCM or DECT channel.
     The file descriptor is applicable to file descriptors including a DECT
     channel.

   \param IFX_TAPI_MAP_DECT_t* The parameter points to a
   \ref IFX_TAPI_MAP_DECT_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_MAP_DECT_t param;
   memset (&param, 0, sizeof(IFX_TAPI_MAP_DECT_t));
   // PCM channel 2 is removed from the phone channel 1
   param.nDstCh = 2;
   // fd1 represents PCM channel 1
   ret = ioctl(fd1, IFX_TAPI_MAP_DECT_REMOVE, &param)
   \endcode   */
#define  IFX_TAPI_MAP_DECT_ADD               _IOW(IFX_TAPI_IOC_MAGIC, 0x37, int)

/** This interface disconnects a DECT channel from another ALM, PCM or
    DECT channel. The file descriptor is applicable to file descriptors
    including a DECT channel.

   \param IFX_TAPI_MAP_DECT_t* The parameter points to a
   \ref IFX_TAPI_MAP_DECT_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_MAP_DECT_t param;
   memset (&param, 0, sizeof(IFX_TAPI_MAP_DECT_t));
   // DECT channel 2 is removed from the phone channel 1
   param.nDstCh = 2;
   // fd1 represents PCM channel 1
   ret = ioctl(fd1, IFX_TAPI_MAP_DECT_REMOVE, &param)
   \endcode   */
#define  IFX_TAPI_MAP_DECT_REMOVE            _IOW(IFX_TAPI_IOC_MAGIC, 0x38, int)

/** This service reads the configured encoding length. The file descriptor is
    applicable to data channel file descriptors.

   \param int* Pointer to the length of frames in milliseconds.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_int32_t fd;
   IFX_int32_t nFrameLength;
   ioctl(fd, IFX_TAPI_ENC_FRAME_LEN_GET, &nFrameLength);
   \endcode
*/
#define  IFX_TAPI_ENC_FRAME_LEN_GET                _IO(IFX_TAPI_IOC_MAGIC, 0x2D)

/** This service is used to configure encoding type and length.

   \remarks The ioctls IFX_TAPI_ENC_TYPE_SET and IFX_TAPI_ENC_FRAME_LEN_SET are
    obsolete and have been replaced by IFX_TAPI_ENC_CFG_SET.

   \param IFX_TAPI_ENC_CFG_t* The parameter points to a
          \ref IFX_TAPI_ENC_CFG_t structure.

  \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

*/
#define  IFX_TAPI_ENC_CFG_SET               _IOW (IFX_TAPI_IOC_MAGIC, 0xAF, int)

/** This service is used to configure the decoder. Currently only the bitpacking
    according to ITU-T I366.2 Bit Alignment for G.726 codecs is configurable.

   \param IFX_TAPI_DEC_CFG_t* The parameter points to a
          \ref IFX_TAPI_DEC_CFG_t structure.

  \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_DEC_CFG_SET               _IOW (IFX_TAPI_IOC_MAGIC, 0xB6, int)

/** Selects a vocoder for the encoding. The file descriptor is applicable to
    data channel file descriptors.

\note This ioctl is obsolete and is replaced by \ref IFX_TAPI_ENC_CFG_SET.

   \param IFX_TAPI_ENC_TYPE_t The parameter specifies the codec as defined
   in \ref IFX_TAPI_ENC_TYPE_t. Default codec is G711, u-Law

  \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   // set the codec
   ioctl(fd, IFX_TAPI_ENC_TYPE_SET, IFX_TAPI_ENC_TYPE_G726_16);
   \endcode  */
#define  IFX_TAPI_ENC_TYPE_SET               _IOW (IFX_TAPI_IOC_MAGIC, 0x89,int)

/** Start encoding and packetization. The file descriptor is applicable to data
    channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The voice is serviced with the drivers read and write interface.
   The data format is dependent on the selected setup (coder, chip setup,
    etc.). For example, an RTP setup will receive RTP packets. Read and
    write are always non-blocking. If the codec has not been set before with
    \ref IFX_TAPI_ENC_CFG_SET the encoder is started with G711.

   \code
   ioctl(fd, IFX_TAPI_ENC_START, 0);
   \endcode */
#define  IFX_TAPI_ENC_START                       _IO (IFX_TAPI_IOC_MAGIC, 0x8A)

/** Stops encoding and packetization on this channel. The file descriptor is
    applicable to data channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_ENC_STOP, 0);
   \endcode   */
#define  IFX_TAPI_ENC_STOP                        _IO (IFX_TAPI_IOC_MAGIC, 0x8B)

/** This service configures the encoding length. The file descriptor is
    applicable to data channel file descriptors.

\note This ioctl is obsolete and is replaced by  \ref IFX_TAPI_ENC_CFG_SET.

   \param int Length of frames in milliseconds

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nFrameLength = 10;
   ioctl(fd, IFX_TAPI_ENC_FRAME_LEN_SET, nFrameLength);
   \endcode   */
#define  IFX_TAPI_ENC_FRAME_LEN_SET         _IOW (IFX_TAPI_IOC_MAGIC, 0x8D, int)

/** This service is used to control the enconder hold functionality. The file
    descriptor is applicable to data channel file descriptors.

   \param IFX_operation_t Enable or disable hold, selected out
   of IFX_operation_t.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_ENC_HOLD                   _IOW(IFX_TAPI_IOC_MAGIC, 0x27, int)

/** Selects a codec for the data channel playout.

   \param \ref IFX_TAPI_ENC_TYPE_t The parameter specifies the codec,
          default G711.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This interface is currently not supported, because the codec
   is determined by the payload type of the received packets. */
#define  IFX_TAPI_DEC_TYPE_SET              _IOW (IFX_TAPI_IOC_MAGIC, 0x90, int)

/** Starts the decoding of data. The file descriptor is applicable to data
   channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_DEC_START, 0);
   \endcode
  */
#define  IFX_TAPI_DEC_START                       _IO (IFX_TAPI_IOC_MAGIC, 0x91)

/** Stops the playout of data. The file descriptor is applicable to data
   channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

\remarks Stop of the decoding will lead to a reset of the connection
    statistics.

   \code
   ioctl(fd, IFX_TAPI_DEC_STOP, 0);
   \endcode   */
#define  IFX_TAPI_DEC_STOP                        _IO (IFX_TAPI_IOC_MAGIC, 0x92)

/** This service switches on/off the high-pass (HP) filters of the decoder path
   in the COD module. The file descriptor is applicable to data channel file
   descriptors.
   \param IFX_boolean_t, value of:
    - IFX_TRUE  switches HP filter ON.
    - IFX_FALSE switches HP filter OFF.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_boolean_t bON;

   bON = IFX_FALSE; // switch off decoder HP filter
   ret = ioctl(fd, IFX_TAPI_COD_DEC_HP_SET, bON);
   \endcode
*/
#define  IFX_TAPI_COD_DEC_HP_SET                   _IOW (IFX_TAPI_IOC_MAGIC, 0xB3, int)

/** Sets the volume settings of the COD module, both for the receiving
    (downstream) and transmitting (upstream)paths. The file descriptor is
    applicable to data channel file descriptors.

   \param IFX_TAPI_PKT_VOLUME_t* This interface expects a pointer to a
   \ref IFX_TAPI_PKT_VOLUME_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PKT_VOLUME_t param;
   memset (&param, 0, sizeof(IFX_TAPI_PKT_VOLUME_t));
   param.nEnc = 6;    // dB
   param.nDec = 9;    // dB
   ret = ioctl(fd, IFX_TAPI_COD_VOLUME_SET, &param);
   \endcode
*/
#define  IFX_TAPI_COD_VOLUME_SET             _IOW(IFX_TAPI_IOC_MAGIC, 0xB4, int)

/** Configures AGC coefficients for a coder module. This implementation assumes
   that an index of an AGC resource is fixedly assigned to the related index of
   the coder module. The file descriptor is applicable to data channel file
   descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_ENC_AGC_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

    \code
   IFX_TAPI_ENC_AGC_CFG_t param;
   memset (&param, 0, sizeof(IFX_TAPI_ENC_AGC_CFG_t));
   param.cin  = -30;
   param.gain = +10;
   param.att  = -20;
   param.lim  = -25;
   ret = ioctl(fd, IFX_TAPI_ENC_AGC_CFG, &param)
   \endcode
*/
#define  IFX_TAPI_ENC_AGC_CFG                _IOW(IFX_TAPI_IOC_MAGIC, 0x96, int)

/** Enables / Disables the AGC. The file descriptor is applicable to data channel
    file descriptors.

   \param Valid values are defined in the \ref IFX_TAPI_ENC_AGC_MODE_t enumerator.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_ENC_AGC_ENABLE             _IOW(IFX_TAPI_IOC_MAGIC, 0x97, int)

/** Configures the voice activity detection and silence handling. Voice Activity
    Detection (VAD) is a feature that allows the codec to determine whether to
    send voice data or silence data. The file descriptor is applicable to data
    channel file descriptors.

   \param IFX_TAPI_ENC_VAD_t Select the VAD mode out of
   \ref IFX_TAPI_ENC_VAD_t.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   Codecs G.723.1 and G.729A/B have a built-in comfort noise generation (CNG).
   Explicitly switching on the CNG is not necessary. Voice activity detection
   may not be available for G.728 and G.729E.

   \code
   // set the VAD on
   ioctl(fd, IFX_TAPI_ENC_VAD_CFG_SET, IFX_TAPI_ENC_VAD_ON);
   \endcode
*/
#define  IFX_TAPI_ENC_VAD_CFG_SET           _IOW (IFX_TAPI_IOC_MAGIC, 0xA9, int)

/** Configuration and start of room noise detection. The file descriptor is
    applicable to data channel file descriptors.

   \param IFX_TAPI_ENC_ROOM_NOISE_DETECT_t* The parameter points to a
   \ref IFX_TAPI_ENC_ROOM_NOISE_DETECT_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_ENC_ROOM_NOISE_DETECT_START _IOW (IFX_TAPI_IOC_MAGIC, 0xAD, int)

/** This function stops room noise detection. The file descriptor is applicable
    to data channel file descriptors.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_ENC_ROOM_NOISE_DETECT_STOP _IOW (IFX_TAPI_IOC_MAGIC, 0xAE, int)


/** Configure the mapping of packet streams to KPI channels or the application.

   \param IFX_TAPI_KPI_CH_CFG_t* The parameter points to a
          \ref IFX_TAPI_KPI_CH_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_KPI_CH_CFG_SET            _IOW (IFX_TAPI_IOC_MAGIC, 0x2F, int)

/*@}*/ /* TAPI_INTERFACE_CON */

/* ======================================================================== */
/* TAPI Miscellaneous Services, ioctl commands (Group TAPI_INTERFACE_MISC)  */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_MISC */
/*@{*/

/** Retrieves the TAPI version string. The file descriptor is applicable to
    device file descriptors.

   \param char* Pointer to version character string.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_int32_t fd;
   IFX_char_t Version[80];
   ioctl(fd, IFX_TAPI_VERSION_GET, (IFX_char_t*) &Version[0]);
   printf("Version:%s\n" , Version);
   \endcode */
#define  IFX_TAPI_VERSION_GET                      _IO(IFX_TAPI_IOC_MAGIC, 0x00)

#ifndef TAPI_DXY_DOC
/** This service masks the reporting of the exceptions.
   Set the mask to 1 disables the reporting of the corresponding event.

   \param int The parameter defines the exception event bits mask,
   defined in IFX_TAPI_EXCEPTION_BITS_t

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks If digit reporting is enabled, digits are also read out in
   \ref IFX_TAPI_EVENT_GET.

   \code
   // mask reporting of exceptions
   IFX_TAPI_EXCEPTION_t    exceptionMask;
   memset (&exceptionMask, 0, sizeof (exceptionMask));
   // mask dtmf ready event
   exceptionMask.Bits.dtmf_ready = 1;
   // mask hook state event
   exceptionMask.Bits.hookstate  = 1;
   ioctl(fd, IFX_TAPI_EXCEPTION_MASK, exceptionMask.Status);
   \endcode
*/
#define  IFX_TAPI_EXCEPTION_MASK                   _IO(IFX_TAPI_IOC_MAGIC, 0x10)
#endif /* TAPI_DXY_DOC */

/** Sets the report levels if the driver is compiled with ENABLE_TRACE. The file
 descriptor It is applicable to device file descriptors.

    \param int A valid argument is one value of
    \ref IFX_TAPI_DEBUG_REPORT_SET_t.

    \return No return value.
*/
#define  IFX_TAPI_DEBUG_REPORT_SET                 _IO(IFX_TAPI_IOC_MAGIC, 0x12)

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/** Checks status information of the phone line.

   \param IFX_TAPI_CH_STATUS_t* The parameter points to a
   \ref IFX_TAPI_CH_STATUS_t array.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks If the exception bits reporting of digits is enabled
   (see \ref IFX_TAPI_EXCEPTION_MASK), the digit fifo is read out once and the
   information is passed to the status. It may occur that more than one
   digit is located in the fifo. Therefore the application must check with
   \ref IFX_TAPI_TONE_DTMF_READY_GET or \ref IFX_TAPI_PULSE_READY if further
   data is contained in the fifos.
   This interface replaces the exception bits.

   \code
   // This example shows the call of the status information of one channel
   #define TAPI_STATUS_DTMF   0x01
   #define TAPI_STATUS_PULSE  0x02
   IFX_TAPI_CH_STATUS_t status[1];
   int fd;
   fd = open ("/dev/vin11", O_RDWR);
   status[0].channels = 0;
   // get the status
   ioctl(fd, IFX_TAPI_CH_STATUS_GET, status);
   if (status[0].dialing & TAPI_STATUS_DTMF ||
       status[0].dialing & TAPI_STATUS_PULSE)
      printf ("Dial key %d detected\n", status.digit);
   \endcode

   \code
   // This example shows a select waiting on one fd for exceptions and on
   // two others for data. In one call the status information of all two
   // channels are queried from the driver.
   IFX_TAPI_CH_STATUS_t stat[4];
   int fdcfg, fd[2], i;
   fd_set rfds;
   FD_ZERO (&rfds);
   fdcfg = open ("/dev/vin10", O_RDWR);
   FD_SET (fdcfg, &rfds);
   width = fdcfg;
   fd[0] = open ("/dev/vin11", O_RDWR);
   FD_SET (fd[0], &rfds);
   if (width < fd[0])
      width = fd[0];
   fd[1] = open ("/dev/vin12", O_RDWR);
   FD_SET (fd[1], &rfds);
   if (width < fd[1])
      width = fd[1];
   ret = select(width + 1, &rfds, NULL, NULL, NULL);
   // select woke up from exception on fdcfg or data on fd[x]
   if (FD_ISSET(fdcfg, &rfds))
   {
      // exception occurred
      status[0].channels = 2;
      // get the status
      ioctl(fdcfg, IFX_TAPI_CH_STATUS_GET, stat);
      for (i = 0; i < 2; i++)
      {
         if (stat[i].hook || stat[i].dialing ||
             stat[i].line || stat[i].digit || stat[i].signal)
            nState = HandleException (pCon, &stat[i]);
      }
   }
   for (i = 0; i < 2; i++)
   {
      if (FD_ISSET(fd[i], &rfds))
         HandleData (fd[i]);
   }
   // go on with state handling
   \endcode

   \code
   // This example shows a select waiting on channel fds for GR909 measurements
   // exceptions, assuming that the low level driver provides io interfaces to
   // start measurements and read results.
   IFX_TAPI_CH_STATUS_t stat[4];
   int fdcon, fd[MAX_CHANNELS], i;
   fd_set rfds;
   FD_ZERO (&rfds);
   fdcon = open ("/dev/vin10", O_RDWR);
   FD_SET (fdcon, &rfds);
   //get channel fds
   for (i = 0; i < MAX_CHANNELS, i++)
   {
      fd[i] = open (achDev[i], O_RDWR);
      if (fd [i] <= 0)
         return (-1);
      // initialize channel
      init_channel (fd[i]);
      //start first measurement
      start_measurement (fd[i], VOLTAGE);
   }
   while (1)
   {
      select(fdcon + 1, &rfds, NULL, NULL, NULL);
      // select woke up from exception on fdcon
      if (FD_ISSET(fdcon, &rfds))
      {
         // exception occurred
         status[0].channels = MAX_CHANNELS;
         // get the status
         ioctl(fdcfg, IFX_TAPI_CH_STATUS_GET, stat);
         for (i = 0; i < MAX_CHANNELS, i++)
         {
            if (status[i].line & IFX_TAPI_LINE_STATUS_GR909RES)
            {
               //read result
               ret = read_gr909_result(fd[i], &result_struct);
               if (ret == 0)
               {
                  switch (result_struct.measurement)
                  {
                  case VOLTAGE:
                     ret = check_voltages(&result_struct);
                     if (ret == 0)
                        start_measurement (fd[i], RESISTIVE_FAULT);
                     break;
                  case RESISTIVE_FAULT:
                     ret = check_resistive_fault(&result_struct);
                     if (ret == 0)
                        start_measurement (fd[i], RECEIVER_OFFHOOK);
                     break;
                  case RECEIVER_OFFHOOK:
                     ret = check_receiver_offhook (&result_struct);
                     if (ret == 0)
                        start_measurement (fd[i], RINGER);
                     break;
                  case RINGER:
                     ret = check_ringer (&result_struct);
                     // all measurements were ok. Stop machine now.
                     if (ret == 0)
                        return (0);
                  }
                  //error: values out of allowed range. Stop machine now.
                  if (ret == -1)
                     return (-1);
               }
            }
         }
      }
   }
   \endcode
*/
#define  IFX_TAPI_CH_STATUS_GET                   _IO (IFX_TAPI_IOC_MAGIC, 0x2C)
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

/** Checks the supported TAPI interface version. The file descriptor is
    applicable to device file descriptors.

   \param IFX_TAPI_VERSION_t The parameter points to an
   \ref IFX_TAPI_VERSION_t structure.

   \return No return value.

   \remarks
   Since an application is always build against one specific TAPI interface
   version it should check if it is supported. If not the application should
   abort. This interface checks if the current TAPI version supports a
   particular version. For example the TAPI versions 2.1 will support TAPI 2.0
   But version 3.0 might not support 2.0.

   \code
   IFX_TAPI_VERSION_t version;
   IFX_int32_t fd;
   memset(&version, 0, sizeof(IFX_TAPI_VERSION_t));
   version.major = 2;
   version.minor = 1;
   if (0 == ioctl(fd, IFX_TAPI_VERSION_CHECK, (IFX_int32_t) &version))
   {
      printf( "Version 2.1 supported\n");
   }
   \endcode */
#define  IFX_TAPI_VERSION_CHECK              _IOW(IFX_TAPI_IOC_MAGIC, 0x39, int)

/** This service returns the number of capabilities. The file desccriptor is
    applicable to device file descriptors.

   \param int Pointer to the number of capabilities which are returned.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_CAP_NR_t param;
   IFX_int32_t fd;
   IFX_int32_t nCapNr;
   // Get the cap list size
   ioctl(fd, IFX_TAPI_CAP_NR, &nCapNr);
   \endcode   */
#define  IFX_TAPI_CAP_NR                          _IO (IFX_TAPI_IOC_MAGIC, 0x80)

/** This service returns the capability lists. The file desccriptor is applicable
    to device file descriptors.

   \param int The parameter points to a \ref IFX_TAPI_CAP_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_CAP_t *pCapList;
   IFX_int32_t fd;
   IFX_return_t ret;
   IFX_int32_t nCapNr;
   IFX_int32_t i;
   // Open channel file descriptor for channel 0 *
   fd = open("/dev/vin11", O_RDWR, 0x644);
   // Get the cap list size
   ioctl(fd, IFX_TAPI_CAP_NR, &nCapNr);
   pCapList = (IFX_TAPI_CAP_t*) malloc (nCapNr * sizeof(IFX_TAPI_CAP_t));
   // Get the cap list
   ioctl(fd, IFX_TAPI_CAP_LIST, pCapList);
   for (i = 0; i < nCapNr; i++)
   {
      switch (pCapList[i].captype)
      {
           case IFX_TAPI_CAP_TYPE_CODEC:
               printf ("Codec: %s\n\r", pCapList[i].desc);
               break;
           case IFX_TAPI_CAP_TYPE_PCM:
              printf ("PCM: %d\n\r", pCapList[i].cap);
              break;
           case IFX_TAPI_CAP_TYPE_CODECS:
              printf ("CODER: %d\n\r", pCapList[i].cap);
              break;
           case IFX_TAPI_CAP_TYPE_PHONES:
              printf ("PHONES: %d\n\r", pCapList[i].cap);
              break;
           default:
               break;
      }
   }
   // Free the allocated memory
   free(pCapList);
   pCapList = IFX_NULL;
   // Close all open fds
   close(fd);
   \endcode   */
#define  IFX_TAPI_CAP_LIST                        _IO (IFX_TAPI_IOC_MAGIC, 0x81)

/** This service checks if a specific capability is supported. The file
   desccriptor is applicable to device file descriptors.

   \param IFX_TAPI_CAP_t* The parameter points to a
   \ref IFX_TAPI_CAP_t structure.

   \return Returns value as follows:
      - 0: Capability not supported
      - 1:  Capability supported
      - -1: In case of an error

   \code
   IFX_TAPI_CAP_t CapList;
   IFX_int32_t fd;
   IFX_return_t ret;
   // Open channel file descriptor for channel 0
   fd = open("/dev/vin11", O_RDWR, 0x644);
   memset (&CapList, 0, sizeof(IFX_TAPI_CAP_t));
   // Check if G726, 16 kBit/s is supported
   CapList.captype = IFX_TAPI_CAP_TYPE_CODEC;
   CapList.cap = IFX_TAPI_COD_TYPE_G726_16;
   ret = ioctl(fd, IFX_TAPI_CAP_CHECK, &CapList);
   if (ret > 0)
   {
      printf( "G726_16 supported\n" );
   }
   // Check how many data channels are supported
   CapList.captype = IFX_TAPI_CAP_TYPE_CODECS;
   ret = ioctl(fd, IFX_TAPI_CAP_CHECK, &CapList);
   if (ret > 0)
   {
      printf("%d data channels supported\n", CapList.cap);
   }
   // Check if POTS port is available
   CapList.captype = IFX_TAPI_CAP_TYPE_PORT;
   CapList.cap = IFX_TAPI_CAP_PORT_POTS;
   ret = ioctl(fd, IFX_TAPI_CAP_CHECK, &CapList);
   if (ret > 0)
   {
      printf("POTS port supported\n");
   }
   // close all open fds
   close(fd);
   \endcode   */
#define  IFX_TAPI_CAP_CHECK                       _IO (IFX_TAPI_IOC_MAGIC, 0x82)

#ifdef ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE
/**
   This service returns the current exception status.

   \param IFX_TAPI_EXCEPTION_t* The parameter points to a
   \ref IFX_TAPI_EXCEPTION_t structure.

   \remarks
   The user application must poll cidrx_supdate, once the CID Receiver was
   started with IFX_TAPI_CID_RX_START. If it is set the application
   calls IFX_TAPI_CID_RX_STATUS_GET in order to know if CID data is
   available for reading or if an error occured during CID Reception.

   \code
   IFX_TAPI_EXCEPTION_t nException;
   //  get the exception
   nException.Status = ioctl(fd, IFX_TAPI_EXCEPTION_GET, 0);
   if (nException.Bits.dtmf_ready)
      printf ("DTMF detected\n");
   \endcode
*/
#define  IFX_TAPI_EXCEPTION_GET             _IOR (IFX_TAPI_IOC_MAGIC, 0x9A, int)
#endif /* ENABLE_OBSOLETE_BITFIELD_EVENT_INTERFACE */

/** This service returns the last error code occured in the the
    TAPI driver or the low level driver. It contains also a error
    stack for tracking down the origin of the error source.

    After calling this service the stack is reset.

   \param IFX_TAPI_Error_t* The parameter points to a
   \ref IFX_TAPI_Error_t structure.

   \return Always 0

   \code
   #include "drv_vmmc_strerrno.h"

   ret = ioctl (fd, IFX_TAPI_LINE_FEED_SET, (int)IFX_TAPI_LINE_FEED_STANDBY);
   if (ret != IFX_SUCCESS)
   {
      ioctl (fd, IFX_TAPI_LASTERR, (int)&error);
      if (error.nCode != -1)
      {
         // we have additional information
         printf ("Error Code 0x%X occured\n", error.nCode);
         for (i = 0; i < ERRNO_CNT; ++i)
         {
            if (drvErrnos[i] == (error.nCode & 0xffff))
            {
               printf ("%s\n", drvErrStrings[i]);
            }
         }
         for (i = 0; i < error.nCnt; ++i)
         {
            printf ("%s:%d Code 0x%X\n", error.stack[i].sFile,
            error.stack[i].nLine,
            error.stack[i].nCode);
         }
      }
   }
   \endcode
   */
#define  IFX_TAPI_LASTERR              _IOW(IFX_TAPI_IOC_MAGIC, 0x48, int)

/*@}*/ /* TAPI_INTERFACE_MISC */


/* ========================================================================== */
/* TAPI Power Ringing Services, ioctl commands (Group TAPI_INTERFACE_RINGING) */
/* ========================================================================== */
/** \addtogroup TAPI_INTERFACE_RINGING */
/*@{*/

/** This service sets the ring configuration for the non-blocking
    Power Ringing Services. The file descriptor is applicable to phone channel
    descriptors containing an analog interface.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_RING_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The configuration has to be set before ringing starts using the
    interface \ref IFX_TAPI_RING_START .

   \code
     IFX_TAPI_RING_CFG_t param;
     memset (&param, 0, sizeof(IFX_TAPI_RING_CFG_t));
     param.mode = 0;
     param.submode = 1;
     ret = ioctl(fd, IFX_TAPI_RING_CFG_SET, &param)
   \endcode
*/
#define  IFX_TAPI_RING_CFG_SET                     _IO(IFX_TAPI_IOC_MAGIC, 0x02)

/** This service sets the high resolution ring cadence for the Power Ringing
   Services. The cadence value has to be set before ringing is started with
   \ref IFX_TAPI_RING_START or \ref IFX_TAPI_RING. The file descriptor is
    applicable to phone channel descriptors containing an analog interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_RING_CADENCE_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error.

   \remarks
   The number of ring bursts can be obtained counting the events in
    \ref IFX_TAPI_EVENT_FXS_RING.
   The initial cadence pattern can be of zero length in which case the initial
   pattern will not be played. If a length for the initial pattern is given
   the initial pattern may not consists of all zero bits but it may consist
   of all bits set to one.
   The periodic pattern must contain at least one zero bit and one bit set to
   one. This implies that the length of the periodic pattern must be at least
   two.

   \code
   // pattern of 3 sec. ring and 1 sec. pause
   char data[10] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00 };
   // initial 1 sec. ring and 1 sec. non ringing signal before ringing
   char initial[5] =0xFF,0xFF,0xF0,0x00, 0x00};
   IFX_TAPI_RING_CADENCE_t Cadence;
   IFX_int32_t fd;

   memset (&Cadence, 0, sizeof(IFX_TAPI_RING_CADENCE_t));
   memcpy (&Cadence.data[0], data, sizeof (data));
   Cadence.nr = 10 * 8;
   // set size in bits
   memcpy (&Cadence.initial[0], initial, sizeof (initial));
   Cadence.initialNr = 5 * 8;
   // set the cadence sequence
   ioctl(fd, IFX_TAPI_RING_CADENCE_HR_SET, &Cadence);
   \endcode
*/
#define  IFX_TAPI_RING_CADENCE_HR_SET              _IO(IFX_TAPI_IOC_MAGIC, 0x03)

/** This service gets the ring configuration for the non-blocking
    Power Ringing Services. The file descriptor is applicable to phone channel
    descriptors containing an analog interface.

   \param IFX_int32_t The parameter points to a
          \ref IFX_TAPI_RING_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

*/
#define  IFX_TAPI_RING_CFG_GET                     _IO(IFX_TAPI_IOC_MAGIC, 0x1F)

/** This service rings the phone. The service is blocking and will not return
    until the phone went off-hook or the maximum number of ring cadences as
    previously set by \ref IFX_TAPI_RING_MAX_SET was played. It is applicable
    to phone channel file descriptors containing an analog interface.

   \param int This interface expects no parameter. It should be set to 0.

   \return The execution status can be:
      - 0: if number of rings reached
      - 1: if phone was hooked off
      - -1: in case of an error

   \code
   int nMaxRing = 3, ret;
   // set the maximum rings
   ioctl(fd, IFX_TAPI_RING_MAX_SET, nMaxRing);
   // ring the phone
   ret = ioctl(fd, IFX_TAPI_RING, 0);
   if (ret == 0)
   {
      // no answer, maximum number of rings reached
    }
    else if (ret == 1)
    {
      // phone hooked off
    }
   \endcode   */
#define  IFX_TAPI_RING                            _IO (IFX_TAPI_IOC_MAGIC, 0x83)

/** This service sets the maximum number of cadences after which ringing stops
    automatically.

   \param int The parameter defines the number of cadences to be played.
              A value of 0 means infinity.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   int nMaxRing = 3;
   ioctl(fd, IFX_TAPI_RING_MAX_SET, nMaxRing);
   \endcode   */
#define  IFX_TAPI_RING_MAX_SET             _IOW (IFX_TAPI_IOC_MAGIC, 0x85, char)

/** This service sets the ring cadence for the non-blocking Power Ringing
    Services. The cadence value has to be set before ringing starts
   (\ref IFX_TAPI_RING_START). This service is non-blocking. The file descriptor
   is applicable to phone channel descriptors containing an analog interface.

   \param IFX_int32_t The parameter defines the cadence. This value contains
   the encoded cadence sequence. One bit represents ring cadence voltage
   for 0.5 sec.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   The number of ring bursts can be obtained counting the events
   \ref IFX_TAPI_EVENT_FXS_RING.

   \code
   // pattern of 3 sec. ring and 1 sec. pause : 1111 1100 1111 1100 ...
   IFX_uint32_t nCadence = 0xFCFCFCFC;
   IFX_int32_t fd;

   // set the cadence sequence
   ioctl(fd, IFX_TAPI_RING_CADENCE_SET, nCadence);
   \endcode   */
#define  IFX_TAPI_RING_CADENCE_SET        _IOW (IFX_TAPI_IOC_MAGIC, 0x86, short)

/** This service starts the non-blocking ringing on the phone line using the
    preconfigured ring cadence. The file descriptor is applicable to phone
    channel descriptors containing an analog interface.

   \param IFX_int32_t Parameter is ignored.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks
   This interface does not not provide caller ID services.
   If ringing with Caller ID wished, \ref IFX_TAPI_CID_TX_SEQ_START should be
   used! The ringing must be stopped with \ref IFX_TAPI_RING_STOP. A second call
   to \ref IFX_TAPI_RING_START while the phone rings returns an error.
   The ringing can be configured with the interfaces \ref IFX_TAPI_RING_CFG_SET,
   \ref IFX_TAPI_RING_CADENCE_SET and \ref IFX_TAPI_RING_CADENCE_HR_SET before
   this interface is called.

   \code
   // start the ringing
   ioctl(fd, IFX_TAPI_RING_START, 0);
   \endcode   */
#define  IFX_TAPI_RING_START                      _IO (IFX_TAPI_IOC_MAGIC, 0x87)

/** This service stops non-blocking ringing on the phone line which was started
   before with service \ref IFX_TAPI_RING_START or \ref IFX_TAPI_CID_TX_SEQ_START.
   The file descriptor is applicable to phone channel descriptors containing an
   analog interface.

   \param IFX_int32_t This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   ioctl(fd, IFX_TAPI_RING_STOP, 0);
   \endcode   */
#define  IFX_TAPI_RING_STOP                       _IO (IFX_TAPI_IOC_MAGIC, 0x88)

/*@}*/ /* TAPI_INTERFACE_RINGING */

/* ==================================================================== */
/* TAPI PCM Services, ioctl commands (Group TAPI_INTERFACE_PCM)         */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_PCM */
/*@{*/

/** This service sets the configuration of the PCM interface, it must be
    called after \ref IFX_TAPI_CH_INIT but before activating a PCM channel.
    After activating a PCM channel, the PCM interface settings cannot be
    modified anymore.
    If a PCM channel is activated without calling the PCM interface ioctl,
    default settings are configured automatically (PCM slave).

    \param IFX_int32_t  Pointer to a \ref IFX_TAPI_PCM_IF_CFG_t
           structure.
    \code
    IFX_TAPI_PCM_IF_CFG_t pcm_if;
    memset(&pcm_if, 0, sizeof(IFX_TAPI_PCM_IF_CFG_t));
    pcm_if.nOpMode       = IFX_TAPI_PCM_IF_MODE_SLAVE;
    pcm_if.nDCLFreq      = IFX_TAPI_PCM_IF_DCLFREQ_2048;
    pcm_if.nDoubleClk    = IFX_DISABLE;
    pcm_if.nSlopeTX      = IFX_TAPI_PCM_IF_SLOPE_RISE;
    pcm_if.nSlopeRX      = IFX_TAPI_PCM_IF_SLOPE_FALL;
    pcm_if.nOffsetTX     = IFX_TAPI_PCM_IF_OFFSET_NONE;
    pcm_if.nOffsetRX     = IFX_TAPI_PCM_IF_OFFSET_NONE;
    pcm_if.nDrive        = IFX_TAPI_PCM_IF_DRIVE_ENTIRE;
    pcm_if.nShift        = IFX_DISABLE;
    pcm_if.nMCTS         = 0x00;
    err = ioctl(fd, IFX_TAPI_PCM_IF_CFG_SET, &pcm_if);
    \endcode
*/
#define  IFX_TAPI_PCM_IF_CFG_SET                  _IO(IFX_TAPI_IOC_MAGIC, 0x11)

/** This service sets the configuration of a PCM channel. The file descriptor
    is aplicable to phone channel file descriptors containing a PCM interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_PCM_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks The parameter rate must be set to the PCM rate, which is applied to the
   device, otherwise an error is returned.

   \code
   IFX_TAPI_PCM_CFG_t Pcm;
   memset (&Pcm, 0, sizeof(IFX_TAPI_PCM_CFG_t));
   Pcm.nTimeslotRX = 5;
   Pcm.nTimeslotTX = 5;
   Pcm.nHighway = 1;
   // 16 bit resolution
   Pcm.nResolution = IFX_TAPI_PCM_RES_LINEAR_16BIT;
   // configure PCM interface
   ioctl(fd, IFX_TAPI_PCM_CFG_SET, &Pcm);
   \endcode   */
#define  IFX_TAPI_PCM_CFG_SET                      _IO(IFX_TAPI_IOC_MAGIC, 0x04)

/** This service gets the configuration of the PCM channel. The file descriptor
    is aplicable to phone channel file descriptors containing a PCM interface.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_PCM_CFG_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_PCM_CFG_t Pcm;
   IFX_int32_t fd;

   memset (&Pcm, 0, sizeof(IFX_TAPI_PCM_CFG_t));
   ioctl(fd, IFX_TAPI_PCM_CFG_GET, &Pcm);
   \endcode   */
#define  IFX_TAPI_PCM_CFG_GET                      _IO(IFX_TAPI_IOC_MAGIC, 0x05)

/** This service activate / deactivates the PCM timeslots configured for this
    channel. The file descriptor is aplicable to phone channel file descriptors
    containing a PCM interface.

   \param IFX_int32_t The parameter defines the activation status
       - 0 deactivate the timeslot
       - 1 activate the timeslot

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   // activate the PCM timeslot
   ioctl(fd, IFX_TAPI_PCM_ACTIVATION_SET, 1);
\endcode   */
#define  IFX_TAPI_PCM_ACTIVATION_SET               _IO(IFX_TAPI_IOC_MAGIC, 0x06)

/** This service receives the activation status from the PCM timeslots configured
   for this channel. The file descriptor is aplicable to phone channel file
   descriptors containing a PCM interface.

   \param bAct The parameter points to an integer which returns the following
    status:
          - 0: The timeslot is not active
          - 1: The timeslot is active

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_LINE_VOLUME_t param;
   IFX_int32_t fd;
   IFX_return_t ret;
   IFX_int32_t bAct;

   ret = ioctl(fd, IFX_TAPI_PCM_ACTIVATION_GET, &bAct);
   if ((IFX_SUCCESS == ret) && (1 == bAct))
   {
      printf("Activated\n");
   }
\endcode   */
#define  IFX_TAPI_PCM_ACTIVATION_GET               _IO(IFX_TAPI_IOC_MAGIC, 0x07)

/** This service switches on/off the HP filter of the decoder path in PCM module.
    The file descriptor is aplicable to phone channel file descriptors containing
    a PCM interface.

   \param IFX_boolean_t,
          Value of IFX_TRUE  switches HP filter ON.
          Value of IFX_FALSE switches HP filter OFF.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_boolean_t bON;
   // switch off decoder HP filter
   bON = IFX_FALSE;
   ret = ioctl(fd, IFX_TAPI_PCM_DEC_HP_SET, bON);
   \endcode
*/
#define  IFX_TAPI_PCM_DEC_HP_SET                   _IO (IFX_TAPI_IOC_MAGIC, 0x20)

/*@}*/ /* TAPI_INTERFACE_PCM */

/* ======================================================================= */
/* TAPI Fax T.38 Services, ioctl commands (Group TAPI_INTERFACE_FAX)       */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_FAX */
/*@{*/

/** This service configures and enables the modulator during a T.38 fax session.
   The file descriptor is applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_T38_MOD_DATA_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This service deactivates the voice data path and configures the
    channel for a fax session.

   \code
     IFX_TAPI_T38_MOD_DATA_t param;
     IFX_int32_t fd;

     memset (&param, 0, sizeof(IFX_TAPI_T38_MOD_DATA_t));
     // set V.21 standard
     param.nStandard = 0x01;
     ioctl(fd, IFX_TAPI_T38_MOD_START, &param);
   \endcode */
#define  IFX_TAPI_T38_MOD_START              _IOW(IFX_TAPI_IOC_MAGIC, 0x1A, int)

/** This service configures and enables the demodulator for a T.38 fax session.
   The file descriptor is applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_T38_DEMOD_DATA_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This service deactivates the voice path and configures the driver
    for a fax session.

   \code
     IFX_TAPI_T38_DEMOD_DATA_t param;
     IFX_int32_t fd;
     memset (&param, 0, sizeof(IFX_TAPI_T38_DEMOD_DATA_t));
     // set V.21 standard as standard used for fax
     param.nStandard1 = 0x01;
     // set V.17/14400 as alternative standard
     param.nStandard2 = 0x09;
     ioctl(fd, IFX_TAPI_T38_DEMOD_START, &param);
   \endcode */
#define  IFX_TAPI_T38_DEMOD_START            _IOW(IFX_TAPI_IOC_MAGIC, 0x1B, int)

/** This service disables the T.38 fax data pump and activates the voice
    path again. The file descriptor is applicable to data channel descriptors.

   \param IFX_int32_t This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
     IFX_TAPI_LINE_VOLUME_t param;
     IFX_int32_t fd;

     ioctl(fd, IFX_TAPI_T38_STOP, 0);
   \endcode */
#define  IFX_TAPI_T38_STOP                   _IOW(IFX_TAPI_IOC_MAGIC, 0x1C, int)

/** This service provides the T.38 fax status on query. The file descriptor is
    applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_T38_STATUS_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks This interface must be used when a fax exception has occurred to
   find out the status.

   \code
     IFX_TAPI_T38_STATUS_t param;
     IFX_int32_t fd;

     memset (&param, 0, sizeof(IFX_TAPI_T38_STATUS_t));
     // request status
     ioctl(fd, IFX_TAPI_T38_STATUS_GET, &param);
   \endcode */
#define  IFX_TAPI_T38_STATUS_GET             _IOR(IFX_TAPI_IOC_MAGIC, 0x1D, int)
/*@}*/ /* TAPI_INTERFACE_FAX */

/* ======================================================================= */
/* TAPI Test Services, ioctl commands (Group TAPI_INTERFACE_TEST)          */
/* ======================================================================= */

/** \addtogroup TAPI_INTERFACE_TEST */
/*@{*/
/** Forces generation of on-/off-hook. The file descriptor is applicable to data
    channel descriptors.

   \param IFX_int32_t The parameter defines off hook or on hook generation.
          0 - IFX_FALSE Generate on hook.
          1 - IFX_TRUE Generate off hook.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \remarks After switching the hook state, the hook event gets to the hook
    state machine for validation. Depending on the timing of calling this
    interface hook flash and pulse dialing can be verified. The example
   shows the generation of a flash hook with a timing of 100 ms. The flash hook
   can be queried afterwards by the \ref IFX_TAPI_EVENT_GET ioctl.

   \code
   // generate on hook
   ret = ioctl(fd, IFX_TAPI_TEST_HOOKGEN, 0);
   // generate off hook for 100 ms
   ret = ioctl(fd, IFX_TAPI_TEST_HOOKGEN, 1);
   sleep (100);
   ret = ioctl(fd, IFX_TAPI_TEST_HOOKGEN, 0);
   \endcode */
#define  IFX_TAPI_TEST_HOOKGEN           _IOW(IFX_TAPI_IOC_MAGIC, 0x3E, int)

/** Enables a local test loop in the analog part. The digital voice data is
   transparently looped back to the network without affecting the downstream
   transmission. The reception of local voice (upstream) is disabled.
   That means,that voice applied to the local phone is not beeing processed,
   but data sent to the phone can still be heard. The file descriptor is
   applicable to data channel descriptors.

   \param IFX_int32_t The parameter points to a
   \ref IFX_TAPI_TEST_LOOP_t structure.

   \return Returns value as follows:
     - \ref IFX_SUCCESS: if successful
     - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_TEST_LOOP_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_TEST_LOOP_t));
   param.bAnalog = 1;
   ret = ioctl(fd, IFX_TAPI_TEST_LOOP, &param);
   \endcode */
#define  IFX_TAPI_TEST_LOOP                  _IOW(IFX_TAPI_IOC_MAGIC, 0x3F, int)

/*@}*/ /* TAPI_INTERFACE_TEST */


/** \todo uncatalogued ioctl command */
#define  PHONE_PSTN_SET_STATE                _IOW(IFX_TAPI_IOC_MAGIC, 0xA4, int)
/** \todo uncatalogued ioctl command */
#define  PHONE_PSTN_GET_STATE                _IO (IFX_TAPI_IOC_MAGIC, 0xA5)
/** \todo uncatalogued ioctl command */
#define  PHONE_WINK_DURATION                 _IOW(IFX_TAPI_IOC_MAGIC, 0xA6, int)
/** \todo uncatalogued ioctl command */
#define  PHONE_WINK                          _IOW(IFX_TAPI_IOC_MAGIC, 0xAA, int)
/** \todo uncatalogued ioctl command */
#define  PHONE_QUERY_CODEC                   _IO (IFX_TAPI_IOC_MAGIC, 0xA7)
/** \todo uncatalogued ioctl command */
#define  PHONE_PSTN_LINETEST                 _IO (IFX_TAPI_IOC_MAGIC, 0xA8)

/* ===================================================================== */
/* TAPI Event Services, ioctl commands (Group TAPI_INTERFACE_EVENT)      */
/* ===================================================================== */

/** \addtogroup TAPI_INTERFACE_EVENT */
/*@{*/

/** IFX_TAPI_EVENT_GET always returns \ref IFX_SUCCESS as long as the channel
   parameter is not out of range, or if no event was available.
   If the parameter is out of range then \ref IFX_ERROR is returned.
   To find that the event data is not valid in this case the value
   \ref IFX_TAPI_EVENT_NONE is returned in the "id" field of the event.
   This ioctl indicates that additional events are be ready to be retrieved.
   The information is provided in the "more" field of the returned
    \ref IFX_TAPI_EVENT_t structure. The file descriptor is a device type.

   \param IFX_int32_t Pointer to a \ref IFX_TAPI_EVENT_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_EVENT_GET                   _IO(IFX_TAPI_IOC_MAGIC, 0xC0)


/** Enables detection of an event. The file descriptor is a device type.

   \param IFX_int32_t Pointer to a \ref IFX_TAPI_EVENT_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_EVENT_ENABLE                _IO(IFX_TAPI_IOC_MAGIC, 0xC1)

/** Disables detection of an event. The file descriptor is a device type.

   \param IFX_int32_t Pointer to a \ref IFX_TAPI_EVENT_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_EVENT_DISABLE               _IO(IFX_TAPI_IOC_MAGIC, 0xC2)
/*@}*/ /* TAPI_INTERFACE_EVENT */

/* ======================================================================== */
/* Polling Services, ioctl commands (Group TAPI_POLLING_SERVICE)            */
/* ======================================================================== */
/** \addtogroup TAPI_POLLING_SERVICE */
/*@{*/

/**   This service switches the driver between polling and interrupt
      mode and vice versa.

   \param    pDrvCtrl - handle to IFX_TAPI_POLL_CONFIG_t data type.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \remarks
      This function will be called only once with any device file descriptor
      to configure all adequately registered LL devices into polling/interrupt
      mode according to the configuration provided.

      In case of switching to packets polling mode (bPollPkts = IFX_TRUE)
      all packet related interrupts are disabled.
      In case of switching to events polling mode (bPollEvts = IFX_TRUE)
      all events related interrupts are disabled.
      In case of switching both to packets and events polling mode
      then all device interrupts are disabled.
      In case of switching back to interrupt mode either for packets or
      events, the reverse applies.
      The packets polling/interrupt configuration shall be applied only to
      those devices that previously have been registered using the
      IFX_TAPI_POLL_PKTS_ADD ioctl, otherwise the default configuration will
      apply which is interrupt mode.
      Also, the events polling/interrupt configuration shall be applied only
      to devices that have previously been registered using the
      IFX_TAPI_POLL_EVTS_ADD ioctl, otherwise the default configuration will
      apply which is interrupt mode.

   \code
   IFX_TAPI_POLL_CONFIG_t DrvCfg;
   int ret, pkt_len;

   DrvCfg.bPollPkts = IFX_TRUE;
   // configure driver for polling of TAPI events
   DrvCfg.bPollEvts = IFX_TRUE;
   // initialize a buffer pool of 64 buffers for polled packet streaming
   pkt_len = (sizeof(PACKET) + 3) & ~3;
   // initialise a buffer poll and provide the pointer of the buffer poll
   //  control data structure
   DrvCfg.pBufferPool = (void *)bufferPoolInit (pkt_len, 64, 0);
   // register a routine to get free buffers from the buffer pool
   DrvCfg.getBuf = (void * (*) (void *))bufferPoolGet;
   // register a routine to put used buffers to the buffer pool
   DrvCfg.putBuf = bufferPoolPut;

   ret = ioctl (fd, IFX_TAPI_POLL_CONFIG, (int)&DrvCfg);
   \endcode
*/
#define IFX_TAPI_POLL_CONFIG                 _IOW(IFX_TAPI_IOC_MAGIC, 0xD0, int)

/** Used to register a TAPI device for polling. The file descriptor is
    applicable to device file descriptors.

\param IFX_int32_t The parameter is not required.

\return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_POLL_DEV_ADD                _IOW(IFX_TAPI_IOC_MAGIC, 0xD1, int)

/** Used to unregister a TAPI device for polling. The file descriptor is
    applicable to device file descriptors.

\param IFX_int32_t The parameter is not required.

\return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_POLL_DEV_REM                _IOW(IFX_TAPI_IOC_MAGIC, 0xD2, int)


/** Used for writing packets to TAPI devices registered for packets polling.

\param IFX_int32_t Pointer to \ref IFX_TAPI_POLL_DATA_t structure.

\return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_POLL_WRITE                  _IOW(IFX_TAPI_IOC_MAGIC, 0xD3, int)


/** Used for reading packets to TAPI devices registered for packets polling.

\param IFX_int32_t Pointer to \ref IFX_TAPI_POLL_PKT_t structure.

\return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_POLL_READ                   _IOW(IFX_TAPI_IOC_MAGIC, 0xD4, int)


/** Used to read interrupts from all polled devices,
   the interrupts will be queued inside TAPI.

\param IFX_int32_t The parameter is not required.

\return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_POLL_EVENTS                 _IOW(IFX_TAPI_IOC_MAGIC, 0xD5, int)
/*@}*/ /* TAPI_POLLING_SERVICE */

/* ======================================================================== */
/* TAPI AUDIO  Services, ioctl commands (Group TAPI_INTERFACE_AUDIO)        */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_AUDIO */
/*@{*/

/** This service sets  the Inputs and Outputs of the Analog Frontent (AFE)
    for Handset, Headset and Handsfree Mode.

   \param Pointer to \ref IFX_TAPI_AUDIO_AFE_CFG_SET_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error

   \remarks
   For a description of the AFE and supported pins please refer to the
   Hardware User's Manual.

   \code
   IFX_TAPI_AUDIO_AFE_CFG_SET_t param;
   memset (&param, 0, sizeof(IFX_TAPI_AUDIO_AFE_CFG_SET_t));
   // Handsfree Microphone.
   param.nHFMic = IFX_TAPI_AUDIO_AFE_PIN_MIC3;
   // Handsfree Output.
   param.nHFOut;
   // Handset Microphone
   param.nHNMic = IFX_TAPI_AUDIO_AFE_PIN_MIC2;
   // Handset Output
   param.nHNOut;
   // Headset Microphone
   param.nHDMic = IFX_TAPI_AUDIO_AFE_PIN_MIC1;
   // Headset Output
   param.nHDOut;
   ioctl(fd, IFX_TAPI_AUDIO_AFE_CFG_SET, (int)&param);
   \endcode
*/
#define  IFX_TAPI_AUDIO_AFE_CFG_SET                          _IO(IFX_TAPI_IOC_MAGIC, 0x69)

/** Selects the volume level for the audio channel. The file descriptor
    is applicable to channel file descriptors containing the audio channel.

   \param IFX_int32_t Volume level (1...8).

   \return Returns value as follows:
      - \ref IFX_SUCCESS: if successful
      - \ref IFX_ERROR: in case of an error

   \code
   int level=4;
   ioctl(fd, IFX_TAPI_AUDIO_VOLUME_SET, level);
   \endcode
*/
#define  IFX_TAPI_AUDIO_VOLUME_SET                          _IO(IFX_TAPI_IOC_MAGIC, 0x70)

/** This service sets the operation mode for audio channel. The file descriptor
    is applicable to channel file descriptors containing the audio channel.

   \param IFX_int32_t The parameter is the audio mode to be set.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_AUDIO_MODE_t mode = IFX_TAPI_AUDIO_MODE_HEADSET;
   ioctl(fd, IFX_TAPI_AUDIO_MODE_SET, mode);
   \endcode
*/
#define  IFX_TAPI_AUDIO_MODE_SET                            _IO(IFX_TAPI_IOC_MAGIC, 0x71)
/** Mutes the audio channel. The file descriptor is applicable to channel file
    descriptors containing the audio channel.

   \param IFX_operation_t The parameter specifies whether to enable
      or disable mute.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_AUDIO_MUTE_SET                            _IO(IFX_TAPI_IOC_MAGIC, 0x72)

/** Enables/disables in-call announcement. The file descriptor is applicable to
   channel file descriptors containing the audio channel.

   \param IFX_int32_t The parameter specifies whether to enable or
   disabling in-call announcement, parameter to be
   selected from \ref IFX_TAPI_AUDIO_ICA_t.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_AUDIO_ICA_SET                             _IO(IFX_TAPI_IOC_MAGIC, 0x73)


/** Starts ringing on the audio channel. The file descriptor is applicable to
   channel file descriptors containing the audio channel.

   \param IFX_int32_t Tone table index containing the ring cadence.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_AUDIO_RING_START                          _IO(IFX_TAPI_IOC_MAGIC, 0x74)

/** Stops ringing on the audio channel. The file descriptor is applicable to
   channel file descriptors containing the audio channel.

   \param IFX_int32_t Not required.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_AUDIO_RING_STOP                           _IO(IFX_TAPI_IOC_MAGIC, 0x75)
/** This service sets volume for ringing on audio channel. The file descriptor
    is applicable to channel file descriptors containing the audio channel.

   \param IFX_int32_t Ringing volume level (1...8).

   \return Returns value as follows:
      - \ref IFX_SUCCESS: if successful
      - \ref IFX_ERROR: in case of an error

   \code
   int level=4;
   ioctl(fd, IFX_TAPI_AUDIO_RING_VOLUME_SET, level);
   \endcode
*/

#define  IFX_TAPI_AUDIO_RING_VOLUME_SET                     _IO(IFX_TAPI_IOC_MAGIC, 0x79)


/** Selects the room type. The file descriptor is applicable to channel file
    descriptors containing the audio channel.

   \param IFX_int32_t The parameter specifies the room type, to be
   selected from \ref IFX_TAPI_AUDIO_ROOM_TYPE_t.

  \return Returns value as follows:
      - \ref IFX_SUCCESS: if successful
      - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_AUDIO_ROOM_TYPE_SET                       _IO(IFX_TAPI_IOC_MAGIC, 0x76)

/** This service sets the mode for both loop/diagnostic ports.

   \param Pointer to \ref IFX_TAPI_AUDIO_TEST_MODE_t type,
          nTestPort0 can be DISABLED, DIAGNOSTIC or LOOP
          nTestPort1 can be DISABLED, DIAGNOSTIC or LOOP
          DISABLED means no data is routed through port
          DIAGNOSTIC stands for data coming from ADCx and to DACx is routed to
          DIAGx_IN and DIAGx_OUT.
          LOOP means that ADCx/DACx is powered down and data is directed to
          appropriate LOOP inout and output signals.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_AUDIO_TEST_SET              _IOW(IFX_TAPI_IOC_MAGIC, 0xE3, int)

/*@}*/ /* TAPI_INTERFACE_AUDIO */

/* ========================================================================== */
/* TAPI DTMF  Services for External keypad, ioctl commands                    */
/* (Group TAPI_INTERFACE_EVENT)                                               */
/* ========================================================================== */
/** \addtogroup TAPI_INTERFACE_EVENT */
/*@{*/


/** This service is used to signal a DTMF event from an external software
   module. It means that an external software communicated to TAPI that a
   DTMD digit has been detected. The file descriptor is a device type.

   \param IFX_int32_t Pointer to a \ref IFX_TAPI_EVENT_EXT_DTMF_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_EVENT_EXT_DTMF                   IFX_TAPI_PKT_EV_GENERATE


/** Configures the support of external DTMF event signaled to TAPI. The file
   descriptor is a device type.

   \param IFX_int32_t Pointer to a \ref IFX_TAPI_EVENT_EXT_DTMF_CFG_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define  IFX_TAPI_EVENT_EXT_DTMF_CFG               IFX_TAPI_PKT_EV_GENERATE_CFG

/*@}*/ /* TAPI_INTERFACE_EVENT */

/* ======================================================================= */
/* TAPI FXO Services, ioctl commands (Group TAPI_INTERFACE_FXO  )          */
/* ======================================================================= */

/** \addtogroup TAPI_INTERFACE_FXO */
/*@{*/

/** Configuration for DTMF dialing. Mainly used on FXO lines but can be also
 used on analog lines. The file descriptor is applicable to data channel file
 descriptors.

   \param a Pointer to \ref IFX_TAPI_FXO_DIAL_CFG_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_DIAL_CFG_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0xD6, int)
/** Configuration of the fxo hook. The file descriptor is applicable to data
   channel file descriptors.

   \param Pointer  to \ref IFX_TAPI_FXO_FLASH_CFG_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_FLASH_CFG_SET           _IOW(IFX_TAPI_IOC_MAGIC, 0xD7, int)
/** Configuration of OSI timing. The file descriptor is applicable to data
    channel file descriptors.

   \param Pointer to \ref IFX_TAPI_FXO_OSI_CFG_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_OSI_CFG_SET             _IOW(IFX_TAPI_IOC_MAGIC, 0xD8, int)
/** Dials DTMF digits. The file descriptor is applicable to data channel file
    descriptors.

   \param Pointer  to \ref IFX_TAPI_FXO_DIAL_t structure.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_DIAL_START              _IOW(IFX_TAPI_IOC_MAGIC, 0xD9, int)
/** Stops dialing digits on FXO interface. The file descriptor is applicable to
    data channel file descriptors.

   \param The parameter is not required.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_DIAL_STOP               _IOW(IFX_TAPI_IOC_MAGIC, 0xDA, int)
/** Issues on-/off-hook in the fxo interface. The file descriptor is applicable
    to data channel file descriptors.

   \param Select hook on-/off-hook from \ref IFX_TAPI_FXO_HOOK_t.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_HOOK_SET                _IOW(IFX_TAPI_IOC_MAGIC, 0xDB, int)
/** Issues flash-hook in the FXO interface. The file descriptor is applicable
   to data channel file descriptors.

   \param The parameter is not required.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_FLASH_SET               _IOW(IFX_TAPI_IOC_MAGIC, 0xDC, int)
/** Receives battery status from the FXO interface. The file descriptor is
    applicable to data channel file descriptors.

   \param Pointer to \ref IFX_boolean_t type, indicating the battery status
       - IFX_TRUE if the FXO port is disconnected from the PSTN (battery absent).
       - IFX_FALSE if the FXO port is connected to the PSTN (battery present).

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_BAT_GET                 _IOW(IFX_TAPI_IOC_MAGIC, 0xDD, int)
/** This service retrieves the current hook state on a FXO channel (set by the
 application itself).

   \param Pointer to \ref IFX_boolean_t type

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_HOOK_GET                _IOW(IFX_TAPI_IOC_MAGIC, 0xDE, int)
/** Retrieves APOH (another phone off-hook) status of the fxo interface.
   The file descriptor is applicable to phone channel file descriptors containing
   an analog interface.

   \param Pointer to \ref IFX_boolean_t type,  indicating APOH status.
      - IFX_TRUE if APOH condition is verified.
      - IFX_FALSE otherwise.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error

*/
#define IFX_TAPI_FXO_APOH_GET                _IOW(IFX_TAPI_IOC_MAGIC, 0xDF, int)

/** Receives ring status from the FXO interface. The file descriptor is
    applicable to data channel file descriptors.

   \param Pointer to \ref IFX_boolean_t type,indicating the ringing status
    of the FXO line.
       - IFX_TRUE the line is ringing.
       - IFX_FALSE the line is not ringing.

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error

*/
#define IFX_TAPI_FXO_RING_GET                _IOW(IFX_TAPI_IOC_MAGIC, 0xE0, int)

/** Receives polarity status from the FXO interface. The file descriptor is
    applicable to data channel file descriptors.

   \param Pointer to \ref IFX_boolean_t type,
         - IFX_TRUE reflects normal polarity,
         - IFX_FALSE reflects reversed polarity

   \return Returns value as follows:
    - \ref IFX_SUCCESS: if successful
    - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_FXO_POLARITY_GET            _IOW(IFX_TAPI_IOC_MAGIC, 0xE1, int)

#ifndef TAPI_DXY_DOC
/** Used for TAPI POLL testing. */
#define IFX_TAPI_POLL_TEST                   _IOW(IFX_TAPI_IOC_MAGIC, 0xE2, int)
#endif /* TAPI_DXY_DOC */

/** maximum number of DTMF digits to be dialed with a single TAPI ioctl
    used in \ref IFX_TAPI_FXO_DIAL_START. */
#define IFX_TAPI_FXO_DIAL_DIGITS             30
/*@}*/ /* TAPI_INTERFACE_FXO */

/* ======================================================================= */
/* TAPI DECT Services, ioctl commands (Group TAPI_INTERFACE_DECT)          */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_DECT */
/*@{*/
/** This service activates/decativates the DECT channel. The file descriptor is
 applicable to file descriptors containing a DECT channel.

   \param value from \ref IFX_operation_t specifies activation or deactivation

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_DECT_ACTIVATION_SET         _IOW(IFX_TAPI_IOC_MAGIC, 0x49, int)
/** This service configures the DECT channel. The file descriptor is applicable
    to file descriptors containing a DECT channel.

   \param Pointer to \ref IFX_TAPI_DECT_CFG_t structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_DECT_CFG_SET                _IOW(IFX_TAPI_IOC_MAGIC, 0x4A, int)
/** This service selects DECT encoding and packetisation time. The file
    descriptor is applicable to file descriptors containing a DECT channel.

   \param Pointer to \ref IFX_TAPI_DECT_ENC_CFG_t structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_DECT_ENC_CFG_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0x4B, int)
/** This service sets the DECT channel encoder/decoder volume. The file
    descriptor is applicable to file descriptors containing a DECT channel.

   \param IFX_int32_t The parameter points to a \ref IFX_TAPI_LINE_VOLUME_t
    structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error

   \code
   IFX_TAPI_LINE_VOLUME_t param;
   IFX_int32_t fd;

   memset (&param, 0, sizeof(IFX_TAPI_LINE_VOLUME_t));
   param.nTx = 3;
   param.nRx = 0;
   ioctl(fd, IFX_TAPI_DECT_VOLUME_SET, &param);
   \endcode
*/
#define  IFX_TAPI_DECT_VOLUME_SET            _IOW(IFX_TAPI_IOC_MAGIC, 0x52, int)
/** This service retrives the DECT statistics. The file descriptor is applicable
    to file descriptors containing a DECT channel.

   \param Pointer to \ref IFX_TAPI_DECT_STATISTICS_t structure.

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_DECT_STATISTICS_GET         _IOW(IFX_TAPI_IOC_MAGIC, 0x4C, int)
/** This service plays a tone on the DECT channel. The file descriptor is
    applicable to descriptors containing a DECT channel.

   \param tone table index

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_TONE_DECT_PLAY              _IOW(IFX_TAPI_IOC_MAGIC, 0x4D, int)
/** This service stops playing a tone on the DECT channel. The file descriptor is
    applicable to descriptors containing a DECT channel.

   \param unused - should be set to 0

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_TONE_DECT_STOP              _IOW(IFX_TAPI_IOC_MAGIC, 0x4E, int)
/** This service configures usage of the echo suppressor. The file descriptor is
    applicable to phone channel file descriptors containing an analog channel.

   \param value from \ref IFX_operation_t specifies activation or deactivation

   \return Returns value as follows:
   - \ref IFX_SUCCESS: if successful
   - \ref IFX_ERROR: in case of an error
*/
#define IFX_TAPI_PHONE_ES_SET                _IOW(IFX_TAPI_IOC_MAGIC, 0x4F, int)
/*@}*/ /* TAPI_INTERFACE_DECT */


/* ========================================================================= */
/*                     TAPI Interface Constants                              */
/* ========================================================================= */

/* ======================================================================== */
/* TAPI Initialization Services, constants (Group TAPI_INTERFACE_INIT)      */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_INIT */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_INIT */

/* ======================================================================= */
/* TAPI Operation Control Services, constants (Group TAPI_INTERFACE_OP)    */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_OP */
/*@{*/

/** TAPI phone volume control */
/** Switches the volume to low, -12 dB. */
#define IFX_TAPI_LINE_VOLUME_LOW                (-12)
/** Switches the volume to medium, -6 dB. */
#define IFX_TAPI_LINE_VOLUME_MEDIUM              (-6)
/** Switches the volume to high, 0 dB */
#define IFX_TAPI_LINE_VOLUME_HIGH                 (0)
/** Switches the volume to minimum gain, -24 dB, note that DTMF detection etc.
    Might not work properly on such low signgals. */
#define IFX_TAPI_LINE_VOLUME_MIN_GAIN           (-24)
/** Switches the volume to maximum gain, +24 dB. */
#define IFX_TAPI_LINE_VOLUME_MAX_GAIN            (24)

/** TAPI Lec control */

/** LEC delay line maximum length. */
#define IFX_TAPI_LEC_LEN_MAX                      (16)
/** LEC delay line minimum length. */
#define IFX_TAPI_LEC_LEN_MIN                      (4)

/*@}*/ /* TAPI_INTERFACE_OP */

/* ================================================================ */
/* TAPI Metering Services, constants (Group TAPI_INTERFACE_METER)   */
/* ================================================================ */
/** \addtogroup TAPI_INTERFACE_METER */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_METER */

/* ====================================================================== */
/* TAPI Tone Services, constants (Group TAPI_INTERFACE_TONE)              */
/* ====================================================================== */
/** \addtogroup TAPI_INTERFACE_TONE */
/*@{*/

/** Maximum number of simple tones which can be played in one go. */
#define IFX_TAPI_TONE_SIMPLE_MAX                  (7)

/** Maximum tone generation steps, also called cadences. */
#define IFX_TAPI_TONE_STEPS_MAX                   (6)

/** Tone minimum index which can be configured by user. */
#define IFX_TAPI_TONE_INDEX_MIN                  (32)

/** Tone maximum index which can be configured by user. */
#define IFX_TAPI_TONE_INDEX_MAX                 (255)

/*@}*/ /* TAPI_INTERFACE_TONE */

/* ===================================================================== */
/* TAPI Misc Services, constants (Group TAPI_INTERFACE_MISC)             */
/* ===================================================================== */
/** \addtogroup TAPI_INTERFACE_MISC */
/*@{*/

/** Used to report events from any channel in the device. This constant is also
   used to report events that can not be associated to a particular channel.*/
#define IFX_TAPI_EVENT_ALL_CHANNELS               0xffff
/*@}*/ /* TAPI_INTERFACE_MISC */

/* =================================================================== */
/* TAPI Signal Detection Services, constants                           */
/* (Group TAPI_INTERFACE_SIGNAL)                                       */
/* =================================================================== */
/** \addtogroup TAPI_INTERFACE_SIGNAL */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_SIGNAL */

/* ===================================================================== */
/* TAPI CID Features Service, constants (Group TAPI_INTERFACE_CID)         */
/* ===================================================================== */
/** \addtogroup TAPI_INTERFACE_CID */
/*@{*/

/** CID Rx Fifo Size. */
#define IFX_TAPI_CID_RX_FIFO_SIZE                (10)

/** CID Rx size of one data buffer.

    \remarks Larger data is automatically split into multiple buffers until
             there is no further data or the fifo defined above is full.
             The data can then be retrived by multiple calls to
             \ref IFX_TAPI_CID_RX_DATA_GET and the application should
             concatenate it before interpreting the data. */
#define IFX_TAPI_CID_RX_SIZE_MAX                (128)

/**
   CID Tx maximum buffer size.

   \remarks
   -  ETSI  :
      call setup cmd : 2,  cli : 22, date/time : 10, name : 52,
      redir num : 22, checksum : 1 => 109 Bytes max in CID buffer
   -  NTT :
      DLE : 3, SOH : 1, Header : 1, STX : 1, ETX : 1, DATA: 119, CRC : 2
      => 128 Bytes max in CID Buffer
   - ETSI SMS (protocol 2):
      type 1, length 1, data 1-255, crc 1 => 258 byte
*/
#define IFX_TAPI_CID_TX_SIZE_MAX                (258)

/** Maximum allowed length of one CID message element (in characters). */
#define IFX_TAPI_CID_MSG_LEN_MAX                 (50)

/*@}*/ /* TAPI_INTERFACE_CID */

#ifndef TAPI_DXY_DOC
/* =================================================================== */
/* TAPI Connection Services, constants (Group TAPI_INTERFACE_CON)      */
/* =================================================================== */
/** \addtogroup TAPI_INTERFACE_CON */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_CON */

/* ======================================================================= */
/* TAPI Miscellaneous Services, constants (Group TAPI_INTERFACE_MISC)      */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_MISC */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_MISC */

#endif /* TAPI_DXY_DOC */
/* ======================================================================= */
/* TAPI Power Ringing Services, constants (Group TAPI_INTERFACE_RINGING)         */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_RINGING */
/*@{*/

/** Maximum number of cadence bytes. */
#define IFX_TAPI_RING_CADENCE_MAX_BYTES                 (40)

/*@}*/ /* TAPI_INTERFACE_RINGING */

/* ======================================================================= */
/* TAPI PCM Services, constants (Group TAPI_INTERFACE_PCM)                 */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_PCM */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_PCM */

/* ======================================================================= */
/* TAPI Fax T.38 Services, constants (Group TAPI_INTERFACE_FAX)            */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_FAX */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_FAX */

/* ========================================================================= */
/*                      TAPI Interface Enumerations                          */
/* ========================================================================= */


/* ======================================================================== */
/* TAPI Initialization Services, enumerations (Group TAPI_INTERFACE_INIT)   */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_INIT */
/*@{*/

/** TAPI Initialization modes, selection for target system. If different modes
    are supported by the implementation, this parameter specifies which mode
    should be set up. The meaning of the mode is dependent on the implementation.*/
typedef enum
{
   /** Default initialization. */
   IFX_TAPI_INIT_MODE_DEFAULT = 0,
   /** Typical VoIP solution. Phone connected to a packet coder (data channel)
    with DSP features for signal detection. */
   IFX_TAPI_INIT_MODE_VOICE_CODER = 1,
   /** Phone to PCM using DSP features for signal detection. */
   IFX_TAPI_INIT_MODE_PCM_DSP = 2,
   /** Phone to PCM not using DSP features for signal detection. */
   IFX_TAPI_INIT_MODE_PCM_PHONE = 3,
   /** Phone to PCM connection without DSP features. */
   IFX_TAPI_INIT_MODE_NONE = 0xff
} IFX_TAPI_INIT_MODE_t;

/** Country selection for \ref IFX_TAPI_CH_INIT_t. If different countries are
    supported by the implementation, this parameter specifies which one.
    \remarks For future purposes, Currently not used. */
typedef enum
{
   /** Default contry. */
   IFX_TAPI_INIT_COUNTRY_DEFAULT = 0,
   /** Germany. */
   IFX_TAPI_INIT_COUNTRY_DE = 1,
   /** USA. */
   IFX_TAPI_INIT_COUNTRY_US = 2,
   /** United Kingdom. */
   IFX_TAPI_INIT_COUNTRY_UK = 3
} IFX_TAPI_INIT_COUNTRY_t;

/*@}*/ /* TAPI_INTERFACE_INIT */

/* ======================================================================== */
/* TAPI Operation Control Services, enumerations (Group TAPI_INTERFACE_OP)  */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_OP */
/*@{*/

/** Defines for linefeeding. */
typedef enum
{
   /** Normal feeding mode for phone off hook. */
   IFX_TAPI_LINE_FEED_ACTIVE = 0,
   /** Normal feeding mode for phone off hook reversed. */
   IFX_TAPI_LINE_FEED_ACTIVE_REV = 1,
   /** Powers down resistance = on hook with hook detection. */
   IFX_TAPI_LINE_FEED_STANDBY = 2,
   /** Switches off the line, but the device is able to test the line,
       not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_HIGH_IMPEDANCE = 3,
   /** Switches off the line and the device. */
   IFX_TAPI_LINE_FEED_DISABLED = 4,
   /** Not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_GROUND_START = 5,
   /** Thresholds for automatic battery switch are set via coefficient settings,
    not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_NORMAL_AUTO = 6,
   /** Thresholds for automatic battery switch are set via coefficient
   settings reversed, not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_REVERSED_AUTO = 7,
   /** Feeding mode for phone off hook with low battery to save power. */
   IFX_TAPI_LINE_FEED_NORMAL_LOW = 8,
   /** feeding mode for phone off hook with low battery to save power
       and reserved polarity. */
   IFX_TAPI_LINE_FEED_REVERSED_LOW = 9,
   /** Reserved, needed for ring call back function. */
   IFX_TAPI_LINE_FEED_RING_BURST = 10,
   /** Reserved. Needed for ring call back function. */
   IFX_TAPI_LINE_FEED_RING_PAUSE = 11,
   /** Reserved. Needed for internal function, not supported for VINETIC 2CPE,
    Revision 2.1.  */
   IFX_TAPI_LINE_FEED_METER = 12,
   /** Reserved. Special test linemode, not supported for VINETIC 2CPE,
    Revision 2.1 .*/
   IFX_TAPI_LINE_FEED_ACTIVE_LOW = 13,
   /** Reserved. Special test linemode, not supported for VINETIC 2CPE,
    Revision 2.1. */
   IFX_TAPI_LINE_FEED_ACTIVE_BOOSTED = 14,
   /** Reserved. Special linemode for S-MAX Slic, not supported for VINETIC 2CPE,
    Revision 2.1. */
   IFX_TAPI_LINE_FEED_ACT_TESTIN = 15,
   /** Reserved. Special linemode for S-MAX Slic,
       not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_DISABLED_RESISTIVE_SWITCH = 16,
   /** Power down resistance = on hook with hook detection,
       not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_PARKED_REVERSED = 17,
   /** High resistive feeding mode with offhook sensing ability and
       normal polarity. */
   IFX_TAPI_LINE_FEED_ACTIVE_RES_NORMAL = 18,
   /** High resistive feeding mode with offhook sensing ability and
       reversed polarity. */
   IFX_TAPI_LINE_FEED_ACTIVE_RES_REVERSED = 19,
   /** Reserved, special linemode for SLIC LCP,
       not supported for VINETIC 2CPE, Revision 2.1. */
   IFX_TAPI_LINE_FEED_ACT_TEST = 20
} IFX_TAPI_LINE_FEED_t;


/** Defines for line mode type. */
typedef enum
{
   /** Wrong line mode type for analog channel. */
   IFX_TAPI_LINE_TYPE_UNKNOWN = -1,
   /** Line mode type FXS narrowband sampling for analog channel. */
   IFX_TAPI_LINE_TYPE_FXS_NB = 0,
   /** Line mode type FXS wideband sampling for analog channel. */
   IFX_TAPI_LINE_TYPE_FXS_WB = 1,
   /** Line mode type FXS automatic NB/WB switching for analog channel. */
   IFX_TAPI_LINE_TYPE_FXS_AUTO = 2,
   /** Line mode type FXO narrowband sampling for analog channel. */
   IFX_TAPI_LINE_TYPE_FXO_NB = 3
} IFX_TAPI_LINE_TYPE_t;

/* map the old names to the NB names */
#define IFX_TAPI_LINE_TYPE_FXS  IFX_TAPI_LINE_TYPE_FXS_NB
#define IFX_TAPI_LINE_TYPE_FXO  IFX_TAPI_LINE_TYPE_FXO_NB

/** Line type configuration \ref IFX_TAPI_LINE_TYPE_SET. */
typedef struct
{
   /** Configures the line type of this analog channel. */
   IFX_TAPI_LINE_TYPE_t    lineType;
   /** corresponding index of the DAA channel defined in drv_daa
       (board specific). */
   IFX_uint8_t             nDaaCh;
} IFX_TAPI_LINE_TYPE_CFG_t;

/** LEC gain levels */
typedef enum
{
   /** Turns LEC off. */
   IFX_TAPI_LEC_GAIN_OFF = 0,
   /** Turns LEC on to low level.*/
   IFX_TAPI_LEC_GAIN_LOW = 1,
   /** Turns LEC on to normal level.*/
   IFX_TAPI_LEC_GAIN_MEDIUM = 2,
   /** Turns LEC on to high level.*/
   IFX_TAPI_LEC_GAIN_HIGH = 3
} IFX_TAPI_LEC_GAIN_t;

/** LEC Type Configuration. */
typedef enum
{
   /** No LEC, turn off. */
   IFX_TAPI_WLEC_TYPE_OFF = 0x00,
   /** LEC Type is NLEC. */
   IFX_TAPI_WLEC_TYPE_NE  = 0x01,
   /** LEC Type is WLEC. */
   IFX_TAPI_WLEC_TYPE_NFE = 0x02
} IFX_TAPI_WLEC_TYPE_t;

/** LEC window size configuration. */
typedef enum
{
   /** LEC window size 4 ms .*/
   IFX_TAPI_WLEN_WSIZE_4 = 4,
   /** LEC window size 6 ms. */
   IFX_TAPI_WLEN_WSIZE_6 = 6,
   /** LEC window size 8 ms. */
   IFX_TAPI_WLEN_WSIZE_8 = 8,
   /** LEC window size 16 ms. */
   IFX_TAPI_WLEN_WSIZE_16 = 16
} IFX_TAPI_WLEC_WIN_SIZE_t;

/** Validation types used for \ref IFX_TAPI_LINE_HOOK_VT_t structure.

   \remarks
   The default values are as follows:

   - 80 ms  <= flash time      <= 200 ms
   - 30 ms  <= digit low time  <= 80 ms
   - 30 ms  <= digit high time <= 80 ms
   - interdigit time =     300 ms
   - off hook time   =      40 ms
   - on hook time    =     400 ms
   !!! open: only min time is validated and pre initialized */
typedef enum
{
   /** Settings for hook validation, if the time matches between nMinTime and
      nMaxTime an exception is raised. */
   IFX_TAPI_LINE_HOOK_VT_HOOKOFF_TIME     = 0x0,
   /** Settings for hook validation, if the time matches between nMinTime and
       nMaxTime an exception is raised. */
   IFX_TAPI_LINE_HOOK_VT_HOOKON_TIME      = 0x1,
   /** Settings for hook flash validation also known as register recall.
       If the time matches between the time defined in the fields nMinTime
       and nMaxTime an exception is raised. */
   IFX_TAPI_LINE_HOOK_VT_HOOKFLASH_TIME   = 0x2,
   /** Settings for pulse digit low, open loop and make validation.
       The time must match between the time defined in the fields nMinTime and
       nMaxTime to recognize it as pulse dialing event */
   IFX_TAPI_LINE_HOOK_VT_DIGITLOW_TIME    = 0x4,
   /** Settings for pulse digit high, close loop and break validation.
       The time must match between the time defined in the fields nMinTime and
       nMaxTime to recognize it as pulse dialing event. */
   IFX_TAPI_LINE_HOOK_VT_DIGITHIGH_TIME   = 0x8,
   /** Settings for pulse digit pause. The time must match the time defined
       in the fields nMinTime and nMaxTime to recognize it as
       pulse dialing event. */
   IFX_TAPI_LINE_HOOK_VT_INTERDIGIT_TIME  = 0x10
} IFX_TAPI_LINE_HOOK_VALIDATION_TYPE_t;

/** LEC NLP (Non Linear Processor) settings.*/
typedef enum
{
   /** Default NLP on.*/
   IFX_TAPI_LEC_NLP_DEFAULT = 0,
   /** Switches on NLP. */
   IFX_TAPI_LEC_NLP_ON = 1,
   /** Switches off NLP. */
   IFX_TAPI_LEC_NLP_OFF = 2
} IFX_TAPI_LEC_NLP_t;

/** WLEC NLP Settings. */
typedef enum
{
   /** Uses default NLP setting. */
   IFX_TAPI_WLEC_NLP_DEFAULT = IFX_TAPI_LEC_NLP_DEFAULT,
   /** Switches on NLP. */
   IFX_TAPI_WLEC_NLP_ON = IFX_TAPI_LEC_NLP_ON,
   /** Switches off NLP. */
   IFX_TAPI_WLEC_NLP_OFF = IFX_TAPI_LEC_NLP_OFF
} IFX_TAPI_WLEC_NLP_t;

/** Specifies the Enable/Disable mode of the high level. */
typedef enum
{
   /** Disables line level. */
   IFX_TAPI_LINE_LEVEL_DISABLE = 0x0,
   /** Enables line level. */
   IFX_TAPI_LINE_LEVEL_ENABLE = 0x1
} IFX_TAPI_LINE_LEVEL_t;

/** Specifies the Enable/Disable mode of the AGC resouce .*/
typedef enum
{
   /** Disables AGC. */
   IFX_TAPI_ENC_AGC_MODE_DISABLE = 0x0,
   /** Enables AGC. */
   IFX_TAPI_ENC_AGC_MODE_ENABLE  = 0x1
} IFX_TAPI_ENC_AGC_MODE_t;

/** Structure used for AGC configuration. */
typedef struct
{
   /** "Compare Level", this is the target level.
       Range: -50dB ... 0dB */
   int   com;
   /** "Maximum Gain", maximum gain that will be applied to the signal.
       Range: 0dB ... 48dB */
   int   gain;
   /** "Maximum Attenuation for AGC", maximum attenuation that will be
       applied to the signal.
       Range: -42dB ... 0dB */
   int   att;
   /** "Minimum Input Level", signals below this threshold won't be
       processed by AGC.
       Range: -60dB ... -25dB */
   int   lim;
} IFX_TAPI_ENC_AGC_CFG_t;

/*@}*/ /* TAPI_INTERFACE_OP */

/* ======================================================================= */
/* TAPI Metering Services, enumerations (Group TAPI_INTERFACE_METER)       */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_METER */
/*@{*/

/** Metering modes. */
typedef enum
{
   /** Normal TTX mode. */
   IFX_TAPI_METER_MODE_TTX = 0,
   /** Reverse polarity mode. */
   IFX_TAPI_METER_MODE_REVPOL = 1
} IFX_TAPI_METER_MODE_t;

/*@}*/ /* TAPI_INTERFACE_METER */

/* ======================================================================= */
/* TAPI Tone Control Services, enumerations (Group TAPI_INTERFACE_TONE)    */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_TONE */
/*@{*/

/** Defines the tone generator usage. */
typedef enum
{
   /** Use tone generator 1. */
   IFX_TAPI_TONE_TG1 = 1,
   /** Use tone generator 2. */
   IFX_TAPI_TONE_TG2 = 2,
   /** Use all tone generators.  */
   IFX_TAPI_TONE_TGALL = 0xff
} IFX_TAPI_TONE_TG_t;

/** Tone Sources. */
typedef enum
{
   /** Tone is played out on default source. */
   IFX_TAPI_TONE_SRC_DEFAULT = 0,
   /** Tone is played out on DSP for the DECT channel. */
   IFX_TAPI_TONE_SRC_DECT    = 0x2000,
   /** Tone is played out on DSP, default, if available. */
   IFX_TAPI_TONE_SRC_DSP     = 0x4000,
   /** Tone is played out on local tone generator in
      the analog part of the device.
      Default if DSP is not available. */
   IFX_TAPI_TONE_SRC_TG      = 0x8000
} IFX_TAPI_TONE_SRC_t;

/** Used for selection of one or more frequencies belonging to a tone cadence. */
typedef enum
{
   /** All frequencies are inactive.*/
   IFX_TAPI_TONE_FREQNONE = 0,
   /** Plays frequency A. */
   IFX_TAPI_TONE_FREQA = 0x1,
   /** Plays frequency B.*/
   IFX_TAPI_TONE_FREQB = 0x2,
   /** Plays frequency C. */
   IFX_TAPI_TONE_FREQC = 0x4,
   /** Plays frequency D. */
   IFX_TAPI_TONE_FREQD = 0x8,
   /** Plays all frequencies. */
   IFX_TAPI_TONE_FREQALL = 0xF
} IFX_TAPI_TONE_FREQ_t;

/** Modulation setting for a cadence step. */
typedef enum
{
   /** Modulation off for the cadence step.*/
   IFX_TAPI_TONE_MODULATION_OFF = 0,
   /** Modulation on for the cadence step.*/
   IFX_TAPI_TONE_MODULATION_ON = 1
} IFX_TAPI_TONE_MODULATION_t;

/** Tone types */
typedef enum
{
   /** Simple tone. */
   IFX_TAPI_TONE_TYPE_SIMPLE = 1,
   /** Composed tone. */
   IFX_TAPI_TONE_TYPE_COMPOSED = 2,
   /** Dual tone. */
   IFX_TAPI_TONE_TYPE_DUAL = 3
} IFX_TAPI_TONE_TYPE_t;

/*@}*/ /* TAPI_INTERFACE_TONE */

/* ==================================================================== */
/* TAPI Dial Services, enumerations (Group TAPI_INTERFACE_DIAL)         */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_DIAL */
/*@{*/

/** Enumeration for dial status events. */
typedef enum
{
   /** DTMF sign detected. */
   IFX_TAPI_DIALING_STATUS_DTMF  = 0x01,
   /** Pulse digit detected. */
   IFX_TAPI_DIALING_STATUS_PULSE = 0x02
} IFX_TAPI_DIALING_STATUS_t;

/*@}*/ /* TAPI_INTERFACE_DIAL */

/* ==================================================================== */
/* TAPI Signal Detection Services, enumerations                         */
/* (Group TAPI_INTERFACE_SIGNAL)                                        */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_SIGNAL */
/*@{*/

/** Lists the tone detection options. Some application maybe not interessted
    whether the signal came from the receive or transmit path. Therefore
    for each signal a mask exists, that includes receive and transmit path.
    */
typedef enum
{
   /** no signal detected .*/
   IFX_TAPI_SIG_NONE        = 0x0,
   /** V.21 Preamble Fax Tone, Digital identification signal (DIS),
      receive path. */
   IFX_TAPI_SIG_DISRX       = 0x1,
   /** V.21 Preamble Fax Tone, Digital identification signal (DIS),
      transmit path. */
   IFX_TAPI_SIG_DISTX       = 0x2,
   /** V.21 Preamble Fax Tone in all path, Digital identification
       signal (DIS).  */
   IFX_TAPI_SIG_DIS         = 0x4,
   /** V.25 2100 Hz (CED) Modem/Fax Tone, receive path. */
   IFX_TAPI_SIG_CEDRX       = 0x8,
   /** V.25 2100 Hz (CED) Modem/Fax Tone, transmit path. */
   IFX_TAPI_SIG_CEDTX       = 0x10,
   /** V.25 2100 Hz (CED) Modem/Fax Tone in all paths. */
   IFX_TAPI_SIG_CED         = 0x20,
   /** CNG Fax Calling Tone (1100 Hz) receive path. */
   IFX_TAPI_SIG_CNGFAXRX    = 0x40,
   /** CNG Fax Calling Tone (1100 Hz) transmit path. */
   IFX_TAPI_SIG_CNGFAXTX    = 0x80,
   /** CNG Fax Calling Tone (1100 Hz) in all paths. */
   IFX_TAPI_SIG_CNGFAX      = 0x100,
   /** CNG Modem Calling Tone (1300 Hz) receive path. */
   IFX_TAPI_SIG_CNGMODRX    = 0x200,
   /** CNG Modem Calling Tone (1300 Hz) transmit path.  */
   IFX_TAPI_SIG_CNGMODTX    = 0x400,
   /** CNG Modem Calling Tone (1300 Hz) in all paths. */
   IFX_TAPI_SIG_CNGMOD      = 0x800,
   /** Phase reversal detection receive path.
       \remarks Not supported phase reversal uses the same
       paths as CED detection. The detector for CED must be configured
       as well . */
   IFX_TAPI_SIG_PHASEREVRX  = 0x1000,
   /** Phase reversal detection transmit path.
      \remarks Not supported phase reversal uses the same
      paths as CED detection. The detector for CED must be configured
      as well  */
   IFX_TAPI_SIG_PHASEREVTX  = 0x2000,
   /** Phase reversal detection in all paths.
       \remarks Phase reversals are detected at the
       end of an CED. If this signal is enabled also CED end detection
       is automatically enabled and reported if it occurs */
   IFX_TAPI_SIG_PHASEREV    = 0x4000,
   /** Amplitude modulation receive path.
      \remarks Not supported amplitude modulation uses the same
      paths as CED detection. The detector for CED must be configured
      as well  */
   IFX_TAPI_SIG_AMRX         = 0x8000,
   /** Amplitude modulation transmit path.
   \remarks Not supported amplitude modulation uses the same
   paths as CED detection. The detector for CED must be configured
   as well  */
   IFX_TAPI_SIG_AMTX                 = 0x10000,
   /** Amplitude modulation.

    \remarks Amplitude modulation is detected at the
    end of an CED. If this signal is enabled also CED end detection
    is automatically enabled and reported if it occurs.

    \note In case of AM detected the driver automatically switches
    to modem coefficients after the ending of CED */
   IFX_TAPI_SIG_AM                   = 0x20000,
   /** Modem tone holding signal stopped receive path.*/
   IFX_TAPI_SIG_TONEHOLDING_ENDRX    = 0x40000,
   /** Modem tone holding signal stopped transmit path. */
   IFX_TAPI_SIG_TONEHOLDING_ENDTX    = 0x80000,
   /** Modem tone holding signal stopped all paths. */
   IFX_TAPI_SIG_TONEHOLDING_END      = 0x100000,
   /** End of signal CED detection receive path.

      \remarks Not supported. CED end detection uses the same
      paths as CED detection. The detector for CED must be configured
      as well.  */
   IFX_TAPI_SIG_CEDENDRX          = 0x200000,
   /** End of signal CED detection transmit path.

      \remarks Not supported. CED end detection uses the same
      paths as CED detection. The detector for CED must be configured
      as well.  */
   IFX_TAPI_SIG_CEDENDTX          = 0x400000,
   /** End of signal CED detection. This signal also includes
      information about phase reversals and amplitude modulation,
      if enabled. */
   IFX_TAPI_SIG_CEDEND            = 0x800000,
   /** Signals a call progress tone detection. This signal is enabled with
       the interface \ref IFX_TAPI_TONE_CPTD_START and stopped with
       \ref IFX_TAPI_TONE_CPTD_STOP. It cannot be activated with
       \ref IFX_TAPI_SIG_DETECT_ENABLE. */
   IFX_TAPI_SIG_CPTD              = 0x1000000,
   /** Signals the V8bis detection on the receive path. */
   IFX_TAPI_SIG_V8BISRX           = 0x2000000,
   /** Signals the V8bis detection on the transmit path. */
   IFX_TAPI_SIG_V8BISTX           = 0x4000000,
   /** Signals that the Caller-ID transmission has finished
       This event cannot be activated with \ref IFX_TAPI_SIG_DETECT_ENABLE.
       Please use IFX_TAPI_CID_RX_START and IFX_TAPI_CID_RX_STOP instead. */
   IFX_TAPI_SIG_CIDENDTX          = 0x8000000,
   /** Enables DTMF reception on locally connected analog line. */
   IFX_TAPI_SIG_DTMFTX            = 0x10000000,
   /** Enables DTMF reception on remote connected line. */
   IFX_TAPI_SIG_DTMFRX            = 0x20000000
} IFX_TAPI_SIG_t;

/** This service offers extended tone detection options. */
typedef enum
{
   /** No signal detected. */
   IFX_TAPI_SIG_EXT_NONE          = 0x0,
   /** 980 Hz single tone (V.21L mark sequence) receive path. */
   IFX_TAPI_SIG_EXT_V21LRX        = 0x1,
   /** 980 Hz single tone (V.21L mark sequence) transmit path. */
   IFX_TAPI_SIG_EXT_V21LTX        = 0x2,
   /** 980 Hz single tone (V.21L mark sequence) all paths. */
   IFX_TAPI_SIG_EXT_V21L          = 0x4,
   /** 1400 Hz single tone (V.18A mark sequence) receive path. */
   IFX_TAPI_SIG_EXT_V18ARX        = 0x8,
   /** 1400 Hz single tone (V.18A mark sequence) transmit path. */
   IFX_TAPI_SIG_EXT_V18ATX        = 0x10,
   /** 1400 Hz single tone (V.18A mark sequence) all paths. */
   IFX_TAPI_SIG_EXT_V18A          = 0x20,
   /** 1800 Hz single tone (V.27, V.32 carrier) receive path. */
   IFX_TAPI_SIG_EXT_V27RX         = 0x40,
   /** 1800 Hz single tone (V.27, V.32 carrier) transmit path. */
   IFX_TAPI_SIG_EXT_V27TX         = 0x80,
   /** 1800 Hz single tone (V.27, V.32 carrier) all paths. */
   IFX_TAPI_SIG_EXT_V27           = 0x100,
   /** 2225 Hz single tone (Bell answering tone) receive path. */
   IFX_TAPI_SIG_EXT_BELLRX        = 0x200,
   /** 2225 Hz single tone (Bell answering tone) transmit path. */
   IFX_TAPI_SIG_EXT_BELLTX        = 0x400,
   /** 2225 Hz single tone (Bell answering tone) all paths. */
   IFX_TAPI_SIG_EXT_BELL          = 0x800,
   /** 2250 Hz single tone (V.22 unscrambled binary ones) receive path. */
   IFX_TAPI_SIG_EXT_V22RX         = 0x1000,
   /** 2250 Hz single tone (V.22 unscrambled binary ones) transmit path. */
   IFX_TAPI_SIG_EXT_V22TX         = 0x2000,
   /** 2250 Hz single tone (V.22 unscrambled binary ones) all paths. */
   IFX_TAPI_SIG_EXT_V22           = 0x4000,
   /** 2225 Hz or 2250 Hz single tone, not possible to distinguish receive path.*/
   IFX_TAPI_SIG_EXT_V22ORBELLRX   = 0x8000,
   /** 2225 Hz or 2250 Hz single tone, not possible to distinguish transmit path.*/
   IFX_TAPI_SIG_EXT_V22ORBELLTX   = 0x10000,
   /** 2225 Hz or 2250 Hz single tone, not possible to distinguish all paths. */
   IFX_TAPI_SIG_EXT_V22ORBELL     = 0x20000,
   /** 600 Hz + 300 Hz dual tone (V.32 AC) receive path. */
   IFX_TAPI_SIG_EXT_V32ACRX       = 0x40000,
   /** 600 Hz + 300 Hz dual tone (V.32 AC) transmit path. */
   IFX_TAPI_SIG_EXT_V32ACTX       = 0x80000,
   /** 600 Hz + 300 Hz dual tone (V.32 AC) all paths. */
   IFX_TAPI_SIG_EXT_V32AC         = 0x100000,
   /** 2130 + 2750 Hz dual tone (Bell Caller ID Type 2 Alert Tone) receive path. */
   IFX_TAPI_SIG_EXT_CASBELLRX     = 0x200000,
   /** 2130 + 2750 Hz dual tone (Bell Caller ID Type 2 Alert Tone) transmit path. */
   IFX_TAPI_SIG_EXT_CASBELLTX     = 0x400000,
   /** 2130 + 2750 Hz dual tone (Bell Caller ID Type 2 Alert Tone) all paths. */
   IFX_TAPI_SIG_EXT_CASBELL       = 0x600000,
   /** 1650 Hz single tone (V.21H mark sequence) receive path. */
   IFX_TAPI_SIG_EXT_V21HRX        = 0x800000,
   /** 1650 Hz single tone (V.21H mark sequence) transmit path. */
   IFX_TAPI_SIG_EXT_V21HTX        = 0x1000000,
   /** 1650 Hz single tone (V.21H mark sequence) all paths. */
   IFX_TAPI_SIG_EXT_V21H          = 0x1800000,
   /** Voice modem discriminator all paths. */
   IFX_TAPI_SIG_EXT_VMD           = 0x2000000
} IFX_TAPI_SIG_EXT_t;

/** Lists the RFC2833 tone events that are detected from network side. */
typedef enum
{
   /** RFC2833 Event number for DTMF tone 0. */
   IFX_TAPI_PKT_EV_NUM_DTMF_0     = 0,
   /** RFC2833 Event number for DTMF tone 1. */
   IFX_TAPI_PKT_EV_NUM_DTMF_1     = 1,
   /** RFC2833 Event number for DTMF tone 2. */
   IFX_TAPI_PKT_EV_NUM_DTMF_2     = 2,
   /** RFC2833 Event number for DTMF tone 3. */
   IFX_TAPI_PKT_EV_NUM_DTMF_3     = 3,
   /** RFC2833 Event number for DTMF tone 4. */
   IFX_TAPI_PKT_EV_NUM_DTMF_4     = 4,
   /** RFC2833 Event number for DTMF tone 5. */
   IFX_TAPI_PKT_EV_NUM_DTMF_5     = 5,
   /** RFC2833 Event number for DTMF tone 6. */
   IFX_TAPI_PKT_EV_NUM_DTMF_6     = 6,
   /** RFC2833 Event number for DTMF tone 7. */
   IFX_TAPI_PKT_EV_NUM_DTMF_7     = 7,
   /** RFC2833 Event number for DTMF tone 8. */
   IFX_TAPI_PKT_EV_NUM_DTMF_8     = 8,
   /** RFC2833 Event number for DTMF tone 9. */
   IFX_TAPI_PKT_EV_NUM_DTMF_9     = 9,
   /** RFC2833 Event number for DTMF tone *. */
   IFX_TAPI_PKT_EV_NUM_DTMF_STAR  = 10,
   /** RFC2833 Event number for DTMF tone #. */
   IFX_TAPI_PKT_EV_NUM_DTMF_HASH  = 11,
   /** RFC2833 Event number for ANS tone. */
   IFX_TAPI_PKT_EV_NUM_ANS        = 32,
   /** RFC2833 Event number for /ANS tone. */
   IFX_TAPI_PKT_EV_NUM_NANS       = 33,
   /** RFC2833 Event number for ANSam tone. */
   IFX_TAPI_PKT_EV_NUM_ANSAM      = 34,
   /** RFC2833 Event number for /ANSam tone. */
   IFX_TAPI_PKT_EV_NUM_NANSAM     = 35,
   /** RFC2833 Event number for CNG tone. */
   IFX_TAPI_PKT_EV_NUM_CNG        = 36,
   /** RFC2833 Event number for DIS signal. */
   IFX_TAPI_PKT_EV_NUM_DIS        = 54,
   /** no support RFC event received or no event received. */
   IFX_TAPI_PKT_EV_NUM_NO_EVENT   = 0xFFFFFFFF
} IFX_TAPI_PKT_EV_NUM_t;

/** Specifies the CPT signal for CPT detection */
typedef enum
{
   /** Receive direction. */
   IFX_TAPI_TONE_CPTD_DIRECTION_RX = 0x1,
   /** Transmit direction. */
   IFX_TAPI_TONE_CPTD_DIRECTION_TX = 0x2
} IFX_TAPI_TONE_CPTD_DIRECTION_t;

/*@}*/ /* TAPI_INTERFACE_SIGNAL */

/* ======================================================================= */
/* TAPI CID Features Service, enumerations (Group TAPI_INTERFACE_CID)        */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_CID */
/*@{*/

/** List of ETSI Alerts. */
typedef enum
{
   /** First Ring Burst.
     Note: Defined only CID transmission associated with ringing.*/
   IFX_TAPI_CID_ALERT_ETSI_FR = 0x0,
   /** DTAS. */
   IFX_TAPI_CID_ALERT_ETSI_DTAS = 0x1,
   /** Ring Pulse. */
   IFX_TAPI_CID_ALERT_ETSI_RP = 0x2,
   /** Line Reversal (alias Polarity Reversal) followed by DTAS. */
   IFX_TAPI_CID_ALERT_ETSI_LRDTAS = 0x3
} IFX_TAPI_CID_ALERT_ETSI_t;

/** List of CID standards.*/
typedef enum
{
   /** Bellcore/Telcordia GR-30-CORE. Using Bell202 FSK coding of CID
    information.*/
   IFX_TAPI_CID_STD_TELCORDIA = 0x0,
   /** ETSI 300-659-1/2/3 V1.3.1. Using V.23 FSK coding to transmit CID
    information.*/
   IFX_TAPI_CID_STD_ETSI_FSK = 0x1,
   /** ETSI 300-659-1/2/3 V1.3.1. Using DTMF transmission of CID information.*/
   IFX_TAPI_CID_STD_ETSI_DTMF = 0x2,
   /** SIN 227 Issue 3.4. Using V.23 FSK coding of CID information.*/
   IFX_TAPI_CID_STD_SIN = 0x3,
   /** NTT standard: TELEPHONE SERVICE INTERFACES, Edition 5. Using a modified
    V.23 FSK coding of CID information.*/
   IFX_TAPI_CID_STD_NTT = 0x4
} IFX_TAPI_CID_STD_t;

/** Caller ID transmission modes.
 \remarks Information required especially for FSK framing.*/
typedef enum
{
   /** On-hook transmission. Applicable to CID type 1 and MWI.*/
   IFX_TAPI_CID_HM_ONHOOK   = 0x00,
   /** Off-hook transmission. Applicable to CID type 2 and MWI.*/
   IFX_TAPI_CID_HM_OFFHOOK  = 0x01
} IFX_TAPI_CID_HOOK_MODE_t;

/** Caller ID Message Type defined in ETSI EN 300 659-3.*/
typedef enum
{
   /** Call Set-up. Corresponds to Caller ID type 1 and type 2. */
   IFX_TAPI_CID_MT_CSUP  = 0x80,
   /** Message Waiting Indicator.*/
   IFX_TAPI_CID_MT_MWI   = 0x82,
   /** Advice of Charge.*/
   IFX_TAPI_CID_MT_AOC   = 0x86,
   /** Short Message Service.*/
   IFX_TAPI_CID_MT_SMS   = 0x89,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES01 = 0xF1,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES02 = 0xF2,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES03 = 0xF3,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES04 = 0xF4,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES05 = 0xF5,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES06 = 0xF6,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES07 = 0xF7,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES08 = 0xF8,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES09 = 0xF9,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0A = 0xFA,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0B = 0xFB,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0C = 0xFC,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0D = 0xFD,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0E = 0xFE,
   /** Reserved for Network Operator Use.*/
   IFX_TAPI_CID_MT_RES0F = 0xFF
} IFX_TAPI_CID_MSG_TYPE_t;

/** Caller ID Services (defined in ETSI EN 300 659-3).*/
typedef enum
{
   /** Date and time presentation.*/
   IFX_TAPI_CID_ST_DATE        = 0x01,
   /** Calling line identity (mandatory).*/
   IFX_TAPI_CID_ST_CLI         = 0x02,
   /** Called line identity.*/
   IFX_TAPI_CID_ST_CDLI        = 0x03,
   /** Reason for absence of CLI.*/
   IFX_TAPI_CID_ST_ABSCLI      = 0x04,
   /** Calling line name.*/
   IFX_TAPI_CID_ST_NAME        = 0x07,
   /** Reason for absence of name.*/
   IFX_TAPI_CID_ST_ABSNAME     = 0x08,
   /** Visual indicator.*/
   IFX_TAPI_CID_ST_VISINDIC    = 0x0B,
   /** Message Identification.*/
   IFX_TAPI_CID_ST_MSGIDENT    = 0x0D,
   /** Last Message CLI.*/
   IFX_TAPI_CID_ST_LMSGCLI     = 0x0E,
   /** Complementary Date and Time.*/
   IFX_TAPI_CID_ST_CDATE       = 0x0F,
   /** Complementary calling line identity.*/
   IFX_TAPI_CID_ST_CCLI        = 0x10,
   /** Call type.*/
   IFX_TAPI_CID_ST_CT          = 0x11,
   /** First called line identity.*/
   IFX_TAPI_CID_ST_FIRSTCLI    = 0x12,
   /** Number of messages.*/
   IFX_TAPI_CID_ST_MSGNR       = 0x13,
   /** Type of forwarded call.*/
   IFX_TAPI_CID_ST_FWCT        = 0x15,
   /** Type of calling user.*/
   IFX_TAPI_CID_ST_USRT        = 0x16,
   /** Number redirection.*/
   IFX_TAPI_CID_ST_REDIR       = 0x1A,
   /** Charge.*/
   IFX_TAPI_CID_ST_CHARGE      = 0x20,
   /** Additional charge.*/
   IFX_TAPI_CID_ST_ACHARGE     = 0x21,
   /** Duration of the call.*/
   IFX_TAPI_CID_ST_DURATION    = 0x23,
   /** Network provider id.*/
   IFX_TAPI_CID_ST_NTID        = 0x30,
   /** Carrier identity.*/
   IFX_TAPI_CID_ST_CARID       = 0x31,
   /** Selection of terminal function.*/
   IFX_TAPI_CID_ST_TERMSEL     = 0x40,
   /** Display information, used as INFO for DTMF.*/
   IFX_TAPI_CID_ST_DISP        = 0x50,
   /** Service Information.*/
   IFX_TAPI_CID_ST_SINFO       = 0x55,
   /** Extension for operator use.*/
   IFX_TAPI_CID_ST_XOPUSE      = 0xE0,
   /** Transparent mode.*/
   IFX_TAPI_CID_ST_TRANSPARENT = 0xFF
} IFX_TAPI_CID_SERVICE_TYPE_t;

/** List of VMWI settings.*/
typedef enum
{
   /** Disable VMWI on CPE.*/
   IFX_TAPI_CID_VMWI_DIS = 0x00,
   /** Enable VMWI on CPE.*/
   IFX_TAPI_CID_VMWI_EN  = 0xFF
} IFX_TAPI_CID_VMWI_t;

/** List of ABSCLI/ABSNAME settings.*/
typedef enum
{
   /** Unavailable/Unknown.*/
   IFX_TAPI_CID_ABSREASON_UNAV = 0x4F,
   /** Private.*/
   IFX_TAPI_CID_ABSREASON_PRIV = 0x50
} IFX_TAPI_CID_ABSREASON_t;

/* Defines for CID Receiver */

/** CID Receiver Status. */
typedef enum
{
   /** CID receiver is not active.*/
   IFX_TAPI_CID_RX_STATE_INACTIVE = 0,
   /** CID Receiver is active.*/
   IFX_TAPI_CID_RX_STATE_ACTIVE   = 1,
   /** CID Receiver is just receiving data.*/
   IFX_TAPI_CID_RX_STATE_ONGOING = 2,
   /** CID Receiver is completed.*/
   IFX_TAPI_CID_RX_STATE_DATA_READY = 3
} IFX_TAPI_CID_RX_STATE_t;

/** CID Receiver Errors.*/
typedef enum
{
   /** No Error during CID Receiver operation.*/
   IFX_TAPI_CID_RX_ERROR_NONE = 0,
   /** Reading error during CID Receiver operation.*/
   IFX_TAPI_CID_RX_ERROR_READ = 1
} IFX_TAPI_CID_RX_ERROR_t;

/*@}*/ /* TAPI_INTERFACE_CID */

/* ========================================================================= */
/* TAPI Connection Services, enumerations (Group TAPI_INTERFACE_CON)         */
/* ========================================================================= */
/** \addtogroup TAPI_INTERFACE_CON */
/*@{*/

/** Definition of codec algorithms. The enumerated elements should be used
    to select the encoding algorithm and as array index for the RTP payload
    type configuration.*/
typedef enum
{
   /** Reserved. */
   IFX_TAPI_COD_TYPE_UNKNOWN  = 0,
   /** G723, 6.3 kBit/s. */
   IFX_TAPI_COD_TYPE_G723_63  = 1,
   /** G723, 5.3 kBit/s. */
   IFX_TAPI_COD_TYPE_G723_53  = 2,
   /** G728, 16 kBit/s. Not available! */
   IFX_TAPI_COD_TYPE_G728     = 6,
   /** G.729 A and B (silence compression), 8 kBit/s. */
   IFX_TAPI_COD_TYPE_G729     = 7,
   /** G711 u-Law, 64 kBit/s. */
   IFX_TAPI_COD_TYPE_MLAW     = 8,
   /** G711 A-Law, 64 kBit/s */
   IFX_TAPI_COD_TYPE_ALAW     = 9,
   /** G.711 u-law, 64 kBit/s. Voice Band Data encoding as defined by V.152. */
   IFX_TAPI_COD_TYPE_MLAW_VBD = 10,
   /** G.711 A-law, 64 kBit/s. Voice Band Data encoding as defined by V.152.*/
   IFX_TAPI_COD_TYPE_ALAW_VBD = 11,
   /** G726, 16 kBit/s. */
   IFX_TAPI_COD_TYPE_G726_16  = 12,
   /** G726, 24 kBit/s. */
   IFX_TAPI_COD_TYPE_G726_24  = 13,
   /** G726, 32 kBit/s. */
   IFX_TAPI_COD_TYPE_G726_32  = 14,
   /** G726, 40 kBit/s .*/
   IFX_TAPI_COD_TYPE_G726_40  = 15,
   /** G729 E, 11.8 kBit/s. */
   IFX_TAPI_COD_TYPE_G729_E   = 16,
   /** iLBC, 13.3 kBit/s. */
   IFX_TAPI_COD_TYPE_ILBC_133 = 17,
   /** iLBC, 15.2 kBit/s. */
   IFX_TAPI_COD_TYPE_ILBC_152 = 18,
   /** Linear Codec, 16 Bit, 8KHz. */
   IFX_TAPI_COD_TYPE_LIN16_8  = 19,
   /** Linear Codec, 16 Bit, 16KHz. */
   IFX_TAPI_COD_TYPE_LIN16_16 = 20,
   /** AMR, 4.75 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_4_75 = 21,
   /** AMR, 5.15 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_5_15 = 22,
   /** AMR, 5.9 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_5_9  = 23,
   /** AMR, 6.7 kBit/s */
   IFX_TAPI_COD_TYPE_AMR_6_7  = 24,
   /** AMR, 7.4 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_7_4  = 25,
   /** AMR, 7.95 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_7_95 = 26,
   /** AMR, 10.2 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_10_2 = 27,
   /** AMR, 12.2 kBit/s. */
   IFX_TAPI_COD_TYPE_AMR_12_2 = 28,

   /** G.722 (wideband), 64 kBit/s.*/
   IFX_TAPI_COD_TYPE_G722_64  = 31,
   /** G.722.1 (wideband), 24 kBit/s. */
   IFX_TAPI_COD_TYPE_G7221_24 = 32,
   /** G.722.1 (wideband), 32 kBit/s. */
   IFX_TAPI_COD_TYPE_G7221_32 = 33,

   /** Maximum number of Codecs. */
   IFX_TAPI_COD_TYPE_MAX
} IFX_TAPI_COD_TYPE_t;

/** Possible codecs to select for \ref IFX_TAPI_ENC_TYPE_SET
   and \ref IFX_TAPI_PCK_AAL_PROFILE_t. */
typedef enum
{
   /** G723, 6.3 kBit/s. */
   IFX_TAPI_ENC_TYPE_G723_63  = IFX_TAPI_COD_TYPE_G723_63,
   /** G723, 5.3 kBit/s. */
   IFX_TAPI_ENC_TYPE_G723_53  = IFX_TAPI_COD_TYPE_G723_53,
   /** G728, 16 kBit/s. */
   IFX_TAPI_ENC_TYPE_G728     = IFX_TAPI_COD_TYPE_G728,
   /** G729 A and B, 8 kBit/s. */
   IFX_TAPI_ENC_TYPE_G729     = IFX_TAPI_COD_TYPE_G729,
   /** G711 u-Law, 64 kBit/s. */
   IFX_TAPI_ENC_TYPE_MLAW     = IFX_TAPI_COD_TYPE_MLAW,
   /** G711 A-Law, 64 kBit/s. */
   IFX_TAPI_ENC_TYPE_ALAW     = IFX_TAPI_COD_TYPE_ALAW,
   /** G711 u-law VBD, 64 kBit/s. Voice Band Data encoding as defined by V.152.*/
   IFX_TAPI_ENC_TYPE_MLAW_VBD = IFX_TAPI_COD_TYPE_MLAW_VBD,
   /** G711 A-law VBD, 64 kBit/s. Voice Band Data encoding as defined by V.152.*/
   IFX_TAPI_ENC_TYPE_ALAW_VBD = IFX_TAPI_COD_TYPE_ALAW_VBD,
   /** G726, 16 kBit/s. */
   IFX_TAPI_ENC_TYPE_G726_16  = IFX_TAPI_COD_TYPE_G726_16,
   /** G726, 24 kBit/s. */
   IFX_TAPI_ENC_TYPE_G726_24  = IFX_TAPI_COD_TYPE_G726_24,
   /** G726, 32 kBit/s. */
   IFX_TAPI_ENC_TYPE_G726_32  = IFX_TAPI_COD_TYPE_G726_32,
   /** G726, 40 kBit/s. */
   IFX_TAPI_ENC_TYPE_G726_40  = IFX_TAPI_COD_TYPE_G726_40,
   /** G729 E, 11.8 kBit/s. */
   IFX_TAPI_ENC_TYPE_G729_E   = IFX_TAPI_COD_TYPE_G729_E,
   /** iLBC, 13.3 kBit/s. */
   IFX_TAPI_ENC_TYPE_ILBC_133 = IFX_TAPI_COD_TYPE_ILBC_133,
   /** iLBC, 15.2 kBit/s. */
   IFX_TAPI_ENC_TYPE_ILBC_152 = IFX_TAPI_COD_TYPE_ILBC_152,
   /** Linear Codec, 16 Bit, 8KHz. */
   IFX_TAPI_ENC_TYPE_LIN16_8 = IFX_TAPI_COD_TYPE_LIN16_8,
   /** Linear Codec, 16 Bit, 16KHz. */
   IFX_TAPI_ENC_TYPE_LIN16_16 = IFX_TAPI_COD_TYPE_LIN16_16,
   /** AMR, 4.75 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_4_75 = IFX_TAPI_COD_TYPE_AMR_4_75,
   /** AMR, 5.15 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_5_15 = IFX_TAPI_COD_TYPE_AMR_5_15,
   /** AMR, 5.9 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_5_9  = IFX_TAPI_COD_TYPE_AMR_5_9,
   /** AMR, 6.7 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_6_7  = IFX_TAPI_COD_TYPE_AMR_6_7,
   /** AMR, 7.4 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_7_4  = IFX_TAPI_COD_TYPE_AMR_7_4,
   /** AMR, 7.95 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_7_95 = IFX_TAPI_COD_TYPE_AMR_7_95,
   /** AMR, 10.2 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_10_2 = IFX_TAPI_COD_TYPE_AMR_10_2,
   /** AMR, 12.2 kBit/s. */
   IFX_TAPI_ENC_TYPE_AMR_12_2 = IFX_TAPI_COD_TYPE_AMR_12_2,
   /** G.722 (wideband), 64 kBit/s. */
   IFX_TAPI_ENC_TYPE_G722_64  = IFX_TAPI_COD_TYPE_G722_64,
   /** G.722.1 (wideband), 24 kBit/s. */
   IFX_TAPI_ENC_TYPE_G7221_24 = IFX_TAPI_COD_TYPE_G7221_24,
   /** G.722.1 (wideband), 32 kBit/s. */
   IFX_TAPI_ENC_TYPE_G7221_32 = IFX_TAPI_COD_TYPE_G7221_32,
   /** Maximum number of Codecs, used by \ref IFX_TAPI_PKT_RTP_PT_CFG_t.*/
   IFX_TAPI_ENC_TYPE_MAX
} IFX_TAPI_ENC_TYPE_t;

/** Packetisation length. */
typedef enum
{
   /** Zero packetization length. Not supported. */
   IFX_TAPI_COD_LENGTH_ZERO = 0,
   /** 2.5 ms packetization length. */
   IFX_TAPI_COD_LENGTH_2_5  = 1,
   /** 5 ms packetization length. */
   IFX_TAPI_COD_LENGTH_5    = 2,
   /** 5.5 ms packetization length. */
   IFX_TAPI_COD_LENGTH_5_5  = 3,
   /** 10 ms packetization length. */
   IFX_TAPI_COD_LENGTH_10   = 4,
   /** 11 ms packetization length. */
   IFX_TAPI_COD_LENGTH_11   = 5,
   /** 20 ms packetization length. */
   IFX_TAPI_COD_LENGTH_20   = 6,
   /** 30 ms packetization length. */
   IFX_TAPI_COD_LENGTH_30   = 7,
   /** 40 ms packetization length. */
   IFX_TAPI_COD_LENGTH_40   = 8,
   /** 50 ms packetization length.*/
   IFX_TAPI_COD_LENGTH_50   = 9,
   /** 60 ms packetization length. */
   IFX_TAPI_COD_LENGTH_60   = 10
} IFX_TAPI_COD_LENGTH_t;

/** packetisation length (old interface, kept for compatibility) */
typedef enum
{
   /** Zero packetization length. Not supported. */
   IFX_TAPI_ENC_LENGTH_ZERO = IFX_TAPI_COD_LENGTH_ZERO,
   /** 2.5 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_2_5  = IFX_TAPI_COD_LENGTH_2_5,
   /** 5 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_5    = IFX_TAPI_COD_LENGTH_5,
   /** 5.5 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_5_5  = IFX_TAPI_COD_LENGTH_5_5,
   /** 10 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_10   = IFX_TAPI_COD_LENGTH_10,
   /** 11 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_11   = IFX_TAPI_COD_LENGTH_11,
   /** 20 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_20   = IFX_TAPI_COD_LENGTH_20,
   /** 30 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_30   = IFX_TAPI_COD_LENGTH_30,
   /** 40 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_40   = IFX_TAPI_COD_LENGTH_40,
   /** 50 ms packetization length.*/
   IFX_TAPI_ENC_LENGTH_50   = IFX_TAPI_COD_LENGTH_50,
   /** 5 ms packetization length. */
   IFX_TAPI_ENC_LENGTH_60   = IFX_TAPI_COD_LENGTH_60
} IFX_TAPI_ENC_LENGTH_t;

/** Possible bitorder to select via \ref IFX_TAPI_ENC_CFG_SET and
    \ref IFX_TAPI_DEC_CFG_SET. */
typedef enum
{
   /** Default bit packing / endianess. */
   IFX_TAPI_COD_RTP_BITPACK,
   /** ITU-T I366.2 Bit Alignment for G.726 codecs. */
   IFX_TAPI_COD_AAL2_BITPACK
} IFX_TAPI_COD_AAL2_BITPACK_t;

/** Type channel for mapping. */
typedef enum
{
   /** Default. It depends on the device and configures the best applicable. */
   IFX_TAPI_MAP_TYPE_DEFAULT = 0,
   /** Type is a coder packet channel. */
   IFX_TAPI_MAP_TYPE_CODER = 1,
   /** Type is a PCM channel. */
   IFX_TAPI_MAP_TYPE_PCM = 2,
   /** Type is a phone channel. */
   IFX_TAPI_MAP_TYPE_PHONE = 3,
   /** Type is a audio channel .*/
   IFX_TAPI_MAP_TYPE_AUDIO = 4,
   /** Type is a audio channel as a auxiliary input for incall anouncement. */
   IFX_TAPI_MAP_TYPE_AUDIO_AUX = 5,
   /** Type is a diagnostic channel attached to data stream 'behind' ADC0. */
   IFX_TAPI_MAP_TYPE_AUDIO_DIAG0_IN=6,
   /** Type is a diagnostic channel attached to data stream 'before' DAC0. */
   IFX_TAPI_MAP_TYPE_AUDIO_DIAG0_OUT=7,
   /** Type is a diagnostic channel attached to data stream 'behind' ADC1. */
   IFX_TAPI_MAP_TYPE_AUDIO_DIAG1_IN=8,
   /** Type is a diagnostic channel attached to data stream 'before' DAC1. */
   IFX_TAPI_MAP_TYPE_AUDIO_DIAG1_OUT=9,
   /** Type is an audio loop that assigns audio module's signals
       Audio_LOOP_I1 and Audio_LOOP_O3 to an additional data channel. */
   IFX_TAPI_MAP_TYPE_AUDIO_LOOP0=10,
   /** Type is an audio loop that assigns audio module's signals
       Audio_LOOP_I2 and Audio_LOOP_O1 to an additional data channel. */
   IFX_TAPI_MAP_TYPE_AUDIO_LOOP1=11,
   /** Type is a DECT channel. */
   IFX_TAPI_MAP_TYPE_DECT = 12
} IFX_TAPI_MAP_TYPE_t;

/** Recording enabling and disabling information. */
typedef enum
{
   /** Does not modify recording status. */
   IFX_TAPI_MAP_ENC_NONE = 0,
   /** Starts recording after mapping. */
   IFX_TAPI_MAP_ENC_START = 1,
   /** Stops recording after mapping. */
   IFX_TAPI_MAP_ENC_STOP = 2
} IFX_TAPI_MAP_ENC_t;

/** Plays out enabling and disabling information. */
typedef enum
{
   /** Does not modify playing status. */
   IFX_TAPI_MAP_DEC_NONE = 0,
   /** Starts playing after mapping. */
   IFX_TAPI_MAP_DEC_START = 1,
   /** Stops playing after mapping. */
   IFX_TAPI_MAP_DEC_STOP = 2
} IFX_TAPI_MAP_DEC_t;

/** Starts/Stops information for data channel mapping. */
typedef enum
{
   /** Does not modify the status of the recorder. */
   IFX_TAPI_MAP_DATA_UNCHANGED = 0,
   /** Recording is started. */
   IFX_TAPI_MAP_DATA_START = 1,
   /** Recording is stopped. */
   IFX_TAPI_MAP_DATA_STOP = 2
} IFX_TAPI_MAP_DATA_START_STOP_t;

/** Jitter buffer adaption. */
typedef enum
{
   /** Local Adaptation OFF. */
   IFX_TAPI_JB_LOCAL_ADAPT_OFF = 0,
   /** Local adaptation ON. */
   IFX_TAPI_JB_LOCAL_ADAPT_ON = 1,
   /** Local Adaptation ON
      with Sample Interpollation. */
   IFX_TAPI_JB_LOCAL_ADAPT_SI_ON = 2
} IFX_TAPI_JB_LOCAL_ADAPT_t;

/** Enumeration used for \ref IFX_TAPI_ENC_VAD_CFG_SET ioctl. */
typedef enum
{
   /** No voice activity detection. */
   IFX_TAPI_ENC_VAD_NOVAD = 0,
   /** Voice activity detection on, in this case also comfort noise and
    spectral information (nicer noise) is switched on.*/
   IFX_TAPI_ENC_VAD_ON = 1,
   /** Voice activity detection on with comfort noise generation without
    spectral information. */
   IFX_TAPI_ENC_VAD_G711 = 2,
   /** Voice activity detection on with comfort noise generation without
       silence compression */
   IFX_TAPI_ENC_VAD_CNG_ONLY = 3,
   /** Voice activity detection on with silence compression without
       comfort noise generation. */
   IFX_TAPI_ENC_VAD_SC_ONLY = 4
} IFX_TAPI_ENC_VAD_t;

/** Used for \ref IFX_TAPI_PCK_AAL_PROFILE_t in case one coder range. The range
   information is specified as UUI Codepoint Range in the specification of
   ATM Forum, AF-VMOA-0145.000 or ITU-T I.366.2. */
typedef enum
{
   /** One range from 0 to 15. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_15 = 0,
   /** Range from 0 to 7 for a two range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_7 = 1,
   /** Range from 8 to 15 for a two range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_15 = 2,
   /** Range from 0 to 3 for a four range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_0_3 = 3,
   /** Range from 4 to 7 for a four range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_4_7 = 4,
   /** Range from 8 to 11 for a four range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_8_11 = 5,
   /** Range from 12 to 15 for a four range profile entry. */
   IFX_TAPI_PKT_AAL_PROFILE_RANGE_12_15 = 6
} IFX_TAPI_PKT_AAL_PROFILE_RANGE_t;

/** Jitter buffer types. */
typedef enum
{
   /** Fixed Jitter Buffer. */
   IFX_TAPI_JB_TYPE_FIXED   = 0x1,
   /** Adaptive Jitter Buffer. */
   IFX_TAPI_JB_TYPE_ADAPTIVE = 0x2
} IFX_TAPI_JB_TYPE_t;

/** Jitter buffer packet adaption. */
typedef enum
{
   /** Reserved. */
   IFX_TAPI_JB_PKT_ADAPT_RES1 = 0,
   /** Reserved. */
   IFX_TAPI_JB_PKT_ADAPT_RES2 = 1,
   /** Packet adaption voice. */
   IFX_TAPI_JB_PKT_ADAPT_VOICE = 2,
   /** Packet adaption data. */
   IFX_TAPI_JB_PKT_ADAPT_DATA = 3
} IFX_TAPI_JB_PKT_ADAPT_t;

/** Out of band or in band definition. */
typedef enum
{
   /** Device default setting. */
   IFX_TAPI_PKT_EV_OOB_DEFAULT = 0,
   /** No event packets, DTMF inband. */
   IFX_TAPI_PKT_EV_OOB_NO = 1,
   /** Event packets, auto suppression of dtmf tones.*/
   IFX_TAPI_PKT_EV_OOB_ONLY = 2,
   /** Event packets, no auto suppression of dtmf tones. */
   IFX_TAPI_PKT_EV_OOB_ALL = 3,
   /** Block event transmission: neither in-band nor out-of-band. */
   IFX_TAPI_PKT_EV_OOB_BLOCK = 4
} IFX_TAPI_PKT_EV_OOB_t;

/** Defines the playout of received RFC2833 event packets. */
typedef enum
{
   /** Device default setting. Not recommended. */
   IFX_TAPI_PKT_EV_OOBPLAY_DEFAULT = 0,
   /** All RFC 2833 packets coming from the net are played out. Upstream and
       downstream RFC 2833 packets have the same payload type. */
   IFX_TAPI_PKT_EV_OOBPLAY_PLAY = 1,
   /** All RFC 2833 packets coming from the net are muted.*/
   IFX_TAPI_PKT_EV_OOBPLAY_MUTE = 2,
   /** All RFC 2833 packets coming from the net are played out. Upstream and
       downstream RFC 2833 packets have different payload types. */
   IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY = 3
}IFX_TAPI_PKT_EV_OOBPLAY_t;

/*@}*/ /* TAPI_INTERFACE_CON */

/* ======================================================================== */
/* TAPI Miscellaneous Services, enumerations (Group TAPI_INTERFACE_MISC)    */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_MISC */
/*@{*/

/** Debug trace levels. */
typedef enum
{
   /** Report off. */
   IFX_TAPI_DEBUG_REPORT_SET_OFF = 0,
   /** Low level report. */
   IFX_TAPI_DEBUG_REPORT_SET_LOW = 1,
   /** Normal level report. */
   IFX_TAPI_DEBUG_REPORT_SET_NORMAL = 2,
   /** High level report. */
   IFX_TAPI_DEBUG_REPORT_SET_HIGH = 3
} IFX_TAPI_DEBUG_REPORT_SET_t;

/** Enumeration for hook status events. */
typedef enum
{
   /** Hook detected. */
   IFX_TAPI_LINE_HOOK_STATUS_HOOK    = 0x01,
   /** Hook flash detected.*/
   IFX_TAPI_LINE_HOOK_STATUS_FLASH   = 0x02,
   /** Detected hook event is an off-hook. */
   IFX_TAPI_LINE_HOOK_STATUS_OFFHOOK = 0x04
} IFX_TAPI_LINE_HOOK_STATUS_t;

/** Enumeration for phone line status information. */
typedef enum
{
   /** Line is ringing. */
   IFX_TAPI_LINE_STATUS_RINGING = 0x01,
   /** Ringing finished. */
   IFX_TAPI_LINE_STATUS_RINGFINISHED = 0x02,
   /** Fax detected -> is replaced by signal. */
   IFX_TAPI_LINE_STATUS_FAX = 0x04,
   /** Ground key detected. */
   IFX_TAPI_LINE_STATUS_GNDKEY = 0x10,
   /** Ground key high detected. */
   IFX_TAPI_LINE_STATUS_GNDKEYHIGH = 0x20,
   /** Over temperature detected. */
   IFX_TAPI_LINE_STATUS_OTEMP = 0x40,
   /** Ground key polarity detected. */
   IFX_TAPI_LINE_STATUS_GNDKEYPOL = 0x80,
   /** GR909 result is available and can be queried with the appropriate
       interface. */
   IFX_TAPI_LINE_STATUS_GR909RES = 0x100,
   /** Caller id received. */
   IFX_TAPI_LINE_STATUS_CIDRX    = 0x200,
   /** Event occurs when the line mode may be switched to
       \ref IFX_TAPI_LINE_FEED_NORMAL_LOW to save power. */
   IFX_TAPI_LINE_STATUS_FEEDLOWBATT = 0x400
} IFX_TAPI_LINE_STATUS_t;

/** Enumeration for runtime errors. */
typedef enum
{
   /** No error. */
   IFX_TAPI_RT_ERROR_NONE = 0x0,
   /** Ring cadence settings error in cid tx. */
   IFX_TAPI_RT_ERROR_RINGCADENCE_CIDTX = 0x1,
   /** No acknowledge during CID sequence. */
   IFX_TAPI_RT_ERROR_CIDTX_NOACK = 0x2,
   /** No 2nd acknowledge during NTT CID onhook tx sequence. This indicates a
    missing "incoming successful signal". */
   IFX_TAPI_RT_ERROR_CIDTX_NOACK2 = 0x4
} IFX_TAPI_RUNTIME_ERROR_t;

/** Enumeration used for phone capability types. */
typedef enum
{
   /** Capability type: representation of the vendor. */
   IFX_TAPI_CAP_TYPE_VENDOR = 0,
   /** Capability type: representation of the Underlying device. */
   IFX_TAPI_CAP_TYPE_DEVICE = 1,
   /** Capability type: information about available ports. */
   IFX_TAPI_CAP_TYPE_PORT = 2,
   /** Capability type: vocoder type. */
   IFX_TAPI_CAP_TYPE_CODEC = 3,
   /** Capability type: DSP functionality available. */
   IFX_TAPI_CAP_TYPE_DSP = 4,
   /** Capability type: number of PCM modules.*/
   IFX_TAPI_CAP_TYPE_PCM = 5,
   /** Capability type: number of coder modules. */
   IFX_TAPI_CAP_TYPE_CODECS = 6,
   /** Capability type: number of analog interfaces. */
   IFX_TAPI_CAP_TYPE_PHONES = 7,
   /** Capability type: number of signaling modules. */
   IFX_TAPI_CAP_TYPE_SIGDETECT = 8,
   /** Capability type: T.38 support. */
   IFX_TAPI_CAP_TYPE_T38 = 9,
   /** Device version, the version is returned in one integer, where the upper
       byte defines the major and the lower byte the minor version.*/
   IFX_TAPI_CAP_TYPE_DEVVERS = 10,
   /** Capability type: Device type. */
   IFX_TAPI_CAP_TYPE_DEVTYPE = 11,
   /** Capability type: DECT support. */
   IFX_TAPI_CAP_TYPE_DECT = 12
} IFX_TAPI_CAP_TYPE_t;

/** Lists the ports for the capability list. */
typedef enum
{
   /** POTS port available. */
   IFX_TAPI_CAP_PORT_POTS    = 0,
   /** PSTN port available. */
   IFX_TAPI_CAP_PORT_PSTN    = 1,
   /** Handset port available. */
   IFX_TAPI_CAP_PORT_HANDSET = 2,
   /** Speaker port available. */
   IFX_TAPI_CAP_PORT_SPEAKER = 3
} IFX_TAPI_CAP_PORT_t;

/** Lists the signal detectors for the capability list. */
typedef enum
{
   /** Signal detection for CNG is available. */
   IFX_TAPI_CAP_SIG_DETECT_CNG = 0,
   /** Signal detection for CED is available. */
   IFX_TAPI_CAP_SIG_DETECT_CED = 1,
   /** Signal detection for DIS is available. */
   IFX_TAPI_CAP_SIG_DETECT_DIS = 2,
   /** Signal detection for line power is available. */
   IFX_TAPI_CAP_SIG_DETECT_POWER = 3,
   /** Signal detection for CPT is available. */
   IFX_TAPI_CAP_SIG_DETECT_CPT = 4,
   /** Signal detection for V8.bis is available. */
   IFX_TAPI_CAP_SIG_DETECT_V8BIS = 5
} IFX_TAPI_CAP_SIG_DETECT_t;

/** Defines the device types. */
typedef enum
{
   /** Device of the DuSLIC family. */
   IFX_TAPI_DEV_TYPE_DUSLIC  = 0,
   /** Device of the DuSLIC-S family. */
   IFX_TAPI_DEV_TYPE_DUSLIC_S = 1,
   /** Device of the DuSLIC-xT family. */
   IFX_TAPI_DEV_TYPE_DUSLIC_XT  = 2,
   /** Device of the VINETIC family. */
   IFX_TAPI_DEV_TYPE_VINETIC = 3,
   /** Device of the INCA-IP family. */
   IFX_TAPI_DEV_TYPE_INCA_IPP = 4,
   /** Device of the VINETIC-CPE family. */
   IFX_TAPI_DEV_TYPE_VINETIC_CPE = 5,
   /** Device of the type voice macro on MIPS (Danube). */
   IFX_TAPI_DEV_TYPE_VOICE_MACRO = 6,
   /** Device of the VINETIC-S. */
   IFX_TAPI_DEV_TYPE_VINETIC_S = 7,
   /** Device of type Voice Gateway. */
   IFX_TAPI_DEV_TYPE_VOICESUB_GW = 8,
   /** Device of the VINETIC SuperVIP. */
   IFX_TAPI_DEV_TYPE_VIN_SUPERVIP = 9
} IFX_TAPI_DEV_TYPE_t;

/*@}*/ /* TAPI_INTERFACE_MISC */

/* ======================================================================= */
/* TAPI Power Ringing Services, enumerations (Group TAPI_INTERFACE_RINGING)      */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_RINGING */
/*@{*/

/** Ring Configuration Mode. */
typedef enum
{
   /** Internal balanced. */
   IFX_TAPI_RING_CFG_MODE_INTERNAL_BALANCED = 0,
   /** Internal unbalanced ROT. */
   IFX_TAPI_RING_CFG_MODE_INTERNAL_UNBALANCED_ROT = 1,
   /** Internal unbalanced ROR. */
   IFX_TAPI_RING_CFG_MODE_INTERNAL_UNBALANCED_ROR = 2,
   /** External SLIC current sense. */
   IFX_TAPI_RING_CFG_MODE_EXTERNAL_IT_CS = 3,
   /** External IO current sense. */
   IFX_TAPI_RING_CFG_MODE_EXTERNAL_IO_CS = 4
} IFX_TAPI_RING_CFG_MODE_t;

/** Ring Configuration SubMode. */
typedef enum
{
   /** DC Ring Trip standard. */
   IFX_TAPI_RING_CFG_SUBMODE_DC_RNG_TRIP_STANDARD = 0,
   /** DC Ring Trip fast. */
   IFX_TAPI_RING_CFG_SUBMODE_DC_RNG_TRIP_FAST = 1,
   /** AC Ring Trip standard. */
   IFX_TAPI_RING_CFG_SUBMODE_AC_RNG_TRIP_STANDARD = 2,
   /** AC Ring Trip fast. */
   IFX_TAPI_RING_CFG_SUBMODE_AC_RNG_TRIP_FAST = 3
} IFX_TAPI_RING_CFG_SUBMODE_t;

/*@}*/ /* TAPI_INTERFACE_RINGING */

/* ==================================================================== */
/* TAPI PCM Services, enumerations (Group TAPI_INTERFACE_PCM)           */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_PCM */
/*@{*/

/** Defines the coding for the PCM channel. */
typedef enum
{
   /** G.711 A-Law 8 bit, narrowband.
       This resolution requires 1 PCM timeslot. */
   IFX_TAPI_PCM_RES_NB_ALAW_8BIT = 0,
   /** G.711 u-Law 8 bit, narrowband.
       This resolution requires 1 PCM timeslot. */
   IFX_TAPI_PCM_RES_NB_ULAW_8BIT = 1,
   /** Linear 16 bit, narrowband.
       This resolution requires 2 consecutive PCM timeslots. */
   IFX_TAPI_PCM_RES_NB_LINEAR_16BIT = 2,
   /** G.711 A-Law 8 bit, wideband.
       This resolution requires 2 consecutive PCM timeslots. */
   IFX_TAPI_PCM_RES_WB_ALAW_8BIT = 3,
   /** G.711 u-Law 8 bit, wideband.
       This resolution requires 2 consecutive PCM timeslots. */
   IFX_TAPI_PCM_RES_WB_ULAW_8BIT = 4,
   /** Linear 16 bit, wideband.
       This resolution requires 4 consecutive PCM timeslots. */
   IFX_TAPI_PCM_RES_WB_LINEAR_16BIT = 5
} IFX_TAPI_PCM_RES_t;

/* map the old names to the NB names */
#define IFX_TAPI_PCM_RES_ALAW_8BIT   IFX_TAPI_PCM_RES_NB_ALAW_8BIT
#define IFX_TAPI_PCM_RES_ULAW_8BIT   IFX_TAPI_PCM_RES_NB_ULAW_8BIT
#define IFX_TAPI_PCM_RES_LINEAR_16BIT   IFX_TAPI_PCM_RES_NB_LINEAR_16BIT

/*@}*/ /* TAPI_INTERFACE_PCM */

/* =================================================================== */
/* TAPI Fax T.38 Services, enumerations (Group TAPI_INTERFACE_FAX)     */
/* =================================================================== */
/** \addtogroup TAPI_INTERFACE_FAX */
/*@{*/

/** T38 Fax Datapump states. */
typedef enum
{
   /** Fax Datapump not active. */
   IFX_TAPI_FAX_T38_DP_OFF = 0x1,
   /** Fax Datapump active. */
   IFX_TAPI_FAX_T38_DP_ON  = 0x2,
   /** Fax transmission is active. */
   IFX_TAPI_FAX_T38_TX_ON  = 0x4,
   /** Fax transmission is not active. */
   IFX_TAPI_FAX_T38_TX_OFF = 0x8
} IFX_TAPI_FAX_T38_STATUS_t;

/** T38 Fax errors. */
typedef enum
{
   /** No Error. */
   IFX_TAPI_FAX_T38_ERROR_NONE = 0,
   /** Error occured : Deactivate Datapump. */
   IFX_TAPI_FAX_T38_ERROR_DATAPUMP = 1,
   /** MIPS Overload. */
   IFX_TAPI_FAX_T38_ERROR_MIPS_OVLD = 2,
   /** Error while reading data. */
   IFX_TAPI_FAX_T38_ERROR_READ = 3,
   /** Error while writing data. */
   IFX_TAPI_FAX_T38_ERROR_WRITE = 4,
   /** Error while setting up modulator or demodulator. */
   IFX_TAPI_FAX_T38_ERROR_SETUP = 5
} IFX_TAPI_FAX_T38_ERROR_t;

/** T38 Fax standard and rate. */
typedef enum
{
   /** V.21 */
   IFX_TAPI_T38_STD_V21       = 0x1,
   /** V.27/2400 */
   IFX_TAPI_T38_STD_V27_2400  = 0x2,
   /** V.27/4800 */
   IFX_TAPI_T38_STD_V27_4800  = 0x3,
   /** V.29/7200 */
   IFX_TAPI_T38_STD_V29_7200  = 0x4,
   /** V.29/9600 */
   IFX_TAPI_T38_STD_V29_9600  = 0x5,
   /** V.17/7200 */
   IFX_TAPI_T38_STD_V17_7200  = 0x6,
   /** V.17/9600 */
   IFX_TAPI_T38_STD_V17_9600  = 0x7,
   /** V.17/12000 */
   IFX_TAPI_T38_STD_V17_12000 = 0x8,
   /** V.17/14400 */
   IFX_TAPI_T38_STD_V17_14400 = 0x9
} IFX_TAPI_T38_STD_t;

/*@}*/ /* TAPI_INTERFACE_FAX */

/* ==================================================================== */
/* TAPI DECT Services, enumerations (Group TAPI_INTERFACE_DECT)         */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_DECT */
/*@{*/

/** DECT encoding type. */
typedef enum
{
   IFX_TAPI_DECT_ENC_TYPE_NONE = 0,
   /** G.711 A-Law 64 kbit/s. */
   IFX_TAPI_DECT_ENC_TYPE_G711_ALAW = 1,
   /** G.711 u-Law 64 kbit/s. */
   IFX_TAPI_DECT_ENC_TYPE_G711_MLAW = 2,
   /** G.726 32 kbit/s. */
   IFX_TAPI_DECT_ENC_TYPE_G726_32 = 3,
   /** G.722 64 kbit/s (wideband). */
   IFX_TAPI_DECT_ENC_TYPE_G722_64 = 4
} IFX_TAPI_DECT_ENC_TYPE_t;

/** DECT packet sizes. */
typedef enum
{
   /** Packet size 2.5ms. */
   IFX_TAPI_DECT_ENC_LENGTH_2_5 = 1,
   /** Packet size 5ms. */
   IFX_TAPI_DECT_ENC_LENGTH_5 = 2,
   /** Packet size 10ms. */
   IFX_TAPI_DECT_ENC_LENGTH_10 = 3
} IFX_TAPI_DECT_ENC_LENGTH_t;

/*@}*/ /* TAPI_INTERFACE_DECT */

/* ========================================================================= */
/*                      TAPI Interface Structures                            */
/* ========================================================================= */

/* ======================================================================== */
/* TAPI Initialization Services, structures (Group TAPI_INTERFACE_INIT)     */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_INIT */
/*@{*/

/** TAPI initialization structure used for IFX_TAPI_CH_INIT_t. */
typedef struct
{
   /** TAPI initialization structure used for \ref IFX_TAPI_CH_INIT.

   Channel initialization mode, to be selected from \ref IFX_TAPI_INIT_MODE_t
   It should be always set to \ref IFX_TAPI_INIT_MODE_VOICE_CODER.*/
   unsigned char nMode;
   /** Reserved. Country selection, for future purposes. */
   unsigned char nCountry;
   /**Pointer to the low-level device initialization structure (for example
      VMMC_IO_INIT for INCA-IP2). For details please refer to the device
      specific driver documentation. */
   void *pProc;
} IFX_TAPI_CH_INIT_t;

/*@}*/ /* TAPI_INTERFACE_INIT */

/* ======================================================================== */
/* TAPI Operation Control Services, structures (Group TAPI_INTERFACE_OP)    */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_OP */
/*@{*/

/** Structure used to configure volume settings.*/
typedef struct
{
   /** Volume setting for the receiving path.
       The value is given in dB with the range (-24dB ... 24dB), 1dB step.*/
   int nGainRx;
   /** Volume setting for the transmitting path.
       The value is given in dB with the range (-24dB ... 24dB), 1dB step. */
   int nGainTx;
} IFX_TAPI_LINE_VOLUME_t;

/** Packet path volume settings. */
typedef struct
{
   /** Volume setting for the encoding path. The value is given in dB with
       the range (-24 dB to 24 dB), 1 dB step. */
   int nEnc;
   /** Volume setting for the decoding path. The value is given in dB with
       the range (-24 dB to 24 dB), 1 dB step. */
   int nDec;
} IFX_TAPI_PKT_VOLUME_t;

/** Structure used for validation times of hook, hook flash and pulse dialing.
    An example of typical timing:
       - 80 ms <= flash time <= 200 ms
       - 30 ms <= digit low time <= 80 ms
       - 30 ms <= digit high time <= 80 ms
       - Interdigit time = 300 ms
       - Off-hook time = 40 ms
       - On-hook time = 400 ms!!! open: only min. time is validated and pre
        initialized.
*/
typedef struct
{
   /** Type of validation time setting. */
   IFX_TAPI_LINE_HOOK_VALIDATION_TYPE_t nType;
   /** Minimum time for validation in ms. */
   unsigned long         nMinTime;
   /** maximum time for validation in ms. */
   unsigned long         nMaxTime;
} IFX_TAPI_LINE_HOOK_VT_t;

/** Line echo canceller (LEC) configuration.
   \ remarks
   This Structure is  obsolete, it is only preserved for backward compatibility.
   Please use \ref IFX_TAPI_WLEC_PHONE_CFG_SET ioctl for new development. */
typedef struct
{
   /** Gain for input
    - 0: IFX_TAPI_LEC_GAIN_OFF, LEC Off
    - 1: IFX_TAPI_LEC_GAIN_LOW, Low Gain
    - 2: IFX_TAPI_LEC_GAIN_MEDIUM,  Medium Gain
    - 3: IFX_TAPI_LEC_GAIN_HIGH, High Gain    */
   char nGainIn;
   /** Gain for ouput
   - 0: IFX_TAPI_LEC_GAIN_OFF, LEC Off
   - 1: IFX_TAPI_LEC_GAIN_LOW, Low Gain
   - 2: IFX_TAPI_LEC_GAIN_MEDIUM, Medium Gain
   - 3: IFX_TAPI_LEC_GAIN_HIGH, High Gain    */
   char nGainOut;
   /** LEC tail length in milliseconds. nLen is currently not supported
       and should be set to zero. */
   char nLen;
   /** Switch the NLP on or off.

   - 0: IFX_TAPI_LEC_NLP_DEFAULT, NLP is default
   - 1: IFX_TAPI_LEC_NLP_ON, NLP is on
   - 2: IFX_TAPI_LEC_NLP_OFF, NLP is off   */
   char bNlp;
} IFX_TAPI_LEC_CFG_t;

/** Advanced Line echo canceller (NLEC/WLEC) configuration. */
typedef struct
{
   /** LEC Type selection.
    - 0x00: IFX_TAPI_WLEC_TYPE_OFF
    - 0x01: IFX_TAPI_WLEC_TYPE_NE
    - 0x02: IFX_TAPI_WLEC_TYPE_NFE
   */
   IFX_TAPI_WLEC_TYPE_t nType;

   /** Switch the NLP on or off.
    - 1: IFX_TAPI_WLEC_NLP_ON, NLP is on
    - 2: IFX_TAPI_WLEC_NLP_OFF, NLP is off   */
   IFX_TAPI_WLEC_NLP_t bNlp;

   /** Size of the near-end window in narrowband (8 kHz) sampling mode.
       For backward compatibility a value of 0 defaults to:
        16 ms if nType is \ref IFX_TAPI_WLEC_TYPE_NE or
         8 ms if nType is \ref IFX_TAPI_WLEC_TYPE_NFE */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBNEwindow;

   /** Size of the far-end window in narrowband (8 kHz) sampling mode.
       A value of 0 defaults to 8 ms for backward compatibility.
       Note: this is used only if nType is set to \ref IFX_TAPI_WLEC_TYPE_NFE */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBFEwindow;

   /** Size of the near-end window in wideband (16 kHz) sampling mode.
       For backward compatibility a value of 0 defaults to 8 ms. */
   IFX_TAPI_WLEC_WIN_SIZE_t   nWBNEwindow;
} IFX_TAPI_WLEC_CFG_t;

/*@}*/ /* TAPI_INTERFACE_OP */

/* ===================================================================== */
/* TAPI Metering Services, structures (Group TAPI_INTERFACE_METER)       */
/* ===================================================================== */
/** \addtogroup TAPI_INTERFACE_METER */
/*@{*/

/** Structure for the configuration of metering. */
typedef struct
{
   /** Metering mode:
   - 0: TAPI_METER_MODE_TTX, TTX mode
   - 1: IFX_TAPI_METER_MODE_REVPOL, reverse polarity  */
   unsigned char             mode;
   /** Reserved. Metering frequency.*/
   unsigned char             freq;
   /** Length of metering burst in ms. burst_len must be greater than zero. */
   unsigned long            burst_len;
   /** Distance between the metering bursts in seconds. */
   unsigned long            burst_dist;
   /** Defines the number of bursts. */
   unsigned long            burst_cnt;
} IFX_TAPI_METER_CFG_t;

/*@}*/ /* TAPI_INTERFACE_METER */

/* ==================================================================== */
/* TAPI Tone Control Services, structures (Group TAPI_INTERFACE_TONE)   */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_TONE */
/*@{*/

/** Structure used for tone level configuration. */
typedef struct
{
   /** Defines the tone generator number whose level has to be modified.
   - 0x1: IFX_TAPI_TONE_TG1, generator one is used
   - 0x2: IFX_TAPI_TONE_TG2, generator two is used
   - 0xFF: IFX_TAPI_TONE_TGALL, all tone generator are affected. */
   unsigned char      nGenerator;
   /** Defines the tone level: value 0x100 describes the default record
   level of -38.79 dB. Maximum value is 0x7FFF which corresponds to a
   record level of approximately 3 dB.
   The tone level in dB can be calculated via formula:
   level_in_dB = +3.17 + 20 log10 (level / 32767) */
   unsigned long      nLevel;
} IFX_TAPI_PREDEF_TONE_LEVEL_t;

/** Structure used to define simple tone characteristics. */
typedef struct
{
    /** Indicates the type of the tone descriptor:
    - 1:  IFX_TAPI_TONE_TYPE_SIMPLE the tone descriptor describes a simple tone.
    - 2: IFX_TAPI_TONE_TYPE_COMPOSED the tone descriptor describes a
         composed tone. */
    IFX_TAPI_TONE_TYPE_t    format;
    /** Name of the simple tone. */
    char name[30];
    /** Tone code ID; 0< ID <255. */
    unsigned int index;
    /** Number of times to play the simple tone, 0 < loop < 8.
        The loop count if not equal 0 defines the time of the entire sequence:
        tone_seq = loop * (onA + offA + onB + offB + onC + offC + pause)*/
    unsigned int loop;
     /** Power level for frequency A in 0.1 dB steps; -300 < levelA < 0. */
    int levelA;
     /** Power level for frequency B in 0.1 dB steps; -300 < levelB < 0.*/
    int levelB;
     /** Power level for frequency C in 0.1 dB steps; -300 < levelC < 0. */
    int levelC;
    /** Power level for frequency D in 0.1 dB steps; -300 < levelD < 0. */
    int levelD;
    /** Tone frequency A in Hz; 0 <= Hz < 4000. */
    unsigned int freqA;
    /** Tone frequency B in Hz; 0 <= Hz < 4000. */
    unsigned int freqB;
    /** Tone frequency C in Hz; 0 <= Hz < 4000. */
    unsigned int freqC;
    /** Tone frequency D in Hz; 0 <= Hz < 4000. */
    unsigned int freqD;
    /**
      IFX_TAPI_TONE_STEPS_MAX Array defining time duration
      for each cadence steps, with 2 ms granularity.
      0 <= cadence <= 32000.
      The first cadence[X] = 0 (starting from X=1) in the array indicates that
      X-1 cadences must be played.
      A tone with cadence[0]=0 can not be processed! */
    unsigned int cadence[IFX_TAPI_TONE_STEPS_MAX];
    /** Active frequencies for the cadence steps. More than one frequency
      can be active in the same cadence step. All active frequencies are
      summed together.
      - 0x0:  IFX_TAPI_TONE_FREQNONE No frequencies.
      - 0x01: IFX_TAPI_TONE_FREQA Frequency A is enabled.
      - 0x02: IFX_TAPI_TONE_FREQB Frequency B is enabled.
      - 0x04: IFX_TAPI_TONE_FREQC Frequency C is enabled.
      - 0x08: IFX_TAPI_TONE_FREQD Frequency D is enabled.
      - 0x0F: IFX_TAPI_TONE_FREQALL All frequencies are enabled. */
    unsigned int frequencies[IFX_TAPI_TONE_STEPS_MAX];
    /** Array containing selection for each cadence steps, whether to
      enable/disable modulation of frequency A with frequency B. Defined
      values for each array entry:
    - 0x0: IFX_TAPI_TONE_MODULATION_OFF Modulation of frequency A with
           frequency B is disabled.
    - 0x1: IFX_TAPI_TONE_MODULATION_ON Modulation of frequency A with
           frequency B is enabled. */
    unsigned int modulation[IFX_TAPI_TONE_STEPS_MAX];
    /** Some tones require an off time at the end of the tone. The offtime will
      be added to the last used cadence. Therefore the maximum value for
      offtime is 32000 - "last used cadence" and have the granularity of
      2 ms;
      0 < 32000 - "last used cadence" < 32000.*/
    unsigned int pause;
} IFX_TAPI_TONE_SIMPLE_t;

/** Structure for definition of composed tones. */
typedef struct
{
    /** Indicate the type of the tone descriptor:
    - 1: IFX_TAPI_TONE_TYPE_SIMPLE, The tone descriptor describes a simple tone
    - 2: IFX_TAPI_TONE_TYPE_COMPOSED, The tone descriptor describes a composed
        tone. */
    IFX_TAPI_TONE_TYPE_t    format;
    /** Name of the composed tone. */
    char name[30];
    /** Tone code ID; 0< ID <255.*/
    unsigned int index;
    /** Number of times to play the tone sequence, 0 for infinite.
        Maximum 7. */
    unsigned int loop;
    /** Indicate if the voice path is active between the loops.*/
    unsigned int alternatVoicePath;
    /** Number of simple tones used in the composed tone. */
    unsigned int count;
    /** Simple tones to be used. The simple tones are played in the same order
        as they are stored in the array.
        Note: In order to create composed tones only simple tones with a finite
        loop count can be used.*/
    unsigned int tones [IFX_TAPI_TONE_SIMPLE_MAX];
} IFX_TAPI_TONE_COMPOSED_t;

/** Structure for definition of Dual tones. */
typedef struct
{
   IFX_TAPI_TONE_TYPE_t    format;

   int levelA;
   int levelB;

   unsigned int freqA;
   unsigned int freqB;
} IFX_TAPI_TONE_DUAL_t;

/** Tone descriptor. */
typedef union
{
   /** The parameter points to a \ref IFX_TAPI_TONE_SIMPLE_t structure.*/
   IFX_TAPI_TONE_SIMPLE_t   simple;
   /**  The parameter points to a \ref IFX_TAPI_TONE_COMPOSED_t structure.*/
   IFX_TAPI_TONE_COMPOSED_t composed;
   /** Descriptor for dual tone. */
   IFX_TAPI_TONE_DUAL_t     dual;

} IFX_TAPI_TONE_t;
/*@}*/ /* TAPI_INTERFACE_TONE */

/* ======================================================================= */
/* TAPI Dial Services, structures (Group TAPI_INTERFACE_DIAL)              */
/* ======================================================================= */
/** \addtogroup TAPI_INTERFACE_DIAL */
/*@{*/

/*@}*/ /* TAPI_INTERFACE_DIAL */

/* ======================================================================== */
/* TAPI Signal Detection Services, structures                               */
/* (Group TAPI_INTERFACE_SIGNAL)                                            */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_SIGNAL */
/*@{*/

/** Structure used for enable and disable signal detection. See
   \ref IFX_TAPI_SIG_DETECT_ENABLE and \ref IFX_TAPI_SIG_DETECT_DISABLE .
   The signals are reported via an TAPI exception service. */
typedef struct
{
   /** Signals to detect. Can be any combination of \ref IFX_TAPI_SIG_t. */
   unsigned long sig;
   /** Signals to detect. Can be any combination of \ref IFX_TAPI_SIG_EXT_t. */
   unsigned long sig_ext;
} IFX_TAPI_SIG_DETECTION_t;

/** DTMF Receiver coefficients settings.
    The following DTMF Receiver Coefficients are to be directly programmed
    in the underlying device. Therefore the passed values must be expressed
    in format ready for programming, as no interpretation of these values is
    attempted.
*/
typedef struct
{
   /** Minimal signal level (dB). */
   IFX_int32_t nLevel;
   /** Maximum allowed signal twist (dB). */
   IFX_int32_t nTwist;
   /** Gain adjustment of the input signal (dB). */
   IFX_int32_t nGain;
} IFX_TAPI_DTMF_RX_CFG_t;

/** Structure used for \ref IFX_TAPI_TONE_CPTD_START and
    \ref IFX_TAPI_TONE_CPTD_STOP. */
typedef struct
{
   /** The index of the tone to detect. The tone index must be programmed
   with IFX_TAPI_TONE_TABLE_CFG_SET. */
   int tone;
   /** The specification of the signal

      - 1: IFX_TAPI_TONE_CPTD_DIRECTION_RX,
           receive direction of the programmed CPT tone
      - 2: IFX_TAPI_TONE_CPTD_DIRECTION_TX,
           transmit direction of the programmed CPT tone */
   int signal;
} IFX_TAPI_TONE_CPTD_t;

/** Structure used to configure fax/modem coefficients. */
typedef struct
{
   /** Tone holding: minimum signal level (dB). Range: from -50 dB up to 0 dB.*/
   IFX_int32_t nHoldLevel;
   /** Tone holding: gap time (ms). Range: from 10 ms up to 100000 ms.*/
   IFX_int32_t nHoldGapTime;
} IFX_TAPI_MFTD_CFG_t;

/*@}*/ /* TAPI_INTERFACE_SIGNAL */

/* ==================================================================== */
/* TAPI CID Features Service, structures (Group TAPI_INTERFACE_CID)       */
/* ==================================================================== */
/** \addtogroup TAPI_INTERFACE_CID */
/*@{*/


/** Structure containing the timing for CID transmission. */
typedef struct
{
   /** Time to wait before data transmission, in ms.
      Default 300 ms. */
   unsigned int beforeData;
   /** Time to wait after data transmission, in ms, for onhook services.
      Default 300 ms. */
   unsigned int dataOut2restoreTimeOnhook;
   /** Time to wait after data transmission, in ms, for offhook services.
      Default 60 ms. */
   unsigned int dataOut2restoreTimeOffhook;
   /** Time to wait after ack detection, in ms.
      Default 55 ms. */
   unsigned int ack2dataOutTime;
   /** Time-out for ack detection, in ms.
      Default 200 ms. */
   unsigned int cas2ackTime;
   /** Time to wait after ACK time-out, in ms.
      Default 0 ms. */
   unsigned int afterAckTimeout;
   /** Time to wait after first ring, in ms, typically before data transmission.
      Default 600 ms. */
   unsigned int afterFirstRing;
   /** Time to wait after ring pulse, in ms, typically before data transmission.
      Default 500 ms. */
   unsigned int afterRingPulse;
   /** Time to wait after DTAS, in ms, typically before data transmission.
      Default 45 ms. */
   unsigned int afterDTASOnhook;
   /** Time to wait after line reversal in ms, typically before data transmission.
      Default 100 ms. */
   unsigned int afterLineReversal;
   /** Time to wait after OSI signal, in ms.
      Default 300 ms. */
   unsigned int afterOSI;
} IFX_TAPI_CID_TIMING_t;

/** Structure containing the configuration information for FSK transmitter and
   receiver. */
typedef struct
{
   /** Signal level for FSK transmission in 0.1 dB steps. Default -140
    (-14 dB).*/
   int            levelTX;
   /** Minimum signal level for FSK reception in 0.1 dB steps. Default -150
    (-15 dB).*/
   int            levelRX;
   /** Number of seizure bits for FSK transmission. This field is relevant only
   for on-hook transmission. Default value:
   - NTT standard: 0 bits.
   - Other standards: 300 bits.*/
   unsigned int   seizureTX;
   /** Minimum number of seizure bits for FSK reception. This field is relevant
   only for on-hook transmission. Default value:
   - NTT standard: 0 bits.
   - Other standards: 200 bits.*/
   unsigned int   seizureRX;
   /** Number of mark bits for on-hook FSK transmission. Default value:
   - NTT standard: 72 bits.
   - Other standards: 180 bits.*/
   unsigned int   markTXOnhook;
   /** Number of mark bits for off-hook FSK transmission. Default value:
   - NTT standard: 72 bits.
   - Other standards: 80 bits.*/
   unsigned int   markTXOffhook;
   /** Minimum number of mark bits for on-hook FSK reception. Default value:
   - NTT standard: 50 bits.
   - Other standards: 150 bits.*/
   unsigned int   markRXOnhook;
   /** Minimum number of mark bits for off-hook FSK reception. Default value:
   - NTT standard: 50 bits.
   - Other standards: 55 bits.*/
   unsigned int   markRXOffhook;
} IFX_TAPI_CID_FSK_CFG_t;

/** Structure containing the configuration information for DTMF CID. */
typedef struct
{
   /** Tone id for starting tone. Default is DTMF A. */
   char           startTone;
   /** Tone id for stop tone. Default is DTMF C. */
   char           stopTone;
   /** Tone id for starting information tone. Default is DTMF B. */
   char           infoStartTone;
   /** Tone id for starting redirection tone. Default is DTMF D. */
   char           redirStartTone;
   /** Time for DTMF digit duration. Default is 50 ms. */
   unsigned int   digitTime;
   /** Time between DTMF digits in ms. Default is 50 ms. */
   unsigned int   interDigitTime;
} IFX_TAPI_CID_DTMF_CFG_t;

/** Structure containing CID configuration for ETSI standard using DTMF
 transmission.*/
typedef struct
{
   /** Length of the coded strings.*/
   unsigned int    len;
   /** String representing code for unavailable/unknown CLI. Default 00.*/
   unsigned char   unavailable[IFX_TAPI_CID_MSG_LEN_MAX];
   /** String representing code for private/withheld CLI. Default 01.*/
   unsigned char   priv[IFX_TAPI_CID_MSG_LEN_MAX];
} IFX_TAPI_CID_ABS_REASON_t;

/** Structure containing CID configuration for Telcordia standard. */
typedef struct
{
   /** Pointer to a structure containing timing information. If the parameter
    is not given, IFX_TAPI_CID_TIMING_t default values will be used. */
   IFX_TAPI_CID_TIMING_t    *pCIDTiming;
   /** Pointer to a structure containing FSK configuration parameters. If the
    parameter is not given, IFX_TAPI_CID_FSK_CFG_t default values will be used.*/
   IFX_TAPI_CID_FSK_CFG_t  *pFSKConf;
   /** Usage of OSI for offhook transmission. Default IFX_FALSE.*/
   unsigned int         OSIoffhook;
   /** Length of the OSI signal in ms. Default 200 ms.*/
   unsigned int         OSItime;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int         nAlertToneOnhook;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int         nAlertToneOffhook;
   /** DTMF ACK after CAS, used for offhook transmission. Default DTMF 'D'. */
   char                 ackTone;
} IFX_TAPI_CID_STD_TELCORDIA_t;

/** Structure containing CID configuration for ETSI standard using FSK
    transmission. */
typedef struct
{
   /** Pointer to a structure containing timing information. If the parameter
    is not given, IFX_TAPI_CID_TIMING_t default values will be used.*/
   IFX_TAPI_CID_TIMING_t       *pCIDTiming;
   /** Pointer to a structure containing FSK configuration parameters. If the
    parameter is not given, IFX_TAPI_CID_FSK_CFG_t default values will be used.*/
   IFX_TAPI_CID_FSK_CFG_t     *pFSKConf;
   /** Type of ETSI Alert of on-hook services associated to ringing
    (enumerated in IFX_TAPI_CID_ALERT_ETSI_t). Default
    IFX_TAPI_CID_ALERT_ETSI_FR.*/
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertRing;
   /** Type of ETSI Alert of on-hook services not associated to ringing
    (enumerated in IFX_TAPI_CID_ALERT_ETSI_t). Default
     IFX_TAPI_CID_ALERT_ETSI_RP.*/
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertNoRing;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int            nAlertToneOnhook;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int            nAlertToneOffhook;
   /** Duration of Ring Pulse, in ms, default 500 ms.*/
   unsigned int            ringPulseTime;
   /** DTMF ACK after CAS, used for off-hook transmission. Default DTMF is D.*/
   char                    ackTone;
} IFX_TAPI_CID_STD_ETSI_FSK_t;

/** Structure containing CID configuration for ETSI standard using DTMF
 transmission. */
typedef struct
{
   /** Pointer to a structure containing timing information. If the parameter
    is not given,\ref IFX_TAPI_CID_TIMING_t default values will be used. */
   IFX_TAPI_CID_TIMING_t       *pCIDTiming;
   /** Pointer to a structure containing DTMF configuration parameters. If the
    parameter is not given, \ref IFX_TAPI_CID_DTMF_CFG_t default values will be
   used. */
   IFX_TAPI_CID_DTMF_CFG_t     *pDTMFConf;
   /** Pointer to a structure containing the coding for
   absence reason of calling number.*/
   IFX_TAPI_CID_ABS_REASON_t   *pABSCLICode;
   /** Type of ETSI Alert of on-hook services associated to ringing (enumerated
    in \ref IFX_TAPI_CID_ALERT_ETSI_t). Default \ref IFX_TAPI_CID_ALERT_ETSI_FR.*/
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertRing;
   /** Type of ETSI Alert of on-hook services not associated to ringing
    (enumerated in \ref IFX_TAPI_CID_ALERT_ETSI_t). Default \ref
    IFX_TAPI_CID_ALERT_ETSI_RP. */
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertNoRing;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int                nAlertToneOnhook;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition. */
   unsigned int                nAlertToneOffhook;
   /** Duration of Ring Pulse, in ms, default is 500 ms. */
   unsigned int                ringPulseTime;
   /** DTMF ACK after CAS, used for offhook transmission. Default DTMF D. */
   char                        ackTone;
} IFX_TAPI_CID_STD_ETSI_DTMF_t;

/** Structure for the configuration of SIN standard. */
typedef struct
{
   /** Pointer to a structure containing timing information.
      If the parameter is NULL, default values will be used. */
   IFX_TAPI_CID_TIMING_t    *pCIDTiming;
   /** Pointer to a structure containing FSK configuration parameters.
      If the parameter is NULL, default values will be used. */
   IFX_TAPI_CID_FSK_CFG_t  *pFSKConf;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int          nAlertToneOnhook;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int          nAlertToneOffhook;
   /** DTMF ACK after CAS, used for offhook transmission. Default DTMF 'D'. */
   char                 ackTone;
} IFX_TAPI_CID_STD_SIN_t;

/** Structure containing CID configuration for NTT standard.*/
typedef struct
{
   /** Pointer to a structure containing timing information. If the parameter
    is not given, IFX_TAPI_CID_TIMING_t default values will be used.*/
   IFX_TAPI_CID_TIMING_t   *pCIDTiming;
   /** Pointer to a structure containing FSK configuration parameters. If the
    parameter is not given, IFX_TAPI_CID_FSK_CFG_t default values will be used.*/
   IFX_TAPI_CID_FSK_CFG_t     *pFSKConf;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int            nAlertToneOnhook;
   /** Tone table index for the alert tone to be used. Required for automatic
    CID/MWI generation. By default TAPI uses an internal tone definition.*/
   unsigned int            nAlertToneOffhook;
   /** Ring pulse on time (CAR signal), in ms, default 500 ms.*/
   unsigned int            ringPulseTime;
   /** Max number of ring pulses (CAR signals), default 5.*/
   unsigned int            ringPulseLoop;
   /** Ring pulse off time (CAR signal), in ms, default 500 ms.*/
   unsigned int            ringPulseOffTime;
   /** Timeout for incoming successful signal to arrive after CID data
    transmission is completed. Default 7000 ms.*/
   unsigned int            dataOut2incomingSuccessfulTimeout;
} IFX_TAPI_CID_STD_NTT_t;

/** Union of the CID configuration structures for different standards.*/
typedef union
{
   /** Structure defining configuration parameters for Telcordia standard. */
   IFX_TAPI_CID_STD_TELCORDIA_t   telcordia;
   /** Structure defining configuration parameters for ETSI standard, with FSK
    transmission.*/
   IFX_TAPI_CID_STD_ETSI_FSK_t    etsiFSK;
   /** Structure defining configuration parameters for ETSI standard, with DTMF
    transmission.*/
   IFX_TAPI_CID_STD_ETSI_DTMF_t   etsiDTMF;
   /** Structure defining configuration parameters for BT SIN standard. */
   IFX_TAPI_CID_STD_SIN_t         sin;
   /** Structure defining configuration parameters for NTT standard. */
   IFX_TAPI_CID_STD_NTT_t         ntt;
} IFX_TAPI_CID_STD_TYPE_t;

/** Structure containing CID configuration possibilities. */
typedef struct
{
   /** Standard used (enumerated in \ref IFX_TAPI_CID_STD_t). Default
    IFX_TAPI_CID_STD_TELCORDIA.*/
   IFX_TAPI_CID_STD_t       nStandard;
   /** Union of the different standards. Default IFX_TAPI_CID_STD_TELCORDIA_t.*/
   IFX_TAPI_CID_STD_TYPE_t  *cfg;
} IFX_TAPI_CID_CFG_t;

/** Structure for element types (\ref IFX_TAPI_CID_SERVICE_TYPE_t) containing
   date and time information.*/
typedef struct
{
   /** Element type. */
   IFX_TAPI_CID_SERVICE_TYPE_t elementType;
   /** Month. */
   unsigned int            month;
   /** Day. */
   unsigned int            day;
   /** Hour. */
   unsigned int            hour;
   /** Minute. */
   unsigned int            mn;
} IFX_TAPI_CID_MSG_DATE_t;

/** Structure for element types ( \ref IFX_TAPI_CID_SERVICE_TYPE_t) with
 dynamic length (line numbers or names).*/
typedef struct
{
   /** Element type. */
   IFX_TAPI_CID_SERVICE_TYPE_t elementType;
   /** Length of the message array. */
   unsigned int            len;
   /** String containing the message element. */
   unsigned char           element[IFX_TAPI_CID_MSG_LEN_MAX];
} IFX_TAPI_CID_MSG_STRING_t;

/** Structure for element types (\ref IFX_TAPI_CID_SERVICE_TYPE_t) with one
 value (length 1).
*/
typedef struct
{
   /** Element type. */
   IFX_TAPI_CID_SERVICE_TYPE_t elementType;
   /** Value for the message element. */
   unsigned char           element;
} IFX_TAPI_CID_MSG_VALUE_t;

/** Structure for service type transparent (\ref IFX_TAPI_CID_SERVICE_TYPE_t).*/
typedef struct
{
   /** Element type. */
   IFX_TAPI_CID_SERVICE_TYPE_t elementType;
   /** Element length. */
   unsigned int            len;
   /** Element buffer. */
   unsigned char           *data;
} IFX_TAPI_CID_MSG_TRANSPARENT_t;

/** Union of element types. */
typedef union
{
   /** Message element including date and time information. */
   IFX_TAPI_CID_MSG_DATE_t     date;
   /** Message element formatted as string.*/
   IFX_TAPI_CID_MSG_STRING_t   string;
   /** Message element formatted as value.*/
   IFX_TAPI_CID_MSG_VALUE_t    value;
   /** Message element to be sent with transparent transmission. */
   IFX_TAPI_CID_MSG_TRANSPARENT_t transparent;
} IFX_TAPI_CID_MSG_ELEMENT_t;


/** Structure containing the CID message type and content as well as
   information about transmission mode. This structure contains all information
   required by IFX_TAPI_CID_TX_INFO_START to start CID generation.*/
typedef struct
{
   /** Defines the Transmission Mode (enumerated in \ref IFX_TAPI_CID_HOOK_MODE_t).
    Default \ref IFX_TAPI_CID_HM_ONHOOK.*/
   IFX_TAPI_CID_HOOK_MODE_t    txMode;
   /** Defines the Message Type to be displayed (enumerated in \ref
   IFX_TAPI_CID_MSG_TYPE_t).*/
   IFX_TAPI_CID_MSG_TYPE_t messageType;
   /** Number of elements of the message array. */
   unsigned int            nMsgElements;
   /** Message array. */
   IFX_TAPI_CID_MSG_ELEMENT_t  *message;
} IFX_TAPI_CID_MSG_t;

/** Structure for caller id receiver status. */
typedef struct
{
   /** Caller id receiver actual status using \ref IFX_TAPI_CID_RX_STATUS_t

   - 0: IFX_TAPI_CID_RX_STATE_INACTIVE, CID Receiver is not active
   - 1: IFX_TAPI_CID_RX_STATE_ACTIVE, CID Receiver is active
   - 2: IFX_TAPI_CID_RX_STATE_ONGOING
   - 3: IFX_TAPI_CID_RX_STATE_DATA_READY
   */
   unsigned char nStatus;
   /** Caller id receiver actual error code using \ref IFX_TAPI_CID_RX_ERROR_t

   - 0: IFX_TAPI_CID_RX_ERROR_NONE
   - 1: IFX_TAPI_CID_RX_ERROR_READ
   */
   unsigned char nError;
} IFX_TAPI_CID_RX_STATUS_t;

/** Structure for caller id receiver data. */
typedef struct
{
   /** Caller id receiver data. */
   unsigned char data [IFX_TAPI_CID_RX_SIZE_MAX];
   /** Caller id receiver datasize in bytes. */
   unsigned int nSize;
} IFX_TAPI_CID_RX_DATA_t;

/*@}*/ /* TAPI_INTERFACE_CID */

/* ======================================================================== */
/* TAPI Connection Services, structures (Group TAPI_INTERFACE_CON)          */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_CON */
/*@{*/

/** Structure used for AAL configuration. */
typedef struct
{
   /** Connection identifier.*/
   unsigned short nCid;
   /** Start value for the timestamp (resolution of 125 s). */
   unsigned short nTimestamp;
   /** Connection Identifier for CPS events. */
   unsigned short nCpsCid;
} IFX_TAPI_PCK_AAL_CFG_t;

/** AAL profile setup structure. */
typedef struct
{
   /** Amount of rows to program. */
   char rows;
   /** Length of packet in bytes - 1. */
   char len[10];
   /** UUI codepoint range indicator, see \ref IFX_TAPI_PKT_AAL_PROFILE_RANGE_t. */
   char nUUI[10];
   /** Codec as listed for \ref IFX_TAPI_COD_TYPE_t. */
   char codec[10];
} IFX_TAPI_PCK_AAL_PROFILE_t;

/** Structure for RTP Configuration. RFC2833 event payload types (ePT) for the
    encoder and decoder part are also configured. Parameter nPlayEvents and
    nEventPlayPT are used to configure the payload type for the decoder part.
    \remarks
    The parameter nEventPlayPT is ignored if the firmware does not support
    configuration of different payload types for the decoder and encoder. */
typedef struct
{
   /** Start value for the sequence number.  */
   unsigned short nSeqNr;
   /** Synchronization source value for the voice and SID packets.
       Note: A change in the SSRC leads to a reset of the RTCP statistics.*/
   unsigned long nSsrc;
   /** Start value for the timestamp (resolution of 125 us).*/
   unsigned long nTimestamp;
   /** Defines how to handle RFC 2833 packets in upstream direction. Set
      parameter as defined in \ref IFX_TAPI_PKT_EV_OOB_t enumerator. */
   unsigned char  nEvents;
   /** Defines whether the received RFC 2833 packets have to be played out.
       Set parameter as defined in \ref IFX_TAPI_PKT_EV_OOBPLAY_t enumerator. */
   unsigned char  nPlayEvents;
   /** Payload type to be used for RFC 2833 frames in encoder direction
      (upstream). */
   unsigned char  nEventPT;
   /** Payload type to be used for RFC 2833 frames in decoder direction
      (downtream). */
   unsigned char  nEventPlayPT;
} IFX_TAPI_PKT_RTP_CFG_t;

/** Structure for RTP payload type configuration. */
typedef struct
{
   /** Table with all payload types, the coder type \ref IFX_TAPI_COD_TYPE_t
       is used as index. */
   unsigned char nPTup [IFX_TAPI_ENC_TYPE_MAX];
   /** Table with all payload types, the coder type \ref IFX_TAPI_COD_TYPE_t
       is used as index. */
   unsigned char nPTdown [IFX_TAPI_ENC_TYPE_MAX];
} IFX_TAPI_PKT_RTP_PT_CFG_t;

/** Structure for RTCP Statistics. It refers to the RFC3550/3551. */
typedef struct
{
   /** Sender generating this report. */
   unsigned long ssrc;
   /** NTP timestamp higher than 32 bits. This field is not filled by TAPI.*/
   unsigned long ntp_sec;
   /** NTP timestamp lower than 32 bits. This field is not filled by TAPI.*/
   unsigned long ntp_frac;
   /** RTP time stamp. */
   unsigned long   rtp_ts;
   /** Sent packet count. */
   unsigned long   psent;
   /** Sent octets count. */
   unsigned long   osent;
   /** Data source. */
   unsigned long   rssrc;
   /** Receivers fraction loss. */
   unsigned int   fraction:8;
   /** Receivers packet lost. */
   int      lost:24;
   /** Extended last seq nr. received.*/
   unsigned long   last_seq;
   /** Receives interarrival jitter. */
   unsigned long   jitter;
   /** Last sender report. This field is not filled by TAPI. */
   unsigned long   lsr;
   /** Delay since last sender report. This field is not filled by TAPI.*/
   unsigned long   dlsr;
} IFX_TAPI_PKT_RTCP_STATISTICS_t;

/** Structure used as parameter by \ref IFX_TAPI_ENC_CFG_SET. */
typedef struct
{
   /** Encoder type. */
   IFX_TAPI_COD_TYPE_t           nEncType;
   /** Frame length in milliseconds. */
   IFX_TAPI_COD_LENGTH_t         nFrameLen;
   /** Desc. missing */
   IFX_TAPI_COD_AAL2_BITPACK_t   AAL2BitPack;
} IFX_TAPI_ENC_CFG_t;

/** Structure used as parameter by \ref IFX_TAPI_DEC_CFG_SET. */
typedef struct
{
  /** Desc. missing */
   IFX_TAPI_COD_AAL2_BITPACK_t   AAL2BitPack;
} IFX_TAPI_DEC_CFG_t;


/** Start/stop event generation. */
typedef enum
{
   /** Stop event generation. */
   IFX_TAPI_EV_GEN_STOP=0,
   /** Start event generation. */
   IFX_TAPI_EV_GEN_START
} IFX_TAPI_PKT_EV_GEN_ACTION_t;

/** This structure is used to report a DTMF event to the TAPI from an external
   software module. */
typedef struct
{
   /** Event code according to RFC2833. */
   IFX_char_t   event;
   /** Start/tone event generation.*/
   IFX_TAPI_PKT_EV_GEN_ACTION_t   action;
   /** Duration of event in unit of 10 ms. 0 means forever. */
   IFX_char_t   duration;
} IFX_TAPI_PKT_EV_GENERATE_t;

/** This structure is used to configure support for the reporting of external
   DTMF events. */
typedef struct
{
   /** Enable or disable local play of the DTMF tone. */
   IFX_boolean_t   local;
} IFX_TAPI_PKT_EV_GENERATE_CFG_t ;

/** Structure for jitter buffer configuration used by  \ref IFX_TAPI_JB_CFG_SET.
   \remarks
    This structure may be changed in the future. */
typedef struct
{
   /** Jitter buffer type, value of \ref IFX_TAPI_JB_TYPE_t. */
   unsigned char    nJbType;
   /** Packet adaptation, value of \ref IFX_TAPI_JB_PKT_ADAPT_t. */
   char     nPckAdpt;
   /** Local adaptation, value of \ref IFX_TAPI_JB_LOCAL_ADAPT_t. Relevant
    only for adaptive jitter buffer. */
   char     nLocalAdpt;
   /** Scaling factor multiplied by 16. In adaptive jitter buffer mode the
      target mean play out delay is equal to the estimated jitter times the
      scaling factor.
      The default value for the scaling factor is about 1,4 (nScaling=22).
      The scaling factor may be increased to reduce the number of discarded
      packets because of jitter. An increase of the scaling factor
      will eventually lead to an increased play out delay.
      Supported range 1 to 16 (nScaling=16 up to 256). */
   char     nScaling;
   /** Initial size of the jitter buffer in timestamps of 125 us in case of
      an adaptive jitter buffer. */
   unsigned short   nInitialSize;
   /** Minimum size of the jitter buffer in timestamps of 125 us in case of an
      adaptive jitter buffer. */
   unsigned short   nMinSize;
   /** Maximum size of the jitter buffer in timestamps of 125 us in case of an
   adaptive jitter buffer. */
   unsigned short   nMaxSize;
} IFX_TAPI_JB_CFG_t;

/** Structure for Jitter Buffer statistics used by
 \ref IFX_TAPI_JB_STATISTICS_GET ioctl.*/
typedef struct
{
   /** Jitter buffer type:
      - 1: fixed
      - 2: adaptive */
   unsigned char    nType;
   /** Incoming time high word total time in timestamp units for all packets since
   the start of the connection which could be played out correctly.
   Not supported anymore.  */
   unsigned long   nInTime;
   /**  Comfort noise generation. Not supported anymore. */
   unsigned long   nCNG;
   /**  Bad frame interpolation. Not supported anymore. */
   unsigned long   nBFI;
   /** Current jitter buffer size. */
   unsigned short nBufSize;
   /** Maximum estimated jitter buffer size. */
   unsigned short nMaxBufSize;
   /** Minimum estimated jitter buffer size. */
   unsigned short nMinBufSize;
   /** Maximum packet delay. Not supported anymore.  */
   unsigned short   nMaxDelay;
   /**  Minimum packet delay. Not supported anymore. */
   unsigned short   nMinDelay;
   /** Network jitter value. Not supported anymore. */
   unsigned short   nNwJitter;
   /**  Playout delay. */
   unsigned short   nPODelay;
   /**  Maximum playout delay. */
   unsigned short   nMaxPODelay;
   /**  Minimum playout delay. */
   unsigned short   nMinPODelay;
   /**  Received packet number.  */
   unsigned long   nPackets;
   /** Lost packets number. Not supported anymore. */
   unsigned short   nLost;
   /**  Invalid packet number. */
   unsigned short   nInvalid;
   /** Duplication packet number. Not supported anymore. */
   unsigned short   nDuplicate;
   /**  Late packets number. */
   unsigned short   nLate;
   /**  Early packets number. */
   unsigned short   nEarly;
   /**  Resynchronizations number. */
   unsigned short   nResync;
   /** Total number of injected samples since the beginning of the connection
       or since the last statistic reset due to jitter buffer underflows.*/
   unsigned long nIsUnderflow;
   /** Total number of injected samples since the beginning of the connection or
       since the last statistic reset in case of normal jitter buffer operation,
       which means when there is not a jitter buffer underflow. */
   unsigned long nIsNoUnderflow;
   /** Total number of injected samples since the beginning of the connection or
       since the last statistic reset in case of jitter buffer increments.*/
   unsigned long nIsIncrement;
   /** Total number of skipped lost samples since the beginning of the
      connection or since the last statistic reset in case of jitter buffer
      decrements. */
   unsigned long nSkDecrement;
   /** Total number of dropped samples since the beginning of the connection
      or since the last statistic reset in case of jitter buffer decrements.*/
   unsigned long nDsDecrement;
   /** Total number of dropped samples since the beginning of the connection
      or since the last statistic reset in case of jitter buffer overflows.*/
   unsigned long nDsOverflow;
   /** Total number of comfort noise samples since the beginning of the
       connection or since the last statistic reset. */
   unsigned long nSid;
   /** Number of received bytes high part including event packets. */
   unsigned long  nRecBytesH;
   /** Number of received bytes low part including event packets. */
   unsigned long  nRecBytesL;
} IFX_TAPI_JB_STATISTICS_t;

/** Phone channel mapping structure used for \ref IFX_TAPI_MAP_DATA_ADD and
\ref IFX_TAPI_MAP_DATA_REMOVE. */
typedef struct
{
   /** Phone channel number to which this channel should be mapped.
   Phone channels numbers start from 0. */
   unsigned char                              nDstCh;
   /** Type of destination channel, defined in \ref IFX_TAPI_MAP_TYPE_t. */
   IFX_TAPI_MAP_TYPE_t                         nChType;
   /** Enables or disables the recording service or leaves it as it is.

   - 0: IFX_TAPI_MAP_DATA_UNCHANGED, Do not modify the status of the recorder
   - 1: IFX_TAPI_MAP_DATA_START, Recording is started, same as
    \ref IFX_TAPI_ENC_START
   - 2: IFX_TAPI_MAP_DATA_STOP, Recording is stopped,same as
    \ref IFX_TAPI_ENC_STOP */
   IFX_TAPI_MAP_DATA_START_STOP_t              nRecStart;
   /** Enables or disables the play service or leaves it as it is.

   - 0: IFX_TAPI_MAP_DATA_UNCHANGED, Do not modify the status of the recorder
   - 1: IFX_TAPI_MAP_DATA_START, Playing is started,same as
      \ref IFX_TAPI_DEC_START
   - 2: IFX_TAPI_MAP_DATA_STOP, Playing is stopped, same as
      \ref IFX_TAPI_DEC_STOP */
   IFX_TAPI_MAP_DATA_START_STOP_t              nPlayStart;
} IFX_TAPI_MAP_DATA_t;

/** Phone channel mapping structure used for \ref IFX_TAPI_MAP_PHONE_ADD and
   \ref IFX_TAPI_MAP_PHONE_REMOVE. */
typedef struct
{
   /** Phone channel number to which this channel should be mapped.
      Phone channel numbers start from 0. */
   unsigned char                               nPhoneCh;
   /** Type of destination channel.
   - 0: IFX_TAPI_MAP_TYPE_DEFAULT, Default selected (analog phone channel)
   - 1: IFX_TAPI_MAP_TYPE_CODER, not supported
   - 2: IFX_TAPI_MAP_TYPE_PCM, type is PCM
   - 3: IFX_TAPI_MAP_TYPE_PHONE, type is analog phone channel
   - 4: IFX_TAPI_MAP_TYPE_AUDIO, type is audio channel
   - 5: IFX_TAPI_MAP_TYPE_AUDIO_AUX, type is audio channel auxiliary input
   - 6: IFX_TAPI_MAP_TYPE_DECT, type is DECT */
   IFX_TAPI_MAP_TYPE_t           nChType;
} IFX_TAPI_MAP_PHONE_t;

/** PCM channel mapping structure used for \ref IFX_TAPI_MAP_PCM_ADD and
\ref IFX_TAPI_MAP_PCM_REMOVE. */
typedef struct
{
   /** Channel number to which this channel should be mapped.
   Channels numbers start from 0. */
   unsigned char                 nDstCh;
   /** Type of the destination channel.

   - 0: IFX_TAPI_MAP_TYPE_DEFAULT, Default selected (phone channel)
   - 1: IFX_TAPI_MAP_TYPE_CODER, not supported
   - 2: IFX_TAPI_MAP_TYPE_PCM, type is PCM
   - 3: IFX_TAPI_MAP_TYPE_PHONE, type is phone channel
   - 4: IFX_TAPI_MAP_TYPE_AUDIO, type is audio channel
   - 5: IFX_TAPI_MAP_TYPE_AUDIO_AUX, type is audio channel auxiliary input
   - 6: IFX_TAPI_MAP_TYPE_DECT, type is DECT */
   IFX_TAPI_MAP_TYPE_t           nChType;
} IFX_TAPI_MAP_PCM_t;

/** DECT channel mapping structure used for \ref IFX_TAPI_MAP_DECT_ADD and
\ref IFX_TAPI_MAP_DECT_REMOVE. */
typedef struct
{
   /** Channel number to which this channel should be mapped.
   Channels numbers start from 0. */
   unsigned char                 nDstCh;
   /** Type of the destination channel.
   - 0: IFX_TAPI_MAP_TYPE_DEFAULT, Default selected (phone channel)
   - 1: IFX_TAPI_MAP_TYPE_CODER, not supported
   - 2: IFX_TAPI_MAP_TYPE_PCM, type is PCM
   - 3: IFX_TAPI_MAP_TYPE_PHONE, type is phone channel
   - 4: IFX_TAPI_MAP_TYPE_AUDIO, type is audio channel
   - 5: IFX_TAPI_MAP_TYPE_AUDIO_AUX, type is audio channel auxiliary input
   - 6: IFX_TAPI_MAP_TYPE_DECT, type is DECT */
   IFX_TAPI_MAP_TYPE_t           nChType;
} IFX_TAPI_MAP_DECT_t;

/** Structure used as parameter for \ref IFX_TAPI_ENC_ROOM_NOISE_DETECT_START. */
typedef struct
{
   /** Detection level in minus dB. */
   IFX_uint32_t     nThreshold;
   /** Count of consecutive voice packets required for event. */
   IFX_uint8_t      nVoicePktCnt;
   /** Count of consecutive silence packets required for event. */
   IFX_uint8_t      nSilencePktCnt;
} IFX_TAPI_ENC_ROOM_NOISE_DETECT_t;

/*@}*/ /* TAPI_INTERFACE_CON */

/* ======================================================================== */
/* TAPI Miscellaneous Services, structures (Group TAPI_INTERFACE_MISC)      */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_MISC */
/*@{*/

/** Capability structure. */
typedef struct
{
   /** Description of the capability. */
   char desc[80];
   /** Defines the capability type, see \ref IFX_TAPI_CAP_TYPE_t.*/
   IFX_TAPI_CAP_TYPE_t captype;
   /** Defines if, what or how many are available. The definition of cap
       depends on the type, see captype. */
   int cap;
   /** The number of this capability. */
   int handle;
} IFX_TAPI_CAP_t;

#ifndef TAPI_DXY_DOC
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
/** Exception bits. */
typedef struct
{
   unsigned int dtmf_ready:1;
   unsigned int hookstate:1;
   unsigned int flash_hook:1;
   unsigned int pstn_ring:1;
   unsigned int callerid_ready:1;
   unsigned int cidrx_supdate:1;
   unsigned int ground_key:1;
   unsigned int pulse_digit_ready:1;
   unsigned int fax_cng:1;
   unsigned int fax_ced:1;
   unsigned int fax_dis:1;
   unsigned int fax_supdate:1;
   unsigned int fault:1;
   unsigned int ground_key_high:1;
   unsigned int ground_key_polarity:1;
   unsigned int otemp:1;
   /* to get with IFX_TAPI_CH_STATUS_t in signal */
   unsigned int signal:1;
   unsigned int device:1;
   unsigned int gr909result:1;
   unsigned int fax_cng_net:1;
   unsigned int fax_ced_net:1;
   /** RFC 2833 event playout detection */
   unsigned int eventDetect:1;
   /** initial ringing finished */
   unsigned int ring_finished:1;
   /** runtime errors during processing of a tapi command */
   unsigned int runtime_error:1;
   /** Event occurs when the line mode may be switched to
       IFX_TAPI_LINE_FEED_NORMAL_LOW to save power. */
   unsigned int feedLowBatt:1;
   unsigned int reserved2:7;
} IFX_TAPI_EXCEPTION_BITS_t;
#elif (__BYTE_ORDER == __BIG_ENDIAN)
/** Exception bits */
typedef struct
{
   unsigned int reserved2:7;
   /** Event occurs when the line mode may be switched to
       IFX_TAPI_LINE_FEED_NORMAL_LOW to save power */
   unsigned int feedLowBatt:1;
   /** runtime errors during processing of a tapi command */
   unsigned int runtime_error:1;
   /** initial ringing finished */
   unsigned int ring_finished:1;
   /** RFC 2833 event playout detection */
   unsigned int eventDetect:1;
   /** Reserved */
   unsigned int fax_ced_net:1;
   /** Reserved */
   unsigned int fax_cng_net:1;
   /** GR909 result is available and can be queried with the appropriate
       interface */
   unsigned int gr909result:1;
   /** Low level device exception. The information can be queried from
       the underlying driver */
   unsigned int device:1;
   /** Signal detected. The information is retrieved with
       \ref IFX_TAPI_CH_STATUS_GET */
   unsigned int signal:1;
   /** Overtemperature detected */
   unsigned int otemp:1;
   /** Ground key polarity detected */
   unsigned int ground_key_polarity:1;
   /** Ground key high detected */
   unsigned int ground_key_high:1;
   /** Fault condition detected, query information via
       \ref IFX_TAPI_CH_STATUS_GET */
   unsigned int fault:1;
   /** FAX status update, query information via \ref IFX_TAPI_CH_STATUS_GET */
   unsigned int fax_supdate:1;
   /** FAX DIS received (no longer supported) */
   unsigned int fax_dis:1;
   /** FAX CED received (no longer supported) */
   unsigned int fax_ced:1;
   /** FAX CNG received (no longer supported) */
   unsigned int fax_cng:1;
   /** Pulse dial digit is ready */
   unsigned int pulse_digit_ready:1;
   /** Ground key detected */
   unsigned int ground_key:1;
   /** Caller id receiver event, query information
   with IFX_TAPI_CID_RX_STATUS_GET */
   unsigned int cidrx_supdate:1;
   /** Caller id data is ready (no longer supported) */
   unsigned int callerid_ready:1;
   /** Reserved */
   unsigned int pstn_ring:1;
   /** Flash hook detected */
   unsigned int flash_hook:1;
   /** Hook state changed */
   unsigned int hookstate:1;
   /** DTMF digit is ready */
   unsigned int dtmf_ready:1;
} IFX_TAPI_EXCEPTION_BITS_t;
#else
#error Please define endian mode
#endif
#endif /* TAPI_DXY_DOC */

#ifndef TAPI_DXY_DOC
/** Contains TAPI exception information */
typedef union
{
   /** Specifies the exception. */
   IFX_TAPI_EXCEPTION_BITS_t Bits;
   /** Contains the complete status. */
   unsigned long             Status;
} IFX_TAPI_EXCEPTION_t;

/** Structure used for IFX_TAPI_CH_STATUS_GET to query the phone status of one
    or more channels. */
typedef struct
{
   /** Size of the list. If 0 only the status of this channel is queried. If
       channels not equal to 0 all channels starting with 0 till the value
       of channels are retrieved.
       As the return value it defines the channel number */
   unsigned char channels;
   /** Lists the hook status events:

   - 0x01: IFX_TAPI_LINE_HOOK_STATUS_HOOK, Hook detected
   - 0x02: IFX_TAPI_LINE_HOOK_STATUS_FLASH, Hook flash detected
   - 0x04: IFX_TAPI_LINE_HOOK_STATUS_OFFHOOK, Detected hook event is an
           off hook.
           May be set additionally to IFX_TAPI_LINE_HOOK_STATUS_HOOK */
   unsigned char hook;
   /** Lists the dial status events. */
   unsigned char dialing;
   /** Contains the digit in case of an dial event. */
   unsigned char digit;
   /** Signals the line status and fault conditions:

      - 0x1: IFX_TAPI_LINE_STATUS_RINGING, PSTN line is ringing
      - 0x2: IFX_TAPI_LINE_STATUS_RINGFINISHED, ringing finished on PSTN line
      - 0x4: IFX_TAPI_LINE_STATUS_FAX, FAX status change
      - 0x10: IFX_TAPI_LINE_STATUS_GNDKEY, Ground key detected
      - 0x20: IFX_TAPI_LINE_STATUS_GNDKEYHIGH, Ground key high detected
      - 0x40: IFX_TAPI_LINE_STATUS_OTEMP, Overtemperature detected
      - 0x80: IFX_TAPI_LINE_STATUS_GNDKEYPOL, Ground key polarity detected
      - 0x100: IFX_TAPI_LINE_STATUS_GR909RES,
      - 0x200: IFX_TAPI_LINE_STATUS_CIDRX, Cid Receiver event occured
      - 0x400: IFX_TAPI_LINE_STATUS_FEEDLOWBATT, Event occurs when the line mode
        may be switched to IFX_TAPI_LINE_FEED_NORMAL_LOW to save power */
   unsigned int line;
   /** Signals the detection of events.
      The signals/events detected are reported in this and the signal_ext
      field. The signals here are represented according to the definition of
      \ref IFX_TAPI_SIG_t. The following description is taken from the
      definition of \ref IFX_TAPI_SIG_t. Please look there for further
      information.

      - 0x0: IFX_TAPI_SIG_NONE,     no signal detected
      - 0x1: IFX_TAPI_SIG_DISRX, V.21 Preamble Fax Tone, Digital
             Identification Signal (DIS), receive path
      - 0x2: IFX_TAPI_SIG_DISTX, V.21 Preamble Fax Tone,
             Digital Identification Signal (DIS), transmit path
      - 0x4: IFX_TAPI_SIG_DIS, V.21 Preamble Fax Tone in all path,
         Digital Identification Signal (DIS)
      - 0x8: IFX_TAPI_SIG_CEDRX, V.25 2100 Hz (CED) Modem/Fax Tone,
             receive path
      - 0x10: IFX_TAPI_SIG_CEDTX, V.25 2100 Hz (CED) Modem/Fax Tone,
              transmit path
      - 0x20: IFX_TAPI_SIG_CED, V.25 2100 Hz (CED) Modem/Fax Tone in
              all paths
      - 0x40: IFX_TAPI_SIG_CNGFAXRX, CNG Fax Calling Tone (1100 Hz)
              receive path
      - 0x80: IFX_TAPI_SIG_CNGFAXTX, CNG Fax Calling Tone (1100 Hz)
              transmit path
      - 0x100: IFX_TAPI_SIG_CNGFAX, CNG Fax Calling Tone (1100 Hz) in
               all paths
      - 0x200: IFX_TAPI_SIG_CNGMODRX, CNG Modem Calling Tone (1300 Hz)
         receive path
      - 0x400: IFX_TAPI_SIG_CNGMODTX, CNG Modem Calling Tone (1300 Hz)
         transmit path
      - 0x800: IFX_TAPI_SIG_CNGMOD, CNG Modem Calling Tone (1300 Hz) in all
               paths
      - 0x1000: IFX_TAPI_SIG_PHASEREVRX, Phase reversal detection
                receive path
      - 0x2000: IFX_TAPI_SIG_PHASEREVTX, Phase reversal detection
                transmit path
      - 0x4000: IFX_TAPI_SIG_PHASEREV, Phase reversal detection in all paths
      - 0x8000: IFX_TAPI_SIG_AMRX, Amplitude modulation receive path
      - 0x10000: IFX_TAPI_SIG_AMTX, Amplitude modulation transmit path
      - 0x20000: IFX_TAPI_SIG_AM, Amplitude modulation.
      - 0x40000: IFX_TAPI_SIG_TONEHOLDING_ENDRX, Modem tone holding signal
         stopped receive path
      - 0x80000: IFX_TAPI_SIG_TONEHOLDING_ENDTX, Modem tone holding signal
         stopped transmit path
      - 0x100000: IFX_TAPI_SIG_TONEHOLDING_END, Modem tone holding signal
         stopped all paths
      - 0x200000: IFX_TAPI_SIG_CEDENDRX, End of signal CED detection
                  receive path
      - 0x400000: IFX_TAPI_SIG_CEDENDTX, End of signal CED detection
                  transmit path
      - 0x800000: IFX_TAPI_SIG_CEDEND, End of signal CED detection.
      - 0x1000000: IFX_TAPI_SIG_CPTD, Call progress tone detected.
      - 0x8000000: IFX_TAPI_SIG_CIDENDTX, Caller ID transmission finished.
      - 0x10000000: IFX_TAPI_SIG_DTMFTX Enables DTMF reception on locally connected analog line
      - 0x20000000: IFX_TAPI_SIG_DTMFRX, Enables DTMF reception on remote connected line  */
   unsigned long signal;

   /** Signals the detection of events.
      The signals/events detected are reported in this and the signal
      field. The signals here are represented according to the definition of
      \ref IFX_TAPI_SIG_EXT_t. The following description is taken from the
      definition of \ref IFX_TAPI_SIG_EXT_t. Please look there for further
      information.

      - 0x0: IFX_TAPI_SIG_EXT_NONE,     no signal detected
      - 0x1: IFX_TAPI_SIG_EXT_V21LRX,
             980 Hz single tone (V.21L mark sequence) receive path
      - 0x2: IFX_TAPI_SIG_EXT_V21LTX,
             980 Hz single tone (V.21L mark sequence) transmit path
      - 0x4: IFX_TAPI_SIG_EXT_V21L,
             980 Hz single tone (V.21L mark sequence) all paths
      - 0x8: IFX_TAPI_SIG_EXT_V18ARX,
             1400 Hz single tone (V.18A mark sequence) receive path
      - 0x10: IFX_TAPI_SIG_EXT_V18ATX,
             1400 Hz single tone (V.18A mark sequence) transmit path
      - 0x20: IFX_TAPI_SIG_EXT_V18A,
             1400 Hz single tone (V.18A mark sequence) all paths
      - 0x40: IFX_TAPI_SIG_EXT_V27RX,
             1800 Hz single tone (V.27, V.32 carrier) receive path
      - 0x80: IFX_TAPI_SIG_EXT_V27TX,
             1800 Hz single tone (V.27, V.32 carrier) transmit path
      - 0x100: IFX_TAPI_SIG_EXT_V27,
             1800 Hz single tone (V.27, V.32 carrier) all paths
      - 0x200: IFX_TAPI_SIG_EXT_BELLRX,
             2225 Hz single tone (Bell answering tone) receive path
      - 0x400: IFX_TAPI_SIG_EXT_BELLTX,
             2225 Hz single tone (Bell answering tone) transmit path
      - 0x800: IFX_TAPI_SIG_EXT_BELL,
             2225 Hz single tone (Bell answering tone) all paths
      - 0x1000: IFX_TAPI_SIG_EXT_V22RX,
             2250 Hz single tone (V.22 unscrambled binary ones) receive path
      - 0x2000: IFX_TAPI_SIG_EXT_V22TX,
             2250 Hz single tone (V.22 unscrambled binary ones) transmit path
      - 0x4000: IFX_TAPI_SIG_EXT_V22,
             2250 Hz single tone (V.22 unscrambled binary ones) all paths
      - 0x8000: IFX_TAPI_SIG_EXT_V22ORBELLRX, 2225 Hz or 2250 Hz single tone,
             not possible to distinguish
      - 0x10000: IFX_TAPI_SIG_EXT_V22ORBELLTX, 2225 Hz or 2250 Hz single tone,
             not possible to distinguish
      - 0x20000: IFX_TAPI_SIG_EXT_V22ORBELL, 2225 Hz or 2250 Hz single tone,
             not possible to distinguish all paths
      - 0x40000: IFX_TAPI_SIG_EXT_V32ACRX,
             600 Hz + 300 Hz dual tone (V.32 AC) receive path
      - 0x80000: IFX_TAPI_SIG_EXT_V32ACTX,
             600 Hz + 300 Hz dual tone (V.32 AC) transmit path
      - 0x100000: IFX_TAPI_SIG_EXT_V32AC,
             600 Hz + 300 Hz dual tone (V.32 AC) all paths   */
   unsigned long signal_ext;
   /** device specific status information */
   unsigned long device;
   /** RFC 2833 event playout information. This field contains the last event,
       that was received from network side.

      - 0:  IFX_TAPI_PKT_EV_NUM_DTMF_0,
            RFC2833 Event number for DTMF tone #0
      - 1:  IFX_TAPI_PKT_EV_NUM_DTMF_1,
            RFC2833 Event number for DTMF tone #1
      - 2:  IFX_TAPI_PKT_EV_NUM_DTMF_2,
            RFC2833 Event number for DTMF tone #2
      - 3:  IFX_TAPI_PKT_EV_NUM_DTMF_3,
            RFC2833 Event number for DTMF tone #3
      - 4:  IFX_TAPI_PKT_EV_NUM_DTMF_4,
            RFC2833 Event number for DTMF tone #4
      - 5:  IFX_TAPI_PKT_EV_NUM_DTMF_5,
            RFC2833 Event number for DTMF tone #5
      - 6:  IFX_TAPI_PKT_EV_NUM_DTMF_6,
            RFC2833 Event number for DTMF tone #6
      - 7:  IFX_TAPI_PKT_EV_NUM_DTMF_7,
            RFC2833 Event number for DTMF tone #7
      - 8:  IFX_TAPI_PKT_EV_NUM_DTMF_8,
            RFC2833 Event number for DTMF tone #8
      - 9:  IFX_TAPI_PKT_EV_NUM_DTMF_9,
            RFC2833 Event number for DTMF tone #9
      - 10: IFX_TAPI_PKT_EV_NUM_DTMF_STAR, RFC2833 Event number for DTMF
            tone STAR
      - 11: IFX_TAPI_PKT_EV_NUM_DTMF_HASH, RFC2833 Event number for DTMF
            tone HASH
      - 32: IFX_TAPI_PKT_EV_NUM_ANS, RFC2833 Event number for ANS tone
      - 33: IFX_TAPI_PKT_EV_NUM_NANS, RFC2833 Event number for /ANS tone
      - 34: IFX_TAPI_PKT_EV_NUM_ANSAM, RFC2833 Event number for ANSam tone
      - 35: IFX_TAPI_PKT_EV_NUM_NANSAM, RFC2833 Event number for /ANSam tone
      - 36: IFX_TAPI_PKT_EV_NUM_CNG, RFC2833 Event number for CNG tone
      - 54: IFX_TAPI_PKT_EV_NUM_DIS, RFC2833 Event number for DIS tone */
   unsigned long event;
   /** Set of runtime errors which happens during the processing of a
       tapi command.

       - 0x0: IFX_TAPI_RT_ERROR_NONE, No runtime error
       - 0x1: IFX_TAPI_RT_ERROR_RINGCADENCE_CIDTX, Ring cadence cofiguration
              error for CID transmission
       - 0x2: IFX_TAPI_RT_ERROR_CIDTX_NOACK, No acknowledge during CID sequence.
       - 0x4: IFX_TAPI_RT_ERROR_CIDTX_NOACK2, No 2nd acknowledge during NTT CID
              onhook tx sequence. This indicates a missing "incoming successful
              signal".
   */
   unsigned long error;
} IFX_TAPI_CH_STATUS_t;
#endif /* TAPI_DXY_DOC */

/** Structure used for the TAPI version support check. */
typedef struct
{
   /** Major version number supported. */
   unsigned char majorNumber;
   /** Minor version number supported. */
   unsigned char minorNumber;
} IFX_TAPI_VERSION_t;

/** Defines the maximum number of stack entries. */
#define IFX_TAPI_MAX_ERROR_ENTRIES 4
#define IFX_TAPI_MAX_FILENAME 20
#define IFX_TAPI_MAX_ERRMSG 16
/** Contains one line of an error source including the error code, the
source code line and the file name (maximum 32 characters) */
typedef struct
{
   /** High level error code, which is set at the detection of the error in the
   high level TAPI driver part.
   The code may change in the flow of the error handling in the upper call stack. */
   IFX_uint16_t nHlCode;
   /** Low level error code, which is set at the detection of the error in the
   low level driver part. This code is device driver specific.
   The code may change in the flow of the error handling in the upper call stack. */
   IFX_uint16_t nLlCode;
   /** Source code line number */
   IFX_uint32_t nLine;
   /** Source code file name */
   IFX_char_t   sFile[IFX_TAPI_MAX_FILENAME];
   /** Any additional information depending on the error, like the last message
       sent to the device, state machine status, etc. */
   IFX_uint32_t msg  [IFX_TAPI_MAX_ERRMSG];
}IFX_TAPI_ErrorLine_t;

/** Error information with the origin source. It contains maximum 10 stack entries.
The item with index 0 is the first error detection. */
typedef struct
{
   /** Error code, which is set at the highest level where the
   error was detected.*/
   IFX_uint32_t nCode;
   /** The channel which causes this error, if any. */
   IFX_uint8_t  nCh;
   /** Error stack information. */
   IFX_TAPI_ErrorLine_t stack[IFX_TAPI_MAX_ERROR_ENTRIES];
   /** Number of stack entries. */
   IFX_uint8_t nCnt;
}IFX_TAPI_Error_t;

/*@}*/ /* TAPI_INTERFACE_MISC */

/* ===================================================================== */
/* TAPI Power Ringing Services, structures (Group TAPI_INTERFACE_RINGING)      */
/* ===================================================================== */
/** \addtogroup TAPI_INTERFACE_RINGING */
/*@{*/

/** Structure for ring cadence used in \ref IFX_TAPI_RING_CADENCE_HR_SET.  */
typedef struct
{
   /** Pointer to data bytes which contain the encoded cadence sequence.
   One bit represents ring cadence voltage for 50 ms. A maximum of
   40 bytes (320 bits) are allowed. */
   char      data[IFX_TAPI_RING_CADENCE_MAX_BYTES];
   /** Number of data bits of cadence sequence. A maximum number of
   320 data bits is possible which corresponds to a maximum cadence
   duration of 16 seconds. */
   int       nr;
   /** Obsolete field, please do not use!*/
   char      initial [IFX_TAPI_RING_CADENCE_MAX_BYTES];
   /** Obsolete field, please do not use!*/
   int       initialNr;
} IFX_TAPI_RING_CADENCE_t;

/** Ringing configuration structure used for \ref IFX_TAPI_RING_CFG_SET ioctl. */
typedef struct
{
   /** Configures the ringing mode values defined in
   \ref IFX_TAPI_RING_CFG_MODE_t .*/
   unsigned char       nMode;
   /** Configures the ringing submode values defined in
    \ref IFX_TAPI_RING_CFG_SUBMODE_t. */
   unsigned char       nSubmode;
} IFX_TAPI_RING_CFG_t;

/*@}*/ /* TAPI_INTERFACE_RINGING */

/* ============================================================ */
/* TAPI PCM Services, structures (Group TAPI_INTERFACE_PCM)     */
/* ============================================================ */
/** \addtogroup TAPI_INTERFACE_PCM */
/*@{*/

/** DCL frequency for the PCM interface.*/
typedef enum
{  /** 512 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_512         = 0,
   /** 1024 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_1024        = 1,
   /** 1536 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_1536        = 2,
   /** 2048 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_2048        = 3,
   /** 4096 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_4096        = 4,
   /** 8192 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_8192        = 5,
   /** 16384 kHz.*/
   IFX_TAPI_PCM_IF_DCLFREQ_16384       = 6
} IFX_TAPI_PCM_IF_DCLFREQ_t;

/** Drive mode for bit 0, in single clocking mode.*/
typedef enum
{
   /** Bit 0 is driven for the entire clock period.*/
   IFX_TAPI_PCM_IF_DRIVE_ENTIRE        = 0,
   /** Bit 0 is driven for the first half of the clock period.*/
   IFX_TAPI_PCM_IF_DRIVE_HALF          = 1
} IFX_TAPI_PCM_IF_DRIVE_t;

 /** PCM interface mode (master/slave).*/
typedef enum
{
   /** Reserved.*/
   IFX_TAPI_PCM_IF_MODE_SLAVE_AUTOFREQ = 0,
   /** Slave mode. The DCL frequency is explicitly programmed.*/
   IFX_TAPI_PCM_IF_MODE_SLAVE          = 1,
   /** Master mode. The DCL frequency is explicitly programmed.*/
   IFX_TAPI_PCM_IF_MODE_MASTER         = 2
} IFX_TAPI_PCM_IF_MODE_t;

/** PCM interface mode transmit/receive offset.*/
typedef enum
{
   /** No offset.*/
   IFX_TAPI_PCM_IF_OFFSET_NONE         = 0,
   /** Offset: one data period is added.*/
   IFX_TAPI_PCM_IF_OFFSET_1            = 1,
   /** Offset: two data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_2            = 2,
   /** Offset: three data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_3            = 3,
   /** Offset: four data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_4            = 4,
   /** Offset: five data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_5            = 5,
   /** Offset: six data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_6            = 6,
   /** Offset: seven data periods are added.*/
   IFX_TAPI_PCM_IF_OFFSET_7            = 7
} IFX_TAPI_PCM_IF_OFFSET_t;

 /** Slope for the PCM interface transmit/receive.*/
typedef enum
{
   /** Rising edge.*/
   IFX_TAPI_PCM_IF_SLOPE_RISE          = 0,
   /** Falling edge.*/
   IFX_TAPI_PCM_IF_SLOPE_FALL          = 1
} IFX_TAPI_PCM_IF_SLOPE_t;

/** Structure for PCM interface configuration.

   \remarks
   Attention: Not all Infineon products support all features that can be
   configured using this structure (for example master mode or slave mode without
   automatic clock detection). Please refer to the product system release note
   to learn about the supported features.
*/
typedef struct
{
   /** PCM interface mode (master or slave mode). */
   IFX_TAPI_PCM_IF_MODE_t        nOpMode;
   /** DCL frequency to be used in master and/or slave mode. */
   IFX_TAPI_PCM_IF_DCLFREQ_t     nDCLFreq;
   /** Activation/deactivation of the double clock mode.
   - IFX_DISABLE: single clocking is used.
   - IFX_ENABLE: double clocking is used. */
   IFX_operation_t               nDoubleClk;
   /** Slope to be considered for the PCM transmit direction.*/
   IFX_TAPI_PCM_IF_SLOPE_t       nSlopeTX;
   /** Slope to be considered for the PCM receive direction. */
   IFX_TAPI_PCM_IF_SLOPE_t       nSlopeRX;
   /** Transmit bit offset.*/
   IFX_TAPI_PCM_IF_OFFSET_t      nOffsetTX;
   /** Receive bit offset.*/
   IFX_TAPI_PCM_IF_OFFSET_t      nOffsetRX;
   /** Drive mode for bit 0.*/
   IFX_TAPI_PCM_IF_DRIVE_t       nDrive;
   /** Enable/disable shift access edge. Shift the access edges by one clock
      cycle.
   - IFX_DISABLE: no shift takes place.
   - IFX_ENABLE: shift takes place.
   Note: This setting is defined only in double clock mode.*/
   IFX_operation_t               nShift;
   /** Reserved.PCM chip specific settings. please set this field to 0x00 if
    not advised otherwise by IFX support team. */
   IFX_uint8_t                   nMCTS;
} IFX_TAPI_PCM_IF_CFG_t;

/** Structure for PCM channel configuration.  */
typedef struct
{
   /** PCM timeslot for the receive direction. */
   unsigned long             nTimeslotRX;
   /** PCM timeslot for the transmit direction. */
   unsigned long             nTimeslotTX;
   /** Defines the PCM highway number which is connected to the channel. */
   unsigned long             nHighway;
   /** Defines the PCM interface coding, values defined in
    \ref IFX_TAPI_PCM_RES_t. */
   unsigned long             nResolution;
} IFX_TAPI_PCM_CFG_t;

/*@}*/ /* TAPI_INTERFACE_PCM */

/* ======================================================================== */
/* TAPI Fax T.38 Services, structures (Group TAPI_INTERFACE_FAX)            */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_FAX */
/*@{*/

/** Structure to setup the modulator for T.38 fax and used for
    \ref IFX_TAPI_T38_MOD_START. */
typedef struct
{
   /** Selects the standard used for Fax T.38 \ref IFX_TAPI_T38_STD_t
   - 0x00: Silence
   - 0x01: V.21
   - 0x02: V.27/2400
   - 0x03: V.27/4800
   - 0x04: V.29/7200
   - 0x05: V.29/9600
   - 0x06: V.17/7200
   - 0x07: V.17/9600
   - 0x08: V.17/12000
   - 0x09: V.17/14400
   - 0x0A: CNG
   - 0x0B: CED
   */
   unsigned char    nStandard;
   /** Signal duration in ms. Used for the tonals signals (CED, CNG)and silence
    only. 1 < nSigLen < 4000 [ms]*/
   unsigned short   nSigLen;
   /** Sets the transmit gain for the downstream direction. */
   unsigned short   nGainTx;
   /** Desired output signal power in -dBm. */
   unsigned char    nDbm;
   /** TEP Generation flag, used by V.27, V.29 and V.17 only,ignored in all
    other cases.

   - 0: no TEP generation
   - 1: TEP generation */
   unsigned char    nTEP;
   /** Training sequence flag, used by V.17 only, ignored in all other cases.

   - 0: Short training sequence
   - 1: Long training sequence */
   unsigned char    nTraining;
   /** Level required before the modulation starts.*/
   unsigned short   nMobsm;
   /**Level required before the modulation requests more data.*/
   unsigned short   nMobrd;
} IFX_TAPI_T38_MOD_DATA_t;

/** Structure to setup the demodulator for T.38 fax and used for
    \ref IFX_TAPI_T38_DEMOD_START. */
typedef struct
{
   /** Selects the standard used for Fax T.38 using \ref IFX_TAPI_T38_STD_t

   - 0x00: Silence
   - 0x01: V.21
   - 0x02: V.27/2400
   - 0x03: V.27/4800
   - 0x04: V.29/7200
   - 0x05: V.29/9600
   - 0x06: V.17/7200
   - 0x07: V.17/9600
   - 0x08: V.17/12000 */
   unsigned char    nStandard1;
   /** Selects the alternative standard used for Fax T.38 uisng
       \ref IFX_TAPI_T38_STD_t

   - 0x00: Silence
   - 0x01: V.21
   - 0x02: V.27/2400
   - 0x03: V.27/4800
   - 0x04: V.29/7200
   - 0x05: V.29/9600
   - 0x06: V.17/7200
   - 0x07: V.17/9600
   - 0x08: V.17/12000
   - 0x09: V.17/14400
   - 0xFF: Use only standard 1. */
   unsigned char    nStandard2;
   /** Signal duration in ms. Used for the tonals signals (CED, CNG)
       and silence only.
       1 < nSigLen < 4000 [ms]*/
   unsigned short   nSigLen;
   /** Sets the receive gain for the upstream direction. */
   unsigned short   nGainRx;
   /** Equalizer configuration flag

   - 0: Reset the equalizer
   - 1: Reuse the equalizer coefficients */
   unsigned char    nEqualizer;
   /** Training sequence flag, used by V.17 only, ignored in all other cases.

   - 0: Short training sequence
   - 1: Long training sequence */
   unsigned char    nTraining;
   /** Level required before the demodulator sends data.*/
   unsigned short   nDmbsd;
} IFX_TAPI_T38_DEMOD_DATA_t;

/** Structure to read the T.38 fax status and used for
    \ref IFX_TAPI_T38_STATUS_GET. */
typedef struct
{
   /** T.38 fax status, refer to \ref IFX_TAPI_FAX_T38_STATUS_t
   - 0: IFX_TAPI_FAX_T38_DP_OFF Data pump is not active.
   - 1: IFX_TAPI_FAX_T38_DP_ON Data pump is active.
   - 2: IFX_TAPI_FAX_T38_TX_ON Transmission is active.
   - 3: IFX_TAPI_FAX_T38_TX_OFF Transmission is finished.
   */
   unsigned char nStatus;
   /** T.38 fax error, refer to \ref IFX_TAPI_FAX_T38_ERROR_t

   - 0x00: No error occurred
   - 0x01: FAX error occurred, the FAX data pump should be deactivated
   - 0x02: MIPS overload
   - 0x03: Error while reading data
   - 0x04: Error while writing data
   - 0x05: Error while setting up the modulator or demodulator
   */
   unsigned char nError;
} IFX_TAPI_T38_STATUS_t;

/*@}*/ /* TAPI_INTERFACE_FAX */

/* ======================================================================== */
/* TAPI Audio Services, structures (Group TAPI_INTERFACE_AUDIO)             */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_AUDIO */
/*@{*/

/** Audio modes for the audio channel. */
enum IFX_TAPI_AUDIO_MODE_t
{
  /** Audio channel is disabled. */
   IFX_TAPI_AUDIO_MODE_DISABLED = 0,
   /** Handset mode. */
   IFX_TAPI_AUDIO_MODE_HANDSET = 1,
   /** Handset mode with open listening. */
   IFX_TAPI_AUDIO_MODE_HANDSET_OPENL,
   /** Headset mode. */
   IFX_TAPI_AUDIO_MODE_HEADSET,
   /** Headset mode with open listening. */
   IFX_TAPI_AUDIO_MODE_HEADSET_OPENL,
   /** Hands-free mode. */
   IFX_TAPI_AUDIO_MODE_HANDSFREE,
   /** Handset mode, wideband. */
   IFX_TAPI_AUDIO_MODE_HANDSET_WDB,
   /** Headset mode, wideband. */
   IFX_TAPI_AUDIO_MODE_HEADSET_WDB,
   /** Handsfree mode, wideband. */
   IFX_TAPI_AUDIO_MODE_HANDSFREE_WDB,
   /** Handset mode with open listening, wideband. */
   IFX_TAPI_AUDIO_MODE_HANDSET_WDB_OPENL,
   /** Headset mode with open listening, wideband. */
   IFX_TAPI_AUDIO_MODE_HEADSET_WDB_OPENL
};

/** Audio modes.
   Room type setting for the Hands-free Acoustic Echo Canceller. */
enum IFX_TAPI_AUDIO_ROOM_TYPE_t
{
   /** Muffled room, low echo level. */
   IFX_TAPI_AUDIO_ROOM_TYPE_MUFFLED = 1,
   /** Medium echo level. */
   IFX_TAPI_AUDIO_ROOM_TYPE_MEDIUM,
   /** Echoic room, high echo level. */
   IFX_TAPI_AUDIO_ROOM_TYPE_ECHOIC
};

/** Selector for Auxiliary channel based functionalities In-Call Announcement
 / Off Hook Voice Announcement. */
typedef enum
{
   /** Disable In-call annoucement/OHVA. */
   IFX_TAPI_AUDIO_ICA_DISABLED = 0,

   /** In-call annoucement, using the audio aux port as output. */
   IFX_TAPI_AUDIO_ICA_OUT = 1,

   /** In-call Annoucement, using the audio aux port as input/output(OHVA). */
   IFX_TAPI_AUDIO_ICA_INOUT = 2
} IFX_TAPI_AUDIO_ICA_t;


/** AFE Microphone Inputs. */
typedef enum
{
   /** Use AFE microphone input 1. Not possible for Handsfree.
   Note: INCA-IP2: pins MIP1/MIN1.
  */
   IFX_TAPI_AUDIO_AFE_PIN_MIC1 = 0,

   /** Use AFE microphone input 2.Not possible for Handsfree.
   Note: INCA-IP2: pins MIP2/MIN2.
 */
   IFX_TAPI_AUDIO_AFE_PIN_MIC2 = 1,

   /** Use AFE microphone input 3. Default for handsfree.
    Note: INCA-IP2: pins MIP3/MIN3. */
   IFX_TAPI_AUDIO_AFE_PIN_MIC3 = 2,

   /** Use AFE microphone input 4. Not possible for Handset/Headset.
    To be used for OHVA.
   Note: INCA-IP2: pins MIP4/MIN4.*/

   IFX_TAPI_AUDIO_AFE_PIN_MIC4 = 3
} IFX_TAPI_AUDIO_AFE_PIN_MIC_t;

/** AFE Outputs. */
typedef enum
{
   /** INCA-IP2: pins HOP1/HON1. */
   IFX_TAPI_AUDIO_AFE_PIN_OUT1 = 0,
   /** INCA-IP2: pins HOP2/HON2.*/
   IFX_TAPI_AUDIO_AFE_PIN_OUT2 = 1,
   /** INCA-IP2: pins LSP1/LSN1. */
   IFX_TAPI_AUDIO_AFE_PIN_OUT3 = 2,
   /** INCA-IP2: pins LSP2/LSN2.*/
   IFX_TAPI_AUDIO_AFE_PIN_OUT4 = 3
} IFX_TAPI_AUDIO_AFE_PIN_OUT_t;


/** AFE Input/Output Selectors for Handsfree, Hand- and Headset. */
typedef struct
{
   /** Handsfree Microphone. */
   IFX_TAPI_AUDIO_AFE_PIN_MIC_t nHFMic;
   /** Handsfree Output. */
   IFX_TAPI_AUDIO_AFE_PIN_OUT_t nHFOut;
   /** Handset Microphone. */
   IFX_TAPI_AUDIO_AFE_PIN_MIC_t nHNMic;
   /** Handset Output. */
   IFX_TAPI_AUDIO_AFE_PIN_OUT_t nHNOut;
   /** Headset Microphone. */
   IFX_TAPI_AUDIO_AFE_PIN_MIC_t nHDMic;
   /** Headset Output. */
   IFX_TAPI_AUDIO_AFE_PIN_OUT_t nHDOut;
} IFX_TAPI_AUDIO_AFE_CFG_SET_t;

/** Audio loop and audio Loop and diagnostics modes. */
typedef enum
{
   /** The test mode is disabled: the audio channel can be used as usual.*/
   IFX_TAPI_AUDIO_TEST_DISABLED = 0,
   /** The output and input of the codec will be directed to DIAG signals.*/
   IFX_TAPI_AUDIO_TEST_DIAGNOSTIC = 1,
   /** The Codec is disconnected and data is directed to loop signals. */
   IFX_TAPI_AUDIO_TEST_LOOP = 2
} IFX_TAPI_AUDIO_TEST_MODES_t;

/** Loop and diagnostics setup structure. */
typedef struct
{
   /** Port 0. */
   IFX_TAPI_AUDIO_TEST_MODES_t nTestPort0;
   /** Port 1. */
   IFX_TAPI_AUDIO_TEST_MODES_t nTestPort1;
} IFX_TAPI_AUDIO_TEST_MODE_t;




/*@}*/ /* TAPI_INTERFACE_AUDIO */

/* ======================================================================== */
/* TAPI Ext keypad  Services, structures (Group TAPI_INTERFACE_CON)  */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_CON */
/*@{*/

#define IFX_TAPI_STOP       IFX_TAPI_EV_GEN_STOP
#define IFX_TAPI_START      IFX_TAPI_EV_GEN_START
#define IFX_TAPI_ACTION_t   IFX_TAPI_PKT_EV_GEN_ACTION_t

#define IFX_TAPI_EVENT_EXT_DTMF_t       IFX_TAPI_PKT_EV_GENERATE_t
#define IFX_TAPI_EVENT_EXT_DTMF_CFG_t   IFX_TAPI_PKT_EV_GENERATE_CFG_t

/*@}*/ /* TAPI_INTERFACE_CON */

/* ======================================================================== */
/* TAPI Test Services, structures (Group TAPI_INTERFACE_TEST)               */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_TEST */
/*@{*/

/** Structure for switching loops for testing. */
typedef struct
{
   /** Switch an analog loop in the device. If switched on, signals that are
      played to the subscriber are looped back to the receiving side.
      - 0x0: Analog loop off
      - 0x1: Analog loop on */
   unsigned char bAnalog;
} IFX_TAPI_TEST_LOOP_t;

/*@}*/ /* TAPI_INTERFACE_TEST */

/* ======================================================================== */
/* TAPI Polling Services (Group TAPI_POLLING_SERVICE)               */
/* ======================================================================== */
/** \addtogroup TAPI_POLLING_SERVICE */
/*@{*/

/** Maximum size of TAPI packet. */
#define IFX_TAPI_POLL_PKT_SIZE      256
/** Defines the packet types supported by polling.*/
typedef enum
{
    /** Packetized voice.*/
    IFX_TAPI_POLL_PKT_TYPE_VOICE  = 0,
   /** Fax relay data pump packets used if the T.38 data pump functionality
    is integrated in the device.*/
    IFX_TAPI_POLL_PKT_TYPE_FRD    = 1,
  /** Available upstream only.*/
    IFX_TAPI_POLL_PKT_TYPE_FRS    = 2
} IFX_TAPI_POLL_PKT_TYPE_t;

/** Structure for polling packet handling.*/
typedef struct
{
   /** Packet header section. */
   unsigned dev                        : 8;  /** Channel identifier [0,1,...].*/
   unsigned ch                         : 8;  /** Device identifier [0,1,...]. */
   unsigned type                       : 4;  /** Packet length in bytes. */
   unsigned len                        : 12; /** Packet type identifier. */

   /** Packet payload section. */
   IFX_uint16_t data[IFX_TAPI_POLL_PKT_SIZE];
} IFX_TAPI_POLL_PKT_t;

/** TAPI driver global polling configuration structure. */
typedef struct
{
   /** Pointer to the buffer pool control structure. The pointer is used
       together with the buffer get (* getBuf) routine to identify the
       buffer pool used.
   */
   void *pBufPool;
   /** Pointer to function used to get an empty buffer from the buffer pool.
       The buffer is protected and can be exclusively used by the user. The
       buffer size is predefined at buffer pool initialization time. */
   void *(*get) (void *pBufPool);
   /** Pointer to function pointer used to return a used buffer back to the
       buffer pool. */
   int (*put) (void *pBuf);
} IFX_TAPI_POLL_CONFIG_t;

/** TAPI driver data buffer structure. */
typedef struct
{
   /** Pointer to the first element in an array of packet pointers. */
   void **ppPkts;
   /** Number of packet pointers in the array. */
   IFX_int32_t nPktsNum;
} IFX_TAPI_POLL_DATA_t;

#ifdef VXWORKS
 /** Polling upstream interface function.

   \param ppPktsUp  Pointer to the first element in an array of packet pointers.
   \param pPktsUpNum  Upon entry, Contains the number of available buffers in
    ppPkts. Upon return, contains the number of buffers
    actually read.

   \return  No return values.
 */
IFX_return_t TAPI_Poll_Up(IFX_void_t **ppPktsUp, IFX_int32_t *pPktsUpNum);
 /**  Polling downstream interface function.

   \param   ppPktsDown  Pointer to the first element in an array of packet
    pointers.
   \param   pPktsDownNum Upon entry, contains the number of packets to be written.
    Upon return, contains the number of buffers actually written.

   \return  No return values.
 */
IFX_return_t TAPI_Poll_Down(IFX_void_t **ppPktsDown, IFX_int32_t *pPktsDownNum);
 /** Polling function to fetch the events occurred in all devices.

   \param  IFX_void_t  Parameter not required.

   \return  No return values.
 */
IFX_void_t TAPI_Poll_Events(IFX_void_t);
#endif

/*@}*/ /* TAPI_POLLING_SERVICE */

/* ======================================================================== */
/* TAPI FXO Services, structures (Group TAPI_INTERFACE_FXO)                 */
/* ======================================================================== */
/** \addtogroup TAPI_INTERFACE_FXO */
/*@{*/

/** Structure including the digits to be dialed used in
    \ref IFX_TAPI_FXO_DIAL_START.
*/
typedef struct
{
   /** The number of digits to be dialed. */
   IFX_uint8_t nDigits;
   /** The string of digits to be dialed.
    Note: Only 0-9 and A,B,C,D are supported. */
   IFX_char_t data [IFX_TAPI_FXO_DIAL_DIGITS];
} IFX_TAPI_FXO_DIAL_t;

/** Structure for FXO dialing configuration, used in
 \ref IFX_TAPI_FXO_DIAL_CFG_SET. */
typedef struct
{
   /** Time between two digits, in ms. Default 100 ms.*/
   IFX_uint16_t nInterDigitTime;
   /** Play time for each digit, in ms. Default 100 ms.*/
   IFX_uint16_t nDigitPlayTime;
} IFX_TAPI_FXO_DIAL_CFG_t;

/** Hook confinguration for FXO, used in \ref IFX_TAPI_FXO_FLASH_CFG_SET. */
typedef struct
{
   /** Duration of a flash-hook. Default 100 ms.*/
   IFX_uint32_t nFlashTime;
} IFX_TAPI_FXO_FLASH_CFG_t;

/** OSI confinguration for FXO, used in \ref IFX_TAPI_FXO_OSI_CFG_SET. */
typedef struct
{
   /** Duration of an OSI. Default 200 ms.*/
   IFX_uint32_t nOSIMax;
} IFX_TAPI_FXO_OSI_CFG_t;

/** Defines the possible hook status for fxo, used in \ref IFX_TAPI_FXO_HOOK_SET,
    \ref IFX_TAPI_FXO_HOOK_GET. */
typedef enum
{
   /** On-hook. */
   IFX_TAPI_FXO_HOOK_ONHOOK  = 0,
   /** Off-hook. */
   IFX_TAPI_FXO_HOOK_OFFHOOK = 1
} IFX_TAPI_FXO_HOOK_t;
/*@}*/ /* TAPI_INTERFACE_FXO */

/** Return code classes for error handling. */
typedef enum
{
   /** Specifies a generic status result or error. */
   TAPI_statusClassSuccess = 0x0000,
   /** Specifies a channel related error in addition to the other classes. */
   TAPI_statusClassCh = 0x1000,
   /** Specifies a warning or information, which does not harm the system
       if handled correctly. The upper layer handle this result as an error and
       maybe signaled to the application. */
   TAPI_statusClassWarn = 0x4000,
   /** Specifies a general error, which may lead to fail function of at least
       that channel or feature. */
   TAPI_statusClassErr = 0x6000,
   /** Specifies a critical error, device or driver maybe out of function. */
   TAPI_statusClassCritical = 0x8000
}TAPI_statusClass_t;

/** This macro checks if the return result is successful or not.
    If the return value is IFX_ERROR it returns IFX_FALSE. Otherwise the
    code is checked if any of the classes Err, Warn or Critical are set
    and returns also IFX_FALSE, if set, otherwise IFX_TRUE. */
#define TAPI_SUCCESS(code)                                              \
   (code == IFX_ERROR ? IFX_FALSE :                                     \
                        ((code & (TAPI_statusClassErr |                 \
                         TAPI_statusClassWarn |                         \
                         TAPI_statusClassCritical)) == 0 ?  \
        IFX_TRUE : IFX_FALSE))

/* ===================================================================== */
/* TAPI DECT Services, structures (Group TAPI_INTERFACE_DECT)            */
/* ===================================================================== */
/** \addtogroup TAPI_INTERFACE_DECT */
/*@{*/

/** Structure for DECT channel configuration by \ref IFX_TAPI_DECT_CFG_SET. */
typedef struct
{
   /** PCM time slot for the receive direction. */
   IFX_uint32_t   nEncDelay;
   /** PCM time slot for the transmit direction. */
   IFX_uint32_t   nDecDelay;
} IFX_TAPI_DECT_CFG_t;

/** Structure used as parameter by \ref IFX_TAPI_DECT_ENC_CFG_SET */
typedef struct
{
   /** Encoder type. */
   IFX_TAPI_DECT_ENC_TYPE_t nEncType;
   /** Frame length. */
   IFX_TAPI_DECT_ENC_LENGTH_t nFrameLen;
} IFX_TAPI_DECT_ENC_CFG_t;

/** Structure for returning DECT channel statistics data,
    Used by \ref IFX_TAPI_DECT_STATISTICS_GET. */
typedef struct
{
   /** Reset the counters after reading: 0 do not reset <>0 reset counters. */
   IFX_uint8_t nReset;
   /** Host to DECT handset packet count: The total number of upstream DECT
       data packets (host to DECT handset) transmitted since start of
       transmission (voice packets). */
   IFX_uint32_t nPktUp;
   /** FP to Host Packet Count: The total number of downstream DECT data
       packets (DECT handset to Host) received since starting transmission
       (voice packets). */
   IFX_uint32_t nPktDown;
   /** Number of SID packets received from DECT handset. */
   IFX_uint32_t nSid;
   /** Number of PLC packets received from DECT handset. */
   IFX_uint32_t nPlc;
   /** Number of DECT handset to host buffer overflows: Number of packets that
       have to be discarded due to overflow. */
   IFX_uint32_t nOverflows;
   /** Number of DECT handset to host buffer underflows: Number of decoder
       buffer underflows every 2.5 ms. The decoder runs on 2.5 ms packets. */
   IFX_uint32_t nUnderflows;
   /** Number of DECT handset to host invalid packets: Number of invalid
       packets that arrive downstream from handset. */
   IFX_uint32_t nInvalid;
} IFX_TAPI_DECT_STATISTICS_t;

/*@}*/ /* TAPI_INTERFACE_DECT */

/* ===================================================================== */
/* TAPI Event Services, structures (Group TAPI_INTERFACE_EVENT)          */
/* ===================================================================== */

/** \addtogroup TAPI_INTERFACE_EVENT */
/*@{*/

/* =============================== */
/* Macros                          */
/* =============================== */

#define IFX_TAPI_EVENT_TYPE_MASK                              0xFFFF0000
#define IFX_TAPI_EVENT_SUBTYPE_MASK                           0x0000FFFF
#define IFX_TAPI_EVENT_TYPE_FAULT_MASK                        0xF0000000

/* =============================== */
/* enum                            */
/* =============================== */

/** List of event types */
typedef enum
{
   /** Reserved. */
   IFX_TAPI_EVENT_TYPE_NONE                                 = 0x00000000,
   /** Event on GPIOs, Channel IOs. */
   IFX_TAPI_EVENT_TYPE_IO_GENERAL                           = 0x10000000,
   /** Reserved. External interrupt. */
   IFX_TAPI_EVENT_TYPE_IO_INTERRUPT                         = 0x11000000,
   /** Ringing, hook events. */
   IFX_TAPI_EVENT_TYPE_FXS                                  = 0x20000000,
   /** Ringing, polarity reversal.*/
   IFX_TAPI_EVENT_TYPE_FXO                                  = 0x21000000,
   /** Linetesting events. */
   IFX_TAPI_EVENT_TYPE_LT                                   = 0x29000000,
   /** Pulse Digit detected. */
   IFX_TAPI_EVENT_TYPE_PULSE                                = 0x30000000,
   /** DTMF Digit detected. */
   IFX_TAPI_EVENT_TYPE_DTMF                                 = 0x31000000,
   /** Caller ID events. */
   IFX_TAPI_EVENT_TYPE_CID                                  = 0x32000000,
   /** Tone generation event e.g. Tone generation ended. */
   IFX_TAPI_EVENT_TYPE_TONE_GEN                             = 0x33000000,
   /** Tone detection event, e.g. Call Progress Tones. */
   IFX_TAPI_EVENT_TYPE_TONE_DET                             = 0x34000000,
   /** Detection of Fax/Modem and V.18 signals. */
   IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL                      = 0x35000000,
   /** e.g. vocoder changed. */
   IFX_TAPI_EVENT_TYPE_COD                                  = 0x40000000,
   /** Reserved */
   IFX_TAPI_EVENT_TYPE_RTP                                  = 0x41000000,
   /** Reserved for AAL events */
   IFX_TAPI_EVENT_TYPE_AAL                                  = 0x42000000,
   /** RFC2833 Frame detected */
   IFX_TAPI_EVENT_TYPE_RFC2833                              = 0x43000000,
   /** KPI interface. */
   IFX_TAPI_EVENT_TYPE_KPI                                  = 0x44000000,
   /** T.38 events. */
   IFX_TAPI_EVENT_TYPE_T38                                  = 0x50000000,
   /** JB events??? maybe not required */
   IFX_TAPI_EVENT_TYPE_JB                                   = 0x60000000,
   /** e.g. FW Download finished, bad CRC, ... */
   IFX_TAPI_EVENT_TYPE_DOWNLOAD                             = 0x70000000,
#ifdef TAPI_EXT_KEYPAD
   /** For external events like Keypad incase of inca2 */
   IFX_TAPI_EVENT_TYPE_EXT                                  = 0x80000000,
#endif /*TAPI_EXT_KEYPAD*/
   /** Reserved for future use */
   IFX_TAPI_EVENT_TYPE_INFO                                 = 0xA0000000,
   /** Debug information, e.g. dump of some registers or memory areas */
   IFX_TAPI_EVENT_TYPE_DEBUG                                = 0xD0000000,
   /** events of the low level driver */
   IFX_TAPI_EVENT_TYPE_LL_DRIVER                            = 0xE0000000,
   /** Reserved */
   IFX_TAPI_EVENT_TYPE_FAULT_GENERAL                        = 0xF1000000,
   /** e.g. Overtemperature, Ground key detected */
   IFX_TAPI_EVENT_TYPE_FAULT_LINE                           = 0xF2000000,
   /** (Reserved) e.g. Watchdog, PLL, ... */
   IFX_TAPI_EVENT_TYPE_FAULT_HW                             = 0xF3000000,
   /** (Reserved)e.g. Mailbox error */
   IFX_TAPI_EVENT_TYPE_FAULT_FW                             = 0xF4000000,
   /** (Reserved) */
   IFX_TAPI_EVENT_TYPE_FAULT_SW                             = 0xF5000000
} IFX_TAPI_EVENT_TYPE_t;


/** List of event IDs */
typedef enum
{
   /* NONE (Reserved) */
   /** Reserved */
   IFX_TAPI_EVENT_NONE                 = IFX_TAPI_EVENT_TYPE_NONE | 0x0000,
   /* IO_GENERAL (Reserved) */
   /** Reserved */
   IFX_TAPI_EVENT_IO_GENERAL_NONE      = IFX_TAPI_EVENT_TYPE_IO_GENERAL | 0x0000,
   /* IO_INTERRUPT (Reserved) */
   /** Reserved */
   IFX_TAPI_EVENT_IO_INTERRUPT_NONE    = IFX_TAPI_EVENT_TYPE_IO_INTERRUPT | 0x0000,

   /* FXS */
   /** No event */
   IFX_TAPI_EVENT_FXS_NONE             = IFX_TAPI_EVENT_TYPE_FXS | 0x0000,
   /** FXS line is ringing. */
   IFX_TAPI_EVENT_FXS_RING             = IFX_TAPI_EVENT_TYPE_FXS | 0x0001,
   /** FXS end of a single ring burst. */
   IFX_TAPI_EVENT_FXS_RINGBURST_END    = IFX_TAPI_EVENT_TYPE_FXS | 0x0002,
   /** FXS end of ringing. */
   IFX_TAPI_EVENT_FXS_RINGING_END      = IFX_TAPI_EVENT_TYPE_FXS | 0x0003,
   /** Hook event: on-hook. */
   IFX_TAPI_EVENT_FXS_ONHOOK           = IFX_TAPI_EVENT_TYPE_FXS | 0x0004,
   /** Hook event: off-hook. */
   IFX_TAPI_EVENT_FXS_OFFHOOK          = IFX_TAPI_EVENT_TYPE_FXS | 0x0005,
   /** Hook event: flash hook. */
   IFX_TAPI_EVENT_FXS_FLASH            = IFX_TAPI_EVENT_TYPE_FXS | 0x0006,
   /** Hook event: on-hook detected by interrupt. */
   IFX_TAPI_EVENT_FXS_ONHOOK_INT       = IFX_TAPI_EVENT_TYPE_FXS | 0x0007,
   /** Hook event: off-hook detected by interrupt. */
   IFX_TAPI_EVENT_FXS_OFFHOOK_INT      = IFX_TAPI_EVENT_TYPE_FXS | 0x0008,

   /* FXO */
   /** No event. */
   IFX_TAPI_EVENT_FXO_NONE             = IFX_TAPI_EVENT_TYPE_FXO | 0x0000,
   /** Battery - line is feeded from FXO. */
   IFX_TAPI_EVENT_FXO_BAT_FEEDED       = IFX_TAPI_EVENT_TYPE_FXO | 0x0001,
   /** Battery - FXO line is not feeded. */
   IFX_TAPI_EVENT_FXO_BAT_DROPPED      = IFX_TAPI_EVENT_TYPE_FXO | 0x0002,
   /** FXO line polarity changed. */
   IFX_TAPI_EVENT_FXO_POLARITY         = IFX_TAPI_EVENT_TYPE_FXO | 0x0003,
   /** Line is ringing, indicates ring bursts. */
   IFX_TAPI_EVENT_FXO_RING_START       = IFX_TAPI_EVENT_TYPE_FXO | 0x0004,
   /** FXO line stopped ringing.*/
   IFX_TAPI_EVENT_FXO_RING_STOP        = IFX_TAPI_EVENT_TYPE_FXO | 0x0005,
   /** OSI signal (short drop of DC voltage, less than 300 ms),
       indicating the start of a CID transmission. */
   IFX_TAPI_EVENT_FXO_OSI              = IFX_TAPI_EVENT_TYPE_FXO | 0x0006,
   /** APOH (another phone off-hook). */
   IFX_TAPI_EVENT_FXO_APOH             = IFX_TAPI_EVENT_TYPE_FXO | 0x0007,
   /** NOPOH (no other phone off-hook). */
   IFX_TAPI_EVENT_FXO_NOPOH            = IFX_TAPI_EVENT_TYPE_FXO | 0x0008,

   /** GR-909 test results ready. */
   IFX_TAPI_EVENT_LT_GR909_RDY         = IFX_TAPI_EVENT_TYPE_LT  | 0x0001,

   /* PULSE */
   /** No event. */
   IFX_TAPI_EVENT_PULSE_NONE           = IFX_TAPI_EVENT_TYPE_PULSE | 0x0000,
   /** Pulse Digit detected. */
   IFX_TAPI_EVENT_PULSE_DIGIT          = IFX_TAPI_EVENT_TYPE_PULSE | 0x0001,

   /* DTMF */
   /** No event. */
   IFX_TAPI_EVENT_DTMF_NONE            = IFX_TAPI_EVENT_TYPE_DTMF | 0x0000,
   /** DTMF tone detected. */
   IFX_TAPI_EVENT_DTMF_DIGIT           = IFX_TAPI_EVENT_TYPE_DTMF | 0x0001,

   /* CID */
   /* TX */
   /** TX No event. */
   IFX_TAPI_EVENT_CID_TX_NONE          = IFX_TAPI_EVENT_TYPE_CID | 0x0000,
   /** Reserved - Start of CID TX sequence. */
   IFX_TAPI_EVENT_CID_TX_SEQ_START     = IFX_TAPI_EVENT_TYPE_CID | 0x0001,
   /** End of CID TX sequence. */
   IFX_TAPI_EVENT_CID_TX_SEQ_END       = IFX_TAPI_EVENT_TYPE_CID | 0x0002,
   /** Start of CID TX information. */
   IFX_TAPI_EVENT_CID_TX_INFO_START    = IFX_TAPI_EVENT_TYPE_CID | 0x0003,
   /** End of CID TX information. */
   IFX_TAPI_EVENT_CID_TX_INFO_END      = IFX_TAPI_EVENT_TYPE_CID | 0x0004,
   /** No acknowledge during CID sequence. */
   IFX_TAPI_EVENT_CID_TX_NOACK_ERR     = IFX_TAPI_EVENT_TYPE_CID | 0x0005,
   /** Ring cadence settings error in CID tx. */
   IFX_TAPI_EVENT_CID_TX_RINGCAD_ERR   = IFX_TAPI_EVENT_TYPE_CID | 0x0006,
   /** CID data buffer underrun. */
   IFX_TAPI_EVENT_CID_TX_UNDERRUN_ERR  = IFX_TAPI_EVENT_TYPE_CID | 0x0007,
   /** No 2nd acknowledge during CID sequence (NTT mode). */
   IFX_TAPI_EVENT_CID_TX_NOACK2_ERR    = IFX_TAPI_EVENT_TYPE_CID | 0x0008,
   /* RX */
   /** CID RX No event. */
   IFX_TAPI_EVENT_CID_RX_NONE          = IFX_TAPI_EVENT_TYPE_CID | 0x0010,
   /** CID CAS detected
       This is just an alias for IFX_TAPI_EVENT_FAXMODEM_CAS_BELL
       Please use IFX_TAPI_EVENT_FAXMODEM_CAS_BELL instead. */
   IFX_TAPI_EVENT_CID_RX_CAS           = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0014,
   /** CID RX, FSK detection ended. */
   IFX_TAPI_EVENT_CID_RX_END           = IFX_TAPI_EVENT_TYPE_CID | 0x0012,
   /** FSK Carrier Detected (Reserved). */
   IFX_TAPI_EVENT_CID_RX_CD            = IFX_TAPI_EVENT_TYPE_CID | 0x0013,
   /** Error during CID reception. */
   IFX_TAPI_EVENT_CID_RX_ERROR_READ    = IFX_TAPI_EVENT_TYPE_CID | 0x0014,
   /** Error during CID reception (Reserved). */
   IFX_TAPI_EVENT_CID_RX_ERROR1        = IFX_TAPI_EVENT_TYPE_CID | 0x0015,
   /** Error during CID reception (Reserved). */
   IFX_TAPI_EVENT_CID_RX_ERROR2        = IFX_TAPI_EVENT_TYPE_CID | 0x0016,

   /* TONE_GEN */
   /** No event. */
   IFX_TAPI_EVENT_TONE_GEN_NONE        = IFX_TAPI_EVENT_TYPE_TONE_GEN | 0x0000,
   /** Tone generator busy. */
   IFX_TAPI_EVENT_TONE_GEN_BUSY        = IFX_TAPI_EVENT_TYPE_TONE_GEN | 0x0001,
   /** Tone generation ended. */
   IFX_TAPI_EVENT_TONE_GEN_END         = IFX_TAPI_EVENT_TYPE_TONE_GEN | 0x0002,
   /** Tone generation end event used internally to trigger the statemachines
       - this event is not available to the application */
   IFX_TAPI_EVENT_TONE_GEN_END_RAW     = IFX_TAPI_EVENT_TYPE_TONE_GEN | 0x0003,

   /* TONE_DET */
   /** No event */
   IFX_TAPI_EVENT_TONE_DET_NONE        = IFX_TAPI_EVENT_TYPE_TONE_DET | 0x0000,
   /** Tone detect receive */
   IFX_TAPI_EVENT_TONE_DET_RECEIVE     = IFX_TAPI_EVENT_TYPE_TONE_DET | 0x0001,
   /** Tone detect transmit. */
   IFX_TAPI_EVENT_TONE_DET_TRANSMIT    = IFX_TAPI_EVENT_TYPE_TONE_DET | 0x0002,
   /** Call progress tone detected. */
   IFX_TAPI_EVENT_TONE_DET_CPT         = IFX_TAPI_EVENT_TYPE_TONE_DET | 0x0003,

   /* FAXMODEM_SIGNAL */
   /** No event. */
   IFX_TAPI_EVENT_FAXMODEM_NONE        = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0000,
   /** DIS preamble signal. */
   IFX_TAPI_EVENT_FAXMODEM_DIS         = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0001,
   /** 2100 Hz (CED) answering tone (ANS). */
   IFX_TAPI_EVENT_FAXMODEM_CED         = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0002,
   /** Phase reversal. */
   IFX_TAPI_EVENT_FAXMODEM_PR          = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0003,
   /** Amplitude modulation. */
   IFX_TAPI_EVENT_FAXMODEM_AM          = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0006,
   /** 1100 Hz single tone (CNG Fax). */
   IFX_TAPI_EVENT_FAXMODEM_CNGFAX      = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0008,
   /** 1300 Hz single tone (CNG Modem). It can indicate CT, V.18 XCI mark
    sequence. */
   IFX_TAPI_EVENT_FAXMODEM_CNGMOD      = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0009,
   /** 980 Hz single tone (V.21L mark sequence). */
   IFX_TAPI_EVENT_FAXMODEM_V21L        = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000a,
   /** 1400 Hz single tone (V.18A mark sequence). */
   IFX_TAPI_EVENT_FAXMODEM_V18A        = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000b,
   /** 1800 Hz single tone (V.27, V.32 carrier). */
   IFX_TAPI_EVENT_FAXMODEM_V27         = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000c,
   /** 2225 Hz single tone (Bell answering tone). */
   IFX_TAPI_EVENT_FAXMODEM_BELL        = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000d,
   /** 2250 Hz single tone (V.22 unscrambled binary ones). */
   IFX_TAPI_EVENT_FAXMODEM_V22         = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000e,
   /** 2225 Hz or 2250 Hz single tone, not possible to distinguish. */
   IFX_TAPI_EVENT_FAXMODEM_V22ORBELL   = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x000f,
   /** 600 Hz + 300 Hz dual tone (V.32 AC). */
   IFX_TAPI_EVENT_FAXMODEM_V32AC       = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0010,
   /** 1375 Hz + 2002 Hz dual tone (V.8bis initiating segment 1). */
   IFX_TAPI_EVENT_FAXMODEM_V8BIS       = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0011,
   /** Hold characteristic. */
   IFX_TAPI_EVENT_FAXMODEM_HOLDEND     = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0012,
   /** End of CED signal.*/
   IFX_TAPI_EVENT_FAXMODEM_CEDEND      = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0013,
   /** 2130 + 2750 Hz dual tone (CPE Alert Signal Bell Caller ID Type 2 Alert Tone) */
   IFX_TAPI_EVENT_FAXMODEM_CAS_BELL    = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0014,
   /** 1650 Hz single tone (V.21H mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V21H        = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0015,
   /** Voice modem discriminator */
   IFX_TAPI_EVENT_FAXMODEM_VMD         = IFX_TAPI_EVENT_TYPE_FAXMODEM_SIGNAL | 0x0016,

   /* CODer */
   /** COD No event (Reserved) . */
   IFX_TAPI_EVENT_COD_NONE             = IFX_TAPI_EVENT_TYPE_COD | 0x0000,
   /** Decoder change event. */
   IFX_TAPI_EVENT_COD_DEC_CHG          = IFX_TAPI_EVENT_TYPE_COD | 0x0001,
   /** Room noise detection: noise detected. */
   IFX_TAPI_EVENT_COD_ROOM_NOISE       = IFX_TAPI_EVENT_TYPE_COD | 0x0002,
   /** Room noise detection: silence detected. */
   IFX_TAPI_EVENT_COD_ROOM_SILENCE     = IFX_TAPI_EVENT_TYPE_COD | 0x0003,

   /* RTP */
   /** RTP No event (Reserved). */
   IFX_TAPI_EVENT_RTP_NONE             = IFX_TAPI_EVENT_TYPE_RTP | 0x0000,

   /* AAL */
   /** AAL No event (Reserved). */
   IFX_TAPI_EVENT_AAL_NONE             = IFX_TAPI_EVENT_TYPE_AAL | 0x0000,

   /* RFC2833 */
   /** No event. */
   IFX_TAPI_EVENT_RFC2833_NONE         = IFX_TAPI_EVENT_TYPE_RFC2833 | 0x0000,
   /** RFC2833 event. */
   IFX_TAPI_EVENT_RFC2833_EVENT        = IFX_TAPI_EVENT_TYPE_RFC2833 | 0x0001,

   /** KPI interface */
   /** No event. */
   IFX_TAPI_EVENT_KPI_NONE             = IFX_TAPI_EVENT_TYPE_KPI | 0x0000,
   /** ingress fifo full */
   IFX_TAPI_EVENT_KPI_INGRESS_FIFO_FULL= IFX_TAPI_EVENT_TYPE_KPI | 0x0001,

   /* T38 */
   /** No event. */
   IFX_TAPI_EVENT_T38_NONE             = IFX_TAPI_EVENT_TYPE_T38 | 0x0000,
   /** Generic error. */
   IFX_TAPI_EVENT_T38_ERROR_GEN        = IFX_TAPI_EVENT_TYPE_T38 | 0x0001,
   /** Overload error. */
   IFX_TAPI_EVENT_T38_ERROR_OVLD       = IFX_TAPI_EVENT_TYPE_T38 | 0x0002,
   /** Read error. */
   IFX_TAPI_EVENT_T38_ERROR_READ       = IFX_TAPI_EVENT_TYPE_T38 | 0x0003,
   /** Write error. */
   IFX_TAPI_EVENT_T38_ERROR_WRITE      = IFX_TAPI_EVENT_TYPE_T38 | 0x0004,
   /** Error in data transmission. The datapump is switched off. */
   IFX_TAPI_EVENT_T38_ERROR_DATA       = IFX_TAPI_EVENT_TYPE_T38 | 0x0005,
   /** Setup error. */
   IFX_TAPI_EVENT_T38_ERROR_SETUP      = IFX_TAPI_EVENT_TYPE_T38 | 0x0006,
   /** Fax Data Pump Request, used only in polling mode */
   IFX_TAPI_EVENT_T38_FDP_REQ          = IFX_TAPI_EVENT_TYPE_T38 | 0x0007,

   /* JB */
   /** JITTER evnets (Reserved) */
   IFX_TAPI_EVENT_JB_NONE              = IFX_TAPI_EVENT_TYPE_JB | 0x0000,

   /* DOWNLOAD */
   /** DOWNLOAD events (reserved). */
   IFX_TAPI_EVENT_DOWNLOAD_NONE        = IFX_TAPI_EVENT_TYPE_DOWNLOAD | 0x0000,

   /* INFORMATION */
   /** INFORMATION events (reserved). */
   IFX_TAPI_EVENT_INFO_NONE            = IFX_TAPI_EVENT_TYPE_INFO | 0x0000,
   /** Information mailbox congestion in downstream direction,
       packet was dropped. */
   IFX_TAPI_EVENT_INFO_MBX_CONGESTION  = IFX_TAPI_EVENT_TYPE_INFO | 0x0001,

   /* DEBUG */
   /** For DEBUG purposes. (Reserved). */
   IFX_TAPI_EVENT_DEBUG_NONE           = IFX_TAPI_EVENT_TYPE_DEBUG | 0x0000,
   /** Debug command error event (reserved). */
   IFX_TAPI_EVENT_DEBUG_CERR           = IFX_TAPI_EVENT_TYPE_DEBUG | 0x0001,

   /* low level driver events */
   /** DEVICE specific events (Reserved). */
   IFX_TAPI_EVENT_LL_DRIVER_NONE       = IFX_TAPI_EVENT_TYPE_LL_DRIVER | 0x0000,
   /** DEVICE specific events (Reserved). */
   IFX_TAPI_EVENT_LL_DRIVER_WD_FAIL = IFX_TAPI_EVENT_TYPE_LL_DRIVER | 0x0001,

   /* FAULT_GENERAL */
   /** Generic fault, no event (reserved). */
   IFX_TAPI_EVENT_FAULT_GENERAL_NONE   = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL | 0x0000,
   /** General system fault (reserved). */
   IFX_TAPI_EVENT_FAULT_GENERAL        = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL | 0x1,
   /** General system fault (reserved). */
   IFX_TAPI_EVENT_FAULT_GENERAL_CHINFO = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL | 0x2,
   /** General device fault (reserved). */
   IFX_TAPI_EVENT_FAULT_GENERAL_DEVINFO = IFX_TAPI_EVENT_TYPE_FAULT_GENERAL | 0x3,

   /* FAULT_LINE */
   /** Reserved. Line fault, no event. */
   IFX_TAPI_EVENT_FAULT_LINE_NONE      = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0000,
   /** Ground Key, positive polarity. */
   IFX_TAPI_EVENT_FAULT_LINE_GK_POS    = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0001,
   /** Ground Key, negative polarity. */
   IFX_TAPI_EVENT_FAULT_LINE_GK_NEG    = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0002,
   /** Ground key low. */
   IFX_TAPI_EVENT_FAULT_LINE_GK_LOW    = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0003,
   /** Ground key high. */
   IFX_TAPI_EVENT_FAULT_LINE_GK_HIGH   = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0004,
   /** Overtemperature. */
   IFX_TAPI_EVENT_FAULT_LINE_OVERTEMP  = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0005,
   /** Overcurrent */
   IFX_TAPI_EVENT_FAULT_LINE_OVERCURRENT = IFX_TAPI_EVENT_TYPE_FAULT_LINE | 0x0006,

   /* FAULT_HW */
   /** Reserved */
   IFX_TAPI_EVENT_FAULT_HW_NONE        = IFX_TAPI_EVENT_TYPE_FAULT_HW | 0x0000,
   /** SPI access error */
   IFX_TAPI_EVENT_FAULT_HW_SPI_ACCESS  = IFX_TAPI_EVENT_TYPE_FAULT_HW | 0x0001,
   /** Clock failure */
   IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL  = IFX_TAPI_EVENT_TYPE_FAULT_HW | 0x0002,
   /** Clock failure */
   IFX_TAPI_EVENT_FAULT_HW_CLOCK_FAIL_END  = IFX_TAPI_EVENT_TYPE_FAULT_HW | 0x0003,
   /** Hardware failure */
   IFX_TAPI_EVENT_FAULT_HW_FAULT       = IFX_TAPI_EVENT_TYPE_FAULT_HW | 0x0004,

   /* FAULT_FW */
   /** Reserved */
   IFX_TAPI_EVENT_FAULT_FW_NONE        = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0000,
   /** Event mailbox out underflow */
   IFX_TAPI_EVENT_FAULT_FW_EBO_UF      = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0001,
   /** Event mailbox out overflow */
   IFX_TAPI_EVENT_FAULT_FW_EBO_OF      = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0002,
   /** Command mailbox out underflow */
   IFX_TAPI_EVENT_FAULT_FW_CBO_UF      = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0003,
   /** Command mailbox out overflow */
   IFX_TAPI_EVENT_FAULT_FW_CBO_OF      = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0004,
   /** Command mailbox in overflow */
   IFX_TAPI_EVENT_FAULT_FW_CBI_OF      = IFX_TAPI_EVENT_TYPE_FAULT_FW | 0x0005,

   /* FAULT_SW */
   /** Reserved */
#ifdef TAPI_EXT_KEYPAD
   IFX_TAPI_EVENT_FAULT_SW_NONE        = IFX_TAPI_EVENT_TYPE_FAULT_SW | 0x0000,
   IFX_TAPI_EVENT_EXT_KEY_UP           = IFX_TAPI_EVENT_TYPE_EXT | 0x0000,
   IFX_TAPI_EVENT_EXT_KEY_DOWN         = IFX_TAPI_EVENT_TYPE_EXT | 0x0001
#else
   IFX_TAPI_EVENT_FAULT_SW_NONE        = IFX_TAPI_EVENT_TYPE_FAULT_SW | 0x0000
#endif /*TAPI_EXT_KEYPAD*/
}IFX_TAPI_EVENT_ID_t;

#ifdef TAPI_EXT_KEYPAD
/** This structure is used to report a DTMF event to the TAPI from an external
    software module.*/
typedef struct
{
   /** Key. */
   IFX_int8_t  key;
   /** Duration. */
   IFX_char_t  duration;
   /*  IFX_uint32_t TimeStamp;*/
}IFX_TAPI_EVENT_DATA_EXT_KEYPAD_t;
#endif /*TAPI_EXT_KEYPAD*/


/** extended T.38 event types for IFX_TAPI_EVENT_T38_ERROR_SETUP */
typedef enum
{
   /** error in data pump disabling */
   IFX_TAPI_EVENT_T38_ERROR_SETUP_DPOFF   = 0x0001,
   /** error in modulator enabling */
   IFX_TAPI_EVENT_T38_ERROR_SETUP_MODON   = 0x0002,
   /** error in demodulator enabling */
   IFX_TAPI_EVENT_T38_ERROR_SETUP_DEMODON = 0x0003
} IFX_TAPI_EVENT_T38_ERROR_SETUP_t;

/** extended T.38 event types for IFX_TAPI_EVENT_T38_ERROR_DATA */
typedef enum
{
   /** Error modulator signal buffer underrun */
   IFX_TAPI_EVENT_T38_ERROR_DATA_MBSU   = 0x0001,
   /** Error demodulator signal buffer overflow */
   IFX_TAPI_EVENT_T38_ERROR_DATA_DBSO   = 0x0002,
   /** Error modulator data buffer overflow */
   IFX_TAPI_EVENT_T38_ERROR_DATA_MBDO   = 0x0003,
   /** Error modulator data buffer underrun */
   IFX_TAPI_EVENT_T38_ERROR_DATA_MBDU   = 0x0004,
   /** Error demodulator data buffer overflow */
   IFX_TAPI_EVENT_T38_ERROR_DATA_DBDO   = 0x0005
} IFX_TAPI_EVENT_T38_ERROR_DATA_t;

/* =============================== */
/* type definition                 */
/* =============================== */
/** This structure contains the data specific to the Pulse dialing event. */
typedef struct
{
   /** Reserved.*/
   IFX_uint16_t reserved:8;
   /** Pulse digit number information. */
   IFX_uint16_t digit:8;
} IFX_TAPI_EVENT_DATA_PULSE_t;

/** This structure contains the data specific to the DTMF event. */
typedef struct
{
   /** direction bit: IFX_TAPI_EVENT_LOCAL or IFX_TAPI_EVENT_NETWORK */
   IFX_uint32_t local:1;
   IFX_uint32_t network:1;
   IFX_uint32_t reserved:6;
   /** DTMF digit number information */
   IFX_uint32_t digit:8;
   /** DTMF digit in ASCII representation */
   IFX_uint32_t ascii:8;
} IFX_TAPI_EVENT_DATA_DTMF_t;

/** This structure contains the data specific to the TONE Generation event. */
typedef struct
{
   /** Detected from local side.*/
   IFX_uint32_t local:1;
   /** Detected from network side. */
   IFX_uint32_t network:1;
   /** Reserved. */
   IFX_uint32_t reserved:6;
   /** TONE table index. */
   IFX_uint32_t index:8;
} IFX_TAPI_EVENT_DATA_TONE_GEN_t;

/** This structure contains the data specific to the FAX event. */
typedef struct
{
   /** Detected from local side. */
   IFX_uint32_t local:1;
   /** Detected from network side. */
   IFX_uint32_t network:1;
   /* Last event or not. only be used in ioctl.*/
   IFX_uint32_t reserved:6;
   /** FAX or modem signal. */
   IFX_uint32_t signal:8;
} IFX_TAPI_EVENT_DATA_FAX_SIG_t;

/** This structure contains the data specific to the RFC2833 event. */
typedef struct
{
   /** Event code contained in the RFC2833 frame. */
   IFX_uint32_t event:16;
} IFX_TAPI_EVENT_DATA_RFC2833_t;

/** This structure contains the data specific to the Decoder Change event. */
typedef struct
{
   /** Type of the coder used. Please refer to coding of
   \ref IFX_TAPI_COD_TYPE_t.*/
   IFX_uint32_t dec_type        : 8;
   /** Frame length. Please refer to coding of \ref IFX_TAPI_COD_LENGTH_t. */
   IFX_uint32_t dec_framelength : 8;
} IFX_TAPI_EVENT_DATA_DEC_CHG_t;

/** This structure contains the data specific to the Command Error event. */
typedef struct
{
   /** Firmware family identifier used to decode the reason field. */
   IFX_uint16_t fw_id;
   /** Reason given by the firmware for the command error. */
   IFX_uint16_t reason;
   /** Header of error command. */
   IFX_uint32_t command;
} IFX_TAPI_EVENT_DATA_CERR_t;

/** Union for the possibly reportable events. */
typedef union
{
   /** Pulse digit information.*/
   IFX_TAPI_EVENT_DATA_PULSE_t pulse;
   /** DTMF digit information. */
   IFX_TAPI_EVENT_DATA_DTMF_t  dtmf;
   /** Tone generation index. */
   IFX_TAPI_EVENT_DATA_TONE_GEN_t tone_gen;
   /** Fax/modem signal information. */
   IFX_TAPI_EVENT_DATA_FAX_SIG_t fax_sig;
   /** RFC2833 event information. */
   IFX_TAPI_EVENT_DATA_RFC2833_t rfc2833;
   /** Decoder change event details. */
   IFX_TAPI_EVENT_DATA_DEC_CHG_t dec_chg;
   /** Command error event details. */
   IFX_TAPI_EVENT_DATA_CERR_t cerr;
   /** Reserved. */
   IFX_uint16_t value;
#ifdef TAPI_EXT_KEYPAD
   /** External keypad event information. */
   IFX_TAPI_EVENT_DATA_EXT_KEYPAD_t  keyinfo;
#endif /*TAPI_EXT_KEYPAD*/
    /** Error line. */
   IFX_TAPI_ErrorLine_t* error;
}IFX_TAPI_EVENT_DATA_t;

/** This structure is reported by an "EVENT_GET" ioctl. For event masking
 "EVENT_MASK" reusing IFX_TAPI_EVENT_t should be used. */
typedef struct
{
   /** Event type and sub-type. */
   IFX_TAPI_EVENT_ID_t id;
   /** Channel information of event. If set to \ref IFX_TAPI_EVENT_ALL_CHANNELS this
       information is global. */
   IFX_uint16_t ch;
   /** This field is used to report whether new events are ready (\ref IFX_TRUE)
    or not (\ref IFX_FALSE). */
   IFX_uint16_t more;
   /** Specific data of individual event. */
   IFX_TAPI_EVENT_DATA_t data;
} IFX_TAPI_EVENT_t;
/*@}*/ /* TAPI_INTERFACE_EVENT */

/* ========================================================================= */
/*                      TAPI Old Interface Adaptations                       */
/* ========================================================================= */

#ifdef TAPI_OLD_IO
/* old TAPI ioctl commands for backward compatibility */
#include "drv_tapi_old_io.h"
#endif /* TAPI_OLD_IO */

#ifndef IFX_TAPI_ENC_CFG_SET_t
#define IFX_TAPI_ENC_CFG_SET_t  IFX_TAPI_ENC_CFG_t
#endif

#ifndef IFX_TAPI_DECT_ENC_CFG_SET_t
#define IFX_TAPI_DECT_ENC_CFG_SET_t  IFX_TAPI_DECT_ENC_CFG_t
#endif

#endif  /* DRV_TAPI_IO_H */
