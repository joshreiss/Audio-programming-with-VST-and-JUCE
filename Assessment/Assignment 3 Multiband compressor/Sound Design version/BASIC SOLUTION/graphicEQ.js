let context= new AudioContext()
let lowFilter = new BiquadFilterNode(context,{type:'lowshelf',frequency:200})
let midFilter = new BiquadFilterNode(context,{type:'peaking',frequency:600,Q:5})
let highFilter = new BiquadFilterNode(context,{type:'highshelf',frequency:1000})
Lows.oninput = () => lowFilter.gain.value=Lows.value
Mids.oninput = () => midFilter.gain.value=Mids.value
Highs.oninput = () => highFilter.gain.value=Highs.value
context.audioWorklet.addModule('worklets.js').then(() => {
  let myNoise = new AudioWorkletNode(context,'noise-generator')
  myNoise.connect(lowFilter).connect(midFilter).connect(highFilter).connect(context.destination)
})
