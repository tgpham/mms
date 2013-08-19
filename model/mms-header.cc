#include "ns3/log.h"
#include "mms-header.h"
 
namespace ns3 {

/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED WRITE RESPONSE APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsTypeHeader);

TypeId
MmsTypeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsTypeHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsTypeHeader> ()
    ;
  return tid;
}

MmsTypeHeader::MmsTypeHeader ()
{
	 m_mmsServiceType = 0;
	 m_MmsConfirmedRequestParam1 = 0;
	 m_MmsConfirmedRequestParam2 = 0;
	 m_invokeId = 0;
	 m_mmsConfirmedType = 0;
}

MmsTypeHeader::~MmsTypeHeader ()
{

}

TypeId
MmsTypeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsTypeHeader::GetSerializedSize (void) const
{
	uint8_t nSmDc = 3;
	if ((m_mmsServiceType == CONFIRMED_RESPONSE) && (6*nSmDc+11 > 0x7f)) return 8;
	else
		return 6;
}

void
MmsTypeHeader::Serialize (Buffer::Iterator start) const
{
	uint8_t nSmDc = 3;
	start.WriteU8(m_mmsServiceType);
	if (m_mmsServiceType == CONFIRMED_REQUEST)
	start.WriteHtonU16(m_MmsConfirmedRequestParam1);
	else
	{
		if (6*nSmDc+11 < 0x7f)
			start.WriteHtonU16(m_MmsConfirmedRequestParam1);
		else
		{
			start.WriteU8(0x82);
			start.WriteHtonU16((6*nSmDc+11));
			start.WriteU8(0x02);
		}
	}
	start.WriteU8(m_MmsConfirmedRequestParam2);
	start.WriteU8(m_invokeId);
	start.WriteU8(m_mmsConfirmedType);
}

uint32_t
MmsTypeHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsServiceType = i.ReadU8();
	m_MmsConfirmedRequestParam1 = i.ReadNtohU16();
	m_MmsConfirmedRequestParam2 = i.ReadU8();
	m_invokeId = i.ReadU8();
	m_mmsConfirmedType = i.ReadU8();
	return GetSerializedSize ();
}

void
MmsTypeHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

uint8_t
MmsTypeHeader::GetTypeMms() const
{
	return m_mmsServiceType;
}

uint8_t
MmsTypeHeader::GetTypeConfirmed() const
{
	return m_mmsConfirmedType;
}

void
MmsTypeHeader::SetType(int typeMms, int typeConfirmed, int m_nReq)
{
	m_mmsServiceType = typeMms;
	if (typeMms == CONFIRMED_REQUEST)
	{
		if (typeConfirmed == WRITE)
		{
			m_MmsConfirmedRequestParam1 = 0x4102;
			m_MmsConfirmedRequestParam2 = 0x01;
			m_invokeId = m_nReq;
			m_mmsConfirmedType = typeConfirmed + 0xa0;
		}
		if (typeConfirmed == READ)
		{
			m_MmsConfirmedRequestParam1 = 0x3202;
			m_MmsConfirmedRequestParam2 = 0x01;
			m_invokeId = m_nReq;
			m_mmsConfirmedType = typeConfirmed + 0xa0;
		}
	}

	if (typeMms == CONFIRMED_RESPONSE)
	{
		if (typeConfirmed == WRITE)
		{
			m_MmsConfirmedRequestParam1 = 0x0702;
			m_MmsConfirmedRequestParam2 = 0x01;
			m_invokeId = m_nReq;
			m_mmsConfirmedType = typeConfirmed + 0xa0;
		}
		if (typeConfirmed == READ)
		{
			uint8_t nSmDc = 3;
			m_MmsConfirmedRequestParam1 = (6*nSmDc+11)*0x100 + 0x02;
			m_MmsConfirmedRequestParam2 = 0x01;
			m_invokeId = m_nReq;
			m_mmsConfirmedType = typeConfirmed + 0xa0;
		}
	}

	if (typeMms == UNCONFIRMED_SRV)
	{
		m_MmsConfirmedRequestParam1 = 0x5fa0;
		m_MmsConfirmedRequestParam2 = 0x5d;
		m_invokeId = 0xa1;
		m_mmsConfirmedType = 0x05;
	}
}

uint8_t MmsTypeHeader::GetInvokeID() const
{
	return m_invokeId;
}

/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED READ RESPONSE APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsUnconfirmedHeader);

TypeId
MmsUnconfirmedHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsUnconfirmedHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsUnconfirmedHeader> ()
    ;
  return tid;
}

MmsUnconfirmedHeader::MmsUnconfirmedHeader ()
{
	 m_Data1 = 0x8003525054a0548a;
	 m_Data2 = 0x0754656d7056616c;
	 m_Data3 = 0x84030638008c0602;
	 m_Data4 = 0xcbc14e26048a1a44;
	 m_Data5 = 0x4b36314c44657669;
	 m_Data6 = 0x6365312f4c4c4e30;
	 m_Data7 = 0x244453544d505661;
	 m_Data8 = 0x6c840205ff850100;
	 m_Data9 = 0x840303000091084c;
	 m_Data10 = 0x73c2a10000009f84;
	 m_Data11 = 0x0202088402020884;
	 m_Data12 = 0x0202;
	 m_Data13 = 0x08;
}

MmsUnconfirmedHeader::~MmsUnconfirmedHeader ()
{

}

TypeId
MmsUnconfirmedHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsUnconfirmedHeader::GetSerializedSize (void) const
{
  return 91;
}

void
MmsUnconfirmedHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU64(m_Data1);
	start.WriteHtonU64(m_Data2);
	start.WriteHtonU64(m_Data3);
	start.WriteHtonU64(m_Data4);
	start.WriteHtonU64(m_Data5);
	start.WriteHtonU64(m_Data6);
	start.WriteHtonU64(m_Data7);
	start.WriteHtonU64(m_Data8);
	start.WriteHtonU64(m_Data9);
	start.WriteHtonU64(m_Data10);
	start.WriteHtonU64(m_Data11);
	start.WriteHtonU16(m_Data12);
	start.WriteU8(m_Data13);
}

uint32_t
MmsUnconfirmedHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_Data1 = i.ReadNtohU64() ;
	m_Data2 = i.ReadNtohU64() ;
	m_Data3 = i.ReadNtohU64() ;
	m_Data4 = i.ReadNtohU64() ;
	m_Data5 = i.ReadNtohU64() ;
	m_Data6 = i.ReadNtohU64() ;
	m_Data7 = i.ReadNtohU64() ;
	m_Data8 = i.ReadNtohU64() ;
	m_Data9 = i.ReadNtohU64() ;
	m_Data10 = i.ReadNtohU64() ;
	m_Data11 = i.ReadNtohU64() ;
	m_Data12 = i.ReadNtohU16() ;
	m_Data13 = i.ReadU8() ;
	return GetSerializedSize ();
}

void
MmsUnconfirmedHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED READ RESPONSE APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsConfirmedReadResponseHeader);

TypeId
MmsConfirmedReadResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsConfirmedReadResponseHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsConfirmedReadResponseHeader> ()
    ;
  return tid;
}

MmsConfirmedReadResponseHeader::MmsConfirmedReadResponseHeader ()
{
	 uint8_t nSmDc = 3;
	 m_Length = 6*nSmDc; //
	 m_mmsReadResponseParam4 = 0xa100 + (6*nSmDc);
	 m_mmsReadResponseData = 0;
}

MmsConfirmedReadResponseHeader::~MmsConfirmedReadResponseHeader ()
{

}

TypeId
MmsConfirmedReadResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsConfirmedReadResponseHeader::GetSerializedSize (void) const
{
	uint8_t nSmDc = 3;
  return 6*nSmDc+7;
}

void
MmsConfirmedReadResponseHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(0x82);
	start.WriteHtonU16(m_Length+4);
//	start.WriteHtonU16(m_mmsReadResponseParam0);
//	start.WriteHtonU32(m_mmsReadResponseParam1);
//	start.WriteHtonU16(m_mmsReadResponseParam2);
	start.WriteHtonU16(0xa182);
	start.WriteHtonU16(m_Length);
	uint8_t nSmDc = 3;
	for (int i=0; i<nSmDc; i++)
	{
		start.WriteHtonU16(0x8504);
		start.WriteHtonU32(m_mmsReadResponseData);
	}
}

uint32_t
MmsConfirmedReadResponseHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_Length = i.ReadU8() ;
	m_Length = i.ReadNtohU16() ;
//	m_mmsReadResponseParam0 = i.ReadNtohU16() ;
//	m_mmsReadResponseParam1 = i.ReadNtohU32() ;
//	m_mmsReadResponseParam2 = i.ReadNtohU16() ;
	m_mmsReadResponseParam4 = i.ReadNtohU16() ;
	m_mmsReadResponseParam4 = i.ReadNtohU16() ;
	uint8_t nSmDc = 3;
	for (int j=0; j<nSmDc; j++)
	{
		uint16_t temp = i.ReadNtohU16();
		temp++;
		m_mmsReadResponseData = i.ReadNtohU32() ;
	}
	return GetSerializedSize ();
}

void
MmsConfirmedReadResponseHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}


/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED READ REQUEST APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsConfirmedReadRequestHeader);

TypeId
MmsConfirmedReadRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsConfirmedReadRequestHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsConfirmedReadRequestHeader> ()
    ;
  return tid;
}

MmsConfirmedReadRequestHeader::MmsConfirmedReadRequestHeader ()
{
	m_mmsConfirmedTypeParam = 0x2d800100a128a026;
	m_MmsConfirmedRequestParam2 = 0x3024a022a1201a0c;
	m_MmsDeviceName = 0x534d5f5554446576;
	m_MmsDeviceID = 0x30303031;
	m_itemID = 0x1a10;
	m_MmsReportName1 = 0x4d4d584e31245761;
	m_MmsReportName2 = 0x7474246d61672020;
}

MmsConfirmedReadRequestHeader::~MmsConfirmedReadRequestHeader ()
{

}

TypeId
MmsConfirmedReadRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsConfirmedReadRequestHeader::GetSerializedSize (void) const
{
  return 46;
}

void
MmsConfirmedReadRequestHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU64(m_mmsConfirmedTypeParam);
	start.WriteHtonU64(m_MmsConfirmedRequestParam2);
	start.WriteHtonU64(m_MmsDeviceName);
	start.WriteHtonU32(m_MmsDeviceID);
	start.WriteHtonU16(m_itemID);
	start.WriteHtonU64(m_MmsReportName1);
	start.WriteHtonU64(m_MmsReportName2);
}

uint32_t
MmsConfirmedReadRequestHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsConfirmedTypeParam = i.ReadNtohU64();
	m_MmsConfirmedRequestParam2 = i.ReadNtohU64() ;
	m_MmsDeviceName = i.ReadNtohU64() ;
	m_MmsDeviceID = i.ReadNtohU32() ;
	m_itemID = i.ReadNtohU16() ;
	m_MmsReportName1 = i.ReadNtohU64() ;
	m_MmsReportName2 = i.ReadNtohU64() ;
	return GetSerializedSize();
}

void
MmsConfirmedReadRequestHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}


/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED WRITE RESPONSE APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsConfirmedWriteResponseHeader);

TypeId
MmsConfirmedWriteResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsConfirmedWriteResponseHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsConfirmedWriteResponseHeader> ()
    ;
  return tid;
}

MmsConfirmedWriteResponseHeader::MmsConfirmedWriteResponseHeader ()
{
	m_mmsWriteResponseData1 = 0x02;
	m_mmsWriteResponseData2 = 0x8100;
}

MmsConfirmedWriteResponseHeader::~MmsConfirmedWriteResponseHeader ()
{

}

TypeId
MmsConfirmedWriteResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsConfirmedWriteResponseHeader::GetSerializedSize (void) const
{
  return 3;
}

void
MmsConfirmedWriteResponseHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_mmsWriteResponseData1);
	start.WriteHtonU16(m_mmsWriteResponseData2);
}

uint32_t
MmsConfirmedWriteResponseHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsWriteResponseData1 = i.ReadU8() ;
	m_mmsWriteResponseData2 = i.ReadNtohU16() ;
	return GetSerializedSize ();
}

void
MmsConfirmedWriteResponseHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

/*-----------------------------------------------------------------------------
 *  MMS CONFIRMED WRITE REQUEST APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsConfirmedWriteRequestHeader);

TypeId
MmsConfirmedWriteRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsConfirmedWriteRequestHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsConfirmedWriteRequestHeader> ()
    ;
  return tid;
}

MmsConfirmedWriteRequestHeader::MmsConfirmedWriteRequestHeader ()
{
	m_mmsConfirmedTypeLength = 0x3c;
	m_mmsConfirmedTypeParam = 0xa035;
	m_MmsConfirmedRequestParam2 = 0x3033a031a12f1a0c;
	m_MmsDeviceName = 0x444b36314c446576;
	m_MmsDeviceID = 0x69636531;
	m_itemID = 0x1a;
	m_MmsReportName1 = 0x1f4c4c4e30245250;
	m_MmsReportName2 = 0x24554e4255464645;
	m_MmsReportName3 = 0x5245445f52434230;
	m_MmsReportName4 = 0x3124527074456e61;
	m_listOfData = 0xa0;
	m_data = 0x038301ff;
}

MmsConfirmedWriteRequestHeader::~MmsConfirmedWriteRequestHeader ()
{

}

TypeId
MmsConfirmedWriteRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsConfirmedWriteRequestHeader::GetSerializedSize (void) const
{
  return 61;
}

void
MmsConfirmedWriteRequestHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_mmsConfirmedTypeLength);
	start.WriteHtonU16(m_mmsConfirmedTypeParam);
	start.WriteHtonU64(m_MmsConfirmedRequestParam2);
	start.WriteHtonU64(m_MmsDeviceName);
	start.WriteHtonU32(m_MmsDeviceID);
	start.WriteU8(m_itemID);
	start.WriteHtonU64(m_MmsReportName1);
	start.WriteHtonU64(m_MmsReportName2);
	start.WriteHtonU64(m_MmsReportName3);
	start.WriteHtonU64(m_MmsReportName4);
	start.WriteU8(m_listOfData);
	start.WriteHtonU32(m_data);
}

uint32_t
MmsConfirmedWriteRequestHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsConfirmedTypeLength = i.ReadU8();
	m_mmsConfirmedTypeParam = i.ReadNtohU16();
	m_MmsConfirmedRequestParam2 = i.ReadNtohU64() ;
	m_MmsDeviceName = i.ReadNtohU64() ;
	m_MmsDeviceID = i.ReadNtohU32() ;
	m_itemID = i.ReadU8() ;
	m_MmsReportName1 = i.ReadNtohU64() ;
	m_MmsReportName2 = i.ReadNtohU64() ;
	m_MmsReportName3 = i.ReadNtohU64() ;
	m_MmsReportName4 = i.ReadNtohU64() ;
	m_listOfData = i.ReadU8() ;
	m_data = i.ReadNtohU32() ;
	return GetSerializedSize ();
}

void
MmsConfirmedWriteRequestHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

/*-----------------------------------------------------------------------------
 *  MMS INITIATE RESPONSE APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsInitiateResponseHeader);

TypeId
MmsInitiateResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsInitiateResponseHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsInitiateResponseHeader> ()
    ;
  return tid;
}

MmsInitiateResponseHeader::MmsInitiateResponseHeader ()
{
	 m_mmsServiceType = 0xa9;
	 m_mmsLengh = 0x25;
	 m_MmsInitResponseParam1 = 0x80022ee081010182;
	 m_MmsInitResponseParam2 = 0x0101830105a41680;
	 m_MmsInitResponseParam3 = 0x0101810305f10082;
	 m_MmsInitResponseParam4 = 0x0c03ee0800000400;
	 m_MmsInitResponseParam5 = 0x00000000;
	 m_MmsInitResponseParam6 = 0x18;
}

MmsInitiateResponseHeader::~MmsInitiateResponseHeader ()
{

}

TypeId
MmsInitiateResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsInitiateResponseHeader::GetSerializedSize (void) const
{
  return 39;
}

void
MmsInitiateResponseHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_mmsServiceType);
	start.WriteU8(m_mmsLengh);
	start.WriteHtonU64(m_MmsInitResponseParam1);
	start.WriteHtonU64(m_MmsInitResponseParam2);
	start.WriteHtonU64(m_MmsInitResponseParam3);
	start.WriteHtonU64(m_MmsInitResponseParam4);
	start.WriteHtonU32(m_MmsInitResponseParam5);
	start.WriteU8(m_MmsInitResponseParam6);
}

uint32_t
MmsInitiateResponseHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsServiceType = i.ReadU8();
	m_mmsLengh = i.ReadU8();
	m_MmsInitResponseParam1 = i.ReadNtohU64();
	m_MmsInitResponseParam2 = i.ReadNtohU64();
	m_MmsInitResponseParam3 = i.ReadNtohU64();
	m_MmsInitResponseParam4 = i.ReadNtohU64();
	m_MmsInitResponseParam5 = i.ReadNtohU32();
	m_MmsInitResponseParam6 = i.ReadU8();
	return GetSerializedSize ();
}

void
MmsInitiateResponseHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS INITIATE REQUEST APDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsInitiateRequestHeader);

TypeId
MmsInitiateRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsInitiateRequestHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsInitiateRequestHeader> ()
    ;
  return tid;
}

MmsInitiateRequestHeader::MmsInitiateRequestHeader ()
{
	 m_mmsServiceType = 0xa8;
	 m_mmsLengh = 0x26;
	 m_MmsInitRequestParam1 = 0x800300fde8810101;
	 m_MmsInitRequestParam2 = 0x820101830105a416;
	 m_MmsInitRequestParam3 = 0x800101810305f100;
	 m_MmsInitRequestParam4 = 0x820c03ee08000004;
	 m_MmsInitRequestParam5 = 0x00000000;
	 m_MmsInitRequestParam6 = 0x0018;
}

MmsInitiateRequestHeader::~MmsInitiateRequestHeader ()
{

}

TypeId
MmsInitiateRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsInitiateRequestHeader::GetSerializedSize (void) const
{
  return 40;
}

void
MmsInitiateRequestHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_mmsServiceType);
	start.WriteU8(m_mmsLengh);
	start.WriteHtonU64(m_MmsInitRequestParam1);
	start.WriteHtonU64(m_MmsInitRequestParam2);
	start.WriteHtonU64(m_MmsInitRequestParam3);
	start.WriteHtonU64(m_MmsInitRequestParam4);
	start.WriteHtonU32(m_MmsInitRequestParam5);
	start.WriteHtonU16(m_MmsInitRequestParam6);
}

uint32_t
MmsInitiateRequestHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_mmsServiceType = i.ReadU8();
	m_mmsLengh = i.ReadU8();
	m_MmsInitRequestParam1 = i.ReadNtohU64();
	m_MmsInitRequestParam2 = i.ReadNtohU64();
	m_MmsInitRequestParam3 = i.ReadNtohU64();
	m_MmsInitRequestParam4 = i.ReadNtohU64();
	m_MmsInitRequestParam5 = i.ReadNtohU32();
	m_MmsInitRequestParam6 = i.ReadNtohU16();
	return GetSerializedSize ();
}

void
MmsInitiateRequestHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS ACSE AARE TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsAcseAareHeader);

TypeId
MmsAcseAareHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsAcseAareHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsAcseAareHeader> ()
    ;
  return tid;
}

MmsAcseAareHeader::MmsAcseAareHeader ()
{
	//default to AARE
	 m_idAcse = 0x61;
	 m_length = 0x57;
	 m_PresentationInitParam1 = 0x80020780a1070605;
	 m_PresentationInitParam2 = 0x28ca220203a20302;
	 m_PresentationInitParam3 = 0x0100a305a1030201;
	 m_PresentationInitParam4 = 0x00a4070605290187;
	 m_PresentationInitParam5 = 0x6701a50302010cbe;
	 m_PresentationInitParam6 = 0x2e282c020103a027;
}

MmsAcseAareHeader::~MmsAcseAareHeader ()
{

}

TypeId
MmsAcseAareHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsAcseAareHeader::GetSerializedSize (void) const
{
  return 50;
}

void
MmsAcseAareHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_idAcse);
	start.WriteU8(m_length);
	start.WriteHtonU64(m_PresentationInitParam1);
	start.WriteHtonU64(m_PresentationInitParam2);
	start.WriteHtonU64(m_PresentationInitParam3);
	start.WriteHtonU64(m_PresentationInitParam4);
	start.WriteHtonU64(m_PresentationInitParam5);
	start.WriteHtonU64(m_PresentationInitParam6);
}

uint32_t
MmsAcseAareHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_idAcse=i.ReadU8();
	m_length=i.ReadU8();
	m_PresentationInitParam1=i.ReadNtohU64();
	m_PresentationInitParam2=i.ReadNtohU64();
	m_PresentationInitParam3=i.ReadNtohU64();
	m_PresentationInitParam4=i.ReadNtohU64();
	m_PresentationInitParam5=i.ReadNtohU64();
	m_PresentationInitParam6=i.ReadNtohU64();
	return GetSerializedSize ();
}

void
MmsAcseAareHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS ACSE AARQ TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsAcseAarqHeader);

TypeId
MmsAcseAarqHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsAcseAarqHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsAcseAarqHeader> ()
    ;
  return tid;
}

MmsAcseAarqHeader::MmsAcseAarqHeader ()
{
	//default to AARQ
	 m_idAcse = 0x60;
	 m_length = 0x59;
	 m_PresentationInitParam1 = 0xa107060528ca2202;
	 m_PresentationInitParam2 = 0x03a2070605290187;
	 m_PresentationInitParam3 = 0x6701a30302010ca6;
	 m_PresentationInitParam4 = 0x06060429018767a7;
	 m_PresentationInitParam5 = 0x0302010cbe332831;
	 m_PresentationInitParam6 = 0x06020151020103a0;
	 m_MMS_length = 0x28;
}

MmsAcseAarqHeader::~MmsAcseAarqHeader ()
{

}

TypeId
MmsAcseAarqHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsAcseAarqHeader::GetSerializedSize (void) const
{
  return 51;
}

void
MmsAcseAarqHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_idAcse);
	start.WriteU8(m_length);
	start.WriteHtonU64(m_PresentationInitParam1);
	start.WriteHtonU64(m_PresentationInitParam2);
	start.WriteHtonU64(m_PresentationInitParam3);
	start.WriteHtonU64(m_PresentationInitParam4);
	start.WriteHtonU64(m_PresentationInitParam5);
	start.WriteHtonU64(m_PresentationInitParam6);
	start.WriteU8(m_MMS_length);
}

uint32_t
MmsAcseAarqHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_idAcse=i.ReadU8();
	m_length=i.ReadU8();
	m_PresentationInitParam1=i.ReadNtohU64();
	m_PresentationInitParam2=i.ReadNtohU64();
	m_PresentationInitParam3=i.ReadNtohU64();
	m_PresentationInitParam4=i.ReadNtohU64();
	m_PresentationInitParam5=i.ReadNtohU64();
	m_PresentationInitParam6=i.ReadNtohU64();
	m_MMS_length=i.ReadU8();
	return GetSerializedSize ();
}

void
MmsAcseAarqHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS INIT REQUEST PRESENTATION PDU HEADER
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsPresentationInitRequestHeader);

TypeId
MmsPresentationInitRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsPresentationInitRequestHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsPresentationInitRequestHeader> ()
    ;
  return tid;
}

MmsPresentationInitRequestHeader::MmsPresentationInitRequestHeader ()
{
	m_idPpdu = 0x3181;
	m_length = 0x9d;
	m_modeSelector = 0xa0;
	m_modeSelectorParam = 0x03800101;
	m_normalModeSelector = 0xa281;
	m_lengthMode = 0x95;
	m_modeParam1 = 0x8104000000018204;
	m_modeParam2 = 0x00000001;
	m_contextList = 0xa4;
	m_contextListLength = 0x23;
	m_contextListParam1 = 0x300f020101060452;
	m_contextListParam2 = 0x0100013004060251;
	m_contextListParam3 = 0x0130100201030605;
	m_contextListParam4 = 0x28ca220201300406;
	m_contextListParam5 = 0x02510161;
	m_userDataLength = 0x62;
	m_pdvListIndication = 0x30;
	m_pdvListLength = 0x60;
	m_presentationDataValue = 0x020101a0;
	m_presentationDataValueLength = 0x5b;
}

MmsPresentationInitRequestHeader::~MmsPresentationInitRequestHeader ()
{

}

TypeId
MmsPresentationInitRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsPresentationInitRequestHeader::GetSerializedSize (void) const
{
  return 69;
}

void
MmsPresentationInitRequestHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU16(m_idPpdu);
	start.WriteU8(m_length);
	start.WriteU8(m_modeSelector);
	start.WriteHtonU32(m_modeSelectorParam);
	start.WriteHtonU16(m_normalModeSelector);
	start.WriteU8(m_lengthMode);
	start.WriteHtonU64(m_modeParam1);
	start.WriteHtonU32(m_modeParam2);
	start.WriteU8(m_contextList);
	start.WriteU8(m_contextListLength);
	start.WriteHtonU64(m_contextListParam1);
	start.WriteHtonU64(m_contextListParam2);
	start.WriteHtonU64(m_contextListParam3);
	start.WriteHtonU64(m_contextListParam4);
	start.WriteHtonU32(m_contextListParam5);
	start.WriteU8(m_userDataLength);
	start.WriteU8(m_pdvListIndication);
	start.WriteU8(m_pdvListLength);
	start.WriteHtonU32(m_presentationDataValue);
	start.WriteU8(m_presentationDataValueLength);
}

uint32_t
MmsPresentationInitRequestHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_idPpdu=i.ReadNtohU16();
	m_length=i.ReadU8();
	m_modeSelector=	i.ReadU8();
	m_modeSelectorParam=i.ReadNtohU32();
	m_normalModeSelector=i.ReadNtohU16();
	m_lengthMode=i.ReadU8();
	m_modeParam1=i.ReadNtohU64();
	m_modeParam2=i.ReadNtohU32();
	m_contextList=i.ReadU8();
	m_contextListLength=i.ReadU8();
	m_contextListParam1=i.ReadNtohU64();
	m_contextListParam2=i.ReadNtohU64();
	m_contextListParam3=i.ReadNtohU64();
	m_contextListParam4=i.ReadNtohU64();
	m_contextListParam5=i.ReadNtohU32();
	m_userDataLength=i.ReadU8();
	m_pdvListIndication=i.ReadU8();
	m_pdvListLength=i.ReadU8();
	m_presentationDataValue=i.ReadNtohU32();
	m_presentationDataValueLength=i.ReadU8();
	return GetSerializedSize ();
}

void
MmsPresentationInitRequestHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS INIT REQUEST PRESENTATION PDU HEADER
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsPresentationHeader);

TypeId
MmsPresentationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsPresentationHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsPresentationHeader> ()
    ;
  return tid;
}

MmsPresentationHeader::MmsPresentationHeader ()
{
	m_idPpdu = 0x61;  // PPDU identifier (2B) = 0x3181
	m_length1 = 0;  // SPDU Length (1B) = 0x88
	m_param1 = 0x30;
	m_length2 = 0;
	m_param2 = 0x020103a0;
	m_length3 = 0;
}

MmsPresentationHeader::~MmsPresentationHeader ()
{

}

TypeId
MmsPresentationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsPresentationHeader::GetSerializedSize (void) const
{
  return 15;
}

void
MmsPresentationHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8(m_idPpdu);  // PPDU identifier (2B) = 0x3181
	start.WriteU8(0x82);
	start.WriteHtonU16(m_length1);  // SPDU Length (1B) = 0x88
	start.WriteU8(m_param1);
	start.WriteU8(0x82);
	start.WriteHtonU16(m_length2);
	start.WriteHtonU32(m_param2);
	start.WriteU8(0x82);
	start.WriteHtonU16(m_length3);
}

uint32_t
MmsPresentationHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_idPpdu = i.ReadU8();  // PPDU identifier (2B) = 0x3181
	m_length1 = i.ReadU8();  // SPDU Length (1B) = 0x88
	m_length1 = i.ReadNtohU16();
	m_param1 = i.ReadU8();
	m_length2 = i.ReadU8();
	m_length2 = i.ReadNtohU16();
	m_param2 = i.ReadNtohU32();
	m_length3 = i.ReadU8();
	m_length3 = i.ReadNtohU16();
	return GetSerializedSize ();
}

void
MmsPresentationHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

void MmsPresentationHeader::SetLength(uint16_t length)
{
	m_length3 = length;
	m_length2 = length + 7;
	m_length1 = length + 11;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS INIT Response PRESENTATION PDU HEADER
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsPresentationInitResponseHeader);

TypeId
MmsPresentationInitResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsPresentationInitResponseHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsPresentationInitResponseHeader> ()
    ;
  return tid;
}

MmsPresentationInitResponseHeader::MmsPresentationInitResponseHeader ()
{
	 m_idPpdu = 0x3181;  // PPDU identifier (2B) = 0x3181
	 m_length = 0x88;  // SPDU Length (1B) = 0x88
	 m_modeSelector = 0xa0; // =
	 m_modeSelectorParam = 0x03800101;
	 m_modeParam1 = 0xa281808304000000;
	 m_modeParam2 = 0x01a5123007800100;
	 m_modeParam3 = 0x8102510130078001;
	 m_modeParam4 = 0x0081025101880206;
	 m_modeParam5 = 0x006160305e020101;
	 m_presentationDataValue = 0xa0;
	 m_presentationDataValueLength = 0x59;
}

MmsPresentationInitResponseHeader::~MmsPresentationInitResponseHeader ()
{

}

TypeId
MmsPresentationInitResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsPresentationInitResponseHeader::GetSerializedSize (void) const
{
  return 50;
}

void
MmsPresentationInitResponseHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU16(m_idPpdu);  // PPDU identifier (2B) = 0x3181
	start.WriteU8(m_length);  // SPDU Length (1B) = 0x88
	start.WriteU8(m_modeSelector); // =
	start.WriteHtonU32(m_modeSelectorParam);
	start.WriteHtonU64(m_modeParam1);
	start.WriteHtonU64(m_modeParam2);
	start.WriteHtonU64(m_modeParam3);
	start.WriteHtonU64(m_modeParam4);
	start.WriteHtonU64(m_modeParam5);
	start.WriteU8(m_presentationDataValue);
	start.WriteU8(m_presentationDataValueLength);
}

uint32_t
MmsPresentationInitResponseHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_idPpdu = i.ReadNtohU16();  // PPDU identifier (2B) = 0x3181
	m_length = i.ReadU8();  // SPDU Length (1B) = 0x88
	m_modeSelector = i.ReadU8(); // =
	m_modeSelectorParam = i.ReadNtohU32();
	m_modeParam1 = i.ReadNtohU64();
	m_modeParam2 =i.ReadNtohU64();
	m_modeParam3 = i.ReadNtohU64();
	m_modeParam4 = i.ReadNtohU64();
	m_modeParam5 = i.ReadNtohU64();
	m_presentationDataValue = i.ReadU8();
	m_presentationDataValueLength = i.ReadU8();
	return GetSerializedSize();
}

void
MmsPresentationInitResponseHeader::Print (std::ostream &os) const
{
//	os << "SPDU Type " << m_idSpdu
//			<< "Length " << m_length;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS SESSION TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsSessionHeader);

TypeId
MmsSessionHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsSessionHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsSessionHeader> ()
    ;
  return tid;
}

MmsSessionHeader::MmsSessionHeader ()
{
	//default to CN Session Header
	m_connectAcceptItem = 0x0506130100160102;
	m_sessionRequirement = 0x14020002;
	m_callingSessionSelector = 0x33020001;
	m_calledSessionSelector = 0x34020001;
	m_sessionUserDataIndication = 0xc1;
	m_sessionUserDataLength = 0xa0;
}

MmsSessionHeader::~MmsSessionHeader ()
{

}

TypeId
MmsSessionHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsSessionHeader::GetSerializedSize (void) const
{
  return 22;
}

void
MmsSessionHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU64(m_connectAcceptItem);
	start.WriteHtonU32(m_sessionRequirement);
	start.WriteHtonU32(m_callingSessionSelector);
	start.WriteHtonU32(m_calledSessionSelector);
	start.WriteU8(m_sessionUserDataIndication);
	start.WriteU8(m_sessionUserDataLength);
}

uint32_t
MmsSessionHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_connectAcceptItem = i.ReadNtohU64();
  m_sessionRequirement = i.ReadNtohU32();
  m_callingSessionSelector = i.ReadNtohU32();
  m_calledSessionSelector = i.ReadNtohU32();
  m_sessionUserDataIndication = i.ReadU8();
  m_sessionUserDataLength = i.ReadU8();
  return GetSerializedSize ();
}

void
MmsSessionHeader::Print (std::ostream &os) const
{
	os << "Connect Accept Item " << m_connectAcceptItem
	   << "Session Requirement " << m_sessionRequirement
	   << "Calling Session Selector " << m_callingSessionSelector
	   << "Called Session Selector " << m_calledSessionSelector
	   << "Session User Data Indication " << m_sessionUserDataIndication
	   << "Session User Data Length " << m_sessionUserDataLength;
}

void MmsSessionHeader::SetConnectAcceptItem (uint64_t connectAcceptItem)
{
	m_connectAcceptItem = connectAcceptItem;
}

uint64_t MmsSessionHeader::GetConnectAcceptItem (void) const
{
	return m_connectAcceptItem;
}

void MmsSessionHeader::SetSessionRequirement (uint32_t sessionRequirement)
{
	m_sessionRequirement = sessionRequirement;
}

uint32_t MmsSessionHeader::GetSessionRequirement (void) const
{
	return m_sessionRequirement;
}

void MmsSessionHeader::SetCallingSessionSelector (uint32_t callingSessionSelector)
{
	m_callingSessionSelector = callingSessionSelector;
}

uint32_t MmsSessionHeader::GetCallingSessionSelector (void) const
{
	return m_callingSessionSelector;
}

void MmsSessionHeader::SetCalledSessionSelector (uint32_t calledSessionSelector)
{
	m_calledSessionSelector = calledSessionSelector;
}

uint32_t MmsSessionHeader::GetCalledSessionSelector (void) const
{
	return m_calledSessionSelector;
}

void MmsSessionHeader::SetSessionUserDataIndication (uint8_t sessionUserDataIndication)
{
	m_sessionUserDataIndication = sessionUserDataIndication;
}

uint8_t MmsSessionHeader::GetSessionUserDataIndication (void) const
{
	return m_sessionUserDataIndication;
}

void MmsSessionHeader::SetSessionUserDataLength (uint8_t sessionUserDataLength)
{
	m_sessionUserDataLength = sessionUserDataLength;
}

uint8_t MmsSessionHeader::GetSessionUserDataLength (void) const
{
	return m_sessionUserDataLength;
}

///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS INIT RESPONSE SESSION TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsSessionInitResponseHeader);

TypeId
MmsSessionInitResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsSessionInitResponseHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsSessionInitResponseHeader> ()
    ;
  return tid;
}

MmsSessionInitResponseHeader::MmsSessionInitResponseHeader ()
{
	//default to CN Session Header
	m_connectAcceptItem = 0x0506130100160102;
	m_sessionRequirement = 0x14020002;
	m_sessionUserDataIndication = 0xc1;
	m_sessionUserDataLength = 0x8b;
}

MmsSessionInitResponseHeader::~MmsSessionInitResponseHeader ()
{

}

TypeId
MmsSessionInitResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsSessionInitResponseHeader::GetSerializedSize (void) const
{
  return 14;
}

void
MmsSessionInitResponseHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteHtonU64(m_connectAcceptItem);
	start.WriteHtonU32(m_sessionRequirement);
	start.WriteU8(m_sessionUserDataIndication);
	start.WriteU8(m_sessionUserDataLength);
}

uint32_t
MmsSessionInitResponseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_connectAcceptItem = i.ReadNtohU64();
  m_sessionRequirement = i.ReadNtohU32();
  m_sessionUserDataIndication = i.ReadU8();
  m_sessionUserDataLength = i.ReadU8();
  return GetSerializedSize ();
}

void
MmsSessionInitResponseHeader::Print (std::ostream &os) const
{
	os << "Connect Accept Item " << m_connectAcceptItem
	   << "Session Requirement " << m_sessionRequirement
	   << "Session User Data Indication " << m_sessionUserDataIndication
	   << "Session User Data Length " << m_sessionUserDataLength;
}

void MmsSessionInitResponseHeader::SetConnectAcceptItem (uint64_t connectAcceptItem)
{
	m_connectAcceptItem = connectAcceptItem;
}

uint64_t MmsSessionInitResponseHeader::GetConnectAcceptItem (void) const
{
	return m_connectAcceptItem;
}

void MmsSessionInitResponseHeader::SetSessionRequirement (uint32_t sessionRequirement)
{
	m_sessionRequirement = sessionRequirement;
}

uint32_t MmsSessionInitResponseHeader::GetSessionRequirement (void) const
{
	return m_sessionRequirement;
}

void MmsSessionInitResponseHeader::SetSessionUserDataIndication (uint8_t sessionUserDataIndication)
{
	m_sessionUserDataIndication = sessionUserDataIndication;
}

uint8_t MmsSessionInitResponseHeader::GetSessionUserDataIndication (void) const
{
	return m_sessionUserDataIndication;
}

void MmsSessionInitResponseHeader::SetSessionUserDataLength (uint8_t sessionUserDataLength)
{
	m_sessionUserDataLength = sessionUserDataLength;
}

uint8_t MmsSessionInitResponseHeader::GetSessionUserDataLength (void) const
{
	return m_sessionUserDataLength;
}


///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  MMS SESSION TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (MmsSessionTypeHeader);

TypeId
MmsSessionTypeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmsSessionTypeHeader")
    .SetParent<Header> ()
    .AddConstructor<MmsSessionTypeHeader> ()
    ;
  return tid;
}

MmsSessionTypeHeader::MmsSessionTypeHeader ()
{
	//default to CN
	m_idSpdu = 13;  // CN (13)
	m_length = 0xb6; // place-holder for length value
}

MmsSessionTypeHeader::~MmsSessionTypeHeader ()
{

}

TypeId
MmsSessionTypeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmsSessionTypeHeader::GetSerializedSize (void) const
{
  return 2;
}

void
MmsSessionTypeHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_idSpdu);
  start.WriteU8 (m_length);
}

uint32_t
MmsSessionTypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_idSpdu = i.ReadU8 ();
  m_length = i.ReadU8 ();
  return GetSerializedSize ();
}

void
MmsSessionTypeHeader::Print (std::ostream &os) const
{
  os << "SPDU Type " << m_idSpdu
     << "Length " << m_length;
}

void MmsSessionTypeHeader::SetIdSpdu (uint8_t idSpdu)
{
	m_idSpdu = idSpdu;
}

uint8_t MmsSessionTypeHeader::GetIdSpdu (void) const
{
	return m_idSpdu;
}
void MmsSessionTypeHeader::SetLength (uint8_t length)
{
	m_length = length;
}

uint8_t MmsSessionTypeHeader::GetLength (void) const
{
	return m_length;
}


///////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  COTP PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (CotpHeader);

TypeId 
CotpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CotpHeader")
    .SetParent<Header> ()
    .AddConstructor<CotpHeader> ()
    ;
  return tid;
}

CotpHeader::CotpHeader ()
{
	m_DstRef = 0;
	m_SrcRef = 0;
	m_CotpParamField1 = 0;
	m_CotpParamField2 = 0;
	m_DstTsap = 0;
	m_CotpParamField3 = 0;
	m_SrcTsap = 0;
}

CotpHeader::~CotpHeader ()
{

}

TypeId 
CotpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t 
CotpHeader::GetSerializedSize (void) const
{
  return 16;
}
 
void 
CotpHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU16 (m_DstRef);
  start.WriteHtonU16 (m_SrcRef);
  start.WriteHtonU32 (m_CotpParamField1);
  start.WriteHtonU16 (m_CotpParamField2);
  start.WriteHtonU16 (m_DstTsap);
  start.WriteHtonU16 (m_CotpParamField3);
  start.WriteHtonU16 (m_SrcTsap);
}

uint32_t 
CotpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_DstRef = i.ReadNtohU16 ();
  m_SrcRef = i.ReadNtohU16 ();
  m_CotpParamField1 = i.ReadNtohU32 ();
  m_CotpParamField2 = i.ReadNtohU16 ();
  m_DstTsap = i.ReadNtohU16 ();
  m_CotpParamField3 = i.ReadNtohU16 ();
  m_SrcTsap = i.ReadNtohU16 ();
  return GetSerializedSize ();
}

void 
CotpHeader::Print (std::ostream &os) const
{
  os << "Dst Ref " << m_DstRef
     << "Src Ref " << m_SrcRef
     << "Param 1 " << m_CotpParamField1
     << "Param 2 " << m_CotpParamField2
     << "Dst TSAP " << m_DstTsap
  	 << "Param 3 " << m_CotpParamField3
  	 << "Src TSAP " << m_SrcTsap;
}

void CotpHeader::SetDstRef (uint16_t ref)
	{
		m_DstRef = ref;
	}

	uint16_t CotpHeader::GetDstRef (void) const
	{
		return m_DstRef;
	}

	void CotpHeader::SetSrcRef (uint16_t ref)
	{
		m_SrcRef = ref;
	}

	uint16_t CotpHeader::GetSrcRef (void) const
	{
		return m_SrcRef;
	}

	void CotpHeader::SetCotpParamField1 (uint32_t param)
	{
		m_CotpParamField1 = param;
	}

	uint32_t CotpHeader::GetCotpParamField1 (void) const
	{
		return m_CotpParamField1;
	}

	void CotpHeader::SetCotpParamField2 (uint16_t param)
	{
		m_CotpParamField2 = param;
	}

	uint16_t CotpHeader::GetCotpParamField2 (void) const
	{
		return m_CotpParamField2;
	}

	void CotpHeader::SetDstTsap (uint16_t dstTsap)
	{
		m_DstTsap = dstTsap;
	}

	uint16_t CotpHeader::GetDstTsap (void) const
	{
		return m_DstTsap;
	}

	void CotpHeader::SetCotpParamField3 (uint16_t param)
	{
		m_CotpParamField2 = param;
	}

	uint16_t CotpHeader::GetCotpParamField3 (void) const
	{
		return m_CotpParamField3;
	}

	void CotpHeader::SetSrcTsap (uint16_t srcTsap)
	{
		m_SrcTsap = srcTsap;
	}

	uint16_t CotpHeader::GetSrcTsap (void) const
	{
		return m_SrcTsap;
	}

/*-----------------------------------------------------------------------------
 *  COTP DT PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (CotpDtHeader);

TypeId
CotpDtHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CotpDtHeader")
    .SetParent<Header> ()
    .AddConstructor<CotpDtHeader> ()
    ;
  return tid;
}

CotpDtHeader::CotpDtHeader ()
{
	m_Param = 0x80;
}

CotpDtHeader::~CotpDtHeader ()
{

}

TypeId
CotpDtHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
CotpDtHeader::GetSerializedSize (void) const
{
  return 1;
}

void
CotpDtHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_Param);
}

uint32_t
CotpDtHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_Param = i.ReadU8 ();
  return GetSerializedSize();
}

void
CotpDtHeader::Print (std::ostream &os) const
{
  os << "Param " << m_Param;
}

//////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
 *  COTP TYPE PDU
 *-----------------------------------------------------------------------------
 */

NS_OBJECT_ENSURE_REGISTERED (CotpTypeHeader);

TypeId
CotpTypeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CotpTypeHeader")
    .SetParent<Header> ()
    .AddConstructor<CotpTypeHeader> ()
    ;
  return tid;
}

CotpTypeHeader::CotpTypeHeader ()
{
	m_TptkVersion = 0x03;  // Version of TPTK (3)
	m_TptkReserved = 0; // TPTK reserved (0)
	m_TptkLength = 0;   // Length of the higher protocol packet transported (2B)
	m_CotpLength = 0;
	m_CotpPduType = CRE;

}

CotpTypeHeader::~CotpTypeHeader ()
{

}

TypeId
CotpTypeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
CotpTypeHeader::GetSerializedSize (void) const
{
  return 6;
}

void
CotpTypeHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_TptkVersion);
  start.WriteU8 (m_TptkReserved);
  start.WriteHtonU16 (m_TptkLength);
  start.WriteU8 (m_CotpLength);
  start.WriteU8 (m_CotpPduType);
}

uint32_t
CotpTypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_TptkVersion = i.ReadU8 ();
  m_TptkReserved = i.ReadU8 ();
  m_TptkLength = i.ReadNtohU16 ();
  m_CotpLength = i.ReadU8 ();
  m_CotpPduType = i.ReadU8 ();
  return GetSerializedSize ();
}

void
CotpTypeHeader::Print (std::ostream &os) const
{
  os << "Version of TPTK " << m_TptkVersion
     << "Reserved TPTK " << m_TptkReserved
     << "Length TPTK " << m_TptkLength
     << "Length of COTP " << m_CotpLength
     << "COTP PDU" << m_CotpPduType;
}

void CotpTypeHeader::SetTptkVersion (uint8_t version)
{
	m_TptkVersion = version;
}

uint8_t CotpTypeHeader::GetTptkVersion (void) const
{
	return m_TptkVersion;
}
void CotpTypeHeader::SetTptkReserved (uint8_t reserved)
{
	m_TptkReserved = reserved;
}

uint8_t CotpTypeHeader::GetTptkReserved (void) const
{
	return m_TptkReserved;
}

void CotpTypeHeader::SetTptkLength (uint16_t length)
{
	m_TptkLength = length;
}

uint16_t CotpTypeHeader::GetTptkLength (void) const
{
	return m_TptkLength;
}

void CotpTypeHeader::SetCotpLength (uint8_t length)
{
	m_CotpLength = length;
}

uint8_t CotpTypeHeader::GetCotpLength (void) const
{
	return m_CotpLength;
}

void CotpTypeHeader::SetCotpPduType (uint8_t type)
{
	m_CotpPduType = type;
}

uint8_t CotpTypeHeader::GetCotpPduType (void) const
{
	return m_CotpPduType;
}


} // namespace ns3
