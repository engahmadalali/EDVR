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


/* rtable.h -*- c++ -*-
   $Id: rtable.h,v 1.3 1999/03/13 03:53:15 haoboy Exp $
   */
#ifndef cmu_rtable_h_
#define cmu_rtable_h_

#include "config.h"
#include "scheduler.h"
#include "queue.h"

#define BIG   250

#define NEW_ROUTE_SUCCESS_NEWENT       0
#define NEW_ROUTE_SUCCESS_OLDENT       1
#define NEW_ROUTE_METRIC_TOO_HIGH      2
#define NEW_ROUTE_ILLEGAL_CANCELLATION 3
#define NEW_ROUTE_INTERNAL_ERROR       4

#ifndef uint
typedef unsigned int uint;
#endif // !uint


class edvr_rtable_ent {
public:
  edvr_rtable_ent() { bzero(this, sizeof(edvr_rtable_ent));}
  nsaddr_t     dst;     // عنوان العقدة الهدف
  nsaddr_t     f_nxt_hop ;     // القفزة الأولى
  nsaddr_t     s_nxt_hop ;     // القفزة الثانية
  uint         metric;  // عدد القفزات
  uint         seqnum;  // أخر رقم متسلسل
  uint		   link_no; // رقم الوصلة
  int         packet_id;  // تستخدم في رزمة طلب المسار للمحافظة على أن العقدة يمر عليها طلب مسار واحد للعقدة الهدف لان هذه الرزمة تنشر عبر الشبكة 
                           
  double       advertise_ok_at; // متى يكون نعم لنشر هذا المسار
  bool         advert_seqnum;  // should we advert rte b/c of new seqnum?
  bool         advert_metric;  // should we advert rte b/c of new metric?
  Event        *trigger_event;

  bool         advertise;  // قيمة بوليانية لنشر هذا المسار ان كان جديد أو تم تعديله

  uint         last_advertised_metric; // metric carried in our last advert
  double       changed_at; // أخر مرة تم تحديث فيها هذا المسار
  double       new_seqnum_at;	// when we last heard a new seq number
  double       wst;     // running wst info
  Event       *timeout_event; // حدث يستخدم لجدولة حدث الانتهاء
  PacketQueue *q;		//تخزين الرزم في رتل للعقدة الهدف
};

// AddEntry لإضافة مسار في جدول التوجيه
//
// GetEntry للحصول على مسار لعقدة

class EDVRRoutingTable {
  public:
    EDVRRoutingTable();
    int AddEntry(const edvr_rtable_ent &ent,nsaddr_t *xxx); //لإضافة مسار لجدول التوجيه
    int Sort_Entry(const edvr_rtable_ent &ent,int first_entry_position, nsaddr_t nod_id);//لإعادة ترتيب جدول التوجيه
    int RemainingLoop();//عدد المسارات المتبقية
    void InitLoop();//لتهيئة الحلقة
	int number_of_elts();//عدد المسارات بجدول التوجيه
	void Show_rt()    ;//لإظهار جدول التوجيه
    edvr_rtable_ent *NextLoop();//الحصول على المسار التالي
    edvr_rtable_ent *GetEntry(nsaddr_t dest);
    edvr_rtable_ent *GetEntry1(nsaddr_t dest, nsaddr_t via,nsaddr_t *myaddr );
    edvr_rtable_ent *GetEntry2(nsaddr_t dest, nsaddr_t via,nsaddr_t *myaddr );

  private:
    edvr_rtable_ent *rtab;
    int         maxelts;
    int         elts;
    int         ctr;
};
    
#endif
