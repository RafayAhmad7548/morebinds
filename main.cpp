#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/wlr/types/wlr_keyboard.h>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/devices/Keyboard.hpp>
#include <hyprland/src/managers/KeybindManager.hpp>

#include <chrono>
#include <cstring>
#include "keyinfo.cpp"

inline HANDLE PHANDLE = nullptr;

bool once = true;
std::function<void(std::string)> dispatchExec;

long long current_time(){
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long long current_time_milliseconds = value.count();
    return current_time_milliseconds;
}

struct keyboard_listener{
    wlr_keyboard* keyboard;
    wl_listener listener;
};
struct keyinfo{
    bool pressedOnce;
    long long time;
    Hyprlang::INT* const* delay;
    Hyprlang::STRING const* cmd_single;
    Hyprlang::STRING const* cmd_double;
    keyinfo() : pressedOnce(false), time(0), delay(nullptr), cmd_single(nullptr), cmd_double(nullptr){}
};

keyinfo esc_info, shift_info, ctrl_info, alt_info, super_info;

void thread_func(keyinfo& info){
    std::this_thread::sleep_for(std::chrono::milliseconds(**info.delay));
    if(info.pressedOnce){
        std::cout<<"single pressed\n";
        if(strlen(*info.cmd_single) != 0) dispatchExec(*info.cmd_single);
        info.pressedOnce = false;
    }
}

void detect_double_press(keyinfo& info){
    if(!info.pressedOnce){
        info.pressedOnce = true;
        info.time = current_time();
        std::thread thr(thread_func, std::ref(info));
        thr.detach();
    }
    else{
        info.pressedOnce = false;
        if(current_time() - info.time <= **info.delay){
            std::cout<<"double pressed\n";
            if(strlen(*info.cmd_double) != 0) dispatchExec(*info.cmd_double);
        }
    }
}

void handleKey(struct wl_listener *listener, void *data){
        
    auto event = static_cast<wlr_keyboard_key_event*>(data);

    if(event->state == WL_KEYBOARD_KEY_STATE_PRESSED){
        keyboard_listener* kbd_listener = wl_container_of(listener, kbd_listener, listener);
        wlr_keyboard* keyboard = kbd_listener->keyboard;
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(keyboard->xkb_state, event->keycode);

        // escape 109
        // l shift 103
        // l control 121
        // l alt 98
        // l super 65469

        switch(keysym){
            case 109: detect_double_press(esc_info); break;
            case 103: detect_double_press(shift_info); break;
            case 121: detect_double_press(ctrl_info); break;
            case 98: detect_double_press(alt_info); break;
            case 65469: detect_double_press(super_info); break;
        }
        
    }
}

void setupKeyListeners(wlr_keyboard* keyboard){
    keyboard_listener* kbd_listener = new keyboard_listener;
    kbd_listener->keyboard = keyboard;
    kbd_listener->listener.notify = handleKey;
    wl_signal_add(&keyboard->events.key, &kbd_listener->listener);
}

APICALL EXPORT std::string PLUGIN_API_VERSION(){
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle){
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    // ALWAYS add this to your plugins. It will prevent random crashes coming from
    // mismatched header versions.
    if(HASH != GIT_COMMIT_HASH){
        HyprlandAPI::addNotification(PHANDLE, "[morebinds] Mismatched headers! Can't proceed.", CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[morebinds] Version mismatch");
    }

    dispatchExec = g_pKeybindManager->m_mDispatchers.at("exec");

    wlr_keyboard* keyboard = g_pSeatManager->keyboard->wlr();
    setupKeyListeners(keyboard);

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:esc_delay", Hyprlang::INT{200});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:shift_delay", Hyprlang::INT{200});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:ctrl_delay", Hyprlang::INT{200});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:alt_delay", Hyprlang::INT{200});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:super_delay", Hyprlang::INT{200});

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:esc_single", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:shift_single", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:ctrl_single", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:alt_single", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:super_single", Hyprlang::STRING{""});

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:esc_double", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:shift_double", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:ctrl_double", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:alt_double", Hyprlang::STRING{""});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:morebinds:super_double", Hyprlang::STRING{""});

    esc_info.delay = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:esc_delay")->getDataStaticPtr();
    shift_info.delay = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:shift_delay")->getDataStaticPtr();
    ctrl_info.delay = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:ctrl_delay")->getDataStaticPtr();
    alt_info.delay = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:alt_delay")->getDataStaticPtr();
    super_info.delay = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:super_delay")->getDataStaticPtr();

    esc_info.cmd_single = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:esc_single")->getDataStaticPtr();
    shift_info.cmd_single = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:shift_single")->getDataStaticPtr();
    ctrl_info.cmd_single = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:ctrl_single")->getDataStaticPtr();
    alt_info.cmd_single = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:alt_single")->getDataStaticPtr();
    super_info.cmd_single = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:super_single")->getDataStaticPtr();

    esc_info.cmd_double = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:esc_double")->getDataStaticPtr();
    shift_info.cmd_double = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:shift_double")->getDataStaticPtr();
    ctrl_info.cmd_double = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:ctrl_double")->getDataStaticPtr();
    alt_info.cmd_double = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:alt_double")->getDataStaticPtr();
    super_info.cmd_double = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:morebinds:super_double")->getDataStaticPtr();

    HyprlandAPI::reloadConfig();

    return {"morebinds", "An amazing plugin that is going to change the world!", "Me", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT(){}