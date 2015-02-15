/*
 * main.c - sample messaging application via xmpp server
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * All rights reserved. Property of Texas Instruments Incorporated.
 * Restricted rights to use, duplicate or disclose this code are
 * granted through contract.
 *
 * The program may not be used without the written permission of
 * Texas Instruments Incorporated or against the terms and conditions
 * stipulated in the agreement under which this program has been supplied,
 * and under no circumstances can it be used with non-TI connectivity device.
 *
 */

/*
 * Application Name     -   Messaging Application using XMPP Client on CC3100
 * Application Overview -   This sample application demonstrates how to connect 
 * and work with XMPP server using CC31xx. It receives/sends an XMPP messages from/to the server.
 *
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Reference_Application
 *                          doc\examples\sls_xmpp_client.pdf
 */


#include <windows.h>
#include <stdio.h>
#include "simplelink.h"
#include "xmpp.h"
#include "config.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

typedef struct{
    _i8 SSID[MAX_SSID_LEN];
    _u8 encryption;
    _i8 password[MAX_PASSKEY_LEN];
}UserInfo;


/*
 * GLOBAL VARIABLES -- Start
 */
_u32  g_Status = 0;

/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 establishConnectionWithAP(UserInfo);
static _i32 initializeAppVariables();
static void displayBanner();
static _i16 GetUserNum();
static UserInfo GetUserInput();

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
 
    switch( pNetAppEvent->Event )
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
    /* Unused in this application */
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
 * ASYNCHRONOUS EVENT HANDLERS -- End
 */

/*
 * Application's entry point
 */
int main(int argc, char** argv)
{
    UserInfo User;

    SlNetAppXmppOpt_t XmppOption;
    SlNetAppXmppUserName_t UserName;
    SlNetAppXmppPassword_t Password;
    SlNetAppXmppDomain_t Domain;
    SlNetAppXmppResource_t Resource;
    _u8 pRemoteJid[MAX_RCV_BUF_SIZE] = {'\0'};
    _u8 pRecvMessage[MAX_RCV_BUF_SIZE] = {'\0'};
    _i32 Status = -1;

    _i8 *pConfig = NULL;

    Status = initializeAppVariables();
    ASSERT_ON_ERROR(Status);

#ifdef SL_IF_TYPE_UART
    params.BaudRate = BAUD_RATE;
    params.FlowControlEnable = 1;
    params.CommPort = COMM_PORT_NUM;

    pConfig = (_i8 *)&params;
#endif /**SL_IF_TYPE_UART /

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
    Status = configureSimpleLinkToDefaultState(pConfig);
    if(Status < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == Status)
            printf(" Failed to configure the device in its default state, Error code: %ld\r\n",Status);

        LOOP_FOREVER();
    }

    printf("Device is configured in default state \r\n");

    /*
     * Assumption is that the device is configured in station mode already
     * and it is in its default state
     */
    Status = sl_Start(0, pConfig, 0);
    if ((Status < 0) ||
        (ROLE_STA != Status) )
    {
        printf("Failed to start the device, Error code: %ld\r\r\n",Status);
        LOOP_FOREVER();
    }
    printf("Device started as STATION \r\n");

    User = GetUserInput();
    /** Connecting to WLAN AP
     * After this call we will be connected and have IP address
     */
    Status = establishConnectionWithAP(User);
    if(Status < 0)
    {
        printf("Failed to establish connection w/ an AP, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }
    printf("Connected to AP %s. IP Acquired.\n\n\r\n",User.SSID);

    printf("Establishing connection with the XMPP server \r\n");

    /* Configuring parameters required for XMPP connection*/
    XmppOption.Port = XMPP_DST_PORT;
    XmppOption.Family = SL_AF_INET;
    XmppOption.SecurityMethod = SO_SECMETHOD_SSLV3;
    XmppOption.SecurityCypher = SECURE_MASK_SSL_RSA_WITH_RC4_128_SHA;
    XmppOption.Ip = XMPP_IP_ADDR;
    Status = sl_NetAppXmppSet(SL_NET_APP_XMPP_ID, NETAPP_XMPP_ADVANCED_OPT,
                     sizeof(SlNetAppXmppOpt_t), (_u8 *)&XmppOption);
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    memcpy(UserName.UserName, XMPP_USER, strlen(XMPP_USER));
    UserName.Length = strlen(XMPP_USER);
    Status = sl_NetAppXmppSet(SL_NET_APP_XMPP_ID, NETAPP_XMPP_USER_NAME,
                     UserName.Length, (_u8 *)&UserName);
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    memcpy(Password.Password, XMPP_PWD, strlen(XMPP_PWD));
    Password.Length = strlen(XMPP_PWD);
    Status = sl_NetAppXmppSet(SL_NET_APP_XMPP_ID, NETAPP_XMPP_PASSWORD,
                     Password.Length, (_u8 *)&Password);
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    memcpy(Domain.DomainName, XMPP_DOMAIN, strlen(XMPP_DOMAIN));
    Domain.Length = strlen(XMPP_DOMAIN);
    Status = sl_NetAppXmppSet(SL_NET_APP_XMPP_ID, NETAPP_XMPP_DOMAIN,
                     Domain.Length, (_u8 *)&Domain);
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    memcpy(Resource.Resource,XMPP_RESOURCE, strlen(XMPP_RESOURCE));
    Resource.Length = strlen(XMPP_RESOURCE);
    Status = sl_NetAppXmppSet(SL_NET_APP_XMPP_ID, NETAPP_XMPP_RESOURCE,
                     Resource.Length, (_u8 *)&Resource);
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    Status = sl_NetAppXmppConnect();
    if(Status < 0)
    {
        printf(" Error during establishing connection with the XMPP server, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    printf("Connection established successfully with the XMPP server\r\n");

    printf("Waiting for message....\r\n");
    Status = sl_NetAppXmppRecv(pRemoteJid, MAX_RCV_BUF_SIZE, pRecvMessage, MAX_RCV_BUF_SIZE );

    while ( Status < 0)
    {
        Status = sl_NetAppXmppRecv(pRemoteJid, MAX_RCV_BUF_SIZE, pRecvMessage, MAX_RCV_BUF_SIZE );
    }

    /* Send back the received message */
    Status = sl_NetAppXmppSend(pRemoteJid, strlen((const char *)pRemoteJid),
                               pRecvMessage, strlen((const char *)pRecvMessage));
    if(Status < 0)
    {
        printf(" Error while sending back the message, Error code: %ld\r\n",Status);
        LOOP_FOREVER();
    }

    printf("Message sent back successfully \r\n");

    /* Stop the CC3100 device */
    Status = sl_Stop(SL_STOP_TIMEOUT);
    if(Status < 0)
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
    \brief Connecting to a WLAN Access point

    This function connects to the required AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]  None

    \return     0 on success, negative error-code on error

    \note

    \warning    If the WLAN connection fails or we don't acquire an IP address,
                We will be stuck in this function forever.
*/
static _i32 establishConnectionWithAP(UserInfo UserParams)
{
    SlSecParams_t secParams = {0};
    _i32 retVal = 0;

    if(UserParams.encryption == 0)
    {
        secParams.Key = (_i8 *)"";
        secParams.KeyLen = 0;
        secParams.Type = SL_SEC_TYPE_OPEN;
    }

    else
    {
        secParams.Key = UserParams.password;
        secParams.KeyLen = strlen((const char *)UserParams.password);
        secParams.Type = UserParams.encryption;
    }
    retVal = sl_WlanConnect(UserParams.SSID,strlen((const char *)UserParams.SSID),0,
                                                                &secParams,0);
    ASSERT_ON_ERROR(retVal);

    printf("Connecting to AP %s...\r\n", UserParams.SSID  );
    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status)));

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
    printf("CC31xx Xmpp_Client App \n");
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
    _i16 flag = 0;
    _u8 input[20];
    _i16 value;

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
    _i16 eflag = 0;
    _i16 wepflag = 0;
    _i16 length = 0;


    printf("Please input the SSID you wish to connect to: \n");
    scanf_s("%[^\n]s",UserFunction.SSID,MAX_SSID_LEN);

    printf("Encryption Types:\n");
    printf("1:  Open\n");
    printf("2:  WEP\n");
    printf("3:  WPA\n");
    printf("Please enter the corresponding number for the encryption type: \n");
    UserFunction.encryption = (_u8)GetUserNum();

    /* Option enum is offset by 1 */
    UserFunction.encryption-=1;

    if (UserFunction.encryption != 0)
    {
        printf ("Please enter the password: \n");
        scanf_s("%s",UserFunction.password,MAX_PASSKEY_LEN);
    }

    return UserFunction;

}
