/*
 * main.c - sample application for AP provisioning
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
 * Application Name     -   Provisioning AP
 * Application Overview -   This is a sample application demonstrating how CC3100's
 *                          internal web-server can be used to associate the device
 *                          with an access-point.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_Provisioning_AP
 *                          doc\examples\provisioning_ap.pdf
 */

#include "simplelink.h"
#include "protocol.h"
#include "sl_common.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

#define NO_AP        "__No_AP_Available__"

#define SCAN_TABLE_SIZE  20
#define SCAN_INTERVAL    10
#define MAX_KEY_LENGTH   32

#define GET_TOKEN    "__SL_G_U"

#define GET_STATUS   "__SL_G_S.0"

/* Use bit 32: Lower bits of status variable are used for NWP events
 *
 *      1 in a 'status_variable', profile is added form web page
 *      0 in a 'status_variable', profile is not added form web page
 */
#define STATUS_BIT_PROFILE_ADDED 31
/* Use bit 31
 *      1 in a 'status_variable', successfully connected to AP
 *      0 in a 'status_variable', AP connection was not successful
 */
#define STATUS_BIT_CONNECTED_TO_CONF_AP (STATUS_BIT_PROFILE_ADDED - 1)
/* Use bit 30
 *      1 in a 'status_variable', AP provisioning process completed
 *      0 in a 'status_variable', AP provisioning process yet to complete
 */
#define STATUS_BIT_PROVISIONING_DONE (STATUS_BIT_CONNECTED_TO_CONF_AP - 1)

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    PROVISIONING_AP_FAILED      = DEVICE_NOT_IN_STATION_MODE -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

#define IS_PROFILE_ADDED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_PROFILE_ADDED)
#define IS_CONNECTED_TO_CONF_AP(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTED_TO_CONF_AP)
#define IS_PROVISIONING_DONE(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_PROVISIONING_DONE)

/*
 * GLOBAL VARIABLES -- Start
 */
_u32    g_Status = 0;
_u32   g_ConnectTimeoutCnt = 0;

_i8 g_Wlan_SSID[MAXIMAL_SSID_LENGTH + 1];
_i8 g_Wlan_Security_Key[MAX_KEY_LENGTH];

Sl_WlanNetworkEntry_t g_netEntries[SCAN_TABLE_SIZE];
_u8 g_ssid_list[SCAN_TABLE_SIZE][MAXIMAL_SSID_LENGTH + 1];

SlSecParams_t g_secParams;
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState();
static _i32 establishConnectionWithAP();
static _i32 initializeAppVariables();
static _i32 ProvisioningAP();
static void FilterList();
static void displayBanner();
static _i32 set_authentication_check (_u8);
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
        CLI_Write(" [WLAN EVENT] NULL Pointer Error \n\r");
    
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

            /* If the user has initiated 'Disconnect' request, 'reason_code' is
             * SL_USER_INITIATED_DISCONNECTION */
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                /* Device disconnected from the AP on application's request */
            }
            else
            {
                CLI_Write(" Device disconnected from the AP on an ERROR..!! \n\r");
            }
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
            /*
             * Information about the connected STA (like name, MAC etc) will be
             * available in 'slPeerInfoAsyncResponse_t' - Applications
             * can use it if required
             *
             * slPeerInfoAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.APModeStaConnected;
             *
             */
            break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
            /*
             * Information about the connected STA (device name, MAC) will be
             * available in 'slPeerInfoAsyncResponse_t' - Applications
             * can use it if required
             *
             * slPeerInfoAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.APModeStaConnected;
             *
             */
            break;

        default:
        {
            CLI_Write(" [WLAN EVENT] Unexpected event \n\r");
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
        CLI_Write(" [NETAPP EVENT] NULL Pointer Error \n\r");
 
    if(pNetAppEvent == NULL)
        CLI_Write(" [NETAPP EVENT] NULL Pointer Error \n\r");
 
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

            /*
             * Information about the connection (like IP, gateway address etc)
             * will be available in 'SlIpV4AcquiredAsync_t'
             * Applications can use it if required
             *
             * SlIpV4AcquiredAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
             *
             */
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
            /*
             * Information about the connection (like leased IP, lease time etc)
             * will be available in 'SlIpLeasedAsync_t'
             * Applications can use it if required
             *
             * SlIpLeasedAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipLeased;
             *
             */
            break;

        default:
        {
            CLI_Write(" [NETAPP EVENT] Unexpected event \n\r");
        }
        break;
    }
}

/*!
    \brief This function handles callback for the HTTP server events

    \param[in]      pEvent - Contains the relevant event information
    \param[in]      pResponse - Should be filled by the user with the
                    relevant response information

    \return         None

    \note

    \warning
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pEvent,
                                  SlHttpServerResponse_t *pResponse)
{
    if(pEvent == NULL || pResponse == NULL)
        CLI_Write(" [HTTP EVENT] NULL Pointer Error \n\r");

    switch (pEvent->Event)
    {
        case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:
        {
            /* HTTP get token */
            _u8 *ptr;
            _u8 num = 0;

            if (0== pal_Memcmp (pEvent->EventData.httpTokenName.data,
                    GET_STATUS,pEvent->EventData.httpTokenName.len))
            {
                /* Token to get the connection status */
                if(IS_CONNECTED_TO_CONF_AP(g_Status))
                {
                    pal_Strcpy (pResponse->ResponseData.token_value.data,
                            "TRUE");
                    pResponse->ResponseData.token_value.len = pal_Strlen("TRUE");
                }
                else
                {
                    pal_Strcpy (pResponse->ResponseData.token_value.data,
                            "FALSE");
                    pResponse->ResponseData.token_value.len = pal_Strlen("FALSE");
                }
            }
            else
            {
                /* Token to get the AP list */
                ptr = pResponse->ResponseData.token_value.data;
                pResponse->ResponseData.token_value.len = 0;
                if(pal_Memcmp(pEvent->EventData.httpTokenName.data, GET_TOKEN,pal_Strlen(GET_TOKEN)) == 0)
                {
                    num = pEvent->EventData.httpTokenName.data[8] - '0';
                    num *= 10;
                    num += pEvent->EventData.httpTokenName.data[9] - '0';

                    if(g_ssid_list[num][0] == '\0')
                    {
                        pal_Strcpy(ptr, NO_AP);
                    }
                    else
                    {
                        pal_Strcpy(ptr, g_ssid_list[num]);
                    }
                    pResponse->ResponseData.token_value.len = pal_Strlen(ptr);
                }
            }
            break;
        }

        case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
        {
            /* HTTP post token */

            if ((0 == pal_Memcmp(pEvent->EventData.httpPostData.token_name.data, "__SL_P_U.C", pEvent->EventData.httpPostData.token_name.len)) &&
                (0 == pal_Memcmp(pEvent->EventData.httpPostData.token_value.data, "Connect", pEvent->EventData.httpPostData.token_value.len)))
            {
                SET_STATUS_BIT(g_Status, STATUS_BIT_PROFILE_ADDED);
            }
            if (0 == pal_Memcmp (pEvent->EventData.httpPostData.token_name.data, "__SL_P_U.D", pEvent->EventData.httpPostData.token_name.len))
            {
                pal_Memcpy (g_Wlan_SSID,  pEvent->EventData.httpPostData.token_value.data, pEvent->EventData.httpPostData.token_value.len);
                g_Wlan_SSID[pEvent->EventData.httpPostData.token_value.len] = 0;
            }
            if (0 == pal_Memcmp (pEvent->EventData.httpPostData.token_name.data, "__SL_P_U.E", pEvent->EventData.httpPostData.token_name.len))
            {
                if (pEvent->EventData.httpPostData.token_value.data[0] == '0')
                {
                    g_secParams.Type =  SL_SEC_TYPE_OPEN;

                }
                else if (pEvent->EventData.httpPostData.token_value.data[0] == '1')
                {
                    g_secParams.Type =  SL_SEC_TYPE_WEP;

                }
                else if (pEvent->EventData.httpPostData.token_value.data[0] == '2')
                {
                    g_secParams.Type =  SL_SEC_TYPE_WPA;
                }
                else if (pEvent->EventData.httpPostData.token_value.data[0] == '3')
                {
                    g_secParams.Type =  SL_SEC_TYPE_WPA;
                }
            }
            if (0 == pal_Memcmp (pEvent->EventData.httpPostData.token_name.data, "__SL_P_U.F", pEvent->EventData.httpPostData.token_name.len))
            {
                pal_Memcpy (g_Wlan_Security_Key,pEvent->EventData.httpPostData.token_value.data, pEvent->EventData.httpPostData.token_value.len);
                g_Wlan_Security_Key[pEvent->EventData.httpPostData.token_value.len] = 0;
                g_secParams.Key = g_Wlan_Security_Key;
                g_secParams.KeyLen = pEvent->EventData.httpPostData.token_value.len;
            }
            if (0 == pal_Memcmp (pEvent->EventData.httpPostData.token_name.data, "__SL_P_U.0", pEvent->EventData.httpPostData.token_name.len))
            {
                SET_STATUS_BIT(g_Status, STATUS_BIT_PROVISIONING_DONE);
            }
            break;
        }
        default:
            break;
    }
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
    CLI_Write(" [GENERAL EVENT] \n\r");
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler

    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    /*
     * This application doesn't work w/ socket - Hence not handling these events
     */
    CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
}
/*
 * ASYNCHRONOUS EVENT HANDLERS -- End
 */

/*
 * Application's entry point
 */
int main(int argc, char** argv)
{
    _i32 retVal = -1;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    /* Stop WDT and initialize the system-clock of the MCU
       These functions needs to be implemented in PAL */
    stopWDT();
    initClk();

    /* Configure command line interface */
    CLI_Configure();

    displayBanner();

    /*
     *  Following function configures the device to default state by cleaning
     * the persistent settings stored in NVMEM (viz. connection profiles &
     * policies, power policy etc)
     *
     * Applications may choose to skip this step if the developer is sure
     * that the device is in its default state at start of application
     *
     * Note that all profiles and persistent settings that were done on the
     * device will be lost
     */
    retVal = configureSimpleLinkToDefaultState();
    if(retVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == retVal)
        {
            CLI_Write(" Failed to configure the device in its default state \n\r");
        }

        LOOP_FOREVER();
    }

    CLI_Write(" Device is configured in default state \n\r");

    /* Switch to AP mode and wait for provisioning to complete */
    retVal = ProvisioningAP();
    if(retVal < 0)
    {
        CLI_Write(" Provisioning using AP mode failed \n\r");
    }
    else
    {
        CLI_Write(" Connection established w/ AP and IP is acquired \n\r");
    }

    /* Stop the CC3100 device */
    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if(retVal < 0)
        LOOP_FOREVER();

    return 0;
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
           - Unregisters mDNS services
           - Remove all filters

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/
static _i32 configureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    _u8           val = 1;
    _u8           configOpt = 0;
    _u8           configLen = 0;
    _u8           power = 0;

    _i32          retVal = -1;
    _i32          mode = -1;

    mode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in staion-mode */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while(!IS_IP_ACQUIRED(g_Status)) { _SlNonOsMainLoopTask(); }
        }

        /* Switch to STA role and restart */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Start(0, 0, 0);
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
        while(IS_CONNECTED(g_Status)) { _SlNonOsMainLoopTask(); }
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
    pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
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
    \brief This function implements the ProvisioningAP functionality

    This function
        1. Starts Device in STA Mode
        2. Scans and Stores all the AP
        3. Switch to AP Mode and Wait for AP Configuration from Browser
        4. Switch to STA Mode and Connect to Configured AP

    \param          None

    \return         None
*/
static _i32 ProvisioningAP()
{
    _u32 intervalInSeconds = SCAN_INTERVAL;
    _i32 retVal = -1;
    _u8 SecType = 0;

    CLR_STATUS_BIT(g_Status, STATUS_BIT_PROFILE_ADDED);


    /* Initializing the CC3100 device */
    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        CLI_Write(" Failed to start the device \n\r");
        ASSERT_ON_ERROR(PROVISIONING_AP_FAILED);
    }

    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID,
            pal_Strlen(SSID_AP_MODE), (_u8 *)SSID_AP_MODE);
    ASSERT_ON_ERROR(retVal);

    SecType = SEC_TYPE_AP_MODE;
    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SECURITY_TYPE, 1,
            (_u8 *)&SecType);
    ASSERT_ON_ERROR(retVal);

    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_PASSWORD, pal_Strlen(PASSWORD_AP_MODE),
            (_u8 *)PASSWORD_AP_MODE);
    ASSERT_ON_ERROR(retVal);

    /* Disables the HTTP Authentication */
    retVal = set_authentication_check(FALSE);
    if(retVal < 0)
        LOOP_FOREVER();

    /* Delete all stored profiles */
    retVal  = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retVal);

    /* Stop the CC3100 device */
    retVal = sl_Stop(SL_STOP_TIMEOUT);

    CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
    CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

    /* Initializing the CC3100 device */
    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        CLI_Write(" Failed to start the device \n\r");
        ASSERT_ON_ERROR(PROVISIONING_AP_FAILED);
    }

    CLI_Write(" Configuring device in AP mode - SSID: ");
    CLI_Write(SSID_AP_MODE);
    CLI_Write("\n\r");

    while(!(IS_PROVISIONING_DONE(g_Status)))
    {

        /* Set the scan policy */
        retVal = sl_WlanPolicySet(SL_POLICY_SCAN,SL_SCAN_POLICY_EN(1),
                (_u8 *)&intervalInSeconds,sizeof(intervalInSeconds));
        ASSERT_ON_ERROR(retVal);

        /* wait for 250 ms to make sure scan has started */
        Delay(250);

        /* Get the scan results */
        sl_WlanGetNetworkList(0,20,&g_netEntries[0]);

        /* Remove duplicate entries */
        FilterList();

        /* Switch to AP Mode */
        retVal = sl_WlanSetMode(ROLE_AP);
        ASSERT_ON_ERROR(retVal);

        /* Stop the CC3100 device */
        retVal = sl_Stop(SL_STOP_TIMEOUT);

        CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

        /* Initializing the CC3100 device */
        retVal = sl_Start(0, 0, 0);
        if ((retVal < 0) ||
            (ROLE_AP != retVal) )
        {
            CLI_Write(" Failed to start the device in AP mode \n\r");
            ASSERT_ON_ERROR(PROVISIONING_AP_FAILED);
        }

        /* Wait for CC3100 to Acquired IP in AP Mode */
        while (!IS_IP_ACQUIRED(g_Status))
        {
            _SlNonOsMainLoopTask();
        }

        /* Wait for AP Configuration */
        while (!IS_PROFILE_ADDED(g_Status) && !(IS_PROVISIONING_DONE(g_Status)))
        {
            _SlNonOsMainLoopTask();
        }

        CLR_STATUS_BIT(g_Status, STATUS_BIT_PROFILE_ADDED);

        /* Switch to STA Mode */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        /* AP Configured, Restart in STA Mode */
        retVal = sl_Stop(SL_STOP_TIMEOUT);

        CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
        CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

        retVal = sl_Start(0, 0, 0);
        if ((retVal < 0) ||
            (ROLE_STA != retVal) )
        {
            CLI_Write(" Failed to start the device in STA mode \n\r");
            ASSERT_ON_ERROR(PROVISIONING_AP_FAILED);
        }

        /* Tries to connect to configured AP */
        retVal = establishConnectionWithAP();
        if(retVal < 0)
        {
            CLI_Write(" Failed to establish connection w/ AP \n\r");
            ASSERT_ON_ERROR(retVal);
        }

        if(IS_CONNECTED(g_Status))
            SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTED_TO_CONF_AP);
        else
            CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTED_TO_CONF_AP);

        if (!IS_PROVISIONING_DONE(g_Status))
        {
            retVal = sl_WlanDisconnect();
            if(0 == retVal)
            {
                /* Wait */
                while(IS_CONNECTED(g_Status)) { _SlNonOsMainLoopTask(); }
            }
        }
    }

    return SUCCESS;
}

/*!
    \brief Connecting to a WLAN Access point

    This function tries to connect to configured AP..
    The function will return once we are connected or the command is timed out

    \param          None

    \return         None

    \note
*/
static _i32 establishConnectionWithAP()
{
    _i32 retVal = 0;

    retVal = sl_WlanConnect(g_Wlan_SSID,pal_Strlen(g_Wlan_SSID),0,&g_secParams,0);
    ASSERT_ON_ERROR(retVal);

    /* pending on connection success and DHCP success */
    while (g_ConnectTimeoutCnt < 2000 &&
            ((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status))))
    {
        _SlNonOsMainLoopTask();
        Delay(1);
        g_ConnectTimeoutCnt++;
    }
    g_ConnectTimeoutCnt = 0;

    return SUCCESS;
}

/*!
    \brief Filter scanned AP list

    This function filters the list the scanned AP and remove the duplicate
    entries.

    \param          None

    \return         None

    \note
*/
static void FilterList()
{
    _u8 idx1 = 0;
    _u8 idx2 = 0;

    pal_Memset(g_ssid_list, '\0', (SCAN_TABLE_SIZE * (MAXIMAL_SSID_LENGTH + 1)));

    for(idx1=0; idx1 < SCAN_TABLE_SIZE; idx1++)
    {
        if(g_netEntries[idx1].ssid[0] != '\0')
        {
            for(idx2=0; idx2 < SCAN_TABLE_SIZE; idx2++)
            {
                if(pal_Strcmp(g_netEntries[idx1].ssid,
                        g_ssid_list[idx2]) == 0)
                    break;
                if(g_ssid_list[idx2][0] == '\0')
                {
                    pal_Strcpy(g_ssid_list[idx2],
                            g_netEntries[idx1].ssid);
                    break;
                }
            }
        }
    }
}

/*!
    \brief Enable/Disable the authentication check for http server,
           By default authentication is disabled.

    \param[in]      enable - false to disable and true to enable the authentication

    \return         None

    \note

    \warning
*/
static _i32 set_authentication_check (_u8 enable)
{
    _NetAppHttpServerGetSet_auth_enable_t auth_enable;
    _i32 status = -1;

    auth_enable.auth_enable = enable;
    status = sl_NetAppSet(SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_AUTH_CHECK,
                 sizeof(_NetAppHttpServerGetSet_auth_enable_t), (_u8 *)&auth_enable);
    ASSERT_ON_ERROR(status);

    return SUCCESS;
}

/*!
    \brief This function initializes the application variables

    \param[in]  None

    \return     0 on success, negative error-code on error
*/
static _i32 initializeAppVariables()
{
    g_Status = 0;
    g_ConnectTimeoutCnt = 0;
    pal_Memset(g_Wlan_SSID, 0, (MAXIMAL_SSID_LENGTH + 1));
    pal_Memset(g_Wlan_Security_Key, 0, MAX_KEY_LENGTH);
    pal_Memset(g_netEntries, 0, sizeof(g_netEntries));
    pal_Memset(g_ssid_list, 0, sizeof(g_ssid_list));
    pal_Memset(&g_secParams, 0, sizeof(SlSecParams_t));

    return SUCCESS;
}

/*!
    \brief This function displays the application's banner

    \param      None

    \return     None
*/
static void displayBanner()
{
    CLI_Write("\n\r\n\r");
    CLI_Write(" Provisioning AP application - Version ");
    CLI_Write(APPLICATION_VERSION);
    CLI_Write("\n\r*******************************************************************************\n\r");
}
