let called = 0
const it = {
  [Symbol.iterator]() {
    return this;
  },
  next() {
    called +=1;
    return {
      value: 42,
      done: true
    };
  }
}

const [a, b, ...c] = it;
print(called);
