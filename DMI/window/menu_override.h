/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _MENU_OVERRIDE_H
#define _MENU_OVERRIDE_H
#include "menu.h"
class menu_override : public menu
{
    public:
    menu_override();
    void setEnabled(bool eoa);
};
#endif