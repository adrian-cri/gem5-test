/*
 * Copyright (c) 2008 The Hewlett-Packard Development Company
 * All rights reserved.
 *
 * Redistribution and use of this software in source and binary forms,
 * with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * The software must be used only for Non-Commercial Use which means any
 * use which is NOT directed to receiving any direct monetary
 * compensation for, or commercial advantage from such use.  Illustrative
 * examples of non-commercial use are academic research, personal study,
 * teaching, education and corporate research & development.
 * Illustrative examples of commercial use are distributing products for
 * commercial advantage and providing services using the software for
 * commercial advantage.
 *
 * If you wish to use this software or functionality therein that may be
 * covered by patents for commercial use, please contact:
 *     Director of Intellectual Property Licensing
 *     Office of Strategy and Technology
 *     Hewlett-Packard Company
 *     1501 Page Mill Road
 *     Palo Alto, California  94304
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.  Redistributions
 * in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.  Neither the name of
 * the COPYRIGHT HOLDER(s), HEWLETT-PACKARD COMPANY, nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.  No right of
 * sublicense is granted herewith.  Derivatives of the software and
 * output created using the software may be prepared, but only for
 * Non-Commercial Uses.  Derivatives of the software may be shared with
 * others provided: (i) the others agree to abide by the list of
 * conditions herein which includes the Non-Commercial Use restrictions;
 * and (ii) such Derivatives of the software include the above copyright
 * notice to acknowledge the contribution from this software where
 * applicable, this list of conditions and the disclaimer below.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Gabe Black
 */

#include "arch/x86/apicregs.hh"
#include "arch/x86/interrupts.hh"
#include "arch/x86/intmessage.hh"
#include "cpu/base.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

int
divideFromConf(uint32_t conf)
{
    // This figures out what division we want from the division configuration
    // register in the local APIC. The encoding is a little odd but it can
    // be deciphered fairly easily.
    int shift = ((conf & 0x8) >> 1) | (conf & 0x3);
    shift = (shift + 1) % 8;
    return 1 << shift;
}

namespace X86ISA
{

ApicRegIndex
decodeAddr(Addr paddr)
{
    ApicRegIndex regNum;
    paddr &= ~mask(3);
    switch (paddr)
    {
      case 0x20:
        regNum = APIC_ID;
        break;
      case 0x30:
        regNum = APIC_VERSION;
        break;
      case 0x80:
        regNum = APIC_TASK_PRIORITY;
        break;
      case 0x90:
        regNum = APIC_ARBITRATION_PRIORITY;
        break;
      case 0xA0:
        regNum = APIC_PROCESSOR_PRIORITY;
        break;
      case 0xB0:
        regNum = APIC_EOI;
        break;
      case 0xD0:
        regNum = APIC_LOGICAL_DESTINATION;
        break;
      case 0xE0:
        regNum = APIC_DESTINATION_FORMAT;
        break;
      case 0xF0:
        regNum = APIC_SPURIOUS_INTERRUPT_VECTOR;
        break;
      case 0x100:
      case 0x108:
      case 0x110:
      case 0x118:
      case 0x120:
      case 0x128:
      case 0x130:
      case 0x138:
      case 0x140:
      case 0x148:
      case 0x150:
      case 0x158:
      case 0x160:
      case 0x168:
      case 0x170:
      case 0x178:
        regNum = APIC_IN_SERVICE((paddr - 0x100) / 0x8);
        break;
      case 0x180:
      case 0x188:
      case 0x190:
      case 0x198:
      case 0x1A0:
      case 0x1A8:
      case 0x1B0:
      case 0x1B8:
      case 0x1C0:
      case 0x1C8:
      case 0x1D0:
      case 0x1D8:
      case 0x1E0:
      case 0x1E8:
      case 0x1F0:
      case 0x1F8:
        regNum = APIC_TRIGGER_MODE((paddr - 0x180) / 0x8);
        break;
      case 0x200:
      case 0x208:
      case 0x210:
      case 0x218:
      case 0x220:
      case 0x228:
      case 0x230:
      case 0x238:
      case 0x240:
      case 0x248:
      case 0x250:
      case 0x258:
      case 0x260:
      case 0x268:
      case 0x270:
      case 0x278:
        regNum = APIC_INTERRUPT_REQUEST((paddr - 0x200) / 0x8);
        break;
      case 0x280:
        regNum = APIC_ERROR_STATUS;
        break;
      case 0x300:
        regNum = APIC_INTERRUPT_COMMAND_LOW;
        break;
      case 0x310:
        regNum = APIC_INTERRUPT_COMMAND_HIGH;
        break;
      case 0x320:
        regNum = APIC_LVT_TIMER;
        break;
      case 0x330:
        regNum = APIC_LVT_THERMAL_SENSOR;
        break;
      case 0x340:
        regNum = APIC_LVT_PERFORMANCE_MONITORING_COUNTERS;
        break;
      case 0x350:
        regNum = APIC_LVT_LINT0;
        break;
      case 0x360:
        regNum = APIC_LVT_LINT1;
        break;
      case 0x370:
        regNum = APIC_LVT_ERROR;
        break;
      case 0x380:
        regNum = APIC_INITIAL_COUNT;
        break;
      case 0x390:
        regNum = APIC_CURRENT_COUNT;
        break;
      case 0x3E0:
        regNum = APIC_DIVIDE_CONFIGURATION;
        break;
      default:
        // A reserved register field.
        panic("Accessed reserved register field %#x.\n", paddr);
        break;
    }
    return regNum;
}
}

Tick
X86ISA::Interrupts::read(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;
    //Make sure we're at least only accessing one register.
    if ((offset & ~mask(3)) != ((offset + pkt->getSize()) & ~mask(3)))
        panic("Accessed more than one register at a time in the APIC!\n");
    ApicRegIndex reg = decodeAddr(offset);
    uint32_t val = htog(readReg(reg));
    DPRINTF(LocalApic,
            "Reading Local APIC register %d at offset %#x as %#x.\n",
            reg, offset, val);
    pkt->setData(((uint8_t *)&val) + (offset & mask(3)));
    pkt->makeAtomicResponse();
    return latency;
}

Tick
X86ISA::Interrupts::write(PacketPtr pkt)
{
    Addr offset = pkt->getAddr() - pioAddr;
    //Make sure we're at least only accessing one register.
    if ((offset & ~mask(3)) != ((offset + pkt->getSize()) & ~mask(3)))
        panic("Accessed more than one register at a time in the APIC!\n");
    ApicRegIndex reg = decodeAddr(offset);
    uint32_t val = regs[reg];
    pkt->writeData(((uint8_t *)&val) + (offset & mask(3)));
    DPRINTF(LocalApic,
            "Writing Local APIC register %d at offset %#x as %#x.\n",
            reg, offset, gtoh(val));
    setReg(reg, gtoh(val));
    pkt->makeAtomicResponse();
    return latency;
}
void
X86ISA::Interrupts::requestInterrupt(uint8_t vector,
        uint8_t deliveryMode, bool level)
{
    /*
     * Fixed and lowest-priority delivery mode interrupts are handled
     * using the IRR/ISR registers, checking against the TPR, etc.
     * The SMI, NMI, ExtInt, INIT, etc interrupts go straight through.
     */
    if (deliveryMode == DeliveryMode::Fixed ||
            deliveryMode == DeliveryMode::LowestPriority) {
        DPRINTF(LocalApic, "Interrupt is an %s.\n",
                DeliveryMode::names[deliveryMode]);
        // Queue up the interrupt in the IRR.
        if (vector > IRRV)
            IRRV = vector;
        if (!getRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, vector)) {
            setRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, vector);
            if (level) {
                setRegArrayBit(APIC_TRIGGER_MODE_BASE, vector);
            } else {
                clearRegArrayBit(APIC_TRIGGER_MODE_BASE, vector);
            }
        }
    } else if (!DeliveryMode::isReserved(deliveryMode)) {
        DPRINTF(LocalApic, "Interrupt is an %s.\n",
                DeliveryMode::names[deliveryMode]);
        if (deliveryMode == DeliveryMode::SMI && !pendingSmi) {
            pendingUnmaskableInt = pendingSmi = true;
            smiVector = vector;
        } else if (deliveryMode == DeliveryMode::NMI && !pendingNmi) {
            pendingUnmaskableInt = pendingNmi = true;
            nmiVector = vector;
        } else if (deliveryMode == DeliveryMode::ExtInt && !pendingExtInt) {
            pendingExtInt = true;
            extIntVector = vector;
        } else if (deliveryMode == DeliveryMode::INIT && !pendingInit) {
            pendingUnmaskableInt = pendingInit = true;
            initVector = vector;
        } else if (deliveryMode == DeliveryMode::SIPI && !pendingStartup) {
            pendingUnmaskableInt = pendingStartup = true;
            startupVector = vector;
        }
    } 
    cpu->wakeup();
}


void
X86ISA::Interrupts::setCPU(BaseCPU * newCPU)
{
    cpu = newCPU;
    assert(cpu);
    regs[APIC_ID] = (cpu->cpuId() << 24);
}


Tick
X86ISA::Interrupts::recvMessage(PacketPtr pkt)
{
    uint8_t id = (regs[APIC_ID] >> 24);
    Addr offset = pkt->getAddr() - x86InterruptAddress(id, 0);
    assert(pkt->cmd == MemCmd::MessageReq);
    switch(offset)
    {
      case 0:
        {
            TriggerIntMessage message = pkt->get<TriggerIntMessage>();
            DPRINTF(LocalApic,
                    "Got Trigger Interrupt message with vector %#x.\n",
                    message.vector);
            // Make sure we're really supposed to get this.
            assert((message.destMode == 0 && message.destination == id) ||
                   (bits((int)message.destination, id)));

            requestInterrupt(message.vector,
                    message.deliveryMode, message.trigger);
        }
        break;
      default:
        panic("Local apic got unknown interrupt message at offset %#x.\n",
                offset);
        break;
    }
    delete pkt->req;
    delete pkt;
    return latency;
}


void
X86ISA::Interrupts::addressRanges(AddrRangeList &range_list)
{
    uint8_t id = (regs[APIC_ID] >> 24);
    range_list.clear();
    Range<Addr> range = RangeEx(x86LocalAPICAddress(id, 0),
                                x86LocalAPICAddress(id, 0) + PageBytes);
    range_list.push_back(range);
    pioAddr = range.start;
}


void
X86ISA::Interrupts::getIntAddrRange(AddrRangeList &range_list)
{
    uint8_t id = (regs[APIC_ID] >> 24);
    range_list.clear();
    range_list.push_back(RangeEx(x86InterruptAddress(id, 0),
                x86InterruptAddress(id, 0) + PhysAddrAPICRangeSize));
}


uint32_t
X86ISA::Interrupts::readReg(ApicRegIndex reg)
{
    if (reg >= APIC_TRIGGER_MODE(0) &&
            reg <= APIC_TRIGGER_MODE(15)) {
        panic("Local APIC Trigger Mode registers are unimplemented.\n");
    }
    switch (reg) {
      case APIC_ARBITRATION_PRIORITY:
        panic("Local APIC Arbitration Priority register unimplemented.\n");
        break;
      case APIC_PROCESSOR_PRIORITY:
        panic("Local APIC Processor Priority register unimplemented.\n");
        break;
      case APIC_ERROR_STATUS:
        regs[APIC_INTERNAL_STATE] &= ~ULL(0x1);
        break;
      case APIC_CURRENT_COUNT:
        {
            if (apicTimerEvent.scheduled()) {
                assert(clock);
                // Compute how many m5 ticks happen per count.
                uint64_t ticksPerCount = clock *
                    divideFromConf(regs[APIC_DIVIDE_CONFIGURATION]);
                // Compute how many m5 ticks are left.
                uint64_t val = apicTimerEvent.when() - curTick;
                // Turn that into a count.
                val = (val + ticksPerCount - 1) / ticksPerCount;
                return val;
            } else {
                return 0;
            }
        }
      default:
        break;
    }
    return regs[reg];
}

void
X86ISA::Interrupts::setReg(ApicRegIndex reg, uint32_t val)
{
    uint32_t newVal = val;
    if (reg >= APIC_IN_SERVICE(0) &&
            reg <= APIC_IN_SERVICE(15)) {
        panic("Local APIC In-Service registers are unimplemented.\n");
    }
    if (reg >= APIC_TRIGGER_MODE(0) &&
            reg <= APIC_TRIGGER_MODE(15)) {
        panic("Local APIC Trigger Mode registers are unimplemented.\n");
    }
    if (reg >= APIC_INTERRUPT_REQUEST(0) &&
            reg <= APIC_INTERRUPT_REQUEST(15)) {
        panic("Local APIC Interrupt Request registers "
                "are unimplemented.\n");
    }
    switch (reg) {
      case APIC_ID:
        newVal = val & 0xFF;
        break;
      case APIC_VERSION:
        // The Local APIC Version register is read only.
        return;
      case APIC_TASK_PRIORITY:
        newVal = val & 0xFF;
        break;
      case APIC_ARBITRATION_PRIORITY:
        panic("Local APIC Arbitration Priority register unimplemented.\n");
        break;
      case APIC_PROCESSOR_PRIORITY:
        panic("Local APIC Processor Priority register unimplemented.\n");
        break;
      case APIC_EOI:
        // Remove the interrupt that just completed from the local apic state.
        clearRegArrayBit(APIC_IN_SERVICE_BASE, ISRV);
        updateISRV();
        return;
      case APIC_LOGICAL_DESTINATION:
        newVal = val & 0xFF000000;
        break;
      case APIC_DESTINATION_FORMAT:
        newVal = val | 0x0FFFFFFF;
        break;
      case APIC_SPURIOUS_INTERRUPT_VECTOR:
        regs[APIC_INTERNAL_STATE] &= ~ULL(1 << 1);
        regs[APIC_INTERNAL_STATE] |= val & (1 << 8);
        if (val & (1 << 9))
            warn("Focus processor checking not implemented.\n");
        break;
      case APIC_ERROR_STATUS:
        {
            if (regs[APIC_INTERNAL_STATE] & 0x1) {
                regs[APIC_INTERNAL_STATE] &= ~ULL(0x1);
                newVal = 0;
            } else {
                regs[APIC_INTERNAL_STATE] |= ULL(0x1);
                return;
            }

        }
        break;
      case APIC_INTERRUPT_COMMAND_LOW:
        {
            InterruptCommandRegLow low = regs[APIC_INTERRUPT_COMMAND_LOW];
            // Check if we're already sending an IPI.
            if (low.deliveryStatus) {
                newVal = low;
                break;
            }
            low = val;
            InterruptCommandRegHigh high = regs[APIC_INTERRUPT_COMMAND_HIGH];
            // Record that an IPI is being sent.
            low.deliveryStatus = 1;
            TriggerIntMessage message;
            message.destination = high.destination;
            message.vector = low.vector;
            message.deliveryMode = low.deliveryMode;
            message.destMode = low.destMode;
            message.level = low.level;
            message.trigger = low.trigger;
            bool timing = sys->getMemoryMode() == Enums::timing;
            switch (low.destShorthand) {
              case 0:
                intPort->sendMessage(message, timing);
                break;
              case 1:
                panic("Self IPIs aren't implemented.\n");
                break;
              case 2:
                panic("Broadcast including self IPIs aren't implemented.\n");
                break;
              case 3:
                panic("Broadcast excluding self IPIs aren't implemented.\n");
                break;
            }
        }
        break;
      case APIC_LVT_TIMER:
      case APIC_LVT_THERMAL_SENSOR:
      case APIC_LVT_PERFORMANCE_MONITORING_COUNTERS:
      case APIC_LVT_LINT0:
      case APIC_LVT_LINT1:
      case APIC_LVT_ERROR:
        {
            uint64_t readOnlyMask = (1 << 12) | (1 << 14);
            newVal = (val & ~readOnlyMask) |
                     (regs[reg] & readOnlyMask);
        }
        break;
      case APIC_INITIAL_COUNT:
        {
            assert(clock);
            newVal = bits(val, 31, 0);
            // Compute how many timer ticks we're being programmed for.
            uint64_t newCount = newVal *
                (divideFromConf(regs[APIC_DIVIDE_CONFIGURATION]));
            // Schedule on the edge of the next tick plus the new count.
            Tick offset = curTick % clock;
            if (offset) {
                reschedule(apicTimerEvent,
                        curTick + (newCount + 1) * clock - offset, true);
            } else {
                reschedule(apicTimerEvent,
                        curTick + newCount * clock, true);
            }
        }
        break;
      case APIC_CURRENT_COUNT:
        //Local APIC Current Count register is read only.
        return;
      case APIC_DIVIDE_CONFIGURATION:
        newVal = val & 0xB;
        break;
      default:
        break;
    }
    regs[reg] = newVal;
    return;
}


X86ISA::Interrupts::Interrupts(Params * p) :
    BasicPioDevice(p), IntDev(this), latency(p->pio_latency), clock(0),
    apicTimerEvent(this),
    pendingSmi(false), smiVector(0),
    pendingNmi(false), nmiVector(0),
    pendingExtInt(false), extIntVector(0),
    pendingInit(false), initVector(0),
    pendingStartup(false), startupVector(0),
    pendingUnmaskableInt(false)
{
    pioSize = PageBytes;
    memset(regs, 0, sizeof(regs));
    //Set the local apic DFR to the flat model.
    regs[APIC_DESTINATION_FORMAT] = (uint32_t)(-1);
    ISRV = 0;
    IRRV = 0;
}


bool
X86ISA::Interrupts::checkInterrupts(ThreadContext *tc) const
{
    RFLAGS rflags = tc->readMiscRegNoEffect(MISCREG_RFLAGS);
    if (pendingUnmaskableInt) {
        DPRINTF(LocalApic, "Reported pending unmaskable interrupt.\n");
        return true;
    }
    if (rflags.intf) {
        if (pendingExtInt) {
            DPRINTF(LocalApic, "Reported pending external interrupt.\n");
            return true;
        }
        if (IRRV > ISRV && bits(IRRV, 7, 4) >
               bits(regs[APIC_TASK_PRIORITY], 7, 4)) {
            DPRINTF(LocalApic, "Reported pending regular interrupt.\n");
            return true;
        }
    }
    return false;
}

Fault
X86ISA::Interrupts::getInterrupt(ThreadContext *tc)
{
    assert(checkInterrupts(tc));
    // These are all probably fairly uncommon, so we'll make them easier to
    // check for.
    if (pendingUnmaskableInt) {
        if (pendingSmi) {
            DPRINTF(LocalApic, "Generated SMI fault object.\n");
            return new SystemManagementInterrupt();
        } else if (pendingNmi) {
            DPRINTF(LocalApic, "Generated NMI fault object.\n");
            return new NonMaskableInterrupt(nmiVector);
        } else if (pendingInit) {
            DPRINTF(LocalApic, "Generated INIT fault object.\n");
            return new InitInterrupt(initVector);
        } else if (pendingStartup) {
            DPRINTF(LocalApic, "Generating SIPI fault object.\n");
            return new StartupInterrupt(startupVector);
        } else {
            panic("pendingUnmaskableInt set, but no unmaskable "
                    "ints were pending.\n");
            return NoFault;
        }
    } else if (pendingExtInt) {
        DPRINTF(LocalApic, "Generated external interrupt fault object.\n");
        return new ExternalInterrupt(extIntVector);
    } else {
        DPRINTF(LocalApic, "Generated regular interrupt fault object.\n");
        // The only thing left are fixed and lowest priority interrupts.
        return new ExternalInterrupt(IRRV);
    }
}

void
X86ISA::Interrupts::updateIntrInfo(ThreadContext *tc)
{
    assert(checkInterrupts(tc));
    if (pendingUnmaskableInt) {
        if (pendingSmi) {
            DPRINTF(LocalApic, "SMI sent to core.\n");
            pendingSmi = false;
        } else if (pendingNmi) {
            DPRINTF(LocalApic, "NMI sent to core.\n");
            pendingNmi = false;
        } else if (pendingInit) {
            DPRINTF(LocalApic, "Init sent to core.\n");
            pendingInit = false;
        } else if (pendingStartup) {
            DPRINTF(LocalApic, "SIPI sent to core.\n");
            pendingStartup = false;
        }
        if (!(pendingSmi || pendingNmi || pendingInit || pendingStartup))
            pendingUnmaskableInt = false;
    } else if (pendingExtInt) {
        pendingExtInt = false;
    } else {
        DPRINTF(LocalApic, "Interrupt %d sent to core.\n", IRRV);
        // Mark the interrupt as "in service".
        ISRV = IRRV;
        setRegArrayBit(APIC_IN_SERVICE_BASE, ISRV);
        // Clear it out of the IRR.
        clearRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, IRRV);
        updateIRRV();
    }
}

X86ISA::Interrupts *
X86LocalApicParams::create()
{
    return new X86ISA::Interrupts(this);
}
