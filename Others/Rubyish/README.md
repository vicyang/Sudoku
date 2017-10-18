### Branch - RubyishOPT_02

* 解题效率（不含printf）：  
  * CPU: E5700 3.00GHz, gcc -O2  
    sudoku17.txt - Time used: 6.215

### Branch - RubyishOPT_03
explore 函数中添加 int lv 参数，取代全局 Head 变量
fill_one_possible_number 函数尝试优化

* 解题效率（不含printf）：  
  * CPU: E5700 3.00GHz, gcc -O2  
    sudoku17.txt - Time used: 6.085

### Branch - RubyishOPT_subfill
添加 rubyish 的 fill 函数以及相关常量

* Tag v0.1
  sudoku17.txt - Time used: 5.085
* Tag v0.2
  部分变量类型改为 static 和 register
  sudoku17.txt - Time used: 4.976

