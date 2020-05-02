function pair(a, b) {
  function inner(getter) {
    return getter(a, b);
  }
  return inner;
}

function first(pair) {
  function getFirst(a, b) {
    return a;
  }
  return pair(getFirst);
}

function second(pair) {
  function getSecond(a, b) {
    return b;
  }
  return pair(getSecond);
}

function sum(list) {
  if (list === undefined) {
    return 0;
  }
  return first(list) + sum(second(list));
}

function main() {
  return sum(pair(1, pair(2, pair(3, pair(4)))));
}

console.log(main());
