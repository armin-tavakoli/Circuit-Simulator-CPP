#include "Circuit.h"
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <queue>
#include <fstream>
#include <cmath>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include "cereal_registration.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Circuit::saveToFile(const std::string& filepath) {
    std::ofstream os(filepath, std::ios::binary);
    if (!os) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }
    cereal::BinaryOutputArchive archive(os);

    archive(components, wires);

    std::cout << "Circuit saved successfully to " << filepath << std::endl;
}

void Circuit::loadFromFile(const std::string& filepath) {
    std::ifstream is(filepath, std::ios::binary);
    if (!is) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }
    cereal::BinaryInputArchive archive(is);

    clear();

    archive(components, wires);

    std::cout << "Circuit loaded successfully from " << filepath << std::endl;
}

const map<string, vector<double>>& Circuit::getSimulationResults() const {
    return simulationResults;
}

void Circuit::runTransientAnalysis(double Tstop, double Tstep, const vector<PrintVariable>& printVars, double Tstart, double Tmaxstep) {
    analyzeCircuit();
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    simulationResults.clear();
    double actual_tstep = Tstep;
    if (Tmaxstep > 0 && Tmaxstep < Tstep) {
        actual_tstep = Tmaxstep;
    }

    bool hasNonLinear = false;
    for (const auto& comp : components) {
        if (comp->isNonLinear()) {
            hasNonLinear = true;
            break;
        }
    }

    VectorXd x = VectorXd::Zero(matrix_size);
    VectorXd x_prev_t = VectorXd::Zero(matrix_size);

    for (double t = 0; t <= Tstop; t += actual_tstep) {
        VectorXd x_nr_guess = x_prev_t;
        if (hasNonLinear) {
            const int MAX_NR_ITER = 100;
            const double NR_TOLERANCE = 1e-6;
            for (int i = 0; i < MAX_NR_ITER; ++i) {
                MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
                VectorXd b = VectorXd::Zero(matrix_size);
                for (const auto& comp : components) {
                    int final_current_idx = -1;
                    if (comp->addsCurrentVariable()) {
                        final_current_idx = nodeCount + currentComponentMap.at(comp->getName()) - 1;
                    }
                    comp->stamp(A, b, x_nr_guess, final_current_idx, actual_tstep, t);
                }
                VectorXd x_next_nr = A.colPivHouseholderQr().solve(b);
                if ((x_next_nr - x_nr_guess).norm() < NR_TOLERANCE) {
                    x_nr_guess = x_next_nr;
                    break;
                }
                x_nr_guess = x_next_nr;
                if (i == MAX_NR_ITER - 1) {
                    cout << "Warning: Newton-Raphson did not converge at t=" << t << endl;
                }
            }
        } else {
            MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
            VectorXd b = VectorXd::Zero(matrix_size);
            for (const auto& comp : components) {
                int final_current_idx = -1;
                if (comp->addsCurrentVariable()) {
                    final_current_idx = nodeCount + currentComponentMap.at(comp->getName()) - 1;
                }
                comp->stamp(A, b, x_nr_guess, final_current_idx, actual_tstep, t);
            }
            x_nr_guess = A.colPivHouseholderQr().solve(b);
        }

        x = x_nr_guess;

        if (t >= Tstart) {
            simulationResults["Time"].push_back(t);
            for (int i = 0; i < nodeCount; ++i) {
                string varName = "V(" + to_string(i + 1) + ")";
                simulationResults[varName].push_back(x(i));
            }
            for (const auto& pair : currentComponentMap) {
                string varName = "I(" + pair.first + ")";
                simulationResults[varName].push_back(x(nodeCount + pair.second - 1));
            }
        }

        x_prev_t = x;
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

void Circuit::runACAnalysis(double startFreq, double stopFreq, int numPoints, const string& sweepType, const vector<PrintVariable>& printVars) {
    analyzeCircuit();
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    bool hasACSource = false;
    for (const auto& comp : components) {
        if (dynamic_cast<ACVoltageSource*>(comp.get())) {
            hasACSource = true;
            break;
        }
    }
    if (!hasACSource) {
        throw runtime_error("AC analysis requires at least one AC Voltage Source in the circuit.");
    }

    simulationResults.clear();
    simulationResults["Frequency"];

    if (printVars.empty()) {
        for (int i = 0; i < nodeCount; ++i) {
            simulationResults["V(" + to_string(i + 1) + ")"];
        }
        for (const auto& pair : currentComponentMap) {
            simulationResults["I(" + pair.first + ")"];
        }
    } else {
        for (const auto& var : printVars) {
            if (toupper(var.type) == 'V') {
                simulationResults["V(" + var.id + ")"];
            } else if (toupper(var.type) == 'I') {
                simulationResults["I(" + var.id + ")"];
            }
        }
    }

    cout << "--- Starting AC Sweep Analysis ---" << endl;

    for (int i = 0; i < numPoints; ++i) {
        double freq = 0;
        if (numPoints == 1) {
            freq = startFreq;
        } else if (sweepType == "Linear") {
            freq = startFreq + i * (stopFreq - startFreq) / (numPoints - 1);
        } else if (sweepType == "Decade") {
            freq = startFreq * pow(10.0, i * (log10(stopFreq / startFreq)) / (numPoints - 1));
        } else if (sweepType == "Octave") {
            freq = startFreq * pow(2.0, i * (log2(stopFreq / startFreq)) / (numPoints - 1));
        }

        if (freq == 0 && startFreq != 0) continue;
        double omega = 2 * M_PI * freq;

        MatrixXcd A = MatrixXcd::Zero(matrix_size, matrix_size);
        VectorXcd b = VectorXcd::Zero(matrix_size);

        for (const auto& comp : components) {
            int final_current_idx = -1;
            if (comp->addsCurrentVariable()) {
                final_current_idx = nodeCount + currentComponentMap.at(comp->getName()) - 1;
            }
            comp->stampAC(A, b, final_current_idx, omega);
        }

        VectorXcd x = A.colPivHouseholderQr().solve(b);

        simulationResults["Frequency"].push_back(freq);
        for (auto const& [key, val] : simulationResults) {
            if (key == "Frequency") continue;

            char type = toupper(key[0]);
            string id = key.substr(key.find("(") + 1, key.find(")") - key.find("(") - 1);

            if (type == 'V') {
                int nodeNum = stoi(id);
                if (nodeNum > 0 && nodeNum <= nodeCount) {
                    double magnitude = std::abs(x(nodeNum - 1));
                    simulationResults[key].push_back(magnitude);
                }
            } else if (type == 'I') {
                if (currentComponentMap.count(id)) {
                    int m_idx = currentComponentMap.at(id);
                    double magnitude = std::abs(x(nodeCount + m_idx - 1));
                    simulationResults[key].push_back(magnitude);
                }
            }
        }
    }
    cout << "AC Sweep analysis finished." << endl;
}

void Circuit::runDCSweep(const string& sweepSourceName, double startVal, double endVal, double increment, const vector<PrintVariable>& printVars) {
    analyzeCircuit();

    Component* sweepSource = findComponent(sweepSourceName);
    if (!sweepSource) {
        throw runtime_error("Sweep source '" + sweepSourceName + "' not found.");
    }

    string propToSweep;
    if (dynamic_cast<VoltageSource*>(sweepSource)) {
        propToSweep = "Voltage";
    } else if (dynamic_cast<CurrentSource*>(sweepSource)) {
        propToSweep = "Current";
    } else {
        throw runtime_error("DC Sweep can only be performed on a DC Voltage or Current source.");
    }

    const double h_dc = 1e12;

    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    vector<int> printIndices;
    vector<string> printHeaders;
    printHeaders.push_back(sweepSourceName);

    for (const auto& var : printVars) {
        if (toupper(var.type) == 'V') {
            int nodeNum = stoi(var.id);
            if (nodeNum > 0 && nodeNum <= nodeCount) {
                printIndices.push_back(nodeNum - 1);
                printHeaders.push_back("V(" + var.id + ")");
            }
        } else if (toupper(var.type) == 'I') {
            if (currentComponentMap.count(var.id)) {
                int m_idx = currentComponentMap.at(var.id);
                printIndices.push_back(nodeCount + m_idx - 1);
                printHeaders.push_back("I(" + var.id + ")");
            }
        }
    }

    cout << "--- Starting DC Sweep Analysis ---" << endl;
    for(const auto& header : printHeaders) {
        cout << left << setw(15) << header;
    }
    cout << endl;

    for (double sweepVal = startVal; sweepVal <= endVal; sweepVal += increment) {
        sweepSource->setProperties({{propToSweep, sweepVal}});

        VectorXd x = VectorXd::Zero(matrix_size);
        const int MAX_NR_ITER = 100;
        const double NR_TOLERANCE = 1e-6;

        for (int i = 0; i < MAX_NR_ITER; ++i) {
            MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
            VectorXd b = VectorXd::Zero(matrix_size);
            for (const auto& comp : components) {
                int final_current_idx = -1;
                if (comp->addsCurrentVariable()) {
                    final_current_idx = nodeCount + currentComponentMap.at(comp->getName()) - 1;
                }
                comp->stamp(A, b, x, final_current_idx, h_dc, 0.0);
            }
            VectorXd x_next = A.colPivHouseholderQr().solve(b);
            if ((x_next - x).norm() < NR_TOLERANCE) {
                x = x_next;
                break;
            }
            x = x_next;
            if (i == MAX_NR_ITER - 1) {
                cout << "Warning: Newton-Raphson did not converge for sweep value " << sweepVal << endl;
            }
        }

        cout << left << setw(15) << fixed << setprecision(6) << sweepVal;
        for (int idx : printIndices) {
            cout << setw(15) << fixed << setprecision(6) << x(idx);
        }
        cout << endl;
    }
    cout << "DC Sweep analysis finished." << endl;
}

void Circuit::checkConnectivity() const {
    if (components.empty()) {
        return;
    }
    set<int> allNodes = getNodes();
    if (allNodes.find(0) == allNodes.end()) {
        throw runtime_error("Error: No ground node (0) detected in the circuit. Analysis requires a ground reference.");
    }
    map<int, vector<int>> adjList;
    for (const auto& comp : components) {
        int n1 = comp->getNode1();
        int n2 = comp->getNode2();
        adjList[n1].push_back(n2);
        adjList[n2].push_back(n1);
        if (auto vcvs = dynamic_cast<const VCVS*>(comp.get())) {
            adjList[vcvs->getCtrlNode1()].push_back(vcvs->getCtrlNode2());
            adjList[vcvs->getCtrlNode2()].push_back(vcvs->getCtrlNode1());
        }
        if (auto vccs = dynamic_cast<const VCCS*>(comp.get())) {
            adjList[vccs->getCtrlNode1()].push_back(vccs->getCtrlNode2());
            adjList[vccs->getCtrlNode2()].push_back(vccs->getCtrlNode1());
        }
    }
    set<int> visitedNodes;
    queue<int> q;
    q.push(0);
    visitedNodes.insert(0);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (adjList.count(u)) {
            for (int v : adjList.at(u)) {
                if (visitedNodes.find(v) == visitedNodes.end()) {
                    visitedNodes.insert(v);
                    q.push(v);
                }
            }
        }
    }
    if (visitedNodes.size() != allNodes.size()) {
        string errorMsg = "Error: Circuit is not fully connected. Floating nodes/islands detected. Reachable nodes from ground: ";
        for(int node : visitedNodes) { errorMsg += to_string(node) + " "; }
        errorMsg += ". Unreachable nodes: ";
        for(int node : allNodes) {
            if(visitedNodes.find(node) == visitedNodes.end()){
                errorMsg += to_string(node) + " ";
            }
        }
        throw runtime_error(errorMsg);
    }
}

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
        if(auto vcvs = dynamic_cast<const VCVS*>(comp.get())) {
            nodes.insert(vcvs->getCtrlNode1());
            nodes.insert(vcvs->getCtrlNode2());
        }
        if(auto vccs = dynamic_cast<const VCCS*>(comp.get())) {
            nodes.insert(vccs->getCtrlNode1());
            nodes.insert(vccs->getCtrlNode2());
        }
    }
    return nodes;
}

void Circuit::renameNode(int oldNode, int newNode) {
    for (auto& comp : components) {
        comp->updateNode(oldNode, newNode);
    }
}

void Circuit::analyzeCircuit() {
    set<int> nodes;
    currentVarCount = 0;
    currentComponentMap.clear();
    for (const auto& comp : components) {
        nodes.insert(comp->getNode1());
        nodes.insert(comp->getNode2());
        if(auto vcvs = dynamic_cast<const VCVS*>(comp.get())) {
            nodes.insert(vcvs->getCtrlNode1());
            nodes.insert(vcvs->getCtrlNode2());
        }
        if(auto vccs = dynamic_cast<const VCCS*>(comp.get())) {
            nodes.insert(vccs->getCtrlNode1());
            nodes.insert(vccs->getCtrlNode2());
        }
        if (comp->addsCurrentVariable()) {
            currentVarCount++;
            currentComponentMap[comp->getName()] = currentVarCount;
        }
    }
    nodeCount = nodes.empty() ? 0 : *nodes.rbegin();

    for (auto& comp : components) {
        string ctrlName = comp->getCtrlVName();
        if (!ctrlName.empty()) {
            if (currentComponentMap.count(ctrlName)) {
                int ctrl_idx = nodeCount + currentComponentMap.at(ctrlName) - 1;
                comp->setCtrlCurrentIdx(ctrl_idx);
            } else {
                throw runtime_error("Dependent source '" + comp->getName() + "' has an undefined control source '" + ctrlName + "'");
            }
        }
    }

    checkConnectivity();
}

Component* Circuit::findComponent(const string& name) {
    for (auto& comp : components) {
        if (comp->getName() == name) {
            return comp.get();
        }
    }
    return nullptr;
}

const vector<unique_ptr<Component>>& Circuit::getComponents() const {
    return components;
}
