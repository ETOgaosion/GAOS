#!/bin/bash
P3DIR=$(readlink -f "$1")

if [ ! -d "${P3DIR}" ]; then
	echo -e "\033[31mError:\033[0m ${P3DIR} not exist or not a directory"
	exit 1
fi
if [ ! -f "${P3DIR}/include/stdio.h" ]; then
	echo -e "\033[33mWarning:\033[0m ${P3DIR}/include/stdio.h not exist or not a directory"
	echo -n "Skip this step or Exit this script?(please input 'S' or 'E')"
	read skip_stdio
	if [ "$skip_stdio" == "E" ]; then
		echo "exit script"
		exit 1
	elif [ "$skip_stdio" != "S" ]; then
		echo -e "\033[31mError:\033[0m invalid input. Please input S or E"
		exit 1
	fi
fi

echo "Applying patch for ${P3DIR}"
echo "try patch old files"
PATCH_FILE="`pwd`/p3-start.patch"
cd "${P3DIR}"
patch -p0 < "${PATCH_FILE}"
echo -e "\033[32m[Done]\033[0m"
cd -

if [ "$skip_stdio" != "S" ]; then
	echo "try moving stdio.h into include/os"
	mv "${P3DIR}/include/stdio.h" "${P3DIR}/include/os/stdio.h"
	echo -e "\033[32m[Done]\033[0m"
	
	echo "patch stdio.h"
	patch "${P3DIR}/include/os/stdio.h" < stdio.patch
	echo -e "\033[32m[Done]\033[0m"
fi

echo "copy new files"
cd p3-new-files/
for i in `find ./ -type f -name "*"`
do
	echo "copy $i into ${P3DIR}/$i"
	cp --parents -r ${i} "${P3DIR}"
done
echo -e "\033[32m[Done]\033[0m"
