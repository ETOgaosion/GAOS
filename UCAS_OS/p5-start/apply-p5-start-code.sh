#!/bin/bash
P5DIR=$(readlink -f "$1")

if [ ! -d "${P5DIR}" ]; then
	echo -e "\033[31mError:\033[0m ${P5DIR} not exist or not a directory"
	exit 1
fi

echo "Applying patch for ${P5DIR}"
echo "try patch old files"
PATCH_FILE="`pwd`/p5-start.patch"
cd "${P5DIR}"
patch -p0 < "${PATCH_FILE}"
echo -e "\033[32m[Done]\033[0m"
cd -

echo "copy new files"
cd p5-new-files/
for i in `find ./ -type f -name "*"`
do
	echo "copy $i into ${P5DIR}/$i"
	cp --parents -r ${i} "${P5DIR}"
done
echo -e "\033[32m[Done]\033[0m"
