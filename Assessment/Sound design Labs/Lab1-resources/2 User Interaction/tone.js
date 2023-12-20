var context = new AudioContext()
var Tone = new OscillatorNode(context)
var Amplitude = new GainNode(context,{gain:0.2})
Tone.start()
var Connected = 0 //Oscillator is not connected in the beginning, so silence
Amplitude.connect(context.destination) // connect Gain node to output (speakers or heaphones)
// Connects/Disconnects the oscillator to the graph
function StartStop() {
  context.resume()
  if (Connected == false) {
    Tone.connect(Amplitude)
    Connected = true
  } else {
    Connected = false
    Tone.disconnect(Amplitude)
  }
}
Frequency.oninput = function () {
  Tone.frequency.value = this.value
  FrequencyLabel.innerHTML = this.value
}
Volume.oninput = function () {
  Amplitude.gain.value = this.value
  VolumeLabel.innerHTML = this.value
  console.log(Amplitude)
}
Type.onchange = function () { Tone.type = this.value }
