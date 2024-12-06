/*!
 * \file GPSsat.h
 * \brief
 *
 *  Created on: Nov 7, 2024
 *      Author: Kris Jaxa
 *            @ Jaxasoft, Freeware
 *              v.1.0.0
 */

#ifndef SATELLITE_H_
#define SATELLITE_H_

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <cmath>
#include "NDisplay.h"

extern NDisplay display;

struct GPSsat
    {
    uint8_t id;
    uint8_t elevation;
    uint8_t SNR;
    uint8_t flag;
    uint8_t indx;
    uint16_t azimuth;
    int xo;
    int yo;

    GPSsat()
        {
        id = 0;
        elevation = 90;
        azimuth = 0;
        SNR = 0;
        xo = X0;
        yo = Y0;
        flag = P;
        indx = 0;
        }

    GPSsat(const GPSsat &oth)
        {
        id = oth.id;
        elevation = oth.elevation;
        SNR = oth.SNR;
        azimuth = oth.azimuth;
        xy_c(elevation, azimuth);
        flag = oth.flag;
        set_indx();
        }

    GPSsat(uint8_t _id, uint8_t _elev, uint16_t _azim, uint8_t _snr) :
            id(_id), elevation(_elev), SNR(_snr), azimuth(_azim)
        {
        xy_c(_elev, _azim);
        flag = 0;
        set_indx();
        }

    // all but not id
    bool operator==(const GPSsat &oth) const
        {
        return !is_moved(oth) && (indx == oth.indx);
        }

    bool is_moved(const GPSsat &oth) const
        {
        return (elevation != oth.elevation) || (azimuth != oth.azimuth);
        }

    void xy_c(int alt, int azim)
        {
        float r = R * (1 - (float) alt / 90.0);
        xo = (int) std::round(r * sinf(D2R * azim) + X0);
        yo = (int) std::round(Y0 - r * cosf(D2R * azim));
        }

    static int get_ix(int snr)
        {
        if (snr > 35) return 0;

        if (snr > 30) return 1;

        if (snr > 25) return 2;

        return 3;
        }

    void set_indx()
        {
        indx = get_ix(SNR);
        }

    static const int X0 {119};
    static const int Y0 {122};
    static const int R {109};
    static constexpr float D2R {M_PI / 180};
    static const uint8_t P {0b10};
    static const uint8_t E {0b01};
    static const uint8_t PE {0b11};

    bool has2clear()
        {
        return (E & flag) == E;
        }
    bool has2paint()
        {
        return (P & flag) == P;
        }
    };


struct Strong
    {
    int id {0};
    int snr {0};
    };

struct SVs
    {
    SVs() = default;

    static const int W {20};
    static const int DW {-10};
    static const int DH {-10};
    static const int DCLR {-14};
    static const int WCLR {31};

    static char buf[64];

    static std::map<int, GPSsat> old_spaceVehicles;
    static std::map<int, GPSsat> new_spaceVehicles;
    static std::vector<Strong> strongest_sats;

    static void erase_sat(const GPSsat &s, const NDisplay &disp)
        {
        sprintf(buf, "picq %d,%d,%d,%d,0", s.xo + DCLR, s.yo + DCLR, WCLR, WCLR);
        disp.sendCommand(buf);
        }

    const static char *COLORS[4];
    static const int COLNUM[4];
    static int indx;

    static int dist(const GPSsat &a, const GPSsat &b)
        {
        return std::max(std::abs(a.xo - b.xo), std::abs(a.yo - b.yo));
        }

    static void draw_sat(const GPSsat &s, const NDisplay &disp)
        {
        sprintf(buf, "cirs %d,%d,14,%s", s.xo, s.yo, COLORS[s.indx]);
        disp.sendCommand(buf);

        sprintf(buf, "xstr %d,%d,%d,%d,4,BLACK,%s,1,1,1,\"%d\"", s.xo + DW, s.yo + DH, W,
                W, COLORS[s.indx], s.id);
        disp.sendCommand(buf);
        }

    // insert/update new sat/values and remove old ones, display them
    static void update_sats()
        {
        // erase vanished satellites
        //first, mark only
        for (auto k_v : old_spaceVehicles)
            {
            if (!new_spaceVehicles.contains(k_v.first))
                {
                k_v.second.flag = GPSsat::E;
                }
            }

        // mark satellites for redrawing (if close to erase area or display order [
        // strongest on top])
        // go through all pair
        for (auto im = old_spaceVehicles.begin(); im != old_spaceVehicles.end(); ++im)
            {
            auto ii = im;
            ++ii;

            for (; ii != old_spaceVehicles.end(); ++ii)
                {
                if (((ii->second.flag & GPSsat::PE) == 0)
                        && (dist(im->second, ii->second) >= WCLR))
                    {
                    ii->second.flag = GPSsat::P;
                    im->second.flag = GPSsat::P;
                    }
                }
            }

        //  clear vanished satellites
        auto it = old_spaceVehicles.begin();
        while (it != old_spaceVehicles.end())
            {
            if (new_spaceVehicles.contains(it->first))
                {
                ++it;
                }
            else
                {
                erase_sat(it->second, display);
                it = old_spaceVehicles.erase(it);
                }
            }

        // repaint
        for (auto i = strongest_sats.begin(); i != strongest_sats.end(); ++i)
            {
            if (old_spaceVehicles[i->id].has2clear())
                {
                erase_sat(old_spaceVehicles[i->id], display);
                }
            }

        for (auto i = strongest_sats.rbegin(); i != strongest_sats.rend(); ++i)
            {
            if (old_spaceVehicles[i->id].has2paint())
                {
                draw_sat(new_spaceVehicles[i->id], display);
                }

            old_spaceVehicles[i->id] = new_spaceVehicles[i->id];
            }
        }        //static void update_sats()

    };

#endif /* SATELLITE_H_ */
