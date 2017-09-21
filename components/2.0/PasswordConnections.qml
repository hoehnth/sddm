/***************************************************************************
* Copyright (c) 2017 Thomas Hoehn <thomas_hoehn@gmx.net>
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
* OR OTHER DEALINGS IN THE SOFTWARE.
*
* Note: container for password renewal logic
*
***************************************************************************/

import QtQuick 2.0
import SddmComponents 2.0

Item {

    property var pwdItem
    property var renewalDialog

    // if provided supposed to have text property
    property var errMsg
    property var txtMsg

    // gets focus when password renewal dialog closes
    property var getsBackFocus

    TextConstants { id: textConstants }

    function clearPwd() {
        if(typeof pwdItem.password !== "undefined")
            pwdItem.password = ""
        else if(typeof pwdItem.text !== "undefined")
            pwdItem.text = ""
    }

    // tell greeter we handle expired passwords,
    // default is no password renewal in old themes
    Component.onCompleted: sddm.enablePwdRenewal()

    // handles password renewal events
    Connections {

        target: renewalDialog

        onOk: {
            sddm.pamResponse(renewalDialog.password)
        }

        onCancel: {
            sddm.cancelPamConv()
        }

        onError: {
            // unlikely: invalid prompt,
            // i.e. new/repeat prompt for dialog not found in request
            if(typeof errMsg !== "undefined")
                errMsg.text = msg
        }

        onVisibleChanged: {
            if(!renewalDialog.visible) {
                renewalDialog.clear() // clear pam infos
                clearPwd()

                // last selected user or input field gets focus back
                if(typeof getsBackFocus !== "undefined")
                    getsBackFocus.forceActiveFocus()
            }
        }
    }

    Connections {

        target: sddm

        onLoginSucceeded: {
            // ...will not be reached as greeter stops after successfull login...
        }

        onLoginFailed: {
            if(typeof txtMsg !== "undefined")
            {
                txtMsg.text = textConstants.loginFailed
                // filter out login failure details
                if(err_msg != "Authentication failure" &&
                   err_msg != "Password change aborted." &&
                   err_msg != "Authentication token manipulation error")
                   txtMsg.text += "\n" + err_msg
            }
            renewalDialog.visible = false
            clearPwd()
        }

        // show messages from pam conversation (for expired passwords)
        onPamConvMsg: {
            // from signal pamConvMsg(pam_msg)
            renewalDialog.append(pam_msg)
        }

        // new pam request arrived, e.g. for expired password,
        onPamRequest: {
            // open password renewalDialog dialog and block other GUI
            renewalDialog.show(request.findNewPwdMessage(),
                               request.findRepeatPwdMessage())
        }
    }
}
