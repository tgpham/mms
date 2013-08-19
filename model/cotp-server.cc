#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "mms-header.h"
#include "mms-adapt-server.h"
#include "mms-server.h"
#include "cotp-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CotpServer");
NS_OBJECT_ENSURE_REGISTERED (CotpServer);

TypeId 
CotpServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CotpServer")
    .SetParent<Object> ()
    .AddConstructor<CotpServer> ()
    ;
  return tid;
}

CotpServer::CotpServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_SrcRefServer = 0;
  m_SrcRefClient = 0;
  m_tcpPort = 102;
  m_adaptCotpserviceEvent = EventId ();

 // For debugging purporse
 // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CWS created!");
}

CotpServer::~CotpServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
CotpServer::Init ()
{
  // Create the socket
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (m_mmsAdaptServer->GetMmsServer ()->GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::ConvertFrom(m_localAddress), m_tcpPort);
      m_socket->Bind (local);
      m_socket->Listen();

      m_socket->SetAcceptCallback (
            MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
            MakeCallback (&CotpServer::HandleAccept, this));

      // Set the callback method ("Adapt" Recv Tcp funtion to UDP-DATA.ind (APDU))
      //m_socket->SetRecvCallback (MakeCallback (&CotpServer::Recv, this));

      // For debugging purporses
      // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s UDPSocketServer created with Ip " << Ipv4Address::ConvertFrom (m_localAddress));
    }
}

void CotpServer::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&CotpServer::Recv, this));
}

void 
CotpServer::Recv (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  CotpTypeHeader cotpTypeHeader;
  // Retrieve the packet sent by the Remote Client and copy the Wrapper Header from the packet
  packet = socket->RecvFrom (from);  
  packet->RemoveHeader (cotpTypeHeader);
  int type = cotpTypeHeader.GetCotpPduType();
  m_remoteAddress = Address (InetSocketAddress::ConvertFrom (from).GetIpv4 ());
  // Adapt RECEIVE UDP function to UDP-DATA.ind (APDU) service
  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpServer::CotpAdaptServices, this, socket, type, INDICATION, packet);
}

void
CotpServer::Send (Ptr<Socket> socket, Ptr<Packet> packet)
{

  // Connect the SAP with the remote CAP 
//  if (Ipv4Address::IsMatchingType(m_remoteAddress) == true)
//    {
//      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_remoteAddress), m_tcpPort)); // m_tcpPort same as Client
//      // For debugging purposes
//      // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s Server connected with Client " << Ipv4Address::ConvertFrom(m_remoteAddress));
//    }
//
  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpServer::CotpAdaptServices, this, socket, DT, REQUEST, packet);

}

void 
CotpServer::CotpAdaptServices (Ptr<Socket> socket, int cotptype, int type_service, Ptr<Packet> packet)
{
//	NS_ASSERT (m_adaptCotpserviceEvent.IsExpired ());
//	Simulator::Cancel (m_adaptCotpserviceEvent);

	if (type_service == INDICATION) // Receive packet from client
	{
		if (cotptype == CRE) // Receive CR, create COTP CC packet
		{
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-S (" << Ipv4Address::ConvertFrom (m_localAddress)
			<< ") <-- received CR ("
			<< packet->GetSize () << "B)");

			// Create dummy packet and add COTP header
			Ptr<Packet> ccPkt = Create<Packet>();
			CotpHeader cotpHeader;
			cotpHeader.SetSrcRef(m_SrcRefServer);
			cotpHeader.SetDstRef(m_SrcRefClient);
			cotpHeader.SetDstTsap(0x0001);
			cotpHeader.SetSrcTsap(0x0000);
			cotpHeader.SetCotpParamField1(0x00c0010a);
			cotpHeader.SetCotpParamField2(0xc202);
			cotpHeader.SetCotpParamField3(0xc102);
			ccPkt->AddHeader (cotpHeader);

			// Add COTP Type header
			CotpTypeHeader cotpTypeHeader;
			cotpTypeHeader.SetCotpPduType(0xd0); //CC
			cotpTypeHeader.SetTptkLength(22);
			cotpTypeHeader.SetCotpLength(17);
			ccPkt->AddHeader (cotpTypeHeader);
			double t = (8*ccPkt->GetSize())/(500000) + 2.235e-3; // assuming an ideal PLC channel
			m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (t), &CotpServer::CotpAdaptServices, this, socket, CC, REQUEST, ccPkt);
			//socket->Send (packet);
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-S (" << Ipv4Address::ConvertFrom (m_localAddress)
			<< ") --> sent CC ("
			<< packet->GetSize () << "B)");
		}

		if (cotptype == DT) //receive the data, notify the adapt layer
		{
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-S (" << Ipv4Address::ConvertFrom (m_localAddress)
			                                                   << ") <-- DT ("
			                                                   << packet->GetSize () << "B)");
			CotpDtHeader cotpDtHeader;
			packet->RemoveHeader(cotpDtHeader);
			m_mmsAdaptServer->RecvSpdu(socket, packet);
		}
	}

	if (type_service == REQUEST) // Send packet to client
	{
		if (cotptype == DT)
		{
			// Add COTP DT header
			CotpDtHeader cotpDtHeader;
			packet->AddHeader(cotpDtHeader);

			// Add COTP Type header
			CotpTypeHeader cotpTypeHeader;
			cotpTypeHeader.SetCotpPduType(0xf0);
			cotpTypeHeader.SetTptkLength(packet->GetSize()+6);
			cotpTypeHeader.SetCotpLength(2);
			packet->AddHeader (cotpTypeHeader);
			// Call SEND through socket
			socket->Send (packet);
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-S (" << Ipv4Address::ConvertFrom (m_localAddress)
			<< ") --> DT ("
			<< packet->GetSize () << "B)");
		}

		if (cotptype == CC)
		{
			socket->Send(packet);
		}

	}

}

void
CotpServer::SetSocket (Ptr<Socket> socket)
{
  m_socket = socket;
}

Ptr<Socket>
CotpServer::GetSocket ()
{
  return m_socket;
}

void 
CotpServer::SetMmsAdaptServer (Ptr<MmsAdaptServer> mmsAdaptServer)
{
  m_mmsAdaptServer = mmsAdaptServer;
}

Ptr<MmsAdaptServer>
CotpServer::GetMmsAdaptServer ()
{
  return m_mmsAdaptServer;
}

void
CotpServer::SetSrcRefServer (Ptr<MmsServer> mmsServer)
{
	mmsServer->SetSrcRef (m_SrcRefServer++);
}

uint16_t
CotpServer::GetSrcRefServer ()
{
 return m_SrcRefServer;
}

uint16_t
CotpServer::GetSrcRefClient ()
{
 return m_SrcRefClient;
}

void 
CotpServer::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
CotpServer::GetTcpport ()
{
  return m_tcpPort;
}


void 
CotpServer::SetLocalAddress (Address ip)
{
  m_localAddress = ip;
}

Address
CotpServer::GetLocalAddress ()
{
  return m_localAddress;
}

void 
CotpServer::SetRemoteAddress (Address ip)
{
  m_localAddress = ip;
}

Address
CotpServer::GetRemoteAddress ()
{
  return m_localAddress;
}

} // namespace ns3
