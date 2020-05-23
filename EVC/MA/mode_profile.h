#pragma once
#include <list>
#include "../Packets/80.h"
#include "../Position/distance.h"
#include "../Supervision/national_values.h"
struct mode_profile
{
    distance start;
    double length;
    Mode mode;
    bool start_SvL;
    double speed;
};
extern std::list<mode_profile> mode_profiles;
void set_mode_profile(ModeProfile profile, distance ref, bool infill)
{
    std::vector<MP_element_packet> mps;
    mps.push_back(profile.element);
    mps.insert(mps.end(), profile.elements.begin(), profile.elements.end());
    distance start = ref;
    for (auto it = mps.begin(); it != mps.end(); ++it) {
        start += it->D_MAMODE.get_value(profile.Q_SCALE);
        mode_profile p;
        p.start = start;
        p.length = it->L_MAMODE.get_value(profile.Q_SCALE);
        switch (it->M_MAMODE)
        {
            case M_MAMODE_t::OS:
                p.mode = Mode::OS;
                p.speed = V_NVONSIGHT;
                break;
            case M_MAMODE_t::LS:
                p.mode = Mode::LS;
                p.speed = V_NVLIMSUPERV;
                break;
            case M_MAMODE_t::SH:
                p.mode = Mode::SH;
                p.speed = V_NVSHUNT;
                break;
        }
        p.start_SvL = it->Q_MAMODE==Q_MAMODE_t::BeginningIsSvL;
        if (it->V_MAMODE != V_MAMODE_t::UseNationalValue)
            p.speed = it->V_MAMODE.get_value();
    }
}