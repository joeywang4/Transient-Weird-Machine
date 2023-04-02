echo "** Testing exception-based gates..."
cd exceptions
make clean
make
./main.elf
echo ""

echo "** Testing branch predictor-based gates..."
cd ../spectre
make clean
make
./spectre.elf
echo ""

echo "** Testing branch target buffer-based gates..."
cd ../spectrev2
make clean
make
./spectrev2.elf
echo ""

echo "** Testing TSX-based gates..."
cd ../TSX
make clean
make
./tsx.elf
echo ""

echo "** Done!"
cd ..
