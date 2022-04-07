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
[ -n "$ssrcloud_clash" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
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

function browser_download {
  echo
  printf    "  download [1] to \e[4m%s\e[0m\n" "$2"
  echo
  echo      "  [1]: $1"
  echo
  read -rp  "  done? "
  echo
}

function update_clash {

  local uri="$1_clash"
  browser_download "${!uri}" "/tmp/raw.yaml"
  diff -u --color=always {"$DOTDIR/$1",/tmp}/raw.yaml || echo

  cd /tmp || exit 1
  read -rp "clash_run will convert yaml as follows ... "
  echo
  clash_convert <raw.yaml | less -SRM +%
  echo
  read -rp 'ok? '
  echo

  cd "$DOTDIR/$1" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  mv -fv raw.yaml "raw.yaml.$(date --iso-8601=minute).$(uuidgen)"
  cp -v /tmp/raw.yaml raw.yaml
  echo

}

function update_shadowrocket {

  local uri="$1_shadowrocket"
  browser_download "${!uri}" "/tmp/shadowrocket_0_base64"

  cd /tmp || exit 1
  rm -fv shadowrocket_{1..9}*
  base64 -d <shadowrocket_0_base64 >shadowrocket_1_percent
  # https://unix.stackexchange.com/questions/159253/decoding-url-encoding-percent-encoding
  sed -E -e 's,%([0-9A-F][0-9A-F]),\\x\1,g' shadowrocket_1_percent | xargs -0 printf "%b" >shadowrocket_2_raw
  sed -E -i \
    -e 's,#云 - ,#,g' \
    -e 's| +|.|g' \
    shadowrocket_2_raw
  base64 -w0 shadowrocket_2_raw >shadowrocket_3_rebase64
  mv -v shadowrocket_3_rebase64 /home/darren/cgi/cgi-tmp/shadowrocket_3_rebase64
  echo
  local POTATSO="http://192.168.0.223/cgi-tmp/shadowrocket_3_rebase64"
  echo "$POTATSO"
  echo
  qrencode -tUTF8 "$POTATSO"
  echo

  # [apple hortcuts]
  # Scan QR/Bar Code
  # Get text from QR/Bar Code
  # Copy Text to clipboard

}

{

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

      echo
      update_clash "$1"
      update_shadowrocket "$1"
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

}
