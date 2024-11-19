#!/bin/bash
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Install typescript from third_party/typescript
# Must run this script in arkguard root directory.

# sudo apt install meson

NEXUS_REPO="${NEXUS_REPO:-nexus.bz-openlab.ru:10443}"

function do_checkout() {
    local repo=$1
    local rev=$2
    local dest=$3
    local patch=$4
    [ -n "${repo}" ] || exit 1
    [ -n "${rev}" ] || exit 1
    [ -n "${dest}" ] || exit 1
    mkdir -p "${dest}"
    pushd "${dest}" || return
        git init && git remote add origin "${repo}"
        # NOTE(titova): add repeat
        git fetch --depth 1 origin "${rev}" || {
            echo "(Some error occurred while fetching rev: ${rev}"
            exit 1
        } && {
            git checkout FETCH_HEAD || exit 1
            [ -n "${patch}" ] && git apply "${patch}"
        }
    popd >/dev/null 2>&1 || return
}

GIT_URL=https://gitee.com/openharmony-sig/arkcompiler_ets_frontend.git
DEST=koala-sig
do_checkout "${GIT_URL}" panda_rev_1 "${DEST}"

cd "${DEST}" || exit

npm config set package-lock false
npm config set strict-ssl false
# npm config set lockfile false
npm config set registry "https://repo.huaweicloud.com/repository/npm/"
npm config set @koalaui:registry "https://$NEXUS_REPO/repository/koala-npm/"
npm config set @panda:registry "https://$NEXUS_REPO/repository/koala-npm/"
npm config set @ohos:registry "https://repo.harmonyos.com/npm/"
if [ -z "${KOALA_REPO}" ] ; then
    npm config set "//$NEXUS_REPO/repository/koala-npm/:_auth=$KOALA_TOKEN"
fi

npm i

# NOTE(ttitova) need to fix it in es2panda
for config in $(find . -name 'arktsconfig*.json') ; do
# out_dir=$(awk -F ':' ' $1 ~ "outDir" { sub(/,/, "", $2) ; print $2 }' $config)
mkdir -p $(dirname "${config}")/build/abc
done

pushd incremental/tools/fast-arktsc/ || exit
npm i
popd >/dev/null 2>&1 || exit

pushd incremental/tools/panda/ || exit
if [ -z "${PANDA_SDK_TARBALL}" ] ; then
npm run panda:sdk:install
else
npm install "${PANDA_SDK_TARBALL}"
fi
popd >/dev/null 2>&1 || exit

pushd arkoala/arkui-common/ || exit
KOALA_BZ=1 npm run ohos-sdk
popd >/dev/null 2>&1 || exit

pushd arkoala-arkts || exit
npm run trivial:all:node:ci
popd >/dev/null 2>&1 || exit
