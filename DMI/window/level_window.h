#ifndef _LEVEL_WINDOW_H
#define _LEVEL_WINDOW_H
#include "data_entry.h"
class level_window : public input_window
{
    public:
    level_window();
    void sendInformation() override;
};
class level_input : public input_data
{
    public:
    level_input();
    void validate() override;
};
#endif