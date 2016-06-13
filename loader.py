#!/bin/env python3

import sys
import os

import struct
import asyncio
import signal

import urllib.request
import urllib.parse

sem = asyncio.Semaphore(2)

read_buffer_size = 1024

@asyncio.coroutine
def download(url, host, port):
    # url = 'https://www.python.org/static/opengraph-icon-200x200.png'
    splitted = urllib.parse.urlsplit(url)
    (_, filename) = os.path.split(splitted.path)
    try:
        with (yield from sem):
            with urllib.request.urlopen(url) as response:

                reader, writer = yield from asyncio.open_connection(host, port, loop=loop)
                
                writer.write(struct.pack('h', len(filename)))
                writer.write(filename.encode())
                
                while True:
                    data = response.read(read_buffer_size)
                    if len(data) == 0:
                        break
                    writer.write(data)
                    if writer.transport._conn_lost:
                        break
                    yield from asyncio.sleep(0.01)
                    
                print('download complete:', url)
    except Exception as e:
        print(e)
        # import traceback
        # traceback.print_tb(e.__traceback__)
    else:
        writer.close()


@asyncio.coroutine
def read_input(host, port):
    while True:
        print('enter url or "exit": ', end='', flush=True)
        s = yield from loop.run_in_executor(None, sys.stdin.readline)
        s = s.replace('\r', '').replace('\n', '').strip()
        
        if s == '':
            pass
        elif s == 'exit':
            break
        else:
            asyncio.ensure_future(download(s, host, port))
        
        yield from asyncio.sleep(0.01)


def stop(*args):
    for t in asyncio.Task.all_tasks():
        print(t.cancel())
    print(asyncio.Task.all_tasks())
    loop.stop()


if __name__ == '__main__':

    import argparse
    parser = argparse.ArgumentParser(description='loader')
    parser.add_argument('--ip', type=str, help='storage service ip address', default='127.0.0.1')
    parser.add_argument('--port', type=int, help='storage service port', default='4001')
    args = parser.parse_args()
    host = args.ip
    port = args.port

    signal.signal(signal.SIGINT, stop)
    loop = asyncio.get_event_loop()

    read_task = asyncio.ensure_future(read_input(host, port))
    try:
        loop.run_until_complete(read_task)
    finally:
        loop.close()
