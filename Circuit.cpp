#include "Circuit.h"
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <queue> // For BFS algorithm to check connectivity

/**
 * @brief Checks if the circuit has a ground node and is fully connected.
 * Throws a runtime_error if the circuit is invalid.
 */
void Circuit::checkConnectivity() const {
    if (components.empty()) {
        return; // An empty circuit is considered valid, nothing to analyze.
    }

    set<int> allNodes = getNodes();

    // 1. Check for the existence of a ground node (node 0)
    if (allNodes.find(0) == allNodes.end()) {
        throw runtime_error("Error: No ground node (0) detected in the circuit. Analysis requires a ground reference.");
    }

    // 2. Check for disconnected parts (islands) using Breadth-First Search (BFS)
    map<int, vector<int>> adjList;
    for (const auto& comp : components) {
        int n1 = comp->getNode1();
        int n2 = comp->getNode2();
        adjList[n1].push_back(n2);
        adjList[n2].push_back(n1);

        // Also consider control nodes for connectivity, as they form part of the graph
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

    q.push(0); // Start BFS from the ground node
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

    // --- NEWLY ADDED ---
    // Perform validity checks right before analysis
    checkConnectivity();
}

void Circuit::runTransientAnalysis(double Tstop, double Tstep, const vector<PrintVariable>& printVars, double Tstart, double Tmaxstep) {
    analyzeCircuit(); // This now includes connectivity checks
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    // تعیین گام زمانی واقعی بر اساس Tmaxstep
    double actual_tstep = Tstep;
    if (Tmaxstep > 0 && Tmaxstep < Tstep) {
        actual_tstep = Tmaxstep;
        cout << "Info: Time step limited by Tmaxstep to " << actual_tstep << "s." << endl;
    }

    bool hasNonLinear = false;
    for (const auto& comp : components) {
        if (comp->isNonLinear()) {
            hasNonLinear = true;
            break;
        }
    }
    cout << "--- Starting Transient Analysis ---" << endl;
    if (hasNonLinear) cout << "Non-linear elements detected. Newton-Raphson solver activated." << endl;

    vector<int> printIndices;
    vector<string> printHeaders;
    bool printAll = printVars.empty();
    printHeaders.push_back("Time(s)");

    if (printAll) {
        for (int i = 1; i <= nodeCount; ++i) printHeaders.push_back("V(" + to_string(i) + ")");
        map<int, string> reverseCurrentMap;
        for(const auto& pair : currentComponentMap) reverseCurrentMap[pair.second] = pair.first;
        for(const auto& pair : reverseCurrentMap) printHeaders.push_back("I(" + pair.second + ")");
    } else {
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
    }

    for(const auto& header : printHeaders) cout << left << setw(15) << header;
    cout << endl;

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

        // فقط در صورتی چاپ کن که زمان فعلی بزرگتر یا مساوی Tstart باشد
        if (t >= Tstart) {
            cout << left << setw(15) << fixed << setprecision(6) << t;
            if (printAll) {
                for (int i = 0; i < matrix_size; ++i) cout << setw(15) << fixed << setprecision(6) << x(i);
            } else {
                for (int idx : printIndices) cout << setw(15) << fixed << setprecision(6) << x(idx);
            }
            cout << endl;
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

Component* Circuit::findComponent(const string& name) {
    for (auto& comp : components) {
        if (comp->getName() == name) {
            return comp.get();
        }
    }
    return nullptr;
}

void Circuit::runDCSweep(const string& sweepSourceName, double startVal, double endVal, double increment, const vector<PrintVariable>& printVars) {
    analyzeCircuit(); // This
}