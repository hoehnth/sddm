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

    property var dialog

    // picture box, password field
    property var pwdItem

    // if provided supposed to have text property
    property var errMsg
    property var txtMsg

    // gets focus when password renewal dialog closes
    property var getsBackFocus

    property int pam_maxtries_result: 11 // TODO: enum

    TextConstants { id: textConstants }

    function clearPwd() {
        if(typeof pwdItem.password !== "undefined")
            pwdItem.password = ""
        else if(typeof pwdItem.text !== "undefined")
            pwdItem.text = ""
        if(typeof getsBackFocus !== "undefined")
            getsBackFocus.forceActiveFocus()
    }

    // tell greeter we handle expired passwords,
    // default is no password renewal in old themes
    Component.onCompleted: sddm.enablePwdRenewal()

    // handles password renewal events
    Connections {

        target: dialog

        onOk: {
            sddm.pamResponse(dialog.password)
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
            if(!dialog.visible) {
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
            // ...not reached as greeter stops after login succeeded...
        }

        onLoginFailed: {
            dialog.close()
            clearPwd() // picture box input
            // have a text field to show error output?
            if(typeof txtMsg !== "undefined") {
                // explain why password change dialog suddenly disappears
                // pam_chauthtok failed with PAM_MAXTRIES
                if(result == pam_maxtries_result)
                    txtMsg.text = textConstants.pamMaxtriesError
                else // filter out login failure details
                    txtMsg.text = textConstants.loginFailed
            }
        }

        // show messages from pam conversation
        onPamConvMsg: {
            // from signal pamConvMsg(pam_msg, result)
            dialog.append(pam_msg)
            /*
            // hint for user why current password is asked again
            if(result == pam_maxtries_result)
                dialog.append(textConstants.pamMaxtriesInfo)
            */
        }

        // new prompt arrived from pam_chauthtok,
        // e.g. for password change, prompt for current or new password
        onPamRequest: {
            // open password change dialog and block other GUI
            // NOTE: only one prompt per request supported!
            dialog.newPrompt(request.findChangePwdMessage() /* prompt message */)
        }
    }
}
