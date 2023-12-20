registerProcessor('noise-generator',class extends AudioWorkletProcessor {
  process(inputs, outputs) {
    for (let i=0;i<outputs[0][0].length;++i)  outputs[0][0][i]=2*Math.random()-1
    return true
  }
})

registerProcessor('gain-processor',class extends AudioWorkletProcessor {
  static get parameterDescriptors() { return [{name:'gain',defaultValue:0}] }
  process(inputs, outputs, parameters) {
    const input = inputs[0],output = outputs[0]
    let linearGain = Math.pow(10,parameters.gain[0]/20)
    for (let channel=0;channel<inputs[0].length;++channel)
      for (let i=0;i<input[channel].length;++i) {
        output[channel][i] = input[channel][i] * linearGain
      }
    return true
  }
})
