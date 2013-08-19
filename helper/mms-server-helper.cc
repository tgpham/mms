#include "ns3/application-container.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/mms-adapt-server.h"
#include "mms-server-helper.h"
#include "ns3/mms-server.h"
#include "ns3/cotp-server.h"

NS_LOG_COMPONENT_DEFINE ("MmsServerHelper");

namespace ns3 {

MmsServerHelper::MmsServerHelper ()
{
}

MmsServerHelper::MmsServerHelper (Ipv4InterfaceContainer interface)
{
  m_interface = interface;
  m_index = 0;
}

void
MmsServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
MmsServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      // Retreive the pointer of the i-node storaged in the NodeContainer
      Ptr<Node> node = *i;  
      // Add the MmsServerStack to the Node (i.e. CotpServer & MmsAdaptServer)
      AddMmsServerStack (node);

      // Create a MmsServerObject
      Ptr<MmsServer> mmsServer = CreateObject<MmsServer> ();
      // Retrieve the pointer of the MmsAdaptServer that has previously aggregated to the node
      Ptr<MmsAdaptServer> mmsAdaptServer = node->GetObject<MmsAdaptServer> ();
      // Retrieve the pointer of the CotpServer that has previously aggregated to the node
      Ptr<CotpServer> cotpServer = node->GetObject<CotpServer> ();
      // Add the MmsServer created to the Node
      node->AddApplication (mmsServer);
      // Set the pointer to the MmsAdaptServer object attached at the node
      mmsServer->SetMmsAdaptServer (mmsAdaptServer);
      // Set the SrcRef
      cotpServer->SetSrcRefServer (mmsServer);
      // Set the Tcp Port listening by the CAL
      mmsServer->SetTcpport (102);
      // Set the Ip address assigned to the node 
      mmsServer->SetLocalAddress (m_interface.GetAddress (m_index));
      // Add the MmsServer created to the ApplicationContainer
      apps.Add (mmsServer);
      // Connect MmsAdaptServer and mmsServer to each other
      mmsAdaptServer->SetMmsServer (mmsServer);
      mmsServer->SetMmsAdaptServer (mmsAdaptServer);
     
      m_index++;  
    }
  return apps;
}

void 
MmsServerHelper::AddMmsServerStack (Ptr<Node> node)
{
  // Create a CotpServer
  Ptr<CotpServer> cotpServer = CreateObject<CotpServer> ();
  // Create a MmsAdaptServer Object
  Ptr<MmsAdaptServer> mmsAdaptServer = CreateObject<MmsAdaptServer> ();
  // Connect CotpServer and MmsAdaptServer to each other
  cotpServer->SetMmsAdaptServer (mmsAdaptServer);
  mmsAdaptServer->SetCotpServer (cotpServer);
  // Aggregate the CotpServer to the node and set Ip Address and Tcp Port number
  node->AggregateObject (cotpServer);
  cotpServer->SetTcpport (102);
  cotpServer->SetLocalAddress (m_interface.GetAddress(m_index));
  // Aggregate the MmsAdaptServer to the node and set its state to CF_IDLE and Tcp Port number
  node->AggregateObject (mmsAdaptServer);
  mmsAdaptServer->SetStateCf (1);
  mmsAdaptServer->SetTcpport (102);
}

} // namespace ns3
