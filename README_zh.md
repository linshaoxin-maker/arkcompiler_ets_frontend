# ts2abc组件<a name="ZH-CN_TOPIC_0000001137330686"></a>

- [ts2abc组件<a name="ZH-CN_TOPIC_0000001137330686"></a>](#ts2abc组件)
  - [简介<a name="section11660541593"></a>](#简介)
  - [目录<a name="section161941989596"></a>](#目录)
  - [编译构建<a name="section0446154755015"></a>](#编译构建)
    - [使用说明<a name="section33105542504"></a>](#使用说明)
  - [相关仓<a name="section1371113476307"></a>](#相关仓)

## 简介<a name="section11660541593"></a>

ts2abc组件是方舟运行时子系统的前端工具，支持将JavaScript文件转换为方舟字节码文件。

更多信息请参考：[方舟运行时子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/ARK-Runtime-Subsystem-zh.md)

## 目录<a name="section161941989596"></a>

```
/ark/ts2abc/
├── test262            # Test262测试配置和运行脚本
├── ts2panda
    ├── doc            # 文档
    ├── scripts        # 依赖的脚本
    ├── src            # 源码存放目录
    ├── templates      # ruby模板文件
    ├── tests          # UT单元测试目录
    ├── tools          # ts2abc提供的工具
    └── ts2abc         # abc文件生成相关
```

## 编译构建<a name="section0446154755015"></a>

ts2abc组件采用命令行交互方式，支持将JavaScript代码转换为方舟字节码文件，使其能够在方舟运行时上运行。支持Windows/Linux/MacOS平台。方舟前端工具在linux平台上可通过全量编译或指定编译前端工具链获取。

```
$ ./build.sh --product-name Hi3516DV300 --build-target ark_ts2abc
```

### 使用说明<a name="section33105542504"></a>

安装`node`和`npm`

使用ts2abc组件将JavaScript文件转换为方舟字节码文件

```
$ cd out/ohos-arm-release/clang_x64/ark/ark/build
$ npm install
$ node --expose-gc src/index.js [options] file.js
```

当不输入任何option参数时，默认生成方舟二进制文件。

<a name="table2035444615598"></a>
<table><thead align="left"><tr id="row535415467591"><th class="cellrowborder" valign="top" width="12.898710128987101%" id="mcps1.1.6.1.1"><p id="p13354134619595"><a name="p13354134619595"></a><a name="p13354134619595"></a>选项</p>
</th>
<th class="cellrowborder" valign="top" width="6.869313068693131%" id="mcps1.1.6.1.2"><p id="p1584312189018"><a name="p1584312189018"></a><a name="p1584312189018"></a>缩写</p>
</th>
<th class="cellrowborder" valign="top" width="19.33806619338066%" id="mcps1.1.6.1.3"><p id="p157281281906"><a name="p157281281906"></a><a name="p157281281906"></a>描述</p>
</th>
<th class="cellrowborder" valign="top" width="25.82741725827417%" id="mcps1.1.6.1.4"><p id="p103276335016"><a name="p103276335016"></a><a name="p103276335016"></a>取值范围</p>
</th>
<th class="cellrowborder" valign="top" width="35.066493350664935%" id="mcps1.1.6.1.5"><p id="p1835494695915"><a name="p1835494695915"></a><a name="p1835494695915"></a>默认值</p>
</th>
</tr>
</thead>
<tbody><tr id="row1435412465598"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p881325510017"><a name="p881325510017"></a><a name="p881325510017"></a>--modules</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p148431189013"><a name="p148431189013"></a><a name="p148431189013"></a>-m</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p072882813015"><a name="p072882813015"></a><a name="p072882813015"></a>按照module模式编译</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p10327833305"><a name="p10327833305"></a><a name="p10327833305"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p076075115014"><a name="p076075115014"></a><a name="p076075115014"></a>-</p>
</td>
</tr>
<tr id="row3355346105920"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p163552462595"><a name="p163552462595"></a><a name="p163552462595"></a>--debug-log</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p48431918607"><a name="p48431918607"></a><a name="p48431918607"></a>-l</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p127284281905"><a name="p127284281905"></a><a name="p127284281905"></a>使能log信息</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p93278335012"><a name="p93278335012"></a><a name="p93278335012"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p1976019511306"><a name="p1976019511306"></a><a name="p1976019511306"></a>-</p>
</td>
</tr>
<tr id="row9355174675912"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p6355104616592"><a name="p6355104616592"></a><a name="p6355104616592"></a>--dump-assembly</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p20843161819020"><a name="p20843161819020"></a><a name="p20843161819020"></a>-a</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p187287280015"><a name="p187287280015"></a><a name="p187287280015"></a>输出为汇编文件</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p932819331104"><a name="p932819331104"></a><a name="p932819331104"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p1475975114013"><a name="p1475975114013"></a><a name="p1475975114013"></a>-</p>
</td>
</tr>
<tr id="row53551046175917"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p13575501218"><a name="p13575501218"></a><a name="p13575501218"></a>--debug</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p48431818104"><a name="p48431818104"></a><a name="p48431818104"></a>-d</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p1372811281608"><a name="p1372811281608"></a><a name="p1372811281608"></a>携带debug信息</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p133287335020"><a name="p133287335020"></a><a name="p133287335020"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p37585513019"><a name="p37585513019"></a><a name="p37585513019"></a>-</p>
</td>
</tr>
<tr id="row8355204635911"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p657125010117"><a name="p657125010117"></a><a name="p657125010117"></a>--show-statistics</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p98433181905"><a name="p98433181905"></a><a name="p98433181905"></a>-s</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p77281528704"><a name="p77281528704"></a><a name="p77281528704"></a>显示字节码相关的统计信息</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p83281633208"><a name="p83281633208"></a><a name="p83281633208"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p17580511404"><a name="p17580511404"></a><a name="p17580511404"></a>-</p>
</td>
</tr>
<tr id="row6355124665910"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p105611505114"><a name="p105611505114"></a><a name="p105611505114"></a>--output</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p1884310183014"><a name="p1884310183014"></a><a name="p1884310183014"></a>-o</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p20728192819015"><a name="p20728192819015"></a><a name="p20728192819015"></a>输出文件路径</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p1332810331508"><a name="p1332810331508"></a><a name="p1332810331508"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p157577519014"><a name="p157577519014"></a><a name="p157577519014"></a>-</p>
</td>
</tr>
<tr id="row235584610599"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p95515501012"><a name="p95515501012"></a><a name="p95515501012"></a>--timeout</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p1684312184012"><a name="p1684312184012"></a><a name="p1684312184012"></a>-t</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p37282028600"><a name="p37282028600"></a><a name="p37282028600"></a>超时门限</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p133281033804"><a name="p133281033804"></a><a name="p133281033804"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p675665112019"><a name="p675665112019"></a><a name="p675665112019"></a>-</p>
</td>
</tr>
<tr id="row135584635915"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p4551501217"><a name="p4551501217"></a><a name="p4551501217"></a>--opt-log-level</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p1843181819011"><a name="p1843181819011"></a><a name="p1843181819011"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p157285282020"><a name="p157285282020"></a><a name="p157285282020"></a>指定编译优化log等级</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p1532819334016"><a name="p1532819334016"></a><a name="p1532819334016"></a>['debug', 'info', 'error', 'fatal']</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p475510516018"><a name="p475510516018"></a><a name="p475510516018"></a>error</p>
</td>
</tr>
<tr id="row133555461596"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p3541550416"><a name="p3541550416"></a><a name="p3541550416"></a>--opt-level</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p148441518404"><a name="p148441518404"></a><a name="p148441518404"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p27281728502"><a name="p27281728502"></a><a name="p27281728502"></a>指定编译优化等级</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p832833312018"><a name="p832833312018"></a><a name="p832833312018"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p1975514517020"><a name="p1975514517020"></a><a name="p1975514517020"></a>1</p>
</td>
</tr>
<tr id="row23556463595"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p135313506120"><a name="p135313506120"></a><a name="p135313506120"></a>--help</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p168448187012"><a name="p168448187012"></a><a name="p168448187012"></a>-h</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p97284281607"><a name="p97284281607"></a><a name="p97284281607"></a>帮助提示</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p43281335010"><a name="p43281335010"></a><a name="p43281335010"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p57545511102"><a name="p57545511102"></a><a name="p57545511102"></a>-</p>
</td>
</tr>
<tr id="row5356124655916"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p185311501910"><a name="p185311501910"></a><a name="p185311501910"></a>--bc-version</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p6844141810019"><a name="p6844141810019"></a><a name="p6844141810019"></a>-v</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p1872818281006"><a name="p1872818281006"></a><a name="p1872818281006"></a>输出当前字节码版本</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p73281733408"><a name="p73281733408"></a><a name="p73281733408"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p77537511606"><a name="p77537511606"></a><a name="p77537511606"></a>-</p>
</td>
</tr>
<tr id="row1335654635915"><td class="cellrowborder" valign="top" width="12.898710128987101%" headers="mcps1.1.6.1.1 "><p id="p175213504115"><a name="p175213504115"></a><a name="p175213504115"></a>--bc-min-version</p>
</td>
<td class="cellrowborder" valign="top" width="6.869313068693131%" headers="mcps1.1.6.1.2 "><p id="p384481811016"><a name="p384481811016"></a><a name="p384481811016"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="19.33806619338066%" headers="mcps1.1.6.1.3 "><p id="p20729728003"><a name="p20729728003"></a><a name="p20729728003"></a>输出当前支持的最低字节码版本</p>
</td>
<td class="cellrowborder" valign="top" width="25.82741725827417%" headers="mcps1.1.6.1.4 "><p id="p4328533205"><a name="p4328533205"></a><a name="p4328533205"></a>-</p>
</td>
<td class="cellrowborder" valign="top" width="35.066493350664935%" headers="mcps1.1.6.1.5 "><p id="p175385118014"><a name="p175385118014"></a><a name="p175385118014"></a>-</p>
</td>
</tr>
</tbody>
</table>

更多使用说明请参考：[方舟运行时使用指南](https://gitee.com/openharmony/ark_js_runtime/blob/master/docs/ARK-Runtime-Usage-Guide-zh.md)

## 相关仓<a name="section1371113476307"></a>

[ark\_runtime\_core](https://gitee.com/openharmony/ark_runtime_core)

[ark\_js\_runtime](https://gitee.com/openharmony/ark_js_runtime)

**[ark\_ts2abc](https://gitee.com/openharmony/ark_ts2abc)**
