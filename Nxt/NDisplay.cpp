/*
 * \file NDisplay.cpp
 * \brief
 *
 *  Created on: Nov 18, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#include "NDisplay.h"
#include <string_view>

const int NDisplay::NEX_RET_EVENT_TOUCH_HEAD {0x65};
const int NDisplay::NEX_RET_STRING_HEAD {0x70};
const int NDisplay::NEX_RET_NUMBER_HEAD {0x71};
const int NDisplay::NEX_RET_EVENT_WAKED {0x87};
/*! \brief Support touch event, string data and numeric data
 *  </summary>
 *
 *  <param name="_huart">handle UART</param>
 *  <param name="sz">number of red bytes</param>
 *
 */
void NDisplay::processRx(UART_HandleTypeDef *_huart, int sz)
    {
    // not real message, ignore
    if (sz < 4)
        {
        waitRxEvent();
        return;
        }

    std::string_view sview(RxData, sz);
    // find first pattern of 0xff 0xff 0xff
    auto pos = sview.find("\xff\xff\xff");
    if (pos == std::string_view::npos)
        {
        waitRxEvent();
        return;
        }

    NextTextLen = pos;

    if (eventHandl.contains(RxData[0]))
        {
        (this->*eventHandl[RxData[0]])();
        }

    waitRxEvent();
    }

HAL_StatusTypeDef NDisplay::init(UART_HandleTypeDef *_uartHandle,
        DMA_HandleTypeDef *_dmaHandle)
    {
    p_uartHandle = _uartHandle;
    p_dmaHandle = _dmaHandle;

    eventHandl[NEX_RET_EVENT_TOUCH_HEAD] = &NDisplay::eventTouchHandl;
    eventHandl[NEX_RET_STRING_HEAD] = &NDisplay::stringHeadHandl;
    eventHandl[NEX_RET_NUMBER_HEAD] = &NDisplay::numberHeadHandl;
    eventHandl[NEX_RET_EVENT_WAKED] = &NDisplay::eventWakedHandl;

    //Start UART transaction using Idle and DMA
    return waitRxEvent();
    }

/*!
 * \brief Add object to map<{{pNo,id}, object}
 *
 * \param _page page No where Nextion object lives
 * \param _id id of object
 * \param _objname name of the object
 * \param _callbackOnPress callback method on Press or nullptr
 * \param _callbackOnRelease callback method on Release
 * \return
 */
NComp NDisplay::addComp(uint8_t _page, uint8_t _id, const std::string &_objname,
        void (*_callbackOnPress)(), void (*_callbackOnRelease)())
    {
    NObject nxtObj(_page, _id, _objname, _callbackOnPress, _callbackOnRelease);
    PnO_Id key {_page, _id};
    dObjects[key] = nxtObj;

    NComp nc(key, *this);

    return nc;
    }

void NDisplay::setVal(PnO_Id key, int iOut)
    {
    std::sprintf(TxData, "%s.val=%d", dObjects[key].objname.c_str(), iOut);
    sendCommand(TxData);
    }

void NDisplay::sendCommand(const std::string &_command) const
    {
    static uint8_t END_MSG[3] {0xff, 0xff, 0xff};
    HAL_UART_Transmit(p_uartHandle, (uint8_t*) _command.c_str(), _command.size(),
            NEXTION_TIMEOUT); //½
    HAL_UART_Transmit(p_uartHandle, END_MSG, 3, NEXTION_TIMEOUT);
    }

void NDisplay::getVal(PnO_Id key)
    {
    sprintf(TxData, "get %s.val", dObjects[key].objname.c_str());
    sendCommand(TxData);
    }

void NDisplay::setText(PnO_Id key, const std::string &txtOut)
    {
    std::sprintf(TxData, "%s.txt=\"%s\"", dObjects[key].objname.c_str(), txtOut.c_str());
    sendCommand(TxData);
    }

//If the received package contains string data
void NDisplay::getText(PnO_Id key)
    {
    std::sprintf(TxData, "get %s.txt", dObjects[key].objname.c_str());
    sendCommand(TxData);
    }

// touch     code page id   event
// expected: 0x65 0xPa 0xId 0xEv 0xFF 0xFF 0xFF
void NDisplay::eventTouchHandl()
    {
    if (NextTextLen != 4)
        {
        return;
        }

    //In case of a touch event call the callback function accordingly.
    PnO_Id p_id {RxData[1], RxData[2]};

    if (dObjects.contains(p_id))
        {
        //Call the desired On Press or On Release callback function,
        if (RxData[3] == PRESS)
            {
            if (dObjects.at(p_id).callbackOnPress != nullptr)
                {
                dObjects.at(p_id).callbackOnPress();
                }
            }
        else if (RxData[3] == RELEASE && dObjects.at(p_id).callbackOnRelease != nullptr)
            {
            dObjects.at(p_id).callbackOnRelease();
            }
        }
    }

void NDisplay::stringHeadHandl()
    {
    NextTxtBuff.assign(&RxData[1], NextTextLen - 1);
    // flag, (we use dma read, no blocking [asynchronous], main loop has to inform
    dataIn = true;
    }

// 0x71 0x01 0x02 0x03 0x04 0xFF 0xFF 0xFF
// Returned when get command to return a number, 4 byte 32-bit value
void NDisplay::numberHeadHandl()
    {
    if (NextTextLen == 5)
        {
        //in little endian order: (0x01 + 0x02*256 + 0x03*65536 + 0x04*16777216)
        NextNumBuff = RxData[1] | (RxData[2] << 8) | ((uint32_t) RxData[3] << 16)
                | ((uint32_t) RxData[4] << 24);
        dataIn = true;
        }
    }
// not implemented
void NDisplay::eventWakedHandl()
    {
// auto wake up
    if (NextTextLen == 1)
        {
// TODO: define a  method ½
        }
    }

// 0x01 0xFF 0xFF 0xFF
// NEX_RET_CMD_FINISHED:

