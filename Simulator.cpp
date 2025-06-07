#include "Simulator.h"
#include <stdexcept>
#include <algorithm>
#include <set>

void Simulator::run() {
    string command;
    cout << "Welcome to the Circuit Simulator!" << endl;
    cout << "Enter commands ('netlist path/to/file.txt', 'run 1m 1u', 'exit')." << endl;

    while (true) {
        cout << "> ";
        getline(cin, command);
        if (command == "exit" || cin.eof()) break;
        if (!command.empty()) {
            try {
                processCommand(command);
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }
    cout << "Simulator terminated." << endl;
}

void Simulator::processCommand(const string& command) {
    stringstream ss(command);
    string token;
    vector<string> tokens;
    while (ss >> token) {
        tokens.push_back(token);
    }
    if (tokens.empty()) return;

    string cmd = tokens[0];
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    if (cmd == "add") handleAdd(tokens);
    else if (cmd == "delete") handleDelete(tokens);
    else if (cmd == "run") handleRun(tokens);
    else if (cmd == "list") handleList(tokens);
    else if (cmd == "nodes") handleNodes();
    else if (cmd == "reset") handleReset();
    else if (cmd == "netlist") handleNetlist(tokens);
    else throw runtime_error("Unknown command '" + tokens[0] + "'");
}

void Simulator::addComponentFromTokens(const vector<string>& args) {
    if (args.empty()) return;

    const string& name = args[0];
    if (circuit.hasComponent(name)) {
        throw runtime_error("Component '" + name + "' already exists.");
    }

    char compType = toupper(name[0]);

    switch (compType) {
        case 'R':
        case 'C':
        case 'L': {
            if (args.size() != 4) throw runtime_error("R/C/L definition requires exactly 4 arguments.");
            int n1 = stoi(args[1]);
            int n2 = stoi(args[2]);
            double value = parseValue(args[3]);
            if (value <= 0) throw runtime_error("Value for R, C, or L must be positive.");
            if (compType == 'R') circuit.addComponent(make_unique<Resistor>(name, n1, n2, value));
            else if (compType == 'C') circuit.addComponent(make_unique<Capacitor>(name, n1, n2, value));
            else if (compType == 'L') circuit.addComponent(make_unique<Inductor>(name, n1, n2, value));
            break;
        }
        case 'V': {
            if (args.size() < 4) throw runtime_error("V source definition requires at least 4 arguments.");
            int n1 = stoi(args[1]);
            int n2 = stoi(args[2]);
            if (args.size() >= 8 && args[3] == "SIN") {
                if (args[4] != "(" || args[7].back() != ')') throw runtime_error("Syntax for SIN: SIN ( Voff Vamp Freq )");
                double v_off = parseValue(args[5]);
                double v_amp = parseValue(args[6]);
                string freq_str = args[7];
                freq_str.pop_back();
                double freq = parseValue(freq_str);
                circuit.addComponent(make_unique<SinusoidalVoltageSource>(name, n1, n2, v_off, v_amp, freq));
            } else if (args.size() == 4) {
                double value = parseValue(args[3]);
                circuit.addComponent(make_unique<VoltageSource>(name, n1, n2, value));
            } else {
                throw runtime_error("Invalid arguments for V source.");
            }
            break;
        }
        default: {
            throw runtime_error("Unknown component type '" + string(1, compType) + "'");
        }
    }
}

void Simulator::handleAdd(const vector<string>& tokens) {
    if (tokens.size() < 2) throw runtime_error("'add' requires arguments.");
    vector<string> args(tokens.begin() + 1, tokens.end());
    addComponentFromTokens(args);
    cout << "Added component " << args[0] << endl;
}

void Simulator::handleNetlist(const vector<string>& tokens) {
    if (tokens.size() != 2) throw runtime_error("Usage: netlist <filepath>");
    string filepath = tokens[1];
    ifstream file(filepath);
    if (!file.is_open()) throw runtime_error("Could not open file: " + filepath);
    handleReset();
    cout << "Loading netlist from " << filepath << "..." << endl;
    string line;
    int line_num = 0;
    while (getline(file, line)) {
        line_num++;
        if (line.empty() || line[0] == '*') continue;
        stringstream ss(line);
        string token;
        vector<string> args;
        while (ss >> token) {
            args.push_back(token);
        }
        if (!args.empty()) {
            try {
                cout << "Processing line " << line_num << ": " << line << endl;
                addComponentFromTokens(args);
            } catch (const exception& e) {
                cerr << "  -> Failed on line " << line_num << ": " << e.what() << endl;
            }
        }
    }
    cout << "Netlist loading finished." << endl;
}

void Simulator::handleDelete(const vector<string>& tokens) {
    if (tokens.size() != 2) throw runtime_error("Usage: delete <CompName>");
    string name = tokens[1];
    if (!circuit.removeComponent(name)) throw runtime_error("Component '" + name + "' not found.");
    cout << "Deleted component " << name << endl;
}

void Simulator::handleRun(const vector<string>& tokens) {
    if (tokens.size() != 3) throw runtime_error("Usage: run <EndTime> <TimeStep>");
    double endTime = parseValue(tokens[1]);
    double timeStep = parseValue(tokens[2]);
    circuit.runTransientAnalysis(endTime, timeStep);
}

void Simulator::handleList(const vector<string>& tokens) {
    if (tokens.size() > 2) throw runtime_error("Usage: list [type]");
    if (tokens.size() == 2) circuit.printCircuit(tokens[1][0]);
    else circuit.printCircuit('A');
}

void Simulator::handleNodes() {
    set<int> nodes = circuit.getNodes();
    if (nodes.empty()) {
        cout << "No nodes in the circuit." << endl;
        return;
    }
    cout << "Available nodes: ";
    for (int node : nodes) cout << node << " ";
    cout << endl;
}

void Simulator::handleReset() {
    circuit.clear();
}
