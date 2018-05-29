/*
 * Qt Authentication library
 * Copyright (C) 2013 Martin Bříza <mbriza@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef SDDM_AUTH_H
#define SDDM_AUTH_H

#include "AuthEnums.h"
#include "AuthPrompt.h"
#include "AuthRequest.h"

#include <QtCore/QObject>
#include <QtCore/QProcessEnvironment>

namespace SDDM {
    /**
    * \brief
    * Main class triggering the authentication and handling all communication
    *
    * \section description
    * There are three basic kinds of authentication:
    *
    *  * Checking only the validity of the user's secrets - The default values
    *
    *  * Logging the user in after authenticating him - You'll have to set the
    *      \ref session property to do that.
    *
    *  * Logging the user in without authenticating - You'll have to set the
    *      \ref session and \ref autologin properties to do that.
    *
    * Usage:
    *
    * Just construct, connect the signals (especially \ref requestChanged)
    * and fire up \ref start
    */
    class Auth : public QObject {
        Q_OBJECT
    public:
        explicit Auth(QObject *parent = 0);
        ~Auth();

        /**
        * If starting a session, you will probably want to provide some basic env variables for the session.
        * This only inserts the variables - if the current key already had a value, it will be overwritten.
        * User-specific data such as $HOME is generated automatically.
        * @param env the environment
        */
        void insertEnvironment(const QProcessEnvironment &env);

        /**
        * Works the same as \ref insertEnvironment but only for one key-value pair
        * @param key key
        * @param value value
        */
        void insertEnvironment(const QString &key, const QString &value);

    private:
        class Private;
        friend Private;
        Private *d { nullptr };
    };
}

#endif // SDDM_AUTH_H
