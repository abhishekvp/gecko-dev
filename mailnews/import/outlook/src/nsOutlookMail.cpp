/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *   Pierre Phaneuf <pp@ludusdesign.com>
 */

/*
  Outlook mail import
*/

#include "nsCOMPtr.h"
#include "nscore.h"
#include "nsIServiceManager.h"
#include "nsIImportService.h"
#include "nsIImportFieldMap.h"
#include "nsIImportMailboxDescriptor.h"
#include "nsIImportABDescriptor.h"
#include "nsIImportMimeEncode.h"
#include "nsXPIDLString.h"
#include "nsOutlookStringBundle.h"
#include "nsABBaseCID.h"
#include "nsIAbCard.h"
#include "nsIAddrDatabase.h"
#include "OutlookDebugLog.h"
#include "nsOutlookMail.h"

static NS_DEFINE_CID(kImportServiceCID,		NS_IMPORTSERVICE_CID);
static NS_DEFINE_CID(kImportMimeEncodeCID,	NS_IMPORTMIMEENCODE_CID);
static NS_DEFINE_IID(kISupportsIID,			NS_ISUPPORTS_IID);

/* ------------ Address book stuff ----------------- */
typedef struct {
	PRInt32		mozField;
	PRInt32		multiLine;
	ULONG		mapiTag;
} MAPIFields;

/*
	Fields in MAPI, not in Mozilla
	PR_OFFICE_LOCATION
	FIX - PR_BIRTHDAY - stored as PT_SYSTIME - FIX to extract for moz address book birthday
	PR_DISPLAY_NAME_PREFIX - Mr., Mrs. Dr., etc.
	PR_SPOUSE_NAME
	PR_GENDER - integer, not text
	FIX - PR_CONTACT_EMAIL_ADDRESSES - multiuline strings for email addresses, needs
		parsing to get secondary email address for mozilla
*/

#define kIsMultiLine	-2
#define	kNoMultiLine	-1

MAPIFields	gMapiFields[] = {
	{ 35, kIsMultiLine, PR_COMMENT},
	{ 6, kNoMultiLine, PR_BUSINESS_TELEPHONE_NUMBER},
	{ 7, kNoMultiLine, PR_HOME_TELEPHONE_NUMBER},
	{ 25, kNoMultiLine, PR_COMPANY_NAME},
	{ 23, kNoMultiLine, PR_TITLE},
	{ 10, kNoMultiLine, PR_CELLULAR_TELEPHONE_NUMBER},
	{ 9, kNoMultiLine, PR_PAGER_TELEPHONE_NUMBER},
	{ 8, kNoMultiLine, PR_BUSINESS_FAX_NUMBER},
	{ 8, kNoMultiLine, PR_HOME_FAX_NUMBER},
	{ 22, kNoMultiLine, PR_COUNTRY},
	{ 19, kNoMultiLine, PR_LOCALITY},
	{ 20, kNoMultiLine, PR_STATE_OR_PROVINCE},
	{ 17, 18, PR_STREET_ADDRESS},
	{ 21, kNoMultiLine, PR_POSTAL_CODE},
	{ 26, kNoMultiLine, PR_PERSONAL_HOME_PAGE},
	{ 27, kNoMultiLine, PR_BUSINESS_HOME_PAGE},
	{ 13, kNoMultiLine, PR_HOME_ADDRESS_CITY},
	{ 16, kNoMultiLine, PR_HOME_ADDRESS_COUNTRY},
	{ 15, kNoMultiLine, PR_HOME_ADDRESS_POSTAL_CODE},
	{ 14, kNoMultiLine, PR_HOME_ADDRESS_STATE_OR_PROVINCE},
	{ 11, 12, PR_HOME_ADDRESS_STREET},
	{ 24, kNoMultiLine, PR_DEPARTMENT_NAME}
};
/* ---------------------------------------------------- */


#define	kCopyBufferSize		(16 * 1024)


nsOutlookMail::nsOutlookMail()
{
	m_gotAddresses = PR_FALSE;
	m_gotFolders = PR_FALSE;
	m_haveMapi = CMapiApi::LoadMapi();
	m_lpMdb = NULL;
}

nsOutlookMail::~nsOutlookMail()
{
	EmptyAttachments();
}

nsresult nsOutlookMail::GetMailFolders( nsISupportsArray **pArray)
{
	if (!m_haveMapi) {
		IMPORT_LOG0( "GetMailFolders called before Mapi is initialized\n");
		return( NS_ERROR_FAILURE);
	}

	nsresult rv = NS_NewISupportsArray( pArray);
	if (NS_FAILED( rv)) {
		IMPORT_LOG0( "FAILED to allocate the nsISupportsArray for the mail folder list\n");
		return( rv);
	}

	NS_WITH_SERVICE( nsIImportService, impSvc, kImportServiceCID, &rv);
	if (NS_FAILED( rv))
		return( rv);

	m_gotFolders = PR_TRUE;

	m_folderList.ClearAll();

	m_mapi.Initialize();
	m_mapi.LogOn();

	if (m_storeList.GetSize() == 0)
		m_mapi.IterateStores( m_storeList);

	int i = 0;
	CMapiFolder *pFolder;
	if (m_storeList.GetSize() > 1) {
		while ((pFolder = m_storeList.GetItem( i))) {
			CMapiFolder *pItem = new CMapiFolder( pFolder);
			pItem->SetDepth( 1);
			m_folderList.AddItem( pItem);
			if (!m_mapi.GetStoreFolders( pItem->GetCBEntryID(), pItem->GetEntryID(), m_folderList, 2)) {
				IMPORT_LOG1( "GetStoreFolders for index %d failed.\n", i);
			}
			i++;
		}
	}
	else {
		if ((pFolder = m_storeList.GetItem( i))) {
			if (!m_mapi.GetStoreFolders( pFolder->GetCBEntryID(), pFolder->GetEntryID(), m_folderList, 1)) {
				IMPORT_LOG1( "GetStoreFolders for index %d failed.\n", i);
			}
		}
	}
	
	// Create the mailbox descriptors for the list of folders
	nsIImportMailboxDescriptor *	pID;
	nsISupports *					pInterface;
	nsCString						name;
	PRUnichar *						pChar;

	for (i = 0; i < m_folderList.GetSize(); i++) {
		pFolder = m_folderList.GetItem( i);
		rv = impSvc->CreateNewMailboxDescriptor( &pID);
		if (NS_SUCCEEDED( rv)) {
			pID->SetDepth( pFolder->GetDepth());
			pID->SetIdentifier( i);
			pFolder->GetDisplayName( name);
			pChar = name.ToNewUnicode();
			pID->SetDisplayName( pChar);
			nsCRT::free( pChar);
			pID->SetSize( 1000);
			rv = pID->QueryInterface( kISupportsIID, (void **) &pInterface);
			(*pArray)->AppendElement( pInterface);
			pInterface->Release();
			pID->Release();
		}
	}
	
	return( NS_OK);	
}

PRBool nsOutlookMail::IsAddressBookNameUnique( nsCString& name, nsCString& list)
{
	nsCString		usedName = "[";
	usedName.Append( name);
	usedName.Append( "],");

	return( list.Find( usedName) == -1);
}

void nsOutlookMail::MakeAddressBookNameUnique( nsCString& name, nsCString& list)
{
	nsCString		newName;
	int				idx = 1;

	newName = name;
	while (!IsAddressBookNameUnique( newName, list)) {
		newName = name;
		newName.Append( ' ');
		newName.Append( (PRInt32) idx);
		idx++;
	}
	
	name = newName;
	list.Append( "[");
	list.Append( name);
	list.Append( "],");
}

nsresult nsOutlookMail::GetAddressBooks( nsISupportsArray **pArray)
{
	if (!m_haveMapi) {
		IMPORT_LOG0( "GetAddressBooks called before Mapi is initialized\n");
		return( NS_ERROR_FAILURE);
	}

	nsresult rv = NS_NewISupportsArray( pArray);
	if (NS_FAILED( rv)) {
		IMPORT_LOG0( "FAILED to allocate the nsISupportsArray for the address book list\n");
		return( rv);
	}

	NS_WITH_SERVICE( nsIImportService, impSvc, kImportServiceCID, &rv);
	if (NS_FAILED( rv))
		return( rv);

	m_gotAddresses = PR_TRUE;
	
	m_addressList.ClearAll();
	m_mapi.Initialize();
	m_mapi.LogOn();
	if (m_storeList.GetSize() == 0)
		m_mapi.IterateStores( m_storeList);

	int i = 0;
	CMapiFolder *pFolder;
	if (m_storeList.GetSize() > 1) {
		while ((pFolder = m_storeList.GetItem( i))) {
			CMapiFolder *pItem = new CMapiFolder( pFolder);
			pItem->SetDepth( 1);
			m_addressList.AddItem( pItem);
			if (!m_mapi.GetStoreAddressFolders( pItem->GetCBEntryID(), pItem->GetEntryID(), m_addressList)) {
				IMPORT_LOG1( "GetStoreAddressFolders for index %d failed.\n", i);
			}
			i++;
		}
	}
	else {
		if ((pFolder = m_storeList.GetItem( i))) {
			if (!m_mapi.GetStoreAddressFolders( pFolder->GetCBEntryID(), pFolder->GetEntryID(), m_addressList)) {
				IMPORT_LOG1( "GetStoreFolders for index %d failed.\n", i);
			}
		}
	}
	
	// Create the mailbox descriptors for the list of folders
	nsIImportABDescriptor *			pID;
	nsISupports *					pInterface;
	nsCString						name;
	PRUnichar *						pChar;
	nsCString						list;

	for (i = 0; i < m_addressList.GetSize(); i++) {
		pFolder = m_addressList.GetItem( i);
		if (!pFolder->IsStore()) {
			rv = impSvc->CreateNewABDescriptor( &pID);
			if (NS_SUCCEEDED( rv)) {
				pID->SetIdentifier( i);
				pFolder->GetDisplayName( name);
				MakeAddressBookNameUnique( name, list);
				pChar = name.ToNewUnicode();
				pID->SetPreferredName( pChar);
				nsCRT::free( pChar);
				pID->SetSize( 100);
				rv = pID->QueryInterface( kISupportsIID, (void **) &pInterface);
				(*pArray)->AppendElement( pInterface);
				pInterface->Release();
				pID->Release();
			}
		}
	}
	
	return( NS_OK);	
}



void nsOutlookMail::OpenMessageStore( CMapiFolder *pNextFolder)
{	
	// Open the store specified
	if (pNextFolder->IsStore()) {
		if (!m_mapi.OpenStore( pNextFolder->GetCBEntryID(), pNextFolder->GetEntryID(), &m_lpMdb)) {
			m_lpMdb = NULL;
			IMPORT_LOG0( "CMapiApi::OpenStore failed\n");
		}
		
		return;
	}
	
	// Check to see if we should open the one and only store
	if (!m_lpMdb) {
		if (m_storeList.GetSize() == 1) {
			CMapiFolder * pFolder = m_storeList.GetItem( 0);
			if (pFolder) {
				if (!m_mapi.OpenStore( pFolder->GetCBEntryID(), pFolder->GetEntryID(), &m_lpMdb)) {
					m_lpMdb = NULL;
					IMPORT_LOG0( "CMapiApi::OpenStore failed\n");
				}
			}
			else {
				IMPORT_LOG0( "Error retrieving the one & only message store\n");
			}
		}
		else {
			IMPORT_LOG0( "*** Error importing a folder without a valid message store\n");
		}
	}
}

nsresult nsOutlookMail::ImportMailbox( PRUint32 *pDoneSoFar, PRBool *pAbort, PRInt32 index, const PRUnichar *pName, nsIFileSpec *pDest, PRInt32 *pMsgCount)
{
	if ((index < 0) || (index >= m_folderList.GetSize())) {
		IMPORT_LOG0( "*** Bad mailbox identifier, unable to import\n");
		*pAbort = PR_TRUE;
		return( NS_ERROR_FAILURE);
	}
	
	PRInt32		dummyMsgCount = 0;
	if (pMsgCount)
		*pMsgCount = 0;
	else
		pMsgCount = &dummyMsgCount;

	CMapiFolder *pFolder = m_folderList.GetItem( index);
	OpenMessageStore( pFolder);
	if (!m_lpMdb) {
		IMPORT_LOG1( "*** Unable to obtain mapi message store for mailbox: %S\n", pName);
		return( NS_ERROR_FAILURE);
	}
	
	if (pFolder->IsStore())
		return( NS_OK);
	
	nsCOMPtr<nsIFileSpec>	compositionFile;
	nsresult	rv;
	if (NS_FAILED( rv = NS_NewFileSpec( getter_AddRefs( compositionFile)))) {
		return( rv);
	}
	
	nsOutlookCompose		compose;
	SimpleBuffer			copy;

	copy.Allocate( kCopyBufferSize);

	// now what?
	CMapiFolderContents		contents( m_lpMdb, pFolder->GetCBEntryID(), pFolder->GetEntryID());

	BOOL		done = FALSE;
	ULONG		cbEid;
	LPENTRYID	lpEid;
	ULONG		oType;
	LPMESSAGE	lpMsg;
	int			attachCount;
	ULONG		totalCount;
	PRFloat64	doneCalc;
	nsCString	fromLine;
	int			fromLen;
	PRBool		lostAttach = PR_FALSE;

	while (!done) {
		(*pMsgCount)++;

		if (!contents.GetNext( &cbEid, &lpEid, &oType, &done)) {
			IMPORT_LOG1( "*** Error iterating mailbox: %S\n", pName);
			return( NS_ERROR_FAILURE);
		}
		
		totalCount = contents.GetCount();
		doneCalc = *pMsgCount;
		doneCalc /= totalCount;
		doneCalc *= 1000;
		if (pDoneSoFar) {
			*pDoneSoFar = (PRUint32) doneCalc;
			if (*pDoneSoFar > 1000)
				*pDoneSoFar = 1000;
		}

		if (!done && (oType == MAPI_MESSAGE)) {
			if (!m_mapi.OpenMdbEntry( m_lpMdb, cbEid, lpEid, (LPUNKNOWN *) &lpMsg)) {
				IMPORT_LOG1( "*** Error opening messages in mailbox: %S\n", pName);
				return( NS_ERROR_FAILURE);
			}
			
			CMapiMessage	msg( lpMsg);
	
			BOOL bResult = msg.FetchHeaders();
			if (bResult)
				bResult = msg.FetchBody();
			if (bResult)
				fromLine = msg.GetFromLine( fromLen);

			attachCount = msg.CountAttachments();
			BuildAttachments( msg, attachCount);

			if (!bResult) {
				IMPORT_LOG1( "*** Error reading message from mailbox: %S\n", pName);
				return( NS_ERROR_FAILURE);
			}
						
			// --------------------------------------------------------------
			compose.SetBody( msg.GetBody(), msg.GetBodyLen());
			compose.SetHeaders( msg.GetHeaders(), msg.GetHeaderLen());
			compose.SetAttachments( &m_attachments);

			/*
			if ((*pMsgCount) == 163)
				NS_PRECONDITION( FALSE, "Manual breakpoint");
			*/
			
			/*
				If I can't get no headers,
				I can't get no satisfaction
			*/
			if (msg.GetHeaderLen()) {
				rv = compose.SendTheMessage( compositionFile);
				if (NS_SUCCEEDED( rv)) {
					rv = compose.CopyComposedMessage( fromLine, compositionFile, pDest, copy);
					DeleteFile( compositionFile);
					if (NS_FAILED( rv)) {
						IMPORT_LOG0( "*** Error copying composed message to destination mailbox\n");
						return( rv);
					}
				}
			}
			else
				rv = NS_OK;

			if (NS_FAILED( rv)) {
				
				/* NS_PRECONDITION( FALSE, "Manual breakpoint"); */

				IMPORT_LOG1( "Message #%d failed.\n", (int) (*pMsgCount));
				DumpAttachments();
	
				// --------------------------------------------------------------

				// This is the OLD way of writing out the message which uses
				// all kinds of crufty old crap for attachments.
				// Since we now use Compose to send attachments, 
				// this is only fallback error stuff.

				// Attachments get lost.		

				if (attachCount) {
					lostAttach = PR_TRUE;
					attachCount = 0;
				}

				BOOL needsTerminate = FALSE;
				if (!WriteMessage( pDest, &msg, attachCount, &needsTerminate)) {
					IMPORT_LOG0( "*** Error writing message\n");
					*pAbort = PR_TRUE;
					return( NS_ERROR_FAILURE);
				}
				
				if (needsTerminate) {
					if (!WriteMimeBoundary( pDest, &msg, TRUE)) {
						IMPORT_LOG0( "*** Error writing message mime boundary\n");
						*pAbort = PR_TRUE;
						return( NS_ERROR_FAILURE);
					}
				}
			}

			// Just for YUCKS, let's try an extra endline
			WriteData( pDest, "\x0D\x0A", 2);
		}
	}


	return( NS_OK);
}

BOOL nsOutlookMail::WriteMessage( nsIFileSpec *pDest, CMapiMessage *pMsg, int& attachCount, BOOL *pTerminate)
{
	BOOL		bResult = TRUE;
	const char *pData;
	int			len;
	BOOL		checkStart = FALSE;

	*pTerminate = FALSE;

	pData = pMsg->GetFromLine( len);
	if (pData) {
		bResult = WriteData( pDest, pData, len);
		checkStart = TRUE;
	}

	pData = pMsg->GetHeaders( len);
	if (pData && len) {
		bResult = WriteWithoutFrom( pDest, pData, len, checkStart);
	}

	// Do we need to add any mime headers???
	//	Is the message multipart?
	//		If so, then we are OK, but need to make sure we add mime
	//		header info to the body of the message
	//	If not AND we have attachments, then we need to add mime headers.
	
	BOOL needsMimeHeaders = pMsg->IsMultipart();
	if (!needsMimeHeaders && attachCount) {
		// if the message already has mime headers
		// that aren't multipart then we are in trouble!
		// in that case, ditch the attachments...  alternatively, we could
		// massage the headers and replace the existing mime headers
		// with our own but I think this case is likely not to occur.
		if (pMsg->HasContentHeader())
			attachCount = 0;
		else {
			if (bResult)
				bResult = WriteMimeMsgHeader( pDest, pMsg);
			needsMimeHeaders = TRUE;
		}
	}

	if (bResult)
		bResult = WriteStr( pDest, "\x0D\x0A");
	
	if (needsMimeHeaders) {
		if (bResult)
			bResult = WriteStr( pDest, "This is a MIME formatted message.\x0D\x0A");
		if (bResult)
			bResult = WriteStr( pDest, "\x0D\x0A");
		if (bResult)
			bResult = WriteMimeBoundary( pDest, pMsg, FALSE);
		if (pMsg->BodyIsHtml()) {
			if (bResult)
				bResult = WriteStr( pDest, "Content-type: text/html\x0D\x0A");
		}
		else {
			if (bResult)
				bResult = WriteStr( pDest, "Content-type: text/plain\x0D\x0A");
		}

		if (bResult)
			bResult = WriteStr( pDest, "\x0D\x0A");
	}


	pData = pMsg->GetBody( len);
	if (pData && len) {
		if (bResult)
			bResult = WriteWithoutFrom( pDest, pData, len, TRUE);
		if ((len < 2) || (pData[len - 1] != 0x0A) || (pData[len - 2] != 0x0D))
			bResult = WriteStr( pDest, "\x0D\x0A");
	}

	*pTerminate = needsMimeHeaders;

	return( bResult);
}


BOOL nsOutlookMail::WriteData( nsIFileSpec *pDest, const char *pData, PRInt32 len)
{	
	PRInt32		written;
	nsresult rv = pDest->Write( pData, len, &written);
	if (NS_FAILED( rv) || (written != len))
		return( FALSE);
	return( TRUE);
}

BOOL nsOutlookMail::WriteStr( nsIFileSpec *pDest, const char *pStr)
{	
	PRInt32		written;
	PRInt32		len = nsCRT::strlen( pStr);

	nsresult rv = pDest->Write( pStr, len, &written);
	if (NS_FAILED( rv) || (written != len))
		return( FALSE);
	return( TRUE);
}

BOOL nsOutlookMail::WriteWithoutFrom( nsIFileSpec *pDest, const char *pData, int len, BOOL checkStart)
{
	int				wLen = 0;
	const char *	pChar = pData;
	const char *	pStart = pData;

	if (!len)
		return( TRUE);
	if (!checkStart) {
		pChar++;
		wLen++;
		len--;
	}
	else {
		if ((len >= 5) && !nsCRT::strncmp( pChar, "From ", 5)) {
			if (!WriteStr( pDest, ">"))
				return( FALSE);
		}
	}

	while (len) {
		if ((len >= 7) && !nsCRT::strncmp( pChar, ("\x0D\x0A" "From "), 7)) {
			wLen += 2;
			len -= 2;
			pChar += 2;
			if (!WriteData( pDest, pStart, wLen))
				return( FALSE);
			if (!WriteStr( pDest, ">"))
				return( FALSE);
			wLen = 5;
			pStart = pChar;
			pChar += 5;
			len -= 5;
		}
		else {
			wLen++;
			pChar++;
			len--;
		}
	}

	if (wLen) {
		if (!WriteData( pDest, pStart, wLen))
			return( FALSE);	
	}

	return( TRUE);
}

BOOL nsOutlookMail::WriteMimeMsgHeader( nsIFileSpec *pDest, CMapiMessage *pMsg)
{
	BOOL	bResult = TRUE;
	if (!pMsg->HasMimeVersion())
		bResult = WriteStr( pDest, "MIME-Version: 1.0\x0D\x0A");
	pMsg->GenerateBoundary();
	if (bResult)
		bResult = WriteStr( pDest, "Content-type: multipart/mixed; boundary=\"");
	if (bResult)
		bResult = WriteStr( pDest, pMsg->GetMimeBoundary());
	if (bResult)
		bResult = WriteStr( pDest, "\"\x0D\x0A");

	return( bResult);
}

BOOL nsOutlookMail::WriteMimeBoundary( nsIFileSpec *pDest, CMapiMessage *pMsg, BOOL terminate)
{
	BOOL	bResult = WriteStr( pDest, "--");
	if (bResult)
		bResult = WriteStr( pDest, pMsg->GetMimeBoundary());
	if (terminate && bResult)
		bResult = WriteStr( pDest, "--");
	if (bResult)
		bResult = WriteStr( pDest, "\x0D\x0A");

	return( bResult);
}


/*
PRBool nsOutlookMail::WriteAttachment( nsIFileSpec *pDest, CMapiMessage *pMsg)
{
	nsCOMPtr<nsIFileSpec> pSpec;
	nsresult rv = NS_NewFileSpec( getter_AddRefs( pSpec));
	if (NS_FAILED( rv) || !pSpec) {
		IMPORT_LOG0( "*** Error creating file spec for attachment\n");
		return( PR_FALSE);
	}

	if (pMsg->GetAttachFileLoc( pSpec)) {
		PRBool	isFile = PR_FALSE;
		PRBool	exists = PR_FALSE;
		pSpec->Exists( &exists);
		pSpec->IsFile( &isFile);

		if (!exists || !isFile) {
			IMPORT_LOG0( "Attachment file does not exist\n");
			return( PR_TRUE);
		}
	}
	else {
		IMPORT_LOG0( "Attachment not processed, unable to obtain file\n");
		return( PR_TRUE);
	}

	// Set up headers...
	BOOL bResult = WriteMimeBoundary( pDest, pMsg, FALSE);
	// Now set up the encoder object

	if (bResult) {
		nsCOMPtr<nsIImportMimeEncode> encoder;
		rv = nsComponentManager::CreateInstance( kImportMimeEncodeCID, nsnull, NS_GET_IID(nsIImportMimeEncode), getter_AddRefs( encoder));
		if (NS_FAILED( rv)) {
			IMPORT_LOG0( "*** Error creating mime encoder\n");
			return( PR_FALSE);
		}

		encoder->Initialize( pSpec, pDest, pMsg->GetFileName(), pMsg->GetMimeType());
		encoder->DoEncoding( &bResult);
	}

	return( bResult);
}
*/






nsresult nsOutlookMail::DeleteFile( nsIFileSpec *pSpec)
{
	PRBool		result;
	nsresult	rv = NS_OK;

	result = PR_FALSE;
	pSpec->IsStreamOpen( &result);
	if (result)
		pSpec->CloseStream();
	result = PR_FALSE;
	pSpec->Exists( &result);
	if (result) {
		result = PR_FALSE;
		pSpec->IsFile( &result);
		if (result) {
			nsFileSpec	spec;
			rv = pSpec->GetFileSpec( &spec);
			if (NS_SUCCEEDED( rv))
				spec.Delete( PR_FALSE);
		}
	}

	return( rv);
}

void nsOutlookMail::EmptyAttachments( void)
{
	PRBool	exists;
	PRBool	isFile;
	PRInt32 max = m_attachments.Count();
	OutlookAttachment *	pAttach;
	for (PRInt32 i = 0; i < max; i++) {
		pAttach = (OutlookAttachment *) m_attachments.ElementAt( i);
		if (pAttach) {
			if (pAttach->pAttachment) {
				exists = PR_FALSE;
				isFile = PR_FALSE;
				pAttach->pAttachment->Exists( &exists);
				if (exists)
					pAttach->pAttachment->IsFile( &isFile);
				if (exists && isFile)
					DeleteFile( pAttach->pAttachment);
				NS_RELEASE( pAttach->pAttachment);
			}
			nsCRT::free( pAttach->description);
			nsCRT::free( pAttach->mimeType);
			delete pAttach;
		}
	}

	m_attachments.Clear();
}

void nsOutlookMail::BuildAttachments( CMapiMessage& msg, int count)
{
	EmptyAttachments();
	if (count) {
		nsIFileSpec *	pSpec;
		nsresult rv;
		for (int i = 0; i < count; i++) {
			if (!msg.GetAttachmentInfo( i)) {
				IMPORT_LOG1( "*** Error getting attachment info for #%d\n", i);
			}

			pSpec = nsnull;
			rv = NS_NewFileSpec( &pSpec);
			if (NS_FAILED( rv) || !pSpec) {
				IMPORT_LOG0( "*** Error creating file spec for attachment\n");
			}
			else {
				if (msg.GetAttachFileLoc( pSpec)) {
					PRBool	isFile = PR_FALSE;
					PRBool	exists = PR_FALSE;
					pSpec->Exists( &exists);
					pSpec->IsFile( &isFile);

					if (!exists || !isFile) {
						IMPORT_LOG0( "Attachment file does not exist\n");
						NS_RELEASE( pSpec);
					}
					else {
						// We have a file spec, now get the other info
						OutlookAttachment *a = new OutlookAttachment;
						a->mimeType = nsCRT::strdup( msg.GetMimeType());
						a->description = nsCRT::strdup( msg.GetFileName());
						if (!nsCRT::strlen( a->description)) {
							nsCRT::free( a->description);
							nsCString	str = "Attachment ";
							str.Append( (PRInt32) i);
							a->description = nsCRT::strdup( (const char *)str);
						}
						a->pAttachment = pSpec;
						m_attachments.AppendElement( a);
					}
				}
				else {
					NS_RELEASE( pSpec);
				}
			}
		}
	}
}

void nsOutlookMail::DumpAttachments( void)
{
#ifdef IMPORT_DEBUG
	PRInt32		count = 0;
	count = m_attachments.Count();
	if (!count) {
		IMPORT_LOG0( "*** No Attachments\n");
		return;
	}
	IMPORT_LOG1( "#%d attachments\n", (int) count);

	OutlookAttachment *	pAttach;
	
	for (PRInt32 i = 0; i < count; i++) {
		IMPORT_LOG1( "\tAttachment #%d ---------------\n", (int) i);
		pAttach = (OutlookAttachment *) m_attachments.ElementAt( i);
		if (pAttach->mimeType)
			IMPORT_LOG1( "\t\tMime type: %s\n", pAttach->mimeType);
		if (pAttach->description)
			IMPORT_LOG1( "\t\tDescription: %s\n", pAttach->description);
		if (pAttach->pAttachment) {
			nsXPIDLCString	path;
			pAttach->pAttachment->GetNativePath( getter_Copies( path));
			IMPORT_LOG1( "\t\tFile: %s\n", (const char *)path);
		}
	}
#endif
}

nsresult nsOutlookMail::ImportAddresses( PRUint32 *pCount, PRUint32 *pTotal, const PRUnichar *pName, PRUint32 id, nsIAddrDatabase *pDb, nsString& errors)
{
	if (id >= (PRUint32)(m_addressList.GetSize())) {
		IMPORT_LOG0( "*** Bad address identifier, unable to import\n");
		return( NS_ERROR_FAILURE);
	}
	
	PRUint32	dummyCount = 0;
	if (pCount)
		*pCount = 0;
	else
		pCount = &dummyCount;

	CMapiFolder *pFolder;
	if (id > 0) {
		PRInt32 idx = (PRInt32) id;
		idx--;
		while (idx >= 0) {
			pFolder = m_addressList.GetItem( idx);
			if (pFolder->IsStore()) {
				OpenMessageStore( pFolder);
				break;
			}
			idx--;
		}
	}

	pFolder = m_addressList.GetItem( id);
	OpenMessageStore( pFolder);
	if (!m_lpMdb) {
		IMPORT_LOG1( "*** Unable to obtain mapi message store for address book: %S\n", pName);
		return( NS_ERROR_FAILURE);
	}
	
	if (pFolder->IsStore())
		return( NS_OK);
	
	nsresult	rv;

	nsCOMPtr<nsIImportFieldMap>		pFieldMap;

	NS_WITH_SERVICE( nsIImportService, impSvc, kImportServiceCID, &rv);
	if (NS_SUCCEEDED( rv)) {
		rv = impSvc->CreateNewFieldMap( getter_AddRefs( pFieldMap));
	}

	CMapiFolderContents		contents( m_lpMdb, pFolder->GetCBEntryID(), pFolder->GetEntryID());

	BOOL			done = FALSE;
	ULONG			cbEid;
	LPENTRYID		lpEid;
	ULONG			oType;
	LPMESSAGE		lpMsg;
	nsCString		type;
	LPSPropValue	pVal;
	nsString		subject;

	while (!done) {
		(*pCount)++;

		if (!contents.GetNext( &cbEid, &lpEid, &oType, &done)) {
			IMPORT_LOG1( "*** Error iterating address book: %S\n", pName);
			return( NS_ERROR_FAILURE);
		}
		
		if (pTotal && (*pTotal == 0))
			*pTotal = contents.GetCount();

		if (!done && (oType == MAPI_MESSAGE)) {
			if (!m_mapi.OpenMdbEntry( m_lpMdb, cbEid, lpEid, (LPUNKNOWN *) &lpMsg)) {
				IMPORT_LOG1( "*** Error opening messages in mailbox: %S\n", pName);
				return( NS_ERROR_FAILURE);
			}
			
			// Get the PR_MESSAGE_CLASS attribute,
			// ensure that it is IPM.Contact
			pVal = m_mapi.GetMapiProperty( lpMsg, PR_MESSAGE_CLASS);
			if (pVal) {
				type.Truncate( 0);
				m_mapi.GetStringFromProp( pVal, type);
				if (!type.Compare( "IPM.Contact")) {
					// This is a contact, add it to the address book!
					subject.Truncate( 0);
					pVal = m_mapi.GetMapiProperty( lpMsg, PR_SUBJECT);
					if (pVal)
						m_mapi.GetStringFromProp( pVal, subject);
					
					nsIMdbRow* newRow = nsnull;
					pDb->GetNewRow( &newRow); 
					// FIXME: Check with Candice about releasing the newRow if it
					// isn't added to the database.  Candice's code in nsAddressBook
					// never releases it but that doesn't seem right to me!
					if (newRow) {
						if (BuildCard( subject.GetUnicode(), pDb, newRow, lpMsg, pFieldMap)) {
							pDb->AddCardRowToDB( newRow);
						}
					}
				}
			}			

			lpMsg->Release();
		}
	}


	return( NS_OK);

}


void nsOutlookMail::SanitizeValue( nsString& val)
{
	val.ReplaceSubstring( "\x0D\x0A", ", ");
	val.ReplaceChar( 13, ',');
	val.ReplaceChar( 10, ',');
}

void nsOutlookMail::SplitString( nsString& val1, nsString& val2)
{
	nsString	temp;

	// Find the last line if there is more than one!
	PRInt32 idx = val1.RFind( "\x0D\x0A");
	PRInt32	cnt = 2;
	if (idx == -1) {
		cnt = 1;
		idx = val1.RFindChar( 13);
	}
	if (idx == -1)
		idx= val1.RFindChar( 10);
	if (idx != -1) {
		val1.Right( val2, val1.Length() - idx - cnt);
		val1.Left( temp, idx);
		val1 = temp;
		SanitizeValue( val1);
	}
}

PRBool nsOutlookMail::BuildCard( const PRUnichar *pName, nsIAddrDatabase *pDb, nsIMdbRow *newRow, LPMAPIPROP pUser, nsIImportFieldMap *pFieldMap)
{
	
	nsString		lastName;
	nsString		firstName;
	nsString		eMail;
	nsString		nickName;
	nsString		middleName;

	LPSPropValue	pProp = m_mapi.GetMapiProperty( pUser, PR_EMAIL_ADDRESS);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, eMail);
		SanitizeValue( eMail);
	}

	pProp = m_mapi.GetMapiProperty( pUser, PR_GIVEN_NAME);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, firstName);
		SanitizeValue( firstName);
	}
	pProp = m_mapi.GetMapiProperty( pUser, PR_SURNAME);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, lastName);
		SanitizeValue( lastName);
	}
	pProp = m_mapi.GetMapiProperty( pUser, PR_MIDDLE_NAME);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, middleName);
		SanitizeValue( middleName);
	}
	pProp = m_mapi.GetMapiProperty( pUser, PR_NICKNAME);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, nickName);
		SanitizeValue( nickName);
	}
	if (firstName.IsEmpty() && lastName.IsEmpty()) {
		firstName = pName;
	}

	nsString	displayName;
	pProp = m_mapi.GetMapiProperty( pUser, PR_DISPLAY_NAME);
	if (pProp) {
		m_mapi.GetStringFromProp( pProp, displayName);
		SanitizeValue( displayName);
	}
	if (displayName.IsEmpty()) {
		if (firstName.IsEmpty())
			displayName = pName;
		else {
			displayName = firstName;
			if (!middleName.IsEmpty()) {
				displayName.Append( ' ');
				displayName.Append( middleName);
			}
			if (!lastName.IsEmpty()) {
				displayName.Append( ' ');
				displayName.Append( lastName);
			}
		}
	}
	
	char *pCStr;
	// We now have the required fields
	// write them out followed by any optional fields!
	if (!displayName.IsEmpty()) {
		pDb->AddDisplayName( newRow, pCStr = displayName.ToNewUTF8String());
		nsCRT::free( pCStr);
	}
	if (!firstName.IsEmpty()) {
		pDb->AddFirstName( newRow, pCStr = firstName.ToNewUTF8String());
		nsCRT::free( pCStr);
	}
	if (!lastName.IsEmpty()) {
		pDb->AddLastName( newRow, pCStr = lastName.ToNewUTF8String());
		nsCRT::free( pCStr);
	}
	if (!nickName.IsEmpty()) {
		pDb->AddNickName( newRow, pCStr = nickName.ToNewUTF8String());
		nsCRT::free( pCStr);
	}
	if (!eMail.IsEmpty()) {
		pDb->AddPrimaryEmail( newRow, pCStr = eMail.ToNewUTF8String());
		nsCRT::free( pCStr);
	}

	// Do all of the extra fields!

	nsString	value;
	nsString	line2;

	if (pFieldMap) {
		int max = sizeof( gMapiFields) / sizeof( MAPIFields);
		for (int i = 0; i < max; i++) {
			pProp = m_mapi.GetMapiProperty( pUser, gMapiFields[i].mapiTag);
			if (pProp) {
				m_mapi.GetStringFromProp( pProp, value);
				if (!value.IsEmpty()) {
					if (gMapiFields[i].multiLine == kNoMultiLine) {
						SanitizeValue( value);
						pFieldMap->SetFieldValue( pDb, newRow, gMapiFields[i].mozField, value.GetUnicode());
					}
					else if (gMapiFields[i].multiLine == kIsMultiLine) {
						pFieldMap->SetFieldValue( pDb, newRow, gMapiFields[i].mozField, value.GetUnicode());
					}
					else {
						line2.Truncate();
						SplitString( value, line2);
						if (!value.IsEmpty())
							pFieldMap->SetFieldValue( pDb, newRow, gMapiFields[i].mozField, value.GetUnicode());
						if (!line2.IsEmpty())
							pFieldMap->SetFieldValue( pDb, newRow, gMapiFields[i].multiLine, line2.GetUnicode());
					}
				}
			}
		}
	}


	return( PR_TRUE);
}
