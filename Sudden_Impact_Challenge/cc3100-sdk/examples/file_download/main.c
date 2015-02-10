/*
 * main.c - file download sample application
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
 * Application Name     -   File download
 * Application Overview -   This sample application demonstrates how to
 *                          connect w/ a server, download a file and save it
 *                          on the serial-flash. The application developers
 *                          can use this scheme to update files over the network
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_File_Download_Application
 *                          doc\examples\file_download.pdf
 */

#include "simplelink.h"
#include "sl_common.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

#define PREFIX_BUFFER   "GET /lit/er/swrz044b/swrz044b.pdf"
#define POST_BUFFER     " HTTP/1.1\nHost:www.ti.com\nAccept: text/html, application/xhtml+xml, */*\n\n"
#define HOST_NAME       "www.ti.com"

#define HTTP_FILE_NOT_FOUND    "404 Not Found" /* HTTP file not found response */
#define HTTP_STATUS_OK         "200 OK"  /* HTTP status ok response */
#define HTTP_CONTENT_LENGTH    "Content-Length:"  /* HTTP content length header */
#define HTTP_TRANSFER_ENCODING "Transfer-Encoding:" /* HTTP transfer encoding header */
#define HTTP_ENCODING_CHUNKED  "chunked" /* HTTP transfer encoding header value */
#define HTTP_CONNECTION        "Connection:" /* HTTP Connection header */
#define HTTP_CONNECTION_CLOSE  "close"  /* HTTP Connection header value */

#define HTTP_END_OF_HEADER  "\r\n\r\n"  /* string marking the end of headers in response */

#define SIZE_45K        46080  /* Serial flash file size 45 KB */
#define READ_SIZE       1450
#define MAX_BUFF_SIZE   1460
#define SPACE           32

/* File on the serial flash */
#define FILE_NAME "cc3000_module.pdf"

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap with host-driver's error codes */
    INVALID_HEX_STRING = DEVICE_NOT_IN_STATION_MODE - 1,
    TCP_RECV_ERROR = INVALID_HEX_STRING - 1,
    TCP_SEND_ERROR = TCP_RECV_ERROR - 1,
    FILE_NOT_FOUND_ERROR = TCP_SEND_ERROR - 1,
    INVALID_SERVER_RESPONSE = FILE_NOT_FOUND_ERROR - 1,
    FORMAT_NOT_SUPPORTED = INVALID_SERVER_RESPONSE - 1,
    FILE_WRITE_ERROR = FORMAT_NOT_SUPPORTED - 1,
    INVALID_FILE = FILE_WRITE_ERROR - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

/*
 * GLOBAL VARIABLES -- Start
 */
_u32 g_Status;
_u32 g_DestinationIP;
_u32 g_BytesReceived; /* variable to store the file size */
_u8  g_buff[MAX_BUFF_SIZE+1];

_i32 g_SockID = 0;

/*
 * GLOBAL VARIABLES -- End
 */


/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 establishConnectionWithAP();
static _i32 configureSimpleLinkToDefaultState();

static _i32 initializeAppVariables();
static void  displayBanner();

static _i32 createConnection(_u32 DestinationIP);

static _i32 getChunkSize(_i32 *len, _u8 **p_Buff, _u32 *chunk_size);
static _i32 hexToi(_u8 *ptr);
static _i32 getFile();
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

            /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                CLI_Write(" Device disconnected from the AP on application's request \n\r");
            }
            else
            {
                CLI_Write(" Device disconnected from the AP on an ERROR..!! \n\r");
            }
        }
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
            CLI_Write(" [NETAPP EVENT] Unexpected event \n\r");
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
                    CLI_Write(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\n\r");
                break;


                default:
                    CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
                break;
            }
        }
        break;

        default:
            CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
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
    _i32 retVal = -1;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    /* Stop WDT and initialize the system-clock of the MCU */
    stopWDT();
    initClk();

    /* Configure command line interface */
    CLI_Configure();

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
    retVal = configureSimpleLinkToDefaultState();
    if(retVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == retVal)
            CLI_Write(" Failed to configure the device in its default state \n\r");

        LOOP_FOREVER();
    }

    CLI_Write(" Device is configured in default state \n\r");

    /*
     * Initializing the CC3100 device
     * Assumption is that the device is configured in station mode already
     * and it is in its default state
     */
    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        CLI_Write(" Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    CLI_Write(" Device started as STATION \n\r");

    /* Connecting to WLAN AP */
    retVal = establishConnectionWithAP();
    if(retVal < 0)
    {
        CLI_Write(" Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    CLI_Write(" Connection established w/ AP and IP is acquired \n\r");

    retVal = sl_NetAppDnsGetHostByName(HOST_NAME, pal_Strlen(HOST_NAME),
                                       &g_DestinationIP, SL_AF_INET);
    if(retVal < 0)
    {
        CLI_Write(" Device couldn't get the IP for the host-name\r\n");
        LOOP_FOREVER();
    }

    /* Create a TCP connection to the Web Server */
    g_SockID = createConnection(g_DestinationIP);
    if(g_SockID < 0)
    {
        CLI_Write(" Device couldn't connect to the server\r\n");
        LOOP_FOREVER();
    }

    CLI_Write(" Successfully connected to the server \r\n");

    /* Download the file, verify the file and replace the exiting file */
    retVal = getFile();
    if(retVal < 0)
    {
        CLI_Write(" Device couldn't download the file from the server \r\n");
        LOOP_FOREVER();
    }

    CLI_Write(" File downloaded successfully \r\n");

    retVal = sl_Close(g_SockID);
    if(retVal < 0)
    {
        CLI_Write(" Socket couldn't be closed\r\n");
        LOOP_FOREVER();
    }

    /* Stop the CC3100 device */
    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if(retVal < 0)
    {
        CLI_Write(" Device couldn't be stopped \r\n");
        LOOP_FOREVER();
    }

    return 0;
}


/*!
    \brief Create connection with server.

    This function opens a socket and create the endpoint communication with server

    \param[in]      DestinationIP - IP address of the server

    \return         socket id for success and negative for error
*/
static _i32 createConnection(_u32 DestinationIP)
{
    SlSockAddrIn_t  Addr = {0};
    _i32           Status = 0;
    _i32           AddrSize = 0;
    _i32           SockID = 0;

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(80);
    Addr.sin_addr.s_addr = sl_Htonl(DestinationIP);

    AddrSize = sizeof(SlSockAddrIn_t);

    SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    ASSERT_ON_ERROR(SockID);

    Status = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, AddrSize);
    if (Status < 0)
    {
        sl_Close(SockID);
        ASSERT_ON_ERROR(Status);
    }

    return SockID;
}

/*!
    \brief Convert hex to decimal base

    \param[in]      ptr - pointer to string containing number in hex

    \return         number in decimal base

*/
static _i32 hexToi(_u8 *ptr)
{
    _u32 result = 0;
    _u32 len = 0;

    _i32  idx = -1;

    len = pal_Strlen(ptr);

    /* convert characters to upper case */
    for(idx = 0; ptr[idx] != '\0'; ++idx)
    {
        if( (ptr[idx] >= 'a') &&
            (ptr[idx] <= 'f') )
        {
            ptr[idx] -= 32;         /* Change case - ASCII 'a' = 97, 'A' = 65 => 97-65 = 32 */
        }
    }

    for(idx = 0; ptr[idx] != '\0'; ++idx)
    {
        if(ptr[idx] >= '0' && ptr[idx] <= '9')
        {
            /* Converting '0' to '9' to their decimal value */
            result += (ptr[idx] - '0') * (1 << (4 * (len - 1 - idx)));
        }
        else if(ptr[idx] >= 'A' && ptr[idx] <= 'F')
        {
            /* Converting hex 'A' to 'F' to their decimal value */
            result += (ptr[idx] - 55) * (1 << (4 * (len -1 - idx))); /* .i.e. 'A' - 55 = 10, 'F' - 55 = 15 */
        }
        else
        {
            ASSERT_ON_ERROR(INVALID_HEX_STRING);
        }
    }

    return result;
}

/*!
    \brief Calculate the file chunk size

    \param[in]      len - pointer to length of the data in the buffer
    \param[in]      p_Buff - pointer to pointer of buffer containing data
    \param[out]     chunk_size - pointer to variable containing chunk size

    \return         0 for success, -ve for error

*/
static _i32 getChunkSize(_i32 *len, _u8 **p_Buff, _u32 *chunk_size)
{
    _i32   idx = -1;
    _u8   lenBuff[10];

    idx = 0;
    pal_Memset(lenBuff, 0, sizeof(lenBuff));
    while(*len >= 0 && **p_Buff != 13) /* check for <CR> */
    {
        if(0 == *len)
        {
            pal_Memset(g_buff, 0, sizeof(g_buff));
            *len = sl_Recv(g_SockID, &g_buff[0], MAX_BUFF_SIZE, 0);
            if(*len <= 0)
                ASSERT_ON_ERROR(TCP_RECV_ERROR);

            *p_Buff = g_buff;
        }

        lenBuff[idx] = **p_Buff;
        idx++;
        (*p_Buff)++;
        (*len)--;
    }

    (*p_Buff) += 2; /* skip <CR><LF> */
    (*len) -= 2;
    *chunk_size = hexToi(lenBuff);

    return SUCCESS;
}

/*!
    \brief Obtain the file from the server

    This function requests the file from the server and save it on serial flash.
    To request a different file for different user needs to modify the
    PREFIX_BUFFER and POST_BUFFER macros.

    \param[in]      None

    \return         0 for success and negative for error

*/
static _i32 getFile()
{
    _u32 Token = 0;
    _u32 recv_size = 0;
    _u8 *pBuff = 0;
    _u8 eof_detected = 0;
    _u8 isChunked = 0;

    _i32 transfer_len = -1;
    _i32 retVal = -1;
    _i32 fileHandle = -1;

    pal_Memset(g_buff, 0, sizeof(g_buff));

    /*Puts together the HTTP GET string.*/
    pal_Strcpy(g_buff, PREFIX_BUFFER);
    pal_Strcat(g_buff, POST_BUFFER);

    /*Send the HTTP GET string to the opened TCP/IP socket.*/
    transfer_len = sl_Send(g_SockID, g_buff, pal_Strlen(g_buff), 0);

    if (transfer_len < 0)
    {
        /* error */
        CLI_Write(" Socket Send Error\r\n");
        ASSERT_ON_ERROR(TCP_SEND_ERROR);
    }

    pal_Memset(g_buff, 0, sizeof(g_buff));

    /*get the reply from the server in buffer.*/
    transfer_len = sl_Recv(g_SockID, &g_buff[0], MAX_BUFF_SIZE, 0);

    if(transfer_len <= 0)
        ASSERT_ON_ERROR(TCP_RECV_ERROR);

    /* Check for 404 return code */
    if(pal_Strstr(g_buff, HTTP_FILE_NOT_FOUND) != 0)
    {
        CLI_Write(" File not found, check the file and try again\r\n");
        ASSERT_ON_ERROR(FILE_NOT_FOUND_ERROR);
    }

    /* if not "200 OK" return error */
    if(pal_Strstr(g_buff, HTTP_STATUS_OK) == 0)
    {
        CLI_Write(" Error during downloading the file\r\n");
        ASSERT_ON_ERROR(INVALID_SERVER_RESPONSE);
    }

    /* check if content length is transferred with headers */
    pBuff = (_u8 *)pal_Strstr(g_buff, HTTP_CONTENT_LENGTH);
    if(pBuff != 0)
    {
        /* not supported */
        CLI_Write(" Server response format is not supported\r\n");
        ASSERT_ON_ERROR(FORMAT_NOT_SUPPORTED);
    }

    /* Check if data is chunked */
    pBuff = (_u8 *)pal_Strstr(g_buff, HTTP_TRANSFER_ENCODING);
    if(pBuff != 0)
    {
        pBuff += pal_Strlen(HTTP_TRANSFER_ENCODING);
        while(*pBuff == SPACE)
            pBuff++;

        if(pal_Memcmp(pBuff, HTTP_ENCODING_CHUNKED, pal_Strlen(HTTP_ENCODING_CHUNKED)) == 0)
        {
            recv_size = 0;
            isChunked = 1;
        }
    }
    else
    {
        /* Check if connection will be closed by after sending data
         * In this method the content length is not received and end of
         * connection marks the end of data */
        pBuff = (_u8 *)pal_Strstr(g_buff, HTTP_CONNECTION);
        if(pBuff != 0)
        {
            pBuff += pal_Strlen(HTTP_CONNECTION);
            while(*pBuff == SPACE)
                pBuff++;

            if(pal_Memcmp(pBuff, HTTP_ENCODING_CHUNKED, pal_Strlen(HTTP_CONNECTION_CLOSE)) == 0)
            {
                /* not supported */
                CLI_Write(" Server response format is not supported\r\n");
                ASSERT_ON_ERROR(FORMAT_NOT_SUPPORTED);
            }
        }
    }

    /* "\r\n\r\n" marks the end of headers */
    pBuff = (_u8 *)pal_Strstr(g_buff, HTTP_END_OF_HEADER);
    if(pBuff == 0)
    {
        CLI_Write(" Invalid response\r\n");
        ASSERT_ON_ERROR(INVALID_SERVER_RESPONSE);
    }
    /* Increment by 4 to skip "\r\n\r\n" */
    pBuff += 4;

    /* Adjust buffer data length for header size */
    transfer_len -= (pBuff - g_buff);

    /* If data in chunked format, calculate the chunk size */
    if(isChunked == 1)
    {
        retVal = getChunkSize(&transfer_len, &pBuff, &recv_size);
        if(retVal < 0)
        {
            /* Error */
            CLI_Write(" Problem with connection to server\r\n");
            return retVal;
        }
    }

    /* Open file to save the downloaded file */
    retVal = sl_FsOpen((_u8 *)FILE_NAME,
                       FS_MODE_OPEN_WRITE, &Token, &fileHandle);
    if(retVal < 0)
    {
        /* File Doesn't exit create a new of 45 KB file */
        retVal = sl_FsOpen((_u8 *)FILE_NAME,
                           FS_MODE_OPEN_CREATE(SIZE_45K,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                           &Token, &fileHandle);
        if(retVal < 0)
        {
            CLI_Write(" Error during opening the file\r\n");
            return retVal;
        }
    }

    while (0 < transfer_len)
    {
        /* For chunked data recv_size contains the chunk size to be received
         * while the transfer_len contains the data in the buffer */
        if(recv_size <= transfer_len)
        {
            /* write the recv_size */
            retVal = sl_FsWrite(fileHandle, g_BytesReceived,
                    (_u8 *)pBuff, recv_size);
            if(retVal < recv_size)
            {
                /* Close file without saving */
                retVal = sl_FsClose(fileHandle, 0, "A", 1);
                CLI_Write(" Error during writing the file\r\n");
                return FILE_WRITE_ERROR;
            }
            transfer_len -= recv_size;
            g_BytesReceived +=recv_size;
            pBuff += recv_size;
            recv_size = 0;

            if(isChunked == 1)
            {
                /* if data in chunked format calculate next chunk size */
                pBuff += 2; /* 2 bytes for <CR> <LF> */
                transfer_len -= 2;

                if(getChunkSize(&transfer_len, &pBuff, &recv_size) < 0)
                {
                    /* Error */
                    break;
                }

                /* if next chunk size is zero we have received the complete file */
                if(recv_size == 0)
                {
                    eof_detected = 1;
                    break;
                }

                if(recv_size < transfer_len)
                {
                    /* Code will enter this section if the new chunk size is less then
                     * then the transfer size. This will the last chunk of file received
                     */
                    retVal =sl_FsWrite(fileHandle, g_BytesReceived,
                            (_u8 *)pBuff, recv_size);
                    if(retVal < recv_size)
                    {
                        /* Close file without saving */
                        retVal = sl_FsClose(fileHandle, 0, "A", 1);
                        CLI_Write(" Error during writing the file\r\n");
                        return FILE_WRITE_ERROR;
                    }
                    transfer_len -= recv_size;
                    g_BytesReceived +=recv_size;
                    pBuff += recv_size;
                    recv_size = 0;

                    pBuff += 2; /* 2bytes for <CR> <LF> */
                    transfer_len -= 2;

                    /* Calculate the next chunk size, should be zero */
                    if(getChunkSize(&transfer_len, &pBuff, &recv_size) < 0)
                    {
                        /* Error */
                        break;
                    }

                    /* if next chunk size is non zero error */
                    if(recv_size != 0)
                    {
                        /* Error */
                        break;
                    }
                    eof_detected = 1;
                    break;
                }
                else
                {
                    /* write data on the file */
                    retVal = sl_FsWrite(fileHandle, g_BytesReceived,
                            (_u8 *)pBuff, transfer_len);
                    if(retVal < transfer_len)
                    {
                        /* Close file without saving */
                        retVal = sl_FsClose(fileHandle, 0, "A", 1);
                        CLI_Write(" Error during writing the file\r\n");
                        ASSERT_ON_ERROR(FILE_WRITE_ERROR);
                    }
                    recv_size -= transfer_len;
                    g_BytesReceived +=transfer_len;
                }
            }
            /* complete file received exit */
            if(recv_size == 0)
            {
                eof_detected = 1;
                break;
            }
        }
        else
        {
            /* write data on the file */
            retVal = sl_FsWrite(fileHandle, g_BytesReceived,
                                 (_u8 *)pBuff, transfer_len);
            if (retVal < 0)
            {
                /* Close file without saving */
                retVal = sl_FsClose(fileHandle, 0, "A", 1);
                CLI_Write(" Error during writing the file\r\n");
                ASSERT_ON_ERROR(FILE_WRITE_ERROR);
            }
            g_BytesReceived +=transfer_len;
            recv_size -= transfer_len;
        }

        pal_Memset(g_buff, 0, sizeof(g_buff));

        transfer_len = sl_Recv(g_SockID, &g_buff[0], MAX_BUFF_SIZE, 0);
        if(transfer_len <= 0)
            ASSERT_ON_ERROR(TCP_RECV_ERROR);

        pBuff = g_buff;
    }

    /* If user file has checksum which can be used to verify the temporary
     * file then file should be verified
     * In case of invalid file (FILE_NAME) should be closed without saving to
     * recover the previous version of file */
    if(0 > transfer_len || eof_detected == 0)
    {
        /* Close file without saving */
        retVal = sl_FsClose(fileHandle, 0, "A", 1);
        CLI_Write(" Error While File Download\r\n");
        ASSERT_ON_ERROR(INVALID_FILE);
    }
    else
    {
        /* Save and close file */
        retVal = sl_FsClose(fileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(retVal);
    }

    return SUCCESS;;
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

    /* If the device is not in station-mode, try configuring it in station-mode */
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
       Number between 0-15, as dB offset from maximum power - 0 will set maximum power */
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
    g_SockID = 0;
    g_DestinationIP = 0;
    g_BytesReceived = 0;
    pal_Memset(g_buff, 0, sizeof(g_buff));

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
    CLI_Write(" File download application - Version ");
    CLI_Write(APPLICATION_VERSION);
    CLI_Write("\n\r*******************************************************************************\n\r");
}

