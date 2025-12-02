#ifndef ZSTR_LUA_H
#define ZSTR_LUA_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "zstr.h"

#define ZSTR_LUA_MT "zstr_mt"

/* Helpers. */

static zstr* check_zstr(lua_State *L, int index) 
{
    return (zstr*)luaL_checkudata(L, index, ZSTR_LUA_MT);
}

static zstr* push_new_zstr(lua_State *L) 
{
    zstr *s = (zstr*)lua_newuserdata(L, sizeof(zstr));
    *s = zstr_init();
    luaL_getmetatable(L, ZSTR_LUA_MT);
    lua_setmetatable(L, -2);
    return s;
}

/* Constructor and destructor. */

// zstr.new(optional_string)
static int l_zstr_new(lua_State *L) 
{
    const char *init_str = luaL_optstring(L, 1, NULL);
    zstr *s = push_new_zstr(L);
    if (init_str) *s = zstr_from(init_str);
    return 1;
}

// zstr.from_file(path)
static int l_zstr_from_file(lua_State *L) 
{
    const char *path = luaL_checkstring(L, 1);
    zstr *s = push_new_zstr(L);
    *s = zstr_read_file(path);
    return 1;
}

// s:clone() -> new_zstr
static int l_zstr_clone(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    zstr *new_s = push_new_zstr(L);
    *new_s = zstr_dup(s);
    return 1;
}

static int l_zstr_gc(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    zstr_free(s);
    return 0;
}

/* Buffer operations. */

// s:append(str1, str2, ...)
static int l_zstr_append(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    int n = lua_gettop(L);
    for (int i = 2; i <= n; i++) 
    {
        size_t len;
        const char *txt = luaL_checklstring(L, i, &len);
        zstr_cat_len(s, txt, len);
    }
    lua_pushvalue(L, 1); // Return self.
    return 1;
}

// s:pop() -> char_string
static int l_zstr_pop(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    char c = zstr_pop_char(s);
    if (c == '\0' && zstr_len(s) == 0) return 0;
    lua_pushlstring(L, &c, 1);
    return 1;
}

// s:reserve(bytes)
static int l_zstr_reserve(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    lua_Integer cap = luaL_checkinteger(L, 2);
    if (cap > 0) zstr_reserve(s, (size_t)cap);
    return 0;
}

// s:shrink()
static int l_zstr_shrink(lua_State *L) 
{
    zstr_shrink_to_fit(check_zstr(L, 1));
    return 0;
}

// s:clear()
static int l_zstr_clear(lua_State *L) 
{
    zstr_clear(check_zstr(L, 1));
    return 0;
}

/* Transformations. */

static int l_zstr_trim(lua_State *L) 
{
    zstr_trim(check_zstr(L, 1));
    return 0;
}

static int l_zstr_lower(lua_State *L) 
{
    zstr_to_lower(check_zstr(L, 1));
    return 0;
}

static int l_zstr_upper(lua_State *L) 
{
    zstr_to_upper(check_zstr(L, 1));
    return 0;
}

// s:replace(target, replacement)
static int l_zstr_replace(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *tgt = luaL_checkstring(L, 2);
    const char *repl = luaL_checkstring(L, 3);
    zstr_replace(s, tgt, repl);
    return 0;
}

/* Queries. */

// s:contains(needle) -> bool
static int l_zstr_contains(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *needle = luaL_checkstring(L, 2);
    lua_pushboolean(L, zstr_contains(s, needle));
    return 1;
}

// s:find(needle) -> index (1-based for Lua) or nil
static int l_zstr_find(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *needle = luaL_checkstring(L, 2);
    ptrdiff_t idx = zstr_find(s, needle);
    if (idx < 0) lua_pushnil(L);
    else lua_pushinteger(L, idx + 1);
    return 1;
}

static int l_zstr_starts_with(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *prefix = luaL_checkstring(L, 2);
    lua_pushboolean(L, zstr_starts_with(s, prefix));
    return 1;
}

static int l_zstr_ends_with(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *suffix = luaL_checkstring(L, 2);
    lua_pushboolean(L, zstr_ends_with(s, suffix));
    return 1;
}

static int l_zstr_is_valid_utf8(lua_State *L) 
{
    lua_pushboolean(L, zstr_is_valid_utf8(check_zstr(L, 1)));
    return 1;
}

static int l_zstr_rune_count(lua_State *L) 
{
    lua_pushinteger(L, zstr_count_runes(check_zstr(L, 1)));
    return 1;
}

static int l_zstr_is_empty(lua_State *L) 
{
    lua_pushboolean(L, zstr_is_empty(check_zstr(L, 1)));
    return 1;
}

static int l_zstr_capacity(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    size_t cap = s->is_long ? s->l.cap : ZSTR_SSO_CAP;
    lua_pushinteger(L, cap);
    return 1;
}

// s:split(delim) -> returns Lua Table of strings
static int l_zstr_split(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    const char *delim = luaL_checkstring(L, 2);
    
    lua_newtable(L);
    int idx = 1;
    
    zstr_view v = zstr_as_view(s);
    zstr_split_iter it = zstr_split_init(v, delim);
    zstr_view part;
    
    while(zstr_split_next(&it, &part)) 
    {
        lua_pushlstring(L, part.data, part.len);
        lua_rawseti(L, -2, idx++);
    }
    return 1;
}

/* Metamethods. */

static int l_zstr_tostring(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    lua_pushlstring(L, zstr_cstr(s), zstr_len(s));
    return 1;
}

static int l_zstr_len(lua_State *L) 
{
    zstr *s = check_zstr(L, 1);
    lua_pushinteger(L, (lua_Integer)zstr_len(s));
    return 1;
}

// zstr == zstr (or string)
static int l_zstr_eq(lua_State *L) 
{
    zstr *a = check_zstr(L, 1);
    // Support comparing zstr == string.
    if (lua_type(L, 2) == LUA_TSTRING) 
    {
        size_t len;
        const char *s = lua_tolstring(L, 2, &len);
        zstr_view v = zstr_as_view(a);
        lua_pushboolean(L, (v.len == len && memcmp(v.data, s, len) == 0));
        return 1;
    }
    zstr *b = check_zstr(L, 2);
    lua_pushboolean(L, zstr_eq(a, b));
    return 1;
}

// zstr .. any -> string (Concatenation returns a standard Lua string)
static int l_zstr_concat(lua_State *L) 
{
    // Note: zstr is mutable, but .. operator implies a new value,
    // so we return a Lua string to avoid side-effects.
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    
    // Arg 1.
    if (lua_isuserdata(L, 1)) 
    {
        zstr *s = check_zstr(L, 1);
        luaL_addlstring(&b, zstr_cstr(s), zstr_len(s));
    } 
    else 
    {
        luaL_addvalue(&b);
    }
    
    // Arg 2.
    if (lua_isuserdata(L, 2)) 
    {
        zstr *s = check_zstr(L, 2);
        luaL_addlstring(&b, zstr_cstr(s), zstr_len(s));
    } 
    else 
    {
        luaL_addvalue(&b);
    }
    
    luaL_pushresult(&b);
    return 1;
}

static const luaL_Reg zstr_methods[] = {
    // Lifecycle.
    {"new",         l_zstr_new},
    {"from_file",   l_zstr_from_file},
    {"clone",       l_zstr_clone},
    
    // Buffer.
    {"append",      l_zstr_append},
    {"pop",         l_zstr_pop},
    {"reserve",     l_zstr_reserve},
    {"shrink",      l_zstr_shrink},
    {"clear",       l_zstr_clear},
    {"capacity",    l_zstr_capacity},

    // Transform.
    {"trim",        l_zstr_trim},
    {"lower",       l_zstr_lower},
    {"upper",       l_zstr_upper},
    {"replace",     l_zstr_replace},

    // Query.
    {"contains",    l_zstr_contains},
    {"find",        l_zstr_find},
    {"starts_with", l_zstr_starts_with},
    {"ends_with",   l_zstr_ends_with},
    {"is_valid_utf8", l_zstr_is_valid_utf8},
    {"rune_count",  l_zstr_rune_count},
    {"is_empty",    l_zstr_is_empty},
    
    // Utils.
    {"split",       l_zstr_split},

    // Meta.
    {"__gc",        l_zstr_gc},
    {"__tostring",  l_zstr_tostring},
    {"__len",       l_zstr_len},
    {"__eq",        l_zstr_eq},
    {"__concat",    l_zstr_concat},
    
    {NULL, NULL}
};

static inline int zstr_register_lib(lua_State *L) 
{
    luaL_newmetatable(L, ZSTR_LUA_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, zstr_methods, 0);
    return 1;
}

#endif // ZSTR_LUA_H
