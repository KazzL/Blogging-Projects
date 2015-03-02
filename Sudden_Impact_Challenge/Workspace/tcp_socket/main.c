/*
 * main.c - TCP socket sample application
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
 * Application Name     -   TCP socket
 * Application Overview -   This is a sample application demonstrating how to open
 *                          and use a standard TCP socket with CC3100.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_TCP_Socket_Application
 *                          doc\examples\tcp_socket.pdf
 */


#include "simplelink.h"
#include "sl_common.h"
#include "spi.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

/* IP addressed of server side socket. Should be in long format,
 * E.g: 0xc0a8010a == 192.168.1.10 */
#define IP_ADDR         0xC0A80212
#define PORT_NUM        5001            /* Port number to be used */

#define BUF_SIZE        1400
#define NO_OF_PACKETS   10

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    TCP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    TCP_RECV_ERROR = TCP_SEND_ERROR -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

/*
 * GLOBAL VARIABLES -- Start
 */
union
{
    _u8 BsdBuf[BUF_SIZE];
    _u32 demobuf[BUF_SIZE/4];
} uBuf;

_u8 g_Status = 0;
/****************************************CARRIED OVER START*/
unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;
unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
unsigned char RxBuffer[128];       			// Allocate 128 byte of RAM
char multiple;
char stopBit = FALSE;
/****************************************CARRIED OVER END*/
char xValue[] = "\0\0\0\0\0\0";
unsigned char errorArray[] = "\0\0\0\0\0\0\0\0\0\0";
int connectedToAP = 0;
int connetcedToSocket = 0;
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
void clearErrorArray();
static _i32 configureSimpleLinkToDefaultState();
static _i32 establishConnectionWithAP();
void configureCC3x00();
void startCC3x00();
void connectCC3x00();
static _i32 initializeAppVariables();
static _i32 BsdTcpClient(_u16 Port);
static _i16 packetTx(_u16 socketID, unsigned char* message, int length, char closeSocket);

static void displayBanner();
int intToChars(int number, char* chars);
int charToInt(signed char value);
void hexByteToChar(char byteIn, char* nibbleA, char* nibbleB);
int plotlyPlotString(int xVal, int yVal, char* dataStream);
void I2CInit();
void I2CTx(unsigned char* TxData, unsigned char numBytes, char stop);
void I2CRx(unsigned char* RxBuffer, unsigned char numBytes);
/*
 * STATIC FUNCTION DEFINITIONS -- End
 */

void clearErrorArray(){
	char i;
	for(i = 0; i < 12; i++){
		errorArray[i] = '\0';
	}
}

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
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)//TODO: Look at all the possible events that can occur and decide if other events need to be handled
{
    if(pWlanEvent == NULL)
        CLI_Write(" [WLAN EVENT] NULL Pointer Error \n\r");

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
        	SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
            intToChars(pWlanEvent->Event, errorArray);
        	CLI_Write(" STATUS_BIT_CONNECTION Value: ");
        	CLI_Write(errorArray);
        	CLI_Write("\r\n\r\n");
        	clearErrorArray();
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
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)//TODO: Add connection flag here to be checked in main loop and reconnect should a connection error occur
            {
                CLI_Write(" Device disconnected from the AP on application's request \n\r");

            	intToChars(pWlanEvent->Event, errorArray);
      				CLI_Write(" Error Code A.0: ");
      				CLI_Write(errorArray);
      				CLI_Write("\r\n\r\n");
      				clearErrorArray();
      				connectedToAP = 0;
      				intToChars(pEventData->reason_code, errorArray);
      				CLI_Write(" Error Code A.1: ");
      				CLI_Write(errorArray);
      				CLI_Write("\r\n\r\n");
      				clearErrorArray();
            }
            else
            {
                CLI_Write(" Device disconnected from the AP on an ERROR..!! \n\r");

              	connectedToAP = 0;
        				intToChars(pWlanEvent->Event, errorArray);
        				CLI_Write(" Error Code A.2: ");
        				CLI_Write(errorArray);
        				CLI_Write("\r\n\r\n");
        				clearErrorArray();
        				connectedToAP = 0;
        				intToChars(pEventData->reason_code, errorArray);
        				CLI_Write(" Error Code A.3: ");
        				CLI_Write(errorArray);
        				CLI_Write("\r\n\r\n");
        				clearErrorArray();
            }
        }
        break;

        default:
        {
            CLI_Write(" [WLAN EVENT] Unexpected event \n\r");

        	connectedToAP = 0;
    			intToChars(pWlanEvent->Event, errorArray);
    			CLI_Write(" Error Code B: ");
    			CLI_Write(errorArray);
    			CLI_Write("\r\n\r\n");
    			clearErrorArray();
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

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
            intToChars(pNetAppEvent->Event, errorArray);
          	CLI_Write(" Event Value: ");
          	CLI_Write(errorArray);
          	CLI_Write("\r\n\r\n");
          	clearErrorArray();


        	intToChars(STATUS_BIT_IP_ACQUIRED, errorArray);
			CLI_Write(" STATUS_BIT_IP_AQUIRED Value: ");
			CLI_Write(errorArray);
			CLI_Write("\r\n\r\n");
			clearErrorArray();
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
            CLI_Write(" [NETAPP EVENT] Unexpected event \n\r");
            intToChars(pNetAppEvent->Event, errorArray);
            CLI_Write(" Error Code C: ");
            CLI_Write(errorArray);
            CLI_Write("\r\n\r\n");
            clearErrorArray();
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
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
    CLI_Write(" [HTTP EVENT] Unexpected event \n\r");
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
  	intToChars(pDevEvent->Event, errorArray);
  	CLI_Write(" Error Code D: ");
  	CLI_Write(errorArray);
  	CLI_Write("\r\n\r\n");
  	clearErrorArray();
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler

    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
        CLI_Write(" [SOCK EVENT] NULL Pointer Error \n\r");

    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
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
                    CLI_Write(" [SOCK EVENT] Close socket operation, failed to transmit all queued packets\n\r");
                    connetcedToSocket = 0;
          					intToChars(pSock->Event, errorArray);
          					CLI_Write(" Error Code E: ");
          					CLI_Write(errorArray);
          					CLI_Write("\r\n\r\n");
          					clearErrorArray();
          					break;


                default:
                    CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
          					connetcedToSocket = 0;
          					intToChars(pSock->Event, errorArray);
          					CLI_Write(" Error Code F: ");
          					CLI_Write(errorArray);
          					CLI_Write("\r\n\r\n");
          					clearErrorArray();
          					break;
          			}
          			break;

		default:
			CLI_Write(" [SOCK EVENT] Unexpected event 2\n\r");
			connetcedToSocket = 0;
			intToChars(pSock->Event, errorArray);
			CLI_Write(" Error Code G: ");
			CLI_Write(errorArray);
			CLI_Write("\r\n\r\n");
			clearErrorArray();
			break;
	}
}
/*
 * ASYNCHRONOUS EVENT HANDLERS -- End
 */


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
    \brief Connecting to a WLAN Access point

    This function connects to the required AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]  None

    \return     0 on success, negative error-code on error

    \note

    \warning    If the WLAN connection fails or we don't acquire an IP address,
                We will be stuck in this function forever.
*/
static _i32 establishConnectionWithAP()
{
    SlSecParams_t secParams = {0};
    _i32 retVal = 0;

    secParams.Key = PASSKEY;
    secParams.KeyLen = pal_Strlen(PASSKEY);
    secParams.Type = SEC_TYPE;

    retVal = sl_WlanConnect(SSID_NAME, pal_Strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(retVal);

    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status))) { _SlNonOsMainLoopTask(); }

    connectedToAP = 1;//TODO: remove flag
    return SUCCESS;
}

/****************************************************************************************************************************************/

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
void configureCC3x00(){
	_i32 retVal = -1;
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
}

/*
 * Assumption is that the device is configured in station mode already
 * and it is in its default state
 */
/* Initializing the CC3100 device */
void startCC3x00(){
	_i32 retVal = -1;
	retVal = sl_Start(0, 0, 0);
	if ((retVal < 0) ||	(ROLE_STA != retVal) )
	{
		CLI_Write(" Failed to start the device \n\r");
		LOOP_FOREVER();
	}
	CLI_Write(" Device started as STATION \n\r");
}


/* Connecting to WLAN AP - Set with static parameters defined at the top
	After this call we will be connected and have IP address */
void connectCC3x00(){
	_i32 retVal = -1;
	retVal = establishConnectionWithAP();
	if(retVal < 0)
	{
		CLI_Write(" Failed to establish connection w/ an AP \n\r");
		LOOP_FOREVER();
	}

	CLI_Write(" Connection established w/ AP and IP is acquired \n\r");
}

/*******************************************************************************************************************************************/

/*!
    \brief Opening a client side socket and sending data

    This function opens a TCP socket and tries to connect to a Server IP_ADDR
    waiting on port PORT_NUM. If the socket connection is successful then the
    function will send 1000 TCP packets to the server.

    \param[in]      port number on which the server will be listening on

    \return         0 on success, -1 on Error.

    \note

    \warning        A server must be waiting with an open TCP socket on the
                    right port number before calling this function.
                    Otherwise the socket creation will fail.
*/
static _i32 BsdTcpClient(_u16 Port)//<- Verify Return type is correct (SocketID)
{
    SlSockAddrIn_t  Addr;

    _u16          idx = 0;
    _u16          AddrSize = 0;
    _i16          SockID = 0;
    _i16          Status = 0;


    for (idx=0 ; idx<BUF_SIZE ; idx++)
    {
        uBuf.BsdBuf[idx] = (_u8)(idx % 10);
    }

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons((_u16)Port);
    Addr.sin_addr.s_addr = sl_Htonl((_u32)IP_ADDR);//TODO: Change this as a passed in value(Keep as defined for current version)
    AddrSize = sizeof(SlSockAddrIn_t);

    SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( SockID < 0 )
    {
        CLI_Write(" [TCP Client] Create socket Error \n\r");
        ASSERT_ON_ERROR(SockID);
    }

    CLI_Write(" Establishing connection with TCP server \n\r");

    Status = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, AddrSize);
    if( Status < 0 )
    {
        sl_Close(SockID);
        CLI_Write(" [TCP Client]  TCP connection Error \n\r");//" Failed to establish connection with TCP server \n\r"
        ASSERT_ON_ERROR(Status);
    }

    CLI_Write(" Connection with TCP server established successfully \n\r");
    connetcedToSocket = 1;//TODO: remove flag
    return  SockID;
}

static _i16 packetTx(_u16 socketID, unsigned char* message, int length, char closeSocket){
    _i16           Status = 0;
    _i8 			sent = -1;
    _i8 			i = 0;
    _i16 			j = 0;

    CLI_Write(" - Initiating message transmit\n\r");
	  while((sent == -1) && (i < 6)){
		Status = sl_Send(socketID, message, length, 0 );
		sent = Status;
		i++;
		if (sent != 0){
			for (j = 0; j < 1000; j++);
		}
	}
    if( Status < 0 )
	{
		CLI_Write(" Error while sending the data \n\r");
		sl_Close(socketID);
		return -1;
	}
	CLI_Write(" - Message transmitted\n\r");

////    if (closeSocket == 1){
//    	sl_Close(socketID);//TODO: Remove close socket
////    }
    return 0;
}

/*!
    \brief This function displays the application's banner

    \param      None

    \return     None
*/
static void displayBanner()
{
    CLI_Write("\n\r\n\r");
    CLI_Write(" TCP socket application - Version ");
    CLI_Write(APPLICATION_VERSION);
    CLI_Write("\n\r*******************************************************************************\n\r");
}

int intToChars(int number, char* chars){
	int val = number;
	int i = 0;
	int j = 0;
	int num;
	int neg = 0;
	char temp;
	if (val < 0){
		val = 0 - val;
		neg = 1;
	}

	if (val == 0){
		chars[0] = '0';
		i = 1;
	}

	while(val != 0){
		num = val/10;
		chars[i] = (char)(val - (num * 10) + 0x30);
		val = num;
		i++;
	}
	i--;//Remove extra count
	int f;
	if (i % 2 != 0){
		f = i / 2 + 1;
	}
	else {
		f = i / 2;
	}
	for (j = 0; j < f; j++){
		temp = chars[j];
		chars[j] = chars[i - j];
		chars[i - j] = temp;
	}
	if (neg){
		for (j = i; j >= 0; j--){
			chars[j + 1] = chars[j];
		}
		chars[0] = '-';
		i++;//increase count to account for negitive sign
	}
	return i + 1; //Counting from zero so add one for true length
}

//Converts Hex representation to characters
int charToInt(signed char value){
	int mambo = 0;
	if (value < 0){
		value = 0 - value;
		mambo = 0 - (int)value;
	}
	else{
		mambo = (int)value;
	}
	return mambo;
}

void hexByteToChar(char byteIn, char* nibbleA, char* nibbleB)
{
	switch(byteIn & 0xF0){
		case 0x0:
			*nibbleA = '0';
			break;
		case 0x10:
			*nibbleA = '1';
			break;
		case 0x20:
			*nibbleA = '2';
			break;
		case 0x30:
			*nibbleA = '3';
			break;
		case 0x40:
			*nibbleA = '4';
			break;
		case 0x50:
			*nibbleA = '5';
			break;
		case 0x60:
			*nibbleA = '6';
			break;
		case 0x70:
			*nibbleA = '7';
			break;
		case 0x80:
			*nibbleA = '8';
			break;
		case 0x90:
			*nibbleA = '9';
			break;
		case 0xA0:
			*nibbleA = 'A';
			break;
		case 0xB0:
			*nibbleA = 'B';
			break;
		case 0xC0:
			*nibbleA = 'C';
			break;
		case 0xD0:
			*nibbleA = 'D';
			break;
		case 0xE0:
			*nibbleA = 'E';
			break;
		case 0xF0:
			*nibbleA = 'F';
			break;
	}
	switch (byteIn & 0x0F){
		case 0x00:
			*nibbleB = '0';
			break;
		case 0x01:
			*nibbleB = '1';
			break;
		case 0x02:
			*nibbleB = '2';
			break;
		case 0x03:
			*nibbleB = '3';
			break;
		case 0x04:
			*nibbleB = '4';
			break;
		case 0x05:
			*nibbleB = '5';
			break;
		case 0x06:
			*nibbleB = '6';
			break;
		case 0x07:
			*nibbleB = '7';
			break;
		case 0x08:
			*nibbleB = '8';
			break;
		case 0x09:
			*nibbleB = '9';
			break;
		case 0x0A:
			*nibbleB = 'A';
			break;
		case 0x0B:
			*nibbleB = 'B';
			break;
		case 0x0C:
			*nibbleB = 'C';
			break;
		case 0x0D:
			*nibbleB = 'D';
			break;
		case 0x0E:
			*nibbleB = 'E';
			break;
		case 0x0F:
			*nibbleB = 'F';
			break;
	}
}


int plotlyPlotString(int xVal, int yVal, char* dataStream){
	const char start[] = "\r\n{\"x\":";
	const char middle[] = ",\"y\":";
	const char end[] = "}\r\n";

//	char xValue[] = "\0\0\0\0\0\0";
	char yValue[] = "\0\0\0\0\0\0";

	intToChars(xVal, xValue);
	intToChars(yVal, yValue);

	int offset = 4;
	strncpy(dataStream + offset, start, sizeof(start) - 1);
	offset = offset + sizeof(start) - 1;
	strncpy(dataStream + offset, xValue, strlen(xValue));
	offset = offset + strlen(xValue);
	strncpy(dataStream + offset, middle, sizeof(middle) - 1);
	offset = offset + sizeof(middle) - 1;
	strncpy(dataStream + offset, yValue, strlen(yValue));
	offset = offset + strlen(yValue);
	strncpy(dataStream + offset, end, sizeof(end)); //Leave out the -1 inorder to include the null terminator

	int streamLength;
	char streamDataLen[] = "\0\0\0\0\0";

	streamLength = sizeof(start) - 2 + strlen(xValue) + sizeof(middle) + strlen(yValue) + sizeof(end) - 3;
	intToChars(streamLength, streamDataLen);
	char happyDay[4];
	hexByteToChar(streamDataLen[0], happyDay, happyDay + 1);
	hexByteToChar(streamDataLen[1], happyDay + 2, happyDay + 3);
	strncpy(dataStream, happyDay, 4);

	return streamLength;
}
/*!
    \brief This function initializes the application variables

    \param[in]  None

    \return     0 on success, negative error-code on error
*/
static _i32 initializeAppVariables()
{
    g_Status = 0;
    pal_Memset(uBuf.BsdBuf, 0, sizeof(uBuf));

    return SUCCESS;
}


void I2CInit(){
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P4SEL |= 0x06;                            // Assign I2C pins to USCI_B0
  UCB1CTL1 |= UCSWRST;                      // Enable SW reset
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB1BR0 = 10;                             // fSCL = SMCLK/12 = ~100kHz
  UCB1BR1 = 0;
  UCB1I2CSA = 0x60;                         // Slave Address is 048h
  UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB1IE |= UCTXIE;                         // Enable TX interrupt
  UCB1IE |= UCRXIE;                         // Enable RX interrupt
}

void I2CTx(unsigned char* TxData, unsigned char numBytes, char stop){
    unsigned int i;
    stopBit = stop;
	for(i=0;i<10;i++);                      // Delay required between transaction
    PTxData = (unsigned char *)TxData;      // TX array start address
                                            // Place breakpoint here to see each
                                            // transmit operation.
    TXByteCtr = numBytes;              // Load TX byte counter

    UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition

    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0, enable interrupts
    __no_operation();                       // Remain in LPM0 until all data

    if(stop){
    	UCB1CTL1 |= UCTXSTP;
    }
                                            // is TX'd
    while (UCB1CTL1 & UCTXSTP);             // Ensure stop condition got sent
}

void I2CRx(unsigned char* RxBuffer, unsigned char numBytes){
	PRxData = (unsigned char *)RxBuffer;    // Start of RX buffer
    RXByteCtr = numBytes;                   // Load RX byte counter
    if (numBytes > 1){
    	multiple = 1;
    }
    else {
    	multiple = 0;
    }
    while (UCB1CTL1 & UCTXSTP);           // Ensure stop condition got sent

    UCB1CTL1 &= ~UCTR;
    UCB1CTL1 |= UCTXSTT;                    // I2C start condition

    if (multiple == 0){
		while(UCB1CTL1 & UCTXSTT);          // Start condition sent?
		UCB1CTL1 |= UCTXSTP;
    }
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0, enable interrupts
                                            // Remain in LPM0 until all data is RX'd
    __no_operation();
}



int mpl3115aRead(){
	unsigned char OUT_P_MSB[] = {0x01};
	unsigned char DR_STATUS[] = {0x06};

	char readPresTemp = 0;

	I2CTx(DR_STATUS, sizeof(DR_STATUS), FALSE);
	I2CRx(RxBuffer, 1);
	if ((RxBuffer[0] == 0x0E) || (RxBuffer[0] == 0xEE)){
		P4OUT |= 0x80;
		P1OUT &= 0xFE;
		readPresTemp = 1;
	}
	else {
		P1OUT |= 0x01;
		P4OUT &= 0x7F;
		readPresTemp = 0;
	}

	if (readPresTemp){
		I2CTx(OUT_P_MSB, sizeof(OUT_P_MSB), FALSE);
		I2CRx(RxBuffer, 5);
		Delay(50);
	}
	return readPresTemp;
}


/*
 * Application's entry point
 */
int main(int argc, char** argv)
{
    _i32 retVal = -1;

  	int temperatureInt;
  	int streamLength;
  	int packetLength;
  	char dataStream[30];
  	int xVal = 0;

  	P1OUT &= 0xFE;
   	P1DIR |= 0x01;
  	P4OUT &= 0x7F;
  	P4DIR |= 0x80;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    /* Stop WDT and initialize the system-clock of the MCU
       These functions needs to be implemented in PAL */
    stopWDT();
    initClk();

    /* Configure command line interface */
    CLI_Configure();
    CLI_Write(" \f\r\n");
    displayBanner();

/*****************************************************TEST FUNCTIONS START*******************************/
	configureCC3x00();
	startCC3x00();
	connectCC3x00();
/*****************************************************TEST FUNCTIONS END*******************************/

 	_i16 socketIDD;
 	socketIDD = BsdTcpClient(PORT_NUM);

//	unsigned char PLOTLY[] = 	 "POST /clientresp HTTP/1.1\r\nHost: 107.21.214.199\r\nUser-Agent: MSP430F5529/0.5.1\r\nContent-Length: 261\r\n\r\nversion=2.2&origin=plot&platform=Stellaris&un=Kas&key=123456789k&args=[{\"y\": [], \"x\": [], \"type\": \"scatter\", \"stream\": {\"token\": \"ABCDEFGHI\", \"maxpoints\": 7200}}]&kwargs={\"fileopt\": \"overwrite\", \"filename\": \"Element14 Temperature Plot\", \"world_readable\": true}\r\n";
// 	unsigned char initPlotly[] = "POST /clientresp HTTP/1.1\r\nHost: 107.21.214.199\r\nUser-Agent: MSP430F5529/0.5.1\r\nContent-Length: 243\r\n\r\n";
//  unsigned char initStream[] = "version=2.2&origin=plot&platform=Stellaris&un=Kas&key=123456789&args=[{\"y\": [], \"x\": [], \"type\": \"scatter\", \"stream\": {\"token\": \"ABCDEFGHI\", \"maxpoints\": 500}}]&kwargs={\"fileopt\": \"overwrite\", \"filename\": \"test plot\", \"world_readable\": true}\r\n\r\n\r\n\r\n\r\n";
    unsigned char openStream[] = "POST / HTTP/1.1\r\nHost: stream.plot.ly\r\nUser-Agent: Python\r\nTransfer-Encoding: chunked\r\nConnection: keep-alive\r\nplotly-streamtoken: ABCDEFGHI\r\n\r\n";

 	packetTx(socketIDD, openStream, (sizeof(openStream) - 1), 0);

 	int dataAvilable;

 	CLI_Write(" Initilizing SPI");
 	int spi_Init();
 	Delay(700);
 	CLI_Write(" Complete \r\n");
 	unsigned char hello[] = {0x03};
 	spi_Device_Write(hello, 1, 0);

 	Delay(500);
 	CLI_Write(" Initilizing MPL3115A");
 	Delay(700);
 	CLI_Write(" Complete\r\n");
	while (1)
  {
		dataAvilable = mpl3115aRead();
		if (dataAvilable == 1){

			temperatureInt = charToInt((signed char)RxBuffer[3]);
			streamLength = plotlyPlotString(xVal, temperatureInt, dataStream);
			packetLength = streamLength + 6;
			packetTx(socketIDD, dataStream, packetLength, 0);

			CLI_Write("\r\n");
			CLI_Write(dataStream);
			CLI_Write("\r\n");

			xVal++;
		}
		Delay(20);
  }
}






//-------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C slave
// to the MSP430 memory. It is structured such that it can be used to receive
// any 2+ number of bytes by pre-loading RXByteCtr with the byte count.
//-------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: break;                           // Vector  4: NACKIFG
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10:                                  // Vector 10: RXIFG
    RXByteCtr--;                            // Decrement RX byte counter
    if (RXByteCtr)
    {
      *PRxData++ = UCB1RXBUF;               // Move RX data to address PRxData
      if (RXByteCtr == 1 && multiple == 1)                   // Only one byte left?
        UCB1CTL1 |= UCTXSTP;                // Generate I2C stop condition
    }
    else
    {
      *PRxData = UCB1RXBUF;                 // Move final RX data to PRxData
      __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
    }
    break;
  case 12:                                  // Vector 12: TXIFG
    if (TXByteCtr)                          // Check TX byte counter
    {
      UCB1TXBUF = *PTxData++;               // Load TX buffer
      TXByteCtr--;                          // Decrement TX byte counter
    }
    else
    {
    if (stopBit){
      UCB1CTL1 |= UCTXSTP;                  // I2C stop condition						//Set a stop flag if this should be sent out or not
    }
      UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
    }
  default: break;
  }
}
