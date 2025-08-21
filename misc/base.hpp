#ifndef BASE
#define BASE
#include <lldb/API/LLDB.h>
#include <iostream>
#include <any>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cxxabi.h>
#include <memory>
#include <vector>
#include <sstream>
#include <cctype>
#include <variant>
#include <array>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <functional>
#include <iomanip>
#include <map>
#include <typeindex>

#ifndef _WIN32
#include <signal.h>
#endif

bool is_debug = false;

class Logger {
private:
    std::ofstream logFile;
    std::string filename;


    std::string currentDateTime() {
        std::time_t now = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buf;
    }

public:
    Logger(std::string file) : filename(file) {
        if (is_debug){
            logFile.open(filename, std::ios::app); 
            if (!logFile.is_open()) {
                throw std::runtime_error("Unable to open log file: " + filename);
            }
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(std::string message,std::string from="*") {
        if (is_debug){
            if (logFile.is_open()) {
                logFile << "[" << currentDateTime() << "] (" << from << ") " << message << std::endl;
            }
        }
    }
};

std::shared_ptr<Logger> logger;

int64_t bp_ids = 1;

uint64_t hextob10(std::string hexStr){
    if (hexStr.find("0x") == 0 || hexStr.find("0X") == 0) {
        hexStr = hexStr.substr(2);
    }

    uint64_t value = std::stoull(hexStr, nullptr, 16);
    return value;
}

std::vector<std::string> split(const std::string& str) {
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    std::string word;
    while (iss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

std::string repeat(const std::string& str, int times) {
    if (times <= 0) {
        return "";
    }
    std::string result;
    result.reserve(str.length() * times); 
    for (int i = 0; i < times; ++i) {
        result += str;
    }
    return result;
}

std::string replace(std::string s, std::string from, std::string to)
{
    if(!from.empty())
        for(std::string::size_type pos = 0; (pos = s.find(from, pos) + 1); pos += to.size())
            s.replace(--pos, from.size(), to);
    return s;
}

bool in(std::string inp, std::vector<std::string> op){
    for (int i = 0; i < op.size() ; i ++){
    if (inp == op[i]){
        return true;
            //Do Stuff
    }}
    return false;
}

template <typename T>
bool inT(T inp, std::vector<T> op){
    for (int i = 0; i < op.size() ; i ++){
    if (inp == op[i]){
        return true;
            //Do Stuff
    }}
    return false;
}

class ArgParser {
    public:
    ArgParser(char** argv,int argc): argv(argv), argc(argc){
    
    }
    bool has(std::string value){
        for (int i = 0; i != argc; i++){
            if (value == std::string(argv[i])){
                return true;
            }
        }
        return false;
    }
    void parse(){
        for (int i = 0; i != argc; i++){
            auto key = std::string(argv[i]);
            if(in(key,sflags)){

                std::string val;
                if (argv[i+1][0] == '\''){
                    while (argv[i+1][std::string(argv[i+1]).size()-1] != '\'' ){
                        val += std::string(argv[i+1]) + " ";
                        i++;

                    }
                    val += std::string(argv[i+1]);
                    val = replace(val,"'","\"");
                } else {
                    val = argv[i+1];
                }
                if (has("-sflags") || has("-flags")){
                    std::cout << key << " : " << val << std::endl;
                }
                if (values.find(key) == values.end()){
                    values[key] = std::vector<std::string>{std::string(val)};
                } else {
                    values[key].push_back(val);
                }

            i++;
            } else {
                if (has("-flags")){
                    std::cout << std::string(argv[i]) << std::endl;
                }
            }
            
        }

    }

    std::unordered_map<std::string,std::vector<std::string>> values;
    std::vector<std::string> sflags{"port","panel"};
    char** argv;
    int argc;
    private:
};

ArgParser args(nullptr,0);
#endif