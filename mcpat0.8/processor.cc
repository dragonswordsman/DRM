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
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <fstream>
#include "parameter.h"
#include "array.h"
#include "const.h"
#include "basic_circuit.h"
#include "XML_Parse.h"
#include "processor.h"
#include "version.h"


Processor::Processor(ParseXML *XML_interface)
:XML(XML_interface),//TODO: using one global copy may have problems.
 mc(0),
 niu(0),
 pcie(0),
 flashcontroller(0)
{
  /*
   *  placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
   *  There is no point to have heterogeneous memory controller on chip,
   *  thus McPAT only support homogeneous memory controllers.
   */
  int i;
  double pppm_t[4]    = {1,1,1,1};
  set_proc_param();
  if (procdynp.homoCore)
	  numCore = procdynp.numCore==0? 0:1;
  else
	  numCore = procdynp.numCore;

  if (procdynp.homoL2)
	  numL2 = procdynp.numL2==0? 0:1;
  else
	  numL2 = procdynp.numL2;

  if (XML->sys.Private_L2 && numCore != numL2)
  {
	  cout<<"Number of private L2 does not match number of cores"<<endl;
      exit(0);
  }

  if (procdynp.homoL3)
	  numL3 = procdynp.numL3==0? 0:1;
  else
	  numL3 = procdynp.numL3;

  if (procdynp.homoNOC)
	  numNOC = procdynp.numNOC==0? 0:1;
  else
	  numNOC = procdynp.numNOC;

//  if (!procdynp.homoNOC)
//  {
//	  cout<<"Current McPAT does not support heterogeneous NOC"<<endl;
//      exit(0);
//  }

  if (procdynp.homoL1Dir)
	  numL1Dir = procdynp.numL1Dir==0? 0:1;
  else
	  numL1Dir = procdynp.numL1Dir;

  if (procdynp.homoL2Dir)
	  numL2Dir = procdynp.numL2Dir==0? 0:1;
  else
	  numL2Dir = procdynp.numL2Dir;

  for (i = 0;i < numCore; i++)
  {
		  cores.push_back(new Core(XML,i, &interface_ip));
		  cores[i]->computeEnergy();
		  cores[i]->computeEnergy(false);
		  if (procdynp.homoCore){
			  core.area.set_area(core.area.get_area() + cores[i]->area.get_area()*procdynp.numCore);
			  set_pppm(pppm_t,cores[i]->clockRate*procdynp.numCore, procdynp.numCore,procdynp.numCore,procdynp.numCore);
			  core.power = core.power + cores[i]->power*pppm_t;
			  set_pppm(pppm_t,1/cores[i]->executionTime, procdynp.numCore,procdynp.numCore,procdynp.numCore);
			  core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
			  area.set_area(area.get_area() + core.area.get_area());//placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
			  power = power  + core.power;
			  rt_power = rt_power  + core.rt_power;
		  }
		  else{
			  core.area.set_area(core.area.get_area() + cores[i]->area.get_area());
			  area.set_area(area.get_area() + cores[i]->area.get_area());//placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm

			  set_pppm(pppm_t,cores[i]->clockRate, 1, 1, 1);
			  core.power = core.power + cores[i]->power*pppm_t;
			  power = power  + cores[i]->power*pppm_t;

			  set_pppm(pppm_t,1/cores[i]->executionTime, 1, 1, 1);
			  core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
			  rt_power = rt_power  + cores[i]->rt_power*pppm_t;
		  }
  }

  if (!XML->sys.Private_L2)
  {
  if (numL2 >0)
	  for (i = 0;i < numL2; i++)
	  {
		  l2array.push_back(new SharedCache(XML,i, &interface_ip));
		  l2array[i]->computeEnergy();
		  l2array[i]->computeEnergy(false);
		  if (procdynp.homoL2){
			  l2.area.set_area(l2.area.get_area() + l2array[i]->area.get_area()*procdynp.numL2);
			  set_pppm(pppm_t,l2array[i]->cachep.clockRate*procdynp.numL2, procdynp.numL2,procdynp.numL2,procdynp.numL2);
			  l2.power = l2.power + l2array[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l2array[i]->cachep.executionTime, procdynp.numL2,procdynp.numL2,procdynp.numL2);
			  l2.rt_power = l2.rt_power + l2array[i]->rt_power*pppm_t;
			  area.set_area(area.get_area() + l2.area.get_area());//placement and routing overhead is 10%, l2 scales worse than cache 40% is accumulated from 90 to 22nm
			  power = power  + l2.power;
			  rt_power = rt_power  + l2.rt_power;
		  }
		  else{
			  l2.area.set_area(l2.area.get_area() + l2array[i]->area.get_area());
			  area.set_area(area.get_area() + l2array[i]->area.get_area());//placement and routing overhead is 10%, l2 scales worse than cache 40% is accumulated from 90 to 22nm

			  set_pppm(pppm_t,l2array[i]->cachep.clockRate, 1, 1, 1);
			  l2.power = l2.power + l2array[i]->power*pppm_t;
			  power = power  + l2array[i]->power*pppm_t;;
			  set_pppm(pppm_t,1/l2array[i]->cachep.executionTime, 1, 1, 1);
			  l2.rt_power = l2.rt_power + l2array[i]->rt_power*pppm_t;
			  rt_power = rt_power  + l2array[i]->rt_power*pppm_t;
		  }
	  }
  }

  if (numL3 >0)
	  for (i = 0;i < numL3; i++)
	  {
		  l3array.push_back(new SharedCache(XML,i, &interface_ip, L3));
		  l3array[i]->computeEnergy();
		  l3array[i]->computeEnergy(false);
		  if (procdynp.homoL3){
			  l3.area.set_area(l3.area.get_area() + l3array[i]->area.get_area()*procdynp.numL3);
			  set_pppm(pppm_t,l3array[i]->cachep.clockRate*procdynp.numL3, procdynp.numL3,procdynp.numL3,procdynp.numL3);
			  l3.power = l3.power + l3array[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l3array[i]->cachep.executionTime, procdynp.numL3,procdynp.numL3,procdynp.numL3);
              l3.rt_power = l3.rt_power + l3array[i]->rt_power*pppm_t;
			  area.set_area(area.get_area() + l3.area.get_area());//placement and routing overhead is 10%, l3 scales worse than cache 40% is accumulated from 90 to 22nm
			  power = power  + l3.power;
			  rt_power = rt_power  + l3.rt_power;

		  }
		  else{
			  l3.area.set_area(l3.area.get_area() + l3array[i]->area.get_area());
			  area.set_area(area.get_area() + l3array[i]->area.get_area());//placement and routing overhead is 10%, l3 scales worse than cache 40% is accumulated from 90 to 22nm
			  set_pppm(pppm_t,l3array[i]->cachep.clockRate, 1, 1, 1);
			  l3.power = l3.power + l3array[i]->power*pppm_t;
			  power = power  + l3array[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l3array[i]->cachep.executionTime, 1, 1, 1);
              l3.rt_power = l3.rt_power + l3array[i]->rt_power*pppm_t;
              rt_power = rt_power  + l3array[i]->rt_power*pppm_t;

		  }
	  }
  if (numL1Dir >0)
	  for (i = 0;i < numL1Dir; i++)
	  {
		  l1dirarray.push_back(new SharedCache(XML,i, &interface_ip, L1Directory));
		  l1dirarray[i]->computeEnergy();
		  l1dirarray[i]->computeEnergy(false);
		  if (procdynp.homoL1Dir){
			  l1dir.area.set_area(l1dir.area.get_area() + l1dirarray[i]->area.get_area()*procdynp.numL1Dir);
			  set_pppm(pppm_t,l1dirarray[i]->cachep.clockRate*procdynp.numL1Dir, procdynp.numL1Dir,procdynp.numL1Dir,procdynp.numL1Dir);
			  l1dir.power = l1dir.power + l1dirarray[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l1dirarray[i]->cachep.executionTime, procdynp.numL1Dir,procdynp.numL1Dir,procdynp.numL1Dir);
              l1dir.rt_power = l1dir.rt_power + l1dirarray[i]->rt_power*pppm_t;
			  area.set_area(area.get_area() + l1dir.area.get_area());//placement and routing overhead is 10%, l1dir scales worse than cache 40% is accumulated from 90 to 22nm
			  power = power  + l1dir.power;
			  rt_power = rt_power  + l1dir.rt_power;

		  }
		  else{
			  l1dir.area.set_area(l1dir.area.get_area() + l1dirarray[i]->area.get_area());
			  area.set_area(area.get_area() + l1dirarray[i]->area.get_area());
			  set_pppm(pppm_t,l1dirarray[i]->cachep.clockRate, 1, 1, 1);
			  l1dir.power = l1dir.power + l1dirarray[i]->power*pppm_t;
			  power = power  + l1dirarray[i]->power;
			  set_pppm(pppm_t,1/l1dirarray[i]->cachep.executionTime, 1, 1, 1);
              l1dir.rt_power = l1dir.rt_power + l1dirarray[i]->rt_power*pppm_t;
			  rt_power = rt_power  + l1dirarray[i]->rt_power;
		  }
	  }

  if (numL2Dir >0)
	  for (i = 0;i < numL2Dir; i++)
	  {
		  l2dirarray.push_back(new SharedCache(XML,i, &interface_ip, L2Directory));
		  l2dirarray[i]->computeEnergy();
		  l2dirarray[i]->computeEnergy(false);
		  if (procdynp.homoL2Dir){
			  l2dir.area.set_area(l2dir.area.get_area() + l2dirarray[i]->area.get_area()*procdynp.numL2Dir);
			  set_pppm(pppm_t,l2dirarray[i]->cachep.clockRate*procdynp.numL2Dir, procdynp.numL2Dir,procdynp.numL2Dir,procdynp.numL2Dir);
			  l2dir.power = l2dir.power + l2dirarray[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l2dirarray[i]->cachep.executionTime, procdynp.numL2Dir,procdynp.numL2Dir,procdynp.numL2Dir);
              l2dir.rt_power = l2dir.rt_power + l2dirarray[i]->rt_power*pppm_t;
			  area.set_area(area.get_area() + l2dir.area.get_area());//placement and routing overhead is 10%, l2dir scales worse than cache 40% is accumulated from 90 to 22nm
			  power = power  + l2dir.power;
			  rt_power = rt_power  + l2dir.rt_power;

		  }
		  else{
			  l2dir.area.set_area(l2dir.area.get_area() + l2dirarray[i]->area.get_area());
			  area.set_area(area.get_area() + l2dirarray[i]->area.get_area());
			  set_pppm(pppm_t,l2dirarray[i]->cachep.clockRate, 1, 1, 1);
			  l2dir.power = l2dir.power + l2dirarray[i]->power*pppm_t;
			  power = power  + l2dirarray[i]->power*pppm_t;
			  set_pppm(pppm_t,1/l2dirarray[i]->cachep.executionTime, 1, 1, 1);
              l2dir.rt_power = l2dir.rt_power + l2dirarray[i]->rt_power*pppm_t;
			  rt_power = rt_power  + l2dirarray[i]->rt_power*pppm_t;
		  }
	  }

  if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
  {
	  mc = new MemoryController(XML, &interface_ip, MC);
	  mc->computeEnergy();
	  mc->computeEnergy(false);
	  mcs.area.set_area(mcs.area.get_area()+mc->area.get_area()*XML->sys.mc.number_mcs);
	  area.set_area(area.get_area()+mc->area.get_area()*XML->sys.mc.number_mcs);
	  set_pppm(pppm_t,XML->sys.mc.number_mcs*mc->mcp.clockRate, XML->sys.mc.number_mcs,XML->sys.mc.number_mcs,XML->sys.mc.number_mcs);
	  mcs.power = mc->power*pppm_t;
	  power = power  + mcs.power;
	  set_pppm(pppm_t,1/mc->mcp.executionTime, XML->sys.mc.number_mcs,XML->sys.mc.number_mcs,XML->sys.mc.number_mcs);
	  mcs.rt_power = mc->rt_power*pppm_t;
	  rt_power = rt_power  + mcs.rt_power;

  }

  if (XML->sys.flashc.number_mcs >0 )//flash controller
  {
	  flashcontroller = new FlashController(XML, &interface_ip);
	  flashcontroller->computeEnergy();
	  flashcontroller->computeEnergy(false);
	  double number_fcs = flashcontroller->fcp.num_mcs;
	  flashcontrollers.area.set_area(flashcontrollers.area.get_area()+flashcontroller->area.get_area()*number_fcs);
	  area.set_area(area.get_area()+flashcontrollers.area.get_area());
	  set_pppm(pppm_t,number_fcs, number_fcs ,number_fcs, number_fcs );
	  flashcontrollers.power = flashcontroller->power*pppm_t;
	  power = power  + flashcontrollers.power;
	  set_pppm(pppm_t,number_fcs , number_fcs ,number_fcs ,number_fcs );
	  flashcontrollers.rt_power = flashcontroller->rt_power*pppm_t;
	  rt_power = rt_power  + flashcontrollers.rt_power;

  }

  if (XML->sys.niu.number_units >0)
  {
	  niu = new NIUController(XML, &interface_ip);
	  niu->computeEnergy();
	  niu->computeEnergy(false);
	  nius.area.set_area(nius.area.get_area()+niu->area.get_area()*XML->sys.niu.number_units);
	  area.set_area(area.get_area()+niu->area.get_area()*XML->sys.niu.number_units);
	  set_pppm(pppm_t,XML->sys.niu.number_units*niu->niup.clockRate, XML->sys.niu.number_units,XML->sys.niu.number_units,XML->sys.niu.number_units);
	  nius.power = niu->power*pppm_t;
	  power = power  + nius.power;
	  set_pppm(pppm_t,XML->sys.niu.number_units*niu->niup.clockRate, XML->sys.niu.number_units,XML->sys.niu.number_units,XML->sys.niu.number_units);
	  nius.rt_power = niu->rt_power*pppm_t;
	  rt_power = rt_power  + nius.rt_power;

  }

  if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels >0)
  {
	  pcie = new PCIeController(XML, &interface_ip);
	  pcie->computeEnergy();
	  pcie->computeEnergy(false);
	  pcies.area.set_area(pcies.area.get_area()+pcie->area.get_area()*XML->sys.pcie.number_units);
	  area.set_area(area.get_area()+pcie->area.get_area()*XML->sys.pcie.number_units);
	  set_pppm(pppm_t,XML->sys.pcie.number_units*pcie->pciep.clockRate, XML->sys.pcie.number_units,XML->sys.pcie.number_units,XML->sys.pcie.number_units);
	  pcies.power = pcie->power*pppm_t;
	  power = power  + pcies.power;
	  set_pppm(pppm_t,XML->sys.pcie.number_units*pcie->pciep.clockRate, XML->sys.pcie.number_units,XML->sys.pcie.number_units,XML->sys.pcie.number_units);
	  pcies.rt_power = pcie->rt_power*pppm_t;
	  rt_power = rt_power  + pcies.rt_power;

  }

  if (numNOC >0)
  {
	  for (i = 0;i < numNOC; i++)
	  {
		  if (XML->sys.NoC[i].type)
		  {//First add up area of routers if NoC is used
			  nocs.push_back(new NoC(XML,i, &interface_ip, 1));
			  if (procdynp.homoNOC)
			  {
				  noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area()*procdynp.numNOC);
				  area.set_area(area.get_area() + noc.area.get_area());
			  }
			  else
			  {
				  noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area());
				  area.set_area(area.get_area() + nocs[i]->area.get_area());
			  }
		  }
		  else
		  {//Bus based interconnect
			  nocs.push_back(new NoC(XML,i, &interface_ip, 1, sqrt(area.get_area()*XML->sys.NoC[i].chip_coverage)));
			  if (procdynp.homoNOC){
				  noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area()*procdynp.numNOC);
				  area.set_area(area.get_area() + noc.area.get_area());
			  }
			  else
			  {
				  noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area());
				  area.set_area(area.get_area() + nocs[i]->area.get_area());
			  }
		  }
	  }

	  /*
	   * Compute global links associated with each NOC, if any. This must be done at the end (even after the NOC router part) since the total chip
	   * area must be obtain to decide the link routing
	   */
	  for (i = 0;i < numNOC; i++)
	  {
		  if (nocs[i]->nocdynp.has_global_link && XML->sys.NoC[i].type)
		  {
			  nocs[i]->init_link_bus(sqrt(area.get_area()*XML->sys.NoC[i].chip_coverage));//compute global links
			  if (procdynp.homoNOC)
			  {
				  noc.area.set_area(noc.area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						  * nocs[i]->nocdynp.total_nodes
						  * procdynp.numNOC);
				  area.set_area(area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						  * nocs[i]->nocdynp.total_nodes
						  * procdynp.numNOC);
			  }
			  else
			  {
				  noc.area.set_area(noc.area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						  * nocs[i]->nocdynp.total_nodes);
				  area.set_area(area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						  * nocs[i]->nocdynp.total_nodes);
			  }
		  }
	  }
	  //Compute energy of NoC (w or w/o links) or buses
	  for (i = 0;i < numNOC; i++)
	  {
		  nocs[i]->computeEnergy();
		  nocs[i]->computeEnergy(false);
		  if (procdynp.homoNOC){
			  set_pppm(pppm_t,procdynp.numNOC*nocs[i]->nocdynp.clockRate, procdynp.numNOC,procdynp.numNOC,procdynp.numNOC);
			  noc.power = noc.power + nocs[i]->power*pppm_t;
			  set_pppm(pppm_t,1/nocs[i]->nocdynp.executionTime, procdynp.numNOC,procdynp.numNOC,procdynp.numNOC);
			  noc.rt_power = noc.rt_power + nocs[i]->rt_power*pppm_t;
			  power = power  + noc.power;
			  rt_power = rt_power  + noc.rt_power;
		  }
		  else
		  {
			  set_pppm(pppm_t,nocs[i]->nocdynp.clockRate, 1, 1, 1);
			  noc.power = noc.power + nocs[i]->power*pppm_t;
			  power = power  + nocs[i]->power*pppm_t;
			  set_pppm(pppm_t,1/nocs[i]->nocdynp.executionTime, 1, 1, 1);
			  noc.rt_power = noc.rt_power + nocs[i]->rt_power*pppm_t;
			  rt_power = rt_power  + nocs[i]->rt_power*pppm_t;


		  }
	  }
  }

//  //clock power
//  globalClock.init_wire_external(is_default, &interface_ip);
//  globalClock.clk_area           =area*1e6; //change it from mm^2 to um^2
//  globalClock.end_wiring_level   =5;//toplevel metal
//  globalClock.start_wiring_level =5;//toplevel metal
//  globalClock.l_ip.with_clock_grid=false;//global clock does not drive local final nodes
//  globalClock.optimize_wire();

}

void Processor::displayDeviceType(int device_type_, uint32_t indent)
{
	string indent_str(indent, ' ');

	switch ( device_type_ ) {

	  case 0 :
		  cout <<indent_str<<"Device Type= "<<"ITRS high performance device type"<<endl;
	    break;
	  case 1 :
		  cout <<indent_str<<"Device Type= "<<"ITRS low standby power device type"<<endl;
	    break;
	  case 2 :
		  cout <<indent_str<<"Device Type= "<<"ITRS low operating power device type"<<endl;
	    break;
	  case 3 :
		  cout <<indent_str<<"Device Type= "<<"LP-DRAM device type"<<endl;
	    break;
	  case 4 :
		  cout <<indent_str<<"Device Type= "<<"COMM-DRAM device type"<<endl;
	    break;
	  default :
		  {
			  cout <<indent_str<<"Unknown Device Type"<<endl;
			  exit(0);
		  }
	}
}

void Processor::displayInterconnectType(int interconnect_type_, uint32_t indent)
{
	string indent_str(indent, ' ');

	switch ( interconnect_type_ ) {

	  case 0 :
		  cout <<indent_str<<"Interconnect metal projection= "<<"aggressive interconnect technology projection"<<endl;
	    break;
	  case 1 :
		  cout <<indent_str<<"Interconnect metal projection= "<<"conservative interconnect technology projection"<<endl;
	    break;
	  default :
		  {
			  cout <<indent_str<<"Unknown Interconnect Projection Type"<<endl;
			  exit(0);
		  }
	}
}

void Processor::displayEnergy(char *outfile_name, uint32_t indent, int plevel, bool is_tdp)
{
    int i;
    bool long_channel = XML->sys.longer_channel_device;

    string indent_str(indent, ' ');
    string indent_str_next(indent+2, ' ');

    if (plevel == 6) {
        // (1) this way of doing things is too hard coded and I guess 
        // is just for one type of core architecture, Alpha;
        // here is the change made by Alex to create an output file
        // with all power numbers on one row, as required by hotspot;
        // for routers he inserts a WARN that I guess fixes manually
        // after that?
        // Q: why don't we use directly the power calculations of McPAT for NoC?

        ofstream outputfile;
        outputfile.open( outfile_name); // mcpatoutput.ptrace was Alex's default hard-coded;

        for(i=0; i< numCore; i++){    
            if(i) outputfile << ' ';
            if(i<10) {
                outputfile << "0" << i << "_l2_left" << ' ';
                outputfile << "0" << i << "_l2_right" << ' ';
                outputfile << "0" << i << "_l2_bottom" << ' ';
                outputfile << "0" << i << "_router" << ' ';
                outputfile << "0" << i << "_icache" << ' ';
                outputfile << "0" << i << "_dcache" << ' ';
                outputfile << "0" << i << "_branchpred" << ' ';
                outputfile << "0" << i << "_pc" << ' ';
                outputfile << "0" << i << "_mbox" << ' ';
                outputfile << "0" << i << "_dtb1" << ' ';
                outputfile << "0" << i << "_dtb2" << ' ';
                outputfile << "0" << i << "_rbroadcast" << ' ';
                outputfile << "0" << i << "_itb" << ' ';
                outputfile << "0" << i << "_intexec1" << ' ';
                outputfile << "0" << i << "_intexec2" << ' ';
                outputfile << "0" << i << "_intexec3" << ' ';
                outputfile << "0" << i << "_intexec4" << ' ';
                outputfile << "0" << i << "_rfile1" << ' ';
                outputfile << "0" << i << "_rfile2" << ' ';
                outputfile << "0" << i << "_intmap" << ' ';
                outputfile << "0" << i << "_intq1" << ' ';
                outputfile << "0" << i << "_intq2" << ' ';
                outputfile << "0" << i << "_intq3" << ' ';
                outputfile << "0" << i << "_l2int" << ' ';
                outputfile << "0" << i << "_ldst1" << ' ';
                outputfile << "0" << i << "_ldst2" << ' ';
                outputfile << "0" << i << "_fpmap" << ' ';
                outputfile << "0" << i << "_fpq" << ' ';
                outputfile << "0" << i << "_fpmul" << ' ';
                outputfile << "0" << i << "_fpreg" << ' ';
                outputfile << "0" << i << "_fpadd" << ' ';
                outputfile << "0" << i << "_fpdiv" << ' ';
            } else {
                outputfile << i << "_l2_left" << ' ';
                outputfile << i << "_l2_right" << ' ';
                outputfile << i << "_l2_bottom" << ' ';
                outputfile << i << "_router" << ' ';
                outputfile << i << "_icache" << ' ';
                outputfile << i << "_dcache" << ' ';
                outputfile << i << "_branchpred" << ' ';
                outputfile << i << "_pc" << ' ';
                outputfile << i << "_mbox" << ' ';
                outputfile << i << "_dtb1" << ' ';
                outputfile << i << "_dtb2" << ' ';
                outputfile << i << "_rbroadcast" << ' ';
                outputfile << i << "_itb" << ' ';
                outputfile << i << "_intexec1" << ' ';
                outputfile << i << "_intexec2" << ' ';
                outputfile << i << "_intexec3" << ' ';
                outputfile << i << "_intexec4" << ' ';
                outputfile << i << "_rfile1" << ' ';
                outputfile << i << "_rfile2" << ' ';
                outputfile << i << "_intmap" << ' ';
                outputfile << i << "_intq1" << ' ';
                outputfile << i << "_intq2" << ' ';
                outputfile << i << "_intq3" << ' ';
                outputfile << i << "_l2int" << ' ';
                outputfile << i << "_ldst1" << ' ';
                outputfile << i << "_ldst2" << ' ';
                outputfile << i << "_fpmap" << ' ';
                outputfile << i << "_fpq" << ' ';
                outputfile << i << "_fpmul" << ' ';
                outputfile << i << "_fpreg" << ' ';
                outputfile << i << "_fpadd" << ' ';
                outputfile << i << "_fpdiv" << ' ';
            }
        }
        outputfile << endl;

        // (2) Alex: power statistics must be added in the same order
        // as above to get the file format that HotSpot uses;
        for(i=0; i< numCore; i++) {

            //L2 left
            outputfile << 4*0.162295*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile << 1*0.162295*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile << "0.8325" << ' ';
            //L2 right
            outputfile << 4*0.162295*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile << 1*0.162295*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile <<"0.8325" << ' ';
            //L2 bottom
            outputfile << 4*0.67541*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile << 1*0.67541*((long_channel? l2array[i]->power.readOp.longer_channel_leakage:l2array[i]->power.readOp.leakage)+l2array[i]->power.readOp.gate_leakage+l2array[i]->rt_power.readOp.dynamic/l2array[i]->cachep.executionTime) << ' ';
            //outputfile << "3.46" << ' ';
            //Router
            outputfile << "WARN ";
            //icache
            outputfile << 4*((long_channel? cores[i]->ifu->icache.power.readOp.longer_channel_leakage:cores[i]->ifu->icache.power.readOp.leakage)+cores[i]->ifu->power.readOp.gate_leakage+cores[i]->ifu->icache.rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //outputfile << 4*((long_channel? cores[i]->ifu->icache.power.readOp.longer_channel_leakage:cores[i]->ifu->icache.power.readOp.leakage)+cores[i]->ifu->power.readOp.gate_leakage+rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';

            //cout <<"\nlong_channel? cores[i]->ifu->icache.power.readOp.longer_channel_leakage:cores[i]->ifu->icache.power.readOp.leakage " << (long_channel? cores[i]->ifu->icache.power.readOp.longer_channel_leakage:cores[i]->ifu->icache.power.readOp.leakage) <<"\ncores[i]->ifu->power.readOp.gate_leakage" <<cores[i]->ifu->power.readOp.gate_leakage<<"\nrt_power.readOp.dynamic" << rt_power.readOp.dynamic <<"\ncores[i]->ifu->executionTime" <<cores[i]->ifu->executionTime << ' ';
            
            //dcache
            outputfile << 4*((long_channel? cores[i]->lsu->dcache.power.readOp.longer_channel_leakage:cores[i]->lsu->dcache.power.readOp.leakage )+cores[i]->lsu->dcache.power.readOp.gate_leakage+cores[i]->lsu->dcache.rt_power.readOp.dynamic/cores[i]->lsu->executionTime) << ' ';
            //branchpred    
            outputfile << 4*((long_channel? cores[i]->ifu->BTB->power.readOp.longer_channel_leakage:cores[i]->ifu->BTB->power.readOp.leakage)+cores[i]->ifu->BTB->power.readOp.gate_leakage+cores[i]->ifu->BPT->rt_power.readOp.dynamic/cores[i]->ifu->executionTime+(long_channel? cores[i]->ifu->BPT->power.readOp.longer_channel_leakage:cores[i]->ifu->BPT->power.readOp.leakage)+cores[i]->ifu->BPT->power.readOp.gate_leakage+cores[i]->ifu->BPT->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';                               
            //PC
            outputfile << 4*((long_channel? // ?
                (cores[i]->ifu->ID_inst->power.readOp.longer_channel_leakage + // 1
                cores[i]->ifu->ID_operand->power.readOp.longer_channel_leakage +
                cores[i]->ifu->ID_misc->power.readOp.longer_channel_leakage):
                (cores[i]->ifu->ID_inst->power.readOp.leakage + // 2
                cores[i]->ifu->ID_operand->power.readOp.leakage +
                cores[i]->ifu->ID_misc->power.readOp.leakage)) +
                cores[i]->ifu->ID_inst->power.readOp.gate_leakage +
                cores[i]->ifu->ID_operand->power.readOp.gate_leakage +
                cores[i]->ifu->ID_misc->power.readOp.gate_leakage +
                (cores[i]->ifu->ID_inst->rt_power.readOp.dynamic + 
                cores[i]->ifu->ID_operand->rt_power.readOp.dynamic +
                cores[i]->ifu->ID_misc->rt_power.readOp.dynamic)/cores[i]->ifu->executionTime) << ' ';
            //mbox
            outputfile << 4*((long_channel? cores[i]->mmu->power.readOp.longer_channel_leakage:cores[i]->mmu->power.readOp.leakage)+cores[i]->mmu->power.readOp.gate_leakage+cores[i]->mmu->rt_power.readOp.dynamic/cores[i]->executionTime) << ' ';
            //dtb1
            outputfile << 4*0.5*((long_channel? cores[i]->mmu->dtlb->power.readOp.longer_channel_leakage:cores[i]->mmu->dtlb->power.readOp.leakage)+cores[i]->mmu->dtlb->power.readOp.gate_leakage+cores[i]->mmu->dtlb->rt_power.readOp.dynamic/cores[i]->mmu->executionTime) << ' ';
            //dtb2
            outputfile << 4*0.5*((long_channel? cores[i]->mmu->dtlb->power.readOp.longer_channel_leakage:cores[i]->mmu->dtlb->power.readOp.leakage)+cores[i]->mmu->dtlb->power.readOp.gate_leakage+cores[i]->mmu->dtlb->rt_power.readOp.dynamic/cores[i]->mmu->executionTime) << ' ';
            //rbroadcast
            outputfile << 4*((long_channel? cores[i]->exu->bypass.power.readOp.longer_channel_leakage:cores[i]->exu->bypass.power.readOp.leakage )+cores[i]->exu->bypass.power.readOp.gate_leakage+cores[i]->exu->bypass.rt_power.readOp.dynamic/cores[i]->exu->executionTime) << ' ';
            //itb
            outputfile << 4*((long_channel? cores[i]->mmu->itlb->power.readOp.longer_channel_leakage:cores[i]->mmu->itlb->power.readOp.leakage)+cores[i]->mmu->itlb->power.readOp.gate_leakage+cores[i]->mmu->itlb->rt_power.readOp.dynamic/cores[i]->mmu->executionTime) << ' ';
            //intexec1
            outputfile << 4*0.17241*((long_channel? cores[i]->exu->exeu->power.readOp.longer_channel_leakage:cores[i]->exu->exeu->power.readOp.leakage)+cores[i]->exu->exeu->power.readOp.gate_leakage+cores[i]->exu->exeu->rt_power.readOp.dynamic/cores[i]->exu->exeu->executionTime) << ' ';
            //intexec2
            outputfile << 4*0.32759*((long_channel? cores[i]->exu->exeu->power.readOp.longer_channel_leakage:cores[i]->exu->exeu->power.readOp.leakage)+cores[i]->exu->exeu->power.readOp.gate_leakage+cores[i]->exu->exeu->rt_power.readOp.dynamic/cores[i]->exu->exeu->executionTime) << ' ';
            //intexec3
            outputfile << 4*0.32759*((long_channel? cores[i]->exu->exeu->power.readOp.longer_channel_leakage:cores[i]->exu->exeu->power.readOp.leakage)+cores[i]->exu->exeu->power.readOp.gate_leakage+cores[i]->exu->exeu->rt_power.readOp.dynamic/cores[i]->exu->exeu->executionTime) << ' ';
            //intexec4
            outputfile << 4*0.17241*((long_channel? cores[i]->exu->exeu->power.readOp.longer_channel_leakage:cores[i]->exu->exeu->power.readOp.leakage)+cores[i]->exu->exeu->power.readOp.gate_leakage+cores[i]->exu->exeu->rt_power.readOp.dynamic/cores[i]->exu->exeu->executionTime) << ' ';
            //regfile1
            outputfile << 4*0.5*((long_channel? cores[i]->exu->rfu->IRF->power.readOp.longer_channel_leakage:cores[i]->exu->rfu->IRF->power.readOp.leakage) + cores[i]->exu->rfu->IRF->power.readOp.gate_leakage+cores[i]->exu->rfu->IRF->rt_power.readOp.dynamic/cores[i]->exu->executionTime) << ' ';
            //regfile2
            outputfile << 4*0.5*((long_channel? cores[i]->exu->rfu->IRF->power.readOp.longer_channel_leakage:cores[i]->exu->rfu->IRF->power.readOp.leakage) + cores[i]->exu->rfu->IRF->power.readOp.gate_leakage+cores[i]->exu->rfu->IRF->rt_power.readOp.dynamic/cores[i]->exu->executionTime) << ' ';
            //intmap
            outputfile << 4*0.24306*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //intq1
            outputfile << 4*0.064816*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //intq2
            outputfile << 4*0.086692*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //intq3
            outputfile << 4*0.16204*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //l2int
            outputfile << 4*((long_channel? cores[i]->lsu->LSQ->power.readOp.longer_channel_leakage:cores[i]->lsu->LSQ->power.readOp.leakage)+cores[i]->lsu->LSQ->power.readOp.gate_leakage+cores[i]->lsu->LSQ->rt_power.readOp.dynamic/cores[i]->lsu->executionTime) << ' ';
            //ldst1
            outputfile << 4*0.5055*((long_channel? cores[i]->lsu->power.readOp.longer_channel_leakage:cores[i]->lsu->power.readOp.leakage )+cores[i]->lsu->power.readOp.gate_leakage+cores[i]->lsu->rt_power.readOp.dynamic/cores[i]->executionTime) << ' ';
            //ldst2
            outputfile << 4*0.4945*((long_channel? cores[i]->lsu->power.readOp.longer_channel_leakage:cores[i]->lsu->power.readOp.leakage )+cores[i]->lsu->power.readOp.gate_leakage+cores[i]->lsu->rt_power.readOp.dynamic/cores[i]->executionTime) << ' ';
            //fpmap
            outputfile << 4*0.2279*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //fpq
            outputfile << 4*0.2155*((long_channel? cores[i]->ifu->IB->power.readOp.longer_channel_leakage:cores[i]->ifu->IB->power.readOp.leakage)+cores[i]->ifu->IB->power.readOp.gate_leakage+cores[i]->ifu->IB->rt_power.readOp.dynamic/cores[i]->ifu->executionTime) << ' ';
            //fpmul
            outputfile << 4*0.6113*((long_channel? cores[i]->exu->mul->power.readOp.longer_channel_leakage:cores[i]->exu->mul->power.readOp.leakage) + cores[i]->exu->mul->power.readOp.gate_leakage+cores[i]->exu->mul->rt_power.readOp.dynamic/cores[i]->exu->mul->executionTime) << ' ';
            //fpreg
            outputfile << 4*((long_channel? cores[i]->exu->rfu->FRF->power.readOp.longer_channel_leakage:cores[i]->exu->rfu->FRF->power.readOp.leakage) + cores[i]->exu->rfu->FRF->power.readOp.gate_leakage+cores[i]->exu->rfu->FRF->rt_power.readOp.dynamic/cores[i]->exu->rfu->executionTime) << ' ';
            //fpadd
            outputfile << 4*0.3887*((long_channel? cores[i]->exu->fp_u->power.readOp.longer_channel_leakage:cores[i]->exu->fp_u->power.readOp.leakage) + cores[i]->exu->fp_u->power.readOp.gate_leakage+cores[i]->exu->fp_u->rt_power.readOp.dynamic/cores[i]->exu->fp_u->executionTime) << ' ';
            //fpdiv
            outputfile << 4*0.3887*((long_channel? cores[i]->exu->mul->power.readOp.longer_channel_leakage:cores[i]->exu->mul->power.readOp.leakage) + cores[i]->exu->mul->power.readOp.gate_leakage+cores[i]->exu->mul->rt_power.readOp.dynamic/cores[i]->exu->mul->executionTime) << ' ';
            
        }   
    }
	if (is_tdp)
	{

		if (plevel<5)
		{
			cout<<"\nMcPAT (version "<< VER_MAJOR <<"."<< VER_MINOR
					<< " of " << VER_UPDATE << ") results (current print level is "<< plevel
			<<", please increase print level to see the details in components): "<<endl;
		}
		else
		{
			cout<<"\nMcPAT (version "<< VER_MAJOR <<"."<< VER_MINOR
								<< " of " << VER_UPDATE << ") results  (current print level is 5)"<< endl;
		}
		cout <<"*****************************************************************************************"<<endl;
		cout <<indent_str<<"Technology "<<XML->sys.core_tech_node<<" nm"<<endl;
		//cout <<indent_str<<"Device Type= "<<XML->sys.device_type<<endl;
		if (long_channel)
			cout <<indent_str<<"Using Long Channel Devices When Appropriate"<<endl;
		//cout <<indent_str<<"Interconnect metal projection= "<<XML->sys.interconnect_projection_type<<endl;
		displayInterconnectType(XML->sys.interconnect_projection_type, indent);
		cout <<indent_str<<"Core clock Rate(MHz) "<<XML->sys.core[0].clock_rate<<endl;
    	cout <<endl;
		cout <<"*****************************************************************************************"<<endl;
		cout <<"Processor: "<<endl;
		cout << indent_str << "Area = " << area.get_area()*1e-6<< " mm^2" << endl;
		cout << indent_str << "Peak Power = " << power.readOp.dynamic +
			(long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage <<" W" << endl;
		cout << indent_str << "Total Leakage = " <<
			(long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage <<" W" << endl;
		cout << indent_str << "Peak Dynamic = " << power.readOp.dynamic << " W" << endl;
		cout << indent_str << "Subthreshold Leakage = " << (long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) <<" W" << endl;
		//cout << indent_str << "Subthreshold Leakage = " << power.readOp.longer_channel_leakage <<" W" << endl;
		cout << indent_str << "Gate Leakage = " << power.readOp.gate_leakage << " W" << endl;
		cout << indent_str << "Runtime Dynamic = " << rt_power.readOp.dynamic << " W" << endl;
		cout <<endl;
		if (numCore >0){
		cout <<indent_str<<"Total Cores: "<<XML->sys.number_of_cores << " cores "<<endl;
		displayDeviceType(XML->sys.device_type,indent);
		cout << indent_str_next << "Area = " << core.area.get_area()*1e-6<< " mm^2" << endl;
		cout << indent_str_next << "Peak Dynamic = " << core.power.readOp.dynamic << " W" << endl;
		cout << indent_str_next << "Subthreshold Leakage = "
			<< (long_channel? core.power.readOp.longer_channel_leakage:core.power.readOp.leakage) <<" W" << endl;
		//cout << indent_str_next << "Subthreshold Leakage = " << core.power.readOp.longer_channel_leakage <<" W" << endl;
		cout << indent_str_next << "Gate Leakage = " << core.power.readOp.gate_leakage << " W" << endl;
		cout << indent_str_next << "Runtime Dynamic = " << core.rt_power.readOp.dynamic << " W" << endl;
		cout <<endl;
		}
		if (!XML->sys.Private_L2)
		{
			if (numL2 >0){
				cout <<indent_str<<"Total L2s: "<<endl;
				displayDeviceType(XML->sys.L2[0].device_type,indent);
				cout << indent_str_next << "Area = " << l2.area.get_area()*1e-6<< " mm^2" << endl;
				cout << indent_str_next << "Peak Dynamic = " << l2.power.readOp.dynamic << " W" << endl;
				cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? l2.power.readOp.longer_channel_leakage:l2.power.readOp.leakage) <<" W" << endl;
				//cout << indent_str_next << "Subthreshold Leakage = " << l2.power.readOp.longer_channel_leakage <<" W" << endl;
				cout << indent_str_next << "Gate Leakage = " << l2.power.readOp.gate_leakage << " W" << endl;
				cout << indent_str_next << "Runtime Dynamic = " << l2.rt_power.readOp.dynamic << " W" << endl;
				cout <<endl;
			}
		}
		if (numL3 >0){
			cout <<indent_str<<"Total L3s: "<<endl;
			displayDeviceType(XML->sys.L3[0].device_type, indent);
			cout << indent_str_next << "Area = " << l3.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << l3.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? l3.power.readOp.longer_channel_leakage:l3.power.readOp.leakage) <<" W" << endl;
			//cout << indent_str_next << "Subthreshold Leakage = " << l3.power.readOp.longer_channel_leakage <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << l3.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << l3.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (numL1Dir >0){
			cout <<indent_str<<"Total First Level Directory: "<<endl;
			displayDeviceType(XML->sys.L1Directory[0].device_type, indent);
			cout << indent_str_next << "Area = " << l1dir.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << l1dir.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? l1dir.power.readOp.longer_channel_leakage:l1dir.power.readOp.leakage) <<" W" << endl;
			//cout << indent_str_next << "Subthreshold Leakage = " << l1dir.power.readOp.longer_channel_leakage <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << l1dir.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << l1dir.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (numL2Dir >0){
			cout <<indent_str<<"Total First Level Directory: "<<endl;
			displayDeviceType(XML->sys.L1Directory[0].device_type, indent);
			cout << indent_str_next << "Area = " << l2dir.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << l2dir.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? l2dir.power.readOp.longer_channel_leakage:l2dir.power.readOp.leakage) <<" W" << endl;
			//cout << indent_str_next << "Subthreshold Leakage = " << l2dir.power.readOp.longer_channel_leakage <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << l2dir.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << l2dir.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (numNOC >0){
			cout <<indent_str<<"Total NoCs (Network/Bus): "<<endl;
			displayDeviceType(XML->sys.device_type, indent);
			cout << indent_str_next << "Area = " << noc.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << noc.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? noc.power.readOp.longer_channel_leakage:noc.power.readOp.leakage) <<" W" << endl;
			//cout << indent_str_next << "Subthreshold Leakage = " << noc.power.readOp.longer_channel_leakage  <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << noc.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << noc.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
		{
			cout <<indent_str<<"Total MCs: "<<XML->sys.mc.number_mcs << " Memory Controllers "<<endl;
			displayDeviceType(XML->sys.device_type, indent);
			cout << indent_str_next << "Area = " << mcs.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << mcs.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? mcs.power.readOp.longer_channel_leakage:mcs.power.readOp.leakage)  <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << mcs.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << mcs.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (XML->sys.flashc.number_mcs >0)
		{
			cout <<indent_str<<"Total Flash/SSD Controllers: "<<flashcontroller->fcp.num_mcs << " Flash/SSD Controllers "<<endl;
			displayDeviceType(XML->sys.device_type, indent);
			cout << indent_str_next << "Area = " << flashcontrollers.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << flashcontrollers.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? flashcontrollers.power.readOp.longer_channel_leakage:flashcontrollers.power.readOp.leakage)  <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << flashcontrollers.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << flashcontrollers.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (XML->sys.niu.number_units >0 )
		{
			cout <<indent_str<<"Total NIUs: "<<niu->niup.num_units << " Network Interface Units "<<endl;
			displayDeviceType(XML->sys.device_type, indent);
			cout << indent_str_next << "Area = " << nius.area.get_area()*1e-6<< " mm^2" << endl;
			cout << indent_str_next << "Peak Dynamic = " << nius.power.readOp.dynamic << " W" << endl;
			cout << indent_str_next << "Subthreshold Leakage = "
				<< (long_channel? nius.power.readOp.longer_channel_leakage:nius.power.readOp.leakage)  <<" W" << endl;
			cout << indent_str_next << "Gate Leakage = " << nius.power.readOp.gate_leakage << " W" << endl;
			cout << indent_str_next << "Runtime Dynamic = " << nius.rt_power.readOp.dynamic << " W" << endl;
			cout <<endl;
		}
		if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels>0)
				{
					cout <<indent_str<<"Total PCIes: "<<pcie->pciep.num_units << " PCIe Controllers "<<endl;
					displayDeviceType(XML->sys.device_type, indent);
					cout << indent_str_next << "Area = " << pcies.area.get_area()*1e-6<< " mm^2" << endl;
					cout << indent_str_next << "Peak Dynamic = " << pcies.power.readOp.dynamic << " W" << endl;
					cout << indent_str_next << "Subthreshold Leakage = "
						<< (long_channel? pcies.power.readOp.longer_channel_leakage:pcies.power.readOp.leakage)  <<" W" << endl;
					cout << indent_str_next << "Gate Leakage = " << pcies.power.readOp.gate_leakage << " W" << endl;
					cout << indent_str_next << "Runtime Dynamic = " << pcies.rt_power.readOp.dynamic << " W" << endl;
					cout <<endl;
				}
		cout <<"*****************************************************************************************"<<endl;
		if (plevel >1)
		{
			for (i = 0;i < numCore; i++)
			{
				cores[i]->displayEnergy(indent+4,plevel,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			if (!XML->sys.Private_L2)
			{
				for (i = 0;i < numL2; i++)
				{
					l2array[i]->displayEnergy(indent+4,is_tdp);
					cout <<"*****************************************************************************************"<<endl;
				}
			}
			for (i = 0;i < numL3; i++)
			{
				l3array[i]->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			for (i = 0;i < numL1Dir; i++)
			{
				l1dirarray[i]->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			for (i = 0;i < numL2Dir; i++)
			{
				l2dirarray[i]->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
			{
				mc->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			if (XML->sys.flashc.number_mcs >0 && XML->sys.flashc.memory_channels_per_mc>0)
			{
				flashcontroller->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			if (XML->sys.niu.number_units >0 )
			{
				niu->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
			if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels>0)
			{
				pcie->displayEnergy(indent+4,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}

			for (i = 0;i < numNOC; i++)
			{
				nocs[i]->displayEnergy(indent+4,plevel,is_tdp);
				cout <<"*****************************************************************************************"<<endl;
			}
		}
	}
	else
	{

	}

}

void Processor::set_proc_param()
{
	bool debug = false;

	procdynp.homoCore = bool(debug?1:XML->sys.homogeneous_cores);
	procdynp.homoL2   = bool(debug?1:XML->sys.homogeneous_L2s);
	procdynp.homoL3   = bool(debug?1:XML->sys.homogeneous_L3s);
	procdynp.homoNOC  = bool(debug?1:XML->sys.homogeneous_NoCs);
	procdynp.homoL1Dir  = bool(debug?1:XML->sys.homogeneous_L1Directories);
	procdynp.homoL2Dir  = bool(debug?1:XML->sys.homogeneous_L2Directories);

	procdynp.numCore = XML->sys.number_of_cores;
	procdynp.numL2   = XML->sys.number_of_L2s;
	procdynp.numL3   = XML->sys.number_of_L3s;
	procdynp.numNOC  = XML->sys.number_of_NoCs;
	procdynp.numL1Dir  = XML->sys.number_of_L1Directories;
	procdynp.numL2Dir  = XML->sys.number_of_L2Directories;
	procdynp.numMC = XML->sys.mc.number_mcs;
	procdynp.numMCChannel = XML->sys.mc.memory_channels_per_mc;

//	if (procdynp.numCore<1)
//	{
//		cout<<" The target processor should at least have one core on chip." <<endl;
//		exit(0);
//	}

	//  if (numNOCs<0 || numNOCs>2)
	//    {
	//  	  cout <<"number of NOCs must be 1 (only global NOCs) or 2 (both global and local NOCs)"<<endl;
	//  	  exit(0);
	//    }

	/* Basic parameters*/
	interface_ip.data_arr_ram_cell_tech_type    = debug?0:XML->sys.device_type;
	interface_ip.data_arr_peri_global_tech_type = debug?0:XML->sys.device_type;
	interface_ip.tag_arr_ram_cell_tech_type     = debug?0:XML->sys.device_type;
	interface_ip.tag_arr_peri_global_tech_type  = debug?0:XML->sys.device_type;

	interface_ip.ic_proj_type     = debug?0:XML->sys.interconnect_projection_type;
	interface_ip.delay_wt                = 100;//Fixed number, make sure timing can be satisfied.
	interface_ip.area_wt                 = 0;//Fixed number, This is used to exhaustive search for individual components.
	interface_ip.dynamic_power_wt        = 100;//Fixed number, This is used to exhaustive search for individual components.
	interface_ip.leakage_power_wt        = 0;
	interface_ip.cycle_time_wt           = 0;

	interface_ip.delay_dev                = 10000;//Fixed number, make sure timing can be satisfied.
	interface_ip.area_dev                 = 10000;//Fixed number, This is used to exhaustive search for individual components.
	interface_ip.dynamic_power_dev        = 10000;//Fixed number, This is used to exhaustive search for individual components.
	interface_ip.leakage_power_dev        = 10000;
	interface_ip.cycle_time_dev           = 10000;

	interface_ip.ed                       = 2;
	interface_ip.burst_len      = 1;//parameters are fixed for processor section, since memory is processed separately
	interface_ip.int_prefetch_w = 1;
	interface_ip.page_sz_bits   = 0;
	interface_ip.temp = debug?360: XML->sys.temperature;
	interface_ip.F_sz_nm         = debug?90:XML->sys.core_tech_node;//XML->sys.core_tech_node;
	interface_ip.F_sz_um         = interface_ip.F_sz_nm / 1000;

	//***********This section of code does not have real meaning, they are just to ensure all data will have initial value to prevent errors.
	//They will be overridden  during each components initialization
	interface_ip.cache_sz            =64;
	interface_ip.line_sz             = 1;
	interface_ip.assoc               = 1;
	interface_ip.nbanks              = 1;
	interface_ip.out_w               = interface_ip.line_sz*8;
	interface_ip.specific_tag        = 1;
	interface_ip.tag_w               = 64;
	interface_ip.access_mode         = 2;

	interface_ip.obj_func_dyn_energy = 0;
	interface_ip.obj_func_dyn_power  = 0;
	interface_ip.obj_func_leak_power = 0;
	interface_ip.obj_func_cycle_t    = 1;

	interface_ip.is_main_mem     = false;
	interface_ip.rpters_in_htree = true ;
	interface_ip.ver_htree_wires_over_array = 0;
	interface_ip.broadcast_addr_din_over_ver_htrees = 0;

	interface_ip.num_rw_ports        = 1;
	interface_ip.num_rd_ports        = 0;
	interface_ip.num_wr_ports        = 0;
	interface_ip.num_se_rd_ports     = 0;
	interface_ip.num_search_ports    = 1;
	interface_ip.nuca                = 0;
	interface_ip.nuca_bank_count     = 0;
	interface_ip.is_cache            =true;
	interface_ip.pure_ram            =false;
	interface_ip.pure_cam            =false;
	interface_ip.force_cache_config  =false;
	if (XML->sys.Embedded)
		{
		interface_ip.wt                  =Global_30;
		interface_ip.wire_is_mat_type = 0;
		interface_ip.wire_os_mat_type = 0;
		}
	else
		{
		interface_ip.wt                  =Global;
		interface_ip.wire_is_mat_type = 2;
		interface_ip.wire_os_mat_type = 2;
		}
	interface_ip.force_wiretype      = false;
	interface_ip.print_detail        = 1;
	interface_ip.add_ecc_b_          =true;
}

Processor::~Processor(){
	while (!cores.empty())
	{
		delete cores.back();
		cores.pop_back();
	}
	while (!l2array.empty())
	{
		delete l2array.back();
		l2array.pop_back();
	}
	while (!l3array.empty())
	{
		delete l3array.back();
		l3array.pop_back();
	}
	while (!nocs.empty())
	{
		delete nocs.back();
		nocs.pop_back();
	}
	if (!mc)
	{
		delete mc;
	}
	if (!niu)
	{
		delete niu;
	}
	if (!pcie)
	{
		delete pcie;
	}
	if (!flashcontroller)
	{
		delete flashcontroller;
	}
};