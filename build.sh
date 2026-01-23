#!/bin/bash
echo "Building DSM Game..."
echo "Compiling server..."
g++ -o server.exe server.cpp -lws2_32 -static-libgcc -static-libstdc++
if [ $? -eq 0 ]; then
    echo "✓ Server compiled successfully"
else
    echo "✗ Server compilation failed"
    exit 1
fi

echo "Compiling client..."
g++ -o client.exe client.cpp -lws2_32 -static-libgcc -static-libstdc++
if [ $? -eq 0 ]; then
    echo "✓ Client compiled successfully"
else
    echo "✗ Client compilation failed"
    exit 1
fi

echo ""
echo "Build complete! Files:"
ls -lh *.exe
