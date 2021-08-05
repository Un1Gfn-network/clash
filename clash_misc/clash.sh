#!/bin/bash
# Edit this file in ~/clash/clash_misc/clash.sh

# # shellcheck -x ~/clash/clash_misc/clash.sh
# pushd ~/clash/build/clash_misc/ && make clean all && sudo make uninstall install && popd

# Deps
# pacman -Syu --needed unzip wget

# shellcheck disable=SC2034
CTDBURL=https://github.com/Dreamacro/maxmind-geoip/releases/latest/download/Country.mmdb
CTDB=Country.mmdb

# shellcheck disable=SC2034
ZIPURL=https://github.com/haishanh/yacd/archive/gh-pages.zip
ZIP=yacd-gh-pages.zip
ZIPROOT=yacd-gh-pages

# # https://github.com/Dreamacro/clash-dashboard
# # ZIPURL="https://github.com/Dreamacro/clash-dashboard/archive/gh-pages.zip"
# ZIPURL="https://codeload.github.com/Dreamacro/clash-dashboard/zip/refs/heads/gh-pages"
# ZIP="clash-dashboard-gh-pages.zip"
# ZIPROOT="clash-dashboard-gh-pages"

DOTDIR=~/.clash
YCDIR=~/.clash/yC
MERGEDIR=/tmp/clash
RAW=raw.yaml

# shellcheck source=/home/darren/.clash/uri.rc
source ~/.clash/uri.rc
[ -n "$ssrcloud_uri" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
# [ -n "$rixcloud_uri" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

function clear_tmp {
  pushd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  # https://www.shellcheck.net/wiki/SC2144
  for i in config*.yaml; do
    [ "$i" = config\*.yaml ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  done
  { /bin/false \
    || [ -e General.yml ] \
    || [ -e rules.yml ] \
    || [ -e clash.yml ] \
  ; } && { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  rm -rfv "$ZIPROOT"  # clash_run
  rm -rfv "$ZIP"      # clash_update yC
  rm -rfv "$CTDB"     # clash_update yC
  rm -rfv "$MERGEDIR" # clash_run
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
  echo

  [ "$#" -eq 0 ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  set -- "ssrcloud"
  [ -e "$DOTDIR/$1" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

  clear_tmp
  echo

  cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  rm -rf "$ZIPROOT"
  unzip "$YCDIR/$ZIP"
  echo

  [ -e "$MERGEDIR" ] && { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  mkdir -v "$MERGEDIR"
  [ -e "$DOTDIR/$1/$CTDB" ] && { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  cp -v "$YCDIR/$CTDB" "$MERGEDIR/$CTDB"
  echo
  clash_convert <"$DOTDIR/$1/$RAW" >"$MERGEDIR/config.yaml" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  echo

  exec clash -d "$MERGEDIR"

  # Should not reach here
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
    mkdir -pv "$YCDIR"
    echo
    for i in ZIP CTDB; do
      j="${i}URL"
      [ -e "/tmp/${!i}" ] && { rm -v "/tmp/${!i}"; echo; }
      cd "$YCDIR/" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
      wget -O "/tmp/${!i}" "${!j}" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
      mv -v "${!i}" "${!i}.$(date --iso-8601=minute).$(uuidgen)" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
      mv -v "/tmp/${!i}" "${!i}" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
      echo
    done
    exit 0
    ;;

  # "rixcloud"|"ssrcloud")
  "ssrcloud")

    uri="$1_uri"

    echo
    read -erp "  Please download ${!uri} to /tmp/raw.yaml "
    echo

    cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    read -erp "clash_run will convert yaml as follows ... "
    echo
    clash_convert <raw.yaml | less -SRM +%
    # echo
    read -erp 'ok? '
    echo

    cd "$DOTDIR/$1" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    mv -fv raw.yaml "raw.yaml.$(date --iso-8601=minute).$(uuidgen)"
    cp -v /tmp/raw.yaml raw.yaml
    echo

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
