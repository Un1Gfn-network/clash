#!/bin/bash
# Edit this file in ~/clash/clash_misc/clash.sh

# # shellcheck -x ~/clash/clash_misc/clash.sh
# pushd ~/clash/build/clash_misc/ && make clean all && sudo make uninstall install && popd

# Deps
# pacman -Syu --needed unzip wget

CTDBURL="https://github.com/Dreamacro/maxmind-geoip/releases/latest/download/Country.mmdb"
CTDB="Country.mmdb"

ZIPURL="https://github.com/haishanh/yacd/archive/gh-pages.zip"
ZIP="yacd-gh-pages.zip"
ZIPROOT="yacd-gh-pages"

# https://github.com/Dreamacro/clash-dashboard
# ZIPURL="https://codeload.github.com/Dreamacro/clash-dashboard/zip/refs/heads/gh-pages"
# ZIPURL="https://github.com/Dreamacro/clash-dashboard/archive/gh-pages.zip"
# ZIP="clash-dashboard-gh-pages.zip"
# ZIPROOT="clash-dashboard-gh-pages"

DOTDIR=~/.clash
YCDIR=~/.clash/yC

# shellcheck source=/home/darren/.clash/uri.rc
source ~/.clash/uri.rc
[ -n "$ssrcloud_uri" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
# [ -n "$rixcloud_uri" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

function clear_tmp {
  pushd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  rm -fv "$ZIP" || true
  rm -fv "$CTDB" || true
  rm -fv config*.yaml || true
  rm -fv General.yml || true
  rm -fv rules.yml || true
  rm -fv clash.yml || true
  popd || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
}

cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

case "$(basename "$0")" in

"clash_run")

  # Prepare
  # if [ "$#" -ne 1 ]; then
  #   echo -e "\n  $(basename "$0") <provider>\n"
  #   exit 1
  # fi
  [ "$#" -eq 0 ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  set -- "ssrcloud"
  [ -e "$DOTDIR/$1" ] || { echo -e "\n  '$DOTDIR/$1' does not exist\n"; exit 1; }

  clear_tmp
  cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

  rm -rf "$ZIPROOT"
  unzip "$YCDIR/$ZIP"

  cd "$DOTDIR/$1" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  rm -fv "./$CTDB"
  cp -v "$YCDIR/$CTDB" ./
  exec clash -d .

  echo "should not reach here"
  { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  ;;

"clash_update")

  if [ "$#" -ne 1 ]; then
    echo
    echo "  $(basename "$0") yC ($ZIP & $CTDB)"
    echo "  $(basename "$0") <provider>"
    echo
    exit 1
  fi

  case "$1" in

  "yC")

    # Exit on connection failure
    clear_tmp
    cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    rm -rfv "$ZIPROOT" || true
    wget -O "$ZIP" "$ZIPURL"
    wget "$CTDBURL"

    mkdir -pv "$YCDIR"
    cd "$YCDIR" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    mv -fv "$ZIP" "$ZIP.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -fv "$CTDB" "Country.mmdb.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -v /tmp/"$ZIP" ./
    mv -v /tmp/"$CTDB" ./

    exit 0
    ;;

  # "rixcloud"|"ssrcloud")
  "ssrcloud")

    uri="$1_uri"

    clear_tmp
    echo
    echo -n "  Please download ${!uri} to /tmp/raw.yaml "
    read -r
    echo

    cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    clash_convert <raw.yaml >config.yaml
    less -SRM +% config.yaml
    echo -n 'ok? '
    read -r

    cd "$DOTDIR/$1" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    mv -v config.yaml "config.yaml.$(date --iso-8601=minute).$(uuidgen)" || true
    mv -v /tmp/config.yaml config.yaml

    exit 0
    ;;

  *)

    echo "Invalid target '$1'"
    exit 1
    ;;

  esac

  ;;

*)

  echo "Please don't call me directly."
  { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

  ;;

esac
