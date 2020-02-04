/* Button module */

function buttons(msgr, btns)
{
  this.msgr=msgr
  this.btns=btns||[]
  for (p in this.btns) {
    pinMode(p, 'input_pullup')
    setWatch(pressed.bind(this, p), p, {repeat:true, edge:'falling', debounce:25})
  }
}

function pressed(pin)
{
  let str = []
  for (p in this.btns) {
    if (p == pin || !digitalRead(p)) {
      str.push(this.btns[p])
    }
  }
  this.msgr.send("button/"+str.join("_"))
}

exports.init = function(msgr, btns) {
  return new buttons(msgr, btns)
}
