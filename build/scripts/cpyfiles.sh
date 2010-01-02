cd build

rm -rf tmp
mkdir tmp

cp kos.sys tmp
cp scripts/menu-$1.lst tmp/menu.lst
cp ../../tools/grub/grldr tmp
