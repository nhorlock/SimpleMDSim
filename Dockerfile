# Main Image
FROM ubuntu:18.04
MAINTAINER Neil Horlock <nhorlock@gmail.com>

RUN apt update \
    && apt install -y sqlite3 \
    && apt install -y strace \
    && mkdir -p /db

EXPOSE 3000

COPY build/smdweb /smdweb
ADD tickdata.db /tickdata.db
COPY launch /launch
CMD /launch
