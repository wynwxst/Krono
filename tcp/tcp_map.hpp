#include "tcp/client.hpp"
#include "tcp/server.hpp"
#include "misc/base.hpp"
#include "process/proc.hpp"
#ifndef TCPMAP
#define TCPMAP


std::vector<std::string> _parseArgs(std::vector<std::string> vec){

    for (int i = 0; i < vec.size();i++){
        if (vec[i].substr(0,2) == "0x"){
            vec[i] = std::to_string(hextob10(vec[i]));
        }

    }
    return vec;
}

std::vector<std::string> parseArgs(std::string args){
    auto argvec = split(args);
    return _parseArgs(argvec);
}


std::string ping(std::string _){
    if (_ == "tennis"){
        return "table tennis";
    }
    return "pong!";
}

std::string whoami(std::string _){
    return "Krono debugger TCP server - 2025";
}

std::string SetTarget(std::string _){

    auto args = parseArgs(_);
    if (args.size() == 0){
        return "Invalid syntax";
    }
    server->targetset = true;
    std::string binary = args[0];
    activeProc = proc.setTarget(binary);
    return "Set target.\n";
    

}

std::string Launch(std::string _){
    activeProc = proc.Launch(activeProc);
    server->launched = true;
    proc.StayExec(activeProc.activeProc,activeProc.target);
    return "Process started with pid: " + std::to_string(activeProc.activeProc->GetProcessID()) + "\n";

}

std::string Kill(std::string _){
    proc.End(activeProc.activeProc);
    return "Process ended";

}

std::string Detach(std::string _){
    proc.Detach(activeProc.activeProc);
    return "Process detached";

}

std::string Continue(std::string _){


    proc.Continue(activeProc.activeProc);
    return "Continuing";

}


std::string List(std::string _){
    auto args = parseArgs(_);
    if (args[0] == "modules" || args[0] == "mods"){
        return proc.listModules(activeProc);
    }
    else if (args[0] == "breakpoints" || args[0] == "bps"){
        return proc.listBreakPoints(activeProc);
    }
    else if (args[0] == "functions" || args[0] == "fns"){
        std::string arg_1;
        if (args.size() > 1){
            arg_1 = args[1];
        }
        return proc.listFunctions(activeProc,arg_1);
    }
    else if (args[0] == "fixfns" || args[0] == "ff"){
        std::string arg_1;
        if (args.size() > 1){
            arg_1 = args[1];
        }
        proc.setFuncFixed(proc.listFunctions(activeProc,arg_1));
        return "Affixed!";
    }else if (args[0] == "getfix"){
        return proc.getFuncFixed();
    
    }
    else {
        return "List: No/Wrong argument specified";
    }

    

}

std::string Disasm(std::string _){
    auto args = parseArgs(_);
    if (args[0] == "pc" || args[0] == "current"){
        return proc.getDisasmPC(activeProc);
    }
    else if (args[0] == "function" || args[0] == "fn"){
        if (args.size() < 2){
            return "Disasm Function: No function name specified";
        }
        return proc.getDisasmFunction(activeProc,args[1]);

    }
    else if (args[0] == "insts" || args[0] == "i"){
        if (args.size() < 3){
            return "Disasm Insts: No start address/amount of instructions specified";
        }
        return proc.getDisasmInsts(activeProc,std::stoull(args[1],nullptr,10),std::stoul(args[2],nullptr,10));

    }
    else if (args[0] == "*"){
        if (args.size() < 3){
            return "Disasm Address: No start/end address specified";
        }
        return proc.getDisasm(activeProc,std::stoull(args[1],nullptr,10),std::stoull(args[2],nullptr,10));

    }
    else if (args[0] == "fix"){
        if (args.size() < 2){
            return "Disasm Fix: Nothing to fix";
        }
        std::string argstr;
        for (int i = 1; i < args.size(); i++){
            argstr += args[i] + (i == args.size()-1 ? "" : " ");
        }

        proc.setDisasmFixed(Disasm(argstr));
        return "Affixed!";

    }
    else if (args[0] == "getfix"){
        return proc.getDisasmFixed();

    }

    else {
        return "Disasm: No/Wrong argument specified";
    }

    

}

std::string Break(std::string _){
    auto args = parseArgs(_);
    if (args[0] == "*"){
        std::string name = "<unnamed>";
        if (args.size() < 2){
            return "Break: Missing address";
        }
        if (args.size() >= 3){
            name = args[2];

        }
        proc.SetBreakOnAddress(activeProc,std::stoull(args[1],nullptr),name);

    } else {
        if (args.size() < 1){
            return "Break: Missing name";
        }
        proc.SetBreakOnName(activeProc,args[0]);

    }
    return "Breakpoint set";

    

}

std::string View(std::string _){
    auto args = parseArgs(_);
    if (args[0] == "stack"){
        return proc.viewStack(activeProc);
    }
    else if (args[0] == "register" || args[0] == "reg"){
        if (args.size() < 2){
            return "View Register: no register provided";
        }
        return proc.viewReg(activeProc,args[1]);
    }
    else if (args[0] == "registers" || args[0] == "regs"){
        if (args.size() >= 2){
            if (args[1] == "all"){
                return proc.viewRegs(activeProc);
            } else {
                // for set specific here
            }
        }
        return proc.viewGPRs(activeProc);
    }
    return "View: invalid view";
}

std::string Write(std::string _){
    auto args = parseArgs(_);
    if (args.size() < 3){
        return "Write: missing destination type/destination/value";
    }   
    auto destty = args[0];
    auto dest = args[1];
    std::string write_val;
    for (int i = 2; i < args.size();i++){
        write_val += args[i] + (i != args.size()-1 ? " " : "");


    }
    
    if (destty == "reg" || destty == "register"){
        return proc.writeRegs(activeProc,dest,write_val);
    } else if (destty == "mem" || destty == "memory"){
        if (dest == "*"){
            if (args.size() < 4){
                return "Write Memory Address: destination/value addresses";
            }
            return proc.writeMemAddr(activeProc,std::stoull(args[2],nullptr),std::stoull(args[3],nullptr));

        }
        return proc.writeMem(activeProc,std::stoull(dest,nullptr),write_val);
    }

    return "Write: invalid write destination";
}

std::string Read(std::string _){
    auto args = parseArgs(_);
    if (args.size() < 3){
        return "Read: missing destination type/destination/size";
    }   
    auto destty = args[0];
    auto dest = args[1];
    
    if (destty == "mem" || destty == "memory"){
        return proc.readMem(activeProc,std::stoull(dest,nullptr),std::stoull(args[2],nullptr));
    }

    return "Read: invalid read destination";
}

std::string Backtrace(std::string _){
    return proc.getBackTrace();

    

}

std::string Restart(std::string _){
    proc.Stop(activeProc.activeProc);
    Detach("");
    Launch("");
    
}



std::string setinput(std::string _){
    *proc.input_buffer = _;
    
    return "received";

    

}

std::string free_input(std::string _){
    proc.can_proceed = true;
    
    return "received";

    

}

std::string launch_flags(std::string _){
    auto args = parseArgs(_);
    for (auto arg: args){
        proc.launch_flags.push_back(arg);
    }
    
    return "set launch flags";

    

}


void register_endpoints(std::shared_ptr<TCPServer> server){
    server->add_map("ping",(void*)ping);
    server->add_map("whoami",(void*)whoami);
    server->add_map("target",(void*)SetTarget);
    server->add_map("launch",(void*)Launch);
    server->add_map("kill",(void*)Kill);
    server->add_map("detach",(void*)Detach);
    server->add_map("restart",(void*)Restart);
    server->add_map("continue",(void*)Continue);;
    server->add_map("list",(void*)List);
    server->add_map("disasm",(void*)Disasm);
    server->add_map("break",(void*)Break);
    server->add_map("view",(void*)View);
    server->add_map("write",(void*)Write);
    server->add_map("read",(void*)Read);
    server->add_map("backtrace",(void*)Backtrace);
    server->add_map("launchflags",(void*)launch_flags);

    // programatically called
    server->add_map("setinput",(void*)setinput);
    server->add_map("free_input",(void*)free_input);


}




#endif