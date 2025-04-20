#!/bin/bash

PS4='\033[1;34m>>>\033[0m '

set -xeu

pip3 install conan

conan profile detect -f

std=20
profile="$(conan profile path default)"

mv "$profile" "${profile}.bak"
sed 's/^\(compiler\.cppstd=\).\{1,\}$/\1'"$std/" "${profile}.bak" > "$profile"
rm "${profile}.bak"


if [ -f conan_cache_save.tgz ]; then
  conan cache restore conan_cache_save.tgz
fi

conan remote add dpconan https://conan.diejor.tech

conan remove \* --lru=1M -c

if [[ "${RUNNER_OS:-}" == "Windows" ]]; then
  echo "=== Windows CI: forcing Dawn→D3D12 only, disabling Vulkan ==="
  conan install . -b missing \
    -s build_type=Release \
    -o dawn:USE_VULKAN=False \
    -o dawn:DAWN_ENABLE_D3D12=True
else
  echo "=== Non‑Windows: using standard conan install flags ==="
  conan install . -b missing
fi

conan cache save '*/*:*' --file=conan_cache_save.tgz
