#include "../plugin_sdk/plugin_sdk.hpp"

PLUGIN_NAME("Peko");
SUPPORTED_CHAMPIONS(champion_id::LeeSin);

#include "leesin.h"


PLUGIN_API bool on_sdk_load(plugin_sdk_core* plugin_sdk_good)
{

    DECLARE_GLOBALS(plugin_sdk_good);


    switch (myhero->get_champion())
    {
    case champion_id::LeeSin:

       leesin::load();
        break;
    default:
        break;
            console->print("Champion %s is not supported!", myhero->get_model_cstr());
        return false;
    }


    return true;
}


PLUGIN_API void on_sdk_unload()
{
    switch (myhero->get_champion())
    {
    case champion_id::LeeSin:

        leesin::unload();
        break;
    default:
        break;
    }
}