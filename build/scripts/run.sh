debug=""
if [ "$1" = "debug" ]; then
	debug="-s -S";
fi

echo $debug

qemu $debug -m 32 -serial file:kos.log -L ../tools/qemu -cdrom build/kos.iso