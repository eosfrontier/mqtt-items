/* Leds animation module */

function animator(num, idle, pin) {
  this.write = require("neopixel").write.bind(null, (pin||0))
  this.num = num
  this.tick = 0
  this.anim = []
  this.anim_rpt = false
  this.curcol = new Uint8ClampedArray(num*3)
  this.rgborder = new Uint8Array([8,16,0])
  this.timer = null

  this.animations = {
    "inc":"r 1:00ff00,00aa00,005500 2:00aa00,005500,00ff00 3:005500,00ff00,00aa00",
    "out":"r 1:00ff00,00aa00,005500 2:00aa00,005500,00ff00 3:005500,00ff00,00aa00",
    "red":"r 0.5:ff0000 0.8:000000,550000,aa0000,ff0000 1.0:000000 1.2:ff0000,aa0000,550000,000000 1.5:ff0000",
    "nowifi":"r 1000:100008,000000 2000:000000 5000:000000 6000:000000,080010 7000:000000 10000:000000",
    "nosubs":"r 500:000108,000000 1000:000000,000810",
    "idle":"2:"+(idle||"000000")
  }

}

function animate(animator)
{
  if (!animator.anim) {
    clearTimeout(animator.timer)
    animator.timer = null
    return
  }
  let anim_end = animator.anim[animator.anim.length-1]
  let tm = getTime()
  let elaps = tm - animator.tick
  if (elaps > anim_end.tm) {
    if (!animator.anim_rpt) {
      for (let i=0; i<animator.curcol.length; i++) {
        animator.curcol[i] = anim_end.col[i%anim_end.col.length]
      }
      animator.write(animator.curcol)
      clearTimeout(animator.timer)
      animator.timer = null
      return
    }
    elaps = elaps % anim_end.tm
    animator.tick = tm-elaps
    animator.anim[0].col = anim_end.col
  }
  let pa = null
  animator.anim.some(function(a) {
    if (pa && elaps < a.tm) {
      let frac = (elaps-pa.tm)/(a.tm-pa.tm)
      for (let i=0; i<animator.curcol.length; i++) {
        animator.curcol[i] = pa.col[i%pa.col.length]*(1-frac)+a.col[i%a.col.length]*frac
      }
      animator.write(animator.curcol)
      return true
    }
    pa=a
    return false
  })
}

animator.prototype.set = function(str)
{
  let ap = this.animations[str]
  if (ap) { str = ap }
  this.anim_rpt = false
  let rgborder=this.rgborder
  this.anim = [{"tm":0,"col":new Uint8Array(this.curcol)}]
  str.split(" ").forEach(function(stp) {
    if (stp.toLowerCase() == 'r') {
      this.anim_rpt = true
    } else {
      let ar = stp.split(':')
      if (ar.length>1) {
        let tm = parseFloat(ar[0], 10)
        if (tm>0) {
          let colar = []
          ar[1].split(',').forEach(c => {
            let rgb = parseInt(c,16)
            colar.push.apply(colar, rgborder.map(s=>rgb>>s&255))
          })
          this.anim.push({"tm":tm, "col":new Uint8Array(colar)})
        }
      }
    }
  }, this)
  this.tick = getTime()
  if (!this.timer) {
    this.timer = setInterval(animate, 40, this)
  }
}

exports.init = function(num, idle, pin) {
  return new animator(num, idle, pin)
}
