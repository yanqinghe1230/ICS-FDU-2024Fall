# Y86 Simulator Report

## CPU的设计
根据Y86指令集规范，构建了一个简单的CPU处理器模型，能够读取给出的`。yo`文件，输出`.yml`文件，并经测试程序验证正确。
总体设计可以分为三个部分：处理输入文件、cpu处理器模拟、格式化输出。
### 文件结构
```
liangcan-23307130417-PJ.zip 
    |__ report.md
    |__ project
        |__ tamplates
        |__ uploads
        |__ cpu
        |__ cpuins
        |__ main.py 
    |__ predefine.h
    |__ y86.cpp
    |__ y86.h 
    |__ run.cpp
    |__ runinitial.cpp
    |__ runins.cpp
```
其中project文件夹中是前端实现的相关文件，其中uploads用于存放前端上传的测试文件，cpu和cpuins是前端两个功能对应的可执行文件，分别由`run.cpp`和`runins.cpp`编译生成。
`runinitial.cpp`能格式化输出指定内容(其余run文件均为了实现相应前端功能，对于输出部分进行了相应的更改)
`y86.cpp`和`y86.h`定义了CPU类以及工具函数。
`predefine.h`预定义了各个寄存器、指令、状态等的编号，以及数据结构`wire`，用于CPU执行各阶段之间的数据传输。

### CPU的设计
#### CPU类的设计
CPU类中包含两部分，分别是成员函数与类内成员变量。
##### 成员变量
根据Programmer visible state，进行如下定义：
```cpp
private:
uint64_t PC=0; 
uint64_t REG[15]={0}; 
uint8_t MEM[mem_size]={0};
bool CC[3]={1,0,0};
wire route;

public:
uint8_t STAT=SAOK;
```
#### 工具函数
* `bool in(std::initializer_list<uint8_t> ins)`:用于判断当前指令是否在某个满足要求的指令集中
* `uint8_t gethigh(uint8_t val)`、`uint8_t getlow(uint8_t val)`:读取一个字节的前四位\后四位数据，用于分离icode和ifun
* `uint64_t getval(uint64_t offset)`:从一个地址开始往后读取八字节的内容，用于读入立即数
#### 成员函数
为每一个阶段均设计了一个函数进行实现
* `void fetch()`：
    1. 检查PC地址是否有效，无效则抛出imem_error
    2. 检查指令是否有效，无效则设置instr_invalid为1表示异常
    3. 读取指令内容，并计算下一状态的PC，存放在valP中
* `void decode()`:使用若干set函数，设置sources和destination寄存器，并读取相应数值
* `void execute()`:进行逻辑运算
    1. 当指令为operation instruction时，根据计算结果设置conditional code
    2. 当指令为conditional instruction时，根据CC和ifun设置cnd，当且仅当cnd为1时执行指令要求的操作
* `void memory()`:
    1. 读取内存内容或是往内存中写入内容
    2. 检查所需要操作的内存地址是否有效，无效则抛出dmem_error
    3. 设置PC状态
* `void updatepc()`: 当无异常抛出时，更新PC值，以执行下一条指令
最后用函数`void runcpu()`将以上阶段综合。
### 处理输入文件
`.yo`文件的格式如下：
`0x000: 30f40002000000000000 | 	irmovq stack, %rsp  	# Set up stack pointer`
分别是：当前指令的地址: 指令内容 | 对指令内容的解释。且指令地址和指令均用十六进制输入。
使用getline按行读取，分开处理地址和指令：地址部分从字符串的第三个字符开始读入，读到`:`停止；指令部分直接从字符串的第八个字符开始读入，读到` `停止。
由于MEM中每一个元素用一个字节存储，故读取指令时需要以两位为单位读取再存储。因此设计了一个工具函数：`hex_to_dec`，将十六进制数转换为十进制数便于计算。

### 格式化输出
* `void output()`：用于输出规定格式的yml文件。通用输出不同的空格数，以替代调用yml库控制格式。(在编译用于前端运行的可执行文件时，由于无需输出内存值，手动注释掉相关代码之后进行的编译，后为了能正常通过测试去掉了注释)

* `void outins()`：用于输出当前执行的指令内容(用于前端输出)
## 前端的设计
### 功能描述
为了便于演示，前端总共设计了以下功能：
1. 用户可以上传所需要执行的`.yo`文件
2. 用户可以选择执行模式： display full result 和 step by step simulation
    * display full result:只显示执行结果。如果正常执行则输出“Execution successful!”，否则输出错误信息以及出错的指令。
    * step by step simulation：可以逐步显示当前的程序步数、执行的指令、寄存器内容以及CC
### 具体实现
使用Flask实现后端，HTML和CSS控制页面格式，使用JavaScript中的AJAX进行前后端交互。
共设计了以下端口：
1. upload：用于上传文件
2. run：用于运行模拟器。其中每个模式对应一个可执行文件，根据前端获取的模式信息传递到后端，选择相应的可执行文件运行并输出结果。
在这里没有让run主程序输出为json文件，而是用Flask直接捕获程序的输出流，使用jsonify工具将其转换为json文件传递到前端；相应的，前端到后端的内容传递是使用AJAX，直接返回json格式的数据。
3. step: 用于逐步展示运行结果。使用变量currentstep计程序步数，该变量在每次调用run端口时（也就是进行一次新的simulate时）被初始化为0。
在每次点击step后，会调用一个update函数更新按钮状态。该函数功能是：如果currentstep大于总程序步数时，会将按钮变成无法点击的灰色状态。
## 代码运行方式
* 在终端中运行：
    1. 运行`g++ runinitial.cpp -o cpuinitial`进行编译
    2. 运行`./cpuinitial < example.yo >example.yml`
* 使用前端：
    1. 在`project`目录下运行：`python main.py`
    2. 在浏览器中打开相应地址
    3. 点击“选择文件”，选择需要执行的本地`.yo`文件，再点击“upload”按钮进行上传，若出现“File uploaded successfully!”说明上传成功。
    4. 在select mode下的下拉框中选择模式，然后点击“run simulator"，便可看到结果显示在右侧。若选择的是step by step，则会出现next step按钮。点击该按钮便可以逐步查看运行结果。