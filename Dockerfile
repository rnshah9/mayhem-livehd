FROM --platform=linux/amd64 ubuntu:20.04 as builder
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install  -y vim less man wget tar git unzip make cmake software-properties-common

RUN apt install -y apt-transport-https curl gnupg 
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg
RUN mv bazel.gpg /etc/apt/trusted.gpg.d/
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN apt update && apt install -y bazel

ADD . /livehd
WORKDIR /livehd

RUN bazel build //main:lgshell


FROM --platform=linux/amd64 ubuntu:20.04 

COPY --from=builder /livehd/bazel-bin/main/lgshell .
