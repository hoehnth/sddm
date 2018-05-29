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

#ifndef HELPERAPP_H
#define HELPERAPP_H

#define SDDM_BACKEND

#include "AuthBase.h"
#include <Auth.h>

#include <QtCore/QProcessEnvironment>

namespace SDDM {
    class Request;
    class Backend;
    class UserSession;

    class HelperApp : public QObject
    {
        Q_OBJECT
    public:
        HelperApp(QString &user, QString &pwd, bool autologin = false, bool maxtries_loop = false, bool cancel = false);

        int doAuth();
        UserSession *session();
        const QString &user() const;

    public slots:
        Request request(const Request &request, bool cancel);
        void info(const QString &message, AuthEnums::Info type, int result);
        void error(const QString &message, AuthEnums::Error type, int result);
        void authenticated(const QString &user);

    private:
        Backend *m_backend { nullptr };
        UserSession *m_session { nullptr };
        QString m_user { };
        QString m_pwd { };
        bool m_cancel { false };
    };
}

#endif // HELPERAPP_H
