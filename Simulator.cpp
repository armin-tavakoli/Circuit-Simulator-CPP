#include "Simulator.h"
#include <stdexcept>
#include <algorithm>
#include <set>

/**
 * @brief حلقه اصلی برنامه را اجرا می‌کند که منتظر دستورات کاربر می‌ماند.
 */
void Simulator::run() {
    string command;
    cout << "Welcome to the Circuit Simulator!" << endl;
    cout << "Enter commands ('netlist path/to/file.txt', 'print TRAN...', 'exit')." << endl;

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

/**
 * @brief یک دستور ورودی را تجزیه و تابع مربوط به آن را فراخوانی می‌کند.
 */
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
    else if (cmd == "rename") handleRenameNode(tokens);
    else if (cmd == "print") handlePrint(tokens);
    else throw runtime_error("Unknown command '" + tokens[0] + "'");
}


/**
 * @brief تابع کمکی برای افزودن یک المان از روی آرگومان‌های تجزیه شده.
 */
void Simulator::addComponentFromTokens(const vector<string>& args) {
    if (args.empty()) return;

    const string& name = args[0];
    if (circuit.hasComponent(name)) {
        throw runtime_error("Component '" + name + "' already exists.");
    }

    char compType = toupper(name[0]);

    // پردازش بر اساس نوع المان
    switch (compType) {
        case 'R':
        case 'C':
        case 'L': {
            if (args.size() != 4) {
                throw runtime_error("R/C/L definition requires exactly 4 arguments.");
            }
            int n1 = stoi(args[1]);
            int n2 = stoi(args[2]);
            double value = parseValue(args[3]);
            if (value <= 0) {
                throw runtime_error("Value for R, C, or L must be positive.");
            }

            if (compType == 'R') circuit.addComponent(make_unique<Resistor>(name, n1, n2, value));
            else if (compType == 'C') circuit.addComponent(make_unique<Capacitor>(name, n1, n2, value));
            else if (compType == 'L') circuit.addComponent(make_unique<Inductor>(name, n1, n2, value));
            break;
        }
        case 'V': {
            if (args.size() < 4) {
                throw runtime_error("V source definition requires at least 4 arguments.");
            }
            int n1 = stoi(args[1]);
            int n2 = stoi(args[2]);

            // بررسی دقیق برای منبع سینوسی
            if (args.size() >= 8 && args[3] == "SIN") {
                if (args[4] != "(" || args[7].back() != ')') {
                    throw runtime_error("Syntax for SIN: SIN ( Voff Vamp Freq )");
                }
                double v_off = parseValue(args[5]);
                double v_amp = parseValue(args[6]);
                string freq_str = args[7];
                freq_str.pop_back();
                double freq = parseValue(freq_str);
                circuit.addComponent(make_unique<SinusoidalVoltageSource>(name, n1, n2, v_off, v_amp, freq));
            }
                // بررسی برای منبع DC
            else if (args.size() == 4) {
                double value = parseValue(args[3]);
                circuit.addComponent(make_unique<VoltageSource>(name, n1, n2, value));
            }
            else {
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

    if (existingNodes.find(newNode) != existingNodes.end()) {
        throw runtime_error("Node name <" + to_string(newNode) + "> already exists");
    }

    circuit.renameNode(oldNode, newNode);
    cout << "SUCCESS: Node renamed from <" << oldNode << "> to <" << newNode << ">" << endl;
}

/**
 * @brief دستور 'run' را پردازش می‌کند. این دستور یک میان‌بر برای چاپ تمام خروجی‌ها است.
 */
void Simulator::handleRun(const vector<string>& tokens) {
    if (tokens.size() != 3) throw runtime_error("Syntax error. Usage: run <EndTime> <TimeStep>");
    double endTime = parseValue(tokens[1]);
    double timeStep = parseValue(tokens[2]);
    // فراخوانی تحلیل با لیست خالی برای چاپ تمام متغیرها
    circuit.runTransientAnalysis(endTime, timeStep, {});
}

/**
 * @brief دستور 'print' را برای تحلیل و چاپ متغیرهای خاص پردازش می‌کند.
 */
void Simulator::handlePrint(const vector<string>& tokens) {
    if (tokens.size() < 4 || tokens[1] != "TRAN") {
        throw runtime_error("Syntax error. Usage: print TRAN <Tstep> <Tstop> <Var1> <Var2> ...");
    }

    double tStep = parseValue(tokens[2]);
    double tStop = parseValue(tokens[3]);

    vector<PrintVariable> printVars;
    // شروع از آرگومان چهارم برای استخراج متغیرها
    for (size_t i = 4; i < tokens.size(); ++i) {
        const string& varStr = tokens[i];
        if (varStr.length() < 4) throw runtime_error("Invalid variable format: " + varStr);

        char type = toupper(varStr[0]);
        string id = varStr.substr(2, varStr.length() - 3); // استخراج id از بین V(id) یا I(id)

        if ((type == 'V' || type == 'I') && varStr[1] == '(' && varStr.back() == ')') {
            printVars.push_back({type, id});
        } else {
            throw runtime_error("Invalid variable format: " + varStr + ". Expected V(node) or I(comp).");
        }
    }

    // اگر هیچ متغیر معتبری پیدا نشد، خطا می‌دهیم
    if (tokens.size() > 4 && printVars.empty()){
        cout << "Warning: No valid variables found to print." << endl;
    }

    circuit.runTransientAnalysis(tStop, tStep, printVars);
}
