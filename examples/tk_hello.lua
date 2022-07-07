tk = require("tk")

t = "Hello, world!"

-- plain Tcl callback
tk.eval(string.format("button .b -text %q -command {puts {tcl: %q}}; pack .b", t, t))
-- this does the same using a Lua callback
tk.eval(string.format("button .b2 -text %q -command {lua print {lua: %q}}; pack .b2", t, t))

-- check that the Tcl interpreter is still up and process events
tk.ready()

-- simple main application loop
tk.main()

-- we get here only after the main window is closed
print("done")
