#ifndef WIREINFO_H
#define WIREINFO_H

#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

struct PointData {
    double x = 0.0;
    double y = 0.0;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(x), CEREAL_NVP(y));
    }
};

struct WireInfo {
    std::string startCompName;
    int startTerminalId = -1;

    std::string endCompName;
    int endTerminalId = -1;

    std::vector<PointData> points;

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(startCompName), CEREAL_NVP(startTerminalId),
           CEREAL_NVP(endCompName), CEREAL_NVP(endTerminalId),
           CEREAL_NVP(points));
    }
};

#endif //WIREINFO_H
