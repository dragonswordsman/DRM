# Copyright (c) 2012-2013 ARM Limited
# All rights reserved
# 
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2006-2008 The Regents of The University of Michigan
# Copyright (c) 2010 Advanced Micro Devices, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Lisa Hsu

import sys
from os import getcwd
from os.path import join as joinpath

import CpuConfig
import MemConfig

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import *

import subprocess

addToPath('../common')

#Milad: changes frequency of an specified core in core_freq_list (core number)
def set_cpu (core_number,core_freq_list_name,np):
    
    if np>= 10:
        if core_number == 0 :
            corrected_core_number = '00'
        elif core_number == 1 :
            corrected_core_number = '01'
        elif core_number == 2 :
            corrected_core_number = '02'
        elif core_number == 3 :
            corrected_core_number = '03'
        elif core_number == 4 :
            corrected_core_number = '04'
        elif core_number == 5 :
            corrected_core_number = '05'
        elif core_number == 6 :
            corrected_core_number = '06'
        elif core_number == 7 :
            corrected_core_number = '07'
        elif core_number == 8 :
            corrected_core_number = '08' 
        elif core_number == 9 :
            corrected_core_number = '09'
        else:
           corrected_core_number=core_number 
    else:
        corrected_core_number=core_number      

    core_freq_list_name[core_number]= 'cpu%s' %corrected_core_number
    return core_freq_list_name


def set_FREQ (freq,core_number,freq_domain,core_freq_list,core_freq_list_name,np):
    core_freq_list[core_number]= freq_domain[freq][core_number]
    if np>= 10:
        if core_number == 0 :
            corrected_core_number = '00'
        elif core_number == 1 :
            corrected_core_number = '01'
        elif core_number == 2 :
            corrected_core_number = '02'
        elif core_number == 3 :
            corrected_core_number = '03'
        elif core_number == 4 :
            corrected_core_number = '04'
        elif core_number == 5 :
            corrected_core_number = '05'
        elif core_number == 6 :
            corrected_core_number = '06'
        elif core_number == 7 :
            corrected_core_number = '07'
        elif core_number == 8 :
            corrected_core_number = '08' 
        elif core_number == 9 :
            corrected_core_number = '09'
        else:
           corrected_core_number=core_number 
    else:
        corrected_core_number=core_number      

    core_freq_list_name[core_number]= '%s%s' %(freq,corrected_core_number)
    
    #calculating the smallest frequency in list to evaluate the simulation time coefficient
    #These values are based on experience 
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_2GHz' in core_freq_list_name [i]:
            coefficient = 1
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1900MHz' in core_freq_list_name [i]:
            coefficient = 1.06
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1800MHz' in core_freq_list_name [i]:
            coefficient = 1.12
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1700MHz' in core_freq_list_name [i]:
            coefficient = 1.18
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1600MHz' in core_freq_list_name [i]:
            coefficient = 1.24
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1500MHz' in core_freq_list_name [i]:
            coefficient = 1.3
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1400MHz' in core_freq_list_name [i]:
            coefficient = 1.36
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1300MHz' in core_freq_list_name [i]:
            coefficient = 1.42
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1200MHz' in core_freq_list_name [i]:
            coefficient = 1.48
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1100MHz' in core_freq_list_name [i]:
            coefficient = 1.54
    for i in range (0 , len(core_freq_list_name) ):
        if 'FREQ_1GHz' in core_freq_list_name [i]:
            coefficient = 1.6
####    
#    coefficient = 1

    return coefficient, core_freq_list, core_freq_list_name 

#Milad
def decrease_frequency(core_number,level,freq_domain,core_freq_list,core_freq_list_name,np): 
    if level == 1 :
        if 'FREQ_2GHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1900MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1900MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1800MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1800MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1700MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1700MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1600MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1600MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1500MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1500MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1400MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1400MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1300MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1300MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1200MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1200MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1100MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1100MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1GHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1GHz' in core_freq_list_name [core_number]:  # return it again
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1GHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)                    
            isFreqChanged = 0

    return isFreqChanged, coefficient, core_freq_list, core_freq_list_name  


#Milad
def increase_frequency(core_number,level,freq_domain,core_freq_list,core_freq_list_name,np): 
    if level == 1 :
        if 'FREQ_2GHz' in core_freq_list_name [core_number]: #return it again 
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_2GHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 0

        elif 'FREQ_1900MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_2GHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1800MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1900MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1700MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1800MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1600MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1700MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1500MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1600MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1400MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1500MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1300MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1400MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1200MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1300MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1100MHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1200MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1

        elif 'FREQ_1GHz' in core_freq_list_name [core_number]:  
            coefficient, core_freq_list, core_freq_list_name = set_FREQ('FREQ_1100MHz',core_number,freq_domain,core_freq_list,core_freq_list_name,np)
            isFreqChanged = 1


    return isFreqChanged, coefficient, core_freq_list, core_freq_list_name  


#Milad    
def CoreMTTFsortLowToHigh(MTTF_list,interval):
    MTTF=[]
    MTTF = MTTF_list[interval]
    listlen=len(MTTF)
    sort=[x for x in range(0,listlen)] 
    for i in range (0,listlen-1):
        for j in range (i+1 , listlen):
           if float(MTTF[i])>float(MTTF[j]):
                temp = MTTF[i]
                MTTF[i] = MTTF[j]
                MTTF[j] = temp
            
                temp = sort[i]
                sort[i] = sort[j]
                sort[j] = temp            
    return sort

#Milad
def CoreMTTFsortHighToLow(MTTF_list,interval):
    MTTF=[]
    MTTF = MTTF_list[interval]
    listlen=len(MTTF)
    sort=[x for x in range(0,listlen)] 
    for i in range (0,listlen-1):
        for j in range (i+1 , listlen):
           if float(MTTF[i])<float(MTTF[j]):
                temp = MTTF[i]
                MTTF[i] = MTTF[j]
                MTTF[j] = temp
                temp = sort[i]
                sort[i] = sort[j]
                sort[j] = temp
                
    return sort





def getCPUClass(cpu_type):
    """Returns the required cpu class and the mode of operation."""
    cls = CpuConfig.get(cpu_type)
    return cls, cls.memory_mode()

def setCPUClass(options):
    """Returns two cpu classes and the initial mode of operation.

       Restoring from a checkpoint or fast forwarding through a benchmark
       can be done using one type of cpu, and then the actual
       simulation can be carried out using another type. This function
       returns these two types of cpus and the initial mode of operation
       depending on the options provided.
    """

    TmpClass, test_mem_mode = getCPUClass(options.cpu_type)
    CPUClass = None
    if TmpClass.require_caches() and \
            not options.caches and not options.ruby:
        fatal("%s must be used with caches" % options.cpu_type)

    if options.checkpoint_restore != None:
        if options.restore_with_cpu != options.cpu_type:
            CPUClass = TmpClass
            TmpClass, test_mem_mode = getCPUClass(options.restore_with_cpu)
    elif options.fast_forward:
        CPUClass = TmpClass
        TmpClass = AtomicSimpleCPU
        test_mem_mode = 'atomic'

    return (TmpClass, test_mem_mode, CPUClass)

def setMemClass(options):
    """Returns a memory controller class."""

    return MemConfig.get(options.mem_type)

def setWorkCountOptions(system, options):
    if options.work_item_id != None:
        system.work_item_id = options.work_item_id
    if options.num_work_ids != None:
        system.num_work_ids = options.num_work_ids
    if options.work_begin_cpu_id_exit != None:
        system.work_begin_cpu_id_exit = options.work_begin_cpu_id_exit
    if options.work_end_exit_count != None:
        system.work_end_exit_count = options.work_end_exit_count
    if options.work_end_checkpoint_count != None:
        system.work_end_ckpt_count = options.work_end_checkpoint_count
    if options.work_begin_exit_count != None:
        system.work_begin_exit_count = options.work_begin_exit_count
    if options.work_begin_checkpoint_count != None:
        system.work_begin_ckpt_count = options.work_begin_checkpoint_count
    if options.work_cpus_checkpoint_count != None:
        system.work_cpus_ckpt_count = options.work_cpus_checkpoint_count

def findCptDir(options, cptdir, testsys):
    """Figures out the directory from which the checkpointed state is read.

    There are two different ways in which the directories holding checkpoints
    can be named --
    1. cpt.<benchmark name>.<instruction count when the checkpoint was taken>
    2. cpt.<some number, usually the tick value when the checkpoint was taken>

    This function parses through the options to figure out which one of the
    above should be used for selecting the checkpoint, and then figures out
    the appropriate directory.
    """

    from os.path import isdir, exists
    from os import listdir
    import re

    if not isdir(cptdir):
        fatal("checkpoint dir %s does not exist!", cptdir)

    cpt_starttick = 0
    if options.at_instruction or options.simpoint:
        inst = options.checkpoint_restore
        if options.simpoint:
            # assume workload 0 has the simpoint
            if testsys.cpu[0].workload[0].simpoint == 0:
                fatal('Unable to find simpoint')
            inst += int(testsys.cpu[0].workload[0].simpoint)

        checkpoint_dir = joinpath(cptdir, "cpt.%s.%s" % (options.bench, inst))
        if not exists(checkpoint_dir):
            fatal("Unable to find checkpoint directory %s", checkpoint_dir)
    else:
        dirs = listdir(cptdir)
        expr = re.compile('cpt\.([0-9]*)')
        cpts = []
        for dir in dirs:
            match = expr.match(dir)
            if match:
                cpts.append(match.group(1))

        cpts.sort(lambda a,b: cmp(long(a), long(b)))

        cpt_num = options.checkpoint_restore
        if cpt_num > len(cpts):
            fatal('Checkpoint %d not found', cpt_num)

        cpt_starttick = int(cpts[cpt_num - 1])
        checkpoint_dir = joinpath(cptdir, "cpt.%s" % cpts[cpt_num - 1])

    return cpt_starttick, checkpoint_dir

def scriptCheckpoints(options, maxtick, cptdir):
    if options.at_instruction or options.simpoint:
        checkpoint_inst = int(options.take_checkpoints)

        # maintain correct offset if we restored from some instruction
        if options.checkpoint_restore != None:
            checkpoint_inst += options.checkpoint_restore

        print "Creating checkpoint at inst:%d" % (checkpoint_inst)
        exit_event = m5.simulate()
        exit_cause = exit_event.getCause()
        print "exit cause = %s" % exit_cause

        # skip checkpoint instructions should they exist
        while exit_cause == "checkpoint":
            exit_event = m5.simulate()
            exit_cause = exit_event.getCause()

        if exit_cause == "a thread reached the max instruction count":
            m5.checkpoint(joinpath(cptdir, "cpt.%s.%d" % \
                    (options.bench, checkpoint_inst)))
            print "Checkpoint written."

    else:
        when, period = options.take_checkpoints.split(",", 1)
        when = int(when)
        period = int(period)
        num_checkpoints = 0

        exit_event = m5.simulate(when - m5.curTick())
        exit_cause = exit_event.getCause()
        while exit_cause == "checkpoint":
            exit_event = m5.simulate(when - m5.curTick())
            exit_cause = exit_event.getCause()

        if exit_cause == "simulate() limit reached":
            m5.checkpoint(joinpath(cptdir, "cpt.%d"))
            num_checkpoints += 1

        sim_ticks = when
        max_checkpoints = options.max_checkpoints

        while num_checkpoints < max_checkpoints and \
                exit_cause == "simulate() limit reached":
            if (sim_ticks + period) > maxtick:
                exit_event = m5.simulate(maxtick - sim_ticks)
                exit_cause = exit_event.getCause()
                break
            else:
                exit_event = m5.simulate(period)
                exit_cause = exit_event.getCause()
                sim_ticks += period
                while exit_event.getCause() == "checkpoint":
                    exit_event = m5.simulate(sim_ticks - m5.curTick())
                if exit_event.getCause() == "simulate() limit reached":
                    m5.checkpoint(joinpath(cptdir, "cpt.%d"))
                    num_checkpoints += 1

    return exit_event

def benchCheckpoints(options, maxtick, cptdir):
    print "\n\n Entering benchCheckpoints"
    print "\n my m5.simulate(500) ended"
    print"\n starting exit_event = m5.simulate(maxtick - m5.curTick())"
    exit_event = m5.simulate(maxtick - m5.curTick())
    print "\n exit_event m5.simulate finished maxtick=%d cur_tick=%d"%(maxtick,m5.curTick())
    exit_cause = exit_event.getCause()

    num_checkpoints = 0
    max_checkpoints = options.max_checkpoints

    while exit_cause == "checkpoint":
        m5.checkpoint(joinpath(cptdir, "cpt.%d"))
        num_checkpoints += 1
        if num_checkpoints == max_checkpoints:
            exit_cause = "maximum %d checkpoints dropped" % max_checkpoints
            break

        exit_event = m5.simulate(maxtick - m5.curTick())
        exit_cause = exit_event.getCause()

    return exit_event

def repeatSwitch(testsys, repeat_switch_cpu_list, maxtick, switch_freq):
    print "starting switch loop"
    while True:
        exit_event = m5.simulate(switch_freq)
        exit_cause = exit_event.getCause()

        if exit_cause != "simulate() limit reached":
            return exit_event

        m5.switchCpus(testsys, repeat_switch_cpu_list)

        tmp_cpu_list = []
        for old_cpu, new_cpu in repeat_switch_cpu_list:
            tmp_cpu_list.append((new_cpu, old_cpu))
        repeat_switch_cpu_list = tmp_cpu_list

        if (maxtick - m5.curTick()) <= switch_freq:
            exit_event = m5.simulate(maxtick - m5.curTick())
            return exit_event

def run(options, root, testsys, cpu_class):

    print "\n\n\nRUN\n\n\n"
    if options.checkpoint_dir:
        cptdir = options.checkpoint_dir
    elif m5.options.outdir:
        cptdir = m5.options.outdir
    else:
        cptdir = getcwd()
    
    if options.fast_forward and options.checkpoint_restore != None:
        fatal("Can't specify both --fast-forward and --checkpoint-restore")

    if options.standard_switch and not options.caches:
        fatal("Must specify --caches when using --standard-switch")

    if options.standard_switch and options.repeat_switch:
        fatal("Can't specify both --standard-switch and --repeat-switch")

    if options.repeat_switch and options.take_checkpoints:
        fatal("Can't specify both --repeat-switch and --take-checkpoints")

    np = options.num_cpus
    switch_cpus = None

    if options.prog_interval:
        print"\noptions.prog_interval"
        for i in xrange(np):
            testsys.cpu[i].progress_interval = options.prog_interval

    if options.maxinsts:
        print"\noptions.maxinsts"
        for i in xrange(np):
            testsys.cpu[i].max_insts_any_thread = options.maxinsts

    if cpu_class:
        print"\ncpu class"
        switch_cpus = [cpu_class(switched_out=True, cpu_id=(i))
                       for i in xrange(np)]

        for i in xrange(np):
            if options.fast_forward:
                testsys.cpu[i].max_insts_any_thread = int(options.fast_forward)
            switch_cpus[i].system =  testsys
            switch_cpus[i].workload = testsys.cpu[i].workload
            switch_cpus[i].clk_domain = testsys.cpu[i].clk_domain
            # simulation period
            if options.maxinsts:
                switch_cpus[i].max_insts_any_thread = options.maxinsts
            # Add checker cpu if selected
            if options.checker:
                switch_cpus[i].addCheckerCpu()

        testsys.switch_cpus = switch_cpus
        switch_cpu_list = [(testsys.cpu[i], switch_cpus[i]) for i in xrange(np)]

    if options.repeat_switch:
        print"\n options.repeat_switch"
        switch_class = getCPUClass(options.cpu_type)[0]
        if switch_class.require_caches() and \
                not options.caches:
            print "%s: Must be used with caches" % str(switch_class)
            sys.exit(1)
        if not switch_class.support_take_over():
            print "%s: CPU switching not supported" % str(switch_class)
            sys.exit(1)

        repeat_switch_cpus = [switch_class(switched_out=True, \
                                               cpu_id=(i)) for i in xrange(np)]

        for i in xrange(np):
            repeat_switch_cpus[i].system = testsys
            repeat_switch_cpus[i].workload = testsys.cpu[i].workload
            repeat_switch_cpus[i].clk_domain = testsys.cpu[i].clk_domain

            if options.maxinsts:
                repeat_switch_cpus[i].max_insts_any_thread = options.maxinsts

            if options.checker:
                repeat_switch_cpus[i].addCheckerCpu()

        testsys.repeat_switch_cpus = repeat_switch_cpus

        if cpu_class:
            repeat_switch_cpu_list = [(switch_cpus[i], repeat_switch_cpus[i])
                                      for i in xrange(np)]
        else:
            repeat_switch_cpu_list = [(testsys.cpu[i], repeat_switch_cpus[i])
                                      for i in xrange(np)]

    if options.standard_switch:
        print"\noptions.standard_switch"
        switch_cpus = [TimingSimpleCPU(switched_out=True, cpu_id=(i))
                       for i in xrange(np)]
        switch_cpus_1 = [DerivO3CPU(switched_out=True, cpu_id=(i))
                        for i in xrange(np)]

        for i in xrange(np):
            switch_cpus[i].system =  testsys
            switch_cpus_1[i].system =  testsys
            switch_cpus[i].workload = testsys.cpu[i].workload
            switch_cpus_1[i].workload = testsys.cpu[i].workload
            switch_cpus[i].clk_domain = testsys.cpu[i].clk_domain
            switch_cpus_1[i].clk_domain = testsys.cpu[i].clk_domain

            # if restoring, make atomic cpu simulate only a few instructions
            if options.checkpoint_restore != None:
                testsys.cpu[i].max_insts_any_thread = 1
            # Fast forward to specified location if we are not restoring
            elif options.fast_forward:
                testsys.cpu[i].max_insts_any_thread = int(options.fast_forward)
            # Fast forward to a simpoint (warning: time consuming)
            elif options.simpoint:
                if testsys.cpu[i].workload[0].simpoint == 0:
                    fatal('simpoint not found')
                testsys.cpu[i].max_insts_any_thread = \
                    testsys.cpu[i].workload[0].simpoint
            # No distance specified, just switch
            else:
                testsys.cpu[i].max_insts_any_thread = 1

            # warmup period
            if options.warmup_insts:
                switch_cpus[i].max_insts_any_thread =  options.warmup_insts

            # simulation period
            if options.maxinsts:
                switch_cpus_1[i].max_insts_any_thread = options.maxinsts

            # attach the checker cpu if selected
            if options.checker:
                switch_cpus[i].addCheckerCpu()
                switch_cpus_1[i].addCheckerCpu()

        testsys.switch_cpus = switch_cpus
        testsys.switch_cpus_1 = switch_cpus_1
        switch_cpu_list = [(testsys.cpu[i], switch_cpus[i]) for i in xrange(np)]
        switch_cpu_list1 = [(switch_cpus[i], switch_cpus_1[i]) for i in xrange(np)]

    # set the checkpoint in the cpu before m5.instantiate is called
    if options.take_checkpoints != None and \
           (options.simpoint or options.at_instruction):

        print"\noptions.take_checkpoints != None and \
           (options.simpoint or options.at_instruction)"
        offset = int(options.take_checkpoints)
        # Set an instruction break point
        if options.simpoint:
            for i in xrange(np):
                if testsys.cpu[i].workload[0].simpoint == 0:
                    fatal('no simpoint for testsys.cpu[%d].workload[0]', i)
                checkpoint_inst = int(testsys.cpu[i].workload[0].simpoint) + offset
                testsys.cpu[i].max_insts_any_thread = checkpoint_inst
                # used for output below
                options.take_checkpoints = checkpoint_inst
        else:
            print"\nelse :options.take_checkpoints != None and \
              (options.simpoint or options.at_instruction)"

            options.take_checkpoints = offset
            # Set all test cpus with the right number of instructions
            # for the upcoming simulation
            for i in xrange(np):
                testsys.cpu[i].max_insts_any_thread = offset

    checkpoint_dir = None
    if options.checkpoint_restore:
        print"\noptions.checkpoint_restore"
        cpt_starttick, checkpoint_dir = findCptDir(options, cptdir, testsys)
    print"\nm5.instantiate started"

    m5.instantiate(checkpoint_dir)

    print"\nm5.instantiate finished"
    # Handle the max tick settings now that tick frequency was resolved
    # during system instantiation
    # NOTE: the maxtick variable here is in absolute ticks, so it must
    # include any simulated ticks before a checkpoint
    explicit_maxticks = 0
    maxtick_from_abs = m5.MaxTick
    maxtick_from_rel = m5.MaxTick
    maxtick_from_maxtime = m5.MaxTick
    if options.abs_max_tick:
        print"\noptions.abs_max_tick"
        maxtick_from_abs = options.abs_max_tick
        explicit_maxticks += 1
    if options.rel_max_tick:
        print"\noptions.rel_max_tick"
        maxtick_from_rel = options.rel_max_tick
        if options.checkpoint_restore:
            # NOTE: this may need to be updated if checkpoints ever store
            # the ticks per simulated second
            maxtick_from_rel += cpt_starttick
            if options.at_instruction or options.simpoint:
                warn("Relative max tick specified with --at-instruction or" \
                     " --simpoint\n      These options don't specify the " \
                     "checkpoint start tick, so assuming\n      you mean " \
                     "absolute max tick")
        explicit_maxticks += 1
    if options.maxtime:
        print"\noptions.maxtime"
        maxtick_from_maxtime = m5.ticks.fromSeconds(options.maxtime)
        explicit_maxticks += 1
    if explicit_maxticks > 1:
        warn("Specified multiple of --abs-max-tick, --rel-max-tick, --maxtime."\
             " Using least")
    maxtick = min([maxtick_from_abs, maxtick_from_rel, maxtick_from_maxtime])
    print "\n maxtick"
    if options.checkpoint_restore != None and maxtick < cpt_starttick:
        fatal("Bad maxtick (%d) specified: " \
              "Checkpoint starts starts from tick: %d", maxtick, cpt_starttick)

    if options.standard_switch or cpu_class:
        print "\n\noptions.standard_switch or cpu_class\n\n"
        if options.standard_switch:
            print "Switch at instruction count:%s" % \
                    str(testsys.cpu[0].max_insts_any_thread)
            exit_event = m5.simulate()
        elif cpu_class and options.fast_forward:
            print "Switch at instruction count:%s" % \
                    str(testsys.cpu[0].max_insts_any_thread)
            exit_event = m5.simulate()
        else:
            print "Switch at curTick count:%s" % str(10000)
            exit_event = m5.simulate(10000)
        print "Switched CPUS @ tick %s" % (m5.curTick())

        m5.switchCpus(testsys, switch_cpu_list)

        if options.standard_switch:
            print "Switch at instruction count:%d" % \
                    (testsys.switch_cpus[0].max_insts_any_thread)

            #warmup instruction count may have already been set
            if options.warmup_insts:
                exit_event = m5.simulate()
            else:
                exit_event = m5.simulate(options.standard_switch)
            print "Switching CPUS @ tick %s" % (m5.curTick())
            print "Simulation ends instruction count:%d" % \
                    (testsys.switch_cpus_1[0].max_insts_any_thread)
            m5.switchCpus(testsys, switch_cpu_list1)

    # If we're taking and restoring checkpoints, use checkpoint_dir
    # option only for finding the checkpoints to restore from.  This
    # lets us test checkpointing by restoring from one set of
    # checkpoints, generating a second set, and then comparing them.
    if options.take_checkpoints and options.checkpoint_restore:
        print"\noptions.take_checkpoints and options.checkpoint_restore"
        if m5.options.outdir:
            cptdir = m5.options.outdir
        else:
            cptdir = getcwd()

    if options.take_checkpoints != None :
        print"\noptions.take_checkpoints != None"
        # Checkpoints being taken via the command line at <when> and at
        # subsequent periods of <period>.  Checkpoint instructions
        # received from the benchmark running are ignored and skipped in
        # favor of command line checkpoint instructions.
        exit_event = scriptCheckpoints(options, maxtick, cptdir) 
    else:
        print"\nelse: options.take_checkpoints != None"
        if options.fast_forward:
            m5.stats.reset()
###############################################################################################################################
#################### **** REAL SIMULATION **** ################################################################################
###############################################################################################################################

        print "**** REAL SIMULATION ****"

        ############ Control Technique ###########
        Timing_Cpu = 0  #ROI_Start_Time_Calc
        ROI_Time_Calc = 0
        Ref_Interval_Calc = 0
        Ref_Interval_Calc_with_switchCpu = 0  #Ref_Interval_Calc #default 0 : it wont reflect the operation done by preprocessing and would be more realistic
        DVFS_Technique = 1
        Thread_Migration_Technique = 0
        accumulative_stats_calculation = DVFS_Technique #it can be 0 for Tread Migration and Ref_Interval_Calc. but should be 1 for DVFS : stats would be more realistic
        GEM5_DIR = "/home/milad/gem5"        
        NUM_INTERVALS = 10

        MTTF_refrence= 0.0008 
        ROI_start_tick_estimation= 2650373437000 
        ROI_tick_duration_estimation=680000000000
        OFFSET=0 
 
       ##########################################

        if Timing_Cpu:
            m5.simulate()

                    
        ##########################################
        subprocess.call(["mkdir", "%s/DRM"%GEM5_DIR])
        subprocess.call(["mkdir", "%s/DRM/run_info"%GEM5_DIR])
        INFO_FILE=open("%s/DRM/run_info/info"%GEM5_DIR , "w")
        INFO_FILE.write("Interval CpuList")
        INFO_FILE.write('\n')
        

		##########################################
        interval_tick_duration = (ROI_tick_duration_estimation/NUM_INTERVALS)
        print "interval_tick_duration= %d" %interval_tick_duration
        ##########################################


        m5.simulate(ROI_start_tick_estimation)
        
        
        ##########################################
        print "\nstart of ROI"
        m5.stats.dump()
        m5.stats.reset() 
        MAIN_CPU = [root.system.cpu[i] for i in xrange(np)]
        #atomic
        #Atomic_CPU = [root.system.AtomicCPU[i] for i in xrange(np)]

        freq_domain={} 
        freq_domain['FREQ_2GHz'] = root.system.FREQ_2GHz
        freq_domain['FREQ_1900MHz'] = root.system.FREQ_1900MHz
        freq_domain['FREQ_1800MHz'] = root.system.FREQ_1800MHz
        freq_domain['FREQ_1700MHz'] = root.system.FREQ_1700MHz
        freq_domain['FREQ_1600MHz'] = root.system.FREQ_1600MHz
        freq_domain['FREQ_1500MHz'] = root.system.FREQ_1500MHz
        freq_domain['FREQ_1400MHz'] = root.system.FREQ_1400MHz
        freq_domain['FREQ_1300MHz'] = root.system.FREQ_1300MHz
        freq_domain['FREQ_1200MHz'] = root.system.FREQ_1200MHz
        freq_domain['FREQ_1100MHz'] = root.system.FREQ_1100MHz
        freq_domain['FREQ_1GHz'] = root.system.FREQ_1GHz
        freq_domain['FREQ_800MHz'] = root.system.FREQ_800MHz #not active
        freq_domain['FREQ_500MHz'] = root.system.FREQ_500MHz #not active
        # to activate add them to set_freq,increase and decrease frequency, and eraze # from corresponding part in fs.py 

        DVFS_list = []
        DVFS_list_names = []
        print 'len(freq_domain)=%s' %len(freq_domain)

        #initializing         
        for i in xrange(0,np):            
            DVFS_list.append(None)#just to build the array
            DVFS_list_names.append(None)
            sim_time_coefficient, DVFS_list, DVFS_list_names = set_FREQ('FREQ_2GHz',i,freq_domain,DVFS_list,DVFS_list_names,np)
        print 'DVFS_list = %s'%DVFS_list_names


        MTTF_per_core_per_interval = [[0.0 for x1 in range(np)] for x2 in range(NUM_INTERVALS)] 
        MTTF_change_per_core_per_interval = [[0.0 for x1 in range(np)] for x2 in range(NUM_INTERVALS)]  

        accumulative_MTTF_per_core = [[0.0 for x1 in range(np)] for x2 in range(NUM_INTERVALS)] 
        accumulative_MTTF_change_per_core = [[0.0 for x1 in range(np)] for x2 in range(NUM_INTERVALS)]  

        MTTF_per_interval = [0.0 for x in range(NUM_INTERVALS)] 
        MTTF_change_per_interval = [0.0 for x in range(NUM_INTERVALS)]  

        accumulative_MTTF_per_interval = [0.0 for x in range(NUM_INTERVALS)] 
        accumulative_MTTF_change_per_interval = [0.0 for x in range(NUM_INTERVALS)]  

        Sorted_CoreMTTF = [0.0 for x in range(np)]
        MigratedThreads = [x for x in range(np)]
        SwappedThreads = [0 for x in range(np)]

        if (OFFSET!=0):
            print "Running OFFSET\n"
            m5.stats.reset()
            m5.simulate(OFFSET)#just for some benchmarks such as dedup16
            m5.stats.dump()
            print "OFFSET finished\n"

        if DVFS_Technique or Thread_Migration_Technique or ROI_Time_Calc or Ref_Interval_Calc:  #optional-if it is 0, it would calculate ref-intervals without SwitchCpus         
            m5.drain(root.system)
            m5.switchCpus(root.system,[(MAIN_CPU[i],DVFS_list[i]) for i in xrange(np)],do_drain=False)
            m5.resume(root.system)
        print"1: Switch OK\n"
        simulation_time=interval_tick_duration # refrence time for 2G

        ############################################################
        if ROI_Time_Calc:
            m5.stats.reset()
            m5.simulate()

        ############################################################################################
        ######################################## Main Loop #########################################
        ############################################################################################
        m5.stats.reset()
        for num_interval in xrange(0,NUM_INTERVALS): 
            #m5.stats.reset()          
            print "\n interval_%s starts ..."%num_interval
            #################################### Refrence Intervals ########################################
            ############################################################################################

            if Ref_Interval_Calc_with_switchCpu:
                m5.drain(root.system)
                m5.switchCpus(root.system,[(DVFS_list[i],MAIN_CPU[i]) for i in xrange(np)],do_drain=False)
                m5.resume(root.system)
                 

            #################################### DVFS Based DRM ########################################
            ############################################################################################
            if DVFS_Technique:
                m5.drain(root.system)
                m5.switchCpus(root.system,[(DVFS_list[i],MAIN_CPU[i]) for i in xrange(np)],do_drain=False)
                #atomic
#                m5.switchCpus(root.system,[(DVFS_list[i],Atomic_CPU[i]) for i in xrange(np)],do_drain=False)
                m5.resume(root.system)
                print"2: Switch OK\n"
                m5.simulate(1000)
                print" m5.simulate OK\n"
                
                if (num_interval ==0 ):
                    for i in xrange(0,np):            
                        sim_time_coefficient, DVFS_list, DVFS_list_names = set_FREQ('FREQ_2GHz',i,freq_domain,DVFS_list,DVFS_list_names,np)
                    print 'DVFS_list = %s'%DVFS_list_names

                if (num_interval >=1 ):
                    MTTF_improvement =  float(accumulative_MTTF_per_interval[num_interval-1])/float(MTTF_refrence) 
                    print 'MTTF_improvement = %s\n'%MTTF_improvement

                    ########################################## Increase Frequency ##########################################
                    if MTTF_improvement >= 2.1:#:4c-2.06 #Increase Frequency
                        Sorted_CoreMTTF=CoreMTTFsortHighToLow(accumulative_MTTF_per_core,num_interval-1)

                        if MTTF_improvement >= 4: #6 : #4c:3 #Increase one level frequency in All cores 

                            for i in range (0 , np):
                                isFreqChanged, sim_time_coefficient, DVFS_list, DVFS_list_names = increase_frequency(i,1,freq_domain,DVFS_list,DVFS_list_names,np) # Increase freq of ith core for 1 level
                    
#                       elif MTTF_improvement >= 2.06 and MTTF_improvement < 3:
#                            np_change_freq=int(float(1.0-(float(3.0 - MTTF_improvement)/0.95))*np)

#                        elif MTTF_improvement >= 2.06 and MTTF_improvement < 6:
#                            np_change_freq=int(float(1.0-(float(6.0 - MTTF_improvement)/3.9))*np)

                        elif MTTF_improvement >= 2.06 and MTTF_improvement < 4:
                            np_change_freq=int(float(1.0-(float(4.0 - MTTF_improvement)/3))*np)#1.95

                            np_change_freq+=1
                            if np_change_freq>np:
                                np_change_freq=np

                            ith=0
                            while ith< np_change_freq:
                                core_index = Sorted_CoreMTTF [ith]                   
                                isFreqChanged, sim_time_coefficient, DVFS_list, DVFS_list_names = increase_frequency(core_index,1,freq_domain,DVFS_list,DVFS_list_names,np) 
                                if isFreqChanged == 0 : #it was not changed
                                    print 'isFreqChanged == 0 , np_change_freq = %s'%np_change_freq
                                    np_change_freq+=1
                                    if np_change_freq>np:
                                         np_change_freq=np
                                    print 'new_np_change_freq = %s'%np_change_freq 
                                ith+=1
                    ########################################## Decrease Frequency ##########################################
                    elif MTTF_improvement <= 1.9: #4c-1.94: #Decrease Frequency 
                        Sorted_CoreMTTF=CoreMTTFsortLowToHigh(accumulative_MTTF_per_core,num_interval-1)

                        if MTTF_improvement <= .9:#.5: #4c-1 : #Decrease one level frequency in All cores 
                            for i in range (0 , np):
                                isFreqChanged, sim_time_coefficient, DVFS_list, DVFS_list_names = decrease_frequency(i,1,freq_domain,DVFS_list,DVFS_list_names,np) # decrease freq of ith core for 1 level
 
#                    elif MTTF_improvement <= 1.95 and MTTF_improvement > 1:
#                        np_change_freq=int(float((1.95 - MTTF_improvement)/0.95)*np)
                    
#                        elif MTTF_improvement <= 1.9 and MTTF_improvement > 0.5:
#                            np_change_freq=int(float((1.9 - MTTF_improvement)/3)*np)
                        elif MTTF_improvement <= 1.9 and MTTF_improvement > 0.9:
                            np_change_freq=int(float((1.9 - MTTF_improvement)/1)*np)#1.5


                            np_change_freq+=1 #round up
                            if np_change_freq>np:
                                np_change_freq-1

                            ith=0
                            while ith< np_change_freq:
                                core_index = Sorted_CoreMTTF [ith]                   
                                isFreqChanged, sim_time_coefficient, DVFS_list, DVFS_list_names = decrease_frequency(core_index,1,freq_domain,DVFS_list,DVFS_list_names,np) 
                                if isFreqChanged == 0 : #it was not changed
                                    print 'isFreqChanged == 0 , np_change_freq = %s'%np_change_freq
                                    np_change_freq+=1
                                    if np_change_freq>np:
                                        np_change_freq=np
                                    print 'new_np_change_freq = %s'%np_change_freq 
                                ith+=1
 
            #############################################################################################################      
            #Thread Migration : If total MTTF is less than target, swapt Hot tiles with Cold tiles
            ############################## ThreadMigration Based DRM ###################################
            ############################################################################################
            if Thread_Migration_Technique:
                print"\nRunning Thread Migration\n"

                if (num_interval >=1 ):
                    if accumulative_stats_calculation:
                        MTTF_improvement =  float(accumulative_MTTF_per_interval[num_interval-1])/float(MTTF_refrence) 
                    else: 
                        MTTF_improvement =  float(MTTF_per_interval[num_interval-1])/float(MTTF_refrence)
                    print 'MTTF_improvement = %s\n'%MTTF_improvement
                    
                    
                    ########################################## Swap Hot with Cold ########################################
                    if MTTF_improvement <= 1.9:  
                        if accumulative_stats_calculation:
                            Sorted_CoreMTTF=CoreMTTFsortHighToLow(accumulative_MTTF_per_core,num_interval-1)
                        else:
                            Sorted_CoreMTTF=CoreMTTFsortHighToLow(MTTF_per_core_per_interval,num_interval-1)

                        print "Sorted_CoreMTTF(immediate) = %s"%  Sorted_CoreMTTF
                        
                        for i in range(0,np):
                            SwappedThreads[i]=Sorted_CoreMTTF[i]
                        if MTTF_improvement <= 1.9:  
                            print "int(np/2) = %s"%int(np/2)
                            for i in range (0 , int(np/2)):
                                #swap(SwappedThreads[i],SwappedThreads[np-1-i])
                                temp = SwappedThreads[np-1-i]
                                SwappedThreads[np-1-i] = SwappedThreads[i]
                                SwappedThreads[i] = temp
                            for i in range (0,np):
                                for j in range (0,np):
                                    if Sorted_CoreMTTF[j]==i:
                                          MigratedThreads[i]=SwappedThreads[j]                                           
                    
#                        elif MTTF_improvement <= 1.9 and MTTF_improvement > 0.5:
#                            np_swap_thread=int(float((1.9 - MTTF_improvement)/3)*np)
#                            np_swap_thread+=1 #round up
#                            np_swap_thread/=2
#                            for i in range(0,np_swap_thread):
#                                #swap(SwappedThreads[i],SwappedThreads[np-1-i])                                               
#                                temp = SwappedThreads[np-1-i]
#                                SwappedThreads[np-1-i] = SwappedThreads[i]
#                                SwappedThreads[i] = temp
#                            for i in range (0,np):
#                                for j in range (0,np):
#                                    if Sorted_CoreMTTF[j]==i:
#                                          MigratedThreads[i]=SwappedThreads[j]   
                
                print "Sorted_CoreMTTF = %s"%  Sorted_CoreMTTF
                print "SwappedThreads = %s"%  SwappedThreads
                print "MigratedThreads = %s"%  MigratedThreads

#temp
                m5.drain(root.system)
                m5.switchCpus(root.system,[(DVFS_list[i],MAIN_CPU[MigratedThreads[i]]) for i in xrange(np)],do_drain=False)
                m5.resume(root.system)

            ##############################################################################################################
            ##################### Common Part ##################################
#temp
            if DVFS_Technique or Thread_Migration_Technique or Ref_Interval_Calc:
                #it is necessary to maintain the correct frequencies either for ThreadMigration or DVFS
                if DVFS_Technique or Thread_Migration_Technique or Ref_Interval_Calc_with_switchCpu:
                    m5.drain(root.system)
                    m5.switchCpus(root.system,[(MAIN_CPU[i],DVFS_list[i]) for i in xrange(np)],do_drain=False)
                    #atomic
#                    m5.switchCpus(root.system,[(Atomic_CPU[i],DVFS_list[i]) for i in xrange(np)],do_drain=False)
                    m5.resume(root.system)
                    print"3: Switch OK\n"


                INFO_FILE=open("%s/DRM/run_info/info"%GEM5_DIR , "a")        
                if num_interval>0:
                    pass_to_bash=''
                    for i in xrange(0,np-1):            
                       pass_to_bash+='%s,'%Sorted_CoreMTTF[i]
                    pass_to_bash+=str(Sorted_CoreMTTF[np-1])
                    INFO_FILE.write('Sorted_CoreMTTF\n') 
                    INFO_FILE.write('%s %s'%(num_interval,pass_to_bash)) 
                    INFO_FILE.write('\n')

                    pass_to_bash=''
                    for i in xrange(0,np-1):            
                       pass_to_bash+='%s,'%SwappedThreads[i]
                    pass_to_bash+=str(SwappedThreads[np-1])
                    INFO_FILE.write('SwappedThreads\n') 
                    INFO_FILE.write('%s %s'%(num_interval,pass_to_bash)) 
                    INFO_FILE.write('\n')
            
                    pass_to_bash=''
                    for i in xrange(0,np-1):            
                       pass_to_bash+='%s->%s,'%(i,MigratedThreads[i])
                    pass_to_bash+='%s->%s'%(np-1,MigratedThreads[np-1])
                    INFO_FILE.write('MigratedThreads\n') 
                    INFO_FILE.write('%s %s'%(num_interval,pass_to_bash)) 
                    INFO_FILE.write('\n')



                pass_to_bash=''
                for i in xrange(0,np-1):            
                    pass_to_bash+='%s,'%DVFS_list_names[i]
                pass_to_bash+=DVFS_list_names[np-1]
                INFO_FILE.write('%s %s'%(num_interval,pass_to_bash)) 
                INFO_FILE.write('\n')

                pass_to_bash=''
                for i in xrange(0,np-1):            
                    pass_to_bash+='%s,'%MigratedThreads[i]
                pass_to_bash+=str(MigratedThreads[np-1])
#                INFO_FILE.write('MigratedThreads\n') 
                INFO_FILE.write('%s %s'%(num_interval,pass_to_bash)) 
                INFO_FILE.write('\n')


                print 'sim_time_coefficient = %s\n'%sim_time_coefficient

                 

                simulation_time=int(sim_time_coefficient * interval_tick_duration)            

#                m5.stats.reset()
                m5.simulate(simulation_time) 
                m5.stats.dump()
                if  accumulative_stats_calculation: 
                    m5.stats.reset() 
                INFO_FILE.close()
                subprocess.call("%s/DRM_files/flow_for_interval_%scores.sh"%(GEM5_DIR,np))
#################### per interval MTTF
                for core_number in xrange (0,np):
                    MTTF_FILE=open("%s/DRM/interval_%s/MTTF_per_core_per_interval/MTTFcore%s.txt"%(GEM5_DIR,num_interval,core_number),"r")
                    for line in MTTF_FILE:
                        if 'SoC final MTTF:'in line:
                            temp=line.split(': ')
                    MTTF_FILE.close()
                    print '%s,%s,%s' %(num_interval,core_number,temp[1])
                    MTTF_per_core_per_interval[num_interval][core_number]=temp[1]
                    print 'MTTF_per_core_per_interval[%s][%s] = %s' %(num_interval,  core_number ,MTTF_per_core_per_interval[num_interval][core_number])           
                    if num_interval>=1 :
                        MTTF_change_per_core_per_interval[num_interval][core_number]= (float(MTTF_per_core_per_interval[num_interval][core_number]) / float( MTTF_per_core_per_interval[num_interval-1][core_number]))

                pass_to_bash1=''
                for i in xrange(0,np):            
                   pass_to_bash1+='MTTF_%s= %.3g , MTTF_changed= %.3g\n'%(DVFS_list_names[i],float(MTTF_per_core_per_interval[num_interval][i]),float(MTTF_change_per_core_per_interval[num_interval][i]))             
                print pass_to_bash1
######################

                MTTF_FILE=open("%s/DRM/interval_%s/MTTF_system_per_interval.txt"%(GEM5_DIR,num_interval),"r")
                for line in MTTF_FILE:
                    if 'SoC final MTTF:'in line:
                        temp=line.split(': ')
                MTTF_FILE.close()
                print '%s,%s' %(num_interval,temp[1])
                MTTF_per_interval[num_interval]=temp[1]
                print 'MTTF_per_interval[%s] = %s' %(num_interval, MTTF_per_interval[num_interval])           
                if num_interval>=1 :
                    MTTF_change_per_interval[num_interval]= (float(MTTF_per_interval[num_interval]) / float( MTTF_per_interval[num_interval-1]))

                pass_to_bash2=''
                pass_to_bash2='MTTF= %.3g , MTTF_changed= %.3g , TotalImprovement= %.3g \n'%(float(MTTF_per_interval[num_interval]),float(MTTF_change_per_interval[num_interval]), (float(MTTF_per_interval[num_interval]) / float( MTTF_per_interval[0])))             
                print pass_to_bash2
######################

                INFO_FILE=open("%s/DRM/run_info/info"%GEM5_DIR , "a")        
                INFO_FILE.write('\n')
                INFO_FILE.write('%s'%pass_to_bash1)
                INFO_FILE.write('%s'%pass_to_bash2)
#            INFO_FILE.write('\n')
######################
            # accumulative MTTF
                for core_number in xrange (0,np):
                    MTTF_FILE=open("%s/DRM/interval_%s/accumulative_MTTF_per_core/MTTFcore%s.txt"%(GEM5_DIR,num_interval,core_number),"r")
                    for line in MTTF_FILE:
                        if 'SoC final MTTF:'in line:
                            temp=line.split(': ')
                    MTTF_FILE.close()
                    print '%s,%s,%s' %(num_interval,core_number,temp[1])
                    accumulative_MTTF_per_core[num_interval][core_number]=temp[1]
                    print 'accumulative_MTTF_per_core[%s][%s] = %s' %(num_interval,  core_number ,accumulative_MTTF_per_core[num_interval][core_number])           
                    if num_interval>=1 :
                        accumulative_MTTF_change_per_core[num_interval][core_number]= (float(accumulative_MTTF_per_core[num_interval][core_number]) / float( accumulative_MTTF_per_core[num_interval-1][core_number]))
                pass_to_bash1=''
                for i in xrange(0,np):            
                   pass_to_bash1+='MTTF_%s= %.3g , MTTF_changed= %.3g\n'%(DVFS_list_names[i],float(accumulative_MTTF_per_core[num_interval][i]),float(accumulative_MTTF_change_per_core[num_interval][i]))             
                print pass_to_bash1
#######################
                MTTF_FILE=open("%s/DRM/interval_%s/accumulative_MTTF_system.txt"%(GEM5_DIR,num_interval),"r")
                for line in MTTF_FILE:
                    if 'SoC final MTTF:'in line:
                        temp=line.split(': ')
                MTTF_FILE.close()
                print '%s,%s' %(num_interval,temp[1])
                accumulative_MTTF_per_interval[num_interval]=temp[1]
                print 'MTTF_per_interval[%s] = %s' %(num_interval, accumulative_MTTF_per_interval[num_interval])           
                if num_interval>=1 :
                    accumulative_MTTF_change_per_interval[num_interval]= (float(accumulative_MTTF_per_interval[num_interval]) / float( accumulative_MTTF_per_interval[num_interval-1]))

                pass_to_bash2=''
                pass_to_bash2='AVG-MTTF= %.3g , MTTF_changed= %.3g , TotalImprovement= %.3gX , Speed=%.2fX\n'%(float(accumulative_MTTF_per_interval[num_interval]),float(accumulative_MTTF_change_per_interval[num_interval]),(float(accumulative_MTTF_per_interval[num_interval]) / MTTF_refrence),sim_time_coefficient)             
                print pass_to_bash2

#######################
                INFO_FILE.write('%s'%pass_to_bash1)
                INFO_FILE.write('%s'%pass_to_bash2)
                INFO_FILE.write('\n')



                INFO_FILE.close()
                # TODO : reset migrated threads
#################################################################################################################
#################################################################################################################
#################################################################################################################
#################################################################################################################


        print "\n end of ROI"        


        # If checkpoints are being taken, then the checkpoint instruction
        # will occur in the benchmark code it self.
        if options.repeat_switch and maxtick > options.repeat_switch:
            print"\noptions.repeat_switch and maxtick > options.repeat_switch"
            exit_event = repeatSwitch(testsys, repeat_switch_cpu_list,
                                      maxtick, options.repeat_switch)        
        else:
            print"\n else :options.repeat_switch and maxtick > options.repeat_switch" 
            exit_event = benchCheckpoints(options, maxtick, cptdir)
            print"\n end else :options.repeat_switch and maxtick > options.repeat_switch "
    print"\nexit RUN"
    print 'Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause())
    if options.checkpoint_at_end:
        print"\options.checkpoint_at_endn"
        m5.checkpoint(joinpath(cptdir, "cpt.%d"))

    if not m5.options.interactive:
        print "\not m5.options.interactiven"
        sys.exit(exit_event.getCode())
    print "\n\n\n END RUN\n\n\n"
