#include "Component.h"
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Resistor::print() const { cout << "Type: Resistor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), R=" << resistance << " Ohms" << endl; }
void Capacitor::print() const { cout << "Type: Capacitor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), C=" << capacitance << " F" << endl; }
void VoltageSource::print() const { cout << "Type: DC Source, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), V=" << voltage << " V" << endl; }
void Inductor::print() const { cout << "Type: Inductor, Name: " << name << ", Nodes: (" << node1 << "," << node2 << "), L=" << inductance << " H" << endl; }
void SinusoidalVoltageSource::print() const {
    cout << "Type: SIN Source, Name: " << name << ", Nodes: (" << node1 << "," << node2
         << "), SIN(" << v_offset << " " << v_amplitude << " " << freq << "Hz)" << endl;
}

void Resistor::stamp(MatrixXd& A, VectorXd& b, int current_idx, double h, double t) {
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

void Capacitor::stamp(MatrixXd& A, VectorXd& b, int current_idx, double h, double t) {
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

void VoltageSource::stamp(MatrixXd& A, VectorXd& b, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = this->voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

void Inductor::stamp(MatrixXd& A, VectorXd& b, int current_idx, double h, double t) {
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

void SinusoidalVoltageSource::stamp(MatrixXd& A, VectorXd& b, int current_idx, double h, double t) {
    int n1 = node1 - 1;
    int n2 = node2 - 1;
    double instantaneous_voltage = v_offset + v_amplitude * sin(2 * M_PI * freq * t);
    if (n1 >= 0) A(current_idx, n1) = 1.0;
    if (n2 >= 0) A(current_idx, n2) = -1.0;
    b(current_idx) = instantaneous_voltage;
    if (n1 >= 0) A(n1, current_idx) = 1.0;
    if (n2 >= 0) A(n2, current_idx) = -1.0;
}

void Component::updateNode(int oldNode, int newNode) {
    if (node1 == oldNode) {
        node1 = newNode;
    }
    if (node2 == oldNode) {
        node2 = newNode;
    }
}