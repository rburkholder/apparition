# prometheus-cpp exporter development kit installation

    sudo apt install --no-install-recommends libcurl4-openssl-dev
    pushd 3rdparty/
    git clone --depth=1 https://github.com/jupp0r/prometheus-cpp.git
    cd prometheus-cpp/
    git submodule init
    git submodule update
    mkdir build
    cd build
    cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=ON -DENABLE_COMPRESSION=OFF
    cmake --build . --parallel 4
    sudo cmake --install .
    popd
