var context = new AudioContext
let Noise = new AudioBufferSourceNode(context,{loop:true}),
    NoiseGain = new GainNode(context,{gain:0}),
    delay= new DelayNode(context,{delayTime:0.01}),
    feedbackGain= new GainNode(context,{gain:0.8}),
    lowPass = new BiquadFilterNode(context,{Q:-3.01,frequency:10000})
Noise.buffer = context.createBuffer(1,context.sampleRate,context.sampleRate)
for (i=0;i<context.sampleRate;i++) Noise.buffer.getChannelData(0)[i] = 2*Math.random()-1
Noise.start()
Noise.connect(NoiseGain)
NoiseGain.connect(delay)
delay.connect(lowPass)
lowPass.connect(feedbackGain)
feedbackGain.connect(delay)
delay.connect(context.destination)
Decay.oninput = function() {
  feedbackGain.gain.value=this.value
  DecayLabel.innerHTML = this.value
}
Delay.oninput = function() {
  delay.delayTime.value=0.001*this.value
  DelayLabel.innerHTML = this.value
}
Width.oninput = function() { WidthLabel.innerHTML = this.value}
Freq.oninput = function() {
  lowPass.frequency.value = this.value
  FreqLabel.innerHTML = this.value
}
Start.onclick = () => {
  context.resume()
  let now = context.currentTime
  NoiseGain.gain.setValueAtTime(0.5, now)
  NoiseGain.gain.linearRampToValueAtTime(0, now + Width.value/1000)
}