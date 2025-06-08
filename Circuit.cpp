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
}

void Circuit::runTransientAnalysis(double endTime, double timeStep, const vector<PrintVariable>& printVars) {
    analyzeCircuit();
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
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
    for (double t = 0; t <= endTime; t += timeStep) {
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
                    comp->stamp(A, b, x_nr_guess, final_current_idx, timeStep, t);
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
                comp->stamp(A, b, x_nr_guess, final_current_idx, timeStep, t);
            }
            x_nr_guess = A.colPivHouseholderQr().solve(b);
        }
        x = x_nr_guess;
        cout << left << setw(15) << fixed << setprecision(6) << t;
        if (printAll) {
            for (int i = 0; i < matrix_size; ++i) cout << setw(15) << fixed << setprecision(6) << x(i);
        } else {
            for (int idx : printIndices) cout << setw(15) << fixed << setprecision(6) << x(idx);
        }
        cout << endl;
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
    Component* sweepSource = findComponent(sweepSourceName);
    if (!sweepSource) {
        throw runtime_error("Sweep source '" + sweepSourceName + "' not found.");
    }

    const double h_dc = 1e12; // h -> infinity for DC analysis

    analyzeCircuit();
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
        sweepSource->set_dc_value(sweepVal);

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
