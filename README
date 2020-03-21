# NINJAM server

## About

NINJAM stands for Novel Intervallic Network Jamming Architecture for Music. 
It provides a way for musicians to "jam" (improvise) together over the Internet; it pioneered the concept of "virtual-time" jamming. 


## Install

### Docker

First, build the Docker image with the following command

```bash
docker build -t server/ninjam .
```
Ninjam runs on port 2049 by default.

Run the image with the following command

```bash
docker run -d -p 2049:2049 server/ninjam
```

The server will be available in port localhost:2049

### Command Line (macOS/Linux)

On your bash, run the following commands:
```bash
git clone https://github.com/r-sacchetti/ninjam.git
cd /ninjam/ninjam/server &&
make
./ninjamsrv example.cfg
```