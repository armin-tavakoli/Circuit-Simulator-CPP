#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <complex> // اضافه شده برای اعداد مختلط
#include <Eigen/Dense>
#include "DiodeModel.h"
#include <QPointF>
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>

using namespace std;
using namespace Eigen;

class Component {
public:
    Component() : node1(-1), node2(-1), ctrlCurrentIdx(-1), posX(0), posY(0) {}
    Component(const string& name, int n1, int n2) : name(name), node1(n1), node2(n2), ctrlCurrentIdx(-1), posX(0), posY(0) {}
    virtual ~Component() = default;

    virtual string toNetlistString() const = 0;
    virtual void print() const = 0;
    // متد stamp برای تحلیل DC و Transient
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) = 0;
    // متد stampAC جدید برای تحلیل AC
    virtual void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const = 0;

    virtual bool addsCurrentVariable() const { return false; }
    virtual bool isNonLinear() const { return false; }
    virtual void resetState() {}
    string getName() const { return name; }
    int getNode1() const { return node1; }
    int getNode2() const { return node2; }
    void updateNode(int oldNode, int newNode);
    void setNodes(int n1, int n2);
    void setCtrlCurrentIdx(int idx) { ctrlCurrentIdx = idx; }
    virtual string getCtrlVName() const { return ""; }
    virtual void setProperties(const map<string, double>& properties);
    virtual map<string, double> getProperties() const;
    virtual string getDisplayValue() const;

    void setPosition(double x, double y) { posX = x; posY = y; }
    QPointF getPosition() const { return QPointF(posX, posY); }

    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(name), CEREAL_NVP(node1), CEREAL_NVP(node2), CEREAL_NVP(ctrlCurrentIdx), CEREAL_NVP(posX), CEREAL_NVP(posY));
    }

protected:
    string name;
    int node1;
    int node2;
    int ctrlCurrentIdx;
    double posX;
    double posY;
};

class Resistor : public Component {
public:
    Resistor() : resistance(0.0) {}
    Resistor(const string& name, int n1, int n2, double res);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(resistance));
    }
private:
    double resistance;
};

class Capacitor : public Component {
public:
    Capacitor() : capacitance(0.0), prev_voltage(0.0) {}
    Capacitor(const string& name, int n1, int n2, double cap);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    void updateVoltage(double new_voltage) { prev_voltage = new_voltage; }
    void resetState() override { prev_voltage = 0.0; }
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(capacitance), CEREAL_NVP(prev_voltage));
    }
private:
    double capacitance;
    double prev_voltage;
};


class Inductor : public Component {
public:
    Inductor() : inductance(0.0), prev_current(0.0) {}
    Inductor(const string& name, int n1, int n2, double ind);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    bool addsCurrentVariable() const override { return true; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    void updateCurrent(double new_current) { prev_current = new_current; }
    void resetState() override { prev_current = 0.0; }
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(inductance), CEREAL_NVP(prev_current));
    }
private:
    double inductance;
    double prev_current;
};

class CurrentSource : public Component {
public:
    CurrentSource() : current(0.0) {}
    CurrentSource(const string& name, int n1, int n2, double current);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(current));
    }
private:
    double current;
};

class VoltageSource : public Component {
public:
    VoltageSource() : voltage(0.0) {}
    VoltageSource(const string& name, int n1, int n2, double vol);
    void print() const override;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    bool addsCurrentVariable() const override { return true; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(voltage));
    }
protected:
    double voltage;
};

class ACVoltageSource : public VoltageSource {
public:
    ACVoltageSource() : ac_magnitude(1.0), ac_phase(0.0) {}
    ACVoltageSource(const string& name, int n1, int n2, double magnitude, double phase = 0.0)
            : VoltageSource(name, n1, n2, 0.0), ac_magnitude(magnitude), ac_phase(phase) {}

    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<VoltageSource>(this), CEREAL_NVP(ac_magnitude), CEREAL_NVP(ac_phase));
    }
private:
    double ac_magnitude;
    double ac_phase;
};

class SinusoidalVoltageSource : public VoltageSource {
public:
    SinusoidalVoltageSource() : v_offset(0.0), v_amplitude(0.0), freq(0.0) {}
    SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<VoltageSource>(this), CEREAL_NVP(v_offset), CEREAL_NVP(v_amplitude), CEREAL_NVP(freq));
    }
private:
    double v_offset;
    double v_amplitude;
    double freq;
};

class PulseVoltageSource : public VoltageSource {
public:
    PulseVoltageSource() : v_initial(0.0), v_pulsed(0.0), t_delay(0.0), t_rise(0.0), t_fall(0.0), t_pulse_width(0.0), t_period(0.0) {}
    PulseVoltageSource(const string& name, int n1, int n2, double v1, double v2, double td, double tr, double tf, double pw, double per);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<VoltageSource>(this), CEREAL_NVP(v_initial), CEREAL_NVP(v_pulsed), CEREAL_NVP(t_delay), CEREAL_NVP(t_rise), CEREAL_NVP(t_fall), CEREAL_NVP(t_pulse_width), CEREAL_NVP(t_period));
    }
private:
    double v_initial, v_pulsed, t_delay, t_rise, t_fall, t_pulse_width, t_period;
    double calculate_voltage_at(double t) const;
};

class Diode : public Component {
public:
    Diode() : Is(0.0), Vt(0.0), n(0.0), Vz(0.0) {}
    Diode(const string& name, int n1, int n2, const DiodeModel& modelParams);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    bool isNonLinear() const override { return true; }
    string toNetlistString() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(modelName), CEREAL_NVP(Is), CEREAL_NVP(Vt), CEREAL_NVP(n), CEREAL_NVP(Vz));
    }
private:
    string modelName;
    double Is, Vt, n, Vz;
};

class VCVS : public Component {
public:
    VCVS() : ctrlNode1(-1), ctrlNode2(-1), gain(0.0) {}
    VCVS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    bool addsCurrentVariable() const override { return true; }
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
private:
    int ctrlNode1, ctrlNode2;
    double gain;
};

class VCCS : public Component {
public:
    VCCS() : ctrlNode1(-1), ctrlNode2(-1), gain(0.0) {}
    VCCS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    int getCtrlNode1() const { return ctrlNode1; }
    int getCtrlNode2() const { return ctrlNode2; }
    void updateCtrlNodes(int oldNode, int newNode);
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
private:
    int ctrlNode1, ctrlNode2;
    double gain;
};

class CCVS : public Component {
public:
    CCVS() : gain(0.0) {}
    CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    bool addsCurrentVariable() const override { return true; }
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlVName), CEREAL_NVP(gain));
    }
private:
    string ctrlVName;
    double gain;
};

class CCCS : public Component {
public:
    CCCS() : gain(0.0) {}
    CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlVName), CEREAL_NVP(gain));
    }
private:
    string ctrlVName;
    double gain;
};

class Ground : public Component {
public:
    Ground() { name = "GND"; }
    Ground(const string& name, int n1, int n2) : Component(name, n1, n2) {}

    void print() const override {}
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override { /* do nothing */ }
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override { /* do nothing */ }
    string toNetlistString() const override { return ""; }

    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this));
    }
};

#endif
