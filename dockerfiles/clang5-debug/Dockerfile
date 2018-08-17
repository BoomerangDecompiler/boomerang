FROM ceeac/boomerang-build-base:latest

RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    clang-5.0

ENV CC  /usr/bin/clang-5.0
ENV CXX /usr/bin/clang++-5.0

