# DTaint README
# DTaint: taint analysis based on DFSan and llvm instrumentation
- This project contains propagation-based taint analysis based on llvm **DFSan**, and inference-based taint analysis **Memlog** based on llvm Pass, which are used for support memory feedback fuzzing.
## DFSan
- 一個沒有什麼卵用的玩具。
- 估計原因:外部呼叫跟隱性control flow
最近才發現[Greyone](https://www.usenix.org/system/files/sec20-gan.pdf)中已經有討論過DFSan在實際應用中的效果，跟我實際實作後的結論差不多。由於DFSan是透過編譯階段插樁實現的，對於許多外部呼叫缺乏對應的實作，造成propagation容易在程式執行初期便失去作用。導致這東西變成了無法在real world使用的花架子。 
### Features
#### Label Propagation
- Each memory byte is binded with a label, which represents the state of this byte(ex. tainted or non tainted).
- These labels will be propagated in run time.
- DFSan maps a large region of shadow memory to record labels for all memory bytes in target program.

#### Abilist
- Determine how DFSan handles each external function call.
    - **discard**
         - Do nothing, discard return value label.
    - **functional**
        - Do nothing, reserve return value label.
    - **custom**
        - Replace original function call with custom function call.
        - func -> __dfsan_func
        - DFSan already defined some glib functions.
        - This project covers common I/O functions.
        - user-defined.

#### What I modified
- **Original** 
    - DFSan only records 2 bytes of label, user needs to define the meaning of label itself(ex. 1 represent tainted, 0 means non tainted).
- **Modified**
    - Similiar to Angora, to understand the concept you can refer to [Angora](https://web.cs.ucdavis.edu/~hchen/paper/chen2018angora.pdf).
    -  Each label is binded with an unique input offset. When we found some instruction's operands is tainted with certain label, we also know which input offset can affect these operands.
    -  details 
        - Change label size from 2 bytes to 4 bytes.
        - Maintain a union table to record mapping between label and input offset.
        - New union operation.
        - Modify DFSan label API ot update new union table.
        - I/O wrapper, especially input, create each input offset, label for each input byte.
        
## Environment
- ```llvm-11 ```
- test on Ubuntu 20.04
## How to build
- Build DFSan, and Memlog passes with following commands.
- ```
   cd ~/DTaint
   mkdir build
   cd build
   cmake ..
  ```
- Option
    - -DDTAINT_DEBUG=ON/OFF
        - Output dtaint debug message.
    - -DMEMLOG_DEBUG=ON/OFF
        - Output memlog debug message.
    - ```
      cmake -DMEMLOG_DEBUG=ON ..
## Use DTaint/Memlog to compile target program
- default
    - memlog mode
- use dtaint mode
    - ```export DTAINT_MODE=1```
- ``` 
  export CC=build/clang-wrapper
  export CXX=build/clang-wrapper++
  // default use memlog mode
  ...
  build target
  // use dtaint mode
  export DTAINT_MODE=1
  ...
  build target
  ```
## Auto-generated abilist
        
- Auto-generated target_abilist.txt from src/abilist/target in compiler time.
- user-defined
    - you can use scripts in tools/.
    - **gen_library_abilist.sh**
        - When we want to test library, and harness will need certain abilist for library API. Auto-generated abilist from library .a, .so.
        - usage
            - ```./gen_library_abilist.sh [library .a, .so] > [output_file] [option]```
            - option
                - functional
                - discard
                - custom
        - Modified from Angora.
    - **gen_udr_abilist.sh**
        - Auto-generate abilist when we encounter undefind reference in compile time.
        - Store the error message manually.
            - ```make 2> erro_message```
        - usage
            - ```./gen_udr_abilist.sh [error message file] [output_file]```
    - Import generaged abilist into project.
        - three ways
            - Move generated abilist to src/abilist manually, and rebuild DFSan.
            - When compiling target program adds compiler option.
                
                - ```-mllvm -dtaint-dfsan-abilist=gen_abilist.txt```
            - gen_target_abilist.sh
                - Combine all abilist files in choosed directory. Since we have set clang-wrapper to import target_abilist defaultly. Similiar to way 1, we just generate target_abilist without rebuild DFSan. 
                - usage
                        - ```./gen_target_abilist.sh [input_dir] [output_file]```
                    - default output directory
                        - ```build/lib/share```

## See the output of DFSan
- **dump debug info**
    - DFSan supports a feature to store some information to file.
    - ``` export DFSAN_OPTIONS=dump_labels_at_exit=filename ```

## Example
- test/hook_test/va_arg_hook1_test.c
- ```=c
  #define MAXSIZE 512
  ...
  // read file to buf src[MAXSIZE]
  fread(src, 1, MAXSIZE, f);
  ...
  
  // size is affected by input
  size = (unsigned char)src[2] + (unsigned char)src[8];
  
  ...
  for(int i = 0; i < MAXSIZE; i++) {
    if (i >= MAXSIZE - 2) {
      // pointer index is controllable
      src[size + i] = '8';
    }
  }
  ```
- **input**
    - ```2434127653dsasdawqewqqw```
- when -DDTAINT_DEBUG=ON
- **output**
- ![](https://i.imgur.com/sne8lXc.png)
- input offset 2, 8 are tainted.

## Use with Fuzzer
- 本來是設計來搭配[AFLplusplus](https://github.com/AFLplusplus/AFLplusplus)使用，需要的runtime code也已經編進去了，包含shared memory跟forkserver的code，可以透過改過的AFLplusplus用另一個forkserver啟動DFSan插樁的target。
- 但現在已經是廢棄狀態，AFLplusplus那邊的code也打算廢棄掉了。

## Memlog
- Inference-based taint analysis需要的插樁code。
- 關鍵的inference-based taint code不在這裡:)
- 未來預定整進AFLplusplus，這裡就懶得講了。
## Reference
[Angora](https://web.cs.ucdavis.edu/~hchen/paper/chen2018angora.pdf)

[Greyone](https://www.usenix.org/system/files/sec20-gan.pdf)

[DataFlowSanitizer](https://clang.llvm.org/docs/DataFlowSanitizerDesign.html)

[LLVM Pass](https://llvm.org/docs/WritingAnLLVMPass.html)
