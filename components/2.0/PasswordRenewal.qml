/***************************************************************************
* Copyright (c) 2017 Thomas Hoehn <Hoehn.Thomas@heidenhain.de>
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
* Note: simple password input dialog for user with expired password (pam conversation)
*
***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

FocusScope {
    id: container
    implicitHeight: dialogHeight
    implicitWidth: infosWidth+2*margins

    readonly property string password: pwd1Input.text

    // prompt messages from pam
    property alias prompt1: prompt1Txt.text
    property alias prompt2: prompt2Txt.text

    // dialog window
    property int dialogHeight: 180
    property int margins: 8

    // infos text area with pam messages
    property int infosWidth: 520
    property int infosHeight: 10

    property color promptColor: "black"
    property color infosColor: "red"
    property alias color: dialog.color // dialog background
    property alias radius: dialog.radius
    property alias border: dialog.border

    // error texts
    property string noMatchWarning: qsTr("Passwords do not match!")
    property string samePwdWarning: qsTr("Password same as before. Please change!")
    property string pwdEmptyWarning: qsTr("Empty password. Please enter password!")

    signal ok // password input completed
    signal cancel // dialog canceled
    signal error(string msg) // invalid prompts

    // set prompts and show dialog
    function show(newPwdTxt, repeatPwdTxt) {
        if(newPwdTxt != "" && repeatPwdTxt != "")
        {
            prompt1Txt.text = newPwdTxt
            prompt2Txt.text = repeatPwdTxt
            container.visible = true
        } else {
            // todo: prompts unknown (new/repeat password)
            error("unknown pam message prompts!\n" +
                     "prompt1=" + newPwdTxt +
                     "prompt2=" + repeatPwdTxt)
        }
    }

    // clear all text
    function clear() {
       infoarea.text = ""
       pwd1Input.text = ""
       pwd2Input.text = ""
    }

    // append new pam error/info
    function append(msg) {
        infoarea.append(msg)
    }

    // internal

    function resetEmptyWarning() {
        emptyWarningShown = false
    }

    // flag: warning about empty password already shown?
    property bool emptyWarningShown: false
    // previously typed password (refused by pam)
    property string previousPwd: ""

    function validatePwds() {
        if(pwd1Input.text.length == 0 ||
           pwd2Input.text.length == 0)
        {
            if(emptyWarningShown==false)
            {
                infoarea.append(pwdEmptyWarning)
                emptyWarningShown = true
            }
            return
        }

        if(pwd1Input.text.length>0)
        {
            resetEmptyWarning()

            if(pwd1Input.text != pwd2Input.text) {
                infoarea.append(noMatchWarning)
                return
            }

            if(previousPwd == pwd1Input.text) {
                // if pam refuses new password (e.g. too similar)
                // and this is presented to pam several times pam
                // seems pissed off and stops conversation, so avoid
                infoarea.append(samePwdWarning)
                return
            }

            // new password ok
            previousPwd = pwd1Input.text
            ok()
        }
    }

    onVisibleChanged: {
        if(visible == true)
            pwd1Input.text = ""
            pwd2Input.text = ""
            pwd1Input.focus = true
            focus = true
    }

    Rectangle {
        id: dialog
        anchors.fill: parent

        GridLayout {
            id: contentGrid
            anchors.fill: parent
            columns: 2

            Item {
                width: infosWidth
                height: infosHeight+36
                Layout.columnSpan: 2
                Layout.minimumWidth: infosWidth
                Layout.margins: margins

                // show info/error from pam conv()
                TextArea {
                    id: infoarea
                    anchors.fill: parent
                    viewport.implicitHeight: infosHeight
                    backgroundVisible: false
                    activeFocusOnPress: false
                    readOnly: true
                    textColor: infosColor
                    frameVisible: false
                    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                }
            }

            // shows message (prompt) from pam conv()
            Text {
                id: prompt1Txt
                Layout.minimumWidth: 64
                Layout.alignment: Qt.AlignRight
                color: promptColor
                text: qsTr("New password:")
            }

            PasswordBox {
                id: pwd1Input
                implicitWidth: 150
                maximumLength: 64
                KeyNavigation.down: pwd2Input
                KeyNavigation.tab: pwd2Input
                Keys.onEnterPressed:pwd2Input.forceActiveFocus()
                Keys.onReturnPressed: pwd2Input.forceActiveFocus()
                onAccepted: {
                    if(pwd1Input.text != "" && pwd2Input.text != "" )
                        validatePwds()
                }
            }

            // shows message (prompt) from pam conv()
            Text {
                id: prompt2Txt
                Layout.minimumWidth: 64
                Layout.alignment: Qt.AlignRight
                color: promptColor
                text: qsTr("Repeat password:")
            }

            PasswordBox {
                id: pwd2Input
                implicitWidth: 150
                maximumLength: 64
                KeyNavigation.down: cancelBtn
                KeyNavigation.tab: cancelBtn
                Keys.onEnterPressed: if(pwd1Input == "") pwd1Input.forceActiveFocus()
                Keys.onReturnPressed: if(pwd1Input == "") pwd1Input.forceActiveFocus()
                onAccepted: validatePwds()
            }

            RowLayout {

                Layout.columnSpan: 2
                Layout.bottomMargin: 32
                Layout.topMargin: margins
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                spacing: 8

                Button {
                    id: cancelBtn
                    text: qsTr("Cancel")
                    activeFocusOnTab : true
                    Keys.onEnterPressed: clicked()
                    Keys.onReturnPressed: clicked()
                    KeyNavigation.right: resetBtn
                    onClicked: {
                        container.visible = false
                        resetEmptyWarning()
                        cancel()
                    }
                }

                Button {
                    id: resetBtn
                    text: qsTr("Reset")
                    activeFocusOnTab : true
                    enabled: pwd1Input.text != "" || pwd2Input.text != ""
                    Keys.onEnterPressed: clicked()
                    Keys.onReturnPressed: clicked()
                    KeyNavigation.right: okBtn
                    KeyNavigation.left: cancelBtn
                    KeyNavigation.up: pwd2Input
                    onClicked: {
                        pwd1Input.text = ""
                        pwd2Input.text = ""
                        resetEmptyWarning()
                        pwd1Input.forceActiveFocus()
                    }
                }

                Button {
                    id: okBtn
                    text: qsTr("Ok")
                    activeFocusOnTab : true
                    enabled: pwd1Input.text != "" && pwd2Input.text != ""
                    Keys.onEnterPressed: clicked()
                    Keys.onReturnPressed: clicked()
                    KeyNavigation.left: resetBtn
                    KeyNavigation.tab: pwd1Input
                    KeyNavigation.up: pwd2Input
                    onClicked: validatePwds()
                }
            }
        }
    }
}
