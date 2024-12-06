/*
 * \file GPS.cpp
 *
 *  Created on: Nov 9, 2024
 *      Author: Kris Jaxa
 */

#include "MyUtil.h"
#include "GPS.h"
#include "GPSsat.h"

static int totNbMsg_exp = 0;
static int currNbMsg_exp = 0;

// commands to setup gps unit:
const char GPS::SET_NMEA_BAUDRATE[] = "$PMTK251,115200*1F\r\n";
const char GPS::SET_NMEA_UPDATERATE[] = "$PMTK220,250*29\r\n"; //
const char GPS::API_SET_OUTPUT[] = "$PMTK314,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";

const std::string GPS::GSV {"$GPGSV"};
const std::string GPS::RMC {"$GPRMC"};

int GPS::gps_second;
int GPS::gps_minute;
int GPS::gps_hour;
int GPS::gps_year;
int GPS::gps_month;
int GPS::gps_day;
int GPS::gps_sattNumb {0};

int GPS::sentenceID;
uint8_t GPS::acquired {0};
uint8_t GPS::gps_rx[RXSZ];

void GPS::unset()
    {
    gps_second = -1;
    gps_minute = -1;
    gps_hour = -1;
    gps_year = -1;
    gps_month = -1;
    gps_day = -1;
    gps_sattNumb = 0;
    }

bool GPS::set_time(const std::string_view &t)
    {
//hhmmss.ccc ignore ccc
    gps_hour = get_nd(t, 0, 2);
    gps_minute = get_nd(t, 2, 2);
    gps_second = get_nd(t, 4, 2);

    if (gps_second == -1 || gps_minute == -1 || gps_hour == -1)
        {
        return false;
        }

    acquired |= RCM_FLT;
    return true;
    }

void GPS::update_strong()
    {
    SVs::strongest_sats.clear();

    for (auto const& [key, val] : SVs::new_spaceVehicles)
        {
        SVs::strongest_sats.push_back( {key, val.SNR});
        }

    // strongest satellite first
    std::sort(std::begin(SVs::strongest_sats), std::end(SVs::strongest_sats),
            [](Strong a, Strong b)
                {return a.snr > b.snr;});
    }

bool GPS::set_date(const std::string_view &d)
    {
    gps_day = get_nd(d, 0, 2);
    gps_month = get_nd(d, 2, 2);
    gps_year = get_nd(d, 4, 2);
    if (gps_day == -1 || gps_month == -1 || gps_year == -1)
        {
        return false;
        }

    acquired |= RCM_FLD;
    return true;
    }

void GPS::read_data()
    {
    totNbMsg_exp = 0;
    currNbMsg_exp = 0;
    SVs::new_spaceVehicles.clear();
    acquired = 0;
    unset();

    parse_sentences();

    if ((acquired & GSV_FLS) == GSV_FLS)
        {
        update_strong();
        }
    }

void GPS::parse_sentences()
    {
// can be more sentences in one packet, split and then split again
    auto split_values = split((char*) gps_rx, "\r\n");

    for (unsigned int i = 0; i < split_values.size(); ++i)
        {
        if (!cntrCheckSum(split_values[i]))
            {
            continue;
            }

        auto fields = split(split_values[i], ",");
        if (!parse_sentence(fields)) continue;
        }
    }

bool GPS::check_what_sentence(const std::string_view &field)
    {
    sentenceID = 0;

    if (field == RMC)
        {
        sentenceID = RMC_ID;
        return true;
        }

    if (field == GSV)
        {
        sentenceID = GSV_ID;
        return true;
        }

    return false;
    }

bool GPS::parse_sentence(const std::vector<std::string_view> &fields)
    {

    static int id_pos = 4;   // then +4
    unsigned int elev_pos = 5;   // ..
    unsigned int azim_pos = 6;
    unsigned int srn_pos = 7;

    int totNbMsg_find;
    int currMsgNb_find;
    GPSsat gpsSV;
    int v;
    std::vector<std::string_view> last;

    if (check_what_sentence(fields[0]))
        {

        switch (sentenceID)
            {
        //1      2      3 4         5 6          7 8  9  0      1   2 3
        //$GPRMC,001225,A,2832.1834,N,08101.0536,W,12,25,251211,1.2,E,A*03
        case RMC_ID:
            if ((acquired & RCM_FLTD) == RCM_FLTD)
                {
                return true; // already in-read
                }

            if (fields[1].empty() || !set_time(fields[1]))
                {
                acquired &= ~RCM_FLTD;
                return false;
                }

            if (fields[9].empty() || !set_date(fields[9]))
                {
                acquired &= ~RCM_FLTD;
                return false;
                }

            acquired |= RCM_FLTD;
            break;

            //id{Satellite ID}, E{elevation}, Az{Azimuth}, S{SNR}
//          0      1 2 3  4  5  6   7  8  9  0   1  2  3  4   5  6  7  18 19
//                        id E  Az  S  id E  Az  S  id E  Az  S  id E  Az  S
//          $GPGSV,3,1,11,10,68,137,19,27,65,176,28,08,63,271,33,23,45,063,29*72

        case GSV_ID:
            ++currNbMsg_exp;

            if ((acquired & GSV_FLS) == GSV_FLS)
                {
                return true; // satellite nb. already in-read
                }
            // sat. info can take up to 3 messages
            totNbMsg_find = get_nd(fields[1], 0, fields[1].size());
            if (totNbMsg_find < 1)
                {
                return false;
                }
            if (currNbMsg_exp == 1)
                {
                totNbMsg_exp = totNbMsg_find;
                }

            currMsgNb_find = get_nd(fields[2], 0, fields[2].size());
            if (currMsgNb_find < 1)
                {
                return false;
                }

            // message not in sequence
            if (totNbMsg_exp != totNbMsg_find || currMsgNb_find != currNbMsg_exp)
                {
                return false;
                }

            gps_sattNumb = get_nd(fields[3], 0, fields[3].size());
            if (gps_sattNumb < 1)
                {
                return false;
                }

            for (std::vector<std::basic_string_view<char>>::size_type i = 0;
                    i < (fields.size() - 4); i += 4)
                {
                if (!read_check(fields[id_pos + i], 1, 79, v))
                    {
                    return false;
                    }

                gpsSV.id = v;

                if (!read_check(fields[elev_pos + i], 0, 90, v))
                    {
                    return false;
                    }

                gpsSV.elevation = v;

                if (!read_check(fields[azim_pos + i], 0, 359, v))
                    {
                    return false;
                    }

                gpsSV.azimuth = v;

                // last field, special, there is a * and check sum in the field
                if (srn_pos + i == fields.size() - 1)
                    {
                    last = split(fields[srn_pos + i], "*");

                    if (!read_check(last[0], 0, 99, v))
                        {
                        return false;
                        }

                    gpsSV.SNR = v;
                    }
                else
                    {
                    if (!read_check(fields[srn_pos + i], 0, 99, v))
                        {
                        return false;
                        }
                    gpsSV.SNR = v;
                    }

                // if we are here one sat. is in-read
                GPSsat sat(gpsSV);
                SVs::new_spaceVehicles.insert( {sat.id, sat});
                if (SVs::new_spaceVehicles.size() == (uint32_t) gps_sattNumb)
                    {
                    acquired |= GSV_FLS;
                    }
                } //for (std::vector

            return true;

        default:
            return false;
            }
        } //if (check_what_sentence(fields[0]))

    return false;
    }

