<!--
 * @Author: linzehui
 * @Date: 2023-03-05 17:04:09
-->

何为脚本语言
代替编译器工作：如同静态语言需要经历语法解析等一系列过程最后生成可执行文件，脚本语言需要对输入的脚本进行解析生成对应的执行指令。
检测并执行指令：静态语言编译后即为机器指令直接可执行，而脚本语言经过上一步，翻译成了我们自己特定的执行指令，因此需要对其进行判断和实际的逻辑处理。这一步的判断实现了沙盒的安全性，而逻辑处理则实现了类似于CPU的运算功能。
为了实现这两方面功能，我们需要一套指令系统，以及一个能够不断执行各条指令的逻辑处理系统。这两个，也就是脚本语言中核心设计之指令集和虚拟机。

指令集
除了单纯读取指令，还要考虑怎样判断读的应该是数据，所以内部设计应该有一种基本操作指令用于单纯读数据

虚拟机
实际应用中往往需要多条指令去实现一个目标，这里就需要多条指令保持一个相对关系并存储对应的值，这儿就和操作系统的栈功能很类似了，我们将这个栈及对应的栈操作称之为虚拟机。

值
只传递单一类型的值：好处就是简单方便，坏处就是过于不通用，牺牲了灵活性，因此几乎不被采纳
标签+联合传值：传递的值开头几位设计为类型的标签，后续才是长度、数值，这种做法好处当然是通用性强，问题是在于会占用更多的内存，需要对内存进行更精细的管理。如Lua等脚本语言就采取此方法，因此gc也就成了重中之重。
不带标签直接传值：将值的正确性交给脚本的编写者承担，这就意味着提高了运行速度的同时也带来了极大的风险。

指令和栈
如果指令均只能访问栈顶元素，那么一个简单的加法，就需要读指令，取值，读指令，取值，然后再相加。如果模拟CPU中的寄存器，则可以大幅提高速度，甚至可以通过寄存器存储栈帧位置实现栈内任意位置的访问。这也是指令和栈设计的另一种模式。两种模式如何选择，需要设计者多多思考。
仅基于栈，则指令会非常简单，代码生成也更容易，但是同样的一段逻辑处理，无疑需要更多的指令，越复杂的逻辑，需要的数量就会更繁琐。Java虚拟机、.NET CLR、Python虚拟机均基于此实现。
栈+寄存器，会导致指令变得复杂，但是相对的指令数就会减少。Lua正式基于这种设计思想而实（5.0之前基于栈，5.0之后改为此形式）。

内存处理
GC




哈希？？？（table的实现会用到的）

字节序（Endianness）（官方lua实现没考虑跨平台）, 大小端问题???     https://www.ruanyifeng.com/blog/2022/06/endianness-analysis.html

浮点数格式(IEEE754)









自己动手实现lua 读书笔记 begin

lua二进制chunk

lua脚本并不是直接被lua解释器解释执行,而是类似java语言那样,先由lua编译器编译为字节码,然后再交给lua虚拟机去执行

一段可以被Lua解释器解释执行的代码就叫作chunk。chunk可以很小，小到只有一两条语句；也可以很大，大到包含成千上万条语句和复杂的函数定义。前面也提到过，为了获得较高的执行效率，Lua并不是直接解释执行chunk，而是先由编译器编译成内部结构（其中包含字节码等信息），然后再由虚拟机执行字节码。这种内部结构在Lua里就叫作预编译（Precompiled）chunk，由于采用了二进制格式，所以也叫二进制（Binary）chunk。

chunk理解成我们平时写的lua代码,二进制chunk其实就是用luac encode了下(转成内部结构,包含字节码等信息),lua解释器能直接跑平时的lua代码,也能直接跑二进制chunk,因为它内部实际还是先把lua代码转成二进制chunk再执行的,所以预编译这操作不会加快执行速度,只会加快加载脚本的速度

Lua编译器以函数为单位进行编译，每一个函数都会被Lua编译器编译为一个内部结构，这个结构叫作“原型”（Prototype）。原型主要包含6部分内容，分别是：函数基本信息（包括参数数量、局部变量数量等）、字节码、常量表、Upvalue表、调式信息、子函数原型列表。由此可知，函数原型是一种递归结构，并且Lua源码中函数的嵌套关系会直接反映在编译后的原型里。

细心的读者一定会想到这样一个问题：前面我们写的“Hello，World！”程序里面只有一条打印语句，并没有定义函数，那么Lua编译器是怎么编译这个文件的呢？由于Lua是脚本语言，如果我们每执行一段脚本都必须要定义一个函数（就像Java那样），岂不是很麻烦？所以这个吃力不讨好的工作就由Lua编译器代劳了。

Lua编译器会自动为我们的脚本添加一个main函数（后文称其为主函数），并且把整个程序都放进这个函数里，然后再以它为起点进行编译，那么自然就把整个程序都编译出来了。这个主函数不仅是编译的起点，也是未来Lua虚拟机解释执行程序时的入口。我们写的“Hello，World！”程序被Lua编译器加工之后，就变成了下面这个样子。

```lua
function main(...)
    print("hello world")
    return
end
```

把主函数编译成函数原型后，Lua编译器会给它再添加一个头部（Header），然后一起dump成luac.out文件，这样，一份热乎的二进制chunk文件就新鲜出炉了。如下图所示。  

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E5%BE%AE%E4%BF%A1%E6%88%AA%E5%9B%BE_20230409170544.png)

luac命令主要有两个用途：第一，作为编译器，把Lua源文件编译成二进制chunk文件：第二，作为反编译器，分析二进制chunk，将信息输出到控制台。

```txt
luac命令使用例子

1，直接编译指定文件，默认在Luac.exe目录生成luac.out
./Luac.exe /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua

2，编译指定文件，同时生成的二进制chunk文件
./Luac.exe -o /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.luac53 /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua

3，反编译指定文件，输出chunk信息到控制台（注意同时会在Luac.exe目录生成luac.out，可以加 -p 参数不生成luac.out，实际传明文lua文件或二进制chunk文件都可以）
./Luac.exe -l /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua
./Luac.exe -l /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.luac53

4，反编译高级版，常量表、局部变量表和upvalue表的信息都会打印出来
./Luac.exe -l -l /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua

5，单纯编译一下指定文件，一般用来检测下语法吧
./Luac.exe -p /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua

6，编译出的二进制chunk文件不带调试信息
./Luac.exe -s /D/Workspace/LearnLua/HelloWorld/res/script/hello_world.lua
```

```txt
// foo_bar.lua
function foo()
    function bar() end
end

// luac decode
./Luac.exe -p -l /D/Workspace/LearnLua/HelloWorld/res/script/foo_bar.lua

main <D:/Workspace/LearnLua/HelloWorld/res/script/foo_bar.lua:0,0> (3 instructions at 01044020)
0+ params, 2 slots, 1 upvalue, 0 locals, 1 constant, 1 function
        1       [3]     CLOSURE         0 0     ; 010458A0
        2       [1]     SETTABUP        0 -1 0  ; _ENV "foo"
        3       [3]     RETURN          0 1

function <D:/Workspace/LearnLua/HelloWorld/res/script/foo_bar.lua:1,3> (3 instructions at 010458A0)
0 params, 2 slots, 1 upvalue, 0 locals, 1 constant, 1 function
        1       [2]     CLOSURE         0 0     ; 010459D0
        2       [2]     SETTABUP        0 -1 0  ; _ENV "bar"
        3       [3]     RETURN          0 1

function <D:/Workspace/LearnLua/HelloWorld/res/script/foo_bar.lua:2,2> (1 instruction at 010459D0)
0 params, 2 slots, 0 upvalues, 0 locals, 0 constants, 0 functions
        1       [2]     RETURN          0 1
```

反编译打印出的函数信息包含两个部分：前面两行是函数基本信息，后面是指令列表。

第一行如果以main开头，说明这是编译器为我们生成的主函数；以function开头，说明这是一个普通函数。接着是定义函数的源文件名和函数在文件里的起止行号（对于主函数，起止行号都是0），然后是指令数量和函数地址。

第二行依次给出函数的固定参数数量（如果有+号，表示这是一个vararg函数）、运行函数所必要的寄存器数量、upvalue数量、局部变量数量、常量数量、子函数数量。

指令列表里的每一条指令都包含指令序号、对应行号、操作码和操作数。分号后面是luac根据指令操作数生成的注释，以便于我们理解指令。

@TODO header 和 proto 的详细解析

























lua指令集

高级编程语言虚拟机是对真实计算机的模拟和抽象。按照实现方式，虚拟机大致可以分为两类：基于栈（StackBased）和基于寄存器（RegisterBased）。

Lua虚拟机则是基于寄存器的虚拟机（实际上Lua在5.0版之前使用的也是基于栈的虚拟机，不过从5.0版开始改成了基于寄存器的虚拟机）。

就如同真实机器有一套指令集（InstructionSet）一样，虚拟机也有自己的指令集：基于栈的虚拟机需要使用PUSH类指令往栈顶推入值，使用POP类指令从栈顶弹出值，其他指令则是对栈顶值进行操作，因此指令集相对比较大，但是指令的平均长度比较短；基于寄存器的虚拟机由于可以直接对寄存器进行寻址，所以不需要PUSH或者POP类指令，指令集相对较小，但是由于需要把寄存器地址编码进指令里，所以指令的平均长度比较长。

按照指令长度是否固定，指令集可以分为定长（Fixed-width）指令集和变长（Variable-width）指令集两种。比如Java虚拟机使用的是变长指令集，指令长度从1到多个字节不等；Lua虚拟机采用的则是定长指令集，每条指令占4个字节（共32比特），其中6比特用于操作码（Opcode），其余26比特用于操作数（Operand）。

Lua5.3一共定义了47条指令，按照作用，这些指令大致可以分为常量加载指令、运算符相关指令、循环和跳转指令、函数调用相关指令、表操作指令以及Upvalue操作指令6大类。

编码模式。每条Lua虚拟机指令占用4个字节，共32个比特（可以用Go语言uint32类型表示），其中低6个比特用于操作码，高26个比特用于操作数。按照高26个比特的分配（以及解释）方式，Lua虚拟机指令可以分为四类，分别对应四种编码模式（Mode）：iABC、iABx、iAsBx、iAx。

iABC模式的指令可以携带A、B、C三个操作数，分别占用8、9、9个比特；iABx模式的指令可以携带A和Bx两个操作数，分别占用8和18个比特；iAsBx模式的指令可以携带A和sBx两个操作数，分别占用8和18个比特；iAx模式的指令只携带一个操作数，占用全部的26个比特。在4种模式中，只有iAsBx模式下的sBx操作数会被解释成有符号整数，其他情况下操作数均被解释为无符号整数。

由于Lua虚拟机指令使用6个比特表示操作码，所以最多只能有64条指令。Lua5.3一共定义了47条指令，操作码从0开始，到46截止。

操作数是指令的参数，每条指令（因编码模式而异）可以携带1到3个操作数。其中操作数A主要用来表示目标寄存器索引，其他操作数按照其表示的信息，可以粗略分为四种类型：OpArgN、OpArgU、OpArgR、OpArgK。

OpArgN类型的操作数不表示任何信息，也就是说不会被使用。比如MOVE指令（iABC模式）只使用A和B操作数，不使用C操作数（OpArgN类型）。

OpArgR类型的操作数在iABC模式下表示寄存器索引，在iAsBx模式下表示跳转偏移。仍以MOVE指令为例，该指令用于将一个寄存器中的值移动到另一个寄存器中，其A操作数表示目标寄存器索引，B操作数（OpArgR类型）表示源寄存器索引。如果用R（N）表示寄存器访问，则MOVE指令可以表示为伪代码R（A）：=R（B）。

OpArgK类型的操作数表示常量表索引或者寄存器索引，具体可以分为两种情况。第一种情况是LOADK指令（iABx模式，用于将常量表中的常量加载到寄存器中），该指令的Bx操作数表示常量表索引，如果用Kst（N）表示常量表访问，则LOADK指令可以表示为伪代码R（A）：=Kst（Bx）；第二种情况是部分iABC模式指令，这些指令的B或C操作数既可以表示常量表索引也可以表示寄存器索引，以加法指令ADD为例，如果用RK（N）表示常量表或者寄存器访问，则该指令可以表示为伪代码R（A）：=RK（B）+RK（C）。

对于上面的第二种情况，既然操作数既可以表示寄存器索引，也可以表示常量表索引，那么如何知道其究竟表示的是哪种索引呢？在iABC模式下，B和C操作数各占9个比特，如果B或C操作数属于OpArgK类型，那么就只能使用9个比特中的低8位，最高位的那个比特如果是1，则操作数表示常量表索引，否则表示寄存器索引。

除了上面介绍的这几种情况，操作数也可能表示布尔值、整数值、upvalue索引、子函数索引等，这些情况都可以归到OpArgU类型里。  

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-03_00-58-46.jpg)  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-19_20-21-02.jpg)  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-05_21-20-44.jpg)  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-19_20-21-39.jpg)  

编码模式有4种，其中 iABC ， iABx ， iAx 这3种要 decode 是比较简单的，直接按位取就好了。但 iAsBx 模式的 decode 会相对麻烦啲，因为 sBx 表示的是有符号整数，有符号整数的编码方式有很多种，例如 2的补码(two's complement) ， lua这边用的是叫 偏移二进制码(offset binary，也叫作excess-k)，具体来说，如果把sBx解释成无符号整数时它的值是x，那么解释成有符号整数时它的值就是x-K。那么K是什么呢？K取sBx所能表示的最大无符号整数值的一半，也就是上面代码中的MAXARG_sBx。






lua stack

栈索引

lua state

LUA_TNONE到底代表什么类型呢？如前所述，Lua栈也可以按索引存取值，如果我们提供给LuaAPI一个无效索引，那么这个索引对应的值的类型就是LUA_TNONE（对应 Object.md 里的疑惑）

lua state的常用方法 , 栈的基本操作 , 外部宿主(host)push值进栈顶 , 外部宿主(host)get栈内的值











lua运算符

Lua语言层面一共有25个运算符，按类别可以分为算术（Arithmetic）运算符、按位（Bitwise）运算符、比较（Comparison）运算符、逻辑（Logical）运算符、长度运算符和字符串拼接运算符。

算术运算符共8个，分别是：+（加）、-（减、一元取反）、*（乘）、/（除）、//（整除）、%（取模）、^（乘方）。
1 , 除法运算符和乘方（Exponentiation或Power）运算符会先把操作数转换为浮点数然后再进行计算，计算结果也一定是浮点数。其他6个算术运算符则会先判断操作数是否都是整数，如果是则进行整数计算，结果也一定是整数（虽然可能会溢出）；若不是整数则把操作数转换成浮点数然后进行计算，结果为浮点数。
2 , 整除（FloorDivision）运算符会将除法结果向下（负无穷方向）取整，
print(5//3) , 结果 1
print(-5//3) , 结果 -2
在Java和Go语言里（C++也是），整除（TruncatedDivision）运算只是将除法结果截断（向0取整）。
3 , 取模（Modulo）运算，也叫取余（Remainder）运算，可以使用整除运算进行定义。
a % b == a - ((a // b) * b)
4 , 乘方运算符和后面将要介绍的字符串拼接运算符具有右结合性（RightAssociative），其他二元运算符则具有左结合性（LeftAssociative）。
print(100/10/2) -- (100/10)/2
print(4^3^2) -- 4^(3^2)

按位运算符共6个，分别是&（按位与）、|（按位或）、~（二元异或、一元按位取反）、<<（左移）和>>（右移）。
1 , 按位运算符会先把操作数转换为整数再进行计算，计算结果也一定是整数。
2 , 右移运算符是无符号右移，空出来的比特只是简单地补0。
3 , 移动 -n 个比特相当于向相反的方向移动n个比特。
print(-1 >> 63) , 输出 1
print(2 << 1) , 输出 1
print("1" << 1.0) , 输出 2

比较运算符又叫关系（Relational）运算符，一共有6个，分别是==（等于）、~=（不等于）、>（大于）、>=（大于等于）、<（小于）、<=（小于等于）。

逻辑运算符有3个，分别是：and（逻辑与）、or（逻辑或）、not（逻辑非）。Lua语言没有给逻辑运算符分配专门的符号，而是给它们分配了三个关键字。

长度运算符是一元运算符，用于提取字符串或表的长度。

字符串拼接运算符用于拼接字符串

运算时的自动类型转换

在API层面，有4个方法专门用来支持Lua运算符。
Arith（）方法用于执行算术和按位运算，Compare（）方法用于执行比较运算，Len（）方法用于执行取长度运算，Concat（）方法用于执行字符串拼接运算。这4个方法覆盖了除逻辑运算符之外的所有Lua运算符

Arith（）方法可以执行算术和按位运算，具体的运算由参数指定，操作数则从栈顶弹出。对于二元运算，该方法会从栈顶弹出两个值进行计算，然后将结果推入栈顶。对于一元运算，该方法从栈顶弹出一个值进行计算，然后把结果推入栈顶。  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-15_01-11-58.jpg)  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-15_01-12-43.jpg)  

Compare（）方法对指定索引处的两个值进行比较，返回结果。该方法不改变栈的状态。  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-15_01-33-28.jpg)

Len（）方法访问指定索引处的值，取其长度，然后推入栈顶。  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-15_01-35-12.jpg)

Concat（）方法从栈顶弹出n个值，对这些值进行拼接，然后把结果推入栈顶。  
![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-04-15_01-40-49.jpg)













lua VM

我们在读一本书的时候，往往无法一口气读完，如果中间累了，或者需要停下来思考问题，或者有其他事情需要处理，就暂时把当前页数记下来，合上书，然后休息、思考或者处理问题，然后再回来继续阅读。我们也经常会跳到之前已经读过的某一页，去回顾一些内容，或者翻到后面还没有读到的某一页，去预览一些内容，然后再回到当前页继续阅读。

就如同人类需要使用大脑（或者书签）去记录正在阅读的书籍的页数一样，计算机也需要一个程序计数器（ProgramCounter，简称PC）来记录正在执行的指令。与人类不同的是，计算机不需要休息和思考，也没有烦恼的事情需要处理（但需要等待IO），一旦接通电源就会不知疲倦地计算PC、取出当前指令、执行指令，如此往复直到指令全部处理完毕或者断电为止。

和真实的机器一样，Lua虚拟机也需要使用程序计数器。如果暂时忽略函数调用等细节，可以使用如下伪代码来描述Lua虚拟机的内部循环。

```txt
loop {
	计算PC
	取出当前指令
	执行当前指令
}
```

```txt
另外再定义些方法 , 因为除了上面对栈的操作和运算符的方法 , 还需要一些PC的操作和对常量表操作的方法才能进一步实现虚拟机
PC()int//返回当前PC（仅测试用）
AddPC(nint)//修改PC（用于实现跳转指令）
Fetch()uint32//取出当前指令；将PC指向下一条指令
GetConst(idxint)//将指定常量推入栈顶
GetRK(rkint)//将指定常量或栈值推入栈顶
```

其中GetRK方法相对复杂点 , GetRK（）方法根据情况调用GetConst（）方法把某个常量推入栈顶，或者调用PushValue（）方法把某个索引处的栈值推入栈顶，代码如下所示。

```txt
func (self *luaState) GetRK(rk int)
{
	if rk > 0xff
	{
		// constant
		self.GetConst(rk & 0xff)
	}
	else
	{
		// register
		self.PushValue(rk + 1)
	}
}
```

我们马上就会看到，传递给GetRK（）方法的参数实际上是iABC模式指令里的OpArgK类型参数。由第3章可知，这种类型的参数一共占9个比特。如果最高位是1，那么参数里存放的是常量表索引，把最高位去掉就可以得到索引值；否则最高位是0，参数里存放的就是寄存器索引值。但是请读者留意，Lua虚拟机指令操作数里携带的寄存器索引是从0开始的，而LuaAPI里的栈索引是从1开始的，所以当需要把寄存器索引当成栈索引使用时，要对寄存器索引加1。

先将指令大致分成4类 , 运算符相关 , 加载相关 , for循环相关 , 其他相关(MOVE , JMP)

1 , 其他相关指令 , MOVE

作用 , MOVE指令（iABC模式）把源寄存器（索引由操作数B指定）里的值移动到目标寄存器（索引由操作数A指定）里。

```txt
MOVE指令常用于局部变量赋值和参数传递，以局部变量赋值为例

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_move.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_move.lua:0,0> (3 instructions at 00A10908)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     MOVE            3 1
        3       [2]     RETURN          0 1
```

虽然说是MOVE指令，但实际上叫作COPY指令可能更合适一些，因为源寄存器的值还原封不动待在原地。

我们先解码指令，得到目标寄存器和源寄存器索引，然后把它们转换成栈索引，最后调用LuaAPI提供的Copy（）方法拷贝栈值。如前文所述，寄存器索引加1才是相应的栈索引，后面不再赘述。

通过MOVE指令我们还可以看到，Lua代码里的局部变量实际就存在寄存器里。由于MOVE等指令使用操作数A（占8个比特）来表示目标寄存器索引，所以Lua函数使用的局部变量不能超过255个。实际上Lua编译器把函数的局部变量数限制在了200个以内，如果超过这个数量，函数就无法编译。

2 , 其他相关指令 , JMP

作用 , JMP指令（iAsBx模式）执行无条件跳转(操作数sBx值作为索引 , 操作数A和Upvalue有关)。该指令往往和后面要介绍的TEST等指令配合使用，但是也可能会单独出现，比如Lua也支持标签和goto语句（虽然强烈不建议使用）。

```txt
./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_jmp.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_jmp.lua:0,0> (2 instructions at 013408D0)
0+ params, 2 slots, 1 upvalue, 0 locals, 0 constants, 0 functions
        1       [1]     JMP             0 -1    ; to 1
        2       [2]     RETURN          0 1
```

3 , 加载相关指令(加载指令用于把nil值、布尔值或者常量表里的常量值加载到寄存器里) , LOADNIL

作用 , LOADNIL指令（iABC模式）用于给连续n个寄存器放置nil值。寄存器的起始索引由操作数A指定，寄存器数量则由操作数B指定，操作数C没有用。

```txt
在Lua代码里，局部变量的默认初始值是nil。LOADNIL指令常用于给连续n个局部变量设置初始值，下面是一个例子。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadnil.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadnil.lua:0,0> (2 instructions at 00C70918)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [1]     RETURN          0 1
```

由第3章可知，Lua编译器在编译函数生成指令表时，会把指令执行阶段所需要的寄存器数量预先算好，保存在函数原型里。这里假定虚拟机在执行第一条指令前，已经根据这一信息调用SetTop（）方法保留了必要数量的栈空间。有了这个假设，我们就可以先调用PushNil（）方法往栈顶推入一个nil值，然后连续调用Copy（）方法将nil值复制到指定寄存器中，最后调用Pop（）方法把一开始推入栈顶的那个nil值弹出，让栈顶指针恢复原状。

4 , 加载相关指令 , LOADBOOL

作用 , LOADBOOL指令（iABC模式）给单个寄存器设置布尔值。寄存器索引由操作数A指定，布尔值由寄存器B指定（0代表false，非0代表true），如果寄存器C非0则跳过下一条指令。

```txt
./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadbool.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadbool.lua:0,0> (6 instructions at 01250988)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     LT              1 1 0
        3       [2]     JMP             0 1     ; to 5
        4       [2]     LOADBOOL        2 0 1
        5       [2]     LOADBOOL        2 1 0
        6       [2]     RETURN          0 1
```

5 , 加载相关指令 , LOADK

作用 , LOADK指令（iABx模式）将常量表里的某个常量加载到指定寄存器，寄存器索引由操作数A指定，常量表索引由操作数Bx指定。

```txt
在Lua函数里出现的字面量（主要是数字和字符串）会被Lua编译器收集起来，放进常量表里。以下面的局部变量赋值语句为例。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadk.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_loadk.lua:0,0> (6 instructions at 00F208E0)
0+ params, 5 slots, 1 upvalue, 5 locals, 3 constants, 0 functions
        1       [1]     LOADNIL         0 0
        2       [1]     LOADK           1 -1    ; 1
        3       [1]     LOADK           2 -2    ; 2
        4       [1]     LOADK           3 -2    ; 2
        5       [1]     LOADK           4 -3    ; "foo"
        6       [1]     RETURN          0 1
```

我们先调用前面准备好的GetConst（）方法把指定常量推入栈顶，然后调用Replace（）方法把它移动到指定索引处。我们知道操作数Bx占18个比特，能表示的最大无符号整数是262143，大部分Lua函数的常量表大小都不会超过这个数，所以这个限制通常不是什么问题。不过Lua也经常被当作数据描述语言使用，所以常量表大小可能超过这个限制也并不稀奇。为了应对这种情况，Lua还提供了一条LOADKX指令。

6 , 加载相关指令 , LOADKX

作用 , LOADKX指令（也是iABx模式）需要和EXTRAARG指令（iAx模式）搭配使用，用后者的Ax操作数来指定常量索引。Ax操作数占26个比特，可以表达的最大无符号整数是67108864，可以满足大部分情况了。

7 , 运算符相关指令 , 二元运算

作用 , 二元算术运算指令（iABC模式），对两个寄存器或常量值（索引由操作数B和C指定）进行运算，将结果放入另一个寄存器（索引由操作数A指定）。

```txt
以加法运算符为例。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_add.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_add.lua:0,0> (3 instructions at 01400908)
0+ params, 5 slots, 1 upvalue, 5 locals, 1 constant, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     ADD             4 1 -1  ; - 100
        3       [2]     RETURN          0 1
```

我们先调用前面准备好的GetRK（）方法把两个操作数推入栈顶，然后调用Arith（）方法进行算术运算。算术运算完毕之后，操作数已经从栈顶弹出，取而代之的是运算结果，我们调用Replace（）方法把它移动到指定寄存器即可。

7 , 运算符相关指令 , 一元运算

作用 , 一元算术运算指令（iABC模式），对操作数B所指定的寄存器里的值进行运算，然后把结果放入操作数A所指定的寄存器中，操作数C没用。








































自己动手实现lua 读书笔记 end



