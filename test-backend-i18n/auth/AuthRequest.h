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

#ifndef REQUEST_H
#define REQUEST_H

#include <QtCore/QObject>

#include <AuthPrompt.h>

namespace SDDM {

    class Request;
    /**
    * \brief
    * AuthRequest is the main class for tracking requests from the underlying auth stack
    *
    * \section description
    * Typically, when logging in, you'll receive a list containing one or two fields:
    *
    *  * First one for the username (if you didn't provide it before);
    *    hidden = false, type = LOGIN_USER, message = whatever the stack provides
    *
    *  * Second one for the user's password
    *    hidden = true, type = LOGIN_PASSWORD, message = whatever the stack provides
    *
    * It's up to you to fill the \ref AuthPrompt::response property.
    * When all the fields are filled to your satisfaction, just trigger the \ref done
    * slot and the response will go back to the authenticator.
    *
    * \todo Decide if it's sane to use the info messages from PAM or to somehow parse them
    * and make the password changing message into a Request::Type of some kind
    *
    *  \note AuthRequest (and AuthPrompt) is mainly used in (Pam)Backend
     * and closely bundled with the Auth.cpp parent which handles user session
     * after authentication etc.
     *
     * But AuthRequest is also secondly used as pure (pam_conv) message container in greeter
     * without the Auth parent overhead (but still with the signal/slot amenities) - depending on
     * the constructor used. In this case it just hands over the pam_conv() messages and possibly
     * user (password) responses between daemon, greeter and qml.
    */
    class AuthRequest : public QObject {
        Q_OBJECT

    public:
        /** @brief for AuthRequest without using Auth parent.
         *
         * when used as parameter container to hand over pam request (prompts with pam
         * messages and user pwd responses) between daemon and greeter (proxy) to qml view.
         */
        AuthRequest(QObject *parent = 0);
        /**
        * @return list of the contained prompts
        */
        QList<AuthPrompt*> prompts();

        /**
         * @brief Find pam message of type CHANGE_NEW in prompts list
         * @return pam message string
         */
        QString findChangePwdMessage();
        /**
          * @brief Write responses into request for pam conv(),
          * for AuthPrompt::LOGIN_USER, AuthPrompt::LOGIN_PASSWORD
          */
        void setLoginResponse(const QString &username, const QString &password);
        void setChangeResponse(const QString &password);

        static AuthRequest *empty();

        void setRequest(const Request * const request = nullptr);
        /**
         * @brief convert AuthRequest to simple Request type
         * @return Request with Prompts
         */
        Request request() const;
        /**
         * @brief find prompt with specified type
         * @param type \ref AuthPrompt::Type
         * @return pointer to prompt of that type
         */
        AuthPrompt *findPrompt(AuthPrompt::Type type) const;

    protected:
        class Private;
        Private *d { nullptr };
    };
}

#endif //REQUEST_H
