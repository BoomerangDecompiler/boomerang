FROM ubuntu:bionic

RUN useradd -m boomerang
WORKDIR /home/boomerang/
ENV HOME /home/boomerang


RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    python3 \
    clang-format


COPY . boomerang
RUN chown -R boomerang:boomerang boomerang

