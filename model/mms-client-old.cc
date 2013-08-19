#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "mms-header.h"
#include "mms-adapt-client.h"
#include "mms-server.h"
#include "mms-client.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/random-variable.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmsClient");
NS_OBJECT_ENSURE_REGISTERED (MmsClient);

TypeId 
MmsClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsClient")
    .SetParent<Application> ()
    .AddConstructor<MmsClient> ()
    ;
  return tid;
}

MmsClient::MmsClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_SrcRef = 0;
  m_tcpPort = 1082;
  m_nextTimeRequest = Seconds (0.0);
  //m_typeRequesting = 0;
  m_reqData = 0; 
  m_sizeReqData = 0;
  m_startRequestEvent = EventId ();
  m_nextRequestEvent = EventId (); 
  m_releaseAAEvent = EventId ();
  m_nSap = 0;
  m_totalNSap = 0; 
  m_enableNewRQ = 0;   
  m_nReq = 1;
  m_count = 0;
  m_count1 = 0;
  m_enableRpt = false;
  m_nSm = 1;
  // For debugging purposes
  // NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CAP created!");
}

MmsClient::~MmsClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
MmsClient::Recv (Ptr<Socket> socket, int typeSpdu, Ptr<Packet> packet, Ptr<MmsServer> mmsServer, Address from)
{
	if (typeSpdu == AC) // The Server accept the connection, MMS Initiate Response received
	{
		InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C " << Ipv4Address::ConvertFrom (m_localAddress) << " INITIATE-RESPONSE "
						<< "MMS-S " << address.GetIpv4());
		ofstream out ((ClientFile1.str ()).c_str (), ios::app);
		out << Simulator::Now ().GetSeconds () << " MMS-C " << Ipv4Address::ConvertFrom (m_localAddress) << " INITIATE-RESPONSE "
				<< "MMS-S " << address.GetIpv4() << endl;

		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
				<< " has established AA with MMS-S (" << mmsServer->GetSrcRef () << ":"
				<< address.GetIpv4() << ")");

		// Save the AA successfully established: (SrcRef, Ptr<MmsServer> mmsServer)
		SaveActiveAa (mmsServer, socket);
		m_count ++;
		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C has associated with total " << m_count << " SMs ");
		if (m_count == m_totalNSap)
		{
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C has associated with " << m_totalNSap << " SMs ");
			m_count = 0;
		}
		if ((m_typeRequesting == 0) || (m_typeRequesting == 1)) //READ Request
		{
			int poll;
			if (m_nSm > 1) poll = 9*m_nSm; else poll = 9;
			for (int i = 0; i < 20; i++)
				for (int j = 0; j < poll; j++)
				{
					Ptr<Packet> packet = Create<Packet>(); // dummy packet
					Simulator::Schedule (MicroSeconds (i*1000000+j*10), &MmsAdaptClient::ConfirmedRequest, m_mmsAdaptClient, socket, READ , from, packet, m_nReq);
				}

//			NS_LOG_INFO (Simulator::Now().GetSeconds() << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
//					<< " has sent CONFIRMED-REQUEST to MMS-S ("
//					<< Ipv4Address::ConvertFrom (mmsServer->GetLocalAddress ()) << ")");
		}
		if (m_typeRequesting == 2) //Enable Reporting Request
		{
			Ptr<Packet> packet = Create<Packet>(); // dummy packet
			Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::ConfirmedRequest, m_mmsAdaptClient, socket, WRITE , from, packet, m_nReq);
		}
	}

	if (typeSpdu == DTD)
	{
		//NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CAL (" << Ipv4Address::ConvertFrom (m_localAddress) << ") --> Get.cnf(NORMAL, Data)");

		MmsPresentationHeader mmsPresentationHeader;
		packet->RemoveHeader(mmsPresentationHeader);

		MmsTypeHeader mmsTypeHeader;
		packet->RemoveHeader(mmsTypeHeader);

		if (mmsTypeHeader.GetTypeMms() == CONFIRMED_RESPONSE)
		{
			packet->RemoveAtStart(packet->GetSize());
			InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C " << Ipv4Address::ConvertFrom (m_localAddress) << " CONFIRMED-RESPONSE"
				<< " MMS-S "
				<< address.GetIpv4());
			ofstream out ((ClientFile1.str ()).c_str (), ios::app);
			out << Simulator::Now ().GetSeconds () << " MMS-C " << Ipv4Address::ConvertFrom (m_localAddress) << " CONFIRMED-RESPONSE"
					<< " MMS-S "
					<< address.GetIpv4() << endl;

			if (m_typeRequesting == 2)
					{
						m_count1++;
						NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C has enabled reporting on " << m_count1 << " SMs ");
						if ((m_count1 == m_totalNSap) && (m_enableRpt == false))
						{
							NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C Enabled reporting on " << m_totalNSap << " SMs ");
							m_itSap = m_containerSap.Begin();
							m_nSap = 0;
							while (m_itSap != m_containerSap.End ())
							{
								Ptr<Application> app = m_containerSap.Get (m_nSap);
								m_currentMmsServer = app->GetObject<MmsServer> ();  // Retrieve the first Saps pointer stored in AppContainer
								Simulator::Schedule (Seconds (0.0), &MmsServer::SendReport, m_currentMmsServer, GetNextTimeRequest ());
								m_itSap ++;  // Increase the value of "it" by one
								m_nSap++;
							}
							m_enableRpt = true;
						}
						//Simulator::Schedule (Seconds (0.0), &MmsServer::SendReport, mmsServer);
					}
		}



		// Set a timer that permits to request new data to the SMs (SAPs)
		if (m_typeRequesting == 0)
		{
//			m_nextRequestEvent = Simulator::Schedule (GetNextTimeRequest (), &MmsClient::NewRequest, this, socket, mmsServer);
		}
		if (m_typeRequesting == 1)
		{
			if (m_nSap == m_totalNSap)
						{
							NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s CAP ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
									<< " has finished the requesting process " << m_nSap << " SM(s)!");
							m_nReq ++;
							// Initialize "it" parameter at the first entry in the Map that contains the SAPs that successfully established an AA
							m_it = m_activeAa.begin ();
							m_it_socket = m_activeSocket.begin();
							m_nSap = 0;
							m_enableNewRQ = 1;

							// Set a delay to request new data to the SMs
							m_nextRequestEvent = Simulator::Schedule (GetNextTimeRequest (), &MmsClient::NewRequest, this, socket, mmsServer);

						}
						else
						{
							if (m_enableNewRQ == 1)
							{
								// Next Server to request (new request)
								m_nextRequestEvent = Simulator::Schedule (Seconds (0.0), &MmsClient::NewRequest, this, socket, mmsServer);
							}
							else
							{
								// Next Server to request (first request)
								m_startRequestEvent = Simulator::Schedule (Seconds (0.0), &MmsClient::StartRequest, this);
							}
						}
		}

	}

}

void 
MmsClient::StartRequest ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (m_startRequestEvent.IsExpired ());
	Simulator::Cancel (m_startRequestEvent);
	NS_LOG_INFO("Current mode " << m_mode);
	NS_LOG_INFO("Current requesting type " << m_typeRequesting);
	if ((m_typeRequesting == 0) || (m_typeRequesting == 2))
	{
		m_itSap = m_containerSap.Begin();
		while (m_itSap != m_containerSap.End ())
		{
			Ptr<Application> app = m_containerSap.Get (m_nSap);
			m_currentMmsServer = app->GetObject<MmsServer> ();  // Retrieve the first Saps pointer stored in AppContainer
			m_itSap ++;  // Increase the value of "it" by one
			m_nSap++;
			/*
			 * Send Initiate Request to the remote MMS Server
			 */
			if (m_mode == 0)
			{
				m_currentMmsServer->Trigger(this);
			}
			else
			{
				Ptr<Packet> packet = NULL; // dummy packet
				Ptr<Socket> socket = NULL;
				Simulator::Schedule(Seconds (0), &MmsAdaptClient::MmsAdapt, m_mmsAdaptClient, socket, INITIATE, REQUEST, m_currentMmsServer->GetLocalAddress(), packet);
			}
//			m_mmsAdaptClient->MmsAdapt(socket, INITIATE, REQUEST, m_currentMmsServer, packet);
//			Time t;
//			t = Simulator::Now() + Seconds(0.001 * m_nSap);
		}
	}
	if (m_typeRequesting == 1)
	{
		if (m_itSap == m_containerSap.Begin ())
		{
			Ptr<Application> app = m_containerSap.Get (m_nSap);
			m_currentMmsServer = app->GetObject<MmsServer> ();  // Retrieve the first Saps pointer stored in AppContainer
			m_nSap ++;   // Increase the value of "m_nSap" by one
			m_itSap ++;  // Increase the value of "it" by one
			/*
			 * Send Initiate Request to the remote MMS Server
			 */
			Ptr<Packet> packet = NULL; // dummy packet
			m_mmsAdaptClient->MmsAdapt(NULL, INITIATE, REQUEST, m_currentMmsServer->GetLocalAddress(), packet);

			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
					<< " has sent INITIATE-REQUEST to MMS-S ("
					<< m_currentMmsServer->GetSrcRef () << ":"
					<< Ipv4Address::ConvertFrom (m_currentMmsServer->GetLocalAddress ()) << ")");
		}
		else
		{
			if (m_itSap != m_containerSap.End())
			{
				Ptr<Application> app = m_containerSap.Get (m_nSap);
				m_currentMmsServer = app->GetObject<MmsServer> ();  // Retrieve the first Saps pointer stored in AppContainer
				m_nSap ++;   // Increase the value of "m_nSap" by one
				m_itSap ++;  // Increase the value of "it" by one
				/*
				 * Send Initiate Request to the remote MMS Server
				 */
				Ptr<Packet> packet = NULL; // dummy packet
				m_mmsAdaptClient->MmsAdapt(NULL, INITIATE, REQUEST, m_currentMmsServer->GetLocalAddress(), packet);

				NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
						<< " has sent INITIATE-REQUEST to MMS-S ("
						<< m_currentMmsServer->GetSrcRef () << ":"
						<< Ipv4Address::ConvertFrom (m_currentMmsServer->GetLocalAddress ()) << ")");
			}
			else
			{
				m_nSap = 0;
			}
		}
	}
}

void
MmsClient::NewRequest (Ptr<Socket> socket, Ptr<MmsServer> mmsServer)
{
	// Only when Data Concentrator Application is present at the node
	if (!m_recvData.IsNull ())
	{
		NS_LOG_FUNCTION_NOARGS ();
		Simulator::Cancel (m_nextRequestEvent);
	}
	else
	{
//		NS_LOG_FUNCTION_NOARGS ();
//		NS_ASSERT (m_nextRequestEvent.IsExpired ());
//		Simulator::Cancel (m_nextRequestEvent);
	}


	if (m_typeRequesting == 0)
	{
		Ptr<Packet> packet = Create<Packet>(); // dummy packet
		Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::ConfirmedRequest, m_mmsAdaptClient, socket, READ , mmsServer->GetLocalAddress(), packet, m_nReq);
//		NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
//				<< " has requested new data to the MMS-S ("
//				<< mmsServer->GetSrcRef () << ":"
//				<< Ipv4Address::ConvertFrom (mmsServer->GetLocalAddress ()) << ")");
	}
	if (m_typeRequesting == 1)
	{
		//  Request new data only to the SAP, which the CAP has established a successfully AA
		if (m_it_socket != m_activeSocket.end())
		{
			m_currentMmsServer = m_it->second;
			Ptr<Socket> socket = m_it_socket->second;
			Ptr<Packet> packet = Create<Packet>(); // dummy packet
			NS_LOG_INFO(m_nSap);
			Simulator::Schedule (Seconds (0.0), &MmsAdaptClient::ConfirmedRequest, m_mmsAdaptClient, socket, READ , m_currentMmsServer->GetLocalAddress(), packet, m_nReq);
			m_it ++;
			m_it_socket ++;
			m_nSap ++;
			NS_LOG_INFO (Simulator::Now ().GetSeconds () << "s MMS-C ("<< m_SrcRef << ":" << Ipv4Address::ConvertFrom (m_localAddress) <<")"
					<< " has requested new data to the MMS-S ("
					<< m_currentMmsServer->GetSrcRef () << ":"
					<< Ipv4Address::ConvertFrom (m_currentMmsServer->GetLocalAddress ()) << ")");
		}
		else
		{
			m_nSap = 0;
			m_enableNewRQ = 0;
		}
	}
}

void 
MmsClient::RequestRelease ()
{

}

void 
MmsClient::SaveActiveAa (Ptr<MmsServer> mmsServer, Ptr<Socket> socket)
{
  // uint16_t dstSrcRef = mmsServer->GetSrcRef ();
  m_activeAa[m_nSap] = mmsServer;
  m_activeSocket[m_nSap] = socket;
}
	
void 
MmsClient::RemoveActiveAa (Ptr<MmsServer> mmsServer)
{
  // Find the SrcRef of the current SAP
  std::map<uint16_t, Ptr<MmsServer> >::iterator it = m_activeAa.find(m_nSap);
  std::map<uint16_t, Ptr<Socket> >::iterator it_socket = m_activeSocket.find(m_nSap);

  if (m_activeAa.end () != it)
    {
       m_activeAa.erase (it);	// Exists the connection, so erase it
    }
  else 
    {
      NS_LOG_INFO ("Error: Doesn't exist the AA requested to release!");
      return;			
    }

  if (m_activeSocket.end () != it_socket)
    {
       m_activeSocket.erase (it_socket);	// Exists the connection, so erase it
    }
  else
    {
      NS_LOG_INFO ("Error: Doesn't exist the AA requested to release!");
      return;
    }
}

void 
MmsClient::SetRecvCallback (Callback<void, uint32_t> recvData)
{
  m_recvData = recvData;
}

void 
MmsClient::SetMmsAdaptClient (Ptr<MmsAdaptClient> mmsAdaptClient)
{
  m_mmsAdaptClient = mmsAdaptClient;
}

Ptr<MmsAdaptClient>
MmsClient::GetMmsAdaptClient ()
{
  return m_mmsAdaptClient;
}

void 
MmsClient::SetSrcRef (uint16_t SrcRef)
{
  m_SrcRef = SrcRef;
}

uint16_t 
MmsClient::GetSrcRef ()
{
  return m_SrcRef;
}

void 
MmsClient::SetTcpport (uint16_t tcpPort)
{
  m_tcpPort = tcpPort ;
}

uint16_t 
MmsClient::GetTcpport ()
{
  return m_tcpPort;
}

void 
MmsClient::SetLocalAddress (Address ip)
{
  m_localAddress = ip;
}

Address
MmsClient::GetLocalAddress ()
{
  return m_localAddress;
}

void MmsClient::StartInitiate(Ptr<MmsServer> mmsServer)
{
	Ptr<Packet> packet = NULL; // dummy packet
	Ptr<Socket> socket = NULL;
	Simulator::Schedule(Seconds (0), &MmsAdaptClient::MmsAdapt, m_mmsAdaptClient, socket, INITIATE, REQUEST, mmsServer->GetLocalAddress(), packet);
}

void 
MmsClient::SetApplicationContainerSap (ApplicationContainer containerSap)
{
  m_containerSap = containerSap;
}

void 
MmsClient::SetCurrentMmsServer (Ptr<MmsServer> currentMmsServer)
{
  m_currentMmsServer = currentMmsServer;
}

Ptr<MmsServer>
MmsClient::GetCurrentMmsServer ()
{
  return m_currentMmsServer;
}

void 
MmsClient::SetTypeRequesting (int typeRequesting)
{
  m_typeRequesting = typeRequesting;
}

bool
MmsClient::GetTypeRequesting ()
{
  return m_typeRequesting;
}

void
MmsClient::SetSm (uint8_t nSm)
{
	m_nSm = nSm;
}

void MmsClient::SetMode (int mode)
{
	m_mode = mode;
}

int MmsClient::GetMode ()
{
	return m_mode;
}

void 
MmsClient::SetNextTimeRequest (Time nextTimeRequest)
{
  m_nextTimeRequest = nextTimeRequest;
}

Time 
MmsClient::GetNextTimeRequest ()
{
  return m_nextTimeRequest;
}

Ptr<Node>
MmsClient::GetNode () const
{
  Ptr<Node> node = Application::GetNode ();
  return node;
}

uint32_t 
MmsClient::GetSizeReqData ()
{
  return m_sizeReqData;
}

void
MmsClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
MmsClient::StartApplication (void)
{ 
  // Set the iterator at the begining of the container
  m_itSap = m_containerSap.Begin ();   
  // Retreive the total number of Server that this Client can request
  m_totalNSap = m_containerSap.GetN ();
  // Event: Create the StartRequest Event
  m_startRequestEvent = Simulator::Schedule (Seconds (0.0), &MmsClient::StartRequest, this);
  ClientFile1 << "mms_client" << "_"  << this->GetNode()->GetId() << "_events.csv";
  ofstream out ((ClientFile1.str ()).c_str (), ios::app);
  out << "Time Client Client_Add Event Server Server_Add" << endl;
  this->GetMmsAdaptClient()->SetNode(this->GetNode()->GetId());
}

void 
MmsClient::StopApplication (void)
{
   // Cancel Events
   Simulator::Cancel (m_nextRequestEvent);
   Simulator::Cancel (m_startRequestEvent); 
   // Initialize "it" parameter at the first entry in the Map that contains the SAPs that successfully established an AA
   m_it = m_activeAa.begin(); 	
   m_nSap = 0;
   // Event: Release AA established with remote SAPs
   m_releaseAAEvent = Simulator::Schedule (Seconds (0.0), &MmsClient::RequestRelease, this);
}

} // namespace ns3

