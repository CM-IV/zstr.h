
#include "zstr_lua.h"

extern int luaopen_zstr(lua_State *L) 
{
    return zstr_register_lib(L);
}
