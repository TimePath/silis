# silis

static interchange lisp in stages

## build

```bash
cmake -H. -Bcmake-build-debug && cmake --build cmake-build-debug -- -j$(nproc) 
./cmake-build-debug/silis ./tests/hello.sil
```

## intrinsics

* `#cond (predicate: expr, true: expr, false: expr)`
    * evaluates `predicate`, branches to `true` if non-zero, else `false`
    * returns the value of the matching branch
    
* `#do (body: expr)`
    * evaluates all expressions in `body`
    * returning the value of the last expression

## todo

### language

* build standard library on top of intrinsics (#)

### type system

* nominal
* structural
* generic
    * use placeholders: #types/T0..n
    * anything produced by `const<T>` or `ref<T>` is equivalent if `T` is

### compiler

* targets
    * C
    * JavaScript (maybe in the form of TypeScript)

### editor

* structural
    * only knows how to handle intrinsics (#), and partially evaluate
* client/server architecture
    * use CRDTs for lag-free local echo
* clients
    * web
    * X11
    * GL
