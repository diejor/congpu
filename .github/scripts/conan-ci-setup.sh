#!/bin/bash
set -xeu
PS4='\033[1;34m>>>\033[0m '

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
conan remove '*' --lru=1M -c

if [[ "${RUNNER_OS:-}" == "Windows" ]]; then
  echo "=== Windows CI: forcing D3D12, disabling Vulkan ==="
  conan install . -b missing \
    -o dawn/*:enable_vulkan=False \
    -o dawn/*:enable_d3d12=True
else
  conan install . -b missing
fi

conan cache save '*/*:*' --file=conan_cache_save.tgz
