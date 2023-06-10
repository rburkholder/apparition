# Installation - LuaJIT

* https://github.com/LuaJIT/LuaJIT.git
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

# Installation - LuaRocks

* not needed currently

# Installation - lua-cjson - https://github.com/mpx/lua-cjson/

* sudo apt install lua5.1 liblua5.1-dev
* pushd 3rd_party
* git clone --depth=1 https://github.com/mpx/lua-cjson.git
* cd lua-cjson
* mkdir build
* cd build
* cmake ..
* make 
* #make install
* #result: Installing: /usr/lib/x86_64-linux-gnu/lua/5.1/cjson.so
* #instead:
* sudo cp cjson.so /usr/local/lib/lua/5.1/
* popd

