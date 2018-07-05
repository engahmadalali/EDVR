/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1997 Regents of the University of Cahmadfornia.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Ported from CMU/Monarch's code, nov'98 -Padma.
 *
 * $Header: /cvsroot/nsnam/ns-2/DSDV/DSDV.cc,v 1.26 2006/02/21 15:20:18 mahrenho Exp $
 */
//================================================== EDVR ==============================
//                                     Last ahmad 
//ÚäÏãÇ ÇáÚŞÏÉ áåÇ ãÓÇÑÇÊ ÊÖãäåã ãä ÎáÇá äÔÑ ÑÒãÉ ÊÍÏíË ÈÑÒã ÑÒãÉ ÇáÊÑÍíÈ ÇáÏæÑíÉ æÅÑÓÇáåã ÈÑÒãÉ ÊÍÏíË æÇÍÏÉ ÈÏáÇõ ãä ÑÒãÉ ÇáÊÑÍíÈ¡ ßãÇ Ãä ÑÒãÉ ÇáÊÍÏíË ÊÊÖãä ÇáãÓÇÑÇÊ ÇáÛíÑ ŞÇÈáÉ ááæÕæá áÅÚáÇã ÇáÚŞÏ ÇáãÌÇæÑÉ ÈåÇ  
// Êã ÅÖÇİÉ ÑŞã ááÑÒãÉ  ááÊÍßã ÈÑÒãÉ ØáÈ ÇáãÓÇÑßí áÇ ÊãÑ äİÓ ÑÒãÉ ØáÈ ÇáãÓÇÑ Úáì ÚŞÏÉ ãÑÊ ÚáíåÇ ãÓÈŞÇğ¡ ßãÇ Êã ÇÓÊÎÏÇã ÚÊÈÉ ØáÈ ÇáãÓÇÑ áÊÎİíÖ ÑÒã ÇáÊÍßã æÊÍÏíÏ ÚÏÏ ÇáŞİÒÇÊ ÇáÊí íãßä Ãä ÊãÑ ÚáíåÇ ÑÒãÉ ØáÈ ÇáãÓÇÑ
//Òãä ÑÒãÉ ÇáÊÑÍíÈ åæ 3 ËÇäíÉ æÚÏÏ ÇáÑÒã ÇáÇÚÙãí Öãä ÇáÑÊá åæ 5 ÑÒãÉ   //ahmad alali
//============================================================================================
extern "C" {
#include <stdarg.h>
#include <float.h>
}
#include "edvr.h"
#include "priqueue.h"										   
#include <random.h>
//#include <iostream.h> //added by ahmad alali
//#include <stdio.h>

#include <cmu-trace.h>
#include <address.h>
#include <mobilenode.h>
#define EDVR_STARTUP_JITTER 2.0	// secs to jitter start of periodic activity from
									// when start-dsr msg sent to agent
#define EDVR_ALMOST_NOW     0.1 // jitter used for events that should be effectively
								  // instantaneous but are jittered to prevent synchronization
#define EDVR_BROADCAST_JITTER 0.01 // jitter for all broadcast packets   0.01
#define EDVR_MIN_TUP_PERIOD 1.0 // minimum time between triggered updates	 1.0//ÇáÒãä ÇáİÇÕá Èíä ÇáÊÍÏíËÇÊ ÇáÏæÑíÉ
#define IP_DEF_TTL   32 // Òãä ÍíÇÉ ÇáÑÒãÉ
#define RREQ_Threshold 2    // ÚÊÈÉ ØáÈ ÇáãÓÇÑ            // ahmad
#define RREQ_TimeOut 1.6	// Òãä ÇäÊåÇÁ ØáÈ ÇáãÓÇÑ	        // ahmad 
#undef TRIGGER_UPDATE_ON_FRESH_SEQNUM

// Returns a random number between 0 and max
static inline double 
jitter (double max, int be_random_)//max íÚíÏ ŞíãÉ ÚÔæÇÆíÉ Èíä 0 æ  
{
  return (be_random_ ? Random::uniform(max) : 0);
}
void EDVR_Agent::
trace (char *fmt,...)
{

  va_list ap;

  if (!tracetarget)
    return;

  va_start (ap, fmt);
  vsprintf (tracetarget->pt_->buffer (), fmt, ap);
  tracetarget->pt_->dump ();
  va_end (ap);

}

void 
EDVR_Agent::tracepkt (Packet * p, double now, int me, const char *type)
{
  char buf[1024];
  unsigned char *walk = p->accessdata ();


  int ct = *(walk++);
  int seq, dst, met;

  snprintf (buf, 1024, "V%s %.5f _%d_ [%d]:", type, now, me, ct);
  while (ct--)
    {
      dst = *(walk++);
	  dst = dst << 8 | *(walk++);
	  dst = dst << 8 | *(walk++);
	  dst = dst << 8 | *(walk++);
	  met = *(walk++);
      seq = *(walk++);

      seq = seq << 8 | *(walk++);
      seq = seq << 8 | *(walk++);
      seq = seq << 8 | *(walk++);
      snprintf (buf, 1024, "%s (%d,%d,%d)", buf, dst, met, seq);
    }
  // Now do trigger handling.
  //trace("VTU %.5f %d", now, me);
  if (verbose_)
  {
      trace ("%s", buf);
  }
}
// ØÈÇÚÉ ÌÏæá ÇáÊæÌíå
void
EDVR_Agent::Print_Routing_Table(const char *prefix, edvr_rtable_ent * prte, EDVR_Agent * a)
{
  a->trace("DFU: deimplemented");
  printf("DFU: deimplemented");
  prte = 0;
  prefix = 0;
#if 0
  printf ("%s%d %d %d %d %f %f %f %f 0x%08x\n",
	  prefix, prte->dst, prte->f_nxt_hop , prte->metric, prte->seqnum,
	  prte->udtime, prte->new_seqnum_at, prte->wst, prte->changed_at,
	  (unsigned int) prte->timeout_event);
  a->trace ("VTE %.5f %d %d %d %d %f %f %f %f 0x%08x",
          Scheduler::instance ().clock (), prte->dst, prte->f_nxt_hop , prte->metric,
	  prte->seqnum, prte->udtime, prte->new_seqnum_at, prte->wst, prte->changed_at,
	    prte->timeout_event);
      ////cout<<" trace now ="<< Scheduler::instance ().clock ();   // added ahmad
	  //****cin.get();
#endif
}
void 
EDVR_Agent::New_Entry_Add(nsaddr_t dst)    //ÅÖÇİÉ ãÓÇÑ áÌÇÑ ÌÏíÏ  ãÚ ÌÏæáÉ Òãä ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ       
{				 // by ahmad 
	    edvr_rtable_ent rte;
		bzero(&rte, sizeof(rte));
       
        Scheduler & s = Scheduler::instance ();
	    double now = s.clock ();
		rte.dst = dst;
		//rte.f_nxt_hop  = hdri->src();
		rte.f_nxt_hop  = dst;
		rte.s_nxt_hop  = -99;  //added by ahmad alali
		rte.metric = 1;
		rte.packet_id =0;	 // áÇ íæÌÏ ÑŞã ÑÒãÉ ŞíãÉ ÈÏÇÆíÉ ÕİÑ 
		rte.seqnum = 0;
		rte.link_no = myaddr_*10000+dst;  // added by ahmad alali

		rte.advertise_ok_at = 0.0; // can always advert ourselves
        rte.advert_seqnum = false;	 //ahmad
		rte.advert_metric = false;	 //ahmad
		rte.advertise = true;  // added    ÇÑÛÈ ÈäÔÑ åĞÇ ÇáãÓÇÑ
		rte.changed_at = now;  
		rte.new_seqnum_at = now;    // canceled 
		rte.wst = 0;
		rte.advertise = true;  // added 

		rte.timeout_event = new Event ();										//   added by ahmad alali ...   Çäå ÌÇÑ ÌÏíÏ ÇÓÊŞÈá áÃæá ãÑÉ  
		s.schedule (helper_, rte.timeout_event, min_update_periods_ * perup_);// ÌÏæáÉ ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ

		rte.q = 0;		// áÇ ÊÎÒä ÇáÑÒã ááÚŞÏÉ äİÓåÇ

		nsaddr_t *ahmad = &myaddr_;   // added by ahmad 
		  
		////cout<< "\n Node (" << myaddr_ << ") considers node (" << dst << ") as a new neighbour. So, it adds the following entry as a route:  " << rte.dst << "  " << rte.f_nxt_hop  << "  " << rte.s_nxt_hop  << "  " << rte.metric << "  " << rte.link_no <<"\n";   // added by ahmad 

		table_->AddEntry (rte,ahmad);     // added by ahmad 
		
		return;
}

//íÊã ÇÓÊÏÚÇÁ åĞÇ ÇáÊÇÈÚ ÚäÏ ÇÓÊŞÈÇá ÍÏË æåĞÇ ÇáÍÏË ÑÈãÇ íßæä Òãä ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ Ãæ ÇäŞØÇÚ ãÓÇÑ
//
void
EDVR_Agent::helper_callback (Event * e)	   //äÓÊÏÚí åĞÇ ÇáÊÇÈÚ ÚäÏ ÍÏæË ÍÏËíä
{										       // 1-  Òãä ÑÒãÉ ÇáÊÑÍíÈ 
	                                           // 2-  ÚäÏ ÇäŞØÇÚ æÕáÉ
	//************************************************ÈÏÃ ÚãáíÉ ÇÓÊŞÈÇá ÍÏË ************************************************************
	Scheduler & s = Scheduler::instance ();	    
	double now = s.clock ();
	edvr_rtable_ent *prte;
	edvr_rtable_ent *pr2;
	int update_type;	 // we want periodic (=1) or triggered (=0) update?
	Packet *p;
	int num_of_advertise_entry = 0;
	int immediatly_update=0; //ahmad  

	
    //ÅĞÇ ßÇä ÇáÍÏË Òãä ÑÒãÉ ÊÑÍíÈ
	// 1-    äİÍÕ Ãä ÇáÍÏË åæ ÇÓÊŞÈÇá ÑÒãÉ ÊÑÍíÈ 
	if (periodic_Hello_ && e == periodic_Hello_)
	{
		update_type = 1;
		//cout << "\n\n\n\n\n\n*******************************************  helper_callback (Event * e) function is invoked  ***********************************************";
		//cout << "\n*******************       At  " << now << "  Node  (" << myaddr_ << ")  calls Make_Hello_Packet function to broadcast a Hello packet ........     ******************";	
		////table_->Show_rt();//  ááÅÙåÇÑ ÌÏæá ÇáÊæÌíå ááÚŞÏÉ ÇáÍÇáíÉ

		for (table_->InitLoop (); (prte = table_->NextLoop ()); )
		{
			
			  if (prte->advertise /*&& prte->metric > 0*/)                //ÊÖãíä ÌãíÚ ÇáãÓÇÑÇÊ ÇáÊí íÌÈ äÔÑåÇ
                 num_of_advertise_entry++;

		}

		 if (num_of_advertise_entry > 0)   //ÅäÔÇÁ ÑÒãÉ ÊÍÏíË áäÔÑåÇ
		
			p = Make_Update_Packet(num_of_advertise_entry);	// ÇäÔÇÁ ÑÒãÉ ÇáÊÍÏíË
		 else   //äÔÑ ÑÒãÉ ÊÑÍíÈ
            p = Make_Hello_Packet (/*in-out*//*update_type*/);
		 if (verbose_)
		{   // åĞíä ÇáÓØÑíä // (file.tr)  åĞÇ áíÓ ááßÊÇÈÉ İí Çáãáİ  
			trace ("VPC %.5f _%d_", now, myaddr_);
			tracepkt (p, now, myaddr_, "PU");
		}

		if (p) 
		{    // Óæİ ÇäÔÑ ÇáÑÒãÉ
			 sendOutBCastPkt1(p);//äÔÑ ÇáÑÒãÉ
	      //assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
	      // send out Route Update Packet jitter to avoid sync
	      //DEBUG
	      //printf("(%d)..sendout update pkt (periodic=%d)\n",myaddr_,update_type);
	      //s.schedule (target_, p, jitter(EDVR_BROADCAST_JITTER, be_random_));
        }
      
      // put the periodic update sending callback back onto the 
      // the scheduler queue for next time....
      //s.schedule (helper_, periodic_Hello_,perup_ * (0.75 + jitter (0.25, be_random_)));

      // this will take the place of any planned triggered updates
      lasttup_ = now;
      return;
    } 
	
	// 3-    İÍÕ Ãä ÇáÍÏË ÇäŞØÇÚ ãÓÇÑ
	//cout << "\n\n\n\n *****************************************    helper_callback (Event * e) function is invoked    ***********************************************";
    // ÇáÈÍË Öãä ÌÏæá ÇáÊæÌíå ÅĞÇ ßÇä íæÌÏ ãÓÇÑ áå äİÓ ÍÏË ÇáÇäÊåÇÁ¡ íÊã ÇáÅÔÇÑÉ áåĞÇ ÇáãÓÇÑ Ãäå ÛíÑ ŞÇÈá ááæÕæá
	//  íÊã æÖÚ  ŞíãÉ ÚÏÏ ÇáŞİÒÇÊ áÇ äåÇíÉ 
	////cout<<" \n I'm looking for this event "<<e;
	//table_->Show_rt();  //
	// ÇáÈÍË Öãä ÌÏæá ÇáÊæÌíå Úä ãÓÇÑ áå äİÓ ÍÏË ÇäÊåÇÁ ÇáãÓÇÑ 
	for (table_->InitLoop (); (prte = table_->NextLoop ());)
	    if (prte->timeout_event && (prte->timeout_event == e))
		{
		  immediatly_update=1;	// ãÓÇÑ ãŞØæÚ¡ áÏíäÇ ãÚáæãÇÊ ÊæÌíå áäÔÑåÇ ãÈÇÔÑÉ áæÌæÏ ÎØÃ
		                        // (ÊÍá ÑÒãÉ ÇáÊÍÏíË ãßÇä ÑÓÇáÉ ÇáÎØÃ ÈåĞå ÇáÍÇáÉ 	   // added by ahmad  
		                        //æÌÏäÇ ÇáãÓÇÑ ÇáãŞØæÚ ÇáĞí äÈÍË Úäå æÈÇáÊÇáí  íÌÈ äÔÑ ÑÒãÉ ÊÍÏíË
		  break;				// äÚã åĞÇ åæ ÇáãÓÇÑ ÇáãŞØæÚ ÇáĞí ßäÇ äÈÍË Úäå
		}

	//null ÇáãÓÇÑ ãæÌæÏ æáíÓ  timeout  Ç1Ç ßÇä 
	if (prte)
	{
		if (verbose_)
		{  
			trace ("VTO %.5f _%d_ %d->%d", now, myaddr_, myaddr_, prte->dst);
		}
      
		for (table_->InitLoop (); (pr2 = table_->NextLoop ()); )
		{
			if (pr2->f_nxt_hop  == prte->dst && pr2->metric !=BIG)	 // ÊãÑ ÇáÚŞÏÉ Úáì ÌÏæá ÊæÌíååÇ ááÅÔÇÑÉ áÃí ãÓÇÑ íÓÊÎÏã ÇáŞİÒÉ ÇáÃæáì äİÓ ÇáåÏİ ááãÓÇÑ ÇáãŞØæÚ Ãäå ãŞØæÚ æÛíÑ ŞÇÈá ááæÕæá 
			{												//	
				if (verbose_)
				{
				   trace ("VTO %.5f _%d_ marking %d", now, myaddr_, pr2->dst);
				}
				pr2->metric = BIG;
				pr2->changed_at = now;
				if(pr2->dst == prte->dst  && pr2->f_nxt_hop  == prte->dst)		//ãÓÇÑ ãÈÇÔÑ
				   pr2->advertise = true;	// İŞØ ÇáãÓÇÑÇÊ ÇáãÈÇÔÑÉ íÌÈ ÇáÃÔÇÑÉ áåÇ ÃäåÇ ãŞØæÚÉ		       
				else
				   pr2->advertise = false;	// Çäå ãÓÇÑ ãŞØæÚ æáßä áÇ íæÌÏ ÏÇÚí áäÔÑå
				
			}
		}

        //Óæİ äÍÑÑå  timeout   ÇäÊåì 
        prte->timeout_event = 0;    
	}
	else 

	if (e)
	 delete e;
	////table_->Show_rt();//ÚÑÖ ÌÏæá ÇáÊæÌíå
		
	if (immediatly_update==1)	  // Êã ÊÚÏíá ÌÏæá ÇáÊæÌíå æÈÇáÊÇáí íÌÈ äÔÑ ÑÒãÉ ÊÍÏíË
	    { 
		  ////cout<< "\n I've to broadcase an immediatly update,  bcz I've broken route to my neighbour ";
		  //ÍÓÇÈ ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí íÌÈ äÔÑåÇ
		  for (table_->InitLoop (); (prte = table_->NextLoop ()); )
		   { 
			  if (prte->advertise /*&& prte->metric >0*/ )	 //ÊÖãíä Ãí ãÓÇÑ íÌÈ äÔÑå
                 num_of_advertise_entry++;

		   }
		  p = Make_Update_Packet(num_of_advertise_entry);	// ÇäÔÇÁ ÑÒãÉ ÊÍÏíË
		  if (p) 
			{    // äÔÑ ÑÒãÉ ÇáÊÍÏíË
				sendOutBCastPkt1(p);
			}
		}	   
//*****************************************************************ÇäÊåÇÁ ÚãáíÉ ÇÓÊŞÈÇá ÍÏË*************************************************
}
void
EDVR_Agent::lost_link (Packet *p)
{
	//*************************************************************************ÈÏÇíÉ ÚãáíÉ İŞÏÇä æÕáÉ***************************************************************
    //ÊÊã åĞå ÇáÚãáíÉ ÎáÇá ÅÑÓÇá ÑÒã ÇáÈíÇäÇÊ ÚäÏãÇ Êßæä ÇáŞİÒÉ ÇáÊÇáíÉ ÛíÑ ŞÇÈáÉ ááæÕæá æĞáß ááÅÔÇÑÉ Ãä ÇáãÓÇÑ ãŞØæÚ æíÌÈ ÅíÌÇÏ ãÓÇÑ ÈÏíá áÅÑÓÇá ÑÒã ÇáÈíÇäÇÊ æíÊã ÅåãÇá ÇáÑÒã İí ÍÇá ÚÏã æÌæÏ ãÓÇÑ
	double now = Scheduler::instance ().clock ();  //added by ahmad alali";

	hdr_ip *hdri = hdr_ip::access(p);   //added by ahmad
	hdr_cmn *hdrc = HDR_CMN (p);

	int src = Address::instance().get_nodeaddr(hdri->saddr());     //added by ahmad
	int dst = Address::instance().get_nodeaddr(hdri->daddr());     //added by ahmad

	int entries4 = 0;
	//cout << " *** In lost_link function...  Routing table of node ( " << myaddr_ << " ) at " << now << " consist of  " << table_->number_of_elts() <<" entries....  \n";
	//table_->Show_rt();  
	////cout << "\n   Number of entries =  " << entries4 ;
	////cout << "  Node ( " << myaddr_ << " ) calls GetEntry(hdrc->next_hop_) function to find a route to node " << hdrc->next_hop_ << "\n";

//ÅíÌÇÏ ãÓÇÑ ááŞİÒÉ ÇáÊÇáíÉ ãä ÇáÚŞÏÉ ÇáÍÇáíÉ
	edvr_rtable_ent *prte = table_->GetEntry1 (hdrc->next_hop_,hdrc->next_hop_,&myaddr_);
	
	if(use_mac_ == 0) 
	{     
		if (prte)				// changed by ahmad 
		{
			if(prte && prte->timeout_event)
	         {//İí ÍÇá æÌæÏ ãÓÇÑ æáßäå ÛíÑ ŞÇÈá ááæÕæá

			    ////cout << "\n Node ( " << myaddr_ << " ) calls helper_callback() function to cancel this nighbour timeout and assign it as a broken route";
              	Scheduler::instance ().cancel (prte->timeout_event);     // ÇáÛÇÁ ÇäÊåÇÁ ÇáÕáÇÍíÉ áåĞÇ ÇáÌÇÑ æÇáÅÔÇÑÉ áãÓÇÑ ãŞØæÚ
			    helper_callback (prte->timeout_event);  //ÇáÅÔÇÑÉ áÇäŞØÇÚ æÕáÉ ãä ÎáÇá ÇÓÊÏÚÇÁ ÚãáíÉ ÇÓÊŞÈÇá ÍÏæË ãä äæÚ ãÓÇÑ ãŞØæÚ       
		      }      
		//else
		//{
			////cout << "\n Node ( " << myaddr_ << " ) trying to find an altarnative route to send this packet...... "<< hdrc->uid();
			//edvr_rtable_ent *prte = table_->GetEntry2(hdrc->next_hop_,-99,&myaddr_);
			 edvr_rtable_ent *prte = table_->GetEntry2(dst,-99,&myaddr_);       //ÇáÍÕæá Úáì ãÓÇÑ ÚÈÑ Çí ÚŞÏÉ áÇÑÓÇá ÇáÑÒãÉ
			if (!prte || prte->metric == BIG)	//ÅĞÇ ßÇä áÇ íæÌÏ ãÓÇÑ Ãæ íæÌÏ æáßäå ãŞØæÚ
			{
               ////cout << "\n Node ( " << myaddr_ << " ) drops the packet number " << hdrc->uid() << " because it didn't find an altarnative route to send this packet...";
			   drop(p, DROP_RTR_MAC_CALLBACK);  //ÇåãÇá ÇáÑÒãÉ áÚÏã æÌæÏ ãÓÇÑ
			   //drop(p, DROP_RTR_NO_ROUTE);		//added
			}
			else
			{//íæÌÏ ãÓÇÑ áÅÑÓÇá ÇáÑÒã áĞÇ íÊã æÖÚ ÇáŞİÒÉ ÇáÊÇáíÉ åí ÚäæÇä ÇáŞİÒÉ ÇáÃæáì ááãÓÇÑ
				////cout << "\n Node ( " << myaddr_ << " ) found the following route to send the data  " << prte->dst << "   " << prte->f_nxt_hop  << "   " << prte->s_nxt_hop  << "   " << prte->metric << "   " << prte->link_no;
				////cout<<" \n old Next f_nxt_hop  is "<<   hdrc->next_hop_;
			    hdrc->next_hop_ = prte->f_nxt_hop ;  //İí ÍÇá æÌæÏ ãÓÇÑ ÊÚÏíá ÇáŞİÒÉ ÇáÊÇáíÉ ááÒãÉ æÇÑÓÇáåÇ æÇÓÊÏÚÇÁ ÊÇÈÚ ÇáÇÓÊŞÈÇá
				////cout<<" \n new Next f_nxt_hop  is "<<   hdrc->next_hop_;
				if (verbose_)
				{
					trace ("Routing pkts outside domain: \
					VFP %.5f _%d_ %d:%d -> %d:%d", now, myaddr_, hdri->saddr(),hdri->sport(), hdri->daddr(), hdri->dport());  
				}		
				//recv(p, 0);	 this means that ,send this packet to receive function where the same packet will take the same procedure again	 //canceled by ahmad
				assert (!HDR_CMN (p)->xmit_failure_ || HDR_CMN (p)->xmit_failure_ == mac_callback);		
				target_->recv(p, (Handler *)0);	  //ÇÑÓÇá åĞå ÇáÑÒãÉ æĞáß áÇäå Êã ÇÑÓÇáåÇ áÊÇÈÚ ÇáÇÓÊŞÈÇá æáßã ßÇä ÇáãÓÇÑ ãŞØæÚ áĞÇ ÈÚÏ ÊÕáíÍ ÇáãÓÇÑ íÊã ÇÑÓÇáåÇ
			}
              		
		}
		////cout << "\n\n****************************************************************************************************************************************";	//
		return;
	}

}
static void 
mac_callback (Packet * p, void *arg)
{
    hdr_ip *hdri = hdr_ip::access(p);   //added by ahmad 
	hdr_cmn *hdrc = HDR_CMN (p);	  //added by ahmad 
    ////cout << "\n\n\n\n In mac_callback function .......  lost_link function is called and passed to it the returned packet No .... " << hdrc ->uid()<<" \n";

	 ////table_->Show_rt();  

    ((EDVR_Agent *) arg)->lost_link (p);
}
Packet *
EDVR_Agent::Make_Hello_Packet (/*int& periodic*/)
{  //============================================== ( ÚãáíÉ ÇäÔÇÁ ÑÒãÉ ÊÑÍíÈ ) ================================================
	//DEBUG
	//printf("(%d)-->Making Hello pkt\n",myaddr_);
	Packet *p = allocpkt ();
	hdr_ip *hdri = hdr_ip::access(p);
	hdr_cmn *hdrc = HDR_CMN (p);
	double now = Scheduler::instance ().clock ();
	unsigned char *walk;
    int change_count;             // ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÑÇÏ ÊÖãíäåÇ
	int no_of_bytes;			// added by ahamd	
	
	////cout << "\n********  helper_callback (Event * e) function is invoked  ***********************************************";
	//cout <<"\n\n\n\n ***** Time:" << now << "   Node (" << myaddr_ << ") broadcasted a Hello packet .... The Packet number is " << hdrc ->uid();
    //table_->Show_rt();  	
	// The packet we send wants to be broadcast
	//hdrc->prev_hop_ = myaddr_;    
    //hdrc->next_hop_ = IP_BROADCAST;
	//hdrc->addr_type_ = NS_AF_INET;		// in AODV .. NS_AF_NONE;	the comment added ahmad
	hdrc->ptype() = PT_HELLO_PACKET;   // ÊÍÏíÏ äãØ ÇáÑÒãÉ ÃäåÇ ÑÒãÉ ÊÑÍíÈ
	//hdri->daddr() = IP_BROADCAST << Address::instance().nodeshift();
	//hdri->dport() = ROUTER_PORT;
    //hdrc->iface() = -2;			// added 
	//hdrc->error() = 0;			// added 
	hdri->saddr() = myaddr_;		//  ÚäæÇä ÇáÚŞÏÉ ÇáãÕÏÑ 
	//hdri->sport() = ROUTER_PORT;	// added 
    hdrc->fst_fwd_hop_ = -99;    // ÚäæÇä Ãæá ÚŞÏÉ ãÑÑÊ ÇáÑÒãÉ åæ áÇ ÔíÆ
	hdrc->p_prev_hop_ = -99;	// ÚäæÇä ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ -99 Ãí áÇ ÔíÆ
	change_count = 0;		// added by ahmad  
	p->allocdata(change_count);			//modified 
	hdrc->size_ = change_count * 12 + IP_HDR_LEN;	// EDVR + IP

	return p;

}//=================================================  äåÇíÉ ÚãáíÉ ÇäÔÇÁ ÑÒãÉ ÇáÊÑÍíÈ  ==========================================
Packet *
EDVR_Agent::Make_Fulldump_Packet(int new_or_broken, nsaddr_t dst)
{//========================================= ( ÚãáíÉ ÇäÔÇÁ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ ) =========================================
	Packet *p = allocpkt ();
	hdr_ip *hdri = hdr_ip::access(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();

	edvr_rtable_ent *prte;
	unsigned char *walk;

	int change_count;             // ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ İí åĞå ÇáÑÒãÉ
	int rtbl_sz;			      // ÚÏÏ ÇáãÓÇÑÇÊ Çáßáí ÈÌÏæá ÇáÊæÌíå
	int unadvertiseable;		  // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí áÇ íãßä äÔÑåÇ
    
	change_count = 0;
	rtbl_sz = 0;
	unadvertiseable = 0;
	
	if (!new_or_broken)
	{
        New_Entry_Add(dst) ;	// ÅÖÇİÉ ãÓÇÑ ÌÏíÏ İí ÌÏæá ÊæÌíå ÇáÚŞÏÉ ÇáÍÇáíÉ ááÚŞÏÉ ÇáÌÇÑ Çä ßÇä ÌÇÑ ÌÏíÏ
		
	}

	int defualt =-88;
	nsaddr_t old_dst = defualt;
	uint old_metric = 255;
        //ÇáÍÕæá Úáì ãÓÇÑ áßá åÏİ æåĞå ÇáÍáŞÉ áÍÓÇÈ ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí íÌÈ ÊÖãíäåÇ æáÇ íÊã ÍÓÇÈ ÇáãÓÇÑÇÊ ÇáãŞØæÚÉ
	for (table_->InitLoop (); (prte = table_->NextLoop ()); )	
	{
		rtbl_sz++;//ÚÏÏ ÇáãÓÇÑÇÊ ÈÌÏæá ÇáÊæÌíå
		if (((prte->dst != old_dst) && (prte->metric != BIG)) || ((prte->dst == old_dst) && (prte->metric < old_metric) && (prte->metric != BIG)))
		{
			 if (prte->dst != old_dst)   
				 change_count++;	    // ááÍÕæá Úáì ÃİÖá ãÓÇÑ áßá ÚŞÏÉ åÏİ ÈÌÏæá ÇáÊæÌíå
			 old_metric = prte->metric;
			 
		}
		old_dst = prte->dst;          // áÚÏã ÇáÍÕæá Úáì ãÓÇÑíä áäİÓ ÇáåÏİ

		if (prte->metric == BIG)      // áÇ ÊÍÓÈ ÇáãÓÇÑÇÊ ÇáãŞØæÚÉ                         
		 	old_dst = defualt;        //  ÊÌÇåá ÇáãÓÇÑ ÇáÓÇÈŞ áÃäå ãŞØæÚ   //ahmad		 			     
	}

	
	////cout << "\n Node (" << myaddr_ << ") unicasts a Full_dump packet  of its RT with  " << change_count << "  entries to node (" << dst << ") ....";

	// ÇáÑÒãÉ íÌÈ Ãä ÊÑÓá áåÏİ ãÍÏÏ		
	hdrc->prev_hop_ = myaddr_;	  // ÚäæÇä ÇáÚŞÏÉ ÇáÓÇÈŞÉ åæ ÇáÚŞÏÉ ÇáÍÇáíÉ
	hdrc->next_hop_ = dst;        	// ÇáÇÑÓÇá áåÏİ ãÍÏÏ  added by ahmad
	hdrc->fst_fwd_hop_ = -99;    // ÚäæÇä Ãæá ÚŞÏÉ ÊãÑÑ åĞå ÇáÑÒãÉ added ahmad 
	hdrc->p_prev_hop_ = -99;	// ÚäæÇä ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ
	
	hdrc->addr_type_ = NS_AF_INET;		// in AODV .. NS_AF_NONE;	
	hdrc->ptype() = PT_FULLDUMP_PACKET;   // added by ahmad .....  äãØ ÇáÑÒãÉ åæ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ
	hdri->daddr() = dst;      // ÇáÇÑÓÇá Úáì Ôßá ÈË ÇÍÇÏí added by ahmad
	hdri->dport() = ROUTER_PORT;
	hdrc->iface() = -2;			// added ahmad
	hdrc->error() = 0;			// added ahmad
	hdri->saddr() = myaddr_;		// added ahmad
	hdri->sport() = ROUTER_PORT;	// added ahmad
	
    //  ÍÌÒ ÍÌã ÇáÑÒãÉ ÍÓÈ ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÑÇÏ ÊÖãíäåÇ
	p->allocdata((change_count * 17) + 1);    // 17 means 4B for dst, 4B for first f_nxt_hop , 4B for s_nxt_hop, 1B for metric and 4B for linkno + 1B for changecount....
											  
	walk = p->accessdata ();

	*(walk++) = change_count;
	
    unsigned char *last_walk=walk;            // added by ahmad   (ááãÍÇİÙÉ Úáì ÃÎÑ ãæŞÚ)  

	hdrc->size_ = change_count * 17 + IP_HDR_LEN;	// EDVR + IP
	int num_of_entries = 0;  // added ahmad
	int num_of_adver_entries = 0;  // added ahmad


	old_metric = 255;  // added 
// ÊÖãíä ÇáãÓÇÑÇÊ İí ÇáÑÒãÉ áÇÑÓÇáåÇ
	for (table_->InitLoop (); (prte = table_->NextLoop ());)
	{   
     if (((prte->dst != old_dst) && (prte->metric != BIG )) || ((prte->dst == old_dst) && (prte->metric < old_metric) && (prte->metric != BIG)))     //modified 
		{
			if ((prte->dst == old_dst) && (prte->metric < old_metric))    // added by ahmad   ÍÕáäÇ Úáì ãÓÇÑ ÃİÖá ãä ÇáãÓÇÑ ÇáĞí Êã ÊÖãíäå 
			{                                                             //  åĞÇ ÇáãÓÇÑ ÃİÖá ãä ÇáÓÇÈŞ
				walk=last_walk;                                           //  ÇÓÊÈÏáå ãä ÎáÇá äŞá ÇáãÄÔÑ áÈÏÇíÉ ÃÎÑ ãÓÇÑ İí åĞå ÇáÑÒãÉ
                change_count++;     // to return change //cout by 1 back      added by ahmad                                
                num_of_entries--;   // ááãÍÇİÙÉ Úáì ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ ÕÍíÍ   added by ahmad

			}
					 
			last_walk=walk;     // added by ahmad     ááÍİÇÙ Úáì ÃÎÑ ãæÖÚ
			num_of_entries++;
			////cout << "\n    Entry number (" << num_of_entries << ") ..  " << prte->dst << "    "  << prte->f_nxt_hop   << "    "  << prte->s_nxt_hop  << "    "  << prte->metric << "    "  << prte->link_no; 

			// ÊÖãíä åĞÇ ÇáãÓÇÑ ÈÇáÑÒãÉ áäÔÑå

			//if (!periodic && verbose_)
			//trace ("VCT %.5f _%d_ %d", now, myaddr_, prte->dst);
		          
			//================================ added by ahmad  ==================
			// ÊÍæí ÇáÑÒãÉ Úáì 17 ÈÇíÊ áßá ãÓÇÑ ãÖãä
			// 17 means 4B for dst, 4B for f_nxt_hop , 4B for s_nxt_hop, 1B for metric and 4B for link_no ....
			//==============================================================================================================

			*(walk++) = prte->dst >> 24;
			*(walk++) = (prte->dst >> 16) & 0xFF;
			*(walk++) = (prte->dst >> 8) & 0xFF;
			*(walk++) = (prte->dst >> 0) & 0xFF;

			*(walk++) = (prte->f_nxt_hop ) >> 24;
			*(walk++) = ((prte->f_nxt_hop ) >> 16) & 0xFF;
			*(walk++) = ((prte->f_nxt_hop ) >> 8) & 0xFF;
			*(walk++) = (prte->f_nxt_hop ) & 0xFF;

			*(walk++) = (prte->s_nxt_hop ) >> 24;
			*(walk++) = ((prte->s_nxt_hop ) >> 16) & 0xFF;
			*(walk++) = ((prte->s_nxt_hop ) >> 8) & 0xFF;
			*(walk++) = (prte->s_nxt_hop ) & 0xFF;

			*(walk++) = prte->metric;

			*(walk++) = (prte->link_no) >> 24;
			*(walk++) = ((prte->link_no) >> 16) & 0xFF;
			*(walk++) = ((prte->link_no) >> 8) & 0xFF;
			*(walk++) = (prte->link_no) & 0xFF;
			 
			old_metric = prte->metric;

		}
           
		old_dst = prte->dst;          // áÚÏã ÇáÍÕæá Úáì ãÓÇÑíä áäİÓ ÇáÚŞÏÉ ÇáåÏİ

		if (prte->metric == 250)      // áÇ ÊÍÓÈ ÇáãÓÇÑÇÊ ÇáãŞØæÚÉ                         
		 	old_dst = defualt;             //  ÊÌÇåá ÇáãÓÇÑ ÇáÓÇÈŞ
		  
		change_count--;	 
		   
	}

    assert(change_count == 0); 
	return p;

  } //================= äåÇíÉ ÚãáíÉ ÃäÔÇÁ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ ============
  
Packet *   
EDVR_Agent::Make_Update_Packet(int change_count)  // added 
{ //========================================= ( ÈÏÇíÉ ÚãáíÉ ÃäÔÇÁ ÑÒãÉ ÊÍÏíË ) =========================================
	edvr_rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ ÈÌÏæá ÇáÊæÌíå
	Packet *p1 = allocpkt ();
	hdr_ip *hdri = hdr_ip::access(p1);
	hdr_cmn *hdrc = HDR_CMN (p1);

	double now = Scheduler::instance ().clock ();

	unsigned char *walk;

	////cout<<"\n\n\n\n\n *****";	
	////cout<<"\n *****    Time: " << now << "   Node (" << myaddr_ << ") Broadcasted ... Update Packet number " << hdrc ->uid() << "          *************";
	////cout<<"\n *****                          The packet includes  " << change_count  << "  entries                                      ****************";
	////cout<<"\n ********************************************************************************************************************* \n";
	// ÇáÑÒãÉ íÌÈ Ãä ÊäÔÑ
	//hdrc->prev_hop_ = myaddr_;	  // added by ahmad
	//hdrc->next_hop_ = IP_BROADCAST;
	//hdrc->addr_type_ = NS_AF_INET;		// in AODV .. NS_AF_NONE;	the comment added on 
	hdrc->ptype() = PT_UPDATE_PACKET;   //äãØ åĞå ÇáÑÒãÉ åæ ÑÒãÉ ÊÍÏíË 
    //hdri->daddr() = IP_BROADCAST << Address::instance().nodeshift();
	//hdri->dport() = ROUTER_PORT;
	//hdrc->iface() = -2;			// added 
	//hdrc->error() = 0;			// added 
	hdri->saddr() = myaddr_;		// ÚäæÇä ÇáÚŞÏÉ ÇáãÕÏÑ åæ ÚäæÇä ÇáÚŞÏÉ ÇáÍÇáíÉ
	//hdri->sport() = ROUTER_PORT;	// 
    hdrc->fst_fwd_hop_ = -99;    // ÚäæÇä Ãæá ÚŞÏÉ ãÑÑÊ ÇáÑÒãÉ
	hdrc->p_prev_hop_ = -99;	// ÚäæÇä ÇáŞİÒÉ ãÇ ŞÈá ÇáÓÇÈŞÉ
	p1->allocdata((change_count * 17) + 1);		// ÍÌã ÇáÑÒãÉ åæ 17 ÈÇíÊ áßá ãÓÇÑ * ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÑÇÏ ÊÖãíäåÇ
	walk = p1->accessdata ();

	*(walk++) = change_count;                     // ÈÇíÊ æÇÍÏ áÚÏÏ ÇáãÓÇÑÇÊ ÇáãÑÇÏ ÊÖãíäåÇ İí ÑÒãÉ ÇáÊÍÏíË
	*(walk++) = (myaddr_ >> 0) & 0xFF;	*/
	hdrc->size_ = change_count * 17 + IP_HDR_LEN;	// EDVR + IP

	//======================= ÊÖãíä ÇáãÓÇÑÇÊ ÈÑÒãÉ ÇáÊÍÏíË ========================================
    ////cout<< "\n *****************              ÇáãÓÇÑÇÊ ÇáÊí íÌÈ ÊÖãíäåÇ ÈÑÒãÉ ÇáÊÍÏíË ....               *****************";
    for (table_->InitLoop (); (prte = table_->NextLoop ()); )
	{
		
		if (prte->advertise /*&& prte->metric > 0*/)	 //  ÊÖãíä Ãí ãÓÇÑ íÌÈ äÔÑå 
		{    

			// ÊÖãíä åĞÇ ÇáãÓÇÑ ÈÑÒãÉ ÇáÊÍÏíË
			*(walk++) = prte->dst >> 24;					// ÃÑÈÚ ÈÇíÊ ááÚŞÏÉ ÇáåÏİ
 			*(walk++) = (prte->dst >> 16) & 0xFF;
 			*(walk++) = (prte->dst >> 8) & 0xFF;
 			*(walk++) = (prte->dst >> 0) & 0xFF;

			*(walk++) = prte->f_nxt_hop  >> 24;					// ÃÑÈÚ ÈÇíÊ áÚäæÇä ÇáŞİÒÉ ÇáÃæáì
 			*(walk++) = (prte->f_nxt_hop  >> 16) & 0xFF;
 			*(walk++) = (prte->f_nxt_hop  >> 8) & 0xFF;
 			*(walk++) = (prte->f_nxt_hop  >> 0) & 0xFF;

            *(walk++) = prte->s_nxt_hop  >> 24;					// ÃÑÈÚ ÈÇíÊ áÚäæÇä ÇáŞİÒÉ ÇáËÇäíÉ
 			*(walk++) = (prte->s_nxt_hop  >> 16) & 0xFF;
 			*(walk++) = (prte->s_nxt_hop  >> 8) & 0xFF;
 			*(walk++) = (prte->s_nxt_hop  >> 0) & 0xFF;


			*(walk++) = prte->metric;						// ÈÇíÊ æÇÍÏ áÚÏÏ ÇáŞİÒÇÊ

			*(walk++) = (prte->link_no) >> 24;				// ÃÑÈÚ ÈÇíÊ áÑŞã ÇáæÕáÉ
			*(walk++) = ((prte->link_no) >> 16) & 0xFF;
			*(walk++) = ((prte->link_no) >> 8) & 0xFF;
			*(walk++) = (prte->link_no) & 0xFF;


			////cout<< "\n *****************		                       " << prte->dst <<"     " << prte->f_nxt_hop  <<"     " << prte->s_nxt_hop  <<"     " << prte->metric <<"     " << prte->link_no << "	   		                            *****************";
			change_count--;
		}

		prte->advertise = false;		// áÇ íæÌÏ ÏÇÚí áäÔÑ åĞÇ ÇáãÓÇÑ ãÑÉ ËÇäíÉ
	}

	////cout<< "\n *****************                                                                                              *****************";

	assert(change_count == 0);
    //s.schedule (target_, p1, jitter(EDVR_BROADCAST_JITTER, be_random_));    //ÃÑÓá ÑÒãÉ ÇáÊÍÏíË åĞå ãÚ İÑæŞ ãä ÃÌá ÇáÊÒÇãä
	return p1;
	
}	//=============== äåÇíÉ ÚãáíÉ ÅäÔÇÁ ÑÒãÉ ÇáÊÍÏíË==============
Packet *
EDVR_Agent::Make_RRQ_Packet(nsaddr_t requester, nsaddr_t destination)  // added 
{	//=============== ÈÏÇíÉ ÚãáíÉ ÅäÔÇÁ ÑÒãÉ ØáÈ ÇáãÓÇÑ ===========

	//rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ ÈÌÏæá ÇáÊæÌíå
	Packet *p = allocpkt ();
	
	hdr_ip *hdri = hdr_ip::access(p);
	hdr_cmn *hdrc = HDR_CMN (p);
    
    //hdri->saddr() = myaddr_;		// added
	double now = Scheduler::instance ().clock ();
   	unsigned char *walk;
	Scheduler & s = Scheduler::instance ();			
	
    ////cout<<"\n *****";	
	////cout<<"\n ***** At " << now << "   Node (" << myaddr_ << ") is Generated and Broadcasted a Route Request  "<<hdrc->uid();
	////cout<<"\n *****       Looking for a route to Node ("<<  destination << ") because it has no route available";
	   
	// ÇáÑÒãÉ ÇáÊí äÑíÏ ÇÑÓÇáåÇ íÌÈ Ãä ÊäÔÑ ÚÈÑ ÇáÔÈßÉ
	/*hdrc->next_hop_ = IP_BROADCAST;    // äÔÑ    added ahmad 
	hdrc->prev_hop_ = myaddr_;    // äÔÑ    added ahmad 
	hdrc->addr_type_ = NS_AF_INET;
		
	hdri->daddr() = IP_BROADCAST << Address::instance().nodeshift();
	hdri->dport() = ROUTER_PORT;       // Ãí ÃäåÇ ÑÒãÉ ÊæÌíå 
    hdri->saddr() = myaddr_;		// added 
	hdri->sport() = ROUTER_PORT;	// added */
	   	
	
    hdrc->fst_fwd_hop_ = -99;    // ÚäæÇä Ãæá ÚŞÏÉ ÊãÑÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
	hdrc->p_prev_hop_ = -99;	// ÚäæÇä ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ

    //hdrc->num_forwards_= 1;  
    hdrc->ptype() = PT_ROUTE_REQUEST_PACKET;   // äãØ åĞå ÇáÑÒãÉ åæ ÑÒãÉ ØáÈ ãÓÇÑ
					
	//========================================
	 int change_count=1;
    p->allocdata(8);			//8 Bytes 
	hdrc->size_ = change_count * 8 + IP_HDR_LEN;	//ÍÌã ÇáÑÒãÉ EDVR + IP
	hdri->ttl_=RREQ_Threshold;	//ÊÍÏíÏ ÚÏÏ ÇáŞİÒÇÊ áåÇ  Ãí ÚÊÈÉ ÑÒãÉ ØáÈ ÇáãÓÇÑ
	walk = p->accessdata ();//ãÄÔÑ Úáì ÈÏÇíÉÇáÑÒãÉ

    //======================= æÖÚ ÇáãÚáæãÇÊ ÈÑÒãÉ ØáÈ ÇáãÓÇÑ ========================================
	*(walk++) =  requester >> 24;                    // ÇÑÈÚÉ ÈÇíÊ áÚäæÇä ÇáÚŞÏÉ ÇáØÇáÈÉ ÇáÊí ÇÑÓáÊ ÑÒãÉ ØáÈ ÇáãÓÇÑ 
	*(walk++) = (requester >> 8) & 0xFF;
	*(walk++) = (requester >> 0) & 0xFF;

	*(walk++) =  destination >> 24;					// ÇÑÈÚÉ ÈÇíÊ áÚäæÇä ÇáÚŞÏÉ ÇáåÏİ ÇáÊí ÇÑÓáÊ ÑÒãÉ ØáÈ ÇáãÓÇÑ ãä ÃÌáåÇ
 	*(walk++) = (destination >> 16) & 0xFF;
 	*(walk++) = (destination >> 8) & 0xFF;
 	*(walk++) = (destination >> 0) & 0xFF;
 
     return p ;
	
}	//================ äåÇíÉ ÚãáíÉ ÅäÔÇÁ ÑÒãÉ ØáÈ ÇáãÓÇÑ  ============
  Packet *            
EDVR_Agent::Make_RRP_Packet(nsaddr_t sende_to,nsaddr_t dst,nsaddr_t f_nxt_hop ,nsaddr_t s_nxt_hop ,int metric,int link_no)
{	//=================   ÈÏÇíÉ ÚãáíÉ ÅäÔÇÁ ÑÒãÉ ÑÏ ÇáãÓÇÑ    ==================================================
    Packet *p = allocpkt ();
	hdr_ip *hdri = hdr_ip::access(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = Scheduler::instance ().clock ();

	//rtable_ent *prte;
	unsigned char *walk;
	
	// ÇáÑÒãÉ íÌÈ Ãä ÊÑÓá áåÏİ ãÍÏÏ 
	//hdrc->next_hop_ = sende_to;                      // ÇÑÓÇáåÇ Úáì Ôßá ÈË ÇÍÇÏí  added by ahmad
	hdrc->addr_type_ = NS_AF_INET;
	hdri->daddr() = sende_to;      // ÚäæÇä ÇáÚŞÏÉ ÇáØÇáÈÉ
	hdri->dport() = ROUTER_PORT;
    hdrc->prev_hop_ = myaddr_;    // added ahmad 
	//hdrc->fst_fwd_hop_ = -99;    // ÚäæÇä Ãæá ÚŞÏÉ ãÑÑÊ ÇáÑÒãÉ added ahmad 
	//hdrc->p_prev_hop_ = -99;  
    hdrc->ptype() = PT_ROUTE_REPLY_PACKET;   // äãØ ÇáÑÒãÉ åæ ÑÒãÉ ÑÏ ÇáãÓÇÑ
	//============================================================
	
	////cout<<"\n *****";	
	////cout<<"\n ***** At " << now << "   Node (" << myaddr_ << ") is Generated and Unicasted a Route Replay Packet";
	////cout<<"\n *****         to Node ("<<sende_to <<"), where found a route to Node ("<<dst <<")";
	////cout<<"\n *****";
	int change_count=1;
	int Number_of_bytes=17;          // ÚÏÏ ÇáÈÇíÊÇÊ ÇáãÖãäÉ ÈåĞå ÇáÑÒãÉ
	p->allocdata(change_count * Number_of_bytes);	 //Packet readt to carry 21 Bytes 4B (myaddr),4B (dst),4B()
	                                //4B (s_nxt_hop ),1B (metric) and 4B for (link_no)
	hdrc->size_ = change_count *17 + IP_HDR_LEN;	// EDVR + IP

	walk = p->accessdata ();

		//	*(walk++) = myaddr_ >> 24;                    // save 4 Bytes for the sender in the Updat packet 
		//	*(walk++) = (myaddr_ >> 16) & 0xFF;
		//	*(walk++) = (myaddr_ >> 8) & 0xFF;
		//	*(walk++) = (myaddr_ >> 0) & 0xFF;

			
			*(walk++) = dst >> 24;					// ÇÑÈÚ ÈÇíÊ áÚäæÇä ÇáÚŞÏÉ ÇáåÏİ ÇáÊí ÇÑÓáÊ ãä ÃÌáåÇ ÑÒãÉ ØáÈ ÇáãÓÇÑ
 			*(walk++) = (dst >> 16) & 0xFF;
 			*(walk++) = (dst >> 8) & 0xFF;
 			*(walk++) = (dst >> 0) & 0xFF;

			*(walk++) = f_nxt_hop  >> 24;					// ÇÑÈÚ ÈÇíÊ áÚäæÇä ÇáŞİÒÉ ÇáÃæáì ÈÇÊÌÇå ÇáÚŞÏÉ ÇáåÏİ
 			*(walk++) = (f_nxt_hop  >> 16) & 0xFF;
 			*(walk++) = (f_nxt_hop  >> 8) & 0xFF;
 			*(walk++) = (f_nxt_hop  >> 0) & 0xFF;

            *(walk++) = s_nxt_hop  >> 24;					// ÇÑÈÚ ÈÇíÊ áÚäæÇä ÇáŞİÒÉ ÇáËÇäíÉ ÈÇÊÌÇå ÇáÚŞÏÉ ÇáåÏİ
 			*(walk++) = (s_nxt_hop >> 16) & 0xFF;
 			*(walk++) = (s_nxt_hop  >> 8) & 0xFF;
 			*(walk++) = (s_nxt_hop  >> 0) & 0xFF;


			*(walk++) = metric;						// ÈÇíÊ æÇÍÏ áÚÏÏ ÇáŞİÒÇÊ

			*(walk++) = (link_no) >> 24;				// ÇÑÈÚ ÈÇíÊ áÑŞã ÇáæÕáÉ
			*(walk++) = ((link_no) >> 16) & 0xFF;
			*(walk++) = ((link_no) >> 8) & 0xFF;
			*(walk++) = (link_no) & 0xFF;
   
	return p;
}   //============================ äåÇíÉ ÚãáíÉ ÅäÔÇÁ ÑÒãÉ ÑÏ ÇáãÓÇÑ   =======================================
void	
EDVR_Agent::Receive_Hello_Packet (Packet * p)
{ //=============== ( ÈÏÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÊÑÍíÈ) ======

	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();
	
	// it's a EDVR packet
	unsigned char *d = p->accessdata ();     // change count  ãÄÔÑ Úáì ÈÏÇíÉ ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ  d
	unsigned char *w = d + 1;    // ãÄÔÑ Úáì ÇáÈÇíÊ ÇáÊÇáí ãä ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÇáåÏİ w
	
	
	edvr_rtable_ent *prte;		// ãÓÇÑ Öãä ÌÏæá ÇáÊæÌíå
		
	int yes_it_is_a_neighbr= check_Neighbor_Process(hdrc->prev_hop_);		//  ÚãáíÉ İÍÕ ÇáÌÇÑ áİÍÕ ãÑÓá ÇáÑÒãÉ Ãí ÇáŞİÒÉ ÇáÊÇáíÉ åá åæ ÌÇÑ ÌÏíÏ Ãã ãÚÑæİ ãÓÈŞÇ  // added by ahmad 
	
	return;

}  //====================== äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÊÑÍíÈ  ====================
void	
EDVR_Agent::Receive_Fulldump_Packet(Packet * p)
{ //========================================= ( ÈÏÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ ) ========================================= 
	int xx;
	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();
	  
	// it's a EDVR packet
	int i;
	unsigned char *d = p->accessdata ();     // change count  ãÄÔÑ Úáì ÈÏÇíÉ ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ  d
	unsigned char *w = d + 1;    // ãÄÔÑ Úáì ÇáÈÇíÊ ÇáÊÇáí ãä ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÇáåÏİ w

	edvr_rtable_ent rte;		// ãÓÇÑ ÌÏíÏ íÊã ÊÚáãå ãä ÇáÑÒãÉ ÇáãÓÊŞÈáÉ
	edvr_rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ Öãä ÌÏæá ÇáÊæÌíå
    int kk = *d;
	
	int elements;
	edvr_rtable_ent *pr2;
	  
	//nsaddr_t FULLDUMP_Sender = Address::instance().get_nodeaddr(hdri->saddr());	// added by ahmad
	int FULLDUMP_Sender = hdrc->prev_hop_;//ÚäæÇä ãÑÓá åĞå ÇáÑÒãÉ

	nsaddr_t dst;

	int change_count = 0;             // ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ ÇáãÓÊŞÈáÉ
	int modify_rt = 0;				  // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí ÊãÊ ÅÖÇİÊåÇ Ãæ ÊÚÏíáåÇ 

	////cout << "\n Node (" << myaddr_ << ") received a full routing information pkt  from node (" << FULLDUMP_Sender << ")........";  //stoped 

   
//======================================================================
	
  	int yes_it_is_a_neighbr = check_Neighbor_Process(hdrc->prev_hop_);		//  İÍÕ ÇáÚŞÏÉ ÇáÓÇÈŞÉ ÇáÊí ãÑÑÊ ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ ÃäåÇ ÌÇÑ 
	
	
//========================================================================================================================================

 	// Óæİ ÊÊÚÇãá ÇáÚŞÏÉ ãÚ ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ ÇáãÓÊŞÈáÉ ãÓÇÑ ãÓÇÑ
    ////cout << "\n\n Node (" << myaddr_ << ") starts to deal with the entries of the full routing information pkt that has been received from node (" << FULLDUMP_Sender<< ").  Number of entries = " << kk;

	for (i = *d; i > 0; i--)     // *d åí ãÄÔÑ Úáì ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ İí ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ
	{  // ãä ÃÌá ßá ãÓÇÑ ãÖãä
		bzero(&rte, sizeof(rte));		//ÊåíÆÉ ãÓÇÑ ÌÏíÏ

		// ÇÓÊÎÑÇÌ ÇáãÚáæãÇÊ ÇáãÓÇÑ ÇáĞí íÊã ÇáÊÚÇãá ãÚå
		rte.dst = *(w++);
		rte.dst = rte.dst << 8 | *(w++);
		rte.dst = rte.dst << 8 | *(w++);
		rte.dst = rte.dst << 8 | *(w++);
		        
		rte.f_nxt_hop  = *(w++);
		rte.f_nxt_hop  =rte.f_nxt_hop  << 8 | *(w++);
		rte.f_nxt_hop  =rte.f_nxt_hop  << 8 | *(w++);
		rte.f_nxt_hop  =rte.f_nxt_hop  << 8 | *(w++);

		rte.s_nxt_hop  = *(w++);
		rte.s_nxt_hop  =rte.s_nxt_hop  << 8 | *(w++);
		rte.s_nxt_hop  =rte.s_nxt_hop  << 8 | *(w++);
		rte.s_nxt_hop  =rte.s_nxt_hop  << 8 | *(w++);

		rte.metric = *(w++);

		rte.link_no = *(w++);
		rte.link_no = rte.link_no << 8 | *(w++);
		rte.link_no = rte.link_no << 8 | *(w++);
		rte.link_no = rte.link_no << 8 | *(w++); 


		//int intry_code = kk-i+1;//ÑŞã ÇáãÓÇÑ
			////cout << "\n\n Entry no (" << intry_code << ") is processed now ...  " << rte.dst << "    " << rte.f_nxt_hop   << "    " << rte.s_nxt_hop  << "    " << rte.metric << "    " << rte.link_no;

		if(myaddr_==rte.dst)    // ãÓÇÑ íÔíÑ ááÚŞÏÉ ÇáÍÇáíÉ
		{	rte.dst = FULLDUMP_Sender;			//rte.dst= ÚäæÇä ÇáÚŞÏÉ ÇáÊí ÃÑÓáÊ ÇáÑÒãÉ
			if (rte.metric>1)
			{
				////cout << "        ....don't consider this entry because it is belongs to me ;
				continue;						// áÇ íÊã ÇÚÊÈÇÑ åĞÇ ÇáãÓÇÑ áÃäå íÔíÑ ááÚŞÏÉ ÇáÍÇáíÉ
			}
		}
		else
		{
			if (rte.metric == 0)    // added			åĞÇ ÇáãÓÇÑ íÔíÑ áãÑÓá ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ (ßá ÚŞÏÉ áÏíåÇ ãÓÇÑ ÎÇÕ ÈåÇ)
			{						// added 
				rte.s_nxt_hop =-99;		// added 			// added 	Çäå ãÓÇÑ ãÈÇÔÑ áĞÇ ÇáŞİÒÉ ÇáËÇäíÉ åæ áÇ ÔíÁ
				rte.link_no = myaddr_ * 10000 + rte.dst;		// added 	ÑŞã ÇáæÕáÉ ÇáÌÏíÏ ÈÏáÇğ ãä ÑŞã ÇáæÕáÉ ÇáãÓÊŞÈá.. íÌÈ Ãä íßæä Èíä ÇáÚŞÏÉ ÇáÍÇáíÉ æãÑÓá ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ 	
			}						// added 

			else					// added 
				rte.s_nxt_hop =rte.f_nxt_hop ;//ÇáŞİÒÉ ÇáËÇäíÉ åí ÇáŞİÒÉ ÇáÃæáì

			if (rte.metric != BIG)
				rte.metric += 1;	  //ÒíÇÏÉ ÚÏÏ ÇáŞİÒÇÊ ÈãŞÏÇÑ æÇÍÏ
		}

		rte.f_nxt_hop  = FULLDUMP_Sender;   // ÇáŞİÒÉ ÇáÃæáì åí ãÑÓá ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ
		
		rte.advert_seqnum = false;	 //ahmad
		rte.advert_metric = false;	 //ahmad
		rte.advertise = true;  // added    íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ
		rte.changed_at = now;  
		////cout << " ..... According to the algorithm, the entry is changed to  " << rte.dst << "   " << rte.f_nxt_hop   << "  " << rte.s_nxt_hop  << "   " << rte.metric << "   " << rte.link_no ;

		//if ((prte = table_->GetEntry1 (rte.dst, FULLDUMP_Sender,&myaddr_)))   // äÑì Åä ßÇä áÏÈäÇ ãÓÇÑ ááåÏİ ÚÈÑ ÇáŞİÒÉ ÇáÃæáì		
			////cout << "\n      - Node (" << myaddr_ << ") found the same route to node (" << rte.dst << ")";

		////*********** äŞÑÑ åá íÌÈ Ãä äÚÏá ÌÏæá ÇáÊæÌíå *********/

		prte = table_->GetEntry1 (rte.dst, FULLDUMP_Sender,&myaddr_);	// äÑì Åä ßÇä íæÌÏ ãÓÇÑ ááåÏİ Úä ØÑíŞ äİÓ ÇáŞİÒÉ ÇáÃæáì		

		if (!prte)
		{  
			// we've heard about a brand new destination
			////cout << "\n      - Node (" << myaddr_ << ") didn't find a route to node (" << rte.dst << ") through node (" << rte.f_nxt_hop  << ")";
			if (rte.metric == 1 && rte.dst == rte.f_nxt_hop )    // ááÊÃßÏ Ãä åĞÇ ÇáãÓÇÑ íÚæÏ áÌÇÑ
			{
				//1-  scheduling the timeout of the new neighbour....
				////cout<< " .This node is considered as a new neighbour.";
				rte.timeout_event = new Event ();
				s.schedule (helper_, rte.timeout_event, min_update_periods_ * perup_);
			}
 
			// ÅÖÇİÉ åĞÇ ÇáãÓÇÑ ÈÌÏæá ÇáÊæÌíå
			////cout << "\n      - Node (" << myaddr_ <<  ") got a new rout to node (" << rte.dst << ")";		   // stopped
			xx =1;
			int write = Update_Route(NULL, &rte,xx);		// added 
			if (write == 1)		// added 
				modify_rt++;      // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí ÊãÊ ÅÖÇİÊåÇ Ãæ ÊÚÏíáåÇ

		}
		else if (rte.metric < prte->metric)			// İí ÍÇá æÌæÏ ãÓÇÑ äÎÊÇÑ ÇáÃİÖá 
			{ //ÇáãÓÇÑ ÇáÍÇáí ÃİÖá ãä ÇáãÓÇÑ ÇáãæÌæÏ
				 //if (prte->metric == BIG)
					////cout << "\n      - Broken route is found. So, the new route is better than the one that we have.  Node (" << myaddr_ << ") is calling Update_Route function to repair the route... ";
				 //else
					////cout << "\n      - The route is found, but the new route is better than the one that we have (new metric is better).  Node (" << myaddr_ << ") is calling Update_Route function to add the new entry... ";


				if (rte.dst == rte.f_nxt_hop )
					rte.timeout_event = new Event ();		// Êã ÇÓÊŞÈÇá ãÓÇÑ íÚæÏ ááÚŞÏÉ ÇáãÑÓáÉ áĞÇ íÌÈ ÅÚÊÈÇÑå ÌÇÑ ÌÏíÏ æíÌÈ ÇäÔÇÁ ÌÏË ÇäÊåÇÁ ÌÏíÏ áå
					
				int xx = 2;
				//Update_Route(prte,&rte,xx);      // stopped
				int write = Update_Route(prte, &rte,xx);		// ÊÚÏíá Ãæ ÅÖÇİÉ ÇáãÓÇÑ
				if (write == 1)		// added 
					modify_rt++;      // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí ÊãÊ ÅÖÇİÊåÇ Ãæ ÊÚÏíáåÇ.

			}
			else
			{
				////cout << ". The new entry is ignored because node (" << myaddr_ << ") has the same or better route to node (" << prte->dst << ")   " << prte->f_nxt_hop  << "   " << prte->s_nxt_hop  << "   " << prte->metric << "   " << prte->link_no;
				continue;  
				// íÊã ÊÌÇåá ÇáãÓÇÑ áÃä ÇáÚŞÏÉ áåÇ ãÓÇÑÃİÖá Ãæ äİÓ ÇáãÓÇÑ
  			}

		  
		// äÑì ÅĞÇ ßÇä ãä Çáããßä Ãä äÑÓá ÇáÑÒã ÇáãÎÒäÉ İí ÇáÑÊá İí ÇáãÓÇÑ ÇáãÈÇÔÑ ÍíË ÊÎÒä ÇáÑÒã ÈÇáãÓÇÑ ÇáãÈÇÔÑ İŞØ			
		prte = NULL;
		int num_of_q_packets = 0;  //áãÚÑİÉ ÚÏÏ ÇáÑÒã ÇáãÎÒäÉ ÈÇáÑÊá                 
        
		if (rte.dst!= rte.f_nxt_hop )   // äÍä äÊÚÇãá ãÚ ãÓÇÑ ÛíÑ ãÈÇÔÑ áĞÇ äÌÏ ãÓÇÑ ãÈÇÔÑ
		{
			prte= table_->GetEntry1 (rte.dst, rte.dst,&myaddr_);  // ÇáÍÕæá Úáì ãÓÇÑ ãÈÇÔÑ ááÚŞÏÉ ÇáåÏİ 																 
			if (prte)
                bcopy(prte, &rte,sizeof(edvr_rtable_ent));
		}

		if(prte)
			if (rte.q )    // äİÍÕ Åä ßÇä íæÌÏ ÑÒã ãÎÒäÉ İí ÇáãÓÇÑ ÇáãÈÇÔÑ ÍÊì áæ Ãä åĞÇ ÇáãÓÇÑ ÇáãÈÇÔÑ ãŞØæÚ
			{	            // áÃäå ÍÕáäÇ Úáì ãÓÇÑ ÌÏíÏ áåĞÇ ÇáåÏİ áÃäå åäÇ áÏíäÇ ãÓÇÑÇÊ ãÊÚÏÏÉ ÍíË íãßä Ãä äÓÊÎÏã ÇáãÓÇÑ ÇáÍÇáí áÇÑÓÇá ÇáÑÒã ÇáãÎÒäÉ
				////cout << "\n In receive_full routing information pkt function .... Node (" << myaddr_ << ") is calling recv(queued_p, 0) ..... to give the packets to ourselves to forward";

				Packet *queued_p;
				while ((queued_p = rte.q->deque()))
				{
					num_of_q_packets++;   //added by ahmad
					// XXX possible loop here  
					// while ((queued_p = rte.q->deque()))
					// Only retry once to avoid looping
					// for (int jj = 0; jj < rte.q->length(); jj++){
					//  queued_p = rte.q->deque();
					recv(queued_p, 0); // ÃÚØí ÇáÑÒã ááÚŞÏÉ ÇáÍÇáíÉ áÊŞæã ÈÊãÑíÑåÇ
				}

				////cout << "\n\n In receive_full routing information pkt function .... Number of packets that were queued  is  " << num_of_q_packets << "\n";
	 
				//==============================================================   // added by ahmad
				// The next 4 statements are used to deque the entry that has been qued to be send...
				// The second statement (rte.q = 0;) and last statment (table_->AddEntry (rte,ahmad); ) to set q field in te routing table to 0.
				// =============================================================
				delete rte.q;	
				rte.q = 0;
				nsaddr_t *ahmad = &myaddr_;

				
				////cout<< "\n In receive_full routing information pkt function .... Node (" << myaddr_ << ") is going to reset the quee the following entry ...  " << rte.dst << "\t" << rte.f_nxt_hop << "\t" << rte.metric<<"\t" << rte.seqnum << "\t" << rte.new_seqnum_at<< "\t" << rte.wst<<" by reseting the q field\n";   // added by ahmad
				//table_->AddEntry(rte,ahmad);        // added by ahmad
				bcopy(&rte, prte,sizeof(edvr_rtable_ent));	 // copy all rte fields over where prte pointing in my Routing table //ahmad
			}
			
	} // äåÇíÉ ÇáÍáŞÉ ÇáÊí ÊŞÑÃ ÇáãÓÇÑÇÊ ÇáãÖãäÉ ÈÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ ãÓÇÑ ãÓÇÑ

  /* ////cout
	if (modify_rt != 0)
	{
		//cout << "\n The Routing table has been updated. Number of entries that have been added or overwritten is " << modify_rt << " ......";
		//cout << "\n\n Routing table of node ( " << myaddr_ << " )  consists of " << table_->number_of_elts() << " entries after dealing with the received full routing information pkt .... \n";
		
		////table_->Show_rt();  //ÇÙåÇÑ ÌÏæá ÇáÊæÌíå
	}
	else 
		//cout << "\n Node (" << myaddr_ << ")  didn't update its routing table ....... \n";
		*/
}	//================  äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ãÚáæãÇÊ ÇáÊæÌíå ÇáßÇãáÉ  ==============
void 
EDVR_Agent::Receive_Update_Packet(Packet * p)
{ //========================================= (ÈÏÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÇáÊÍÏíË ) ========================================= 
	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();

	unsigned char *d = p->accessdata ();     // ãÄÔÑ áÈÏÇíÉ ÇáÑÒãÉ áÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ d 
	unsigned char *w = d + 1;    // ãÄÔÑ ááÈÇíÊ ÇáÊÇáí ÇáĞí åæ ÇáÚŞÏÉ ÇáåÏİ

	edvr_rtable_ent rte;		// ÇáãÓÇÑ ÇáÌÏíÏ ÇáĞí ÓíÊã ÊÚáãå ãä ÑÒãÉ ÇáÊÍÏíË ÇáãÓÊŞÈáÉ
	edvr_rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ ÈÌÏæá ÇáÊæÌíå

	int xx;
  
	// nsaddr_t first = Address::instance().get_nodeaddr(hdri->saddr());	// added by ahmad
	//nsaddr_t src= hdrc->prev_hop_;	 //ÚäæÇä ãÑÓá ÇáÑÒãÉ     ahmad
	//nsaddr_t dst;
	////cout <<	"\n u r in the Receive_Update_Packet sender by the hdrc->prev_hop = "<<src;

	bzero(&rte, sizeof(rte));		//ÊåíÆÉ ÇáãÓÇÑ ÇáÌÏíÏ

	// ÇÓÊÎÑÇÌ ÇáÈíÇäÇÊ ãä ÑÒãÉ ÇáÊÍÏíË
    int modify_rt=0;
	int change_count = *d;
	int kk = change_count;		// ÚÏÇÏ áÚÏÏ ÇáãÓÇÑÇÊ


	int packet_sender = hdrc->prev_hop_;//ãÑÓá ÑÒãÉ ÇáÊÍÏíË
	
	////cout <<	"\n u r in the receive_Updatepacket	packet sender by the paet itself is = "<<src;
    if (myaddr_ == hdrc->prev_hop_)
	{//ÇáÚŞÏÉ ÇáÍÇáíÉ åí ãÑÓá åĞå ÇáÑÒãÉ íÊã ÇåãÇá ÇáÑÒãÉ
		// ÇáÑÒãÉ ÇäÇ ÇÑÓáÊåÇ äåãáåÇ
		////cout << "\n the Route Update Packet is discarded ..... because the packet has been sent by me (loop).   the condition is ..(myaddr_ == packet_sender)\n";
	    drop(p, DROP_RTR_ROUTE_LOOP);   // added 
		return;
	}
	//======================================================================
		
	int Yes_it_is_a_neighbor= check_Neighbor_Process(hdrc->prev_hop_);		//  ÊİÍÕ ãÑÓá ÇáÑÒãÉ Åä ßÇä ÌÇÑ ãÚÑæİ ãÓÈŞÇ Ãæ ÌÇÑ ÌÏíÏ
	
  	
//========================================================================================================================================

	////cout<<"\n\n Node (" << myaddr_ << ") strats to deal with the the received Route Update Packet...... ";

	while(change_count!=0)	 // ÇáÊÚÇãá ãÚ ÇáãÓÇÑÇÊ ÇáãÖãäÉ ãÓÇÑ ãÓÇÑ
	{
		int	destination = *(w++);
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);
		
		int first_hop = *(w++);
		first_hop = first_hop << 8 | *(w++);
		first_hop = first_hop << 8 | *(w++);
		first_hop = first_hop << 8 | *(w++);

	    int second_hop = *(w++);
		second_hop = second_hop << 8 | *(w++);
		second_hop = second_hop << 8 | *(w++);
		second_hop = second_hop << 8 | *(w++);

		int	number_of_hops = *(w++);

		int	link_no = *(w++);
		link_no = link_no << 8 | *(w++);
		link_no = link_no << 8 | *(w++);
		link_no = link_no << 8 | *(w++); 

		int intry_code = kk-change_count+1;		// ÑŞã ÇáãÓÇÑ ÇáĞí íÊã ÇáÊÚÇãá ãÚå
		////cout<<"\n	Entry  (" << intry_code << ")  is    " << destination << "   " << first_hop << "   " << second_hop << "   " << number_of_hops << "   " << link_no;
		
		if(number_of_hops==BIG)		   // this is such as an Error packet // added by ahmad 
		{	 
			//cout<<"\n my address is "<< myaddr_ << "   destination = "<< destination   <<"   packet_sender="<<packet_sender<<"  is it a neighbor "<< Yes_it_is_a_neighbor; 
				 //ãÓÇÑ ãŞØæÚ
      		 if(destination==myaddr_ && first_hop==myaddr_ && Yes_it_is_a_neighbor==1)  // 
			 {	//Ãí Çä ÇáÌÇÑ íÎÈÑäí Çä ÇáæÕáÉ ÈíääÇ ãŞØæÚÉ áĞÇ íÌÈ ÇÑÓÇá ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ áå												  //  
				 //cout<<"\n Yes my address is "<< myaddr_ << "   destination = "<< destination   <<"   packet_sender="<<packet_sender<<"  Yes_it_is_a_neighbor"; 
				 ////cout<<"\n This nighbour tells me that the link between it and me is broke .. I've to send it a full routing information pkt  "; 
				 ////cout<<"\n This Packet f_nxt_hop  is "<<hdrc->fst_fwd_hop_  <<"\n This Packet p_previous hop is "<<hdrc->p_prev_hop_;
				 
				 Packet *p = Make_Fulldump_Packet(1, packet_sender);         // åĞÇ ÇáÊÇÈÚ íÓÊÏÚì áÊÍÖíÑ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ áÇä ÇáæÕáÉ ßÇäÊ ãŞØæÚÉ áåĞÇ ÇáåÏİ æÊã ÊÕáíÍåÇ æáÇ íæÌÏ ÏÇÚí áÅÖÇİÉ ãÓÇÑ ÌÏíÏ
														
				if (p)
				{	
					
					assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
				}
			 }
			 else
				 if( second_hop ==-99)   //ãÓÇÑ ãÈÇÔÑ
			        Broken_Route_Process(destination, packet_sender, destination, link_no);	   // added by  ahmad 
				 else
				    //Broken_Route_Process(destination, first_hop , second_hop , link_no);	   // added by  ahmad 
					Broken_Route_Process(destination, -1 , -1 , link_no);	   // added by  ahmad 

			 change_count--;
			 modify_rt++;      // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí ÊãÊ ÅÖÇİÊåÇ Ãæ ÊÚÏíáåÇ
			 continue;
	    }
      
		if (myaddr_ == first_hop || myaddr_ == second_hop)    // ÊÌÇåá åĞÇ ÇáãÓÇÑ áÃäå ÍáŞÉ
		{
			////cout<<"\t  .... íÊã ÇåãÇá ÇáãÓÇÑ áÇä ÚäæÇä ÇáÚŞÏÉ ÇáÍÇáíÉ åæ ÇáŞİÒÉ ÇáÃæáì Ãæ ÇáŞİÒÉ ÇáËÇäíÉ
			change_count--;
			continue;
		}
	
		// ÍİÙ ÍŞæá ÇáãÓÇÑ İí ãÓÇÑ ÌÏíÏ
		// ÊÍÖíÑ ãÓÇÑ ÌÏíÏ			
		rte.dst = destination;
		rte.f_nxt_hop  = packet_sender;
		rte.s_nxt_hop  = first_hop;
		rte.metric =number_of_hops+1;
		rte.link_no = link_no;
		rte.advert_seqnum = false;	 //ahmad
		rte.advert_metric = false;	 //ahmad
		rte.advertise = true;  //   íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ
		rte.changed_at = now;  
		////cout  << "\t\t .... according to the algorithm the entry is changed to  " << rte.dst << "    " << rte.f_nxt_hop  << "    " << rte.s_nxt_hop  << "    " << rte.metric << "    " << rte.link_no;

		if (myaddr_ != rte.dst)
		{
			xx =1;
			int write = Update_Route(NULL, &rte,xx);//ÅÖÇİÉ åĞÇ ÇáãÓÇÑ ãÚ ÇáãÍÇİÙÉ Úáì ÇáãÓÇÑÇÊ ÇáÃİÖá æÇáãÓÇÑÇÊ ÇáãÊÚÏÏÉ

			if (write == 1)
				modify_rt++;      // ÚÏÏ ÇáãÓÇÑÇÊ ÇáÊí ÊãÊ ÇÖÇİÊåÇ Ãæ ÊÚÏíáåÇ
		}		
        //============================================================================================  added
        prte = NULL;
		int num_of_q_packets = 0;  //added by ahmad alali áãÚÑİÉ ÚÏÏ ÇáÑÒã ÇáÊí Êã ÊÎÒíäåÇ                 
        
		// áäÑì Çä ßÇä ÈÇáÇãßÇä ÇÑÓÇá ÑÒã ãÎÒäÉ İí ãÓÇÑ ãÈÇÔÑ áÃäå ÊÎÒä ÇáÑÒã İí ÇáãÓÇÑ ÇáãÈÇÔÑ İŞØ
		if (rte.dst!= rte.f_nxt_hop )   // äÍä äÊÚÇãá ãÚ ãÓÇÑ ÛíÑ ãÈÇÔÑ
		{
			prte= table_->GetEntry1 (rte.dst, rte.dst,&myaddr_);  // ÇíÌÇÏ ãÓÇÑ ãÈÇÔÑ															 
			if (prte)
                bcopy(prte, &rte,sizeof(edvr_rtable_ent));	// ßÊÇÈÉ ÇáãÓÇÑ ÇáãÈÇÔÑ ÈÌÏæá ÇáÊæÌíå
		}

		if(prte)
			if (rte.q )    // äİÍÕ Åä ßÇä íæÌÏ ÑÒã ãÎÒäÉ İí ÇáãÓÇÑ ÇáãÈÇÔÑ ÍÊì áæ Ãä åĞÇ ÇáãÓÇÑ ÇáãÈÇÔÑ ãŞØæÚ
			{	            // Ãäå ÍÕáäÇ Úáì ãÓÇÑ ÌÏíÏ áåĞÇ ÇáåÏİ áÃäå åäÇ áÏíäÇ ãÓÇÑÇÊ ãÊÚÏÏÉ ÍíË íãßä Ãä äÓÊÎÏã ÇáãÓÇÑ ÇáÍÇáí áÇÑÓÇá ÇáÑÒã ÇáãÎÒäÉ
				////cout << "\n In receive_update() function .... Node (" << myaddr_ << ") is calling recv(queued_p, 0) ..... to give the packets to ourselves to forward";


				Packet *queued_p;
				while ((queued_p = rte.q->deque()))
				{
					num_of_q_packets++;   //added by ahmad alali
					// XXX possible loop here  
					// while ((queued_p = rte.q->deque()))
					// Only retry once to avoid looping
					// for (int jj = 0; jj < rte.q->length(); jj++){
					//  queued_p = rte.q->deque();
					recv(queued_p, 0); // ÇÚØÇÁ ÇáÑÒã ááÚŞÏÉ ÇáÍÇáíÉ áÊãÑÑåÇ
				}

				//cout << "\n\n In receive_full routing information pkt function .... Number of packets that were queued  is  " << num_of_q_packets << "\n";
	 
				//==============================================================   // added by ahmad
				// The next 4 statements are used to deque the entry that has been qued to be send...
				// The second statement (rte.q = 0;) and last statment (table_->AddEntry (rte,ahmad); ) to set q field in te routing table to 0.
				// =============================================================
				delete rte.q;	
				rte.q = 0;
				nsaddr_t *ahmad = &myaddr_;

				
				////cout<< "\n In receive_full routing information pkt function .... Node (" << myaddr_ << ") is going to reset the quee the following entry ...  " << rte.dst << "\t" << rte.f_nxt_hop << "\t" << rte.metric<<"\t" << rte.seqnum << "\t" << rte.new_seqnum_at<< "\t" << rte.wst<<" by reseting the q field\n";   // added by ahmad
				//table_->AddEntry(rte,ahmad);        // added by ahmad
				bcopy(&rte, prte,sizeof(edvr_rtable_ent));
			}
		change_count--;		
	}
	
	if (modify_rt==0)		// ÅĞÇ áã íÊã ÊÚÏíá ÌÏæá ÇáÊæÌíÚ
	{
		////cout<<"\n Node (" << myaddr_ << ") didn't updates its routing table. So, it is going to drop this UPDATE_PACKET..... ";
	   // drop(p, DROP_RTR_NOT_NEEDED);   // ÇåãÇá åĞå ÇáÑÒãÉ áÃäå áã íÊÚã ÊÚÏíá ÌÏæá ÇáÊæÌíå
		return;    
	}
	//else	
		////cout<< "\n The Routing table has been updated regarding to the last UPDATE_PACKET, and I've got " << modify_rt <<" new entries\n"; 

	////cout << "\n Routing table of node ( " << myaddr_ << " )  consists of " << table_->number_of_elts() << " entries after deahmadng with the received UpdatePacket .... \n";
	////table_->Show_rt();  //
}	//****************************************  äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÇáÊÍÏíË  **********************************************
void	
EDVR_Agent::Receive_RREQ_Packet (Packet *p)
{	//============================ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ØáÈ ÇáãÓÇÑ  ================================
	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock();  //ahmad
	unsigned char *d = p->accessdata ();     // d ãÄÔÑ áÈÏÇíÉ ÑÒãÉ ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáÊí åí ÇáÚŞÏÉ ÇáØÇáÈÉ
	unsigned char *w = d ;    // w ãÄÔÑ ááÈÇíÊ ÇáÊÇáí ÈÇáÑÒãÉ æÇáĞí åæ ÇáÚŞÏÉ ÇáåÏİ ÇáÊí ÃÑÓá ãä ÃÌáåÇ ÑÒãÉ ØáÈ ÇáãÓÇÑ

	edvr_rtable_ent rte;		// ÇáãÓÇÑ ÇáÌÏíÏ ÇáĞí ÓíÊã ÊÚáãå
	edvr_rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ ÈÌÏæá ÇáÊæÌíå
	bzero(&rte, sizeof(rte));
	edvr_rtable_ent *pr2;
	
	int drop_it =0;
	if(hdri->ttl_ == 0)
		drop_it=1;


	prte = NULL;
    int sender=Address::instance().get_nodeaddr(hdri->saddr());//ÇáÚŞÏÉ ÇáãÑÓáÉ
	int previous_node=hdrc->prev_hop_;  //ÇáÚŞÏÉ ÇáÓÇÈŞÉ
    int p_previous_node=hdrc->p_prev_hop_;// ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ 
    int first_hop_node=hdrc->fst_fwd_hop_;// Ãæá ÚŞÏÉ ãÑÑÊ åĞå ÇáÑÒãÉ

	/*//cout<<"\n these information at receive RREQ .........previous_node="<<hdrc->prev_hop_;  
	//cout<<"\n these information at receive RREQ .........p_previous_node="<<hdrc->p_prev_hop_; 
    //cout<<"\n these information at receive RREQ .........first_hop_node="<<hdrc->fst_fwd_hop_;
    //cout<<"\n these information at receive RREQ .........No of forwarss="<<hdrc->num_forwards();*/
	// extracting data from the Error packet....
	//int flag = 0;
	int Found_before=0;	 // áãÚÑİÉ Åä ßÇäÊ ÇáÑÒãÉ ãÓÊŞÈáÉ ãÓÈŞÇğ Ãã áÇ 
    //========================
    int packet_requester  = *(w++);// ÇáÚŞÏÉ ÇáØÇáÈÉ ÇáÊí ÃÑÓáÊ ØáÈ ÇáãÓÇÑ
		packet_requester  = packet_requester  << 8 | *(w++);
		packet_requester  = packet_requester  << 8 | *(w++);
		packet_requester  = packet_requester << 8 | *(w++);
			
	int	destination = *(w++);//ÇáÚŞÏÉ ÇáåÏİ
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);

	   
		//cout << "\n Node (" << myaddr_ << ") received a Route Requist Packet...from Node (" << packet_requester<< ")    Looking for Node (" << destination << ") Previous (" << hdrc->prev_hop_ << ") Packet id (" << hdrc->uid()<<")";

	
	int expected_hops=hdrc->num_forwards_;      //ÚÏÏ ÇáŞİÒÇÊ ÇáãÊæŞÚÉ = max TTL (30)-ttl_
	// //cout<< "\n At the begining of Receive Route requist................\n";
	// //cout << "\n Node (" << myaddr_ << ") ***received a Route Requist message...from Node*** (     " << previous_node<< "     )*** looking for Node** (     " << destination <<"    )***  for this Node ("<<packet_requester<< ") No of forwards="<<expected_hops;
    
	if (myaddr_ == sender ||myaddr_ == packet_requester ||myaddr_ == previous_node||myaddr_ == p_previous_node)		// ÅĞÇ ßÇäÊ ÇáÚŞÏÉ ÇáÍÇáíÉ åí ãÑÓá åĞå ÇáÑÒãÉ
	{
		//cout << "\n I am the sender or I've rebroadcastes this Route requist packet.... So, I have to drop it because of (loop).....  myaddr == packet sender......";
		 //Packet::free(p);	// do nothing and free the packet...
	    drop(p, DROP_RTR_ROUTE_LOOP);   // ÇåãÇá ÇáÑÒãÉ áÇäå ÇäÇ ÇáÚŞÏÉ ÇáãÑÓáÉ áåĞå ÇáÑÒãÉ
		return;
	}	
    int Yes_it_is_a_neighbor= check_Neighbor_Process(hdrc->prev_hop_);		//  İÍÕ ÇáÚŞÏÉ ÇáÓÇÈŞÉ Åä ßÇäÊ ÌÇÑ ãÚÑæİ ãÓÈŞÇ Ãã ÌÇÑ ÌÏíÏ

	   // áäÑì Çä ßÇäÊ åĞå ÇáÑÒãÉ ãÓÊŞÈáÉ ãä ŞÈá 
	 for (table_->InitLoop (); (pr2 = table_->NextLoop ()); )	// ÇáÈÍË Öãä ÌÏæá ÇáÊæÌíå
	 {
			if(pr2->dst == packet_requester )  // ÅĞÇ ßÇä áÏí ãÓÇÑ áãÑÓá ÑÒãÉ ØáÈ ÇáãÓÇÑ.
			{
				////cout<<"\n I found Route to this Packet requester "<<	pr2->dst<<" ="<< packet_requester<<" in my routing table and its packet_id =="<<pr2->packet_id;
			   if(pr2->packet_id ==	hdrc->uid())	// ÇÓÊŞÈáÊ åĞå ÇáÑÒãÉ ãÓÈŞÇ
			   {  ////cout<<"\n I've received this RREQ Before .."<< pr2->packet_id;
				   Found_before=1;	// åĞå ÇáÑÒãÉ ãÓÊŞÈáÉ ãä ŞÈá
				   break;
			   }
			}
	 }	
	//=======================================================================================================
   // ÇáÍÕæá Úáì ãÓÇÑ áãÑÓá ÑÒãÉ ØáÈ ÇáãÓÇÑ
	    
		rte.dst = packet_requester;//ÚäæÇä ÇáÚŞÏÉ ÇáåÏİ åæ ãÑÓá ÑÒãÉ ØáÈ ÇáãÓÇÑ
		rte.f_nxt_hop  = previous_node;//ÚäæÇä ÇáŞİÒÉ ÇáÃæáì = ÚäæÇä ÇáŞİÒÉ ÇáÓÇÈŞÉ
		rte.s_nxt_hop  =p_previous_node;//ÚäæÇä ÇáŞİÒÉ ÇáËÇäíÉ= ÚäæÇä ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ		 // added by ahmad
		rte.metric =expected_hops;  //ÚÏÏ ÇáŞİÒÇÊ = ÚÏÏ ÇáŞİÒÇÊ ÇáãÊæŞÚÉ   
		                                
		
		////cout<<"\n Information ..... packet_requester= "<<packet_requester<<"   previous="<<previous_node<<"   p_previous_node= "<<p_previous_node<<"    first hop node= "<<first_hop_node;
		
		//rte.seqnum = 0;
		  if(first_hop_node==-99)//ÅĞÇ ßÇäÊ Ãæá ÚŞÏÉ ãÑÑÊ ÑÒãÉ ØáÈ ÇáãÓÇÑ áÇÔíÆ
            rte.link_no = myaddr_*10000+packet_requester;  // ÑŞã ÇáæÕáÉ ÈåĞå ÇáÍÇáÉ
		  else
		    rte.link_no = first_hop_node*10000+packet_requester;  // ÑŞã ÇáæÕáÉ ÈåĞå ÇáÍÇáÉ
		//rte.advertise_ok_at = now + 604800;	// check back next week... :)
		rte.advert_seqnum = false;	 //ahmad
		rte.advert_metric = false;	 //ahmad
		rte.packet_id =	hdrc ->uid();		 //Packet id
		rte.advertise = true;  // ÇáÅÔÇÑÉ Åáì Ãäå íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ
		rte.changed_at = now;  		
		nsaddr_t *myadress = &myaddr_;
	
 int f_nxt_hop ;
 int s_nxt_hop ;
 int metric;
 int link_no;

   
		//cout<< "\n In Route requist receive... Node (" << myaddr_ << ") try to add an entry to Packet sender node " << packet_requester << ". So, it calls AddEntry() function... to add the following entry:  " << rte.dst << "   " << rte.f_nxt_hop  << "   " << rte.s_nxt_hop << "   " << rte.metric << "   " << rte.link_no << "   " << rte.q << "\n";   // added by ahmad 
		table_->AddEntry (rte,&myaddr_);//ÅÖÇİÉ ÇáãÓÇÑ áÌÏæá ÇáÊæÌíå	 // this entry could be added, could be not if availble before
        prte = NULL; 
		prte = table_->GetEntry2 (packet_requester,-99,&myaddr_);  //ÇáÍÕæá Úáì ÃİÖá ãÓÇÑ ááÚŞÏÉ ÇáØÇáÈÉ ÈÛÖ ÇáäÙÑ Úä ÚäæÇä ÇáŞİÒÉ ÇáÃæáì  // -99 
		 if(prte && prte->packet_id !=hdrc ->uid())
					prte->packet_id=hdrc ->uid();//ÊÚÏíá ÑŞã ÇáÑÒãÉ ááãÓÇÑ

		               
	
			
	int Need_to_rebroadcast_the_same_RREQ =1;  //int Flags=1;   
    if (destination ==myaddr_ )		// ÅĞÇ ßÇäÊ ÇáÚŞÏÉ ÇáÍÇáíÉ åí ÇáÚŞÏÉ ÇáåÏİ
	{
        ////cout << "\n \n Node (" << myaddr_ << ") .. The destination that Node ("<<sender <<") looking for is Me ("<<destination<<")\n";
        // //cout << "\n Node (" << myaddr_ << ") I've received a route requist from ***  "<<packet_requester <<" looking for myself... "<<destination<<"\n";
		 Need_to_rebroadcast_the_same_RREQ=0;  //áÇ íæÌÏ ÏÇÚí áÅÚÇÏÉ äÔÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
         f_nxt_hop =-99;
         s_nxt_hop =-99;
         metric=0;
		 link_no=myaddr_*10000+previous_node;  // ÑŞã ÇáæÕáÉ ÇáÊí ÓÊÖãä ÈÑÒãÉ ÑÏ ÇáãÓÇÑ
		  // ÇáÇäÊŞÇá áÇäÔÇÁ ÑÒãÉ ÑÏ ÇáãÓÇÑ Need_to_rebroadcast_the_same_RREQ (Flags)=1	
    }
	else  
		//for (table_->InitLoop (); (pr2 = table_->NextLoop ()); )	// go through my routing table......
		   prte = NULL; 
		   prte = table_->GetEntry2 (destination,-99,&myaddr_);  //ahmad  // ÇáÍÕæá Úáì ãÓÇÑ ááÚŞÏÉ ÇáåÏİ ÈÛÖ ÇáäÙÑ Úä ÚäæÇä ÇáŞİÒÉ ÇáÃæáì
			if(prte && prte->metric != BIG)//İí ÍÇá æÌæÏ ãÓÇÑ æåĞÇ ÇáãÓÇÑ ÛíÑ ãŞØæÚ
			//if ((pr2->dst == destination ) && pr2->metric != BIG)  // if I reach any destination through the error packet sender...assign this rout as a bad route
			{ 
				Need_to_rebroadcast_the_same_RREQ =0;   //áÇ íæÌÏ ÏÇÚí áÅÚÇÏÉ äÔÑ åĞå ÇáÑÒãÉ
                ////cout << "\n Node (" << myaddr_ << ") I found route to this node ... "<<destination <<"    in my routing table ";
				////cout<< "\n path="<<prte->dst<<"  "<<prte->f_nxt_hop <<"  "<<prte->s_nxt_hop <<"  "<<prte->metric<<"  "<<prte->link_no;
			    f_nxt_hop =prte->f_nxt_hop ;//ÚäæÇä Ãæá ŞİÒÉ äİÓå
				s_nxt_hop =prte->s_nxt_hop ;//ÚäæÇä ËÇäí ŞİÒÉ äİÓå
				metric=prte->metric;//ÚÏÏ ÇáŞİÒÇÊ äİÓå
				link_no=prte->link_no;//ÑŞã ÇáæÕáÉ äİÓå
			    //  ÇáÇäÊŞÇá áÇäÔÇÁ ÑÒãÉ ÑÏ ÇáãÓÇÑ  Need_to_rebroadcast_the_same_RREQ (Flags)=1	
             // break;
			}
 		
 	if (/*Flags==1*/ Need_to_rebroadcast_the_same_RREQ && Found_before==0 && drop_it==0)	// áÇ íæÌÏ ãÓÇÑ ááÚŞÏÉ ÇáåÏİ áĞÇ íÌÈ ÅÚÇÏÉ äÔÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
	{
		   // ÇäÇ ÇÑíÏ Ãä ÇÚíÏ äÔÑ åĞå ÇáÑÒãÉ ãÑÉ ÃÎÑì áÇäå áã ÇÌÏ ãÓÇÑ ááÚŞÏÉ ÇáØÇáÈÉ

		////cout << "\n\n Routing table of node ( " << myaddr_ << " ) after dealing with the Route Requist PACKET .........  "  << " \n"  ;
		////table_->Show_rt();

		////cout<<"\n Node ( " << myaddr_ << " ) Rebrodcasted the ROUTE REQUIST PACKET,  because No Route available in its RT to this Destination ("<<destination <<") \n";       
        ////cout<<"\n\n\n\n\n *****";
        ////cout << "\n ***** At  " << now << "  node  " << myaddr_ << " Rebroadcasted the Route Requist Packet, No Route available "<<hdrc ->uid();
		      
       if(hdrc->fst_fwd_hop_==-99)
	       hdrc->fst_fwd_hop_ = myaddr_; //ÚäæÇä ÇáÚŞÏÉ ÇáÍÇáíÉ åæ Ãæá ÚäæÇä íãÑÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
	   
	   hdrc->p_prev_hop_ = hdrc->prev_hop_; //ÚäæÇä ÇáÚŞÏÉ ãÇ ŞÈá ÇáÓÇÈŞÉ åæ ÚäæÇä ÇáÚŞÏÉ ÇáÓÇÈŞÉ
 
       sendOutBCastPkt1(p);	 //ÇÓÊÏÚÇÁ ÚãáíÉ ÇáäÔÑ áäÔÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ

	   if (verbose_)	   
	       trace ("VBP %.5f _%d_ %d:%d -> %d:%d", now, myaddr_,hdri->saddr(), hdri->sport(), hdri->daddr(), hdri->dport());
	  
	}  
	
	if (/*Flags==0*/ !Need_to_rebroadcast_the_same_RREQ && Found_before==0)
	{
		//ÅäÔÇÁ ÑÒãÉ ÑÏ ÇáãÓÇÑ
		    Packet *p1= Make_RRP_Packet(packet_requester,destination,f_nxt_hop ,s_nxt_hop ,metric,link_no);  // ahmad
	        if(p1)
			   Forward_Packet_Process(p1); //ÊãÑíÑ åĞå ÇáÑÒãÉ
			else
			{
				 ////cout<<"\n ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRor \n";
				 abort();
			}
		
		Packet::free(p);
		//drop(p, DROP_RTR_NOT_NEEDED);   // added 
        ////cout<<"\n\n Node ( " << myaddr_ << " ) Has freed the ROUTE REQUIST Packet because it sent a ROUTE REPLAY ..\n";
		return;
	}
  if(drop_it)          //***************åäÇ ÇáÚŞÏÉ ÊİÍÕ Ãä ßÇäÊ ŞíãÉ Òãä ÍíÇÉ ÇáÑÒãÉ åæ ÕİÑ Êåãá åĞå ÇáÑÒãÉ
  {
	 ////cout << "\n Node  " << myaddr_ << "  drops the Packet because (src != myaddr_ && hdrc->num_forwards() != 0) && hdri->ttl_ == 0 .....";
	 ////cout<<"\n  Packet Type............  = "<<hdrc->ptype(); 
     ////cout<<"\n  Packet No..............  = "<<hdrc->uid(); 		
	drop(p, DROP_RTR_TTL);
		 
  }
return;
}//================== äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÑÏ ÇáãÓÇÑ  ===================

void	
EDVR_Agent::receive_RREP_Packet(Packet * p)
{//==================== ÈÏÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÑÏ ÇáãÓÇÑ ==================
	
	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);

	Scheduler & s = Scheduler::instance ();
	double now = s.clock();  //ahmad
	unsigned char *w = p->accessdata ();     // d ãÄÔÑ áÈÏÇíÉ ÇáÈíÇäÇÊ ÈÇáÑÒãÉ æÇáĞí åæ ÇáÚŞÏÉ ÇáãÑÓá (ÇáÚŞÏÉ ÇáåÏİ ÇáĞí ÇÑÓáÊ ãä ÃÌáåÇ ÑÒãÉ ÑÏ ÇáãÓÇÑ
	//unsigned char *w = d ;    // w ãÄÔÑ ááÈÇíÊ ÇáÊÇáí ãä ÇáÑÒãÉ æÇáĞí åæ ÇáåÏİ áåÏå ÇáÑÒãÉ (ÇáÚŞÏÉ ÇáÊí ÃÑÓáÊ ÑÒãÉ ØáÈ ÇáãÓÇÑ

	edvr_rtable_ent rte;		// new rte learned from update being processed
	edvr_rtable_ent *prte;		// ptr to entry *in* routing tbl
	bzero(&rte, sizeof(rte));
	edvr_rtable_ent *pr2;

	prte = NULL;
    int sender=Address::instance().get_nodeaddr(hdri->saddr());//ÇáÚŞÏÉ ÇáãÑÓáÉ áÑÒãÉ ÑÏ ÇáãÓÇÑ
	int sent_to=Address::instance().get_nodeaddr(hdri->daddr());//ÇáÚŞÏÉ ÇáåÏİ ÇáãÑÓáÉ áåÇ ÑÒãÉ ÑÏ ÇáãÓÇÑ
	int previous_node=hdrc->prev_hop_;  //ÇáÚŞÏÉ ÇáÓÇÈŞÉ   

			
	int	destination = *(w++);//ÇáÚŞÏÉ ÇáåÏİ
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);
		destination = destination << 8 | *(w++);

    int	f_nxt_hop = *(w++);//ÇáŞİÒÉ ÇáÃæáì ÈÇÊÌÇå ÇáåÏİ
		f_nxt_hop  = f_nxt_hop  << 8 | *(w++);
		f_nxt_hop  = f_nxt_hop  << 8 | *(w++);
		f_nxt_hop  = f_nxt_hop  << 8 | *(w++);

	int	s_nxt_hop = *(w++);//ÇáŞİÒÉ ÇáËÇäíÉ ÈÇÊÌÇå ÇáåÏİ
		s_nxt_hop  = s_nxt_hop  << 8 | *(w++);
		s_nxt_hop  = s_nxt_hop  << 8 | *(w++);
		s_nxt_hop  = s_nxt_hop  << 8 | *(w++);
	
	int metric = *(w++);//ÚÏÏ ÇáŞİÒÇÊ

	int	link_no = *(w++);//ÑŞã ÇáæÕáÉ
		link_no = link_no << 8 | *(w++);
		link_no = link_no << 8 | *(w++);
		link_no = link_no << 8 | *(w++); 

		////cout << "\n Node (" << myaddr_ << ") received a Route Replay Packet...from Node:  " << sender<< "  Send to Node: "<< sent_to<< " About Found Node:   " << destination << "   where previous Node: =" << previous_node << "  Packet No:  =" << hdrc->uid()<<"   No of forward: =" <<hdrc->num_forwards_ <<"   No of TTl: =" <<hdri->ttl_;    

	
	if (verbose_)
	{
		trace ("VTO %.5f _%d_ %d->%d", now, myaddr_, myaddr_, prte->dst);
		
	}
        //  ÇİÍÕ Òãä ÍíÇÉ ÇáÑÒãÉ ÅĞÇ ßÇä ÕİÑ Çåãá ÇáÑÒãÉ
     ////cout<<" \n time to leav is = "<<hdri->ttl_ <<"\n";

	if(--hdri->ttl_ == 0)          //***************ÊİÍÕ åäÇ ÇäåÇ áíÓÊ ÇáÚŞÏÉ ÇáãÕÏÑ æÒãä ÍíÇÉ ÇáÑÒãÉ ÕİÑ
	 {
		Time now = Scheduler::instance().clock();   // added by ahmad alali  
		////cout << "\n Node  " << myaddr_ << "  drops the Packet because (src != myaddr_ && hdrc->num_forwards() != 0) && hdri->ttl_ == 0 .....";
		drop(p, DROP_RTR_TTL);//ÇåãÇá ÇáÑÒãÉ áÇäÊåÇÁ Òãä ÇáÍíÇÉ
		return;
	 }
		
	  int Yes_it_is_a_neighbor= check_Neighbor_Process(hdrc->prev_hop_);		//  İÍÕ ãÑÓá ÇáÑÒãÉ Åä ßÇä ÌÇÑ ÌÏíÏ Ãæ ÌÇÑ ãÚÑæİ ãÓÈŞÇğ
		
  			//=======================================================================================================
			//                     Gathering Routing Information from this Packet for future .............
			//=======================================================================================================
   
	    // ÇáÍÕæá Úáì ãÓÇÑ ááÚŞÏÉ ÇáåÏİ ÚäÏ ÇáÚŞÏÉ ÇáÍÇáíÉ
		
	    rte.dst=destination ;
        rte.f_nxt_hop =previous_node;
        rte.s_nxt_hop =f_nxt_hop ;
        rte.metric=metric+1;
		rte.link_no =link_no ;
		rte.q=0 ;
   
		rte.seqnum = 0;
		rte.advert_seqnum = false;	 //ahmad
		rte.advert_metric = false;	 //ahmad
		rte.advertise = true;  // ÇáÅÔÇÑÉ Ãäå íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ
		rte.changed_at = now;   
		rte.wst = wst0_;
        if (rte.metric == 1 && rte.dst == rte.f_nxt_hop )    // ááÊÃßÏ Ãä åĞÇ ÇáãÓÇÑ áÚŞÏÉ ÌÇÑ
			{
				//1- ÌÏæáÉ Òãä ÇáÇäÊåÇÁ áåĞÇ ÇáÌÇÑ....
				////cout<< " \n ....This node:  "<<rte.dst <<"  .. is considered as a new neighbour...........\n ";
				rte.timeout_event = new Event ();//ÌÏæáÉ Òãä ÇáÇäÊåÇÁ áåĞÇ ÇáÌÇÑ
				s.schedule (helper_, rte.timeout_event, min_update_periods_ * perup_);
			}
	
        ////cout << "\n ... For getting Routing information from any packet pass through,  Node (" << myaddr_ << ") has changed the entry to  \n";
	    //cout << rte.dst << "   " << rte.f_nxt_hop   << "  " << rte.s_nxt_hop  << "   " << rte.metric << "   " << rte.link_no << " for itself \n";			
			
			
		if (sent_to !=myaddr_ )		// ÇáÚŞÏÉ ÇáÍÇáíÉ áíÓÊ ÇáÚŞÏÉ ÇáåÏİ
		{//ÇáÚŞÏÉ ÇáÍÇáíÉ íÌÈ Ãä ÊÚíÏ ÇÑÓÇá ÑÒãÉ ÑÏ ÇáãÓÇÑ
			////cout << "\n \n Because this Packet is sent to another Node: " << sent_to << " Therefore; \n";
			////cout << "I need to make and forward a new Route Repaly Packet to the same destinatin  Node: "<<sent_to << "  according to the latest routing Information I gathered and free the received one (old)..\n";
			//ÊäÔÃ ÇáÚŞÏÉ ÇáÍÇáíÉ ÑÒãÉ ÑÏ ÇáãÓÇÑ 
				Packet *p1= Make_RRP_Packet(sent_to,rte.dst,rte.f_nxt_hop ,rte.s_nxt_hop ,rte.metric,rte.link_no);  // ahmad
				if(p1)
				Forward_Packet_Process(p1); //ÊãÑíÑ åĞå ÇáÑÒãÉ
				else
				{
					//cout<<"\n EEERROR \n";
					abort();
				}  
		}
	
	  
	////cout<< "\n In Route replay receive... Node (" << myaddr_ << ") try to add an entry to node " << destination  << ". So, it calls AddEntry() function... to add the following entry:  " << rte.dst << "   " << rte.f_nxt_hop  << "   " << rte.s_nxt_hop << "   " << rte.metric << "   " << rte.link_no << "   " << rte.q <<"\n";   // added by ahmad 
	table_->AddEntry (rte,&myaddr_);      //ÅÖÇİÉ ÇáãÓÇÑ ÈÌÏæá ÇáÊæÌíå            // added by ahmad
	
	 //İí åĞÇ ÇáÌÒÁ ÊÃßÏäÇ Ãä åĞå ÇáÑÒãÉ áí áäÑì Åä ßÇä ÈÇáÇãßÇä Ãä äÑÓá Ãí ÑÒã ãÎÒäÉ İí ÇáÑÊá İí ãÓÇÑ ãÈÇÔÑ áÃä ÇáÑÒã ÊÎÒä İŞØ ÈÇáãÓÇÑ ÇáãÈÇÔÑ
	prte = NULL;
	int num_of_q_packets = 0;  //added by ahmad alali  áãÚÑİÉ ÚÏÏ ÇáÑÒã ÇáÊí Êã ÊÎÒíäåÇ ÈÇáÑÊá                
        
		
	prte= table_->GetEntry1 (rte.dst, rte.dst,&myaddr_);  // äİÍÕ Åä ßÇä íæÌÏ ãÓÇÑ ãÈÇÔÑ ááÚŞÏÉ ÇáåÏİ 																 
		if (prte)
           bcopy(prte, &rte,sizeof(edvr_rtable_ent));	   //rte Åáì ÇáãÓÇÑ  ptre äÓÎ ÌãíÚ ÍŞæá ÇáãÄÔÑ 
		
		if(prte)
			if (rte.q )    // äİÍÕ Çä ßÇä íæÌÏ ÑÒã ãÎÒäÉ İí åĞÇ ÇáãÓÇÑ ÇáãÈÇÔÑ ÍÊì áæ Çäå ãŞØæÚ áÇÑÓÇá åĞå ÇáÑÒã
			{	            //áÇäå ÍÕáäÇ Úáì ãÓÇÑ íãßä ÇÓÊÎÏÇãå İí ÇÑÓÇá ÑÒã ÇáÈíÇäÇÊ ÇáãÎÒäÉ
				//cout << "\n In receive Route Replay() function .... Node (" << myaddr_ << ") is calling recv(queued_p, 0) ..... to give the packets to ourselves to forward";

				Packet *queued_p;
				while ((queued_p = rte.q->deque()))
				{
					num_of_q_packets++;   //added by ahmad alali
					recv(queued_p, 0); // ÇÓÊŞÈÇá åĞå ÇáÑÒãÉ
				}

				////cout << "\n\n In receive Route Replay()  function .... Number of packets that were queued  is  " << num_of_q_packets << "\n";
	 
				//==============================================================   // added by ahmad
				// The next 4 statements are used to deque the entry that has been qued to be send...
				// The second statement (rte.q = 0;) and last statment bcopy(&rte, prte,sizeof(edvr_rtable_ent)); to set q field in te routing table to 0.
				// =============================================================
				delete rte.q;	
				rte.q = 0;
				rte.packet_id=0;   //áÅÚÇÏÉ ÖÈØ åĞÇ ÇáÍŞá áÅÚØÇÁ ÇáÚŞÏÉ İÑÕÉ ÅÑÓÇá ÑÒãÉ ØáÈ ÇáãÓÇÑ ÈÍÇáÉ İÔá åĞÇ ÇáãÓÇÑ
				////cout<< "\n In receive Route Replay()  function .... Node (" << myaddr_ << ") is going to reset the quee the following entry ...  " << rte.dst << "\t" << rte.f_nxt_hop << "\t" << rte.metric<<"\t" << rte.seqnum << "\t" << rte.new_seqnum_at<< "\t" << rte.wst<<" by reseting the q field\n";   // added by ahmad
				bcopy(&rte, prte,sizeof(edvr_rtable_ent));   //  ptre Åáì ÇáãÄÔÑ   rte äÓÎ ÌãíÚ ÍŞæá ÇáãÓÇÑ
			}
			
		////cout << "\n\n Routing table of node ( " << myaddr_ << " )  consists of " << table_->number_of_elts() << " entries after dealing with the Route Replay .... \n";
		////table_->Show_rt();

    Packet::free(p); // ÊÍÑíÑ ÇáÑÒãÉ ÇáãÓÊŞÈáÉ Úáì ÃíÉ ÍÇá

}	//================= äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÑÏ ÇáãÓÇÑ  =============
void 
EDVR_Agent::Broken_Route_Process (int destination, int first_hop , int second_hop ,int link_no)					
{	//========================================= ( ÈÏÇíÉ ÚãáíÉ ÑÓÇáÉ ÇáÎØÃ) ========================================= 
	
	
	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();

	edvr_rtable_ent *pr2;
	//int xx;
	//nsaddr_t src= hdrc->prev_hop_;	 //address of the previous packet sender   ahmad
	//cout <<"\n u r in the Broken_Route_Process function and this is the broken route ";
	////cout <<"\n destination  "<<destination << "   first_hop   "<<first_hop  << "  second_hop  "<<second_hop<<"  link_no " <<link_no  <<"\n";
	////table_->Show_rt();
	//======================================================================

		for (table_->InitLoop (); (pr2 = table_->NextLoop ()); )	// ÇáãÑæÑ Úáì ÌÏæá ÇáÊæÌíå
		{
			
			if ((pr2->link_no  == link_no || (pr2->f_nxt_hop  == first_hop  && pr2->s_nxt_hop  ==second_hop )) && pr2->metric != BIG)  // ÅĞÇ æÕáÊ ááåÏİ ÚÈÑ ÑŞã ÇáæÕáÉ İí ÑÒãÉ ÇáÎØÃ
			{//Ãí ãÓÇÑ áå äİÓ ÑŞã ÇáæÕáÉ ÇáãŞØæÚÉ Ãæ ÇáŞİÒÉ ÇáÇæáì ÈÇÊÌÇå ÇáåİÏ åæ ãÑÓá ÑÒãÉ ÇáÊÍÏíË íÊã ÇáÇÔÇÑÉ áå Ãäå ÛíÑ ŞÇÈá ááæÕæá 
				//Flags=1;
				if (verbose_)
					trace ("VTO %.5f _%d_ marking %d", now, myaddr_, pr2->dst);

				if (pr2->metric == 1)	// íÌÈ Úáí Ãä ÇßÊÔİ ÇáãÓÇÑ ÇáãŞØæÚ ááÚŞÏÉ ÇáÌÇÑÉ ÈäİÓí.
				{
					//if (pr2->timeout_event)
					//	Scheduler::instance ().cancel (pr2->timeout_event);

					//pr2->timeout_event = 0;  
					continue;	
				}

				pr2->metric = BIG;     // ÇáÇÔÇÑÉ áåĞÇ ÇáãÓÇÑ Ãäå ãŞØæÚ
				pr2->changed_at = now;
				if(pr2->link_no  == link_no )
				    pr2->advertise=true;	   // íÌÈ Úáí äÔÑ ÇáãÓÇÑ ÅĞÇ ßÇä áå äİÓ ÑŞã ÇáæÕáÉ İŞØ
			}
		}
	
  ////table_->Show_rt();
 return;
}//****************************************************  äåÇíÉ ÚãáíÉ ÑÓÇáÉ ÇáÎØÃ  *********************************************************
//ÚäÏãÇ ÊÓÊŞÈá ÇáÚŞÏÉ ÑÒãÉ ÊÍßã ÊİÍÕ Åä ßÇäÊ ÇáÚŞÏÉ ÇáÓÇÈŞÉ ÇáÊí ãÑÑÊ åĞå ÇáÑÒãÉ åá åí ÌÇÑ ãÚÑæİ ãÓÈŞÇğ ßÇä ãä Çáããßä ÇáæÕæá áå Ãã ÌÇÑ ãÚÑæİ ãÓÈŞÇğ ßÇä ãä ÛíÑ Çáããßä ÇáæÕæá áå Ãã ÌÇÑ ÌÏíÏ
int
EDVR_Agent::check_Neighbor_Process(nsaddr_t src)
 {//****************************************************  ÈÏÇíÉ ÚãáíÉ İÍÕ ÇáÌÇÑ  *********************************************************
	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();
	edvr_rtable_ent *prte;		// ãÄÔÑ áãÓÇÑ ÈÌÏæá ÇáÊæÌíå
	Packet *p;
	prte = NULL; 
    prte = table_->GetEntry1 (src, src,&myaddr_);  //  İÍÕ Åä ßÇäÊ ÇáÚŞÏÉ åí ÌÇÑ
 	int is_it_a_neighbor=0;
	if (prte && prte->metric == 1)	 //	 ãÑÓá ÇáÑÒãÉ ÌÇÑ ãÚÑæİ ãÓÈŞÇ
	{	   
		    ////cout << "\n Node (" << myaddr_ << ") found an active route to node (" << src << ") and consider it as an old neighbour, the route is  " << prte->src << "   " << prte->f_nxt_hop  << "   "<< prte->s_nxt_hop  << "   "<< prte->metric << "   "<< prte->link_no << "    the timeout is rescheduled  " << prte->timeout_event->time_ ;
			Scheduler::instance ().cancel (prte->timeout_event);
			s.schedule (helper_, prte->timeout_event, min_update_periods_ * perup_);    // ÌÏæáÉ Òãä ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ	
			is_it_a_neighbor=1;
	}
	else   
	{	  
		if (prte && prte->metric == BIG)  
		   {					//åæ ÌÇÑ ÇáãÓÇÑ ßÇä áå ãŞØæÚ æÇÊì ãä ÌÏíÏ íÌÈ ÊÕáíÍ ÇáãÓÇÑ		
				////cout << "\n Node (" << myaddr_ << ") receives a Beacon message from an old broken neighbour which is node (" << src << "), it will activate the direct route and unicasts a full routing information pkt of its RT to the Beacon sender.";

				prte->timeout_event = new Event ();		// ÊÕáíÍ ÇáãÓÇÑ æÇÚÊÈÇÑå ãÓÇÑ ÌÏíÏ
				prte->metric = 1;
				prte->advert_seqnum = false;	 //ahmad
				prte->advert_metric = false;	 //ahmad
				prte->advertise = true;  // íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ İí ÑÒãÉ ÇáÊÍÏíË ÇáÊÇáíÉ
				prte->changed_at = now;  

				p = Make_Fulldump_Packet(1, src);         // ÊÑÓá ÇáÚŞÏÉ áåĞÇ ÇáÌÇÑ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ áÊÚáãå ÈÇáãÓÇÑÇÊ áÈŞíÉ ÇáÚŞÏ ÈÇáÔÈßÉ 

														
				if (p)
				{	
					
					assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
					 
					
				}
			   s.schedule (helper_, prte->timeout_event, min_update_periods_ * perup_);    // ÌÏæáÉ Òãä ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ
			}
		else
		   if (!prte)  // åĞÇ íÚäí Ãä ÇáÌÇÑ åæ ÌÇÑ ÌÏíÏ
		   {
			////cout << "\n Node (" << myaddr_ << ") receives a Beacon message from a new neighbour which is node (" << src << "), generates a link id, adds new entry, and unicasts a full routing information pkt of its RT to the Beacon sender.";
			p = Make_Fulldump_Packet(0, src);         // ÊÑÓá ÇáÚŞÏÉ áåĞÇ ÇáÌÇÑ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ áÊÚáãå ÈÇáãÓÇÑÇÊ áÈŞíÉ ÇáÚŞÏ ÈÇáÔÈßÉ 
										
			if (p)
			{
				assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
				// send out Route Update Packet jitter to avoid sync
				//DEBUG
				////cout<<"\n I've to send a full routing information pkt  \n";    stopped on 
				//s.schedule (target_, p, jitter(DSDV_BROADCAST_JITTER, be_random_));   // DSDV
					    
								
			}
		}
	}
	if (verbose_)
	{   // keep 
		trace ("VPC %.5f _%d_", now, myaddr_);
		tracepkt (p, now, myaddr_, "PU");
	}  
  if(prte)//Ãí Ãä ÇáãÓÇÑ ãŞØæÚ æÊã ÊÕáíÍå áĞÇ äİÍÕ ÇáÑÊá Åä ßÇä íæÌÏ ÑÒã ááåÏİ áÅÑÓÇáåÇ
	if (prte->q )    // 
	{	            // áÃäå ÍÕáäÇ Úáì ãÓÇÑ ááåÏİ íãßä ÇÑÓÇá ÇáÑÒã Úä ØÑíŞå   
      edvr_rtable_ent rte;	
	  bzero(&rte, sizeof(rte));		//ÊåíÆÉ ãÓÇÑ ÌÏíÏ
		//cout << "\n Node (" << myaddr_ << ") is calling recv(queued_p, 0) ..... to give the packets to ourselves to forward";

		Packet *queued_p;
		while ((queued_p = prte->q->deque()))
		{
			//num_of_q_packets++;   //added by ahmad
			// XXX possible loop here  
			// while ((queued_p = rte.q->deque()))
			// Only retry once to avoid looping
			// for (int jj = 0; jj < rte.q->length(); jj++){
			//  queued_p = rte.q->deque();
			recv(queued_p, 0); // ÊãÑíÑ ÇáÑÒãÉ ãä ÎáÇá ÚãáíÉ ÇÓÊŞÈÇá ÇáÑÒãÉ
		}

 
		//==============================================================   // added by ahmad
		// The next 4 statements are used to deque the entry that has been qued to be send...
		// The second statement (rte.q = 0;) and last statment (table_->AddEntry (rte,ahmad); ) to set q field in te routing table to 0.
		// =============================================================
		delete prte->q;	
		prte->q = 0;
		nsaddr_t *ahmad = &myaddr_;
        bcopy(prte, &rte,sizeof(edvr_rtable_ent));
		////cout<< "\n Node (" << myaddr_ << ") calls AddEntry after calling recv(queued_p, 0) function to deque the following entry ...  " << prte->src << "\t" << prte->f_nxt_hop  << "\t" << prte->s_nxt_hop<< "\t" << prte->metric<< "\t" << prte->link_no<<" to reset the q field\n";   // added by ahmad
		//table_->AddEntry(rte,ahmad);        // äÓÊÚÏÚÈ åĞÇ ÇáÊÇÈÚ áÅÚÇÏÉ ÖÈØ ÍŞá ÇáÑÊá
        
	}
   // put the periodic update sending callback back onto the 
	// the scheduler queue for next time....

	// this will take the place of any planned triggered updates
	lasttup_ = now;
	return(is_it_a_neighbor);
	//****************************************************  äåÇíÉ ÚãáíÉ İÍÕ ÇáÌÇÑ *********************************************************
 }
int
EDVR_Agent::Update_Route(edvr_rtable_ent *old_rte, edvr_rtable_ent *new_rte,int xx)
{//****************************************************  ÈÏÇíÉ ÊÇÈÚ ÊÍÏíË ÇáãÓÇÑ  *********************************************************
 	int negvalue = -1;
	assert(new_rte);

	Time now = Scheduler::instance().clock();
    new_rte->changed_at = now;  //   added 
	char buf[1024];
	//  snprintf (buf, 1024, "%c %.5f _%d_ (%d,%d->%d,%d->%d,%d->%d,%lf)",

	snprintf (buf, 1024, "%c %.5f _%d_ (%d,%d->%d,%d->%d,%d->%d,%f)",
	(new_rte->metric != BIG && (!old_rte || old_rte->metric != BIG)) ? 'D' : 'U', 
	now, myaddr_, new_rte->dst, 
	old_rte ? old_rte->metric : negvalue, new_rte->metric, 
	old_rte ? old_rte->seqnum : negvalue,  new_rte->seqnum,
	old_rte ? old_rte->f_nxt_hop  : -1,  new_rte->f_nxt_hop , 
	new_rte->advertise_ok_at);

	edvr_rtable_ent *prte;		//added by ahmad

	nsaddr_t *ahmad = &myaddr_;//ÚäæÇä ÇáÚŞÏÉ ÇáÍÇáíÉ
				  
	////cout << "\n      - Node (" << myaddr_ << ") is planning to add the following entry....  " << new_rte->dst << "     " << new_rte->f_nxt_hop  << "     " << new_rte->s_nxt_hop  << "     " << new_rte->metric << "     " << new_rte->link_no;   // added by ahmad
	////table_->Show_rt();  

   if (new_rte->metric > 0)		 // added 
	  new_rte->advertise = true;  // íÌÈ äÔÑ åĞÇ ÇáãÓÇÑ

	if (new_rte->metric > 1)
		new_rte->timeout_event = 0;//

	int write =table_->AddEntry (*new_rte,ahmad);    // ÅÖÇİÉ ÇáãÓÇÑ ÈÌÏæá ÇáÊæÌíå ãÚ ÇáÍİÇÙ Úáì ÇáãÓÇÑÇÊ ÇáÃİÖá æÇáãÓÇÑÇÊ ÇáãÊÚÏÏÉ æÇáÛíÑ ãÊÔÇÈåÉ ÈÇáÚŞÏ
   
	if (trace_wst_)
	    trace ("VWST %.12lf frm %d to %d wst %.12lf nxthp %d [of %d]",
		now, myaddr_, new_rte->dst, new_rte->wst, new_rte->f_nxt_hop , 
		new_rte->metric);

	if (verbose_)
	   trace ("VS%s", buf);
	////table_->Show_rt(); 
	return(write);
	//****************************************************  äåÇíÉ ÊÇÈÚ ÊÍÏíË ÇáãÓÇÑ  *********************************************************
}
void 
EDVR_Agent::Process_Packet(Packet * p)
{//************************************  ÈÏÇíÉ ÚãáíÉ ãÚÇáÌÉ ÇáÑÒãÉ  ***********************************************

	// it's a EDVR packet

	int HELLO_PACKET = 20;   // ÑÒãÉ ÊÑÍíÈ
	int FULLDUMP_PACKET = 22;	   // ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ
	int UPDATE_PACKET = 24;	   // ÑÒãÉ ÊÍÏíË
    int RREQ_PACKET = 26;      // ÑÒãÉ ØáÈ ÇáãÓÇÑ
    int RREP_PACKET = 27;      // ÑÒãÉ ÑÏ ÇáãÓÇÑ
	hdr_ip *hdri = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN (p);
	  
	unsigned char *d = p->accessdata ();     // change count  ãÄÔÑ Úáì ÈÏÇíÉ ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÚÏÏ ÇáãÓÇÑÇÊ ÇáãÖãäÉ  d
	unsigned char *w = d + 1;    // ãÄÔÑ Úáì ÇáÈÇíÊ ÇáÊÇáí ãä ÇáÈíÇäÇÊ ÇáãÖãäÉ ÈÇáÑÒãÉ æÇáĞí åí ÇáåÏİ w

	if ( hdrc->ptype() == HELLO_PACKET)		// ÅĞÇ ßÇäÊ ÑÒãÉ ÊÑÍíÈ ÇáŞíÇã ÈÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÊÑÍíÈ
		Receive_Hello_Packet(p);

	if (hdrc->ptype() == FULLDUMP_PACKET)			// ÅĞÇ ßÇäÊ ÑÒãÉ ãÚáæãÇÊ ÊæÌíå ßÇãáÉ ÇáŞíÇã ÈÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ãÚáæãÇÊ ÊÑÌíå ßÇãáÉ
		Receive_Fulldump_Packet(p);

	if (hdrc->ptype() == UPDATE_PACKET)           // ÅĞÇ ßÇäÊ ÑÒãÉ ÊÍÏíË ÇáŞíÇã ÈÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÊÍÏíË
		Receive_Update_Packet(p);


    if (hdrc->ptype() == RREQ_PACKET)          // ÅĞÇ ßÇäÊ ÑÒãÉ ØáÈ ÇáãÓÇÑ ÇáŞíÇã ÈÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ØáÈ ÇáãÓÇÑ
		Receive_RREQ_Packet(p);

    if (hdrc->ptype() == RREP_PACKET)          // ÅĞÇ ßÇäÊ ÑÒãÉ ÑÏ ÇáãÓÇÑ ÇáŞíÇã ÈÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ ÑÏ ÇáãÓÇÑ
		receive_RREP_Packet(p);
//****************************************************  äåÇíÉ ÚãáíÉ ãÚÇáÌÉ ÇáÑÒãÉ  *********************************************************
}
int 
EDVR_Agent::diff_subnet(int dst)//íİÍÕ İíãÇ ÇĞÇ åĞÇ ÇáåÏİ áäİÓ ÇáÚŞÏÉ Çæ áÚŞÏÉ ÇÎÑì
{
	char* dstnet = Address::instance().get_subnetaddr(dst);
	if (subnet_ != NULL) {
		if (dstnet != NULL) {
			if (strcmp(dstnet, subnet_) != 0) {
				delete [] dstnet;
				return 1;
			}
			delete [] dstnet;
		}
	}
	return 0;
}
void
EDVR_Agent::Forward_Packet_Process (Packet * p)//ÊÊã åĞå ÇáÚãáíÉ áÊãÑíÑ ÑÒãÉ ÇáÈíÇäÇÊ ááåÏİ ÇáÕÍíÍ ÚäÏãÇ ÊÓÊŞÈá ÇáÚŞÏÉ ÑÒãÉ ÈíÇäÇÊ áÚŞÏÉ ÃÎÑì æáíÓÊ åí ÇáÚŞÏÉ ÇáåÏİ
{//****************************************************  ÈÏÇíÉ ÚãáíÉ ÊãÑíÑ ÇáÑÒãÉ  *********************************************************
	hdr_ip *hdri = HDR_IP(p);
	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();
	hdr_cmn *hdrc = HDR_CMN (p);
	edvr_rtable_ent *prte;
  
	int src = Address::instance().get_nodeaddr(hdri->saddr());		 // ahmad
    int dst = Address::instance().get_nodeaddr(hdri->daddr());		 // ahmad
	int change_count=0; //ahmad
	// íÌÈ Ãä äŞæã ÈÊæÌíå ÇáÑÒãÉ ÇáÍÇáíÉ
	// printf("(%d)-->forwardig pkt\n",myaddr_);
	//cmn header field to down  áÊãÑíÑ ÇáÑÒãÉ íÌÈ Ãä äÖÚ  
	// set direction of pkt to -1 , i.e downward // ÚäÏãÇ íßæä ÇáÇÊÌÇå ááÇÓİá Çí Çä åĞå ÇáÑÒãÉ áÚŞÏÉ ÇÎÑì æÇĞÇ ßÇä ááÇÚáì íÚäí Çä ÇáÑÒãÉ áäİÓ ÇáÚŞÏÉ
	hdrc->direction() = hdr_cmn::DOWN;		//ÇĞÇ ßäÇ äÑíÏ ÊæÌíå ÇáÑÒãÉ down íÌÈ Ãä äÖÇÚ ÇáÇÊÌÇå // added by ahmad
											// The forward_data() function decideds whether a packet has to delivered to the upper-layer agents or to 
											//  be forwarded to other nod.  if direction = DOWN (hdrc->direction() = hdr_cmn::DOWN;), this means that 
											// the packet is forwarded to an other node, if its UP this means the packet is to this node (to myself) //ahmad
	// ÇĞ ÇáåÏİ ÎÇÑÌ ãÌÇá ÇáÚŞÏÉ ÇáãÊäŞáÉ
	//base_stn ÊãÑíÑ åĞå ÇáÑÒãÉ ááãÍØÉ ÇáŞÇÚÏíÉ 
	// ãáÇÍÙÉ: áÇ íÊã ÊÎÒíä ÇáÑÒãÉ ÅĞÇ ÇáãÓÇÑ ááãÍÏÉ ÇáŞÇÚÏíÉ ÛíÑ ãÚÑæİ

	// ÇáãÑÍáÉ 1:====================================================================
	//ÅĞÇ ÇáåÏİ ÎÇÑÌ Ïæãíä ÇáÚŞÏÉ ÇáãÊäŞáÉ ãÑÑ ÇáÑÒãÉ ááãÍØÉ ÇáÃÓÇÓíÉ 
	//ÅĞÇ ßÇä ÇáãÓÇÑ ááãÍØÉ ÇáÃÓÇÓíÉ ÛíÑ ãÚÑæİ áÇ íÊã ÊÎÒíä ÇáÑÒã
    //      ÊÍÏÏ İíãÇ ÅĞÇ ÇáåÏİ íÚæÏ Åáì ÔÈßÉ ãÎÊáİÉ   
	if (diff_subnet(hdri->daddr())) 
	{
		prte = table_->GetEntry2 (dst,-99,&myaddr_);  //ÇáÍÕæá Úáì ÃİÖá ãÓÇÑ ááÚŞÏÉ ÇáåÏİ ÈÛÖ ÇáäÙÑ Úä ÇáŞİÒÉ ÇáÃæáì
		if (prte && prte->metric != BIG) 
			goto send;//ÇÑÓÇá ÇáÑÒãÉ
		  
		dst = node_->base_stn();// ÇĞÇ ÇáÚŞÏÉ ÇáåÏİ ÎÇÑÌ ãÌÇá ÇáÚŞÏÉ äãÑÑ ÇáÑÒãÉ ááãÍØÉ ÇáÇÓÇÓíÉ æĞÇ ßÇä ÇáãÓÇÑ áåÇ ÛíÑ ãÚÑæİ áÇ íÊã ÊÎÒíä ÇáÑÒãÉ
		prte = table_->GetEntry2 (dst,-99,&myaddr_);  // ÇáÍÕæá Úáì ÃİÖá ãÓÇÑ ááåÏİ
		if (prte && prte->metric != BIG) 
			goto send;//ÇäÊŞá ááÅÑÓÇá
		  
		else
		{//ÇåãÇá ÇáÑÒãÉ
			//drop pkt with warning
			fprintf(stderr, "warning: Route to base_stn not known: dropping pkt\n");
			Packet::free(p);//ÊÍÑíÑ ÇáÑÒãÉ
			return;
		}
	}
	// ÇáãÑÍáÉ ÇáËÇäíÉ:================================================================
    // İí ÍÇá ÇáãÓÇÑ ãæÌæÏ ÈÛÖ ÇáäÙÑ Úä ãÑÓá ÇáÑÒãÉ

	////table_->Show_rt();  //print routing table 
	////cout<<"Node ("<<myaddr_ <<") Looking for a Route to Node ("<< dst <<") * * *\n"; 
	prte = table_->GetEntry2 (dst,-99,&myaddr_);  //ÇáÍÕæá Úáì ÃİÖá ãÓÇÑ ááÚŞÏÉ ÇáåÏİ ÈÛÖ ÇáäÙÑ Úä ÇáŞİÒÉ ÇáÃæáì
	if (prte && prte->metric != BIG)
	{
		//printf("(%d)-have route for dst\n",myaddr_);
		////cout<<"     Ok.. Confirmed Go to send it.  ..  ...   .....\n";
		goto send; //ÇäÊŞá ááÅÑÓÇá
	}  
    
      //İí ÍÇá áÇ íæÌÏ ãÓÇÑ Çæ ÇáãÓÇÑ ÇáãæÌæÏ ãŞØæÚ
	// ÇáãÑÍáÉ ÇáËÇáËÉ:===================================================================================
    // İí ÍÇáÉ ÃäÇ áÓÊ ãÑÓá åĞå ÇáÑÒãÉ æÇáãÑÓá ÇáÃÕáí ÚŞÏÉ ãÎÊáİÉ 
	         
	if(prte && prte->metric == BIG && myaddr_!=src)	 // ÅĞÇ ßÇä ÇáãÓÇÑ ãŞØæÚ æÇäÇ áÓÊ ãÑÓá åĞå ÇáÑÒãÉ
	{
	   ////cout << "\n Node (" << myaddr_ << ") found a broken route. I'm not this packet original sender..";
	   ////cout << "\n I'm only a forwarder hop So, I'm going to Brodcaste immediatly update Routing Information (Instead of sending Error Packet)";
		    //	 áÏíäÇ ãÚáæãÇÊ áäÔÑåÇ		  ahmad 
	   for (table_->InitLoop (); (prte = table_->NextLoop ()); )
	   {
			
			  if (prte && prte->advertise /*&& prte->metric > 0*/)	 //  ÊÖãíä Ãí ãÓÇÑ íÌÈ äÔÑå 
                  change_count++;
	   }  
		  
	   if(change_count>0)
	   {
			Packet *p1 = Make_Update_Packet(change_count);	 // ÅäÔÇÁ ÑÒãÉ ÊÍÏíË áäÔÑåÇ
			if (p1) 
			 sendOutBCastPkt1(p1);  // Óæİ ÃŞæã ÈäÔÑ ÑÒãÉ ÊÍÏíË
	   }   
	   ////cout << "\n drop the packet number " << hdrc->uid() << "\n";
	   drop(p, DROP_RTR_NO_ROUTE);		//ÇåãÇá åĞå ÇáÑÒãÉ
	   ////table_->Show_rt();  //
	   return;
	}   
	else	 //  ÃäÇ ãÑÓá åĞå ÇáÑÒãÉÇáÃÕáí áßä áÇ íæÌÏ ãÓÇÑ Ãæ ÇáãÓÇÑ ãŞØæÚ
	 // ÇáãÑÍáÉ 4:===================================================================================     
	{
			if (prte && prte->dst !=prte->f_nxt_hop )	   // ÇáãÓÇÑ ááÚŞÏÉ ÇáåÏİ ÛíÑ ãÈÇÔÑ 
				{
					prte = NULL; 
					prte = table_->GetEntry1 (dst, dst,&myaddr_);  //   ÇáÍÕæá Úáì ÇáãÓÇÑ ÇáãÈÇÔÑ
				}

			if (prte && (prte->dst==prte->f_nxt_hop )&& prte->metric == BIG)	 // äİÍÕ Çä ßÇä ÇáãÓÇÑ ÇáãÈÇÔÑ ãæÌæÏ æİí ÍÇá ÛíÑ ãæÌæÏ ääÔÆå
			//if (prte)	 
			{
				/* íÌÈ Ãä äÖÚ åĞå ÇáÑÒãÉ ÈÇáÑÊá */
				//printf("(%d)-no route, queue pkt\n",myaddr_);
				//==========================================================================================================
				// åäÇ ÇäÔÇÁ ÇáÑÊá Åä áã íÊã ÇäÔÇÆå ÈÚÏ  //ahmad

				////cout << "\n\n Node (" << myaddr_ << ") found a broken route. So, it is going to que the packet number " << hdrc->uid() << "\n";

				if (!prte->q)
				{
					prte->q = new PacketQueue ();
				}
				           
				prte->q->enque(p);		// ÅÏÎÇá ÇáÑÒãÉ ááÑÊá
				////cout << "....Queue This packet ...."<<hdrc->uid()<<"\n";
				if (verbose_)
					trace ("VBP %.5f _%d_ %d:%d -> %d:%d", now, myaddr_, hdri->saddr(),hdri->sport(), hdri->daddr(), hdri->dport());
						
				while (prte->q->length () > MAX_QUEUE_LENGTH)
				{	//added 
					////cout << "\n\n Node (" << myaddr_ << ") ÇåãÇá ÇáÑÒãÉ ÈÓÈÈ ÇãÊáÇÁ ÇáÑÊá
					drop (prte->q->deque (), DROP_RTR_QFULL);
				}	//added 
		        // double RREQ_TimeOut =1.6;  moved up
				if(prte->packet_id==0 ||prte->changed_at+RREQ_TimeOut<now )	  //RREQ_Timeout ã íÊã ØáÈ ãÓÇÑ áåĞÇ ÇáåÏİ ãä ŞÈá Ãæ ãÖì Úáì ÃÎÑ ÑÒãÉ ØáÈ ãÓÇÑ áäİÓ ÇáåÏİ ÇáÑÒãä 
				{													          

					////cout<<"\n	prte->packet_id ="<< prte->packet_id<<"\n";
					Packet *p= Make_RRQ_Packet(myaddr_,dst);  //  ÊæáíÏ ÑÒãÉ ØáÈ ÇáãÓÇÑ áäÔÑåÇ ÚÈÑ ÇáÔÈßÉ
					if(p)
					{
						hdr_ip *hdri = HDR_IP(p);
						hdr_cmn *hdrc = HDR_CMN (p);
						prte->packet_id=hdrc->uid();	 //RREQ ááÊÍßã ÈÑÒãÉ ØáÈ ÇáãÓÇÑ 
						prte->changed_at=now;			 // RREQ ááÊÍßã ÈÑÒãÉ ØáÈ ÇáãÓÇÑ
						assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
						sendOutBCastPkt1(p);   // äÔÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
						////cout<<"\n "<<  prte->dst<<"  "<< prte->f_nxt_hop <<"  "<<prte->s_nxt_hop <<"  "<< prte->metric<<"  "<<  prte->link_no<<"  "<<prte->q<<"  "<<prte->packet_id;		
						////cout<<"\n 2- old broken entry I need to generate and broadcaste make requist  \n"; 
					}   
				}
				return;	 
			}
			else
			{
				// Brand new destination
				//============================================================================================
				//áÇ íæÌÏ ãÓÇÑ ááåÏİ¡ áĞÇ äÍÊÇÌ Ãä äÍÕá Úáì ãÚáæãÇÊ ãä åĞå ÇáÑÒãÉ áÇÓÊÎÏÇã åĞå ÇáãÚáæãÇÊ ãÓÊŞÈáÇ
				//=============================================================================================
				//ÇáÍÕæá Úáì ãÓÇÑ ãÈÇÔÑ
				edvr_rtable_ent rte;
				double now = s.clock();     
				bzero(&rte, sizeof(rte));
				rte.dst = dst;
				rte.f_nxt_hop  = dst;
				rte.s_nxt_hop  = -99;			//added by ahmad
				rte.metric = BIG;
				rte.packet_id =0;		// initial value added by ahmad
				rte.seqnum = 0;
				rte.link_no = myaddr_*10000+dst;  // ÑŞã ÇáæÕáÉ added by ahmad
				rte.advertise_ok_at = now + 604800;	// check back next week... :)
				rte.advert_seqnum = false;	 //ahmad
				rte.advert_metric = false;	 //ahmad
				rte.advertise = false;  // added   áÇ ÃÑíÏ Ãä ÇäÔÑ åĞÇ ÇáãÓÇÑ áÇäå ãŞØæÚ
				rte.changed_at = now;  	// was now + wst0_, why??? XXX -dam
				rte.wst = wst0_;
				rte.timeout_event = 0;
				rte.q = new PacketQueue();
				rte.q->enque(p);   
					  
				assert (rte.q->length() == 1 && 1 <= MAX_QUEUE_LENGTH);
						
				nsaddr_t *ahmad = &myaddr_;
					  
				if (verbose_)
					trace ("VBP %.5f _%d_ %d:%d -> %d:%d", now, myaddr_,hdri->saddr(), hdri->sport(), hdri->daddr(), hdri->dport());

				////cout << "....Queue This packet ...."<<hdrc->uid()<<"\n";
				//ÇäÔÇÁ Ñæäì ØáÈ ÇáãÓÇÑ ááÚŞÏÉ ÇáåÏİ áÃæá ãÑÉ
				Packet *p= Make_RRQ_Packet(myaddr_,dst);  //  ÊæáíÏ ÑÒãÉ ØáÈ ÇáãÓÇÑ
				if(p)
				{
					hdr_ip *hdri = HDR_IP(p);
					hdr_cmn *hdrc = HDR_CMN (p);
					rte.packet_id=hdrc->uid();	 //ááÊÍßã ÈÑÒãÉ ØáÈ ÇáãÓÇÑ
					////cout << "\n ....This packet ...."<<hdrc->uid()<<"\n";
					assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2
					sendOutBCastPkt1(p);          // äÔÑ ÑÒãÉ ØáÈ ÇáãÓÇÑ
					////cout<<"\n  new nighbour  I need to generate and broadcaste make requist  \n";  
				}    

				////cout<< "\n In ForwardPacket... Node (" << myaddr_ << ") didn't find a route to node " << rte.dst << ". So, it calls AddEntry() function... to add the following entry:  " << rte.dst << "   " << rte.f_nxt_hop  << "   " << rte.s_nxt_hop << "   " << rte.metric << "   " << rte.link_no << "\n";   // added by ahmad
				table_->AddEntry (rte,&myaddr_);   //ÅÖÇİÉ ÇáãÓÇÑ ÇáãÈÇÔÑ áÌÏæá ÇáÊæÌíå               // added by ahmad  
				return;	
			}
	}
	send:

	////cout << "\n In ForwardPacket function node (" << myaddr_ << ") calls (Send) using the route:     " << prte->dst << "    " << prte->f_nxt_hop  << "    " << prte->s_nxt_hop  << "    " << prte->metric << "    " << prte->link_no;

	hdrc->addr_type_ = NS_AF_INET;
	hdrc->xmit_failure_ = mac_callback;
	hdrc->xmit_failure_data_ = this;
	hdrc->prev_hop_ = myaddr_;              // added by ahmad

	if (prte->metric > 1)
			// We need a function to select the best rote   .... Added 
		hdrc->next_hop_ = prte->f_nxt_hop ;//ÊÍÏíÏ ÇáŞİÒÉ ÇáÊÇáíÉ
	else
		hdrc->next_hop_ = dst;

	if (verbose_)
	{
		trace ("Routing pkts outside domain: \
			VFP %.5f _%d_ %d:%d -> %d:%d", now, myaddr_, hdri->saddr(),hdri->sport(), hdri->daddr(), hdri->dport());  
	}

	assert (!HDR_CMN (p)->xmit_failure_ || HDR_CMN (p)->xmit_failure_ == mac_callback);

	//cout << "\n Node (" << myaddr_ << ") uses the route    " << prte->dst << "    " << prte->f_nxt_hop  << "    " << prte->s_nxt_hop  << "    " << prte->metric << "    " << prte->link_no << "    to send the packet\n\n\n";	// modified	

	target_->recv(p, (Handler *)0);//ÇÓÊŞÈÇá ÇáÑÒãÉ

	////cout << "\n...... returned from target_->recv(p, (Handler *)0) function ...............\n";		// stopped 
	return;
	 //****************************************************  äåÇíÉ ÚãáíÉ ÊãÑíÑ ÇáÑÒãÉ ********************************************************* 
}
void 
EDVR_Agent::sendOutBCastPkt1(Packet *p)
{	//****************************************************  ÈÏÇíÉ ÚãáíÉ äÔÑ ÇáÑÒãÉ  *********************************************************
	hdr_ip *hdri = HDR_IP(p);
	Scheduler & s = Scheduler::instance ();
	double now = s.clock ();
	hdr_cmn *hdrc = HDR_CMN (p);
	hdrc->prev_hop_ = myaddr_;    
    hdrc->next_hop_ = IP_BROADCAST;
	hdrc->addr_type_ = NS_AF_INET;		// in AODV .. NS_AF_NONE;	the comment added on 
	hdri->daddr() = IP_BROADCAST << Address::instance().nodeshift();
	hdri->dport() = ROUTER_PORT;
    hdrc->iface() = -2;			// added 
	hdrc->error() = 0;			// added 
	hdri->sport() = ROUTER_PORT;	// added 
	hdrc->direction() = hdr_cmn::DOWN;       //important: change the packet's direction	 */
   
	int HELLO_PACKET = 20;
	//int ERROR = 23;
			 
		
		assert (!HDR_CMN (p)->xmit_failure_);	// DEBUG 0x2

		    s.schedule (target_, p, jitter(EDVR_BROADCAST_JITTER, be_random_));    //ÇäÔÑ åĞå ÇáÑÒãÉ ÈÚÏ İÊÑÉ ÒãäíÉ ÚÔæÇÆíÉ æĞáß ãä ÃÌá ÚãáíÉ ÇáÊÒÇãä
        
	   if (hdrc->ptype() != HELLO_PACKET)	 // ÅĞÇ ßÇäÊ ÇáÑÒãÉ áíÓÊ ÑÒãÉ ÊÑÍíÈ
	  {	   ////cout << "\n ***** Old periodic_Beacon time  "  << periodic_Hello_->time_ << "  is canceled";
		   s.cancel(periodic_Hello_);   // ÊÌÇåá ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ
	  }
	   // put the periodic update sending callback back onto the 
       // the scheduler queue for next time.... jitter to avoid sync
	   //ÌÏæáÉ ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ
	      s.schedule (helper_, periodic_Hello_,perup_ * (0.75 + jitter (0.25, be_random_)));
	      
		////cout << "\n The next periodic call back time is  "  << periodic_Hello_->time_ << "\n";
		////cout << "\n ***** Next periodic_Hello time is  " << periodic_Hello_->time_ <<"\n\n";

	 return;
//****************************************************  äåÇíÉ ÚãáíÉ äÔÑ ÇáÑÒãÉ  *********************************************************
	 }
void 
EDVR_Agent::sendOutBCastPkt(Packet *p)
{
  Scheduler & s = Scheduler::instance ();
  // send out bcast pkt with jitter to avoid sync
  s.schedule (target_, p, jitter(EDVR_BROADCAST_JITTER, be_random_));
}   
void
EDVR_Agent::recv (Packet * p, Handler *)
{//****************************************************  ÈÏÇíÉ ÚãáíÉ ÇáÇÓÊŞÈÇá  *********************************************************

    hdr_ip *hdri = HDR_IP(p);
    hdr_cmn *hdrc = HDR_CMN(p);
    int src = Address::instance().get_nodeaddr(hdri->saddr());
    int dst = Address::instance().get_nodeaddr(hdri->daddr());
			

    Time now = Scheduler::instance().clock(); 
    

	//printf("\n\n\n\n At %.8f Node  %02d  received a message from node  %02d ..... Original Source node %02d ",now,myaddr_,hdrc->prev_hop_,src);

	char *packet_type;
	switch(hdrc->ptype())//ÊÍÏíÏ äãØ ÇáÑÒãÉ ÇáãÓÊŞÈáÉ
	{
		case 0:
			packet_type="DATA";
			break;
	   case 2:
			packet_type="TRAFFIC";
			break;
		case 5:
			packet_type="ACKNOWLEDGMENT MESSAGE";				 
			break;
		case 13:
			packet_type="MESSAGE";
			break;
		case 20:
			packet_type="A HELLO_PACKET MESSAGE";
			break;
		case 22:
			packet_type="FULLDUMP_PACKET (Full Routing Information. Packet)";
			break;
		case 24:
			packet_type="UPDATE_PACKET (Route Upadet Packet)";
			break;
		case 25:
			packet_type="ERROR PACKET";
			break;
        case 26:
			packet_type="RREQ (Route Requist Packet)";
			break;
        case 27:
			packet_type="RREP (Route Replay Packet)";
			break;
		default:
			packet_type="Not Listed";
			     //<< hdrc->ptype();
			break;
	}   
   
	//******************************* ÊİÍÕ ÇáÚŞÏÉ Åä ßÇäÊ åí ãÑÓá åĞå ÇáÑÒãÉ  *****************
	 
    if(src == myaddr_ && hdrc->num_forwards() == 0)
    {//ÇáÚŞÏÉ ÇáÍÇáíÉ åí ãÑÓá åĞå ÇáÑÒãÉ æÚÏÏ ÇáŞİÒÇÊ åæ ÕİÑ ÊÖíİ ÊÑæíÓÉ áÇíÈí áÊãÑÑåÇ İíãÇ ÈÚÏ
		// Add the IP Header
		hdrc->size() += IP_HDR_LEN;    
		hdri->ttl_ = IP_DEF_TTL;
		////cout << "\n Node  " << myaddr_ << "  adds the IP Header because the packet has been generated by me.";     stopped 
		////cout << "\n Node  " << myaddr_ << "  adds the IP Header.";
	}
    
    //  ÇÓÊŞÈáÊ ÑÒãÉ ÃäÇ ÃÑÓáÊåÇ¡ ãä ÇáãÍÊãá ÍáŞÉ
	else if(src == myaddr_) 
	{// ÇáÚŞÏÉ ÇáÍÇáíÉ åí ãÑÓá åĞå ÇáÑÒãÉ æÚÏÏ ÇáŞİÒÇÊ áÇ íÓÇæí ÇáÕİÑ áĞÇ Êåãá åĞå ÇáÑÒãÉ
	     //Time now = Scheduler::instance().clock();   // added by ahmad 
		////cout << "\n Node  " << myaddr_ << "  drops the Packet because of (loop)  src == myaddr.";
	    drop(p, DROP_RTR_ROUTE_LOOP); 
		return;
	}

	
	else
	{

		if(--hdri->ttl_ == 0)          //***************åäÇ ÇáÚŞÏÉ áíÓÊ ÇáãÑÓá áßä ÇäÊåì Òãä ÇáÍíÇÉ áåÇ
		{
			// áÇ ÊŞæã ÈÔíÆ
		}
	}
    if ((src != myaddr_) && (hdri->dport() == ROUTER_PORT))
	{ 
	    Process_Packet(p);//ÇáÑÒãÉ åí ÑÒãÉ ÊæÌíå
		
    }
    else if (hdri->daddr() == ((int)IP_BROADCAST) && (hdri->dport() != ROUTER_PORT))
	{//ÇáÑÒãÉ áíÓÊ ÑÒãÉ ÊÍßã æåí äÔÑ
		if (src == myaddr_) 
		{
			// handle brdcast pkt

			Time now = Scheduler::instance().clock();   // added by ahmad  
			////cout << "\n\n In Receive function ....   Node (" << myaddr_ << ") is calling sendOutBCastPkt function at  " << now << "\n";	      // added by ahmad
             //äÔÑ åĞå ÇáÑÒãÉ
			sendOutBCastPkt(p);
		}
		else
		{	
				// hand it over to the port-demux
				Time now = Scheduler::instance().clock();   // added by ahmad  
				////cout << "\n\n In Receive function ....   Node (" << myaddr_ << ") is calling port_dmux_->recv function at  " << now << "\n";	      // added by ahmad

				port_dmux_->recv(p, (Handler*)0);//ÑÒãÉ áíÓÊ ÑÒãÉ ÊÍßã æáíÓÊ äÔÑ É
		}
	}
	else 
	{//ÇáÚŞÏÉ ÇáÍÇáíÉ áíÓÊ åí ÇáÚŞÏÉ ÇáåÏİ æÇáÑÒãÉ áíÓÊ ÑÒãÉ ÊÍßã áĞÇ ÊãÑÑ åĞå ÇáÑÒãÉ
		////cout << "Node (" << myaddr_ << ") trying to forward the packet to node  " << dst;	      // modified 
        Forward_Packet_Process(p);//ÇáŞíÇã ÈÚãáíÉ ÊãÑíÑ ÇáÑÒãÉ
	}
//****************************************************   äåÇíÉ ÚãáíÉ ÇÓÊŞÈÇá ÑÒãÉ  *********************************************************	
}
static class EDVRClass:public TclClass
{
  public:
  EDVRClass ():TclClass ("Agent/EDVR")
  {	
	////cout<<"EDVR-VER2.00   by ahmad alali \n";	 // ahmad
	////cout<< "\n\n Initiahmadzation State: AT 0.0000 Time";
    ////cout <<"\n                                                                                   D   1H    2H     M   L-No.\n";
  }
  TclObject *create (int, const char *const *)
  {
    return (new EDVR_Agent ());
  }
 } class_edvr;
EDVR_Agent::EDVR_Agent (): Agent (PT_MESSAGE), ll_queue (0), seqno_ (0), linkno_ (0),
  myaddr_ (0), subnet_ (0), node_ (0), port_dmux_(0),
  periodic_Hello_ (0), be_random_ (1), 
  use_mac_ (0), verbose_ (1), trace_wst_ (0), lasttup_ (-10), 
  alpha_ (0.875),  wst0_ (2), perup_ (15), 
  periodic_update_ (0), min_update_periods_ (3)	// constants
 {
  table_ = new EDVRRoutingTable ();
  helper_ = new EDVR_Helper (this);

  bind_time ("wst0_", &wst0_);
  bind_time ("perup_", &perup_);

  bind ("use_mac_", &use_mac_);
  bind ("be_random_", &be_random_);
  bind ("alpha_", &alpha_);
  bind ("min_update_periods_", &min_update_periods_);
  bind ("verbose_", &verbose_);
  bind ("trace_wst_", &trace_wst_);
  //DEBUG
  address = 0;
}
void
EDVR_Agent::Intialization_Process()
{//****************************************************  ÈÏÇíÉ ÚãáíÉ ÇáÊåíÆÉ  *********************************************************
	Time now = Scheduler::instance().clock(); 
	subnet_ = Address::instance().get_subnetaddr(myaddr_);
	 
	//DEBUG
	address = Address::instance().print_nodeaddr(myaddr_);
	//printf("myaddress: %d -> %s\n",myaddr_,address);
	 
	edvr_rtable_ent rte;
	bzero(&rte, sizeof(rte));
//ÊäÔÆ åäÇ ÇáÚŞÏÉ ãÓÇÑ áäİÓåÇ ÈÌÏæá ÇáÊæÌíå Úáì ÇáÔßá ÇáÊÇáí
	rte.dst = myaddr_;
	rte.f_nxt_hop  = myaddr_;
	rte.s_nxt_hop  = -99;   //added by ahmad
	rte.metric = 0;
	rte.seqnum = seqno_;
	rte.link_no = myaddr_*10000+myaddr_;  // added by ahmad
	seqno_ += 2;

	rte.advertise_ok_at = 0.0; // 
	rte.advert_seqnum = false;	 //ahmad
	rte.advert_metric = false;	 //ahmad
	rte.advertise = false;  // added   áÇ íæÌÏ ÏÇÚí áäÔÑ åĞÇ ÇáãÓÇÑ áÃäå íÔíÑ ááÚŞÏÉ äİÓåÇ
	rte.changed_at = now;  
	rte.new_seqnum_at = now;
	rte.wst = 0;
	rte.timeout_event = 0;		// åĞÇ ÇáãÓÇÑ áÇ ÊäÊåí ÕáÇÍíÊå :)
	  
	rte.q = 0;		// áÇ íãßä ÊÎÒíä ÑÒã ÈÇáÑÊá ááÚŞÏÉ ÇáÍÇáíÉ
	  
	nsaddr_t *ahmad = &myaddr_;
		  

	table_->AddEntry (rte,ahmad);              // added by ahmad

	   
	// ÌÏæáÉ ÑÒãÉ ÇáÊÑÍíÈ ÇáÊÇáíÉ
	periodic_Hello_ = new Event ();
	Scheduler::instance ().schedule (helper_, periodic_Hello_, jitter (EDVR_STARTUP_JITTER, be_random_));

	/* canceled by ahmad 
	//cout<< "\n\n In Intialization function node ( " << myaddr_ << " ) calls AddEntry function.....  " << rte.dst << "\t\t" << rte.f_nxt_hop << "\t" << rte.metric<<"\t" << rte.seqnum << "\t" << rte.new_seqnum_at<< "\t" << rte.wst;   // added by ahmad
	//cout<< "\n\n In Intialization function node ( " << myaddr_ << " ) calls AddEntry() function to add the following entry.....  " << rte.dst << "\t\t" << rte.f_nxt_hop << "\t" << rte.metric<<"\t" << rte.seqnum << "\t" << rte.new_seqnum_at<< "\t" << rte.wst;   // added by ahmad
	//cout << "\n Routing table of node ( " << myaddr_ << " )  consists of " << table_->number_of_elts() << " entry ....        Next periodic time is  " << periodic_Hello_->time_ << "\n";
	//table_->Show_rt();  //   */

	// added by ahmad 
	
	//cout<< "\n In Intialization ...  Node ( " << myaddr_ << " ) Adds an entry in its routing table for Itself........  " << rte.dst << "\t" << rte.f_nxt_hop << "\t" << rte.s_nxt_hop << "\t\t" << rte.metric<<"\t" << rte.link_no;   // added by ahmad
	////cout << "\n Routing table of node ( " << myaddr_ << " )  consists of " << table_->number_of_elts() << " entry ....        Next periodic time is  " << periodic_callback_->time_ << "\n";
	//table_->Show_rt();
//****************************************************  äåÇíÉ ÚãáíÉ ÇáÊåíÆÉ  *********************************************************
	}
int 
EDVR_Agent::command (int argc, const char *const *argv)
{

	//Time now = Scheduler::instance().clock();  // added by ahmad alali
 
	if (argc == 2)
    {
	   if (strcmp (argv[1], "start-dsdv") == 0)
	   {
	     Intialization_Process();

		 return (TCL_OK);
	   }
       else if (strcmp (argv[1], "dumprtab") == 0)
	   {

			Packet *p2 = allocpkt ();
  			hdr_ip *iph2 = HDR_IP(p2);
	        edvr_rtable_ent *prte;

	        printf ("Table Dump %d[%d]\n----------------------------------\n",
		    iph2->saddr(), iph2->sport());
	        trace ("VTD %.5f %d:%d\n", Scheduler::instance ().clock (),
		    iph2->saddr(), iph2->sport());

	        // Freeing a routing layer packet --> don't need to call drop here.
	             
	        Packet::free (p2);

	        for (table_->InitLoop (); (prte = table_->NextLoop ());)
	            Print_Routing_Table ("\t", prte, this);
	            printf ("\n");

				return (TCL_OK);
	   }
       else if (strcasecmp (argv[1], "ll-queue") == 0)											  
	   {
	       if (!(ll_queue = (PriQueue *) TclObject::lookup (argv[2])))
	       {
	           fprintf (stderr, "EDVR_Agent: ll-queue lookup of %s failed\n", argv[2]);
	           return TCL_ERROR;
		   }

		   //printf("ll-queue \t At  %.9f \t my address  %02d\n"'now,myaddr_); added by ahmad alali
		   return TCL_OK;
	   }
	}
    else if (argc == 3)
    {
        if (strcasecmp (argv[1], "addr") == 0) 
		{
		    int temp;
		    temp = Address::instance().str2addr(argv[2]);
		    myaddr_ = temp;

		    return TCL_OK;
		}
 
	    TclObject *obj;

  	    if ((obj = TclObject::lookup (argv[2])) == 0)
		{
	        fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1],argv[2]);
		    return TCL_ERROR;
		}
        if (strcasecmp (argv[1], "tracetarget") == 0)
		{

			tracetarget = (Trace *) obj;
		    return TCL_OK;
		}
        else if (strcasecmp (argv[1], "node") == 0) 
			 {

				 node_ = (MobileNode*) obj;
				return TCL_OK;
			 }
			 else if (strcasecmp (argv[1], "port-dmux") == 0) 
				  {	
				      port_dmux_ = (NsObject *) obj;
				      return TCL_OK;
				  }
	}
	return (Agent::command (argc, argv));
}
                     