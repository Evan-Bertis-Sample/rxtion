# qb.sh -- Quick Build & Test, using MINGW
echo "Quick Build & Test, using MINGW"
bash ./proc/win/mingw.sh
echo ""
echo "Running..."
gdb ./bin/rxtion.exe