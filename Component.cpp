#include "Component.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cereal/types/polymorphic.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const complex<double> j(0, 1);

// تابع کمکی برای فرمت کردن مقادیر با پیشوندهای مهندسی
string formatValue(double value) {
    stringstream ss;
    if (abs(value) >= 1e9) ss << fixed << setprecision(2) << value / 1e9 << "G";
    else if (abs(value) >= 1e6) ss << fixed << setprecision(2) << value / 1e6 << "M";
    else if (abs(value) >= 1e3) ss << fixed << setprecision(2) << value / 1e3 << "k";
    else if (abs(value) >= 1) ss << fixed << setprecision(2) << value;
    else if (abs(value) >= 1e-3) ss << fixed << setprecision(2) << value * 1e3 << "m";
    else if (abs(value) >= 1e-6) ss << fixed << setprecision(2) << value * 1e6 << "u";
    else if (abs(value) >= 1e-9) ss << fixed << setprecision(2) << value * 1e9 << "n";
    else ss << scientific << setprecision(2) << value;
    return ss.str();
}

// --- Component Base Class ---
void Component::setProperties(const map<string, double>& properties) { /* Base does nothing */ }
map<string, double> Component::getProperties() const { return {}; }
string Component::getDisplayValue() const { return ""; }

void Component::updateNode(int oldNode, int newNode) {
    for (int& node : nodes) {
        if (node == oldNode) {
            node = newNode;
        }
    }
    // این بخش باید برای به‌روزرسانی گره‌های کنترلی منابع وابسته تکمیل شود
    if (auto vcvs = dynamic_cast<VCVS*>(this)) { vcvs->updateCtrlNodes(oldNode, newNode); }
    if (auto vccs = dynamic_cast<VCCS*>(this)) { vccs->updateCtrlNodes(oldNode, newNode); }
}

// --- Resistor ---
Resistor::Resistor(const string& name, int n1, int n2, double res) : Component(name, {n1, n2}), resistance(res) {}
void Resistor::setProperties(const map<string, double>& properties) { if (properties.count("Resistance")) resistance = properties.at("Resistance"); }
map<string, double> Resistor::getProperties() const { return {{"Resistance", resistance}}; }
string Resistor::getDisplayValue() const { return formatValue(resistance) + "Ohm"; }
void Resistor::print() const { cout << "Type: Resistor, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), R=" << resistance << " Ohms" << endl; }
string Resistor::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(resistance); }
void Resistor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g = 1.0 / resistance;
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(n1, n1) += g;
    if (n2 >= 0) A(n2, n2) += g;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= g; A(n2, n1) -= g; }
}
void Resistor::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    double g = 1.0 / resistance;
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(n1, n1) += g;
    if (n2 >= 0) A(n2, n2) += g;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= g; A(n2, n1) -= g; }
}

// --- Capacitor ---
Capacitor::Capacitor(const string& name, int n1, int n2, double cap) : Component(name, {n1, n2}), capacitance(cap), prev_voltage(0.0) {}
void Capacitor::setProperties(const map<string, double>& properties) { if (properties.count("Capacitance")) capacitance = properties.at("Capacitance"); }
map<string, double> Capacitor::getProperties() const { return {{"Capacitance", capacitance}}; }
string Capacitor::getDisplayValue() const { return formatValue(capacitance) + "F"; }
void Capacitor::print() const { cout << "Type: Capacitor, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), C=" << capacitance << " F" << endl; }
string Capacitor::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(capacitance); }
void Capacitor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g_eq = capacitance / h;
    double I_eq = g_eq * prev_voltage;
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(n1, n1) += g_eq;
    if (n2 >= 0) A(n2, n2) += g_eq;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= g_eq; A(n2, n1) -= g_eq; }
    if (n1 >= 0) b(n1) += I_eq;
    if (n2 >= 0) b(n2) -= I_eq;
}
void Capacitor::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    complex<double> admittance = j * omega * capacitance;
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(n1, n1) += admittance;
    if (n2 >= 0) A(n2, n2) += admittance;
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= admittance; A(n2, n1) -= admittance; }
}

// --- Inductor ---
Inductor::Inductor(const string& name, int n1, int n2, double ind) : Component(name, {n1, n2}), inductance(ind), prev_current(0.0) {}
void Inductor::setProperties(const map<string, double>& properties) { if (properties.count("Inductance")) inductance = properties.at("Inductance"); }
map<string, double> Inductor::getProperties() const { return {{"Inductance", inductance}}; }
string Inductor::getDisplayValue() const { return formatValue(inductance) + "H"; }
void Inductor::print() const { cout << "Type: Inductor, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), L=" << inductance << " H" << endl; }
string Inductor::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(inductance); }
void Inductor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    double R_eq = inductance / h;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    A(current_idx, current_idx) = -R_eq;
    b(current_idx) = -R_eq * prev_current;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void Inductor::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    complex<double> impedance = j * omega * inductance;
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    A(current_idx, current_idx) = -impedance;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- CurrentSource ---
CurrentSource::CurrentSource(const string& name, int n1, int n2, double current) : Component(name, {n1, n2}), current(current) {}
void CurrentSource::setProperties(const map<string, double>& properties) { if (properties.count("Current")) current = properties.at("Current"); }
map<string, double> CurrentSource::getProperties() const { return {{"Current", current}}; }
string CurrentSource::getDisplayValue() const { return formatValue(current) + "A"; }
void CurrentSource::print() const { cout << "Type: Current Source, Name: " << name << ", Nodes: (" << getNode(0) << " -> " << getNode(1) << "), I=" << current << " A" << endl; }
string CurrentSource::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(current); }
void CurrentSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) b(n1) += current;
    if (n2 >= 0) b(n2) -= current;
}
void CurrentSource::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) b(n1) += current;
    if (n2 >= 0) b(n2) -= current;
}

// --- VoltageSource ---
VoltageSource::VoltageSource(const string& name, int n1, int n2, double vol) : Component(name, {n1, n2}), voltage(vol) {}
void VoltageSource::setProperties(const map<string, double>& properties) { if (properties.count("Voltage")) voltage = properties.at("Voltage"); }
map<string, double> VoltageSource::getProperties() const { return {{"Voltage", voltage}}; }
string VoltageSource::getDisplayValue() const { return formatValue(voltage) + "V"; }
void VoltageSource::print() const { cout << "Type: DC Source, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), V=" << voltage << " V" << endl; }
string VoltageSource::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(voltage); }
void VoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = this->voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void VoltageSource::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = this->voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- ACVoltageSource ---
void ACVoltageSource::print() const { cout << "Type: AC Source, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), Mag=" << ac_magnitude << "V, Phase=" << ac_phase << "deg" << endl; }
string ACVoltageSource::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " AC " + to_string(ac_magnitude) + " " + to_string(ac_phase); }
void ACVoltageSource::setProperties(const map<string, double>& properties) {
    if (properties.count("Magnitude")) ac_magnitude = properties.at("Magnitude");
    if (properties.count("Phase")) ac_phase = properties.at("Phase");
}
map<string, double> ACVoltageSource::getProperties() const { return {{"Magnitude", ac_magnitude}, {"Phase", ac_phase}}; }
string ACVoltageSource::getDisplayValue() const { return "AC " + formatValue(ac_magnitude) + "V"; }
void ACVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = 0.0; // DC value is zero for AC analysis
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void ACVoltageSource::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    complex<double> ac_val = polar(ac_magnitude, ac_phase * M_PI / 180.0);
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = ac_val;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- SinusoidalVoltageSource ---
SinusoidalVoltageSource::SinusoidalVoltageSource(const string& name, int n1, int n2, double offset, double amplitude, double frequency) : VoltageSource(name, n1, n2, offset), v_offset(offset), v_amplitude(amplitude), freq(frequency) {}
void SinusoidalVoltageSource::setProperties(const map<string, double>& properties) {
    if (properties.count("Offset")) v_offset = properties.at("Offset");
    if (properties.count("Amplitude")) v_amplitude = properties.at("Amplitude");
    if (properties.count("Frequency")) freq = properties.at("Frequency");
}
map<string, double> SinusoidalVoltageSource::getProperties() const {
    return {{"Offset", v_offset}, {"Amplitude", v_amplitude}, {"Frequency", freq}};
}
string SinusoidalVoltageSource::getDisplayValue() const { return "SIN"; }
void SinusoidalVoltageSource::print() const { cout << "Type: SIN Source, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), SIN(" << v_offset << " " << v_amplitude << " " << freq << "Hz)" << endl; }
string SinusoidalVoltageSource::toNetlistString() const { stringstream ss; ss << name << " " << getNode(0) << " " << getNode(1) << " SIN ( " << v_offset << " " << v_amplitude << " " << freq << " )"; return ss.str(); }
void SinusoidalVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    double instantaneous_voltage = v_offset + v_amplitude * sin(2 * M_PI * freq * t);
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- PulseVoltageSource ---
PulseVoltageSource::PulseVoltageSource(const string& name, int n1, int n2, double v1, double v2, double td, double tr, double tf, double pw, double per) : VoltageSource(name, n1, n2, v1), v_initial(v1), v_pulsed(v2), t_delay(td), t_rise(tr), t_fall(tf), t_pulse_width(pw), t_period(per) {}
void PulseVoltageSource::setProperties(const map<string, double>& properties) {
    if (properties.count("Initial Value")) v_initial = properties.at("Initial Value");
    if (properties.count("Pulsed Value")) v_pulsed = properties.at("Pulsed Value");
    if (properties.count("Delay")) t_delay = properties.at("Delay");
    if (properties.count("Rise Time")) t_rise = properties.at("Rise Time");
    if (properties.count("Fall Time")) t_fall = properties.at("Fall Time");
    if (properties.count("Pulse Width")) t_pulse_width = properties.at("Pulse Width");
    if (properties.count("Period")) t_period = properties.at("Period");
}
map<string, double> PulseVoltageSource::getProperties() const {
    return {
            {"Initial Value", v_initial}, {"Pulsed Value", v_pulsed},
            {"Delay", t_delay}, {"Rise Time", t_rise}, {"Fall Time", t_fall},
            {"Pulse Width", t_pulse_width}, {"Period", t_period}
    };
}
string PulseVoltageSource::getDisplayValue() const { return "PULSE"; }
void PulseVoltageSource::print() const { cout << "Type: PULSE Source, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), PULSE(...)" << endl; }
string PulseVoltageSource::toNetlistString() const { stringstream ss; ss << name << " " << getNode(0) << " " << getNode(1) << " PULSE ( " << v_initial << " " << v_pulsed << " " << t_delay << " " << t_rise << " " << t_fall << " " << t_pulse_width << " " << t_period << " )"; return ss.str(); }
double PulseVoltageSource::calculate_voltage_at(double t) const {
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
void PulseVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    double instantaneous_voltage = calculate_voltage_at(t);
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- Diode ---
Diode::Diode(const string& name, int n1, int n2, const DiodeModel& modelParams) : Component(name, {n1, n2}) { this->modelName = modelParams.name; this->Is = modelParams.Is; this->Vt = modelParams.Vt; this->n = modelParams.n; this->Vz = modelParams.Vz; }
void Diode::print() const { cout << "Type: Diode, Name: " << name << ", Nodes: (" << getNode(0) << "," << getNode(1) << "), Model: " << modelName << endl; }
string Diode::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + modelName; }
void Diode::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
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
void Diode::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const { /* AC model for diode not implemented */ }

// --- VCVS ---
VCVS::VCVS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain) : Component(name, {n1, n2}), ctrlNode1(ctrl_n1), ctrlNode2(ctrl_n2), gain(gain) {}
void VCVS::setProperties(const map<string, double>& properties) { if (properties.count("Gain")) gain = properties.at("Gain"); }
map<string, double> VCVS::getProperties() const { return {{"Gain", gain}}; }
string VCVS::getDisplayValue() const { return "Gain=" + formatValue(gain); }
void VCVS::print() const { cout << "Type: VCVS, Name: " << name << ", Out: (" << getNode(0) << "," << getNode(1) << "), Control: (" << ctrlNode1 << "," << ctrlNode2 << "), Gain=" << gain << endl; }
string VCVS::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(ctrlNode1) + " " + to_string(ctrlNode2) + " " + to_string(gain); }
void VCVS::updateCtrlNodes(int oldNode, int newNode) { if (ctrlNode1 == oldNode) ctrlNode1 = newNode; if (ctrlNode2 == oldNode) ctrlNode2 = newNode; }
void VCVS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    int cn1 = ctrlNode1 - 1; int cn2 = ctrlNode2 - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (cn1 >= 0) A(current_idx, cn1) = -gain;
    if (cn2 >= 0) A(current_idx, cn2) = gain;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void VCVS::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    int cn1 = ctrlNode1 - 1; int cn2 = ctrlNode2 - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (cn1 >= 0) A(current_idx, cn1) = -gain;
    if (cn2 >= 0) A(current_idx, cn2) = gain;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- VCCS ---
VCCS::VCCS(const string& name, int n1, int n2, int ctrl_n1, int ctrl_n2, double gain) : Component(name, {n1, n2}), ctrlNode1(ctrl_n1), ctrlNode2(ctrl_n2), gain(gain) {}
void VCCS::setProperties(const map<string, double>& properties) { if (properties.count("Gain")) gain = properties.at("Gain"); }
map<string, double> VCCS::getProperties() const { return {{"Gain", gain}}; }
string VCCS::getDisplayValue() const { return "Gain=" + formatValue(gain); }
void VCCS::print() const { cout << "Type: VCCS, Name: " << name << ", Out: (" << getNode(0) << "->" << getNode(1) << "), Control: (" << ctrlNode1 << "," << ctrlNode2 << "), Gain=" << gain << endl; }
string VCCS::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + to_string(ctrlNode1) + " " + to_string(ctrlNode2) + " " + to_string(gain); }
void VCCS::updateCtrlNodes(int oldNode, int newNode) { if (ctrlNode1 == oldNode) ctrlNode1 = newNode; if (ctrlNode2 == oldNode) ctrlNode2 = newNode; }
void VCCS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    int cn1 = ctrlNode1 - 1; int cn2 = ctrlNode2 - 1;
    if (n1 >= 0) { if (cn1 >= 0) A(n1, cn1) += gain; if (cn2 >= 0) A(n1, cn2) -= gain; }
    if (n2 >= 0) { if (cn1 >= 0) A(n2, cn1) -= gain; if (cn2 >= 0) A(n2, cn2) += gain; }
}
void VCCS::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    int cn1 = ctrlNode1 - 1; int cn2 = ctrlNode2 - 1;
    if (n1 >= 0) { if (cn1 >= 0) A(n1, cn1) += gain; if (cn2 >= 0) A(n1, cn2) -= gain; }
    if (n2 >= 0) { if (cn1 >= 0) A(n2, cn1) -= gain; if (cn2 >= 0) A(n2, cn2) += gain; }
}

// --- CCVS ---
CCVS::CCVS(const string& name, int n1, int n2, const string& vctrl_name, double gain) : Component(name, {n1, n2}), ctrlVName(vctrl_name), gain(gain) {}
void CCVS::setProperties(const map<string, double>& properties) { if (properties.count("Gain")) gain = properties.at("Gain"); }
map<string, double> CCVS::getProperties() const { return {{"Gain", gain}}; }
string CCVS::getDisplayValue() const { return "Gain=" + formatValue(gain); }
void CCVS::print() const { cout << "Type: CCVS, Name: " << name << ", Out: (" << getNode(0) << "," << getNode(1) << "), Control Current: I(" << ctrlVName << "), Gain=" << gain << endl; }
string CCVS::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + ctrlVName + " " + to_string(gain); }
void CCVS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (ctrlCurrentIdx != -1) A(current_idx, ctrlCurrentIdx) = gain;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}
void CCVS::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1; int n2 = getNode(1) - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    if (ctrlCurrentIdx != -1) A(current_idx, ctrlCurrentIdx) = gain;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- CCCS ---
CCCS::CCCS(const string& name, int n1, int n2, const string& vctrl_name, double gain) : Component(name, {n1, n2}), ctrlVName(vctrl_name), gain(gain) {}
void CCCS::setProperties(const map<string, double>& properties) { if (properties.count("Gain")) gain = properties.at("Gain"); }
map<string, double> CCCS::getProperties() const { return {{"Gain", gain}}; }
string CCCS::getDisplayValue() const { return "Gain=" + formatValue(gain); }
void CCCS::print() const { cout << "Type: CCCS, Name: " << name << ", Out: (" << getNode(0) << "->" << getNode(1) << "), Control Current: I(" << ctrlVName << "), Gain=" << gain << endl; }
string CCCS::toNetlistString() const { return name + " " + to_string(getNode(0)) + " " + to_string(getNode(1)) + " " + ctrlVName + " " + to_string(gain); }
void CCCS::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (ctrlCurrentIdx != -1) {
        if (n1 >= 0) A(n1, ctrlCurrentIdx) += gain;
        if (n2 >= 0) A(n2, ctrlCurrentIdx) -= gain;
    }
}
void CCCS::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const {
    int n1 = getNode(0) - 1;
    int n2 = getNode(1) - 1;
    if (ctrlCurrentIdx != -1) {
        if (n1 >= 0) A(n1, ctrlCurrentIdx) += gain;
        if (n2 >= 0) A(n2, ctrlCurrentIdx) -= gain;
    }
}

// --- Ground ---
Ground::Ground(const string& name, int n1) : Component(name, {n1}) {}
void Ground::print() const { cout << "Type: Ground, Name: " << name << ", Node: " << getNode(0) << endl; }
string Ground::toNetlistString() const { return name + " " + to_string(getNode(0)); }
void Ground::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) { /* Ground does not stamp */ }
void Ground::stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const { /* Ground does not stamp */ }

