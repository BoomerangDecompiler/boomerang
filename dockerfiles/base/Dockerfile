FROM ubuntu:cosmic

RUN useradd -m boomerang
WORKDIR /home/boomerang/
ENV HOME /home/boomerang

RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    make \
    cmake \
    ccache \
    qt5-default \
    git \
    libcapstone-dev \
    bison \
    flex


COPY . boomerang
RUN chown -R boomerang:boomerang boomerang

