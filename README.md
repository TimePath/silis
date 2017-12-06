# silis

static interchange lisp in stages

## build

```bash
cmake -H. -Bcmake-build-debug && cmake --build cmake-build-debug -- -j$(nproc) 
./cmake-build-debug/silis ./tests/hello.sil
```

## benchmark

```
nix-build -A silis-static release.nix && time ./result/bin/silis tests/hello.sil | wc -l
nix-build -A silis-musl-static release.nix && time ./result/bin/silis tests/hello.sil | wc -l
```

## profile

```
valgrind --tool=callgrind ./cmake-build-debug/silis ./tests/hello.sil >/dev/null
```

`kcachegrind` the generated `callgrind.out.${PID}` file

## intrinsics

* `#cond (predicate: expr, true: expr, false: expr)`
    * evaluates `predicate`, branches to `true` if non-zero, else `false`
    * returns the value of the matching branch
    
* `#do (body: expr)`
    * evaluates all expressions in `body`
    * returning the value of the last expression

* `#while (predicate: expr, body: expr)`
    * evaluates `predicate`, branches to `body` if non-zero
    * repeat

## todo

### generators

* place all variables (including arguments) into a struct, redirect reads/writes
* assign each yield point a number
* return an object with current index + state
    * .next() calls original function (since rewritten with no args)
        * passes `this`
        * yield: sets index, returns value  
        * original function knows how to jump to a yield point
            * or each part is its own function
 
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
