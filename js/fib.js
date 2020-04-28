function fib(n) {
  if (n === 1) {
    return 1;
  }

  if (n === 2) {
    return 1;
  }

  return fib(n - 1) + fib(n - 2);
}

function main() {
  return fib(25);
}

console.log(main());
