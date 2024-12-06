/*!
 * \file GPS.h
 * \brief
 *
 *  Created on: Oct 28, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#ifndef GPS_H_
#define GPS_H_

#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include <cstdbool>
#include <cstring>
#include "main.h"
#include "GPSsat.h"

class GPS
    {
private:
    const static uint8_t GSV_FLS {0b001};
    const static uint8_t RCM_FLT {0b010};
    const static uint8_t RCM_FLD {0b100};
    const static uint8_t RCM_FLTD {RCM_FLT | RCM_FLT};

    //static std::vector<std::string> mainBuf;

public:
// GPS buffer
    const static int RXSZ {403};

// if you change parameter, check for right checksum
    const static char SET_NMEA_BAUDRATE[];
    const static char SET_NMEA_UPDATERATE[];
    const static char API_SET_OUTPUT[];

    const static uint8_t ALL_OK {GSV_FLS | RCM_FLTD};

    static uint8_t acquired;
    static uint8_t gps_rx[RXSZ];
    static uint8_t gps_rxTmp[RXSZ];

// parser
// accepted sentences
    static const std::string GSV;
    static const std::string RMC;
    static const int RMC_ID {1};
    static const int GSV_ID {2};
    static int sentenceID;

    static int gps_second;
    static int gps_minute;
    static int gps_hour;
    static int gps_year;
    static int gps_month;
    static int gps_day;

    static int gps_sattNumb;

    inline static bool has_date_time()
        {
        return ((acquired & RCM_FLTD) == RCM_FLTD);
        }
    inline static bool has_sat_data()
        {
        return ((acquired & GSV_FLS) == GSV_FLS);
        }

    static void read_data();
    static void parse_sentences();
    static bool check_what_sentence(const std::string_view &fields);
    static bool parse_sentence(const std::vector<std::string_view>&);
    static void update_strong();

// check (not exact)
    static bool date_valid()
        {
        return gps_year > -1 && gps_year < 99 && gps_month > 0 && gps_month < 13
                && gps_day > 0 && gps_day < 32;
        }

    static void save(int sz)
        {
        if ((gps_rx[0] == '$') && (sz > 50) && (sz < (RXSZ - 1)))
            {
            gps_rx[sz] = 0;
            }
        else
            {
            gps_rx[0] = 0;
            }
        }

    static bool time_valid()
        {
        return gps_hour > -1 && gps_hour < 60 && gps_minute > -1 && gps_minute < 60
                && gps_second > -1 && gps_second <= 60;
        }

    static void unset();

    static bool set_time(const std::string_view &t);
    static bool set_date(const std::string_view &d);
    static void gps_init()
        {
        unset();
        }

    /*!
     * \brief counts MNEA check sum
     *
     * whole sentence (include $ and * must be send to the method
     *
     * \param s message to check
     * \param size return pos. of '*' character, or -1
     * \return check sum or -1 if syntax error
     */
    static int count_check_sum(const std::string_view &s, std::string::size_type &size)
        {
        int sum = 0;
        size = -1;
        int dollars_nb = 0;

        for (std::string::size_type i = 0; i < s.length(); ++i)
            {
            char c = s[i];

            if (c == '$')
                {
                ++dollars_nb;
                if (dollars_nb > 1)
                    {
// pair $ and * don't match
                    return -1;
                    }

                continue;
                }

            if (dollars_nb == 0)
                {
                continue;
                }

            if (c == '*')
                {
                size = i;
                return sum;
                }

            sum ^= c;
            }

        return -1;
        }

    static bool cntrCheckSum(const std::string_view &s)
        {
        std::string::size_type sz = s.length();
        std::string::size_type p = 0;

        int csum = count_check_sum(s, p);
        if (csum == -1) return false;

        if (sz < (p + 3)) return false;

// we have counted in 10, in s, control number is in hex, hex => 10
        int ctr_sum = std::stoi(std::string(s.substr(p + 1, 2)), 0, 16);
        if (csum != ctr_sum)
            {
            return false;
            }

        return true;
        }
    };

#endif /* GPS_H_ */
