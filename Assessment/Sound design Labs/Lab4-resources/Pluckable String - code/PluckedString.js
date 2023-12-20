var recorder
var context = new AudioContext
let output = new GainNode(context)
output.connect(context.destination)
//set up call backs from interface
Decay.oninput = function() {
  DecayLabel.innerHTML = this.value
}
Delay.oninput = function() {
  DelayLabel.innerHTML = this.value
}
Width.oninput = function() {
  WidthLabel.innerHTML = this.value
}
Freq.oninput = function() {
  FreqLabel.innerHTML = this.value
}
Source.oninput = function() {
}
Play.onclick = function() {
  context.resume()
}

// Recording
recorder = new Recorder(output) //change output to whichever node you want to record
Start.onclick = () => {
  context.resume()
  recorder.record()
}
Stop.onclick = () => {
  recorder.stop()
  recorder.exportWAV(blob => document.querySelector("audio").src = URL.createObjectURL(blob) )
}
