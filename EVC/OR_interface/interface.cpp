/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "interface.h"
#include <orts/client.h>
#include <orts/common.h>
#include <set>
#include <bitset>
#include <algorithm>
#include "../Position/distance.h"
#include "../Supervision/speed_profile.h"
#include "../MA/movement_authority.h"
#include "../Position/linking.h"
#include "../Packets/messages.h"
#include "../TrainSubsystems/power.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
using namespace ORserver;
using std::string;
using std::cout;
using std::endl;
using std::thread;
using std::mutex;
extern mutex loop_mtx;
extern std::condition_variable evc_cv;
extern double V_est;
double V_set;
extern distance d_estfront;
extern bool EB_commanded;
extern bool SB_commanded;
extern bool desk_open;
extern bool sleep_signal;
double or_dist;
POSIXclient *s_client;
ParameterManager manager;
mutex iface_mtx;
static threadwait *poller;
//std::list<euroradio_message_traintotrack> pendingmessages;
void parse_command(string str, bool lock);
static unsigned char base_64_dec[256];
const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::vector<bool> base64_decode(std::string str, bool remove_padding=true)
{
    std::vector<bool> bits;
    for (int i=0; i<str.size(); i++) {
        if (str[i] == '=') break;
        unsigned char c = base_64_dec[str[i]];
        for (int j=5; j>=0; j--) {
            bits.push_back(((c>>j) & 1));
        }
    }
    if (remove_padding) bits.erase(bits.end()-bits.size()%8, bits.end());
    return bits;
}
void SetParameters()
{
    std::unique_lock<mutex> lck(iface_mtx);
    Parameter *p = new Parameter("distance");
    p->SetValue = [](string val) {
        or_dist = stof(val);
        if (odometer_value > or_dist)
            odometer_direction = -1;
        else if (odometer_value < or_dist)
            odometer_direction = 1;
        
        odometer_value = or_dist;
    };
    manager.AddParameter(p);

    p = new Parameter("speed");
    p->SetValue = [](string val) {
        double prev = V_est;
        V_est = stof(val)/3.6;
        V_ura = 0.007*V_est;
        if (V_est < 0.2)
        {
            V_est = V_ura = 0;
        }
        if (prev != 0 && V_est == 0)
            position_report_reasons[0] = true;
    };
    manager.AddParameter(p);

    p = new Parameter("acceleration");
    p->SetValue = [](string val) {
        A_est = stof(val);
    };
    manager.AddParameter(p);

    p = new Parameter("master_key");
    p->SetValue = [](string val) {
        desk_open = val == "1";
        sleep_signal = !desk_open;
    };
    manager.AddParameter(p);

    p = new Parameter("controller::direction");
    p->SetValue = [](string val) {
        reverser_direction = stoi(val);
    };
    manager.AddParameter(p);

    p = new Parameter("train_orientation");
    p->SetValue = [](string val) {
        current_odometer_orientation = stoi(val);
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::telegram");
    p->SetValue = [](string val) {
        std::vector<bool> message;
        for (int i=0; i<val.size(); i++) {
            message.push_back(val[i]=='1');
        }
        bit_manipulator r(message);
        eurobalise_telegram t(r);
        pending_telegrams.push_back({t,distance(odometer_value-odometer_reference, odometer_orientation, 0)});
        evc_cv.notify_all();
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::emergency");
    p->GetValue = []() {
        return EB_commanded ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::fullbrake");
    p->GetValue = []() {
        return SB_commanded ? "true" : "false";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::tractioncutoff");
    p->GetValue = []() {
        return traction_cutoff_status ? "false" : "true";
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::neutral_section");
    p->GetValue = []() {
        return (neutral_section_info.start ? std::to_string(*neutral_section_info.start) : "")+";"+(neutral_section_info.end ? std::to_string(*neutral_section_info.end) : "");
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::lower_pantographs");
    p->GetValue = []() {
        return (lower_pantograph_info.start ? std::to_string(*lower_pantograph_info.start) : "")+";"+(lower_pantograph_info.end ? std::to_string(*lower_pantograph_info.end) : "");
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::atf");
    p->GetValue = []() {
        if (mode != Mode::FS) return std::string("-1");
        double atf = V_perm;
        if (V_target < V_perm) atf = std::max(V_perm - 1, V_target);
        return std::to_string(atf);
    };
    manager.AddParameter(p);

    p = new Parameter("cruise_speed");
    p->SetValue = [](string val) {
        V_set = stof(val)/3.6;
    };
    manager.AddParameter(p);
    
    p = new Parameter("etcs::vperm");
    p->GetValue = []() {
        return std::to_string(V_perm*3.6);
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::vtarget");
    p->GetValue = []() {
        return std::to_string(V_target*3.6);
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::vsbi");
    p->GetValue = []() {
        return std::to_string(V_sbi*3.6);
    };
    manager.AddParameter(p);
    
    p = new Parameter("etcs::supervision");
    p->GetValue = []() {
        string s = "";
        extern SupervisionStatus supervision;
        switch(supervision)
        {
            case NoS:
                s += "NoS";
                break;
            case IndS:
                s += "IndS";
                break;
            case OvS:
                s += "OvS";
                break;
            case WaS:
                s += "WaS";
                break;
            case IntS:
                s += "IntS";
                break;
        }
        return s;
    };
    manager.AddParameter(p);

    p = new Parameter("etcs::dmi::feedback");
    p->SetValue = [](string val) {
        parse_command(val, false);
    };
    manager.AddParameter(p);
}
void register_parameter(string parameter)
{
    s_client->WriteLine("register("+parameter+")");
}
#include <unistd.h>
void polling()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    while(s_client->connected) {
        int nfds = poller->poll(300);
        s_client->handle();
        string s = s_client->ReadLine();
        std::unique_lock<mutex> lck(iface_mtx);
        std::unique_lock<mutex> lck2(loop_mtx);
        while(s!="") {
            manager.ParseLine(s_client, s);
            s = s_client->ReadLine();
        }
        std::for_each(manager.parameters.begin(), manager.parameters.end(), [](Parameter* p){p->Send();});
    }
}
void start_or_iface()
{
    for (int i=0; i<64; i++)
    {
        base_64_dec[base64_table[i]]=i;
    }
	base_64_dec['='] = 0;
    poller = new threadwait();
    s_client = TCPclient::connect_to_server(poller);
    s_client->WriteLine("register(speed)");
    s_client->WriteLine("register(distance)");
    s_client->WriteLine("register(acceleration)");
    s_client->WriteLine("register(etcs::telegram)");
    s_client->WriteLine("register(cruise_speed)");
    s_client->WriteLine("register(etcs::dmi::feedback)");
    s_client->WriteLine("register(master_key)");
    s_client->WriteLine("register(controller::direction)");
    s_client->WriteLine("register(train_orientation)");
    SetParameters();
    thread t(polling);
    t.detach();
}
