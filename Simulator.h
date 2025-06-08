#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "Circuit.h"
#include "Component.h"
#include "ValueParser.h"
#include "PrintRequest.h"
#include "DiodeModel.h"

using namespace std;

class Simulator {
public:
    Simulator();
    void run();

private:
    void processCommand(const string& command);

    void handleAdd(const vector<string>& tokens);
    void handleDelete(const vector<string>& tokens);
    void handleRun(const vector<string>& tokens);
    void handleList(const vector<string>& tokens);
    void handleNodes();
    void handleNetlist(const vector<string>& tokens);
    void handleReset();
    void handleRenameNode(const vector<string>& tokens);
    void handlePrint(const vector<string>& tokens);
    void handleGnd(const vector<string>& tokens);
    void handleShow(const vector<string>& tokens);
    void handleDC(const vector<string>& tokens);
    void handleHelp();

    void addComponentFromTokens(const vector<string>& args);

    map<string, DiodeModel> diodeModels;
    void setupDefaultModels();

    Circuit circuit;
};

#endif //SIMULATOR_H
