/*
 * NextWorker.h
 * \brief
 *
 *  Created on: Nov 18, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 *
 ***********************************************************************
 *  Some Nextion commands:
 *  page    usage: page <pid>
 *          page 0 // Change page to indexed page 0
 *---
 *  ref     Refresh component
 *          usage: ref <cid>
 *---
 *  get   t0.txt or n0.val
 *---
 * sendme  sends NO of currently used page
 *---
 * rest     usage: immediate reset of Nextion device - reboot.
 *---
 * ussp     Sleep on no Serial   ussp=30
 *          Sets internal No-serial-then-sleep timer to specified value in seconds
 *          min 3, max 65535, default 0
 *          Nextion will auto-enter sleep mode if and when this timer expires.
 *---
 * thsp    Sleep on No Touch    thsp=30
 *---
 * thup    Auto Wake on Touch: thup=0 (do not wake), thup=1 (wake on touch)
 *---
 * Observation: when in sleep mode setVal does't work
 */

#ifndef NDISPLAY_H_
#define NDISPLAY_H_

#include <string>
#include <map>
#include <functional>
//Include HAL Library from main header file
#include "main.h"
#include "NObject.h"
#include "NComp.h"

class NDisplay
    {
    friend NComp;

public:
    /*!
     * \fn void processRx(UART_HandleTypeDef*, int)
     * \brief This method parses data from Nextion and performs commands
     * \param _huart Handle for UART.
     * \param _size Nubmer of bytes in RX buffer.
     */
    void processRx(UART_HandleTypeDef *_huart, int _size);

    /*!
     * \fn HAL_StatusTypeDef init(UART_HandleTypeDef*, DMA_HandleTypeDef*)
     * \brief The function sets handle of UART and initialize map with pointers to
     *  event handlers
     * \param _huart address of handle to UART
     * \param address of dma handle
     * \return HAL_Status enum, expected: HAL_OK
     */
    HAL_StatusTypeDef init(UART_HandleTypeDef *_huart, DMA_HandleTypeDef *dmaHandle);

    /*!
     * \fn HAL_StatusTypeDef waitRxEvent()
     * \brief Initiates next waiting for incoming data from Nextion
     *
     * \return HAL_Status enum, expected: HAL_OK
     */
    HAL_StatusTypeDef waitRxEvent()
        {
        // wait for next data reading idle event
        HAL_StatusTypeDef retVal = HAL_UARTEx_ReceiveToIdle_DMA(p_uartHandle,
                (uint8_t*) RxData, BUFF_SIZE);
        // we don't need interrupt on half buffer filled - disable
        __HAL_DMA_DISABLE_IT(p_dmaHandle, DMA_IT_HT);

        return retVal;
        }

    /*!
     * \fn NComp addComp(uint8_t, uint8_t, const std::string&, void(*)()=nullptr, void(*)()=nullptr)
     * \brief This method creates Nextion objects and returns a handle to this object.
     *
     * \param _page page NO
     * \param _id object id, as in the Nextion Editor
     * \param _objname object name as in the Nextion Editor
     * \param _callbackOnPress user function which calls on press the object, omit if no needs
     * \param _callbackOnRelease user function which calls on release the object, omit if no needs
     * \return handle to object, e.q. NComp object.
     */
    NComp addComp(uint8_t _page, uint8_t _id, const std::string &_objname,
            void (*_callbackOnPress)()= nullptr, void (*_callbackOnRelease)()= nullptr);

    /*!
     * \fn void sendCommand(const std::string&) const
     * \brief The method sends commands to Nextion.
     *
     * \param _command string with command
     */
    void sendCommand(const std::string &_command) const;

private:
    /*! Because data from Nextion reads asynchronous, it needs some signal talking that
     * data is read
     */
    bool dataIn {false};

// don't change, there are Nextion's constants which we use in sending commands and
// parsing of responses from Nextion
//@forma:off
  static  const int RELEASE      {0x00};
  static const int PRESS         {0x01};
  //
  static const int NEX_RET_INVALID_CMD                {0x00};
  static const int NEX_RET_CMD_FINISHED               {0x01};
  static const int NEX_RET_INVALID_COMPONENT_ID       {0x02};
  static const int NEX_RET_INVALID_PAGE_ID            {0x03};
  static const int NEX_RET_INVALID_PICTURE_ID         {0x04};
  static const int NEX_RET_INVALID_FONT_ID            {0x05};
  static const int NEX_RET_INVALID_BAUD               {0x11};
  static const int NEX_RET_INVALID_VARIABLE           {0x1A};
  static const int NEX_RET_INVALID_OPERATION          {0x1B};
  static const int NEX_RET_EVENT_TOUCH_HEAD;
  static const int NEX_RET_EVENT_POSITION_HEAD        {0x67};
  static const int NEX_RET_CURRENT_PAGE_ID_HEAD       {0x66};
  static const int NEX_RET_EVENT_SLEEP_POSITION_HEAD  {0x68};
  static const int NEX_RET_STRING_HEAD;
  static const int NEX_RET_NUMBER_HEAD;
  static const int NEX_RET_EVENT_LAUNCHED             {0x88};
  static const int NEX_RET_EVENT_WAKED;
//    const int NEX_RET_EVENT_UPGRADED             {0x89};
//@forma:on

// wait for the result message from Nextion
    const uint32_t NEXTION_TIMEOUT {150};

    static const int BUFF_SIZE {96};

    const uint32_t COM_DELAY {20};

    char RxData[BUFF_SIZE];
    char TxData[BUFF_SIZE];

    uint32_t NextTextLen;

    std::string NextTxtBuff;
    int32_t NextNumBuff;

    UART_HandleTypeDef *p_uartHandle;
    DMA_HandleTypeDef *p_dmaHandle;

    std::map<std::pair<uint8_t, uint8_t>, NObject> dObjects;

    std::map<int, void (NDisplay::*)()> eventHandl;

    void eventTouchHandl();
    void stringHeadHandl();
    void numberHeadHandl();
    void eventWakedHandl();

    void setVal(PnO_Id key, int iOut);
    void getVal(PnO_Id key);
    void setText(PnO_Id key, const std::string &txtOut);
    void getText(PnO_Id key);
    };

#endif /* NDISPLAY_H_ */
