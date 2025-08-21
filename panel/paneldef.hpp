#include "misc/base.hpp"
#include "panel/panelutils.hpp"
#include "tcp/client.hpp"
#ifndef DEMOPANEL
#define DEMOPANEL



class ClientPanel {
public:
    std::shared_ptr<bool> update;
    std::shared_ptr<TCPClient> client;
    std::string name;
    std::vector<std::string> preload;
    Logger* log;

    ClientPanel() {}
    
    void init(int port = 8080) {
        
        //log = new Logger(name + ".log");

        update = std::make_shared<bool>(false);
        client = std::make_shared<TCPClient>("127.0.0.1", port);
        

        // Try connecting to server with retries
        int attempts = 5;
        while (attempts > 0) {
            try {
                client->connectToServer();
                break;
            } catch (const std::runtime_error& e) {
                std::cerr << "Connect failed, retrying...\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                attempts--;
            }
        }


    }
    bool checkUpdate(){
        std::string upres = client->sendNrecv("pendingupdate");
        if (upres == "yes"){
            *update = true;
        } else if (upres == "specific"){
            std::string upresty = client->receive();
            if (upresty == name){
                *update = true;
                client->sendMessage("yes");
                return true;
            } else {
                client->sendMessage("no");
            }

        }
        return false;
    }

    void deinit() {
        if (client) client->disconnect();
    }

    virtual std::vector<std::string> draw() { return {}; }






};


class BreakpointPanel : public ClientPanel {
    public:
    BreakpointPanel(int port=8080){
        name = "Breakpoints";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || (*update)){
            
            auto content = client->sendCommand("list","breakpoints");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            *update = false;
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
        }

        return preload;
    }
};


class ModulePanel : public ClientPanel {
    public:
    bool done = false;
    ModulePanel(int port=8080){
        name = "Modules";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        if (!done){
            
            auto content = client->sendCommand("list","modules");
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
            done = true;
        }

        return preload;
    }
};

class FunctionsPanel : public ClientPanel {
    public:
    FunctionsPanel(int port=8080){
        name = "Functions";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || *update){
            auto content = client->sendCommand("list","getfix");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
        }

        return preload;
    }
};

class DisasmPCPanel : public ClientPanel {
    public:
    DisasmPCPanel(int port=8080){
        name = "Disassembly (Current)";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || (*update)){
            
            auto content = client->sendCommand("disasm","current");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            *update = false;
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
        }

        return preload;
    }
};

class DisasmFixedPanel : public ClientPanel {
    public:
    DisasmFixedPanel(int port=8080){
        name = "Disassembly (Fixed)";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || (*update)){
            
            auto content = client->sendCommand("disasm","getfix");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            *update = false;
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
        }

        return preload;
    }
};

class StackPanel : public ClientPanel {
    public:
    StackPanel(int port=8080){
        name = "Stack View";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || (*update)){
            
            auto content = client->sendCommand("view","stack");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            *update = false;
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
            if (preload.size() == 0){
                preload.push_back("Received nothing.");
            }
        }

        return preload;
    }
};

class RegPanel : public ClientPanel {
    public:
    RegPanel(int port=8080){
        name = "Register View";
        init(port);
    }
    std::vector<std::string> draw() override {
        if (!client) throw std::runtime_error("TCPClient not initialized");
        bool is_specific = checkUpdate();
        if (preload.empty() || (*update)){
            
            auto content = client->sendCommand("view","registers");
            if (!is_specific && !preload.empty()) client->sendCommand("updated");
            *update = false;
            std::stringstream ss(content);
            std::string line;
            preload.clear();

            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    preload.push_back(line);
                }
            }
            if (preload.size() == 0){
                preload.push_back("Received nothing.");
            }
        }

        return preload;
    }
};
#endif