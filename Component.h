#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <Eigen/Dense>
#include "DiodeModel.h"

// هدرهای مورد نیاز برای کتابخانه Cereal
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>


using namespace std;
using namespace Eigen;

class Component {
public:
    // سازنده پیش‌فرض برای سریال‌سازی مورد نیاز است
    Component() : node1(-1), node2(-1), ctrlCurrentIdx(-1) {}
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

    // --- Generic Property Management ---
    virtual void setProperties(const map<string, double>& properties);
    virtual map<string, double> getProperties() const;
    virtual string getDisplayValue() const;

    // --- تابع سریال‌سازی برای Cereal ---
    // این تابع فیلدهای مشترک بین تمام قطعات را ذخیره می‌کند
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(CEREAL_NVP(name), CEREAL_NVP(node1), CEREAL_NVP(node2), CEREAL_NVP(ctrlCurrentIdx));
    }


protected:
    string name;
    int node1;
    int node2;
    int ctrlCurrentIdx;
};

// ================== Resistor ==================
class Resistor : public Component {
public:
    Resistor() : resistance(0.0) {} // سازنده پیش‌فرض
    Resistor(const string& name, int n1, int n2, double res);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی برای مقاومت
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(resistance));
    }
private:
    double resistance;
};

// ================== Capacitor ==================
class Capacitor : public Component {
public:
    Capacitor() : capacitance(0.0), prev_voltage(0.0) {} // سازنده پیش‌فرض
    Capacitor(const string& name, int n1, int n2, double cap);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;
    void updateVoltage(double new_voltage) { prev_voltage = new_voltage; }
    void resetState() override { prev_voltage = 0.0; }

    // تابع سریال‌سازی برای خازن
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(capacitance), CEREAL_NVP(prev_voltage));
    }
private:
    double capacitance;
    double prev_voltage; // وضعیت داخلی خازن نیز باید ذخیره شود
};

// ================== Inductor ==================
class Inductor : public Component {
public:
    Inductor() : inductance(0.0), prev_current(0.0) {} // سازنده پیش‌فرض
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

    // تابع سریال‌سازی برای سلف
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(inductance), CEREAL_NVP(prev_current));
    }
private:
    double inductance;
    double prev_current; // وضعیت داخلی سلف نیز باید ذخیره شود
};

// ================== CurrentSource ==================
class CurrentSource : public Component {
public:
    CurrentSource() : current(0.0) {} // سازنده پیش‌فرض
    CurrentSource(const string& name, int n1, int n2, double current);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی برای منبع جریان
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(current));
    }
private:
    double current;
};


// ================== VoltageSource (Base) ==================
class VoltageSource : public Component {
public:
    VoltageSource() : voltage(0.0) {} // سازنده پیش‌فرض
    VoltageSource(const string& name, int n1, int n2, double vol);
    void print() const override;
    virtual void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی برای کلاس پایه منابع ولتاژ
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(voltage));
    }
protected:
    double voltage;
};

// ================== SinusoidalVoltageSource ==================
class SinusoidalVoltageSource : public VoltageSource {
public:
    SinusoidalVoltageSource() : v_offset(0.0), v_amplitude(0.0), freq(0.0) {} // سازنده پیش‌فرض
    SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<VoltageSource>(this), CEREAL_NVP(v_offset), CEREAL_NVP(v_amplitude), CEREAL_NVP(freq));
    }
private:
    double v_offset;
    double v_amplitude;
    double freq;
};

// ================== PulseVoltageSource ==================
class PulseVoltageSource : public VoltageSource {
public:
    PulseVoltageSource() : v_initial(0.0), v_pulsed(0.0), t_delay(0.0), t_rise(0.0), t_fall(0.0), t_pulse_width(0.0), t_period(0.0) {} // سازنده پیش‌فرض
    PulseVoltageSource(const string& name, int n1, int n2, double v1, double v2, double td, double tr, double tf, double pw, double per);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<VoltageSource>(this), CEREAL_NVP(v_initial), CEREAL_NVP(v_pulsed), CEREAL_NVP(t_delay), CEREAL_NVP(t_rise), CEREAL_NVP(t_fall), CEREAL_NVP(t_pulse_width), CEREAL_NVP(t_period));
    }
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
    Diode() : Is(0.0), Vt(0.0), n(0.0), Vz(0.0) {} // سازنده پیش‌فرض
    Diode(const string& name, int n1, int n2, const DiodeModel& modelParams);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool isNonLinear() const override { return true; }
    string toNetlistString() const override;

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(modelName), CEREAL_NVP(Is), CEREAL_NVP(Vt), CEREAL_NVP(n), CEREAL_NVP(Vz));
    }
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
    VCVS() : ctrlNode1(-1), ctrlNode2(-1), gain(0.0) {} // سازنده پیش‌فرض
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

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
private:
    int ctrlNode1;
    int ctrlNode2;
    double gain;
};

// ================== VCCS ==================
class VCCS : public Component {
public:
    VCCS() : ctrlNode1(-1), ctrlNode2(-1), gain(0.0) {} // سازنده پیش‌فرض
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

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlNode1), CEREAL_NVP(ctrlNode2), CEREAL_NVP(gain));
    }
private:
    int ctrlNode1, ctrlNode2;
    double gain;
};

// ================== CCVS ==================
class CCVS : public Component {
public:
    CCVS() : gain(0.0) {} // سازنده پیش‌فرض
    CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    bool addsCurrentVariable() const override { return true; }
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlVName), CEREAL_NVP(gain));
    }
private:
    string ctrlVName;
    double gain;
};

// ================== CCCS ==================
class CCCS : public Component {
public:
    CCCS() : gain(0.0) {} // سازنده پیش‌فرض
    CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain);
    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    string getCtrlVName() const override { return ctrlVName; }
    string toNetlistString() const override;
    void setProperties(const map<string, double>& properties) override;
    map<string, double> getProperties() const override;
    string getDisplayValue() const override;

    // تابع سریال‌سازی
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(ctrlVName), CEREAL_NVP(gain));
    }
private:
    string ctrlVName;
    double gain;
};

#endif
