#ifndef AUTHENUMS_H
#define AUTHENUMS_H

namespace SDDM {

namespace AuthEnums {

    enum Info {
        INFO_NONE = 0,
        INFO_UNKNOWN,
        INFO_PAM_CONV,
        INFO_PASS_CHANGE_REQUIRED,
        _INFO_LAST
    };

    enum Error {
        ERROR_NONE = 0,
        ERROR_UNKNOWN,
        ERROR_PAM_CONV,
        ERROR_AUTHENTICATION,
        ERROR_INTERNAL,
        _ERROR_LAST
    };

    enum HelperExitStatus {
        HELPER_SUCCESS = 0,
        HELPER_AUTH_ERROR,
        HELPER_SESSION_ERROR,
        HELPER_OTHER_ERROR
    };
}

}

#endif // AUTHENUMS_H
