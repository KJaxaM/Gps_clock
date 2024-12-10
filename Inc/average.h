/*!
 * \file average.h
 * \brief Template class for correcting timer average distance from gps-pps signal.
 *
 * Average is counting for random 90..180 second.
 *
 *  Created on: Nov 28, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 *	
 */

#ifndef INC_AVERAGE_H_
#define INC_AVERAGE_H_

#include <ctime>
#include <random>

template<typename T>
class Average
    {
private:
    std::random_device rd;
    uint32_t memory;

    T sum {0};
    uint32_t number {0};

    std::mt19937 mt;
    std::uniform_int_distribution<uint32_t> dist;

    uint32_t get_mem()
        {
        return dist(mt);
        }
public:
    // 90..180 arbitrarily chosen
    Average() : mt(time(nullptr)) , dist(90, 180)
        {
        // first time correct fast
        memory = 10;
        }

    bool addItem(T item)
        {
        sum += item;
        ++number;
        if (number > memory)
            {
            sum = getAvr();
            // give some stability (fake 5 times same value)
            number = 5;
            sum *= number;
            memory = get_mem();
            return true;
            }

        return false;
        }

    T getAvr() const
        {
        return number != 0 ? sum / number : 0;
        }
    };

#endif /* INC_AVERAGE_H_ */
