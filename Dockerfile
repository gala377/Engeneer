FROM gcc:latest

RUN apt-get -y update && apt-get install -y

RUN apt-get -y install g++ 

RUN apt-get -y install libboost-test-dev 
RUN apt-get -y install libboost-system-dev 
RUN apt-get -y install libboost-filesystem-dev 

RUN apt-get -y install wget

RUN apt remove --purge --auto-remove cmake
RUN mkdir /temp_cmake
WORKDIR /temp_cmake

ARG version=3.13
ARG build=0 
RUN wget https://cmake.org/files/v${version}/cmake-${version}.${build}-Linux-x86_64.sh 
RUN mkdir /opt/cmake 
RUN sh cmake-${version}.${build}-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
RUN echo "CMAKE IS: $(cmake --version)"

COPY . /usr/src/test
RUN mkdir /usr/src/test/build
WORKDIR /usr/src/test/build

# build all 
RUN cmake ..
RUN make 

CMD ["./src/build/main"]

