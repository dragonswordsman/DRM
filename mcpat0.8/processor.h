/*****************************************************************************
 *                                McPAT
 *                      SOFTWARE LICENSE AGREEMENT
 *            Copyright 2009 Hewlett-Packard Development Company, L.P.
 *                          All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Any User of the software ("User"), by accessing and using it, agrees to the
 * terms and conditions set forth herein, and hereby grants back to Hewlett-
 * Packard Development Company, L.P. and its affiliated companies ("HP") a
 * non-exclusive, unrestricted, royalty-free right and license to copy,
 * modify, distribute copies, create derivate works and publicly display and
 * use, any changes, modifications, enhancements or extensions made to the
 * software by User, including but not limited to those affording
 * compatibility with other hardware or software, but excluding pre-existing
 * software applications that may incorporate the software.  User further
 * agrees to use its best efforts to inform HP of any such changes,
 * modifications, enhancements or extensions.
 *
 * Correspondence should be provided to HP at:
 *
 * Director of Intellectual Property Licensing
 * Office of Strategy and Technology
 * Hewlett-Packard Company
 * 1501 Page Mill Road
 * Palo Alto, California  94304
 *
 * The software may be further distributed by User (but not offered for
 * sale or transferred for compensation) to third parties, under the
 * condition that such third parties agree to abide by the terms and
 * conditions of this license.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITH ANY AND ALL ERRORS AND DEFECTS
 * AND USER ACKNOWLEDGES THAT THE SOFTWARE MAY CONTAIN ERRORS AND DEFECTS.
 * HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THE SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL
 * HP BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE SOFTWARE.
 *
 ***************************************************************************/
#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include "XML_Parse.h"
#include "area.h"
#include "decoder.h"
#include "parameter.h"
#include "array.h"
#include "arbiter.h"
#include <vector>
#include "basic_components.h"
#include "core.h"
#include "memoryctrl.h"
#include "router.h"
#include "sharedcache.h"
#include "noc.h"
#include "iocontrollers.h"

class Processor : public Component
{
  public:
	ParseXML *XML;
	vector<Core *> cores;
    vector<SharedCache *> l2array;
    vector<SharedCache *> l3array;
    vector<SharedCache *> l1dirarray;
    vector<SharedCache *> l2dirarray;
    vector<NoC *>  nocs;
    MemoryController * mc;
    NIUController    * niu;
    PCIeController   * pcie;
    FlashController  * flashcontroller;
    InputParameter interface_ip;
    ProcParam procdynp;
    //wire	globalInterconnect;
    //clock_network globalClock;
    Component core, l2, l3, l1dir, l2dir, noc, mcs, cc, nius, pcies,flashcontrollers;
    int  numCore, numL2, numL3, numNOC, numL1Dir, numL2Dir;
    Processor(ParseXML *XML_interface);
    void compute();
    void set_proc_param();
    void displayEnergy(char *outfile_name, 
        uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
    void displayDeviceType(int device_type_, uint32_t indent = 0);
    void displayInterconnectType(int interconnect_type_, uint32_t indent = 0);
    ~Processor();
};

#endif /* PROCESSOR_H_ */
