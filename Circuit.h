#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <Eigen/Dense>
#include "Component.h"
#include "PrintRequest.h" // فایل هدر جدید را اضافه می‌کنیم

using namespace std;
using namespace Eigen;

class Circuit {
public:
    // ... متدهای قبلی بدون تغییر
    void addComponent(unique_ptr<Component> component);
    bool removeComponent(const string& name);
    bool hasComponent(const string& name) const;
    void printCircuit(char type = 'A') const;
    void clear();
    set<int> getNodes() const;
    void renameNode(int oldNode, int newNode);

    // --- امضای متد به‌روز شده ---
    // این متد حالا لیستی از متغیرها برای چاپ را می‌پذیرد.
    // اگر لیست خالی باشد، همه چیز را چاپ می‌کند (رفتار پیش‌فرض).
    void runTransientAnalysis(double endTime, double timeStep, const vector<PrintVariable>& printVars = {});

private:
    vector<unique_ptr<Component>> components;
    map<string, int> currentComponentMap;
    int nodeCount = 0;
    int currentVarCount = 0;

    void analyzeCircuit();
};

#endif // CIRCUIT_H
