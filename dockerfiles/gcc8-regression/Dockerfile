FROM ceeac/boomerang-build-base:latest

RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    g++-8 \
    python3

ENV CC  /usr/bin/gcc-8
ENV CXX /usr/bin/g++-8

