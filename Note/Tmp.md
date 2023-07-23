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

浮点字节（FloatingPointByte）的编码方式








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






lua stack @TODO

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

7 , 运算符相关指令 , 二元运算(ADD , SUB , MUL , MOD , POW , DIV , IDIV , BAND , BOR , BXOR , SHL , SHR)

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

8 , 运算符相关指令 , 一元运算(UNM , BNOT)

作用 , 一元算术运算指令（iABC模式），对操作数B所指定的寄存器里的值进行运算，然后把结果放入操作数A所指定的寄存器中，操作数C没用。

```txt
以按位取反运算符为例。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_bnot.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_bnot.lua:0,0> (3 instructions at 00CA08D0)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     BNOT            3 1
        3       [2]     RETURN          0 1
```

9 , 长度指令 , LEN

作用 , LEN指令（iABC模式）进行的操作和一元算术运算指令类似 , 对操作数B所指定的寄存器里的值取其长度 , 然后把结果放入操作数A所指定的寄存器中 , 操作数C没用

```txt
LEN指令对应Lua语言里的长度运算符，如下所示。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_len.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_len.lua:0,0> (3 instructions at 01572F80)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     LEN             3 1
        3       [2]     RETURN          0 1
```

10 , 拼接指令 , CONCAT

作用 , CONCAT指令（iABC模式），将连续n个寄存器（起止索引分别由操作数B和C指定）里的值拼接，将结果放入另一个寄存器（索引由操作数A指定）。

```txt
CONCAT指令对应Lua语言里的拼接运算符，如下所示。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_concat.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_concat.lua:0,0> (6 instructions at 005B07C8)
0+ params, 5 slots, 1 upvalue, 2 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 1
        2       [2]     MOVE            2 0
        3       [2]     MOVE            3 0
        4       [2]     MOVE            4 0
        5       [2]     CONCAT          1 2 4
        6       [2]     RETURN          0 1
```

11 , 比较指令

作用 , 比较指令（iABC模式），比较寄存器或常量表里的两个值（索引分别由操作数B和C指定），如果比较结果和操作数A（转换为布尔值）匹配，则跳过下一条指令。比较指令不改变寄存器状态

```txt
比较指令对应Lua语言里的比较运算符（当用于赋值时，需要和LOADBOOL指令搭配使用），以等于运算符为例。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_eq.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_eq.lua:0,0> (6 instructions at 011F0770)
0+ params, 5 slots, 1 upvalue, 5 locals, 1 constant, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     EQ              1 1 -1  ; - "foo"
        3       [2]     JMP             0 1     ; to 5
        4       [2]     LOADBOOL        0 0 1
        5       [2]     LOADBOOL        0 1 0
        6       [2]     RETURN          0 1
```

我们先调用GetRK（）方法把两个要比较的值推入栈顶，然后调用Compare（）方法执行比较运算，如果比较结果和操作数A一致则把PC加1。由于Compare（）方法并没有把栈顶值弹出，所以我们需要自己调用Pop（）方法清理栈顶。

12 , 逻辑运算指令 , NOT

作用 , NOT指令（iABC模式）进行的操作和一元算术运算指令类似 , 对操作数B所指定的寄存器里的值进行逻辑非运算 , 然后把结果放入操作数A所指定的寄存器中 , 操作数C没用

```txt
NOT指令对应Lua语言里的逻辑非运算符，如下所示。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_not.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_not.lua:0,0> (3 instructions at 00E00740)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     NOT             3 1
        3       [2]     RETURN          0 1
```

13 , 逻辑运算指令 , TESTSET

作用 , TESTSET指令（iABC模式），判断寄存器B（索引由操作数B指定）中的值转换为布尔值之后是否和操作数C表示的布尔值一致，如果一致则将寄存器B中的值复制到寄存器A（索引由操作数A指定）中，否则跳过下一条指令。

```txt
TESTSET指令对应Lua语言里的逻辑与(AND)和逻辑或(OR)运算符，以逻辑与为例。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_testset.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_testset.lua:0,0> (5 instructions at 00860798)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     TESTSET         1 3 0
        3       [2]     JMP             0 1     ; to 5
        4       [2]     MOVE            1 4
        5       [2]     RETURN          0 1
```

14 , 逻辑运算指令 , TEST

作用 , TEST指令（iABC模式），判断寄存器A（索引由操作数A指定）中的值转换为布尔值之后是否和操作数C表示的布尔值一致，如果一致，则跳过下一条指令。TEST指令不使用操作数B，也不改变寄存器状态

```txt
TEST指令是TESTSET指令的特殊形式，如下所示。

./Luac.exe -p -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_test.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_test.lua:0,0> (5 instructions at 008307C8)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     TEST            1 0
        3       [2]     JMP             0 1     ; to 5
        4       [2]     MOVE            1 4
        5       [2]     RETURN          0 1
```

15 , FOR循环指令 @TODO

作用 , Lua语言的for循环语句有两种形式：数值（Numerical）形式和通用（Generic）形式。数值for循环用于按一定步长遍历某个范围内的数值，通用for循环主要用于遍历表。

这里目前主要讨论数值形式

数值for循环需要借助两条指令来实现：FORPREP和FORLOOP。

```txt
下面是数值形式循环的例子

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_for_numerical.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_for_numerical.lua:0,0> (8 instructions at 007E40E8)
0+ params, 5 slots, 1 upvalue, 4 locals, 4 constants, 0 functions
        1       [1]     LOADK           0 -1    ; 1
        2       [1]     LOADK           1 -2    ; 2
        3       [1]     LOADK           2 -3    ; 100
        4       [1]     FORPREP         0 2     ; to 7
        5       [2]     GETTABUP        4 0 -4  ; _ENV "f"
        6       [2]     CALL            4 1 1
        7       [1]     FORLOOP         0 -3    ; to 5
        8       [3]     RETURN          0 1
constants (4) for 007E40E8:
        1       1
        2       2
        3       100
        4       "f"
locals (4) for 007E40E8:
        0       (for index)     4       8
        1       (for limit)     4       8
        2       (for step)      4       8
        3       i       5       7
upvalues (1) for 007E40E8:
        0       _ENV    1       0
```

@REMIND 留意下面的示意图中的 step 和 limit 中的值应该是搞反了..

从反编译输出的局部变量表可知，Lua编译器为了实现for循环，使用了三个特殊的局部变量，这三个特殊局部变量的名字里都包含圆括号（属于非法标识符），这样就避免了和程序中出现的普通变量重名的可能。由名字可知，这三个局部变量分别存放数值、限制和步长，并且在循环开始之前就已经预先初始化好了（对应三条LOADK指令）。

此外我们还可以看出，这三个特殊的变量正好对应前面伪代码中的R（A）、R（A+1）和R（A+2）这三个寄存器，我们自己在for循环里定义的变量i则对应R（A+3）寄存器。由此可知，FORPREP指令执行的操作其实就是在循环开始之前预先给数值减去步长，然后跳转到FORLOOP指令正式开始循环，如下图所示。

FORLOOP指令则是先给数值加上步长，然后判断数值是否还在范围之内。如果已经超出范围，则循环结束；若未超过范围则把数值拷贝给用户定义的局部变量，然后跳转到循环体内部开始执行具体的代码块。上面例子中的第一次循环可以用下图表示。

还有一点需要解释，也就是FORLOOP指令伪代码中的“<？=”符号。当步长是正数时，这个符号的含义是“<=”，也就是说继续循环的条件是数值不大于限制；当步长是负数时，这个符号的含义是“>=”，循环继续的条件就变成了数值不小于限制。





















lua Table

Lua官方在5.0以前也是简单使用哈希表来实现Lua表的，不过由于在实践中数组的使用非常频繁，所以为了专门优化数组的效率，Lua5.0开始改用混合数据结构来实现表。简单来说，这种混合数据结构同时包含了数组和哈希表两部分。如果表的键是连续的正整数，那么哈希表就是空的，值全部按索引存储在数组里。这样，Lua数组无论是在空间利用率上（不需要显式存储键），还是时间效率上（可以按索引存取值，不需要计算哈希码）都和真正的数组相差无几。如果表并没有被当成数组使用，那么数据完全存储在哈希表里，数组部分是空的，也没什么损失。

书中对于 table 的实现是通过 go 语言实现的, 所以具体实现会相对简单, 不像 c 那么复杂. 首先 table 很强大, 使用上它能以 array 的方式使用, 也能以 map 的方式使用. 所以它内部实现同时需要有 array 的逻辑, 也有 map 的逻辑. 主要要留意的地方是, 1, 构造 table, 里面可能需要同时创建 array 的容器 和 map 的容器. 2, 除了 nil 都能作为 table 的key. 3, get 方法, 用什么逻辑判断是从 array 取还是从 map 取(如果 key 是数字, 先尝试从 array 取, 取不到再从 map 取, key 不是数字, 直接从 map 取). 4, set 方法, 同样需要用什么逻辑判断是 set 进 array 还是 map, 还有空洞的问题, 例如, 一个 table {[1]=1,[2]=2,[4]=4} 本身是以 map 的方式使用, 可能通过增加 key 变成 {[1]=1,[2]=2,[3]=3,[4]=4} 就能以 array 的方式使用. 5, 还有对于 array 和 map 的迭代器.

table 相关指令

表相关指令一共有4条：NEWTABLE指令创建空表，GETTABLE指令根据键从表里取值，SETTABLE指令根据键往表里写入值，SETLIST指令按索引批量更新数组元素。

1, TABLE相关指令, NEWTABLE

作用, NEWTABLE指令（iABC模式）创建空表，并将其放入指定寄存器。寄存器索引由操作数A指定，表的初始数组容量和哈希表容量分别由操作数B和C指定。

```txt
Lua代码里的每一条表构造器语句都会产生一条NEWTABLE指令，下面是一个例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_newtable.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_newtable.lua:0,0> (6 instructions at 0105F298)
0+ params, 5 slots, 1 upvalue, 4 locals, 4 constants, 0 functions
        1       [1]     LOADNIL         0 3
        2       [2]     NEWTABLE        4 0 2
        3       [2]     SETTABLE        4 -1 -2 ; "x" 1
        4       [2]     SETTABLE        4 -3 -4 ; "y" 2
        5       [2]     MOVE            1 4
        6       [2]     RETURN          0 1
constants (4) for 0105F298:
        1       "x"
        2       1
        3       "y"
        4       2
locals (4) for 0105F298:
        0       a       2       7
        1       b       2       7
        2       c       2       7
        3       d       2       7
upvalues (1) for 0105F298:
        0       _ENV    1       0
```

其他都很好理解，但是这个Fb2int（）函数起到什么作用呢？因为NEWTABLE指令是iABC模式，操作数B和C只有9个比特，如果当作无符号整数的话，最大也不能超过512。但是我们在前面也提到过，因为表构造器便捷实用，所以Lua也经常被用来描述数据（类似JSON），如果有很大的数据需要写成表构造器，但是表的初始容量又不够大，就容易导致表频繁扩容从而影响数据加载效率。

为了解决这个问题，NEWTABLE指令的B和C操作数使用了一种叫作浮点字节（FloatingPointByte）的编码方式。这种编码方式和浮点数的编码方式类似，只是仅用一个字节。具体来说，如果把某个字节用二进制写成eeeeexxx，那么当eeeee==0时该字节表示的整数就是xxx，否则该字节表示的整数是（1xxx）*2^（eeeee1）。

2, TABLE相关指令, GETTABLE

作用, GETTABLE指令（iABC模式）根据键从表里取值，并放入目标寄存器中。其中表位于寄存器中，索引由操作数B指定；键可能位于寄存器中，也可能在常量表里，索引由操作数C指定；目标寄存器索引则由操作数A指定。

```txt
GETTABLE指令对应Lua代码里的表索引取值操作，如下所示。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_gettable.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_gettable.lua:0,0> (4 instructions at 015A0768)
0+ params, 5 slots, 1 upvalue, 5 locals, 1 constant, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     GETTABLE        3 1 2
        3       [3]     GETTABLE        3 1 -1  ; 100
        4       [3]     RETURN          0 1
constants (1) for 015A0768:
        1       100
locals (5) for 015A0768:
        0       a       2       5
        1       t       2       5
        2       k       2       5
        3       v       2       5
        4       e       2       5
upvalues (1) for 015A0768:
        0       _ENV    1       0
```

3, TABLE相关指令, SETTABLE

作用, SETTABLE指令（iABC模式）根据键往表里赋值。其中表位于寄存器中，索引由操作数A指定；键和值可能位于寄存器中，也可能在常量表里，索引分别由操作数B和C指定。

```txt
SETTABLE指令对应Lua代码里的表索引赋值操作，如下所示。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_settable.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_settable.lua:0,0> (4 instructions at 011A07A0)
0+ params, 5 slots, 1 upvalue, 5 locals, 2 constants, 0 functions
        1       [1]     LOADNIL         0 4
        2       [2]     SETTABLE        1 2 3
        3       [3]     SETTABLE        1 -1 -2 ; 100 "foo"
        4       [3]     RETURN          0 1
constants (2) for 011A07A0:
        1       100
        2       "foo"
locals (5) for 011A07A0:
        0       a       2       5
        1       t       2       5
        2       k       2       5
        3       v       2       5
        4       e       2       5
upvalues (1) for 011A07A0:
        0       _ENV    1       0
```

4, TABLE相关指令, SETLIST

作用, SETTABLE是通用指令，每次只处理一个键值对，具体操作交给表去处理，并不关心实际写入的是表的哈希部分还是数组部分。SETLIST指令（iABC模式）则是专门给数组准备的，用于按索引批量设置数组元素。其中数组位于寄存器中，索引由操作数A指定；需要写入数组的一系列值也在寄存器中，紧挨着数组，数量由操作数B指定；数组起始索引则由操作数C指定。

那么数组的索引到底是怎么计算的呢？这里的情况和GETTABLE指令有点类似。因为C操作数只有9个比特，所以直接用它表示数组索引显然不够用。这里的解决办法是让C操作数保存批次数，然后用批次数乘上批大小（对应伪代码中的FPF）就可以算出数组起始索引。以默认的批大小50为例，C操作数能表示的最大索引就是扩大到了25600（50*512）。

但是如果数组长度大于25600呢？是不是后面的元素就只能用SETTABLE指令设置了？也不是。这种情况下SETLIST指令后面会跟一条EXTRAARG指令，用其Ax操作数来保存批次数。综上所述，如果指令的C操作数大于0，那么表示的是批次数加1，否则，真正的批次数存放在后续的EXTRAARG指令里。

(注意有一种情况 , SETLIST的操作数B是0的情况 , 大概遇到 {1, 2, 3, func()} 这样的写法的时候就会出现 , 这时候设置 table 的值会依赖 func() 的返回值 , 具体可以看书里378页 , 涉及到CALL指令)

```txt
下面我们结合一个简单的例子来观察一下SETLIST指令。(建议把注释的lua代码打开后再看看会对批数理解直观点)

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_setlist.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_setlist.lua:0,0> (7 instructions at 00E007C8)
0+ params, 5 slots, 1 upvalue, 1 local, 4 constants, 0 functions
        1       [1]     NEWTABLE        0 4 0
        2       [1]     LOADK           1 -1    ; 1
        3       [1]     LOADK           2 -2    ; 2
        4       [1]     LOADK           3 -3    ; 3
        5       [1]     LOADK           4 -4    ; 4
        6       [1]     SETLIST         0 4 1   ; 1
        7       [1]     RETURN          0 1
constants (4) for 00E007C8:
        1       1
        2       2
        3       3
        4       4
locals (1) for 00E007C8:
        0       t       7       8
upvalues (1) for 00E007C8:
        0       _ENV    1       0
```




























lua 函数调用

参数

返回值

函数调用栈(Call Stack) , 调用帧(Call Frame)

前面我们详细讨论了Lua栈。有趣的是，函数调用也经常借助“栈”这种数据结构来实现。为了区别于Lua栈，我们称其为函数调用栈，简称调用栈（CallStack）。Lua栈里面存放的是Lua值，调用栈里存放的则是调用栈帧，简称为调用帧（CallFrame）。

当我们调用一个函数时，要先往调用栈里推入一个调用帧，然后把参数传递给调用帧。函数依托调用帧执行指令，可能会调用其他函数，以此类推。当函数执行完毕之后，调用帧里会留下函数需要返回的值。我们把调用帧从调用栈顶弹出，并且把返回值返回给底部的调用帧，这样一次函数调用就结束了。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/Xnip2023-05-16_01-12-52.jpg)

先说说lua函数的特点 , 首先 , 当然能支持固定参数列表 , 同时也能支持变长参数列表 . 固定参数列表时 , 如果入参数量小于参数列表 , 剩下的参数置为nil , 如果入餐数量大于参数列表 , 多余的入参被忽略 . 变长参数列表时 , 多余的入餐就会收纳在vararg里 . 然后 , lua函数支持多个返回值 . 如果接收个数小于返回值个数 , 那么多余的返回值忽略 . 如果接收个数大于返回值个数 , 那么剩下的接收变量置为nil

主要要分清2个东西，一个是调用栈，一个是调用帧。调用栈的结构实现在 lua_state 里，实际就是调度函数的过程，链表结构，链表头部是栈顶。调用帧的结构实现在 lua_stack 里，实际就是处理函数执行的过程，根据指令在value栈上处理数据。大概如下图。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202023-06-18%20152923.png)

书中执行一个函数的过程（就是执行一个 closure 的过程）并不是一直在同一个 stack 上进行的。例如，当前 stack_1 上有函数funcA，参数params_1，参数params_2，那执行funcA(params_1, params_2)的过程是，先创建一个新的stack_2（表示栈帧），然后 stack_1 pop出funcA，params_1，params_2一共3个value，新的 stack_2 push入params_1，params_2一共2个value，接着实际上是在新的 stack_2 上执行funcA函数要处理的逻辑，执行完指令后，stack_2上只有返回值，最后把stack_2上的返回值pop到stack_1里，完成一次函数执行。大概如下图（具体看书350页，为什么要创一个stack上执行，而不是在当前stack上，应该是函数的通用设计就是如此吧，进栈出栈，函数现场等。。。具体官方C版本怎样实现要看源码 @TODO）

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/closure_01.drawio.png)

函数调用指令

1，函数调用指令， CLOSURE

作用，CLOSURE指令（iBx模式）把当前Lua函数的子函数原型实例化为闭包，放入由操作数A指定的寄存器中。子函数原型来自于当前函数原型的子函数原型表，索引由操作数Bx指定。

所谓的函数原型实例化，实际就是通过二进制chunk的信息创建closure

```txt
CLOSURE指令对应Lua脚本里的函数定义语句或者表达式，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_closure.lua

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_closure.lua:0,0> (4 instructions at 006D1748)
0+ params, 5 slots, 1 upvalue, 5 locals, 0 constants, 2 functions
        1       [1]     LOADNIL         0 2
        2       [2]     CLOSURE         3 0     ; 006D5140
        3       [3]     CLOSURE         4 1     ; 006D5030
        4       [3]     RETURN          0 1
constants (0) for 006D1748:
locals (5) for 006D1748:
        0       a       2       5
        1       b       2       5
        2       c       2       5
        3       f       3       5
        4       g       4       5
upvalues (1) for 006D1748:
        0       _ENV    1       0

function <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_closure.lua:2,2> (1 instruction at 006D5140)
0 params, 2 slots, 0 upvalues, 0 locals, 0 constants, 0 functions
        1       [2]     RETURN          0 1
constants (0) for 006D5140:
locals (0) for 006D5140:
upvalues (0) for 006D5140:

function <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_closure.lua:3,3> (1 instruction at 006D5030)
0 params, 2 slots, 0 upvalues, 0 locals, 0 constants, 0 functions
        1       [3]     RETURN          0 1
constants (0) for 006D5030:
locals (0) for 006D5030:
upvalues (0) for 006D5030:
```

2，函数调用指令， CALL

作用，CALL指令（iABC模式）调用Lua函数。其中被调函数位于寄存器中，索引由操作数A指定。需要传递给被调函数的参数值也在寄存器中，紧挨着被调函数，数量由操作数B指定。函数调用结束后，原先存放函数和参数值的寄存器会被返回值占据，具体有多少个返回值则由操作数C指定。

（大概执行流程类似上面funcA的例子，但需要考虑多个返回值的情况，还有就是对书里代码的一些疑惑，其中一处是函数逻辑执行完后，返回值已经pop回栈上，但代码里还用replace接口处理了下，感觉有点多余或者我有些情况没想到。。）

```txt
CALL指令对应Lua脚本里的函数调用语句或者表达式，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_call.lua

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_call.lua:0,0> (7 instructions at 00EA1778)
0+ params, 5 slots, 1 upvalue, 3 locals, 5 constants, 0 functions
        1       [1]     GETTABUP        0 0 -1  ; _ENV "f"
        2       [1]     LOADK           1 -2    ; 1
        3       [1]     LOADK           2 -3    ; 2
        4       [1]     LOADK           3 -4    ; 3
        5       [1]     LOADK           4 -5    ; 4
        6       [1]     CALL            0 5 4
        7       [1]     RETURN          0 1
constants (5) for 00EA1778:
        1       "f"
        2       1
        3       2
        4       3
        5       4
locals (3) for 00EA1778:
        0       a       7       8
        1       b       7       8
        2       c       7       8
upvalues (1) for 00EA1778:
        0       _ENV    1       0

// 多返回值情况 留意 B C 操作数
./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_call.lua

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_call.lua:0,0> (7 instructions at 0137E878)
0+ params, 4 slots, 1 upvalue, 0 locals, 4 constants, 0 functions
        1       [6]     GETTABUP        0 0 -1  ; _ENV "f"
        2       [6]     LOADK           1 -2    ; 1
        3       [6]     LOADK           2 -3    ; 2
        4       [6]     GETTABUP        3 0 -4  ; _ENV "g"
        5       [6]     CALL            3 1 0
        6       [6]     CALL            0 0 1
        7       [6]     RETURN          0 1
constants (4) for 0137E878:
        1       "f"
        2       1
        3       2
        4       "g"
locals (0) for 0137E878:
upvalues (1) for 0137E878:
        0       _ENV    1       0
```

3，函数调用指令， RETURN

作用，RETURN指令（iABC模式）把存放在连续多个寄存器里的值返回给主调函数。其中第一个寄存器的索引由操作数A指定，寄存器数量由操作数B指定，操作数C没用。

我们需要将返回值推入栈顶。如果操作数B等于1，则不需要返回任何值；如果操作数B大于1，则需要返回B-1个值，这些值已经在寄存器里了，循环调用PushValue（）方法复制到栈顶即可。如果操作数B等于0，则一部分返回值已经在栈顶了，调用_fixStack（）函数把另一部分也推入栈顶。

```txt
RETURN指令对应Lua脚本里的return语句，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_return.lu
a

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_return.lua:0,0> (6 instructions at 01312D60)
0+ params, 5 slots, 1 upvalue, 2 locals, 1 constant, 0 functions
        1       [1]     LOADNIL         0 1
        2       [2]     LOADK           2 -1    ; 1
        3       [2]     MOVE            3 0
        4       [2]     MOVE            4 1
        5       [2]     RETURN          2 4
        6       [2]     RETURN          0 1
constants (1) for 01312D60:
        1       1
locals (2) for 01312D60:
        0       a       2       7
        1       b       2       7
upvalues (1) for 01312D60:
        0       _ENV    1       0
```

4，函数调用指令， VARARG

作用，VARARG指令（iABC模式）把传递给当前函数的变长参数加载到连续多个寄存器中。其中第一个寄存器的索引由操作数A指定，寄存器数量由操作数B指定，操作数C没有用。

（对书中的一处有些疑惑，如果操作数B为0，那么vararg的参数将全部load进栈里，而且紧跟其后会把操作数A也push进栈里，目前自己的理解是因为可能之后要读vararg的参数，所以要利用操作数A为基础作索引。。）

```txt
VARARG指令对应Lua脚本里的vararg表达式，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_vararg.lu
a

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_vararg.lua:0,0> (3 instructions at 0067E850)
0+ params, 5 slots, 1 upvalue, 5 locals, 1 constant, 0 functions
        1       [5]     LOADK           0 -1    ; 100
        2       [5]     VARARG          1 5
        3       [5]     RETURN          0 1
constants (1) for 0067E850:
        1       100
locals (5) for 0067E850:
        0       a       3       4
        1       b       3       4
        2       c       3       4
        3       d       3       4
        4       e       3       4
upvalues (1) for 0067E850:
        0       _ENV    1       0
```

由于编译器生成的主函数也是vararg函数，所以也可以在里面使用vararg表达式。假定调用主函数时传递给它的参数是1、2、3，那么上例中的VARARG指令如图所示。

5，函数调用指令， TAILCALL

作用 , 们已经知道，函数调用一般通过调用栈来实现。用这种方法，每调用一个函数都会产生一个调用帧。如果方法调用层次太深（特别是递归调用函数时），就容易导致调用栈溢出。那么，有没有一种技术，既能让我们发挥递归函数的威力，又能避免调用栈溢出呢？有，那就是尾递归优化。利用这种优化，被调函数可以重用主调函数的调用帧，因此可以有效缓解调用栈溢出症状。不过尾递归优化只适用于某些特定的情况，并不能包治百病。

TAILCALL指令（iABC模式） , 操作数ABC表示的意义跟 CALL 指令一样.

```txt
对尾递归优化的详细介绍超出了本书讨论范围，这里我们只要知道 return f(args)这样的返回语句会被Lua编译器编译成TAILCALL指令就可以了。下面来看一个例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_tailcall.l
ua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_tailcall.lua:0,0> (7 instructions at 00ED56E8)
0+ params, 4 slots, 1 upvalue, 0 locals, 4 constants, 0 functions
        1       [1]     GETTABUP        0 0 -1  ; _ENV "f"
        2       [1]     GETTABUP        1 0 -2  ; _ENV "a"
        3       [1]     GETTABUP        2 0 -3  ; _ENV "b"
        4       [1]     GETTABUP        3 0 -4  ; _ENV "c"
        5       [1]     TAILCALL        0 4 0
        6       [1]     RETURN          0 0
        7       [1]     RETURN          0 1
constants (4) for 00ED56E8:
        1       "f"
        2       "a"
        3       "b"
        4       "c"
locals (0) for 00ED56E8:
upvalues (1) for 00ED56E8:
        0       _ENV    1       0
```

6，函数调用指令， SELF

作用 , Lua虽然不是面向对象语言，但是提供了一些语法和底层支持，利用这些支持，用户就可以构造出一套完整的面向对象体系。SELF指令主要用来优化方法调用语法糖。比如说obj：f（a，b，c），虽然从语义的角度来说完全等价于obj.f（obj，a，b，c），但是Lua编译器并不是先去掉语法糖再按普通的函数调用处理，而是会生成SELF指令，这样就可以节约一条指令。SELF指令（iABC模式）把对象和方法拷贝到相邻的两个目标寄存器中。对象在寄存器中，索引由操作数B指定。方法名在常量表里，索引由操作数C指定。目标寄存器索引由操作数A指定。

```txt
下面有2个例子对比参照.

1 , 使用 SELF 指令的例子

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_self.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_self.lua:0,0> (5 instructions at 00726EE8)
0+ params, 5 slots, 1 upvalue, 2 locals, 1 constant, 0 functions
        1       [4]     LOADNIL         0 1
        2       [5]     SELF            2 1 -1  ; "f"
        3       [5]     MOVE            4 0
        4       [5]     CALL            2 3 1
        5       [5]     RETURN          0 1
constants (1) for 00726EE8:
        1       "f"
locals (2) for 00726EE8:
        0       a       2       6
        1       obj     2       6
upvalues (1) for 00726EE8:
        0       _ENV    1       0

2 , 不用 SELF 指令的例子

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_self.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_self.lua:0,0> (6 instructions at 01706EC8)
0+ params, 5 slots, 1 upvalue, 2 locals, 1 constant, 0 functions
        1       [10]    LOADNIL         0 1
        2       [11]    GETTABLE        2 1 -1  ; "f"
        3       [11]    MOVE            3 1
        4       [11]    MOVE            4 0
        5       [11]    CALL            2 3 1
        6       [11]    RETURN          0 1
constants (1) for 01706EC8:
        1       "f"
locals (2) for 01706EC8:
        0       a       2       7
        1       obj     2       7
upvalues (1) for 01706EC8:
        0       _ENV    1       0
```























native 函数调用（书里主要介绍的就是调Go函数）

首先分清为什么又 lua函数 又 native函数，主要因为是 lua函数 是提供脚本层的逻辑，实际上都是对 lua value 的计算和管理，它不能取系统时间，加载文件等。一般要使用这些功能要引入c的库和调用c的函数，所以lua这边要加入一种方式调用这些native的函数。

要想让Lua函数调用Go语言编写函数，就需要一种机制能够给Go函数传递参数，并且接收Go函数返回的值。可是Lua函数只能操作Lua栈，这可如何是好？答案就在问题之中。我们已经知道，Lua栈对于Lua函数的调用和执行至关重要。在执行Lua函数时，Lua栈充当虚拟寄存器以供指令操作。在调用Lua函数时，Lua栈充当栈帧以供参数和返回值传递。那么我们自然也可以利用Lua栈来给Go函数传递参数和接收返回值。

我们约定，Go函数必须满足这样的签名：接收一个LuaState接口类型的参数，返回一个整数。在Go函数开始执行之前，Lua栈里是传入的参数值，别无它值。当Go函数结束之后，把需要返回的值留在栈顶，然后返回一个整数表示返回值个数。由于Go函数返回了返回值数量，这样它在执行完毕时就不用对栈进行清理了，把返回值留在栈顶即可。

实际的实现方式其实是相对简单的，定义 native 函数的闭包类型（主要就是参数需要一个 Lua_State 类型，返回值表示返回值数量），调用时通过判断是闭包里是 lua 的 proto 还是 native 执行不同的处理，但实处理也跟之前执行 lua 函数差不多，new 一个栈帧，把参数推入栈帧后执行 native 的函数，返回值留在栈帧上，最后以多退少补的逻辑把返回值推回到主调用栈帧上。









lua 注册表

实际就是在 Lua_State 类型里加了个 Table 类型的成员



lua 全局环境
实际就是上面的注册表里面的一个表而已







自己动手实现lua 读书笔记 end



