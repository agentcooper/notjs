function sum(a) {
  function inner(b) {
    return a + b;
  }

  return inner;
}

function main() {
  return sum(40)(2);
}
