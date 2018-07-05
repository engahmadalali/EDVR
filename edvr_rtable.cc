// multipath....

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
#include <math.h>
#include <string.h>
//#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "edvr_rtable.h"
static int rtent_trich(const void *a, const void *b) {//هذا التابع يعيد 1 إذا كان هدف المسار الأول أكبر من هدف المسار الثاني ويعيد -1 اذا كان الثاني أكبر وإلا يعيد 0 اذا متساويين
  nsaddr_t ia = ((const edvr_rtable_ent  *) a)->dst;
  nsaddr_t ib = ((const edvr_rtable_ent *) b)->dst;
  if (ia > ib) return 1;
  if (ib > ia) return -1;
  return 0;
}
//==========================    حيث يبحث بناء على الهدف والقفزة الأولى check1 هذا التابع مشابه للتابع    =============================
static int check2(const void *a, const void *b,nsaddr_t nod_id){
  nsaddr_t ia1 = ((const edvr_rtable_ent  *) a)->dst;
  nsaddr_t ib1 = ((const edvr_rtable_ent  *) b)->dst;
  nsaddr_t ia2 = ((const edvr_rtable_ent  *) a)->f_nxt_hop ;
  nsaddr_t ib2 = ((const edvr_rtable_ent  *) b)->f_nxt_hop ;
    
  if ((ia1 == ib1)&& (ia2 == ib2))    // add by ahmad 
      return 0;          // يعيد هذا التابع 0 إذا المسار موجود   

  if ((ia1 < ib1) || ((ia1 == ib1) && (ia2 < ib2))) // add by ahmad 
	  return 1;      // يعيد1 اذا المسار غير موجود ليكمل البحث للأعلى

  if ((ia1 > ib1) || ((ia1 == ib1)&& (ia2 > ib2)))  // add by ahmad 
	  return 2;      // يعيد 1 اذا المسار غير موجود ليكمل البحث للأسفل 

 //=======================================   check2 نهاية التابع  ================================================================================
}
static int check1(const void *a, const void *b,nsaddr_t nod_id){
  nsaddr_t dst_a = ((const edvr_rtable_ent  *) a)->dst;
  nsaddr_t dst_b = ((const edvr_rtable_ent  *) b)->dst;

 
//========================== هذا التابع يبحث حسب الهدف فقط للتعامل مع المسارات الغير متشابهة ============================
  if (dst_a == dst_b)    // add by ahmad        
      return 0;          // يعيد هذا التابع 0 إذا المسار موجود
  
  if (dst_a < dst_b) 
	  return 1;      // يعيد1 اذا المسار غير موجود ليكمل البحث للأعلى

  if (dst_a > dst_b)   // add by ahmad 
	  return 2;      //يعيد 2 اذا المسار غير موجود ليكمل البحث للأسفل
//================================================================================================================================================
}
EDVRRoutingTable::EDVRRoutingTable() {
  elts = 0;
  maxelts = 80;//عدد المسارات الأعظمية بجدول التوجيه
  rtab = new edvr_rtable_ent [maxelts];//بناء جدول توجيه مكون من 80 مسار كجد أعظمي
}
int 
EDVRRoutingTable::AddEntry(const edvr_rtable_ent  &ent,nsaddr_t *xxx)  //تابع لإضافة مسار بجدول التوجيه مع المحافظة على المسارات الأفضل والمسارات المتعددة الغير متشابهة بالعقد
{
   edvr_rtable_ent  *it;

   //DEBUG
   assert(ent.metric <= BIG);

   
   int kk=0;

   int c=2;
   int N=0;   //المقطع الأصغر       //ahmad
   int elts1=0;                            //ahmad
   int min1=0;	//ahmad
   int max1=0;
   int first_entry_position;//موضع أول مسار
   if (elts > 0)		//ahmad
   { 
		max1=elts-1;	//ahmad
		if(max1==0)
           elts1=0;
		else
		{
            elts1=round(max1/2.0);   //	elts1=ceil(max1/2);	//ahmad
		}

		do
		{

             c = check1(&ent, &rtab[elts1],*xxx);	//ahmad            
             if(c==0)
		     {//أي أن المسار للعقدة الهدف المراد اضافته موجود
				int found1=0;
				int found2=0;
				int found3=0;
				int entry_no=0;

				while(rtab[elts1].dst==ent.dst && elts1 >=0)   //  الانتقال لأول مسار ضمن جدول التوجيه يعود لهذا الهدف
				{
					elts1--;
				}
				
				elts1++;                    // هذا أول مسار يعود للهدف
                first_entry_position=elts1;  //عنوان أول مسار للعقدة الهدف

				while(rtab[elts1].dst==ent.dst)
				{
					if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].f_nxt_hop ==ent.f_nxt_hop )&&(rtab[elts1].metric<=ent.metric)&&(rtab[elts1].metric!=BIG)&& elts1 >= 0) //تجاهل هذا المسار لان المسارين لهم نفس الهدف والقفزة الاولى والمسار الموجود بجدول التوجيه عدد القفزات له أصغر أو يساوي المسار الجديد
					{
						//cout << "\n      تجنب هذا المسار لان له نفس الهدف ونفس القفزة الأولى وعدد القفزات أكبر أو تساوي المسارالموجود  ";
						return(0);  //لا تتم عملية الكتابة
					}

					if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].f_nxt_hop !=ent.f_nxt_hop )&&((rtab[elts1].f_nxt_hop ==ent.s_nxt_hop )||(rtab[elts1].s_nxt_hop ==ent.f_nxt_hop ))&&(ent.s_nxt_hop !=ent.dst)&&(rtab[elts1].metric<=ent.metric)&&(rtab[elts1].metric!=BIG)&& elts1 >= 0)// 
					{																													//  تجنب هذا المسار لأنه متشابه بالعقد
						//cout << "\n      تجنب هذا المسار لأنه متشابه بالعقد القفزة الأولى = القفزة الثانية  ";
						return(0);  //لا تتم الكتابة
					}
                     
					if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].f_nxt_hop !=ent.f_nxt_hop )&&(rtab[elts1].s_nxt_hop ==ent.s_nxt_hop )&&(ent.s_nxt_hop !=ent.dst)&&(rtab[elts1].metric<=ent.metric)&&(rtab[elts1].metric!=BIG)&& elts1 >= 0)// تجنب هذا المسار المسارين لهم نفس الهدف، القفزة الأولى مختلفة، نفس القفزة الثانية والهدف ليس القفزة الثانية
					{																													//  تجنب هذا المسار لأنه متشابه بالعقد
						//cout << "\n      تجنب هذا المسار لأنه متشابه بالعقد القفزة الثانية نفسها ";
						return(0);  //لا تتم عملية الكتابة
					}
						
					if ((rtab[elts1].dst==ent.dst)&& (rtab[elts1].link_no==ent.link_no)&&(rtab[elts1].metric<=ent.metric)&&(rtab[elts1].metric!=BIG)&& elts1 >= 0)// 
					{																													//  تجنب هذا المسار
						//cout << "\n      تجنب هذا المسار لوجود مسار بجدول التوجيه له نفس رقم الوصلة ونفس الهدف وأقصر من حيث عدد القفزات ";
						return(0);  //لا تتم عملية الكتابة
					}

					if ((rtab[elts1].dst==ent.dst)&&((rtab[elts1].f_nxt_hop !=ent.f_nxt_hop )&&(rtab[elts1].s_nxt_hop ==ent.s_nxt_hop )&&(rtab[elts1].dst!=ent.s_nxt_hop ))&& elts1 >= 0)// المسارين لهم نفس الهدف والقفزة الأولى مختلفة والقفزة الثانية نفسها والقفزة الثانية ليست الهدف
					{																													//نفس القفزة الثانية والقفزة الأولى مختلفة  
						found3=1;              // اقل أولوية                                                                                          
						entry_no=elts1;
					}

					if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].link_no==ent.link_no)&& elts1 >= 0)// المسارين لهم نفس الهدف ونفس رقم الوصلة
					{																	//المسار متشابه بالوصلات
						found2=1;             //أولوية متوسطة
						entry_no=elts1;	
                        
					}
						
                         
	                if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].f_nxt_hop ==ent.f_nxt_hop )&& elts1 >= 0)   // المسارين لهم نفس الهدف ونفس القفزة الأولى
					{                               							  // تجنب هذا المسار لأن له نفس رقم القفزة الأولة
						found1=1;            // أعلى أولوية
						entry_no=elts1;
                        
                            
					}
					elts1++;	
		                   
				}

				if (found1+found2+found3==1)
				{
					elts1=entry_no;          //الحصول على مسار مشبه لذا القبام بعملية التعديل والكتابة 
					//cout << "\n      الحصول على مسار لنفس العقدة الهدف, ";
					//if (found1 == 1)
						//cout << " نفس القفزة الأولى ";
					//if (found2 == 1)
						//cout << " نفس رقم الوصلة ";
					//if (found3 == 1)
						//cout << " نفس القفزة الثانية والقفزة الأولى مختلفة ";
				}
				
				if (found1+found2+found3>1)
				{
					elts1=entry_no;   
                    //cout<< "\n      يوجد أكثر من مسار مشابه نفضل الموجود ولا نقوم بأي شيئ \n";
					//return(0);               ////    يوجد أكثر من مسار مشابه نفضل الموجود ولا نقوم بأي شيئ 
				}
                 
				if (found1+found2+found3==0)
				{
					elts1=first_entry_position;//يجب إضافة هذا المسار بالموضع الصحيح له
                    while(rtab[elts1].dst==ent.dst)
					{
                        if ((rtab[elts1].dst==ent.dst)&&(rtab[elts1].f_nxt_hop > ent.f_nxt_hop ))
						break;
						elts1++;	  
					}

					//cout << "\n found1+found2+found3= " << found1+found2+found3 << "     elts1= " << elts1 << "    kk= " << kk <<  "\n"; 
                    kk=elts1;
					goto insert_entry;//إضافة هذا المسار
				}					   
			}

            kk=elts1;		
            if ((c==0)||(elts1==0)||(min1==max1))
               break;
		     
		   
            //=============== المسار غير موجود بعد ==========================================================
            if(c==1)                //  غير موجود وربما يكون موجود بالقسم الأعلى 
			  max1=elts1-1;

            if(max1<min1)
			  max1=min1;

			if(c== 2) 	            // غير موجود وربما يكون موجود بالقسم الاسفل
			   min1=elts1+1;

			if(min1>max1)
			   min1=max1;

             elts1=round((min1+max1)/2.0) ;// تنفذ في حالة c==1 or c==2 
			    
		 }  while(elts1>=N);
		
		
   }

  //=================================================================================
  if (c == 0 && elts != 0)//أي يوجد مسار مشابه   
  {			                   //في جدول التوجيه لذا التعديل عليه يعدل بالجدول kkمؤشر للمسار الذي رقمه it
	  it=&rtab[kk];           
                             //القيمة 0 تعني أنه لا  حاجة لترتيب جدول التوجيه بعد التعديل في حالة أن المسار الجديد كتب مكان مسار قديم لان له نفس الهدف ونفس القفزة أي وضع بالمكان الصحيح
      int F_sort=0;          
    
	  if((rtab[kk].dst==ent.dst) && (rtab[kk].f_nxt_hop !=ent.f_nxt_hop )) // نفحص ان كان جدول التوجيه يجب أن يرتب بعد تبديل المسار الجديد 
		     F_sort=1;        // يجب تعديل جدول التوجيه لان المسار الجديد يجب أن يستبدل مسار بنفس الهدف لكن القفزة الأولى مختلفة
	  
      if ((rtab[kk].metric >= ent.metric) || ent.metric ==BIG)    // اختار المسار الأفضل إذا عدد القفزات للمسار القديم أكبر أو تساوي عدد القفزات للمسار الجديد أو عدد قفزات المسار الجديد كبير )
	  {
		  //cout<<"..القيام بعملية الكتابة والاستبدال \n";   // ahmad 
		  bcopy(&ent,it,sizeof(edvr_rtable_ent ));  // it نسخ المسار الجديد بدل المسار القديم بجدول التوجيه
 		  //it->advertise = true;  // added 

		 nsaddr_t nod_id =*xxx; 
	 	 if (F_sort==1)   // هل تم استبدال المسار الجديد بالقديم لان له نفس الهدف ونفس رقم الوصلة
			int z= Sort_Entry(ent,first_entry_position, nod_id);// نرى ‘ن كان المسار الجديد بموضع خاطئ بحاجة لترتيب جدول التوجيه    

      
		 return(1);      // يعيد 1 إذا تم استبدال المسار بنجاح
	  }
	  else
		  //cout << " and less number of hops.  So, no need to overwrite it  ";		// هذا يعني أن المسار الموجود به أفضل من المسار الجديد ولا يوجد داعي لتعديله

      return(0);                  // يرجع صفر إذا لم يتم استبدال المسار.
      
  }


insert_entry:
   //=============================================================
    // إذا كان جدول التوجيه ممتلئ نضاعف حجمه
   //=============================================================
 
    if (elts == maxelts)//إذا امتلئ جدول التوجيه ضاعف حجمه
	{  
		edvr_rtable_ent  *tmp = rtab;
		assert(temp);

		maxelts *= 2;
		rtab = new edvr_rtable_ent [maxelts];
		assert(rtab);
		bcopy(tmp, rtab, elts*sizeof(edvr_rtable_ent ));
		delete tmp;
	}


    //=============================================================
    // ضافة المسار الجديد بموضعه للحفاظ على جدول التوجيه مرتب
    //=============================================================

    // ******************* هذا الجزء تم الغائه، هذا الجزء لإيجاد أين نضع المسار الجديد
    //                     تم الغائه لأننا وجدنا المكان مسبقا
     

    if((c==2) && (elts!=0))    // added by ahmad 
	   kk++;

    int max=kk;               // added by ahmad 

    // Copy all the further out guys out yet another.
    // bcopy does not seem to quite work on sunos???
  
    // نقل المسارات من أخر واحد واحد واحد حتى الوصول للمكان الذي يجب أن يوضع فيه المسار الجديد

//نقل المسارات قبل المسار الصحيح إلى موضع المسار التالي لإتاحة مكان فارغ للمسار الجديد
    int i = elts-1;			
    while (i >= max)     
	{	
		rtab[i+1] = rtab[i];
		i--;
	}


	if (max < 0)
		max = 0;



	bcopy(&ent, &rtab[max], sizeof(edvr_rtable_ent ));    //إضافة المسار الجديد بالموضع الصحيح
	//rtab[max].advertise = true;  // added 

    if ((c==1)|| (c=2 && ent.dst == rtab[max].dst))   // added by ahmad
       rtab[max].trigger_event = 0;
    
    elts++;     //elts زيادة العداد  

    return(1);
}
void 
EDVRRoutingTable::InitLoop() {//للبدء بحلقة جديدة للمرور على جدول التوجيه من أول مسار
  ctr = 0;
}

edvr_rtable_ent  *
EDVRRoutingTable::NextLoop() {//مؤشر على المسار الحالي ctr للحصول على المسار التالي بجدول التوجيه حيث   
  if (ctr >= elts)
    return 0;

  return &rtab[ctr++];
}
// للحصول على عدد المسارات بجدول التوجيه
int 
EDVRRoutingTable::number_of_elts() {
  return elts;
}
int 
EDVRRoutingTable::RemainingLoop() {//عدد المسارات المتبقية بجدول التوجيه
  return elts-ctr;
}

edvr_rtable_ent  *
EDVRRoutingTable::GetEntry(nsaddr_t dest) {       // dest  التابع الاساسي للحصول على مسار ضمن جدول توجيه العقدة الحالية للعقدة الهدف 
  edvr_rtable_ent  ent;	
  ent.dst = dest;	
  return (edvr_rtable_ent  *) bsearch(&ent, rtab, elts, sizeof(edvr_rtable_ent ), rtent_trich);  

}
edvr_rtable_ent  *
EDVRRoutingTable::GetEntry1(nsaddr_t dest, nsaddr_t via,nsaddr_t *myaddr )  // via الحصول على مسار له القفزة الأولى 
{     
	edvr_rtable_ent  ent;	// مسار فارغ بجدول التوجيه
	ent.dst = dest;	// وضع العقدة الهدف في هذا المسار
	ent.f_nxt_hop  = via ;   // وضع القفزة الأولى في هذا المسار
	 
	edvr_rtable_ent  *prte;

	//================================== جزء جديد ===============================
	//cout<<" I'm in GetEntry1***************************************************************";
	//cin.get();
    int c=2;
    int N=0;   //المقطع الأصغر
    int elts1=0;                            //ahmad
    int min1=0;	//ahmad
    int max1=0;
    if (elts > 0)		//ahmad
	{ 
		max1=elts-1;	//ahmad
		if(max1==0)
           elts1=0;
		else
		{
            elts1=round(max1/2.0);   //	elts1=ceil(max1/2);	//ahmad
		}

		do
		{
		    c = check2(&ent, &rtab[elts1],*myaddr);	//فحث العقدة الهدف مع القفزة الأولى           			
            if ((c==0)||(elts1==0)||(min1==max1))
               break;//اخرج من الحلقة 
		     
		   
            //=============== المسار غير موجود بعد ==========================================================
            if(c==1)                // المسار غير موجود وربما موجود بالبحث للأعلى  
			  max1=elts1-1;

            if(max1<min1)
			  max1=min1;

			if(c== 2) 	            // المسار غير موجود وربما موجود بالبحث للأسفل
			   min1=elts1+1;

			if(min1>max1)
			   min1=max1;

             elts1=round((min1+max1)/2.0) ;// executed in both c==1 or c==2 
			    
		 }  while(elts1>=N);
		
		 if(c==0)
		 {
			//cout<<" finaly *********************** وجد المسار أخيرا ****************************";
			prte=&rtab[elts1]; 
			return prte; //إرجاع المسار المطلوب      		
		 }
         return 0;
	}

}
edvr_rtable_ent  * 
EDVRRoutingTable::GetEntry2(nsaddr_t dest, nsaddr_t via,nsaddr_t *myaddr ) // via == f_nxt_hop    // الحصول على أفضل مسار للعقدة الهدف بغض النظر عن القفزة الأولى
{   
	edvr_rtable_ent  ent;	// مسار جديد فارغ
	ent.dst = dest;	// اسناد الهدف للمسار الجديد
	ent.f_nxt_hop  = via ;   // اسناد القفزة الأولى للمسار الجديد
	ent.metric=250;   //  نضع عدد القفزات قيمة كبيرة للحصول على المسار الأفضل
	 
	edvr_rtable_ent  *it;

	int c=2;
	int N=0;   //المقطع الأصغر
	int elts1=0;                            //ahmad
	int min1=0;	//ahmad
	int max1=0;
    if (elts > 0)		//ahmad
	{
		max1=elts-1;	//ahmad
		if(max1==0)
           elts1=0;
		else
		{
                elts1=round(max1/2.0);   //	elts1=ceil(max1/2);	//ahmad
		}

		do
		{
             c = check1(&ent, &rtab[elts1],*myaddr);	//ahmad                        
             if(c==0)
			 {//أي ان المسار موجود
				int entry_no=0;
				while(rtab[elts1].dst==ent.dst)   //  الانتقال لأول مسار يعود إلى الهدف
				{
					elts1--;
				}
				elts1++;                    // هذا عنوان أول مسار يعود للهدف 

				while(rtab[elts1].dst==ent.dst)//للحصول على أفضل مسار الذي له عدد القفزات أقل ما يمكن
				{
					if (rtab[elts1].metric<=ent.metric)	//
					{ 
						int j=elts1;                        
						//28/05/08printf("\n%3d\t%3d\t%3d\t%3d\t  %8d \t   %p  \t\t%p\t\t%p",rtab[j].dst,rtab[j].f_nxt_hop ,rtab[j].s_nxt_hop ,rtab[j].metric,rtab[j].link_no, rtab[j].trigger_event, rtab[j].timeout_event, rtab[j].q);				  
                        ent.metric= rtab[elts1].metric;
						it=&rtab[elts1]; 
					}		
					elts1++;	     
				}
			 }
             			
             if ((c==0)||(elts1==0)||(min1==max1))
               break;
		       
             //=============== المسار غير موجود بعد ==========================================================
             if(c==1)                //  المسار غير موجود وربما موجود بالبحث للأعلى  
			    max1=elts1-1;

             if(max1<min1)
			    max1=min1;

			 if(c== 2) 	            // المسار غير موجود وربما موجود بالبحث للأسفل
			    min1=elts1+1;

			 if(min1>max1)
			    min1=max1;

             elts1=round((min1+max1)/2.0) ;// executed in both c==1 or c==2 
			    
		}  while(elts1>=N);
		
		if(c==0 && (it->dst>=0&&it->dst<=250))
        // if(c==0 && (it->dst>=0))
		{
			//cout<<" finaly *********************** اخيرا وجد المسار ****************************";
			//t<<"\n The best route to node (" << it->dst << ") is .....     " << it->dst << "    " << it->f_nxt_hop  << "    " << it->s_nxt_hop  << "    " << it->metric <<"    " << it->link_no<<"    " << it->changed_at<<"    " << it->timeout_event<<"    " << it->q<<"    " << it->advertise<<"    " << it->packet_id;                         
	
			return it;//ارجاع المسار
		}
		
	}
	return 0;
} 
int     //=========================== بداية تابع ترتيب جدول التوجيه ==========================================================================
EDVRRoutingTable::Sort_Entry(const edvr_rtable_ent  &ent,int first_entry_position, nsaddr_t nod_id)   //  new function // added by ahmad 
{
	//هذا التابع يستخدم فقط عندما مسار جديد يستبدل مسار قديم في هذه  الحالة بعض المسارات الجديدة ستكون بالمكان الغلط 
  
 
	int sort=1; 
	edvr_rtable_ent  rte1;
    bzero(&rte1, sizeof(rte1));				
										
	do
	{
		sort=1;   // flag ;للإستمرار أو لإيقاف الترتيب
		int kk= first_entry_position;  // أول موضع للمسار الجديد 
			                           //لا يوجد داعي لترتيب كل جدول التوجيه فقط الجزء حيث تم اضافة المسار الجديد
			                           // لان جدول التوجيه مرتب مسبقا قبل تعديل المسار الجديد

		while((rtab[kk].dst==ent.dst)&& (rtab[kk+1].dst==ent.dst))  // رتب فقط جزء محدد من جدول التوجيه حيث تمت إضافة المسار الجديد
		{
			if ((rtab[kk].f_nxt_hop >rtab[kk+1].f_nxt_hop )&& (kk+1<elts)) // في كل مرة أفحص مسارين إذا مرتبين، وإذا لا اقلبهم
			{
																				   
				int j=kk;
				//cout<<"\n*** NOTE:   Table Not SORTED Well ...  Therfore I'm going to swap the entry ... In position No. "<<j<<"\n";
				//printf("%3d\t%3d\t%3d\t%3d\t  %8d \t  %.8f \t %p  \t\t%p\t\t%p \n",rtab[j].dst,rtab[j].f_nxt_hop ,rtab[j].s_nxt_hop ,rtab[j].metric,rtab[j].link_no,rtab[j].changed_at,rtab[j].trigger_event,rtab[j].timeout_event,rtab[j].q);				                               
				j++;
				//<<"With the entry ........ In position No. "<<j<<" because they are not sorted .......\n";
				//printf("%3d\t%3d\t%3d\t%3d\t  %8d \t  %.8f \t %p  \t\t%p\t\t%p",rtab[j].dst,rtab[j].f_nxt_hop ,rtab[j].s_nxt_hop ,rtab[j].metric,rtab[j].link_no,rtab[j].changed_at,rtab[j].trigger_event,rtab[j].timeout_event,rtab[j].q);				    
									 
				//cout << "\n\n Routing table of node (" << nod_id << ")  consists of " << number_of_elts() << " entries before swaping .... \n";
				//Show_rt();
												
				sort=0;

				//=======================( القيام بعملية التبديل)=================================         
				bcopy(&rtab[kk],&rte1,sizeof(edvr_rtable_ent ));                         								
				rtab[kk] = rtab[kk+1];
				bcopy(&rte1,&rtab[kk+1], sizeof(edvr_rtable_ent ));    
				//=======================( انتهاء عملية التبديل )==========================   
										 
				//cout << "\n\n Routing table of node (" << nod_id << ")  consists of " << number_of_elts() << " entries after swaping .... \n";
				//Show_rt();
								                      
			}

			kk++;   
		}
	}while (sort==0);

return 0;
			      
}   //=========================== نهاية تابع الترتيب ==============================================
void EDVRRoutingTable::Show_rt()   //  تابع لعرض جدول التوجيه
{
   edvr_rtable_ent *krte;
   //cout << "\n Routing table of node ( " << *xxx << " )  consists of " << elts << " entries after Addentry() function is called .... \n";
 //  cout << "--------------------------------------------------------------------------------------------------------------------------------------------\n";
   //cout << " Dst   1-hop  2-hop   metric   link-no	  change at        timeout_event        Queed-Data	      Need Advertise          Sent_RREQ\n";
   //cout << "--------------------------------------------------------------------------------------------------------------------------------------------";

	for (InitLoop (); (krte = NextLoop ()); )
          printf("\n %d %7d %7d %7d  %8d %3s %.8f %-10s %-6p %-12s %-6p %-12s %-6p %-12s %d",krte->dst,krte->f_nxt_hop ,krte->s_nxt_hop ,krte->metric,krte->link_no,"   ",krte->changed_at,"          ",krte->timeout_event,"            ",krte->q,"            ",krte->advertise,"            ",krte->packet_id);
	 //     cout << "\n--------------------------------------------------------------------------------------------------------------------------------------------\n";

} 
 /*  cout << "--------------------------------------------------------------------------------------------------------------------------------------------\n";
   cout << " dst   f_nxt_hop   s_nxt_hop    metric   link no	  change at    trigger_event   timeout_event       q	\n";
   cout << "--------------------------------------------------------------------------------------------------------------------------------------------";

	for (InitLoop (); (krte = NextLoop ()); )
	{
 		printf("\n%3d\t%3d\t%3d\t%3d\t  %8d \t  %.8f \t %p  \t\t%p\t\t%p\t\t%p ",krte->dst,krte->f_nxt_hop ,krte->s_nxt_hop ,krte->metric,krte->link_no,krte->changed_at,krte->trigger_event,krte->timeout_event,krte->q,krte->advertise);
		if (krte->timeout_event)
			cout << krte->timeout_event->time_;
	}

	cout << "\n--------------------------------------------------------------------------------------------------------------------------------------------\n";
	*/

