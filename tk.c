
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <tcl.h>
#include <tk.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* The Tcl interpreter. */

static Tcl_Interp* interp;

/* Handle X11 protocol errors */

static int XErrorProc(ClientData data, XErrorEvent *errEventPtr)
{
  fprintf(stderr, "X protocol error: ");
  fprintf(stderr, "error=%d request=%d minor=%d\n",
	  errEventPtr->error_code, errEventPtr->request_code,
	  errEventPtr->minor_code);
  return 0;
}

/* Helper functions for error handling. */

static inline const char *get_result(Tcl_Interp* interp)
{
  return Tcl_GetStringResult(interp);
}

static inline bool check_result(Tcl_Interp* interp)
{
  const char *res = Tcl_GetStringResult(interp);
  return res && *res;
}

static inline void set_result(char **result, const char *s)
{
  *result = malloc(strlen(s)+1);
  if (*result) strcpy(*result, s);
}

/* Tcl command to invoke Lua callbacks. */

static int tk_lua(ClientData clientData,
		  Tcl_Interp *interp,
		  int argc, char **argv)
{
  int i, n;
  lua_State *L = (lua_State*)clientData;
  Tcl_ResetResult(interp);
  if (argc < 2) {
    /* Callback is missing. */
    Tcl_AppendResult(interp, "missing callback", NULL);
    return TCL_ERROR;
  }
  n = lua_gettop(L);
  /* Set up function and arguments. */
  lua_getglobal(L, argv[1]);
  for (i = 2; i < argc; i++)
    lua_pushstring(L, argv[i]);
  /* Do the call. */
  if (lua_pcall(L, argc-2, LUA_MULTRET, 0)) {
    /* Callback raised an exception. */
    Tcl_AppendResult(interp, "exception while executing callback ", argv[1],
		     "\n", lua_tostring(L, -1), NULL);
    lua_pop(L, 1);
    return TCL_ERROR;
  } else {
    /* Get the results. */
    int m = lua_gettop(L), l = m-n;
    const char **av = (const char**)malloc(l*sizeof(char*));
    char *ret;
    for (i = 0; i < l; i++)
      av[i] = lua_tostring(L, i+n+1);
    /* This creates a proper Tcl list from the return values. */
    ret = Tcl_Merge(l, av);
    Tcl_AppendResult(interp, ret, NULL);
    Tcl_Free(ret);
    lua_pop(L, l);
    /* Callback was executed successfully. */
    return TCL_OK;
  }
}

/* Starting and stopping the Tcl interpreter. */

static void tk_stop(void);

static bool tk_start(lua_State *L, char **result)
{
  static bool first_init = false;
  Tk_Window mainw;
  if (!first_init) {
    first_init = true;
    /* this works around a bug in some Tcl/Tk versions */
    Tcl_FindExecutable(NULL);
    /* finalize Tcl at program exit -- this causes the Lua interpreter to hang
       at exit time, disabled for now */
    //atexit(Tcl_Finalize);
  }
  *result = NULL;
  if (interp) return true;
  /* start up a new interpreter */
  if (!(interp = Tcl_CreateInterp())) return false;
  if (Tcl_Init(interp) != TCL_OK) {
    if (check_result(interp))
      set_result(result, get_result(interp));
    else
      set_result(result, "error initializing Tcl");
    tk_stop();
    return false;
  }
  /* create a command to invoke Pure callbacks from Tcl */
  Tcl_CreateCommand(interp, "lua", (Tcl_CmdProc*)tk_lua,
		    (ClientData)L, NULL);
  /* oddly, there are no `env' variables passed, and this one is needed */
  Tcl_SetVar2(interp, "env", "DISPLAY", getenv("DISPLAY"), TCL_GLOBAL_ONLY);
  if (Tk_Init(interp) != TCL_OK) {
    if (check_result(interp))
      set_result(result, get_result(interp));
    else
      set_result(result, "error initializing Tk");
    tk_stop();
    return false;
  }
  /* set up an X error handler */
  mainw = Tk_MainWindow(interp);
  Tk_CreateErrorHandler(Tk_Display(mainw), -1, -1, -1,
			XErrorProc, (ClientData)mainw);
  return true;
}

static void tk_stop(void)
{
  if (interp) {
    Tcl_DeleteInterp(interp);
    interp = NULL;
  }
}

/* Process events in the Tcl interpreter. */

static bool do_event(void)
{
  return Tcl_DoOneEvent(TCL_DONT_WAIT);
}

static void tk_do_events(void)
{
  if (!interp) return;
  while (Tk_MainWindow(interp) && do_event()) ;
  if (!Tk_MainWindow(interp)) tk_stop();
}

static bool tk_running(void)
{
  tk_do_events();
  return interp != NULL;
}

/* Evaluate Tcl commands. */

static bool tk_eval(const char *s, char **result)
{
  int status;
  char *cmd;
  *result = NULL;
  if (!interp) return false;
  cmd = malloc(strlen(s)+1);
  if (!cmd) return false;
  strcpy(cmd, s);
  status = Tcl_Eval(interp, cmd);
  if (interp && check_result(interp))
    set_result(result, get_result(interp));
  else if (status == TCL_BREAK)
    set_result(result, "invoked \"break\" outside of a loop");
  else if (status == TCL_CONTINUE)
    set_result(result, "invoked \"continue\" outside of a loop");
  if (status == TCL_BREAK || status == TCL_CONTINUE)
    status = TCL_ERROR;
  tk_do_events();
  free(cmd);
  return status != TCL_ERROR;
}

/* Error handling. */

static int tk_error(lua_State *L, const char *result)
{
  char msg[1024];
  snprintf(msg, 1024, "tk_error: %s", result);
  lua_pushstring(L, msg);
  return lua_error(L);
}

/* Interface functions. */

static int tk(lua_State *L)
{
  char *result = NULL;
  const char *s = luaL_checkstring(L, 1);
  if (!s)
    return tk_error(L, "tk: expected string argument");
  if (tk_start(L, &result)) {
    bool res;
    /* Make sure that we don't pull the rug under ourselves. */
    Tcl_Interp* _interp = interp;
    Tcl_Preserve(_interp);
    res = tk_eval(s, &result);
    Tcl_Release(_interp);
    if (!res)
      return tk_error(L, result);
    else if (result&&*result) {
      lua_pushstring(L, result);
      return 1;
    } else
      return 0;
  } else
    return tk_error(L, result);
}

int tk_set(lua_State *L)
{
  char *result = NULL;
  const char *name = luaL_checkstring(L, 1);
  const char *val = luaL_checkstring(L, 2);
  if (!name)
    return tk_error(L, "tk_set: expected string argument #1 (name)");
  if (!val)
    return tk_error(L, "tk_set: expected string argument #2 (value)");
  if (tk_start(L, &result)) {
    const char *res = Tcl_SetVar(interp, name, val, TCL_GLOBAL_ONLY);
    if (res) {
      lua_pushstring(L, res);
      return 1;
    } else
      // XXXTODO: We should maybe give more useful diagnostics here.
      return tk_error(L, "tk_set: error setting variable");
  } else
    return tk_error(L, result);
}

int tk_unset(lua_State *L)
{
  char *result = NULL;
  const char *name = luaL_checkstring(L, 1);
  if (!name)
    return tk_error(L, "tk_unset: expected string argument");
  if (tk_start(L, &result)) {
    int res = Tcl_UnsetVar(interp, name, TCL_GLOBAL_ONLY);
    if (res == TCL_OK)
      return 0;
    else
      // XXXTODO: We should maybe give more useful diagnostics here.
      return tk_error(L, "tk_unset: error unsetting variable");
  } else
    return tk_error(L, result);
}

int tk_get(lua_State *L)
{
  char *result = NULL;
  const char *name = luaL_checkstring(L, 1);
  if (!name)
    return tk_error(L, "tk_get: expected string argument");
  if (tk_start(L, &result)) {
    const char *res = Tcl_GetVar(interp, name, TCL_GLOBAL_ONLY);
    if (res) {
      lua_pushstring(L, res);
      return 1;
    } else
      // XXXTODO: We should maybe give more useful diagnostics here.
      return tk_error(L, "tk_get: error getting variable");
  } else
    return tk_error(L, result);
}

int tk_split(lua_State *L)
{
  int argc, ret;
  const char **argv;
  const char *s = luaL_checkstring(L, 1);
  if (!s)
    return tk_error(L, "tk_split: expected string argument");
  ret = Tcl_SplitList(NULL, s, &argc, &argv);
  if (ret == TCL_OK) {
    int i;
    lua_createtable(L, argc, 0);
    for (i = 0; i < argc; i++) {
      lua_pushinteger(L, i+1);
      lua_pushstring(L, argv[i]);
      lua_settable(L, -3);
    }
    Tcl_Free((char *)argv);
    return 1;
  } else {
    if (argv) Tcl_Free((char *)argv);
    return tk_error(L, "tk_split: error splitting value");
  }
}

int tk_join(lua_State *L)
{
  if (lua_istable(L, -1)) {
    int argc, i;
    const char **argv = NULL;
    char *ret;
#if LUA_VERSION_NUM < 502
    argc = lua_objlen(L, -1);
#else // 5.2 style
    argc = lua_rawlen(L, -1);
#endif // LUA_VERSION_NUM < 502
    if (argc > 0) argv = malloc(argc * sizeof(const char*));
    i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      const char *s = lua_tostring(L, -1);
      if (i == argc) {
	lua_pop(L, 2);
	if (argv) free(argv);
	return tk_error(L, "tk_join: too many table elements");
      } else if (!s) {
	lua_pop(L, 2);
	if (argv) free(argv);
	return tk_error(L, "tk_join: expected string table");
      } else
	argv[i] = s;
      lua_pop(L, 1);
      ++i;
    }
    lua_pop(L, 1);
    if (i != argc) {
      if (argv) free(argv);
      return tk_error(L, "tk_join: too few table elements");
    }
    ret = Tcl_Merge(argc, argv);
    if (argv) free(argv);
    lua_pushstring(L, ret);
    Tcl_Free(ret);
    return 1;
  } else {
    return tk_error(L, "tk_join: expected table argument");
  }
}

int tk_quit(lua_State *L)
{
  tk_stop();
  return 0;
}

int tk_ready(lua_State *L)
{
  lua_pushboolean (L, tk_running());
  return 1;
}

int tk_main(lua_State *L)
{
  char *result = NULL;
  if (tk_start(L, &result)) {
    while (interp && Tk_MainWindow(interp) && Tcl_DoOneEvent(0)) ;
    if (interp && !Tk_MainWindow(interp)) tk_stop();
    return 0;
  } else
    return tk_error(L, result);
}

static const struct luaL_Reg l_tk [] = {
  {"eval", tk},
  {"set", tk_set},
  {"unset", tk_unset},
  {"get", tk_get},
  {"split", tk_split},
  {"join", tk_join},
  {"quit", tk_quit},
  {"ready", tk_ready},
  {"main", tk_main},
  {NULL, NULL}  /* sentinel */
};

int luaopen_tk (lua_State *L) {
  luaL_newlib(L, l_tk);
  return 1;
}
