# Lua Tk module

This module provides a basic interface between Lua and Tcl/Tk. The operations of this module allow you to execute arbitrary commands in the Tcl interpreter, set and retrieve variable values in the interpreter, and invoke Lua callbacks from Tcl/Tk.

This is a fairly straightforward port of the corresponding Pure module, see <https://agraef.github.io/pure-docs/pure-tk.html>.

This version comes with a rockspec which lets you conveniently install the module using [luarocks](https://luarocks.org/).

To build and install: `sudo luarocks make`

To uninstall: `sudo luarocks remove luatk`

The module can also be installed in the usual way if you don't have luarocks, as follows:

To build and install: `make && sudo make install`

To uninstall: `sudo make uninstall`

Please check the examples folder for some sample Lua scripts showing how to utilize this module.
