#ifndef VALUEPARSER_H
#define VALUEPARSER_H

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

inline double parseValue(string valStr) {
    if (valStr.empty()) {
        throw invalid_argument("Input string for value parsing is empty.");
    }

    transform(valStr.begin(), valStr.end(), valStr.begin(),
              [](unsigned char c){ return tolower(c); });

    double multiplier = 1.0;
    string number_part = valStr;

    if (valStr.length() > 3 && valStr.substr(valStr.length() - 3) == "meg") {
        multiplier = 1e6;
        number_part = valStr.substr(0, valStr.length() - 3);
    } else {
        char suffix = valStr.back();
        if (isalpha(suffix)) {
            number_part = valStr.substr(0, valStr.length() - 1);
            switch (suffix) {
                case 'g': multiplier = 1e9; break;
                case 'k': multiplier = 1e3; break;
                case 'm': multiplier = 1e-3; break;
                case 'u': multiplier = 1e-6; break;
                case 'n': multiplier = 1e-9; break;
                case 'p': multiplier = 1e-12; break;
                case 'f': multiplier = 1e-15; break;
                default:
                    throw invalid_argument("Invalid unit suffix: " + string(1, suffix));
            }
        }
    }

    stringstream ss(number_part);
    double baseVal;
    ss >> baseVal;

    if (ss.fail() || !ss.eof()) {
        throw invalid_argument("Invalid number format: '" + number_part + "'");
    }

    return baseVal * multiplier;
}

#endif //VALUEPARSER_H
