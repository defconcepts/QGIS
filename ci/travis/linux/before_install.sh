export DEBIAN_FRONTEND=noninteractive

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo add-apt-repository 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.7 main' -y

sudo add-apt-repository ppa:ubuntugis/ppa -y
sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable -y # For postgresql-9.1-postgis-2.1
sudo add-apt-repository ppa:grass/grass-stable -y
sudo add-apt-repository ppa:smspillaz/cmake-3.0.2 -y
sudo add-apt-repository ppa:kedazo/doxygen-updates-precise -y # For doxygen 1.8.8
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -qq
sudo apt-get install --force-yes --no-install-recommends --no-install-suggests \
        bison \
        cmake \
        cmake-data \
        doxygen \
        flex \
        git \
        graphviz \
        grass-dev \
        grass7-dev \
        libexpat1-dev \
        libfcgi-dev \
        libgdal1-dev \
        libgeos-dev \
        libgsl0-dev \
        libpq-dev \
        libproj-dev \
        libqca2-dev \
        libqca2-plugin-ossl \
        libqscintilla2-dev \
        libqt4-dev \
        libqt4-opengl-dev \
        libqt4-sql-sqlite \
        libqtwebkit-dev \
        libqwt-dev \
        libspatialindex-dev \
        libspatialite-dev \
        libsqlite3-dev \
        lighttpd \
        pkg-config \
        poppler-utils \
        pyqt4-dev-tools \
        python \
        python-dev \
        python-qt4 \
        python-qt4-dev \
        python-qt4-sql \
        python-sip \
        python-sip-dev \
        python-gdal \
        spawn-fcgi \
        txt2tags \
        xauth \
        xfonts-100dpi \
        xfonts-75dpi \
        xfonts-base \
        xfonts-scalable \
        xvfb \
        postgresql-9.1-postgis-2.1/precise # from ubuntugis-unstable, not pgdg

#update clang
sudo apt-get install --force-yes llvm-3.7 llvm-3.7-dev clang-3.7 libstdc++-4.9-dev
export CXX="clang++-3.7" 
export CC="clang-3.7"

cmake --version
clang --version
