FROM alpine:latest

RUN apk add --no-cache build-base cmake make gmp-dev

RUN mkdir -p build

COPY . /usr/src/elite

WORKDIR /usr/src/elite/build

RUN cmake .. && make

WORKDIR /usr/src/elite

ENTRYPOINT [ "./build/elite" ]