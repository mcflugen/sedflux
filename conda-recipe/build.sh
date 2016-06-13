if [ `uname` == 'Darwin' ]; then
  (cd /usr/local && mv bin bin.hide && mv lib lib.hide && mv include include.hide)
fi

mkdir _build && cd _build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=Release
make all -j4
make install

if [ `uname` == 'Darwin' ]; then
  (cd /usr/local && mv bin.hide bin && mv lib.hide lib && mv include.hide include)
fi
