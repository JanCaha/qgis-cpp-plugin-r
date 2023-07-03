FROM ubuntu:jammy

RUN apt-get update && \
    apt-get -y -q install gpg wget lsb-core software-properties-common dirmngr git && \
    apt-get clean autoclean && \
    apt-get autoremove --yes && \
    rm -rf /var/lib/apt/lists/*

RUN gpg -k && \
    KEYRING=/usr/share/keyrings/qgis-archive-keyring.gpg && \
    wget -O $KEYRING https://download.qgis.org/downloads/qgis-archive-keyring.gpg && \
    touch /etc/apt/sources.list.d/qgis.sources && \
    echo 'Types: deb deb-src' | tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'URIs: https://qgis.org/ubuntugis' | tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Suites: '$(lsb_release -c -s) | tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Components: main' | tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Signed-By: '$KEYRING | tee -a /etc/apt/sources.list.d/qgis.sources && \
    KEYRING=/usr/share/keyrings/ubuntugis-archive-keyring.gpg && \
    gpg --no-default-keyring --keyring $KEYRING --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 6B827C12C2D425E227EDCA75089EBE08314DF160 && \
    echo 'Types: deb deb-src' | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'URIs: https://ppa.launchpadcontent.net/ubuntugis/ppa/ubuntu' | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Suites: '$(lsb_release -c -s) | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Components: main' | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Signed-By: '$KEYRING | tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Types: deb deb-src' | tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    echo 'URIs:https://ppa.launchpadcontent.net/ubuntugis/ubuntugis-unstable/ubuntu' | tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    echo 'Suites: '$(lsb_release -c -s)| tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    echo 'Components: main' | tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    echo 'Signed-By: '$KEYRING | tee -a /etc/apt/sources.list.d/ubuntugis-unstable.sources && \
    KEYRING=/usr/share/keyrings/r-archive-keyring.asc && \
    wget -qO- https://cloud.r-project.org/bin/linux/ubuntu/marutter_pubkey.asc | tee -a $KEYRING && \
    touch /etc/apt/sources.list.d/r.sources && \
    echo 'Types: deb deb-src' | tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'URIs: https://cloud.r-project.org/bin/linux/ubuntu' | tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Suites: '$(lsb_release -c -s)'-cran40/' | tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Architectures:'$(dpkg --print-architecture) | tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Signed-By: '$KEYRING | tee -a /etc/apt/sources.list.d/r.sources


RUN export DEBIAN_FRONTEND=noninteractive && \
    echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections && \
    apt-get update && \
    apt-get -y -q install --no-install-recommends wget cmake software-properties-common build-essential ca-certificates python3-pip dialog apt-utils && \
    apt-get -y -q install clang clang-14 lld-14 libclang-14-dev  ninja-build g++-12 doxygen cmake libgtest-dev && \
    apt -y -q install qgis qgis-dev qgis-plugin-grass && \
    apt -y -q install r-base r-base-dev && \
    # libraries for r packages
    apt -y -q install libudunits2-dev gdal-bin  libgdal-dev libgit2-dev libharfbuzz-dev libfribidi-dev libssh-dev qpdf && \
    apt-get clean autoclean && \
    apt-get autoremove --yes && \
    rm -rf /var/lib/apt/lists/*

RUN Rscript -e "install.packages(c('Rcpp', 'tidyverse', 'sf', 'terra', 'raster', 'star', 'wk'))"

RUN export CXX=/usr/bin/clang++ && \
    export CC=/usr/bin/clang

RUN git clone https://github.com/JanCaha/qgis-cpp-plugin-r.git && \
    cd /qgis-cpp-plugin-r && \
    mkdir build && \
    mkdir plugin && \
    cmake -S . -B build -G Ninja -D DOCKER_RELEASE=ON && \
    cmake --build build --config Release --target all -- && \
    cp build/src/libr_plugin.so  /lib/qgis/plugins/libr_plugin.so 