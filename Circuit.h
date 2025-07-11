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

using namespace std;
using namespace Eigen;

class Circuit {
public:
    void addComponent(unique_ptr<Component> component);
    bool removeComponent(const string& name);
    bool hasComponent(const string& name) const;
    void printCircuit(char type = 'A') const;
    void runTransientAnalysis(double Tstop, double Tstep, const vector<PrintVariable>& printVars = {}, double Tstart = 0.0, double Tmaxstep = 0.0);
    void runACAnalysis(double startFreq, double stopFreq, int numPoints, const string& sweepType, const vector<PrintVariable>& printVars = {});
    void clear();
    set<int> getNodes() const;
    void renameNode(int oldNode, int newNode);
    const vector<unique_ptr<Component>>& getComponents() const;
    void runDCSweep(const string& sweepSourceName, double startVal, double endVal, double increment, const vector<PrintVariable>& printVars);
    Component* findComponent(const string& name);
    const map<string, vector<double>>& getSimulationResults() const;

    void saveToFile(const std::string& filepath);
    void loadFromFile(const std::string& filepath);

    vector<WireInfo>& getWires() { return wires; }

private:
    vector<unique_ptr<Component>> components;
    vector<WireInfo> wires;

    map<string, int> currentComponentMap;
    int nodeCount = 0;
    int currentVarCount = 0;
    map<string, vector<double>> simulationResults;

    void analyzeCircuit();
    void checkConnectivity() const;
};

#endif
