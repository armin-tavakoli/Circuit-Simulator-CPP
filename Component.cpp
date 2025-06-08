#include "Component.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- متدهای چاپ ---
void Resistor::print() const { cout << "Type: Resistor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), R=" << resistance << " Ohms" << endl; }
void Capacitor::print() const { cout << "Type: Capacitor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), C=" << capacitance << " F" << endl; }
void VoltageSource::print() const { cout << "Type: DC Source, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), V=" << voltage << " V" << endl; }
void CurrentSource::print() const { cout << "Type: Current Source, Name: " << name << ", Nodes: (" << node1 << " -> " << node2 << "), I=" << current << " A" << endl; }
void Inductor::print() const { cout << "Type: Inductor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), L=" << inductance << " H" << endl; }
void SinusoidalVoltageSource::print() const {
    cout << "Type: SIN Source, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), SIN(" << v_offset << " " << v_amplitude << " " << freq << "Hz)" << endl;
}
void PulseVoltageSource::print() const {
    cout << "Type: PULSE Source, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), PULSE(" << v_initial << " " << v_pulsed << " " << t_delay << " "
         << t_rise << " " << t_fall << " " << t_pulse_width << " " << t_period << ")" << endl;
}
void Diode::print() const {
    cout << "Type: Diode, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), Model: " << modelName << endl;
}
void VCVS::print() const {
    cout << "Type: VCVS, Name: " << name << ", Out: (" << node1 << "," << node2
         << "), Control: (" << ctrlNode1 << "," << ctrlNode2 << "), Gain=" << gain << endl;
}
void VCCS::print() const {
    cout << "Type: VCCS, Name: " << name << ", Out: (" << node1 << "->" << node2
         << "), Control: (" << ctrlNode1 << "," << ctrlNode2 << "), Gain=" << gain << endl;
}
void CCVS::print() const {
    cout << "Type: CCVS, Name: " << name << ", Out: (" << node1 << "," << node2
         << "), Control Current: I(" << ctrlVName << "), Gain=" << gain << endl;
}
void CCCS::print() const {
    cout << "Type: CCCS, Name: " << name << ", Out: (" << node1 << "->" << node2
         << "), Control Current: I(" << ctrlVName << "), Gain=" << gain << endl;
}


// --- متدهای سازنده ---
Resistor::Resistor(const string& name, int n1, int n2, double res) : Component(name, n1, n2), resistance(res) {}
Capacitor::Capacitor(const string& name, int n1, int n2, double cap) : Component(name, n1, n2), capacitance(cap), prev_voltage(0.0) {}
VoltageSource::VoltageSource(const string& name, int n1, int n2, double vol) : Component(name, n1, n2), voltage(vol) {}
CurrentSource::CurrentSource(const string& name, int n1, int n2, double current) : Component(name, n1, n2), current(current) {}
Inductor::Inductor(const string& name, int n1, int n2, double ind) : Component(name, n1, n2), inductance(ind), prev_current(0.0) {}
SinusoidalVoltageSource::SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency) : VoltageSource(name, n1, n2, offset), v_offset(offset), v_amplitude(amplitude), freq(frequency) {}
Diode::Diode(const string& name, int n1, int n2, const DiodeModel& modelParams) : Component(name, n1, n2) {
    this->modelName = modelParams.name;
    this->Is = modelParams.Is;
    this->Vt = modelParams.Vt;
    this->n = modelParams.n;
    this->Vz = modelParams.Vz;
}
VCVS::VCVS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain) : Component(name, n1, n2), ctrlNode1(ctrl_n1), ctrlNode2(ctrl_n2), gain(gain) {}
VCCS::VCCS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain) : Component(name, n1, n2), ctrlNode1(ctrl_n1), ctrlNode2(ctrl_n2), gain(gain) {}
CCVS::CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain) : Component(name, n1, n2), ctrlVName(vctrl_name), gain(gain) {}
CCCS::CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain) : Component(name, n1, n2), ctrlVName(vctrl_name), gain(gain) {}

PulseVoltageSource::PulseVoltageSource(const string& name, int n1, int n2, double v1, double v2, double td, double tr, double tf, double pw, double per)
        : VoltageSource(name, n1, n2, v1), v_initial(v1), v_pulsed(v2), t_delay(td), t_rise(tr), t_fall(tf), t_pulse_width(pw), t_period(per) {}

// --- متدهای Stamp ---

void Resistor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g = 1.0 / resistance;
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(n1, n1) += g;
    if (n2 >= 0) A(n2, n2) += g;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= g; A(n2, n1) -= g; }
}

void Capacitor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g_eq = capacitance / h;
    double I_eq = g_eq * prev_voltage;
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(n1, n1) += g_eq;
    if (n2 >= 0) A(n2, n2) += g_eq;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= g_eq; A(n2, n1) -= g_eq; }
    if (n1 >= 0) b(n1) += I_eq;
    if (n2 >= 0) b(n2) -= I_eq;
}

// **نسخه اصلاح شده**
void VoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    // KVL: V(n1) - V(n2) = V
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = this->voltage;

    // KCL: جریان از گره مثبت خارج و به گره منفی وارد می‌شود
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

void CurrentSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) b(n1) += current;
    if (n2 >= 0) b(n2) -= current;
}

void Inductor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    double R_eq = inductance / h;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    A(current_idx, current_idx) = -R_eq;
    b(current_idx) = -R_eq * prev_current;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// **نسخه اصلاح شده**
void SinusoidalVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    double instantaneous_voltage = v_offset + v_amplitude * sin(2 * M_PI * freq * t);
    // KVL
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;
    // KCL
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void PulseVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;

    double instantaneous_voltage = calculate_voltage_at(t);

    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;

    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

void Diode::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    double v1 = (n1 >= 0) ? x_prev_nr(n1) : 0.0;
    double v2 = (n2 >= 0) ? x_prev_nr(n2) : 0.0;
    double Vd = v1 - v2;
    if (Vz > 0 && Vd < -Vz) {
        const double Gz = 100.0;
        double Ieq_zener = Gz * Vz;
        if (n1 >= 0) { A(n1, n1) += Gz; b(n1) -= Ieq_zener; }
        if (n2 >= 0) { A(n2, n2) += Gz; b(n2) += Ieq_zener; }
        if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= Gz; A(n2, n1) -= Gz; }
        return;
    }
    double Vd_limited = (Vd > 0.7) ? 0.7 : Vd;
    double exp_val = exp(Vd_limited / (n * Vt));
    double Id = Is * (exp_val - 1.0);
    double Geq = (Is / (n * Vt)) * exp_val;
    double Ieq_comp = Id - Geq * Vd_limited;
    if (n1 >= 0) { A(n1, n1) += Geq; b(n1) -= Ieq_comp; }
    if (n2 >= 0) { A(n2, n2) += Geq; b(n2) += Ieq_comp; }
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= Geq; A(n2, n1) -= Geq; }
}

void VCVS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    int cn1 = ctrlNode1 - 1;
    int cn2 = ctrlNode2 - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (cn1 >= 0) A(current_idx, cn1) = -gain;
    if (cn2 >= 0) A(current_idx, cn2) = gain;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// **نسخه اصلاح شده**
void VCCS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    int cn1 = ctrlNode1 - 1;
    int cn2 = ctrlNode2 - 1;
    // I_out = gain * (V(cn1) - V(cn2))
    // جریان از n1 خارج و به n2 وارد می‌شود
    // KCL at n1: ... + I_out = 0 => ... + gain*(V(cn1) - V(cn2)) = 0
    // KCL at n2: ... - I_out = 0 => ... - gain*(V(cn1) - V(cn2)) = 0
    if (n1 >= 0) {
        if (cn1 >= 0) A(n1, cn1) += gain;
        if (cn2 >= 0) A(n1, cn2) -= gain;
    }
    if (n2 >= 0) {
        if (cn1 >= 0) A(n2, cn1) -= gain;
        if (cn2 >= 0) A(n2, cn2) += gain;
    }
}

// **نسخه اصلاح شده**
void CCVS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    // V_out = gain * I_ctrl
    // The MNA current variable I_vsource flows OUT of the positive terminal.
    // The SPICE control current flows INTO the positive terminal.
    // Therefore, I_ctrl = -I_vsource.
    // KVL: V(n1) - V(n2) - gain * (-I_vsource) = 0
    // => V(n1) - V(n2) + gain * I_vsource = 0
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (ctrlCurrentIdx != -1) A(current_idx, ctrlCurrentIdx) = gain; // Sign corrected to +

    // KCL
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

void CCCS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    // I_out = gain * I_ctrl
    // I_ctrl = -I_vsource
    // I_out = -gain * I_vsource
    // KCL at n1: ... - I_out = 0 => ... - (-gain * I_vsource) = 0 => ... + gain * I_vsource = 0
    // KCL at n2: ... + I_out = 0 => ... + (-gain * I_vsource) = 0 => ... - gain * I_vsource = 0
    if (ctrlCurrentIdx != -1) {
        if (n1 >= 0) A(n1, ctrlCurrentIdx) += gain; // Correct
        if (n2 >= 0) A(n2, ctrlCurrentIdx) -= gain; // Correct
    }
}

void Component::updateNode(int oldNode, int newNode) {
    if (node1 == oldNode) node1 = newNode;
    if (node2 == oldNode) node2 = newNode;
    if (auto vcvs = dynamic_cast<VCVS*>(this)) vcvs->updateCtrlNodes(oldNode, newNode);
    if (auto vccs = dynamic_cast<VCCS*>(this)) vccs->updateCtrlNodes(oldNode, newNode);
}

void VCVS::updateCtrlNodes(int oldNode, int newNode) {
    if (ctrlNode1 == oldNode) ctrlNode1 = newNode;
    if (ctrlNode2 == oldNode) ctrlNode2 = newNode;
}

void VCCS::updateCtrlNodes(int oldNode, int newNode) {
    if (ctrlNode1 == oldNode) ctrlNode1 = newNode;
    if (ctrlNode2 == oldNode) ctrlNode2 = newNode;
}

double PulseVoltageSource::calculate_voltage_at(double t) const {
    // اگر دوره تناوب صفر یا منفی باشد، پالس تکرار نمی‌شود
    if (t_period <= 0) {
        if (t <= t_delay) return v_initial;
        t -= t_delay;
        if (t <= t_rise) return v_initial + (v_pulsed - v_initial) * t / t_rise;
        t -= t_rise;
        if (t <= t_pulse_width) return v_pulsed;
        t -= t_pulse_width;
        if (t <= t_fall) return v_pulsed + (v_initial - v_pulsed) * t / t_fall;
        return v_initial;
    }

    // مدیریت پالس‌های متناوب
    t = fmod(t, t_period);

    if (t <= t_delay) return v_initial;
    t -= t_delay;
    if (t_rise > 0 && t <= t_rise) return v_initial + (v_pulsed - v_initial) * t / t_rise;
    t -= t_rise;
    if (t <= t_pulse_width) return v_pulsed;
    t -= t_pulse_width;
    if (t_fall > 0 && t <= t_fall) return v_pulsed + (v_initial - v_pulsed) * t / t_fall;

    return v_initial;
}