function pair(a, b) {
  function inner(c) {
    return c(a, b);
  }
  return inner;
}

function first(pair) {
  function getFirst(a, b) {
    return a;
  }
  return pair(getFirst);
}

function second(p) {
  function getSecond(a, b) {
    return b;
  }
  return p(getSecond);
}

function print(list) {
  if (list === null) {
    return;
  }
  console.log(first(list));
  print(second(list));
}

const list = pair(1, pair(2, pair(3, pair(4, null))));

print(list);
