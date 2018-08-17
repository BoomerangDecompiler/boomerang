FROM ceeac/boomerang-build-base:latest

RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    g++-7 \
    lcov \
    curl

ENV CC  /usr/bin/gcc-7
ENV CXX /usr/bin/g++-7

