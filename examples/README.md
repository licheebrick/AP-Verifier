本文档介绍了net plumber以及将要实现的AP-Verifier所接受的网络配置规则集的构成及语法;

## 概述
一个规则集由以下三部分构成：
* 以router_name + ".rules.json"为名的，用于指明一个router上的所有规则的规则文件；（每个router单独一个）
* 描述该规则集所应用网络的拓扑结构的"topology.json"文件，指明port之间的连接情况；
* 描述将要在该规则集上进行的校验的"policy.json"文件；

## 更加具体的解释
### "rules.json"文件
参考文件夹中给出的router1.rules.json的例子，一个此类文件指明一个router的信息，整体json的结构包括如下部分：
1. "rules": *为rule的列表*；注意，rules的优先级是从上向下递增的！也就是最下部的rule的优先级是最高的（在交叠的情况下）；具体的，每个rule又包括：
  * "in_ports" & "out_ports": *均为整数的列表*，指明该rule所涉及的inports和outports，列表就支持了广播；
  * "action": 表示该rule所对包进行的操作，可以是"fwd", "rw";
  * "match": 匹配项，表该规则所执行操作的包的集合；
  * "rewrite"及"match": 在规则为"rw"时，共同表示该rule对包头的修改方法，其中"match"相当于一个掩码，只有在"match"位为１的位才会进行改写；
2. "ports": 该router上的所有rule所涉及的ports列表；
3. "id": 该router的编号；

### "topology.json"文件
参考文件夹中给出的示例，描述了拓扑；

### "policy.json"文件
该文件描述了对该规则集的一系列操作，包括：
* 动态update网络配置的操作如"add_link", "add_rule", "add_table"等...
* 一系列的对网络进行check的操作，如可达性check， bypass node check等，一般都是通过：
  * add_link to source node (source node可以理解为从某点向网络中注入流量)
  * add_source　实际添加source node
  * add_link to source probe (source probe可以理解为放置在网络中具有强大功能的探针)
  * add_source_probe 实际添加source probe，实际上此部分比较复杂，由于HSA实现了各种校验目标，针对每个校验目标所要放置的probe参数大不相同，此处先暂不考虑；（如有需要，会另起文档进行具体说明）

## 一些需求
* port编号和router编号希望有较规整的结构，能够比较容易地看出port所属的router.(比如router1上的port编号为10001,10002,...)
* policy.json其实现阶段不太需要生成，基本上只需要给我提供一下注入的router及port　＆　要查看可达性的router及port即可。
* 再单独给一个file列出router名；
