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
void Inductor::print() const { cout << "Type: Inductor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), L=" << inductance << " H" << endl; }
void SinusoidalVoltageSource::print() const {
    cout << "Type: SIN Source, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), SIN(" << v_offset << " " << v_amplitude << " " << freq << "Hz)" << endl;
}
void Diode::print() const {
    cout << "Type: Diode, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), Model: " << modelName << endl;
}


// --- متدهای Stamp ---

void Resistor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g = 1.0 / resistance;
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(n1, n1) += g;
    if (n2 >= 0) A(n2, n2) += g;
    if (n1 >= 0 && n2 >= 0) {
        A(n1, n2) -= g;
        A(n2, n1) -= g;
    }
}

void Capacitor::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    double g_eq = capacitance / h;
    double I_eq = g_eq * prev_voltage;
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(n1, n1) += g_eq;
    if (n2 >= 0) A(n2, n2) += g_eq;
    if (n1 >= 0 && n2 >= 0) {
        A(n1, n2) -= g_eq;
        A(n2, n1) -= g_eq;
    }
    if (n1 >= 0) b(n1) += I_eq;
    if (n2 >= 0) b(n2) -= I_eq;
}

void VoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = this->voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
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

void SinusoidalVoltageSource::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    double instantaneous_voltage = v_offset + v_amplitude * sin(2 * M_PI * freq * t);
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

// --- پیاده‌سازی کلاس دیود (نسخه نهایی و اصلاح شده) ---

Diode::Diode(const string& name, int n1, int n2, const DiodeModel& modelParams)
        : Component(name, n1, n2) {
    this->modelName = modelParams.name;
    this->Is = modelParams.Is;
    this->Vt = modelParams.Vt;
    this->n = modelParams.n;
    this->Vz = modelParams.Vz;
}

void Diode::stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;

    double v1 = (n1 >= 0) ? x_prev_nr(n1) : 0.0;
    double v2 = (n2 >= 0) ? x_prev_nr(n2) : 0.0;
    double Vd = v1 - v2;

    // مدل ناحیه شکست زنر
    if (Vz > 0 && Vd < -Vz) {
        // مدل‌سازی ناحیه شکست با یک رسانایی بزرگ و یک منبع ولتاژ
        // I_D = Gz * (Vd + Vz)
        const double Gz = 100.0; // رسانایی بزرگ در ناحیه شکست
        double Ieq_zener = Gz * Vz;

        // اعمال بخش رسانایی (Gz)
        if (n1 >= 0) A(n1, n1) += Gz;
        if (n2 >= 0) A(n2, n2) += Gz;
        if (n1 >= 0 && n2 >= 0) {
            A(n1, n2) -= Gz;
            A(n2, n1) -= Gz;
        }

        // اعمال بخش منبع جریان.
        if (n1 >= 0) b(n1) -= Ieq_zener;
        if (n2 >= 0) b(n2) += Ieq_zener;
        return;
    }

    // مدل دیود استاندارد (بایاس مستقیم و معکوس غیرشکست)
    double Vd_limited = Vd;
    // فقط ولتاژ مثبت را برای جلوگیری از سرریز عددی محدود می‌کنیم
    if (Vd > 0.7) {
        Vd_limited = 0.7;
    }

    double exp_val = exp(Vd_limited / (n * Vt));
    double Id = Is * (exp_val - 1.0);
    double Geq = (Is / (n * Vt)) * exp_val;
    double Ieq_comp = Id - Geq * Vd_limited;

    if (n1 >= 0) { A(n1, n1) += Geq; b(n1) -= Ieq_comp; }
    if (n2 >= 0) { A(n2, n2) += Geq; b(n2) += Ieq_comp; }
    if (n1 >= 0 && n2 >= 0) { A(n1, n2) -= Geq; A(n2, n1) -= Geq; }
}


void Component::updateNode(int oldNode, int newNode) {
    if (node1 == oldNode) {
        node1 = newNode;
    }
    if (node2 == oldNode) {
        node2 = newNode;
    }
}
