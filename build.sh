cd ~/CTP/AutoTrader/build
rm -rf *
export CC=/usr/local/bin/clang
export CXX=/usr/local/bin/clang
cmake ..
make
