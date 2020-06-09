#include "xmlOper.h"

#include <afxdisp.h> 
#include <comutil.h>
/*������Ϊһ��XML�ĵ�����������*/


xmlOPer::xmlOPer()
{
	spDoc = NULL;
}

xmlOPer::~xmlOPer()
{
	CoUninitialize();
}

CComPtr<IXMLDOMElement> xmlOPer::openXmlDoc()
{
	////��Ϊһ������
	//CComPtr<IXMLDOMDocument> spDoc; //DOM
	//HRESULT  hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER/*CLSCTX_ALL*/, IID_IXMLDOMDocument, (LPVOID*)&spDoc);
	//if (hr != S_OK)
	//	return;
	//VARIANT_BOOL vb;
	//spDoc->load(CComVariant(OLESTR("./systemini.xml")), &vb); //����XML�ļ�
	//CComPtr<IXMLDOMElement> spRootEle;
	//spDoc->get_documentElement(&spRootEle); //���ڵ�
	//CComPtr<IXMLDOMNodeList> spNodeList;
	//spRootEle->get_childNodes(&spNodeList); //�ӽڵ��б�
	//long nLen;
	//spNodeList->get_length(&nLen); //�ӽڵ���
	//for (long i = 0; i != nLen; ++i) //�����ӽڵ�
	//{
	//	CComPtr<IXMLDOMNode> spNode;
	//	spNodeList->get_item(i, &spNode);
	//	ProcessNode(spNode); //�ڵ㴦����
	//}


	//��Ϊһ������
	CoInitialize(NULL);//�����Ҫ�ͷ�CoUninitialize();
					   //CoUninitialize();
	//CComPtr<IXMLDOMDocument> spDoc; //DOM
	HRESULT  hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER/*CLSCTX_ALL*/, IID_IXMLDOMDocument, (LPVOID*)&spDoc);
	if (hr != S_OK)
		return nullptr;
	//spDoc.CoCreateInstance(CLSID_DOMDocument);
	VARIANT_BOOL vb;
	//spDoc->load(CComVariant(OLESTR("./rx10.xml")), &vb); //����XML�ļ�
	//BSTR xml = _bstr_t("< ? xml version = \"1.0\" encoding = \"utf - 8\" ? ><root></root>");
	hr = spDoc->load(CComVariant(OLESTR("./systemini.xml")), &vb);



	CComPtr<IXMLDOMElement> spRootEle;
	spDoc->get_documentElement(&spRootEle); //���ڵ�


	return spRootEle;
}
void xmlOPer::ProcessNode(CComPtr<IXMLDOMNode>& spNode)
{
	CComBSTR bsNodeName;
	spNode->get_nodeName(&bsNodeName); //�ڵ���
	AfxMessageBox(COLE2CT(bsNodeName));
	CComVariant varVal;
	spNode->get_nodeValue(&varVal); //�ڵ�ֵ
	AfxMessageBox(COLE2CT(varVal.bstrVal));
	DOMNodeType eNodeType;
	spNode->get_nodeType(&eNodeType);
	if (eNodeType == NODE_ELEMENT) //ֻ��NODE_ELEMENT���Ͳ��ܰ��������Ժ��ӽڵ�
	{
		//�ݹ�����ڵ�����
		CComPtr<IXMLDOMNamedNodeMap> spNameNodeMap;
		spNode->get_attributes(&spNameNodeMap);
		long nLength;
		spNameNodeMap->get_length(&nLength);
		for (long i = 0; i != nLength; ++i)
		{
			CComPtr<IXMLDOMNode> spNodeAttrib; //ע������Ҳ��һ��IXMLDOMNode
			spNameNodeMap->get_item(i, &spNodeAttrib);
			ProcessNode(spNodeAttrib);
		}
		//�ݹ�����ӽڵ�
		CComPtr<IXMLDOMNodeList> spNodeList;
		spNode->get_childNodes(&spNodeList);
		spNodeList->get_length(&nLength);
		for (long i = 0; i != nLength; ++i)
		{
			CComPtr<IXMLDOMNode> spChildNode;
			spNodeList->get_item(i, &spChildNode);
			ProcessNode(spChildNode);
		}
	}
}

void xmlOPer::writeXmlDoc(CComPtr<IXMLDOMElement> spRootEle, CComPtr<IXMLDOMDocument> spDoc, CComPtr<IXMLDOMNode>& spNode)
{
	//��Ϊһ������
	/*< ? xml version = "1.0" encoding = "utf-8" ? >
	<root>
	<node1>text1< / node1>
	<node2>
	<childnode1 attrib1 = "value1" attrib2 = "value2" / >
	<childnode2 attrib1 = "value1" attrib2 = "value2">childtext1< / childnode2>
	< / node2>
	< / root>*/

	//д��XML
	//CComPtr<IXMLDOMNode> spNode;
	spRootEle->selectSingleNode(OLESTR("/root/node1"), &spNode);
	spNode->put_text(OLESTR("newText")); //д��text
	spRootEle->selectSingleNode(OLESTR("/root/node2/childnode1/@attrib1"), &spNode);
	spNode->put_nodeValue(CComVariant(OLESTR("newValue"))); //д��value
	CComPtr<IXMLDOMNode> spNewNode;
	spDoc->createNode(CComVariant(NODE_ELEMENT), OLESTR("childnode3"), OLESTR(""), &spNewNode); //�����½ڵ�
	spRootEle->selectSingleNode(OLESTR("/root/node2"), &spNode);
	spNode->appendChild(spNewNode, &spNewNode); //���½ڵ��Ϊnode2���ӽڵ�
	spNewNode->put_text(OLESTR("childtext2")); //д���½ڵ�text
	CComQIPtr<IXMLDOMElement> spEle = spNewNode; //ע������ʹ��CComQIPtr
	spEle->setAttribute(OLESTR("attrib1"), CComVariant(OLESTR("value1")));//���½ڵ��������
	spDoc->save(CComVariant(OLESTR("stocks.xml")));

}
void xmlOPer::selectNodes(CComPtr<IXMLDOMElement> spRootEle)
{
	CComPtr<IXMLDOMNodeList> spNodeList;
	CComPtr<IXMLDOMNode> spNode;
	spRootEle->selectNodes(OLESTR("/root/node2/*"), &spNodeList); //�õ�node2�µ������ӽڵ�
	spRootEle->selectSingleNode(OLESTR("/root/node2/childnode1/@attrib1"), &spNode); //�õ�childnode1��attrib1����
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



CComPtr<IXMLDOMElement> xmlOPer::openXmlDocFromMemory(CString xmlContent)
{
	//��Ϊһ������
	CoInitialize(NULL);//�����Ҫ�ͷ�CoUninitialize();
	//CoUninitialize();
	CComPtr<IXMLDOMDocument> spDoc; //DOM
	HRESULT  hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER/*CLSCTX_ALL*/, IID_IXMLDOMDocument, (LPVOID*)&spDoc);
	if (hr != S_OK)
		return nullptr;
	//spDoc.CoCreateInstance(CLSID_DOMDocument);
	VARIANT_BOOL vb;
	//spDoc->load(CComVariant(OLESTR("./rx10.xml")), &vb); //����XML�ļ�
	//BSTR xml = _bstr_t("< ? xml version = \"1.0\" encoding = \"utf - 8\" ? ><root></root>");
	spDoc->loadXML(_bstr_t(xmlContent), &vb);



	CComPtr<IXMLDOMElement> spRootEle;
	spDoc->get_documentElement(&spRootEle); //���ڵ�

	//CComPtr<IXMLDOMNodeList> spNodeList;
	//spRootEle->get_childNodes(&spNodeList); //�ӽڵ��б�
	//long nLen;
	//spNodeList->get_length(&nLen); //�ӽڵ���
	//for (long i = 0; i != nLen; ++i) //�����ӽڵ�
	//{
	//	CComPtr<IXMLDOMNode> spNode;
	//	spNodeList->get_item(i, &spNode);
	//	ProcessNode(spNode); //�ڵ㴦����
	//}
	return spRootEle;
}

CComPtr<IXMLDOMNodeList> xmlOPer::selectNodes(CComPtr<IXMLDOMElement> spRootEle, CString node)
{
	CComPtr<IXMLDOMNodeList> spNodeList;

	//spRootEle->selectNodes(OLESTR("/root/node2/*"), &spNodeList); //�õ�node2�µ������ӽڵ�
	spRootEle->selectNodes(_bstr_t(node), &spNodeList); //�õ�node2�µ������ӽڵ�
	return spNodeList;
}

CComPtr<IXMLDOMNode> xmlOPer::selectSingleNode(CComPtr<IXMLDOMElement> spRootEle, CString node)
{

	CComPtr<IXMLDOMNode> spNode;
	//spRootEle->selectSingleNode(OLESTR("/root/node2/childnode1/@attrib1"), &spNode); //�õ�childnode1��attrib1����
	spRootEle->selectSingleNode(_bstr_t(node), &spNode); //�õ�childnode1��attrib1����
	return spNode;
}



key_value xmlOPer::selectSingleNodeKeyValue(CComPtr<IXMLDOMNode> xmlOPer)
{
	key_value m_key_value = {  };
	CComBSTR bsNodeName;
	xmlOPer->get_nodeName(&bsNodeName); //�ڵ���
	m_key_value.xml_key = COLE2CT(bsNodeName);
	//AfxMessageBox(COLE2CT(bsNodeName));



	long nLength;						 //�ݹ�����ӽڵ�
	CComPtr<IXMLDOMNodeList> spNodeList;
	xmlOPer->get_childNodes(&spNodeList);
	spNodeList->get_length(&nLength);

	CComPtr<IXMLDOMNode> spChildNode;
	spNodeList->get_item(nLength - 1, &spChildNode);


	CComVariant varVal;
	spChildNode->get_nodeValue(&varVal); //�ڵ�ֵ
	//AfxMessageBox(COLE2CT(varVal.bstrVal));
	m_key_value.xml_value = COLE2CT(varVal.bstrVal);
	return m_key_value;
}


list<X_ScalarWebAPI_ServiceList> xmlOPer::ProcessNodeList(CComPtr<IXMLDOMNodeList>nodeList)
{
	//X_ScalarWebAPI_ServiceList cc;// = new X_ScalarWebAPI_ServiceList();
	list<X_ScalarWebAPI_ServiceList> ServiceList;
	//ServiceList.push_back(cc);
	int index = 0;
	long nLen;
	nodeList->get_length(&nLen); //�ӽڵ���
	for (long i = 0; i != nLen; ++i) //�����ӽڵ�
	{
		CComPtr<IXMLDOMNode> spNode;
		nodeList->get_item(i, &spNode);
		ProcessNodes(spNode, ServiceList, NULL, index); //�ڵ㴦����
	}
	list<X_ScalarWebAPI_ServiceList> m_ServiceList;
	X_ScalarWebAPI_ServiceList * m_X_ScalarWebAPI_ServiceList = nullptr;
	for (int index = 0; ServiceList.size() > 0; index++)
	{
		if (index % 2 == 0)
		{
			m_X_ScalarWebAPI_ServiceList = new X_ScalarWebAPI_ServiceList();
		}
		X_ScalarWebAPI_ServiceList m_tmp = ServiceList.front();

		if ((index == 0) || (index % 2 == 0))
		{
			m_X_ScalarWebAPI_ServiceList->X_ScalarWebAPI_AccessType = m_tmp.X_ScalarWebAPI_AccessType;
		}
		else
		{
			m_X_ScalarWebAPI_ServiceList->X_ScalarWebAPI_ActionList_URL = m_tmp.X_ScalarWebAPI_AccessType;
			m_ServiceList.push_back(*m_X_ScalarWebAPI_ServiceList);
		}
		ServiceList.pop_front();

	}



	return m_ServiceList;
}


void xmlOPer::ProcessNodes(CComPtr<IXMLDOMNode>& spNode, list<X_ScalarWebAPI_ServiceList> &ServiceList, X_ScalarWebAPI_ServiceList *m_ServiceList, int &index)
{
	if (ServiceList.size() % 2 == 0)
	{
		//�Ѿ�û���ӽڵ��ˣ�Ӧȡ��ֵ
		m_ServiceList = new X_ScalarWebAPI_ServiceList();
	}

	CComBSTR bsNodeName;
	spNode->get_nodeName(&bsNodeName); //�ڵ���
	//AfxMessageBox(COLE2CT(bsNodeName));
	//CComVariant varVal;
	//spNode->get_nodeValue(&varVal); //�ڵ�ֵ
	//AfxMessageBox(COLE2CT(varVal.bstrVal));

	DOMNodeType eNodeType;
	spNode->get_nodeType(&eNodeType);
	if (eNodeType == NODE_ELEMENT) //ֻ��NODE_ELEMENT���Ͳ��ܰ��������Ժ��ӽڵ�
	{
		//�ݹ�����ڵ�����
		CComPtr<IXMLDOMNamedNodeMap> spNameNodeMap;
		spNode->get_attributes(&spNameNodeMap);
		long nLength;
		spNameNodeMap->get_length(&nLength);
		for (long i = 0; i != nLength; ++i)
		{
			CComPtr<IXMLDOMNode> spNodeAttrib; //ע������Ҳ��һ��IXMLDOMNode
			spNameNodeMap->get_item(i, &spNodeAttrib);
			ProcessNodes(spNodeAttrib, ServiceList, m_ServiceList, index);
		}
		//�ݹ�����ӽڵ�
		CComPtr<IXMLDOMNodeList> spNodeList;
		spNode->get_childNodes(&spNodeList);
		spNodeList->get_length(&nLength);
		if (nLength == 1)
		{

			key_value m_key_value = selectSingleNodeKeyValue(spNode);

			m_ServiceList->X_ScalarWebAPI_AccessType = m_key_value.xml_value;

			m_ServiceList->index = ServiceList.size();
			ServiceList.push_back(*m_ServiceList);
			//AfxMessageBox(m_key_value.xml_key+" "+ m_key_value.xml_value);
		}
		else
		{

			for (long i = 0; i != nLength; ++i)
			{
				CComPtr<IXMLDOMNode> spChildNode;
				spNodeList->get_item(i, &spChildNode);
				//ProcessNodes(spChildNode);
				ProcessNodes(spChildNode, ServiceList, m_ServiceList, index);
			}
		}


	}
}
CComPtr<IXMLDOMDocument> xmlOPer::getDocument()
{
	if (spDoc != NULL)
		return spDoc;
}