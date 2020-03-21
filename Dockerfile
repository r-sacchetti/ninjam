FROM ubuntu

# Update image and install dependencies
RUN set -x \
    && apt-get update \
    && apt-get install -y build-essential git jq --no-install-recommends \
    && apt-get install g++ \
    && apt-get install -y ca-certificates \
    && apt-get clean

    # get sources and build ninjam server
RUN git clone https://github.com/r-sacchetti/ninjam.git && \
    # build ninjamsrv
    cd /ninjam/ninjam/server && \
    make


CMD ["/ninjam/ninjam/server/ninjamsrv","/ninjam/ninjam/server/example.cfg"]