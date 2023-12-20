let context= new AudioContext()
let lowFilter = new BiquadFilterNode(context,{type:'lowshelf',frequency:100})
let midlowFilter = new BiquadFilterNode(context,{type:'peaking',frequency:200,Q:3})
let midFilter = new BiquadFilterNode(context,{type:'peaking',frequency:400,Q:3})
let midhighFilter = new BiquadFilterNode(context,{type:'peaking',frequency:800,Q:3})
let highFilter = new BiquadFilterNode(context,{type:'highshelf',frequency:1600})
let finalGain = new GainNode(context)
Lows.oninput = () => lowFilter.gain.value=Lows.value
Midlows.oninput = () => midlowFilter.gain.value=Midlows.value
Mids.oninput = () => midFilter.gain.value=Mids.value
Midhighs.oninput = () => midhighFilter.gain.value=Midhighs.value
Highs.oninput = () => highFilter.gain.value=Highs.value
finalGain.connect(context.destination)
recorder = new Recorder(finalGain)
context.audioWorklet.addModule('worklets.js').then(() => {
  let myNoise = new AudioWorkletNode(context,'noise-generator')
  myNoise.connect(lowFilter)
  lowFilter.connect(midlowFilter)
  midlowFilter.connect(midFilter)
  midFilter.connect(midhighFilter)
  midhighFilter.connect(highFilter)
  highFilter.connect(finalGain)
})
