FROM ubuntu:18.04

# Install required architecture
RUN dpkg --add-architecture i386 && apt-get update

# Install pg deps
RUN apt-get install -y libpq-dev:i386

# Install compiler + utils
RUN apt-get install -y curl build-essential g++-8 g++-8-multilib

# Set g++-8 the default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

# Install premake5
RUN cd /tmp && \
    curl -sLo premake5.tar.gz https://github.com/premake/premake-core/releases/download/v5.0.0-alpha13/premake-5.0.0-alpha13-linux.tar.gz && \
    tar xvf premake5.tar.gz && \
    mv premake5 /usr/local/bin/premake5 && \
    rm premake5.tar.gz

# We expect /project
ADD entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]