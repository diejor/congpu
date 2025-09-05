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

ensure_recipe() {
  local name="$1"
  local version="$2"
  local repo="https://github.com/diejor/conan-${name}.git"
  local ref="${name}/${version}"
  if ! conan download "${ref}" --recipe -r=dpconan; then
    if git ls-remote "${repo}" &>/dev/null; then
      echo "Falling back to local recipe for ${ref}"
      tmpdir=$(mktemp -d)
      git clone --depth=1 "${repo}" "${tmpdir}"
      conan export "${tmpdir}"
      rm -rf "${tmpdir}"
    else
      echo "No local recipe for ${ref}, skipping"
    fi
  fi
}

grep -Eo 'self\.requires\("[^"/]+/[^"/]+"\)' conanfile.py \
  | sed -E 's/self\.requires\("([^"/]+)\/([^"/]+)"\)/\1 \2/' \
  | while read -r name version; do
    ensure_recipe "$name" "$version"
  done

if [[ "${RUNNER_OS:-}" == "Windows" ]]; then
  echo "=== Windows CI: forcing D3D12, disabling Vulkan ==="
  conan install . -b missing \
    -o dawn/*:force_vulkan=False \
    -o dawn/*:force_d3d12=True \
    -o dawn/*:force_system_component_load=True
else
  conan install . -b missing
fi

conan cache save '*/*:*' --file=conan_cache_save.tgz
