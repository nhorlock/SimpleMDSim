# Main Image
FROM ubuntu:18.04
MAINTAINER Neil Horlock <nhorlock@gmail.com>

RUN apt update \
    && apt install -y sqlite3 \
    && apt install -y strace

COPY build/smdweb /smdweb
COPY tickdata.db /tickdata.db
#CMD /smdweb
CMD /bin/bash
