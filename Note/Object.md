<!--
 * @Author: linzehui
 * @Date: 2023-01-14 21:59:03
-->

# Lua 的数据类型
## 能表达的类型
```lua
local v = 666
v = "haha"
v = funciton(a, b)
    return a + b
end
v = {}
```
通常我们使用 lua 的时候都知道 lua 可以类似上面那样声明个变量并对它赋值不同类型的值，这是动态语言的特性，但底层上它是怎么实现出来的呢。
```c
// Lua/Lualib/src/lua.h

/*
** basic types
* 
* lua 能表达的各种数据类型
*/
#define LUA_TNONE		(-1)        // 应该是在底层表示 无效 无意义之类 用的

#define LUA_TNIL		0           // 空类型
#define LUA_TBOOLEAN		1       // 布尔类型
#define LUA_TLIGHTUSERDATA	2       // void* 指针，指向的内容的生命周期由外部管理，lua不管的
#define LUA_TNUMBER		3           // 数字类型
#define LUA_TSTRING		4           // 字符串类型
#define LUA_TTABLE		5           // table 类型
#define LUA_TFUNCTION		6       // 函数类型
#define LUA_TUSERDATA		7       // void* 指针，指向的内容的生命周期由 lua 管，注册进 lua 的 gc 模块
#define LUA_TTHREAD		8           // lua 虚拟机，协程

#define LUA_NUMTYPES		9       // lua 脚本的类型数量
```
由上面可以看出 lua 脚本上能表达的类型一共就 9 种。那么怎么在底层设计一种通用的结构表达多种类型呢。
## 通用的结构
```c
// Lua/Lualib/src/lobject.h

/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)     表示实际类型是什么（ nil, number, bool, func, table 等等 ）
** bits 4-5: variant bits                       扩展数据位，感觉就是类型的 meta 信息，比如 number 有 int 或 float，bool 有 true 或 false，string 有短字符串或长字符串之类
** bit 6: whether value is collectable          表示是否由 lua gc 管理
*/

/**
* add variant bits to a type
* 
* 就是用来加上扩展数据位的，类型的 meta 信息
*/
#define makevariant(t,v)	((t) | ((v) << 4))

/*
** Union of all Lua values
* 
* 能表示 lua 脚本上所有类型的结构
*/
typedef union Value {
  struct GCObject *gc;    /* collectable objects */
  void *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_Number n;    /* float numbers */
} Value;

/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#define TValuefields	Value value_; lu_byte tt_

/**
* 其实就是 Value 搭上个 tt_(unsigned char) 能表示上 Value 是什么数据类型
*/
typedef struct TValue {
  TValuefields;
} TValue;

/**
* raw type tag of a TValue
* 
* 返回 TValue 类型对象的 tt_ 属性
*/
#define rawtt(o)	((o)->tt_)

/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
* 
* 所有 gc 类型的通用 header
*/
#define CommonHeader	struct GCObject *next; lu_byte tt; lu_byte marked


/**
* Common type for all collectable objects
* 
* 理解成 gc 类型的基类吧
*/
typedef struct GCObject {
  CommonHeader;
} GCObject;


/**
* Bit mark for collectable types
* 
* 表示是否 gc 类型，第 6 位是不是 1
*/
#define BIT_ISCOLLECTABLE	(1 << 6)

/**
* 判断 TValue 是否需要 gc ，其实就是睇 TValue 的 tt_ 的第 6 位是不是 1
*/
#define iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)

/**
* mark a tag as collectable
* 
* 加上 gc 标志位
*/
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)

/*
** Header for a string value.
*/
typedef struct TString {
  CommonHeader;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  lu_byte shrlen;  /* length for short strings */
  unsigned int hash;
  union {
    size_t lnglen;  /* length for long strings */
    struct TString *hnext;  /* linked list for hash table */
  } u;
  char contents[1];
} TString;

typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of size of 'node' array */
  unsigned int alimit;  /* "limit" of 'array' array */
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  struct Table *metatable;
  GCObject *gclist;
} Table;
```
```c
// Lua/Lualib/src/lstate.h

/*
** Union of all collectable objects (only for conversions)
** ISO C99, 6.5.2.3 p.5:
** "if a union contains several structures that share a common initial
** sequence [...], and if the union object currently contains one
** of these structures, it is permitted to inspect the common initial
** part of any of them anywhere that a declaration of the complete type
** of the union is visible."
*/
union GCUnion {
  GCObject gc;  /* common header */
  struct TString ts;
  struct Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct lua_State th;  /* thread */
  struct UpVal upv;
};


/*
** ISO C99, 6.7.2.1 p.14:
** "A pointer to a union object, suitably converted, points to each of
** its members [...], and vice versa."
*/
#define cast_u(o)	cast(union GCUnion *, (o))
```
上面列了一大堆源码，主要都是说明这个能表达多个类型的通用结构。  
1. **类型分 没gc 和 可gc**  
    lua 脚本的类型根据 tt_ 值中的第 6 位判断该类型是不是 可gc 。可gc 的类型可以看到它们结构都包含 CommonHeader 宏（例如 String，Table等）。所以 没gc 的类型是 nil ， bool ， lightUserData ， number 。 可gc 的类型是 string ， table ， function ， userData ， thread 。 可gc 的类型在设置它们的 tt_ 的时候会用 ctb 宏加上 gc 标志位。
2. **可gc 类型的联合体**  
    可gc 类型都包含 CommonHeader 宏，并在类型声明的最开始部分。所以可以将 CommonHeader 这共有部分的整体理解为一个基类，而其他 可gc 类型均从这个基类中继承下来，所以它们的结构体声明开始部分都是这个成员。而 GCUnion 这个联合体把所有 可gc 类型都包含了进来，同时自己声明最开始的部分是 GCObject 类型，实际同样是 CommonHeader 的结构，所以根据 C99 标准， GCObject 可以强转 GCUnion 。到此，所有 可gc 类型都可以用 GCObject 表示 ，同时 GCObject 可以转 GCUnion 读回具体的值。
3. **GCObject**  
    GCObject 类似基类表示了所有 可gc 类型。 next 属性指向下一个GC链表的成员， tt 表示数据的类型， marked gc 用的标记。
4. **表示脚本所有的类型**  
    上面已经能把多种 可gc 类型表示出来，那么同理，把 可gc 和 没gc 的类型包含在一起就能表示脚本所有的类型，该结构就是 Value 联合体。  
5. **通用结构**  
    脚本所有的类型已经能表示出来了，那么最后加上一个字段说明当前数据实际是什么类型，最终这个通用结构就出来了，该结构就是 TValue 。  
## 总结




（疑问 实际 可以 GCObject 转 TString 这样的嚒）
