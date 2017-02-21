/*
 * PAM authentication backend
 * Copyright (C) 2013 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "PamBackend.h"
#include "PamHandle.h"
#include "HelperApp.h"
#include "UserSession.h"
#include "Auth.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

#include <stdlib.h>

namespace SDDM {
    static Request loginRequest {
        {   { AuthPrompt::LOGIN_USER, QStringLiteral("login:"), false },
            { AuthPrompt::LOGIN_PASSWORD, QStringLiteral("Password: "), true }
        }
    };

    static Request changePassRequest {
        {   { AuthPrompt::CHANGE_CURRENT, QStringLiteral("(current) UNIX password: "), true },
            { AuthPrompt::CHANGE_NEW, QStringLiteral("New password: "), true },
            { AuthPrompt::CHANGE_REPEAT, QStringLiteral("Retype new password: "), true }
        }
    };

    static Request changePassNoOldRequest {
        {   { AuthPrompt::CHANGE_NEW, QStringLiteral("New password: "), true },
            { AuthPrompt::CHANGE_REPEAT, QStringLiteral("Retype new password: "), true }
        }
    };

    static Request invalidRequest { {} };

    static Prompt invalidPrompt {};

    PamData::PamData() { }

    AuthPrompt::Type PamData::detectPrompt(const struct pam_message* msg) const {
        if (msg->msg_style == PAM_PROMPT_ECHO_OFF) {
            QString message = QString::fromLocal8Bit(msg->msg);
            if (message.indexOf(QRegExp(QStringLiteral("\\bpassword\\b"), Qt::CaseInsensitive)) >= 0) {
                if (message.indexOf(QRegExp(QStringLiteral("\\b(re-?(enter|type)|again|confirm|repeat)\\b"), Qt::CaseInsensitive)) >= 0) {
                    return AuthPrompt::CHANGE_REPEAT;
                }
                else if (message.indexOf(QRegExp(QStringLiteral("\\bnew\\b"), Qt::CaseInsensitive)) >= 0) {
                    return AuthPrompt::CHANGE_NEW;
                }
                else if (message.indexOf(QRegExp(QStringLiteral("\\b(old|current)\\b"), Qt::CaseInsensitive)) >= 0) {
                    return AuthPrompt::CHANGE_CURRENT;
                }
                else {
                    return AuthPrompt::LOGIN_PASSWORD;
                }
            }
        }
        else {
            return AuthPrompt::LOGIN_USER;
        }

        return AuthPrompt::UNKNOWN;
    }

    const Prompt& PamData::findPrompt(const struct pam_message* msg) const {
        AuthPrompt::Type type = detectPrompt(msg);

        for (const Prompt &p : m_currentRequest.prompts) {
            if (type == p.type && p.message == QString::fromLocal8Bit(msg->msg))
                return p;
        }

        return invalidPrompt;
    }

    Prompt& PamData::findPrompt(const struct pam_message* msg) {
        AuthPrompt::Type type = detectPrompt(msg);

        for (Prompt &p : m_currentRequest.prompts) {
            if (type == AuthPrompt::UNKNOWN && QString::fromLocal8Bit(msg->msg) == p.message)
                return p;
            if (type == p.type)
                return p;
        }

        return invalidPrompt;
    }

    /*
    * Expects an empty prompt list if the previous request has been processed
    *
    * @return true if new prompt was inserted or prompt message was set
    * and false if prompt was found and already sent
    */
    bool PamData::insertPrompt(const struct pam_message* msg, bool predict) {
        Prompt &p = findPrompt(msg);

        // first, check if we already have stored this prompt
        if (p.valid()) {
            // we have a response already - do nothing
            if (m_sent)
                return false;
            // we don't have a response yet - replace the message and prepare to send it
            p.message = QString::fromLocal8Bit(msg->msg);
            return true;
        }
        // this prompt is not stored but we have some prompts
        else if (m_currentRequest.prompts.length() != 0) {
            // check if we have already sent this - if we did, get rid of the answers
            if (m_sent) {
                m_currentRequest.clear();
                m_sent = false;
            }
        }

        // we'll predict what will come next
        if (predict) {
            AuthPrompt::Type type = detectPrompt(msg);
            switch (type) {
                case AuthPrompt::LOGIN_USER:
                    m_currentRequest = Request(loginRequest);
                    return true;
                case AuthPrompt::CHANGE_CURRENT:
                    m_currentRequest = Request(changePassRequest);
                    return true;
                case AuthPrompt::CHANGE_NEW:
                    m_currentRequest = Request(changePassNoOldRequest);
                    return true;
                default:
                    break;
            }
        }

        // or just add whatever comes exactly as it comes
        m_currentRequest.prompts.append(Prompt(detectPrompt(msg), QString::fromLocal8Bit(msg->msg), msg->msg_style == PAM_PROMPT_ECHO_OFF));

        return true;
    }

    /*
     * Distinguish types of pam conversation infos.
     *
     * Currently different info types are not really used (see \ref Display::slotAuthInfo which
     * just fires one signal for all info types). Just builds new request for "changing" keyword.
     */
    Auth::Info PamData::handleInfo(const struct pam_message* msg, bool &newRequest, bool predict) {
        if (QString::fromLocal8Bit(msg->msg).indexOf(QRegExp(QStringLiteral("^Changing password for [^ ]+$"))) >= 0)
        {
            if (predict) {
                m_currentRequest = Request(changePassRequest);
                newRequest = true;
                m_sent = false;
            }
            return Auth::INFO_PASS_CHANGE_REQUIRED;
        }
        return Auth::INFO_PAM_CONV;
    }

    /* Figure out if pam (chAuthTok) sent an error or just an info during conversation
     *
     * Real error message e.g. "Password change aborted." But most are just info messages
     * e.g. "change your password ... (administrator enforced)" with type PAM_ERROR_MSG.
     */
    Auth::Error PamData::handleErr(const struct pam_message* msg, bool &newRequest, bool predict) {
        // bad password during password renewal (for expired password)
        if (QString::fromLocal8Bit(msg->msg).startsWith(QStringLiteral("BAD PASSWORD:")))
        {
            if (predict)
            {
                // prepare new (empty) request, new password prompt will follow
                m_currentRequest = Request();
                newRequest = false;
                m_sent = false;
            }
            return Auth::ERROR_NONE;
        }

        if(QString::fromLocal8Bit(msg->msg).startsWith(QStringLiteral("Password change aborted."))) {
            return Auth::ERROR_AUTHENTICATION;
        }

        return Auth::ERROR_NONE;
    }

    /*
    * Destroys the prompt with that response
    */
    QByteArray PamData::getResponse(const struct pam_message* msg) {
        QByteArray response = findPrompt(msg).response;
        m_currentRequest.prompts.removeOne(findPrompt(msg));
        if (m_currentRequest.prompts.length() == 0)
            m_sent = false;
        return response;
    }

    /* As long as m_currentRequest is not sent (m_sent is false) return current request */
    const Request& PamData::getRequest() const {
        if (!m_sent)
            return m_currentRequest;
        else
            return invalidRequest;
    }

    /* Use new request if prompts are equal to current one and set sent true. */
    void PamData::completeRequest(const Request& request) {
        if (request.prompts.length() != m_currentRequest.prompts.length()) {
            qWarning() << "[PAM] Different request/response list length, ignoring";
            return;
        }

        for (int i = 0; i < request.prompts.length(); i++) {
            if (request.prompts[i].type != m_currentRequest.prompts[i].type
                || request.prompts[i].message != m_currentRequest.prompts[i].message
                || request.prompts[i].hidden != m_currentRequest.prompts[i].hidden) {
                qWarning() << "[PAM] Order or type of the messages doesn't match, ignoring";
                return;
            }
        }

        m_currentRequest = request;
        m_sent = true;
    }




    PamBackend::PamBackend(HelperApp *parent)
            : Backend(parent)
            , m_data(new PamData())
            , m_pam(new PamHandle(this)) {
    }

    PamBackend::~PamBackend() {
        delete m_data;
        delete m_pam;
    }

    bool PamBackend::start(const QString &user) {
        bool result;

        QString service = QStringLiteral("sddm");

        if (user == QStringLiteral("sddm") && m_greeter)
            service = QStringLiteral("sddm-greeter");
        else if (m_autologin)
            service = QStringLiteral("sddm-autologin");
        result = m_pam->start(service, user);

        if (!result)
            m_app->error(m_pam->errorString(), Auth::ERROR_INTERNAL);

        return result;
    }

    bool PamBackend::authenticate() {
        m_convCanceled = false; // reset for converse()
        if (!m_pam->authenticate()) {
            m_app->error(m_pam->errorString(), Auth::ERROR_AUTHENTICATION);
            return false;
        }
        if (!m_pam->acctMgmt()) {
            m_app->error(m_pam->errorString(), Auth::ERROR_AUTHENTICATION);
            return false;
        }
        return true;
    }

    bool PamBackend::openSession() {
        if (!m_pam->setCred(PAM_ESTABLISH_CRED)) {
            m_app->error(m_pam->errorString(), Auth::ERROR_AUTHENTICATION);
            return false;
        }

        QProcessEnvironment sessionEnv = m_app->session()->processEnvironment();
        if (sessionEnv.value(QStringLiteral("XDG_SESSION_TYPE")) == QLatin1String("x11")) {
            QString display = sessionEnv.value(QStringLiteral("DISPLAY"));
            if (!display.isEmpty()) {
#ifdef PAM_XDISPLAY
                m_pam->setItem(PAM_XDISPLAY, qPrintable(display));
#endif
                m_pam->setItem(PAM_TTY, qPrintable(display));
            }
        } else if (sessionEnv.value(QStringLiteral("XDG_SESSION_TYPE")) == QLatin1String("wayland")) {
            QString tty = QStringLiteral("/dev/tty%1").arg(sessionEnv.value(QStringLiteral("XDG_VTNR")));
            m_pam->setItem(PAM_TTY, qPrintable(tty));
        }

        if (!m_pam->putEnv(sessionEnv)) {
            m_app->error(m_pam->errorString(), Auth::ERROR_INTERNAL);
            return false;
        }
        if (!m_pam->openSession()) {
            m_app->error(m_pam->errorString(), Auth::ERROR_INTERNAL);
            return false;
        }
        sessionEnv.insert(m_pam->getEnv());
        m_app->session()->setProcessEnvironment(sessionEnv);
        return Backend::openSession();
    }

    bool PamBackend::closeSession() {
        if (m_pam->isOpen()) {
            qDebug() << "[PAM] Closing session";
            m_pam->closeSession();
            m_pam->setCred(PAM_DELETE_CRED);
            return true;
        }
        qWarning() << "[PAM] Asked to close the session but it wasn't previously open";
        return Backend::closeSession();
    }

    QString PamBackend::userName() {
        return QString::fromLocal8Bit((const char*) m_pam->getItem(PAM_USER));
    }

    /** @internal for debug log: get string representation of pam message msg_style */
    const QString &PamBackend::msgStyleString(int msg_style)
    {
        static const QString msgStyle[] = {
            QStringLiteral("PAM_PROMPT_ECHO_OFF"),
            QStringLiteral("PAM_PROMPT_ECHO_ON"),
            QStringLiteral("PAM_ERROR_MSG"),
            QStringLiteral("PAM_TEXT_INFO"),
            QStringLiteral("UNKNOWN"),
        };

        switch(msg_style) {
            case PAM_PROMPT_ECHO_OFF:
                return msgStyle[0]; break;
            case PAM_PROMPT_ECHO_ON:
                return msgStyle[1]; break;
            case PAM_ERROR_MSG:
                return msgStyle[2]; break;
            case PAM_TEXT_INFO:
                return msgStyle[3]; break;
            default: break;
        }

        return msgStyle[4];
    }

    int PamBackend::converse(int n, const struct pam_message **msg, struct pam_response **resp) {
        qDebug() << "[PAM] Conversation with" << n << "messages";

        bool newRequest = false;

        if (n <= 0 || n > PAM_MAX_NUM_MSG)
            return PAM_CONV_ERR;

        // see whats going on in pam_conv
        for (int i = 0; i < n; i++) {
            qDebug() << "[PAM] pam_conv: style=" << msgStyleString(msg[i]->msg_style)
                            << "(" << msg[i]->msg_style << "), msg[" << i << "]=" << QString::fromLocal8Bit(msg[i]->msg);
        }

        for (int i = 0; i < n; i++) {

            QString convMsg = QString::fromLocal8Bit(msg[i]->msg);
            Auth::Error convErr;

            switch(msg[i]->msg_style) {
                case PAM_PROMPT_ECHO_OFF:
                case PAM_PROMPT_ECHO_ON:
                    newRequest = m_data->insertPrompt(msg[i], n == 1);
                    break;
                case PAM_ERROR_MSG:
                    convErr = m_data->handleErr(msg[i], newRequest, n == 1);
                    // upon pwd change pam sends error message and then starts conversation,
                    // so treat most of them actually as infos
                    if(convErr==Auth::ERROR_NONE)
                        m_app->info(convMsg, Auth::INFO_PAM_CONV);
                    else {
                        qDebug() << "[PAM] PamBackend: pam error message, msg=" << convMsg;
                        m_app->error(convMsg, Auth::ERROR_AUTHENTICATION /* convErr */);
                        return PAM_CONV_ERR;
                    }
                    break;
                case PAM_TEXT_INFO:
                    // send pam conversation msg to greeter via HelperApp, SocketServer, Display
                    // if there's only the info message, let's predict the prompts too
                    m_app->info(convMsg, m_data->handleInfo(msg[i], newRequest, n == 1));
                    break;
                default:
                    break;
            }
        }

        // pam (chauthtok) does not shut up after canceled with PAM_CONV_ERR in previous pam_conv
        if(m_convCanceled) {
            qDebug() << "[PAM] PamBackend: conversation canceled, dump trailing messages";
            return PAM_CONV_ERR;
        }

        if (newRequest) {
            // get current request
            Request send = m_data->getRequest();
            Request received;

            // any prompt?
            if (send.valid()) {
                // send request to daemon (ask for a response)
                received = m_app->request(send, m_convCanceled);

                // password input canceled in greeter
                if (m_convCanceled) {
                    return PAM_CONV_ERR;
                }

                if (!received.valid())
                    return PAM_CONV_ERR;

                // compare request with response
                m_data->completeRequest(received);
            }
        }

        *resp = (struct pam_response *) calloc(n, sizeof(struct pam_response));
        if (!*resp) {
            return PAM_BUF_ERR;
        }

        for (int i = 0; i < n; i++) {
            QByteArray response = m_data->getResponse(msg[i]);

            resp[i]->resp = (char *) malloc(response.length() + 1);
            // on error, get rid of everything
            if (!resp[i]->resp) {
                for (int j = 0; j < n; j++) {
                    free(resp[i]->resp);
                    resp[i]->resp = nullptr;
                }
                free(*resp);
                *resp = nullptr;
                return PAM_BUF_ERR;
            }

            memcpy(resp[i]->resp, response.constData(), response.length());
            resp[i]->resp[response.length()] = '\0';
        }

        return PAM_SUCCESS;
    }
}
