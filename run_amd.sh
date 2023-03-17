echo "** Testing exception-based gates..."
cd exceptions
make clean
make amd
./main-amd.elf
echo ""

echo "** Done!"
cd ..
