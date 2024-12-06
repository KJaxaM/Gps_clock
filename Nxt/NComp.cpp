/*
 * NComp.cpp
 * \brief
 *
 *  Created on: Nov 19, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 *	
 */

#include "NComp.h"
#include "NDisplay.h"

void NComp::setVal(int iOut) const
    {
    display.setVal(key, iOut);
    }

bool NComp::getVal(int &iIn) const
    {
    display.dataIn = false;
    display.getVal(key);

    for (int i = 0; i < 10; ++i)
        {
        HAL_Delay(10);
        if (display.dataIn)
            {
            iIn = display.NextNumBuff;
            display.dataIn = false;
            return true;
            }
        }
    return false;
    }

void NComp::setText(const std::string &txtOut) const
    {
    display.setText(key, txtOut);
    }

bool NComp::getText(std::string &txtIn) const
    {
    display.dataIn = false;
    display.getText(key);

    for (int i = 0; i < 10; ++i)
        {
        HAL_Delay(10);
        if (display.dataIn)
            {
            txtIn = display.NextTxtBuff;
            display.dataIn = false;
            return true;
            }
        }

    return false;
    }
