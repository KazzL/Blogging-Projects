/*
 * main.c - sample application to switch to AP mode and ping client
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
 * Application Name     -   Getting started with Wi-Fi Access Point mode
 *
 * Application Overview -   This sample application demonstrates how to configure 
 *                          CC3100 in Access-Point mode. Any WLAN station in its 
 *                          range can connect/communicate to/with it as per the 
 *                          standard networking protocols. On a successful connection 
 *                          of client, the device ping's the connected station.
 *
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Getting_started_with_WLAN_AP
 *                          doc\examples\sls_getting_started_with_wlan_ap.pdf
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
                /* Handling the error-codes is specifc to the application */ \
                if (error_code < 0) return error_code; \
                /* else, continue w/ execution */ \
            }

#define SUCCESS         0
#define MAX_SSID_LEN    32
#define MAX_PASSKEY_LEN 32
#define BAUD_RATE       115200

#define PING_INTERVAL        1000
#define PING_SIZE            20
#define PING_REQUEST_TIMEOUT 3000
#define PING_ATTEMPT         3

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM 21
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */

/* Status bits - These are used to set/reset the corresponding bits in a 'status_variable' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in a 'status_variable', the device is connected to the AP
                                 *      0 in a 'status_variable', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_ACQUIRED,       /* If this bit is:
                                  *      1 in a 'status_variable', the device has acquired an IP
                                  *      0 in a 'status_variable', the device has not acquired an IP
                                  */

    STATUS_BIT_PING_DONE,        /* If this bit is:
                                  *      1 in a 'status_variable', the device has completed the ping operation
                                  *      0 in a 'status_variable', the device has not completed the ping operation
                                  */

    STATUS_BIT_IP_LEASED,        /* If this bit is:
                                  *      1 in a 'status_variable', the device has a client connected to it
                                  *      and the IP has been leased
                                  *      0 in a 'status_variable', the device has no clients connected to it
                                  */
    STATUS_BIT_STA_CONNECTED     /* If this bit is:
                                  *      1 in a 'status_variable', a station has connected to the device
                                  *      and the IP has been leased
                                  *      0 in a 'status_variable', the station couldn't connect to the device
                                  */

}e_StatusBits;

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))

#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_ACQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_ACQUIRED)
#define IS_PING_DONE(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_PING_DONE)
#define IS_IP_LEASED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_LEASED)
#define IS_STA_CONNECTED(status_variable)       GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_STA_CONNECTED)

typedef struct{
    _u8 SSID[MAX_SSID_LEN];
    _i32 encryption;
    _u8 password[MAX_PASSKEY_LEN];
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */
_u8 g_Status = 0;
_u32 g_PingPacketsRecv = 0;
_u32 sta_IP = 0;
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 initializeAppVariables();
static _i16 GetUserNum();
static _i32 ConfigureAPmode(UserInfo UserParams);
static UserInfo GetUserInput();
static void displayBanner();
/*
 * STATIC FUNCTION DEFINITIONS -- End
 */


/*!
    \brief This function handles WLAN events

    \param[in]      pWlanEvents is the event passed to the handler

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

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_STA_CONNECTED);
        }
        break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
        {
            CLR_STATUS_BIT(g_Status, STATUS_BIT_STA_CONNECTED);
            CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_LEASED);
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
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            sta_IP = pNetAppEvent->EventData.ipLeased.ip_address;
            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_LEASED);
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
    /*
     * This application doesn't work with HTTP server - Hence these
     * events are not handled here
     */
    printf(" [HTTP EVENT] Unexpected event \r\n");
}

/*!
    \brief This function handles ping report events

    \param[in]      pPingReport holds the ping report statistics

    \return         None

    \note

    \warning
*/
void SimpleLinkPingReport(SlPingReport_t *pPingReport)
{
    SET_STATUS_BIT(g_Status, STATUS_BIT_PING_DONE);
    g_PingPacketsRecv = pPingReport->PacketsReceived;
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
 * ASYNCHRONOUS EVENT HANDLERS -- End
 */


/*
 * Application's entry point
 */
int main(void)
{
    SlPingStartCommand_t PingParams;
    SlPingReport_t Report;
    UserInfo User;

    _u32 IpAddr = 0;
    _u32 Mask = 0;
    _u32 Gw = 0;
    _u32 Dns = 0;
    _u8  IsDhcp = 0;

    _i32 Status = -1;
    _i32 mode = ROLE_STA;
    _i8  *pConfig = NULL;
    _i32 retVal = -1;

#ifdef SL_IF_TYPE_UART
    params.BaudRate = BAUD_RATE;
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
     * Asumption is that the device is configured in station mode already
     * and it is in its default state
     */

    /* Initializing the CC3100 device */
    mode = sl_Start(0, pConfig, 0);
    if (ROLE_AP == mode)
    {
        /* If some other application has configured the device in AP mode,
           then we need to wait for this event */
        while(!IS_IP_ACQUIRED(g_Status))
            ;
    }
    else
    {
        /* Configure CC3100 to start in AP mode */
        retVal = sl_WlanSetMode(ROLE_AP);
        if(retVal < 0)
            LOOP_FOREVER();

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        if(retVal < 0)
            LOOP_FOREVER();
        mode = sl_Start(0, pConfig, 0);
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while(!IS_IP_ACQUIRED(g_Status)) 
                ;
        }
        else
        {
            printf(" Device couldn't be configured in AP mode \n\r");
            LOOP_FOREVER();
        }
    }

    User = GetUserInput();

    retVal = ConfigureAPmode(User);
    if(retVal < 0)
    {
        printf(" Failed to set AP mode configuration, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }
    printf("Configured CC3100 in AP mode, Restarting CC3100 in AP mode\n");

    /* Restart the CC3100 */
    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if(retVal < 0)
        LOOP_FOREVER();

    g_Status = 0;

    mode  = sl_Start(0, pConfig, 0);
    if (ROLE_AP == mode)
    {
        /* If the device is in AP mode, we need to wait for this event before doing anything */
        while(!IS_IP_ACQUIRED(g_Status)) 
            ;
    }
    else
    {
        printf(" Device couldn't come-up in AP mode \r\n");
        LOOP_FOREVER();
    }

    printf("Connect client to AP %s\n",User.SSID);

    while((!IS_IP_LEASED(g_Status)) || (!IS_STA_CONNECTED(g_Status)))
        ;

    printf("Client connected \n");

     /* Set the ping parameters */
    PingParams.PingIntervalTime = PING_INTERVAL;
    PingParams.PingSize = PING_SIZE;
    PingParams.PingRequestTimeout = PING_REQUEST_TIMEOUT;
    PingParams.TotalNumberOfAttempts = PING_ATTEMPT;
    PingParams.Flags = 0;
    PingParams.Ip = sta_IP; /* Fill the station IP address connected to CC3100 */

    /* Ping client connected to CC3100 */
    printf("Start pinging to client\n");
    retVal = sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                       (SlPingReport_t*)&Report, SimpleLinkPingReport);
    if(retVal < 0)
        LOOP_FOREVER();

    while(!IS_PING_DONE(g_Status))
        ;

    if (g_PingPacketsRecv)
    {
        /* Ping successful */
        printf("Ping successful\n");
        system("PAUSE");
        return 0;
    }
    else
    {
        /* Problem with Pinging to client */
        printf("Problem with pinging to client\n");
        system("PAUSE");
        LOOP_FOREVER();
    }

    return 0;
}

/*!
    \brief This function puts the device in its default state. It:
           - Set the mode to STATION
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
    g_PingPacketsRecv = 0;
    sta_IP = 0;

    return SUCCESS;
}

/*!
    \brief Read option from the user

    \param[in]      none

    \return         _i16 - user option value

    \note

    \warning
*/
static _i16 GetUserNum()
{
    _i32   flag = 0;
    _i8    input[20] = {'\0'};
    _i16   value = -1;

    while (!flag)
    {
        if(scanf_s("%s",input,sizeof(input)) != 0)
        {
            value = atoi((const char *)input);
            if (value > 0 && value < 5 )
            {
                flag = 1;
            }
            else
            {
                printf("Invalid entry. Please try again:\n");
            }
        }
    }

    return value;
}

/*!
    \brief Get the AP parameters from the user

    \param[in]      none

    \return         UserInfo - structure containing SSID, encryption type and
                    pass key of AP

    \note

    \warning
*/
static UserInfo GetUserInput()
{
    UserInfo UserFunction;
    _i32   eflag = -1;
    _i32   wepflag = -1;
    _i32   length = -1;

    printf("Please input the SSID name for AP mode: \n");
    scanf_s("%[^\n]s",UserFunction.SSID,MAX_SSID_LEN);

    printf("Encryption Types for AP mode:\n");
    printf("1:  Open\n");
    printf("2:  WEP\n");
    printf("3:  WPA\n");
    printf("Please enter the corresponding number for the encryption type: \n");
    UserFunction.encryption = GetUserNum();

    if (UserFunction.encryption != 1)
    {
        printf ("Please enter the password for AP mode: \n");
        scanf_s("%s",UserFunction.password,MAX_PASSKEY_LEN);
    }

    return UserFunction;
}

/*!
    \brief Configures the device in AP mode

    \param[in]  UserParams - Structure having SSID, security type and pass key
                             of AP mode

    \return         none

*/
static _i32 ConfigureAPmode(UserInfo UserParams)
{
    _i32 retVal = -1;
    _u8 val = SL_SEC_TYPE_OPEN;

    /* Configure the SSID of the CC3100 */
    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID,
               strlen((const char *)UserParams.SSID),
               UserParams.SSID);
    ASSERT_ON_ERROR(retVal);

    /* Configure the Security parameter in the AP mode */
    switch(UserParams.encryption)
    {
        case 1:
        {
            val = SL_SEC_TYPE_OPEN;
            retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID,
                       WLAN_AP_OPT_SECURITY_TYPE,
                       1, &val);
            ASSERT_ON_ERROR(retVal);
        }
        break;

        case 2:
        {
            val = SL_SEC_TYPE_WEP;
            retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID,
                       WLAN_AP_OPT_SECURITY_TYPE,
                       1, &val);
            ASSERT_ON_ERROR(retVal);

            retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID,
                       WLAN_AP_OPT_PASSWORD,
                       strlen((const char *)UserParams.password),
                       UserParams.password);
            ASSERT_ON_ERROR(retVal);
        }
        break;

        case 3:
        {
            val = SL_SEC_TYPE_WPA;
            retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID,
                       WLAN_AP_OPT_SECURITY_TYPE,
                       1, &val);
            ASSERT_ON_ERROR(retVal);

            retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID,
                       WLAN_AP_OPT_PASSWORD,
                       strlen((const char *)UserParams.password),
                       UserParams.password);
            ASSERT_ON_ERROR(retVal);
        }
        break;
    }

    return SUCCESS;
}

/*!
    \brief This function displays the application's banner

    \param      None

    \return     None
*/
static void displayBanner()
{
    printf("\r\n*****************************************************************\r\n");
    printf("Getting started with AP  App \n");
    printf(APPLICATION_VERSION);
    printf("\r\n*****************************************************************\r\n");
}
