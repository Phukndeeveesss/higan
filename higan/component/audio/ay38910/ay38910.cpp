#include <emulator/emulator.hpp>
#include "ay38910.hpp"

namespace higan {

#include "serialization.cpp"

auto AY38910::clock() -> array<uint4[3]> {
  toneA.clock();
  toneB.clock();
  toneC.clock();
  noise.clock();
  envelope.clock();

  array<uint4[3]> output;
  if((toneA.output | channelA.tone) & (noise.output | channelA.noise)) {
    output[0] = channelA.envelope ? envelope.output : channelA.volume;
  }
  if((toneB.output | channelB.tone) & (noise.output | channelB.noise)) {
    output[1] = channelB.envelope ? envelope.output : channelB.volume;
  }
  if((toneC.output | channelC.tone) & (noise.output  |channelC.noise)) {
    output[2] = channelC.envelope ? envelope.output : channelC.volume;
  }
  return output;
}

auto AY38910::Tone::clock() -> void {
  if(++counter >= period) {
    counter = 0;
    output ^= 1;
  }
}

auto AY38910::Noise::clock() -> void {
  if(++counter >= period) {
    counter = 0;
    if(flip ^= 1) {
      output = !lfsr.bit(0);
      lfsr = (lfsr.bit(0) ^ lfsr.bit(3)) << 16 | lfsr >> 1;
    }
  }
}

auto AY38910::Envelope::clock() -> void {
  if(!holding && ++counter >= period) {
    counter = 0;
    if(!attacking) {
      if(output !=  0) return (void)output--;
    } else {
      if(output != 15) return (void)output++;
    }

    if(!repeat) {
      output = 0;
      holding = 1;
    } else if(hold) {
      if(alternate) output ^= 15;
      holding = 1;
    } else if(alternate) {
      attacking ^= 1;
      if(!attacking) {
        output--;
      } else {
        output++;
      }
    } else {
      output = !attacking ? 15 : 0;
    }
  }
}

auto AY38910::read() -> uint8 {
  uint8 data;

  switch(io.register) {
  case  0:
    data.bits(0,7) = toneA.period.bits(0, 7);
    break;
  case  1:
    data.bits(0,3) = toneA.period.bits(8,11);
    break;
  case 2:
    data.bits(0,7) = toneB.period.bits(0, 7);
    break;
  case 3:
    data.bits(0,3) = toneB.period.bits(8,11);
    break;
  case  4:
    data.bits(0,7) = toneC.period.bits(0, 7);
    break;
  case  5:
    data.bits(0,3) = toneC.period.bits(8,11);
    break;
  case  6:
    data.bits(0,4) = noise.period;
    break;
  case  7:
    data.bit(0) = channelA.tone;
    data.bit(1) = channelB.tone;
    data.bit(2) = channelC.tone;
    data.bit(3) = channelA.noise;
    data.bit(4) = channelB.noise;
    data.bit(5) = channelC.noise;
    data.bit(6) = portA.direction;
    data.bit(7) = portB.direction;
    break;
  case  8:
    data.bits(0,3) = channelA.volume;
    data.bit (4)   = channelA.envelope;
    break;
  case  9:
    data.bits(0,3) = channelB.volume;
    data.bit (4)   = channelB.envelope;
    break;
  case 10:
    data.bits(0,3) = channelC.volume;
    data.bit (4)   = channelC.envelope;
    break;
  case 11:
    data.bits(0,7) = envelope.period.bits(0, 7);
    break;
  case 12:
    data.bits(0,7) = envelope.period.bits(8,15);
    break;
  case 13:
    data.bit(0) = envelope.hold;
    data.bit(1) = envelope.alternate;
    data.bit(2) = envelope.attack;
    data.bit(3) = envelope.repeat;
    break;
  case 14:
    data.bits(0,7) = 0xff;  //portA.data
    break;
  case 15:
    data.bits(0,7) = 0xff;  //portB.data
    break;
  }

  return data;
}

auto AY38910::write(uint8 data) -> void {
  switch(io.register) {
  case  0:
    toneA.period.bits(0, 7) = data.bits(0,7);
    break;
  case  1:
    toneA.period.bits(8,11) = data.bits(0,3);
    break;
  case  2:
    toneB.period.bits(0, 7) = data.bits(0,7);
    break;
  case  3:
    toneB.period.bits(8,11) = data.bits(0,3);
    break;
  case  4:
    toneC.period.bits(0, 7) = data.bits(0,7);
    break;
  case  5:
    toneC.period.bits(8,11) = data.bits(0,3);
    break;
  case  6:
    noise.period = data.bits(0,4);
    break;
  case  7:
    channelA.tone   = data.bit(0);
    channelB.tone   = data.bit(1);
    channelC.tone   = data.bit(2);
    channelA.noise  = data.bit(3);
    channelB.noise  = data.bit(4);
    channelC.noise  = data.bit(5);
    portA.direction = data.bit(6);
    portB.direction = data.bit(7);
    break;
  case  8:
    channelA.volume   = data.bits(0,3);
    channelA.envelope = data.bit (4);
    break;
  case  9:
    channelB.volume   = data.bits(0,3);
    channelB.envelope = data.bit (4);
    break;
  case 10:
    channelC.volume   = data.bits(0,3);
    channelC.envelope = data.bit (4);
    break;
  case 11:
    envelope.period.bits(0, 7) = data.bits(0,7);
    break;
  case 12:
    envelope.period.bits(8,15) = data.bits(0,7);
    break;
  case 13:
    envelope.hold      = data.bit(0);
    envelope.alternate = data.bit(1);
    envelope.attack    = data.bit(2);
    envelope.repeat    = data.bit(3);
    envelope.holding   = 0;
    envelope.attacking = envelope.attack;
    envelope.output    = !envelope.attacking ? 15 : 0;
    break;
  case 14:
    portA.data = data;
    break;
  case 15:
    portB.data = data;
    break;
  }
}

auto AY38910::select(uint4 data) -> void {
  io.register = data;
}

auto AY38910::power() -> void {
  toneA = {};
  toneB = {};
  toneC = {};
  noise = {};
  envelope = {};
  channelA = {};
  channelB = {};
  channelC = {};
  portA = {};
  portB = {};
  io = {};
}

}