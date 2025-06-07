#include "Circuit.h"
#include <iostream>
#include <iomanip>
#include <set>
#include <map>

void Circuit::addComponent(unique_ptr<Component> component) {
    components.push_back(move(component));
}

void Circuit::printCircuit(char type) const {
    cout << "--- Circuit Components List ---" << endl;
    if (components.empty()) {
        cout << "The circuit is empty." << endl;
    } else {
        bool found = false;
        char filterType = toupper(type);
        for (const auto& comp : components) {
            if (filterType == 'A' || toupper(comp->getName()[0]) == filterType) {
                comp->print();
                found = true;
            }
        }
        if (!found && filterType != 'A') {
            cout << "No components of type '" << type << "' found." << endl;
        }
    }
    cout << "-----------------------------" << endl;
}

bool Circuit::hasComponent(const string& name) const {
    for (const auto& comp : components) {
        if (comp->getName() == name) return true;
    }
    return false;
}

bool Circuit::removeComponent(const string& name) {
    for (auto it = components.begin(); it != components.end(); ++it) {
        if ((*it)->getName() == name) {
            components.erase(it);
            return true;
        }
    }
    return false;
}

void Circuit::clear() {
    components.clear();
    currentComponentMap.clear();
    nodeCount = 0;
    currentVarCount = 0;
    cout << "Circuit has been reset." << endl;
}

set<int> Circuit::getNodes() const {
    set<int> nodes;
    for (const auto& comp : components) {
        nodes.insert(comp->getNode1());
        nodes.insert(comp->getNode2());
    }
    return nodes;
}

void Circuit::analyzeCircuit() {
    set<int> nodes;
    currentVarCount = 0;
    currentComponentMap.clear();
    for (const auto& comp : components) {
        nodes.insert(comp->getNode1());
        nodes.insert(comp->getNode2());
        if (comp->addsCurrentVariable()) {
            currentVarCount++;
            currentComponentMap[comp->getName()] = currentVarCount;
        }
    }
    nodeCount = nodes.empty() ? 0 : *nodes.rbegin();
}

void Circuit::runTransientAnalysis(double endTime, double timeStep) {
    analyzeCircuit();
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    cout << "--- Starting Transient Analysis ---" << endl;
    cout << "Nodes: " << nodeCount << ", Current Vars: " << currentVarCount << ", Matrix: " << matrix_size << "x" << matrix_size << endl;

    cout << left << setw(12) << "Time(s)";
    for (int i = 1; i <= nodeCount; ++i) cout << setw(12) << ("V(" + to_string(i) + ")");
    map<int, string> reverseCurrentMap;
    for(const auto& pair : currentComponentMap) reverseCurrentMap[pair.second] = pair.first;
    for(const auto& pair : reverseCurrentMap) cout << setw(12) << ("I(" + pair.second + ")");
    cout << endl;

    for (double t = 0; t <= endTime; t += timeStep) {
        MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
        VectorXd b = VectorXd::Zero(matrix_size);

        for (const auto& comp : components) {
            int final_current_idx = -1;
            if (comp->addsCurrentVariable()) {
                int m_idx = currentComponentMap.at(comp->getName());
                final_current_idx = nodeCount + m_idx - 1;
            }
            comp->stamp(A, b, final_current_idx, timeStep, t);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b);

        cout << left << setw(12) << fixed << setprecision(6) << t;
        for (int i = 0; i < nodeCount; ++i) cout << setw(12) << fixed << setprecision(6) << x(i);
        for (const auto& pair : reverseCurrentMap) {
            int current_idx = nodeCount + pair.first - 1;
            cout << setw(12) << fixed << setprecision(6) << x(current_idx);
        }
        cout << endl;

        for (auto& comp : components) {
            if (auto cap = dynamic_cast<Capacitor*>(comp.get())) {
                double v1 = (cap->getNode1() > 0) ? x(cap->getNode1() - 1) : 0.0;
                double v2 = (cap->getNode2() > 0) ? x(cap->getNode2() - 1) : 0.0;
                cap->updateVoltage(v1 - v2);
            }
            if (auto ind = dynamic_cast<Inductor*>(comp.get())) {
                int m_idx = currentComponentMap.at(ind->getName());
                ind->updateCurrent(x(nodeCount + m_idx - 1));
            }
        }
    }
    cout << "Transient analysis finished." << endl;
}
