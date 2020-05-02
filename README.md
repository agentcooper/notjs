# NotJS

Very naive attempt to do a JS engine in C++17. Doing this for learning purposes.

## To-Do

1. [x] Passing parameters to a function
2. [x] Recursion
3. [x] Booleans, comparision operators
4. [x] Ternary expression
5. [x] Let
6. [x] Closures
7. [ ] Parser
8. [ ] Explore JIT and inline caching techniques

## Notes

- I'm intentionally keeping everything in one file for now.
- For naming and AST structure I'm referring to the [TypeScript AST](https://ts-ast-viewer.com/).

## Compile and run

```
g++ -std=c++17 -O3 -Wall -I ./notjs ./notjs/main.cpp && time ./a.out
```
