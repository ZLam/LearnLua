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

15 , FOR循环指令

作用 , Lua语言的for循环语句有两种形式：数值（Numerical）形式和通用（Generic）形式。数值for循环用于按一定步长遍历某个范围内的数值，通用for循环主要用于遍历表。

这里目前主要讨论数值形式 @TODO

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

访问方式，伪索引（@TODO ，其实就是划分区间不同区间对应不同的逻辑去索引值，具体看C那边的实现吧）



lua 全局环境

实际就是上面的注册表里面的一个表而已
























lua 闭包 / upvalue

闭包是计算机编程语言里非常普遍的一个概念，不过Upvalue却是Lua语言独有的。

闭包概念begin

一等函数

如果在一门编程语言里，函数属于一等公民（First-classCitizen），我们就说这门语言里的函数是一等函数（First-classFunction）。具体是什么意思呢？其实就是说函数用起来和其他类型的值（比如数字或者字符串）没什么分别，比如说可以把函数存储在数据结构里、赋值给变量、作为参数传递给其他函数或者作为返回值从其他函数里返回等。

支持一等函数的语言非常多。函数式编程语言，比如Haskell、Scala等，全部支持一等函数；很多脚本语言也支持一等函数，比如JavaScript、Python和本书讨论的Lua，C、C++和本书所使用的Go语言也都支持一等函数。作为反例，在Java语言里函数就不是一等公民（Java甚至没有函数的概念，只有方法）。

如果一个函数以其他函数为参数，或者返回其他函数，我们称这个函数为高阶函数（Higher-orderFunction）；反之，我们称这个函数为一阶函数（First-orderFunction）。

如果可以在函数的内部定义其他函数，我们称内部定义的函数为嵌套函数（NestedFunction），外部的函数为外围函数（EnclosingFunction）。在许多支持一等函数的语言里，函数实际上都是匿名的，在这些语言里，函数名就是普通的变量名，只是变量的值恰好是函数而已。比如在Lua里，函数定义语句只是函数定义表达式（或者叫函数构造器，也可以认为是函数字面量）和赋值语句的语法糖，下面两行代码在语义上完全等价。

```lua
function add(x, y) return x + y end
add = function(x, y) return x + y end
```

变量作用域

支持一等函数的编程语言有一个非常重要的问题需要处理，就是非局部变量的作用域问题。最简单的做法就是不支持嵌套函数，比如C语言，这样就完全避开了这个问题。对于支持嵌套函数的语言，有两种处理方式：动态作用域（DynamicScoping），或者静态作用域（StaticScoping）。

在使用动态作用域的语言里，函数的非局部变量名具体绑定的是哪个变量只有等到函数运行时才能确定。由于动态作用域会导致程序不容易被理解，所以现代编程语言大多使用静态作用域。在目前流行的语言里，使用动态作用域的有shell语言Bash和PowerShell等。以Bash语言为例，下面的脚本（来自于Wikipedia）在执行时会先后打印出3和1。

```bash
x=1
function g() {echo $x; x=2;}
function f() {local x=3; g;}
f               # 3
echo $x         # 1
```

与动态作用域不同，静态作用域在编译时就可以确定非局部变量名绑定的变量，因此静态作用域也叫作词法作用域（LexicalScoping）。Lua也采用静态作用域，我们把上面的Bash脚本用Lua语言翻译一下，代码如下所示。

```lua
x = 1
function g() print(x); x = 2 end
function f() local x = 3; g() end
f()             -- 1
print(x)        -- 2
```

介绍完各种函数和作用域的概念，闭包（Closure）就非常好理解了。所谓闭包，就是按词法作用域捕获了非局部变量的嵌套函数。现在大家知道为什么在Lua内部函数被称为闭包了吧？因为Lua函数本质上全都是闭包。就算是编译器为我们生成的主函数也不例外，它从外部捕获了_ENV变量。

闭包概念end

upvalue概念begin

如前所述，闭包是一个通用的概念，很多语言都支持闭包。但Upvalue是Lua里才有的术语，那么究竟什么是Upvalue呢？实际上Upvalue就是闭包内部捕获的非局部变量，可能是因为历史原因，这个术语一直被沿用至今。下面我们来看一个简单的例子。

```lua
local u, v, w
local function f() u = v end
```

我们已经知道，Lua编译器会把脚本包装进一个主函数，因此上面的脚本在编译时大致会变成如下样子。

```lua
function main()
    local u, v, w
    local function f() u = v end
end
```

函数f捕获了主函数里的两个局部变量，因此我们可以说f有两个Upvalue，分别是u和v。Lua编译器会把Upvalue相关信息编译进函数原型，存放在Upvalue表里。如果我们用luac命令（搭配两个“-l”选项）观察上面这段脚本，可以看到函数f原型里确实有两个Upvalue，如下所示。

```txt
./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_01.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_01.lua:0,0> (3 instructions at 010217F8)
0+ params, 4 slots, 1 upvalue, 4 locals, 0 constants, 1 function
        1       [1]     LOADNIL         0 2
        2       [4]     CLOSURE         3 0     ; 010218F8
        3       [4]     RETURN          0 1
constants (0) for 010217F8:
locals (4) for 010217F8:
        0       u       2       4
        1       v       2       4
        2       w       2       4
        3       f       3       4
upvalues (1) for 010217F8:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_01.lua:2,4> (3 instructions at 010218F8)
0 params, 2 slots, 2 upvalues, 0 locals, 0 constants, 0 functions
        1       [3]     GETUPVAL        0 1     ; v
        2       [3]     SETUPVAL        0 0     ; u
        3       [4]     RETURN          0 1
constants (0) for 010218F8:
locals (0) for 010218F8:
upvalues (2) for 010218F8:
        0       u       1       0
        1       v       1       1
```

函数原型Upvalue表的每一项都有4列：第一列是序号，从0开始递增；第二列给出Upvalue的名字；第三列指出Upvalue捕获的是否是直接外围函数的局部变量，1表示是，0表示否；如果Upvalue捕获的是直接外围函数的局部变量，第四列给出局部变量在外围函数调用帧里的索引。

如果闭包捕获的是非直接外围函数的局部变量会出现什么情况呢？比如下面这个例子。

```lua
local u, v, w
local function f()
    local function g()
        u = v
    end
end
```

在上面这段脚本里，函数f没有访问任何非局部变量，但是函数g访问了主函数里定义的局部变量u和v。

```txt
./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_02.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_02.lua:0,0> (3 instructions at 00483F40)
0+ params, 4 slots, 1 upvalue, 4 locals, 0 constants, 1 function
        1       [1]     LOADNIL         0 2
        2       [6]     CLOSURE         3 0     ; 0047F508
        3       [6]     RETURN          0 1
constants (0) for 00483F40:
locals (4) for 00483F40:
        0       u       2       4
        1       v       2       4
        2       w       2       4
        3       f       3       4
upvalues (1) for 00483F40:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_02.lua:2,6> (2 instructions at 0047F508)
0 params, 2 slots, 2 upvalues, 1 local, 0 constants, 1 function
        1       [5]     CLOSURE         0 0     ; 0047F6E8
        2       [6]     RETURN          0 1
constants (0) for 0047F508:
locals (1) for 0047F508:
        0       g       2       3
upvalues (2) for 0047F508:
        0       u       1       0
        1       v       1       1

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_upvalue_02.lua:3,5> (3 instructions at 0047F6E8)
0 params, 2 slots, 2 upvalues, 0 locals, 0 constants, 0 functions
        1       [4]     GETUPVAL        0 1     ; v
        2       [4]     SETUPVAL        0 0     ; u
        3       [5]     RETURN          0 1
constants (0) for 0047F6E8:
locals (0) for 0047F6E8:
upvalues (2) for 0047F6E8:
        0       u       0       0
        1       v       0       1
```

可见，虽然函数f并没有直接访问主函数中的局部变量，但是为了能够让函数g捕获u和v这两个Upvalue，函数f也必须捕获它们。

可以看到，函数原型Upvalue表的第三列都变成了0，这说明这些Upvalue捕获的并非是直接外围函数中的局部变量，而是更外围的函数的局部变量。在这种情况下，Upvalue已经由外围函数捕获，嵌套函数直接使用即可，所以第四列表示的是外围函数的Upvalue表索引。

像Lua这种，需要借助外围函数来捕获更外围函数局部变量的闭包，叫作扁平闭包（FlatClosures）。

Upvalue是非局部变量，换句话说，就是某外围函数中定义的局部变量。那么全局变量又是什么呢？我们还是来看一个例子吧。

```lua
local function f()
    local function g()
        x = y
    end
end
```

```txt
./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/test_global_01.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_global_01.lua:0,0> (2 instructions at 011412A0)
0+ params, 2 slots, 1 upvalue, 1 local, 0 constants, 1 function
        1       [5]     CLOSURE         0 0     ; 011414C0
        2       [5]     RETURN          0 1
constants (0) for 011412A0:
locals (1) for 011412A0:
        0       f       2       3
upvalues (1) for 011412A0:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_global_01.lua:1,5> (2 instructions at 011414C0)
0 params, 2 slots, 1 upvalue, 1 local, 0 constants, 1 function
        1       [4]     CLOSURE         0 0     ; 011415A0
        2       [5]     RETURN          0 1
constants (0) for 011414C0:
locals (1) for 011414C0:
        0       g       2       3
upvalues (1) for 011414C0:
        0       _ENV    0       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/test_global_01.lua:2,4> (3 instructions at 011415A0)
0 params, 2 slots, 1 upvalue, 0 locals, 2 constants, 0 functions
        1       [3]     GETTABUP        0 0 -2  ; _ENV "y"
        2       [3]     SETTABUP        0 -1 0  ; _ENV "x"
        3       [4]     RETURN          0 1
constants (2) for 011415A0:
        1       "x"
        2       "y"
locals (0) for 011415A0:
upvalues (1) for 011415A0:
        0       _ENV    0       0
```

在上面这段脚本里，函数g使用了x和y这两个变量，但无论是f还是主函数都没有定义这两个局部变量，那么x和y到底是什么呢？

函数g的原型里并没有x和y这两个Upvalue，但是出现一个奇怪的Upvalue，名字是_ENV。

可见，函数f的原型里也有这个Upvalue。我们再来看一下主函数的反编译输出。

到这里可以揭晓答案了，全局变量实际上是某个特殊的表的字段，而这个特殊的表正是我们在第9章实现的全局环境。Lua编译器在生成主函数时会在它的外围隐式声明一个局部变量，类似下面这样。

```lua
local _ENV  -- 全局环境
function main(...)
    -- 其他代码
end
```

然后编译器会把全局变量的读写翻译成_ENV字段的读写，也就是说，全局变量实际上也是语法糖，去掉语法糖后，前面的例子和下面这段脚本完全等价。

```lua
local function f()
    local function g()
        _ENV.x = _ENV.y
    end
end
```

到此，我们可以对Lua的变量类型进行一个简单的总结了。Lua的变量可以分为三类：局部变量在函数内部定义（本质上是函数调用帧里的寄存器），Upvalue是直接或间接外围函数定义的局部变量，全局变量则是全局环境表的字段（通过隐藏的Upvalue，也就是_ENV进行访问）。

对于某个Upvalue来说，对它的任何改动都必须反映在其他该Upvalue可见的地方。另外，当嵌套函数执行时，外围函数的局部变量有可能已经退出作用域了。

Lua闭包捕获的Upvalue数量已经由编译器计算好了，我们在创建Lua闭包时预先分配好空间即可。不仅Lua闭包可以捕获Upvalue，Go闭包也可以捕获Upvalue，与Lua闭包不同的是，我们需要在创建Go闭包时明确指定Upvalue的数量。

我们需要根据函数原型里的Upvalue表来初始化闭包的Upvalue值。对于每个Upvalue，又有两种情况需要考虑：如果某一个Upvalue捕获的是当前函数的局部变量（Instack==1），那么我们只要访问当前函数的局部变量即可；如果某一个Upvalue捕获的是更外围的函数中的局部变量（Instack==0），该Upvalue已经被当前函数捕获，我们只要把该Upvalue传递给闭包即可。

对于第一种情况，如果Upvalue捕获的外围函数局部变量还在栈上，直接引用即可，我们称这种Upvalue处于开放（Open）状态；反之，必须把变量的实际值保存在其他地方，我们称这种Upvalue处于闭合（Closed）状态。为了能够在合适的时机（比如局部变量退出作用域时，详见10.3.5节）把处于开放状态的Upvalue闭合，需要记录所有暂时还处于开放状态的Upvalue，我们把这些Upvalue记录在被捕获局部变量所在的栈帧里。（@TODO 目前还不太清楚干嘛要处理这个开放闭合问题）

至于 native 的方法也可以有 upvalue ，大概的逻辑就是要处理，将 native方法 转成 native的闭包，然后在 push native闭包进栈时，把当前栈顶的n个 value pop 出栈成为 native闭包的 upvalue ，然后 native闭包 推入栈顶。

upvalue概念end

upvalue相关指令begin

1， GETUPVAL

作用，GETUPVAL指令（iABC模式），把当前闭包的某个Upvalue值拷贝到目标寄存器中。其中目标寄存器的索引由操作数A指定，Upvalue索引由操作数B指定，操作数C没用。

```txt
如果我们在函数中访问Upvalue值，Lua编译器就会在这些地方生成GETUPVAL指令，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_getupval.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_getupval.lua:0,0> (4 instructions at 008546D0)
0+ params, 4 slots, 1 upvalue, 3 locals, 1 constant, 1 function
        1       [1]     LOADNIL         0 2
        2       [4]     CLOSURE         3 0     ; 008548A8
        3       [2]     SETTABUP        0 -1 3  ; _ENV "f"
        4       [4]     RETURN          0 1
constants (1) for 008546D0:
        1       "f"
locals (3) for 008546D0:
        0       u       2       5
        1       v       2       5
        2       w       2       5
upvalues (1) for 008546D0:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_getupval.lua:2,4> (6 instructions at 008548A8)
0 params, 5 slots, 3 upvalues, 5 locals, 2 constants, 0 functions
        1       [3]     LOADK           0 -1    ; 1
        2       [3]     LOADK           1 -2    ; 2
        3       [3]     GETUPVAL        2 0     ; u
        4       [3]     GETUPVAL        3 1     ; v
        5       [3]     GETUPVAL        4 2     ; w
        6       [4]     RETURN          0 1
constants (2) for 008548A8:
        1       1
        2       2
locals (5) for 008548A8:
        0       a       6       7
        1       b       6       7
        2       c       6       7
        3       d       6       7
        4       e       6       7
upvalues (3) for 008548A8:
        0       u       1       0
        1       v       1       1
        2       w       1       2
```

在函数f（）里，我们使用了u、v、w这三个Upvalue，分别赋值给c、d、e这三个局部变量，产生了三条GETUPVAL指令。其中第二条GETUPVAL指令（对应d=v）如图所示。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E6%9C%AA%E5%91%BD%E5%90%8D1692798985.png)

2， SETUPVAL

作用，SETUPVAL指令（iABC模式），使用寄存器中的值给当前闭包的Upvalue赋值。其中寄存器索引由操作数A指定，Upvalue索引由操作数B指定，操作数C没用。

```txt
如果我们在函数给Upvalue赋值，Lua编译器就会在这些地方生成SETUPVAL指令，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_setupval.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_setupval.lua:0,0> (4 instructions at 011755D0)
0+ params, 4 slots, 1 upvalue, 3 locals, 1 constant, 1 function
        1       [1]     LOADNIL         0 2
        2       [5]     CLOSURE         3 0     ; 0117F428
        3       [2]     SETTABUP        0 -1 3  ; _ENV "f"
        4       [5]     RETURN          0 1
constants (1) for 011755D0:
        1       "f"
locals (3) for 011755D0:
        0       u       2       5
        1       v       2       5
        2       w       2       5
upvalues (1) for 011755D0:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_setupval.lua:2,5> (7 instructions at 0117F428)
0 params, 5 slots, 3 upvalues, 3 locals, 0 constants, 0 functions
        1       [3]     LOADNIL         0 2
        2       [4]     MOVE            3 0
        3       [4]     MOVE            4 1
        4       [4]     SETUPVAL        2 2     ; w
        5       [4]     SETUPVAL        4 1     ; v
        6       [4]     SETUPVAL        3 0     ; u
        7       [5]     RETURN          0 1
constants (0) for 0117F428:
locals (3) for 0117F428:
        0       a       2       8
        1       b       2       8
        2       c       2       8
upvalues (3) for 0117F428:
        0       u       1       0
        1       v       1       1
        2       w       1       2
```

在函数f（）里，我们给u、v、w这三个Upvalue赋值，因此产生了三条SETUPVAL指令。其中第三条SETUPVAL指令（对应u=a）如图所示。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E6%9C%AA%E5%91%BD%E5%90%8D1692878471.png)

3， GETTABUP

作用，如果当前闭包的某个Upvalue是表，则GETTABUP指令（iABC模式）可以根据键从该表里取值，然后把值放入目标寄存器中。其中目标寄存器索引由操作数A指定，Upvalue索引由操作数B指定，键（可能在寄存器中也可能在常量表中）索引由操作数C指定。GETTABUP指令相当于GETUPVAL和GETTABLE这两条指令的组合，不过前者的效率明显要高一些。

```txt
如果我们在函数里按照键从Upvalue里取值，Lua编译器就会在这些地方生成GETTABUP指令，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_gettabup.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_gettabup.lua:0,0> (4 instructions at 00BD1830)
0+ params, 2 slots, 1 upvalue, 1 local, 1 constant, 1 function
        1       [1]     LOADNIL         0 0
        2       [6]     CLOSURE         1 0     ; 00BD69B0
        3       [2]     SETTABUP        0 -1 1  ; _ENV "f"
        4       [6]     RETURN          0 1
constants (1) for 00BD1830:
        1       "f"
locals (1) for 00BD1830:
        0       u       2       5
upvalues (1) for 00BD1830:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_gettabup.lua:2,6> (4 instructions at 00BD69B0)
0 params, 5 slots, 1 upvalue, 5 locals, 1 constant, 0 functions
        1       [3]     LOADNIL         0 4
        2       [4]     GETTABUP        3 0 2   ; u
        3       [5]     GETTABUP        3 0 -1  ; u 100
        4       [6]     RETURN          0 1
constants (1) for 00BD69B0:
        1       100
locals (5) for 00BD69B0:
        0       a       2       5
        1       b       2       5
        2       c       2       5
        3       d       2       5
        4       e       2       5
upvalues (1) for 00BD69B0:
        0       u       1       0
```

上面例子里的两条GETTABUP指令覆盖了C操作数的两种情况，以第一条指令为例（对应d=u[k]），键在寄存器中，如图所示。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E6%9C%AA%E5%91%BD%E5%90%8D1692879247.png)

4， SETTABUP

作用，如果当前闭包的某个Upvalue是表，则SETTABUP指令（iABC模式）可以根据键往该表里写入值。其中Upvalue索引由操作数A指定，键和值可能在寄存器中也可能在常量表中，索引分别由操作数B和C指定。和GETTABUP指令类似，SETTABUP指令相当于GETUPVAL和SETTABLE这两条指令的组合，不过一条指令的效率要高一些。

```txt
如果我们在函数里根据键往Upvalue里写入值，Lua编译器就会在这些地方生成SETTABUP指令，下面是一个简单的例子。

./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_settabup.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_settabup.lua:0,0> (4 instructions at 008712A8)
0+ params, 2 slots, 1 upvalue, 1 local, 1 constant, 1 function
        1       [1]     LOADNIL         0 0
        2       [6]     CLOSURE         1 0     ; 00871540
        3       [2]     SETTABUP        0 -1 1  ; _ENV "f"
        4       [6]     RETURN          0 1
constants (1) for 008712A8:
        1       "f"
locals (1) for 008712A8:
        0       u       2       5
upvalues (1) for 008712A8:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_settabup.lua:2,6> (4 instructions at 00871540)
0 params, 5 slots, 1 upvalue, 5 locals, 2 constants, 0 functions
        1       [3]     LOADNIL         0 4
        2       [4]     SETTABUP        0 2 3   ; u
        3       [5]     SETTABUP        0 -1 -2 ; u 100 "haha"
        4       [6]     RETURN          0 1
constants (2) for 00871540:
        1       100
        2       "haha"
locals (5) for 00871540:
        0       a       2       5
        1       b       2       5
        2       c       2       5
        3       d       2       5
        4       e       2       5
upvalues (1) for 00871540:
        0       u       1       0
```

上面例子里有两条SETTABUP指令，其中第一条指令（对应u[k]=v）的B和C操作数都代表寄存器索引，第二条指令的B和C操作数都代表常量表索引。以第一条SETTABUP指令为例，如图所示。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E6%9C%AA%E5%91%BD%E5%90%8D1692879985.png)

5， JMP

作用，JMP指令除了可以进行无条件跳转之外，还兼顾着闭合处于开启状态的Upvalue的责任。如果某个块内部定义的局部变量已经被嵌套函数捕获，那么当这些局部变量退出作用域（也就是块结束）时，编译器会生成一条JMP指令，指示虚拟机闭合相应的Upvalue。

```txt
./Luac.exe -p -l -l /D/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_jmp.lua

main <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_jmp.lua:0,0> (5 instructions at 005A55D0)
0+ params, 7 slots, 1 upvalue, 6 locals, 1 constant, 1 function
        1       [5]     LOADNIL         0 5
        2       [10]    CLOSURE         6 0     ; 005AD878
        3       [8]     SETTABUP        0 -1 6  ; _ENV "foo"
        4       [10]    JMP             4 0     ; to 5
        5       [11]    RETURN          0 1
constants (1) for 005A55D0:
        1       "foo"
locals (6) for 005A55D0:
        0       x       2       6
        1       y       2       6
        2       z       2       6
        3       a       2       5
        4       b       2       5
        5       c       2       5
upvalues (1) for 005A55D0:
        0       _ENV    1       0

function <D:/Workspace_HDD/LearnLua/HelloWorld/res/script/instruction_jmp.lua:8,10> (3 instructions at 005AD878)
0 params, 2 slots, 1 upvalue, 0 locals, 1 constant, 0 functions
        1       [9]     LOADK           0 -1    ; 1
        2       [9]     SETUPVAL        0 0     ; b
        3       [10]    RETURN          0 1
constants (1) for 005AD878:
        1       1
locals (0) for 005AD878:
upvalues (1) for 005AD878:
        0       b       1       4
```

可以看到，由于函数foo捕获了外部的局部变量b，所以在do语句的后面，编译器生成了一条JMP指令，该JMP指令的sBx操作数是0，所以其实并没有起到任何跳转作用，这条指令的真正用途，是闭合a、b、c这三个Upvalue。

处于开启状态的Upvalue引用了还在寄存器里的Lua值，我们把这些Lua值从寄存器里复制出来，然后更新Upvalue，这样就将其改为了闭合状态。

(看到这里大概能理解这个处理开放闭合 upvalue 是解决什么问题的了。首先当情况是嵌套函数的 upvalue 捕获的是上层函数的局部变量的时候，嵌套函数的 upvalue表 是直接引用上层函数的局部变量的，理解成直接将局部变量的地址存在了 upvalue表 里，这样就会引出一个问题，如果执行嵌套函数的时候并使用 upvalue表 里的值，但上层函数已经结束它的生命周期，那么直接使用 upvalue表 的值应该会抛出访问内存异常，实际上也可能出现这样的状况，一个可能的例子就是上层函数返回这个嵌套函数，然后这个嵌套函数在其他的地方调用，所以这个处理开放闭合 upvalue 的问题，本质上就是解决防止因闭包生命周期结束而引起捕获了它的局部变量作为 upvalue 访问异常，文中在闭包里定义的 openuvs成员 本质就是记录下哪些局部变量用去作为 upvalue 了，然后解决方式就是当该闭包生命周期结束，局部变量理应回收，所有利用 openuvs 记录下来的值复制一遍，维持指向的地址不变，这样之后的 upvalue 就能正常使用)

upvalue相关指令end

总结下，在Lua里，函数属于一等公民。Lua函数实际上全部都是匿名的，函数定义语句只不过是函数定义表达式和赋值语句的语法糖而已。此外，Lua函数本质上是按词法作用域捕获了非局部变量（Upvalue）的闭包。



















元编程begin

所谓元程序（Metaprogram），是指能够处理程序的程序。这里的“处理”包括读取、生成、分析、转换等。而元编程（Metaprogramming），就是指编写元程序的编程技术。元编程有很多种形式，比如C语言的宏（Macro）和C++语言的模板（Template）可以在编译期生成代码、Java语言的反射（Reflection）可以在运行期获取程序信息、JavaScript语言的eval（）函数可以在运行期生成并执行代码，这些都属于元编程的范畴。

在Lua里，每个值都可以有一个元表。如果值的类型是表或者用户数据（本书不讨论用户数据），则可以拥有自己“专属”的元表，其他类型的值则是每种类型共享一个元表。新创建的表默认没有元表，nil、布尔、数字、函数类型默认也没有元表，不过string标准库给字符串类型设置了元表。

真正让元表变得与众不同的，是元方法。比如当我们对两个表进行加法运算，Lua会看这两个表是否有元表；如果有，则进一步看元表里是否有__add元方法；如果有，则将两个表作为参数调用这个方法，将返回结果作为加法运算的结果。

每一个表都可以拥有自己的元表，其他值则是每种类型共享一个元表，所以我们要做的第一件事就是把值和元表关联起来。对于表来说，很自然的做法就是给底层的结构体添加一个字段，用来存放元表。对于其他类型的值，我们可以把元表存放在注册表里。

1，算术元方法

对于算术运算，只有当一个操作数不是（或者无法自动转换为）数字时，才会调用对应的元方法。

如果操作数都是（或者可以转换为）数字，则执行正常的算术运算逻辑，否则尝试查找并执行算术元方法。

2，长度元方法

对于长度运算（#），Lua首先判断值是否是字符串，如果是，结果就是字符串长度；否则看值是否有__len元方法，如果有，则以值为参数调用元方法，将元方法返回值作为结果，如果找不到对应元方法，但值是表，结果就是表的长度。长度运算是由API方法Len（）实现的。

3，拼接元方法

对于拼接运算（..），如果两个操作数都是字符串（或者数字），则进行字符串拼接；否则，尝试调用__concat元方法，查找和调用规则同二元算术元方法。拼接运算是由API方法Concat（）实现的。

4，比较元方法

Lua内部只实现了==、<、<=这三个运算符。a~=b可以转换为not（a==b），a>b可以转换为b<a，a>=b则可以转换为b<=a。相应的，和比较运算对应的元方法也只有三个，分别是__eq、__lt和__le。

对于等于（==）运算，当且仅当两个操作数是不同的表时，才会尝试执行__eq元方法。元方法的查找和执行规则与二元算术运算符类似，但是执行结果会被转换为布尔值。

对于小于（<）运算，如果两个操作数都是数字，则进行数字比较；如果两个操作数都是字符串，则进行字符串比较，否则，尝试调用__lt元方法。元方法的查找和调用规则与__eq元方法类似。

小于等于运算（<=）规则类似小于运算，与之对应的元方法是__le。有所不同的是，如果Lua找不到__le元方法，则会尝试调用__lt元方法（假设a<=b等价于not（b<a））。

5，索引元方法

索引元方法有两个作用：如果一个值不是表，索引元方法可以让这个值用起来像一个表；如果一个值是表，索引元方法可以拦截表操作。索引元方法有两个：__index和__newindex，前者对应索引取值操作，后者对应索引赋值操作。下面我们分别来讨论这两个元方法。

__index , 当Lua执行t[k]表达式时，如果t不是表，或者k在表中不存在，就会触发__index元方法。虽然名为元方法，但实际上__index元方法既可以是函数，也可以是表。如果是函数，那么Lua会以t和k为参数调用该函数，以函数返回值为结果；如果是表，Lua会以k为键访问该表，以值为结果（可能会继续触发__index元方法）。

__newindex , 当Lua执行t[k]=v语句时，如果t不是表，或者k在表中不存在，就会触发__newindex元方法。和__index元方法一样，__newindex元方法也可以是函数或者表。如果是函数，那么Lua会以t、k和v为参数调用该函数；如果是表，Lua会以k为键v为值给该表赋值（可能会继续触发__newindex元方法）。

6，函数调用元方法

当我们试图调用一个非函数类型的值时，Lua会看这个值是否有__call元方法，如果有，Lua会以该值为第一个参数，后跟原方法调用的其他参数，来调用元方法，以元方法返回值为返回值。函数调用是由API方法Call（）实现的。

元编程end










迭代器begin

Lua语言支持两种形式的for循环语句：数值for循环和通用for循环。数值for循环用于在两个数值范围内按照一定的步长进行迭代；通用for循环常用于对表进行迭代。不过通用for循环之所以“通用”，就在于它并不仅仅适用于表，实际上，通用for循环可以对任何迭代器进行迭代。

迭代器（Iterator）模式是一种经典的设计模式，在这种模式里，我们使用迭代器（模式因此得名）对集合（Collection）或者容器（Container）里的元素进行遍历。

为了对集合或者容器进行遍历，迭代器需要保存一些内部状态。在Java这种面向对象的语言里，迭代器自然是以对象形式存在。在Lua语言里，更自然的方式则是用函数来表示迭代器，内部状态可以由闭包捕获。下面我们来看一个对数组进行迭代的例子。

```lua
function ipairs(t)
    local i = 0
    return function()
        i = i + 1
        if t[i] == nil then
            return nil, nil
        else
            return i, t[i]
        end
    end
end
```

可以把上面的ipairs（）函数看作一个工厂，该函数每次调用都会返回一个数组迭代器。迭代器从外部捕获了t和i这两个变量（Upvalue），把它们作为内部状态用于控制迭代，并通过第一个返回值（是否是nil）通知使用者迭代是否结束。下面的代码演示了如何创建迭代器并利用它对数组进行遍历。

```lua
t = {10, 20, 30}
iter = ipairs(t)
while true do
    local i, v = iter()
    if i == nil then
        break
    end
    print(i, v)
end
```

和Java语言里的for-each语句类似，Lua语言也提供了for-in语句，也就是我们前面提到的通用for循环语句，从语言层面对迭代器给予支持。上面的代码可以用for-in语句简写成下面这样。

```lua
t = {10, 20, 30}
for i, v in ipairs(t) do
    print(i, v)
end
```

以上是给数组（严格的说是给序列）创建迭代器，那么如何给关联数组创建迭代器呢？由于关联数组没办法通过递增下标的方式来遍历索引，所以Lua标准库提供了next（）函数来帮助我们创建关联数组迭代器。next（）函数接收两个参数——表和键。返回两个值——下一个键值对。如果传递给next（）函数的键是nil，表示迭代开始；如果next（）函数返回的键是nil，表示迭代结束。下面的代码演示了next（）函数的用法。

```lua
function pairs(t)
    local k, v
    return function()
        k, v = next(t, k)
        return k, v
    end
end

t = {a = 10, b = 20, c = 30}
for k, v in pairs(t) do
    print(k, v)
end
```

通过上面的例子，我们对迭代器和通用for循环有了一个大致的了解，下面给出通用for循环语句的一般形式。

```txt
for var_ 1, ..., var_ n in explist do block end
```

上面的for循环语句和下面的代码等价：

```txt
do
    local _f, _s, _var = explist
    while true do
        local var_1, ..., var_n = _f(_s, _var)
        if var_1 == nil then
            break
        end
        _var = var_1
        block
    end
end
```

其中_f、_s和_var是通用for循环内部使用的，由explist求值得到。_f是迭代器函数，_s是一个不变量，_var是控制变量。乍看起来有点复杂，实际上很好理解。我们用前面的pairs（）函数作为对比：_f相当于next（）函数，_s相当于表，_var则用于存放键。可见，虽然可以用闭包保存迭代器内部状态，不过通用for循环也很贴心，可以帮我们保存一些状态，这样很多时候就可以免去闭包创建之烦。

Next（）方法根据键获取表的下一个键值对。其中表的索引由参数指定，上一个键从栈顶弹出。如果从栈顶弹出的键是nil，说明刚开始遍历表，把表的第一个键值对推入栈顶并返回true；否则，如果遍历还没有结束，把下一个键值对推入栈顶并返回true；如果表是空的，或者遍历已经结束，不用往栈里推入任何值，直接返回false即可。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202023-12-03%20221807.png)

上图，栈里原有4个值，其中索引2处是一个表，栈顶是该表的某个键。假设表还没有遍历结束，那么执行Next（2）之后，键会从栈顶弹出，取而代之的是表的下一个键值对。

通用for循环指令 (TFORCALL , TFORLOOP)

数值for循环是借助FORPREP和FORLOOP这两条指令来实现的。与之类似，通用for循环也是用TFORCALL和TFORLOOP这两条指令实现的。

```txt
./Luac.exe -p -l -l /D/Workspace/LearnLua/HelloWorld/res/script/instruction_for_common.lua

main <D:/Workspace/LearnLua/HelloWorld/res/script/instruction_for_common.lua:0,0> (11 instructions at 00F53AE8)
0+ params, 8 slots, 1 upvalue, 5 locals, 3 constants, 0 functions
        1       [1]     GETTABUP        0 0 -1  ; _ENV "pairs"
        2       [1]     GETTABUP        1 0 -2  ; _ENV "t"
        3       [1]     CALL            0 2 4
        4       [1]     JMP             0 4     ; to 9
        5       [2]     GETTABUP        5 0 -3  ; _ENV "print"
        6       [2]     MOVE            6 3
        7       [2]     MOVE            7 4
        8       [2]     CALL            5 3 1
        9       [1]     TFORCALL        0 2
        10      [1]     TFORLOOP        2 -6    ; to 5
        11      [3]     RETURN          0 1
constants (3) for 00F53AE8:
        1       "pairs"
        2       "t"
        3       "print"
locals (5) for 00F53AE8:
        0       (for generator) 4       11
        1       (for state)     4       11
        2       (for control)   4       11
        3       k       5       9
        4       v       5       9
upvalues (1) for 00F53AE8:
        0       _ENV    1       0 
```

类似数值for循环，编译器为了实现通用for循环也使用了三个特殊的局部变量，这三个特殊变量对应上面提到的_f、_s和_var。编译器给通用for循环生成的指令可以分为三个部分。第一部分指令利用in和do之间的表达式列表给三个特殊变量赋值，第二部分执行for循环体，第三部分就是TFORCALL和TFORLOOP指令。

以上面的输出为例，前三条指令可以用代码表示为_f、_s、_var=pairs（t），第四条是一个跳转指令，把控制跳转到TFORCALL指令。这四条指令属于第一部分；接下来的四条指令对应循环体，属于第二部分；然后是TFORCALL和TFORLOOP指令，属于第三部分。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202023-12-09%20231850.png)

上图是TFORCALL指令示意图，编译器使用的第一个特殊变量存放的就是迭代器。TFORCALL指令会使用其他两个特殊变量来调用迭代器，把结果保存到用户定义的变量里。

![](https://raw.githubusercontent.com/ZLam/LearnLua/main/Note/Photo/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202023-12-09%20231911.png)

上图是TFORLOOP指令示意图，如果迭代器返回的第一个值不是nil，则把该值拷贝到_var，然后跳转到循环体；若是nil，循环结束。

迭代器end


















自己动手实现lua 读书笔记 end



