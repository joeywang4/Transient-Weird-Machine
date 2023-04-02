echo "** Testing ARM gates..."
echo "** This can take several minutes to complete..."
cd ARM
make clean
make
./arm.elf
echo ""

echo "** Done!"
