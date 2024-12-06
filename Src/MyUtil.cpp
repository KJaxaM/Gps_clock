/*!
 * \file MyUtil.cpp
 *
 *  Created on: Nov 9, 2024
 *      Author: Kris Jaxa
 */

#include "MyUtil.h"

/*!
 * when split goes with "\r\n",  empty fields should be killed;
 * when delimiter is ",", empty fields transforms to zero;
 * when extra split of last field is made with delimiter "*", two field will be produced,
 * first SNR must be zero if field is empty, second should be killed.
 */
std::vector<std::string_view> split(std::string_view buffer,
        const std::string_view delimiter)
    {

    std::vector<std::string_view> result;
    std::string_view::size_type pos;

    while ((pos = buffer.find(delimiter)) != std::string_view::npos)
        {
        auto match = buffer.substr(0, pos);

        if (match.empty() && (delimiter == "," || delimiter == "*"))
            {
            // null (empty field best be expressed as zero
            result.push_back("0");
            }

        if (!match.empty())
            {
            result.push_back(match);
            }

        buffer.remove_prefix(pos + delimiter.size());
        }

    if (!buffer.empty() && delimiter != "*")
        {
        result.push_back(buffer);
        }

    return result;
    }

/* string to int */
int get_nd(const std::string_view &field, int pos, int n)
    {
    int r = -1;
    if (pos < 0 || n < 1 || n > 9)
        {
        return -1;
        }

    if (!vtonum(field.substr(pos, n), r))
        {
        return -1;
        }

    return r;
    }
