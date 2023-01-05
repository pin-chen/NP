#! /usr/bin/python3
import os

env_variables = [
    'REQUEST_METHOD',
    'REQUEST_URI',
    'QUERY_STRING',
    'SERVER_PROTOCOL',
    'HTTP_HOST',
    'SERVER_ADDR',
    'SERVER_PORT',
    'REMOTE_ADDR',
    'REMOTE_PORT'
]

print('Content-type: text/plain', end='\r\n\r\n')

for var in env_variables:
    print(var, '=', os.getenv(var, ''))