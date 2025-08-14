#include "SubCircuit.h"
#include "Circuit.h"
#include <stdexcept>
#include <iostream>
#include <QString>
#include <QFileInfo>

SubCircuit::SubCircuit(const std::string& name, const std::vector<int>& externalNodes, const std::string& definitionFile)
        : Component(name, externalNodes), m_definitionFile(definitionFile) {}

SubCircuit::SubCircuit() : Component() {}

unique_ptr<Component> SubCircuit::clone() const {
    return make_unique<SubCircuit>(*this);
}

void SubCircuit::print() const {
    std::cout << "Type: SubCircuit, Name: " << name << ", Definition: " << m_definitionFile << ", Nodes: (";
    for (size_t i = 0; i < nodes.size(); ++i) {
        std::cout << nodes[i] << (i == nodes.size() - 1 ? "" : ",");
    }
    std::cout << ")" << std::endl;
}

void SubCircuit::stamp(MatrixXd&, VectorXd&, const VectorXd&, int, double, double) {
    throw std::logic_error("SubCircuit::stamp should not be called directly. Circuit should be flattened first.");
}

void SubCircuit::stampAC(MatrixXcd&, VectorXcd&, int, double) const {
    throw std::logic_error("SubCircuit::stampAC should not be called directly. Circuit should be flattened first.");
}

string SubCircuit::toNetlistString() const {
    std::string str = name;
    for (int node : nodes) {
        str += " " + std::to_string(node);
    }
    if (m_definitionFile.empty()) {
        str += " EMPTY_DEF";
        return str;
    }
    QString fullPath = QString::fromStdString(m_definitionFile);
    QFileInfo fileInfo(fullPath);
    std::string definitionName = fileInfo.baseName().toStdString();

    str += " " + definitionName;
    return str;
}

unique_ptr<Circuit> SubCircuit::loadInternalCircuit() const {
    if (m_definitionFile.empty()) {
        return nullptr;
    }
    auto circuit = make_unique<Circuit>();
    try {
        circuit->loadFromFile(m_definitionFile);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load subcircuit '" + name + "' from file '" + m_definitionFile + "': " + e.what());
    }
    return circuit;
}