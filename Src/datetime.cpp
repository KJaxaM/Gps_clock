/*!
 * \file datetime.cpp
 * \brief
 *
 *  Created on: Aug 17, 2021.
 *      Author: Kris Jaxa
 *            © Jaxasoft, Freeware,
 *              v.1.00
 *******************************************************************************
 ** History:
 *
 * 24-11-11 only UTC shows
 * 21-10-17 fix error in ctor, tested leap second                     v. 1.00
 */

#include "datetime.h"

const std::string L_TIME[] = {"?", "CET", "CEST"};

const std::string THE_MONTHS[13] = {"?", "January", "February", "March", "April", "May",
        "June", "July", "August", "September", "October", "November", "December"};

const std::string THE_WEEKD[8] = {"?", "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday"};

void Date_time::add_sec()
    {
    ++sec;
    if (sec < 60) return;

    if (leap_s_will_be_intro && minute == 59 && day == 1
            && ((month == 7 && hour == 1) || (month == 1 && hour == 0)))
        {
        // show second 60 (leap sec.)
        leap_s_will_be_intro = false;
        return;
        }

    sec = 0;
    add_min();
    }

void Date_time::add_min()
    {
    ++minute;
    if (minute < 60) return;

    minute = 0;
    add_h();
    }

void Date_time::add_h()
    {
    ++hour;
//    // last Sunday in March, 1 h CET is one hour to change time to CEST
//    if (!isSu && hour == 1 && day == suFirst && month == 3)
//        {
//        cest_announ = true;
//        }

//    if (isSu && hour == 2 && day == suLast && month == 10)
//        {
//        cest_announ = true;
//        }

// summer time? (CET = UTC + 1), change UTC = 1
    if (!useUTC)
        {
        if (cest_announ && hour == 2 && day == suFirst && month == 3)
            {
            hour = 3;
            isSu = true;
            cest_announ = false;
            }
        // end summer time? (CEST = UTC + 2), change UTC = 1
        if (isSu && hour == 3 && day == suLast && month == 10)
            {
            hour = 2;
            isSu = false;
            cest_announ = false;
            }
        }

    if (hour < 24) return;

    hour = 0;
    add_d();
    }

void Date_time::add_d()
    {
    ++day;
    if (day <= max_day(*this)) return;

    day = 1;
    add_month();
    }

void Date_time::add_month()
    {
    ++month;
    if (month < 13) return;

    month = 1;
    add_y();
    }

void Date_time::add_y()
    {
    ++year;
    setSummerStart();
    }

// https://en.wikipedia.org/wiki/Zeller%27s_congruence
uint8_t Date_time::get_wday() const
    {
    if (!month || !day) return 0;

    int m = month;
    int y = year;

    if (m <= 2)
        {
        m += 12;
        y -= 1;
        }

    return get_wday(y, m, day);
    }

inline uint8_t Date_time::get_wday(int y, int m, int d)
    {
    if (m <= 2)
        {
        m += 12;
        y -= 1;
        }

    return (((d + 13 * (m + 1) / 5 + y + (y >> 2) + 105)) + 5) % 7 + 1;
    }

//int Date_time::get_weekNb() const
//
//    {
//    int wd, d0;
//    int dnb;
//
//    Date_time tmp(year, 12, 31, 0, 0, 0);
//
//    if (month == 12 && day >= 28)
//        {
//        wd = tmp.get_wday();
//        if (wd <= 3)
//            {
//            if (day >= (32 - wd))
//                return 1;   // 1 week next year
//            }
//        }
//
//// find start of the weak 1 of the year
//    tmp.day = 1;
//    tmp.month = 1;
//
//    wd = tmp.get_wday();
//    if (wd < 5)
//        {
//        d0 = wd - 2;
//        }
//    else
//        {
//        d0 = wd - 9;
//
//        if (month == 1)
//            {
//            if (day < (9 - wd))
//                {
//                tmp.month = 12;
//                tmp.day = 31;
//                --tmp.year;
//                return tmp.get_weekNb();
//                }
//            }
//        }
//    dnb = d0;
//    tmp.year = year;
//
//    for (uint8_t i = 1; i <= month; ++i)
//        {
//        if (i < month)
//            {
//            tmp.month = i;
//            dnb += max_day(tmp);
//            continue;
//            }
//        dnb += day;
//
//        return dnb / 7 + 1;
//        }
//
//    return 0;
//    }

int max_day(int m, int y)
    {
    int maxDay = 31;

    if (m == 4 || m == 6 || m == 9 || m == 11)
        {
        maxDay = 30;
        }
    else if (m == 2)
        {
        if (y % 4)
            {
            maxDay = 28;
            }
        else
            {
            maxDay = 29;
            }
        }

    return maxDay;
    }

/*! @brief Set first and last Summer Time days
 *
 * In all locations in Europe where summer time is observed (the EU, EFTA and associated
 * countries), European Summer Time begins at 01:00 UTC/WET (02:00 CET, 03:00 EET) on the
 * last Sunday in March (between 25 and 31 March) and ends at 01:00 UTC (02:00 WEST, 03:00
 * CEST, 04:00 EEST) on the last Sunday in October (between 25 and 31 October).
 * https://en.wikipedia.org/wiki/Summer_time_in_Europe
 */
bool Date_time::setSummerStart()
    {
    if (!year || useUTC)
        {
        return false;
        }

    // W -> S, last Sunday in March
    suFirst = getLastWday(3, 7);
    // S -> W, last Sunday in October
    suLast = getLastWday(10, 7);

    if (!month || !day)
        {
        return false;
        }

    if ((month > 3 && month < 10) || (month == 3 && day > suFirst)
            || (month == 10 && day < suLast))
        {
        isSu = true;
        return true;
        }

    if ((month > 10 || month < 3) || (month == 10 && day > suLast)
            || (month == 3 && day < suFirst))
        {
        isSu = false;
        return true;
        }

    if (month == 3 && day == suFirst)
        {
        if (hour <= 2)
            {
            isSu = false;
            return true;
            }
        if (hour >= 3)
            {
            isSu = true;
            return true;
            }
        }

    if (month == 10 && day == suLast)
        {
        if (hour <= 2)
            {
            isSu = true;
            return true;
            }
        if (hour >= 3)
            {
            isSu = false;
            return true;
            }
        }

    return true;
    }

std::string Date_time::getDateStr()
    {
    if (year == 0) return "";

    std::string ans = THE_WEEKD[get_wday()];
    ans += ", ";
    ans += THE_MONTHS[month];
    ans += " ";
    ans += std::to_string(day);
    ans += ", ";
    ans += std::to_string(2000 + year);
    if (!useUTC)
        {
        ans += "  (";
        ans += L_TIME[isSu ? 2 : 1];
        ans += ")";
        }
    return ans;
    }

//Date_time fromS2W(const Date_time &dt)
//    {
//    if (!dt.isSu) return dt;
//
//    Date_time dtECT(dt);
//
//    if (dtECT.hour > 1)
//        {
//        --dtECT.hour;
//        return dtECT;
//        }
//
//    dtECT.hour = 23;
//    if (dtECT.day > 1)
//        {
//        --dtECT.day;
//        return dtECT;
//        }
//
//    int m = dt.month;
//    if (m > 1)
//        {
//        --m;
//        dtECT.day = max_day(m, dt.year);
//        dtECT.month = m;
//        return dtECT;
//        }
//
//    dtECT.hour = 0;
//    dtECT.day = 31;
//    dtECT.month = 12;
//    dtECT.year = dt.year - 1;
//    return dtECT;
//    }

////////////////////////////////////////////////////////////////////

//int sex_2_parts(float sex, int *mm, int *sec)
//    {
//    int h = (int) sex;
//    float fmin = 60.0 * (sex - h);
//    *mm = (int) fmin;
//    *sec = (int) (60.0 * (fmin - *mm));
//
//    return h;
//    }

