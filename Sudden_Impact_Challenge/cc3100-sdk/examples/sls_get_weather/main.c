/*
 * main.c - get weather details sample application
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
 * Application Name     -   Get weather
 * Application Overview -   This is a sample application demonstrating how to
                            connect to openweathermap.org server and request for
                            weather details of a city.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Get_Weather_Application
 *                          doc\examples\sls_get_weather.pdf
 */

#include <string.h>
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
                if (error_code < 0) \
                {\
                   printf(" Error Code %d\n", error_code);\
                   return error_code; \
                }\
                /* else, continue w/ execution */ \
            }

#define WEATHER_SERVER      "openweathermap.org"

#define BAUD_RATE           115200
#define MAX_RECV_BUFF_SIZE  1024
#define MAX_SEND_BUFF_SIZE  512
#define MAX_CITY_NAME_SIZE  64
#define MAX_HOSTNAME_SIZE   40
#define MAX_PASSKEY_SIZE    32
#define MAX_SSID_SIZE       32

#define PREFIX_BUFFER       "GET /data/2.5/weather?q="
#define POST_BUFFER         "&mode=xml&units=imperial HTTP/1.1\r\nHost:api.openweathermap.org\r\nAccept: */"
#define POST_BUFFER2        "*\r\n\r\n"

#define SUCCESS             0

#define CONNECTION_STATUS_BIT   0
#define IP_AQUIRED_STATUS_BIT   1

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM 21
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,/* Choosing this number to avoid overlap w/ host-driver's error codes */
    HTTP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    HTTP_RECV_ERROR = HTTP_SEND_ERROR - 1,
    HTTP_INVALID_RESPONSE = HTTP_RECV_ERROR -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


/* Status bits - These are used to set/reset the corresponding bits in 'g_Status' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in 'g_Status', the device is connected to the AP
                                 *      0 in 'g_Status', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_ACQUIRED,       /* If this bit is:
                                 *      1 in 'g_Status', the device has acquired an IP
                                 *      0 in 'g_Status', the device has not acquired an IP
                                 */

}e_StatusBits;


#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))

#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_ACQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_ACQUIRED)

typedef struct{
    _i8 SSID[MAX_SSID_SIZE];
    _i32 encryption;
    _i8 password[MAX_PASSKEY_SIZE];
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */
struct
{
    _u8 Recvbuff[MAX_RECV_BUFF_SIZE];
    _u8 SendBuff[MAX_SEND_BUFF_SIZE];

    _i8 HostName[MAX_HOSTNAME_SIZE];
    _u8 CityName[MAX_CITY_NAME_SIZE];

    _u32 DestinationIP;
    _i16 SockID;
}appData;

_u32  g_Status = 0;
/*
 * GLOBAL VARIABLES -- End
 */


 /*
 * STATIC FUNCTION DEFINITIONS  -- Start
 */

static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 establishConnectionWithAP(UserInfo);
static _i32 initializeAppVariables();
static void displayBanner();
static _i16 GetUserNum();
static UserInfo GetUserInput();
static _i32 GetData();
static _i32 CreateConnection();
static _i32 getWeather();
static _i32 GetHostIP();
/*
 * STATIC FUNCTION DEFINITIONS -- End
 */


/*
 * * ASYNCHRONOUS EVENT HANDLERS -- Start
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
    /*
     * This application doesn't work with HTTP server - Hence these
     * events are not handled here
     */
    printf(" [HTTP EVENT] Unexpected event \r\n");
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
                    printf(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\r\n");
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
int main(void)
{
    UserInfo User = {0};
    _i32 retVal = 0;
    _i8 *pConfig = NULL;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

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
    printf("Device started as STATION \r\n");

    /* Read the user input */
    User = GetUserInput();

    retVal = establishConnectionWithAP(User);
    if(retVal < 0)
    {
        printf(" Failed to establish connection with AP, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    printf("Connection established w/ AP and IP is acquired \n\r");

    /* Get weather report */
    retVal = getWeather();
    if(retVal < 0)
        LOOP_FOREVER();

    return retVal;
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
    memset(&appData, 0, sizeof(appData));

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
    printf("CC3100 Get weather App \n");
    printf(APPLICATION_VERSION);
    printf("\r\n*****************************************************************\r\n");
}

/*!
    \brief Read option fron the user

    \param[in]      none

    \return         int - user option value

    \note

    \warning
*/
static _i16 GetUserNum()
{
    _u8 input[10] = {'\0'};
    _u8 flag = 0;
    _i16 value = -1;

    while (!flag)
    {
        if (scanf_s("%s",input,sizeof(input)) != 0)
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
    _i32 eflag = -1;
    _i32 wepflag = -1;
    _i32 length = -1;

    printf("Please input the SSID you wish to connect to: \n");
    scanf_s("%[^\n]s",UserFunction.SSID,MAX_SSID_SIZE);

    printf("Encryption Types:\n");
    printf("1:  Open\n");
    printf("2:  WEP\n");
    printf("3:  WPA\n");
    printf("Please enter the corresponding number for the encryption type: \n");
    UserFunction.encryption = GetUserNum();

    if (2 == UserFunction.encryption || /* WEP */
        3 == UserFunction.encryption)   /* WAP */
    {
        printf("Please enter the password: \n");
        scanf_s("%s",UserFunction.password,MAX_PASSKEY_SIZE);
    }

    return UserFunction;
}

/*!
    \brief This function Obtains the required data from the server

    \param[in]      none

    \return         None

    \note

    \warning
*/
static _i32 GetData()
{
    _u8 *p_startPtr = NULL;
    _u8 *p_endPtr = NULL;
    _u8 *p_bufLocation = NULL;
    _i32 retVal = -1;

    memset(appData.Recvbuff, 0, sizeof(appData.Recvbuff));

    /* Puts together the HTTP GET string. */
    p_bufLocation = appData.SendBuff;
    memcpy(p_bufLocation, PREFIX_BUFFER,strlen(PREFIX_BUFFER));

    p_bufLocation += strlen(PREFIX_BUFFER);
    memcpy(p_bufLocation , appData.CityName,strlen((const char *)appData.CityName));

    p_bufLocation += strlen((const char *)appData.CityName);
    memcpy(p_bufLocation, POST_BUFFER,strlen(POST_BUFFER));

    p_bufLocation += strlen(POST_BUFFER);
    memcpy(p_bufLocation, POST_BUFFER2,strlen(POST_BUFFER2));

    /* Send the HTTP GET string to the open TCP/IP socket. */
    retVal = sl_Send(appData.SockID, appData.SendBuff, strlen((const char *)appData.SendBuff), 0);
    if(retVal != strlen((const char *)appData.SendBuff))
        ASSERT_ON_ERROR(HTTP_SEND_ERROR);

    /* Receive response */
    retVal = sl_Recv(appData.SockID, &appData.Recvbuff[0], MAX_RECV_BUFF_SIZE, 0);
    if(retVal <= 0)
        ASSERT_ON_ERROR(HTTP_RECV_ERROR);

    appData.Recvbuff[strlen((const char *)appData.Recvbuff)] = '\0';

    /*Get ticker name*/
    p_startPtr =(_u8 *) strstr((const char *)appData.Recvbuff, "name=");
    if( NULL != p_startPtr )
    {
        p_startPtr = p_startPtr + strlen("name=") + 1;
        p_endPtr = (_u8 *)strstr((const char *)p_startPtr, "\">");
        if( NULL != p_endPtr )
        {
            *p_endPtr = 0;
        }
    }

    printf("\n************************ \r\n");
    printf("City: \n");
    if(p_startPtr == NULL)
    {
        printf("N/A\r\n");
    }
    else
    {
        printf("%s\r\n",p_startPtr);
    }

    if(p_endPtr == NULL)
    {
        printf("Error during parsing the data.\r\n");
        ASSERT_ON_ERROR(HTTP_INVALID_RESPONSE);
    }

    /* Get Temperature Value */
    p_startPtr = (_u8 *)strstr((const char *)p_endPtr+1, "temperature value");
    if( NULL != p_startPtr )
    {
        p_startPtr = p_startPtr + strlen("temperature value") + 2;
        p_endPtr = (_u8 *)strstr((const char *)p_startPtr, "\" ");
        if( NULL != p_endPtr )
        {
            *p_endPtr = 0;
        }
    }

    printf("Temperature: \n");
    if(p_startPtr == NULL)
    {
        printf("N/A\r\n");
    }
    else
    {
        printf("%s F\r\n",p_startPtr);
    }

    /* Get weather */
    p_startPtr = (_u8 *)strstr((const char *)p_endPtr+1, "weather number");
    if( NULL != p_startPtr )
    {
        p_startPtr = p_startPtr + strlen("weather number") + 14;
        p_endPtr = (_u8 *)strstr((const char *)p_startPtr, "\" ");
        if( NULL != p_endPtr )
        {
            *p_endPtr = 0;
        }
    }

    printf("Weather Condition: \n");
    if(p_startPtr == NULL)
    {
        printf("N/A\r\n");
    }
    else
    {
        printf("%s\r\n",p_startPtr);
    }
    printf("\n************************ \r\n");

    return SUCCESS;
}

/*!
    \brief Create TCP connection with openweathermap.org

    \param[in]      none

    \return         Socket descriptor for success otherwise negative

    \warning
*/
static _i32 CreateConnection()
{
    SlSockAddrIn_t  Addr;

    _i16 sd = 0;
    _i16 AddrSize = 0;
    _i32 ret_val = 0;

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(80);

    /* Change the DestinationIP endianity, to big endian */
    Addr.sin_addr.s_addr = sl_Htonl(appData.DestinationIP);

    AddrSize = sizeof(SlSockAddrIn_t);

    sd = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( sd < 0 )
    {
        printf("Error creating socket\r\n");
        ASSERT_ON_ERROR(sd);
    }

    ret_val = sl_Connect(sd, ( SlSockAddr_t *)&Addr, AddrSize);
    if( ret_val < 0 )
    {
        /* error */
        printf("Error connecting to socket\r\n");
        ASSERT_ON_ERROR(ret_val);
    }

    return sd;
}

/*!
    \brief Connecting to a WLAN Access point

    This function connects to the AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]  UserParams - Structure having SSID, security type and pass key
                of AP to connect

    \return         0 for success and negative for error

    \note

    \warning        If the WLAN connection fails or we don't acquire an IP
                    address, We will be stuck in this function forever.
*/
static _i32 establishConnectionWithAP(UserInfo UserParams)
{
    SlSecParams_t secParams = {0};
    _i32 retVal = -1;

    if (UserParams.encryption == 1)
    {
        secParams.Key = (_i8 *)"";
        secParams.KeyLen = 0;
        secParams.Type = SL_SEC_TYPE_OPEN;
    }

    else
    {
        secParams.Key = UserParams.password;
        secParams.KeyLen = strlen((const char *)UserParams.password);

        if (UserParams.encryption == 2) secParams.Type = SL_SEC_TYPE_WEP;
        if (UserParams.encryption == 3) secParams.Type = SL_SEC_TYPE_WPA;
    }


    retVal = sl_WlanConnect(UserParams.SSID,strlen((const char *)UserParams.SSID),0,
                                                                &secParams,0);

    ASSERT_ON_ERROR(retVal);

    printf("Connecting to AP %s...\n", UserParams.SSID  );

    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status)));

    return SUCCESS;
}

/*!
    \brief This function obtains the server IP address

    \param[in]      none

    \return         zero for success and -1 for error

    \warning
*/
static _i32 GetHostIP()
{
    _i32 status = -1;

    status = sl_NetAppDnsGetHostByName(appData.HostName,
                                       strlen((const char *)appData.HostName),
                                       &appData.DestinationIP, SL_AF_INET);
    ASSERT_ON_ERROR(status);

    return SUCCESS;
}

/*!
    \brief Get the Weather from server

    \param[in]      none

    \return         zero for success and -1 for error

    \warning
*/
static _i32 getWeather()
{
    _u32 i = 0;
    _i32 retVal = -1;
    _u8 readBuff[MAX_CITY_NAME_SIZE] = {'\0'};

    memcpy(appData.HostName,WEATHER_SERVER,strlen(WEATHER_SERVER));

    retVal = GetHostIP();
    if(retVal < 0)
    {
        printf(" Failed to get the host IP, Error code: %ld\r\n",retVal);
        ASSERT_ON_ERROR(retVal);
    }
    
    appData.SockID = (_i16)CreateConnection();
    if(appData.SockID < 0)
    {
        printf(" Failed to create connection, Error code: %ld\r\n",appData.SockID);
        ASSERT_ON_ERROR(appData.SockID);
    }
    while (1)
    {
        printf("\r\nEnter the city name or \"QUIT\" to quit#: \n");
        fflush(stdin);
        scanf_s("%[^\n]s",readBuff,MAX_CITY_NAME_SIZE);

        memset(appData.CityName, 0x00, sizeof(appData.CityName));

        memcpy(appData.CityName, readBuff, strlen((const char *)readBuff));

        if(!strcmp((const char *)appData.CityName,"QUIT") || !strcmp((const char *)appData.CityName,"quit"))
            break;

        retVal = GetData();
        ASSERT_ON_ERROR(retVal);
    }

    retVal = sl_Close(appData.SockID);
    ASSERT_ON_ERROR(retVal);

    printf("\r\nThank you!\r\n\r\n");

    return SUCCESS;
}
