/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1997 Regents of the University of California.
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
 */
/* Ported from CMU/Monarch's code, nov'98 -Padma.*/

/* dsdv.h -*- c++ -*-
   $Id: dsdv.h,v 1.6 1999/08/20 18:03:16 haoboy Exp $

   */

#ifndef cmu_dsdv_h_
#define cmu_dsdv_h_
#include "config.h"
#include "agent.h"
#include "ip.h"		
#include "delay.h"
#include "scheduler.h"
#include "queue.h"
#include "trace.h"
#include "arp.h"
#include "ll.h"
#include "mac.h"
#include "priqueue.h"

#include "edvr_rtable.h"

#if defined(WIN32) && !defined(snprintf)
#define snprintf _snprintf
#endif /* WIN32 && !snprintf */

typedef double Time;

#define MAX_QUEUE_LENGTH 5
#define ROUTER_PORT      0xff //أي أن الرزمة رزمة توجيه

class EDVR_Helper;

class EDVR_Agent : public Agent {
  friend class EDVR_Helper;
public:
  EDVR_Agent();
  virtual int command(int argc, const char * const * argv);
  void lost_link(Packet *p);
  
protected:
  void helper_callback(Event *e);
  Packet* rtable(int);
  virtual void recv(Packet *, Handler *);
  void trace(char* fmt, ...);
  void tracepkt(Packet *, double, int, const char *);
  Packet * Make_Hello_Packet (/*int& periodic*/);//انشاء رزمة ترحيب دورية
  Packet * Make_Fulldump_Packet(int new_or_broken, nsaddr_t dst);   //انشاء رزمة معلومات توجيه كاملة
  Packet * Make_Update_Packet(int change_count);   // انشاء رزمة تحديث
  Packet * Make_RRQ_Packet(nsaddr_t requister, nsaddr_t destination);  // انشاء رزمة معلومات توجيه كاملة
  Packet * Make_RRP_Packet(nsaddr_t sende_to,nsaddr_t dst,nsaddr_t f_nxt_hop ,nsaddr_t s_nxt_hop ,int metric,int link_no); // انشاء رزمة رد المسار
  int Update_Route(edvr_rtable_ent *old_rte, edvr_rtable_ent *new_rte,int);//لإضافة مسار أو تحديثة
  void Process_Packet(Packet * p);//عملية المعالجة
  void New_Entry_Add(nsaddr_t dst);	 // لإضافة مسار جديد
  void Receive_Hello_Packet(Packet * p);//استقبال رزمة ترحيب
  void Receive_Fulldump_Packet(Packet * p);  // استقبال رزمة معلومات توجيه كاملة
  void Receive_Update_Packet(Packet * p);	  // استقبال رزمة تحديث
  void Receive_RREQ_Packet(Packet *p);
  void receive_RREP_Packet(Packet * p);
  void Broken_Route_Process(int prev_hop_, int packet_sender, int destination, int link_no);	// رسالة الخطأ
  int check_Neighbor_Process(int src);	                // تفحص الجار
  void Forward_Packet_Process(Packet * p);//تمرير الرزمة
  void Intialization_Process();//عملية التهيئة
  int  diff_subnet(int dst);
  void sendOutBCastPkt(Packet *p);
  void sendOutBCastPkt1(Packet *p);	            // عملية النشر    
  
  Trace *tracetarget;       // Trace Target
  EDVR_Helper  *helper_;    // EDVR Helper, handles callbacks
  EDVRRoutingTable *table_;   // جدول التوجيه
  PriQueue *ll_queue;       // link level output queue
  int seqno_;               // الرقم المتسلسل
  int myaddr_;              // عنوان العقدة الحالية
  int linkno_;				// رقم الوصلة
  // Extensions for mixed type simulations using wired and wireless
  // nodes
  char *subnet_;            // My subnet
  MobileNode *node_;        // My node
  char *address;
  NsObject *port_dmux_;    // my port dmux

  Event *periodic_Hello_;           // ترحيب دوري

  Event *periodic_update_;           // تحديث دوري

  
  // Randomness/MAC/logging parameters
  int be_random_;
  int use_mac_;
  int verbose_;
  int trace_wst_;
  
  //اخر مرة ارسل بها تحديث دوري..
  double lasttup_;		// زمن أخر تحديث
  double next_tup;		// زمن التحديث القادم
  
  // DSDV constants:

  double alpha_;  // 0.875
  double wst0_;   // 6 (secs)
  double perup_;  // 15 (secs)  الفترة الزمنية بين التحديثات
  int    min_update_periods_;    //الفترة الزمنية التي يجب أن يصل خلالها رزمة ترحيب أو تحديث
                                 // اذا بم يصل تحديث أو ترحيب خلال هذه الفترى نعتبر الجار غير قابل للوصول
  
  void Print_Routing_Table(const char *prefix, edvr_rtable_ent *prte, EDVR_Agent *a);
  
};

class EDVR_Helper : public Handler {
  public:
    EDVR_Helper(EDVR_Agent *a_) { a = a_; }
    virtual void handle(Event *e) { a->helper_callback(e); }

  private:
    EDVR_Agent *a;
};

#endif
