#!/bin/bash -e
# (1) Edit this file in /home/darren/clash/clash_misc/clash.sh
# (2) make clean
# (3) make install_sh

# Deps
# pacman -Syu --needed unzip wget

# export PATH="$PATH:/home/darren/.clash/bin"

countrydb='Country.mmdb'
zip='yacd-gh-pages.zip'

dotdir="/home/darren/.clash"
yCdir="/home/darren/.clash/yC"

source /home/darren/.clash/uri.rc
test -n "$ssrcloud_uri"
# test -n "$rixcloud_uri"

function clear_tmp {
  pushd /tmp
  rm -fv "$zip" || true
  rm -fv "$countrydb" || true
  rm -fv config*.yaml || true
  rm -fv General.yml || true
  rm -fv rules.yml || true
  rm -fv clash.yml || true
  popd
}

function handler {
  echo 'SIGINT'
  exit 1
}
trap handler SIGINT

cd /tmp

if [ "$(basename "$0")" = "clash_run" ]; then

  test "$#" -eq 0
  set -- "ssrcloud"

  # Prepare
  if [ "$#" -ne 1 ]; then
    echo -e "\n  $(basename "$0") <provider>\n" 
    exit 1
  fi
  if [ ! -e "$dotdir/$1" ]; then
    echo -e "\n  '$dotdir/$1' does not exist\n"
    exit 1
  fi

  clear_tmp
  cd /tmp

  rm -rf yacd-gh-pages || true
  unzip "$yCdir/$zip"

  cd "$dotdir/$1"
  rm -fv "./$countrydb" || true
  cp -v "$yCdir/$countrydb" ./
  exec clash -d .

  exit 0

fi

if [ "$(basename "$0")" = "clash_update" ]; then

  if [ "$#" -ne 1 ]; then
    echo
    echo "  $(basename "$0") yC ($zip & $countrydb)"
    echo "  $(basename "$0") <provider>"
    echo
    exit 1
  fi

  if [ "$1" = "yC" ]; then

    # Exit on connection failure
    clear_tmp
    cd /tmp
    rm -rfv yacd-gh-pages || true
    wget -O "$zip" 'https://github.com/haishanh/yacd/archive/gh-pages.zip'
    wget 'https://github.com/Dreamacro/maxmind-geoip/releases/latest/download/Country.mmdb'

    mkdir -pv "$yCdir"
    cd "$yCdir"
    mv -fv "$zip" "$zip.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -fv "$countrydb" "Country.mmdb.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -v /tmp/"$zip" ./
    mv -v /tmp/"$countrydb" ./

    exit 0

  fi

  # if [[ "$1" = "rixcloud" || "$1" = "ssrcloud" ]]; then
  if [[ "$1" = "ssrcloud" ]]; then

    uri="$1_uri"

    clear_tmp
    echo
    echo -n "  Please download ${!uri} to /tmp/raw.yaml "
    read -r

    cd /tmp
    convert.out <raw.yaml >config.yaml
    less -SRM +% config.yaml
    echo -n 'ok? '
    read -r

    cd "$dotdir/$1"
    mv -v config.yaml "config.yaml.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -v /tmp/config.yaml config.yaml

    exit 0

  fi

  echo "Invalid target '$1'"
  exit 1

fi

echo "Please don't call me directly."
exit 1
