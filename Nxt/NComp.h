/*
 * \file NComp.h
 * \brief
 *
 *  Created on: Nov 19, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 *	
 */

#ifndef NCOMP_H_
#define NCOMP_H_

#include <utility>
#include <cinttypes>
#include <string>

typedef std::pair<uint8_t, uint8_t> PnO_Id;

class NDisplay;

class NComp
    {
public:

//    NComp():
//        {}
    friend class NDisplay;

    void setVal(int iIn) const;
    bool getVal(int &iOut) const;
    void setText(const std::string &txtIn) const;
    bool getText(std::string &txtOut) const;

private:
    NComp(PnO_Id _key, NDisplay &_display) :
            key(_key), display(_display)
        {
        }

    PnO_Id key;
    NDisplay &display;
    };

#endif /* NCOMP_H_ */
