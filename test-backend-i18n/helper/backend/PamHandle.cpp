/*
 * PAM API Qt wrapper
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
#include "PamHandle.h"
#include "PamBackend.h"
#include "PamWorkState.h"
#include "Utils.h"

#include <QtCore/QDebug>

namespace SDDM {
    bool PamHandle::putEnv(const QProcessEnvironment& env) {
        qDebug() << "[PAM] putEnv()";
        foreach (const QString& s, env.toStringList()) {
            m_result = pam_putenv(m_handle, qPrintable(s));
            if (m_result != PAM_SUCCESS) {
                qWarning() << "[PAM] putEnv:" << pam_strerror(m_handle, m_result);
                return false;
            }
        }
        return true;
    }

    QProcessEnvironment PamHandle::getEnv() {
        QProcessEnvironment env;
        qDebug() << "[PAM] getEnv()";
        // get pam environment
        char **envlist = pam_getenvlist(m_handle);
        LOG_PAM_PTR_RC(pam_getenvlist, envlist);
        if (envlist == NULL) {
            qWarning() << "[PAM] getEnv: Returned NULL";
            return env;
        }

        // copy it to the env map
        for (int i = 0; envlist[i] != nullptr; ++i) {
            QString s = QString::fromLocal8Bit(envlist[i]);

            // find equal sign
            int index = s.indexOf(QLatin1Char('='));

            // add to the hash
            if (index != -1)
                env.insert(s.left(index), s.mid(index + 1));

            free(envlist[i]);
        }
        free(envlist);
        return env;
    }

    /** return codes:
     *
     * - PAM_AUTHTOK_ERR
     * - PAM_AUTHTOK_RECOVERY_ERR
     * - PAM_AUTHTOK_LOCK_BUSY
     * - PAM_AUTHTOK_DISABLE_AGING
     * - PAM_PERM_DENIED
     * - PAM_TRY_AGAIN
     * - PAM_MAXTRIES
     * - PAM_USER_UNKNOWN
     * - PAM_SUCCESS
     */
    bool PamHandle::chAuthTok(int flags) {
        LOG_WORK_STATE(pam_chauthtok, m_workState.value);
        do {
            m_result = pam_chauthtok(m_handle, flags | m_silent);
            LOG_PAM_RC(pam_chauthtok, m_result);
            if (m_result != PAM_SUCCESS) {
                QString errmsg(pam_strerror(m_handle, m_result));
                qWarning() << "[PAM] chAuthTok:" << errmsg;
                if(m_result == PAM_MAXTRIES && m_maxtries_loop) {
                    /* tell greeter about PAM_MAXTRIES result, or user might be
                     * puzzled about multi (current password) question rounds */
                    emit error(errmsg, AuthEnums::Error::ERROR_PAM_CONV, m_result); //m_app->error
                }
            }
        /* ignore PAM_MAXTRIES result and loop chauthtok? */
        } while(m_maxtries_loop && m_result == PAM_MAXTRIES);

        LOG_WORK_STATE(pam_chauthtok, m_workState.value);
        return m_result == PAM_SUCCESS;
    }

    bool PamHandle::acctMgmt(int flags) {
        LOG_WORK_STATE(pam_acct_mgmt, m_workState.value);
        m_result = pam_acct_mgmt(m_handle, flags | m_silent);
        LOG_PAM_RC(pam_acct_mgmt, m_result);
        if (m_result == PAM_NEW_AUTHTOK_REQD) {
            m_workState.value = PamWorkState::STATE_CHANGEAUTHTOK;
            LOG_WORK_STATE(pam_acct_mgmt, m_workState.value);
            return chAuthTok(PAM_CHANGE_EXPIRED_AUTHTOK);
        }
        else if (m_result != PAM_SUCCESS) {
            qWarning() << "[PAM] acctMgmt:" << pam_strerror(m_handle, m_result);
            return false;
        }
        return true;
    }

    /** return codes:
     *
     * - PAM_ABORT
     * - PAM_AUTH_ERR
     * - PAM_CRED_INSUFFICIENT
     * - PAM_AUTHINFO_UNVAIL
     * - PAM_MAXTRIES
     * - PAM_USER_UNKNOWN
     * - PAM_SUCCESS
     */
    bool PamHandle::authenticate(int flags) {
        qDebug() << "[PAM] Authenticating...";
        if(m_workState.value == PamWorkState::STATE_STARTED)
            m_workState.value = PamWorkState::STATE_AUTHENTICATE;
        LOG_WORK_STATE(pam_authenticate, m_workState.value);
        m_result = pam_authenticate(m_handle, flags | m_silent);
        LOG_PAM_RC(pam_authenticate, m_result);
        if (m_result == PAM_SUCCESS)
            m_workState.value = PamWorkState::STATE_AUTHENTICATED;
        else
            qWarning() << "[PAM] authenticate:" << pam_strerror(m_handle, m_result);
        LOG_WORK_STATE(pam_authenticate, m_workState.value);
        qDebug() << "[PAM] returning.";
        return m_result == PAM_SUCCESS;
    }

    /** return codes:
     *
     * - PAM_BUF_ERR
     * - PAM_CRED_ERR
     * - PAM_CRED_EXPIRED
     * - PAM_CRED_UNAVAIL
     * - PAM_SYSTEM_ERR
     * - PAM_USER_UNKNOWN
     * - PAM_SUCCESS
     */
    bool PamHandle::setCred(int flags) {
        m_result = pam_setcred(m_handle, flags | m_silent);
        LOG_PAM_RC(pam_setcred, m_result);
        if (m_result == PAM_SUCCESS) {
            if(m_workState.value == PamWorkState::STATE_AUTHENTICATED)
                m_workState.value = PamWorkState::STATE_CREDITED;
        } else
            qWarning() << "[PAM] setCred:" << pam_strerror(m_handle, m_result);

        return m_result == PAM_SUCCESS;
    }

    bool PamHandle::openSession() {
        m_result = pam_open_session(m_handle, m_silent);
        LOG_PAM_RC(pam_open_session, m_result);
        if (m_result == PAM_SUCCESS) {
            if(m_workState.value == PamWorkState::STATE_CREDITED)
                m_workState.value = PamWorkState::STATE_SESSION_STARTED;
        } else
            qWarning() << "[PAM] openSession:" << pam_strerror(m_handle, m_result);
        m_open = m_result == PAM_SUCCESS;
        return m_open;
    }

    bool PamHandle::closeSession() {
        m_result = pam_close_session(m_handle, m_silent);
        LOG_PAM_RC(pam_close_session, m_result);
        if (m_result != PAM_SUCCESS) {
            qWarning() << "[PAM] closeSession:" << pam_strerror(m_handle, m_result);
        }
        m_workState.value = PamWorkState::STATE_FINISHED;
        return m_result == PAM_SUCCESS;
    }

    bool PamHandle::isOpen() const {
        return m_open;
    }

    bool PamHandle::setItem(int item_type, const void* item) {
        m_result = pam_set_item(m_handle, item_type, item);
        LOG_PAM_ITEM(pam_get_item, item_type, m_result);
        if (m_result != PAM_SUCCESS) {
            qWarning() << "[PAM] setItem:" << pam_strerror(m_handle, m_result);
        }
        return m_result == PAM_SUCCESS;
    }

    const void* PamHandle::getItem(int item_type) {
        const void *item;
        m_result = pam_get_item(m_handle, item_type, &item);
        LOG_PAM_ITEM(pam_get_item, item_type, m_result);
        if (m_result != PAM_SUCCESS) {
            qWarning() << "[PAM] getItem:" << pam_strerror(m_handle, m_result);
        }
        return item;
    }

    /** return codes:
     *
     * - PAM_BUF_ERR
     * - PAM_CONV_ERR
     * - PAM_SUCCESS
     */
    int PamHandle::converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data) {
        qDebug() << "[PAM] Preparing to converse...";
        PamBackend *c = static_cast<PamBackend *>(data);
        return c->converse(n, msg, resp);
    }

    /** return codes:
     *
     * - PAM_ABORT
     * - PAM_BUF_ERR
     * - PAM_SYSTEM_ERR
     * - PAM_SUCCESS
     */
    bool PamHandle::start(const QString &service, const QString &user) {
        qDebug() << "[PAM] Starting...";
        qDebug() << "[PAM] pam_start( service =" << service << ", user =" << user << ")";
        if (user.isEmpty())
            m_result = pam_start(qPrintable(service), NULL, &m_conv, &m_handle);
        else
            m_result = pam_start(qPrintable(service), qPrintable(user), &m_conv, &m_handle);
        LOG_PAM_RC(pam_start, m_result);
        if (m_result == PAM_SUCCESS)
            m_workState.value = PamWorkState::STATE_STARTED;
        else {
            qWarning() << "[PAM] start" << pam_strerror(m_handle, m_result);
            return false;
        }
        return true;
    }

    bool PamHandle::end(int flags) {
        if (!m_handle)
            return false;
        m_result = pam_end(m_handle, m_result | flags);
        // pam finished, ignore rc
        m_workState.value = PamWorkState::STATE_FINISHED;
        LOG_PAM_RC(pam_end, m_result);
        if (m_result != PAM_SUCCESS) {
            qWarning() << "[PAM] end:" << pam_strerror(m_handle, m_result);
            return false;
        }
        else {
            qDebug() << "[PAM] Ended.";
        }
        m_handle = NULL;
        return true;
    }

    int PamHandle::getResult() {
        return m_result;
    }

    QString PamHandle::errorString() {
        return QString::fromLocal8Bit(pam_strerror(m_handle, m_result));
    }

    void PamHandle::setMaxTriesLoop(bool loop) {
        m_maxtries_loop = loop;
    }

    PamHandle::PamHandle(PamWorkState &ref, PamBackend *parent)
        : m_workState(ref) { // use pam work state from parent
        // create context
        m_conv = { &PamHandle::converse, parent };
    }

    PamHandle::~PamHandle() {
        // stop service
        end();
    }
}
