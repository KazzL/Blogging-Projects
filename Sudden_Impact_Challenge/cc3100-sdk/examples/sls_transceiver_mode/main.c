/*
 * main.c - sample code for using transceiver mode
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
 * Application Name     -  Transceiver Mode Application
 * Application Overview -  This sample application demonstrates how to use 
 *                         CC3100 transceiver mode of operation. 
 * Application Details  -  http://processors.wiki.ti.com/index.php/CC31xx_SLS_Transceiver_Mode_Application
 *                          doc\examples\sls_transceiver_mode.pdf
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

#define SUCCESS              0
#define MAC_ADDR_LENGTH     (6)
#define MAC_ADDR_LEN_STR    (18)    /*xx:xx:xx:xx:xx:xx*/
#define IP_ADDR_LENGTH      (4)
#define IP_ADDR_LEN_STR     (16)    /*xxx:xxx:xxx:xxx*/
#define MAX_RECV_BUF_SIZE   1536
#define SWAP_UINT32(val)    (((val>>24) & 0x000000FF) + ((val>>8) & 0x0000FF00) + \
                            ((val<<24) & 0xFF000000) + ((val<<8) & 0x00FF0000))
#define SWAP_UINT16(val)    ((((val)>>8) & 0x00FF) + (((val)<<8) & 0xFF00))

#define POWER_LEVEL_TONE    1   /* Power level tone valid range 0-15 */
#define PREAMBLE            1   /* Preamble value 0- short, 1- long */
#define MAX_BUF_SIZE        256
#define MAX_BUF_RX_STAT     1500
#define BYTES_TO_RECV       1470

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM       63
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */


typedef struct{
    _i32 choice;
    _i16 channel;
    _u16 packets;
}UserIn;

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
_u32  g_Status = 0;

const _u8 RawData_Ping[] = {
                       0x88,   /* version , type sub type */
                       0x02,   /* Frame control flag */
                       0x2C, 0x00,
                       0x00, 0x23, 0x75, 0x55,0x55, 0x55,   /* destination */
                       0x00, 0x22, 0x75, 0x55,0x55, 0x55,   /* bssid */
                       0x00, 0x22, 0x75, 0x55,0x55, 0x55,   /* source */
                       0x80, 0x42, 0x00, 0x00,
                       0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, /* LLC */
                       /*---- ip header start -----*/
                       0x45, 0x00, 0x00, 0x54, 0x96, 0xA1, 0x00, 0x00, 0x40, 0x01,
                       0x57, 0xFA,                          /* checksum */
                       0xc0, 0xa8, 0x01, 0x64,              /* src ip */
                       0xc0, 0xa8, 0x01, 0x65,              /* dest ip  */
                       /* payload - ping/icmp */
                       0x08, 0x00, 0xA5, 0x51,
                       0x5E, 0x18, 0x00, 0x00, 0x41, 0x08, 0xBB, 0x8D, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00
                       };
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 initializeAppVariables();
static void displayBanner();
static _i32 GetUserNum(_i32 boundlow, _i32 boundhigh, _i32 pack, _i32 *p_flag);
static UserIn UserInput();
static _i32 TxContinues(_i16 channel, SlRateIndex_e rate,
                 _u16 numberOfPackets, DWORD interval);
static _i32 RxStatisticsCollect(_i16 channel);
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

            /* If the user has initiated 'Disconnect' request, 'reason_code' is 200 */
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
int main()
{
    _u8 input[MAX_BUF_SIZE] = {0};
    UserIn User = {0};
    _i32 flag = 1;
    _i32 first = 0;
    _i32 retVal=-1;
    _i8 *pConfig = NULL;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

#ifdef SL_IF_TYPE_UART
    params.BaudRate = 115200;
    params.FlowControlEnable = 1;
    params.CommPort = COMM_PORT_NUM;

    pConfig = (_i8 *)&params;
#endif /* SL_IF_TYPE_UART */

    /* This line is for Eclipse CDT only due to a known bug in console buffering
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
    retVal = sl_Start(0, pConfig, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        printf(" Failed to start the device, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    printf("Device started as STATION \r\n");

    while(flag)
    {
        User = UserInput();
        switch(User.choice)
        {
            case 1:
                retVal = TxContinues(User.channel, RATE_11M, User.packets, 100);
                if(retVal < 0)
                {
                    printf("Failed to send the packet over phy, Error code: %ld",retVal);
                    LOOP_FOREVER();
                }
                break;
            case 2:
                retVal = RxStatisticsCollect(User.channel);
                if(retVal < 0)
                {
                    printf("Failed to collect statics, Error code: %ld",retVal);
                    LOOP_FOREVER();
                }
                break;
        }

        printf("\nEnter \"1\" to restart or \"0\" to quit: \n");
        fgets((char *)input, sizeof(input), stdin);
        printf("\n");
        flag = atoi((const char *)input);

        first = 1;
    }

    return SUCCESS;
}


static void displayBanner()
{
    printf("\r\n\r\n");
    printf("Sniffer with  Filter Application - Version \r\n");
    printf(APPLICATION_VERSION);
    printf("\r\n******************************************************\r\n");
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
    
    return SUCCESS;
}

/*!
    \brief Read option fron the user

    \param[in]      none

    \return         int - user option value

    \note

    \warning
*/
static _i32 GetUserNum(_i32 boundlow, _i32 boundhigh, _i32 pack, _i32 *p_flag)
{
    _u8 input[MAX_BUF_SIZE] = {'\0'};
    _i32 user = -1;

    if (fgets((char *)input, sizeof(input), stdin) != NULL)
    {
        user = atoi((const char *)input);
        if( !pack)
        {
            if (user > boundlow && user < boundhigh)
                *p_flag = 1;
            else
                printf("\nInvalid choice.\n");
        }
        else
        {
            if (user > 0)
                *p_flag = 1;
            else
                printf("\nInvalid choice.\n");
        }
    }

    return user;
}

/*!
    \brief Get the Raw socket parameters from the user

    \param[in]      none

    \return         UserIn - structure containing Raw socket parameters

    \note

    \warning
*/
static UserIn UserInput()
{
    UserIn User;
    _i32 flag = 0;

    memset(&User, 0, sizeof(User));

    while (!flag)
    {
        printf("Options:\n");
        printf("1. Send packets.\n");
        printf("2. Collect statistics about received packets.\n");
        printf("Please enter the option you would like to use: \n");
        User.choice = GetUserNum(0, 3, 0, &flag);
        printf("\n");
    }

    flag = 0;
    while (!flag)
    {
        printf("Enter a channel between 1 and 13 to use: \n");
        User.channel = (_i16)GetUserNum(0, 14, 0, &flag);
        printf("\n");
    }

    flag = 0;

    if (User.choice == 1)
    {
        printf("Enter the number of packets to send : \n");
        User.packets = (_u16)GetUserNum(0, 0, 1, &flag);
        printf("\n");
    };

    return User;
}

/*!
    \brief Entering raw Transmitter\Receiver mode in order to send raw data
           over the WLAN PHY

    This function shows how to send raw data, in this case ping packets over
    the air in transmitter mode.

    \param[in]      Channel - number on which the data will be sent
    \param[in]      rate    - Rate for tx
    \param[in]      numberOfPackets - packets to be send
    \param[in]      interval - interval between each packet, should be greater
                    than 50ms.

    \return         0 for success, -ve otherwise

    \warning        We must be disconnected from WLAN AP in order to succeed
                    changing to transmitter mode
*/
static _i32 TxContinues(_i16 channel, SlRateIndex_e rate,
                 _u16 numberOfPackets, DWORD interval)
{
    _i16 sd = -1;
    _i16 len = -1;
    _u32 retVal = -1;

    sd = sl_Socket(SL_AF_RF, SL_SOCK_RAW, channel);
    if(sd < 0)
    {
        printf("Error In Creating the Socket\n");
        ASSERT_ON_ERROR(sd);
    }

    printf("Transmitting pinging data...\n");
    len = sizeof(RawData_Ping);
    while(numberOfPackets > 0)
    {
        retVal = sl_Send(sd, RawData_Ping, len,
                SL_RAW_RF_TX_PARAMS(channel, rate, POWER_LEVEL_TONE, PREAMBLE));
        ASSERT_ON_ERROR(retVal);

        Sleep(interval);
        numberOfPackets--;
    }
    printf("Transmitting complete.\n");
    
    retVal = sl_Close(sd);
    ASSERT_ON_ERROR(retVal);
    
    return SUCCESS;
}

/*!
    \brief Entering raw Transmitter\Receiver mode in order to perform Rx
           statistic over a specific WLAN channel


    \param[in]      Channel number on which the statistics will be calculated

    \return         0 for success, -ve otherwise

    \note

    \warning        We must be disconnected from WLAN AP in order to succeed
                    changing to Receiver mode
*/
static _i32 RxStatisticsCollect(_i16 channel)
{
    SlGetRxStatResponse_t rxStatResp;
    _u8 buffer[MAX_BUF_RX_STAT] = {'\0'};
    _u8 var[MAX_BUF_SIZE] = {'\0'};

    _i32 idx = -1;
    _i16 sd = -1;
    _i32 retVal = -1;

    memset(&rxStatResp, 0, sizeof(rxStatResp));

    sd = sl_Socket(SL_AF_RF,SL_SOCK_RAW,channel);
    if(sd < 0)
    {
        printf("Error In Creating the Socket\n");
        ASSERT_ON_ERROR(sd);
    }

    retVal = sl_Recv(sd, buffer, BYTES_TO_RECV, 0);
    ASSERT_ON_ERROR(retVal);

    printf("Press \"Enter\" to start collecting statistics.\n");
    fgets((char *)var, sizeof(var), stdin);
    
    retVal = sl_WlanRxStatStart();
    ASSERT_ON_ERROR(retVal);

    printf("Press \"Enter\" to get the statistics.\n");
    fgets((char *)var, sizeof(var), stdin);
    
    retVal = sl_WlanRxStatGet(&rxStatResp, 0);
    ASSERT_ON_ERROR(retVal);

    printf("\n\n*********************Rx Statistics**********************\n\n");
    printf("Received Packets - %d\n",rxStatResp.ReceivedValidPacketsNumber);
    printf(" Received FCS - %d\n",rxStatResp.ReceivedFcsErrorPacketsNumber);
    printf(" Received PLCP - %d\n",rxStatResp.ReceivedPlcpErrorPacketsNumber);
    printf("Average Rssi for management: %d    Average Rssi for other packets: %d\n",
                    rxStatResp.AvarageMgMntRssi,rxStatResp.AvarageDataCtrlRssi);
    for(idx = 0 ; idx < SIZE_OF_RSSI_HISTOGRAM ; idx++)
    {
        printf("Rssi Histogram cell %d is %d\n", idx, rxStatResp.RssiHistogram[idx]);
    }

    printf("\n");
    for(idx = 0 ; idx < NUM_OF_RATE_INDEXES; idx++)
    {
        printf("Rate Histogram cell %d is %d\n", idx, rxStatResp.RateHistogram[idx]);
    }

    printf("The data was sampled in %u microseconds.\n",
          ((_i16)rxStatResp.GetTimeStamp - rxStatResp.StartTimeStamp));
    printf("\n\n*******************End Rx Statistics********************\n");

    retVal = sl_WlanRxStatStop();
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Close(sd);
    ASSERT_ON_ERROR(retVal);

    return SUCCESS;
}
