tk = require("tk")

-- Initialize (create the application's widgets).

function init_app()
   -- Start a new interpreter.
   tk.quit()
   -- Unmap the main window (will be shown when all widgets have been created,
   -- to reduce flickering).
   tk.eval("wm withdraw .")
   -- Set the window title.
   tk.eval("wm title . \"Lua-Tk Example\"")
   -- Title label.
   tk.eval("label .title -font 8x13bold -text \"A Simple Lua-Tk Application\"")
   tk.eval([[label .descr -text "Type in a Lua expression and click Eval to evaluate.
Click Quit to quit."]])
   -- Main area: input expression and output result with labels.
   tk.eval("frame .f")
   tk.eval("label .f.expr_l -text \"Expression:\"")
   tk.eval("entry .f.expr_e -textvariable expr_val -width 40")
   tk.eval("label .f.result_l -text \"Result:\"")
   tk.eval("entry .f.result_e -textvariable result_val -width 40")
   -- Buttons.
   tk.eval("frame .buttons")
   tk.eval("button .buttons.eval -text Eval -command { lua eval_cb }")
   tk.eval("button .buttons.quit -text Quit -command { lua quit_cb }")
   -- Pack widgets into the main window.
   -- Title and main frame.
   tk.eval("pack .title .descr")
   tk.eval("pack .f -fill x")
   -- Grid layout of the main frame.
   tk.eval("grid config .f.expr_l -column 0 -row 0 -sticky w")
   tk.eval("grid config .f.expr_e -column 1 -row 0 -sticky we")
   tk.eval("grid config .f.result_l -column 0 -row 1 -sticky w")
   tk.eval("grid config .f.result_e -column 1 -row 1 -sticky we")
   -- Make entry widgets expand horizontally to fill main window.
   tk.eval("grid columnconfigure .f 0 -weight 0")
   tk.eval("grid columnconfigure .f 1 -weight 1")
   -- Buttons.
   tk.eval("pack .buttons")
   tk.eval("pack .buttons.eval .buttons.quit -side left")
   -- Bind Return key in expression entry to the evaluation command.
   tk.eval("bind .f.expr_e <Return> { lua eval_cb }")
   -- Show the main window.
   tk.eval("wm deiconify .")
end

-- Callback definitions.

function eval_cb()
   res, val = pcall(load("return " .. (tk.get("expr_val"))))
   res = res and tostring(val) or "*** syntax error ***"
   tk.set("result_val", res)
end

function quit_cb()
   tk.quit()
end

-- The main program: Evaluate Pure expressions in a Tk window.

function main()
   init_app()
   tk.main()
end

main()
