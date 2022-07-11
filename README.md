[In Next Weekend教程解析](Analysis/NextWeek.md)
[In One Weekend教程解析](Analysis/InOneWeekend.md)


## 过程中需要复习的知识
### 1. `static_cast`
```cpp
1. static_cast是一个c++运算符，功能是把一个表达式转换为某种类型，但没有运行时类型检查来保证转换的安全性。
2. 向下取值
```


### 2. `inline`内联函数
```cpp
在 c/c++ 中，为了解决一些频繁调用的小函数大量消耗栈空间（栈内存）的问题，特别的引入了 inline 修饰符，表示为内联函数。

栈空间就是指放置程序的局部数据（也就是函数内数据）的内存空间。

在系统下，栈空间是有限的，假如频繁大量的使用就会造成因栈空间不足而导致程序出错的问题，如，函数的死循环递归调用的最终结果就是导致栈内存空间枯竭。
```
https://www.runoob.com/w3cnote/cpp-inline-usage.html


### 3. 运算符重载

### 4. const
### 5. 虚函数
### 6. shared_ptr
### 7. 继承

> 好多采样都是50，所以跟图片跟教程有出入

[知乎大佬笔记](https://www.zhihu.com/column/c_1504519612624035840)