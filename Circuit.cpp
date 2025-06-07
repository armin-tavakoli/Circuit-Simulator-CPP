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
        if (comp->addsCurrentVariable()) {
            currentVarCount++;
            currentComponentMap[comp->getName()] = currentVarCount;
        }
    }
    nodeCount = nodes.empty() ? 0 : *nodes.rbegin();
}

void Circuit::runTransientAnalysis(double endTime, double timeStep, const vector<PrintVariable>& printVars) {
    analyzeCircuit();
    int matrix_size = nodeCount + currentVarCount;
    if (matrix_size == 0) {
        cout << "Circuit is empty. Cannot run analysis." << endl;
        return;
    }

    cout << "--- Starting Transient Analysis ---" << endl;
    cout << "Nodes: " << nodeCount << ", Current Vars: " << currentVarCount << ", Matrix: " << matrix_size << "x" << matrix_size << endl;

    // --- ساخت هدر و لیست اندیس‌ها برای چاپ پویا ---
    vector<int> printIndices;
    vector<string> printHeaders;

    bool printAll = printVars.empty();

    // هدر زمان همیشه چاپ می‌شود
    printHeaders.push_back("Time(s)");

    if (printAll) {
        // رفتار پیش‌فرض: چاپ تمام ولتاژها و جریان‌ها
        for (int i = 1; i <= nodeCount; ++i) printHeaders.push_back("V(" + to_string(i) + ")");
        map<int, string> reverseCurrentMap;
        for(const auto& pair : currentComponentMap) reverseCurrentMap[pair.second] = pair.first;
        for(const auto& pair : reverseCurrentMap) printHeaders.push_back("I(" + pair.second + ")");
    } else {
        // چاپ فقط متغیرهای درخواستی
        for (const auto& var : printVars) {
            if (toupper(var.type) == 'V') {
                int nodeNum = stoi(var.id);
                if (nodeNum > 0 && nodeNum <= nodeCount) {
                    printIndices.push_back(nodeNum - 1);
                    printHeaders.push_back("V(" + var.id + ")");
                } else {
                    cerr << "Warning: Node " << nodeNum << " not found. Ignoring." << endl;
                }
            } else if (toupper(var.type) == 'I') {
                if (currentComponentMap.count(var.id)) {
                    int m_idx = currentComponentMap.at(var.id);
                    printIndices.push_back(nodeCount + m_idx - 1);
                    printHeaders.push_back("I(" + var.id + ")");
                } else {
                    cerr << "Warning: Component '" << var.id << "' for current measurement not found or has no current variable. Ignoring." << endl;
                }
            }
        }
    }

    // چاپ هدر نهایی
    for(const auto& header : printHeaders) {
        cout << left << setw(15) << header;
    }
    cout << endl;

    // --- حلقه اصلی شبیه‌سازی ---
    for (double t = 0; t <= endTime; t += timeStep) {
        MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
        VectorXd b = VectorXd::Zero(matrix_size);

        for (const auto& comp : components) {
            int final_current_idx = -1;
            if (comp->addsCurrentVariable()) {
                final_current_idx = nodeCount + currentComponentMap.at(comp->getName()) - 1;
            }
            comp->stamp(A, b, final_current_idx, timeStep, t);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b);

        // چاپ ردیف نتایج
        cout << left << setw(15) << fixed << setprecision(6) << t;
        if (printAll) {
            for (int i = 0; i < matrix_size; ++i) {
                cout << setw(15) << fixed << setprecision(6) << x(i);
            }
        } else {
            for (int idx : printIndices) {
                cout << setw(15) << fixed << setprecision(6) << x(idx);
            }
        }
        cout << endl;

        // به‌روزرسانی حالت المان‌ها برای گام بعدی
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
