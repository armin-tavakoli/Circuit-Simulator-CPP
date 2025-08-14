#include "Circuit.h"
#include <fstream>
#include <iostream>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include "cereal_registration.h"

void Circuit::saveToFile(const std::string& filepath) {
    std::ofstream os(filepath, std::ios::binary);
    if (!os) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }
    cereal::BinaryOutputArchive archive(os);
    archive(CEREAL_NVP(components), CEREAL_NVP(wires), CEREAL_NVP(m_externalPorts));
    std::cout << "Circuit saved successfully to " << filepath << std::endl;
}

void Circuit::loadFromFile(const std::string& filepath) {
    std::ifstream is(filepath, std::ios::binary);
    if (!is) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }
    cereal::BinaryInputArchive archive(is);
    clear();
    archive(components, wires, m_externalPorts);
    std::cout << "Circuit loaded successfully from " << filepath << std::endl;
}

const map<string, vector<double>>& Circuit::getSimulationResults() const {
    return simulationResults;
}


TheveninEquivalent Circuit::calculateTheveninEquivalent(int port1_node, int port2_node)
{
    TheveninEquivalent result;

    {
        unique_ptr<Circuit> vth_circuit = this->clone();
        vth_circuit->analyzeCircuit();

        int matrix_size = vth_circuit->getNodeCount() + vth_circuit->getCurrentVarCount();
        if (matrix_size == 0) return result;

        MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
        VectorXd b = VectorXd::Zero(matrix_size);
        VectorXd x_guess = VectorXd::Zero(matrix_size); // برای DC guess اولیه صفر است

        for (const auto& comp : vth_circuit->getComponents()) {
            int current_idx = -1;
            if (comp->addsCurrentVariable()) {
                current_idx = vth_circuit->getNodeCount() + vth_circuit->currentComponentMap.at(comp->getName()) - 1;
            }
            comp->stamp(A, b, x_guess, current_idx, 1e12, 0.0);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b);

        double v1 = (port1_node > 0) ? x(port1_node - 1) : 0.0;
        double v2 = (port2_node > 0) ? x(port2_node - 1) : 0.0;
        result.Vth = v1 - v2;
    }

    {
        unique_ptr<Circuit> rth_circuit = this->clone();

        for (const auto& comp_ptr : rth_circuit->getComponents()) {
            if (dynamic_cast<VoltageSource*>(comp_ptr.get()) || dynamic_cast<CurrentSource*>(comp_ptr.get())) {
                comp_ptr->setProperties({{"Voltage", 0.0}, {"Current", 0.0}});
            }
        }

        rth_circuit->addComponent(make_unique<CurrentSource>("I_test", port1_node, port2_node, 1.0));
        rth_circuit->analyzeCircuit();

        int matrix_size = rth_circuit->getNodeCount() + rth_circuit->getCurrentVarCount();
        MatrixXd A = MatrixXd::Zero(matrix_size, matrix_size);
        VectorXd b = VectorXd::Zero(matrix_size);
        VectorXd x_guess = VectorXd::Zero(matrix_size);

        for (const auto& comp : rth_circuit->getComponents()) {
            int current_idx = -1;
            if (comp->addsCurrentVariable()) {
                current_idx = rth_circuit->getNodeCount() + rth_circuit->currentComponentMap.at(comp->getName()) - 1;
            }
            comp->stamp(A, b, x_guess, current_idx, 1e12, 0.0);
        }

        VectorXd x = A.colPivHouseholderQr().solve(b);

        double v1 = (port1_node > 0) ? x(port1_node - 1) : 0.0;
        double v2 = (port2_node > 0) ? x(port2_node - 1) : 0.0;
        result.Rth = v1 - v2;
    }

    return result;
}