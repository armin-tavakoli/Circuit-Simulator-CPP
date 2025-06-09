#ifndef DIODEMODEL_H
#define DIODEMODEL_H

#include <string>

using namespace std;

struct DiodeModel {
    string name;
    double Is = 1e-14;
    double Vt = 0.02585;
    double n = 1.0;
    double Vz = -1.0;
};

#endif
