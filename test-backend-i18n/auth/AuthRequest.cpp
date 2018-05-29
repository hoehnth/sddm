/*
 * Qt Authentication Library
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

#include "AuthBase.h"
#include "AuthRequest.h"

namespace SDDM {
    class AuthRequest::Private : public QObject {
        Q_OBJECT
    public:
        Private(QObject *parent);
        QList<AuthPrompt*> prompts { };
    };

    AuthRequest::Private::Private(QObject* parent)
            : QObject(parent) { }

    AuthRequest::AuthRequest(QObject *parent)
            : QObject(parent)
            , d(new Private(this)) { }

    void AuthRequest::setRequest(const Request * const request) {
        QList<AuthPrompt*> promptsCopy(d->prompts);
        d->prompts.clear();
        if (request != nullptr) {
            Q_FOREACH (const Prompt& p, request->prompts) {
                AuthPrompt *qap = new AuthPrompt(&p, this);
                d->prompts << qap;
            }
        }
        if (request == nullptr) {
            qDeleteAll(promptsCopy);
        }
    }

    QList<AuthPrompt*> AuthRequest::prompts() {
        return d->prompts;
    }

    Request AuthRequest::request() const {
        Request r;
        Q_FOREACH (const AuthPrompt* qap, d->prompts) {
            Prompt p;
            p.hidden = qap->hidden();
            p.message = qap->message();
            p.response = qap->response();
            p.type = qap->type();
            r.prompts << p;
        }
        return r;
    }

    QString AuthRequest::findChangePwdMessage() {
        Q_FOREACH(const AuthPrompt* qap, d->prompts) {
            if(qap->type()==AuthPrompt::CHANGE_PASSWORD)
                return qap->message();
        }
        return QString();
    }

    AuthPrompt *AuthRequest::findPrompt(AuthPrompt::Type type) const {
        Q_FOREACH(AuthPrompt* qap, d->prompts) {
            if(qap->type()==type)
                return qap;
        }
        return NULL;
    }

    void AuthRequest::setLoginResponse(const QString &username, const QString &password) {
        AuthPrompt* prompt;

        if(!username.isNull()) {
            prompt = findPrompt(AuthPrompt::LOGIN_USER);
            if(prompt) prompt->setResponse(qPrintable(username));
        }
        if(!password.isNull()) {
            prompt = findPrompt(AuthPrompt::LOGIN_PASSWORD);
            if(prompt) prompt->setResponse(qPrintable(password));
        }
    }

    void AuthRequest::setChangeResponse(const QString &password) {
        AuthPrompt* prompt;

        if(!password.isNull()) {
            prompt = findPrompt(AuthPrompt::CHANGE_PASSWORD);
            if(prompt) prompt->setResponse(qPrintable(password));
        }
    }
}

#include "AuthRequest.moc"
