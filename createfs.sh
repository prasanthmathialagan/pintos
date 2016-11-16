#! /bin/sh

cd /home/pintos/pintos/src/examples
make
cd /home/pintos/pintos/src/userprog
make
cd build
echo "Creating a file system.."
pintos-mkdisk filesys.dsk --filesys-size=2
echo "Formatting the file system.."
pintos -f -q
echo "Copying examples..."
pintos -p ../../examples/echo -a echo -- -q
pintos -p ../../examples/pwd -a pwd -- -q
pintos -p ../../examples/cat -a cat -- -q
pintos -p ../../examples/cp -a cp -- -q
pintos -p ../../examples/ls -a ls -- -q
pintos -p ../../examples/mkdir -a mkdir -- -q
pintos -p ../../examples/halt -a halt -- -q
