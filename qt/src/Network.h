#pragma once

#include <QString>

namespace Net {

struct LoginResult { bool success=false; QString message; };

bool checkConnection(int timeoutMs = 1000);
LoginResult login(const QString& account, const QString& password);

}

