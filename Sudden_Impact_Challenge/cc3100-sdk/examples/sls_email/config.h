/*
 * config.h - SLS email application configuration file
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

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

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

/* Modify the following settings as necessary to run the email application */
#define SMTP_BUF_LEN            100
#define GMAIL_HOST_NAME         "smtp.gmail.com"
#define GMAIL_HOST_PORT         465
#define YAHOO_HOST_NAME         "smtp.mail.yahoo.com"
#define YAHOO_HOST_PORT         25

/* Source email credentials */

/* Username should be less than (MAX_USERNAME_LEN) characters */
#define USER                    "<username>"
/* Password should be less than (MAX_PASSWORD_LEN) characters */
#define PASS                    "<password>"

#define SUCCESS         0
#define MAX_SSID_LEN    32
#define MAX_PASSKEY_LEN 32

#define UART_COMMAND_CC31xx_SIMPLE_CONFIG_START (0x31)
#define UART_COMMAND_CC31xx_EMAIL_HEADER        (0x33)
#define UART_COMMAND_CC31xx_EMAIL_MESSAGE       (0x34)
#define UART_COMMAND_CC31xx_EMAIL_SEND          (0x35)

#define INPUT_BUF_SIZE          65
#define BAUD_RATE               115200

#ifdef SL_IF_TYPE_UART
#define COMM_PORT_NUM 21
SlUartIfParams_t params;
#endif /* SL_IF_TYPE_UART */


/* Status bits - These are used to set/reset the corresponding bits in 'g_Status' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in 'g_Status', the device is connected to the AP
                                 *      0 in 'g_Status', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_ACQUIRED       /* If this bit is:
                                 *      1 in 'g_Status', the device has acquired an IP
                                 *      0 in 'g_Status', the device has not acquired an IP
                                 */

}e_StatusBits;

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    EMAIL_SET_INVALID_MESSAGE = DEVICE_NOT_IN_STATION_MODE - 1,
    EMAIL_SET_INVALID_CASE =EMAIL_SET_INVALID_MESSAGE - 1,
    EMAIL_CONNECT_INVALID_CONFIURATION = EMAIL_SET_INVALID_CASE - 1,
    TCP_RECV_ERROR = EMAIL_CONNECT_INVALID_CONFIURATION - 1,
    TCP_SEND_ERROR = TCP_RECV_ERROR - 1,
    SMTP_ERROR = TCP_SEND_ERROR,
    SMTP_INVALID_CASE = SMTP_ERROR -1,
    INVALID_INPUT = SMTP_INVALID_CASE - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;



#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))

#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_ACQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_ACQUIRED)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* DEMO_CONFIG_H */
