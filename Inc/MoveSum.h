/*!
 * \file MoveSum.h
 * \brief
 *
 *  Created on: Aug 31, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0
 */

#ifndef INC_MOVESUM_H_
#define INC_MOVESUM_H_

#include <array>

/*!
 * \brief Template: moving sum of type T.
 */
template<typename T, int N>
class MovSum
    {
public:

    bool IsReady() const
        {
        return isReady;
        }

    /*!
     * Return current index of the sum.
     *
     * \return int
     */
    int Inx() const
        {
        return i;
        }

    T getSum() const
        {
        return sum;
        }

    /*! Adds a new item, subtract the last (modulo N).
     *
     * \param item, item to add.
     * \return The sum after updating.
     */
    T addItem(T item)
        {
        ++i;
        if (i == N)
            {
            isReady = true;
            i = 0;
            }

        sum += item - buf[i];
        buf[i] = item;

        return sum;
        }

private:
    int i{-1};
    T sum{0};
    std::array<T, N> buf;
    bool isReady;
    };

#endif /* INC_MOVESUM_H_ */
