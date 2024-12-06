/*!
 * \file main.cpp
 * \brief main file
 *
 * It's only for fun, learn C++, serial communication. I use C++ v.20 GNU compiler.
 * Own Library for Nextion in test. Own library for communicate with and pars GPS
 * messages.
 *
 *  Created on: Nov 11, 2024.
 *      Author: Kris Jaxa
 *            © Jaxasoft, Freeware,
 *              v.1.0.0
 *
 *******************************************************************************
 *
 *
 * GPS board is my old GPS breakout board FGPMMOPA6H from "servicepunkten electronics".
 *
 ** History:
 *  24-11-15 fix of satellites,
 *  24-11-13 better updating of satellites on display, color code for SNR.
 *
 *******************************************************************************
 *----
 * Nextion display
 *
 * PC5     ------> USART3_RX
 * PB10    ------> USART3_TX
 * baud=230400 (fast!)
 * ----
 * GPS board
 *
 * PA0     ------> UART4_TX
 * PA1     ------> UART4_RX
 * PPS from GPS: rise = second start ±10ns (timer's TIM2 frequency is 180/2 = 90 MHz,
 * so t = 1/f = 11.1ns, in practice ± 50ns is observed. Then, datasheet says that
 * interrupt takes 12 clock = 12 / 180MHz = 66.7ns [we ignore this]).
 *
 * PA8 (D7)------> PPS
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <tuple>
#include "main.h"
#include <dma.h>
#include <gpio.h>
#include <GPS_Init.h>
#include <tim.h>
#include <usart.h>
#include "MoveSum.h"
#include "average.h"
#include "datetime.h"
#include "NDisplay.h"
#include "GPS.h"
#include "GPSsat.h"

/* Private variables ---------------------------------------------------------*/
int rxdataSize;

// TODO: choose better C last for crystal
//const double _TIM_FREQ {90e6 - 2318}; // 56p
const double _TIM_FREQ {90e6 + 3958};   // 39p  linear appr.  => 49.7p
const int CNT_SEC = {(int) std::round(_TIM_FREQ)};

// I want to separate interrupt, data processing and data gathering.
const int DELTA {(int) std::round(0.5e-3 * _TIM_FREQ)};
// read and save gps messages
const int READ_ZONE_ST {(int) std::round(0.1 * _TIM_FREQ)};
const int READ_ZONE_END {(int) std::round(0.45 * _TIM_FREQ)};
const int WORK_ZONE_ST {(int) std::round(0.55 * _TIM_FREQ)};
const int WORK_ZONE_END {(int) std::round(0.9 * _TIM_FREQ)};
// if diff to big reset timer
const int TOO_BIG = {(int) std::round(0.05 * _TIM_FREQ)};
// data to nextion 1μs after one second
const int OUT_TIME {(int) std::round(1e-3 * _TIM_FREQ)};
int actualSec {CNT_SEC};
int half = actualSec >> 1;

// variables for timing
const int PULS_NMB {3};
int starter {0};

// help variable for doing things only ones per second
int progress {0};

bool init_done {false};

// variables for timer control
MovSum<int, 2> mSum2;
Average<double, 100> avr;

// timer value when PPS interrupt from GPS chip
int pps {0};
int cur_time {0};

// Date object
Date_time dt;
// Nextion library main object
NDisplay display;

int page_nb {0};

namespace sat
{
void to_page0();
void to_page1();
void show_satellites();
}

namespace tim
{
void set_time();

// I want update display so little as possible, most time only last figure of second.
struct Tm
    {
    int s10 = 0;
    int os10 = -1;
    int m = 0;
    int om = -1;
    int h = 0;
    int oh = -1;
    };

Tm tm;
}

namespace nxt
{
/*! Register the component in the library with parameters:
 * Page, ID, Object Name, Callback Function On Press, Callback Function On Release
 * If you don't want any callback function, drop last parameters.
 * The method addComp returns handle to a object managed by display.
 */
//     page 0
NComp nh = display.addComp(0, 1, "nh");
NComp nm = display.addComp(0, 3, "nm");
NComp ns2 = display.addComp(0, 4, "ns2");
NComp ns1 = display.addComp(0, 7, "ns1");
NComp nsat0 = display.addComp(0, 10, "nsat");
NComp txtDate = display.addComp(0, 5, "txtDate");
NComp butPage1 = display.addComp(0, 12, "butPage2", sat::to_page1);
NComp error = display.addComp(0, 13, "h0");

//       page 1
NComp nsat1 = display.addComp(1, 2, "nsat");
NComp id0 = display.addComp(1, 5, "id0");
NComp id1 = display.addComp(1, 6, "id1");
NComp id2 = display.addComp(1, 7, "id2");
NComp id3 = display.addComp(1, 8, "id3");
NComp id4 = display.addComp(1, 9, "id4");
NComp id5 = display.addComp(1, 10, "id5");
NComp id6 = display.addComp(1, 11, "id6");
NComp id7 = display.addComp(1, 12, "id7");

// progress bar
NComp snr0 = display.addComp(1, 19, "snr0");
NComp snr1 = display.addComp(1, 18, "snr1");
NComp snr2 = display.addComp(1, 17, "snr2");
NComp snr3 = display.addComp(1, 16, "snr3");
NComp snr4 = display.addComp(1, 15, "snr4");
NComp snr5 = display.addComp(1, 14, "snr5");
NComp snr6 = display.addComp(1, 13, "snr6");
NComp snr7 = display.addComp(1, 1, "snr7");
NComp butPage0 = display.addComp(1, 4, "butPage0", sat::to_page0);

void show_date(bool force = false)
    {
    std::div_t s10s1;

    if (force)
        {
        tim::tm.oh = tim::tm.om = tim::tm.os10 = -1;
        }

    // minimal display output, only last second figure
    s10s1 = std::div(dt.getSec(), 10);
    ns1.setVal(s10s1.rem);

    if (tim::tm.os10 != s10s1.quot)
        {
        ns2.setVal(s10s1.quot);
        tim::tm.os10 = s10s1.quot;
        }

    tim::tm.m = dt.getMinute();
    if (tim::tm.om != tim::tm.m)
        {
        nm.setVal(tim::tm.m);
        tim::tm.om = tim::tm.m;
        }

    tim::tm.h = dt.getHour();
    if (tim::tm.oh != tim::tm.h)
        {
        nh.setVal(tim::tm.h);
        tim::tm.oh = tim::tm.h;
        txtDate.setText(dt.getDateStr().c_str());
        }
    }

} //namespace nxt

namespace sat
{
const unsigned NB_SAT_SHOW = 8;

int id_old[8] {-1, -1, -1, -1, -1, -1, -1, -1};
int snr_old[8] {-1, -1, -1, -1, -1, -1, -1, -1};

std::tuple<const NComp&, const NComp&> graf[8] { {nxt::id0, nxt::snr0}, {nxt::id1,
        nxt::snr1}, {nxt::id2, nxt::snr2}, {nxt::id3, nxt::snr3}, {nxt::id4, nxt::snr4}, {
        nxt::id5, nxt::snr5}, {nxt::id6, nxt::snr6}, {nxt::id7, nxt::snr7}};

void show_page1_SNR()
    {

    if (page_nb == 1 && GPS::has_sat_data())
        {
        unsigned int sz = SVs::strongest_sats.size();
        unsigned int to = std::min(NB_SAT_SHOW, sz);

        for (uint32_t i = 0; i < to; ++i)
            {
            if (i < sz)
                {

                if (SVs::strongest_sats[i].id != id_old[i]
                        || SVs::strongest_sats[i].snr != snr_old[i])
                    {
                    id_old[i] = SVs::strongest_sats[i].id;
                    snr_old[i] = SVs::strongest_sats[i].snr;
                    std::get<0>(graf[i]).setVal(id_old[i]);
                    std::get<1>(graf[i]).setVal(snr_old[i]);
                    }
                }
            else
                {
                id_old[i] = 0;
                snr_old[i] = 0;
                std::get<0>(graf[i]).setVal(0);
                std::get<0>(graf[i]).setVal(0);
                }
            }

        sat::show_satellites();
        }
    }

void show_satellites()
    {
    if (page_nb != 1)
        {
        return;
        }

    nxt::nsat1.setVal(GPS::gps_sattNumb);

    for (auto &id_sat : SVs::old_spaceVehicles)
        {
        id_sat.second.flag = 0;
        }

    for (const auto &s_new : SVs::new_spaceVehicles)
        {
        int id = s_new.first;
        if (SVs::old_spaceVehicles.contains(id))
            {
            // check  elevation and azimuth and signal to noise ratio
            if (s_new.second == SVs::old_spaceVehicles[id])
                {
                continue;
                }

            if (!s_new.second.is_moved(SVs::old_spaceVehicles[id]))
                {
                SVs::old_spaceVehicles[id].flag = GPSsat::P;
                }
            else
                {
                // PE means erase and paint flags
                SVs::old_spaceVehicles[id].flag = GPSsat::PE;
                }
            }
        else
            {
            SVs::old_spaceVehicles.insert(s_new);
            // a new satellite, only paint
            SVs::old_spaceVehicles[id].flag = GPSsat::P;
            }
        }

    SVs::update_sats();
    }

void to_page0()
    {
    page_nb = 0;
    display.sendCommand("page 0");
    }

void to_page1()
    {
    page_nb = 1;
    SVs::old_spaceVehicles.clear();
    for (auto &sat : SVs::old_spaceVehicles)
        {
        SVs::erase_sat(sat.second, display);
        }

    id_old[0] = id_old[1] = id_old[2] = id_old[3] = id_old[4] = id_old[5] = id_old[6] =
            id_old[7] = -1;
    snr_old[0] = snr_old[1] = snr_old[2] = snr_old[3] = snr_old[4] = snr_old[5] =
            snr_old[6] = snr_old[7] = -1;

    display.sendCommand("page 1");
    }

} //namespace sat

void SystemClock_Config(void);

// for printf
#ifdef __cplusplus
extern "C"
{
#endif

int __io_putchar(int ch)
    {
    HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
    setbuf(stdout, NULL);
    return ch;
    return 1;
    }
int _write(int file, char *ptr, int len)
    {
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
        {
        __io_putchar(*ptr++);
        }
    return len;
    }
#ifdef __cplusplus
}
#endif

float pidVal = 0;

/*!
 * @brief  The application entry point.
 * @retval int (never returns)
 */
int main(void)
    {
    /* MCU Configuration--------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    MX_TIM2_Init();
    HAL_TIM_Base_Start_IT(&htim2);

    printf("\x1b[2J\x1b[H");

    for (int i = 0; i < 4; ++i)
        {
        if (gps_init::connect_gpsBoard() != 115200)
            {
            std::printf("sorry no GPS communication  - next attempt ... \r\n");
            }
        else
            {
            std::printf("GPS board is initialized\r\n");
            goto keepgoing;
            }
        }

    std::printf("Sorry can't to continue , stop\r\n");

    for (;;)
        {
        std::printf("can't to continue\r\n");
        HAL_Delay(5000);
        }

keepgoing:

//Initialize Nextion with the configured UART and DMA handle
    display.init(&huart3, &hdma_usart3_rx);

    nxt::txtDate.setText("Welcome and be amused");
    HAL_Delay(3000);

    printf("\x1b[2J\x1b[H");

    init_done = true;

    display.sendCommand("page 0");
    // from timer counter corresponding -1us to 1 us to 0 - 200 on progress bar
    // y = ERR_A * pps + 100
    const float ERR_A = 1.0e9 / _TIM_FREQ;
    TIM2->ARR = CNT_SEC;
// _______________________     forever     _______________________
    for (;;)
        {
        if (starter < PULS_NMB)
            {
            HAL_Delay(50);
            continue;
            }

        cur_time = TIM2->CNT;

        if (cur_time < OUT_TIME)
            {
            continue;
            }

        if (cur_time > WORK_ZONE_END)
            {
            progress = 0;
            continue;
            }

        // show date on display short after second starts
        if (progress == 0)
            {
            ++progress;
            if (page_nb == 0)
                {
                nxt::show_date();
                }
            continue;
            }

        if (cur_time < READ_ZONE_END)
            {
            continue;
            }

        if ((progress == 1) && (cur_time < WORK_ZONE_END))
            {
            ++progress;
            // parse gps data and set data/time object
            tim::set_time();

            switch (page_nb)
                {
            case 0:
                nxt::error.setVal(std::clamp(0, 200, (int) (ERR_A * pps + 100)));
                nxt::show_date(true);
                nxt::nsat0.setVal(GPS::gps_sattNumb);
                break;

            case 1:
                sat::show_page1_SNR();
                break;
            default:
                std::printf("error page# %d\r\n", page_nb);
                break;
                }

            pidVal = 0;

            avr.addItem(pps);
            mSum2.addItem(pps);

            int corr = (int) std::round((float) mSum2.getSum() / 6.0 + 8 * avr.getAvr()); //½
            actualSec = CNT_SEC + corr;
            TIM2->ARR = actualSec;

            std::printf("$%d %d %d;\r\n", pps, corr, (int) std::round(avr.getAvr()));
            }
        } //for (;;)
    } //int main(void)

/**********         System         *******************/
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
    {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
        Error_Handler();
        }

    /** Activate the Over-Drive mode
     */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
        {
        Error_Handler();
        }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
        {
        Error_Handler();
        }
    }

/*!
 *
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
    {
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
        {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        HAL_Delay(200);
        }
    }

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
    {
//* User can add his own implementation to report the file name and line number,
    std::printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
//   */
    }
#endif /* USE_FULL_ASSERT */

////////////////////////////////////////////////////////////////

namespace tim
{
int s_diff {0};
int s_diff_o {0};

void set_time()
    {
// parse data from GPS chip
    GPS::read_data();

    if (!GPS::has_date_time())
        {
        return;
        }

    int sec = GPS::gps_second; // we work with data from previous second
    if (sec < 2 || sec > 56)
        {
        return;
        }

    s_diff = sec - dt.getSec();
    if (s_diff_o != s_diff && s_diff != 0)
        {
        printf("s:%d->%d\r\n", dt.getSec(), sec);
        // we work with data from previous second
        dt.setSec(sec);
        }
    s_diff_o = s_diff;

    if (dt.getMinute() != GPS::gps_minute)
        {
        printf("min:%d->%d\r\n", dt.getMinute(), GPS::gps_minute);
        dt.setMinute(GPS::gps_minute);
        }
    if (dt.getHour() != GPS::gps_hour)
        {
        printf("h:%d->%d\r\n", dt.getHour(), GPS::gps_hour);
        dt.setHour(GPS::gps_hour);
        }
    if (dt.getDay() != GPS::gps_day)
        {
        printf("d:%d->%d\r\n", dt.getDay(), GPS::gps_day);
        dt.setDay(GPS::gps_day);
        }
    if (dt.getMonth() != GPS::gps_month)
        {
        printf("mo:%d->%d\r\n", dt.getMonth(), GPS::gps_month);
        dt.setMonth(GPS::gps_month);
        }
    if (dt.getYear() != GPS::gps_year)
        {
        printf("y:%d->%d\r\n", dt.getYear(), GPS::gps_year);
        dt.setYear(GPS::gps_year);
        }
    }
}

volatile int trx;
/*!
 * \fn void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef*, uint16_t)
 * \brief This methods is called recursively in infinite loop.
 *
 * For USATR3 (dedicated to Nextion) this recursive call comes from processRx.
 *
 * \param huart UART (USART) handle
 * \param Size number bytes in buffer
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
    {

    if (huart->Instance == USART3)
        {
        display.processRx(huart, Size);
        }

    else if (huart->Instance == UART4)
        {
        rxdataSize = Size;
        if (gps_init::gps_test)
            {
            if (Size == 19)
                {
                memcpy(gps_init::buf, GPS::gps_rx, Size);
                }
            goto restart;
            }

        trx = TIM2->CNT;

        if (trx > READ_ZONE_ST && trx < READ_ZONE_END)
            {
            GPS::save(Size);
            }

//     restart idle DMA receive, disable half buffer interrupt
restart:
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, GPS::gps_rx, GPS::RXSZ);
        __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
        }
    }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *th)
    {
    if (init_done && th->Instance == TIM2)
        {
        dt.add_sec();
        }
    }

/*!
 * \fn void HAL_GPIO_EXTI_Callback(uint16_t)
 * \brief PPS interrupt callback
 *
 * \param GPIO_Pin Pin that caused interrupt
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
    {
    if (starter > PULS_NMB)
        {
        pps = TIM2->CNT;

        pps -= DELTA;

        if (pps > half)
            {
            pps -= actualSec;
            }

        if (std::abs(pps) > TOO_BIG)
            {
            starter = 0;
            }
        }
    else
        {
        ++starter;
        TIM2->CNT = DELTA;
        pps = 0;
        }
    }
