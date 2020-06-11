/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "tcp-elastic.h"
#include "ns3/log.h" 
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpElastic");

NS_OBJECT_ENSURE_REGISTERED (TcpElastic);

TypeId TcpElastic::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpElastic")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpElastic> ()
    .SetGroupName ("Internet")
   ;
   return tid; 
}

std::string TcpElastic::GetName () const
{
  return "TcpElastic";
}

TcpElastic::TcpElastic (void)
  : TcpNewReno (),
    m_baseRtt (Time::Max ()),
    m_maxRtt (Time::Min ()),
    m_curRtt (Time::Min ()),
    m_mdFactor (0.5),
    m_ackCnt (0)
{
  NS_LOG_FUNCTION (this);
}

TcpElastic::TcpElastic (const TcpElastic& sock)
  : TcpNewReno (sock),
    m_baseRtt (sock.m_baseRtt),
    m_maxRtt (sock.m_maxRtt),
    m_curRtt (sock.m_curRtt),
    m_mdFactor (sock.m_mdFactor),
    m_ackCnt (sock.m_ackCnt)
{
  NS_LOG_FUNCTION (this);
}

TcpElastic::~TcpElastic (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<TcpCongestionOps> TcpElastic::Fork (void)
{
  NS_LOG_FUNCTION (this);
  return CopyObject<TcpElastic> (this);
}

double TcpElastic::Wwf (Ptr<TcpSocketState> tcb)
{
  double ratio = (m_maxRtt.GetSeconds()/m_curRtt.GetSeconds())*tcb->GetCwndInSegments ();
  double s = std::sqrt(ratio);
  return s;
}


void TcpElastic::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  // NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  // if (segmentsAcked > 0)
  // {
  //   double adder = static_cast<double> (tcb->m_segmentSize * /*tcb->m_segmentSize*/  Wwf(tcb)) / tcb->m_cWnd.Get ();
  //   //double adder = Wwf(tcb)/tcb->m_cWnd.Get();
  //   adder = std::max (1.0, adder);
  //   tcb->m_cWnd += static_cast<uint32_t> (adder);
  //   NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<" ssthresh " << tcb->m_ssThresh);
  // }
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  uint32_t segCwnd = tcb->GetCwndInSegments ();
  uint32_t oldCwnd = segCwnd;

  if (segmentsAcked > 0)
    {
      double  coeffA = Wwf (tcb);
      m_ackCnt += segmentsAcked * coeffA;
    }

  while (m_ackCnt >= segCwnd)
    {
      m_ackCnt -= segCwnd;
      segCwnd += 1;
    }

  if (segCwnd != oldCwnd)
    {
      tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }
}

void TcpElastic::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
  {
    if (segmentsAcked >= 1)
    {
      tcb->m_cWnd += tcb->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      segmentsAcked = segmentsAcked - 1;
    }
  }
  if(tcb->m_cWnd >= tcb->m_ssThresh)
  {
    CongestionAvoidance (tcb, segmentsAcked);
  }
}

uint32_t TcpElastic::GetSsThresh (Ptr<const TcpSocketState> tcb,uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);

  // m_maxRtt = Time:: Min();
  // m_baseRtt = Time:: Max();

  uint32_t segCwnd = bytesInFlight / tcb->m_segmentSize;

  uint32_t ssThresh = static_cast<uint32_t> (std::max (2.0, segCwnd/2.0));
  ssThresh = ssThresh - 1;

  return  ssThresh * tcb->m_segmentSize;
}

void TcpElastic::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,const Time &rtt)
{
    NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);

  if (rtt.IsZero ())
  {
    //tcb->m_cWnd = tcb->m_cWnd.Get ()/2.0;
    return;
  }


  if(tcb->m_cWnd >= tcb->m_ssThresh)
  {
    m_curRtt = rtt;

    m_maxRtt = std::max (m_maxRtt, rtt);
    NS_LOG_DEBUG ("Updated m_minRtt = " << m_maxRtt);

    m_baseRtt = std::min (m_baseRtt, rtt);
    NS_LOG_DEBUG ("Updated m_baseRtt = " << m_baseRtt);
  }
  else{
    m_curRtt = rtt;
    m_maxRtt = Time:: Min();
    m_baseRtt = Time:: Max();
  }
}

}
