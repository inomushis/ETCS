#pragma once
#include <set>
#include "../antenna.h"
#include <string>
enum brake_position_types
{
    PassengerP,
    FreightP,
    FreightG
};
extern brake_position_types brake_position;
extern double A_ebmax;
extern double L_TRAIN;
extern double T_brake_emergency;
extern double T_brake_service; 
extern double T_traction_cutoff;
extern double M_rotating_nom;
extern double V_train;
extern bool Q_airtight;
extern int axle_number;
extern int brake_percentage;
extern int cant_deficiency;
extern std::set<int> other_train_categories;
extern std::string special_train_data;
bool train_data_valid();
void validate_train_data();
void invalidate_train_data();