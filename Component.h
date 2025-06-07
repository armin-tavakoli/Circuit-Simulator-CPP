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
    Component(const string& name, int n1, int n2) : name(name), node1(n1), node2(n2), ctrlCurrentIdx(-1) {}
    virtual ~Component() = default;
    virtual void print() const = 0;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) = 0;
    virtual bool addsCurrentVariable() const { return false; }
    virtual bool isNonLinear() const { return false; }
    string getName() const { return name; }
    int getNode1() const { return node1; }
    int getNode2() const { return node2; }
    void updateNode(int oldNode, int newNode);

    void setCtrlCurrentIdx(int idx) { ctrlCurrentIdx = idx; }
    virtual string getCtrlVName() const { return ""; }

protected:
    string name;
    int node1;
    int node2;
    int ctrlCurrentIdx;
};

class Resistor : public Component {
public:
    Resistor(const string& name, int n1, int n2, double res);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
private:
    double resistance;
};

class Capacitor : public Component {
public:
    Capacitor(const string& name, int n1, int n2, double cap);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void updateVoltage(double new_voltage) { prev_voltage = new_voltage; }
private:
    double capacitance;
    double prev_voltage;
};

class VoltageSource : public Component {
public:
    VoltageSource(const string& name, int n1, int n2, double vol);
    void print() const override;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
protected:
    double voltage;
};

class CurrentSource : public Component {
public:
    CurrentSource(const string& name, int n1, int n2, double current);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
private:
    double current;
};

class Inductor : public Component {
public:
    Inductor(const string& name, int n1, int n2, double ind);
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
    SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
private:
    double v_offset;
    double v_amplitude;
    double freq;
};

class Diode : public Component {
public:
    Diode(const string& name, int n1, int n2, const DiodeModel& modelParams);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool isNonLinear() const override { return true; }
private:
    string modelName;
    double Is;
    double Vt;
    double n;
    double Vz;
};

class VCVS : public Component {
public:
    VCVS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
private:
    int ctrlNode1;
    int ctrlNode2;
    double gain;
};

class VCCS : public Component {
public:
    VCCS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
private:
    int ctrlNode1, ctrlNode2;
    double gain;
};

class CCVS : public Component {
public:
    CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string getCtrlVName() const override { return ctrlVName; }
private:
    string ctrlVName;
    double gain;
};

class CCCS : public Component {
public:
    CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string getCtrlVName() const override { return ctrlVName; }
private:
    string ctrlVName;
    double gain;
};


#endif // COMPONENT_H
