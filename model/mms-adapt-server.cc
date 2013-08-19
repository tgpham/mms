#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "mms-header.h"
#include "mms-adapt-server.h"
#include "mms-server.h"
#include "cotp-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmsAdaptServer");
NS_OBJECT_ENSURE_REGISTERED (MmsAdaptServer);

TypeId 
MmsAdaptServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsAdaptServer")
    .SetParent<Object> ()
    .AddConstructor<MmsAdaptServer> ()
    ;
  return tid;
}

MmsAdaptServer::MmsAdaptServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_typeService = REQUEST;
  m_typeGet = 0;
  m_stateCf = CF_INACTIVE;
  m_tcpPort = 102;
  m_changeStateEvent = EventId ();
  m_sendApduEvent = EventId ();
  m_invokeMmsAdaptService = EventId ();

  // For debuggin purposes
  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAL created!");
}

MmsAdaptServer::~MmsAdaptServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
MmsAdaptServer::MmsAdapt(Ptr<Socket> socket, int mmsService, int typeService, Ptr<Packet> packet)
{
	if (typeService == REQUEST)
	{
		if (mmsService == INITIATE)
		{
			MmsSessionInitResponseHeader mmsSessionInitResponseHeader;
			packet->AddHeader(mmsSessionInitResponseHeader);
			MmsSessionTypeHeader mmsSessionTypeHeader;
			mmsSessionTypeHeader.SetIdSpdu(AC);
			mmsSessionTypeHeader.SetLength(0x99);
			packet->AddHeader(mmsSessionTypeHeader);

			m_changeStateEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::SetStateCf, this, CF_ASSOCIATED);
			m_sendApduEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::sendApdu, this, socket, packet);
		}

		if (mmsService == CONFIRMED)
		{
			m_sendApduEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::sendApdu, this, socket, packet);
		}

		if (mmsService == UNCONFIRMED_SRV)
		{
			m_sendApduEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::sendApdu, this, socket, packet);
		}

	}
}

void 
MmsAdaptServer::sendApdu (Ptr<Socket> socket, Ptr<Packet> packet)
{
//  NS_ASSERT (m_sendApduEvent.IsExpired ());
//  Simulator::Cancel (m_sendApduEvent);
  m_cotpServer->Send (socket, packet);
}

void
MmsAdaptServer::RecvSpdu (Ptr<Socket> socket, Ptr<Packet> packet)
{
	// Remove the Session layer header
	MmsSessionTypeHeader mmsSessionTypeHeader;
	packet->RemoveHeader(mmsSessionTypeHeader);
	if (mmsSessionTypeHeader.GetIdSpdu() == CN)
	{
		// Event: Inform to the SAP that a remote CAP has requested an establishment of an AA
		Simulator::Schedule (Seconds (0.0), &MmsServer::Recv, m_mmsServer, socket, CN, INDICATION, packet); // Check if it really works?????
		// Event: Change the state of SAL to ASSOCIATION_PENDING
		m_changeStateEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::SetStateCf, this, CF_ASSOCIATION_PENDING);
	}

	if (mmsSessionTypeHeader.GetIdSpdu() == DTD)
	{
		packet->RemoveHeader(mmsSessionTypeHeader); //Remove one more
		Simulator::Schedule (Seconds (0.0), &MmsServer::Recv, m_mmsServer, socket, DTD, INDICATION, packet);
	}
}

void
MmsAdaptServer::UnconfirmedService (Ptr<Socket> socket, Ptr<Packet> packet)
{
	MmsPresentationHeader mmsPresentationHeader;
	mmsPresentationHeader.SetLength(packet->GetSize());
	packet->AddHeader(mmsPresentationHeader);

	MmsSessionTypeHeader mmsSessionTypeHeader;
	mmsSessionTypeHeader.SetIdSpdu(0x01);
	mmsSessionTypeHeader.SetLength(00);
	packet->AddHeader(mmsSessionTypeHeader);
	packet->AddHeader(mmsSessionTypeHeader);

	m_invokeMmsAdaptService = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::MmsAdapt, this, socket, UNCONFIRMED_SRV, REQUEST, packet);

}

void
MmsAdaptServer::ConfirmedResponse(Ptr<Socket> socket, int typeConfirmed, Ptr<Packet> packet)
{
	if (typeConfirmed == WRITE)
	{
		MmsPresentationHeader mmsPresentationHeader;
		mmsPresentationHeader.SetLength(packet->GetSize());
		packet->AddHeader(mmsPresentationHeader);

		MmsSessionTypeHeader mmsSessionTypeHeader;
		mmsSessionTypeHeader.SetIdSpdu(0x01);
		mmsSessionTypeHeader.SetLength(00);
		packet->AddHeader(mmsSessionTypeHeader);
		packet->AddHeader(mmsSessionTypeHeader);

		m_invokeMmsAdaptService = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::MmsAdapt, this, socket, CONFIRMED, REQUEST, packet);
	}

	if (typeConfirmed == READ)
	{
		MmsPresentationHeader mmsPresentationHeader;
		mmsPresentationHeader.SetLength(packet->GetSize());
		packet->AddHeader(mmsPresentationHeader);

		MmsSessionTypeHeader mmsSessionTypeHeader;
		mmsSessionTypeHeader.SetIdSpdu(0x01);
		mmsSessionTypeHeader.SetLength(00);
		packet->AddHeader(mmsSessionTypeHeader);
		packet->AddHeader(mmsSessionTypeHeader);

		m_invokeMmsAdaptService = Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::MmsAdapt, this, socket, CONFIRMED, REQUEST, packet);
	}
}

void 
MmsAdaptServer::SetMmsServer (Ptr<MmsServer> mmsServer)
{
  m_mmsServer = mmsServer;
}
	
Ptr<MmsServer>
MmsAdaptServer::GetMmsServer ()
{
  return m_mmsServer;
}

void 
MmsAdaptServer::SetCotpServer (Ptr<CotpServer> cotpServer)
{
  m_cotpServer = cotpServer;
}
	
Ptr<CotpServer>
MmsAdaptServer::GetCotpServer ()
{
  return m_cotpServer;
}

void 
MmsAdaptServer::SetStateCf (int state)
{
  NS_ASSERT (m_changeStateEvent.IsExpired ());
  Simulator::Cancel (m_changeStateEvent);  // necessary?

  if (state == CF_IDLE)
    {
      m_stateCf = state;
      NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAL (" << Ipv4Address::ConvertFrom (m_cotpServer->GetLocalAddress ())
                                                   << ") --> IDLE");
     /* Ptr<Socket> skt = m_cotpServer->GetSocket ();
      if (skt != 0) 
        {
          Simulator::Schedule (Seconds (0.00001), &Socket::Close, skt); // Close a socket
          NS_LOG_INFO ("Socket closed!!");
        }*/
    } 
  else if (state == CF_ASSOCIATION_PENDING)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAL (" << Ipv4Address::ConvertFrom (m_cotpServer->GetLocalAddress ())
                                                     << ") --> ASSOCIATION_PENDING");
      }
  else if (state == CF_ASSOCIATED)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAL (" << Ipv4Address::ConvertFrom (m_cotpServer->GetLocalAddress ())
                                                     << ") --> ASSOCIATED");
      } 
  else if (state == CF_ASSOCIATION_RELEASE_PENDING)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAL (" << Ipv4Address::ConvertFrom (m_cotpServer->GetLocalAddress ())
                                                     << ") --> ASSOCIATION_RELEASE_PENDING");
      } 
  else
    {
      NS_LOG_INFO ("Error: No change of stare AL");
    }
}

int 
MmsAdaptServer::GetStateCf ()
{
  return m_stateCf;
}
	
int 
MmsAdaptServer::GetTypeService ()
{
  return m_typeService;
}

void 
MmsAdaptServer::SetTypeService (int typeService)
{
  m_typeService = typeService;
}
	
int 
MmsAdaptServer::GetTypeGet ()
{
  return m_typeGet;
}

void 
MmsAdaptServer::SetTypeGet (int typeGet)
{
  m_typeGet = typeGet;
}
 
void 
MmsAdaptServer::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
MmsAdaptServer::GetTcpport ()
{
  return m_tcpPort;
}

void 
MmsAdaptServer::Init ()
{
  m_cotpServer-> Init ();
}
	
} // namespace ns3
