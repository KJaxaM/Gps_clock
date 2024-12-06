/*!
 * \file NexObject.h
 *
 * \brief The definition of class NexObject.
 */
#ifndef __NEXOBJECT_H__
#define __NEXOBJECT_H__

#include <string>
#include "NDisplay.h"

typedef std::pair<uint8_t, uint8_t> PnO_Id;

class NObject
    {
public:
    friend class NDisplay;

    NObject() :
            pNO_id( {0, 0}),
            objname(""),
            callbackOnPress(nullptr),
            callbackOnRelease(nullptr)
        {
        }

private:
    NObject(const NObject &oth) :
            pNO_id(oth.pNO_id),
            objname(oth.objname),
            callbackOnPress(oth.callbackOnPress),
            callbackOnRelease(oth.callbackOnRelease)
        {
        }

    NObject(uint8_t _page, uint8_t _id, const std::string &_objname,
            void (*_callbackOnPress)(), void (*_callbackOnRelease)()) :
            pNO_id( {_page, _id}),
            objname(_objname),
            callbackOnPress(_callbackOnPress),
            callbackOnRelease(_callbackOnRelease)

        {
        }

    NObject& operator=(const NObject &oth)
        {
        if (this != &oth)
            {
            pNO_id = oth.pNO_id;
            objname = oth.objname;
            callbackOnPress = oth.callbackOnPress;
            callbackOnRelease = oth.callbackOnRelease;
            }

        return *this;
        }

    //Variables for storing page and Id for every component
    PnO_Id pNO_id;
    //Variable for storing object name
    std::string objname;
    //Function pointers for storing the callback functions
    void (*callbackOnPress)();
    void (*callbackOnRelease)();
    };

#endif /* #ifndef __NEXOBJECT_H__ */
