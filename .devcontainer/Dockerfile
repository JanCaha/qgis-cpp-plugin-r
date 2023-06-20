FROM mcr.microsoft.com/devcontainers/base:jammy

RUN sudo gpg -k && \
    KEYRING=/usr/share/keyrings/qgis-archive-keyring.gpg && \
    wget -O $KEYRING https://download.qgis.org/downloads/qgis-archive-keyring.gpg && \
    touch /etc/apt/sources.list.d/qgis.sources && \
    echo 'Types: deb deb-src' | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'URIs: https://qgis.org/ubuntu' | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Suites: '$(lsb_release -c -s) | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Components: main' | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Signed-By: '$KEYRING | sudo tee -a /etc/apt/sources.list.d/qgis.sources && \
    echo 'Types: deb deb-src' | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    echo 'URIs: https://qgis.org/ubuntu-nightly' | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    echo 'Suites: '$(lsb_release -c -s) | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    echo 'Components: main' | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    echo 'Signed-By: '$KEYRING | sudo tee -a /etc/apt/sources.list.d/qgis-nightly.sources && \
    LASTSUPPORTED=focal && \
    KEYRING=/usr/share/keyrings/ubuntugis-archive-keyring.gpg && \
    sudo gpg --no-default-keyring --keyring $KEYRING --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 6B827C12C2D425E227EDCA75089EBE08314DF160 && \
    echo 'Types: deb deb-src' | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'URIs: https://ppa.launchpadcontent.net/ubuntugis/ppa/ubuntu' | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Suites: '$LASTSUPPORTED | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Architectures: '$(dpkg --print-architecture) | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Components: main' | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    echo 'Signed-By: '$KEYRING | sudo tee -a /etc/apt/sources.list.d/ubuntugis-stable.sources && \
    KEYRING=/usr/share/keyrings/r-archive-keyring.asc && \
    wget -qO- https://cloud.r-project.org/bin/linux/ubuntu/marutter_pubkey.asc | sudo tee -a $KEYRING && \
    touch /etc/apt/sources.list.d/r.sources && \
    echo 'Types: deb deb-src' | sudo tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'URIs: https://cloud.r-project.org/bin/linux/ubuntu'   | sudo tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Suites: '$(lsb_release -c -s)'-cran40/' | sudo tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Architectures:'$(dpkg --print-architecture) | sudo tee -a /etc/apt/sources.list.d/r.sources && \
    echo 'Signed-By: '$KEYRING | sudo tee -a /etc/apt/sources.list.d/r.sources

RUN sudo apt update && \
    sudo apt upgrade -y -q

RUN sudo apt update -y && \
    sudo apt-get install -y r-base r-base-dev && \
    Rscript -e "install.packages(c('Rcpp', 'RInside'))"

RUN export DEBIAN_FRONTEND=noninteractive && \
    echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections && \
    apt-get update && \
    apt-get -y -q install --no-install-recommends wget software-properties-common build-essential ca-certificates python3-pip dialog apt-utils && \
    apt-get -y -q install cmake clang

RUN sudo apt -y -q install pdal && \
    sudo apt -y -q install qgis qgis-dev
# && \
# apt-get clean autoclean && \
# apt-get autoremove --yes && \
# rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir pytest pytest-qgis pytest-cov pillow flake8 mypy pycodestyle yapf pb_tool pytest-qt 

ENV DEBIAN_FRONTEND=noninteractive
ENV QT_QPA_PLATFORM=offscreen
ENV XDG_RUNTIME_DIR=/tmp
ENV PYTHONPATH=/usr/share/qgis/python/plugins:/usr/share/qgis/python