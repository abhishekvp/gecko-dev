/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#include "nsIDOMHTMLScriptElement.h"
#include "nsIScriptObjectOwner.h"
#include "nsIDOMEventReceiver.h"
#include "nsIHTMLContent.h"
#include "nsHTMLGenericContent.h"
#include "nsHTMLAtoms.h"
#include "nsHTMLIIDs.h"
#include "nsIStyleContext.h"
#include "nsStyleConsts.h"
#include "nsIPresContext.h"

static NS_DEFINE_IID(kIDOMHTMLScriptElementIID, NS_IDOMHTMLSCRIPTELEMENT_IID);

class nsHTMLScript : public nsIDOMHTMLScriptElement,
                     public nsIScriptObjectOwner,
                     public nsIDOMEventReceiver,
                     public nsIHTMLContent
{
public:
  nsHTMLScript(nsIAtom* aTag);
  ~nsHTMLScript();

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIDOMNode
  NS_IMPL_IDOMNODE_USING_GENERIC(mInner)

  // nsIDOMElement
  NS_IMPL_IDOMELEMENT_USING_GENERIC(mInner)

  // nsIDOMHTMLElement
  NS_IMPL_IDOMHTMLELEMENT_USING_GENERIC(mInner)

  // nsIDOMHTMLScriptElement
  NS_IMETHOD GetText(nsString& aText);
  NS_IMETHOD SetText(const nsString& aText);
  NS_IMETHOD GetHtmlFor(nsString& aHtmlFor);
  NS_IMETHOD SetHtmlFor(const nsString& aHtmlFor);
  NS_IMETHOD GetEvent(nsString& aEvent);
  NS_IMETHOD SetEvent(const nsString& aEvent);
  NS_IMETHOD GetCharset(nsString& aCharset);
  NS_IMETHOD SetCharset(const nsString& aCharset);
  NS_IMETHOD GetDefer(PRBool* aDefer);
  NS_IMETHOD SetDefer(PRBool aDefer);
  NS_IMETHOD GetSrc(nsString& aSrc);
  NS_IMETHOD SetSrc(const nsString& aSrc);
  NS_IMETHOD GetType(nsString& aType);
  NS_IMETHOD SetType(const nsString& aType);

  // nsIScriptObjectOwner
  NS_IMPL_ISCRIPTOBJECTOWNER_USING_GENERIC(mInner)

  // nsIDOMEventReceiver
  NS_IMPL_IDOMEVENTRECEIVER_USING_GENERIC(mInner)

  // nsIContent
  NS_IMPL_ICONTENT_USING_GENERIC(mInner)

  // nsIHTMLContent
  NS_IMPL_IHTMLCONTENT_USING_GENERIC(mInner)

protected:
  nsHTMLGenericLeafContent mInner;
  nsString mText;
};

nsresult
NS_NewHTMLScript(nsIHTMLContent** aInstancePtrResult, nsIAtom* aTag)
{
  NS_PRECONDITION(nsnull != aInstancePtrResult, "null ptr");
  if (nsnull == aInstancePtrResult) {
    return NS_ERROR_NULL_POINTER;
  }
  nsIHTMLContent* it = new nsHTMLScript(aTag);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return it->QueryInterface(kIHTMLContentIID, (void**) aInstancePtrResult);
}

nsHTMLScript::nsHTMLScript(nsIAtom* aTag)
{
  NS_INIT_REFCNT();
  mInner.Init(this, aTag);
}

nsHTMLScript::~nsHTMLScript()
{
}

NS_IMPL_ADDREF(nsHTMLScript)

NS_IMPL_RELEASE(nsHTMLScript)

nsresult
nsHTMLScript::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  NS_IMPL_HTML_CONTENT_QUERY_INTERFACE(aIID, aInstancePtr, this)
  if (aIID.Equals(kIDOMHTMLScriptElementIID)) {
    nsIDOMHTMLScriptElement* tmp = this;
    *aInstancePtr = (void*) tmp;
    AddRef();
    return NS_OK;
  }
  return NS_NOINTERFACE;
}

nsresult
nsHTMLScript::CloneNode(nsIDOMNode** aReturn)
{
  nsHTMLScript* it = new nsHTMLScript(mInner.mTag);
  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  mInner.CopyInnerTo(this, &it->mInner);
  return it->QueryInterface(kIDOMNodeIID, (void**) aReturn);
}

NS_IMETHODIMP
nsHTMLScript::GetText(nsString& aValue)
{
  // XXX out of memory errors
  aValue = mText;
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::SetText(const nsString& aValue)
{
  // XXX out of memory errors
  mText = aValue;
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::GetHtmlFor(nsString& aValue)
{
  // XXX write me
//  mInner.GetAttribute(nsHTMLAtoms::charset, aValue);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::SetHtmlFor(const nsString& aValue)
{
  // XXX write me
//  return mInner.SetAttr(nsHTMLAtoms::charset, aValue, eSetAttrNotify_None);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::GetEvent(nsString& aValue)
{
  // XXX write me
//  mInner.GetAttribute(nsHTMLAtoms::charset, aValue);
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::SetEvent(const nsString& aValue)
{
  // XXX write me
//  return mInner.SetAttr(nsHTMLAtoms::charset, aValue, eSetAttrNotify_None);
  return NS_OK;
}

NS_IMPL_STRING_ATTR(nsHTMLScript, Charset, charset, eSetAttrNotify_None)
NS_IMPL_BOOL_ATTR(nsHTMLScript, Defer, defer, eSetAttrNotify_None)
NS_IMPL_STRING_ATTR(nsHTMLScript, Src, src, eSetAttrNotify_Restart)
NS_IMPL_STRING_ATTR(nsHTMLScript, Type, type, eSetAttrNotify_Restart)

NS_IMETHODIMP
nsHTMLScript::StringToAttribute(nsIAtom* aAttribute,
                                const nsString& aValue,
                                nsHTMLValue& aResult)
{
  return NS_CONTENT_ATTR_NOT_THERE;
}

NS_IMETHODIMP
nsHTMLScript::AttributeToString(nsIAtom* aAttribute,
                                nsHTMLValue& aValue,
                                nsString& aResult) const
{
  return mInner.AttributeToString(aAttribute, aValue, aResult);
}

NS_IMETHODIMP
nsHTMLScript::MapAttributesInto(nsIStyleContext* aContext,
                                nsIPresContext* aPresContext)
{
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLScript::HandleDOMEvent(nsIPresContext& aPresContext,
                             nsEvent* aEvent,
                             nsIDOMEvent** aDOMEvent,
                             PRUint32 aFlags,
                             nsEventStatus& aEventStatus)
{
  return mInner.HandleDOMEvent(aPresContext, aEvent, aDOMEvent,
                               aFlags, aEventStatus);
}
