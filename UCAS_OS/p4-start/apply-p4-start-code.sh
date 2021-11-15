#!/bin/bash
P4DIR=$(readlink -f "$1")

if [ ! -d "${P4DIR}" ]; then
	echo -e "\033[31mError:\033[0m ${P4DIR} not exist or not a directory"
	exit 1
fi
if [ ! -f "${P4DIR}/tiny_libc/syscall.S" ]; then
	echo -e "\033[33mWarning:\033[0m ${P4DIR}/tiny_libc/syscall.S not exist or not a directory"
	echo -n "Skip this step or Exit this script?(please input 'S' or 'E')"
	read skip_syscalls
	if [ "$skip_syscalls" == "E" ]; then
		echo "exit script"
		exit 1
	elif [ "$skip_syscalls" != "S" ]; then
		echo -e "\033[31mError:\033[0m invalid input. Please input S or E"
		exit 1
	fi
fi
if [ ! -f "${P4DIR}/arch/riscv/kernel/head.S" ]; then
	echo -e "\033[33mWarning:\033[0m ${P4DIR}/arch/riscv/kernel/head.S not exist or not a directory"
	echo -n "Skip this step or Exit this script?(please input 'S' or 'E')"
	read skip_heads
	if [ "$skip_heads" == "E" ]; then
		echo "exit script"
		exit 1
	elif [ "$skip_heads" != "S" ]; then
		echo -e "\033[31mError:\033[0m invalid input. Please input S or E"
		exit 1
	fi
fi

echo "Applying patch for ${P4DIR}"
echo "try patch old files"
PATCH_FILE="`pwd`/p4-start.patch"
cd "${P4DIR}"
patch -p0 < "${PATCH_FILE}"
echo -e "\033[32m[Done]\033[0m"
cd -

if [ "$skip_syscalls" != "S" ]; then
	echo "try to rename syscall.S to invoke_syscall.S"
	mv "${P4DIR}/tiny_libc/syscall.S" "${P4DIR}/tiny_libc/invoke_syscall.S"
	echo -e "\033[32m[Done]\033[0m"
fi
if [ "$skip_heads" != "S" ]; then
	echo "try to copy head.S to start.S"
	cp "${P4DIR}/arch/riscv/kernel/head.S" "${P4DIR}/arch/riscv/kernel/start.S"
	sed -i "s/call main/call boot_kernel/g" "${P4DIR}/arch/riscv/kernel/head.S"
	echo -e "\033[32m[Done]\033[0m"
fi

echo "copy new files"
cd p4-new-files/
for i in `find ./ -type f -name "*"`
do
	echo "copy $i into ${P4DIR}/$i"
	cp --parents -r ${i} "${P4DIR}"
done
echo -e "\033[32m[Done]\033[0m"
