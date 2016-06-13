# Loader

loader downloads file over http and send them to the storage over socket

required: python3.5

run: ./loader.py

# Storage

storage gets data from socket and store it to the files

required: gcc-5.3

build: make

run: make run
