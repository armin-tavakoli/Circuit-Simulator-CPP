#ifndef DIODEMODEL_H
#define DIODEMODEL_H

#include <string>

using namespace std;

// ساختاری برای نگهداری پارامترهای یک مدل دیود
struct DiodeModel {
    string name;

    // پارامترهای دیود استاندارد
    double Is = 1e-14;  // Saturation Current
    double Vt = 0.02585; // Thermal Voltage
    double n = 1.0;      // Ideality factor

    // پارامترهای اضافی برای دیود زنر
    double Vz = -1.0; // Zener Breakdown Voltage (مقدار منفی نشان‌دهنده غیرفعال بودن است)
};

#endif // DIODEMODEL_H
