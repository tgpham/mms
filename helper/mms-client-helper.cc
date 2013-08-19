#include "ns3/mms-adapt-client.h"
#include "ns3/application-container.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "mms-client-helper.h"
#include "ns3/mms-client.h"
#include "ns3/cotp-client.h"

NS_LOG_COMPONENT_DEFINE ("MmsClientHelper");

namespace ns3 {

MmsClientHelper::MmsClientHelper ()
{
}

MmsClientHelper::MmsClientHelper (ApplicationContainer MmsServerContainer, Ipv4InterfaceContainer interface, Time interval, int type, int mode, uint8_t nSm)
{
  m_mmsServerContainer = MmsServerContainer;
  m_interface = interface;
  m_interval = interval;
  m_index = 0;
  m_type = type;
  m_mode = mode;
  m_nSm = nSm;
}

void
MmsClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
MmsClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      // Retreive the pointer of the i-node storaged in the NodeContainer
      Ptr<Node> node = *i;  
      // Add the MmsClientStack to the Node (i.e. CotpClient & MmsAdaptClient)
      AddMmsClientStack (node);

      // Create a MmsClientObject
      //Ptr<MmsClient> mmsClient =  m_factory.Create<MmsClient> ();
      Ptr<MmsClient> mmsClient = CreateObject<MmsClient> ();
      // Retrieve the pointer of the MmsAdaptClient that has previously aggregated to the node
      Ptr<MmsAdaptClient> mmsAdaptClient = node->GetObject<MmsAdaptClient> ();
      // Retrieve the pointer of the CotpClient that has previously aggregated to the node
      Ptr<CotpClient> cotpClient = node->GetObject<CotpClient> ();
      // Add the MmsClient created to the Node
      node->AddApplication (mmsClient);
      // Set the pointer to the MmsAdaptClient object attached at the node
      mmsClient->SetMmsAdaptClient (mmsAdaptClient);
      // Set the SrcRef
      cotpClient->SetSrcRefClient (mmsClient);
      // Set the Tcp Port listening by the CAL
      mmsClient->SetTcpport (46000);
      mmsClient->SetSm(m_nSm);
      mmsClient->SetTypeRequesting(m_type);
      mmsClient->SetMode(m_mode);
      // Set the Ip address assigned to the node 
      mmsClient->SetLocalAddress (m_interface.GetAddress (m_index));
      // Retreive the pointer of ApplicationContainer (MmsServers)
      mmsClient->SetApplicationContainerSap (m_mmsServerContainer);
      // Add the MmsClient created to the ApplicationContainer
      apps.Add (mmsClient);

      // Set interval for requesting data to remote SAPs
      mmsClient->SetNextTimeRequest (m_interval);
      // Connect MmsAdaptClient and mmsClient to each other
      mmsAdaptClient->SetMmsClient (mmsClient);
      mmsClient->SetMmsAdaptClient (mmsAdaptClient);
     
      m_index++;  
    }
  return apps;
}

void 
MmsClientHelper::AddMmsClientStack (Ptr<Node> node)
{
  // Create a CotpClient
  Ptr<CotpClient> cotpClient = CreateObject<CotpClient> ();
  // Create a MmsAdaptClient Object
  Ptr<MmsAdaptClient> mmsAdaptClient = CreateObject<MmsAdaptClient> ();
  // Connect CotpClient and MmsAdaptClient to each other
  cotpClient->SetMmsAdaptClient (mmsAdaptClient);
  mmsAdaptClient->SetCotpClient (cotpClient);
  // Aggregate the CotpClient to the node and set Ip Address and Tcp Port number
  node->AggregateObject (cotpClient);
  cotpClient->SetTcpport (46000);
  cotpClient->SetLocalAddress (m_interface.GetAddress(m_index));
  // Aggregate the MmsAdaptClient to the node and set its state to CF_IDLE and Tcp Port number
  node->AggregateObject (mmsAdaptClient);
  mmsAdaptClient->SetStateCf (1);
  mmsAdaptClient->SetTcpport (46000);
}

} // namespace ns3
