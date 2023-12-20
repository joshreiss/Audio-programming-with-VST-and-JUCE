registerProcessor('noise-generator',class extends AudioWorkletProcessor {
  process(inputs, outputs) {
    for (let i=0;i<outputs[0][0].length;++i) outputs[0][0][i]=2*Math.random()-1
    return true
  }
})

registerProcessor('lowpass-filter', class extends AudioWorkletProcessor {
  static get parameterDescriptors() { return [{name:'c',defaultValue:0.5}] }
  constructor() {
    super()
    this.lastIn = 0
  }
  process(inputs, outputs, parameters) {
    let input = inputs[0],output = outputs[0]
    let coeff = parameters.c[0]
    for (let i = 0; i < outputs[0][0].length; ++i) {
      outputs[0][0][i]=inputs[0][0][i] * coeff +(1-coeff)*this.lastIn
      this.lastIn=inputs[0][0][i]
    }
    return true
  }
})
