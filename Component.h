#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <map>
#include "DiodeModel.h"

using namespace std;
using namespace Eigen;

class Component {
public:
    Component(const string& name, int n1, int n2) : name(name), node1(n1), node2(n2), ctrlCurrentIdx(-1) {}
    virtual ~Component() = default;

    // --- Core Simulation Methods ---
    virtual string toNetlistString() const = 0;
    virtual void print() const = 0;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) = 0;
    virtual bool addsCurrentVariable() const { return false; }
    virtual bool isNonLinear() const { return false; }
    virtual void resetState() {}

    // --- Getters and Setters for Connectivity ---
    string getName() const { return name; }
    int getNode1() const { return node1; }
    int getNode2() const { return node2; }
    void updateNode(int oldNode, int newNode);
    void setNodes(int n1, int n2);
    void setCtrlCurrentIdx(int idx) { ctrlCurrentIdx = idx; }
    virtual string getCtrlVName() const { return ""; }

    // --- New Generic Property Management ---
    virtual void setProperties(const map<string, double>& properties);
    virtual map<string, double> getProperties() const;
    virtual string getDisplayValue() const;


protected:
    string name;
    int node1;
    int node2;
    int ctrlCurrentIdx;
};

// ================== Resistor ==================
class Resistor : public Component {
public:
    Resistor(const string& name, int n1, int n2, double res);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    double resistance;
};

// ================== Capacitor ==================
class Capacitor : public Component {
public:
    Capacitor(const string& name, int n1, int n2, double cap);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    void updateVoltage(double new_voltage) { prev_voltage = new_voltage; }
    void resetState() override { prev_voltage = 0.0; }
private:
    double capacitance;
    double prev_voltage;
};

// ================== Inductor ==================
class Inductor : public Component {
public:
    Inductor(const string& name, int n1, int n2, double ind);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    void updateCurrent(double new_current) { prev_current = new_current; }
    void resetState() override { prev_current = 0.0; }
private:
    double inductance;
    double prev_current;
};

// ================== CurrentSource ==================
class CurrentSource : public Component {
public:
    CurrentSource(const string& name, int n1, int n2, double current);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    double current;
};


// ================== VoltageSource (Base) ==================
class VoltageSource : public Component {
public:
    VoltageSource(const string& name, int n1, int n2, double vol);
    void print() const override;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
protected:
    double voltage;
};

// ================== SinusoidalVoltageSource ==================
class SinusoidalVoltageSource : public VoltageSource {
public:
    SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    double v_offset;
    double v_amplitude;
    double freq;
};

// ================== PulseVoltageSource ==================
class PulseVoltageSource : public VoltageSource {
public:
    PulseVoltageSource(const string& name, int n1, int n2, double v1, double v2, double td, double tr, double tf, double pw, double per);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    double v_initial;
    double v_pulsed;
    double t_delay;
    double t_rise;
    double t_fall;
    double t_pulse_width;
    double t_period;
    double calculate_voltage_at(double t) const;
};

// ================== Diode ==================
class Diode : public Component {
public:
    Diode(const string& name, int n1, int n2, const DiodeModel& modelParams);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool isNonLinear() const override { return true; }
    string toNetlistString() const override;
private:
    string modelName;
    double Is;
    double Vt;
    double n;
    double Vz;
};

// ================== VCVS ==================
class VCVS : public Component {
public:
    VCVS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    int ctrlNode1;
    int ctrlNode2;
    double gain;
};

// ================== VCCS ==================
class VCCS : public Component {
public:
    VCCS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    int ctrlNode1, ctrlNode2;
    double gain;
};

// ================== CCVS ==================
class CCVS : public Component {
public:
    CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    string ctrlVName;
    double gain;
};

// ================== CCCS ==================
class CCCS : public Component {
public:
    CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
private:
    string ctrlVName;
    double gain;
};

#endif
