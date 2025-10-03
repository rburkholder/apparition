# Installation - LuaJIT

    cd  3rdparty
    git clone --depth=1 https://github.com/LuaJIT/LuaJIT.git
    cd LuaJIT
    sed -i 's/#XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT/XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT/' src/Makefile
    make 
    sudo make install

# Installation - LuaRocks

* not needed currently

# Installation - lua-cjson

    pushd 3rdparty
    git clone --depth=1 https://github.com/mpx/lua-cjson.git
    cd lua-cjson
    sed -i 's/luaL_setfuncs/compat_luaL_setfuncs/' lua_cjson.c
    mkdir build
    cd build
    export LUA_DIR=/usr/local/include/luajit-2.1/
    cmake -D LUA_INCLUDE_DIR=/usr/local/include/luajit-2.1/ -D LUA_LIBRARY=/usr/local/lib/libluajit-5.1  ..
    make 
    #make install
    #result: Installing: /usr/lib/x86_64-linux-gnu/lua/5.1/cjson.so
    #instead:
    popd
    cp 3rdparty/lua-cjson/build/cjson.so var/lib/lua/

