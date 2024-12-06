/*!
 * \file GPS_Init.cpp
 * \brief Find baud rate and init my GPS board
 *
 *  Created on: Nov 5, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#include <GPS_Init.h>
#include "usart.h"
#include "GPS.h"

/*!
 * check baud: send twice $PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28, means
 * send no messages, first time, can last packet come in, but second only acknowledge:
 * $PMTK010,001*2E<CR><LF>.
 */

char gps_init::buf[SZ];
bool gps_init::gps_test = true;
const char SET_ZERO_OUTPUT[] = "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";
const char ACK[] = "$PMTK001,314,3*36\r\n";

void gps_init::stimulus()
    {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, GPS::gps_rx, GPS::RXSZ);
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);

    HAL_UART_Transmit(&huart4, (uint8_t*) SET_ZERO_OUTPUT, strlen(SET_ZERO_OUTPUT), 300);
    HAL_Delay(200);
    memset(buf, 0, SZ);

    HAL_UART_Transmit(&huart4, (uint8_t*) SET_ZERO_OUTPUT, strlen(SET_ZERO_OUTPUT), 300);
    HAL_Delay(150);
    }

int gps_init::connect_gpsBoard()
    {
    My_UART4_Init(9600);
    printf("try communicate with GPS in 9600 Bd\r\n");

    for (int i = 0; i < 3; ++i)
        {
        stimulus();

        if (strcmp(ACK, buf) == 0)
            {
            printf("GPS board uses 9600 Bd\r\nTrying to change to 115200 ...\r\n");
            HAL_UART_Transmit(&huart4, (uint8_t*) GPS::SET_NMEA_BAUDRATE,
                    strlen(GPS::SET_NMEA_BAUDRATE), 300);
            }
        }
    // now we must close our port and start in new Bd rate.
    if (HAL_UART_DeInit(&huart4) != HAL_OK)
        {
        return -1;
        }

    My_UART4_Init(115200);
    HAL_Delay(150);

    for (int i : {1, 2, 3})
        {
        stimulus();

        if (strcmp(ACK, buf) != 0)
            {
            printf("try # %d with 115200 Bd\r\n", i);

            if (i == 3) return -1;
            }
        else
            {

            printf("GPS uses now 115200 Bd\r\n");
            break;
            }
        }

    // send packet with 4Hz
    HAL_UART_Transmit(&huart4, (uint8_t*) GPS::SET_NMEA_UPDATERATE,
            strlen(GPS::SET_NMEA_UPDATERATE), 250);

    // which sentences and how fast to send
    HAL_UART_Transmit(&huart4, (uint8_t*) GPS::API_SET_OUTPUT,
            strlen(GPS::API_SET_OUTPUT), 250);

    gps_test = false;

    return 115200;
    }

// this is copy of org. HAL function with parameter for baud.
void gps_init::My_UART4_Init(int baud)
    {
    huart4.Instance = UART4;
    huart4.Init.BaudRate = baud;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart4) != HAL_OK)
        {
        Error_Handler();
        }
    }
