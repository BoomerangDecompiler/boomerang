FROM ceeac/boomerang-build-base:latest

RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    clang-7

ENV CC  /usr/bin/clang-7
ENV CXX /usr/bin/clang++-7

