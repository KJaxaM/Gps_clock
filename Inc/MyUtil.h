/*!
 * \file view2num.h
 * \brief
 *
 *  Created on: Nov 3, 2024
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#ifndef INC_MYUTIL_H_
#define INC_MYUTIL_H_

#include <charconv>
#include <vector>
#include <string_view>

int get_nd(const std::string_view &field, int pos, int n);

inline bool read_check(const std::string_view &field, int minVal, int maxVal, int &v)
    {
    v = get_nd(field, 0, field.size());
    if (v < minVal || v > maxVal)
        {
        return false;
        }

    return true;
    }

template<typename T>
bool vtonum(const std::string_view &view, T &value)
    {
    if (view.empty())
        {
        return false;
        }

    const char *first = view.data();
    const char *last = view.data() + view.length();

    std::from_chars_result res = std::from_chars(first, last, value);

    if (res.ec != std::errc())
        {
        return false;
        }

    if (res.ptr != last)
        {
        return false;
        }

    return true;
    }

std::vector<std::string_view> split(std::string_view buffer,
        const std::string_view delimiter);

#endif /* INC_MYUTIL_H_ */
