/*
 * main.c - sample application to send email
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
 * Application Name     -   CC31xx Email Application
 *
 * Application Overview -   This sample application demonstrates how CC3100 can
 *                          be used to send an email over SMTP. The application
 *                          configures CC3100 to connect with an SMTP server and
 *                          sends email to it. SMTP server forwards it to the
 *                          recipient's email-server and the recipient receives
 *                          the email from his email-server using IMAP/POP3 and/
 *                          or other proprietary protocol.
 *
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Email_Demo_Application
 *                          doc\examples\sls_email.pdf
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "email.h"
#include "config.h"
#include "simplelink.h"

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
UserInfo User;

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
static _i32 ReadAndProcessInput();
static UserInfo GetUserInput();
static void OutputMenu(void);
static _i32 UARTCommandHandler(_u8 *usBuffer);
static _i32 SetEmail();
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
   _i8 *pConfig = NULL;
   _i32 retVal = -1;

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

    /* Generate Banner and Output Menu for Application */
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

    printf("Device started as STATION \r\n\r\n");

    /* configure the Source email */
    retVal = SetEmail();
    if(retVal < 0)
    {
        printf("Failed to configure the source Email, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    /* Read connection parameters */
    User = GetUserInput();

    /* Display the command menu */
    OutputMenu();

    /* Read the command over Application UART and process it */
    retVal = ReadAndProcessInput();
    if(retVal < 0)
        LOOP_FOREVER();

    printf("\r\nThank you!\r\n");

    system("PAUSE");

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
    memset(&User, 0, sizeof(User));
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
    printf("CC31xx Email App \n");
    printf(APPLICATION_VERSION);
    printf("\r\n*****************************************************************\r\n");
}

/*!
    \brief Read option fron the user

    \param[in]      none

    \return         _i16 - user option value

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
    \brief Send the email to the preconfigured email ID

    \param[in]      none

    \return         0 for success , -1 otherwise

    \note

    \warning
*/
static _i32 ReadAndProcessInput()
{
    _i32 retVal = -1;
    _u8 inputBuff[INPUT_BUF_SIZE] = {'\0'};

    /* Main loop. Waits for commands from PC. */
    while(1)
    {
        memset(inputBuff, 0x00, sizeof(inputBuff));
        fflush(stdin);
        scanf_s("%[^\n]%*c", inputBuff, _countof(inputBuff));
        retVal = UARTCommandHandler(inputBuff);
        if(retVal < 0)
        {
            if(retVal == INVALID_INPUT)
                printf("Invalid input\r\n");
            else
                return retVal;
        }

        if(!strncmp((const char*)inputBuff, "05", 2))
            break;
    }

    return SUCCESS;
}

/*!
    \brief Get the AP parameters from the user

    \param[in]      none

    \return         UserInfo - structure contains SSID, encryption type and
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
    UserFunction.encryption =(_u8) GetUserNum();

    /* Option enum is offset by 1 */
    UserFunction.encryption-=1;

    if (UserFunction.encryption != 0)
    {
        printf ("Please enter the password: \n");
        scanf_s("%s",UserFunction.password,MAX_PASSKEY_LEN);
    }

    return UserFunction;

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

    printf("Connecting to AP %s...\n", UserParams.SSID  );

    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status)));

    return SUCCESS;
}

/*!
    \brief This function displays the menu over the application UART terminal

    \param[in]      none

    \return         None

    \note

    \warning
*/
static void OutputMenu(void)
{
    /* Display Menu Options for Application */
    printf("\r\nCOMMAND MENU\r\n");
    printf("\r\n============================\r\n");
    printf("01 to connect\r\n");
    printf("03,<to>,<subject>\r\n");
    printf("04,<message>\r\n");
    printf("05 to send message\r\n\r\n");
}

/*!
    \brief This function Process the commands received over application uart

    \param[in]      usBuffer - pointer to the command

    \return         none

    \note

    \warning
*/
static _i32 UARTCommandHandler(_u8 *usBuffer)
{
    _i32 retVal = -1;
    _u16 index = -1;

    _u32 paramcount = 0;

    switch(usBuffer[1])
    {
        /* Case 01: Start a smart configuration process    */
        case UART_COMMAND_CC31xx_SIMPLE_CONFIG_START:
            retVal = establishConnectionWithAP(User);
            if(retVal < 0)
            {
                printf("Failed to establish connection w/ AP, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }
            
            printf("Connected to AP %s. IP Acquired\n",User.SSID);
        break;

        /* Case 03: Configure sender (source) email */
        case UART_COMMAND_CC31xx_EMAIL_HEADER:
        {
            SlNetAppDestination_t destEmailAdd;
            SlNetAppEmailSubject_t emailSubject;
            _u8 *pcOfemailto, *pcOfemailsubject;

            memset(destEmailAdd.Email, '\0', MAX_DEST_EMAIL_LEN);
            memset(emailSubject.Value, '\0', MAX_SUBJECT_LEN);

            pcOfemailto = (_u8 *)(destEmailAdd.Email);
            pcOfemailsubject = (_u8 *)(emailSubject.Value);
            /* '<' To maintain RFC 2821 format */
            *pcOfemailto++= '<';
            index = 2;
            while (usBuffer[index] != 0x0D  && usBuffer[index] != '\0')
            {
                /* look for comma ',' for separation of params */
                if(usBuffer[index] == 44)
                {
                    paramcount++;
                }
                else
                {
                    if(paramcount==1)
                    {
                        /* Enter destination email address */
                        *pcOfemailto++ = usBuffer[index];
                    }

                    if(paramcount==2)
                    {
                        /* Enter email subject */
                        *pcOfemailsubject++ = usBuffer[index];
                    }
                }
                index++;
            }
            /* '>' To maintain RFC 2821 format */
            *pcOfemailto++= '>';
            *pcOfemailto++= '\0';
            *pcOfemailsubject= '\0';

            retVal = sl_NetAppEmailSet(NETAPP_DEST_EMAIL,
                strlen((const char *)destEmailAdd.Email), destEmailAdd.Email);
            if(retVal < 0)
            {
                printf("Failed to configure the destination email address, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }


            retVal = sl_NetAppEmailSet(NETAPP_SUBJECT,
                strlen((const char *)emailSubject.Value), emailSubject.Value);
            if(retVal < 0)
            {
                printf("Failed to set the email subject, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }

            printf("\rOK\r\n");
        }
        break;

        /* Case 04: Record email message */
        case UART_COMMAND_CC31xx_EMAIL_MESSAGE:
        {
            _u8 *pcOfemailmessage;
            pcOfemailmessage = (usBuffer + 3);
            /* Enter "Message" */
            index =0;
            if((*pcOfemailmessage) == 60)
                pcOfemailmessage++;

            while ((*pcOfemailmessage) != 0x0D && (*pcOfemailmessage) != '\0')
            {
                if((*pcOfemailmessage) == 62)
                {
                    break;
                }
                index++;
                pcOfemailmessage++;
            }

            retVal = sl_NetAppEmailSet(NETAPP_MESSAGE,index,(usBuffer + 3));
            if(retVal < 0)
            {
                printf("\r\nFailed to set the email message, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }
            
            printf("\rOK\r\n");
        }
        break;

        /* Case 05: Send email message using configurations    */
        case UART_COMMAND_CC31xx_EMAIL_SEND:
        {
            retVal = sl_NetAppEmailConnect();
            if(retVal < 0)
            {
                printf("\r\nFailed to connect to gmail server, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }

            retVal = sl_NetAppEmailSend();
            if(retVal < 0)
            {
                printf("\r\nFailed to send email, Error code: %ld\r\n",retVal);
                ASSERT_ON_ERROR(retVal);
            }
        
            printf("\r\nMessage Sent\r\n");
        }
        break;

        default:
            ASSERT_ON_ERROR(INVALID_INPUT);
        break;
    }

    return SUCCESS;
}

/*!
    \brief This function configures the source email using parameters defined
           in "config.h" file

    \param[in]      none

    \return         none

    \note

    \warning
*/
static _i32 SetEmail()
{
    _i32 retVal = -1;

    SlNetAppSourceEmail_t sourceEmailId;
    SlNetAppSourcePassword_t sourceEmailPwd;
    SlNetAppEmailOpt_t eMailServerSetting;

    memcpy(sourceEmailId.Username,USER,strlen(USER)+1);
    retVal = sl_NetAppEmailSet(NETAPP_SOURCE_EMAIL,strlen(USER)+1,
                                            (_u8*)&sourceEmailId);
    ASSERT_ON_ERROR(retVal);

    memcpy(sourceEmailPwd.Password,PASS,strlen(PASS)+1);
    retVal = sl_NetAppEmailSet(NETAPP_PASSWORD,strlen(PASS)+1,
                                            (_u8*)&sourceEmailPwd);
    ASSERT_ON_ERROR(retVal);

    eMailServerSetting.Family = SL_AF_INET;
    eMailServerSetting.Port = GMAIL_HOST_PORT;
    eMailServerSetting.Ip = SL_IPV4_VAL(74,125,129,108);
    eMailServerSetting.SecurityMethod = SL_SO_SEC_METHOD_SSLV3;
    eMailServerSetting.SecurityCypher = SL_SEC_MASK_SSL_RSA_WITH_RC4_128_MD5;

    retVal = sl_NetAppEmailSet(NETAPP_ADVANCED_OPT,sizeof(SlNetAppEmailOpt_t),
                                        (_u8*)&eMailServerSetting);
    ASSERT_ON_ERROR(retVal);

    return SUCCESS;
}
