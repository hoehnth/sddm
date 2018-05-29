#include "Utils.h"

#include <QHash>
#include <QDebug>

#include <string.h>
#include <stdlib.h>

extern char **environ;

Utils::Utils(QObject *parent) : QObject(parent)
{
}

/** @internal get string representation of pam message msg_style, for debug logging */
const QString &Utils::msgStyleString(int msg_style)
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

/** @internal get string representation of pam work state for debug logging */
const QString &Utils::workStateString(SDDM::PamWorkState::State work_state)
{
    static const QString workState[] = {
        QStringLiteral("STATE_INITIAL"),
        QStringLiteral("STATE_STARTED"),
        QStringLiteral("STATE_AUTHENTICATE"),
        QStringLiteral("STATE_CHANGEAUTHTOK"),
        QStringLiteral("STATE_AUTHENTICATED"),
        QStringLiteral("STATE_AUTHORIZED"),
        QStringLiteral("STATE_CREDITED"),
        QStringLiteral("STATE_SESSION_STARTED"),
        QStringLiteral("STATE_FINISHED"),
        QStringLiteral("UNKNOWN")
    };

    switch(work_state) {
        case SDDM::PamWorkState::State::STATE_INITIAL:
            return workState[0]; break;
        case SDDM::PamWorkState::State::STATE_STARTED:
            return workState[1]; break;
        case SDDM::PamWorkState::State::STATE_AUTHENTICATE:
            return workState[2]; break;
        case SDDM::PamWorkState::State::STATE_CHANGEAUTHTOK:
            return workState[3]; break;
        case SDDM::PamWorkState::State::STATE_AUTHENTICATED:
            return workState[4]; break;
        case SDDM::PamWorkState::State::STATE_AUTHORIZED:
            return workState[5]; break;
        case SDDM::PamWorkState::State::STATE_CREDITED:
            return workState[6]; break;
        case SDDM::PamWorkState::State::STATE_SESSION_STARTED:
            return workState[7]; break;
        case SDDM::PamWorkState::State::STATE_FINISHED:
            return workState[8]; break;
        default: break;
    }

    return workState[9];
}

/** @internal get string representation of pam item, for debug logging */
const QString Utils::pamItemString(int item_type)
{
    static const QHash<int, QString> itemTypes(
    {
        { PAM_SERVICE,    QStringLiteral("PAM_SERVICE")},
        { PAM_USER,       QStringLiteral("PAM_USER")},
        { PAM_USER_PROMPT,QStringLiteral("PAM_USER_PROMPT")},
        { PAM_TTY,        QStringLiteral("PAM_TTY")},
        { PAM_RHOST,      QStringLiteral("PAM_RHOST")},
        { PAM_AUTHTOK,    QStringLiteral("PAM_AUTHTOK")},
        { PAM_OLDAUTHTOK, QStringLiteral("PAM_OLDAUTHTOK")},
        { PAM_CONV,       QStringLiteral("PAM_CONV")},
    });

    static const QString &unknown = QStringLiteral("PAM_UNKNOWN_TYPE");

    if(itemTypes.contains(item_type))
        return itemTypes[item_type];

    // not in our list
    return unknown;
}

/** @internal get string representation of pam status code (return codes), for debug logging */
const QString Utils::pamRcString(int rc)
{
    // https://stackoverflow.com/questions/6576036/initialise-global-key-value-hash
    static const QHash<int, QString> statusCodes(
    {
        { PAM_SUCCESS,              QStringLiteral("PAM_SUCCESS")},
        { PAM_BAD_ITEM,             QStringLiteral("PAM_BAD_ITEM")},
        { PAM_PERM_DENIED,          QStringLiteral("PAM_PERM_DENIED")},
        { PAM_SYSTEM_ERR,           QStringLiteral("PAM_SYSTEM_ERR")},
        { PAM_SYMBOL_ERR,           QStringLiteral("PAM_SYMBOL_ERR")},
        { PAM_SERVICE_ERR,          QStringLiteral("PAM_SERVICE_ERR")},
        { PAM_SYSTEM_ERR,           QStringLiteral("PAM_SYSTEM_ERR")},
        { PAM_BUF_ERR,              QStringLiteral("PAM_BUF_ERR")},
        { PAM_CONV_ERR,             QStringLiteral("PAM_CONV_ERR")},
        { PAM_PERM_DENIED,          QStringLiteral("PAM_PERM_DENIED")},
        { PAM_MAXTRIES,             QStringLiteral("PAM_MAXTRIES")},
        { PAM_AUTH_ERR,             QStringLiteral("PAM_AUTH_ERR")},
        { PAM_NEW_AUTHTOK_REQD,     QStringLiteral("PAM_NEW_AUTHTOK_REQD")},
        { PAM_CRED_INSUFFICIENT,    QStringLiteral("PAM_CRED_INSUFFICIENT")},
        { PAM_AUTHINFO_UNAVAIL,     QStringLiteral("PAM_AUTHINFO_UNAVAIL")},
        { PAM_USER_UNKNOWN,         QStringLiteral("PAM_USER_UNKNOWN")},
        { PAM_CRED_UNAVAIL,         QStringLiteral("PAM_CRED_UNAVAIL")},
        { PAM_CRED_EXPIRED,         QStringLiteral("PAM_CRED_EXPIRED")},
        { PAM_CRED_ERR,             QStringLiteral("PAM_CRED_ERR")},
        { PAM_ACCT_EXPIRED,         QStringLiteral("PAM_ACCT_EXPIRED")},
        { PAM_AUTHTOK_EXPIRED,      QStringLiteral("PAM_AUTHTOK_EXPIRED")},
        { PAM_SESSION_ERR,          QStringLiteral("PAM_SESSION_ERR")},
        { PAM_AUTHTOK_ERR,          QStringLiteral("PAM_AUTHTOK_ERR")},
        { PAM_AUTHTOK_RECOVERY_ERR, QStringLiteral("PAM_AUTHTOK_RECOVERY_ERR")},
        { PAM_AUTHTOK_LOCK_BUSY,    QStringLiteral("PAM_AUTHTOK_LOCK_BUSY")},
        { PAM_AUTHTOK_DISABLE_AGING,QStringLiteral("PAM_AUTHTOK_DISABLE_AGING")},
        { PAM_NO_MODULE_DATA,       QStringLiteral("PAM_NO_MODULE_DATA")},
        { PAM_IGNORE,               QStringLiteral("PAM_IGNORE")},
        { PAM_ABORT,                QStringLiteral("PAM_ABORT")},
        { PAM_TRY_AGAIN,            QStringLiteral("PAM_TRY_AGAIN")},
        { PAM_MODULE_UNKNOWN,       QStringLiteral("PAM_MODULE_UNKNOWN")},
    });

    static const QString &unknown = QStringLiteral("PAM_UNKNOWN_RC");

    if(statusCodes.contains(rc))
        return statusCodes[rc];

    // not in our list
    return unknown;
}

/** @internal return specified environment as list */
QVector<QStringList> Utils::getEnvList(char **env) {

    QVector<QStringList> list;

    char *p, *dup;

    if(!env)
        return list;

    for(char **var=env; var && *var; var++) {
        dup = strdup(*var);
        if(!dup) {
            qWarning() << "strdup failed.";
            return list;
        }
        p = strchr(dup, '=');
        if(p) {
            *(p++) = 0;
            list.append(QStringList { QString(dup), QString(p) } );
        }
        free(dup);
    }

    return list;
}

/** @internal return list of pam environment variables */
QVector<QStringList> Utils::getPamEnvList(pam_handle_t *pam_handle) {
    char **env = pam_getenvlist(pam_handle);
    QVector<QStringList> result = getEnvList(env);
    for(char **var = env; var && *var; var++)
        free(*var);
    if(env) free(env);
    return result;
}

/** @internal return list of environment variables */
QVector<QStringList> Utils::getEnvList() {
    return getEnvList(environ);
}

void Utils::printEnvList(QVector<QStringList> &list, bool pam_env) {
    const char *prefix = pam_env ? "[ENV-PAM] " : "[ENV-USR] ";
    if(list.size()==0)
        qDebug().nospace() << prefix << "<Null>";
    for(int i = 0; i < list.size(); i++) {
        QStringList item = list.at(i);
        qDebug().noquote().nospace() << prefix << item.at(0) << "=" << item.at(1);
    }
}

void Utils::printPamEnvList(pam_handle_t *pam_handle) {
    QVector<QStringList> env = getPamEnvList(pam_handle);
    printEnvList(env, true);
}

void Utils::printEnvList() {
    QVector<QStringList> env = getEnvList();
    printEnvList(env);
}

