import assert from 'assert'

import lazy { var1 } from './export_api'
import lazy { var1 as var1Alias} from './export_api'
import lazy { default as maxFunc1 } from './export_api'
import lazy { var2, var3, } from './export_api'
import lazy { var2 as var2Alias, var3 as var3lias, } from './export_api'


import lazy {type typeAlias1 } from './export_type'
import lazy {type typeAlias2 as  typeAlias2_new} from './export_type'
import lazy {type default as typeAlias_new4 } from './export_type'
import lazy { as, type typeAlias3} from './export_type'
import lazy {typeAlias4, type typeAlias5 as  type5_new} from './export_type'

import lazy from './export_api'
import {lazy} from './export_api'


import "";
import ''
import './export_api'
