#include "Simulator.h"
#include <stdexcept>
#include <algorithm>
#include <set>
#include <filesystem>

Simulator::Simulator() { setupDefaultModels(); }

void Simulator::setupDefaultModels() {
    DiodeModel standard;
    standard.name = "D";
    diodeModels["D"] = standard;
    DiodeModel zener;
    zener.name = "Z";
    zener.Vz = 5.1;
    diodeModels["Z"] = zener;
}

void Simulator::run() {
    string command;
    cout << "Welcome to the Circuit Simulator!" << endl;
    cout << "Enter commands ('dc...', 'exit')." << endl;
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

    // دستورات .tran و tran را به print TRAN هدایت می‌کنیم
    if (cmd == ".tran" || (cmd == "print" && tokens.size() > 1 && tokens[1] == "TRAN")) {
        handlePrint(tokens);
    }
    else if (cmd == "add") handleAdd(tokens);
    else if (cmd == "delete") handleDelete(tokens);
    else if (cmd == "run") handleRun(tokens);
    else if (cmd == "list") handleList(tokens);
    else if (cmd == "nodes") handleNodes();
    else if (cmd == "reset") handleReset();
    else if (cmd == "netlist") handleNetlist(tokens);
    else if (cmd == "rename") handleRenameNode(tokens);
    else if (cmd == "gnd") handleGnd(tokens);
    else if (cmd == "show") handleShow(tokens);
    else if (cmd == "dc") handleDC(tokens);
    else throw runtime_error("Unknown command '" + tokens[0] + "'");
}

void Simulator::addComponentFromTokens(const vector<string>& args) {
    if (args.empty()) return;
    const string& name = args[0];
    if (circuit.hasComponent(name)) throw runtime_error("Component '" + name + "' already exists.");
    char compType = toupper(name[0]);

    switch (compType) {
        case 'R': case 'C': case 'L': case 'I': {
            if (args.size() != 4) throw runtime_error("R/C/L/I definition requires 4 arguments.");
            int n1 = stoi(args[1]), n2 = stoi(args[2]);
            double value = parseValue(args[3]);
            if (value <= 0 && compType != 'I') throw runtime_error("Value for R/C/L must be positive.");
            if (compType == 'R') circuit.addComponent(make_unique<Resistor>(name, n1, n2, value));
            else if (compType == 'C') circuit.addComponent(make_unique<Capacitor>(name, n1, n2, value));
            else if (compType == 'L') circuit.addComponent(make_unique<Inductor>(name, n1, n2, value));
            else if (compType == 'I') circuit.addComponent(make_unique<CurrentSource>(name, n1, n2, value));
            break;
        }
        case 'V': {
            if (args.size() < 4) throw runtime_error("V source definition requires at least 4 arguments.");
            int n1 = stoi(args[1]), n2 = stoi(args[2]);
            if (args.size() >= 5 && args[3] == "SIN") {
                auto it_open = find(args.begin(), args.end(), "(");
                auto it_close = find(args.begin(), args.end(), ")");
                if (it_open == args.end() || it_close == args.end() || distance(it_open, it_close) != 4) throw runtime_error("Syntax for SIN: SIN ( Voff Vamp Freq )");
                double v_off = parseValue(*(it_open + 1)), v_amp = parseValue(*(it_open + 2)), freq = parseValue(*(it_open + 3));
                circuit.addComponent(make_unique<SinusoidalVoltageSource>(name, n1, n2, v_off, v_amp, freq));
            } else if (args.size() == 4) {
                double value = parseValue(args[3]);
                circuit.addComponent(make_unique<VoltageSource>(name, n1, n2, value));
            } else {
                throw runtime_error("Invalid arguments for V source.");
            }
            break;
        }
        case 'D': {
            if (args.size() != 4) throw runtime_error("Diode definition requires: D<name> n1 n2 <model>");
            int n1 = stoi(args[1]), n2 = stoi(args[2]);
            const string& modelName = args[3];
            if (diodeModels.find(modelName) == diodeModels.end()) throw runtime_error("Model <" + modelName + "> not found");
            circuit.addComponent(make_unique<Diode>(name, n1, n2, diodeModels.at(modelName)));
            break;
        }
        case 'E': {
            if (args.size() != 6) throw runtime_error("VCVS(E) definition requires: E<name> n+ n- c_n+ c_n- gain");
            int n1 = stoi(args[1]), n2 = stoi(args[2]), cn1 = stoi(args[3]), cn2 = stoi(args[4]);
            double gain = parseValue(args[5]);
            circuit.addComponent(make_unique<VCVS>(name, n1, n2, cn1, cn2, gain));
            break;
        }
        case 'G': {
            if (args.size() != 6) throw runtime_error("VCCS(G) definition requires: G<name> n+ n- c_n+ c_n- gain");
            int n1 = stoi(args[1]), n2 = stoi(args[2]), cn1 = stoi(args[3]), cn2 = stoi(args[4]);
            double gain = parseValue(args[5]);
            circuit.addComponent(make_unique<VCCS>(name, n1, n2, cn1, cn2, gain));
            break;
        }
        case 'H': {
            if (args.size() != 5) throw runtime_error("CCVS(H) definition requires: H<name> n+ n- v_ctrl gain");
            int n1 = stoi(args[1]), n2 = stoi(args[2]);
            const string& vctrl_name = args[3];
            double gain = parseValue(args[4]);
            circuit.addComponent(make_unique<CCVS>(name, n1, n2, vctrl_name, gain));
            break;
        }
        case 'F': {
            if (args.size() != 5) throw runtime_error("CCCS(F) definition requires: F<name> n+ n- v_ctrl gain");
            int n1 = stoi(args[1]), n2 = stoi(args[2]);
            const string& vctrl_name = args[3];
            double gain = parseValue(args[4]);
            circuit.addComponent(make_unique<CCCS>(name, n1, n2, vctrl_name, gain));
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

void Simulator::handleRenameNode(const vector<string>& tokens) {
    if (tokens.size() != 4 || tokens[1] != "node") {
        throw runtime_error("Syntax error. Usage: rename node <old_name> <new_name>");
    }
    int oldNode = stoi(tokens[2]);
    int newNode = stoi(tokens[3]);
    set<int> existingNodes = circuit.getNodes();
    if (existingNodes.find(oldNode) == existingNodes.end()) {
        throw runtime_error("Node <" + to_string(oldNode) + "> does not exist in the circuit");
    }
    if (newNode != 0 && existingNodes.find(newNode) != existingNodes.end()) {
        throw runtime_error("Node name <" + to_string(newNode) + "> already exists");
    }
    circuit.renameNode(oldNode, newNode);
    cout << "SUCCESS: Node renamed from <" << oldNode << "> to <" << newNode << ">" << endl;
}

void Simulator::handleRun(const vector<string>& tokens) {
    if (tokens.size() < 3) throw runtime_error("Usage: run <Tstop> <Tstep> [Tstart] [Tmaxstep]");
    double Tstop = parseValue(tokens[1]);
    double Tstep = parseValue(tokens[2]);
    double Tstart = (tokens.size() > 3) ? parseValue(tokens[3]) : 0.0;
    double Tmaxstep = (tokens.size() > 4) ? parseValue(tokens[4]) : 0.0;
    circuit.runTransientAnalysis(Tstop, Tstep, {}, Tstart, Tmaxstep);
}

void Simulator::handlePrint(const vector<string>& tokens) {
    // .tran Tstep Tstop [Tstart] [Tmaxstep] V(1) ...
    // print TRAN Tstep Tstop [Tstart] [Tmaxstep] V(1) ...

    // پیدا کردن اولین پارامتر زمانی
    int time_params_start_idx = (tokens[0] == ".tran") ? 1 : 2;

    if (tokens.size() < time_params_start_idx + 2) {
        throw runtime_error("Syntax error. Not enough parameters for transient analysis.");
    }

    double Tstep = parseValue(tokens[time_params_start_idx]);
    double Tstop = parseValue(tokens[time_params_start_idx + 1]);

    // پیدا کردن اندیس شروع متغیرهای چاپ
    int vars_start_idx = time_params_start_idx + 2;

    double Tstart = 0.0;
    if (tokens.size() > vars_start_idx && tokens[vars_start_idx][0] != 'V' && tokens[vars_start_idx][0] != 'I') {
        Tstart = parseValue(tokens[vars_start_idx]);
        vars_start_idx++;
    }

    double Tmaxstep = 0.0;
    if (tokens.size() > vars_start_idx && tokens[vars_start_idx][0] != 'V' && tokens[vars_start_idx][0] != 'I') {
        Tmaxstep = parseValue(tokens[vars_start_idx]);
        vars_start_idx++;
    }

    vector<PrintVariable> printVars;
    for (size_t i = vars_start_idx; i < tokens.size(); ++i) {
        const string& varStr = tokens[i];
        if (varStr.length() < 4) throw runtime_error("Invalid variable format: " + varStr);
        char type = toupper(varStr[0]);
        string id = varStr.substr(2, varStr.length() - 3);
        if ((type == 'V' || type == 'I') && varStr[1] == '(' && varStr.back() == ')') {
            printVars.push_back({type, id});
        } else {
            throw runtime_error("Invalid variable format: " + varStr + ". Expected V(node) or I(comp).");
        }
    }

    circuit.runTransientAnalysis(Tstop, Tstep, printVars, Tstart, Tmaxstep);
}

void Simulator::handleGnd(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        throw runtime_error("Syntax error. Usage: gnd <node_number>");
    }
    int oldNode;
    try {
        oldNode = stoi(tokens[1]);
    } catch (const invalid_argument& e) {
        throw runtime_error("Invalid node number: " + tokens[1]);
    }
    if (oldNode == 0) {
        cout << "Node " << oldNode << " is already the ground node." << endl;
        return;
    }
    int newNode = 0;
    set<int> existingNodes = circuit.getNodes();
    if (existingNodes.find(oldNode) == existingNodes.end()) {
        throw runtime_error("Node <" + to_string(oldNode) + "> does not exist in the circuit");
    }
    circuit.renameNode(oldNode, newNode);
    cout << "SUCCESS: Node " << oldNode << " is now connected to ground (renamed to 0)." << endl;
}

void Simulator::handleShow(const vector<string>& tokens) {
    if (tokens.size() != 2 || tokens[1] != "schematics") {
        throw runtime_error("Syntax error. Usage: show schematics");
    }
    string path = ".";
    vector<string> schematicFiles;
    cout << "Searching for schematics in current directory..." << endl;
    for (const auto& entry : filesystem::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            schematicFiles.push_back(entry.path().filename().string());
        }
    }
    if (schematicFiles.empty()) {
        cout << "No schematic files (.txt) found in the current directory." << endl;
        return;
    }
    cout << "--- Available Schematics ---" << endl;
    for (size_t i = 0; i < schematicFiles.size(); ++i) {
        cout << i + 1 << "- " << schematicFiles[i] << endl;
    }
    cout << "----------------------------" << endl;
    while (true) {
        cout << "Choose a schematic (1-" << schematicFiles.size() << ") or type 'return' to go back: ";
        string choice_str;
        getline(cin, choice_str);
        if (choice_str == "return") {
            cout << "Returning to main menu." << endl;
            break;
        }
        try {
            int choice = stoi(choice_str);
            if (choice >= 1 && choice <= schematicFiles.size()) {
                string selectedFile = schematicFiles[choice - 1];
                processCommand("netlist " + selectedFile);
                break;
            } else {
                cerr << "Error: Invalid number. Please choose between 1 and " << schematicFiles.size() << "." << endl;
            }
        } catch (const invalid_argument& e) {
            cerr << "Error: Invalid input. Please enter a number or 'return'." << endl;
        }
    }
}

void Simulator::handleDC(const vector<string>& tokens) {
    if (tokens.size() < 5) {
        throw runtime_error("Syntax error. Usage: DC <SrcName> <Start> <End> <Incr> <Var1> ...");
    }
    const string& srcName = tokens[1];
    double start = parseValue(tokens[2]);
    double end = parseValue(tokens[3]);
    double incr = parseValue(tokens[4]);

    if (incr == 0) throw runtime_error("Increment for DC sweep cannot be zero.");

    vector<PrintVariable> printVars;
    for (size_t i = 5; i < tokens.size(); ++i) {
        const string& varStr = tokens[i];
        if (varStr.length() < 4) throw runtime_error("Invalid variable format: " + varStr);
        char type = toupper(varStr[0]);
        string id = varStr.substr(2, varStr.length() - 3);
        if ((type == 'V' || type == 'I') && varStr[1] == '(' && varStr.back() == ')') {
            printVars.push_back({type, id});
        } else {
            throw runtime_error("Invalid variable format: " + varStr);
        }
    }

    if (tokens.size() > 5 && printVars.empty()) cout << "Warning: No valid variables found to print." << endl;

    circuit.runDCSweep(srcName, start, end, incr, printVars);
}