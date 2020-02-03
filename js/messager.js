/* Messaging over UDP module */

const mappings = [
  ["eos/portal/buttons_in/button/b1","idle","set","red"],
  ["eos/portal/buttons_in/button/b2","idle","set","inc"],
  ["eos/portal/buttons_out/button/b2","idle","set","out"],
  ["eos/portal/.*/button/.*",".*","set","idle"]
]

const subscribes = [
  "eos/portal/*/button/*"
]

function messager(leds, name, port)
{
  this.leds=leds
  this.name=name||"eos/portal/light"
  this.port=port||1883
  this.socket=require("dgram").createSocket("udp4")
  this.socket.on('message', receive.bind(this))
  this.socket.bind(this.port)
  this.subs=[]
  subscribe(this)
  setInterval(subscribe, 30000, this)
}

function subscribe(msgr)
{
  msgr.send("status",JSON.stringify({battery:Math.round(607*analogRead())/100,connected:true}))
  let ip=require("Wifi").getIP()
  let nm=ip.netmask.split(".")
  let bc=ip.ip.split(".").map((o,i)=>o|(nm[i]^255))
  subscribes.forEach(s => msgr.socket.send("SUB\n"+s, msgr.port, bc))
}

function strmatch(patt, str)
{
  return RegExp("^"+patt.replace(/\*/g,"[^/]*")+"$").test(str)
}

messager.prototype.send = function(topic, msg)
{
  this.subs.forEach(s => msg_send_sub(this,topic,msg,s), this)
}

function msg_send_sub(msgr, topic, msg, sub)
{
  if (strmatch(sub.topic, topic)) {
    msgr.socket.send(msgr.name+"/"+topic+"\n"+msg, msgr.port, sub.ip)
  }
}

function add_sub(msgr, topic, rinfo)
{
  let parts=topic.split('/')
  let namelen=msgr.name.split('/').length
  let prefix=parts.slice(0,namelen).join('/')
  let postfix=parts.slice(namelen).join('/')
  if (RegExp("^"+prefix.replace(/\*/g,"[^/]*")).test(msgr.name)) {
    let ip=rinfo.address
    let entry=msgr.subs.find(e => e.topic==postfix && e.ip==ip)
    if (!entry) {
      entry = {"topic":postfix,"ip":ip,"last":0}
      msgr.subs.push(entry)
    }
    entry["last"]=getTime()
  }
}

function receive(msg, rinfo)
{
  let mar=msg.split("\n")
  console.log("Received: "+mar[0]+" -> "+mar[1]+"\n")
  if (mar[0] == "SUB") {
    add_sub(this, mar[1], rinfo)
  } else if (mar[0] == this.name+"/set") {
    this.leds.set(mar[1])
    this.send("ack", mar[1])
  } else {
    let mapped = mappings.find(mp => strmatch(mp[0],mar[0]) && strmatch(mp[1],mar[1]))
    if (mapped) {
      if (mapped[2] == "set") {
        let msg = mapped[3] || mar[1]
        this.leds.set(msg)
        this.send("ack", msg)
      }
    }
  }
}

exports.setup = function(leds) {
  return new messager(leds)
}
