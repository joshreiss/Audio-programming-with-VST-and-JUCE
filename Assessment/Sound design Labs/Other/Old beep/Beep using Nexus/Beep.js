//Define audio context
context = new AudioContext();
triggerBeep.on('change', function(v) {
  if (v) {
    context.resume()
    let now = context.currentTime;
    masterGain.gain.setValueAtTime(0.0, now);
    masterGain.gain.linearRampToValueAtTime(1, now + attack.value/1000);
    masterGain.gain.linearRampToValueAtTime(0, now + attack.value/1000 + decay.value/1000);
  }
});
frequency.on('change', function(v) {osc.frequency.value=frequency.value});
var masterGain = new GainNode(context,{gain:0});
var osc = new OscillatorNode(context,{type:'square',frequency:800});
osc.connect(this.masterGain);
masterGain.connect(context.destination);
osc.start();
