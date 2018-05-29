/*
 * Main authentication application class
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

#include <QtCore/QDebug>

#include "Backend.h"
#include "UserSession.h"
#include "HelperApp.h"
#include "Utils.h"

#include <iostream>

#include <locale.h>

// Helper macro for HelperApp::request to log faked request
#define LOG_REQUEST(type_str, prompt) \
    qDebug().noquote().nospace() \
        << prefix << " " << type_str << " request: \"" \
        << AuthPrompt::typeToString(prompt->type()) << "\""

// Helper macro for HelperApp::request to log faked response
#define LOG_RESPONSE(type_str, value) \
    qDebug().noquote().nospace() \
        << prefix << " Send response (" \
        << type_str << "): \"" << value << "\""

const static QString prefix = "[HelperApp]";

namespace SDDM {
    HelperApp::HelperApp(QString &user, QString &pwd, bool autologin, bool maxtries_loop, bool cancel) :
        m_backend(Backend::get(this)),
        m_session(new UserSession(this)),
        m_user(user), m_pwd(pwd), m_cancel(cancel) {
        m_backend->setAutologin(autologin);
        m_backend->setChAuthTokLoop(maxtries_loop);
    }

    int HelperApp::doAuth() {

        if (!m_backend->start(m_user)) {
            qDebug() << prefix << "Backend->start: failed";
            return AuthEnums::HELPER_AUTH_ERROR;
        }

        if (!m_backend->authenticate()) {
            qDebug().noquote() << prefix << "Backend->authenticate: failed";
            return AuthEnums::HELPER_AUTH_ERROR;
        }

        m_user = m_backend->userName();
        authenticated(m_user);

        qDebug().noquote() << prefix << "doAuth: \"success\"";
        return AuthEnums::HELPER_SUCCESS;
    }

    void HelperApp::info(const QString& message, AuthEnums::Info type, int result) {
        qInfo().noquote() << prefix << "info(): message =" << message
                          << ", type =" << type << ", rc =" << Utils::pamRcString(result);
    }

    void HelperApp::error(const QString& message, AuthEnums::Error type, int result) {
        qInfo().noquote() << prefix << "error(): message =" << message
                          << ", type =" << type << ", rc =" << Utils::pamRcString(result);
    }

    // answer request from (PAM) backend with fake response
    Request HelperApp::request(const Request &request, bool cancel) {

        // !!! TODO !!!
        if(m_cancel) {
            // fake password renewal canceled in greeter
            qDebug() << prefix << "CANCEL backend conversation request... !!! TODO !!!";
        }

        AuthRequest auth;
        AuthPrompt *prompt;
        std::string input;

        auth.setRequest(&request);

        // more likely
        if((prompt = auth.findPrompt(AuthPrompt::LOGIN_PASSWORD))) {
            LOG_REQUEST("Login", prompt);
            LOG_RESPONSE("password", m_pwd);
        }
        // less likely
        else if((prompt = auth.findPrompt(AuthPrompt::LOGIN_USER))) {
            LOG_REQUEST("Login", prompt);
            LOG_RESPONSE("user name", m_user);
        }

        if(prompt)
            auth.setLoginResponse(m_user, m_pwd);
        else
            if((prompt = auth.findPrompt(AuthPrompt::CHANGE_PASSWORD))) {
                LOG_REQUEST("Change", prompt);

                std::cout << request.prompts.at(0).message.toLocal8Bit().constData();
                std::cin >> input;

                auth.setChangeResponse(input.c_str());
        }

        // send (greeter) fake response to backend
        return auth.request();
    }

    void HelperApp::authenticated(const QString &user) {
        qInfo().noquote() << prefix << "authenticated:" << user;
    }

    UserSession *HelperApp::session() {
        return m_session;
    }

    const QString& HelperApp::user() const {
        return m_user;
    }
}

int main(int argc, char** argv) {

    int rc;

    qDebug() << "\nNote:" << argv[0] << "needs root rights to read /etc/shadow !!!\n";

    if(argc<3) {
        qWarning() << "Usage: PamAuthTest <user> <pwd>";
        return 1;
    }

    QString user(argv[1]);
    QString pwd(argv[2]);

    setlocale (LC_ALL, "");
    SDDM::HelperApp app(user, pwd, false /* autologin */, true /* maxtries_loop */);
    rc = app.doAuth();

    return rc;
}
