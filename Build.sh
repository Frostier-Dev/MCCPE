echo "Build start"
echo "-----------"
make clean
make
echo "-----------"
echo "Build Complete"
echo ""
echo "Start to copy"
echo "-----------"
cp minecraft_nds_e.nds /mnt/e/roms/dev/
echo "End copying"