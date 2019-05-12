FROM alpine:3.8
MAINTAINER Yohei KIKUTA "diracdiego@gmail.com"

RUN apk --update add \
    ghostscript \
    gcc \
    libc-dev

WORKDIR /work

CMD ["/bin/sh"]
