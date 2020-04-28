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

function print(list) {
  if (list === undefined) {
    return;
  }
  console.log(first(list));
  print(second(list));
}

const list = pair(1, pair(2, pair(3, pair(4, undefined))));

print(list);
