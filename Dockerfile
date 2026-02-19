FROM gcc:13

RUN apt-get update && apt-get install -y \
    libcurl4-openssl-dev

WORKDIR /app
COPY . .

WORKDIR /app/backend

RUN mkdir -p build
RUN g++ -std=c++17 src/*.cpp -Iinclude -lcurl -o build/server

EXPOSE 8080

CMD ["./build/server"]
