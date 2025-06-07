#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <Eigen/Dense>
#include "Component.h"

using namespace std;
using namespace Eigen;

class Circuit {
public:
    void addComponent(unique_ptr<Component> component);
    bool removeComponent(const string& name);
    bool hasComponent(const string& name) const;
    void printCircuit(char type = 'A') const;
    void runTransientAnalysis(double endTime, double timeStep);
    void clear();
    set<int> getNodes() const;

private:
    vector<unique_ptr<Component>> components;
    map<string, int> currentComponentMap;
    int nodeCount = 0;
    int currentVarCount = 0;

    void analyzeCircuit();
};

#endif // CIRCUIT_H
