# kbinput

## Overview

libkbinput is an event based terminal keyboard input library,
with support for the [kitty keyboard protocol](https://sw.kovidgoyal.net/kitty/keyboard-protocol).

## Installation

1. Clone the repository
```bash
git clone https://github.com/ilmu23/kbinput
cd kbinput
```
2. Make and install the library
```bash
make && sudo make install
```
3. (optional) Install development headers
```bash
sudo make install-headers
```

By default all files are installed to /usr. This can be
overridden by setting the INSTALL_PATH variable to
the desired install location.
```bash
make install install-headers INSTALL_PATH=~/.local
```

## Usage

kbinput is based on creating listener instances, which
are then configured to listen for certain keys to be pressed.

<br>

Here is a small example of what that might look like.

```C
#include <kbinput/kbinput.h>

void    *move_up(void *arg);
void    *move_down(void *arg);

int main(void) {
    kbinput_listener_id listener;
    const kbinput_key   *event;
    int                 rv;

    kbinput_init();
    if (kbinput_get_input_protocol == KB_INPUT_PROTOCOL_ERROR)
        return 1;
    listener = kbinput_new_listener();
    if (listener < 0)
        return 1;
    switch (kbinput_get_input_protocol()) {
        case KB_INPUT_PROTOCOL_KITTY:
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_UP, 0, KB_EVENT_PRESS, move_up)))
                return 1;
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_DOWN, 0, KB_EVENT_PRESS, move_down)))
                return 1;
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_UP, 0, KB_EVENT_RELEASE, move_up)))
                return 1;
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_DOWN, 0, KB_EVENT_RELEASE, move_down)))
                return 1;
            break ;
        case KB_INPUT_PROTOCOL_LEGACY:
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_LEGACY_UP, 0, KB_EVENT_PRESS, move_up)))
                return 1;
            if (!kbinput_add_listener(listener, kbinput_key(KB_KEY_LEGACY_DOWN, 0, KB_EVENT_PRESS, move_down)))
                return 1;
            break ;
        case KB_INPUT_PROTOCOL_ERROR:
            return 1;
    }
    rv = 0;
    do {
        event = kbinput_listen(listener);
        if (!event) {
            rv = 1;
            break ;
        }
        if (!event->fn((void *)event))
            break ;
    } while (event);
    kbinput_cleanup();
    return rv;
}
```

We start off by calling `kbinput_init` which initializes some internal
data structures and terminal settings in addition to determining if
the terminal supports the kitty keyboard protocol.

After the init is done we create a new listener instance with `kbinput_new_listener`,
which returns us a new listener id.

Using this new id, we configure the listener to listen for the user pressing
either the up or down arrow key by calling `kbinput_add_listener`. We are using
the `kbinput_key` convenience macro for specifying the key to listen for,
the modifiers we want to be present, the type of event to listen for and
the function to register for that event.

After we're done setting up all the events we want to listen for,
we start listening for them with `kbinput_listen`.

Lastly, we make sure to call `kbinput_cleanup` to free all internal
resources, as well as restoring the terminal settings. This is especially
important when using a terminal that supports the kitty keyboard protocol,
since most shells won't understand the escape sequences the terminal will
be sending when pressing keys if the terminal settings aren't restored.

For a more complex example with multiple listener instances, see the
source code of [netpong](https://github.com/ilmu23/netpong_client).
