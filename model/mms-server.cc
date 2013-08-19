#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "mms-header.h"
#include "mms-adapt-server.h"
#include "mms-client.h"
#include "mms-server.h"
#include "ns3/lte-module.h"
#include "ns3/lte-ue-rrc.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmsServer");
NS_OBJECT_ENSURE_REGISTERED (MmsServer);

TypeId 
MmsServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsServer")
    .SetParent<Application> ()
    .AddConstructor<MmsServer> ()
    ;
  return tid;
}

MmsServer::MmsServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_srcRef = 0;
  m_tcpPort = 0;
 
  // For debugging purposes
  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s SAP created!");
}

MmsServer::~MmsServer ()
{

}

void 
MmsServer::SendReport(Time interval)
{
	//Create MMS Unconfirmed Service packet
	Ptr<Packet> rptPkt = Create<Packet>();
	MmsUnconfirmedHeader mmsUnconfirmedHeader;
	rptPkt->AddHeader(mmsUnconfirmedHeader);

	MmsTypeHeader mmsTypeHeader;
	mmsTypeHeader.SetType(UNCONFIRMED_SRV, -1, -1);
	rptPkt->AddHeader(mmsTypeHeader);

	Simulator::Schedule (Seconds (0.0), &MmsAdaptServer::UnconfirmedService, m_mmsAdaptServer, m_clientSocket, rptPkt);
	NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-S (" << Ipv4Address::ConvertFrom (m_localAddress) << ") <-- sent REPORTING");
	Simulator::Schedule (interval, &MmsServer::SendReport, this, interval);
}

void
MmsServer::Recv (Ptr<Socket> socket, int typeSpdu, int typeService, Ptr<Packet> packet)
{
	if (typeSpdu == CN)
	{
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-S (" << Ipv4Address::ConvertFrom (m_localAddress) << ") <-- received INITIATE-REQUEST");
		m_clientSocket = socket;
		//Create MMS Initiate Response packet
		Ptr<Packet> initRespPkt = Create<Packet>();
		MmsInitiateResponseHeader mmsInitiateResponseHeader;
		initRespPkt->AddHeader(mmsInitiateResponseHeader);
		MmsAcseAareHeader mmsAcseAareHeader;
		initRespPkt->AddHeader(mmsAcseAareHeader);
		MmsPresentationInitResponseHeader mmsPresentationInitResponseHeader;
		initRespPkt->AddHeader(mmsPresentationInitResponseHeader);
		// Event: Invoke the MMS Adapt service
		Simulator::Schedule (Seconds (0.01), &MmsAdaptServer::MmsAdapt, m_mmsAdaptServer, socket, INITIATE, REQUEST, initRespPkt);
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-S (" << Ipv4Address::ConvertFrom (m_localAddress) << ") --> sent INITIATE-RESPONSE");
	}

	if (typeSpdu == DTD)
	{
		MmsPresentationHeader mmsPresentationHeader;
		packet->RemoveHeader(mmsPresentationHeader);

		MmsTypeHeader mmsTypeHeader;
		packet->RemoveHeader(mmsTypeHeader);
		int nReq = mmsTypeHeader.GetInvokeID();

		if (mmsTypeHeader.GetTypeMms() == CONFIRMED_REQUEST)
		{
			if (mmsTypeHeader.GetTypeConfirmed() - 0xa0 == WRITE)
			{
				Ptr<Packet> res_packet = Create<Packet>(); // dummy packet

				MmsConfirmedWriteResponseHeader mmsConfirmedWriteResponseHeader;
				res_packet->AddHeader(mmsConfirmedWriteResponseHeader);

				mmsTypeHeader.SetType(CONFIRMED_RESPONSE, WRITE, nReq);
				res_packet->AddHeader(mmsTypeHeader);
				//double t = (8*res_packet->GetSize ())/(500000) + 2.235e-3;
				Simulator::Schedule (Seconds (0.01), &MmsAdaptServer::ConfirmedResponse, m_mmsAdaptServer, socket, WRITE , res_packet);
			}

			if (mmsTypeHeader.GetTypeConfirmed() - 0xa0 == READ)
			{
				uint32_t resNum =  packet->GetSize() / 78 + 1;

				for (uint32_t i = 0; i < resNum; i++)
				{
					Ptr<Packet> res_packet = Create<Packet>(); // dummy packet

					MmsConfirmedReadResponseHeader mmsConfirmedReadResponseHeader;
					res_packet->AddHeader(mmsConfirmedReadResponseHeader);

					mmsTypeHeader.SetType(CONFIRMED_RESPONSE, READ, nReq);
					res_packet->AddHeader(mmsTypeHeader);
					//double t = (8*res_packet->GetSize ())/(500000) + 2.235e-3;
					Simulator::Schedule (MicroSeconds (i*10), &MmsAdaptServer::ConfirmedResponse, m_mmsAdaptServer, socket, READ , res_packet);
				}
			}

		}
	}

}

Ptr<Node>
MmsServer::GetNode () const
{
  Ptr<Node> node = Application::GetNode ();
  return node;
}

void 
MmsServer::SetMmsAdaptServer (Ptr<MmsAdaptServer> cosemAlServer)
{
  m_mmsAdaptServer = cosemAlServer;
}

Ptr<MmsAdaptServer>
MmsServer::GetMmsAdaptServer ()
{
  return m_mmsAdaptServer;
}

void
MmsServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
MmsServer::SetSrcRef (uint16_t srcRef)
{
  m_srcRef = srcRef;
}

uint16_t 
MmsServer::GetSrcRef ()
{
  return m_srcRef;
}

void 
MmsServer::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
MmsServer::GetTcpport ()
{
  return m_tcpPort;
}

void 
MmsServer::SetLocalAddress (Address ip)
{
  m_localAddress = ip;
}

void
MmsServer::Trigger(Ptr<MmsClient> from)
{
	if (this->GetNode()->GetDevice(0)->GetObject<LteUeNetDevice> ()->GetRrc()->GetState() != LteUeRrc::CONNECTED_NORMALLY)
	{
		//NS_LOG_INFO(Simulator::Now().GetSeconds() << "s Server at " << Ipv4Address::ConvertFrom(m_localAddress) << " not connected to LTE Network");
		Simulator::Schedule(Seconds(0.001), &MmsServer::Trigger, this, from);
	}
	else
	{
		NS_LOG_INFO(Simulator::Now().GetSeconds() << "s Server at " << Ipv4Address::ConvertFrom(m_localAddress) << " connected to LTE Network");
		Simulator::Schedule(Seconds(0.0), &MmsClient::StartInitiate, from, this);
	}

}

Address
MmsServer::GetLocalAddress ()
{
  return m_localAddress;
}

void 
MmsServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_mmsAdaptServer->Init ();
}

void 
MmsServer::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  // do nothing 
}

} // namespace ns3


