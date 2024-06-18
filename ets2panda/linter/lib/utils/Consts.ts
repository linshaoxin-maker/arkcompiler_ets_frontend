/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export const ALLOWED_STD_SYMBOL_API = ['iterator'];

export const ARKTS_IGNORE_DIRS_NO_OH_MODULES = ['node_modules', 'build', '.preview'];

export const ARKTS_IGNORE_DIRS_OH_MODULES = 'oh_modules';

export const ARKTS_IGNORE_DIRS = [...ARKTS_IGNORE_DIRS_NO_OH_MODULES, ARKTS_IGNORE_DIRS_OH_MODULES];

export const ARKTS_IGNORE_FILES = ['hvigorfile.ts'];

export const ES_OBJECT = 'ESObject';

export const FUNCTION_HAS_NO_RETURN_ERROR_CODE = 2366;

export const LIMITED_STANDARD_UTILITY_TYPES = [
  'Uncapitalize',
  'Capitalize',
  'Lowercase',
  'Uppercase',
  'ThisType',
  'OmitThisParameter',
  'ThisParameterType',
  'InstanceType',
  'ReturnType',
  'ConstructorParameters',
  'Parameters',
  'NonNullable',
  'Extract',
  'Exclude',
  'Omit',
  'Pick',
  'Awaited'
];

export const LIMITED_STD_GLOBAL_FUNC = ['eval'];

export const LIMITED_STD_OBJECT_API = [
  '__proto__',
  '__defineGetter__',
  '__defineSetter__',
  '__lookupGetter__',
  '__lookupSetter__',
  'assign',
  'create',
  'defineProperties',
  'defineProperty',
  'freeze',
  // was relaxed but revert
  'fromEntries',
  'getOwnPropertyDescriptor',
  'getOwnPropertyDescriptors',
  'getOwnPropertySymbols',
  'getPrototypeOf',
  'hasOwnProperty',
  'is',
  'isExtensible',
  'isFrozen',
  'isPrototypeOf',
  'isSealed',
  'preventExtensions',
  'propertyIsEnumerable',
  'seal',
  'setPrototypeOf'
];

export const LIMITED_STD_PROXYHANDLER_API = [
  'apply',
  'construct',
  'defineProperty',
  'deleteProperty',
  'get',
  'getOwnPropertyDescriptor',
  'getPrototypeOf',
  'has',
  'isExtensible',
  'ownKeys',
  'preventExtensions',
  'set',
  'setPrototypeOf'
];

export const LIMITED_STD_REFLECT_API = [
  'apply',
  'construct',
  'defineProperty',
  'deleteProperty',
  'getOwnPropertyDescriptor',
  'getPrototypeOf',
  'isExtensible',
  'preventExtensions',
  'setPrototypeOf'
];

export const NON_INITIALIZABLE_PROPERTY_DECORATORS = ['Link', 'Consume', 'ObjectLink', 'Prop', 'BuilderParam'];

export const NON_INITIALIZABLE_PROPERTY_CLASS_DECORATORS = ['CustomDialog'];

export const NON_RETURN_FUNCTION_DECORATORS = ['AnimatableExtend', 'Builder', 'Extend', 'Styles'];

export const PROPERTY_HAS_NO_INITIALIZER_ERROR_CODE = 2564;

export const SENDABLE_DECORATOR = 'Sendable';

export const USE_SHARED = 'use shared';

export const STANDARD_LIBRARIES = [
  'lib.es5.d.ts',
  'lib.es2015.iterable.d.ts',
  'lib.es2015.generator.d.ts',
  'lib.es2015.proxy.d.ts',
  'lib.es2015.promise.d.ts',
  'lib.es2015.symbol.wellknown.d.ts',
  'lib.es2015.symbol.d.ts',
  'lib.es2015.reflect.d.ts',
  'lib.es2015.collection.d.ts',
  'lib.es2015.core.d.ts',
  'lib.es2016.array.include.d.ts',
  'lib.es2017.sharedmemory.d.ts',
  'lib.es2017.typedarrays.d.ts',
  'lib.es2017.string.d.ts',
  'lib.es2017.intl.d.ts',
  'lib.es2017.object.d.ts',
  'lib.es2018.intl.d.ts',
  'lib.es2018.regexp.d.ts',
  'lib.es2018.asyncgenerator.d.ts',
  'lib.es2018.asynciterable.d.ts',
  'lib.es2018.promise.d.ts',
  'lib.es2019.string.d.ts',
  'lib.es2019.array.d.ts',
  'lib.es2019.symbol.d.ts',
  'lib.es2019.object.d.ts',
  'lib.es2019.intl.d.ts',
  'lib.es2020.bigint.d.ts',
  'lib.es2020.symbol.wellknown.d.ts',
  'lib.es2020.string.d.ts',
  'lib.es2020.sharedmemory.d.ts',
  'lib.es2020.promise.d.ts',
  'lib.es2020.number.d.ts',
  'lib.es2020.intl.d.ts',
  'lib.es2020.date.d.ts',
  'lib.es2021.intl.d.ts',
  'lib.es2021.weakref.d.ts',
  'lib.es2021.string.d.ts',
  'lib.es2021.promise.d.ts',
  'lib.es2022.regexp.d.ts',
  'lib.es2022.array.d.ts',
  'lib.es2022.string.d.ts',
  'lib.es2022.error.d.ts',
  'lib.es2022.object.d.ts',
  'lib.es2022.sharedmemory.d.ts',
  'lib.es2022.intl.d.ts',
  'lib.es2023.array.d.ts',
  'lib.dom.iterable.d.ts',
  'lib.dom.d.ts',
  'lib.webworker.importscripts.d.ts',
  'lib.webworker.iterable.d.ts',
  'lib.webworker.d.ts',
  'lib.decorators.legacy.d.ts',
  'lib.decorators.d.ts',
  'lib.scripthost.d.ts'
];

export const ARKTS_COLLECTIONS_D_ETS = '@arkts.collections.d.ets';

export const COLLECTIONS_NAMESPACE = 'collections';

export const ARKTS_COLLECTIONS_TYPES = [
  'Array',
  'Int8Array',
  'Uint8Array',
  'Int16Array',
  'Uint16Array',
  'Int32Array',
  'Uint32Array'
];

export const ARKTS_LANG_D_ETS = '@arkts.lang.d.ets';

export const LANG_NAMESPACE = 'lang';

export const ISENDABLE_TYPE = 'ISendable';

export const TYPED_ARRAYS = [
  'Int8Array',
  'Uint8ClampedArray',
  'Uint8Array',
  'Uint16Array',
  'Int16Array',
  'Uint32Array',
  'Int32Array',
  'Float64Array',
  'Float32Array',
  'BigUint64Array',
  'BigInt64Array'
];
