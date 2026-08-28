/*
** $Id: lnoparser.c $
** Front-end stubs for the bytecode-only Lua library
** See Copyright Notice in lua.h
*/

/*
** Added for Oidua. Not part of upstream Lua.
**
** The game stub only ever loads precompiled bytecode, so the Lua front-end
** is dead weight in it. This file is what makes dropping the front-end cost
** nothing in the Lua sources themselves: every .c and .h file next to it is
** byte-identical to the official lua-5.4.8 release, and the entire delta
** lives here and in the liblua_bco target in CMakeLists.txt. Re-basing on a
** future 5.4.x is then a plain drop-in of src/.
**
** Only three symbols tie the rest of the library to the front-end:
**
**   luaX_init    called once from f_luaopen in lstate.c to intern the
**                reserved words; the only reference into llex.c from
**                outside the parser
**   luaY_parser  called from f_parser in ldo.c to compile source text; the
**                only reference into lparser.c and lcode.c
**   luaU_dump    called from lua_dump in lapi.c to write bytecode; the only
**                reference into ldump.c
**
** Defining them here lets lparser.c, lcode.c, llex.c and ldump.c stay out
** of the archive. This file also stands in for linit.c, with a
** luaL_openlibs that opens only the sandboxed subset of the standard
** library, which is what lets loadlib.c, liolib.c and ldblib.c go too.
**
** Because these are real definitions and not #ifdef'd-away call sites,
** linking any of those files back into liblua_bco is a duplicate symbol
** error at build time rather than a parser that quietly comes back to life.
*/

#define lnoparser_c
#define LUA_CORE
#define LUA_LIB

#include "lprefix.h"


#include <stddef.h>

#include "lua.h"

#include "lauxlib.h"
#include "ldo.h"
#include "llex.h"
#include "llimits.h"
#include "lobject.h"
#include "lparser.h"
#include "lualib.h"
#include "lundump.h"


/*
** Interning the reserved words only ever serves the lexer, which is not in
** this build, so there is nothing to intern.
*/
void luaX_init (lua_State *L) {
  UNUSED(L);
}


/*
** Compiling source text is precisely what this build exists to prevent.
** Fail the way the lexer would have: LUA_ERRSYNTAX with a message left on
** the stack, so lua_load and luaL_loadbuffer report it through their normal
** path and callers never have to know they are talking to a stub.
*/
LClosure *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff, Dyndata *dyd,
                       const char *name, int firstchar) {
  UNUSED(z); UNUSED(buff); UNUSED(dyd); UNUSED(firstchar);
  luaO_pushfstring(L, "%s: source loading is disabled (bytecode only)", name);
  luaD_throw(L, LUA_ERRSYNTAX);
  return NULL;  /* to avoid warnings; luaD_throw does not return */
}


/*
** Writing bytecode needs ldump.c. lua_dump reads any non-zero return as
** "could not dump", the same answer it gives for a C function, so there is
** no new failure mode to document here.
*/
int luaU_dump (lua_State *L, const Proto *f, lua_Writer w, void *data,
               int strip) {
  UNUSED(L); UNUSED(f); UNUSED(w); UNUSED(data); UNUSED(strip);
  return 1;
}


/*
** Stands in for linit.c. package (and with it require), io and debug are
** left out: none of the three is part of the documented Oidua scripting
** API, and leaving them out is what keeps loadlib.c, liolib.c and ldblib.c
** out of the archive.
*/
static const luaL_Reg loadedlibs[] = {
  {LUA_GNAME, luaopen_base},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}
