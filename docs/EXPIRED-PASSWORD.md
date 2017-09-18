## Introduction

SDDM can handle expired passwords during login.
A password renewal dialog is provided to change the expired password.
The greeter frontend will talk with the backend to handle the pam conversation.
Support for password renewal is provided with two components used in greeter themes:

* ``PasswordRenewal.qml`` basic dialog for password renewal, for password input and confirmation
* ``PasswordConnections.qml`` container for password renewal logic, hides signal handling

## Usage

For new themes the two components have to be included in the ``Main.qml``.
See the built-in greeter theme ``Main.qml`` for an example,
and add this to  ``Main.qml``:

```
// container with password renewal logic
PasswordConnections {
    sddmProxy: sddm // fix assignment
    requestData: request // fix assignment
    renewalDialog: renewal // PasswordRenewal dialog id (see below)
    disabledItems: [usersContainer] // block these items during password renewal
    pwdItem: listView.currentItem // gets password input from currentItem.password
    getsBackFocus: listView // item where focus falls back after dialog closes
    errMsg: errMessage // if set defines (text) item which shows errors
    txtMsg: txtMessage // if set (text) item which shows pam infos
}

...

            // password renewal dialog
            PasswordRenewal {
                id: renewal
                // customize here e.g.:
                //anchors.horizontalCenter: parent.horizontalCenter
                //anchors.bottom: usersContainer.top
                //visible: false
                //color: "#22888888"
                //promptColor: "white"
                //infosColor: "lightcoral"
            }
```

## Some more details

The following qml objects are available in ``Main.qml``,
they are used for password renewal and pam conversation:

### Signals

Signals coming from daemon backend:

* ``pamConvMsg(pam_msg)``
Provides infos/errors from (pam) backend conversation to present to user.

* ``pamRequest()``
New request from (pam) backend, user response is required.

### Property

* ``request`` (type AuthRequest)
Prompts from (pam) backend with messages from pam_conv.

### Response

Responses from greeter frontend to the backend:

* ``sddm.enablePwdRenewal()``
The theme tells the greeter it can handle password renewal (has a dialog and logic as described above).
For themes which do not have a password renewal dialog, this method is not called.
In that case the pam conversation is just canceled (otherwise pam_conv user session sits there waiting for password response).
This will keep compatibility to (older) themes which do not support expired passwords yet.

* ``sddm.pamResponse(password)``
Send password response to (pam) backend, i.e. pam_conv.

* ``sddm.cancelPamConv()``
Cancel pam conversation with pam_conv.
