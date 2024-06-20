# morebinds | A Hyprland plugin
morebinds is a hyprland plugin that allows you to set keymaps for double tapping mod keys i.e. shift, ctrl, alt, esc, super. 
## Configuration
The delay after in which a double tap will be registered is configurable and can be changed as follows in the hyprland.conf
```
plugin {
  morebinds {
    # the delay is in milliseconds
    shift_delay = 200
    super_delay = 400
    alt_delay = 700
    ctrl_delay = 200
    esc_delay = 200
  }
}
```
to set the command to be executed for single press or double press
```
plugin {
  morebinds {
    shift_single = firefox
    shift_double = kitty
  }
}
```
it is recommended not to use the default way to set keybind for single press of mod keys as it might be buggy with this plugin
