registerProcessor('noise-generator',class extends AudioWorkletProcessor {
  static get parameterDescriptors() { return [{name:'gain',defaultValue:0}] }
  process(inputs, outputs, parameters) {
    let linearGain = Math.pow(10,parameters.gain[0]/20)
    for (let i=0;i<outputs[0][0].length;++i) {
      outputs[0][0][i] = (2*Math.random()-1) * linearGain
    }
    return true
  }
})
