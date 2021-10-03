#include "state.h"

namespace state {

QString AppState::qt_version() const {
    return QString(qVersion());
}

}
