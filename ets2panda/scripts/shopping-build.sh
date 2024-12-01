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

git clone https://gitee.com/openharmony-sig/arkcompiler_ets_frontend.git koala-sig -b shopping
cd koala-sig

npm config set package-lock false
npm config set strict-ssl false
# npm config set lockfile false
npm config set registry https://repo.huaweicloud.com/repository/npm/
npm config set @koalaui:registry "https://$NEXUS_REPO/repository/koala-npm/"
npm config set @panda:registry "https://$NEXUS_REPO/repository/koala-npm/"
npm config set @ohos:registry https://repo.harmonyos.com/npm/
npm config set "//$NEXUS_REPO/repository/koala-npm/:_auth='a29hbGEtcHViOnkzdCFuMHRoZXJQ'"

npm i

pushd incremental/tools/panda/
PANDA_SDK_VERSION=1.5.0-dev.5526 npm run panda:sdk:install
popd

pushd arkoala/arkui-common/
KOALA_BZ=1 npm run ohos-sdk
popd

pushd arkoala-arkts/arkui
npm run compile:arkts:unmemoized
popd

pushd arkoala-arkts/loader
npm run compile:ets
# npm run compile:arkts
popd
