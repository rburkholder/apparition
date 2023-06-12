# Installation - LuaJIT

* pushd 3rd_party/
* git clone --depth=1 https://github.com/LuaJIT/LuaJIT.git
* cd LuaJIT
* make a change to the srcMakefile:

```
$ git diff src/Makefile
diff --git a/src/Makefile b/src/Makefile
index 30d64be..8c943f2 100644
--- a/src/Makefile
+++ b/src/Makefile
@@ -99,7 +99,7 @@ XCFLAGS=
 # enabled by default. Some other features that *might* break some existing
 # code (e.g. __pairs or os.execute() return values) can be enabled here.
 # Note: this does not provide full compatibility with Lua 5.2 at this time.
-#XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT
+XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT
 #
 # Disable the JIT compiler, i.e. turn LuaJIT into a pure interpreter.
 #XCFLAGS+= -DLUAJIT_DISABLE_JIT
``` 
* make 
* sudo make install
* popd

# Installation - LuaRocks

* not needed currently

# Installation - lua-cjson - https://github.com/mpx/lua-cjson/

* pushd 3rd_party
* git clone --depth=1 https://github.com/mpx/lua-cjson.git
* cd lua-cjson
* delete a bunch of lines in lua_cjson.c

```
 git diff lua_cjson.c
diff --git a/lua_cjson.c b/lua_cjson.c
index 22f33f1..4d4a46e 100644
--- a/lua_cjson.c
+++ b/lua_cjson.c
@@ -1304,26 +1304,6 @@ static int json_decode(lua_State *l)

 /* ===== INITIALISATION ===== */

-#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
-/* Compatibility for Lua 5.1.
- *
- * luaL_setfuncs() is used to create a module table where the functions have
- * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source. */
-static void luaL_setfuncs (lua_State *l, const luaL_Reg *reg, int nup)
-{
-    int i;
-
-    luaL_checkstack(l, nup, "too many upvalues");
-    for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
-        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
-            lua_pushvalue(l, -nup);
-        lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
-        lua_setfield(l, -(nup + 2), reg->name);
-    }
-    lua_pop(l, nup);  /* remove upvalues */
-}
-#endif
-
 /* Call target function in protected mode with all supplied args.
  * Assumes target function only returns a single non-nil value.
  * Convert and return thrown errors as: nil, "error message" */
```

* mkdir build
* cd build
* export LUA_DIR=/usr/local/include/luajit-2.1/
* cmake -D LUA_INCLUDE_DIR=/usr/local/include/luajit-2.1/  ..
* make 
* #make install
* #result: Installing: /usr/lib/x86_64-linux-gnu/lua/5.1/cjson.so
* #instead:
* sudo mkdir -p /usr/local/lib/lua/luajit/
* sudo cp cjson.so /usr/local/lib/lua/luajit/
* popd

