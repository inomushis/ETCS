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
#pragma once
#include <list>
#include <memory>
#include <functional>
#include "../Position/distance.h"
#include "../Position/linking.h"
#include "messages.h"
struct etcs_information
{
    int index_level;
    int index_mode;
    std::function<void()> handle_fun;
    virtual void handle() {handle_fun();}
    distance ref;
    int64_t timestamp;
    bool infill;
    communication_session *fromRBC;
    int dir;
    bg_id nid_bg;
    int version;
    bool reevaluated = false;
    optional<std::shared_ptr<euroradio_message>> message;
    std::list<std::shared_ptr<ETCS_packet>> linked_packets;
    etcs_information() : index_mode(-1), index_level(-1){}
    etcs_information(int index) : index_mode(index), index_level(index){}
    etcs_information(int index_level, int index_mode, std::function<void()> fun = nullptr) : index_level(index_level), index_mode(index_mode), handle_fun(fun) {}
};
void try_handle_information(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message);