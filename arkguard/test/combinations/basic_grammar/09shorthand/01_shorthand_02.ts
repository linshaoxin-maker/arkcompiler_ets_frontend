import assert from 'assert';

let name1 = 1;
let obj = {name1};
{
  const {name1 = 2} = obj;
  name1;
  assert(name1 === 1);
}
assert(obj.name1 === 1);

(function() {
  var s0;
  for ({s0=5} of [{s0:1}]) {
    assert(s0 === 1);
  }
})();
(function() {
  var s1;
  for ({s1:s1=5} of [{s1}]) {
    assert(s1 === 5);
  }
})();
(function() {
  let y;
  ({y=5} = {y:1})
  assert(y === 1);
})();
(function() {
  let y2:string, y3: {x:number};
  let obj:any = {};
  ({y2:y2 = '5', y3:y3={x:1}}=obj);
  assert(y2 === '5');
  assert(y3.x === 1);
})();
(function() {
  let z;
  ({z:z={x:5}}={z:{x:1}});
  assert(z.x === 1)
})();
