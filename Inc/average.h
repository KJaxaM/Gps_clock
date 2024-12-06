/*
 * average.h
 * \brief
 *
 *  Created on: Nov 28, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 *	
 */

#ifndef INC_AVERAGE_H_
#define INC_AVERAGE_H_

template<typename T, uint32_t memory>
class Average
    {

private:
    T sum {0};
    uint32_t number {0};

public:
    void addItem(T item)
        {
        sum += item;
        ++number;
        if (number > memory)
            {
            sum = getAvr();
            number = memory >> 3;
            sum *= number;
            }
        }

    T getAvr() const
        {
        return number != 0 ? sum / number : 0;
        }
    };

#endif /* INC_AVERAGE_H_ */
