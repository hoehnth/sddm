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

#include "Auth.h"

#include <QDebug>

namespace SDDM {

    class Auth::Private : public QObject {
        Q_OBJECT
    public:
        Private(Auth *parent);
    public:
        QProcessEnvironment environment { };
    };

    Auth::Private::Private(Auth *parent)
            : QObject(parent) {
        //QProcessEnvironment env;
        //env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
    }

    Auth::Auth(QObject *parent)
            : QObject(parent)
            , d(new Private(this)) {
    }

    Auth::~Auth() {
        delete d;
    }

    void Auth::insertEnvironment(const QProcessEnvironment &env) {
        qDebug() << "Auth: insertEnvironment";
        d->environment.insert(env);
    }

    void Auth::insertEnvironment(const QString &key, const QString &value) {
        qDebug() << "Auth: insertEnvironment, key =" << key << ", value =" << value;
        d->environment.insert(key, value);
    }
}

#include "Auth.moc"
