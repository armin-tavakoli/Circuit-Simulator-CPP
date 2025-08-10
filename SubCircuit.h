#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "Component.h"
#include <vector>
#include <string>
#include <memory>

class Circuit;

class SubCircuit : public Component {
public:
    SubCircuit(const std::string& name, const std::vector<int>& externalNodes, const std::string& definitionFile);
    SubCircuit();
    ~SubCircuit() override = default;

    unique_ptr<Component> clone() const override;

    void print() const override;
    void stamp(MatrixXd& A, VectorXd& b, const VectorXd& x_prev_nr, int current_idx, double h, double t) override;
    void stampAC(MatrixXcd& A, VectorXcd& b, int current_idx, double omega) const override;
    string toNetlistString() const override;

    unique_ptr<Circuit> loadInternalCircuit() const;
    const std::string& getDefinitionFile() const { return m_definitionFile; }

    template<class Archive>
    void serialize(Archive & ar) {
        ar(cereal::base_class<Component>(this), CEREAL_NVP(m_definitionFile));
    }

private:
    std::string m_definitionFile;
};

#endif
