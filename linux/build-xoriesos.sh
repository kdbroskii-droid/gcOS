#!/bin/sh
set -eu

cd "$(dirname "$0")"

mkdir -p build-output
rm -rf config binary chroot cache .build

sh auto/config

lb build

ISO="$(find . -maxdepth 1 -type f -name '*.hybrid.iso' -print -quit)"

if [ -z "$ISO" ]; then
  echo "ERROR: live-build did not produce an ISO."
  exit 1
fi

rm -f build-output/xoriesOS.iso
mv "$ISO" build-output/xoriesOS.iso

SIZE_BYTES="$(stat -c '%s' build-output/xoriesOS.iso)"
MAX_BYTES=$((2 * 1024 * 1024 * 1024))

echo "xoriesOS ISO size: $SIZE_BYTES bytes"

if [ "$SIZE_BYTES" -gt "$MAX_BYTES" ]; then
  echo "ERROR: xoriesOS ISO is larger than 2 GiB."
  exit 1
fi

echo "xoriesOS ISO is within the 2 GiB limit."
