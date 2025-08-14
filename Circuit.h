#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <Eigen/Dense>
#include "Component.h"
#include "PrintRequest.h"
#include "WireInfo.h"

// اضافه کردن هدرهای لازم برای سریال‌سازی
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

using namespace std;
using namespace Eigen;

struct TheveninEquivalent {
    double Vth = 0.0;
    double Rth = 0.0;
};

class Circuit {
public:
    Circuit() = default;
    unique_ptr<Circuit> clone() const;

    void addComponent(unique_ptr<Component> component);
    bool removeComponent(const string& name);
    bool hasComponent(const string& name) const;
    void printCircuit(char type = 'A') const;
    void runTransientAnalysis(double Tstop, double Tstep, const vector<PrintVariable>& printVars = {}, double Tstart = 0.0, double Tmaxstep = 0.0);
    void runACAnalysis(double startFreq, double stopFreq, int numPoints, const string& sweepType, const vector<PrintVariable>& printVars = {});
    void runPhaseAnalysis(double baseFreq, double startPhase, double stopPhase, int numPoints, const vector<PrintVariable>& printVars = {});
    void runDCSweep(const string& sweepSourceName, double startVal, double endVal, double increment, const vector<PrintVariable>& printVars);
    void clear();
    TheveninEquivalent calculateTheveninEquivalent(int port1_node, int port2_node);

    set<int> getNodes() const;
    int getNodeCount() const { return nodeCount; }
    int getCurrentVarCount() const { return currentVarCount; }

    void renameNode(int oldNode, int newNode);
    const vector<unique_ptr<Component>>& getComponents() const;
    Component* findComponent(const string& name);
    const map<string, vector<double>>& getSimulationResults() const;

    void saveToFile(const std::string& filepath);
    void loadFromFile(const std::string& filepath);

    vector<WireInfo>& getWires() { return wires; }

    void setExternalPorts(const std::vector<int>& ports) { m_externalPorts = ports; }
    const std::vector<int>& getExternalPorts() const { return m_externalPorts; }

    void analyzeCircuit();

    // --- تابع جدید اضافه شده برای سریال‌سازی ---
    template<class Archive>
    void serialize(Archive & ar) {
        ar(CEREAL_NVP(components), CEREAL_NVP(wires), CEREAL_NVP(m_externalPorts));
    }

private:
    vector<unique_ptr<Component>> components;
    vector<WireInfo> wires;
    vector<int> m_externalPorts;

    map<string, int> currentComponentMap;
    int nodeCount = 0;
    int currentVarCount = 0;
    map<string, vector<double>> simulationResults;

    void flattenCircuit();
    void checkConnectivity() const;
};

#endif
