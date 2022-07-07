
--[[ This is a simple example which shows how to employ the Tcl/Tk bindings of
   VTK (the "visualization toolkit", http://www.vtk.org/) to render 3D
   visualizations in Lua, a rotating globe in this example. You need VTK with
   the Tcl bindings installed to make this work, and the earth.ppm texture
   must be in the current directory when running this program. ]]

tk = require("tk")

--[[ Load the GUI and the rendering pipeline which is defined in the
   accompanying Tcl script, earth.tcl. NOTE: Depending on whether you have
   Gnocl installed, you'll either get a GTK+ GUI or just a plain Tk GUI. The
   Tcl script contains code for both. ]]

local function readfile(file)
   local f = assert(io.open(file, "rb"))
   local content = f:read("*all")
   f:close()
   return content
end

earth = readfile("earth.tcl")

--[[ The rendering callback, which rotates the representation by 3 degrees every
   10 milliseconds while the Rotate checkbutton is enabled. This shows how to
   work with the rendering pipeline from the Lua script. Note that most of
   the user interaction in the rendering window is handled by VTK itself. In
   particular, you can press the left, middle and right mouse buttons in the
   rendering window to rotate, pan and zoom the image, respectively. ]]

function render_cb(v)
   if tonumber(v) ~= 0 then
      tk.eval("renWin Render; [ren1 GetActiveCamera] Azimuth 3; after 10 {lua render_cb $rotate}")
   end
end

--[[ Two callbacks to change the object representation and the interaction
   style. Toggling the 'Wireframe' checkbutton or, equivalently, pressing
   'w'/'s' in the rendering window changes between wireframe and solid
   (surface) display. Similarly, toggling the 'Trackball Mode' checkbutton or
   pressing 't'/'j' switches between the "trackball" and "joystick" style of
   interaction. (In "trackball" mode, you need to drag the mouse to rotate/
   pan/zoom the image, while in "joystick" mode just pressing the mouse is
   enough.) ]]

function wireframe_cb(v)
   local s = tonumber(v) ~= 0 and "Wireframe" or "Surface"
   tk.eval(string.format("[world GetProperty] SetRepresentationTo%s; renWin Render", s))
end

function interactor_cb(v)
   local s = tonumber(v) ~= 0 and "Trackball" or "Joystick"
   tk.eval(string.format("[[renWin GetInteractor] GetInteractorStyle] SetCurrentStyleTo%sCamera", s))
end

--[[ The main program: execute the Tcl script and enter the main loop. You can
   also specify the -tk command line option to enforce a Tk GUI even if Gnocl
   is present. ]]

function main()
   if #arg > 0 and arg[1] == "-tk" then
      -- force the plain Tk GUI
      tk.eval(string.format("set GTK %d", 0))
   end
   tk.eval(earth)
   tk.main()
end

main()
