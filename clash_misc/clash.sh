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
[ -n "$dler_clash" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

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

function clash_convert_dler {

  ((1==$#)) || { echo "${FUNCNAME[0]}: there should be one and only one parameter, which should be the file to convert"; exit 1; }

  cmp -n 22 <(cat <<____EOF | sed 's/^    //g' | head --bytes=-1
    ---
    proxies:
    - server:
____EOF
  ) "$1" || { echo "${FUNCNAME[0]}: ill-formed header in '$1'"; exit 1; }

  cat <<__EOF | sed 's/^  //g'
  port: 8080
  socks-port: 1080
  allow-lan: true
  bind-address: '*'
  mode: global
  log-level: info
  ipv6: false
  external-controller: 0.0.0.0:9090
  external-ui: /tmp/yacd-gh-pages
  secret:
  profile:
    store-selected: true
  dns:
    enable: false
  proxies:
__EOF

  tail --lines=+3 "$1" | head --lines=-1 | sed \
    -e 's,\\U0001F1E8\\U0001F1F3,\\U0001F1F9\\U0001F1FC,g' \
    -e 's,\\U0001F1ED\\U0001F1F0,\\U0001F5BE\\U00002612,g'

}

function update_dler {

  local provider=dler
  local uri="${provider}_clash"
  browser_download "${!uri}" "/tmp/raw.yaml"
  diff -u --color=always {"$DOTDIR/$provider",/tmp}/raw.yaml || echo

  cd /tmp || exit 1
  read -rp "clash_run will convert yaml as follows ... "
  echo
  # clash_convert_dler raw.yaml
  clash_convert_dler raw.yaml | less -SRM +%
  echo
  read -rp 'ok? '
  echo

  cd "$DOTDIR/$provider" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  mv -fv raw.yaml "raw.yaml.$(date --iso-8601=minute).$(uuidgen)"
  cp -v /tmp/raw.yaml raw.yaml
  echo

}

function update_ssrcloud {

  local provider=ssrcloud
  local uri="${provider}_clash"
  browser_download "${!uri}" "/tmp/raw.yaml"
  diff -u --color=always {"$DOTDIR/$provider",/tmp}/raw.yaml || echo

  cd /tmp || exit 1
  read -rp "clash_run will convert yaml as follows ... "
  echo
  clash_convert <raw.yaml | less -SRM +%
  echo
  exit 1
  read -rp 'ok? '
  echo

  cd "$DOTDIR/$provider" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
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

function clash_run {

  # Prepare
  # if [ "$#" -ne 1 ]; then
  #   echo -e "\n  $(basename "$0") <provider>\n"
  #   exit 1
  # fi
  echo

  # set -- "ssrcloud"
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
  case "$1" in
  "dler")
    clash_convert_dler "$DOTDIR/$1/$RAW" >"$MERGEDIR/config.yaml" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  ;;
  "ssrcloud")
    clash_convert <"$DOTDIR/$1/$RAW" >"$MERGEDIR/config.yaml" || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  ;;
  *)
    { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
  ;;
  esac
  echo

  # shellcheck disable=SC2093
  exec clash -d "$MERGEDIR"

  # Should not reach here
  { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
}

{

  cd /tmp || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }

  case "$(basename "$0")" in

  "clash_run")
    ((1==$#)) || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    [ -e "$DOTDIR/$1" ] || { echo "${BASH_SOURCE[0]}:$LINENO:${FUNCNAME[0]}: err"; exit 1; }
    clash_run "$1"
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

    "ssrcloud")
      echo
      update_ssrcloud
      update_shadowrocket ssrcloud
      exit 0
      ;;

    "dler")
      echo
      update_dler
      # update_shadowrocket dler
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
