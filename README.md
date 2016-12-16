kaudiocreator
-------------

## Build && Install
```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr    \
    -DCMAKE_C_COMPILER=clang    \
    -DCMAKE_CXX_COMPILER=clang++    \
    -DECM_ENABLE_SANITIZERS='address;leak;undefined'    \
    -DENABLE_DEBUG=ON   \
    -DBUILD_TESTING=ON
make
sudo make install
```
