# NotJS

Very naive attempt to do a JS engine in C++17. Doing this for learning purposes.

## To-Do

1. [ ] Passing parameters to a function
2. [ ] Recursion
3. [ ] Booleans, comparision operators
4. [ ] Ternary expression 
5. [ ] Parser
6. [ ] Explore JIT and inline caching techniques

## Notes

- For naming and AST structure I'm referring to the [TypeScript AST](https://ts-ast-viewer.com/).

## Compile and run

```
g++ -std=c++17 -I./notjs notjs/main.cpp && ./a.out
```
