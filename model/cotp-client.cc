#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "mms-header.h"
#include "mms-server.h"
#include "mms-adapt-client.h"
#include "mms-client.h"
#include "cotp-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CotpClient");
NS_OBJECT_ENSURE_REGISTERED (CotpClient);

TypeId 
CotpClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CotpClient")
    .SetParent<Object> ()
    .AddConstructor<CotpClient> ()
    ;
  return tid;
}

CotpClient::CotpClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_srcRefClient = 0;
  m_srcRefServer = 0;
  m_tcpPort = 1082;
  EventId m_adaptCotpserviceEvent = EventId ();

  // For debugging purposes
  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CWC created!");
}

CotpClient::~CotpClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
CotpClient::Recv (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  CotpTypeHeader cotpTypeHeader;
  // Retrieve the packet sent by the Remote Server and copy the Header from the packet
  packet = socket->RecvFrom (from);  // Check while???
  packet->RemoveHeader (cotpTypeHeader);

  if (cotpTypeHeader.GetCotpPduType() == CC) // CC
  {
	  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpClient::CotpAdaptServices, this, socket, CC, INDICATION, packet, from);
  }

  else if (cotpTypeHeader.GetCotpPduType() == DT) // DT
  {
	  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpClient::CotpAdaptServices, this, socket, DT, INDICATION, packet, from);
  }

  else packet->RemoveAtStart(packet->GetSize());

}

void
CotpClient::Send (Ptr<Socket> socket, Ptr<Packet> packet, int type, Address from)
{ 
  // Retrieve the peer address
  m_remoteAddress = from;
//  mmsServer->GetLocalAddress ();
//  uint16_t dstRef = mmsServer->GetSrcRef ();
  uint16_t srcRef = m_mmsAdaptClient->GetMmsClient ()->GetSrcRef ();
  Ptr<Socket> temp_socket;
  // Create the socket if not already
  if (socket == NULL)
  {
	  TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");

	  temp_socket = Socket::CreateSocket (m_mmsAdaptClient->GetMmsClient ()->GetNode (), tid);
	  m_tcpPort++;
	  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_tcpPort);
	  temp_socket->Bind (local);

	  // For debugging purposes
	  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s UDPSocketClient created with Ip " << Ipv4Address::ConvertFrom(m_localAddress));

	  temp_socket->SetRecvCallback (MakeCallback (&CotpClient::Recv, this));
	  // Connect the CAP with the remote SAP
	  if (Ipv4Address::IsMatchingType(m_remoteAddress) == true)
	  {
		  temp_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_remoteAddress), 102)); // m_tcpPort of Server
		  // For debugging purposes
		  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s Client connected with Server " << Ipv4Address::ConvertFrom(m_remoteAddress));
	  }
  }
  else temp_socket = socket;

  if (type == CN)
  {
	  // Create dummy packet and add COTP header
	  Ptr<Packet> crPkt = Create<Packet>();
	  CotpHeader cotpHeader;
	  cotpHeader.SetSrcRef(srcRef);
	  cotpHeader.SetDstRef(0x0001);
	  cotpHeader.SetDstTsap(0x0001);
	  cotpHeader.SetSrcTsap(srcRef);
	  cotpHeader.SetCotpParamField1(0x00c0010a);
	  cotpHeader.SetCotpParamField2(0xc202);
	  cotpHeader.SetCotpParamField3(0xc102);
	  crPkt->AddHeader (cotpHeader);

	  // Add COTP Type header
	  CotpTypeHeader cotpTypeHeader;
	  cotpTypeHeader.SetTptkLength(22);
	  cotpTypeHeader.SetCotpLength(17);
	  crPkt->AddHeader (cotpTypeHeader);
	  Address temp;
	  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpClient::CotpAdaptServices, this, temp_socket, CRE, REQUEST, crPkt, temp);
  }

  if (type == DTD)
  {
	  Address temp;
	  m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (0.0), &CotpClient::CotpAdaptServices, this, temp_socket, DT, REQUEST, packet, temp);
  }

}


void 
CotpClient::CotpAdaptServices (Ptr<Socket> socket, int cotptype, int type_service, Ptr<Packet> packet, Address from)
{
//	NS_ASSERT (m_adaptCotpserviceEvent.IsExpired ());
//	Simulator::Cancel (m_adaptCotpserviceEvent);

	if (cotptype == CRE)
	{

		// Call SEND Udp function (through UdpSocket)
		socket->Send (packet);
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-C (" << Ipv4Address::ConvertFrom (m_localAddress)
		<< ") --> sent CR ("
		<< packet->GetSize () << "B)");
	}

	if (cotptype == CC) //Receive the COTP CC, create the MMS Initiate Request packet
	{
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-C (" << Ipv4Address::ConvertFrom (m_localAddress)
		<< ") <-- received CC ("
		<< packet->GetSize () << "B)");
		Ptr<Packet> initPkt = Create<Packet>();
		MmsInitiateRequestHeader mmsInitiateRequestHeader;
		initPkt->AddHeader(mmsInitiateRequestHeader);
		MmsAcseAarqHeader mmsAcseAarqHeader;
		initPkt->AddHeader(mmsAcseAarqHeader);
		MmsPresentationInitRequestHeader mmsPresentationInitRequestHeader;
		initPkt->AddHeader(mmsPresentationInitRequestHeader);
		MmsSessionHeader mmsSessionHeader;
		initPkt->AddHeader(mmsSessionHeader);
		MmsSessionTypeHeader mmsSessionTypeHeader;
		initPkt->AddHeader(mmsSessionTypeHeader);
		MmsPresentationInitRequestHeader mmsPresentationHeader;
		double t = (8*initPkt->GetSize())/(500000) + 2.235e-3;
		Address temp;
		m_adaptCotpserviceEvent = Simulator::Schedule (Seconds (t), &CotpClient::CotpAdaptServices, this, socket, DT, REQUEST, initPkt, temp);
	}

	if (cotptype == DT)
	{

		if (type_service == REQUEST) // Send the data
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

			// Call SEND Udp function (through UdpSocket)
			socket->Send (packet);
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-C (" << Ipv4Address::ConvertFrom (m_localAddress)
			<< ") --> DT ("
			<< packet->GetSize () << "B)");
		}

		if (type_service == INDICATION) // Receive the data
		{
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s COTP-C (" << Ipv4Address::ConvertFrom (m_localAddress)
			                                                   << ") <-- DT ("
			                                                   << packet->GetSize () << "B)");
			if (packet->GetSize() >= 51)
			{
			CotpDtHeader cotpDtHeader;
			packet->RemoveHeader(cotpDtHeader);
			m_mmsAdaptClient->RecvSpdu(socket, packet, from);
			}
			else (packet->RemoveAtStart(packet->GetSize()));
		}



	}
}

void
CotpClient::SetSocket (Ptr<Socket> socket)
{
  // Send the packet to the lower layers
  m_socket = socket;
}

Ptr<Socket>
CotpClient::GetSocket ()
{
  return m_socket;
}

void 
CotpClient::SetMmsAdaptClient (Ptr<MmsAdaptClient> mmsAdaptClient)
{
  m_mmsAdaptClient = mmsAdaptClient;
}

Ptr<MmsAdaptClient>
CotpClient::GetMmsAdaptClient ()
{
  return m_mmsAdaptClient;
}

void
CotpClient::SetSrcRefClient (Ptr<MmsClient> mmsClient)
{
  mmsClient->SetSrcRef (m_srcRefClient++);
}

uint16_t
CotpClient::GetSrcRefClient ()
{
 return m_srcRefClient;
}

uint16_t
CotpClient::GetSrcRefServer ()
{
 return m_srcRefServer;
}

void 
CotpClient::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
CotpClient::GetTcpport ()
{
  return m_tcpPort;
}

void 
CotpClient::SetLocalAddress (Address ip)
{
  m_localAddress = ip;
}

Address
CotpClient::GetLocalAddress ()
{
  return m_localAddress;
}

void 
CotpClient::SetRemoteAddress (Address ip)
{
  m_remoteAddress = ip;
}

Address
CotpClient::GetRemoteAddress ()
{
  return m_remoteAddress;
}

} // namespace ns3
