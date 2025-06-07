#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <iostream>
#include <Eigen/Dense>
#include <map>
#include "DiodeModel.h"

using namespace std;
using namespace Eigen;

class Component {
public:
    Component(const string& name, int n1, int n2) : name(name), node1(n1), node2(n2) {}
    virtual ~Component() = default;
    virtual void print() const = 0;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) = 0;
    virtual bool addsCurrentVariable() const { return false; }
    virtual bool isNonLinear() const { return false; }
    string getName() const { return name; }
    int getNode1() const { return node1; }
    int getNode2() const { return node2; }
    void updateNode(int oldNode, int newNode);

protected:
    string name;
    int node1;
    int node2;
};

class Resistor : public Component {
public:
    Resistor(const string& name, int n1, int n2, double res) : Component(name, n1, n2), resistance(res) {}
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
private:
    double resistance;
};

class Capacitor : public Component {
public:
    Capacitor(const string& name, int n1, int n2, double cap)
            : Component(name, n1, n2), capacitance(cap), prev_voltage(0.0) {}
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void updateVoltage(double new_voltage) { prev_voltage = new_voltage; }
private:
    double capacitance;
    double prev_voltage;
};

class VoltageSource : public Component {
public:
    VoltageSource(const string& name, int n1, int n2, double vol) : Component(name, n1, n2), voltage(vol) {}
    void print() const override;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
protected:
    double voltage;
};

class Inductor : public Component {
public:
    Inductor(const string& name, int n1, int n2, double ind)
            : Component(name, n1, n2), inductance(ind), prev_current(0.0) {}
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    void updateCurrent(double new_current) { prev_current = new_current; }
private:
    double inductance;
    double prev_current;
};

class SinusoidalVoltageSource : public VoltageSource {
public:
    SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency)
            : VoltageSource(name, n1, n2, offset), v_offset(offset), v_amplitude(amplitude), freq(frequency) {}
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
private:
    double v_offset;
    double v_amplitude;
    double freq;
};

class Diode : public Component {
public:
    // سازنده حالا پارامترهای مدل را مستقیماً دریافت می‌کند
    Diode(const string& name, int n1, int n2, const DiodeModel& modelParams);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool isNonLinear() const override { return true; }

private:
    // دیود حالا پارامترهای مدل خود را ذخیره می‌کند
    string modelName;
    double Is;
    double Vt;
    double n;
    double Vz; // ولتاژ شکست زنر
};

#endif // COMPONENT_H
