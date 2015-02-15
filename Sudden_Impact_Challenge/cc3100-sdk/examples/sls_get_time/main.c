/*
 * main.c - get time sample application
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
 * Application Name     -   Get time
 * Application Overview -   This sample application demonstrates how
 *                          to connect to an SNTP server and request for time
 *                          information. The application processes the received
 *                          data and displays the time on console
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_Get_Time_Application
 *                          doc\examples\get_time.pdf
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
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
                   printf("Error Code %d\n", error_code);\
                   return error_code; \
                }\
                /* else, continue w/ execution */ \
            }

#define SUCCESS             0
#define MAX_BUF_SIZE        48
#define MAX_PASSKEY_SIZE    32
#define MAX_SSID_SIZE       32

#define TIME2013        3565987200       /* 43 years + 11 days of leap years */
#define YEAR2013        2013
#define SEC_IN_MIN      60
#define SEC_IN_HOUR     3600
#define SEC_IN_DAY      86400

/*
 * Values for below macros shall be modified for setting the time-zone
 */
#define GMT_TIME_ZONE_HR    5
#define GMT_TIME_ZONE_MIN   30

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM 21
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    SNTP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    SNTP_RECV_ERROR = SNTP_SEND_ERROR - 1,
    SNTP_SERVER_RESPONSE_ERROR = SNTP_RECV_ERROR - 1,

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
    _i16 gmt_hr;
    _i16 gmt_min;
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */
_u32  g_Status = 0;
struct{
    _u32   DestinationIP;
    _u32   elapsedSec;
    _u32   uGeneralVar;
    _u32   uGeneralVar1;

    _u16   ccLen;

    _i16   SockID;
    _i32   sGeneralVar;

    _u8 time[30];
    _u8 *ccPtr;

}appData;
/*
 * GLOBAL VARIABLES -- End
 */


/*
 * CONTSTANT -- Start
 */
/*! ######################### List of SNTP servers ############################
 *! ##
 *! ##          hostname         |        IP        |       location
 *! ## ------------------------------------------------------------------------
 *! ## nist1-nj2.ustiming.org    | 165.193.126.229  | Weehawken, NJ
 *! ## nist1-pa.ustiming.org     | 206.246.122.250  | Hatfield, PA
 *! ## time-a.nist.gov           | 129.6.15.28      | NIST, Gaithersburg, Maryland
 *! ## time-b.nist.gov           | 129.6.15.29      | NIST, Gaithersburg, Maryland
 *! ## time-c.nist.gov           | 129.6.15.30      | NIST, Gaithersburg, Maryland
 *! ## ntp-nist.ldsbc.edu        | 198.60.73.8      | LDSBC, Salt Lake City, Utah
 *! ## nist1-macon.macon.ga.us   | 98.175.203.200   | Macon, Georgia
 *! ## 0.in.pool.ntp.org         | 123.108.225.6    | India
 
 *! ## For more SNTP server link visit 'http://tf.nist.gov/tf-cgi/servers.cgi'
 *! ###########################################################################
 */
const _u8 SNTPserver[30] = "nist1-nj2.ustiming.org"; /* Add one of the above servers */

/* Tuesday is the 1st day in 2013 - the relative year */
const _u8 daysOfWeek2013[7][4] = {{"Tue"},
                                   {"Wed"},
                                   {"Thu"},
                                   {"Fri"},
                                   {"Sat"},
                                   {"Sun"},
                                   {"Mon"}};

const _u8 monthOfYear[12][4] = {{"Jan"},
                                 {"Feb"},
                                 {"Mar"},
                                 {"Apr"},
                                 {"May"},
                                 {"Jun"},
                                 {"Jul"},
                                 {"Aug"},
                                 {"Sep"},
                                 {"Oct"},
                                 {"Nov"},
                                 {"Dec"}};

const _u8 numOfDaysPerMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

const _u8 digits[] = "0123456789";
/*
 * CONTSTANT -- End
 */


/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 establishConnectionWithAP(UserInfo);
static _i32 disconnectFromAP();
static _i32 configureSimpleLinkToDefaultState(_i8 *);
static _i32 initializeAppVariables();
static void  displayBanner();
static _i16 GetUserNum();
static UserInfo GetUserInput();
static _i32 getHostIP();
static _i16 createConnection();
static _i32 getSNTPTime(_i16 gmt_hr, _i16 gmt_min);
static _u16 iToa(_i32 cNum, _u8 *cString);

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
                printf("Device disconnected from the AP on application's request \r\n");
            }
            else
            {
                printf("Device disconnected from the AP on an ERROR..!! \r\n");
            }
        }
        break;

        default:
        {
            printf("[WLAN EVENT] Unexpected event \r\n");
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
             * Information about the connected AP's IP, gateway, DNS etc
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
            printf("[NETAPP EVENT] Unexpected event \r\n");
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
    printf("[HTTP EVENT] Unexpected event \r\n");
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
    printf("[GENERAL EVENT] \r\n");
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
                    printf("[SOCK EVENT] Close socket operation failed to transmit all queued packets\r\n");
                break;


                default:
                    printf("[SOCK EVENT] Unexpected event \r\n");
                break;
            }
        }
        break;

        default:
            printf("[SOCK EVENT] Unexpected event \r\n");
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
    UserInfo User = {0};
    _i32 retVal = -1;
    _i8 *pConfig = NULL;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

#ifdef SL_IF_TYPE_UART
    params.BaudRate = BAUD_RATE;
    params.FlowControlEnable = 1;
    params.CommPort = COMM_PORT_NUM;

    pConfig = &params;
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
        printf("Failed to configure the device in its default state, Error code : ld\r\n", retVal);

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
        printf("Failed to start the device, Error Code : %ld\r\n", retVal);
        LOOP_FOREVER();
    }

    printf("Device started as STATION \r\n");

    User = GetUserInput();

    /* Connecting to WLAN AP */
    retVal = establishConnectionWithAP(User);
    if(retVal < 0)
    {
        printf("Failed to establish connection w/ an AP, Error code: %ld\r\n", retVal);
        LOOP_FOREVER();
    }

    printf("Connection established w/ AP and IP is acquired \r\n");

    retVal = getHostIP();
    if(retVal < 0)
    {
        printf("Unable to get host IP, Error code: %ld\r\n\r\n",retVal);
        LOOP_FOREVER();
    }

    appData.SockID = createConnection();
    if(appData.SockID < 0)
    {
        printf("Error creating socket, Error code: %ld\r\n\r\n", appData.SockID);
        LOOP_FOREVER();
    }

    retVal = getSNTPTime(User.gmt_hr,User.gmt_min);
    if(retVal < 0)
        LOOP_FOREVER();

    retVal = sl_Close(appData.SockID);
    if(retVal < 0)
        LOOP_FOREVER();

    retVal = disconnectFromAP();
    if(retVal < 0)
    {
        printf("Failed to disconnect from the AP, Error code: %ld\r\n",retVal);
        LOOP_FOREVER();
    }

    system("PAUSE");

    return 0;
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
    _u8 GMT_value[7] = {0};
    _i32 eflag = -1;
    _i32 wepflag = -1;
    _i32 length = -1;
    _i8 *token = NULL;
    _i8 *next_token = NULL;

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

    printf("Please enter the GMT value of the Zone (HH:MM): \n");
    scanf_s("%s",GMT_value,6);

    token = (_i8 *)strtok_s((char *)GMT_value,":",(char **)&next_token);
    if(token == NULL)
        printf("Invalid Input \r\n");
        
    UserFunction.gmt_hr = (_i16)strtol((const char *)token, NULL, 10);

    token = (_i8 *)strtok_s(NULL, ":",(char **)&next_token);
    if(token == NULL)
        printf("Invalid Input \r\n");
    
    UserFunction.gmt_min = (_i16)strtol((const char *)token, NULL, 10);

    token = (_i8 *)strtok_s(NULL, ":",(char **)&next_token);
    if(token != NULL)
        printf("Invalid Input \r\n");

    if(UserFunction.gmt_hr < 0)
                UserFunction.gmt_min *= -1;
        

    return UserFunction;
}

/*!
    \brief Convert integer to ASCII in decimal base

    \param[in]      cNum - integer number to convert

    \param[OUT]     cString - output string

    \return         number of ASCII characters

    \warning
*/
static _u16 iToa(_i32 cNum, _u8 *cString)
{
    _u16 length = 0;
    _u8* ptr = NULL;
    _i32 uTemp = cNum;

    /* value 0 is a special case */
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';

        return length;
    }

    /* Find out the length of the number, in decimal base */
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    /* Do the actual formatting, right to left */
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = digits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}

/*!
    \brief Get the required data from the server.

    \param[in]      gmt_hr - GMT offset hours

    \param[in]      gmt_min - GMT offset minutes

    \return         0 on success, -ve otherwise

    \warning
*/
static _i32 getSNTPTime(_i16 gmt_hr, _i16 gmt_min)
{
    /*
                                NTP Packet Header:


           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                          Root  Delay                           |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                       Root  Dispersion                         |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                     Reference Identifier                       |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                                                                |
          |                    Reference Time-stamp (64)                    |
          |                                                                |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                                                                |
          |                    Originate Time-stamp (64)                    |
          |                                                                |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                                                                |
          |                     Receive Time-stamp (64)                     |
          |                                                                |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                                                                |
          |                     Transmit Time-stamp (64)                    |
          |                                                                |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                 Key Identifier (optional) (32)                 |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                                                                |
          |                                                                |
          |                 Message Digest (optional) (128)                |
          |                                                                |
          |                                                                |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
    
    SlSockAddrIn_t  LocalAddr;
    SlSockAddr_t Addr;

    _u8    dataBuf[MAX_BUF_SIZE];
    _i32     retVal = -1;
    _i16     AddrSize = 0;

    sl_Memset(dataBuf, 0, sizeof(dataBuf));
    dataBuf[0] = '\x1b';

    Addr.sa_family = AF_INET;
    /* the source port */
    Addr.sa_data[0] = 0x00;
    Addr.sa_data[1] = 0x7B;    /* 123 */
    Addr.sa_data[2] = (_u8)((appData.DestinationIP >> 24) & 0xff);
    Addr.sa_data[3] = (_u8)((appData.DestinationIP >> 16) & 0xff);
    Addr.sa_data[4] = (_u8)((appData.DestinationIP >> 8) & 0xff);
    Addr.sa_data[5] = (_u8) (appData.DestinationIP & 0xff);

    retVal = sl_SendTo(appData.SockID, dataBuf, sizeof(dataBuf), 0,
                     &Addr, sizeof(Addr));
    if (retVal != sizeof(dataBuf))
    {
        /* could not send SNTP request */
        printf("Device couldn't send SNTP request\r\n\r\n");
        ASSERT_ON_ERROR(SNTP_SEND_ERROR);
    }

    AddrSize = sizeof(SlSockAddrIn_t);
    LocalAddr.sin_family = SL_AF_INET;
    LocalAddr.sin_port = 0;
    LocalAddr.sin_addr.s_addr = 0;
    sl_Bind(appData.SockID,(SlSockAddr_t *)&LocalAddr, AddrSize);

    retVal = sl_RecvFrom(appData.SockID, dataBuf, sizeof(dataBuf), 0,
                       (SlSockAddr_t *)&LocalAddr,  (SlSocklen_t*)&AddrSize);
    if (retVal <= 0)
    {
        printf("Device couldn't receive time information \r\n");
        ASSERT_ON_ERROR(SNTP_RECV_ERROR);
    }

    if ((dataBuf[0] & 0x7) != 4)    /* expect only server response */
    {
        /* MODE is not server, abort */
        printf("Device is expecting response from server only!\r\n");
        ASSERT_ON_ERROR(SNTP_SERVER_RESPONSE_ERROR);
    }
    else
    {
        _u8 index;

        appData.elapsedSec = dataBuf[40];
        appData.elapsedSec <<= 8;
        appData.elapsedSec += dataBuf[41];
        appData.elapsedSec <<= 8;
        appData.elapsedSec += dataBuf[42];
        appData.elapsedSec <<= 8;
        appData.elapsedSec += dataBuf[43];

        appData.elapsedSec -= TIME2013;

        /* correct the time zone */
        appData.elapsedSec += (gmt_hr * SEC_IN_HOUR);
        appData.elapsedSec += (gmt_min * SEC_IN_MIN);

        appData.ccPtr = &appData.time[0];

        /* day */
        appData.sGeneralVar = appData.elapsedSec/SEC_IN_DAY;
        memcpy(appData.ccPtr, daysOfWeek2013[appData.sGeneralVar%7], 3);
        appData.ccPtr += 3;
        *appData.ccPtr++ = '\x20';

        /* month */
        appData.sGeneralVar %= 365;
        for (index = 0; index < 12; index++)
        {
            appData.sGeneralVar -= numOfDaysPerMonth[index];
            if (appData.sGeneralVar < 0)
                break;
        }

        memcpy(appData.ccPtr, monthOfYear[index], 3);
        appData.ccPtr += 3;
        *appData.ccPtr++ = '\x20';

        /* date */
        /* restore the day in current month*/
        appData.sGeneralVar += numOfDaysPerMonth[index];
        appData.ccLen = iToa(appData.sGeneralVar + 1, appData.ccPtr);
        appData.ccPtr += appData.ccLen;
        *appData.ccPtr++ = '\x20';

        /* year */
        /* number of days since beginning of 2013 */
        appData.uGeneralVar = appData.elapsedSec/SEC_IN_DAY;
        appData.uGeneralVar /= 365;
        appData.ccLen = iToa(YEAR2013 + appData.uGeneralVar , appData.ccPtr);
        appData.ccPtr += appData.ccLen;
        *appData.ccPtr++ = '\x20';

        /* time */
        appData.uGeneralVar = appData.elapsedSec%SEC_IN_DAY;
        /* number of seconds per hour */
        appData.uGeneralVar1 = appData.uGeneralVar%SEC_IN_HOUR;
        appData.uGeneralVar /= SEC_IN_HOUR;               /* number of hours */
        appData.ccLen = iToa(appData.uGeneralVar, appData.ccPtr);
        appData.ccPtr += appData.ccLen;
        *appData.ccPtr++ = ':';
        /* number of minutes per hour */
        appData.uGeneralVar = appData.uGeneralVar1/SEC_IN_MIN;
        /* number of seconds per minute */
        appData.uGeneralVar1 %= SEC_IN_MIN;
        appData.ccLen = iToa(appData.uGeneralVar, appData.ccPtr);
        appData.ccPtr += appData.ccLen;
        *appData.ccPtr++ = ':';
        appData.ccLen = iToa(appData.uGeneralVar1, appData.ccPtr);
        appData.ccPtr += appData.ccLen;
        *appData.ccPtr++ = '\x20';

        *appData.ccPtr++ = '\0';

        printf("\r\n Server %s",SNTPserver);
        printf("has responded with time information\r\n\r\n");
        printf("%s\r\n\r\n",appData.time);
    }

    return SUCCESS;
}

/*!
    \brief Create UDP socket to communicate with server.

    \param[in]      none

    \return         Socket descriptor for success otherwise negative

    \warning
*/
static _i16 createConnection()
{
    _i16 sd = 0;

    sd = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, IPPROTO_UDP);
    ASSERT_ON_ERROR(sd);

    return sd;
}

/*!
    \brief Gets the Server IP address

    \param[in]      none

    \return         zero for success and -1 for error

    \warning
*/
static _i32 getHostIP()
{
    _i32 status = 0;
    appData.DestinationIP = 0;

    status = sl_NetAppDnsGetHostByName((_i8*)SNTPserver, strlen((const char *)SNTPserver),
                                       &appData.DestinationIP, SL_AF_INET);
    ASSERT_ON_ERROR(status);

    return SUCCESS;
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
    \brief Disconnecting from a WLAN Access point

    This function disconnects from the connected AP

    \param[in]      None

    \return         none

    \note

    \warning        If the WLAN disconnection fails, we will be stuck in this function forever.
*/
static _i32 disconnectFromAP()
{
    _i32 retVal = -1;

    /*
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
    retVal = sl_WlanDisconnect();
    if(0 == retVal)
    {
        /* Wait */
        while(IS_CONNECTED(g_Status));
    }

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
    printf("\r\n\r\n");
    printf("Get time application - Version \r\n");
    printf(APPLICATION_VERSION);
    printf("\r\n*******************************************************************************\r\n");
}
