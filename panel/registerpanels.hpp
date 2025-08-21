#include "misc/base.hpp"
#include "panel/paneldef.hpp"
#ifndef REGPAN
#define REGPAN
void register_panels(PanelManager* pm, int port,std::string panel_group){


    if (panel_group == "list"){
        auto bp = std::make_shared<BreakpointPanel>(port);
        auto mp = std::make_shared<ModulePanel>(port);
        auto fp = std::make_shared<FunctionsPanel>(port);

        pm->add_panel({
            bp->name,
            bp,
            true
        });

        pm->add_panel({
            mp->name,
            mp,
            true
        });

        pm->add_panel({
            fp->name,
            fp,
            true
        });
    } else if (panel_group == "disasm"){
        auto dPCp = std::make_shared<DisasmPCPanel>(port);
        auto dfp = std::make_shared<DisasmFixedPanel>(port);
        pm->add_panel({
            dPCp->name,
            dPCp,
            true
        });
        pm->add_panel({
            dfp->name,
            dfp,
            true
        });
    }
    else if (panel_group == "view"){
        auto sp = std::make_shared<StackPanel>(port);
        auto rp = std::make_shared<RegPanel>(port);
        pm->add_panel({
            sp->name,
            sp,
            true
        });
        pm->add_panel({
            rp->name,
            rp,
            true
        });
    }



}
#endif