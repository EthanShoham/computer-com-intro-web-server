@echo off
if not exist bin\server md bin\server

clang -Wall -o bin\server\server.exe main.c web-server\web_server.c web-server\web-server-internal\*.c

echo on
@echo Build Finished.
