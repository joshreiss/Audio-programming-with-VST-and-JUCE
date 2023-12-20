var context = new AudioContext
let Noise = new AudioBufferSourceNode(context,{loop:true}),
    NoiseGain = new GainNode(context,{gain:0})
Noise.buffer = context.createBuffer(1,context.sampleRate,context.sampleRate)
for (i=0;i<context.sampleRate;i++) Noise.buffer.getChannelData(0)[i] = 2*Math.random()-1
Noise.start()
Noise.connect(NoiseGain)
NoiseGain.connect(context.destination)
Width.oninput = function() { WidthLabel.innerHTML = this.value}
Start.onclick = () => {
  context.resume()
  let now = context.currentTime
  NoiseGain.gain.setValueAtTime(0.5, now)
  NoiseGain.gain.linearRampToValueAtTime(0, now + Width.value/1000)
}
