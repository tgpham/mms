#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "mms-header.h"
#include "mms-adapt-client.h"
#include "mms-server.h"
#include "mms-client.h"
#include "mms-adapt-server.h"
#include "cotp-client.h"
#include "ns3/inet-socket-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmsAdaptClient");
NS_OBJECT_ENSURE_REGISTERED (MmsAdaptClient);

TypeId 
MmsAdaptClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsAdaptClient")
    .SetParent<Object> ()
    .AddConstructor<MmsAdaptClient> ()
    ;
  return tid;
}

MmsAdaptClient::MmsAdaptClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_typeService = REQUEST;
  m_typeGet = 0;
  m_stateCf = CF_INACTIVE;
  m_tcpPort = 102;
  m_changeStateEvent = EventId ();
  m_sendApduEvent = EventId ();
  m_invokeMmsAdaptService = EventId ();
  // For debugging purposes
  //NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CAL created!");
}

MmsAdaptClient::~MmsAdaptClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
MmsAdaptClient::MmsAdapt(Ptr<Socket> socket, int mmsService, int typeService, Address from, Ptr<Packet> packet)
{
	if (typeService == REQUEST)
	{
		if (mmsService == INITIATE)
		{
			NS_LOG_INFO (Simulator::Now().GetSeconds () << "s MMS-C " << Ipv4Address::ConvertFrom (this->GetMmsClient()->GetLocalAddress()) << " INITIATE-REQUEST MMS-S "
								<< Ipv4Address::ConvertFrom (from));
			ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			out << Simulator::Now().GetSeconds () << " MMS-C " << Ipv4Address::ConvertFrom (this->GetMmsClient()->GetLocalAddress()) << " INITIATE-REQUEST MMS-S "
					<< Ipv4Address::ConvertFrom (from) << endl;
			// Event: Change the state of CAL to ASSOCIATION_PENDING
			m_changeStateEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::SetStateCf, this, CF_ASSOCIATION_PENDING);

			// Event: Request an AA establishment: Construct the AARQ APDU of ACSE service

			m_sendApduEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::sendApdu, this, socket, packet, from);
		}
		if (mmsService == CONFIRMED)
		{
			m_sendApduEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::sendConfirmedRequestApdu, this, socket, packet, from);
		}
	}
}

void
MmsAdaptClient::ConfirmedRequest (Ptr<Socket> socket, int mmsService, Address from, Ptr<Packet> packet, uint8_t m_nReq)
{
	NS_LOG_INFO (Simulator::Now().GetSeconds() << "s MMS-C " << Ipv4Address::ConvertFrom (this->GetMmsClient()->GetLocalAddress())
			<< " CONFIRMED-REQUEST MMS-S "
			<< InetSocketAddress::ConvertFrom (from).GetIpv4());
	ofstream out ((ClientFile1.str ()).c_str (), ios::app);
	out << Simulator::Now().GetSeconds() << " MMS-C " << Ipv4Address::ConvertFrom (this->GetMmsClient()->GetLocalAddress())
	<< " CONFIRMED-REQUEST MMS-S "
	<< InetSocketAddress::ConvertFrom (from).GetIpv4() << endl;

	if (mmsService == READ)
	{
		MmsConfirmedReadRequestHeader mmsConfirmedReadRequestHeader;
		packet->AddHeader(mmsConfirmedReadRequestHeader);

		MmsTypeHeader mmsTypeHeader;
		mmsTypeHeader.SetType(CONFIRMED_REQUEST, READ, m_nReq);
		packet->AddHeader(mmsTypeHeader);
	}

	if (mmsService == WRITE)
	{
		MmsConfirmedWriteRequestHeader mmsConfirmedWriteRequestHeader;
		packet->AddHeader(mmsConfirmedWriteRequestHeader);

		MmsTypeHeader mmsTypeHeader;
		mmsTypeHeader.SetType(CONFIRMED_REQUEST, WRITE, m_nReq);
		packet->AddHeader(mmsTypeHeader);
	}

	MmsPresentationHeader mmsPresentationHeader;
	mmsPresentationHeader.SetLength(packet->GetSize());
	packet->AddHeader(mmsPresentationHeader);

	MmsSessionTypeHeader mmsSessionTypeHeader;
	mmsSessionTypeHeader.SetIdSpdu(0x01);
	mmsSessionTypeHeader.SetLength(00);
	packet->AddHeader(mmsSessionTypeHeader);
	packet->AddHeader(mmsSessionTypeHeader);

	m_invokeMmsAdaptService = Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::MmsAdapt, this, socket, CONFIRMED, REQUEST, from, packet);
	
}



void
MmsAdaptClient::RecvSpdu (Ptr<Socket> socket, Ptr<Packet> packet, Address from)
{
	// Remove the Session layer header
	MmsSessionTypeHeader mmsSessionTypeHeader;
	packet->RemoveHeader(mmsSessionTypeHeader);
	Ptr<MmsServer> currentMmsServer =  m_mmsClient->GetCurrentMmsServer();
	if (mmsSessionTypeHeader.GetIdSpdu() == AC)
	{
		// Event: Inform to the MMS Client that a remote Server has accepted the request
		Simulator::Schedule (Seconds (0.0), &MmsClient::Recv, m_mmsClient, socket, AC, packet, currentMmsServer, from);
		m_changeStateEvent = Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::SetStateCf, this, CF_ASSOCIATED);
	}

	if (mmsSessionTypeHeader.GetIdSpdu() == DTD)
	{
		//Remove one more
		packet->RemoveHeader(mmsSessionTypeHeader);
		Simulator::Schedule (Seconds (0.0), &MmsClient::Recv, m_mmsClient, socket, DTD, packet, currentMmsServer, from);
	}
}

void 
MmsAdaptClient::sendApdu (Ptr<Socket> socket, Ptr<Packet> packet,  Address from)
{
//  NS_ASSERT (m_sendApduEvent.IsExpired ());
//  Simulator::Cancel (m_sendApduEvent);
  m_cotpClient->Send (socket, packet, CN, from);
}

void 
MmsAdaptClient::sendConfirmedRequestApdu (Ptr<Socket> socket, Ptr<Packet> packet,  Address from)
{
//  NS_ASSERT (m_sendApduEvent.IsExpired ());
//  Simulator::Cancel (m_sendApduEvent);
  m_cotpClient->Send (socket, packet, DTD, from);
}

void
MmsAdaptClient::SetMmsClient (Ptr<MmsClient> mmsClient)
{
  m_mmsClient = mmsClient;
}
	
Ptr<MmsClient>
MmsAdaptClient::GetMmsClient ()
{
  return m_mmsClient;
}

void 
MmsAdaptClient::SetCotpClient (Ptr<CotpClient> cotpClient)
{
  m_cotpClient = cotpClient;
}
	
Ptr<CotpClient>
MmsAdaptClient::GetCotpClient ()
{
  return m_cotpClient;
}

void 
MmsAdaptClient::SetStateCf (int state)
{
//  NS_ASSERT (m_changeStateEvent.IsExpired ());
//  Simulator::Cancel (m_changeStateEvent);

  if (state == CF_IDLE)
    {
      m_stateCf = state;
      NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C (" << Ipv4Address::ConvertFrom (m_cotpClient->GetLocalAddress ())
                                                   << ") --> IDLE");
    } 
  else if (state == CF_ASSOCIATION_PENDING)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C (" << Ipv4Address::ConvertFrom (m_cotpClient->GetLocalAddress ())
                                                     << ") --> ASSOCIATION_PENDING");
      }
  else if (state == CF_ASSOCIATED)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C (" << Ipv4Address::ConvertFrom (m_cotpClient->GetLocalAddress ())
                                                     << ") --> ASSOCIATED");
      } 
  else if (state == CF_ASSOCIATION_RELEASE_PENDING)    
      {
        m_stateCf = state;
        NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CAL (" << Ipv4Address::ConvertFrom (m_cotpClient->GetLocalAddress ())
                                                     << ") --> ASSOCIATION_RELEASE_PENDING");
      } 
  else
    {
      NS_LOG_INFO ("Error: No change of state");
    }
}

int 
MmsAdaptClient::GetStateCf ()
{
  return m_stateCf;
}

void MmsAdaptClient::SetNode (int node)
{
	m_node = node;
	ClientFile1 << "mms_client" << "_"  << m_node << "_events.csv";
}
	
int 
MmsAdaptClient::GetTypeService ()
{
  return m_typeService;
}

void 
MmsAdaptClient::SetTypeService (int typeService)
{
  m_typeService = typeService;
}
	
int 
MmsAdaptClient::GetTypeGet ()
{
  return m_typeGet;
}

void 
MmsAdaptClient::SetTypeGet (int typeGet)
{
  m_typeGet = typeGet;
}
 
void 
MmsAdaptClient::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
MmsAdaptClient::GetTcpport ()
{
  return m_tcpPort;
}

void
MmsAdaptClient::SetChangeStateEvent (EventId changeStateEvent)
{
  m_changeStateEvent = changeStateEvent;
}

} // namespace ns3
