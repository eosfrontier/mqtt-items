/* Messaging over UDP module */

let wifi=require("Wifi")

//const mappings = [
//  ["eos/portal/buttons_in/button/b1","idle","set","red"],
//  ["eos/portal/buttons_in/button/b2","idle","set","inc"],
//  ["eos/portal/buttons_out/button/b2","idle","set","out"],
//  ["eos/portal/*/button/*","*","set","idle"]
//]

//const subscribes = [
//  "eos/portal/*/button/*"
//]

const mappings = [
  ["eos/portal/light/ack","*","set",null]
]

const subscribes = [
  "eos/portal/light/ack"
]

function messager(actions, name, port)
{
  this.actions=actions||{}
  this.name=name||"eos/portal/light"
  this.port=port||1883
  this.socket=require("dgram").createSocket("udp4")
  this.socket.on('message', receive.bind(this))
  this.socket.bind(this.port)
  this.subs=[]
  this.state="nowifi"
  subscribe(this)
  setInterval(subscribe, 30000, this)
}

function subscribe(msgr)
{
  if (wifi.getStatus().station != "connected") {
    if (msgr.state != "nowifi") {
      msgr.setstate("nowifi")
      msgr.subs=[]
    }
  }
  if (msgr.state == "nowifi") {
    msgr.setstate("nosubs")
  }
  msgr.publish("status",JSON.stringify({battery:Math.round(607*analogRead())/100,connected:true,state:msgr.state}))

  let timeout=getTime()-100
  msgr.subs = msgr.subs.filter(s => s.last>timeout)
  if (!msgr.subs.length) {
    msgr.setstate("nosubs")
  }

  let ip=wifi.getIP()
  let nm=ip.netmask.split(".")
  let bc=ip.ip.split(".").map((o,i)=>o|(nm[i]^255))
  subscribes.forEach(s => msgr.socket.send("SUB\n"+s, msgr.port, bc))
}

function strmatch(patt, str, prefix) {
  let sa=str.split('/')
  return patt.split('/').every((p,i)=>(p=='*'||p==sa[i]))
}

messager.prototype.publish = function(topic, msg)
{
  if (!msg) { msg = this.state }
  console.log("Sending: "+topic+" -> "+msg)
  this.subs.forEach(s => msg_send_sub(this,topic,msg,s), this)
}

messager.prototype.setstate = function(state)
{
  if (this.state != state) {
    this.state = state
    if (this.actions.set) { this.actions.set(state) }
  }
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
  if (strmatch(prefix, msgr.name)) {
    let ip=rinfo.address
    let entry=msgr.subs.find(e => e.topic==postfix && e.ip==ip)
    if (!entry) {
      entry = {"topic":postfix,"ip":ip,"last":0}
      msgr.subs.push(entry)
      subscribes.forEach(s => msgr.socket.send("SUB\n"+s, msgr.port, ip))
    }
    entry["last"]=getTime()
    if (msgr.state == "nosubs") {
      msgr.setstate("idle")
    }
    msg_send_sub(msgr, "ack", msgr.state, entry)
  }
}

function receive(msg, rinfo)
{
  let mar=msg.split("\n")
  console.log("Received("+rinfo.address+"): "+mar[0]+" -> "+mar[1]+"\n")
  if (mar[0] == "SUB") {
    add_sub(this, mar[1], rinfo)
  } else if (mar[0].startsWith(this.name+"/")) {
    let topic = mar[0].substr(this.name.length+1)
    let act = this.actions[topic]
    if (act) {
      act(mar[1])
      this.publish("ack", mar[1])
      if (topic == "set") { this.state = mar[1] }
    }
  } else {
    let mapped = mappings.find(mp => (strmatch(mp[0],mar[0]) && strmatch(mp[1],mar[1])))
    if (mapped) {
      let act = this.actions[mapped[2]]
      if (act) {
        let msg = mapped[3] || mar[1]
        act(msg)
        this.publish("ack", msg)
        if (mapped[2] == "set") { this.state = msg }
      }
    }
  }
}

exports.init = function(leds, name) {
  return new messager({set:c=>leds.set(c)}, name)
}
