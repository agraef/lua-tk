# Lua Tk module

This module provides a basic interface between Lua and Tcl/Tk. The operations of this module allow you to execute arbitrary commands in the Tcl interpreter, set and retrieve variable values in the interpreter, and invoke Lua callbacks from Tcl/Tk. It should thus provide all the machinery required to code fairly sophisticated Tcl/Tk applications in Lua.

This is a fairly straightforward port of the corresponding Pure module, see <https://agraef.github.io/pure-docs/pure-tk.html>. It implements most of the basic operations of that module in Lua, but lacks some of the more advanced functionality. A brief synopsis of the available functions can be found below. Please also check the examples folder for some sample Lua scripts showing how to utilize this module.

## Installation

You'll need Lua, of course (Lua 5.2 or later should be fine, 5.4 has been tested), and Tcl/Tk (8.6 has been tested). (A few of the examples may also need additional Tcl modules, such as Gnocl and VTK, please check the Tcl sources for details.)

This version comes with a rockspec which lets you conveniently install the module using [luarocks](https://luarocks.org/).

To build and install: `sudo luarocks install lua-tk`

To uninstall: `sudo luarocks remove lua-tk`

The module can also be installed in the usual way if you don't have luarocks, as follows:

To build and install: `make && sudo make install`

To uninstall: `sudo make uninstall`

## Synopsis

To load this module: `tk = require("tk")`

`tk.eval(s)`: Executes the command `s` (a string) in the Tcl interpreter. An instance of the interpreter will be created if none is currently running.

In the Tcl interpreter, you can call back to Lua using the Tcl `lua` command which is invoked as `lua function args ...`, where `function` is the name of a Lua function (which must be global), and `args` are the (string) arguments the function is to be invoked with.

`tk.quit()`: Stop the Tcl interpreter.

`tk.main()`: Call the Tk main loop which keeps processing events until either `tk.quit()` is called or the main window of the application is closed.

`tk.ready()`: Checks whether the Tcl interpreter is still up and running, after processing any pending events in the interpreter. This provides an alternative to `tk.main()` which enables you to code your own custom main loops in Lua.

`tk.set(var, val)`, `tk.unset(var)`, `tk.get(var)`: Set and get global Tcl variables.

`tk.join(xs)`, `tk.split(s)`: Convert between Lua and Tcl lists.

## Notes

There are various Tcl add-on packages which can be useful for Lua programmers. In particular, you may want to look at [Gnocl](https://dr-baum.net/gnocl/), which lets you create nice-looking GTK+ GUIs in Tcl, and [VTK](https://www.vtk.org/), a powerful scientific visualization package which has Tcl bindings. As I find these so useful, and ready-made packages can be hard to find, I've provided some suitable Arch and Debian/Ubuntu packages for your convenience. You can find the former in the Arch User Repositories ([Gnocl](https://aur.archlinux.org/packages?O=0&K=gnocl), [VTK](https://aur.archlinux.org/packages/vtk6-legacy)), and the latter at the Open Build Service ([Gnocl](https://build.opensuse.org/project/show/home:aggraef:gnocl), [VTK](https://build.opensuse.org/project/show/home:aggraef:vtk6)).
