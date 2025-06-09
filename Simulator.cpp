#include "Simulator.h"
#include <stdexcept>
#include <algorithm>
#include <set>
#include <filesystem>
#include <regex>


Simulator::Simulator() {
    setupDefaultModels();
}
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
    cout << "Enter commands or type help." << endl;

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

void Simulator::processCommand(const string& command_in) {
    string command = command_in;
    command = regex_replace(command, regex("\\("), " ( ");
    command = regex_replace(command, regex("\\)"), " ) ");

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
    else if (cmd == "rename") handleRenameNode(tokens);
    else if (cmd == "print") handlePrint(tokens);
    else if (cmd == "gnd") handleGnd(tokens);
    else if (cmd == "show") handleShow(tokens);
    else if (cmd == "dc") handleDC(tokens);
    else if (cmd == "help") handleHelp();
    else if (cmd == "save") handleSave(tokens);
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
        case 'L':
        case 'I': {
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

            string sourceType = (args.size() > 3) ? args[3] : "DC";
            transform(sourceType.begin(), sourceType.end(), sourceType.begin(), ::toupper);

            if (sourceType == "SIN") {
                if (args.size() < 8) throw runtime_error("Syntax for SIN: SIN ( Voff Vamp Freq )");
                auto it_open = find(args.begin(), args.end(), "(");
                auto it_close = find(args.begin(), args.end(), ")");
                if (it_open == args.end() || it_close == args.end() || distance(it_open, it_close) != 4) {
                    throw runtime_error("Syntax error for SIN source. Expected format: SIN ( Voff Vamp Freq )");
                }
                double v_off = parseValue(*(it_open + 1));
                double v_amp = parseValue(*(it_open + 2));
                double freq = parseValue(*(it_open + 3));
                circuit.addComponent(make_unique<SinusoidalVoltageSource>(name, n1, n2, v_off, v_amp, freq));
            }
            else if (sourceType == "PULSE") {
                if (args.size() < 12) throw runtime_error("Syntax for PULSE: PULSE ( V1 V2 Td Tr Tf Pw Per )");
                auto it_open = find(args.begin(), args.end(), "(");
                auto it_close = find(args.begin(), args.end(), ")");
                if (it_open == args.end() || it_close == args.end() || distance(it_open, it_close) != 8) {
                    throw runtime_error("Syntax error for PULSE source. Expected format: PULSE ( V1 V2 Td Tr Tf Pw Per )");
                }
                double v1 = parseValue(*(it_open + 1));
                double v2 = parseValue(*(it_open + 2));
                double td = parseValue(*(it_open + 3));
                double tr = parseValue(*(it_open + 4));
                double tf = parseValue(*(it_open + 5));
                double pw = parseValue(*(it_open + 6));
                double per = parseValue(*(it_open + 7));
                circuit.addComponent(make_unique<PulseVoltageSource>(name, n1, n2, v1, v2, td, tr, tf, pw, per));
            }
            else {
                if (args.size() != 4) throw runtime_error("DC Voltage source definition requires 4 arguments: V<name> n1 n2 <value>");
                double value = parseValue(args[3]);
                circuit.addComponent(make_unique<VoltageSource>(name, n1, n2, value));
            }
            break;
        }
        case 'D': case 'E': case 'G': case 'H': case 'F': {
            if (args.size() < 5) throw runtime_error("Dependent source definitions require at least 5 arguments.");
            if (compType == 'D') {
                if (args.size() != 4) throw runtime_error("Diode definition requires: D<name> n1 n2 <model>");
                int n1 = stoi(args[1]), n2 = stoi(args[2]);
                const string& modelName = args[3];
                if (diodeModels.find(modelName) == diodeModels.end()) throw runtime_error("Model <" + modelName + "> not found");
                circuit.addComponent(make_unique<Diode>(name, n1, n2, diodeModels.at(modelName)));
            } else if (compType == 'E') {
                if (args.size() != 6) throw runtime_error("VCVS(E) requires: E<name> n+ n- c_n+ c_n- gain");
                int n1 = stoi(args[1]), n2 = stoi(args[2]), cn1 = stoi(args[3]), cn2 = stoi(args[4]);
                double gain = parseValue(args[5]);
                circuit.addComponent(make_unique<VCVS>(name, n1, n2, cn1, cn2, gain));
            } else if (compType == 'G') {
                if (args.size() != 6) throw runtime_error("VCCS(G) requires: G<name> n+ n- c_n+ c_n- gain");
                int n1 = stoi(args[1]), n2 = stoi(args[2]), cn1 = stoi(args[3]), cn2 = stoi(args[4]);
                double gain = parseValue(args[5]);
                circuit.addComponent(make_unique<VCCS>(name, n1, n2, cn1, cn2, gain));
            } else if (compType == 'H') {
                if (args.size() != 5) throw runtime_error("CCVS(H) requires: H<name> n+ n- v_ctrl gain");
                int n1 = stoi(args[1]), n2 = stoi(args[2]);
                const string& vctrl_name = args[3];
                double gain = parseValue(args[4]);
                circuit.addComponent(make_unique<CCVS>(name, n1, n2, vctrl_name, gain));
            } else if (compType == 'F') {
                if (args.size() != 5) throw runtime_error("CCCS(F) requires: F<name> n+ n- v_ctrl gain");
                int n1 = stoi(args[1]), n2 = stoi(args[2]);
                const string& vctrl_name = args[3];
                double gain = parseValue(args[4]);
                circuit.addComponent(make_unique<CCCS>(name, n1, n2, vctrl_name, gain));
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
        line = regex_replace(line, regex("\\("), " ( ");
        line = regex_replace(line, regex("\\)"), " ) ");
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
    string first_token = (tokens.size() > 1) ? tokens[1] : "";
    transform(first_token.begin(), first_token.end(), first_token.begin(), ::toupper);

    if (tokens.size() < 3 || first_token != "TRAN") {
        throw runtime_error("Syntax error. Expected: print TRAN <Tstep> <Tstop> ...");
    }

    double Tstep = parseValue(tokens[2]);
    double Tstop = parseValue(tokens[3]);

    size_t vars_start_idx = 4;
    double Tstart = 0.0;
    double Tmaxstep = 0.0;

    // Logic to parse optional Tstart and Tmaxstep parameters
    if (vars_start_idx < tokens.size() && (tokens[vars_start_idx].find_first_of("vViI") != 0)) {
        Tstart = parseValue(tokens[vars_start_idx]);
        vars_start_idx++;
    }
    if (vars_start_idx < tokens.size() && (tokens[vars_start_idx].find_first_of("vViI") != 0)) {
        Tmaxstep = parseValue(tokens[vars_start_idx]);
        vars_start_idx++;
    }

    vector<PrintVariable> printVars;

    while (vars_start_idx < tokens.size()) {
        const string& type_token = tokens[vars_start_idx];
        char type;
        if (type_token.length() == 1 && (toupper(type_token[0]) == 'V' || toupper(type_token[0]) == 'I')) {
            type = toupper(type_token[0]);
        } else {
            throw runtime_error("Invalid variable format: '" + type_token + "'. Expected 'V' or 'I' to start a variable specification.");
        }

        if (vars_start_idx + 1 >= tokens.size() || tokens[vars_start_idx + 1] != "(") {
            throw runtime_error("Invalid variable format for '" + type_token + "': Missing '('. Expected V(node) or I(comp).");
        }

        if (vars_start_idx + 2 >= tokens.size()) {
            throw runtime_error("Invalid variable format: Missing node or component ID after '('.");
        }
        const string& id = tokens[vars_start_idx + 2];

        if (vars_start_idx + 3 >= tokens.size() || tokens[vars_start_idx + 3] != ")") {
            throw runtime_error("Invalid variable format: Missing ')' after ID '" + id + "'.");
        }

        printVars.push_back({type, id});
        vars_start_idx += 4;
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
        string filename = entry.path().filename().string();
        if (entry.is_regular_file() && entry.path().extension() == ".txt" && filename != "CMakeLists.txt") {
            schematicFiles.push_back(filename);
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
        throw runtime_error("Syntax error. Usage: DC <SrcName> <Start> <End> <Incr> [Var1] ...");
    }
    const string& srcName = tokens[1];
    double start = parseValue(tokens[2]);
    double end = parseValue(tokens[3]);
    double incr = parseValue(tokens[4]);

    if (incr == 0) throw runtime_error("Increment for DC sweep cannot be zero.");

    vector<PrintVariable> printVars;
    size_t vars_start_idx = 5;

    // --- START OF BUG FIX ---
    // This new loop correctly parses tokenized variables like "V", "(", "1", ")"
    while (vars_start_idx < tokens.size()) {
        const string& type_token = tokens[vars_start_idx];
        char type;
        if (type_token.length() == 1 && (toupper(type_token[0]) == 'V' || toupper(type_token[0]) == 'I')) {
            type = toupper(type_token[0]);
        } else {
            throw runtime_error("Invalid variable format: '" + type_token + "'. Expected 'V' or 'I' to start a variable specification.");
        }

        if (vars_start_idx + 1 >= tokens.size() || tokens[vars_start_idx + 1] != "(") {
            throw runtime_error("Invalid variable format for '" + type_token + "': Missing '('. Expected V(node) or I(comp).");
        }

        if (vars_start_idx + 2 >= tokens.size()) {
            throw runtime_error("Invalid variable format: Missing node or component ID after '('.");
        }
        const string& id = tokens[vars_start_idx + 2];

        if (vars_start_idx + 3 >= tokens.size() || tokens[vars_start_idx + 3] != ")") {
            throw runtime_error("Invalid variable format: Missing ')' after ID '" + id + "'.");
        }

        printVars.push_back({type, id});
        vars_start_idx += 4;
    }
    // --- END OF BUG FIX ---

    if (tokens.size() > 5 && printVars.empty()) {
        // This check might be misleading now, but we can leave it.
        // It's hard to distinguish between "no variables provided" and "invalid variable format" without more complex logic.
        cout << "Warning: No valid variables found to print." << endl;
    }

    circuit.runDCSweep(srcName, start, end, incr, printVars);
}
void Simulator::handleHelp() {
    cout << "--- Circuit Simulator Help ---" << endl;
    cout << "Available Commands:" << endl << endl;

    cout << "  netlist <filepath>" << endl;
    cout << "    - Loads a circuit description from a file." << endl;
    cout << "    - Example: netlist rlc_filter.txt" << endl << endl;

    cout << "  add <CompDefinition>" << endl;
    cout << "    - Adds a single component to the current circuit." << endl;
    cout << "    - Basic Example: add R1 1 2 1k" << endl;
    cout << "    - Dependent Sources Examples:" << endl;
    cout << "      - add E1 3 4 1 2 2.5    (VCVS: E<name> n+ n- cn+ cn- gain)" << endl;
    cout << "      - add G1 5 0 1 2 0.1    (VCCS: G<name> n+ n- cn+ cn- gain)" << endl;
    cout << "      - add H1 4 0 Vdummy 50  (CCVS: H<name> n+ n- v_ctrl gain)" << endl;
    cout << "      - add F1 5 2 Vmeas 100 (CCCS: F<name> n+ n- v_ctrl gain)" << endl << endl;



    cout << "  delete <CompName>" << endl;
    cout << "    - Deletes a component by its name." << endl;
    cout << "    - Example: delete R1" << endl << endl;

    cout << "  list [type]" << endl;
    cout << "    - Lists all components in the circuit. Can be filtered by type (R, C, V, etc.)." << endl;
    cout << "    - Example: list or list C" << endl << endl;

    cout << "  dc <Src> <Start> <End> <Incr> <Var1> ... " << endl;
    cout << "    - Performs a DC sweep analysis." << endl;
    cout << "    - Example: dc Vs 0 10 0.5 V(2)" << endl << endl;

    cout << "  run <EndTime> <TimeStep>" << endl;
    cout << "    - Runs a simple transient analysis, printing all variables." << endl;
    cout << "    - Example: run 1m 1u" << endl << endl;

    cout << "  print TRAN <Tstep> <Tstop> [<Tstart>] [<Tmaxstep>] <Var1> ..." << endl;
    cout << "    - Runs a transient analysis, printing only specified variables." << endl;
    cout << "    - Tstart and Tmaxstep are optional." << endl;
    cout << "    - Example: print TRAN 1u 10m 5m V(2)" << endl << endl;

    cout << "  nodes" << endl;
    cout << "    - Lists all unique node numbers in the circuit." << endl << endl;

    cout << "  gnd <node_number>" << endl;
    cout << "    - Connects a specified node to ground (renames it to 0)." << endl;
    cout << "    - Example: gnd 3" << endl << endl;

    cout << "  show schematics" << endl;
    cout << "    - Shows available netlist files in the current directory." << endl << endl;

    cout << "  save <filename.txt>" << endl;
    cout << "    - Saves the current manually built circuit to a netlist file." << endl << endl;


    cout << "  reset" << endl;
    cout << "    - Clears the current circuit." << endl << endl;

    cout << "  exit" << endl;
    cout << "    - Exits the simulator." << endl << endl;
    cout << "-----------------------------" << endl;
}
void Simulator::handleSave(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        throw runtime_error("Syntax: save <filepath.txt>");
    }

    string filename = tokens[1];
    // یک بررسی ساده برای اطمینان از فرمت txt
    if (filename.rfind(".txt") == string::npos || filename.rfind(".txt") != filename.length() - 4) {
        filename += ".txt";
    }

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw runtime_error("Error: Could not open file for writing: " + filename);
    }

    const auto& components = circuit.getComponents();
    if (components.empty()) {
        cout << "Circuit is empty. Nothing to save." << endl;
        return;
    }

    for (const auto& comp : components) {
        outFile << comp->toNetlistString() << endl;
    }

    outFile.close();
    cout << "Circuit successfully saved to " << filename << endl;
}