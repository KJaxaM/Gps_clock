/*!
 * \file datetime.h
 * \brief
 *
 * Data-time counter, CET, supports leap second, summer/winter time.
 *  Created on: Aug 17, 2021.
 *      Author: Kris Jaxa
 *            © Jaxasoft, Freeware
 *            v.2.00
 *
 * --- History: ----------------------------------------------------------------
 * 2021-10-17 fix error in ctor, tested leap second                     v. 2.00
 */

#ifndef INC_DATIME_H_
#define INC_DATIME_H_

#include <string>
#include <tuple>

extern const std::string L_TIME[3];
extern const std::string THE_MONTHS[13];
extern const std::string THE_WEEKD[8];

int max_day(int m, int y);

class Date_time
    {

public:

    bool useUTC {true};

    Date_time(const Date_time &other) :
            sec(other.sec),
            minute(other.minute),
            hour(other.hour),
            day(other.day),
            month(other.month),
            year(other.year)
        {
        setSummerStart();
        }

    Date_time(int y = 0, int mo = 0, int d = 0, int h = 0, int mi = 0, int s = 0) :
            sec(s), minute(mi), hour(h), day(d), month(mo), year(y)
        {
        setSummerStart();
        }

    inline void set(int y = 0, int mo = 0, int d = 0, int h = 0, int mi = 0, int s = 0)
        {
        sec = s;
        minute = mi;
        hour = h;
        month = mo;
        day = d;

        if (year != y && y != 0)
            {
            year = y;
            setSummerStart();
            }
        }

//    friend Date_time fromS2W(const Date_time &dt);

    friend bool operator<(const Date_time &l, const Date_time &r)
        { //@form:off
        return std::tie(l.year, l.month, l.day, l.hour, l.minute, l.sec)
                < std::tie(r.year, r.month, r.day, r.hour, r.minute, r.sec);
        }                                                   //@form:on

    inline bool friend operator>(const Date_time &lhs, const Date_time &rhs)
        {
        return rhs < lhs;
        }
    inline bool friend operator<=(const Date_time &lhs, const Date_time &rhs)
        {
        return !(lhs > rhs);
        }
    inline bool friend operator>=(const Date_time &lhs, const Date_time &rhs)
        {
        return !(lhs < rhs);
        }

    friend bool operator ==(const Date_time &lhs, const Date_time &rhs)
        {
        return ((lhs.sec == rhs.sec) && (lhs.minute == rhs.minute)
                && (lhs.hour == rhs.hour) && (lhs.day == rhs.day)
                && (lhs.month == rhs.month) && (lhs.year == rhs.year));
        }
    bool friend operator !=(const Date_time &dl, const Date_time &dh)
        {
        return !(dl == dh);
        }

    void add_sec();
    void add_min();
    void add_h();
    void add_d();
    void add_month();
    void add_y();

    uint8_t get_wday() const;
    uint8_t static get_wday(int y, int m, int d);
//    int get_weekNb() const;

    int getDay() const
        {
        return day;
        }
    void setDay(int _day)
        {
        day = _day;
        }

    int getHour() const
        {
        return hour;
        }
    void setHour(int _hour)
        {
        hour = _hour;
        }

    int getMonth() const
        {
        return month;
        }
    void setMonth(int _month)
        {
        month = _month;
        }

    int getYear() const
        {
        return year;
        }
    void setYear(int _year)
        {
        if (year != _year)
            {
            year = _year;
            setSummerStart();
            }
        }

    int getMinute() const
        {
        return minute;
        }
    void setMinute(int _minute)
        {
        minute = _minute;
        }

    int getSec() const
        {
        return sec;
        }
    void setSec(int _sec)
        {
        sec = _sec;
        }

    bool isSummer() const
        {
        return isSu;
        }

    bool isCestAnnoun() const
        {
        return cest_announ;
        }

    bool isLeapAnnoun() const
        {
        if (leap_s_will_be_intro
                && ((month == 7 && day == 1 && hour == 1)
                        || (month == 1 && day == 1 && hour == 0)))
            {
            return true;
            }

        return false;
        }

    bool willBeLeapS() const
        {
        return leap_s_will_be_intro;
        }

    void setLeapWillBe(bool leap_intro = false)
        {
        this->leap_s_will_be_intro = leap_intro;
        }

    int getLastWday(int mo, int wd)
        {
        if (year == 0)
            {
            return 0;
            }

        int lastDay = max_day(mo, year);
        uint8_t w = get_wday(year, mo, lastDay);
        int del = w - wd;
        lastDay -= del;

        return del >= 0 ? lastDay : lastDay - 7;
        }

    std::string getDateStr();

private:
    int sec = 0;   // 0..60 (60 if leaps sec)
    int minute;   // 0..23
    int hour;
    int day;
    int month;   // 1..12
    int year {0};            // 0..99

    int suFirst;
    int suLast;

    bool setSummerStart();

// from SERVICE DE LA ROTATION TERRESTRE DE L'IERS, Bulletin C
// unpredictable, message comes from observations 6 months in advance.
    bool leap_s_will_be_intro {false};
    bool cest_announ {false};
    bool isSu {true};
    };

inline int max_day(const Date_time &pdt)
    {
    return max_day(pdt.getMonth(), pdt.getYear());
    }

//int sex_2_parts(float sex, int *mm, int *sec);

#endif /* INC_DATIME_H_ */
