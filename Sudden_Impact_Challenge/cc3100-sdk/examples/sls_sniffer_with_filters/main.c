/*
 * main.c - sample application for using Sniffer with NWP filters
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * Application Name     -  SLS Sniffer with NWP Filter Application
 * Application Overview -  This sample application demonstrates
 *                         a. How to use CC3100 as Sniffer
 *                         b. How NWP filters can be enabled to filtering unwanted packets
 *
 * Application Details  -  http://processors.wiki.ti.com/index.php/CC31xx_SLS_Sniffer_with_Filters_Application 
 *                          doc\examples\sls_sniffer_with_filters.pdf
 */

#include <windows.h>
#include <stdio.h>
#include "simplelink.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

/**/
#define LOOP_FOREVER() \
            {\
                while(1); \
            }

#define ASSERT_ON_ERROR(error_code) \
            {\
                /* Handling the error-codes is specific to the application */ \
                if (error_code < 0) return error_code; \
                /* else, continue w/ execution */ \
            }

#define SUCCESS             0
#define MAC_ADDR_LENGTH     (6)
#define MAC_ADDR_LEN_STR    (18)    /*xx:xx:xx:xx:xx:xx*/
#define IP_ADDR_LENGTH      (4)
#define IP_ADDR_LEN_STR     (16)    /*xxx:xxx:xxx:xxx*/
#define MAX_RECV_BUF_SIZE   1536
#define SWAP_UINT32(val)    (((val>>24) & 0x000000FF) + ((val>>8) & 0x0000FF00) + \
                            ((val<<24) & 0xFF000000) + ((val<<8) & 0x00FF0000))
#define SWAP_UINT16(val)    ((((val)>>8) & 0x00FF) + (((val)<<8) & 0xFF00))

#define FRAME_TYPE_MASK     0x0C
#define FRAME_SUBTYPE_MASK  0xF0

typedef _u8 SlrxFilterOperationResult_t;

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM 21
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */



typedef struct{
    _u8   rate;          /* Received Rate:at ETxRateClassId format */
    _u8   channel;       /* The received channel */
    _i8    rssi;          /* The computed RSSI value in db of current frame */
    _u8   padding;       /* pad to align to 32 bits */
    _u32  timestamp;     /* Time stamp in microseconds */

}TransceiverRxOverHead_t;

/* Status bits - These are used to set/reset the corresponding bits in a 'status_variable' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in a 'status_variable', the device is connected to the AP
                                 *      0 in a 'status_variable', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_ACQUIRED       /* If this bit is:
                                 *      1 in a 'status_variable', the device has acquired an IP
                                 *      0 in a 'status_variable', the device has not acquired an IP
                                 */

}e_StatusBits;

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    INVALID_PARENT_FILTER_ID   = DEVICE_NOT_IN_STATION_MODE - 1,
    
    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))

#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_ACQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_ACQUIRED)

/*
 * GLOBAL VARIABLES -- Start
 */
_u8 buffer[MAX_RECV_BUF_SIZE] = {'\0'};
_u32  g_Status = 0;
_u8   g_Exit = 0;
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 initializeAppVariables();
static void displayBanner();
static _i32 RxFiltersExample(_i8 input, _i32 filterNumber,
                                const _u8 *filterParam, _i8 equalOrNot,
                                _i8 dropOrNot, _i8 parentId);
static void PrintFilterType(_i32 sel);
static _i32 FiltersMenu();
static _i32 ChooseFilters();
static void PrintFrameSubtype(_u8 MAC);
static _i32 Sniffer(_i32 channel,_i32 numpackets);
/*
 * STATIC FUNCTION DEFINITIONS -- End
 */



/*
 * ASYNCHRONOUS EVENT HANDLERS -- Start
 */

/*!
    \brief This function handles WLAN events

    \param[in]      pWlanEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
        printf(" [WLAN EVENT] NULL Pointer Error \n\r");
    
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

            /*
             * Information about the connected AP (like name, MAC etc) will be
             * available in 'slWlanConnectAsyncResponse_t' - Applications
             * can use it if required
             *
             * slWlanConnectAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
             *
             */
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                printf(" Device disconnected from the AP on application's request \r\n");
            }
            else
            {
                printf(" Device disconnected from the AP on an ERROR..!! \r\n");
            }
        }
        break;

        default:
        {
            printf(" [WLAN EVENT] Unexpected event \r\n");
        }
        break;
    }
}


/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication

    \param[in]      pNetAppEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
        printf(" [NETAPP EVENT] NULL Pointer Error \n\r");
 
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {

            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
            /*
             * Information about the connected AP's ip, gateway, DNS etc
             * will be available in 'SlIpV4AcquiredAsync_t' - Applications
             * can use it if required
             *
             * SlIpV4AcquiredAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
             * <gateway_ip> = pEventData->gateway;
             *
             */

        }
        break;

        default:
        {
            printf(" [NETAPP EVENT] Unexpected event \r\n");
        }
        break;
    }
}



/*!
    \brief This function handles callback for the HTTP server events

    \param[in]      pHttpEvent - Contains the relevant event information
    \param[in]      pHttpResponse - Should be filled by the user with the
                    relevant response information

    \return         None

    \note

    \warning
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    printf(" [HTTP EVENT] Unexpected event\r\n");
}

/*!
    \brief This function handles general error events indication

    \param[in]      pDevEvent is the event passed to the handler

    \return         None
*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
    printf(" [GENERAL EVENT] \r\n");
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler

    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
        printf(" [SOCK EVENT] NULL Pointer Error \n\r");
    
    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
        {
            /*
            * TX Failed
            *
            * Information about the socket descriptor and status will be
            * available in 'SlSockEventData_t' - Applications can use it if
            * required
            *
            * SlSockEventData_t *pEventData = NULL;
            * pEventData = & pSock->EventData;
            */
            switch( pSock->EventData.status )
            {
                case SL_ECLOSE:
                  /*
                   * Close Socket operation:- Failed to transmit all the
                   * queued packets.
                   */
                    printf(" [SOCK EVENT] Failed to transmit packets \r\n");
                break;
                default:
                    printf(" [SOCK EVENT] Unexpected event \r\n");
                break;
            }
        }
        break;

        default:
            printf(" [SOCK EVENT] Unexpected event \r\n");
        break;
    }
}
/*
 * * ASYNCHRONOUS EVENT HANDLERS -- End
 */

/*
 * Application's entry point
 */
int main( int argc, char *argv[] )
{
    _i32 channel = 0;
    _i32 numpackets = 0;
    _i32 retVal = -1;
    _i8 *pConfig = NULL;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

#ifdef SL_IF_TYPE_UART
    params.BaudRate = 115200;
    params.FlowControlEnable = 1;
    params.CommPort = COMM_PORT_NUM;

    pConfig = (_i8 *)&params;
#endif /* SL_IF_TYPE_UART */


    /*This line is for Eclipse CDT only due to a known bug in console buffering
     * See https://bugs.eclipse.org/bugs/show_bug.cgi?id=173732 */
    setvbuf(stdout, NULL, _IONBF, 0);

    displayBanner();

    /*
     * Following function configures the device to default state by cleaning
     * the persistent settings stored in NVMEM (viz. connection profiles &
     * policies, power policy etc)
     *
     * Applications may choose to skip this step if the developer is sure
     * that the device is in its default state at start of application
     *
     * Note that all profiles and persistent settings that were done on the
     * device will be lost
     */
    retVal = configureSimpleLinkToDefaultState(pConfig);
    if(retVal < 0)
    {
        printf(" Failed to configure the device in its default state, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    printf("Device is configured in default state \r\n");

    /*
     * Assumption is that the device is configured in station mode already
     * and it is in its default state
     */
    retVal = sl_Start(0, pConfig, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        printf(" Failed to start the device, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    printf("Device is configured in default state \r\n");
    
    printf("Notes: There is at least a 0.5 second delay between each packet displayed.\n");
    printf("The command prompt can display a maximum of 50 packets.\n\n");
    printf("Please input desired channel number and click \"Enter\".\n");
    printf("Valid channels range from 1 to 13 (11 is standard): \n");
    fflush(stdin);
    scanf_s("%d",&channel, sizeof(channel));
    printf("Please input desired number of packets and click \"Enter\": \n");
    fflush(stdin);
    scanf_s("%d",&numpackets, sizeof(numpackets));
    retVal = Sniffer(channel, numpackets);
    if(retVal < 0)
        LOOP_FOREVER();

    system("PAUSE");

    return 0;
}

/*!
    \brief This function displays the application's banner

    \param      None

    \return     None
*/
static void displayBanner()
{
    printf("\r\n\r\n");
    printf("Sniffer with  Filter Application: \r\n");
    printf(APPLICATION_VERSION);
    printf("\r\n******************************************************\r\n");
}

/*!
    \brief This function configure the SimpleLink device in its default state. It:
           - Sets the mode to STATION
           - Configures connection policy to Auto and AutoSmartConfig
           - Deletes all the stored profiles
           - Enables DHCP
           - Disables Scan policy
           - Sets Tx power to maximum
           - Sets power policy to normal
           - Unregister mDNS services
           - Remove all filters

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/
static _i32 configureSimpleLinkToDefaultState(_i8 *pConfig)
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    _u8           val = 1;
    _u8           configOpt = 0;
    _u8           configLen = 0;
    _u8           power = 0;

    _i32          retVal = -1;
    _i32          mode = -1;

    mode = sl_Start(0, pConfig, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in station-mode */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while(!IS_IP_ACQUIRED(g_Status));
        }

        /* Switch to STA role and restart */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Start(0, pConfig, 0);
        ASSERT_ON_ERROR(retVal);

        /* Check if the device is in station again */
        if (ROLE_STA != retVal)
        {
            /* We don't want to proceed if the device is not coming up in station-mode */
            ASSERT_ON_ERROR(DEVICE_NOT_IN_STATION_MODE);
        }
    }

    /* Get the device's version-information */
    configOpt = SL_DEVICE_GENERAL_VERSION;
    configLen = sizeof(ver);
    retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (_u8 *)(&ver));
    ASSERT_ON_ERROR(retVal);

    printf("Host Driver Version: %s\n",SL_DRIVER_VERSION);
    printf("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n",
                        ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
                        ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
                        ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
                        ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
                        ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove all profiles */
    retVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retVal);

    /*
     * Device in station-mode. Disconnect previous connection if any
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
    retVal = sl_WlanDisconnect();
    if(0 == retVal)
    {
        /* Wait */
        while(IS_CONNECTED(g_Status));
    }

    /* Enable DHCP client*/
    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);
    ASSERT_ON_ERROR(retVal);

    /* Disable scan */
    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
    power = 0;
    retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *)&power);
    ASSERT_ON_ERROR(retVal);

    /* Set PM policy to normal */
    retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Unregister mDNS services */
    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove  all 64 filters (8*8) */
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(retVal);

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    return retVal; /* Success */
}

/*!
    \brief This function initializes the application variables

    \param[in]  None

    \return     0 on success, negative error-code on error
*/
static _i32 initializeAppVariables()
{
    g_Status = 0;
    g_Exit = 0;
    
    return SUCCESS;
}

/*!
    \brief This function creates filters based on rule specified by user

    \param[in]   Input : Filter chosen   
    \param[in]   FilterNumber : Chosen filter type ( Source MAC ID, Dst MAC ID,
                                BSS ID,  IP Address etc)
    \param[in]   Filterparams: parameters  of chosen filter type 
    \param[in]   Filter Rule:  Check for equal or Not
    \param[in]   Filter Rule : If Rule match, to drop packet or pass to Host
    \param[in]   parent Id : in case sub-filter of existing filer, id of the parent filter

    \return      Unique filter ID in long format for success, -ve otherwise

    \note

    \warning
*/
static _i32 RxFiltersExample(_i8 input, _i32 filterNumber,
                                const _u8 *filterParam, _i8 equalOrNot,
                                _i8 dropOrNot, _i8 parentId)
{
    SlrxFilterID_t          FilterId = 0;
    SlrxFilterRuleType_t    RuleType = 0;
    SlrxFilterFlags_t       FilterFlags = {0};
    SlrxFilterRule_t        Rule = {0};
    SlrxFilterTrigger_t     Trigger = {0};
    SlrxFilterAction_t      Action = {0};

    _u8 MacMAsk[6]      = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    _u8 FrameMask[1]    = {0xFF};
    _u8 IPMask[4]       = {0xFF,0xFF,0xFF,0xFF};
    _u8 zeroMask[4]     = {0x00,0x00,0x00,0x00};

    _i32 retVal = -1;
    _u8 frameByte = 0;

    switch(input)
    {
        case '1': /* Create filter */
            /* Rule definition */
            RuleType = HEADER;
            FilterFlags.IntRepresentation = RX_FILTER_BINARY;
            /* When RX_FILTER_COUNTER7 is bigger than 0 */
            Trigger.Trigger = NO_TRIGGER;

            /* connection state and role */
            Trigger.TriggerArgConnectionState.IntRepresentation = RX_FILTER_CONNECTION_STATE_STA_NOT_CONNECTED;
            Trigger.TriggerArgRoleStatus.IntRepresentation = RX_FILTER_ROLE_PROMISCUOUS;

            switch (filterNumber)
            {
                case 1:
                    Rule.HeaderType.RuleHeaderfield = MAC_SRC_ADDRESS_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB6BytesRuleArgs[0],
                                                                       filterParam, 6);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                           MacMAsk, 6);
                    break;
                case 2:
                    Rule.HeaderType.RuleHeaderfield = MAC_DST_ADDRESS_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB6BytesRuleArgs[0],
                                                                       filterParam, 6);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                           MacMAsk, 6);
                    break;
                case 3:
                    Rule.HeaderType.RuleHeaderfield = BSSID_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB6BytesRuleArgs[0],
                                                                       filterParam, 6);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                           MacMAsk, 6);
                    break;
                case 4:
                {
                    frameByte = (*filterParam & FRAME_TYPE_MASK);

                    Rule.HeaderType.RuleHeaderfield = FRAME_TYPE_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB1BytesRuleArgs[0],
                                                                       &frameByte, 1);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                          FrameMask, 1);
                }
                break;
                case 5:
                {
                    if(parentId <=0 )
                    {
                        printf("\n[Error] Enter a parent frame type filter id for frame subtype filter\r\n");
                        return INVALID_PARENT_FILTER_ID;
                    }
                    
                    frameByte = (*filterParam & FRAME_SUBTYPE_MASK);

                    Rule.HeaderType.RuleHeaderfield = FRAME_SUBTYPE_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB1BytesRuleArgs[0],
                                                                       &frameByte, 1);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                          FrameMask, 1);
                }
                    break;
                case 6:
                    Rule.HeaderType.RuleHeaderfield = IPV4_SRC_ADRRESS_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB4BytesRuleArgs[0],
                                                                       filterParam, 4);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                            IPMask, 4);
                    break;
                case 7:
                    Rule.HeaderType.RuleHeaderfield = IPV4_DST_ADDRESS_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB4BytesRuleArgs[0],
                                                                       filterParam, 4);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                            IPMask, 4);
                    break;
                case 8:
                    Rule.HeaderType.RuleHeaderfield = FRAME_LENGTH_FIELD;
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB4BytesRuleArgs[0],
                                                                          zeroMask, 4);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                            IPMask, 4);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgs.RxFilterDB4BytesRuleArgs[1],
                                                                       filterParam, 4);
                    memcpy(Rule.HeaderType.RuleHeaderArgsAndMask.RuleHeaderArgsMask,
                                                                            IPMask, 4);
                    break;
            }

            switch(equalOrNot)
            {
                case 'y':
                    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_EQUAL;
                    break;
                case 'h':
                    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_NOT_IN_BETWEEN;
                    break;
                case 'l':
                    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_IN_BETWEEN;
                    break;
                case 'n':
                    Rule.HeaderType.RuleCompareFunc = COMPARE_FUNC_NOT_EQUAL_TO;
                    break;
            }

            Trigger.ParentFilterID = parentId;

            /* Action */
            if(dropOrNot == 'y')
            {
                Action.ActionType.IntRepresentation = RX_FILTER_ACTION_DROP;
            }
            else
            {
                Action.ActionType.IntRepresentation = RX_FILTER_ACTION_NULL;
            }

            retVal = sl_WlanRxFilterAdd(RuleType,
                                            FilterFlags,
                                            &Rule,
                                            &Trigger,
                                            &Action,
                                            &FilterId);
            if( retVal < 0)
            {
                printf("\nError creating the filter. Error number: %d.\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }
            
            printf("\nThe filter ID is %d\n",FilterId);
            break;

        case '2': /* remove filter */
        {
                _WlanRxFilterOperationCommandBuff_t     RxFilterIdMask ;
                memset(RxFilterIdMask.FilterIdMask, 0xFF , 8);
                retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                                    sizeof(_WlanRxFilterOperationCommandBuff_t));
                ASSERT_ON_ERROR(retVal);

        }
        break;

        case '3' : /* enable\disable filter */
        {
            _WlanRxFilterOperationCommandBuff_t     RxFilterIdMask ;
            memset(RxFilterIdMask.FilterIdMask, 0xFF , 8);
            retVal = sl_WlanRxFilterSet(SL_ENABLE_DISABLE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                                sizeof(_WlanRxFilterOperationCommandBuff_t));
            ASSERT_ON_ERROR(retVal);
        }

        break;
    }

    return FilterId;
}

/*!
    \brief This function displays the parameter required to create the filter 
           of selected type.

    \param[in]   sel : filter type selected

    \return         None

    \note

    \warning
*/
static void PrintFilterType(_i32 sel)
{
    printf("\nFilter Parameter:\n");
    switch(sel)
    {
        case 1:
            printf("Source MAC Address\n");
            break;
        case 2:
            printf("Destination MAC Address\n");
            break;
        case 3:
            printf("BSSID\n");
            break;
        case 4:
            printf("Frame type\n");
            break;
        case 5:
            printf("Frame Subtype\n");
            break;
        case 6:
            printf("Source IP Address\n");
            break;
        case 7:
            printf("Destination IP Address\n");
            break;
        case 8:
            printf("Packet Length\n");
            break;
    }
}

/*!
    \brief This function displays the filter options and read parameters to 
            create the filter.

    \param[in]   none

    \return      0 for success, -ve otherwise

    \note

    \warning
*/
static _i32 FiltersMenu()
{
    _i32  selection = -1;
    _i8   equalYesNo = -1;
    _i8   dropYesNo = -1;
    _u32  frameTypeLength = 0;
    _i32  fatherId = 0;
    _u32  macAddress[MAC_ADDR_LENGTH] = {'\0'};
    _u32  ipAddress[IP_ADDR_LENGTH] = {'\0'};
    _u8   filterData[MAC_ADDR_LENGTH] = {'\0'};
    _i32  retVal = -1;
    _i32  idx = 0;

    printf("\nPlease select a filter parameter:\n");
    printf("\n1. Source MAC address\n");
    printf("2. Destination MAC address\n");
    printf("3. BSSID\n");
    printf("4. Frame type\n");
    printf("5. Frame subtype\n");
    printf("6. Source IP address\n");
    printf("7. Destination IP address\n");
    printf("8. Packet length\n");
    printf("9. Remove filter and exit menu\n");
    printf("10. Enable filter and exit menu\n");
    printf("Selection: \n");
    while(1)
    {
        fflush(stdin);
        scanf_s("%d",&selection, sizeof(selection));

        switch(selection)
        {
            case 1:
            case 2:
            case 3:
                PrintFilterType(selection);
                printf("\nEnter the MAC address (xx:xx:xx:xx:xx:xx): \n");
                fflush(stdin);
                scanf("%2x:%2x:%2x:%2x:%2x:%2x", &macAddress[0],
                                                 &macAddress[1],
                                                 &macAddress[2],
                                                 &macAddress[3],
                                                 &macAddress[4],
                                                 &macAddress[5]);
                for(idx = 0 ; idx < MAC_ADDR_LENGTH ; idx++ )
                {
                    filterData[idx] = (_u8)macAddress[idx];
                }
                break;

            case 4:
                PrintFilterType(selection);
                printf("Enter the frame type byte: \n");
                fflush(stdin);
                scanf_s("%2x",&frameTypeLength, sizeof(frameTypeLength));
                filterData[0] = (_u8)frameTypeLength;
                break;

            case 5:
                PrintFilterType(selection);

                printf("\nCreating a frame subtype filter requires a parent frame type filter\r\n");

                printf("Enter the frame type byte: \n");
                fflush(stdin);
                scanf_s("%2x",&frameTypeLength, sizeof(frameTypeLength));
                filterData[0] = (_u8)frameTypeLength;
                break;

            case 6:
            case 7:
                PrintFilterType(selection);
                printf("Enter the IP address: \n");
                fflush(stdin);
                scanf("%u.%u.%u.%u",&ipAddress[0],&ipAddress[1],&ipAddress[2],
                                                                 &ipAddress[3]);
                for(idx = 0 ; idx < IP_ADDR_LENGTH ; idx++ )
                {
                    filterData[idx] = (_u8)ipAddress[idx];
                }
                break;

            case 8:
                PrintFilterType(selection);
                printf("Enter desired length in Bytes (Maximum = 1472): \n");
                fflush(stdin);
                scanf_s("%u",&frameTypeLength, sizeof(frameTypeLength));
                *(_u32 *)filterData = SWAP_UINT32(frameTypeLength);
                printf("Target what lengths? (h - Higher than %u | l - Lower than %u): \n",
                                                  frameTypeLength,frameTypeLength);
                fflush(stdin);
                scanf_s("%c",&equalYesNo, sizeof(equalYesNo));
                printf("Drop packets or not? (y/n): \n");
                fflush(stdin);
                scanf_s("%c",&dropYesNo, sizeof(dropYesNo));
                printf("Enter filter ID of parent. Otherwise 0: \n");
                fflush(stdin);
                scanf_s("%u", &fatherId, sizeof(fatherId));
                retVal= RxFiltersExample('1',selection,filterData,equalYesNo,dropYesNo,
                                                                   (_i8)fatherId);
                ASSERT_ON_ERROR(retVal);

                printf("\nPlease select a filter parameter:\n");
                printf("\n1. Source MAC address\n");
                printf("2. Destination MAC address\n");
                printf("3. BSSID\n");
                printf("4. Frame type\n");
                printf("5. Frame subtype\n");
                printf("6. Source IP address\n");
                printf("7. Destination IP address\n");
                printf("8. Packet length\n");
                printf("9. Remove filter and exit menu\n");
                printf("10. Enable filter and exit menu\n");
                printf("\nSelection: \n");
                continue;
                break;

            case 9:
                retVal = RxFiltersExample('2',0,NULL,0,0,0);
                ASSERT_ON_ERROR(retVal);

                return SUCCESS;
                break;

            case 10:
                retVal = RxFiltersExample('3',0,NULL,0,0,0);
                ASSERT_ON_ERROR(retVal);

                return SUCCESS;
                break;

            default:
                continue;
        }

        printf("Equal or not equal? (y/n): \n");
        fflush(stdin);
        scanf_s("%c",&equalYesNo, sizeof(equalYesNo));

        printf("Drop the packet? (y/n): \n");
        fflush(stdin);
        scanf_s("%c",&dropYesNo, sizeof(dropYesNo));

        printf("Enter filter ID of parent. Otherwise 0:: \n");
        fflush(stdin);
        scanf_s("%u", &fatherId, sizeof(fatherId));

        retVal = RxFiltersExample('1', selection, filterData,
                         equalYesNo, dropYesNo, (_i8)fatherId);
        if((retVal < 0) && (retVal != INVALID_PARENT_FILTER_ID))
        {
            return retVal;
        }

        printf("\nPlease select a filter parameter:\n");
        printf("\n1. Source MAC address\n");
        printf("2. Destination MAC address\n");
        printf("3. BSSID\n");
        printf("4. Frame type\n");
        printf("5. Frame subtype\n");
        printf("6. Source IP address\n");
        printf("7. Destination IP address\n");
        printf("8. Packet length\n");
        printf("9. Remove and exit\n");
        printf("10. Enable and exit\n");
        printf("Selection:\n");
    }

    return SUCCESS;
}

/*!
    \brief This function configures the filter based on user input

    \param[in]   none

    \return         0 on sucess, -ve otherwise

    \note

    \warning
*/
static _i32 ChooseFilters()
{
    _i32 retVal = -1;
    _i8  ch = -1;

    printf("Enter 'f' to configure MAC filters or 'q' to exit: \n");
    fflush(stdin);

    ch = getchar();

    if(ch == 'f' || ch == 'F')
    {
        retVal = FiltersMenu();
        ASSERT_ON_ERROR(retVal);
    }
    else if(ch == 'q' || ch == 'Q')
    {
        g_Exit = 1;
    }

    Sleep(500);

    return SUCCESS;
}

/*!
    \brief This function display the frame subtype of the received packet.

    \param[in]   none

    \return         None

    \note

    \warning
*/
static void PrintFrameSubtype(_u8 MAC)
{
    printf("Frame Subtype:\n");
    switch (MAC)
    {
        case 0x8:
            printf("Data (%02x)\n",MAC);
            break;

        case 0x40:
            printf("Probe Request (%02x)\n",MAC);
            break;

        case 0x50:
            printf("Probe Response (%02x)\n",MAC);
            break;

        case 0x80:
            printf("Beacon (%02x)\n",MAC);
            break;

        case 0x88:
            printf("QOS Data (%02x)\n",MAC);
            break;

        case 0xd4:
            printf("Acknowledgement (%02x)\n",MAC);
            break;

        case 0x0b:
            printf("Authentication (%02x)\n",MAC);
            break;

        case 0x1c:
            printf("Clear to Send (%02x)\n",MAC);
            break;

        case 0x1b:
            printf("Request to Send (%02x)\n",MAC);
            break;

        case 0x09:
            printf("ATIM (%02x)\n",MAC);
            break;

        case 0x19:
            printf("802.11 Block Acknowledgement (%02x)\n",MAC);
            break;

        default:
            printf("Unknown (%02x)\n",MAC);
            break;
    }
}

/*!
    \brief This function opens a raw socket, receives the frames and display them.

    \param[in]   channel    : channel for the raw socket
    \param[in]   numpackets : number of packets to be received

    \return      0 for success, -ve otherwise

    \note

    \warning
*/
static _i32 Sniffer(_i32 channel,_i32 numpackets)
{
    TransceiverRxOverHead_t *frameRadioHeader = 0;
    _u8 MAC[MAX_RECV_BUF_SIZE] = {'\0'};
    _u8 hexempty = 0xcc;

    _i32 retVal = -1;
    _i16 sd = -1;

    /********************* Open Socket for transceiver   *********************/
    sd = sl_Socket(SL_AF_RF,SL_SOCK_RAW,(_i16)channel);
    ASSERT_ON_ERROR(sd);

    /************************************ Creating filters *****************/
    retVal = ChooseFilters();
    ASSERT_ON_ERROR(retVal);

    /************ Receiving frames from the CC3100 and printing to screen*****/
    if (!g_Exit)
    {
        printf("\nCollecting Packets...\n");

        while(numpackets > 0)
        {
            retVal = sl_Recv(sd,buffer,MAX_RECV_BUF_SIZE,0);
            ASSERT_ON_ERROR(retVal);

            frameRadioHeader = (TransceiverRxOverHead_t *)buffer;
            printf("\nTimestamp: %i microsec\n",frameRadioHeader->timestamp);
            printf("Signal Strength: %i dB\n",frameRadioHeader->rssi);

            memcpy(MAC, buffer, sizeof(buffer));

            PrintFrameSubtype(MAC[8]);

            printf("Destination MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            MAC[12],MAC[13], MAC[14],MAC[15], MAC[16],MAC[17]);

            printf("Source MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            MAC[18],MAC[19], MAC[20],MAC[21], MAC[22],MAC[23]);
            
            numpackets--;
            Sleep(500);
        }
    }

    retVal = sl_Close(sd);
    ASSERT_ON_ERROR(retVal);

    return SUCCESS;
}
