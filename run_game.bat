@echo off
echo Starting Game Challenge Server...
start build\server\Release\server.exe

echo Waiting 1 second for server to initialize...
timeout /t 1 /nobreak > nul

echo Starting 3 Clients...
start build\client\Release\client.exe Alfa
start build\client\Release\client.exe Beta
start build\client\Release\client.exe Gamma

echo All processes started!
